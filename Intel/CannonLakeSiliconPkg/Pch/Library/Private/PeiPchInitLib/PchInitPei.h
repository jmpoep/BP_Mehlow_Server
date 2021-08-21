/** @file
  Header file for the PCH Init PEIM

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2018 Intel Corporation.

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
#ifndef _PCH_INIT_PEI_H_
#define _PCH_INIT_PEI_H_

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/TimerLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PchEspiLib.h>
#include <Library/PchPcieRpLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchSbiAccessLib.h>
#include <Library/PchInfoLib.h>
#include <Library/SataLib.h>
#include <Private/Library/PchTraceHubLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchPolicyLib.h>
#include <Private/Library/PchFiaLib.h>
#include <Private/Library/PchPciExpressHelpersLib.h>
#include <Private/Library/PchInitCommonLib.h>
#include <Ppi/SiPolicy.h>
#include <Ppi/PchReset.h>
#include <Library/PchResetLib.h>
#include <Library/GpioLib.h>
#include <Library/GpioNativeLib.h>
#include <Library/PmcLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <IndustryStandard/Pci30.h>
#include <MeChipset.h>
#include "PchUsb.h"
#include <Private/Library/DciPrivateLib.h>
#include <Private/Library/PchPsfPrivateLib.h>
#include <PchResetPlatformSpecific.h>
#include <PchInfoHob.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Private/Library/PeiPmcPrivateLib.h>
#include <Private/PchConfigHob.h>
#include <PchLimits.h>
#include <Private/Library/PeiThermalLib.h>
#include <Private/Library/PeiPchPcieClocksLib.h>
#include <Private/Library/PeiCnviPrivateLib.h>
#include <Private/Library/PeiP2sbPrivateLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/PeiPchDmiLib.h>
#include <Private/Library/PeiItssLib.h>
#include <Private/Library/PeiPchTraceHubLib.h>
#include <Private/Library/SiScheduleResetLib.h>
#include <Private/Library/PeiScsLib.h>
#include <Private/Library/PeiLpcLib.h>

extern EFI_GUID gPchDeviceTableHobGuid;



/**
  Hide rootports disabled by policy. This needs to be done in premem,
  because graphics init from SystemAgent code depends on those ports
  being already hidden

  @param[in] PcieRpPreMemConfig   Platform policy
**/
VOID
EarlyPcieRpDisabling (
  IN PCH_PCIE_RP_PREMEM_CONFIG *PcieRpPreMemConfig
  );

/**
  Initializes ports with NonCommonClock configuration. Such ports can't detect endpoints
  before NCC init ends. To prevent boot delays, it happens in pre-mem
**/
VOID
EarlyPcieNccHandling (
  VOID
  );

/**
  Configure root port function number mapping

**/
VOID
PchConfigureRpfnMapping (
  VOID
  );

/**
  Perform Root Port Initialization.

  @param[in]  PortIndex               The root port to be initialized (zero based)
  @param[in]  SiPolicy                The SI Policy PPI
  @param[in] TempPciBus               The temporary Bus number for root port initialization

**/
VOID
PchInitSingleRootPort (
  IN  UINT8                                     PortIndex,
  IN  CONST SI_POLICY_PPI                       *SiPolicy,
  IN  SI_PREMEM_POLICY_PPI                      *SiPreMemPolicyPpi,
  IN  UINT8                                     TempPciBus,
  OUT BOOLEAN                                   *Gen3DeviceFound
  );

/**
  Perform Initialization of the Downstream Root Ports.

  @param[in] SiPolicy             The SI Policy PPI
  @param[in] TempPciBusMin        The temporary minimum Bus number for root port initialization
  @param[in] TempPciBusMax        The temporary maximum Bus number for root port initialization

  @retval EFI_SUCCESS             The function completed successfully
**/
VOID
PchInitRootPorts (
  IN CONST SI_POLICY_PPI     *SiPolicy
  );


/**
  The function is used to detemine if a ChipsetInitSync with CSME is required and syncs with CSME if required.

  @retval EFI_SUCCESS             BIOS and CSME ChipsetInit settings are in sync
  @retval EFI_UNSUPPORTED         BIOS and CSME ChipsetInit settings are not in sync
**/
EFI_STATUS
ChipsetInitSync (
  VOID
  );

/**
  The function program HSIO registers.

  @param[in] SiPreMemPolicyPpi          The SI PREMEM Policy PPI instance
**/
VOID
PchHsioBiosProgPreMem (
  IN  SI_PREMEM_POLICY_PPI             *SiPreMemPolicyPpi
  );

/**
  The function program HSIO registers.

  @param[in] PchPolicyPpi               The PCH Policy PPI instance

**/
VOID
PchHsioMiscBiosProg (
  IN  SI_POLICY_PPI    *SiPolicyPpi
  );

/**
  Configure PCIe Grant Counts
**/
VOID
PchConfigurePcieGrantCounts (
  VOID
  );

/**
  Build PchDeviceTableHob

  @param[in] SiPolicy     The Silicon Policy PPI instance

  @retval None
**/
VOID
BuildPcieDeviceTableHob (
  IN  SI_POLICY_PPI  *SiPolicy
  );

/**
  Clear EISS (Enable InSMM.STS).
  The usage to clear EISS is to support set variable in PEI phase.

  NOTE: This does not work if LE bit is already set.
**/
VOID
LpcSpiClearEiss (
  VOID
  );

/**
  The function performs PSTH specific programming.

  @param[in] SiPolicy          The SI Policy instance
**/
VOID
PchPsthConfigure (
  IN  SI_POLICY_PPI           *SiPolicy
  );

/**
  The function performs iClk specific programming.
**/
VOID
PchIClkConfigure (
  VOID
  );

/**
  The function update pch config hob in the end of PchInit.

  @param[in]      SiPolicy               The SI Policy PPI instance
**/
VOID
BuildPchConfigHob (
  IN     SI_POLICY_PPI          *SiPolicy
  );

/**
  The function update pch info hob in the end of PchInit.
**/
VOID
BuildPchInfoHob (
  VOID
  );
#endif
