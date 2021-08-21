/** @file
  ISH policy

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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
#ifndef _ISH_CONFIG_H_
#define _ISH_CONFIG_H_

#define ISH_PREMEM_CONFIG_REVISION 1
#define ISH_CONFIG_REVISION 1
extern EFI_GUID gIshPreMemConfigGuid;
extern EFI_GUID gIshConfigGuid;

#pragma pack (push,1)

///
/// The PCH_ISH_CONFIG block describes Integrated Sensor Hub device.
///
typedef struct {
  CONFIG_BLOCK_HEADER   Header;     ///< Config Block Header
  UINT32    SpiGpioAssign   :  1;   ///< ISH SPI GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    Uart0GpioAssign :  1;   ///< ISH UART0 GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    Uart1GpioAssign :  1;   ///< ISH UART1 GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    I2c0GpioAssign  :  1;   ///< ISH I2C0 GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    I2c1GpioAssign  :  1;   ///< ISH I2C1 GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    I2c2GpioAssign  :  1;   ///< ISH I2C2 GPIO pins assigned: <b>0: False</b> 1: True
  UINT32    Gp0GpioAssign   :  1;   ///< ISH GP_0 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp1GpioAssign   :  1;   ///< ISH GP_1 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp2GpioAssign   :  1;   ///< ISH GP_2 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp3GpioAssign   :  1;   ///< ISH GP_3 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp4GpioAssign   :  1;   ///< ISH GP_4 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp5GpioAssign   :  1;   ///< ISH GP_5 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp6GpioAssign   :  1;   ///< ISH GP_6 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    Gp7GpioAssign   :  1;   ///< ISH GP_7 GPIO pin assigned: <b>0: False</b> 1: True
  UINT32    PdtUnlock       :  1;   ///< ISH PDT Unlock Msg: <b>0: False</b> 1: True
  UINT32    RsvdBits0       : 17;   ///< Reserved Bits
} PCH_ISH_CONFIG;

///
/// Premem Policy for Integrated Sensor Hub device.
///
typedef struct {
  CONFIG_BLOCK_HEADER   Header;     ///< Config Block Header
  /**
    ISH Controler 0: Disable; <b>1: Enable</b>.
    For Desktop sku, the ISH POR should be disabled. <b> 0:Disable </b>.
  **/
  UINT32    Enable          :  1;
  UINT32    RsvdBits0       : 31;   ///< Reserved Bits
} PCH_ISH_PREMEM_CONFIG;

#pragma pack (pop)

#endif // _ISH_CONFIG_H_
