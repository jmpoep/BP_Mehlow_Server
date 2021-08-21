/** @file
  PEI Function to initialize SA PciExpress.

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
**/
#include <Private/Library/SaPcieInitLib.h>
#include <Library/CpuPlatformLib.h>

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
  return;
}

/**
  Initialize the SA PciExpress in PEI

  @param[in] IN SA_MISC_PEI_PREMEM_CONFIG MiscPeiPreMemConfig to access Platform relevant information
  @param[in] IN PCIE_PEI_PREMEM_CONFIG    PciePeiPreMemConfig to access the PCIe Config related information
**/
VOID
PciExpressInit (
  IN       SA_MISC_PEI_PREMEM_CONFIG   *MiscPeiPreMemConfig,
  IN       PCIE_PEI_PREMEM_CONFIG      *PciePeiPreMemConfig
  )
{
  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  PcieAccessStructure;
  SA_PCIE_PRIVATE_FUNCTION_CALLS    PciePrivateDataStructure;
  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess;
  SA_PCIE_PRIVATE_FUNCTION_CALLS    *PciePrivate;
  PCIE_PORT_INFO                    PciePorts[SA_PEG_MAX_FUN];
  UINT8                             PciePortsLength;
  UINT8                             PegDisableMask;
  EFI_STATUS                        Status;
  UINT8                             PortIndex;

  FlowControlCreditProgrammingNoPegLib ();

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

  PcieAccess->GetPcieRootPorts (PcieAccess, &(PciePorts[0]), &PciePortsLength);

  if (!PcieAccess->PcieExists (PcieAccess)) {
    ///
    /// Disable all PEG Controllers.
    ///
    PegDisableMask = V_PEG_DISABLE_MASK;
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      PciePrivate->DisableUnusedPcieControllers (PcieAccess, PegDisableMask, &PciePorts[PortIndex]);
    }
  }
  return;
}

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
  )
{
  return EFI_SUCCESS;
}
