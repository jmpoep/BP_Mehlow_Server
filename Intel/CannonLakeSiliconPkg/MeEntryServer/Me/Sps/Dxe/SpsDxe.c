/**

Copyright (c) 1996 - 2017, Intel Corporation.

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


@file SpsDxe.c

  This driver implements the DXE phase of SPS support as defined in
  SPS ME-BIOS Interface Specification.
  
**/
#include <Base.h>
#include <Uefi.h>
#include <PiDxe.h>

#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MmPciLib.h>
#include <Library/PciLib.h>

#include <PcieRegs.h>
#include <SaAccess.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include <Guid/EventGroup.h>
#include <Guid/GlobalVariable.h>

#include <Guid/HobList.h>
#include <Guid/EventLegacyBios.h>
#include <Guid/SpsInfoHobGuid.h>

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>

#include <MrcInterface.h>
#include <Protocol/MemInfo.h>
#include <Protocol/HeciProtocol.h>

#include <MeHeciRegs.h>
#include <Sps.h>

#include <MePolicyHob.h>

#include "SpsDxe.h"


/*****************************************************************************
 * Local definitions
 */


/*****************************************************************************
 * Local functions prototypes
 */


/******************************************************************************
 * Variables
 */
EFI_EVENT mSpsEndOfDxeEvent = NULL;
EFI_EVENT mSpsReadyToBootEvent = NULL;
SPS_DXE_CONTEXT *mpSpsContext = NULL;


/******************************************************************************
 * Functions
 */
/**
    TBD.
    @param pSpsMode - 
    @param pSystemTable - pointer to system table
    @param pHob - 
    @retval EFI status is returned.
**/
EFI_STATUS
SpsGetHob(
  IN  EFI_SYSTEM_TABLE *SystemTable,
  OUT SPS_INFO_HOB     *pSpsHob
  )
{
  EFI_HOB_GUID_TYPE    *pGuidHob;
  SPS_INFO_HOB         *pHobData;
  
  DEBUG((EFI_D_INFO, "[SPS] Looking for SPS HOB info from PEI\n"));
  
  pGuidHob = GetFirstGuidHob(&gSpsInfoHobGuid);
  if (pGuidHob == NULL)
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot locate SPS HOB\n"));
    return EFI_UNSUPPORTED;
  }
  pHobData = GET_GUID_HOB_DATA(pGuidHob);
  
  DEBUG((DEBUG_INFO, "[SPS] Building HOB: flow %d, MEFS %08X, ME-BIOS ver %d.%d, "
                     "features %02X, flow %d, boot mode %d, cores to disable %d\n",
         pHobData->WorkFlow, pHobData->Mefs1, pHobData->IfVerMajor, pHobData->IfVerMinor,
         pHobData->FeatureSet, pHobData->PwrOptBoot, pHobData->Cores2Disable));
  
  CopyMem(pSpsHob, pHobData, sizeof(*pSpsHob));
  
  return EFI_SUCCESS;
} // SpsGetHob()

VOID
SpsSendCupsConfiguration(
  VOID
  )
{
  EFI_STATUS            Status;
  UINT32                Index;
  UINT16                LinkSts;
  UINTN                 BaseAddress;
  UINT32                NumPorts = 0;
  MEM_INFO_PROTOCOL    *MemInfoHobProtocol;
  HECI_PROTOCOL        *pHeci;
  UINT32                ReqLen;
  UINT32                RspLen = 0;
  union
  {
    MKHI_MSG_MS_CUPS_CFG_REQ MsCupsCfgReq;
    MKHI_MSG_MS_CUPS_CFG_RSP MsCupsCfgRsp;
  } HeciMsg;

  DEBUG((EFI_D_INFO, "[SPS] CUPS enabled in SPS firmware\n"));

  Status = gBS->LocateProtocol (&gMemInfoProtocolGuid, NULL, &MemInfoHobProtocol);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot locate MemInfo protocol (status: %r)\n", Status));
  }
  else
  {
    gBS->SetMem(&HeciMsg, sizeof(HeciMsg), 0);

    // ratio value from MRC output base off how it's interpreted by memory controller. it's half of actual ratio
    // look into MrcGetCurrentMemoryFrequency(), MrcRatioToClock(), MrcRatioToFrequency()
    HeciMsg.MsCupsCfgReq.MemInfo.Ratio = MemInfoHobProtocol->MemInfoData.Ratio * 2;
    // value is plus 1 from MRC output in MemInfo protocol setup
    // look into SetupPlatformPolicies()
    HeciMsg.MsCupsCfgReq.MemInfo.R100Mhz = MemInfoHobProtocol->MemInfoData.RefClk == 2 ? 1 : 0;
    for (Index = 0; Index < DIMM_NUM; Index++)
    {
      HeciMsg.MsCupsCfgReq.MemInfo.C0Ranks |= (MemInfoHobProtocol->MemInfoData.RankInDimm[Index]) << (Index * RANK_BITS);
      HeciMsg.MsCupsCfgReq.MemInfo.C1Ranks |= (MemInfoHobProtocol->MemInfoData.RankInDimm[Index + CH_NUM]) << (Index * RANK_BITS);
    }
    for (Index = 0; Index < SA_PEG_MAX_FUN; Index++)
    {
      BaseAddress = MmPciBase(SA_PEG_BUS_NUM, SA_PEG_DEV_NUM, Index);
      if (MmioRead32 (BaseAddress) == 0xFFFFFFFF) {
        continue;
      }
      LinkSts = MmioRead16(BaseAddress + R_SA_PEG_LSTS_OFFSET);
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].LineRate = LinkSts & B_PCIE_LSTS_CLS;
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].LaneWidth = (LinkSts & B_PCIE_LSTS_NLW) >> N_PCIE_LSTS_NLW;
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].LinkActive = ((MmioRead32 (BaseAddress + R_SA_PEG_PEGSTS_OFFSET) >> 16) & 0xF) == 7 ? 1 : 0;
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].Fun = Index;
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].Dev = SA_PEG_DEV_NUM;
      HeciMsg.MsCupsCfgReq.PortInfo[NumPorts].Bus = SA_PEG_BUS_NUM;

      NumPorts++;
    }
    HeciMsg.MsCupsCfgReq.PortsNum = NumPorts;

    Status = gBS->LocateProtocol(&gHeciProtocolGuid, NULL, &pHeci);
    if (EFI_ERROR(Status))
    {
      DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot locate HECI protocol (status: %r)\n", Status));
    }
    else
    {
      DEBUG((DEBUG_INFO, "[SPS] Sending CUPS configuration request to ME\n"));
      DEBUG((DEBUG_INFO, "[SPS]   DDR freq: %d\n", MemInfoHobProtocol->MemInfoData.ddrFreq));
      DEBUG((DEBUG_INFO, "[SPS]   DimmRatio: %d", HeciMsg.MsCupsCfgReq.MemInfo.Ratio));
      DEBUG((DEBUG_INFO, ", DimmFreqType: %dMhz\n", HeciMsg.MsCupsCfgReq.MemInfo.R100Mhz == 1 ? 100 : 133));
      DEBUG((DEBUG_INFO, "[SPS]   RankInDimm Channel 0: %d\n", HeciMsg.MsCupsCfgReq.MemInfo.C0Ranks));
      DEBUG((DEBUG_INFO, "[SPS]   RankInDimm Channel 1: %d\n", HeciMsg.MsCupsCfgReq.MemInfo.C1Ranks));
      for (Index = 0; Index < NumPorts; Index++)
      {
        DEBUG((DEBUG_INFO, "[SPS]   PCIe Bus: %d, Dev: %d, Fun: %d, ",
               HeciMsg.MsCupsCfgReq.PortInfo[Index].Bus,
               HeciMsg.MsCupsCfgReq.PortInfo[Index].Dev,
               HeciMsg.MsCupsCfgReq.PortInfo[Index].Fun));
        DEBUG((DEBUG_INFO, "LineRate: %d, ", HeciMsg.MsCupsCfgReq.PortInfo[Index].LineRate));
        DEBUG((DEBUG_INFO, "LaneWidth: %d, ", HeciMsg.MsCupsCfgReq.PortInfo[Index].LaneWidth));
        DEBUG((DEBUG_INFO, "LinkActive: %d\n", HeciMsg.MsCupsCfgReq.PortInfo[Index].LinkActive));
      }

      HeciMsg.MsCupsCfgReq.Mkhi.Fields.GroupId = MKHI_GRP_MS;
      HeciMsg.MsCupsCfgReq.Mkhi.Fields.Command = MS_CMD_CUPSCFG;
      ReqLen = sizeof(HeciMsg.MsCupsCfgReq) - (UINT32)((sizeof(HeciMsg.MsCupsCfgReq.PortInfo) / IIO_SLOTS_MAX) * (SA_PEG_MAX_FUN - NumPorts));
      RspLen = sizeof(HeciMsg.MsCupsCfgRsp);
      Status = pHeci->SendwAck(HECI1_DEVICE, (UINT32 *)&HeciMsg.MsCupsCfgReq,
                                  ReqLen, &RspLen,
                                  SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);
      if (EFI_ERROR(Status))
      {
        DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot send CUPS configuration to ME (%r)\n", Status));
      }
      else if (HeciMsg.MsCupsCfgRsp.Mkhi.Fields.GroupId != MKHI_GRP_MS ||
        HeciMsg.MsCupsCfgRsp.Mkhi.Fields.Command != MS_CMD_CUPSCFG ||
        HeciMsg.MsCupsCfgRsp.Mkhi.Fields.IsResponse != 1)
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: Sending CUPS configuration to ME failed (wrong response MKHI: %08X)\n", HeciMsg.MsCupsCfgRsp.Mkhi.Data));
      }
      else if (HeciMsg.MsCupsCfgRsp.Mkhi.Fields.Result != 0)
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: Sending CUPS configuration to ME failed (status: %d)\n",
                            HeciMsg.MsCupsCfgRsp.Mkhi.Fields.Result));
      }
    }
  }
}

/**
    SPS DXE driver entry point.
    
    param ImageHandle - handle to SPS DXE image
    param pSystemTable - pointer to system table
    
    return Standard EFI_STATUS is returned.
**/
EFI_STATUS
SpsDxeEntryPoint(
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE  *pSystemTable)
{
  EFI_STATUS            Status;
  SPS_NMFS              Nmfs;
  EFI_HOB_GUID_TYPE               *GuidHob;
  
  
  Status = gBS->AllocatePool(EfiBootServicesData, sizeof(*mpSpsContext), (VOID**)&mpSpsContext);
  if (Status != EFI_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Memory allocation failed (%r)\n", Status));
    return Status;
  }
  SetMem(mpSpsContext, sizeof(*mpSpsContext), 0);

  GuidHob = GetFirstGuidHob (&gMePreMemPolicyHobGuid);
  if (GuidHob != NULL) {
    mpSpsContext->MePeiPreMemConfig = GET_GUID_HOB_DATA (GuidHob);
    if (mpSpsContext->MePeiPreMemConfig == NULL) {
      gBS->FreePool(mpSpsContext);
      DEBUG((EFI_D_ERROR, "[SPS] ERROR: Get policy from HOB failed (%r)\n", Status));
      ASSERT_EFI_ERROR(EFI_NOT_READY);
      return EFI_NOT_READY;
    }
  } else {
    gBS->FreePool(mpSpsContext);
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Get HOB failed (%r)\n", Status));
    ASSERT_EFI_ERROR(EFI_NOT_READY);
    return EFI_NOT_READY;
  }

  // This is test option which break RTC configuration
#if SPS_TESTMENU_FLAG
#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71
#define RTC_ADDRESS_REGISTER_A    10
#define RTC_BAD_VALUE             54
    if (mpSpsContext->MePeiPreMemConfig->BreakRtcEnable)
    {
      IoWrite8 (PCAT_RTC_ADDRESS_REGISTER, (UINT8) (RTC_ADDRESS_REGISTER_A | (UINT8) (IoRead8 (PCAT_RTC_ADDRESS_REGISTER) & 0x80)));
      IoWrite8 (PCAT_RTC_DATA_REGISTER, RTC_BAD_VALUE);
    }
#endif

  Status = SpsGetHob(pSystemTable, &mpSpsContext->SpsHob);
  if (EFI_ERROR(Status) || (mpSpsContext->SpsHob.WorkFlow == SpsFlowMeErr)) {
    gBS->FreePool(mpSpsContext);
    return Status;
  }
  if (GetBootModeHob() != BOOT_ON_S4_RESUME &&
      (mpSpsContext->MePeiPreMemConfig->NmPwrOptBoot != mpSpsContext->SpsHob.PwrOptBoot ||
       mpSpsContext->MePeiPreMemConfig->NmCores2Disable != mpSpsContext->SpsHob.Cores2Disable))
  {
    DEBUG((EFI_D_INFO, "[SPS] Updating boot mode %d->%d, cores to disable %d -> %d\n",
           mpSpsContext->MePeiPreMemConfig->NmPwrOptBoot, mpSpsContext->SpsHob.PwrOptBoot,
           mpSpsContext->MePeiPreMemConfig->NmCores2Disable, mpSpsContext->SpsHob.Cores2Disable));
    mpSpsContext->MePeiPreMemConfig->NmPwrOptBoot = mpSpsContext->SpsHob.PwrOptBoot;
    mpSpsContext->MePeiPreMemConfig->NmCores2Disable = mpSpsContext->SpsHob.Cores2Disable;
  }
  if (mpSpsContext->SpsHob.FeatureSet.Bits.NodeManager)
  {
    Nmfs.DWord = SpsHeciPciReadNmfs();
    if (Nmfs.DWord != 0xFFFFFFFF && Nmfs.Bits.NmEnabled)
    {
      DEBUG((EFI_D_INFO, "[SPS] NM enabled in SPS firmware\n"));
      GatherCPUInfoData(mpSpsContext);
    }
    else
    {
      DEBUG((DEBUG_ERROR, "[SPS] ERROR: NMFS not configured while NM enabled "
                          "(feature set: 0x%08X, feature set 2: 0x%08X, NMFS: 0x%08X)\n",
                          mpSpsContext->SpsHob.FeatureSet.Data.Set1,
                          mpSpsContext->SpsHob.FeatureSet.Data.Set2, Nmfs.DWord));
    }
  }

  if (mpSpsContext->SpsHob.FeatureSet.Bits.CUPS)
  {
    //
    // Not supported on Mehlow
    //
    //SpsSendCupsConfiguration();
  }

  Status = gBS->CreateEventEx(EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
                              SpsEndOfDxeCallback, mpSpsContext,
                              &gEfiEndOfDxeEventGroupGuid, &mSpsEndOfDxeEvent);
  ASSERT_EFI_ERROR(Status);
  Status = EfiCreateEventReadyToBootEx(TPL_CALLBACK, SpsReadyToBootCallback,
                                       mpSpsContext, &mSpsReadyToBootEvent);
  ASSERT_EFI_ERROR(Status);
    
  return EFI_SUCCESS;
} // SpsDxeEntryPoint()

