/** @file
  PCH private PEI PMC Library for all PCH generations.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2018 Intel Corporation.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/ConfigBlockLib.h>
#include <Ppi/SiPolicy.h>
#include <Library/PmcLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Private/Library/DciPrivateLib.h>
#include <Library/PchEspiLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PchFiaLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Private/Library/PeiItssLib.h>
#include <Register/PchRegsPmc.h>
#include <Register/PchRegsClk.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsLan.h>
#include "PmcPrivateLibInternal.h"

/**
  Check if PCH PM Timer enabled based on platform policy

  @retval TRUE       PCH PM Timer is enabled.
  @retval FALSE      PCH PM Timer is disabled.
**/
BOOLEAN
PmcIsPchPmTimerEnabled (
  VOID
  )
{
  BOOLEAN           PchPmTimerEnabled;
  EFI_STATUS        Status;
  SI_POLICY_PPI     *SiPolicy;
  PCH_PM_CONFIG     *PmConfig;

  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPolicy
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  PchPmTimerEnabled = TRUE;
  if (!PmConfig->EnableTcoTimer) {
    PchPmTimerEnabled = FALSE;
  }

  DEBUG ((DEBUG_INFO, "PmcIsPchPmTimerEnabled () = %x\n", PchPmTimerEnabled));

  return PchPmTimerEnabled;
}

/**
  Configure PM Timer.
  Once PM Timer is disabled, TCO timer stops counting.
  This must be executed after uCode PM timer emulation.

  @param[in] SiPolicy                  The SI Policy PPI instance
**/
VOID
PmcConfigurePmTimer (
  IN  SI_POLICY_PPI                    *SiPolicy
  )
{
  EFI_STATUS                            Status;
  PCH_PM_CONFIG                         *PmConfig;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  if (PmConfig->EnableTcoTimer) {
    return;
  }

  DEBUG ((DEBUG_INFO, "PmcConfigurePmTimer - Disable ACPI timer.\n"));
  //
  // For CNL: Disable ACPI timer (set PwrmBase + 0x18FC[1] = 1), and the TCO timer will also stop.
  //
  MmioWrite32 (PmcGetPwrmBase () + R_PMC_PWRM_ACPI_TMR_CTL, B_PMC_PWRM_ACPI_TMR_CTL_DIS);
}

/**
  Lock down PMC settings

  @param[in] SiPolicy   The SI Policy PPI instance
**/
VOID
PmcLockSettings (
  IN  SI_POLICY_PPI     *SiPolicy
  )
{

  UINT32         PchPwrmBase;
  PchPwrmBase = PmcGetPwrmBase ();

  ///
  /// Set PWRMBASE Offset 0x1048 [24]
  ///
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_ETR3, BIT24);

  ///
  /// PM_SYNC_LOCK
  /// Set PWRMBASE Offset 0x18C8 [15]
  ///
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_PMSYNC_MISC_CFG, B_PMC_PWRM_PMSYNC_PM_SYNC_LOCK);
}

/**
  Configure deep Sx settings

  @param[in] SiPolicy                   The SI Policy instance

  @retval EFI_SUCCESS                   The function completed successfully
**/
STATIC
VOID
ConfigureDeepSxSettings (
  SI_POLICY_PPI               *SiPolicy
  )
{
  UINT32                                PchPwrmBase;
  UINT32                                S3Data32;
  UINT32                                S4Data32;
  UINT32                                S5Data32;
  EFI_STATUS                            Status;
  PCH_PM_CONFIG                         *PmConfig;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  PchPwrmBase = PmcGetPwrmBase ();
  ///
  /// PCH BIOS Spec Section 19.11  Deep Sx Power Policies
  /// The System BIOS can perform the following register programming guidelines to enable system
  /// enter Deep S3, Deep S4 or Deep S5.
  ///
  ///                            DPS3_EN_DC         DPS3_EN_AC         DPS4_EN_DC         DPS4_EN_AC         DPS5_EN_DC          DPS5_EN_AC
  ///                         S3_PWRGATE_POL[1]  S3_PWRGATE_POL[0]  S4_PWRGATE_POL[1]  S4_PWRGATE_POL[0]  S5_PWRGATE_POL[15]  S5_PWRGATE_POL[14]
  /// Deep Sx disabled                 0                0                  0                  0                  0                   0
  ///
  /// Enabled in S5                    0                0                  0                  0                  1                   1
  ///
  /// Enabled in S4 and S5             0                0                  1                  1                  1                   1
  ///
  /// Enabled in S3, S4 and S5         1                1                  1                  1                  1                   1
  ///
  /// Configuration supported by MOBILE:
  /// Enabled in S5                    0                0                  0                  0                  1                   0
  /// (Battery mode)
  /// Enabled in S4 and S5             0                0                  1                  0                  1                   0
  /// (Battery Mode)
  /// Enabled in S3, S4 and S5         1                0                  1                  0                  1                   0
  /// (Battery Mode)
  ///
  /// NOTE: Mobile platforms support Deep S3/S4/S5 in DC ONLY,
  /// Desktop and Intel C206 Chipset (LPC Dev ID 0x1C56) platforms support Deep S3/S4/S5 in AC ONLY,
  /// Intel C204 Chipset (LPC Dev ID 0x1C54) and Intel C202 Chipset (LPC Dev ID 0x1C52) platforms DO NOT support Deep S4/S5.
  ///
  /// Deep Sx disabled                 0                0                  0                  0                  0                   0
  ///
  switch (PmConfig->PchDeepSxPol) {
    case PchDpS5AlwaysEn:
      ///
      /// Configuration 2: Enabled in S5/AC-DC
      /// DEEP_S3_POL.DPS3_EN_DC = 0; DEEP_S3_POL.DPS3_EN_AC = 0;
      /// DEEP_S4_POL.DPS4_EN_DC = 0; DEEP_S4_POL.DPS4_EN_AC = 0;
      /// DEEP_S5_POL.DPS5_EN_DC = 1; DEEP_S5_POL.DPS5_EN_AC = 1;
      ///
      S3Data32  = 0;
      S4Data32  = 0;
      S5Data32  = (UINT32) (B_PMC_PWRM_S5_PWRGATE_POL_S5AC_GATE_SUS | B_PMC_PWRM_S5_PWRGATE_POL_S5DC_GATE_SUS);
      break;

    case PchDpS4S5AlwaysEn:
      ///
      /// Configuration 4: Enabled only in S4-S5
      /// DEEP_S3_POL.DPS3_EN_DC = 0; DEEP_S3_POL.DPS3_EN_AC = 0;
      /// DEEP_S4_POL.DPS4_EN_DC = 1; DEEP_S4_POL.DPS4_EN_AC = 1;
      /// DEEP_S5_POL.DPS5_EN_DC = 1; DEEP_S5_POL.DPS5_EN_AC = 1;
      ///
      S3Data32  = 0;
      S4Data32  = (UINT32) (B_PMC_PWRM_S4_PWRGATE_POL_S4AC_GATE_SUS | B_PMC_PWRM_S4_PWRGATE_POL_S4DC_GATE_SUS);
      S5Data32  = (UINT32) (B_PMC_PWRM_S5_PWRGATE_POL_S5AC_GATE_SUS | B_PMC_PWRM_S5_PWRGATE_POL_S5DC_GATE_SUS);
      break;
    case PchDpS5BatteryEn:
      ///
      /// Configuration 1: Enabled in S5/Battery only
      /// DEEP_S3_POL.DPS3_EN_DC = 0; DEEP_S3_POL.DPS3_EN_AC = 0;
      /// DEEP_S4_POL.DPS4_EN_DC = 0; DEEP_S4_POL.DPS4_EN_AC = 0;
      /// DEEP_S5_POL.DPS5_EN_DC = 1; DEEP_S5_POL.DPS5_EN_AC = 0;
      ///
      S3Data32  = 0;
      S4Data32  = 0;
      S5Data32  = (UINT32) (B_PMC_PWRM_S5_PWRGATE_POL_S5DC_GATE_SUS);
      break;

    case PchDpS4S5BatteryEn:
      ///
      /// Configuration 3: Enabled only in S4-S5/Battery Mode
      /// DEEP_S3_POL.DPS3_EN_DC = 0; DEEP_S3_POL.DPS3_EN_AC = 0;
      /// DEEP_S4_POL.DPS4_EN_DC = 1; DEEP_S4_POL.DPS4_EN_AC = 0;
      /// DEEP_S5_POL.DPS5_EN_DC = 1; DEEP_S5_POL.DPS5_EN_AC = 0;
      ///
      S3Data32  = 0;
      S4Data32  = (UINT32) (B_PMC_PWRM_S4_PWRGATE_POL_S4DC_GATE_SUS);
      S5Data32  = (UINT32) (B_PMC_PWRM_S5_PWRGATE_POL_S5DC_GATE_SUS);
      break;
    case PchDeepSxPolDisable:
    default:
      ///
      /// Configuration 5: DeepSx Disabled
      /// DEEP_S3_POL.DPS3_EN_DC = 0; DEEP_S3_POL.DPS3_EN_AC = 0;
      /// DEEP_S4_POL.DPS4_EN_DC = 0; DEEP_S4_POL.DPS4_EN_AC = 0;
      /// DEEP_S5_POL.DPS5_EN_DC = 0; DEEP_S5_POL.DPS5_EN_AC = 0;
      ///
      S3Data32  = 0;
      S4Data32  = 0;
      S5Data32  = 0;
      break;
  }
  MmioWrite32 ((PchPwrmBase + R_PMC_PWRM_S3_PWRGATE_POL), S3Data32);
  MmioWrite32 ((PchPwrmBase + R_PMC_PWRM_S4_PWRGATE_POL), S4Data32);
  MmioWrite32 ((PchPwrmBase + R_PMC_PWRM_S5_PWRGATE_POL), S5Data32);
}

/**
  Configure PCH to CPU energy report

  @param[in] SiPolicy                  The SI Policy instance
**/
STATIC
VOID
ConfigureEnergyReport (
  IN  SI_POLICY_PPI                    *SiPolicy
  )
{
  UINT32                                PchPwrmBase;
  EFI_STATUS                            Status;
  PCH_PM_CONFIG                         *PmConfig;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  if (PmConfig->DisableEnergyReport) {
    return;
  }

  PchPwrmBase = PmcGetPwrmBase ();

  //
  // Enable Energy Reporting
  // PM_CFG (0x1818h[2]) = 1b
  // PM_CFG (0x1818h[24]) = 1b  Must perform this setting as a separate write
  //
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_CFG, B_PMC_PWRM_CFG_ER_EN);
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_CFG, B_PMC_PWRM_CFG_ER_LOCK);

}

/**
  Configure Sleep signal assertion when under debug

  @param[in] ProbeType                 The debugger prode Type
  @param[in] OverrideSlpS0ProbeType    Override policy for the debugger prode Type
**/
STATIC
VOID
ConfigurePmcSleepSignal (
  IN  PLATFORM_DEBUG_CONSENT_PROBE_TYPE           ProbeType,
  IN  SLP_S0_OVERRIDE                             OverrideSlpS0ProbeType
  )
{
  ///
  /// BIOS to enable/disable of slp_s0# assertion during S0ix based on setup for B0 stepping onwards
  ///
  if (!(IsCnlPch () && IsPchLp () && (PchStepping () < PCH_B0))) {
    if (OverrideSlpS0ProbeType == SlpS0OverrideAuto) {
      switch (ProbeType) {
        case ProbeTypeDciOobDbc:
        case ProbeTypeUsb2Dbc:
          PmcDisableSlpS0AssertionInDebugMode ();
          DEBUG ((DEBUG_INFO, "ConfigurePmcSleepSignal - Disable slp_s0# assertion when debug is enabled\n"));
          break;

        case ProbeTypeDciOob:
        case ProbeTypeXdp3:
        case ProbeTypeUsb3Dbc:
        case ProbeTypeDisabled:
          PmcEnableSlpS0AssertionInDebugMode ();
          DEBUG ((DEBUG_INFO, "ConfigurePmcSleepSignal - Enable slp_s0# assertion when debug is enabled\n"));
          break;

        default:
          break;
      }
    } else {
      if (OverrideSlpS0ProbeType == SlpS0OverrideEnabled) {
        PmcDisableSlpS0AssertionInDebugMode ();
        DEBUG ((DEBUG_INFO, "ConfigurePmcSleepSignal - Disable slp_s0# assertion when debug is enabled\n"));
      } else {
        PmcEnableSlpS0AssertionInDebugMode ();
        DEBUG ((DEBUG_INFO, "ConfigurePmcSleepSignal - Enable slp_s0# assertion when debug is enabled\n"));
      }
    }
  }
}

typedef struct {
  UINT16 Address;
  UINT32 AndData;
  UINT32 OrData;
} SLPS0_SETTING_OVERRIDE_TAB;

typedef enum {
  SlpS0Reg1B1C,
  SlpS0Reg1B4C,
  SlpS0Reg1E4C,
  SlpS0RegMax
} SLPS0_SETTING_OVERRIDE_REG;

/**
  Override S0ix settings for platform debug

  @param[in] ProbeType                 The debugger prode Type
  @param[in] OverridePolicyProbeType   Override Policy for the debugger prode Type
**/
STATIC
VOID
OverrideSlpS0ConfigForDebug (
  PLATFORM_DEBUG_CONSENT_PROBE_TYPE    ProbeType,
  S0IX_DISQ_PROBE_TYPE                 OverridePolicyProbeType
  )
{
  UINT32                               PchPwrmBase;
  UINT8                                Index;
  SLPS0_SETTING_OVERRIDE_TAB           SlpS0OverrideTab[SlpS0RegMax] = {
                                        {R_PMC_PWRM_1B1C, 0xFFFFFFFF, 0},
                                        {R_PMC_PWRM_1B4C, 0xFFFFFFFF, 0},
                                        {R_PMC_PWRM_1E4C, 0xFFFFFFFF, 0}
                                        };
  PchPwrmBase = PmcGetPwrmBase ();
  // DCI debug probe types other than USB2DbC and BSSB need to set PWRMBASE + 0x1B4C [18, 20, 21] = 000b
  //
  if (OverridePolicyProbeType == S0ixDisQAuto) {
    switch (ProbeType) {
      case ProbeTypeDciOobDbc:
      case ProbeTypeUsb2Dbc:
        ///
        /// To make USB2DbC survive during S0ix, need disable Voltage Margining to disqualify itself from S0ix.
        /// PWRMBASE + 0x1B1C [13, 22, 25] = 010b
        /// PWRMBASE + 0x1B4C [18, 20, 21] = 111b
        /// PWRMBASE + 0x1E4C [15, 19] = 11b
        ///
        SlpS0OverrideTab[SlpS0Reg1B1C].AndData &= (UINT32)~(BIT13 | BIT25);
        SlpS0OverrideTab[SlpS0Reg1B1C].OrData |= BIT22;
        SlpS0OverrideTab[SlpS0Reg1B4C].OrData |= (BIT18 | BIT20 | BIT21);
        SlpS0OverrideTab[SlpS0Reg1E4C].OrData |= (BIT15 | BIT19);
        break;

      case ProbeTypeDciOob:
        //
        // Disqualify ModPhy Core PG for DCI OOB
        //
        SlpS0OverrideTab[SlpS0Reg1B4C].AndData &= (UINT32)~(BIT18 | BIT20);
        SlpS0OverrideTab[SlpS0Reg1B4C].OrData |= BIT21;
        break;

      case ProbeTypeXdp3:
      case ProbeTypeUsb3Dbc:
      case ProbeTypeDisabled:
        SlpS0OverrideTab[SlpS0Reg1B4C].AndData &= (UINT32)~(BIT18 | BIT20);
        break;

      default:
        break;
    }
  } else {
    switch (OverridePolicyProbeType) {
      case S0ixDisQUsb2Dbc:
        ///
        /// To make USB2DbC survive during S0ix, need disable Voltage Margining to disqualify itself from S0ix.
        /// PWRMBASE + 0x1B1C [13, 22, 25] = 010b
        /// PWRMBASE + 0x1B4C [18, 20, 21] = 111b
        /// PWRMBASE + 0x1E4C [15, 19] = 11b
        ///
        SlpS0OverrideTab[SlpS0Reg1B1C].AndData &= (UINT32)~(BIT13 | BIT25);
        SlpS0OverrideTab[SlpS0Reg1B1C].OrData |= BIT22;
        SlpS0OverrideTab[SlpS0Reg1B4C].OrData |= (BIT18 | BIT20 | BIT21);
        SlpS0OverrideTab[SlpS0Reg1E4C].OrData |= (BIT15 | BIT19);
        break;

      case S0ixDisQDciOob:
        //
        // Disqualify ModPhy Core PG for DCI OOB
        //
        SlpS0OverrideTab[SlpS0Reg1B4C].AndData &= (UINT32)~(BIT18 | BIT20);
        SlpS0OverrideTab[SlpS0Reg1B4C].OrData |= BIT21;
        break;

      case S0ixDisQNoChange:
        SlpS0OverrideTab[SlpS0Reg1B4C].AndData &= (UINT32)~(BIT18 | BIT20);
        break;

      default:
        break;
    }
  }

  for (Index = 0; Index < SlpS0RegMax; Index++) {
    MmioAndThenOr32 (
      PchPwrmBase + SlpS0OverrideTab[Index].Address,
      SlpS0OverrideTab[Index].AndData,
      SlpS0OverrideTab[Index].OrData
      );
  }
}

/**
  S0ix settings

  @param[in] PmConfig                 PmConfig
  @param[in] ProbeType                Platform Debug Consent Probe type
**/
STATIC
VOID
SlpS0Config (
  PCH_PM_CONFIG                         *PmConfig,
  PLATFORM_DEBUG_CONSENT_PROBE_TYPE     ProbeType
  )
{
  BOOLEAN                               VmSupported;
  UINT32                                Data32And;
  UINT32                                Data32Or;
  UINT32                                PchPwrmBase;
  BOOLEAN                               GbeEnable;
  BOOLEAN                               EnableSlpS0WithGbeSupport;

  PchPwrmBase = PmcGetPwrmBase ();

  GbeEnable = (PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LAN, PCI_FUNCTION_NUMBER_PCH_LAN, 0)) != 0xFFFFFFFF);

  VmSupported = ((PmConfig->SlpS0VmRuntimeControl || PmConfig->SlpS0Vm070VSupport || PmConfig->SlpS0Vm075VSupport) != 0);

  EnableSlpS0WithGbeSupport = PmConfig->SlpS0WithGbeSupport && GbeEnable;

  if (EnableSlpS0WithGbeSupport) {
    //
    // if LAN device exists, apply settings to support SLP_S0 with GbE connected, which requires platform to disable voltage marginning and also apply PMC VM disabling settings.
    //
    VmSupported = 0;
  }

  if (PmConfig->SlpS0Enable) {
    ///
    /// If SLP_S0 is enabled apply below configuration:
    ///  PWRMBASE + 0x1B1C [29,25,24,23,15] = all 1s
    ///  PWRMBASE + 0x1B1C [22,15,14,13,12] = all 0s
    ///  PWRMBASE + 0x1B1C [8:0] to 0xA
    ///  If Voltage Margining is supported:
    ///   PWRMBASE + 0x1B1C [13] = 1b
    ///   PWRMBASE + 0x1B1C [22] = 1b (only for configuration without 0.70V support)
    ///  If Voltage Margining is NOT supported:
    ///   PWRMBASE + 0x1B1C [22,12] = all 1s
    ///  If SLP_S0 with GBE Support is enabled:
    ///   PWRMBASE + 0x1B1C [30] = 1b
    /// Other fields in 0x1B1C - HW default
    ///
    Data32And = (UINT32) ~(BIT30 | BIT22 | BIT15 | BIT14 | BIT13 | BIT12 | 0x000001FF);
    Data32Or  = BIT29 | BIT25 | BIT24 | BIT23 | BIT15 | 0x00A;

    ASSERT (!(!PmConfig->SlpS0VmRuntimeControl && PmConfig->SlpS0Vm070VSupport && PmConfig->SlpS0Vm075VSupport));

    if (VmSupported) {
      Data32Or |= BIT13;

      //
      // Voltage Margining configuration without 0.70V support
      //
      if ((!PmConfig->SlpS0Vm070VSupport) &&
          (PmConfig->SlpS0Vm075VSupport)) {
        Data32Or |= BIT22;
      }

    } else {
      Data32Or |= BIT22 | BIT12;
    }

    if (EnableSlpS0WithGbeSupport) {
      Data32Or |= BIT30;
    }

    MmioAndThenOr32 (
      PchPwrmBase + R_PMC_PWRM_1B1C,
      Data32And,
      Data32Or
      );

    ///
    /// If SLP_S0 is enabled apply below configuration:
    ///  PWRMBASE + 0x1B4C [31,30,28,25,19] = all 1s
    ///  PWRMBASE + 0x1B4C [27,26] = 0
    ///  PWRMBASE + 0x1B4C [22] = 1b if Voltage Margining is NOT supported, otherwise set it to 0b
    ///  PWRMBASE + 0x1B4C [21] = 1b for CNP-LP B0 and CNP-H A0 B0 Steppings or if GbE is enabled
    ///  If SLP_S0 with GbE Support is enabled:
    ///   PWRMBASE + 0x1B4C [21] = 1b
    ///
    Data32And = (UINT32) ~(BIT30 | BIT27 | BIT26 | BIT22 | BIT21);
    Data32Or  = BIT31 | BIT30 | BIT28 | BIT25 | BIT19;

    if (!VmSupported) {
      Data32Or |= BIT22;
    }

    if (IsCnlPch () &&
        ((IsPchLp () && (PchStepping () == PCH_B0)) ||
         (IsPchH () && (PchStepping () == PCH_A0)))) {
      Data32Or |= BIT21;
    } else if (EnableSlpS0WithGbeSupport) {
      Data32Or |= BIT21;
    }

    MmioAndThenOr32 (
      PchPwrmBase + R_PMC_PWRM_1B4C,
      Data32And,
      Data32Or
      );

    ///
    /// If SLP_S0 is enabled apply below configuration:
    ///  PWRMBASE + 0x1E4C [20,17,15,3,0] = all 1s
    ///  PWRMBASE + 0x1E4C [19] = 1b if Voltage Margining is NOT supported
    ///
    Data32Or  = BIT20 | BIT17 | BIT15 | BIT3 | BIT0;
    Data32And = ~0u;

    if (!VmSupported) {
      Data32Or |= BIT19;
    }

    MmioAndThenOr32 (
      PchPwrmBase + R_PMC_PWRM_1E4C,
      Data32And,
      Data32Or
      );

    ///
    /// Configure PM settings if debug is enabled
    ///
    if (IsDciDebugEnabled()) {
      OverrideSlpS0ConfigForDebug (ProbeType, (S0IX_DISQ_PROBE_TYPE) PmConfig->SlpS0DisQForDebug);
      ConfigurePmcSleepSignal (ProbeType, (SLP_S0_OVERRIDE) PmConfig->SlpS0Override);
    }
  }
}


/**
  This function sets IRQ number used for SCI interrupt

  @param[in] Irq                 Irq number (9, 10, 11, 20, 21, 22, 23)
**/
VOID
PmcSetSciIrq (
  IN  UINT8          Irq
  )
{
  UINT32  Data32Or;
  //
  // IRQx    Value to set in register
  // IRQ9    000b
  // IRQ10   001b
  // IRQ11   010b
  // IRQ20   100b
  // IRQ21   101b
  // IRQ22   110b
  // IRQ23   111b
  //
  switch (Irq) {
    case 9:  Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ9;  break;
    case 10: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ10; break;
    case 11: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ11; break;
    case 20: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ20; break;
    case 21: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ21; break;
    case 22: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ22; break;
    case 23: Data32Or = V_PMC_PWRM_ACPI_CNT_SCIS_IRQ23; break;
    default:
      //
      // Unsupported SCI IRQ
      //
      ASSERT (FALSE);
      return;
  }
  MmioAndThenOr32 (
    PmcGetPwrmBase () + R_PMC_PWRM_ACPI_CNT,
    ~(UINT32)B_PMC_PWRM_ACPI_CNT_SCIS,
    Data32Or
    );
}

/**
  Perform PCH PMC initialization

  @param[in] SiPolicy           The SI Policy PPI instance
**/
VOID
PmcInit (
  IN  SI_POLICY_PPI           *SiPolicy
  )
{
  UINT32                    PchPwrmBase;
  UINT32                    Data32;
  UINT32                    Data32And;
  UINT32                    Data32Or;
  UINT8                     PortNum;
  UINT8                     MaxUsb3PortNum;
  EFI_STATUS                Status;
  PCH_PM_CONFIG             *PmConfig;
  PCH_DMI_CONFIG            *DmiConfig;
  SI_PREMEM_POLICY_PPI      *SiPreMemPolicy;
  PCH_DCI_PREMEM_CONFIG     *DciPreMemConfig;
  USB_CONFIG                *UsbConfig;
  UINT32                    PmConA;
  UINT32                    PmConB;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPreMemPolicy
             );
  ASSERT_EFI_ERROR (Status);

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gDmiConfigGuid, (VOID *) &DmiConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicy, &gDciPreMemConfigGuid, (VOID *) &DciPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  PchPwrmBase = PmcGetPwrmBase ();

  //
  // Perform PM recommendation settings
  //

  ///
  /// GEN_PMCON_A[22] = 1, Allow OPI PLL Shutdown in C0 (ALLOW_OPI_PLL_SD_INC0)
  /// GEN_PMCON_A[20] = 1, Allow SPXB Clock Gating in C0 (ALLOW_SPXB_CG_INC0)
  /// GEN_PMCON_A[19] = 1, Allow L1.LOW Entry during C0 (ALLOW_L1LOW_C0)
  /// GEN_PMCON_A[17] = 1, Allow L1.LOW Entry with OPI Voltage ON (ALLOW_L1LOW_OPI_ON)
  /// GEN_PMCON_A[13] = 1, Allow L1.LOW Entry with CPU BCLK REQ Asserted (ALLOW_L1LOW_BCLKREQ_ON)
  ///
  //
  // Do not clear those RW/1C bits by accident.
  //
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_GEN_PMCON_A,
    (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_GBL_RST_STS |
               B_PMC_PWRM_GEN_PMCON_A_MS4V |
               B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR |
               B_PMC_PWRM_GEN_PMCON_A_PWR_FLR |
               B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS),
    B_PMC_PWRM_GEN_PMCON_A_ALLOW_OPI_PLL_SD_INC0 |
    B_PMC_PWRM_GEN_PMCON_A_ALLOW_SPXB_CG_INC0 |
    B_PMC_PWRM_GEN_PMCON_A_ALLOW_L1LOW_C0 |
    B_PMC_PWRM_GEN_PMCON_A_ALLOW_L1LOW_OPI_ON |
    B_PMC_PWRM_GEN_PMCON_A_ALLOW_L1LOW_BCLKREQ_ON
    );

  ///
  /// PWRMBASE + BM_CX_CNF[14, 10, 9] to all 1
  /// This register MUST be programmed before PM_SYNC_MODE is programmed.
  ///
  MmioOr32 (
    PchPwrmBase + R_PMC_PWRM_BM_CX_CNF,
    B_PMC_PWRM_BM_CX_CNF_PHOLD_BM_STS_BLOCK |
    B_PMC_PWRM_BM_CX_CNF_BM_STS_ZERO_EN |
    B_PMC_PWRM_BM_CX_CNF_PM_SYNC_MSG_MODE
    );

  ///
  /// PWRMBASE + 0x1814 [24,18,16,15,13,11,10,9,7,6,5,3,1,0] = all 1s
  ///
  MmioOr32 (
    PchPwrmBase + R_PMC_PWRM_1814,
    BIT24 | BIT18 | BIT16 | BIT15 | BIT13 | BIT11 | BIT10 | BIT9 | BIT7 | BIT6 | BIT5 | BIT3 | BIT1 | BIT0
    );

  ///
  /// PWRMBASE + PM_CFG2[31:29]
  /// For PCH power button override period, 0=4sec, 011b=10sec
  ///
  Data32Or = (PmConfig->PwrBtnOverridePeriod) << N_PMC_PWRM_CFG2_PBOP;
  ///
  /// PWRMBASE + PM_CFG2[28] for power button native mode disable
  ///
  if (PmConfig->DisableNativePowerButton) {
    Data32Or |= B_PMC_PWRM_CFG2_PB_DIS;
  }
  //
  // Set debounce timer for pin that supports PWRBTN
  // Do this even if PWRBTN is not used in case debounce feature
  // of this pin is to be used for a different purposes
  //
  if (PmConfig->PowerButtonDebounce > 0) {
    GpioSetPwrBtnDebounceTimer (PmConfig->PowerButtonDebounce);
  }
  if (PmConfig->PmcDbgMsgEn) {
    Data32Or |= B_PMC_PWRM_CFG2_EN_DBG_MSG;
  }
  Data32And = (UINT32)~(B_PMC_PWRM_CFG2_PBOP | B_PMC_PWRM_CFG2_PB_DIS | B_PMC_PWRM_CFG2_EN_DBG_MSG);
  MmioAndThenOr32 (PchPwrmBase + R_PMC_PWRM_CFG2, Data32And, Data32Or);
  ///
  /// PWRMBASE + 0x1848 = 0x0E2D9F01
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_SN_SLOW_RING, 0x0E2D9F01);
  ///
  /// PWRMBASE + 0x184C = 0x00000001
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_SN_SLOW_RING2, 1);

  ///
  /// PWRMBASE + 0x1850 = 0x0E2D9F01
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_SN_SA, 0x0E2D9F01);
  ///
  /// PWRMBASE + 0x1854 = 0x00000001
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_SN_SA2, 1);

  ///
  /// PWRMBASE + 0x1858 = 0x00018000
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_SN_SLOW_RING_CF, 0x00018000);
  ///
  /// PWRMBASE + 0x1868 = 0x04043400
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_NS_SA, 0x04043400);

  //
  // Configure which clock wake signals should set the
  // SLOW_RING, SA, FAST_RING_CF and SLOW_RING_CF indication sent up to the CPU/PCH
  //
  PmcInitClockWakeEnable ();

  ///
  /// PWRMBASE + 0x18A8 = 0x00181300
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_PA_SLOW_RING, 0x00181300);

  ///
  /// PWRMBASE + 0x18AC = 0x02006001
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_PA_SLOW_RING2, 0x02006001);

  ///
  /// PWRMBASE + 0x18B0 = 0x00181300
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_PA_SA, 0x00181300);

  ///
  /// PWRMBASE + 0x18B4 = 0x02006001
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_PA_SA2, 0x02006001);

  PmcConfigurePmSyncEventsSettings ();

  ///
  /// PWRMBASE + 0x18D0 = 0x00200840
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_PM_SYNC_STATE_HYS, 0x00200840);

  ///
  /// PWRMBASE + 0x18D4 = 0x7000F4E6
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_PM_SYNC_MODE, 0x7000F4E6);

  ///
  /// If eSPI boot, PWRMBASE + 0x18E0 = 0x01040009
  /// If SPI boot, PWRMBASE + 0x18E0 = 0x01040001
  /// If CPU_C10_GATE# pin is used, PWRMBASE + 0x18E0[2] = 1
  /// For wake on WLAN from S3/S4/S5, set PWRM_CFG3 [BIT17], HOST_WLAN_PP_EN = 1
  /// For wake on WLAN from DeepSx S3/S4/S5, set PWRM_CFG3 [BIT16], DSX_WLAN_PP_EN = 1
  ///
  if (IsEspiEnabled ()) {
    Data32 = 0x01040009;
  } else {
    Data32 = 0x01040001;
  }

  if (PmConfig->CpuC10GatePinEnable) {
    Data32 |= B_PMC_PWRM_CFG3_HOST_MISC_CORE_CFG_CPU_VCC_MAP;
  }

  if (PmConfig->WakeConfig.WoWlanEnable == TRUE) {
    Data32 |= B_PMC_PWRM_CFG3_HOST_WLAN_PP_EN;
    if (PmConfig->WakeConfig.WoWlanDeepSxEnable == TRUE) {
      Data32 |= B_PMC_PWRM_CFG3_DSX_WLAN_PP_EN;
    }
  }

  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_CFG3, Data32);

  if (PmConfig->CpuC10GatePinEnable) {
    GpioEnableCpuC10GatePin ();
  }

  ///
  /// PWRMBASE + 0xF4 = 0x7000F4C0
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_PM_SYNC_MODE_C0, 0x7000F4C0);
  ///
  /// For <B0, PWRMBASE + 0x1A00[9,0] = 1b,0b
  ///    >=B0, PWRMBASE + 0x1A00[9,0] = 1b,1b
  ///
  if ((IsCnlPch ()) && (PchStepping () < PCH_B0)) {
    MmioAndThenOr32 (
      PchPwrmBase + R_PMC_PWRM_PMLDOCTRL,
      (UINT32) ~(B_PMC_PWRM_PMLDOCTRL_USB2TS1P3LDODSEN),
      B_PMC_PWRM_PMLDOCTRL_CNVIP24LDOLPEN
     );
  } else {
    MmioOr32 (PchPwrmBase + R_PMC_PWRM_PMLDOCTRL, B_PMC_PWRM_PMLDOCTRL_CNVIP24LDOLPEN | B_PMC_PWRM_PMLDOCTRL_USB2TS1P3LDODSEN);
  }

  ///
  /// For LP, set R_PMC_PWRM_MODPHY_PM_CFG1[MLS0SWPGP] Bits 15:0 = 0xFFFF
  ///         set R_PMC_PWRM_MODPHY_PM_CFG5[MSPDRTREQ] Bits [25,24,23,22,21,16] = all 1's
  ///
  if (IsPchLp ()) {
    MmioWrite16 (PchPwrmBase + R_PMC_PWRM_MODPHY_PM_CFG1, 0xFFFF);
    MmioOr32 (PchPwrmBase + R_PMC_PWRM_MODPHY_PM_CFG5, 0x3E10000);
  }

  ///
  /// Clear corresponding bit in MLSXSWPGP when ModPHY SUS Well Lane Power Gating is not permitted in Sx
  /// As USB3 lanes do not support SUS Well Power Gating as they support in-band wake, the bits corresponding
  /// to USB3 lanes need to be cleared
  ///
  MaxUsb3PortNum = GetPchXhciMaxUsb3PortNum ();
  Data32And = 0;
  for (PortNum = 0; PortNum < MaxUsb3PortNum; PortNum++) {
    if (PchFiaIsUsb3PortConnected (PortNum)) {
      Data32And |= (1 << PortNum);
    }
  }
  MmioAnd32 (
    PchPwrmBase + R_PMC_PWRM_MODPHY_PM_CFG2,
    ~Data32And
    );
  ///
  /// MODPHY_PM_CFG3 (0x10C8[29:24]) = 101010, External FET Ramp Time (EFRT)
  ///
  /// If DCI is enabled, skip 0x10C8 [29:24] programming. Disable ModPHY sus power gating feature
  ///
  Data32And = (UINT32) ~B_PMC_PWRM_MODPHY_PM_CFG3_EFRT;
  Data32Or  = 0;
  if (PmConfig->ModPhySusPgEnable && IsPchLp ()) {
    if (!IsDciDebugEnabled ()) {
      Data32Or  |= (B_PMC_PWRM_MODPHY_PM_CFG3_EMFC | (V_PMC_PWRM_MODPHY_PM_CFG3_EFRT_200US << N_PMC_PWRM_MODPHY_PM_CFG3_EFRT));
    }
    ///
    /// ModPHY Lane SUS Power Domain Dynamic Gating Enable
    /// MODPHY_PM_CFG3 (0x10C8[30]) = 1
    ///
    Data32Or |= B_PMC_PWRM_MODPHY_PM_CFG3_MLSPDDGE;
  }

  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_MODPHY_PM_CFG3,
    Data32And,
    Data32Or
    );

  ///
  /// PM_CFG4 (0x18E8h[8:0]) = 018h, CPU I/O VR Ramp Duration
  /// For PCH-LP and USB2 PHY SUS Well Power Gating enabled:
  ///    PM_CFG4 (0x18E8h[30])  = 1, USB2 PHY SUS Well Power Gating Enable (U2_PHY_PG_EN)
  ///
  /// For PCH-H and PS_ON Enabled:
  ///   Note: PS_ON allows support for a new C10 state from the CPU on desktop SKUs that
  ///         enables a lower power target that will be required by the California Energy
  ///         Commission (CEC).
  /// PM_CFG4 (0x18E8h[31])  = 1,    CEC_EN
  /// PM_CFG4 (0x18E8h[29:23]) = 0Ah (5 ms), SLPS0_PSON_TMR
  /// PM_CFG4 (0x18E8h[22:18]) = 1Fh (620 ms), PSON_SLPS0_TMR
  ///
  Data32And = (UINT32) ~B_PMC_PWRM_CFG4_CPU_IOVR_RAMP_DUR;
  Data32Or  = (V_PMC_PWRM_CFG4_CPU_IOVR_RAMP_DUR_240US << N_PMC_PWRM_CFG4_CPU_IOVR_RAMP_DUR);

  ///
  /// USB2 PHY SUS Well Power Gating must be done after USB2 AFE programming
  ///
  if (!IsPchH () && UsbConfig->Usb2PhySusPgEnable) {
    Data32Or |= B_PMC_PWRM_CFG4_U2_PHY_PG_EN;
  }

  if (IsPchH () && PmConfig->PsOnEnable) {
    Data32And &= (UINT32) ~(B_PMC_PWRM_CFG4_SLPS0_PSON_TMR | B_PMC_PWRM_CFG4_PSON_SLPS0_TMR);
    Data32Or  |= (V_PMC_PWRM_CFG4_SLPS0_PSON_TMR_5MS << N_PMC_PWRM_CFG4_SLPS0_PSON_TMR) |
                 (V_PMC_PWRM_CFG4_PSON_SLPS0_TMR_620MS << N_PMC_PWRM_CFG4_PSON_SLPS0_TMR);
  }

  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CFG4,
    Data32And,
    Data32Or
    );
	
	
  ///
  /// PWRMBASE + 0x1B20 = 0x0005DB01
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_CPPM_MISC_CFG, 0x0005DB01);
  ///
  /// CPPM_CG_POL1A (0x1B24h[30])  = 1,    CPPM Shutdown Qualifier Enable for Clock Source Group 1 (CPPM_G1_QUAL)
  /// CPPM_CG_POL1A (0x1B24h[8:0]) = 008h, LTR Threshold for Clock Source Group 1 (LTR_G1_THRESH)
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CPPM_CG_POL1A,
    (UINT32) ~B_PMC_PWRM_CPPM_CG_POLXA_LTR_GX_THRESH,
    B_PMC_PWRM_CPPM_CG_POLXA_CPPM_GX_QUAL |
    0x008
    );
  ///
  /// CPPM_CG_POL2A (0x1B40h[30])  = 1,    CPPM Shutdown Qualifier Enable for Clock Source Group 2 (CPPM_G2_QUAL)
  /// CPPM_CG_POL2A (0x1B40h[29])  = 1,    ASLT/PLT Selection for Clock Source Group 1
  /// CPPM_CG_POL2A (0x1B40h[8:0]) = 008h, LTR Threshold for Clock Source Group 2 (LTR_G2_THRESH)
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CPPM_CG_POL2A,
    (UINT32) ~B_PMC_PWRM_CPPM_CG_POLXA_LTR_GX_THRESH,
    B_PMC_PWRM_CPPM_CG_POLXA_CPPM_GX_QUAL | B_PMC_PWRM_CPPM_CG_POLXA_LT_GX_SEL |
    0x008
    );
  ///
  /// CPPM_CG_POL3A (0x1BA8[30])  = 1,    CPPM Shutdown Qualifier Enable for Clock Source Group 3 (CPPM_G3_QUAL)
  /// CPPM_CG_POL3A (0x1BA8[29])  = 1,    ASLT/PLT Selection for Clock Source Group 1
  /// CPPM_CG_POL3A (0x1BA8[8:0]) = 029h, LTR Threshold for Clock Source Group 3 (LTR_G3_THRESH)
  ///
  Data32And = (UINT32) ~B_PMC_PWRM_CPPM_CG_POLXA_LTR_GX_THRESH;
  Data32Or = B_PMC_PWRM_CPPM_CG_POLXA_CPPM_GX_QUAL | B_PMC_PWRM_CPPM_CG_POLXA_LT_GX_SEL | 0x029;

  if (IsCnlPch ()) {
    ///
    /// Leave CPPM_CG_POL3A (0x1BA8[30]) cleared for CNL PCH
    ///
    Data32Or &= ~(UINT32)B_PMC_PWRM_CPPM_CG_POLXA_CPPM_GX_QUAL;
  }

  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CPPM_CG_POL3A,
    Data32And,
    Data32Or
    );
  ///
  /// PWRMBASE + 0x1B14 = 0x1E0A4616
  ///
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_OBFF_CFG, 0x1E0A4616);

  ///
  /// Configure SLP_S0 settings
  ///
  SlpS0Config (PmConfig, (PLATFORM_DEBUG_CONSENT_PROBE_TYPE) DciPreMemConfig->PlatformDebugConsent);

  ///
  /// CS_SD_CTL1 (0x1BE8[22:20]) = 010, Clock Source 5 Control Configuration (CS5_CTL_CFG)
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CS_SD_CTL1,
    (UINT32) ~(B_PMC_PWRM_CS_SD_CTL1_CS5_CTL_CFG | B_PMC_PWRM_CS_SD_CTL1_CS1_CTL_CFG),
    (2 << N_PMC_PWRM_CS_SD_CTL1_CS5_CTL_CFG)
    );

  ///
  /// Set CS_SD_CTL2.CS3_OVR_EN for platforms that do not require the use of OC PLL
  /// ICLK.ICLK_CMU_ONE_DWORD0.MUXSEL_CPUBCLK != 1 (OC_PLL)
  ///
  Data32 = (PchPcrRead32 (PID_ICLK, R_ICLK_PCR_CMU_ONE_DWORD0) & B_ICLK_PCR_CMU_ONE_DWORD0_MUXSEL_CPUBCLK) >> N_ICLK_PCR_CMU_ONE_DWORD0_MUXSEL_CPUBCLK;
  if (Data32 != V_ICLK_PCR_CMU_ONE_DWORD0_MUXSEL_CPUBCLK_OCPLL) {
    MmioOr32 (
      PchPwrmBase + R_PMC_PWRM_CS_SD_CTL2,
      B_PMC_PWRM_CS_SD_CTL2_CS3_OVR_EN
      );
  }

  if (IsPchH ()) {
    ///
    /// CMU_ONE_DWORD16 (0x22C0[28]) = 1, ck_hdapllbmc_modectrl
    ///
    PchPcrAndThenOr32 (
      PID_ICLK,
      R_ICLK_PCR_CMU_ONE_DWORD16,
      ~0u,
      B_ICLK_PCR_CMU_ONE_DWORD16_CK_HDAPLLBMC_MODECTRL
      );
  }

  ///
  /// Configure PWRMBASE + 0x1E00
  ///
  PmcConfigureRegPwrm1E00 ();

  ///
  /// PWRMBASE + 0x1E04[29] = 1
  /// PWRMBASE + 0x1E04[10:6] = 02h
  /// PWRMBASE + 0x1E04[3:0] = 2
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_1E04,
    (UINT32) ~((0x1F << 6) | 0xF),
    (BIT29 | (2 << 6) | 2)
    );

  ///
  /// PWRMBASE + 0x1BD0 [31,28,27,15] = 1b, 1b, 1b, 1b, 1b
  /// PWRMBASE + 0x1BD0 [21:16] = 0Ah
  /// PWRMBASE + 0x1BD0 [30] = 1b, done in a separate programming
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_1BD0,
    (UINT32) ~(0x3F << 16),
    BIT31 | BIT28 | BIT27 | BIT15 | (0x0A << 16)
    );
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_1BD0, BIT30);

  ///
  /// CPPM_MPG_POL1A (0x10E0h[30])  = 1,    CPPM Shutdown Qualifier Enable for ModPHY (CPPM_MODPHY_QUAL)
  /// CPPM_MPG_POL1A (0x10E0h[29])  = 1,    ASLT/PLT Selection for ModPHY (LT_MODPHY_SEL)
  /// CPPM_MPG_POL1A (0x10E0h[8:0]) = 069h, LTR Treshold for ModPHY (LTR_MODPHY_THRESH)
  ///
  MmioAndThenOr32 (
    PchPwrmBase + R_PMC_PWRM_CPPM_MPG_POL1A,
    (UINT32) ~B_PMC_PWRM_CPPM_MPG_POL1A_LTR_MODPHY_THRESH,
    B_PMC_PWRM_CPPM_MPG_POL1A_CPPM_MODPHY_QUAL |
    B_PMC_PWRM_CPPM_MPG_POL1A_LT_MODPHY_SEL |
    0x69
    );

  ///
  /// Handling Status Registers
  /// System BIOS must set 1b to clear the following registers during power-on
  /// and resuming from Sx sleep state.
  /// - PWRMBASE + PRSTS[0] = 1b
  /// - PWRMBASE + PRSTS[4] = 1b
  /// - PWRMBASE + PRSTS[5] = 1b
  ///
  Data32 = B_PMC_PWRM_PRSTS_FIELD_1;
  ///
  /// Clear power / reset status bits on PCH Corporate
  ///
  if (PmConfig->MeWakeSts) {
    Data32 |= B_PMC_PWRM_PRSTS_ME_WAKE_STS;
  }
  if (PmConfig->WolOvrWkSts) {
    Data32 |= B_PMC_PWRM_PRSTS_WOL_OVR_WK_STS;
  }

  MmioOr32 (PchPwrmBase + R_PMC_PWRM_PRSTS, Data32);

  ///
  /// We need to enable LAN_WAKE_PIN_DSX_EN for Wake from both SX and DSX
  ///
  Data32 = MmioRead32 (PchPwrmBase + R_PMC_PWRM_DSX_CFG);
  if (PmConfig->WakeConfig.LanWakeFromDeepSx == TRUE) {
    Data32 |= B_PMC_PWRM_DSX_CFG_LAN_WAKE_EN;
  } else {
    Data32 &= ~B_PMC_PWRM_DSX_CFG_LAN_WAKE_EN;
  }
  //
  // Disable PCH internal AC PRESENT pulldown
  //
  if (PmConfig->DisableDsxAcPresentPulldown) {
    Data32 |= B_PMC_PWRM_DSX_CFG_ACPRES_PD_DSX_DIS;
  } else {
    Data32 &= ~B_PMC_PWRM_DSX_CFG_ACPRES_PD_DSX_DIS;
  }
  ///
  /// Enable WAKE_PIN__DSX_EN for Wake
  ///
  if (PmConfig->WakeConfig.PcieWakeFromDeepSx) {
    Data32 |= B_PMC_PWRM_DSX_CFG_WAKE_PIN_DSX_EN;
  } else {
    Data32 &= ~B_PMC_PWRM_DSX_CFG_WAKE_PIN_DSX_EN;
  }
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_DSX_CFG, Data32);

  ///
  /// Handle wake policy
  ///
  PmConA = MmioRead32 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_A);
  PmConB = MmioRead32 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_B);

  //
  // Don't clear B_PMC_PWRM_GEN_PMCON_A_GBL_RST_STS, B_PMC_PWRM_GEN_PMCON_A_MS4V, B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR
  // B_PMC_PWRM_GEN_PMCON_A_PWR_FLR, B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS by accident since those are RW/1C.
  //
  PmConA &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_GBL_RST_STS | B_PMC_PWRM_GEN_PMCON_A_MS4V | B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR | B_PMC_PWRM_GEN_PMCON_A_PWR_FLR | B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS);

  PmConA &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_PME_B0_S5_DIS);
  PmConB &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_B_WOL_EN_OVRD);

  if (PmConfig->WakeConfig.PmeB0S5Dis) {
    PmConA |= B_PMC_PWRM_GEN_PMCON_A_PME_B0_S5_DIS;
  }

  if (PmConfig->WakeConfig.WolEnableOverride) {
    ///
    /// Wake-On-LAN (WOL) Implementation
    /// Step 1
    /// Clear PWRMBASE + GEN_PMCON_A[30] = 0b to ensure the LAN PHY will be powered for WOL
    /// when the power source is either the AC or the DC battery.
    ///
    PmConA &= (UINT32) ~B_PMC_PWRM_GEN_PMCON_A_DC_PP_DIS;

    ///
    /// Step 2
    /// Clear PWRMBASE + GEN_PMCON_A[29] = 0b to ensure the LAN PHY will be powered for WOL in Deep Sx.
    ///
    PmConA &= (UINT32) ~B_PMC_PWRM_GEN_PMCON_A_DSX_PP_DIS;

    ///
    /// Step 3
    /// Set PWRMBASE + GEN_PMCON_A[28] = 1b to ensure the LAN PHY will be powered for WOL after a G3 transition.
    ///
    PmConA |= (UINT32) B_PMC_PWRM_GEN_PMCON_A_AG3_PP_EN;

    ///
    /// Step 4
    /// Set PWRMBASE + GEN_PMCON_A[27] = 1b to ensure the LAN PHY will be powered for WOL from Sx.
    ///
    PmConA |= (UINT32) B_PMC_PWRM_GEN_PMCON_A_SX_PP_EN;

    ///
    /// Step 5
    /// "PME_B0_EN", ABASE + Offset 28h[13], bit must be programmed to enable wakes
    /// from S1-S4 at the Power Management Controller
    /// Done in ASL code(_PRW)
    ///
    ///
    /// Step 6
    /// Set "WOL Enable Override", PWRMBASE + GEN_PMCON_B[13], bit to 1b to guarantee the
    /// LAN-Wakes are enabled at the Power Management Controller, even in surprise
    /// S5 cases such as power loss/return and Power Button Override
    ///
    PmConB |= B_PMC_PWRM_GEN_PMCON_B_WOL_EN_OVRD;

    ///
    /// Step 7
    /// Moreover, system BIOS also require to enables in the LAN device by performing
    /// the WOL configuration requirements in the GbE region of the SPI flash.
    /// Done in PchSmmSxGoToSleep() SMM handler.
    ///
  } else {
    ///
    /// PWRMBASE + GEN_PMCON_A[30:27] and PWRMBASE + GEN_PMCON_B[13] are all in RTC or DSW well, so BIOS also
    /// needs to program them while WOL setup option is disabled.
    ///
    PmConA &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_AG3_PP_EN | B_PMC_PWRM_GEN_PMCON_A_SX_PP_EN);
    PmConA |= (B_PMC_PWRM_GEN_PMCON_A_DC_PP_DIS | B_PMC_PWRM_GEN_PMCON_A_DSX_PP_DIS);

    PmConB &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_B_WOL_EN_OVRD);
  }

  ///
  /// Configure On DC PHY Power Disable according to policy SlpLanLowDc.
  /// When this bit is set, SLP_LAN# will be driven low when ACPRESENT is low.
  /// This indicates that LAN PHY should be powered off on battery mode.
  /// This will override the DC_PP_DIS setting by WolEnableOverride.
  ///
  if (PmConfig->SlpLanLowDc) {
    PmConA |= (UINT32) (B_PMC_PWRM_GEN_PMCON_A_DC_PP_DIS);
  } else {
    PmConA &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_DC_PP_DIS);
  }

  ///
  /// Enabling SLP_S3# and SLP_S4# Stretch
  /// PWRMBASE + GEN_PMCON_A[12:10]
  /// PWRMBASE + GEN_PMCON_A[5:3]
  ///
  PmConA &= (UINT32) ~(B_PMC_PWRM_GEN_PMCON_A_SLP_S3_MAW | B_PMC_PWRM_GEN_PMCON_A_SLP_S4_MAW);

  switch (PmConfig->PchSlpS3MinAssert) {
    case PchSlpS360us:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S3_MAW_60US;
      break;

    case PchSlpS31ms:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S3_MAW_1MS;
      break;

    case PchSlpS350ms:
    default:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S3_MAW_50MS;
      break;

    case PchSlpS32s:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S3_MAW_2S;
      break;
  }

  switch (PmConfig->PchSlpS4MinAssert) {
    case PchSlpS4PchTime:
      PmConA &= (UINT32) (~B_PMC_PWRM_GEN_PMCON_A_SLP_S4_ASE);
      break;

    case PchSlpS41s:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S4_MAW_1S | B_PMC_PWRM_GEN_PMCON_A_SLP_S4_ASE;
      break;

    case PchSlpS42s:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S4_MAW_2S | B_PMC_PWRM_GEN_PMCON_A_SLP_S4_ASE;
      break;

    case PchSlpS43s:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S4_MAW_3S | B_PMC_PWRM_GEN_PMCON_A_SLP_S4_ASE;
      break;

    case PchSlpS44s:
    default:
      PmConA |= V_PMC_PWRM_GEN_PMCON_A_SLP_S4_MAW_4S | B_PMC_PWRM_GEN_PMCON_A_SLP_S4_ASE;
      break;
  }

  if (PmConfig->SlpStrchSusUp == FALSE) {
    PmConA |= B_PMC_PWRM_GEN_PMCON_A_DISABLE_SX_STRETCH;
  } else {
    PmConA &= (UINT32)~B_PMC_PWRM_GEN_PMCON_A_DISABLE_SX_STRETCH;
  }
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_A, PmConA);
  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_B, PmConB);

  ///
  /// Set CF9LOCK (PWRMBASE + ETR3[31]  = 1b)
  /// Done in Intel Management Engine Framework Reference Code
  ///

  ///
  /// Note: PWRMBASE + PM_CFG[19:16] are platform dependent settings (0Fh provides longest assertion),
  /// please consult with your board design engineers for correct values to be programmed to.
  ///
  /// For PWRMBASE + PM_CFG[9:8] Reset Power Cycle Duration could be customized, please refer to EDS
  /// and make sure the setting correct, which never less than the following register.
  /// - GEN_PMCON_3.SLP_S3_MIN_ASST_WDTH
  /// - GEN_PMCON_3.SLP_S4_MIN_ASST_WDTH
  /// - PM_CFG.SLP_A_MIN_ASST_WDTH
  /// - PM_CFG.SLP_LAN_MIN_ASST_WDTH
  ///
  Data32 = MmioRead32 (PchPwrmBase + R_PMC_PWRM_CFG);
  Data32 &= (UINT32) ~(B_PMC_PWRM_CFG_SSMAW_MASK | B_PMC_PWRM_CFG_SAMAW_MASK | B_PMC_PWRM_CFG_RPCD_MASK);
  switch (PmConfig->PchSlpSusMinAssert) {
    case PchSlpSus0ms:
      Data32 |= V_PMC_PWRM_CFG_SSMAW_0S;
      break;

    case PchSlpSus500ms:
      Data32 |= V_PMC_PWRM_CFG_SSMAW_0_5S;
      break;

    case PchSlpSus1s:
      Data32 |= V_PMC_PWRM_CFG_SSMAW_1S;
      break;

    case PchSlpSus4s:
    default:
      Data32 |= V_PMC_PWRM_CFG_SSMAW_4S;
      break;
  }
  switch (PmConfig->PchSlpAMinAssert) {
    case PchSlpA0ms:
      Data32 |= V_PMC_PWRM_CFG_SAMAW_0S;
      break;

    case PchSlpA4s:
      Data32 |= V_PMC_PWRM_CFG_SAMAW_4S;
      break;

    case PchSlpA98ms:
      Data32 |= V_PMC_PWRM_CFG_SAMAW_98ms;
      break;

    case PchSlpA2s:
    default:
      Data32 |= V_PMC_PWRM_CFG_SAMAW_2S;
      break;
  }
  switch (PmConfig->PchPwrCycDur) {
    case 0:  // treat as PCH default
      Data32 |= V_PMC_PWRM_CFG_RPCD_4S;
      break;

    case 1:
      Data32 |= V_PMC_PWRM_CFG_RPCD_1S;
      break;

    case 2:
      Data32 |= V_PMC_PWRM_CFG_RPCD_2S;
      break;

    case 3:
      Data32 |= V_PMC_PWRM_CFG_RPCD_3S;
      break;

    case 4:
      Data32 |= V_PMC_PWRM_CFG_RPCD_4S;
      break;

    default:
      Data32 |= V_PMC_PWRM_CFG_RPCD_4S;
      DEBUG ((DEBUG_ERROR, "Invalid value for PchPwrCycDur. Using 4Sec as the default value.\n"));
      break;
  }

  //
  // Enable Global Reset on uncorrectable error on PMC SRAM interface
  // Allow 24MHz Crystal Oscillator Shutdown, Please note: below programming must be done after USB2 GLOBAL PORT 2 programming
  // Allow USB2 PHY Core Power Gating
  //
  Data32 |= (B_PMC_PWRM_CFG_EN_PMC_UNC_ERR | B_PMC_PWRM_CFG_ALLOW_USB2_CORE_PG | B_PMC_PWRM_CFG_ALLOW_24_OSC_SD);

  //
  // Clear CPU OC Strap
  //
  Data32 &= (UINT32) ~(B_PMC_PWRM_CFG_COCS);

  MmioWrite32 (PchPwrmBase + R_PMC_PWRM_CFG, Data32);


  //
  // Configure VrAlert GPIO pin
  //
  if (PmConfig->VrAlert) {
    MmioOr32 (PchPwrmBase + R_PMC_PWRM_THROT_1, B_PMC_PWRM_THROT_1_VR_ALERT);
    GpioEnableVrAlert ();
  } else {
    MmioAnd32 (PchPwrmBase + R_PMC_PWRM_THROT_1, ~(UINT32)B_PMC_PWRM_THROT_1_VR_ALERT);
  }

  //
  // Configure Energy Report
  //
  ConfigureEnergyReport (SiPolicy);

  //
  // Deep Sx Enabling
  //
  ConfigureDeepSxSettings (SiPolicy);

  //
  // Configure SCI interrupt
  //
  PmcSetSciIrq (ItssGetPmcSciDevIntConfig (SiPolicy));

}
