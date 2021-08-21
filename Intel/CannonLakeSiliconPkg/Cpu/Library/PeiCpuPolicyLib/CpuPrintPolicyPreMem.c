/** @file
  This file is PeiCpuPolicy library.

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
#include "PeiCpuPolicyLibrary.h"
#include <Library/ConfigBlockLib.h>

/**
  Print CPU_SECURITY_PREMEM_CONFIG and serial out.

  @param[in] CpuSecurityPreMemConfig     Pointer to a CPU_SECURITY_PREMEM_CONFIG
**/
VOID
CpuSecurityPreMemConfigPrint (
  IN CONST CPU_SECURITY_PREMEM_CONFIG        *CpuSecurityPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ CPU Security PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : PrmrrSize = 0x%x\n", CpuSecurityPreMemConfig->PrmrrSize));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : BiosSize = 0x%x\n", CpuSecurityPreMemConfig->BiosSize));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : BiosGuard = 0x%x\n", CpuSecurityPreMemConfig->BiosGuard));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : BiosGuardToolsInterface = 0x%x\n", CpuSecurityPreMemConfig->BiosGuardToolsInterface));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : EnableSgx = 0x%x\n", CpuSecurityPreMemConfig->EnableSgx));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : Txt = 0x%x\n", CpuSecurityPreMemConfig->Txt));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : SkipStopPbet = 0x%x\n", CpuSecurityPreMemConfig->SkipStopPbet));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : EnableC6Dram = 0x%x\n", CpuSecurityPreMemConfig->EnableC6Dram));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : ResetAux = 0x%x\n", CpuSecurityPreMemConfig->ResetAux));
  DEBUG ((DEBUG_INFO, "CPU_SECURITY_PREMEM_CONFIG : TxtAcheckRequest = 0x%x\n", CpuSecurityPreMemConfig->TxtAcheckRequest));
}

/**
  Print CPU_TXT_PREMEM_CONFIG and serial out.

  @param[in] CpuTxtPreMemConfig     Pointer to a CPU_TXT_PREMEM_CONFIG
**/
VOID
CpuTxtPreMemConfigPrint (
  IN CONST CPU_TXT_PREMEM_CONFIG        *CpuTxtPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ CPU TXT PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : SinitMemorySize = 0x%x\n", CpuTxtPreMemConfig->SinitMemorySize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TxtHeapMemorySize = 0x%x\n", CpuTxtPreMemConfig->TxtHeapMemorySize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TgaSize = 0x%x\n", CpuTxtPreMemConfig->TgaSize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TxtLcpPdBase = 0x%x\n", CpuTxtPreMemConfig->TxtLcpPdBase));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TxtLcpPdSize = 0x%x\n", CpuTxtPreMemConfig->TxtLcpPdSize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TxtDprMemoryBase = 0x%x\n", CpuTxtPreMemConfig->TxtDprMemoryBase));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : TxtDprMemorySize = 0x%x\n", CpuTxtPreMemConfig->TxtDprMemorySize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : BiosAcmSize = 0x%x\n", CpuTxtPreMemConfig->BiosAcmSize));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : BiosAcmBase = 0x%x\n", CpuTxtPreMemConfig->BiosAcmBase));
  DEBUG ((DEBUG_INFO, "CPU_TXT_PREMEM_CONFIG : ApStartupBase = 0x%x\n", CpuTxtPreMemConfig->ApStartupBase));
}

/**
  Print CPU_CONFIG_LIB_PREMEM_CONFIG and serial out.

  @param[in] CpuConfigLibPreMemConfig     Pointer to a CPU_CONFIG_LIB_PREMEM_CONFIG

**/
VOID
CpuConfigLibPreMemConfigPrint (
  IN CONST CPU_CONFIG_LIB_PREMEM_CONFIG        *CpuConfigLibPreMemConfig
  )
{
  CPU_GENERATION CpuGeneration;
  CpuGeneration = GetCpuGeneration();
  DEBUG ((DEBUG_INFO, "------------------ CPU Config Lib PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : HyperThreading = 0x%x\n", CpuConfigLibPreMemConfig->HyperThreading));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : BootFrequency = 0x%x\n", CpuConfigLibPreMemConfig->BootFrequency));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : ActiveCoreCount = 0x%x\n", CpuConfigLibPreMemConfig->ActiveCoreCount));
  if(CpuGeneration == EnumCflCpu){
    DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : FClkFrequency = 0x%x\n", CpuConfigLibPreMemConfig->FClkFrequency));
  }
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : JtagC10PowerGateDisable = 0x%x\n", CpuConfigLibPreMemConfig->JtagC10PowerGateDisable));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : BistOnReset = 0x%x\n", CpuConfigLibPreMemConfig->BistOnReset));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : VmxEnable = 0x%x\n", CpuConfigLibPreMemConfig->VmxEnable));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : CpuRatio = 0x%x\n", CpuConfigLibPreMemConfig->CpuRatio));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : PeciSxReset = 0x%x\n", CpuConfigLibPreMemConfig->PeciSxReset));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : PeciC10Reset = 0x%x\n", CpuConfigLibPreMemConfig->PeciC10Reset));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : SkipMpInit = 0x%x\n", CpuConfigLibPreMemConfig->SkipMpInit));
  DEBUG ((DEBUG_INFO, "CPU_CONFIG_LIB_PREMEM_CONFIG : DpSscMarginEnable = 0x%x\n", CpuConfigLibPreMemConfig->DpSscMarginEnable));
}

/**
  Print CPU_OVERCLOCKING_PREMEM_CONFIG and serial out.

  @param[in] CpuOverClockingConfig   Pointer to a CPU_OVERCLOCKING_CONFIG
**/
VOID
CpuOverClockingPreMemConfigPrint (
  IN CONST CPU_OVERCLOCKING_PREMEM_CONFIG   *CpuOverClockingPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ CPU OverClocking Config ------------------\n"));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: OcSupport : 0x%X\n", CpuOverClockingPreMemConfig->OcSupport));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: OcLock : 0x%X\n", CpuOverClockingPreMemConfig->OcLock));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CoreVoltageMode : 0x%X\n", CpuOverClockingPreMemConfig->CoreVoltageMode));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CoreMaxOcRatio  : 0x%X\n", CpuOverClockingPreMemConfig->CoreMaxOcRatio));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CoreVoltageOverride : 0x%X\n", CpuOverClockingPreMemConfig->CoreVoltageOverride));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CoreVoltageAdaptive : 0x%X\n", CpuOverClockingPreMemConfig->CoreVoltageAdaptive));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CoreVoltageOffset : 0x%X\n", CpuOverClockingPreMemConfig->CoreVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingVoltageMode : 0x%X\n", CpuOverClockingPreMemConfig->RingVoltageMode));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingVoltageOverride : 0x%X\n", CpuOverClockingPreMemConfig->RingVoltageOverride));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingVoltageAdaptive : 0x%X\n", CpuOverClockingPreMemConfig->RingVoltageAdaptive));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingVoltageOffset : 0x%X\n", CpuOverClockingPreMemConfig->RingVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingMaxOcRatio  : 0x%X\n", CpuOverClockingPreMemConfig->RingMaxOcRatio));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingDownBin     : 0x%X\n", CpuOverClockingPreMemConfig->RingDownBin));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: Avx2RatioOffset : 0x%X\n", CpuOverClockingPreMemConfig->Avx2RatioOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: Avx3RatioOffset : 0x%X\n", CpuOverClockingPreMemConfig->Avx3RatioOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: BclkAdaptiveVoltage  : 0x%X\n", CpuOverClockingPreMemConfig->BclkAdaptiveVoltage));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: CorePllVoltageOffset : 0x%X\n", CpuOverClockingPreMemConfig->CorePllVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: GtPllVoltageOffset   : 0x%X\n", CpuOverClockingPreMemConfig->GtPllVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: RingPllVoltageOffset : 0x%X\n", CpuOverClockingPreMemConfig->RingPllVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: SaPllVoltageOffset   : 0x%X\n", CpuOverClockingPreMemConfig->SaPllVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: McPllVoltageOffset   : 0x%X\n", CpuOverClockingPreMemConfig->McPllVoltageOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: TjMaxOffset          : 0x%X\n", CpuOverClockingPreMemConfig->TjMaxOffset));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: TvbRatioClipping     : 0x%X\n", CpuOverClockingPreMemConfig->TvbRatioClipping));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: TvbVoltageOptimization : 0x%X\n", CpuOverClockingPreMemConfig->TvbVoltageOptimization));
#ifndef CPU_CFL
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: FivrFaults           : 0x%X\n", CpuOverClockingPreMemConfig->FivrFaults));
  DEBUG ((DEBUG_INFO, " CPU_OVERCLOCKING_PREMEM_CONFIG:: FivrEfficiency       : 0x%X\n", CpuOverClockingPreMemConfig->FivrEfficiency));
#endif

}


/**
  Print whole CPU Config blocks of SI_PREMEM_POLICY_PPI and serial out in PreMem.

  @param[in] SiPreMemPolicyPpi The SI Pre-Mem Policy PPI instance
**/
VOID
CpuPreMemPrintPolicy (
  IN  SI_PREMEM_POLICY_PPI       *SiPreMemPolicyPpi
  )
{
DEBUG_CODE_BEGIN();
  EFI_STATUS                    Status;
  CPU_SECURITY_PREMEM_CONFIG    *CpuSecurityPreMemConfig;
  CPU_TXT_PREMEM_CONFIG         *CpuTxtPreMemConfig;
  CPU_CONFIG_LIB_PREMEM_CONFIG    *CpuConfigLibPreMemConfig;
  CPU_OVERCLOCKING_PREMEM_CONFIG  *CpuOverclockingPreMemConfig;

  DEBUG ((DEBUG_INFO, "\n------------------------ CPU - SiPreMemPolicyPpi Print Begin in PreMem -----------------\n"));

  DEBUG ((DEBUG_INFO, " Revision= %x\n", SiPreMemPolicyPpi->TableHeader.Header.Revision));

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuSecurityPreMemConfigGuid, (VOID *) &CpuSecurityPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuTxtPreMemConfigGuid, (VOID *) &CpuTxtPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuConfigLibPreMemConfigGuid, (VOID *) &CpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuOverclockingPreMemConfigGuid, (VOID *) &CpuOverclockingPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  CpuSecurityPreMemConfigPrint(CpuSecurityPreMemConfig);
  CpuTxtPreMemConfigPrint(CpuTxtPreMemConfig);
  CpuConfigLibPreMemConfigPrint(CpuConfigLibPreMemConfig);
  CpuOverClockingPreMemConfigPrint(CpuOverclockingPreMemConfig);

  DEBUG ((DEBUG_INFO, "\n------------------------ CPU - SiPreMemPolicyPpi Print End -----------------\n\n"));
DEBUG_CODE_END();
}
