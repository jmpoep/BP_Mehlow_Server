/** @file
  PCH PMC Library.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

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

@par Specification Reference:
**/

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PmcLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <PchReservedResources.h>
#include <Register/PchRegsPmc.h>
#include <Register/PchRegsItss.h> //UPServer

/**
  Get PCH ACPI base address.

  @retval Address                   Address of PWRM base address.
**/
UINT16
PmcGetAcpiBase (
  VOID
  )
{
  return PcdGet16 (PcdAcpiBaseAddress);
}

/**
  Get PCH PWRM base address.

  @retval Address                   Address of PWRM base address.
**/
UINT32
PmcGetPwrmBase (
  VOID
  )
{
  return PCH_PWRM_BASE_ADDRESS;
}

/**
  This function enables Power Button SMI
**/
VOID
PmcEnablePowerButtonSmi (
  VOID
  )
{
  IoOr16 (PmcGetAcpiBase () + R_ACPI_IO_PM1_EN, B_ACPI_IO_PM1_EN_PWRBTN);
}

/**
  This function disables Power Button SMI
**/
VOID
PmcDisablePowerButtonSmi (
  VOID
  )
{
  IoAnd16 (PmcGetAcpiBase () + R_ACPI_IO_PM1_EN, (UINT16)~B_ACPI_IO_PM1_EN_PWRBTN);
}

/**
  This function reads PM Timer Count driven by 3.579545 MHz clock

  @retval PM Timer Count
**/
UINT32
PmcGetTimerCount (
  VOID
  )
{
  return IoRead32 (PmcGetAcpiBase () + R_ACPI_IO_PM1_TMR) & B_ACPI_IO_PM1_TMR_VAL;
}

/**
  Get Sleep Type that platform has waken from

  @retval SleepType                Sleep Type
**/
PMC_SLEEP_STATE
PmcGetSleepTypeAfterWake (
  VOID
  )
{
  UINT16  AcpiBase;
  UINT32  PmconA;

  AcpiBase = PmcGetAcpiBase ();
  PmconA   = MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A);

  DEBUG ((DEBUG_INFO, "PWRM_PMCON_A = 0x%x\n", PmconA));

  //
  // If Global Reset Status, Power Failure. Host Reset Status bits are set, return S5 State
  //
  if ((PmconA & (B_PMC_PWRM_GEN_PMCON_A_GBL_RST_STS | B_PMC_PWRM_GEN_PMCON_A_PWR_FLR | B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS)) != 0) {
    return PmcNotASleepState;
  }

  if (IoRead16 (AcpiBase + R_ACPI_IO_PM1_STS) & B_ACPI_IO_PM1_STS_WAK) {
    switch (IoRead16 (AcpiBase + R_ACPI_IO_PM1_CNT) & B_ACPI_IO_PM1_CNT_SLP_TYP) {
      case V_ACPI_IO_PM1_CNT_S0:
        return PmcInS0State;

      case V_ACPI_IO_PM1_CNT_S1:
        return PmcS1SleepState;

      case V_ACPI_IO_PM1_CNT_S3:
        return PmcS3SleepState;

      case V_ACPI_IO_PM1_CNT_S4:
        return PmcS4SleepState;

      case V_ACPI_IO_PM1_CNT_S5:
        return PmcS5SleepState;

      default:
        ASSERT (FALSE);
        return PmcUndefinedState;
    }
  } else {
    return PmcNotASleepState;
  }
}

/**
  Clear PMC Wake Status
**/
VOID
PmcClearWakeStatus (
  VOID
  )
{
  IoWrite16 (PmcGetAcpiBase () + R_ACPI_IO_PM1_STS, B_ACPI_IO_PM1_STS_WAK);
}

/**
  Configure sleep state

  @param[in] SleepState         S0/S1/S3/S4/S5, refer to PMC_SLEEP_STATE
**/
VOID
PmcSetSleepState (
  PMC_SLEEP_STATE  SleepState
  )
{
  UINT16  Data16;

  switch (SleepState) {
    case PmcInS0State:
      Data16 = V_ACPI_IO_PM1_CNT_S0;
      break;

    case PmcS1SleepState:
      Data16 = V_ACPI_IO_PM1_CNT_S1;
      break;

    case PmcS3SleepState:
      Data16 = V_ACPI_IO_PM1_CNT_S3;
      break;

    case PmcS4SleepState:
      Data16 = V_ACPI_IO_PM1_CNT_S4;
      break;

    case PmcS5SleepState:
      Data16 = V_ACPI_IO_PM1_CNT_S5;
      break;

    default:
      ASSERT (FALSE);
      return;

  }
  IoAndThenOr16 (PmcGetAcpiBase () + R_ACPI_IO_PM1_CNT, (UINT16) ~B_ACPI_IO_PM1_CNT_SLP_TYP, Data16);
}

/**
  Check if platform boots after shutdown caused by power button override event

  @retval  TRUE   Power Button Override occurred in last system boot
  @retval  FALSE  Power Button Override didn't occur
**/
BOOLEAN
PmcIsPowerButtonOverrideDetected (
  VOID
  )
{
  return ((IoRead16 (PmcGetAcpiBase () + R_ACPI_IO_PM1_STS) & B_ACPI_IO_PM1_STS_PRBTNOR) != 0);
}

/**
  This function sets tPCH25 timing

  @param[in] TimingValue       tPCH25 timing value (10us, 100us, 1ms, 10ms)
**/
VOID
PmcSetTPch25Timing (
  IN PMC_TPCH25_TIMING    TimingValue
  )
{
  ASSERT (TimingValue <= PmcTPch25_10ms);

  MmioAndThenOr32 (
    (UINTN) (PmcGetPwrmBase () + R_PMC_PWRM_CFG),
    (UINT32)~(B_PMC_PWRM_CFG_TIMING_TPCH25),
    TimingValue
    );
}

/**
  This function checks if RTC Power Failure occurred by
  reading RTC_PWR_FLR bit

  @retval RTC Power Failure state: TRUE  - Battery is always present.
                                   FALSE - CMOS is cleared.
**/
BOOLEAN
PmcIsRtcBatteryGood (
  VOID
  )
{
  return ((MmioRead8 (PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_B) & B_PMC_PWRM_GEN_PMCON_B_RTC_PWR_STS) == 0);
}

/**
  This function checks if Power Failure occurred by
  reading PWR_FLR bit

  @retval Power Failure state
**/
BOOLEAN
PmcIsPowerFailureDetected (
  VOID
  )
{
  return ((MmioRead16 (PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A) & B_PMC_PWRM_GEN_PMCON_A_PWR_FLR) != 0);
}

/**
  This function checks if RTC Power Failure occurred by
  reading SUS_PWR_FLR bit

  @retval SUS Power Failure state
**/
BOOLEAN
PmcIsSusPowerFailureDetected (
  VOID
  )
{
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_B) & B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR) != 0);
}

/**
  This function clears Power Failure status (PWR_FLR)
**/
VOID
PmcClearPowerFailureStatus (
  VOID
  )
{
  //
  // Write 1 to clear PWR_FLR
  // Avoid clearing other W1C bits
  //
  MmioAndThenOr8 (
    PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A + 1,
    (UINT8) ~(B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS >> 8),
    B_PMC_PWRM_GEN_PMCON_A_PWR_FLR >> 8
    );
}

/**
  This function clears Global Reset status (GBL_RST_STS)
**/
VOID
PmcClearGlobalResetStatus (
  VOID
  )
{
  //
  // Write 1 to clear GBL_RST_STS
  // Avoid clearing other W1C bits
  //
  MmioAndThenOr8 (
    PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A + 3,
    (UINT8) ~0,
    B_PMC_PWRM_GEN_PMCON_A_GBL_RST_STS >> 24
    );
}

/**
  This function clears Host Reset status (HOST_RST_STS)
**/
VOID
PmcClearHostResetStatus (
  VOID
  )
{
  //
  // Write 1 to clear HOST_RST_STS
  // Avoid clearing other W1C bits
  //
  MmioAndThenOr8 (
    PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A + 1,
    (UINT8) ~(B_PMC_PWRM_GEN_PMCON_A_PWR_FLR >> 8),
    B_PMC_PWRM_GEN_PMCON_A_HOST_RST_STS >> 8
    );
}

/**
  This function clears SUS Power Failure status (SUS_PWR_FLR)
**/
VOID
PmcClearSusPowerFailureStatus (
  VOID
  )
{
  //
  // BIOS clears this bit by writing a '1' to it.
  // Take care of other fields, so we don't clear them accidentally.
  //
  MmioAndThenOr8 (
    PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A + 2,
    (UINT8) ~(B_PMC_PWRM_GEN_PMCON_A_MS4V >> 16),
    B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR >> 16
    );
}

/**
  This function sets state to which platform will get after power is reapplied

  @param[in] PowerStateAfterG3          0: S0 state (boot)
                                        1: S5/S4 State
**/
VOID
PmcSetPlatformStateAfterPowerFailure (
  IN UINT8 PowerStateAfterG3
  )
{
  UINT32                PchPwrmBase;

  PchPwrmBase = PmcGetPwrmBase ();

  if (PowerStateAfterG3) {
    MmioOr8 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_A, B_PMC_PWRM_GEN_PMCON_A_AFTERG3_EN);
  } else {
    MmioAnd8 (PchPwrmBase + R_PMC_PWRM_GEN_PMCON_A, (UINT8)~B_PMC_PWRM_GEN_PMCON_A_AFTERG3_EN);
  }
}

/**
  This function will set the DISB - DRAM Initialization Scratchpad Bit.
**/
VOID
PmcSetDramInitScratchpad (
  VOID
  )
{
  //
  // Set B_CNL_PCH_PWRM_GEN_PMCON_A_DISB.
  // NOTE: Byte access and not clear BIT18 and BIT16 (W1C bits)
  //
  MmioAndThenOr8 (
    PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_A + 2,
    (UINT8) ~((B_PMC_PWRM_GEN_PMCON_A_MS4V | B_PMC_PWRM_GEN_PMCON_A_SUS_PWR_FLR) >> 16),
    B_PMC_PWRM_GEN_PMCON_A_DISB >> 16
    );
}

/**
  This function returns PMC, PUNIT and CPU Crashlog Size allocated on PMC SSRAM

  @param[out] PmcLogSize        Pointer to PMC CrashLog Size
  @param[out] PunitLogSize      Pointer to PUNIT CrashLog Size
  @param[out] CpuLogSize        Pointer to CPU CrashLog Size
**/
VOID
PmcSsramGetCrashLogSize (
  OUT UINT32     *PmcLogSize,
  OUT UINT32     *PunitLogSize,
  OUT UINT32     *CpuLogSize
  )
{
  PmcSsramSocCrashLogSize (PmcLogSize, PunitLogSize, CpuLogSize);
}

/**
  This function sends PMC IPC CrashLog Discovery Command

  @param[out] DiscoveryBuffer     Pointer to CrashLog Discovery Data

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_INVALID_PARAMETER   NULL argument
  @retval EFI_DEVICE_ERROR        Crash Log Discovery command failed with an error
  @retval EFI_TIMEOUT             Crash Log Discovery command did not complete
**/
EFI_STATUS
PmcCrashLogDiscovery (
  OUT PMC_IPC_DISCOVERY_BUF     *DiscoveryBuffer
  )
{
  EFI_STATUS                Status;
  PMC_IPC_COMMAND_BUFFER    Wbuf;
  PMC_IPC_COMMAND_BUFFER    Rbuf;

  if (DiscoveryBuffer == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));
  ZeroMem (&Rbuf, sizeof (PMC_IPC_COMMAND_BUFFER));
  Status = PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_CRASHLOG, V_PMC_PWRM_IPC_CMD_CMD_ID_CRASHLOG_DISCOVERY, 4, &Wbuf, &Rbuf);
  if (!EFI_ERROR (Status)) {
    DiscoveryBuffer->Uint32 = Rbuf.Buf0;
  }
  return Status;
}

/**
  This function sends PMC IPC CrashLog Disable Command

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log Disable command failed with an error
  @retval EFI_TIMEOUT             Crash Log Disable command did not complete
**/
EFI_STATUS
PmcCrashLogDisable (
  VOID
  )
{
  PMC_IPC_COMMAND_BUFFER    Wbuf;

  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));
  return PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_CRASHLOG, V_PMC_PWRM_IPC_CMD_CMD_ID_CRASHLOG_DISABLE, 4, &Wbuf, NULL);
}

/**
  This function sends PMC IPC to clear CrashLog from PMC SSRAM area

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log Clear command failed with an error
  @retval EFI_TIMEOUT             Crash Log Clear command did not complete
**/
EFI_STATUS
PmcCrashLogClear (
  VOID
  )
{
  PMC_IPC_COMMAND_BUFFER    Wbuf;

  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));
  return PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_CRASHLOG, V_PMC_PWRM_IPC_CMD_CMD_ID_CRASHLOG_ERASE, 4, &Wbuf, NULL);
}

/**
  This function sends PMC IPC to populate CrashLog on all reboot. The SSRAM area will be cleared on G3 by PMC automatically

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log command failed with an error
  @retval EFI_TIMEOUT             Crash Log command did not complete
**/
EFI_STATUS
PmcCrashLogOnAllReset (
  VOID
  )
{
  PMC_IPC_COMMAND_BUFFER    Wbuf;

  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));
  return PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_CRASHLOG, V_PMC_PWRM_IPC_CMD_CMD_ID_CRASHLOG_ON_RESET, 4, &Wbuf, NULL);;
}

/**
  Check if this system boot is due to Overclocking flow reset during S3
  - BCLK update causes reset
  - PLL voltage changed causes reset

  @retval  TRUE   PLL or BCLK reset occured during S3 on previous boot
  @retval  FALSE  PLL or BCLK reset during S3 did not occur
**/
BOOLEAN
PmcIsBclkS3Boot (
  VOID
  )
{
  UINT8 PmcScratchPad;

  PmcScratchPad = MmioRead8 ((UINTN) (PmcGetPwrmBase () + R_PMC_PWRM_BIOS_SCRATCHPAD_1));

  if (PmcScratchPad & B_PMC_PWRM_BIOS_SCRATCHPAD_1_BCLK_S3) {
    DEBUG ((DEBUG_INFO, "OC PLL or BCLK reset occured during S3 resume on the previous boot.\n"));
    return TRUE;
  }

  DEBUG ((DEBUG_INFO, "OC PLL or BCLK reset during S3 did not occur.\n"));
  return FALSE;
}

/**
  Set OC BCLK S3 scratchpad bit

**/
VOID
PmcSetBclkS3 (
  VOID
  )
{
  ///
  /// Set OC S3 of scratchpad register to indicate PLL or BCLK OC reset during S3
  ///
  MmioOr8 (PmcGetPwrmBase () + R_PMC_PWRM_BIOS_SCRATCHPAD_1, B_PMC_PWRM_BIOS_SCRATCHPAD_1_BCLK_S3);
}


/**
  Clear OC BCLK S3 scratchpad bit

**/
VOID
PmcClearBclkS3 (
  VOID
  )
{
  ///
  /// Clear OC S3 of scratchpad register so we do not detect another PLL or BCLK reset.
  ///
  MmioAnd8 ((UINTN) (PmcGetPwrmBase () + R_PMC_PWRM_BIOS_SCRATCHPAD_1), (UINT8) (~B_PMC_PWRM_BIOS_SCRATCHPAD_1_BCLK_S3));
}

/**
  This function checks if Debug Mode is locked

  @retval Debug Mode Lock state
**/
BOOLEAN
PmcIsDebugModeLocked (
  VOID
  )
{
  //
  // Get lock info from PWRMBASE + PM_CFG
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_CFG) & B_PMC_PWRM_CFG_DBG_MODE_LOCK) != 0);
}

/**
  This function checks PMC Set Strap Message interface lock

  @retval State of PMC Set Strap Message Interface lock
**/
BOOLEAN
PmcIsSetStrapMsgInterfaceLocked (
  VOID
  )
{
  return ((MmioRead32 ((UINTN) (PmcGetPwrmBase () + R_PMC_PWRM_SSML)) & B_PMC_PWRM_SSML_SSL) != 0);
}

/**
  This function checks if SMI Lock is set

  @retval SMI Lock state
**/
BOOLEAN
PmcIsSmiLockSet (
  VOID
  )
{
  return ((MmioRead8 ((UINTN) (PmcGetPwrmBase () + R_PMC_PWRM_GEN_PMCON_B)) & B_PMC_PWRM_GEN_PMCON_B_SMI_LOCK) != 0);
}

/**
  Check global SMI enable is set

  @retval TRUE  Global SMI enable is set
          FALSE Global SMI enable is not set
**/
BOOLEAN
PmcIsGblSmiEn (
  VOID
  )
{
  return !!(IoRead32 (PmcGetAcpiBase () + R_ACPI_IO_SMI_EN) & B_ACPI_IO_SMI_EN_GBL_SMI);
}

