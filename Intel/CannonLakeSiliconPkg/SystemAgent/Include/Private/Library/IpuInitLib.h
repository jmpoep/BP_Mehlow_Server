/** @file
  IPU device header file

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
#ifndef _IPU_INIT_H_
#define _IPU_INIT_H_

#include <SaPolicyCommon.h>

#ifndef CPU_CFL
//
// GPIO Community Port ID for IPU FW
//
typedef enum {
  GpioCommunity0 = 0xE7,
  GpioCommunity1 = 0xE8,
  GpioCommunity3 = 0xE9,
  GpioCommunity4 = 0xEA
} GpioCommunityPortId;

typedef union {
  struct {
    UINT32    PadCfgAddr:16;             ///< Offset  0-15 The GPIO_PADCFG DW0 address of the GPIO pin
    UINT32    PortId:8;                  ///< Offset 16-23 The GPIO Community Port ID of the GPIO pin
    UINT32    Rsvd:7;                    ///< Offset 24-30 Reserved
    UINT32    Enabled:1;                 ///< Offset    31 The Enable bit of PADCFG of the GPIO pin
  } Data;
  UINT32    IpuFwGpioPadCfg;
} IPU_FW_GPIO_PADCFG;
#endif

/**
  IpuInit: Initialize the IPU device

  @param[in] IPU_PREMEM_CONFIG   IpuPreMemPolicy

**/
VOID
IpuInit (
  IN       IPU_PREMEM_CONFIG         *IpuPreMemPolicy
  );

/**
  IsIpuEnabled: Check the IPU is enabled or not

  @retval FALSE = IPU Disabled by policy or fuse, TRUE = IPU Enabled.
**/
BOOLEAN
IsIpuEnabled (
  VOID
  );

/**
  GetIpuImrConfiguration: Get the IPU IMR Configuration

  @retval IPU IMR Configuration, 0 = IPU Camera, 1 = IPU Gen
**/
UINT8
GetIpuImrConfiguration (
  VOID
  );
#endif
