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
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Ppi/SiPolicy.h>
#include <Private/Library/PeiItssLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Library/PchSerialIoLib.h>
#include <Library/CnviLib.h>
#include <Library/GpioLib.h>
#include <Library/GpioNativeLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchPcrLib.h>
#include <Private/Library/SiScheduleResetLib.h>
#include "PchResetPlatformSpecific.h"

/**
  Updates Chip Select configuration in SPI MMIO registers

  @param[in] Controller
  @param[in] SpiCsPolarity

  @retval None
**/
VOID
SerialIoSpiConfigureCsPolarity (
  IN PCH_SERIAL_IO_CONTROLLER Controller,
  IN UINT32                   SpiCsPolarity
  )
{
  UINTN    Base;

  Base = FindSerialIoBar(Controller, 0);
  //
  // set Invert Frame Signal before enabling pins to ensure correct initial ChipSelect polarity
  //
  if (SpiCsPolarity == PchSerialIoCsActiveLow) {
    MmioAnd32 (Base + R_SERIAL_IO_MEM_SSCR1, (UINT32) ~(B_SERIAL_IO_MEM_SSCR1_IFS));
    MmioOr32 (Base + R_SERIAL_IO_MEM_SPI_CS_CONTROL, B_SERIAL_IO_MEM_SPI_CS_CONTROL_STATE);
  } else {
    MmioOr32 (Base + R_SERIAL_IO_MEM_SSCR1, B_SERIAL_IO_MEM_SSCR1_IFS);
    MmioAnd32 (Base + R_SERIAL_IO_MEM_SPI_CS_CONTROL, (UINT32) ~B_SERIAL_IO_MEM_SPI_CS_CONTROL_STATE);
  }
  MmioOr32 (Base + R_SERIAL_IO_MEM_SPI_CS_CONTROL, B_SERIAL_IO_MEM_SPI_CS_CONTROL_MODE);
}

/**
  Enable I2C native pins and configure their termination

  @param[in] UINT32 Controller
  @param[in] UINT8  I2cPadsTermination
**/
STATIC
VOID
SerialIoI2cEnable (
  IN UINT32 SerialIoI2cControllerNumber,
  IN UINT8  I2cPadsTermination
  )
{
  switch (I2cPadsTermination) {
    case GpioTermDefault:
    case GpioTermNone:
    case GpioTermWpu1K:
    case GpioTermWpu5K:
    case GpioTermWpu20K:
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Gpio Pad Termination must be set as Default, None, 1k, 5k or 20k WPU\n"));
      ASSERT (FALSE);
  }
  GpioEnableSerialIoI2c (SerialIoI2cControllerNumber, I2cPadsTermination);
}
/*
  Configures Serial IO Controllers gpio pads based on the current policy settings
  Configuration is skipped if controller is Disabled or SkipInit set in policy

  @param[in] PCH_SERIAL_IO_CONFIG SerialIoConfig
  @param[in] BOOLEAN              IsCnviPresent

  @retval None
*/
VOID
SerialIoGpioConfiguration (
  IN PCH_SERIAL_IO_CONFIG *SerialIoConfig,
  IN BOOLEAN              IsCnviPresent
  )
{
  PCH_SERIAL_IO_CONTROLLER    Controller;

  for (Controller = 0; Controller < PchSerialIoIndexMax; Controller++) {
    // TODO: Add verification if GPIO is already set
    if (SerialIoConfig->DevMode[Controller] != PchSerialIoDisabled) {
      switch (GetSerialIoControllerType (Controller)) {
        case SERIAL_IO_I2C:
          SerialIoI2cEnable (Controller, SerialIoConfig->I2cPadsTermination[Controller]);
          break;
        case SERIAL_IO_SPI:
          SerialIoSpiConfigureCsPolarity (Controller, SerialIoConfig->SpiCsPolarity[Controller-PchSerialIoIndexSpi0]);
          GpioEnableSerialIoSpi (Controller-PchSerialIoIndexSpi0);
          break;
        case SERIAL_IO_UART:
          //
          //  RTS and CTS enabled if Hardware Flow Control is used
          //
          if (Controller == PchSerialIoIndexUart0) {
            //
            // UART0 cannot be routed to physical pins if CNVi is present due to
            //   1. The pin set which is shared with CNVi RF interface is muxed out to RF signals by hardware default.
            //
            if (!(IsCnviPresent && SerialIoConfig->Uart0PinMuxing)) {
              GpioEnableSerialIoUart (Controller-PchSerialIoIndexUart0, SerialIoConfig->UartHwFlowCtrl[Controller-PchSerialIoIndexUart0], SerialIoConfig->Uart0PinMuxing);
            } else if (SerialIoConfig->Uart0PinMuxing) {
              DEBUG ((DEBUG_ERROR, "The 2nd pin set contains pads which are also used for CNVi purpose, setting Uart0PinMuxing exclude from CNVi is present.\n"));
              ASSERT (FALSE);
            }
          } else {
            GpioEnableSerialIoUart (Controller-PchSerialIoIndexUart0, SerialIoConfig->UartHwFlowCtrl[Controller-PchSerialIoIndexUart0], 0);
          }
          break;
        case SERIAL_IO_UNKNOWN:
        default:
          DEBUG ((DEBUG_ERROR, "ConfigureSerialIoPei Unknown Controller\n"));
          break;
      }
    }
  }
}

/**
  Configures SerilIo devices interrupt pin and IRQ assignment

  @param[in] SiPolicy        Policy
  @param[in] Controller      Serial IO Controller
**/
VOID
SerialIoIntSet (
  IN SI_POLICY_PPI               *SiPolicy,
  IN PCH_SERIAL_IO_CONTROLLER    Controller
  )
{
  UINT16    Offset;
  UINT32    Data32Or;
  UINT32    Data32And;
  UINT8     DeviceNumber;
  UINT8     FunctionNumber;
  UINT8     InterruptPin;
  UINT8     Irq;

  Offset = 0x0;

  DeviceNumber = GetSerialIoDeviceNumber (Controller);
  FunctionNumber = GetSerialIoFunctionNumber (Controller);

  ///
  /// Get SerialIo controller interrupt configuration
  ///
  ItssGetDevIntConfig (
    SiPolicy,
    DeviceNumber,
    FunctionNumber,
    &InterruptPin,
    &Irq
    );

  //
  // Match SerialIo Dxx:Fx with appropriate PCICFGCTRLx register
  //
  Offset = GetSerialIoConfigControlOffset (DeviceNumber, FunctionNumber);

  ASSERT (Offset != 0);

  //
  // Set Interrupt Pin and Irq number
  //
  Data32Or  = (UINT32) ((InterruptPin << N_SERIAL_IO_PCR_PCICFGCTRL_INT_PIN) |
                        (Irq << N_SERIAL_IO_PCR_PCICFGCTRL_PCI_IRQ));
  Data32And =~(UINT32) (B_SERIAL_IO_PCR_PCICFGCTRL_PCI_IRQ | B_SERIAL_IO_PCR_PCICFGCTRL_ACPI_IRQ | B_SERIAL_IO_PCR_PCICFGCTRL_INT_PIN);

  PchPcrAndThenOr32 (PID_SERIALIO, Offset, Data32And, Data32Or);
}

/**
  Configures all func0 Serial IO Controllers

  @param[in] PCH_SERIAL_IO_CONFIG
**/
VOID
ConfigureSerialIoFunc0Controllers (
  IN PCH_SERIAL_IO_CONFIG        *SerialIoConfig,
  IN SI_POLICY_PPI               *SiPolicy
  )
{
  PCH_SERIAL_IO_CONTROLLER    Controller;
  UINT8                       FuncNum;
  BOOLEAN                     PsfDisable;
  UINT8                       DevNumOfFunc0;

  for (Controller = 0; Controller < PchSerialIoIndexMax; Controller++) {
    if (IsSerialIoFunctionZero (Controller)) {
      PsfDisable = TRUE;
      DevNumOfFunc0 = GetSerialIoDeviceNumber (Controller);
      //
      // Check all other func devs(1 to 7) status except func 0.
      //
      for (FuncNum = 1; FuncNum <= PCI_MAX_FUNC; FuncNum++) {
        if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, DevNumOfFunc0, FuncNum, PCI_DEVICE_ID_OFFSET)) != 0xFFFF) {
          PsfDisable = FALSE;
        }
      }
      ConfigureSerialIoController (Controller, SerialIoConfig->DevMode[Controller], PsfDisable);
      if ((SerialIoConfig->DevMode[Controller] != PchSerialIoDisabled) || (!PsfDisable)) {
        SerialIoIntSet (SiPolicy, Controller);
      }
    }
  }
}

/**
  Initializes Serial IO Controllers

  @param[in] SiPolicy

  @retval None
**/
VOID
SerialIoInit (
  IN SI_POLICY_PPI           *SiPolicy
  )
{
  PCH_SERIAL_IO_CONTROLLER    Controller;
  EFI_STATUS                  Status;
  PCH_SERIAL_IO_CONFIG        *SerialIoConfig;
  PCH_CNVI_CONFIG             *CnviConfig;
  PCH_RESET_DATA              ResetData;
  BOOLEAN                     IsSerialIoDisabled;
  BOOLEAN                     DisableSerialIo;

  DEBUG ((DEBUG_INFO, "SerialIoInit() Start\n"));

  Status = GetConfigBlock ((VOID *) SiPolicy, &gSerialIoConfigGuid, (VOID *) &SerialIoConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gCnviConfigGuid, (VOID *) &CnviConfig);
  ASSERT_EFI_ERROR (Status);

  IsSerialIoDisabled = PmcIsSerialIoStaticallyDisabled ();

  if (!IsSerialIoDisabled) {
    //
    // enable clock gating and power gating
    //
    PchPcrAndThenOr32 (PID_SERIALIO, R_SERIAL_IO_PCR_GPPRVRW2, 0xFFFFFFFF, V_SERIAL_IO_PCR_GPPRVRW2_CLK_GATING);
    PchPcrAndThenOr32 (PID_SERIALIO, R_SERIAL_IO_PCR_PMCTL   , 0xFFFFFFFF, V_SERIAL_IO_PCR_PMCTL_PWR_GATING);

    for (Controller = 0; Controller < PchSerialIoIndexMax; Controller++) {
      if (!IsSerialIoFunctionZero (Controller)) {
        ConfigureSerialIoController (Controller, SerialIoConfig->DevMode[Controller], TRUE);
      if (SerialIoConfig->DevMode[Controller] != PchSerialIoDisabled) {
           SerialIoIntSet (SiPolicy, Controller);
        }
      }
    }
    //
    // If Pch SerialIO Controller is Function 0 do not hide it in PSF, higher functions have to remain enabled and available to PCI enumerator
    // Only hide in PSF if all the other functions of same device are disabled.
    //
    ConfigureSerialIoFunc0Controllers (SerialIoConfig, SiPolicy);
  }

  //
  // Check if all SerialIo controllers should be disabled in PMC
  //
  DisableSerialIo = TRUE;
  for (Controller = 0; Controller < PchSerialIoIndexMax; Controller++) {
    if (SerialIoConfig->DevMode[Controller] != PchSerialIoDisabled) {
      DisableSerialIo = FALSE;
    }
  }

  if (DisableSerialIo && !IsSerialIoDisabled) {
    //
    // If all SertialIo controllers are disabled do static power gating in PMC
    //
    PmcStaticDisableSerialIo ();
  } else if (!DisableSerialIo && IsSerialIoDisabled) {
    //
    // If at least one SertialIo controller is to be used remove static power gating in PMC
    //
    PmcEnableSerialIo ();
  }

  //
  // Trigger reset if SerialIo static power gating state has to be changed
  //
  if (IsSerialIoDisabled != DisableSerialIo) {
    DEBUG ((DEBUG_INFO, "Reset due to changes in SerialIo FunctionDisable\n"));
    CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
    StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
    SiScheduleResetSetType (EfiResetPlatformSpecific, &ResetData);
  }

  SerialIoGpioConfiguration (
    SerialIoConfig,
    CnviIsPresent ()
    );

  DEBUG ((DEBUG_INFO, "SerialIoInit() End\n"));
}
