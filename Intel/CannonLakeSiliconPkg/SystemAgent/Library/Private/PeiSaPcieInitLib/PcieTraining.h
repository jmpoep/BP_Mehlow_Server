/** @file
  Header file for PcieTraining Initialization Driver.

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

@par Specification Reference:
**/
#ifndef _PCIETRAINING_H_
#define _PCIETRAINING_H_

#include <Library/PciSegmentLib.h>
#include <Library/TimerLib.h>
#include <Library/PeiServicesLib.h>
#include <SaPolicyCommon.h>
#include <Private/Library/SaPcieInitLib.h>
#include <Private/SaPegHob.h>
#include <Library/SaPlatformLib.h>

///
/// Data structures
///

typedef struct {
  INT8  Depth;
  UINT8 Step;
  UINT8 ReportedMargin;
  UINT8 DoubleMargin;
} JITTER_SETTING;

#define JITTER_LENGTH                           25
#define JITTER_MARGIN_INITIAL_OFFSET            1
#define VOC_MARGIN_INITIAL_OFFSET               1
#define MARGIN_CONVERGANCE_ALLOWED_DELTA        100
#define MARGIN_CONVERGANCE_MIN_MATCHES          2
#define MARGIN_CONVERGANCE_MAX_REPEATS          30
#define JITTER_TEST_DEFAULT_DWELL_TIME          3 * STALL_ONE_MILLI_SECOND
#define JITTER_TEST_DEFAULT_ERROR_TARGET        2
#define JITTER_MASK                             400
#define VOC_TEST_DEFAULT_DWELL_TIME             10 * STALL_ONE_MILLI_SECOND
#define VOC_TEST_DEFAULT_ERROR_TARGET           2
#define VOC_HIGH_SIDE_MASK                      170
#define VOC_LOW_SIDE_MASK                       -170
#define VOC_CORNERS_TEST_PASS_CREDIT            3000
#define VOC_CODES_LENGTH                        89
#define VOC_X1_MAX                              12
#define VOC_X2_MAX                              4
#define VOC_X4_MAX                              8

typedef struct {
  INT16   Offset;
  UINT16  VocCode;
} VOC_CODE;

typedef struct {
  UINT8     Lane;
  UINT16    InitialDs0Code;
  VOC_CODE  Ds0Codes[VOC_CODES_LENGTH];
  UINT16    InitialDs1Code;
  VOC_CODE  Ds1Codes[VOC_CODES_LENGTH];
  INT16     Ds0MaxUpMargin;
  INT16     Ds0MaxDownMargin;
  INT16     Ds1MaxUpMargin;
  INT16     Ds1MaxDownMargin;
  INT16     MaxUpMargin;
  INT16     MaxDownMargin;
} VOC_STATE;

///
///
#ifndef CPU_CFL
#define SA_PEI_MONITOR_OFFSET                 0xFED83000
#else
#define SA_PEI_MONITOR_OFFSET                 0xFED85000
#endif

///
/// Register Definitions
///
#define B_SA_PEG_LTSSMC_WIDTH_MASK            0xFFFFFFE0

#define B_SA_PEG_REUT_PH_CTR_PHYRESET_MASK    0x1
#define B_SA_PEG_REUT_PH_CTR_RESETMOD_MASK    0x2

#define R_SA_PEG_REUT_PH1_PIS_OFFSET          0x464
#define B_SA_PEG_REUT_PH1_PIS_ST_MASK         0x3F
#define B_SA_PEG_REUT_PH1_PIS_ST_STEP         0x8

#define R_SA_PEG_DS0_LOADBUS_OFFSET           0x31
#define R_SA_PEG_DS1_LOADBUS_OFFSET           0x32
#define B_SA_PEG_VOC_LOADBUS_MASK             0xFFF

#define B_SA_PEG_BCTRL_SRESET_MASK            BIT6


///
/// Function Prototypes
///


/**
  Runs a Margin Test on the specified root ports.  The MarginData parameter must
  be an array with capacity of PciePortsLength elements or more.

  @param[in]  This                          - Low level function table
  @param[in]  PciePorts                     - PCIe Root Ports to margin
  @param[in]  PciePortsLength               - Length of the PciePorts array
  @param[in]  InputParameters               - SW EQ Input Parameters
  @param[in]  MonitorPort                   - Monitor Port
  @param[in]  MarginTest                    - Type of Margin Test to Run
  @param[out] MarginData                    - Margin Data, must be array of size >= PciePortsLength
  @param[out] DeferredPlatformResetRequired - A platform reset is needed after saving Eq data to NVRAM

  @retval EFI_SUCCESS            - Margin Data Generated Successfully
  @retval EFI_INVALID_PARAMETER  - Parameter issue found
**/
EFI_STATUS
EFIAPI
RunMarginTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  UINT32                            MonitorPort,
  IN  MARGIN_TEST_TYPE                  MarginTest,
  OUT PCIE_SWEQ_PORT_OUTPUT             *MarginData,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  );

EFI_STATUS
LaneLevelJitterTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  UINT32                            MonitorPort,
  IN  UINT8                             *LaneList,
  IN  UINT8                             LaneListLength,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  IN  BOOLEAN                           TxJitterTest,
  OUT INT32                             *MarginData,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  );

EFI_STATUS
PortLevelJitterTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  UINT32                            MonitorPort,
  IN  UINT8                             *LaneList,
  IN  UINT8                             LaneListLength,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  OUT INT32                             *MarginData,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  );

EFI_STATUS
VocCornersMarginTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  UINT32                            MonitorPort,
  IN  UINT8                             *LaneList,
  IN  UINT8                             LaneListLength,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth,
  OUT INT32                             *MarginData,
  OUT BOOLEAN                           *DeferredPlatformResetRequired
  );


UINT32
SaPciePointTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINTN                             DwellTime,
  IN  UINT32                            MonitorPort,
  IN  UINT32                            InitialRecoveryCount
  );

INT32
CalculateMarginDifference (
  IN INT32                   Margin1,
  IN INT32                   Margin2
  );

INT32
InterpolateMargin (
  IN UINT32 ErrorTarget,
  IN UINT32 CurrentErrorCount,
  IN UINT32 PreviousErrorCount,
  IN INT32  FailingPoint
  );

UINT32
NaturalLog (
  IN UINT32 Input
  );

EFI_STATUS
SetJitterTolerance (
  IN UINT8   *LaneList,
  IN UINT8   LaneListLength,
  IN UINT8   ReportedMargin
  );

VOID
EnableTxJitterInjection (
  IN UINT8    Lane,
  IN BOOLEAN  EnableTxJitter
  );

VOID
ConfigureTxJitterMux (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             Lane
  );

/**
 Enable VOC margining

  @param[in] EnableVoc           - TRUE to enable VOC, FALSE otherwise

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
SetVocEnable (
  IN  BOOLEAN   EnableVoc
  );

/**
 Collects and computes data needed to perform VOC margining on a specific lane

  @param[in]  Lane               - Lane to prepare for VOC margining
  @param[out] VocState           - VOC margining data

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
InitializeVocMargining (
  IN  UINT8     Lane,
  OUT VOC_STATE *VocState
  );

/**
 Stops all VOC stimulus on a lane and returns it to normal operation

  @param[in] VocState            - VOC margining data
**/
VOID
FinalizeVocMargining (
  IN VOC_STATE *VocState
  );

/**
 Set VOC stimulus by computing DS0 & 1 DAC/Value pairs and programming them

  @param[in] ReportedMargin      - VOC stimulus level
  @param[in] VocState            - VOC margining data
**/
EFI_STATUS
SaPcieSetVoc (
  IN INT32     ReportedMargin,
  IN VOC_STATE *VocState
  );

/**
 Sets VOC stimulus to a target mask

  @param[in]  VocMaskValue       - VOC mask
  @param[in]  VocState           - VOC margining data

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
SaPcieSetVocToMaskValue (
  IN INT32     VocMaskValue,
  IN VOC_STATE *VocState
  );

#endif
