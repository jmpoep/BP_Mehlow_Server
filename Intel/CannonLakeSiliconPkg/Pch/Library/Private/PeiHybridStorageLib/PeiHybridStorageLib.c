/** @file
  This file contains functions for hybrid storage devices which are devices with both Optane memory
  and QLC SSD.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2018 Intel Corporation.

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
#include <Private/Library/PeiHybridStorageLib.h>
#include <Base.h>
#include <Register/PchRegs.h>

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT16 mHybridStorageDidList[] = {
  0x0975,
  0x09AC,
  0x09AD,
  0x09AE,
  0x09AF,
  0x09B0,
  0x09B1,
  0x09B2,
  0x09B3,
  0x09B4,
  0x09B5,
  0x09B6,
  0x09B7,
  0x09B8,
  0x09B9,
  0x09BA,
  0x09BB
};

/**
  Determine if device is a Hybrid Device or not

  @param[in] VendorId               Vendor Id of the device
  @param[in] DeviceId               Device Id of the device

  @retval TRUE                      Device is a Hybrid Storage Device
  @retval FALSE                     Device is not a Hybrid Storage Device
**/
BOOLEAN
IsHybridStorageDevice (
  IN    UINT16      VendorId,
  IN    UINT16      DeviceId
  )
{
  UINT8  Index;

  if (VendorId == V_PCH_INTEL_VENDOR_ID) {
    for (Index = 0; Index < ARRAY_SIZE (mHybridStorageDidList); Index ++) {
      if (DeviceId == mHybridStorageDidList[Index]) {
        return TRUE;
      }
    }
  }
  return FALSE;
}
