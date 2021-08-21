/** @file
  The header file for Ptt SMM driver.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

#ifndef __PTT_SMM_H__
#define __PTT_SMM_H__

#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/FirmwareVolume2.h>

#include <Guid/MemoryOverwriteControl.h>
#include <IndustryStandard/Tpm2Acpi.h>
#include <Guid/Tcg2PhysicalPresenceData.h>
#include <Library/Tcg2PhysicalPresenceLib.h>

//
// Below definition is generic, but NOT in GreenH
//
#include <Protocol/SmmVariable.h>
//
// Below definition is chipset specific
//
#include <MeDataHob.h>
#include <CpuRegs.h>
#include <Library/CpuPlatformLib.h>
#include <PttPtpRegs.h>
#include <Private/Protocol/MeNvsArea.h>
#include <Private/PttNvsAreaDef.h>

//
// Below definition is driver specific
//

#pragma pack(1)
typedef struct {
  UINT8                  OpRegionOp;
  UINT32                 NameString;
  UINT8                  RegionSpace;
  UINT8                  DWordPrefix;
  UINT32                 RegionOffset;
  UINT8                  BytePrefix;
  UINT8                  RegionLen;
} AML_OP_REGION_32_8;
#pragma pack()

//
// The definition for TCG physical presence ACPI function
//
#define ACPI_FUNCTION_GET_PHYSICAL_PRESENCE_INTERFACE_VERSION      1
#define ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS                       2
#define ACPI_FUNCTION_GET_PENDING_REQUEST_BY_OS                    3
#define ACPI_FUNCTION_GET_PLATFORM_ACTION_TO_TRANSITION_TO_BIOS    4
#define ACPI_FUNCTION_RETURN_REQUEST_RESPONSE_TO_OS                5
#define ACPI_FUNCTION_SUBMIT_PREFERRED_USER_LANGUAGE               6
#define ACPI_FUNCTION_SUBMIT_REQUEST_TO_BIOS_2                     7
#define ACPI_FUNCTION_GET_USER_CONFIRMATION_STATUS_FOR_REQUEST     8

//
// The return code for Get User Confirmation Status for Operation
//
#define PP_REQUEST_NOT_IMPLEMENTED                                 0
#define PP_REQUEST_BIOS_ONLY                                       1
#define PP_REQUEST_BLOCKED                                         2
#define PP_REQUEST_ALLOWED_AND_PPUSER_REQUIRED                     3
#define PP_REQUEST_ALLOWED_AND_PPUSER_NOT_REQUIRED                 4

//
// The return code for Sumbit TPM Request to Pre-OS Environment
// and Sumbit TPM Request to Pre-OS Environment 2
//
#define PP_SUBMIT_REQUEST_SUCCESS                                  0
#define PP_SUBMIT_REQUEST_NOT_IMPLEMENTED                          1
#define PP_SUBMIT_REQUEST_GENERAL_FAILURE                          2
#define PP_SUBMIT_REQUEST_BLOCKED_BY_BIOS_SETTINGS                 3

#define PP_RETURN_TPM_OPERATION_RESPONSE_SUCCESS                   0
#define PP_RETURN_TPM_OPERATION_RESPONSE_FAILURE                   1

//
// The definition for TCG MOR
//
#define ACPI_FUNCTION_DSM_MEMORY_CLEAR_INTERFACE                   1
#define ACPI_FUNCTION_PTS_CLEAR_MOR_BIT                            2

//
// The return code for Memory Clear Interface Functions
//
#define MOR_REQUEST_SUCCESS                                        0
#define MOR_REQUEST_GENERAL_FAILURE                                1

//
// Below definition should be in platorm scope
//
// @todo: Use policy to input these data...
#ifndef EFI_TPM2_PP_SW_SMI
#define EFI_TPM2_PP_SW_SMI     0x9E
#endif
#ifndef EFI_TPM2_MOR_SW_SMI
#define EFI_TPM2_MOR_SW_SMI    0x9F
#endif

//
// Physical Presence Interface Version supported by Platform
//
#define PHYSICAL_PRESENCE_VERSION_TAG                              "$PV"
#define PHYSICAL_PRESENCE_VERSION_SIZE                             4

#endif  // __PTT_SMM_H__
