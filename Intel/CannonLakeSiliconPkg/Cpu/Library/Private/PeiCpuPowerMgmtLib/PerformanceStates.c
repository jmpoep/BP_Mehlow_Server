/** @file
  This file contains P States and Turbo Power Management configuration functions for
  processors.

  <b>Acronyms:</b>
     - PPM:  Processor Power Management
     - TM:   Thermal Monitor
     - IST:  Intel(R) Speedstep technology
     - HT:   Hyper-Threading Technology
     - ITBM: Intel(R) Turbo Boost Max Technology 3.0

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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
#include <CpuDataStruct.h>

///
/// Global Variables
///
UINT8 *mHighestPerfPerCore = NULL; ///< Pointer to Buffer to store highest performance of cores.
CONST EFI_PEI_SERVICES   **mPeiServices = NULL; ///< Pointer to EFI PEI Serives.

/**
  Initializes Turbo Ratio limits in the processor.
**/
VOID
InitTurboRatioLimits (
  VOID
  )
{
  MSR_REGISTER                       TurboRatioLimit;
  MSR_REGISTER                       FlexRatioMsr;
  MSR_REGISTER                       Ia32MiscEnableMsr;
  MSR_REGISTER                       PowerCtlMsr;
  MSR_REGISTER                       RingRatioMsr;
  UINT16                             CoreCount;
  UINT8                              OverclockingBins;
  UINT8                              OneCoreRatioLimit;
  UINT8                              TwoCoreRatioLimit;
  UINT8                              ThreeCoreRatioLimit;
  UINT8                              FourCoreRatioLimit;
  UINT8                              FiveCoreRatioLimit;
  UINT8                              SixCoreRatioLimit;
  UINT8                              SevenCoreRatioLimit;
  UINT8                              EightCoreRatioLimit;
  BOOLEAN                            ValidRatioLimits;
  CPUID_THERMAL_POWER_MANAGEMENT_EAX Eax;

  ///
  /// Test for Turbo Mode supported and initialize if true.
  ///
  AsmCpuid (CPUID_THERMAL_POWER_MANAGEMENT, &Eax.Uint32, NULL, NULL, NULL);
  Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  if (gCpuNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
    ///
    /// Clear Turbo Mode disable bit in IA32 Misc Enable MSR
    ///
    Ia32MiscEnableMsr.Qword &= ~B_MSR_IA32_MISC_DISABLE_TURBO;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  } else if (((Ia32MiscEnableMsr.Qword & B_MSR_IA32_MISC_DISABLE_TURBO) == 0) &&
             (Eax.Bits.TurboBoostTechnology == 1)) {
    ///
    /// If Turbo mode is supported but required to be disabled (by Policy setting)
    /// Set Turbo Mode disable bit in IA32 Misc Enable MSR since it might be temporarily enabled earlier.
    ///
    Ia32MiscEnableMsr.Qword |= B_MSR_IA32_MISC_DISABLE_TURBO;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  }

  ///
  /// Check if processor turbo-ratio can be overriden
  ///
  if (!mRatioLimitProgrammble) {
    DEBUG ((DEBUG_WARN, "Turbo Ratio Limit is NOT programmable. Platform Info MSR (0xCE) [28] is not set \n"));
    return;
  }
  ///
  /// Read the overclocking bins
  ///
  FlexRatioMsr.Qword  = AsmReadMsr64 (MSR_FLEX_RATIO);
  OverclockingBins    = (UINT8) RShiftU64 ((FlexRatioMsr.Dwords.Low & B_OVERCLOCKING_BINS), 17);
  if (FlexRatioMsr.Dwords.Low & B_OVERCLOCKING_LOCK) { ///<  Check for Overclocking Lock bit
    DEBUG ((DEBUG_INFO, "OverClocking Lock Bit is set. Disable the Lock and reset the system\n"));
    return;
  }
  TurboRatioLimit.Qword = AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT);
  OneCoreRatioLimit     = (UINT8) (TurboRatioLimit.Qword & B_MSR_TURBO_RATIO_LIMIT_1C);
  TwoCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_2C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  ThreeCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_3C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  FourCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_4C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  FiveCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_5C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  SixCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_6C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  SevenCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_7C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  EightCoreRatioLimit = (UINT8) (RShiftU64
                                (TurboRatioLimit.Qword, N_MSR_TURBO_RATIO_LIMIT_8C)
                                & B_MSR_TURBO_RATIO_LIMIT_1C
                                );
  ///
  /// For Overclocking and locked parts, verify ratio overide is within the allowable limits
  /// Locked parts will have OverclockingBins value as 0 so the below condition will take care of locked parts also
  ///
  if (OverclockingBins < MAX_OVERCLOCKING_BINS) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit > (OneCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->OneCoreRatioLimit = OneCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->TwoCoreRatioLimit > (TwoCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->TwoCoreRatioLimit = TwoCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->ThreeCoreRatioLimit > (ThreeCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->ThreeCoreRatioLimit = ThreeCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->FourCoreRatioLimit > (FourCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->FourCoreRatioLimit = FourCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->FiveCoreRatioLimit > (FiveCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->FiveCoreRatioLimit = FiveCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->SixCoreRatioLimit > (SixCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->SixCoreRatioLimit = SixCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->SevenCoreRatioLimit > (SevenCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->SevenCoreRatioLimit = SevenCoreRatioLimit + OverclockingBins;
    }
    if (gCpuPowerMgmtBasicConfig->EightCoreRatioLimit > (EightCoreRatioLimit + OverclockingBins)) {
      gCpuPowerMgmtBasicConfig->EightCoreRatioLimit = EightCoreRatioLimit + OverclockingBins;
    }
  }
  ///
  /// Max Turbo ratio or P0 = Fused 1C Turbo Ratio Limit + No of over clocking Bins.
  ///
  mTurboBusRatio = OneCoreRatioLimit + OverclockingBins;
  ///
  /// Initialize turbo ratio limit MSR.
  /// Find the number of active cores and initialize the ratio limits only if they are available.
  ///
  GetEnabledCount (NULL, &CoreCount, NULL, NULL);

  ValidRatioLimits = TRUE;

  if (CoreCount >= 2) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->TwoCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->TwoCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 3) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->ThreeCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->ThreeCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 4) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->FourCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->FourCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 5) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->FiveCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->FiveCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 6) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->SixCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->SixCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 7) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->SevenCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->SevenCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (CoreCount >= 8) {
    if (gCpuPowerMgmtBasicConfig->OneCoreRatioLimit < gCpuPowerMgmtBasicConfig->EightCoreRatioLimit ||
        gCpuPowerMgmtBasicConfig->EightCoreRatioLimit < mMaxBusRatio) {
      ValidRatioLimits = FALSE;
    }
  }
  if (ValidRatioLimits == TRUE) {
    if (CoreCount >= 1) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_1C;
      TurboRatioLimit.Dwords.Low |= (UINT32) gCpuPowerMgmtBasicConfig->OneCoreRatioLimit;
    }
    if (CoreCount >= 2) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_2C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gCpuPowerMgmtBasicConfig->TwoCoreRatioLimit, 8);
    }
    if (CoreCount >= 3) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_3C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gCpuPowerMgmtBasicConfig->ThreeCoreRatioLimit, 16);
    }
    if (CoreCount >= 4) {
      TurboRatioLimit.Dwords.Low &= ~B_MSR_TURBO_RATIO_LIMIT_4C;
      TurboRatioLimit.Dwords.Low |= LShiftU64 (gCpuPowerMgmtBasicConfig->FourCoreRatioLimit, 24);
    }
    if (CoreCount >= 5) {
      TurboRatioLimit.Dwords.High &= ~B_MSR_TURBO_RATIO_LIMIT_1C;
      TurboRatioLimit.Dwords.High |= (UINT32) gCpuPowerMgmtBasicConfig->FiveCoreRatioLimit;
    }
    if (CoreCount >= 6) {
      TurboRatioLimit.Dwords.High &= ~B_MSR_TURBO_RATIO_LIMIT_2C;
      TurboRatioLimit.Dwords.High |= LShiftU64 (gCpuPowerMgmtBasicConfig->SixCoreRatioLimit, 8);
    }
    if (CoreCount >= 7) {
      TurboRatioLimit.Dwords.High &= ~B_MSR_TURBO_RATIO_LIMIT_3C;
      TurboRatioLimit.Dwords.High |= LShiftU64 (gCpuPowerMgmtBasicConfig->SevenCoreRatioLimit, 16);
    }
    if (CoreCount >= 8) {
      TurboRatioLimit.Dwords.High &= ~B_MSR_TURBO_RATIO_LIMIT_4C;
      TurboRatioLimit.Dwords.High |= LShiftU64 (gCpuPowerMgmtBasicConfig->EightCoreRatioLimit, 24);
    }
    AsmWriteMsr64 (MSR_TURBO_RATIO_LIMIT, TurboRatioLimit.Qword);
  }
  ///
  /// For fully unlocked CPU's, configure Turbo Ratio as 0xFF (max possible P-State)
  ///
  if (OverclockingBins == MAX_OVERCLOCKING_BINS) {
    mTurboBusRatio  = 0xFF;
  }

  ///
  /// Configure Energy Efficient Turbo Disable: POWER_CTL[19]
  /// This is a disable bit, so set 1 to disable EE Turbo
  ///
  PowerCtlMsr.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  PowerCtlMsr.Dwords.Low &= ~B_ENERGY_EFFICIENT_TURBO_FEATURE_DISABLE;
  if (gCpuPowerMgmtTestConfig->EnergyEfficientTurbo == DISABLED) {
    PowerCtlMsr.Dwords.Low |= B_ENERGY_EFFICIENT_TURBO_FEATURE_DISABLE;
  }
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtlMsr.Qword);

  ///
  /// If  a non-default ring ratio is specified, we need to update the ring ratio limit MSR
  ///
  if ((gCpuPowerMgmtBasicConfig->MinRingRatioLimit != 0) || (gCpuPowerMgmtBasicConfig->MaxRingRatioLimit != 0)) {
    RingRatioMsr.Qword = AsmReadMsr64 (MSR_RING_RATIO_LIMIT);
    if (gCpuPowerMgmtBasicConfig->MaxRingRatioLimit != 0) {
      RingRatioMsr.Bytes.FirstByte &= ~V_MSR_RING_RATIO_LIMIT_MASK;
      RingRatioMsr.Bytes.FirstByte |= gCpuPowerMgmtBasicConfig->MaxRingRatioLimit & V_MSR_RING_RATIO_LIMIT_MASK;
    }
    if (gCpuPowerMgmtBasicConfig->MinRingRatioLimit != 0) {
      RingRatioMsr.Bytes.SecondByte &= ~V_MSR_RING_RATIO_LIMIT_MASK;
      RingRatioMsr.Bytes.SecondByte |= gCpuPowerMgmtBasicConfig->MinRingRatioLimit & V_MSR_RING_RATIO_LIMIT_MASK;
    }
    DEBUG ((DEBUG_INFO, "PPM: Programming Ring Ratio Limits: Min = 0x%X, Max = 0x%X\n",gCpuPowerMgmtBasicConfig->MinRingRatioLimit, gCpuPowerMgmtBasicConfig->MaxRingRatioLimit ));
    AsmWriteMsr64 (MSR_RING_RATIO_LIMIT, RingRatioMsr.Qword);
  }

  return;
}

/**
  Initializes Energy efficient P-state feature.
**/
VOID
InitEnergyEfficientPState (
  VOID
  )
{
  MSR_REGISTER       PowerCtlMsr;

  ///
  /// Configure Energy Efficient P-state : POWER_CTL[18]
  ///
  PowerCtlMsr.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  PowerCtlMsr.Dwords.Low &= ~B_ENERGY_EFFICIENT_P_STATE_FEATURE_ENABLE;
  if (gCpuNvsAreaConfig->Area->PpmFlags & PPM_EEPST) {
    PowerCtlMsr.Dwords.Low |= B_ENERGY_EFFICIENT_P_STATE_FEATURE_ENABLE;
  }
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtlMsr.Qword);

  return;
}

/**
  Initializes Race To Halt feature.
**/
VOID
InitRaceToHalt (
  VOID
  )
{
  MSR_REGISTER       PowerCtlMsr;
  ///
  /// Configure Race To Halt feature : POWER_CTL[20]
  ///
  PowerCtlMsr.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  PowerCtlMsr.Dwords.Low &= ~B_MSR_POWER_CTL_RACE_TO_HALT_DISABLE;
  if (gCpuPowerMgmtTestConfig->RaceToHalt == DISABLED) {
    PowerCtlMsr.Dwords.Low |= B_MSR_POWER_CTL_RACE_TO_HALT_DISABLE;
  }
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtlMsr.Qword);

  return;
}

/**
  Create default FVID table with max and min states only.

  @param[in out] FvidPointer  Pointer to a table to be updated
**/
VOID
CreateDefaultFvidTable (
  IN OUT FVID_TABLE *FvidPointer
  )
{
  UINT64 wPower1;
  UINT64 wPower2;

  ///
  /// Fill in the FVid table header.
  ///
  FvidPointer[0].FvidHeader.Stepping    = gCpuNvsAreaConfig->Area->Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = mMaxBusRatio;
  FvidPointer[0].FvidHeader.EistStates  = 2;
  ///
  /// First entry is state 0, highest state.
  ///
  FvidPointer[1].FvidState.State    = 0;
  FvidPointer[1].FvidState.BusRatio = mMaxBusRatio;
  ///
  /// Power is calculated in milliwatts
  ///
  FvidPointer[1].FvidState.Power = (mPackageTdpWatt * 1000);
  ///
  /// Second entry is state 1, lowest state.
  ///
  FvidPointer[2].FvidState.State    = 1;
  FvidPointer[2].FvidState.BusRatio = (UINT16) mMinBusRatio;
  ///
  /// Calculate Relative Power
  ///
  wPower1 = (mMaxBusRatio - FvidPointer[2].FvidState.BusRatio) * 625;
  wPower1 = (110000 - wPower1);
  wPower1 = DivU64x32 (wPower1, 11);
  wPower1 = DivU64x32 (MultU64x64 (wPower1, wPower1), 1000);
  //
  // Power is calculated in milliwatts
  //
  wPower2 = (((FvidPointer[2].FvidState.BusRatio * 100000) / mMaxBusRatio) / 100);
  wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, DivU64x32 (wPower1, 100)), mPackageTdpWatt), 1000);
  FvidPointer[2].FvidState.Power = (UINT16) wPower2;
}

/**
  Calculate the ratio for the requested p state

  @param[in] MaxRatio           Maximum Supported Ratio (HFM)
  @param[in] MinRatio           Minimum Supported Ratio (LFM)
  @param[in] MaxNumberOfStates  Number of entries in the table pointed to by FvidPointer
  @param[in] PStateNumber       Desired P State from range 0..MaxNumberOfStates

  @retval Ratio for the requested Pstate
**/
UINT16
ComputePstateRatio (
  IN UINT16 MaxRatio,
  IN UINT16 MinRatio,
  IN UINT16 MaxNumberOfStates,
  IN UINT16 PStateNumber
  )
{
  UINT16 RatioRange;
  UINT16 NumGaps;
  UINT16 PStateRatio;

  RatioRange  = MaxRatio - MinRatio;
  NumGaps     = MaxNumberOfStates - 1;
  PStateRatio = MaxRatio - (((PStateNumber * RatioRange) + (NumGaps / 2)) / NumGaps);

  return PStateRatio;
}

/**
  Create FVID table

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in]     MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer

  @retval EFI_SUCCESS            FVID table created successfully.
  @retval EFI_INVALID_PARAMETER  The bus ratio range don't permit FVID table calculation;
                                 a default FVID table should be constructed.
**/
EFI_STATUS
CreateFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates
  )
{
  UINT16 BusRatioRange;
  UINT16 NumberOfStates;
  UINT16 NumberOfNonTurboStates;
  UINT16 Turbo;
  UINT16 index;
  UINT64 wPower1;
  UINT64 wPower2;

  ///
  /// Determine the bus ratio range, P1 to Pn. The +1 is required
  /// to ensure Pn entry is counted.
  ///
  BusRatioRange = mMaxBusRatio - mMinBusRatio + 1;
  if (((INT16) BusRatioRange < 1) || (MaxNumberOfStates == 0)) {
    return EFI_INVALID_PARAMETER;
  }
  ///
  /// Determine the HFM state index
  ///
  Turbo = ((gCpuNvsAreaConfig->Area->PpmFlags & PPM_TURBO) ? 1 : 0);
  ///
  ///  Determine the number of states as cpu supported range or Maximum _PSS limit
  ///
  NumberOfStates = ((BusRatioRange + Turbo) < MaxNumberOfStates ? (BusRatioRange + Turbo) : MaxNumberOfStates);
  ///
  /// Determine whether there are more than 16 P-states
  ///
  mOver16Pstates = (NumberOfStates > 16 ? TRUE : FALSE);
  ///
  /// Ensure we have at least two states
  ///
  if (NumberOfStates < 2) {
    ///
    /// In case HFM = LFM and no Turbo, at least have two states with same ratio values.
    ///
    NumberOfStates = 2;
  }
  ///
  /// Number of allowed P-states between P1 to Pn.
  ///
  NumberOfNonTurboStates = NumberOfStates - Turbo;
  ///
  /// Fill in the table header
  ///
  FvidPointer[0].FvidHeader.Stepping    = gCpuNvsAreaConfig->Area->Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = (Turbo ? mTurboBusRatio : mMaxBusRatio);
  FvidPointer[0].FvidHeader.EistStates  = (UINT16) NumberOfStates;
  ///
  /// Add Turbo as P0 if Turbo Mode supported and initialize.
  ///
  if (gCpuNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
    FvidPointer[1].FvidState.BusRatio = mTurboBusRatio;
    FvidPointer[1].FvidState.Power = (mPackageTdpWatt * 1000); // power is calculated in milliwatts
    if (mOver16Pstates) {
      FvidPointer[1].FvidState.Limit16BusRatio = mTurboBusRatio;
      FvidPointer[1].FvidState.Limit16Power = (mPackageTdpWatt * 1000);
    }
  }
  ///
  /// Add HFM as P1 based on Max Turbo availablity
  ///
  FvidPointer[1 + Turbo].FvidState.State    = Turbo;
  FvidPointer[1 + Turbo].FvidState.BusRatio = mMaxBusRatio;
  //
  // Power is calculated in milliwatts
  //
  FvidPointer[1 + Turbo].FvidState.Power = (mPackageTdpWatt * 1000);
  if (mOver16Pstates) {
    FvidPointer[1 + Turbo].FvidState.Limit16State    = Turbo;
    FvidPointer[1 + Turbo].FvidState.Limit16BusRatio = mMaxBusRatio;
    FvidPointer[1 + Turbo].FvidState.Limit16Power    = (mPackageTdpWatt * 1000);
  }

  ///
  /// Fill in the table starting at the last entry
  /// The algorithm is available in the processor BIOS writer's guide.
  ///
  for (index = 1; index < NumberOfNonTurboStates; index++) {
    FvidPointer[index + 1 + Turbo].FvidState.State    = index + Turbo;
    FvidPointer[index + 1 + Turbo].FvidState.BusRatio = ComputePstateRatio (mMaxBusRatio, mMinBusRatio, NumberOfNonTurboStates, index);
    ///
    /// Calculate Relative Power
    ///
    wPower1 = (mMaxBusRatio - FvidPointer[index + 1 + Turbo].FvidState.BusRatio) * 625;
    wPower1 = (110000 - wPower1);
    wPower1 = DivU64x32 (wPower1, 11);
    wPower1 = MultU64x64 (wPower1, wPower1);
    //
    // Power is calculated in milliwatts
    //
    wPower2 = (((FvidPointer[index + 1 + Turbo].FvidState.BusRatio * 100) / mMaxBusRatio));
    wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, wPower1), mPackageTdpWatt), 10000000);
    FvidPointer[index + 1 + Turbo].FvidState.Power = (UINT32) wPower2;
  }

  if (mOver16Pstates) {
    for (index = 1; index < (LPSS_FVID_MAX_STATES - Turbo); index++) {
      FvidPointer[index + 1 + Turbo].FvidState.Limit16State    = index + Turbo;
      FvidPointer[index + 1 + Turbo].FvidState.Limit16BusRatio = ComputePstateRatio (mMaxBusRatio, mMinBusRatio, (LPSS_FVID_MAX_STATES - Turbo), index);
      ///
      /// Calculate Relative Power
      ///
      wPower1 = (mMaxBusRatio - FvidPointer[index + 1 + Turbo].FvidState.Limit16BusRatio) * 625;
      wPower1 = (110000 - wPower1);
      wPower1 = DivU64x32 (wPower1, 11);
      wPower1 = MultU64x64 (wPower1, wPower1);
      //
      // Power is calculated in milliwatts
      //
      wPower2 = (((FvidPointer[index + 1 + Turbo].FvidState.Limit16BusRatio * 100) / mMaxBusRatio));
      wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, wPower1), mPackageTdpWatt), 10000000);
      FvidPointer[index + 1 + Turbo].FvidState.Limit16Power = (UINT32) wPower2;
    }
  }
  return EFI_SUCCESS;
}

/**
  Create a custom FVID table based on setup options.

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in]     MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
**/
VOID
CreateCustomFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates
  )
{
  UINT16  Index;
  BOOLEAN UseStateRatioMax16;

  if (gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries > MaxNumberOfStates) {
    DEBUG (
            (DEBUG_WARN,
             "VidNumber(%d) is greater than maximum(%d) supported.",
             gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries,
             MaxNumberOfStates)
            );
    gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries = (UINT8) MaxNumberOfStates;
  }

  ///
  /// Determine whether there are more than 16 P-states
  ///
  mOver16Pstates = (gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries > 16 ? TRUE : FALSE);

  ///
  /// Fill in the table header
  ///
  FvidPointer[0].FvidHeader.Stepping    = gCpuPowerMgmtCustomConfig->CustomRatioTable.Cpuid;
  FvidPointer[0].FvidHeader.MaxBusRatio = (UINT16) gCpuPowerMgmtCustomConfig->CustomRatioTable.MaxRatio;
  FvidPointer[0].FvidHeader.EistStates  = gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries;

  UseStateRatioMax16 = TRUE;
  if (gCpuPowerMgmtCustomConfig->CustomRatioTable.StateRatioMax16[0] == 0) {
    UseStateRatioMax16 = FALSE;
  }

  ///
  /// Fill in the state data
  ///
  for (Index = 0; Index < gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries; Index++) {
    FvidPointer[Index + 1].FvidState.State    = Index;
    FvidPointer[Index + 1].FvidState.BusRatio = (UINT16) gCpuPowerMgmtCustomConfig->CustomRatioTable.StateRatio[Index];
    if (mOver16Pstates && (Index < LPSS_FVID_MAX_STATES)) {
      FvidPointer[Index + 1].FvidState.Limit16State    = Index;
      if (UseStateRatioMax16 == TRUE) {
        FvidPointer[Index + 1].FvidState.Limit16BusRatio = (UINT16) gCpuPowerMgmtCustomConfig->CustomRatioTable.StateRatioMax16[Index];
      } else {
        FvidPointer[Index + 1].FvidState.Limit16BusRatio = (UINT16) gCpuPowerMgmtCustomConfig->CustomRatioTable.StateRatio[Index];
      }
    }
  }
}

/**
  This function updates the table provided with the FVID data for the processor.
  If CreateDefaultTable is TRUE, a minimum FVID table will be provided.
  The maximum number of states must be greater then or equal to two.
  The table should be initialized in such a way as for the caller to determine if the
  table was updated successfully.

  @param[in out] FvidPointer         Pointer to a table to be updated
  @param[in]     MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
**/
VOID
InitFvidTable (
  IN OUT FVID_TABLE *FvidPointer,
  IN UINT16         MaxNumberOfStates
  )
{
  EFI_STATUS Status;

  Status = EFI_SUCCESS;

  ///
  /// Create FVID table.
  /// If specified, create a custom FVID table.
  ///
  if (gCpuPowerMgmtCustomConfig->CustomRatioTable.NumberOfEntries >= 2) {
    CreateCustomFvidTable (FvidPointer, MaxNumberOfStates);
  } else {
    Status = CreateFvidTable (FvidPointer, MaxNumberOfStates);
  }

  if (EFI_ERROR (Status)) {
    CreateDefaultFvidTable (FvidPointer);
    gCpuNvsAreaConfig->Area->PpmFlags &= ~PPM_TURBO;
  }

  return;
}

/**
  Enables EIST support in a logical processor.
  This function must be MP safe.
**/
VOID
EFIAPI
ApSafeEnableEist (
  VOID
  )
{
  MSR_REGISTER Ia32MiscEnable;
  MSR_REGISTER MiscPwrMgmt;

  ///
  /// Enable EIST in the CPU MSR.
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_EIST;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  ///
  /// If CMP is disabled, disable hardware coordination.
  ///
  if (!(gCpuNvsAreaConfig->Area->PpmFlags & PPM_CMP)) {
    MiscPwrMgmt.Qword = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
    MiscPwrMgmt.Qword |= B_MISC_PWR_MGMT_SINGLE_PCTL_EN;
    AsmWriteMsr64 (MSR_MISC_PWR_MGMT, MiscPwrMgmt.Qword);
  }

  return;
}

/**
  Enables EIST support in the processor.

  @param[in] PeiServices      Pointer to PEI Services Table
**/
VOID
InitEist (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  ///
  /// Enable EIST on all APs. For BSP, it was already enabled during pre-RESET_CPL initialization.
  ///
  ApSafeEnableEist ();
  gMpServicesPpi->StartupAllAPs (
                    PeiServices,
                    gMpServicesPpi,
                    (EFI_AP_PROCEDURE) ApSafeEnableEist,
                    FALSE,
                    0,
                    NULL
                    );

  return;
}

/**
  Configures the Interrupt Redirection Mode Selection for Logical Interrupts.
**/
VOID
InitPpmIrmConfiguration (
  VOID
  )
{
  UINTN  MchBar;
  UINT32 Data32And;
  UINT32 Data32Or;

  ///
  /// Get the MCH space base address and program MMIO register MCHBAR+0x5418 to enable specific routing algorithm.
  ///
  MchBar    = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;
  Data32And = (UINT32) ~(BIT2 + BIT1 + BIT0);
  Data32Or  = (UINT32) (gCpuPowerMgmtTestConfig->PpmIrmSetting & (BIT2 + BIT1 + BIT0));
  MmioAndThenOr32 (MchBar + 0x5418, Data32And, Data32Or);
}

/**
  Set HWP Interrupt
**/
VOID
EFIAPI
ApSetHwpInterrupt (
  VOID
  )
{
  MSR_REGISTER HwpInterrupt;

  HwpInterrupt.Qword = AsmReadMsr64 (MSR_IA32_HWP_INTERRUPT);

  ///
  /// Set bits 0,1 to enable HWP interrupt support
  ///
  HwpInterrupt.Dwords.Low |= (B_IA32_HWP_CHANGE_TO_GUARANTEED);
  AsmWriteMsr64 (MSR_IA32_HWP_INTERRUPT, HwpInterrupt.Qword);

  return;
}

/**
  Initializes HWP
  Enable HWP and if LVT interrupt supported, enable APIC to generate SMI

  @param[in] PeiServices    Pointer to PEI Services Table
**/
VOID
InitializeHwp (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  MSR_REGISTER MiscPwrMgmt;

  MiscPwrMgmt.Qword  = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
  if (gCpuPowerMgmtBasicConfig->Hwp == 1) {
    ///
    /// Set bit 6 of MSR MISC_PWR_MGMT (1AAh) to enable HWP support
    ///
    if (gCpuNvsAreaConfig->Area->PpmFlags & PPM_HWP) {
      MiscPwrMgmt.Dwords.Low |= B_MISC_PWR_MGMT_ENABLE_HWP;
    }

    ///
    /// Set bit 7 of MSR MISC_PWR_MGMT (1AAh) to enable HWP interrupt
    ///
    if ((gCpuNvsAreaConfig->Area->PpmFlags & PPM_HWP) &&
        (gCpuNvsAreaConfig->Area->PpmFlags & PPM_HWP_LVT)) {

      ///
      /// Set HWP interrupt support on all threads
      ///
      ApSetHwpInterrupt ();
      gMpServicesPpi->StartupAllAPs (
                        PeiServices,
                        gMpServicesPpi,
                        (EFI_AP_PROCEDURE) ApSetHwpInterrupt,
                        FALSE,
                        0,
                        NULL
                        );
    }
  }
    if (MiscPwrMgmt.Dwords.Low & B_MISC_PWR_MGMT_ENABLE_HWP) {
      if (MiscPwrMgmt.Dwords.Low & B_MISC_PWR_MGMT_ENABLE_HWP_INTERRUPT) {
        MiscPwrMgmt.Dwords.Low &= ~B_MISC_PWR_MGMT_ENABLE_HWP_INTERRUPT;

        ///
        /// If HWP Interrupt was enabled in setup, 0x1AA[7] will be set
        ///
        MiscPwrMgmt.Dwords.Low |= ((gCpuPowerMgmtBasicConfig->HwpInterruptControl) << N_MISC_PWR_MGMT_ENABLE_HWP_INTERRUPT) & B_MISC_PWR_MGMT_ENABLE_HWP_INTERRUPT;
      }

    }
  AsmWriteMsr64 (MSR_MISC_PWR_MGMT, MiscPwrMgmt.Qword);
  return;
}


/**
  Get highest performance of all cores.
**/
VOID
EFIAPI
GetHighestPerf (
  VOID                 *Buffer
  )
{
  EFI_STATUS      Status;
  UINT32          LibStatus;
  UINT32          MailboxData;
  UINTN Index = 0;

  gMpServicesPpi->WhoAmI (
                    mPeiServices,
                    gMpServicesPpi,
                    &Index
                    );

  Status = MailboxRead (MAILBOX_TYPE_OC, MAILBOX_READ_PER_CORE_RATIO_LIMITS_CMD, &MailboxData, &LibStatus);
  //
  // The highest performance is recorded on a mailbox read success. Incase of a mailbox read failure,
  // the initalized zero value is retained. In ASL code, we only patch the _CPC table for highest perf
  // if it is a non-zero value.
  //
  if ((Status == EFI_SUCCESS) && (LibStatus == PCODE_MAILBOX_CC_SUCCESS)) {
    //
    // Bits [15:8] Fused Max Core Ratio.
    //
    MailboxData = (MailboxData >> 8) & 0xFF;
    mHighestPerfPerCore[Index] = (UINT8)(MailboxData);
  }
  return;
}

/**
  Initializes Intel Turbo Boost Max Technology 3.0

  @param[in] PeiServices    Pointer to PEI Services Table
**/
VOID
InitializeItbm (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  UINT8 LowestMaxPerf = 0;
  UINTN NumberOfProcessors;
  UINTN NumberOfEnabledProcessors;
  UINTN Index;
  EFI_STATUS Status;

  DEBUG((DEBUG_INFO, "Itbm: InitializeItbm() \n"));

  mPeiServices = PeiServices;

  if ((gCpuNvsAreaConfig->Area->PpmFlags & PPM_TURBO_BOOST_MAX)
       && (gCpuPowerMgmtBasicConfig->EnableItbm == 0)) {
    ///
    /// Intel Turbo Boost Max Technology 3.0 is available and reference code policy is disabled
    /// Here the _CPC object will report the highest performance of the slowest core.
    ///
    DEBUG((DEBUG_INFO, "Itbm: Itbm support is present in Silicon, but reference code policy is disabled\n"));
    Status = gMpServicesPpi->GetNumberOfProcessors (
                               PeiServices,
                               gMpServicesPpi,
                               &NumberOfProcessors,
                               &NumberOfEnabledProcessors
                               );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "Itbm: Num of Processors = 0x%x \n", NumberOfProcessors));

    mHighestPerfPerCore = AllocateZeroPool (NumberOfProcessors * sizeof (UINT8));
    if (mHighestPerfPerCore == NULL) {
      ASSERT (mHighestPerfPerCore != NULL);
      return;
    }

    GetHighestPerf (NULL); ///< Run on BSP.
    gMpServicesPpi->StartupAllAPs (
                      PeiServices,
                      gMpServicesPpi,
                      GetHighestPerf,
                      FALSE,
                      0,
                      NULL
                      );

    LowestMaxPerf = mHighestPerfPerCore [0];
    for (Index = 1; Index < NumberOfProcessors; Index++) {
      if (LowestMaxPerf > mHighestPerfPerCore [Index]) {
        LowestMaxPerf = mHighestPerfPerCore [Index];
      }
    }
    DEBUG((DEBUG_INFO, "Itbm: Highest performance of slowest core LowestMaxPerf = 0x%x \n", LowestMaxPerf));
    //
    // Update NVS with Highest Performance of the slowest core.
    //
    gCpuNvsAreaConfig->Area->LowestMaxPerf = LowestMaxPerf;

    FreePool (mHighestPerfPerCore);
  }
  return;
}

/**
  Initializes P States and Turbo Power management features

  @param[in] PeiServices    Pointer to PEI Services Table
**/
VOID
InitPStates (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  MSR_REGISTER          Ia32MiscEnableMsr;

  //
  // InitTurboRatioLimits has to be called before InitFvidTable as InitFvidTable uses the Turbo Ratio Limit programmed.
  //
  InitTurboRatioLimits (); ///< Initialize InitTurboRatioLimits

  InitEnergyEfficientPState (); ///< Initialize Energy Efficient P-state

  InitRaceToHalt (); ///< Initialize Race To Halt feature

  //
  // Initialize P states
  //
  if (gCpuNvsAreaConfig->Area->PpmFlags & PPM_EIST) {
    ///
    /// Initialize the FVID tables.
    ///
    InitFvidTable (mFvidPointer, TPSS_FVID_MAX_STATES);
    ASSERT (mFvidPointer->FvidHeader.EistStates != 0);

    if (mMaxBusRatio != mMinBusRatio) {
      InitEist ((CONST EFI_PEI_SERVICES **) PeiServices);
    } else {
      DEBUG ((DEBUG_WARN, "CPU MaxBusRatio is equal to MinBusRatio, InitEist() is skipped!\n"));
    }
  } else {
    ///
    /// Clear EIST bit in IA32 Misc Enable MSR that was initially set earilier in PEI
    ///
    Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
    Ia32MiscEnableMsr.Qword &= (UINT64) ~B_MSR_IA32_MISC_ENABLE_EIST;

    ///
    /// Disable Turbo if EIST and HWP is disabled
    ///
    if (!(gCpuNvsAreaConfig->Area->PpmFlags & PPM_HWP)
        && gCpuNvsAreaConfig->Area->PpmFlags & PPM_TURBO) {
      Ia32MiscEnableMsr.Qword |= B_MSR_IA32_MISC_DISABLE_TURBO;
    }
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnableMsr.Qword);
  }

  ///
  /// Initialize Power Aware Interrupt Routing (PAIR) Configuration
  ///
  InitPpmIrmConfiguration ();
}
