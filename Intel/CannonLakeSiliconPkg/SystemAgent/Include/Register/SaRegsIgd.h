/** @file
  Register names for IGD block
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
  Copyright 2016 - 2018 Intel Corporation.

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
#ifndef _SA_REGS_IGD_H_
#define _SA_REGS_IGD_H_

///
/// Device 2 Register Equates
///
//
// The following equates must be reviewed and revised when the specification is ready.
//
#define SA_IGD_BUS           0x00
#define SA_IGD_DEV           0x02
#define SA_IGD_FUN_0         0x00
#define SA_IGD_DEV_FUN       (SA_IGD_DEV << 3)
#define SA_IGD_BUS_DEV_FUN   (SA_MC_BUS << 8) + SA_IGD_DEV_FUN

#define V_SA_IGD_VID         0x8086
#define SA_GT_APERTURE_SIZE_256MB    1      ///< 256MB is the recommanded GT Aperture Size as per BWG.

#ifndef CPU_CFL
#define V_SA_PCI_DEV_2_GT2_ULX_1_ID  0x5A51 ///< Dev2 CNL Y GT2 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULX_2_ID  0x5A59 ///< Dev2 CNL Y GT1.5 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULX_3_ID  0x5A41 ///< Dev2 CNL Y GT1 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULX_4_ID  0x5A49 ///< Dev2 CNL Y GT0.5 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULT_1_ID  0x5A52 ///< Dev2 CNL U GT2 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULT_2_ID  0x5A5A ///< Dev2 CNL U GT1.5 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULT_3_ID  0x5A42 ///< Dev2 CNL U GT1 (2+2)
#define V_SA_PCI_DEV_2_GT2_ULT_4_ID  0x5A4A ///< Dev2 CNL U GT0.5 (2+2)
#define V_SA_PCI_DEV_2_GT3_ULT_ID    0x5A62 ///< Dev2 CNL U GT3 (4+3e)
#define V_SA_PCI_DEV_2_GT2_HALO_1_ID 0x5A54 ///< Dev2 CNL H GT2 (6+2)
#define V_SA_PCI_DEV_2_GT2_HALO_2_ID 0x5A44 ///< Dev2 CNL H GT1 (6+2)
#define V_SA_PCI_DEV_2_GT2_HALO_3_ID 0x5A56 ///< Dev2 CNL H GT2 (8+2)
#define V_SA_PCI_DEV_2_GT2_HALO_4_ID 0x5A5C ///< Dev2 CNL H GT1.5 (8+2)
#define V_SA_PCI_DEV_2_GT2_DT_1_ID   0x5A55 ///< Dev2 CNL S GT2 (6+2)
#define V_SA_PCI_DEV_2_GT2_DT_2_ID   0x5A45 ///< Dev2 CNL S GT1 (4+1)
#else  // CPU_CFL
///
///
/// For KBL IGD
///
#define V_SA_PCI_DEV_2_GT3_KULTM_ID    0x05926 ///< Dev2-KBL ULT GT3 (3+3/3E) Mobile
#define V_SA_PCI_DEV_2_GT2_KHALM_ID    0x0591B ///< Dev2-KBL Halo GT2 (4/2+2)
#define V_SA_PCI_DEV_2_GT2_KDT_ID      0x05912 ///< Dev2-KBL GT2 (4/2+2) Desktop
#define V_SA_PCI_DEV_2_GT4_KWKS_ID     0x0593D ///< Dev2-KBL GT4 (4+4E) WorkStation
///
/// For CFL IGD
///
#define V_SA_PCI_DEV_2_GT3_CFL_ULT_1_ID   0x3EA5 ///< Dev2 CFL-U GT3
#define V_SA_PCI_DEV_2_GT3_CFL_ULT_2_ID   0x3EA6 ///< Dev2 CFL-U GT3
#define V_SA_PCI_DEV_2_GT3_CFL_ULT_3_ID   0x3EA7 ///< Dev2 CFL-U GT3
#define V_SA_PCI_DEV_2_GT3_CFL_ULT_4_ID   0x3EA2 ///< Dev2 CFL-U GT3
#define V_SA_PCI_DEV_2_GT2_CFL_ULT_1_ID   0x3EA0 ///< Dev2 CFL-U GT2
#define V_SA_PCI_DEV_2_GT2_CFL_ULT_2_ID   0x3EA3 ///< Dev2 CFL-U GT2
#define V_SA_PCI_DEV_2_GT2_CFL_ULT_3_ID   0x3EA9 ///< Dev2 CFL-U GT2
#define V_SA_PCI_DEV_2_GT1_CFL_ULT_1_ID   0x3EA1 ///< Dev2 CFL-U GT1
#define V_SA_PCI_DEV_2_GT1_CFL_ULT_2_ID   0x3EA4 ///< Dev2 CFL-U GT1
#define V_SA_PCI_DEV_2_GT1_CFL_DT_1_ID    0x3E90 ///< Dev2-CFL GT1 Desktop
#define V_SA_PCI_DEV_2_GT1_CFL_DT_2_ID    0x3E93 ///< Dev2-CFL GT1 Desktop
#define V_SA_PCI_DEV_2_GT1_CFL_DT_3_ID    0x3E99 ///< Dev2 CFL GT1 Desktop
#define V_SA_PCI_DEV_2_GT2_CFL_DT_1_ID    0x3E92 ///< Dev2-CFL GT2 Desktop
#define V_SA_PCI_DEV_2_GT2_CFL_DT_2_ID    0x3E91 ///< Dev2-CFL GT2 Desktop
#define V_SA_PCI_DEV_2_GT2_CFL_DT_3_ID    0x3E98 ///< Dev2 CFL GT2 Desktop
#define V_SA_PCI_DEV_2_GT2_CFL_HALO_1_ID  0x3E9B ///< Dev2 CFL-H GT2
#define V_SA_PCI_DEV_2_GT1_CFL_HALO_1_ID  0x3E9C ///< Dev2 CFL-H GT1
#define V_SA_PCI_DEV_2_GT2_CFL_HALO_2_ID  0x3E94 ///< Dev2 CFL-H Xeon GT2
#define V_SA_PCI_DEV_2_GT2_CFL_WS_ID_1_ID 0x3E96 ///< Dev2 CFL-WS GT2
#define V_SA_PCI_DEV_2_GT2_CFL_WS_ID_2_ID 0x3E9A ///< Dev2 CFL-WS GT2

#endif  // CPU_CFL

#define R_SA_IGD_VID               0x00
#define R_SA_IGD_DID               0x02
#define R_SA_IGD_CMD               0x04
///
/// GTTMMADR aligned to 16MB (Base address = [38:24])
///
#define R_SA_IGD_GTTMMADR          0x10
#define R_SA_IGD_GMADR             0x18
#define R_SA_IGD_IOBAR             0x20

#define R_SA_IGD_BSM_OFFSET        0x005C  ///< Base of Stolen Memory
#define R_SA_IGD_MSAC_OFFSET       0x0062  ///< Multisize Aperture Control
#define R_SA_IGD_MSICAPID_OFFSET   0x0090  ///< MSI Capabilities ID
#define R_SA_IGD_MC_OFFSET         0x0092  ///< Message Control
#define R_SA_IGD_MA_OFFSET         0x0094  ///< Message Address
#define R_SA_IGD_MD_OFFSET         0x0098  ///< Message Data
#define R_SA_IGD_SWSCI_OFFSET      0x00E8
#define R_SA_IGD_ASLS_OFFSET       0x00FC  ///< ASL Storage

#define R_SA_IGD_AUD_FREQ_CNTRL_OFFSET 0x65900 ///< iDisplay Audio BCLK Frequency Control
#define B_SA_IGD_AUD_FREQ_CNTRL_TMODE     BIT15   ///< T-Mode: 0b - 2T, 1b - 1T
#define B_SA_IGD_AUD_FREQ_CNTRL_96MHZ  BIT4    ///< 96 MHz BCLK
#define B_SA_IGD_AUD_FREQ_CNTRL_48MHZ  BIT3    ///< 48 MHz BCLK

#define R_SA_GTTMMADR_PP_STATUS         0xC7200
#define R_SA_GTTMMADR_PP_CONTROL        0xC7204

#ifdef CPU_CFL
#define R_SA_GTTMMADR_GTDOORBELL_OFFSET 0x124828  ///< iTouch GT Doorbell Register
#define B_SA_GTTMMADR_GTDOORBELL_LOCK   BIT40     ///< 1b - locks BDF
#define R_SA_GTTMMADR_GSA_TOUCH_OFFSET  0x1300B4  ///< iTouch GSA Touch Register
#define B_SA_GTTMMADR_GSA_TOUCH_LOCK    BIT0      ///< 1b - locks BDF
#endif

#define GSA_TOUCH_BUS   0x0   ///< Bus 0
#define GSA_TOUCH_DEV   0x16  ///< Device 22
#define GSA_TOUCH_FUN   0x4   ///< Functional 4

#ifndef CPU_CFL
#define R_SA_GTTMMADR_GTDOORBELL_OFFSET 0x10c008  ///< iTouch GT Doorbell BDF Register
#define R_SA_GTTMMADR_GSA_TOUCH_OFFSET  0x101078  ///< GSA Touch DBF Register
#define R_SA_GTTMMADR_GSA_AUDIO_OFFSET  0x101074  ///< GSA Audio Register
#define GSA_AUDIO_BUS   0x0   ///< Bus 0
#define GSA_AUDIO_DEV   0x1F  ///< Device 31
#define GSA_AUDIO_FUN   0x3   ///< Function 3
#endif //CPU_CFL

///
/// GT MMIO Registers for CD Clock Programming
///
#define R_SA_GTTMMADR_FUSE_STATUS_OFFSET           0x42000  ///< FUSE_STATUS
#define B_SA_GTTMMADR_FUSE_STATUS_PG0_DIST_STATUS  BIT27    ///< PG0 Fuse Status
#define B_SA_GTTMMADR_FUSE_STATUS_PG1_DIST_STATUS  BIT26    ///< PG1 Fuse Status
#define B_SA_GTTMMADR_FUSE_STATUS_PG2_DIST_STATUS  BIT25    ///< PG2 Fuse Status
#define R_SA_GTTMMADR_PWR_WELL_CTL_OFFSET          0x45400  ///< PWR_WELL_CTL
#define R_SA_GTTMMADR_PWR_WELL_CTL_PG_1_STATE      BIT28    ///< PG1 State
#define R_SA_GTTMMADR_PWR_WELL_CTL_PG_1_ENABLE     BIT29    ///< PG1 Enable
#define R_SA_GTTMMADR_PWR_WELL_CTL_PG_2_STATE      BIT30    ///< PG2 State
#define R_SA_GTTMMADR_PWR_WELL_CTL_PG_2_ENABLE     BIT31    ///< PG2 Enable
#define R_SA_GTTMMADR_DBUF_CTL_OFFSET            0x45008   ///< DBUF_CTL
#define R_SA_GTTMMADR_CDCLK_CTL_OFFSET           0x46000   ///< CDCLK_CTL
#ifndef CPU_CFL
#define R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET    0x46070   ///< CDCLK_PLL_ENABLE
#else //CPU_CFL
#define R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET    0x46010   ///< CDCLK_PLL_ENABLE
#define R_SA_GTTMMADR_DPLL_CTRL_OFFSET           0x6C058   ///< DPLL_CTRL1
#endif //CPU_CFL
#define R_SA_GTTMMADR_NDE_RSTWRN_OPT_OFFSET      0x46408   ///< NDE_RSTWRN_OPT
#ifndef CPU_CFL
#define R_SA_GTTMMADR_AUDIO_PIN_BUF_CTL_OFFSET   0x48414   ///< AUDIO_PIN_BUF_CTL
#define B_SA_GTTMMADR_AUDIO_PIN_BUF_CTL_ENABLE   BIT31
#define R_SA_GTTMMADR_DSSM_OFFSET                0x51004   ///< DSSM
#endif //CPU_CFL
#define R_SA_GTTMMADR_MAILBOX_INTERFACE_OFFSET   0x138124  ///< GTDRIVER_MAILBOX_INTERFACE
#define R_SA_GTTMMADR_MAILBOX_DATA_LOW_OFFSET    0x138128  ///< GTDRIVER_MAILBOX_DATA0
#define R_SA_GTTMMADR_MAILBOX_DATA_HIGH_OFFSET   0x13812c  ///< GTDRIVER_MAILBOX_DATA1
#define B_SA_CDCLK_PLL_ENABLE_BIT                  BIT31
#define B_SA_CDCLK_PLL_LOCK_BIT                    BIT30
#ifdef CPU_CFL
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_450       0x382
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_540       0x436
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_337_5     0x2A1
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_675       0x544
#define V_SA_CDCLK_CTL_CD_FREQ_SELECT_0          0x0
#define V_SA_CDCLK_CTL_CD_FREQ_SELECT_1          BIT26
#define V_SA_CDCLK_CTL_CD_FREQ_SELECT_2          BIT27
#define V_SA_CDCLK_CTL_CD_FREQ_SELECT_3          BIT27 | BIT26
#define B_SA_GT_CD_CLK_FREQ_MASK                 0xF3FFF800
#else
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_168       0x14E
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_336       0x29E
#define V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_528       0x41E
#define V_SA_CDCLK_CTL_CD2X_DIVIDE_BY_1          0x0
#define V_SA_CDCLK_CTL_CD2X_DIVIDE_BY_2          BIT23
#define V_SA_CDCLK_PLL_RATIO_REF_CLK_19_2MHZ     0x23
#define V_SA_CDCLK_PLL_RATIO_REF_CLK_24_MHZ      0x1c
#define B_SA_GT_CD_CLK_FREQ_MASK                 0xFF3FF800
#define B_SA_GTTMMADR_CDCLK_PLL_RATIO_MASK       0xFFFFFF00
#endif //CPU_CFL
#define V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_0    0x00000000
#define V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_1    0x00000001
#define V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_2    0x00000002
#define V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_3    0x00000003

#define R_SA_GTTMMADR_PSMIBASE_OFFSET            0xBF0     ///< PSMI Base
#define R_SA_GTTMMADR_PSMILIMIT_OFFSET           0xBF8     ///< PSMI Limit
#define B_SA_PAVPC_PAVPC_MASK                    (0xfff00000)
#define B_SA_PAVPC_SIZE_MASK                     (0x180)
#define B_SA_PSMI_LOCK_MASK                      (0x1)
#define B_SA_PSMIBASE_LSB_MASK                   (0xfffff000)
#define B_SA_PSMILIMIT_LSB_MASK                  (0xffffffc0)
#define B_SA_PSMI_MSB_MASK                       (0x3f)

#ifdef CPU_CFL
#define B_SA_GT_SLICE_0_ENABLE_BIT                BIT25
#define B_SA_GT_SUB_SLICE_0_ENABLE_BIT            BIT20
#else
#define B_SA_GT_SLICE_0_ENABLE_BIT                BIT22
#define B_SA_GT_SLICE_1_ENABLE_BIT                BIT23
#define B_SA_GT_SUB_SLICE_0_ENABLE_BIT            BIT18
#endif //CPU_CFL

#endif
