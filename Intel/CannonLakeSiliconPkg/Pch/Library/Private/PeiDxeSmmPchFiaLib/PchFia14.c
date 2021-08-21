/** @file
  This file contains routines used to communicate with FIA 14.0 hardware

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
#include "PchFiaLibInternal.h"
#include "PchFia14.h"

/**
  Get FIA lane owner

  @param[in] FiaInst  FIA Instance
  @param[in] LaneNum  lane number

  @retval PCH_FIA_LANE_OWNER  FIA lane owner
**/
PCH_FIA_LANE_OWNER
PchFia14GetLaneOwner (
  IN FIA_INSTANCE FiaInst,
  IN UINT8        LaneNum
  )
{
  UINT32  LosValue;
  UINT32  LosRegisterIndex;

  LosRegisterIndex = LaneNum / 8;
  //
  // Only 4 LOS registers in FIA 14.0
  //
  if (LosRegisterIndex > 3) {
    ASSERT (FALSE);
    return PchFiaOwnerInvalid;
  }

  LosValue = PchPcrRead32 (FiaInst.SbPortId, R_PCH_FIA_14_PCR_LOS1_REG_BASE + (4 * LosRegisterIndex));
  //
  // Invalid LOS register value means that we are in post boot SAI.
  // This code should not be called at this point so ASSERT.
  //
  if (LosValue == 0xFFFFFFFF) {
    ASSERT (FALSE);
    return PchFiaOwnerInvalid;
  }

  return PchFiaDecodeOwner (LosValue >> (4 * (LaneNum % 8)) & B_PCH_FIA_PCR_L0O, LaneNum);
}

/**
  Print FIA LOS registers.

  @param[in] FiaInst  FIA Instance
**/
VOID
PchFia14PrintLosRegisters (
  IN FIA_INSTANCE FiaInst
  )
{
  UINT32  Index;
  UINT32  MaxLosRegister;

  MaxLosRegister = ((PchFiaGetMaxLaneNum () - 1) / 8) + 1;
  for (Index = 0; Index < MaxLosRegister; Index++) {
    DEBUG ((DEBUG_INFO, "FIA LOS%d = 0x%08X\n", Index, PchPcrRead32 (FiaInst.SbPortId, R_PCH_FIA_14_PCR_LOS1_REG_BASE + (4 * Index))));
  }
}

/**
  Assigns CLKREQ# to PCH PCIe ports

  @param[in] FiaInst        FIA Instance
  @param[in] ClkReqMap      Mapping between PCH PCIe ports and CLKREQ#
  @param[in] ClkReqMapSize  Size of the map
**/
VOID
PchFia14AssignPchPciePortsClkReq (
  IN FIA_INSTANCE FiaInst,
  IN UINT8        *ClkReqMap,
  IN UINT8        ClkReqMapSize
  )
{
  UINT32  DrcrmValue;
  UINT8   DrcrmIndex;
  UINT8   PortIndex;
  UINT8   FirstClkReqIndex;
  UINT8   LastClkReqIndex;
  UINT8   ClkReqPosition;

  ASSERT (ClkReqMapSize <= PCH_FIA_14_MAX_PCIE_CLKREQ);

  for (DrcrmIndex = 0; DrcrmIndex < PCH_FIA_14_MAX_DRCRM; DrcrmIndex++) {

    DrcrmValue = 0;
    FirstClkReqIndex = DrcrmIndex * S_PCH_FIA_14_PCR_DRCRM_FIELDS_PER_REG;
    LastClkReqIndex = FirstClkReqIndex + S_PCH_FIA_14_PCR_DRCRM_FIELDS_PER_REG;

    for (PortIndex = FirstClkReqIndex; (PortIndex < LastClkReqIndex) && (PortIndex < PCH_FIA_14_MAX_PCIE_CLKREQ); PortIndex++) {
      ClkReqPosition = S_PCH_FIA_14_PCR_DRCRM_BITS_PER_FIELD * (PortIndex % S_PCH_FIA_14_PCR_DRCRM_FIELDS_PER_REG);
      if ((PortIndex < ClkReqMapSize) && (ClkReqMap[PortIndex] != PCH_FIA_NO_CLKREQ)) {
        DrcrmValue |= ClkReqMap[PortIndex] << ClkReqPosition;
      } else {
        DrcrmValue |= V_PCH_FIA_14_PCR_DRCRM_NO_CLKREQ << ClkReqPosition;
      }
    }

    //
    // On FIA 14 DRCRM4 register is used to map both PCIe and GbE CLKREQ
    // so we have to be careful not to change GbE CLKREQ# assignment
    //
    if (DrcrmIndex == 3) {
      PchPcrAndThenOr32 (
        FiaInst.SbPortId,
        R_PCH_FIA_14_PCR_DRCRM4,
        ~(UINT32) B_PCH_FIA_14_PCR_DRCRM4_PCIE_PORTS_MAPPING,
        DrcrmValue & B_PCH_FIA_14_PCR_DRCRM4_PCIE_PORTS_MAPPING);
    } else {
      PchPcrWrite32 (FiaInst.SbPortId, R_PCH_FIA_14_PCR_DRCRM1 + (DrcrmIndex * 4), DrcrmValue);
    }
  }
}

/**
  Assigns CLKREQ# to GbE

  @param[in] FiaInst     FIA Instance
  @param[in]  ClkReqNum  CLKREQ# number
**/
VOID
PchFia14AssignGbeClkReq (
  IN FIA_INSTANCE FiaInst,
  IN UINT8        ClkReqNum
  )
{
  if (ClkReqNum == PCH_FIA_NO_CLKREQ) {
    ClkReqNum = V_PCH_FIA_14_PCR_DRCRM_NO_CLKREQ;
  }
  PchPcrAndThenOr32 (
    FiaInst.SbPortId,
    R_PCH_FIA_14_PCR_DRCRM4,
    ~(UINT32) B_PCH_FIA_14_PCR_DRCRM_GBECLKREQ,
    ClkReqNum << N_PCH_FIA_14_PCR_DRCRM_GBECLKREQ);
}

/**
  Configures lower bound of delay between ClkReq assertion and driving RefClk.
  The delay is hardcoded to 15us, due to hardware design. Clocks may not be stable earlier.

  @param[in] FiaInst     FIA Instance
**/
VOID
PchFia14SetClockOutputDelay (
  IN FIA_INSTANCE FiaInst
  )
{
  UINT32 Data32Or;
  UINT32 Data32And;
  UINT32 Index;

  Data32Or = 0;
  Data32And = 0xFFFFFFFF;
  for (Index = 0; Index < S_PCH_FIA_PCR_TREFCLKON_FIELDS_PER_REG; Index++) {
    Data32Or |= (V_PCH_FIA_PCR_TREFCLKON_15us << (Index * S_PCH_FIA_PCR_TREFCLKON_FIELD));
    Data32And &= ~(B_PCH_FIA_PCR_TREFCLKON_FIELD << (Index * S_PCH_FIA_PCR_TREFCLKON_FIELD));
  }
  PchPcrAndThenOr32 (FiaInst.SbPortId, R_PCH_FIA_PCR_TREFCLKONTMR1, Data32And, Data32Or);
  PchPcrAndThenOr32 (FiaInst.SbPortId, R_PCH_FIA_PCR_TREFCLKONTMR2, Data32And, Data32Or);
}

