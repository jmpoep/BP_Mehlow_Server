/** @file
  CPU Config Block.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation.

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
#ifndef _CPU_CONFIG_H_
#define _CPU_CONFIG_H_

#define CPU_CONFIG_REVISION 3

extern EFI_GUID gCpuConfigGuid;

#pragma pack (push,1)

/**
  CPU Configuration Structure.

  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Deprecate and move SkipMpInit to CpuConfigLibPreMemConfig.
  <b>Revision 3</b>:
  - Move DebugInterfaceEnable from CPU_TEST_CONFIG.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  /**
    Enable or Disable Advanced Encryption Standard (AES) feature.
    For some countries, this should be disabled for legal reasons.
    -    0: Disable
    - <b>1: Enable</b>
  **/
  UINT32 AesEnable                : 1;
  /**
    Enable or Disable Trusted Execution Technology (TXT) feature.
    -    0: Disable
    - <b>1: Enable</b>
  **/
  UINT32 TxtEnable                : 1;
  UINT32 SkipMpInit               : 1;            ///< @deprecated since revision 2. For Fsp only, Silicon Initialization will skip MP Initialization (including BSP) if enabled. For non-FSP, this should always be 0.
  UINT32 DebugInterfaceEnable     : 1;            ///< Enable or Disable processor debug features; <b>0: Disable</b>; 1: Enable.
  UINT32 RsvdBits                 : 28;           ///< Reserved for future use
  EFI_PHYSICAL_ADDRESS MicrocodePatchAddress;     ///< Pointer to microcode patch that is suitable for this processor.
} CPU_CONFIG;

#pragma pack (pop)

#endif // _CPU_CONFIG_H_
