/** @file
  PEIM to initialize IPU device

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2017 Intel Corporation.

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
#include <Ppi/SiPolicy.h>
#include <Library/PeiServicesLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <SaAccess.h>
#include <PchAccess.h>
#include <Library/PciSegmentLib.h>
#include <Private/Library/PchRcLib.h>
#include <Private/Library/IpuInitLib.h>
#include <CpuRegs.h>
#include <Library/CpuPlatformLib.h>
#include <Private/SaConfigHob.h>
#include <Private/Library/GpioPrivateLib.h>
#include <IndustryStandard/Pci30.h>

/**
  IpuInit: Initialize the IPU device

  @param[in] IPU_PREMEM_CONFIG   IpuPreMemPolicy

**/
VOID
IpuInit (
  IN       IPU_PREMEM_CONFIG         *IpuPreMemPolicy
  )
{
  UINT32              Reg32;
  BOOLEAN             IpuDisabled;
  CPU_STEPPING        CpuSteppingId;
  CPU_FAMILY          CpuFamilyId;
  CPU_GENERATION      CpuGeneration;
  UINT64              IpuBaseAddress;
  SA_CONFIG_HOB       *SaConfigHob;

  CpuSteppingId = GetCpuStepping ();
  CpuFamilyId   = GetCpuFamily ();
  IpuBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, 0);

  Reg32 = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_CAPID0_B));
  IpuDisabled = FALSE;
  CpuGeneration = GetCpuGeneration ();

  if (CpuGeneration == EnumCflCpu) {
    DEBUG ((DEBUG_INFO, "CFL doesn't support IPU\n"));
    IpuDisabled = TRUE;
  }

  if (Reg32 & BIT31) {
    DEBUG ((DEBUG_INFO, "IPU Fused off\n"));
    IpuDisabled = TRUE;
  } else {
    //
    // IPU is not fused off
    //
    if ((PciSegmentRead16 (IpuBaseAddress + PCI_VENDOR_ID_OFFSET)) == 0xFFFF) {
      DEBUG ((DEBUG_WARN, "IPU config space not accessible!\n"));
      IpuDisabled = TRUE;
    }
    if (IpuPreMemPolicy->SaIpuEnable == 0) {
      DEBUG ((DEBUG_INFO, "Policy decides to disable IPU\n"));
      IpuDisabled = TRUE;
    }

    if (IpuDisabled) {
      //
      // If IPU is not fused off it is enabled anyway so here BIOS has to disable it if required
      //
      DEBUG ((DEBUG_INFO, "Disable IPU\n"));
      PciSegmentAnd32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_DEVEN), (UINT32) ~B_SA_DEVEN_D5EN_MASK);
    }
  }

  if ((CpuFamilyId == EnumCpuCnlUltUlx) && (CpuSteppingId >= EnumCnlB0)) {
    if (!IpuDisabled) {
      //
      // Configure IMGUCLK
      //
      GpioEnableImguClkOut ();
    }
  }

  //
  // Initialize specific policy into Hob for DXE phase use.
  //
  SaConfigHob = NULL;
  SaConfigHob = (SA_CONFIG_HOB *) GetFirstGuidHob (&gSaConfigHobGuid);
  if (SaConfigHob != NULL) {
    //
    // Update IPU ACPI mode depending on IGFX present or not
    //
    if ((!IpuDisabled) && (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0)) != 0xFFFF)) {
      //
      // Set IPU ACPI mode as IGFX Child device
      //
      SaConfigHob->IpuAcpiMode = 1;
    } else {
      //
      // Set IPU ACPI mode as Disabled
      //
      SaConfigHob->IpuAcpiMode = 0;
    }
  }
}

/**
  IsIpuEnabled: Check the IPU is enabled or not

  @retval FALSE = IPU Disabled by policy or fuse, TRUE = IPU Enabled.
**/
BOOLEAN
IsIpuEnabled (
  VOID
  )
{
  SI_PREMEM_POLICY_PPI *SiPreMemPolicyPpi;
  IPU_PREMEM_CONFIG    *IpuPreMemPolicy;
  EFI_STATUS           Status;
  UINT32               Data32;

  SiPreMemPolicyPpi  = NULL;
  IpuPreMemPolicy = NULL;

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

  Status = GetConfigBlock((VOID *) SiPreMemPolicyPpi, &gIpuPreMemConfigGuid, (VOID *) &IpuPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  Data32 = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_CAPID0_B));
  if (Data32 & BIT31) {
    DEBUG ((DEBUG_INFO, "IPU Fused off\n"));
    return FALSE;
  }

  if (IpuPreMemPolicy->SaIpuEnable) {
    return TRUE;
  } else {
    return FALSE;
  }
}

/**
  GetIpuImrConfiguration: Get the IPU IMR Configuration

  @retval IPU IMR Configuration, 0 = IPU Camera, 1 = IPU Gen
**/
UINT8
GetIpuImrConfiguration (
  VOID
  )
{
  SI_PREMEM_POLICY_PPI *SiPreMemPolicyPpi;
  IPU_PREMEM_CONFIG    *IpuPreMemPolicy;
  EFI_STATUS           Status;

  SiPreMemPolicyPpi  = NULL;
  IpuPreMemPolicy = NULL;

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

  Status = GetConfigBlock((VOID *) SiPreMemPolicyPpi, &gIpuPreMemConfigGuid, (VOID *) &IpuPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  return (UINT8) IpuPreMemPolicy->SaIpuImrConfiguration;
}
