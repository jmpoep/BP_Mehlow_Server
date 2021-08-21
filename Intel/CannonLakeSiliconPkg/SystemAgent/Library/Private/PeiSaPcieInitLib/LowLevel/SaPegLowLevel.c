/** @file
  SA PCIe Initialization Low Level functions

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

@par Specification
**/

#include "SaPegLowLevel.h"
#include <Library/SaPlatformLib.h>

/**
  Invert the lane if LaneReversal bit is set based on PEG X16 width for PEG0 or PEG X4 for PEG3

  @param[in]  PegDev              - Device Number
  @param[in]  Lane                - Lane to potentially invert
  @param[in]  LaneReversal        - LaneReversal bit

  @retval     Lane number compensated for LaneReversal
**/
UINT8
ReverseLane (
  IN  UINT8 PegDev,
  IN  UINT8 Lane,
  IN  UINT8 LaneReversal
  )
{
  if (LaneReversal != 0) {
    if (PegDev == SA_PEG0_DEV_NUM) {
      return (SA_PEG0_CNT_MAX_LANE - Lane - 1);
    } else {
      return (SA_PEG3_CNT_MAX_LANE - Lane - 1);
    }
  } else {
    return Lane;
  }
}

/**
  Translate an array of lane numbers in to an array of bundle numbers

  @param[in]  LaneList            - Lanes to translate to bundles
  @param[in]  LaneListLength      - Length of LaneList array
  @param[out] BundleList          - Resulting array of bundle numbers
  @param[out] BundleListLength    - Length of BundleList array
**/
VOID
GetBundleList (
  IN  UINT8 *LaneList,
  IN  UINT8 LaneListLength,
  OUT UINT8 *BundleList,
  OUT UINT8 *BundleListLength
  )
{
  UINT8   Bundle;
  BOOLEAN HasBundle;
  UINT8   i;
  UINT8   j;

  (*BundleListLength) = 0;
  if (LaneListLength > PCIE_MAX_LANE) {
    ///
    /// LaneListLength cannot exceed LaneList array size.
    ///
    LaneListLength = PCIE_MAX_LANE;
  }
  for (i = 0; i < LaneListLength; ++i) {
    Bundle = LaneList[i] / 2;

    ///
    ///
    HasBundle = FALSE;
    for (j = 0; j < (*BundleListLength); ++j) {
      if (BundleList[j] == Bundle) {
        HasBundle = TRUE;
        break;
      }
    }

    if (!HasBundle) {
      BundleList[*BundleListLength] = Bundle;
      ++ (*BundleListLength);
    }
  }
}

/**
  Set Load Bus

  @param[in] DmiBar        - DMIBAR address
  @param[in] Dev           - Device Number
  @param[in] Lane          - Number of Lane
  @param[in] LoadSel       - Load selection value
  @param[in] LoadData      - Load Data
**/
VOID
SetLoadBus (
  IN UINT32 DmiBar,
  IN UINTN  Dev,
  IN UINTN  Lane,
  IN UINT32 LoadSel,
  IN UINT32 LoadData
  )
{
  UINT32  lbcvalue;
  UINT32  lbcdata;
  UINT32  lbclnsel;
  UINT32  lbcldsel;
  UINT32  lbcaddr;

  lbcaddr   = R_SA_PEG_LOADBUSCTL0_OFFSET + BUNDLE_STEP * (Lane >> 1);
  lbcvalue  = 0x70000000;

  lbclnsel  = ((Lane & 1) == 0) ? 0x80000 : 0x100000;
  lbcldsel  = (LoadSel & 0x3f) << 21;
  lbcdata   = (LoadData << 1) & 0x7FFF;

  lbcvalue  = lbcvalue | lbclnsel | lbcldsel | lbcdata;

  if (Dev == 0) {
    MmioWrite32 (DmiBar + lbcaddr, lbcvalue);
  } else {
    PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, Dev, SA_PEG0_FUN_NUM, lbcaddr), lbcvalue);
  }

  return;
}

/**
  Get monitor bus from the lane selected

  @param[in] DmiBar        - DMIBAR address
  @param[in] Dev           - Device number
  @param[in] Lane          - Number of Lane
  @param[in] LoadSel       - Load selection value

  @retval UINT32 - Load bus address
**/
UINT32
GetMonBus (
  IN UINT32 DmiBar,
  IN UINTN  Dev,
  IN UINTN  Lane,
  IN UINT32 LoadSel
  )
{
  UINT32  monvalue;
  UINT32  prevalue;
  UINT32  lbclnsel;
  UINT32  lbcldsel;
  UINT32  lbcaddr;
  UINT32  Result;
  UINT64  PegBaseAddress;

  lbcaddr   = R_SA_PEG_LOADBUSCTL0_OFFSET + BUNDLE_STEP * (Lane >> 1);
  monvalue  = 0x068008000;

  if (Dev == 0) {
    prevalue = MmioRead32 (DmiBar + lbcaddr);
  } else {
    prevalue = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, Dev, SA_PEG0_FUN_NUM, lbcaddr));
  }
  monvalue  = monvalue | (prevalue & 0x7FFE);

  lbclnsel  = ((Lane & 1) == 0) ? 0x80000 : 0x100000;
  lbcldsel  = (LoadSel & 0x3f) << 21;
  monvalue  = monvalue | lbclnsel | lbcldsel;

  if (Dev == 0) {
    MmioWrite32 (DmiBar + lbcaddr, monvalue);
    Result = MmioRead32 (DmiBar + lbcaddr);
  } else {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, Dev, SA_PEG0_FUN_NUM, 0);
    PciSegmentWrite32 (PegBaseAddress + lbcaddr, monvalue);
    Result = PciSegmentRead32 (PegBaseAddress + lbcaddr);
  }

  return (Result >> 1) & 0x3FFF;
}

/**
  Determine if all enabled PEG root ports are present in the given list of ports

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array

  @retval TRUE  - All enabled root ports are present
  @retval FALSE - Some or all of the enabled root ports are missing
**/
BOOLEAN
AllRootPortsSpecified (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  UINT32  HwStrap;
  UINT8   Index;
  UINT8   PegBus;
  UINT8   PegDev;
  UINT8   PegFunc;
  BOOLEAN AllRpsSpecified;
  BOOLEAN FoundFun0;
  BOOLEAN FoundFun1;
  BOOLEAN FoundFun2;
#ifndef CPU_CFL
  BOOLEAN FoundFun3;
#endif

  AllRpsSpecified = FALSE;
  FoundFun0       = FALSE;
  FoundFun1       = FALSE;
  FoundFun2       = FALSE;
#ifndef CPU_CFL
  FoundFun3       = FALSE;
#endif
  for (Index = 0; Index < PciePortsLength; Index++) {
    PegBus  = PciePorts[Index].Bus;
    PegDev  = PciePorts[Index].Device;
    PegFunc = PciePorts[Index].Function;
    if (PegBus == SA_PEG_BUS_NUM && PegDev == SA_PEG_DEV_NUM) {
      switch (PegFunc) {
        case SA_PEG0_FUN_NUM:
          FoundFun0 = TRUE;
          break;
        case SA_PEG1_FUN_NUM:
          FoundFun1 = TRUE;
          break;
        case SA_PEG2_FUN_NUM:
          FoundFun2 = TRUE;
          break;
        default:
          break;
      }
#ifndef CPU_CFL
    } else if (PegBus == SA_PEG_BUS_NUM && PegDev == SA_PEG3_DEV_NUM && PegFunc == SA_PEG3_FUN_NUM) {
      FoundFun3 = TRUE;
#endif
    }
  }

  HwStrap = (PciSegmentRead32 (
               PCI_SEGMENT_LIB_ADDRESS (
                 SA_SEG_NUM,
                 SA_PEG_BUS_NUM,
                 SA_PEG_DEV_NUM,
                 SA_PEG0_FUN_NUM,
                 R_SA_PEG_FUSESCMN_OFFSET
                 )
               ) >> 16) & 0x3;
  switch (HwStrap) {
    case SA_PEG_x8_x4_x4:
#ifdef CPU_CFL
      if (FoundFun0 && FoundFun1 && FoundFun2) {
#else
      if (FoundFun0 && FoundFun1 && FoundFun2 && FoundFun3) {
#endif
        AllRpsSpecified = TRUE;
      }
      break;
    case SA_PEG_x8_x8_x0:
#ifdef CPU_CFL
    if (FoundFun0 && FoundFun1) {
#else
      if (FoundFun0 && FoundFun1 && FoundFun3) {
#endif
        AllRpsSpecified = TRUE;
      }
      break;
    default:
    case SA_PEG_x16_x0_x0:
#ifdef CPU_CFL
    if (FoundFun0) {
#else
      if (FoundFun0 && FoundFun3) {
#endif
        AllRpsSpecified = TRUE;
      }
      break;
  }
  return AllRpsSpecified;
}
