/** @file
  Initializes Serial IO Controllers.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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
#include <Library/BaseMemoryLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>

#include "PchInit.h"
#include <Library/PchSerialIoLib.h>
#include <Library/DevicePathLib.h>
#include <IndustryStandard/Pci30.h>
#include <Register/PchRegsSerialIo.h>

typedef struct {
  ACPI_HID_DEVICE_PATH          RootPort;
  ACPI_EXTENDED_HID_DEVICE_PATH AcpiDev;
  CHAR8                         HidString[8];
  CHAR8                         UidString;
  CHAR8                         CidString;
  EFI_DEVICE_PATH_PROTOCOL      End;
} SERIALIO_DEVICE_PATH;

#define gPciRootBridge {{ACPI_DEVICE_PATH, ACPI_DP, {(UINT8)(sizeof(ACPI_HID_DEVICE_PATH)), 0}}, EISA_PNP_ID (0x0A03), 0}
#define gAcpiDev {{ACPI_DEVICE_PATH,ACPI_EXTENDED_DP,{(UINT8)(sizeof(ACPI_EXTENDED_HID_DEVICE_PATH)+SERIALIO_TOTAL_ID_LENGTH),0}},0,0,0}
#define gEndEntire {END_DEVICE_PATH_TYPE,END_ENTIRE_DEVICE_PATH_SUBTYPE,{END_DEVICE_PATH_LENGTH,0}}

GLOBAL_REMOVE_IF_UNREFERENCED SERIALIO_DEVICE_PATH gSerialIoPath = {
  gPciRootBridge,
  gAcpiDev,
  "\0\0\0\0\0\0\0",
  '\0',
  '\0',
  gEndEntire
};

/**
Mark memory used by SerialIo devices in ACPI mode as allocated

@retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
AllocateSerialIoMemory (
  VOID
  )
{
  PCH_SERIAL_IO_CONTROLLER i;
  UINT8                    BarNumber;
  UINTN                    Bar;
  EFI_STATUS               Status;

  for (i=0; i<PchSerialIoIndexMax; i++) {
    if (mPchConfigHob->SerialIo.DevMode[i] == PchSerialIoHidden ||
        mPchConfigHob->SerialIo.DevMode[i] == PchSerialIoAcpi) {
      for (BarNumber = 0; BarNumber<=1; BarNumber++) {
        Bar = FindSerialIoBar (i,BarNumber);
        Status = gDS->AddMemorySpace (
                        EfiGcdMemoryTypeReserved,
                        Bar,
                        V_SERIAL_IO_CFG_BAR_SIZE,
                        0
                        );
        ASSERT_EFI_ERROR (Status);
        if (EFI_ERROR (Status)) {
          return Status;
        }
        Status = gDS->AllocateMemorySpace (
                        EfiGcdAllocateAddress,
                        EfiGcdMemoryTypeReserved,
                        N_SERIAL_IO_CFG_BAR_ALIGNMENT,
                        V_SERIAL_IO_CFG_BAR_SIZE,
                        &Bar,
                        mImageHandle,
                        NULL
                        );
        ASSERT_EFI_ERROR (Status);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }
    }
  }
  return EFI_SUCCESS;
}

VOID
CreateSerialIoHandles (
  VOID
  )
{
  EFI_HANDLE NewHandle;
  EFI_DEVICE_PATH_PROTOCOL *NewPath;
  UINT32 Controller;

  for (Controller = 0; Controller < PchSerialIoIndexMax; Controller++) {
    if (mPchConfigHob->SerialIo.DevMode[Controller] == PchSerialIoAcpi) {
      NewHandle = NULL;
      CopyMem (gSerialIoPath.HidString, GetSerialIoAcpiHid (Controller), SERIALIO_HID_LENGTH);
      NewPath = DuplicateDevicePath ((EFI_DEVICE_PATH_PROTOCOL*)&gSerialIoPath);
      gBS->InstallMultipleProtocolInterfaces (
             &NewHandle,
             &gEfiDevicePathProtocolGuid,
             NewPath,
             NULL );
    }
  }
}

/**
  Puts all SerialIo controllers (except UARTs in debug mode) in D3.
  Clears MemoryEnable for all PCI-mode controllers on S3 resume
**/
VOID
ConfigureSerialIoAtS3Resume (
  VOID
  )
{
  PCH_SERIAL_IO_CONTROLLER Index;
  UINTN                    PciCfgBase;
  UINT32                   Data32;

  for (Index = 0; Index < PchSerialIoIndexMax; Index++) {
    if (mPchConfigHob->SerialIo.DevMode[Index] == PchSerialIoDisabled) {
      if (IsSerialIoFunctionZero (Index)) {
        if (IsSerialIoDeviceEnabled (GetSerialIoDeviceNumber (Index), GetSerialIoFunctionNumber (Index))) {
          PciCfgBase = FindSerialIoBar (Index,1);
          Data32 = MmioRead32 (PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS);
          Data32 |= B_SERIAL_IO_CFG_PME_CTRL_STS_PWR_ST;
          S3BootScriptSaveMemWrite (S3BootScriptWidthUint32, PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS, 1, &Data32);
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
    Data32 = MmioRead32 (PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS);
    Data32 |= B_SERIAL_IO_CFG_PME_CTRL_STS_PWR_ST;
    S3BootScriptSaveMemWrite (S3BootScriptWidthUint32, PciCfgBase + R_SERIAL_IO_CFG_PME_CTRL_STS, 1, &Data32);
    if (mPchConfigHob->SerialIo.DevMode[Index] == PchSerialIoPci) {
      Data32 = MmioRead32 (PciCfgBase + PCI_COMMAND_OFFSET);
      Data32 &= (UINT32)~(EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER);
      S3BootScriptSaveMemWrite (S3BootScriptWidthUint32, PciCfgBase + PCI_COMMAND_OFFSET, 1, &Data32);
    }
  }
}
