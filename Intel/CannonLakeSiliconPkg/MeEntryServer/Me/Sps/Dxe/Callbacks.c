/**

Copyright (c) 2010 - 2017, Intel Corporation.

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

@file  Callbacks.c

  This file contains callbacks of events that SPS is interested in.
   
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
#include <Library/PciLib.h>
#include <Library/MmPciLib.h>
#include <Library/S3BootScriptLib.h>

#include <Private/Library/PmcPrivateLib.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include <Guid/GlobalVariable.h>

#include <Guid/HobList.h>
#include <Guid/EventLegacyBios.h>
#include <Guid/SpsInfoHobGuid.h>

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/CpuLib.h>
#include <Library/DxeMeLib.h>

#include <Protocol/CpuIo.h>
#include <Protocol/HeciProtocol.h>

#include <Protocol/PciIo.h>

#include <MeHeciRegs.h>
#include <Sps.h>
#include <PchAccess.h>

#include <MePolicyHob.h>

#include "SpsDxe.h"


/*****************************************************************************
 * Function prototypes
 *****************************************************************************/


/*****************************************************************************
 * Functions
 *****************************************************************************/
/**
    SPS callback for ExitPmAuth event.
    
    Performs actions required by SPS ME-BIOS Spec before OS is loaded.
    
    @param Event    - the event, not used
    @param pContext - pointer to SPS firmware description
    @retval Void.
**/
VOID SpsEndOfDxeCallback(
  IN EFI_EVENT      Event,
  IN  VOID         *pContext)
{
  EFI_STATUS        Status;
  UINT32            Data32[2];
  UINT8             Data8;
  UINT32            RspLen;
  HECI_PROTOCOL    *pHeci;
  SPS_DXE_CONTEXT  *pSpsContext = (SPS_DXE_CONTEXT*)pContext;
  
  union
  {
    MCTP_SET_BUS_OWNER_REQ     MctpSetBusOwnerReq;
    MCTP_SET_BUS_OWNER_RSP     MctpSetBusOwnerRsp;
  } HeciMsg;
  
  gBS->CloseEvent(Event);
  
  Status = gBS->LocateProtocol(&gHeciProtocolGuid, NULL, &pHeci);
  if (EFI_ERROR(Status))
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot locate HECI protocol (%r)\n", Status));
  }
  else
  {
    //
    // Send MCPT Bus Owner Proxy configuration if set
    //
    if (pSpsContext->MePeiPreMemConfig->SpsMctpBusOwner != 0 &&
        pSpsContext->SpsHob.FeatureSet.Bits.MctpProxy)
    {
      DEBUG((EFI_D_INFO, "[SPS] Sending MCTP_SET_BUS_OWNER with address 0x%X\n",
             pSpsContext->MePeiPreMemConfig->SpsMctpBusOwner));
      HeciMsg.MctpSetBusOwnerReq.Command = MCPT_CMD_SET_BUS_OWNER_REQ;
      HeciMsg.MctpSetBusOwnerReq.Reserved0[0] =
      HeciMsg.MctpSetBusOwnerReq.Reserved0[1] =
      HeciMsg.MctpSetBusOwnerReq.Reserved0[2] = 0;
      HeciMsg.MctpSetBusOwnerReq.PCIeAddress = (UINT16)(pSpsContext->MePeiPreMemConfig->SpsMctpBusOwner);
      HeciMsg.MctpSetBusOwnerReq.Location = 0;
      HeciMsg.MctpSetBusOwnerReq.Reserved1 = 0;
      RspLen = sizeof(HeciMsg.MctpSetBusOwnerRsp);
      Status = pHeci->SendwAck(HECI1_DEVICE, (UINT32*)&HeciMsg.MctpSetBusOwnerReq,
                               sizeof(HeciMsg.MctpSetBusOwnerReq), &RspLen,
                               SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MCTP);
      if (EFI_ERROR(Status))
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot send MCTP_SET_BUS_OWNER (%r)\n", Status));
      }
      else if (HeciMsg.MctpSetBusOwnerRsp.Result != MCPT_CMD_SET_BUS_OWNER_RSP_SUCCESS)
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: MCTP_SET_BUS_OWNER failure (cause: %d)\n",
               HeciMsg.MctpSetBusOwnerRsp.Result));
      }
    }
  } // if (HECI protocol located)
  
  if (pSpsContext->SpsHob.FeatureSet.Bits.NodeManager)
  {
    if (MeHeci2PciRead16(HECI_R_VID) == 0x8086) // Make sure HECI-2 is enabled
    {
      //
      // Configure HECI-2 for use in ACPI
      //
      MeHeci2PciWrite8(HECI_R_HIDM, HECI_HIDM_SCI);
      MeHeci2PciWrite8(HECI_R_CMD, MeHeci2PciRead8(HECI_R_CMD) | HECI_CMD_BME | HECI_CMD_MSE);

      DEBUG((EFI_D_INFO, "[SPS] Saving HECI-2 configuration to S3 boot script\n"));
      //
      // If HECI-2 (D22 F1) is in SCI mode there is no regular OS driver and we must save
      // its configuration in S3 resume script. S3BootScript library is using specific form of
      // PCI address, let's define macro for it.
      //
#define S3BOOTSCRIPT_PCIADDR(Bus, Dev, Fun, Reg) \
                    (UINT64)(((Bus) & 0xFF) << 24 | ((Dev) & 0x1F) << 16 | ((Fun) & 0x07) << 8 | ((Reg) & 0xFF ))
    
      S3BootScriptSavePciCfgWrite(S3BootScriptWidthUint8,
                                  S3BOOTSCRIPT_PCIADDR(ME_BUS, ME_DEV, ME_FUN_HECI2, HECI_R_HIDM),
                                  1, &Data8);
      Data32[0] = MeHeci2PciRead32(HECI_R_MBAR);
      Data32[1] = MeHeci2PciRead32(HECI_R_MBAR + 4);
      S3BootScriptSavePciCfgWrite(S3BootScriptWidthUint32,
                                  S3BOOTSCRIPT_PCIADDR(ME_BUS, ME_DEV, ME_FUN_HECI2, HECI_R_MBAR),
                                  2, &Data32[0]);
      Data8 = MeHeci2PciRead8(HECI_R_IRQ);
      S3BootScriptSavePciCfgWrite(S3BootScriptWidthUint8,
                                  S3BOOTSCRIPT_PCIADDR(ME_BUS, ME_DEV, ME_FUN_HECI2, HECI_R_IRQ),
                                  1, &Data8);
      Data8 = MeHeci2PciRead8(HECI_R_CMD);
      S3BootScriptSavePciCfgWrite(S3BootScriptWidthUint8,
                                  S3BOOTSCRIPT_PCIADDR(ME_BUS, ME_DEV, ME_FUN_HECI2, HECI_R_CMD),
                                  1, &Data8);
    }
    else
    {
      DEBUG((DEBUG_WARN, "[SPS] WARNING: HECI-2 was disabled for NM firmware running in ME\n"));
    }
  }
  return;
} // SpsEndOfDxeCallback()


/**
    SPS callback for ready-to-boot event.

    Performs actions required by SPS ME-BIOS Spec before OS is loaded.

    @param Event    - the event, not used
    @param pContext - pointer to SPS firmware description
    @retval Void.
**/
VOID SpsReadyToBootCallback(
  IN EFI_EVENT      Event,
  IN VOID          *pContext)
{
  EFI_STATUS        Status;
  UINT32            RspLen;
  HECI_PROTOCOL    *pHeci;
#ifdef SPS_TESTMENU_FLAG
  SPS_DXE_CONTEXT  *pSpsContext = (SPS_DXE_CONTEXT*)pContext;
#endif
  
  union
  {
    MKHI_MSG_END_OF_POST_REQ   EopReq;
    MKHI_MSG_END_OF_POST_RSP   EopRsp;
  } HeciMsg;
  
  gBS->CloseEvent(Event);
  
  Status = gBS->LocateProtocol(&gHeciProtocolGuid, NULL, &pHeci);
  if (EFI_ERROR (Status))
  {
    DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot locate HECI protocol (%r)\n", Status));
  }
  else
  {
#ifdef SPS_TESTMENU_FLAG
    //
    // Send END_OF_POST notification if not desibled in ME debug options
    //
    if (pSpsContext->MePeiPreMemConfig->EndOfPostMessage)
#endif
    {
      DEBUG((EFI_D_INFO, "[SPS] Sending END_OF_POST to ME\n"));
      
      HeciMsg.EopReq.Mkhi.Data = 0x00000CFF;
      RspLen = sizeof(HeciMsg.EopRsp);
      Status = pHeci->SendwAck(HECI1_DEVICE, (UINT32*)&HeciMsg.EopReq,
                               sizeof(HeciMsg.EopReq), &RspLen,
                               SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);
      if (EFI_ERROR(Status))
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: Cannot send END_OF_POST (%r)\n", Status));
      }
      else if (HeciMsg.EopRsp.Mkhi.Data != 0x00008CFF)
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: Invalid END_OF_POST response (MKHI: 0x%X)\n",
                            HeciMsg.EopRsp.Mkhi.Data));
        Status = EFI_PROTOCOL_ERROR;
      }
      else if (RspLen == sizeof(HeciMsg.EopRsp) && // if response contains Action and
               HeciMsg.EopRsp.Action == 1)         // global reset was requested
      {
        DEBUG((EFI_D_ERROR, "[SPS] ERROR: END_OF_POST response requests Global Reset!!!\n"));
        // TODO: Call PCH to perform GlobalReset
      }
      if (EFI_ERROR(Status))
      {
        MeReportError(MSG_EOP_ERROR);
        DisableAllMeDevices();
      }
    } // if (SystemCfgData.EopEnabled)
  } // if (HECI protocol located)
#ifdef SPS_TESTMENU_FLAG
  //
  // Lock global reset in PMIR regiser if not desibled in ME debug options
  //
  if (pSpsContext->MePeiPreMemConfig->MeGrLockEnabled)
#endif
  {
    DEBUG((EFI_D_INFO, "[SPS] Disabling Global Reset capability\n"));
    MmioAndThenOr32(PmcGetPwrmBase() + R_PMC_PWRM_ETR3,
                    (UINT32)~B_PMC_PWRM_ETR3_CF9GR, (UINT32)B_PMC_PWRM_ETR3_CF9LOCK);
  }
  return;
} // SpsReadyToBootCallback()


