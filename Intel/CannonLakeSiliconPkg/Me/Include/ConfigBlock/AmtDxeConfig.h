/** @file
  AMT Config Block for DXE phase

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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
#ifndef _AMT_DXE_CONFIG_H_
#define _AMT_DXE_CONFIG_H_

#define AMT_DXE_CONFIG_REVISION 1
extern EFI_GUID gAmtDxeConfigGuid;

#pragma pack (push,1)

/**
  AMT Dxe Configuration Structure.

  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Deprecate ManageabilityMode and ForceMebxSyncUp.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                 ///< Config Block Header
  UINT32  CiraRequest                     : 1;  ///< Trigger CIRA boot. <b>0: No CIRA request</b>; 1: Trigger CIRA request
  UINT32  ManageabilityMode               : 1;  ///< @deprecated since version 2. Manageability Mode sync with Mebx. 0: Disabled; <b>1: AMT</b>
  UINT32  UnConfigureMe                   : 1;  ///< OEMFlag Bit 15: Unconfigure ME with resetting MEBx password to default. <b>0: No</b>; 1: Un-configure ME without password
  UINT32  MebxDebugMsg                    : 1;  ///< OEMFlag Bit 14: Enable OEM debug menu in MEBx. <b>0: Disable</b>; 1: Enable
  UINT32  ForcMebxSyncUp                  : 1;  ///< @deprecated since version 2. <b>0: No</b>; 1: Force MEBX execution
  UINT32  HideUnConfigureMeConfirm        : 1;  ///< OEMFlag Bit 6: Hide Unconfigure ME confirmation prompt when attempting ME unconfiguration. <b>0: Don't hide</b>; 1: Hide
  UINT32  UsbProvision                    : 1;  ///< Enable/Disable of AMT USB Provisioning. <b>0: Disable</b>; 1: Enable
  UINT32  AmtbxHotkeyPressed              : 1;  ///< OEMFlag Bit 1: Enable automatic MEBx hotkey press. <b>0: Disable</b>; 1: Enable
  /**
    OEMFlag Bit 2: Enable MEBx selection screen with 2 options:
    Press 1 to enter ME Configuration Screens
    Press 2 to initiate a remote connection
     - <b>0: Disabled</b>
     - 1: Enabled
  **/
  UINT32  AmtbxSelectionScreen            : 1;
  UINT32  RsvdBits                        : 23; ///< Reserved for future use & Config block alignment
  UINT32  CiraTimeout                     : 8;
  /**
    CPU Replacement Timeout
    <b>0: 10 seconds</b>
       1: 30 seconds
     2~5: Reserved
       6: No delay
       7: Unlimited delay
  **/
  UINT32  CpuReplacementTimeout           : 8;
  UINT32  MebxNonUiTextMode               : 4;  ///< Resolution for non-UI text mode. <b>0: Auto</b>; 1: 80x25; 2: 100x31
  UINT32  MebxUiTextMode                  : 4;  ///< Resolution for UI text mode. <b>0: Auto</b>; 1: 80x25; 2: 100x31
  UINT32  MebxGraphicsMode                : 4;  ///< Resolution for graphics mode. <b>0: Auto</b>; 1: 640x480; 2: 800x600; 3: 1024x768
  UINT32  OemResolutionSettingsRsvd       : 4;  ///< Reserved for future use & Config block alignment
  UINT32  PciDeviceFilterOutTable;              ///< Pointer to a list which contain on-board devices bus/device/fun number
  UINT8   TestMebxLaunchTimeout;                ///< Maximum time to wait for FW ready to receive HECI messages. <b>0: No delay</b>; 1: 1 minutes; 5: 5 minutes; 10: 10 minutes; others: No delay
  UINT8   TestMebxRsvd[3];                      ///< Reserved for future use & Config block alignment
} AMT_DXE_CONFIG;

#pragma pack (pop)

#endif // _AMT_DXE_CONFIG_H_
