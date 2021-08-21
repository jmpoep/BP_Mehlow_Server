/** @file
  Pei HSIO Library for CNL-PCH

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

@par Specification Reference:
**/
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PchInfoLib.h>
#include <Library/PchSbiAccessLib.h>
#include <Private/Library/PeiHsioLib.h>
#include <Private/PchHsio.h>
#include <Register/PchRegsPcr.h>

/**
  Get HSIO lane representation needed to perform any operation on the lane.

  @param[in]  LaneIndex  Number of the HSIO lane
  @param[out] HsioLane   HSIO lane representation
**/
VOID
HsioGetLane (
  IN   UINT8       LaneIndex,
  OUT  HSIO_LANE   *HsioLane
  )
{
  // PCH-LP
  // iolane  0 -  5 : 0xAB - 000, 400, 800, c00, 1000, 1400
  // iolane  6 -  9 : 0xAA - 000, 400, 800, c00
  // iolane 10 - 15 : 0xA9 - 000, 400, 800, c00, 1000, 1400
  // PCH-H
  // iolane  0 -  9 : 0xAB - 000, 400, 800, c00, 1000, 1400, 1800, 1c00, 2000, 2400
  // iolane 10 - 21 : 0xAA - 000, 400, 800, c00, 1000, 1400, 1800, 1c00, 2000, 2400, 2800, 2c00
  // iolane 22 - 37 : 0xA8 - 000, 400, 800, c00, 1000, 1400, 1800, 1c00, 2000, 2400, 2800, 2c00, 3000, 3400, 3800, 3c00

  static UINT8 IoLanesLp[] = { 0, 6, 10, 16 };
  static UINT8 PidsLp[] = { PID_MODPHY0, PID_MODPHY1, PID_MODPHY2 };

  static UINT8 IoLanesH[] = { 0, 10, 22, 38 };
  static UINT8 PidsH[] = { PID_MODPHY0, PID_MODPHY1, PID_MODPHY3 };

  UINT8  *IoLanes;
  UINT8  *Pids;
  UINT8  PidMax;
  UINT32 Index;

  ASSERT (HsioLane != NULL);

  if (IsPchLp ()) {
    IoLanes = IoLanesLp;
    Pids    = PidsLp;
    PidMax  = (sizeof (IoLanesLp) / sizeof (UINT8)) - 1;
  } else {
    IoLanes = IoLanesH;
    Pids    = PidsH;
    PidMax  = (sizeof (IoLanesH) / sizeof (UINT8)) - 1;
  }
  ASSERT (LaneIndex < IoLanes[PidMax]);

  for (Index = 0; Index < PidMax; ++Index) {
    if (LaneIndex < IoLanes[Index + 1]) {
      HsioLane->Index = LaneIndex;
      HsioLane->Pid   = Pids[Index];
      HsioLane->Base  = (LaneIndex - IoLanes[Index]) * 0x400;
      return;
    }
  }
  ASSERT (FALSE);
}

/**
  Intializes the HSIO ChipsetInit SUS Table Data Structure

  @param[in out] PchHsioChipsetInitSusTblData        Pointer to HSIO ChipsetInit SUS table Data

  @retval EFI_SUCCESS                 Located SUS Table PPI
  @retval EFI_OUT_OF_RESOURCES        Unable to allocate memory for SUS Table PPI
**/
EFI_STATUS
HsioGetChipsetInitSusTblData (
  PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI   **PchHsioChipsetInitSusTblData
  )
{
  EFI_PEI_PPI_DESCRIPTOR                    *PchHsioChipsetInitSusTblDataPpiDesc;
  STATIC PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI   *mPchHsioChipsetInitSusTblDataPtr;
  PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI          *PchHsioChipsetInitSusTblDataPtr;
  EFI_STATUS                                Status;

  if (mPchHsioChipsetInitSusTblDataPtr != NULL) {
    *PchHsioChipsetInitSusTblData = mPchHsioChipsetInitSusTblDataPtr;
    return EFI_SUCCESS;
  }

  PchHsioChipsetInitSusTblDataPtr = NULL;

  Status = PeiServicesLocatePpi (
             &gPchHsioChipsetInitSusTblDataPpiGuid,
             0,
             NULL,
             (VOID **) &PchHsioChipsetInitSusTblDataPtr
             );
  if (Status == EFI_NOT_FOUND) {
    PchHsioChipsetInitSusTblDataPtr = (PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI *) AllocateZeroPool (sizeof (PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI));
    if (PchHsioChipsetInitSusTblDataPtr == NULL) {
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    PchHsioChipsetInitSusTblDataPtr->PchHsioSusUsb2TuningPtr = AllocateZeroPool (PCH_HSIO_CHIPSETINIT_USB2_SUS_MAX_SIZE * sizeof (PCH_HSIO_CMD_FIELD));
    if (PchHsioChipsetInitSusTblDataPtr->PchHsioSusUsb2TuningPtr == NULL) {
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    PchHsioChipsetInitSusTblDataPtr->PchHsioSusModPhyTuningPtr = AllocateZeroPool (PCH_HSIO_CHIPSETINIT_MODPHY_SUS_MAX_SIZE * sizeof (PCH_HSIO_CMD_FIELD));
    if (PchHsioChipsetInitSusTblDataPtr->PchHsioSusModPhyTuningPtr == NULL) {
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    PchHsioChipsetInitSusTblDataPtr->PchHsioSusUsb2TuningCount = 0;
    PchHsioChipsetInitSusTblDataPtr->PchHsioSusModPhyTuningCount = 0;

    PchHsioChipsetInitSusTblDataPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
    if (PchHsioChipsetInitSusTblDataPpiDesc == NULL) {
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    PchHsioChipsetInitSusTblDataPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    PchHsioChipsetInitSusTblDataPpiDesc->Guid = &gPchHsioChipsetInitSusTblDataPpiGuid;
    PchHsioChipsetInitSusTblDataPpiDesc->Ppi = PchHsioChipsetInitSusTblDataPtr;
    DEBUG ((DEBUG_INFO, "Installing HSIO ChipsetInit SUS Table Data PPI\n"));
    Status = PeiServicesInstallPpi (PchHsioChipsetInitSusTblDataPpiDesc);
    ASSERT_EFI_ERROR (Status);
  }

  *PchHsioChipsetInitSusTblData = PchHsioChipsetInitSusTblDataPtr;
  mPchHsioChipsetInitSusTblDataPtr = PchHsioChipsetInitSusTblDataPtr;
  return Status;
}

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
  )
{
  PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI  *PchHsioChipsetInitSusTblData;
  PCH_HSIO_CMD_FIELD                PchHsioCmdTblEntry;
  UINT32                            *DestPtr;
  EFI_STATUS                        Status;

  PchHsioCmdTblEntry.Command = PhyCmdSendPosted;
  PchHsioCmdTblEntry.Size    = PMC_DATA_CMD_SIZE;
  PchHsioCmdTblEntry.Pid     = Pid;
  PchHsioCmdTblEntry.Bar     = 0;
  PchHsioCmdTblEntry.OpCode  = PrivateControlWrite;
  PchHsioCmdTblEntry.Fbe     = 0x0F;
  PchHsioCmdTblEntry.Fid     = 0x00;
  PchHsioCmdTblEntry.Offset  = (UINT16) Offset;
  PchHsioCmdTblEntry.Value   = Data32;

  PchHsioChipsetInitSusTblData = NULL;
  Status = HsioGetChipsetInitSusTblData (&PchHsioChipsetInitSusTblData);
  ASSERT_EFI_ERROR (Status);
  if (PchHsioChipsetInitSusTblData == NULL) {
    return EFI_DEVICE_ERROR;
  }
  if (Pid == PID_USB2) {
    DestPtr = PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningPtr +
      ((PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningCount * sizeof (PCH_HSIO_CMD_FIELD)) / sizeof (PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningPtr));
    PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningCount++;
    if (PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningCount > PCH_HSIO_CHIPSETINIT_USB2_SUS_MAX_SIZE) {
      ASSERT (FALSE); //USB2 SUS Table size exceed allocated buffer size
      return EFI_DEVICE_ERROR;
    }
  } else {
    DestPtr = PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningPtr +
      ((PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningCount * sizeof (PCH_HSIO_CMD_FIELD)) / sizeof (PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningPtr));
    PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningCount++;
    if (PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningCount > PCH_HSIO_CHIPSETINIT_MODPHY_SUS_MAX_SIZE) {
      ASSERT (FALSE); //ModPHY Tuning SUS Table size exceed allocated buffer size
      return EFI_DEVICE_ERROR;
    }
  }
  CopyMem (DestPtr, (VOID *) &PchHsioCmdTblEntry, sizeof (PCH_HSIO_CMD_FIELD));
  return EFI_SUCCESS;
}