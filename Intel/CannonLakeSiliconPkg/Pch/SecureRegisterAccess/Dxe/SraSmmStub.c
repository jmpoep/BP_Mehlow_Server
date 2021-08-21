/** @file
  SRA STUB SMM Driver.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2018 Intel Corporation.

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

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/SmmMemLib.h>
#include <Guid/EventGroup.h>
#include "SraSmmCommunication.h"
#include <Library/CpuPlatformLib.h>
#include <Library/PchSbiAccessLib.h>
#include <Library/PmcLib.h>
#include <Private/Library/SmmPchPrivateLib.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsPmc.h>

EFI_HANDLE   mSmmSraHandle;

/**
  Execute the SBI transaction.

  @param[in out]  SraUsb2ReadWrite  SRA USB2 READ WRITE structure
  @param[in]      IsReadOpcode      If ReadOpcode is true

  @retval         EFI_STATUS
**/
STATIC
EFI_STATUS
SraUsb2Execution (
  SMM_SRA_USB2_READ_WRITE   *SraUsb2ReadWrite,
  BOOLEAN                   IsReadOpcode
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;
  UINT8       Response;
  UINT32      PwrmBase;

  if (IsBiosGuardEnabled ()) {
    return EFI_UNSUPPORTED;
  }

  PchSetInSmmSts ();
  PwrmBase = PmcGetPwrmBase ();
  MmioAnd32 (PwrmBase + R_PMC_PWRM_CFG, (UINT32)~B_PMC_PWRM_CFG_ALLOW_USB2_CORE_PG);
  MmioRead32 (PwrmBase + R_PMC_PWRM_CFG);

  Data32 = SraUsb2ReadWrite->Data32;
  Response = 0;
  Status = PchSbiExecution (
             PID_USB2,
             SraUsb2ReadWrite->Offset,
             (IsReadOpcode) ? PrivateControlRead : PrivateControlWrite,
             (IsReadOpcode) ? FALSE : TRUE,
             &Data32,
             &Response
             );

  SraUsb2ReadWrite->Data32   = Data32;
  SraUsb2ReadWrite->Response = Response;

  MmioOr32 (PwrmBase + R_PMC_PWRM_CFG, B_PMC_PWRM_CFG_ALLOW_USB2_CORE_PG);
  MmioRead32 (PwrmBase + R_PMC_PWRM_CFG);
  PchClearInSmmSts ();

  return Status;
}

/**
  Communication service SMI Handler entry.

  @param[in]     DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]     RegisterContext Points to an optional handler context which was specified when the
                                 handler was registered.
  @param[in, out] CommBuffer     A pointer to a collection of data in memory that will be conveyed
                                 from a non-SMM environment into an SMM environment.
  @param[in, out] CommBufferSize The size of the CommBuffer.

  @retval EFI_SUCCESS            Initialization complete.
**/
EFI_STATUS
EFIAPI
SmmSraHandler (
  IN     EFI_HANDLE                                DispatchHandle,
  IN     CONST VOID                                *RegisterContext,
  IN OUT VOID                                      *CommBuffer,
  IN OUT UINTN                                     *CommBufferSize
  )
{
  EFI_STATUS                                       Status;
  SMM_SRA_COMMUNICATE_FUNCTION_HEADER              *SmmSraFunctionHeader;
  UINTN                                            CommBufferPayloadSize;
  UINTN                                            TempCommBufferSize;
  SMM_SRA_USB2_READ_WRITE                           *SmmSraUsb2ReadWrite;

  //
  // If input is invalid, stop processing this SMI
  //
  if ((CommBuffer == NULL) || (CommBufferSize == NULL)) {
    return EFI_SUCCESS;
  }

  TempCommBufferSize = *CommBufferSize;

  if (TempCommBufferSize < SMM_SRA_COMMUNICATE_HEADER_SIZE) {
    DEBUG ((DEBUG_ERROR, "SmmSraHandler: SMM communication buffer size invalid!\n"));
    return EFI_SUCCESS;
  }
  CommBufferPayloadSize = TempCommBufferSize - SMM_SRA_COMMUNICATE_HEADER_SIZE;

  if (!SmmIsBufferOutsideSmmValid ((UINTN)CommBuffer, TempCommBufferSize)) {
    DEBUG ((DEBUG_ERROR, "SmmSraHandler: SMM communication buffer in SMRAM or overflow!\n"));
    return EFI_SUCCESS;
  }

  SmmSraFunctionHeader = (SMM_SRA_COMMUNICATE_FUNCTION_HEADER *)CommBuffer;

  switch (SmmSraFunctionHeader->Function) {
    case SRA_FUNCTION_USB2_READ:
    case SRA_FUNCTION_USB2_WRITE:
      if (CommBufferPayloadSize < sizeof (SMM_SRA_USB2_READ_WRITE)) {
        DEBUG ((DEBUG_ERROR, "SRA_FUNCTION_USB2_READ_WRITE: SMM communication buffer size invalid!\n"));
        return EFI_SUCCESS;
      }
      SmmSraUsb2ReadWrite = (SMM_SRA_USB2_READ_WRITE *) SmmSraFunctionHeader->Data;
      Status = SraUsb2Execution (SmmSraUsb2ReadWrite, SmmSraFunctionHeader->Function == SRA_FUNCTION_USB2_READ);
      break;

    default:
      Status = EFI_UNSUPPORTED;
  }

  SmmSraFunctionHeader->ReturnStatus = Status;

  return EFI_SUCCESS;
}

/**
  SRA SMM STUB Driver Entry Point.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
**/
EFI_STATUS
EFIAPI
InstallSraSmmStub (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS                      Status;

  DEBUG ((DEBUG_INFO, "InstallSraSmmStub() Start\n"));

  //
  // Register SMM SRA SMI handler
  //
  Status = gSmst->SmiHandlerRegister (SmmSraHandler, &gPchSraProtocolGuid, &mSmmSraHandle);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "InstallSraSmmStub() End\n"));

  return EFI_SUCCESS;
}
