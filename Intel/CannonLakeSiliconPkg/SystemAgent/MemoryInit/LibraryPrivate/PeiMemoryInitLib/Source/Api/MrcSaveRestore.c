/** @file

  Power state and boot mode save and restore data functions.

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

//
// Include files
//

#include "MrcTypes.h"
#include "MrcApi.h"
#include "MrcCommon.h"
#include "MrcGeneral.h"
#include "MrcGlobal.h"
#include "MrcSaveRestore.h"
#include "MrcSpdProcessing.h"
#include "MrcTimingConfiguration.h"

//
// ------- IMPORTANT NOTE --------
// MRC_REGISTER_COUNT_COMMON and MRC_REGISTER_COUNT_SAGV in MrcInterface.h should match these tables.
// Update these define's whenever you add/remove registers from the tables below.

//
// These registers must be saved only once, they don't depend on SA GV point.
// Total bytes to save = 720 + 168 + 192 + 144 + 96 + 4 + 72 = 1396
//
const SaveDataControlShort SaveDataCommonPerByte[] = {
  {0x08, 0x0C}, // 8 RxTrainRank2/3
  {0x18, 0x1C}, // 8 RxPerBitRank2/3
  {0x28, 0x2C}, // 8 TxTrainRank2/3
  {0x38, 0x3C}, // 8 TxPerBitRank2/3
  {0x58, 0x5C}, // 8 RxOffsetVdqRank2/3
}; // 8 * 5 = 40 bytes, 9 bytes / 2 ch => 40 * 18 = 720 bytes

// Also need to save registers from SaveDataSaGvPerByte for the ECC Byte:
// 84 * 2ch = 168 bytes

 const SaveDataControl SaveDataCommon[] = {
  {0x1E2C, 0x1E2C},  //  4   DdrDataComp2
  {0x1E40, 0x1E44},  //  8   DdrCrVssHiComp, DdrCrDimmVrefComp
  {0x2000, 0x2004},  //  8   SCRAM ScrambleCh0/1
  {0x202C, 0x2030},  //  8   DdrScrambleCh2, DdrScrambleCh3
  {0x2034, 0x2038},  //  8   DdrMiscControl1, DdrMiscControl2
  {0x203C, 0x2078},  //  64  LP4 DeSwizzleChannel0RankPair
  {0x209C, 0x20A8},  //  16  LP4 DELTADQSDEVUSEMAPCHANNEL 0:3
  {0x20B8, 0x20BC},  //  8   DdrMiscControl3, DdrMiscControl4
  {0x20D0, 0x20D4},  //  8   DdrMiscControl5, DdrMiscControl6
  {0x2D04, 0x2D04},  //  4   CRVREFCONTROL
  {0x2E04, 0x2E08},  //  8   vsshibota_DDRCRVSSHICONTROL, vsshibota_DDRCRVTTFORVSSHICORNRTROL
  {0x2F04, 0x2F08},  //  8   vsshitopa_DDRCRVSSHICONTROL, vsshitopa_DDRCRVTTFORVSSHICORNRTROL
  {0x3004, 0x3008},  //  8   vsshicenta_DDRCRVSSHICONTROL, vsshicenta_DDRCRVTTFORVSSHICORNRTROL
  {0x3100, 0x3100},  //  4   vtttop0 _DDRCRVTTGENCONTROL
  {0x3200, 0x3200},  //  4   vttmidhigh0_DDRCRVTTGENCONTROL
  {0x3300, 0x3300},  //  4   vttmidlow0_DDRCRVTTGENCONTROL
  {0x3400, 0x3400},  //  4   vttbot0_DDRCRVTTGENCONTROL
  {0x3500, 0x3500},  //  4   vtttop1_DDRCRVTTGENCONTROL
  {0x3600, 0x3600},  //  4   vttmidhigh1_DDRCRVTTGENCONTROL
  {0x3700, 0x3700},  //  4   vttmidlow1_DDRCRVTTGENCONTROL
  {0x3800, 0x3800},  //  4   vttbot1_DDRCRVTTGENCONTROL
                     //  Total DDRIO (without per-byte fubs): 48 * 4 = 192 bytes
  {0x4030, 0x4038},  //  12  Ch0 DftMisc, EccDft, ScPrCntConfig
  {0x4040, 0x4044},  //  8   Ch0 PmPdwnConfig, WmmReadConfig
  {0x4068, 0x406C},  //  8   Ch0 ScWdbwm
  {0x4198, 0x4198},  //  4   Ch0 McschedsSpare
  {0x4210, 0x4210},  //  4   Ch0 DdrMrParams
  {0x4230, 0x4234},  //  8   Ch0 DeswizzleLow, DeswizzleHigh
  {0x4254, 0x4254},  //  4   Ch0 McInitState
  {0x4280, 0x4284},  //  8   Ch0 DeswizzleLowErm, DeswizzleHighErm
  {0x42B8, 0x42C0},  //  12  Ch0 RhLfsr
  {0x43FC, 0x43FC},  //  4   Ch0 McmntsSpare
  {0x4430, 0x4438},  //  12  Ch1 DftMisc, EccDft, ScPrCntConfig
  {0x4440, 0x4444},  //  8   Ch1 PmPdwnConfig, WmmReadConfig
  {0x4468, 0x446C},  //  8   Ch1 ScWdbwm
  {0x4598, 0x4598},  //  4   Ch1 McschedsSpare
  {0x4610, 0x4610},  //  4   Ch1 DdrMrParams
  {0x4630, 0x4634},  //  8   Ch1 DeswizzleLow, DeswizzleHigh
  {0x4654, 0x4654},  //  4   Ch1 McInitState
  {0x4680, 0x4684},  //  8   Ch1 DeswizzleLowErm, DeswizzleHighErm
  {0x46B8, 0x46C0},  //  12  Ch1 RhLfsr
  {0x47FC, 0x47FC},  //  4   Ch1 McmntsSpare
                     //  Total MC Channel: ((12 * 2) + (8 * 4) + (4 * 4)) * 2ch = 144 bytes
  {0x5000, 0x5010},  //  20  MadInterChannel, MadIntraCh0/1, MadDimmCh0/1
  {0x5018, 0x501C},  //  8   McdecsCbit
  {0x5024, 0x5028},  //  8   ChannelHash, ChanneleHash
  {0x5030, 0x5034},  //  8   McInitStateG, MrcRevision
  {0x5060, 0x5060},  //  4   PmSrefConfig
  {0x5078, 0x5078},  //  4   IpcMcArb
  {0x50C0, 0x50DC},  //  32  BitErrorRecoverySourceSysaddr0/1/2/3
  {0x5138, 0x5138},  //  4   McGlobalDriverGateCfg
  {0x5140, 0x5144},  //  8   BitERrorRecoveryRangeSysaddr
                     //  Total MCDECS: 96 bytes
  {0x5200, 0x5200},  //  4   DdrplCfgDtf
                     //  Total Probeless: 4 bytes
  {0x5880, 0x5880},  //  4   PCU DdrPtmCtl
  {0x5884, 0x5888},  //  8   PCU DdrEnergyScaleFactor, DdrRaplChannelPowerFloor,
  {0x5890, 0x589C},  //  16  PCU DdrWarmThresholdCh0/1, DdrHotThresholdCh0/1
  {0x58A4, 0x58A4},  //  4   PCU DdrVoltage
  {0x58B0, 0x58BC},  //  16  PCU DdrDimmTempeatureCh0/1, DdrDimmHottestAbsolute, DdrDimmHottestRelative
  {0x58D0, 0x58E4},  //  24  PCU DdrWarmBudgetCh0/1, DdrHotBudgetCh0/1, DdrRaplLimit
                     //  Total PCU: 72
};

//
// These registers must be saved for each SA GV point.
// Total bytes to save = 1280 + 380 + 484 + 4 = 2148 bytes
//

 const SaveDataControlShort SaveDataSaGvPerByte[] = {
  {0x00, 0x04}, // 8  RxTrainRank0/1
  {0x10, 0x14}, // 8  RxPerBitRank0/1
  {0x20, 0x24}, // 8  TxTrainRank0/1
  {0x30, 0x34}, // 8  TxPerBitRank0/1
  {0x48, 0x48}, // 4  TxXtalk
  {0x50, 0x54}, // 8  RxOffsetVdqRank0/1
  {0x68, 0x70}, // 12 DataOffsetComp, DataControl1, DataControl2
  {0x7c, 0x80}, // 8  DataOffsetTrain, DataControl0
  {0x88, 0x94}, // 16 DataControl3, DataControl4, DataControl5, DataControl6
 }; // 4 + (8 * 6) + 12 + 16 = 80 bytes, 8 bytes / 2 ch => 80 * 16 = 1280 bytes

 const SaveDataControl SaveDataSaGv[] = {
  {0x1204, 0x1204},  //   4 Ch0 CKE   CmdCompOffset
  {0x1214, 0x1224},  //  20 Ch0 CKE   CtlCompOffset, CtlPiCoding, CtlControls, CtlRanksUsed, CtlControls1
  {0x1304, 0x1304},  //   4 Ch1 CKE   CmdCompOffset
  {0x1314, 0x1324},  //  20 Ch1 CKE   CtlCompOffset, CtlPiCoding, CtlControls, CtlRanksUsed, CtlControls1
  {0x1404, 0x1410},  //  16 Ch0 CMDB  CmdCompOffset, CmdPiCoding, CmdControls, CmdControls1
  {0x1504, 0x1510},  //  16 Ch1 CMDB  CmdCompOffset, CmdPiCoding, CmdControls, CmdControls1
  {0x1614, 0x1624},  //  20 Ch0 CCC   CtlCompOffset, CtlPiCoding, CtlControls, CtlRanksUsed , CtlControls1
  {0x1630, 0x1630},  //   4 Ch0 CCC   ClkCompOffset
  {0x1638, 0x1640},  //  12 Ch0 CCC   Ccc4CkeControls, Ccc4CkeRanksUsed, CccCompOffset1,
  {0x1714, 0x1724},  //  20 Ch1 CCC   CtlCompOffset, CtlPiCoding, CtlControls, CtlRanksUsed , CtlControls1
  {0x1730, 0x1730},  //   4 Ch1 CCC   ClkCompOffset
  {0x1738, 0x1740},  //  12 Ch1 CCC   Ccc4CkeControls, Ccc4CkeRanksUsed, CccCompOffset1,
  {0x1A04, 0x1A10},  //  16 Ch0 CMDA  CmdCompOffset, CmdPiCoding, CmdControls, CmdControls1
  {0x1B04, 0x1B10},  //  16 Ch1 CMDA  CmdCompOffset, CmdPiCoding, CmdControls, CmdControls1
  {0x1C14, 0x1C24},  //  20 Ch0 CTL   CtlCompoffset, CtlPiCoding, CtlControls, CtlRanksUsed, CtlControls1
  {0x1C30, 0x1C30},  //  4  Ch0 CLK   ClkCompOffset
  {0x1D14, 0x1D24},  //  20 Ch1 CTL   CtlCompoffset, CtlPiCoding, CtlControls, CtlRanksUsed, CtlControls1
  {0x1D30, 0x1D30},  //  4  Ch1 CLK   ClkCompOffset
  {0x1E00, 0x1E00},  //  4   Comp     DataComp0
  {0x1E08, 0x1E1C},  //  24  Comp     CmdComp, CtlComp, ClkComp CompCtl0, CompCtl1, DdrCrCompVsshi
  {0x1E28, 0x1E28},  //  4   Comp     CompCtl2
  {0x1E3C, 0x1E3C},  //  4   Comp     CompCtl3
  {0x1E48, 0x1E4C},  //  8   Comp     CompCtl4, CompCtl5
  {0x2008, 0x2028},  //  36  MiscControl0, WriteCfgCh0/1/2/3, ReadCfgCh0/1/2/3
  {0x207C, 0x2098},  //  32  LP4 RoCountTrainCh[0:3]Dev[0:1]
  {0x20AC, 0x20AC},  //  4   LP4 DeltaDqsCommon0
  {0x20C0, 0x20CC},  //  16  WriteCfgChAChB[0:1], ReadCfgChAChB[0:1]
  {0x20E0, 0x20E4},  //  8   RxDqFifoRdEnChAChB[0:1]
  {0x2D08, 0x2D0C},  //  8   DDRCRVREFADJUST 1:2
                     //  Total DDR IO (without per-byte fubs): 380 bytes
  {0x4000, 0x4004},  //  8   Ch0 TcPre, TcAct
  {0x400C, 0x4018},  //  16  Ch0 TcRdRd, TcRdWr, TcWrRd, TcWrWr,
  {0x4020, 0x4024},  //  8   Ch0 RoundTrip latency
  {0x4028, 0x402C},  //  8   CH0 SchedCbit, SchedSecondCbit
  {0x403C, 0x403C},  //  4   Ch0 ScPcit
  {0x4050, 0x4054},  //  8   Ch0 TcPowerDown
  {0x4070, 0x4078},  //  12  Ch0 TcOdt, McSchedsSpare
  {0x4080, 0x4080},  //  4   Ch0 ScOdtMatrix
  {0x4088, 0x4088},  //  4   Ch0 ScGsCfg
  {0x41A0, 0x41A4},  //  8   Ch0 SchedThirdCbit, DeadlockBreaker
  {0x4238, 0x4250},  //  28  Ch0 TcRfp, TcRftp, TcSrftp, McRefreshStagger, TcZqcal, TcMr2Shaddow, TcMr4Shaddow
  {0x4260, 0x4270},  //  20  Ch0 PmDimmIdleEnergy, PmDimmPdEnergy, PmDimmActEnergy, PmDimmRdEnergy, PmDimmWrEnergy
  {0x4278, 0x4278},  //  4   Ch0 ScWrAddDelay
  {0x4288, 0x4288},  //  4   Ch0 ScPbr
  {0x42C4, 0x42C8},  //  8   Ch0 TcSrExitTp, Lp4DqsOscillatorParams
  {0x4380, 0x439C},  //  32  Ch0 Lpddr4DiscreteMrValues[0:7]
  {0x43A0, 0x43D4},  //  56  Ch0 Ddr4Mr0Mr1Content, Ddr4Mr2Mr3Content, Ddr4Mr4Mr5Content, Ddr4Mr6Mr7Content, Ddr4Mr2RttWrValues,
                     //          Ddr4Mr6VrefValuesCh[0:1], LpddrMrContent, MrsFsmControl
  {0x43DC, 0x43DC},  //  4   Ch0 Ddr4Mr1OdicValues
  {0x4400, 0x4404},  //  8   Ch1 TcPre, TcAct
  {0x440C, 0x4418},  //  16  Ch1 TcRdRd, TcRdWr, TcWrRd, TcWrWr,
  {0x4420, 0x4424},  //  8   Ch1 RoundTrip latency
  {0x4428, 0x442C},  //  8   Ch1 SchedCbit, SchedSecondCbit
  {0x443C, 0x443C},  //  4   Ch1 ScPcit
  {0x4450, 0x4454},  //  8   Ch1 TcPowerDown
  {0x4470, 0x4478},  //  12  Ch1 TcOdt, McSchedsSpare
  {0x4480, 0x4480},  //  4   Ch1 ScOdtMatrix
  {0x4488, 0x4488},  //  4   Ch1 ScGsCfg
  {0x45A0, 0x45A4},  //  8   Ch1 SchedThirdCbit, DeadlockBreaker
  {0x4638, 0x4650},  //  28  Ch1 TcRfp, TcRftp, TcSrftp, McRefreshStagger, TcZqcal, TcMr2Shaddow, TcMr4Shaddow
  {0x4660, 0x4670},  //  20  Ch1 PmDimmIdleEnergy, PmDimmPdEnergy, PmDimmActEnergy, PmDimmRdEnergy, PmDimmWrEnergy
  {0x4678, 0x4678},  //  4   Ch1 ScWrAddDelay
  {0x4688, 0x4688},  //  4   Ch1 ScPbr
  {0x46C4, 0x46C8},  //  8   Ch1 TcSrExitTp, Lp4DqsOscillatorParams
  {0x4780, 0x479C},  //  32  Ch1 Lpddr4DiscreteMrValues[0:7]
  {0x47A0, 0x47D4},  //  56  Ch1 Ddr4Mr0Mr1Content, Ddr4Mr2Mr3Content, Ddr4Mr4Mr5Content, Ddr4Mr6Mr7Content, Ddr4Mr2RttWrValues,
                     //          Ddr4Mr6VrefValuesCh[0:1], LpddrMrContent, MrsFsmControl
  {0x47DC, 0x47DC},  //  4       Ddr4Mr1OdicValues

  {0x5130, 0x5134},  //  8   ScQos
  {0x5160, 0x5160},  //  4   ScQos2
                     // Total MC: (236) * 2ch + 8 + 4 = 484 bytes
  {0x59B8, 0x59B8},  //  4  PCU MrcOdtPowerSaving
                     // Total PCU: 4
};

/**
  This function verifies that neither CPU fuses or DIMMs have changed.

  @param[in] MrcData - Include all MRC global data.

  @retval mrcSuccess if fast boot is allowed, otherwise mrcColdBootRequired.
**/
MrcStatus
MrcFastBootPermitted (
  IN     MrcParameters *const MrcData
  )
{
  const MrcInput            *Inputs;
  const MRC_FUNCTION        *MrcCall;
  const MrcControllerIn     *ControllerIn;
  const MrcChannelIn        *ChannelIn;
  const MrcDimmIn           *DimmIn;
  const UINT8               *CrcStart;
  MrcOutput                 *Outputs;
  MrcDebug                  *Debug;
  MrcSaveData               *Save;
  MrcContSave               *ControllerSave;
  MrcChannelSave            *ChannelSave;
  MrcDimmOut                *DimmSave;
  MrcCapabilityId           Capid0Reg;
  UINT32                    CrcSize;
  UINT32                    Offset;
  UINT16                    DimmCrc;
  UINT8                     Controller;
  UINT8                     Channel;
  UINT8                     Dimm;

  CrcStart = NULL;
  CrcSize  = 0;
  Inputs   = &MrcData->Inputs;
  MrcCall = Inputs->Call.Func;
  Save     = &MrcData->Save.Data;
  Outputs  = &MrcData->Outputs;
  Debug    = &Outputs->Debug;

  // Obtain the capabilities of the memory controller and see if they have changed.
  Offset = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_A_0_0_0_PCI_CNL_REG);
  Capid0Reg.Data32.A.Data = MrcCall->MrcMmioRead32 (Offset);
  Capid0Reg.Data32.B.Data = MrcCall->MrcMmioRead32 (Offset + 4);
  Capid0Reg.Data32.C.Data = MrcCall->MrcMmioRead32 (Offset + 8);
  if (Capid0Reg.Data != Save->McCapId.Data) {
    MRC_DEBUG_MSG (
      Debug,
      MSG_LEVEL_NOTE,
      "Capabilities have changed, cold boot required\n '%X_%X_%X' --> '%X_%X_%X'\n",
      Save->McCapId.Data32.A.Data,
      Save->McCapId.Data32.B.Data,
      Save->McCapId.Data32.C.Data,
      Capid0Reg.Data32.A.Data,
      Capid0Reg.Data32.B.Data,
      Capid0Reg.Data32.C.Data
      );
    return mrcColdBootRequired;
  }
  // See if any of the DIMMs have changed.
  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerIn   = &Inputs->Controller[Controller];
    ControllerSave = &Save->Controller[Controller];
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelIn   = &ControllerIn->Channel[Channel];
      ChannelSave = &ControllerSave->Channel[Channel];
      for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
        DimmIn   = &ChannelIn->Dimm[Dimm];
        DimmSave = &ChannelSave->Dimm[Dimm];
        if (DimmIn->Status == DIMM_DISABLED) {
          DimmCrc = 0;
        } else {
          CrcStart = MrcSpdCrcArea (MrcData, Controller, Channel, Dimm, &CrcSize);
          GetDimmCrc ((const UINT8 *const) CrcStart, CrcSize, &DimmCrc);
        }

        MRC_DEBUG_MSG (
          Debug,
          MSG_LEVEL_NOTE,
          "Channel %u Dimm %u DimmCrc %Xh, DimmSave->Crc %Xh\n",
          Channel,
          Dimm,
          DimmCrc,
          DimmSave->Crc
          );
        if (DimmCrc != DimmSave->Crc) {
          MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Dimm has changed, cold boot required\n");
          return mrcColdBootRequired;
        }
      }
    }
  }
  // Set RestoreMRs flag to use trained Opt Param Values for Power Savings.
  Outputs->RestoreMRs = TRUE;

  return mrcSuccess;
}

/**
  This function saves any values that need to be used during non-cold boots.

  @param[in, out] MrcData - Include all the MRC global data.

  @retval mrcSuccess if the save occurred with no errors, otherwise returns an error code.
**/
MrcStatus
MrcSaveMCValues (
  IN OUT MrcParameters *const MrcData
  )
{
  const SaveDataControl       *SaveIt;
  const SaveDataControlShort  *SaveItShort;
  const MrcInput        *Inputs;
  const MrcControllerIn *ControllerIn;
  const MrcChannelIn    *ChannelIn;
  const MrcSpd          *SpdIn;
  const MRC_FUNCTION    *MrcCall;
  MrcIntOutput          *MrcIntData;
  MrcOutput             *Outputs;
  MrcDebug              *Debug;
  MrcSaveData           *SaveData;
  MrcSaveHeader         *SaveHeader;
  MrcControllerOut      *ControllerOut;
  MrcChannelOut         *ChannelOut;
  MrcContSave           *ControllerSave;
  MrcChannelSave        *ChannelSave;
  MrcProfile            Profile;
  UINT32                *McRegister;
  UINT32                *McRegisterStart;
  UINT32                Offset;
  UINT32                RegOffset;
  UINT32                Index;
  UINT32                Value;
  UINT32                Byte;
  UINT8                 *SpdBegin;
  UINT8                 Controller;
  UINT8                 Channel;
  UINT8                 Dimm;
  BOOLEAN               Any2Dpc;
  CAPID0_A_0_0_0_PCI_CNL_STRUCT Capid0A;
  CAPID0_B_0_0_0_PCI_CNL_STRUCT Capid0B;
  CAPID0_C_0_0_0_PCI_CNL_STRUCT Capid0C;

  // Copy channel and DIMM information to the data area that will be saved.
  Inputs      = &MrcData->Inputs;
  MrcCall     = Inputs->Call.Func;
  Outputs     = &MrcData->Outputs;
  SaveData    = &MrcData->Save.Data;
  SaveHeader  = &MrcData->Save.Header;
  Debug       = &Outputs->Debug;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));
  Any2Dpc     = FALSE;

  if ((Inputs->SaGv != MrcSaGvEnabled) || (MrcIntData->SaGvPoint == MrcSaGvPointLow)) {
    MrcCall->MrcSetMem ((UINT8 *) &MrcData->Save, sizeof (MrcSave), 0);
  }

  // Obtain the capabilities of the memory controller.
  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_A_0_0_0_PCI_CNL_REG);
  Capid0A.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_B_0_0_0_PCI_CNL_REG);
  Capid0B.Data = MrcCall->MrcMmioRead32 (Offset);

  Offset       = Inputs->PciEBaseAddress + MrcCall->MrcGetPcieDeviceAddress (0, 0, 0, CAPID0_C_0_0_0_PCI_CNL_REG);
  Capid0C.Data = MrcCall->MrcMmioRead32 (Offset);

  SaveData->McCapId.Data32.A.Data = Capid0A.Data;
  SaveData->McCapId.Data32.B.Data = Capid0B.Data;
  SaveData->McCapId.Data32.C.Data = Capid0C.Data;

  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerIn                  = &Inputs->Controller[Controller];
    ControllerOut                 = &Outputs->Controller[Controller];
    ControllerSave                = &SaveData->Controller[Controller];
    ControllerSave->ChannelCount  = ControllerOut->ChannelCount;
    ControllerSave->Status        = ControllerOut->Status;

    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelIn   = &ControllerIn->Channel[Channel];
      ChannelOut  = &ControllerOut->Channel[Channel];
      ChannelSave                     = &ControllerSave->Channel[Channel];
      ChannelSave->DimmCount          = ChannelOut->DimmCount;
      Any2Dpc                        |= (ChannelOut->DimmCount > 1);
      ChannelSave->ValidRankBitMask   = ChannelOut->ValidRankBitMask;
      ChannelSave->ValidSubChBitMask  = ChannelOut->ValidSubChBitMask;
      ChannelSave->ValidByteMask      = ChannelOut->ValidByteMask;
      ChannelSave->Status             = ChannelOut->Status;
      for (Profile = STD_PROFILE; Profile < MAX_PROFILE; Profile++) {
        MrcCall->MrcCopyMem ((UINT8 *) &ChannelSave->Timing[Profile], (UINT8 *) &ChannelOut->Timing[Profile], sizeof (MrcTiming));
      }
      for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
        MrcCall->MrcCopyMem ((UINT8 *) &ChannelSave->Dimm[Dimm], (UINT8 *) &ChannelOut->Dimm[Dimm], sizeof (MrcDimmOut));
        SpdIn = &ChannelIn->Dimm[Dimm].Spd.Data;
        SpdBegin = (UINT8 *) &SpdIn->Ddr4.ManufactureInfo;
        ChannelSave->DimmSave[Dimm].SpdDramDeviceType = SpdIn->Ddr4.Base.DramDeviceType.Data;
        ChannelSave->DimmSave[Dimm].SpdModuleType = SpdIn->Ddr4.Base.ModuleType.Data;
        ChannelSave->DimmSave[Dimm].SpdModuleMemoryBusWidth = SpdIn->Ddr4.Base.ModuleMemoryBusWidth.Data;
        // Save just enough SPD information so it can be restored during non-cold boot.
        MrcCall->MrcCopyMem ((UINT8 *) &ChannelSave->DimmSave[Dimm].SpdSave[0], SpdBegin, sizeof (ChannelSave->DimmSave[Dimm].SpdSave));
      } // for Dimm
    } // for Channel
  } // for Controller

  for (Profile = STD_PROFILE; Profile < MAX_PROFILE; Profile++) {
    SaveData->VddVoltage[Profile] = Outputs->VddVoltage[Profile];
  }

  // Copy specified memory controller MMIO registers to the data area that will be saved.
  // Start with the common section.
  if ((Inputs->SaGv == MrcSaGvEnabled) && (MrcIntData->SaGvPoint != MrcSaGvPointHigh)) {
     // If SA GV is enabled, only save the Common registers at the last point (currently High).
  } else {

    McRegister = SaveData->RegSaveCommon;

    for (Index = 0; Index < (sizeof (SaveDataCommon) / sizeof (SaveDataControl)); Index++) {
      SaveIt = &SaveDataCommon[Index];
      for (Offset = SaveIt->StartMchbarOffset; Offset <= SaveIt->EndMchbarOffset; Offset += sizeof (UINT32)) {
        Value         = MrcReadCR (MrcData, Offset);
        *McRegister++ = Value;
      }
    }
    // Common per-byte registers
    for (Index = 0; Index < (sizeof (SaveDataCommonPerByte) / sizeof (SaveDataControlShort)); Index++) {
      SaveItShort = &SaveDataCommonPerByte[Index];
      for (Offset = SaveItShort->StartMchbarOffset; Offset <= SaveItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
        for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
          for (Byte = 0; Byte < MAX_SDRAM_IN_DIMM; Byte++) {
            RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                                 (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Byte;
            Value = MrcReadCR (MrcData, RegOffset);
            *McRegister++ = Value;
          }
        }
      }
    }
    // Portion of the ECC byte, per channel
    for (Index = 0; Index < (sizeof (SaveDataSaGvPerByte) / sizeof (SaveDataControlShort)); Index++) {
      SaveItShort = &SaveDataSaGvPerByte[Index];
      for (Offset = SaveItShort->StartMchbarOffset; Offset <= SaveItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
        for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
          RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                               (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * 8;
          Value = MrcReadCR (MrcData, RegOffset);
          *McRegister++ = Value;
        }
      }
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Common section: saved %d bytes\n", (McRegister - SaveData->RegSaveCommon) * 4);
    if ((UINT32) (McRegister - SaveData->RegSaveCommon) > MRC_REGISTER_COUNT_COMMON) {
      MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "ERROR: RegSaveCommon overflow!\n");
      return mrcFail;
    }
  } // if SAGV and Low point

  if (MrcIntData->SaGvPoint == MrcSaGvPointHigh) {
    McRegister = SaveData->RegSaveHigh;
  } else if (MrcIntData->SaGvPoint == MrcSaGvPointMid) {
    McRegister = SaveData->RegSaveMid;
  } else {
    McRegister = SaveData->RegSaveLow;
  }
  McRegisterStart = McRegister;

  for (Index = 0; Index < (sizeof (SaveDataSaGv) / sizeof (SaveDataControl)); Index++) {
    SaveIt = &SaveDataSaGv[Index];
    for (Offset = SaveIt->StartMchbarOffset; Offset <= SaveIt->EndMchbarOffset; Offset += sizeof (UINT32)) {
      Value         = MrcReadCR (MrcData, Offset);
      *McRegister++ = Value;
    }
  }

  // Per-byte registers
  for (Index = 0; Index < (sizeof (SaveDataSaGvPerByte) / sizeof (SaveDataControlShort)); Index++) {
    SaveItShort = &SaveDataSaGvPerByte[Index];
    for (Offset = SaveItShort->StartMchbarOffset; Offset <= SaveItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        for (Byte = 0; Byte < MAX_SDRAM_IN_DIMM - 1; Byte++) {
          RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                               (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Byte;
          Value = MrcReadCR (MrcData, RegOffset);
          *McRegister++ = Value;
        }
      }
    }
  }

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SAGV section: saved %d bytes\n", (McRegister - McRegisterStart) * 4);
  if ((UINT32) (McRegister - McRegisterStart) > MRC_REGISTER_COUNT_SAGV) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "ERROR: RegSaveHigh/Mid/Low overflow!\n");
    return mrcFail;
  }

  MrcVersionGet (MrcData, &SaveData->Version);
  SaveData->CpuModel               = Inputs->CpuModel;
  SaveData->CpuStepping            = Inputs->CpuStepping;
  SaveData->CpuFamily              = Inputs->CpuFamily;
  SaveData->Frequency              = Outputs->Frequency;
  SaveData->FreqMax                = Outputs->FreqMax;
  SaveData->BurstLength            = Outputs->BurstLength;
  SaveData->HighFrequency          = Outputs->HighFrequency;
  SaveData->MemoryClock            = Outputs->MemoryClock;
  SaveData->Ratio                  = Outputs->Ratio;
  SaveData->RefClk                 = Outputs->RefClk;
  SaveData->EccSupport             = Outputs->EccSupport;
  SaveData->DdrType                = Outputs->DdrType;
  SaveData->Lp4x                   = Outputs->Lp4x;
  SaveData->Lp4x8                  = Outputs->Lp4x8;
  SaveData->EnhancedChannelMode    = Outputs->EnhancedChannelMode;
  SaveData->TCRSensitiveHynixDDR4  = Outputs->TCRSensitiveHynixDDR4;
  SaveData->TCRSensitiveMicronDDR4 = Outputs->TCRSensitiveMicronDDR4;
  SaveData->XmpProfileEnable       = Outputs->XmpProfileEnable;
  SaveData->BerEnable              = Outputs->BerEnable;
  SaveData->LpddrEctDone           = Outputs->LpddrEctDone;
  SaveData->Any2Dpc                = Any2Dpc;
  SaveData->DramDqOdtEn            = Outputs->DramDqOdtEn;
  SaveData->ODTCaCsCkSupport       = Outputs->ODTCaCsCkSupport;

  for (Index = 0; Index < 4; Index++) {
    SaveData->BerAddress[Index] = Outputs->BerAddress[Index];
  }

  SaveData->MeStolenSize           = Inputs->MeStolenSize;
  SaveData->ImrAlignment           = Inputs->ImrAlignment;
#ifdef UP_SERVER_FLAG
  if(Inputs->BoardType == btUpServer) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Output UP CLTM TSOD Offset\nValue = c0d0:%xh\tcod1:%xh\tc1d0:%xh\tc1d1:%xh\n", Outputs->ThermOffset[0][0],Outputs->ThermOffset[0][1], Outputs->ThermOffset[1][0], Outputs->ThermOffset[1][1] );
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
        SaveData->ThermOffset[Channel][Dimm] = Outputs->ThermOffset[Channel][Dimm];                        ///TSOD Thermal Offset
      }
    }
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Save UP CLTM TSOD Offset  \nValue = c0d0:%xh\tcod1:%xh\tc1d0:%xh\tc1d1:%xh\n", SaveData->ThermOffset[0][0],SaveData->ThermOffset[0][1], SaveData->ThermOffset[1][0], SaveData->ThermOffset[1][1] );
  }
#endif

  SaveData->SaMemCfgCrc = MrcCalculateCrc32 ((UINT8 *) Inputs->SaMemCfgAddress.Ptr, Inputs->SaMemCfgSize);
  SaveHeader->Crc       = MrcCalculateCrc32 ((UINT8 *) SaveData, sizeof (MrcSaveData));
  MrcData->Save.Size    = sizeof (MrcSave);
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Saved data CRC = %xh\n", SaveHeader->Crc);

  return mrcSuccess;
}

/**
  This function saves any updates to values that need to be used during non-cold boots.

  @param[in, out] MrcData - Include all the MRC global data.

  @retval mrcSuccess if the save occurred with no errors, otherwise returns an error code.
**/
MrcStatus
MrcUpdateSavedMCValues (
  IN OUT MrcParameters *const MrcData
  )
{
  const MrcInput        *Inputs;
  MrcSaveData           *SaveData;
  MrcSaveHeader         *SaveHeader;
  MrcStatus             Status;

  Inputs      = &MrcData->Inputs;
  SaveData    = &MrcData->Save.Data;
  SaveHeader  = &MrcData->Save.Header;
  Status      = mrcSuccess;

  // In Fast Boot, MeStolenSize may have changed. This should be updated within Save Data structure.
  SaveData->MeStolenSize           = Inputs->MeStolenSize;
  SaveData->ImrAlignment           = Inputs->ImrAlignment;

  SaveData->SaMemCfgCrc = MrcCalculateCrc32 ((UINT8 *) Inputs->SaMemCfgAddress.Ptr, Inputs->SaMemCfgSize);
  SaveHeader->Crc       = MrcCalculateCrc32 ((UINT8 *) SaveData, sizeof (MrcSaveData));
  MRC_DEBUG_MSG (&MrcData->Outputs.Debug, MSG_LEVEL_NOTE, "Saved data CRC = %xh\n", SaveHeader->Crc);

  return Status;
}

/**
  This function copies the non-training information that needs to be restored
  from the 'save' data structure to the 'Output' data structure.

  @param[in, out] MrcData - include all the MRC global data.

  @retval mrcSuccess if the copy completed with no errors, otherwise returns an error code.
**/
MrcStatus
MrcRestoreNonTrainingValues (
  IN OUT MrcParameters *const MrcData
  )
{
  MRC_FUNCTION      *MrcCall;
  MrcInput          *Inputs;
  MrcControllerIn   *ControllerIn;
  MrcChannelIn      *ChannelIn;
  MrcSaveData       *SaveData;
  MrcContSave       *ControllerSave;
  MrcChannelSave    *ChannelSave;
  MrcDimmOut        *DimmSave;
  MrcOutput         *Outputs;
  MrcDebug          *Debug;
  MrcIntOutput      *MrcIntData;
  MrcControllerOut  *ControllerOut;
  MrcChannelOut     *ChannelOut;
  MrcDimmOut        *DimmOut;
  MrcSpd            *SpdIn;
  UINT8             *SpdBegin;
  MrcProfile        Profile;
  MrcSaGvPoint      SaGvPoint;
  UINT8             Controller;
  UINT8             Channel;
  UINT8             Dimm;
  UINT8             Index;

  SaveData    = &MrcData->Save.Data;
  Outputs     = &MrcData->Outputs;
  Debug       = &Outputs->Debug;
  Inputs      = &MrcData->Inputs;
  MrcCall     = Inputs->Call.Func;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));
  SaGvPoint   = MrcIntData->SaGvPoint;

  for (Controller = 0; Controller < MAX_CONTROLLERS; Controller++) {
    ControllerIn                = &Inputs->Controller[Controller];
    ControllerSave              = &SaveData->Controller[Controller];
    ControllerOut               = &Outputs->Controller[Controller];
    ControllerOut->ChannelCount = ControllerSave->ChannelCount;
    ControllerOut->Status       = ControllerSave->Status;
    for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
      ChannelIn                     = &ControllerIn->Channel[Channel];
      ChannelSave                   = &ControllerSave->Channel[Channel];
      ChannelOut                    = &ControllerOut->Channel[Channel];
      ChannelOut->DimmCount         = ChannelSave->DimmCount;
      ChannelOut->ValidRankBitMask  = ChannelSave->ValidRankBitMask;
      ChannelOut->Status            = ChannelSave->Status;
      ChannelOut->ValidSubChBitMask = ChannelSave->ValidSubChBitMask;
      ChannelOut->ValidByteMask     = ChannelSave->ValidByteMask;
      for (Profile = STD_PROFILE; Profile < MAX_PROFILE; Profile++) {
        MrcCall->MrcCopyMem ((UINT8 *) &ChannelOut->Timing[Profile], (UINT8 *) &ChannelSave->Timing[Profile], sizeof (MrcTiming));
      }
      for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
        DimmSave = &ChannelSave->Dimm[Dimm];
        DimmOut  = &ChannelOut->Dimm[Dimm];
        if (DimmSave->Status == DIMM_PRESENT || DimmSave->Status == DIMM_DISABLED) {
          SpdIn   = &ChannelIn->Dimm[Dimm].Spd.Data;
          MrcCall->MrcCopyMem ((UINT8 *) DimmOut, (UINT8 *) DimmSave, sizeof (MrcDimmOut));
          SpdBegin = (UINT8 *) &SpdIn->Ddr4.ManufactureInfo;
          // Restore just enough SPD information so it can be passed out in the HOB.
          // If SAGV enabled, only do this on the last pass, due to LPDDR VendorId patching.
          if ((Inputs->SaGv != MrcSaGvEnabled) || (SaGvPoint == MrcSaGvPointHigh)) {
            MrcCall->MrcCopyMem (SpdBegin, (UINT8 *) &ChannelSave->DimmSave[Dimm].SpdSave[0], sizeof (ChannelSave->DimmSave[Dimm].SpdSave));
          }
        } else {
          DimmOut->Status = DimmSave->Status;
        }
      } // for Dimm
    } // for Channel
  } // for Controller

  for (Profile = STD_PROFILE; Profile < MAX_PROFILE; Profile++) {
    Outputs->VddVoltage[Profile] = SaveData->VddVoltage[Profile];
  }

// ------- IMPORTANT NOTE --------
// MeStolenSize should not be saved/restored (except on S3).  There is no rule stating that ME FW cannot request
// a different amount of ME UMA space from one boot to the next.  Also, if ME FW is updated/changed, the UMA
// Size requested from the previous version should not be restored.
//
  Inputs->CpuModel                = SaveData->CpuModel;
  Inputs->CpuStepping             = SaveData->CpuStepping;
  Inputs->CpuFamily               = SaveData->CpuFamily;
  Outputs->FreqMax                = SaveData->FreqMax;
  Outputs->Frequency              = SaveData->Frequency;
  Outputs->HighFrequency          = SaveData->HighFrequency;
  Outputs->MemoryClock            = SaveData->MemoryClock;
  Outputs->BurstLength            = SaveData->BurstLength;
  Outputs->Ratio                  = SaveData->Ratio;
  Outputs->RefClk                 = SaveData->RefClk;
  Outputs->EccSupport             = SaveData->EccSupport;
  Outputs->DdrType                = SaveData->DdrType;
  Outputs->EnhancedChannelMode    = SaveData->EnhancedChannelMode;
  Outputs->Lp4x                   = SaveData->Lp4x;
  Outputs->Lp4x8                  = SaveData->Lp4x8;
  Outputs->TCRSensitiveHynixDDR4  = SaveData->TCRSensitiveHynixDDR4;
  Outputs->TCRSensitiveMicronDDR4 = SaveData->TCRSensitiveMicronDDR4;
  Outputs->XmpProfileEnable       = SaveData->XmpProfileEnable;
  Outputs->ODTCaCsCkSupport       = SaveData->ODTCaCsCkSupport;

  if (SaGvPoint == MrcSaGvPointLow) {
    // Set the Low point for SA GV. On Cold flow this is done in SPD processing.
    if (Inputs->FreqSaGvLow) {
      Outputs->Frequency = Inputs->FreqSaGvLow;
    } else {
      Outputs->Frequency = (Outputs->DdrType == MRC_DDR_TYPE_DDR4) ? f1333 : f1067;
    }
    Outputs->HighFrequency = Outputs->Frequency;
  } else if (SaGvPoint == MrcSaGvPointMid) {
    // Set the Mid point for SA GV. On Cold flow this is done in SPD processing.
    if (Inputs->FreqSaGvMid) {
      Outputs->Frequency = Inputs->FreqSaGvMid;
    } else {
      Outputs->Frequency = (Outputs->DdrType == MRC_DDR_TYPE_DDR4) ? f2133 : f1867;
    }
    Outputs->HighFrequency = Outputs->Frequency;
  }

  Outputs->MemoryClock    = ConvertFreq2Clock (MrcData, Outputs->Frequency);
  Outputs->Ratio          = MrcFrequencyToRatio (MrcData, Outputs->Frequency, Outputs->RefClk, Inputs->BClkFrequency);
  MRC_DEBUG_MSG (
    Debug,
    MSG_LEVEL_NOTE,
    "SAGV %s point: Frequency=%u, tCK=%ufs, Ratio=%u\n",
    gFreqPointStr[SaGvPoint],
    Outputs->Frequency,
    Outputs->MemoryClock,
    Outputs->Ratio
    );

  Outputs->LpddrEctDone     = SaveData->LpddrEctDone;
  Outputs->BerEnable        = SaveData->BerEnable;
  Outputs->DramDqOdtEn      = SaveData->DramDqOdtEn;
  for (Index = 0; Index < 4; Index++) {
    Outputs->BerAddress[Index] = SaveData->BerAddress[Index];
  }

  if(Inputs->BootMode == bmS3) {
    Inputs->MeStolenSize      = SaveData->MeStolenSize;
    Inputs->ImrAlignment      = SaveData->ImrAlignment;
  }

#ifdef UP_SERVER_FLAG
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Save UP CLTM TSOD Offset  \nValue = c0d0:%xh\tcod1:%xh\tc1d0:%xh\tc1d1:%xh\n", SaveData->ThermOffset[0][0],SaveData->ThermOffset[0][1], SaveData->ThermOffset[1][0], SaveData->ThermOffset[1][1] );
  for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
    for (Dimm = 0; Dimm < MAX_DIMMS_IN_CHANNEL; Dimm++) {
      Outputs->ThermOffset[Channel][Dimm] = SaveData->ThermOffset[Channel][Dimm];                        ///TSOD Thermal Offset
    }
  }
MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Output UP CLTM TSOD Offset\nValue = c0d0:%xh\tcod1:%xh\tc1d0:%xh\tc1d1:%xh\n", Outputs->ThermOffset[0][0],Outputs->ThermOffset[0][1], Outputs->ThermOffset[1][0], Outputs->ThermOffset[1][1] );
#endif

  return mrcSuccess;
}

/**
  This function writes the previously determined training values back to the memory controller,
  for the SAGV section

  @param[in] MrcData    - Include all the MRC global data.
  @param[in] McRegister - Data array to restore the values from.

  @retval mrcSuccess if the memory controller write back completed with no errors, otherwise returns an error code.
**/
MrcStatus
MrcRestoreTrainingSaGv (
  IN MrcParameters *const MrcData,
  IN UINT32               *McRegister
  )
{
  const SaveDataControl       *RestoreIt;
  const SaveDataControlShort  *RestoreItShort;
  MrcDebug                    *Debug;
  INT64                       GetSetVal;
  UINT32                      *McRegisterStart;
  UINT32                      Offset;
  UINT32                      RegOffset;
  UINT32                      Index;
  UINT32                      Value;
  UINT32                      Channel;
  UINT32                      Byte;

  Debug    = &MrcData->Outputs.Debug;

  McRegisterStart = McRegister;

  for (Index = 0; Index < (sizeof (SaveDataSaGv) / sizeof (SaveDataControl)); Index++) {
    RestoreIt = &SaveDataSaGv[Index];
    for (Offset = RestoreIt->StartMchbarOffset; Offset <= RestoreIt->EndMchbarOffset; Offset += sizeof (UINT32)) {
      Value = *McRegister++;
      MrcWriteCR (MrcData, Offset, Value);
      // Read Turnaround Register values into cache.
      switch (Offset) {
        case CH0_CR_TC_RDRD_CNL_REG:
        case CH1_CR_TC_RDRD_CNL_REG:
          Channel = ((Offset - CH0_CR_TC_RDRD_CNL_REG) > 0) ? 1 : 0;
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDRDsg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDRDdg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDRDdr, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDRDdd, ReadCached, &GetSetVal);
          break;

        case CH0_CR_TC_RDWR_CNL_REG:
        case CH1_CR_TC_RDWR_CNL_REG:
          Channel = ((Offset - CH0_CR_TC_RDWR_CNL_REG) > 0) ? 1 : 0;
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDWRsg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDWRdg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDWRdr, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctRDWRdd, ReadCached, &GetSetVal);
          break;

        case CH0_CR_TC_WRRD_CNL_REG:
        case CH1_CR_TC_WRRD_CNL_REG:
          Channel = ((Offset - CH0_CR_TC_WRRD_CNL_REG) > 0) ? 1 : 0;
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRRDsg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRRDdg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRRDdr, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRRDdd, ReadCached, &GetSetVal);
          break;

        case CH0_CR_TC_WRWR_CNL_REG:
        case CH1_CR_TC_WRWR_CNL_REG:
          Channel = ((Offset - CH0_CR_TC_WRWR_CNL_REG) > 0) ? 1 : 0;
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRWRsg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRWRdg, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRWRdr, ReadCached, &GetSetVal);
          MrcGetSetDdrIoGroupChannel (MrcData, Channel, GsmMctWRWRdd, ReadCached, &GetSetVal);
          break;

        default:
          break;
      }
    }
  }

  // Apply Turnaround timings to ChannelOut structure from Cache
  MrcUpdateTatOutputs (MrcData);

  // Per-byte registers
  for (Index = 0; Index < (sizeof (SaveDataSaGvPerByte) / sizeof (SaveDataControlShort)); Index++) {
    RestoreItShort = &SaveDataSaGvPerByte[Index];
    for (Offset = RestoreItShort->StartMchbarOffset; Offset <= RestoreItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        for (Byte = 0; Byte < MAX_SDRAM_IN_DIMM - 1; Byte++) {
          RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                               (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Byte;
          Value = *McRegister++;
          MrcWriteCR (MrcData, RegOffset, Value);
        }
      }
    }
  }

  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "SAGV section: Restored %d bytes\n", (McRegister - McRegisterStart) * 4);
  if ((UINT32) (McRegister - McRegisterStart) > MRC_REGISTER_COUNT_SAGV) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "ERROR: RegSaveHigh/Mid/Low overflow!\n");
    return mrcFail;
  }

  return mrcSuccess;
}

/**
  This function writes the previously determined training values back to the memory controller.
  We also have SAGV flow for S3/Warm/Fast boot here.

  @param[in] MrcData - Include all the MRC global data.

  @retval mrcSuccess if the memory controller write back completed with no errors, otherwise returns an error code.
**/
MrcStatus
MrcRestoreTrainingValues (
  IN     MrcParameters *const MrcData
  )
{
  const SaveDataControl       *RestoreIt;
  const SaveDataControlShort  *RestoreItShort;
#ifdef UP_SERVER_FLAG
  const MrcInput        *Inputs;
#endif
  MrcDebug              *Debug;
  MrcIntOutput          *MrcIntData;
  MrcOutput             *Outputs;
  MrcSaveData           *SaveData;
  MrcStatus             Status;
  UINT32                *McRegister;
  UINT32                Offset;
  UINT32                RegOffset;
  UINT32                Index;
  UINT32                Value;
  UINT32                Channel;
  UINT32                Byte;

#ifdef UP_SERVER_FLAG
  Inputs   = &MrcData->Inputs;
#endif
  Outputs  = &MrcData->Outputs;
  Debug    = &Outputs->Debug;
  SaveData = &MrcData->Save.Data;
  MrcIntData  = ((MrcIntOutput *) (MrcData->IntOutputs.Internal));

  // Wait on RCOMP Done.  Needed to ensure Rcomp completes on warm reset/S3 before restoring dclk_enable.
  // dclk_enable gets restored with the training registers
  Status = CheckFirstRcompDone (MrcData);
  if (Status != mrcSuccess) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "RComp did not complete before the timeout in McFrequencySet\n");
    return Status;
  }

  // First restore the Common section
  McRegister = SaveData->RegSaveCommon;
  for (Index = 0; Index < (sizeof (SaveDataCommon) / sizeof (SaveDataControl)); Index++) {
    RestoreIt = &SaveDataCommon[Index];
    for (Offset = RestoreIt->StartMchbarOffset; Offset <= RestoreIt->EndMchbarOffset; Offset += sizeof (UINT32)) {
      Value = *McRegister++;
      MrcWriteCR (MrcData, Offset, Value);
    }
  }

  // Common per-byte registers
  for (Index = 0; Index < (sizeof (SaveDataCommonPerByte) / sizeof (SaveDataControlShort)); Index++) {
    RestoreItShort = &SaveDataCommonPerByte[Index];
    for (Offset = RestoreItShort->StartMchbarOffset; Offset <= RestoreItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        for (Byte = 0; Byte < MAX_SDRAM_IN_DIMM; Byte++) {
          RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                               (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Byte;
          Value = *McRegister++;
          MrcWriteCR (MrcData, RegOffset, Value);
        }
      }
    }
  }
  // Portion of the ECC byte, per channel
  for (Index = 0; Index < (sizeof (SaveDataSaGvPerByte) / sizeof (SaveDataControlShort)); Index++) {
    RestoreItShort = &SaveDataSaGvPerByte[Index];
    for (Offset = RestoreItShort->StartMchbarOffset; Offset <= RestoreItShort->EndMchbarOffset; Offset += sizeof (UINT32)) {
      for (Channel = 0; Channel < MAX_CHANNEL; Channel++) {
        RegOffset = Offset + (DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel +
                             (DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * 8;
        Value = *McRegister++;
        MrcWriteCR (MrcData, RegOffset, Value);
      }
    }
  }
  MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Common section: Restored %d bytes\n", (McRegister - SaveData->RegSaveCommon) * 4);
  if ((UINT32) (McRegister - SaveData->RegSaveCommon) > MRC_REGISTER_COUNT_COMMON) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_ERROR, "ERROR: RegSaveCommon overflow!\n");
    return mrcFail;
  }

  // Now restore the SAGV section, RegSaveHigh is used when SAGV is disabled
  if (MrcIntData->SaGvPoint == MrcSaGvPointHigh) {
    McRegister = SaveData->RegSaveHigh;
  } else if (MrcIntData->SaGvPoint == MrcSaGvPointMid) {
      McRegister = SaveData->RegSaveMid;
  } else {
    McRegister = SaveData->RegSaveLow;
  }

  Status = MrcRestoreTrainingSaGv (MrcData, McRegister);
  if (Status != mrcSuccess) {
    return Status;
  }

  ForceRcomp (MrcData);

#ifdef UP_SERVER_FLAG
  if (Inputs->BoardType == btUpServer) {
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Restoring CLTM TSOD Values.\n");
    CltmTsodConfiguration (MrcData);
#ifdef MRC_DEBUG_PRINT
    MrcCltmPrintMchRegisters (MrcData);
#endif //MRC_DEBUG_PRINT
  }
#endif

  return mrcSuccess;
}

/**
  Calculates a CRC-32 of the specified data buffer.

  @param[in] Data     - Pointer to the data buffer.
  @param[in] DataSize - Size of the data buffer, in bytes.

  @retval The CRC-32 value.
**/
UINT32
MrcCalculateCrc32 (
  IN     const UINT8       *const Data,
  IN     const UINT32      DataSize
  )
{
  UINT32 i;
  UINT32 j;
  UINT32 crc;
  UINT32 CrcTable[256];

  crc = (UINT32) (-1);

  // Initialize the CRC base table.
  for (i = 0; i < 256; i++) {
    CrcTable[i] = i;
    for (j = 8; j > 0; j--) {
      CrcTable[i] = (CrcTable[i] & 1) ? (CrcTable[i] >> 1) ^ 0xEDB88320 : CrcTable[i] >> 1;
    }
  }
  // Calculate the CRC.
  for (i = 0; i < DataSize; i++) {
    crc = (crc >> 8) ^ CrcTable[(UINT8) crc ^ (Data)[i]];
  }

  return ~crc;
}


#ifdef UP_SERVER_FLAG
#ifdef MRC_DEBUG_PRINT
/**
  This function Print the CLTM related registers.

  @param MrcData - Include all the MRC global data.

  @retval None.
**/
void
MrcCltmPrintMchRegisters (
  MrcParameters          *MrcData
  )
{
  MrcOutput               *Outputs;
  MrcDebug                *Debug;

  Outputs                 = &MrcData->Outputs;
  Debug                   = &Outputs->Debug;

    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "UP Power weight Energy registers...\n");
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_ENERGY_SCALEFACTOR %Xh: %Xh \n", PCU_CR_DDR_ENERGY_SCALEFACTOR_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_ENERGY_SCALEFACTOR_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH0_CR_PM_DIMM_RD_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH0_CR_PM_DIMM_RD_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH0_CR_PM_DIMM_RD_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH1_CR_PM_DIMM_RD_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH1_CR_PM_DIMM_RD_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH1_CR_PM_DIMM_RD_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH0_CR_PM_DIMM_WR_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH0_CR_PM_DIMM_WR_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH0_CR_PM_DIMM_WR_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH1_CR_PM_DIMM_WR_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH1_CR_PM_DIMM_WR_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH1_CR_PM_DIMM_WR_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH0_CR_PM_DIMM_ACT_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH0_CR_PM_DIMM_ACT_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH0_CR_PM_DIMM_ACT_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH1_CR_PM_DIMM_ACT_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH1_CR_PM_DIMM_ACT_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH1_CR_PM_DIMM_ACT_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH0_CR_PM_DIMM_IDLE_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH0_CR_PM_DIMM_IDLE_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH0_CR_PM_DIMM_IDLE_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH1_CR_PM_DIMM_IDLE_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH1_CR_PM_DIMM_IDLE_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH1_CR_PM_DIMM_IDLE_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH0_CR_PM_DIMM_PD_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH0_CR_PM_DIMM_PD_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH0_CR_PM_DIMM_PD_ENERGY_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "MCHBAR_CH1_CR_PM_DIMM_PD_ENERGY_REG %Xh: %Xh \n", MCHBAR_CH1_CR_PM_DIMM_PD_ENERGY_REG, MrcReadCR (MrcData, MCHBAR_CH1_CR_PM_DIMM_PD_ENERGY_REG));

    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Power budget registers ...\n");

    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_WARM_BUDGET_CH0 %Xh: %Xh \n", PCU_CR_DDR_WARM_BUDGET_CH0_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_WARM_BUDGET_CH0_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_WARM_BUDGET_CH1 %Xh: %Xh \n", PCU_CR_DDR_WARM_BUDGET_CH1_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_WARM_BUDGET_CH1_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_HOT_BUDGET_CH0 %Xh: %Xh \n", PCU_CR_DDR_HOT_BUDGET_CH0_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_HOT_BUDGET_CH0_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_HOT_BUDGET_CH1 %Xh: %Xh \n", PCU_CR_DDR_HOT_BUDGET_CH1_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_HOT_BUDGET_CH1_PCU_REG));


    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "Thermal Thresholds registers...\n");
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_WARM_THRESHOLD_CH0 %Xh: %Xh \n", PCU_CR_DDR_WARM_THRESHOLD_CH0_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_WARM_THRESHOLD_CH0_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_WARM_THRESHOLD_CH1 %Xh: %Xh \n", PCU_CR_DDR_WARM_THRESHOLD_CH1_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_WARM_THRESHOLD_CH1_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_HOT_THRESHOLD_CH0 %Xh: %Xh \n", PCU_CR_DDR_HOT_THRESHOLD_CH0_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_HOT_THRESHOLD_CH0_PCU_REG));
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_HOT_THRESHOLD_CH1 %Xh: %Xh \n", PCU_CR_DDR_HOT_THRESHOLD_CH1_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_HOT_THRESHOLD_CH1_PCU_REG));

    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "CLTM Configuration registers...\n");
    MRC_DEBUG_MSG (Debug, MSG_LEVEL_NOTE, "DDR_PTM_CTL %Xh: %Xh \n", PCU_CR_DDR_PTM_CTL_PCU_REG, MrcReadCR (MrcData, PCU_CR_DDR_PTM_CTL_PCU_REG));
}
#endif //MRC_DEBUG_PRINT
#endif
