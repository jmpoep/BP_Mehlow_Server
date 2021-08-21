/**@file

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

#include <PcieRegs.h>
#include <Register/PchRegsCnvi.h>

Scope (\_SB.PCI0) {
  Device(CNVW)
  {
    Name(_ADR, 0x00140003)

    //
    // Define a Memory Region that will allow access to the CNVi WiFi PCI Configuration Space
    //
    OperationRegion(CWAR, PCI_Config, 0x00, 0x100)
    Field(CWAR, WordAcc, NoLock, Preserve) {
      VDID, 32, // 0x00, VID DID
          ,  1,
      WMSE,  1, // MSE
      WBME,  1, // BME
      Offset(R_PCI_BAR0_OFFSET),
      WBR0, 64, // BAR0
      Offset(R_CNVI_CFG_WIFI_GIO_DEV_CAP),
          , 28,
      WFLR,  1, // Function Level Reset Capable
      Offset(R_CNVI_CFG_WIFI_GIO_DEV_CTRL),
          , 15,
      WIFR,  1, // Init Function Level Reset
      Offset(R_CNVI_CFG_WIFI_PMCSR),
      WPMS, 32,
    }

    //
    // _DSM Device Specific Method for WiAMT DMA
    // Arg0: UUID Unique function identifier
    // Arg1: Integer Revision Level
    // Arg2: Integer Function Index (0 = Return Supported Functions)
    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      If (LEqual (Arg0, ToUUID ("079FF457-64A8-44BE-BD8A-6955052B9B92"))) {
        Switch (ToInteger (Arg2)) {
          //
          // Function Index 0: Return supported functions, based on revision
          // one bit for each function index, starting with zero.
          // Bit 0 - Indicates whether there is support for any functions other than function 0 for the specified UUID and Revision ID.
          // Bit 1 - Indicates support to get DMA buffer address(size: 512kB) for WiAMT
          //
          case (0) {
            switch (Arg1) {
              // revision 1: functions 1 are supported
              case (1) {
                return (Buffer() {0x3})
              }
            }
            // no functions are supported other than revision 1
            Return (Buffer() {0})
          }

          //
          // Function Index 1: Return 512KB DMA buffer address for WiAMT
          //
          Case(1) {
            Return (RMRC)
          }
        } // End - Switch (ToInteger (Arg2))
      } Else { // _DSM Mismatch (GUID not found)
        Return (Buffer() {0})
      }
    }

    Method(_S0W, 0x0, NotSerialized)
    {
      Return (0x3)
    }
    Method(_PRW, 0)
    {
      Return (GPRW(0x6D, 4)) // can wakeup from S4 state
    }

    Method(_DSW, 3) {}

    // Define Platform-level device reset power resource
    PowerResource(WRST, 5, 0)
    {
      // Define the PowerResource for CNVi WiFi
      // PowerResource expects to have _STA, _ON and _OFF Method per ACPI Spec. Not having one of them will cause BSOD

      // Method: Dummy _STA() to comply with ACPI Spec
      Method(_STA)
      {
        Return (0x01)
      }

      // Method: Dummy _ON() to comply with ACPI Spec
      Method(_ON, 0)
      {
      }

      // Method: Dummy _OFF() to comply with ACPI Spec
      Method(_OFF, 0)
      {
      }

      Method(_RST, 0, NotSerialized)
      {
        If(LEqual (WFLR, 1))
        {
          Store (0, WBR0)
          Store (0, WPMS)
          Store (0, WBME)
          Store (0, WMSE)
          Store (1, WIFR)
        }
      }
    } // End WRST

    Name(_PRR, Package(){WRST})
  }

  //
  // CNVi is present
  //
  Method(CNIP)
  {
    If (LNotEqual (\_SB.PCI0.CNVW.VDID, 0xFFFFFFFF)){
      Return (0x01)
    } Else {
      Return (0x00)
    }
  }

  //
  // Set BT_EN
  //
  Method(SBTE, 0x1, Serialized)
  {
    //
    // Arg0 - Value to BT_EN
    //
    If (LEqual (PCHS, PCHL)) {
      Store (GPIO_CNL_LP_VGPIO0, Local0)
    } Else {
      Store (GPIO_CNL_H_VGPIO0, Local0)
    }
    \_SB.SGOV (Local0, Arg0)
  }

  //
  // Get BT_EN value
  //
  Method(GBTE, 0)
  {
    If (LEqual (PCHS, PCHL)) {
      Store (GPIO_CNL_LP_VGPIO0, Local0)
    } Else {
      Store (GPIO_CNL_H_VGPIO0, Local0)
    }
    Return (\_SB.GGOV (Local0))
  }
}


