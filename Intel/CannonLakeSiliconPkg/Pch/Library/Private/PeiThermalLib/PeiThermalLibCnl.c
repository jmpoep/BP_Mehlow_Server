/** @file
  Initializes PCH thermal controller for CNL.

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

#include <Uefi/UefiBaseType.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/Pci30.h>
#include <Library/PchInfoLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PmcLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PchDmiLib.h>
#include <Private/Library/PeiThermalLib.h>
#include <Private/Library/CpuInitLib.h>
#include <Private/Library/PeiItssLib.h>

/**
  Perform Thermal Management Support initialization

  @param[in] SiPolicyPpi          The SI Policy PPI instance

**/
VOID
ThermalInit (
  IN  SI_POLICY_PPI             *SiPolicyPpi
  )
{
  EFI_STATUS                    Status;
  UINT64                        ThermalPciBase;
  UINT32                        PchPwrmBase;
  UINT32                        ThermalBase;
  UINT16                        Data16;
  UINT32                        Data32;  //UPServer
  UINT32                        PchTTLevels;
  UINT32                        Temperature;
  UINT8                         OrgCmd;
  UINT32                        OrgTbarLow;
  UINT32                        OrgTbarHigh;
  PCH_THERMAL_CONFIG            *ThermalConfig;
  DMI_THERMAL_THROTTLING        DmiThermalThrottling;
  UINT32                        TempMemBaseAddr;
  UINT8                         InterruptPin;

  DEBUG ((DEBUG_INFO, "ThermalInit () Start\n"));

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gThermalConfigGuid, (VOID *) &ThermalConfig);
  ASSERT_EFI_ERROR (Status);

  OrgCmd      = 0;
  OrgTbarLow  = 0;
  OrgTbarHigh = 0;

  ThermalPciBase = PCI_SEGMENT_LIB_ADDRESS (
                     DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_THERMAL,
                     PCI_FUNCTION_NUMBER_PCH_THERMAL,
                     0
                     );

  TempMemBaseAddr = PcdGet32 (PcdSiliconInitTempMemBaseAddr);

  ThermalBase = TempMemBaseAddr;

  //
  // Save original COMMAND and TBAR register
  // The TBAR might be assigned during S3 resume path.
  //
  OrgCmd      = PciSegmentRead8 (ThermalPciBase + PCI_COMMAND_OFFSET);
  OrgTbarLow  = PciSegmentRead32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBAR);
  OrgTbarHigh = PciSegmentRead32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBARH);

  PchPwrmBase = PmcGetPwrmBase ();

  ///
  /// Thermal PCI offset 10h[31:0], with a 64-bit BAR for BIOS.
  /// Enable MSE of Thermal PCI offset 04h[1] to enable memory decode
  ///
  PciSegmentAnd8 (ThermalPciBase + PCI_COMMAND_OFFSET, (UINT8) ~EFI_PCI_COMMAND_MEMORY_SPACE);
  PciSegmentWrite32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBAR, ThermalBase);
  PciSegmentWrite32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBARH, 0);
  PciSegmentOr8 (ThermalPciBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);

  ///
  /// Thermal Subsystem Device Initialization
  /// The System BIOS must perform the following steps to initialize the PCH thermal subsystem device.
  /// Step 1
  /// Enable Thermal Subsystem device by making sure FD.TTD is cleared.
  /// The default value of FD.TTD is cleared.
  ///
  /// Step 2
  /// Optionally program Thermal controller Interrupt Pin/Route registers
  /// Left this to platform code
  ///
  /// Step 3
  /// Go through general PCI enumeration and assign standard PCI resource, etc.
  /// Left this to platform code
  ///
  /// Step 4
  /// Initialize relevant Thermal subsystems for the desired features.
  ///

  ///
  /// Initializing Thermal Sensors
  /// Step 1
  /// - Set various trip points based on the particular usage model.  Note that Cat Trip must always be programmed.
  /// - CTT must be programmed for Cat Trip, CTT must never be changed while the TS enable is set.
  ///   This rule prevents a spurious trip from occurring and causing a system shutdown.
  ///   TSC must then be written to 0x81 to enable the power down and lock the register.
  ///   TSC is programmed later.
  /// - TAHV and TAHL may be programmed if the BIOS or driver wish to force a SW notification for PCH temperature
  ///    - If TAHL/TAHV programmed much later in the flow when a driver is loaded, this means that the TS had been
  ///      enabled long before this, the thermal sensor must be disabled when TAHL/TAHV are programmed, and then
  ///      re-enabled.
  ///    - TSPIEN or TSGPEN may be programmed to cause either an interrupt or SMI/SCI.
  ///    - It is recommended that TAHV, TALV, TSPIEN and TSGPEN be left at their default value, unless there is a
  ///      specific usage that requires these to be programmed.
  ///
  MmioWrite16 (ThermalBase + R_THERMAL_MEM_CTT, 0x0154);

  ///
  /// Step 2
  /// Clear trip status from TSS/TAS. BIOS should write 0xFF to clear any bit that was inadvertently set while programming
  /// the TS. This write of 0xFF should be done before continuing to the next steps.
  ///
  MmioWrite8 (ThermalBase + R_THERMAL_MEM_TSS, 0xFF);
  MmioWrite8 (ThermalBase + R_THERMAL_MEM_TAS, 0xFF);

  ///
  /// Step 3
  /// Enable the desired thermal trip alert methods, i.e. GPE (TSGPEN), SMI (TSMIC) or Interrupt (TSPIEN).
  /// Only one of the methods should be enabled and the method will be depending on the platform implementation.
  /// - TSGPEN: BIOS should leave this as default 00h, unless it is required to enable GPE.
  /// - TSMIC: BIOS should leave TSMIC[7:0] as default 00h, unless the SMI handler is loaded
  ///   and it's safe to enable SMI for these events.
  /// - TSPIEN: BIOS should leave this as default 0x00, so that a driver can enable later
  ///
  //Leave the default settings.
  //MmioWrite8 (ThermalBase + R_THERMAL_MEM_TSGPEN, 0x00);
  //MmioWrite8 (ThermalBase + R_THERMAL_MEM_TSMIC, 0x00);
  //MmioWrite8 (ThermalBase + R_THERMAL_MEM_TSPIEN, 0x00);

  ///
  /// Step 4
  /// If thermal reporting to an EC over SMBus is supported, then write 0x01 to TSREL, else leave at default.
  ///
  MmioWrite8 (ThermalBase + R_THERMAL_MEM_TSREL, 0x01);

  ///
  /// Step 5
  /// If the PCH_Hot pin reporting is supported, then write the temperature value and set the enable in PHL.
  /// Note: For PCHHOT# support, we need to make sure if PCHHOT# pin is set to native mode.
  /// And the value in PHL register is valid only if it is between 00h and 1FFh.
  ///
  if ((ThermalConfig->PchHotEnable) &&
      (ThermalConfig->PchHotLevel < 0x0200)) {
    ///
    /// Enable PCHHOT# pin
    ///
    GpioEnablePchHot ();
    ///
    /// Program PHL register according to PchHotLevel setting.
    ///
    Data16 = (ThermalConfig->PchHotLevel | B_THERMAL_MEM_PHLE);
    MmioWrite16 (ThermalBase + R_THERMAL_MEM_PHL, Data16);
  }

  ///
  /// Step 6
  /// If thermal throttling is supported, then set the desired values in TL.
  ///
  if (ThermalConfig->TTLevels.SuggestedSetting == FALSE) {
    ///
    /// Trip Point Temperature = (Trip Point Register [8:0]) / 2 - 50 centigrade degree
    /// If Trip Point Temperature <= T0Level, the system will be in T0 state.
    /// If T1Level >= Trip Point Temperature > T0Level, the system will be in T1 state.
    /// If T2Level >= Trip Point Temperature > T1Level, the system will be in T2 state.
    /// If Trip Point Temperature > T2Level, the system will be in T3 state.
    ///
    PchTTLevels = (UINT32) (((ThermalConfig->TTLevels.T2Level + 50) * 2) << 20) |
      (UINT32) (((ThermalConfig->TTLevels.T1Level + 50) * 2) << 10) |
      (UINT32)  ((ThermalConfig->TTLevels.T0Level + 50) * 2);
    MmioWrite32 (ThermalBase + R_THERMAL_MEM_TL, PchTTLevels);

    MmioOr32 (
      ThermalBase + R_THERMAL_MEM_TL,
      (UINT32) (ThermalConfig->TTLevels.TTLock << 31) |
      (UINT32) (ThermalConfig->TTLevels.TTState13Enable << 30) |
      (UINT32) (ThermalConfig->TTLevels.TTEnable << 29)
      );
  } else {
    //
    // Check if PCH LP, it also indicates the ULT.
    //
    if (IsPchLp () && (ThermalConfig->TTLevels.PchCrossThrottling == TRUE)) {
      ///
      /// If processor is capable of cross throttling and it is enabled
      ///   T0L = ((cross throlling trip point + 50) * 2)
      ///   T1L = T0L + 5'C
      ///   T2L = T1L + 5'C
      ///   Set TBAR + 40h[31:29] = 101b
      ///   Set TBAR + 50h[14] = 1b
      ///   Set PWRMBASE + 18C4h[26:24] = 101b
      ///
      //
      // Check RATL mode by checking MSR 1A2h[6:0] != 0
      //
      Temperature = CpuGetCrossThrottlingTripPoint ();
      PchTTLevels = (UINT32) (((Temperature + 10 + 50) * 2) << 20) |
        (UINT32) (((Temperature +  5 + 50) * 2) << 10) |
        (UINT32) ((Temperature + 50) * 2);
      ///
      /// Program TBAR + 40h[28:0]
      ///
      MmioWrite32 (ThermalBase + R_THERMAL_MEM_TL, PchTTLevels);
      ///
      /// Program TBAR + 40h[31:29]
      /// TL [31] locks the thermal throttling registers
      ///
      MmioOr32 (ThermalBase + R_THERMAL_MEM_TL, B_THERMAL_MEM_TL_LOCK | B_THERMAL_MEM_TL_TTEN);
      ///
      /// Set TBAR + 50h[14] = 1b
      ///
      MmioOr16 (ThermalBase + R_THERMAL_MEM_TL2, R_THERMAL_MEM_TL2_PMCTEN);
      ///
      /// Set PWRMBASE + C4h[26:24] = 101b
      ///
      MmioAndThenOr32 (PchPwrmBase + R_PMC_PWRM_PMSYNC_TPR_CONFIG,
        (UINT32) ~(B_PMC_PWRM_PMSYNC_PCH2CPU_TT_STATE),
        (B_PMC_PWRM_PMSYNC_PCH2CPU_TT_EN | (V_PMC_PWRM_PMSYNC_PCH2CPU_TT_STATE_1 << N_PMC_PWRM_PMSYNC_PCH2CPU_TT_STATE))
        );
    } else {
      ///
      /// Set TBAR + 40h[28:20] = 148h
      /// Set TBAR + 40h[18:10] = 142h
      /// Set TBAR + 40h[8:0] = 13Ch
      /// Set TBAR + 40h[31:29] = 101b in separate write
      ///
      PchTTLevels = ((0x148 << 20) | (0x142 << 10) | (0x13C));
      MmioWrite32 (ThermalBase + R_THERMAL_MEM_TL, PchTTLevels);
      MmioOr32 (ThermalBase + R_THERMAL_MEM_TL, B_THERMAL_MEM_TL_LOCK | B_THERMAL_MEM_TL_TTEN);
    }
  }

  if (IsPchLp ()) {
    ///
    /// Lock PMSYNC_TPR_CFG
    ///
    MmioOr32 (PchPwrmBase + R_PMC_PWRM_PMSYNC_TPR_CONFIG, B_PMC_PWRM_PMSYNC_TPR_CONFIG_LOCK);
  }

  ///
  /// Step 7
  /// Enable thermal sensor power management
  ///   Write C8h to TSPM[7:0]
  ///   Write 101b to TSPM[11:9]
  ///   Write 100b to TSPM[14:12]
  ///   Lock the register by writing '1' to TSPM[15], do it later
  ///
  MmioWrite16 (ThermalBase + R_THERMAL_MEM_TSPM, 0x4AC8);

  ///
  /// Step 9
  /// Program ThermalBar + 0xA4[4,3,2,1,0] = 1b,1b,1b,1b,1b
  ///
  MmioOr16 (ThermalBase + R_THERMAL_MEM_A4, (BIT4 | BIT3 | BIT2 | BIT1 | BIT0));
  ///
  ///  UPServer
  ///
  /// 
  /// Override DTS fuse setting before thermal sensor enable
  /// if the fuse data is not expected, override the fuse setting.
  ///
  Data32 = MmioRead32 (ThermalBase + R_THERMAL_MEM_C4);
  if ((Data32 != 0) && (Data32 & 0x0000001E) != 0xE) {
    MmioWrite32 (ThermalBase + R_THERMAL_MEM_D0, (Data32 & ~(UINT32)0x0000001E) | BIT31 | 0xE);
  }

  ///
  ///  UPServer
  ///
  ///
  /// Step 10
  ///   Enable thermal sensor by programming TSEL register to 0x01.
  ///   This should be done after all thermal initialization steps are finished.
  ///
  MmioOr8 (ThermalBase + R_THERMAL_MEM_TSEL, B_THERMAL_MEM_TSEL_ETS);

  //
  // Per interface throttling actions
  //

  if (IsPchH ()) {
    ///
    /// DMI Thermal Throttling
    ///
    if (ThermalConfig->DmiHaAWC.SuggestedSetting) {
      PchDmiSetRecommendedThermalThrottling ();
    } else if (ThermalConfig->DmiHaAWC.DmiTsawEn) {
      DmiThermalThrottling.ThermalSensor0TargetWidth = ThermalConfig->DmiHaAWC.TS0TW;
      DmiThermalThrottling.ThermalSensor1TargetWidth = ThermalConfig->DmiHaAWC.TS1TW;
      DmiThermalThrottling.ThermalSensor2TargetWidth = ThermalConfig->DmiHaAWC.TS2TW;
      DmiThermalThrottling.ThermalSensor3TargetWidth = ThermalConfig->DmiHaAWC.TS3TW;
      PchDmiSetCustomThermalThrottling (DmiThermalThrottling);
    }
  }

  ///
  /// Configure Thermal device interrupt
  ///
  InterruptPin = ItssGetDevIntPin (SiPolicyPpi, PCI_DEVICE_NUMBER_PCH_THERMAL, PCI_FUNCTION_NUMBER_PCH_THERMAL);

  PciSegmentWrite8 (ThermalPciBase + PCI_INT_PIN_OFFSET, InterruptPin);

  //
  // Security Lock
  //

  ///
  /// It tis recommended that TSC[7] set to '1' to lock the CAT Trip behavior
  /// TSC must then be written to 0x81 to enable the power down and lock the register.
  ///
  MmioOr8 (ThermalBase + R_THERMAL_MEM_TSC, B_THERMAL_MEM_TSC_PLD | B_THERMAL_MEM_TSC_CPDE);

  ///
  /// TSMIC [7] locks SMI reporting of thermal events
  ///
  if (ThermalConfig->TsmicLock == TRUE) {
    MmioOr8 (ThermalBase + R_THERMAL_MEM_TSMIC, B_THERMAL_MEM_TSMIC_PLD);
  }

  ///
  /// PHLC[0] locks the PHL and PHLC registers for PCH_HOT#
  ///
  MmioOr8 (ThermalBase + R_THERMAL_MEM_PHLC, B_THERMAL_MEM_PHLC_LOCK);
  ///
  /// TL[31] locks the thermal throttling registers
  /// Done above.
  ///

  ///
  /// TL2[15] locks the thermal throttling 2 register
  ///
  MmioOr16 (ThermalBase + R_THERMAL_MEM_TL2, R_THERMAL_MEM_TL2_LOCK);
  ///
  /// TSEL [7] locks the thermal sensor enable, after TAHV and TAHL are
  /// programmed by BIOS or driver later in case
  ///
  MmioOr8 (ThermalBase + R_THERMAL_MEM_TSEL, B_THERMAL_MEM_TSEL_PLD);

  ///
  /// Lock TSPM the register by writing '1' to TSPM[15]
  ///
  MmioOr16 (ThermalBase + R_THERMAL_MEM_TSPM, B_THERMAL_MEM_TSPM_TSPMLOCK);

  ///
  /// Restore BAR and command register and disable memory decode
  ///
  PciSegmentAnd8 (ThermalPciBase + PCI_COMMAND_OFFSET, (UINT8) ~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER));
  //
  // Restore original COMMAND and TBAR register
  // The TBAR might be assigned during S3 resume path.
  //
  PciSegmentWrite32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBAR, OrgTbarLow);
  PciSegmentWrite32 (ThermalPciBase + R_THERMAL_CFG_MEM_TBARH, OrgTbarHigh);
  PciSegmentWrite8 (ThermalPciBase + PCI_COMMAND_OFFSET, OrgCmd);

  DEBUG ((DEBUG_INFO, "ThermalInit () End\n"));
}

/**
  Init Memory Throttling.

  @param[in] SiPolicy                   SI policy PPI instance.
**/
VOID
MemoryThrottlingInit (
  IN  SI_POLICY_PPI                     *SiPolicy
  )
{
  EFI_STATUS                            Status;
  UINT32                                PwrmBase;
  UINT32                                Data32;
  PCH_THERMAL_CONFIG                    *ThermalConfig;
  PCH_MEMORY_THROTTLING                 *MemoryThrottling;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gThermalConfigGuid, (VOID *) &ThermalConfig);
  ASSERT_EFI_ERROR (Status);

  MemoryThrottling = &(ThermalConfig->MemoryThrottling);

  if (MemoryThrottling->Enable == 0) {
    return;
  }

  PwrmBase = PmcGetPwrmBase ();
  //
  // Select 0: CPU_GP_0 (default) or 1: CPU_GP_1 for TsGpioC
  // Select 0: CPU_GP_3 (default) or 1: CPU_GP_2 for TsGpioD
  // Set PWRMBASE offset 0x18C8 [11:10] accordingly.
  //
  Data32 = 0;
  if (MemoryThrottling->TsGpioPinSetting[TsGpioC].PinSelection) {
    Data32 |= B_PMC_PWRM_PMSYNC_GPIO_C_SEL;
  }
  if (MemoryThrottling->TsGpioPinSetting[TsGpioD].PinSelection) {
    Data32 |= B_PMC_PWRM_PMSYNC_GPIO_D_SEL;
  }
  MmioAndThenOr32 (
    PwrmBase + R_PMC_PWRM_PMSYNC_MISC_CFG,
    (UINT32) ~(B_PMC_PWRM_PMSYNC_GPIO_C_SEL | B_PMC_PWRM_PMSYNC_GPIO_D_SEL),
    Data32
    );

  //
  // Enable Enable GPIO_C/GPIO_D PMSYNC Pin Mode in C0
  // Set PWRMBASE offset 0x18F4 [15:14] accordingly.
  //
  Data32 = 0;
  if (MemoryThrottling->TsGpioPinSetting[TsGpioC].C0TransmitEnable) {
    Data32 |= BIT14;
  }
  if (MemoryThrottling->TsGpioPinSetting[TsGpioD].C0TransmitEnable) {
    Data32 |= BIT15;
  }
  MmioAndThenOr32 (
    PwrmBase + R_PMC_PWRM_PM_SYNC_MODE_C0,
    (UINT32) ~(BIT15 | BIT14),
    Data32
    );

  //
  // Enable GPIO_C/GPIO_D PMSYNC pin mode.
  // Set PWRMBASE offset 0x18D4 [15:14] accordingly.
  // Override to GPIO native mode.
  //
  Data32 = 0;
  if (MemoryThrottling->TsGpioPinSetting[TsGpioC].PmsyncEnable) {
    Data32 |= BIT14;
    if (MemoryThrottling->TsGpioPinSetting[TsGpioC].PinSelection == 0) {
      GpioEnableCpuGpPin (0);
    } else {
      GpioEnableCpuGpPin (1);
    }
  }
  if (MemoryThrottling->TsGpioPinSetting[TsGpioD].PmsyncEnable) {
    Data32 |= BIT15;
    if (MemoryThrottling->TsGpioPinSetting[TsGpioD].PinSelection == 0) {
      GpioEnableCpuGpPin (3);
    } else {
      GpioEnableCpuGpPin (2);
    }
  }
  MmioAndThenOr32 (
    PwrmBase + R_PMC_PWRM_PM_SYNC_MODE,
    (UINT32) ~(BIT15 | BIT14),
    Data32
    );
}
