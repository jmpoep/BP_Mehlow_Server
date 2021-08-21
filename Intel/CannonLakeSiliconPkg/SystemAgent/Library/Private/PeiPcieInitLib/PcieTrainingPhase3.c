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

@par Specification
**/

#include "PcieTrainingGeneric.h"

#include <Private/Library/MeInitLib.h>

PCIE_SWEQ_PORT_INPUT*
GetPortInputParameters (
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT8   PortIndex;

  for (PortIndex = 0; PortIndex < InputParameters->PerPortInputParametersLength; PortIndex++) {
    Bus      = InputParameters->PerPortInputParameters[PortIndex].PciePort->Bus;
    Device   = InputParameters->PerPortInputParameters[PortIndex].PciePort->Device;
    Function = InputParameters->PerPortInputParameters[PortIndex].PciePort->Function;
    if ((Bus      == PciePort->Bus)         &&
        (Device   == PciePort->Device)      &&
        (Function == PciePort->Function)) {
      return &(InputParameters->PerPortInputParameters[PortIndex]);
    }
  }
  return NULL;
}

PCIE_SWEQ_PORT_OUTPUT*
GetPortOutput (
  IN  PCIE_SWEQ_OUTPUT                  *OutputData,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
  UINT8   PortIndex;

  for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
    Bus      = OutputData->PortOutputList[PortIndex].PciePort->Bus;
    Device   = OutputData->PortOutputList[PortIndex].PciePort->Device;
    Function = OutputData->PortOutputList[PortIndex].PciePort->Function;
    if ((Bus      == PciePort->Bus)         &&
        (Device   == PciePort->Device)      &&
        (Function == PciePort->Function)) {
      return &(OutputData->PortOutputList[PortIndex]);
    }
  }
  return NULL;
}

UINT8
SelectBestPresetForLane (
  IN  PCIE_PORT_INFO              *PciePort,
  IN  UINT8                       LaneIndex,
  IN  PCIE_SWEQ_INPUT_PARAMETERS  *InputParameters,
  IN  UINT8                       TrainingPreset
  )
{
  PCIE_SWEQ_PORT_INPUT  *PortConfig;
  UINT8                 BestPreset;

  ///
  /// Get Port Level Input/Output parameters
  ///
  PortConfig = GetPortInputParameters (InputParameters, PciePort);
  ASSERT (PortConfig != NULL);

  ///
  /// Init to EV default
  ///
  BestPreset = 7;

  ///
  /// EnableMargin    FoundUsablePreset    HaveDataFromLastBoot
  ///            Y                    Y                       X  -> Use training results
  ///            X                    N                       X  -> Use Policy value
  ///            N                    X                       Y  -> Restore from Last Boot
  ///
  if (LaneIndex < PciePort->MaxPortLaneListLength) {
    DEBUG ((DEBUG_INFO, "Preset for Lane %2d: ", PciePort->MaxPortLaneList[LaneIndex]));
    if ((PortConfig != NULL) && ((PciePort->SwEqData.EnableMargin) && (PciePort->SwEqData.FoundUsableTxEq))) {
      ///
      /// Use the best preset found during training
      ///
      BestPreset = TrainingPreset;
      DEBUG ((DEBUG_INFO, "Search Result: P%d", BestPreset));
    } else if ((PortConfig != NULL) && ((!PciePort->SwEqData.EnableMargin) && (InputParameters->HaveDataFromLastBoot))) {
      ///
      /// Use the preset found on a previous boot
      ///
      BestPreset = PortConfig->LastBootBestPresets[LaneIndex];
      DEBUG ((DEBUG_INFO, "Restoring previous value: P%d", BestPreset));
    } else if ((PortConfig != NULL) && (!PciePort->SwEqData.FoundUsableTxEq)) {
      ///
      /// Use the policy value
      ///
      BestPreset = PortConfig->StaticEndpointPresets[LaneIndex];
      DEBUG ((DEBUG_INFO, "Applying Policy value: P%d", BestPreset));
    }
    DEBUG ((DEBUG_INFO, "\n"));
  } else {
    ///
    /// Error: Non-existent lane
    ///
    DEBUG ((DEBUG_ERROR, "Illegal Lane: %d", LaneIndex));
  }

  return BestPreset;
}

/**
  The PCIe Software Equalization algorithm.  Provides an adaptive EQ Phase 3
  implementation in software.

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to wait for
  @param[in]  PciePortsLength             - Length of the PciePorts array
  @param[in]  InputParameters             - Configuration options for SW EQ
  @param[out] OutputData                  - The data that the algorithm generated
**/
VOID
PcieGen3SoftwareEqualization (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  PCIE_SWEQ_INPUT_PARAMETERS        *InputParameters,
  OUT PCIE_SWEQ_OUTPUT                  *OutputData
  )
{
  PCIE_SWEQ_PORT_INPUT  *PortConfig;
  PCIE_SWEQ_PORT_OUTPUT *PortOutput;
  UINT8                 Index;
  INT32                 MarginScore;
  BOOLEAN               LoadedSavedPreset;
  BOOLEAN               EndpointDeviceChanged;
  BOOLEAN               SwEqEnabledOnPort;
  UINT8                 TempIndex;
  INT32                 LaneScores[PCIE_MAX_LANE];
  UINT8                 Presets[PCIE_MAX_LANE];
  EFI_STATUS            Status;
  UINT32                MonitorPort;
  UINT8                 PortIndex;
  UINT8                 Lane;
  BOOLEAN               AnyGen3CapableLinks;
  BOOLEAN               AnyMarginingNeeded;
  UINT32                NominalRecoveryCount;
  UINT8                 DetectedReplacedCpu;
  UINT8                 Preset;
  BOOLEAN               UnrecoverableWidthDowngrade;

  AnyMarginingNeeded          = FALSE;
  UnrecoverableWidthDowngrade = FALSE;
  if (InputParameters->PresetsToTestLength <= 0) {
    DEBUG ((DEBUG_WARN, "No TxEQs to test, exiting SW EQ\n"));
    return;
  }
  ASSERT (OutputData->PortOutputListLength >= PciePortsLength);

  ///
  /// Initialize Arrays
  ///
  for (Lane = 0; Lane < PCIE_MAX_LANE; Lane++) {
    LaneScores[Lane]  = -1;
  }
  for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
    for (Lane = 0; Lane < PCIE_MAX_LANE; Lane++) {
      OutputData->PortOutputList[PortIndex].BestScores[Lane]  = -1;
      OutputData->PortOutputList[PortIndex].BestPresets[Lane] = InputParameters->PresetsToTest[0];
    }
  }

  DEBUG ((DEBUG_INFO, "PCIe Gen3 Software Equalization\n"));

  ///
  /// If ME is supported and the CPU has been replaced, redo the Preset Search.
  /// Note that calling CpuReplacementCheck() can induce an ME-required warm reset.
  ///
  DetectedReplacedCpu = 0;
  Status              = EFI_SUCCESS;

  //
  //
  if (InputParameters->HaveDataFromLastBoot) {
    DEBUG ((DEBUG_INFO, "Calling CpuReplacementCheck\n"));
    Status = CpuReplacementCheck (NULL, &DetectedReplacedCpu);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, " ME reported CPU Replacement value: %x\n", DetectedReplacedCpu));
  }

  ///
  /// Make sure we are at Gen3 before starting, if not attempt reset and see if that helps
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if ((PcieAccess->GetCurrentLinkSpeed (PcieAccess, &(PciePorts[PortIndex])) < 3) &&
        (PciePorts[PortIndex].SwEqData.MaxCapableSpeed >= 3)) {
      PcieAccess->ResetEndpointPerst (PcieAccess, &(PciePorts[PortIndex]), InputParameters);
    }
  }

  ///
  /// Determine which PEG ports require testing
  ///
  AnyGen3CapableLinks = FALSE;
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    ///
    /// If we already have Best Preset value from previous boot, use it and skip PresetSearch when end point device no change
    ///
    LoadedSavedPreset     = FALSE;
    EndpointDeviceChanged = TRUE;
    SwEqEnabledOnPort = FALSE;
    PortConfig            = GetPortInputParameters (InputParameters, &(PciePorts[PortIndex]));
    ASSERT (PortConfig != NULL);
    if (PortConfig != NULL) {
      SwEqEnabledOnPort     = PortConfig->EnableSwEq;
      if (InputParameters->HaveDataFromLastBoot) {
        LoadedSavedPreset = TRUE;
        if (PortConfig->LastBootEndpointVendorIdDeviceId == PciePorts[PortIndex].EndpointVendorIdDeviceId) {
          EndpointDeviceChanged = FALSE;
        }
      }
    }
    DEBUG ((DEBUG_INFO, " PCIe RP (%x:%x:%x) - LoadedSavedPreset = %d. EndpointDeviceChanged = %d. SwEqEnabledOnPort = %d.\n",
            PciePorts[PortIndex].Bus, PciePorts[PortIndex].Device, PciePorts[PortIndex].Function,
            LoadedSavedPreset, EndpointDeviceChanged, SwEqEnabledOnPort));

    PcieAccess->ReportPcieLinkStatus (PcieAccess, &(PciePorts[PortIndex]));

    if (((!LoadedSavedPreset)                 ||
         (EndpointDeviceChanged)               ||
         (DetectedReplacedCpu != 0)            ||
         (InputParameters->AlwaysAttemptSwEq)) &&
        SwEqEnabledOnPort) {
      PciePorts[PortIndex].SwEqData.EnableMargin = TRUE;
    } else {
      PciePorts[PortIndex].SwEqData.EnableMargin = FALSE;
    }

    if (PciePorts[PortIndex].SwEqData.MaxCapableSpeed < 3 ||
        !PciePorts[PortIndex].EndpointPresent) {
      PciePorts[PortIndex].SwEqData.EnableMargin = FALSE;
    } else {
      AnyGen3CapableLinks = TRUE;
    }
  } ///< End of for each port

  if (!AnyGen3CapableLinks) {
    DEBUG ((DEBUG_INFO, "Skipping Software Equalization - No Gen3 capable links\n"));
    return;
  }

  ///
  /// Determine if any ports need to be trained.
  /// If any ports are trained, the corresponding endpoint should also be reset with PERST#.
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].SwEqData.EnableMargin == TRUE) {
      AnyMarginingNeeded = TRUE;
      break;
    }
  }

  if (AnyMarginingNeeded) {
    MonitorPort = PcieAccess->OpenMonitor (PcieAccess);

    ///
    ///
    PcieAccess->PollingComplianceMode (PcieAccess, PciePorts, PciePortsLength, FALSE);

    ///
    /// Presets Loop start
    ///
    for (Index = 0; Index < InputParameters->PresetsToTestLength; Index++) {

      for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
        ///
        /// Clear out old values
        ///
        PortOutput = GetPortOutput (OutputData, &(PciePorts[PortIndex]));
        ASSERT (PortOutput != NULL);
        if (PortOutput != NULL) {
          PortOutput->PresetScores[Index].Preset = InputParameters->PresetsToTest[Index];
          for (TempIndex = 0; TempIndex < PCIE_MAX_LANE; TempIndex++) {
            PortOutput->PresetScores[Index].MarginScore[TempIndex] = 0;
          } ///< End of for each Lane
        }

        PciePorts[PortIndex].SwEqData.SkipMargin = FALSE;
        if (PciePorts[PortIndex].SwEqData.MaxCapableSpeed < 3) {
          DEBUG ((
                   DEBUG_INFO,
                   "Skipping PCIe RP (%x:%x:%x) - Not Gen3 capable\n",
                   PciePorts[PortIndex].Bus,
                   PciePorts[PortIndex].Device,
                   PciePorts[PortIndex].Function
                   ));
          continue;
        }
        if (!(PciePorts[PortIndex].SwEqData.EnableMargin)) {
          DEBUG ((
                   DEBUG_INFO,
                   "Skipping PCIe RP (%x:%x:%x) - SW EQ not needed\n",
                   PciePorts[PortIndex].Bus,
                   PciePorts[PortIndex].Device,
                   PciePorts[PortIndex].Function
                   ));
          continue;
        }

        ///
        /// Program the Preset to Test to all lanes on this port
        ///
        PcieAccess->ProgramUniformPortPhase3TxEq (
                      PcieAccess,
                      &(PciePorts[PortIndex]),
                      InputParameters->PresetsToTest[Index]
                      );
        PcieAccess->WaitForL0 (PcieAccess, &(PciePorts[PortIndex]));

        ///
        /// No need to margin if link couldn't get to Gen3 with this preset
        ///
        PcieAccess->EnsureLinkIsHealthy (
                      PcieAccess,
                      InputParameters,
                      &(PciePorts[PortIndex]),
                      3,
                      PcieAccess->GetNegotiatedWidth (PcieAccess, &(PciePorts[PortIndex])),
                      &(OutputData->DeferredPlatformResetRequired)
                      );
        if (PcieAccess->GetCurrentLinkSpeed (PcieAccess, &(PciePorts[PortIndex])) < 3) {
          PciePorts[PortIndex].SwEqData.SkipMargin = TRUE;
          continue;
        }

        NominalRecoveryCount = PcieAccess->GetErrorCount (PcieAccess, MonitorPort, &(PciePorts[PortIndex]));
        MicroSecondDelay (InputParameters->JitterDwellTime * STALL_ONE_MICRO_SECOND);
        NominalRecoveryCount = PcieAccess->GetErrorCount (PcieAccess, MonitorPort, &(PciePorts[PortIndex])) - NominalRecoveryCount;
        if (NominalRecoveryCount > 0) {
          PciePorts[PortIndex].SwEqData.SkipMargin = TRUE;
          continue;
        }
        PciePorts[PortIndex].SwEqData.FoundUsableTxEq = TRUE;
      } ///< End of for each Port

      ///
      ///
      ASSERT (OutputData->PortOutputListLength >= PciePortsLength);
      Status = PcieAccess->RunMarginTest (
                             PcieAccess,
                             PciePorts,
                             PciePortsLength,
                             InputParameters,
                             MonitorPort,
                             LaneLevelRxJitter,
                             OutputData->PortOutputList,
                             &(OutputData->DeferredPlatformResetRequired)
                             );
      if (EFI_ERROR (Status)) {
        for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
          ///
          ///
          if (PcieAccess->GetNegotiatedWidth (
                            PcieAccess,
                            OutputData->PortOutputList[PortIndex].PciePort
                            )
              < OutputData->PortOutputList[PortIndex].PciePort->SwEqData.MaxCapableWidth) {
            UnrecoverableWidthDowngrade = TRUE;
            break;
          }
          ///
          ///
          for (TempIndex = 0; TempIndex < PciePorts[PortIndex].MaxPortLaneListLength; TempIndex++) {
            OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[TempIndex] = 0;
          }
        }
        if (UnrecoverableWidthDowngrade) {
          for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
            OutputData->PortOutputList[PortIndex].PciePort->SwEqData.FoundUsableTxEq = FALSE;
          }
          break;
        }
      } else {
        ///
        ///
        for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
          for (TempIndex = 0; TempIndex < PciePorts[PortIndex].MaxPortLaneListLength; TempIndex++) {
            OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[TempIndex] = OutputData->PortOutputList[PortIndex].TempMarginData[TempIndex];
          }
        }
      }
      if (InputParameters->EnableVocTest) {
        Status = PcieAccess->RunMarginTest (
                               PcieAccess,
                               PciePorts,
                               PciePortsLength,
                               InputParameters,
                               MonitorPort,
                               VocCorners,
                               OutputData->PortOutputList,
                               &(OutputData->DeferredPlatformResetRequired)
                               );
        if (!EFI_ERROR (Status)) {
          ///
          ///
          for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
            for (TempIndex = 0; TempIndex < OutputData->PortOutputList[PortIndex].PciePort->MaxPortLaneListLength; TempIndex++) {
              if (OutputData->PortOutputList[PortIndex].TempMarginData[TempIndex] != -1) {
                if (OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[TempIndex] >= JITTER_MASK) {
                  OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[TempIndex] += OutputData->PortOutputList[PortIndex].TempMarginData[TempIndex];
                }
              }
            }
          } ///< End of for each port
        } else {
          ///
          ///
          for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
            if (PcieAccess->GetNegotiatedWidth (
                              PcieAccess,
                              OutputData->PortOutputList[PortIndex].PciePort
                              )
                < OutputData->PortOutputList[PortIndex].PciePort->SwEqData.MaxCapableWidth) {
              UnrecoverableWidthDowngrade = TRUE;
              break;
            }
          }
          if (UnrecoverableWidthDowngrade) {
            for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
              OutputData->PortOutputList[PortIndex].PciePort->SwEqData.FoundUsableTxEq = FALSE;
            }
            break;
          }
        }
      } ///< End of VOC test enabled
    } ///< Presets Loop end

    ///
    ///
    PcieAccess->PollingComplianceMode (PcieAccess, PciePorts, PciePortsLength, TRUE);
    PcieAccess->CloseMonitor (PcieAccess, MonitorPort);

    ///
    /// Find the preset with the maximum margin (largest of all margin values)
    ///
    for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
      for (Index = 0; Index < InputParameters->PresetsToTestLength; Index++) {
        ///
        ///
        for (Lane = 0; Lane < PCIE_MAX_LANE; Lane++) {
          LaneScores[Lane] = -1;
        }
        for (Lane = 0; Lane < OutputData->PortOutputList[PortIndex].PciePort->MaxPortLaneListLength; Lane++) {
          ///
          ///
          if (OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[Lane] != -1) {
            MarginScore = OutputData->PortOutputList[PortIndex].PresetScores[Index].MarginScore[Lane];
            LaneScores[Lane] = MarginScore;
          }
          ///
          ///
          if (LaneScores[Lane] > OutputData->PortOutputList[PortIndex].BestScores[Lane]) {
            OutputData->PortOutputList[PortIndex].BestScores[Lane]  = LaneScores[Lane];
            OutputData->PortOutputList[PortIndex].BestPresets[Lane] = OutputData->PortOutputList[PortIndex].PresetScores[Index].Preset;
          }
        } ///< End of for each Lane
      } ///< End of for each Preset
    } ///< End of for each port
  } ///< End of AnyMarginingNeeded

  ///
  /// Choose the best TxEQ for each lane
  ///
  for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
    if (OutputData->PortOutputList[PortIndex].PciePort->SwEqData.MaxCapableSpeed < 3) {
      continue;
    }
    for (TempIndex = 0; TempIndex < OutputData->PortOutputList[PortIndex].PciePort->MaxPortLaneListLength; TempIndex++) {
      Preset = SelectBestPresetForLane (
                 OutputData->PortOutputList[PortIndex].PciePort,
                 TempIndex,
                 InputParameters,
                 OutputData->PortOutputList[PortIndex].BestPresets[TempIndex]
                 );
      OutputData->PortOutputList[PortIndex].BestPresets[TempIndex] = Preset;
    }
  }

  ///
  /// Program the presets.  If any link was margined, also reset the
  /// endpoints in order to return the endpoints to a known-good state.
  ///
  if (AnyMarginingNeeded) {
    ///
    ///
    for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
      PcieAccess->SetLinkDisable (PcieAccess, OutputData->PortOutputList[PortIndex].PciePort, TRUE);
    }
    for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
      PcieAccess->SetPerst (PcieAccess, OutputData->PortOutputList[PortIndex].PciePort, InputParameters, TRUE);
    }
  }

  for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
    for (Lane = 0; Lane < PCIE_MAX_LANE; Lane++) {
      Presets[Lane] = 7;
    }
    for (TempIndex = 0; TempIndex < OutputData->PortOutputList[PortIndex].PciePort->MaxPortLaneListLength; TempIndex++) {
      Preset = OutputData->PortOutputList[PortIndex].BestPresets[TempIndex];
      Presets[OutputData->PortOutputList[PortIndex].PciePort->MaxPortLaneList[TempIndex]] = Preset;
    }
    PcieAccess->ProgramPortPhase3TxEq (PcieAccess, OutputData->PortOutputList[PortIndex].PciePort, &(Presets[0]));
  }

  if (AnyMarginingNeeded) {
    ///
    ///
    MicroSecondDelay (100 * STALL_ONE_MICRO_SECOND);
    ///
    ///
    for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
      PcieAccess->SetLinkDisable (PcieAccess, OutputData->PortOutputList[PortIndex].PciePort, FALSE);
    }
    for (PortIndex = 0; PortIndex < OutputData->PortOutputListLength; PortIndex++) {
      PcieAccess->SetPerst (PcieAccess, OutputData->PortOutputList[PortIndex].PciePort, InputParameters, FALSE);
    }
  }

/* @todo BDAT support not yet ready for SKL, will be fixed in an upcoming release.
#ifdef BDAT_SUPPORT
  if (InputParameters->EnableBdatScoreSchema) {
    Status = CreateEqPhase3Schema (PeiServices, PortInfoList, PortInfoListLength, SkipBundle0, BestPresets);
    DEBUG ((DEBUG_INFO, "Creating EQ Phase 3 BDAT Schema: %r\n", Status));
    if (AnyMarginingNeeded) {
      Status = CreateScoreSchema (PeiServices, &(MarginData[0]), InputParameters->PresetsToTestLength);
      DEBUG ((DEBUG_INFO, "Creating Score BDAT Schema: %r\n", Status));
    }
  }
#endif
*/

  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    PcieAccess->WaitForL0 (PcieAccess, &(PciePorts[PortIndex]));
    PcieAccess->ReportPcieLinkStatus (PcieAccess, &(PciePorts[PortIndex]));
  }

  DEBUG ((DEBUG_INFO, "PCIe Gen3 Software Equalization done\n\n"));

  return;
}
