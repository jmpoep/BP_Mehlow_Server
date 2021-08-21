/** @file
  This driver trains the PEG interface.

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
#include "PcieTraining.h"

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
  )
{
  EFI_STATUS Status;
  UINT8      *LaneList;
  UINT8      LaneIndex;
  UINT8      PortIndex;
  UINT8      MarginDataIndex;
  UINT8      OriginalSpeed;
  UINT8      OriginalWidth;
  UINT8      LaneListLength;
  UINT8      Bus;
  UINT8      Device;
  UINT8      Function;

  Status = EFI_SUCCESS;

  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    for (MarginDataIndex = 0; MarginDataIndex < PciePortsLength; MarginDataIndex++) {
      Bus      = MarginData[MarginDataIndex].PciePort->Bus;
      Device   = MarginData[MarginDataIndex].PciePort->Device;
      Function = MarginData[MarginDataIndex].PciePort->Function;
      if ((Bus      == PciePorts[PortIndex].Bus)         &&
          (Device   == PciePorts[PortIndex].Device)      &&
          (Function == PciePorts[PortIndex].Function)) {
        break;
      }
    }
    ASSERT (MarginDataIndex < PciePortsLength);
    ///
    /// Initialize Margins to -1.  Since -1 is an invalid value, we know that lane wasn't tested if its margin == -1
    ///
    for (LaneIndex = 0; LaneIndex < PciePorts[PortIndex].MaxPortLaneListLength; LaneIndex++) {
      MarginData[MarginDataIndex].TempMarginData[LaneIndex] = -1;
    }

    ///
    /// Test all lanes associated with the current port
    ///
    LaneList       = &(PciePorts[PortIndex].SwEqData.ActiveLaneList[0]);
    LaneListLength = PciePorts[PortIndex].SwEqData.ActiveLaneListLength;
    ASSERT (LaneListLength <= PCIE_MAX_LANE);
    if (LaneListLength > PCIE_MAX_LANE) {
      return EFI_INVALID_PARAMETER;
    }
    if (PciePorts[PortIndex].SwEqData.EnableMargin == FALSE ||
        PciePorts[PortIndex].SwEqData.SkipMargin   == TRUE) {
      continue;
    }

    if (PciePorts[PortIndex].SwEqData.MaxCapableSpeed >= 3) {
      OriginalSpeed = 3;
    } else {
      OriginalSpeed = This->GetCurrentLinkSpeed (This, &(PciePorts[PortIndex]));
    }
    OriginalWidth = This->GetNegotiatedWidth (This, &(PciePorts[PortIndex]));

    switch (MarginTest) {
      case LaneLevelRxJitter:
        Status = LaneLevelJitterTest (
                   This,
                   &(PciePorts[PortIndex]),
                   InputParameters,
                   MonitorPort,
                   LaneList,
                   LaneListLength,
                   OriginalSpeed,
                   OriginalWidth,
                   FALSE,
                   &(MarginData[MarginDataIndex].TempMarginData[0]),
                   DeferredPlatformResetRequired
                   );
        break;
      case PortLevelRxJitter:
        Status = PortLevelJitterTest (
                   This,
                   &(PciePorts[PortIndex]),
                   InputParameters,
                   MonitorPort,
                   LaneList,
                   LaneListLength,
                   OriginalSpeed,
                   OriginalWidth,
                   &(MarginData[MarginDataIndex].TempMarginData[0]),
                   DeferredPlatformResetRequired
                   );
        break;
      case VocCorners:
        Status = VocCornersMarginTest (
                   This,
                   &(PciePorts[PortIndex]),
                   InputParameters,
                   MonitorPort,
                   LaneList,
                   LaneListLength,
                   OriginalSpeed,
                   OriginalWidth,
                   &(MarginData[MarginDataIndex].TempMarginData[0]),
                   DeferredPlatformResetRequired
                   );
        break;
      default:
        DEBUG ((DEBUG_WARN, "Invalid Margin Test Requested.\n"));
        break;
    }
  }

  return Status;
}

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
  )
{
  EFI_STATUS Status;
  UINT8      LaneListIndex;
  INT8       Jitter;
  UINT32     Errors;
  UINT32     PreviousErrors;
  UINT8      Lane;
  UINT32     RecoveryCount;
  BOOLEAN    AbortMargin;
  UINTN      DwellTime;
  UINT16     ErrorTarget;
  UINT8      ConvergenceCounter;
  INT32      LastMargin;
  INT32      MarginDifference;
  INT32      StartJitter;
  INT8       MarginDirection;
  UINT8      RepeatCount;
  UINT8      RecoveryAttempt;

  DwellTime   = InputParameters->JitterDwellTime;
  ErrorTarget = InputParameters->JitterErrorTarget;

  for (LaneListIndex = 0; LaneListIndex < LaneListLength; LaneListIndex++) {
    Lane                      = LaneList[LaneListIndex];
    Errors                    = 0;
    AbortMargin               = FALSE;
    MarginData[LaneListIndex] = 0;
    MarginDirection           = 1;

    if (TxJitterTest) {
      ConfigureTxJitterMux (This, Lane);
      EnableTxJitterInjection (Lane, TRUE);
    }

    ///
    /// Determine value to start at
    ///
    if (LaneListIndex == 0) {
      StartJitter = 0;
    } else {
      StartJitter = (MarginData[LaneListIndex - 1] / 100) - JITTER_MARGIN_INITIAL_OFFSET;
      if (StartJitter < 0) {
        StartJitter = 0;
      }
    }

    ConvergenceCounter = 0;
    LastMargin         = -1;
    RepeatCount        = 0;
    while (ConvergenceCounter < MARGIN_CONVERGANCE_MIN_MATCHES && RepeatCount < MARGIN_CONVERGANCE_MAX_REPEATS) {
      RepeatCount++;
      ///
      /// Determine whether to go up or down from starting point
      ///
      AbortMargin   = FALSE;
      RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
      Status        = SetJitterTolerance (&Lane, 1, (UINT8) StartJitter);
      ASSERT_EFI_ERROR (Status);

      Errors = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);
      if (Errors >= ErrorTarget) {
        if (StartJitter == 0) {
          MarginData[LaneListIndex] = 0;
          AbortMargin               = TRUE;
        } else {
          MarginDirection = -1;
        }
      } else {
        MarginDirection = 1;
      }
      for (Jitter =  (INT8) (StartJitter + MarginDirection);
           Jitter <  JITTER_LENGTH &&
           Jitter >= 0             &&
           (!AbortMargin);
           Jitter = (INT8) (Jitter + MarginDirection)) {
        ///
        /// Check for a link downgrade
        ///
        AbortMargin = LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth);
        if (AbortMargin) {
          if (MarginDirection < 0) {
            LastMargin         = -1;
            ConvergenceCounter = 0;
            StartJitter        = 0;
            MarginDirection    = 1;
          } else {
            if (Jitter == 0) {
              MarginData[LaneListIndex] = 0;
            } else {
              MarginData[LaneListIndex] = (Jitter - 1) * 100;
            }
          }
          break;
        }

        ///
        /// Get initial recovery count
        ///
        RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
        Status        = SetJitterTolerance (&Lane, 1, Jitter);
        ASSERT_EFI_ERROR (Status);

        PreviousErrors = Errors;
        Errors         = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);

        if (MarginDirection < 0) {
          if (Errors < ErrorTarget) { ///< Downward direction has started passing
            MarginData[LaneListIndex] = InterpolateMargin (ErrorTarget, PreviousErrors, Errors, (INT32) Jitter);
            break;
          }
        } else {
          if (Errors >= ErrorTarget) { ///< Upward direction has started failing
            MarginData[LaneListIndex] = InterpolateMargin (ErrorTarget, Errors, PreviousErrors, (INT32) Jitter);
            break;
          }
        }
      } ///< End of for loop

      ///
      /// Check if we never reached the error target
      ///
      if (MarginDirection < 0) {
        if ((Errors >= ErrorTarget) && (!AbortMargin)) {
          MarginData[LaneListIndex] = 0;
        }
      } else {
        if ((Errors < ErrorTarget) && (!AbortMargin)) {
          MarginData[LaneListIndex] = JITTER_LENGTH * 100;
        }
      }

      ///
      /// Compute the next margin point to start at
      ///
      StartJitter = (MarginData[LaneListIndex] / 100) - JITTER_MARGIN_INITIAL_OFFSET;
      if (StartJitter < 0) {
        StartJitter = 0;
      }

      ///
      /// Check for convergance
      ///
      if (LastMargin == -1) {
        LastMargin = MarginData[LaneListIndex];
      } else {
        MarginDifference = CalculateMarginDifference (LastMargin, MarginData[LaneListIndex]);
        if (MarginDifference <= MARGIN_CONVERGANCE_ALLOWED_DELTA) {
          ConvergenceCounter++;
        } else {
          ConvergenceCounter = 0;
        }
        LastMargin = MarginData[LaneListIndex];
      }

      if (LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth)) {
        Status = SetJitterTolerance (&Lane, 1, 0);
        ASSERT_EFI_ERROR (Status);
      }

      ///
      /// If the link degraded in any way, bring it back to functional state
      ///
      for (RecoveryAttempt = 0; RecoveryAttempt < 3; RecoveryAttempt++) {
        Status = This->EnsureLinkIsHealthy (
                         This,
                         InputParameters,
                         PciePort,
                         OriginalLinkSpeed,
                         OriginalLinkWidth,
                         DeferredPlatformResetRequired
                         );
        if (!EFI_ERROR (Status)) {
          break;
        }
        AbortMargin = TRUE;
        Status = SetJitterTolerance (&Lane, 1, 0);
        ASSERT_EFI_ERROR (Status);
        StartJitter = 0;
        MicroSecondDelay (STALL_ONE_MILLI_SECOND * 10);
      }
      if (EFI_ERROR (Status)) {
        return Status;
      }
    } ///< End of repeat while loop

    ///
    /// Remove Jitter in preparation for testing the next lane
    ///
    Status = SetJitterTolerance (&Lane, 1, 0);
    ASSERT_EFI_ERROR (Status);
    if (TxJitterTest) {
      EnableTxJitterInjection (Lane, FALSE);
    }
  } ///< End of for each lane loop

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS Status;
  INT8      Jitter;
  UINT32    Errors;
  UINT32    PreviousErrors;
  UINT32    RecoveryCount;
  BOOLEAN   AbortMargin;
  UINTN     DwellTime;
  UINT16    ErrorTarget;
  UINT8     ConvergenceCounter;
  INT32     StartJitter;
  INT32     LastMargin;
  INT32     MarginDifference;
  INT8      MarginDirection;
  UINT8     RepeatCount;

  DwellTime   = InputParameters->JitterDwellTime;
  ErrorTarget = InputParameters->JitterErrorTarget;

  ///
  /// Determine value to start at
  ///
  StartJitter        = 0;
  MarginDirection    = 1;
  ConvergenceCounter = 0;
  LastMargin         = -1;
  RepeatCount        = 0;
  while (ConvergenceCounter < MARGIN_CONVERGANCE_MIN_MATCHES && RepeatCount < MARGIN_CONVERGANCE_MAX_REPEATS) {
    RepeatCount++;
    ///
    /// Determine whether to go up or down from starting point
    ///
    AbortMargin   = FALSE;
    RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
    Status        = SetJitterTolerance (&(LaneList[0]), LaneListLength, (UINT8) StartJitter);
    ASSERT_EFI_ERROR (Status);

    Errors = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);
    if (Errors >= ErrorTarget) {
      if (StartJitter == 0) {
        MarginData[0] = 0;
        AbortMargin   = TRUE;
      } else {
        MarginDirection = -1;
      }
    } else {
      MarginDirection = 1;
    }
    for (Jitter =  (INT8) (StartJitter + MarginDirection);
         Jitter <  JITTER_LENGTH &&
         Jitter >= 0             &&
         (!AbortMargin);
         Jitter = (INT8) (Jitter + MarginDirection)) {
      ///
      /// Check for a link downgrade
      ///
      AbortMargin = LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth);
      if (AbortMargin) {
        if (MarginDirection < 0) {
          LastMargin         = -1;
          ConvergenceCounter = 0;
          StartJitter        = 0;
          MarginDirection    = 1;
        } else {
          if (Jitter == 0) {
            MarginData[0] = 0;
          } else {
            MarginData[0] = (Jitter - 1) * 100;
          }
        }
        break;
      }

      ///
      /// Get initial recovery count
      ///
      RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
      Status        = SetJitterTolerance (&(LaneList[0]), LaneListLength, Jitter);
      ASSERT_EFI_ERROR (Status);

      PreviousErrors = Errors;
      Errors         = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);

      if (MarginDirection < 0) {
        if (Errors < ErrorTarget) { ///< Downward direction has started passing
          MarginData[0] = InterpolateMargin (ErrorTarget, PreviousErrors, Errors, (INT32) Jitter);
          break;
        }
      } else {
        if (Errors >= ErrorTarget) { ///< Upward direction has started failing
          MarginData[0] = InterpolateMargin (ErrorTarget, Errors, PreviousErrors, (INT32) Jitter);
          break;
        }
      }
    } ///< End of for loop

    ///
    /// Check if we never reached the error target
    ///
    if (MarginDirection < 0) {
      if ((Errors >= ErrorTarget) && (!AbortMargin)) {
        MarginData[0] = 0;
      }
    } else {
      if ((Errors < ErrorTarget) && (!AbortMargin)) {
        MarginData[0] = JITTER_LENGTH * 100;
      }
    }

    ///
    /// Compute the next margin point to start at
    ///
    StartJitter = (MarginData[0] / 100) - JITTER_MARGIN_INITIAL_OFFSET;
    if (StartJitter < 0) {
      StartJitter = 0;
    }

    ///
    /// Check for convergance
    ///
    if (LastMargin == -1) {
      LastMargin = MarginData[0];
    } else {
      MarginDifference = CalculateMarginDifference (LastMargin, MarginData[0]);
      if (MarginDifference <= MARGIN_CONVERGANCE_ALLOWED_DELTA) {
        ConvergenceCounter++;
      } else {
        ConvergenceCounter = 0;
      }
      LastMargin = MarginData[0];
    }

    if (LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth)) {
      Status = SetJitterTolerance (&(LaneList[0]), LaneListLength, 0);
      ASSERT_EFI_ERROR (Status);
    }

    ///
    /// If the link degraded in any way, bring it back to functional state
    ///
    Status = This->EnsureLinkIsHealthy (
                     This,
                     InputParameters,
                     PciePort,
                     OriginalLinkSpeed,
                     OriginalLinkWidth,
                     DeferredPlatformResetRequired
                     );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  } ///< End of repeat while loop

  ///
  /// Margin complete, remove jitter
  ///
  Status = SetJitterTolerance (&(LaneList[0]), LaneListLength, 0);
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS Status;
  UINT8      LaneListIndex;
  VOC_STATE  VocState;
  UINT32     Errors;
  UINT32     RecoveryCount;
  UINT8      Lane;
  UINT32     DwellTime;
  UINT16     ErrorTarget;
  UINT8      RecoveryAttempt;
  UINT8      RetestCounter;
  BOOLEAN    MaskTestSuccessful;
  BOOLEAN    LinkReset;

  DwellTime   = InputParameters->VocDwellTime;
  ErrorTarget = InputParameters->VocErrorTarget;

  ///
  /// Enable VOC circuits
  ///
  Status = SetVocEnable (TRUE);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (LaneListIndex = 0; LaneListIndex < LaneListLength; LaneListIndex++) {
    Lane   = LaneList[LaneListIndex];
    Errors = 0;

    ///
    /// Get initial trained values
    ///
    Status = InitializeVocMargining (Lane, &VocState);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    //
    MaskTestSuccessful = FALSE;
    for (RetestCounter = 0; RetestCounter <= MARGIN_CONVERGANCE_MIN_MATCHES && !MaskTestSuccessful; RetestCounter++) {
      LinkReset = FALSE;
      do {
        ///
        /// Check High Side Mask
        ///
        RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
        Status        = SaPcieSetVocToMaskValue (VOC_HIGH_SIDE_MASK, &VocState);
        if (Status == EFI_ABORTED) {
          ///
          ///
          break;
        }
        ASSERT_EFI_ERROR (Status);

        Errors = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);
        if (Errors >= ErrorTarget) {
          break;
        }
        if (LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth)) {
          break;
        }

        ///
        /// Check Low Side Mask
        ///
        RecoveryCount = This->GetErrorCount (This, MonitorPort, PciePort);
        Status        = SaPcieSetVocToMaskValue (VOC_LOW_SIDE_MASK, &VocState);
        if (Status == EFI_ABORTED) {
          ///
          ///
          break;
        }
        ASSERT_EFI_ERROR (Status);

        Errors = SaPciePointTest (This, PciePort, DwellTime, MonitorPort, RecoveryCount);
        if (Errors >= ErrorTarget) {
          break;
        }
        if (LinkIsDowngraded (This, PciePort, OriginalLinkSpeed, OriginalLinkWidth)) {
          break;
        }

        ///
        /// Passed the VOC Mask test
        ///
        MarginData[LaneListIndex] = VOC_CORNERS_TEST_PASS_CREDIT;
        MaskTestSuccessful        = TRUE;
      } while (0);

      ///
      /// Set back to nominal in preparation for testing the next lane
      ///
      FinalizeVocMargining (&VocState);

      ///
      /// If the link degraded in any way, bring it back to functional state
      ///
      for (RecoveryAttempt = 0; RecoveryAttempt < 3; RecoveryAttempt++) {
        Status = This->EnsureLinkIsHealthy (
                         This,
                         InputParameters,
                         PciePort,
                         OriginalLinkSpeed,
                         OriginalLinkWidth,
                         DeferredPlatformResetRequired
                         );
        if (!EFI_ERROR (Status)) {
          break;
        }
        LinkReset = TRUE;
        FinalizeVocMargining (&VocState);
        MicroSecondDelay (STALL_ONE_MILLI_SECOND * 10);
      }
      if (EFI_ERROR (Status)) {
        SetVocEnable (FALSE);
        return Status;
      }
      if (LinkReset) {
        ///
        ///
        Status = InitializeVocMargining (Lane, &VocState);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }
    } ///< End of Retest loop
  } ///< End of for each lane loop

  ///
  /// Disable VOC circuits for power savings
  ///
  Status = SetVocEnable (FALSE);
  return Status;
}


UINT32
SaPciePointTest (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINTN                             DwellTime,
  IN  UINT32                            MonitorPort,
  IN  UINT32                            InitialRecoveryCount
  )
{
  UINT32  Data32;

  MicroSecondDelay (DwellTime);
  Data32 = This->GetErrorCount (This, MonitorPort, PciePort) - InitialRecoveryCount;

  return Data32;
}

INT32
CalculateMarginDifference (
  IN INT32                   Margin1,
  IN INT32                   Margin2
  )
{
  if (Margin1 < Margin2) {
    return Margin2 - Margin1;
  } else {
    return Margin1 - Margin2;
  }
}

INT32
InterpolateMargin (
  IN UINT32 ErrorTarget,
  IN UINT32 CurrentErrorCount,
  IN UINT32 PreviousErrorCount,
  IN INT32  FailingPoint
  )
{
  UINT32 LnErrorTarget;
  UINT32 LnCurrentErrorCount;
  UINT32 LnPreviousErrorCount;
  INT32  Margin;

  if (ErrorTarget > 40000) {
    ErrorTarget = 40000;
  }
  if (CurrentErrorCount > 40000) {
    CurrentErrorCount = 40000;
  }
  if (PreviousErrorCount > 40000) {
    PreviousErrorCount = 40000;
  }

  LnErrorTarget        = NaturalLog (ErrorTarget * 100);
  LnCurrentErrorCount  = NaturalLog (CurrentErrorCount * 100);
  LnPreviousErrorCount = NaturalLog (PreviousErrorCount * 100);

  if (FailingPoint >= 0) {
    if ((LnCurrentErrorCount - LnPreviousErrorCount) == 0) {
      Margin = (FailingPoint - 1) * 100;
    } else {
      Margin = ((LnErrorTarget - LnPreviousErrorCount) * 100) /
        (LnCurrentErrorCount - LnPreviousErrorCount)   +
        ((FailingPoint - 1) * 100);
    }
    if (Margin < 0) {
      Margin = 0;
    }
  } else {
    if ((LnCurrentErrorCount - LnPreviousErrorCount) == 0) {
      Margin = (FailingPoint + 1) * 100;
    } else {
      Margin = ((FailingPoint + 1) * 100)                     -
        ((LnErrorTarget - LnPreviousErrorCount) * 100) /
        (LnCurrentErrorCount - LnPreviousErrorCount);
    }
    if (Margin > 0) {
      Margin = 0;
    }
  }

  return Margin;
}

//
//
UINT32
NaturalLog (
  IN UINT32 Input
  )
  /*++

  Routine Description:

    This function calculates the Natural Log of the Input parameter using integers

  Arguments:

    Input           - 100 times a number to get the Natural log from.
                    - Max Input Number is 40,000 (without 100x)

  Returns:

    Output          - 100 times the actual result. Accurate within +/- 2

  --*/
{
  UINT32  Output;

  ///
  ///Special case - treat 0 recoveries as 1 recovery for interpolation purposes
  ///
  if (Input == 0) {
    return 0;
  }
  ///
  ///Eliminate numerical error for the case of ln(1) = 0
  ///
  if (Input == 100) {
    return 0;
  }

  Output = 0;
  while (Input > 271) {
    Input   = (Input * 1000) / 2718;
    Output += 100;
  }

  Output += ((-16 * Input * Input + 11578 * Input - 978860) / 10000);

  return Output;
}

