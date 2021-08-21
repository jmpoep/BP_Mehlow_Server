/** @file
  CpuPowerOnConfig PEI Disable Library

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

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Private/Library/CpuPowerOnConfigLib.h>
#include <Private/Library/PchRcLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Ppi/MasterBootMode.h>
#include <CpuAccess.h>
#include <SaAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Library/PciSegmentLib.h>
#include <Library/SaPlatformLib.h>
#include <Library/PostCodeLib.h>
/**
  Set Early Power On Configuration setting based on default value for SKU.

  @param[in] SiPreMemPolicyPpi - Address of the SI_POLICY_PPI.

  @retval EFI_SUCCESS       - Function completed successfully.
**/
EFI_STATUS
SetCpuStrapAndEarlyPowerOnConfig (
  IN SI_PREMEM_POLICY_PPI        *SiPreMemPolicyPpi
  )
{
  EFI_STATUS                  Status;
  EFI_BOOT_MODE               BootMode;
  CPU_GENERATION              CpuGeneration;
  UINT64                      MchBar;
  CPU_RESET_TYPE              ResetType;
  CPU_EPOC_SET                CpuEpocSet;
  UINT32                      CpuEpocData;
  CPU_FAMILY                  CpuFamily;
  CPU_SKU                     CpuSku;
  BOOLEAN                     PegDisabled;


  DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Start \n"));
  PostCode (0xC0A);

  Status = PeiServicesGetBootMode (&BootMode);
  DEBUG ((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig: BootMode = %X\n", BootMode));
  if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
    //
    // Lock CPU Straps
    //
    DEBUG ((DEBUG_INFO, "EarlyPowerOnConfig doesn't execute during S3 resume path, only locking straps\n"));
    Status = PchCpuStrapSet (LockCpuStrapSetData, NULL);
    return EFI_SUCCESS;
  }

  CpuGeneration = GetCpuGeneration();
  if(CpuGeneration == EnumCflCpu){
    CpuEpocSet.Uint32 = 0;
    CpuEpocData       = 0;
    ResetType         = NO_RESET;

    //
    // Early Power On Configuration
    //
    DEBUG ((DEBUG_INFO, "Set Early Power On Configuration\n"));
    CpuFamily = GetCpuFamily ();
    CpuSku    = GetCpuSku ();

  //
  // Set based on default EPOC value.
  // - 0: 800 MHz (ULT/ULX).
  // - 1: 1 GHz (DT/Halo).
  //
  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;
  if (IsPchLinkDmi (CpuFamily) && (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG_DEV_NUM, SA_PEG0_FUN_NUM, PCI_VENDOR_ID_OFFSET)) != 0xFFFF)) {
    PegDisabled = MmioRead32 ((UINTN) MchBar + R_SA_MCHBAR_BIOS_RESET_CPL_OFFSET) & BIT3;
  } else {
    PegDisabled = 1;
  }

    ///
    /// Halo SKU and PEG Disabled or ULX/ULT
    ///
    if (((CpuSku == EnumCpuHalo) && (PegDisabled)) || (CpuSku != EnumCpuTrad)) {
        CpuEpocSet.Bits.Fclk = 0;
    } else {
        CpuEpocSet.Bits.Fclk = 1;
    }

    Status = PchCpuEpocSet (GetCpuEpocData, &CpuEpocData);

    if ((Status == EFI_SUCCESS) && (CpuEpocData != CpuEpocSet.Uint32)) {
      DEBUG ((DEBUG_INFO, "Current EPOC Data = 0x%04X\n", CpuEpocData));
      CpuEpocData = CpuEpocSet.Uint32;
      DEBUG ((DEBUG_INFO, "Setting EPOC Data = 0x%04X\n", CpuEpocData));
      Status = PchCpuEpocSet (SetCpuEpocData, &CpuEpocData);
      if (Status == EFI_SUCCESS) {
        ResetType = COLD_RESET;
      }
    } else {
      DEBUG ((DEBUG_INFO, "Skipping EPOC Configuration.\n"));
    }

    if (ResetType != NO_RESET) {
      ///
      /// Perform Cold Reset
      ///
      DEBUG ((DEBUG_INFO, "Reset Required. Performing Cold Reset to read the new EPOC values\n"));
      DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Reset \n"));
      PostCode (0xC13);
      PerformWarmOrColdReset (ResetType);
    }
  }

  //
  // Lock CPU Straps
  //
  Status = PchCpuStrapSet (LockCpuStrapSetData, NULL);

  DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Done \n"));
  PostCode (0xC14);

  return EFI_SUCCESS;
}
