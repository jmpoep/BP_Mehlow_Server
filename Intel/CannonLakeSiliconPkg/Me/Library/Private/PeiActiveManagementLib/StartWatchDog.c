/** @file
  Start Watchdog timer in PEI phase

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include "StartWatchDog.h"

/**
  Perform the platform spefific initializations.

  @retval EFI_SUCCESS             if the interface could be successfully installed.
**/
EFI_STATUS
PeiInitStartWatchDog (
  VOID
  )
{
  EFI_STATUS    Status;
  EFI_BOOT_MODE BootMode;
  HECI_PPI      *HeciPpi;
  UINT16        BiosWatchdogTimer = 0;

  DEBUG ((DEBUG_INFO, "PeiInitStartWatchDog () - Start\n"));

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (!EFI_ERROR (Status) && (BootMode == BOOT_ON_S3_RESUME)) {
    return EFI_SUCCESS;
  }

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

  ///
  /// If it is corporate SKU, the first thing for BIOS is to stop ASF BIOS watchdog timer
  /// when BIOS proceeds with normal POST after DID message. The reason is because BIOS
  /// doesn't know what the state of ASF BIOS watchdog timer was in the previous boot.
  /// Start BIOS Watchdog only if conditions below are met.
  ///
  PeiHeciAsfStopWatchDog (HeciPpi);

  if (PeiAmtIsWatchdogTimerEnabled ()) {
    BiosWatchdogTimer = PeiAmtGetBiosWatchdogTimer ();
    if (BiosWatchdogTimer != 0) {
      Status = PeiHeciAsfStartWatchDog (HeciPpi, BiosWatchdogTimer);
    } else {
      DEBUG ((DEBUG_ERROR, "0 timeout value, unable to start ASF BIOS WDT \n"));
      Status = EFI_INVALID_PARAMETER;
    }
  } else {
    DEBUG ((DEBUG_ERROR, "AMT WatchDog policy is not enabled, don't  start ASF BIOS WDT \n"));
    Status = EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "PeiInitStartWatchDog () - End\n"));
  return Status;
}
