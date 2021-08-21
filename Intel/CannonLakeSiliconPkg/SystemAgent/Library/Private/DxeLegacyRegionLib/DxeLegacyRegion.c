/** @file
  This code provides a private implementation of the Legacy Region protocol.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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
#include <Private/Library/LegacyRegion.h>

///
/// Module Global:
///   Since this driver will only ever produce one instance of the Private Data
///   protocol you are not required to dynamically allocate the PrivateData.
///
GLOBAL_REMOVE_IF_UNREFERENCED LEGACY_REGION_INSTANCE        mPrivateData;

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                         mRomData[16] = {
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00,
  0x00
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                         mLockData[16] = {
  0x01,
  0x10,
  0x01,
  0x10,
  0x01,
  0x10,
  0x01,
  0x10,
  0x01,
  0x10,
  0x01,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                         mUnlockData[16] = {
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x30,
  0x30,
  0x30,
  0x30
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                         mMaskData[16] = {
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x30,
  0x03,
  0x00,
  0x00,
  0x00,
  0x00
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8                         mReg[16] = {
  R_SA_PAM1,
  R_SA_PAM1,
  R_SA_PAM2,
  R_SA_PAM2,
  R_SA_PAM3,
  R_SA_PAM3,
  R_SA_PAM4,
  R_SA_PAM4,
  R_SA_PAM5,
  R_SA_PAM5,
  R_SA_PAM6,
  R_SA_PAM6,
  R_SA_PAM0,
  R_SA_PAM0,
  R_SA_PAM0,
  R_SA_PAM0
};

/**
  The Legacy Region library constructuor.

  The function does the necessary initialization work for this library
  instance.

  @param[in]  ImageHandle       The firmware allocated handle for the UEFI image.
  @param[in]  SystemTable       A pointer to the EFI system table.

  @retval EFI_SUCCESS - Legacy Region protocol installed
  @retval Other       - No protocol installed.

**/
EFI_STATUS
EFIAPI
LegacyRegionLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return LegacyRegionInstall (ImageHandle);
}

/**
  Modify PAM registers for region specified to MODE state.

  @param[in] This    - Pointer to EFI_LEGACY_REGION2_PROTOCOL instance.
  @param[in] Start   - Starting address of a memory region covered by PAM registers (C0000h - FFFFFh).
  @param[in] Length  - Memory region length.
  @param[in] Mode    - Action to perform on a PAM region: PAM_UNLOCK, LOCK or BOOTLOCK.
  @param[out] Granularity - Granularity of region in bytes.

  @retval EFI_SUCCESS - PAM region(s) state modified as requested.
  @retval EFI_INVALID_PARAMETER - parameter out of boundary
**/
EFI_STATUS
LegacyRegionManipulation (
  IN  EFI_LEGACY_REGION2_PROTOCOL * This,
  IN  UINT32                      Start,
  IN  UINT32                      Length,
  IN  UINT32                      Mode,
  OUT UINT32                      *Granularity
  )
{
  UINT8                           Index;
  UINT8                           Data;
  UINT8                           TempData;
  UINT32                          TempAddr;
  UINT32                          NewStartAddr;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *Pci;
  EFI_CPU_ARCH_PROTOCOL           *CpuArch;
  UINT64                          PciAddress;
  UINT64                          Attributes;
  EFI_STATUS                      Status;

  Pci                     = mPrivateData.PciRootBridgeIo;
  if ((Start  < 0xC0000) ||
      (Start  > 0xFFFFF) ||
      (Length > 0x40000) ||
      ((Start + Length - 1) > 0xFFFFF)) {
    return EFI_INVALID_PARAMETER;
  }
  TempAddr                = Start;
  Index                   = (UINT8) ((TempAddr - 0xC0000) / PAM_GRANULARITY);
  NewStartAddr            = TempAddr = (TempAddr / PAM_GRANULARITY) * PAM_GRANULARITY;
  while (TempAddr <= (Start + Length - 1)) {
    if (Index >= 16) {
      return EFI_INVALID_PARAMETER;
    }
    if ((Mode == PAM_LOCK) || (Mode == PAM_BOOTLOCK)) {
      Data = mLockData[Index];
    } else {
      if (Mode == PAM_UNLOCK) {
        Data = mUnlockData[Index];
      } else {
        Data = mRomData[Index];
      }
    }

    PciAddress = EFI_PCI_ADDRESS (0, 0, 0, mReg[Index]);
    Pci->Pci.Read (Pci, EfiPciWidthUint8, PciAddress, 1, &TempData);
    TempData = (UINT8) ((TempData & mMaskData[Index]) | Data);
    Pci->Pci.Write (Pci, EfiPciWidthUint8, PciAddress, 1, &TempData);
    TempAddr += PAM_GRANULARITY;
    Index++;
  }

  if (Granularity != NULL) {
    *Granularity = PAM_GRANULARITY;
  }
  ///
  ///  Program the MTRRs
  ///
  switch (Mode) {

    case PAM_UNLOCK:
      Attributes = EFI_MEMORY_WT;
      break;

    case PAM_LOCK:
      Attributes = EFI_MEMORY_WP;
      break;

    case PAM_BOOTLOCK:
      Attributes = EFI_MEMORY_WP;
      break;

    default:
      Attributes = EFI_MEMORY_UC;

  }

  Status = gBS->LocateProtocol (
                  &gEfiCpuArchProtocolGuid,
                  NULL,
                  (VOID **) &CpuArch
                  );
  ASSERT_EFI_ERROR (Status);
  Status = CpuArch->SetMemoryAttributes (
                      CpuArch,
                      NewStartAddr,
                      TempAddr - NewStartAddr,
                      Attributes
                      );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

/**
  Enable/Disable decoding of the given region

  @param[in] This    - Pointer to EFI_LEGACY_REGION2_PROTOCOL instance.
  @param[in] Start  - Starting address of region.
  @param[in] Length - Length of region in bytes.
  @param[in] On     - 0 = Disable decoding, 1 = Enable decoding.

  @retval EFI_SUCCESS - Decoding change affected.
**/
EFI_STATUS
EFIAPI
LegacyRegionDecode (
  IN  EFI_LEGACY_REGION2_PROTOCOL * This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity,
  IN  BOOLEAN                      *On
  )
{
  if (*On) {
    return LegacyRegionManipulation (This, Start, Length, PAM_UNLOCK, Granularity);
  } else {
    return LegacyRegionManipulation (This, Start, Length, LEGACY_REGION_DECODE_ROM, Granularity);
  }
}

/**
  Make the indicated region read from RAM / write to ROM.

  @param[in] This        - Pointer to EFI_LEGACY_REGION2_PROTOCOL instance.
  @param[in] Start       - Starting address of region.
  @param[in] Length      - Length of region in bytes.
  @param[out] Granularity - Granularity of region in bytes.

  @retval EFI_SUCCESS - Region locked or made R/O.
**/
EFI_STATUS
EFIAPI
LegacyRegionBootLock (
  IN  EFI_LEGACY_REGION2_PROTOCOL * This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity
  )
{
  return LegacyRegionManipulation (This, Start, Length, PAM_BOOTLOCK, Granularity);
}

/**
  Make the indicated region read from RAM / write to ROM.

  @param[in] This    - Pointer to EFI_LEGACY_REGION2_PROTOCOL instance.
  @param[in] Start       - Starting address of region.
  @param[in] Length      - Length of region in bytes.
  @param[out] Granularity - Granularity of region in bytes.

  @retval EFI_SUCCESS - Region locked or made R/O.
**/
EFI_STATUS
EFIAPI
LegacyRegionLock (
  IN  EFI_LEGACY_REGION2_PROTOCOL * This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity
  )
{
  return LegacyRegionManipulation (This, Start, Length, PAM_LOCK, Granularity);
}

/**
  Make the indicated region read from RAM / write to RAM.

  @param[in] This    - Pointer to EFI_LEGACY_REGION2_PROTOCOL instance.
  @param[in] Start       - Starting address of region.
  @param[in] Length      - Length of region in bytes.
  @param[out] Granularity - Granularity of region in bytes.

  @retval EFI_SUCCESS - Region unlocked or made R/W.
**/
EFI_STATUS
EFIAPI
LegacyRegionUnlock (
  IN  EFI_LEGACY_REGION2_PROTOCOL   * This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity
  )
{
  return LegacyRegionManipulation (This, Start, Length, PAM_UNLOCK, Granularity);
}

/**
  Get region information for the attributes of the Legacy Region.

  This function is used to discover the granularity of the attributes for the memory in the legacy
  region. Each attribute may have a different granularity and the granularity may not be the same
  for all memory ranges in the legacy region.

  @param  This[in]              Indicates the EFI_LEGACY_REGION2_PROTOCOL instance.
  @param  DescriptorCount[out]  The number of region descriptor entries returned in the Descriptor
                                buffer.
  @param  Descriptor[out]       A pointer to a pointer used to return a buffer where the legacy
                                region information is deposited. This buffer will contain a list of
                                DescriptorCount number of region descriptors.  This function will
                                provide the memory for the buffer.

  @retval EFI_SUCCESS           The region's attributes were successfully modified.
  @retval EFI_INVALID_PARAMETER If Start or Length describe an address not in the Legacy Region.

**/

EFI_STATUS
EFIAPI
LegacyRegionGetInfo (
  IN  EFI_LEGACY_REGION2_PROTOCOL   *This,
  OUT UINT32                        *DescriptorCount,
  OUT EFI_LEGACY_REGION_DESCRIPTOR  **Descriptor
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Install Driver to produce Legacy Region protocol.

  @param[in] ImageHandle             Handle for the image of this driver

  @retval EFI_SUCCESS - Legacy Region protocol installed
  @retval Other       - No protocol installed, unload driver.
**/
EFI_STATUS
LegacyRegionInstall (
  IN EFI_HANDLE           ImageHandle
  )
{
  LEGACY_REGION_INSTANCE  *Private;
  Private = &mPrivateData;

  DEBUG ((DEBUG_INFO, "Initializing Legacy Region\n"));
  ///
  /// Grab a copy of all the protocols we depend on. Any error would
  /// be a dispatcher bug!.
  ///
  gBS->LocateProtocol (
         &gEfiPciRootBridgeIoProtocolGuid,
         NULL,
         (VOID **) &Private->PciRootBridgeIo
         );
  Private->Signature              = LEGACY_REGION_INSTANCE_SIGNATURE;
  Private->LegacyRegion.Decode    = LegacyRegionDecode;
  Private->LegacyRegion.Lock      = LegacyRegionLock;
  Private->LegacyRegion.BootLock  = LegacyRegionBootLock;
  Private->LegacyRegion.UnLock    = LegacyRegionUnlock;
  Private->LegacyRegion.GetInfo   = LegacyRegionGetInfo;
  Private->ImageHandle            = ImageHandle;

  ///
  /// Make a new handle and install the protocol
  ///
  Private->Handle = NULL;
  return gBS->InstallProtocolInterface (
                &Private->Handle,
                &gEfiLegacyRegion2ProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &Private->LegacyRegion
                );
}
