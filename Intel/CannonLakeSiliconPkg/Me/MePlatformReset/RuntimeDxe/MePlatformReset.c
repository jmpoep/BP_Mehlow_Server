/** @file
  Provide the ResetSystem AP

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2018 Intel Corporation.

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
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/ResetSystemLib.h>
#include <Protocol/MePlatformGetResetType.h>
#include <PchResetPlatformSpecific.h>

/**
  Resets the entire platform.

  @param[in] ResetType            UEFI defined reset type.
  @param[in] ResetStatus          The status code for the reset.
  @param[in] DataSize             The size of ResetData in bytes.
  @param[in] ResetData            Optional element used to introduce a platform specific reset.
                                  The exact type of the reset is defined by the EFI_GUID that follows
                                  the Null-terminated Unicode string.

**/
VOID
ResetSystem (
  IN EFI_RESET_TYPE     ResetType,
  IN EFI_STATUS         ResetStatus,
  IN UINTN              DataSize,
  IN VOID               *ResetData OPTIONAL
  )
{
  switch (ResetType) {
  case EfiResetWarm:
    ResetWarm ();
    break;

  case EfiResetCold:
    ResetCold ();
    break;

  case EfiResetShutdown:
    ResetShutdown ();
    return;

  case EfiResetPlatformSpecific:
    ResetPlatformSpecific (DataSize, ResetData);
    return;

  default:
    return;
  }

  //
  // Given we should have reset getting here would be bad
  //
  ASSERT (FALSE);
}


/**
  Reset the system

  @param[in] ResetType            Warm or cold
  @param[in] ResetStatus          Possible cause of reset
  @param[in] DataSize             Size of ResetData in bytes
  @param[in] ResetData            Optional Unicode string

  @retval Does not return if the reset takes place.
**/
VOID
EFIAPI
PlatformResetSystem (
  IN EFI_RESET_TYPE               ResetType,
  IN EFI_STATUS                   ResetStatus,
  IN UINTN                        DataSize,
  IN VOID                         *ResetData OPTIONAL
  )
{
  EFI_STATUS                          Status;
  ME_PLATFORM_GET_RESET_TYPE_PROTOCOL *MePlatformGetResetType;
  ME_SPECIAL_RESET_TYPES              MeResetType;
  ME_SPECIAL_RESET_TYPES              OverrideMeResetType;
  EFI_RESET_TYPE                      OverrideResetType;
  UINTN                               NumberMePlatformGetResetHandles;
  EFI_HANDLE                          *MePlatformGetResetHandles;
  UINTN                               Index;

  MeResetType         = (ME_SPECIAL_RESET_TYPES) ResetType;
  OverrideMeResetType = (ME_SPECIAL_RESET_TYPES) ResetType;
  OverrideResetType   = ResetType;

  if (!EfiAtRuntime ()) {
    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gMePlatformGetResetTypeGuid,
                    NULL,
                    &NumberMePlatformGetResetHandles,
                    &MePlatformGetResetHandles
                    );
    if (!EFI_ERROR (Status)) {
      for (Index = 0; Index < NumberMePlatformGetResetHandles; Index++) {
        Status = gBS->HandleProtocol (
                        MePlatformGetResetHandles[Index],
                        &gMePlatformGetResetTypeGuid,
                        (VOID **) &MePlatformGetResetType
                        );
        if (!EFI_ERROR (Status)) {
          MeResetType = MePlatformGetResetType->MeGetResetType (ResetType);
          DEBUG ((DEBUG_INFO, "Returned ResetType is: %x\n", MeResetType));
          if (MeResetType >= MaxRestReq) {
            DEBUG ((DEBUG_ERROR, "Platform Reset failed, invalid parameter\n"));
            ASSERT (FALSE);
          }
          if (OverrideMeResetType < MeResetType) {
            DEBUG ((DEBUG_INFO, "Previous ResetType is: %x\n", OverrideMeResetType));
            OverrideMeResetType = MeResetType;
            OverrideResetType        = (EFI_RESET_TYPE) OverrideMeResetType;
            switch (OverrideMeResetType) {
              case PowerCycleResetReq:
                OverrideResetType    = EfiResetCold;
                break;
              case GlobalResetReq:
              case GlobalResetWithEcReq:
                OverrideResetType    = EfiResetPlatformSpecific;
                if (ResetData == NULL) {
                  DataSize = sizeof (PCH_RESET_DATA);
                  ResetData = AllocateZeroPool (DataSize);
                  CopyMem (&((PCH_RESET_DATA *)ResetData)->Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
                  StrCpyS (((PCH_RESET_DATA *)ResetData)->Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
                }
                break;
              default:
                break;
            }
          }
          DEBUG ((DEBUG_INFO, "Current ResetType is: %x\n", OverrideResetType));
        }
      }
    }
  }

  ResetSystem (OverrideResetType, ResetStatus, DataSize, ResetData);

  ASSERT (FALSE);
  CpuDeadLoop();
}

/**
  Entry point of Platform Reset driver.

  @param[in] ImageHandle          Standard entry point parameter
  @param[in] SystemTable          Standard entry point parameter

  @retval EFI_SUCCESS             Reset RT protocol installed
  @retval All other error conditions encountered result in an ASSERT
**/
EFI_STATUS
EFIAPI
InitializePlatformReset (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_HANDLE                      Handle;
  EFI_STATUS                      Status;

  DEBUG ((DEBUG_INFO, "Install MePlatformReset() Start\n"));
  ///
  /// Make sure the Reset Architectural Protocol is not already installed in the system
  ///
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiResetArchProtocolGuid);

  ///
  /// Hook the runtime service table
  ///
  SystemTable->RuntimeServices->ResetSystem = PlatformResetSystem;
  ///
  /// Now install the Reset RT AP on a new handle
  ///
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiResetArchProtocolGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

