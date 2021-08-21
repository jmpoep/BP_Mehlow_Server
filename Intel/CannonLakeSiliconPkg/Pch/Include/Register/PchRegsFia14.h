/** @file
  Register definition for FIA 14.0 component

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
#ifndef _PCH_REGS_FIA_14_H_
#define _PCH_REGS_FIA_14_H_

#define PCH_FIA_14_MAX_DRCRM                                4
#define PCH_FIA_14_MAX_PCIE_CLKREQ                          20

#define R_PCH_FIA_14_PCR_DRCRM1                              0x100
#define R_PCH_FIA_14_PCR_DRCRM2                              0x104
#define R_PCH_FIA_14_PCR_DRCRM3                              0x108
#define R_PCH_FIA_14_PCR_DRCRM4                              0x10C
#define B_PCH_FIA_14_PCR_DRCRM4_PCIE_PORTS_MAPPING           0x3FF // In DRCRM4 only bits [9:0] are used for PCIe CLKREQ# mapping

#define N_PCH_FIA_14_PCR_DRCRM_GBECLKREQ                     10    // CLKREQ for GBE start at 10th bit in DRCRM4
#define B_PCH_FIA_14_PCR_DRCRM_GBECLKREQ                     (BIT14 | BIT13 | BIT12 | BIT11 | BIT10)
#define S_PCH_FIA_14_PCR_DRCRM_BITS_PER_FIELD                5     // CLKREQ number is encoded in 5 bits
#define B_PCH_FIA_14_PCR_DRCRM_BITS_PER_FIELD                0x1F  // CLKREQ number is encoded in 5 bits
#define S_PCH_FIA_14_PCR_DRCRM_FIELDS_PER_REG                6     // each DRCRM register contains bitfields for 6 rootports
#define V_PCH_FIA_14_PCR_DRCRM_NO_CLKREQ                     0x1F  // encoding for ClkReq not present

#define R_PCH_FIA_14_PCR_LOS1_REG_BASE                    0x250
#define R_PCH_FIA_14_PCR_LOS2_REG_BASE                    0x254
#define R_PCH_FIA_14_PCR_LOS3_REG_BASE                    0x258
#define R_PCH_FIA_14_PCR_LOS4_REG_BASE                    0x25C

#define R_PCH_FIA_14_PCR_STRPFUSECFG1_REG_BASE               0x200
#define B_PCH_FIA_14_PCR_STRPFUSECFG1_GBE_PCIE_PEN           BIT31
#define B_PCH_FIA_14_PCR_STRPFUSECFG1_GBE_PCIEPORTSEL        (BIT30 | BIT29 | BIT28)
#define N_PCH_FIA_14_PCR_STRPFUSECFG1_GBE_PCIEPORTSEL        28
#define R_PCH_FIA_14_PCR_PCIESATA_FUSECFG_REG_BASE           0x204
#define R_PCH_FIA_14_PCR_PCIESATA_STRPCFG_REG_BASE           0x208
#define R_PCH_FIA_14_PCR_PCIEUSB3_STRPFUSECFG_REG_BASE       0x20C
#define R_PCH_FIA_14_PCR_EXP_FUSECFG_REG_BASE                0x210
#define R_PCH_FIA_14_PCR_USB3SSIC_STRPFUSECFG_REG_BASE       0x214
#define R_PCH_FIA_14_PCR_CSI3_STRPFUSECFG_REG_BASE           0x218
#define R_PCH_FIA_14_PCR_USB3SATA_STRPFUSECFG_REG_BASE       0x21C
#define R_PCH_FIA_14_PCR_UFS_STRPFUSECFG_REG_BASE            0x220

#endif
