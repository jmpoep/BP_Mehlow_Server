/** @file
  PEI Function to initialize SA PciExpress.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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
#include <Private/Library/SaPcieInitLib.h>
#include <Private/Library/SaInitLib.h>
#include "PcieTraining.h"
#include <Private/Library/PmcPrivateLib.h>
#include <CpuRegs.h>
#include <CpuAccess.h>
#include <Library/CpuPlatformLib.h>

#include <Library/PchInfoLib.h>
#include <Library/ConfigBlockLib.h>

#include <Private/SaPegHob.h>
#include <Private/SaConfigHob.h>
#include <ConfigBlock/SiConfig.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PeiPchPcieClocksLib.h>

#ifndef CPU_CFL
/**
  Read the SA PCIE policy to see if PCIe IMR is enabled.

  @retval TRUE             PCIe IMR is enabled
  @retval FALSE            PCIe IMR is disabled
**/
BOOLEAN
SaIsPcieImrEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;
  PCIE_PEI_PREMEM_CONFIG          *PciePeiPreMemConfig;
  SI_PREMEM_POLICY_PPI           *SiPreMemPolicyPpi;
  ///
  /// Get policy settings through the SiPreMemPolicyPpi
  ///
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  if ((Status != EFI_SUCCESS) || (SiPreMemPolicyPpi == NULL)) {
    return FALSE;
  }

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gSaPciePeiPreMemConfigGuid, (VOID *) &PciePeiPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    if (PciePeiPreMemConfig->PegImrEnable != 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Read the SA PCIE policy to get CPU PCIe IMR Size.

  @retval PcieImrSize      Size of PCIe Imr in MB
**/
UINT32
SaGetPcieImrSize (
  VOID
  )
{
  EFI_STATUS                      Status;
  PCIE_PEI_PREMEM_CONFIG          *PciePeiPreMemConfig;
  SI_PREMEM_POLICY_PPI            *SiPreMemPolicyPpi;
  ///
  /// Get policy settings through the SiPreMemPolicyPpi
  ///
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  if ((Status != EFI_SUCCESS) || (SiPreMemPolicyPpi == NULL)) {
    return 0;
  }

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gSaPciePeiPreMemConfigGuid, (VOID *) &PciePeiPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    return (UINT32) PciePeiPreMemConfig->PegImrSize;
  }
  return 0;
}
#endif

/**
  Programs static equalization settings for SA PEG

  @param[in]  This                        - Low level function table
  @param[in]  PCIE_PEI_PREMEM_CONFIG      - PciePeiPreMemConfig
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
PcieGen3StaticEq (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PEI_PREMEM_CONFIG            *PciePeiPreMemConfig,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  PCIE_PORT_EQS                     PciePortEqs[SA_PEG_MAX_FUN];
  UINT8                             *RootPortPreset;
  UINT8                             *EndPointPreset;
  UINT8                             *EndPointHint;
  UINT8                             PortIndex;
  UINT8                             LaneIndex;
  UINT8                             Lane;

  RootPortPreset = PciePeiPreMemConfig->PegGen3RootPortPreset;
  EndPointPreset = PciePeiPreMemConfig->PegGen3EndPointPreset;
  EndPointHint   = PciePeiPreMemConfig->PegGen3EndPointHint;

  if (PciePortsLength > SA_PEG_MAX_FUN) {
    DEBUG ((DEBUG_ERROR, "Number of root ports (%d) exceeds max (%d)\n", PciePortsLength, SA_PEG_MAX_FUN));
    ASSERT (FALSE);
    return;
  }

  ///
  /// Convert data from SA Policy PPI to PCIE_PORT_EQS structure
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    PciePortEqs[PortIndex].PciePort = &(PciePorts[PortIndex]);
    for (LaneIndex = 0; LaneIndex < PciePorts[PortIndex].MaxPortLaneListLength; LaneIndex++) {
      Lane = PciePorts[PortIndex].MaxPortLaneList[LaneIndex];
      PciePortEqs[PortIndex].RootPortPresets[LaneIndex] = RootPortPreset[Lane];
      PciePortEqs[PortIndex].EndpointPresets[LaneIndex] = EndPointPreset[Lane];
      PciePortEqs[PortIndex].EndpointHints[LaneIndex]   = EndPointHint[Lane];
    }
  }

  ///
  /// Program the equalization settings
  ///
  PcieAccess->ProgramStaticGen3Eq (PcieAccess, &(PciePortEqs[0]), PciePortsLength);
}

/**
Checks if an Endpoint in inserted in a slot which was not populated in previous boot

@param[in]       PcieAccess                  - Low level function table
@param[in]       PciePorts                   - PCIe Root Ports to wait for
@param[in]       PciePortsLength             - Length of the PciePorts array
@param[in, out]  SaPegHob                    - The HOB containing SA data
**/
VOID
IsPcieTopologyChanged (
  IN      PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN      PCIE_PORT_INFO                    *PciePorts,
  IN      UINT8                             PciePortsLength,
  IN OUT  SA_PEG_HOB                        *SaPegHob
  )
{
  UINT8 Index;
  BOOLEAN PortDisabled;

  for (Index = 0; Index < PciePortsLength; Index++) {
    PortDisabled = SaPegHob->PegData.PegLinkFailMask & (BIT0 < Index);
    if (PortDisabled && PcieAccess->GetSlotPresenceDetect(PcieAccess, &(PciePorts[Index]))) {
      DEBUG((DEBUG_ERROR, "A new Endpoint is detected in RpIndex %d\n", Index));
      SaPegHob->PegData.PegLinkFailMask &= ~(BIT0 < Index);
    }
  }
}

/**
  Waits for the Data Link Layer on all given root ports to reach the DL_Active
  state.  Updates the SaPegHob with the new fail mask.

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to wait for
  @param[in]  PciePortsLength             - Length of the PciePorts array
  @param[in]  SaPegHob                    - The HOB containing SA data
**/
VOID
SaPcieWaitForDataLinkLayerLinkActive (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  SA_PEG_HOB                        *SaPegHob
  )
{
  UINT32                    FailMask[PCIE_ROOT_PORT_BITMAP_LENGTH];
  PCIE_PEI_PREMEM_CONFIG    *PciePeiPreMemConfig;
  EFI_STATUS                Status;
  EFI_BOOT_MODE             BootMode;
  UINT8                     PegLinkFailMask;
  UINT8                     Index;
  BOOLEAN                   AllRpsDisabled;
  BOOLEAN                   WarmReset;

  WarmReset = FALSE;
  PciePeiPreMemConfig = ((SA_PCIE_PRIVATE_FUNCTION_CALLS*) PcieAccess->PrivateData)->PciePeiPreMemConfig;
  for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
    FailMask[Index] = 0xFFFFFFFF;
  }
  ///
  /// Set Fail Mask so all PEG controllers that are not disabled will be checked
  ///
  PegLinkFailMask = 0xFF;
  for (Index = 0; Index < PciePortsLength; Index++) {
    ///
    /// If a controller is forced to disabled, then don't wait for it
    ///
    if (!SaPolicyForceDisablesPort (&PciePorts[Index], PciePeiPreMemConfig)) {
      if (PciePorts[Index].Device == SA_PEG0_DEV_NUM) {
        PegLinkFailMask &= (UINT8) ~(B_PCIE_PEG0_BITMAP << PciePorts[Index].Function);
      } else if (PciePorts[Index].Device == SA_PEG3_DEV_NUM) {
        PegLinkFailMask &= (UINT8) ~(B_PCIE_PEG3_BITMAP);
      }
    }
  }
  ///
  /// If this is S3 Resume or Warm Boot, check if PEG delay can be skipped when no PEG devices populated.
  ///
  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);
  if (PmcIsMemoryPlacedInSelfRefreshState()){
    WarmReset = TRUE;
  }

  if (((BootMode == BOOT_ON_S3_RESUME) || (WarmReset == TRUE)) &&
      (SaPegHob != NULL)                                                     &&
      (SaPegHob->PegData.PegLinkFailMask != 0)) {
    //
    // During warm reboot, there is a possibility of the user inserting or removing a PCIe card.
    // So check for change in topology.
    //
    if (WarmReset == TRUE) {
      IsPcieTopologyChanged (PcieAccess, PciePorts, PciePortsLength, SaPegHob);
    }
    PegLinkFailMask = SaPegHob->PegData.PegLinkFailMask;
    DEBUG ((DEBUG_INFO, "Previous Link Training Fail Mask 0x%2.2X\n", SaPegHob->PegData.PegLinkFailMask));
  }
  DEBUG ((DEBUG_INFO, "New Link Training Fail Mask 0x%2.2X\n", PegLinkFailMask));

  ///
  /// Convert the Fail Mask Bitmap from each bit representing a PCIe function
  /// number to each bit representing an array index.
  ///
  for (Index = 0; Index < PciePortsLength; Index++) {
    if ((PciePorts[Index].Device == SA_PEG0_DEV_NUM) &&
        ((((UINT8) (0x1 << PciePorts[Index].Function)) & PegLinkFailMask) == 0))
    {
      FailMask[Index / 32] &= (UINT32) ~(0x1 << (PciePorts[Index].Function));
    }
    if ((PciePorts[Index].Device == SA_PEG3_DEV_NUM) &&
        (((B_PCIE_PEG3_BITMAP) & PegLinkFailMask) == 0))
    {
      FailMask[Index / 32] &= (UINT32) ~(B_PCIE_PEG3_BITMAP);
    }
  }
  ///
  /// If all PEG root ports are disabled, then no need to wait for VC0 pending to be complete
  ///
  AllRpsDisabled = TRUE;
  for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
    if (FailMask[Index] != 0xFFFFFFFF) {
      AllRpsDisabled = FALSE;
      break;
    }
  }
  if (AllRpsDisabled) {
    DEBUG ((DEBUG_INFO, "All active PEG Root Ports are disabled, skipping VC0 check\n"));
  } else {
    ///
    /// Call generic VC0 Wait function
    ///
    WaitForDataLinkLayerLinkActiveOnAllPorts (PcieAccess, PciePorts, PciePortsLength, FALSE, &(FailMask[0]));
  }

  ///
  /// Update the SaPegHob with the new fail mask
  ///
  if (SaPegHob != NULL) {
    PegLinkFailMask = 0xFF;
    for (Index = 0; Index < PciePortsLength; Index++) {
      if ((PciePorts[Index].Device == SA_PEG0_DEV_NUM) &&
          ((FailMask[Index / 32] & (0x1 << (PciePorts[Index].Function))) == 0)) {
        PegLinkFailMask &= (UINT8) ~(0x1 << PciePorts[Index].Function);
      }
      if ((PciePorts[Index].Device == SA_PEG3_DEV_NUM) &&
          ((FailMask[Index / 32] & (B_PCIE_PEG3_BITMAP)) == 0)) {
        PegLinkFailMask &= (UINT8) ~(B_PCIE_PEG3_BITMAP);
      }
    }
    SaPegHob->PegData.PegLinkFailMask = PegLinkFailMask;
    DEBUG ((DEBUG_INFO, "Returned PegLinkFailMask 0x%2.2X\n", PegLinkFailMask));
  }
}

/**
 This function performs the GEN2 Auto Speed Disable

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to wait for
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
Gen2AutoSpeedDisable (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  UINT32                          FailMask[PCIE_ROOT_PORT_BITMAP_LENGTH];
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  UINTN                           PortIndex;
  UINT8                           DisableAutoSpeedUp;

  PciePrivate        = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) PcieAccess->PrivateData;
  DisableAutoSpeedUp = 0;

  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {

#ifdef UP_SERVER_FLAG
	if ((SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePrivate->PciePeiPreMemConfig) == PEG_GEN1)  || 
        (SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePrivate->PciePeiPreMemConfig))) {
      continue; 
    }
#endif
	
	///
    /// If Gen2 endpoint was detected or If VC0 is still pending
    /// And presence of an endpoint was detected, enable GEN2 auto speed disable
    ///
#ifndef UP_SERVER_FLAG
    if ((!PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]))) &&
        PcieAccess->GetSlotPresenceDetect (PcieAccess, &(PciePorts[PortIndex]))     &&
        !SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePrivate->PciePeiPreMemConfig))
#else
    if ((PciePorts[PortIndex].EndpointMaxLinkSpeed == 0x2                        ||
        (!PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex])))) &&
        PcieAccess->GetSlotPresenceDetect (PcieAccess, &(PciePorts[PortIndex]))     &&
        !SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePrivate->PciePeiPreMemConfig))
#endif
    {
  	  DisableAutoSpeedUp |= 1 << PciePorts[PortIndex].Function;
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), TRUE);
      PciePrivate->SetDisableAutoSpeedUp (PcieAccess, &(PciePorts[PortIndex]), TRUE);
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), FALSE);
	}
#ifdef UP_SERVER_FLAG
    else if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePrivate->PciePeiPreMemConfig) == PEG_GEN2) {
      DisableAutoSpeedUp |= 1 << PciePorts[PortIndex].Function;
	}
#endif
  }
  ///
  /// If needed, reinitialize link after disable
  ///
  if (DisableAutoSpeedUp != 0) {
    WaitForDataLinkLayerLinkActiveOnAllPorts (PcieAccess, PciePorts, PciePortsLength, TRUE, &(FailMask[0]));

    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if (((DisableAutoSpeedUp >> PciePorts[PortIndex].Function) & 1) == 1) {
#ifdef UP_SERVER_FLAG
	    ///
        /// Go to Gen2
        ///
        PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 2);
#endif
		///
        /// Retrain to allow link to reach GEN2
        ///
#ifdef UP_SERVER_FLAG
        DEBUG ((DEBUG_INFO, "RetrainLink to Gen2 : Port %x\n",PortIndex));
#endif
		PcieAccess->RetrainLink (PcieAccess, &(PciePorts[PortIndex]));
      }
    }
    ///
    /// Ensure all links are now ready
    ///
    WaitForDataLinkLayerLinkActiveOnAllPorts (PcieAccess, PciePorts, PciePortsLength, TRUE, &(FailMask[0]));
    DEBUG ((DEBUG_INFO, "PEG Link Status after auto speed disable:\n"));
  }
}

#ifdef CPU_CFL
/**
 This function checks if icomp needs to be enabled if there are untrained endpoints detected
 to save power.

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to wait for
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
IcompEnableCheck (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  UINT32                          FailMask[PCIE_ROOT_PORT_BITMAP_LENGTH];
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  UINTN                           PortIndex;
  UINT8                           UntrainedEndpointMask;

  PciePrivate        = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) PcieAccess->PrivateData;

  ///
  /// Check if there is at least one PEG with VC0 is still pending and endpoint presence detected
  ///
  UntrainedEndpointMask = 0;
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if ((!PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]))) &&
        PcieAccess->GetSlotPresenceDetect (PcieAccess, &(PciePorts[PortIndex]))     &&
        !SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePrivate->PciePeiPreMemConfig)) {
      UntrainedEndpointMask |= (1 << PortIndex);
    }
  }
  ///
  /// If there are no untrained endpoints, it is finished.
  ///
  if (UntrainedEndpointMask == 0) {
    DEBUG ((DEBUG_INFO, "Icomp check: no untrained endpoints found\n"));
    return;
  }
  ///
  /// For all untrained endpoint detected, retry link training with icomp enabled
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (((UntrainedEndpointMask >> PortIndex) & 0x1) == 1) {
      PciePrivate->SetRootPortIcompDisable (PcieAccess, &PciePorts[PortIndex], FALSE);
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), TRUE);
      MicroSecondDelay (STALL_ONE_MICRO_SECOND);
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), FALSE);
    }
  }
  WaitForDataLinkLayerLinkActiveOnAllPorts (PcieAccess, PciePorts, PciePortsLength, TRUE, &(FailMask[0]));
  ///
  /// Again check if at least one PEG has VC0 is still pending and endpoint presence detected
  ///
  UntrainedEndpointMask = 0;
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if ((!PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]))) &&
        PcieAccess->GetSlotPresenceDetect (PcieAccess, &(PciePorts[PortIndex]))     &&
        !SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePrivate->PciePeiPreMemConfig)) {
      UntrainedEndpointMask |= (1 << PortIndex);
    }
  }
  ///
  /// If there are no untrained endpoints, it is finished.
  ///
  if (UntrainedEndpointMask == 0) {
    DEBUG ((DEBUG_INFO, "Icomp check: no untrained endpoints found\n"));
    return;
  }
  ///
  /// Since at least one untrained endpoint was still detected, revert icomp to disable for
  /// the root port and retrain the link
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (((UntrainedEndpointMask >> PortIndex) & 0x1) == 1) {
      PciePrivate->SetRootPortIcompDisable (PcieAccess, &PciePorts[PortIndex], TRUE);
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), TRUE);
      MicroSecondDelay (STALL_ONE_MICRO_SECOND);
      PcieAccess->SetLinkDisable (PcieAccess, &(PciePorts[PortIndex]), FALSE);
    }
  }
}
#endif

/**
  Converts information from the SA_PEG_HOB and SI_SA_POLICY_PPI into a
  format that the PcieGen3SoftwareEqualization() function can use.  Afterwards,
  the PcieGen3SoftwareEqualization() function is called.

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to wait for
  @param[in]  PciePortsLength             - Length of the PciePorts array
  @param[in]  PciePeiPreMemConfig         - PCIE_PEI_PREMEM_CONFIG to access policy data
  @param[in]  SaPegHob                    - The HOB containing SA data
**/
VOID
EFIAPI
SaPcieGen3SoftwareEqualization (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  PCIE_PEI_PREMEM_CONFIG            *PciePeiPreMemConfig,
  IN  SA_PEG_HOB                        *SaPegHob
  )
{
  PCIE_SWEQ_PORT_INPUT        PortConfigs[SA_PEG_MAX_FUN];
  PCIE_SWEQ_PORT_OUTPUT       PortOutputs[SA_PEG_MAX_FUN];
  PCIE_SWEQ_INPUT_PARAMETERS  InputParameters;
  PCIE_SWEQ_OUTPUT            OutputData;
  PCIE_SWEQ_GPIO_CONFIG       GpioConfig;
  UINT8                       *EndPointPreset;
  UINT8                       Lane;
  UINT8                       PortIndex;
  UINT8                       LaneIndex;
  UINT8                       PresetIndex;
  SA_DATA_HOB                 *SaDataHob;

  //
  // Initialize the default value for GpioConfig
  //
  GpioConfig.EnableGpioPerstSupport = FALSE;
  GpioConfig.GpioPad                = 0;
  GpioConfig.GpioActiveHigh         = 0;

  ///
  /// Convert data from the PciePeiPreMemConfig and SaPegHob in to the generic
  /// structures expected by SW EQ
  ///
  EndPointPreset = PciePeiPreMemConfig->PegGen3EndPointPreset;

  if (PciePeiPreMemConfig->PegGpioData.GpioSupport != 0) {
    GpioConfig.EnableGpioPerstSupport = TRUE;
  } else {
    GpioConfig.EnableGpioPerstSupport = FALSE;
  }

  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    ///
    /// PEG0/1/2 only has 1 PERST# GPIO for all PEG ports
    ///
    if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM && PciePorts[PortIndex].Function == SA_PEG0_FUN_NUM) {
      GpioConfig.GpioPad        = PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.GpioPad;
      GpioConfig.GpioActiveHigh = (UINT8) (UINTN) PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.Active;
    } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
      GpioConfig.GpioPad        = PciePeiPreMemConfig->PegGpioData.SaPeg3ResetGpio.GpioPad;
      GpioConfig.GpioActiveHigh = (UINT8) (UINTN) PciePeiPreMemConfig->PegGpioData.SaPeg3ResetGpio.Active;
    }
    ///
    /// Convert Port Level Input Parameters
    ///
    PortConfigs[PortIndex].PciePort = &(PciePorts[PortIndex]);
    for (LaneIndex = 0; LaneIndex < PciePorts[PortIndex].MaxPortLaneListLength; LaneIndex++) {
      Lane                                                    = PciePorts[PortIndex].MaxPortLaneList[LaneIndex];
      PortConfigs[PortIndex].StaticEndpointPresets[LaneIndex] = EndPointPreset[Lane];
      if (SaPegHob != NULL) {
        PortConfigs[PortIndex].LastBootBestPresets[LaneIndex] = SaPegHob->PegData.BestPreset[Lane];
      }
    }
    if (SaPegHob != NULL) {
      PortConfigs[PortIndex].LastBootEndpointVendorIdDeviceId = SaPegHob->PegData.EndPointVendorIdDeviceId[PciePorts[PortIndex].Function];
    }
    PortConfigs[PortIndex].EnableSwEq = SaPolicySwEqEnabledOnPort (&(PciePorts[PortIndex]), PciePeiPreMemConfig);
    PortConfigs[PortIndex].GpioConfig = GpioConfig;
    ///
    /// Convert Port Level Output Parameters
    ///
    PortOutputs[PortIndex].PciePort = &(PciePorts[PortIndex]);
    for (LaneIndex = 0; LaneIndex < PCIE_MAX_LANE; LaneIndex++) {
      PortOutputs[PortIndex].TempMarginData[LaneIndex] = -1;   ///< -1 is a special value that means that the lane was not margined
      PortOutputs[PortIndex].BestPresets[LaneIndex]    = 7;
      PortOutputs[PortIndex].BestScores[LaneIndex]     = 0;
    }
    for (PresetIndex = 0; PresetIndex < PCIE_SWEQ_MAX_PRESETS; PresetIndex++) {
      for (LaneIndex = 0; LaneIndex < PCIE_MAX_LANE; LaneIndex++) {
        PortOutputs[PortIndex].PresetScores[PresetIndex].MarginScore[LaneIndex] = -1;
      }
      PortOutputs[PortIndex].PresetScores[PresetIndex].Preset = 7;
    }
  }

  ///
  /// Convert Input Parameters
  ///
  InputParameters.PerPortInputParameters = &(PortConfigs[0]);
  InputParameters.PerPortInputParametersLength = PciePortsLength;
  if (SaPegHob != NULL) {
    InputParameters.HaveDataFromLastBoot = TRUE;
  } else {
    InputParameters.HaveDataFromLastBoot = FALSE;
  }
  InputParameters.PresetsToTest[0] = 7;
  InputParameters.PresetsToTest[1] = 8;
  InputParameters.PresetsToTest[2] = 3;
  InputParameters.PresetsToTest[3] = 5;
  InputParameters.PresetsToTest[4] = 0;
  InputParameters.PresetsToTest[5] = 1;
  InputParameters.PresetsToTest[6] = 2;
  InputParameters.PresetsToTest[7] = 6;
  InputParameters.PresetsToTest[8] = 9;
  switch (PciePeiPreMemConfig->Gen3SwEqNumberOfPresets) {
    case 1:
      InputParameters.PresetsToTestLength = MAX_PRESETS;
      break;
    case 0:
    case 2:
    default:
      InputParameters.PresetsToTestLength = BEST_PRESETS;
      break;
  }
  if (PciePeiPreMemConfig->Gen3SwEqAlwaysAttempt == 1) {
    InputParameters.AlwaysAttemptSwEq = TRUE;
  } else {
    InputParameters.AlwaysAttemptSwEq = FALSE;
  }
  InputParameters.JitterDwellTime   = (UINTN) PciePeiPreMemConfig->Gen3SwEqJitterDwellTime;
  InputParameters.JitterErrorTarget = PciePeiPreMemConfig->Gen3SwEqJitterErrorTarget;
  InputParameters.VocDwellTime      = (UINTN) PciePeiPreMemConfig->Gen3SwEqVocDwellTime;
  InputParameters.VocErrorTarget    = PciePeiPreMemConfig->Gen3SwEqVocErrorTarget;
  switch (PciePeiPreMemConfig->Gen3SwEqEnableVocTest) {
    case 0:
      InputParameters.EnableVocTest = FALSE;
      break;
    case 1:
    case 2:
    default:
      InputParameters.EnableVocTest = TRUE;
      break;
  }
  //
  //@todo BDAT support is presently not implemented, will be added in a future release
  //
  /*if (MiscPeiPreMemConfig->BdatEnable) {
    InputParameters.EnableBdatScoreSchema = TRUE;
  } else {
    InputParameters.EnableBdatScoreSchema = FALSE;
  }*/
  InputParameters.EnableBdatScoreSchema = FALSE;

  ///
  /// Convert Output Parameters
  ///
  OutputData.PortOutputList                 = &(PortOutputs[0]);
  OutputData.PortOutputListLength           = PciePortsLength;
  OutputData.DeferredPlatformResetRequired  = FALSE;

  ///
  /// Data converted, invoke Software Equalization
  ///
  PcieGen3SoftwareEqualization (PcieAccess, PciePorts, PciePortsLength, &InputParameters, &OutputData);

  ///
  /// Convert Output in to SA_PEG_HOB format
  ///
  if (SaPegHob != NULL) {
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      for (LaneIndex = 0; LaneIndex < PciePorts[PortIndex].MaxPortLaneListLength; LaneIndex++) {
        Lane                                = PciePorts[PortIndex].MaxPortLaneList[LaneIndex];
        SaPegHob->PegData.BestPreset[Lane] = OutputData.PortOutputList[PortIndex].BestPresets[LaneIndex];
      }
      if (PciePorts[PortIndex].EndpointPresent) {
        SaPegHob->PegData.EndPointVendorIdDeviceId[PciePorts[PortIndex].Function] = PciePorts[PortIndex].EndpointVendorIdDeviceId;
      } else {
        SaPegHob->PegData.EndPointVendorIdDeviceId[PciePorts[PortIndex].Function] = 0xFFFFFFFF;
      }
    }
    ///
    /// Set SA Data hob to request a reset if required.
    ///
    SaDataHob = NULL;
    SaDataHob = (SA_DATA_HOB *) GetFirstGuidHob (&gSaDataHobGuid);
    if (SaDataHob != NULL) {
      SaDataHob->PegPlatformResetRequired = OutputData.DeferredPlatformResetRequired;
    }
  }
}

/**
  Initialize the SA PciExpress in PEI

  @param[in] IN SA_MISC_PEI_PREMEM_CONFIG MiscPeiPreMemConfig to access Platform relevant information
  @param[in] IN PCIE_PEI_PREMEM_CONFIG    PciePeiPreMemConfig to access the PCIe Config related information
**/
VOID
PciExpressInit (
  IN  SA_MISC_PEI_PREMEM_CONFIG     *MiscPeiPreMemConfig,
  IN  PCIE_PEI_PREMEM_CONFIG        *PciePeiPreMemConfig
  )
{
  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  PcieAccessStructure;
  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess;
  SA_PCIE_PRIVATE_FUNCTION_CALLS    PciePrivateDataStructure;
  SA_PCIE_PRIVATE_FUNCTION_CALLS    *PciePrivate;
  PCIE_PORT_INFO                    PciePorts[SA_PEG_MAX_FUN];
  PCIE_SWEQ_INPUT_PARAMETERS        InputParameters;
  PCIE_SWEQ_PORT_INPUT              PortConfigs[SA_PEG_MAX_FUN];
  PCIE_SWEQ_GPIO_CONFIG             GpioConfig;
  EFI_BOOT_MODE                     BootMode;
  UINT8                             PciePortsLength;
  SA_PEG_HOB                        *SaPegHob;
  SA_CONFIG_HOB                     *SaConfigHob;
  EFI_STATUS                        Status;
  UINT8                             PortIndex;
  UINT8                             Lane;
  UINT8                             PwrDnUnusedBundlesSetupData;
  UINT8                             PegDisableMask;
  UINT8                             LinkStatusGood;
  UINT8                             CurrentLinkWidth;
  BOOLEAN                           Gen3Capable;
  BOOLEAN                           SwEqEnabled;
#ifndef CPU_CFL
  UINT64                            PegBaseAddress;
#endif
  CPU_STEPPING                      CpuSteppingId;
  GpioConfig.EnableGpioPerstSupport = FALSE;
  GpioConfig.GpioPad = 0;
  GpioConfig.GpioActiveHigh = 0;
  CpuSteppingId = GetCpuStepping ();

  ///
  /// Get the Low Level Function Calls
  ///
  Status = GetSaPcieLowLevelFunctionCalls (
             PciePeiPreMemConfig,
             &PcieAccessStructure
             );
  if (EFI_ERROR (Status)) {
    return;
  }
  Status = GetSaPciePrivateData (
             PciePeiPreMemConfig,
             MiscPeiPreMemConfig,
             &PciePrivateDataStructure
             );
  if (EFI_ERROR (Status)) {
    return;
  }
  PciePrivate                     = &PciePrivateDataStructure;
  PcieAccessStructure.PrivateData = (VOID*) PciePrivate;
  PcieAccess                      = &PcieAccessStructure;

  ///
  /// Check to see if SA PCIe exists and leave initialization function if non-existent
  ///
  if (!PcieAccess->PcieExists (PcieAccess)) {
    DEBUG ((DEBUG_INFO, "PEG controller not detected\n"));
    FlowControlCreditProgrammingNoPegLib ();
    return;
  }

  ///
  /// Determine PEG topology
  ///
  PcieAccess->GetPcieRootPorts (PcieAccess, &(PciePorts[0]), &PciePortsLength);
  ASSERT (PciePortsLength <= SA_PEG_MAX_FUN);
  if (PciePortsLength > SA_PEG_MAX_FUN) {
    return;
  }

  ///
  /// Check to make sure Ph2 and Ph3 options are compatible
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (!SaPolicyGetEqPhase2Enable (&(PciePorts[PortIndex]), PciePeiPreMemConfig)) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
        switch (PciePorts[PortIndex].Function) {
        case 0:
          if ((PciePeiPreMemConfig->Peg0Gen3EqPh3Method == PH3_METHOD_AUTO) ||
              (PciePeiPreMemConfig->Peg0Gen3EqPh3Method == PH3_METHOD_HWEQ)) {
            ///
            /// If Ph2 is disabled we can't enable Ph3
            ///
            PciePeiPreMemConfig->Peg0Gen3EqPh3Method = PH3_METHOD_DISABLED;
          }
          break;
        case 1:
          if((PciePeiPreMemConfig->Peg1Gen3EqPh3Method == PH3_METHOD_AUTO) ||
             (PciePeiPreMemConfig->Peg1Gen3EqPh3Method == PH3_METHOD_HWEQ)) {
            ///
            /// If Ph2 is disabled we can't enable Ph3
            ///
            PciePeiPreMemConfig->Peg1Gen3EqPh3Method = PH3_METHOD_DISABLED;
          }
          break;
        case 2:
          if((PciePeiPreMemConfig->Peg2Gen3EqPh3Method == PH3_METHOD_AUTO) ||
             (PciePeiPreMemConfig->Peg2Gen3EqPh3Method == PH3_METHOD_HWEQ)) {
            ///
            /// If Ph2 is disabled we can't enable Ph3
            ///
            PciePeiPreMemConfig->Peg2Gen3EqPh3Method = PH3_METHOD_DISABLED;
          }
          break;
        }
      } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM){
        if ((PciePeiPreMemConfig->Peg3Gen3EqPh3Method == PH3_METHOD_AUTO) ||
            (PciePeiPreMemConfig->Peg3Gen3EqPh3Method == PH3_METHOD_HWEQ)) {
          ///
          /// If Ph2 is disabled we can't enable Ph3
          ///
          PciePeiPreMemConfig->Peg3Gen3EqPh3Method = PH3_METHOD_DISABLED;
        }
      }
    }
  }


  ///
  /// Restore SA Data HOB's PEG data
  ///
  SaPegHob = (SA_PEG_HOB *) GetFirstGuidHob (&gSaPegHobGuid);

  if (SaPegHob != NULL) {
    SwEqEnabled = SaPolicyEnablesSwEq (PciePeiPreMemConfig, &(PciePorts[0]), PciePortsLength);
    if (PciePeiPreMemConfig->PegDataPtr != NULL) {
      DEBUG ((DEBUG_INFO, "\nRestore SA PEG DATA from previous boot: Size=%X\n", sizeof (SA_PEG_DATA)));
      CopyMem (&(SaPegHob->PegData), PciePeiPreMemConfig->PegDataPtr, sizeof (SA_PEG_DATA));
      if ((SaPegHob->PegData.PegGen3PresetSearch != (UINT8) SwEqEnabled) && (SwEqEnabled == FALSE)) {
        ///
        /// Zero out previous boot GEN3 Preset data so old data won't be re-used when PegGen3PresetSearch re-enabled later
        ///
        DEBUG ((DEBUG_INFO, "\nGen3 Software Equalization is disabled, Clear old Preset data\n"));
        for (PortIndex = 0; PortIndex < SA_PEG_MAX_FUN; PortIndex++) {
          SaPegHob->PegData.EndPointVendorIdDeviceId[PortIndex] = 0;
          for (Lane = 0; Lane < SA_PEG_MAX_LANE; Lane++) {
            SaPegHob->PegData.BestPreset[Lane] = 0;
          }
        }
      }
    }
    ///
    /// Locate SaConfigHob
    ///
    SaConfigHob = (SA_CONFIG_HOB *) GetFirstGuidHob (&gSaConfigHobGuid);

    ///
    /// Copy Bundle Power Down Settings to SaPegHob
    ///
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      PwrDnUnusedBundlesSetupData = SaPolicyGetPowerDownUnusedLanes (&(PciePorts[PortIndex]), PciePeiPreMemConfig);
      if (PwrDnUnusedBundlesSetupData != 0) {
        PwrDnUnusedBundlesSetupData = 0xff;
      }
      if (SaConfigHob) {
        SaConfigHob->PowerDownUnusedBundles[PortIndex] = PwrDnUnusedBundlesSetupData;
      }
    }
    SaPegHob->PegData.PegGen3PresetSearch = (UINT8) SwEqEnabled;
  }

  ///
  /// PEG Pre-Detection Programming and Recipe
  ///
  PciePrivate->PreDetectionProgramming (PcieAccess, &(PciePorts[0]), PciePortsLength);

  ///
  /// Perform PEG Gen3 Equalization steps and load preset values
  ///
  Gen3Capable = PciePrivate->IsGen3Capable (PcieAccess);
  if (Gen3Capable) {
    if (PciePeiPreMemConfig->PegGen3ProgramStaticEq != 0) {
      DEBUG ((DEBUG_INFO, "Program PEG Gen3 Static Equalization...\n"));
      PcieGen3StaticEq (PcieAccess, PciePeiPreMemConfig, &(PciePorts[0]), PciePortsLength);
    }
  }

  ///
  /// Program the PEG speed & width according to SA policy options before
  /// endpoint enumeration, so that uncompliant card can train at a lower speed.
  ///
  PciePrivate->ConfigureMaxSpeedWidth (PcieAccess, &(PciePorts[0]), PciePortsLength, Gen3Capable);

  ///
  /// RxCEM Loopback (LPBK) Mode
  ///
  if (PciePeiPreMemConfig->PegRxCemTestingMode == 1) {
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      PciePrivate->EnableRxCemLoopbackMode (PcieAccess, (UINT8) PciePeiPreMemConfig->PegRxCemLoopbackLane, &(PciePorts[PortIndex]));
    }
  }

  ///
  /// Disable Spread Spectrum Clocking if required by policy
  ///
  if (PciePeiPreMemConfig->PegDisableSpreadSpectrumClocking == 1) {
    PciePrivate->DisableSpreadSpectrumClocking (PcieAccess);
  }

  ///
  /// Bypass Phase 2 if needed
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (SaPolicySwEqEnabledOnPort (&(PciePorts[PortIndex]), PciePeiPreMemConfig) ||
        (SaPolicyGetEqPhase3Method (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == PH3_METHOD_DISABLED) ||
        (!SaPolicyGetEqPhase2Enable (&(PciePorts[PortIndex]), PciePeiPreMemConfig))) {
      Status = PcieAccess->SetPhase2Bypass (PcieAccess, &(PciePorts[PortIndex]), 1, TRUE);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_WARN, "Error setting Phase2 bypass for Bus:%x Device:%x Function:%x\n", PciePorts[PortIndex].Bus, PciePorts[PortIndex].Device, PciePorts[PortIndex].Function));
      }
    }
  }

  ///
  /// Clear DEFER_OC on all PCIe controllers to start the training sequence
  ///

  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (GetCpuGeneration () == EnumCnlCpu) {
      ///
      if ( !(GetCpuSku () == EnumCpuHalo && CpuSteppingId == EnumCnlG0) ) {
        PciePrivate->ProgramVRefRxDet (PcieAccess, &(PciePorts[PortIndex]));
      }
      if (CpuSteppingId == EnumCnlP0) {
        ///
        ///
        PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_AFEOVR_OFFSET, (UINT32) (BIT18 | BIT19));
      }
    }
    PciePrivate->ClearDeferOc (PcieAccess, &(PciePorts[PortIndex]));
  }

  ///
  /// Delay for 100ms to meet the timing requirements of the PCI Express Base
  /// Specification, Revision 1.0A, Section 6.6 ("...software must wait at least
  /// 100 ms from the end of reset of one or more device before it is permitted
  /// to issue Configuration Requests to those devices").
  ///
  SaPcieWaitForDataLinkLayerLinkActive (PcieAccess, &(PciePorts[0]), PciePortsLength, SaPegHob);

  ///
#ifndef UP_SERVER_FLAG
  Gen2AutoSpeedDisable (PcieAccess, &(PciePorts[0]), PciePortsLength);
#endif

#ifndef CPU_CFL
  if (CpuSteppingId == EnumCnlP0) {
    PciePrivate->SetRootPortRXSQEXCTL (0x0004);
  }
#endif

  ///
  /// Detect endpoint presence and determine what the link's capabilities are
  ///
  DEBUG ((DEBUG_INFO, "Presence detect table...\n"));
  PcieAccess->DetectEndpointPresence (PcieAccess, &(PciePorts[0]), PciePortsLength);
#ifdef UP_SERVER_FLAG
  Gen2AutoSpeedDisable (PcieAccess, &(PciePorts[0]), PciePortsLength);
#endif

  PeiServicesGetBootMode(&BootMode);
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    ///
    /// PEG0/1/2 only has 1 PERST# GPIO for all PEG ports
    ///
    if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM && PciePorts[PortIndex].Function == SA_PEG0_FUN_NUM &&
        BootMode != BOOT_ON_S3_RESUME && PmcIsMemoryPlacedInSelfRefreshState () && IsPchH ()) {
      if (PciePeiPreMemConfig->PegGpioData.GpioSupport != 0) {
        GpioConfig.EnableGpioPerstSupport = TRUE;
      } else {
        GpioConfig.EnableGpioPerstSupport = FALSE;
      }

      GpioConfig.GpioPad        = PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.GpioPad;
      GpioConfig.GpioActiveHigh = (UINT8) (UINTN) PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.Active;
      PortConfigs[PortIndex].PciePort   = &(PciePorts[PortIndex]);
      PortConfigs[PortIndex].GpioConfig = GpioConfig;
      InputParameters.PerPortInputParameters = &(PortConfigs[PortIndex]);
      InputParameters.PerPortInputParametersLength = PciePortsLength;
      PcieAccess->ResetEndpointPerst(PcieAccess, &(PciePorts[PortIndex]), &InputParameters);
    }
  }

  ///
  /// Clear endpoint presence on disabled ports so no additional unneeded work is done
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].EndpointPresent &&
        SaPolicyForceDisablesPort (&PciePorts[PortIndex], PciePeiPreMemConfig)) {
      DEBUG ((
               DEBUG_INFO,
               " PCIe RP (%x:%x:%x) - RP is Disabled by policy, clearing endpoint data.\n",
               PciePorts[PortIndex].Bus,
               PciePorts[PortIndex].Device,
               PciePorts[PortIndex].Function
               ));
      PciePorts[PortIndex].EndpointPresent          = FALSE;
      PciePorts[PortIndex].EndpointVendorIdDeviceId = 0xFFFFFFFF;
      PciePorts[PortIndex].EndpointMaxLinkSpeed     = 0;
      PciePorts[PortIndex].EndpointMaxLinkWidth     = 0;
      PciePorts[PortIndex].SwEqData.MaxCapableSpeed = 0;
      PciePorts[PortIndex].SwEqData.MaxCapableWidth = 0;
    }
  }

  ///
  /// Equalization programming that needs to be done post-detection
  ///
#ifdef CFL_SIMICS
  PciePrivate->PostDetectionEqProgramming (PcieAccess, &(PciePorts[0]), PciePortsLength);
#else
  PciePrivate->PostDetectionEqProgramming (PcieAccess, &(PciePorts[0]), PciePortsLength);
#endif
  ///
  /// Software Equalization
  ///

  if (SaPolicyEnablesSwEq (PciePeiPreMemConfig, &(PciePorts[0]), PciePortsLength) && SaPolicyEnablesGen3 (PciePeiPreMemConfig, &(PciePorts[0]), PciePortsLength)) {
    SaPcieGen3SoftwareEqualization (PcieAccess, &(PciePorts[0]), PciePortsLength, PciePeiPreMemConfig, SaPegHob);
  }

  ///
  /// Bypass Ph2/3 Eq depending on SA PCIe Configuration
  ///
  PciePrivate->EqPh2Ph3BypassProgramming (PcieAccess, &(PciePorts[0]), PciePortsLength);

  ///
  /// Determine which ports to disable and perform post detection additional programming
  ///
  PegDisableMask = V_PEG_DISABLE_MASK;
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    ///
    /// Force retrain link width to be match SA PCIe Configuration
    ///
    if ((SaPolicyGetPegMaxLinkWidth (&(PciePorts[PortIndex]), PciePeiPreMemConfig) != 0) &&
        PciePorts[PortIndex].EndpointPresent) {
      CurrentLinkWidth = PcieAccess->GetNegotiatedWidth (PcieAccess, &PciePorts[PortIndex]);
      if (SaPolicyGetPegMaxLinkWidth (&(PciePorts[PortIndex]), PciePeiPreMemConfig) != CurrentLinkWidth) {
        PciePrivate->SetLinkWidth (
                       PcieAccess,
                       &PciePorts[PortIndex],
                       SaPolicyGetPegMaxLinkWidth (&(PciePorts[PortIndex]), PciePeiPreMemConfig)
                       );
      }
    }
    if (PciePorts[PortIndex].EndpointPresent) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
          PegDisableMask &= (UINT8) ~(B_PEG0_DISABLE_MASK << (PciePorts[PortIndex].Function));
      } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
        PegDisableMask &= (UINT8) ~(B_PEG3_DISABLE_MASK);
      }
      PciePrivate->PostDetectionAdditionalProgramming (
                     PcieAccess,
                     &PciePorts[PortIndex]
                     );
    }
    if (SaPolicyGetPowerDownUnusedLanes (&(PciePorts[PortIndex]), PciePeiPreMemConfig) != 0) {
      PciePrivate->PowerDownUnusedLanes (PcieAccess, &PciePorts[PortIndex]);
    }
  }
  ///
  /// If a port is forced to enable/disable then always enable/disable it
  ///
  for (PortIndex = 0; PortIndex < SA_PEG_MAX_FUN; PortIndex++) {
    if (SaPolicyForceEnablesPort (&PciePorts[PortIndex], PciePeiPreMemConfig)) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
        PegDisableMask &= (UINT8) ~(B_PEG0_DISABLE_MASK << (PciePorts[PortIndex].Function));
      } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
        PegDisableMask &= (UINT8) ~(B_PEG3_DISABLE_MASK);
      }
    } else if (SaPolicyForceDisablesPort (&(PciePorts[PortIndex]), PciePeiPreMemConfig)) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
        PegDisableMask |= (UINT8) (B_PEG0_DISABLE_MASK << (PciePorts[PortIndex].Function));
      } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
        PegDisableMask |= (UINT8) (B_PEG3_DISABLE_MASK);
      }
    }
  }

  ///
  /// Disable Unused PEG Controllers
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    PciePrivate->DisableUnusedPcieControllers (PcieAccess, PegDisableMask, &PciePorts[PortIndex]);
  }

  ///
  /// Report Link Status
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
      if ((PegDisableMask & (B_PEG0_DISABLE_MASK << (PciePorts[PortIndex].Function))) == 0) {
        WaitForDataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]));
        PcieAccess->ReportPcieLinkStatus (PcieAccess, &(PciePorts[PortIndex]));
      }
    } else if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
      if ((PegDisableMask & (B_PEG3_DISABLE_MASK)) == 0) {
        WaitForDataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]));
        PcieAccess->ReportPcieLinkStatus (PcieAccess, &(PciePorts[PortIndex]));
      }
    }
  }

  ///
  /// Re-check Link again and see if PegLinkFailMask in SaPegHob needs updating
  ///
  if (SaPegHob != NULL) {
    LinkStatusGood = 0;
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PegDisableMask & (0x1 << (PciePorts[PortIndex].Function))) == 0) {
        if (PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[PortIndex]))) {
          LinkStatusGood |= (0x1 << (PciePorts[PortIndex].Function));
        }
      }
    }
    if (SaPegHob->PegData.PegLinkFailMask != (UINT8) (~LinkStatusGood)) {
      DEBUG ((DEBUG_INFO, "Original PegLinkFailMask=%X, Final PegLinkFailMask=%X\n", SaPegHob->PegData.PegLinkFailMask, (UINT8) (~LinkStatusGood)));
      SaPegHob->PegData.PegLinkFailMask = (UINT8) (~LinkStatusGood);
    }
  }

/* @todo BDAT support not yet ready.
#ifdef BDAT_SUPPORT
  if (MiscPeiPreMemConfig->BdatEnable &&
      (PciePeiPreMemConfig->PegGenerateBdatMarginTable != 0)) {
    PegGenerateMarginData (PciePeiPreMemConfig, SaPegHob);
  }
#endif*/
#ifndef CPU_CFL
  //
  // Program PTM Pipe stage delays
  //
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, PciePorts[PortIndex].Bus, PciePorts[PortIndex].Device, PciePorts[PortIndex].Function, 0);
    if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
      PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC1, 0xC02FE727);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC2, 0x7715A633);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC3, 0x511B5D19);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC4, 0x390F450D);
      PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC5, 0x9B353311);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC6, 0x3211501A);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC7, 0x4B19952F);
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC8, 0xFFFF0000, 0x00003019);
  }
  if (PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) {
      PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC1, 0xC02FE727);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC2, 0x7715A633);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC3, 0x511B4719);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC4, 0x390F450D);
      PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PCIE_CFG_PTMPSDC5, 0xFFFF0000, 0x00003311);
  }
  //
  // Program PTM Extended Configuration Register - Note: Software is expected to program this register prior to setting PTM Enable
  //
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PCIE_CFG_PTMECFG, 0xFFFFFF8F, BIT6 | BIT4);
    //
    // Program PTM CAPR and CTLR
    //
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PCIE_CFG_PTMCAPR, 0xFFFFFFF9, BIT2 | BIT1);
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PCIE_CFG_PTMCTLR, 0xFFFFFFFC, BIT1 | BIT0);

    //
    // Program ACS Control Register
    //
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_ACSCTLR_OFFSET, 0xFFFFFFE0, BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
  }
#endif
  //
  // Program flow control credits according to the root port configuration
  //
  PciePrivate->FlowControlCreditProgramming (PcieAccess, PegDisableMask);

  ///
  /// Program Spine clock Gating
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    PciePrivate->SpineClockGatingProgramming (PcieAccess, &(PciePorts[PortIndex]));
  }

  return;
}

/**
  This function enumerate the bridge on the device

  @param[in]      PegBus                  - Particular Bus number
  @param[in]      PegDev                  - Particular Device number
  @param[in]      PegFunc                 - Particular Func number
  @param[in, out] PegMmioLength           - PEG MMIO length
  @param[in]      OpRomScanTempMmioBar    - Temporary BAR to MMIO map OpROMs during VGA scanning
  @param[in]      OpRomScanTempMmioLimit  - Limit address for OpROM MMIO range
  @param[in]      ScanForLegacyOpRom      - TRUE to scan for legacy only VBIOS, FALSE otherwise
  @param[out]     FoundLegacyOpRom        - If legacy only VBIOS found, returns TRUE

  @retval CardDetect : TRUE if current bridge device has a Graphics card.
  @retval CardDetect : FALSE if current bridge device has no Graphics card.
**/
BOOLEAN
EnumerateBridgeDevice (
  IN      UINT8     PegBus,
  IN      UINT8     PegDev,
  IN      UINT8     PegFunc,
  IN OUT  UINT32    *PegMmioLength,
  IN      UINT32    OpRomScanTempMmioBar,
  IN      UINT32    OpRomScanTempMmioLimit,
  IN      BOOLEAN   ScanForLegacyOpRom,
  OUT     BOOLEAN   *FoundLegacyOpRom
  )
{
  UINT64  DeviceBaseAddress;
  UINT32  MmioLength;
  UINT8   Bus;
  UINT8   Dev;
  UINT8   SubBusNum;
  UINT16  Buffer16;
  BOOLEAN CardDetect;

  CardDetect = FALSE;

  ///
  /// Temporarily program the secondary and subordinate bus numbers
  /// of PEG bridge to (1, 0xFF) so that devices behind the bridge can be seen
  ///
  Bus = 1;
  DeviceBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, PegBus, PegDev, PegFunc, 0);
  PciSegmentWrite8 (DeviceBaseAddress + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET, Bus);
  PciSegmentWrite8 (DeviceBaseAddress + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET, 0xFF);

  ///
  /// A config write is required in order for the device to re-capture the Bus number,
  /// according to PCI Express Base Specification, 2.2.6.2
  /// Write to a read-only register VendorID to not cause any side effects.
  ///
  PciSegmentWrite16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, Bus, 0, 0, PCI_VENDOR_ID_OFFSET), 0);

  SubBusNum = EnumerateDownstream (Bus);

  for (Bus = 1; Bus <= SubBusNum; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      ///
      /// Read Vendor ID to check if device exists
      /// if no device exists, then check next device
      ///
      if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, Bus, Dev, 0, PCI_VENDOR_ID_OFFSET)) == 0xFFFF) {
        continue;
      }
      ///
      /// Add the MMIO address space requirement for this device to the total
      ///
      FindPciDeviceMmioLength (Bus, Dev, 0, &MmioLength);
      (*PegMmioLength) += MmioLength;
      Buffer16 = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, Bus, Dev, 0, R_PCI_SCC_OFFSET));
      ///
      /// Video cards can have Base Class 0 with Sub-class 1
      /// or Base Class 3 Sub-class 0
      ///
      if ((Buffer16 == 0x0001) || (Buffer16 == 0x0300)) {
        CardDetect = TRUE;
        if (ScanForLegacyOpRom) {
          (*FoundLegacyOpRom) = CheckForLegacyOnlyOpRom (OpRomScanTempMmioBar, OpRomScanTempMmioLimit, SA_SEG_NUM, Bus, Dev, 0);
        }
      }
    }
  }
  ///
  /// Clear bus number on all the bridges that we have opened so far.
  /// We have to do it in the reverse Bus number order.
  ///
  for (Bus = SubBusNum; Bus >= 1; Bus--) {
    for (Dev = 0; Dev < 32; Dev++) {
      ///
      /// Read Vendor ID to check if device exists
      /// if no device exists, then check next device
      ///
      DeviceBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, Bus, Dev, 0, 0);
      if (PciSegmentRead16 (DeviceBaseAddress + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
        continue;
      }

      Buffer16 = PciSegmentRead16 (DeviceBaseAddress + R_PCI_SCC_OFFSET);
      ///
      /// Clear Bus Number for PCI/PCI Bridge Device
      ///
      if (Buffer16 == 0x0604) {
        PciSegmentWrite32 (DeviceBaseAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0);
      }
    }
  }
  ///
  /// Clear the bus numbers on the PEG bridge
  ///
  PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, PegBus, PegDev, PegFunc, PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET), 0);

  return CardDetect;
}

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
  IN OUT   DISPLAY_DEVICE               *PrimaryDisplay,
  IN       UINT8                        PrimaryDisplaySelection,
  IN OUT   UINT32                       *PegMmioLength,
  IN       UINT32                       OpRomScanTempMmioBar,
  IN       UINT32                       OpRomScanTempMmioLimit,
  IN       BOOLEAN                      ScanForLegacyOpRom,
  OUT      BOOLEAN                      *FoundLegacyOpRom
  )
{
  UINT16    ClassCode;
  BOOLEAN   IgdPresent;
  UINT8     PegBus;
  UINT8     PegDev;
  UINT8     PegFunc;
  UINT64    PegBaseAddress;
  UINT64    B1BaseAddress;
  UINT16    PegDevenReg;
  UINT8     PegComplete;
  UINT16    PegEnable;
  BOOLEAN   CardDetect;
  UINT32    MmioLength;
  UINT8     Func;
  UINT8     MaxFunction;
  UINT8     HeaderType;
  UINT8     PegVgaFunc;

#ifdef CPU_CFL
  PEG_DEVEN PegDevenTable[3];
#else
  PEG_DEVEN PegDevenTable[4];
#endif
  MmioLength = 0;
  CardDetect = FALSE;
  PegVgaFunc = 0xFF;
  B1BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, 1, 0, 0, 0);

  ///
  ///  Bus, Device, Function, DevenMask
  ///
  PegDevenTable[0].Bus       = SA_PEG_BUS_NUM;
  PegDevenTable[0].Device    = SA_PEG0_DEV_NUM;
  PegDevenTable[0].Function  = SA_PEG0_FUN_NUM;
  PegDevenTable[0].DevenMask = BIT3;
  PegDevenTable[1].Bus       = SA_PEG_BUS_NUM;
  PegDevenTable[1].Device    = SA_PEG1_DEV_NUM;
  PegDevenTable[1].Function  = SA_PEG1_FUN_NUM;
  PegDevenTable[1].DevenMask = BIT2;
  PegDevenTable[2].Bus       = SA_PEG_BUS_NUM;
  PegDevenTable[2].Device    = SA_PEG2_DEV_NUM;
  PegDevenTable[2].Function  = SA_PEG2_FUN_NUM;
  PegDevenTable[2].DevenMask = BIT1;
#ifndef CPU_CFL
  PegDevenTable[3].Bus       = SA_PEG_BUS_NUM;
  PegDevenTable[3].Device    = SA_PEG3_DEV_NUM;
  PegDevenTable[3].Function  = SA_PEG3_FUN_NUM;
  PegDevenTable[3].DevenMask = BIT13;
#endif
  ///
  /// Read the DEVEN register for PEG 0/1/2/3 controllers configuration
  ///
  PegDevenReg = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, R_SA_DEVEN)) & (BIT13 + BIT3 + BIT2 + BIT1);

  ///
  /// If IGD is disabled
  /// or not present IgdPresent is set to false
  ///
  if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 2, 0, PCI_VENDOR_ID_OFFSET)) == 0xFFFF) {
    IgdPresent = FALSE;
  } else {
    IgdPresent = TRUE;
  }
  ///
  /// Scan PEG device vs DEVEN register for PEG controllers configuration
  ///
  for (PegComplete = 0; PegComplete < ((sizeof (PegDevenTable)) / (sizeof (PEG_DEVEN))); PegComplete++) {

    PegBus    = PegDevenTable[PegComplete].Bus;
    PegDev    = PegDevenTable[PegComplete].Device;
    PegFunc   = PegDevenTable[PegComplete].Function;
    PegEnable = PegDevenTable[PegComplete].DevenMask;
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, PegBus, PegDev, PegFunc, 0);

    if ((PegDevenReg & PegEnable) == 0) {
      continue;
    }
    ///
    /// Check for a card presence in the PEG slot.
    /// We don't know if it's a graphics card yet.
    ///
    if ((PciSegmentRead8 (PegBaseAddress + R_SA_PEG_SLOTSTS_OFFSET) & BIT6) == 0) {
      continue;
    }
    ///
    /// Set PEG PortBus = 1 to Read Endpoint.
    ///
    PciSegmentAndThenOr32 (PegBaseAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF0000FF, 0x00010100);

    ///
    /// A config write is required in order for the device to re-capture the Bus number,
    /// according to PCI Express Base Specification, 2.2.6.2
    /// Write to a read-only register VendorID to not cause any side effects.
    ///
    PciSegmentWrite16 (B1BaseAddress + PCI_VENDOR_ID_OFFSET, 0);

    ///
    /// Read Vendor ID to check if endpoint exists
    /// if no device exists, then check next device
    ///
    if (PciSegmentRead16 (B1BaseAddress + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check for a multifunction device
    ///
    HeaderType = PciSegmentRead8 (B1BaseAddress + PCI_HEADER_TYPE_OFFSET);
    if ((HeaderType & HEADER_TYPE_MULTI_FUNCTION) != 0) {
      MaxFunction = 7;
    } else {
      MaxFunction = 0;
    }
    ///
    /// Calculate total PEG MMIO length on all functions of the endpoint
    ///
    for (Func = 0; Func <= MaxFunction; Func++) {
      if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, 1, 0, Func, PCI_VENDOR_ID_OFFSET)) == 0xFFFF) {
        continue;
      }

      FindPciDeviceMmioLength (1, 0, Func, &MmioLength);
      *PegMmioLength += MmioLength;
    }
    ///
    /// Perform PEG Endpoint Class Code Check.  If the Endpoint Class Code is
    /// not GFX, then the Port is being used as a standard PCI Express Port.
    ///
    ClassCode = PciSegmentRead16 (B1BaseAddress + R_PCI_SCC_OFFSET);
    if ((ClassCode == 0x0001) || (ClassCode == 0x0300)) {
      ///
      /// Disable PEG if IGD or PCI VGA take precedence.
      ///
      ///
      /// If IGD is present and selected as primary, skip the PEG VGA enabling
      ///
      if (IgdPresent && (PrimaryDisplaySelection == IGD)) {
        PciSegmentAnd32 (PegBaseAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF0000FF);
        continue;
      }
      ///
      /// If PCI video card was detected, skip the PEG VGA enabling
      ///
      if (*PrimaryDisplay == PCI) {
        PciSegmentAnd32 (PegBaseAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF0000FF);
        continue;
      }
      ///
      /// Enable PEG video and Execute 16-bit address decodes on VGA I/O accesses
      ///
      /// Check if PEG VGA already detected
      ///
      if (*PrimaryDisplay != PEG) {
        PciSegmentOr16 (PegBaseAddress + R_SA_PEG_BCTRL_OFFSET, (BIT3 + BIT4));
        *PrimaryDisplay = PEG;
        PegVgaFunc = PegFunc;
        if (ScanForLegacyOpRom) {
          (*FoundLegacyOpRom) = CheckForLegacyOnlyOpRom (OpRomScanTempMmioBar, OpRomScanTempMmioLimit, SA_SEG_NUM, 1, 0, 0);
        }
        DEBUG ((DEBUG_INFO, "PCIe card on PEG%x%x (%x:%x:%x) enabled as VGA.\n", PegDev, PegFunc, PegBus, PegDev, PegFunc));
      }
    }

    if (ClassCode == 0x0604) {
      CardDetect =  EnumerateBridgeDevice (
                      PegBus,
                      PegDev,
                      PegFunc,
                      PegMmioLength,
                      OpRomScanTempMmioBar,
                      OpRomScanTempMmioLimit,
                      ScanForLegacyOpRom,
                      FoundLegacyOpRom
                      );
      if (CardDetect == TRUE) {
        ///
        /// Check if PEG VGA already detected
        ///
        if (*PrimaryDisplay != PEG) {
          PciSegmentOr16 (PegBaseAddress + R_SA_PEG_BCTRL_OFFSET, (BIT3 + BIT4));
          *PrimaryDisplay = PEG;
          PegVgaFunc = PegFunc;
          DEBUG ((DEBUG_INFO, "PCIe card on PEG%x%x (%x:%x:%x) enabled as VGA.\n", PegDev, PegFunc, PegBus, PegDev, PegFunc));
        }
      }
    }
    ///
    /// Restore bus numbers on the PEG bridge.
    ///
    PciSegmentAnd32 (PegBaseAddress + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF0000FF);
  } // End of the for Loop

  ///
  /// If a PEG device is used for primary graphics, set the ISAEN bit on all other PEG ports.
  ///
  if (PegVgaFunc != 0xFF) {
    for (PegComplete = 0; PegComplete < ((sizeof (PegDevenTable)) / (sizeof (PEG_DEVEN))); PegComplete++) {
      if (PegVgaFunc == PegComplete) {
        continue;
      }
      PegBus    = PegDevenTable[PegComplete].Bus;
      PegDev    = PegDevenTable[PegComplete].Device;
      PegFunc   = PegDevenTable[PegComplete].Function;
      PciSegmentOr16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, PegBus, PegDev, PegFunc, R_SA_PEG_BCTRL_OFFSET), BIT2);
      DEBUG ((DEBUG_INFO, "PEG%x%x (%x:%x:%x) ISAEN has been set.\n", PegDev, PegFunc, PegBus, PegDev, PegFunc));
    }
  }
}
