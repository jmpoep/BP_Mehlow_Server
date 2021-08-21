/** @file
  Header file for functions to get Intel ME information

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
#ifndef _DXE_ME_LIB_H_
#define _DXE_ME_LIB_H_

#include <Protocol/HeciProtocol.h>
#include <Protocol/PlatformMeHook.h>
#include <Protocol/MePolicy.h>
#include <CoreBiosMsg.h>
#include <BupMsgs.h>
#include <IshMsgs.h>
#include <MeiBusMsg.h>

#define MAX_ASSET_TABLE_ALLOCATED_SIZE  0x3000
#define HWA_TABLE_PUSH_CMD              0

#pragma pack(push, 1)

///
/// ME Platform Capabilities
///
typedef struct {
  UINT32  MeEnabled          : 1;        ///< [0]     ME enabled/Disabled
  UINT32  Reserved           : 2;        ///< [2:1]   Reserved, must set to 0
  UINT32  IntelAmtFw         : 1;        ///< [3]     Intel AMT FW support
  UINT32  IntelAmtFwStandard : 1;        ///< [4]     Intel AMT Standard FW support
  UINT32  Reserved1          : 9;        ///< [13:5]  Reserved
  UINT32  IntelKVM           : 1;        ///< [14]    Intel KVM supported
  UINT32  LocalWakeupTimer   : 1;        ///< [15]    Local Wakeup Timer support
  UINT32  Reserved2          : 16;       ///< [31:16] Reserved, must set to 0
  UINT32  MeMinorVer         : 16;       ///< [47:32] ME FW Minor Version.
  UINT32  MeMajorVer         : 16;       ///< [63:48] ME FW Major Version.
  UINT32  MeBuildNo          : 16;       ///< [79:64] ME FW Build Number.
  UINT32  MeHotFixNo         : 16;       ///< [95:80] ME FW Hotfix Number
} ME_CAP;

///
/// Hardware Asset Table Index
///
typedef enum {
  HWAI_TABLE_TYPE_INDEX_FRU_DEVICE  = 0,
  HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE,
  HWAI_TABLE_TYPE_INDEX_SMBIOS,
  HWAI_TABLE_TYPE_INDEX_ASF,
  HWAI_TABLE_TYPE_INDEX_MAX
} HWAI_TABLE_TYPE_INDEX;

typedef struct {
  UINT16  Offset;
  UINT16  Length;
} SINGLE_TABLE_POSITION;

typedef struct {
  SINGLE_TABLE_POSITION Tables[HWAI_TABLE_TYPE_INDEX_MAX];
  UINT8                 TableData[1];
} TABLE_PUSH_DATA;

///
/// Definitions for Hardware Asset Header
///
typedef union {
  UINT32  Data;
  struct {
    UINT32  MessageLength      : 16;
    UINT32  Command            : 4;      ///< only supported command would be HWA_TABLE_PUSH=0;
    UINT32  FRUTablePresent    : 1;
    UINT32  MediaTablePresent  : 1;
    UINT32  SMBIOSTablePresent : 1;
    UINT32  ASFTablePresent    : 1;
    UINT32  Reserved           : 8;
  } Fields;
} AU_MESSAGE_HEADER;

///
/// Asset Update Table Message
///
typedef struct {
  AU_MESSAGE_HEADER Header;
  TABLE_PUSH_DATA   Data;
} AU_TABLE_PUSH_MSG;

#pragma pack(pop)

//
// FW Feature Shipment Time State Override bitmasks definitions
//
#define CLEAR_FEATURES_BITMASK 0x00000000
#define MANAGEABILITY_BITMASK  BIT2
#define ISH_BITMASK            BIT10
#define PTT_BITMASK            BIT29

//
// Prototype for HECI message DXE phase
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
  IN CONST CHAR8*                FileName
  );

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
  );

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
  );

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
  );

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
  OUT MEFWCAPS_SKU               *RuleData
  );

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to query the local firmware update interface status.

  @param[out] RuleData            1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetLocalFwUpdate (
  OUT UINT8                       *RuleData
  );

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
  OUT MEFWCAPS_SKU                *RuleData
  );

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get Ibex Peak platform type.
  One of usages is to utilize this command to determine if the platform runs in
  Consumer or Corporate SKU size firmware.

  @param[out] RuleData            PlatformBrand,
                                  IntelMeFwImageType,
                                  SuperSku,
                                  PlatformTargetUsageType

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA     *RuleData
  );

/**
  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.FWInitComplete
  value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

  @param[out] *RuleData             MEFWCAPS_SKU message

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwFeatureStateMsg (
  OUT MEFWCAPS_SKU                *RuleData
  );

/**
  The firmware will respond to GET OEM TAG message even after the End of Post (EOP).

  @param[out] RuleData            Default is zero. Tool can create the OEM specific OEM TAG data.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetOemTagMsg (
  OUT UINT32                      *RuleData
  );

/**
  This message is sent by the BIOS when it wants to query
  ME Unconfig on RTC Clear Disable state.

  @param[out] RuleData            1 - Unconfig on RTC clear is disabled
                                  0 - Unconfig on RTC clear is not disabled

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetUnconfigOnRtcClearDisableMsg (
  OUT UINT32                *RuleData
  );

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
  );

/**
  Send Set Rule Data Request to CSME

  @param[in]   RuleId             Identifies the rule which data needs to be changed.
  @param[in]   RuleDataLength     Rule Data Length.
  @param[in]   RuleData           Pointer to new rule data.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciFwCapsSetRuleData (
  IN  UINT32                      RuleId,
  IN  UINT8                       RuleDataLength,
  IN  UINT32                      RuleData
  );

/**
  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable or disable the local firmware update interface.
  The firmware allows a single update once it receives the enable command

  @param[in] RuleData             1 - local firmware update interface enable
                                  0 - local firmware update interface disable

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetLocalFwUpdate (
  IN UINT8                        RuleData
  );

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
  );

/**
  This message is sent by the BIOS to inform ME FW whether or not to take the
  TPM 1.2 Deactivate flow

  @param[in] TpmDeactivate        0 - ME FW should not take the deactivate flow.
                                  1 - ME FW should take the deactivate flow.

  @retval EFI_UNSUPPORTED           Current ME mode doesn't support this function
  @retval EFI_SUCCESS               Command succeeded
  @retval EFI_DEVICE_ERROR          HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT               HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL      Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetAcmTpmData (
  IN UINT8               TpmDeactivate
  );

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
  );


//
// MKHI_HMRFPO_GROUP_ID = 0x05
//

/**
  Sends a message to ME to unlock a specified SPI Flash region for writing and receiving a response message.
  It is recommended that HMRFPO_ENABLE MEI message needs to be sent after all OPROMs finish their initialization.

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
  );

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
  );

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
  );

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
  );

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
  OUT UINT8 *DamState
  );

/**
  This message is sent by the BIOS to save data in a file stored in ME firmware NVM.

  @param[in]  FilePath            File path of file to write
  @param[in]  Offset              File offset
  @param[in]  DataSize            Size of data to write
  @param[in]  Flags               Flags
  @param[in]  *Buffer             Pointer to the data buffer

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_ABORTED             Cannot allocate memory
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciSetFileMsg (
  IN CONST CHAR8 *FilePath,
  IN UINT32      Offset,
  IN UINT32      DataSize,
  IN UINT8       Flags,
  IN UINT8       *DataBuffer
  );

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
  );

/**
  This message is sent by the BIOS after HeciSetFileMsg to notify ME firmware that it needs to
  commit previously set files.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciCommitFilesMsg (
  VOID
  );

//
// MKHI_MCA_GROUP_ID = 0x0A
//

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
  );

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
  );

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
  );

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
  );

/**
  Send HMRFPO disable MKHI to CSME.

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
HeciHmrfpoDisable (
  VOID
  );

//
// MKHI_GEN_GROUP_ID = 0xFF
//

/**
  Send Get Firmware Version Request to ME

  @param[in, out] *MsgGenGetFwVersionAckData  Return the message of FW version

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
HeciGetFwVersionMsg (
  IN OUT GET_FW_VER_ACK_DATA  *GetFwVersionAckData
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Retrieves the number of currently used ICC clock profile

  @param[out] IccClockSettings    ICC Clock Settings of the requested clock

  @retval EFI_UNSUPPORTED         ICC clock profile doesn't support
  @retval EFI_NOT_READY           Heci device hasn't ready yet
  @retval EFI_INVALID_PARAMETER   IccClockSettings is a NULL pointer
  @retval EFI_SUCCESS             Command succeeded
**/
EFI_STATUS
HeciGetIccClockSettings (
  OUT ICC_GETSET_CLOCK_SETTINGS_REQRSP  *IccClockSettings
  );

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
  IN ICC_GETSET_CLOCK_SETTINGS_REQRSP  *IccClockSettings
  );

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
  );

//
// Prototype for ME Policy DXE phase
//

/**
  Check if End of Post Message is enabled in setup options.

  @retval FALSE                   EndOfPost is disabled.
  @retval TRUE                    EndOfPost is enabled.
**/
BOOLEAN
MeEndOfPostEnabled (
  VOID
  );

/**
  Check if HECI2 should be removed from PCI space or not

  @retval FALSE                   HECI2 should be Disabled and removed from PCI space
  @retval TRUE                    HECI2 should be enabled and not removed from PCI space
**/
BOOLEAN
MeHeci2Enabled (
  VOID
  );

/**
  Read the ME policy to see if HECI retry is enabled.

  @retval TRUE                    HECI retry is enabled
  @retval FALSE                   HECI retry is disabled
**/
BOOLEAN
MeHeciRetryEnabled (
  VOID
  );

/**
  Read the ME policy to see if HECI message check is enabled for Bios Boot Path.

  @retval TRUE                    HECI message check is enabled
  @retval FALSE                   HECI message check is disabled
**/
BOOLEAN
MeHeciMessageCheckEnabled (
  VOID
  );

/**
  Show Me Error message. This is used to support display error message on the screen for localization
  description

  @param[in] MsgId                Me error message ID for displaying on screen message
**/
VOID
MeReportError (
  IN ME_ERROR_MSG_ID              MsgId
  );

/**
  Check if MeFwDowngrade is enabled in setup options.

  @retval FALSE                   MeFwDowngrade is disabled.
  @retval TRUE                    MeFwDowngrade is enabled.
**/
BOOLEAN
MeFwDowngradeSupported (
  VOID
  );

/**
  Check if Core Bios Done Message is enabled in setup options.

  @retval FALSE                   Core Bios Done is disabled.
  @retval TRUE                    Core Bios Done is enabled.
**/
BOOLEAN
MeCoreBiosDoneEnabled (
    VOID
  );

/**
  Read the ME policy to see if HECI timeouts are enabled.

  @retval FALSE                   HECI timeouts is disabled.
  @retval TRUE                    HECI timeouts is enabled.
**/
BOOLEAN
MeHeciTimeoutsEnabled (
  VOID
  );

/**
  Read the ME policy to see if D0I3 for HECI devices are enabled.

  @retval TRUE                   D0I3 for HECI devices are enabled
  @retval FALSE                  D0I3 for HECI devices are disabled
**/
BOOLEAN
MeHeciD0I3Enabled (
  VOID
  );

/**
  Checks if EndOfPost event already happened

  @retval TRUE                    End of post already happened
  @retval FALSE                   End of post did not happen yet
**/
BOOLEAN
MeIsAfterEndOfPost (
  VOID
  );

/**
  Set EndOfPost done
**/
VOID
MeSetEndOfPostDone (
  VOID
  );

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
  IN  HECI_DEVICE HeciDev,
  IN  UINT8       HostAddr,
  IN  UINT8       MeAddr
  );


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
  OUT EFI_GUID*   ProtocolGuid
  );


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
  IN  HECI_DEVICE HeciDev
  );


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
  IN  HECI_DEVICE HeciDev
  );


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
  IN  HECI_DEVICE    HeciDevice,
  OUT UINT8         *ValidAddresses
  );


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
  );

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
  );

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
  );

#endif
