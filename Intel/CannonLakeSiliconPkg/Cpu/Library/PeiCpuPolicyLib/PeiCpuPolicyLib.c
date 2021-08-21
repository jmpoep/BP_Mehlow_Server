/** @file
  This file is PeiCpuPolicy library.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary and
  confidential information of Intel Corporation and its suppliers and licensors,
  and is protected by worldwide copyright and trade secret laws and treaty
  provisions. No part of the Material may be used, copied, reproduced, modified,
  published, uploaded, posted, transmitted, distributed, or disclosed in any way
  without Intel's prior express written permission.

  No license under any patent, copyright, trade secret or other intellectual
  property right is granted to or conferred upon you by disclosure or delivery
  of the Materials, either expressly, by implication, inducement, estoppel or
  otherwise. Any license under such intellectual property rights must be
  express and approved by Intel in writing.

  Unless otherwise agreed by Intel in writing, you may not remove or alter
  this notice or any other notice embedded in Materials by Intel or
  Intel's suppliers or licensors in any way.

  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
  the terms of your license agreement with Intel or your vendor. This file may
  be modified by the user, subject to additional terms of the license agreement.

@par Specification
**/
#include "PeiCpuPolicyLibrary.h"
#include <SaAccess.h>
#include <CpuAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/PciSegmentLib.h>
#include <Library/SaPlatformLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>
#include <Library/PcdLib.h>

#ifndef FSP_FLAG
/**
  Get the next microcode patch pointer.

  @param[in, out] MicrocodeData - Input is a pointer to the last microcode patch address found,
                                  and output points to the next patch address found.

  @retval EFI_SUCCESS           - Patch found.
  @retval EFI_NOT_FOUND         - Patch not found.
**/
EFI_STATUS
EFIAPI
RetrieveMicrocode (
  IN OUT CPU_MICROCODE_HEADER **MicrocodeData
  )
{
  UINTN                MicrocodeStart;
  UINTN                MicrocodeEnd;
  UINTN                TotalSize;

  if ((FixedPcdGet32 (PcdFlashMicrocodeFvBase) == 0) || (FixedPcdGet32 (PcdFlashMicrocodeFvSize) == 0)) {
    return EFI_NOT_FOUND;
  }

  ///
  /// Microcode binary in SEC
  ///
  MicrocodeStart = (UINTN) FixedPcdGet32 (PcdFlashMicrocodeFvBase) +
          ((EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FixedPcdGet32 (PcdFlashMicrocodeFvBase))->HeaderLength +
          sizeof (EFI_FFS_FILE_HEADER);

  MicrocodeEnd = (UINTN) FixedPcdGet32 (PcdFlashMicrocodeFvBase) + (UINTN) FixedPcdGet32 (PcdFlashMicrocodeFvSize);

  if (*MicrocodeData == NULL) {
    *MicrocodeData = (CPU_MICROCODE_HEADER *) (UINTN) MicrocodeStart;
  } else {
    if (*MicrocodeData < (CPU_MICROCODE_HEADER *) (UINTN) MicrocodeStart) {
      DEBUG ((DEBUG_INFO, "[CpuPolicy]*MicrocodeData < MicrocodeStart \n"));
      return EFI_NOT_FOUND;
    }

    TotalSize = (UINTN) ((*MicrocodeData)->TotalSize);
    if (TotalSize == 0) {
      TotalSize = 2048;
    }

    *MicrocodeData = (CPU_MICROCODE_HEADER *) ((UINTN)*MicrocodeData + TotalSize);
    if (*MicrocodeData >= (CPU_MICROCODE_HEADER *) (UINTN) (MicrocodeEnd) || (*MicrocodeData)->TotalSize == (UINT32) -1) {
      DEBUG ((DEBUG_INFO, "[CpuPolicy]*MicrocodeData >= MicrocodeEnd \n"));
      return EFI_NOT_FOUND;
    }
  }
  return EFI_SUCCESS;
}

/**
  Get the microcode patch pointer.

  @retval EFI_PHYSICAL_ADDRESS - Address of the microcode patch, or NULL if not found.
**/
EFI_PHYSICAL_ADDRESS
PlatformCpuLocateMicrocodePatch (
  VOID
  )
{
  EFI_STATUS           Status;
  CPU_MICROCODE_HEADER *MicrocodeData;
  EFI_CPUID_REGISTER   Cpuid;
  UINT32               UcodeRevision;
  UINTN                MicrocodeBufferSize;
  VOID                 *MicrocodeBuffer = NULL;

  AsmCpuid (
    CPUID_VERSION_INFO,
    &Cpuid.RegEax,
    &Cpuid.RegEbx,
    &Cpuid.RegEcx,
    &Cpuid.RegEdx
    );

  UcodeRevision = GetCpuUcodeRevision ();
  MicrocodeData = NULL;
  while (TRUE) {
    ///
    /// Find the next patch address
    ///
    Status = RetrieveMicrocode (&MicrocodeData);
    DEBUG ((DEBUG_INFO, "MicrocodeData = %x\n", MicrocodeData));

    if (Status != EFI_SUCCESS) {
      break;
    } else if (CheckMicrocode (Cpuid.RegEax, MicrocodeData, &UcodeRevision)) {
      break;
    }
  }

  if (EFI_ERROR (Status)) {
    return (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
  }

  ///
  /// Check that microcode patch size is <= 128K max size,
  /// then copy the patch from FV to temp buffer for faster access.
  ///
  MicrocodeBufferSize = (UINTN) MicrocodeData->TotalSize;

  if (MicrocodeBufferSize <= MAX_MICROCODE_PATCH_SIZE) {
    MicrocodeBuffer = AllocatePages (EFI_SIZE_TO_PAGES (MicrocodeBufferSize));
    if (MicrocodeBuffer != NULL) {
      DEBUG(( DEBUG_INFO, "Copying Microcode to temp buffer.\n"));
      CopyMem (MicrocodeBuffer, MicrocodeData, MicrocodeBufferSize);

      return (EFI_PHYSICAL_ADDRESS) (UINTN) MicrocodeBuffer;
    } else {
      DEBUG(( DEBUG_ERROR, "Failed to allocate enough memory for Microcode Patch.\n"));
    }
  } else {
    DEBUG(( DEBUG_ERROR, "Microcode patch size is greater than max allowed size of 128K.\n"));
  }
  return (EFI_PHYSICAL_ADDRESS) (UINTN) NULL;
}
#endif

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_CONFIG  *CpuConfig;
  CpuConfig  = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuConfig->Header.GuidHob.Name = %g\n", &CpuConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU configuration
  ********************************/
  CpuConfig->AesEnable             = CPU_FEATURE_ENABLE;
#ifndef FSP_FLAG
  CpuConfig->MicrocodePatchAddress = PlatformCpuLocateMicrocodePatch ();
#endif //FSP_FLAG
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadBiosGuardConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  BIOS_GUARD_CONFIG  *BiosGuardConfig;
  BiosGuardConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "BiosGuardConfig->Header.GuidHob.Name = %g\n", &BiosGuardConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "BiosGuardConfig->Header.GuidHob.Header.HobLength = 0x%x\n", BiosGuardConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU BIOS Guard configuration
  ********************************/
  BiosGuardConfig->BgpdtHash[0]                = 0xae7295370672663c;
  BiosGuardConfig->BgpdtHash[1]                = 0x220375c996d23a36;
  BiosGuardConfig->BgpdtHash[2]                = 0x73aaea0f2afded9d;
  BiosGuardConfig->BgpdtHash[3]                = 0x707193b768a0829e;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuSgxConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_SGX_CONFIG  *CpuSgxConfig;
  CpuSgxConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuSgxConfig->Header.GuidHob.Name = %g\n", &CpuSgxConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuSgxConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuSgxConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU SGX configuration
  ********************************/
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPowerMgmtBasicConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_POWER_MGMT_BASIC_CONFIG  *CpuPowerMgmtBasicConfig;
  CPU_SKU      CpuSku;
  MSR_REGISTER TempMsr;

  CpuPowerMgmtBasicConfig = ConfigBlockPointer;
  CpuSku                  = GetCpuSku();

  DEBUG ((DEBUG_INFO, "CpuPowerMgmtBasicConfig->Header.GuidHob.Name = %g\n", &CpuPowerMgmtBasicConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPowerMgmtBasicConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPowerMgmtBasicConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU Power Management Basic configuration
  ********************************/
  CpuPowerMgmtBasicConfig->Hwp                          = TRUE;
  CpuPowerMgmtBasicConfig->HdcControl                   = TRUE;
  CpuPowerMgmtBasicConfig->PowerLimit2                  = TRUE;
  CpuPowerMgmtBasicConfig->PowerLimit3Lock              = TRUE;
  CpuPowerMgmtBasicConfig->TccOffsetLock                = FALSE;
  CpuPowerMgmtBasicConfig->EnableItbm                   = TRUE;
  CpuPowerMgmtBasicConfig->EnableItbmDriver             = FALSE;

  ///
  /// Initialize RATL (Runtime Average Temperature Limit) Config for ULX.
  ///
  if (CpuSku == EnumCpuUlx) {
    CpuPowerMgmtBasicConfig->TccActivationOffset        = 15;
    CpuPowerMgmtBasicConfig->TccOffsetTimeWindowForRatl = 5000; // 5 sec
    CpuPowerMgmtBasicConfig->TccOffsetClamp             = CPU_FEATURE_ENABLE;
  }
  CpuPowerMgmtBasicConfig->TurboMode                    = TRUE;

  TempMsr.Qword = AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT);
  CpuPowerMgmtBasicConfig->OneCoreRatioLimit = TempMsr.Bytes.FirstByte;
  CpuPowerMgmtBasicConfig->TwoCoreRatioLimit = TempMsr.Bytes.SecondByte;
  CpuPowerMgmtBasicConfig->ThreeCoreRatioLimit = TempMsr.Bytes.ThirdByte;
  CpuPowerMgmtBasicConfig->FourCoreRatioLimit = TempMsr.Bytes.FouthByte;
  CpuPowerMgmtBasicConfig->FiveCoreRatioLimit = TempMsr.Bytes.FifthByte;
  CpuPowerMgmtBasicConfig->SixCoreRatioLimit = TempMsr.Bytes.SixthByte;
  CpuPowerMgmtBasicConfig->SevenCoreRatioLimit = TempMsr.Bytes.SeventhByte;
  CpuPowerMgmtBasicConfig->EightCoreRatioLimit = TempMsr.Bytes.EighthByte;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPowerMgmtVrConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  EFI_STATUS                Status;
  CPU_POWER_MGMT_VR_CONFIG  *CpuPowerMgmtVrConfig;
  UINTN                     Index;
  SI_PREMEM_POLICY_PPI      *SiPreMemPolicyPpi;
  CPU_OVERCLOCKING_PREMEM_CONFIG  *CpuOverclockingPreMemConfig;
  CpuPowerMgmtVrConfig      = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuPowerMgmtVrConfig->Header.GuidHob.Name = %g\n", &CpuPowerMgmtVrConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPowerMgmtVrConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPowerMgmtVrConfig->Header.GuidHob.Header.HobLength));

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuOverclockingPreMemConfigGuid, (VOID *) &CpuOverclockingPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize the static defaults from the VR config block for all SKUs.
  // Dynamic VR settings vary by SKU. If left as default 0 (auto), they are updated during VR programming.
  //
  for (Index = 0; Index < MAX_NUM_VRS; Index++) {
    CpuPowerMgmtVrConfig->VrConfigEnable[Index] = TRUE;
    CpuPowerMgmtVrConfig->TdcTimeWindow[Index] = 1;   // 1 ms default
    CpuPowerMgmtVrConfig->Psi1Threshold[Index] = 80;  // 20A is default
    CpuPowerMgmtVrConfig->Psi2Threshold[Index] = 20;  // 5A is default
    CpuPowerMgmtVrConfig->Psi3Threshold[Index] = 4;   // 1A is default
    CpuPowerMgmtVrConfig->Psi3Enable[Index] = TRUE;
    CpuPowerMgmtVrConfig->Psi4Enable[Index] = TRUE;
    //
    // When overclocking is enabled, we need to ensure the VR defaults are
    // optimized for overclocking
    //
    if (CpuOverclockingPreMemConfig->OcSupport == 1) {
      //
      // Disable VR TDC for all VR domains
      //
      CpuPowerMgmtVrConfig->TdcEnable[Index]       = 0;
      CpuPowerMgmtVrConfig->TdcPowerLimit[Index]   = 0;
    }
  }

  if (CpuOverclockingPreMemConfig->OcSupport == 1) {
    //
    // If OC is enabled, disable noise mitigation and keep fast slew rate
    //
    CpuPowerMgmtVrConfig->AcousticNoiseMitigation = 0;
    CpuPowerMgmtVrConfig->FastPkgCRampDisableIa = 0;
    CpuPowerMgmtVrConfig->FastPkgCRampDisableGt = 0;
    CpuPowerMgmtVrConfig->FastPkgCRampDisableSa = 0;
    CpuPowerMgmtVrConfig->FastPkgCRampDisableFivr = 0;
  }

  CpuPowerMgmtVrConfig->VrPowerDeliveryDesign = EnumUnknownCpuVrPowerDesign;
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPowerMgmtCustomConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_POWER_MGMT_CUSTOM_CONFIG  *CpuPowerMgmtCustomConfig;
  CpuPowerMgmtCustomConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuPowerMgmtCustomConfig->Header.GuidHob.Name = %g\n", &CpuPowerMgmtCustomConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPowerMgmtCustomConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPowerMgmtCustomConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU Power Management Custom configuration
  ********************************/
  CpuPowerMgmtCustomConfig->CustomRatioTable.Cpuid = (UINT16) ((GetCpuFamily() | GetCpuStepping()) & (0x0FFF));
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPowerMgmtPsysConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_POWER_MGMT_PSYS_CONFIG  *CpuPowerMgmtPsysConfig;
  CPU_SKU                   CpuSku;

  CpuSku = GetCpuSku();
  CpuPowerMgmtPsysConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuPowerMgmtPsysConfig->Header.GuidHob.Name = %g\n", &CpuPowerMgmtPsysConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPowerMgmtPsysConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPowerMgmtPsysConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU Power Management Psys configuration
  ********************************/
  //
  // Identify whether SKU has EDRAM support
  //

  //
  // Update Psys Pmax on psys enabled skus
  //
  switch(CpuSku) {
     case EnumCpuUlx:
     case EnumCpuUlt:
      //
      //  Update for Cannonlake Silicon
      //
    default:
        break;
   }
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuTestConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_TEST_CONFIG  *CpuTestConfig;
  CPU_SKU          CpuSku;
  CpuTestConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuTestConfig->Header.GuidHob.Name = %g\n", &CpuTestConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuTestConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuTestConfig->Header.GuidHob.Header.HobLength));

  CpuSku = GetCpuSku();
  /********************************
    CPU Test configuration
  ********************************/

  CpuTestConfig->MlcStreamerPrefetcher    = CPU_FEATURE_ENABLE;
  CpuTestConfig->MlcSpatialPrefetcher     = CPU_FEATURE_ENABLE;
  CpuTestConfig->MonitorMwaitEnable       = CPU_FEATURE_ENABLE;
  CpuTestConfig->MachineCheckEnable       = CPU_FEATURE_ENABLE;
  CpuTestConfig->DebugInterfaceLockEnable = CPU_FEATURE_ENABLE;

  if ((CpuSku == EnumCpuUlx) || (CpuSku == EnumCpuUlt)){
    /**
    This policy should be used to enable or disable Voltage Optimization feature. Recommended defaults:
     Enable  - For Mobile SKUs(U/Y)
     Disable - Rest of all SKUs other than Mobile.
    **/
    CpuTestConfig->VoltageOptimization      = CPU_FEATURE_ENABLE;
  }
  else {
    CpuTestConfig->VoltageOptimization      = CPU_FEATURE_DISABLE;
  }
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPidTestConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_PID_TEST_CONFIG  *CpuPidTestConfig;
  CpuPidTestConfig = ConfigBlockPointer;

  DEBUG ((DEBUG_INFO, "CpuPidTestConfig->Header.GuidHob.Name = %g\n", &CpuPidTestConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPidTestConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPidTestConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU PID Test configuration
  ********************************/
}

/**
  Load Config block default

  @param[in] ConfigBlockPointer         Pointer to config block
**/
VOID
LoadCpuPowerMgmtTestConfigDefault (
  IN VOID          *ConfigBlockPointer
  )
{
  CPU_POWER_MGMT_TEST_CONFIG  *CpuPowerMgmtTestConfig;
  CpuPowerMgmtTestConfig = ConfigBlockPointer;
  CPU_GENERATION  CpuGeneration;
#ifdef CPU_CFL
  UINT16      CpuDid;
  CpuDid    = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));
#endif

  DEBUG ((DEBUG_INFO, "CpuPowerMgmtTestConfig->Header.GuidHob.Name = %g\n", &CpuPowerMgmtTestConfig->Header.GuidHob.Name));
  DEBUG ((DEBUG_INFO, "CpuPowerMgmtTestConfig->Header.GuidHob.Header.HobLength = 0x%x\n", CpuPowerMgmtTestConfig->Header.GuidHob.Header.HobLength));

  /********************************
    CPU Power Management Test configuration
  ********************************/
  CpuPowerMgmtTestConfig->Eist                          = TRUE;
  CpuPowerMgmtTestConfig->EnergyEfficientPState         = TRUE;
  CpuPowerMgmtTestConfig->EnergyEfficientTurbo          = TRUE;
#ifdef CPU_CFL
  if ((CpuDid == V_SA_DEVICE_ID_CFL_DT_1) || (CpuDid == V_SA_DEVICE_ID_CFL_DT_2)
     || (CpuDid == V_SA_DEVICE_ID_CFL_DT_3) || (CpuDid == V_SA_DEVICE_ID_CFL_DT_4)) {
    ///
    /// CFL-S 6+2, CFL S 8+2, CFl S 4+2, CFL S 2+2
    ///
    CpuPowerMgmtTestConfig->EnergyEfficientTurbo      = FALSE;
  }
#endif
  CpuPowerMgmtTestConfig->BiProcHot                     = TRUE;
  CpuPowerMgmtTestConfig->DisableProcHotOut             = TRUE;
  CpuPowerMgmtTestConfig->AutoThermalReporting          = TRUE;
  CpuPowerMgmtTestConfig->ThermalMonitor                = TRUE;
  CpuPowerMgmtTestConfig->Cx                            = TRUE;
  CpuPowerMgmtTestConfig->PmgCstCfgCtrlLock             = TRUE;
  CpuPowerMgmtTestConfig->C1e                           = TRUE;
  CpuPowerMgmtTestConfig->C1AutoDemotion                = TRUE;
  CpuPowerMgmtTestConfig->C1UnDemotion                  = TRUE;
  CpuGeneration = GetCpuGeneration();
  if(CpuGeneration == EnumCflCpu){
    CpuPowerMgmtTestConfig->C3AutoDemotion                = TRUE;
    CpuPowerMgmtTestConfig->C3UnDemotion                  = TRUE;
  }

  CpuPowerMgmtTestConfig->CStatePreWake                 = TRUE;
  CpuPowerMgmtTestConfig->RaceToHalt                    = TRUE;
#ifdef CPU_CFL
  CpuPowerMgmtTestConfig->CstateLatencyControl0TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl1TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl2TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl3TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl4TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl5TimeUnit = TimeUnit1024ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl0Irtl     = C3_LATENCY;
  CpuPowerMgmtTestConfig->CstateLatencyControl1Irtl     = C6_C7_SHORT_LATENCY;
  CpuPowerMgmtTestConfig->CstateLatencyControl2Irtl     = C6_C7_LONG_LATENCY;
  CpuPowerMgmtTestConfig->CstateLatencyControl3Irtl     = C8_LATENCY;
  CpuPowerMgmtTestConfig->CstateLatencyControl4Irtl     = C9_LATENCY;
  CpuPowerMgmtTestConfig->CstateLatencyControl5Irtl     = C10_LATENCY;
#else
  CpuPowerMgmtTestConfig->CstateLatencyControl1TimeUnit = TimeUnit32768ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl2TimeUnit = TimeUnit32768ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl3TimeUnit = TimeUnit32768ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl4TimeUnit = TimeUnit32768ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl5TimeUnit = TimeUnit32768ns;
  CpuPowerMgmtTestConfig->CstateLatencyControl1Irtl     = FIVE_MS_RESPONSE_TIME;
  CpuPowerMgmtTestConfig->CstateLatencyControl2Irtl     = FIVE_MS_RESPONSE_TIME;
  CpuPowerMgmtTestConfig->CstateLatencyControl3Irtl     = FIVE_MS_RESPONSE_TIME;
  CpuPowerMgmtTestConfig->CstateLatencyControl4Irtl     = FIVE_MS_RESPONSE_TIME;
  CpuPowerMgmtTestConfig->CstateLatencyControl5Irtl     = FIVE_MS_RESPONSE_TIME;
#endif

  CpuPowerMgmtTestConfig->PkgCStateLimit                = PkgAuto;
  CpuPowerMgmtTestConfig->CustomPowerUnit               = PowerUnit125MilliWatts;
#ifdef CPU_CFL
  CpuPowerMgmtTestConfig->PpmIrmSetting                 = PpmIrmPairFixedPriority;
#else
  CpuPowerMgmtTestConfig->PpmIrmSetting                 = PpmIrmFixedPriority;
#endif
}

static COMPONENT_BLOCK_ENTRY  mCpuIpBlocks [] = {
  {&gCpuConfigGuid,                     sizeof (CPU_CONFIG),                         CPU_CONFIG_REVISION,                        LoadCpuConfigDefault},
  {&gBiosGuardConfigGuid,               sizeof (BIOS_GUARD_CONFIG),                  BIOS_GUARD_CONFIG_REVISION,                 LoadBiosGuardConfigDefault},
  {&gCpuSgxConfigGuid,                  sizeof (CPU_SGX_CONFIG),                     CPU_SGX_CONFIG_REVISION,                    LoadCpuSgxConfigDefault},
  {&gCpuPowerMgmtBasicConfigGuid,       sizeof (CPU_POWER_MGMT_BASIC_CONFIG),        CPU_POWER_MGMT_BASIC_CONFIG_REVISION,       LoadCpuPowerMgmtBasicConfigDefault},
  {&gCpuPowerMgmtVrConfigGuid,          sizeof (CPU_POWER_MGMT_VR_CONFIG),           CPU_POWER_MGMT_VR_CONFIG_REVISION,          LoadCpuPowerMgmtVrConfigDefault},
  {&gCpuPowerMgmtCustomConfigGuid,      sizeof (CPU_POWER_MGMT_CUSTOM_CONFIG),       CPU_POWER_MGMT_CUSTOM_CONFIG_REVISION,      LoadCpuPowerMgmtCustomConfigDefault},
  {&gCpuPowerMgmtPsysConfigGuid,        sizeof (CPU_POWER_MGMT_PSYS_CONFIG),         CPU_POWER_MGMT_PSYS_CONFIG_REVISION,        LoadCpuPowerMgmtPsysConfigDefault},
  {&gCpuTestConfigGuid,                 sizeof (CPU_TEST_CONFIG),                    CPU_TEST_CONFIG_REVISION,                   LoadCpuTestConfigDefault},
  {&gCpuPidTestConfigGuid,              sizeof (CPU_PID_TEST_CONFIG),                CPU_PID_TEST_CONFIG_REVISION,               LoadCpuPidTestConfigDefault},
  {&gCpuPowerMgmtTestConfigGuid,        sizeof (CPU_POWER_MGMT_TEST_CONFIG),         CPU_POWER_MGMT_TEST_CONFIG_REVISION,        LoadCpuPowerMgmtTestConfigDefault},
};

/**
  Get CPU config block table total size.

  @retval Size of CPU config block table
**/
UINT16
EFIAPI
CpuGetConfigBlockTotalSize (
  VOID
  )
{
  return GetComponentConfigBlockTotalSize (&mCpuIpBlocks[0], sizeof (mCpuIpBlocks) / sizeof (COMPONENT_BLOCK_ENTRY));
}

/**
  CpuAddConfigBlocks add all Cpu config blocks.

  @param[in] ConfigBlockTableAddress    The pointer to add CPU config blocks

  @retval EFI_SUCCESS                   The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES          Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
CpuAddConfigBlocks (
  IN     VOID      *ConfigBlockTableAddress
  )
{
  EFI_STATUS Status;
  DEBUG((DEBUG_INFO, "CPU Post-Mem Entry \n"));
  PostCode (0xC00);

  Status = AddComponentConfigBlocks (ConfigBlockTableAddress, &mCpuIpBlocks[0], sizeof (mCpuIpBlocks) / sizeof (COMPONENT_BLOCK_ENTRY));
  DEBUG ((DEBUG_INFO, "CpuAddConfigBlocks Done \n"));
  PostCode (0xC09);

  return Status;
}

