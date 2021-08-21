/** @file
  Implementation file for Heci Message functionality

@copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2018 Intel Corporation.

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

@par Specification Reference:
**/

#include "MePeiLibInternals.h"
#include <Private/PchHsio.h>

//
// ME Client - MKHI
//

//
// MKHI_CBM_GROUP_ID = 0x00
//

/**
  Send Core BIOS Reset Request Message through HECI to reset the system.

  @param[in] ResetOrigin         Reset source
  @param[in] ResetType           Global or Host reset

  @retval EFI_UNSUPPORTED        Current ME mode doesn't support this function
  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_NOT_FOUND          No ME present
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT            HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciSendCbmResetRequest (
  IN  UINT8                      ResetOrigin,
  IN  UINT8                      ResetType
  )
{
  HECI_PPI                       *HeciPpi;
  EFI_STATUS                     Status;
  UINT32                         HeciLength;
  UINT32                         AckLength;
  CBM_RESET_REQ                  CbmResetRequest;
  UINT32                         MeMode;
  UINT32                         Result;

  DEBUG((DEBUG_INFO, "ME-BIOS: ME: Global Reset Request Entry.\n"));
  PostCode (0xE06);

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by HECI service not found.\n"));
    PostCode (0xEA6);
    return EFI_NOT_FOUND;
  }

  if (MeTypeIsSps ()) {
    DEBUG ((DEBUG_ERROR, "[SPS] Skip Global Reset Request for SPS firmware.\n"));
    PostCode (0xE86);
    return EFI_UNSUPPORTED;
  }

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by message is not allowed.\n"));
    PostCode (0xE86);
    return EFI_UNSUPPORTED;
  }

  CbmResetRequest.MkhiHeader.Data               = 0;
  CbmResetRequest.MkhiHeader.Fields.GroupId     = MKHI_CBM_GROUP_ID;
  CbmResetRequest.MkhiHeader.Fields.Command     = CBM_RESET_CMD;
  CbmResetRequest.Data.RequestOrigin            = ResetOrigin;
  CbmResetRequest.Data.ResetType                = ResetType;

  HeciLength = sizeof (CBM_RESET_REQ);
  AckLength = sizeof (CBM_RESET_ACK);

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &CbmResetRequest,
                      HeciLength,
                      &AckLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PeiHeciSendCbmResetRequest() - Unable to Send Reset Request - %r\n", Status));
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by message sent fail.\n"));
    PostCode (0xEC6);
    return Status;
  }

  Result = ((CBM_RESET_ACK*)&CbmResetRequest)->MkhiHeader.Fields.Result;
  if (Result != 0) {
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by message ack error. Result: %x\n", Result));
    Status = EFI_DEVICE_ERROR;
    PostCode (0xEE6);
  } else {
    DEBUG((DEBUG_INFO, "ME-BIOS: ME: Global Reset Request Exit - Success.\n"));
    PostCode (0xE26);
  }

  return Status;
}

//
// MKHI_FWCAPS_GROUP_ID = 0x03
//

/**
  Send Get Rule Data Request to CSME

  @param[in]   RuleId               Identifies the rule which data is requested.
  @param[out]  *RuleData            Pointer to requested rule data.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciFwCapsGetRuleData (
  IN  UINT32                      RuleId,
  OUT UINT32                      *RuleData
  )
{
  EFI_STATUS                      Status;
  GET_RULE_BUFFER                 GetRuleMsg;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PPI                        *HeciPpi;
  UINT32                          MeMode;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  GetRuleMsg.Request.MkhiHeader.Data               = 0;
  GetRuleMsg.Request.MkhiHeader.Fields.GroupId     = MKHI_FWCAPS_GROUP_ID;
  GetRuleMsg.Request.MkhiHeader.Fields.Command     = FWCAPS_GET_RULE_CMD;
  GetRuleMsg.Request.RuleId                        = RuleId;
  Length                                           = sizeof (GET_RULE);
  RecvLength                                       = sizeof (GET_RULE_ACK);

  ///
  /// Send Get Rule Data Request to ME
  ///
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &GetRuleMsg,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  if (!EFI_ERROR (Status) && ((GetRuleMsg.Response.MkhiHeader.Fields.Command) == FWCAPS_GET_RULE_CMD) &&
      ((GetRuleMsg.Response.MkhiHeader.Fields.IsResponse) == 1) &&
      (GetRuleMsg.Response.MkhiHeader.Fields.Result == 0)
      ) {
    *RuleData = GetRuleMsg.Response.RuleData;
  }

  return Status;
}

/**
  Send Get Firmware SKU Request to ME

  @param[out] FwCapsSku             ME Firmware Capability SKU

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwCapsSkuMsg (
  OUT MEFWCAPS_SKU             *RuleData
  )
{
  return PeiHeciFwCapsGetRuleData (FW_CAPS_RULE_ID, (UINT32*)RuleData);
}

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.FWInitComplete
  value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData              ME Firmware Capability SKU

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetFwFeatureStateMsg (
  OUT MEFWCAPS_SKU                *RuleData
  )
{
  return PeiHeciFwCapsGetRuleData (FW_FEATURE_STATE_RULE_ID, (UINT32*)RuleData);
}

/**
  This message is sent by the BIOS or IntelR MEBX. One of usages is to utilize
  this command to determine if the platform runs in Consumer or Corporate SKU
  size firmware.

  @param[out] RuleData              Including PlatformBrand, IntelMeFwImageType,
                                    SuperSku, and PlatformTargetUsageType

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA     *RuleData
  )
{
  return PeiHeciFwCapsGetRuleData (PLATFORM_TYPE_RULE_ID, (UINT32*)RuleData);
}

/**
  This message is sent by the BIOS in PEI phase to query
  ME Unconfig on RTC Clear Disable state.

  @param[out] RuleData            1 - Unconfig on RTC clear is disabled
                                  0 - Unconfig on RTC clear is enabled

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetUnconfigOnRtcClearDisableMsg (
  OUT UINT32                *RuleData
  )
{
  return PeiHeciFwCapsGetRuleData (UNCONFIG_ON_RTC_CLEAR_RULE_ID, RuleData);
}

/**
  Send Set Rule Data Request to CSME

  @param[in]   RuleId             Identifies the rule which data needs to be changed.
  @param[in]   RuleDataLength     Rule Data Length.
  @param[in]   RuleData           Pointer to new rule data.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciFwCapsSetRuleData (
  IN  UINT32                      RuleId,
  IN  UINT8                       RuleDataLength,
  IN  UINT32                      RuleData
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PPI                        *HeciPpi;
  SET_RULE_BUFFER                 SetRuleMsg;
  UINT32                          MeMode;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  SetRuleMsg.Request.MkhiHeader.Data           = 0;
  SetRuleMsg.Request.MkhiHeader.Fields.GroupId = MKHI_FWCAPS_GROUP_ID;
  SetRuleMsg.Request.MkhiHeader.Fields.Command = FWCAPS_SET_RULE_CMD;
  SetRuleMsg.Request.RuleId                    = RuleId;
  SetRuleMsg.Request.RuleDataLen               = RuleDataLength;
  SetRuleMsg.Request.RuleData                  = RuleData;
  Length                                       = sizeof (SET_RULE);
  RecvLength                                   = sizeof (SET_RULE_ACK);

  ///
  /// Send Set Rule Data Request to ME
  ///
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &SetRuleMsg,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  return Status;
}


/**
  This message is sent by the BIOS in PEI phase to set
  ME Unconfig on RTC Clear Disable state.

  @param[in] RuleData             1 - Disable Unconfig on RTC clear
                                  0 - Enable Unconfig on RTC clear

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function or EOP was sent
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciSetUnconfigOnRtcClearDisableMsg (
  IN UINT32                RuleData
  )
{
  return PeiHeciFwCapsSetRuleData (UNCONFIG_ON_RTC_CLEAR_RULE_ID, RULE_DATA_LENGTH, RuleData);
}

//
// BUP_COMMON_GROUP_ID = 0xF0
//

/**
  Send DRAM init done message through HECI to inform ME of memory initialization done.

  @param[in]  ImrBaseLow                Base address for IMR region (Low DWORD)
  @param[in]  ImrBaseHigh               Base address for IMR region (High DWORD)
  @param[in]  MemStatus                 Memory init status
  @param[out] MkhiResult                MKHI Error Code
  @param[out] Flags                     Flags
  @param[out] BiosAction                ME response to DID
  @param[out] PciImrBaseLow             Base address for PCI IMR (Low DWORD)
  @param[out] PciImrBaseHig             Base address for PCI IMR (High DWORD)
  @retval EFI_SUCCESS                   Command succeeded
  @retval EFI_DEVICE_ERROR              HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT                   HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciSendDid (
  IN  UINT32  ImrBaseLow,
  IN  UINT32  ImrBaseHigh,
  IN  UINT8   MemStatus,
  OUT UINT8   *MkhiResult,
  OUT UINT8   *Flags,
  OUT UINT8   *BiosAction,
  OUT UINT32  *PciImrBaseLow,
  OUT UINT32  *PciImrBaseHigh
  )
{
  EFI_STATUS                 Status;
  HECI_PPI                   *HeciPpi;
  DRAM_INIT_DONE_CMD_BUFFER  DidBuffer;
  UINT32                     ReqLength;
  UINT32                     RespLength;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  ZeroMem (&DidBuffer, sizeof(DRAM_INIT_DONE_CMD_BUFFER));
  DidBuffer.Request.MkhiHeader.Data                = 0;
  DidBuffer.Request.MkhiHeader.Fields.GroupId      = BUP_COMMON_GROUP_ID;
  DidBuffer.Request.MkhiHeader.Fields.Command      = DRAM_INIT_DONE_CMD;
  DidBuffer.Request.ImrData.BiosImrsBaLow          = ImrBaseLow;
  DidBuffer.Request.ImrData.BiosImrsBaHigh         = ImrBaseHigh;
  DidBuffer.Request.MemStatus                      = MemStatus;

  ReqLength  = sizeof (DRAM_INIT_DONE_CMD_REQ);
  RespLength = sizeof (DRAM_INIT_DONE_CMD_RESP);

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &DidBuffer,
                      ReqLength,
                      &RespLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (DidBuffer.Response.MkhiHeader.Fields.IsResponse == 0) {
    DEBUG ((DEBUG_ERROR, "HeciSend DRAM Init Done unsuccessful - no response\n"));
    return EFI_DEVICE_ERROR;
  }

  *MkhiResult     = (UINT8)DidBuffer.Response.MkhiHeader.Fields.Result;
  *Flags          = DidBuffer.Response.Flags;
  *BiosAction     = DidBuffer.Response.BiosAction;
  *PciImrBaseLow  = DidBuffer.Response.Pci2PrivBase.AdrLow;
  *PciImrBaseHigh = DidBuffer.Response.Pci2PrivBase.AdrHigh;

  if (*MkhiResult != MkhiStatusSuccess) {
    DEBUG ((DEBUG_WARN, "HeciSend DRAM Init Done processed with MKHI Error Code. Response: \n"));
    DEBUG ((DEBUG_WARN, "  MkhiResult = 0x%02x\n",  *MkhiResult));
  } else {
    DEBUG ((DEBUG_INFO, "HeciSend DRAM Init Done successful. Response:\n"));
  }
  DEBUG ((DEBUG_INFO, "  Pci2Priv Base Low = 0x%08x\n",  DidBuffer.Response.Pci2PrivBase.AdrLow));
  DEBUG ((DEBUG_INFO, "  Pci2Priv Base High = 0x%08x\n", DidBuffer.Response.Pci2PrivBase.AdrHigh));
  DEBUG ((DEBUG_INFO, "  Flags = 0x%02x\n",              DidBuffer.Response.Flags));
  DEBUG ((DEBUG_INFO, "  BIOS Action = 0x%02x\n",        DidBuffer.Response.BiosAction));

  return Status;
}

/**
  Send Get MBP from FW

  @param[in, out] MbpHeader         MBP header of the response
  @param[in, out] MbpItems          MBP items of the response
  @paran[in]      SkipMbp           Skip MBP

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciGetMbpMsg (
  IN OUT MBP_HEADER            *MbpHeader,
  IN OUT UINT32                *MbpItems,
  IN BOOLEAN                   SkipMbp
  )
{
  EFI_STATUS     Status;
  MBP_CMD_REQ    *MsgGetMbp;
  MBP_CMD_RESP   *MsgGetMbpAck;
  UINT8          MsgGetMbpAckBuffer[sizeof (MBP_CMD_RESP) + MAX_MBP_SIZE - 1];
  UINT32         Length;
  UINT32         RecvLength;
  HECI_PPI       *HeciPpi;

  ZeroMem (MsgGetMbpAckBuffer, sizeof (MsgGetMbpAckBuffer));

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  MsgGetMbp                                = (MBP_CMD_REQ*) &MsgGetMbpAckBuffer;
  MsgGetMbp->MkhiHeader.Data               = 0;
  MsgGetMbp->MkhiHeader.Fields.GroupId     = BUP_COMMON_GROUP_ID;
  MsgGetMbp->MkhiHeader.Fields.Command     = MBP_REQUEST_CMD;
  MsgGetMbp->SkipMbp                       = SkipMbp;
  Length                                   = sizeof (MBP_CMD_REQ);
  RecvLength                               = sizeof (MBP_CMD_RESP) + MAX_MBP_SIZE - 1;

  ///
  /// Send Get MBP Request to ME
  ///
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) MsgGetMbp,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  MsgGetMbpAck = (MBP_CMD_RESP*) MsgGetMbpAckBuffer;
  DEBUG ((DEBUG_INFO, "ReadMsg returned %r\n", Status));
  DEBUG ((DEBUG_INFO, "MsgGetMbpAck->MkhiHeader.Fields.Command = %d\n", MsgGetMbpAck->MkhiHeader.Fields.Command));
  DEBUG ((DEBUG_INFO, "MsgGetMbpAck->MkhiHeader.Fields.IsResponse = %d\n", MsgGetMbpAck->MkhiHeader.Fields.IsResponse));
  DEBUG ((DEBUG_INFO, "MsgGetMbpAck->MkhiHeader.Fields.Result = %d\n", MsgGetMbpAck->MkhiHeader.Fields.Result));

  if (!EFI_ERROR (Status) && !SkipMbp &&
      ((MsgGetMbpAck->MkhiHeader.Fields.Command) == MBP_REQUEST_CMD) &&
      ((MsgGetMbpAck->MkhiHeader.Fields.IsResponse) == 1) &&
      (MsgGetMbpAck->MkhiHeader.Fields.Result == 0)
      ) {
    CopyMem (MbpHeader, &MsgGetMbpAck->MbpHeader, sizeof (MBP_HEADER));
    CopyMem (MbpItems, &MsgGetMbpAck->MbpItems, RecvLength - sizeof (MBP_HEADER) - sizeof (MKHI_MESSAGE_HEADER));
  }

  return Status;
}

/**
  This message is sent by the BIOS to retrieve from CSME total size of IMRs.
  BIOS needs to provide mask of disabled IMRs and requested size for PCIe IMR.

  @param[in]  BiosImrDisableMask0  Low DWORD of BIOS IMR Disable mask
  @param[in]  BiosImrDisableMask1  High DWORD of BIOS IMR Disable mask
  @param[in]  PciImrSize           Requested IMR size for PCI
  @param[out] MkhiResult           MKHI Error Code
  @param[out] ImrsSize             Total IMR size
  @param[out] Alignment            Required address alignment

  @retval EFI_SUCCESS              Command succeeded
  @retval EFI_DEVICE_ERROR         HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT              HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciGetImrSizeMsg (
  IN  UINT32     BiosImrDisableMask0,
  IN  UINT32     BiosImrDisableMask1,
  IN  UINT32     PciImrSize,
  OUT UINT8      *MkhiResult,
  OUT UINT32     *ImrsSize,
  OUT UINT32     *Alignment
  )
{
  EFI_STATUS              Status;
  HECI_PPI                *HeciPpi;
  GET_IMR_CMD_BUFFER      GetImrCmd;
  UINT32                  ReqLength;
  UINT32                  RespLength;

  DEBUG ((DEBUG_INFO, "Heci Get IMR Size Msg\n"));

  *ImrsSize  = 0;
  *Alignment = 0;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  GetImrCmd.Request.MkhiHeader.Data           = 0;
  GetImrCmd.Request.MkhiHeader.Fields.GroupId = BUP_COMMON_GROUP_ID;
  GetImrCmd.Request.MkhiHeader.Fields.Command = GET_IMR_SIZE_CMD;

  GetImrCmd.Request.Data.BiosImrDisableMask0  = BiosImrDisableMask0;
  GetImrCmd.Request.Data.BiosImrDisableMask1  = BiosImrDisableMask1;
  GetImrCmd.Request.Data.Pci2PrivSize         = PciImrSize;

  ReqLength                                   = sizeof (GET_IMR_CMD_REQ);
  RespLength                                  = sizeof (GET_IMR_CMD_RESP);

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &GetImrCmd,
                      ReqLength,
                      &RespLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Heci Get IMR Size Msg Fail, Status = %r\n", Status));
    return Status;
  }

  if (GetImrCmd.Response.MkhiHeader.Fields.IsResponse == 0) {
    DEBUG ((DEBUG_ERROR, "Heci Get IMR Size Msg Fail - no response\n"));
    return EFI_DEVICE_ERROR;
  }

  *MkhiResult = (UINT8)GetImrCmd.Response.MkhiHeader.Fields.Result;
  *ImrsSize   = GetImrCmd.Response.Data.ImrsSize;
  *Alignment  = GetImrCmd.Response.Data.Alignment;

  if (*MkhiResult != MkhiStatusSuccess) {
    DEBUG ((DEBUG_WARN, "Heci Get IMR Size processed with MKHI Error Code. Response: \n"));
    DEBUG ((DEBUG_WARN, "  MkhiResult = 0x%02x\n", *MkhiResult));
  } else {
    DEBUG ((DEBUG_INFO, "Heci Get IMR Size Msg successful. Response:\n"));
  }

  DEBUG ((DEBUG_INFO, "  IMR Size = 0x%08x\n",          *ImrsSize));
  DEBUG ((DEBUG_INFO, "  Largest IMR Size = 0x%08x\n",  *Alignment));
  DEBUG ((DEBUG_INFO, "  Flags = 0x%08x\n",             GetImrCmd.Response.Data.Flags));


  return Status;
}

/**
  Send Manufacturing Reset and Halt

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciBupManufacturingResetAndHalt (
  VOID
  )
{
  EFI_STATUS                  Status;
  MANUF_RESET_AND_HALT_BUFFER ManufResetAndHalt;
  UINT32                      Length;
  UINT32                      RecvLength;
  HECI_PPI                    *HeciPpi;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  ManufResetAndHalt.Request.MkhiHeader.Data           = 0;
  ManufResetAndHalt.Request.MkhiHeader.Fields.GroupId = BUP_COMMON_GROUP_ID;
  ManufResetAndHalt.Request.MkhiHeader.Fields.Command = MANUF_RESET_AND_HALT_CMD;
  Length                                              = sizeof (MANUF_RESET_AND_HALT_REQ);
  RecvLength                                          = sizeof (MANUF_RESET_AND_HALT_RESP);

  ///
  /// Send Manufacturing Reset and Halt Request to ME
  ///
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &ManufResetAndHalt,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  if (ManufResetAndHalt.Response.MkhiHeader.Fields.Result != 0) {
    return EFI_DEVICE_ERROR;
  }

  return Status;
}


//
// BUP_ICC_GROUP_ID = 0xF1
//

/**
  Send ICC request through HECI to query if CSME FW requires the warm reset flow from a previous boot.

  @param[out] WarmResetRequired   1 - CSME requires a warm reset to complete BCLK ramp en flow

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciIccBclkMsg (
  OUT UINT8    *WarmResetRequired
  )
{
  EFI_STATUS        Status;
  HECI_PPI          *HeciPpi;
  ICC_CMD_BUFFER    IccCmdBuffer;
  UINT32            Length;
  UINT32            RespLength;

  DEBUG ((DEBUG_INFO, "(ICC) PeiHeciIccBclkMsg\n"));

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  IccCmdBuffer.Response.Data.FwNeedsWarmResetFlag = 0; //@Todo: remove this after CSME fixes response structure issue.
  IccCmdBuffer.Request.MkhiHeader.Data            = 0;
  IccCmdBuffer.Request.MkhiHeader.Fields.GroupId  = BUP_ICC_GROUP_ID;
  IccCmdBuffer.Request.MkhiHeader.Fields.Command  = ICC_CMD;
  Length                                          = sizeof (ICC_CMD_REQ);
  RespLength                                      = sizeof (ICC_CMD_RESP);

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &IccCmdBuffer,
                      Length,
                      &RespLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  if (!EFI_ERROR (Status) && (IccCmdBuffer.Response.MkhiHeader.Fields.Result == 0)) {
    *WarmResetRequired = (UINT8) IccCmdBuffer.Response.Data.FwNeedsWarmResetFlag;
  }

  return Status;
}

//
// BUP_HSIO_GROUP_ID = 0xF2
//

/**
  This function is deprecated. Send HSIO request through HECI to get the HSIO settings version on CSME side.

  @param[in]  BiosCmd              HSIO command: 0 - close interface, 1 - report HSIO version - deprecated
  @param[out] Crc                  CRC16 of ChipsetInit Table
  @param[out] Version              Version of ChipsetInit Table

  @retval EFI_SUCCESS              Command succeeded
  @retval EFI_DEVICE_ERROR         HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT              HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciHsioMsg (
  IN  UINT32                      BiosCmd,
  OUT UINT16                      *Crc,
  OUT UINT8                       *Version
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Send the required system ChipsetInit Table to ME FW.
  This function is deprecated. Maintained only for backward compatibility in CNL.

  @param[in] ChipsetInitTable     The required system ChipsetInit Table.
  @param[in] ChipsetInitTableLen  Length of the table in bytes
  @param[in] BypassPhySyncReset   Bypass the reset after PhySync

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciChipsetInitSyncMsg (
  IN  UINT8                       *ChipsetInitTable,
  IN  UINT32                      ChipsetInitTableLen,
  IN  BOOLEAN                     BypassPhySyncReset
  )
{
  return EFI_UNSUPPORTED;
}
//
// BUP_PM_GROUP_ID = 0xF3
//

/**
  Send Host Reset Notification Message to determine if warm reset is required.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
PeiHeciSendHostResetNotificationMsg (
  VOID
  )
{
  EFI_STATUS             Status;
  HECI_PPI               *HeciPpi;
  UINT32                 Length;
  UINT32                 RecvLength;
  HR_NOTIFICATION_BUFFER HrNotification;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  HrNotification.Request.MkhiHeader.Data           = 0;
  HrNotification.Request.MkhiHeader.Fields.GroupId = BUP_PM_GROUP_ID;
  HrNotification.Request.MkhiHeader.Fields.Command = NPCR_NOTIFICATION_CMD;
  Length                                           = sizeof (HR_NOTIFICATION_CMD);
  RecvLength                                       = sizeof (HR_NOTIFICATION_CMD_RESP);

  ///
  /// Send Host Reset Notification Request to ME
  ///
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) &HrNotification,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_MKHI_MESSAGE_ADDR
                      );

  if (!EFI_ERROR(Status) && (HrNotification.Response.ResetRequested == 0x1)) {
    (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetWarm, EFI_SUCCESS, 0, NULL);
  }

  return Status;
}


/**
  Send the required system ChipsetInit Table to ME FW.

  @param[in] ChipsetInitPtr       Pointer to the required system ChipsetInit Table
  @param[in] ChipsetInitTableLen  Length of the table in bytes

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_OUT_OF_RESOURCES    HECI Could not allocate Memory
**/
EFI_STATUS
PeiHeciWriteChipsetInitMsg (
  IN  UINT8                       *ChipsetInitPtr,
  IN  UINT32                      ChipsetInitTableLen
  )
{
  HECI_PPI                       *HeciPpi;
  EFI_STATUS                     Status;
  UINT32                         ReqSize;
  HSIO_WRITE_SETTINGS_REQ        *HsioWriteSettingsReqPtr;

  DEBUG ((DEBUG_INFO, "PeiHeciWriteChipsetInitMsg: Start\n"));
  if (ChipsetInitTableLen > PCH_HSIO_CHIPSETINIT_TBL_MAX_SIZE) {
    return EFI_DEVICE_ERROR;
  }

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  //
  // Setup the HECI message for a HSIO Write
  //
  ReqSize = sizeof (HSIO_WRITE_SETTINGS_REQ) + ChipsetInitTableLen;
  HsioWriteSettingsReqPtr = AllocateZeroPool (ReqSize);
  if (HsioWriteSettingsReqPtr == NULL) {
    DEBUG ((DEBUG_ERROR, "(HSIO) PeiHeciWriteChipsetInitMsg: Could not allocate Memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }
#ifdef UP_SERVER_FLAG
  HsioWriteSettingsReqPtr->Header.ApiVersion   = CANNONLAKE_H_PCH_PLATFORM;
#else // UP_SERVER_FLAG
  HsioWriteSettingsReqPtr->Header.ApiVersion   = CANNONLAKE_PCH_PLATFORM;
#endif //UP_SERVER_FLAG

  HsioWriteSettingsReqPtr->Header.IccCommand   = ICC_SET_HSIO_SETTINGS_CMD;
  HsioWriteSettingsReqPtr->Header.BufferLength = ReqSize - sizeof (ICC_HEADER);
  CopyMem (HsioWriteSettingsReqPtr + 1, ChipsetInitPtr, ChipsetInitTableLen);

  //
  // Send ChipsetInit Table to ME
  //
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) HsioWriteSettingsReqPtr,
                      ReqSize,
                      &ReqSize,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_ICC_MESSAGE_ADDR
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "(HSIO) ERROR: Write HSIO Settings Message failed! EFI_STATUS = %r\n", Status));
  } else if (HsioWriteSettingsReqPtr->Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "(HSIO) ERROR: Write HSIO Settings failed!: FW Response=0x%x\n", HsioWriteSettingsReqPtr->Header.IccResponse));
    Status = EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_INFO, "PeiHeciWriteChipsetInitMsg(): End\n"));
  return Status;
}

/**
  Read the ChipsetInit table from CSME

  @param[out]     ChipsetInitTablePtr  Pointer to the required system ChipsetInit Table.
  @param[in, out] ChipsetInitTableLen  Pointer to the length of the table in bytes

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciReadChipsetInitMsg (
  OUT UINT8               *ChipsetInitTablePtr,
  IN OUT UINT32           *ChipsetInitTableLen
  )
{
  HECI_PPI                       *HeciPpi;
  EFI_STATUS                     Status;
  UINT32                         Length;
  UINT32                         RecvLength;
  HSIO_READ_MPHY_BUFFER          *HsioReadMphy;


  if ((ChipsetInitTablePtr == NULL) || (ChipsetInitTableLen == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_INFO, "PeiHeciReadChipsetInitMsg(): Start\n"));

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return EFI_NOT_FOUND;
  }

  Length     = sizeof (HSIO_READ_MPHY_REQ);
  RecvLength = sizeof (HSIO_READ_MPHY_ACK) + *ChipsetInitTableLen;

  // ChipsetInitTablePtr allocated in PCH RC equivalent to RecvLength as PEI HOB size is limited
  // No need to create new buffer

  HsioReadMphy = AllocateZeroPool (RecvLength);
  if (HsioReadMphy == NULL) {
    DEBUG ((DEBUG_ERROR, "(HSIO) PeiHeciReadChipsetInitMsg: Could not allocate Memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }
#ifdef UP_SERVER_FLAG
  HsioReadMphy->Request.Header.ApiVersion   = CANNONLAKE_H_PCH_PLATFORM;
#else // UP_SERVER_FLAG
  HsioReadMphy->Request.Header.ApiVersion   = CANNONLAKE_PCH_PLATFORM;
#endif //UP_SERVER_FLAG
  HsioReadMphy->Request.Header.IccCommand   = ICC_READ_FROM_MPHY_CMD;
  HsioReadMphy->Request.Header.BufferLength = Length - sizeof (ICC_HEADER);
  HsioReadMphy->Request.Offset              = 0;
  HsioReadMphy->Request.NumBytesToRead      = *ChipsetInitTableLen;

  //
  // Get ChipsetInit Table from CSME
  //
  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) HsioReadMphy,
                      Length,
                      &RecvLength,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_ICC_MESSAGE_ADDR
                      );

  if (!EFI_ERROR (Status)) {
    if (HsioReadMphy->Response.Header.IccResponse == ICC_STATUS_SUCCESS) {
      if (HsioReadMphy->Response.NumBytesActuallyRead > PCH_HSIO_CHIPSETINIT_TBL_MAX_SIZE) {
        *ChipsetInitTableLen = 0;
        ASSERT (FALSE);  // ChipsetInit table should not get too large
        Status = EFI_DEVICE_ERROR;
      } else {
        if (*ChipsetInitTableLen < HsioReadMphy->Response.NumBytesActuallyRead) {
          *ChipsetInitTableLen = 0;
          ASSERT (FALSE);  // ChipsetInit buffer size is too small to copy full data
          Status = EFI_BUFFER_TOO_SMALL;
        } else {
          CopyMem (ChipsetInitTablePtr, HsioReadMphy->Response.Payload, HsioReadMphy->Response.NumBytesActuallyRead);
          *ChipsetInitTableLen = HsioReadMphy->Response.NumBytesActuallyRead;
        }
      }
    } else {
      *ChipsetInitTableLen = 0;
      DEBUG ((DEBUG_ERROR, "(HSIO) ERROR: Get HSIO Settings failed!: FW Response=0x%x\n", HsioReadMphy->Response.Header.IccResponse));
      Status = EFI_DEVICE_ERROR;
    }
  }

  DEBUG ((DEBUG_INFO, "PeiHeciReadChipsetInitMsg(): End\n"));
  return Status;
}

/**
  Retrieves information on whether Pcie root port configuration SoftStrap override is being done or not

  @param[in]      NumberOfControllers    On input, it is the number of controllers caller expects.

  @param[in, out] ControllerConfig       Pointer to the controller config message.

  @retval EFI_SUCCESS                   Command succeeded
  @retval EFI_DEVICE_ERROR              HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT                   HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL          Message Buffer is too small for the Acknowledge
  @retval EFI_UNSUPPORTED               Current ME mode doesn't support send this message through this HECI
  @retval EFI_OUT_OF_RESOURCES          Unable to allocate required resources
**/
EFI_STATUS
PeiHeciGetSoftStrpConfigMsg (
  IN      UINT16     NumberOfControllers,
  IN OUT  UINT8      *ControllerConfig
  )
{
  EFI_STATUS                             Status;
  UINT32                                 CommandSize;
  UINT32                                 ResponseSize;
  ICC_GET_SOFT_STRAP_CONFIG_CMD_BUFFER   *GetSoftStrapBuffer;
  HECI_PPI                               *HeciPpi;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "(ICC) PeiHeciGetSoftStrpConfigMsg Message\n"));
  ResponseSize         = NumberOfControllers * sizeof (UINT8) + sizeof (ICC_GET_SOFT_STRAP_CONFIG_CMD_RESP) + (4 * sizeof (UINT32));//(4 * sizeof (UINT32)) is needed for reserved field
  GetSoftStrapBuffer   = AllocateZeroPool (ResponseSize);
  if (GetSoftStrapBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CommandSize                                     = sizeof (ICC_GET_SOFT_STRAP_CONFIG_CMD_REQ);
#ifdef UP_SERVER_FLAG
  GetSoftStrapBuffer->Message.Header.ApiVersion   = CANNONLAKE_H_PCH_PLATFORM;
#else // UP_SERVER_FLAG
  GetSoftStrapBuffer->Message.Header.ApiVersion   = CANNONLAKE_PCH_PLATFORM;
#endif //UP_SERVER_FLAG
  GetSoftStrapBuffer->Message.Header.IccCommand   = ICC_GET_SOFT_STRAP_CONFIG_CMD;
  GetSoftStrapBuffer->Message.Header.BufferLength = CommandSize - sizeof (ICC_HEADER);

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *) GetSoftStrapBuffer,
                      CommandSize,
                      &ResponseSize,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_ICC_MESSAGE_ADDR
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Message failed! Status = %r\n", Status));
  } else if (GetSoftStrapBuffer->Response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Wrong response! IccHeader.Response = 0x%x\n", GetSoftStrapBuffer->Response.Header.IccResponse));
    Status = EFI_DEVICE_ERROR;
  } else if (GetSoftStrapBuffer->Response.NumOfControllers != NumberOfControllers) {
    DEBUG ((DEBUG_ERROR, "Controller number not match! Number Of Controllers = 0x%x\n", GetSoftStrapBuffer->Response.NumOfControllers));
    Status = EFI_DEVICE_ERROR;
  } else {
    CopyMem (ControllerConfig, GetSoftStrapBuffer->Response.ControllerConfig, GetSoftStrapBuffer->Response.NumOfControllers);
  }

  FreePool (GetSoftStrapBuffer);
  DEBUG ((DEBUG_INFO, "(ICC) PeiHeciGetSoftStrpConfigMsg Status = %r\n", Status));
  return Status;
}

/**
  Set Pcie Root port configuration SoftStrap override for the specified Pcie Root Port controller

  @param[in] NumberOfControllers         On input, it is the number of controllers caller expects.
  @param[in] ConfigIndex                 RpIndex where TG is present and softstraps need to be overridden, if Invalid number then clear the override
  @param[in] OverrideIndex               Config override index, Please refer to PCIE_CONTROLLER_SOFTSTRAP_OVERRIDE

  @retval EFI_SUCCESS                    Command succeeded
  @retval EFI_DEVICE_ERROR               HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT                    HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL           Message Buffer is too small for the Acknowledge
  @retval EFI_UNSUPPORTED                Current ME mode doesn't support send this message through this HECI
  @retval EFI_OUT_OF_RESOURCES           Unable to allocate required resources
**/
EFI_STATUS
PeiHeciOverrideSoftStrapMsg (
  IN UINT16                                NumberOfControllers,
  IN UINT8                                 ConfigIndex,
  IN PCIE_CONTROLLER_SOFTSTRAP_OVERRIDE    OverrideIndex
  )
{
  EFI_STATUS                                  Status;
  UINT32                                      CommandSize;
  UINT32                                      ResponseSize;
  ICC_OVERRIDE_PCIE_SOFT_STRAP_CMD_BUFFER     *OverrideSoftStrapBuffer;
  HECI_PPI                                    *HeciPpi;
  UINT8                                       *ConfigBuffer;

  Status = PeiServicesLocatePpi (
             &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "(ICC) PeiHeciOverrideSoftStrapMsg Message\n"));

  CommandSize = NumberOfControllers * sizeof (UINT8) + sizeof (ICC_OVERRIDE_SOFT_STRAP_CMD_REQ) + (4 * sizeof (UINT32));//(4 * sizeof (UINT32)) is needed for reserved field
  OverrideSoftStrapBuffer = AllocateZeroPool (CommandSize);
  if (OverrideSoftStrapBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ResponseSize = sizeof (ICC_OVERRIDE_SOFT_STRAP_CMD_RESP);
#ifdef UP_SERVER_FLAG
  OverrideSoftStrapBuffer->Message.Header.ApiVersion    = CANNONLAKE_H_PCH_PLATFORM;
#else // UP_SERVER_FLAG
  OverrideSoftStrapBuffer->Message.Header.ApiVersion    = CANNONLAKE_PCH_PLATFORM;
#endif //UP_SERVER_FLAG
  OverrideSoftStrapBuffer->Message.Header.IccCommand    = ICC_OVERRIDE_SOFT_STRAP_CMD;
  OverrideSoftStrapBuffer->Message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  OverrideSoftStrapBuffer->Message.NumOfControllers     = NumberOfControllers;
  ConfigBuffer                                          = OverrideSoftStrapBuffer->Message.ControllerConfig;
  ConfigBuffer [ConfigIndex]                            = OverrideIndex;

  Status = HeciPpi->SendwAck (
                      HECI1_DEVICE,
                      (UINT32 *)OverrideSoftStrapBuffer,
                      CommandSize,
                      &ResponseSize,
                      BIOS_FIXED_HOST_ADDR,
                      HECI_ICC_MESSAGE_ADDR
                      );

  if (EFI_ERROR (Status) || (OverrideSoftStrapBuffer->Response.Header.IccResponse != ICC_STATUS_SUCCESS)) {
    DEBUG ((DEBUG_ERROR, "(ICC) PeiHeciOverrideSoftStrapMsg: Message failed! Status = %r\n", Status));
    Status = EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_INFO, "(ICC) PeiHeciOverrideSoftStrapMsg: Message Status = %r\n", Status));
  FreePool (OverrideSoftStrapBuffer);
  return Status;
}

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Retrieves information on whether Pcie root port configuration SoftStrap override is being done or not

  @param[out] Response            Pointer to the response message

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support send this message through this HECI
**/
EFI_STATUS
PeiGetTgSsOvrdMsg (
  OUT     ICC_GET_PCIE_RPCFG_SS_OVRD_BUFFER      *Response
  )
{
  return EFI_UNSUPPORTED;
}

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Set Pcie Root port configuration SoftStrap override for the specified Pcie Root Port controller

  @param[in] RpIndex              RpIndex where TG is present and softstraps need to be overridden, if Invalid number then clear the override

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support send this message through this HECI
  @retval EFI_OUT_OF_RESOURCES    Unable to get resources.
**/
EFI_STATUS
PeiSetTgSsOvrdMsg (
  IN           UINT8             RpIndex
  )
{
  return EFI_UNSUPPORTED;
}
