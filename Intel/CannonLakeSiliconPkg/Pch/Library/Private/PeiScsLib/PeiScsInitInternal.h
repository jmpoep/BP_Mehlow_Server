/** @file
  SCS PEI init library header file.

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

#ifndef _PEI_SCS_INTERNAL_H_
#define _PEI_SCS_INTERNAL_H_

#include <Uefi/UefiBaseType.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchInfoLib.h>
#include <Library/HobLib.h>
#include <Library/TimerLib.h>
#include <Library/PchPcrLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/PchPsfPrivateLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PeiItssLib.h>
#include <ConfigBlock/ScsConfig.h>
#include <Ppi/SiPolicy.h>
#include <Register/PchRegsScs.h>
#include "PeiIosf2Ocp.h"

/**
  Programs passed MmioBase address into BAR register.

  @param[in] PciBaseAddress  Address of the PCI config space
  @param[in] MmioBase        Address to be used to access MMIO space
**/
VOID
ScsControllerEnableMmio (
  IN UINT64  PciBaseAddress,
  IN UINTN   MmioAddress
  );

/**
  This function clears BAR address.

  @param[in] PciBaseAddress  Address of PCI config space
**/
VOID
ScsControllerDisableMmio (
  IN UINT64  PciBaseAddress
  );

/**
  This function puts SCS controller into D3 state

  @param[in] PciBaseAddress  Address of the PCI config space
**/
VOID
ScsControllerPutToD3 (
  IN UINT64  PciBaseAddress
  );

typedef struct {
  PCH_SBI_PID   PortId;
  UINT8         LaneMask;
} SCS_UFS_MMP;

/**
  Enables or disables UFS host controller.

  @param[in] SiPolicy            Pointer to SI_POLICY_PPI
  @param[in] ScsConfig           Pointer to UFS configuration
  @param[in] UfsIndex            Index of the UFS controler to init
  @param[in] TempMemBaseAddress  Temporary MMIO address to be used during MMIO init
**/
VOID
ScsUfsInit (
  IN SI_POLICY_PPI  *SiPolicy,
  IN PCH_SCS_CONFIG *ScsConfig,
  IN UINT8          UfsIndex,
  IN UINTN          TempMemBaseAddress
  );

/**
  Applies overrides to MMP lane as described in override table

  @param[in] PortId             Sideband port id of the MMP
  @param[in] MmpLaneIndex       Lane index of the MMP
**/
VOID
MmpApplyOverride (
  IN PCH_SBI_PID   PortId,
  IN UINT8         MmpLaneIndex
  );

/**
  Writes a 1 byte value to specified MMP Private register

  @param[in] PortId          MMP port id
  @param[in] MmpLaneIndex    MMP lane index
  @param[in] RegisterOffset  Offset of a private register
  @param[in] Value           Value to be written
**/
VOID
MmpPrivateWrite8 (
  IN PCH_SBI_PID  PortId,
  IN UINT8        MmpLaneIndex,
  IN UINT16       RegisterOffset,
  IN UINT8        Value
  );

/**
  Initializes MMP instance

  @param[in] MmpInstance        MMP instance
**/
VOID
MmpInit (
  IN SCS_UFS_MMP   *MmpInstance
  );

/**
  Disables MMP lanes

  @param[in] MmpInstance  MMP instance
**/
VOID
MmpDisable (
  IN SCS_UFS_MMP  *MmpInstance
  );

typedef struct {
  UINT32  TxCmdDelayControl;      // Offset 820h: Tx CMD Delay Control
  UINT32  TxDataDelayControl1;    // Offset 824h: Tx Data Delay Control 1
  UINT32  TxDataDelayControl2;    // Offset 828h: Tx Data Delay Control 2
  UINT32  RxCmdDataDelayControl1; // Offset 82Ch: Rx CMD + Data Delay Control 1
  UINT32  RxCmdDataDelayControl2; // Offset 834h: Rx CMD + Data Delay Control 2
  UINT32  RxStrobeDelayControl;   // Offset 830h: Rx Strobe Delay Control
} SCS_SD_DLL;

typedef struct {
  UINT32  CapReg1;
  UINT32  CapReg2;
} SCS_SD_CAPS;

/**
  Enables or disables eMMC host controller.

  @param[in] SiPolicy            Pointer to SI_POLICY_PPI
  @param[in] ScsConfig           Pointer to PCH_SCS_CONFIG
  @param[in] TempMemBaseaddress  Temporary MMIO address to use during MMIO init
**/
VOID
ScsEmmcInit (
  IN SI_POLICY_PPI   *SiPolicy,
  IN PCH_SCS_CONFIG  *ScsConfig,
  IN UINTN           TempMemBaseAddress
  );

/**
  Enables or disables SdCard host controller.

  @param[in] SiPolicy            Pointer to SI_POLICY_PPI
  @param[in] ScsConfig           Pointer to SdCard configuration
  @param[in] TempMemBaseAddress  Temporary MMIO address to use during MMIO init
**/
VOID
ScsSdCardInit (
  IN SI_POLICY_PPI   *SiPolicy,
  IN PCH_SCS_CONFIG  *ScsConfig,
  IN UINTN           TempMemBaseAddress
  );

/**
  Gets the device and function number of the SdCard.

  @param[out] DeviceNum    Pointer to the variable to store device number
  @param[out] FunctionNum  Pointer to the variable to store function number
**/
VOID
ScsGetSdCardBdf (
  OUT UINT8  *DeviceNum,
  OUT UINT8  *FunctionNum
  );

/**
  Gets the default DLL values for SdCard controller.

  @return SCS_SD_DLL  Pointer to SdCard default DLL
**/
SCS_SD_DLL*
ScsGetSdCardDefaultDll (
  VOID
  );

/**
  Gets the default capabilities of SdCard controller.

  @return SCS_SD_CAPS  Pointer to SdCard capabilities
**/
SCS_SD_CAPS*
ScsGetSdCardDefaultCapabilities (
  VOID
  );

/**
  Used to check if SdCard supports 64 bit addressing.

  @retval TRUE   SdCard supports 64 bit addressing
  @retval FALSE  SdCard doesn't support 64 bit addressing
**/
BOOLEAN
ScsSdCardIs64bitAddressingSupported (
  VOID
  );

/**
  Gets the device and function number of the eMMC.

  @param[out] DeviceNum    Pointer to the variable to store device number
  @param[out] FunctionNum  Pointer to the variable to store function number
**/
VOID
ScsGetEmmcBdf (
  OUT UINT8  *DeviceNum,
  OUT UINT8  *FunctionNum
  );

/**
  Gets the default DLL values for eMMC controller.

  @return SCS_SD_DLL  Pointer to eMMC default DLL
**/
SCS_SD_DLL*
ScsGetEmmcDefaultDll (
  VOID
  );

/**
  Gets the default capabilities of eMMC controller.

  @return SCS_SD_CAPS  Pointer to the eMMC capabilities
**/
SCS_SD_CAPS*
ScsGetEmmcDefaultCapabilities (
  VOID
  );

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
  );

/**
  Returns the MMP descriptor that given UFS controller is using.

  @param[in] UfsIndx  Index of the UFS controller

  @return SCS_UFS_MMP  Pointer to the MMP instance
**/
SCS_UFS_MMP*
ScsGetUfsMmp (
  IN UINT8  UfsIndex
  );

/**
  Checks if given UFS is used as a boot medium.

  @param[in] UfsIndex  Index of the UFS controller

  @retval  TRUE   UFS is used as a boot medium
  @retval  FALSE  UFS is not used as a boot medium
**/
BOOLEAN
ScsIsUfsBootMedium (
  IN UINT8  UfsIndex
  );

#endif

