/** @file
  PCH Reset Protocol

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _PCH_RESET_PROTOCOL_H_
#define _PCH_RESET_PROTOCOL_H_

//
// Member functions
//
/**
  Execute call back function for Pch Reset.

  @param[in] ResetType            Reset Types which includes GlobalReset.
  @param[in] ResetTypeGuid        Pointer to an EFI_GUID, which is the Reset Type Guid.

  @retval EFI_SUCCESS             The callback function has been done successfully
  @retval EFI_NOT_FOUND           Failed to find Pch Reset Callback protocol. Or, none of
                                  callback protocol is installed.
  @retval Others                  Do not do any reset from PCH
**/
typedef
EFI_STATUS
(EFIAPI *PCH_RESET_CALLBACK) (
  IN  EFI_RESET_TYPE    ResetType,
  IN  EFI_GUID          *ResetTypeGuid
  );

/**
  This protocol is used to execute PCH Reset from the host controller.
  If drivers need to run their callback function right before issuing the PCH Reset,
  they can install PCH Reset Callback Protocol before PCH Reset DXE driver to achieve that.
**/
typedef struct {
  PCH_RESET_CALLBACK  ResetCallback;
} PCH_RESET_CALLBACK_PROTOCOL;

#endif
