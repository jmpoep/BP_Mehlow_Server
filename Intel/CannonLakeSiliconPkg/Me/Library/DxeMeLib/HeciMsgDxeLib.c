/** @file
  Implementation file for Heci Message functionality

@copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2019 Intel Corporation.

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

#include "MeDxeLibInternals.h"

#define ME_STATE_STALL_1_SECOND 1000000
#define MSG_MAX_WAIT_TIMEOUT    5

//
// Interface functions of HeciMsgLib
//

//
// ME Client - ISH Service
//

/**
  This message is sent by the BIOS when it wants to send
  PDT Unlock Message to ISH.

  @param[in] FileName             File name of bios2ish file.

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function or EOP was sent
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciPdtUnlockMsg (
  IN CONST CHAR8*               FileName
  )
{
  EFI_STATUS                    Status;
  UINT16                        Length;
  UINT32                        RecvLength;
  PDT_MESSAGE_BUFFER            PdtUnlock;
  UINT32                        MeMode;
  HECI_PROTOCOL                 *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Length                                         = sizeof (PDT_MESSAGE);
  RecvLength                                     = sizeof (PDT_MESSAGE_ACK);
  PdtUnlock.Request.Command                      = ISH_SET_FILE_CMD;
  PdtUnlock.Request.MessageLength                = Length - 4;
  PdtUnlock.Request.NumberOfElements             = 0x1;
  PdtUnlock.Request.Data.DataType                = PDT_UNLOCK_DATA_DATA_TYPE;
  PdtUnlock.Request.Data.PayloadSize             = PDT_UNLOCK_DATA_PAYLOAD_SIZE;

  ASSERT (AsciiStrLen (FileName) <= (sizeof (PdtUnlock.Request.FileName)));

  ZeroMem (&PdtUnlock.Request.FileName, sizeof (PdtUnlock.Request.FileName));
  CopyMem (&PdtUnlock.Request.FileName, FileName, AsciiStrLen (FileName));

  Status = Heci->SendwAck (
                 HECI1_DEVICE,
                 (UINT32 *) &PdtUnlock,
                 Length,
                 &RecvLength,
                 BIOS_FIXED_HOST_ADDR,
                 HECI_ISH_SERVICE_MESSAGE_ADDR
                 );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (PdtUnlock.Response.Status != BiosToIshStatusSuccess) {
    DEBUG ((DEBUG_ERROR, "BIOS2ISH Response Status: %x\n", PdtUnlock.Response.Status));
    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  This message is sent by the BIOS when it wants to send
  PDT Update Message

  @param[in] FileName             File name of bios2ish file.
  @param[in] PdtFileBuffer        Contains ISH PDT file
  @param[in] PdtBufferSize        Size of PDT file

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_INVALID_PARAMETER   Invalid input parameter
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function or EOP was sent
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_ABBORTED            Unable to allocate memory
**/
EFI_STATUS
HeciPdtUpdateMsg (
  IN CONST CHAR8*  FileName,
  IN UINT8         *PdtFileBuffer,
  IN UINT16        PdtBufferSize
  )
{
  EFI_STATUS                    Status;
  UINT16                        Length;
  UINT32                        RecvLength;
  UINT32                        MeMode;
  HECI_PROTOCOL                 *Heci;
  PDT_MESSAGE_BUFFER            *BiosToIshBuffer;

  if ((PdtFileBuffer == NULL) || (PdtBufferSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  RecvLength = sizeof (PDT_MESSAGE_ACK);
  Length     = sizeof (PDT_MESSAGE) + PdtBufferSize;
  //
  // Get 4 byte aligned Length
  //
  if (Length % 4) {
    Length = Length + (4 - (Length % 4));
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  BiosToIshBuffer   = (PDT_MESSAGE_BUFFER *) AllocateZeroPool ((UINTN) Length);
  if (BiosToIshBuffer == NULL) {
    DEBUG ((DEBUG_ERROR, "Could not allocate Memory\n"));
    return EFI_ABORTED;
  }
  BiosToIshBuffer->Request.Command            = ISH_SET_FILE_CMD;
  BiosToIshBuffer->Request.MessageLength      = Length - 4; // Command and MessageLength should not be counted here
  BiosToIshBuffer->Request.NumberOfElements   = 0x1;
  BiosToIshBuffer->Request.Data.DataType      = PDT_UPDATE_DATA_DATA_TYPE;
  BiosToIshBuffer->Request.Data.PayloadSize   = PdtBufferSize;

  ASSERT (AsciiStrLen (FileName) <= (sizeof (BiosToIshBuffer->Request.FileName)));

  ZeroMem (&BiosToIshBuffer->Request.FileName, sizeof (BiosToIshBuffer->Request.FileName));
  CopyMem (&BiosToIshBuffer->Request.FileName, FileName, AsciiStrLen (FileName));

  CopyMem((void *)((UINTN)BiosToIshBuffer + sizeof(PDT_MESSAGE)), PdtFileBuffer, (UINTN)PdtBufferSize);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) BiosToIshBuffer,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_ISH_SERVICE_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && (BiosToIshBuffer->Response.Status != BiosToIshStatusSuccess)) {
    DEBUG ((DEBUG_ERROR, "BIOS2ISH Response Status: %x\n", BiosToIshBuffer->Response.Status));
    Status = EFI_DEVICE_ERROR;
  }

  FreePool (BiosToIshBuffer);

  return Status;
}

//
// ME Client - MKHI
//

//
// MKHI_CBM_GROUP_ID = 0x00
//

/**
  Send Core BIOS Reset Request Message through HECI to reset the system.

  @param[in] ResetOrigin          Reset source
  @param[in] ResetType            Global or Host reset

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciSendCbmResetRequest (
  IN  UINT8                      ResetOrigin,
  IN  UINT8                      ResetType
  )
{
  HECI_PROTOCOL                  *Heci;
  EFI_STATUS                     Status;
  UINT32                         HeciLength;
  UINT32                         AckLength;
  CBM_RESET_REQ                  CbmResetRequest;
  PLATFORM_ME_HOOK_PROTOCOL      *PlatformMeHook;
  WDT_PROTOCOL                   *WdtProtocol;
  UINT32                         MeMode;
  UINT32                         Result;

  DEBUG ((DEBUG_INFO, "ME-BIOS: ME: Global Reset Request Entry.\n"));
  PostCode (0xE06);
  
 if (MeTypeIsSps ()) {
    DEBUG ((DEBUG_ERROR, "[SPS] Skip Global Reset Request for SPS firmware.\n"));
    PostCode (0xE86);
    return EFI_UNSUPPORTED;
  }

  if (MeIsAfterEndOfPost ()) {
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by message is not allowed.\n"));
    PostCode (0xE86);
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gPlatformMeHookProtocolGuid,
                  NULL,
                  (VOID **) &PlatformMeHook
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Unable to Locate PlatformMeHook Protocol for Global Reset Hook, so skip instead.- %r\n", Status));
  } else {
    PlatformMeHook->PreGlobalReset ();
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ME-BIOS: ME: Global Reset Request Exit - Error by HECI service not found.\n"));
    PostCode (0xEA6);
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
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
  HeciLength                                    = sizeof (CBM_RESET_REQ);
  AckLength                                     = sizeof (CBM_RESET_ACK);

  Status = gBS->LocateProtocol (&gWdtProtocolGuid, NULL, (VOID **) &WdtProtocol);
  ASSERT_EFI_ERROR (Status);
  WdtProtocol->AllowKnownReset ();

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &CbmResetRequest,
                   HeciLength,
                   &AckLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to Send Reset Request - %r\n", Status));
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
    DEBUG ((DEBUG_INFO, "ME-BIOS: ME: Global Reset Request Exit - Success.\n"));
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
HeciFwCapsGetRuleData (
  IN  UINT32                      RuleId,
  OUT UINT32                      *RuleData
  )
{
  EFI_STATUS                      Status;
  GET_RULE_BUFFER                 GetRuleMsg;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
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
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &GetRuleMsg,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && ((GetRuleMsg.Response.MkhiHeader.Fields.Command) == FWCAPS_GET_RULE_CMD) &&
      ((GetRuleMsg.Response.MkhiHeader.Fields.IsResponse) == 1) &&
      (GetRuleMsg.Response.MkhiHeader.Fields.Result == 0)) {
    *RuleData = GetRuleMsg.Response.RuleData;
  }

  return Status;
}

/**
  Send Get Firmware SKU Request to ME

  @param[out] *FwCapsSku            Firmware Capabilities

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwCapsSkuMsg (
  OUT MEFWCAPS_SKU         *RuleData
  )
{
  EFI_STATUS               Status;

  Status = HeciFwCapsGetRuleData (FW_CAPS_RULE_ID, (UINT32*)RuleData);

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to query the local firmware update interface status.

  @param[out] RuleData              1 - local firmware update interface enable
                                    0 - local firmware update interface disable

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetLocalFwUpdate (
  OUT UINT8                *RuleData
  )
{
  EFI_STATUS               Status;

  Status = HeciFwCapsGetRuleData (LOCAL_FW_UPDATE_RULE_ID, (UINT32*)RuleData);

  return Status;
}

/**
  Send Get User Capabilities State Request to ME.

  @param[out] MEFWCAPS_SKU       User Capabilities State

  @retval EFI_UNSUPPORTED        Current ME mode doesn't support this function
  @retval EFI_SUCCESS            Command succeeded
  @retval EFI_DEVICE_ERROR       HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT            HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL   Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetUserCapabilitiesState (
  OUT MEFWCAPS_SKU *RuleData
  )
{
  DEBUG ((DEBUG_INFO, "Getting User Capabilities State\n"));
  return HeciFwCapsGetRuleData (USER_CAPS_RULE_ID, (UINT32*)RuleData);
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get Ibex Peak platform type.
  One of usages is to utilize this command to determine if the platform runs in
  Consumer or Corporate SKU size firmware.

  @param[out] RuleData             PlatformBrand,
                                   IntelMeFwImageType,
                                   SuperSku,
                                   PlatformTargetUsageType

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA     *RuleData
  )
{
  EFI_STATUS                      Status;

  Status = HeciFwCapsGetRuleData (PLATFORM_TYPE_RULE_ID, (UINT32*)RuleData);

  return Status;
}

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.FWInitComplete
  value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] RuleData              MEFWCAPS_SKU message

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwFeatureStateMsg (
  OUT MEFWCAPS_SKU                *RuleData
  )
{
  EFI_STATUS                      Status;

  Status = HeciFwCapsGetRuleData (FW_FEATURE_STATE_RULE_ID, (UINT32*)RuleData);

  return Status;
}

/**
  The firmware will respond to GET OEM TAG message even after the End of Post (EOP).

  @param[in] RuleData             Default is zero. Tool can create the OEM specific OEM TAG data.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetOemTagMsg (
  OUT UINT32                      *RuleData
  )
{
  EFI_STATUS                      Status;

  Status = HeciFwCapsGetRuleData (OEM_TAG_RULE_ID, RuleData);

  return Status;
}

/**
  This message is sent by the BIOS when it wants to query
  ME Unconfig on RTC Clear Disable state.

  @param[out] RuleData            1 - Unconfig on RTC clear is disabled
                                  0 - Unconfig on RTC clear is not disabled

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetUnconfigOnRtcClearDisableMsg (
  OUT UINT32               *RuleData
  )
{
  return HeciFwCapsGetRuleData (UNCONFIG_ON_RTC_CLEAR_RULE_ID, RuleData);
}

/**
  This message is sent by the BIOS on the boot where the host wants to get the firmware provisioning state.
  The firmware will respond to AMT BIOS SYNC INFO message even after the End of Post.

  @param[out] RuleData            Bit [1:0] Reserved
                                  Bit [2]   Corporate provisioning is available.
                                  Bit [4:3] Provisioning State
                                    00 - Pre - Provisioning
                                    01 - In - Provisioning
                                    02 - Post - Provisioning
                                  Bit [31:5] Reserved

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciAmtBiosSyncInfo (
  OUT UINT32               *RuleData
  )
{
  return HeciFwCapsGetRuleData (AMT_BIOS_SYNC_INFO_RULE_ID, RuleData);
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
HeciFwCapsSetRuleData (
  IN  UINT32                      RuleId,
  IN  UINT8                       RuleDataLength,
  IN  UINT32                      RuleData
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PROTOCOL                   *Heci;
  SET_RULE_BUFFER                 SetRuleMsg;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
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
  Status = Heci->SendwAck (
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
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable or disable the local firmware update interface.
  The firmware allows a single update once it receives the enable command

  @param[in] RuleData             1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetLocalFwUpdate (
  IN UINT8                 RuleData
  )
{
  EFI_STATUS               Status;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = HeciFwCapsSetRuleData (LOCAL_FW_UPDATE_RULE_ID, RULE_DATA_LENGTH, (UINT32)RuleData);

  return Status;
}

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to disable the ME State. The firmware allows a single
  update once it receives the disable command Once firmware receives this message,
  the firmware will work in "Soft Temporary Disable" mode (HFS[19:16] = 3) after a
  global reset. Note, this message is not allowed when AT firmware is enrolled/configured.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetMeDisableMsg (
  VOID
  )
{
  EFI_STATUS                      Status;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (MeMode == ME_MODE_TEMP_DISABLED) {
    return EFI_SUCCESS;
  }

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = HeciFwCapsSetRuleData (ME_PLATFORM_STATUS_RULE_ID, RULE_DATA_LENGTH, ME_DISABLE);

  return Status;
}

/**
  This message is sent by the BIOS to inform ME FW whether or not to take the
  TPM 1.2 Deactivate flow

  @param[in] UINT8 TpmDeactivate  0 - ME FW should not take the
                                      deactivate flow.
                                  1 - ME FW should take the deactivate
                                      flow.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetAcmTpmData (
  IN UINT8                 TpmDeactivate
  )
{
  EFI_STATUS               Status;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = HeciFwCapsSetRuleData (ACM_TPM_DATA_RULE_ID, RULE_DATA_LENGTH, (UINT32)TpmDeactivate);

  return Status;
}


/**
  This message is sent by the BIOS when it wants to set
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
HeciSetUnconfigOnRtcClearDisableMsg (
  IN UINT32                RuleData
  )
{
  EFI_STATUS               Status;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = HeciFwCapsSetRuleData (UNCONFIG_ON_RTC_CLEAR_RULE_ID, RULE_DATA_LENGTH, RuleData);

  return Status;
}

//
// MKHI_HMRFPO_GROUP_ID = 0x05
//

/**
  Sends a message to ME to unlock a specified SPI Flash region for writing and receiving a response message.
  It is recommended that HMRFPO_ENABLE MEI message needs to be sent after all OROMs finish their initialization.

  @param[out] Result              HMRFPO_ENABLE response

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciHmrfpoEnable (
  OUT UINT8                       *Result
  )
{
  EFI_STATUS                      Status;
  HECI_PROTOCOL                   *Heci;
  HMRFPO_ENABLE_BUFFER            HmrfpoEnable;
  UINT32                          Length;
  UINT32                          RecvLength;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  ZeroMem (&HmrfpoEnable, sizeof (HMRFPO_ENABLE_BUFFER));
  HmrfpoEnable.Request.MkhiHeader.Data           = 0;
  HmrfpoEnable.Request.MkhiHeader.Fields.GroupId = MKHI_HMRFPO_GROUP_ID;
  HmrfpoEnable.Request.MkhiHeader.Fields.Command = HMRFPO_ENABLE_CMD_ID;
  Length                                         = sizeof (HMRFPO_ENABLE);
  RecvLength                                     = sizeof (HMRFPO_ENABLE_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &HmrfpoEnable,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (!EFI_ERROR (Status) && (HmrfpoEnable.Response.MkhiHeader.Fields.Result == 0)) {
    *Result = HmrfpoEnable.Response.Status;
  }

  DEBUG ((DEBUG_INFO, "Send HMRFPO_ENABLE_CMD_ID Result - %r\n", Status));
  return Status;
}

/**
  Sends a message to ME to lock a specified SPI Flash region for writing and receiving a response message.

  @param[out] Result              Status report

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_NOT_READY           HECI Device hasn't ready yet
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciHmrfpoLock (
  OUT UINT8                       *Result
  )
{
  EFI_STATUS                      Status;
  HECI_PROTOCOL                   *Heci;
  HMRFPO_LOCK_BUFFER              HmrfpoLock;
  UINT32                          Length;
  UINT32                          RecvLength;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  ZeroMem (&HmrfpoLock, sizeof (HMRFPO_LOCK_BUFFER));
  HmrfpoLock.Request.MkhiHeader.Data           = 0;
  HmrfpoLock.Request.MkhiHeader.Fields.GroupId = MKHI_HMRFPO_GROUP_ID;
  HmrfpoLock.Request.MkhiHeader.Fields.Command = HMRFPO_LOCK_CMD_ID;
  Length                                       = sizeof (HMRFPO_LOCK);
  RecvLength                                   = sizeof (HMRFPO_LOCK_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &HmrfpoLock,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && (HmrfpoLock.Response.MkhiHeader.Fields.Result == 0)) {
    *Result = HmrfpoLock.Response.Status;
  }

  DEBUG ((DEBUG_INFO, "Send HMRFPO_LOCK_CMD_ID Result - %r\n", Status));
  return Status;
}

/**
  System BIOS sends this message to get status for HMRFPO_LOCK message.

  @param[out] Result              HMRFPO_GET_STATUS response

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciHmrfpoGetStatus (
  OUT UINT8                       *Result
  )
{
  EFI_STATUS                      Status;
  HECI_PROTOCOL                   *Heci;
  HMRFPO_GET_STATUS_BUFFER        HmrfpoGetStatus;
  UINT32                          Length;
  UINT32                          RecvLength;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  ZeroMem (&HmrfpoGetStatus, sizeof (HMRFPO_GET_STATUS_BUFFER));
  HmrfpoGetStatus.Request.MkhiHeader.Data           = 0;
  HmrfpoGetStatus.Request.MkhiHeader.Fields.GroupId = MKHI_HMRFPO_GROUP_ID;
  HmrfpoGetStatus.Request.MkhiHeader.Fields.Command = HMRFPO_GET_STATUS_CMD_ID;
  Length                                            = sizeof (HMRFPO_GET_STATUS);
  RecvLength                                        = sizeof (HMRFPO_GET_STATUS_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &HmrfpoGetStatus,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && (HmrfpoGetStatus.Response.MkhiHeader.Fields.Result == 0)) {
    *Result = HmrfpoGetStatus.Response.Status;
  }

  DEBUG ((DEBUG_INFO, "Send HMRFPO_GET_STATUS_CMD_ID Result - %r\n", Status));
  return Status;
}

//
// MKHI_MCA_GROUP_ID = 0x0A
//

/**
  This message is sent by the BIOS to retrieve file stored in ME firmware NVM.

  @param[in]      FilePath        File path of file to read
  @param[in]      Offset          File offset
  @param[in, out] DataSize        On input - size of data to read, on output - size of read data
  @param[in]      Flags           Flags
  @param[out]     *Buffer         Pointer to the data buffer

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_ABORTED             Cannot allocate memory
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciReadFileMsg (
  IN CONST CHAR8 *FilePath,
  IN UINT32      Offset,
  IN OUT UINT32  *DataSize,
  IN UINT8       Flags,
  OUT UINT8      *DataBuffer
  )
{
  EFI_STATUS       Status;
  UINT32           Length;
  UINT32           RecvLength;
  READ_FILE_BUFFER *ReadFile;
  UINT32           MeMode;
  HECI_PROTOCOL    *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }


  Length     = sizeof (READ_FILE);
  RecvLength = sizeof (READ_FILE_ACK) + *DataSize;

  ReadFile = AllocateZeroPool (MAX(RecvLength,Length));

  if (ReadFile == NULL) {
    DEBUG ((DEBUG_ERROR, "HeciReadFileMsg Error: Could not allocate Memory\n"));
    return EFI_ABORTED;
  }

  ReadFile->Request.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  ReadFile->Request.MkhiHeader.Fields.Command = MCA_READ_FILE_CMD;
  ReadFile->Request.Offset                    = Offset;
  ReadFile->Request.DataSize                  = *DataSize;
  ReadFile->Request.Flags                     = Flags;

  ASSERT (AsciiStrLen (FilePath) <= (sizeof (ReadFile->Request.FilePath)));
  CopyMem (ReadFile->Request.FilePath, FilePath, AsciiStrLen (FilePath));

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) ReadFile,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (!EFI_ERROR (Status)) {
    if (ReadFile->Response.MkhiHeader.Fields.Command == MCA_READ_FILE_CMD &&
       (ReadFile->Response.MkhiHeader.Fields.IsResponse == 1) &&
       (ReadFile->Response.MkhiHeader.Fields.Result == MkhiStatusSuccess)) {
      CopyMem (DataBuffer, ReadFile->Response.Data, ReadFile->Response.DataSize);
    } else {
      Status = EFI_DEVICE_ERROR;
    }
  }

  FreePool (ReadFile);

  return Status;
}

/**
  Read Delayed Authentication Mode from ME NVM

  @param[out] *DamState           Delayed Authentication Mode State

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_ABORTED             Cannot allocate memory
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetDamState (
  OUT UINT8  *DamState
  )
{
  UINT32 DataSize;

  DataSize = MCA_DAM_FILE_SIZE;

  return HeciReadFileMsg ("/home/hotham/dbg_dam_req", 0, &DataSize, 0, DamState);
}

/**
  This message is sent by the BIOS to save data in a file stored in ME firmware NVM.

  @param[in]   FilePath           File path of file to write
  @param[in]   Offset             File offset
  @param[in]   DataSize           Size of data to write
  @param[in]   Flags              Flags
  @param[in]   *Buffer            Pointer to the data buffer

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_ABORTED             Cannot allocate memory
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetFileMsg (
  IN CONST CHAR8  *FilePath,
  IN UINT32       Offset,
  IN UINT32       DataSize,
  IN UINT8        Flags,
  IN UINT8        *DataBuffer
  )
{
  EFI_STATUS      Status;
  UINT32          Length;
  UINT32          RecvLength;
  SET_FILE_BUFFER *SetFile;
  UINT32          MeMode;
  HECI_PROTOCOL   *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Length     = sizeof (SET_FILE) + DataSize;
  RecvLength = sizeof (SET_FILE_ACK);

  SetFile = AllocateZeroPool (Length);
  if (SetFile == NULL) {
    DEBUG ((DEBUG_ERROR, "HeciSetFileMsg Error: Could not allocate Memory\n"));
    return EFI_ABORTED;
  }

  SetFile->Request.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  SetFile->Request.MkhiHeader.Fields.Command = MCA_SET_FILE_CMD;
  SetFile->Request.Offset                    = Offset;
  SetFile->Request.DataSize                  = DataSize;
  SetFile->Request.Flags                     = Flags;

  CopyMem (SetFile->Request.Data, DataBuffer, DataSize);

  ASSERT (AsciiStrLen (FilePath) <= (sizeof (SetFile->Request.FilePath)));
  CopyMem (SetFile->Request.FilePath, FilePath, AsciiStrLen (FilePath));

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) SetFile,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (SetFile->Response.MkhiHeader.Fields.Result != MkhiStatusSuccess) {
    Status = EFI_DEVICE_ERROR;
  }

  FreePool (SetFile);

  return Status;
}

/**
  Set Delayed Authentication Mode in ME NVM

  @param[in] DamState             New Delayed Authentication Mode State:
                                    0 - DAM Disable
                                    1 - DAM Enable

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_ABORTED             Cannot allocate memory
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetDamState (
  IN UINT8 DamState
  )
{
  EFI_STATUS Status;

  Status = HeciSetFileMsg ("/home/hotham/dbg_dam_req", 0, MCA_DAM_FILE_SIZE, 0, &DamState);
  if EFI_ERROR (Status) {
    return Status;
  }

  return HeciCommitFilesMsg ();
}

/**
  This message is sent by the BIOS after HeciSetFileMsg to notify ME firmware that it needs to
  commit previously set files. Caller shold invoke global reset afterwards.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciCommitFilesMsg (
  VOID
  )
{
  EFI_STATUS    Status;
  UINT32        Length;
  UINT32        RecvLength;
  COMMIT_FILES  CommitFiles;
  UINT32        MeMode;
  HECI_PROTOCOL *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  CommitFiles.MkhiHeader.Data           = 0;
  CommitFiles.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  CommitFiles.MkhiHeader.Fields.Command = MCA_COMMIT_FILES_CMD;
  Length                                = sizeof (COMMIT_FILES);
  RecvLength                            = sizeof (COMMIT_FILES);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &CommitFiles,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (CommitFiles.MkhiHeader.Fields.Result != MkhiStatusSuccess) {
    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}

/**
  This message is sent by the BIOS to notify ME firmware that the BIOS is starting
  to run 3rd party code.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function or EOP was sent
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciCoreBiosDoneMsg (
  VOID
  )
{
  EFI_STATUS                                Status;
  UINT32                                    Length;
  UINT32                                    RecvLength;
  CORE_BIOS_DONE                            CoreBiosDone;
  UINT32                                    MeMode;
  HECI_PROTOCOL                             *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  CoreBiosDone.MkhiHeader.Data           = 0;
  CoreBiosDone.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  CoreBiosDone.MkhiHeader.Fields.Command = MCA_CORE_BIOS_DONE_CMD;
  Length                                 = sizeof (CORE_BIOS_DONE);
  RecvLength                             = sizeof (CORE_BIOS_DONE_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &CoreBiosDone,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  return Status;
}

/**
  This command indicates to the FW that it shall commit ARBSVN to fuse.
  This command is initially (CNL PV) intended for committing main RBE SVN,
  but shall be available for committing individual ARBSVN value for modules such as PMC / OEM_KM / etc in future

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
**/
EFI_STATUS
HeciArbSvnCommitMsg (
  VOID
  )
{
  EFI_STATUS               Status;
  UINT32                   Length;
  UINT32                   RecvLength;
  ARB_SVN_COMMIT_BUFFER    ArbSvnCommit;
  UINT32                   MeMode;
  HECI_PROTOCOL            *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  ArbSvnCommit.Request.MkhiHeader.Data           = 0;
  ArbSvnCommit.Request.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  ArbSvnCommit.Request.MkhiHeader.Fields.Command = MCA_ARB_SVN_COMMIT_CMD;
  Length                                         = sizeof (ARB_SVN_COMMIT);
  RecvLength                                     = sizeof (ARB_SVN_COMMIT_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &ArbSvnCommit,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  return Status;
}

/**
  The command retrieves anti-replay SVN information.
  Caller can set Entries as 0 to get the correct number of entries CSME contains.

  @param[in, out] Entries         On input, it is the number of entries caller expects.
                                  On output, it indicates the number of entries CSME contains.
  @param[in, out] ArbSvnInfo      Anti-Rollback SVN Information

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate required resources
  @retval EFI_BUFFER_TOO_SMALL    The Entries is too small for the result
**/
EFI_STATUS
HeciArbSvnGetInfoMsg (
  IN OUT UINT32                *Entries,
  IN OUT ARB_SVN_INFO_ENTRY    *ArbSvnInfo
  )
{
  EFI_STATUS                Status;
  UINT32                    Length;
  UINT32                    RecvLength;
  ARB_SVN_GET_INFO_BUFFER   *ArbSvnGetInfo;
  UINT32                    MeMode;
  HECI_PROTOCOL             *Heci;
  UINT32                    NumberOfEntries;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  if (Entries == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  NumberOfEntries = *Entries;
  ArbSvnGetInfo   = AllocateZeroPool (NumberOfEntries * sizeof (ARB_SVN_INFO_ENTRY) + sizeof (ARB_SVN_GET_INFO_BUFFER));
  if (ArbSvnGetInfo == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ArbSvnGetInfo->Request.MkhiHeader.Fields.GroupId = MKHI_MCA_GROUP_ID;
  ArbSvnGetInfo->Request.MkhiHeader.Fields.Command = MCA_ARB_SVN_GET_INFO_CMD;
  Length                                           = sizeof (ARB_SVN_GET_INFO);
  RecvLength                                       = NumberOfEntries * sizeof (ARB_SVN_INFO_ENTRY) + sizeof (ARB_SVN_GET_INFO_BUFFER);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) ArbSvnGetInfo,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (EFI_ERROR (Status) && (Status == EFI_BUFFER_TOO_SMALL)) {
    *Entries = ArbSvnGetInfo->Response.NumEntries;
  }

  if (!EFI_ERROR (Status) &&
      ((ArbSvnGetInfo->Response.MkhiHeader.Fields.Command) == MCA_ARB_SVN_GET_INFO_CMD) &&
      ((ArbSvnGetInfo->Response.MkhiHeader.Fields.IsResponse) == 1) &&
      (ArbSvnGetInfo->Response.MkhiHeader.Fields.Result == 0)) {
    ASSERT (sizeof (ArbSvnInfo) <= (sizeof (ArbSvnGetInfo)));
    *Entries = ArbSvnGetInfo->Response.NumEntries;
    CopyMem (ArbSvnInfo, ArbSvnGetInfo->Response.ArbSvnEntry, ArbSvnGetInfo->Response.NumEntries * sizeof (ARB_SVN_INFO_ENTRY));
  }

  FreePool (ArbSvnGetInfo);
  return Status;
}

//
// BUP_COMMON_GROUP_ID = 0xF0
//

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to enable the ME State. The firmware allows a single
  update once it receives the enable command. Once firmware receives this message,
  the firmware will be in normal mode after a global reset.

  @retval EFI_SUCCESS             ME enabled message sent
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout or transition failed
                                  within allowed time limit
**/
EFI_STATUS
HeciSetMeEnableMsg (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PROTOCOL                   *Heci;
  ME_ENABLE_CMD_REQ               MeEnableReq;
  UINT32                          MeMode;
  UINT64                          HeciBaseAddress;
  HECI_FWS_REGISTER               MeFirmwareStatus;
  UINTN                           Timeout;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Heci->GetMeMode (&MeMode);
  if (MeMode == ME_MODE_NORMAL) {
    return EFI_SUCCESS;
  }

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  if (EFI_ERROR (Status) || (MeMode != ME_MODE_TEMP_DISABLED)) {
    return EFI_UNSUPPORTED;
  }

  MeEnableReq.MkhiHeader.Data               = 0;
  MeEnableReq.MkhiHeader.Fields.GroupId     = BUP_COMMON_GROUP_ID;
  MeEnableReq.MkhiHeader.Fields.Command     = ME_ENABLE_CMD;
  Length                                    = sizeof (ME_ENABLE_CMD_REQ);
  RecvLength                                = sizeof (ME_ENABLE_CMD_RESP);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &MeEnableReq,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && ((ME_ENABLE_CMD_RESP*)&MeEnableReq)->MkhiHeader.Fields.Result != 0) {
    //
    // Command succeeded but ME was not in disabled state
    //
    Status = EFI_UNSUPPORTED;
  }

  Timeout = 0;
  HeciBaseAddress = PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, 0);
  while (TRUE) {
    ///
    /// Read ME status and check for Current State
    ///
    MeFirmwareStatus.ul = PciSegmentRead32 (HeciBaseAddress + R_ME_HFS);
    if (MeFirmwareStatus.r.CurrentState == ME_STATE_NORMAL) {
      break;
    }

    MicroSecondDelay (ME_STATE_STALL_1_SECOND);
    Timeout++;

    if (Timeout >= MSG_MAX_WAIT_TIMEOUT) {
      DEBUG ((DEBUG_ERROR, "Timeout occured, MeFirmwareStatus: %08x.\n", MeFirmwareStatus.ul));
      return EFI_TIMEOUT;
    }
  }

  return Status;
}

/**
  Send HMRFPO disable MKHI to CSME.

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoDisable (
  VOID
  )
{
  EFI_STATUS                  Status;
  HECI_PROTOCOL               *Heci;
  HMRFPO_DISABLE_CMD_BUFFER   HmrfpoDisable;
  UINT32                      Length;
  UINT32                      RecvLength;
  UINT32                      MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  ZeroMem (&HmrfpoDisable, sizeof (HMRFPO_DISABLE_CMD_BUFFER));
  HmrfpoDisable.Request.MkhiHeader.Data           = 0;
  HmrfpoDisable.Request.MkhiHeader.Fields.GroupId = BUP_COMMON_GROUP_ID;
  HmrfpoDisable.Request.MkhiHeader.Fields.Command = HMRFPO_DISABLE_CMD;
  Length                                          = sizeof (HMRFPO_DISABLE_CMD_REQ);
  RecvLength                                      = sizeof (HMRFPO_DISABLE_CMD_RESP);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &HmrfpoDisable,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && (HmrfpoDisable.Response.MkhiHeader.Fields.Result != 0)) {
    //
    // Command succeeded but ME is in wrong state to process it
    //
    DEBUG ((DEBUG_ERROR, "HMRFPO_DISABLE ACK returns unsuccessful result - %r\n", Status));
    Status = EFI_UNSUPPORTED;
  }

  return Status;
}

//
// MKHI_GEN_GROUP_ID = 0xFF
//

/**
  Send Get Firmware Version Request to ME

  @param[in, out] MsgGenGetFwVersionAck   Return the message of FW version

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwVersionMsg (
  IN OUT GET_FW_VER_ACK_DATA      *GetFwVersionAckData
  )
{
  EFI_STATUS                      Status;
  GET_FW_VER_ACK                  GetFwVersion;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }
  ///
  /// Allocate MsgGenGetFwVersion data structure
  ///
  GetFwVersion.MkhiHeader.Data              = 0;
  GetFwVersion.MkhiHeader.Fields.GroupId    = MKHI_GEN_GROUP_ID;
  GetFwVersion.MkhiHeader.Fields.Command    = GEN_GET_FW_VERSION_CMD;
  Length                                    = sizeof (GET_FW_VER);
  RecvLength                                = sizeof (GET_FW_VER_ACK);
  ///
  /// Send Get Firmware Version Request to ME
  ///
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &GetFwVersion,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );
  if (!EFI_ERROR (Status) && (GetFwVersion.MkhiHeader.Fields.Command == GEN_GET_FW_VERSION_CMD) &&
      (GetFwVersion.MkhiHeader.Fields.IsResponse == 1) &&
      (GetFwVersion.MkhiHeader.Fields.Result == 0)) {
    *GetFwVersionAckData = GetFwVersion.Data;
  }
  return Status;
}

/**
  Get ME Unconfiguration Status.

  @param[in] UnconfigStatus       Unconfiguration Status:
                                    0x01 - Unconfiguration in progress
                                    0x02 - No unconfigure w/o password to be processed
                                    0x03 - CSME finished processing unconfigure w/o password
                                    0x80 - Error encountered by CSME during unconfigure w/o password

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
MeGetUnconfigureStatus (
  OUT UINT8* UnconfigStatus
  )
{
  GEN_GET_ME_UNCFG_STATE UnconfigureStatus;
  UINT32                 Length;
  EFI_STATUS             Status;
  HECI_PROTOCOL          *Heci;
  UINT32                 MeMode;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  UnconfigureStatus.MkhiHeader.Data           = 0;
  UnconfigureStatus.MkhiHeader.Fields.GroupId = MKHI_GEN_GROUP_ID;
  UnconfigureStatus.MkhiHeader.Fields.Command = GEN_GET_ME_UNCFG_STATE_CMD;
  Length                                      = sizeof (GEN_GET_ME_UNCFG_STATE);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32*)&UnconfigureStatus,
                   Length,
                   &Length,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (!EFI_ERROR (Status) && (UnconfigureStatus.MkhiHeader.Fields.Command == GEN_GET_ME_UNCFG_STATE_CMD) &&
      (UnconfigureStatus.MkhiHeader.Fields.IsResponse == 1)) {
    *UnconfigStatus = (UINT8) UnconfigureStatus.MkhiHeader.Fields.Result;
  }

  return Status;
}

/**
  Sends the MKHI Enable/Disable FW Features.

  @param[in] EnableBitMask        Bitmask of features to enable
  @param[in] DisableBitMask       Bitmask of features to disable

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
  @retval EFI_ACCESS_DENIED       Command request is rejected
**/
EFI_STATUS
HeciFwFeatureStateOverride (
  IN UINT32                       EnableBitmask,
  IN UINT32                       DisableBitmask
  )
{
  EFI_STATUS                                      Status;
  UINT32                                          Length;
  UINT32                                          RecvLength;
  UINT32                                          MeMode;
  FW_FEATURE_SHIPMENT_TIME_STATE_OVERRIDE_BUFFER  FwFeatureStateOverride;
  HECI_PROTOCOL                                   *Heci;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  FwFeatureStateOverride.Request.MkhiHeader.Data             = 0;
  FwFeatureStateOverride.Request.MkhiHeader.Fields.GroupId   = MKHI_GEN_GROUP_ID;
  FwFeatureStateOverride.Request.MkhiHeader.Fields.Command   = GEN_FW_FEATURE_STATE_OVERRIDE_CMD;
  FwFeatureStateOverride.Request.FeatureState.EnableFeature  = EnableBitmask;
  FwFeatureStateOverride.Request.FeatureState.DisableFeature = DisableBitmask;
  Length                                                     = sizeof (FW_FEATURE_SHIPMENT_TIME_STATE_OVERRIDE);
  RecvLength                                                 = sizeof (FW_FEATURE_SHIPMENT_TIME_STATE_OVERRIDE_ACK);

  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &FwFeatureStateOverride,
                   Length,
                   &RecvLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (!EFI_ERROR (Status) && (FwFeatureStateOverride.Response.Status == SetFeatureStateRejected)) {
    Status = EFI_ACCESS_DENIED;
  }

  return Status;
}

/**
  Send Set Manufacturing Me Reset and Halt Request to ME

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetManufacturingMeResetAndHalt (
  VOID
  )
{
  EFI_STATUS                      Status;
  SET_ME_RESET_HALT_ACK           SetMeResetAndHalt;
  UINT32                          HeciSendLength;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  UINT64                          HeciBaseAddress;
  HECI_FWS_REGISTER               MeFirmwareStatus;
  UINTN                           TimeOut;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  HeciBaseAddress = PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, 0);
  DEBUG ((DEBUG_INFO, "HeciSetManufacturingMeResetAndHalt: MeFirmwareStatus: %08x before messaging.\n", PciSegmentRead32 (HeciBaseAddress + R_ME_HFS)));

  ///
  /// Allocate MsgGenSetMeResetAndHalt data structure
  ///
  SetMeResetAndHalt.MkhiHeader.Data              = 0;
  SetMeResetAndHalt.MkhiHeader.Fields.GroupId    = MKHI_GEN_GROUP_ID;
  SetMeResetAndHalt.MkhiHeader.Fields.Command    = GEN_SET_ME_RESET_HALT_CMD;
  HeciSendLength                                 = sizeof (SET_ME_RESET_HALT);

  Status = Heci->SendMsg (
                   HECI1_DEVICE,
                   (UINT32 *) &SetMeResetAndHalt,
                   HeciSendLength,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_MKHI_MESSAGE_ADDR
                   );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  TimeOut = 0;
  while (TRUE) {
    ///
    /// Read ME status and check for operation mode
    ///
    MeFirmwareStatus.ul = PciSegmentRead32 (HeciBaseAddress + R_ME_HFS);
    if (MeFirmwareStatus.r.MeOperationMode == ME_OPERATION_MODE_SOFT_TEMP_DISABLE) {
      break;
    }

    MicroSecondDelay (ME_STATE_STALL_1_SECOND);
    TimeOut++;

    if (TimeOut >= MSG_MAX_WAIT_TIMEOUT) {
      DEBUG ((DEBUG_ERROR, "Timeout occured, MeFirmwareStatus: %08x.\n", MeFirmwareStatus.ul));
      return EFI_TIMEOUT;
    }
  }

  DEBUG ((DEBUG_INFO, "HeciSetManufacturingMeResetAndHalt succeeds.\n"));
  return EFI_SUCCESS;
}

//
// ME Client - Integrated Clock Chip (ICC)
//
/**
  Retrieves the number of currently used ICC clock profile

  @param[out] Profile             Number of current ICC clock profile
  @param[out] NumProfiles         Number of supported motherboard profiles
  @param[out] IccSelectAllowed    Allow ICC profile changeable
  @param[out] ProfileDescriptions Pointer of Profile Descriptions

  @retval EFI_UNSUPPORTED         ICC clock profile doesn't support
  @retval EFI_NOT_READY           Heci device hasn't ready yet
  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciGetIccProfile (
  OUT UINT8                       *Profile,
  OUT UINT8                       *NumProfiles,
  OUT UINT8                       *IccSelectAllowed,
  OUT ICC_PROFILE_DESC            *ProfileDescriptions
  )
{
  EFI_STATUS                      Status;
  ICC_GET_PROFILE_BUFFER          Buffer;
  UINT32                          CommandSize;
  UINT32                          ResponseSize;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  UINT32                          MeStatus;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || ((MeStatus & 0xF) != ME_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((DEBUG_INFO, "(ICC) GetIccProfile\n"));
  ///
  /// IccGetProfile response size will vary based on the amount of ICC profiles installed on the system.
  ///
  CommandSize                         = sizeof (ICC_GET_PROFILE_MESSAGE);
  ResponseSize                        = sizeof (ICC_GET_PROFILE_RESPONSE);
  Buffer.Message.Header.ApiVersion    = CANNONLAKE_PCH_PLATFORM;
  Buffer.Message.Header.IccCommand    = ICC_GET_PROFILE_CMD;
  Buffer.Message.Header.IccResponse   = 0;
  Buffer.Message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.Message.Header.Reserved      = 0;

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F0);
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &Buffer,
                   CommandSize,
                   &ResponseSize,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_ICC_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "(ICC) GetIccProfile: Message failed! EFI_STATUS = %r\n", Status));
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F2);
    return Status;
  }

  if (Buffer.Response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (DEBUG_ERROR,
       "(ICC) GetIccProfile: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.Response.Header.IccResponse)
      );
    Status = EFI_DEVICE_ERROR;
  } else {
    DEBUG ((DEBUG_INFO, "(ICC) GetIccProfile: Current profile = 0x%x\n", Buffer.Response.IccProfileIndex));
  }

  ///
  /// Copy ICC Profile information
  ///
  if (Profile != NULL) {
    *Profile = Buffer.Response.IccProfileIndex;
  }

  if (NumProfiles != NULL) {
    *NumProfiles = Buffer.Response.IccNumOfProfiles;
  }

    if (IccSelectAllowed != NULL) {
      *IccSelectAllowed = Buffer.Response.OemParams.Fields.IccRuntimeSelect;
    }

  if (ProfileDescriptions != NULL) {
    CopyMem (ProfileDescriptions, Buffer.Response.IccProfileDesc, sizeof (ICC_PROFILE_DESC)*MAX_NUM_ICC_PROFILES);
  }

  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F1);

  return Status;
}

/**
  Sets ICC clock profile to be used on next and following boots

  @param[in] Profile              Number of profile to be used

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_DEVICE_ERROR        Wrong response
  @retval EFI_NOT_READY           Heci device hasn't ready yet
  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciSetIccProfile (
  IN UINT8                        Profile
  )
{
  EFI_STATUS                      Status;
  ICC_SET_PROFILE_BUFFER          Buffer;
  UINT32                          CommandSize;
  UINT32                          ResponseSize;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  UINT32                          MeStatus;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || ((MeStatus & 0xF) != ME_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((DEBUG_INFO, "(ICC) SetIccProfile\n"));

  CommandSize                         = sizeof (ICC_SET_PROFILE_MESSAGE);
  ResponseSize                        = sizeof (ICC_SET_PROFILE_RESPONSE);
  Buffer.Message.Header.ApiVersion    = CANNONLAKE_PCH_PLATFORM;
  Buffer.Message.Header.IccCommand    = ICC_SET_PROFILE_CMD;
  Buffer.Message.Header.IccResponse   = 0;
  Buffer.Message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.Message.Header.Reserved      = 0;
  Buffer.Message.BiosIccProfile       = Profile;
  Buffer.Message.PaddingA             = 0;
  Buffer.Message.PaddingB             = 0;

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8100);
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &Buffer,
                   CommandSize,
                   &ResponseSize,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_ICC_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "(ICC) SetIccProfile: Message failed! EFI_STATUS = %r\n", Status));
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8102);
    return Status;
  }

  if (Buffer.Response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (DEBUG_ERROR,
       "(ICC) SetIccProfile: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.Response.Header.IccResponse)
      );
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8103);
    return EFI_DEVICE_ERROR;
  }
  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8101);

  return Status;
}

/**
  Retrieves the number of currently used ICC clock profile

  @param[out] Profile             number of current ICC clock profile

  @retval EFI_UNSUPPORTED         ICC clock profile doesn't support
  @retval EFI_NOT_READY           Heci device hasn't ready yet
  @retval EFI_INVALID_PARAMETER   IccClockSettings is a NULL pointer
  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciGetIccClockSettings (
  OUT ICC_GETSET_CLOCK_SETTINGS_REQRSP  *IccClockSettings
  )
{
  EFI_STATUS                      Status;
  ICC_GET_CLOCK_SETTINGS_BUFFER   Buffer;
  UINT32                          CommandSize;
  UINT32                          ResponseSize;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  UINT32                          MeStatus;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  if (IccClockSettings == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || ((MeStatus & 0xF) != ME_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((DEBUG_INFO, "(ICC) HeciGetIccClockSettings\n"));
  CommandSize                         = sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP);
  ResponseSize                        = sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP);
  Buffer.Message.Header.ApiVersion    = CANNONLAKE_PCH_PLATFORM;
  Buffer.Message.Header.IccCommand    = ICC_GET_CLOCK_SETTINGS_CMD;
  Buffer.Message.Header.IccResponse   = 0;
  Buffer.Message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.Message.Header.Reserved      = 0;
  Buffer.Message.ReqClock             = IccClockSettings->ReqClock;
  Buffer.Message.SettingType          = IccClockSettings->SettingType;

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F0);
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &Buffer,
                   CommandSize,
                   &ResponseSize,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_ICC_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "(ICC) HeciGetIccClockSettings: Message failed! EFI_STATUS = %r\n", Status));
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F2);
    return Status;
  }

  if (Buffer.Response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG (
      (DEBUG_ERROR,
       "(ICC) HeciGetIccClockSettings: Wrong response! IccHeader.Response = 0x%x\n",
       Buffer.Response.Header.IccResponse)
      );
    Status = EFI_DEVICE_ERROR;
  } else {
    DEBUG ((DEBUG_INFO, "(ICC) HeciGetIccClockSettings: Current profile = 0x%x\n"));
  }

  if (IccClockSettings != NULL) {
    CopyMem (IccClockSettings, &Buffer.Response.Header, sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP));
  }

  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x80F1);

  return Status;
}

/**
  Sets ICC clock profile to be used on next and following boots

  @param[in] IccClockSettings     Pointer for requested ICC Clock Settings

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_DEVICE_ERROR        Wrong response
  @retval EFI_NOT_READY           Heci device hasn't ready yet
  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciSetIccClockSettings (
  IN ICC_GETSET_CLOCK_SETTINGS_REQRSP *IccClockSettings
  )
{
  EFI_STATUS                      Status;
  ICC_SET_CLOCK_SETTINGS_BUFFER   Buffer;
  UINT32                          CommandSize;
  UINT32                          ResponseSize;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  UINT32                          MeStatus;

  if (MeIsAfterEndOfPost ()) {
    return EFI_UNSUPPORTED;
  }

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  Status = Heci->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || ((MeStatus & 0xF) != ME_READY)) {
    return EFI_NOT_READY;
  }

  DEBUG ((DEBUG_INFO, "(ICC) HeciSetIccClockSettings\n"));

  CommandSize                         = sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP);
  ResponseSize                        = sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP);

  ///
  ///  Copy input clock settings data to Heci message clock settings buffer
  ///
  CopyMem (&Buffer.Message, IccClockSettings, sizeof (ICC_GETSET_CLOCK_SETTINGS_REQRSP));

  ///
  ///  Update HECI buffer header
  ///
  Buffer.Message.Header.ApiVersion    = CANNONLAKE_PCH_PLATFORM;
  Buffer.Message.Header.IccCommand    = ICC_SET_CLOCK_SETTINGS_CMD;
  Buffer.Message.Header.IccResponse   = 0;
  Buffer.Message.Header.BufferLength  = CommandSize - sizeof (ICC_HEADER);
  Buffer.Message.Header.Reserved      = 0;

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8100);
  Status = Heci->SendwAck (
                   HECI1_DEVICE,
                   (UINT32 *) &Buffer,
                   CommandSize,
                   &ResponseSize,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_ICC_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "(ICC) HeciSetIccClockSettings: Message failed! EFI_STATUS = %r\n", Status));
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8102);
    return Status;
  }

  if (Buffer.Response.Header.IccResponse != ICC_STATUS_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "(ICC) HeciSetIccClockSettings: Wrong response! IccHeader.Response = 0x%x\n", Buffer.Response.Header.IccResponse));
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8103);
    return EFI_DEVICE_ERROR;
  }
  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8101);

  return Status;
}

//
// ME Client - Hardware Asset
//

/**
  Send Hardware Asset Tables to Firmware

  @param[in] AssetTableData       Hardware Asset Table Data
  @param[in] TableDataSize        Size of Asset table

  @retval EFI_SUCCESS             Table sent
  @retval EFI_ABORTED             Could not allocate Memory
**/
EFI_STATUS
HeciAssetUpdateFwMsg (
  IN TABLE_PUSH_DATA              *AssetTableData,
  IN UINT16                       TableDataSize
  )
{
  AU_TABLE_PUSH_MSG *SendAssetTableDataMsg;
  EFI_STATUS        Status;
  HECI_PROTOCOL     *Heci;
  UINT32            Length;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Subtract off single byte from TABLE_PUSH_DATA.TableData[1]
  ///
  SendAssetTableDataMsg = AllocateZeroPool (sizeof (AU_TABLE_PUSH_MSG) + MAX_ASSET_TABLE_ALLOCATED_SIZE - 1);
  if (SendAssetTableDataMsg == NULL) {
    DEBUG ((DEBUG_ERROR, "AssetUpdateFwMsg Error: Could not allocate Memory\n"));
    return EFI_ABORTED;
  }

  if (TableDataSize > MAX_ASSET_TABLE_ALLOCATED_SIZE) {
    TableDataSize = MAX_ASSET_TABLE_ALLOCATED_SIZE;
  }

  SendAssetTableDataMsg->Header.Data = 0;
  ///
  /// Subtract off single byte from TABLE_PUSH_DATA.TableData[1]
  ///
  SendAssetTableDataMsg->Header.Fields.MessageLength      = TableDataSize + sizeof (TABLE_PUSH_DATA) - 1;
  SendAssetTableDataMsg->Header.Fields.Command            = HWA_TABLE_PUSH_CMD;
  SendAssetTableDataMsg->Header.Fields.FRUTablePresent    = 1;
  SendAssetTableDataMsg->Header.Fields.SMBIOSTablePresent = 1;
  SendAssetTableDataMsg->Header.Fields.ASFTablePresent    = 1;

  if (AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE].Length == 0) {
    SendAssetTableDataMsg->Header.Fields.MediaTablePresent = 0;
  } else {
    SendAssetTableDataMsg->Header.Fields.MediaTablePresent = 1;
  }

  CopyMem (&SendAssetTableDataMsg->Data, AssetTableData, SendAssetTableDataMsg->Header.Fields.MessageLength);

  Length = SendAssetTableDataMsg->Header.Fields.MessageLength + sizeof (AU_MESSAGE_HEADER);

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x3020);
  Status = Heci->SendMsg (
                   HECI1_DEVICE,
                   (UINT32 *) SendAssetTableDataMsg,
                   Length,
                   BIOS_FIXED_HOST_ADDR,
                   HECI_HWA_MESSAGE_ADDR
                   );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "AssetUpdateFwMsg: Failed to Send SendAssetTableDataMsg\n"));
  }
  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x3021);

  FreePool (SendAssetTableDataMsg);

  return Status;

}

/**
  Checks if CSME supports HBM protocol version 1.0, the one implemented in BIOS

  @param[in]    HeciDev          HECI device number

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
  @retval EFI_ABORTED               Wrong response was received
  @retval EFI_INCOMPATIBLE_VERSION  CSME does not support protocol version required by BIOS
**/
EFI_STATUS
HbmHostVersion (
  IN HECI_DEVICE HeciDev
  )
{
  EFI_STATUS                     Status;
  HBM_HOST_VERSION_BUFFER        HbmHostVersion;
  UINT32                         MsgLength;
  HECI_PROTOCOL                  *Heci;

  ZeroMem (&HbmHostVersion, sizeof (HbmHostVersion));
  HbmHostVersion.Request.Command.Data             = HOST_VERSION_REQUEST;
  HbmHostVersion.Request.HostVersion.MajorVersion = HBM_MAJOR_VERSION;
  HbmHostVersion.Request.HostVersion.MinorVersion = HBM_MINOR_VERSION;
  MsgLength                                       = sizeof (HBM_HOST_VERSION_REQUEST);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Heci->SendwAck (HeciDev, (UINT32 *)&HbmHostVersion, MsgLength, &MsgLength, BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (HbmHostVersion.Response.Command.Fields.Command !=  HOST_VERSION_REQUEST || !HbmHostVersion.Response.Command.Fields.IsResponse) {
    return EFI_ABORTED;
  }
  if (HbmHostVersion.Response.HostVersionSupported != 0x01) {
    return EFI_INCOMPATIBLE_VERSION;
  }

  return Status;
}

/**
  Instructs CSME to stop all communication on given Heci device

  @param[in]    HeciDev          HECI device number

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HbmHostStop (
  IN HECI_DEVICE HeciDev
  )
{
  EFI_STATUS                     Status;
  HBM_HOST_STOP_REQUEST          HbmHostStop;
  UINT32                         MsgLength;
  HECI_PROTOCOL                  *Heci;

  ZeroMem (&HbmHostStop, sizeof (HbmHostStop));
  HbmHostStop.Command.Data = HOST_STOP_REQUEST;
  HbmHostStop.Reason       = STOP_REASON_REQUEST_FROM_DRIVER;
  MsgLength                = sizeof(HBM_HOST_STOP_REQUEST);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = Heci->SendwAck (HeciDev, (UINT32 *)&HbmHostStop, MsgLength, &MsgLength, BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);
  return Status;
}

/**
  Checks at which CSME addresses there are any clients

  @param[in]    HeciDev          HECI device number
  @param[out]   ValidAddresses   mask of addresses that have a client behind them

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
  @retval EFI_ABORTED               Wrong response was received
**/
EFI_STATUS
HbmHostEnumRequest (
  IN HECI_DEVICE    HeciDevice,
  OUT UINT8         *ValidAddresses
  )
{
  EFI_STATUS                     Status;
  HBM_HOST_ENUMERATION_BUFFER    HostEnum;
  UINT32                         MsgLength;
  HECI_PROTOCOL                  *Heci;

  ZeroMem (&HostEnum, sizeof (HostEnum));
  HostEnum.Request.Command.Data = HOST_ENUMERATE_REQUEST;
  MsgLength                     = sizeof(HBM_HOST_ENUMERATION_RESPONSE);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Heci->SendwAck (
                   HeciDevice,
                   (UINT32 *)&HostEnum,
                   sizeof (HBM_HOST_ENUMERATION_REQUEST),
                   &MsgLength,
                   BIOS_FIXED_HOST_ADDR,
                   0
                   );

  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (HostEnum.Response.Command.Fields.Command != HOST_ENUMERATE_REQUEST || !HostEnum.Response.Command.Fields.IsResponse) {
    return EFI_ABORTED;
  }

  CopyMem (ValidAddresses, &HostEnum.Response.ValidAddresses, sizeof (HostEnum.Response.ValidAddresses));

  return Status;
}

/**
  Retrieves GUID of a single CSME client

  @param[in]    HeciDev          HECI device number
  @param[in]    ClientAddr       Address of the CSME client
  @param[out]   ProtocolGuid     GUID of the client

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
  @retval EFI_ABORTED               Wrong response was received
**/
EFI_STATUS
HbmGetClientProperties (
  IN  HECI_DEVICE HeciDevice,
  IN  UINT8       ClientAddr,
  OUT EFI_GUID    *ProtocolGuid
  )
{
  HBM_HOST_CLIENT_PROPERTIES_BUFFER    HostClientProperties;//resized
  UINT32                               MsgLength;
  UINT32                               RecvLength;
  EFI_STATUS                           Status;
  HECI_PROTOCOL                        *Heci;

  HostClientProperties.Request.Command.Data = HOST_CLIENT_PROPERTIES_REQUEST;
  HostClientProperties.Request.Address      = ClientAddr;
  MsgLength                                 = sizeof (HBM_HOST_CLIENT_PROPERTIES_REQUEST);
  RecvLength                                = sizeof (HBM_HOST_CLIENT_PROPERTIES_RESPONSE);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Heci->SendwAck (HeciDevice, (UINT32*)&HostClientProperties, MsgLength, &RecvLength, BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);

  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (HostClientProperties.Response.Status ||
      HostClientProperties.Response.Command.Fields.Command != HOST_CLIENT_PROPERTIES_REQUEST ||
      !HostClientProperties.Response.Command.Fields.IsResponse) {
    DEBUG ((DEBUG_ERROR, "Identify failed\n"));
    return EFI_ABORTED;
  }

  CopyMem (ProtocolGuid, &HostClientProperties.Response.ClientProperties.ProtocolName, sizeof (EFI_GUID));

  return EFI_SUCCESS;
}

/**
  Creates a connection between given host and CSME addresses

  @param[in]    HeciDev          HECI device number
  @param[in]    MeAddress        Address of the CSME client
  @param[in]    HostAddress      Address of the Host client

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
  @retval EFI_ABORTED               Wrong response was received
**/
EFI_STATUS
HbmHostClientConnect (
  IN HECI_DEVICE HeciDev,
  IN UINT8       MeAddress,
  IN UINT8       HostAddress
  )
{
  EFI_STATUS                     Status;
  HBM_CLIENT_CONNECT_BUFFER      HbmHostClientConnect;
  UINT32                         MsgLength;
  HECI_PROTOCOL                  *Heci;

  ZeroMem (&HbmHostClientConnect, sizeof (HbmHostClientConnect));
  HbmHostClientConnect.Request.Command.Data = CLIENT_CONNECT_REQUEST;
  HbmHostClientConnect.Request.MeAddress    = MeAddress;
  HbmHostClientConnect.Request.HostAddress  = HostAddress;
  MsgLength                                 = sizeof(HBM_CLIENT_CONNECT_REQUEST);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Heci->SendwAck ( HeciDev, (UINT32 *)&HbmHostClientConnect, MsgLength, &MsgLength, BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);

  if(EFI_ERROR (Status)) {
    return Status;
  }
  if (HbmHostClientConnect.Response.Command.Fields.Command != CLIENT_CONNECT_REQUEST
      || !HbmHostClientConnect.Response.Command.Fields.IsResponse) {
    DEBUG ((DEBUG_ERROR, "HbmHostClientConnect (%x:%x) %r %x %x\n", MeAddress, HostAddress, Status,
                                                      HbmHostClientConnect.Response.Command.Fields.Command,
                                                      HbmHostClientConnect.Response.Command.Fields.IsResponse));
    return EFI_ABORTED;
  }

  if (HbmHostClientConnect.Response.Status) {
    DEBUG ((DEBUG_ERROR, "HbmHostClientConnect (%x:%x) %x\n", MeAddress, HostAddress, HbmHostClientConnect.Response.Status));
    return EFI_ABORTED;
  }
  return Status;
}

/**
  Disconnects an established connection between a Host and CSME clients

  @param[in]    HeciDev          HECI device number
  @param[in]    HostAddr         Host Address of the connection
  @param[in]    MeAddr           CSME Address of the connection

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
  @retval EFI_ABORTED               Wrong response was received
**/
EFI_STATUS
HbmClientDisconnect (
  IN HECI_DEVICE HeciDev,
  IN UINT8       HostAddr,
  IN UINT8       MeAddr
  )
{
  HBM_CLIENT_DISCONNECT_BUFFER ClientDisconnect;
  UINT32                       MsgLength;
  EFI_STATUS                   Status;
  HECI_PROTOCOL                *Heci;

  ZeroMem ((UINT8 *)&ClientDisconnect, sizeof(HBM_CLIENT_DISCONNECT_REQUEST));
  ClientDisconnect.Request.MeAddress    = MeAddr;
  ClientDisconnect.Request.HostAddress  = HostAddr;
  ClientDisconnect.Request.Command.Data = CLIENT_DISCONNECT_REQUEST;
  MsgLength                             = sizeof(HBM_CLIENT_DISCONNECT_REQUEST);

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = Heci->SendwAck (HeciDev, (UINT32*)&ClientDisconnect, MsgLength, &MsgLength, BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);

  if(EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HbmClientDisconnect: %r\n", Status));
    return Status;
  }

  if (ClientDisconnect.Response.Status ||
      ClientDisconnect.Response.Command.Fields.Command != CLIENT_DISCONNECT_REQUEST ||
      !ClientDisconnect.Response.Command.Fields.IsResponse) {
    Status = EFI_ABORTED;
  }

  return Status;
}

/**
  Sends a flow control token from BIOS to CSME that allows CSME to send one message over connection
  identified by host:Me address pair

  @param[in]    HeciDev          HECI device number
  @param[in]    HostAddr         Address of the Host client
  @param[in]    MeAddr           Address of the CSME client

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HbmSendFlowControlToken (
  IN HECI_DEVICE   HeciDev,
  IN UINT8         HostAddr,
  IN UINT8         MeAddr
  )
{
  HBM_FLOW_CONTROL  FlowControl;
  EFI_STATUS        Status;
  HECI_PROTOCOL     *Heci;

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ZeroMem (&FlowControl, sizeof(HBM_FLOW_CONTROL));
  FlowControl.Command.Fields.Command = FLOW_CONTROL;
  FlowControl.HostAddress            = HostAddr;
  FlowControl.MeAddress              = MeAddr;
  Status = Heci->SendMsg (HeciDev, (UINT32 *)&FlowControl, sizeof (HBM_FLOW_CONTROL), BIOS_FIXED_HOST_ADDR, HECI_HBM_MESSAGE_ADDR);
  return Status;
}

/**
  Attempts to receive a flow control token from CSME. This token allows BIOS to send one message over commection
  identified by Host:Me address pair

  @param[in]    HeciDev          HECI device number
  @param[in]    HostAddr         Address of the Host client
  @param[in]    MeAddr           Address of the CSME client

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HbmGetFlowControlToken (
  IN HECI_DEVICE HeciDev,
  IN UINT8       HostAddr,
  IN UINT8       MeAddr
  )
{
  EFI_STATUS        Status = EFI_SUCCESS;
  HBM_FLOW_CONTROL  FlowControl;
  UINT32            TimeLeft;
  UINT32            Length;
  HECI_PROTOCOL     *Heci;

  Status = gBS->LocateProtocol (&gHeciProtocolGuid, NULL, (VOID **) &Heci);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  TimeLeft = 1000;
  while (TimeLeft != 0) {
    MicroSecondDelay (1000);
    TimeLeft--;
    Length =sizeof (HBM_FLOW_CONTROL);
    Status = Heci->ReadMsg (HeciDev, NON_BLOCKING, (UINT32*)&FlowControl, &Length);
    if (Status == EFI_NO_RESPONSE) {
      continue;
    } else if (EFI_ERROR (Status)) {
      return Status;
    } else {
      if (FlowControl.Command.Fields.Command == FLOW_CONTROL &&
          FlowControl.HostAddress == HostAddr &&
          FlowControl.MeAddress == MeAddr) {
        return EFI_SUCCESS;
      } else {
        DEBUG ((DEBUG_ERROR, "Wrong Token received\n"));
        return EFI_DEVICE_ERROR;
      }
    }
  }
  DEBUG ((DEBUG_ERROR, "Token not received %r\n", Status));
  return Status;
}

