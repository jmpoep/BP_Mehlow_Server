/** @file
  Implementation file for Alert Standard Format library functionality
  This library depends on Alert Standard Format Protocol.
  Caller should check drivers' dependency.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2018 Intel Corporation.

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

#include <Uefi.h>
#include <Base.h>
#include <PiDxe.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeAmtLib.h>
#include <Protocol/AlertStandardFormat.h>
#include <Amt.h>
#include <MeChipset.h>
#include <MebxDataHob.h>

//
// Global variables
//
GLOBAL_REMOVE_IF_UNREFERENCED ASF_BOOT_OPTIONS            *mAsfBootOptions = NULL;


//
// Function implementations
//

/**
  Check if ASF boot options is present.

  @retval True                    ASF boot option is present.
  @retval False                   ASF boot option is not present
**/
BOOLEAN
AsfIsBootOptionsPresent (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return FALSE;
  }

  return (mAsfBootOptions->SubCommand == ASF_BOOT_OPTIONS_PRESENT);
}

/**
  Check if LANA ID of ASF boot options is Industry ID.

  @retval True                    IANA ID of ASF boot options is Industry ID.
  @retval False                   IANA ID of ASF boot options is not Industry ID.
**/
BOOLEAN
AsfIsIndustryIanaId (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return FALSE;
  }

  if (AsfIsBootOptionsPresent () && (mAsfBootOptions->IanaId == ASF_INDUSTRY_CONVERTED_IANA)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check if LANA ID of ASF boot options is Intel ID.

  @retval True                    IANA ID of ASF boot options is Intel ID.
  @retval False                   IANA ID of ASF boot options is not Intel ID.
**/
BOOLEAN
IsIntelIanaId (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return FALSE;
  }

  if (AsfIsBootOptionsPresent () && (mAsfBootOptions->IanaId == ASF_INTEL_CONVERTED_IANA)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Check if it is Intel ASF boot options.

  @retval True                    It is Intel ASF boot options.
  @retval False                   It is not Intel ASF boot options.
**/
BOOLEAN
IsIntelAmtBootOptions (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return FALSE;
  }

  if (IsIntelIanaId () && (mAsfBootOptions->SpecialCommand == ASF_INTEL_OEM_CMD)) {
    return TRUE;
  }

  return FALSE;
}

///
/// Special Command Parameters
///

/**
  Check the Special Command Parameter of Intel ASF boot options

  @param[in] Options              Special Command Parameter bit we want to check
                                  Bit 0: Set if Storage Redirection is to be used on the next boot. Parameter 2 is set
                                  to the driver number to be used.
                                  Bit 1: Set if Secure Boot is enforced over Storage Redirection
                                  Bit 2: Set if the BIOS is to be re-flashed on the next boot
                                  Bit 3: Set if the BIOS is to boot into the BIOS set-up screen.
                                  Bit 4: Boot into BIOS Pause on the next boot is supported
                                  Bit 5: Set if the BIOS is to participate in KVM session
                                  Bit 6: Set if the BIOS is to start a Remote Secure Erase (RSE) session
  @param[in, out] CurrentState    Return the state of result
                                  True  - Special Command Parameter bit in Options is enabled.
                                  False - Special Command Parameter bit in Options is disabled.
**/
VOID
GetSpecialParamState (
  IN     UINT16   Options,
  IN OUT BOOLEAN  *CurrentState
  )
{
  *CurrentState = FALSE;

  if (mAsfBootOptions == NULL) {
    return;
  }

  if (IsIntelAmtBootOptions () && ((mAsfBootOptions->SpecialCommandParam & Options) == Options)) {
    *CurrentState = TRUE;
  }

  return;
}

/**
  Check the OEM Parameter of Intel ASF boot options

  @param[in] Options              OEM Parameter bit we want to check
                                  Bit 0: Set if SOL is to be used on the next boot.
  @param[in, out] CurrentState    Return the state of result
                                  True : OEM Parameter bit in Options is enabled.
                                  False : OEM Parameter bit in Options is disabled.
**/
VOID
GetBootOptionsMaskState (
  IN     UINT16   Options,
  IN OUT BOOLEAN  *CurrentState
  )
{
  *CurrentState = FALSE;
  if (mAsfBootOptions == NULL) {
    return;
  }

  if (AsfIsBootOptionsPresent () && ((mAsfBootOptions->BootOptions & Options) == Options)) {
    *CurrentState = TRUE;
  }

  return;
}

/**
  Check the OEM Parameter of Intel ASF boot options

  @param[in] Options              OEM Parameter bit we want to check
                                  Bit 0: Set if SOL is to be used on the next boot.
  @param[in, out] CurrentState    Return the state of result
                                  True  : OEM Parameter bit in Options is enabled.
                                  False : OEM Parameter bit in Options is disabled.
**/
VOID
GetOemParamatersState (
  IN     UINT16   Options,
  IN OUT BOOLEAN  *CurrentState
  )
{
  *CurrentState = FALSE;
  if (mAsfBootOptions == NULL) {
    return;
  }

  if (IsIntelAmtBootOptions () && ((mAsfBootOptions->OemParameters & Options) == Options)) {
    *CurrentState = TRUE;
  }

  return;
}

/**
  Get Special Commands of Intel ASF boot options

  @retval UINT16            Special Commands.
**/
UINT8
AsfGetSpecialCommand (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return 0;
  }
  return mAsfBootOptions->SpecialCommand;
}

/**
  Get Parameters of Special Commands in Intel ASF boot options

  @retval UINT16            Parameters of Special Commands.
**/
UINT16
AsfGetSpecialCommandParam (
  VOID
  )
{
  if (mAsfBootOptions == NULL) {
    return 0;
  }
  return mAsfBootOptions->SpecialCommandParam;
}


/**
  This will return Storage Redirection Boot Option.
  True if the option is enabled.

  @retval True                    Storage Redirection boot option is enabled.
  @retval False                   Storage Redirection boot option is disabled.
**/
BOOLEAN
AsfIsStorageRedirectionEnabled (
  VOID
  )
{
  BOOLEAN                 CurrentState;

  CurrentState  = FALSE;
  GetSpecialParamState (USE_STORAGE_REDIR, &CurrentState);
  return CurrentState;
}

/**
  This will return Storage Redirection boot device number/type

  @retval            Return the boot device number to boot
                     Bit 1  Bit0
                      0    0    USBr is connected to CD/DVD device
                      0    1    USBr is connected to floppy device
                     Bits 2-7: Reserved set to 0
**/
UINT8
AsfGetStorageRedirectionBootDevice (
  VOID
  )
{
  UINT8    StorageRedirBootDevice;

  if (mAsfBootOptions == NULL) {
    StorageRedirBootDevice = 0;
  } else {
    StorageRedirBootDevice = (UINT8) ((mAsfBootOptions->SpecialCommandParam & STORAGE_REDIRECTION_BOOT_DEVICE_MASK)
                                       >> STORAGE_REDIRECTION_BOOT_DEVICE_SHIFT);
  }
  return StorageRedirBootDevice;
}

/**
  This will return Enforce Secure Boot over Storage Redirection Boot Option.
  True if the option is enabled.

  @retval True                    Enforce Secure Boot is enabled.
  @retval False                   Enforce Secure Boot is disabled.
**/
BOOLEAN
AsfIsEnforceSecureBootEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetSpecialParamState (ENFORCE_SECURE_BOOT, &CurrentState);
  return CurrentState;
}

/**
  This will return Remote Flash Boot Option.
  True if the option is enabled.

  @retval True                    Remote Flash is enabled.
  @retval False                   Remote Flash is disabled.
**/
BOOLEAN
AsfGetRemoteFlashState (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetSpecialParamState (REFLASH_BIOS, &CurrentState);
  return CurrentState;
}


/**
  This will return BIOS Setup Boot Option.
  True if the option is enabled.

  @retval True                    BIOS Setup is enabled.
  @retval False                   BIOS Setup is disabled.
**/
BOOLEAN
AsfIsEnterSetupEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetSpecialParamState (BIOS_SETUP, &CurrentState);
  return CurrentState;
}

/**
  This will return BIOS Pause Boot Option.
  True if the option is enabled.

  @retval True                    BIOS Pause is enabled.
  @retval False                   BIOS Pause is disabled.
**/
BOOLEAN
AsfIsPauseBootEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetSpecialParamState (BIOS_PAUSE, &CurrentState);
  return CurrentState;
}

/**
  This will return KVM Boot Option.
  True if the option is enabled.

  @retval True                    KVM is enabled.
  @retval False                   KVM is disabled.
**/
BOOLEAN
AsfIsKvmEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetSpecialParamState (USE_KVM, &CurrentState);
  return CurrentState;
}

/**
  This will return Secure Erase Boot Option.
  True if the option is enabled.

  @retval True                    Secure Erase is enabled.
  @retval False                   Secure Erase is disabled.
**/
BOOLEAN
AsfIsSecureEraseEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  if (IsIntelAmtBootOptions ()) {
    GetSpecialParamState (ENABLE_SECURE_ERASE, &CurrentState);
  }

  return CurrentState;
}

/**
  This will return Serial-over-Lan Boot Option.
  True if the option is enabled.

  @retval True                    Serial-over-Lan is enabled.
  @retval False                   Serial-over-Lan is disabled.
**/
BOOLEAN
AsfIsSolEnabled (
  VOID
  )
{
  BOOLEAN                 CurrentState;

  CurrentState = FALSE;
  if (AmtIsMebxSolEnabled ()) {
    GetOemParamatersState (USE_SOL, &CurrentState);
  }

  return CurrentState;
}

/**
  This will return Console Lock Boot Option.
  True if the option is enabled.

  @retval True                    Console Lock is enabled.
  @retval False                   Console Lock is disabled.
**/
BOOLEAN
AsfIsConsoleLocked (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetBootOptionsMaskState (LOCK_KEYBOARD, &CurrentState);
  return CurrentState;
}

/**
  This will return Progress Event Option.
  True if the option is enabled.

  @retval True                    Progress Event is enabled.
  @retval False                   Progress Event is disabled.
**/
BOOLEAN
AsfIsFwProgressEnabled (
  VOID
  )
{
  BOOLEAN    CurrentState;

  CurrentState = FALSE;
  GetBootOptionsMaskState (FORCE_PROGRESS_EVENTS, &CurrentState);
  return CurrentState;
}


/**
  AlertStandardFormat protocol notify callback function, it used to get ASF Boot Options

  @param  Event    The event of notify protocol.
  @param  Context  Notify event context.
**/
VOID
EFIAPI
InitializeAsfLib (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  )
{
  EFI_STATUS                          Status;
  ALERT_STANDARD_FORMAT_PROTOCOL      *AsfProtocol;

  ///
  /// Get Protocol for ASF
  ///
  Status = gBS->LocateProtocol (
                  &gAlertStandardFormatProtocolGuid,
                  NULL,
                  (VOID **) &AsfProtocol
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return;
  }

  ///
  /// Get ASF Boot Options
  ///
  Status = AsfProtocol->GetBootOptions (AsfProtocol, &mAsfBootOptions);
  ASSERT_EFI_ERROR (Status);
  return;
}

/**
  The constructor function

  @param[in]  ImageHandle  The firmware allocated handle for the EFI image.
  @param[in]  SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS      The constructor completed successfully.
**/
EFI_STATUS
EFIAPI
DxeAsfConstructor (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                          Status;
  EFI_EVENT                           AsfEvent;
  VOID                                *Registration;
  ALERT_STANDARD_FORMAT_PROTOCOL      *AsfProtocol;

  ///
  /// Check policy if AMT is supported
  ///
  if (!IsAmtBiosSupportEnabled () || !IsAsfBiosSupportEnabled ()) {
    return EFI_SUCCESS;
  }

  if (mAsfBootOptions != NULL) {
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol (&gAlertStandardFormatProtocolGuid, NULL, (VOID **) &AsfProtocol);
  if (EFI_ERROR (Status)) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    InitializeAsfLib,
                    NULL,
                    &AsfEvent
                    );
    ASSERT_EFI_ERROR (Status);

    Status = gBS->RegisterProtocolNotify (
                    &gAlertStandardFormatProtocolGuid,
                    AsfEvent,
                    &Registration
                    );
    ASSERT_EFI_ERROR (Status);
  } else {
    InitializeAsfLib (NULL, NULL);
  }

  return EFI_SUCCESS;
}

