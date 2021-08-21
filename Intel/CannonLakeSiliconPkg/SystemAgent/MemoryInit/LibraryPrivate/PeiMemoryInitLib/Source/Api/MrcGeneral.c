/** @file
  This file contains all the MRC general API to the MRC wrapper.

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
#include "MrcGeneral.h"
#include "MrcDdrCommon.h"
#include "MrcDdr4.h"
#include "MrcLpddr4.h"
#include "MrcMemoryApi.h"
#include "MrcStartMemoryConfiguration.h"
#include "MrcTimingConfiguration.h"

GLOBAL_REMOVE_IF_UNREFERENCED const MrcVersion  cVersion[] = {
  #include "MrcVersion.h"
};

//
// RCOMP target values for { RdOdt, WrDS, WrDSCmd, WrDSCtl, WrDSClk } - for CNL with LPDDR4[X]/DDR4
//
const UINT16 RcompTargetCnlLpddr4[MAX_RCOMP_TARGETS]  = {  40, 40, 40, 40, 30 };
const UINT16 RcompTargetCnlLpddr4x[MAX_RCOMP_TARGETS] = {  60, 40, 40, 40, 30 };
const UINT16 RcompTargetCnlUDdr4[MAX_RCOMP_TARGETS]   = { 100, 33, 32, 33, 28 };
const UINT16 RcompTargetCnlHDdr4[MAX_RCOMP_TARGETS]   = {  40, 30, 33, 33, 30 };

#ifdef MRC_DEBUG_PRINT
extern const char CcdString[];
const char TrainEnString[]  = "TrainingEnables";
const char BaseTimeString[] = "BaseTime";
const char ThermEnString[]  = "ThermalEnables";
const char PrintBorder[]    = "*************************************\n";
#endif // MRC_DEBUG_PRINT

// This table is used for LPDDR4 MR5 decoding
struct {
  UINT8   VendorId;
  UINT16  JedecId;
  char    *VendorName;
} DramVendorList [] = {
  { 1,    0xCE00, "Samsung" },
  { 3,    0xFE02, "Elpida"  },
  { 6,    0xAD00, "Hynix"   },
  { 0xFF, 0x2C00, "Micron"  },
};
/**
  Enable/Disable DLL WeakLock if needed.
  Note: We don't enable it in McConfig because CKE is still low during that step.

  @param[in] MrcData - The MRC general data.
  @param[in]  Enable - BOOLEAN control to enable (if TRUE), or disable (if FALSE) WeakLock.

  @retval None
**/
void
MrcWeaklockEnDis (
  IN MrcParameters *const MrcData,
  IN BOOLEAN              Enable
  )
{
  const MrcInput  *Inputs;
  MrcOutput       *Outputs;
  UINT32          Offset;
  UINT8           Channel;
  INT64           GetSetVal;
  DDRCTLCLKCKE0_CR_DDRCRCTLCONTROLS_CNL_STRUCT  DdrCrCCCControls;
  DDRCMDCH0_CR_DDRCRCMDCONTROLS_CNL_STRUCT      DdrCrCmdControls;
  DDRCKECH0_CR_DDRCRCTLCONTROLS_CNL_STRUCT      DdrCrCkeControls;
  DDRCTLCH0_CR_DDRCRCTLCONTROLS_CNL_STRUCT      DdrCrCtlControls;

  Inputs  = &MrcData->Inputs;
  Outputs = &MrcData->Outputs;

  GetSetVal = (Enable) ? 1 : 0;

  MrcGetSetDdrIoGroupController0 (MrcData, GsmIocDllWlEn, WriteCached, &GetSetVal);
  MrcGetSetDdrIoGroupChannelStrobe (MrcData, MAX_CHANNEL, MAX_SDRAM_IN_DIMM, GsmIocDllWeakLock, WriteCached, &GetSetVal);

  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    if (!(MrcChannelExist (Outputs, Channel))) {
      continue;
    }
    Offset = OFFSET_CALC_CH (DDRCMDACH0_CR_DDRCRCMDCONTROLS_CNL_REG, DDRCMDACH1_CR_DDRCRCMDCONTROLS_CNL_REG, Channel);
    DdrCrCmdControls.Data = MrcReadCR (MrcData, Offset);
    DdrCrCmdControls.Bits.DllWeakLock = Enable;
    MrcWriteCR (MrcData, Offset, DdrCrCmdControls.Data);

    Offset = OFFSET_CALC_CH (DDRCMDBCH0_CR_DDRCRCMDCONTROLS_CNL_REG, DDRCMDBCH1_CR_DDRCRCMDCONTROLS_CNL_REG, Channel);
    DdrCrCmdControls.Data = MrcReadCR (MrcData, Offset);
    DdrCrCmdControls.Bits.DllWeakLock = Enable;
    MrcWriteCR (MrcData, Offset, DdrCrCmdControls.Data);

    Offset = OFFSET_CALC_CH (DDRCTLCH0_CR_DDRCRCTLCONTROLS_CNL_REG, DDRCTLCH1_CR_DDRCRCTLCONTROLS_CNL_REG, Channel);
    DdrCrCtlControls.Data = MrcReadCR (MrcData, Offset);
    DdrCrCtlControls.Bits.DllWeakLock = Enable;
    MrcWriteCR (MrcData, Offset, DdrCrCtlControls.Data);

    Offset = OFFSET_CALC_CH (DDRCKECH0_CR_DDRCRCTLCONTROLS_CNL_REG, DDRCKECH1_CR_DDRCRCTLCONTROLS_CNL_REG, Channel);
    DdrCrCkeControls.Data = MrcReadCR (MrcData, Offset);
    DdrCrCkeControls.Bits.DllWeakLock = Enable;
    MrcWriteCR (MrcData, Offset, DdrCrCkeControls.Data);

    if (Inputs->CpuidModel == cmCNL_DT_HALO) {
      Offset = OFFSET_CALC_CH (DDRCTLCLKCKE0_CR_DDRCRCTLCONTROLS_CNL_REG, DDRCTLCLKCKE1_CR_DDRCRCTLCONTROLS_CNL_REG, Channel);
      DdrCrCCCControls.Data = MrcReadCR (MrcData, Offset);
      DdrCrCCCControls.Bits.DllWeakLock = Enable;
      MrcWriteCR (MrcData, Offset, DdrCrCCCControls.Data);
    }
  } // for Channel
}

/**
  Read LPDDR information from MR5 and MR8 and print to the debug log.
  Also update the Manufacturer's ID in the SPD table, for BIOS Setup and SMBIOS table.

  @param[in] MrcData - include all the MRC general data.

  @retval    Returns mrcWrongInputParameter, if MR8 IO Width and SPD SDARM width does not match
             else returns mrcSuccess.
**/
MrcStatus
ShowLpddrInfo (
  IN  MrcParameters *const MrcData
  )
{
  MrcInput        *Inputs;
  MrcDebug        *Debug;
  MrcOutput       *Outputs;
  MrcIntOutput    *MrcIntData;
  MrcDimmOut      *DimmOut;
  MrcSpd          *SpdIn;
  UINT32          Channel;
  UINT32          SubCh;
  UINT32          Rank;
  UINT8           MrrResult[4];
  UINT32          MrAddr;
  UINT32          Device;
  UINT32          Index;
  BOOLEAN         VendorFound;
  UINT16          JedecId;
  MrcStatus       Status;
  UINT8           MaxDevice;
  LPDDR4_MODE_REGISTER_8_TYPE *Mrr8Result;

  Inputs      = &MrcData->Inputs;
  Outputs     = &MrcData->Outputs;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));
  Debug       = &Outputs->Debug;
  VendorFound = FALSE;
  Index       = 0;
  Status      = mrcSuccess;
  MaxDevice   = 2;

  if (Inputs->BootMode != bmCold) {
    // Full deswizzle table is not present on non-cold flows, so cannot parse MR read.
    return Status;
  }
  // LPDDR: Read MR5 and MR8
  if (Outputs->DdrType == MRC_DDR_TYPE_LPDDR4) {
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
        if (!MrcRankInChannelExist (MrcData, (UINT8) Rank, (UINT8) Channel)) {
          continue;
        }
        for (SubCh = 0; SubCh < MAX_SUB_CHANNEL; SubCh++) {
          if (!MrcSubChannelExist (MrcData, Channel, SubCh)) {
            continue;
          }
          // MR5 - Manufacturer ID
          MrAddr = 5;
          MrcIssueMrr (MrcData, Channel, SubCh, Rank, MrAddr, MrrResult);
          for (Device = 0; Device < MaxDevice; Device++) {
            MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\tDevice[%u]= 0x%02X", Device, MrrResult[Device]);
            VendorFound = FALSE;
            for (Index = 0; Index < sizeof (DramVendorList) / sizeof (DramVendorList[0]); Index++) {
              if (DramVendorList[Index].VendorId == MrrResult[Device]) {
                MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " %s\n", DramVendorList[Index].VendorName);
                VendorFound = TRUE;
                break;
              }
            }
            if (!VendorFound) {
              MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " Unknown\n");
            }
          }

          if (VendorFound) {
            // Patch SPD data with vendor ID code.
            // This is consumed by BIOS Setup and SMBIOS Type 17 table creation code.
            // If SAGV enabled, only do this on the last pass: SaGv High.
            if ((Inputs->SaGv != MrcSaGvEnabled) || (MrcIntData->SaGvPoint == MrcSaGvPointHigh)) {
              JedecId = DramVendorList[Index].JedecId;
              SpdIn = &Inputs->Controller[0].Channel[Channel].Dimm[dDIMM0].Spd.Data;
              SpdIn->Lpddr.ManufactureInfo.ModuleId.IdCode.Data = JedecId;
              SpdIn->Lpddr.ManufactureInfo.DramIdCode.Data      = JedecId;
            }
          }

          // MR8 - I/O Width
          MrAddr = 8;
          DimmOut = &MrcData->Outputs.Controller[0].Channel[Channel].Dimm[dDIMM0];
          MrcIssueMrr (MrcData, Channel, SubCh, Rank, MrAddr, MrrResult);
          for (Device = 0; Device < MaxDevice; Device++) {
            Mrr8Result = (LPDDR4_MODE_REGISTER_8_TYPE *) &MrrResult[Device];
            if (((DimmOut->SdramWidth == 8) && (Mrr8Result->Bits.IoWidth != 1)) || ((DimmOut->SdramWidth == 16) && (Mrr8Result->Bits.IoWidth != 0))) {
              MRC_DEBUG_MSG (Debug, MSG_LEVEL_WARNING, "Warning:  Ch[%d]Dimm[%d]Device[%d]: SPD SDRAM Width(=X%x) is not matching with MR[8] IO Width bits(=%d).\n", Channel, (Rank / MAX_RANK_IN_DIMM), Device, DimmOut->SdramWidth, MrrResult[Device]);
              Status = mrcWrongInputParameter;
            }
          } // for Device
        } // for Rank
      } // for SubCh
    } // for Channel
  } // if DdrType
  return Status;
}

/**
  This function changes the MC to normal mode, enables the ECC
  if needed. If the ECC is enabled, this function should be
  called after memory is cleaned.

  @param[in, out] MrcData - Include all MRC global data.

  @retval         Returns mrcSuccess or failure if DRAM width doesn't match.
**/
MrcStatus
MrcMcActivate (
  IN     MrcParameters *const MrcData
  )
{
  const MRC_FUNCTION  *MrcCall;
  const MrcInput      *Inputs;
  MrcDebug            *Debug;
  MrcIntOutput        *MrcIntData;
  MrcOutput           *Outputs;
  INT64               GetSetVal;
  INT64               GetSetDis;
  INT64               OdtSampExtEn;
  INT64               LpddrLongOdtEn;
  INT64               OdtDelay;
  INT64               OdtDuration;
  INT64               RxClkStgNum;
  INT64               RxFifoRdEnRank;
  INT32               Temp;
  UINT32              SubCh;
  UINT32              Offset;
  UINT32              GeneratedSeed;
  UINT32              i;
  UINT16              SAFE;
  UINT16              NS;
  UINT16              MaxRdDataValid;
  UINT16              RdDataValid;
  INT8                OdtTurnOff;
  UINT8               Controller;
  UINT8               Channel;
  UINT8               Byte;
  UINT8               Rank;
  UINT8               MaxRcvEn;
  UINT8               RcvEnDrift;
  UINT8               RcvEnTurnOff;
  UINT8               RcvEn;
  MrcStatus           Status;
  BOOLEAN             Lpddr;
  MAD_INTRA_CH0_CNL_STRUCT  MadIntra;
  DDRSCRAM_CR_DDRSCRAMBLECH0_CNL_STRUCT  DdrScramble;
  DDRSCRAM_CR_DDRMISCCONTROL2_CNL_STRUCT MiscControl2;
  CH0_CR_MC_INIT_STATE_CNL_STRUCT        McInitState;
  static const UINT32 ScramblerRegOffsets[] = {  // The offsets are not consecutive, hence use a table
    DDRSCRAM_CR_DDRSCRAMBLECH0_CNL_REG,
    DDRSCRAM_CR_DDRSCRAMBLECH1_CNL_REG,
    DDRSCRAM_CR_DDRSCRAMBLECH2_CNL_REG,
    DDRSCRAM_CR_DDRSCRAMBLECH3_CNL_REG
  };

  MrcIntData    = ((MrcIntOutput *)(MrcData->IntOutputs.Internal));
  Inputs        = &MrcData->Inputs;
  MrcCall       = Inputs->Call.Func;
  Outputs       = &MrcData->Outputs;
  Debug         = &MrcData->Outputs.Debug;
  GeneratedSeed = 0;
  SAFE          = 0;
  GetSetDis     = 0;
  Lpddr         = (Outputs->DdrType == MRC_DDR_TYPE_LPDDR4);
  Status        = mrcSuccess;

  if (Inputs->SafeMode) {
    SAFE = 0xFFFF;
  }
  NS = ~SAFE;

  MrcBeforeNormalModeTestMenu (MrcData);
  // Oem hook before normal mode configuration starts
  MrcInternalCheckPoint (MrcData, OemBeforeNormalMode, NULL);


  if (Outputs->DdrType == MRC_DDR_TYPE_DDR4) {
    MrcSetMrShadows (MrcData);
  }

  // Read LPDDR MR5 and MR8 info
  Status = ShowLpddrInfo (MrcData);
  if (Status != mrcSuccess) {
    return Status;
  }

  // Program DllWeaklock bit after training, when CKE is high
  if (Inputs->WeaklockEn && (!Inputs->SafeMode)) {
    MrcWeaklockEnDis (MrcData, MRC_ENABLE);
  }

  GetSetVal = 0;
  MrcGetSetDdrIoGroupController0 (MrcData, GsmIocDisDataIdlClkGate, WriteCached, &GetSetVal);

  // Enable Scrambling
  if (Inputs->ScramblerSupport == TRUE) {
    DdrScramble.Data          = 0;
    DdrScramble.Bits.ScramEn  = 1;
    DdrScramble.Bits.ClockGateAB = NS;
    DdrScramble.Bits.ClockGateC  = NS;
    DdrScramble.Bits.EnableDbiAB = NS;
    for (i = 0; i < ARRAY_COUNT (ScramblerRegOffsets); i++) {
      MrcCall->MrcGetRandomNumber (&GeneratedSeed);
      DdrScramble.Bits.ScramKey = GeneratedSeed;
      MrcWriteCR (MrcData, ScramblerRegOffsets[i], DdrScramble.Data);
    }
  }

  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (!(MrcChannelExist (Outputs, Channel))) {
        continue;
      }
      // Enable the command tri state at the end of the training.
      MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMccCmdTriStateDis, WriteNoCache, &GetSetDis);
      // Set the MC to ECC mode for all channels if needed.
      if (Outputs->EccSupport == TRUE) {
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ECC support\n");
        Offset = OFFSET_CALC_CH (MAD_INTRA_CH0_CNL_REG, MAD_INTRA_CH1_CNL_REG, Channel);
        MadIntra.Data      = MrcReadCR (MrcData, Offset);
        MadIntra.Bits.ECC = emBothActive;
        MrcWriteCR (MrcData, Offset, MadIntra.Data);
      }
      // Tell MC that we are in FSP-OP = 1
      if (Lpddr) {
        Offset = OFFSET_CALC_CH (CH0_CR_MC_INIT_STATE_CNL_REG, CH1_CR_MC_INIT_STATE_CNL_REG, Channel);
        McInitState.Data = MrcReadCR (MrcData, Offset);
        McInitState.Bits.LPDDR4_current_FSP = 1;
        MrcWriteCR (MrcData, Offset, McInitState.Data);
      }
    } // for Channel
  }

  // Update Odt timing, Samp timing and SlaveDLL to minimize power
  // Since LPDDR4 does not run TAT, we need to adjust Senseamp and ODT timings.
  if ((Inputs->TrainingEnables.TAT == 0) || Lpddr) {
    UpdateSampOdtTiming (MrcData, 1); // Use guardband of 1, as 0 is too aggressive sometimes
  }
  UpdateSlaveDLLLength (MrcData);

  // Program RxClkStgNum
  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (!MrcChannelExist (Outputs, Channel)) {
        continue;
      }
      // Read from Byte 0 as these should be the same across all bytes.
      MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, 0, GsmIocLpLongOdtEn, ReadFromCache, &LpddrLongOdtEn);
      MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, 0, GsmIocOdtSampExtEn, ReadFromCache, &OdtSampExtEn);
      MaxRcvEn   = 0;
      for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
        if (!MrcRankInChannelExist (MrcData, Rank, Channel)) {
          continue;
        }
        for (Byte = 0; Byte < Outputs->SdramCount; Byte++) {
          MrcGetSetDdrIoGroupStrobe (MrcData, Channel, Rank, Byte, RecEnDelay, ReadFromCache, &GetSetVal);
          Temp = (INT32) GetSetVal;
          Temp = Temp / 64;
          RcvEn = (UINT8) Temp;
          MaxRcvEn = MAX (MaxRcvEn, RcvEn);
        }
      }
      RcvEnDrift   = (Lpddr) ? (UINT8) ((tDQSCK_DRIFT + Outputs->Qclkps - 1) / Outputs->Qclkps) : 1;
      RcvEnTurnOff = MaxRcvEn + (5 - 6) + 1 + 7 + 3 + 3 + 2 + (2 * RcvEnDrift);
      if (LpddrLongOdtEn == 1) {
        RcvEnTurnOff++;
      }

      for (Byte = 0; Byte < Outputs->SdramCount; Byte++) {
        MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, Byte, McOdtDelay,     ReadFromCache, &OdtDelay);
        MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, Byte, McOdtDuration,  ReadFromCache, &OdtDuration);
        MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, Byte, GsmIocRxClkStg, ReadFromCache, &RxClkStgNum);

        OdtTurnOff = (INT8) (OdtDelay + OdtDuration + 14);
        OdtTurnOff = MIN (OdtTurnOff, DDRDATA_CR_DDRCRDATACONTROL2_CNL_RxClkStgNum_MAX);

        RxClkStgNum = (OdtSampExtEn == 1) ? MAX (RxClkStgNum, RcvEnTurnOff) : MAX (17, OdtTurnOff);
        MrcGetSetDdrIoGroupChannelStrobe (MrcData, Channel, Byte, GsmIocRxClkStg, WriteCached, &RxClkStgNum);
      }
    }
  }

  // Calculate the DDRIO RdDataValid per channel/rank and save the max value.
  // This is based on the longest RxFIFO timing in the system.
  MaxRdDataValid = MrcIntData->MaxRdDataValid;
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    for (SubCh = 0; SubCh < MAX_SUB_CHANNEL; SubCh++) {
      if (!MrcSubChannelExist (MrcData, Channel, SubCh)) {
        continue;
      }
      MrcGetSetDdrIoGroupChSch (MrcData, Channel, SubCh, RxFifoRdEnTclDelay,      ReadFromCache, &GetSetVal);
      RdDataValid = (UINT16) GetSetVal;
      MrcGetSetDdrIoGroupChSch (MrcData, Channel, SubCh, RxDqDataValidDclkDelay,  ReadFromCache, &GetSetVal);
      RdDataValid += (UINT16) GetSetVal;

      RxFifoRdEnRank = 0;
      for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
        if (!MrcRankInChannelExist (MrcData, Rank, Channel)) {
          continue;
        }
        MrcGetSetDdrIoGroupChSchRnk (MrcData, Channel, SubCh, Rank, RxFifoRdEnFlybyDelay, ReadFromCache, &GetSetVal);
        RxFifoRdEnRank = MAX (GetSetVal, RxFifoRdEnRank);
      } // Rank
      RdDataValid += (UINT16) RxFifoRdEnRank;
      MaxRdDataValid = MAX(MaxRdDataValid, RdDataValid);
    } // For SubCh
  } // For Channel
  MrcIntData->MaxRdDataValid = MAX(MaxRdDataValid, MrcIntData->MaxRdDataValid);
  // If we're at the last point, or SAGV is not enabled, program the RX Grace counter
  if ((MrcIntData->SaGvPoint == MrcSaGvPointHigh) || (MrcData->Inputs.SaGv != MrcSaGvEnabled)) {
    MiscControl2.Data = MrcReadCR (MrcData, DDRSCRAM_CR_DDRMISCCONTROL2_CNL_REG);
       MiscControl2.Bits.rx_analogen_grace_cnt = (MrcIntData->MaxRdDataValid + RX_ANALOG_GRACE_COUNT_GUARDBAND);
    MrcWriteCR (MrcData, DDRSCRAM_CR_DDRMISCCONTROL2_CNL_REG, MiscControl2.Data);
  }

  MrcFlushRegisterCachedData (MrcData);

  // @todo: <CNL> Enable New Probless Features Here

  // Oem hook when normal mode configuration is done
  MrcInternalCheckPoint (MrcData, OemAfterNormalMode, NULL);
#ifdef UP_SERVER_FLAG
  if(Inputs->BoardType != btUpServer){
#endif
    if (Inputs->UserPowerWeightsEn == 0) {
      // Apply power weight values
// @todo: <CNL> Update  MrcPowerWeight (MrcData);
    }
#ifdef UP_SERVER_FLAG
  } else{
    if ((Inputs->UserPowerWeightsEn == 0) ||(Inputs->UserBudgetEnable == 0) || (Inputs->TsodManualEnable == 0) || (Inputs->UserThresholdEnable == 0)   ) {
      // Apply Budget, Power Weights , Thermal Threshold, TSOD register values according to DIMM(s) features
      MrcUpPowerWeightAndTsod (MrcData);
    }
  }
#endif

// @todo: <CNL> Update  MrcBerSetup (MrcData);

  // Enable Self Refresh
  GetSetVal = 1;
  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccEnableSr, WriteNoCache, &GetSetVal);

  return mrcSuccess;
}

/**
  This function enables Normal Mode and configures the Power Down Modes.
  We also have special flow here for SAGV in S3/Warm boot modes.

  @param[in] MrcData - The MRC general data.

  @retval Always returns mrcSuccess.
**/
MrcStatus
MrcNormalMode (
  IN MrcParameters *const MrcData
  )
{
  const MRC_FUNCTION  *MrcCall;
  const MrcInput      *Inputs;
  MRC_BOOT_MODE       BootMode;
  INT64               GetSetVal;
  UINT32              Timeout;
  UINT8               Channel;
  UINT32              Offset;
  UINT32              SecondCbit;
  UINT32              MrParams[MAX_CHANNEL];
  BOOLEAN             SaGvAndSelfRefresh;
  BOOLEAN             Sagv;
  BOOLEAN             Flag;
  BOOLEAN             MemInSr;
  REUT_CH_SEQ_CFG_0_CNL_STRUCT          ReutChSeqCfg;
  STALL_DRAIN_CNL_STRUCT                StallDrain;
  CH0_CR_SCHED_SECOND_CBIT_CNL_STRUCT   SchedSecondCbit;
  CH0_CR_DDR_MR_PARAMS_CNL_STRUCT       DdrMrParams;

  Inputs     = &MrcData->Inputs;
  MrcCall    = Inputs->Call.Func;
  BootMode   = Inputs->BootMode;
  Sagv       = (Inputs->SaGv == MrcSaGvEnabled);
  MemInSr    = (BootMode == bmWarm) || (BootMode == bmS3);
  SecondCbit = 0;

  MrcCall->MrcSetMem ((UINT8 *) MrParams, sizeof (MrParams), 0);
  // Check if SAGV is enabled and memory is in Self-Refresh right now (Warm reset or S3 resume)
  SaGvAndSelfRefresh = Sagv && MemInSr;

  if (Sagv && !MemInSr) {
    MrcDramStateFinalize (MrcData);
  }

  if (SaGvAndSelfRefresh) {
     //
     // Disable MR4 reads before going to Normal Mode (which will exit SelfRefresh):
     // 1. SCHED_SECOND_CBIT.dis_srx_mr4 = 1
     // 2. SCHED_SECOND_CBIT.dis_SRX_MRS_MR4 = 1
     // 3. DDR_MR_PARAMS.MR4_PERIOD = 0
     // 4. DDR_MR_PARAMS.DDR4_TS_readout_en = 0
     //
     SchedSecondCbit.Data = MrcReadCR(MrcData, CH0_CR_SCHED_SECOND_CBIT_CNL_REG);
     SecondCbit = SchedSecondCbit.Data;                    // Backup the current value
     SchedSecondCbit.Bits.dis_srx_mr4 = 1;
     SchedSecondCbit.Bits.dis_SRX_MRS_MR4 = 1;
     MrcWriteCrMulticast(MrcData, BC_CR_SCHED_SECOND_CBIT_CNL_REG, SchedSecondCbit.Data);

     for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
       Offset = OFFSET_CALC_CH (CH0_CR_DDR_MR_PARAMS_CNL_REG, CH1_CR_DDR_MR_PARAMS_CNL_REG, Channel);
       DdrMrParams.Data = MrcReadCR(MrcData, Offset);
       MrParams[Channel] = DdrMrParams.Data;               // Backup the current value
       DdrMrParams.Bits.MR4_PERIOD = 0;
       DdrMrParams.Bits.DDR4_TS_readout_en = 0;
       MrcWriteCR(MrcData, Offset, DdrMrParams.Data);
     }
}

  // Set Normal Operation Mode
  // Write to both channels, we need to set this even if a channel is not populated, due to MC requirements for SAGV.
  ReutChSeqCfg.Data                     = 0;
  ReutChSeqCfg.Bits.Initialization_Mode = NOP_Mode;
  MrcWriteCR64 (MrcData, REUT_CH_SEQ_CFG_0_CNL_REG, ReutChSeqCfg.Data);
  MrcWriteCR64 (MrcData, REUT_CH_SEQ_CFG_1_CNL_REG, ReutChSeqCfg.Data);

  // Poll until STALL_DRAIN_CNL_STRUCT.sr_state becomes zero (DDR is not in self-refresh)
  Timeout = (UINT32) MrcCall->MrcGetCpuTime (MrcData) + 10000; // 10 seconds timeout
  do {
    StallDrain.Data = MrcReadCR (MrcData, STALL_DRAIN_CNL_REG);
    Flag            = (StallDrain.Bits.sr_state == 1);
  } while (Flag && ((UINT32) MrcCall->MrcGetCpuTime (MrcData) < Timeout));

  if (Flag) {
    return mrcFail;
  }

  if (SaGvAndSelfRefresh) {
    // Program DRAM MRs to match the High point
    MrcProgramMrsFsm (MrcData, 0x3);  // Run FSM on both channels in parallel
    MrcWriteCrMulticast(MrcData, BC_CR_SCHED_SECOND_CBIT_CNL_REG, SecondCbit);
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      Offset = OFFSET_CALC_CH (CH0_CR_DDR_MR_PARAMS_CNL_REG, CH1_CR_DDR_MR_PARAMS_CNL_REG, Channel);
      MrcWriteCR(MrcData, Offset, MrParams[Channel]);
    }
  }

  // Configure Power Down CR
  MrcPowerDownConfig (MrcData);

  // Ensure that pure_srx must be cleared so for FSM's to work.
  GetSetVal = 0;
  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccPureSrx, WriteNoCache, &GetSetVal);

  return mrcSuccess;
}

/**
  Clear Delta DQS before switching SA GV point

  @param[in] MrcData - include all the MRC general data.
**/
void
MrcClearDeltaDqs (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput                         *Inputs;
  const MRC_FUNCTION                     *MrcCall;
  MrcOutput                              *Outputs;
  MrcDebug                               *Debug;
  UINT32                                 Offset;
  UINT32                                 Timeout;
  UINT8                                  Rank;
  BOOLEAN                                Busy;
  MCMISCS_DELTADQSCOMMON0_CNL_STRUCT     DeltaDqsCommon0;
  DDRDATA_CR_DELTADQSRANK0_CNL_STRUCT    DeltaDqsRank;
  DDRSCRAM_CR_DDRMISCCONTROL0_CNL_STRUCT DdrMiscControl0;

  Inputs  = &MrcData->Inputs;
  Outputs = &MrcData->Outputs;
  Debug   = &Outputs->Debug;
  MrcCall = Inputs->Call.Func;

  Offset = MCMISCS_DELTADQSCOMMON0_CNL_REG;
  DeltaDqsCommon0.Data = MrcReadCR (MrcData, Offset);
  if (DeltaDqsCommon0.Bits.Lp4DeltaDQSTrainMode == 1) {
    Timeout = (UINT32) MrcCall->MrcGetCpuTime (MrcData) + 10000; // 10 seconds timeout
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Clear Lp4DeltaDQSTrainMode\n");
    DeltaDqsCommon0.Bits.Lp4DeltaDQSTrainMode = 0;
    MrcWriteCR (MrcData, Offset, DeltaDqsCommon0.Data);
    MrcWait (MrcData, 5 * HPET_1US);
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Clear DeltaDQS registers\n");
    for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
      Offset = DDRDATA_CR_DELTADQSRANK0_CNL_REG +
               ((DDRDATA_CR_DELTADQSRANK1_CNL_REG - DDRDATA_CR_DELTADQSRANK0_CNL_REG) * Rank);
      if (Rank == 3) {
        Offset += (DDRDATA_CR_DELTADQSRANK1_CNL_REG - DDRDATA_CR_DELTADQSRANK0_CNL_REG);
      }
      DeltaDqsRank.Data = 0;
      MrcWriteCR (MrcData, Offset, DeltaDqsRank.Data);
    } // for Rank
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Force DeltaDQS update\n");
    Offset = DDRSCRAM_CR_DDRMISCCONTROL0_CNL_REG;
    DdrMiscControl0.Data = MrcReadCR (MrcData, Offset);
    DdrMiscControl0.Bits.ForceDeltaDQSUpdate = 1;
    MrcWriteCR (MrcData, Offset, DdrMiscControl0.Data);
    // Wait for DeltaDQS Update to complete
    // Poll until DDRSCRAM_CR_DDRMISCCONTROL0_CNL_STRUCT.Bits.ForceDeltaDQSUpdate '0'
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Waiting for DeltaDQS Update to complete\n");
    do {
      DdrMiscControl0.Data = MrcReadCR (MrcData, Offset);
      Busy = (DdrMiscControl0.Bits.ForceDeltaDQSUpdate == 1);
    } while (Busy && ((UINT32) MrcCall->MrcGetCpuTime (MrcData) < Timeout));
    if (Busy) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "%s DeltaDQS Update did not to complete\n", gErrString);
    }
  }
}

/**
  Local function called by MrcSetOverrides if MrcSafeConfig is TRUE.
  This function will override MRC Inputs based on current safe configuration.
  Some configurations will be based on DDR type.

  @param[in]  MrcData - Pointer to global MRC data.

  @retval - mrcSuccess
**/
MrcStatus
MrcSafeMode (
  IN  MrcParameters *const  MrcData
  )
{
  MrcInput    *Inputs;

  Inputs  = &MrcData->Inputs;

  Inputs->EnablePwrDn       = 1;
  Inputs->EnablePwrDnLpddr  = 1;
  Inputs->PowerDownMode     = 0;    // Disables CKE Power Down Mode
  Inputs->RhPrevention      = FALSE;
  Inputs->SafeMode          = TRUE;

  Inputs->TrainingEnables.EWRTC2D     = 0;
  Inputs->TrainingEnables.ERDTC2D     = 0;
  Inputs->TrainingEnables.WRDSEQT     = 0;
  Inputs->TrainingEnables.CMDVC       = 0;
  Inputs->TrainingEnables.RTL         = 0;
  Inputs->TrainingEnables.TAT         = 0;
  Inputs->TrainingEnables.MEMTST      = 0;
  Inputs->TrainingEnables.ALIASCHK    = 0;
  Inputs->TrainingEnables.RMC         = 1;
  Inputs->TrainingEnables.WRDSUDT     = 0;

  return mrcSuccess;
}

/**
  SA GV flow for the cold boot

  @param[in] MrcData - include all the MRC general data.

  @retval mrcStatus if succeeded.
**/
MrcStatus
MrcSaGvSwitch (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput      *Inputs;
  MrcDebug            *Debug;
  const MRC_FUNCTION  *MrcCall;
  MrcIntOutput        *MrcIntData;
  MrcOutput           *Outputs;
  MrcStatus           Status;
  INT64               GetSetVal;
  UINT32              Timeout;
  UINT32              FreqIndex;
  BOOLEAN             Busy;

  Inputs  = &MrcData->Inputs;
  Outputs = &MrcData->Outputs;
  MrcCall = Inputs->Call.Func;
  Debug   = &Outputs->Debug;
  Status  = mrcSuccess;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));

  // At this point the MC is in Normal mode with Refreshes enabled
  Timeout = (UINT32) MrcCall->MrcGetCpuTime (MrcData) + 10000; // 10 seconds timeout

  // Save the current point
  GetSetVal = 1;
  FreqIndex = MrcIntData->SaGvPoint;
  MrcGetSetDdrIoGroupFreqIndex (MrcData, FreqIndex, GsmMccSaveFreqPoint, WriteNoCache, &GetSetVal);
  // Poll for acknowledgment
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Waiting for SAGV %s point save acknowledge\n", gFreqPointStr[FreqIndex]);
  do {
    MrcGetSetDdrIoGroupFreqIndex (MrcData, FreqIndex, GsmMccSaveFreqPoint, ReadNoCache, &GetSetVal);
    Busy = (GetSetVal == 1);
  } while (Busy && ((UINT32) MrcCall->MrcGetCpuTime (MrcData) < Timeout));

  if (Busy) {
    return mrcDeviceBusy;
  }

  return Status;
}

/**
  SA GV flow for the Fixed mode.

  @param[in] MrcData  - include all the MRC general data.
  @param[in] SaGvMode - The SAGV mode to be fixed to.

  @retval mrcStatus if succeeded.
**/
MrcStatus
MrcSetSaGvFixed (
  IN  MrcParameters *const MrcData,
  IN  MrcSaGv              SaGvMode
  )
{
  const MrcInput      *Inputs;
  MrcDebug            *Debug;
  const MRC_FUNCTION  *MrcCall;
  MrcOutput           *Outputs;
  MrcStatus           Status;
  UINT32              Data32;
  UINT32              MailboxStatus;

  Inputs      = &MrcData->Inputs;
  Outputs     = &MrcData->Outputs;
  MrcCall     = Inputs->Call.Func;
  Debug       = &Outputs->Debug;
  Status      = mrcSuccess;


  // Set the fixed point via CPU Mailbox
  // Assumption here that MrcSaGv values for FixedLow, FixedMed,
  // and FixedHigh match the CPU_MAILBOX command encoding.
  Data32 = SaGvMode;
  MrcCall->MrcCpuMailboxWrite (MAILBOX_TYPE_PCODE, CPU_MAILBOX_CMD_SAGV_SET_POLICY, Data32, &MailboxStatus);
  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "CPU_MAILBOX_CMD_SAGV_SET_POLICY %s. MailboxStatus = %Xh\n",
    (MailboxStatus == PCODE_MAILBOX_CC_SUCCESS) ? "success" : "failed",
    MailboxStatus
    );

  return Status;
}

/**
  Energy Performance Gain.

  @param[in]  MrcData - Pointer to the MRC global data structure

  @retval - Status.
**/
MrcStatus
MrcEnergyPerfGain (
  IN MrcParameters *const MrcData
  )
{
  MrcStatus           Status;
  const MrcInput      *Inputs;
  MrcIntOutput        *MrcIntData;
  MrcCpuModel         CpuModel;
  /*
  UINT32              MailboxStatus;
  UINT8               Index;
  UINT16              MilliWatt[2];
  UINT16              Data[2];
  */

  Inputs   = &MrcData->Inputs;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));

  CpuModel    = Inputs->CpuModel;

  Status  = mrcSuccess;

  if ((Inputs->SaGv != MrcSaGvEnabled) || (MrcIntData->SaGvPoint == MrcSaGvPointHigh)) {
    // Check for processor ID.
    if ((CpuModel == cmCNL_ULX_ULT) || (CpuModel == cmCNL_DT_HALO)) {
      /*
      // If enabled, set up EPG.
      if (Inputs->EpgEnable == 1) {
        for (Index = 0; Index < 2; Index++) {
          // Calculate the 3 parameters (mW):  (Idd3x * Vdd * number of DIMMs present in the system) / 1000.
          MilliWatt[Index] = (UINT16) ((((Index == 0) ? (Inputs->Idd3n) : (Inputs->Idd3p))
              * (Outputs->VddVoltage[Inputs->MemoryProfile]) * (Outputs->Controller[0].Channel[0].DimmCount
              + Outputs->Controller[0].Channel[1].DimmCount)) / 1000);
          // Convert to fixed point 8.8 value.  Integer8.8 = (milliwatts / 1000) * 2^8
          Data[Index] = MilliWatt[Index] * 256 / 1000;
          // Write to mailbox register.
          if (MrcCall->MrcCpuMailboxWrite != NULL) {
            MrcCall->MrcCpuMailboxWrite (MAILBOX_TYPE_PCODE,
              ((Index == 0) ? SET_EPG_BIOS_POWER_OVERHEAD_0_CMD : SET_EPG_BIOS_POWER_OVERHEAD_1_CMD),
              Data[Index], &MailboxStatus);
            MRC_DEBUG_MSG (
              Debug,
              MSG_LEVEL_NOTE,
              "SET_EPG_BIOS_POWER_OVERHEAD_%d_CMD %s. Value = %08Xh. MailboxStatus = %Xh\n",
              Index,
              (MailboxStatus == PCODE_MAILBOX_CC_SUCCESS) ? "success" : "failed",
              Data[Index],
              MailboxStatus);
          }  // Write to mailbox register.
        }  // for loop
      }  // EpgEnable
      */
    }  // processor ID
  }
  return Status;
}

/**
  this function is the last function that call from the MRC core.
    the function set DISB and set the MRC_Done.

  @param[in] MrcData - include all the MRC general data.

  @retval Always returns mrcSuccess.
**/
MrcStatus
MrcDone (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput        *Inputs;
  const MRC_FUNCTION    *MrcCall;
  MrcDebug              *Debug;
  MrcSaGv               SaGv;
  INT64                 GetSetVal;
  UINT32                Timeout;
  BOOLEAN               Flag;
  M_COMP_PCU_CNL_STRUCT MCompPcu;

  Inputs  = &MrcData->Inputs;
  Debug   = &MrcData->Outputs.Debug;
  MrcCall = Inputs->Call.Func;
  SaGv    = Inputs->SaGv;

  //if SAGV mode is fixed to one mode
  if ((SaGv != MrcSaGvDisabled) && (SaGv != MrcSaGvEnabled)) {
    MrcSetSaGvFixed (MrcData, SaGv);
  }

  // Not Used in CNL: MrcBerActivate (MrcData);

  GetSetVal = 1;
  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccEnableRefresh, WriteNoCache, &GetSetVal);

  // used to know what is the state of the boot mode.
  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccPuMrcDone, WriteNoCache, &GetSetVal);

  MrcGetSetDdrIoGroupController0 (MrcData, GsmMccMrcDone, WriteNoCache, &GetSetVal);

  // lock the MC and memory map registers.
  McRegistersLock (MrcData);

  // Wait for mc_init_done_ack
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Waiting for mc_init_done Acknowledge\n");

  Timeout = (UINT32) MrcCall->MrcGetCpuTime (MrcData) + 10000; // 10 seconds timeout

  do {
    MrcGetSetDdrIoGroupController0 (MrcData, GsmMccMcInitDoneAck, ReadNoCache, &GetSetVal);
    Flag = (GetSetVal == 0);
  } while (Flag && ((UINT32) MrcCall->MrcGetCpuTime (MrcData) < Timeout));
  if (Flag) {
    return mrcFail;
  }

  // Enable Periodic Comp with periodic interval = 10uS*2^COMP_INT
  MCompPcu.Data               = 0;
  MCompPcu.Bits.COMP_INTERVAL = COMP_INT;
  MrcWriteCR (MrcData, M_COMP_PCU_CNL_REG, MCompPcu.Data);

  // We use this register to indicate "Training done"
  MrcWmRegSetBits(MrcData, SSKPD_PCU_CNL_SKPD_TRAIN_DONE);

  return mrcSuccess;
}

/**
  Print the MRC version to the MRC output device.

  @param[in] Debug   - Pointer to the MRC Debug structure.
  @param[in] Version - The MRC version.

  @retval Nothing.
**/
void
MrcVersionPrint (
  IN MrcParameters     *MrcData,
  IN const MrcVersion  *Version
  )
{
#ifdef MRC_DEBUG_PRINT
  MrcDebug *Debug;

  Debug    = &MrcData->Outputs.Debug;
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "*********************************************************************\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Copyright (c) 2011- 2018 Intel Corporation. All rights reserved. **\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Cannonlake memory detection and initialization code.            **\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Major version number is:   %2u                                   **\n", Version->Major);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Minor version number is:   %2u                                   **\n", Version->Minor);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Rev version number is:     %2u                                   **\n", Version->Rev);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "** Build number is:           %2u                                   **\n", Version->Build);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "*********************************************************************\n");
#endif
  return;
}

/**
  This function return the MRC version.

  @param[in] MrcData - Include all MRC global data.
  @param[out] Version - Location to store the MRC version.

  @retval Nothing.
**/
void
MrcVersionGet (
  IN  const MrcParameters *const MrcData,
  OUT MrcVersion *const Version
  )
{
  const MrcInput     *Inputs;
  const MRC_FUNCTION *MrcCall;

  if (Version != NULL) {
    Inputs  = &MrcData->Inputs;
    MrcCall = Inputs->Call.Func;
    MrcCall->MrcCopyMem ((UINT8 *) Version, (UINT8 *) &cVersion[Inputs->CpuFamily], sizeof (MrcVersion));
  }
}

/**
  This function set the MRC vertion to MCDECS_SPARE register.
  The function need to be call by the wrapper after MrcStartMemoryConfiguration function where the MC CLK enable.
  The function write:
  Major number to bits 16-23
  Minor number to bits 8-15
  Build number to bits 0 - 7

  @param[in] MrcData - Include all MRC global data.

  @retval Nothing.
**/
MrcStatus
MrcSetMrcVersion (
  IN     MrcParameters *const MrcData
  )
{
  MrcVersion const         *Version;
  MRC_REVISION_CNL_STRUCT  MrcRevision;

  Version = &MrcData->Outputs.Version;
  MrcRevision.Data = (((UINT32) Version->Major) << 24) |
                     (((UINT32) Version->Minor) << 16) |
                     (((UINT32) Version->Rev)   << 8)  |
                     (((UINT32) Version->Build));

  MrcWriteCR (MrcData, MRC_REVISION_CNL_REG, MrcRevision.Data);
  return mrcSuccess;
}

/**
  This function locks the memory controller and memory map registers.

  @param[in] MrcData - Include all MRC global data.

  @retval Nothing.
**/
void
McRegistersLock (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput                                 *Inputs;
  const MRC_FUNCTION                             *MrcCall;
  MrcDebug                                       *Debug;
  UINT32                                         Offset;
  UINT32                                         PciEBaseAddress;
  TOM_0_0_0_PCI_CNL_STRUCT                       Tom;
  TOLUD_0_0_0_PCI_CNL_STRUCT                     Tolud;
  TOUUD_0_0_0_PCI_CNL_STRUCT                     Touud;
  BDSM_0_0_0_PCI_CNL_STRUCT                      Bdsm;
  BGSM_0_0_0_PCI_CNL_STRUCT                      Bgsm;
  GGC_0_0_0_PCI_CNL_STRUCT                       Ggc;
  DPR_0_0_0_PCI_CNL_STRUCT                       Dpr;
  DDR_PTM_CTL_PCU_CNL_STRUCT                     DdrPtmCtl;
  MCHBAR_IMRIAEXCBASE_MCHBAR_CBO_INGRESS_STRUCT  ImrIaExcBase;
  MCHBAR_IMRIAEXCBASE_MCHBAR_CBO_INGRESS_STRUCT  ImrIaExcLimit;


  Debug   = &MrcData->Outputs.Debug;
  Inputs  = &MrcData->Inputs;
  MrcCall = Inputs->Call.Func;
  PciEBaseAddress = Inputs->PciEBaseAddress;

  // Lock PRMRR.  Convert from MB to Address.
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MrcSetLockPrmrr: 0x%x\n", MrcCall->MrcSetLockPrmrr);
  MrcCall->MrcSetLockPrmrr (MrcData->Outputs.MemoryMapData.PrmrrBase << 20, Inputs->PrmrrSize << 20);

  // Lock the memory map registers.
  // Lock TOM.
  Offset        = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, TOM_0_0_0_PCI_CNL_REG);
  Tom.Data32[0] = MrcCall->MrcMmioRead32 (Offset);
  Tom.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Tom.Data32[0]);

  // Lock TOLUD.
  Offset          = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, TOLUD_0_0_0_PCI_CNL_REG);
  Tolud.Data      = MrcCall->MrcMmioRead32 (Offset);
  Tolud.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Tolud.Data);

  // Lock TOUUD.
  Offset          = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, TOUUD_0_0_0_PCI_CNL_REG);
  Touud.Data32[0] = MrcCall->MrcMmioRead32 (Offset);
  Touud.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Touud.Data32[0]);

  // Lock DPR register
  Offset        = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, DPR_0_0_0_PCI_CNL_REG);
  Dpr.Data      = MrcCall->MrcMmioRead32 (Offset);
  Dpr.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Dpr.Data);

  // Lock BDSM.
  Offset         = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, BDSM_0_0_0_PCI_CNL_REG);
  Bdsm.Data      = MrcCall->MrcMmioRead32 (Offset);
  Bdsm.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Bdsm.Data);

  // Lock BGSM.
  Offset         = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, BGSM_0_0_0_PCI_CNL_REG);
  Bgsm.Data      = MrcCall->MrcMmioRead32 (Offset);
  Bgsm.Bits.LOCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Bgsm.Data);

  // Lock GGC.
  Offset          = PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, GGC_0_0_0_PCI_CNL_REG);
  Ggc.Data        = (UINT16) MrcCall->MrcMmioRead32 (Offset);
  Ggc.Bits.GGCLCK = 1;
  MrcCall->MrcMmioWrite32 (Offset, Ggc.Data);

  // Lock POWER THERMAL MANAGEMENT CONTROL
  DdrPtmCtl.Data                   = MrcReadCR (MrcData, DDR_PTM_CTL_PCU_CNL_REG);
  DdrPtmCtl.Bits.LOCK_PTM_REGS_PCU = Inputs->LockPTMregs;
  MrcWriteCR (MrcData, DDR_PTM_CTL_PCU_CNL_REG, DdrPtmCtl.Data);

  // Lock ImrIaExcBase
  ImrIaExcBase.Data = MrcReadCR64 (MrcData, IMRIAEXCBASE_MCHBAR_CBO_INGRESS_REG);
  ImrIaExcBase.Bits.LOCK  = 1;
  MrcWriteCR64 (MrcData, IMRIAEXCBASE_MCHBAR_CBO_INGRESS_REG, ImrIaExcBase.Data);

  // Lock ImrIaExcLimit
  ImrIaExcLimit.Data = MrcReadCR64 (MrcData, IMRIAEXCLIMIT_MCHBAR_CBO_INGRESS_REG);
  ImrIaExcLimit.Bits.LOCK  = 1;
  MrcWriteCR64 (MrcData, IMRIAEXCLIMIT_MCHBAR_CBO_INGRESS_REG, ImrIaExcLimit.Data);

  // Lock ImrGtExcBase
  ImrIaExcBase.Data = MrcReadCR64 (MrcData, IMRGTEXCBASE_MCHBAR_CBO_INGRESS_REG);
  ImrIaExcBase.Bits.LOCK  = 1;
  MrcWriteCR64 (MrcData, IMRGTEXCBASE_MCHBAR_CBO_INGRESS_REG, ImrIaExcBase.Data);

  // Lock ImrGtExcLimit
  ImrIaExcLimit.Data = MrcReadCR64 (MrcData, IMRGTEXCLIMIT_MCHBAR_CBO_INGRESS_REG);
  ImrIaExcLimit.Bits.LOCK  = 1;
  MrcWriteCR64 (MrcData, IMRGTEXCLIMIT_MCHBAR_CBO_INGRESS_REG, ImrIaExcLimit.Data);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\nMemory map registers are locked\n");

  return;
}

/**
  This function returns the recommended MRC boot mode.

  @param[in] MrcData - The global host structure

  @retval bmWarm if we are in self refresh and the DISB bit is set, otherwise returns bmCold.
**/
MRC_BOOT_MODE
MrcGetBootMode (
  IN MrcParameters * const MrcData
  )
{
  MRC_BOOT_MODE BootMode;
  MrcInput     *Inputs;
  MRC_FUNCTION *MrcCall;
  UINT32       RegisterVal32;

  Inputs    = &MrcData->Inputs;
  MrcCall   = Inputs->Call.Func;

  RegisterVal32 = MrcCall->MrcMmioRead32((UINTN)PCH_PWRM_BASE_ADDRESS + R_CNL_PCH_PWRM_GEN_PMCON_A);
  if ((((RegisterVal32 & B_CNL_PCH_PWRM_GEN_PMCON_A_MEM_SR_MRC) != 0) && ((RegisterVal32 & B_CNL_PCH_PWRM_GEN_PMCON_A_DISB_MRC) != 0)) ||
      ((MrcWmRegGet(MrcData) & SSKPD_PCU_CNL_SKPD_MEM_TEST_DONE) == SSKPD_PCU_CNL_SKPD_MEM_TEST_DONE)) {
    BootMode = bmWarm;
  } else {
    BootMode = bmCold;
  }

  return BootMode;
}

/**
  This function sets the DISB bit in General PM Configuration.

  @param[in] MrcData - The global host structure

  @retval Nothing.
**/
void
MrcSetDISB (
  IN MrcParameters *const MrcData
  )
{
  MrcInput      *Inputs;
  MRC_FUNCTION  *MrcCall;
  UINT32        RegOffset;
  UINT8         RegisterVal;
  UINT8         Rw1cBits;

  Inputs    = &MrcData->Inputs;
  MrcCall   = Inputs->Call.Func;

  RegOffset = PCH_PWRM_BASE_ADDRESS + R_CNL_PCH_PWRM_GEN_PMCON_A;
  // The bits we care about are between Bit 16-23.  There are many RW/1C bits which we would like to avoid.
  // Thus we will read/write a UINT8 at the register offset +2 Bytes.
  RegOffset += 2;

  RegisterVal = MrcCall->MrcMmioRead8 (RegOffset);

  // Bit 23 is DISB.  So we want to set this.  Bit 23 of the register is bit 7 at RegOffset+2
  RegisterVal |=  MRC_BIT7;

  // Bit 18 and Bit 16 are RW/1C.  So we will set these bits to 0 before writing the register.
  // Bit 18 is Bit 2 of the Byte.
  // Bit 16 is Bit 0 of the Byte.
  Rw1cBits = MRC_BIT0 | MRC_BIT2;
  // Invert the mask so RW/1C bits are 0 and the rest are 1's.  Then AND this with the value to be written.
  Rw1cBits = ~Rw1cBits;
  RegisterVal &= Rw1cBits;

  MrcCall->MrcMmioWrite8 (RegOffset, RegisterVal);
}

/**
  This function resets the DISB bit in General PM Configuration.

  @param[in] MrcData - The global host structure

  @retval Nothing.
**/
void
MrcResetDISB (
  IN MrcParameters *const MrcData
  )
{
  MrcInput      *Inputs;
  MRC_FUNCTION  *MrcCall;
  UINT32        RegOffset;
  UINT8         RegisterVal;
  UINT8         Rw1cBits;

  Inputs    = &MrcData->Inputs;
  MrcCall   = Inputs->Call.Func;

  RegOffset = PCH_PWRM_BASE_ADDRESS + R_CNL_PCH_PWRM_GEN_PMCON_A;
  // The bits we care about are between Bit 16-23.  There are many RW/1C bits which we would like to avoid.
  // Thus we will read/write a UINT8 at the register offset +2 Bytes.
  RegOffset += 2;

  RegisterVal = MrcCall->MrcMmioRead8 (RegOffset);

  // Bit 18 and Bit 16 are RW/1C.  So we will set these bits to 0 before writing the register.
  // Bit 18 is Bit 2 of the Byte.
  // Bit 16 is Bit 0 of the Byte.
  Rw1cBits = MRC_BIT0 | MRC_BIT2;
  // Bit 23 is DISB.  So we want to clear this.  Bit 23 of the register is bit 7 at RegOffset+2
  Rw1cBits |=  MRC_BIT7;
  // Invert the mask so RW/1C bits and DISB are 0 and the rest are 1's.  Then AND this with the value to be written.
  Rw1cBits = ~Rw1cBits;

  RegisterVal &= Rw1cBits;

  MrcCall->MrcMmioWrite8 (RegOffset, RegisterVal);
}

/**
  This function reads the CAPID0 register and sets the memory controller's capability.

  @param[in, out] MrcData - All the MRC global data.

  @retval Returns mrcSuccess if the memory controller's capability has been determined, otherwise returns mrcFail.
**/
MrcStatus
MrcMcCapability (
  IN OUT MrcParameters *const MrcData
  )
{
  const MrcInput            *Inputs;
  MrcDebug                  *Debug;
  const MRC_FUNCTION        *MrcCall;
  MrcOutput                 *Outputs;
  MrcControllerOut          *ControllerOut;
  MrcChannelOut             *ChannelOut;
  MrcDimmOut                *DimmOut;
  MrcDdrType                DdrType;
  MrcProfile                Profile;
  UINT32                    ChannelCount;
  UINT32                    DimmCount;
  UINT32                    Max;
  UINT32                    Size;
  UINT32                    ChannelNum;
  UINT32                    DimmNum;
  UINT32                    ChDimmCount;
  UINT32                    Offset;
  UINT16                    NModeMinimum;
  UINT8                     Controller;
  UINT8                     Channel;
  UINT8                     SubCh;
  UINT8                     Dimm;
  UINT8                     BytesPerSch;
  UINT8                     ByteStart;
  UINT8                     ByteEnd;
  BOOLEAN                   Cmd2N;
  BOOLEAN                   UlxUlt;
  BOOLEAN                   Lpddr4;
  BOOLEAN                   Ddr4;
  BOOLEAN                   EccSupport;
  BOOLEAN                   IgnoreNonEccDimm;
  const char                *StrDdrType;
  CAPID0_A_0_0_0_PCI_CNL_STRUCT Capid0A;
  CAPID0_C_0_0_0_PCI_CNL_STRUCT Capid0C;
  DEVEN_0_0_0_PCI_CNL_STRUCT    Deven;

  Inputs      = &MrcData->Inputs;
  MrcCall     = Inputs->Call.Func;
  Outputs     = &MrcData->Outputs;
  Debug       = &Outputs->Debug;
  ChDimmCount = MAX_DIMMS_IN_CHANNEL;
  Profile     = Inputs->MemoryProfile;
  DdrType     = Outputs->DdrType;
  UlxUlt      = (Inputs->CpuModel == cmCNL_ULX_ULT);
  Ddr4        = (DdrType == MRC_DDR_TYPE_DDR4);
  Lpddr4      = (DdrType == MRC_DDR_TYPE_LPDDR4);

  // Obtain the capabilities of the memory controller.
  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_A_0_0_0_PCI_CNL_REG);
  Capid0A.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_C_0_0_0_PCI_CNL_REG);
  Capid0C.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset     = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, DEVEN_0_0_0_PCI_CNL_REG);
  Deven.Data = MrcCall->MrcMmioRead32 (Offset);

  // Check that current DDR type is allowed on this CPU
  StrDdrType = NULL;
  if (Inputs->SimicsFlag == 0) {
    if (Lpddr4 && (Capid0C.Bits.LPDDR4_EN == 0)) {
      StrDdrType = gDdrTypeStr[MRC_DDR_TYPE_LPDDR4];
    } else if (Ddr4 && (Capid0C.Bits.DDR4_EN == 0)) {
      StrDdrType = gDdrTypeStr[MRC_DDR_TYPE_DDR4];
    }
  }

  if (StrDdrType != NULL) {
    // MRC detected a memory technology and CAPID value shows the memory tech is not supported by this CPU.
    // e.g. LPDDR4 memory detected but CPU only supports DDR4.
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "ERROR: %s is not supported on this CPU\n", StrDdrType);
    return mrcDimmNotSupport;
  }

  // Determine if the internal graphics engine is supported.
  if ((Capid0A.Bits.IGD == 0) && (Deven.Bits.D2EN > 0)) {
    Outputs->GraphicsStolenSize = Inputs->GraphicsStolenSize;
    Outputs->GraphicsGttSize    = Inputs->GraphicsGttSize;
    Outputs->GtPsmiRegionSize   = Inputs->GtPsmiRegionSize;
  } else {
    Outputs->GraphicsStolenSize = 0;
    Outputs->GraphicsGttSize    = 0;
    Outputs->GtPsmiRegionSize   = 0;
  }

  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "Memory allocated for IGD = %uMB and for GTT = %uMB.\n",
    Outputs->GraphicsStolenSize,
    Outputs->GraphicsGttSize
    );

  // Determine the maximum size of memory per channel, based on fuses.
  switch (Capid0A.Bits.DDRSZ) {
  case tcs32GB:
    Outputs->MrcTotalChannelLimit = (32 * 1024);
    break;

  case tcs8GB:
    Outputs->MrcTotalChannelLimit = (8 * 1024);
    break;

  case tcs4GB:
    Outputs->MrcTotalChannelLimit = (4 * 1024);
    break;

  case tcs2GB:
  default:
    Outputs->MrcTotalChannelLimit = (2 * 1024);
    break;
  }

  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "Maximum size of memory allowed on a channel = %uMB.\n",
    Outputs->MrcTotalChannelLimit
    );

  // Determine how many channels are supported on this memory controller,
  // based on fuse and how many channels have DIMMs installed.
  ChannelCount  = (Capid0A.Bits.PDCD == 0) ? MAX_CHANNEL : 1;
  DimmCount     = (Capid0A.Bits.DDPCD == 0) ? MAX_DIMMS_IN_CHANNEL : 1;

  if ((Inputs->Force1Dpc == TRUE) || UlxUlt) {
    // Only 1DPC is supported on ULX / ULT platform
    DimmCount = 1;
  }

  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "Number of channels supported = %u\nNumber of DIMMs per channel supported = %u\n",
    ChannelCount,
    DimmCount
    );

  // Determine the minimum NMode supported on this memory controller.
  NModeMinimum = (Capid0A.Bits.D1NM == 0) ? 1 : 2;

  // Determine the ECC capability of the memory controller.
  IgnoreNonEccDimm = (Capid0A.Bits.FDEE == 0) ? FALSE : TRUE;

  // Set EccSupport flag to TRUE if we must NOT ignore ECC DIMMs
  if (IgnoreNonEccDimm == TRUE) {
    Outputs->EccSupport = TRUE;
    EccSupport = TRUE; // FDEE has presedence over ECCDIS
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ALL DIMMs MUST be ECC capable\n");
  } else {
    EccSupport = ((Capid0A.Bits.ECCDIS > 0) || (Outputs->EccSupport == FALSE)) ? FALSE : TRUE;
  }
  // Now copy ECC and NMode information to the channel and DIMM results.
  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerOut = &Outputs->Controller[Controller];
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelOut = &ControllerOut->Channel[Channel];
      if (ChannelOut->Status == CHANNEL_PRESENT) {
        Cmd2N = (NModeMinimum == 2);
        if (Inputs->MemoryProfile == STD_PROFILE) {
          // Okay to use Outputs->Frequency here as we don't have frequency switching in DDR4,
          // and LPDDR4 doesn't support stretched commands.
          if ((Ddr4 && (Outputs->Frequency > f2133)) ||
              (Ddr4 && !UlxUlt && (Outputs->Frequency >= f1333))
             ) {
            Cmd2N = TRUE;
          }
        }
        if (Cmd2N) {
          ChannelOut->Timing[Profile].NMode = MAX (2, ChannelOut->Timing[Profile].NMode);
        }
        for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
          DimmOut = &ChannelOut->Dimm[Dimm];
          if (DimmOut->Status == DIMM_PRESENT) {
            DimmOut->Timing[Profile].NMode = ChannelOut->Timing[Profile].NMode;
            MRC_DEBUG_MSG (
              Debug,
              MSG_LEVEL_NOTE,
              "  %s %u/%u/%u NMode = %u\n",
              CcdString,
              Controller,
              Channel,
              Dimm,
              DimmOut->Timing[Profile].NMode
            );
            if (EccSupport == TRUE) {
              if ((DimmOut->EccSupport == FALSE) && (IgnoreNonEccDimm == TRUE)) {
                DimmOut->Status = DIMM_DISABLED;
                MRC_DEBUG_MSG (
                  Debug,
                  MSG_LEVEL_NOTE,
                  "  %s %u/%u/%u Disabling non-ECC capable DIMM\n",
                  CcdString,
                  Controller,
                  Channel,
                  Dimm
                  );
              } else if (DimmOut->EccSupport == TRUE) {
                DimmOut->EccSupport = TRUE;
                DimmOut->SdramCount = MAX_SDRAM_IN_DIMM;
              } else {
                DimmOut->SdramCount = MAX_SDRAM_IN_DIMM - 1;
                Outputs->EccSupport = FALSE; // Final ECCSupport must be disabled if one DIMM is NOT capable
              }
            } else {
              DimmOut->EccSupport = FALSE;
              DimmOut->SdramCount = MAX_SDRAM_IN_DIMM - 1;
              Outputs->EccSupport = FALSE; // Final ECCSupport must be disabled if ECCDIS is set
            }
          }
        }
      }
    }
  }

  // Make sure we have the same NMode limit on both channels
  Cmd2N = FALSE;
  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerOut = &Outputs->Controller[Controller];
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (ControllerOut->Channel[Channel].Timing[Profile].NMode == 2) {
        Cmd2N = TRUE;
        break;
      }
    }
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ControllerOut->Channel[Channel].Timing[Profile].NMode = (Cmd2N) ? 2 : 1;
    }
  }

  // Update Final SdramCount
  Outputs->SdramCount = (Outputs->EccSupport == TRUE) ? MAX_SDRAM_IN_DIMM : (MAX_SDRAM_IN_DIMM - 1);

  // Determine the number of SubChannels the chip supports.
  // Determine the populated Bytes per Channel.
  Outputs->SubChCount = MAX_SUB_CHANNEL;
  ControllerOut       = &Outputs->Controller[CONTROLLER_0];
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    ChannelOut = &ControllerOut->Channel[Channel];
    BytesPerSch  = Outputs->SdramCount / Outputs->SubChCount;
    for (SubCh = 0; SubCh < Outputs->SubChCount; SubCh++) {
      ByteStart  = BytesPerSch * SubCh;
      ByteEnd    = ByteStart + BytesPerSch - 1;
      // ECC Byte is attached to the last SubChannel.
      if ((SubCh == 1) && Outputs->EccSupport) {
        ByteEnd++;
      }

      ChannelOut->ByteStart[SubCh]  = ByteStart;
      ChannelOut->ByteEnd[SubCh]    = ByteEnd;
      if ((1 << SubCh) & ChannelOut->ValidSubChBitMask) {
        ChannelOut->ValidByteMask |= (((1 << (ByteEnd - ByteStart + 1)) - 1) << ByteStart);
      }
    }
  }
#ifdef MRC_DEBUG_PRINT
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Sub Channel:\t0\t1\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ByteStart:");
  for (SubCh = 0; SubCh < Outputs->SubChCount; SubCh++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\t%d", ControllerOut->Channel[0].ByteStart[SubCh]);
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\nByteEnd:");
  for (SubCh = 0; SubCh < Outputs->SubChCount; SubCh++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\t%d", ControllerOut->Channel[0].ByteEnd[SubCh]);
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\n");
#endif // MRC_DEBUG_PRINT

  // Determine the size of memory in each channel.
  // Also determine the channel with the largest amount.
  Max = ChannelNum = Outputs->MemoryMapData.TotalPhysicalMemorySize = 0;
  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerOut = &Outputs->Controller[Controller];
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelOut = &ControllerOut->Channel[Channel];
      Size        = 0;
      if (ChannelOut->Status == CHANNEL_PRESENT) {
        for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
          DimmOut = &ChannelOut->Dimm[Dimm];
          if (DimmOut->Status == DIMM_PRESENT) {
            Size += DimmOut->DimmCapacity;
          }
        }

        ChannelOut->Capacity = Size;
        if (Size > Max) {
          Max         = Size;
          ChannelNum  = Channel;
          ChDimmCount = ChannelOut->DimmCount;
        } else if ((Size == Max) && (DimmCount == 1)) {
          // Choose channel with least amount of DIMMs if 2DPC is disabled
          if (ChannelOut->DimmCount < ChDimmCount) {
            ChDimmCount = ChannelOut->DimmCount;
            ChannelNum  = Channel;
          }
        }
      }

      Outputs->MemoryMapData.TotalPhysicalMemorySize += ChannelOut->Capacity;
    }
  }

  if (ChannelCount == 1) {
    // Determine which channels are supported on this memory controller.
    // If fused for one channel, we pick the channel with the most memory.
    for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
      ControllerOut = &Outputs->Controller[Controller];
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        ChannelOut = &ControllerOut->Channel[Channel];
        if ((ChannelOut->Status == CHANNEL_PRESENT) && (Channel != ChannelNum)) {
          // Disable Channel don't skip DIMM capacity
          MrcChannelDisable (MrcData, (UINT8) Channel, 0);
        }
      }

      MRC_DEBUG_MSG (
        Debug,
        MSG_LEVEL_NOTE,
        "Controller configured to one channel, we've selected channel %u.\n",
        ChannelNum
        );
    }
  }

  if (DimmCount == 1) {
    // Determine which DIMMs are supported on this memory controller.
    // If fused for one DIMM per channel, we pick the DIMM in a channel with the most memory.
    for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
      ControllerOut = &Outputs->Controller[Controller];
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        ChannelOut = &ControllerOut->Channel[Channel];
        Max                   = Size = DimmNum = 0;
        if (ChannelOut->Status == CHANNEL_PRESENT) {
          for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
            DimmOut = &ChannelOut->Dimm[Dimm];
            if (DimmOut->Status == DIMM_PRESENT) {
              Size = DimmOut->DimmCapacity;
              MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "C%uD%uDimmCapacity = 0x%x\n", Channel, Dimm, DimmOut->DimmCapacity);
              if (Size > Max) {
                Max     = Size;
                DimmNum = Dimm;
              }
            }
          }

          for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
            DimmOut = &ChannelOut->Dimm[Dimm];
            if ((DimmOut->Status == DIMM_PRESENT) && (Dimm != DimmNum)) {
              DimmOut->Status = DIMM_DISABLED;
            }
          }

          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_NOTE,
            "Controller configured to one DIMM per channel, we've selected channel %u, Dimm %u.\n",
            Channel,
            DimmNum
            );
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ValidRankBitMask = 0x%x\n", ChannelOut->ValidRankBitMask);
        }
      }
    }
  }

  // Now that we know the enabled and disabled DIMM/Channel population,
  // determine if all enabled DIMMS support ASR.
  // It is necessary to have all DIMMS in ASR or no DIMMS in ASR
  // when enabling 2x Refresh.
  if (Inputs->RhPrevention) {
    Outputs->AutoSelfRefresh = TRUE;
    for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
      ControllerOut = &Outputs->Controller[Controller];
      if (ControllerOut->Status == CONTROLLER_PRESENT) {
        for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
          ChannelOut = &ControllerOut->Channel[Channel];
          if (ChannelOut->Status == CHANNEL_PRESENT) {
            for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
              DimmOut = &ChannelOut->Dimm[Dimm];
              if ((DimmOut->Status == DIMM_PRESENT) && (DimmOut->AutoSelfRefresh == FALSE)) {
                MRC_DEBUG_MSG (
                  Debug,
                  MSG_LEVEL_NOTE,
                  "Channel %d, Dimm %d does not support Auto Self Refresh.  Disabling ASR since 2x Refresh is enabled!\n",
                  Channel,
                  Dimm
                  );
                Outputs->AutoSelfRefresh = FALSE;
              }
            }
          }
        }
      }
    }
  }

  return mrcSuccess;
}

/**
  This function reads the CAPID0 register and sets the following values
  according to memory controller's capability and user input:
    Outputs->RefClk
    Outputs->FreqMax
    Outputs->Capable100
    Outputs->MemoryClockMax

  @param[in, out] MrcData - All the MRC global data.

  @retval Always returns mrcSuccess.
**/
MrcStatus
MrcMcCapabilityPreSpd (
  IN OUT MrcParameters *const MrcData
  )
{
  const MrcInput                *Inputs;
  const MRC_FUNCTION            *MrcCall;
  MrcDebug                      *Debug;
  MrcOutput                     *Outputs;
  MrcIntOutput                  *IntOutputs;
  MrcFrequency                  FreqMax;
  MrcFrequency                  MidFreq;
  MrcRefClkSelect               RefClk;
  MrcClockRatio                 MaxRatio;
  MrcDdrType                    DdrType;
  BOOLEAN                       RefClk100En;
  UINT32                        MaxFreqCap;
  UINT32                        Offset;
  BOOLEAN                       Ddr4;
  BOOLEAN                       Lpddr4;
  BOOLEAN                       OverclockCapable;
  BOOLEAN                       DtHalo;
  CAPID0_A_0_0_0_PCI_CNL_STRUCT Capid0A;
  CAPID0_B_0_0_0_PCI_CNL_STRUCT Capid0B;
  CAPID0_C_0_0_0_PCI_CNL_STRUCT Capid0C;

  Inputs  = &MrcData->Inputs;
  MrcCall = Inputs->Call.Func;
  Outputs = &MrcData->Outputs;
  Debug   = &Outputs->Debug;
  IntOutputs = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));

  DdrType     = Outputs->DdrType;
  Ddr4        = (DdrType == MRC_DDR_TYPE_DDR4);
  Lpddr4      = (DdrType == MRC_DDR_TYPE_LPDDR4);
  DtHalo      = (Inputs->CpuModel == cmCNL_DT_HALO);

  // Obtain the capabilities of the memory controller.
  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_A_0_0_0_PCI_CNL_REG);
  Capid0A.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_B_0_0_0_PCI_CNL_REG);
  Capid0B.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_C_0_0_0_PCI_CNL_REG);
  Capid0C.Data = MrcCall->MrcMmioRead32 (Offset);

  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "CAPID0_A: %08Xh\nCAPID0_B: %08Xh\nCAPID0_C: %08Xh\n",
    Capid0A.Data,
    Capid0B.Data,
    Capid0C.Data
    );

  // Determine the maximum memory frequency supported and the memory reference clock.
  switch (DdrType) {
    case MRC_DDR_TYPE_DDR4:
      MaxFreqCap = Capid0C.Bits.MAX_F_DDR4;
      break;

    case MRC_DDR_TYPE_LPDDR4:
      MaxFreqCap = Capid0C.Bits.MAX_F_LPDDR4;
      break;

    default:
      MaxFreqCap = Capid0C.Bits.MAX_F_DDR4;
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "Invalid DDR Type detected. Using Type DDR4\n");
  }

  OverclockCapable = (Capid0A.Bits.DDR_OVERCLOCK > 0) ? TRUE : FALSE;
  RefClk100En       = (BOOLEAN) Capid0B.Bits.PLL_REF100_CFG;

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_OVERCLOCK: %d, PLL_REF100_CFG: %d\n", OverclockCapable, RefClk100En);

  Outputs->RefClk  = Inputs->RefClk;
  Outputs->FreqMax = ((Inputs->FreqMax > fNoInit) && (Inputs->FreqMax < fInvalid)) ? Inputs->FreqMax : MAX_FREQ_OC_133;

  if (!RefClk100En) {
    Outputs->RefClk = MRC_REF_CLOCK_133;
  }

  RefClk = Outputs->RefClk;
  if (OverclockCapable) {
    MaxFreqCap = 0;
    if (RefClk100En) {
      Outputs->Capable100 = TRUE;
    }
  }

  MaxRatio = (MrcClockRatio) ((MaxFreqCap == 0) ? 15 : MaxFreqCap); // Unlimited gives 15 * 266.7 + OddRatioEn = 4133
  FreqMax = MrcRatioToFrequency (MrcData, MaxRatio, RefClk, BCLK_DEFAULT);

  // If overclocking is supported, then there is no frequency limitation, otherwise check for limitation.
  // Note 1: If we are using standard memory profile, DIMMS should run at RefClk 133.
  if (Inputs->MemoryProfile == STD_PROFILE) {
    RefClk = MRC_REF_CLOCK_133;
    if (OverclockCapable) {
      FreqMax = MAX_FREQ_OC_133;
    }
  } else {
    if (OverclockCapable) {
      FreqMax = (RefClk == MRC_REF_CLOCK_100) ? MAX_FREQ_OC_100 : MAX_FREQ_OC_133;
    }
  }

  if (FreqMax < Outputs->FreqMax) {
    Outputs->FreqMax  = FreqMax;
    Outputs->RefClk   = RefClk;
  }

  if (IntOutputs->SaGvPoint == MrcSaGvPointLow) {
    // Set the LOW point for SA GV
    if (Inputs->FreqSaGvLow != 0) { // 0 means Auto
      Outputs->FreqMax = Inputs->FreqSaGvLow;
    } else {
      Outputs->FreqMax = (Ddr4) ? f1333 : f1067;
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SAGV Low point\n");
  } else if (IntOutputs->SaGvPoint == MrcSaGvPointMid) {
    // Set the MID point for SA GV
    if (Inputs->FreqSaGvMid != 0) { // 0 means Auto
      Outputs->FreqMax = Inputs->FreqSaGvMid;
    } else {
      MidFreq = (Ddr4) ? 2133 : 1867;
      Outputs->FreqMax = MIN (MidFreq, Outputs->FreqMax);
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SAGV Mid point\n");
  }
  if ((Inputs->FreqMax == 0) && (Lpddr4)){
    if (DtHalo) {
      Outputs->FreqMax = MIN (f2667, Outputs->FreqMax);
    } else if (Capid0C.Bits.MAX_F_LPDDR4 == 0xC) {
      Outputs->FreqMax = MIN (f1867, Outputs->FreqMax);
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Frequency is set with unlimited.. Limiting to %d\n", Outputs->FreqMax);
  }

  if ((Inputs->FreqMax == 0) && (Ddr4) && (Capid0C.Bits.MAX_F_DDR4 == 0xC)) {
    Outputs->FreqMax = MIN (f2133, Outputs->FreqMax);
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Frequency is set with unlimited.. Limiting to %d\n", Outputs->FreqMax);
  }
  Outputs->MemoryClockMax = ConvertFreq2Clock (MrcData, Outputs->FreqMax);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "The maximum memory frequency allowed is %u, tCK=%ufs\n", Outputs->FreqMax, Outputs->MemoryClockMax);
  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "%uMHz reference clock is selected\n",
    (Outputs->RefClk == MRC_REF_CLOCK_133) ? 133 : 100
    );

  if ((Inputs->DramDqOdt == MrcAuto) || (Inputs->DramDqOdt == MrcEnable)) {
    Outputs->DramDqOdtEn = TRUE;
  }
  else {
    Outputs->DramDqOdtEn = FALSE;
  }

  return mrcSuccess;
}

/**
  This function reads the input data structure and sets the appropriate overrides in the output structure.

  @param[in, out] MrcData - All the MRC global data.

  @retval Returns mrcSuccess if the timing overrides have been completed.
**/
MrcStatus
MrcSetOverrides (
  IN OUT MrcParameters *const MrcData
  )
{
  MrcInput      *Inputs;
  MrcOutput     *Outputs;
  MrcDimmOut    *DimmOut;
  MrcDebug      *Debug;
  const UINT16  *RcompTarget;
  MrcDdrType    DdrType;
  MrcStatus     Status;
  UINT16        ReqRdOdt;
  UINT16        ValidRdOdt;
  UINT8         Index;
  BOOLEAN       UlxUlt;
  BOOLEAN       Lpddr4;
  UINT8         Channel;
  UINT8         NumRanks;
  BOOLEAN       SagvEnabled;

  Inputs      = &MrcData->Inputs;
  Outputs     = &MrcData->Outputs;
  Debug       = &Outputs->Debug;
  DdrType     = Outputs->DdrType;
  Lpddr4      = (DdrType == MRC_DDR_TYPE_LPDDR4);
  UlxUlt      = (Inputs->CpuModel == cmCNL_ULX_ULT);
  Status      = mrcSuccess;
  RcompTarget = NULL;
  SagvEnabled = (Inputs->SaGv == MrcSaGvEnabled);

  Outputs->EccSupport      = Inputs->EccSupport != 0;
  Outputs->VddVoltageDone  = FALSE;
  // If RcompResistors are not zero, then user is overriding default termination
#ifdef MRC_DEBUG_PRINT
  if ((Inputs->RcompResistor[0] == 0) || (Inputs->RcompResistor[1] == 0) || (Inputs->RcompResistor[2] == 0)) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Updating Rcomp Resistors:\n");
  }
#endif // MRC_DEBUG_PRINT
  for (Index = 0; Index < MAX_RCOMP; Index++) {
    if (Inputs->RcompResistor[Index] == 0) {
      Inputs->RcompResistor[Index] = 100; // All platform designs default to 100 Ohm.
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " RcompResistor[%d]: %u\n", Index, Inputs->RcompResistor[Index]);
    }
  }

  // Determine RcompTargets based on CPU type and DDR Type
    switch (DdrType) {
      case MRC_DDR_TYPE_DDR4:
        RcompTarget = (UlxUlt) ? RcompTargetCnlUDdr4 : RcompTargetCnlHDdr4;
        break;

      case MRC_DDR_TYPE_LPDDR4:
        RcompTarget = (Outputs->Lp4x) ? RcompTargetCnlLpddr4x : RcompTargetCnlLpddr4;
        break;

      default:
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "%s %s: %d\n", gErrString, gUnsupportedTechnology, DdrType);
        Status = mrcFail;
        break;
    }
#ifdef MRC_DEBUG_PRINT
  if ((Inputs->RcompTarget[0] == 0) || (Inputs->RcompTarget[1] == 0) || (Inputs->RcompTarget[2] == 0) ||
      (Inputs->RcompTarget[3] == 0) || (Inputs->RcompTarget[4] == 0)) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Updating Rcomp Targets:\n");
  }
#endif // MRC_DEBUG_PRINT
  for (Index = 0; Index < MAX_RCOMP_TARGETS; Index++) {
    if ((Inputs->RcompTarget[Index] == 0) && (RcompTarget != NULL)) {
      Inputs->RcompTarget[Index] = RcompTarget[Index];
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " RcompTarget[%s]: %u\n", GlobalCompOffsetStr[Index], Inputs->RcompTarget[Index]);
    }
  }

  // Determine the IO ODT Termination mode:
  //  Technology  DT/HALO(2 DPC)  DT/HALO(1 DPC)  ULX/ULT
  //-----------------------------------------------------
  //  LP3           <DNE>           VTT             VTT
  //  Lp4           VSS             VSS             VSS
  //  Ddr4          VDDq            VDDq            VTT
  //  Default       CTT             CTT             CTT
  if (Outputs->OdtMode == MrcOdtModeDefault) {
    switch (DdrType) {
      case MRC_DDR_TYPE_DDR4:
        Outputs->OdtMode = MrcOdtModeVddq;
        break;

      case MRC_DDR_TYPE_LPDDR4:
        Outputs->OdtMode = MrcOdtModeVss;
        break;

      default:
        Outputs->OdtMode = MrcOdtModeCtt;
        break;
    }
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDRIO ODT Mode: %s\n", gIoOdtModeStr[Outputs->OdtMode]);

  // If we are LPDDR4, we need to check that the request CPU ODT matches one of the MR values for SOC_ODT.
  ReqRdOdt  = Inputs->RcompTarget[RdOdt];
  ValidRdOdt = MrcCheckForSocOdtEnc (MrcData, ReqRdOdt);
  if (ValidRdOdt != ReqRdOdt) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_WARNING, "RdOdt target of %d is not supported by the memory.  Updated to %d.  Please Update RcompTarget[RdOdt] to a correct value\n", ReqRdOdt, ValidRdOdt);
    Inputs->RcompTarget[RdOdt] = ValidRdOdt;
  }

  // ZQ Command Serialize Options:
  // MRC should auto detect 2 Rank x8 Device Width or any 4 Rank LPDDR4
  // and set Serialize_ZQ = 1.
  if (Lpddr4) {
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      if (!MrcChannelExist (Outputs, Channel)) {
        continue;
      }
      DimmOut = &Outputs->Controller[0].Channel[Channel].Dimm[dDIMM0];
      NumRanks = MrcGetRankInDimm (MrcData, Channel, dDIMM0);
      if (((DimmOut->SdramWidth == 8) && (NumRanks == 2)) || (NumRanks == 4)) {
        Inputs->SharedZqPin = 1;
      }
    }
  }

  Outputs->ODTCaCsCkSupport = ((Inputs->CpuModel == cmCNL_DT_HALO) && (Inputs->CpuStepping > csCnlP0)) ? TRUE : FALSE;
  // If we have ECT disabled, set the ECT done flag so Reset Flows will behave normally for LPDDR systems.
  if ((Inputs->TrainingEnables.ECT == 0) || (Inputs->SimicsFlag == 1)) {
    Outputs->LpddrEctDone = TRUE;
  }

  if (Lpddr4 && (((!UlxUlt) && (Inputs->CpuStepping == csCnlP0)) || (UlxUlt && (Inputs->CpuStepping < csCnlD0)))) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_WARNING, "Disabling Re-Training\n");
    Inputs->Lp4DqsOscEn = 0;
  }

  // Disable low supply enable for C0 and lower steppings in ULT_ULX and P0 and lower settings for Dt/Halo.
  if (!(Lpddr4 && ((UlxUlt && (Inputs->CpuStepping > csCnlC0)) || ((!UlxUlt) && (Inputs->CpuStepping > csCnlP0))))) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_WARNING, "Disabling Low Suppy Enable\n");
    Inputs->LowSupplyEnCcc = 0;
  }

  if ((Outputs->Lp4x) && (SagvEnabled) && (!Outputs->ODTCaCsCkSupport)) {
    Inputs->TrainingEnables.RDODTT = 0;
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_WARNING, "Disabling RxOdt on LP4x\n");
  }

  return Status;
}

/**
  This function get the current value of the sticky scratchpad register.

  @param[in] MrcData - include all the MRC data.

  @retval The current value of the sticky scratchpad register.

  **/
UINT64
MrcWmRegGet (
  IN     MrcParameters *const MrcData
  )
{
  return (MrcReadCR64 (MrcData, SSKPD_PCU_CNL_REG));
}

/**
This function Set a newvalue of the sticky scratchpad register by set new givin Bit(s)

@param[in] MrcData   - include all the MRC data.
@param[in] SskpdBits - Bit(s) Need to Set

@retval The current value of the sticky scratchpad register.

**/
void
MrcWmRegSetBits(
  IN     MrcParameters *const MrcData,
  IN     UINT64        SskpdBits
)
{
  UINT64 Sskpd;

  Sskpd = MrcReadCR64 (MrcData, SSKPD_PCU_CNL_REG);
  MrcWriteCR64(MrcData, SSKPD_PCU_CNL_REG, (Sskpd | SskpdBits));
}

/**
This function Set a newvalue of the sticky scratchpad register by clear givin Bit(s)

@param[in] MrcData   - include all the MRC data.
@param[in] SskpdBits - Bit(s) Need to Clear

@retval The current value of the sticky scratchpad register.

**/
void
MrcWmRegClrBits(
  IN     MrcParameters *const MrcData,
  IN     UINT64        SskpdBits
)
{
  UINT64 Sskpd;

  Sskpd = MrcReadCR64 (MrcData, SSKPD_PCU_CNL_REG);
  MrcWriteCR64(MrcData, SSKPD_PCU_CNL_REG, Sskpd & (~SskpdBits));
}

/**
  This function fills the scratch registers
  [DDRDATAxCHx_CR_DATATRAINFEEDBACK_REG] with MR data per rank and channel.
  These MRs are copied from the host structure.
  It also logs the final MR values and saves MR registers to MRS FSM for SA GV.

  ------------------- DDR4 ------------------------
  Note 1: DDR4 values are for Byte 0 only
  Note 2: MR5 is for DDR4 only.
  Per channel:
   DDRDATA0CHx_CR_DATATRAINFEEDBACK    [MR1_Rank0, MR0_Rank0] (31-16, 15-0)
   DDRDATA1CHx_CR_DATATRAINFEEDBACK    [MR1_Rank1, MR0_Rank1] (31-16, 15-0)
   DDRDATA2CHx_CR_DATATRAINFEEDBACK    [MR1_Rank2, MR0_Rank2] (31-16, 15-0)
   DDRDATA3CHx_CR_DATATRAINFEEDBACK    [MR1_Rank3, MR0_Rank3] (31-16, 15-0)
   DDRDATA4CHx_CR_DATATRAINFEEDBACK    [MR2_Rank1, MR2_Rank0] (31-16, 15-0)
   DDRDATA5CHx_CR_DATATRAINFEEDBACK    [MR2_Rank3, MR2_Rank2] (31-16, 15-0)
   DDRDATA6CHx_CR_DATATRAINFEEDBACK    [MR5_Rank1, MR5_Rank0] (31-16, 15-0)
   DDRDATA7CHx_CR_DATATRAINFEEDBACK    [MR5_Rank3, MR5_Rank2] (31-16, 15-0)

------------------- LPDDR4 ------------------------
  Per channel:
   DDRDATA0CHx_CR_DATATRAINFEEDBACK    [MR23_Rank0,      MR22_Rank0,      MR2_Rank0,        MR1_Rank0     ] (31-24, 23-16, 15-8, 7-0)
   DDRDATA1CHx_CR_DATATRAINFEEDBACK    [MR23_Rank1,      MR22_Rank1,      MR2_Rank1,        MR1_Rank1     ] (31-24, 23-16, 15-8, 7-0)
   DDRDATA2CHx_CR_DATATRAINFEEDBACK    [MR14_Rank0_Sch0, MR12_Rank0_Sch0, MR11_Rank0_Sch0,  MR3_Rank0_Sch0] (31-24, 23-16, 15-8, 7-0)
   DDRDATA3CHx_CR_DATATRAINFEEDBACK    [MR14_Rank1_Sch0, MR12_Rank1_Sch0, MR11_Rank1_Sch0,  MR3_Rank1_Sch0] (31-24, 23-16, 15-8, 7-0)
   DDRDATA4CHx_CR_DATATRAINFEEDBACK    [MR14_Rank0_Sch1, MR12_Rank0_Sch1, MR11_Rank0_Sch1,  MR3_Rank0_Sch1] (31-24, 23-16, 15-8, 7-0)
   DDRDATA5CHx_CR_DATATRAINFEEDBACK    [MR14_Rank1_Sch1, MR12_Rank1_Sch1, MR11_Rank1_Sch1,  MR3_Rank1_Sch1] (31-24, 23-16, 15-8, 7-0)

  @param[in] MrcData - The global host structure

  @retval mrcSuccess.
**/
MrcStatus
MrcFillMrScratchRegs (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput          *Inputs;
  MrcDebug                *Debug;
  MrcOutput               *Outputs;
  const MrcControllerOut  *ControllerOut;
  const MrcChannelOut     *ChannelOut;
  const MrcRankOut        *RankOut;
  MrcDdrType              DdrType;
  MRC_FUNCTION            *MrcCall;
  UINT32                  Offset;
  UINT32                  DclkPs;
  UINT8                   VrefCode;
  UINT8                   Rank;
  UINT8                   Byte;
  UINT8                   Channel;
  UINT8                   SubCh;
  UINT8                   RankMod2;
  UINT8                   Mr;
  BOOLEAN                 FsmSaved;
  BOOLEAN                 SaGv;
  BOOLEAN                 Lpddr4;

  DDRDATA0CH0_CR_DATATRAINFEEDBACK_CNL_STRUCT   DataTrainFeedback[3];
  CH0_CR_LPDDR_MR_CONTENT_CNL_STRUCT            LpddrMrContentCnl;
  CH0_CR_DDR4_MR0_MR1_CONTENT_CNL_STRUCT        Mr0Mr1Content;
  CH0_CR_DDR4_MR2_MR3_CONTENT_CNL_STRUCT        Mr2Mr3Content;
  CH0_CR_DDR4_MR4_MR5_CONTENT_CNL_STRUCT        Mr4Mr5Content;
  CH0_CR_DDR4_MR6_MR7_CONTENT_CNL_STRUCT        Mr6Mr7Content;
  CH0_CR_DDR4_MR6_VREF_VALUES_0_CNL_STRUCT      Mr6VrefValues;
  CH0_CR_MRS_FSM_CONTROL_CNL_STRUCT             MrsFsmControl;
  CH0_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_STRUCT Lpddr4DiscreteMrValue;
  CH0_CR_MCMNTS_SPARE_CNL_STRUCT                McmntsSpare;

#ifdef MRC_DEBUG_PRINT
  static const UINT8      Lpddr4MrRankIndex[4]    = { mrIndexMR1, mrIndexMR2, mrIndexMR22, mrIndexMR23 };
  static const UINT8      Lpddr4MrSubChIndex[4]   = { mrIndexMR3, mrIndexMR11, mrIndexMR12, mrIndexMR14 };
  static const UINT8      Lpddr4MrRankAddress[4]  = { mrMR1, mrMR2, mrMR22, mrMR23 };
  static const UINT8      Lpddr4MrSubChAddress[4] = { mrMR3, mrMR11, mrMR12, mrMR14 };
#endif

  Inputs        = &MrcData->Inputs;
  MrcCall       = MrcData->Inputs.Call.Func;
  Outputs       = &MrcData->Outputs;
  Debug         = &Outputs->Debug;
  DdrType       = Outputs->DdrType;
  ControllerOut = &Outputs->Controller[0];
  Lpddr4        = (DdrType == MRC_DDR_TYPE_LPDDR4);
  DclkPs        = Outputs->Qclkps * 2;
  MrsFsmControl.Data = 0;
  MrsFsmControl.Bits.do_ZQCL = (Lpddr4) ? 0 : 1;
  SaGv = (Inputs->SaGv == MrcSaGvEnabled);

  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    ChannelOut = &ControllerOut->Channel[Channel];
    FsmSaved = FALSE;
    if (Lpddr4) {
      for (Rank = 0; Rank < MAX_RANK_IN_DIMM; Rank++) {
        RankMod2 = Rank % 2;
        RankOut = &ChannelOut->Dimm[dDIMM0].Rank[RankMod2];
        if (MrcRankInChannelExist (MrcData, Rank, Channel)) {
#ifdef MRC_DEBUG_PRINT
          for (Mr = 0; Mr < ARRAY_COUNT(Lpddr4MrRankIndex); Mr++) {
            MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Ch %d Rank %d MR[%d] = 0x%04X\n", Channel, Rank, Lpddr4MrRankAddress[Mr], RankOut->MR[0][Lpddr4MrRankIndex[Mr]]);
          }
#endif
          // MR1, MR2, MR22 and MR23 - Per Rank
          DataTrainFeedback[0].Data8[0] = (UINT8) RankOut->MR[0][1];
          DataTrainFeedback[0].Data8[1] = (UINT8) RankOut->MR[0][2];
          DataTrainFeedback[0].Data8[2] = (UINT8) RankOut->MR[0][mrIndexMR22];
          DataTrainFeedback[0].Data8[3] = (UINT8) RankOut->MR[0][mrIndexMR23];
          Offset = MrcGetOffsetDataTrainFeedback (MrcData, Channel, Rank);
          MrcWriteCR (MrcData, Offset, DataTrainFeedback[0].Data);
        }

        for (SubCh = 0; SubCh < MAX_SUB_CHANNEL; SubCh++) {
          // MR3, MR11, MR12 and MR14 - Per SubCh, Per Rank
          DataTrainFeedback[1].Data8[0] = (UINT8) RankOut->MR[0][3];
          DataTrainFeedback[1].Data8[1] = (UINT8) RankOut->MR[0][mrIndexMR11];
          DataTrainFeedback[1].Data8[2] = (UINT8) RankOut->MR[0][mrIndexMR12];
          DataTrainFeedback[1].Data8[3] = (UINT8) RankOut->MR[0][mrIndexMR14];
          Offset = MrcGetOffsetDataTrainFeedback (MrcData, Channel, (2 * SubCh) + Rank + 2);
          MrcWriteCR (MrcData, Offset, DataTrainFeedback[1].Data);

          if (MrcSubChannelExist (MrcData, Channel, SubCh)) {
            if (MrcRankInChannelExist (MrcData, Rank, Channel)) {
#ifdef MRC_DEBUG_PRINT
              for (Mr = 0; Mr < ARRAY_COUNT(Lpddr4MrSubChIndex); Mr++) {
                MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Ch %d SubCh %d Rank %d MR[%d] = 0x%04X\n", Channel, SubCh, Rank, Lpddr4MrSubChAddress[Mr], RankOut->MR[0][Lpddr4MrSubChIndex[Mr]]);
              }
#endif
              Lpddr4DiscreteMrValue.Data = 0;
              Lpddr4DiscreteMrValue.Bits.PDDS     = (RankOut->MR[0][mrIndexMR3]  >> 3) & 0x7;
              Lpddr4DiscreteMrValue.Bits.DQ_ODT   =  RankOut->MR[0][mrIndexMR11]       & 0x7;
              Lpddr4DiscreteMrValue.Bits.CA_ODT   = (RankOut->MR[0][mrIndexMR11] >> 4) & 0x7;
              Lpddr4DiscreteMrValue.Bits.CA_VREF  =  RankOut->MR[0][mrIndexMR12]       & 0x7F;
              Lpddr4DiscreteMrValue.Bits.DQ_VREF  =  RankOut->MR[0][mrIndexMR14]       & 0x7F;
              Lpddr4DiscreteMrValue.Bits.CODT     =  RankOut->MR[0][mrIndexMR22]       & 0x7;
              Offset = CH0_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_REG +
                ((CH1_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_REG - CH0_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_REG) * Channel) +
                ((CH0_CR_LPDDR4_DISCRETE_MR_VALUES_2_CNL_REG - CH0_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_REG) * SubCh) +
                ((CH0_CR_LPDDR4_DISCRETE_MR_VALUES_1_CNL_REG - CH0_CR_LPDDR4_DISCRETE_MR_VALUES_0_CNL_REG) * Rank);
              MrcWriteCR (MrcData, Offset, Lpddr4DiscreteMrValue.Data);
            }
          }

          if (!FsmSaved) {
            LpddrMrContentCnl.Data = 0;
            LpddrMrContentCnl.Bits.MR1  = RankOut->MR[0][1];
            LpddrMrContentCnl.Bits.MR2  = RankOut->MR[0][2];
            LpddrMrContentCnl.Bits.MR3  = RankOut->MR[0][3];
            LpddrMrContentCnl.Bits.MR11 = RankOut->MR[0][mrIndexMR11];
            LpddrMrContentCnl.Bits.MR12 = RankOut->MR[0][mrIndexMR12];
            LpddrMrContentCnl.Bits.MR13 = RankOut->MR[0][mrIndexMR13];
            LpddrMrContentCnl.Bits.MR22 = RankOut->MR[0][mrIndexMR22];
            LpddrMrContentCnl.Bits.MR23 = RankOut->MR[0][mrIndexMR23];
            Offset = OFFSET_CALC_CH (CH0_CR_LPDDR_MR_CONTENT_CNL_REG, CH1_CR_LPDDR_MR_CONTENT_CNL_REG, Channel);
            MrcWriteCR64 (MrcData, Offset, LpddrMrContentCnl.Data);
            MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CH%d_LPDDR_MR_CONTENT = 0x%016llX\n", Channel, LpddrMrContentCnl.Data);

            MrsFsmControl.Bits.LPDDR_Restore_MR = ((Outputs->Lp4x) && (!Outputs->ODTCaCsCkSupport)) ? 0x17F : 0x1FF;

            if (Outputs->Lp4x && Outputs->ODTCaCsCkSupport) {
              Offset = OFFSET_CALC_CH (CH0_CR_MCMNTS_SPARE_CNL_REG, CH1_CR_MCMNTS_SPARE_CNL_REG, Channel);
              McmntsSpare.Data = MrcReadCR (MrcData, Offset);
              McmntsSpare.Bits.Spare_RW |= MRC_BIT0;
              MrcWriteCR (MrcData, Offset, McmntsSpare.Data);
            }

            MrsFsmControl.Bits.LPDDR4_switch_FSP = 1;
            MrsFsmControl.Bits.do_dq_osc_start   = (Inputs->Lp4DqsOscEn) ? 1 : 0;
            MrsFsmControl.Bits.tVREFDQ = DIVIDECEIL (MRC_LP4_tFC_LONG_NS * 1000, DclkPs);
            FsmSaved = TRUE;
          }
        } // For SubCh

        // Update MR22 with Rank1 value, In case of DIMM1 only, Rank2 value will be updated above
        if ((Outputs->Lp4x) && (Outputs->ODTCaCsCkSupport) && (Rank == 1)) {
          Offset = OFFSET_CALC_CH (CH0_CR_LPDDR_MR_CONTENT_CNL_REG, CH1_CR_LPDDR_MR_CONTENT_CNL_REG, Channel);
          LpddrMrContentCnl.Data = MrcReadCR64 (MrcData, Offset);
          LpddrMrContentCnl.Bits.MR22 = RankOut->MR[0][mrIndexMR22];
          MrcWriteCR64 (MrcData, Offset, LpddrMrContentCnl.Data);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CH%d_LPDDR_MR_CONTENT = 0x%016llX\n", Channel, LpddrMrContentCnl.Data);
        }
      } // For Rank
    } else { // DDR4
      for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
        RankMod2 = Rank % 2;
        RankOut = &ChannelOut->Dimm[RANK_TO_DIMM_NUMBER (Rank)].Rank[RankMod2];

        // MR0 - MR1
        for (Mr = 0; Mr < 2; Mr++) {
          DataTrainFeedback[0].Data16[Mr % 2] = RankOut->MR[0][Mr];
        }
        Offset = MrcGetOffsetDataTrainFeedback (MrcData, Channel, Rank);
        MrcWriteCR (MrcData, Offset, DataTrainFeedback[0].Data);

        // MR2
        DataTrainFeedback[1].Data16[RankMod2] = RankOut->MR[0][2];
        if (RankMod2) {
          Offset = MrcGetOffsetDataTrainFeedback (MrcData, Channel, 4 + (Rank / 2));
          MrcWriteCR (MrcData, Offset, DataTrainFeedback[1].Data);
        }

        // MR5
        DataTrainFeedback[2].Data16[RankMod2] = RankOut->MR[0][5];
        if (RankMod2) {
          Offset = MrcGetOffsetDataTrainFeedback (MrcData, Channel, 6 + (Rank / 2));
          MrcWriteCR (MrcData, Offset, DataTrainFeedback[2].Data);
        }
        // Per-device registers restore for DDR4 - for EV
        if (MrcRankInChannelExist (MrcData, Rank, Channel)) {
          Mr6VrefValues.Data = 0;
          for (Byte = 0; Byte < 8; Byte++) { // Only 8 bytes, no place for ECC byte
            VrefCode = RankOut->Ddr4PdaMr6[Byte] & 0x7F;
            Mr6VrefValues.Data |= MrcCall->MrcLeftShift64 ((UINT64) VrefCode, Byte * CH1_CR_DDR4_MR6_VREF_VALUES_0_CNL_Byte1_OFF);
            MRC_DEBUG_MSG (
              Debug,
              MSG_LEVEL_NOTE,
              "CH%d RANK%d BYTE%d: Vref offset = %d (0 = 820mV) Code = 0x%X \n",
              Channel,
              Rank,
              Byte,
              MrcVrefDqToOffsetDdr4 (VrefCode),
              VrefCode
            );
          }
          Offset = CH0_CR_DDR4_MR6_VREF_VALUES_0_CNL_REG +
            (CH1_CR_DDR4_MR6_VREF_VALUES_0_CNL_REG - CH0_CR_DDR4_MR6_VREF_VALUES_0_CNL_REG) * Channel +
            (CH0_CR_DDR4_MR6_VREF_VALUES_1_CNL_REG - CH0_CR_DDR4_MR6_VREF_VALUES_0_CNL_REG) * Rank;
          MrcWriteCR64 (MrcData, Offset, Mr6VrefValues.Data);
          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_NOTE,
            "CH%d RANK%d: DDR4_MR6_VREF_VALUES = 0x%08X%08X\n",
            Channel,
            Rank,
            Mr6VrefValues.Data32[1],
            Mr6VrefValues.Data32[0]
            );
        }

        if (!FsmSaved && MrcRankInChannelExist (MrcData, Rank, Channel)) {
          Mr0Mr1Content.Data = 0;
          Mr2Mr3Content.Data = 0;
          Mr4Mr5Content.Data = 0;
          Mr6Mr7Content.Data = 0;
          Mr0Mr1Content.Bits.MR0 = RankOut->MR[0][0];
          Mr0Mr1Content.Bits.MR1 = RankOut->MR[0][1];
          Mr2Mr3Content.Bits.MR2 = RankOut->MR[0][2];
          Mr2Mr3Content.Bits.MR3 = RankOut->MR[0][3];
          MrsFsmControl.Bits.DDR4_Restore_MR = 0xF; // Restore MR0..MR3

          Offset = CH0_CR_DDR4_MR0_MR1_CONTENT_CNL_REG +
            (CH1_CR_DDR4_MR0_MR1_CONTENT_CNL_REG - CH0_CR_DDR4_MR0_MR1_CONTENT_CNL_REG) * Channel;
          MrcWriteCR (MrcData, Offset, Mr0Mr1Content.Data);

          Offset = CH0_CR_DDR4_MR2_MR3_CONTENT_CNL_REG +
            (CH1_CR_DDR4_MR2_MR3_CONTENT_CNL_REG - CH0_CR_DDR4_MR2_MR3_CONTENT_CNL_REG) * Channel;
          MrcWriteCR (MrcData, Offset, Mr2Mr3Content.Data);
          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_NOTE,
            "CH%d: MR0_MR1_CONTENT = 0x%08X, MR2_MR3_CONTENT = 0x%08X\n",
            Channel,
            Mr0Mr1Content.Data,
            Mr2Mr3Content.Data
            );

          Mr4Mr5Content.Bits.MR4 = RankOut->MR[0][4];
          Mr4Mr5Content.Bits.MR5 = RankOut->MR[0][5];
          Mr6Mr7Content.Bits.MR6 = RankOut->MR[0][6];
          MrsFsmControl.Bits.DDR4_Restore_MR             = 0x7F; // Restore MR0..MR6
          MrsFsmControl.Bits.DDR4_Restore_MR6_Per_Device = (Outputs->Ddr4PdaEnable);
          MrsFsmControl.Bits.vref_time_per_byte          = (Outputs->Ddr4PdaEnable);
          MrsFsmControl.Bits.add_initial_vref            = (Outputs->Ddr4PdaEnable);
          MrsFsmControl.Bits.tVREFDQ = DIVIDECEIL (tVREF_DQ, DclkPs);
          Offset = CH0_CR_DDR4_MR4_MR5_CONTENT_CNL_REG +
            (CH1_CR_DDR4_MR4_MR5_CONTENT_CNL_REG - CH0_CR_DDR4_MR4_MR5_CONTENT_CNL_REG) * Channel;
          MrcWriteCR (MrcData, Offset, Mr4Mr5Content.Data);

          Offset = CH0_CR_DDR4_MR6_MR7_CONTENT_CNL_REG +
            (CH1_CR_DDR4_MR6_MR7_CONTENT_CNL_REG - CH0_CR_DDR4_MR6_MR7_CONTENT_CNL_REG) * Channel;
          MrcWriteCR (MrcData, Offset, Mr6Mr7Content.Data);
          MRC_DEBUG_MSG (
            Debug,
            MSG_LEVEL_NOTE,
            "CH%d: MR4_MR5_CONTENT = 0x%08X, MR6_MR7_CONTENT = 0x%08X\n",
            Channel,
            Mr4Mr5Content.Data,
            Mr6Mr7Content.Data
            );
          FsmSaved = TRUE;
        } // if !FsmSaved and rank exists
      } // for Rank
    } // DDR4

    if (SaGv) {
      Offset = OFFSET_CALC_CH (CH0_CR_MRS_FSM_CONTROL_CNL_REG, CH1_CR_MRS_FSM_CONTROL_CNL_REG, Channel);
      MrcWriteCR64 (MrcData, Offset, MrsFsmControl.Data);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CH%d_MRS_FSM_CONTROL = 0x%08X\n", Channel, MrsFsmControl.Data);
    }
  } // for Channel

#ifdef MRC_DEBUG_PRINT
  if (DdrType == MRC_DDR_TYPE_DDR4) {
    for (Rank = 0; Rank < MAX_RANK_IN_CHANNEL; Rank++) {
      if (((1 << Rank) & Outputs->ValidRankMask) == 0) {
        // Skip if this rank is not present on any of the channels
        continue;
      }
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Rank %d\tCh0\tCh1", Rank);
      RankMod2 = Rank % 2;
      for (Mr = 0; Mr <= 6; Mr++) {
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\nMR[%d]:", Mr);
        for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
          ChannelOut = &ControllerOut->Channel[Channel];
          RankOut = &ChannelOut->Dimm[RANK_TO_DIMM_NUMBER (Rank)].Rank[RankMod2];
          if (!MrcRankInChannelExist (MrcData, Rank, Channel)) {
            if (Channel == 0) {
              MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\t");
            }
            continue;
          }
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\t0x%04X", RankOut->MR[0][Mr]);
        } // for Channel
      } // for Mr
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\n");
    } // for Rank
  } // if DDR4
#endif

  return mrcSuccess;
}

/**
  Program BER Addresses if enabled

  @param[in] MrcData - The MRC general data.

**/
void
MrcBerSetup (
  IN     MrcParameters *const MrcData
  )
{
  const MRC_FUNCTION                        *MrcCall;
  const MrcInput                            *Inputs;
  MrcDebug                                  *Debug;
  MrcOutput                                 *Outputs;
  UINT8                                     MaxIndex;
  UINT8                                     MaxIndexMinus;
  UINT8                                     Index1;
  UINT8                                     Index2;
  UINT8                                     ValidAddress;
  UINT8                                     OverlappingAddresses;
  UINT64                                    Address;
  MrcStatus                                 Status;
  UINT64                                    Touud;
  UINT32                                    LowerMask;
  UINT32                                    UpperMask;
  UINT32                                    Data32;

  Inputs        = &MrcData->Inputs;
  MrcCall       = Inputs->Call.Func;
  Outputs       = &MrcData->Outputs;
  Debug         = &Outputs->Debug;

  MaxIndex = Inputs->BerEnable;
#ifdef MRC_DEBUG_PRINT
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "IN BER Enable : %u\n", MaxIndex);
  for (Index1 = 0; Index1 < 4; Index1++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "IN BER Address[%u] : 0x%x_%08x\n", Index1, (UINT32)(MrcCall->MrcRightShift64 (Inputs->BerAddress[Index1], 32)), (UINT32)(Inputs->BerAddress[Index1]));
  }
#endif //MRC_DEBUG_PRINT
  if (MaxIndex != 0) {
    MrcCall  = Inputs->Call.Func;
    OverlappingAddresses = 0;

    if (MaxIndex <= 4) {
      // Passed in addresses
      MrcCall->MrcCopyMem ((UINT8 *) Outputs->BerAddress, (UINT8 *) Inputs->BerAddress, sizeof (UINT64) * MaxIndex);
      ValidAddress = (MRC_BIT0 << MaxIndex) - 1;

      // Align to 64B cacheline
      for (Index1 = 0; Index1 < MaxIndex; Index1++) {
        Outputs->BerAddress[Index1] &= 0x07FFFFFFFC0ULL;
      }

      // Check for overlap addresses after cacheline alignment
      MaxIndexMinus = MaxIndex - 1;
      for (Index1 = 0; Index1 < MaxIndexMinus; Index1++) {
        for (Index2 = (Index1 + 1); Index2 < MaxIndex; Index2++) {
          if (Outputs->BerAddress[Index1] == Outputs->BerAddress[Index2]) {
            ValidAddress &= ~(MRC_BIT0 << Index1);
            OverlappingAddresses++;
            break;
          }
        }
      }

      // Check if within Memory Map
      for (Index1 = 0; Index1 < MaxIndex; Index1++) {
        if (ValidAddress & (MRC_BIT0 << Index1)) {
          if ((MrcBitErrRecAddressCompare (MrcData, Outputs->BerAddress[Index1])) == mrcFail) {
            ValidAddress &= ~(MRC_BIT0 << Index1);
            OverlappingAddresses++;
          }
        }
      }

      // Set # of BER Enable to match
      Outputs->BerEnable = MaxIndex - OverlappingAddresses;

      // Shift Addresses in array if there were overlapping addresses and have any valid addresses left
      if ((OverlappingAddresses) && (Outputs->BerEnable)) {
        for (Index1 = 0; Index1 < MaxIndexMinus; Index1++) {
          if (ValidAddress & (MRC_BIT0 << Index1)) {
            // ValidAddress, skip to next address
            continue;
          }
          for (Index2 = (Index1 + 1); Index2 < MaxIndex; Index2++) {
            if (ValidAddress & (MRC_BIT0 << Index2)) {
              // Shift Index2 address to Index1 location and adjust ValidAddress accordingly
              Outputs->BerAddress[Index1] = Outputs->BerAddress[Index2];
              ValidAddress |= (MRC_BIT0 << Index1);
              ValidAddress &= ~(MRC_BIT0 << Index2);
              break;
            }
          }
        }
      } //if (OverlappingAddresses)
    } else {
      // Randomize addresses and adjust MaxIndex to # of random addresses
      MaxIndexMinus = MaxIndex - 4;
      MaxIndex = MIN (MaxIndexMinus, 4);
      Outputs->BerEnable = MaxIndex;
      Touud = (UINT64)Outputs->MemoryMapData.TouudBase;
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "TOUUD_MB: 0x%x, ", (UINT32)Touud);
      Touud = MrcCall->MrcLeftShift64 (Touud, 20);
      Touud--;
      LowerMask = ((UINT32)Touud & 0xFFFFFFC0);
      UpperMask = (UINT32) (MrcCall->MrcRightShift64 (Touud, 32));
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Lower Mask: 0x%08x, UpperMask: 0x%08x\n", LowerMask, UpperMask);

      for (Index1 = 0; Index1 < MaxIndex; Index1++) {
        do {
          // TOUUD Max is bit 38 and Data32 is for upper 32bit of 64bit address (thus the mask of 0x07F)
          MrcCall->MrcGetRandomNumber (&Data32);
          Address = MrcCall->MrcLeftShift64 ((UINT64) (Data32 & UpperMask), 32);
          MrcCall->MrcGetRandomNumber (&Data32);
          Address |= (UINT64) (Data32 & LowerMask);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Random Address : 0x%x_%08x\n", (UINT32)(MrcCall->MrcRightShift64 (Address, 32)), (UINT32)(Address));

          Status = MrcBitErrRecAddressCompare (MrcData, Address);
        } while (Status == mrcFail);
        Outputs->BerAddress[Index1] = Address;
      }
    }
#ifdef MRC_DEBUG_PRINT
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "OUT BER Enable : %u\n", Outputs->BerEnable);
    for (Index1 = 0; Index1 < Outputs->BerEnable; Index1++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "OUT BER Address[%u] : 0x%x_%08x\n", Index1, (UINT32)(MrcCall->MrcRightShift64 (Outputs->BerAddress[Index1], 32)), (UINT32)(Outputs->BerAddress[Index1]));
    }
#endif //MRC_DEBUG_PRINT
  } else {
    Outputs->BerEnable = 0;
  }
}

/**
  Enable Bit Error Recovery feature

  @param[in] MrcData - The MRC general data.

**/
void
MrcBerActivate (
  IN  MrcParameters *const MrcData
  )
{
  const MRC_FUNCTION  *MrcCall;
  const MrcInput      *Inputs;
  MrcDebug            *Debug;
  MrcOutput           *Outputs;
  MrcMemoryMap        *MemoryMap;
  UINT32              Offset;
  UINT32              Index;
  UINT64              TomAddress;
  UINT64              RemapBaseAddress;
  BIT_ERROR_RECOVERY_SOURCE_SYSADDR_0_CNL_STRUCT   BerAddress;

  Inputs  = &MrcData->Inputs;
  MrcCall = Inputs->Call.Func;
  Outputs = &MrcData->Outputs;
  Debug   = &Outputs->Debug;

  if (Outputs->BerEnable) {
    // BER addresses
    MemoryMap = &Outputs->MemoryMapData;
    if (MemoryMap->ReclaimEnable) {
      TomAddress       = MrcCall->MrcLeftShift64 ((UINT64)(MemoryMap->TotalPhysicalMemorySize), 20);
      RemapBaseAddress = MrcCall->MrcLeftShift64 ((UINT64)(MemoryMap->RemapBase), 20);
    } else {
      TomAddress       = 0;
      RemapBaseAddress = 0;
    }

    for (Index = 0; Index < Outputs->BerEnable; Index++) {
      BerAddress.Data = Outputs->BerAddress[Index];

      // Check if BER Address is between below TOM and above Remap Base
      if ((MemoryMap->ReclaimEnable) && (BerAddress.Data < TomAddress) && (BerAddress.Data >= RemapBaseAddress)) {
        BerAddress.Bits.is_tcm = 1;
      } else {
        BerAddress.Bits.is_tcm = 0;
      }
      BerAddress.Bits.valid  = 1;

      Offset = BIT_ERROR_RECOVERY_SOURCE_SYSADDR_0_CNL_REG +
        ((BIT_ERROR_RECOVERY_SOURCE_SYSADDR_1_CNL_REG -
          BIT_ERROR_RECOVERY_SOURCE_SYSADDR_0_CNL_REG) * Index);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BERSSAR[%u] = 0x%x_%08x\n", Index,
        (UINT32)(MrcCall->MrcRightShift64 (BerAddress.Data, 32)), (UINT32)BerAddress.Data);
      MrcWriteCR64 (MrcData, Offset, BerAddress.Data);
    }
  }
}

/**
  Gets pointers to functions inside of core.

  @param[in]  MrcData                     - All the MRC global data.
  @param[out] CallChannelExist            - Pointer to the function MrcChannelExist
  @param[out] CallPrintf                  - Pointer to the function MrcPrintf
  @param[out] CallChangeMargin            - Pointer to the function ChangeMargin
  @param[out] CallSignExtend              - Pointer to the function MrcSignExtend
  @param[out] CallShiftPIforCmdTraining   - Pointer to the function ShiftPIforCmdTraining
  @param[out] CallMrcUpdateVref           - Pointer to the function MrcUpdateVref

  @retval Returns mrcSuccess if the function succeeds.
**/
MrcStatus
MrcGetCoreFunction (
  IN const MrcParameters *const MrcData,
  OUT UINTN                    *CallChannelExist,
  OUT UINTN                    *CallPrintf,
  OUT UINTN                    *CallChangeMargin,
  OUT UINTN                    *CallSignExtend,
  OUT UINTN                    *CallShiftPIforCmdTraining,
  OUT UINTN                    *CallMrcUpdateVref
  )
{
  *CallChannelExist            = (UINTN) &MrcChannelExist;
  *CallPrintf                  = (UINTN) &MrcPrintf;
  *CallChangeMargin            = (UINTN) &ChangeMargin;
  *CallSignExtend              = (UINTN) &MrcSE;
  *CallShiftPIforCmdTraining   = (UINTN) &ShiftPIforCmdTraining;
  *CallMrcUpdateVref           = (UINTN) &MrcUpdateVref;
  return (mrcSuccess);
}

/**
  Function returns TRUE if two DIMMs are populated in any of the channels else returns FLASE

  @param[in, out] MrcData - Include all MRC global data.

  @retval TRUE two DIMMs per channel are populated else FALSE
**/
BOOLEAN
Is2DPC (
  IN  MrcParameters *const MrcData
  )
{
  MrcInput              *Inputs;
  MrcControllerIn       *ControllerIn;
  MrcChannelIn          *ChannelIn;
  MRC_BOOT_MODE         BootMode;
  UINT8                 Controller;
  UINT8                 Channel;
  UINT8                 *Buffer1;
  UINT8                 *Buffer2;

  Inputs = &MrcData->Inputs;
  BootMode = Inputs->BootMode;
  if (BootMode == bmWarm || BootMode == bmS3) {
    return (MrcData->Save.Data.Any2Dpc ? TRUE : FALSE);
  }

  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerIn = &Inputs->Controller[Controller];
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelIn = &ControllerIn->Channel[Channel];
      if (ChannelIn->Status == CHANNEL_PRESENT) {
        Buffer1 = (UINT8 *) &ChannelIn->Dimm[0].Spd.Data;
        Buffer2 = (UINT8 *) &ChannelIn->Dimm[1].Spd.Data;
        if ((Buffer1[2] != 0) && (Buffer2[2] != 0)) {
          return (TRUE);
        }
      }
    }
  }
  return (FALSE);
}

#ifdef MRC_DEBUG_PRINT
/**
  Print the input parameters to the debug message output port.

  @param[in] MrcData - The MRC global data.

  @retval mrcSuccess
**/
MrcStatus
MrcPrintInputParameters (
  IN MrcParameters *const MrcData
  )
{
  MrcDebug                              *Debug;
  const MrcInput                        *Inputs;
  const MRC_FUNCTION                    *MrcCall;
  const MrcControllerIn                 *ControllerIn;
  const MrcChannelIn                    *ChannelIn;
  const MrcDimmIn                       *DimmIn;
  const MrcTiming                       *Timing;
  const TrainingStepsEn                 *TrainingSteps;
  const TrainingStepsEn2                *TrainingSteps2;
  const UINT8                           *Buffer;
  UINT16                                Line;
  UINT16                                Address;
  UINT16                                Offset;
  UINT8                                 Controller;
  UINT8                                 Channel;
  UINT8                                 Dimm;
  UINT8                                 Iteration;
  UINT32                                Index;
  CHAR8                                 HexDump[16 * 3 + 16 + 1];
  CHAR8                                 *p;
  UINT8                                 Data8;

  Inputs  = &MrcData->Inputs;
  MrcCall = Inputs->Call.Func;
  Debug   = &MrcData->Outputs.Debug;

  // The following are system level definitions. All memory controllers in the system are set to these values.
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Debug.Stream: %Xh\n", Debug->Stream);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Debug.Level: %Xh\n", Debug->Level);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "HeapBase: %08Xh\n", Inputs->HeapBase.DataN);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "HeapSize: %u\n", Inputs->HeapSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SerialBuffer: %08Xh\n", Inputs->SerialBuffer.DataN);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SerialBufferSize: %u\n", Inputs->SerialBufferSize);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "FreqMax: %u\n", Inputs->FreqMax);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "FreqSaGvLow: %u\nFreqSaGvMid: %u\n", Inputs->FreqSaGvLow, Inputs->FreqSaGvMid);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Ratio: %u\n", Inputs->Ratio);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RefClk: %uMHz\n", (Inputs->RefClk == MRC_REF_CLOCK_100) ? 100 : 133);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BClk: %uHz\n", Inputs->BClkFrequency);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "OddRatioMode: %u\n", Inputs->OddRatioMode);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MemoryProfile: %Xh\n", Inputs->MemoryProfile);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BoardType: %u\nCmdRanksTerminated: %Xh\n", Inputs->BoardType, Inputs->CmdRanksTerminated);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DqPinsInterleaved: %u\nSeparateCkeDelayDdr4: %u\n", Inputs->DqPinsInterleaved, Inputs->SeparateCkeDelayDdr4);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CpuModel: %Xh\nCpuStepping: %Xh\n", Inputs->CpuModel, Inputs->CpuStepping);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "GraphicsStolenSize: %Xh\n", Inputs->GraphicsStolenSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "GraphicsGttSize: %Xh\n", Inputs->GraphicsGttSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Iteration: %Xh\n", Inputs->Iteration);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "VddVoltage: %u mV\n", Inputs->VddVoltage);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BootMode: %Xh\n", Inputs->BootMode);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "TxtFlag: %Xh\nSimicsFlag: %Xh\n", Inputs->TxtFlag, Inputs->SimicsFlag);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MobilePlatform: %Xh\n", Inputs->MobilePlatform);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "EccSupport: %Xh\nCleanMemory: 0x%X\n", Inputs->EccSupport, Inputs->CleanMemory);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "GfxIsVersatileAcceleration: %Xh\n", Inputs->GfxIsVersatileAcceleration);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ScramblerSupport: %Xh\n", Inputs->ScramblerSupport);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RemapEnable: %Xh\n", Inputs->RemapEnable);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "PowerDownMode: %Xh\n", Inputs->PowerDownMode);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "PwdwnIdleCounter: %Xh\n", Inputs->PwdwnIdleCounter);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RankInterleave: %Xh\n", Inputs->RankInterleave);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "EnhancedInterleave: %Xh\n", Inputs->EnhancedInterleave);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "WeaklockEn: %Xh\n", Inputs->WeaklockEn);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "EnCmdRate: %Xh\n", Inputs->EnCmdRate);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BaseAddresses\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "  PciE: %Xh\n", Inputs->PciEBaseAddress);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "  MchBar: %Xh\n", Inputs->MchBarBaseAddress);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "  Smbus: %Xh\n", Inputs->SmbusBaseAddress);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "  Hpet: %Xh\n\n", Inputs->HpetBaseAddress);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MeStolenSize: %Xh\n", Inputs->MeStolenSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MmioSize: %Xh\n", Inputs->MmioSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "TsegSize: %Xh\n", Inputs->TsegSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "IedSize: %Xh\n", Inputs->IedSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DprSize: %Xh\n", Inputs->DprSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "PrmrrSize: %Xh\n", Inputs->PrmrrSize);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "VddSettleWaitTime: %Xh\n", Inputs->VddSettleWaitTime);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "VccIomV: %d\n", Inputs->VccIomV);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RmtPerTask: %d\nTrainTrace: %d\n", Inputs->RmtPerTask, Inputs->TrainTrace);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "AutoSelfRefreshSupport: %u\n", Inputs->AutoSelfRefreshSupport);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ExtTemperatureSupport: %u\n", Inputs->ExtTemperatureSupport);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RhPrevention: %u\n", Inputs->RhPrevention);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RhSolution: %Xh\n", Inputs->RhSolution);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RhActProbability: %Xh\nRetrainOnFastFail: %u\nForceOltmOrRefresh2x: %u\nLpDdrDqDqsReTraining (Lp4DqsOscEn) : %Xh\n",
    Inputs->RhActProbability, Inputs->RetrainOnFastFail, Inputs->ForceOltmOrRefresh2x, Inputs->Lp4DqsOscEn);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MrcSafeConfig: %Xh\nSafeMode: %Xh\n", Inputs->MrcSafeConfig, Inputs->SafeMode);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ChHashEnable: %Xh\n", Inputs->ChHashEnable);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ChHashMask: %Xh\n", Inputs->ChHashMask);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ChHashInterleaveBit: %Xh\nPerBankRefresh: %Xh\n", Inputs->ChHashInterleaveBit, Inputs->PerBankRefresh);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ProbelessTrace: %u\nMemoryTrace: %u\n", Inputs->ProbelessTrace, Inputs->MemoryTrace);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Force1Dpc: %u\n", Inputs->Force1Dpc);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ForceSingleRank: %u\n", Inputs->ForceSingleRank);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BER Enable: %u\n", Inputs->BerEnable);
  for (Offset = 0; Offset < 4; Offset++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BER Address[%u]: %X_%08Xh\n", Offset, (UINT32)(MrcCall->MrcRightShift64 (Inputs->BerAddress[Offset], 32)), (UINT32)(Inputs->BerAddress[Offset]));
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "StrongWkLeaker: %u\n", Inputs->StrongWkLeaker);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "VttCompForVsshi: %u\n", Inputs->VttCompForVsshi);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CaVrefConfig: %u\n", Inputs->CaVrefConfig);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "BdatEnable %u\nBdatTestType %Xh\n", Inputs->BdatEnable, Inputs->BdatTestType);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RMTLoopCount %Xh\n", Inputs->RMTLoopCount);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DllBwEn0: %u\nDllBwEn1: %u\nDllBwEn2: %u\nDllBwEn3: %u\n",
    Inputs->DllBwEn0, Inputs->DllBwEn1, Inputs->DllBwEn2, Inputs->DllBwEn3);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "PowerTrainingMode: %s optimization\n", Inputs->PowerTrainingMode ? "margin" : "power");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****    MRC TRAINING STEPS     *****\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  TrainingSteps = &Inputs->TrainingEnables;
  TrainingSteps2 = &Inputs->TrainingEnables2;
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.SOT: %u\n", TrainEnString, TrainingSteps->SOT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.ERDMPRTC2D: %u\n", TrainEnString, TrainingSteps->ERDMPRTC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.ECT: %u\n", TrainEnString, TrainingSteps->ECT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RCVET: %u\n", TrainEnString, TrainingSteps->RCVET);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDMPRT: %u\n", TrainEnString, TrainingSteps->RDMPRT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.JWRL: %u\n", TrainEnString, TrainingSteps->JWRL);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EWRTC2D: %u\n", TrainEnString, TrainingSteps->EWRTC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.ERDTC2D: %u\n", TrainEnString, TrainingSteps->ERDTC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRTC1D: %u\n", TrainEnString, TrainingSteps->WRTC1D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRVC1D: %u\n", TrainEnString, TrainingSteps->WRVC1D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDTC1D: %u\n", TrainEnString, TrainingSteps->RDTC1D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR4MAP: %u\n", Inputs->DDR4MAP);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.DIMMODTT: %u\n", TrainEnString, TrainingSteps->DIMMODTT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.DIMMRONT: %u\n", TrainEnString, TrainingSteps->DIMMRONT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRDSEQT: %u\n", TrainEnString, TrainingSteps->WRDSEQT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRDSUDT: %u\n", TrainEnString, TrainingSteps->WRDSUDT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRSRT: %u\n", TrainEnString, TrainingSteps->WRSRT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDODTT: %u\n", TrainEnString, TrainingSteps->RDODTT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDEQT: %u\n", TrainEnString, TrainingSteps->RDEQT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDAPT: %u\n", TrainEnString, TrainingSteps->RDAPT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.CMDVC: %u\n", TrainEnString, TrainingSteps->CMDVC);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRTC2D: %u\n", TrainEnString, TrainingSteps->WRTC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDTC2D: %u\n", TrainEnString, TrainingSteps->RDTC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WRVC2D: %u\n", TrainEnString, TrainingSteps->WRVC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDVC2D: %u\n", TrainEnString, TrainingSteps->RDVC2D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.LCT: %u\n", TrainEnString, TrainingSteps->LCT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RTL: %u\n", TrainEnString, TrainingSteps->RTL);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.TAT: %u\n", TrainEnString, TrainingSteps->TAT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RMT: %u\n", TrainEnString, TrainingSteps->RMT);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.MEMTST: %u\n", TrainEnString, TrainingSteps->MEMTST);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.ALIASCHK: %u\n", TrainEnString, TrainingSteps->ALIASCHK);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RCVENC1D: %u\n", TrainEnString, TrainingSteps->RCVENC1D);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RMC: %u\n", TrainEnString, TrainingSteps->RMC);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RDVC1D: %u\n%s.TXTCO: %u\n%s.CLKTCO: %u\n", TrainEnString, TrainingSteps2->RDVC1D,
                 TrainEnString, TrainingSteps2->TXTCO, TrainEnString, TrainingSteps2->CLKTCO);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****      MRC TIMING DATA      *****\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);

  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerIn = &Inputs->Controller[Controller];
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Controller[%u] ChannelCount: %Xh\n", Controller, ControllerIn->ChannelCount);
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelIn = &ControllerIn->Channel[Channel];
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Channel[%u].Status: %Xh\n", Channel, ChannelIn->Status);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Channel[%u].DimmCount: %Xh\n", Channel, ChannelIn->DimmCount);

      for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
        DimmIn = &ChannelIn->Dimm[Dimm];
        Timing = &DimmIn->Timing;
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u Status: %Xh\n", CcdString, Controller, Channel, Dimm, DimmIn->Status);
        if (Inputs->MemoryProfile == USER_PROFILE) {
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tCK    : %u\n", CcdString, Controller, Channel, Dimm, Timing->tCK);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u NMode  : %u\n", CcdString, Controller, Channel, Dimm, Timing->NMode);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tCL    : %u\n", CcdString, Controller, Channel, Dimm, Timing->tCL);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tCWL   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tCWL);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tFAW   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tFAW);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tRAS   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tRAS);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tRCDtRP: %u\n", CcdString, Controller, Channel, Dimm, Timing->tRCDtRP);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tREFI  : %u\n", CcdString, Controller, Channel, Dimm, Timing->tREFI);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tRFC   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tRFC);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tRRD   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tRRD);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tRTP   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tRTP);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tWR    : %u\n", CcdString, Controller, Channel, Dimm, Timing->tWR);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tWTR   : %u\n", CcdString, Controller, Channel, Dimm, Timing->tWTR);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tWTR_L : %u\n", CcdString, Controller, Channel, Dimm, Timing->tWTR_L);
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u tWTR_S : %u\n", CcdString, Controller, Channel, Dimm, Timing->tWTR_S);
        }
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s %u/%u/%u SpdAddress: %Xh\n", CcdString, Controller, Channel, Dimm, DimmIn->SpdAddress);
        Buffer = (UINT8 *) &DimmIn->Spd.Data;
        MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SPD:           00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        for (Line = 0; Line < (sizeof (MrcSpd) / 16); Line++) {
          Address = Line * 16;
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " % 4Xh(% 5u): ", Address, Address);
          p = HexDump;
          for (Offset = 0; Offset < 16; Offset++) {
            p += MrcSPrintf (MrcData, p, sizeof (HexDump) - (p - HexDump), "%02X ", Buffer[Address + Offset]) - 1;
          }
          for (Offset = 0; Offset < 16; Offset++) {
            Data8 = Buffer[Address + Offset];
            *p++ = isprint (Data8) ? Data8 : '.';
          }
          *p = '\0';
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s\n", HexDump);
        }
      }
    }
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****    TURNAROUND TIMING    *******\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\tSG\tDG\tDR\tDD\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "tRd2Rd:\t%Xh\t%Xh\t%Xh\t%Xh\n", Inputs->tRd2Rd.SG, Inputs->tRd2Rd.DG, Inputs->tRd2Rd.DR, Inputs->tRd2Rd.DD);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "tRd2Wr:\t%Xh\t%Xh\t%Xh\t%Xh\n", Inputs->tRd2Wr.SG, Inputs->tRd2Wr.DG, Inputs->tRd2Wr.DR, Inputs->tRd2Wr.DD);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "tWr2Rd:\t%Xh\t%Xh\t%Xh\t%Xh\n", Inputs->tWr2Rd.SG, Inputs->tWr2Rd.DG, Inputs->tWr2Rd.DR, Inputs->tWr2Rd.DD);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "tWr2Wr:\t%Xh\t%Xh\t%Xh\t%Xh\n", Inputs->tWr2Wr.SG, Inputs->tWr2Wr.DG, Inputs->tWr2Wr.DR, Inputs->tWr2Wr.DD);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****    THERMAL OVERWRITE    *******\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnableExtts     : %Xh\n",   ThermEnString, Inputs->EnableExtts);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnableCltm      : %Xh\n",   ThermEnString, Inputs->EnableCltm);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnableOltm      : %Xh\n",   ThermEnString, Inputs->EnableOltm);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnablePwrDn     : %Xh\n",   ThermEnString, Inputs->EnablePwrDn);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnablePwrDnLpddr: %Xh\n",   ThermEnString, Inputs->EnablePwrDnLpddr);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.Refresh2X       : %Xh\n",   ThermEnString, Inputs->Refresh2X);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.DdrThermalSensor: %Xh\n",   ThermEnString, Inputs->DdrThermalSensor);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.LockPTMregs     : %Xh\n",   ThermEnString, Inputs->LockPTMregs);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.UserPowerWeightsEn: %Xh\n", ThermEnString, Inputs->UserPowerWeightsEn);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.EnergyScaleFact : %Xh\n",   ThermEnString, Inputs->EnergyScaleFact);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim2Lock    : %Xh\n",   ThermEnString, Inputs->RaplLim2Lock);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim2WindX   : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim2WindX);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim2WindY   : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim2WindY);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim2Ena     : %Xh\n",   ThermEnString, Inputs->RaplLim2Ena);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim2Pwr     : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim2Pwr);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim1WindX   : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim1WindX);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim1WindY   : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim1WindY);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim1Ena     : %Xh\n",   ThermEnString, Inputs->RaplLim1Ena);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RaplLim1Pwr     : %Xh\n",   ThermEnString, Inputs->ThermalEnables.RaplLim1Pwr);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RaplPwrFlCh0       : %Xh\n",   Inputs->RaplPwrFlCh0);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RaplPwrFlCh01      : %Xh\n",   Inputs->RaplPwrFlCh1);
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WarmThresholdCh[%u]Dimm[%u] : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.WarmThreshold[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.HotThresholdCh[%u]Dimm[%u]  : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.HotThreshold[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WarmBudgetCh[%u]Dimm[%u]    : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.WarmBudget[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.HotBudgetCh[%u]Dimm[%u]     : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.HotBudget[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.IdleEnergyCh[%u]Dimm[%u]    : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.IdleEnergy[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.PdEnergyCh[%u]Dimm[%u]      : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.PdEnergy[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.ActEnergyCh[%u]Dimm[%u]     : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.ActEnergy[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.RdEnergyCh[%u]Dimm[%u]      : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.RdEnergy[Channel][Dimm]);
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s.WrEnergyCh[%u]Dimm[%u]      : %Xh\n", ThermEnString, Channel, Dimm, Inputs->ThermalEnables.WrEnergy[Channel][Dimm]);
    }
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SrefCfgEna      : %Xh\n",      Inputs->SrefCfgEna);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SrefCfgIdleTmr  : %Xh\n",      Inputs->SrefCfgIdleTmr);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ThrtCkeMinDefeat: %Xh\n",      Inputs->ThrtCkeMinDefeat);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ThrtCkeMinTmr   : %Xh\n",      Inputs->ThrtCkeMinTmr);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ThrtCkeMinDefeatLpddr: %Xh\n", Inputs->ThrtCkeMinDefeatLpddr);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "ThrtCkeMinTmrLpddr   : %Xh\n", Inputs->ThrtCkeMinTmrLpddr);

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****     DQ BYTE MAPPING     *******\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  ControllerIn = &Inputs->Controller[0];
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    ChannelIn = &ControllerIn->Channel[Channel];
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Ch %u:\n", Channel);
    for (Iteration = 0; Iteration < MrcIterationMax; Iteration++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "  Iteration %u: %02X %02X\n", Iteration, ChannelIn->DQByteMap[Iteration][0], ChannelIn->DQByteMap[Iteration][1]);
    }
  }

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "*****   DQS BYTE SWIZZLING    *******\n");
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%s", PrintBorder);
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    ChannelIn = &ControllerIn->Channel[Channel];
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Ch %u:", Channel);
    for (Iteration = 0; Iteration < (sizeof (ChannelIn->DqsMapCpu2Dram) / sizeof (ChannelIn->DqsMapCpu2Dram[0])); Iteration++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, " %u", ChannelIn->DqsMapCpu2Dram[Iteration]);
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\n");
  }
  for (Index = 0; Index < MAX_RCOMP; Index++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RcompResistor[%u]: %u\n", Index, Inputs->RcompResistor[Index]);
  }
  for (Index = 0; Index < MAX_RCOMP_TARGETS; Index++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "RcompTarget[%s]: %u\n", GlobalCompOffsetStr[Index], Inputs->RcompTarget[Index]);
  }

  return mrcSuccess;
}

/**
  Print the specified memory to the serial message debug port.

  @param[in] Debug - Serial message debug structure.
  @param[in] Start - The starting address to dump.
  @param[in] Size  - The amount of data in bytes to dump.

  @retval Nothing.
**/
void
MrcPrintMemory (
  IN MrcDebug *const    Debug,
  IN const UINT8 *const Start,
  IN const UINT32       Size
  )
{
  const UINT8  *Address;
  const UINT8  *End;
  UINT32       Line;
  UINT32       Offset;
  union {
    UINT64     Data64[2];
    UINT32     Data32[4];
    UINT16     Data16[8];
    UINT8      Data8[16];
  } Buffer;

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "          ");
  for (Offset = 0; Offset < 16; Offset++) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%02X ", ((UINTN) Start + Offset) % 16);
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\n");
  End = Start + Size;
  for (Line = 0; Line < ((Size / 16) + 1); Line++) {
    Address = Start + (Line * 16);
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "% 8X: ", Address);
    for (Offset = 0; Offset < 16; Offset++) {
      Buffer.Data8[Offset] = ((Address + Offset) < End) ? Address[Offset] : 0;
    }
    for (Offset = 0; Offset < 16; Offset++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, ((Address + Offset) < End) ? "%02X " : "   ", Buffer.Data8[Offset]);
    }
    for (Offset = 0; (Offset < 16) && ((Address + Offset) < End); Offset++) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "%c", isprint (Buffer.Data8[Offset]) ? Buffer.Data8[Offset] : '.');
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "\n");
  }
  return;
}
#endif
