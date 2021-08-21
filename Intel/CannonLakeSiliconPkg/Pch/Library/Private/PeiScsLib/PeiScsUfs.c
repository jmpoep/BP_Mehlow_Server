/** @file
  Initializes UFS host controller located on SCS Iosf2Ocp bridge.

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

#include <Register/PchRegs.h>
#include <Register/PchRegsScsUfs.h>
#include "PeiScsInitInternal.h"

/**
  Waits until test condition is satisfied. This function guarantees that
  timeout will be at least as long as the one specified.

  @param[in] RegisterAddress  Address of the register to test
  @param[in] RegisterMask     Value of the mask to apply to the register
  @param[in] TestValue        Test value against which a register is tested
  @param[in] TimeoutUs        Value of the timeout in us granularity

  @retval TRUE if register assumed test value before timeout, FALSE otherwise
**/
STATIC
BOOLEAN
WaitUntil (
  IN UINTN   RegisterAddress,
  IN UINT32  RegisterMask,
  IN UINT32  TestValue,
  IN UINT32  TimeoutUs
  )
{
  while (TimeoutUs > 0) {
    if ((MmioRead32 (RegisterAddress) & RegisterMask) == TestValue) {
      return TRUE;
    }
    MicroSecondDelay (10);
    TimeoutUs -= 10;
  }
  return FALSE;
}

/**
  Enables UFS host controller.

  @param[in] MmioBase Address of the MMIO base

  @retval TRUE if HC has been enabled, FALSE otherwise
**/
STATIC
BOOLEAN
ScsUfsEnableHc (
  IN UINTN  MmioBase
  )
{
  MmioWrite32 (MmioBase + R_SCS_MEM_UFS_EN, B_SCS_MEM_UFS_EN);
  return WaitUntil (MmioBase + R_SCS_MEM_UFS_EN, B_SCS_MEM_UFS_EN, B_SCS_MEM_UFS_EN, 50);
}

/**
  Performs UIC layer programming.
  This function assumes that UFS HC has already been enabled.

  @param[in] MmioBase  Address of the MMIO base
**/
STATIC
VOID
ScsUfsUicLayerProgramming (
  IN UINTN  MmioBase
  )
{
  BOOLEAN ControllerReady;

  //
  // Clear IS.UCCS status
  //
  MmioAnd32 (MmioBase + R_SCS_MEM_UFS_IS, ~(UINT32)B_SCS_MEM_UFS_IS_UCCS);

  ControllerReady = WaitUntil (
                      MmioBase + R_SCS_MEM_UFS_HC_STATUS,
                      B_SCS_MEM_UFS_HCS_UCRDY,
                      B_SCS_MEM_UFS_HCS_UCRDY,
                      50);
  if (ControllerReady) {
    //
    // We only have to set one attribute in UIC layer - LCCEnable(0x155E0000) = 0
    //
    MmioWrite32 (MmioBase + R_SCS_MEM_UFS_UCMD_ARG1, 0x155E0000);
    MmioWrite32 (MmioBase + R_SCS_MEM_UFS_UCMD_ARG2, 0x0);
    MmioWrite32 (MmioBase + R_SCS_MEM_UFS_UCMD_ARG3, 0x0);
    MmioWrite32 (MmioBase + R_SCS_MEM_UFS_UIC_CMD, V_SCS_MEM_UFS_UIC_CMD_DME_SET);
  } else {
    DEBUG ((DEBUG_ERROR, "UFS not ready for UIC programming\n"));
    ASSERT (FALSE);
  }
}

/**
  Configure Ufs controller

  @param[in] PciBaseAddress  PCI config base address of the controller
  @param[in] MmioBase        MMIO base address
**/
STATIC
VOID
ScsUfsInitMmioRegisters (
  IN UINT64  PciBaseAddress,
  IN UINTN   MmioBase
  )
{
  ScsControllerEnableMmio (PciBaseAddress, MmioBase);
  //
  // Configure UFS REF_CLK
  //
  MmioWrite32 (MmioBase + R_SCS_MEM_UFS_REF_CLK, V_SCS_MEM_UFS_REF_CLK);

  //
  // Enable host controller and perform UIC layer programming.
  // Reference code has to leave UFS enabled after performing this programming since UIC settings are volatile.
  // UFS pre-boot driver is assumed not to reset host controller during controller init.
  //
  if (ScsUfsEnableHc (MmioBase)) {
    ScsUfsUicLayerProgramming (MmioBase);
  } else {
    DEBUG ((DEBUG_ERROR, "Failed to enable UFS host controller!\n"));
  }
  ScsControllerDisableMmio (PciBaseAddress);
}

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
  )
{
  UINT64         UfsBaseAddress;
  IOSF2OCP_PORT  *UfsIosf2OcpPort;
  UINT8          DeviceNum;
  UINT8          FunctionNum;
  SCS_UFS_MMP    *Mmp;
  BOOLEAN        IsBootMedium;
  UINT8          InterruptPin;
  UINT8          Irq;

  DEBUG ((DEBUG_INFO, "UFS init start, UFS index = %d\n", UfsIndex));
  ScsGetUfsBdf (UfsIndex, &DeviceNum, &FunctionNum);
  UfsBaseAddress = PCI_SEGMENT_LIB_ADDRESS (
                     DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     DeviceNum,
                     FunctionNum,
                     0
                     );
  UfsIosf2OcpPort = Iosf2OcpGetUfsPort (UfsIndex);
  Mmp = ScsGetUfsMmp (UfsIndex);
  IsBootMedium = ScsIsUfsBootMedium (UfsIndex);

  Iosf2OcpDisableBar1 (UfsIosf2OcpPort);
  PsfDisableDeviceBar (PsfScsUfsPort (UfsIndex), (BIT3 | BIT2));
  if (ScsConfig->ScsUfsEnabled) {
    MmpInit (Mmp);
    if (!IsBootMedium) {
      Iosf2OcpEnableUfs (UfsIosf2OcpPort);
      ScsUfsInitMmioRegisters (UfsBaseAddress, TempMemBaseAddress);
    }
    ItssGetDevIntConfig (
      SiPolicy,
      DeviceNum,
      FunctionNum,
      &InterruptPin,
      &Irq
      );
    Iosf2OcpConfigureInterrupts (UfsIosf2OcpPort, InterruptPin, Irq);
  } else {
    if (!IsBootMedium) {
      DEBUG ((DEBUG_INFO, "Disabling UFS\n"));
      PciSegmentOr16 (
        UfsBaseAddress + R_SCS_CFG_PG_CONFIG,
        (B_SCS_CFG_PG_CONFIG_SE| B_SCS_CFG_PG_CONFIG_PGE | B_SCS_CFG_PG_CONFIG_I3E)
        );
      ScsControllerPutToD3 (UfsBaseAddress);
      MmpDisable (Mmp);
      Iosf2OcpDisableUfs (UfsIosf2OcpPort);
      PsfDisableDevice (PsfScsUfsPort (UfsIndex));
      PmcDisableScsUfs (UfsIndex);
    }
  }
  DEBUG ((DEBUG_INFO, "UFS init finished\n"));
}

