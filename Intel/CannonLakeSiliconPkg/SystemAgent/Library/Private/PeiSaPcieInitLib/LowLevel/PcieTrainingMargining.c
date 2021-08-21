/** @file
  Margining for PEG Training

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2017 Intel Corporation.

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

@par Specification
**/
#include <SystemAgent/Library/Private/PeiSaPcieInitLib/PcieTraining.h>
#include "SaPegLowLevel.h"

///
/// Jitter Calculations
///
JITTER_SETTING Jitter_Gen3[] = {{1, 40, 1, 0}, {2, 60, 2, 0}, {3, 68, 3, 0}, {4, 104, 4, 0}, {5, 123, 5, 0}, {7, 144, 6, 0}, {8, 168, 7, 0}, {8, 187, 8, 0}, {11, 213, 9, 0}, {12, 221, 10, 0}, {11, 238, 11, 0}, {15, 134, 12, 1}, {17, 147, 13, 1}, {14, 153, 14, 1}, {17, 168, 15, 1}, {16, 177, 16, 1}, {20, 191, 17, 1}, {19, 198, 18, 1}, {20, 205, 19, 1}, {22, 216, 20, 1}, {24, 227, 21, 1}, {29, 243, 22, 1}, {24, 246, 23, 1}, {23, 248, 24, 1}};

EFI_STATUS
SetRawJitterTolerance (
  IN UINT8   *LaneList,
  IN UINT8   LaneListLength,
  IN UINT8   Step,
  IN UINT8   Depth,
  IN UINT8   DoubleMargin,
  IN BOOLEAN EnableJitter
  )
{
  UINT8  BundleList[SA_PEG_MAX_BUNDLE];
  UINT8  BundleListLength;
  UINT8  i;
  UINT8  Lane;
  UINT8  Bundle;
  UINT64 McD0BaseAddress;

  BundleListLength = 0;

  GetBundleList (LaneList, LaneListLength, &BundleList[0], &BundleListLength);

  McD0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, 0);
  ///
  ///
  if (LaneListLength > PCIE_MAX_LANE) {
    LaneListLength = PCIE_MAX_LANE;
  }
  for (i = 0; i < LaneListLength; ++i) {
    Lane = LaneList[i];

    ///
    ///
    PciSegmentAnd32 (McD0BaseAddress + R_SA_PEG_AFELN0CFG0_OFFSET + (Lane * LANE_STEP), 0xFFFFFBFF);
  }

  ///
  ///
  if (BundleListLength > SA_PEG_MAX_BUNDLE) {
    BundleListLength = SA_PEG_MAX_BUNDLE;
  }
  for (i = 0; i < BundleListLength; ++i) {
    Bundle = BundleList[i];

    ///
    ///
    PciSegmentAndThenOr32 (
      McD0BaseAddress + R_SA_PEG_AFEBND0CFG0_OFFSET + (Bundle * BUNDLE_STEP),
      (UINT32) ~(BIT28 | BIT17 | BIT16 | BIT15 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10),
      (UINT32)  ((DoubleMargin << 28) | (Step << 10))
      );

    ///
    ///
    PciSegmentAndThenOr32 (
      McD0BaseAddress + R_SA_PEG_AFEBND0CFG3_OFFSET + (Bundle * BUNDLE_STEP),
      (UINT32) ~(BIT16 | BIT15 | BIT14 | BIT13 | BIT12 | BIT11),
      (UINT32)  (Depth << 11)
      );
  }

  if (EnableJitter) {
    ///
    ///
    for (i = 0; i < LaneListLength; ++i) {
      Lane = LaneList[i];
      ///
      ///
      PciSegmentOr32 (McD0BaseAddress + R_SA_PEG_AFELN0CFG0_OFFSET + (Lane * LANE_STEP), BIT10);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SetJitterTolerance (
  IN UINT8 *LaneList,
  IN UINT8 LaneListLength,
  IN UINT8 ReportedMargin
  )
{
  JITTER_SETTING Settings;
  UINT8         i;
  BOOLEAN       Found;

  ZeroMem ((void *) &Settings, sizeof (JITTER_SETTING));

  if (ReportedMargin > 0) {
    ///
    ///
    Found = FALSE;
    for (i = 0; i < (JITTER_LENGTH - 1); ++i) {
      Settings = Jitter_Gen3[i];
      if (Settings.ReportedMargin == ReportedMargin) {
        Found = TRUE;
        break;
      }
    }

    if (Found) {
      return SetRawJitterTolerance (
               LaneList,
               LaneListLength,
               Settings.Step,
               Settings.Depth,
               Settings.DoubleMargin,
               TRUE
               );
    } else {
      return EFI_INVALID_PARAMETER;
    }
  } else {
    ///
    ///
    return SetRawJitterTolerance (
             LaneList,
             LaneListLength,
             Settings.Step,
             Settings.Depth,
             Settings.DoubleMargin,
             FALSE
             );
  }
}

VOID
EnableTxJitterInjection (
  IN UINT8   Lane,
  IN BOOLEAN EnableTxJitter
  )
{
  UINT8  Bundle;
  UINT64 McD0BaseAddress;

  Bundle = Lane / 2;
  McD0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, 0);

  if (EnableTxJitter) {
    PciSegmentAndThenOr32 (
      McD0BaseAddress + R_SA_PEG_AFEBND0CFG0_OFFSET + (Bundle * BUNDLE_STEP),
      (UINT32) ~(BIT1 | BIT2),
      0x6
      );
  } else {
    PciSegmentAnd32 (
      McD0BaseAddress + R_SA_PEG_AFEBND0CFG0_OFFSET + (Bundle * BUNDLE_STEP),
      (UINT32) ~(BIT1 | BIT2)
      );
  }
}

VOID
ConfigureTxJitterMux (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             Lane
  )
{
  CPU_SKU  CpuSku;

  CpuSku = GetCpuSku ();

  if (!((CpuSku == EnumCpuHalo) || (CpuSku == EnumCpuTrad))) {
    DEBUG ((DEBUG_WARN, "ConfigureTxJitterMux: System unsupported, no PEG.\n"));
  } else {
    PciSegmentAndThenOr32 (
      PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, R_SA_PEG_AFEBND0CFG5_OFFSET + ((Lane / 2) * BUNDLE_STEP)),
      ~(BIT30 | BIT31),
      (((Lane % 2) << 30) | BIT31)
      );
  }

  return;
}

/**
 Enable VOC margining

  @param[in] EnableVoc           - TRUE to enable VOC, FALSE otherwise

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
SetVocEnable (
  IN  BOOLEAN   EnableVoc
  )
{
  ///
  /// Always enabled on SKL
  ///
  return EFI_SUCCESS;
}

/**
 Program VOC using the given low level values for DS0 and DS1

  @param[in] Lane                - Physical Lane number to program
  @param[in] Ds0VocCode         - DS0 VOC Digital Code
  @param[in] Ds1VocCode         - DS1 VOC Digital Code
**/
VOID
SetRawVoc (
  IN UINT8  Lane,
  IN UINT16 Ds0VocCode,
  IN UINT16 Ds1VocCode
  )
{
  ///
  ///

  SetLoadBus (0, 1, (UINTN) Lane, R_SA_PEG_DS0_LOADBUS_OFFSET, (UINT32) (Ds0VocCode & B_SA_PEG_VOC_LOADBUS_MASK));
  SetLoadBus (0, 1, (UINTN) Lane, R_SA_PEG_DS1_LOADBUS_OFFSET, (UINT32) (Ds1VocCode & B_SA_PEG_VOC_LOADBUS_MASK));
}

/**
 Retrieve the current low level VOC values for DS0 and DS1

  @param[in] Lane                - Physical Lane number to get current VOC for
  @param[out] Ds0VocCode         - DS0 VOC Digital Code
  @param[out] Ds1VocCode         - DS1 VOC Digital Code
**/
VOID
GetRawVoc (
  IN  UINT8  Lane,
  OUT UINT16 *Ds0VocCode,
  OUT UINT16 *Ds1VocCode
  )
{
  UINT32 Data;

  Data      = GetMonBus (0, 1, (UINTN) Lane, R_SA_PEG_DS0_LOADBUS_OFFSET);
  *Ds0VocCode = (UINT16) (Data & B_SA_PEG_VOC_LOADBUS_MASK);

  Data      = GetMonBus (0, 1, (UINTN) Lane, R_SA_PEG_DS1_LOADBUS_OFFSET);
  *Ds1VocCode = (UINT16) (Data & B_SA_PEG_VOC_LOADBUS_MASK);
}

/**
 Generate VOC codes from initial VOC code

  @param[in]  InitialVocCode     - Initial VOC code
  @param[out] VocCodes           - Generated VOC codes based on initial VOC code
                                   Array must be exactly VOC_CODES_LENGTH (89) elements long
**/
EFI_STATUS
GenerateVocCodes (
  IN  UINT16 InitialVocCode,
  OUT VOC_CODE  *VocCodes
  )
{
  INT16   Offset;
  UINT16  SignBit;
  INT8    Sign;
  UINT8   Index;
  UINT8   InitialX1;
  UINT8   InitialX2;
  UINT8   InitialX4;
  UINT8   X1;
  UINT8   X2;
  UINT8   X4;

  Offset    = 0;
  Index     = 1;
  SignBit   = InitialVocCode & BIT11;
  Sign      = (SignBit == BIT11) ? 1 : -1;
  X1        = InitialVocCode & 0xF;
  X2        = (InitialVocCode >> 4) & 0x7;
  X4        = (InitialVocCode >> 7) & 0xF;
  InitialX1 = X1;
  InitialX2 = X2;
  InitialX4 = X4;
  ///
  /// Bounds Checking
  ///
  if (X1 > VOC_X1_MAX) {
    ASSERT (X1 <= VOC_X1_MAX);
    return EFI_DEVICE_ERROR;
  }
  if (X2 > VOC_X2_MAX) {
    ASSERT (X2 <= VOC_X2_MAX);
    return EFI_DEVICE_ERROR;
  }
  if (X4 > VOC_X4_MAX) {
    ASSERT (X4 <= VOC_X4_MAX);
    return EFI_DEVICE_ERROR;
  }
  ///
  /// Generate Table of Codes
  ///
  VocCodes[0].Offset  = 0;
  VocCodes[0].VocCode = InitialVocCode;
  while (!(X1 >= VOC_X1_MAX && X2 >= VOC_X2_MAX && X4 >= VOC_X4_MAX)) {
    Offset += Sign;
    if (X1 < VOC_X1_MAX) {
      X1++;
    } else if (X2 < VOC_X2_MAX) {
      X1--;
      X2++;
    } else {
      X2--;
      X4++;
    }
    ASSERT (Index < VOC_CODES_LENGTH);
    if (Index >= VOC_CODES_LENGTH) {
      return EFI_INVALID_PARAMETER;
    }
    VocCodes[Index].Offset  = Offset;
    VocCodes[Index].VocCode = SignBit | X1 | (X2 << 4) | (X4 << 7);
    Index++;
  }
  Offset  = 0;
  X1      = InitialX1;
  X2      = InitialX2;
  X4      = InitialX4;
  while (!(X1 <= 0 && X2 <= 0 && X4 <= 0)) {
    Offset += (-1 * Sign);
    if (X1 > 0) {
      X1--;
    } else if (X2 > 0) {
      X1++;
      X2--;
    } else {
      X2++;
      X4--;
    }
    ASSERT (Index < VOC_CODES_LENGTH);
    if (Index >= VOC_CODES_LENGTH) {
      return EFI_INVALID_PARAMETER;
    }
    VocCodes[Index].Offset  = Offset;
    VocCodes[Index].VocCode = SignBit | X1 | (X2 << 4) | (X4 << 7);
    Index++;
  }
  Sign   *= -1;
  SignBit = Sign > 0 ? BIT11 : 0;
  Offset += Sign;
  X1      = 1;
  X2      = 0;
  X4      = 0;
  ASSERT (Index < VOC_CODES_LENGTH);
  if (Index >= VOC_CODES_LENGTH) {
    return EFI_INVALID_PARAMETER;
  }
  VocCodes[Index].Offset  = Offset;
  VocCodes[Index].VocCode = SignBit | 0x1;
  Index++;
  while (!(X1 >= VOC_X1_MAX && X2 >= VOC_X2_MAX && X4 >= VOC_X4_MAX)) {
    Offset += Sign;
    if (X1 < VOC_X1_MAX) {
      X1++;
    } else if (X2 < VOC_X2_MAX) {
      X1--;
      X2++;
    } else {
      X2--;
      X4++;
    }
    ASSERT (Index < VOC_CODES_LENGTH);
    if (Index >= VOC_CODES_LENGTH) {
      return EFI_INVALID_PARAMETER;
    }
    VocCodes[Index].Offset  = Offset;
    VocCodes[Index].VocCode = SignBit | X1 | (X2 << 4) | (X4 << 7);
    Index++;
  }
  return EFI_SUCCESS;
}

/**
 Collects and computes data needed to perform VOC margining on a specific lane

  @param[in]  Lane               - Lane to prepare for VOC margining
  @param[out] VocState           - VOC margining data

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
InitializeVocMargining (
  IN  UINT8     Lane,
  OUT VOC_STATE *VocState
  )
{
  EFI_STATUS  Status;
  UINT16      Ds0VocCode;
  UINT16      Ds1VocCode;
  INT16       Ds0Max;
  INT16       Ds1Max;
  UINT8       Index;

  GetRawVoc (Lane, &Ds0VocCode, &Ds1VocCode);
  VocState->Lane            = Lane;
  VocState->InitialDs0Code  = Ds0VocCode;
  VocState->InitialDs1Code  = Ds1VocCode;
  ZeroMem (&(VocState->Ds0Codes[0]), sizeof (VocState->Ds0Codes));
  ZeroMem (&(VocState->Ds1Codes[0]), sizeof (VocState->Ds1Codes));
  Status = GenerateVocCodes (Ds0VocCode, &(VocState->Ds0Codes[0]));
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = GenerateVocCodes (Ds1VocCode, &(VocState->Ds1Codes[0]));
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Ds0Max = 0;
  Ds1Max = 0;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds0Codes[Index].Offset > Ds0Max) {
      Ds0Max = VocState->Ds0Codes[Index].Offset;
    }
  }
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds1Codes[Index].Offset > Ds1Max) {
      Ds1Max = VocState->Ds1Codes[Index].Offset;
    }
  }

  ///
  ///
  VocState->Ds0MaxUpMargin  = Ds0Max;
  VocState->Ds1MaxUpMargin  = Ds1Max;
  if (Ds0Max > Ds1Max) {
    VocState->MaxUpMargin   = Ds1Max;
  } else {
    VocState->MaxUpMargin   = Ds0Max;
  }

  Ds0Max = 0;
  Ds1Max = 0;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds0Codes[Index].Offset < Ds0Max) {
      Ds0Max = VocState->Ds0Codes[Index].Offset;
    }
  }
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds1Codes[Index].Offset < Ds1Max) {
      Ds1Max = VocState->Ds1Codes[Index].Offset;
    }
  }

  ///
  ///
  VocState->Ds0MaxDownMargin  = Ds0Max;
  VocState->Ds1MaxDownMargin  = Ds1Max;
  if (Ds0Max < Ds1Max) {
    VocState->MaxDownMargin   = Ds1Max;
  } else {
    VocState->MaxDownMargin   = Ds0Max;
  }
  return EFI_SUCCESS;
}

/**
 Stops all VOC stimulus on a lane and returns it to normal operation

  @param[in] VocState            - VOC margining data
**/
VOID
FinalizeVocMargining (
  IN VOC_STATE *VocState
  )
{
  ///
  ///
  SetRawVoc (
    VocState->Lane,
    VocState->InitialDs0Code,
    VocState->InitialDs1Code
    );
}

/**
 Set VOC stimulus by computing DS0 & 1 code pairs and programming them

  @param[in] ReportedMargin      - VOC stimulus level
  @param[in] VocState            - VOC margining data

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
SaPcieSetVoc (
  IN INT32     ReportedMargin,
  IN VOC_STATE *VocState
  )
{
  UINT16      Ds0VocCode;
  UINT16      Ds1VocCode;
  UINT8       Index;
  BOOLEAN     FoundCode;

  Ds0VocCode  = 0;
  Ds1VocCode  = 0;
  FoundCode   = FALSE;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds0Codes[Index].Offset == ReportedMargin) {
      Ds0VocCode  = VocState->Ds0Codes[Index].VocCode;
      FoundCode   = TRUE;
      break;
    }
  }
  if (!FoundCode) {
    return EFI_INVALID_PARAMETER;
  }
  FoundCode = FALSE;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds1Codes[Index].Offset == ReportedMargin) {
      Ds1VocCode  = VocState->Ds1Codes[Index].VocCode;
      FoundCode   = TRUE;
      break;
    }
  }
  if (!FoundCode) {
    return EFI_INVALID_PARAMETER;
  }

  SetRawVoc (VocState->Lane, Ds0VocCode, Ds1VocCode);

  return EFI_SUCCESS;
}

/**
 Computes the magnitude for a VOC stimulus level

  @param[in]  Offset             - VOC stimulus level
  @param[in]  VocCodes           - Table of VOC codes
  @param[out] OffsetMagnitude    - The magnitude for the VOC stimulus level

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
GetMagnitudeForOffset (
  IN  INT16     Offset,
  IN  VOC_CODE  *VocCodes,
  OUT INT32     *OffsetMagnitude
  )
{
  ///
  ///
  UINT16  OffsetVocCode;
  INT8    Sign;
  UINT8   Index;
  UINT8   X1;
  UINT8   X2;
  UINT8   X4;
  BOOLEAN FoundCode;

  OffsetVocCode = 0;
  FoundCode     = FALSE;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocCodes[Index].Offset == Offset) {
      OffsetVocCode = VocCodes[Index].VocCode;
      FoundCode     = TRUE;
      break;
    }
  }
  if (!FoundCode) {
    return EFI_INVALID_PARAMETER;
  }
  Sign              = ((OffsetVocCode & BIT11) == BIT11) ? 1 : -1;
  X1                = OffsetVocCode & 0xF;
  X2                = (OffsetVocCode >> 4) & 0x7;
  X4                = (OffsetVocCode >> 7) & 0xF;
  *OffsetMagnitude  = ((UINT32) Sign) * (((UINT32) X1) * 15 + ((UINT32) X2) * 40 + ((UINT32) X4) * 80);
  return EFI_SUCCESS;
}

/**
 Computes VOC stimulus level for a target mask

  @param[in]  VocMaskValue       - VOC mask
  @param[in]  DataSampler        - DS0 or DS1
  @param[in]  VocState           - VOC margining data
  @param[out] OffsetForMask      - VOC stimulus level for requested mask

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
CalculateOffsetForVocMaskValue (
  IN INT32     VocMaskValue,
  IN UINT8     DataSampler,
  IN VOC_STATE *VocState,
  OUT INT16    *OffsetForMask
  )
{
  EFI_STATUS  Status;
  VOC_CODE    *VocCodes;
  INT32       OffsetMagnitude;
  INT32       ZeroOffsetMagnitude;
  INT32       OffsetDifference;
  INT32       TargetDifference;
  INT16       MaxUpMargin;
  INT16       MaxDownMargin;
  INT16       Offset;

  switch (DataSampler) {
    case 0:
      VocCodes      = &(VocState->Ds0Codes[0]);
      MaxUpMargin   = VocState->Ds0MaxUpMargin;
      MaxDownMargin = VocState->Ds0MaxDownMargin;
      break;
    case 1:
      VocCodes      = &(VocState->Ds1Codes[0]);
      MaxUpMargin   = VocState->Ds1MaxUpMargin;
      MaxDownMargin = VocState->Ds1MaxDownMargin;
      break;
    default:
      return EFI_INVALID_PARAMETER;
  }
  Status = GetMagnitudeForOffset (0, VocCodes, &ZeroOffsetMagnitude);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  OffsetDifference  = 0;
  Offset            = 0;
  if (VocMaskValue > 0) {
    TargetDifference = VocMaskValue;
  } else {
    TargetDifference = -1 * VocMaskValue;
  }
  while (OffsetDifference < TargetDifference) {
    if (VocMaskValue > 0) {
      Offset++;
    } else {
      Offset--;
    }
    if ((Offset > MaxUpMargin) || (Offset < MaxDownMargin)) {
      return EFI_ABORTED;
    }
    Status = GetMagnitudeForOffset (Offset, VocCodes, &OffsetMagnitude);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if (OffsetMagnitude < ZeroOffsetMagnitude) {
      OffsetDifference = ZeroOffsetMagnitude - OffsetMagnitude;
    } else {
      OffsetDifference = OffsetMagnitude - ZeroOffsetMagnitude;
    }
  }
  *OffsetForMask = Offset;
  return EFI_SUCCESS;
}

/**
 Sets VOC stimulus to a target mask

  @param[in]  VocMaskValue       - VOC mask
  @param[in]  VocState           - VOC margining data

  @returns                       - EFI_SUCCESS on success, error code otherwise
**/
EFI_STATUS
SaPcieSetVocToMaskValue (
  IN INT32     VocMaskValue,
  IN VOC_STATE *VocState
  )
{
  EFI_STATUS  Status;
  INT16       Ds0Offset;
  INT16       Ds1Offset;
  UINT16      Ds0VocCode;
  UINT16      Ds1VocCode;
  UINT8       Index;
  BOOLEAN     FoundCode;

  ///
  /// Calculate Mask Offsets
  ///
  Status = CalculateOffsetForVocMaskValue (VocMaskValue, 0, VocState, &Ds0Offset);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = CalculateOffsetForVocMaskValue (VocMaskValue, 1, VocState, &Ds1Offset);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Convert Offsets to Codes
  ///
  Ds0VocCode  = 0;
  Ds1VocCode  = 0;
  FoundCode   = FALSE;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds0Codes[Index].Offset == Ds0Offset) {
      Ds0VocCode  = VocState->Ds0Codes[Index].VocCode;
      FoundCode   = TRUE;
      break;
    }
  }
  if (!FoundCode) {
    return EFI_INVALID_PARAMETER;
  }
  FoundCode = FALSE;
  for (Index = 0; Index < VOC_CODES_LENGTH; Index++) {
    if (VocState->Ds1Codes[Index].Offset == Ds1Offset) {
      Ds1VocCode  = VocState->Ds1Codes[Index].VocCode;
      FoundCode   = TRUE;
      break;
    }
  }
  if (!FoundCode) {
    return EFI_INVALID_PARAMETER;
  }
  SetRawVoc (VocState->Lane, Ds0VocCode, Ds1VocCode);
  return EFI_SUCCESS;
}
