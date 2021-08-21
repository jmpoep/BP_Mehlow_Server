/** @file
 Implement the I2C master protocol.

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

#include <Uefi.h>
#include <Base.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciIo.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PchSerialIoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/I2cMaster.h>
#include <Pi/PiI2c.h>
#include <Library/DevicePathLib.h>
#include <Private/Library/I2cMasterCommonLib.h>
#include <Register/PchRegsSerialIo.h>

///
/// "I2Cp"
///
#define I2C_PORT_SIGNATURE      0x70433249

typedef struct {
  UINTN                           Signature;
  EFI_I2C_MASTER_PROTOCOL         MasterApi;
  EFI_EVENT                       RequestEvent;
  EFI_STATUS                      *RequestStatus;
  EFI_EVENT                       Timer;
  I2C_MASTER_CONTEXT              Master;
} I2C_DRIVER_CONTEXT;

///
/// Locate I2C_MASTER_CONTEXT from protocol
///
#define I2C_MASTER_CONTEXT_FROM_MASTER_PROTOCOL(a)  CR (a, I2C_DRIVER_CONTEXT, MasterApi, I2C_PORT_SIGNATURE)

#define I2C_MASTER_POLLING_PERIOD 500 //microseconds

GLOBAL_REMOVE_IF_UNREFERENCED CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[] = {
  { "eng", L"I2C Master X" }, //X - to be substituted with controller number (0-5)
  { NULL , NULL }
};
GLOBAL_REMOVE_IF_UNREFERENCED CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[] = {
  { "eng", L"I2C Master" },
  { NULL , NULL }
};

typedef struct {
  ACPI_EXTENDED_HID_DEVICE_PATH   AcpiExtendedPath;
  CHAR8                           Strings[3];//this field's real size depends on length of HID, CID and UID strings
} ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STRINGS;



/**
  PerformTransfer. For synchronous transfer this function is called in a loop
  and for asynchronous transfers, as a timer callback. It writes data and/or
  read requests to hadrware, copies read data to destination buffers. When
  transfer completes, it cleans up Sw context and Hw registers in preparation
  for new transfer

  @param[in] Event     obligatory parameter for callback functions, not used here
  @param[in] Context   driver context
**/
VOID
EFIAPI
AsyncTransfer (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
{
  I2C_DRIVER_CONTEXT    *DriverContext;

  DriverContext = (I2C_DRIVER_CONTEXT *) Context;
  PerformTransfer (&DriverContext->Master);
  if (!DriverContext->Master.TransferInProgress) {
    gBS->SetTimer (DriverContext->Timer, TimerCancel, 0);
    //set return values
    if (DriverContext->RequestStatus != NULL) {
      *(DriverContext->RequestStatus) = DriverContext->Master.TransferStatus;
    }
    if (DriverContext->RequestEvent != NULL) {
      gBS->SignalEvent (DriverContext->RequestEvent);
    }
  }
}

/**
  Set the I2C controller bus clock frequency.

  This routine must be called at or below TPL_NOTIFY.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the controller will use lower frequency for the I2C to avoid exceeding
  the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval EFI_SUCCESS       The bus frequency was set successfully.
  @retval EFI_UNSUPPORTED   The controller does not support this frequency.
**/

EFI_STATUS
EFIAPI
I2cPortBusFrequencySet (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN OUT UINTN                     *BusClockHertz
  )
{
  I2C_DRIVER_CONTEXT *Context;

  Context = I2C_MASTER_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  return FrequencySet (Context->Master.MmioAddress, BusClockHertz);
}

/**
  Get PCI Device ID from device pointed to by handle.
  If valid DeviceID can't be retrieved, 0xFFFF is returned

  @param[in] Handle   device handle

  @retval             PCI Device ID
**/
UINT16
GetPciDevId (
  EFI_HANDLE Handle
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINT32              VenDevId;

  Status = gBS->OpenProtocol (
                  Handle,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  Handle,
                  Handle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return 0xFFFF;
  }
  Status = PciIo->Pci.Read (PciIo, EfiPciIoWidthUint32, 0, 1, &VenDevId);
  gBS->CloseProtocol ( Handle,
         &gEfiPciIoProtocolGuid,
         Handle,
         Handle );
  if (EFI_ERROR (Status)) {
    return 0xFFFF;
  }
  return (VenDevId >> 16);
}

/**
  Get Acpi HID from device pointed to by handle.
  If valid Acpi HID can't be retrieved, null string is returned

  @param[in] Handle   device handle

  @retval             HID string
**/
CHAR8*
GetAcpiHid (
  EFI_HANDLE Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL *EndPath;

  DevicePath = DevicePathFromHandle ( Handle );
  if ( NULL == DevicePath ) {
    return "";
  }
  EndPath = DevicePath;
  while ( !IsDevicePathEnd ( EndPath )) {
    DevicePath = EndPath;
    EndPath = NextDevicePathNode ( EndPath );
  }
  if ((DevicePath->Type == ACPI_DEVICE_PATH) && (DevicePath->SubType == ACPI_EXTENDED_DP)) {
    return ((ACPI_EXTENDED_HID_DEVICE_PATH_WITH_STRINGS*) DevicePath)->Strings;
  }
  return "";
}

/**
  Reset the I2C controller

  @param[in]     This       Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
**/
EFI_STATUS
EFIAPI
I2cPortReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This
  )
{
  I2C_DRIVER_CONTEXT *Context;

  Context = I2C_MASTER_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  return I2cReset (Context->Master.MmioAddress);
}


/**
  Start an I2C operation on the controller

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  The typical consumer of this API is the I2C host driver.
  Extreme care must be taken by other consumers of this API to
  prevent confusing the third party I2C drivers due to a state
  change at the I2C device which the third party I2C drivers did
  not initiate.  I2C platform drivers may use this API within
  these guidelines.

  This function initiates an I2C operation on the controller.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all data to the I2C device. If Event parameter is NULL,
  it means a synchronous transfer. In that case, function will return only after the
  transfer has finished. Otherwise, it's an asynchronous transfer. In that case
  function will start the transfer, set a timer event and return. Timer event will
  wake it up later so that it can continue or finish the transfer.

  @param[in] This               Address of an EFI_I2C_MASTER_PROTOCOL
                                structure
  @param[in] SlaveAddress       Address of the device on the I2C bus.
  @param[in] Event              Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in] RequestPacket      Address of an EFI_I2C_REQUEST_PACKET
                                structure describing the I2C operation
  @param[out] RequestStatus     Optional buffer to receive the I2C operation
                                completion status

  @retval EFI_SUCCESS           The requested synchronous operation completed successfully or
                                requested asynchronous operation was started.
  @retval EFI_ALREADY_STARTED   The controller is busy with another transfer
  @retval EFI_BAD_BUFFER_SIZE   Transfer size too big
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL, invalid Operation flags
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave address.
  @retval EFI_UNSUPPORTED       10bit i2c address or "ping" operation attempted (0-byte transfer, address byte not followed by any data)

**/
EFI_STATUS
EFIAPI
I2cPortMasterStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL  *This,
  IN UINTN                          SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET         *RequestPacket,
  IN EFI_EVENT                      Event          OPTIONAL,
  OUT EFI_STATUS                    *RequestStatus OPTIONAL
  )
{
  I2C_DRIVER_CONTEXT  *Context;
  EFI_STATUS          Status;

  Context = I2C_MASTER_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  Status = ValidateRequest (&Context->Master, RequestPacket);
  if (EFI_ERROR (Status)) {
    DEBUG (( DEBUG_INFO, "I2cPort: ValidateRequest failed, %r\n", Status ));
    return Status;
  }

  Context->RequestStatus = RequestStatus;
  Context->RequestEvent = Event;

  Status = InitializeTransfer (&Context->Master, SlaveAddress, RequestPacket);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AsyncTransfer (Context->Timer, Context);
  if (Event == NULL) {
    while (Context->Master.TransferInProgress) {
      MicroSecondDelay (I2C_MASTER_POLLING_PERIOD);
      AsyncTransfer (Context->Timer, Context);
    }
    return Context->Master.TransferStatus;
  } else {
    gBS->SetTimer (Context->Timer, TimerPeriodic, 10*I2C_MASTER_POLLING_PERIOD);
    return EFI_SUCCESS;
  }

}



/**
  Verify the controller type

  This routine determines if an I2C controller is available.

  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to test.
  @param [in] RemainingDevicePath  Not used.

  @retval EFI_SUCCESS              This driver supports this device.
  @retval other                    This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN EFI_HANDLE                    Controller,
  IN EFI_DEVICE_PATH_PROTOCOL      *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  PCH_SERIAL_IO_CONTROLLER    Number;

  Status = gBS->OpenProtocol (Controller, &gEfiI2cMasterProtocolGuid, NULL, NULL, NULL, EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
  if ( !EFI_ERROR ( Status )) {
    Status = EFI_ALREADY_STARTED;
    return Status;
  }
  if (IsSerialIoPciDevId (GetPciDevId (Controller), &Number) || IsSerialIoAcpiHid (GetAcpiHid (Controller), &Number)) {
    if (Number <= GetMaxI2cNumber ()) {
      return EFI_SUCCESS;
    }
  }
  return EFI_UNSUPPORTED;
}

/**
  Connect to the I2C controller

  This routine initializes an instance of the I2C driver for this
  controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in I2cPortDriverSupported.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS              This driver is added to Controller.
  @retval other                    This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE                  Controller,
  IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
  )
{
  I2C_DRIVER_CONTEXT       *Context;
  EFI_STATUS               Status;
  PCH_SERIAL_IO_CONTROLLER ControllerNumber;

  if (!IsSerialIoPciDevId (GetPciDevId (Controller), &ControllerNumber) || IsSerialIoAcpiHid (GetAcpiHid (Controller), &ControllerNumber)) {
    return EFI_UNSUPPORTED;
  }
  if (ControllerNumber>GetMaxI2cNumber ()) {
    return EFI_UNSUPPORTED;
  }

  Context = AllocateZeroPool ( sizeof (I2C_DRIVER_CONTEXT));
  if ( NULL == Context ) {
    return EFI_OUT_OF_RESOURCES;
  }

  DEBUG ((DEBUG_INFO, "I2cPortDriverStart on i2c%d\n", ControllerNumber));

  Context->Master.ConfigAddress      = FindSerialIoBar (ControllerNumber,1);
  Context->Master.MmioAddress        = FindSerialIoBar (ControllerNumber,0);
  Context->Master.TransferInProgress = FALSE;

  Status = PrepareController (&Context->Master);
  if ( EFI_ERROR ( Status )) {
    FreePool ( Context );
    return Status;
  }

  Context->Signature = I2C_PORT_SIGNATURE;

  Context->MasterApi.SetBusFrequency                = I2cPortBusFrequencySet;
  Context->MasterApi.Reset                          = I2cPortReset;
  Context->MasterApi.StartRequest                   = I2cPortMasterStartRequest;
  Context->MasterApi.I2cControllerCapabilities      = &(Context->Master.Capabilities);
  Context->Master.Capabilities.StructureSizeInBytes = sizeof (EFI_I2C_CONTROLLER_CAPABILITIES);
  Context->Master.Capabilities.MaximumReceiveBytes  = (UINT32) -1;
  Context->Master.Capabilities.MaximumTransmitBytes = (UINT32) -1;
  Context->Master.Capabilities.MaximumTotalBytes    = (UINT32) -1;
  Status = gBS->CreateEvent (EVT_TIMER | EVT_NOTIFY_SIGNAL, TPL_NOTIFY, AsyncTransfer, (VOID*) Context, &Context->Timer);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiI2cMasterProtocolGuid,
                  &Context->MasterApi,
                  NULL
                  );
  if ( EFI_ERROR ( Status )) {
    FreePool ( Context );
    return Status;
  }

  return EFI_SUCCESS;
}


/**
  Disconnect from the I2C port controller.

  This routine disconnects from the I2C controller.

  This routine is called by DriverUnload when the I2C port driver
  is being unloaded.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to stop driver on.
  @param [in] NumberOfChildren     How many children need to be stopped.
  @param [in] ChildHandleBuffer    Not used.

  @retval EFI_SUCCESS              This driver is removed Controller.
  @retval EFI_DEVICE_ERROR         The device could not be stopped due to a device error.
  @retval other                    This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  I2C_DRIVER_CONTEXT      *I2cPort;
  EFI_I2C_MASTER_PROTOCOL *I2cMasterProtocol;
  EFI_STATUS              Status;

  //
  //  Disconnect any connected drivers and locate the context
  //  structure
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cMasterProtocolGuid,
                  (VOID**)&I2cMasterProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE
                  );
  if ( !EFI_ERROR ( Status )) {
    I2cPort = I2C_MASTER_CONTEXT_FROM_MASTER_PROTOCOL ( I2cMasterProtocol );

    //
    //  Done with the i2C port protocol
    //
    Status = gBS->CloseProtocol ( Controller,
                    &gEfiI2cMasterProtocolGuid,
                    DriverBinding->DriverBindingHandle,
                    Controller
                    );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Remove the I2C port protocol
      //
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      Controller,
                      &gEfiI2cMasterProtocolGuid,
                      I2cMasterProtocol,
                      NULL
                      );
      if ( !EFI_ERROR ( Status )) {
        MmioOr32 (I2cPort->Master.ConfigAddress + R_SERIAL_IO_CFG_PME_CTRL_STS, B_SERIAL_IO_CFG_PME_CTRL_STS_PWR_ST );
        FreePool ( I2cPort );
      } else {
        DEBUG (( DEBUG_ERROR, "ERROR - Failed to uninstall I2C master protocol, Status: %r\n", Status ));
      }
    }
    else {
      DEBUG (( DEBUG_ERROR, "ERROR - Failed to close I2C master protocol, Status: %r\n", Status ));
    }
  }

  DEBUG (( DEBUG_INFO, "I2cPortDriverStop exiting, Status: %r\n", Status ));

  //
  //  Return the stop status
  //
  return Status;
}


/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL gI2cPortDriverBinding = {
  I2cPortDriverSupported,
  I2cPortDriverStart,
  I2cPortDriverStop,
  0x10,
  NULL,
  NULL
};

/**
  <b>I2C Master DXE Module Entry Point</b>\n
  - <b>Introduction</b>\n
    This is DXE driver binding driver of I2C master protocol to comply the I2C driver stack of PI spec 1.3.

  - @result
    The I2C master DXE driver installs a driver binding protocol and produces EFI_I2C_MASTER_PROTOCOL.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
**/
EFI_STATUS
EFIAPI
I2cMasterEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  EFI_STATUS Status;

  //
  // Install UEFI Driver Model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gI2cPortDriverBinding,
             ImageHandle,
             NULL,
             NULL
             );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

