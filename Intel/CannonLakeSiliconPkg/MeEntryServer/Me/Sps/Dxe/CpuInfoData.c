/*++

Copyright (c) 2010-2018, Intel Corporation.

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


Module Name:

  CpuInfoData.c

Abstract:

   This implements filling out the HECI Message responsible of passing 
   CPU Info data. 

--*/
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

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include <Guid/GlobalVariable.h>

#include <Guid/HobList.h>
#include <Guid/EventLegacyBios.h>
#include <Guid/SpsInfoHobGuid.h>

#include <Protocol/MpService.h>

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/CpuLib.h>

#include <Protocol/CpuIo.h>
#include <Protocol/HeciProtocol.h>
#include <Protocol/PciIo.h>
#include <Protocol/CpuNvsArea.h>

#include <Sps.h>
#include <CpuRegs.h>
#include <CpuPowerMgmt.h>
#include <Private/CpuInitDataHob.h>
#include <Private/PowerMgmtNvsStruct.h>

#include <MePolicyHob.h>

#include "SpsDxe.h"


/*****************************************************************************
 * Local definitions
 *****************************************************************************/
#define MAX_TSTATES               7
#define MAX_TSTATES_FINE_GRAINED  15

#define CPUID_CORE_TOPOLOGY               0xB


/*****************************************************************************
 @brief
  Gather and send to ME host configuration data needed by Node Manager

  @param[in] pSpsCtx   sps cONTEXT

  @retval EFI_SUCCESS             MeMode copied
  @retval EFI_INVALID_PARAMETER   Pointer of MeMode is invalid
**/
EFI_STATUS 
GatherCPUInfoData(
  SPS_DXE_CONTEXT       *pSpsCtx)
{
  EFI_STATUS             Status;
  UINT32                 Reg;
  UINT32                 RspLen;
  UINT32                 Index;
  UINT64                 Msr;
  HECI_PROTOCOL         *pHeci;
  VOID                  *pCpuHob;
  CPU_INIT_DATA_HOB     *pCpuData;
  FVID_TABLE            *pFvidTable;
  CPU_NVS_AREA_PROTOCOL *pCpuNvs;
  union
  {
    MKHI_MSG_NM_HOST_CFG_REQ Req;
    MKHI_MSG_NM_HOST_CFG_RSP Rsp;
  } HeciMsg;

  Status = gBS->LocateProtocol(&gHeciProtocolGuid, NULL, &pHeci);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot locate HECI protocol (%r)\n", Status));
    return Status;
  }
  ///
  /// Get CPU Init Data Hob
  ///
  pCpuHob = GetFirstGuidHob(&gCpuInitDataHobGuid);
  if (pCpuHob == NULL)
  {
    DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot locat processor HOB\n"));
    return EFI_NOT_FOUND;
  }
  pCpuData = (CPU_INIT_DATA_HOB*)((UINTN)pCpuHob + sizeof(EFI_HOB_GUID_TYPE));
  pFvidTable = (FVID_TABLE*)(UINTN)pCpuData->FvidTable;
  ///
  /// Locate CPU Nvs Protocol.
  ///
  Status = gBS->LocateProtocol(&gCpuNvsAreaProtocolGuid, NULL, (VOID**)&pCpuNvs);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "[SPS] ERROR: Cannot locate gCpuNvsAreaProtocol (%r)", Status));
    return Status;
  }

  gBS->SetMem(&HeciMsg, sizeof(HeciMsg), 0);
  
  HeciMsg.Req.Mkhi.Data = (NM_CMD_HOSTCFG << 8) | MKHI_GRP_NM;
  
  HeciMsg.Req.Capabilities.Word = 0;
  HeciMsg.Req.Capabilities.Bits.MsgVer = MKHI_MSG_NM_HOST_CFG_VER;
  HeciMsg.Req.Capabilities.Bits.TurboEn = (pCpuNvs->Area->PpmFlags & PPM_TURBO) != 0;
#ifdef SPS_TESTMENU_FLAG
  HeciMsg.Req.Capabilities.Bits.PowerMsmt = (UINT16)(pSpsCtx->MePeiPreMemConfig->NmPowerMsmtOverride);
  if (pSpsCtx->MePeiPreMemConfig->NmHwChangeOverride) {
    HeciMsg.Req.Capabilities.Bits.HwChange = (UINT16)(pSpsCtx->MePeiPreMemConfig->NmHwChangeStatus);
  } else
#endif
  {
    HeciMsg.Req.Capabilities.Bits.HwChange =  (UINT16)(pSpsCtx->SpsHob.NmHwChangeStatus);
  }

  if (pCpuNvs->Area->PpmFlags & PPM_EIST)
  {
    if (pFvidTable[0].FvidHeader.EistStates > MAX_ACPI_PSTATES)
    {
      DEBUG((DEBUG_ERROR, "[SPS] WARNING: Number of FVID (%d) exceeds max ACPI P-states (%d) - truncated\n",
                          pFvidTable[0].FvidHeader.EistStates, MAX_ACPI_PSTATES));
      HeciMsg.Req.PStatesNumber = MAX_ACPI_PSTATES;
    }
    else
    {
      HeciMsg.Req.PStatesNumber = (UINT8)pFvidTable[0].FvidHeader.EistStates;
    }
    for (Index = 0; Index < HeciMsg.Req.PStatesNumber; Index++)
    {
      if (pFvidTable[0].FvidHeader.EistStates > MAX_ACPI_PSTATES)
        HeciMsg.Req.PStatesRatio[Index] = (UINT8)pFvidTable[Index + 1].FvidState.Limit16BusRatio; 
      else
        HeciMsg.Req.PStatesRatio[Index] = (UINT8)pFvidTable[Index + 1].FvidState.BusRatio; 
    }
  }
  if (pCpuNvs->Area->PpmFlags & PPM_TSTATES)
  {
    HeciMsg.Req.TStatesNumber = (pCpuNvs->Area->PpmFlags & PPM_TSTATE_FINE_GRAINED) ?
                                 MAX_TSTATES_FINE_GRAINED : MAX_TSTATES;
  }
  HeciMsg.Req.MaxPower = 0;
  HeciMsg.Req.MinPower = 0;
  
  //
  // Provide host processors information.
  // Number of processors is fixed at 1 for UP server BIOS.
  // Number of cores implemented can be found in CPU ID function 11 0 core topology.
  // Core topology subfunction 1 gives number possible logical threads, subfunction 0
  // number of threads per core. Must calculate cores by dividing threads by threads per core.
  // Number of enabled cores and threads can be found in CORE_THREAD_COUNT MSR (0x35).
  //
  HeciMsg.Req.ProcNumber = 1; // Fixed for UP server
  
  AsmCpuidEx(CPUID_CORE_TOPOLOGY, 1, NULL, &Reg, NULL, NULL);
  HeciMsg.Req.ProcCoresNumber = (UINT8)Reg;
  AsmCpuidEx(CPUID_CORE_TOPOLOGY, 0, NULL, &Reg, NULL, NULL);
  HeciMsg.Req.ProcCoresNumber /= (UINT8)Reg;
  
  Msr = AsmReadMsr64(MSR_CORE_THREAD_COUNT);
  HeciMsg.Req.ProcCoresEnabled = (UINT8)(Msr >> N_CORE_COUNT_OFFSET);
  HeciMsg.Req.ProcThreadsEnabled = (UINT8)(Msr & B_THREAD_COUNT_MASK);
  HeciMsg.Req.PlatformInfo = AsmReadMsr64(MSR_PLATFORM_INFO);
  HeciMsg.Req.Altitude = (INT16)pSpsCtx->MePeiPreMemConfig->SpsAltitude;
  
  DEBUG((EFI_D_INFO, "[SPS] Sending host configuration to NM\n"));
  DEBUG((EFI_D_INFO, "[SPS]   Capabilities : 0x%04x\n", HeciMsg.Req.Capabilities.Word));
  DEBUG((EFI_D_INFO, "[SPS]        TurboEn   : %d\n", HeciMsg.Req.Capabilities.Bits.TurboEn));
  DEBUG((EFI_D_INFO, "[SPS]        SmiOptim  : %d\n", HeciMsg.Req.Capabilities.Bits.SmiOptim));
  DEBUG((EFI_D_INFO, "[SPS]        PowerMsmt : %d\n", HeciMsg.Req.Capabilities.Bits.PowerMsmt));
  DEBUG((EFI_D_INFO, "[SPS]        HwChange  : %d\n", HeciMsg.Req.Capabilities.Bits.HwChange));
  DEBUG((EFI_D_INFO, "[SPS]        MsgVer    : %d\n", HeciMsg.Req.Capabilities.Bits.MsgVer));
  DEBUG((EFI_D_INFO, "[SPS]   P/T-states:    %d/%d\n",
                     HeciMsg.Req.PStatesNumber, HeciMsg.Req.TStatesNumber));
  DEBUG((EFI_D_INFO, "[SPS]   Min/max power: %d/%d\n",
                     HeciMsg.Req.MinPower, HeciMsg.Req.MaxPower));
  DEBUG((EFI_D_INFO, "[SPS]   Processor packages: %d\n", HeciMsg.Req.ProcNumber));
  DEBUG((EFI_D_INFO, "[SPS]   Processor cores:    %d\n", HeciMsg.Req.ProcCoresNumber));
  DEBUG((EFI_D_INFO, "[SPS]   Processor cores enabled:   %d\n", HeciMsg.Req.ProcCoresEnabled));
  DEBUG((EFI_D_INFO, "[SPS]   processor threads enabled: %d\n", HeciMsg.Req.ProcThreadsEnabled));
  DEBUG((EFI_D_INFO, "[SPS]   Platform info:     0x%08X%08X\n",
                     (UINT32)(HeciMsg.Req.PlatformInfo >> 32), (UINT32)HeciMsg.Req.PlatformInfo));
  DEBUG((EFI_D_INFO, "[SPS]   Altitude: %d\n", HeciMsg.Req.Altitude));
  DEBUG((EFI_D_INFO, "[SPS]   P-state ratios:"));
  for (Index = 0;
       Index < sizeof(HeciMsg.Req.PStatesRatio)/sizeof(HeciMsg.Req.PStatesRatio[0]); Index++)
  {
    DEBUG((EFI_D_INFO, " %02X", HeciMsg.Req.PStatesRatio[Index]));
  }
  DEBUG((EFI_D_INFO, "\n"));
  RspLen = sizeof(HeciMsg);
  Status = pHeci->SendwAck(HECI1_DEVICE, (UINT32*)&HeciMsg.Req, sizeof(HeciMsg.Req), &RspLen,
                           SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);
  if (EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot send host configration to NM (%r)\n", Status));
  }
  else if (HeciMsg.Rsp.Status != NM_STS_SUCCESS)
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Sending host configuration to NM failed (status: %d)\n",
                        HeciMsg.Rsp.Status));
    Status = EFI_PROTOCOL_ERROR;
  }
  return Status;
} // GatherCPUInfoData()

