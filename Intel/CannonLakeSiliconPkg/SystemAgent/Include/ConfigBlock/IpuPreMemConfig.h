/** @file
  IPU policy definitions (PreMem)

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 Intel Corporation.

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
#ifndef _IPU_CONFIG_PREMEM_H_
#define _IPU_CONFIG_PREMEM_H_

#pragma pack(push, 1)

#define IPU_PREMEM_CONFIG_REVISION 1

#define SA_IMR_IPU_CAMERA   0
#define SA_IMR_IPU_GEN      1

/**
 IPU PreMem configuration\n
  <b>Revision 1</b>:
  - Initial version.
**/
typedef struct {
  CONFIG_BLOCK_HEADER  Header;           ///< Offset  0-27 Config Block Header
  /**
  Offset 28:0 :
  <b>(Test)</b> It enables the SA IPU Device if supported and not fused off.
  If FALSE, all other policies in this config block will be ignored.
  <b>1=TRUE</b>;
  0=FALSE.
  **/
  UINT32    SaIpuEnable:1;
  /**
  Offset 28:1 :
  <b>(Test)</b> It configure the IPU IMR to IPU Camera or IPU Gen when IPU is enabled.
  If FALSE, all other policies in this config block will be ignored.
  <b>0=IPU Camera</b>;
  1=IPU Gen
  **/
  UINT32    SaIpuImrConfiguration:1;
  UINT32    RsvdBits0:30;                     /// Offset 28:2 :Reserved for future use.
} IPU_PREMEM_CONFIG;
#pragma pack(pop)

#endif // _IPU_PREMEM_CONFIG_H_
