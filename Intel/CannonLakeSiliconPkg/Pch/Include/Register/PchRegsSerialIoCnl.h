/** @file
  Device IDs for PCH Serial IO Controllers for CNL PCH

  Conventions:

  - Register definition format:
    Prefix_[GenerationName]_[ComponentName]_SubsystemName_RegisterSpace_RegisterName
  - Prefix:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values within the bits
    Definitions beginning with "S_" are register size
    Definitions beginning with "N_" are the bit position
  - [GenerationName]:
    Three letter acronym of the generation is used (e.g. SKL,KBL,CNL etc.).
    Register name without GenerationName applies to all generations.
  - [ComponentName]:
    This field indicates the component name that the register belongs to (e.g. PCH, SA etc.)
    Register name without ComponentName applies to all components.
    Register that is specific to -H denoted by "_PCH_H_" in component name.
    Register that is specific to -LP denoted by "_PCH_LP_" in component name.
  - SubsystemName:
    This field indicates the subsystem name of the component that the register belongs to
    (e.g. PCIE, USB, SATA, GPIO, PMC etc.).
  - RegisterSpace:
    MEM - MMIO space register of subsystem.
    IO  - IO space register of subsystem.
    PCR - Private configuration register of subsystem.
    CFG - PCI configuration space register of subsystem.
  - RegisterName:
    Full register name.

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
#ifndef _PCH_REGS_SERIAL_IO_CNL_
#define _PCH_REGS_SERIAL_IO_CNL_

//
//  Serial IO I2C0 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C0            21
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C0          0

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C0_DEVICE_ID       0x9DE8
#define V_CNL_PCH_H_SERIAL_IO_CFG_I2C0_DEVICE_ID        0xA368

//
//  Serial IO I2C1 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C1            21
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C1          1

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C1_DEVICE_ID       0x9DE9
#define V_CNL_PCH_H_SERIAL_IO_CFG_I2C1_DEVICE_ID        0xA369

//
//  Serial IO I2C2 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C2            21
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C2          2

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C2_DEVICE_ID       0x9DEA
#define V_CNL_PCH_H_SERIAL_IO_CFG_I2C2_DEVICE_ID        0xA36A

//
//  Serial IO I2C3 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C3            21
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C3          3

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C3_DEVICE_ID       0x9DEB
#define V_CNL_PCH_H_SERIAL_IO_CFG_I2C3_DEVICE_ID        0xA36B

//
//  Serial IO I2C4 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C4            25
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C4          0

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C4_DEVICE_ID       0x9DC5

//
//  Serial IO I2C5 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C5            25
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C5          1

#define V_CNL_PCH_LP_SERIAL_IO_CFG_I2C5_DEVICE_ID       0x9DC6

//
//  Serial IO SPI0 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI0            30
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI0          2

#define V_CNL_PCH_LP_SERIAL_IO_CFG_SPI0_DEVICE_ID       0x9DAA
#define V_CNL_PCH_H_SERIAL_IO_CFG_SPI0_DEVICE_ID        0xA32A

//
//  Serial IO SPI1 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI1            30
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI1          3

#define V_CNL_PCH_LP_SERIAL_IO_CFG_SPI1_DEVICE_ID       0x9DAB
#define V_CNL_PCH_H_SERIAL_IO_CFG_SPI1_DEVICE_ID        0xA32B

//
//  Serial IO SPI2 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI2            18
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI2          6

#define V_CNL_PCH_LP_SERIAL_IO_CFG_SPI2_DEVICE_ID       0x9DFB
#define V_CNL_PCH_H_SERIAL_IO_CFG_SPI2_DEVICE_ID        0xA37B

//
//  Serial IO UART0 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART0           30
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART0         0

#define V_CNL_PCH_LP_SERIAL_IO_CFG_UART0_DEVICE_ID      0x9DA8
#define V_CNL_PCH_H_SERIAL_IO_CFG_UART0_DEVICE_ID       0xA328

//
//  Serial IO UART1 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART1           30
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART1         1

#define V_CNL_PCH_LP_SERIAL_IO_CFG_UART1_DEVICE_ID      0x9DA9
#define V_CNL_PCH_H_SERIAL_IO_CFG_UART1_DEVICE_ID       0xA329

//
//  Serial IO UART2 Controller Registers
//
#define PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART2           25
#define PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART2         2

#define V_CNL_PCH_LP_SERIAL_IO_CFG_UART2_DEVICE_ID      0x9DC7
#define V_CNL_PCH_H_SERIAL_IO_CFG_UART2_DEVICE_ID       0xA347

#endif
