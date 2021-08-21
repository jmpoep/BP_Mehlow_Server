/** @file
  Processes ASF messages.

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

#include "AlertStandardFormatDxe.h"

GLOBAL_REMOVE_IF_UNREFERENCED ASF_BOOT_OPTIONS                mAsfBootOptions;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                         mBootOptionsValid;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                         mProgressEventEnabled;
GLOBAL_REMOVE_IF_UNREFERENCED ALERT_STANDARD_FORMAT_INSTANCE  *mAsfContext;
GLOBAL_REMOVE_IF_UNREFERENCED AMT_PET_QUEUE_PROTOCOL          mAmtPetQueue;
GLOBAL_REMOVE_IF_UNREFERENCED AMT_PET_QUEUE_PROTOCOL          *mAmtPetQueueProtocol;
GLOBAL_REMOVE_IF_UNREFERENCED AMT_FORCE_PUSH_PET_POLICY_HOB   *mAmtForcePushPetPolicyHob;

/**
  Update ASF Boot Options data in ACPI ASF RMCP table

  @param[in] AsfBootOptions     ASF Boot Options data for ACPI ASF RMCP table used
**/
VOID
UpdateAcpiAsfRmcpBootOptions (
  ASF_BOOT_OPTIONS                    *AsfBootOptions
  )
{
  EFI_STATUS                          Status;
  ACPI_2_0_ASF_DESCRIPTION_TABLE      *AcpiAsfTable;
  UINTN                               Handle;
  UINT16                              *WordPointer;
  EFI_ACPI_TABLE_PROTOCOL             *AcpiTable;

  ///
  /// Locate ACPI Table Protocol
  ///
  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **) &AcpiTable);

  ///
  /// Initialize ASL manipulation library
  ///
  InitializeAslUpdateLib ();

  ///
  /// Locate the ASF Table
  ///
  AcpiAsfTable = NULL;
  Status = LocateAcpiTableBySignature (
             EFI_ACPI_ASF_DESCRIPTION_TABLE_SIGNATURE,
             (EFI_ACPI_DESCRIPTION_HEADER **) &AcpiAsfTable,
             &Handle
             );
  if (EFI_ERROR (Status) || (AcpiAsfTable == NULL)) {
    return;
  }

  if (AsfBootOptions->SubCommand == ASF_BOOT_OPTIONS_PRESENT) {
    ///
    /// Check Asf table if get boot option successfully
    ///
    AcpiAsfTable->AsfRmcp.RMCPCompletionCode  = 00;
    AcpiAsfTable->AsfRmcp.RMCPIANA            = AsfBootOptions->IanaId;
    AcpiAsfTable->AsfRmcp.RMCPSpecialCommand  = AsfBootOptions->SpecialCommand;
    WordPointer = (UINT16 *) &AcpiAsfTable->AsfRmcp.RMCPSpecialCommandParameter;
    *WordPointer = AsfBootOptions->SpecialCommandParam;
    WordPointer = (UINT16 *) &AcpiAsfTable->AsfRmcp.RMCPBootOptions;
    *WordPointer = AsfBootOptions->BootOptions;
    WordPointer = (UINT16 *) &AcpiAsfTable->AsfRmcp.RMCPOEMParameters;
    *WordPointer = AsfBootOptions->OemParameters;
  } else {
    ///
    /// Failed to get boot option, update the completion code to 0x1
    ///
    AcpiAsfTable->AsfRmcp.RMCPCompletionCode = 01;
  }
  if (Handle != 0) {
    Status = AcpiTable->UninstallAcpiTable (
                          AcpiTable,
                          Handle
                          );
  }
  ///
  /// Update the Acpi Asf table
  ///
  Status = AcpiTable->InstallAcpiTable (
                        AcpiTable,
                        AcpiAsfTable,
                        AcpiAsfTable->Header.Length,
                        &Handle
                        );
  FreePool (AcpiAsfTable);
}

/**
  16 bits are changed backward for string transfer to value used

  @param[in] Value                The value to be transferred

  @retval UINT16 Value            The value after transferring
**/
UINT16
Swap16 (
  IN  UINT16                      Value
  )
{
  UINT16                          OutValue;
  UINT8                           *TempIn;
  UINT8                           *TempOut;

  TempIn      = (UINT8 *) &Value;
  TempOut     = (UINT8 *) &OutValue;

  TempOut[0]  = TempIn[1];
  TempOut[1]  = TempIn[0];

  return OutValue;
}

/**
  The driver entry point - detect ASF support or not, if support, will install relative protocol.

  @param[in] ImageHandle          Handle for this drivers loaded image protocol.
  @param[in] SystemTable          EFI system table.

  @retval EFI_SUCCESS             The driver installed without error.
  @retval EFI_UNSUPPORTED         The chipset is unsupported by this driver.
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate necessary data structures.
**/
EFI_STATUS
EFIAPI
AlertStandardFormatDriverEntryPoint (
  IN EFI_HANDLE                   ImageHandle,
  IN EFI_SYSTEM_TABLE             *SystemTable
  )
{
  EFI_STATUS                      Status;
  ASF_BOOT_OPTIONS                *AsfBootOptions;
  EFI_RSC_HANDLER_PROTOCOL        *AsfRscHandlerProtocol;
  EFI_EVENT                       AsfExitBootServicesEvent;

  ///
  /// First check if ASF support is enabled in Setup.
  ///
  DEBUG ((DEBUG_INFO, "AlertStandardFormatDriverEntryPoint start\n"));

  if (!IsAsfBiosSupportEnabled ()) {
    return EFI_UNSUPPORTED;
  }

  mBootOptionsValid = FALSE;
  mAsfContext       = AllocateZeroPool (sizeof (ALERT_STANDARD_FORMAT_INSTANCE));
  ASSERT (mAsfContext != NULL);
  if (mAsfContext == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  ///
  /// Get ReportStatusCodeHandler Protocol
  ///
  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &AsfRscHandlerProtocol
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "Locate gEfiRscHandlerProtocolGuid Protocol status is %r\n", Status));

  mAsfContext->Handle = ImageHandle;
  mAsfContext->Signature = ALERT_STANDARD_FORMAT_PRIVATE_DATA_SIGNATURE;
  mAsfContext->AlertStandardFormat.GetSmbusAddr = AsfGetSmbusAddr;
  mAsfContext->AlertStandardFormat.SetSmbusAddr = AsfSetSmbusAddr;
  mAsfContext->AlertStandardFormat.GetBootOptions = AsfGetBootOptions;
  mAsfContext->AlertStandardFormat.SendAsfMessage = AsfSendAsfMessage;

  ///
  /// Sending ASF Messaging if ManageabilityMode is not zero
  ///
  if (AmtIsManageabilityModeEnabled ()) {
    DEBUG ((DEBUG_INFO, "Sending ASF Messaging if ManageabilityMode is not zero\n"));
    ///
    /// ActiveManagement Protocol is not ready at this point.  Need to Check Boot Options Manually
    ///
    mProgressEventEnabled = FALSE;
    Status                = AsfGetBootOptions (&(mAsfContext->AlertStandardFormat), &AsfBootOptions);
    if (!EFI_ERROR (Status)) {
      if (AsfBootOptions->SubCommand == ASF_BOOT_OPTIONS_PRESENT) {
        if ((AsfBootOptions->BootOptions & FORCE_PROGRESS_EVENTS) == FORCE_PROGRESS_EVENTS) {
          mProgressEventEnabled = TRUE;
        }
      }
    }
    ///
    /// If no boot options available, check policy
    ///
    if (!mProgressEventEnabled) {
      mProgressEventEnabled = IsFwProgressSupported ();
    }
    ///
    /// Create message queue
    ///
    AmtCreateMessageQueue ();

    ///
    /// Get ForcePushPetPolicy Hob
    ///
    mAmtForcePushPetPolicyHob = GetForcePushPetPolicy ();

    ///
    /// Save PEI force push error event from hob to variable
    ///
    SaveForcePushErrorEventFromPeiToDxe ();

    ///
    /// Try to send message
    ///
    SendPetMessageInQueue ();

    ///
    /// Register our Status Code Report Function.
    /// This function is notified at the highest TPL
    ///
    Status = AsfRscHandlerProtocol->Register (AsfStatusCodeReportListener, TPL_HIGH_LEVEL);
    ASSERT_EFI_ERROR (Status);
    DEBUG ((DEBUG_INFO, "Register AsfStatusCodeReportListener status is %r\n", Status));

    ///
    /// Create the notify function to unregister AsfStatusCodeReportListener on ExitBootServices Event.
    ///
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    AsfExitBootServicesEventNotify,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &AsfExitBootServicesEvent
                    );
    ASSERT_EFI_ERROR (Status);

  }

  ///
  /// Install the AlertStandardFormat interface
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mAsfContext->Handle,
                  &gAlertStandardFormatProtocolGuid,
                  &mAsfContext->AlertStandardFormat,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Return the SMBus address used by the ASF driver.
  Not applicable in Intel ME/HECI system, need to return EFI_UNSUPPORTED.

  @param[in] This                 The address of protocol
  @param[in] SmbusDeviceAddress   Out put the Smbus Address

  @retval EFI_UNSUPPORTED         The function is unsupported by this driver
**/
EFI_STATUS
EFIAPI
AsfGetSmbusAddr (
  IN  ALERT_STANDARD_FORMAT_PROTOCOL       *This,
  OUT UINTN                                *SmbusDeviceAddress
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Set the SMBus address used by the ASF driver. 0 is an invalid address.
  Not applicable in Intel ME/HECI system, need to return EFI_UNSUPPORTED.

  @param[in] This                 The address of protocol
  @param[in] SmbusDeviceAddress   SMBus address of the device

  @retval EFI_UNSUPPORTED         The function is unsupported by this driver
**/
EFI_STATUS
EFIAPI
AsfSetSmbusAddr (
  IN  ALERT_STANDARD_FORMAT_PROTOCOL       *This,
  IN  UINTN                                SmbusDeviceAddress
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Return EFI_SUCCESS if Firmware Init Complete is set in HFS[9].

  @param[in] Heci                 EFI_HECI_PROTOCOL
  @param[in] Timeout              Time in second

  @retval EFI_SUCCESS             Firmware Init Complete is set
  @retval EFI_TIMEOUT             Firmware Init Complete is not set in 5 seconds
**/
EFI_STATUS
WaitFirmwareInitDone (
  HECI_PROTOCOL                   *Heci,
  UINT32                          Timeout
  )
{
  UINT32      Index;
  EFI_STATUS  Status;
  UINT32      MeStatus;

  Status  = EFI_SUCCESS;
  Index     = 0;

  Heci->GetMeStatus (&MeStatus);

  while (!ME_STATUS_IS_ME_FW_INIT_COMPLETE (MeStatus)) {
    MicroSecondDelay (100000);
    Index++;
    if (Index > Timeout * 10) {
      Status = EFI_TIMEOUT;
      break;
    }

    Heci->GetMeStatus (&MeStatus);
  }

  return Status;
}

/**
  Initialize KVM by sending HECI message to ME

  @param[in] Event                The event registered.
  @param[in] Context              Event context. Not used in this event handler.
**/
VOID
EFIAPI
QueryKvm (
  IN EFI_EVENT                    Event,
  IN VOID                         *Context
  )
{
  EFI_STATUS                      Status;
  UINT32                          ResponseCode;

  gBS->CloseEvent (Event);

  MeReportError (MSG_KVM_WAIT);
  Status = AmtQueryKvm (QueryRequest, &ResponseCode);
  if (EFI_ERROR (Status)) {
    MeReportError (MSG_KVM_TIMES_UP);
    AmtQueryKvm (CancelRequest, &ResponseCode);
  } else if (ResponseCode == KvmSessionCancelled) {
    MeReportError (MSG_KVM_REJECTED);
  }

}

/**
  Return the ASF Boot Options obtained from the controller. If the
  Boot Options parameter is NULL and no boot options have been retrieved,
  Query the ASF controller for its boot options.
  Get ASF Boot Options through HECI.

  @param[in] This                 The address of protocol
  @param[in] AsfBootOptions       Pointer to ASF boot options to copy current ASF Boot options

  @retval EFI_SUCCESS             Boot options copied
  @retval EFI_NOT_READY           No boot options
**/
EFI_STATUS
EFIAPI
AsfGetBootOptions (
  IN      ALERT_STANDARD_FORMAT_PROTOCOL       *This,
  IN  OUT ASF_BOOT_OPTIONS                     **AsfBootOptions
  )
{
  EFI_STATUS                          Status;
  UINT32                              MeStatus;
  UINT32                              MeMode;
  EFI_STATUS                          TempStatus;
  VOID                                *AfterConsolOutNotifyReg;
  EFI_EVENT                           AfterConsolOutInstalledEvent;
  HECI_PROTOCOL                       *Heci;
  GET_BOOT_OPTIONS_RESPONSE           BootOptionsResponse;

  DEBUG ((DEBUG_INFO, "ME-BIOS: AMT: Get Boot Options Entry.\n"));
  PostCode (0xE09);

  if (!mBootOptionsValid) {
    Status = gBS->LocateProtocol (
                    &gHeciProtocolGuid,
                    NULL,
                    (VOID **) &Heci
                    );

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ME-BIOS: AMT: Get Boot Options Exit - Error by HECI service not found, Status: %r\n", Status));
      PostCode (0xE89);
      return Status;
    }

    ZeroMem ((VOID *) &mAsfBootOptions, sizeof (ASF_BOOT_OPTIONS));
    mAsfBootOptions.SubCommand  = ASF_BOOT_OPTIONS_NOT_PRESENT;
    mAsfBootOptions.Version     = ASF_VERSION;
    UpdateAcpiAsfRmcpBootOptions (&mAsfBootOptions);

    ///
    /// Get ME Status
    ///
    TempStatus = Heci->GetMeStatus (&MeStatus);
    ASSERT_EFI_ERROR (TempStatus);

    ///
    /// Get ME Operation Mode
    ///
    Heci->GetMeMode (&MeMode);

    ///
    /// Only Send ASF Get Boot Options message when ME is ready and ME FW INIT is completed
    ///
    if (ME_STATUS_IS_ME_FW_BOOT_OPTIONS_PRESENT (MeStatus) &&
        (AmtIsManageabilityModeEnabled ()) &&
        (MeMode == ME_MODE_NORMAL)) {
      if (WaitFirmwareInitDone (Heci, FW_INIT_DONE_TIMEOUT_VALUE) != EFI_TIMEOUT) {
        ZeroMem ((VOID *) &BootOptionsResponse, sizeof (GET_BOOT_OPTIONS_RESPONSE));
        Status = GetBootOptions (&BootOptionsResponse);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_ERROR, "ME-BIOS: AMT: Get Boot Options Exit - Error by HECI access fail.\n"));
          PostCode (0xEE9);
          return Status;
        }

        CopyMem ((VOID *)&mAsfBootOptions, (VOID *)&(BootOptionsResponse.AsfBootOptions), sizeof (ASF_BOOT_OPTIONS));

        ///
        /// By default, this table is belong to all ACPI table versions.
        /// It is ok if the table is not found and that means the platform does not publish that version.
        ///
        UpdateAcpiAsfRmcpBootOptions (&mAsfBootOptions);

        mAsfBootOptions.SpecialCommandParam = Swap16 (mAsfBootOptions.SpecialCommandParam);

        ///
        /// ASF Get Boot Options -  Clear Boot Options
        /// Need to do this so don't get caught in an endless loop plus by
        /// definition get boot options is a one time boot situation.
        /// So if server on other end of ASF device wants another back to back
        /// boot it will request it. Do this only if there were boot options
        ///
        if (mAsfBootOptions.SubCommand == ASF_BOOT_OPTIONS_PRESENT) {
          if ((mAsfBootOptions.SpecialCommand == ASF_INTEL_OEM_CMD) &&
              (mAsfBootOptions.SpecialCommandParam & ENABLE_SECURE_ERASE)) {
            //
            // When secure erase boot option is set, don't clear boot options, but discard
            // any other boot option except secure erase, KVM and SOL
            //
            DEBUG ((DEBUG_INFO, "ASF Clear Boot Options skipped due to secure erase command\n"));
            DEBUG ((DEBUG_INFO, "Dumping SpecialCommandParam, BootOptions and OemParameters before discarding\n"));
            DEBUG ((DEBUG_INFO, "mAsfBootOptions.SpecialCommandParam = 0x%x\n", mAsfBootOptions.SpecialCommandParam));
            DEBUG ((DEBUG_INFO, "mAsfBootOptions.BootOptions         = 0x%x\n", mAsfBootOptions.BootOptions));
            DEBUG ((DEBUG_INFO, "mAsfBootOptions.OemParameters       = 0x%x\n", mAsfBootOptions.OemParameters));
            DEBUG ((DEBUG_INFO, "End\n\n" ));
            mAsfBootOptions.SpecialCommandParam  &= (ENABLE_SECURE_ERASE | USE_KVM);
            mAsfBootOptions.OemParameters        &= USE_SOL;
            mAsfBootOptions.BootOptions           = 0x0;
          } else {
            Status = ClearBootOptions ();
            if (EFI_ERROR (Status)) {
              DEBUG ((DEBUG_ERROR, "ME-BIOS: AMT: Get Boot Options Exit - Error by ClearBootOptions., Status: %r\n", Status));
              PostCode (0xEC9);
              return Status;
            }
          }
        }
      }
    }
    ///
    /// Set flag so we don't try to get the Boot options again.
    ///
    mBootOptionsValid = TRUE;

    ///
    /// Set up Event for KVM for when Output Display Console is installed
    ///
    if ((mAsfBootOptions.SpecialCommandParam & USE_KVM) == USE_KVM) {
      Status = gBS->CreateEvent (
                      EVT_NOTIFY_SIGNAL,
                      TPL_NOTIFY,
                      QueryKvm,
                      NULL,
                      &AfterConsolOutInstalledEvent
                      );

      Status = gBS->RegisterProtocolNotify (
                      &gEfiConsoleOutDeviceGuid,
                      AfterConsolOutInstalledEvent,
                      &AfterConsolOutNotifyReg
                      );

    }
  }

  if (AsfBootOptions) {
    *AsfBootOptions = &mAsfBootOptions;

    ///
    /// If we're asking for the options, then resend them to the debug output just encase they've been hosed.
    ///
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.SubCommand          = 0x%x\n", mAsfBootOptions.SubCommand));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.Version             = 0x%x\n", mAsfBootOptions.Version));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.IanaId              = 0x%x\n", mAsfBootOptions.IanaId));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.SpecialCommand      = 0x%x\n", mAsfBootOptions.SpecialCommand));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.SpecialCommandParam = 0x%x\n", mAsfBootOptions.SpecialCommandParam));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.BootOptions         = 0x%x\n", mAsfBootOptions.BootOptions));
    DEBUG ((DEBUG_INFO, "mAsfBootOptions.OemParameters       = 0x%x\n", mAsfBootOptions.OemParameters));
  }

  DEBUG ((DEBUG_INFO, "ME-BIOS: AMT: Get Boot Options Exit - Success.\n"));
  PostCode (0xE29);
  return EFI_SUCCESS;
}

/**
  Send ASF Message through HECI.

  @param[in] This                 The address of protocol
  @param[in] AsfMessage           Pointer to ASF message

  @retval EFI_SUCCESS             Boot options copied
  @retval EFI_INVALID_PARAMETER   Invalid pointer
  @retval EFI_NOT_READY           No controller
**/
EFI_STATUS
EFIAPI
AsfSendAsfMessage (
  IN  ALERT_STANDARD_FORMAT_PROTOCOL       *This,
  IN  ASF_MESSAGE                          *AsfMessage
  )
{
  EFI_STATUS                  Status;
  HECI_PROTOCOL               *Heci;
  UINT32                      HeciLength;
  HECI_ASF_PUSH_PROGRESS_CODE HeciAsfPushProgressCode;
  UINT32                      MeStatus;
  UINT32                      MeMode;
  EFI_STATUS                  TempStatus;

  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (!EFI_ERROR (Status)) {
    TempStatus = Heci->GetMeStatus (&MeStatus);
    ASSERT_EFI_ERROR (TempStatus);

    ///
    /// Get ME Operation Mode
    ///
    Heci->GetMeMode (&MeMode);

    ///
    /// Only send ASF Push Progress code when ME is ready and ME is in normal mode.  Ignore FW Init Status.
    ///
    if (ME_STATUS_ME_STATE_ONLY (MeStatus) == ME_READY && MeMode == ME_MODE_NORMAL) {
      ZeroMem ((VOID *) &HeciAsfPushProgressCode, sizeof (HECI_ASF_PUSH_PROGRESS_CODE));
      HeciAsfPushProgressCode.Command   = ASF_MESSAGE_COMMAND_MESSAGE;
      HeciAsfPushProgressCode.ByteCount = ASF_MESSAGE_BYTE_COUNT_MAP (HECI_ASF_PUSH_PROGRESS_CODE);
      HeciLength                        = sizeof (HECI_ASF_PUSH_PROGRESS_CODE);
      CopyMem ((VOID *) &(HeciAsfPushProgressCode.AsfMessage), (VOID *) AsfMessage, sizeof (ASF_MESSAGE));

      Status = Heci->SendMsg (
                       HECI1_DEVICE,
                       (UINT32 *) &HeciAsfPushProgressCode,
                       HeciLength,
                       BIOS_ASF_HOST_ADDR,
                       HECI_ASF_MESSAGE_ADDR
                       );
    }
  }

  return Status;
}

/**
  This routine returns ForcePushPetPolicy information.

  @retval AMT_FORCE_PUSH_PET_POLICY_HOB   ForcePushPetPolicy information.
**/
AMT_FORCE_PUSH_PET_POLICY_HOB *
GetForcePushPetPolicy (
  VOID
  )
{
  AMT_FORCE_PUSH_PET_POLICY_HOB *AmtForcePushPetPolicyHob;
  EFI_STATUS                    Status;

  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **) &AmtForcePushPetPolicyHob);
  ASSERT_EFI_ERROR (Status);

  return GetNextGuidHob (&gAmtForcePushPetPolicyGuid, AmtForcePushPetPolicyHob);
}

/**
  This routine checks whether current message is ForcePush message.

  @param[in] MessageType          AMT PET Message Type.

  @retval TRUE                    It is ForcePush message.
  @retval FALSE                   It is not ForcePush message.
**/
BOOLEAN
IsForcePushErrorEvent (
  IN  ASF_FRAMEWORK_MESSAGE_TYPE  MessageType
  )
{
  UINTN Index;
  UINTN Number;

  Number = (mAmtForcePushPetPolicyHob->EfiHobGuidType.Header.HobLength - sizeof (EFI_HOB_GUID_TYPE)) /
    sizeof (ASF_FRAMEWORK_MESSAGE_TYPE);
  for (Index = 0; Index < Number; Index++) {
    if (mAmtForcePushPetPolicyHob->MessageType[Index] == MessageType) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Filters all the progress and error codes for Asf.

  @param[in]  CodeType         Indicates the type of status code being reported.
  @param[in]  Value            Describes the current status of a hardware or software entity.
                               This included information about the class and subclass that is used to
                               classify the entity as well as an operation.
  @param[in]  Instance         The enumeration of a hardware or software entity within
                               the system. Valid instance numbers start with 1.
  @param[in]  CallerId         This optional parameter may be used to identify the caller.
                               This parameter allows the status code driver to apply different rules to
                               different callers.
  @param[in]  Data             This optional parameter may be used to pass additional data.

  @retval EFI_SUCCESS          Reported all the progress and error codes for Asf successfully.
  @retval EFI_DEVICE_ERROR     Sends a POST packet across Asf fail.

**/
EFI_STATUS
EFIAPI
AsfStatusCodeReportListener (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId,
  IN EFI_STATUS_CODE_DATA     *Data OPTIONAL
  )
{
  UINTN           Index;
  EFI_STATUS      Status;

  Status = EFI_SUCCESS;

  if (((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_PROGRESS_CODE) && (mProgressEventEnabled == TRUE)) {
    for (Index = 0; Index < AsfProgressDataHubMapSize / sizeof (ASF_DATA_HUB_MAP); Index++) {
      if (mAsfProgressDataHubMap[Index].StatusCodeValue == Value) {
        Status = SendPostPacket (mAsfProgressDataHubMap[Index].MessageType);
      }
    }
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) {
    for (Index = 0; Index < AsfErrorDataHubMapSize / sizeof (ASF_DATA_HUB_MAP); Index++) {
      if (mAsfErrorDataHubMap[Index].StatusCodeValue == Value) {
        Status = SendPostPacket (mAsfErrorDataHubMap[Index].MessageType);
        if ((Status == EFI_DEVICE_ERROR) && IsForcePushErrorEvent (mAsfErrorDataHubMap[Index].MessageType)) {
          SaveForcePushErrorEvent (mAsfErrorDataHubMap[Index].MessageType);
        }
      }
    }
  }

  return Status;
}

/**
  Unregister status code callback functions only available at boot time from
  report status code router when exiting boot services.

  @param[in]  Event         Event whose notification function is being invoked.
  @param[in]  Context       Pointer to the notification function's context, which is
                            always zero in current implementation.

**/
VOID
EFIAPI
AsfExitBootServicesEventNotify (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
  EFI_STATUS                      Status;
  EFI_RSC_HANDLER_PROTOCOL        *AsfRscHandlerProtocol;

  DEBUG ((DEBUG_INFO, "AsfExitBootServicesEventNotify\n"));
  Status = gBS->CloseEvent (Event);

  ///
  /// Get ReportStatusCodeHandler Protocol
  ///
  Status = gBS->LocateProtocol (
                  &gEfiRscHandlerProtocolGuid,
                  NULL,
                  (VOID **) &AsfRscHandlerProtocol
                  );
  ASSERT_EFI_ERROR (Status);
  if (!EFI_ERROR (Status)) {
    Status = AsfRscHandlerProtocol->Unregister (AsfStatusCodeReportListener);
    DEBUG ((DEBUG_INFO, "Unregister AsfStatusCodeReportListener Status = %r\n", Status));
  }

  return;
}

/**
  Sends a POST packet across ASF

  @param[in] MessageType          POST Status Code

  @retval EFI_DEVICE_ERROR        No message found
  @retval EFI_SUCCESS             Boot options copied
  @retval EFI_INVALID_PARAMETER   Invalid pointer
  @retval EFI_NOT_READY           No controller
**/
EFI_STATUS
SendPostPacket (
  IN  ASF_FRAMEWORK_MESSAGE_TYPE      MessageType
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  Status = EFI_DEVICE_ERROR;
  ///
  /// Find the message to send across the wire
  ///
  for (Index = 0; Index < AsfFrameworkMessageSize / sizeof (ASF_FRAMEWORK_MESSAGE); Index++) {
    if (mAsfFrameworkMessage[Index].MessageType == MessageType) {
      Status = AsfSendAsfMessage (NULL, &mAsfFrameworkMessage[Index].Message);
      break;
    }
  }

  return Status;
}

/**
  Provides an interface that a software module can call to report an ASF DXE status code.

  @param[in] Type                 Indicates the type of status code being reported.
  @param[in] Value                Describes the current status of a hardware or software entity.
                                  This included information about the class and subclass that is
                                  used to classify the entity as well as an operation.
  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_DEVICE_ERROR        The function should not be completed due to a device error.
**/
EFI_STATUS
EFIAPI
AmtReportStatusCode (
  IN  EFI_STATUS_CODE_TYPE        Type,
  IN  EFI_STATUS_CODE_VALUE       Value
  )
{
  UINTN       Index;
  EFI_STATUS  Status;

  if (mProgressEventEnabled) {
    if ((Type & EFI_STATUS_CODE_TYPE_MASK) == EFI_PROGRESS_CODE) {
      for (Index = 0; Index < AsfProgressDataHubMapSize / sizeof (ASF_DATA_HUB_MAP); Index++) {
        if (mAsfProgressDataHubMap[Index].StatusCodeValue == Value) {
          return SendPostPacket (mAsfProgressDataHubMap[Index].MessageType);
        }
      }
    }
  }

  if ((Type & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) {
    for (Index = 0; Index < AsfErrorDataHubMapSize / sizeof (ASF_DATA_HUB_MAP); Index++) {
      if (mAsfErrorDataHubMap[Index].StatusCodeValue == Value) {
        Status = SendPostPacket (mAsfErrorDataHubMap[Index].MessageType);
        if ((Status == EFI_DEVICE_ERROR) && IsForcePushErrorEvent (mAsfErrorDataHubMap[Index].MessageType)) {
          SaveForcePushErrorEvent (mAsfErrorDataHubMap[Index].MessageType);
        }

        return Status;
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  This routine puts PET message to MessageQueue, which will be sent later.

  @param[in] Type                 StatusCode message type.
  @param[in] Value                StatusCode message value.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate necessary data structures
**/
EFI_STATUS
QueuePetMessage (
  IN  EFI_STATUS_CODE_TYPE        Type,
  IN  EFI_STATUS_CODE_VALUE       Value
  )
{
  AMT_PET_QUEUE_NODE  *NewNode;

  NewNode = AllocateZeroPool (sizeof (AMT_PET_QUEUE_NODE));
  ASSERT (NewNode != NULL);
  if (NewNode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NewNode->Signature  = AMT_PET_QUEUE_NODE_SIGNATURE;
  NewNode->Type       = Type;
  NewNode->Value      = Value;
  InsertTailList ((LIST_ENTRY *) &mAmtPetQueueProtocol->MessageList, (LIST_ENTRY *) &NewNode->Link);

  return EFI_SUCCESS;
}

/**
  This routine sends PET message in MessageQueue.

  @param[in] VOID

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_NOT_READY           No controller
**/
EFI_STATUS
SendPetMessageInQueue (
  VOID
  )
{
  EFI_STATUS            Status;
  AMT_PET_QUEUE_HOB     *PetQueueHob;
  EFI_LIST_ENTRY        *Link;
  AMT_PET_QUEUE_NODE    *Node;
  EFI_PEI_HOB_POINTERS  Hob;

  HECI_PROTOCOL         *Heci;
  UINT32                MeStatus;
  EFI_STATUS            TempStatus;

  ///
  /// Try HECI state
  ///
  Status = gBS->LocateProtocol (
                  &gHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );
  if (EFI_ERROR (Status)) {
    return EFI_NOT_READY;
  }

  TempStatus = Heci->GetMeStatus (&MeStatus);
  ASSERT_EFI_ERROR (TempStatus);

  ///
  /// Only send ASF Push Progress code when ME is ready. Ignore FW Init Status.
  ///
  if (ME_STATUS_ME_STATE_ONLY (MeStatus) != ME_READY) {
    return EFI_NOT_READY;
  }
  ///
  /// Get PETQueueHob
  ///
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **) &PetQueueHob);

  while (TRUE) {
    PetQueueHob = GetNextGuidHob (&gAmtPetQueueHobGuid, PetQueueHob);
    if (PetQueueHob == NULL) {
      break;
    }
    ///
    /// Send message
    ///
    AmtReportStatusCode (PetQueueHob->Type, PetQueueHob->Value);

    ///
    /// Mark it as sent
    ///
    PetQueueHob->Type = (UINT32) -1;

    ///
    /// Need find next one
    ///
    Hob.Raw     = (VOID *) PetQueueHob;
    PetQueueHob = (AMT_PET_QUEUE_HOB *) GET_NEXT_HOB (Hob);
  }
  ///
  /// Send DXEQueue
  ///
  Link = (EFI_LIST_ENTRY *) GetFirstNode ((LIST_ENTRY *) &mAmtPetQueueProtocol->MessageList);

  while (!IsNull ((LIST_ENTRY *) &mAmtPetQueueProtocol->MessageList, (LIST_ENTRY *) Link)) {
    Node = AMT_PET_QUEUE_NODE_FROM_LINK (Link);

    ///
    /// Send message
    ///
    AmtReportStatusCode (Node->Type, Node->Value);

    ///
    /// Mark it as sent
    ///
    Node->Type = (UINT32) -1;

    Link = (EFI_LIST_ENTRY *) GetNextNode (
                                (LIST_ENTRY *) &mAmtPetQueueProtocol->MessageList,
                                (LIST_ENTRY *) &Node->Link
                                );
  }

  return EFI_SUCCESS;
}

/**
  This routine creates PET MessageQueue.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
AmtCreateMessageQueue (
  VOID
  )
{
  ///
  /// Create Queue for later usage
  ///
  mAmtPetQueueProtocol = &mAmtPetQueue;

  InitializeListHead ((LIST_ENTRY *) &mAmtPetQueueProtocol->MessageList);

  return EFI_SUCCESS;
}

/**
  This routine saves current ForcePush ErrorEvent to Variable, which will be sent again.

  @param[in] MessageType          ASF PET message type.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate necessary data structures
**/
EFI_STATUS
SaveForcePushErrorEvent (
  IN  ASF_FRAMEWORK_MESSAGE_TYPE  MessageType
  )
{
  EFI_STATUS                      Status;
  UINTN                           Size;
  ASF_FRAMEWORK_MESSAGE_TYPE      *Message;
  UINT32                          VarAttributes;

  ///
  /// Create PET queue variable
  ///
  Message = NULL;
  Size    = 0;
  Status = gRT->GetVariable (
                  AMT_FORCE_PUSH_PET_VARIABLE_NAME,
                  &gAmtForcePushPetVariableGuid,
                  &VarAttributes,
                  &Size,
                  NULL
                  );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    ///
    /// Get the exist message
    ///
    Message = AllocateZeroPool (Size + sizeof (MessageType));
    ASSERT (Message != NULL);
    if (Message == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = gRT->GetVariable (
                    AMT_FORCE_PUSH_PET_VARIABLE_NAME,
                    &gAmtForcePushPetVariableGuid,
                    &VarAttributes,
                    &Size,
                    Message
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// Fill new item
    ///
    *(ASF_FRAMEWORK_MESSAGE_TYPE *) ((UINTN) Message + Size) = MessageType;
    Size += sizeof (MessageType);
  } else if (Status == EFI_NOT_FOUND) {
    ///
    /// Create a new one
    ///
    Size    = sizeof (MessageType);
    Message = AllocateZeroPool (sizeof (MessageType));
    ASSERT (Message != NULL);
    if (Message == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    VarAttributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS;
    *Message = MessageType;
  } else {
    ASSERT (FALSE);
  }
  ///
  /// Set PET message to variable
  ///
  if (Message != NULL) {
    Status = gRT->SetVariable (
                    AMT_FORCE_PUSH_PET_VARIABLE_NAME,
                    &gAmtForcePushPetVariableGuid,
                    VarAttributes,
                    Size,
                    Message
                    );
    ASSERT_EFI_ERROR (Status);

    FreePool (Message);
  }

  return EFI_SUCCESS;
}

/**
  This routine converts Hob ForcePush ErrorEvent to Variable, which will be sent again.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
SaveForcePushErrorEventFromPeiToDxe (
  VOID
  )
{
  AMT_FORCE_PUSH_PET_HOB  *AmtForcePushPetHob;
  EFI_PEI_HOB_POINTERS    Hob;

  ///
  /// Find ASF ForcePush PET Hob
  ///
  EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **) &AmtForcePushPetHob);

  while (TRUE) {
    AmtForcePushPetHob = GetNextGuidHob (&gAmtForcePushPetHobGuid, AmtForcePushPetHob);
    if (AmtForcePushPetHob == NULL) {
      break;
    }

    SaveForcePushErrorEvent (AmtForcePushPetHob->MessageType);

    ///
    /// Need find next one
    ///
    Hob.Raw             = (VOID *) AmtForcePushPetHob;
    AmtForcePushPetHob  = (AMT_FORCE_PUSH_PET_HOB *) GET_NEXT_HOB (Hob);
  }

  return EFI_SUCCESS;
}
