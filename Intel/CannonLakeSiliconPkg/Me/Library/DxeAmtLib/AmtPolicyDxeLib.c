/** @file
  Implementation file for AMT Policy functionality

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

#include "AmtDxeLibInternals.h"
#include <MebxDataHob.h>

//
// Global variables
//
GLOBAL_REMOVE_IF_UNREFERENCED AMT_PEI_CONFIG      *mAmtPeiConfig = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED AMT_DXE_CONFIG      *mAmtDxeConfig = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED MEBX_DATA_HOB       *mMebxDataHob  = NULL;

/**
  Initialize module variable - mAmtPeiConfig for AMT PEI Config Block.

  @retval EFI_SUCCESS             mAmtPeiConfig is initialized.
  @retval All other error conditions encountered when mAmtPeiConfig initialized fail.
**/
STATIC
EFI_STATUS
AmtPeiConfigInit (
  VOID
  )
{
  EFI_PEI_HOB_POINTERS            HobPtr;

  if (mAmtPeiConfig != NULL) {
    return EFI_SUCCESS;
  }

  //
  // Get AMT Policy HOB.
  //
  HobPtr.Guid  = GetFirstGuidHob (&gAmtPolicyHobGuid);
  ASSERT (HobPtr.Guid != NULL);
  if (HobPtr.Guid != NULL) {
    mAmtPeiConfig = (AMT_PEI_CONFIG *) GET_GUID_HOB_DATA (HobPtr.Guid);
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

/**
  Initialize module variable - mAmtDxeConfig for AMT DXE Config Block.

  @retval EFI_SUCCESS             mAmtDxeConfig is initialized.
  @retval All other error conditions encountered when mAmtDxeConfig initialized fail.
**/
EFI_STATUS
AmtDxeConfigInit (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_POLICY_PROTOCOL             *DxeAmtPolicy;

  if (mAmtDxeConfig != NULL) {
    return EFI_SUCCESS;
  }

  DxeAmtPolicy = NULL;
  Status = gBS->LocateProtocol (&gDxeAmtPolicyGuid, NULL, (VOID **) &DxeAmtPolicy);
  if (EFI_ERROR (Status) || (DxeAmtPolicy == NULL)) {
    DEBUG ((DEBUG_ERROR, "No AMT Policy Protocol available"));
    return EFI_UNSUPPORTED;
  }

  Status = GetConfigBlock ((VOID *) DxeAmtPolicy, &gAmtDxeConfigGuid, (VOID *) &mAmtDxeConfig);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Initialize module variable - mMebxDataHob for MEBX Data Hob.

  @retval EFI_SUCCESS             mMebxDataHob is initialized.
  @retval All other error conditions encountered when mMebxDataHob initialized fail.
**/
STATIC
EFI_STATUS
AmtMebxDataInit (
  VOID
  )
{
  EFI_HOB_GUID_TYPE    *GuidHob;
  MEBX_DATA_HOB        *MebxData;

  GuidHob  = NULL;
  MebxData = NULL;

  if (mMebxDataHob != NULL) {
    return EFI_SUCCESS;
  }

  GuidHob = GetFirstGuidHob (&gAmtMebxDataGuid);
  if (GuidHob != NULL) {
    MebxData = (MEBX_DATA_HOB *) GET_GUID_HOB_DATA (GuidHob);
  }
  if (MebxData != NULL) {
    mMebxDataHob = MebxData;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}


/**
  Check if Asf is enabled in setup options.

  @retval FALSE                   Asf is disabled.
  @retval TRUE                    Asf is enabled.
**/
BOOLEAN
IsAsfBiosSupportEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // First check if ASF support is enabled in Setup.
  //
  if (mAmtPeiConfig->AsfEnabled != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Check if Amt is enabled in setup options.

  @retval FALSE                   Amt is disabled.
  @retval TRUE                    Amt is enabled.
**/
BOOLEAN
IsAmtBiosSupportEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // First check if AMT support is enabled in Setup.
  //
  if (mAmtPeiConfig->AmtEnabled != 1) {
    return FALSE;
  }

  return TRUE;
}

/**
  Check if AMT BIOS Extension hotkey pressed is enabled in setup options.

  @retval FALSE                   MEBx hotkey pressed option is disabled.
  @retval TRUE                    MEBx hotkey pressed option is enabled.
**/
BOOLEAN
AmtIsHotkeyPressedEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // First check if AMT Setup Prompt is enabled in Setup.
  //
  if (mAmtDxeConfig->AmtbxHotkeyPressed == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if AMT BIOS Extension Selection Screen is enabled in setup options.

  @retval FALSE                   AMT Selection Screen is disabled.
  @retval TRUE                    AMT Selection Screen is enabled.
**/
BOOLEAN
AmtIsSelectionScreenEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  //
  // First check if AMT Selection Screen is enabled in Setup.
  //
  if (mAmtDxeConfig->AmtbxSelectionScreen == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if AMT Watchdog Timer is enabled in setup options.

  @retval FALSE                   AMT Watchdog Timer is disabled.
  @retval TRUE                    AMT Watchdog Timer is enabled.
**/
BOOLEAN
AmtIsWatchdogTimerEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (AmtIsManageabilityModeEnabled () && IsAsfBiosSupportEnabled () && (mAmtPeiConfig->WatchDog == 1)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Return BIOS watchdog timer value

  @retval UINT16                  BIOS ASF Watchdog Timer value
**/
UINT16
AmtGetBiosWatchdogTimer (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return 0;
  }

  return mAmtPeiConfig->WatchDogTimerBios;
}

/**
  Return OS Watchdog timer value

  @retval UINT16                  OS ASF Watchdog Timer value
**/
UINT16
AmtGetOsWatchdogTimer (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return 0;
  }

  return mAmtPeiConfig->WatchDogTimerOs;
}

/**
  Provide CIRA request information from OEM code.

  @retval Check if any CIRA requirement during POST
**/
BOOLEAN
AmtIsCiraRequested (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return mAmtDxeConfig->CiraRequest == 1;
}

/**
  Provide CIRA request Timeout from OEM code.

  @retval CIRA require Timeout for MPS connection to be established
**/
UINT8
AmtGetCiraRequestTimeout (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return (UINT8) (UINTN) mAmtDxeConfig->CiraTimeout;
}

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Provide Manageability Mode setting from MEBx BIOS Sync Data

  @retval UINT8                   Manageability Mode = MNT_ON or MNT_OFF
**/
UINT8
GetManageabilityModeSetting (
  VOID
  )
{
  if (AmtIsManageabilityModeEnabled ()) {
    return MNT_ON;
  } else {
    return MNT_OFF;
  }
}

/**
  Provide Manageability Mode setting from MEBx BIOS Sync Data

  @retval TRUE                    Manageability Mode is enabled.
  @retval FALSE                   Manageability Mode is disabled.
**/
BOOLEAN
AmtIsManageabilityModeEnabled (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = AmtMebxDataInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  DEBUG ((DEBUG_INFO, "Is Manageability Mode Enabled : 0x%x\n", (mMebxDataHob->PlatformMngSel == MNT_ON) ? "TRUE" : "FALSE"));
  return (mMebxDataHob->PlatformMngSel == MNT_ON);
}

/**
  Provide Remote Assistance Availablilty from MEBx BIOS Sync Data

  @retval TRUE                    Remote Assistance Mode is available.
  @retval FALSE                   Remote Assistance Mode is NOT available.
**/
BOOLEAN
AmtIsMebxRemoteAssistanceEnabled (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = AmtMebxDataInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  DEBUG ((DEBUG_INFO, "Is Remote Assistance Enabled? %a\n", (mMebxDataHob->RemoteAssistanceTriggerAvailablilty == 1) ? "TRUE" : "FALSE"));
  return (mMebxDataHob->RemoteAssistanceTriggerAvailablilty == 1);
}


/**
  Provide Serial-over-Lan setting from MEBx BIOS Sync Data

  @retval TRUE                    Serial-over-Lan is enabled.
  @retval FALSE                   Serial-over-Lan is disabled.
**/
BOOLEAN
AmtIsMebxSolEnabled (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = AmtMebxDataInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return (mMebxDataHob->AmtSol == SOL_ENABLE);
}

/**
  Provide KVM setting from MEBx BIOS Sync Data

  @retval FALSE                   KVM is disabled.
  @retval TRUE                    KVM is enabled.
**/
BOOLEAN
AmtIsMebxKvmEnabled (
  VOID
  )
{
  EFI_STATUS    Status;

  Status = AmtMebxDataInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  DEBUG ((DEBUG_INFO, "Is Mebx Kvm Enabled? %a\n", (mMebxDataHob->KvmEnable == KVM_ENABLE) ? "TRUE" : "FALSE"));
  return (mMebxDataHob->KvmEnable == KVM_ENABLE);
}

/**
  Provide UnConfigure ME without password request from OEM code.

  @retval Check if Unconfigure ME without password request
**/
BOOLEAN
AmtIsUnconfigureMeEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return mAmtDxeConfig->UnConfigureMe == 1;
}

/**
  Provide 'Hiding the Unconfigure ME without password confirmation prompt' request from OEM code.

  @retval Check if 'Hide Unconfigure ME without password Confirmation prompt' request
**/
BOOLEAN
AmtIsHideUnconfigureMeConfPromptEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return mAmtDxeConfig->HideUnConfigureMeConfirm == 1;
}

/**
  Provide show MEBx debug message request from OEM code.

  @retval Check show MEBx debug message request
 **/
BOOLEAN
AmtIsMebxDebugMsgEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return mAmtDxeConfig->MebxDebugMsg == 1;
}

/**
  Provide on-board device list table and do not need to report them to AMT.  AMT only need to know removable PCI device
  information.

  @retval on-board device list table pointer other than system device.
**/
UINT32
AmtPciDeviceFilterOutTable (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  return mAmtDxeConfig->PciDeviceFilterOutTable;
}

/**
  Check if USB provisioning enabled/disabled in Policy.

  @retval FALSE                   USB provisioning is disabled.
  @retval TRUE                    USB provisioning is enabled.
**/
BOOLEAN
IsUsbProvisionSupportEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (mAmtDxeConfig->UsbProvision == 1) {
    return TRUE;
  }

  return FALSE;
}

/**
  This will return Progress event Option.
  True if the option is enabled.

  @retval True                    Progress Event is enabled.
  @retval False                   Progress Event is disabled.
**/
BOOLEAN
IsFwProgressSupported (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (mAmtPeiConfig->FwProgress == 1) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check if ForcMebxSyncUp is enabled.

  @retval FALSE                   ForcMebxSyncUp is disabled.
  @retval TRUE                    ForcMebxSyncUp is enabled.
**/
BOOLEAN
AmtIsForceMebxSyncUpEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;

  Status = AmtPeiConfigInit ();
  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if (mAmtPeiConfig->ForcMebxSyncUp == 1) {
    return TRUE;
  }

  return FALSE;
}
