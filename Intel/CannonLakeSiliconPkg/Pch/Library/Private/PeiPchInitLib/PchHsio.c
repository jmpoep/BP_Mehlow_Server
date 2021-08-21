/** @file
  PCH HSIO Initialization file

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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

#include <Library/PeiMeLib.h>
#include "PchInitPei.h"
#include <Library/PeiServicesTablePointerLib.h>
#include <Private/PchHsio.h>
#include <ChipsetInfoHob.h>
#include <BupMsgs.h>
#include <Library/ConfigBlockLib.h>
#include <SaPolicyCommon.h>
#include <ConfigBlock/PciePeiPreMemConfig.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/DciPrivateLib.h>
#include <Private/Library/PeiHsioLib.h>
#include <MeBiosPayloadHob.h>

GLOBAL_REMOVE_IF_UNREFERENCED const UINT16  mHsioCrc16Table[256] = {
  0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
  0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
  0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
  0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
  0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
  0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
  0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
  0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
  0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
  0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
  0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
  0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
  0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
  0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
  0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
  0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
  0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
  0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
  0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
  0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
  0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
  0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
  0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
  0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
  0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
  0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
  0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
  0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
  0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
  0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
  0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
  0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

/**
  Calculate CRC16 for target data.

  @param[in]  Data              The target data.
  @param[in]  DataSize          The target data size.

  @return UINT16                The CRC16 value.
**/
STATIC UINT16
PchHsioCalculateCrc16 (
  IN  UINT8   *Data,
  IN  UINTN   DataSize
  )
{
  UINTN   Index;
  UINT8   *Ptr;
  UINT16  Crc;

  Crc = 0;
  for (Index = 0, Ptr = Data; Index < DataSize; Index++, Ptr++) {
    Crc = (Crc >> 8) ^ (mHsioCrc16Table[((UINT8) Crc ^ *Ptr) & 0xFF]);
  }
  return Crc;
}

/**
  The function is used to detemine if a ChipsetInitSync with ME is required and syncs with ME if required.
  @todo: This function is deprecated nd should be removed on TBD date.

  @retval EFI_SUCCESS             BIOS and ME ChipsetInit settings are in sync
  @retval EFI_UNSUPPORTED         BIOS and ME ChipsetInit settings are not in sync
  @retval EFI_OUT_OF_RESOURCES    Could not allocate Memory
**/
EFI_STATUS
PchHsioDeprecatedChipsetInitProg (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT16                      BiosChipInitCrc;
  UINT16                      ComputedCrc;
  UINT8                       MeChipInitVersion;
  UINT8                       BiosChipInitVersion;
  EFI_BOOT_MODE               BootMode;
  UINT8                       *PchChipsetInitTable;
  UINT32                      PchChipsetInitTableLength;
  PCH_STEPPING                PchStep;
  UINT16                      MeChipInitCrc;
  BOOLEAN                     Usb2DbcEnabled;
  UINT8                       *ChipsetInitTblPtr;
  UINT8                       *MeChipsetInitTblPtr;
  UINT32                      MeChipsetInitTblLen;
  SI_PREMEM_POLICY_PPI        *SiPreMemPolicyPpi;
  PCH_DCI_PREMEM_CONFIG       *DciPreMemConfig;
  PCH_HSIO_VER_INFO           *CsmeChipsetInitVerInfoPtr;
  ME_BIOS_PAYLOAD_HOB         *MbpHob;
  CHIPSET_INIT_INFO           ChipsetInitHobStruct;
  CHIPSET_INIT_INFO           *ChipsetInitHob;

  MeChipInitVersion = 0;
  DciPreMemConfig = NULL;
  ///
  /// Step 1
  /// GetBootMode, do not perform ChipsetInit Sync check on S3 RESUME
  ///
  Status = PeiServicesGetBootMode (&BootMode);
  if (BootMode == BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }
  //
  // Get Policy settings through the SiPreMemPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gDciPreMemConfigGuid, (VOID *) &DciPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  ///
  /// Step 2
  /// Assign appropriate ChipsetInit table
  ///
  PchStep                   = PchStepping ();
  PchChipsetInitTable       = NULL;
  PchChipsetInitTableLength = 0;
  MeChipInitCrc             = 0;
  Usb2DbcEnabled            = (DciPreMemConfig->PlatformDebugConsent == ProbeTypeDciOobDbc) || (DciPreMemConfig->PlatformDebugConsent == ProbeTypeUsb2Dbc) || (IsDbcConnected ());

  ///
  /// Allocate memory for maximum ChipsetInit Table size and HECI Message Header
  ///
  ChipsetInitTblPtr =  AllocateZeroPool (PCH_HSIO_CHIPSETINIT_TBL_MAX_SIZE);
  if (ChipsetInitTblPtr == NULL) {
      DEBUG ((DEBUG_ERROR, "(PCHHSIO) PchHsioDeprecatedChipsetInitProg: Could not allocate Memory\n"));
      return EFI_OUT_OF_RESOURCES;
  }

  if (IsPchLp ()) {
    switch (PchStep) {
      case PCH_B0:
        if (Usb2DbcEnabled) {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_eDBC_B0;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_eDBC_B0);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_eDBC_B0 table \n"));
        } else {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_B0;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_B0);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_B0 table \n"));
        }
        break;
      case PCH_B1:
        if (Usb2DbcEnabled) {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_eDBC_Bx;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_eDBC_Bx);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_eDBC_Bx table \n"));
        } else {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_Bx;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_Bx);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_Bx table \n"));
        }
        break;
      case PCH_D0:
      case PCH_D1:
        if (Usb2DbcEnabled) {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_eDBC_Dx;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_eDBC_Dx);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_eDBC_Dx table \n"));
        } else {
          PchChipsetInitTable = CnlPchLpChipsetInitTable_Dx;
          PchChipsetInitTableLength = sizeof (CnlPchLpChipsetInitTable_Dx);
          DEBUG ((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchLpChipsetInitTable_Dx table \n"));
        }
        break;
      default:
        PchChipsetInitTable = NULL;
        PchChipsetInitTableLength   = 0;
        DEBUG ((DEBUG_ERROR, "PchHsioChipsetInitDeprecatedProg: Unsupported PCH Stepping\n"));
        break;
    }
  } else if (IsPchH ()) {
    switch (PchStep) {
    case PCH_A0:
      if (Usb2DbcEnabled) {
        PchChipsetInitTable = CnlPchHChipsetInitTable_eDBC_A0;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_eDBC_A0);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_eDBC_A0 table \n"));
      } else {
        PchChipsetInitTable = CnlPchHChipsetInitTable_A0;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_A0);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_A0 table \n"));
      }
      break;
    case PCH_A1:
      if (Usb2DbcEnabled) {
        PchChipsetInitTable = CnlPchHChipsetInitTable_eDBC_Ax;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_eDBC_Ax);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_eDBC_Ax table \n"));
      } else {
        PchChipsetInitTable = CnlPchHChipsetInitTable_Ax;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_Ax);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_Ax table \n"));
      }
      break;
    case PCH_B0:
      if (Usb2DbcEnabled) {
        PchChipsetInitTable = CnlPchHChipsetInitTable_eDBC_Bx;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_eDBC_Bx);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_eDBC_Bx table \n"));
      } else {
        PchChipsetInitTable = CnlPchHChipsetInitTable_Bx;
        PchChipsetInitTableLength = sizeof (CnlPchHChipsetInitTable_Bx);
        DEBUG((DEBUG_INFO, "PchHsioChipsetInitDeprecatedProg: Using CnlPchHChipsetInitTable_Bx table \n"));
      }
      break;
    default:
      PchChipsetInitTable = NULL;
      PchChipsetInitTableLength   = 0;
      DEBUG ((DEBUG_ERROR, "PchHsioChipsetInitDeprecatedProg: Unsupported PCH Stepping\n"));
      break;
    }
  }
  if (PchChipsetInitTable == NULL) {
    return EFI_UNSUPPORTED;
  }
  CopyMem (ChipsetInitTblPtr, (VOID *) PchChipsetInitTable, PchChipsetInitTableLength);

  ///
  /// Step 3
  /// Send the HECI HSIO Message
  ///
  Status         = EFI_SUCCESS;
  //
  // Get ChipsetInit table indentifier from the one found in the code
  //
  BiosChipInitCrc     = *((UINT16*) PchChipsetInitTable);
  BiosChipInitVersion = *((UINT8*) PchChipsetInitTable + 2);
  ComputedCrc = PchHsioCalculateCrc16 ((PchChipsetInitTable + 36), (PchChipsetInitTableLength - 36));
  DEBUG ((DEBUG_INFO, "(Hsio) BIOS ChipsetInit Base Table CRC = 0x%04X, Computed = 0x%04X\n", BiosChipInitCrc, ComputedCrc));

  ///
  /// Allocate memory for maximum ChipsetInit Table size and HECI Message Header
  ///
  MeChipsetInitTblLen = PCH_HSIO_CHIPSETINIT_TBL_MAX_SIZE;
  MeChipsetInitTblPtr =  AllocateZeroPool (MeChipsetInitTblLen);
  if (MeChipsetInitTblPtr == NULL) {
    DEBUG ((DEBUG_ERROR, "PchHsioChipsetInitDeprecatedProg: Could not allocate Memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  Status = PeiHeciReadChipsetInitMsg (MeChipsetInitTblPtr, &MeChipsetInitTblLen);
  MeChipInitCrc     = *((UINT16*) MeChipsetInitTblPtr);
  MeChipInitVersion = *((UINT8*) MeChipsetInitTblPtr + 2);

  if (Status == EFI_SUCCESS) {
    DEBUG ((DEBUG_INFO, "(Hsio) ME Reported CRC = 0x%04X\n", MeChipInitCrc));
    DEBUG ((DEBUG_INFO, "(Hsio) BIOS ChipsetInit Version = 0x%x, ME ChipsetInit Version = 0x%x\n", BiosChipInitVersion, MeChipInitVersion));
    if (MeChipInitCrc != BiosChipInitCrc) {
      DEBUG((DEBUG_INFO, "(Hsio) Heci ChipsetInit Sync Message\n"));
      Status = PeiHeciWriteChipsetInitMsg (ChipsetInitTblPtr, PchChipsetInitTableLength);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "ChipsetInit Sync Error: %r\n", Status));
        if ((Status == EFI_UNSUPPORTED) || (Status == EFI_DEVICE_ERROR)) {
          DEBUG ((DEBUG_ERROR, "Current CSME BIOS boot path doesn't support Chipset Init Sync message, continue to boot\n", Status));
        } else {
          ASSERT_EFI_ERROR (Status);
          return EFI_TIMEOUT;
        }
      } else {
        DEBUG ((DEBUG_INFO, "Reset required for ChipsetInit Settings sync\n"));
        SiScheduleResetSetType (EfiResetCold, NULL);
      }
    }
  } else {
    DEBUG ((DEBUG_INFO, "(Hsio) Syncing ChipsetInit with ME failed! Error: %r\n", Status));
  }

  ///
  /// Get CSME ChipsetInit Version Data from MBP
  ///
  MbpHob = NULL;
  MbpHob = GetFirstGuidHob (&gMeBiosPayloadHobGuid);
  CsmeChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) MbpHob->MeBiosPayload.ChipsetInitVerData;

  //
  // Initialize ChipsetInitHob
  //
  ZeroMem (&ChipsetInitHobStruct, sizeof (CHIPSET_INIT_INFO));
  ChipsetInitHobStruct.BaseVersion = BiosChipInitVersion;
  ChipsetInitHobStruct.OemVersion  = CsmeChipsetInitVerInfoPtr->OemVersion;

  ChipsetInitHob = BuildGuidDataHob (
                     &gChipsetInitHobGuid,
                     &ChipsetInitHobStruct,
                     sizeof (CHIPSET_INIT_INFO)
                     );

  if (ChipsetInitHob == NULL) {
    DEBUG ((DEBUG_ERROR, "(Hsio) ChipsetInitHob could not be created\n"));
    ASSERT (FALSE);
    return EFI_ABORTED;
  }

  return EFI_SUCCESS;
}


/**
  The function programs HSIO for PCIe Compliance.
**/
VOID
PchHsioEnablePcieComplianceMode (
  VOID
  )
{
  UINT32     MaxPcieLanes;
  UINT32     PcieIndex;
  UINT8      HsioLaneNo;
  HSIO_LANE  HsioLane;

  MaxPcieLanes = GetPchMaxPciePortNum ();
  for (PcieIndex = 0; PcieIndex < MaxPcieLanes; ++PcieIndex) {
    if (PchFiaGetPcieLaneNum (PcieIndex, &HsioLaneNo)) {
      DEBUG ((DEBUG_INFO, "PCIe compliance mode for PCIe%d, HSIO%d\n", PcieIndex + 1, HsioLaneNo));
      HsioGetLane (HsioLaneNo, &HsioLane);
      HsioLaneAndThenOr32 (&HsioLane, R_HSIO_PCR_PCS_DWORD4, (UINT32) ~BIT23, 0);
      HsioChipsetInitSusWrite32 (
        HsioLane.Pid,
        R_HSIO_PCR_PCS_DWORD4,
        HsioLaneRead32 (&HsioLane, R_HSIO_PCR_PCS_DWORD4));
    }
  }
}

/**
  The function programs the Pcie Pll SSC registers.

  @param[in]  SiPolicyPpi        The SI Policy PPI instance
**/
VOID
PchPciePllSscProg (
  IN  SI_POLICY_PPI      *SiPolicyPpi
  )
{
  UINT8                       PciePllSsc;
  EFI_STATUS                  Status;
  PCH_PM_CONFIG               *PmConfig;
  UINT8                       PciePllSscTable[21];

  PciePllSscTable[0] = 0x0;  // 0%
  PciePllSscTable[1] = 0x6;  // 0.1%
  PciePllSscTable[2] = 0xD;  // 0.2%
  PciePllSscTable[3] = 0x14; // 0.3%
  PciePllSscTable[4] = 0x1A; // 0.4%
  PciePllSscTable[5] = 0x21; // 0.5%
  PciePllSscTable[6] = 0x28; // 0.6%
  PciePllSscTable[7] = 0x2E; // 0.7%
  PciePllSscTable[8] = 0x35; // 0.8%
  PciePllSscTable[9] = 0x3C; // 0.9%
  PciePllSscTable[10] = 0x42; // 1.0%
  PciePllSscTable[11] = 0x49; // 1.1%
  PciePllSscTable[12] = 0x50; // 1.2%
  PciePllSscTable[13] = 0x56; // 1.3%
  PciePllSscTable[14] = 0x5D; // 1.4%
  PciePllSscTable[15] = 0x64; // 1.5%
  PciePllSscTable[16] = 0x6A; // 1.6%
  PciePllSscTable[17] = 0x71; // 1.7%
  PciePllSscTable[18] = 0x78; // 1.8%
  PciePllSscTable[19] = 0x7E; // 1.9%
  PciePllSscTable[20] = 0x85; // 2.0%

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  if (PmConfig->PciePllSsc == PCIE_PLL_SSC_AUTO) {
    return;
  }

  PciePllSsc = PmConfig->PciePllSsc;
  if (PciePllSsc > MAX_PCIE_PLL_SSC_PERCENT) {
    PciePllSsc = MAX_PCIE_PLL_SSC_PERCENT;
  }

  ///
  /// Step 1:  Clear i_sscen_h to 0b
  ///
  PchPcrAndThenOr32 (
    PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    (UINT32) ~B_HSIO_PCR_PLL_SSC_DWORD2_SSCSEN,
    0
  );

  HsioChipsetInitSusWrite32 (PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    PchPcrRead32 (PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD2));

  ///
  /// Step 2:  Write the desired Down Spread % bit values in i_sscstepsize_7_0
  ///
  PchPcrAndThenOr32 (
    PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    (UINT32) ~B_HSIO_PCR_PLL_SSC_DWORD2_SSCSTEPSIZE_7_0,
    (UINT32) (PciePllSscTable[PciePllSsc] << N_HSIO_PCR_PLL_SSC_DWORD2_SSCSTEPSIZE_7_0)
    );
  HsioChipsetInitSusWrite32 (PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    PchPcrRead32 (PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD2));

  ///
  /// Step 3:  Set i_ssc_propagate_h to 1b in the LCPLL_CFG1 register
  ///
  PchPcrAndThenOr32 (
    PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD3,
    (UINT32) ~B_HSIO_PCR_PLL_SSC_DWORD3_SSC_PROPAGATE,
    (UINT32) B_HSIO_PCR_PLL_SSC_DWORD3_SSC_PROPAGATE
    );
  HsioChipsetInitSusWrite32 (PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD3,
    PchPcrRead32 (PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD3));

  ///
  /// Step 4:  Set i_sscen_h to 1b
  ///
  PchPcrAndThenOr32 (
    PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    (UINT32) ~B_HSIO_PCR_PLL_SSC_DWORD2_SSCSEN,
    (UINT32) B_HSIO_PCR_PLL_SSC_DWORD2_SSCSEN
    );
  HsioChipsetInitSusWrite32 (PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    PchPcrRead32 (PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD2));

  DEBUG ((DEBUG_INFO, "HSIO : PortID = 0x%02x, Offset = 0x%04x, PciePllSsc = 0x%08x\n", PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD2, PchPcrRead32 (PID_MODPHY0, R_HSIO_PCR_PLL_SSC_DWORD2)));
}

/**
  The function program HSIO registers.

  @param[in] SiPolicyPpi               The SI Policy PPI instance

**/
VOID
PchHsioMiscBiosProg (
  IN  SI_POLICY_PPI    *SiPolicyPpi
  )
{
  EFI_STATUS        Status;
  PCH_PCIE_CONFIG   *PcieRpConfig;
  DEBUG ((DEBUG_INFO, "PchHsioMiscBiosProg() Start\n"));

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  if (PcieRpConfig->ComplianceTestMode) {
    PchHsioEnablePcieComplianceMode ();
  }
  PchPciePllSscProg (SiPolicyPpi);

  if (GetCpuSku () == EnumCpuUlx) {
    ///
    /// For CNL-Y CPU Sku, apply below configuration:
    ///  MODPHY2 Lane14 Dword0 [11] = 1
    ///  MODPHY2 Lane15 Dword0 [11] = 1
    ///
    PchPcrAndThenOr32 (PID_MODPHY2, R_HSIO_PCR_IOLANE14_PCS_DWORD0, 0xFFFFFFFF, BIT11);
    HsioChipsetInitSusWrite32 (PID_MODPHY2,
      R_HSIO_PCR_IOLANE14_PCS_DWORD0,
      PchPcrRead32 (PID_MODPHY2, R_HSIO_PCR_IOLANE14_PCS_DWORD0));
    PchPcrAndThenOr32 (PID_MODPHY2, R_HSIO_PCR_IOLANE15_PCS_DWORD0, 0xFFFFFFFF, BIT11);
    HsioChipsetInitSusWrite32 (PID_MODPHY2,
      R_HSIO_PCR_IOLANE15_PCS_DWORD0,
      PchPcrRead32 (PID_MODPHY2, R_HSIO_PCR_IOLANE15_PCS_DWORD0));
  }
  DEBUG ((DEBUG_INFO, "PchHsioMiscBiosProg() End\n"));
}

/**
  Prints the ChipsetInit table being pushed to CSME

  @param[in] TblPtr         Pointer to ChipsetInit Table buffer.
  @param[in] TblSz          ChipsetInit Table size.
  @param[in] SusSubTbl      TblPtr points to the SUS Sub-Table Only.
**/
VOID
PrintChipsetInitTable (
  IN UINT8     *TblPtr,
  IN UINT32    TblSz,
  IN BOOLEAN   SusSubTbl
  )
{
  DEBUG_CODE_BEGIN ();

  PCH_HSIO_VER_INFO  *ChipsetInitVerInfoPtr;
  PCH_HSIO_CMD_FIELD *PchChipsetInitTblRecordPtr;
  UINT8              *PchChipsetInitTblRecordStartPtr;
  UINT8              *ChipsetInitTblHdrPtr;
  UINT8              Field;
  UINT32             OffsetTracker;
  PCH_HSIO_HDR_FIELD *HdrEntryPtr;

  if (SusSubTbl) {
    OffsetTracker = 0;
  } else {
    OffsetTracker = sizeof (PCH_HSIO_VER_INFO) + (PCH_HSIO_HDR_FIELD_NUM * sizeof (PCH_HSIO_HDR_FIELD));
  }

  if (!SusSubTbl) {
    DEBUG ((DEBUG_INFO, "//*****************ChipsetInitTable BIOS Table Start*****************\n\n"));

    DEBUG ((DEBUG_INFO, "// Version Information\n"));
    ChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) TblPtr;
    DEBUG ((DEBUG_INFO, "0x%02x, 0x%02x, //Current Base CRC: 0x%x\n",
      TblPtr[0], TblPtr[1], ChipsetInitVerInfoPtr->BaseCrc));
    DEBUG ((DEBUG_INFO, "0x%02x, 0x%02x, //Current SUS CRC: 0x%x\n",
      TblPtr[2], TblPtr[3], ChipsetInitVerInfoPtr->SusCrc));
    DEBUG ((DEBUG_INFO, "0x%02x, 0x%02x, //Current OEM CRC: 0x%x\n",
      TblPtr[4], TblPtr[5], ChipsetInitVerInfoPtr->OemCrc));
    DEBUG ((DEBUG_INFO, "0x%02x, //Version: 0x%x\n",
      TblPtr[6], ChipsetInitVerInfoPtr->Version));
    DEBUG ((DEBUG_INFO, "0x%02x, //Product: 0x%x\n",
      TblPtr[7], ChipsetInitVerInfoPtr->Product));
    DEBUG ((DEBUG_INFO, "0x%02x, //Base Layer: 0x%x, Metal Layer: 0x%x\n",
      TblPtr[8], ChipsetInitVerInfoPtr->BaseLayer, ChipsetInitVerInfoPtr->MetalLayer));
    DEBUG ((DEBUG_INFO, "0x%02x, //OEM Version: %x\n", TblPtr[9], ChipsetInitVerInfoPtr->OemVersion));
    DEBUG ((DEBUG_INFO, "0x%02x, 0x%02x, //Debug Mode: %x, OEM CRC Valid: %x, SUS CRC Valid: %x, Base CRC Valid: %x, Reserved[0]: %x\n",
      TblPtr[10], TblPtr[11], ChipsetInitVerInfoPtr->DebugMode, ChipsetInitVerInfoPtr->OemCrcValid,
      ChipsetInitVerInfoPtr->SusCrcValid, ChipsetInitVerInfoPtr->BaseCrcValid, ChipsetInitVerInfoPtr->Reserved & BIT0));

    DEBUG ((DEBUG_INFO, "// Phy Data Group Table (Fixed table with 16 Entries)\n"));
    ChipsetInitTblHdrPtr = (UINT8*) (TblPtr + sizeof (PCH_HSIO_VER_INFO));
    HdrEntryPtr = (PCH_HSIO_HDR_FIELD *) (ChipsetInitTblHdrPtr);
    for (Field = 0; Field < (PCH_HSIO_HDR_FIELD_NUM * 2); Field+=2, HdrEntryPtr++) {
      DEBUG ((DEBUG_INFO, "0x%02x, 0x%02x, //Id: 0x%x, RecordOffset: 0x%x\n",
        ChipsetInitTblHdrPtr[Field], ChipsetInitTblHdrPtr[Field + 1],
        HdrEntryPtr->Id, RECORD_OFFSET (HdrEntryPtr->RecordOffsetHigh, HdrEntryPtr->RecordOffsetLow)));
    }
  }

  if (SusSubTbl) {
    PchChipsetInitTblRecordPtr = (PCH_HSIO_CMD_FIELD *) (TblPtr);
    PchChipsetInitTblRecordStartPtr = (UINT8 *) (TblPtr);
    DEBUG ((DEBUG_INFO, "//*****************SUS Table Start*****************\n"));
  } else {
    PchChipsetInitTblRecordPtr = (PCH_HSIO_CMD_FIELD *) (TblPtr + sizeof (PCH_HSIO_VER_INFO) + (PCH_HSIO_HDR_FIELD_NUM * sizeof (PCH_HSIO_HDR_FIELD)));
    PchChipsetInitTblRecordStartPtr = (UINT8 *) (TblPtr + sizeof (PCH_HSIO_VER_INFO) + (PCH_HSIO_HDR_FIELD_NUM * sizeof (PCH_HSIO_HDR_FIELD)));
  }
  DEBUG ((DEBUG_INFO, "// HSIO Command Table\n"));
  while (1) {
    if (OffsetTracker >= TblSz) {
      break;
    } else if (PchChipsetInitTblRecordStartPtr[0] == 0xF) {
      for (Field = 0; Field < 4; Field++) {
        DEBUG ((DEBUG_INFO, "0x%02x, ", PchChipsetInitTblRecordStartPtr[Field]));
      }
      DEBUG ((DEBUG_INFO, "//End Table Marker\n"));
      PchChipsetInitTblRecordStartPtr += 4;
      OffsetTracker += 4;
      PchChipsetInitTblRecordPtr = (PCH_HSIO_CMD_FIELD *) (PchChipsetInitTblRecordStartPtr);
    } else {
      for (Field = 0; Field < 12; Field++) {
        DEBUG ((DEBUG_INFO, "0x%02x, ", PchChipsetInitTblRecordStartPtr[Field]));
      }
      DEBUG ((DEBUG_INFO, "//Command: 0x%x, Size: 0x%x, PID: 0x%x, OpCode: 0x%x, Bar: 0x%x, FBE: 0x%x, FID: 0x%x, Offset: 0x%x, Value: 0x%x\n",
        PchChipsetInitTblRecordPtr->Command, PchChipsetInitTblRecordPtr->Size, PchChipsetInitTblRecordPtr->Pid,
        PchChipsetInitTblRecordPtr->OpCode, PchChipsetInitTblRecordPtr->Bar, PchChipsetInitTblRecordPtr->Fbe,
        PchChipsetInitTblRecordPtr->Fid, PchChipsetInitTblRecordPtr->Offset, PchChipsetInitTblRecordPtr->Value));
      PchChipsetInitTblRecordStartPtr += 12;
      OffsetTracker += 12;
      PchChipsetInitTblRecordPtr++;
    }
  }

  if (SusSubTbl) {
    DEBUG ((DEBUG_INFO, "//*****************SUS Table End*****************\n"));
  } else {
    DEBUG ((DEBUG_INFO, "//*****************ChipsetInitTable BIOS Table End*****************\n"));
  }

  DEBUG_CODE_END ();
}


/**
  Add entries to the ChipsetInit Table header

  @param[in]      ChipsetInitTblHdrPtr   Pointer to Header in the ChipsetInit Table.
  @param[in]      Hid                    HSIO Id.
  @param[in]      RecordOffset           Record Offset to be populated in the header.

  @retval     EFI_SUCCESS       The function completes successfully
  @retval     EFI_DEVICE_ERROR  The function encounters an error
**/
EFI_STATUS
AddHdrEntry (
  IN UINT8        *ChipsetInitTblHdrPtr,
  IN UINT8        Hid,
  IN UINT16       RecordOffset
  )
{
  PCH_HSIO_HDR_FIELD *HdrEntryPtr;
  INT8              Field;

  // Pointer to the last entry in the ChipsetInit Table header
  HdrEntryPtr = (PCH_HSIO_HDR_FIELD *) (ChipsetInitTblHdrPtr + (PCH_HSIO_HDR_FIELD_NUM * sizeof (PCH_HSIO_HDR_FIELD)) - sizeof (PCH_HSIO_HDR_FIELD));
  for (Field = 15; Field >= 0; Field--, HdrEntryPtr--) {
    // Check if the pointer is pointing to a the last validexisting entry
    if (RECORD_OFFSET (HdrEntryPtr->RecordOffsetHigh, HdrEntryPtr->RecordOffsetLow) != 0x00 || HdrEntryPtr->Id != 0x00) {
      if (Field != 15) {
        // Add the Header
        HdrEntryPtr++;
        HdrEntryPtr->Id = Hid;
        HdrEntryPtr->RecordOffsetLow = RecordOffset & 0xF;
        HdrEntryPtr->RecordOffsetHigh = (RecordOffset  & 0xFF0) >> 4;
        return EFI_SUCCESS;
      } else {
        ASSERT_EFI_ERROR (FALSE);
        DEBUG ((DEBUG_ERROR, "AddHdrEntry: All 16 Header Entries are present, no further Header Entry can be added\n"));
        return EFI_DEVICE_ERROR;
      }
    }
  }
  ASSERT_EFI_ERROR (FALSE);
  DEBUG ((DEBUG_ERROR, "AddHdrEntry: No Header Entries present, at least one Header Entry is expected to be non-zero\n"));
  return EFI_DEVICE_ERROR;
}

/**
  This function will append, delete or modify the SUS table as required

  @param[in, out] ChipsetInitTable       Pointer to buffer for the required system ChipsetInit Table.
  @param[in]      SusTblPtr              Pointer to SUS Table built by BIOS.
  @param[in, out] ChipsetInitTableLen    Pointer to the length of the table in bytes.
  @param[in]      Usb2SusTblSize         USB2 Tuning SUS Table Size.
  @param[in]      ModPhySusTblSize       ModPhy Tuning SUS Table Size.

  @retval     EFI_SUCCESS  The function completes successfully
  @retval     others
**/
EFI_STATUS
UpdateSusTable (
  IN OUT UINT8     *ChipsetInitTblPtr,
  IN UINT8         *SusTblPtr,
  IN OUT UINT32    *ChipsetInitTblLen,
  IN UINT32        Usb2SusTblSize,
  IN UINT32        ModPhySusTblSize
  )
{
  EFI_STATUS             Status;
  PCH_HSIO_HDR_FIELD     *HdrEntryPtr;
  PCH_HSIO_VER_INFO      *ChipsetInitVerInfoPtr;
  UINT32                 SusTblSize;
  UINT16                 RecordOffset;
  UINT8                  *PchChipsetInitTblRecordStartPtr;
  UINT8                  *ChipsetInitSusTblPtr;
  UINT8                  Field;
  UINT16                 Usb2TuningRecordOffset;
  UINT16                 ModPhyTuningRecordOffset;
  UINT8                  *ChipsetInitTblHdrPtr;
  BOOLEAN                Usb2TuningRecordPresent;
  BOOLEAN                ModPhyTuningRecordPresent;
  BOOLEAN                LastRecordPresent;

  Status = EFI_SUCCESS;
  HdrEntryPtr = (PCH_HSIO_HDR_FIELD *) (ChipsetInitTblPtr + sizeof (PCH_HSIO_VER_INFO));
  Usb2TuningRecordPresent = FALSE;
  ModPhyTuningRecordPresent = FALSE;
  LastRecordPresent = FALSE;
  ChipsetInitTblHdrPtr = NULL;

  for (Field = 0; Field < 16; Field++) {
    if (HdrEntryPtr->Id == PCH_HSIO_HEADER_ID_USB2_TUNING) {
      ///
      /// Obtain and clear USB2 Tuning Record Offset
      ///
      Usb2TuningRecordOffset = RECORD_OFFSET (HdrEntryPtr->RecordOffsetHigh, HdrEntryPtr->RecordOffsetLow);
      Usb2TuningRecordPresent = TRUE;
      ZeroMem (HdrEntryPtr, sizeof (PCH_HSIO_HDR_FIELD));
    } else if (HdrEntryPtr->Id == PCH_HSIO_HEADER_ID_MODPHY_TUNING) {
      ///
      /// Obtain and clear ModPhy Tuning Record Offset
      ///
      ModPhyTuningRecordOffset = RECORD_OFFSET (HdrEntryPtr->RecordOffsetHigh, HdrEntryPtr->RecordOffsetLow);
      ModPhyTuningRecordPresent = TRUE;
      ZeroMem (HdrEntryPtr, sizeof (PCH_HSIO_HDR_FIELD));
    } else if (HdrEntryPtr->RecordOffsetHigh == 0x00  &&
      HdrEntryPtr->RecordOffsetLow == 0x00 &&
      HdrEntryPtr->Id == 0xF) {
      ///
      /// Identify and clear Last Record Offset
      ///
      LastRecordPresent = TRUE;
      ZeroMem (HdrEntryPtr, sizeof (PCH_HSIO_HDR_FIELD));
      break;
    }
    HdrEntryPtr++;
  }

  PchChipsetInitTblRecordStartPtr = (UINT8 *) (ChipsetInitTblPtr + sizeof (PCH_HSIO_VER_INFO) + (PCH_HSIO_HDR_FIELD_NUM * sizeof (PCH_HSIO_HDR_FIELD)));

  ///
  /// Find ChipsetInit SUS Table Pointer Offset
  /// SUS Table Offset Pointer should point USB2 Tuning or
  /// ModPHY tuning depending on which comes first
  ///
  if (Usb2TuningRecordPresent || ModPhyTuningRecordPresent) {
    if (Usb2TuningRecordPresent) {
      if (ModPhyTuningRecordPresent) {
        if (Usb2TuningRecordOffset < ModPhyTuningRecordOffset) {
          ChipsetInitSusTblPtr = PchChipsetInitTblRecordStartPtr + Usb2TuningRecordOffset * 4;
        } else {
          ChipsetInitSusTblPtr = PchChipsetInitTblRecordStartPtr + ModPhyTuningRecordOffset * 4;
        }
      } else {
        ChipsetInitSusTblPtr = PchChipsetInitTblRecordStartPtr + Usb2TuningRecordOffset * 4;
      }
    } else {
      ChipsetInitSusTblPtr = PchChipsetInitTblRecordStartPtr + ModPhyTuningRecordOffset * 4;
    }

    ///
    /// Clear SUS Table Data
    ///
    ZeroMem (ChipsetInitSusTblPtr, *ChipsetInitTblLen - (ChipsetInitSusTblPtr - ChipsetInitTblPtr));

    ///
    /// Set new ChipsetInitTblLen with deleted SUS Table
    ///
    *ChipsetInitTblLen = ChipsetInitSusTblPtr - ChipsetInitTblPtr;
  }

  ChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) ChipsetInitTblPtr;
  if (ChipsetInitVerInfoPtr->SusCrcValid) {
    SusTblSize = Usb2SusTblSize + ModPhySusTblSize;
    ChipsetInitSusTblPtr = ChipsetInitTblPtr + *ChipsetInitTblLen;

    ///
    /// Append SUS Table to existing ChipsetInit Table
    ///
    CopyMem (ChipsetInitSusTblPtr, (VOID *) SusTblPtr, SusTblSize);

    ///
    /// Set new ChipsetInitTblLen with appended SUS Table
    ///
    *ChipsetInitTblLen = *ChipsetInitTblLen + SusTblSize;

    ///
    /// Update USB2 Tuning Record Offset
    ///
    ChipsetInitTblHdrPtr = (UINT8*) (ChipsetInitTblPtr + sizeof (PCH_HSIO_VER_INFO));
    if (Usb2SusTblSize) {
      RecordOffset = (UINT16) ((ChipsetInitSusTblPtr-PchChipsetInitTblRecordStartPtr)/4);
      Status = AddHdrEntry (ChipsetInitTblHdrPtr, PCH_HSIO_HEADER_ID_USB2_TUNING, RecordOffset);
    }

    ///
    /// Update ModPhy Tuning Record Offset
    /// Note: Assumption is that ModPhy Tuning Table is always appended to the end of the tuning table
    ///
    if (ModPhySusTblSize) {
      if (Usb2SusTblSize) {
        RecordOffset = RecordOffset + (((UINT16)Usb2SusTblSize)/4);
      } else {
        RecordOffset = (UINT16) ((ChipsetInitSusTblPtr-PchChipsetInitTblRecordStartPtr)/4);
      }
      Status = AddHdrEntry (ChipsetInitTblHdrPtr, PCH_HSIO_HEADER_ID_MODPHY_TUNING, RecordOffset);
    }
  }

  ///
  /// Update Last Record in header
  ///
  if ((ChipsetInitTblHdrPtr != NULL) && LastRecordPresent) {
    Status = AddHdrEntry (ChipsetInitTblHdrPtr, 0xF, 0x00);
  }
  return Status;
}

/**
  This function is used to ensure CSME has the latest Base, OEM and SUS tables.
  A BIOS<->CSME ChipsetInit sync is performed if there is a mismatch.

  @retval EFI_SUCCESS             BIOS and CSME ChipsetInit settings are in sync
  @retval EFI_UNSUPPORTED         BIOS and CSME ChipsetInit settings are not in sync
**/
EFI_STATUS
ChipsetInitSync (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_BOOT_MODE                     BootMode;
  ME_BIOS_PAYLOAD_HOB               *MbpHob;
  SI_POLICY_PPI                     *SiPolicyPpi;
  PCH_HSIO_CONFIG                   *HsioConfig;
  PCH_HSIO_VER_INFO                 *BiosChipsetInitVerInfoPtr;
  PCH_HSIO_VER_INFO                 *CsmeChipsetInitVerInfoPtr;
  PCH_HSIO_VER_INFO                 *ChipsetInitVerInfoPtr;
  PCH_HSIO_CHIPSETINIT_SUS_TBL_PPI  *PchHsioChipsetInitSusTblData;
  UINT8                             *SusTblPtr;
  UINT32                            Usb2SusTblSize;
  UINT32                            ModPhySusTblSize;
  UINT16                            BaseCrc;
  UINT16                            BiosComputedSusCrc;
  UINT16                            OemCrc;
  UINT8                             Version;
  UINT8                             OemVersion;
  UINT8                             *ChipsetInitTblPtr;
  UINT32                            ChipsetInitTblLen;
  BOOLEAN                           ClearSusValidBit;
  BOOLEAN                           SkipCsmeChipsetInitSync;
  UINT32                            EndTableMarker;
  CHIPSET_INIT_INFO                 *ChipsetInitHob;
  CHIPSET_INIT_INFO                 ChipsetInitHobStruct;
  PCH_RESET_DATA                    ResetData;

  DEBUG ((DEBUG_INFO, "ChipsetInitSync() Start\n"));

  ///
  /// GetBootMode, do not perform ChipsetInit Sync check on S3 RESUME
  ///
  Status = PeiServicesGetBootMode (&BootMode);
  if (BootMode == BOOT_ON_S3_RESUME) {
    DEBUG ((DEBUG_INFO, "Exit function as flow is not required on S3 resume\n"));
    return EFI_SUCCESS;
  }

  ///
  /// Get CSME ChipsetInit Version Data from MBP
  ///
  MbpHob = NULL;
  MbpHob = GetFirstGuidHob (&gMeBiosPayloadHobGuid);
  if (MbpHob == NULL) {
    DEBUG ((DEBUG_ERROR, "ChipsetInit Version information not present in MBP data: %r\n", Status));
    PchHsioDeprecatedChipsetInitProg (); //@todo: Remove code after backward compatibility is not required and replace with Assert
    return EFI_SUCCESS;
  }

  CsmeChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) MbpHob->MeBiosPayload.ChipsetInitVerData;

  DEBUG ((DEBUG_INFO, " Get CSME ChipsetInit Version Data from MBP \n"));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Base CRC           = %x\n", CsmeChipsetInitVerInfoPtr->BaseCrc));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary OEM CRC            = %x\n", CsmeChipsetInitVerInfoPtr->OemCrc));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary SUS CRC            = %x\n", CsmeChipsetInitVerInfoPtr->SusCrc));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Version            = %x\n", CsmeChipsetInitVerInfoPtr->Version));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Product            = %x\n", CsmeChipsetInitVerInfoPtr->Product));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Metal Layer        = %x\n", CsmeChipsetInitVerInfoPtr->MetalLayer));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Base Layer         = %x\n", CsmeChipsetInitVerInfoPtr->BaseLayer));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary OEM Version        = %x\n", CsmeChipsetInitVerInfoPtr->OemVersion));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Debug Mode         = %x\n", CsmeChipsetInitVerInfoPtr->DebugMode));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary OEM CRC Valid      = %x\n", CsmeChipsetInitVerInfoPtr->OemCrcValid));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary SUS CRC Valid      = %x\n", CsmeChipsetInitVerInfoPtr->SusCrcValid));
  DEBUG ((DEBUG_INFO, " ChipsetInit Binary Base CRC Valid     = %x\n", CsmeChipsetInitVerInfoPtr->BaseCrcValid));

  //@todo: Remove code after backward compatibility is not required
  if(CsmeChipsetInitVerInfoPtr->OemCrc == 0x0000) {
    PchHsioDeprecatedChipsetInitProg ();
    return EFI_SUCCESS;
  }

  ///
  /// Return from function if the DebugMode bit is set
  ///
  if (CsmeChipsetInitVerInfoPtr->DebugMode == 1) {
    DEBUG ((DEBUG_INFO, "Exit function as Debug bit set in the Version Information field obtained from MBP\n"));
    DEBUG ((DEBUG_INFO, "ChipsetInitSync () End\n"));
    return EFI_SUCCESS;
  }

  ///
  /// Locate gPchHsioChipsetInitSusTblDataPpiGuid
  ///
  ClearSusValidBit = FALSE;
  ModPhySusTblSize = 0;
  Usb2SusTblSize = 0;
  SusTblPtr = NULL;
  EndTableMarker = 0xF;
  Status = PeiServicesLocatePpi (
             &gPchHsioChipsetInitSusTblDataPpiGuid,
             0,
             NULL,
             (VOID **) &PchHsioChipsetInitSusTblData
             );
  if (Status == EFI_SUCCESS) {
    ///
    /// Allocate memory for SUS Table
    ///
    SusTblPtr = AllocateZeroPool (PCH_HSIO_CHIPSETINIT_USB2_SUS_MAX_SIZE + PCH_HSIO_CHIPSETINIT_MODPHY_SUS_MAX_SIZE);
    if (SusTblPtr == NULL) {
      DEBUG ((DEBUG_ERROR, "ChipsetInitSync: Could not allocate Memory\n"));
      return EFI_OUT_OF_RESOURCES;
    }
    ///
    /// Copy USB2 and ModPhy Tuning tables to the allocate SUS table memory
    ///
    Usb2SusTblSize = PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningCount * sizeof (PCH_HSIO_CMD_FIELD);
    ModPhySusTblSize = PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningCount * sizeof (PCH_HSIO_CMD_FIELD);
    if (PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningCount) {
      CopyMem (SusTblPtr, (VOID *) PchHsioChipsetInitSusTblData->PchHsioSusUsb2TuningPtr, Usb2SusTblSize);
      CopyMem (SusTblPtr + Usb2SusTblSize, (VOID *) &EndTableMarker, sizeof (EndTableMarker));
      Usb2SusTblSize = Usb2SusTblSize + sizeof (EndTableMarker); // End table marker is 4 bytes
    }
    if (PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningCount) {
      CopyMem (SusTblPtr + Usb2SusTblSize, (VOID *) PchHsioChipsetInitSusTblData->PchHsioSusModPhyTuningPtr,  ModPhySusTblSize);
      CopyMem (SusTblPtr + Usb2SusTblSize + ModPhySusTblSize, (VOID *) &EndTableMarker, sizeof (EndTableMarker));
      ModPhySusTblSize = ModPhySusTblSize + sizeof (EndTableMarker); // End table marker is 4 bytes
    }


    ///
    /// Calculate SUS Table CRC
    ///
    BiosComputedSusCrc = PchHsioCalculateCrc16 (SusTblPtr, Usb2SusTblSize + ModPhySusTblSize);
    DEBUG ((DEBUG_INFO, "Computing BIOS Computed SUS Table CRC: %x\n", BiosComputedSusCrc));
  } else {
    DEBUG ((DEBUG_INFO, "SUS Table not generated by BIOS\n"));
    BiosComputedSusCrc = 0xFFFF;
    ClearSusValidBit = TRUE;
  }

  ///
  /// Get Policy settings through the SiPolicy PPI
  ///
  HsioConfig = NULL;
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicyPpi
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SiPolicyPpi is not located! Error: %r\n", Status));
    return EFI_UNSUPPORTED;
  }
  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gHsioConfigGuid, (VOID *) &HsioConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Allocate memory for maximum ChipsetInit Table size and HECI Message Header
  ///
  ChipsetInitTblLen = PCH_HSIO_CHIPSETINIT_TBL_MAX_SIZE;
  ChipsetInitTblPtr =  AllocateZeroPool (ChipsetInitTblLen);
  if (ChipsetInitTblPtr == NULL) {
    DEBUG ((DEBUG_ERROR, "ChipsetInitSync: Could not allocate Memory\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  SkipCsmeChipsetInitSync = FALSE;
  if ((HsioConfig->ChipsetInitBinPtr == 0) ||
    (HsioConfig->ChipsetInitBinLen == 0)) {
    if ((CsmeChipsetInitVerInfoPtr->SusCrc == BiosComputedSusCrc) &&
      (CsmeChipsetInitVerInfoPtr->SusCrcValid == 1)) {
      ///
      /// No ChipsetInit BIOS <-> CSME sync required as all CRCs and Valid bits match
      ///
      DEBUG ((DEBUG_INFO, "No ChipsetInit BIOS <-> CSME sync required as all CRCs and Valid bits match\n"));
      SkipCsmeChipsetInitSync = TRUE;
    } else {
      ///
      /// Read ChipsetInit Binary from CSME
      /// The Buffer passed in contains space for the ChipsetInit Table as well as HSIO_WRITE_SETTINGS_REQ
      ///
      DEBUG ((DEBUG_INFO, "Read ChipsetInit Binary from CSME\n"));
      Status = PeiHeciReadChipsetInitMsg (ChipsetInitTblPtr, &ChipsetInitTblLen);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Unable to read the ChipsetInit binary from CSME: %r\n", Status));
        ASSERT_EFI_ERROR (Status);
      }
      BiosChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) ChipsetInitTblPtr;
    }
  } else {
    BiosChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) HsioConfig->ChipsetInitBinPtr;
    if ((CsmeChipsetInitVerInfoPtr->BaseCrc == BiosChipsetInitVerInfoPtr->BaseCrc) &&
      (CsmeChipsetInitVerInfoPtr->OemCrc == BiosChipsetInitVerInfoPtr->OemCrc) &&
      (CsmeChipsetInitVerInfoPtr->BaseCrcValid == BiosChipsetInitVerInfoPtr->BaseCrcValid) &&
      (CsmeChipsetInitVerInfoPtr->OemCrcValid == BiosChipsetInitVerInfoPtr->OemCrcValid) &&
      (CsmeChipsetInitVerInfoPtr->SusCrc == BiosComputedSusCrc) &&
      (CsmeChipsetInitVerInfoPtr->SusCrcValid == 1)) {
      ///
      /// No ChipsetInit BIOS <-> CSME sync required as all CRCs and Valid bits match
      ///
      DEBUG ((DEBUG_INFO, "No ChipsetInit BIOS <-> CSME sync required as all CRCs and Valid bits match\n"));
      SkipCsmeChipsetInitSync = TRUE;
    } else {
      ///
      /// Use ChipsetInit Table embedded in BIOS
      ///
      DEBUG ((DEBUG_INFO, "Using ChipsetInit Table embedded in BIOS\n"));
      DEBUG ((DEBUG_INFO, "ChipsetInit Binary Location: %x\n", HsioConfig->ChipsetInitBinPtr));
      DEBUG ((DEBUG_INFO, "ChipsetInit Binary Size: %x\n", HsioConfig->ChipsetInitBinLen));
      CopyMem (ChipsetInitTblPtr, (VOID *) HsioConfig->ChipsetInitBinPtr, HsioConfig->ChipsetInitBinLen);
      ChipsetInitTblLen = HsioConfig->ChipsetInitBinLen;
    }
  }

  if (SkipCsmeChipsetInitSync) {
    BaseCrc = CsmeChipsetInitVerInfoPtr->BaseCrc;
    OemCrc = CsmeChipsetInitVerInfoPtr->OemCrc;
    Version = CsmeChipsetInitVerInfoPtr->Version;
    OemVersion = CsmeChipsetInitVerInfoPtr->OemVersion;
  } else {
    ///
    /// Update SUS CRC and SUS Valid Bit in the BIOS version
    ///
    DEBUG ((DEBUG_INFO, "Update SUS CRC and SUS Valid Bit in the BIOS version\n"));

    ChipsetInitVerInfoPtr = (PCH_HSIO_VER_INFO *) ChipsetInitTblPtr;

    BaseCrc = ChipsetInitVerInfoPtr->BaseCrc;
    OemCrc = ChipsetInitVerInfoPtr->OemCrc;
    Version = ChipsetInitVerInfoPtr->Version;
    OemVersion = ChipsetInitVerInfoPtr->OemVersion;

    if (ClearSusValidBit) {
      DEBUG ((DEBUG_INFO, "Clearing SUS CRC Valid bit in Version Information\n"));
      ChipsetInitVerInfoPtr->SusCrc = 0;
      ChipsetInitVerInfoPtr->SusCrcValid = 0;
    } else {
      DEBUG ((DEBUG_INFO, "Updating SUS CRC and setting SUS CRC Valid bit in Version Information\n"));
      ChipsetInitVerInfoPtr->SusCrc = BiosComputedSusCrc;
      ChipsetInitVerInfoPtr->SusCrcValid = 1;
    }

    ///
    /// Update, Delete or Append ChipsetInit Table
    ///
    DEBUG ((DEBUG_INFO, "Update, Delete or Append ChipsetInit Table\n"));
    Status = UpdateSusTable (ChipsetInitTblPtr, SusTblPtr, &ChipsetInitTblLen, Usb2SusTblSize, ModPhySusTblSize);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ChipsetInitSync: Error updating SUS table %r\n", Status));
      ASSERT_EFI_ERROR (Status);
    }


    ///
    /// Push Updated ChipsetInit table with CSME
    /// The Buffer passed in contains space for the HSIO_WRITE_SETTINGS_REQ structure as well as  ChipsetInit Table
    ///
    DEBUG ((DEBUG_INFO, "Push Updated ChipsetInit table with CSME\n"));
    Status = PeiHeciWriteChipsetInitMsg (ChipsetInitTblPtr, ChipsetInitTblLen);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ChipsetInit Sync Error: %r\n", Status));
      ASSERT_EFI_ERROR (Status);
      return EFI_UNSUPPORTED;
    }

    if ((CsmeChipsetInitVerInfoPtr->BaseCrc == BiosChipsetInitVerInfoPtr->BaseCrc) &&
        (CsmeChipsetInitVerInfoPtr->BaseCrcValid == BiosChipsetInitVerInfoPtr->BaseCrcValid) &&
        (CsmeChipsetInitVerInfoPtr->OemCrc == BiosChipsetInitVerInfoPtr->OemCrc) &&
        (CsmeChipsetInitVerInfoPtr->OemCrcValid == BiosChipsetInitVerInfoPtr->OemCrcValid) &&
        ((CsmeChipsetInitVerInfoPtr->SusCrc == BiosChipsetInitVerInfoPtr->SusCrc) ||
         (CsmeChipsetInitVerInfoPtr->SusCrcValid == 0))) {
      DEBUG ((DEBUG_INFO, "No global reset required\n"));
    } else {
      DEBUG ((DEBUG_INFO, "Trigger global reset\n"));
      CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
      StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
      SiScheduleResetSetType (EfiResetPlatformSpecific, &ResetData);
    }
  }

  //
  // Initialize ChipsetInitHob
  //
  ZeroMem (&ChipsetInitHobStruct, sizeof (CHIPSET_INIT_INFO));
  ChipsetInitHobStruct.Revision    = CHIPSET_INIT_INFO_REVISION;
  ChipsetInitHobStruct.BaseVersion = Version;
  ChipsetInitHobStruct.OemVersion  = OemVersion;
  ChipsetInitHobStruct.BaseCrc     = BaseCrc;
  ChipsetInitHobStruct.SusCrc      = BiosComputedSusCrc;
  ChipsetInitHobStruct.OemCrc      = OemCrc;

  ChipsetInitHob = BuildGuidDataHob (
                      &gChipsetInitHobGuid,
                      &ChipsetInitHobStruct,
                      sizeof (CHIPSET_INIT_INFO)
                      );
  if (ChipsetInitHob == NULL) {
    DEBUG ((DEBUG_ERROR, "(Hsio) ChipsetInitHob could not be created\n"));
    ASSERT (FALSE);
    return EFI_ABORTED;
  }

  DEBUG ((DEBUG_INFO, "ChipsetInitSync() End\n"));
  return Status;
}
