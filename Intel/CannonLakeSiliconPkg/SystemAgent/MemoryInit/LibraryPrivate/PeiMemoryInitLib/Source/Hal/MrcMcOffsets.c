/** @file
  This file contains functions to get Memory Controller (MC) Offsets
  for timings, refresh, scheduler, and anything else MC specific.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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

#include "McAddress.h"
#include "MrcCommon.h"

///
/// Local Functions
///
/**
  This local function returns the CR address for Per Bank Refresh configuration.
  If the Channel parameter is greater than MAX_CHANNEL, the broadcast address is returned.

  @param[in]  Channel - 0 based index defining which channel to address.

  @retval CR Offset.
**/
UINT32
PerBankRefreshOffset (
  IN  UINT32  Channel
  )
{
  UINT32  Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = BC_CR_SC_PBR_CNL_REG;
  } else {
    Offset = CH0_CR_SC_PBR_CNL_REG +
      (CH1_CR_SC_PBR_CNL_REG - CH0_CR_SC_PBR_CNL_REG) * Channel;
  }

  return Offset;
}

///
/// Public Functions
///
/**
  Function used to get the CR Offset for Memory Controller Timings.

  @param[in]  Group     - MC Timing Group being accessed.
  @param[in]  Socket    - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel   - DDR Channel Number within the processor socket (0-based).
  @param[in]  FreqIndex - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
MrcGetMcTimingRegOffset (
  IN  MrcParameters *MrcData,
  IN  GSM_GT  Group,
  IN  UINT32  Socket,
  IN  UINT32  Channel,
  IN  UINT32  FreqIndex
  )
{
  UINT32 Offset;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case GsmMctRP:
    case GsmMctRPabExt:
    case GsmMctRPab:
    case GsmMctRAS:
    case GsmMctRDPRE:
    case GsmMctPPD:
    case GsmMctWRPRE:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_PRE_CNL_REG;
      } else {
        Offset = CH0_CR_TC_PRE_CNL_REG + ((CH1_CR_TC_PRE_CNL_REG - CH0_CR_TC_PRE_CNL_REG) * Channel);
      }
      break;

    case GsmMctRFCpb:
      Offset = PerBankRefreshOffset (Channel);
      break;

    case GsmMctFAW:
    case GsmMctRRDsg:
    case GsmMctRRDdg:
    case GsmMctLpDeratingExt:
    case GsmMctRCDwr:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_ACT_CNL_REG;
      } else {
        Offset = CH0_CR_TC_ACT_CNL_REG + ((CH1_CR_TC_ACT_CNL_REG - CH0_CR_TC_ACT_CNL_REG) * Channel);
      }
      break;

    case GsmMctOdtRdDuration:
    case GsmMctOdtRdDelay:
    case GsmMctWrEarlyOdt:
    case GsmMctOdtWrDuration:
    case GsmMctOdtWrDelay:
    case GsmMctCL:
    case GsmMctCWL:
    case GsmMctAONPD:
    case GsmMctOdtAlwaysRank0:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_ODT_CNL_REG;
      } else {
        Offset = CH0_CR_TC_ODT_CNL_REG + ((CH1_CR_TC_ODT_CNL_REG - CH0_CR_TC_ODT_CNL_REG) * Channel);
      }
      break;

    case GsmMctRDRDsg:
    case GsmMctRDRDdg:
    case GsmMctRDRDdr:
    case GsmMctRDRDdd:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_RDRD_CNL_REG;
      } else {
        Offset = CH0_CR_TC_RDRD_CNL_REG + ((CH1_CR_TC_RDRD_CNL_REG - CH0_CR_TC_RDRD_CNL_REG) * Channel);
      }
      break;

    case GsmMctRDWRsg:
    case GsmMctRDWRdg:
    case GsmMctRDWRdr:
    case GsmMctRDWRdd:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_RDWR_CNL_REG;
      } else {
        Offset = CH0_CR_TC_RDWR_CNL_REG + ((CH1_CR_TC_RDWR_CNL_REG - CH0_CR_TC_RDWR_CNL_REG) * Channel);
      }
      break;

    case GsmMctWRRDsg:
    case GsmMctWRRDdg:
    case GsmMctWRRDdr:
    case GsmMctWRRDdd:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_WRRD_CNL_REG;
      } else {
        Offset = CH0_CR_TC_WRRD_CNL_REG + ((CH1_CR_TC_WRRD_CNL_REG - CH0_CR_TC_WRRD_CNL_REG) * Channel);
      }
      break;

    case GsmMctWRWRsg:
    case GsmMctWRWRdg:
    case GsmMctWRWRdr:
    case GsmMctWRWRdd:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_WRWR_CNL_REG;
      } else {
        Offset = CH0_CR_TC_WRWR_CNL_REG + ((CH1_CR_TC_WRWR_CNL_REG - CH0_CR_TC_WRWR_CNL_REG) * Channel);
      }
      break;

    case GsmMctCKE:
    case GsmMctXP:
    case GsmMctXPDLL:
    case GsmMctPRPDEN:
    case GsmMctRDPDEN:
    case GsmMctWRPDEN:
      if (Channel >= MAX_CHANNEL) {
        // Channel Broadcast
        Offset = BC_CR_TC_PWRDN_CNL_REG;
      } else {
        Offset = CH0_CR_TC_PWRDN_CNL_REG;
        Offset += (CH1_CR_TC_PWRDN_CNL_REG - CH0_CR_TC_PWRDN_CNL_REG) * Channel;
      }
      break;

    case GsmMctXSDLL:
    case GsmMctZQOPER:
    case GsmMctMOD:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_SRFTP_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_TC_SRFTP_CNL_REG, CH1_CR_TC_SRFTP_CNL_REG, Channel);
      }
      break;

    // @todo: Hook LP4/DDR4 tXSabort timing.  Uses the tXSR field in CNL.
    //case GsmMctXSRabort:
    case GsmMctXSR:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_SREXITTP_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_TC_SREXITTP_CNL_REG, CH1_CR_TC_SREXITTP_CNL_REG, Channel);
      }
      break;

    case GsmMctZQCAL:
    case GsmMctZQCS:
    case GsmMctZQCSPeriod:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_ZQCAL_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_TC_ZQCAL_CNL_REG, CH1_CR_TC_ZQCAL_CNL_REG, Channel);
      }
      break;

    case GsmMctCPDED:
    case GsmMctCAL:
    case GsmMctCKCKEH:
    case GsmMctCSCKEH:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_SC_GS_CFG_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_SC_GS_CFG_CNL_REG, CH1_CR_SC_GS_CFG_CNL_REG, Channel);
      }
      break;

    case GsmMctSrIdle:
      Offset = PM_SREF_CONFIG_CNL_REG;
      break;

    case GsmMctREFI:
    case GsmMctRFC:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_RFTP_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_TC_RFTP_CNL_REG, CH1_CR_TC_RFTP_CNL_REG, Channel);
      }
      break;

    case GsmMctOrefRi:
    case GsmMctRefreshHpWm:
    case GsmMctRefreshPanicWm:
    case GsmMctREFIx9:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_RFP_CNL_REG;
      } else {
        Offset = OFFSET_CALC_CH (CH0_CR_TC_RFP_CNL_REG, CH1_CR_TC_RFP_CNL_REG, Channel);
      }
      break;

    default:
      break;
  }

  return Offset;
}

/**
  Function used to get the CR Offset for Memory Controller Configuration Settings.

  @param[in]  Group     - MC Timing Group being accessed.
  @param[in]  Socket    - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel   - DDR Channel Number within the processor socket (0-based).
  @param[in]  FreqIndex - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
MrcGetMcConfigRegOffset (
  IN  MrcParameters *MrcData,
  IN  GSM_GT  Group,
  IN  UINT32  Socket,
  IN  UINT32  Channel,
  IN  UINT32  FreqIndex
  )
{
  UINT32 Offset;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case GsmMccLp4FifoRdWr:
    case GsmMccIgnoreCke:
    case GsmMccMaskCs:
    case GsmMccCmdMirrorEn:
    case GsmMccCpgcInOrder:
    case GsmMccLpddr2nCsMrw:
    case GsmMccResetOnCmd:
    case GsmMccResetDelay:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_SC_GS_CFG_TRAINING_CNL_REG;
      } else {
        Offset = CH0_CR_SC_GS_CFG_TRAINING_CNL_REG +
          (CH1_CR_SC_GS_CFG_TRAINING_CNL_REG - CH0_CR_SC_GS_CFG_TRAINING_CNL_REG) * Channel;
      }
      break;

    case GsmMccDramType:
    case GsmMccCmdStretch:
    case GsmMccCmdGapRatio:
    case GsmMccAddrMirror:
    case GsmMccCmdTriStateDis:
    case GsmMccFreqPoint:
    case GsmMccEnableOdtMatrix:
    case GsmMccX8Device:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_SC_GS_CFG_CNL_REG;
      } else {
        Offset = CH0_CR_SC_GS_CFG_CNL_REG;
        Offset += (CH1_CR_SC_GS_CFG_CNL_REG - CH0_CR_SC_GS_CFG_CNL_REG) * Channel;
      }
      break;

    case GsmMccAddrDecodeDdrType:
    case GsmMccEnhChannelMode:
    case GsmMccStackedMode:
    case GsmMccStackChMap:
    case GsmMccLChannelMap:
    case GsmMccSChannelSize:
    case GsmMccStackedChHash:
      Offset = MAD_INTER_CHANNEL_CNL_REG;
      break;

    case GsmMccLDimmMap:
    case GsmMccRankInterleave:
    case GsmMccEnhancedInterleave:
    case GsmMccEccMode:
    case GsmMccHORI:
    case GsmMccHORIAddress:
      // No Broadcast supported for this.
      if (Channel < MAX_CHANNEL) {
        Offset = MAD_INTRA_CH0_CNL_REG +
          ((MAD_INTRA_CH1_CNL_REG - MAD_INTRA_CH0_CNL_REG) * Channel);
      }
      break;

    case GsmMccLDimmSize:
    case GsmMccLDimmDramWidth:
    case GsmMccLDimmRankCnt:
    case GsmMccLDimm8Gb:
    case GsmMccSDimmSize:
    case GsmMccSDimmDramWidth:
    case GsmMccSDimmRankCnt:
    case GsmMccSDimm8Gb:
      // No Broadcast supported for this.
      if (Channel < MAX_CHANNEL) {
        Offset = MAD_DIMM_CH0_CNL_REG +
          ((MAD_DIMM_CH1_CNL_REG - MAD_DIMM_CH0_CNL_REG) * Channel);
      }
      break;

    case GsmMccPuMrcDone:
    case GsmMccSaveFreqPoint:
    case GsmMccEnableRefresh:
    case GsmMccMcInitDoneAck:
    case GsmMccMrcDone:
    case GsmMccEnableDclk:
    case GsmMccPureSrx:
    case GsmMccDdrReset:
    case GsmMccSafeSr:
      Offset = MC_INIT_STATE_G_CNL_REG;
      break;

    case GsmMccEnableSr:
      Offset = PM_SREF_CONFIG_CNL_REG;
      break;

    case GsmMccRankOccupancy:
    case GsmMccMcSrx:
      Offset = CH0_CR_MC_INIT_STATE_CNL_REG +
        ((CH1_CR_MC_INIT_STATE_CNL_REG - CH0_CR_MC_INIT_STATE_CNL_REG) * Channel);
      break;

    case GsmMccRefInterval:
    case GsmMccRefStaggerEn:
    case GsmMccRefStaggerMode:
    case GsmMccDisableStolenRefresh:
    case GsmMccEnRefTypeDisplay:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_MC_REFRESH_STAGGER_CNL_REG;
      } else {
        Offset = CH0_CR_MC_REFRESH_STAGGER_CNL_REG +
          (CH1_CR_MC_REFRESH_STAGGER_CNL_REG - CH0_CR_MC_REFRESH_STAGGER_CNL_REG) * Channel;
      }
      break;

    case GsmMccHashMask:
    case GsmMccLsbMaskBit:
    case GsmMccHashMode:
      Offset = CHANNEL_HASH_CNL_REG;
      break;

    case GsmMccMr2Shadow:
    case GsmMccSrtAvail:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_TC_MR2_SHADDOW_CNL_REG;
      } else {
        Offset = CH0_CR_TC_MR2_SHADDOW_CNL_REG +
          (CH1_CR_TC_MR2_SHADDOW_CNL_REG - CH0_CR_TC_MR2_SHADDOW_CNL_REG) * Channel;
      }
      break;

    case GsmMccDisableCkTristate:
      if (Channel >= MAX_CHANNEL) {
        Offset = BC_CR_SCHED_SECOND_CBIT_CNL_REG;
      } else {
        Offset = CH0_CR_SCHED_SECOND_CBIT_CNL_REG +
          (CH1_CR_SCHED_SECOND_CBIT_CNL_REG - CH0_CR_SCHED_SECOND_CBIT_CNL_REG) *Channel;
      }
      break;

    case GsmMccPbrDis:
      Offset = PerBankRefreshOffset (Channel);
      break;

    default:
      break;
  }

  return Offset;
}

/**
  This function returns the offset for the MRS FSM Control register.

  @param[in]  MrcData - Pointer to global MRC data.

  @retval Register Offset.
**/
UINT32
MrcGetMrsFsmCtlOffset (
  IN  MrcParameters *MrcData,
  IN  UINT32        Channel
  )
{
  UINT32 Offset;

  Offset = CH0_CR_MRS_FSM_CONTROL_CNL_REG;
  Offset += (CH1_CR_MRS_FSM_CONTROL_CNL_REG - CH0_CR_MRS_FSM_CONTROL_CNL_REG) * Channel;

  return Offset;
}

/**
  This function returns the offset for MRS run FSM Control register.

  @param[in]  MrcData - Pointer to global MRC data.

  @retval Register Offset.
**/
UINT32
MrcGetMrsRunFsmOffset (
  IN  MrcParameters *MrcData,
  IN  UINT32        Channel
  )
{
  UINT32 Offset;

  Offset = CH0_CR_MRS_FSM_RUN_CNL_REG +
      ((CH1_CR_MRS_FSM_RUN_CNL_REG - CH0_CR_MRS_FSM_RUN_CNL_REG) * Channel);

  return Offset;
}
