/** @file
  CpuPowerOnConfig PEI Library

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

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Private/Library/CpuPowerOnConfigLib.h>
#include <Private/Library/PchRcLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/CpuCommonLib.h>
#include <Ppi/MasterBootMode.h>
#include <Register/Cpuid.h>
#include <Register/Msr.h>
#include <CpuAccess.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>

/**
  Check if VT is fused and disabled by Setup Option so reset is needed.

  @param[in] VmxEnable - VmxEnable CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval COLD_RESET        - Update requires a cold reset.
**/
CPU_RESET_TYPE
CheckVmxIfNeedReset (
  IN UINT8         VmxEnable
  )
{
  MSR_IA32_FEATURE_CONTROL_REGISTER Msr;
  CPUID_VERSION_INFO_ECX            Ecx;

  AsmCpuid (CPUID_VERSION_INFO, &Ecx.Uint32, NULL, NULL, NULL);

  ///
  /// Check if VMX is supported
  ///
  if (Ecx.Bits.VMX == 0) {
    return NO_RESET;
  }

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_FEATURE_CONTROL);

  ///
  /// Need to reset only if locked and VMX state has to be changed.
  ///
  if ((Msr.Bits.Lock == 1) && (Msr.Bits.EnableVmxOutsideSmx != VmxEnable)) {
    return COLD_RESET;
  }

  return NO_RESET;
}

/**
  Compare the flex multiplier setup options against the CPU strap settings
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] CpuRatio - CpuRatio CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
**/
CPU_RESET_TYPE
ProgramProcessorFlexMultiplier (
  IN CPU_STRAP_SET               *CpuStrapSet,
  IN UINT8                       CpuRatio
  )
{
  UINT64              FlexRatioData;
  UINT64              CurrentClockFlexRatio;
  UINT64              MsrData;
  CPU_RESET_TYPE      ResetType;

  ResetType         = NO_RESET;

  FlexRatioData = (UINT64) CpuRatio;

  DEBUG ((DEBUG_INFO, "Ratio from Policy is 0x%X\n", FlexRatioData));

  ///
  /// Read and save current Flex Ratio data; disregard enable bit (MSR 194h [15:8])
  ///
  MsrData               = AsmReadMsr64 (MSR_FLEX_RATIO);
  CurrentClockFlexRatio = ((RShiftU64 (MsrData, N_FLEX_RATIO)) & 0xFF);
  DEBUG ((DEBUG_INFO, "Current Flex Ratio from MSR is 0x%X\n", CurrentClockFlexRatio));
  DEBUG ((DEBUG_INFO, "Current Flex Ratio from CPU Strap: 0x%X\n", CpuStrapSet->FlexRatio));
  ///
  /// Check and set Flex Ratio to requested ratio if possible
  ///
  if (FlexRatioData == CpuStrapSet->FlexRatio) {
    ///
    /// Do nothing, ratio is already set to requested value and enabled
    ///
    DEBUG ((DEBUG_INFO, "No need to set Flex Ratio.\n"));
  } else {
    ///
    /// Set Flex Ratio to user selected value
    ///
    /// Write new data to Flex Ratio register
    /// First clear MSR of any previous value
    ///
    MsrData &= RATIO_FLEX_CLEAR_MASK;

    ///
    /// Enter the new data
    ///
    MsrData |= (LShiftU64 ((FlexRatioData & 0xFF), N_FLEX_RATIO));
    MsrData |= B_FLEX_EN;
    AsmWriteMsr64 (MSR_FLEX_RATIO, MsrData);

    ///
    /// Set Soft Reset Data for Flex Ratio
    ///
    CpuStrapSet->FlexRatio = (UINT32) FlexRatioData;

    ///
    /// Set RESET flag
    ///
    DEBUG ((DEBUG_INFO, "Setting Flex Ratio to 0x%X\n", CpuStrapSet->FlexRatio));
    ResetType |= WARM_RESET;
  }

  return ResetType;
}


/**
  Compare the BIST setup option against the CPU strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] BistOnResetPostMem - BistOnResetPostMem CPU Test Config Policy.

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
**/
CPU_RESET_TYPE
BistEnableDisable (
  IN CPU_STRAP_SET     *CpuStrapSet,
  IN UINT8             BistOnReset
  )
{
  if (CpuStrapSet->Bist == BistOnReset) {
    return NO_RESET;
  } else {
    CpuStrapSet->Bist = BistOnReset;
    DEBUG ((DEBUG_INFO, "BIST configuration doesn't match the setup value\n"));
    return WARM_RESET;
  }
}

/**
  Compare the hyper threading setup option against the CPU strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] CpuConfigLibPreMemConfig  - CPU Config Lib PreMem Config Block

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
**/
CPU_RESET_TYPE
HtEnableDisable (
  IN CPU_STRAP_SET     *CpuStrapSet,
  IN UINT8             HyperThreading
  )
{
  CPU_RESET_TYPE     ResetType;
  UINT16             TotalThreadsPerCore;
  EFI_CPUID_REGISTER Cpuid0B = { 0, 0, 0, 0 };

  ResetType = NO_RESET;

  ///
  /// Read CPUID(0xB) with ECX=0 for number of logical processors per Core.
  /// This value does NOT change based on Intel HT Technology disable and core disables.
  ///
  Cpuid0B.RegEcx = 0;
  AsmCpuidEx (CPUID_EXTENDED_TOPOLOGY, 0, NULL, &Cpuid0B.RegEbx, NULL, NULL);
  TotalThreadsPerCore = (UINT16) Cpuid0B.RegEbx;

  if (TotalThreadsPerCore > 1) {
    ///
    /// HT is supported
    ///
    DEBUG ((DEBUG_INFO, "HT is supported\n"));
    ///
    /// Check if the configuration of HT matches the BIOS Setup option.
    ///
    if (CpuStrapSet->HtDisabled == HyperThreading) {
      ///
      /// HT configuration doesn't match BIOS Setup option; update the Strap Set Data and Issue a Warm reset
      ///
      DEBUG ((DEBUG_INFO, "HT configuration doesn't match the setup value\n"));
      CpuStrapSet->HtDisabled = !(HyperThreading);
      ResetType |= WARM_RESET;
    }
  } else {
    ///
    /// HT is not supported by default fusing.
    ///
    DEBUG ((DEBUG_WARN, "HT is NOT supported\n"));
    CpuStrapSet->HtDisabled = 1;
  }

  return ResetType;
}

/**
  Compare the number of active cores setup option against the CPU strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] ActiveCoreCount - ActiveCoreCount CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
  @retval COLD_RESET        - Update requires a cold reset.
**/
CPU_RESET_TYPE
SetActiveCores (
  IN CPU_STRAP_SET     *CpuStrapSet,
  IN UINT8             ActiveCoreCount
  )
{
  CPU_RESET_TYPE     ResetType;
  UINT32             Data32;
  UINT32             NumberOfActiveCores;
  UINT32             NumberOfActiveThreads;

  ResetType = NO_RESET;

  /* CORE_THREAD_COUNT - msr 0x35
     Symbol      Name        MSB:LSB  Description
     CoreCount   CoreCount   31:16    The Core Count reflects the enabled cores based
                                      on the above thread count and the value of threads_
                                      available (to determine if HT is on) at reset time.

     ThreadCount ThreadCount 15:0     The Thread Count reflects the enabled threads based
                                      on the factory-configured thread count and the value
                                      of the PCH Soft Reset Data register for Client processors
                                      at reset time.

  Read MSR for Active Core and Thread Count.
  */
  Data32                = (UINT32) AsmReadMsr64 (MSR_CORE_THREAD_COUNT);
  NumberOfActiveCores   = (UINT32) ((Data32 >> 16) & 0xFFFF);
  NumberOfActiveThreads = (UINT32) (Data32 & 0xFFFF);

  DEBUG ((DEBUG_INFO, "Number of Active Cores / Threads = %x / %x\n", NumberOfActiveCores, NumberOfActiveThreads));

  ///
  /// Check if the configuration of "Active Core" matches the BIOS Setup option.
  ///
  if (CpuStrapSet->NumberOfActiveCores != ActiveCoreCount) {
    DEBUG (
            (DEBUG_INFO,
             "NumberOfActiveCores doesn't match the value from Setup = %x / %x\n",
             CpuStrapSet->NumberOfActiveCores,
             ActiveCoreCount)
            );
    CpuStrapSet->NumberOfActiveCores = ActiveCoreCount;
    ResetType |= COLD_RESET;
  }

  return ResetType;
}

/**
  Compare the boot frequency setup option against the boot ratio strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] BootFrequency  - BootFrequency CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
**/
CPU_RESET_TYPE
BootRatioEnableDisable (
  IN CPU_STRAP_SET       *CpuStrapSet,
  IN UINT8               BootFrequency
  )
{
  CPU_RESET_TYPE ResetType;
  ResetType = NO_RESET;

  ///
  /// Check if the configuration of BootRatio from Bit12 of strap setting is not aligned with the BootFrequency setup option.
  ///
  if (((CpuStrapSet->BootRatio == 1) && (BootFrequency == 0)) ||
      ((CpuStrapSet->BootRatio == 0) && (BootFrequency > 0))) {
      DEBUG (
              (DEBUG_INFO,
               "Boot Ratio strap setting of %x does not match the BootFrequency policy %x\n",
               CpuStrapSet->BootRatio,
               BootFrequency)
              );
      if (BootFrequency > 0) {
        CpuStrapSet->BootRatio = 1;
      } else {
        CpuStrapSet->BootRatio = 0;
      }
      ResetType |= WARM_RESET;
  }

  return ResetType;
}

/**
  Compare the JTAG power gate setup option against the CPU strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] JtagC10PowerGateDisable - JtagC10PowerGateDisable CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval WARM_RESET        - Update requires a warm reset.
**/
CPU_RESET_TYPE
JtagPowerGateEnableDisable (
  IN CPU_STRAP_SET           *CpuStrapSet,
  IN UINT8                   JtagC10PowerGateDisable
  )
{
  CPU_RESET_TYPE ResetType;

  ResetType = NO_RESET;

  ///
  /// Check if the configuration of "JtagC10PowerGateDisable" from Bit13 of strap setting matches the BIOS Setup option.
  ///
  if (CpuStrapSet->JtagC10PowerGateDisable != JtagC10PowerGateDisable) {
      DEBUG (
              (DEBUG_INFO,
               "JtagC10PowerGateDisable strap setting doesn't match the value from Setup = %x / %x\n",
               CpuStrapSet->JtagC10PowerGateDisable,
               JtagC10PowerGateDisable)
              );
      CpuStrapSet->JtagC10PowerGateDisable = JtagC10PowerGateDisable;
      ResetType |= WARM_RESET;
  }

  return ResetType;
}

/**
  Compare the display port SSC margin setup option against the CPU strap setting
  and in case of mismatch request a reset.

  @param[in] CpuStrapSet    - The current strap setting.
  @param[in] DpSscMarginEnable - DpSscMarginEnable CPU policy.

  @retval NO_RESET          - No reset is needed.
  @retval COLD_RESET        - Update requires a cold reset.
**/
CPU_RESET_TYPE
DpSscMarginEnableDisable (
  IN CPU_STRAP_SET           *CpuStrapSet,
  IN UINT8                   DpSscMarginEnable
  )
{
  CPU_RESET_TYPE ResetType;

  ResetType = NO_RESET;

  ///
  /// Check if the configuration of "DpSscMarginEnable" from Bit15 of strap setting matches the BIOS Setup option.
  ///
  if (CpuStrapSet->DpSscMarginEnable != DpSscMarginEnable) {
      DEBUG (
              (DEBUG_INFO,
               "DpSscMarginEnable strap setting doesn't match the value from Setup = %x / %x\n",
               CpuStrapSet->DpSscMarginEnable,
               DpSscMarginEnable)
              );
      CpuStrapSet->DpSscMarginEnable = DpSscMarginEnable;
      ResetType |= COLD_RESET;
  }

  return ResetType;
}

/**
  Set Cpu Strap and Set Early Power On Configuration setting for feature change.

  @param[in] SiPreMemPolicyPpi    - Address of the SI_PREMEM_POLICY_PPI.

  @retval EFI_SUCCESS       - Function completed successfully.
**/
EFI_STATUS
SetCpuStrapAndEarlyPowerOnConfig (
  IN SI_PREMEM_POLICY_PPI       *SiPreMemPolicyPpi
  )
{
  EFI_STATUS                  Status;
  CPU_RESET_TYPE              ResetType;
  CPU_STRAP_SET               CpuStrapSet = { 0 };
  UINT16                      CpuStrapSetData;
  EFI_BOOT_MODE               BootMode;
  CPU_CONFIG_LIB_PREMEM_CONFIG *CpuConfigLibPreMemConfig;
  CPU_GENERATION              CpuGeneration;
  CPU_SKU                     CpuSku;
  BOOLEAN                     SkipEpocConfig;
  CPU_EPOC_SET                CpuEpocSet;
  UINT32                      CpuEpocData;

  SkipEpocConfig = FALSE;
  CpuSku = GetCpuSku();

  ResetType         = NO_RESET;
  CpuStrapSetData   = 0;

  DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Start \n"));
  PostCode (0xC0A);

  Status = PeiServicesGetBootMode (&BootMode);
  DEBUG ((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig: BootMode = %X\n", BootMode));
  if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
    //
    // Lock CPU Straps
    //
    DEBUG ((DEBUG_INFO, "CpuStrap And EarlyPowerOnConfig doesn't execute during S3 resume path, only locking straps\n"));
    Status = PchCpuStrapSet (LockCpuStrapSetData, NULL);
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_INFO, "Set CPU Soft Straps Entry Point\n"));

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuConfigLibPreMemConfigGuid, (VOID *) &CpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  ///
  /// Get Cpu Strap Set Data
  ///
  Status                        = PchCpuStrapSet (GetCpuStrapSetData, &CpuStrapSetData);
  *((UINT16 *) (&CpuStrapSet))  = CpuStrapSetData;
  DEBUG ((DEBUG_INFO, "Current CPU Strap Data = 0x%04X\n", CpuStrapSetData));

  if (Status == EFI_SUCCESS) {
    ResetType |= HtEnableDisable (&CpuStrapSet, (UINT8)CpuConfigLibPreMemConfig->HyperThreading);
    ResetType |= SetActiveCores (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->ActiveCoreCount);
    ResetType |= BistEnableDisable (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->BistOnReset);
    ResetType |= DpSscMarginEnableDisable (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->DpSscMarginEnable);

    ///
    /// Perform Flex Ratio if processor is fused to perform Flex Ratio
    ///
    if ((AsmReadMsr64 (MSR_FLEX_RATIO) & B_FLEX_EN) == B_FLEX_EN) {
      ResetType |= ProgramProcessorFlexMultiplier (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->CpuRatio);
    }
    ResetType |= BootRatioEnableDisable (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->BootFrequency);
    ResetType |= JtagPowerGateEnableDisable (&CpuStrapSet, (UINT8) CpuConfigLibPreMemConfig->JtagC10PowerGateDisable);

    if (ResetType != NO_RESET) {
      CpuStrapSetData = *((UINT16 *) (&CpuStrapSet));
      DEBUG ((DEBUG_INFO, "New CPU Strap Data = 0x%04X\n", CpuStrapSetData));

      Status = PchCpuStrapSet (SetCpuStrapSetData, &CpuStrapSetData);

      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "NO Reset\n"));
        ResetType = NO_RESET;
      }
    }
  }

  CpuGeneration = GetCpuGeneration();
  if(CpuGeneration == EnumCflCpu){
    CpuEpocSet.Uint32 = 0;
    CpuEpocData       = 0;

    //
    // Early Power On Configuration
    //
    DEBUG ((DEBUG_INFO, "Set Early Power On Configuration\n"));
    CpuEpocSet.Bits.Fclk = CpuConfigLibPreMemConfig->FClkFrequency;

    //
    // Skip EPOC Configuration if Processor family doesn't support the policy value.
    //
    if (((CpuSku == EnumCpuUlt) || (CpuSku == EnumCpuUlx)) && (CpuEpocSet.Uint32 == 1)) {
      //
      // ULT/ULX does not support 1GHz FCLK setting.
      //
      SkipEpocConfig = TRUE;
    }
    Status = PchCpuEpocSet (GetCpuEpocData, &CpuEpocData);

    if ((Status == EFI_SUCCESS) && (CpuEpocData != CpuEpocSet.Uint32) && (SkipEpocConfig == FALSE)) {
      DEBUG ((DEBUG_INFO, "Current EPOC Data = 0x%04X\n", CpuEpocData));
      CpuEpocData = CpuEpocSet.Uint32;
      DEBUG ((DEBUG_INFO, "Setting EPOC Data = 0x%04X\n", CpuEpocData));
      Status = PchCpuEpocSet (SetCpuEpocData, &CpuEpocData);
      if (Status == EFI_SUCCESS) {
        ResetType = COLD_RESET;
      }
    } else {
      DEBUG ((DEBUG_INFO, "Skipping EPOC Configuration.\n"));
    }
  }

  if (CpuConfigLibPreMemConfig->SkipMpInit == 0) {
    ResetType |= CheckVmxIfNeedReset ((UINT8) CpuConfigLibPreMemConfig->VmxEnable);
  }

  if (ResetType != NO_RESET) {
    ///
    /// Perform Warm or Cold Reset
    ///
    DEBUG (
            (DEBUG_INFO,
             "Reset Required. Performing Cold/Warm Reset to read the new strap and/or EPOC values - ResetType = %x\n",
             ResetType)
            );
  DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Reset \n"));
  PostCode (0xC13);

    PerformWarmOrColdReset (ResetType);
  }

  Status = PchCpuStrapSet (LockCpuStrapSetData, &CpuStrapSetData);

  DEBUG((DEBUG_INFO, "SetCpuStrapAndEarlyPowerOnConfig Done \n"));
  PostCode (0xC14);

  return EFI_SUCCESS;
}
