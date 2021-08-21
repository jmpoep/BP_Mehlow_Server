/** @file
  Teton Glacier policy

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
#ifndef _TETON_GLACIER_CONFIG_H_
#define _TETON_GLACIER_CONFIG_H_

#define TETON_GLACIER_CONFIG_REVISION 2

extern EFI_GUID gTetonGlacierConfigGuid;

#pragma pack (push,1)

/**
  The TETON_GLACIER_CONFIG block describes the expected configuration for Teton Glacier device

  <b>Revision 1</b>:
  - Init version
  <b>Revision 2</b>:
  - Deprecated TetonGlacierSupport and Added TetonGlacierMode to provide support for dynamic configuration of clkreq when TetonGlacier is present.
**/

typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  /**
    Teton Glacier Support
    <b>0: Disable</b>, 1: Enable                  //@deprecated
  **/
  UINT32    TetonGlacierSupport         :  1;
  /**
    Specifies the cycle Router to which Teton Glacier is Connected
    <b>0: Cycle Router 0</b>, 1: Cycle Router 1, 2: Cycle Router 2
    Default is Cycle Router 0 for CNP-H system and Cycle Router 1 for CNP-LP system
  **/
  UINT32    TetonGlacierCR              :  2;
  /**
  Teton Glacier Mode
  <b>0: Disable</b>, 1: Enable Static Configuration 2: Enable Dynamic Configuration
  **/
  UINT32    TetonGlacierMode            :  2;
  UINT32    RsvdBits                    : 27;     ///< Reserved bits
} TETON_GLACIER_CONFIG;

#pragma pack (pop)

#endif // _TETON_GLACIER_CONFIG_H_
