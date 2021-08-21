/** @file
  Register names for PEG block
  <b>Conventions</b>:
  - Prefixes:
    - Definitions beginning with "R_" are registers
    - Definitions beginning with "B_" are bits within registers
    - Definitions beginning with "V_" are meaningful values of bits within the registers
    - Definitions beginning with "S_" are register sizes
    - Definitions beginning with "N_" are the bit position
  - In general, SA registers are denoted by "_SA_" in register names
  - Registers / bits that are different between SA generations are denoted by
    "_SA_[generation_name]_" in register/bit names. e.g., "_SA_HSW_"
  - Registers / bits that are different between SKUs are denoted by "_[SKU_name]"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a SA generation will be just named
    as "_SA_" without [generation_name] inserted.

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
#ifndef _SA_REGS_PEG_H_
#define _SA_REGS_PEG_H_
//
// Device 1 Memory Mapped IO Register Offset Equates
//
#define SA_PEG_BUS_NUM     0x00
#define SA_PEG_DEV_NUM     0x01
#define SA_PEG0_DEV_NUM    SA_PEG_DEV_NUM
#define SA_PEG0_FUN_NUM    0x00
#define SA_PEG1_DEV_NUM    SA_PEG_DEV_NUM
#define SA_PEG1_FUN_NUM    0x01
#define SA_PEG2_DEV_NUM    SA_PEG_DEV_NUM
#define SA_PEG2_FUN_NUM    0x02
#define SA_PEG3_DEV_NUM    0x06
#define SA_PEG3_FUN_NUM    0x00

#define V_SA_PEG_VID       0x8086
#ifdef CPU_CFL
#define V_SA_PEG_DID       0x2A41
#else
#define V_SA_PEG0_DID     0x5A01
#define V_SA_PEG1_DID     0x5A05
#define V_SA_PEG2_DID     0x5A09
#endif

#define V_PH3_FS_CR_OVR       0x3E
#define B_PH3_FS_CR_OVR_EN    BIT8
#define V_PH3_LF_CR_OVR       0x14
#define B_PH3_LF_CR_OVR_EN    BIT16

#define V_IMR_BASE_LOW_MSB    0xF80

//
// Temporary Device & Function Number used for Switchable Graphics DGPU
//
#define SA_TEMP_DGPU_DEV   0x00
#define SA_TEMP_DGPU_FUN   0x00

//
// SA PCI Express* Port configuration
//
#ifndef CPU_CFL
#define SA_PEG_MAX_FUN     0x04
#define SA_PEG_MAX_LANE    0x14
#define SA_PEG_MAX_BUNDLE  0x0A
#else
#define SA_PEG_MAX_FUN     0x03
#define SA_PEG_MAX_LANE    0x10
#define SA_PEG_MAX_BUNDLE  0x08
#endif

#define SA_PEG0_CNT_MAX_LANE     0x10
#define SA_PEG0_CNT_MAX_BUNDLE   0x08
#define SA_PEG0_CNT_FIRST_LANE   0x00
#define SA_PEG0_CNT_FIRST_BUNDLE 0x00
#define SA_PEG0_CNT_LAST_LANE    0x0F
#define SA_PEG0_CNT_LAST_BUNDLE  0x07

#ifndef CPU_CFL
#define SA_PEG3_CNT_MAX_LANE     0x04
#define SA_PEG3_CNT_MAX_BUNDLE   0x02
#define SA_PEG3_CNT_FIRST_LANE   0x00
#define SA_PEG3_CNT_FIRST_BUNDLE 0x00
#define SA_PEG3_CNT_LAST_LANE    0x03
#define SA_PEG3_CNT_LAST_BUNDLE  0x01
#else
#define SA_PEG3_CNT_MAX_LANE     0x00
#define SA_PEG3_CNT_MAX_BUNDLE   0x00
#define SA_PEG3_CNT_FIRST_LANE   0x00
#define SA_PEG3_CNT_FIRST_BUNDLE 0x00
#define SA_PEG3_CNT_LAST_LANE    0x00
#define SA_PEG3_CNT_LAST_BUNDLE  0x00
#endif
//
// Silicon and SKU- specific MAX defines
//
#define SA_PEG_CNL_H_MAX_FUN           SA_PEG_MAX_FUN      // CNL-H- SKU supports 4 controllers with 20 PEG lanes and 10 bundles
#define SA_PEG_CNL_H_MAX_LANE          SA_PEG_MAX_LANE
#define SA_PEG_CNL_H_MAX_BUNDLE        SA_PEG_MAX_BUNDLE
#define SA_PEG_NON_CNL_H_MAX_FUN       0x03                // All non-CNL-H- SKU supports 3 controllers with 16 PEG lanes and 8 bundles
#define SA_PEG_NON_CNL_H_MAX_LANE      0x10
#define SA_PEG_NON_CNL_H_MAX_BUNDLE    0x08


#define SA_SWING_FULL      0x0
#define SA_SWING_REDUCED   0x1
#define SA_SWING_MINIMUM   0x2

//
// PCI Express* Port configuration Hardware Strapping value
//
#define SA_PEG_x8_x4_x4  0x00
#define SA_PEG_x8_x8_x0  0x02
#define SA_PEG_x16_x0_x0 0x03

///
/// Device 1 Register Equates
///
#define R_SA_PEG_VID_OFFSET            0x00  ///< Vendor ID
#define R_SA_PEG_DID_OFFSET            0x02  ///< Device ID
#define R_SA_PEG_BCTRL_OFFSET          0x3E  ///< Bridge Control
#define R_SA_PEG_SSCAPID_OFFSET        0x88  ///< Subsystem ID and Vendor ID Capabilities IS
#define R_SA_PEG_SS_OFFSET             0x8C  ///< Subsystem ID
#define R_SA_PEG_MSICAPID_OFFSET       0x90  ///< MSI Capabilities ID
#define R_SA_PEG_MC_OFFSET             0x92  ///< Message Control
#define R_SA_PEG_MA_OFFSET             0x94  ///< Message Address
#define R_SA_PEG_MD_OFFSET             0x98  ///< Message Data
#define R_SA_PEG_CAPL_OFFSET           0xA0  ///< PEG Capabilities List
#define R_SA_PEG_CAP_OFFSET            0xA2  ///< PEG Capabilities
#define R_SA_PEG_DCAP_OFFSET           0xA4  ///< Device Capabilities
#define R_SA_PEG_DCTL_OFFSET           0xA8  ///< Device Control
#define R_SA_PEG_DSTS_OFFSET           0xAA  ///< Device Status
#define R_SA_PEG_LCAP_OFFSET           0xAC  ///< Link Capabilities
#define R_SA_PEG_LCTL_OFFSET           0xB0  ///< Link Control
#define R_SA_PEG_LSTS_OFFSET           0xB2  ///< Link Status
#define R_SA_PEG_SLOTCAP_OFFSET        0xB4  ///< Slot Capabilities
#define R_SA_PEG_SLOTCTL_OFFSET        0xB8  ///< Slot Control
#define R_SA_PEG_SLOTSTS_OFFSET        0xBA  ///< Slot Status
#define R_SA_PEG_RCTL_OFFSET           0xBC  ///< Root Control
#define R_SA_PEG_RSTS_OFFSET           0xC0  ///< Root Status
#define R_SA_PEG_DCAP2_OFFSET          0xC4  ///< Device Capability 2
#define R_SA_PEG_DCTL2_OFFSET          0xC8  ///< Device Control 2
#define R_SA_PEG_LCTL2_OFFSET          0xD0  ///< Link Control 2
#define R_SA_PEG_LSTS2_OFFSET          0xD2  ///< Link Status 2
#define R_SA_PEG_PEGLC_OFFSET          0xEC  ///< PEG Legacy Control
#define R_SA_PEG_OFFSET_F0             0xF0  ///< PEG Offset F0
#define R_SA_PEG_OFFSET_F4             0x0F4 ///< PEG Offset F4
#define R_SA_PEG_VCECH_OFFSET          0x100 ///< PEG Virtual Channel Enhanced Capability
#define R_SA_PEG_VCCAP1_OFFSET         0x104 ///< PEG Port VC Capability
#define R_SA_PEG_VCCAP2_OFFSET         0x108 ///< PEG Port VC Capability 2
#define R_SA_PEG_VCCTL_OFFSET          0x10C ///< PEG Port VC Control
#define R_SA_PEG_VC0RCAP_OFFSET        0x110 ///< PEG VC0 Resource Capability
#define R_SA_PEG_VC0RCTL0_OFFSET       0x114 ///< PEG VC0 Resource Control
#define R_SA_PEG_VC0RSTS_OFFSET        0x11A ///< PEG VC0 Resource Status
#define R_SA_PEG_RCLDECH_OFFSET        0x140 ///< PEG Root Complex Link Declaration
#define R_SA_PEG_ESD_OFFSET            0x144 ///< PEG Element Self Description
#define R_SA_PEG_LE1D_OFFSET           0x150 ///< PEG Link Entry 1 Description
#define R_SA_PEG_LE1A_OFFSET           0x158 ///< PEG Link Entry 1 Address
#define R_SA_PEG_ACSCTLR_OFFSET        0x168 ///< PEG ACS Control Register
#define R_SA_PCIE_CFG_PTMCAPR          0x174 ///< PEG PTM Capability Register
#define R_SA_PCIE_CFG_PTMCTLR          0x178 ///< PEG PTM Control Register
#define R_SA_PCIE_CFG_PTMECFG          0x188 ///< PEG PTM Extended Configuration
#define R_SA_PCIE_CFG_PTMPSDC1         0x194 ///< PEG PTM Pipe Stage Delay Configuration 1
#define R_SA_PCIE_CFG_PTMPSDC2         0x198 ///< PEG PTM Pipe Stage Delay Configuration 2
#define R_SA_PCIE_CFG_PTMPSDC3         0x19C ///< PEG PTM Pipe Stage Delay Configuration 3
#define R_SA_PCIE_CFG_PTMPSDC4         0x1A0 ///< PEG PTM Pipe Stage Delay Configuration 4
#define R_SA_PCIE_CFG_PTMPSDC5         0x1A4 ///< PEG PTM Pipe Stage Delay Configuration 5
#define R_SA_PCIE_CFG_PTMPSDC6         0x1A8 ///< PEG PTM Pipe Stage Delay Configuration 6
#define R_SA_PCIE_CFG_PTMPSDC7         0x1AC ///< PEG PTM Pipe Stage Delay Configuration 7
#define R_SA_PCIE_CFG_PTMPSDC8         0x1B0 ///< PEG PTM Pipe Stage Delay Configuration 8
#define R_SA_PEG_PEGUESTS_OFFSET       0x1C4 ///< PEG Error Status
#define R_SA_PEG_PEGCESTS_OFFSET       0x1D0 ///< PEG Error Status
#define R_SA_PEG_PEGCC_OFFSET          0x208 ///< PEG Countdown Control
#define R_SA_PEG_PEGSTS_OFFSET         0x214 ///< PEG Status
#define R_SA_PEG_LTSSMC_OFFSET         0x224 ///< PEG LTSSMC Control
#define R_SA_PEG_L0SLAT_OFFSET         0x22C ///< PEG L0s Control
#define R_SA_PEG_CFG2_OFFSET           0x250 ///< PEG Config 2
#define R_SA_PEG_CFG4_OFFSET           0x258 ///< PEG Config 4
#define R_SA_PEG_CFG5_OFFSET           0x25C ///< PEG Config 5
#define R_SA_PEG_CFG6_OFFSET           0x260 ///< PEG Config 6
#define R_SA_PEG_VC0PRCA_OFFSET        0x308 ///< PEG VC0 Posted
#define R_SA_PEG_VC0NPRCA_OFFSET       0x314 ///< PEG VC0 Non-Posted
#define R_SA_PEG_VC0CCL_OFFSET         0x31C ///< PEG Completion Credit Limit
#define R_SA_PEG_VC01CL_OFFSET         0x320 ///< PEG Chaining Limit
#define R_SA_PEG_VC1PRCA_OFFSET        0x32C ///< PEG VC1 Posted
#define R_SA_PEG_VC1NPRCA_OFFSET       0x330 ///< PEG VC1 Non-Posted
#define R_SA_PEG_EQPH3_OFFSET          0x384 ///< PEG Phase 3
#define R_SA_PEG_REUT_PH_CTR_OFFSET    0x444 ///< PEG PHY Control
#define R_SA_PEG_REUT_PH_CTR1_OFFSET   0x448 ///< PEG PHY Control 1
#define R_SA_PEG_REUT_PH1_PIS_OFFSET   0x464 ///< PEG PH1 PIS
#define R_SA_PEG_REUT_OVR_CTL_OFFSET   0x490 ///< PEG REUT Override
#define R_SA_PEG_FUSESCMN_OFFSET       0x504 ///< PEG Fuses
#define R_SA_PEG_AFELN0VMTX2_OFFSET    0x70C ///< PEG AFE Lane VMTX2
#define R_SA_PEG_AFELN0IOCFG0_OFFSET   0x808 ///< PEG AFE Lane IO Config 0
#define R_SA_PEG_AFEBND0CFG0_OFFSET    0x900 ///< PEG AFE Bundle Config 0
#define R_SA_PEG_AFEBND0CFG1_OFFSET    0x904 ///< PEG AFE Bundle Config 1
#define R_SA_PEG_AFEBND0CFG2_OFFSET    0x908 ///< PEG AFE Bundle Config 2
#define R_SA_PEG_AFEBND0CFG3_OFFSET    0x90C ///< PEG AFE Bundle Config 3
#define R_SA_PEG_AFEBND0CFG4_OFFSET    0x910 ///< PEG AFE Bundle Config 4
#define R_SA_PEG_LOADBUSCTL0_OFFSET    0x914 ///< PEG Load Bus Control
#define R_SA_PEG_G3CTL0_OFFSET         0x918 ///< PEG Gen3 Control
#define R_SA_PEG_BND0SPARE_OFFSET      0x91C ///< PEG Bundle 0 Spare Register
#define R_SA_PEG_AFELN0CFG0_OFFSET     0xA00 ///< PEG AFE Lane Config 0
#define R_SA_PEG_AFELN0CFG1_OFFSET     0xA04 ///< PEG AFE Lane Config 1
#define R_SA_PEG_AFELN0CFG2_OFFSET     0xA08 ///< PEG AFE Lane Config 2
#define R_SA_PEG_IMRLE_MMR_OFFSET      0xBE0 ///< PEG IMR Access Enable and Lock Register
#define R_SA_PEG_IMRAMMU32_MMR_OFFSET  0xBE4 ///< PEG IMR Access Memory Mask Upper Portion Register
#define R_SA_PEG_IMRAMBU32_MMR_OFFSET  0xBE8 ///< PEG IMR Access Memory Base Upper Portion Register
#define R_SA_PEG_IMRAMBL_MMR_OFFSET    0xBEC ///< PEG IMR Access Memory Mask, Base Register
#define R_SA_PEG_AFEOVR_OFFSET         0xC20 ///< PEG AFE Override
#define R_SA_PEG_AFE_PWRON_OFFSET      0xC24 ///< PEG AFE Power-on
#define R_SA_PEG_AFE_PM_TMR_OFFSET     0xC28 ///< PEG AFE PM Timer
#define R_SA_PEG_CMNCFG7_OFFSET        0xC30 ///< PEG Common Config 7
#define R_SA_PEG_CMNRXERR_OFFSET       0xC34 ///< PEG Common Error
#define R_SA_PEG_CMNSPARE_OFFSET       0xC38 ///< PEG Common Spare
#define R_SA_PEG_G3PLINIT_OFFSET       0xCD4 ///< PEG Gen3 Physical Init
#define R_SA_PEG_PEGTST_OFFSET         0xD0C ///< PEG TEST
#define R_SA_PEG_DEBUP2_OFFSET         0xD10 ///< PEG Debug
#define R_SA_PEG_DEBUP3_OFFSET         0xD14 ///< PEG Debug
#define R_SA_PEG_PEGCOMLCGCTRL_OFFSET  0xD20 ///< PEG Clock Gating Control
#define R_SA_PEG_FCLKGTTLLL_OFFSET     0xD24 ///< PEG FCLK Clock Gating
#define R_SA_PEG_PEGCLKGTCMN_OFFSET    0xD2C ///< PEG Spine Clock Gating
#define R_SA_PEG_LCTL3_OFFSET          0xD98 ///< PEG Link Control 3
#define R_SA_PEG_LNERRSTAT_OFFSET      0xD9C ///< PEG Lane Error Status
#define R_SA_PEG_EQCTL0_1_OFFSET       0xDA0 ///< PEG Lane Equalization
#define R_SA_PEG_EQPRESET1_2_OFFSET    0xDC0 ///< PEG Coefficients for P1 and P2
#define R_SA_PEG_EQPRESET2_3_4_OFFSET  0xDC4 ///< PEG Coefficients for P2, P3, and P4
#define R_SA_PEG_EQPRESET4_5_OFFSET    0xDC8 ///< PEG Coefficients for P4 and P5
#define R_SA_PEG_EQPRESET6_7_OFFSET    0xDCC ///< PEG Coefficients for P6 and P7
#define R_SA_PEG_EQPRESET7_8_9_OFFSET  0xDD0 ///< PEG Coefficients for P7, P8, and P9
#define R_SA_PEG_EQPRESET9_10_OFFSET   0xDD4 ///< PEG Coefficients for P9 and P10
#define R_SA_PEG_EQCFG_OFFSET          0xDD8 ///< PEG Equalization Config
#define R_SA_PEG_EQPRESET11_OFFSET     0xDDC ///< PEG Coefficients for P11
#define R_SA_PEG_AFEBND0CFG5_OFFSET    0xE04 ///< PEG AFE Bundle Config 5
#define R_SA_PEG_PLUCTLH0_OFFSET       0xE0C ///< PEG Lane Config

#ifdef CPU_CFL
#define R_SA_MSRIO_PEG_AFE_PH3_CFG0_BND_0_OFFSET  0x200 ///< PEG AFE PH3 Bundle Config 0
#define R_SA_MSRIO_PEG_AFE_PH3_CFG1_BND_0_OFFSET  0x210 ///< PEG AFE PH3 Bundle Config 1
#define R_SA_MSRIO_PEG_AFE_PH3_CFG2_BND_0_OFFSET  0x204 ///< PEG AFE PH3 Bundle Config 2

#else
#define R_SA_PEG_BND0_CRI2_FUSE_DWORD14_OFFSET    0x0138 ///< PEG Bundle 0 CRI2 FUSE register DWORD 14
#define R_SA_PEG_BND10_CRI2_FUSE_DWORD14_OFFSET   0x2338 ///< PEG Bundle 10(DMI) CRI2 FUSE register DWORD 14

#define R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET      0x0038 ///< PEG Bundle 0 CRI0 CR register DWORD 14

#define R_SA_PEG_BND0_CRI0_CR_DWORD22_OFFSET      0x0058 ///< PEG Bundle 0 CRI0 CR register DWORD 22
#define R_SA_PEG_BND10_CRI0_CR_DWORD22_OFFSET     0x2258 ///< PEG Bundle 10(DMI) CRI0 CR register DWORD 22

#define R_SA_PEG_BND0_CRI2_FUSE_DWORD15_OFFSET    0x013C ///< PEG Bundle 0 CRI2 FUSE register DWORD 15
#define R_SA_PEG_BND10_CRI2_FUSE_DWORD15_OFFSET   0x233C ///< PEG Bundle 10(DMI) CRI2 FUSE register DWORD 15

#define R_SA_PEG_BND0_CRI0_CR_DWORD24_OFFSET      0x0060 ///< PEG Bundle 0 CRI0 CR register DWORD 24
#define R_SA_PEG_BND10_CRI0_CR_DWORD24_OFFSET     0x2260 ///< PEG Bundle 10(DMI) CRI0 CR register DWORD 24

#define R_SA_PEG_BND0_CRI0_CR_DWORD26_OFFSET      0x0068 ///< PEG Bundle 0 CRI0 CR register DWORD 26
#define R_SA_PEG_BND10_CRI0_CR_DWORD26_OFFSET     0x2268 ///< PEG Bundle 10(DMI) CRI0 CR register DWORD 26

#define R_SA_PEG_BND0_CRI0_CR_DWORD28_OFFSET      0x0070 ///< PEG Bundle 0 CRI0 CR register DWORD 28
#define R_SA_PEG_BND1_CRI0_CR_DWORD28_OFFSET      0x0270 ///< PEG Bundle 1 CRI0 CR register DWORD 28
#define R_SA_PEG_BND2_CRI0_CR_DWORD28_OFFSET      0x0470 ///< PEG Bundle 2 CRI0 CR register DWORD 28
#define R_SA_PEG_BND3_CRI0_CR_DWORD28_OFFSET      0x0670 ///< PEG Bundle 3 CRI0 CR register DWORD 28
#define R_SA_PEG_BND4_CRI0_CR_DWORD28_OFFSET      0x0870 ///< PEG Bundle 4 CRI0 CR register DWORD 28
#define R_SA_PEG_BND5_CRI0_CR_DWORD28_OFFSET      0x0A70 ///< PEG Bundle 5 CRI0 CR register DWORD 28
#define R_SA_PEG_BND6_CRI0_CR_DWORD28_OFFSET      0x0C70 ///< PEG Bundle 6 CRI0 CR register DWORD 28
#define R_SA_PEG_BND7_CRI0_CR_DWORD28_OFFSET      0x0E70 ///< PEG Bundle 7 CRI0 CR register DWORD 28
#define R_SA_PEG_BND8_CRI0_CR_DWORD28_OFFSET      0x1070 ///< PEG Bundle 8 CRI0 CR register DWORD 28
#define R_SA_PEG_BND9_CRI0_CR_DWORD28_OFFSET      0x1270 ///< PEG Bundle 9 CRI0 CR register DWORD 28
#define R_SA_PEG_BND10_CRI0_CR_DWORD28_OFFSET     0x2270 ///< PEG Bundle 10(DMI) CRI0 CR register DWORD 28

#define R_SA_PEG_BND10_CRI2_FUSE_DWORD16_OFFSET   0x2340 ///< PEG Bundle 10(DMI) CRI2 FUSE register DWORD 16
#endif
#endif
