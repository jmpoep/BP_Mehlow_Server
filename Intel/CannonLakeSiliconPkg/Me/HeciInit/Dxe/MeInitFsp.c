/** @file
  The implementation about Me ReadyToBootEvent for FSP support.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation.

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

#include <Guid/EventGroup.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/ConfigBlockLib.h>
#include <MeChipset.h>
#include <Ppi/Wdt.h>
#include <Ppi/SiPolicy.h>
#include <Library/PeiMeLib.h>
#include <Library/MeShowBufferLib.h>
#include <Private/Library/HeciInitLib.h>
#include "MeInit.h"
#include <PchResetPlatformSpecific.h>
#include <Library/PcdLib.h>
#include <Private/Library/PmcPrivateLib.h>

EFI_STATUS
EFIAPI
MeReadyToBootEventFsp (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  );

EFI_STATUS
EFIAPI
MeEndOfFirmwareEventFsp (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  );

STATIC EFI_PEI_NOTIFY_DESCRIPTOR  mMeReadyToBootNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK  | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiEventReadyToBootGuid,
    MeReadyToBootEventFsp
  }
};

STATIC EFI_PEI_NOTIFY_DESCRIPTOR  mMeEndOfFirmwareNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK  | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gFspEventEndOfFirmwareGuid,
    MeEndOfFirmwareEventFsp
  }
};

/**
  The notification registered as a callback to perform Me event prior to boot

  @param[in]  PeiServices         An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation
  @param[in]  NotifyDescriptor    Address of the notification descriptor data structure.
  @param[in]  Ppi                 Address of the PPI that was installed.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    No enough buffer to allocate
**/
EFI_STATUS
EFIAPI
MeReadyToBootEventFsp (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
{
  EFI_STATUS                      Status;
  WDT_PPI                         *Wdt;
  UINT32                          RequestedActions;
  PCH_RESET_DATA                  ResetData;
  ME_BIOS_BOOT_PATH               MeBiosPath;
  BOOLEAN                         DisableMe;

  DisableMe = FALSE;

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
  if ((MeBiosPath == MeNormalBiosPath || MeBiosPath == MeErrorBiosPath || MeBiosPath == MeRecoveryBiosPath) &&
      (GetBootModeHob () != BOOT_ON_S3_RESUME)) {
    if (MeEndOfPostEnabled ()) {
      RequestedActions = HeciEopStatusSuccess;
      Status = MeEndOfPostEvent (&RequestedActions);
      ///
      /// Set EndOfPostDone right after EOP message is sent before reset system.
      ///
      MeSetEndOfPostDone ();
      if (EFI_ERROR (Status)) {
        DisableMe = TRUE;
        DEBUG ((DEBUG_ERROR, "MeEndOfPostEvent failed with Status = %r\n", Status));
      } else if (RequestedActions == HeciEopPerformGlobalReset) {
        ///
        /// Locate WDT PPI for access to Wdt->AllowKnownReset()
        ///
        Status = PeiServicesLocatePpi (
                   &gWdtPpiGuid,
                   0,
                   NULL,
                   (VOID **) &Wdt
                   );
        if (!EFI_ERROR (Status)) {
          Wdt->AllowKnownReset ();
        }
        DEBUG ((DEBUG_INFO, "%a - Global Reset requested by FW EOP ACK %r\n", __FUNCTION__, Status));
        CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
        StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
        (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetPlatformSpecific, EFI_SUCCESS, sizeof (PCH_RESET_DATA), &ResetData);
      }
    }
  }

  PmcDisableMeDevices (DisableMe);

  ///
  /// PMIR Configuration & FDSW Lockdown
  ///
  LockConfig ();

  return EFI_SUCCESS;
}


/**
  The notification registered as a callback to perform Me EndOfFirmware event prior to boot.

  @param[in]  PeiServices         An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation
  @param[in]  NotifyDescriptor    Address of the notification descriptor data structure.
  @param[in]  Ppi                 Address of the PPI that was installed.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
EFIAPI
MeEndOfFirmwareEventFsp (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
{
  UINT32 HeciBars[PCI_MAX_BAR];
  UINT8  MeDevFunction;

  ///
  /// Don't set D0I3 if disabled by policy
  ///
  if (MeHeciD0I3Enabled () == FALSE) {
    DEBUG ((DEBUG_INFO, "MeEndOfFirmwareEventFsp - SKIP setting D0I3 bits for HECI devices on boot time\n"));
    return EFI_SUCCESS;
  }

  DEBUG ((DEBUG_INFO, "MeEndOfFirmwareEventFsp - setting D0I3 bits for HECI devices on boot time\n"));

  //
  // 1. Save MMIO BAR addresses for all HECI devices.
  // 2. Set to default (32-bit) BAR addresses for all HECI devices.
  // 3. Set D0i3 bit
  // 4. Restore MMIO BAR addresses for all HECI devices.
  //
  for (MeDevFunction = HECI1; MeDevFunction <= HECI4; MeDevFunction++) {
    if (!IsHeciDeviceFunction (MeDevFunction)) {
      continue;
    }
    MeSaveBars (MeDevFunction, HeciBars);
    MeDeviceInit (MeDevFunction, PcdGet32 (PcdSiliconInitTempMemBaseAddr), 0);
    SetD0I3Bit (MeDevFunction);
    MeRestoreBars (MeDevFunction, HeciBars);
  }

  return EFI_SUCCESS;
}

/**
  FSP HeciInit Module Entry Point for signal ReadyToBoot FSP event

  @param[in] FileHandle           PEIM's file handle
  @param[in] PeiServices          An indirect pointer to the EFI_PEI_SERVICES table published by
                                  the PEI Foundation

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver
**/
EFI_STATUS
EFIAPI
MeInitEntryPointFsp (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "Fsp MeInitEntryPointFsp() Start\n"));

  Status = PeiServicesNotifyPpi (mMeReadyToBootNotifyList);
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesNotifyPpi (mMeEndOfFirmwareNotifyList);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "Fsp MeInitEntryPointFsp() End\n"));

  return Status;
}
