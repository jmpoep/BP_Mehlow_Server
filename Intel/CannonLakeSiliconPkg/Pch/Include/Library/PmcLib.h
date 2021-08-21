/** @file
  Header file for PmcLib.

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
#ifndef _PMC_LIB_H_
#define _PMC_LIB_H_

#pragma pack(1)

typedef enum {
  PmcTPch25_10us = 0,
  PmcTPch25_100us,
  PmcTPch25_1ms,
  PmcTPch25_10ms,
} PMC_TPCH25_TIMING;

typedef enum {
  PmcNotASleepState,
  PmcInS0State,
  PmcS1SleepState,
  PmcS2SleepState,
  PmcS3SleepState,
  PmcS4SleepState,
  PmcS5SleepState,
  PmcUndefinedState,
} PMC_SLEEP_STATE;

typedef struct {
  UINT32    Buf0;
  UINT32    Buf1;
  UINT32    Buf2;
  UINT32    Buf3;
} PMC_IPC_COMMAND_BUFFER;

//
// Structure to Check different attributes for CrashLog supported by PMC.
//
typedef union {
  struct {
    UINT32  Avail      : 1;        ///< CrashLog feature availability bit
    UINT32  Dis        : 1;        ///< CrasLog Disable bit
    UINT32  Rsvd       : 14;       ///< Reserved
    UINT32  BaseOffset : 16;       ///< Start offset of CrashLog in PMC SSRAM
  } Bits;
  UINT32  Uint32;
} PMC_IPC_DISCOVERY_BUF;

#pragma pack()

/**
  Get PCH ACPI base address.

  @retval Address                   Address of PWRM base address.
**/
UINT16
PmcGetAcpiBase (
  VOID
  );

/**
  Get PCH PWRM base address.

  @retval Address                   Address of PWRM base address.
**/
UINT32
PmcGetPwrmBase (
  VOID
  );

/**
  This function sets tPCH25 timing

  @param[in] TimingValue       tPCH25 timing value (10us, 100us, 1ms, 10ms)
**/
VOID
PmcSetTPch25Timing (
  IN PMC_TPCH25_TIMING    TimingValue
  );

/**
  This function checks if RTC Power Failure occurred by
  reading RTC_PWR_FLR bit

  @retval RTC Power Failure state: TRUE  - Battery is always present.
                                   FALSE - CMOS is cleared.
**/
BOOLEAN
PmcIsRtcBatteryGood (
  VOID
  );

/**
  This function checks if Power Failure occurred by
  reading PWR_FLR bit

  @retval Power Failure state
**/
BOOLEAN
PmcIsPowerFailureDetected (
  VOID
  );

/**
  This function checks if RTC Power Failure occurred by
  reading SUS_PWR_FLR bit

  @retval SUS Power Failure state
**/
BOOLEAN
PmcIsSusPowerFailureDetected (
  VOID
  );

/**
  This function clears Power Failure status (PWR_FLR)
**/
VOID
PmcClearPowerFailureStatus (
  VOID
  );

/**
  This function clears Global Reset status (GBL_RST_STS)
**/
VOID
PmcClearGlobalResetStatus (
  VOID
  );

/**
  This function clears Host Reset status (HOST_RST_STS)
**/
VOID
PmcClearHostResetStatus (
  VOID
  );

/**
  This function clears SUS Power Failure status (SUS_PWR_FLR)
**/
VOID
PmcClearSusPowerFailureStatus (
  VOID
  );

/**
  This function sets state to which platform will get after power is reapplied

  @param[in] PowerStateAfterG3          0: S0 state (boot)
                                        1: S5/S4 State
**/
VOID
PmcSetPlatformStateAfterPowerFailure (
  IN UINT8 PowerStateAfterG3
  );

/**
  This function enables Power Button SMI
**/
VOID
PmcEnablePowerButtonSmi (
  VOID
  );

/**
  This function disables Power Button SMI
**/
VOID
PmcDisablePowerButtonSmi (
  VOID
  );

/**
  This function reads PM Timer Count driven by 3.579545 MHz clock

  @retval PM Timer Count
**/
UINT32
PmcGetTimerCount (
  VOID
  );

/**
  Get Sleep Type that platform has waken from

  @retval SleepType                Sleep Type
**/
PMC_SLEEP_STATE
PmcGetSleepTypeAfterWake (
  VOID
  );

/**
  Clear PMC Wake Status
**/
VOID
PmcClearWakeStatus (
  VOID
  );

/**
  Configure sleep state

  @param[in] SleepState         S0/S1/S3/S4/S5, refer to PMC_SLEEP_STATE
**/
VOID
PmcSetSleepState (
  PMC_SLEEP_STATE  SleepState
  );

/**
  Check if platform boots after shutdown caused by power button override event

  @retval  TRUE   Power Button Override occurred in last system boot
  @retval  FALSE  Power Button Override didn't occur
**/
BOOLEAN
PmcIsPowerButtonOverrideDetected (
  VOID
  );

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
  );

/**
  This function sends PMC IPC CrashLog Disable Command

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log Disable command failed with an error
  @retval EFI_TIMEOUT             Crash Log Disable command did not complete
**/
EFI_STATUS
PmcCrashLogDisable (
  VOID
  );

/**
  This function sends PMC IPC to clear CrashLog from PMC SSRAM area

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log Clear command failed with an error
  @retval EFI_TIMEOUT             Crash Log Clear command did not complete
**/
EFI_STATUS
PmcCrashLogClear (
  VOID
  );

/**
  This function sends PMC IPC to populate CrashLog on all reboot. The SSRAM area will be cleared on G3 by PMC automatically

  @retval EFI_SUCCESS             Command was executed successfully
  @retval EFI_DEVICE_ERROR        Crash Log command failed with an error
  @retval EFI_TIMEOUT             Crash Log command did not complete
**/
EFI_STATUS
PmcCrashLogOnAllReset (
  VOID
  );

//
// UPServer
//
/**
  This function gets the 8254CGE bit

  @param[out]  ClockGateState    TRUE : 8254CGE is set, FALSE: 8254CGE is clear.

  @retval  EFI_SUCCESS           Get 8254CGE status successfully.
           EFI_UNSUPPORTED       Doesn't support 8254CGE get.
**/
EFI_STATUS
PmcGet8254ClockGateState (
  OUT BOOLEAN  *ClockGeteState
  );

/**
  This function sets the 8254CGE bit

  @param[in]  ClockGateEnable    Enable or disable clock gate

  @retval  EFI_SUCCESS           Set 8254CGE status successfully.
           EFI_UNSUPPORTED       Doesn't support 8254CGE set.
**/
EFI_STATUS
PmcSet8254ClockGateState (
  IN BOOLEAN   ClockGateEnable
  );
//
// UPServer
//

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
  );

/**
  This function will set the DISB - DRAM Initialization Scratchpad Bit.
**/
VOID
PmcSetDramInitScratchpad (
  VOID
  );

/**
  Check if this system boot is due to Overclocking flow reset during S3
  - BCLK update causes reset
  - PLL voltage changed causes reset

  @retval  TRUE   OC reset occured during S3 on previous boot
  @retval  FALSE  OC reset during S3 did not occur
**/
BOOLEAN
PmcIsBclkS3Boot (
  VOID
  );

/**
  Set OC BCLK S3 scratchpad bit

**/
VOID
PmcSetBclkS3 (
  VOID
  );

/**
  Clear OC BCLK S3 scratchpad bit

**/
VOID
PmcClearBclkS3 (
  VOID
  );

/**
  This function checks if Debug Mode is locked

  @retval Debug Mode Lock state
**/
BOOLEAN
PmcIsDebugModeLocked (
  VOID
  );

/**
  This function checks PMC Set Strap Message interface lock

  @retval State of PMC Set Strap Message Interface lock
**/
BOOLEAN
PmcIsSetStrapMsgInterfaceLocked (
  VOID
  );

/**
  This function checks if SMI Lock is set

  @retval SMI Lock state
**/
BOOLEAN
PmcIsSmiLockSet (
  VOID
  );

/**
  Check global SMI enable is set

  @retval TRUE  Global SMI enable is set
          FALSE Global SMI enable is not set
**/
BOOLEAN
PmcIsGblSmiEn (
  VOID
  );

#endif // _PMC_LIB_H_
