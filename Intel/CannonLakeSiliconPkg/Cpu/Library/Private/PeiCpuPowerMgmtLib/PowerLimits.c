/** @file
  This file contains power management configuration functions for processors.

  <b>Acronyms:</b>
     - PPM:  Processor Power Management
     - TM:   Thermal Monitor
     - IST:  Intel(R) Speedstep technology
     - HT:   Hyper-Threading Technology

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
#include <Private/Library/CpuCommonLib.h>
#include <Ppi/SiPolicy.h>

///
/// PowerLimits Override table for all SKUs. Non-cTDP parts would have '0' data for TDP level information.
///
GLOBAL_REMOVE_IF_UNREFERENCED PPM_OVERRIDE_TABLE mPowerLimitsOverrideTable[]= {
///
/// CpuIdentifier             TDP      MSR PL1   MSR PL2   TdpUp   TdpUp    TdpNominal   TdpNominal  TdpDown   TdpDown  MSR PL4
///                                    PL1       PL2       PL1     PL2      PL1          PL2         PL1       PL2
  {EnumCflS35Watt22CpuId,     3500,    3500,     4400,     0,      4400,    0,           4400,       0,        4400,     7200}, /// 35W CFL-S 2+2
  {EnumCflS35Watt42CpuId,     3500,    3500,     6900,     0,      6900,    0,           6900,       0,        6900,     9000}, /// 35W CFL-S 4+2
  {EnumCflS35Watt62CpuId,     3500,    3500,     9200,     0,      9200,    0,           9200,       0,        9200,    12000}, /// 35W CFL-S 6+2
  {EnumCflS35Watt82CpuId,     3500,    3500,     9200,     0,      9200,    0,           9200,       0,        9200,    12000}, /// 35W CFL-S 8+2
  {EnumCflS54Watt22CpuId,     5400,    5400,     5900,     0,         0,    0,              0,       0,           0,     7400}, /// 54W CFL-S
  {EnumCflS58Watt22CpuId,     5800,    5800,     5900,     0,         0,    0,              0,       0,           0,     7400}, /// 58W CFL-S
  {EnumCflS62Watt42CpuId,     6200,    6200,     9000,     0,         0,    0,              0,       0,           0,    10600}, /// 62W CFL-S 4+2
  {EnumCflS62Watt42CpuId,     6200,    6200,     9000,     0,         0,    0,              0,       0,           0,    10600}, /// 62W CFL-S 4+2
  {EnumCflS65Watt42CpuId,     6500,    6500,     9000,     0,         0,    0,              0,       0,           0,    10600}, /// 65W CFL-S 4+2
  {EnumCflS65Watt62CpuId,     6500,    6500,    12200,     0,         0,    0,              0,       0,           0,    15900}, /// 65W CFL-S 6+2
  {EnumCflS65Watt82CpuId,     6500,    6500,    20000,     0,         0,    0,              0,       0,           0,    25800}, /// 65W CFL-S 8+2
  {EnumCflS71Watt42CpuId,     7100,    7100,    10000,     0,         0,    0,              0,       0,           0,    13400}, /// 71W CFL-S 4+2 workstation
  {EnumCflS83Watt42CpuId,     8300,    8300,    10000,     0,         0,    0,              0,       0,           0,    13200}, /// 83W CFL-S 4+2 workstation
  {EnumCflS80Watt62CpuId,     8000,    8000,    11200,     0,         0,    0,              0,       0,           0,    15400}, /// 80W CFL-S 6+2 workstation
  {EnumCflS80Watt82CpuId,     8000,    8000,    21000,     0,         0,    0,              0,       0,           0,    26600}, /// 80W CFL-S 8+2 workstation
  {EnumCflS95Watt42CpuId,     9500,    9500,    10000,     0,         0,    0,              0,       0,           0,    15100}, /// 95W CFL-S 4+2
  {EnumCflS95Watt62CpuId,     9500,    9500,    13100,     0,         0,    0,              0,       0,           0,    16400}, /// 95W CFL-S 6+2
  {EnumCflS95Watt82CpuId,     9500,    9500,    21000,     0,         0,    0,              0,       0,           0,    26600}, /// 95W CFL-S 8+2
  {EnumCflH45Watt4f2CpuId,    4500,    4500,     7800,     0,      7800,    0,           7800,       0,        7800,    10500}, /// 45W CFL-H 4+2
  {EnumCflH45Watt62CpuId,     4500,    4500,     9000,     0,      9000,    0,           9000,       0,        9000,    13000}, /// 45W CFL-H 6+2
  {EnumCflH45Watt82CpuId,     4500,    4500,    10700,     0,     10700,    0,          10700,       0,       10700,    16300}, /// 45W CFL-H 8+2
  {EnumCflH65Watt82CpuId,     6500,    6500,    15600,     0,     15600,    0,          15600,       0,       15600,    20700}, /// 65W CFL-H 8+2
  {EnumCflH45Watt4f2CpuId,    4500,    4500,     7800,     0,         0,    0,              0,       0,           0,    10500}, /// 45W CFL-H 4+2
  {EnumCflH45Watt62CpuId,     4500,    4500,     9000,     0,         0,    0,              0,       0,           0,    13000}, /// 45W CFL-H 6+2
  {EnumCflH65Watt62CpuId,     6500,    6500,    12000,     0,         0,    0,              0,       0,           0,    15600}, /// 65W CFL-H 6+2
  {EnumCflH65Watt42CpuId,     6500,    6500,     7800,     0,         0,    0,              0,       0,           0,    11000}, /// 65W CFL-H 4+2
  {EnumCflU28Watt43eCpuId,    2800,    2800,     6400,     0,      6400,    0,           6400,       0,        6400,     8400}, /// 28W CFL-U 4+3E
  {EnumCflUR15Watt43eCpuId,   1500,    1500,     5500,     0,      5500,    0,           5500,       0,        5500,     7800}, /// 15W CFL-UR 4+3e
  {EnumCflUR28Watt43eCpuId,   2800,    2800,     6400,     0,      6400,    0,           6400,       0,        6400,     8400}, /// 28W CFL-UR 4+3e
  {EnumCnlY9Watt22CpuId,       900,     900,     3300,     0,      3300,    0,           3300,       0,        3300,     5200}, /// 9W  CNL-Y 2+2
  {EnumCnlU15Watt22CpuId,     1500,    1500,     3600,     0,      3600,    0,           3600,       0,        3600,     4700}, /// 15W CNL-U 2+2
  {EnumCnlH45Watt82CpuId,     4500,    4500,    12000,     0,         0,    0,              0,       0,           0,    21000}, /// 45W CNL-H 8+2
  {EnumCnlH65Watt82CpuId,     6500,    6500,    17000,     0,         0,    0,              0,       0,           0,    23000}, /// 65W CNL-H 8+2
  {EnumCnlH95Watt82CpuId,     9500,    9500,    20000,     0,         0,    0,              0,       0,           0,    27000}, /// 95W CNL-H 8+2
  {EnumWhlU15Watt22fCpuId,    1500,    1500,     2900,     0,      2900,    0,           2900,       0,        2900,     5100}, /// 15W WHL-U 2f+2
  {EnumWhlU15Watt2f1fCpuId,   1500,    1500,     2900,     0,      2900,    0,           2900,       0,        2900,     5100}, /// 15W WHL-U 2f+1f
  {EnumWhlU15Watt42fCpuId,    1500,    1500,     5100,     0,      5100,    0,           5100,       0,        5100,     7100}  /// 15W WHL-U 4+2
};

/**
  Configurable TDP BIOS Initialization

  @exception EFI_UNSUPPORTED  Ctdp not Supported
  @retval EFI_SUCCESS         Ctdp Initiation done
**/
EFI_STATUS
InitializeConfigurableTdp (
  VOID
  )
{
  EFI_STATUS Status;
  UINTN      Index;

  if (gCpuNvsAreaConfig == NULL) {
    DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Require gCpuNvsAreaConfig.\n"));
    return EFI_UNSUPPORTED;
  }
  ///
  /// Intialize PPM Global NVS with custom CTDP level settings or CPU provided.
  ///
  Status = InitConfigurableTdpSettings ();
  if (Status != EFI_SUCCESS) {
    return EFI_UNSUPPORTED;
  }

  ///
  /// In case of LFM == TDP Down Ratio/Tdp Nominal , consider TDP Down TAR as the new LFM to insert fake P state.
  ///
  for (Index = 0; Index < (gCpuNvsAreaConfig->Area->CtdpLevelsSupported); Index++) {
    if (mMinBusRatio == gCpuNvsAreaConfig->Area->CtdpTar[Index] + 1) {
      mMinBusRatio = gCpuNvsAreaConfig->Area->CtdpTar[Index];
      DEBUG ((DEBUG_INFO, "PPM:: mMinBusRatio Modified for Ctdp %d\n", mMinBusRatio));
    }
  }

  return EFI_SUCCESS;
}

/**
  Verify and fix Custom Power Limit values

  @param[in] CustomPowerLimit  Custom Power Limit value
  @param[in] CustomPlUnit  Custom Power Limit Unit
**/
UINT16
VerifyAndFixCustomPowerLimit (
  IN UINT32 CustomPowerLimit,
  IN UINT16 CustomPlUnit
  )
{
  UINT16 ConvertedPowerLimit;
  UINT16 CpuConvertedPowerLimitMaxLimit;

  ConvertedPowerLimit = (UINT16) ((CustomPowerLimit * mProcessorPowerUnit) / CustomPlUnit);
  if (mPackageMaxPower == 0 && ConvertedPowerLimit >= mPackageMinPower) {
    ///
    ///  If PACKAGE_POWER_SKU_MSR [46:32] = 0 means there is no upper limit ( since this field is 15 bits, the max value is 2^15 - 1 )
    ///
    CpuConvertedPowerLimitMaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
    if (ConvertedPowerLimit > CpuConvertedPowerLimitMaxLimit) {
      ///
      /// If new Power Limit 1 is > CpuConvertedPowerLimit1MaxLimit, program Power Limit 1 to CpuConvertedPowerLimit1MaxLimit
      ///
      ConvertedPowerLimit = CpuConvertedPowerLimitMaxLimit;
    }
  } else if (mPackageMinPower == 0 && ConvertedPowerLimit > 0 && ConvertedPowerLimit <= mPackageMaxPower) {
    ///
    ///  If PACKAGE_POWER_SKU_MSR [30:16] = 0 means there is no lower limit
    ///
    ConvertedPowerLimit = (UINT16) ((CustomPowerLimit * mProcessorPowerUnit) / CustomPlUnit);
  } else {
    ///
    /// Power Limit 1 needs to be between mPackageMinPower and mPackageMaxPower
    ///
    CpuConvertedPowerLimitMaxLimit = mPackageMaxPower;
    if (ConvertedPowerLimit < mPackageMinPower) {
      ///
      /// If new Power Limit 1 is < mPackageMinPower, program Power Limit 1 to mPackageMinPower
      ///
      ConvertedPowerLimit = mPackageMinPower;
    } else if (ConvertedPowerLimit > CpuConvertedPowerLimitMaxLimit) {
      ///
      /// If new Power Limit 1 is > mPackageMaxPower, program Power Limit 1 to mPackageMaxPower
      ///
      ConvertedPowerLimit = CpuConvertedPowerLimitMaxLimit;
    }
  }

  return ConvertedPowerLimit;
}

/**
  Verify and fix Custom Ratio values
  Custom Ratio should be between MaxTurboFrequency and LFM

  @param[in] CustomRatio  Custom Ratio value
**/
UINT8
VerifyAndFixCustomRatio (
  IN UINT8 CustomRatio
  )
{
  if (CustomRatio > mTurboBusRatio) {
    ///
    /// Use HFM as max value if Turbo is not supported
    ///
    if (mTurboBusRatio == 0) {
      CustomRatio = (UINT8) mMaxBusRatio;
    } else {
      CustomRatio = (UINT8) mTurboBusRatio;
    }
  } else if (CustomRatio < mMinBusRatio) {
    ///
    /// Use LFM as min value
    ///
    CustomRatio = (UINT8) mMinBusRatio;
  }

  return CustomRatio;
}

/**
  Initalizes CTDP BIOS settings from silicon defaults and overrides custom cTDP settings if needed

  @exception EFI_UNSUPPORTED  Ctdp not supported
  @retval EFI_SUCCESS         Ctdp Settings Initialized successfully from MSRs
**/
EFI_STATUS
InitConfigurableTdpSettings (
  VOID
  )
{
  MSR_REGISTER TempMsr;
  UINTN        Index;
  UINT16       CpuConfigTdpNominalTdp;
  UINT16       CpuConfigTdpLevel1Tdp;
  UINT16       CpuConfigTdpLevel2Tdp;
  UINT8        CpuConfigTdpNominalRatio;
  UINT8        CpuConfigTdpLevel1Ratio;
  UINT8        CpuConfigTdpLevel2Ratio;
  UINT16       CpuConfigTdpLevels;

  ///
  /// Get the number of configurable TDP Levels supported
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_INFO);
  TempMsr.Qword &= V_CONFIG_TDP_NUM_LEVELS_MASK;
  CpuConfigTdpLevels = (UINT8) RShiftU64 (TempMsr.Qword, N_MSR_PLATFORM_INFO_CONFIG_TDP_NUM_LEVELS_OFFSET);
  DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Supported Levels=%d\n", CpuConfigTdpLevels));
  ///
  /// Return if ConfigTDP Levels not supported
  ///
  if (CpuConfigTdpLevels == 0) {
    DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Levels not supported\n"));
    return EFI_UNSUPPORTED;
  }
  gCpuNvsAreaConfig->Area->CtdpLevelsSupported    = (UINT8) CpuConfigTdpLevels + 1;
  gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex = (UINT8) gCpuPowerMgmtTestConfig->ConfigTdpLevel;
  ///
  /// Get PKG_TDP for Config TDP Nominal
  ///
  TempMsr.Qword             = AsmReadMsr64 (MSR_CONFIG_TDP_NOMINAL);
  CpuConfigTdpNominalRatio  = (UINT8) (TempMsr.Dwords.Low & CONFIG_TDP_NOMINAL_RATIO_MASK);
  CpuConfigTdpNominalTdp    = mPackageTdp;
  DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Nominal Ratio=%d Tdp=%d\n", CpuConfigTdpNominalRatio, CpuConfigTdpNominalTdp));
  ///
  /// Set Level0 as Tdp Nominal
  ///
  gCpuNvsAreaConfig->Area->CtdpPowerLimit1[0] = mPackageTdp;
  gCpuNvsAreaConfig->Area->CtdpPowerLimit2[0] = GetCtdpPowerLimit2 (gCpuNvsAreaConfig->Area->CtdpPowerLimit1[0]);
  gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[0] = POWER_LIMIT1_TIME_DEFAULT;
  gCpuNvsAreaConfig->Area->CtdpTar[0] = (UINT8) (CpuConfigTdpNominalRatio - 1);
  gCpuNvsAreaConfig->Area->CtdpCtc[0] = CONFIG_TDP_NOMINAL;
  ///
  /// Get PKG_TDP and Ratio for Config TDP Level1
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_CONFIG_TDP_LVL1);
  CpuConfigTdpLevel1Ratio = (UINT8) RShiftU64 (
                                      TempMsr.Qword & CONFIG_TDP_LVL1_RATIO_MASK,
                                      CONFIG_TDP_LVL1_RATIO_OFFSET
                                      );
  CpuConfigTdpLevel1Tdp = (UINT16) (TempMsr.Dwords.Low & CONFIG_TDP_LVL1_PKG_TDP_MASK);
  DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Level1 Ratio=%d Tdp=%d\n", CpuConfigTdpLevel1Ratio, CpuConfigTdpLevel1Tdp));
  ///
  /// Set Level 1
  ///
  Index = 1;
  if (CpuConfigTdpLevel1Ratio != 0) {
    gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index] = CpuConfigTdpLevel1Tdp;
    gCpuNvsAreaConfig->Area->CtdpPowerLimit2[Index] = GetCtdpPowerLimit2 (gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index]);
    gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[Index] = gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[0];
    gCpuNvsAreaConfig->Area->CtdpTar[Index] = (UINT8) (CpuConfigTdpLevel1Ratio - 1);
    gCpuNvsAreaConfig->Area->CtdpCtc[Index] = CONFIG_TDP_LEVEL1;
    Index++;
  }
  ///
  /// If two levels are supported or Level1 was not valid
  /// then read Level2 registers
  ///
  if (CpuConfigTdpLevels == CONFIG_TDP_LEVEL2 || CpuConfigTdpLevel1Ratio == 0) {
    ///
    /// Get PKG_TDP and Ratio for Config TDP Level2
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_CONFIG_TDP_LVL2);
    CpuConfigTdpLevel2Ratio = (UINT8) RShiftU64 (
                                        TempMsr.Qword & CONFIG_TDP_LVL2_RATIO_MASK,
                                        CONFIG_TDP_LVL2_RATIO_OFFSET
                                        );
    CpuConfigTdpLevel2Tdp = (UINT16) (TempMsr.Dwords.Low & CONFIG_TDP_LVL2_PKG_TDP_MASK);
    DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP Level2 Ratio=%d Tdp=%d\n", CpuConfigTdpLevel2Ratio, CpuConfigTdpLevel2Tdp));
    ///
    /// Set Level2
    ///
    gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index] = CpuConfigTdpLevel2Tdp;
    gCpuNvsAreaConfig->Area->CtdpPowerLimit2[Index] = GetCtdpPowerLimit2 (gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index]);
    gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[Index] = gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[0];
    gCpuNvsAreaConfig->Area->CtdpTar[Index] = (UINT8) (CpuConfigTdpLevel2Ratio - 1);
    gCpuNvsAreaConfig->Area->CtdpCtc[Index] = CONFIG_TDP_LEVEL2;
  }

  ///
  /// Override any custom ConfigTdp information if applicable. This will only update if a custom
  /// setting change is detected, otherwise the CPU based default cTDP  settings will be applied.
  ///
  for (Index = 0; Index < MAX_CUSTOM_CTDP_ENTRIES; Index++) {
    ///
    /// Verify and fix Custom configured CTDP Levels PL1
    ///
    if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1 != 0) {
      gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1 = VerifyAndFixCustomPowerLimit (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1,mCustomPowerUnit);
      gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index] = (UINT16) gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1;
    }

    ///
    /// Ctdp PL1 Time Window
    ///
    if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1Time != 0) {
      gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[Index] = (UINT8) gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1Time;
    }

    ///
    /// Verify and fix Custom configured CTDP Levels PL2
    ///
    if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit2 != 0) {
      gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit2 = VerifyAndFixCustomPowerLimit (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit2,mCustomPowerUnit);
      gCpuNvsAreaConfig->Area->CtdpPowerLimit2[Index] = (UINT16) gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit2;
    }

    ///
    /// cTDP Turbo Activation Ratio
    ///
    if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomTurboActivationRatio != 0) {
      gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomTurboActivationRatio = VerifyAndFixCustomRatio ((UINT8) gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomTurboActivationRatio+1)-1;
      gCpuNvsAreaConfig->Area->CtdpTar[Index] = (UINT8) gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomTurboActivationRatio;
    }
  }

  return EFI_SUCCESS;
}

/**
  Get Power Limit2 based on Power Limit1 on Config TDP

  @param[in] PowerLimit1  Power Limit 1 Value

  @retval Calculated Power Limit2 value
**/
UINT16
GetCtdpPowerLimit2 (
  IN UINT16 PowerLimit1
  )
{
  UINT16 ConvertedPowerLimit2;
  UINT16 Multiplier;

  ///
  /// By default,for Mobile & Desktop Processors: Short duration Power Limit  = 1.25 * Package TDP
  ///
  Multiplier = 125;
  ///
  /// For XE/non-ULV skus Configure PL2 as (1.25 x cTDP).
  ///
  ConvertedPowerLimit2 = EFI_IDIV_ROUND ((Multiplier * PowerLimit1), 100);

  return ConvertedPowerLimit2;
}

/**
  Patch Fvid Table with Ctdp Tar ratio and Tar-1 Ratio

  @param[in] FvidPointer  Pointer to Fvid Table
**/
VOID
CtdpPatchFvidTable (
  IN OUT FVID_TABLE *FvidPointer
  )
{
  UINTN PssIndex;
  UINTN Index;
  UINTN TempRatio;

  ///
  /// Check and patch Fvid table for TAR ratios
  ///
  for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
    TempRatio = gCpuNvsAreaConfig->Area->CtdpTar[Index];
    ///
    /// Fvid Table's first entry is Header only, then state ratios start from the second entry.
    /// Size of table is EistStates plus one, but need to stop iterating one entry early, due to use of PssIndex + 1.
    ///
    for (PssIndex = 1; PssIndex < FvidPointer[0].FvidHeader.EistStates; PssIndex++) {
      if (FvidPointer[PssIndex + 1].FvidState.BusRatio < TempRatio) {
        if (FvidPointer[PssIndex].FvidState.BusRatio != TempRatio) {
          ///
          /// If TAR not Found, Replace Turbo Activation Ratio at PssIndex
          ///
          CtdpReplaceFvidRatio (FvidPointer, PssIndex, TempRatio);
          DEBUG ((DEBUG_INFO, " TAR Ratio Replace at %x with %x \n", PssIndex, TempRatio));
        }
        break;
      }
    }
  }
  ///
  /// Check and patch Fvid table for CTDP ratios.
  /// This is done separately to make sure Ctdp ratios are not override by TAR ratios
  /// when CTDP ratios are adjacent.
  ///
  for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
    TempRatio = gCpuNvsAreaConfig->Area->CtdpTar[Index] + 1;
    ///
    /// Fvid Table's first entry is Header only, then state ratios start from the second entry.
    /// Size of table is EistStates plus one, but need to stop iterating one entry early, due to use of PssIndex + 1.
    ///
    for (PssIndex = 1; PssIndex < FvidPointer[0].FvidHeader.EistStates; PssIndex++) {
      if (FvidPointer[PssIndex + 1].FvidState.BusRatio == TempRatio) {
        gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex + 1].FvidState.State;
      }

      if (FvidPointer[PssIndex + 1].FvidState.BusRatio < TempRatio) {
        if (FvidPointer[PssIndex].FvidState.BusRatio == TempRatio) {
          ///
          /// Found Turbo Activation Ratio + 1 at PssIndex
          ///
          gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex].FvidState.State;
          break;
        } else {
          ///
          /// If TAR+1 not Found, Replace Turbo Activation Ratio + 1 at PssIndex
          ///
          CtdpReplaceFvidRatio (FvidPointer, PssIndex, TempRatio);
          gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex].FvidState.State;
          DEBUG ((DEBUG_INFO, " CTDP Ratio Replace at %x with %x \n", PssIndex, TempRatio));
          break;
        }
      }
    }
  }
}

/**
  Patch Fvid Table with Ctdp Tar ratio and Tar-1 Ratio for Legacy OS

  @param[in] FvidPointer  Pointer to Fvid Table
**/
VOID
CtdpPatchFvidTableforLimitPstate (
  IN OUT FVID_TABLE *FvidPointer
  )
{
  UINTN PssIndex;
  UINTN Index;
  UINTN TempRatio;

  ///
  /// Check and patch Fvid table for TAR ratios
  ///
  for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
    TempRatio = gCpuNvsAreaConfig->Area->CtdpTar[Index];
    ///
    /// Fvid Table's first entry is Header only, then state ratios start from the second entry.
    /// Size of table is EistStates plus one, but need to stop iterating one entry early, due to use of PssIndex + 1.
    ///
    for (PssIndex = 1; PssIndex < LPSS_FVID_MAX_STATES; PssIndex++) {
      if (FvidPointer[PssIndex + 1].FvidState.Limit16BusRatio < TempRatio) {
        if (FvidPointer[PssIndex].FvidState.Limit16BusRatio != TempRatio) {
          ///
          /// If TAR not Found, Replace Turbo Activation Ratio at PssIndex
          ///
          CtdpReplaceFvidRatio (FvidPointer, PssIndex, TempRatio);
          DEBUG ((DEBUG_INFO, " TAR Ratio Replace at %x with %x \n", PssIndex, TempRatio));
        }
        break;
      }
    }
  }
  ///
  /// Check and patch Fvid table for CTDP ratios.
  /// This is done separately to make sure Ctdp ratios are not override by TAR ratios
  /// when CTDP ratios are adjacent.
  ///
  for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
    TempRatio = gCpuNvsAreaConfig->Area->CtdpTar[Index] + 1;
    ///
    /// Fvid Table's first entry is Header only, then state ratios start from the second entry.
    /// Size of table is EistStates plus one, but need to stop iterating one entry early, due to use of PssIndex + 1.
    ///
    for (PssIndex = 1; PssIndex < LPSS_FVID_MAX_STATES; PssIndex++) {
      if (FvidPointer[PssIndex + 1].FvidState.Limit16BusRatio == TempRatio) {
        gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex + 1].FvidState.Limit16State;
      }

      if (FvidPointer[PssIndex + 1].FvidState.Limit16BusRatio < TempRatio) {
        if (FvidPointer[PssIndex].FvidState.Limit16BusRatio == TempRatio) {
          ///
          /// Found Turbo Activation Ratio + 1 at PssIndex
          ///
          gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex].FvidState.Limit16State;
          break;
        } else {
          ///
          /// If TAR+1 not Found, Replace Turbo Activation Ratio + 1 at PssIndex
          ///
          CtdpReplaceFvidRatio (FvidPointer, PssIndex, TempRatio);
          gCpuNvsAreaConfig->Area->CtdpPpc[Index] = (UINT8) FvidPointer[PssIndex].FvidState.Limit16State;
          DEBUG ((DEBUG_INFO, " CTDP Ratio Replace at %x with %x \n", PssIndex, TempRatio));
          break;
        }
      }
    }
  }
}

/**
  Replace P state with given ratio

  @param[in out] FvidPointer  Pointer to Fvid Table
  @param[in]     PssIndex     FVID table index of P state to be replaced
  @param[in]     Ratio        Target Ratio to put in
**/
VOID
CtdpReplaceFvidRatio (
  IN OUT FVID_TABLE *FvidPointer,
  UINTN             PssIndex,
  UINTN             Ratio
  )
{
  UINT64 wPower1;
  UINT64 wPower2;

  ///
  /// Replace new Ratio
  ///
  if (mOver16Pstates) {
    FvidPointer[PssIndex].FvidState.Limit16BusRatio = (UINT16) Ratio;
  } else {
    FvidPointer[PssIndex].FvidState.BusRatio = (UINT16) Ratio;
  }
  ///
  /// Calculate relative Power
  ///
  if (mOver16Pstates) {
    wPower1 = (mMaxBusRatio - FvidPointer[PssIndex].FvidState.Limit16BusRatio) * 625;
  } else {
    wPower1 = (mMaxBusRatio - FvidPointer[PssIndex].FvidState.BusRatio) * 625;
  }
  wPower1 = (110000 - wPower1);
  wPower1 = DivU64x32 (wPower1, 11);
  wPower1 = MultU64x64 (wPower1, wPower1);
  //
  // Power is calculated in milliwatts
  //
  if (mOver16Pstates) {
    wPower2 = (((FvidPointer[PssIndex].FvidState.Limit16BusRatio * 100) / mMaxBusRatio));
    wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, wPower1), mPackageTdpWatt), 10000000);
    FvidPointer[PssIndex].FvidState.Limit16Power = (UINT32) wPower2;
  } else {
    wPower2 = (((FvidPointer[PssIndex].FvidState.BusRatio * 100) / mMaxBusRatio));
    wPower2 = DivU64x32 (MultU64x32 (MultU64x64 (wPower2, wPower1), mPackageTdpWatt), 10000000);
    FvidPointer[PssIndex].FvidState.Power = (UINT32) wPower2;
  }
}

/**
  Configures following fields of MSR 0x610 based on user configuration:
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration turbo mode (power limit 2)
**/
VOID
ConfigurePowerLimitsNonConfigTdpSkus (
  VOID
  )
{
  MSR_REGISTER            PackagePowerLimitMsr;
  UINT16                  ConvertedPowerLimit1;
  UINT8                   ConvertedPowerLimit1Time;
  UINT16                  ConvertedShortDurationPowerLimit;
  UINT16                  CpuConvertedPowerLimit1MaxLimit;
  UINT16                  CpuConvertedPowerLimit2MaxLimit;
  UINT16                  Multiplier;
  UINTN                   MchBar;
  PPM_OVERRIDE_TABLE      *PpmOverrideTable;
  UINTN                   NoOfOverrides;
  UINTN                   Index;

  ///
  /// Override table parameters
  ///
  PpmOverrideTable = mPowerLimitsOverrideTable;

  if (sizeof (PPM_OVERRIDE_TABLE) != 0) {
    NoOfOverrides = (sizeof (mPowerLimitsOverrideTable)) / (sizeof (PPM_OVERRIDE_TABLE));
  }

  CpuConvertedPowerLimit1MaxLimit = 0;
  CpuConvertedPowerLimit2MaxLimit = 0;
  ConvertedPowerLimit1Time        = 0;
  ///
  /// By default, for Mobile & Desktop Processors: Short duration Power Limit  = 1.25 * Package TDP
  ///
  Multiplier = 125;
  ///
  ///  Check if TDP limits are programmable
  ///   - Platform Info MSR (0xCE) [29]
  ///
  if (mTdpLimitProgrammble) {
    PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
    ///
    /// Initialize the Power Limit 1 and Power Limit 1 enable bit
    ///  - Power Limit 1: Turbo Power Limit MSR  [14:0]
    ///  - Power Limit 1 Enable: Turbo Power Limit MSR  [15]
    ///
    ///
    /// By default, program Power Limit 1 to Package TDP limit
    ///
    ConvertedPowerLimit1 = mPackageTdp;
    if (gCpuPowerMgmtBasicConfig->PowerLimit1 != AUTO) {
      ///
      /// gCpuPowerMgmtBasicConfig->PowerLimit1 is in mW or watts. We need to convert it to
      /// CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      /// Since we are converting from Watts to CPU power units, multiply by
      /// PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      /// Refer to BWG 14.13.7 for Power Limit 1 limits.
      ///
      ConvertedPowerLimit1 = (UINT16) ((gCpuPowerMgmtBasicConfig->PowerLimit1 * mProcessorPowerUnit) / mCustomPowerUnit);
      if (mPackageMaxPower == 0 && ConvertedPowerLimit1 >= mPackageMinPower) {
        ///
        ///  If PACKAGE_POWER_SKU_MSR [46:32] = 0 means there is no upper limit ( since this field is 15 bits, the max value is 2^15 - 1 )
        ///
        CpuConvertedPowerLimit1MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
        if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
          ///
          /// If new Power Limit 1 is > CpuConvertedPowerLimit1MaxLimit, program Power Limit 1 to CpuConvertedPowerLimit1MaxLimit
          ///
          ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
        }
      } else if (mPackageMinPower == 0 && ConvertedPowerLimit1 > 0 && ConvertedPowerLimit1 <= mPackageMaxPower) {
        ///
        ///  If PACKAGE_POWER_SKU_MSR [30:16] = 0 means there is no lower limit
        ///
        ConvertedPowerLimit1 = (UINT16) ((gCpuPowerMgmtBasicConfig->PowerLimit1 * mProcessorPowerUnit) / mCustomPowerUnit);

      } else {
        ///
        /// Power Limit 1 needs to be between mPackageMinPower and mPackageMaxPower
        ///
        CpuConvertedPowerLimit1MaxLimit = mPackageMaxPower;

        if (ConvertedPowerLimit1 < mPackageMinPower) {
          ///
          /// If new Power Limit 1 is < mPackageMinPower, program Power Limit 1 to mPackageMinPower
          ///
          ConvertedPowerLimit1 = mPackageMinPower;
        } else if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
          ///
          /// If new Power Limit 1 is > mPackageMaxPower, program Power Limit 1 to mPackageMaxPower
          ///
          ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
        }
      }
    } else {
      DEBUG ((DEBUG_INFO, "Power Limit 1 = AUTO\n"));
      for (Index = 0; Index < NoOfOverrides; Index++, PpmOverrideTable++) {
        if ( (PpmOverrideTable->CpuIdentifier == mCpuIdentifier) && (PpmOverrideTable->MsrPowerLimit1 != 0) ) {
          ConvertedPowerLimit1 = (UINT16) (PpmOverrideTable->MsrPowerLimit1 / 100) * 8;
          break;
        }
      }
    }
    PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
    PackagePowerLimitMsr.Dwords.Low |= (UINT32) ((ConvertedPowerLimit1) & POWER_LIMIT_MASK);
    DEBUG (
            (DEBUG_INFO,
             "New Power Limit 1  %d watt (%d in CPU power unit)\n",
             gCpuPowerMgmtBasicConfig->PowerLimit1,
             ConvertedPowerLimit1)
            );
    ///
    /// Force Power Limit 1 override to be enabled
    ///
    PackagePowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
    ///
    /// Program Power Limit 1 (Long Duration Turbo) Time Window
    ///  If PowerLimit1Time is AUTO OR If PowerLimit1Time is > MAX_POWER_LIMIT_1_TIME_IN_SECONDS
    ///    program default values
    ///
    if ((gCpuPowerMgmtBasicConfig->PowerLimit1Time == AUTO) ||
        (gCpuPowerMgmtBasicConfig->PowerLimit1Time > MAX_POWER_LIMIT_1_TIME_IN_SECONDS)
        ) {
      gCpuPowerMgmtBasicConfig->PowerLimit1Time = POWER_LIMIT1_TIME_DEFAULT;
    }
    ConvertedPowerLimit1Time = GetConvertedTime (gCpuPowerMgmtBasicConfig->PowerLimit1Time, SecondsTimeWindowConvert);
    ///
    ///  Configure Power Limit 1 (Long Duration Turbo) time windows: PACKAGE_POWER_LIMIT_MSR 0x610 [23:17]
    ///
    PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
    PackagePowerLimitMsr.Dwords.Low |= (UINT32) (LShiftU64 (ConvertedPowerLimit1Time, 17) & POWER_LIMIT_1_TIME_MASK);

    ///
    /// Configure Power Limit 2 : PACKAGE_POWER_LIMIT_MSR 0x610 [46:32]
    /// gCpuPowerMgmtBasicConfig->PowerLimit2Power value is in mW or watts. We need to convert it to
    /// CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
    /// Since we are converting from Watts to CPU power units, multiply by
    /// PACKAGE_POWER_SKU_UNIT_MSR[3:0]
    ///
    ConvertedShortDurationPowerLimit = (UINT16) ((gCpuPowerMgmtBasicConfig->PowerLimit2Power * mProcessorPowerUnit) / mCustomPowerUnit);
    PackagePowerLimitMsr.Dwords.High &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
    ///
    /// If PowerLimit2 is AUTO OR if PowerLimit2 is > mPackageMaxPower OR if PowerLimit2 < mPackageMinPower
    /// program defaul values.
    ///
    CpuConvertedPowerLimit2MaxLimit = mPackageMaxPower;
    if (CpuConvertedPowerLimit2MaxLimit == 0) {
      CpuConvertedPowerLimit2MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
    }
    if (gCpuPowerMgmtBasicConfig->PowerLimit2Power == AUTO) {
      ConvertedShortDurationPowerLimit = EFI_IDIV_ROUND ((Multiplier * mPackageTdp), 100);
      ///
      /// If Power Limit 2 is set to AUTO, then program PL2 defaults by sku
      ///
      DEBUG ((DEBUG_INFO, "Power Limit 2 overrides\n"));
      PpmOverrideTable = mPowerLimitsOverrideTable;
      for (Index = 0; Index < NoOfOverrides; Index++, PpmOverrideTable++) {
        if ((PpmOverrideTable->CpuIdentifier == mCpuIdentifier) && (PpmOverrideTable->MsrPowerLimit2 != 0)) {
          DEBUG ((DEBUG_INFO, "Power Limit 2 override found! PpmOverrideTable->MsrPowerLimit2 = %d\n", PpmOverrideTable->MsrPowerLimit2));
          ConvertedShortDurationPowerLimit = (UINT16) (PpmOverrideTable->MsrPowerLimit2 / 100) * 8;
          break;
        }
      }
    }
    if (ConvertedShortDurationPowerLimit > CpuConvertedPowerLimit2MaxLimit) {
      ConvertedShortDurationPowerLimit = CpuConvertedPowerLimit2MaxLimit;
    }
    if (ConvertedShortDurationPowerLimit < mPackageMinPower) {
      ConvertedShortDurationPowerLimit = mPackageMinPower;
    }
    PackagePowerLimitMsr.Dwords.High |= (UINT32) ((ConvertedShortDurationPowerLimit) & POWER_LIMIT_MASK);

    if (gCpuPowerMgmtBasicConfig->PowerLimit2 == TRUE) {
      PackagePowerLimitMsr.Dwords.High |= B_POWER_LIMIT_ENABLE;
    } else {
      ///
      /// When we disable Power Limit 2, we need to write power limit = 0
      ///
      PackagePowerLimitMsr.Dwords.High &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
    }

    DEBUG (
            (DEBUG_INFO,
             "Short duration Power limit enabled, Power Limit = %d Watts\n",
             gCpuPowerMgmtBasicConfig->PowerLimit2Power)
            );

    DEBUG ((DEBUG_INFO,"MSR(610h)=%08X%08X\n",PackagePowerLimitMsr.Dwords.High,PackagePowerLimitMsr.Dwords.Low));

    AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);
  }
  ///
  /// PL1 and PL2 BIOS Overrides for 57W Non CTDP SKU
  ///
  if (mPackageTdp == 57 * mProcessorPowerUnit) {
    PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
    ///
    /// PL1=67W
    ///
    PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
    PackagePowerLimitMsr.Dwords.Low |= (UINT32) ((67 * mProcessorPowerUnit) & POWER_LIMIT_MASK);
    ///
    /// PL2=83.75W
    ///
    PackagePowerLimitMsr.Dwords.High &= ~POWER_LIMIT_MASK;
    PackagePowerLimitMsr.Dwords.High |= (UINT32) (((8375 * mProcessorPowerUnit) / 100) & POWER_LIMIT_MASK);

    AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);
  }
  ///
  /// Set PACKAGE_POWER_LIMIT.CRITICAL_POWER_CLAMP_1(bit 16)
  ///
  PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
  PackagePowerLimitMsr.Dwords.Low |= (UINT32) B_CRITICAL_POWER_CLAMP_ENABLE;
  AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);

  ///
  /// Get the MCH space base address.
  ///
  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;

  ///
  /// Program MMIO PL1 clamp enable
  ///
  MmioOr32 (MchBar + MMIO_TURBO_POWER_LIMIT, B_CRITICAL_POWER_CLAMP_ENABLE);

  ///
  /// Pass the power limits of the non-CTDP part to the Global NVS Area for use by DPTF
  ///
  PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
  gCpuNvsAreaConfig->Area->CtdpPowerLimit1[0]      = (UINT16) (PackagePowerLimitMsr.Dwords.Low & POWER_LIMIT_MASK);
  gCpuNvsAreaConfig->Area->CtdpPowerLimit2[0]      = (UINT16) (PackagePowerLimitMsr.Dwords.High & POWER_LIMIT_MASK);
  gCpuNvsAreaConfig->Area->CtdpPowerLimitWindow[0] = (UINT8) gCpuPowerMgmtBasicConfig->PowerLimit1Time;
  gCpuNvsAreaConfig->Area->CtdpTar[0]              = (UINT8) mTurboBusRatio;
  gCpuNvsAreaConfig->Area->CtdpCtc[0]              = 1;
  gCpuNvsAreaConfig->Area->CtdpLevelsSupported     = 1;
  gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex  = 0;
}

/**
  Configures following fields of MSR 0x615
    Configures power limit 3 power level and time window
**/
VOID
ConfigurePl3PowerLimits (
  VOID
  )
{
  MSR_REGISTER PlatformPowerLimitMsr;
  UINT16       ConvertedPowerLimit3;
  UINT8        ConvertedPowerLimit3Time;

  ///
  /// PL3 is supported
  ///

  PlatformPowerLimitMsr.Qword = AsmReadMsr64 (MSR_PL3_CONTROL);
  DEBUG ((DEBUG_INFO," PL3 MSR 615 Before Writing %x \n",PlatformPowerLimitMsr.Dwords.Low));
  ///
  /// Configure PL3 Power Limit if custom value is avaiable
  ///
  if (gCpuPowerMgmtBasicConfig->PowerLimit3 != AUTO) {
    ConvertedPowerLimit3 = (UINT16) (((gCpuPowerMgmtBasicConfig->PowerLimit3 * mProcessorPowerUnit) / mCustomPowerUnit) & POWER_LIMIT_MASK);
    PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
    PlatformPowerLimitMsr.Dwords.Low |= (UINT32) (ConvertedPowerLimit3);
    PlatformPowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  }

  ///
  /// Configure PL3 Time window if custom value is avaiable
  ///
  if (gCpuPowerMgmtBasicConfig->PowerLimit3Time != AUTO) {
    ConvertedPowerLimit3Time = GetConvertedTime (gCpuPowerMgmtBasicConfig->PowerLimit3Time, MilliSecondsTimeWindowConvert);
    PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_3_TIME_MASK;
    PlatformPowerLimitMsr.Dwords.Low |= (UINT32) (LShiftU64 (ConvertedPowerLimit3Time, 17) & POWER_LIMIT_3_TIME_MASK);
    PlatformPowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  }

  ///
  /// Configure PL3 Duty Cycle if custom value is avaiable
  ///
  if (gCpuPowerMgmtBasicConfig->PowerLimit3DutyCycle != AUTO) {
    PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_3_DUTY_CYCLE_MASK;
    PlatformPowerLimitMsr.Dwords.Low |= (UINT32) (LShiftU64 (gCpuPowerMgmtBasicConfig->PowerLimit3DutyCycle, 24) & POWER_LIMIT_3_DUTY_CYCLE_MASK);
    PlatformPowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  }

  //
  // Enable/Disable PL3 lock
  //
  if (gCpuPowerMgmtBasicConfig->PowerLimit3Lock == TRUE) {
    PlatformPowerLimitMsr.Dwords.Low |= (UINT32) B_POWER_LIMIT_LOCK;
  } else {
    PlatformPowerLimitMsr.Dwords.Low &= (~((UINT32) B_POWER_LIMIT_LOCK));
  }

  if ((gCpuPowerMgmtBasicConfig->PowerLimit3DutyCycle == AUTO) &&
      (gCpuPowerMgmtBasicConfig->PowerLimit3Time == AUTO) &&
      (gCpuPowerMgmtBasicConfig->PowerLimit3 == AUTO)) {
    //
    // Explicitly disable PL3 if all options are set to AUTO
    //
    PlatformPowerLimitMsr.Dwords.Low &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
  }

  AsmWriteMsr64 (MSR_PL3_CONTROL, PlatformPowerLimitMsr.Qword);
  DEBUG ((DEBUG_INFO," PL3 MSR 615 After Writing %x \n",PlatformPowerLimitMsr.Dwords.Low));
}

/**
  Configure PL4 limits by programming the CURRENT LIMIT and LOCK fields of MSR 601
**/
VOID
ConfigurePl4PowerLimits (
  VOID
  )
{
  MSR_REGISTER            PowerLimit4Msr;
  UINT16                  ConvertedPowerLimit4;
  UINTN                   NoOfOverrides;
  UINTN                   Index;
  PPM_OVERRIDE_TABLE      *PpmOverrideTable;

  NoOfOverrides = 0;

  ///
  /// If CFL-S, update Cpu Identifier based on VrPowerDeliveryDesign input
  ///
  if (GetCpuFamily () == EnumCpuCflDtHalo) {
   if ( gCpuPowerMgmtVrConfig->VrPowerDeliveryDesign == EnumUnknownCpuVrPowerDesign) {
     DEBUG ((DEBUG_ERROR, "VR ERROR: VrPowerDeliveryDesign is missing! VR PL4 override values may cause system instability!\n"));
   } else if ( gCpuPowerMgmtVrConfig->VrPowerDeliveryDesign <= EnumMaxCflVrPowerDesign ){
     ///
     /// This field is required to be non-zero on CFL S-based platforms.
     /// Used to communicate the power delivery design of the board.
     /// The VR override code traditionally looked at the CPU sku to determine the default VR
     /// override values. However, the CFL-S platforms require different VR values based on the
     /// board power delivery design. This is due to CPU socket compatibility across different board
     /// designs. Without this field populated, the system may experience VR OCP shutdowns, hangs,
     /// thermal and performance loss.
     ///
     mCpuIdentifier = (CPU_OVERRIDE_IDENTIFIER) gCpuPowerMgmtVrConfig->VrPowerDeliveryDesign;
   }
  }

  PowerLimit4Msr.Qword = AsmReadMsr64 (MSR_PL4_CONTROL);
  PowerLimit4Msr.Dwords.Low &= ~V_POWER_LIMIT_4_MASK;
  DEBUG ((DEBUG_INFO," PL4 MSR 601 Before Writing %x \n", PowerLimit4Msr.Dwords.Low));

  if (gCpuPowerMgmtBasicConfig->PowerLimit4 != AUTO) {
    ///
    /// User defined PL4
    ///
    ConvertedPowerLimit4 = (UINT16) (((gCpuPowerMgmtBasicConfig->PowerLimit4 * mProcessorPowerUnit) / mCustomPowerUnit) & V_POWER_LIMIT_4_MASK);
    PowerLimit4Msr.Dwords.Low &= ~V_POWER_LIMIT_4_MASK;
    PowerLimit4Msr.Dwords.Low |= (UINT32) (ConvertedPowerLimit4);
  } else {
    //
    // Set PL4 power limit = 0 when AUTO is selected
    //
    if (gCpuOverClockingPreMemConfig->OcSupport == 0) {
      PpmOverrideTable = mPowerLimitsOverrideTable;
      if (sizeof (PPM_OVERRIDE_TABLE) != 0) {
        NoOfOverrides = (sizeof (mPowerLimitsOverrideTable)) / (sizeof (PPM_OVERRIDE_TABLE));
      }
      ///
      /// Override the PL4 power limit with the table value
      ///
      for (Index = 0; Index < NoOfOverrides; Index++, PpmOverrideTable++) {
        if ((PpmOverrideTable->CpuIdentifier == mCpuIdentifier) && (PpmOverrideTable->MsrPowerLimit4 != 0)) {
          ///
          /// We need to check that the CpuIdentifier matches before updating PL4
          ///
          PowerLimit4Msr.Dwords.Low = (((PpmOverrideTable->MsrPowerLimit4 * mProcessorPowerUnit) / 100) & V_POWER_LIMIT_4_MASK);
          break;
        }
      }
    } else {
      ///
      /// Disable PL4 when AUTO is selected and overclocking is enabled.
      ///
      PowerLimit4Msr.Dwords.Low = 0;
    }
  }

  //
  // Enable/Disable PL4 lock
  //
  if (gCpuPowerMgmtBasicConfig->PowerLimit4Lock == TRUE) {
    PowerLimit4Msr.Dwords.Low |= (UINT32) B_POWER_LIMIT_LOCK;
  } else {
    PowerLimit4Msr.Dwords.Low &= (~((UINT32) B_POWER_LIMIT_LOCK));
  }

  AsmWriteMsr64 (MSR_PL4_CONTROL, PowerLimit4Msr.Qword);
  DEBUG ((DEBUG_INFO," PL4 MSR 601 After Writing %x \n ", PowerLimit4Msr.Dwords.Low));
}

/**
  Configures following fields of MSR 0x610
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration turbo mode (power limit 2)
**/
VOID
ConfigureCtdpPowerLimits (
  VOID
  )
{
  MSR_REGISTER PackagePowerLimitMsr;
  UINT16       ConvertedPowerLimit1;
  UINT16       ConvertedPowerLimit2;
  UINT8        ConvertedPowerLimit1Time;
  UINTN        MchBar;
  UINT16       Multiplier;
  UINTN        Index;

  ConvertedPowerLimit1Time = 0;
  ///
  /// By default, for Mobile & Desktop Processors: Short duration Power Limit  = 1.25 * Package TDP
  ///
  Multiplier = 125;
  //
  // For ConfigTdp enabled skus
  //
  PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
  PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
  PackagePowerLimitMsr.Dwords.High &= ~POWER_LIMIT_MASK;
  ///
  /// Initialize the Power Limit 1/2 and Power Limit 2 enable bit in MSR
  ///  Power Limit 1: Turbo Power Limit MSR  [14:0] and Power Limit 2: Turbo Power Limit MSR  [46:32]
  ///  Set MSR value for Power Limit 1/2 to Max Package Power Value or Maximum Supported Value
  ///
  ///
  if (mPackageMaxPower) {
    ConvertedPowerLimit1 = mPackageMaxPower;
    ///
    ///  Short duration Power Limit (PL2)  = 1.25 * PL1
    ///
    ConvertedPowerLimit2 = EFI_IDIV_ROUND ((Multiplier * ConvertedPowerLimit1), 100);
    if (ConvertedPowerLimit2 > PACKAGE_TDP_POWER_MASK) {
      ConvertedPowerLimit2 = PACKAGE_TDP_POWER_MASK;
    }
  } else {
    ///
    /// Set Maximum value for Turbo Power Limit MSR  [14:0] and [46:32] =
    /// Max of CTDP Level Power Limts
    ///
    ConvertedPowerLimit1 = 0;
    for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
      if (ConvertedPowerLimit1 < gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index]) {
        ConvertedPowerLimit1 = gCpuNvsAreaConfig->Area->CtdpPowerLimit1[Index];
      }
    }
    ConvertedPowerLimit2 = 0;
    for (Index = 0; Index < gCpuNvsAreaConfig->Area->CtdpLevelsSupported; Index++) {
      if (ConvertedPowerLimit2 < gCpuNvsAreaConfig->Area->CtdpPowerLimit2[Index]) {
        ConvertedPowerLimit2 = gCpuNvsAreaConfig->Area->CtdpPowerLimit2[Index];
      }
    }
  }
  ///
  /// Program Power Limit 1 (Long Duration Turbo) Time Window
  ///  If PowerLimit1Time is AUTO OR If PowerLimit1Time is > MAX_POWER_LIMIT_1_TIME_IN_SECONDS
  ///    program default values
  ///
  if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[0].CustomPowerLimit1Time != 0) {
    ConvertedPowerLimit1Time = GetConvertedTime (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[0].CustomPowerLimit1Time, SecondsTimeWindowConvert);
  } else {
    gCpuPowerMgmtBasicConfig->PowerLimit1Time = POWER_LIMIT1_TIME_DEFAULT;
    ConvertedPowerLimit1Time = GetConvertedTime (gCpuPowerMgmtBasicConfig->PowerLimit1Time, SecondsTimeWindowConvert);
  }
  ///
  /// Set MSR_PACKAGE_POWER_LIMIT.CRITICAL_POWER_CLAMP_1(bit 16)
  ///
  PackagePowerLimitMsr.Dwords.Low |= (UINT32) B_CRITICAL_POWER_CLAMP_ENABLE;

  ///
  /// Get the MCH space base address.
  ///
  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;

  ///
  /// Program MMIO PL1 clamp enable
  ///
  MmioOr32 (MchBar + MMIO_TURBO_POWER_LIMIT, B_CRITICAL_POWER_CLAMP_ENABLE);

  ///
  ///  Configure Power Limit 1 (Long Duration Turbo) time windows: Turbo Power Limit MSR [23:17]
  ///
  PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
  PackagePowerLimitMsr.Dwords.Low |= (UINT32) LShiftU64 (ConvertedPowerLimit1Time, 17);
  PackagePowerLimitMsr.Dwords.High |= B_POWER_LIMIT_ENABLE;
  PackagePowerLimitMsr.Dwords.Low |= (UINT32) (ConvertedPowerLimit1);
  PackagePowerLimitMsr.Dwords.High |= (UINT32) (ConvertedPowerLimit2);
  AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);
}

/**
  Configures BIOS overrides in MSR 0x610
    Configures Long duration Turbo Mode (power limit 1) power level and time window
    Configures Short duration turbo mode (power limit 2)
**/
VOID
ConfigureCtdpPowerLimitsOverrides (
  VOID
  )
{
  UINTN                   Index;
  UINTN                   NoOfOverrides;
  PPM_OVERRIDE_TABLE      *PpmCtdpOverrideTable;
  MSR_REGISTER            PackagePowerLimitMsr;

  PpmCtdpOverrideTable = mPowerLimitsOverrideTable;
  if (sizeof (PPM_OVERRIDE_TABLE) != 0) {
    NoOfOverrides = (sizeof (mPowerLimitsOverrideTable)) / (sizeof (PPM_OVERRIDE_TABLE));
  }
  PackagePowerLimitMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);

  for (Index = 0; Index < NoOfOverrides; Index++, PpmCtdpOverrideTable++) {
    if (PpmCtdpOverrideTable->CpuIdentifier == mCpuIdentifier) {
      ///
      /// MSR Overrides
      ///
      if (PpmCtdpOverrideTable->MsrPowerLimit1) {
        PackagePowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
        PackagePowerLimitMsr.Dwords.Low |= ((PpmCtdpOverrideTable->MsrPowerLimit1 * mProcessorPowerUnit) / 100) & POWER_LIMIT_MASK;
      }
      AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);
      ///
      /// MMIO Overrides
      ///
      if (PpmCtdpOverrideTable->CtdpNominalPowerLimit1) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit1[0] = (UINT16) ((PpmCtdpOverrideTable->CtdpNominalPowerLimit1 * mProcessorPowerUnit) / 100);
      }
      if (PpmCtdpOverrideTable->CtdpUpPowerLimit1) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit1[2] = (UINT16) ((PpmCtdpOverrideTable->CtdpUpPowerLimit1 * mProcessorPowerUnit) / 100);
      }
      if (PpmCtdpOverrideTable->CtdpDownPowerLimit1) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit1[1] = (UINT16) ((PpmCtdpOverrideTable->CtdpDownPowerLimit1 * mProcessorPowerUnit) / 100);
      }
      break;
    }
  }

  ///
  /// Process cTDP power limit override table
  ///
  PpmCtdpOverrideTable = mPowerLimitsOverrideTable;
  for (Index = 0; Index < NoOfOverrides; Index++, PpmCtdpOverrideTable++) {
    if (PpmCtdpOverrideTable->CpuIdentifier == mCpuIdentifier) {
      ///
      /// MSR Overrides
      ///
      if (PpmCtdpOverrideTable->MsrPowerLimit2) {
        PackagePowerLimitMsr.Dwords.High &= ~POWER_LIMIT_MASK;
        PackagePowerLimitMsr.Dwords.High |= ((PpmCtdpOverrideTable->MsrPowerLimit2 * mProcessorPowerUnit) / 100) & POWER_LIMIT_MASK;
      }
      AsmWriteMsr64 (MSR_PACKAGE_POWER_LIMIT, PackagePowerLimitMsr.Qword);

      ///
      /// MMIO Overrides
      ///
      if (PpmCtdpOverrideTable->CtdpNominalPowerLimit2) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit2[0] = (UINT16) ((PpmCtdpOverrideTable->CtdpNominalPowerLimit2 * mProcessorPowerUnit) / 100);
      }
      if (PpmCtdpOverrideTable->CtdpDownPowerLimit2) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit2[1] = (UINT16) ((PpmCtdpOverrideTable->CtdpDownPowerLimit2 * mProcessorPowerUnit) / 100);
      }
      if (PpmCtdpOverrideTable->CtdpUpPowerLimit2) {
        gCpuNvsAreaConfig->Area->CtdpPowerLimit2[2] = (UINT16) ((PpmCtdpOverrideTable->CtdpUpPowerLimit2 * mProcessorPowerUnit) / 100);
      }
      break;
    }
  }
}

/**
  Configure cTDP BIOS MSRs to Boot Ctdp values
    - Configures CONFIG_TDP_CONTROL MSR
    - Configures TURBO_ACTIVATION_RATIO MSR

  @param[in] CpuConfigTdpBootLevel  ConfigTdpBootLevel policy setting by user
**/
VOID
SelectCtdpLevel (
  IN UINT8             CpuConfigTdpBootLevel
  )
{
  MSR_REGISTER TempMsr;

  ///
  /// Select cTDP Nominal if cTDP is disabled or the level is not supported.
  ///
  if (CpuConfigTdpBootLevel == CONFIG_TDP_DEACTIVATE || CpuConfigTdpBootLevel >= gCpuNvsAreaConfig->Area->CtdpLevelsSupported) {
    CpuConfigTdpBootLevel = 0;
  }

  if (CpuConfigTdpBootLevel >= 3) {
    DEBUG ((DEBUG_ERROR, "ERROR: Invalid CpuConfigTdpBootLevel=%x, and CpuConfigTdpBootLevel should be range in 0-2 \n", CpuConfigTdpBootLevel));
    ASSERT (FALSE);
    return;
  }

  mCpuConfigTdpBootRatio     = gCpuNvsAreaConfig->Area->CtdpTar[CpuConfigTdpBootLevel]+1;
  gCpuNvsAreaConfig->Area->ConfigurablePpc = gCpuNvsAreaConfig->Area->CtdpPpc[CpuConfigTdpBootLevel];
  ///
  /// Program the selected level 00:nominal,01:level1,10:level2 to
  /// CONFIG TDP CONTROL MSR.
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_CONFIG_TDP_CONTROL);
  if ((TempMsr.Qword & CONFIG_TDP_CONTROL_LOCK) == 0) {
    TempMsr.Dwords.Low  = (UINT16) TempMsr.Dwords.Low &~CONFIG_TDP_CONTROL_LVL_MASK;
    TempMsr.Dwords.Low  = (UINT16) TempMsr.Dwords.Low | (CpuConfigTdpBootLevel & CONFIG_TDP_CONTROL_LVL_MASK);
    if (gCpuPowerMgmtCustomConfig->ConfigTdpLock == TRUE ) {
      TempMsr.Dwords.Low |= CONFIG_TDP_CONTROL_LOCK;
      DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP MSR_CONFIG_TDP_CONTROL is locked\n"));
    }
    AsmWriteMsr64 (MSR_CONFIG_TDP_CONTROL, TempMsr.Qword);
    DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP MSR_CONFIG_TDP_CONTROL=%x\n", TempMsr.Qword));
  } else {
    DEBUG ((DEBUG_INFO, "PPM:: Could not write MSR_CONFIG_TDP_CONTROL\n"));
  }
  ///
  /// Program the max non-turbo ratio corresponding to default selected level
  /// in TURBO_ACTIVATION_RATIO MSR.
  ///
  TempMsr.Qword = AsmReadMsr64 (MSR_TURBO_ACTIVATION_RATIO);
  if ((TempMsr.Qword & MSR_TURBO_ACTIVATION_RATIO_LOCK) == 0) {
    TempMsr.Dwords.Low &= ~MSR_TURBO_ACTIVATION_RATIO_MASK;
    TempMsr.Dwords.Low |= (UINT32) ((mCpuConfigTdpBootRatio - 1) & MSR_TURBO_ACTIVATION_RATIO_MASK);
    if (gCpuPowerMgmtCustomConfig->ConfigTdpLock == TRUE) {
      TempMsr.Dwords.Low |= MSR_TURBO_ACTIVATION_RATIO_LOCK;
      DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP MSR_TURBO_ACTIVATION_RATIO is locked\n"));
    }
    AsmWriteMsr64 (MSR_TURBO_ACTIVATION_RATIO, TempMsr.Qword);
    DEBUG ((DEBUG_INFO, "PPM:: ConfigTDP MSR_TURBO_ACTIVATION_RATIO=%x\n", TempMsr.Qword));
  } else {
    DEBUG ((DEBUG_INFO, "PPM:: Could not write MSR_TURBO_ACTIVATION_RATIO\n"));
  }
}

/**
  Configures the TURBO_POWER_LIMIT MMIO for Boot ConfigTdp Level

  @param[in] CpuConfigTdpBootLevel  ConfigTdpBootLevel policy setting by user
**/
VOID
SelectCtdpPowerLimits (
  IN UINT8             CpuConfigTdpBootLevel
  )
{
  UINTN        MchBar;
  UINT32       Data32And;
  UINT32       Data32Or;
  UINT16       PowerLimit1;
  UINT16       PowerLimit2;
  MSR_REGISTER TempMsr;

  ///
  /// Select cTDP Nominal if Ctdp disabled or boot level not supported.
  ///
  if (CpuConfigTdpBootLevel >= gCpuNvsAreaConfig->Area->CtdpLevelsSupported) {
    CpuConfigTdpBootLevel = 0;
  }

  PowerLimit1 = gCpuNvsAreaConfig->Area->CtdpPowerLimit1[CpuConfigTdpBootLevel];
  PowerLimit2 = gCpuNvsAreaConfig->Area->CtdpPowerLimit2[CpuConfigTdpBootLevel];
  ///
  /// Check if Power Limits are initalized
  ///
  if (PowerLimit1 != 0 && PowerLimit2 != 0) {
    ///
    /// Get the MCH space base address.
    /// Program Turbo Power Limit MMIO register MCHBAR+0x59A0 Bits [14:0] and [46:32]
    /// for ConfigTdp mode PL1 and PL2
    ///
    MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;
    ///
    /// Read PowerLimit MSR
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_LIMIT);
    ///
    /// Program cTDP Power Limit1
    ///
    Data32And = (UINT32) ~(PACKAGE_TDP_POWER_MASK);
    Data32Or  = (UINT32) (PowerLimit1 | (TempMsr.Dwords.Low & ~PACKAGE_TDP_POWER_MASK));
    MmioAndThenOr32 (MchBar + MMIO_TURBO_POWER_LIMIT, Data32And, Data32Or);
    ///
    /// Program cTDP Power Limit2
    ///
    Data32And = (UINT32) ~(PACKAGE_TDP_POWER_MASK);
    Data32Or  = (UINT32) (PowerLimit2 | (TempMsr.Dwords.High & ~PACKAGE_TDP_POWER_MASK));
    MmioAndThenOr32 (MchBar + MMIO_TURBO_POWER_LIMIT + 4, Data32And, Data32Or);
  }
}

/**
  Configures following fields of MSR 0x618 based on corresponding MMIO register (MCHBAR+0x58E0):
    Configures Long duration Turbo Mode (power limit 1) power level and time window for DDR domain
    Configures Short duration Turbo Mode (power limit 2) power level and time window for DDR domain
**/
VOID
ConfigureDdrPowerLimits (
  VOID
  )
{
  MSR_REGISTER  DdrPowerLimitMsr;
  UINTN         MchBar;

  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) & ~BIT0;

  DdrPowerLimitMsr.Qword = 0;
  DdrPowerLimitMsr.Qword = MmioRead64 (MchBar + MMIO_DDR_RAPL_LIMIT);

  DEBUG (
    (DEBUG_INFO,
     "DDR Power Limit 1 = %d\n",
     DdrPowerLimitMsr.Dwords.Low & POWER_LIMIT_MASK)
    );
  DEBUG (
    (DEBUG_INFO,
     "DDR Power Limit 2 = %d\n",
     DdrPowerLimitMsr.Dwords.High & POWER_LIMIT_MASK)
    );

  AsmWriteMsr64 (MSR_DDR_RAPL_LIMIT, DdrPowerLimitMsr.Qword);
}

/**
  Configures MSR 0x65C platform power limits (PSys)
    -Configures Platform Power Limit 1 Enable, power and time window
    -Configures Platform Power Limit 2 Enable, power
    -Platform power limits are limited by the Package Max and Min power
**/
VOID
ConfigurePlatformPowerLimits (
  VOID
  )
{
  MSR_REGISTER PlatformPowerLimitMsr;
  UINT16       ConvertedPowerLimit1;
  UINT8        ConvertedPowerLimit1Time;
  UINT16       ConvertedPowerLimit2;
  UINT16       CpuConvertedPowerLimit1MaxLimit;
  UINT16       CpuConvertedPowerLimit2MaxLimit;
  UINT16       Multiplier;

  CpuConvertedPowerLimit1MaxLimit = 0;
  CpuConvertedPowerLimit2MaxLimit = 0;
  ConvertedPowerLimit1Time        = 0;
  ///
  /// By default, for Mobile & Desktop Processors: Platform Power Limit 2 = 1.25 * Package TDP
  ///
  Multiplier = 125;

  PlatformPowerLimitMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_POWER_LIMIT);
  ///
  /// Initialize the Power Limit 1 and Power Limit 1 enable bit
  ///  - Power Limit 1: Platform Power Limit MSR  [14:0]
  ///  - Power Limit 1 Enable: Platform Power Limit MSR  [15]
  ///
  ///
  /// By default, program Power Limit 1 to Package TDP limit
  ///
  ConvertedPowerLimit1 = mPackageTdp;
  if (gCpuPowerMgmtPsysConfig->PsysPowerLimit1Power != AUTO) {
    ///
    /// gCpuPowerMgmtPsysConfig->PsysPowerLimit1Power is in mW or watts. We need to
    /// convert it to CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
    /// Since we are converting from Watts to CPU power units, multiply by
    /// PACKAGE_POWER_SKU_UNIT_MSR[3:0].
    ///
    ConvertedPowerLimit1 = (UINT16) ((gCpuPowerMgmtPsysConfig->PsysPowerLimit1Power * mProcessorPowerUnit) / mCustomPowerUnit);
    if (mPackageMaxPower == 0 && ConvertedPowerLimit1 >= mPackageMinPower) {
      ///
      ///  If PACKAGE_POWER_SKU_MSR [46:32] = 0 means there is no upper limit ( since this field is 15 bits, the max value is 2^15 - 1 )
      ///
      CpuConvertedPowerLimit1MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
      if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
        ///
        /// If new Power Limit 1 is > CpuConvertedPowerLimit1MaxLimit, program Power Limit 1 to CpuConvertedPowerLimit1MaxLimit
        ///
        ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
      }
    } else if (mPackageMinPower == 0 && ConvertedPowerLimit1 > 0 && ConvertedPowerLimit1 <= mPackageMaxPower) {
      ///
      ///  If PACKAGE_POWER_SKU_MSR [30:16] = 0 means there is no lower limit
      ///
      ConvertedPowerLimit1 = (UINT16) ((gCpuPowerMgmtBasicConfig->PowerLimit1 * mProcessorPowerUnit) / mCustomPowerUnit);

    } else {
      ///
      /// Power Limit 1 needs to be between mPackageMinPower and mPackageMaxPower
      ///
      CpuConvertedPowerLimit1MaxLimit = mPackageMaxPower;

      if (ConvertedPowerLimit1 < mPackageMinPower) {
        ///
        /// If new Power Limit 1 is < mPackageMinPower, program Power Limit 1 to mPackageMinPower
        ///
        ConvertedPowerLimit1 = mPackageMinPower;
      } else if (ConvertedPowerLimit1 > CpuConvertedPowerLimit1MaxLimit) {
        ///
        /// If new Power Limit 1 is > mPackageMaxPower, program Power Limit 1 to mPackageMaxPower
        ///
        ConvertedPowerLimit1 = CpuConvertedPowerLimit1MaxLimit;
      }
    }
  }
  PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
  PlatformPowerLimitMsr.Dwords.Low |= (UINT32) (ConvertedPowerLimit1);
  DEBUG (
          (DEBUG_INFO,
           "New Platform Power Limit 1  %d watt (%d in CPU power unit)\n",
           gCpuPowerMgmtPsysConfig->PsysPowerLimit1Power,
           ConvertedPowerLimit1)
          );

  ///
  /// Set PlatformPowerLimitMsr.CRITICAL_POWER_CLAMP_1(bit 16)
  ///
  PlatformPowerLimitMsr.Dwords.Low |= (UINT32)B_CRITICAL_POWER_CLAMP_ENABLE;

  ///
  /// Update Platform Power Limit 1 enable bit
  ///
  if (gCpuPowerMgmtPsysConfig->PsysPowerLimit1) {
    PlatformPowerLimitMsr.Dwords.Low |= B_POWER_LIMIT_ENABLE;
  } else {
    ///
    /// When we disable Platform Power Limit we need to write power limit = 0
    ///
    PlatformPowerLimitMsr.Dwords.Low &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
  }

  ///
  /// Program Platform Power Limit 1 Time Window
  ///  If PlatformPowerLimit1Time is AUTO OR If PlatformPowerLimit1Time
  ///  is > MAX_POWER_LIMIT_1_TIME_IN_SECONDS program default values
  ///
  if ((gCpuPowerMgmtPsysConfig->PsysPowerLimit1Time == AUTO) ||
      (gCpuPowerMgmtPsysConfig->PsysPowerLimit1Time > MAX_POWER_LIMIT_1_TIME_IN_SECONDS)
      ) {
    gCpuPowerMgmtPsysConfig->PsysPowerLimit1Time = POWER_LIMIT1_TIME_DEFAULT;
  }
  ConvertedPowerLimit1Time = GetConvertedTime (gCpuPowerMgmtPsysConfig->PsysPowerLimit1Time, SecondsTimeWindowConvert);
  ///
  ///  Configure Platform Power Limit 1 time windows: Platform Power Limit MSR [23:17]
  ///
  PlatformPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
  PlatformPowerLimitMsr.Dwords.Low |= (UINT32) LShiftU64 (ConvertedPowerLimit1Time, 17);
  ///
  /// gCpuPowerMgmtPsysConfig->PsysPowerLimit2Power value is in mW or watts. We need to convert it to
  /// CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].Since we are converting
  /// from Watts to CPU power units, multiply by PACKAGE_POWER_SKU_UNIT_MSR[3:0]
  ///
  ConvertedPowerLimit2 = (UINT16) ((gCpuPowerMgmtPsysConfig->PsysPowerLimit2Power * mProcessorPowerUnit) / mCustomPowerUnit);
  PlatformPowerLimitMsr.Dwords.High &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
  ///
  /// If PlatformPowerLimit2 is AUTO OR if PlatformPowerLimit2 is > mPackageMaxPower
  /// OR if Platform PowerLimit2 < mPackageMinPower program defaul values.
  ///
  CpuConvertedPowerLimit2MaxLimit = mPackageMaxPower;
  if (CpuConvertedPowerLimit2MaxLimit == 0) {
    CpuConvertedPowerLimit2MaxLimit = (UINT16) (LShiftU64 (2, 15) - 1);
  }
  if (gCpuPowerMgmtPsysConfig->PsysPowerLimit2Power == AUTO) {
    ConvertedPowerLimit2 = EFI_IDIV_ROUND ((Multiplier * mPackageTdp), 100);

  }
  if (ConvertedPowerLimit2 > CpuConvertedPowerLimit2MaxLimit) {
    ConvertedPowerLimit2 = CpuConvertedPowerLimit2MaxLimit;
  }
  if (ConvertedPowerLimit2 < mPackageMinPower) {
    ConvertedPowerLimit2 = mPackageMinPower;
  }
  PlatformPowerLimitMsr.Dwords.High |= (UINT32) (ConvertedPowerLimit2);

  if (gCpuPowerMgmtPsysConfig->PsysPowerLimit2 == TRUE) {
    PlatformPowerLimitMsr.Dwords.High |= B_POWER_LIMIT_ENABLE;
  } else {
    ///
    /// When we disable Platform Power Limit we need to write power limit = 0
    ///
    PlatformPowerLimitMsr.Dwords.High &= ~(POWER_LIMIT_MASK | B_POWER_LIMIT_ENABLE);
  }

  DEBUG (
          (DEBUG_INFO,
           "Platform Power Limit 2 Power = %d Watts (%d in CPU power unit)\n",
           gCpuPowerMgmtPsysConfig->PsysPowerLimit2Power,
           ConvertedPowerLimit2)
          );

  DEBUG ((DEBUG_INFO,"MSR(65Ch)=%08X%08X\n",PlatformPowerLimitMsr.Dwords.High,PlatformPowerLimitMsr.Dwords.Low));

  AsmWriteMsr64 (MSR_PLATFORM_POWER_LIMIT, PlatformPowerLimitMsr.Qword);
}

/**
  Configures PowerLimits and Config TDP values
**/
VOID
ConfigureCtdp (
  VOID
  )
{
  UINT8 CustomPowerLimits;
  UINT8 Index;
  ///
  /// Configure CTDP power limits.Refer Rev 0.6.0 BWG sec 16.7.1: Enabling Intel Configurable TDP support
  ///
  ConfigureCtdpPowerLimits ();

  ///
  ///
  /// Check if any power limits are overriden, flag CustomPowerLimits if detected
  ///
  CustomPowerLimits = FALSE;
  for (Index = 0; Index < MAX_CUSTOM_CTDP_ENTRIES; Index++) {
    if (gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1 != 0 ||
        gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit1Time != 0 ||
        gCpuPowerMgmtCustomConfig->CustomConfigTdpTable[Index].CustomPowerLimit2 != 0) {
      CustomPowerLimits = TRUE;
    }
  }

  ///
  /// BIOS power limit overrides
  /// Don't override if custom ctdp settings are provided.
  ///
  if (CustomPowerLimits == FALSE) {
    ConfigureCtdpPowerLimitsOverrides ();
  }
  ///
  /// To avoid issues and race conditions it is recommended for the below order to be followed:
  ///    - For TDP Up program the Power Limits followed by Config TDP Level
  ///    - For TDP Down program the Config TDP Level followed by Power Limits
  ///
  if (gCpuPowerMgmtTestConfig->ConfigTdpLevel == CONFIG_TDP_UP) {
    SelectCtdpPowerLimits (gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex);
    SelectCtdpLevel (gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex);
  } else {
    SelectCtdpLevel (gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex);
    SelectCtdpPowerLimits (gCpuNvsAreaConfig->Area->ConfigTdpBootModeIndex);
  }
}

