/** @file
  Register definition for CNL PSF component

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
  Copyright 2015 - 2018 Intel Corporation.

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
#ifndef _PCH_REGS_PSF_CNL_H_
#define _PCH_REGS_PSF_CNL_H_

//
// Private chipset register (Memory space) offset definition
// The PCR register defines is used for PCR MMIO programming and PCH SBI programming as well.
//

//
// PSFx segment registers
//
#define R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG0_PORT0         0x4020                  ///< PSF Segment Port Configuration Register
#define R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT0         0x4024
#define R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT1         0x4028
#define R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT2         0x402C

#define V_CNL_PCH_LP_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_P2SB   0x38C00
#define V_CNL_PCH_H_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_P2SB    0x38C00
#define V_CNL_PCH_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_DMI       0x10000

//
// PSF1 PCRs
//
// PSF1 PCH-LP Specific Base Address
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI1_REG_BASE        0x0300                  ///< D22F0 PSF base address (CSME: HECI1)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI2_REG_BASE        0x0400                  ///< D22F1 PSF base address (CSME: HECI2)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_IDER_REG_BASE         0x0500                  ///< D22F2 PSF base address (CSME: IDER)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_KT_REG_BASE           0x0600                  ///< D22F3 PSF base address (CSME: KT)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI3_REG_BASE        0x0700                  ///< D22F4 PSF base address (CSME: HECI3)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI4_REG_BASE        0x0800                  ///< D22F5 PSF base address (CSME: HECI4)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_NVM_D3_F0_BASE        0x1E00                  ///< D3F0  PSF base address (uncore NVM)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_SATA_REG_BASE         0x1F00                  ///< PCH-LP D23F0 PSF base address (SATA)

#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE01_RS0_REG_BASE   0x2000                  ///< PCH-LP D28F0 PSF base address (PCIE PORT 01)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE02_RS0_REG_BASE   0x2100                  ///< PCH-LP D28F1 PSF base address (PCIE PORT 02)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE03_RS0_REG_BASE   0x2200                  ///< PCH-LP D28F2 PSF base address (PCIE PORT 03)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE04_RS0_REG_BASE   0x2300                  ///< PCH-LP D28F3 PSF base address (PCIE PORT 04)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE05_RS0_REG_BASE   0x2400                  ///< PCH-LP D28F4 PSF base address (PCIE PORT 05)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE06_RS0_REG_BASE   0x2500                  ///< PCH-LP D28F5 PSF base address (PCIE PORT 06)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE07_RS0_REG_BASE   0x2600                  ///< PCH-LP D28F6 PSF base address (PCIE PORT 07)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE08_RS0_REG_BASE   0x2700                  ///< PCH-LP D28F7 PSF base address (PCIE PORT 08)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE09_RS0_REG_BASE   0x2800                  ///< PCH-LP D29F0 PSF base address (PCIE PORT 09)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE10_RS0_REG_BASE   0x2900                  ///< PCH-LP D29F1 PSF base address (PCIE PORT 10)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE11_RS0_REG_BASE   0x2A00                  ///< PCH-LP D29F2 PSF base address (PCIE PORT 11)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE12_RS0_REG_BASE   0x2B00                  ///< PCH-LP D29F3 PSF base address (PCIE PORT 12)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE13_RS0_REG_BASE   0x2C00                  ///< PCH-LP D29F4 PSF base address (PCIE PORT 13)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE14_RS0_REG_BASE   0x2D00                  ///< PCH-LP D29F5 PSF base address (PCIE PORT 14)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE15_RS0_REG_BASE   0x2E00                  ///< PCH-LP D29F6 PSF base address (PCIE PORT 15)
#define R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE16_RS0_REG_BASE   0x2F00                  ///< PCH-LP D29F7 PSF base address (PCIE PORT 16)

#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE01_RS3_REG_BASE   0x0E00                  ///< PCH-LP D28F0 PSF base address (PCIE PORT 01)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE02_RS3_REG_BASE   0x0F00                  ///< PCH-LP D28F1 PSF base address (PCIE PORT 02)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE03_RS3_REG_BASE   0x1000                  ///< PCH-LP D28F2 PSF base address (PCIE PORT 03)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE04_RS3_REG_BASE   0x1100                  ///< PCH-LP D28F3 PSF base address (PCIE PORT 04)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE05_RS3_REG_BASE   0x1200                  ///< PCH-LP D28F4 PSF base address (PCIE PORT 05)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE06_RS3_REG_BASE   0x1300                  ///< PCH-LP D28F5 PSF base address (PCIE PORT 06)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE07_RS3_REG_BASE   0x1400                  ///< PCH-LP D28F6 PSF base address (PCIE PORT 07)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE08_RS3_REG_BASE   0x1500                  ///< PCH-LP D28F7 PSF base address (PCIE PORT 08)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE09_RS3_REG_BASE   0x1600                  ///< PCH-LP D29F0 PSF base address (PCIE PORT 09)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE10_RS3_REG_BASE   0x1700                  ///< PCH-LP D29F1 PSF base address (PCIE PORT 10)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE11_RS3_REG_BASE   0x1800                  ///< PCH-LP D29F2 PSF base address (PCIE PORT 11)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE12_RS3_REG_BASE   0x1900                  ///< PCH-LP D29F3 PSF base address (PCIE PORT 12)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE13_RS3_REG_BASE   0x1A00                  ///< PCH-LP D29F4 PSF base address (PCIE PORT 13)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE14_RS3_REG_BASE   0x1B00                  ///< PCH-LP D29F5 PSF base address (PCIE PORT 14)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE15_RS3_REG_BASE   0x1C00                  ///< PCH-LP D29F6 PSF base address (PCIE PORT 15)
#define R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE16_RS3_REG_BASE   0x1D00                  ///< PCH-LP D29F7 PSF base address (PCIE PORT 16)

// PSF1 PCH-H Specific Base Address
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE         0x0200                  ///< D22F0 PSF base address (CSME: HECI1)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI2_REG_BASE         0x0280                  ///< D22F1 PSF base address (CSME: HECI2)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_IDER_REG_BASE          0x0300                  ///< D22F2 PSF base address (CSME: IDER)
#define R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_1_REG_BASE          0x0300                  ///< PCH PSF base address (SATA 1) (CDF PCH)
#define R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_2_REG_BASE          0x0400                  ///< PCH PSF base address (SATA 2) (CDF PCH)
#define R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_3_REG_BASE          0x0500                  ///< PCH PSF base address (SATA 2) (CDF PCH)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_KT_REG_BASE            0x0380                  ///< D22F3 PSF base address (CSME: KT)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI3_REG_BASE         0x0400                  ///< D22F4 PSF base address (CSME: HECI3)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI4_REG_BASE         0x0480                  ///< D22F5 PSF base address (CSME: HECI4)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_SATA_REG_BASE          0x1300                  ///< PCH-H D23F0 PSF base address (SATA)

#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE01_RS0_REG_BASE    0x2000                  ///< PCH-H D28F0 PSF base address (PCIE PORT 01)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE02_RS0_REG_BASE    0x2080                  ///< PCH-H D28F1 PSF base address (PCIE PORT 02)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE03_RS0_REG_BASE    0x2100                  ///< PCH-H D28F2 PSF base address (PCIE PORT 03)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE04_RS0_REG_BASE    0x2180                  ///< PCH-H D28F3 PSF base address (PCIE PORT 04)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE09_RS0_REG_BASE    0x2200                  ///< PCH-H D29F0 PSF base address (PCIE PORT 09)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE10_RS0_REG_BASE    0x2280                  ///< PCH-H D29F1 PSF base address (PCIE PORT 10)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE11_RS0_REG_BASE    0x2300                  ///< PCH-H D29F2 PSF base address (PCIE PORT 11)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE12_RS0_REG_BASE    0x2380                  ///< PCH-H D29F3 PSF base address (PCIE PORT 12)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE13_RS0_REG_BASE    0x2400                  ///< PCH-H D29F4 PSF base address (PCIE PORT 13)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE14_RS0_REG_BASE    0x2480                  ///< PCH-H D29F5 PSF base address (PCIE PORT 14)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE15_RS0_REG_BASE    0x2500                  ///< PCH-H D29F6 PSF base address (PCIE PORT 15)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE16_RS0_REG_BASE    0x2580                  ///< PCH-H D29F7 PSF base address (PCIE PORT 16)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE17_RS0_REG_BASE    0x2600                  ///< PCH-H D27F0 PSF base address (PCIE PORT 17)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE18_RS0_REG_BASE    0x2680                  ///< PCH-H D27F1 PSF base address (PCIE PORT 18)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE19_RS0_REG_BASE    0x2700                  ///< PCH-H D27F2 PSF base address (PCIE PORT 19)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE20_RS0_REG_BASE    0x2780                  ///< PCH-H D27F3 PSF base address (PCIE PORT 20)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE05_RS0_REG_BASE    0x2800                  ///< PCH-H D28F4 PSF base address (PCIE PORT 05)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE06_RS0_REG_BASE    0x2880                  ///< PCH-H D28F5 PSF base address (PCIE PORT 06)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE07_RS0_REG_BASE    0x2900                  ///< PCH-H D28F6 PSF base address (PCIE PORT 07)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE08_RS0_REG_BASE    0x2980                  ///< PCH-H D28F7 PSF base address (PCIE PORT 08)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE21_RS0_REG_BASE    0x2A00                  ///< PCH-H D27F4 PSF base address (PCIE PORT 21)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE22_RS0_REG_BASE    0x2A80                  ///< PCH-H D27F5 PSF base address (PCIE PORT 22)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE23_RS0_REG_BASE    0x2B00                  ///< PCH-H D27F6 PSF base address (PCIE PORT 23)
#define R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE24_RS0_REG_BASE    0x2B80                  ///< PCH-H D27F7 PSF base address (PCIE PORT 24)

#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE01_RS3_REG_BASE    0x0700                  ///< PCH-H D28F0 PSF base address (PCIE PORT 01)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE02_RS3_REG_BASE    0x0780                  ///< PCH-H D28F1 PSF base address (PCIE PORT 02)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE03_RS3_REG_BASE    0x0800                  ///< PCH-H D28F2 PSF base address (PCIE PORT 03)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE04_RS3_REG_BASE    0x0880                  ///< PCH-H D28F3 PSF base address (PCIE PORT 04)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE09_RS3_REG_BASE    0x0900                  ///< PCH-H D29F0 PSF base address (PCIE PORT 09)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE10_RS3_REG_BASE    0x0980                  ///< PCH-H D29F1 PSF base address (PCIE PORT 10)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE11_RS3_REG_BASE    0x0A00                  ///< PCH-H D29F2 PSF base address (PCIE PORT 11)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE12_RS3_REG_BASE    0x0A80                  ///< PCH-H D29F3 PSF base address (PCIE PORT 12)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE13_RS3_REG_BASE    0x0B00                  ///< PCH-H D29F4 PSF base address (PCIE PORT 13)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE14_RS3_REG_BASE    0x0B80                  ///< PCH-H D29F5 PSF base address (PCIE PORT 14)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE15_RS3_REG_BASE    0x0C00                  ///< PCH-H D29F6 PSF base address (PCIE PORT 15)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE16_RS3_REG_BASE    0x0C80                  ///< PCH-H D29F7 PSF base address (PCIE PORT 16)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE17_RS3_REG_BASE    0x0D00                  ///< PCH-H D27F0 PSF base address (PCIE PORT 17)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE18_RS3_REG_BASE    0x0D80                  ///< PCH-H D27F1 PSF base address (PCIE PORT 18)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE19_RS3_REG_BASE    0x0E00                  ///< PCH-H D27F2 PSF base address (PCIE PORT 19)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE20_RS3_REG_BASE    0x0E80                  ///< PCH-H D27F3 PSF base address (PCIE PORT 20)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE05_RS3_REG_BASE    0x0F00                  ///< PCH-H D28F4 PSF base address (PCIE PORT 05)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE06_RS3_REG_BASE    0x0F80                  ///< PCH-H D28F5 PSF base address (PCIE PORT 06)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE07_RS3_REG_BASE    0x0000                  ///< PCH-H D28F6 PSF base address (PCIE PORT 07)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE08_RS3_REG_BASE    0x0080                  ///< PCH-H D28F7 PSF base address (PCIE PORT 08)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE21_RS3_REG_BASE    0x1100                  ///< PCH-H D27F4 PSF base address (PCIE PORT 21)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE22_RS3_REG_BASE    0x1180                  ///< PCH-H D27F5 PSF base address (PCIE PORT 22)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE23_RS3_REG_BASE    0x1200                  ///< PCH-H D27F6 PSF base address (PCIE PORT 23)
#define R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE24_RS3_REG_BASE    0x1280                  ///< PCH-H D27F7 PSF base address (PCIE PORT 24)

// Other PSF1 PCRs definition
#define R_CNL_PCH_LP_PSF1_PCR_PSF_PORT_CONFIG_PG1_PORT7                 0x4040      ///< PSF Port Configuration Register
#define B_PCH_PSFX_PCR_PORT_CONFIG_PGX_PORTX_ISM_CPL_TRACK              BIT5        ///< ISM Completion Tracking Enable

//PSF 1 Multicast Message Configuration
#define R_CNL_PCH_LP_PSF1_PCR_PSF_MC_CONTROL_MCAST0_EOI                 0x404C      ///< Multicast Control Register
#define R_CNL_PCH_LP_PSF1_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI              0x4064      ///< Destination ID
#define R_CNL_PCH_H_PSF1_PCR_PSF_MC_CONTROL_MCAST0_EOI                  0x403C      ///< Multicast Control Register
#define R_CNL_PCH_H_PSF1_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI               0x4054      ///< Destination ID

#define R_CNL_PCH_LP_PSF1_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1           0x4058      ///< Multicast Control Register
#define R_CNL_PCH_LP_PSF1_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1        0x40B0      ///< Destination ID
#define R_CNL_PCH_H_PSF1_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1            0x4048      ///< Multicast Control Register
#define R_CNL_PCH_H_PSF1_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1         0x4078      ///< Destination ID

//
// controls the PCI configuration header of a PCI function
//
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F7   0x4328   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F6   0x432C   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F5   0x4330   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F4   0x4334   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F3   0x4338   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F2   0x433C   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F1   0x4340   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F0   0x4344   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F7   0x4348   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F6   0x434C   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F5   0x4350   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F4   0x4354   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F3   0x4358   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F2   0x435C   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F1   0x4360   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F0   0x4364   ///< SPA

#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F7   0x42B4   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F6   0x42B8   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F5   0x42BC   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F4   0x42C0   ///< SPD
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F3   0x42C4   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F2   0x42C8   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F1   0x42CC   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F0   0x42D0   ///< SPC
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F7   0x42D4   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F6   0x42D8   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F5   0x42DC   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F4   0x42E0   ///< SPB
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F3   0x42E4   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F2   0x42E8   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F1   0x42EC   ///< SPA
#define R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F0   0x42F0   ///< SPA


#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F7    0x4250   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F6    0x4254   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F5    0x4258   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F4    0x425C   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F7    0x4260   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F6    0x4264   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F5    0x4268   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F4    0x426C   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F3    0x4270   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F2    0x4274   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F1    0x4278   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F0    0x427C   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F7    0x4280   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F6    0x4284   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F5    0x4288   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F4    0x428C   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F3    0x4290   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F2    0x4294   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F1    0x4298   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F0    0x429C   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F3    0x42A0   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F2    0x42A4   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F1    0x42A8   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F0    0x42AC   ///< SPA

#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F7    0x41C0   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F6    0x41C4   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F5    0x41C8   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F4    0x41CC   ///< SPF
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F7    0x41D0   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F6    0x41D4   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F5    0x41D8   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F4    0x41DC   ///< SPB
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F3    0x41E0   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F2    0x41E4   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F1    0x41E8   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F0    0x41EC   ///< SPE
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F7    0x41F0   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F6    0x41F4   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F5    0x41F8   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F4    0x41FC   ///< SPD
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F3    0x4200   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F2    0x4204   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F1    0x4208   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F0    0x420C   ///< SPC
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F3    0x4210   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F2    0x4214   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F1    0x4218   ///< SPA
#define R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F0    0x421C   ///< SPA

//
// PSF1 grant count registers
//
#define R_CNL_PCH_LP_PSF1_DEV_GNTCNT_RELOAD_DGCR0           0x436C
#define R_CNL_PCH_LP_PSF1_TARGET_GNTCNT_RELOAD_PG1_TGT0     0x4610
#define R_CNL_PCH_H_PSF1_DEV_GNTCNT_RELOAD_DGCR0            0x42B4
#define R_CNL_PCH_H_PSF1_TARGET_GNTCNT_RELOAD_PG1_TGT0      0x43F0

//
// PSF2 PCRs (PID:PSF2)
//
#define R_CNL_PCH_PSF2_PCR_T0_SHDW_TRH_REG_BASE                0x0100               ///< D18F0 PSF base address (Thermal). // LP&H
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_UFS_REG_BASE             0x0200               ///< D18F5 PSF base address (SCC: UFS)
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_XHCI_REG_BASE            0x0300               ///< D20F0 PSF base address (XHCI)
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_OTG_REG_BASE             0x0400               ///< D20F1 PSF base address (USB device controller: OTG)
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_TRACE_HUB_ACPI_REG_BASE  0x0500               ///< D20F4 PSF base address (TraceHub ACPI) // LP
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_EMMC_REG_BASE            0x0600               ///< D26F0 PSF base address (SCC: eMMC)
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_TRACE_HUB_REG_BASE       0x0700               ///< D31F7 PSF base address (TraceHub PCI) // LP
#define R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_XHCI_VTIO_REG_BASE       0x0800               ///< D20F6 PSF base address (XHCI VTIO)

#define R_CNL_PCH_H_PSF2_PCR_T0_SHDW_XHCI_REG_BASE             0x0180               ///< D20F0 PSF base address (XHCI)
#define R_CNL_PCH_H_PSF2_PCR_T0_SHDW_OTG_REG_BASE              0x0200               ///< D20F1 PSF base address (USB device controller: OTG)
#define R_CNL_PCH_H_PSF2_PCR_T0_SHDW_TRACE_HUB_ACPI_REG_BASE   0x0280               ///< D20F4 PSF base address (TraceHub ACPI) // H
#define R_CNL_PCH_H_PSF2_PCR_T0_SHDW_TRACE_HUB_REG_BASE        0x0300               ///< D31F7 PSF base address (TraceHub PCI) // H
#define R_CNL_PCH_H_PSF2_PCR_T0_SHDW_XHCI_VTIO_REG_BASE        0x0380               ///< D20F6 PSF base address (XHCI VTIO)

// Other PSF2 PCRs definition
#define R_CNL_PCH_LP_PSF2_PCR_PSF_PORT_CONFIG_PG1_PORT4        0x4034               ///< PSF Port Configuration Register

//
// PSF3 PCRs (PID:PSF3)
//
// PSF3 PCH-LP Specific Base Address
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI2_REG_BASE         0x0100                  ///< D18F6 PSF base address (SerialIo: SPI2)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_ISH_REG_BASE          0x0200                  ///< D19F0 PSF base address (ISH)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_CNVI_REG_BASE         0x0400                  ///< D20F3 PSF base address (CNVi)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SDCARD_REG_BASE       0x0500                  ///< D20F5 PSF base address (SCC: SDCard)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C0_REG_BASE         0x0600                  ///< D21F0 PSF base address (SerialIo: I2C0)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C1_REG_BASE         0x0700                  ///< D21F1 PSF base address (SerialIo: I2C1)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C2_REG_BASE         0x0800                  ///< D21F2 PSF base address (SerialIo: I2C2)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C3_REG_BASE         0x0900                  ///< D21F3 PSF base address (SerialIo: I2C3)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C4_REG_BASE         0x0A00                  ///< D25F0 PSF base address (SerialIo: I2C4)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C5_REG_BASE         0x0B00                  ///< D25F1 PSF base address (SerialIo: I2C5)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART2_REG_BASE        0x0C00                  ///< D25F2 PSF base address (SerialIo: UART2)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART0_REG_BASE        0x0D00                  ///< D30F0 PSF base address (SerialIo: UART0)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART1_REG_BASE        0x0E00                  ///< D30F1 PSF base address (SerialIo: UART1)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI0_REG_BASE         0x0F00                  ///< D30F2 PSF base address (SerialIo: SPI0)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI1_REG_BASE         0x1000                  ///< D30F3 PSF base address (SerialIo: SPI1)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_LPC_REG_BASE          0x1100                  ///< D31F0 PSF base address (LPC)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_P2SB_REG_BASE         0x1300                  ///< D31F1 PSF base address (P2SB)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_PMC_REG_BASE          0x1400                  ///< D31F2 PSF base address (PMC)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_AUD_REG_BASE          0x1500                  ///< D31F3 PSF base address (HDA, ADSP)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SMBUS_REG_BASE        0x1600                  ///< D31F4 PSF base address (SMBUS)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI_SPI_REG_BASE      0x1700                  ///< D31F5 PSF base address (SPI SPI)
#define R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_GBE_REG_BASE          0x1800                  ///< D31F6 PSF base address (GBE)

// PSF3 PCH-H Specific Base Address
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI2_REG_BASE          0x0100                  ///< D18F6 PSF base address (SerialIo: SPI2)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_ISH_REG_BASE           0x0180                  ///< D19F0 PSF base address (ISH)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_CNVI_REG_BASE          0x0280                  ///< D20F3 PSF base address (CNVi)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SDCARD_REG_BASE        0x0300                  ///< D20F5 PSF base address (SCC: SDCard)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C0_REG_BASE          0x0380                  ///< D21F0 PSF base address (SerialIo: I2C0)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C1_REG_BASE          0x0400                  ///< D21F1 PSF base address (SerialIo: I2C1)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C2_REG_BASE          0x0480                  ///< D21F2 PSF base address (SerialIo: I2C2)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C3_REG_BASE          0x0500                  ///< D21F3 PSF base address (SerialIo: I2C3)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART2_REG_BASE         0x0580                  ///< D25F2 PSF base address (SerialIo: UART2)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART0_REG_BASE         0x0600                  ///< D30F0 PSF base address (SerialIo: UART0)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART1_REG_BASE         0x0680                  ///< D30F1 PSF base address (SerialIo: UART1)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI0_REG_BASE          0x0700                  ///< D30F2 PSF base address (SerialIo: SPI0)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI1_REG_BASE          0x0780                  ///< D30F3 PSF base address (SerialIo: SPI1)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_LPC_REG_BASE           0x0800                  ///< D31F0 PSF base address (LPC)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_P2SB_REG_BASE          0x0900                  ///< D31F1 PSF base address (P2SB)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_PMC_REG_BASE           0x0980                  ///< D31F2 PSF base address (PMC)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_AUD_REG_BASE           0x0A00                  ///< D31F3 PSF base address (HDA, ADSP)H
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SMBUS_REG_BASE         0x0A80                  ///< D31F4 PSF base address (SMBUS)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI_SPI_REG_BASE       0x0B00                  ///< D31F5 PSF base address (SPI SPI)
#define R_CNL_PCH_H_PSF3_PCR_T0_SHDW_GBE_REG_BASE           0x0B80                  ///< D31F6 PSF base address (GBE)

// Other PSF3 PCRs definition
#define R_CNL_PCH_LP_PSF3_PCR_PSF_PORT_CONFIG_PG1_PORT0     0x4024                  ///< PSF Port Configuration Register
#define R_CNL_PCH_PSF3_PCR_PSF_MC_CONTROL_MCAST0_EOI        0x4058                  ///< Multicast Control Register  // LP&H
#define R_CNL_PCH_PSF3_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI     0x4064                  ///< Destination ID  // LP&H

//
// PSF4 PCRs (PID:PSF4)
//
//
// PSF4 grant count registers
//
#define R_CNL_PCH_LP_PSF4_PCR_DEV_GNTCNT_RELOAD_DGCR0       0x4050                  ///< Grant Count Reload Register
#define R_CNL_PCH_H_PSF4_PCR_DEV_GNTCNT_RELOAD_DGCR0        0x407C                  ///< Grant Count Reload Register

//
// PSF6 PCRs (PID:PSF6)
//
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE01_REG_BASE        0x2000                  ///< PCH-H D28F0 PSF base address (PCIE PORT 01)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE02_REG_BASE        0x2080                  ///< PCH-H D28F1 PSF base address (PCIE PORT 02)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE03_REG_BASE        0x2100                  ///< PCH-H D28F2 PSF base address (PCIE PORT 03)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE04_REG_BASE        0x2180                  ///< PCH-H D28F3 PSF base address (PCIE PORT 04)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE09_REG_BASE        0x2200                  ///< PCH-H D29F0 PSF base address (PCIE PORT 09)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE10_REG_BASE        0x2280                  ///< PCH-H D29F1 PSF base address (PCIE PORT 10)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE11_REG_BASE        0x2300                  ///< PCH-H D29F2 PSF base address (PCIE PORT 11)
#define R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE12_REG_BASE        0x2380                  ///< PCH-H D29F3 PSF base address (PCIE PORT 10)

#define R_CNL_PCH_H_PSF6_PCR_PSF_MC_CONTROL_MCAST0_EOI              0x4030          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF6_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI           0x4048          ///< Destination ID
#define R_CNL_PCH_H_PSF6_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1        0x403C          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF6_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1     0x4070          ///< Destination ID

#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F3    0x4098          ///< SPC
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F2    0x409C          ///< SPC
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F1    0x40A0          ///< SPC
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F0    0x40A4          ///< SPC
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F3    0x40A8          ///< SPA
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F2    0x40AC          ///< SPA
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F1    0x40B0          ///< SPA
#define R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F0    0x40B4          ///< SPA

#define R_CNL_PCH_H_PSF6_PCR_DEV_GNTCNT_RELOAD_DGCR0        0x40BC
#define R_CNL_PCH_H_PSF6_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0  0x41A0

//
// PSF7 PCRs (PID:PSF7)
//
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE13_REG_BASE        0x2000                  ///< PCH-H D29F4 PSF base address (PCIE PORT 13)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE14_REG_BASE        0x2080                  ///< PCH-H D29F5 PSF base address (PCIE PORT 14)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE15_REG_BASE        0x2100                  ///< PCH-H D29F6 PSF base address (PCIE PORT 15)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE16_REG_BASE        0x2180                  ///< PCH-H D29F7 PSF base address (PCIE PORT 16)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE17_REG_BASE        0x2200                  ///< PCH-H D27F0 PSF base address (PCIE PORT 17)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE18_REG_BASE        0x2280                  ///< PCH-H D27F1 PSF base address (PCIE PORT 18)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE19_REG_BASE        0x2300                  ///< PCH-H D27F2 PSF base address (PCIE PORT 19)
#define R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE20_REG_BASE        0x2380                  ///< PCH-H D27F3 PSF base address (PCIE PORT 20)

#define R_CNL_PCH_H_PSF7_PCR_PSF_MC_CONTROL_MCAST0_EOI              0x4030          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF7_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI           0x4048          ///< Destination ID
#define R_CNL_PCH_H_PSF7_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1        0x403C          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF7_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1     0x4070          ///< Destination ID

#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F3    0x4098          ///< SPE
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F2    0x409C          ///< SPE
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F1    0x40A0          ///< SPE
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F0    0x40A4          ///< SPE
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F7    0x40A8          ///< SPD
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F6    0x40AC          ///< SPD
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F5    0x40B0          ///< SPD
#define R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F4    0x40B4          ///< SPD

#define R_CNL_PCH_H_PSF7_PCR_DEV_GNTCNT_RELOAD_DGCR0        0x40BC
#define R_CNL_PCH_H_PSF7_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0  0x41A0

//
// PSF8 PCRs (PID:PSF8)
//
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE05_REG_BASE        0x2000                  ///< PCH-H D28F4 PSF base address (PCIE PORT 05)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE06_REG_BASE        0x2080                  ///< PCH-H D28F5 PSF base address (PCIE PORT 06)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE07_REG_BASE        0x2100                  ///< PCH-H D28F6 PSF base address (PCIE PORT 07)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE08_REG_BASE        0x2180                  ///< PCH-H D28F7 PSF base address (PCIE PORT 08)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE21_REG_BASE        0x2200                  ///< PCH-H D27F4 PSF base address (PCIE PORT 21)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE22_REG_BASE        0x2280                  ///< PCH-H D27F5 PSF base address (PCIE PORT 22)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE23_REG_BASE        0x2300                  ///< PCH-H D27F6 PSF base address (PCIE PORT 23)
#define R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE24_REG_BASE        0x2380                  ///< PCH-H D27F7 PSF base address (PCIE PORT 24)

#define R_CNL_PCH_H_PSF8_PCR_PSF_MC_CONTROL_MCAST0_EOI              0x4030          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF8_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI           0x4048          ///< Destination ID
#define R_CNL_PCH_H_PSF8_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1        0x403C          ///< Multicast Control Register
#define R_CNL_PCH_H_PSF8_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1     0x4070          ///< Destination ID

#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F7    0x4098          ///< SPF
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F6    0x409C          ///< SPF
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F5    0x40A0          ///< SPF
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F4    0x40A4          ///< SPF
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F7    0x40A8          ///< SPB
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F6    0x40AC          ///< SPB
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F5    0x40B0          ///< SPB
#define R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F4    0x40B4          ///< SPB

#define R_CNL_PCH_H_PSF8_PCR_DEV_GNTCNT_RELOAD_DGCR0        0x40BC
#define R_CNL_PCH_H_PSF8_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0  0x41A0
#endif
