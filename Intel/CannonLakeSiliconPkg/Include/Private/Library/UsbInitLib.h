/** @file
  Header file for USB initialization library.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2018 Intel Corporation.

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
#ifndef _USB_INIT_LIB_H_
#define _USB_INIT_LIB_H_

#include <Ppi/SiPolicy.h>

/**
  Common entry point for PCH and CPU xDCI controller

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XdciPciMmBase       xDCI PCI config space address
**/
VOID
XdciConfigure (
  IN  USB_CONFIG      *UsbConfig,
  IN  UINT64          XhciPciMmBase
  );

/**
  Common entry point for PCH and CPU xHCI controller

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciMmBase       xHCI PCI config space address
**/
VOID
XhciConfigure (
  IN  USB_CONFIG      *UsbConfig,
  IN  UINT64          XhciPciMmBase
  );

/**
  Configure xHCI after initialization

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciMmBase       XHCI PCI CFG Base Address
**/
VOID
XhciConfigureAfterInit (
  IN  USB_CONFIG      *UsbConfig,
  IN  UINT64          XhciPciMmBase
  );

/**
  Locks xHCI configuration by setting the proper lock bits in controller

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciBase         xHCI PCI config space address
**/
VOID
XhciLockConfiguration (
  IN  USB_CONFIG      *UsbConfig,
  IN  UINT64          XhciPciBase
  );

/**
  Tune the USB 2.0 high-speed signals quality.

  @param[in]  UsbConfig           The USB_CONFIG policy instance
**/
VOID
Usb2AfeProgramming (
  IN  USB_CONFIG      *UsbConfig
  );
#endif // _USB_INIT_LIB_H_
