/** @file
  This file contains the memory scrubbing and alias checking functions.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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

#include "MrcTypes.h"
#include "MrcApi.h"
#include "MrcMemoryScrub.h"
#include "MrcCpgcApi.h"
#include "MrcSpdProcessing.h"
#include "Cpgc15Patterns.h"

/**
  This function sets all the memory to a known value when ECC is enabled and
  either we are not in warm boot or we are in warm boot and TXT is set (which is forced to fast cold).

  @param[in] MrcData - Include all MRC global data.

  @retval mrcSuccess if the clean succeeded, otherwise an error status.
**/
MrcStatus
MrcEccClean (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput        *Inputs;
  MrcDebug              *Debug;
  const MRC_FUNCTION    *MrcCall;
  static const UINT8    WrapCarryEn[MrcReutFieldMax]   = {0, 0, 0, 0};
  static const UINT8    WrapTriggerEn[MrcReutFieldMax] = {0, 0, 0, 0};
  static const UINT8    AddrInvertEn[MrcReutFieldMax]  = {0, 0, 0, 0};
  static const UINT32   DataStaticSeeds[MRC_NUM_MUX_SEEDS] = {0, 0, 0};
  MrcControllerOut      *ControllerOut;
  MrcDimmOut            *DimmOut;
  MrcOutput             *Outputs;
  MrcTiming             *Timing;
  MrcStatus             Status;
  MRC_REUTAddress       ReutAddress;
  INT64                 GetSetVal;
  UINT32                Offset;
  UINT32                DdrMrParamsSave[MAX_CHANNEL];
  UINT32                tREFI;
  UINT8                 Rank;
  UINT8                 BankCount;
  UINT8                 Channel;
  UINT8                 ActiveChBitMask;
  UINT8                 RankToDimm;
  UINT8                 Chunk;
  BOOLEAN               EccEnabled;
  MRC_PATTERN_CTL       WDBPattern;
  UINT64_STRUCT         Patterns[MRC_CPGC_MAX_CHUNKS];
  CH0_CR_DDR_MR_PARAMS_CNL_STRUCT             DdrMrParams;
  REUT_CH0_SUBSEQ_CTL_0_CNL_STRUCT            ReutSubSeqCtl;
  REUT_CH_SEQ_CFG_0_CNL_STRUCT                ReutChSeqCfg;
  CH0_CR_SC_PCIT_CNL_STRUCT                   ScPcit;
  CH0_CR_SC_PCIT_CNL_STRUCT                   ScPcitSave[MAX_CHANNEL];
  REUT_PG_PAT_CL_MUX_CFG_CNL_STRUCT           ReutPgPatMuxCfg;
  MAD_INTRA_CH0_CNL_STRUCT                    MadIntra;
  MAD_INTRA_CH0_CNL_STRUCT                    MadIntraSave[MAX_CHANNEL];
  REUT_PG_ID_PATTERN_BUFFER_CTL_CNL_STRUCT    PatBufCtl;

#ifdef MRC_DEBUG_PRINT
  static const char *SourceStr[3] = {"Ecc", "CleanMemory"};
#endif

  Inputs        = &MrcData->Inputs;
  MrcCall       = Inputs->Call.Func;
  Outputs       = &MrcData->Outputs;
  ControllerOut = &Outputs->Controller[0];
  Debug         = &Outputs->Debug;
  Status        = mrcSuccess;
  EccEnabled    = (Outputs->EccSupport == TRUE);
  MrcCall->MrcSetMem ((UINT8 *) &ReutAddress, sizeof (ReutAddress), 0);

  if (EccEnabled || (Inputs->CleanMemory == TRUE)) {
    // Enable refreshes on MC before we start ECC scrubbing.
    GetSetVal = 1;
    MrcGetSetDdrIoGroupController0 (MrcData, GsmMccEnableRefresh, WriteNoCache, &GetSetVal);

    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Scrubbing Memory due to %s\n", (EccEnabled) ? SourceStr[0] : SourceStr[1]);

    // Setup Reut for both channels. Reut test will write 0x0 to all memory
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (!MrcChannelExist (Outputs, Channel)) {
        continue;
      }
      // Set Reut sequence cfg register
      ReutChSeqCfg.Data                                    = 0;
      ReutChSeqCfg.Bits.Initialization_Mode                = REUT_Testing_Mode;
      ReutChSeqCfg.Bits.Global_Control                     = 1;
      ReutChSeqCfg.Bits.Start_Test_Delay                   = 2;
      ReutChSeqCfg.Bits.Address_Update_Rate_Mode           = 0;
      ReutChSeqCfg.Bits.Stop_Base_Sequence_On_Wrap_Trigger = 0;
      MrcWriteCR64 (
        MrcData,
        OFFSET_CALC_CH (REUT_CH_SEQ_CFG_0_CNL_REG, REUT_CH_SEQ_CFG_1_CNL_REG, Channel),
        ReutChSeqCfg.Data
      );
      // Change PCIT to 0xFF
      Offset = OFFSET_CALC_CH (CH0_CR_SC_PCIT_CNL_REG, CH1_CR_SC_PCIT_CNL_REG, Channel);
      ScPcit.Data = MrcReadCR (MrcData, Offset);
      ScPcitSave[Channel] = ScPcit;
      ScPcit.Bits.PCIT = 0xFF;
      MrcWriteCR (MrcData, Offset, ScPcit.Data);

      // Enable the ECC in IO and disable the ECC Logic.
      Offset = MAD_INTRA_CH0_CNL_REG + ((MAD_INTRA_CH1_CNL_REG - MAD_INTRA_CH0_CNL_REG) * Channel);
      MadIntra.Data = MrcReadCR (MrcData, Offset);
      MadIntraSave[Channel].Data = MadIntra.Data;
      MadIntra.Bits.ECC          = emEccIoActive;
      MrcWriteCR (MrcData, Offset, MadIntra.Data);

    }

    // Continue to setup Reut and Run Per Rank
    for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
      if (((1 << Rank) & Outputs->ValidRankMask) == 0) {
        continue;
      }
      RankToDimm = RANK_TO_DIMM_NUMBER (Rank);
      // Determine the Active Channels
      ActiveChBitMask = 0;
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        ActiveChBitMask |= SelectReutRanks (MrcData, Channel, MRC_BIT0 << Rank, FALSE, 0);
      }

      // Program the sequence addresses and loopcount per channel
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        if (((ActiveChBitMask & (1 << Channel)) == 0) || (!MrcRankInChannelExist (MrcData, Rank, Channel))) {
          continue;
        }
        WDBPattern.IncRate    = 0;
        WDBPattern.Start      = 0;
        WDBPattern.Stop       = 0;
        WDBPattern.DQPat      = StaticPattern;
        WDBPattern.PatSource  = MrcPatSrcDynamic;
        WDBPattern.EnableXor  = FALSE;

        // Program Data Patttern Controls.  PGs are selected for Data
        MrcSelectPatternGen (MrcData, ActiveChBitMask, 0);
        for (Chunk = 0; Chunk < MRC_CPGC_MAX_CHUNKS; Chunk++) {
          Patterns[Chunk].Data = 0;
        }
        MrcSetPgPattern (MrcData, Patterns, MRC_CPGC_MAX_CHUNKS, 0);

        // Write the LFSR seeds
        if (WDBPattern.DQPat == StaticPattern) {
          MrcInitPgMux (MrcData, DataStaticSeeds, 0, MRC_NUM_MUX_SEEDS);
        }

        Offset = REUT_PG_PAT_CL_MUX_CFG_CNL_REG;
        ReutPgPatMuxCfg.Data = 0;
        if (WDBPattern.DQPat == StaticPattern) {
          ReutPgPatMuxCfg.Bits.Mux0_Control = MrcPgMuxBitBuffer;
          ReutPgPatMuxCfg.Bits.Mux1_Control = MrcPgMuxBitBuffer;
          ReutPgPatMuxCfg.Bits.Mux2_Control = MrcPgMuxBitBuffer;
        }
        ReutPgPatMuxCfg.Bits.LFSR_Type_0 = MrcLfsr16;
        ReutPgPatMuxCfg.Bits.LFSR_Type_1 = MrcLfsr16;
        ReutPgPatMuxCfg.Bits.LFSR_Type_2 = MrcLfsr16;
        MrcWriteCR64 (MrcData, Offset, ReutPgPatMuxCfg.Data);

        MrcProgramDataPatternRotation (MrcData, &WDBPattern, Channel);

        // Update reut_ch_seq_base_addr registers.
        DimmOut = &ControllerOut->Channel[Channel].Dimm[RankToDimm];
        BankCount = DimmOut->BankGroups * DimmOut->Banks;

        // Go over all addresses
        ReutAddress.Stop[MrcReutFieldCol]  = (DimmOut->ColumnSize) - WDB_CACHE_LINE_SIZE;
        ReutAddress.Stop[MrcReutFieldRow]  = (DimmOut->RowSize) - 1;
        ReutAddress.Stop[MrcReutFieldBank] = BankCount - 1;

        ReutAddress.IncVal[MrcReutFieldCol]  = 1;  // Each write is 1 cache line which is 8 column addresses worth of data.
        ReutAddress.IncVal[MrcReutFieldRow]  = 1;  // Walk through rows 1 at a time.
        ReutAddress.IncVal[MrcReutFieldBank] = 1;  // Walk through bank 1 at a time.

        ReutAddress.IncRate[MrcReutFieldRow]  = DimmOut->ColumnSize / WDB_CACHE_LINE_SIZE;
        ReutAddress.IncRate[MrcReutFieldBank] = DimmOut->RowSize * (DimmOut->ColumnSize / WDB_CACHE_LINE_SIZE);

        Offset = REUT_PG_ID_PATTERN_BUFFER_CTL_CNL_REG;
        PatBufCtl.Data = MrcReadCR (MrcData, Offset);
        PatBufCtl.Bits.ECC_Replace_Byte_Control = ActiveChBitMask;
        MrcWriteCR (MrcData, Offset, PatBufCtl.Data);

        MrcProgramSequenceAddress (
          MrcData,
          Channel,
          ReutAddress.Start,
          ReutAddress.Stop,
          ReutAddress.Order,
          ReutAddress.IncRate,
          ReutAddress.IncVal,
          WrapTriggerEn,
          WrapCarryEn,
          AddrInvertEn,
          0,                    // AddrInvertRate
          TRUE
        );

        // Set up the Subsequence control.
        ReutSubSeqCtl.Data = 0;
        ReutSubSeqCtl.Bits.Subsequence_Type = BWr;
        ReutSubSeqCtl.Bits.Number_of_Cachelines = MrcLog2 ((DimmOut->ColumnSize / WDB_CACHE_LINE_SIZE)) - 1; // MrcLog2 is giving log value plus 1
        ReutSubSeqCtl.Bits.Number_of_Cachelines_Scale = 0;
        MrcWriteCR (
          MrcData,
          OFFSET_CALC_CH (REUT_CH0_SUBSEQ_CTL_0_CNL_REG, REUT_CH1_SUBSEQ_CTL_0_CNL_REG, Channel),
          ReutSubSeqCtl.Data
        );

        // Program loopcount registers
        MrcSetLoopcount (MrcData, Channel, BankCount * (DimmOut->RowSize));
      } // for Channel

      // Run the test on both Channels
      MrcRunMemoryScrub (MrcData, ActiveChBitMask, TRUE);
    } // for Rank

    // To avoid an invalid timing of Temp read, we need to do the following:
    //   Save previous value
    //   Set MR4_PERIOD to 0
    //   Wait tREFI
    //   Go to Normal mode
    //   Restore MR4_PERIOD
    if (EccEnabled) {
      tREFI = 0;
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        if (MrcChannelExist (Outputs, Channel)) {

          // Restore the ECC mode
          Offset = MAD_INTRA_CH0_CNL_REG + ((MAD_INTRA_CH1_CNL_REG - MAD_INTRA_CH0_CNL_REG) * Channel);
          MadIntra.Data = MadIntraSave[Channel].Data;
          MrcWriteCR (MrcData, Offset, MadIntra.Data);

          Offset = CH0_CR_DDR_MR_PARAMS_CNL_REG +
            (CH1_CR_DDR_MR_PARAMS_CNL_REG - CH0_CR_DDR_MR_PARAMS_CNL_REG) * Channel;
          DdrMrParamsSave[Channel] = MrcReadCR (MrcData, Offset);
          DdrMrParams.Data = DdrMrParamsSave[Channel];
          DdrMrParams.Bits.MR4_PERIOD = 0;
          MrcWriteCR (MrcData, Offset, DdrMrParams.Data);
          Timing = &Outputs->Controller[0].Channel[Channel].Timing[Inputs->MemoryProfile];
          tREFI = MAX (tREFI, Timing->tREFI);
        }
      }
      // Memory Clock is in fs.  Scale to ps then multiply by tREFI.  Then Ciel to nearest ns.
      tREFI = tREFI * (Outputs->MemoryClock / 1000);
      tREFI = DIVIDECEIL (tREFI, 1000);
      MrcWait (MrcData, tREFI / HPET_MIN_NS);
    }

    // Return to normal operation mode
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (MrcChannelExist (Outputs, Channel)) {
        ReutChSeqCfg.Data                     = 0;
        ReutChSeqCfg.Bits.Initialization_Mode = NOP_Mode;
        MrcWriteCR (
          MrcData,
          OFFSET_CALC_CH (REUT_CH_SEQ_CFG_0_CNL_REG, REUT_CH_SEQ_CFG_1_CNL_REG, Channel),
          (UINT32) ReutChSeqCfg.Data
          );
      }
    }

    // Restore MR4
    if (EccEnabled) {
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        if (MrcChannelExist (Outputs, Channel)) {
          Offset = CH0_CR_DDR_MR_PARAMS_CNL_REG +
            (CH1_CR_DDR_MR_PARAMS_CNL_REG - CH0_CR_DDR_MR_PARAMS_CNL_REG) * Channel;
          MrcWriteCR (MrcData, Offset, DdrMrParamsSave[Channel]);
        }
      }
    }

    if (Status != mrcSuccess) {
      MrcCall->MrcDebugHook (MrcData, MRC_ECC_CLEAN_ERROR);
    }
    // Restore PCIT value
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (MrcChannelExist (Outputs, Channel)) {
        Offset = OFFSET_CALC_CH (CH0_CR_SC_PCIT_CNL_REG, CH1_CR_SC_PCIT_CNL_REG, Channel);
        MrcWriteCR (MrcData, Offset, ScPcitSave[Channel].Data);
      }
    }
  } // if scrubbing required

  return Status;
}

/**
  This function performs a memory alias check.

  @param[in] MrcData - The global host structure

  @retval mrcSuccess or error value.
**/
MrcStatus
MrcAliasCheck (
  IN OUT MrcParameters *const MrcData
  )
{
  const MRC_FUNCTION            *MrcCall;
  MrcDebug                      *Debug;
  MrcInput                      *Inputs;
  MrcOutput                     *Outputs;
  MrcControllerOut              *ControllerOut;
  MrcDimmOut                    *DimmOut;
  MrcStatus                     Status;
  INT64                         RefreshSave;
  INT64                         GetSetVal;
  UINT32                        SdramAddressingCapacity;
  UINT32                        CrOffset;
  UINT32                        SdramCapacity;
  UINT8                         Rank;
  UINT8                         RankToDimm;
  UINT8                         Channel;
  UINT8                         ActiveChBitMask;
  BOOLEAN                       InvalidSpdAddressingCapacity;
  MRC_REUTAddress               ReutAddress;
  REUT_CH_SEQ_CFG_0_CNL_STRUCT  ReutChSeqCfg;
  MAD_INTRA_CH0_CNL_STRUCT      MadIntraOrig[MAX_CHANNEL];
  MAD_INTRA_CH0_CNL_STRUCT      MadIntra;
  BOOLEAN                       ReutUninitialized;
  BOOLEAN                       CmdMirroredSave;
  MrcIntOutput                  *IntOutputs;
  UINT16                        MinRowSize;
  UINT32                        MaxRowSize;
  UINT8                         BankCount;
  BOOLEAN                       Lpddr4;
  CH0_CR_SC_PCIT_CNL_STRUCT     ScPcit;
  INT64                         IocEccEn;
  CH0_CR_SC_PCIT_CNL_STRUCT     ScPcitOrg[MAX_CHANNEL];
  CH0_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_STRUCT Lp4DqsOsclParamsOrg[MAX_CHANNEL];
  CH0_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_STRUCT Lp4DqsOsclParams;
  MCMISCS_DELTADQSCOMMON0_CNL_STRUCT          DeltaDqsCommon;
  MCMISCS_DELTADQSCOMMON0_CNL_STRUCT          DeltaDqsCommonOrg;
  REUT_CH_SEQ_RANK_LOGICAL_TO_PHYSICAL_MAPPING_0_CNL_STRUCT  ReutChSeqRankL2PMapping;

  Outputs       = &MrcData->Outputs;
  Inputs        = &MrcData->Inputs;
  MrcCall       = Inputs->Call.Func;
  Debug         = &Outputs->Debug;
  ControllerOut = &Outputs->Controller[0];
  Status        = mrcSuccess;
  Lpddr4        = (Outputs->DdrType == MRC_DDR_TYPE_LPDDR4);
  IntOutputs    = (MrcIntOutput *) (MrcData->IntOutputs.Internal);

  // Disable Mirror
  CmdMirroredSave = IntOutputs->CmdMirrored;
  IntOutputs->CmdMirrored = FALSE;
  GetSetVal = 0;
  MrcGetSetDdrIoGroupChannel (MrcData, MAX_CHANNEL, GsmMccCmdMirrorEn, WriteNoCache, &GetSetVal);

  InvalidSpdAddressingCapacity  = FALSE;
  MinRowSize = (1 << 14); // Minimum row size of CNL DDR4 or LP4.
  MaxRowSize = (1 << 17); // Maximum row size of CNL DDR4 or LP4.

  // Check to see if the SDRAM Addressing * Primary Bus Width == SDRAM capacity.
  // If not, report an alias and exit.
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank += MAX_RANK_IN_DIMM) {
      if (MrcRankInChannelExist(MrcData, Rank, Channel)) {
        RankToDimm = RANK_TO_DIMM_NUMBER(Rank);
        DimmOut                 = &ControllerOut->Channel[Channel].Dimm[RankToDimm];
        SdramAddressingCapacity = (DimmOut->ColumnSize * DimmOut->RowSize);
        // Since the minimum number of row and column bits are 12 and 9 respectively,
        // we can shift by 20 to get the result in Mb before multiplying by the bus width.
        SdramAddressingCapacity  = SdramAddressingCapacity >> 20;
        SdramAddressingCapacity *= DimmOut->Banks;
        SdramAddressingCapacity *= DimmOut->BankGroups;
        SdramAddressingCapacity *= DimmOut->SdramWidth;
        if (Lpddr4) {
          // SPD Density is per die, but SdramWidth is per LP4 channel, so need to multiple by number of channels
          SdramAddressingCapacity *= DimmOut->ChannelsPerSdramPackage;
        }
        SdramCapacity            = SdramCapacityTable[DimmOut->DensityIndex] * 8;
        BankCount = (Lpddr4) ? 8 : ((DimmOut->SdramWidth == 8) ? 16 : 8);

        if ((SdramCapacity != SdramAddressingCapacity) || (DimmOut->ColumnSize != 0x400)
             || ((DimmOut->Banks * DimmOut->BankGroups) != BankCount)
             || (DimmOut->RowSize < MinRowSize) || (DimmOut->RowSize > MaxRowSize)) {
          InvalidSpdAddressingCapacity = TRUE;
          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_ERROR,
            "ERROR: Channel %d Dimm %d SPD SDRAM Adressing Capacity(0x%xMb) does not match SDRAM Capacity(0x%xMb)\nPlease verify:\n",
            Channel,
            RankToDimm,
            SdramAddressingCapacity,
            SdramCapacity
            );
          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_ERROR,
            " Capacity: 0x%x\n RowSize: 0x%x\n ColumnSize: 0x%x\n Banks: 0x%x\n Bank Groups: 0x%x\n Device Width: 0x%x\n",
            SdramCapacity,
            DimmOut->RowSize,
            DimmOut->ColumnSize,
            DimmOut->Banks,
            DimmOut->BankGroups,
            DimmOut->SdramWidth
            );
          break;
        }
      }
    }
  }
  // Since we will not hang the system, signal that an Alias could exist and return mrcSuccess.
  if (TRUE == InvalidSpdAddressingCapacity) {
    Outputs->SpdSecurityStatus = MrcSpdStatusAliased;
    MRC_DEBUG_ASSERT (
      (InvalidSpdAddressingCapacity == FALSE),
      Debug,
      "...Memory Alias detected - Invalid Spd Addressing Capacity...\n"
    );
    return Status;
  }

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Performing Alias Test\n");
  MrcCall->MrcSetMem ((UINT8 *) &ReutAddress, sizeof (ReutAddress), 0);
  ReutUninitialized = TRUE;

  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    if (MrcChannelExist (Outputs, Channel)) {
      // Determine if we are ECC enabled.  If so, disable ECC since the ECC scrub has yet to occur.
      if (Outputs->EccSupport == TRUE) {
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ECC enabled.  Disabling ECC for the test.  Must scrub after this!!!\n");
        CrOffset = MAD_INTRA_CH0_CNL_REG + ((MAD_INTRA_CH1_CNL_REG - MAD_INTRA_CH0_CNL_REG) * Channel);
        MadIntraOrig[Channel].Data = MrcReadCR (MrcData, CrOffset);
        MadIntra.Data              = MadIntraOrig[Channel].Data;
        MadIntra.Bits.ECC          = emNoEcc;
        MrcWriteCR (MrcData, CrOffset, MadIntra.Data);
      }
      if (Lpddr4) {
        // Disable the DQS Osillator for LP4.
        CrOffset = OFFSET_CALC_CH (CH0_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_REG, CH1_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_REG, Channel);
        Lp4DqsOsclParams.Data                 = MrcReadCR (MrcData, CrOffset);
        Lp4DqsOsclParamsOrg[Channel].Data     = Lp4DqsOsclParams.Data;
        Lp4DqsOsclParams.Bits.DQSOSCL_PERIOD  = 0;
        Lp4DqsOsclParams.Bits.DIS_SRX_DQSOSCL = 1;
        MrcWriteCR (MrcData, CrOffset, Lp4DqsOsclParams.Data);
      }
      // Change PCIT to 0xFF
      CrOffset            = OFFSET_CALC_CH (CH0_CR_SC_PCIT_CNL_REG, CH1_CR_SC_PCIT_CNL_REG, Channel);
      ScPcit.Data         = MrcReadCR (MrcData, CrOffset);
      ScPcitOrg[Channel]  = ScPcit;
      ScPcit.Bits.PCIT    = 0xFF;
      MrcWriteCR (MrcData, CrOffset, ScPcit.Data);
    }
  }
  if (Outputs->EccSupport) {
    MrcGetSetDdrIoGroupController0 (MrcData, GsmIocEccEn, ReadFromCache, &IocEccEn);
    GetSetVal = 0;
    MrcGetSetDdrIoGroupController0 (MrcData, GsmIocEccEn, ForceWriteCached, &GetSetVal);
  }
  DeltaDqsCommonOrg.Data = 0;
  if (Lpddr4) {
   // Disable DqDqs Retraining feature
    CrOffset = MCMISCS_DELTADQSCOMMON0_CNL_REG;
    DeltaDqsCommon.Data = MrcReadCR (MrcData, CrOffset);
    DeltaDqsCommonOrg.Data = DeltaDqsCommon.Data;
    if (DeltaDqsCommonOrg.Bits.Lp4DeltaDQSTrainMode == 1) {
      DeltaDqsCommon.Bits.Lp4DeltaDQSTrainMode = 0;
      MrcWriteCR (MrcData, CrOffset, DeltaDqsCommon.Data);
    }
  }

  // Enable Refreshes. Save previous state.
  MrcGetSetDdrIoGroupController0(MrcData, GsmMccEnableRefresh, ReadNoCache, &RefreshSave);
  GetSetVal = 1;
  MrcGetSetDdrIoGroupController0(MrcData, GsmMccEnableRefresh, WriteNoCache, &GetSetVal);

  // Run Alias check test Per Dimm
  for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank += MAX_RANK_IN_DIMM){
    if ((MRC_BIT0 << Rank) & (Outputs->ValidRankMask)) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Testing Dimm %d\n", Rank / 2);
      ReutUninitialized = TRUE;
      MrcMemoryCheckSetup (MrcData, &ReutAddress, Rank, 0, &ReutUninitialized);
      // Determine Active Channels
      ActiveChBitMask = 0;
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        ActiveChBitMask |= SelectReutRanks (MrcData, Channel, MRC_BIT0 << Rank, FALSE, 0);
        CrOffset = REUT_CH_SEQ_RANK_LOGICAL_TO_PHYSICAL_MAPPING_0_CNL_REG + ((REUT_CH_SEQ_RANK_LOGICAL_TO_PHYSICAL_MAPPING_1_CNL_REG - REUT_CH_SEQ_RANK_LOGICAL_TO_PHYSICAL_MAPPING_0_CNL_REG) * Channel);
        ReutChSeqRankL2PMapping.Data = MrcReadCR (MrcData, CrOffset);
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "Ch:%x, ReutChSeqRankL2PMapping:%x\n", Channel, ReutChSeqRankL2PMapping.Data);
      }
      // Run the test
      Status = MrcRunMemoryScrub (MrcData, ActiveChBitMask, TRUE);
      // For LPDDR4 run scrub on a single rank per channel.
      // since other ranks in the same subchannel or on the other the subchannel in the channel are of same size .
      if ((Status != mrcSuccess) || (Lpddr4)) {
        break;
      }
    }
  }

  if (Lpddr4 && (DeltaDqsCommonOrg.Bits.Lp4DeltaDQSTrainMode == 1)) {
    // Restore DqDqs Retraining value
    CrOffset = MCMISCS_DELTADQSCOMMON0_CNL_REG;
    MrcWriteCR (MrcData, CrOffset, DeltaDqsCommonOrg.Data);
  }
  if (Outputs->EccSupport) {
    MrcGetSetDdrIoGroupController0 (MrcData, GsmIocEccEn, ForceWriteCached, &IocEccEn);
  }
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    if (MrcChannelExist (Outputs, Channel)) {
      // Restore PCIT value
      CrOffset  = OFFSET_CALC_CH (CH0_CR_SC_PCIT_CNL_REG, CH1_CR_SC_PCIT_CNL_REG, Channel);
      MrcWriteCR (MrcData, CrOffset, ScPcitOrg[Channel].Data);
      if (Lpddr4) {
        // Restore the DQS Oscillator value for LP4.
        CrOffset = OFFSET_CALC_CH (CH0_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_REG, CH1_CR_LP4_DQS_OSCILLATOR_PARAMS_CNL_REG, Channel);
        MrcWriteCR (MrcData, CrOffset, Lp4DqsOsclParamsOrg[Channel].Data);
      }
      // ReEnable ECC logic.
      if (Outputs->EccSupport == TRUE) {
        CrOffset = MAD_INTRA_CH0_CNL_REG + ((MAD_INTRA_CH1_CNL_REG - MAD_INTRA_CH0_CNL_REG) * Channel);
        MrcWriteCR (MrcData, CrOffset, MadIntraOrig[Channel].Data);
      }
      // Wait 4 usec after enabling the ECC IO, needed by HW
      MrcWait (MrcData, 4 * HPET_1US);
      // Enable Mirror
      IntOutputs->CmdMirrored = CmdMirroredSave;
      if (CmdMirroredSave == TRUE) {
        GetSetVal = 1;
        MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMccCmdMirrorEn, WriteNoCache, &GetSetVal);
      }
      // Return to normal operation mode
      CrOffset = OFFSET_CALC_CH (REUT_CH_SEQ_CFG_0_CNL_REG, REUT_CH_SEQ_CFG_1_CNL_REG, Channel);
      ReutChSeqCfg.Data = 0;
      ReutChSeqCfg.Bits.Initialization_Mode = NOP_Mode;
      MrcWriteCR (MrcData, CrOffset, (UINT32) ReutChSeqCfg.Data);
    }
  }

  if (mrcSuccess != Status) {
    Outputs->SpdSecurityStatus = MrcSpdStatusAliased;
    MRC_DEBUG_ASSERT (
      (Status == mrcSuccess),
      Debug,
      "*** Alias Detected!  See REUT Error above. ***\n Error Status : %x \n", Status
    );
    Status = mrcSuccess;
  }

  // Restore Refreshes.
  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccEnableRefresh, WriteNoCache, &RefreshSave);

  return Status;
}

/**
  This function runs the srcubbing test reporting any timeouts/errors.

  @param[in] MrcData - The global host structure
  @param[in] ChBitMask - Bitmask of channels the test is run on.
  @param[in] MsgPrint  - Print debug messages

  @retval mrcSuccess or error value.
**/
MrcStatus
MrcRunMemoryScrub (
  IN OUT MrcParameters *const MrcData,
  IN     UINT8                ChBitMask,
  IN     BOOLEAN              MsgPrint
  )
{
  const MRC_FUNCTION *MrcCall;
  MrcDebug           *Debug;
  UINT8              Channel;
  UINT8              SubCh;
  UINT16             Offset;
  MrcDebugMsgLevel   DebugLevel;
  MrcStatus          Status;
  UINT8              ErrorStatus;
  UINT8              TestDoneStatus;
  REUT_GLOBAL_CTL_CNL_STRUCT       ReutGlobalCtl;
  REUT_GLOBAL_ERR_CNL_STRUCT       ReutGlobalErr;
  REUT_SUBCH_ERROR_ADDR_0_CNL_STRUCT SubChErrAddr;
  UINT32  Timer;

  Status  = mrcSuccess;
  Debug   = &MrcData->Outputs.Debug;
  MrcCall = MrcData->Inputs.Call.Func;
  DebugLevel = MsgPrint ? MSG_LEVEL_ERROR : MSG_LEVEL_NEVER;

  IoReset (MrcData);
  // Setup Timer and run the test
  Timer = (UINT32) MrcCall->MrcGetCpuTime (MrcData) + 20000; // 20 Second timeout
  ReutGlobalCtl.Data                                = 0;
  ReutGlobalCtl.Bits.Global_Start_Test              = 1;
  ReutGlobalCtl.Bits.Global_Clear_Errors            = 1;
  ReutGlobalCtl.Bits.Global_Stop_Test_On_Any_Error  = NSOE;
  MrcWriteCR (MrcData, REUT_GLOBAL_CTL_CNL_REG, ReutGlobalCtl.Data);

  // Wait until Channel test done status matches ChbitMask or TimeoutCounter value reaches 0;
  do {
    ReutGlobalErr.Data = MrcReadCR (MrcData, REUT_GLOBAL_ERR_CNL_REG);
    TestDoneStatus     = (UINT8) ((ReutGlobalErr.Bits.Channel_Test_Done_Status_1 << 1) |
                                ReutGlobalErr.Bits.Channel_Test_Done_Status_0);
  } while (((TestDoneStatus & ChBitMask) != ChBitMask) && ((UINT32) MrcCall->MrcGetCpuTime (MrcData) < Timer));

  if ((TestDoneStatus & ChBitMask) != ChBitMask) {
    Status = mrcDeviceBusy;
    MRC_DEBUG_MSG (
      Debug,
      DebugLevel,
      "Timeout occured while running the test: ReutGlobalErr: 0x%X.\n",
      ReutGlobalErr.Data
      );
  }

  // For x64 Channels, we can break out as soon as either SubChannel has an error for the channels populated.
  // Same as Error Status mask.
  // Current assumption is SubChannels are run sequentially.  Traffic is only sent on tested sub channel.  If a failure occurs, report it as an error for that Channel.
  // If a Sch is not populated, its Error status is Don't Care.
  // Not Valid (NV)
  // Sc1,Sc0   | 0,0 | 0,1 | 1,1 | 1,0 |
  // Sc1E,Sc0E |-----------------------|
  //    0,0    | NV  |  0  |  0  |  0  |
  //    0,1    | NV  |  1  |  1  |  0  |
  //    1,1    | NV  |  1  |  1  |  1  |
  //    1,0    | NV  |  0  |  1  |  1  |
  //           |-----------------------|
  // SA:RestricteContent - @todo: <CNL> Modify when enabling 2 SCH parallel execution
  ErrorStatus = (UINT8) ((ReutGlobalErr.Bits.Channel_Error_Status_3 | ReutGlobalErr.Bits.Channel_Error_Status_2) << 1 |
                          (ReutGlobalErr.Bits.Channel_Error_Status_1 | ReutGlobalErr.Bits.Channel_Error_Status_0));
  if (ErrorStatus & ChBitMask) {
    Status = mrcReutSequenceError;
    MRC_DEBUG_MSG (
      Debug,
      DebugLevel,
      "REUT Error: Channel(s):%s%s%s%s\n",
      (ReutGlobalErr.Bits.Channel_Error_Status_0 == 1) ? " 0" : "",
      (ReutGlobalErr.Bits.Channel_Error_Status_1 == 1) ? " 1" : "",
      (ReutGlobalErr.Bits.Channel_Error_Status_2 == 1) ? " 2" : "",
      (ReutGlobalErr.Bits.Channel_Error_Status_3 == 1) ? " 3" : ""
      );
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (!MrcChannelExist (&MrcData->Outputs, Channel)) {
        continue;
      }
      // Read out per byte error results
      for (SubCh = 0; SubCh < MrcData->Outputs.SubChCount; SubCh++) {
        if (!MrcSubChannelExist (MrcData, Channel, SubCh)) {
          continue;
        }
        Offset = REUT_SUBCH_ERROR_ADDR_0_CNL_REG +
          ((REUT_SUBCH_ERROR_ADDR_2_CNL_REG - REUT_SUBCH_ERROR_ADDR_0_CNL_REG) * Channel) +
          ((REUT_SUBCH_ERROR_ADDR_2_CNL_REG - REUT_SUBCH_ERROR_ADDR_0_CNL_REG) * SubCh);
        SubChErrAddr.Data = MrcReadCR64 (MrcData, Offset);
        MRC_DEBUG_MSG (
          Debug,
          DebugLevel,
          "REUT Error: Ch[%d] SubCh[%d] SUBCH_ERROR_ADDR Rank:0x%x, Bank:%d, Row:0x%x, Col:0x%x \n",
          Channel,
          SubCh,
          SubChErrAddr.Bits.Rank_Address,
          SubChErrAddr.Bits.Bank_Address,
          SubChErrAddr.Bits.Row_Address,
          SubChErrAddr.Bits.Column_Address
        );
      }
    }
  }

  return Status;
}

/**
  This function provides MRC core hook to call TXT Alias Check before DRAM Init Done.

  @param[in]  MrcData - Pointer to global MRC structure.

  @retval mrcSuccess.
**/
MrcStatus
MrcTxtAliasCheck (
  IN MrcParameters *const MrcData
  )
{
  MrcData->Inputs.Call.Func->MrcTxtAcheck();
  return mrcSuccess;
}
