/** @file
  This library contains power management configuration functions for processors.

  <b>Acronyms:</b>
     - PPM:  Processor Power Management
     - TM:   Thermal Monitor
     - IST:  Intel(R) Speedstep technology
     - HT:   Hyper-Threading Technology

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2017 Intel Corporation.

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

#include "PowerMgmtCommon.h"

/**
  Get TCC cross throllling teamperature.
  Set Thermal trip point temperature indicated by MSR 1A2h
  If (RATL mode), T0L = MSR 1A2h[23:16]
  Else          , T0L = MSR 1A2h[23:16] - MSR 1A2h[29:24]

  @retval  Value of the activation temperature
**/
UINT32
CpuGetCrossThrottlingTripPoint (
  VOID
  )
{
  MSR_REGISTER TempMsr;
  UINT32       Temperature;

  TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
  //
  // Check RATL mode by checking MSR 1A2h[6:0] != 0
  //
  if ((TempMsr.Bytes.FirstByte & B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_TIME_WINDOW) != 0) {
    Temperature = TempMsr.Bytes.ThirdByte;
  } else {
    Temperature = TempMsr.Bytes.ThirdByte - (TempMsr.Bytes.FouthByte & V_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_OFFSET_MASK);
  }

  return Temperature;
}

/**
  This will perform general thermal initialization other then TM1, TM2, or
  PROCHOT# on all logical processors.

  @param[in] PeiServices    Pointer to PEI Services Table
**/
VOID
InitThermal (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  MSR_REGISTER TempMsr;
  UINT8        MaxRefTemp;
  UINT8        TccActivationOffsetMask;

  ///
  /// Run thermal code on all logical processors.
  ///
  ApSafeInitThermal (gCpuPowerMgmtTestConfig);
  gMpServicesPpi->StartupAllAPs (
                    PeiServices,
                    gMpServicesPpi,
                    (EFI_AP_PROCEDURE) ApSafeInitThermal,
                    FALSE,
                    0,
                    (VOID *) gCpuPowerMgmtTestConfig
                    );
  ///
  /// Automatic Thermal Reporting for Thermal Management
  ///
  if (gCpuPowerMgmtTestConfig->AutoThermalReporting) {
    ///
    /// Thermal Reporting for Critical trip
    /// MSR 1A2 bits 23:16 define the temperature that this specific processor can
    /// function upto. It is recommended that this value + 5  be used as default Critical trip point
    /// _CRT.
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
    TempMsr.Qword &= (UINT64) B_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_TEMPERATURE_MASK;
    MaxRefTemp  = (UINT8) RShiftU64 (TempMsr.Qword, N_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_TEMPERATURE_OFFSET);
    gCpuNvsAreaConfig->Area->AutoCriticalTripPoint = MaxRefTemp + 5;
    ///
    /// Thermal Reporting for Active Thermal Management
    /// It is recommended that the processor specific value in MSR 1A2 bits 15:8
    /// be used as the highest Active trip point i.e. _AC0.
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
    TempMsr.Qword &= (UINT64) B_MSR_TEMPERATURE_TARGET_FAN_TEMP_TARGET_OFFSET;
    gCpuNvsAreaConfig->Area->AutoActiveTripPoint = MaxRefTemp - (UINT8) RShiftU64 (
                                                                                TempMsr.Qword,
                                                                                N_MSR_TEMPERATURE_TARGET_FAN_TEMP_TARGET_OFFSET
                                                                                );

    ///
    /// Tcc activation offset in temperature target MSR changes from 4 bits [27:24] to 6 bits [29:24] on ULT C step onwards
    ///
    if (GetCpuSku () == EnumCpuUlt) {
      TccActivationOffsetMask = 0x3F;
    } else {
      TccActivationOffsetMask = 0xF;
    }

    ///
    /// Thermal Reporting for Passive Thermal Management
    /// On all turbo enabled systems, it is recommended that the ACPI _PSV point be
    /// set to a temperature above the Active cooling temperature and Tcc activation
    /// temperature.
    /// If platform embedded controller will issue PECI commands to reduce power as a
    /// passive thermal action, then it is recommended to use the package's max temperature
    /// for passive thermal control.
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);
    TempMsr.Qword &= (UINT64) (TccActivationOffsetMask << N_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LIMIT);
    gCpuNvsAreaConfig->Area->AutoPassiveTripPoint = gCpuNvsAreaConfig->Area->AutoCriticalTripPoint+3;
  }

  EnableProcHot ();
  return;
}

/**
  This will perform enable thermal initialization. TM1, TM2 and adaptive thermal
  throttling are enabled/disabled together.

  This function must be MP safe.

  @param[in] Buffer    Pointer to the function parameters passed in.
**/
VOID
EFIAPI
ApSafeInitThermal (
  IN OUT VOID *Buffer
  )
{
  MSR_REGISTER                      TempMsr;
  CPU_POWER_MGMT_TEST_CONFIG        *CpuPowerMgmtTestConfig;
  MSR_IA32_THERM_INTERRUPT_REGISTER ThermInterruptMsr;

  ///
  /// Extract parameters from the buffer
  ///
  CpuPowerMgmtTestConfig = (CPU_POWER_MGMT_TEST_CONFIG *) Buffer;
  if (CpuPowerMgmtTestConfig == NULL) {
     return;
  }
  ///
  /// Configure Adaptive thermal monitor. IA32_MISC_ENABLE[3]
  /// (1A0h)IA32_MISC_ENABLE - Bit3:Intel Adaptive Thermal Monitor Enable
  ///   System BIOS must always set this bit to be operating within spec.
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  TempMsr.Dwords.Low |= B_MSR_IA32_MISC_ENABLE_TME;
  if (CpuPowerMgmtTestConfig->ThermalMonitor == 0) {
    TempMsr.Dwords.Low &= ~B_MSR_IA32_MISC_ENABLE_TME;
  }
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, TempMsr.Qword);
  ///
  /// Set the Lock TM interrupt bit so that thermal interrupts are routed to all the cores
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
  TempMsr.Qword |= B_MISC_PWR_MGMT_LOCK_TERM_INT;
  AsmWriteMsr64 (MSR_MISC_PWR_MGMT, TempMsr.Qword);

  ///
  /// Enable Critical Temperature Interrupt
  ///
  ThermInterruptMsr.Uint64 = AsmReadMsr64 (MSR_IA32_THERM_INTERRUPT);
  ThermInterruptMsr.Bits.CriticalTempEnable = 1;
  AsmWriteMsr64 (MSR_IA32_THERM_INTERRUPT, ThermInterruptMsr.Uint64);

  return;
}

/**
  Enables the bi-directional PROCHOT# signal.

  @retval EFI_SUCCESS  PROCHOT# configured successfully
**/
EFI_STATUS
EnableProcHot (
  VOID
  )
{
  MSR_REGISTER PowerCtl;

  ///
  /// Enable PROCHOT# in the CPU MSR if TM is enabled,
  ///  else disable it.
  ///
  PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  if (gCpuNvsAreaConfig->Area->PpmFlags & (PPM_TM)) {
    PowerCtl.Qword &= (UINT64) ~B_MSR_POWER_CTL_DISABLE_VR_THERMAL_ALERT;
    if (gCpuPowerMgmtTestConfig->DisableVrThermalAlert == 1) {
      DEBUG ((DEBUG_INFO, "VR Thermal Alert is disabled\n"));
      PowerCtl.Qword |= B_MSR_POWER_CTL_DISABLE_VR_THERMAL_ALERT;
    }
    ///
    /// Check PROCHOT Lock,skip programming the below as it will lock bits 0, 21, 22
    ///
    if (!(PowerCtl.Qword & B_MSR_POWER_CTL_PROCHOT_LOCK)) {
      PowerCtl.Qword &= (UINT64) ~B_MSR_POWER_CTL_BI_PROCHOT;
      if (gCpuPowerMgmtTestConfig->BiProcHot) {
        PowerCtl.Qword |= B_MSR_POWER_CTL_BI_PROCHOT;
        ///
        /// Initialize PROCHOT# OUT basing on Bi-directional PROCHOT# setting
        /// If Bi-directional PROCHOT# is enabled, PROCHOT# OUT can be disabled selectively
        ///
        PowerCtl.Qword &= (UINT64) ~B_MSR_POWER_CTL_DISABLE_PROCHOT_OUT;
        if (gCpuPowerMgmtTestConfig->DisableProcHotOut == 1) {
          DEBUG ((DEBUG_INFO, "PROCHOT# OUT is disabled\n"));
          PowerCtl.Qword |= B_MSR_POWER_CTL_DISABLE_PROCHOT_OUT;
        }
      }
      PowerCtl.Qword &= (UINT64) ~B_MSR_POWER_CTL_PROCHOT_RESPONSE;
      if (gCpuPowerMgmtTestConfig->ProcHotResponse == 1) {
        DEBUG ((DEBUG_INFO, "PROCHOT# Response is enabled\n"));
        PowerCtl.Qword |= B_MSR_POWER_CTL_PROCHOT_RESPONSE;
      }
    }
    AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);
  }

  return EFI_SUCCESS;
}
