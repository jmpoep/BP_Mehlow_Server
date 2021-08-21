/** @file
  SA PCIe Initialization Library header file

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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
#ifndef _SA_PEG_LOW_LEVEL_H_
#define _SA_PEG_LOW_LEVEL_H_

#include <Private/Library/SaPcieInitLib.h>
#include <SaAccess.h>
#include <PchAccess.h>
#include <CpuRegs.h>
#include <Library/CpuPlatformLib.h>

#define LANE_STEP     0x10
#define BUNDLE_STEP   0x20

typedef struct {
  UINT32 RegOffset;
  UINT32 Value;
} CREDIT_CONTROL_INIT_ENTRY;

///
///

/**
  Invert the lane if LaneReversal bit is set based on PEG X16 width for PEG0 or PEGX4 for PEG3

  @param[in]  PegDev              - Device Number
  @param[in]  Lane                - Lane to potentially invert
  @param[in]  LaneReversal        - LaneReversal bit

  @retval     Lane number compensated for LaneReversal
**/
UINT8
ReverseLane (
  IN  UINT8 PegDev,
  IN  UINT8 Lane,
  IN  UINT8 LaneReversal
  );

/**
  Translate an array of lane numbers in to an array of bundle numbers

  @param[in]  LaneList            - Lanes to translate to bundles
  @param[in]  LaneListLength      - Length of LaneList array
  @param[out] BundleList          - Resulting array of bundle numbers
  @param[out] BundleListLength    - Length of BundleList array
**/
VOID
GetBundleList (
  IN  UINT8 *LaneList,
  IN  UINT8 LaneListLength,
  OUT UINT8 *BundleList,
  OUT UINT8 *BundleListLength
  );

/**
  Set Load Bus

  @param[in] DmiBar        - DMIBAR address
  @param[in] Dev           - Device Number
  @param[in] Lane          - Number of Lane
  @param[in] LoadSel       - Load selection value
  @param[in] LoadData      - Load Data
**/
VOID
SetLoadBus (
  IN UINT32 DmiBar,
  IN UINTN  Dev,
  IN UINTN  Lane,
  IN UINT32 LoadSel,
  IN UINT32 LoadData
  );

/**
  Get monitor bus from the lane selected

  @param[in] DmiBar        - DMIBAR address
  @param[in] Dev           - Device number
  @param[in] Lane          - Number of Lane
  @param[in] LoadSel       - Load selection value

  @retval UINT32 - Load bus address
**/
UINT32
GetMonBus (
  IN UINT32 DmiBar,
  IN UINTN  Dev,
  IN UINTN  Lane,
  IN UINT32 LoadSel
  );

/**
  Determine if all enabled PEG root ports are present in the given list of ports

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array

  @retval TRUE  - All enabled root ports are present
  @retval FALSE - Some or all of the enabled root ports are missing
**/
BOOLEAN
AllRootPortsSpecified (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  );

///
///

/**
  Get Full Swing value for EndPoint Transmitter

  @param[in]  This                        - Low level function table
  @param[in]  PegDev                      - Device Number
  @param[in]  Lane                        - Physical Lane Number
  @param[out] FullSwing                   - Full Swing value
**/
VOID
EFIAPI
GetLinkPartnerFullSwing (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDev,
  IN  UINT8                             Lane,
  OUT UINT8                             *FullSwing
  );

/**
  Sets the Phase 3 Hijack Equalization Coefficients

  @param[in]  This                        - Low level function table
  @param[in]  PegDev                      - Device Number
  @param[in]  Lane                        - Physical Lane Number
  @param[in]  PreCursor                   - Computed Pre-Cursor
  @param[in]  Cursor                      - Computed Cursor
  @param[in]  PostCursor                  - Computed Post-Cursor
**/
VOID
EFIAPI
SetPartnerTxCoefficients (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDev,
  IN  UINT8                             Lane,
  IN  UINT8                             *PreCursor,
  IN  UINT8                             *Cursor,
  IN  UINT8                             *PostCursor
  );

///
///

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
  );

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
  );

///
///

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
  );

/**
  Resets the endpoint connected to the given root port by directly pulsing the
  PERST# signal

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  InputParameters             - SW EQ Input Parameters

  @retval EFI_SUCCESS            - Pulsed PERST# and retrained link successfully
  @retval EFI_UNSUPPORTED        - Didn't assert GPIO
  @retval EFI_INVALID_PARAMETER  - Didn't assert GPIO
  @retval EFI_TIMEOUT            - Link did not train after pulsing PERST#
**/
EFI_STATUS
EFIAPI
SklResetEndpointPerst (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters
  );

///
///
/**
  Open port for monitor

  @param[in]  This                          - Low level function table

  @retval Monitor Port
**/
UINT32
EFIAPI
SklOpenMonitor (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  );

/**
  Close port for monitor

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
**/
VOID
EFIAPI
SklCloseMonitor (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN UINT32                             MonitorPort
  );

/**
  Get Current Error Count

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
  @param[in]  PciePort                      - PCIe Root Port
**/
UINT32
EFIAPI
SklSaPcieGetErrorCount (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT32                            MonitorPort,
  IN  PCIE_PORT_INFO                    *PciePort
  );

/**
  Clear Current Error Count for all Root Ports

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
**/
VOID
EFIAPI
SklSaPcieClearErrorCount (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT32                            MonitorPort
  );

///
///

/**
  This function gets the low level functions for the SA PCIe interface

  @param[in]  PCIE_PEI_PREMEM_CONFIG      - PciePeiPreMemConfig
  @param[out] SaPcieLowLevelFunctionCalls - Table of function calls for SA PEG

  @retval EFI_SUCCESS - Table of function calls returned successfully
**/
EFI_STATUS
GetSklPegLowLevelFunctionCalls (
  IN       PCIE_PEI_PREMEM_CONFIG             *PciePeiPreMemConfig,
  OUT      PCIE_SI_LOW_LEVEL_FUNCTION_CALLS   *SaPcieLowLevelFunctionCalls
  );

/**
  This function gets the private data for the SA PCIe low level functions

  @param[in]  PCIE_PEI_PREMEM_CONFIG   - PciePeiPreMemConfig
  @param[in]  SA_MISC_PEI_CONFIG       - MISC config block from SA Policy PPI
  @param[out] SaPciePrivateData        - Table of function calls for SA PEG

  @retval EFI_SUCCESS - Table of function calls returned successfully
**/
EFI_STATUS
GetSklPegPrivateData (
  IN       PCIE_PEI_PREMEM_CONFIG             *PciePeiPreMemConfig,
  IN       SA_MISC_PEI_PREMEM_CONFIG          *MiscPeiPreMemConfig,
  OUT      SA_PCIE_PRIVATE_FUNCTION_CALLS     *SaPciePrivateData
  );

#ifndef CPU_CFL
/**
  Reads, modifies and writes to PCODE mail box as per the input.

  @param[in]  CrOffset                    - Config Register Offset
  @param[in]  Data32And                   - Data to Clear
  @param[in]  Data32Or                    - Data to Set
**/
EFI_STATUS
PcodeMailboxReadThenWrite (
  IN  UINT32           CrOffset,
  IN  UINT32           Data32And,
  IN  UINT32           Data32Or,
  IN  CONTROLLER_TYPE  ControllerType
  );
#endif
#endif  ///< _SA_PEG_LOW_LEVEL_H_
