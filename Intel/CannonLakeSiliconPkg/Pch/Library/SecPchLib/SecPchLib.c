/** @file
  SEC PCH library in C.

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
#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchSerialIoLib.h>
#include <Library/PchSerialIoUartLib.h>
#include <Library/PmcLib.h>
#include <Library/PrintLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <PchReservedResources.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsLpc.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsP2sb.h>
#include <Register/PchRegsSmbus.h>
#include <IndustryStandard/Pci30.h>

#ifdef FSP_FLAG
#include <FspEas.h>
#include <FsptUpd.h>

/**
  This interface gets FspInfoHeader pointer

  @return   FSP binary base address.

**/
UINT32
EFIAPI
AsmGetFspInfoHeader (
  VOID
  );
#endif

/**
  This is helper function to initialize SerialIoUart in early init.
**/
VOID
EFIAPI
SerialIoUartConfiguration (
  VOID
  )
{
  UINT8     SerialIoUartDebugEnable;
  UINT8     SerialIoUartNumber;
  UINT32    SerialIoUartInputClock;
  UINT8     Uart0PinMuxing;
  CHAR8     CarInitBuffer[32];
#ifdef FSP_FLAG
  FSP_INFO_HEADER   *FspInfoHeader;
  FSPT_UPD          *FsptUpd;
  AsciiSPrint (CarInitBuffer, sizeof (CarInitBuffer), "FSP-T: CAR Init\n");

  FspInfoHeader = (FSP_INFO_HEADER *) AsmGetFspInfoHeader ();
  FsptUpd = (FSPT_UPD *) (FspInfoHeader->ImageBase + FspInfoHeader->CfgRegionOffset);
  SerialIoUartDebugEnable = FsptUpd->FsptConfig.PcdSerialIoUartDebugEnable;
  SerialIoUartNumber = FsptUpd->FsptConfig.PcdSerialIoUartNumber;
  SerialIoUartInputClock = FsptUpd->FsptConfig.PcdSerialIoUartInputClock;
  Uart0PinMuxing = FsptUpd->FsptConfig.PcdSerialIoUart0PinMuxing;
#else
  AsciiSPrint (CarInitBuffer, sizeof (CarInitBuffer), "CAR Init\n");
  SerialIoUartDebugEnable = PcdGet8 (PcdSerialIoUartDebugEnable);
  SerialIoUartNumber = PcdGet8 (PcdSerialIoUartNumber);
  SerialIoUartInputClock = PcdGet32 (PcdSerialIoUartInputClock);
  Uart0PinMuxing = PcdGet8 (PcdSerialIoUart0PinMuxing);
#endif

  //
  // Initialize SerialIo UART for debug message
  //
  if (SerialIoUartDebugEnable == 1) {
    ConfigureSerialIoController (SerialIoUartNumber + PchSerialIoIndexUart0, PchSerialIoPci, TRUE);
    if (SerialIoUartNumber == 0) {
      GpioEnableSerialIoUart (SerialIoUartNumber, FALSE, Uart0PinMuxing);
    } else {
      GpioEnableSerialIoUart (SerialIoUartNumber, FALSE, 0);
    }
    PchSerialIoUartInit (SerialIoUartNumber, TRUE, (SerialIoUartInputClock / 16), 3, FALSE);
    PchSerialIoUartOut (SerialIoUartNumber, (UINT8 *)CarInitBuffer, AsciiStrLen(CarInitBuffer));
  }
}
/**
  This function do the PCH cycle decoding initialization.
**/
VOID
EFIAPI
EarlyCycleDecoding (
  VOID
  )
{
  UINT64   P2sbBase;
  UINT64   SmbusBase;

  //
  // Enable PCR base address in PCH
  //
  P2sbBase = PCI_SEGMENT_LIB_ADDRESS (
               DEFAULT_PCI_SEGMENT_NUMBER_PCH,
               DEFAULT_PCI_BUS_NUMBER_PCH,
               PCI_DEVICE_NUMBER_PCH_P2SB,
               PCI_FUNCTION_NUMBER_PCH_P2SB,
               0
               );
  PciSegmentWrite32 (P2sbBase + R_P2SB_CFG_SBREG_BAR, PCH_PCR_BASE_ADDRESS);
  //
  // Enable P2SB MSE
  //
  PciSegmentOr8 (P2sbBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);

  //
  // Program PWRM BASE
  //
  PmcSetPwrmBase (PCH_PWRM_BASE_ADDRESS);

  //
  // Program ACPI Base.
  //
  PmcSetAcpiBase (PcdGet16 (PcdAcpiBaseAddress));

  //
  // Program and Enable TCO Base
  //
  PchTcoBaseSet (PcdGet16 (PcdTcoBaseAddress));

  SmbusBase = PCI_SEGMENT_LIB_ADDRESS (
                DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                DEFAULT_PCI_BUS_NUMBER_PCH,
                PCI_DEVICE_NUMBER_PCH_SMBUS,
                PCI_FUNCTION_NUMBER_PCH_SMBUS,
                0
                );
  //
  // Set SMBus PCI 0x64 = 0x0A0A0000
  //
  PciSegmentWrite32 (SmbusBase + R_SMBUS_CFG_64, 0x0A0A0000);
  //
  // Initialize SMBUS IO BAR
  //
  PciSegmentWrite16 (SmbusBase + R_SMBUS_CFG_BASE, PcdGet16 (PcdSmbusBaseAddress));
  //
  // Enable the Smbus I/O Enable
  //
  PciSegmentOr8 (SmbusBase + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_IO_SPACE);
  //
  // Enable the SMBUS Controller
  //
  PciSegmentOr8 (SmbusBase + R_SMBUS_CFG_HOSTC, B_SMBUS_CFG_HOSTC_HST_EN);

  //
  // Enable the upper 128-byte bank of RTC RAM.
  // PCR [RTC] + 0x3400 [2] = 1
  //
  PchPcrAndThenOr32 (PID_RTC_HOST, R_RTC_PCR_CONF, (UINT32) ~0, B_RTC_PCR_CONF_UCMOS_EN);
}
