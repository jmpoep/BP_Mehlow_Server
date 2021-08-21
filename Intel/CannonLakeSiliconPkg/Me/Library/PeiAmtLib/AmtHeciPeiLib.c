/** @file
  This is a library for Amt Heci Message functionality.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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

#include <PiPei.h>

#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/DebugLib.h>

#include <Ppi/Wdt.h>
#include <Ppi/HeciPpi.h>
#include <CoreBiosMsg.h>
#include <AsfMsgs.h>
#include <Library/PeiAmtLib.h>

/**
  Start ASF BIOS Watch Dog Timer HECI message
  The WDT will be started only if AMT WatchDog policy is enabled and corresponding timer value is not zero.

  @param[in] HeciPpi              The pointer to HECI PPI
  @param[in] WaitTimerBios        The value of waiting limit in seconds

  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this function
  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_INVALID_PARAMETER   Timeout value is invalid as 0
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
**/
EFI_STATUS
PeiHeciAsfStartWatchDog (
  IN  HECI_PPI                    *HeciPpi,
  IN  UINT16                      WaitTimerBios
  )
{
  EFI_STATUS                      Status;
  ASF_START_WDT                   AsfStartWdt;
  UINT32                          MeStatus;
  UINT32                          Length;
  UINT32                          MeMode;

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    DEBUG ((DEBUG_ERROR, "MeMode is %x, unable to start ASF BIOS WDT \n", MeMode));
    return EFI_UNSUPPORTED;
  }

  Status = HeciPpi->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || (ME_STATUS_ME_STATE_ONLY (MeStatus) != ME_READY)) {
    DEBUG ((DEBUG_ERROR, "MeStatus is %x, unable to start ASF BIOS WDT \n", MeStatus));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "PeiHeciAsfStartWatchDog () - Starting ASF BIOS WDT with timeout %d seconds \n", WaitTimerBios));

  AsfStartWdt.AsfHeader.Fields.Command         = ASF_MESSAGE_COMMAND_MANAGEMENT_CONTROL;
  AsfStartWdt.AsfHeader.Fields.ByteCount       = ASF_MESSAGE_BYTE_COUNT_MAP (ASF_START_WDT);
  AsfStartWdt.AsfHeader.Fields.SubCommand      = ASF_MESSAGE_SUBCOMMAND_START_WATCH_DOG_TIMER;
  AsfStartWdt.AsfHeader.Fields.VersionNumber   = ASF_VERSION;
  AsfStartWdt.TimeoutLow                       = (UINT8) WaitTimerBios;
  AsfStartWdt.TimeoutHigh                      = (UINT8) (WaitTimerBios >> 8);
  AsfStartWdt.EventSensorType                  = ASF_EVENT_SENSOR_TYPE_WATCHDOG2;
  AsfStartWdt.EventType                        = ASF_EVENT_TYPE_SENSOR_SPECIFIC;
  AsfStartWdt.EventOffset                      = ASF_EVENT_OFFSET_TIMER_EXPIRED;
  AsfStartWdt.EventSourceType                  = ASF_EVENT_SOURCE_TYPE_PLATFORM_FIRMWARE;
  AsfStartWdt.EventSeverity                    = ASF_EVENT_SEVERITY_CODE_CRITICAL;
  AsfStartWdt.SensorDevice                     = ASF_SENSOR_DEVICE;
  AsfStartWdt.SensorNumber                     = ASF_SENSOR_NUMBER;
  AsfStartWdt.Entity                           = ASF_ENTITY_UNSPECIFIED;
  AsfStartWdt.EntityInstance                   = ASF_ENTITY_INSTANCE_UNSPECIFIED;
  AsfStartWdt.EventData[0]                     = ASF_WD_EVENT_DATA1;
  AsfStartWdt.EventData[1]                     = ASF_WD_EVENT_DATA2_BIOS_TIMEOUT;

  Length = sizeof (ASF_START_WDT);;

  Status = HeciPpi->SendMsg (
                      HECI1_DEVICE,
                      (UINT32 *) &AsfStartWdt,
                      Length,
                      BIOS_ASF_HOST_ADDR,
                      HECI_ASF_MESSAGE_ADDR
                      );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to start ASF BIOS WDT \n"));
  }
  return Status;
}

/**
  Stop ASF BIOS Watch Dog Timer HECI message. The function call in PEI phase
  will not check WatchDog policy, always disable WDT once it is invoked to disable
  any initialized WDT.

  @param[in] HeciPpi              The pointer to HECI PPI
**/
VOID
PeiHeciAsfStopWatchDog (
  IN  HECI_PPI                    *HeciPpi
  )
{
  EFI_STATUS                      Status;
  ASF_STOP_WDT                    AsfStopWdt;
  UINT32                          Length;
  UINT32                          MeStatus;
  UINT32                          MeMode;

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    DEBUG ((DEBUG_ERROR, "MeMode is %x, unable to stop ASF BIOS WDT \n", MeMode));
    return;
  }

  Status = HeciPpi->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status) || (ME_STATUS_ME_STATE_ONLY (MeStatus) != ME_READY)) {
    DEBUG ((DEBUG_ERROR, "MeStatus is %x, unable to stop ASF BIOS WDT \n", MeStatus));
    return;
  }

  DEBUG ((DEBUG_INFO, "PeiHeciAsfStopWatchDog () - Stopping ASF BIOS WDT...\n"));

  AsfStopWdt.AsfHeader.Fields.Command        = ASF_MESSAGE_COMMAND_MANAGEMENT_CONTROL;
  AsfStopWdt.AsfHeader.Fields.ByteCount      = ASF_MESSAGE_BYTE_COUNT_MAP (ASF_STOP_WDT);
  AsfStopWdt.AsfHeader.Fields.SubCommand     = ASF_MESSAGE_SUBCOMMAND_STOP_WATCH_DOG_TIMER;
  AsfStopWdt.AsfHeader.Fields.VersionNumber  = ASF_VERSION;

  Length = sizeof (ASF_STOP_WDT);

  Status = HeciPpi->SendMsg (
                      HECI1_DEVICE,
                      (UINT32 *) &AsfStopWdt,
                      Length,
                      BIOS_ASF_HOST_ADDR,
                      HECI_ASF_MESSAGE_ADDR
                      );

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Unable to stop ASF BIOS WDT \n"));
  }
}
