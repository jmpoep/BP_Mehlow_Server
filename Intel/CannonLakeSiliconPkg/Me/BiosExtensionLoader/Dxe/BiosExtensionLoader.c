/** @file
  Loads the AMT Bios Extensions and calls the module prior to boot.
  Setup options control whether the user is allowed to change the provisioning of AMT
  or not for boot speed optimization.

  Configuration and invocation of the AMT Bios Extensions is done as per
  the AMT Bios Writers Guide.

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

#include "BiosExtensionLoader.h"

#define EVENT_REC_TOK                   "EventRec"

extern AMT_DXE_CONFIG                                         *mAmtDxeConfig;

GLOBAL_REMOVE_IF_UNREFERENCED AMT_READY_TO_BOOT_PROTOCOL      mAmtReadyToBoot = {
  AMT_READY_TO_BOOT_PROTOCOL_REVISION,
  AmtReadyToBoot
};
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                           mFwImageType;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                           mFwPlatformBrand;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                         mFwMediaTableReqAvail = FALSE;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                         mFwMediaTablePush     = FALSE;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                         mMebxLaunched         = FALSE;

/**
  Signal an event for Amt ready to boot.

  @retval EFI_SUCCESS             Mebx launched or no controller
**/
EFI_STATUS
EFIAPI
AmtReadyToBoot (
  VOID
  )
{
  ///
  /// only launch MEBx once during POST
  ///
  if (mMebxLaunched) {
    return EFI_SUCCESS;
  }

  mMebxLaunched = TRUE;

  ///
  /// Launch MEBx, do not assert on error as this may be valid case
  ///
  MebxNotifyEvent ();

  ///
  /// In the end of Ready To Boot phase(before sending End Of Post message),
  /// when BIOS is about to pass control to OS load, BIOS should stop
  /// the watchdog to avoid false watchdog expiration
  ///
  AsfStopWatchDog ();
  ///
  /// Start ASF OS watchdog timer if the corresponding option policy is true
  /// with the non-zero value in the OS timeout setting
  ///
  AsfStartWatchDog (ASF_START_OS_WDT);
  ///
  /// End of watch dog setup option
  ///
  return EFI_SUCCESS;
}

/**
  Create the FRU table to send to AMT FW

  @param[in] AssetTableData       Buffer of all Asset tables to send to FW
  @param[in] AmtData              Structure holds BIOS pointers for Asset tables

  @retval EFI_SUCCESS             FRU table calculated
**/
EFI_STATUS
CalculateFruTable (
  IN  TABLE_PUSH_DATA  *AssetTableData,
  IN  AMT_DATA         *AmtData
  )
{
  PCI_DEV_INFO    PciDevInfoPtr;
  HWAI_FRU_TABLE  *FruHeader;
  HWAI_PCI_FRU    *PciFru;
  UINT32          TableAddress;
  UINT16          Index;

  Index = 0;
  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_FRU_DEVICE].Offset = 0;

  FruHeader = (HWAI_FRU_TABLE *) &AssetTableData->TableData[0];
  PciFru = (HWAI_PCI_FRU *) &FruHeader->Data[0];

  CopyMem (&PciDevInfoPtr, (VOID *) (UINTN) AmtData->PciDevAssertInfoPtr, sizeof (PCI_DEV_INFO));

  if (AmtData->SupportedTablesFlags & MEBX_STF_PCI_DEV_TABLE) {
    FruHeader->StructureCount = PciDevInfoPtr.PciDevCount;
  } else {
    FruHeader->StructureCount = 0;
  }

  TableAddress = (UINT32) AmtData->PciDevAssertInfoPtr + sizeof (PCI_DEV_INFO);

  if (FruHeader->StructureCount) {
    FruHeader->TableByteCount = FruHeader->StructureCount * sizeof (HWAI_PCI_FRU);
    for (Index = 0; Index < FruHeader->StructureCount; Index++) {
      PciFru->SmbiosType    = 0;
      PciFru->Length        = sizeof (HWAI_PCI_FRU);
      PciFru->SmbiosHandle  = 0;
      PciFru->FruType       = HWAI_FRU_TYPE_PCI;
      CopyMem (&PciFru->FruData, (VOID *) (UINTN) TableAddress, sizeof (HWAI_PCI_DATA));
      ++PciFru;
      TableAddress += sizeof (HWAI_PCI_DATA);
    }
  } else {
    FruHeader->TableByteCount = 0;
  }

  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_FRU_DEVICE].Length = FruHeader->TableByteCount +
    sizeof (FruHeader->StructureCount) + sizeof (FruHeader->TableByteCount);

  return EFI_SUCCESS;
}

/**
  Create the Media table to send to AMT FW

  @param[in] AssetTableData       Buffer of all Asset tables to send to FW
  @param[in] TableOffset          Offset into AssetTableData that Media table will be located
  @param[in] AmtDataPtr           Structure holds BIOS pointers for Asset tables

  @retval EFI_SUCCESS             Media table created
**/
EFI_STATUS
CalculateMediaTable (
  IN  TABLE_PUSH_DATA  *AssetTableData,
  IN  UINT16           TableOffset,
  IN  AMT_DATA         *AmtDataPtr
  )
{
  HWAI_MEDIA_TABLE  *MediaHeaderPtr;
  HWAI_MEDIA_ENTRY  *MediaEntryPtr;
  MEDIA_DEV_INFO    MediaDevStruct;
  UINT32            TableAddress;
  UINT16            Index;

  Index = 0;

  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE].Offset = TableOffset;

  MediaHeaderPtr = (HWAI_MEDIA_TABLE *) &AssetTableData->TableData[TableOffset];
  MediaEntryPtr = (HWAI_MEDIA_ENTRY *) &MediaHeaderPtr->Data[0];

  CopyMem (&MediaDevStruct, (VOID *) (UINTN) AmtDataPtr->MediaDevAssetInfoPtr, sizeof (MEDIA_DEV_INFO));
  TableAddress = (UINT32) AmtDataPtr->MediaDevAssetInfoPtr + sizeof (MEDIA_DEV_INFO);

  if (!(AmtDataPtr->SupportedTablesFlags & MEBX_STF_MEDIA_DEV_TABLE)) {
    MediaHeaderPtr->TableByteCount = 0;
    AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE].Length = 0;
    return EFI_SUCCESS;
  }

  MediaHeaderPtr->StructureCount = MediaDevStruct.MediaDevCount;
  MediaHeaderPtr->TableByteCount = (MediaHeaderPtr->StructureCount * sizeof (HWAI_MEDIA_ENTRY));
  for (Index = 0; Index < MediaHeaderPtr->StructureCount; Index++) {
    MediaEntryPtr->Length       = sizeof (HWAI_MEDIA_ENTRY);
    MediaEntryPtr->SmbiosHandle = 0;
    MediaEntryPtr->SmbiosType   = 0;

    CopyMem (&MediaEntryPtr->MediaData, (VOID *) (UINTN) TableAddress, sizeof (MEBX_FRU_MEDIA_DEVICES));
    ++MediaEntryPtr;
    TableAddress += sizeof (MEBX_FRU_MEDIA_DEVICES);
  }

  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE].Length = MediaHeaderPtr->TableByteCount +
    sizeof (MediaHeaderPtr->StructureCount) + sizeof (MediaHeaderPtr->TableByteCount);

  return EFI_SUCCESS;
}

/**
  Create the SMBIOS table to send to AMT FW

  @param[in] AssetTableData       Buffer of all Asset tables to send to FW
  @param[in] TableOffset          Offset into AssetTableData that the SMBIOS table will be located
  @param[in] PtrSmbiosTable       Pointer to BIOS SMBIOS tables

  @retval EFI_ABORTED             PtrSmbiosTable data is invalid.
  @retval EFI_SUCCESS             Smbios table created.
**/
EFI_STATUS
CalculateSmbiosTable (
  IN  TABLE_PUSH_DATA  *AssetTableData,
  IN  UINT16           TableOffset,
  IN  UINT32           PtrSmbiosTable
  )
{
  SMBIOS_ENTRY_POINT              SmbEntryStruct;
  SMBIOS_HEADER                   *SmbiosHeaderPtr;
  SMBIOS_TABLE_TYPE_ONE           *Type1Ptr;
  SMBIOS_TABLE_TYPE_THREE         *Type3Ptr;
  SMBIOS_TABLE_TYPE_FOUR          *Type4Ptr;
  SMBIOS_TABLE_TYPE_TWENTY_THREE  *Type23Ptr;
  UINT32                          TableAddress;
  UINT16                          OrignalTableOffset;
  UINT16                          StringCounter;
  UINT8                           DataByte;
  BOOLEAN                         EndOfTable;
  BOOLEAN                         EndOfEntry;
  BOOLEAN                         PreviousNull;
  BOOLEAN                         KeepEntry;

  OrignalTableOffset  = 0;
  StringCounter       = 1;
  DataByte            = 0;
  EndOfTable          = FALSE;
  EndOfEntry          = FALSE;
  PreviousNull        = FALSE;
  KeepEntry           = FALSE;

  OrignalTableOffset  = TableOffset;
  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_SMBIOS].Offset = OrignalTableOffset;

  CopyMem (&SmbEntryStruct, (VOID *) (UINTN) PtrSmbiosTable, sizeof (SMBIOS_ENTRY_POINT));

  if (SmbEntryStruct.Signature != SMB_SIG) {
    DEBUG ((DEBUG_WARN, "CalulateSmbiosTable Error:  SmbEntryStruct.Signature != SMB_SIG\n"));
    return EFI_ABORTED;
  }

  if (SmbEntryStruct.TableLength == 0) {
    DEBUG ((DEBUG_WARN, "CalulateSmbiosTable Error:  TableSize == 0\n"));
    return EFI_ABORTED;
  }

  if (SmbEntryStruct.TableLength > (MAX_ASSET_TABLE_ALLOCATED_SIZE - TableOffset)) {
    DEBUG ((DEBUG_WARN, "SMBIOS Tables Are Larger Than 0x%x\n", (MAX_ASSET_TABLE_ALLOCATED_SIZE - TableOffset)));
    return EFI_ABORTED;
  }

  TableAddress = SmbEntryStruct.TableAddress;

  while (!EndOfTable) {
    CopyMem (&AssetTableData->TableData[TableOffset], (VOID *) (UINTN) TableAddress, sizeof (SMBIOS_HEADER));
    SmbiosHeaderPtr = (SMBIOS_HEADER *) &AssetTableData->TableData[TableOffset];
    switch (SmbiosHeaderPtr->Type) {
      case 13:
      case 15:
      case 25:
      case 32:
        KeepEntry = FALSE;
        ///
        /// Not needed by AMT
        ///
        break;

      case 127:
        KeepEntry   = TRUE;
        EndOfTable  = TRUE;
        break;

      default:
        KeepEntry = TRUE;
    }

    if (KeepEntry) {
      TableOffset += sizeof (SMBIOS_HEADER);
      TableAddress += sizeof (SMBIOS_HEADER);
      CopyMem (
        &AssetTableData->TableData[TableOffset],
        (VOID *) (UINTN) TableAddress,
        (SmbiosHeaderPtr->Length - sizeof (SMBIOS_HEADER))
        );

      ///
      /// Make any need modifications to entrys with changing fields
      ///
      switch (SmbiosHeaderPtr->Type) {
        case 1:
          Type1Ptr          = (SMBIOS_TABLE_TYPE_ONE *) SmbiosHeaderPtr;
          Type1Ptr->WakeUp  = 0;
          break;

        case 3:
          Type3Ptr                    = (SMBIOS_TABLE_TYPE_THREE *) SmbiosHeaderPtr;
          Type3Ptr->BootState         = 0;
          Type3Ptr->PowerSupplyState  = 0;
          Type3Ptr->ThermalState      = 0;
          Type3Ptr->SecurityStatus    = 0;
          break;

        case 4:
          Type4Ptr                = (SMBIOS_TABLE_TYPE_FOUR *) SmbiosHeaderPtr;
          Type4Ptr->MaxSpeed      = Type4Ptr->MaxSpeed - (Type4Ptr->MaxSpeed % 100);
          Type4Ptr->CurrentSpeed  = Type4Ptr->CurrentSpeed - (Type4Ptr->CurrentSpeed % 100);
          break;

        case 23:
          Type23Ptr             = (SMBIOS_TABLE_TYPE_TWENTY_THREE *) SmbiosHeaderPtr;
          Type23Ptr->ResetCount = 0;
          break;

        default:
          break;
      }
      ///
      /// update both table pointer and scratch offset to be beyond this entry
      ///
      TableOffset += (SmbiosHeaderPtr->Length - sizeof (SMBIOS_HEADER));
      TableAddress += (SmbiosHeaderPtr->Length - sizeof (SMBIOS_HEADER));

    } else {
      ///
      /// skip this entry
      /// Move table address to beyond this entry, do not change scratch table offset
      ///
      TableAddress += SmbiosHeaderPtr->Length;
    }
    ///
    /// Copy any remaining unformatted data til end of type structure
    /// that is marked by double NULL bytes.
    ///
    EndOfEntry    = FALSE;
    PreviousNull  = FALSE;
    do {
      ///
      /// Read byte from bios data
      ///
      CopyMem (&DataByte, (VOID *) (UINTN) TableAddress++, 1);
      if (DataByte == 0x00) {
        if (SmbiosHeaderPtr->Type == 0) {
          StringCounter++;
        }

        if (PreviousNull) {
          ///
          /// this null marks end of entry
          ///
          EndOfEntry = TRUE;
        } else {
          ///
          /// flag we have seen first null
          ///
          PreviousNull = TRUE;
        }
      } else {
        ///
        /// clear null that terminated string
        ///
        PreviousNull = FALSE;
      }

      if (KeepEntry) {
        AssetTableData->TableData[TableOffset++] = DataByte;
      }

    } while (!EndOfEntry);

  }
  ///
  /// while !EndOfTable
  ///
  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_SMBIOS].Length = TableOffset - OrignalTableOffset;

  return EFI_SUCCESS;
}

/**
  Create the ASF table to send to AMT FW

  @param[in] AssetTableData       Buffer of all Asset tables to send to FW
  @param[in] TableOffset          Offset into AssetTableData that the ASF table will be located
  @param[in] PtrAcpiRsdt          Pointer to BIOS ASF constructed tables

  @retval EFI_ABORTED             AssetTableData data is invalid.
  @retval EFI_SUCCESS             ASF table created.
**/
EFI_STATUS
CalculateAsfTable (
  IN  TABLE_PUSH_DATA  *AssetTableData,
  IN  UINT16           TableOffset,
  IN  UINT32           PtrAcpiRsdt
  )
{
  ACPI_HEADER *AcpiHeaderPtr;
  UINT32      TableAddress;
  UINT32      ListAddress;
  UINT16      ArrayLength;

  TableAddress  = 0;
  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_ASF].Offset = TableOffset;
  CopyMem ((UINT8 *) &AssetTableData->TableData[TableOffset], (VOID *) (UINTN) PtrAcpiRsdt, sizeof (ACPI_HEADER));
  AcpiHeaderPtr = (ACPI_HEADER *) &AssetTableData->TableData[TableOffset];

  if (AcpiHeaderPtr->Signature != RSDT_SIG) {
    DEBUG ((DEBUG_WARN, "ASF ACPI Signature Does Not Match\n"));
    return EFI_ABORTED;
  }

  ArrayLength = (((UINT16) AcpiHeaderPtr->Length) - sizeof (ACPI_HEADER)) / sizeof (UINT32);
  ListAddress = PtrAcpiRsdt + sizeof (ACPI_HEADER);

  while (ArrayLength) {
    ///
    /// copy header at this new table address into scratch area
    ///
    CopyMem (&TableAddress, (VOID *) (UINTN) ListAddress, sizeof (UINT32));
    CopyMem (&AssetTableData->TableData[TableOffset], (VOID *) (UINTN) TableAddress, sizeof (ACPI_HEADER));

    if (AcpiHeaderPtr->Signature == ASF_SIG) {
      break;
    }

    ArrayLength--;
    ListAddress += sizeof (UINT32);
  }

  if (!ArrayLength) {
    ///
    /// We hit end of table
    ///
    DEBUG ((DEBUG_WARN, "Did Not Find ASF Signature \n"));
    return EFI_ABORTED;
  }

  CopyMem (&AssetTableData->TableData[TableOffset], (VOID *) (UINTN) TableAddress, AcpiHeaderPtr->Length);

  AssetTableData->Tables[HWAI_TABLE_TYPE_INDEX_ASF].Length = (UINT16) AcpiHeaderPtr->Length;

  return EFI_SUCCESS;
}

/**
  Constructs each of the lower level asset tables

  @param[in] AssetTablesData      Buffer of all Asset tables to send to FW
  @param[in] TablesSize           Size of all tables combined
  @param[in] AmtData              Structure that holds all the BIOS constructed tables

  @retval EFI_SUCCESS             Tables crated.
  @retval EFI_ABORTED             AssetTableData data is invalid.
**/
EFI_STATUS
CalculateTableData (
  IN  TABLE_PUSH_DATA  *AssetTablesData,
  IN  UINT16           *TablesSize,
  IN  AMT_DATA         *AmtData
  )
{
  EFI_STATUS  Status;
  UINT16      TableOffset;

  Status      = EFI_SUCCESS;
  TableOffset = 0;

  Status      = CalculateFruTable (AssetTablesData, AmtData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TableOffset = AssetTablesData->Tables[HWAI_TABLE_TYPE_INDEX_FRU_DEVICE].Length;
  Status      = CalculateMediaTable (AssetTablesData, TableOffset, AmtData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TableOffset = TableOffset + AssetTablesData->Tables[HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE].Length;
  Status      = CalculateSmbiosTable (AssetTablesData, TableOffset, AmtData->PtrSmbiosTable);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TableOffset = TableOffset + AssetTablesData->Tables[HWAI_TABLE_TYPE_INDEX_SMBIOS].Length;
  Status      = CalculateAsfTable (AssetTablesData, TableOffset, AmtData->PtrAcpiRsdt);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  TableOffset = TableOffset + AssetTablesData->Tables[HWAI_TABLE_TYPE_INDEX_ASF].Length;

  *TablesSize = TableOffset;

  return EFI_SUCCESS;
}

/**
  Constructs all asset tables and send them to FW

  @param[in] AmtData              Structure that holds all the BIOS constructed tables

  @retval EFI_ABORTED             Unable to allocate necessary AssetTableData data structure.
**/
EFI_STATUS
SendAssetTables2Firmware (
  IN  AMT_DATA  *AmtData
  )
{
  HECI_PROTOCOL     *Heci;
  TABLE_PUSH_DATA   *AssetTablesData;
  EFI_STATUS        Status;
  UINT16            TableOffset;
  UINT32            MeMode;
  UINT32            MeStatus;

  Status      = EFI_SUCCESS;
  TableOffset = 0;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AssetTablesData = AllocateZeroPool (sizeof (TABLE_PUSH_DATA) + MAX_ASSET_TABLE_ALLOCATED_SIZE);

  if (AssetTablesData == NULL) {
    DEBUG ((DEBUG_WARN, "SendAssetTables2Firmware Error: Could not allocate Memory\n"));
    return EFI_ABORTED;
  }

  Status = CalculateTableData (AssetTablesData, &TableOffset, AmtData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "SendAssetTables2Firmware: Error calculating Asset tables - No Data Pushed\n"));
    return Status;
  }

  Status = Heci->GetMeMode (&MeMode);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "SendAssetTables2Firmware: Could not read FW Mode\n"));
    return Status;
  }

  Status = Heci->GetMeStatus (&MeStatus);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "SendAssetTables2Firmware: Could not read FW Status"));
    return Status;
  }

  if ((MeMode == ME_MODE_NORMAL) && (ME_STATUS_ME_STATE_ONLY (MeStatus) == ME_READY)) {
    Status = HeciAssetUpdateFwMsg (AssetTablesData, TableOffset);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_WARN, "SendAssetTables2Firmware Error: AssetUpdateFwMsg() returned Status %x\n", Status));
    }
  }

  FreePool (AssetTablesData);

  return Status;
}

/**
  Print Mebx Data
**/
VOID
PrintMebxData (
  VOID
  )
{
  EFI_HOB_GUID_TYPE    *GuidHob;
  MEBX_DATA_HOB        *MebxData;

  GuidHob  = NULL;
  MebxData = NULL;

  GuidHob = GetFirstGuidHob (&gAmtMebxDataGuid);
  if (GuidHob != NULL) {
    MebxData = (MEBX_DATA_HOB *) GET_GUID_HOB_DATA (GuidHob);
  }
  if (MebxData == NULL) {
    return;
  }
  DEBUG ((DEBUG_INFO, "\n------------------------ Mebx Data Print Begin -----------------\n"));
  DEBUG ((DEBUG_INFO, " PlatformMngSel                      : 0x%x\n", MebxData->PlatformMngSel));
  DEBUG ((DEBUG_INFO, " AmtSol                              : 0x%x\n", MebxData->AmtSol));
  DEBUG ((DEBUG_INFO, " RemoteAssistanceTriggerAvailablilty : 0x%x\n", MebxData->RemoteAssistanceTriggerAvailablilty));
  DEBUG ((DEBUG_INFO, " KvmEnable                           : 0x%x\n", MebxData->KvmEnable));
  DEBUG ((DEBUG_INFO, "\n------------------------ Mebx Data Print End -------------------\n"));
  return;
}

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
  )
{
  EFI_STATUS                                    Status;
  MEBX_BPF                                      MebxBiosParams;
  AMT_DATA                                      AmtData;
  EFI_ACPI_3_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Acpi3RsdPtr;
  EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Acpi2RsdPtr;
  EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER  *Acpi1RsdPtr;
  VOID                                          *TempPointer;
  USB_KEY_PROVISIONING                          *UsbKeyProvisioningData;
  UINT8                                         InvokeMebx;
  UINT8                                         Data8;
  EFI_BOOT_MODE                                 BootMode;
  BOOLEAN                                       WarmReset;
  BOOLEAN                                       SendTables;
  BOOLEAN                                       FirstBoot;
  AMT_SAVE_MEBX_PROTOCOL                        *AmtSaveMebx;
  EFI_HOB_GUID_TYPE                             *GuidHob;
  MEBX_DATA_HOB                                 MebxData;

  DEBUG ((DEBUG_INFO, "Entering BiosExtensionLoader Driver\n"));
  FirstBoot = FALSE;
  GuidHob   = NULL;

  ///
  /// Verify FW SKU - dispatch correct images Corporate FW SKU only
  ///
  if (mFwImageType == IntelMeConsumerFw) {
    return;
  }

  GuidHob = GetFirstGuidHob (&gAmtMebxDataGuid);
  if (GuidHob != NULL) {
    CopyMem (&MebxData, GET_GUID_HOB_DATA (GuidHob), sizeof (MEBX_DATA_HOB));
  }

  if ((GuidHob == NULL) || AmtIsForceMebxSyncUpEnabled ()) {
    DEBUG ((DEBUG_WARN, "MebxData Hob does not exist: create with default values\n"));
    ///
    /// Create the variable when it does not exist
    ///
    FirstBoot                                    = TRUE;
    MebxData.PlatformMngSel                      = MEBX_SETUP_PLATFORM_MNT_DEFAULT;
    MebxData.AmtSol                              = MEBX_SETUP_SOL_DEFAULT;
    MebxData.RemoteAssistanceTriggerAvailablilty = 0;
    MebxData.KvmEnable                           = 0;

    Status = gBS->LocateProtocol (&gAmtSaveMebxProtocolGuid, NULL, (VOID **) &AmtSaveMebx);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Platform must implement this function to save Mebx setting\n"));
      ASSERT_EFI_ERROR (Status);
    } else {
      Status = AmtSaveMebx->SaveMebx (sizeof (MEBX_DATA_HOB), (VOID *) &MebxData);
      ASSERT_EFI_ERROR (Status);
    }
  }

  DEBUG_CODE_BEGIN ();
  DEBUG ((DEBUG_INFO, "Mebx data before calling MEBx:\n"));
  PrintMebxData ();
  DEBUG_CODE_END ();

  ZeroMem (&AmtData, sizeof (AMT_DATA));
  ZeroMem (&MebxBiosParams, sizeof (MEBX_BPF));

  MebxBiosParams.BpfVersion            = MEBX_VERSION;
  MebxBiosParams.CpuReplacementTimeout = (UINT8)(UINTN) mAmtDxeConfig->CpuReplacementTimeout;

  ///
  /// Setup CIRA data
  ///
  if (AmtIsCiraRequested ()) {
    MebxBiosParams.ActiveRemoteAssistanceProcess = 1;
    MebxBiosParams.CiraTimeout                   = AmtGetCiraRequestTimeout ();
  }

  ///
  /// Check for OEM Flags presence
  ///
  if (AmtIsHotkeyPressedEnabled ()) {
    MebxBiosParams.OemFlags |= MEBX_USER_ENTERED_RESPONSE;
  }
  if (AmtIsSelectionScreenEnabled ()) {
    MebxBiosParams.OemFlags |= MEBX_RA_SELECTION_MENU;
  }
  if (AmtIsUnconfigureMeEnabled ()) {
    MebxBiosParams.OemFlags |= MEBX_UNCONFIGURE;
  }
  if (AmtIsHideUnconfigureMeConfPromptEnabled ()) {
    MebxBiosParams.OemFlags |= MEBX_HIDE_UNCONFIGURE_CONF_PROMPT;
  }
  if (!AsfIsSolEnabled () && AmtIsMebxDebugMsgEnabled ()) {
    MebxBiosParams.OemFlags |= MEBX_DEBUG_MSG;
  }

  if (FirstBoot == TRUE) {
    MebxBiosParams.MebxDefaultSol = 1;
  }

  ///
  /// Pass OEM MEBx resolution settings through BPF
  ///
  MebxBiosParams.OemResolutionSettings.MebxGraphicsMode  = (UINT16)(UINTN) mAmtDxeConfig->MebxGraphicsMode;
  MebxBiosParams.OemResolutionSettings.MebxNonUiTextMode = (UINT16)(UINTN) mAmtDxeConfig->MebxNonUiTextMode;
  MebxBiosParams.OemResolutionSettings.MebxUiTextMode    = (UINT16)(UINTN) mAmtDxeConfig->MebxUiTextMode;

  AmtData.PciDevAssertInfoPtr  = (UINT32)(UINTN) &mAmtPciFru;
  AmtData.MediaDevAssetInfoPtr = (UINT32)(UINTN) &mAmtMediaFru;
  AmtData.VersionInfo          = AMT_DATA_VERSION;

  ///
  /// Now set the PCI devices
  ///
  Status = BuildPciFru ();
  if (EFI_ERROR (Status)) {
    return;
  }

  if (mAmtPciFru.PciDevicesHeader.DevCount != 0) {
    AmtData.SupportedTablesFlags |= MEBX_STF_PCI_DEV_TABLE;
  }

  ///
  /// HW asset tables need to be sent when:
  /// - FW asked for it, or
  /// - FW is alive (brand!=0) and platform isn't booting from a warm reset unless it's first boot
  ///
  WarmReset = PmcIsMemoryPlacedInSelfRefreshState ();

  SendTables = (mFwMediaTableReqAvail && mFwMediaTablePush) || (mFwPlatformBrand != NoBrand && (!WarmReset || FirstBoot));

  ///
  /// ME BWG HWT_PushBIOSTables
  /// Built Media List for 8.1 firmware onwards only if Firmware request it or full BIOS boot path (Note 2, 4)
  /// Always built media list with older firmware (indicated by missing mFwMediaTableReqAvail)
  /// Above all, don't waste time building media list if tables are not going to be sent.
  ///
  BootMode = GetBootModeHob ();

  if (SendTables &&
      ((BootMode != BOOT_WITH_MINIMAL_CONFIGURATION && BootMode != BOOT_ON_S4_RESUME) ||
       !mFwMediaTableReqAvail || (mFwMediaTableReqAvail && mFwMediaTablePush))) {
    //
    // Build Media List
    //
    BuildMediaList ();
    AmtData.SupportedTablesFlags |= MEBX_STF_MEDIA_DEV_TABLE;
  } else {
    DEBUG ((DEBUG_INFO, "No Media Asset Table is sent\n"));
  }

  mUsbProvsionDone = FALSE;
  ///
  /// If user selected for USB provisioning, then only use the provisioning file.
  ///
  if (IsUsbProvisionSupportEnabled () && IsAmtBiosSupportEnabled ()) {
    ///
    /// Check USB Key Provision
    ///
    UsbKeyProvisioning ();
  }

  ///
  /// fill in the USB provisioning data...
  ///
  if (mUsbProvsionDone == TRUE) {
    UsbKeyProvisioningData = (USB_KEY_PROVISIONING *) (UINTN) AllocateZeroPool (mUsbProvDataSize + sizeof (USB_KEY_PROVISIONING));
    ASSERT (UsbKeyProvisioningData != NULL);
    UsbKeyProvisioningData->USBKeyLocationInfo = (mUsbKeyPort << 16) |
                                                 (mUsbKeyBus << 8) |
                                                 (mUsbKeyDevice << 3) |
                                                 mUsbKeyFunction;
    MebxBiosParams.UsbKeyDataStructurePtr = (UINT32) (UINTN) UsbKeyProvisioningData;
    CopyMem (
      (VOID *) ((UINTN) MebxBiosParams.UsbKeyDataStructurePtr + sizeof (USB_KEY_PROVISIONING)),
      mUsbProvData,
      mUsbProvDataSize
      );
    UsbConsumedDataRecordRemove ();
  }

  ///
  /// Get SMBIOS table pointer.
  ///
  Status = EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid, (VOID **) &TempPointer);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "Error getting SMBIOS Table GUID %g -> %r\n", gEfiSmbiosTableGuid, Status));
    AmtData.PtrSmbiosTable = (UINT32) (UINTN) NULL;
  } else {
    AmtData.PtrSmbiosTable = (UINT32) (UINTN) TempPointer;
  }

  ///
  /// Find the AcpiSupport protocol returns RSDP (or RSD PTR) address.
  ///
  ///
  /// ACPI 2.0 or newer tables use gEfiAcpiTableGuid.
  ///
  Status = EfiGetSystemConfigurationTable (&gEfiAcpiTableGuid, (VOID *) &Acpi3RsdPtr);

  if (EFI_ERROR (Status) || (Acpi3RsdPtr == NULL)) {
    Status = EfiGetSystemConfigurationTable (&gEfiAcpi20TableGuid, (VOID *) &Acpi2RsdPtr);

    if (EFI_ERROR (Status) || (Acpi2RsdPtr == NULL)) {
      Status = EfiGetSystemConfigurationTable (&gEfiAcpiTableGuid, (VOID *) &Acpi1RsdPtr);
      if (EFI_ERROR (Status) || (Acpi1RsdPtr == NULL)) {
        DEBUG ((DEBUG_WARN, "Error getting ACPI Table -> %r\n", Status));
        return;
      } else {
        AmtData.PtrAcpiRsdt = Acpi1RsdPtr->RsdtAddress;
      }
    } else {
      AmtData.PtrAcpiRsdt = Acpi2RsdPtr->RsdtAddress;
    }
  } else {
    AmtData.PtrAcpiRsdt = Acpi3RsdPtr->RsdtAddress;
  }

  ///
  /// ME BWG HWT_PushBIOSTables
  /// Send Tables to Firmware as long as not on warm reset and AMT not permanently disabled (brand = 0)
  /// 8.1 firmware onwards, still send table to firmware as long as not on warm reset and AMT not permanently disable (note 2, 3)
  /// 8.1 firmware onwards, warm reset with MediaPush set will send asset table to firmware (Note 1)
  ///
  if (SendTables) {
    Status = SendAssetTables2Firmware (&AmtData);
    DEBUG ((DEBUG_INFO, "Send Asset Tables to AMT FW - Status = %r\n", Status));
  }

  ///
  /// Check if firmware INVOKE_MEBX bit is set
  ///
  Data8       = 0;
  Data8       = PciSegmentRead8 (PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, R_ME_HFS_2));
  InvokeMebx  = (Data8 & INVOKE_MEBX_BIT) >> 3;
  DEBUG ((DEBUG_INFO, "InvokeMebx = 0x%x\n", InvokeMebx));

  ///
  /// Check for BIOS invoke reason
  ///
  if (!InvokeMebx) {
    CheckForBiosInvokeReason (&InvokeMebx, &MebxBiosParams);
  }

  if (InvokeMebx) {
    PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8000);
    Status = AdjustAndExecuteMebxImage (&MebxBiosParams);
    PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8001);
  }

}

/**
  Detect EFI MEBx support; Loading and execute.

  @param[in] MebxBiosParamsBuffer MebxBiosParams Flat pointer

  @retval EFI_SUCCESS             Function executed successfully.
  @retval EFI_NOT_STARTED         MEBx not started.
**/
EFI_STATUS
AdjustAndExecuteMebxImage (
  IN MEBX_BPF *MebxBiosParams
  )
{
  EFI_STATUS        Status;
  MEBX_PROTOCOL     *MebxProtocol;
  MEBX_EXIT_CODE    MebxExitCode;

  ZeroMem (&MebxExitCode, sizeof (MEBX_EXIT_CODE));

  DEBUG ((DEBUG_INFO, "Calling MEBx\n"));

  ///
  /// Whenever the user gets into setup screens like BIOS setup, MEBX, or PXE,
  /// the BIOS should stop the watchdog and restart it when user exits the setup
  /// screens, otherwise the watchdog will expire in the case where the user enters the
  /// setup screen and does not dismiss it within the watchdog timer interval.
  /// Stop ASF BIOS watchdog timer if the corresponding option policy is true with the
  /// non-zero value in the BIOS timeout setting
  ///
  AsfStopWatchDog ();

  ///
  /// Locate MEBX protocol
  ///
  Status = gBS->LocateProtocol (&gMebxProtocolGuid, NULL, (VOID **) &MebxProtocol);
  if (!EFI_ERROR (Status)) {
    DEBUG_CODE_BEGIN ();
    ///
    /// Dump MebxBiosParams before launching MEBx
    ///
    DxeMebxBiosParamsDebugDump (MebxBiosParams);
    DEBUG_CODE_END ();

    DEBUG ((DEBUG_INFO, "ME-BIOS: MEBx Core Entry.\n"));
    PostCode (0xE07);

    MebxProtocol->CoreMebxEntry (MebxBiosParams, &MebxExitCode);

    DEBUG ((DEBUG_INFO, "ME-BIOS: MEBx Core Exit - Success.\n"));
    PostCode (0xE27);
  }

  ///
  /// Whenever the user gets into setup screens like BIOS setup, MEBX, or PXE,
  /// the BIOS should stop the watchdog and restart it when user exits the setup
  /// screens, otherwise the watchdog will expire in the case where the user enters the
  /// setup screen and does not dismiss it within the watchdog timer interval.
  /// Re-Start ASF Watch Dog after exiting MEBx Setup
  ///
  AsfStartWatchDog (ASF_START_BIOS_WDT);

  DEBUG ((DEBUG_INFO, "MEBx return BIOS_CMD_DATA:%x, BIOS_CMD:%x\n", MebxExitCode.BiosCmdData, MebxExitCode.BiosCmd));
  DEBUG ((DEBUG_INFO, "MEBx return MEBX_STATUS_CODE:%x\n", MebxExitCode.MebxStatusCode));

  ///
  /// Restore data record when needed
  ///
  if (mUsbProvsionDone == TRUE) {
    CopyMem (
      mUsbProvDataBackup,
      (VOID *) ((UINTN) MebxBiosParams->UsbKeyDataStructurePtr + sizeof (USB_KEY_PROVISIONING)),
      mUsbProvDataSize
      );
    UsbConsumedDataRecordRestore ();
  }

  switch (MebxExitCode.BiosCmd & MEBX_RET_CODE_MASK) {
    ///
    /// mask off reserved bits 3-7 from mebx exit status code
    ///
    case MEBX_RET_ACTION_CONTINUE_TO_BOOT:
      REPORT_STATUS_CODE (EFI_PROGRESS_CODE, EFI_SOFTWARE_UNSPECIFIED | EFI_SW_DXE_MEBX_OPROM_DONE);
      break;

    case MEBX_RET_ACTION_RESET:
      REPORT_STATUS_CODE (EFI_ERROR_CODE, EFI_SOFTWARE_UNSPECIFIED | EFI_SW_DXE_MEBX_RESET_ACTION);
      if (!(MebxExitCode.BiosCmdData & MEBX_RET_ACTION_GLOBAL_RESET)) {
        DEBUG ((DEBUG_INFO, "MEBx requires Global Reset.\n"));
        HeciSendCbmResetRequest (CBM_RR_REQ_ORIGIN_MEBX, CBM_HRR_GLOBAL_RESET);
        CpuDeadLoop ();
      } else {
        DEBUG ((DEBUG_INFO, "MEBx requires Host Reset.\n"));
        gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
      }
      break;

    default:
      REPORT_STATUS_CODE (EFI_ERROR_CODE, EFI_SOFTWARE_UNSPECIFIED | EFI_SW_DXE_MEBX_OTHER_UNSPECD);
      Status = EFI_NOT_STARTED;
      break;
  }

  DEBUG_CODE_BEGIN ();
  DEBUG ((DEBUG_INFO, "Mebx data after calling MEBx:\n"));
  PrintMebxData ();
  DEBUG_CODE_END ();

  ///
  /// Send PET Alert Message
  ///
  /// BIOS Specific Code
  ///
  /// Indicate OS BOOT handoff so that PET/ASF Push msg can be sent out to indicate
  /// all done now booting os.
  ///
  REPORT_STATUS_CODE (
    EFI_PROGRESS_CODE,
    EFI_SOFTWARE_UNSPECIFIED | EFI_SW_DXE_BS_PC_LEGACY_BOOT_EVENT
    );

  return Status;
}

/**
  The driver entry point - MEBx Driver main body.

  @param[in] ImageHandle          Handle for this drivers loaded image protocol.
  @param[in] SystemTable          EFI system table.

  @retval EFI_SUCCESS             The driver installed without error.
  @retval EFI_UNSUPPORTED         The chipset is unsupported by this driver.
**/
EFI_STATUS
EFIAPI
MebxDriverEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS            Status;
  HECI_PROTOCOL         *Heci;
  UINT32                MeMode;
  EFI_HANDLE            Handle;
  ME_BIOS_PAYLOAD_HOB   *MbpHob;

  MbpHob        = NULL;
  mFwImageType  = IntelMeCorporateFw;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Heci->GetMeMode (&MeMode);
  if (MeMode != ME_MODE_NORMAL) {
    return EFI_UNSUPPORTED;
  }

  ///
  /// Init AMT ConfigBlock
  ///
  Status = AmtDxeConfigInit ();
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  ///
  /// Install an Amt ready to boot protocol.
  ///
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gAmtReadyToBootProtocolGuid,
                  &mAmtReadyToBoot,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    //
    // Get the MBP Data.
    //
    MbpHob = GetFirstGuidHob (&gMeBiosPayloadHobGuid);
    if (MbpHob == NULL) {
      DEBUG ((DEBUG_WARN, "MEBx: No MBP Data HOB available\n"));
      return Status;
    } else {
      ///
      /// Pass FW SKU Type
      ///
      mFwImageType      = (UINT8) MbpHob->MeBiosPayload.FwPlatType.RuleData.Fields.IntelMeFwImageType;
      mFwPlatformBrand  = (UINT8) MbpHob->MeBiosPayload.FwPlatType.RuleData.Fields.PlatformBrand;
      ///
      /// Save for Later use when MBPdata is deallocated
      ///
      mFwMediaTableReqAvail = MbpHob->MeBiosPayload.HwaRequest.Available;
      mFwMediaTablePush     = (BOOLEAN) MbpHob->MeBiosPayload.HwaRequest.Data.Fields.MediaTablePush;
    }
  }

  return Status;
}

/**
  Detect BIOS invoke reasons.

  @param[in] InvokeMebx           Pointer to the Invoke MEBx flag
  @param[in] MebxBiosParamsBuffer MebxBiosParams Flat pointer
**/
VOID
CheckForBiosInvokeReason (
  IN  UINT8       *InvokeMebx,
  IN  MEBX_BPF    *MebxBiosParams
  )
{
  ///
  /// Check for BIOS invoke reason
  ///
  if (MebxBiosParams->OemFlags & MEBX_USER_ENTERED_RESPONSE) {
    DEBUG ((DEBUG_INFO, "InvokeMebx Reason = MEBX_USER_ENTERED_RESPONSE\n"));
    *InvokeMebx = 1;
  }

  if (MebxBiosParams->OemFlags & MEBX_UNCONFIGURE) {
    DEBUG ((DEBUG_INFO, "InvokeMebx Reason = MEBX_UNCONFIGURE\n"));
    *InvokeMebx = 1;
  }

  if (MebxBiosParams->UsbKeyDataStructurePtr) {
    DEBUG ((DEBUG_INFO, "InvokeMebx Reason = UsbKeyDataStructurePtr\n"));
    *InvokeMebx = 1;
  }

  if (MebxBiosParams->ActiveRemoteAssistanceProcess) {
    DEBUG ((DEBUG_INFO, "InvokeMebx Reason = ActiveRemoteAssistanceProcess\n"));
    *InvokeMebx = 1;
  }

  if (AmtIsForceMebxSyncUpEnabled ()) {
    DEBUG ((DEBUG_INFO, "InvokeMebx Reason = forcing ME-BIOS sync-up\n"));
    MebxBiosParams->OemFlags  = 0;
    *InvokeMebx               = 1;
  }

  return;
}
