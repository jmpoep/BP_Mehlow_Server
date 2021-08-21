/** @file
  ME Firmware Version Info implementation.

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

#ifndef _MBP_DATA_H
#define _MBP_DATA_H

#include <MeBiosPayloadData.h>
#include <MeBiosPayloadHob.h>
#include <Ppi/HeciPpi.h>

/**
  Install MbpData protocol.

  @param[in]  HeciPpi                 The pointer with Heci Ppi
  @param[out] MbpHobPtr               The pointer with MbpHob installed.

**/
VOID
InstallMbp (
  IN HECI_PPI                         *HeciPpi,
  OUT ME_BIOS_PAYLOAD_HOB             **MbpHobPtr
  );

/**
  This routine returns ME-BIOS Payload information.

  @param[out] MbpPtr              ME-BIOS Payload information.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_DEVICE_ERROR        Failed to consume MBP
  @retval EFI_OUT_OF_RESOURCES    Out of resource
**/
EFI_STATUS
PrepareMeBiosPayload (
  OUT  ME_BIOS_PAYLOAD                *MbpPtr,
  OUT  ME_BIOS_PAYLOAD_SENSITIVE      *MbpSensitivePtr
  );


/**
  Dump MBP_DATA_PROTOCOL

  @param[in] MbpData              Pointer to MBP_DATA_PROTOCOL

**/
VOID
MbpDebugPrint (
  IN UINT8                     MbpRevision,
  IN UINT8                     MbpSensitiveRevision,
  IN ME_BIOS_PAYLOAD           *MbpPtr,
  IN ME_BIOS_PAYLOAD_SENSITIVE *MbpSensitivePtr
  );


#endif
