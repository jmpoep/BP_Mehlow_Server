/** @file
  Power Management EarlyPost initializations.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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

#include <Ppi/SiPolicy.h>
#include <Library/CpuPlatformLib.h>
#include <Library/CpuMailboxLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Private/Library/CpuOcLib.h>
#include <Private/Library/BiosGuardInit.h>
#include <Library/ConfigBlockLib.h>
#include "Features.h"
#include <Library/PostCodeLib.h>
#include <Library/PciSegmentLib.h>

///
/// Table to convert MilliSeconds into equivalent MSR values
/// This table is used for VR TDC Time Window programming
/// 9ms does not have a unique encoding, so use 10ms encoding.
///
GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mMilliSecondsToMsrValueTable[][2] = {
  ///
  ///  MilliSeconds   MSR Value
  ///
  {        0,         0x00     },
  {        1,         0x00     },
  {        2,         0x01     },
  {        3,         0x41     },
  {        4,         0x02     },
  {        5,         0x22     },
  {        6,         0x42     },
  {        7,         0x62     },
  {        8,         0x03     },
  {        9,         0x23     },
  {       10,         0x23     }
};

GLOBAL_REMOVE_IF_UNREFERENCED CPU_VR_OVERRIDE_TABLE mCpuVrOverrideTable[]={
///
/// Cpu Identifier          IaIccMax GtIccMax SaIccMax VccIcc IaTdc GtTdc SaTdc VccTdc IaAcLL IaDcLL GtAcLL GtDcLL SaAcLL SaDcLL VccAc VccDc VccIn
///                         1/4A     1/4A     1/4A     1/4A   1/8A  1/8A  1/8A  1/8A   1/100 mOhm                                            Limit
  { EnumCflS35Watt22CpuId,  160,     140,      44,       0,    232,  200,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS54Watt22CpuId,  232,     180,      44,       0,    248,  240,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS58Watt22CpuId,  316,     180,      44,       0,    312,  240,    0,    0,    210,   210,   310,   310,     0,     0,    0,    0,    0},
  { EnumCflS35Watt42CpuId,  264,     140,      44,       0,    360,  200,    0,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS62Watt42CpuId,  316,     180,      44,       0,    496,  240,    0,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS65Watt42CpuId,  316,     180,      44,       0,    496,  240,    0,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS95Watt42CpuId,  400,     180,      44,       0,    560,  240,    0,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS35Watt62CpuId,  416,     140,      44,       0,    592,  200,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS65Watt62CpuId,  532,     180,      44,       0,    728,  240,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS95Watt62CpuId,  552,     180,      44,       0,    800,  240,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS35Watt82CpuId,  416,     140,      44,       0,    592,  200,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS65Watt82CpuId,  744,     180,      44,       0,   1168,  240,   80,    0,    160,   160,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS95Watt82CpuId,  772,     180,      44,       0,   1200,  240,   80,    0,    160,   160,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflU28Watt23eCpuId, 256,     256,      34,       0,    360,  304,   32,    0,    240,   240,   200,   200,  1030,  1030,    0,    0,    0},
  { EnumCflS71Watt42CpuId,  400,     180,      44,       0,    544,  200,    0,    0,    210,   210,   310,   310,  1000,  1000,    0,    0,    0},
  { EnumCflS83Watt42CpuId,  400,     180,      44,       0,    560,  240,    0,    0,    210,   210,   310,   310,  1000,  1000,    0,    0,    0},
  { EnumCflS80Watt62CpuId,  532,     180,      44,       0,    712,  240,   80,    0,    210,   210,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflS80Watt82CpuId,  772,     180,      44,       0,   1200,  240,   80,    0,    160,   160,   310,   310,  1000,     0,    0,    0,    0},
  { EnumCflU28Watt43eCpuId, 256,     256,      34,       0,    360,  304,    0,    0,    240,   240,   200,   200,  1030,  1030,    0,    0,    0},
  { EnumCflUR15Watt43eCpuId,248,     208,      34,       0,    280,  240,   32,    0,    240,   240,   200,   200,  1030,  1030,    0,    0,    0},
  { EnumCflUR28Watt43eCpuId,256,     256,      34,       0,    360,  304,   32,    0,    240,   240,   200,   200,  1030,  1030,    0,    0,    0},
  { EnumWhlU15Watt42fCpuId, 280,     124,      24,       0,    384,  144,   32,    0,    180,   180,   310,   310,  1030,  1030,    0,    0,    0},
  { EnumWhlU15Watt22fCpuId, 140,     124,      24,       0,    192,  144,    0,    0,    240,   240,   310,   310,  1030,  1030,    0,    0,    0},
  { EnumWhlU15Watt2f1fCpuId,140,     124,      24,       0,    192,  144,    0,    0,    240,   240,   310,   310,  1030,  1030,    0,    0,    0},
  { EnumCnlU15Watt22CpuId,  128,     148,      64,       0,    176,  232,    0,    0,    280,   280,   300,   300,   960,   960,    0,    0,    0},
  { EnumCnlU15Watt20CpuId,  128,       0,      64,       0,      0,    0,    0,    0,    280,   280,     0,     0,   960,   960,    0,    0,    0},
  { EnumCnlY9Watt22CpuId,     0,       0,       0,     132,      0,    0,    0,  160,      0,      0,    0,     0,     0,     0,  650,  200, 1890},
  { EnumCflH45Watt4f2CpuId, 344,     128,      44,       0,    480,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCflH45Watt62CpuId,  512,     128,      44,       0,    640,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCflH45Watt82CpuId,  560,     128,      44,       0,    688,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCflH65Watt82CpuId,  660,     128,      44,       0,    936,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCflH65Watt62CpuId,  512,     128,      44,       0,    728,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCflH65Watt42CpuId,  364,     128,      44,       0,    480,  200,    0,     0,   180,   180,   270,   270,  1030,  1030,    0,    0,    0},
  { EnumCnlH45Watt82CpuId,    0,       0,       0,     600,      0,    0,    0,   760,     0,      0,    0,     0,     0,     0,  150,  150, 1900},
  { EnumCnlH65Watt82CpuId,    0,       0,       0,     660,      0,    0,    0,   848,     0,      0,    0,     0,     0,     0,  150,  150, 1900},
  { EnumCnlH95Watt82CpuId,    0,       0,       0,     768,      0,    0,    0,  1056,     0,      0,    0,     0,     0,     0,  140,  140, 2000}
};

/**
  Based on ResetType, perform warm or cold reset using PCH reset PPI.

  @param[in] ResetType    - CPU_RESET_TYPE to indicate which reset should be performed.

  @retval EFI_SUCCESS     - Function successful (system should already reset).
  @retval EFI_UNSUPPORTED - Reset type unsupported.
**/
EFI_STATUS
PerformWarmOrColdReset (
  IN CPU_RESET_TYPE ResetType
  )
{
  EFI_STATUS                 Status;

  Status = EFI_SUCCESS;

  ///
  /// Perform the requested reset using EFI RESET2 PeiService.
  ///
  switch (ResetType) {
    case COLD_RESET:
     (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetCold, EFI_SUCCESS, 0, NULL);
      break;

    case WARM_RESET:
     (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetWarm, EFI_SUCCESS, 0, NULL);
      break;

    default:
      DEBUG ((DEBUG_ERROR, "CpuInitPreResetCpl: PerformWarmOrColdReset - ResetType %d not supported: \n", ResetType));
      Status = EFI_UNSUPPORTED;
      ASSERT_EFI_ERROR (Status);
  }

  return Status;
}

/**
  Program Tcc Activation Offset and  Running Average Temperature Limit (RATL).

  @param[in] SiPolicyPpi    - The SI Policy PPI instance
**/
VOID
InitRatl (
  IN SI_POLICY_PPI *SiPolicyPpi
  )
{
  MSR_REGISTER                TempMsr;
  BOOLEAN                     IsTccActivationOffsetProgrammable;
  CPU_POWER_MGMT_BASIC_CONFIG *CpuPowerMgmtBasicConfig;
  BOOLEAN                     IsTimeWindowInSeconds;
  UINT8                       ConvertedTccOffsetTime;
  EFI_STATUS                  Status;

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtBasicConfigGuid, (VOID *) &CpuPowerMgmtBasicConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Read Temperature Target MSR
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_TEMPERATURE_TARGET);

  ///
  /// Check Tcc Activation Offset Programmable Setting from Platform Info MSR Bits[30]
  ///
  IsTccActivationOffsetProgrammable = (AsmReadMsr64 (MSR_PLATFORM_INFO) & B_PLATFORM_INFO_PROG_TCC_ACTIVATION_OFFSET) != 0;

  ///
  /// If TccActivationOffset is greater than max supported value, set it to max value.
  ///
  if (CpuPowerMgmtBasicConfig->TccActivationOffset > V_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_OFFSET_MASK) {
      CpuPowerMgmtBasicConfig->TccActivationOffset = V_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_OFFSET_MASK;
  }

  ///
  /// First check if TCC Activation Offset is programmable based on PLATFORM INFO MSR [30]
  /// If TCC Activation Offset is programable, program the TCC Activation offset value
  /// from Policy, and the Tcc activation offset programming should be dependent on RESET_CPL done.
  ///
  if (IsTccActivationOffsetProgrammable) {
    TempMsr.Dwords.Low &= ~(UINT32) LShiftU64 (V_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_OFFSET_MASK, N_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LIMIT);
    CpuPowerMgmtBasicConfig->TccActivationOffset &= V_MSR_TEMPERATURE_TARGET_TCC_ACTIVATION_OFFSET_MASK;
    TempMsr.Dwords.Low |= (UINT32) LShiftU64 (CpuPowerMgmtBasicConfig->TccActivationOffset, N_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LIMIT);
  }

  ///
  /// Running Average Temperature Limit (RATL) permits advanced thermal management with bursts of excursion above Tjmax
  /// while maintaining the overall average temperature.
  ///
  IsTimeWindowInSeconds = (CpuPowerMgmtBasicConfig->TccOffsetTimeWindowForRatl / 1000) >= 1;
  if (IsTimeWindowInSeconds) {
    ConvertedTccOffsetTime = GetConvertedTime ((CpuPowerMgmtBasicConfig->TccOffsetTimeWindowForRatl / 1000), SecondsTimeWindowConvert);
  } else {
    ConvertedTccOffsetTime = GetConvertedTime (CpuPowerMgmtBasicConfig->TccOffsetTimeWindowForRatl, MilliSecondsTimeWindowConvert);
  }
  TempMsr.Bytes.FirstByte = 0;
  TempMsr.Dwords.Low &= ~B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LOCK;
  if (CpuPowerMgmtBasicConfig->TccOffsetTimeWindowForRatl != 0) {
    ///
    /// Get converted time window based on setting of policy
    ///
    TempMsr.Bytes.FirstByte |= (ConvertedTccOffsetTime & B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_TIME_WINDOW);
  }

  ///
  /// When Clamp enable bit set, allow throttling below P1.
  ///
  TempMsr.Bytes.FirstByte &= ~B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_CLAMP_BIT;
  if (CpuPowerMgmtBasicConfig->TccOffsetClamp) {
    TempMsr.Bytes.FirstByte |= B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_CLAMP_BIT;
  }

  ///
  /// Lock the MSR by setting BIT31 of MSR 1A2h.
  ///
  TempMsr.Dwords.Low &= ~B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LOCK;
  if (CpuPowerMgmtBasicConfig->TccOffsetLock) {
    TempMsr.Dwords.Low |= B_MSR_TEMPERATURE_TARGET_TCC_OFFSET_LOCK;
  }

  ///
  /// Write Temperature Target MSR
  ///
  AsmWriteMsr64 (MSR_TEMPERATURE_TARGET, TempMsr.Qword);

  return;
}

/**
  Programs the McIVR RFI settings based on XTAL clock frequency.

  @param[IN] CpuPowerMgmtVrConfig  - CPU Power Management VR Config Block
**/
VOID
UpdateMcivrRfiSettings (
  IN CPU_POWER_MGMT_VR_CONFIG *CpuPowerMgmtVrConfig
  )
{
  UINTN                       MchBar;
  UINT32                      RfiData;
  UINT8                       OldRfiFreqAdj;
  UINT8                       OldRfiSpread;
  UINT8                       NewRfiFreqAdj;
  UINT8                       NewRfiSpread;
  UINT32                      RegEcx;

  ///
  /// Retrieve current RFI settings
  ///
  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;
  RfiData = MmioRead32 (MchBar + R_RFI_CONTROL_0_0_0_MCHBAR_PCU);
  OldRfiFreqAdj = RfiData & B_RFI_FREQ_ADJ_MASK;
  OldRfiSpread = (RfiData & B_RFI_FREQ_SPREAD_MASK) >> 8;

  AsmCpuid (CPUID_TIME_STAMP_COUNTER, NULL, NULL, &RegEcx, NULL);

  ///
  /// Calculate frequency adjustment based on XTAL clock frequency
  ///
  if (RegEcx == CLOCK_FREQUENCY_19MHz) {
    if ((CpuPowerMgmtVrConfig->McivrRfiFreqAdj) > V_RFI_FREQ_ADJ_MAX) {
      NewRfiFreqAdj = V_RFI_FREQ_ADJ_MAX;
    } else {
      NewRfiFreqAdj = CpuPowerMgmtVrConfig->McivrRfiFreqAdj;
    }
  } else if (RegEcx == CLOCK_FREQUENCY_24MHz) {
    if ((CpuPowerMgmtVrConfig->McivrRfiFreqAdj) > V_RFI_FREQ_ADJ_MAX) {
      NewRfiFreqAdj = V_RFI_FREQ_ADJ_MAX;
    } else {
      NewRfiFreqAdj = CpuPowerMgmtVrConfig->McivrRfiFreqAdj;
    }
  } else {
    DEBUG ((DEBUG_ERROR, "Unexpected Clock Frequency! RegEcx: %d\n", RegEcx));
    return;
  }

  ///
  /// Change New Frequency Adjustment to 2s compliment if prefix is negative
  ///
  if (CpuPowerMgmtVrConfig->McivrRfiFrequencyPrefix == 1) {
    NewRfiFreqAdj = ~(NewRfiFreqAdj) + 1;
  }

  NewRfiSpread = CpuPowerMgmtVrConfig->McivrSpreadSpectrum;
  if (NewRfiSpread > V_MCIVR_SPREAD_SPECTRUM_MAX) {
    NewRfiSpread = V_MCIVR_SPREAD_SPECTRUM_MAX;
  }

  ///
  /// Check if frequency was adjusted
  ///
  if (NewRfiFreqAdj != OldRfiFreqAdj) {
    DEBUG ((DEBUG_INFO, "VR: Updating RFI Freq Adj = %d, RFI Spread = %d\n", NewRfiFreqAdj, NewRfiSpread));
    RfiData = (RfiData & ~B_RFI_FREQ_ADJ_MASK);
    RfiData = (RfiData & ~B_RFI_FREQ_SPREAD_MASK);
    RfiData = RfiData | NewRfiFreqAdj;
    RfiData = RfiData | (NewRfiSpread << 8);
    MmioWrite32 ((MchBar + R_RFI_CONTROL_0_0_0_MCHBAR_PCU), RfiData);

  ///
  /// If frequency doesn't need to be adjusted, check if spread has been adjusted
  ///
  } else if (NewRfiSpread != OldRfiSpread) {
    DEBUG ((DEBUG_INFO, "VR: Updating RFI Spread = %d\n", NewRfiSpread));
    RfiData = (RfiData & ~B_RFI_FREQ_SPREAD_MASK);
    RfiData = RfiData | (NewRfiSpread << 8);
    MmioWrite32 ((MchBar + R_RFI_CONTROL_0_0_0_MCHBAR_PCU), RfiData);
  }
}

/**
  Programs the FIVR RFI settings based on XTAL clock frequency.

  @param[IN] CpuPowerMgmtVrConfig  - CPU Power Management VR Config Block
**/
VOID
UpdateFivrRfiSettings (
  IN CPU_POWER_MGMT_VR_CONFIG *CpuPowerMgmtVrConfig
  )
{
  UINTN                       MchBar;
  UINT32                      RfiData;
  UINT32                      RegEcx;
  UINT16                      RfiValue;
  UINT16                      DoubledValue;
  UINT8                       RfiLowBits;
  UINT8                       RfiHighByte;
  UINT8                       LowOffset;
  UINT8                       HighOffset;
  UINT32                      RfiMask;
  UINT32                      SpreadSpectrumData;
  UINT8                       EncodedSpectrum;

  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;

  if (CpuPowerMgmtVrConfig->FivrSpreadSpectrum != 0) {
    //
    // Sets clock spectrum spread percentage:
    // 0x00=0.2% , 0x3F=10%
    // 1 LSB = 0.1% increase in spread (for settings 0x01 thru 0x1C)
    // 1 LSB = 0.2% increase in spread (for settings 0x1E thru 0x3F)
    // The bit[7:6] are not used.
    //
    EncodedSpectrum = 0;
    if (CpuPowerMgmtVrConfig->FivrSpreadSpectrum > 2) {
      EncodedSpectrum = CpuPowerMgmtVrConfig->FivrSpreadSpectrum - 2;
      if (EncodedSpectrum > V_SPREAD_SPECTRUM_POINT_ONE_ENCODING_MAX) {
        EncodedSpectrum = EncodedSpectrum - V_SPREAD_SPECTRUM_POINT_ONE_ENCODING_MAX;
        EncodedSpectrum = EncodedSpectrum / 2;
        EncodedSpectrum = EncodedSpectrum + V_SPREAD_SPECTRUM_POINT_ONE_ENCODING_MAX;
        if (EncodedSpectrum > V_FIVR_SPREAD_SPECTRUM_MAX) {
          EncodedSpectrum = V_FIVR_SPREAD_SPECTRUM_MAX;
        }
      }
    }

    //
    // Write the Spread Spectrum value.
    //
    SpreadSpectrumData = MmioRead32 (MchBar + R_EMI_CONTROL_0_0_0_MCHBAR_PCU);
    SpreadSpectrumData = (SpreadSpectrumData & ~B_SPREAD_SPECTRUM_MASK);
    SpreadSpectrumData = SpreadSpectrumData | EncodedSpectrum;
    SpreadSpectrumData = SpreadSpectrumData | B_SPREAD_SPECTRUM_ENABLE;
    MmioWrite32 ((MchBar + R_EMI_CONTROL_0_0_0_MCHBAR_PCU), SpreadSpectrumData);

  }

  if (CpuPowerMgmtVrConfig->FivrRfiFrequency != 0) {
    RfiData = MmioRead32 (MchBar + R_RFI_CONTROL_0_0_0_MCHBAR_PCU);

    ///
    /// Determine XTAL clock frequency
    ///
    AsmCpuid(CPUID_TIME_STAMP_COUNTER, NULL, NULL, &RegEcx, NULL);

    ///
    /// RfiValue = Policy->RfiFreq * 100KHz * (128/ClockInKhz).
    ///
    if(RegEcx == CLOCK_FREQUENCY_19MHz) {
      //
      // RfiValue = Config->RfiFreq * 12800 / 19200.
      // RfiValue = Config->RfiFreq * 2 / 3.
      // Round to nearest by doubling before dividing by 3, then add 1 to the result and divide by 2.
      //
      DoubledValue = (CpuPowerMgmtVrConfig->FivrRfiFrequency * 4) / 3;
      RfiValue = (DoubledValue + 1) / 2;
      //
      // Set offsets and mask based on clock.
      //
      LowOffset = N_RFI_FREQ_LO_PF1_OFFSET;
      HighOffset = N_RFI_FREQ_HI_PF1_OFFSET;
      RfiMask = B_RFI_FREQ_PF1_MASK;
    } else if(RegEcx == CLOCK_FREQUENCY_24MHz) {
      //
      // RfiValue = Config->RfiFreq * 12800 / 24000.
      // RfiValue = Config->RfiFreq * 8 / 15.
      // Round to nearest by doubling before dividing by 15, then add 1 to the result and divide by 2.
      //
      DoubledValue = (CpuPowerMgmtVrConfig->FivrRfiFrequency * 16) / 15;
      RfiValue = (DoubledValue + 1) / 2;

      //
      // Set offsets and mask based on clock.
      //
      LowOffset = N_RFI_FREQ_LO_PF2_OFFSET;
      HighOffset = N_RFI_FREQ_HI_PF2_OFFSET;
      RfiMask = B_RFI_FREQ_PF2_MASK;
    } else {
      DEBUG ((DEBUG_ERROR, "Unexpected Clock Frequency! RegEcx: %d\n", RegEcx));
      return;
    }

    if (RfiValue > V_MAX_RFI_VALUE) {
      RfiValue = V_MAX_RFI_VALUE;
    }

    RfiData = (RfiData & ~RfiMask);
    RfiLowBits = (RfiValue & (BIT1 | BIT0));
    RfiHighByte = (UINT8) (RfiValue >> 2);
    RfiData = RfiData | (UINT32) (RfiLowBits << LowOffset);
    RfiData = RfiData | (UINT32) (RfiHighByte << HighOffset);
    MmioWrite32 ((MchBar + R_RFI_CONTROL_0_0_0_MCHBAR_PCU), RfiData);

  }
}

/**
  Program RFI settings for McIVR and FIVR.

  @param[IN] CpuPowerMgmtVrConfig  - CPU Power Management VR Config Block
**/
VOID
UpdateRfiSettings (
  IN CPU_POWER_MGMT_VR_CONFIG *CpuPowerMgmtVrConfig
  )
{
  CPU_SKU       CpuSku;

  CpuSku = GetCpuSku ();
#ifdef CPU_CFL
  CpuSku = EnumCpuUlt;  //KBL has external VRs, same as CNL-U.
#endif
  if (CpuSku == EnumCpuUlx) {
    UpdateMcivrRfiSettings (CpuPowerMgmtVrConfig);
  } else if ((CpuSku == EnumCpuTrad) || (CpuSku == EnumCpuHalo)) {
    UpdateFivrRfiSettings (CpuPowerMgmtVrConfig);
  }
}

/**
  Updates the Vr Config block with Intel default override values if needed

  @param[IN] CpuPowerMgmtVrConfig  - CPU Power Management VR Config Block
**/
VOID
UpdateVrOverrides (
  IN CPU_POWER_MGMT_VR_CONFIG *CpuPowerMgmtVrConfig
  )
{
  EFI_STATUS    Status;
  UINTN         TableIndex;
  UINTN         NoOfOverrides;
  CPU_VR_OVERRIDE_TABLE *VrOverrideTable;
  CPU_OVERRIDE_IDENTIFIER CpuIdentifier;
  SI_PREMEM_POLICY_PPI    *SiPreMemPolicyPpi;
  CPU_OVERCLOCKING_PREMEM_CONFIG *CpuOverClockingConfig;

  VrOverrideTable = mCpuVrOverrideTable;
  NoOfOverrides = 0;
  CpuOverClockingConfig = NULL;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  DEBUG ((DEBUG_INFO, "VR: Locate SiPreMemPolicyPpi Status = %X\n", Status));
  if ((Status == EFI_SUCCESS) && (SiPreMemPolicyPpi != NULL)) {
    Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuOverclockingPreMemConfigGuid, (VOID *) &CpuOverClockingConfig);
    DEBUG ((DEBUG_INFO, "VR: Get OverClockingConfig Status = %X\n", Status));
    if (!EFI_ERROR (Status)) {
      ///
      /// When overclocking is enabled, we don't update the VR settings so that OC systems
      /// rely solely on config block input values.
      ///
      if (CpuOverClockingConfig->OcSupport == 1) {
        DEBUG ((DEBUG_INFO, "VR: Overclocking is enabled. Bypassing VR overrides.\n"));
        return;
      }
    }
  }

  ///
  /// Get CpuIdentifier to identify which set of VR values we need to override
  ///
  CpuIdentifier = GetCpuIdentifier ();
  DEBUG ((DEBUG_INFO, "VR: Cpu Identifier = %X\n", CpuIdentifier));
  if (CpuIdentifier == EnumUnknownCpuId) {
    DEBUG ((DEBUG_ERROR, "VR: Unknown Cpu Identifier, bypassing VR overrides.\n"));
    return;
  }

  ///
  /// If CFL-S/H, update Cpu Identifier based on VrPowerDeliveryDesign input
  ///
  if (GetCpuFamily () == EnumCpuCflDtHalo) {
    if ( CpuPowerMgmtVrConfig->VrPowerDeliveryDesign == EnumUnknownCpuVrPowerDesign) {
      DEBUG ((DEBUG_ERROR, "VR ERROR: VrPowerDeliveryDesign is missing! VR override values may cause system instability!\n"));
    } else if ( CpuPowerMgmtVrConfig->VrPowerDeliveryDesign <= EnumMaxCflVrPowerDesign ){
      ///
      /// This field is required to be non-zero on CFL S-based platforms.
      /// Used to communicate the power delivery design of the board.
      /// The VR override code traditionally looked at the CPU sku to determine the default VR
      /// override values. However, the CFL-S platforms require different VR values based on the
      /// board power delivery design. This is due to CPU socket compatibility across different board
      /// designs. Without this field populated, the system may experience VR OCP shutdowns, hangs,
      /// thermal and performance loss.
      ///
      CpuIdentifier = (CPU_OVERRIDE_IDENTIFIER) CpuPowerMgmtVrConfig->VrPowerDeliveryDesign;
    }
  }

  ///
  /// Update the VR config block with the VR override table data.
  /// Only override if the config block value is AUTO/default and the
  /// override table data is non-zero.
  ///
  if (sizeof(CPU_VR_OVERRIDE_TABLE)!= 0){
    NoOfOverrides = (sizeof (mCpuVrOverrideTable)) / (sizeof (CPU_VR_OVERRIDE_TABLE));
  }

  for (TableIndex = 0; TableIndex < NoOfOverrides; TableIndex++, VrOverrideTable++) {
    ///
    /// If Cpu Identifier matches, then update with overrides values
    ///
    if (VrOverrideTable->CpuIdentifier == CpuIdentifier) {
      ///
      /// ICC Max Overrides. Only override default if left at AUTO.
      ///
      if ((CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_IA] == AUTO) && (VrOverrideTable->IaIccMax != 0)) {
        CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_IA] = VrOverrideTable->IaIccMax;
      }
      if ((CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_GT] == AUTO) && (VrOverrideTable->GtIccMax != 0)) {
        CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_GT] = VrOverrideTable->GtIccMax;
      }
      if ((CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_SA] == AUTO) && (VrOverrideTable->SaIccMax != 0)) {
        CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_SA] = VrOverrideTable->SaIccMax;
      }
      if ((CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_FIVR] == AUTO) && (VrOverrideTable->VccInIccMax != 0)) {
        CpuPowerMgmtVrConfig->IccMax[CPU_VR_DOMAIN_FIVR] = VrOverrideTable->VccInIccMax;
      }

      ///
      /// VR TDC Overrides. Only override default if left at AUTO.
      ///
      if ((CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_IA] == AUTO) && (VrOverrideTable->IaTdclimit != 0)) {
        CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_IA] = VrOverrideTable->IaTdclimit;
        CpuPowerMgmtVrConfig->TdcEnable[CPU_VR_DOMAIN_IA] = 1;
      }
      if ((CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_GT] == AUTO) && (VrOverrideTable->GtTdclimit != 0)) {
        CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_GT] = VrOverrideTable->GtTdclimit;
        CpuPowerMgmtVrConfig->TdcEnable[CPU_VR_DOMAIN_GT] = 1;
      }
      if ((CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_SA] == AUTO) && (VrOverrideTable->SaTdclimit != 0)) {
        CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_SA] = VrOverrideTable->SaTdclimit;
        CpuPowerMgmtVrConfig->TdcEnable[CPU_VR_DOMAIN_SA] = 1;
      }
      if ((CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_FIVR] == AUTO) && (VrOverrideTable->VccInTdclimit != 0)) {
        CpuPowerMgmtVrConfig->TdcPowerLimit[CPU_VR_DOMAIN_FIVR] = VrOverrideTable->VccInTdclimit;
        CpuPowerMgmtVrConfig->TdcEnable[CPU_VR_DOMAIN_FIVR] = 1;
      }

      ///
      /// AC/DC Loadlines. Only override default if left at AUTO.
      ///
      if ((CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_IA] == AUTO) && (VrOverrideTable->IaAcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_IA] = VrOverrideTable->IaAcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_IA] == AUTO) && (VrOverrideTable->IaDcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_IA] = VrOverrideTable->IaDcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_GT] == AUTO)  && (VrOverrideTable->GtAcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_GT] = VrOverrideTable->GtAcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_GT] == AUTO) && (VrOverrideTable->GtDcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_GT] = VrOverrideTable->GtDcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_SA] == AUTO) && (VrOverrideTable->SaAcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_SA] = VrOverrideTable->SaAcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_SA] == AUTO) && (VrOverrideTable->SaDcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_SA] = VrOverrideTable->SaDcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_FIVR] == AUTO) && (VrOverrideTable->VccInAcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->AcLoadline[CPU_VR_DOMAIN_FIVR] = VrOverrideTable->VccInAcLoadLine;
      }
      if ((CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_FIVR] == AUTO) && (VrOverrideTable->VccInDcLoadLine != 0)) {
        CpuPowerMgmtVrConfig->DcLoadline[CPU_VR_DOMAIN_FIVR] = VrOverrideTable->VccInDcLoadLine;
      }

      ///
      ///  VccIn VR Voltage Limit. Only override if default is left at AUTO.
      ///
      if ((CpuPowerMgmtVrConfig->VrVoltageLimit[CPU_VR_DOMAIN_FIVR] == AUTO) && (VrOverrideTable->VrVoltageLimit != 0)) {
        CpuPowerMgmtVrConfig->VrVoltageLimit[CPU_VR_DOMAIN_FIVR] = VrOverrideTable->VrVoltageLimit;
      }

      ///
      /// Exit loop when the correct CPU overrides have been applied.
      ///
      break;
    }
  }
}

/**
  Prints the Vr Config block before programming

  @param[IN] CpuPowerMgmtVrConfig  - CPU Power Management VR Config Block
**/
VOID
PrintVrOverrides (
  IN CPU_POWER_MGMT_VR_CONFIG *CpuPowerMgmtVrConfig
  )
{
  UINT32        Index = 0;

  DEBUG ((DEBUG_INFO, "VR: Print VR overrides:\n"));
  DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : VrPowerDeliveryDesign : 0x%X\n", CpuPowerMgmtVrConfig->VrPowerDeliveryDesign));

  for (Index = 0; Index < MAX_NUM_VRS; Index++) {
    DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : VrConfigEnable[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->VrConfigEnable[Index]));
    if (CpuPowerMgmtVrConfig->VrConfigEnable[Index] == 1) {
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : TdcPowerLimit[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->TdcPowerLimit[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : AcLoadline[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->AcLoadline[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : DcLoadline[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->DcLoadline[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : Psi1Threshold[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->Psi1Threshold[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : Psi2Threshold[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->Psi2Threshold[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : Psi3Threshold[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->Psi3Threshold[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : Psi3Enable[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->Psi3Enable[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : Psi4Enable[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->Psi4Enable[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : ImonSlope1[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->ImonSlope1[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : ImonOffset[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->ImonOffset[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : IccMax[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->IccMax[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : VrVoltageLimit[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->VrVoltageLimit[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : TdcEnable[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->TdcEnable[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : TdcTimeWindow[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->TdcTimeWindow[Index]));
      DEBUG ((DEBUG_INFO, " CPU_POWER_MGMT_VR_CONFIG : TdcLock[%X] : 0x%X\n", Index, CpuPowerMgmtVrConfig->TdcLock[Index]));
    }
  }
}

/**
  Programs the VR parameters for the external VR's which support SVID communication.

  @param[IN OUT] SiPolicyPpi    - The SI Policy PPI instance
**/
VOID
ConfigureSvidVrs(
  IN OUT SI_POLICY_PPI *SiPolicyPpi
  )
{
  EFI_STATUS                  Status;
  UINT32                      MailboxData;
  UINT32                      MailboxCmd;
  UINT32                      MailboxStatus;
  UINT32                      MailboxType;
  UINT64                      TempAcLoadline;
  UINT64                      TempDcLoadline;
  VR_TOPOLOGY_DATA            VrTopology;
  UINT8                       TempVrAddress;
  UINT8                       SvidEnabled;
  UINT8                       ConvertedTimeWindow;
  UINTN                       VrIndex;
  UINTN                       MaxVrIndex;
  CPU_CONFIG                  *CpuConfig;
  CPU_POWER_MGMT_VR_CONFIG    *CpuPowerMgmtVrConfig;
  CPU_TEST_CONFIG             *CpuTestConfig;
  CPU_POWER_MGMT_PSYS_CONFIG  *CpuPowerMgmtPsysConfig;
#ifndef CPU_CFL
  CPU_SKU                     CpuSku;
  CPU_FAMILY                  CpuFamily;

  CpuSku    = GetCpuSku ();
  CpuFamily = GetCpuFamily ();
#endif

  DEBUG ((DEBUG_INFO, "VR: Configure SVID Vr's\n"));

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuConfigGuid, (VOID *) &CpuConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtVrConfigGuid, (VOID *) &CpuPowerMgmtVrConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuTestConfigGuid, (VOID *) &CpuTestConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtPsysConfigGuid, (VOID *) &CpuPowerMgmtPsysConfig);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "VR: Update VR overrides\n"));
  UpdateVrOverrides (CpuPowerMgmtVrConfig);
  PrintVrOverrides (CpuPowerMgmtVrConfig);

  ///
  /// Send command for MPS IMPV8 VR if requested
  ///
  if (CpuPowerMgmtVrConfig->SendVrMbxCmd == MPS_VR_CMD_REQUESTED) {
    MailboxType = MAILBOX_TYPE_PCODE;
    MailboxCmd  = WRITE_MPS_VR_IMPV8_CMD;
    MailboxData = 0x1;
    Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
    if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
      DEBUG ((DEBUG_ERROR, "VR: Error writing MPS VR IMPV8 command. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
    }
  }

  ///
  /// Send command for PS4 exit failures on specific VRs if requested
  ///
  if (CpuPowerMgmtVrConfig->SendVrMbxCmd == PS4_EXIT_VR_CMD_REQUESTED) {
    MailboxType = MAILBOX_TYPE_PCODE;
    MailboxCmd  = WRITE_PS4_EXIT_VR_CMD;
    MailboxData = 0x0;
    Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
    if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
      DEBUG ((DEBUG_ERROR, "VR: Error writing PS4 Exit VR command. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
    }
  }

  ///
  /// Send Intersil VR command if requested.
  ///
  /// 0 - No command requested
  /// 1 - Send command for IA/GT
  /// 2 - Send command for IA/GT/SA
  ///
  if (CpuPowerMgmtVrConfig->IslVrCmd) {
    MailboxType = MAILBOX_TYPE_PCODE;
    MailboxCmd  = WRITE_ISL_VR_MBX_CMD;
    MailboxData = ISL_VR_IA_GT_CMD_DATA; // Default as ISL_VR_IA_GT_CMD_REQUESTED
    if (CpuPowerMgmtVrConfig->IslVrCmd == ISL_VR_IA_GT_SA_CMD_REQUESTED) {
      MailboxData = ISL_VR_IA_GT_SA_CMD_DATA;
    }

    DEBUG ((DEBUG_INFO, "VR: Sending ISL VR mailbox command with data = %X\n", MailboxData));
    Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
    if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
      DEBUG ((DEBUG_ERROR, "VR: Error writing ISL VR command. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
    }
  }

  ///
  /// Initialize Reliability features
  ///
  MailboxCmd  = WRITE_VOLTAGE_OPTIMIZATION_CMD;
  MailboxType = MAILBOX_TYPE_PCODE;
  ///
  /// Voltage Optimization mailbox command uses reverse encoding. 0 - Enable, 1 - Disable.
  ///
  MailboxData = ~CpuTestConfig->VoltageOptimization & BIT0;
  Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
  if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
     DEBUG ((DEBUG_ERROR, "VR: Error writing Voltage Optimization. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
  }

  ///
  /// CPU wakeup Timer programming. 0 - 8 seconds, 1 - 180 seconds.
  ///
  MailboxCmd  = WRITE_CPU_WAKEUP_TIMER_CMD;
  MailboxType = MAILBOX_TYPE_PCODE;
  MailboxData = CpuTestConfig->CpuWakeUpTimer;
  Status = MailboxWrite(MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
  if (EFI_ERROR(Status) || (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS)) {
     DEBUG ((DEBUG_ERROR, "VR: Error writing Cpu Wakeup Timer. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
  }

  ///
  /// CPU VR MSR mailbox
  ///
  MailboxType = MAILBOX_TYPE_VR_MSR;

  /**
    Configure Platform Level controls

    PSYS Config
    -PsysSlope is defined as U10.1.9 fixed point
    -Policy Psys slope is defined in 1/100 increments
    -Mailbox PsysSlope = (PlatPolicyPsysSlope * 2^9)/100
  **/
  if (CpuPowerMgmtVrConfig->PsysOffset != 0 || CpuPowerMgmtVrConfig->PsysSlope != 0 ) {
    MailboxData =  (UINT32)(CpuPowerMgmtVrConfig->PsysOffset) |
                   (((CpuPowerMgmtVrConfig->PsysSlope * (1 << 9)) /100) << PSYS_SLOPE_OFFSET);
    MailboxCmd = WRITE_PSYS_CONFIG_CMD;
    Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
    if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "VR: Error Writing PSYS Config. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
    }
  }

  ///
  /// PSYS Pmax
  /// -PMax is defined as U16.10.6 fixed point
  /// -Policy Pmax is defined in 1/8 W increments
  /// -Mailbox Pmax = (PlatPolicyPmax * 2^6)/8
  ///
  if (CpuPowerMgmtPsysConfig->PsysPmax != 0) {
    MailboxData =  (UINT32)((CpuPowerMgmtPsysConfig->PsysPmax * (1<<6))/8);
    MailboxCmd = WRITE_PSYS_PMAX_CMD;
    Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
    if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "VR: Error Writing Psys PMAX. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
    }
  }

  ///
  /// Get CPU VR topology
  ///
  Status = MailboxRead (MailboxType, READ_VR_STRAP_CONFIG_CMD, (UINT32*)&VrTopology.Fields, &MailboxStatus);
  if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "VR: Error Reading VR topology. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
  }

  ///
  /// Print VR Topology data
  ///
  DEBUG ((DEBUG_INFO, "VR: VR Topology data = 0x%x\n", VrTopology.Fields));
  DEBUG ((DEBUG_INFO, "    VR Type 0 = SVID, VR Type 1 = non-SVID\n"));
  DEBUG ((DEBUG_INFO, "    SA VR Address    = 0x%x\n", VrTopology.Fields.VrSaAddress));
  DEBUG ((DEBUG_INFO, "    SA VR Type       = 0x%x\n", VrTopology.Fields.VrSaSvidType));
  DEBUG ((DEBUG_INFO, "    IA VR Address    = 0x%x\n", VrTopology.Fields.VrIaAddress));
  DEBUG ((DEBUG_INFO, "    IA VR Type       = 0x%x\n", VrTopology.Fields.VrIaSvidType));
  DEBUG ((DEBUG_INFO, "    Ring VR Address  = 0x%x\n", VrTopology.Fields.VrRingAddress));
  DEBUG ((DEBUG_INFO, "    Ring VR Type     = 0x%x\n", VrTopology.Fields.VrRingSvidType));
  DEBUG ((DEBUG_INFO, "    GT VR Address    = 0x%x\n", VrTopology.Fields.VrGtAddress));
  DEBUG ((DEBUG_INFO, "    GT VR Type       = 0x%x\n", VrTopology.Fields.VrGtSvidType));
  DEBUG ((DEBUG_INFO, "    FIVR VR Address  = 0x%x\n", VrTopology.Fields.VrFivrAddress));
  DEBUG ((DEBUG_INFO, "    FIVR VR Type     = 0x%x\n", VrTopology.Fields.VrFivrSvidType));

#ifdef CPU_CFL
  ///
  /// No FIVR architecture on CFL CPU
  ///
  MaxVrIndex = CPU_VR_DOMAIN_GT;
#else
  if ((CpuFamily == EnumCpuCnlUltUlx) && (CpuSku == EnumCpuUlx)) {
    ///
    /// CNL-Y uses McIVR VR architecture which is the only sku with VccIn overrides
    ///
    MaxVrIndex = CPU_VR_DOMAIN_FIVR;
  } else {
    ///
    /// This will bypass the VccIn domain from being programmed
    ///
    MaxVrIndex = CPU_VR_DOMAIN_GT;
  }
#endif

  ///
  /// Set VR configuration parameters for all VR domains
  /// SA = 0, IA = 1, Ring = 2, GT = 3, FIVR = 4
  ///
  for (VrIndex = CPU_VR_DOMAIN_SA; VrIndex <= MaxVrIndex; VrIndex++) {
    switch (VrIndex) {

      case CPU_VR_DOMAIN_SA:
        TempVrAddress = (UINT8) VrTopology.Fields.VrSaAddress;
        SvidEnabled = (UINT8) (~VrTopology.Fields.VrSaSvidType) & BIT0;
        break;

      case CPU_VR_DOMAIN_IA:
        TempVrAddress = (UINT8) VrTopology.Fields.VrIaAddress;
        SvidEnabled = (UINT8) (~VrTopology.Fields.VrIaSvidType) & BIT0;
        break;

      case CPU_VR_DOMAIN_GT:
        TempVrAddress = (UINT8) VrTopology.Fields.VrGtAddress;
        SvidEnabled = (UINT8) (~VrTopology.Fields.VrGtSvidType) & BIT0;
        break;

      case CPU_VR_DOMAIN_FIVR:
        TempVrAddress = (UINT8) VrTopology.Fields.VrFivrAddress;
        SvidEnabled = (UINT8) (~VrTopology.Fields.VrFivrSvidType) & BIT0;
        break;

      default:
        TempVrAddress = 0;
        SvidEnabled = 0;
        break;
    }

    if (CpuPowerMgmtVrConfig->VrConfigEnable[VrIndex] == 1 && SvidEnabled == 1) {
      ///
      /// AC/DC Loadline
      ///
      if (CpuPowerMgmtVrConfig->AcLoadline[VrIndex] != 0 || CpuPowerMgmtVrConfig->DcLoadline[VrIndex] != 0) {
        ///
        ///  Check max AC/DC loadline boundary. Max allowed is 6249 (62.49 mOhm)
        ///
        if (CpuPowerMgmtVrConfig->AcLoadline[VrIndex] > AC_DC_LOADLINE_MAX){
          CpuPowerMgmtVrConfig->AcLoadline[VrIndex] = AC_DC_LOADLINE_MAX;
        } else if (CpuPowerMgmtVrConfig->DcLoadline[VrIndex] > AC_DC_LOADLINE_MAX){
          CpuPowerMgmtVrConfig->DcLoadline[VrIndex] = AC_DC_LOADLINE_MAX;
        }

        ///
        ///  Loadline is 1/100 mOhm units. Mailbox interface requires Loadline in U-4.20 Ohms format.
        ///  After multiplying by 2^20, we must divide the result by 100,000 to convert to Ohms.
        ///  Adding half of divisor to dividend to account for rounding errors in fixed point arithmetic.
        ///
        TempAcLoadline = MultU64x64(CpuPowerMgmtVrConfig->AcLoadline[VrIndex], LShiftU64 (1, 20));
        TempAcLoadline = DivU64x32(TempAcLoadline + 50000, 100000);

        TempDcLoadline = MultU64x64(CpuPowerMgmtVrConfig->DcLoadline[VrIndex], LShiftU64 (1, 20));
        TempDcLoadline = DivU64x32(TempDcLoadline + 50000, 100000);

        MailboxData = (UINT32) (TempAcLoadline | (LShiftU64 (TempDcLoadline, DC_LOADLINE_OFFSET)));
        MailboxCmd = WRITE_ACDC_LOADLINE_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing AC/DC Loadline. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// PS Cutoff Current
      ///
      MailboxData =  (UINT32)(CpuPowerMgmtVrConfig->Psi1Threshold[VrIndex] & PSI_THRESHOLD_MASK) |
                     ((CpuPowerMgmtVrConfig->Psi2Threshold[VrIndex] & PSI_THRESHOLD_MASK) << PSI2_THRESHOLD_OFFSET) |
                     ((CpuPowerMgmtVrConfig->Psi3Threshold[VrIndex] & PSI_THRESHOLD_MASK) << PSI3_THRESHOLD_OFFSET) |
                     ((~CpuPowerMgmtVrConfig->Psi3Enable[VrIndex] & BIT0) << PSI3_ENABLE_OFFSET) |
                     ((~CpuPowerMgmtVrConfig->Psi4Enable[VrIndex] & BIT0) << PSI4_ENABLE_OFFSET);
      MailboxCmd = WRITE_PSI_CUTOFF_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "VR: Error Writing PS Cutoff Current. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
      }

      ///
      /// IMON Config
#ifdef CPU_CFL
      /// -ImonSlope1 is defined as U10.1.9 fixed point [17:8]
      /// -ImonOffset is defined as S13.5.8 fixed point [31:18].
      /// -Policy Imon slope is defined in 1/100 increments
      /// -Policy Imon offset is defined in 1/1000 increments
      /// -Mailbox ImonSlope1 = (PlatPolicyImonSlope * 2^9)/100
      /// -Mailbox ImonOffset = (PlatPolicyImonOffset * 2^8)/1000
      /// -Adding half of divisor to dividend to account for rounding errors in fixed point arithmetic.
      ///
      MailboxData =  (UINT32)(((((CpuPowerMgmtVrConfig->ImonOffset[VrIndex] *(1 << 8) + 500) / 1000) << VR_IMON_OFFSET_HIGH_RES_OFFSET) & VR_IMON_OFFSET_HIGH_RES_MASK)
                           | (((CpuPowerMgmtVrConfig->ImonSlope1[VrIndex] * (1 << 9) + 50) / 100) << VR_IMON_SLOPE_OFFSET));

#else
      /// -ImonOffset is defined as S16.7.8 fixed point
      /// -ImonSlope1 is defined as U16.1.15 fixed point
      /// -Policy Imon offset is defined in 1/1000 increments
      /// -Policy Imon slope is defined in 1/100 increments
      /// -Mailbox ImonOffset = (PlatPolicyImonOffset * 2^8)/1000
      /// -Mailbox ImonSlope1 = (PlatPolicyImonSlope * 2^15)/100
      /// -Adding half of divisor to dividend to account for rounding errors in fixed point arithmetic.
      ///
      MailboxData =  (UINT32)(((CpuPowerMgmtVrConfig->ImonOffset[VrIndex] *(1 << 8) + 500) / 1000))
                             | (((CpuPowerMgmtVrConfig->ImonSlope1[VrIndex] * (1 << 15) + 50) / 100) << VR_IMON_SLOPE_OFFSET);
#endif
      MailboxCmd = WRITE_IMON_CONFIG_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "VR: Error Writing IMON Config. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
      }

      ///
      /// Icc Max
      ///
      if (CpuPowerMgmtVrConfig->IccMax[VrIndex] != 0) {
        MailboxData =  (UINT32)CpuPowerMgmtVrConfig->IccMax[VrIndex];
        MailboxCmd = WRITE_VR_ICC_MAX_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing IccMax. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// VR Voltage Limit
      /// -Mailbox Voltage Limit defined as U16.3.13, Range 0-7.999V
      /// -Policy defined in mV, Range 0-7999mV
      /// -Adding half of divisor to dividend to account for rounding errors in fixed point arithmetic.
      ///
      if (CpuPowerMgmtVrConfig->VrVoltageLimit[VrIndex] != 0) {
        MailboxData =  (UINT32)((CpuPowerMgmtVrConfig->VrVoltageLimit[VrIndex] * (1 << 13) + 500) / 1000) & VR_VOLTAGE_LIMIT_MASK;
        MailboxCmd = WRITE_VR_VOLTAGE_LIMIT_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing VR Voltage Limit. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// VR TDC Settings
      /// -Mailbox TDC Current Limit defined as U15.12.3, Range 0-4095A
      ///    -Policy defined in 1/8 A increments
      /// -Time window mailbox format, in binary, is xxyyyyy, time window = (1+X/4)*pow(2,Y)
      ///    -Set to 1ms default (mailbox value = 0000000b)
      ///

      ///
      /// Ensure Time Window Value is within the supported range.
      ///
      if (CpuPowerMgmtVrConfig->TdcTimeWindow[VrIndex] <= VR_TDC_TIME_WINDOW_MAX) {
        ConvertedTimeWindow = mMilliSecondsToMsrValueTable[CpuPowerMgmtVrConfig->TdcTimeWindow[VrIndex]][1];
      } else {
        ConvertedTimeWindow = 0;
      }

      MailboxData =  (UINT32)((CpuPowerMgmtVrConfig->TdcPowerLimit[VrIndex] & VR_TDC_CURRENT_LIMIT_MASK) |
                             ((CpuPowerMgmtVrConfig->TdcEnable[VrIndex] & BIT0) << VR_TDC_ENABLE_OFFSET) |
                             ((ConvertedTimeWindow & VR_TDC_TIME_WINDOW_MASK) << VR_TDC_TIME_WINDOW_OFFSET) |
                             ((CpuPowerMgmtVrConfig->TdcLock[VrIndex] & BIT0) << VR_TDC_LOCK_OFFSET));
      MailboxCmd = WRITE_VR_TDC_CONFIG_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_TDC_ADDRESS_OFFSET);

      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "VR: Error Writing VR TDC Config. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
      }
    }

    ///
    /// Acoustic Noise Mitigation
    ///
    if (CpuPowerMgmtVrConfig->AcousticNoiseMitigation) {

      ///
      /// PreWake, Ramp Up and Ramp Down times programming for Acoustic Noise Mitigation.
      ///
      MailboxCmd  = WRITE_ACOUSTIC_MIGITATION_RANGE;
      MailboxData = (CpuPowerMgmtVrConfig->PreWake & NOISE_MIGITATION_RANGE_MASK)
                    | ((CpuPowerMgmtVrConfig->RampUp & NOISE_MIGITATION_RANGE_MASK) << RAMP_UP_OFFSET)
                    | ((CpuPowerMgmtVrConfig->RampDown & NOISE_MIGITATION_RANGE_MASK) << RAMP_DOWN_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "Error Writing Acoustic Migitation Range. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
      }

      ///
      /// Set Fast and Slow Slew Rate for Deep Package C States
      ///

      ///
      /// IA domain
      ///
      if (VrIndex == CPU_VR_DOMAIN_IA) {
        MailboxCmd  = WRITE_SVID_SET_DISABLE_FAST_PKGC_RAMP_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->FastPkgCRampDisableIa;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing disable Fast Deep Package C States for IA. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }

        MailboxCmd  = WRITE_SVID_SET_VR_SLEW_RATE_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->SlowSlewRateForIa;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing Slow Slew Rate for IA. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// GT domain
      ///
      if (VrIndex == CPU_VR_DOMAIN_GT) {
        MailboxCmd  = WRITE_SVID_SET_DISABLE_FAST_PKGC_RAMP_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->FastPkgCRampDisableGt;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing disable Fast Deep Package C States for GT. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }

        MailboxCmd  = WRITE_SVID_SET_VR_SLEW_RATE_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->SlowSlewRateForGt;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing Slow Slew Rate for GT. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// FIVR domain
      ///
      if (VrIndex == CPU_VR_DOMAIN_FIVR) {
        MailboxCmd  = WRITE_SVID_SET_DISABLE_FAST_PKGC_RAMP_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->FastPkgCRampDisableFivr;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing disable Fast Deep Package C States for FIVR. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }

        MailboxCmd  = WRITE_SVID_SET_VR_SLEW_RATE_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->SlowSlewRateForFivr;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing Slow Slew Rate for FIVR. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }

      ///
      /// SA domain
      ///
      if (VrIndex == CPU_VR_DOMAIN_SA) {
        MailboxCmd  = WRITE_SVID_SET_DISABLE_FAST_PKGC_RAMP_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->FastPkgCRampDisableSa;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing disable Fast Deep Package C States for SA. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }

        MailboxCmd  = WRITE_SVID_SET_VR_SLEW_RATE_CMD | ((TempVrAddress & VR_ADDRESS_MASK) << VR_ADDRESS_OFFSET);
        MailboxData = CpuPowerMgmtVrConfig->SlowSlewRateForSa;
        Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
        if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
          DEBUG ((DEBUG_ERROR, "VR: Error Writing Slow Slew Rate for SA. EFI_STATUS = %r, Mailbox Status = %X\n", Status, MailboxStatus));
        }
      }
    }
  }

  ///
  /// RFI Mitigation
  ///
  UpdateRfiSettings (CpuPowerMgmtVrConfig);
}

/**
  Programs the PID parameters

  @param[in] SiPolicyPpi - The SI Policy PPI instance
**/
VOID
ConfigurePidSettings (
  IN OUT SI_POLICY_PPI *SiPolicyPpi
  )
{
  EFI_STATUS          Status;
  UINT32              MailboxData;
  UINT32              MailboxCmd;
  UINT32              MailboxStatus;
  UINT32              MailboxType;
  UINTN               Index;
  CPU_PID_TEST_CONFIG *CpuPidTestConfig;

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPidTestConfigGuid, (VOID *) &CpuPidTestConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// PID tuning must be enabled to continue
  ///
  if (CpuPidTestConfig->PidTuning == 0) {
    return;
  }

  ///
  /// Send PID mailbox commands. If any setting is non-zero we need to update
  /// that setting for all domains.
  ///
  DEBUG ((DEBUG_INFO, "PID: Configure PID Settings\n"));
  MailboxType = MAILBOX_TYPE_PCODE;

  ///
  /// RATL
  ///
  if ( (CpuPidTestConfig->Ratl[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->Ratl[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->Ratl[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->Ratl[Index];
      MailboxCmd = WRITE_PID_RATL | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing RATL for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// VR TDC for VR0
  ///
  if ( (CpuPidTestConfig->VrTdcVr0[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->VrTdcVr0[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->VrTdcVr0[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->VrTdcVr0[Index];
      MailboxCmd = WRITE_PID_VR0_TDC | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing VR TDC VR0 for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// VR TDC for VR1
  ///
  if ( (CpuPidTestConfig->VrTdcVr1[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->VrTdcVr1[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->VrTdcVr1[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->VrTdcVr1[Index];
      MailboxCmd = WRITE_PID_VR1_TDC | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing VR TDC VR1 for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// VR TDC for VR2
  ///
  if ( (CpuPidTestConfig->VrTdcVr2[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->VrTdcVr2[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->VrTdcVr2[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->VrTdcVr2[Index];
      MailboxCmd = WRITE_PID_VR2_TDC | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing VR TDC VR2 for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// VR TDC for VR3
  ///
  if ( (CpuPidTestConfig->VrTdcVr3[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->VrTdcVr3[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->VrTdcVr3[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->VrTdcVr3[Index];
      MailboxCmd = WRITE_PID_VR3_TDC | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing VR TDC VR3 for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM PSYS PL1 MSR
  ///
  if ( (CpuPidTestConfig->PbmPsysPl1Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPsysPl1Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPsysPl1Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPsysPl1Msr[Index];
      MailboxCmd = WRITE_PID_PSYS_PL1_MSR| (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPsysPl1Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM PSYS PL1 MMIO PCS
  ///
  if ( (CpuPidTestConfig->PbmPsysPl1MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPsysPl1MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPsysPl1MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPsysPl1MmioPcs[Index];
      MailboxCmd = WRITE_PID_PSYS_PL1_MMIO| (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPsysPl1MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM PSYS PL2 MSR
  ///
  if ( (CpuPidTestConfig->PbmPsysPl2Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPsysPl2Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPsysPl2Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPsysPl2Msr[Index];
      MailboxCmd = WRITE_PID_PSYS_PL2_MSR | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPsysPl2Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM PSYS PL2 MMIO PCS
  ///
  if ( (CpuPidTestConfig->PbmPsysPl2MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPsysPl2MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPsysPl2MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPsysPl2MmioPcs[Index];
      MailboxCmd = WRITE_PID_PSYS_PL2_MMIO | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPsysPl2MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM Package PL1 MSR
  ///
  if ( (CpuPidTestConfig->PbmPkgPl1Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPkgPl1Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPkgPl1Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPkgPl1Msr[Index];
      MailboxCmd = WRITE_PID_PKG_PL1_MSR | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPkgPl1Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM Package PL1 MMIO PCS
  ///
  if ( (CpuPidTestConfig->PbmPkgPl1MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPkgPl1MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPkgPl1MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPkgPl1MmioPcs[Index];
      MailboxCmd = WRITE_PID_PKG_PL1_MMIO | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPkgPl1MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM Package PL2 MSR
  ///
  if ( (CpuPidTestConfig->PbmPkgPl2Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPkgPl2Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPkgPl2Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPkgPl2Msr[Index];
      MailboxCmd = WRITE_PID_PKG_PL2_MSR | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPkgPl2Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// PBM Package PL2 MMIO PCS
  ///
  if ( (CpuPidTestConfig->PbmPkgPl2MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->PbmPkgPl2MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->PbmPkgPl2MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->PbmPkgPl2MmioPcs[Index];
      MailboxCmd = WRITE_PID_PKG_PL2_MMIO | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing PbmPkgPl2MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// DDR PL1 MSR
  ///
  if ( (CpuPidTestConfig->DdrPl1Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->DdrPl1Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->DdrPl1Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->DdrPl1Msr[Index];
      MailboxCmd = WRITE_PID_DDR_PL1_MSR | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing DdrPl1Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// DDR PL1 MMIO PCS
  ///
  if ( (CpuPidTestConfig->DdrPl1MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->DdrPl1MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->DdrPl1MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->DdrPl1MmioPcs[Index];
      MailboxCmd = WRITE_PID_DDR_PL1_MMIO | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing DdrPl1MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// DDR PL2 MSR
  ///
  if ( (CpuPidTestConfig->DdrPl2Msr[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->DdrPl2Msr[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->DdrPl2Msr[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->DdrPl2Msr[Index];
      MailboxCmd = WRITE_PID_DDR_PL2_MSR | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing DdrPl2Msr for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

  ///
  /// DDR PL2 MMIO PCS
  ///
  if ( (CpuPidTestConfig->DdrPl2MmioPcs[PID_DOMAIN_KP] != 0) ||
       (CpuPidTestConfig->DdrPl2MmioPcs[PID_DOMAIN_KI] != 0) ||
       (CpuPidTestConfig->DdrPl2MmioPcs[PID_DOMAIN_KD] != 0) ) {

    for (Index = PID_DOMAIN_KP; Index <= PID_DOMAIN_KD; Index++) {
      MailboxData = CpuPidTestConfig->DdrPl2MmioPcs[Index];
      MailboxCmd = WRITE_PID_DDR_PL2_MMIO | (Index << MAILBOX_PARAM_1_OFFSET);
      Status = MailboxWrite (MailboxType, MailboxCmd, MailboxData, &MailboxStatus);
      if (MailboxStatus != PCODE_MAILBOX_CC_SUCCESS) {
        DEBUG ((DEBUG_ERROR, "PID: Error Writing DdrPl2MmioPcs for index %X. EFI_STATUS = %r, Mailbox Status = %X\n", Index, Status, MailboxStatus));
      }
    }
  }

}

/**
  Set processor P state based on input parameter.

  @param[in out] Buffer - Pointer to UINT8 BootRatio parameter.
**/
VOID
EFIAPI
ApSafeSetBootPState (
  IN OUT VOID *Buffer
  )
{
  UINT8        BootRatio;
  MSR_REGISTER Ia32PerfCtl;

  ///
  /// Extract the boot ratio from the buffer.
  ///
  BootRatio = *((UINT8 *) Buffer);

  Ia32PerfCtl.Qword = AsmReadMsr64 (MSR_IA32_PERF_CTL);
  Ia32PerfCtl.Qword &= (UINT64) ~B_IA32_PERF_CTRLP_STATE_TARGET;
  Ia32PerfCtl.Qword |= (LShiftU64 (BootRatio, N_IA32_PERF_CTRLP_STATE_TARGET) & B_IA32_PERF_CTRLP_STATE_TARGET);
  AsmWriteMsr64 (MSR_IA32_PERF_CTL, Ia32PerfCtl.Qword);

  return;
}

/**
  Set the Boot Frequency across all logical processors.

  @param[in] PeiServices   Pointer to PEI Services Table
  @param[in] SiPolicyPpi - The SI Policy PPI instance.
**/
VOID
SetBootFrequency (
  IN CONST EFI_PEI_SERVICES  **PeiServices,
  IN SI_POLICY_PPI           *SiPolicyPpi
  )
{
  UINT8                              BootRatio;
  MSR_REGISTER                       PlatformInfoMsr;
  MSR_REGISTER                       Ia32MiscEnable;
  MSR_REGISTER                       MsrValue;
  UINT8                              MaxBusRatio;
  UINT8                              MinBusRatio;
  UINT8                              BootFrequencyMode;
  CPUID_THERMAL_POWER_MANAGEMENT_EAX Eax;
  UINT8                              NumberOfConfigTdpLevels;
  UINT8                              SupportedCtdpLevels;
  UINT8                              SelectedCtdpLevel;
  UINT8                              CtdpTarRatioPlusOne;
  CPU_POWER_MGMT_BASIC_CONFIG        *CpuPowerMgmtBasicConfig;
  CPU_POWER_MGMT_TEST_CONFIG         *CpuPowerMgmtTestConfig;
  CPU_POWER_MGMT_CUSTOM_CONFIG       *CpuPowerMgmtCustomConfig;
  CPU_CONFIG_LIB_PREMEM_CONFIG       *CpuConfigLibPreMemConfig;
  EFI_STATUS                         Status;
  SI_PREMEM_POLICY_PPI               *SiPreMemPolicyPpi;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtBasicConfigGuid, (VOID *) &CpuPowerMgmtBasicConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtCustomConfigGuid, (VOID *) &CpuPowerMgmtCustomConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuPowerMgmtTestConfigGuid, (VOID *) &CpuPowerMgmtTestConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuConfigLibPreMemConfigGuid, (VOID *) &CpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Get Maximum Non-Turbo bus ratio (HFM) from Platform Info MSR Bits[15:8]
  /// Get Maximum Efficiency bus ratio (LFM) from Platform Info MSR Bits[47:40]
  ///
  PlatformInfoMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_INFO);
  MaxBusRatio = PlatformInfoMsr.Bytes.SecondByte;
  MinBusRatio = PlatformInfoMsr.Bytes.SixthByte;

  BootRatio = MinBusRatio;
  BootFrequencyMode = (UINT8) CpuConfigLibPreMemConfig->BootFrequency;

  ///
  /// Temporarily enable EIST on BSP.
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_EIST;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  if (CpuConfigLibPreMemConfig->BootFrequency == 2) {
    if (CpuPowerMgmtBasicConfig->TurboMode && (CpuPowerMgmtTestConfig->Eist || CpuPowerMgmtBasicConfig->Hwp)) {
      //
      // Set processor P state as TURBO_RATIO_LIMIT_1C if available
      //
      AsmCpuid (CPUID_THERMAL_POWER_MANAGEMENT, &Eax.Uint32, NULL, NULL, NULL);
      if ((Eax.Bits.TurboBoostTechnology == 1) ||
          ((Ia32MiscEnable.Qword & B_MSR_IA32_MISC_DISABLE_TURBO) == B_MSR_IA32_MISC_DISABLE_TURBO)) {
        //
        // Enable Turbo
        //
        Ia32MiscEnable.Qword &= ~B_MSR_IA32_MISC_DISABLE_TURBO;
        AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
        //
        // Set Boot Ratio to TURBO_RATIO_LIMIT_1C
        //
        BootRatio = (UINT8) (AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT) & B_MSR_TURBO_RATIO_LIMIT_1C);
      } else {
        //
        // Turbo is not available, down to HFM
        //
        DEBUG ((DEBUG_INFO, "CPU: Turbo mode is not available, down to HFM mode.\n"));
        BootFrequencyMode = 1;
        BootRatio = MaxBusRatio;
      }
    } else {
      //
      // Turbo is disabled, down to HFM
      //
      BootFrequencyMode = 1;
      BootRatio = MaxBusRatio;
    }
  } else if (CpuConfigLibPreMemConfig->BootFrequency == 1) {
    BootRatio = MaxBusRatio;
  } else {
    BootRatio = MinBusRatio;
  }

  ///
  /// If there are any configurable TDP Levels supported, set the boot ratio
  /// based on BootFrequencyMode and cTDP TAR ratio.
  ///
  PlatformInfoMsr.Qword &= V_CONFIG_TDP_NUM_LEVELS_MASK;
  NumberOfConfigTdpLevels = (UINT8) RShiftU64 (PlatformInfoMsr.Qword, N_MSR_PLATFORM_INFO_CONFIG_TDP_NUM_LEVELS_OFFSET);
  if (NumberOfConfigTdpLevels != 0) {
    //
    // Find the selected level and the TAR ratio.
    //
    SupportedCtdpLevels = NumberOfConfigTdpLevels + 1;
    SelectedCtdpLevel = (UINT8) CpuPowerMgmtTestConfig->ConfigTdpLevel;

    //
    // Select cTDP Nominal if cTDP is disabled or the level is not supported.
    //
    if (SelectedCtdpLevel == CONFIG_TDP_DEACTIVATE || SelectedCtdpLevel >= SupportedCtdpLevels) {
      SelectedCtdpLevel = 0;
    }

    if (SelectedCtdpLevel >= 3) {
      DEBUG ((DEBUG_ERROR, "ERROR: Invalid SelectedCtdpLevel=%x, and SelectedCtdpLevel should be range in 0-2 \n", SelectedCtdpLevel));
      ASSERT (FALSE);
      return;
    }

    if (CpuPowerMgmtCustomConfig->CustomConfigTdpTable[SelectedCtdpLevel].CustomTurboActivationRatio != 0) {
      CtdpTarRatioPlusOne = (UINT8) CpuPowerMgmtCustomConfig->CustomConfigTdpTable[SelectedCtdpLevel].CustomTurboActivationRatio + 1;
    } else {
      MsrValue.Qword = AsmReadMsr64 (MSR_CONFIG_TDP_NOMINAL + SelectedCtdpLevel);
      if (SelectedCtdpLevel == 0) {
        CtdpTarRatioPlusOne = MsrValue.Bytes.FirstByte;
      } else if (SelectedCtdpLevel == 1) {
        if (MsrValue.Bytes.ThirdByte == 0) {
          //
          // If Level1 was not valid, then read Level2 register
          //
          MsrValue.Qword = AsmReadMsr64 (MSR_CONFIG_TDP_LVL2);
        }
        CtdpTarRatioPlusOne = MsrValue.Bytes.ThirdByte;
      } else {
        CtdpTarRatioPlusOne = MsrValue.Bytes.ThirdByte;
      }
    }

    //
    // Make sure the TAR+1 ratio is between the fused min and (max+1) ratios.
    //
    if (CtdpTarRatioPlusOne < MinBusRatio) {
      CtdpTarRatioPlusOne = MinBusRatio;
    } else if (CtdpTarRatioPlusOne > MaxBusRatio + 1) {
      CtdpTarRatioPlusOne = MaxBusRatio + 1;
    }

    if (BootFrequencyMode == 2) {
      //
      // Use TAR Ratio + 1 to go to Turbo mode once ConfigTDP is fully configured and the TAR ratio MSR is configured.
      //
      BootRatio = CtdpTarRatioPlusOne;
    } else if (BootFrequencyMode == 1) {
      //
      // Use TAR Ratio as max non-turbo ratio, which is equal to ConfigTDP boot ratio - 1.
      //
      BootRatio = CtdpTarRatioPlusOne - 1;

      //
      // If EIST is disabled use ConfigTDP boot ratio, which is equal to TAR+1.
      //
      if (!CpuPowerMgmtTestConfig->Eist) {
        BootRatio = BootRatio + 1;
      }
    } else {
      //
      // Use MinBusRatio (LFM) unless ConfigTDP boot ratio is equal to LFM.
      // In that case, use 'fake' P-State of ConfigTDP boot ratio - 1, which is equal to TAR ratio.
      //
      BootRatio = MinBusRatio;
      if ((CtdpTarRatioPlusOne == MinBusRatio) && CpuPowerMgmtTestConfig->Eist) {
        BootRatio = CtdpTarRatioPlusOne - 1;
      }
    }
  }

  ///
  /// Set MSR_IA32_PERF_CTL on BSP.
  ///
  ApSafeSetBootPState (&BootRatio);

  ///
  /// Set MSR_IA32_PERF_CTL on all APs unless disabled by policy setting.
  ///
  if (CpuPowerMgmtBasicConfig->SkipSetBootPState != CPU_FEATURE_ENABLE) {
    gMpServicesPpi->StartupAllAPs (
                      PeiServices,
                      gMpServicesPpi,
                      (EFI_AP_PROCEDURE) ApSafeSetBootPState,
                      FALSE,
                      0,
                      &BootRatio
                      );
  }
}

/**
  Initialize performance and power management features before RESET_CPL at Post-memory phase.

  @param[in] PeiServices     Pointer to PEI Services Table
  @param[in] SiPolicyPpi     The SI Policy PPI instance.
**/
VOID
CpuInitPreResetCpl (
  IN CONST EFI_PEI_SERVICES   **PeiServices,
  IN SI_POLICY_PPI             *SiPolicyPpi
  )
{
  EFI_STATUS                         Status;
  CPUID_VERSION_INFO_ECX             Ecx;
  CPU_TEST_CONFIG                    *CpuTestConfig;
  CPU_CONFIG                         *CpuConfig;
  CPU_CONFIG_LIB_PREMEM_CONFIG       *CpuConfigLibPreMemConfig;
  SI_PREMEM_POLICY_PPI               *SiPreMemPolicyPpi;

  DEBUG((DEBUG_INFO, "CpuInitPreResetCpl Start \n"));
  PostCode (0xC20);

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuTestConfigGuid, (VOID *) &CpuTestConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gCpuConfigGuid, (VOID *) &CpuConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuConfigLibPreMemConfigGuid, (VOID *) &CpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  DEBUG((DEBUG_INFO, "ProcessorsPrefetcherInitialization Start \n"));
  PostCode (0xC21);

  ///
  /// Initializes Processor Prefetcher
  ///
  ProcessorsPrefetcherInitialization (
          CpuTestConfig->MlcStreamerPrefetcher,
          CpuTestConfig->MlcSpatialPrefetcher
          );

  DEBUG((DEBUG_INFO, "InitRatl Start \n"));
  PostCode (0xC22);

  ///
  /// Program Tcc Activation Offset and  Running Average Temperature Limit (RATL)
  ///
  InitRatl (SiPolicyPpi);

  DEBUG((DEBUG_INFO, "ConfigureSvidVrs Start \n"));
  PostCode (0xC23);

  ///
  /// Program platform power and SVID VR's
  ///
  ConfigureSvidVrs (SiPolicyPpi);

  DEBUG((DEBUG_INFO, "ConfigurePidSettings Start \n"));
  PostCode (0xC24);

  ///
  /// Program PID Settings
  ///
  ConfigurePidSettings (SiPolicyPpi);

  ///
  /// Check to see if CPU supports EIST capability.
  ///
  if (CpuConfigLibPreMemConfig->SkipMpInit == 0) {
    AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &Ecx.Uint32, NULL);
    if (Ecx.Bits.EIST == 1) {
      DEBUG((DEBUG_INFO, "SetBootFrequency Start \n"));
      PostCode (0xC25);
      ///
      /// Program Boot Frequency on all threads.
      ///
      SetBootFrequency (PeiServices, SiPolicyPpi);
    }
  }

  ///
  /// BIOS Guard Initialization
  ///
  BiosGuardInit (SiPolicyPpi);

  DEBUG((DEBUG_INFO, "CpuInitPreResetCpl Done \n"));
  PostCode (0xC3F);

  return;
}
