/** @file
  This driver recovers the PEG link.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

///
/// Function Declarations
///
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
  );

/**
  Wait until link is up.

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port

  @retval EFI_SUCCESS     - Completed successfully before timeout
  @retval EFI_TIMEOUT     - Timed out
**/
EFI_STATUS
EFIAPI
SklWaitForL0 (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT32        Index;
  EFI_STATUS    Status;
  BOOLEAN       CheckEq;
  BOOLEAN       CompletedEq;
  UINT32        EqStatus;
  UINT32        LinkStatus;

  Status         = EFI_TIMEOUT;
  CheckEq        = (PciePort->EndpointMaxLinkSpeed >= 0x3) ? TRUE : FALSE;
  CompletedEq    = FALSE;
  Index          = 0;

  ///
  /// If endpoint's LCAP.MLS (Spec section 7.8.6) indicated Gen3 capability, first wait for equalization to complete.
  /// Check equalization status LSTS2.EC (Spec section 7.8.20) until Gen3 equalization successfully completed.
  ///
  if (CheckEq) {
    for (; Index < 100; Index++) {
      EqStatus = PciSegmentRead16 (PciePort->ConfigSpaceBase + R_SA_PEG_LSTS2_OFFSET);
      EqStatus = (EqStatus >> 1) & 0x1;
      if (EqStatus == 0x1) {
        CompletedEq = TRUE;
        break;
      }
      MicroSecondDelay (STALL_ONE_MILLI_SECOND);
    }
  }

  ///
  /// Check for L0 status.
  /// Continue up to 100 msec of combined delay.
  /// Skip if equalization was needed but didn't successfully complete.
  ///
  if ((CheckEq && CompletedEq) || !CheckEq) {
    for (; Index < 100; Index++) {
      LinkStatus = PciSegmentRead32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGSTS_OFFSET);
      LinkStatus = (LinkStatus >> 16) & 0xF;
      if (LinkStatus == 0x7) {
        Status = EFI_SUCCESS;
        break;
      }
      MicroSecondDelay (STALL_ONE_MILLI_SECOND);
    }
  }

  return Status;
}

/**
  Resets the endpoint connected to the given root port by directly pulsing the
  PERST# signal.  The minimum assertion time, T_PERST (100 usec), is defined in
  the PCIe CEM Specification.

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  InputParameters             - SW EQ Input Parameters

  @retval EFI_SUCCESS            - GPIO set successfully
  @retval EFI_UNSUPPORTED        - GPIO is not supported
  @retval EFI_INVALID_PARAMETER  - GPIO pin number is invalid
  @retval EFI_TIMEOUT            - Link did not train after pulsing PERST#
**/
EFI_STATUS
EFIAPI
SklResetEndpointPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters
  )
{
  PCIE_PORT_INFO        AffectedPorts[SA_PEG_MAX_FUN];
  PCIE_SWEQ_GPIO_CONFIG *GpioConfig;
  EFI_STATUS            Status;
  UINT8                 PortIndex;
  UINT8                 Bus;
  UINT8                 Device;
  UINT8                 Function;
  UINT8                 Stall;
  UINT8                 AffectedPortsLength;
  BOOLEAN               EqComplete;

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

  ///
  /// Determine which ports will be impacted by the PERST# pulse
  ///
  AffectedPortsLength = 0;
  for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
    if ((InputParameters->PerPortInputParameters[PortIndex].GpioConfig.EnableGpioPerstSupport == GpioConfig->EnableGpioPerstSupport) &&
        (InputParameters->PerPortInputParameters[PortIndex].GpioConfig.GpioPad                == GpioConfig->GpioPad)                &&
        (InputParameters->PerPortInputParameters[PortIndex].GpioConfig.GpioActiveHigh         == GpioConfig->GpioActiveHigh)) {
        CopyMem (&AffectedPorts[AffectedPortsLength], (InputParameters->PerPortInputParameters[PortIndex].PciePort), sizeof (PCIE_PORT_INFO));
      AffectedPortsLength++;
    }
  }

  ///
  /// Bypass phase2 and assert slot reset
  ///
  This->SetPhase2Bypass (This, &(AffectedPorts[0]), AffectedPortsLength, TRUE);
  Status = TogglePegSlotReset (This, PciePort, InputParameters);
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Success.\n"));

    ///
    /// Wait for Equalization Done
    ///
    EqComplete = FALSE;
    for (Stall = 0; Stall < 100; Stall++) {
      if ((PciSegmentRead16 (PciePort->ConfigSpaceBase + R_SA_PEG_LSTS2_OFFSET) & BIT1) == BIT1) {
        EqComplete = TRUE;
        break;
      }
      MicroSecondDelay (STALL_ONE_MILLI_SECOND);
    }
    if (!EqComplete) {
      return EFI_TIMEOUT;
    }
    ///
    /// Wait for flow control credits exchange
    ///
    WaitForDataLinkLayerLinkActive (This, PciePort);
  } else {
    DEBUG ((DEBUG_INFO, "Error 0x%x.\n", Status));
  }
  return Status;
}
