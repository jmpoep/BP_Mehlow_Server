/** @file
  Initializes SCS Iosf2Ocp bridges.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2019 Intel Corporation.

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

#include "PeiScsInitInternal.h"
#include <Register/PchRegsScsCnl.h>
#include <Register/PchRegsScsUfs.h>

typedef struct {
  UINT16  RegisterOffset;
  UINT8   Value;
} MMP_OVERRIDE;

MMP_OVERRIDE  mMmpA0OverrideTable[] = {
  {R_SCS_PCR_MMPUFS_DLANEX_IMP24_TX_TOB_EXTN, 0x1},
  {R_SCS_PCR_MMPUFS_DLANEX_IMP25_MK0_DETECT_LSB, 0x0},
  {R_SCS_PCR_MMPUFS_DLANEX_IMP25_MK0_DETECT_MSB, 0x3},
  {R_SCS_PCR_MMPUFS_DLANEX_RX3_HS_G1_SYNC_LEN_CAP, 0x4B},
  {R_SCS_PCR_MMPUFS_DLANEX_RX6_HS_G2_SYNC_LEN_CAP, 0x4B},
  {R_SCS_PCR_MMPUFS_DLANEX_RX6_HS_G3_SYNC_LEN_CAP, 0x4B},
  {R_SCS_PCR_MMPUFS_DLANEX_IMP21_RX_SQUELCH_ENABLE, 0x64},
  {R_SCS_PCR_MMPUFS_DLANEX_TX4_LCC_ENABLE, 0x0}
};

/**
  Applies overrides to MMP lane as described in override table

  @param[in] PortId             Sideband port id of the MMP
  @param[in] MmpLaneIndex       Lane index of the MMP
**/
VOID
MmpApplyOverride (
  IN PCH_SBI_PID   PortId,
  IN UINT8         MmpLaneIndex
  )
{
  UINT8  Index;

  if (PchStepping () < PCH_B0) {
    for (Index = 0; Index < ARRAY_SIZE (mMmpA0OverrideTable); Index++) {
      DEBUG ((DEBUG_INFO, "MMP: applying override, offset = %X, value = %X\n", mMmpA0OverrideTable[Index].RegisterOffset, mMmpA0OverrideTable[Index].Value));
      MmpPrivateWrite8 (
        PortId,
        MmpLaneIndex,
        mMmpA0OverrideTable[Index].RegisterOffset,
        mMmpA0OverrideTable[Index].Value
        );
    }
  }
}

/**
  Gets the device and function number of the SdCard.

  @param[out] DeviceNum    Pointer to the variable to store device number
  @param[out] FunctionNum  Pointer to the variable to store function number
**/
VOID
ScsGetSdCardBdf (
  OUT UINT8  *DeviceNum,
  OUT UINT8  *FunctionNum
  )
{
  *DeviceNum = PCI_DEVICE_NUMBER_PCH_CNL_SCS_SDCARD;
  *FunctionNum = PCI_FUNCTION_NUMBER_PCH_CNL_SCS_SDCARD;
}

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL mSdCardLpCflDll = {0x0505, 0x0A0F, 0x252B2828, 0x2B073C3C, 0x21813, 0x0};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL mSdCardLpCnlDll = {0x0505, 0x0A0F, 0x252b2828, 0x2b073c3c, 0x21813, 0x0};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL mSdCardLpWhlDll = {0x0505, 0x0A0E, 0x22262828, 0x2A2B3B3B, 0x10016, 0x0};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL mSdCardHCflDll = {0x0505, 0x0A0F, 0x25272828, 0x25242e2e, 0x10001, 0x0};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL mSdCardHCnlDll = {0x0505, 0x0A10, 0x25242828, 0x0A0B1414, 0x1806, 0x0};

/**
  Gets the default DLL values for SdCard controller.

  @return SCS_SD_DLL  Pointer to SdCard default DLL
**/
SCS_SD_DLL*
ScsGetSdCardDefaultDll (
  VOID
  )
{
  if (PcdGetBool (PcdCflCpuEnable)) {
    if (IsPchLp ()) {
      if (IsWhlCpu()) {
        return &mSdCardLpWhlDll;
      } else {
        return &mSdCardLpCflDll;
      }
    } else {
      return &mSdCardHCflDll;
    }
  }
  if (IsPchLp ()) {
    return &mSdCardLpCnlDll;
  } else {
    return &mSdCardHCnlDll;
  }
}

SCS_SD_CAPS mSdCardCaps = {0x1050E75C, 0x40000C8};

/**
  Gets the default capabilities of SdCard controller.

  @return SCS_SD_CAPS  Pointer to SdCard capabilities
**/
SCS_SD_CAPS*
ScsGetSdCardDefaultCapabilities (
  VOID
  )
{
  return &mSdCardCaps;
}

/**
  Used to check if SdCard supports 64 bit addressing.

  @retval TRUE   SdCard supports 64 bit addressing
  @retval FALSE  SdCard doesn't support 64 bit addressing
**/
BOOLEAN
ScsSdCardIs64bitAddressingSupported (
  VOID
  )
{
  if (IsCnlPch () && IsPchLp () && (PchStepping () < PCH_B0)) {
    return FALSE;
  }
  return TRUE;
}

/**
  Gets the device and function number of the eMMC.

  @param[out] DeviceNum    Pointer to the variable to store device number
  @param[out] FunctionNum  Pointer to the variable to store function number
**/
VOID
ScsGetEmmcBdf (
  OUT UINT8  *DeviceNum,
  OUT UINT8  *FunctionNum
  )
{
  *DeviceNum = PCI_DEVICE_NUMBER_PCH_CNL_SCS_EMMC;
  *FunctionNum = PCI_FUNCTION_NUMBER_PCH_CNL_SCS_EMMC;
}

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL  mEmmcCflDll = {0x505, 0x0B0B, 0x1C292828, 0x1C0B5F32, 0x20008, 0x1818};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL  mEmmcWhlDll = {0x505, 0x0C0B, 0x1C292929, 0x1C101616, 0x20006, 0x1515};

GLOBAL_REMOVE_IF_UNREFERENCED SCS_SD_DLL  mEmmcCnlDll = {0x505, 0x0B0B, 0x1C292929, 0x1C101616, 0x21806, 0x1616};

/**
  Gets the default DLL values for eMMC controller.

  @return SCS_SD_DLL  Pointer to eMMC default DLL
**/
SCS_SD_DLL*
ScsGetEmmcDefaultDll (
  VOID
  )
{
  if (PcdGetBool (PcdCflCpuEnable)) {
    if (IsWhlCpu ()) {
      return &mEmmcWhlDll;
    } else {
      return &mEmmcCflDll;
    }
  }
  return &mEmmcCnlDll;
}

SCS_SD_CAPS mEmmcCaps = {0x3050EB1E, 0x40040C8};

/**
  Gets the default capabilities of eMMC controller.

  @return SCS_SD_CAPS  Pointer to the eMMC capabilities
**/
SCS_SD_CAPS*
ScsGetEmmcDefaultCapabilities (
  VOID
  )
{
  return &mEmmcCaps;
}

/**
  Gets the device and function number of the UFS

  @param[in]  UfsIndex     Index of the UFS controller
  @param[out] DeviceNum    Pointer to the variable to store device number
  @param[out] FunctionNum  Pointer to the variable to store function number
**/
VOID
ScsGetUfsBdf (
  IN  UINT8  UfsIndex,
  OUT UINT8  *DeviceNum,
  OUT UINT8  *FunctionNum
  )
{
  if (UfsIndex != 0) {
    ASSERT (FALSE);
    return;
  }
  *DeviceNum = PCI_DEVICE_NUMBER_PCH_CNL_SCS_UFS;
  *FunctionNum = PCI_FUNCTION_NUMBER_PCH_CNL_SCS_UFS;
}

SCS_UFS_MMP  mUfsMmp = {
  PID_MMP_UFSX2,
  (BIT1 | BIT0)
  };

/**
  Returns the MMP descriptor that given UFS controller is using.

  @param[in] UfsIndx  Index of the UFS controller

  @return SCS_UFS_MMP  Pointer to the MMP instance
**/
SCS_UFS_MMP*
ScsGetUfsMmp (
  IN UINT8  UfsIndex
  )
{
  if (UfsIndex != 0) {
    ASSERT (FALSE);
    return NULL;
  }
  return &mUfsMmp;
}

/**
  Checks if given UFS is used as a boot medium.

  @param[in] UfsIndex  Index of the UFS controller

  @retval  TRUE   UFS is used as a boot medium
  @retval  FALSE  UFS is not used as a boot medium
**/
BOOLEAN
ScsIsUfsBootMedium (
  IN UINT8  UfsIndex
  )
{
  return FALSE;
}

