/** @file
  CPU Trace Hub policy

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

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
#ifndef _CPU_TRACEHUB_CONFIG_H_
#define _CPU_TRACEHUB_CONFIG_H_

#define CPU_TRACEHUB_CONFIG_REVISION 1
extern EFI_GUID gCpuTraceHubConfigGuid;

#pragma pack (push,1)

/**
  This data structure includes TraceHub configuration variables for SA.
  <b>Revision 1</b>:
  - Initial version.
**/

typedef struct {
  CONFIG_BLOCK_HEADER   Header;     ///< Offset 0-27 Config Block Header
/**
  Offset 28:0
  If this policy is disabled, all other policies will be ignored.
  <b> 0 = Disable, </b>
  1 = Target Debugger enabled,
**/
  UINT32  EnableMode                :  2;
  /**
  CPU Trace hub memory buffer region size policy
  The avaliable memory size options are: <b>0:0MB (none)</b>, 1:1MB, 2:8MB, 3:64MB, 4:128MB, 5:256MB, 6:512MB.
  Note : Limitation of total buffer size (PCH + PCU) is 512MB.
  **/
  UINT32  CpuTraceHubMemReg0Size    :  8;
  UINT32  CpuTraceHubMemReg1Size    :  8;
  UINT32  RsvdBits0                 : 14;  ///< Reserved bits
} CPU_TRACE_HUB_CONFIG;

#pragma pack (pop)

#endif // _CPU_TRACEHUB_CONFIG_H_
