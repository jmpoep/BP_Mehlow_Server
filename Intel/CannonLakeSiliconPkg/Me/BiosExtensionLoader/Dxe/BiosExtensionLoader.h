/** @file
  Include for AMT Bios Extentions Loader

@copyright
  INTEL CONFIDENTIAL
  Copyright 2004 - 2018 Intel Corporation.

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
#ifndef _AMT_PTBX_H_
#define _AMT_PTBX_H_

#include <Uefi.h>
#include <PiDxe.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Protocol/DevicePath.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PerformanceLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>
#include <IndustryStandard/Pci22.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/IdeControllerInit.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/Acpi.h>
#include <Protocol/AmtPolicy.h>
#include <Protocol/Wdt.h>
#include <Protocol/MebxProtocol.h>
#include <MebxDataHob.h>
#include <Protocol/AmtReadyToBoot.h>
#include <Protocol/AmtSaveMebxProtocol.h>
#include <MeChipset.h>
#include <PchAccess.h>
#include <Library/PciSegmentLib.h>
#include <Library/DxeMeLib.h>
#include <Library/DxeAmtLib.h>
#include <Library/DxeAsfLib.h>
#include <Library/DxeAmtPolicyLib.h>
#include <Library/TimerLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <MeBiosPayloadHob.h>
#include "Inventory.h"
#include "UsbProvision.h"


#define MEBX_RET_ACTION_CONTINUE_TO_BOOT  0x01
#define MEBX_RET_ACTION_RESET             0x04
#define MEBX_RET_CODE_MASK                0x07

#define MEBX_RET_ACTION_GLOBAL_RESET      0x01

#define MEBX_USER_ENTERED_RESPONSE        BIT1
#define MEBX_RA_SELECTION_MENU            BIT2
#define MEBX_HIDE_UNCONFIGURE_CONF_PROMPT BIT6
#define MEBX_DEBUG_MSG                    BIT14
#define MEBX_UNCONFIGURE                  BIT15

#define MEBX_STF_PCI_DEV_TABLE            0x0001
#define MEBX_STF_MEDIA_DEV_TABLE          0x0002

#define MEBX_MEDIA_IN_ATA                 0x0000
#define MEBX_MEDIA_IN_ATAPI               0x0001
#define MEBX_MEDIA_IN_SATA                0x0002
#define MEBX_MEDIA_IN_PCIE                0x0003

#define MEBX_MEDIA_DT_HDD                 0x0000
#define MEBX_MEDIA_DT_CDROM               0x0001
#define MEBX_MEDIA_DT_DVD                 0x0002
#define MEBX_MEDIA_DT_FDD                 0x0003
#define MEBX_MEDIA_DT_MO                  0x0004

//
// MEBX Software Class DXE Subclass Progress Code definitions.
//
#define EFI_SW_DXE_MEBX_OPROM_DONE        (EFI_OEM_SPECIFIC | 0x00000000)
#define EFI_SW_DXE_MEBX_OTHER_UNSPECD     (EFI_OEM_SPECIFIC | 0x00000009)
#define EFI_SW_DXE_MEBX_RESET_ACTION      (EFI_OEM_SPECIFIC | 0x0000000a)

#pragma pack(1)

#define DEVICES_LIST_VERSION               0x0001
#define AMT_DATA_VERSION                   0x0101
#define MEBX_VERSION                       0xC000      ///< for MEBx 12.0.0.1 or later
#define MEBX_SETUP_PLATFORM_MNT_DEFAULT    MNT_OFF
#define MEBX_SETUP_SOL_DEFAULT             0
#define MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH  20
#define MEDIA_DEVICE_VERSION_NO_MAX_LENGTH 8
#define MEDIA_DEVICE_MODEL_NO_MAX_LENGTH   40

#define SMB_SIG                            0x5F4D535F
#define RSDT_SIG                           0x54445352  ///< RSDT (in reverse)
#define ASF_SIG                            0x21465341  ///< ASF! (in reverse)
#define ONEMIN                             60000000
#define FIVEMIN                            300000000
#define TENMIN                             600000000

#define INVOKE_MEBX_BIT                    BIT3

typedef enum {
  HWAI_FRU_TYPE_NONE = 0,
  HWAI_FRU_TYPE_PCI
} HWAI_FRU_TYPE;

typedef struct {
  UINT16  VersionInfo;
  UINT8   DevCount;
  UINT8   Reserved1[5];
} AMT_FRU_DEVICES_HEADER;

typedef struct {
  UINT16  Vid;
  UINT16  Did;
  UINT16  Rid;
  UINT16  SubSysVid;
  UINT16  SubSysDid;
  UINT32  ClassCode;
  UINT16  BusDevFcn;
} MEBX_FRU_PCI_DEVICES;

typedef struct {
  AMT_FRU_DEVICES_HEADER PciDevicesHeader;
  MEBX_FRU_PCI_DEVICES   PciDevInfo[PCI_MAX_BUS + 1];
} AMT_PCI_FRU;

typedef struct {
  UINT16 VersionInfo;
  UINT8  PciDevCount;
  UINT8  Reserved[5];
} PCI_DEV_INFO;

typedef struct {
  UINT16 VendorID;
  UINT16 DeviceID;
  UINT16 RevID;
  UINT16 SubsystemVendorID;
  UINT16 SubsystemDeviceID;
  UINT32 ClassCode;
  UINT16 BusDevFunc;
} HWAI_PCI_DATA;

typedef struct {
  UINT8         SmbiosType;
  UINT8         Length;
  UINT16        SmbiosHandle;
  UINT32        FruType;
  HWAI_PCI_DATA FruData;
} HWAI_PCI_FRU;

typedef struct {
  UINT16        TableByteCount;
  UINT16        StructureCount;
  HWAI_PCI_FRU  Data[1];
} HWAI_FRU_TABLE;

typedef struct {
  UINT16 StructSize;
  UINT8  Interface;
  UINT8  DevType;
  UINT8  Rsvd[40];
  UINT8  SerialNo[MEDIA_DEVICE_SERIAL_NO_MAX_LENGTH];
  UINT8  VersionNo[MEDIA_DEVICE_VERSION_NO_MAX_LENGTH];
  UINT8  ModelNo[MEDIA_DEVICE_MODEL_NO_MAX_LENGTH];
  UINT64 MaxMediaSize;
  UINT16 SupportedCmdSets[3];
} MEBX_FRU_MEDIA_DEVICES;

typedef struct {
  UINT8                  SmbiosType;
  UINT8                  Length;
  UINT16                 SmbiosHandle;
  MEBX_FRU_MEDIA_DEVICES MediaData;
} HWAI_MEDIA_ENTRY;

typedef struct {
  UINT16            TableByteCount;
  UINT16            StructureCount;
  HWAI_MEDIA_ENTRY  Data[1];
} HWAI_MEDIA_TABLE;

typedef struct {
  UINT16  VersionInfo;
  UINT8   MediaDevCount;
  UINT8   Reserved[5];
} MEDIA_DEV_INFO;

typedef struct {
  AMT_FRU_DEVICES_HEADER MediaDevicesHeader;
  MEBX_FRU_MEDIA_DEVICES MediaDevInfo[PCI_MAX_DEVICE + 1];
} AMT_MEDIA_FRU;

typedef struct {
  UINT8   Type;
  UINT8   Length;
  UINT16  Handle;
} SMBIOS_HEADER;

typedef struct {
  UINT32 Signature;
  UINT8  EpChecksum;
  UINT8  EpLength;
  UINT8  MajorVersion;
  UINT8  MinorVersion;
  UINT16 MaxStruct_Size;
  UINT8  EpRevision;
  UINT8  RevInfo[5];
  UINT8  DmiSignature[5];                          ///< _DMI_
  UINT8  IntChecksum;
  UINT16 TableLength;
  UINT32 TableAddress;
  UINT16 StructureCnt;
  UINT8  SmbBcdRevision;
} SMBIOS_ENTRY_POINT;

typedef struct {
  SMBIOS_HEADER Header;
  UINT8         ManufacturerStr;
  UINT8         ProductNameStr;
  UINT8         VersionStr;
  UINT8         SerialNumberStr;
  UINT8         Uuid[16];
  UINT8         WakeUp;
} SMBIOS_TABLE_TYPE_ONE;

typedef struct {
  SMBIOS_HEADER Header;
  UINT8         ManufacturerStr;
  UINT8         Type;
  UINT8         VersionStr;
  UINT8         SerialNumberStr;
  UINT8         AssetTagStr;
  UINT8         BootState;
  UINT8         PowerSupplyState;
  UINT8         ThermalState;
  UINT8         SecurityStatus;
  UINT32        VendorInfo;
} SMBIOS_TABLE_TYPE_THREE;

typedef struct {
  SMBIOS_HEADER Header;
  UINT8         SocketStr;
  UINT8         Type;
  UINT8         Family;
  UINT8         Manufacturer;
  UINT8         ProcId[8];
  UINT8         VersionStr;
  UINT8         Voltage;
  UINT16        ExternalClock;
  UINT16        MaxSpeed;
  UINT16        CurrentSpeed;
  UINT8         Status;
  UINT8         Upgrade;
  UINT16        L1CacheHandle;
  UINT16        L2CacheHandle;
  UINT16        L3CacheHandle;
} SMBIOS_TABLE_TYPE_FOUR;

typedef struct {
  SMBIOS_HEADER Header;
  UINT8         Capabilities;
  UINT16        ResetCount;
  UINT16        ResetLimit;
  UINT16        TimerInterval;
  UINT16        Timeout;
} SMBIOS_TABLE_TYPE_TWENTY_THREE;

typedef struct {
  UINT32 Signature;
  UINT32 Length;
  UINT8  Revision;
  UINT8  Checksum;
  UINT8  OemId[6];
  UINT8  OemTableId[8];
  UINT32 OemRevision;
  UINT8  CreatorId[4];
  UINT32 CreatorRev;
} ACPI_HEADER;

typedef struct {
  UINT16 VersionInfo;
  UINT8  Reserved[2];
  UINT32 PtrSmbiosTable;
  UINT32 PtrAcpiRsdt;
  UINT16 SupportedTablesFlags;
  UINT32 PciDevAssertInfoPtr;
  UINT32 MediaDevAssetInfoPtr;
} AMT_DATA;

#pragma pack()

/**
  Signal a event for Amt ready to boot.

  @retval EFI_SUCCESS             Mebx launched or no controller
**/
EFI_STATUS
EFIAPI
AmtReadyToBoot (
  VOID
  );

/**
  This routine is run at boot time.
  1) Initialize AMT library.
  2) Check if MEBx is required to be launched by user.
  3) Build and send asset tables to ME FW.
  4) Check USB provision.
  5) Hide unused AMT devices in prior to boot.

**/
VOID
MebxNotifyEvent (
  VOID
  );

/**
  Detect EFI MEBx support; Loading and execute.

  @param[in] MebxBiosParamsBuffer MebxBiosParams Flat pointer

  @retval EFI_SUCCESS             Function executed successfully.
  @retval EFI_NOT_STARTED         MEBx not started.
**/
EFI_STATUS
AdjustAndExecuteMebxImage (
  IN MEBX_BPF *MebxBiosParams
  );

/**
  Detect BIOS invoke reasons.

  @param[in] InvokeMebx           Pointer to the Invoke MEBx flag
  @param[in] MebxBiosParamsBuffer MebxBiosParams Flat pointer
**/
VOID
CheckForBiosInvokeReason (
  IN  UINT8       *InvokeMebx,
  IN  MEBX_BPF    *MebxBiosParams
  );

/**
  Dump MEBx BIOS Params

  @param[in]   MebxBiosParams - MEBx BIOS params
**/
VOID
DxeMebxBiosParamsDebugDump (
  IN MEBX_BPF *MebxBiosParams
  );

extern AMT_MEDIA_FRU  mAmtMediaFru;
extern AMT_PCI_FRU    mAmtPciFru;

//
// USB Provisioning
//
extern VOID           *mUsbProvData;
extern VOID           *mUsbProvDataBackup;
extern UINTN          mUsbProvDataSize;
extern BOOLEAN        mUsbProvsionDone;
extern UINT8          mUsbKeyBus, mUsbKeyPort, mUsbKeyDevice, mUsbKeyFunction;

#endif // _AMT_PTBX_H_
