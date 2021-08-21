/** @file
 Implement the I2C port control.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation.

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
#include <PiPei.h>
#include <Uefi.h>
#include <Base.h>
#include <IndustryStandard/Pci.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/PchSerialIoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Ppi/I2cMaster.h>
#include <Pi/PiI2c.h>
#include <Library/PeiServicesLib.h>
#include <Library/TimerLib.h>
#include <Library/PchInfoLib.h>
#include <Private/Library/I2cMasterCommonLib.h>

/**
  Checks if 'This' points to valid I2c controller
  If yes, returns its controller number

  @param[in]     This       Address of an EFI_I2C_MASTER_PPI structure
  @param[out]    Number     if pointer is valid, returns number of i2c controller

  @retval TRUE              'This' points to valid controler; controller number is returned through Number parameter
  @retval FALSE             'This' does not point to valid controler; Number perameter not updated
**/
BOOLEAN
IsI2cController (
  IN CONST EFI_PEI_I2C_MASTER_PPI *This,
  OUT PCH_SERIAL_IO_CONTROLLER    *Number
  )
{
  if (CompareGuid (&(This->Identifier), &gI2c0MasterGuid)) {
    *Number = PchSerialIoIndexI2C0;
    return TRUE;
  }
  if (CompareGuid (&(This->Identifier), &gI2c1MasterGuid)) {
    *Number = PchSerialIoIndexI2C1;
    return TRUE;
  }
  if (CompareGuid (&(This->Identifier), &gI2c2MasterGuid)) {
    *Number = PchSerialIoIndexI2C2;
    return TRUE;
  }
  if (CompareGuid (&(This->Identifier), &gI2c3MasterGuid)) {
    *Number = PchSerialIoIndexI2C3;
    return TRUE;
  }
  if (IsPchLp ()) {
    if (CompareGuid (&(This->Identifier), &gI2c4MasterGuid)) {
      *Number = PchSerialIoIndexI2C4;
      return TRUE;
    }
    if (CompareGuid (&(This->Identifier), &gI2c5MasterGuid)) {
      *Number = PchSerialIoIndexI2C5;
      return TRUE;
    }
  }
  return FALSE;
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

  @param[in] This               Address of an EFI_PEI_I2C_MASTER_PPI
                                structure
  @param[in] BusClockHertz      New I2C bus clock frequency in Hertz

  @retval EFI_SUCCESS           The bus frequency was set successfully.
  @retval EFI_UNSUPPORTED       The controller does not support this frequency.
  @retval EFI_INVALID_PARAMETER 'This' pointer doesn't point to valid i2c master device
**/
EFI_STATUS
EFIAPI
I2cPortSetBusFrequency (
  IN EFI_PEI_I2C_MASTER_PPI *This,
  IN OUT UINTN              *BusClockHertz
  )
{
  UINTN                    MmioAddress;
  PCH_SERIAL_IO_CONTROLLER Controller;

  DEBUG (( DEBUG_INFO, "I2cSetBusFrequencyPei\n" ));
  if (!IsI2cController (This, &Controller)) {
    return EFI_INVALID_PARAMETER;
  }
  if (Controller > GetMaxI2cNumber ()) {
    return EFI_INVALID_PARAMETER;
  }
  MmioAddress = FindSerialIoBar (Controller, 0);

  return FrequencySet (MmioAddress, BusClockHertz);
}

/**
  Reset the I2C controller

  @param[in]     This           Address of an EFI_I2C_MASTER_PPI
                                structure
  @retval EFI_SUCCESS           Reset completed.
  @retval EFI_INVALID_PARAMETER 'This' pointer doesn't point to valid i2c master device
**/
EFI_STATUS
EFIAPI
I2cPortReset (
  IN CONST EFI_PEI_I2C_MASTER_PPI *This
  )
{
  UINTN              MmioAddress;
  PCH_SERIAL_IO_CONTROLLER Controller;

  if (!IsI2cController (This, &Controller)) {
    return EFI_INVALID_PARAMETER;
  }
  if (Controller > GetMaxI2cNumber ()) {
    return EFI_INVALID_PARAMETER;
  }
  MmioAddress = FindSerialIoBar (Controller, 0);

  return I2cReset (MmioAddress);
}


/**
  Start an I2C operation on the controller

  The operation is performed by selecting the I2C device with its slave
  address and then sending all data to the I2C device. Unlike driver for DXE phase, this
  one only supports synchronous transfers.

  @param[in] This           Address of an EFI_I2C_MASTER_PPI
                            structure
  @param[in] SlaveAddress   Address of the device on the I2C bus.
  @param[in] RequestPacket  Address of an EFI_I2C_REQUEST_PACKET
                            structure describing the I2C operation

  @retval EFI_SUCCESS           The requested synchronous operation completed successfully or
                                requested asynchronous operation was started.
  @retval EFI_ALREADY_STARTED   The controller is busy with another transfer
  @retval EFI_BAD_BUFFER_SIZE   Transfer size too big
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK during data transfer)
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL, invalid Operation flags
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the slave address (NACK after address byte).
  @retval EFI_UNSUPPORTED       "ping" operation attempted (0-byte transfer, address byte not followed by any data)
**/
EFI_STATUS
EFIAPI
I2cPortStartRequest (
  IN CONST EFI_PEI_I2C_MASTER_PPI *This,
  IN UINTN SlaveAddress,
  IN EFI_I2C_REQUEST_PACKET *RequestPacket
  )
{
  EFI_STATUS               Status;
  PCH_SERIAL_IO_CONTROLLER Controller;
  I2C_MASTER_CONTEXT       Context;

  if (!IsI2cController (This, &Controller)) {
    return EFI_INVALID_PARAMETER;
  }
  if (Controller > GetMaxI2cNumber ()) {
    return EFI_NOT_FOUND;
  }

  Context.MmioAddress = FindSerialIoBar (Controller, 0);
  Context.ConfigAddress = FindSerialIoBar (Controller, 1);
  Context.Capabilities.MaximumReceiveBytes = 0xFFFFFFFF;
  Context.Capabilities.MaximumTotalBytes = 0xFFFFFFFF;
  Context.Capabilities.MaximumTransmitBytes = 0xFFFFFFFF;
  Context.TransferInProgress = FALSE;

  Status = ValidateRequest (&Context, RequestPacket);
  if (EFI_ERROR (Status)) {
    DEBUG (( DEBUG_INFO, "I2cStartRequest: ValidateRequest failed, %r\n", Status ));
    return Status;
  }

  Status = InitializeTransfer (&Context, SlaveAddress, RequestPacket);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  while (Context.TransferInProgress) {
    PerformTransfer (&Context);
  }
  return Context.TransferStatus;
}

/**
  Retrieve the GUID of SerialIo controller

  @param[in]             Index of SerialIo controller

  @retval                GUID of the SerialIo controller
**/
CONST EFI_GUID*
GetInstanceGuid (
  IN PCH_SERIAL_IO_CONTROLLER Controller
  )
{
  switch (Controller) {
    case PchSerialIoIndexI2C0:
      return &gI2c0MasterGuid;
    case PchSerialIoIndexI2C1:
      return &gI2c1MasterGuid;
    case PchSerialIoIndexI2C2:
      return &gI2c2MasterGuid;
    case PchSerialIoIndexI2C3:
      return &gI2c3MasterGuid;
    case PchSerialIoIndexI2C4:
      return &gI2c4MasterGuid;
    case PchSerialIoIndexI2C5:
      return &gI2c5MasterGuid;
    default:
      return NULL;
  }
}

/*
  Allocates memory for I2cMaster PPI descriptor

  @param[in] Controller   number of I2c controller

  @retval                 Pointer to memory allocated and filled with PPI descriptor data
*/
EFI_PEI_PPI_DESCRIPTOR*
PrepareDescriptor (
  IN PCH_SERIAL_IO_CONTROLLER Controller
  )
{
  EFI_PEI_PPI_DESCRIPTOR          *I2cMasterDescriptor;
  EFI_PEI_I2C_MASTER_PPI          *I2cMasterPpi;
  EFI_I2C_CONTROLLER_CAPABILITIES *I2cControllerCapabilities;
  CONST EFI_GUID                  *InstanceGuid;

  InstanceGuid = GetInstanceGuid (Controller);
  if(InstanceGuid == NULL) {
    return NULL;
  }

  I2cMasterDescriptor = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  I2cMasterPpi = (EFI_PEI_I2C_MASTER_PPI *) AllocateZeroPool (sizeof (EFI_PEI_I2C_MASTER_PPI));
  I2cControllerCapabilities = (EFI_I2C_CONTROLLER_CAPABILITIES *) AllocateZeroPool (sizeof (EFI_I2C_CONTROLLER_CAPABILITIES));
  if ((I2cMasterDescriptor == NULL) ||
      (I2cMasterPpi == NULL) ||
      (I2cControllerCapabilities == NULL)) {
    DEBUG ((DEBUG_ERROR, "PrepareDescriptor allocate zero pool error\n"));
    return NULL;
  }
  I2cControllerCapabilities->StructureSizeInBytes = sizeof (EFI_I2C_CONTROLLER_CAPABILITIES);
  I2cControllerCapabilities->MaximumTotalBytes = (UINT32) -1;
  I2cControllerCapabilities->MaximumTransmitBytes = (UINT32) -1;
  I2cControllerCapabilities->MaximumReceiveBytes = (UINT32) -1;
  I2cMasterPpi->SetBusFrequency = I2cPortSetBusFrequency;
  I2cMasterPpi->Reset = I2cPortReset;
  I2cMasterPpi->StartRequest = I2cPortStartRequest;
  I2cMasterPpi->I2cControllerCapabilities = I2cControllerCapabilities;
  CopyGuid (&(I2cMasterPpi->Identifier), InstanceGuid);
  I2cMasterDescriptor->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  I2cMasterDescriptor->Guid  = &gEfiPeiI2cMasterPpiGuid;
  I2cMasterDescriptor->Ppi   = I2cMasterPpi;

  return I2cMasterDescriptor;
}

/*
  Wrapper for LocatePpi for I2cMaster
  If there are multiple I2c master devices in system, each has a copy of
  I2cMasterPpi instaled on it. Code that wants to use particular MasterPpi needs to
  loop over all Ppi instances and check which one contains the correct value in its
  Identifier field. This function abstracts the locate & compare loop

  @param[in] Controller - number of I2c controller

  @retval NULL when requested Ppi instance can't be found
  @retval Pointer to the correct instance of I2cMasterPpi
*/
EFI_PEI_I2C_MASTER_PPI*
LocateI2cMasterPpi (
  PCH_SERIAL_IO_CONTROLLER Controller
  )
{
  UINTN                  Instance;
  EFI_STATUS             Status;
  EFI_PEI_I2C_MASTER_PPI *MasterPpi;
  CONST EFI_GUID         *InstanceGuid;

  Instance = 0;
  Status = EFI_SUCCESS;
  InstanceGuid = GetInstanceGuid (Controller);
  if(InstanceGuid == NULL) {
    return NULL;
  }

  while (TRUE) {
    Status = PeiServicesLocatePpi (&gEfiPeiI2cMasterPpiGuid, Instance, NULL, (VOID **)&MasterPpi);
    if (EFI_ERROR (Status)) {
      return NULL;
    }
    if (CompareGuid (&(MasterPpi->Identifier), InstanceGuid)) {
      return MasterPpi;
    }
    Instance++;
  }
}

/*
  Installs I2cMaster PPIs for each I2c controller.

  @param[in] Controller - number of I2c controller

*/
VOID
InstallI2cMasterPpi (
  PCH_SERIAL_IO_CONTROLLER Controller
  )
{
  EFI_PEI_I2C_MASTER_PPI *I2cMasterPpi;
  EFI_STATUS             Status;

  I2cMasterPpi = NULL;

  if (Controller > GetMaxI2cNumber ()) {
    DEBUG ((DEBUG_ERROR, "InstallI2cMasterPpi: Unsupported controller number %d\n", Controller));
    ASSERT (FALSE);
    return;
  }

  I2cMasterPpi = LocateI2cMasterPpi (Controller);
  if (I2cMasterPpi != NULL) {
    DEBUG ((DEBUG_INFO, "InstallI2cMasterPpi: The I2cMaster PPI 0x%02x is already installed\n", Controller));
    return;
  }

  DEBUG ((DEBUG_INFO, "InstallI2cMasterPpi End\n"));
  Status = PeiServicesInstallPpi (PrepareDescriptor (Controller));
  ASSERT_EFI_ERROR (Status);
}

