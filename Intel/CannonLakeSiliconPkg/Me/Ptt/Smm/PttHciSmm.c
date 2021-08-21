/** @file
  It updates TPM2 items in ACPI table and registers SMI2 callback
  functions for TrEE physical presence, ClearMemory, and sample
  for dTPM StartMethod.

  Caution: This module requires additional review when modified.
  This driver will have external input - variable and ACPINvs data in SMM mode.
  This external input must be validated carefully to avoid security issue.

  PhysicalPresenceCallback() and MemoryClearCallback() will receive untrusted input
  and do some check.

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

#include "PttHciSmm.h"
#include <MeChipset.h>
#include <HeciRegs.h>
#include <Protocol/MePolicy.h>
#include <Library/ConfigBlockLib.h>

extern EFI_GUID gMeSsdtAcpiTableStorageGuid;
extern EFI_GUID gTpm2AcpiTableStorageGuid;
extern EFI_GUID gEfiTrEEPhysicalPresenceGuid;

GLOBAL_REMOVE_IF_UNREFERENCED EFI_TPM2_ACPI_TABLE  mTpm2AcpiTemplate = {
  {
    EFI_ACPI_5_0_TRUSTED_COMPUTING_PLATFORM_2_TABLE_SIGNATURE,
    sizeof (mTpm2AcpiTemplate),
    EFI_TPM2_ACPI_TABLE_REVISION,
    //
    // Compiler initializes the remaining bytes to 0
    // These fields should be filled in in production
    //
  },
  0, // Flags
  (EFI_PHYSICAL_ADDRESS)(UINTN)0xFFFFFFFF, // Control Area
   EFI_TPM2_ACPI_TABLE_START_METHOD_COMMAND_RESPONSE_BUFFER_INTERFACE,
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_SMM_VARIABLE_PROTOCOL    *mSmmVariable;
GLOBAL_REMOVE_IF_UNREFERENCED TCG_NVS                      *mTcgNvs;
GLOBAL_REMOVE_IF_UNREFERENCED EFI_TPM2_ACPI_CONTROL_AREA   mControlArea; // Smm copy, because we need cache Command & Response address
GLOBAL_REMOVE_IF_UNREFERENCED ME_NVS_AREA_PROTOCOL         mMeNvsAreaProtocol;

/**
  Software SMI callback for TPM physical presence which is called from ACPI method.

  Caution: This function may receive untrusted input.
  Variable and ACPINvs are external input, so this function will validate
  its data structure to be valid value.

  @param[in]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.
  @param[in, out] CommBuffer      A pointer to a collection of data in memory that will
                                  be conveyed from a non-SMM environment into an SMM environment.
  @param[in, out] CommBufferSize  The size of the CommBuffer.

  @retval EFI_SUCCESS             The interrupt was handled successfully.

**/
EFI_STATUS
EFIAPI
PhysicalPresenceCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN CONST VOID                  *Context,
  IN OUT VOID                    *CommBuffer,
  IN OUT UINTN                   *CommBufferSize
  )
{
  UINT32                MostRecentRequest;
  UINT32                Response;

  if (mTcgNvs->PhysicalPresenceParameter == TCG_ACPI_FUNCTION_RETURN_REQUEST_RESPONSE_TO_OS) {
    mTcgNvs->PhysicalPresenceReturnCode = Tcg2PhysicalPresenceLibReturnOperationResponseToOsFunction (
                                             &MostRecentRequest,
                                             &Response
                                             );
    mTcgNvs->PhysicalPresenceLastRequest = MostRecentRequest;
    mTcgNvs->PhysicalPresenceResponse = Response;
    return EFI_SUCCESS;
  } else if ((mTcgNvs->PhysicalPresenceParameter == TCG_ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS)
           || (mTcgNvs->PhysicalPresenceParameter == TCG_ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS_2)) {
    mTcgNvs->PhysicalPresenceReturnCode = Tcg2PhysicalPresenceLibSubmitRequestToPreOSFunction (
                                             mTcgNvs->PhysicalPresenceRequest,
                                             mTcgNvs->PhysicalPresenceRequestParameter
                                             );
  } else if (mTcgNvs->PhysicalPresenceParameter == TCG_ACPI_FUNCTION_GET_USER_CONFIRMATION_STATUS_FOR_REQUEST) {
    mTcgNvs->PhysicalPresenceReturnCode = Tcg2PhysicalPresenceLibGetUserConfirmationStatusFunction (mTcgNvs->PhysicalPresenceRequest);
  }

  return EFI_SUCCESS;
}


/**
  Software SMI callback for MemoryClear which is called from ACPI method.

  Caution: This function may receive untrusted input.
  Variable and ACPINvs are external input, so this function will validate
  its data structure to be valid value.

  @param[in]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.

  @retval EFI_SUCCESS             The interrupt was handled successfully.
**/
EFI_STATUS
EFIAPI
MemoryClearCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext
  )
{
  EFI_STATUS                     Status;
  UINTN                          DataSize;
  UINT8                          MorControl;
  UINT32                         VarAttributes;

  VarAttributes = 0;
  DataSize = sizeof (UINT8);
  Status = mSmmVariable->SmmGetVariable (
                           MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                           &gEfiMemoryOverwriteControlDataGuid,
                           &VarAttributes,
                           &DataSize,
                           &MorControl
                           );
  if (EFI_ERROR (Status) && (VarAttributes == 0)) {
    //
    // If the variable is not found, create one with current MorControl value.
    //
    VarAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
  }
  mTcgNvs->MemoryClearReturnCode = MOR_REQUEST_SUCCESS;
  if (mTcgNvs->MemoryClearParameter == ACPI_FUNCTION_DSM_MEMORY_CLEAR_INTERFACE) {
    MorControl = (UINT8) mTcgNvs->MemoryClearRequest;
  } else if (mTcgNvs->MemoryClearParameter == ACPI_FUNCTION_PTS_CLEAR_MOR_BIT) {
    if (EFI_ERROR (Status)) {
      mTcgNvs->MemoryClearReturnCode = MOR_REQUEST_GENERAL_FAILURE;
      DEBUG ((DEBUG_WARN, "[TPM] Get MOR variable failure! Status = %r\n", Status));
      return EFI_SUCCESS;
    }

    if (MOR_CLEAR_MEMORY_VALUE (MorControl) == 0x0) {
      return EFI_SUCCESS;
    }
    MorControl &= ~MOR_CLEAR_MEMORY_BIT_MASK;
  }

  DataSize = sizeof (UINT8);
  Status = mSmmVariable->SmmSetVariable (
                           MEMORY_OVERWRITE_REQUEST_VARIABLE_NAME,
                           &gEfiMemoryOverwriteControlDataGuid,
                           VarAttributes,
                           DataSize,
                           &MorControl
                           );
  if (EFI_ERROR (Status)) {
    mTcgNvs->MemoryClearReturnCode = MOR_REQUEST_GENERAL_FAILURE;
    DEBUG ((DEBUG_WARN, "[TPM] Set MOR variable failure! Status = %r\n", Status));
  }

  return EFI_SUCCESS;
}

/**
  Find the operation region in TCG ACPI table by given Name and Size,
  and initialize it if the region is found.

  @param[in, out] Table          The TPM item in ACPI table.
  @param[in]      Name           The name string to find in TPM table.
  @param[in]      Size           The size of the region to find.

  @return                        The allocated address for the found region.

**/
VOID *
AssignOpRegion (
  EFI_ACPI_DESCRIPTION_HEADER    *Table,
  UINT32                         Name,
  UINT16                         Size
  )
{
  EFI_STATUS                     Status;
  AML_OP_REGION_32_8             *OpRegion;
  EFI_PHYSICAL_ADDRESS           MemoryAddress;

  MemoryAddress = 0xFFFFFFFF;

  //
  // Patch some pointers for the ASL code before loading the SSDT.
  //
  for (OpRegion  = (AML_OP_REGION_32_8 *) (Table + 1);
       OpRegion <= (AML_OP_REGION_32_8 *) ((UINT8 *) Table + Table->Length);
       OpRegion  = (AML_OP_REGION_32_8 *) ((UINT8 *) OpRegion + 1)) {
    if ((OpRegion->OpRegionOp  == AML_EXT_REGION_OP) &&
        (OpRegion->NameString  == Name) &&
        (OpRegion->DWordPrefix == AML_DWORD_PREFIX) &&
        (OpRegion->BytePrefix  == AML_BYTE_PREFIX)) {

      Status = gBS->AllocatePages (AllocateMaxAddress, EfiACPIMemoryNVS, EFI_SIZE_TO_PAGES (Size), &MemoryAddress);
      ASSERT_EFI_ERROR (Status);
      ZeroMem ((VOID *) (UINTN) MemoryAddress, Size);
      OpRegion->RegionOffset = (UINT32) (UINTN) MemoryAddress;
      OpRegion->RegionLen    = (UINT8) Size;
      break;
    }
  }

  return (VOID *) (UINTN) MemoryAddress;
}

VOID *
GetTpm2AcpiTableFromFv (
  VOID
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                           NumberOfHandles;
  EFI_FV_FILETYPE                 FileType;
  UINT32                          FvStatus;
  EFI_FV_FILE_ATTRIBUTES          Attributes;
  UINTN                           Size;
  UINTN                           Index;
  INTN                            Instance;
  EFI_ACPI_COMMON_HEADER          *CurrentTable;
  EFI_FIRMWARE_VOLUME2_PROTOCOL   *FwVol;

  FwVol = NULL;

  ///
  /// Locate protocol.
  /// There is little chance we can't find an FV protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  FwVol = NULL;
  ///
  /// Looking for FV with ACPI storage file
  ///
  for (Index = 0; Index < NumberOfHandles; Index++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (void **)&FwVol
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// See if it has the ACPI storage file
    ///
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &gTpm2AcpiTableStorageGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (Status == EFI_SUCCESS) {
      break;
    } else {
      FwVol = NULL;
    }
  }

  if ((Index == NumberOfHandles) || (FwVol == NULL)) {
    return NULL;
  }

  Instance      = 0;
  CurrentTable  = NULL;

  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      &gTpm2AcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (void **)&CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      ///
      /// Check the Signature ID to modify the table
      ///
      if ((((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->Signature == SIGNATURE_32 ('S', 'S', 'D', 'T')) &&
          (((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->OemTableId == SIGNATURE_64 ('T', 'p', 'm', '2', 'T', 'a', 'b', 'l'))) {
        //
        // Find it.
        //
        break;
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      CurrentTable = NULL;
    }
  }

  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  ///
  ///
  /// Free any allocated buffers
  ///
  FreePool (HandleBuffer);

  return CurrentTable;
}

/**
  Patch version string of Physical Presence interface supported by platform. The initial string tag in TPM
  ACPI table is "$PV".

  @param[in, out] Table          The TPM item in ACPI table.
  @param[in]      PPVer          Version string of Physical Presence interface supported by platform.

  @retval   EFI_SUCCESS          The Physical Presence Version updates successfully.
  @retval   Others               The Physical Presence Version not found.
**/
EFI_STATUS
UpdatePPVersion (
  EFI_ACPI_DESCRIPTION_HEADER    *Table,
  CHAR8                          *PPVer
  )
{
  EFI_STATUS  Status;
  UINT8       *DataPtr;

  //
  // Patch some pointers for the ASL code before loading the SSDT.
  //
  for (DataPtr  = (UINT8 *)(Table + 1);
       DataPtr <= (UINT8 *)((UINT8 *) Table + Table->Length - PHYSICAL_PRESENCE_VERSION_SIZE);
       DataPtr += 1) {
    if (AsciiStrCmp ((CHAR8 *)DataPtr,  PHYSICAL_PRESENCE_VERSION_TAG) == 0) {
      Status = AsciiStrCpyS ((CHAR8 *)DataPtr, PHYSICAL_PRESENCE_VERSION_SIZE, PPVer);
      DEBUG ((DEBUG_INFO, "TPM2 Physical Presence Interface Version update status 0x%x\n", Status));
      return Status;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Initialize and publish TPM items in ACPI table.

  @retval   EFI_SUCCESS     The TCG ACPI table is published successfully.
  @retval   Others          The TCG ACPI table is not published.
**/
EFI_STATUS
PublishAcpiTable (
  VOID
  )
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;
  EFI_ACPI_DESCRIPTION_HEADER    *Table;
  UINTN                          TableSize;

  Table = GetTpm2AcpiTableFromFv ();
  ASSERT (Table != NULL);

  if (Table == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Update Table version before measuring it to PCR
  //
  Status = UpdatePPVersion (Table, (CHAR8 *)PcdGetPtr(PcdTcgPhysicalPresenceInterfaceVer));
  ASSERT_EFI_ERROR (Status);

  TableSize = Table->Length;

  ASSERT (Table->OemTableId == SIGNATURE_64 ('T', 'p', 'm', '2', 'T', 'a', 'b', 'l'));
  mTcgNvs = AssignOpRegion (Table, SIGNATURE_32 ('T', 'N', 'V', 'S'), (UINT16) sizeof (TCG_NVS));
  ASSERT (mTcgNvs != NULL);

  //
  // Publish the TPM ACPI table
  //
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);

  TableKey = 0;
  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        Table,
                        TableSize,
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

EFI_STATUS
PublishPttAcpiTable (
  VOID
  )
/**
  Publish TPM2 ACPI table

  @retval   EFI_SUCCESS     The TPM2 ACPI table is published successfully.
  @retval   Others          The TPM2 ACPI table is not published.
**/
{
  EFI_STATUS                     Status;
  EFI_ACPI_TABLE_PROTOCOL        *AcpiTable;
  UINTN                          TableKey;
  EFI_TPM2_ACPI_CONTROL_AREA     *ControlArea;
  ME_POLICY_PROTOCOL             *DxeMePolicy;
  ME_DXE_CONFIG                  *MeDxeConfig;

  DxeMePolicy = NULL;
  Status = gBS->LocateProtocol (&gDxeMePolicyGuid, NULL, (VOID **) &DxeMePolicy);
  if (EFI_ERROR (Status) || (DxeMePolicy == NULL)) {
    DEBUG ((DEBUG_ERROR, "PublishPttAcpiTable, No ME Policy Protocol available"));
    return EFI_NOT_READY;
  }

  Status = GetConfigBlock ((VOID *) DxeMePolicy, &gMeDxeConfigGuid, (VOID *) &MeDxeConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Construct ACPI table
  ///
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);
  ASSERT_EFI_ERROR (Status);

  mTpm2AcpiTemplate.AddressOfControlArea = 0xFED40040;

  ControlArea = (EFI_TPM2_ACPI_CONTROL_AREA *) (UINTN) mTpm2AcpiTemplate.AddressOfControlArea;
  ZeroMem (ControlArea, sizeof (*ControlArea));
  ControlArea->CommandSize  = 0xF80;
  ControlArea->ResponseSize = 0xF80;

  ControlArea->Command      = (UINTN) mTpm2AcpiTemplate.AddressOfControlArea + 0x40;
  ControlArea->Response     = (UINTN) mTpm2AcpiTemplate.AddressOfControlArea + 0x40;

  CopyMem (&mControlArea, ControlArea, sizeof (mControlArea));

  DEBUG ((DEBUG_INFO, "Ptt Windows Buffer Control Area Address = %x\n", mTpm2AcpiTemplate.AddressOfControlArea));
  DEBUG ((DEBUG_INFO, "Ptt Windows Command/Response Buffer Address = %x\n", ControlArea->Command));

  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        &mTpm2AcpiTemplate,
                        sizeof (mTpm2AcpiTemplate),
                        &TableKey
                        );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Initialize MEFW SSDT ACPI tables

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.

  @retval EFI_SUCCESS    ACPI tables are initialized successfully
  @retval EFI_NOT_FOUND  ACPI tables not found
**/
EFI_STATUS
InitializeMeSsdtAcpiTables (
  IN EFI_HANDLE                  ImageHandle
  )
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  EFI_FV_FILETYPE               FileType;
  UINT32                        FvStatus;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINTN                         Size;
  UINTN                         i;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol;
  INTN                          Instance;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  UINTN                         AcpiTableKey;
  UINT8                         *CurrPtr;
  UINT8                         *EndPtr;
  UINT32                        *Signature;
  EFI_ACPI_DESCRIPTION_HEADER   *MeAcpiTable;
  EFI_ACPI_TABLE_PROTOCOL       *AcpiTable;

  Status = gBS->AllocatePool (EfiACPIMemoryNVS, sizeof (ME_NVS_AREA), (VOID **) &mMeNvsAreaProtocol.Area);
  ASSERT_EFI_ERROR (Status);
  ZeroMem ((VOID *) mMeNvsAreaProtocol.Area, sizeof (ME_NVS_AREA));

  FwVol       = NULL;
  MeAcpiTable = NULL;

  ///
  /// Locate ACPI Table protocol
  ///
  DEBUG ((DEBUG_INFO, "Init ME SSDT table\n"));
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (void **)&AcpiTable);
  if (Status != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Fail to locate EfiAcpiTableProtocol.\n"));
    return EFI_NOT_FOUND;
  }

  ///
  /// Locate protocol.
  /// There is little chance we can't find an FV protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareVolume2ProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  ///
  /// Looking for FV with ACPI storage file
  ///
  for (i = 0; i < NumberOfHandles; i++) {
    ///
    /// Get the protocol on this handle
    /// This should not fail because of LocateHandleBuffer
    ///
    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
                    &gEfiFirmwareVolume2ProtocolGuid,
                    (void **)&FwVol
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// See if it has the ACPI storage file
    ///
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &gMeSsdtAcpiTableStorageGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    ///
    /// If we found it, then we are done
    ///
    if (Status == EFI_SUCCESS) {
      break;
    }
  }
  ///
  /// Free any allocated buffers
  ///
  FreePool (HandleBuffer);

  ///
  /// Sanity check that we found our data file
  ///
  ASSERT (FwVol != NULL);
  if (FwVol == NULL) {
    DEBUG ((DEBUG_WARN, "ME Global NVS table not found\n"));
    return EFI_NOT_FOUND;
  }
  ///
  /// Our exit status is determined by the success of the previous operations
  /// If the protocol was found, Instance already points to it.
  /// Read tables from the storage file.
  ///
  Instance      = 0;
  CurrentTable  = NULL;
  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      &gMeSsdtAcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (void **)&CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      ///
      /// Check the table ID to modify the table
      ///
      if (((EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable)->OemTableId == SIGNATURE_64 ('M', 'e', 'S', 's', 'd', 't', ' ', 0)) {
        MeAcpiTable = (EFI_ACPI_DESCRIPTION_HEADER *) CurrentTable;
        ///
        /// Locate the SSDT package
        ///
        CurrPtr = (UINT8 *) MeAcpiTable;
        EndPtr  = CurrPtr + MeAcpiTable->Length;

        for (; CurrPtr <= EndPtr; CurrPtr++) {
          Signature = (UINT32 *) (CurrPtr + 3);
          if (*Signature == SIGNATURE_32 ('M', 'E', 'N', 'V')) {
            ASSERT (*(UINT32 *) (CurrPtr + 3 + sizeof (*Signature) + 2) == 0xFFFF0000);
            ASSERT (*(UINT16 *) (CurrPtr + 3 + sizeof (*Signature) + 2 + sizeof (UINT32) + 1) == 0xAA55);
            ///
            /// ME Global NVS Area address
            ///

            *(UINT32 *) (CurrPtr + 3 + sizeof (*Signature) + 2) = (UINT32) (UINTN) mMeNvsAreaProtocol.Area;
            ///
            /// ME Global NVS Area size
            ///
            *(UINT16 *) (CurrPtr + 3 + sizeof (*Signature) + 2 + sizeof (UINT32) + 1) =
              sizeof (ME_NVS_AREA);

            AcpiTableKey = 0;
            Status = AcpiTable->InstallAcpiTable (
                                  AcpiTable,
                                  MeAcpiTable,
                                  MeAcpiTable->Length,
                                  &AcpiTableKey
                                  );
            ASSERT_EFI_ERROR (Status);

            Status = gBS->InstallMultipleProtocolInterfaces (
                            &ImageHandle,
                            &gMeNvsAreaProtocolGuid,
                            &mMeNvsAreaProtocol,
                            NULL
                            );
            ASSERT_EFI_ERROR (Status);
            return EFI_SUCCESS;
          }
        }
      }
      ///
      /// Increment the instance
      ///
      Instance++;
      CurrentTable = NULL;
    }
  }

  return Status;
}
/**
  The driver's entry point.

  It install callbacks for TPM physical presence and MemoryClear, and locate
  SMM variable to be used in the callback function.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.
  @param[in] SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval Others          Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
InitializePttSmm (
  IN EFI_HANDLE                  ImageHandle,
  IN EFI_SYSTEM_TABLE            *SystemTable
  )
{
  EFI_STATUS                     Status;
  EFI_SMM_SW_DISPATCH2_PROTOCOL  *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT    SwContext;
  EFI_HANDLE                     SwHandle;
  ME_NVS_AREA_PROTOCOL           *MeNvsAreaProtocol;
  ME_NVS_AREA                    *MeNvsArea;
  UINT32                         TpmStsFtif;
  ME_POLICY_PROTOCOL             *DxeMePolicy;
  ME_DXE_CONFIG                  *MeDxeConfig;

  TpmStsFtif     = MmioRead32 (R_PTT_TXT_STS_FTIF);

  if ((TpmStsFtif & V_FTIF_FTPM_PRESENT) != V_FTIF_FTPM_PRESENT) {
    DEBUG ((DEBUG_WARN, "InitializePttSmm - PTT not enabled\n"));
    return EFI_SUCCESS;
  }

  DxeMePolicy = NULL;
  Status = gBS->LocateProtocol (&gDxeMePolicyGuid, NULL, (VOID **) &DxeMePolicy);
  if (EFI_ERROR (Status) || (DxeMePolicy == NULL)) {
    DEBUG ((DEBUG_ERROR, "No ME Policy Protocol available"));
    return Status;
  }

  Status = GetConfigBlock ((VOID *) DxeMePolicy, &gMeDxeConfigGuid, (VOID *) &MeDxeConfig);
  ASSERT_EFI_ERROR (Status);


  Status = PublishAcpiTable ();
  ASSERT_EFI_ERROR (Status);

  //
  // Get the Sw dispatch protocol and register SMI callback functions.
  //
  Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID**)&SwDispatch);
  ASSERT_EFI_ERROR (Status);
  SwContext.SwSmiInputValue = EFI_TPM2_PP_SW_SMI;
  Status = SwDispatch->Register (SwDispatch, (EFI_SMM_HANDLER_ENTRY_POINT2) PhysicalPresenceCallback, &SwContext, &SwHandle);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mTcgNvs->PhysicalPresenceSoftwareSmi = (UINT8) SwContext.SwSmiInputValue;

  SwContext.SwSmiInputValue = EFI_TPM2_MOR_SW_SMI;
  Status = SwDispatch->Register (SwDispatch, (EFI_SMM_HANDLER_ENTRY_POINT2) MemoryClearCallback, &SwContext, &SwHandle);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mTcgNvs->MemoryClearSoftwareSmi = (UINT8) SwContext.SwSmiInputValue;

  ///
  /// Locate SmmVariableProtocol.
  ///
  Status = gSmst->SmmLocateProtocol (&gEfiSmmVariableProtocolGuid, NULL, (VOID**) &mSmmVariable);
  ASSERT_EFI_ERROR (Status);

  Status = InitializeMeSsdtAcpiTables (ImageHandle);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Publish TPM2 ACPI table
  ///
  Status = PublishPttAcpiTable ();
  ASSERT_EFI_ERROR (Status);

  ///
  /// Locate Global NVS and update PTT Buffer Address
  ///
  Status = gBS->LocateProtocol (&gMeNvsAreaProtocolGuid, NULL, (VOID **) &MeNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);

  MeNvsArea             = MeNvsAreaProtocol->Area;
  MeNvsArea->PTTAddress = mTpm2AcpiTemplate.AddressOfControlArea;

  return EFI_SUCCESS;
}

