/** @file
  Performs PCI and Media device inventory for AMT.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2017 Intel Corporation.

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
#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Protocol/DevicePath.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PchInfoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include "BiosExtensionLoader.h"
#include "Inventory.h"
#include <Library/UefiLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED AMT_MEDIA_FRU mAmtMediaFru;
GLOBAL_REMOVE_IF_UNREFERENCED AMT_PCI_FRU   mAmtPciFru;

/**
  Transfer each other while being front and back.

  @param[in] Data                 The address of data
  @param[in] Size                 Size of data

**/
VOID
SwapEntries (
  IN CHAR8                        *Data,
  IN UINT8                        Size
  )
{
  UINT16                          Index;
  CHAR8                           Temp8;

  Index = 0;
  while (Data[Index] != 0 && Data[Index + 1] != 0) {
    Temp8           = Data[Index];
    Data[Index]     = Data[Index + 1];
    Data[Index + 1] = Temp8;
    Index += 2;
    if (Index >= Size) {
      break;
    }
  }

  return;
}

/**
  This function get the next bus range of given address space descriptors.
  It also moves the pointer backward a node, to get prepared to be called
  again.

  @param[in] Descriptors          points to current position of a serial of address space
                                  descriptors
  @param[in] MinBus               The lower range of bus number
  @param[in] MaxBus               The upper range of bus number
  @param[in] IsEnd                Meet end of the serial of descriptors

  @retval EFI_SUCCESS             The command completed successfully
**/
EFI_STATUS
PciGetNextBusRange (
  IN OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR  **Descriptors,
  OUT    UINT16                             *MinBus,
  OUT    UINT16                             *MaxBus,
  OUT    BOOLEAN                            *IsEnd
  )
{
  *IsEnd = FALSE;

  ///
  /// When *Descriptors is NULL, Configuration() is not implemented, so assume
  /// range is 0~PCI_MAX_BUS
  ///
  if ((*Descriptors) == NULL) {
    *MinBus = 0;
    *MaxBus = PCI_MAX_BUS;
    return EFI_SUCCESS;
  }
  ///
  /// *Descriptors points to one or more address space descriptors, which
  /// ends with a end tagged descriptor. Examine each of the descriptors,
  /// if a bus typed one is found and its bus range covers bus, this handle
  /// is the handle we are looking for.
  ///
  if ((*Descriptors)->Desc == ACPI_END_TAG_DESCRIPTOR) {
    *IsEnd = TRUE;
  }

  while ((*Descriptors)->Desc != ACPI_END_TAG_DESCRIPTOR) {
    if ((*Descriptors)->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
      *MinBus = (UINT16) (*Descriptors)->AddrRangeMin;
      *MaxBus = (UINT16) (*Descriptors)->AddrRangeMax;
    }

    (*Descriptors) ++;
  }

  return EFI_SUCCESS;
}

/**
  This function gets the protocol interface from the given handle, and
  obtains its address space descriptors.

  @param[in] Handle               The PCI_ROOT_BRIDIGE_IO_PROTOCOL handle
  @param[in] IoDev                Handle used to access configuration space of PCI device
  @param[in] Descriptors          Points to the address space descriptors

  @retval EFI_SUCCESS             The command completed successfully
**/
EFI_STATUS
PciGetProtocolAndResource (
  IN  EFI_HANDLE                            Handle,
  OUT EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL       **IoDev,
  OUT EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR     **Descriptors
  )
{
  EFI_STATUS  Status;

  ///
  /// Get interface from protocol
  ///
  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  (VOID **) IoDev
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Call Configuration() to get address space descriptors
  ///
  Status = (*IoDev)->Configuration (*IoDev, (VOID **) Descriptors);
  if (Status == EFI_UNSUPPORTED) {
    *Descriptors = NULL;
    return EFI_SUCCESS;
  } else {
    return Status;
  }
}

///
/// This is where we list any chipset PCI device that is NOT a FRU.
/// Platform specific PCI devices are listed elsewhere.
///
UINT64  mPciDeviceFilterOutTable[] = {
  EFI_PCI_ADDRESS (00, 00, 00, 00), ///< MCH
  EFI_PCI_ADDRESS (00, 01, 00, 00), ///< PCIe PEG
  EFI_PCI_ADDRESS (00, 02, 00, 00), ///< IGD
  EFI_PCI_ADDRESS (00, 03, 00, 00), ///< Intel High Definition Audio Controller (inside processor)
  EFI_PCI_ADDRESS (00, 30, 00, 00), ///< DMI to PCI Express Bridge
  EFI_PCI_ADDRESS (00, 31, 00, 00), ///< LPC Controller
  EFI_PCI_ADDRESS (00, 31, 02, 00), ///< Serial ATA Controller #1
  EFI_PCI_ADDRESS (00, 31, 04, 00), ///< SMBus Controller
  EFI_PCI_ADDRESS (00, 31, 05, 00), ///< Serial ATA Controller #2
  EFI_PCI_ADDRESS (00, 31, 06, 00), ///< Thermal Subsystem
  EFI_PCI_ADDRESS (00, 29, 00, 00), ///< USB EHCI Controller #1
  EFI_PCI_ADDRESS (00, 29, 01, 00), ///< USB UHCI Controller #1
  EFI_PCI_ADDRESS (00, 29, 02, 00), ///< USB UHCI Controller #2
  EFI_PCI_ADDRESS (00, 29, 03, 00), ///< USB UHCI Controller #3
  EFI_PCI_ADDRESS (00, 29, 04, 00), ///< USB UHCI Controller #4
  EFI_PCI_ADDRESS (00, 29, 07, 00), ///< USBr #1
  EFI_PCI_ADDRESS (00, 28, 00, 00), ///< PCI Express Port #1
  EFI_PCI_ADDRESS (00, 28, 01, 00), ///< PCI Express Port #2
  EFI_PCI_ADDRESS (00, 28, 02, 00), ///< PCI Express Port #3
  EFI_PCI_ADDRESS (00, 28, 03, 00), ///< PCI Express Port #4
  EFI_PCI_ADDRESS (00, 28, 04, 00), ///< PCI Express Port #5
  EFI_PCI_ADDRESS (00, 28, 05, 00), ///< PCI Express Port #6
  EFI_PCI_ADDRESS (00, 28, 06, 00), ///< PCI Express Port #7
  EFI_PCI_ADDRESS (00, 28, 07, 00), ///< PCI Express Port #8
  EFI_PCI_ADDRESS (00, 27, 00, 00), ///< Intel High Definition Audio Controller (inside chipset)
  EFI_PCI_ADDRESS (00, 26, 00, 00), ///< USB EHCI Controller #2
  EFI_PCI_ADDRESS (00, 26, 01, 00), ///< USB UHCI Controller #5
  EFI_PCI_ADDRESS (00, 26, 02, 00), ///< USB UHCI Controller #6
  EFI_PCI_ADDRESS (00, 26, 03, 00), ///< USB UHCI Controller #7
  EFI_PCI_ADDRESS (00, 26, 07, 00), ///< USBr #2
  EFI_PCI_ADDRESS (00, 25, 00, 00), ///< GbE Controller
  EFI_PCI_ADDRESS (00, 24, 00, 00), ///< Dual Channel NAND Controller
  EFI_PCI_ADDRESS (00, 23, 00, 00), ///< Virtualization Engine
  EFI_PCI_ADDRESS (00, 22, 00, 00), ///< HECI #1
  EFI_PCI_ADDRESS (00, 22, 01, 00), ///< HECI #2
  EFI_PCI_ADDRESS (00, 22, 03, 00), ///< KT
  EFI_PCI_ADDRESS (00, 21, 00, 00), ///< Virtualized SATA Controller
  EFI_PCI_ADDRESS (00, 20, 00, 00), ///< xHCI Controller
  MAX_ADDRESS,
};
#define MAX_FILTER_OUT_DEVICE_NUMBER  0x100

/**
  AMT only need to know removable PCI device information.

  @retval EFI_SUCCESS             mAmtPciFru will be update.
  @retval EFI_OUT_OF_RESOURCES    System on-board device list is larger than
                                  MAX_FILTER_OUT_DEVICE_NUMBER supported.
**/
EFI_STATUS
BuildPciFru (
  VOID
  )
{
  EFI_STATUS                        Status;
  EFI_HANDLE                        *HandleBuffer;
  UINTN                             NumberOfHandles;
  UINTN                             Index;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *IoDev;
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptors;
  UINT16                            MinBus;
  UINT16                            MaxBus;
  BOOLEAN                           IsEnd;
  UINT16                            Bus;
  UINT16                            Device;
  UINT16                            Func;
  UINT64                            Address;
  PCI_COMMON_HEADER                 PciHeader;
  PCI_CONFIG_SPACE                  ConfigSpace;
  BOOLEAN                           ValidDevice;
  UINT8                             CurrentFruDev;
  UINTN                             FilterOutIndex;
  UINT64                            *PciDeviceFilterOutTable;
  UINT64                            *PlatformPciDeviceFilterOutTable;

  CurrentFruDev = 0;
  ValidDevice   = FALSE;

  ///
  /// Get PCI Device Filter Out table about on-board device list from Amt Policy
  /// Only need to report removable device to AMT
  ///
  PlatformPciDeviceFilterOutTable = (UINT64 *) (UINTN) AmtPciDeviceFilterOutTable ();

  ///
  /// System on-board device list
  ///
  if (sizeof (mPciDeviceFilterOutTable) / sizeof (UINT64) > MAX_FILTER_OUT_DEVICE_NUMBER) {
    return EFI_OUT_OF_RESOURCES;
  }

  PciDeviceFilterOutTable = mPciDeviceFilterOutTable;

  ///
  /// Get all instances of PciRootBridgeIo
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiPciRootBridgeIoProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// For each handle, which decides a segment and a bus number range,
  /// enumerate all devices on it.
  ///
  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = PciGetProtocolAndResource (
               HandleBuffer[Index],
               &IoDev,
               &Descriptors
               );
    if (EFI_ERROR (Status)) {
      FreePool (HandleBuffer);
      return Status;
    }
    ///
    /// No document say it's impossible for a RootBridgeIo protocol handle
    /// to have more than one address space descriptors, so find out every
    /// bus range and for each of them do device enumeration.
    ///
    while (TRUE) {
      Status = PciGetNextBusRange (&Descriptors, &MinBus, &MaxBus, &IsEnd);

      if (EFI_ERROR (Status)) {
        FreePool (HandleBuffer);
        return Status;
      }

      if (IsEnd) {
        break;
      }

      for (Bus = MinBus; Bus <= MaxBus; Bus++) {
        ///
        /// For each devices, enumerate all functions it contains
        ///
        for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
          ///
          /// For each function, read its configuration space and print summary
          ///
          for (Func = 0; Func <= PCI_MAX_FUNC; Func++) {
            Address = EFI_PCI_ADDRESS (Bus, Device, Func, 0);
            ///
            /// Read the vendor information
            ///
            IoDev->Pci.Read (
                         IoDev,
                         EfiPciWidthUint16,
                         Address,
                         1,
                         &PciHeader.VendorId
                         );

            ///
            /// If VendorId = 0xFFFF, there does not exist a device at this
            /// location. For each device, if there is any function on it,
            /// there must be 1 function at Function 0. So if Func = 0, there
            /// will be no more functions in the same device, so we can break
            /// loop to deal with the next device.
            ///
            if (PciHeader.VendorId == 0xFFFF && Func == 0) {
              break;
            }

            if (PciHeader.VendorId != 0xFFFF) {
              IoDev->Pci.Read (
                           IoDev,
                           EfiPciWidthUint32,
                           Address,
                           sizeof (PciHeader) / sizeof (UINT32),
                           &PciHeader
                           );

              ValidDevice = TRUE;

              ///
              /// Check if any onboard system devices which need to be filter-out and do not report to AMT
              ///
              if (ValidDevice == TRUE) {
                for (FilterOutIndex = 0; FilterOutIndex < (sizeof (mPciDeviceFilterOutTable) / sizeof (UINT64)); FilterOutIndex++) {
                  if (PciDeviceFilterOutTable[FilterOutIndex] == Address) {
                    ///
                    /// Match found so ignore it.
                    ///
                    ValidDevice = FALSE;
                  }

                  if (PciDeviceFilterOutTable[FilterOutIndex] == Address ||
                      PciDeviceFilterOutTable[FilterOutIndex] == MAX_ADDRESS) {
                    ///
                    /// Match or end of list.
                    ///
                    break;
                  }
                }
              }
              ///
              /// Check if any Platform specific onboard devices which need to be filter-out and do not report to AMT
              ///
              if (ValidDevice == TRUE) {
                if (PlatformPciDeviceFilterOutTable != NULL) {
                  for (FilterOutIndex = 0; FilterOutIndex < MAX_FILTER_OUT_DEVICE_NUMBER; FilterOutIndex++) {
                    if (PlatformPciDeviceFilterOutTable[FilterOutIndex] == Address) {
                      ///
                      /// Match found so ignore it.
                      ///
                      ValidDevice = FALSE;
                    }

                    if (PlatformPciDeviceFilterOutTable[FilterOutIndex] == Address ||
                        PlatformPciDeviceFilterOutTable[FilterOutIndex] == MAX_ADDRESS) {
                      ///
                      /// Match or end of list.
                      ///
                      break;
                    }
                  }
                }
              }
              ///
              /// Filter-out bridge
              ///
              if (ValidDevice == TRUE) {
                switch (PciHeader.ClassCode[2]) {
                  case PCI_CLASS_BRIDGE:
                    ValidDevice = FALSE;
                    break;
                }
              }

              if (ValidDevice == TRUE) {

                if (CurrentFruDev >= PCI_MAX_BUS) {
                  Status = EFI_DEVICE_ERROR;
                  goto Done;
                }
                mAmtPciFru.PciDevInfo[CurrentFruDev].Vid  = PciHeader.VendorId;
                mAmtPciFru.PciDevInfo[CurrentFruDev].Did  = PciHeader.DeviceId;
                mAmtPciFru.PciDevInfo[CurrentFruDev].Rid  = PciHeader.RevisionId;
                mAmtPciFru.PciDevInfo[CurrentFruDev].ClassCode = ((UINT32) PciHeader.ClassCode[0]) | ((UINT32) PciHeader.ClassCode[1] << 0x8) | ((UINT32) PciHeader.ClassCode[2] << 0x10);

                Status = IoDev->Pci.Read (
                                      IoDev,
                                      EfiPciWidthUint8,
                                      Address,
                                      sizeof (ConfigSpace),
                                      &ConfigSpace
                                      );

                mAmtPciFru.PciDevInfo[CurrentFruDev].SubSysVid  = ConfigSpace.NonCommon.Device.SubVendorId;
                mAmtPciFru.PciDevInfo[CurrentFruDev].SubSysDid  = ConfigSpace.NonCommon.Device.SubSystemId;
                mAmtPciFru.PciDevInfo[CurrentFruDev].BusDevFcn  = (UINT16) (Bus << 0x08 | Device << 0x03 | Func);
                CurrentFruDev++;
              }
              ///
              /// If this is not a multi-function device, we can leave the loop
              /// to deal with the next device.
              ///
              if (Func == 0 && ((PciHeader.HeaderType & HEADER_TYPE_MULTI_FUNCTION) == 0x00)) {
                break;
              }
            }
          }
        }
      }
      ///
      /// If Descriptor is NULL, Configuration() returns EFI_UNSUPPRORED,
      /// we assume the bus range is 0~PCI_MAX_BUS. After enumerated all
      /// devices on all bus, we can leave loop.
      ///
      if (Descriptors == NULL) {
        break;
      }
    }
  }

  mAmtPciFru.PciDevicesHeader.VersionInfo = DEVICES_LIST_VERSION;
  mAmtPciFru.PciDevicesHeader.DevCount    = CurrentFruDev;

Done:
  FreePool (HandleBuffer);

  return Status;
}


/**
  Get Nvme device identify data.

  @param[in]   NvmeDevice        The pointer to the NVME_PASS_THRU_DEVICE data structure.
  @param[in]   NamespaceId       NamespaceId for an NVM Express namespace present on the NVM Express controller
  @param[in]   IdentifyStructure Specifies the information to be returned to host.
  @param[out]  Buffer            The buffer used to store the identify controller data.

  @return EFI_SUCCESS            Successfully get the identify controller data.
  @return EFI_DEVICE_ERROR       Fail to get the identify controller data.
**/
EFI_STATUS
NvmeIdentifyCommand (
  IN  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL    *NvmeDevice,
  IN  UINT32                                NamespaceId,
  IN  UINT32                                IdentifyStructure,
  OUT VOID                                  *Buffer
  )
{
  EFI_STATUS                                Status;
  EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET  CommandPacket;
  EFI_NVM_EXPRESS_COMMAND                   Command;
  EFI_NVM_EXPRESS_COMPLETION                Completion;

  ZeroMem (&CommandPacket, sizeof(EFI_NVM_EXPRESS_PASS_THRU_COMMAND_PACKET));
  ZeroMem (&Command, sizeof(EFI_NVM_EXPRESS_COMMAND));
  ZeroMem (&Completion, sizeof(EFI_NVM_EXPRESS_COMPLETION));

  DEBUG ((DEBUG_INFO, "Sending Identify Command with Cns = %d\n", IdentifyStructure));

  Command.Cdw0.Opcode          = NVME_ADMIN_IDENTIFY_CMD;

  Command.Nsid                 = NamespaceId;

  CommandPacket.NvmeCmd        = &Command;
  CommandPacket.NvmeCompletion = &Completion;
  CommandPacket.TransferBuffer = Buffer;
  CommandPacket.TransferLength = sizeof (NVME_ADMIN_CONTROLLER_DATA);
  CommandPacket.CommandTimeout = NVME_GENERIC_TIMEOUT;
  CommandPacket.QueueType      = NVME_ADMIN_QUEUE;
  //
  // Set bit 0 (Cns bit) to 0 to identify a namespace / 1 to identify a controller
  //
  Command.Cdw10                = IdentifyStructure;
  Command.Flags                = CDW10_VALID;

  Status = NvmeDevice->PassThru (NvmeDevice, NamespaceId, &CommandPacket, NULL);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Identify Command Status=%r\n", Status));
  }

  return Status;
}

/**
  Checks if the given device is a USBr device.

  @param[in]   DevicePath         A pointer to a device path data structure.

  @retval      True               Current device is a USBr device.
  @return      False              Current device is not a USBr device.
**/
BOOLEAN
EFIAPI
IsUsbrDevice(
  IN  EFI_DEVICE_PATH_PROTOCOL          *DevicePath
  )
{
  EFI_DEVICE_PATH_PROTOCOL             *DevicePathNode;

  DevicePathNode  = DevicePath;

  while (!IsDevicePathEnd (DevicePathNode)) {
    if ((DevicePathType (DevicePathNode) == MESSAGING_DEVICE_PATH) && (DevicePathSubType (DevicePathNode) == MSG_USB_DP)) {
      if (((IsPchH ()) && (((USB_DEVICE_PATH *)DevicePathNode)->ParentPortNumber == V_ME_PCH_H_USBR2_PORT_NUMBER)) ||
          ((IsPchLp ()) && (((USB_DEVICE_PATH *)DevicePathNode)->ParentPortNumber == V_ME_PCH_LP_USBR2_PORT_NUMBER))) {
        return TRUE;
      }
    }
    DevicePathNode = NextDevicePathNode (DevicePathNode);
  }
  return FALSE;
}


/**
  Searches the device path for the handle that has a pass thru protocol installed.

  @param[in]   DevicePath         A pointer to a device path data structure.
  @param[in]   ProtocolGuid       Guid of a protocol that is being searched.

  @retval      True               Pass Thru Protocol was located.
  @return      False              Failed to locate Pass Thru Protocol.
**/
BOOLEAN
EFIAPI
IsPassThruProtocolAvailable (
  IN  EFI_DEVICE_PATH_PROTOCOL          *DevicePath,
  IN  EFI_GUID                          ProtocolGuid
  )
{
  EFI_STATUS                            Status;
  EFI_HANDLE                            Handle;

  Status = gBS->LocateDevicePath (
                  &ProtocolGuid,
                  &DevicePath,
                  &Handle
                  );
  if (EFI_ERROR (Status)) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Get Sata HDD identify data.

  @param[in]          AtaDevice           The pointer to the EFI_ATA_PASS_THRU_PROTOCOL.
  @param[in]          Port                Port number on the ATA controller
  @param[in]          PortMultiplierPort  Port multiplier port number on the ATA controller
  @param[out]         ControllerData      The buffer used to store the identify controller data.

  @return             EFI_SUCCESS         Successfully get the identify controller data.
  @return             EFI_DEVICE_ERROR    Fail to get the identify controller data.
**/
EFI_STATUS
GetHddIdentifyData (
  IN EFI_ATA_PASS_THRU_PROTOCOL             *AtaDevice,
  IN UINT16                                 Port,
  IN UINT16                                 PortMultiplierPort,
  OUT ATA_IDENTIFY_DATA                     *ControllerData
  )
{
  EFI_STATUS                               Status;
  EFI_ATA_PASS_THRU_COMMAND_PACKET         Packet;
  EFI_ATA_COMMAND_BLOCK                    Acb;
  EFI_ATA_STATUS_BLOCK                     Asb;

  ZeroMem (&Packet, sizeof(EFI_ATA_PASS_THRU_COMMAND_PACKET));
  ZeroMem (&Acb, sizeof(EFI_ATA_COMMAND_BLOCK));
  ZeroMem (&Asb, sizeof(EFI_ATA_STATUS_BLOCK));

  Acb.AtaCommand             = ATA_CMD_IDENTIFY_DRIVE;

  Packet.Protocol            = EFI_ATA_PASS_THRU_PROTOCOL_PIO_DATA_IN;
  Packet.Acb                 = &Acb;
  Packet.Asb                 = &Asb;
  Packet.InDataBuffer        = ControllerData;
  Packet.InTransferLength    = sizeof (ATA_IDENTIFY_DATA);
  Packet.Length              = EFI_ATA_PASS_THRU_LENGTH_BYTES | EFI_ATA_PASS_THRU_LENGTH_SECTOR_COUNT;
  Packet.Timeout             = EFI_TIMER_PERIOD_SECONDS (3);

  Status = AtaDevice->PassThru (AtaDevice, Port, PortMultiplierPort, &Packet, NULL);

  return Status;

}

/**
  Get device identify data using Disk Info Protocol.

  @param[in]   Handle             Handle to the device.
  @param[out]  IdentifyDriveInfo  The buffer used to store the device identify data.

  @return EFI_SUCCESS             Successfully got the device identify data.
  @return Other                   Failed to get the identify controller data.
**/
EFI_STATUS
GetIdentifyDataFromDiskInfo (
  IN  EFI_HANDLE                        Handle,
  OUT EFI_IDENTIFY_DATA                 *IdentifyDriveInfo
  )
{
  EFI_STATUS                           Status;
  EFI_DISK_INFO_PROTOCOL               *DiskInfo;
  UINT32                               BufferSize;

  Status = gBS->HandleProtocol (
                  Handle,
                  &gEfiDiskInfoProtocolGuid,
                  (VOID **) &DiskInfo
                  );

  if (EFI_ERROR (Status)) {
   return Status;
  }

  BufferSize = sizeof (EFI_IDENTIFY_DATA);
  ZeroMem (IdentifyDriveInfo, sizeof (EFI_IDENTIFY_DATA));

  Status = DiskInfo->Identify (
                       DiskInfo,
                       IdentifyDriveInfo,
                       &BufferSize
                       );
  ASSERT_EFI_ERROR (Status);

  return Status;
}


/**
  Detects all Ata Devices with Ata Pass Thru Protocol installed and adds them to
  media table.

  @param[in]   MediaDeviceCount   A pointer to number of media devices in media table.

  @return EFI_SUCCESS             Successfully added Ata devices into media table.
  @return EFI_BUFFER_TOO_SMALL    Maximum number of media table entries was reached.
  @return Other                   Failed to add Ata devices into media table.
**/
EFI_STATUS
DetectAtaDevices (
  IN OUT UINT8                             *MediaDeviceCount
  )
{
  EFI_STATUS                           Status;
  UINTN                                HandleNum;
  EFI_HANDLE                           *AtaPassThruHandles;
  UINTN                                Index;
  UINTN                                WordOffset;
  EFI_ATA_PASS_THRU_PROTOCOL           *AtaDevice;
  EFI_ATA_IDENTIFY_DATA                AtaIdentifyData;
  UINT16                               Port;
  UINT16                               PortMultiplierPort;
  UINT64                               DriveSize;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiAtaPassThruProtocolGuid,
                  NULL,
                  &HandleNum,
                  &AtaPassThruHandles
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Number of detected ATA Handles: %d\n", HandleNum));

  for (Index = 0; Index < HandleNum; Index++) {

    Status = gBS->HandleProtocol (
                    AtaPassThruHandles[Index],
                    &gEfiAtaPassThruProtocolGuid,
                    (VOID **) &AtaDevice
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "gBS->Handle Protocol : Status = %r\n", Status));
      return Status;
    }

    //
    // Go through all of the ports and portmultiplierports and get identify data
    //
    Port = 0xFFFF;

    while (TRUE) {

      Status = AtaDevice->GetNextPort (AtaDevice, &Port);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          return EFI_SUCCESS;
        } else {
          return Status;
        }
      }

      PortMultiplierPort = 0xFFFF;

      while (TRUE) {
        Status = AtaDevice->GetNextDevice (AtaDevice, Port, &PortMultiplierPort);
        if (EFI_ERROR (Status)) {
          if (Status == EFI_NOT_FOUND) {
            break;
          } else {
            return Status;
          }
        }

        ZeroMem (&AtaIdentifyData, sizeof(ATA_IDENTIFY_DATA));
        Status = GetHddIdentifyData (AtaDevice, Port, PortMultiplierPort, &AtaIdentifyData);
        if (EFI_ERROR (Status)) {
          continue;
        }

        DriveSize = AtaIdentifyData.maximum_lba_for_48bit_addressing[0];
        ///
        /// Lower byte goes first: word[100] is the lowest word, word[103] is highest
        ///
        for (WordOffset = 1; WordOffset < 4; WordOffset++) {
          DriveSize |= LShiftU64 (AtaIdentifyData.maximum_lba_for_48bit_addressing[WordOffset], 16 * WordOffset);
        }
        DriveSize = MultU64x32 (DriveSize, 512);

        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].Interface = MEBX_MEDIA_IN_SATA;
        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].DevType = MEBX_MEDIA_DT_HDD;
        SwapEntries ((CHAR8 *) AtaIdentifyData.ModelName, (UINT8)MEDIA_DEVICE_MODEL_NO_MAX_LENGTH);
        SwapEntries ((CHAR8 *) AtaIdentifyData.SerialNo, (UINT8)MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH);
        CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SerialNo, AtaIdentifyData.SerialNo, MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH);
        CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].VersionNo, AtaIdentifyData.FirmwareVer, MEDIA_DEVICE_VERSION_NO_MAX_LENGTH);
        CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].ModelNo, AtaIdentifyData.ModelName, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH);
        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SupportedCmdSets[0] = AtaIdentifyData.command_set_supported_82;
        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SupportedCmdSets[1] = AtaIdentifyData.command_set_supported_83;
        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SupportedCmdSets[2] = AtaIdentifyData.command_set_feature_extn;
        mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].MaxMediaSize = DriveSize;
        (*MediaDeviceCount)++;
        if ((*MediaDeviceCount) == PCI_MAX_DEVICE + 1) {
          return EFI_BUFFER_TOO_SMALL;
        }
      }
    }
  }

  return Status;
}


/**
  Detects all Nvm Express Devices with Nvm Express Pass Thru Protocol installed and adds them to
  media table.

  @param[in]   MediaDeviceCount   A pointer to number of media devices in media table.

  @return EFI_SUCCESS             Successfully added Ata devices into media table.
  @return EFI_BUFFER_TOO_SMALL    Maximum number of media table entries was reached.
  @return Other                   Failed to add Nvme devices into media table.
**/
EFI_STATUS
DetectNvmeDevices (
  IN OUT UINT8                             *MediaDeviceCount
  )
{
  EFI_STATUS                           Status;
  EFI_NVM_EXPRESS_PASS_THRU_PROTOCOL   *NvmeDevice;
  NVME_ADMIN_CONTROLLER_DATA           ControllerData;
  NVME_ADMIN_NAMESPACE_DATA            NamespaceData;
  UINT32                               NamespaceId;
  UINTN                                HandleNum;
  EFI_HANDLE                           *NvmePassThruHandles;
  UINTN                                Index;
  UINT64                               DriveSize;

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiNvmExpressPassThruProtocolGuid,
                  NULL,
                  &HandleNum,
                  &NvmePassThruHandles
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((DEBUG_INFO, "Number of detected NVMe Handles: %d\n", HandleNum));

  for (Index = 0; Index < HandleNum; Index++) {
    Status = gBS->HandleProtocol (
                    NvmePassThruHandles[Index],
                    &gEfiNvmExpressPassThruProtocolGuid,
                    (VOID **) &NvmeDevice
                    );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "gBS->Handle Protocol : Status=%r\n", Status));
      return Status;
    }

    NamespaceId = NVME_ALL_NAMESPACES;

    while (TRUE) {
      Status = NvmeDevice->GetNextNamespace (NvmeDevice, &NamespaceId);
      if (EFI_ERROR (Status)) {
        if (Status == EFI_NOT_FOUND) {
          Status = EFI_SUCCESS;
          break;
        } else {
          return Status;
        }
      }
      ZeroMem (&ControllerData, sizeof(NVME_ADMIN_CONTROLLER_DATA));
      ZeroMem (&NamespaceData, sizeof(NVME_ADMIN_NAMESPACE_DATA));
      Status = NvmeIdentifyCommand (NvmeDevice, NamespaceId, NVME_IDENTIFY_NAMESPACE_STRUCT, (VOID *) &NamespaceData);
      if (EFI_ERROR (Status)) {
        continue;
      }
      DriveSize = MultU64x32 (NamespaceData.Ncap, 512);
      //
      // Due to the fact that RAID Driver expects other value of NamespaceId parameter than AHCI driver,
      // need to send the Identify Controller command twice - once with the NamespaceId retrieved
      // using GetNextNamespace. If it fails we need to set NamespaceId = 0.
      //
      Status = NvmeIdentifyCommand (NvmeDevice, NamespaceId, NVME_IDENTIFY_CONTROLLER_STRUCT, (VOID *) &ControllerData);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_WARN, "NvmeIdentifyCommand Error. Sending Identify Command once again\n"));
        Status = NvmeIdentifyCommand (NvmeDevice, NVME_CONTROLLER_ID, NVME_IDENTIFY_CONTROLLER_STRUCT, (VOID *) &ControllerData);
        if (EFI_ERROR (Status)) {
          continue;
        }
      }
      mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].Interface = MEBX_MEDIA_IN_PCIE;
      mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].DevType   = MEBX_MEDIA_DT_HDD;
      CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].ModelNo, ControllerData.Mn, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH);
      CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SerialNo,ControllerData.Sn, MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH);
      CopyMem (mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].VersionNo, ControllerData.Fr, MEDIA_DEVICE_VERSION_NO_MAX_LENGTH);
      mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SupportedCmdSets[0] = ControllerData.Oacs;
      mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].SupportedCmdSets[1] = ControllerData.Oncs;
      mAmtMediaFru.MediaDevInfo[*MediaDeviceCount].MaxMediaSize = DriveSize;
      (*MediaDeviceCount)++;
      if ((*MediaDeviceCount) == PCI_MAX_DEVICE + 1) {
        return EFI_BUFFER_TOO_SMALL;
      }
    }
  }

  return Status;
}

/**
  Get last device before end of device path node.

  @param[in]   DevicePath         A pointer to a device path data structure.
  @param[out]  Type               The Type field of the device path node specified by Node.
  @param[out]  SubType            The SubType field of the device path node specified by Node.
**/
VOID
EFIAPI
GetLastDeviceFromDevicePath (
  IN  EFI_DEVICE_PATH_PROTOCOL         *DevicePath,
  OUT UINT8                            *Type,
  OUT UINT8                            *Subtype
  )
{
  EFI_DEVICE_PATH_PROTOCOL             *DevicePathNode;

  DevicePathNode  = DevicePath;

  while (!IsDevicePathEnd (DevicePathNode)) {
    *Type = DevicePathType (DevicePathNode);
    *Subtype = DevicePathSubType (DevicePathNode);
    DevicePathNode = NextDevicePathNode (DevicePathNode);
  }
}

/**
  Collect all media devices. First look for Pass Thru Protocols for ATA and NVMe drives.
  Check device paths afterwards and look for the messaging device paths.
**/
VOID
EFIAPI
BuildMediaList (
  VOID
  )
{
  EFI_STATUS                           Status;
  UINTN                                HandleCount;
  EFI_HANDLE                           *HandleBuffer;
  EFI_BLOCK_IO_PROTOCOL                *BlkIo;
  EFI_DEVICE_PATH_PROTOCOL             *DevicePath;
  UINTN                                Index;
  EFI_IDENTIFY_DATA                    IdentifyDriveInfo;
  UINT8                                MediaDeviceCount;
  UINT8                                Type;
  UINT8                                Subtype;
  BOOLEAN                              ProtocolAvailable;

  MediaDeviceCount  = 0;

  Status = DetectAtaDevices (&MediaDeviceCount);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    return;
  }

  Status = DetectNvmeDevices (&MediaDeviceCount);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    return;
  }

  ///
  /// Locate all media devices connected.
  /// We look for the Block I/O protocol to be attached to the device.
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiBlockIoProtocolGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return;
  }

  DEBUG ((DEBUG_INFO, "HandleCount = 0x%x\n", HandleCount));
  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiBlockIoProtocolGuid,
                    (VOID **) &BlkIo
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// We want to remove any Block I/O instances that refer to Logical partitions.
    /// A Block I/O instance is added to the raw device and any partition discovered on
    /// the media. This prevents duplications in our table.
    ///
    if (BlkIo->Media->LogicalPartition) {
      continue;
    }

    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiDevicePathProtocolGuid,
                    (VOID *) &DevicePath
                    );
    ASSERT_EFI_ERROR (Status);

    ZeroMem (&mAmtMediaFru.MediaDevInfo[MediaDeviceCount], sizeof (MEBX_FRU_MEDIA_DEVICES));
    mAmtMediaFru.MediaDevInfo[MediaDeviceCount].StructSize  = sizeof (MEBX_FRU_MEDIA_DEVICES);
    GetLastDeviceFromDevicePath (DevicePath, &Type, &Subtype);
    if (Type == MESSAGING_DEVICE_PATH) {
      switch (Subtype) {
        case MSG_ATAPI_DP:
          mAmtMediaFru.MediaDevInfo[MediaDeviceCount].MaxMediaSize = MultU64x32 (BlkIo->Media->LastBlock + 1, BlkIo->Media->BlockSize);
          mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_ATAPI;
          if (BlkIo->Media->RemovableMedia) {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType = MEBX_MEDIA_DT_DVD;
          } else {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType = MEBX_MEDIA_DT_HDD;
          }

          Status = GetIdentifyDataFromDiskInfo (HandleBuffer[Index], &IdentifyDriveInfo);
          if (!EFI_ERROR (Status)) {
            SwapEntries ((CHAR8 *) &IdentifyDriveInfo.AtapiData.ModelName, (UINT8)MEDIA_DEVICE_MODEL_NO_MAX_LENGTH);
            SwapEntries ((CHAR8 *) &IdentifyDriveInfo.AtapiData.SerialNo, (UINT8)MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH);
            CopyMem (mAmtMediaFru.MediaDevInfo[MediaDeviceCount].SerialNo, IdentifyDriveInfo.AtapiData.SerialNo, MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH);
            CopyMem (mAmtMediaFru.MediaDevInfo[MediaDeviceCount].VersionNo, IdentifyDriveInfo.AtapiData.FirmwareVer, MEDIA_DEVICE_VERSION_NO_MAX_LENGTH);
            CopyMem (mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, IdentifyDriveInfo.AtapiData.ModelName, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH);
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].SupportedCmdSets[0] = IdentifyDriveInfo.AtapiData.cmd_set_support_82;
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].SupportedCmdSets[1] = IdentifyDriveInfo.AtapiData.cmd_set_support_83;
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].SupportedCmdSets[2] = IdentifyDriveInfo.AtapiData.cmd_feature_support;
            MediaDeviceCount++;
            break;
          }

          AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "ATAPI");
          MediaDeviceCount++;
          break;

        case MSG_USB_DP:
          if (IsUsbrDevice (DevicePath) == FALSE) {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].MaxMediaSize = MultU64x32 (BlkIo->Media->LastBlock + 1, BlkIo->Media->BlockSize);
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_ATA;
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType   = MEBX_MEDIA_DT_HDD;
            AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "USB");
            MediaDeviceCount++;
          }
          break;

        case MSG_SATA_DP:
          //
          // If this is a Removable Media it means the device is a ODD
          //
          if (BlkIo->Media->RemovableMedia) {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].MaxMediaSize = MultU64x32 (BlkIo->Media->LastBlock + 1, BlkIo->Media->BlockSize);
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_SATA;
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType   = MEBX_MEDIA_DT_DVD;
            AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "DVD Disk");
            MediaDeviceCount++;
            break;
          }
          //
          // If Protocol is available it means that the device was already identified and we should not increase
          // media device count.
          //
          ProtocolAvailable = IsPassThruProtocolAvailable (DevicePath, gEfiAtaPassThruProtocolGuid);
          if (ProtocolAvailable == FALSE) {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].MaxMediaSize = MultU64x32 (BlkIo->Media->LastBlock + 1, BlkIo->Media->BlockSize);
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_SATA;
            if (BlkIo->Media->RemovableMedia) {
              mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType = MEBX_MEDIA_DT_DVD;
            } else {
              mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType = MEBX_MEDIA_DT_HDD;
            }
            AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "Unknown SATA Disk");
            MediaDeviceCount++;
          }
          break;

        case MSG_NVME_NAMESPACE_DP:
          ProtocolAvailable = IsPassThruProtocolAvailable (DevicePath, gEfiNvmExpressPassThruProtocolGuid);
          if (ProtocolAvailable == FALSE) {
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].MaxMediaSize = MultU64x32 (BlkIo->Media->LastBlock + 1, BlkIo->Media->BlockSize);
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_PCIE;
            mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType   = MEBX_MEDIA_DT_HDD;
            AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "Unknown NVMe Disk");
            MediaDeviceCount++;
          }
          break;

        default:
          mAmtMediaFru.MediaDevInfo[MediaDeviceCount].Interface = MEBX_MEDIA_IN_ATA;
          mAmtMediaFru.MediaDevInfo[MediaDeviceCount].DevType   = MEBX_MEDIA_DT_MO;
          AsciiStrCpyS ((CHAR8 *) mAmtMediaFru.MediaDevInfo[MediaDeviceCount].ModelNo, MEDIA_DEVICE_MODEL_NO_MAX_LENGTH, "Unknown");
          MediaDeviceCount++;
          break;
      }

      if (MediaDeviceCount == PCI_MAX_DEVICE + 1) {
        break;
      }
    }
  }

  mAmtMediaFru.MediaDevicesHeader.VersionInfo = DEVICES_LIST_VERSION;
  mAmtMediaFru.MediaDevicesHeader.DevCount    = MediaDeviceCount;

  FreePool (HandleBuffer);
}
