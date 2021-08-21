/** @file
  Framework PEIM to HECI.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2008 - 2019 Intel Corporation.

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

#ifdef UP_SPS_SUPPORT
#include <MeHeciRegs.h>
#endif // UP_SPS_SUPPORT
#include "HeciInit.h"

//
// Function Declarations
//
static HECI_PPI               mHeciPpi = {
  HeciSendwAckWithRetry,
  HeciReceive,
  HeciSendWithRetry,
  HeciInitialize,
  HeciGetMeStatus,
  HeciGetMeMode
};

static EFI_PEI_PPI_DESCRIPTOR mInstallHeciPpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gHeciPpiGuid,
  &mHeciPpi
};

//
// Function Implementations
//

/**
  Initialize MMIO BAR address for HECI devices

  @retval EFI_SUCCESS             HECI devices initialize successfully
  @retval others                  Error occur
**/
EFI_STATUS
PeiHeciDevicesInit (
  VOID
  )
{
  SI_PREMEM_POLICY_PPI *SiPreMemPolicyPpi;
  EFI_STATUS           Status;
  ME_PEI_PREMEM_CONFIG *MePeiPreMemConfig;
  BOOLEAN              HeciCommunication2;
#ifdef UP_SPS_SUPPORT
  BOOLEAN              HeciCommunication;
  BOOLEAN              HeciCommunication3;

  HeciCommunication   = TRUE;
  HeciCommunication3  = TRUE;
#else // UP_SPS_SUPPORT
#endif // UP_SPS_SUPPORT

  ///
  /// Get Policy settings through the SiPreMemPolicy PPI
  ///
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  if (!EFI_ERROR (Status)) {
    Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gMePeiPreMemConfigGuid, (VOID *) &MePeiPreMemConfig);
    ASSERT_EFI_ERROR (Status);

    MeDeviceInit (HECI1, MePeiPreMemConfig->Heci1BarAddress, 0);
    MeDeviceInit (HECI2, MePeiPreMemConfig->Heci2BarAddress, 0);
    MeDeviceInit (HECI3, MePeiPreMemConfig->Heci3BarAddress, 0);
#ifdef UP_SPS_SUPPORT
    MeDeviceInit (HECI4, MePeiPreMemConfig->Heci4BarAddress, 0);
#endif // UP_SPS_SUPPORT

    ///
    /// Determine if ME devices should be Enabled/Disable and Program Subsystem IDs if Enabled
    /// Zero in Bit x enables the device
    ///
    HeciCommunication2 = MeHeci2Enabled ();
#ifdef UP_SPS_SUPPORT
    if (MePeiPreMemConfig->HeciCommunication == FORCE_DISABLE) {
      HeciCommunication = FALSE;
    }
    if (MePeiPreMemConfig->HeciCommunication3 == FORCE_DISABLE) {
      HeciCommunication3 = FALSE;
    }
    if (HeciCommunication == FALSE) {
      HeciCommunication2 = FALSE;
    }
#else // UP_SPS_SUPPORT
#endif // UP_SPS_SUPPORT

#ifdef UP_SPS_SUPPORT
    if (HeciCommunication) {
      DEBUG ((DEBUG_INFO, "HeciEnable\n"));
      HeciEnable ();
    } else {
      DEBUG ((DEBUG_INFO, "HeciDisable\n"));
      HeciDisable ();
    }
#else // UP_SPS_SUPPORT
      DEBUG ((DEBUG_INFO, "HeciEnable\n"));
      HeciEnable ();
#endif // UP_SPS_SUPPORT

    if (HeciCommunication2) {
      DEBUG ((DEBUG_INFO, "Heci2Enable\n"));
      Heci2Enable ();
    } else {
      DEBUG ((DEBUG_INFO, "Heci2Disable\n"));
      Heci2Disable ();
    }

#ifdef UP_SPS_SUPPORT
    if (HeciCommunication3 && HeciCommunication) {
      DEBUG ((DEBUG_INFO, "Heci3Enable\n"));
      Heci3Enable ();
    } else {
      DEBUG ((DEBUG_INFO, "Heci3Disable\n"));
      Heci3Disable ();
    }
#else // UP_SPS_SUPPORT
      DEBUG ((DEBUG_INFO, "Heci3Enable\n"));
      Heci3Enable ();
#endif // UP_SPS_SUPPORT
    if (MePeiPreMemConfig->KtDeviceEnable) {
      DEBUG ((DEBUG_INFO, "SolEnable\n"));
      SolEnable ();
    } else {
      DEBUG ((DEBUG_INFO, "SolDisable\n"));
      SolDisable ();
    }
#ifdef UP_SPS_SUPPORT
  } else {
     MeDeviceInit (HECI1, HECI1_MBAR_DEFAULT, 0);
     HeciEnable ();
     Status = EFI_SUCCESS;
#endif // UP_SPS_SUPPORT
  }

  return Status;
}

/**
  Internal function performing Heci PPIs init needed in PEI phase

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_DEVICE_ERROR        ME FPT is bad
**/
EFI_STATUS
InstallHeciPpi (
  VOID
  )
{
  EFI_STATUS  Status;
#ifdef UP_SPS_SUPPORT
  HECI_PPI   *HeciPpi;
#endif // UP_SPS_SUPPORT

  DEBUG((DEBUG_INFO, "ME-BIOS: HECI PPI Entry.\n"));
  PostCode (0xE03);

  ///
  /// Check for HECI device present and ME FPT Bad
  ///
  if ((PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, PCI_VENDOR_ID_OFFSET)) == 0x0) ||
      (PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, PCI_VENDOR_ID_OFFSET)) == 0xFFFFFFFF)
#ifndef UP_SPS_SUPPORT
      || ((PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_ME_HFS)) & 0x0020) != 0)
#endif
    ) {
    DEBUG((DEBUG_ERROR, "ME-BIOS: HECI PPI Exit - Error by HECI device error.\n"));
    PostCode (0xE83);
    return EFI_DEVICE_ERROR;
  }

#ifdef UP_SPS_SUPPORT
  Status = PeiServicesLocatePpi (
              &gHeciPpiGuid,
             0,
             NULL,
             (VOID **) &HeciPpi
             );
  if (!EFI_ERROR (Status)) {
     DEBUG((DEBUG_INFO, "HECI Ppi already installed.\n"));
     return EFI_SUCCESS;
  }
#endif // UP_SPS_SUPPORT

  ///
  /// Initialize Heci platform PPIs
  ///
  Status = HeciInitialize (HECI1_DEVICE);

  ///
  /// Heci Ppi should be installed always due to DID message might be required still.
  /// Unsupported messages requested will be blocked when utilize Heci Ppi
  ///
  Status = PeiServicesInstallPpi (&mInstallHeciPpi);

  if (!EFI_ERROR (Status)) {
    DEBUG((DEBUG_INFO, "ME-BIOS: HECI PPI Exit - Success.\n"));
    PostCode (0xE23);
  } else {
    DEBUG((DEBUG_ERROR, "ME-BIOS: HECI PPI Exit - Error by install HeciPpi fail, Status: %r\n", Status));
    ASSERT_EFI_ERROR (Status);
    PostCode (0xEA3);
  }

  return Status;
}

/**
  Internal function performing PM register initialization for Me
**/
VOID
MePmInit (
  VOID
  )
{
  ///
  /// Before system memory initialization, BIOS should check the wake status to determine if Intel Management Engine
  /// has reset the system while the host was in a sleep state. If platform was not in a sleep state
  /// BIOS should ensure a non-sleep exit path is taken. One way to accomplish this is by forcing
  /// an S5 exit path by the BIOS.
  ///
  if (PmcGetSleepTypeAfterWake () == PmcNotASleepState) {
    PmcSetSleepState (PmcS5SleepState);
    DEBUG ((DEBUG_INFO, "MePmInit () - Force an S5 exit path.\n"));
  }
}

