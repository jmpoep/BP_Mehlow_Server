/** @file
  SA PCIe Initialization Library header file

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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
  - 1) PCI Express Base Specification, www.pcisig.com
**/
#ifndef _SA_PCIE_INIT_LIB_H_
#define _SA_PCIE_INIT_LIB_H_

#include <SaPolicyCommon.h>
#include <Private/Library/GraphicsInitLib.h>
#include <Private/Library/PcieInitLib.h>

#define PEG_AUTO              0
#define PEG_GEN1              1
#define PEG_GEN2              2
#define PEG_GEN3              3

#define DMI_AUTO              0
#define DMI_GEN1              1
#define DMI_GEN2              2
#define DMI_GEN3              3

#define PEG_MPS_AUTO          0xFF
#define PEG_MPS_128           0
#define PEG_MPS_A256          1

#define PH3_METHOD_AUTO       0x0
#define PH3_METHOD_HWEQ       0x1
#define PH3_METHOD_SWEQ       0x2
#define PH3_METHOD_STATIC     0x3
#define PH3_METHOD_DISABLED   0x4

#define MAX_PRESETS           9
#define BEST_PRESETS          4

//
// The PEG Disable Mask BIT0 ~ BIT3 for PEG0 ~ PEG3
//
#define V_PEG_DISABLE_MASK    0x0F
#define B_PEG0_DISABLE_MASK   BIT0
#define B_PEG1_DISABLE_MASK   BIT1
#define B_PEG2_DISABLE_MASK   BIT2
#define B_PEG3_DISABLE_MASK   BIT3

/**
  Read the SA PCIE policy to see if PCIe IMR is enabled.

  @retval TRUE             PCIe IMR is enabled
  @retval FALSE            PCIe IMR is disabled
**/
BOOLEAN
SaIsPcieImrEnabled (
  VOID
  );

/**
  Read the SA PCIE policy to get CPU PCIe IMR Size.

  @retval PcieImrSize      Size of PCIe Imr in MB
**/
UINT32
SaGetPcieImrSize (
  VOID
  );

/**
Program flow control credits for configurations where PEG is disabled or x16x0x0.
**/
VOID
FlowControlCreditProgrammingNoPegLib (
  VOID
  );

/**
  Initialize the SA PciExpress in PEI

  @param[in] IN SA_MISC_PEI_PREMEM_CONFIG     MiscPeiPreMemConfig to access Platform relevant information
  @param[in] IN PCIE_PEI_PREMEM_CONFIG        PciePeiPreMemConfig to access the PCIe Config related information
**/
VOID
PciExpressInit (
  IN       SA_MISC_PEI_PREMEM_CONFIG    *MiscPeiPreMemConfig,
  IN       PCIE_PEI_PREMEM_CONFIG       *PciePeiPreMemConfig
  );

/**
  CheckAndInitializePegVga:  Check if PEG card is present and configure accordingly

  @param[in, out] PrimaryDisplay          - Primary Display - default is IGD
  @param[in]      PrimaryDisplaySelection - Primary display selection from BIOS Setup
  @param[in, out] PegMmioLength           - Total PEG MMIO length on all PEG ports
  @param[in]      OpRomScanTempMmioBar    - Temporary BAR to MMIO map OpROMs during VGA scanning
  @param[in]      OpRomScanTempMmioLimit  - Limit address for OpROM MMIO range
  @param[in]      ScanForLegacyOpRom      - TRUE to scan for legacy only VBIOS, FALSE otherwise
  @param[out]     FoundLegacyOpRom        - If legacy only VBIOS found, returns TRUE
**/
VOID
CheckAndInitializePegVga (
  IN OUT   DISPLAY_DEVICE          *PrimaryDisplay,
  IN       UINT8                   PrimaryDisplaySelection,
  IN OUT   UINT32                  *PegMmioLength,
  IN       UINT32                  OpRomScanTempMmioBar,
  IN       UINT32                  OpRomScanTempMmioLimit,
  IN       BOOLEAN                 ScanForLegacyOpRom,
  OUT      BOOLEAN                 *FoundLegacyOpRom
  );

///
///

/**
  Determines if SA Policy enables Gen3 on any of the PEG root ports

  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array

  @retval TRUE  - At least 1 root port has Gen3 enabled
  @retval FALSE - otherwise
**/
BOOLEAN
SaPolicyEnablesGen3 (
  IN  PCIE_PEI_PREMEM_CONFIG     *PciePeiPreMemConfig,
  IN  PCIE_PORT_INFO             *PciePorts,
  IN  UINT8                      PciePortsLength
  );

/**
  Determines the SA Policy Phase 3 Equalization Method for a given root port

  @param[in]  PciePorts                   - PCIe Root Ports to GetEqPhase3Method
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval Equalization Phase 3 Method
**/
UINT8
SaPolicyGetEqPhase3Method (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy enabled Phase 2 Equalization on a given root port

  @param[in]  PciePorts                   - PCIe Root Ports Pointer
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval TRUE  - Equalization Phase 2 is enabled
  @retval FALSE - Equalization Phase 2 is disabled
**/
BOOLEAN
SaPolicyGetEqPhase2Enable (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy enables Phase 3 Software Equalization on a given root port

  @param[in]  PciePorts                   - Pointer to PCIe Root Ports list
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval TRUE  - Phase 3 Software Equalization is enabled
  @retval FALSE - Phase 3 Software Equalization is disabled
**/
BOOLEAN
SaPolicySwEqEnabledOnPort (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy enables Phase 3 Software Equalization any of the PEG ports

  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array

  @retval TRUE  - At least 1 root port has Phase 3 Software Equalization enabled
  @retval FALSE - Otherwise
**/
BOOLEAN
SaPolicyEnablesSwEq (
  IN  PCIE_PEI_PREMEM_CONFIG   *PciePeiPreMemConfig,
  IN  PCIE_PORT_INFO           *PciePorts,
  IN  UINT8                    PciePortsLength
  );

/**
  Gets the SA Policy defined max link width for a given root port

  @param[in]  PegPort                     - Root Port Number
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval Max link width
**/
UINT8
SaPolicyGetPegMaxLinkWidth (
  IN PCIE_PORT_INFO          *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG  *PciePeiPreMemConfig
  );

/**
  Gets the SA Policy defined max link speed for a given root port

  @param[in]  PciePorts                   - Pointer to PCIe Root Ports list
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval Max link speed
**/
UINT8
SaPolicyGetPegMaxLinkSpeed (
  IN PCIE_PORT_INFO          *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG  *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy forces a given root port to always be enabled

  @param[in]  PciePorts                   - Pointer to PCIe Root Ports list
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval TRUE  - SA Policy forces the root port to be enabled
  @retval FALSE - SA Policy does not force the root port to be enabled
**/
BOOLEAN
SaPolicyForceEnablesPort (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy forces a given root port to always be disabled

  @param[in]  PciePorts                   - Pointer to PCIe Root Ports list
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval TRUE  - SA Policy forces the root port to be disabled
  @retval FALSE - SA Policy does not force the root port to be disabled
**/
BOOLEAN
SaPolicyForceDisablesPort (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if SA Policy allows unused lanes to be powered down

  @param[in]  PciePorts                   - Pointer to PCIe Root Ports list
  @param[in]  PciePeiPreMemConfig         - SA Policy PCIe Config Block

  @retval 0x1 - SA Policy allows unused lanes to be powered down
  @retval 0x0 - SA Policy disallows unused lanes to be powered down
**/
UINT8
SaPolicyGetPowerDownUnusedLanes (
  IN PCIE_PORT_INFO         *PciePorts,
  IN PCIE_PEI_PREMEM_CONFIG *PciePeiPreMemConfig
  );

/**
  Determines if the PCIe root ports are capable of PCIe 3.0 speed

  @param[in]  This                        - Low level function table

  @retval TRUE - PCIe Root Ports are PCIe 3.0 capable
  @retval FALSE - otherwise
**/
typedef
BOOLEAN
(EFIAPI *SA_PCIE_IS_GEN3_CAPABLE)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  );

/**
  Performs AFE and RP programming that needs to be done prior to enabling the
  PCIe link and allowing it to train to active state for the first time.

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
typedef
VOID
(EFIAPI *SA_PCIE_PREDETECTION_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  );

/**
  Programs the PCIe/DMI recipe.

  @param[in]  DmiBar                      - DMIBAR
  @param[in]  MchBar                      - MCHBAR
  @param[in]  Gen3CtlePeaking             - Array of CTLE Peaking values to program per bundle
  @param[in]  Gen3CtlePeakingLength       - Length of the Gen3CtlePeaking array
  @param[in]  Gen3RxCtleOverride          - RxCTLE override configuration
  @param[in]  ProgramDmiRecipe            - Set to TRUE to program DMI, FALSE to program PCIe
**/
typedef
VOID
(EFIAPI *SA_PCIE_DMI_RECIPE)(
  IN  UINT64                            DmiBar,
  IN  UINT64                            MchBar,
  IN  UINT8                             *Gen3CtlePeaking,
  IN  UINT8                             Gen3CtlePeakingLength,
  IN  UINT8                             Gen3RxCtleOverride,
  IN  BOOLEAN                           ProgramDmiRecipe
  );

/**
  Configure PCIe Max Speed and Width

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array
  @param[in]  Gen3Capable                 - PEG is Gen3 capable
**/
typedef
VOID
(EFIAPI *SA_PCIE_CONFIGURE_MAX_SPEED_WIDTH)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  BOOLEAN                           Gen3Capable
  );

/**
  Enable RxCEM Loopback (LPBK) Mode

  @param[in]  This                        - Low level function table
  @param[in]  RxCemLoopbackLane           - Lane to use for RxCEM Testing
  @param[in]  PciePorts                   - PCIe Root Ports
**/
typedef
VOID
(EFIAPI *SA_PCIE_ENABLE_RXCEM_LOOPBACK_MODE)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             RxCemLoopbackLane,
  IN  PCIE_PORT_INFO                    *PciePorts
  );

/**
  Clear DEFER_OC and allow the PCIe controllers to begin training

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to Clear Defer OC
**/
typedef
VOID
(EFIAPI *SA_PCIE_CLEAR_DEFER_OC)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts
  );

/**
  Program VRef Rx Detect

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to Clear Defer OC
**/
typedef
VOID
(EFIAPI *SA_PCIE_PROGRAM_VREF_RX_DETECT)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts
  );

/**
  Set DisableAutoSpeedUp bit

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  DisableAutoSpeedUp          - New value for DisableAutoSpeedUp
**/
typedef
VOID
(EFIAPI *SA_PCIE_SET_DISABLE_AUTO_SPEED_UP)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  BOOLEAN                           DisableAutoSpeedUp
  );

/**
  Performs any additional equalization programming that needs to be done after
  initial link training and endpoint detection

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to program Phase2 for
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
typedef
VOID
(EFIAPI *SA_PCIE_POST_DETECTION_EQ_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  );

/**
  Get Full Swing value for EndPoint Transmitter

  @param[in]  This                        - Low level function table
  @param[in]  PegDev                      - Device Number
  @param[in]  Lane                        - Physical Lane Number
  @param[out] FullSwing                   - Full Swing value
**/
typedef
VOID
(EFIAPI *SA_PCIE_GET_LINK_PARTNER_FULL_SWING)(
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
typedef
VOID
(EFIAPI *SA_PCIE_SET_PARTNER_TX_COEFFICIENTS)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDev,
  IN  UINT8                             Lane,
  IN  UINT8                             *PreCursor,
  IN  UINT8                             *Cursor,
  IN  UINT8                             *PostCursor
  );

/**
 This function programs Equalization Phase 2/3 Bypass

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to sampler calibrate
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
typedef
VOID
(EFIAPI *SA_PCIE_EQ_PH2_PH3_BYPASS_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  );

/**
  Program EQ Phase1 preset value

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Port
  @param[in]  Direction                   - 0 = Root Port, 1 = End Point
  @param[in]  PresetValue                 - Preset value to program
  @param[in]  LogicalLane                 - Logical Lane to be configured
**/
typedef
VOID
(EFIAPI *SA_PCIE_PROGRAM_PHASE1_PRESET)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             Direction,
  IN  UINT8                             PresetValue,
  IN  UINT8                             LogicalLane
  );

/**
  Power Down Unused Lanes on the given PCIe Root Port

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
**/
typedef
VOID
(EFIAPI *SA_PCIE_POWER_DOWN_UNUSED_LANES)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  );

/**
  Power Down All Lanes on the given PCIe Root Port

  @param[in]  This                        - Low level function table
  @param[in]  PegDevice                   - The device number of the PEG port to power down
  @param[in]  PegFunction                 - The function number of the PEG port to power down
**/
typedef
VOID
(EFIAPI *SA_PCIE_POWER_DOWN_ALL_LANES)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDevice,
  IN  UINT8                             PegFunction
  );

/**
  Sets the link width

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  LinkWidth                   - Desired Link Width
**/
typedef
VOID
(EFIAPI *SA_PCIE_SET_LINK_WIDTH)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             LinkWidth
  );

/**
  Additional Programming steps that need to be performed post endpoint detection

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
**/
typedef
VOID
(EFIAPI *SA_PCIE_POST_DETECTION_ADDITIONAL_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  );

/**
  Disable Unused PEG Controllers

  @param[in]  This                        - Low level function table
  @param[in]  PegDisableMask              - Bitmap of controllers to disable by function number
  @param[in]  PciePort                    - PCIe Root Port
**/
typedef
VOID
(EFIAPI *SA_PCIE_DISABLE_UNUSED_PCIE_CONTROLLERS)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDisableMask,
  IN  PCIE_PORT_INFO                    *PciePort
  );

/**
  Perform any flow control credit programming that is needed

  @param[in]  This                        - Low level function table
  @param[in]  PegDisableMask              - Bitmap of controllers to disable by function number
**/
typedef
VOID
(EFIAPI *SA_PCIE_FLOW_CONTROL_CREDIT_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDisableMask
  );

#ifdef CPU_CFL
/**
  Disable or enable Icomp for a root port
  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  Disable                     - TRUE to Disable, FALSE to Enable
**/
typedef
VOID
(EFIAPI *SA_PCIE_SET_ROOT_PORT_ICOMP_DISABLE)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  BOOLEAN                           Disable
  );
#endif

#ifndef CPU_CFL
/**
  Program RXSQEXCTL FUSE values
  @param[in]  Value                       - Value to be programmed
**/
typedef
VOID
(EFIAPI *SA_PCIE_SET_RXSQEXCTL_VALUE)(
  IN  UINT32                             Value
  );
#endif

/**
  Program the Spine Clock Gating feature according to the configuration

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - Root Port to check for VC0 negotiation complete
**/
typedef
VOID
(EFIAPI *SA_PCIE_SPINE_CLOCK_GATING_PROGRAMMING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  );

/**
  Disable Spread Spectrum clocking according to the configuration

  @param[in]  This                        - Low level function table
**/
typedef
VOID
(EFIAPI *SA_PCIE_DISABLE_SPREAD_SPECTRUM_CLOCKING)(
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  );

typedef struct {
  UINT64                                        DmiBar;
  UINT64                                        MchBar;
#ifdef CPU_CFL
  UINT32                                        GdxcBar;
  SA_PCIE_SET_ROOT_PORT_ICOMP_DISABLE           SetRootPortIcompDisable;
#else
  SA_PCIE_SET_RXSQEXCTL_VALUE                   SetRootPortRXSQEXCTL;
#endif
  PCIE_PEI_PREMEM_CONFIG                        *PciePeiPreMemConfig;
  SA_PCIE_IS_GEN3_CAPABLE                       IsGen3Capable;
  SA_PCIE_PREDETECTION_PROGRAMMING              PreDetectionProgramming;
  SA_PCIE_DMI_RECIPE                            PcieDmiRecipe;
  SA_PCIE_CONFIGURE_MAX_SPEED_WIDTH             ConfigureMaxSpeedWidth;
  SA_PCIE_ENABLE_RXCEM_LOOPBACK_MODE            EnableRxCemLoopbackMode;
  SA_PCIE_PROGRAM_VREF_RX_DETECT                ProgramVRefRxDet;
  SA_PCIE_CLEAR_DEFER_OC                        ClearDeferOc;
  SA_PCIE_SET_DISABLE_AUTO_SPEED_UP             SetDisableAutoSpeedUp;
  SA_PCIE_POST_DETECTION_EQ_PROGRAMMING         PostDetectionEqProgramming;
  SA_PCIE_GET_LINK_PARTNER_FULL_SWING           GetLinkPartnerFullSwing;
  SA_PCIE_SET_PARTNER_TX_COEFFICIENTS           SetPartnerTxCoefficients;
  SA_PCIE_EQ_PH2_PH3_BYPASS_PROGRAMMING         EqPh2Ph3BypassProgramming;
  SA_PCIE_PROGRAM_PHASE1_PRESET                 ProgramPhase1Preset;
  SA_PCIE_POWER_DOWN_UNUSED_LANES               PowerDownUnusedLanes;
  SA_PCIE_POWER_DOWN_ALL_LANES                  PowerDownAllLanes;
  SA_PCIE_SET_LINK_WIDTH                        SetLinkWidth;
  SA_PCIE_POST_DETECTION_ADDITIONAL_PROGRAMMING PostDetectionAdditionalProgramming;
  SA_PCIE_DISABLE_UNUSED_PCIE_CONTROLLERS       DisableUnusedPcieControllers;
  SA_PCIE_FLOW_CONTROL_CREDIT_PROGRAMMING       FlowControlCreditProgramming;
  SA_PCIE_SPINE_CLOCK_GATING_PROGRAMMING        SpineClockGatingProgramming;
  SA_PCIE_DISABLE_SPREAD_SPECTRUM_CLOCKING      DisableSpreadSpectrumClocking;
} SA_PCIE_PRIVATE_FUNCTION_CALLS;

/**
  This function gets the table of function calls for the SA PEG interface

  @param[in]  PciePeiPreMemConfig         - SA Policy PPI
  @param[out] SaPcieLowLevelFunctionCalls - Table of function calls for SA PEG

  @retval EFI_SUCCESS                     - Table of function calls returned successfully
**/
EFI_STATUS
GetSaPcieLowLevelFunctionCalls (
  IN  PCIE_PEI_PREMEM_CONFIG                  *PciePeiPreMemConfig,
  OUT PCIE_SI_LOW_LEVEL_FUNCTION_CALLS        *SaPcieLowLevelFunctionCalls
  );

/**
  This function gets the private data for the SA PCIe low level functions

  @param[in]  PciePeiPreMemConfig                  - PCIe config block from SA Policy PPI
  @param[in]  MiscPeiPreMemConfig                  - MISC config block from SA Policy PPI
  @param[out] SaPciePrivateData           - Table of function calls for SA PEG

  @retval EFI_SUCCESS                     - Table of function calls returned successfully
**/
EFI_STATUS
GetSaPciePrivateData (
  IN  PCIE_PEI_PREMEM_CONFIG                             *PciePeiPreMemConfig,
  IN  SA_MISC_PEI_PREMEM_CONFIG                          *MiscPeiPreMemConfig,
  OUT SA_PCIE_PRIVATE_FUNCTION_CALLS          *SaPciePrivateData
  );

/**
  PowerDownUnusedBundles: Program the PEG BundleSpare registers for power on sequence [PowerOff unused bundles for PEGs]

  @param[in] PegFunc              - Points to PEG0/PEG1/PEG2/...
  @param[in] HwStrap              - Points to PEG configuration information [x16_x0_x0/x8_x8_x0/x8_x4_x4/...]
  @param[in] BndlPwrdnCount       - Points to how many bundles are unused and should be powered down
**/
VOID
PowerDownUnusedBundles (
  IN UINT8                     PegFunc,
  IN UINT8                     HwStrap,
  IN UINT8                     BndlPwrdnCount
  );

/**
  PCI Express Post Memory Initialization.

  @param[in] PCIE_PEI_PREMEM_CONFIG    *PciePeiPreMemConfig
  @param[in] PCIE_PEI_CONFIG           *PciePeiConfig

  @retval EFI_SUCCESS     - Pci Express successfully started and ready to be used
  @exception EFI_UNSUPPORTED - Pci Express can't be initialized
**/
EFI_STATUS
PciExpressInitPostMem (
  IN PCIE_PEI_PREMEM_CONFIG    *PciePeiPreMemConfig,
  IN PCIE_PEI_CONFIG           *PciePeiConfig
  );

/**
    Perform Egress Port 0 Initialization.

    @param[in] EgressPortBar   - EPBAR Address

    @retval EFI_SUCCESS        - Egress Port 0 initialization successed.
**/
EFI_STATUS
Cid1EgressPort0Init (
  IN  UINT32      EgressPortBar
  );

/**
    Conditionally perform PEG Port Initialization.

    @param[in]   PCIE_PEI_PREMEM_CONFIG      PciePeiPreMemConfig,
    @param[in]   PCIE_PEI_CONFIG             PciePeiConfig,

    @retval EFI_SUCCESS            - PEG Port initialization successed.
**/
EFI_STATUS
Cid1PegPortInit (
    IN       PCIE_PEI_PREMEM_CONFIG     *PciePeiPreMemConfig,
    IN       PCIE_PEI_CONFIG            *PciePeiConfig
  );

/**
    DMI Port Initialization for both CID1 (Port 1 in MCH) and CID2 (Port 0 in PCH).

    @param[in] DmiBar                   - DMIBAR Address
    @param[in] PCIE_PEI_PREMEM_CONFIG   *PciePeiPreMemConfig

    @retval EFI_SUCCESS            - DMI Port initialization successed.
**/
EFI_STATUS
Cid1Cid2DmiPortInit (
  IN  UINT32                       DmiBar,
  IN PCIE_PEI_CONFIG               *PciePeiConfig
  );

/**
    Perform Root Complex Topology Initialization for CID1.

    @param[in] EgressPortBar     - EPBAR Address
    @param[in] DmiBar            - DMIBAR Address

    @retval EFI_SUCCESS          - Root Complex topology initialization for CID1 successed.
**/
EFI_STATUS
Cid1TopologyInit (
  IN  UINT32       EgressPortBar,
  IN  UINT32       DmiBar
  );

/**
  Perform Root Complex Topology Initialization for CID2.
  Note: This sequence follows PCH BIOS specification Ver 0.5 section 8.3
        Root Complex Topology Programming

  @param[in] DmiBar            - DMIBAR Address

  @retval EFI_SUCCESS          - Root Complex topology initialization for CID2 successed.
**/
EFI_STATUS
Cid2TopologyInit (
  IN UINT32   DmiBar
  );


/**
  Additional PEG Programming Steps as in SA BIOS spec

  @param[in] Bus         - Pci Bus Number
  @param[in] pegDev      - Pci Device Number
  @param[in] pegFn       - Pci Func Number
  @param[in] PCIE_PEI_PREMEM_CONFIG    *PciePeiPreMemConfig
  @param[in] PCIE_PEI_CONFIG           *PciePeiConfig
**/
VOID
AdditionalPEGProgramStepsBeforeASPM (
  IN  UINT8                          Bus,
  IN  UINT8                          pegDev,
  IN  UINT8                          pegFn,
  IN  PCIE_PEI_PREMEM_CONFIG         *PciePeiPreMemConfig,
  IN PCIE_PEI_CONFIG                 *PciePeiConfig
  );

/**
  Disable Unused PEG Controllers

  @param[in]  This                        - Low level function table
  @param[in]  PegDisableMask              - Bitmap of controllers to disable by function number
  @param[in]  PciePort                    - PCIe Root Port
**/
VOID
EFIAPI
SklDisableUnusedPcieControllers (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDisableMask,
  IN  PCIE_PORT_INFO                    *PciePort
  );

#endif
