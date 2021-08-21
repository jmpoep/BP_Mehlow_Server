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

#include "PcieLowLevel.h"
#include <SystemAgent/Library/Private/PeiPcieInitLib/PcieTrainingGeneric.h>

///
/// Function Declarations
///

EFI_STATUS
RecoverLinkFailure (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  );

EFI_STATUS
RecoverLinkSpeed (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed
  );

/**
  Check the status of the given PCIe link, detect and correct and downgrades

  @param[in]  This                          - Low level function table
  @param[in]  InputParameters               - SW EQ Input Parameters
  @param[in]  PciePort                      - PCIe Root Port
  @param[in]  OriginalLinkSpeed             - Expected speed of the PCIe link
  @param[in]  OriginalLinkWidth             - Expected width of the PCIe link
  @param[out] DeferredPlatformResetRequired - A platform reset is needed after saving Eq data to NVRAM

  @retval EFI_SUCCESS            - Link is running at the correct speed/width
  @retval EFI_UNSUPPORTED        - Unable to correct failure due to lack of GPIO PERST# support
  @retval EFI_INVALID_PARAMETER  - Unable to correct failure because the GPIO pin number is invalid
  @retval EFI_TIMEOUT            - Link did not train after pulsing PERST#
**/
EFI_STATUS
EFIAPI
PcieEnsureLinkIsHealthy (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  OUT BOOLEAN                           *PlatformResetRequired
  )
{
  EFI_STATUS              Status;

  Status = EFI_SUCCESS;
  ///
  /// Check Link Status and Recover the Link if needed
  ///
  Status = This->RecoverLinkWidth (This, PciePort, OriginalLinkWidth);
  if (EFI_ERROR (Status)) {
    ///
    ///
    Status = RecoverLinkFailure (This, InputParameters, PciePort, OriginalLinkSpeed, OriginalLinkWidth, PlatformResetRequired);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  Status = RecoverLinkSpeed (This, PciePort, OriginalLinkSpeed);
  if (EFI_ERROR (Status)) {
    ///
    ///
    Status = RecoverLinkFailure (This, InputParameters, PciePort, OriginalLinkSpeed, OriginalLinkWidth, PlatformResetRequired);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  Status = This->WaitForL0 (This, PciePort);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "DOWNGRADE, Link is not in L0\n"));
    This->RetrainLink (This, PciePort);
    Status = This->WaitForL0 (This, PciePort);
    if (EFI_ERROR (Status)) {
      ///
      ///
      Status = RecoverLinkFailure (This, InputParameters, PciePort, OriginalLinkSpeed, OriginalLinkWidth, PlatformResetRequired);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  return Status;
}

/**
  This function asserts a GPIO that controls PERST#.
  The specific GPIO and its active level is provided by GpioConfig.
  The GPIO minimum assertion time, T_PERST (100 usec), is defined in the PCIe CEM Specification.

  @param[in] This                - Low level function table
  @param[in] GpioConfig          - Pointer to PCIE_SWEQ_GPIO_CONFIG

  @retval EFI_SUCCESS            - GPIO set successfully
  @retval EFI_UNSUPPORTED        - GPIO is not supported
  @retval EFI_INVALID_PARAMETER  - GPIO pad is invalid
**/
EFI_STATUS
AssertPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_SWEQ_GPIO_CONFIG             *GpioConfig
  )
{
  EFI_STATUS    Status;
  GPIO_PAD      GpioPad;
  UINT8         AssertLevel;

  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_INFO, "Asserting PCIe slot reset.\n"));

  if (!(GpioConfig->EnableGpioPerstSupport == TRUE)) {
    Status = EFI_UNSUPPORTED;
  }
  if (!EFI_ERROR (Status)) {
    GpioPad   = GpioConfig->GpioPad;
    if (GpioConfig->GpioActiveHigh) {
      AssertLevel = 1;
    } else {
      AssertLevel = 0;
    }
    Status = This->SetPchGpio (This, GpioPad, AssertLevel);
  }

  return Status;
}

/**
  This function deasserts a GPIO that controls PERST#.
  The specific GPIO and its active level is provided by GpioConfig.
  The GPIO minimum assertion time, T_PERST (100 usec), is defined in the PCIe CEM Specification.

  @param[in] This                - Low level function table
  @param[in] GpioConfig          - Pointer to PCIE_SWEQ_GPIO_CONFIG

  @retval EFI_SUCCESS            - GPIO set successfully
  @retval EFI_UNSUPPORTED        - GPIO is not supported
  @retval EFI_INVALID_PARAMETER  - GPIO pad is invalid
**/
EFI_STATUS
DeassertPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_SWEQ_GPIO_CONFIG             *GpioConfig
  )
{
  EFI_STATUS    Status;
  GPIO_PAD      GpioPad;
  UINT8         DeassertLevel;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "Deasserting PEG slot reset.\n"));

  if (!(GpioConfig->EnableGpioPerstSupport == TRUE)) {
    Status = EFI_UNSUPPORTED;
  }
  if (!EFI_ERROR (Status)) {
    GpioPad   = GpioConfig->GpioPad;
    if (GpioConfig->GpioActiveHigh) {
      DeassertLevel = 0;
    } else {
      DeassertLevel = 1;
    }
    Status = This->SetPchGpio (This, GpioPad, DeassertLevel);
  }

  return Status;
}

/**
  This function asserts/deasserts a GPIO that controls PERST#.  The minimum
  assertion time, T_PERST (100 usec), is defined in the PCIe CEM Specification.

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  InputParameters             - SW EQ Input Parameters
  @param[in]  AssertPerst                 - TRUE to assert PERST#, FALSE to deassert

  @retval EFI_SUCCESS            - GPIO set successfully
  @retval EFI_UNSUPPORTED        - GPIO is not supported
  @retval EFI_INVALID_PARAMETER  - GPIO pin number is invalid
**/
EFI_STATUS
EFIAPI
PcieSetPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  BOOLEAN                           assertPerst
  )
{
  PCIE_SWEQ_GPIO_CONFIG *GpioConfig;
  UINT8                 PortIndex;
  UINT8                 Bus;
  UINT8                 Device;
  UINT8                 Function;
  EFI_STATUS            Status;

  ///
  /// Get GPIO Configuration for this port
  ///
  GpioConfig = NULL;
  for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
    Bus      = InputParameters->PerPortInputParameters[PortIndex].PciePort->Bus;
    Device   = InputParameters->PerPortInputParameters[PortIndex].PciePort->Device;
    Function = InputParameters->PerPortInputParameters[PortIndex].PciePort->Function;
    if ((Bus      == PciePort->Bus)         &&
        (Device   == PciePort->Device)      &&
        (Function == PciePort->Function)) {
      GpioConfig = &(InputParameters->PerPortInputParameters[PortIndex].GpioConfig);
      break;
    }
  }
  if (GpioConfig == NULL) {
    return EFI_NOT_FOUND;
  }

  if (assertPerst) {
    Status = AssertPerst (This, GpioConfig);
    MicroSecondDelay (100);
  } else {
    Status = DeassertPerst (This, GpioConfig);
  }

  return Status;
}

EFI_STATUS
RecoverLinkFailure (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  )
{
  EFI_STATUS    Status;
  UINT8         CurrentLinkWidth;
  UINT8         CurrentLinkSpeed;

  ///
  /// A platform reset should be done after presets are saved in NVRAM
  ///
  if (InputParameters->AlwaysAttemptSwEq) {
    (*DeferredPlatformResetRequired) = FALSE;
  } else {
    (*DeferredPlatformResetRequired) = TRUE;
  }

  Status = This->ResetEndpointPerst (This, PciePort, InputParameters);

  CurrentLinkWidth = This->GetNegotiatedWidth (This, PciePort);
  if (CurrentLinkWidth < OriginalLinkWidth) {
    DEBUG ((DEBUG_WARN, "Link Width DOWNGRADED!\n"));
    Status = EFI_TIMEOUT;
  }
  CurrentLinkSpeed = This->GetCurrentLinkSpeed (This, PciePort);
  if (CurrentLinkSpeed < OriginalLinkSpeed) {
    DEBUG ((DEBUG_WARN, "Link Speed DOWNGRADED!\n"));
    Status = EFI_TIMEOUT;
  }

  return Status;
}

EFI_STATUS
RecoverLinkSpeed (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed
  )
{
  EFI_STATUS Status;
  UINT8      CurrentLinkSpeed;

  Status = EFI_SUCCESS;

  CurrentLinkSpeed = This->GetCurrentLinkSpeed (This, PciePort);
  if (CurrentLinkSpeed < OriginalLinkSpeed) {
    DEBUG ((DEBUG_INFO, "DOWNGRADE from Gen %d to Gen %d detected\n", OriginalLinkSpeed, CurrentLinkSpeed));

    This->RetrainLink (This, PciePort);
    Status = This->WaitForL0 (This, PciePort);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Link Speed Recovery FAILED!\n"));
      return Status;
    }

    CurrentLinkSpeed = This->GetCurrentLinkSpeed (This, PciePort);
    if (CurrentLinkSpeed < OriginalLinkSpeed) {
      DEBUG ((DEBUG_INFO, "Link Speed Recovery FAILED!\n"));
      Status = EFI_DEVICE_ERROR;
    } else {
      DEBUG ((DEBUG_INFO, "Link Speed Recovery Successful\n"));
    }
  }

  return Status;
}
