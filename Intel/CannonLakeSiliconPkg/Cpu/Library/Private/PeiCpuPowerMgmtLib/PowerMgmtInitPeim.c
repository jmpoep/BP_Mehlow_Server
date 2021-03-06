/** @file
  Processor Power Management initialization code. This code determines current
  user configuration and modifies and loads ASL as well as initializing chipset
  and processor features to enable the proper power management.

  Acronyms:
    PPM - Processor Power Management
    TM  - Thermal Monitor
    IST - Intel(R) Speedstep technology
    HT  - Hyper-Threading Technology
   ITBM - Intel(R) Turbo Boost Max Technology 3.0

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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

#include "PowerMgmtInitPeim.h"
#include <Library/PciSegmentLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>

extern EFI_GUID gCpuInitDataHobGuid;

//
// Global variables
//

///
/// Power Managment policy configurations
///
SI_POLICY_PPI                    *gSiCpuPolicy                  = NULL;
SI_PREMEM_POLICY_PPI             *gSiCpuPreMemPolicy            = NULL;
CPU_POWER_MGMT_BASIC_CONFIG      *gCpuPowerMgmtBasicConfig      = NULL;
CPU_POWER_MGMT_CUSTOM_CONFIG     *gCpuPowerMgmtCustomConfig     = NULL;
CPU_POWER_MGMT_PSYS_CONFIG       *gCpuPowerMgmtPsysConfig       = NULL;
CPU_POWER_MGMT_TEST_CONFIG       *gCpuPowerMgmtTestConfig       = NULL;
CPU_OVERCLOCKING_PREMEM_CONFIG   *gCpuOverClockingPreMemConfig  = NULL;
CPU_POWER_MGMT_VR_CONFIG         *gCpuPowerMgmtVrConfig         = NULL;
CPU_CONFIG_LIB_PREMEM_CONFIG     *gCpuConfigLibPreMemConfig     = NULL;

//
// Values for FVID table calculate.
//
UINT16 mTurboBusRatio             = 0;
UINT16 mMaxBusRatio               = 0;
UINT16 mMinBusRatio               = 0;
UINT16 mProcessorFlavor           = 0;
UINT16 mPackageTdp                = 0;                        ///< Processor TDP value in MSR_PACKAGE_POWER_SKU.
UINT16 mPackageTdpWatt            = 0;                        ///< Processor TDP value in Watts.
UINT16 mCpuConfigTdpBootRatio     = 0;                        ///< Config TDP Boot settings
UINT16 mCustomPowerUnit           = 1;
BOOLEAN mOver16Pstates            = FALSE;

///
/// Fractional part of Processor Power Unit in Watts. (i.e. Unit is 1/mProcessorPowerUnit)
///
UINT8 mProcessorPowerUnit        = 0;
///
/// Maximum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
UINT16 mPackageMaxPower           = 0;
///
/// Minimum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
/// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
///
UINT16 mPackageMinPower           = 0;
UINT8 mRatioLimitProgrammble      = 0; ///< Programmable Ratio Limit
UINT8 mTdpLimitProgrammble        = 0; ///< Porgrammable TDP Limit
CPU_NVS_AREA_CONFIG mCpuNvsAreaConfig  = {0};
CPU_NVS_AREA_CONFIG *gCpuNvsAreaConfig = NULL;

///
/// Cpu Identifier used for PM overrides
///
UINT16 mCpuIdentifier;

//
// FVID Table Information
// Default FVID table
// One header field plus states
//
FVID_TABLE *mFvidPointer              = NULL;

/**
  Power Management init after memory PEI module

  @param[in] PeiServices Pointer to PEI Services Table.

  @retval EFI_SUCCESS           The driver installes/initialized correctly.
  @retval EFI_NOT_FOUND         CPU Data HOB is not available.
  @retval EFI_OUT_OF_RESOURCES  No enough resoruces (such as out of memory).
  @retval Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.
**/
EFI_STATUS
CpuPowerMgmtInit (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  EFI_STATUS                 Status;
  UINTN                      Size;
  EFI_PHYSICAL_ADDRESS       Area;
  VOID                       *Hob;
  CPU_INIT_DATA_HOB          *CpuInitDataHob;
  EFI_BOOT_MODE              BootMode;

  CpuInitDataHob = NULL;

  DEBUG((DEBUG_INFO, "CpuPowerMgmtInit Start \n"));
  PostCode (0xC6A);


  DEBUG ((DEBUG_INFO, " PeimInitializePowerMgmt Entry\n"));

  ///
  /// Locate platform configuration information
  ///
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &gSiCpuPolicy);
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &gSiCpuPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Initialize the Global pointer for Power Managment Policy
  ///
  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuPowerMgmtBasicConfigGuid, (VOID *) &gCpuPowerMgmtBasicConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuPowerMgmtCustomConfigGuid, (VOID *) &gCpuPowerMgmtCustomConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuPowerMgmtPsysConfigGuid, (VOID *) &gCpuPowerMgmtPsysConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuPowerMgmtTestConfigGuid, (VOID *) &gCpuPowerMgmtTestConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) gSiCpuPreMemPolicy, &gCpuOverclockingPreMemConfigGuid, (VOID *) &gCpuOverClockingPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuConfigGuid, (VOID *) &mCpuConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPolicy, &gCpuPowerMgmtVrConfigGuid, (VOID *) &gCpuPowerMgmtVrConfig);
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) gSiCpuPreMemPolicy, &gCpuConfigLibPreMemConfigGuid, (VOID *) &gCpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesGetBootMode (&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (gCpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ///
    /// Locate MpService Ppi
    ///
    Status = PeiServicesLocatePpi (
                         &gEfiPeiMpServicesPpiGuid,
                         0,
                         NULL,
                         (VOID **) &gMpServicesPpi);
    ASSERT_EFI_ERROR (Status);

    ///
    /// Get CPU Init Data Hob
    ///

    Hob = GetFirstGuidHob (&gCpuInitDataHobGuid);
    if (Hob == NULL) {
      DEBUG ((DEBUG_ERROR, "CPU Data HOB not available\n"));
      return EFI_NOT_FOUND;
    }
    CpuInitDataHob = (CPU_INIT_DATA_HOB *) ((UINTN) Hob + sizeof (EFI_HOB_GUID_TYPE));
  }

  ///
  /// Allocated ACPI NVS type memory for Cpu Nvs Configuration.
  ///
  Size = sizeof (CPU_NVS_AREA);

  if (BootMode == BOOT_ON_S3_RESUME) {

    //
    // On S3 resume, NVS should not be created again.
    // Due to CFL/CNL late milestone, only minimal code change can be done.
    // The W/A only needed for FSP is to create NVS in a different region that will not be seen by the OS and ignored.
    // If use AllocatePage for S3 resume, memory may be allocated over the existing OS NVS and over write it.
    // This will taken care of without W/A in next project by creating NVS region in DXE.
    //

    Area = (EFI_PHYSICAL_ADDRESS)(UINTN)AllocatePool (Size);
    ASSERT (Area != 0);
  } else {
    Status = PeiServicesAllocatePages (EfiACPIMemoryNVS, EFI_SIZE_TO_PAGES (Size), &Area);
    ASSERT_EFI_ERROR (Status);
  }

  if (Area == 0) {
    return EFI_OUT_OF_RESOURCES;
  }

  gCpuNvsAreaConfig = &mCpuNvsAreaConfig;
  mCpuNvsAreaConfig.Area = (CPU_NVS_AREA *) (UINTN) Area;
  ZeroMem ((VOID *) mCpuNvsAreaConfig.Area, sizeof (CPU_NVS_AREA));


  if (gCpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ///
    /// Saved Cpu Nvs pointer to CpuInitDataHob
    ///
    CpuInitDataHob->CpuGnvsPointer = (EFI_PHYSICAL_ADDRESS) (UINTN) Area;

    mCpuNvsAreaConfig.Area->Revision = CPU_NVS_AREA_REVISION;

    ///
    /// Initialize FVID table pointer
    ///
    mFvidPointer = (FVID_TABLE *) (UINTN) CpuInitDataHob->FvidTable;
  }
  ///
  /// Initialize Power management Global variables
  ///
  InitPowerManagementGlobalVariables ();

  ///
  /// Initialize CPU Power management code (determine HW and configured state, configure hardware and software accordingly)
  ///
  Status = InitPpm ((CONST EFI_PEI_SERVICES **) PeiServices);
  ASSERT_EFI_ERROR (Status);
  DEBUG((DEBUG_INFO, "CPU Post-Mem Exit \n"));
  PostCode (0xC7F);

  return EFI_SUCCESS;
}

/**
  Initializes the platform power management global variable.
  This must be called prior to any of the functions being used.
**/
VOID
InitPowerManagementGlobalVariables (
  VOID
  )
{
  MSR_REGISTER TempMsr;
  MSR_REGISTER PackagePowerSKUUnitMsr;
  MSR_REGISTER PlatformInfoMsr;

  mCpuNvsAreaConfig.Area->Cpuid = GetCpuFamily () | GetCpuStepping ();

  ///
  /// Get Platform ID by reading System Agent's Device ID (B0:D0:F0:R02)
  ///
  mProcessorFlavor = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));

  ///
  /// Get Maximum Non-Turbo bus ratio (HFM) from Platform Info MSR Bits[15:8]
  ///
  PlatformInfoMsr.Qword = AsmReadMsr64 (MSR_PLATFORM_INFO);
  mMaxBusRatio          = PlatformInfoMsr.Bytes.SecondByte;
  ///
  /// Get Maximum Efficiency bus ratio (LFM) from Platform Info MSR Bits[47:40]
  ///
  mMinBusRatio = PlatformInfoMsr.Bytes.SixthByte;
  ///
  /// Get Max Turbo Ratio from Turbo Ratio Limit MSR Bits [7:0]
  ///
  TempMsr.Qword   = AsmReadMsr64 (MSR_TURBO_RATIO_LIMIT);
  mTurboBusRatio  = (UINT16) (TempMsr.Dwords.Low & B_MSR_TURBO_RATIO_LIMIT_1C);
  ///
  /// Check if Turbo Ratio Limit is programmable
  ///  Platform Info MSR (0xCE) [28]
  ///
  mRatioLimitProgrammble = (UINT8) RShiftU64 ((PlatformInfoMsr.Qword & B_PLATFORM_INFO_RATIO_LIMIT), 28);
  ///
  /// Check if TDP Limit is programmable
  ///  Platform Info MSR (0xCE) [29]
  ///
  mTdpLimitProgrammble = (UINT8) RShiftU64 ((PlatformInfoMsr.Qword & B_PLATFORM_INFO_TDC_TDP_LIMIT), 29);

  ///
  /// Get Processor TDP
  /// Get Maximum Power from Turbo Power Limit MSR Bits[46:32]
  /// and convert it to units specified by Package Power SKU
  /// Unit MSR [3:0]
  ///
  TempMsr.Qword                 = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU);
  PackagePowerSKUUnitMsr.Qword  = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);
  mProcessorPowerUnit           = (PackagePowerSKUUnitMsr.Bytes.FirstByte & PACKAGE_POWER_UNIT_MASK);
  if (mProcessorPowerUnit == 0) {
    mProcessorPowerUnit = 1;
  } else {
    mProcessorPowerUnit = (UINT8) LShiftU64 (2, (mProcessorPowerUnit - 1));
  }
  mPackageTdp = (UINT16) (TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK);
  mPackageTdpWatt = (UINT16) DivU64x32 (mPackageTdp , mProcessorPowerUnit);
  mPackageMaxPower  = (UINT16) (TempMsr.Dwords.High & PACKAGE_MAX_POWER_MASK);
  mPackageMinPower  = (UINT16) RShiftU64 ((TempMsr.Dwords.Low & PACKAGE_MIN_POWER_MASK), 16);

  ///
  /// Set mCustomPowerUnit to user selected Power unit
  ///
  mCustomPowerUnit = 1;
  if (gCpuPowerMgmtTestConfig->CustomPowerUnit == PowerUnit125MilliWatts) {
        mCustomPowerUnit = 8; ///< Unit is 125 milli watt
  }

  if (gCpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ///
    /// Calculate the number of Oc bins supported. Read in MSR 194h FLEX_RATIO bits (19:17)
    ///
    TempMsr.Qword = AsmReadMsr64 (MSR_FLEX_RATIO);
    mCpuNvsAreaConfig.Area->OcBins = (UINT8) RShiftU64 ((TempMsr.Dwords.Low & B_OVERCLOCKING_BINS), 17);

    ///
    /// Update NVS ASL items.
    ///
    mCpuNvsAreaConfig.Area->HwpSmi = (UINTN) PcdGet8 (PcdHwpSmi); ///< HWP SMI for setting up LVT

    ///
    /// Intel Turbo Boost Max Technology 3.0
    ///
    mCpuNvsAreaConfig.Area->ItbmSmi = (UINTN) PcdGet8 (PcdItbmSmi); ///< SMI for resuming the periodic SMM if the OS supports frequency re-read.
    mCpuNvsAreaConfig.Area->EnableItbm = gCpuPowerMgmtBasicConfig->EnableItbm;
    mCpuNvsAreaConfig.Area->EnableItbmDriver = gCpuPowerMgmtBasicConfig->EnableItbmDriver; ///< ACPI device for Intel Turbo Boost Max Technology 3.0 driver.

    mCpuIdentifier = GetCpuIdentifier ();

    ///
    /// Initialize flags based on processor capablities
    ///
    SetPpmFlags ();

    ///
    /// Determine current user configuration
    ///
    SetUserConfigurationPpmFlags ();
  }
  return;
}

/**
  Set the PPM flags
**/
VOID
SetPpmFlags (
  VOID
  )
{
  MSR_REGISTER                       Ia32MiscEnable;
  MSR_REGISTER                       FlexRatioMsr;
  CPUID_VERSION_INFO_ECX             VersionInfoEcx;
  CPUID_VERSION_INFO_EDX             VersionInfoEdx;
  CPUID_MONITOR_MWAIT_ECX            MonitorMwaitEcx;
  CPUID_MONITOR_MWAIT_EDX            MonitorMwaitEdx;
  CPUID_THERMAL_POWER_MANAGEMENT_EAX PowerEax;
  UINT16                             ThreadsPerCore;
  UINT16                             CoreCount;
  UINT8                              OverclockingBins;
  UINTN                              States;
  BOOLEAN                            CpuidLimitingEnabled;
  UINT32                             PpmFlags;

  PpmFlags = 0;

  ///
  /// Check if the processor has multiple threads.
  /// @todo CMP should only be set if the processor has multiple cores,
  /// but current usage expects it to be set based on threads.
  ///
  GetEnabledCount (&ThreadsPerCore, &CoreCount, NULL, NULL);
  if ((ThreadsPerCore * CoreCount) > 1) {
    PpmFlags |= PPM_CMP;
  }

  ///
  /// Disable CPUID limiting (and save current setting) if enabled
  /// and enable MONITOR/MWAIT support
  ///
  Ia32MiscEnable.Qword  = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  CpuidLimitingEnabled  = (BOOLEAN) (Ia32MiscEnable.Qword & B_MSR_IA32_MISC_ENABLE_CPUID_MAX);
  if (CpuidLimitingEnabled) {
    Ia32MiscEnable.Qword &= (UINT64) ~B_MSR_IA32_MISC_ENABLE_CPUID_MAX;
  }
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  ///
  /// Read the CPUID values we care about. To get the correct MONITOR/MWAIT-related values,
  /// we need to read after we have disabled limiting and enabled MONITOR/MWAIT
  ///
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, &VersionInfoEcx.Uint32, &VersionInfoEdx.Uint32);
  AsmCpuid (CPUID_MONITOR_MWAIT, NULL, NULL, &MonitorMwaitEcx.Uint32, &MonitorMwaitEdx.Uint32);
  AsmCpuid (CPUID_THERMAL_POWER_MANAGEMENT, &PowerEax.Uint32, NULL, NULL, NULL);

  ///
  /// Check Thermal Monitor capable and update the flag.
  ///
  if (VersionInfoEdx.Bits.TM == 1) {
    PpmFlags |= PPM_TM;
  }

  ///
  /// Check Enhanced Intel SpeedStep(R) technology capable.
  ///
  if (VersionInfoEcx.Bits.EIST == 1) {
    PpmFlags |= (PPM_EIST);
    DEBUG ((DEBUG_INFO, "EIST capable\n"));
  }

  ///
  /// Determine if the MONITOR/MWAIT instructions are supported.
  ///
  if ((VersionInfoEcx.Bits.MONITOR == 1) && (MonitorMwaitEcx.Bits.ExtensionsSupported == 1)) {
    PpmFlags |= PPM_MWAIT_EXT;
  }

  ///
  /// Determine the C-State and Enhanced C-State support present.
  /// Monitor/MWAIT parameters function describes the numbers supported.
  ///
  States = RShiftU64 (MonitorMwaitEdx.Uint32, 4) & 0xF;
  if (States >= ENHANCED_CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C1 + PPM_C1E;
  } else if (States == CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C1;
  }
#ifdef CPU_CFL
  States = RShiftU64 (MonitorMwaitEdx.Uint32, 8) & 0xF;
  if ((States >= CSTATE_SUPPORTED) && (PpmFlags & PPM_C1)) {
    PpmFlags |= PPM_C3;
  }
#endif
  States = RShiftU64 (MonitorMwaitEdx.Uint32, 12) & 0xF;
  if (States >= C6_C7_LONG_LATENCY_SUPPORTED) { // Both Long and Short Latency C6 supported
    PpmFlags |= (PPM_C6 | C6_LONG_LATENCY_ENABLE);
  } else if (States >= C6_C7_SHORT_LATENCY_SUPPORTED) { // Only Short Latency C6 supported.
    PpmFlags |= PPM_C6;
  }

  States = RShiftU64 (MonitorMwaitEdx.Uint32, 16) & 0xF;
  switch (States) {
    case C7s_LONG_LATENCY_SUPPORTED:
      //
      // C7 & C7s Long and Short supported
      //
      PpmFlags |= (PPM_C7S | C7s_LONG_LATENCY_ENABLE | PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C7s_SHORT_LATENCY_SUPPORTED:
      //
      // C7s Long Latency is not supported.
      //
      PpmFlags |= (PPM_C7S | PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C6_C7_LONG_LATENCY_SUPPORTED:
      //
      // C7 Long and Short supported
      //
      PpmFlags |= (PPM_C7 | C7_LONG_LATENCY_ENABLE);
      break;
    case C6_C7_SHORT_LATENCY_SUPPORTED:
      //
      // C7 Long Latency is not supported.
      //
      PpmFlags |= PPM_C7;
      break;
    default:
      break;
  }

  States = RShiftU64 (MonitorMwaitEdx.Uint32, 20) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C8;
  }
  States = RShiftU64 (MonitorMwaitEdx.Uint32, 24) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C9;
  }
  States = RShiftU64 (MonitorMwaitEdx.Uint32, 28) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    PpmFlags |= PPM_C10;
  }

  ///
  /// Check TimedMwait is supported and update the flag
  ///
  if (AsmReadMsr64 (MSR_PLATFORM_INFO) & B_PLATFORM_INFO_TIMED_MWAIT_SUPPORTED) {
    PpmFlags |= PPM_TIMED_MWAIT;
  }
  if (PpmFlags & (PPM_C8 |PPM_C9 | PPM_C10)) {
    PpmFlags |= PPM_CD;
  }

  ///
  /// Check if turbo mode is supported and update the flag
  ///
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
  if ((PowerEax.Bits.TurboBoostTechnology == 0) &&
      ((Ia32MiscEnable.Qword & B_MSR_IA32_MISC_DISABLE_TURBO) == 0)
      ) {
    ///
    /// Turbo Mode is not available in this physical processor package.
    /// BIOS should not attempt to enable Turbo Mode via IA32_MISC_ENABLE MSR.
    /// BIOS should show Turbo Mode as Disabled and Not Configurable.
    ///
  } else if (PowerEax.Bits.TurboBoostTechnology == 0) {
    ///
    /// Turbo Mode is available but globally disabled for the all logical
    /// processors in this processor package.
    /// BIOS can enable Turbo Mode by IA32_MISC_ENABLE MSR 1A0h bit [38] = 0.
    ///
    PpmFlags |= PPM_TURBO;
  } else if (PowerEax.Bits.TurboBoostTechnology == 1) {
    ///
    /// Turbo Mode is factory-configured as available and enabled for all logical processors in this processor package.
    /// This case handles the cases where turbo mode is enabled before PPM gets chance to enable it
    ///
    PpmFlags |= PPM_TURBO;
  }

  ///
  /// Restore the CPUID limit setting.
  ///
  if (CpuidLimitingEnabled) {
    Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLE);
    Ia32MiscEnable.Qword |= B_MSR_IA32_MISC_ENABLE_CPUID_MAX;
    AsmWriteMsr64 (MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }
  PpmFlags |= PPM_TSTATES; ///< Set the T-states flag

  ///
  /// Determine if Clock modulation duty cycle extension is supported
  ///
  if (PowerEax.Bits.ECMD == 1) {
    PpmFlags |= PPM_TSTATE_FINE_GRAINED;
  }
  PpmFlags |= PPM_EEPST;                                ///< Energy Efficient P-state feature is supported

  ///
  /// Check HWP support
  ///
  if (PowerEax.Bits.HWP == 1) {
    PpmFlags |= PPM_HWP;
    DEBUG ((DEBUG_INFO, "HWP capable\n"));
  }

  if (PowerEax.Bits.HWP_Notification == 1) {
    PpmFlags |= PPM_HWP_LVT;
    DEBUG ((DEBUG_INFO, "HWP interrupt supported\n"));
  }

  ///
  /// Check if overclocking is fully unlocked
  ///
  FlexRatioMsr.Qword = AsmReadMsr64 (MSR_FLEX_RATIO);
  OverclockingBins = (UINT8) RShiftU64 ((FlexRatioMsr.Dwords.Low & B_OVERCLOCKING_BINS), N_OVERCLOCKING_BINS);
  if (OverclockingBins == MAX_OVERCLOCKING_BINS) {
    PpmFlags |= PPM_OC_UNLOCKED;
  }

  ///
  /// Check Intel Turbo Boost Max Technology 3.0 support.
  ///
  if ((PowerEax.Uint32 & B_ITBM_ENABLE)) { ///< Translates to Bit14 of CPUID_EAX(6)
    ///
    /// @todo: Replace check with actual structure member in PowerEax.Bits; once it is updated in UefiCpuPkg/Cpuid.h.
    ///        If CPUID_EAX(6) Bit14 is set, Intel Turbo Boost Max Technology 3.0 is supported.
    ///
    PpmFlags |= PPM_TURBO_BOOST_MAX;
    DEBUG((DEBUG_INFO, "Itbm: Intel Turbo Boost Max Technology 3.0 supported\n"));
  }

  mCpuNvsAreaConfig.Area->PpmFlags = PpmFlags; ///< Update the PPM NVS area PPM flags

  return;
}

/**
  Set the PPM flags based on current user configuration
**/
VOID
SetUserConfigurationPpmFlags (
  VOID
  )
{
  UINT32 UserPpmFlag;

  //
  // In advance to clear following PPM flags which are related with policies that user can enabled/disabled.
  //
  UserPpmFlag = (UINT32) ~(PPM_EIST | PPM_C1 | PPM_C1E | PPM_TM | PPM_TURBO | PPM_TSTATES |
                           PPM_TSTATE_FINE_GRAINED | PPM_EEPST | PPM_TIMED_MWAIT | PPM_HWP | PPM_OC_UNLOCKED);
  ///
  /// Configure flag based on user selections
  ///
  if (gCpuPowerMgmtTestConfig->Eist) {
    UserPpmFlag |= PPM_EIST;
  }
  if (gCpuPowerMgmtTestConfig->Cx) {
    UserPpmFlag |= PPM_C1;
    if (gCpuPowerMgmtTestConfig->C1e) {
      UserPpmFlag |= PPM_C1E;
    }
#ifdef CPU_CFL
  } else {
    UserPpmFlag &= ~(PPM_C3 | PPM_C6 | C6_LONG_LATENCY_ENABLE |
                     PPM_C7S | PPM_C7 | C7_LONG_LATENCY_ENABLE | C7s_LONG_LATENCY_ENABLE | PPM_CD |
                     PPM_C8 | PPM_C9 | PPM_C10);
  }
#else
  } else {
    UserPpmFlag &= ~( PPM_C6 | C6_LONG_LATENCY_ENABLE |
                     PPM_C7S | PPM_C7 | C7_LONG_LATENCY_ENABLE | C7s_LONG_LATENCY_ENABLE | PPM_CD |
                     PPM_C8 | PPM_C9 | PPM_C10);
  }
#endif
  if (gCpuPowerMgmtTestConfig->ThermalMonitor) {
    UserPpmFlag |= PPM_TM;
  }
  if (gCpuPowerMgmtBasicConfig->TurboMode) {
    UserPpmFlag |= PPM_TURBO;
  }
  if (gCpuPowerMgmtTestConfig->TStates) {
    UserPpmFlag |= (PPM_TSTATES | PPM_TSTATE_FINE_GRAINED);
  }
  if (gCpuPowerMgmtTestConfig->EnergyEfficientPState) {
    UserPpmFlag |= PPM_EEPST;
  }
  if (gCpuPowerMgmtTestConfig->TimedMwait) {
    UserPpmFlag |= PPM_TIMED_MWAIT;
  }
  if (gCpuPowerMgmtBasicConfig->Hwp) {
    UserPpmFlag |= PPM_HWP;
  }
  if (gCpuOverClockingPreMemConfig->OcSupport) {
    UserPpmFlag |= PPM_OC_UNLOCKED;
  }
  ///
  /// Modify PpmFlags based on user selections
  ///
  mCpuNvsAreaConfig.Area->PpmFlags &= UserPpmFlag;
}

/**
  Initialize the processor power management based on hardware capabilities
  and user configuration settings.

  @param[in] PeiServices    Pointer to PEI Services Table

  @retval EFI_SUCCESS - on success
  @retval Appropiate failure code on error
**/
EFI_STATUS
InitPpm (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
  EFI_STATUS Status;
  EFI_STATUS CtdpSupport;

  Status      = EFI_SUCCESS;
  CtdpSupport = EFI_UNSUPPORTED;

  DEBUG((DEBUG_INFO, "InitPpm \n"));
  PostCode (0xC71);
  if (gCpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ///
    /// Initialize Config TDP
    ///
    CtdpSupport = InitializeConfigurableTdp ();

    ///
    /// Initialize P states
    ///
    InitPStates ((CONST EFI_PEI_SERVICES **) PeiServices);

    ///
    /// Initialize Hwp
    ///
    InitializeHwp ((CONST EFI_PEI_SERVICES **) PeiServices);

    ///
    /// Initalize Intel Turbo Boost Max Technology 3.0
    ///
    InitializeItbm ((CONST EFI_PEI_SERVICES **) PeiServices);

    ///
    /// Initialize C State (IdleStates)
    ///
    InitCState ((CONST EFI_PEI_SERVICES **) PeiServices);

    //
    // Patch P state table (Fvid table) with ctdp settings.
    //
    CtdpPatchFvidTable (mFvidPointer);
    if (mOver16Pstates) {
      CtdpPatchFvidTableforLimitPstate (mFvidPointer);
    }

    ///
    /// Initialize thermal features
    ///
    InitThermal ((CONST EFI_PEI_SERVICES **) PeiServices);
  }

  ///
  /// Initialise Miscellaneous features
  ///
  InitMiscFeatures (CtdpSupport);

  if (gCpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ///
    /// Lock down all settings
    ///
    PpmLockDown ((CONST EFI_PEI_SERVICES **) PeiServices);
  }

  return Status;
}
