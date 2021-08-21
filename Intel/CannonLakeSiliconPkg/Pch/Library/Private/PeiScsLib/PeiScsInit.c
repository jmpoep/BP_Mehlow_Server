/** @file
  Helper functions for SCS controllers initialization.

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

#include "PeiScsInitInternal.h"

/**
  Programs passed MmioBase address into BAR register.

  @param[in] PciBaseAddress  Address of the PCI config space
  @param[in] MmioBase        Address to be used to access MMIO space
**/
VOID
ScsControllerEnableMmio (
  IN UINT64  PciBaseAddress,
  IN UINT32  MmioBase
  )
{
  //
  // Disable MSE
  //
  PciSegmentAnd16 (PciBaseAddress + PCI_COMMAND_OFFSET, (UINT16) ~EFI_PCI_COMMAND_MEMORY_SPACE);
  //
  // Write temp address to lower base address
  //
  PciSegmentWrite32 (PciBaseAddress + PCI_BASE_ADDRESSREG_OFFSET, MmioBase);
  //
  // Clear upper base address
  //
  PciSegmentWrite32 (PciBaseAddress + (PCI_BASE_ADDRESSREG_OFFSET + 4), 0);
  //
  // Enable MSE
  //
  PciSegmentOr16 (PciBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
}

/**
  This function clears BAR address.

  @param[in] PciBaseAddress  Address of PCI config space
**/
VOID
ScsControllerDisableMmio (
  IN UINT64  PciBaseAddress
  )
{
  //
  // Disable MSE
  //
  PciSegmentAnd16 (PciBaseAddress + PCI_COMMAND_OFFSET, (UINT16) ~EFI_PCI_COMMAND_MEMORY_SPACE);
  //
  // Clear lower base address
  //
  PciSegmentWrite32 (PciBaseAddress + PCI_BASE_ADDRESSREG_OFFSET, 0);
}

/**
  This function puts SCS controller into D3 state

  @param[in] PciBaseAddress  Address of the PCI config space
**/
VOID
ScsControllerPutToD3 (
  IN UINT64  PciBaseAddress
  )
{
  PciSegmentOr32 (PciBaseAddress + R_SCS_CFG_PCS, B_SCS_CFG_PCS_PS_D3HOT);

  PciSegmentRead32 (PciBaseAddress + R_SCS_CFG_PCS);
}

/**
  Configures Storage and Communication Subsystem (SCS)
  Controllers before PCI enumeration

  @param[in] SiPolicy                 The SI Policy PPI instance
**/
VOID
ScsInit (
  IN SI_POLICY_PPI  *SiPolicy
  )
{
  EFI_STATUS      Status;
  PCH_SCS_CONFIG  *ScsConfig;
  UINT8           UfsIndex;
  UINTN           TempMmioBase;

  DEBUG ((DEBUG_INFO, "ScsInit(): Start\n"));
  Iosf2OcpInitializeBridges ();

  Status = GetConfigBlock ((VOID*) SiPolicy, &gScsConfigGuid, (VOID*) &ScsConfig);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ScsInit(): Failed to locate SCS config block\n"));
    ASSERT (FALSE);
    return;
  }
  TempMmioBase = PcdGet32 (PcdSiliconInitTempMemBaseAddr);

  if (IsPchSdCardSupported ()) {
    ScsSdCardInit (SiPolicy, ScsConfig, TempMmioBase);
  }

  if (IsPchEmmcSupported ()) {
    ScsEmmcInit (SiPolicy, ScsConfig, TempMmioBase);
  }

  for (UfsIndex = 0; UfsIndex < PchGetMaxUfsNum (); UfsIndex++) {
    ScsUfsInit (SiPolicy, ScsConfig, UfsIndex, TempMmioBase);
  }

  DEBUG ((DEBUG_INFO, "ScsInit(): Finished\n"));
}

