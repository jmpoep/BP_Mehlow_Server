/** @file
  Internal header file for PMC Private library

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
#ifndef _PMC_PRIVATE_LIB_INTERNAL_H_
#define _PMC_PRIVATE_LIB_INTERNAL_H_

/**
  Get PMC firmware patch version.

  @return PMC patch version number.
**/
UINT32
PmcGetFirmwarePatchVersion (
  VOID
  );

/**
  Check if PMC supports IPC CMD 0xA9.

  @retval TRUE  PMC supports CMD A9
  @retval FALSE PMC doesn't support CMD A9
**/
BOOLEAN
PmcIsIpcCmdA9Supported (
  VOID
  );

/**
  Check if MODPHY SUS PG is supported

  @retval  Status of MODPHY SUS PG support
**/
BOOLEAN
PmcIsModPhySusPgSupported (
  VOID
  );

/**
  This function is part of PMC init and configures which clock wake signals should
  set the SLOW_RING, SA, FAST_RING_CF and SLOW_RING_CF indication sent up to the CPU/PCH
**/
VOID
PmcInitClockWakeEnable (
  VOID
  );

/**
  This function configures PWRMBASE + 0x1E00 register
**/
VOID
PmcConfigureRegPwrm1E00 (
  VOID
  );

/**
  This function configures Misc PM_SYNC events settings
**/
VOID
PmcConfigurePmSyncEventsSettings (
  VOID
  );

#endif
