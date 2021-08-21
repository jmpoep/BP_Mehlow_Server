/** @file
  This code provides an instance of CPU TraceHub Lib.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

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
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Private/Library/PeiCpuTraceHubLib.h>
#include <Library/CpuPlatformLib.h>
#include <Ppi/MpServices.h>
#include <Library/PeiServicesTablePointerLib.h>

#ifndef CPU_CFL
typedef struct _MTRR_SET_MEMORY_ATTRIBUTE_CPU {
  PHYSICAL_ADDRESS        BaseAddress;
  UINT64                  Length;
  MTRR_MEMORY_CACHE_TYPE  Attribute;
} MTRR_SET_MEMORY_ATTRIBUTE_CPU;

VOID
MtrrSetMemoryAttributeCpuWrapper (
 IN OUT VOID * Buffer
)
{
 MTRR_SET_MEMORY_ATTRIBUTE_CPU *MtrrSetMemoryAttributeContent;

 MtrrSetMemoryAttributeContent = (MTRR_SET_MEMORY_ATTRIBUTE_CPU *) Buffer;
 MtrrSetMemoryAttribute (MtrrSetMemoryAttributeContent->BaseAddress, MtrrSetMemoryAttributeContent->Length, MtrrSetMemoryAttributeContent->Attribute);
}

/**
  Configure CPU Trace Hub

  @param[in] CpuTraceHubConfig - Instance of CPU_TRACE_HUB_CONFIG
  @retval EFI_SUCCESS.
  @retval EFI_OUT_OF_RESOURCES.
**/
EFI_STATUS
ConfigureCpuTraceHub (
  IN CPU_TRACE_HUB_CONFIG *CpuTraceHubConfig
  )
{
  EFI_STATUS                        Status;
  UINT16                            DeviceId;
  UINT32                            Data32;
  UINT64                            CpuTraceHubBaseAddress;
  UINT32                            ScratchpadReg;
  UINT32                            LowPowerModeEnable;
  UINT32                            SaMchBar;
  UINT32                            SaRegBar;
  UINT32                            D9F0Psf1BaseAddress;
  UINT32                            D9F1Psf1BaseAddress;
  CPU_FAMILY                        CpuFamilyId;
  CPU_STEPPING                      CpuStepping;
  UINT32                            TempIstot;
  MTRR_SET_MEMORY_ATTRIBUTE_CPU     MtrrSetMemoryAttributeContent;
  EFI_PEI_MP_SERVICES_PPI           *CpuMpPpi;

  //
  // Get MP Services Protocol
  //
  Status = PeiServicesLocatePpi (
             &gEfiPeiMpServicesPpiGuid,
             0,
             NULL,
             (VOID **)&CpuMpPpi
             );
  ASSERT_EFI_ERROR (Status);

  CpuStepping = GetCpuStepping ();
  CpuFamilyId = GetCpuFamily ();
  DEBUG ((DEBUG_INFO, "ConfigureCpuTraceHub () - Start\n"));

  CpuTraceHubBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_TH_BUS, SA_TH_DEV, SA_TH_FUN, 0);
  SaMchBar                 = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;
  SaRegBar                 = MmioRead32 (SaMchBar + R_SA_MCHBAR_REGBAR_OFFSET) & ~BIT0;
  D9F0Psf1BaseAddress      = SaRegBar + V_SA_PSF1_OFFSET + R_SA_PSF1_T0_SHDW_TRACE_HUB_D9F0_ACPI_REG_BASE;
  D9F1Psf1BaseAddress      = SaRegBar + V_SA_PSF1_OFFSET + R_SA_PSF1_T0_SHDW_TRACE_HUB_D9F1_ACPI_REG_BASE;

  //
  // If debugger mode is disabled, write CPU Trace Hub DEVEN to 0.
  //
  if (IsCpuDebugDisabled ()) {
    DEBUG ((DEBUG_INFO, "DCD bit is set, disable CPU Trace Hub\n"));
    PciSegmentAnd32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_DEVEN), (UINT32) ~(B_SA_DEVEN_ITH_MASK));
    PciSegmentWrite8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET, 0);
    return EFI_SUCCESS;
  }
  //
  // Step 1 : Read Device ID from (0, 9, 0) to make sure CPU Intel Trace Hub exists.
  //
  DeviceId = PciSegmentRead16 (CpuTraceHubBaseAddress + PCI_DEVICE_ID_OFFSET);
  if (DeviceId == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "CPU Intel Trace Hub does not exist in the system\n."));
    return EFI_SUCCESS;
  }

  //
  // Step 2 : Enable the device in DEVEN[12] to keep CPU Intel Trace Hub enabled.
  //
  PciSegmentOr32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_DEVEN), (UINT32)(B_SA_DEVEN_ITH_MASK));

  //
  // Program the MTB Base Address Register fixed BAR and enable MSE
  //
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_MTB_LBAR, (UINT32) PcdGet32 (PcdCpuTraceHubMtbBarBase));
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_MTB_UBAR, 0);
  PciSegmentOr8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);

  //
  // Clear LPP_CTL.LPMEN bit before initializing Trace Hub
  //
  DEBUG ((DEBUG_INFO, "Clearing LPP_CTL_LPME\n"));
  MmioAnd32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_TRACE_HUB_LPP_CTL, (UINT32) ~(B_SA_TRACE_HUB_LPP_CTL_LPMEN));

  //
  // Step 3 : Check if LPMEN bit in LPP_CTL is set, or SCRPD0[24] is 0 and BIOS Setup disable the CPU Trace Hub, BIOS should stop.
  //
  LowPowerModeEnable = MmioRead32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_TRACE_HUB_LPP_CTL);
  ScratchpadReg = MmioRead32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_SCRPD0);
  DEBUG((DEBUG_INFO, "LPP_CTL = %x, SCRPD0 = %x EnableMode %x \n", LowPowerModeEnable, ScratchpadReg, CpuTraceHubConfig->EnableMode ));
  if (((LowPowerModeEnable & B_SA_TRACE_HUB_LPP_CTL_LPMEN) == B_SA_TRACE_HUB_LPP_CTL_LPMEN)
       || (((ScratchpadReg & B_SA_DEBUGGER_IN_USE_MASK) == 0) && (CpuTraceHubConfig->EnableMode == TraceHubModeDisabled)))
  {
    //
    // Clear MSE
    //
    PciSegmentWrite8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET, 0);
    //
    // Clear MTB_BAR
    //
    PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_MTB_LBAR, 0);

    DEBUG ((DEBUG_INFO, "CPU Intel Trace Hub is in Low Power Mode or not connected and is disabled by SETUP menu . Stop the initialization.\n"));
    PciSegmentAnd32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_DEVEN), (UINT32) ~(B_SA_DEVEN_ITH_MASK));
    return EFI_SUCCESS;
  }

  //
  // Step 4. Clear MSE
  //
  PciSegmentAnd8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET,(UINT8) ~(EFI_PCI_COMMAND_MEMORY_SPACE));

  //
  // Step 5. Program the SW Base Address Register with fixed BAR
  //
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_SW_LBAR, (UINT32) PcdGet32 (PcdCpuTraceHubSwBarBase));
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_SW_UBAR, 0);
  Status = MtrrSetMemoryAttribute ((EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubSwBarBase) ,  PcdGet32 (PcdCpuTraceHubSwBarSize) , CacheUncacheable);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Setting SW_BAR UNCACHEABLE failed!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  MtrrSetMemoryAttributeContent.BaseAddress = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubSwBarBase);
  MtrrSetMemoryAttributeContent.Length = PcdGet32 (PcdCpuTraceHubSwBarSize);
  MtrrSetMemoryAttributeContent.Attribute = CacheUncacheable;

  Status = CpuMpPpi->StartupAllAPs (
                       GetPeiServicesTablePointer (),
                       CpuMpPpi,
                       (EFI_AP_PROCEDURE) MtrrSetMemoryAttributeCpuWrapper,
                       FALSE,
                       0,
                       &MtrrSetMemoryAttributeContent
                       );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Setting SW_BAR UNCACHEABLE for All APs failed!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  //
  // Step 6. Program the FW BAR and Shadow PCI Device
  // (PCI offset 0x70+0x74)
  //
  // At this point, a shadow PCI device (0/9/1) within the backbone PSF needs to be programmed
  // with the value of FW BAR, have its memory space enabled, and then hide the shadow device
  //
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_FW_LBAR, (UINT32) PcdGet32 (PcdCpuTraceHubFwBarBase));
  PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_FW_UBAR, 0);
  Status = MtrrSetMemoryAttribute ((EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubFwBarBase) , PcdGet32 (PcdCpuTraceHubFwBarSize) , CacheUncacheable);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Setting FW_BAR UNCACHEABLE failed!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }
  MtrrSetMemoryAttributeContent.BaseAddress = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubFwBarBase);
  MtrrSetMemoryAttributeContent.Length = PcdGet32 (PcdCpuTraceHubFwBarSize);
  MtrrSetMemoryAttributeContent.Attribute = CacheUncacheable;

  Status = CpuMpPpi->StartupAllAPs (
                       GetPeiServicesTablePointer (),
                       CpuMpPpi,
                       (EFI_AP_PROCEDURE) MtrrSetMemoryAttributeCpuWrapper,
                       FALSE,
                       0,
                       &MtrrSetMemoryAttributeContent
                       );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "Setting FW_BAR UNCACHEABLE for All APs failed!\n"));
    ASSERT_EFI_ERROR (Status);
    return Status;
  }

  MmioWrite32 (D9F1Psf1BaseAddress + R_SA_PSF1_T0_SHDW_BAR0, PcdGet32 (PcdCpuTraceHubFwBarBase));
  MmioWrite32 (D9F1Psf1BaseAddress + R_SA_PSF1_T0_SHDW_BAR1, 0);
  MmioOr32 (D9F1Psf1BaseAddress + R_SA_PSF1_T0_SHDW_PCIEN, B_SA_PSF1_T0_SHDW_PCIEN_MEMEN);
  MmioOr32 (D9F1Psf1BaseAddress + R_SA_PSF1_T0_SHDW_CFG_DIS, B_SA_PSF1_T0_SHDW_CFG_DIS_CFGDIS);

  CpuWriteTraceHubAcpiBase (PcdGet32 (PcdCpuTraceHubFwBarBase));

  //
  // Enable MSE
  //
  DEBUG ((DEBUG_INFO, "Enabling MSE\n"));
  PciSegmentOr8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);

  Data32 = MmioRead32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_STHCAP1);
  if (Data32 & B_SA_STHCAP1_RTITCNT_MASK) {
    //
    // Program RTIT BAR when RTITCNT is not 0.
    //
    PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_RTIT_LBAR, (UINT32) PcdGet32 (PcdCpuTraceHubRtitBarBase));
    PciSegmentWrite32 (CpuTraceHubBaseAddress + R_SA_RTIT_UBAR, 0);
    Status = MtrrSetMemoryAttribute ((EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubRtitBarBase) , PcdGet32 (PcdCpuTraceHubRtitBarSize) , CacheWriteCombining);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Setting RTIT_BAR USWC failed!\n"));
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
    MtrrSetMemoryAttributeContent.BaseAddress = (EFI_PHYSICAL_ADDRESS) PcdGet32 (PcdCpuTraceHubRtitBarBase);
    MtrrSetMemoryAttributeContent.Length = PcdGet32 (PcdCpuTraceHubRtitBarSize);
    MtrrSetMemoryAttributeContent.Attribute = CacheWriteCombining;

    Status = CpuMpPpi->StartupAllAPs (
                         GetPeiServicesTablePointer (),
                         CpuMpPpi,
                         (EFI_AP_PROCEDURE) MtrrSetMemoryAttributeCpuWrapper,
                         FALSE,
                         0,
                         &MtrrSetMemoryAttributeContent
                         );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "Setting RTIT_BAR USWC for All APs failed!\n"));
      ASSERT_EFI_ERROR (Status);
      return Status;
    }
  }
  //
  // Disable RTIT BAR via PSF1 register for all CNL SKU
  //
  MmioOr32 (D9F0Psf1BaseAddress + R_SA_PSF1_T0_SHDW_PCIEN, B_SA_PSF1_T0_SHDW_PCIEN_BAR4DIS);

  DEBUG ((DEBUG_INFO, "Assigned BARs:\n"));
  DEBUG ((DEBUG_INFO, "FW_LBAR   = 0x%08x\n", PciSegmentRead32 (CpuTraceHubBaseAddress + R_SA_FW_LBAR)));
  DEBUG ((DEBUG_INFO, "MTB_LBAR  = 0x%08x\n", PciSegmentRead32 (CpuTraceHubBaseAddress + R_SA_MTB_LBAR)));
  DEBUG ((DEBUG_INFO, "RTIT_LBAR = 0x%08x\n", PciSegmentRead32 (CpuTraceHubBaseAddress + R_SA_RTIT_LBAR)));
  DEBUG ((DEBUG_INFO, "SW_LBAR   = 0x%08x\n", PciSegmentRead32 (CpuTraceHubBaseAddress + R_SA_SW_LBAR)));

  //
  // Enable BME
  //
  DEBUG ((DEBUG_INFO, "Enabling BME\n"));
  PciSegmentOr8 (CpuTraceHubBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_BUS_MASTER);

  //
  // BIOS is required to set SDC.CE (CHAP Enable) bit for Punit to perform save/restore without checking the bit.
  //
  MmioOr32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_SDC, B_SA_SDC_CE_MASK);

  if (((CpuFamilyId == EnumCpuCnlUltUlx) && (CpuStepping >= EnumCnlB0)) || (CpuFamilyId == EnumCpuCnlDtHalo)) {
    TempIstot = PciSegmentRead32(CpuTraceHubBaseAddress + R_SA_TRACE_HUB_ISTOT);
    PciSegmentWrite32(CpuTraceHubBaseAddress + R_SA_TRACE_HUB_ISTOT, 0);

    //
    // BIOS is required to set PMCTS.BLKDRNEN (Block and Drain Enable) bit to enable Automatic Block and Drain/Unblock feature for CNL B0 CPU onward.
    //
    MmioWrite32 (PcdGet32 (PcdCpuTraceHubMtbBarBase) + R_SA_MTB_PMTCS, 0x40 | B_SA_MTB_PMTCS_BLKDRNEN);

    PciSegmentWrite32(CpuTraceHubBaseAddress + R_SA_TRACE_HUB_ISTOT, TempIstot);
  }

  DEBUG ((DEBUG_INFO, "ConfigureCpuTraceHub () - End\n"));

  return EFI_SUCCESS;
}
#endif
