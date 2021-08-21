/** @file
  ME Firmware Version Info implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2017 Intel Corporation.

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

#ifndef _ME_INIT_H
#define _ME_INIT_H

/**
  This is a dummy event to be hooked to provide ME or Platform
  code before EOP Heci message is sent

  @param[in] Event                The event that triggered this notification function
  @param[in] Context              Pointer to the notification functions context
**/
VOID
EFIAPI
MeEmptyEvent (
  IN EFI_EVENT Event,
  IN VOID      *Context
  );

/**
  Signal a event to save Me relevant registers and this event must be run before ExitPmAuth.

  @param[in] Event                The event that triggered this notification function
  @param[in] ParentImageHandle    Pointer to the notification functions context

**/
VOID
EFIAPI
MeScriptSaveEvent (
  IN EFI_EVENT Event,
  IN VOID      *ParentImageHandle
  );

/**
  Signal a event for Me ready to boot.
  Performing below tasks in order:
  1. Signal AmtReadyToBoot events for AMT platform hooks.
  2. Signal MePlatformReadyToBoot event for platform hooks.
  3. Report FWSTS and save to SMBIOS
  4. Me device de-configure for supporting late status reflect
     situation if any FW error has reported after PEI phase
  5. Send EOP message
  6. Lock config for manufacturing mode
  7. Remove PSF access

  @param[in] Event                The event that triggered this notification function
  @param[in] Context              Pointer to the notification functions context

**/
VOID
EFIAPI
MeReadyToBootEvent (
  IN EFI_EVENT Event,
  IN VOID      *Context
  );

/**
  Send ME the BIOS end of Post message.

  @param[out] RequestedActions    Action request returned by EOP ACK
                                    0x00 (HECI_EOP_STATUS_SUCCESS) - Continue to boot
                                    0x01 (HECI_EOP_PERFORM_GLOBAL_RESET) - Global reset

  @retval EFI_SUCCESS             EOP is sent successfully
  @retval EFI_DEVICE_ERROR        No HECI1 device
  @retval EFI_TIMEOUT             Timeout when send EOP message
**/
EFI_STATUS
MeEndOfPostEvent (
  OUT UINT32                       *RequestedActions
  );

/**
  1. Cf9Gr Lock Config
      - PCH BIOS Spec Rev 0.9 Section 18.4  Additional Power Management Programming
        Step 2
        Set "Power Management Initialization Register (PMIR) Field 1", D31:F0:ACh[31] = 1b
        for production machine according to "RS - PCH Intel Management Engine
        (Intel(r) ME) BIOS Writer's Guide".
  2. Remove PSF access prior to boot

**/
VOID
LockConfig (
  VOID
  );

#endif
