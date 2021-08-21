/** @file
  CNVI policy

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
#ifndef _CNVI_CONFIG_H_
#define _CNVI_CONFIG_H_

#define CNVI_CONFIG_REVISION 2
extern EFI_GUID gCnviConfigGuid;

#pragma pack (push,1)

/**
  CNVi Mode options
**/
typedef enum {
  CnviModeDisabled = 0,
  CnviModeAuto
} CNVI_MODE;

/**
  CNVi MfUart1 connection options
**/
typedef enum {
  CnviMfUart1Ish = 0,
  CnviMfUart1SerialIo,
  CnviBtUart1ExtPads,
  CnviBtUart1NotConnected
} CNVI_MFUART1_TYPE;


/**
  The PCH_CNVI_CONFIG block describes CNVi IP in CNL-PCH.

  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Remove BtInterface and BtUartType.

**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;               ///< Config Block Header
  /**
    This option allows for automatic detection of Connectivity Solution.
    Auto Detection assumes that CNVi will be enabled when available;
    Disable allows for disabling CNVi.
    CnviModeDisabled = Disabled,
    <b>CnviModeAuto = Auto Detection</b>
  **/
  UINT32 Mode                  :  1;
  /**
    <b>(Test)</b> This option configures Uart type which connects to MfUart1
    For production configuration ISH is the default, for tests SerialIO Uart0 or external pads can be used
    Use CNVI_MFUART1_TYPE enum for selection
    <b>CnviMfUart1Ish = MfUart1 over ISH Uart0</b>,
    CnviMfUart1SerialIo = MfUart1 over SerialIO Uart2,
    CnviBtUart1ExtPads = MfUart1 over exteranl pads,
    CnviBtUart1NotConnected = MfUart1 not connected
  **/
  UINT32 MfUart1Type           :  2;
  UINT32 RsvdBits              : 29;
} PCH_CNVI_CONFIG;

#pragma pack (pop)

#endif // _CNVI_CONFIG_H_

