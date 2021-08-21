/** @file
  Definitions for HECI driver

@copyright
  INTEL CONFIDENTIAL
  Copyright 2006 - 2016 Intel Corporation.

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
#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/PchCycleDecodingLib.h>

#include "HeciHpet.h"
#include <HeciRegs.h>
#include "HeciCore.h"

GLOBAL_REMOVE_IF_UNREFERENCED UINT32    mHpetBaseAddr = 0;

/**
  Enable Hpet function.

  @retval UINT32                  Return the High Precision Event Timer base address

**/
volatile
UINT32 *
EnableHpet (
  VOID
  )
{
  UINT32            HpetTimer;
  UINT32            *HpetTimerPtr;

  HpetTimer = 0;

  ///
  /// Get the High Precision Event Timer base address and enable the memory range
  ///
  if (mHpetBaseAddr == 0) {
    PchHpetBaseGet (&HpetTimer);
    mHpetBaseAddr = HpetTimer;
  } else {
    HpetTimer = mHpetBaseAddr;
  }

  HpetTimerPtr = (UINT32*) (UINTN) HpetTimer;

  ///
  /// Start the timer so it is up and running
  ///
  HpetTimerPtr[HPET_GEN_CONFIG_LOW] = HPET_START;

  return HpetTimerPtr;
}

/**
  Used for calculating timeouts

  @param[in] Start                Snapshot of the HPET timer
  @param[in] End                  Calculated time when timeout period will be done
  @param[in] Time                 Timeout period in microseconds

**/
volatile
UINT32 *
StartTimer (
  OUT UINT32                      *Start,
  OUT UINT32                      *End,
  IN  UINT32                      Time
  )
{
  UINT32          Ticks;
  volatile UINT32 *HpetTimer;
  ///
  /// Make sure that HPET is enabled and running
  ///
  HpetTimer = EnableHpet ();

  ///
  /// Read current timer value into start time from HPET
  ///
  *Start = HpetTimer[HPET_MAIN_COUNTER_LOW];

  ///
  /// Convert microseconds into 41.6 ns timer ticks
  ///
  Ticks = Time * HPET_TICKS_PER_MICRO;

  ///
  /// Compute end time
  ///
  *End = *Start + Ticks;

  return HpetTimer;
}

/**
  Used to determine if a timeout has occured.

  @param[in] Start                Snapshot of the HPET timer when the timeout period started.
  @param[in] End                  Calculated time when timeout period will be done.
  @param[in] HpetTimer            The value of High Precision Event Timer

  @retval EFI_TIMEOUT             Timeout occured.
  @retval EFI_SUCCESS             Not yet timed out
**/
EFI_STATUS
Timeout (
  IN  UINT32                      Start,
  IN  UINT32                      End,
  IN  volatile UINT32             *HpetTimer
  )
{
  UINT32  Current;

  ///
  /// Read HPET and assign the value as the current time.
  ///
  Current = HpetTimer[HPET_MAIN_COUNTER_LOW];

  ///
  /// Test basic case (no overflow)
  ///
  if ((Start < End) && (End <= Current)) {
    return EFI_TIMEOUT;
  }
  ///
  /// Test basic start/end conditions with overflowed timer
  ///
  if ((Start < End) && (Current < Start)) {
    return EFI_TIMEOUT;
  }
  ///
  /// Test for overflowed start/end condition
  ///
  if ((Start > End) && ((Current < Start) && (Current > End))) {
    return EFI_TIMEOUT;
  }
  ///
  /// Catch corner case of broken arguments
  ///
  if (Start == End) {
    return EFI_TIMEOUT;
  }
  ///
  /// Else, we have not yet timed out
  ///
  return EFI_SUCCESS;
}

