/** @file
  PCIe Initialization Library header file

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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
#ifndef _PCIE_LOW_LEVEL_H_
#define _PCIE_LOW_LEVEL_H_

#include <Private/Library/PcieInitLib.h>

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
  );

/**
  Resets the endpoint connected to the given root port by directly pulsing the
  PERST# signal

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  InputParameters             - SW EQ Input Parameters
  @param[in]  AssertPerst                 - TRUE to assert PERST#, FALSE to deassert

  @retval EFI_SUCCESS            - Pulsed PERST# and retrained link successfully
  @retval EFI_UNSUPPORTED        - Didn't assert GPIO
  @retval EFI_INVALID_PARAMETER  - Didn't assert GPIO
  @retval EFI_TIMEOUT            - Link did not train after pulsing PERST#
**/
EFI_STATUS
EFIAPI
PcieSetPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  BOOLEAN                           AssertPerst
  );

#endif  ///< _PCIE_LOW_LEVEL_H_
