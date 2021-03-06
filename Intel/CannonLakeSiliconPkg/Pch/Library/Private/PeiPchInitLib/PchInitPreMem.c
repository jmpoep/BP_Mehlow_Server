/** @file
  The PCH Init PEIM implements the PCH PEI Init PPI.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2018 Intel Corporation.

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
#include "PchInitPei.h"
#include "PchUsb.h"
#include <MeChipset.h>
#include <Private/PchHsio.h>
#include <Library/PostCodeLib.h>
#include <Library/OcWdtLib.h>
#include <Library/PchResetLib.h>
#include <Library/SpiLib.h>
#include <Private/Library/PchDmiLib.h>
#include <Private/Library/PeiP2sbPrivateLib.h>
#include <Private/Library/PeiRtcLib.h>
#include <Private/Library/PeiWdtLib.h>
#include <Private/Library/PeiSmbusLib.h>
#include <Private/Library/PeiDciLib.h>
#include <Private/Library/PeiItssLib.h>
EFI_STATUS
EFIAPI
PchTraceHubPowerGateNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );

static EFI_PEI_NOTIFY_DESCRIPTOR  mPeiPchTraceHubNotifyList = {
  (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gMeBeforeDidSentPpiGuid,
  (EFI_PEIM_NOTIFY_ENTRY_POINT) PchTraceHubPowerGateNotifyCallback
};

/**
  Notify function to disable and power gate Pch trace hub

  @param[in]  PeiServices       General purpose services available to every PEIM.
  @param[in]  NotifyDescriptor  Notify that this module published.
  @param[in]  Ppi               PPI that was installed.

  @retval     EFI_SUCCESS       The function completed successfully.
  @retval     EFI_UNSUPPORTED   The device is not supported
**/
EFI_STATUS
EFIAPI
PchTraceHubPowerGateNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  return PchTraceHubDisable ();
}


/**
  Check some PCH policies are valid for debugging unexpected problem if these
  values are not initialized or assigned incorrect resource.

  @param[in] SiPreMemPolicy  The SI PreMem Policy PPI instance

**/
VOID
PchValidatePolicy (
  IN  SI_PREMEM_POLICY_PPI   *SiPreMemPolicy
  )
{
  EFI_STATUS                  Status;
  PCH_GENERAL_PREMEM_CONFIG   *PchGeneralPreMemConfig;
  PCH_TRACE_HUB_PREMEM_CONFIG *PchTraceHubPreMemConfig;
#ifndef CPU_CFL
  CPU_TRACE_HUB_CONFIG        *CpuTraceHubConfig;
#endif
  UINT32                      TotalTraceBufferSize;
  UINT32                      TraceHubBufferSizeTab[8] = {0, SIZE_1MB, SIZE_8MB, SIZE_64MB, SIZE_128MB, SIZE_256MB, SIZE_512MB, SIZE_1GB};

  TotalTraceBufferSize      = 0;
#ifndef CPU_CFL
  Status = GetConfigBlock((VOID *) SiPreMemPolicy, &gCpuTraceHubConfigGuid, (VOID *) &CpuTraceHubConfig);
  ASSERT_EFI_ERROR(Status);
  TotalTraceBufferSize += TraceHubBufferSizeTab[CpuTraceHubConfig->CpuTraceHubMemReg0Size] + TraceHubBufferSizeTab[CpuTraceHubConfig->CpuTraceHubMemReg1Size];
#endif
  Status = GetConfigBlock ((VOID *) SiPreMemPolicy, &gPchTraceHubPreMemConfigGuid, (VOID *) &PchTraceHubPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  TotalTraceBufferSize += TraceHubBufferSizeTab[PchTraceHubPreMemConfig->MemReg0Size] + TraceHubBufferSizeTab[PchTraceHubPreMemConfig->MemReg1Size];
  if (TotalTraceBufferSize > SIZE_512MB) {
    DEBUG ((DEBUG_ERROR, "Total Trace Hub Memory size over limited 512MB, reset trace hub memory region size to policy default\n"));
    PchTraceHubPreMemConfig->MemReg0Size = TraceBuffer64M;
    PchTraceHubPreMemConfig->MemReg1Size = TraceBuffer64M;
    DEBUG ((DEBUG_ERROR, "Enforce PCH TH mem reg 0/1 policy to 64MB/64MB\n"));
#ifndef CPU_CFL
    CpuTraceHubConfig->CpuTraceHubMemReg0Size = TraceBuffer128M;
    CpuTraceHubConfig->CpuTraceHubMemReg1Size = TraceBuffer128M;
    DEBUG ((DEBUG_ERROR, "Enforce CPU TH mem reg 0/1 policy to 128MB/128MB\n"));
#endif
  }

  Status = GetConfigBlock ((VOID *) SiPreMemPolicy, &gPchGeneralPreMemConfigGuid, (VOID *) &PchGeneralPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  ASSERT (SiPreMemPolicy->TableHeader.Header.Revision == SI_PREMEM_POLICY_REVISION);
}

/**
  Build Memory Mapped IO Resource which is used to build E820 Table in LegacyBios,
  the resource range should be preserved in ACPI as well.

**/
VOID
PchPreservedMmioResource (
  VOID
  )
{
  //
  // This function builds a HOB that describes a chunk of system memory.
  //
  BuildResourceDescriptorHob (
    EFI_RESOURCE_MEMORY_MAPPED_IO,
    (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
    PCH_PRESERVED_BASE_ADDRESS,
    PCH_PRESERVED_MMIO_SIZE
    );
  //
  // This function builds a HOB for the memory allocation.
  //
  BuildMemoryAllocationHob (
    PCH_PRESERVED_BASE_ADDRESS,
    PCH_PRESERVED_MMIO_SIZE,
    EfiMemoryMappedIO
    );
}

/**
  This function performs basic initialization for PCH in PEI phase after Policy produced.
  If any of the base address arguments is zero, this function will disable the corresponding
  decoding, otherwise this function will enable the decoding.
  This function locks down the AcpiBase.

  @param[in] SiPreMemPolicyPpi         The Silicon PreMem Policy PPI instance
**/
VOID
EFIAPI
PchOnPolicyInstalled (
  IN  SI_PREMEM_POLICY_PPI    *SiPreMemPolicyPpi
  )
{
  EFI_STATUS                  Status;
  PCH_GENERAL_PREMEM_CONFIG   *PchGeneralPreMemConfig;
  PCH_TRACE_HUB_PREMEM_CONFIG *PchTraceHubPreMemConfig;
  PCH_PCIE_RP_PREMEM_CONFIG   *PcieRpPreMemConfig;
  BOOLEAN                     DisableTraceHub;

  PostCode (0xB48);
  DEBUG ((DEBUG_INFO, "PchOnPolicyInstalled() - Start\n"));

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPchGeneralPreMemConfigGuid, (VOID *) &PchGeneralPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPchTraceHubPreMemConfigGuid, (VOID *) &PchTraceHubPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPcieRpPreMemConfigGuid, (VOID *) &PcieRpPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Set PWRM Base in PMC device
  ///
  Status = PmcSetPwrmBase (PCH_PWRM_BASE_ADDRESS);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Set ACPI Base in PMC device
  ///
  Status = PmcSetAcpiBase (PcdGet16 (PcdAcpiBaseAddress));
  ASSERT_EFI_ERROR (Status);

  //
  // Check to disable and lock WDT according to Policy.
  //
  WdtDisableAndLock (SiPreMemPolicyPpi);

  //
  // Configure BIOS HSIO Pre-Mem Settings
  //
  PostCode (0xB49);
  PchHsioBiosProgPreMem (SiPreMemPolicyPpi);

  EarlyPcieRpDisabling (PcieRpPreMemConfig);
  EarlyPcieNccHandling ();

  PostCode (0xB4A);
  DciConfiguration (SiPreMemPolicyPpi);

  PostCode (0xB4D);
  DisableTraceHub = TraceHubInitialize ((UINT8) PchTraceHubPreMemConfig->EnableMode);
  if (DisableTraceHub) {
    //
    // Register notify to power gate pch trace hub
    // It needs to be power gated between ME GetImrSize and SendDID
    //
    if (EFI_ERROR (PeiServicesNotifyPpi (&mPeiPchTraceHubNotifyList))) {
      ASSERT (FALSE);
    }
  }

  PostCode (0xB15);
  Status = SmbusInit (SiPreMemPolicyPpi);

  if (PchGeneralPreMemConfig->Port80Route == PchReservedPageToLpc) {
    PchDmiSetReservedPageRegToLpc ();
  } else {
    PchDmiSetReservedPageRegToPcieRootPort ();
  }

  PostCode (0xB52);
  P2sbHpetInit ();

  LpcOnPolicyConfigure (SiPreMemPolicyPpi);

  PostCode (0xB55);
  DEBUG ((DEBUG_INFO, "PchOnPolicyInstalled() - End\n"));
}

/**
  This code is intended to handle all cases where IP could be fuse disabled, static power gated
  or disabled due to soft strap lane assignment

  @retval EFI_SUCCESS
**/
VOID
PchEarlyDisabledDeviceHandling (
  VOID
  )
{
  UINTN                   RpStrpFuseCfg;
  UINT8                   CtrlIndex;
  UINT8                   RpIndex;
  UINT8                   MaxPciePortNum;
  UINT32                  Data32;
  UINT32                  SpxPcd[PCH_MAX_PCIE_CONTROLLERS];
  UINT32                  SpxMask;
  UINT8                   Response;
  EFI_STATUS              Status;
  UINT32                  SataCtrlIndex;

  ZeroMem (SpxPcd, sizeof (SpxPcd));

  MaxPciePortNum = GetPchMaxPciePortNum ();

  DEBUG ((DEBUG_INFO, "PchEarlyDisabledDeviceHandling() - Start\n"));

  ///
  /// BWG 5.3.1 Fuse Disabled, Static Disabled, non-Static function disable Device Handling
  ///

  //
  // Get PCIE Port configuration and disable value for each RP
  // and perform PSF function disable
  //
  for (CtrlIndex = 0; CtrlIndex < MaxPciePortNum / PCH_PCIE_CONTROLLER_PORTS; CtrlIndex++) {
    SpxPcd[CtrlIndex] = PchPcrRead32 (PchGetPcieControllerSbiPid (CtrlIndex), R_SPX_PCR_PCD);
    DEBUG ((DEBUG_INFO, "SP%c Controller PCD Value = %x\n", 'A' + CtrlIndex, SpxPcd[CtrlIndex]));
  }

  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex++) {
    CtrlIndex  = RpIndex / PCH_PCIE_CONTROLLER_PORTS;
    SpxMask = B_SPX_PCR_PCD_P1D << (RpIndex % PCH_PCIE_CONTROLLER_PORTS);
    if ((SpxPcd[CtrlIndex] & SpxMask) == SpxMask) {
      PsfDisablePcieRootPort (RpIndex);
    }
  }

  for (CtrlIndex = 0; CtrlIndex < MaxPciePortNum / PCH_PCIE_CONTROLLER_PORTS; CtrlIndex++) {
    DEBUG ((DEBUG_INFO, "Multilane check for PCIE Controller %d: ", CtrlIndex));
    RpIndex = CtrlIndex * PCH_PCIE_CONTROLLER_PORTS;
    //
    // Get PCIE PCI register from SBI in case it's disabled.
    //
    Status = PchSbiRpPciRead32 (RpIndex, R_PCH_PCIE_CFG_STRPFUSECFG, &Data32);
    if (Status == EFI_SUCCESS) {
      RpStrpFuseCfg = (Data32 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC;
      switch (RpStrpFuseCfg) {
        case V_PCH_PCIE_CFG_STRPFUSECFG_RPC_4:
          //
          // Disable ports 2, 3, 4 of a controller when it's set to 1 x4
          //
          DEBUG ((DEBUG_INFO, "Disabling RP %d/%d/%d\n", RpIndex+2, RpIndex+3, RpIndex+4));
          PsfDisablePcieRootPort (RpIndex + 1);
          PsfDisablePcieRootPort (RpIndex + 2);
          PsfDisablePcieRootPort (RpIndex + 3);
          break;
        case V_PCH_PCIE_CFG_STRPFUSECFG_RPC_2_2:
          //
          // Disable ports 2, 4 of a controller when it's set to 2 x2
          //
          DEBUG ((DEBUG_INFO, "Disabling RP %d/%d\n", RpIndex+2, RpIndex+4));
          PsfDisablePcieRootPort (RpIndex + 1);
          PsfDisablePcieRootPort (RpIndex + 3);
          break;
        case V_PCH_PCIE_CFG_STRPFUSECFG_RPC_2_1_1:
          //
          // Disable port 2 of a controller when it's set to 1 x2
          //
          DEBUG ((DEBUG_INFO, "Disabling RP %d\n", RpIndex+2));
          PsfDisablePcieRootPort (RpIndex + 1);
          break;
        case V_PCH_PCIE_CFG_STRPFUSECFG_RPC_1_1_1_1:
        default:
          DEBUG ((DEBUG_INFO, "No ports disabled\n"));
          break;
      }
    }
  }

  //
  // OTG fused
  //
  if (!PmcIsXdciSupported ()) {
    DEBUG ((DEBUG_INFO, "xDCI (OTG) Fuse disabled - performing PSF disable\n"));
    PsfDisableXdciDevice ();
  }
  //
  // SATA fused
  //
  for (SataCtrlIndex = 0; SataCtrlIndex < GetPchMaxSataControllerNum (); SataCtrlIndex++) {
    if (!PmcIsSataSupported (SataCtrlIndex)) {
      DEBUG ((DEBUG_INFO, "SATA %d Fuse disabled - performing PSF disable\n", SataCtrlIndex + 1));
      PsfDisableSataDevice (SataCtrlIndex);
    }
  }
  //
  // LPSS fused
  //
  if (!PmcIsSerialIoSupported ()) {
    DEBUG ((DEBUG_INFO, "SerialIo SPI1 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoSpiPort (1));
    DEBUG ((DEBUG_INFO, "SerialIo SPI0 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoSpiPort (0));
    DEBUG ((DEBUG_INFO, "SerialIo SPI2 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoSpiPort (2));
    DEBUG ((DEBUG_INFO, "SerialIo UART2 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoUartPort (2));
    DEBUG ((DEBUG_INFO, "SerialIo UART1 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoUartPort (1));
    DEBUG ((DEBUG_INFO, "SerialIo UART0 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoUartPort (0));
    if (IsPchLp ()) {
      DEBUG ((DEBUG_INFO, "SerialIo I2C5 Static PG disabled - performing PSF disable\n"));
      PsfDisableDevice (PsfSerialIoI2cPort (5));
      DEBUG ((DEBUG_INFO, "SerialIo I2C4 Static PG disabled - performing PSF disable\n"));
      PsfDisableDevice (PsfSerialIoI2cPort (4));
    }
    DEBUG ((DEBUG_INFO, "SerialIo I2C3 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoI2cPort (3));
    DEBUG ((DEBUG_INFO, "SerialIo I2C2 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoI2cPort (2));
    DEBUG ((DEBUG_INFO, "SerialIo I2C1 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoI2cPort (1));
    DEBUG ((DEBUG_INFO, "SerialIo I2C0 Static PG disabled - performing PSF disable\n"));
    PsfDisableDevice (PsfSerialIoI2cPort (0));
  }
  //
  // For H, put SerialIo I2C4 and I2C5 in D3, and set function disable in PMC
  //
  if (IsPchH ()) {
    DEBUG ((DEBUG_INFO, "Put SerialIo I2C4 in D3\n"));
    Data32 = BIT18 | BIT17 | BIT16;
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_D0I3MAXDEVPG,
      PciConfigWrite,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C4 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C4),
      &Data32,
      &Response
      );

    Data32 = BIT1 | BIT0;
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_PME_CTRL_STS,
      PciConfigWrite,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C4 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C4),
      &Data32,
      &Response
      );

    //
    // perform a read to avoid ordering and noncoherency problems
    //
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_PME_CTRL_STS,
      PciConfigRead,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C4 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C4),
      &Data32,
      &Response
      );

    DEBUG ((DEBUG_INFO, "Put SerialIo I2C5 in D3\n"));
    Data32 = BIT18 | BIT17 | BIT16;
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_D0I3MAXDEVPG,
      PciConfigWrite,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C5 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C5),
      &Data32,
      &Response
      );

    Data32 = BIT1 | BIT0;
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_PME_CTRL_STS,
      PciConfigWrite,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C5 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C5),
      &Data32,
      &Response
      );

    //
    // perform a read to avoid ordering and noncoherency problems
    //
    PchSbiExecutionEx (
      PID_SERIALIO,
      R_SERIAL_IO_CFG_PME_CTRL_STS,
      PciConfigRead,
      FALSE,
      0xF,
      0,
      ((PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C5 << 3) | PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C5),
      &Data32,
      &Response
      );
  }

  //
  // SCC fused
  //
  if (IsPchLp ()) {
    if (!PmcIsScsEmmcSupported ()) {
      DEBUG ((DEBUG_INFO, "SCC eMMC Fuse disabled - performing PSF disable\n"));
      PsfDisableDevice (PsfScsEmmcPort ());
    }

    if (!PmcIsScsSdCardSupported ()) {
      DEBUG ((DEBUG_INFO, "SCC SD Card Fuse disabled - performing PSF disable\n"));
      PsfDisableDevice (PsfScsSdCardPort ());
    }

    if (!PmcIsScsUfsSupported (0)) {
      DEBUG ((DEBUG_INFO, "SCC UFS Fuse disabled - performing PSF disable\n"));
      PsfDisableDevice (PsfScsUfsPort (0));
    }
  }

  //
  // ISH fused
  //
  if (!PmcIsIshSupported ()) {
    DEBUG ((DEBUG_INFO, "ISH Fuse disabled - performing PSF disable\n"));
    PsfDisableIshDevice ();
  }
  //
  // Gbe fused
  //
  if (!PmcIsGbeSupported ()) {
    DEBUG ((DEBUG_INFO, "GBE Fuse disabled - performing PSF disable\n"));
    PsfDisableGbeDevice ();
  }

  if (!PmcIsCnviSupported ()) {
    DEBUG ((DEBUG_INFO, "CNVi WiFi function Fuse disabled - performing PSF disable\n"));
    PsfDisableCnviWifiDevice ();
  }

  DEBUG ((DEBUG_INFO, "PchEarlyDisabledDeviceHandling() - End\n"));
}

/**
  Show PCH related information
**/
VOID
PchShowInformation (
  VOID
  )
{
  DEBUG_CODE_BEGIN ();
  CHAR8           Buffer[PCH_STEPPING_STR_LENGTH_MAX];

  DEBUG ((DEBUG_INFO, "PCH Series   : %a\n", PchGetSeriesStr ()));
  PchGetSteppingStr (Buffer, sizeof (Buffer));
  DEBUG ((DEBUG_INFO, "PCH Stepping : %a\n", Buffer));
  DEBUG ((DEBUG_INFO, "PCH SKU      : %a\n", PchGetSkuStr ()));
  DEBUG_CODE_END ();
}

/**
  PCH init pre-memory entry point
**/
VOID
EFIAPI
PchInitPrePolicy (
  VOID
  )
{
  //
  // Dump PCH information
  //
  DEBUG_CODE_BEGIN ();
  PchShowInformation ();
  PchFiaPrintLosRegisters ();
  DEBUG_CODE_END ();

  PostCode (0xB00);
  DEBUG ((DEBUG_INFO, "PchInitPrePolicy() - Start\n"));

  //
  // Check if Pch is supported
  //
  if (!IsPchSupported ()) {
    DEBUG ((DEBUG_ERROR, "PCH SKU is not supported due to no proper PCH LPC found!\n"));
    ASSERT (FALSE);
  }

  //
  // Check if SBREG has been set.
  //
  if (!P2sbIsSbregReady ()) {
    DEBUG ((DEBUG_INFO, "SBREG should be programmed before here\n"));
    ASSERT (FALSE);
  }

  //
  // Check if PCH PWRM Base has been set
  //
  DEBUG ((DEBUG_INFO, "PCH PWRM Base needs to be programmed before here\n"));
  ASSERT (PsfGetPmcPwrmBase () != 0);

  //
  // Build Memory Mapped IO Resource which is used to build E820 Table in LegacyBios.
  // the resource range should be preserved in ACPI as well.
  //
  PchPreservedMmioResource ();

#ifndef FSP_FLAG
  ///
  /// If there was unexpected reset but no WDT expiration and no resume from S3/S4,
  /// clear unexpected reset status and enforce expiration. This is to inform Firmware
  /// which has no access to unexpected reset status bit, that something went wrong.
  ///
  OcWdtResetCheck ();
#endif
  //
  // Initialize WDT and install WDT PPI
  //
  WdtInstallResetCallback ();
  OcWdtInit ();

  //
  // Install EFI PEI RESET2 PeiService
  //
  PchInitializeReset ();

  //
  // Installs PCH SPI PPI
  //
  SpiServiceInit ();

  PostCode (0xB02);

  //
  // Perform PSF early initialization.
  //
  PsfEarlyInit ();

  //
  // Before any interrupt is enabled, set master message enable.
  //
  ItssSetMasterMessageEnable ();

  //
  // PCH-H DMI early init (before any upstream bus master transactions are enabled)
  //
  if (IsPchH ()) {
    PchDmi15EarlyInit ();
  }

  //
  // According to bios spec,
  // Setup "uCode Patch Region Enable" in DMI
  //
  PchDmiEnableUCodePatchRegion ();

  //
  // Do P2SB early configuration.
  //
  P2sbEarlyConfig ();

  //
  // Configura RTC when power failure case.
  //
  if (!PmcIsRtcBatteryGood ()) {
    RtcPowerFailureConfiguration ();
  }

  //
  // Clear CF9GR if it's set in previous boot.
  //
  PmcDisableCf9GlobalReset ();

  //
  // Enhance port 8xh LPC decoding
  //
  LpcOnEarlyPeiConfigure ();

  //
  // Handle all static disabled controllers.
  //
  PchEarlyDisabledDeviceHandling ();

  PostCode (0xB7F);
  DEBUG ((DEBUG_INFO, "PchInitPrePolicy() - End\n"));
}

