/** @file
  Implementation file for PCH PCIe Policy functionality for PEIM

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 Intel Corporation.

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

#include <PiPei.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/PostCodeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Ppi/SiPolicy.h>
#include <Library/ConfigBlockLib.h>
#include <ConfigBlock.h>
#include <ConfigBlock/MePeiConfig.h>
#include <Library/PeiPchPcieLib.h>

/**
  Initialize PCH PCIe PEI PreMem Config Block.

  @retval EFI_SUCCESS             PCH PCIe PEI PreMem Config Block is initialized.
  @retval All other error conditions encountered when PCH PCIe PEI PreMem Config Block initialized fail.
**/
STATIC
EFI_STATUS
PchPciePeiPreMemConfigInit (
  IN OUT PCH_PCIE_RP_PREMEM_CONFIG **PchPciePeiPreMemConfig
  )
{
  EFI_STATUS                      Status;
  SI_PREMEM_POLICY_PPI            *SiPreMemPolicyPpi;

  ///
  /// Get Policy settings through the SiPreMemPolicy PPI
  ///
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  if (!EFI_ERROR (Status)) {
    Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPcieRpPreMemConfigGuid, (VOID *) PchPciePeiPreMemConfig);
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

/**
  Read the PCH PCIE policy to check port number for IMR

  @retval   port number
**/
BOOLEAN
PchGetPcieImrPortNumber (
  VOID
  )
{
  EFI_STATUS                      Status;
  PCH_PCIE_RP_PREMEM_CONFIG       *PchPciePeiPreMemConfig;

  Status = PchPciePeiPreMemConfigInit (&PchPciePeiPreMemConfig);
  if (!EFI_ERROR (Status)) {
    return PchPciePeiPreMemConfig->ImrRpSelection;
  }
  return 0;
}

/**
  Read the PCH PCIE policy to see if PCIe IMR is enabled.

  @retval TRUE             PCIe IMR is enabled
  @retval FALSE            PCIe IMR is disabled
**/
BOOLEAN
PchIsPcieImrEnabled (
  VOID
  )
{
  EFI_STATUS                      Status;
  PCH_PCIE_RP_PREMEM_CONFIG       *PchPciePeiPreMemConfig;

  Status = PchPciePeiPreMemConfigInit (&PchPciePeiPreMemConfig);
  if (!EFI_ERROR (Status)) {
    if (PchPciePeiPreMemConfig->PcieImrEnabled != 0) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Read the PCH PCIE policy to get PCIe IMR Size.

  @retval PcieImrSize      Size of PCIe Imr in MB
**/
UINT32
PchGetPcieImrSize (
  VOID
  )
{
  EFI_STATUS                      Status;
  PCH_PCIE_RP_PREMEM_CONFIG       *PchPciePeiPreMemConfig;

  Status = PchPciePeiPreMemConfigInit (&PchPciePeiPreMemConfig);
  if (!EFI_ERROR (Status)) {
    return (UINT32) PchPciePeiPreMemConfig->PcieImrSize;
  }

  return 0;
}

/**
  This function puts PCIe IMR related information into HOB

  @param[in] BaseLow    lower 32 bits of PCIe IMR address
  @param[in] BaseHigh   higher 32 bits of PCIe IMR address
**/
VOID
BuildPciImrHob (
  UINT32 BaseLow,
  UINT32 BaseHigh
  )
{
  PCI_IMR_HOB *PciImrHobPtr;
  EFI_STATUS  Status;

  Status = PeiServicesCreateHob (EFI_HOB_TYPE_GUID_EXTENSION, sizeof (PCI_IMR_HOB), (VOID **) &PciImrHobPtr);
  if (EFI_ERROR (Status)) {
    return;
  }
  PciImrHobPtr->Header.Name  = gPciImrHobGuid;
  PciImrHobPtr->PciImrBase   = BaseLow + LShiftU64 (BaseHigh, 32);
}

