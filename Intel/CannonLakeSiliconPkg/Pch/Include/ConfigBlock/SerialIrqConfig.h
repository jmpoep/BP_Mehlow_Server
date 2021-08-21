/** @file
  Serial IRQ policy

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#ifndef _SERIAL_IRQ_CONFIG_H_
#define _SERIAL_IRQ_CONFIG_H_

#define SERIAL_IRQ_CONFIG_REVISION 1
extern EFI_GUID gSerialIrqConfigGuid;

#pragma pack (push,1)

typedef enum {
  PchQuietMode,
  PchContinuousMode
} PCH_SIRQ_MODE;
///
/// Refer to PCH EDS for the details of Start Frame Pulse Width in Continuous and Quiet mode
///
typedef enum {
  PchSfpw4Clk,
  PchSfpw6Clk,
  PchSfpw8Clk
} PCH_START_FRAME_PULSE;

///
/// The PCH_LPC_SIRQ_CONFIG block describes the expected configuration of the PCH for Serial IRQ.
///
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Config Block Header
  UINT32                SirqEnable      :  1;     ///< Determines if enable Serial IRQ. 0: Disable; <b>1: Enable</b>.
  UINT32                SirqMode        :  2;     ///< Serial IRQ Mode Select. Refer to PCH_SIRQ_MODE for each value. <b>0: quiet mode</b> 1: continuous mode.
  UINT32                StartFramePulse :  3;     ///< Start Frame Pulse Width. Refer to PCH_START_FRAME_PULSE for each value. Default is <b>PchSfpw4Clk</b>.
  UINT32                RsvdBits0       : 26;     ///< Reserved bits
} PCH_LPC_SIRQ_CONFIG;

#pragma pack (pop)

#endif // _SERIAL_IRQ_CONFIG_H_
