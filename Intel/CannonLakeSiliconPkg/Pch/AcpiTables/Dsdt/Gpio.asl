/**@file
 ACPI definition for GPIO controller

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
// PCH_CNL - Default
Include("GpioAcpiDefinesCnl.h")

Scope(\_SB.PCI0) {
  //----------------------------
  //  GPIO Controller
  //----------------------------
  Device (GPI0)
  {
    Method (_HID) {
      // Return motherboard reserved resources HID when GPIO is hidden
      if(LEqual(GPHD, 1)) { Return("PNP0C02") }
      // Return HID based on PCH Product
      if(LEqual(PCHS, PCHH)){ Return(GPIO_CNL_H_ACPI_HID) }
      Return(GPIO_CNL_LP_ACPI_HID)
    }

    Name (LINK,"\\_SB.PCI0.GPI0")

    Method (_CRS, 0x0, NotSerialized) {
      Name (RBUF, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x00000000, 0x00010000, RBR0)
        Memory32Fixed (ReadWrite, 0x00000000, 0x00010000, RBR1)
        Memory32Fixed (ReadWrite, 0x00000000, 0x00010000, RBR2)
        Interrupt (ResourceConsumer, Level, ActiveLow, Shared, , , IRQ) { 14 } //Interrupt IRQ_EN
      })
      Name (CBUF, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0x00000000, 0x00010000, CBR0)
      })

      CreateDWordField(RBUF,RBR0._BAS,COM0)
      CreateDWordField(RBUF,RBR1._BAS,COM1)
      CreateDWordField(RBUF,IRQ._INT,IRQN)
      Store( Add(SBRG,PCH_GPIO_COM0), COM0)
      Store( Add(SBRG,PCH_GPIO_COM1), COM1)
      Store(SGIR,IRQN)

      if(LEqual(PCHS, PCHH)){
        CreateDWordField(RBUF,RBR2._BAS,CMH3)
        Store( Add(SBRG,PCH_GPIO_COM3), CMH3)
        CreateDWordField(CBUF,CBR0._BAS,CMH4)
        Store( Add(SBRG,PCH_GPIO_COM4), CMH4)
        Return (ConcatenateResTemplate(RBUF, CBUF)) // Combine resources as PCH-H uses 1 more community
      } Else {
        CreateDWordField(RBUF,RBR2._BAS,CML4)
        Store( Add(SBRG,PCH_GPIO_COM4), CML4)
        Return (RBUF)
      }
    }

    Method (_STA, 0x0, NotSerialized) {
      If(LEqual(GPHD, 1)) { // Hide GPIO ACPI device
        Return(0x3)
      }
      Return(0xF)
    }
  } // END Device(GPIO)
} // END Scope(\_SB.PCI0)

