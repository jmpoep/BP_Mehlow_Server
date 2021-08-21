/** @file
  This library will determine memory configuration information from the chipset
  and memory and create SMBIOS memory structures appropriately.

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
  SMBIOS Specification version 2.8.0 from DMTF: http://www.dmtf.org/standards/smbios
   - SMBUS Host Controller Protocol Specification
  Unified Extensible Firmware Interface (UEFI) Specifications: http://www.uefi.org/specs/
**/
#include "SmbiosMemory.h"

//
// Module-wide global variables
//
GLOBAL_REMOVE_IF_UNREFERENCED SA_POLICY_PROTOCOL              *mSaPolicy = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED MEMORY_INFO_DATA_HOB            *mMemInfo = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMBIOS_HANDLE               mSmbiosType16Handle = 0;
GLOBAL_REMOVE_IF_UNREFERENCED MEMORY_DXE_CONFIG               *mMemoryDxeConfig = NULL;

/**
  Initialize the module's protocols

  @retval EFI_SUCCESS          - if the protocols are successfully found.
  @retval EFI_NOT_FOUND        - if a protocol could not be located.
**/
EFI_STATUS
InitializeProtocols (
  VOID
  )
{
  EFI_STATUS Status;

  Status = gBS->LocateProtocol (&gSaPolicyProtocolGuid, NULL, (VOID **) &mSaPolicy);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetConfigBlock ((VOID *)mSaPolicy, &gMemoryDxeConfigGuid, (VOID *)&mMemoryDxeConfig);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

/**
  This function will determine memory configuration information from the chipset
  and memory and install SMBIOS table types 16, 17, and 19.
  Memory Device Mapped Address (Type 20) is optional and it will not be generated by RC.

  @param[in] SmbiosProtocol     - Instance of Smbios Protocol

  @retval EFI_SUCCESS           - if the data is successfully reported.
  @retval EFI_OUT_OF_RESOURCES  - if not able to get resources.
  @retval EFI_INVALID_PARAMETER - if a required parameter in a subfunction is NULL.
**/
EFI_STATUS
EFIAPI
SmbiosMemory (
  IN  EFI_SMBIOS_PROTOCOL *SmbiosProtocol
  )
{
  EFI_STATUS         Status;
  SA_POLICY_PROTOCOL *SaPolicy;
  EFI_HOB_GUID_TYPE  *GuidHob;

  Status = EFI_SUCCESS;
  //
  // Retrieve MEMORY_INFO_DATA_HOB from HOB
  //
  GuidHob = NULL;
  GuidHob = GetFirstGuidHob (&gSiMemoryInfoDataGuid);
  ASSERT (GuidHob != NULL);
  if (GuidHob == NULL) {
    return EFI_NOT_FOUND;
  }
  mMemInfo = (MEMORY_INFO_DATA_HOB *) GET_GUID_HOB_DATA (GuidHob);

  //
  // If gSaPolicyProtocolGuid already installed, invoke the callback directly.
  //
  Status = gBS->LocateProtocol (&gSaPolicyProtocolGuid, NULL, (VOID **) &SaPolicy);
  if (!EFI_ERROR (Status)) {
    Status = InitializeProtocols ();
    if (EFI_ERROR (Status)) {
      ASSERT_EFI_ERROR (Status);
    }

    Status = InstallSmbiosType16 (SmbiosProtocol);
    ASSERT_EFI_ERROR (Status);

    Status = InstallSmbiosType17 (SmbiosProtocol);
    ASSERT_EFI_ERROR (Status);

    Status = InstallSmbiosType19 (SmbiosProtocol);
    ASSERT_EFI_ERROR (Status);
  } else {
    ASSERT_EFI_ERROR (Status);
  }
  return Status;
}

/**
  Add an SMBIOS table entry using EFI_SMBIOS_PROTOCOL.
  Create the full table record using the formatted section plus each non-null string, plus the terminating (double) null.

  @param[in]  Entry                 The data for the fixed portion of the SMBIOS entry.
                                    The format of the data is determined by EFI_SMBIOS_TABLE_HEADER.
                                    Type. The size of the formatted area is defined by
                                    EFI_SMBIOS_TABLE_HEADER. Length and either followed by a
                                    double-null (0x0000) or a set of null terminated strings and a null.
  @param[in]  TableStrings          Set of string pointers to append onto the full record.
                                    If TableStrings is null, no strings are appended. Null strings
                                    are skipped.
  @param[in]  NumberOfStrings       Number of TableStrings to append, null strings are skipped.
  @param[in]  SmbiosProtocol        Instance of Smbios Protocol
  @param[out] SmbiosHandle          A unique handle will be assigned to the SMBIOS record.

  @retval     EFI_SUCCESS           Table was added.
  @retval     EFI_OUT_OF_RESOURCES  Table was not added due to lack of system resources.
**/
EFI_STATUS
AddSmbiosEntry (
  IN  EFI_SMBIOS_TABLE_HEADER *Entry,
  IN  CHAR8                   **TableStrings,
  IN  UINT8                   NumberOfStrings,
  IN  EFI_SMBIOS_PROTOCOL     *SmbiosProtocol,
  OUT EFI_SMBIOS_HANDLE       *SmbiosHandle
  )
{
  EFI_STATUS              Status;
  EFI_SMBIOS_TABLE_HEADER *Record;
  CHAR8                   *StringPtr;
  UINTN                   Size;
  UINTN                   i;
  EFI_SMBIOS_PROTOCOL     *Smbios;

  Smbios = SmbiosProtocol;
  ///
  /// Calculate the total size of the full record
  ///
  Size = Entry->Length;

  ///
  /// Add the size of each non-null string
  ///
  if (TableStrings != NULL) {
    for (i = 0; i < NumberOfStrings; i++) {
      if (TableStrings[i] != NULL) {
        Size += AsciiStrSize (TableStrings[i]);
      }
    }
  }

  ///
  /// Add the size of the terminating double null
  /// If there were any strings added, just add the second null
  ///
  if (Size == Entry->Length) {
    Size += 2;
  } else {
    Size += 1;
  }

  ///
  /// Initialize the full record
  ///
  Record = (EFI_SMBIOS_TABLE_HEADER *) AllocateZeroPool (Size);
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, Entry, Entry->Length);

  ///
  /// Copy the strings to the end of the record
  ///
  StringPtr = ((CHAR8 *) Record) + Entry->Length;
  if (TableStrings != NULL) {
    for (i = 0; i < NumberOfStrings; i++) {
      if (TableStrings[i] != NULL) {
        AsciiStrCpyS (StringPtr, Size - Entry->Length, TableStrings[i]);
        StringPtr += AsciiStrSize (TableStrings[i]);
        Size -= AsciiStrSize (TableStrings[i]);
      }
    }
  }

  *SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->Add (Smbios, NULL, SmbiosHandle, Record);

  FreePool (Record);
  return Status;
}
