/**

Copyright (c) 2006 - 2019, Intel Corporation.

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.

  @file SpsPei.c

    This driver manages the initial phase of SPS ME firmware support specified in
    SPS ME-BIOS Interface Specification.

  **/
#include <Base.h>
#include <PiPei.h>
#include <Uefi.h>

#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/PciCfg.h>
#include <Ppi/EndOfPeiPhase.h>
#include <Ppi/HeciPpi.h>
#include <Ppi/SiPolicy.h>

#include <Guid/SpsInfoHobGuid.h>

#include <Guid/GlobalVariable.h>

#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/PciLib.h>
#include <Library/TimerLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/PchPcrLib.h>
#include <Library/MeTypeLib.h>

#include <Register/PchRegsPsf.h>    //Common definitions
#include <Register/PchRegsPsfCnl.h> //Project specific definitions

#include <PchAccess.h>

#include <CpuRegs.h>
#include <Register/Cpuid.h>
#include <Register/ArchitecturalMsr.h>
#include <MeHeciRegs.h>
#include <Sps.h>
#include <SpsPei.h>
#include <Ppi/SpsHwChangePpi.h>

#include <MePolicyHob.h>
#include <Library/HobLib.h>

/*****************************************************************************
 * Local definitions
 */
#define SPS_INIT_TIMEOUT  2000      // [ms] Timeout when waiting for InitComplete
#define SPS_RESET_TIMEOUT 5000000  // [us] Timeout when waiting for InitComplete
#define STALL_1US         1
#define STALL_1MS         1000
#define STALL_1S          1000000


/*****************************************************************************
 * Local functions prototypes
 */

EFI_STATUS SpsNonS3Path (IN CONST EFI_PEI_SERVICES**, IN EFI_PEI_NOTIFY_DESCRIPTOR*, IN VOID*);
EFI_STATUS SpsS3Path (IN CONST EFI_PEI_SERVICES**, IN EFI_PEI_NOTIFY_DESCRIPTOR*, IN VOID*);
EFI_STATUS SpsHwChangeSetStatus (SPS_HW_CHANGE_PPI*, BOOLEAN, BOOLEAN);

/*****************************************************************************
 * Variables
 */
static EFI_PEI_NOTIFY_DESCRIPTOR  mSpsPeiNonS3NotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEndOfPeiSignalPpiGuid,
    SpsNonS3Path
  }
};
static EFI_PEI_NOTIFY_DESCRIPTOR  mSpsPeiS3NotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEndOfPeiSignalPpiGuid,
    SpsS3Path
  }
};

static SPS_HW_CHANGE_PPI mSpsHwChangePpi = {
  SpsHwChangeSetStatus
};

static EFI_PEI_PPI_DESCRIPTOR mInstallSpsHwChangePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gSpsHwChangePpiGuid,
  &mSpsHwChangePpi
};

/*****************************************************************************
 @brief
  Set boot mode requested by NM in ME.

  @param[in] PerfOptBoot - Power vs performance optimized boot is requested

  @retval EFI_SUCCESS     Performance was set as requested
  @retval EFI_UNSUPPORTED Processor does not support P-states
**/
EFI_STATUS
NmSetBootMode (
  BOOLEAN PerfOptBoot)
{
  CPUID_VERSION_INFO_ECX  Ecx;
  union
  {
    UINT64 QWord;
    struct
    {
      UINT32 DWordL;
      UINT32 DWordH;
    } Bits;
  } PlatformInfo, PerfCtrl;

  //
  // Check if P-states control (EIST) is supported in processor.
  //
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &Ecx.Uint32, NULL);
  if (! (Ecx.Bits.EIST))
  {
    return EFI_UNSUPPORTED;
  }
  //
  // In Platform Info MSR we got minimum and maximum ratio for processor.
  //
  PlatformInfo.QWord = AsmReadMsr64 (MSR_PLATFORM_INFO);
  //
  // Read current Peformance Control MSR, clear ratio bits
  //
  PerfCtrl.QWord = AsmReadMsr64 (MSR_IA32_PERF_CTL) & ~0x7F00;

  if (PerfOptBoot)
  { //
    // Set max ratio in Performance Control MSR
    //
    PerfCtrl.Bits.DWordL |= PlatformInfo.Bits.DWordL & 0x7F00;
    DEBUG ((DEBUG_INFO, "[SPS] NM Boot Mode: Performance Optimized\n"));
  }
  else
  { //
    // Set min ratio in Performance Control MSR
    //
    PerfCtrl.Bits.DWordL |= PlatformInfo.Bits.DWordH & 0x7F00;
    DEBUG ((DEBUG_INFO, "[SPS] NM Boot Mode: Power Optimized\n"));
  }
  AsmWriteMsr64 (MSR_IA32_PERF_CTL, PerfCtrl.QWord);

  return EFI_SUCCESS;
} // NmSetBootMode ()


/*****************************************************************************
 @brief
   Create HOB with SPS info for DXE

 @param[in] PeiServices   General purpose services available to every PEIM
 @param[in] WorkFlow      The state of ME firmware observed at PEI
 @param[in] FeatureSet    ME features enabled reported in Get ME-BIOS Interface response
 @param[in] PwrOptBoot    Whether NM in ME requests power optimized boot
 @param[in] Cores2Disable Whetherm NM in ME requests disabling cores

 @retval EFI_SUCCESS      The function completed successfully.
 @retval EFI_OUT_OF_RESOURCES HOB creation failed
**/
SPS_INFO_HOB*
SpsHobCreate (
  SPS_FLOW       WorkFlow,
  UINT32         Mefs1,
  UINT8          IfVerMajor,
  UINT8          IfVerMinor,
  UINT32         FeatureSet,
  UINT32         FeatureSet2,
  BOOLEAN        PwrOptBoot,
  UINT8          Cores2Disable)
{
  SPS_INFO_HOB   SpsHob;
  SPS_INFO_HOB  *pSpsHob;

  DEBUG ((DEBUG_INFO, "[SPS] Building HOB: flow %d, MEFS %08X, ME-BIOS ver %d.%d, "
                     "features %08X, features 2 %08X, boot mode %d, cores to disable %d\n",
         WorkFlow, Mefs1, IfVerMajor, IfVerMinor, FeatureSet, FeatureSet2, PwrOptBoot, Cores2Disable));

  //
  // Zero the HOB buffer
  //
  ZeroMem (&SpsHob, sizeof (SpsHob));
  //
  // Initialize the HOB data
  //
  SpsHob.WorkFlow         = WorkFlow;
  SpsHob.Mefs1            = Mefs1;
  SpsHob.IfVerMajor       = IfVerMajor;
  SpsHob.IfVerMinor       = IfVerMinor;
  SpsHob.FeatureSet.Data.Set1 = FeatureSet;
  SpsHob.FeatureSet.Data.Set2 = FeatureSet2;
  SpsHob.PwrOptBoot       = PwrOptBoot;
  SpsHob.Cores2Disable    = Cores2Disable;

  pSpsHob = BuildGuidDataHob (&gSpsInfoHobGuid, &SpsHob, sizeof (SpsHob));
  if (pSpsHob == NULL)
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: SPS HOB could not be registered"));
  }

  return pSpsHob;
} // SpsHobCreate ()


/*****************************************************************************
 @brief
  Waits For SPS ME FW initialization

  @param[in] pSpsPolicy    SPS confgiguration

  @retval EFI_SUCCESS      Success
  @retval EFI_UNSUPPORTED  Failure, go to non-functional flow

**/
EFI_STATUS
WaitMeFwInitialization (
    SI_PREMEM_POLICY_PPI *pSpsPolicy)
{
  UINT32 SpsInitTimeout;
  SPS_MEFS1 Mefs1;
  SPS_MEFS2 Mefs2;

  SpsInitTimeout = SPS_INIT_TIMEOUT / STALL_1MS;

  //
  // Wait for ME init complete. Timeout is 2 seconds.
  //
  DEBUG ((DEBUG_INFO, "[SPS] Waiting for ME firmware init complete\n"));
  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  while (!Mefs1.Bits.InitComplete) {
    if (!SpsInitTimeout) {
      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Timeout when waiting for ME init complete\n"));
      return EFI_UNSUPPORTED;
    }
    MicroSecondDelay (STALL_1MS);
    Mefs1.DWord = SpsHeciPciReadMefs1 ();
    SpsInitTimeout--;
  }

  switch (Mefs1.Bits.CurrentState) {
  case MEFS1_CURSTATE_RECOVERY:
    Mefs2.DWord = SpsHeciPciReadMefs2 ();
    DEBUG ((DEBUG_WARN, "[SPS] WARNING: ME is in recovery mode (cause: %d)\n", Mefs2.Bits.RecoveryCause));
    // Fall through to normal case
  case MEFS1_CURSTATE_NORMAL:
    break;

  default:
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;

} // WaitMeFwInitialization ()

/*****************************************************************************
 @brief
  This function is called on S3 resume path. It sends END_OF_POST message to
  SPS firmware.

  NOTE: This function runs after S3 script. It MUST PRESERVE content of
        all the registers it uses.

  @param[in] PeiServices      unused
  @param[in] NotifyDescriptor unused
  @param[in] Ppi              unused

  @retval EFI_SUCCESS             MeMode copied
  @retval EFI_INVALID_PARAMETER   Pointer of MeMode is invalid
**/
EFI_STATUS SpsS3Path (
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDescriptor,
  IN VOID                      *Ppi)
{
  EFI_STATUS                    Status;
  HECI_PPI                     *pHeciPpi;
  UINT32                        Len;
  SI_PREMEM_POLICY_PPI         *SiPreMemPolicyPpi = NULL;
  SPS_MEFS1                     Mefs1;

  UINT32 FuncDisHeci1;
  UINT32 MbarL = 0;
  UINT32 MbarH = 0;
  UINT8  Cmd = 0;
  UINT8  Irq = 0;

  union
  {
    MKHI_MSG_END_OF_POST_REQ EopReq;
    MKHI_MSG_END_OF_POST_RSP EopRsp;
  } HeciMsg;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    SiPreMemPolicyPpi = NULL;
  }

  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  Status = WaitMeFwInitialization (SiPreMemPolicyPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: ME not initialized correctly (%r)\n", Status));
    SpsHobCreate (SpsFlowMeErr, Mefs1.DWord, 0, 0, 0, 0, FALSE, 0);
    return Status;
  }

  Status = (**PeiServices).LocatePpi (PeiServices, &gHeciPpiGuid,
                                     0, NULL, &pHeciPpi);
  if (EFI_ERROR (Status))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot locate HECI PPI\n"));
    return Status;
  }
  //
  // Save the registers that may change in this function
  //
  FuncDisHeci1 = MmioRead32 (PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN));
  MmioWrite32 (PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI2_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN),
              FuncDisHeci1 & ~B_PCH_PSFX_PCR_T0_SHDW_PCIEN_FUNDIS);
  MbarL = MeHeci1PciRead32 (HECI_R_MBAR);
  MbarH = MeHeci1PciRead32 (HECI_R_MBAR + 4);
  Cmd = MeHeci1PciRead8 (HECI_R_CMD);
  Irq = MeHeci1PciRead8 (HECI_R_IRQ);
  //
  // If HECI is already configured to 64-bit address force its reinitialization to 32-bit.
  // PEI works in 32-bit.
  //
  if (MbarH != 0 && (MbarL & 0x4))
  {
    MeHeci1PciWrite32 (HECI_R_MBAR, 0);
  }
  pHeciPpi->InitializeHeci (HECI1_DEVICE);

  DEBUG ((DEBUG_INFO, "[SPS] Sending END_OF_POST to ME\n"));
  HeciMsg.EopReq.Mkhi.Data = 0x00000CFF;
  Len = sizeof (HeciMsg);
  Status = pHeciPpi->SendwAck (HECI1_DEVICE, (UINT32*)&HeciMsg.EopReq,
                          sizeof (HeciMsg.EopReq), &Len, SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);
  if (EFI_ERROR (Status))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot send END_OF_POST (%r)\n", Status));
  }
  else if (HeciMsg.EopRsp.Mkhi.Data != 0x00008CFF)
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Invalid END_OF_POST response (MKHI: 0x%X)\n",
                        HeciMsg.EopRsp.Mkhi.Data));
  }
  //
  // Restore the registers that may have changed
  //
  DEBUG ((DEBUG_INFO, "[SPS] Restoring S3 Boot Script MBARs\n"));
  MeHeci1PciWrite32 (HECI_R_MBAR + 4, MbarH);
  MeHeci1PciWrite32 (HECI_R_MBAR, MbarL);
  MeHeci1PciWrite8 (HECI_R_IRQ, Irq);
  MeHeci1PciWrite8 (HECI_R_CMD, Cmd);
  MmioAndThenOr32 (
   PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN),
   (UINT32)~B_PCH_PSFX_PCR_T0_SHDW_PCIEN_FUNDIS, FuncDisHeci1);

  return EFI_SUCCESS;
} // SpsS3Path ()


/*****************************************************************************
 @brief
  This function is called on non S3 boot path.

  @param[in] PeiServices      unused
  @param[in] NotifyDescriptor unused
  @param[in] Ppi              unused

  @retval EFI_SUCCESS             MeMode copied
  @retval EFI_INVALID_PARAMETER   Pointer of MeMode is invalid
**/
EFI_STATUS SpsNonS3Path (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi)
{
  EFI_STATUS                     Status;
  SPS_MEFS1                      Mefs1;
  SPS_NMFS                       Nmfs;
  SPS_FEATURE_SET                SpsFeatureSet;
  HECI_PPI                      *pHeciPpi;
  UINT32                         RspLen;
  SI_PREMEM_POLICY_PPI          *SiPreMemPolicyPpi = NULL;
  ME_PEI_PREMEM_CONFIG          *MePeiPreMemConfig = NULL;
  union
  {
    SPS_MSG_GET_MEBIOS_INTERFACE_REQ MeBiosVerReq;
    SPS_MSG_GET_MEBIOS_INTERFACE_RSP MeBiosVerRsp;
  } HeciMsg;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    SiPreMemPolicyPpi = NULL;
  }

  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  Status = WaitMeFwInitialization (SiPreMemPolicyPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: ME not initialized correctly (%r)\n", Status));
    SpsHobCreate (SpsFlowMeErr, Mefs1.DWord, 0, 0, 0, 0, FALSE, 0);
    return Status;
  }

  Status = (**PeiServices).LocatePpi (PeiServices, &gHeciPpiGuid,
                                     0, NULL, &pHeciPpi);
  if (EFI_ERROR (Status))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot locate HECI PPI\n"));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "[SPS] Sending ME-BIOS Interface Version request\n"));
  HeciMsg.MeBiosVerReq.Command = SPS_CMD_GET_MEBIOS_INTERFACE_REQ;
  RspLen = sizeof (HeciMsg);
  Status = pHeciPpi->SendwAck (HECI1_DEVICE, (UINT32*)&HeciMsg.MeBiosVerReq,
                              sizeof (HeciMsg.MeBiosVerReq), &RspLen,
                              SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_SPS);
  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  if (EFI_ERROR (Status))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot send SPS_GET_MEBIOS_INTERFACE (%r)\n", Status));
  }
  else if (HeciMsg.MeBiosVerRsp.Command != SPS_CMD_GET_MEBIOS_INTERFACE_RSP ||
           RspLen < sizeof (HeciMsg.MeBiosVerRsp))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Invalid GET_MEBIOS_INTERFACE response "
                        " (cmd: 0x%X, len %d)\n", HeciMsg.MeBiosVerRsp.Command, RspLen));
    Status = EFI_ABORTED;
  }
  else if ((Mefs1.Bits.CurrentState == MEFS1_CURSTATE_RECOVERY) ?
         !SpsMeBiosRcvVersionVerify (HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor) :
         !SpsMeBiosOprVersionVerify (HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor))
  {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Incompatible SPS ME-BIOS interface definition %d.%d\n",
                        HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor));
    Status = EFI_UNSUPPORTED;
  }
  if (EFI_ERROR (Status))
  {
    SpsHobCreate (SpsFlowMeErr, Mefs1.DWord,
                 HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor, 0, 0, FALSE, 0);
    return Status;
  }
  DEBUG ((DEBUG_INFO, "[SPS] SPS ME-BIOS interface version is %d.%d\n",
         HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor));
  DEBUG ((DEBUG_INFO, "      Feature set is   0x%08X\n", HeciMsg.MeBiosVerRsp.FeatureSet));
  DEBUG ((DEBUG_INFO, "      Feature set 2 is 0x%08X\n", HeciMsg.MeBiosVerRsp.FeatureSet2));

  SpsFeatureSet.Data.Set1 = HeciMsg.MeBiosVerRsp.FeatureSet;
  SpsFeatureSet.Data.Set2 = HeciMsg.MeBiosVerRsp.FeatureSet2;
  if (SpsFeatureSet.Bits.NodeManager)
  {
    Nmfs.DWord = SpsHeciPciReadNmfs ();
    if (Nmfs.DWord != 0xFFFFFFFF && Nmfs.Bits.NmEnabled)
    {
      Status = PeiServicesLocatePpi (
                 &gSiPreMemPolicyPpiGuid,
                 0,
                 NULL,
                 (VOID **) &SiPreMemPolicyPpi
                 );
      ASSERT_EFI_ERROR (Status);
      if (EFI_ERROR (Status)) {
        SpsHobCreate (SpsFlowMeErr, Mefs1.DWord,
                     HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor, 0, 0, FALSE, 0);
        return Status;
      }
      Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gMePeiPreMemConfigGuid, (VOID *) &MePeiPreMemConfig);
      ASSERT_EFI_ERROR (Status);

      if (MePeiPreMemConfig == NULL) {
        SpsHobCreate (SpsFlowMeErr, Mefs1.DWord,
                     HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor, 0, 0, FALSE, 0);
        Status = EFI_NOT_READY;
        DEBUG ((EFI_D_ERROR, "[SPS] ERROR: Get policy failed (%r)\n", Status));
        ASSERT_EFI_ERROR (Status);
        return Status;
      }
      if (MePeiPreMemConfig->NmPwrOptBootOverride)
      {
        Nmfs.Bits.PerfOptBoot = !MePeiPreMemConfig->NmPwrOptBoot;
      }
      if (MePeiPreMemConfig->NmCores2DisableOverride)
      {
        Nmfs.Bits.Cores2Disable = MePeiPreMemConfig->NmCores2Disable;
      }
      NmSetBootMode ((BOOLEAN)Nmfs.Bits.PerfOptBoot);
    }
    else
    {
      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: NMFS not configured while NM enabled "
                          " (feature set: 0x%08X, feature set 2: 0x%08X, NMFS: 0x%08X)\n",
                          SpsFeatureSet.Data.Set1, SpsFeatureSet.Data.Set2, Nmfs.DWord));
    }
    //
    // Register SPS_HW_CHANGE_PPI for PTU support
    //
    Status = PeiServicesInstallPpi(&mInstallSpsHwChangePpi);
    ASSERT_EFI_ERROR (Status);
  }
  else
  {
    Nmfs.DWord = 1; // no cores to disable, performance optimized boot
  }
  SpsHobCreate (SpsFlowMeOk, Mefs1.DWord,
               HeciMsg.MeBiosVerRsp.VerMajor, HeciMsg.MeBiosVerRsp.VerMinor,
               HeciMsg.MeBiosVerRsp.FeatureSet,
               HeciMsg.MeBiosVerRsp.FeatureSet2,
               !Nmfs.Bits.PerfOptBoot, (UINT8)Nmfs.Bits.Cores2Disable);

  return EFI_SUCCESS;
} // SpsNonS3Path ()

/*****************************************************************************
 @brief
   Function reads ME Reset Counter from ME FS1

 @retval UINT       Read ME Reset Counter
**/
UINTN
GetMeResetCounter (
  VOID
  )
{
  SPS_MEFS1 Mefs1;

  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  return Mefs1.Bits.MeResetCounter;
} // GetMeResetCounter ()

/*****************************************************************************
 @brief
   Function SPS meaningfull state (Normal or Recovery)

    @param WaitTimeout -  Pointer to wait timeout

    @retval EFI_SUCCESS   - ME is in Normal in Recovery state
    @retval EFI_TIMEOUT   - ME is not in Normal or Recovery state
                            within defined timeout
**/

EFI_STATUS
WaitForMeMeaningfulState (
  UINT32  *SpsMeaningfulStateTimeout
  )
{
  SPS_MEFS1 Mefs1;
  UINT32    SpsTimeout = SPS_INIT_TIMEOUT / STALL_1MS;

  DEBUG ((DEBUG_INFO, "[SPS] Wait for ME Normal or Recovery state\n"));
  if (SpsMeaningfulStateTimeout == NULL) {
    SpsMeaningfulStateTimeout = &SpsTimeout;
  }

  // Wait for ME meaningful state
  Mefs1.DWord = SpsHeciPciReadMefs1 ();
  while ((Mefs1.Bits.CurrentState != MEFS1_CURSTATE_RECOVERY) &&
         (Mefs1.Bits.CurrentState != MEFS1_CURSTATE_NORMAL)) {

    if (!*SpsMeaningfulStateTimeout) {
      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Timeout when waiting for ME meaningful state\n"));
      return EFI_TIMEOUT;
    }
    MicroSecondDelay (STALL_1MS);
    Mefs1.DWord = SpsHeciPciReadMefs1 ();
    (*SpsMeaningfulStateTimeout)--;
  }

  DEBUG ((DEBUG_INFO, "[SPS] Wait for ME state reaches ME State = %d\n",
         Mefs1.Bits.CurrentState));

  return EFI_SUCCESS;
} // WaitForMeMeaningfulState ()

/*****************************************************************************
 @brief
   Function waits for ME Reset Counter change.

  @param[in] StartResetCounter Reset counter we wait to be changed
  @param[in] pSpsInitTimeout  reset counter we wait to be changed

 @retval EFI_SUCCESS       The function completed successfully.
 @retval EFI_TIMEOUT       Timeout on ME transition
**/
EFI_STATUS
WaitMeResetCounterChange (
  IN UINTN   StartResetCounter,
  IN UINT32  *pSpsInitTimeout
  )
{
  UINT32 InitTimeout = SPS_INIT_TIMEOUT / STALL_1MS;
  if (pSpsInitTimeout == NULL) {
    pSpsInitTimeout = &InitTimeout;
  }

  while (StartResetCounter == GetMeResetCounter ()) {
    if (!*pSpsInitTimeout) {

      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Timeout when waiting for ME Reset Counter Change\n"));
      return EFI_TIMEOUT;
    }
    MicroSecondDelay (STALL_1MS);
    (*pSpsInitTimeout)--;
  }
  return EFI_SUCCESS;
} // WaitMeResetCounterChange ()


/*****************************************************************************
 @brief
   Execute SPS pre DID reset

  @param[in] VOID

 @retval EFI_SUCCESS       The function completed successfully.
 @retval EFI_DEVICE_ERROR  No Heci device
 @retval EFI_TIMEOUT       Timeout on ME transition
**/
EFI_STATUS
SpsExecutePreDidReset (
  VOID
  )
{
  EFI_STATUS Status = EFI_SUCCESS;
  UINT32 SpsInitTimeout = SPS_RESET_TIMEOUT / STALL_1MS;
  HECI_PPI *pHeciPpi;
  UINT32 RspLen;
  UINTN resetCounter;
  union {
    MKHI_MSG_HMRFPO_MERESET_REQ Req;
    MKHI_MSG_HMRFPO_MERESET_RSP Rsp;
  } HeciMsg;

  DEBUG ((DEBUG_WARN, "[SPS] WARNING: Execute ME pre-DID reset\n"));

  Status = PeiServicesLocatePpi (&gHeciPpiGuid, 0, NULL, (VOID**) &pHeciPpi);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot locate HECI PPI (%r)\n", Status));
    ASSERT_EFI_ERROR (Status);
    return EFI_DEVICE_ERROR;
  }

  resetCounter = GetMeResetCounter ();

  HeciMsg.Req.Mkhi.Data = 0;
  HeciMsg.Req.Mkhi.Fields.Command = HMRFPO_CMD_MERESET;
  HeciMsg.Req.Mkhi.Fields.IsResponse = 0;
  HeciMsg.Req.Mkhi.Fields.GroupId = MKHI_GRP_HMRFPO;
  HeciMsg.Req.Mkhi.Fields.Reserved = 0;
  HeciMsg.Req.Mkhi.Fields.Result = 0;
  HeciMsg.Req.Nonce = 0;

  RspLen = sizeof (HeciMsg);
  Status = pHeciPpi->SendwAck (HECI1_DEVICE, (UINT32*) &HeciMsg.Req,
      sizeof (HeciMsg.Req), &RspLen,
      SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_SPS);
  if (!EFI_ERROR (Status)) {
    // check if reset has been accepted
    if ((HeciMsg.Req.Mkhi.Fields.Command == HMRFPO_CMD_MERESET)
        && (HeciMsg.Req.Mkhi.Fields.GroupId == MKHI_GRP_HMRFPO)
        && (HeciMsg.Req.Mkhi.Fields.IsResponse == 1)
        && (HeciMsg.Rsp.Status == 0)) {
      // Wait for reset counter change
      Status = WaitMeResetCounterChange (resetCounter, &SpsInitTimeout);
      if (!EFI_ERROR (Status)) {
        Status = WaitForMeMeaningfulState (&SpsInitTimeout);
        if (!EFI_ERROR (Status)) {
          DEBUG ((DEBUG_INFO, "[SPS] Pre-DID reset has been executed successfully\n"));
        } else {
          Status = EFI_TIMEOUT;
        }
      } else {
        DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Pre-DID reset, ME Reset Counter not changed (%r)\n", Status));
      }
    } else {
      DEBUG ((DEBUG_ERROR, "[SPS] Pre-DID reset is not accepted by ME. Continue without reset (%r)\n", Status));
    }
  } else {
    DEBUG ((DEBUG_ERROR, "[SPS] Pre-DID reset, no ACK. Continue without reset (%r)\n", Status));
    Status = EFI_SUCCESS;
  }

  return Status;
} // SpsExecutePreDidReset ()


/*****************************************************************************
* @brief
*   SPS PEI entry point.
*
* @param[in] FileHandle  PEIM file handle
* @param[in] PeiServices General purpose services available to every PEIM
*
* @retval EFI_SUCCESS     The function completed successfully.
* @retval EFI_UNSUPPORTED Non SPS firmware is running in ME.
**/
EFI_STATUS
SpsPeiEntryPoint (
    IN EFI_PEI_FILE_HANDLE FileHandle,
    IN CONST EFI_PEI_SERVICES **PeiServices)
{
  EFI_STATUS Status;
  UINT32 SpsInitTimeout;
  SPS_FLOW SpsFlow = SpsFlowMeErr;
  SPS_MEFS1 Mefs1;
  SPS_MEFS2 Mefs2;
  EFI_BOOT_MODE BootMode;
  EFI_PEI_NOTIFY_DESCRIPTOR *pNotifyList;
  SI_PREMEM_POLICY_PPI *SiPreMemPolicyPpi = NULL;
  ME_PEI_PREMEM_CONFIG *MePeiPreMemConfig = NULL;

  //
  // Make sure necessary interfaces are not disabled
  //
  MeDeviceControl (HECI1, TRUE);
  MeDeviceControl (HECI2, TRUE);
  //
  // Wait for ME init complete. Default timeout is 2 seconds.
  //

  SpsInitTimeout = SPS_INIT_TIMEOUT;
  Status = PeiServicesLocatePpi (&gSiPreMemPolicyPpiGuid, 0,
  NULL, (VOID **) &SiPreMemPolicyPpi);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gMePeiPreMemConfigGuid,
      (VOID *) &MePeiPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  if (MePeiPreMemConfig != NULL) {
    SpsInitTimeout = MePeiPreMemConfig->MeInitTimeout;
  } else {
    Status = EFI_NOT_READY;
    DEBUG ((EFI_D_ERROR, "[SPS] ERROR: Get policy failed (%r)\n", Status));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "[SPS] Waiting up to %d ms for ME firmware init complete\n", SpsInitTimeout));
  while (1) {
    if (MeTypeIsDfx ()) {
      DEBUG ((DEBUG_WARN, "[SPS] WARNING: DFX firmware detected in ME\n"));
      SpsFlow = SpsFlowNonSps;
      goto MeNonFunctional;
    }
    Mefs1.DWord = SpsHeciPciReadMefs1 ();
    if (Mefs1.Bits.ErrorCode != MEFS1_ERRCODE_NOERROR) {
      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Timeout when waiting for ME init complete\n"));
      SpsFlow = SpsFlowMeErr;
      goto MeNonFunctional;
    }
    if (Mefs1.Bits.CurrentState > MEFS1_CURSTATE_INIT && !MeTypeIsSps ()) {
      DEBUG((DEBUG_WARN, "[SPS] WARNING: Non SPS firmware detected in ME\n"));
      SpsFlow = SpsFlowNonSps;
      goto MeNonFunctional;
    }
    if (Mefs1.Bits.InitComplete) {
      break;
    }
    if (SpsInitTimeout-- == 0) {
      DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Timeout when waiting for ME init complete\n"));
      SpsFlow = SpsFlowMeErr;
      goto MeNonFunctional;
    }
    MicroSecondDelay (STALL_1MS);
  }
  //
  // Verify if it is SPS firmware running in ME.
  // If not just stop talking to ME.
  //
  if (!MeTypeIsSps ()) {
    goto MeNonFunctional;
  }
  switch (Mefs1.Bits.CurrentState) {
    case MEFS1_CURSTATE_RECOVERY:
      Mefs2.DWord = SpsHeciPciReadMefs2 ();
      DEBUG ((DEBUG_WARN, "[SPS] WARNING: ME is in recovery mode (cause: %d)\n",
                         Mefs2.Bits.RecoveryCause));
      // Fall through to normal case
    case MEFS1_CURSTATE_NORMAL:
      break;

    default:
      goto MeNonFunctional;
  }
  //
  // Install function to be called once HECI initialization for PEI is performed
  //
  BootMode = BOOT_WITH_DEFAULT_SETTINGS;
  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Cannot read boot mode (%r)\n", Status));
    BootMode = BOOT_WITH_DEFAULT_SETTINGS;
  }

  //
  // Verify if it is SPS firmware running in Recovery.
  // If yes, try reset SPS
  //
  DEBUG ((DEBUG_INFO, "[SPS] Pre-DID reset "));
  if (BootMode == BOOT_ON_S3_RESUME
      || !MePeiPreMemConfig->PreDidMeResetEnabled) {
    DEBUG ((DEBUG_INFO, " is disabled\n"));

  } else {
    // [SPS] Pre-DID reset
    DEBUG ((DEBUG_INFO, " execution\n"));
    Status = WaitForMeMeaningfulState (NULL);
    if (EFI_ERROR (Status)) {
      goto MeNonFunctional;
    }

    Mefs1.DWord = SpsHeciPciReadMefs1 ();
    if (Mefs1.Bits.CurrentState == MEFS1_CURSTATE_RECOVERY) {

      Mefs2.DWord = SpsHeciPciReadMefs2 ();
      if (Mefs2.Bits.RecoveryCause == SPS_RCAUSE_MEERROR) {

        DEBUG ((DEBUG_WARN, "[SPS] WARNING: ME is in recovery mode (cause: %d)\n", Mefs2.Bits.RecoveryCause));
        Status = SpsExecutePreDidReset ();
        Mefs1.DWord = SpsHeciPciReadMefs1 ();
        switch (Status) {
        case EFI_TIMEOUT:
          DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Pre-DID reset timeout failure causes ME non-functional flow\n"));
          goto MeNonFunctional;
        case EFI_SUCCESS:
          if (Mefs1.Bits.CurrentState == MEFS1_CURSTATE_NORMAL) {
            DEBUG ((DEBUG_INFO, "[SPS] Pre-DID reset finished successfully, ME in Normal state\n"));
          }
          break;
        default:
          DEBUG ((DEBUG_ERROR, "[SPS] ERROR: Pre-DID reset failure. Continue according to ME state\n"));
          break;
        }
      }
    }
  }

  if (BootMode == BOOT_ON_S3_RESUME) {
    DEBUG ((DEBUG_INFO, "[SPS] S3 resume path\n"));
    pNotifyList = &mSpsPeiS3NotifyList[0];
  } else {
    DEBUG ((DEBUG_INFO, "[SPS] Non S3 boot path\n"));
    pNotifyList = &mSpsPeiNonS3NotifyList[0];
  }
  Status = (*PeiServices)->NotifyPpi (PeiServices, pNotifyList);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[SPS] ERROR: Cannot register PPI notify handler (%r)\n", Status));
  }
  return EFI_SUCCESS;

MeNonFunctional:
  SpsHobCreate (SpsFlow, Mefs1.DWord, 0, 0, 0, 0, FALSE, 0);
  return EFI_UNSUPPORTED;
} // SpsPeiEntryPoint ()

/*****************************************************************************
 @brief
  This function sets power measurement suport status and hardware configuration
  change status to be sent to ME. It is needed when platform power measurement
  using Power/Thermal Utility (PTU) Option ROM is supported.

  NOTE: This function can be used only when Node Manager is enabled in ME.

  @param[in] pThis            Pointer to this PPI
  @param[in] PwrMsmtSupport   True if power measurement using PTU is supported,
                              otherwise false.
  @param[in] HwCfgChanged     True if hardware configuration changed since
                              last boot, otherwise false.

  @return EFI_STATUS is returned.
**/
EFI_STATUS EFIAPI
SpsHwChangeSetStatus (
  IN     SPS_HW_CHANGE_PPI *pThis,
  IN     BOOLEAN            PwrMsmtSupport,
  IN     BOOLEAN            HwCfgChanged
  )
{
  EFI_HOB_GUID_TYPE        *pGuidHob;
  SPS_INFO_HOB             *pSpsHob;

  pGuidHob = GetFirstGuidHob(&gSpsInfoHobGuid);
  pSpsHob = GET_GUID_HOB_DATA(pGuidHob);
  if (pGuidHob == NULL || !pSpsHob->FeatureSet.Bits.NodeManager) {
    return EFI_UNSUPPORTED;
  }
  pSpsHob->NmPowerMsmtSupport = PwrMsmtSupport;
  pSpsHob->NmHwChangeStatus = HwCfgChanged;
  return EFI_SUCCESS;
}
