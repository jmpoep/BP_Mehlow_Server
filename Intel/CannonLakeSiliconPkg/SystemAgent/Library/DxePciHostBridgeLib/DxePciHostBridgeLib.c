/** @file
  PciHostBridge Library

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2018 Intel Corporation.

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

@par Specification
**/
#include <PiDxe.h>
#include <IndustryStandard/Pci.h>
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/SaPolicy.h>
#include <Private/Protocol/SaNvsArea.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PciHostBridgeLib.h>
#include <Library/HobLib.h>
#include <Library/PciSegmentLib.h>
#include <Private/SaConfigHob.h>

///
/// Support 4G address space
///
#define HIGHEST_SUPPORTED_ADDRESS 0x8000000000L

typedef union {
  struct {
    UINT32  Low;
    UINT32  High;
  } Data32;
  UINT64 Data;
} UINT64_STRUCT;

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                           mEnableAbove4GBMmioBiosAssignemnt;
GLOBAL_REMOVE_IF_UNREFERENCED SYSTEM_AGENT_NVS_AREA_PROTOCOL  *mSaNvsAreaProtocol;
GLOBAL_REMOVE_IF_UNREFERENCED UINT64                          mMmio64Base;
GLOBAL_REMOVE_IF_UNREFERENCED UINT64                          mMmio64Length;
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                          mMmio32Base;
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                          mMmio32Length;

GLOBAL_REMOVE_IF_UNREFERENCED CHAR16 *mPciHostBridgeLibAcpiAddressSpaceTypeStr[] = {
  L"Mem", L"I/O", L"Bus"
};
ACPI_HID_DEVICE_PATH mRootBridgeDeviceNode = {
  {
    ACPI_DEVICE_PATH,
    ACPI_DP,
    {
      (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)),
      (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8)
    }
  },
  EISA_PNP_ID (0x0A03),
  0
};

PCI_ROOT_BRIDGE mRootBridge = {
  0,
  EFI_PCI_ATTRIBUTE_ISA_MOTHERBOARD_IO |
  EFI_PCI_ATTRIBUTE_IDE_PRIMARY_IO |
  EFI_PCI_ATTRIBUTE_ISA_IO |
  EFI_PCI_ATTRIBUTE_ISA_IO_16 |
  EFI_PCI_ATTRIBUTE_VGA_PALETTE_IO |
  EFI_PCI_ATTRIBUTE_VGA_PALETTE_IO_16 |
  EFI_PCI_ATTRIBUTE_VGA_MEMORY |
  EFI_PCI_ATTRIBUTE_VGA_IO |
  EFI_PCI_ATTRIBUTE_VGA_IO_16, // Supports;
  EFI_PCI_ATTRIBUTE_VGA_IO_16, // Attributes;
  FALSE, // DmaAbove4G;
  FALSE, // NoExtendedConfigSpace;
  FALSE, // ResourceAssigned;
  EFI_PCI_HOST_BRIDGE_COMBINE_MEM_PMEM, // AllocationAttributes
  { 0, 255 }, // Bus
  { 0, 0 }, // Io - to be fixed later
  { 0, 0 }, // Mem - to be fixed later
  { BASE_4GB, 0 }, // MemAbove4G - default set Base > Limit to disable Above4G, dynamic update the limit when enable Above4G
  { MAX_UINT32, 0 }, // PMem - COMBINE_MEM_PMEM indicating no PMem and PMemAbove4GB, set Base > Limit to disable it.
  { MAX_UINT32, 0 }, // PMemAbove4G, set Base > Limit to disable it.
  NULL // DevicePath;
};

/**
  A protocol callback which updates 64bits MMIO Base and Length in SA GNVS area

  @param[in] Event    - The triggered event.
  @param[in] Context  - Context for this event.

**/
VOID
UpdateSaGnvsForMmioResourceBaseLength (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
  EFI_STATUS                    Status;

  Status = gBS->LocateProtocol (&gSaNvsAreaProtocolGuid, NULL, (VOID **) &mSaNvsAreaProtocol);
  if (Status != EFI_SUCCESS) {
    return;
  }
  gBS->CloseEvent (Event);

  DEBUG ((DEBUG_INFO, "[PciHostBridgeLib] Update SA GNVS Area.\n"));
  mSaNvsAreaProtocol->Area->Mmio64Base                    = mMmio64Base;
  mSaNvsAreaProtocol->Area->Mmio64Length                  = mMmio64Length;
  mSaNvsAreaProtocol->Area->Mmio32Base                    = mMmio32Base;
  mSaNvsAreaProtocol->Area->Mmio32Length                  = mMmio32Length;
}

PCI_ROOT_BRIDGE *
EFIAPI
PciHostBridgeGetRootBridges (
  UINTN                                 *Count
  )
{
  EFI_STATUS                Status;
  EFI_PHYSICAL_ADDRESS      PciBaseAddress;
#ifdef CPU_CFL
  UINT64_STRUCT             RemapBase;
  UINT64_STRUCT             RemapLimit;
  UINT64_STRUCT             MeSegMask;
  EFI_PHYSICAL_ADDRESS      MeStolenSize;
  BOOLEAN                   MeStolenEnable;
#endif
  UINT32                    Tolud;
  UINT64                    Length;
  UINT64                    McD0BaseAddress;
  UINTN                     ResMemLimit1;
  SA_CONFIG_HOB             *SaConfigHob;
  EFI_EVENT                 SaGlobalNvsUpdateEvent;
  VOID                      *SaGlobalNvsInstalledRegistration;
  SA_POLICY_PROTOCOL        *SaPolicy;
  MISC_DXE_CONFIG           *MiscDxeConfig;
#ifndef CPU_CFL
  UINT64                    RegBarBase;
  UINT64_STRUCT             MchBar;
  UINT64                    Size;

  RegBarBase = 0;
  MchBar.Data32.High = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, 0, 0, 0, R_SA_MCHBAR + 4));
  MchBar.Data32.Low  = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, 0, 0, 0, R_SA_MCHBAR));
  MchBar.Data       &= (UINT64) ~(B_SA_MCHBAR_MCHBAREN_MASK);
#endif

  SaConfigHob                       = NULL;
  SaPolicy                          = NULL;
  mEnableAbove4GBMmioBiosAssignemnt = 0;
  mMmio64Base                       = 0;
  mMmio64Length                     = 0;
  mMmio32Base                       = 0;
  mMmio32Length                     = 0;

  ///
  /// Allocate 56 KB of I/O space [0x2000..0xFFFF]
  ///
  DEBUG ((DEBUG_INFO, " Assign IO resource for PCI_ROOT_BRIDGE from 0x%X to 0x%X\n", PcdGet16 (PcdPciReservedIobase) ,PcdGet16 (PcdPciReservedIoLimit)));
  mRootBridge.Io.Base = PcdGet16 (PcdPciReservedIobase);
  mRootBridge.Io.Limit = PcdGet16 (PcdPciReservedIoLimit);

  ///
  /// Read memory map registers
  ///
  McD0BaseAddress        = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, 0);
#ifdef CPU_CFL
  RemapBase.Data32.High  = PciSegmentRead32 (McD0BaseAddress + R_SA_REMAPBASE + 4);
  RemapBase.Data32.Low   = PciSegmentRead32 (McD0BaseAddress + R_SA_REMAPBASE);
  RemapBase.Data        &= B_SA_REMAPBASE_REMAPBASE_MASK;
  RemapLimit.Data32.High = PciSegmentRead32 (McD0BaseAddress + R_SA_REMAPLIMIT + 4);
  RemapLimit.Data32.Low  = PciSegmentRead32 (McD0BaseAddress + R_SA_REMAPLIMIT);
  RemapLimit.Data       &= B_SA_REMAPLIMIT_REMAPLMT_MASK;
#else
  RegBarBase             = MmioRead64 (MchBar.Data + R_SA_MCHBAR_REGBAR_OFFSET) & ~(BIT0);
#endif
  Tolud                  = PciSegmentRead32 (McD0BaseAddress + R_SA_TOLUD) & B_SA_TOLUD_TOLUD_MASK;
  PciBaseAddress         = Tolud;

#ifdef CPU_CFL
  MeSegMask.Data32.High  = PciSegmentRead32 (McD0BaseAddress + R_SA_MESEG_MASK + 4);
  MeSegMask.Data32.Low   = PciSegmentRead32 (McD0BaseAddress + R_SA_MESEG_MASK);
  MeStolenEnable         = (BOOLEAN) ((MeSegMask.Data & B_SA_MESEG_MASK_ME_STLEN_EN_MASK) != 0);

  ///
  /// First check if memory remap is used
  ///
  if ((RemapBase.Data > RemapLimit.Data) && (MeStolenEnable)) {
    MeStolenSize = MeSegMask.Data & B_SA_MESEG_MASK_MEMASK_MASK;
    if (MeStolenSize != 0) {
      MeStolenSize = HIGHEST_SUPPORTED_ADDRESS - MeStolenSize;
    }
    ///
    /// Remap is disabled -> PCI starts at TOLUD + ME Stolen size
    ///
    PciBaseAddress += MeStolenSize;
  }
#endif

  ResMemLimit1 = PcdGet32 (PcdPciReservedMemLimit);
  if (ResMemLimit1 == 0) {
    ResMemLimit1 = (UINTN) PcdGet64 (PcdPciExpressBaseAddress);
  }

  Length = ResMemLimit1 - PciBaseAddress;

  if (Length != 0) {
    DEBUG ((DEBUG_INFO, " Assign Memory Resource for PCI_ROOT_BRIDGE from 0x%X to 0x%X\n", (UINT32) PciBaseAddress, (UINT32)
            (PciBaseAddress + Length - 1)));
    mMmio32Base = (UINT32) PciBaseAddress;
    mMmio32Length = (UINT32) Length;

    mRootBridge.Mem.Base = mMmio32Base;
    mRootBridge.Mem.Limit = mMmio32Base + mMmio32Length - 1;
  }

  ///
  /// Retrieve SaPolicy and see if above 4GB MMIO BIOS assignment enabled
  ///
  if (SaPolicy == NULL) {
    Status = gBS->LocateProtocol (&gSaPolicyProtocolGuid, NULL, (VOID **) &SaPolicy);
    if (SaPolicy != NULL) {
      Status = GetConfigBlock ((VOID *) SaPolicy, &gMiscDxeConfigGuid, (VOID *)&MiscDxeConfig);
      ASSERT_EFI_ERROR (Status);
      if ((MiscDxeConfig != NULL) && (MiscDxeConfig->EnableAbove4GBMmio == 1)) {
        mEnableAbove4GBMmioBiosAssignemnt = 1;
      }
    }
  }

  ///
  /// Enable Above 4GB MMIO when Aperture Size is 2GB or higher
  ///
  ///
  /// Get SaConfigHob HOB
  ///
  SaConfigHob = (SA_CONFIG_HOB *) GetFirstGuidHob (&gSaConfigHobGuid);
  if ((SaConfigHob != NULL) && (SaConfigHob->ApertureSize >= 15)) {
    mEnableAbove4GBMmioBiosAssignemnt = 1;
  }
  if (mEnableAbove4GBMmioBiosAssignemnt == 1) {
    ///
    /// Provide 256GB available above 4GB MMIO resource
    /// limited to use single variable MTRR to cover this above 4GB MMIO region.
    ///
    PciBaseAddress = PcdGet64 (PcdAbove4GBMmioBase);
    Length         = PcdGet64 (PcdAbove4GBMmioSize);
    if (Length != 0) {
      DEBUG ((DEBUG_INFO, " PCI space that above 4GB MMIO is from 0x%lX", (UINT64) PciBaseAddress));
      DEBUG ((DEBUG_INFO, " to 0x%lX\n", (UINT64) (PciBaseAddress + Length - 1)));
      ///
      /// Update variables for above 4GB MMIO Base/Length.
      ///
      mMmio64Base   = PciBaseAddress;
      mMmio64Length = Length;

      mRootBridge.MemAbove4G.Base = mMmio64Base;
      mRootBridge.MemAbove4G.Limit = mMmio64Base + mMmio64Length - 1;
    }
  }
  ///
  /// If SA Global NVS protocol not installed yet, register SA Global Nvs protocol callback event
  ///
  Status = gBS->LocateProtocol (&gSaNvsAreaProtocolGuid, NULL, (VOID **) &mSaNvsAreaProtocol);
  if (Status == EFI_NOT_FOUND) {
    Status = gBS->CreateEvent (
                    EVT_NOTIFY_SIGNAL,
                    TPL_CALLBACK,
                    (EFI_EVENT_NOTIFY) UpdateSaGnvsForMmioResourceBaseLength,
                    NULL,
                    &SaGlobalNvsUpdateEvent
                    );
    ASSERT_EFI_ERROR (Status);
    Status = gBS->RegisterProtocolNotify (
                    &gSaNvsAreaProtocolGuid,
                    SaGlobalNvsUpdateEvent,
                    &SaGlobalNvsInstalledRegistration
                    );
    ASSERT_EFI_ERROR (Status);
  } else {
    UpdateSaGnvsForMmioResourceBaseLength (NULL, NULL);
  }

  if (mRootBridge.MemAbove4G.Base < mRootBridge.MemAbove4G.Limit) {
    mRootBridge.AllocationAttributes |= EFI_PCI_HOST_BRIDGE_MEM64_DECODE;
  }

  mRootBridge.DmaAbove4G = PcdGetBool (PcdPciDmaAbove4G);
  mRootBridge.NoExtendedConfigSpace = PcdGetBool (PcdPciNoExtendedConfigSpace);

  mRootBridge.DevicePath = AppendDevicePathNode (NULL, &mRootBridgeDeviceNode.Header);

#ifndef CPU_CFL
  ///
  /// Reserve 256KB for REGBAR memory in CNL
  ///
  Size = 0x40000;
  DEBUG ((EFI_D_INFO, "Allocating 256KB for Regbar MMIO space at 0x%X\n", RegBarBase));
  Status = gDS->AddMemorySpace (
              EfiGcdMemoryTypeMemoryMappedIo,
              RegBarBase,
              Size,
              0
              );
#endif

  *Count = 1;
  return &mRootBridge;
}

VOID
EFIAPI
PciHostBridgeFreeRootBridges (
  PCI_ROOT_BRIDGE *Bridges,
  UINTN           Count
  )
{
  ASSERT (Count == 1);
  FreePool (Bridges->DevicePath);
}

/**
  Inform the platform that the resource conflict happens.

  @param HostBridgeHandle Handle of the Host Bridge.
  @param Configuration    Pointer to PCI I/O and PCI memory resource descriptors.
                          The Configuration contains the resources for all the
                          root bridges. The resource for each root bridge is
                          terminated with END descriptor and an additional END
                          is appended indicating the end of the whole resources.
                          The resource descriptor field values follow the description
                          in EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL.SubmitResources().
**/

VOID
EFIAPI
PciHostBridgeResourceConflict (
  EFI_HANDLE                        HostBridgeHandle,
  VOID                              *Configuration
  )
{
  EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *Descriptor;
  UINTN                             RootBridgeIndex;
  DEBUG ((EFI_D_ERROR, "PciHostBridge: Resource conflict happens!\n"));

  RootBridgeIndex = 0;
  Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) Configuration;
  while (Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR) {
    DEBUG ((EFI_D_ERROR, "RootBridge[%d]:\n", RootBridgeIndex++));
    for (; Descriptor->Desc == ACPI_ADDRESS_SPACE_DESCRIPTOR; Descriptor++) {
      ASSERT (Descriptor->ResType <
              sizeof (mPciHostBridgeLibAcpiAddressSpaceTypeStr) / sizeof (mPciHostBridgeLibAcpiAddressSpaceTypeStr[0])
              );
      DEBUG ((EFI_D_ERROR, " %s: Length/Alignment = 0x%lx / 0x%lx\n",
              mPciHostBridgeLibAcpiAddressSpaceTypeStr[Descriptor->ResType], Descriptor->AddrLen, Descriptor->AddrRangeMax));
      if (Descriptor->ResType == ACPI_ADDRESS_SPACE_TYPE_MEM) {
        DEBUG ((EFI_D_ERROR, "     Granularity/SpecificFlag = %ld / %02x%s\n",
                Descriptor->AddrSpaceGranularity, Descriptor->SpecificFlag,
                ((Descriptor->SpecificFlag & EFI_ACPI_MEMORY_RESOURCE_SPECIFIC_FLAG_CACHEABLE_PREFETCHABLE) != 0) ? L" (Prefetchable)" : L""
                ));
      }
    }
    //
    // Skip the END descriptor for root bridge
    //
    ASSERT (Descriptor->Desc == ACPI_END_TAG_DESCRIPTOR);
    Descriptor = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *) ((EFI_ACPI_END_TAG_DESCRIPTOR *) Descriptor + 1);
  }

  ASSERT (FALSE);
}
