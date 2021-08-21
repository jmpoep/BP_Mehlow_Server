/** @file
  This driver recovers the PEG link.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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

@par Specification
**/

#include "SaPegLowLevel.h"
#include <SystemAgent/Library/Private/PeiSaPcieInitLib/PcieTraining.h>
#include <Ppi/PchReset.h>
#include <Library/GpioLib.h>
#include <Library/GpioNativeLib.h>

/**
  This function sets a GPIO to a particular level.

  @param[in] This                - Low level function table
  @param[in] GpioNumber          - PCH GPIO Pad
  @param[in] Level               - 0 = Low, 1 = High

  @retval EFI_SUCCESS            - GPIO set successfully
  @retval EFI_UNSUPPORTED        - GPIO is not supported
  @retval EFI_INVALID_PARAMETER  - GPIO pin number is invalid
**/
EFI_STATUS
EFIAPI
SetPchGpio (
  IN PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN GPIO_PAD                          GpioPad,
  IN UINT8                             Level
  )
{
  EFI_STATUS  Status;

  Status = GpioSetOutputValue (GpioPad, Level);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  return Status;
}

/**
  This function asserts and deasserts a GPIO that controls PERST#.
  The specific GPIO and its active level is provided by a policy.
  The GPIO minimum assertion time, T_PERST (100 usec), is defined in the PCIe CEM Specification.

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  InputParameters             - SW EQ Input Parameters

  @retval EFI_SUCCESS            - Did toggle GPIO
  @retval EFI_UNSUPPORTED        - Didn't toggle GPIO
  @retval EFI_INVALID_PARAMETER  - Didn't toggle GPIO
**/
EFI_STATUS
TogglePegSlotReset (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters
  )
{
  EFI_STATUS  Status;
  UINT8       PortIndex;

  DEBUG ((DEBUG_INFO, "Toggling PCIe slot PERST#.\n"));
  ///
  ///
  for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
    PciSegmentOr16 (PciePort[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, BIT4);
  }
  Status = This->SetPerst (This, PciePort, InputParameters, TRUE);
  if (!EFI_ERROR (Status)) {
    MicroSecondDelay (100 * STALL_ONE_MICRO_SECOND);
    ///
    ///
    for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
      PciSegmentAnd16 (PciePort[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, (UINT16) ~(BIT4));
    }
    Status = This->SetPerst (This, PciePort, InputParameters, FALSE);
  } else {
    ///
    ///
    for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
      PciSegmentAnd16 (PciePort[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, (UINT16) ~(BIT4));
    }
  }
  return Status;
}

/**
  Recovers a link width downgrade back to the original width.  Generally this
  doesn't need to be called directly since EnsureLinkIsHealthy() checks link
  width in addition to other link health checks.

  @param[in]  This               - Low level function table
  @param[in]  PciePort           - PCIe Root Port
  @param[in]  OriginalLinkWidth  - Original Link Width

  @retval EFI_SUCCESS            - Link is running at the correct width
  @retval EFI_DEVICE_ERROR       - Unable to correct link width downgrade
  @retval EFI_TIMEOUT            - Link did not successfully retrain
**/
EFI_STATUS
EFIAPI
RecoverLinkWidth (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkWidth
  )
{
  EFI_STATUS Status;
  UINT8      CurrentLinkWidth;

  Status = EFI_SUCCESS;

  CurrentLinkWidth = This->GetNegotiatedWidth (This, PciePort);
  if (CurrentLinkWidth < OriginalLinkWidth) {
    DEBUG ((DEBUG_INFO, "DOWNGRADE from x%d to x%d detected\n", OriginalLinkWidth, CurrentLinkWidth));
    ///
    ///
    PciSegmentAndThenOr32 (
      PciePort->ConfigSpaceBase + R_SA_PEG_LTSSMC_OFFSET,
      B_SA_PEG_LTSSMC_WIDTH_MASK,
      OriginalLinkWidth
      );
    ///
    ///
    This->SetLinkDisable (This, PciePort, TRUE);
    MicroSecondDelay (STALL_ONE_MICRO_SECOND);
    This->SetLinkDisable (This, PciePort, FALSE);
    ///
    ///
    Status = This->WaitForL0 (This, PciePort);
    ///
    ///
    PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_LTSSMC_OFFSET, 0x1F);
    ///
    ///
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Width Recovery FAILED!\n"));
      return Status;
    }

    CurrentLinkWidth = This->GetNegotiatedWidth (This, PciePort);
    if (CurrentLinkWidth == OriginalLinkWidth) {
      DEBUG ((DEBUG_INFO, "Width Recovery Successful\n"));
      Status = EFI_SUCCESS;
    } else {
      DEBUG ((DEBUG_INFO, "Width Recovery FAILED!\n"));
      Status = EFI_DEVICE_ERROR;
    }
  }

  return Status;
}
