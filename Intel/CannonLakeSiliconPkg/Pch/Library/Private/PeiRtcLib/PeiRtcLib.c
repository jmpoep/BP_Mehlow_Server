/** @file
  Initializes PCH RTC Device in PEI

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


#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PchPcrLib.h>
#include <Ppi/SiPolicy.h>
#include <Register/PchRegsLpc.h>
#include <Register/PchRegsPmc.h>
#include <Register/PchRegsPcr.h>

/**
  Configure RTC when power failure case.
**/
VOID
RtcPowerFailureConfiguration (
  VOID
  )
{
  EFI_STATUS              Status;
  UINT16                  TcoBase;
  ///
  /// PCH BIOS Spec Power Failure Considerations
  /// RTC_PWR_STS bit, GEN_PMCON_B (PMC PCI offset A4h[2])
  /// When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
  /// Software should clear this bit. For example, changing the RTC battery sets this bit.
  /// System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
  /// The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set
  /// before memory initialization. This will ensure that the RTC state machine has been
  /// initialized.
  /// 1. If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a
  ///    battery failure, steps 2 through 5 should be executed.
  /// 2. Set RTC Register 0Ah[6:4] to 110b or 111b
  /// 3. Set RTC Register 0Bh[7].
  /// 4. Set RTC Register 0Ah[6:4] to 010b
  /// 5. Clear RTC Register 0Bh[7].
  ///

  ///
  /// 2. Set RTC Register 0Ah[6:4] to 110b or 111b
  ///
  IoWrite8 (R_RTC_IO_INDEX_ALT, (UINT8) R_RTC_IO_REGA);
  IoAndThenOr8 (
    R_RTC_IO_TARGET_ALT,
    (UINT8) ~(BIT6 | BIT5 | BIT4),
    (UINT8) (BIT6 | BIT5)
    );
  ///
  /// 3. Set RTC Register 0Bh[7].
  ///
  IoWrite8 (R_RTC_IO_INDEX_ALT, (UINT8) R_RTC_IO_REGB);
  IoOr8 (R_RTC_IO_TARGET_ALT, (UINT8) B_RTC_IO_REGB_SET);
  ///
  /// 4. Set RTC Register 0Ah[6:4] to 010b
  ///
  IoWrite8 (R_RTC_IO_INDEX_ALT, (UINT8) R_RTC_IO_REGA);
  IoAndThenOr8 (
    R_RTC_IO_TARGET_ALT,
    (UINT8) ~(BIT6 | BIT5 | BIT4),
    (UINT8) (BIT5)
    );
  ///
  /// 5. Clear RTC Register 0Bh[7].
  ///
  IoWrite8 (R_RTC_IO_INDEX_ALT, (UINT8) R_RTC_IO_REGB);
  IoAnd8 (R_RTC_IO_TARGET_ALT, (UINT8) ~B_RTC_IO_REGB_SET);

  ///
  /// The INTRUDER# input is overloaded as a pin-strap in ICP,
  /// there might be false INTRUDER event status set on RTC reset.
  /// BIOS must clear INTRUDER status if RTC_PWR_STS is set.
  ///
  Status = PchTcoBaseGet (&TcoBase);
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    IoWrite16 (TcoBase + R_TCO_IO_TCO2_STS, B_TCO_IO_TCO2_STS_INTRD_DET);
  }
}

/**
  Wait for RTC CP ready.
**/
STATIC
VOID
RtcWaitForCp (
  VOID
  )
{
  UINT32 Timeout;

  Timeout = 10000;
  while (Timeout > 0) {
    if (PchPcrRead32 (PID_RTC_HOST, R_RTC_PCR_PG1_CP_LO) == 0x0121001F) {
      break;
    }
    MicroSecondDelay (10);
    Timeout -= 10;
  }
}

/**
  The function performs RTC specific programming.

  @param[in] SiPolicyPpi        The SI Policy PPI instance

**/
VOID
PchRtcConfigure (
  IN  SI_POLICY_PPI           *SiPolicyPpi
  )
{
  ///
  /// Set PCR[RTC] + 3400h[6] = 1b
  ///
  PchPcrAndThenOr32 (
    PID_RTC_HOST, R_RTC_PCR_CONF,
    ~0u,
    B_RTC_PCR_CONF_HPM_HW_DIS
    );

  ///
  /// Set PCR[RTC] + 3418h[2:0] = 111b
  ///
  PchPcrAndThenOr32 (
    PID_RTC_HOST, R_RTC_PCR_RTCDCG,
    ~0u,
    R_RTC_PCR_RTCDCG_RTCPGCBDCGEN | R_RTC_PCR_RTCDCG_RTCPCICLKDCGEN | R_RTC_PCR_RTCDCG_RTCROSIDEDCGEN
    );

  ///
  /// Set PCR[RTC] + 3F00h[8] = 1b
  ///
  PchPcrAndThenOr32 (
    PID_RTC_HOST, R_RTC_PCR_3F00,
    ~0u,
    BIT8
    );

  //
  // Clear RTC SMI enable and status.
  //
  PchPcrWrite32 (PID_RTC_HOST, R_RTC_PCR_UIPSMI, 0x00030000);

  ///
  /// Set Daylight Saving Override bit in RTC by default
  ///
  PchPcrAndThenOr32 (PID_RTC_HOST, R_RTC_PCR_BUC, (UINT32)~0, B_RTC_PCR_BUC_DSO);

  //
  // Wait for RTC CP ready.
  //
  RtcWaitForCp ();

  ///
  /// Configure access to RTC PCRs
  ///
  PchPcrAndThenOr32 (
    PID_RTC_HOST, R_RTC_PCR_PG1_AC_LO,
    ~0u,
    0x1F
  );
}
