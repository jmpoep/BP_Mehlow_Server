/** @file
  Include file for ASF Driver.

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
#ifndef _ALERT_STANDARD_FORMAT_DXE_H_
#define _ALERT_STANDARD_FORMAT_DXE_H_

#include <PiDxe.h>
#include <Amt.h>
#include <Pi/PiStatusCode.h>
#include <IndustryStandard/AlertStandardFormatTable.h>
#include <Guid/ConsoleOutDevice.h>
#include <Guid/HobList.h>
#include <Library/HobLib.h>
#include <Library/AslUpdateLib.h>
#include <Library/DxeAmtLib.h>
#include <Library/DxeMeLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/TimerLib.h>
#include <Library/PostCodeLib.h>
#include <Protocol/Runtime.h>
#include <Protocol/ReportStatusCodeHandler.h>
#include <Protocol/AlertStandardFormat.h>
#include <Private/Library/AlertStandardFormatLib.h>
#include <AlertStandardFormat.h>
#include <AmtForcePushPetPolicy.h>
#include <AmtForcePushPetHob.h>
#include <AmtPetQueueHob.h>

#define AMT_FORCE_PUSH_PET_VARIABLE_NAME  L"AmtForcePushErrorPET"
#define FW_INIT_DONE_TIMEOUT_VALUE        15
#define ALERT_STANDARD_FORMAT_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('a', 's', 'f', 'd')

///
/// Declare a local instance structure for this driver
///
typedef struct _ALERT_STANDARD_FORMAT_INSTANCE {
  UINTN                               Signature;
  EFI_HANDLE                          Handle;

  ///
  /// Published interface
  ///
  ALERT_STANDARD_FORMAT_PROTOCOL      AlertStandardFormat;
} ALERT_STANDARD_FORMAT_INSTANCE;

#define AMT_PET_QUEUE_NODE_SIGNATURE      SIGNATURE_32 ('A', 'M', 'T', 'Q')

typedef struct {
  UINT32                Signature;
  EFI_LIST_ENTRY        Link;
  EFI_STATUS_CODE_VALUE Value;
  EFI_STATUS_CODE_TYPE  Type;
} AMT_PET_QUEUE_NODE;

#define AMT_PET_QUEUE_NODE_FROM_LINK(_node) CR (_node, AMT_PET_QUEUE_NODE, Link, AMT_PET_QUEUE_NODE_SIGNATURE)

typedef struct {
  EFI_LIST_ENTRY  MessageList;
} AMT_PET_QUEUE_PROTOCOL;

//
// Prototypes
//

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  This routine returns ForcePushPetPolicy information.

  @retval AMT_FORCE_PUSH_PET_POLICY_HOB   ForcePushPetPolicy information.
**/
AMT_FORCE_PUSH_PET_POLICY_HOB *
GetForcePushPetPolicy (
  VOID
  );

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
  );

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
  );

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
  );

/**
  This routine sends PET message in MessageQueue.

  @param[in] PeiServices          PeiServices pointer.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_NOT_READY           No controller
**/
EFI_STATUS
SendPetMessageInQueue (
  VOID
  );

/**
  This routine creates PET MessageQueue.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
AmtCreateMessageQueue (
  VOID
  );

/**
  This routine saves current ForcePush ErrorEvent to Variable, which will be sent again.

  @param[in] MessageType          ASF PET message type.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate necessary data structures
**/
EFI_STATUS
SaveForcePushErrorEvent (
  IN  ASF_FRAMEWORK_MESSAGE_TYPE      MessageType
  );

/**
  This routine converts Hob ForcePush ErrorEvent to Variable, which will be sent again.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
SaveForcePushErrorEventFromPeiToDxe (
  VOID
  );
#endif
