/** @file
  Header file for AMT functionality

@copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2017 Intel Corporation.

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
#ifndef _DXE_AMT_LIB_H_
#define _DXE_AMT_LIB_H_

#include <AsfMsgs.h>

//
// Prototype for AMT Policy
//
/**
  Initialize module variable - mAmtDxeConfig for AMT DXE Config Block.

  @retval EFI_SUCCESS             mAmtDxeConfig is initialized.
  @retval All other error conditions encountered when mAmtDxeConfig initialized fail.
**/
EFI_STATUS
AmtDxeConfigInit (
  VOID
  );

/**
  Check if Asf is enabled in setup options.

  @retval FALSE                   Asf is disabled.
  @retval TRUE                    Asf is enabled.
**/
BOOLEAN
IsAsfBiosSupportEnabled (
  VOID
  );

/**
  Check if Amt is enabled in setup options.

  @retval FALSE                   Amt is disabled.
  @retval TRUE                    Amt is enabled.
**/
BOOLEAN
IsAmtBiosSupportEnabled (
  VOID
  );

/**
  Check if AMT BIOS Extension hotkey pressed is enabled in setup options.

  @retval FALSE                   MEBx hotkey pressed option is enabled.
  @retval TRUE                    MEBx hotkey pressed option is enabled.
**/
BOOLEAN
AmtIsHotkeyPressedEnabled (
  VOID
  );

/**
  Check if AMT BIOS Extension Selection Screen is enabled in setup options.

  @retval FALSE                   AMT Selection Screen is disabled.
  @retval TRUE                    AMT Selection Screen is enabled.
**/
BOOLEAN
AmtIsSelectionScreenEnabled (
  VOID
  );

/**
  Check if AMT Watchdog Timer is enabled in setup options.

  @retval FALSE                   AMT Watchdog Timer is disabled.
  @retval TRUE                    AMT Watchdog Timer is enabled.
**/
BOOLEAN
AmtIsWatchdogTimerEnabled (
  VOID
  );

/**
  Return BIOS watchdog timer value

  @retval UINT16                  BIOS ASF Watchdog Timer value
**/
UINT16
AmtGetBiosWatchdogTimer (
  VOID
  );

/**
  Return OS Watchdog timer value

  @retval UINT16                  OS ASF Watchdog Timer value
**/
UINT16
AmtGetOsWatchdogTimer (
  VOID
  );

/**
  Provide CIRA request information from OEM code.

  @retval Check if any CIRA requirement during POST
**/
BOOLEAN
AmtIsCiraRequested (
  VOID
  );

/**
  Provide CIRA request Timeout from OEM code.

  @retval CIRA require Timeout for MPS connection to be established
**/
UINT8
AmtGetCiraRequestTimeout (
  VOID
  );

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Provide Manageability Mode setting from MEBx BIOS Sync Data

  @retval UINT8                   Manageability Mode = MNT_ON or MNT_OFF
**/
UINT8
GetManageabilityModeSetting (
  VOID
  );

/**
  Provide Manageability Mode setting from MEBx BIOS Sync Data

  @retval TRUE                    Manageability Mode is enabled.
  @retval FALSE                   Manageability Mode is disabled.
**/
BOOLEAN
AmtIsManageabilityModeEnabled (
  VOID
  );

/**
  Provide Remote Assistance Availablilty from MEBx BIOS Sync Data

  @retval TRUE                    Remote Assistance Mode is available.
  @retval FALSE                   Remote Assistance Mode is NOT available.
**/
BOOLEAN
AmtIsMebxRemoteAssistanceEnabled (
  VOID
  );

/**
  Provide Serial-over-Lan setting from MEBx BIOS Sync Data

  @retval TRUE                    Serial-over-Lan is enabled.
  @retval FALSE                   Serial-over-Lan is disabled.
**/
BOOLEAN
AmtIsMebxSolEnabled (
  VOID
  );

/**
  Provide KVM setting from MEBx BIOS Sync Data

  @retval FALSE                   KVM is disabled.
  @retval TRUE                    KVM is enabled.
**/
BOOLEAN
AmtIsMebxKvmEnabled (
  VOID
  );

/**
  Provide UnConfigure ME without password request from OEM code.

  @retval Check if Unconfigure ME without password request
**/
BOOLEAN
AmtIsUnconfigureMeEnabled (
  VOID
  );

/**
  Provide 'Hiding the Unconfigure ME without password confirmation prompt' request from OEM code.

  @retval Check if 'Hide unConfigure ME without password Confirmation prompt' request
**/
BOOLEAN
AmtIsHideUnconfigureMeConfPromptEnabled (
  VOID
  );

/**
  Provide show MEBx debug message request from OEM code.

  @retval Check show MEBx debug message request
 **/
BOOLEAN
AmtIsMebxDebugMsgEnabled (
  VOID
  );

/**
  Provide on-board device list table and do not need to report them to AMT.  AMT only need to know removable PCI device
  information.

  @retval on-board device list table pointer other than system device.
**/
UINT32
AmtPciDeviceFilterOutTable (
  VOID
  );

/**
  Check if USB provisioning enabled/disabled in Policy.

  @retval FALSE                   USB provisioning is disabled.
  @retval TRUE                    USB provisioning is enabled.
**/
BOOLEAN
IsUsbProvisionSupportEnabled(
  VOID
  );

/**
  This will return progress event Option.
  True if the option is enabled.

  @retval True                    progress event is enabled.
  @retval False                   progress event is disabled.
**/
BOOLEAN
IsFwProgressSupported (
  VOID
  );

/**
  Check if ForcMebxSyncUp is enabled.

  @retval FALSE                   ForcMebxSyncUp is disabled.
  @retval TRUE                    ForcMebxSyncUp is enabled.
**/
BOOLEAN
AmtIsForceMebxSyncUpEnabled (
  VOID
  );

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Dump ME_BIOS_EXTENSION_SETUP variable
**/
VOID
DxeMebxSetupVariableDebugDump (
  VOID
  );

//
// Heci Messages for AMT/ASF ME Client
//

/**
  Stop ASF Watch Dog Timer HECI message.
**/
VOID
AsfStopWatchDog (
  VOID
  );

/**
  Start ASF Watch Dog Timer

  @param[in] WatchDogType         Which kind of WatchDog, ASF OS WatchDog Timer setting or ASF BIOS WatchDog Timer setting

**/
VOID
AsfStartWatchDog (
  IN  UINT8                       WatchDogType
  );

/**
  This is used to send KVM request message to Intel ME. When
  Bootoptions indicate that a KVM session is requested then BIOS
  will send this message before any graphical display output to
  ensure that FW is ready for KVM session.

  @param[in] QueryType            0 - Query Request
                                  1 - Cancel Request
  @param[out] ResponseCode        1h - Continue, KVM session established.
                                  2h - Continue, KVM session cancelled.

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
AmtQueryKvm (
  IN  UINT32                      QueryType,
  OUT UINT32                      *ResponseCode
  );

/**
  Send secure erase operation status using PET

  @param[in]    OperationResult   Status of secure erase operation

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
SendRsePetAlert (
  IN EFI_STATUS                        OperationResult
  );

/**
  Use ASF_GetRsePassword to get disk password from the FW

  @param[in,out]   Password            Preallocated buffer to save c string
                                       password to. It has to be at least 32
                                       characters wide.

  @retval EFI_SUCCESS                  Buffer Password contains returned password
  @retval EFI_NOT_FOUND                Either there is no password in AMT memory
                                       or Heci communication failed
  @retval EFI_DEVICE_ERROR             Failed to initialize HECI
  @retval EFI_TIMEOUT                  HECI is not ready for communication
  @retval EFI_UNSUPPORTED              Current ME mode doesn't support send this function
**/
EFI_STATUS
GetRsePassword (
  IN OUT CHAR16                       *Password
  );

/**
  Clears boot options by sending a proper ASF command through HECI

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
ClearBootOptions (
  VOID
  );

/**
  Get boot options by sending a proper ASF command through HECI

  @param[out] BootOptionsResponse  Boot Options returned from HECI

  @retval EFI_UNSUPPORTED          Current ME mode doesn't support this function
  @retval EFI_SUCCESS              Command succeeded
  @retval EFI_DEVICE_ERROR         HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT              HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL     Message Buffer is too small for the Acknowledge
  @retval EFI_INVALID_PARAMETER    BootOptionsResponse is a NULL pointer
**/
EFI_STATUS
GetBootOptions (
  OUT GET_BOOT_OPTIONS_RESPONSE        *BootOptionsResponse
  );

/**
  Send ASF_ReportBiosStatus message

  @param[in]       BiosStatus     Current bios status

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
**/
EFI_STATUS
ReportBiosStatus (
  EFI_STATUS                           BiosStatus
  );

#endif
