/** @file
  Initializes Serial IO Controllers.

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

@par Specification Reference:
**/
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include "PchInit.h"
#include <Library/PchSerialIoLib.h>
#include <IndustryStandard/Pci30.h>
#include <Register/PchRegsSerialIo.h>

/**
  Puts all SerialIo controllers (except UARTs in debug mode) in D3
  Clears MemoryEnable for all PCI-mode controllers
**/
EFI_STATUS
ConfigureSerialIoAtBoot (
  VOID
  )
{
  PCH_SERIAL_IO_CONTROLLER Index;
  UINTN                    PciCfgBase;

  for (Index = 0; Index < PchSerialIoIndexMax; Index++) {
    if (mPchConfigHob->SerialIo.DevMode[Index] == PchSerialIoDisabled) {
      if (IsSerialIoFunctionZero (Index)) {
        if (IsSerialIoDeviceEnabled (GetSerialIoDeviceNumber (Index), GetSerialIoFunctionNumber (Index))) {
          PciCfgBase = FindSerialIoBar (Index,1);
          MmioOr32 (PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS, B_SERIAL_IO_CFG_PME_CTRL_STS_PWR_ST);
        }
      }
      continue;
    }
    if ((Index >= PchSerialIoIndexUart0) &&
        (mPchConfigHob->SerialIo.EnableDebugUartAfterPost) &&
        (mPchConfigHob->SerialIo.DebugUartNumber == (UINT32) (Index - PchSerialIoIndexUart0))) {
      continue;
    }
    PciCfgBase = FindSerialIoBar (Index,1);
    MmioOr32 (PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS, B_SERIAL_IO_CFG_PME_CTRL_STS_PWR_ST);
    MmioRead32 (PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS);
    if (mPchConfigHob->SerialIo.DevMode[Index] == PchSerialIoPci) {
      MmioAnd32 (PciCfgBase + PCI_COMMAND_OFFSET, (UINT32)~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER) );
      if (mPchConfigHob->SerialIo.DebugUartNumber == (UINT32) (Index - PchSerialIoIndexUart0)) {
        continue;
      }
      MmioWrite32 (PciCfgBase + R_SERIAL_IO_CFG_BAR0_LOW, 0);
      MmioWrite32 (PciCfgBase + R_SERIAL_IO_CFG_BAR0_HIGH, 0);
    }
  }
  return EFI_SUCCESS;
}

