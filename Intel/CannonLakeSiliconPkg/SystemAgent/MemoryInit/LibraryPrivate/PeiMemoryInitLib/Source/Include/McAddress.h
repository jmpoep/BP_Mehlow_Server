/** @file
  The contents of this file has all the memory controller register addresses
  and register bit fields for the MRC.

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
#ifndef __McAddress_h__
#define __McAddress_h__

#define MRC_IGNORE_ARG_8      (0xFF)
#define MRC_IGNORE_ARG_16     (0xFFFF)
#define MRC_IGNORE_ARG        (0xDEADBEEF)
#define MRC_CHANNEL_MULTICAST (0xFFFFFFFF)
#define MRC_BYTE_MULTICAST    (0xFFFFFFFF)
#define SOCKET_0              (0)
#define CONTROLLER_0          (0)

#include "MrcTypes.h"
#include "Pci000Cnl.h"
#include "MrcMcRegisterCnl.h"
#include "MrcMcRegisterStructCnl.h"
#include "MrcHalRegisterAccess.h"


///
/// The following is a copy of M_PCU_CR_SSKPD_PCU_STRUCT, modified to add in the
/// definition of the scratch pad bit fields.
///
typedef union {
  struct {
    UINT64 MrcDone    : 1;  ///< Bits 0:0
    UINT64 Reserved   : 63; ///< Bits 63:1
  } Bits;
  UINT64 Data;
  UINT32 Data32[2];
  UINT16 Data16[4];
  UINT8  Data8[8];
} M_PCU_CR_SSKPD_PCU_STRUCT;

#define PCU_CR_SSKPD_PCU_MRCDONE_OFF  (0)
#define PCU_CR_SSKPD_PCU_MRCDONE_WID  (1)
#define PCU_CR_SSKPD_PCU_MRCDONE_MSK  (0x1)
#define PCU_CR_SSKPD_PCU_MRCDONE_MAX  (0x1)
#define PCU_CR_SSKPD_PCU_MRCDONE_DEF  (0x1)

#define PCIE_CR_OPIO_RX_DLL_GLOBAL3_DMIBAR_REG  (0xB1C)

typedef union {
  struct {
    UINT32 Reserved           : 30; ///< Bits 29:0
    UINT32 select_vccio_level :  2; ///< Bits 31:30  0 = 0.85v, 1 = 0.95v
  } Bits;
  UINT32 Data;
  UINT16 Data16[2];
  UINT8  Data8[4];
} PCIE_CR_OPIO_RX_DLL_GLOBAL3_STRUCT;

//
// CNL CMI_CMF_GLOBAL_CFG in MCHBAR
//
#define CMI_CMF_GLOBAL_CFG_REG     (0x6420)

typedef union {
  struct {
    UINT32 CREDITS_CONFIG_DONE  : 1; ///< Bits 0:0
    UINT32 RESERVED_0           : 7; ///< Bits 7:1
    UINT32 ISM_IDLE_TIME        : 8; ///< Bits 15:8
    UINT32 AGENT_WR_RSP         : 4; ///< Bits 19:16
    UINT32 FIXED_WINDOW         : 1; ///< Bits 20:20
    UINT32 RESERVED_1           : 3; ///< Bits 23:21
    UINT32 CLK_GATE_EN          : 1; ///< Bits 24:24
    UINT32 FORCE_ISM_ACTIVE     : 1; ///< Bits 25:25
    UINT32 BYPASS_EN            : 1; ///< Bits 26:26
    UINT32 REQ_FAST_WAKE        : 4; ///< Bits 30:27
    UINT32 FAST_WAKE_EN         : 1; ///< Bits 31:31
  } Bits;
  UINT32 Data;
  UINT16 Data16[2];
  UINT8  Data8[4];
} CMI_CMF_GLOBAL_CFG_STRUCT;

#define MAX_NUMBER_WDB_CACHELINES                           (CH0_CR_WDB_RD_WR_DFX_CTL_CNL_WID_MAX + 1)
#define MAX_WDB_INC_RATE_EXPONENTIAL_VAL                    (42)
#define MAX_WDB_INC_RATE_LINEAR_VAL                         (CH0_CR_REUT_CH_PAT_WDB_CL_CTRL_CNL_WDB_Increment_Rate_MAX + 1)
#define MAX_NUMBER_PAT_GENS                                 (REUT_PG_ID_PATTERN_BUFFER_CTL_CNL_Pattern_Generator_Id_WID)
#define MAX_NUMBER_PAT_GEN_CHUNKS                           (REUT_PG_ID_PATTERN_BUFFER_CTL_CNL_Pattern_Buffer_Entry_MAX + 1)
#define MAX_DQDB_PAT_GEN_ROTATE_RATE_VAL                    (REUT_PG_PAT_CL_COUNTER_CFG_CNL_Cl_Counter_For_Shift_MAX)
#define MAX_CADB_PAT_GEN_ROTATE_RATE_VAL                    (REUT_PG_ID_PATTERN_BUFFER_CTL_CNL_SubSequence_Count_To_Shift_MAX)
#define MAX_NUMBER_PAT_GEN_UNISEQS                          (3)
#define MAX_PAT_GEN_UNISEQ_SEED_VAL                         (REUT_PG_PAT_CL_MUX_WR_PB_0_CNL_Pattern_Buffer_MAX)
#define MAX_PAT_GEN_UNISEQ_L_VAL                            (REUT_PG_PAT_CL_MUX_LMN_CNL_L_counter_MAX)
#define MAX_PAT_GEN_UNISEQ_M_VAL                            (REUT_PG_PAT_CL_MUX_LMN_CNL_M_counter_MAX)
#define MAX_PAT_GEN_UNISEQ_N_VAL                            (REUT_PG_PAT_CL_MUX_LMN_CNL_N_counter_MAX)
#define MAX_PAT_GEN_UNISEQ_SEED_RELOAD_RATE_VAL             (REUT_PG_PAT_CL_MUX_CFG_CNL_Reload_LFSR_Seed_Rate_MAX)
#define MAX_PAT_GEN_UNISEQ_SEED_SAVE_RATE_VAL               (REUT_PG_PAT_CL_MUX_CFG_CNL_Save_LFSR_Seed_Rate_MAX)
#define MAX_PAT_GEN_INV_DC_ROTATE_RATE_EXPONENTIAL_VAL      (REUT_PG_PAT_INV_CNL_Inv_or_DC_Shift_Rate_MAX)
#define MAX_START_DELAY_VAL                                 (REUT_CH_SEQ_CFG_0_CNL_Start_Test_Delay_MAX)
#define IS_LOOP_COUNT_EXPONENTIAL                           (FALSE)
#define MAX_LOOP_COUNT_VAL                                  (REUT_CH_SEQ_LOOPCOUNT_STATUS_0_CNL_Current_Loopcount_MAX)
#define MAX_NUMBER_SUBSEQS                                  (REUT_CH_SEQ_SUBSEQ_PNTR_0_CNL_Current_Subsequence_Pointer_MAX + 1)
#define MAX_BURST_LENGTH_EXPONENT_VAL                       (15)
#define MAX_BURST_LENGTH_LINEAR_VAL                         (REUT_CH0_SUBSEQ_CTL_0_CNL_Number_of_Cachelines_MAX)
#define MAX_INTER_SUBSEQ_WAIT_VAL                           (214)
#define MAX_OFFSET_ADDR_UPDATE_RATE_VAL                     (REUT_CH0_SUBSEQ_OFFSET_CTL_0_CNL_Offset_Address_Update_Rate_MAX)
#define MAX_ADDR_INVERT_RATE_VAL                            (REUT_CH_SEQ_BASE_ADDR_ORDER_CARRY_INVERT_CTL_0_CNL_Base_Address_Invert_Rate_MAX)
#define MAX_RANK_ADDR_INC_RATE_EXPONENT_VAL                 (223)
#define MAX_RANK_ADDR_INC_RATE_LINEAR_VAL                   (REUT_CH_SEQ_BASE_ADDR_INC_CTL_0_CNL_Rank_Base_Address_Update_Rate_MAX)
#define MIN_RANK_ADDR_INC_VAL                               (-4)
#define MAX_RANK_ADDR_INC_VAL                               (3)
#define MAX_BANK_ADDR_INC_RATE_EXPONENT_VAL                 (223)
#define MAX_BANK_ADDR_INC_RATE_LINEAR_VAL                   (REUT_CH_SEQ_BASE_ADDR_INC_CTL_0_CNL_Bank_Base_Address_Update_Rate_MAX)
#define MIN_BANK_ADDR_INC_VAL                               (-8)
#define MAX_BANK_ADDR_INC_VAL                               (7)
#define MAX_ROW_ADDR_INC_RATE_EXPONENT_VAL                  (210)
#define MAX_ROW_ADDR_INC_RATE_LINEAR_VAL                    (REUT_CH_SEQ_BASE_ADDR_INC_CTL_0_CNL_Row_Base_Address_Update_Rate_MAX)
#define MIN_ROW_ADDR_INC_VAL                                (-2048)
#define MAX_ROW_ADDR_INC_VAL                                (2047)
#define MAX_COL_ADDR_INC_RATE_EXPONENT_VAL                  (220)
#define MAX_COL_ADDR_INC_RATE_LINEAR_VAL                    (REUT_CH_SEQ_BASE_ADDR_INC_CTL_0_CNL_Column_Base_Address_Update_Rate_MAX)
#define MIN_COL_ADDR_INC_VAL                                (-256)
#define MAX_COL_ADDR_INC_VAL                                (255)
#define MAX_NUMBER_RANK_MAP_ENTRIES                         (REUT_CH_SEQ_BASE_ADDR_START_0_CNL_Rank_Address_MAX + 1)
#define MAX_NUMBER_BANK_MAP_ENTRIES                         (REUT_CH_SEQ_BASE_ADDR_START_0_CNL_Bank_Address_MAX + 1)
#define MAX_NUMBER_ROW_ADDR_SWIZZLE_ENTRIES                 (17)
#define MAX_ROW_ADDR_SWIZZLE_VAL                            (16)
#define MAX_VAL_CACHELINES                                  (REUT_CH_ERR_CTL_0_CNL_Selective_Error_Enable_Cacheline_WID)
#define MAX_STOP_ON_NTH_ERROR_COUNT_VAL                     (REUT_CH_ERR_CTL_0_CNL_Stop_on_Nth_Error_MAX)
#define MAX_NUMBER_ERROR_COUNTERS                           (18)
#define MAX_SCRAMBLER_KEY_VAL                               (DDRSCRAM_CR_DDRSCRAMBLECH0_CNL_ScramKey_MAX)
#define MIN_REFRESH_IDLE_TIMER_VAL                          (512)
#define MAX_REFRESH_IDLE_TIMER_VAL                          (PM_SREF_CONFIG_CNL_Idle_timer_MAX)
#define COMMAND_VICTIM_SPREAD                               (7)

#endif // __McAddress_h__
