/** @file
  Register names for PCH Thermal Device

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
  Copyright 1999 - 2017 Intel Corporation.

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
#ifndef _PCH_REGS_THERMAL_H_
#define _PCH_REGS_THERMAL_H_

//
//  Thermal Device Registers (D18:0)
//
#define PCI_DEVICE_NUMBER_PCH_THERMAL   18
#define PCI_FUNCTION_NUMBER_PCH_THERMAL 0
#define R_THERMAL_CFG_MEM_TBAR              0x10
#define V_THERMAL_CFG_MEM_TBAR_SIZE         (4 * 1024)
#define N_THREMAL_CFG_MEM_TBAR_ALIGNMENT    12
#define B_THERMAL_CFG_MEM_TBAR_MASK         0xFFFFF000
#define R_THERMAL_CFG_MEM_TBARH             0x14
#define R_THERMAL_CFG_MEM_TBARB             0x40
#define V_THERMAL_CFG_MEM_TBARB_SIZE        (4 * 1024)
#define N_THREMAL_CFG_MEM_TBARB_ALIGNMENT   12
#define B_THERMAL_CFG_MEM_SPTYPEN           BIT0
#define R_THERMAL_CFG_MEM_TBARBH            0x44
#define B_THERMAL_CFG_MEM_TBARB_MASK        0xFFFFF000

//
//  Thermal TBAR MMIO registers
//
#define R_THERMAL_MEM_TSC                  0x04
#define B_THERMAL_MEM_TSC_PLD              BIT7
#define B_THERMAL_MEM_TSC_CPDE             BIT0
#define R_THERMAL_MEM_TSS                  0x06
#define R_THERMAL_MEM_TSEL                 0x08
#define B_THERMAL_MEM_TSEL_PLD             BIT7
#define B_THERMAL_MEM_TSEL_ETS             BIT0
#define R_THERMAL_MEM_TSREL                0x0A
#define R_THERMAL_MEM_TSMIC                0x0C
#define B_THERMAL_MEM_TSMIC_PLD            BIT7
#define B_THERMAL_MEM_TSMIC_SMIE           BIT0
#define R_THERMAL_MEM_CTT                  0x10
#define R_THERMAL_MEM_TAHV                 0x14
#define R_THERMAL_MEM_TALV                 0x18
#define R_THERMAL_MEM_TSPM                 0x1C
#define B_THERMAL_MEM_TSPM_LTT             (BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0)
#define V_THERMAL_MEM_TSPM_LTT             0x0C8
#define B_THERMAL_MEM_TSPM_MAXTSST         (BIT11 | BIT10 | BIT9)
#define V_THERMAL_MEM_TSPM_MAXTSST         (0x4 << 9)
#define B_THERMAL_MEM_TSPM_MINTSST         BIT12
#define B_THERMAL_MEM_TSPM_DTSSIC0         BIT13
#define B_THERMAL_MEM_TSPM_DTSSS0EN        BIT14
#define B_THERMAL_MEM_TSPM_TSPMLOCK        BIT15
#define R_THERMAL_MEM_TL                   0x40
#define B_THERMAL_MEM_TL_LOCK              BIT31
#define B_THERMAL_MEM_TL_TTEN              BIT29
#define R_THERMAL_MEM_TL2                  0x50
#define R_THERMAL_MEM_TL2_LOCK             BIT15
#define R_THERMAL_MEM_TL2_PMCTEN           BIT14
#define R_THERMAL_MEM_PHL                  0x60
#define B_THERMAL_MEM_PHLE                 BIT15
#define R_THERMAL_MEM_PHLC                 0x62
#define B_THERMAL_MEM_PHLC_LOCK            BIT0
#define R_THERMAL_MEM_TAS                  0x80
#define R_THERMAL_MEM_TSPIEN               0x82
#define R_THERMAL_MEM_TSGPEN               0x84
#define B_THERMAL_MEM_TL2_PMCTEN           BIT14
#define R_THERMAL_MEM_A4                   0xA4
#define R_THERMAL_MEM_C0                   0xC0
#define R_THERMAL_MEM_C4                   0xC4
#define R_THERMAL_MEM_C8                   0xC8
#define R_THERMAL_MEM_CC                   0xCC
#define R_THERMAL_MEM_D0                   0xD0
#define R_THERMAL_MEM_E0                   0xE0
#define R_THERMAL_MEM_E4                   0xE4
#define R_THERMAL_MEM_TCFD                 0xF0                ///< Thermal controller function disable
#define B_THERMAL_MEM_TCFD_TCD             BIT0                ///< Thermal controller disable

#endif
