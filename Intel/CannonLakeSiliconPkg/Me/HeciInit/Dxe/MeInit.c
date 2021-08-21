/** @file
  ME Initialize implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2018 Intel Corporation.

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

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/S3BootScriptLib.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/AmtReadyToBoot.h>
#include <Library/DxeMeLib.h>
#include <Library/MeTypeLib.h> // UP_SPS_SUPPORT
#include <Library/PchCycleDecodingLib.h>
#include <Private/Library/HeciInitLib.h>
#include "HeciInit.h"
#include "MeInit.h"
#include <Protocol/Wdt.h>
#include <Library/ConfigBlockLib.h>
#include <PchResetPlatformSpecific.h>
#include <Private/Library/PmcPrivateLib.h>

extern HECI_INSTANCE    *mHeciContext;

/**
  Dummy return for Me signal event use

  @param[in] Event                The event that triggered this notification function
  @param[in] Context              Pointer to the notification functions context
**/
VOID
EFIAPI
MeEmptyEvent (
  IN  EFI_EVENT                   Event,
  IN  VOID                        *Context
  )
{
  return;
}

/**
  Signal a event to save Me relevant registers and this event must be run before ExitPmAuth.

  @param[in] Event                The event that triggered this notification function
  @param[in] ParentImageHandle    Pointer to the notification functions context

**/
VOID
EFIAPI
MeScriptSaveEvent (
  IN  EFI_EVENT                   Event,
  IN  VOID                        *ParentImageHandle
  )
{
  EFI_STATUS                      Status;
  HECI_PROTOCOL                   *Heci;
  UINT32                          MeMode;
  HECI_FWS_REGISTER               MeFirmwareStatus;

  ///
  /// PMIR Configuration Save
  ///
  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return;
  }
  ///
  /// Check ME Status
  ///
  Status = Heci->GetMeMode (&MeMode);
  ASSERT_EFI_ERROR (Status);

  MeFirmwareStatus.ul = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_ME_HFS));

  ///
  /// PCH BIOS Spec Rev 0.9 Section 18.4  Additional Power Management Programming
  /// Step 2
  ///   Set "Power Management Initialization Register (ETR3) Field 1", D31:F2:ACh[31] = 1b
  ///   for production machine according to "RS - PCH Intel Management Engine
  ///  (Intel(r) ME) BIOS Writer's Guide".
  ///
  /// PCH ME BWG section 4.5.1
  /// The IntelR FPT tool /GRST option uses CF9GR bit to trigger global reset.
  /// Based on above reason, the BIOS should not lock down CF9GR bit during Manufacturing and
  /// Re-manufacturing environment.
  ///
  /// ETR3 is a resume well register and has no script save for it.
  /// System may go through S3 resume path from G3 if DeepSleep is enabled,
  /// that means all resume well registers will be reset to defaults.
  /// Save boot script for ETR3 register if DeepSleep is enabled.
  ///
  if (!MeTypeIsSps() && ((MeMode == ME_MODE_NORMAL) || (MeMode == ME_MODE_TEMP_DISABLED)) && !(MeFirmwareStatus.r.ManufacturingMode)) { // UP_SPS_SUPPORT
    ///
    /// PCH ME BWG section 4.4.1
    /// BIOS must also ensure that CF9GR is cleared and locked (via bit31 of the same register) before
    /// handing control to the OS in order to prevent the host from issuing global resets and resetting
    /// Intel Management Engine.
    ///
    PmcDisableCf9GlobalResetWithLockInS3BootScript ();
  } else {
    PmcDisableCf9GlobalResetInS3BootScript ();
  }

  //
  //Sending notification that BIOS is starting to run 3rd party code.
  //
  if (MeCoreBiosDoneEnabled ()) {
    HeciCoreBiosDoneMsg ();
  }

  gBS->CloseEvent (Event);
  return;
}

/**
  Signal a event for Me ready to boot.
  Performing below tasks in order:
  1. Signal AmtReadyToBoot events for AMT platform hooks.
  2. Signal MePlatformReadyToBoot event for platform hooks.
  3. Report FWSTS and save to SMBIOS
  4. Me device de-configure for supporting late status reflect
     situation if any FW error has reported after PEI phase
  5. Send EOP message
  6. Lock config for manufacturing mode
  7. Remove PSF access

  @param[in] Event                The event that triggered this notification function
  @param[in] Context              Pointer to the notification functions context

**/
VOID
EFIAPI
MeReadyToBootEvent (
  IN  EFI_EVENT                   Event,
  IN  VOID                        *Context
  )
{
  EFI_STATUS                      Status;
  EFI_EVENT                       MePlatformReadyToBootEvent;
  EFI_HANDLE                      *Handles;
  UINTN                           Index;
  UINTN                           Count;
  AMT_READY_TO_BOOT_PROTOCOL      *AmtReadyToBoot;
  ME_BIOS_BOOT_PATH               MeBiosPath;
  UINT32                          RequestedActions;
  WDT_PROTOCOL                    *WdtProtocol;
  PCH_RESET_DATA                  ResetData;
  UINT32                          MeStatus;
  BOOLEAN                         DisableMe;

  DisableMe = FALSE;

  ///
  /// We will trigger all events in order
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gAmtReadyToBootProtocolGuid,
                  NULL,
                  &Count,
                  &Handles
                  );
  if (!EFI_ERROR (Status)) {
    for (Index = 0; Index < Count; Index++) {
      Status = gBS->HandleProtocol (Handles[Index], &gAmtReadyToBootProtocolGuid, (VOID **) &AmtReadyToBoot);
      ASSERT_EFI_ERROR (Status);
      AmtReadyToBoot->Signal ();
    }
  }

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  MeEmptyEvent,
                  NULL,
                  &gMePlatformReadyToBootGuid,
                  &MePlatformReadyToBootEvent
                  );
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    gBS->SignalEvent (MePlatformReadyToBootEvent);
    gBS->CloseEvent (MePlatformReadyToBootEvent);
  }

  ///
  /// Report FWSTS and save to SMBIOS.
  ///
  AddFwStsSmbiosEntry (NULL, NULL);

  ///
  /// Check for ME Error before MEI1 gets disabled
  ///
  HeciGetMeStatus (&MeStatus);

  ///
  /// If ME is running in below BIOS paths then hide MEI1 prior to boot
  ///
  CheckMeBootPath (&MeBiosPath);
  if ((MeBiosPath == MeSecoverMeiMsgBiosPath) || (MeBiosPath == MeSwTempDisableBiosPath) || (MeBiosPath == MeSecoverJmprBiosPath)) {
    DisableMe = TRUE;
  }

  ///
  /// Send EoP message for BIOS paths below - this is crucial for platform security
  ///
  if (MeBiosPath == MeNormalBiosPath || MeBiosPath == MeErrorBiosPath || MeBiosPath == MeRecoveryBiosPath) {
    CheckFwUpdInProgress ();
    MeWarningMessage ();

    if (MeEndOfPostEnabled ()) {
      RequestedActions = HeciEopStatusSuccess;
      Status = MeEndOfPostEvent (&RequestedActions);
      ///
      /// Set EndOfPostDone right after EOP message is sent before reset system.
      ///
      MeSetEndOfPostDone ();
      if (EFI_ERROR (Status)) {
        DisableMe = TRUE;
        MeReportError (MSG_EOP_ERROR);
        DEBUG ((DEBUG_ERROR, "MeEndOfPostEvent failed with Status = %r\n", Status));
      } else if (RequestedActions == HeciEopPerformGlobalReset) {
        Status = gBS->LocateProtocol (&gWdtProtocolGuid, NULL, (VOID **) &WdtProtocol);
        if (!EFI_ERROR (Status)) {
          WdtProtocol->AllowKnownReset ();
        }
        DEBUG ((DEBUG_INFO, "%a - Global Reset requested by FW EOP ACK %r\n", __FUNCTION__, Status));
        CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
        StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
        gRT->ResetSystem (EfiResetPlatformSpecific, EFI_SUCCESS, sizeof (PCH_RESET_DATA), &ResetData);
      }
    }
  } else if (ME_STATUS_ME_STATE_ONLY (MeStatus) == ME_DISABLE_WAIT) {
    MeReportError (MSG_PLAT_DISABLE_WAIT);
  }

  PmcDisableMeDevices (DisableMe);

  if (!MeEndOfPostEnabled ()) {
    CheckAndFixHeciForAccess (HECI1_DEVICE);
  }

#ifndef FSP_WRAPPER_FLAG
  ///
  /// PMIR Configuration & FDSW Lockdown
  ///
  LockConfig ();
#endif

  gBS->CloseEvent (Event);

  return;
}

