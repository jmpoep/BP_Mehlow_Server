/** @file
  Implementation file for AMT Policy functionality for PEIM

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

#include <PiPei.h>
#include <MebxDataHob.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiAmtLib.h>
#include <Library/ConfigBlockLib.h>

/**
  Get config block of AMT PEI from AMT policy ppi

  @param[in] AmtPeiConfig         The AMT config block.

  @retval EFI_SUCCESS             Get config block of AMT PEI successfully.
  @retval All other error conditions encountered result in an ASSERT.
**/
EFI_STATUS
PeiAmtConfigBlockInit (
  IN AMT_PEI_CONFIG   **AmtPeiConfig
  )
{
  EFI_STATUS             Status;
  SI_POLICY_PPI          *SiPolicyPpi;

  ///
  /// Locate system configuration variable
  ///
  SiPolicyPpi = NULL;
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gAmtPeiConfigGuid, (VOID *) AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Check if AMT Watchdog Timer is enabled in setup options.

  @retval FALSE                   AMT Watchdog Timer is disabled.
  @retval TRUE                    AMT Watchdog Timer is enabled.
**/
BOOLEAN
PeiAmtIsWatchdogTimerEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_PEI_CONFIG                  *AmtPeiConfig;

  if (PeiAmtIsManageabilityModeEnabled ()) {
    Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
    ASSERT_EFI_ERROR (Status);
    if (AmtPeiConfig->WatchDog == 1) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Get BIOS Watchdog Timer value.

  @retval UINT16                  BIOS Watchdog Timer value
**/
UINT16
PeiAmtGetBiosWatchdogTimer (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_PEI_CONFIG                  *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  if (!EFI_ERROR (Status)) {
    return AmtPeiConfig->WatchDogTimerBios;
  } else {
    return 0;
  }

}

/**
  Check if AMT is enabled in setup options.

  @retval FALSE                   ActiveManagement is disabled.
  @retval TRUE                    ActiveManagement is enabled.
**/
BOOLEAN
PeiIsAmtBiosSupportEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_PEI_CONFIG                  *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  if (AmtPeiConfig->AmtEnabled == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  Check if ASF is enabled in setup options.

  @retval FALSE                   ASF is disabled.
  @retval TRUE                    ASF is enabled.
**/
BOOLEAN
PeiIsAsfBiosSupportEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_PEI_CONFIG                  *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  if (AmtPeiConfig->AsfEnabled == 1) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  This function is deprecated. Maintained only for backward compatibility in CNL.
  Provide Manageability Mode setting from MEBx BIOS Sync Data

  @retval UINT8                   Manageability Mode = MNT_ON or MNT_OFF
**/
UINT8
PeiGetManageabilityModeSetting (
  VOID
  )
{
  if (PeiAmtIsManageabilityModeEnabled ()) {
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
PeiAmtIsManageabilityModeEnabled (
  VOID
  )
{
  EFI_STATUS        Status;
  AMT_PEI_CONFIG    *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  return (AmtPeiConfig->ManageabilityMode == MNT_ON);
}

/**
  This will return Progress Event Option.
  True if the option is enabled.

  @retval True                    Progress Event is enabled.
  @retval False                   Progress Event is disabled.
**/
BOOLEAN
PeiIsFwProgressSupported (
  VOID
  )
{
  EFI_STATUS                      Status;
  AMT_PEI_CONFIG                  *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  if (AmtPeiConfig->FwProgress == 1) {
    return TRUE;
  }

  return FALSE;
}

/**
  Provide Amt Sol feature setting from MEBx BIOS Sync Data

  @retval TRUE                    Amt Sol feature is enabled.
  @retval FALSE                   Amt Sol feature is disabled.
**/
BOOLEAN
PeiAmtIsSolFeatureEnabled (
  VOID
  )
{
  EFI_STATUS        Status;
  AMT_PEI_CONFIG    *AmtPeiConfig;

  Status = PeiAmtConfigBlockInit (&AmtPeiConfig);
  ASSERT_EFI_ERROR (Status);

  return (AmtPeiConfig->AmtSolEnabled == SOL_ENABLE);
}
