/** @file
  HSIO routines

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2018 Intel Corporation.

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
#ifndef _PEI_HSIO_LIB_H_
#define _PEI_HSIO_LIB_H_

extern EFI_GUID gPchHsioChipsetInitSusTblDataPpiGuid;

typedef struct {
  UINT32            *PchHsioSusUsb2TuningPtr;     ///< Pointer to USB2 SUS Table
  UINT32            PchHsioSusUsb2TuningCount;    ///< Total number of entries in the USB2 SUS Table
  UINT32            *PchHsioSusModPhyTuningPtr;   ///< Pointer to ModPHY Tuning SUS Table
  UINT32            PchHsioSusModPhyTuningCount;  ///< Total number of entries in the ModPHY Tuning SUS Table
} PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI;

/**
  Represents HSIO lane
**/
typedef struct {
  UINT8        Index; ///< Lane index
  UINT8        Pid;   ///< Sideband ID
  UINT16       Base;  ///< Sideband base address
} HSIO_LANE;

/**
  Provides a pointer to the ChipsetInit SUS Table and creates it if required

  @param[in out] PchHsioChipsetInitSusTblData        Pointer to HSIO ChipsetInit SUS table Data

  @retval EFI_SUCCESS                 Located SUS Table PPI
  @retval EFI_OUT_OF_RESOURCES        Unable to allocate memory for SUS Table PPI
**/
EFI_STATUS
HsioGetChipsetInitSusTblData (
  IN OUT PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI  **PchHsioChipsetInitSusTblData
  );

/**
  Get HSIO lane representation needed to perform any operation on the lane.

  @param[in]  LaneIndex  Number of the HSIO lane
  @param[out] HsioLane   HSIO lane representation
**/
VOID
HsioGetLane (
  IN   UINT8       LaneIndex,
  OUT  HSIO_LANE   *HsioLane
  );

/**
  Read HSIO lane DWORD

  @param[in] HsioLane   HSIO Lane
  @param[in] Offset     PCR offset
  @return DWORD value
**/
UINT32
HsioLaneRead32 (
  CONST HSIO_LANE *HsioLane,
  UINT32 Offset
  );

/**
  Write HSIO lane DWORD

  @param[in] HsioLane   HSIO Lane
  @param[in] Offset     PCR offset
  @param[in] Data32     DWORD Value
**/
VOID
HsioLaneWrite32 (
  CONST HSIO_LANE *HsioLane,
  UINT32          Offset,
  UINT32          Data32
  );

/**
  Modify HSIO lane DWORD.

  @param[in] HsioLane   HSIO Lane
  @param[in] Offset     PCR offset
  @param[in] AndMask    Mask to be ANDed with original value.
  @param[in] OrMask     Mask to be ORed with original value.
**/
VOID
HsioLaneAndThenOr32 (
  CONST HSIO_LANE *HsioLane,
  UINT32          Offset,
  UINT32          AndMask,
  UINT32          OrMask
  );

/**
  Initialize HSIO lane for non-common clock PCIe port.

  @param[in] HsiLane   HSIO Lane
**/
VOID
HsioPcieNccLaneInit (
  CONST HSIO_LANE     *HsioLane
);

/**
  Writes PHY Tuning values to the ChipsetInit SUS Table

  @param[in] Pid        Sideband ID
  @param[in] Offset     PCR offset
  @param[in] Data32     DWORD Value

  @retval EFI_SUCCESS             Copy to SUS Table was successful
  @retval EFI_DEVICE_ERROR        Unable to save writes to SUS Table
**/
EFI_STATUS
HsioChipsetInitSusWrite32 (
  PCH_SBI_PID  Pid,
  UINT32       Offset,
  UINT32       Data32
  );

#endif // _PEI_HSIO_LIB_H_
