/** @file
  SRA DXE Driver.

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
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/SmmCommunication.h>
#include <Protocol/PchSecureRegisterAccess.h>
#include "SraSmmCommunication.h"

EFI_SMM_COMMUNICATION_PROTOCOL     *mSmmCommunication        = NULL;

/**
  Initialize the communicate buffer using DataSize and Function number.

  @param[out]      CommunicateBuffer The communicate buffer. Caller should free it after use.
  @param[out]      DataPtr           Points to the data in the communicate buffer. Caller should not free it.
  @param[in]       DataSize          The payload size.
  @param[in]       Function          The function number used to initialize the communicate header.
**/
STATIC
VOID
InitCommunicateBuffer (
  OUT     VOID                              **CommunicateBuffer,
  OUT     VOID                              **DataPtr,
  IN      UINTN                             DataSize,
  IN      UINTN                             Function
  )
{
  EFI_SMM_COMMUNICATE_HEADER                *SmmCommunicateHeader;
  SMM_SRA_COMMUNICATE_FUNCTION_HEADER       *SmmSraFunctionHeader;

  //
  // The whole buffer size: SMM_COMMUNICATE_HEADER_SIZE + SMM_SRA_COMMUNICATE_HEADER_SIZE + DataSize.
  //
  SmmCommunicateHeader = AllocateRuntimeZeroPool (SMM_COMMUNICATE_HEADER_SIZE + SMM_SRA_COMMUNICATE_HEADER_SIZE + DataSize);
  if (SmmCommunicateHeader == NULL) {
    ASSERT (FALSE);
    return;
  }
  //
  // Prepare data buffer.
  //
  CopyGuid (&SmmCommunicateHeader->HeaderGuid, &gPchSraProtocolGuid);
  SmmCommunicateHeader->MessageLength = SMM_SRA_COMMUNICATE_HEADER_SIZE + DataSize;

  SmmSraFunctionHeader = (SMM_SRA_COMMUNICATE_FUNCTION_HEADER *) SmmCommunicateHeader->Data;
  SmmSraFunctionHeader->Function = Function;
  SmmSraFunctionHeader->ReturnStatus = EFI_NOT_READY;

  *CommunicateBuffer = SmmCommunicateHeader;
  if (DataPtr != NULL) {
    *DataPtr = SmmSraFunctionHeader->Data;
  }
}

/**
  Finish the communicate buffer.

  @param[in,out]      CommunicateBuffer The communicate buffer. Caller should free it after use.
**/
STATIC
VOID
FinishCommunicateBuffer (
  IN OUT  VOID                              **CommunicateBuffer
  )
{
  FreePool (*CommunicateBuffer);
  *CommunicateBuffer = NULL;
}

/**
  Send the data in communicate buffer to SMI handler and get response.

  @param[in, out]  SmmCommunicateHeader    The communicate buffer.
  @param[in]       DataSize                The payload size.

  @return          The status from SMM communication
**/
EFI_STATUS
SendCommunicateBuffer (
  IN OUT  EFI_SMM_COMMUNICATE_HEADER        *SmmCommunicateHeader,
  IN      UINTN                             DataSize
  )
{
  EFI_STATUS                                Status;
  UINTN                                     CommSize;
  SMM_SRA_COMMUNICATE_FUNCTION_HEADER       *SmmSraFunctionHeader;

  CommSize = SMM_COMMUNICATE_HEADER_SIZE + SMM_SRA_COMMUNICATE_HEADER_SIZE + DataSize;
  Status = mSmmCommunication->Communicate (mSmmCommunication, SmmCommunicateHeader, &CommSize);
  ASSERT_EFI_ERROR (Status);

  SmmSraFunctionHeader = (SMM_SRA_COMMUNICATE_FUNCTION_HEADER *) SmmCommunicateHeader->Data;
  return  SmmSraFunctionHeader->ReturnStatus;
}


/**
  Execute USB2 read.

  @param[in] Offset                     Offset of the SBI message
  @param[out] Data32                    Read data
  @param[out] Response                  Response

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_DEVICE_ERROR              Transaction fail
  @retval EFI_INVALID_PARAMETER         Invalid parameter
  @retval EFI_TIMEOUT                   Timeout while waiting for response
**/
EFI_STATUS
EFIAPI
SraUsb2Read (
  IN     UINT64                         Offset,
  OUT    UINT32                         *Data32,
  OUT    UINT8                          *Response
  )
{
  EFI_STATUS                            Status;
  UINTN                                 PayloadSize;
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommunicateHeader;
  SMM_SRA_USB2_READ_WRITE               *SmmSraUsb2ReadWrite;

  PayloadSize = sizeof (SMM_SRA_USB2_READ_WRITE);

  //
  // Initialize the communicate buffer.
  //
  InitCommunicateBuffer ((VOID **)&SmmCommunicateHeader, (VOID **)&SmmSraUsb2ReadWrite, PayloadSize, SRA_FUNCTION_USB2_READ);

  //
  // Fill input data
  //
  SmmSraUsb2ReadWrite->Offset = Offset;

  //
  // Send data to SMM.
  //
  Status = SendCommunicateBuffer (SmmCommunicateHeader, PayloadSize);
  *Data32 = SmmSraUsb2ReadWrite->Data32;
  *Response = SmmSraUsb2ReadWrite->Response;
  FinishCommunicateBuffer ((VOID **)&SmmCommunicateHeader);

  //
  // Check output data
  //
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SraUsb2Read - Communication : %r\n", Status));
  }

  return Status;
}

/**
  Execute USB2 write.

  @param[in] Offset                     Offset of the SBI message
  @param[in] Data32                     Write data
  @param[out] Response                  Response

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_DEVICE_ERROR              Transaction fail
  @retval EFI_INVALID_PARAMETER         Invalid parameter
  @retval EFI_TIMEOUT                   Timeout while waiting for response
**/
EFI_STATUS
EFIAPI
SraUsb2Write (
  IN     UINT64                         Offset,
  IN     UINT32                         Data32,
  OUT    UINT8                          *Response
  )
{
  EFI_STATUS                            Status;
  UINTN                                 PayloadSize;
  EFI_SMM_COMMUNICATE_HEADER            *SmmCommunicateHeader;
  SMM_SRA_USB2_READ_WRITE               *SmmSraUsb2ReadWrite;

  PayloadSize = sizeof (SMM_SRA_USB2_READ_WRITE);

  //
  // Initialize the communicate buffer.
  //
  InitCommunicateBuffer ((VOID **)&SmmCommunicateHeader, (VOID **)&SmmSraUsb2ReadWrite, PayloadSize, SRA_FUNCTION_USB2_WRITE);

  //
  // Fill input data
  //
  SmmSraUsb2ReadWrite->Offset = Offset;
  SmmSraUsb2ReadWrite->Data32 = Data32;

  //
  // Send data to SMM.
  //
  Status = SendCommunicateBuffer (SmmCommunicateHeader, PayloadSize);
  *Response = SmmSraUsb2ReadWrite->Response;
  FinishCommunicateBuffer ((VOID **)&SmmCommunicateHeader);

  //
  // Check output data
  //
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SraUsb2Write - Communication : %r\n", Status));
  }

  return Status;
}

PCH_SRA_PROTOCOL  mSraProtocol = {
  PCH_SRA_REVISION,
  SraUsb2Read,
  SraUsb2Write
};

/**
  SRA DXE Driver Entry Point.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
  @exception EFI_UNSUPPORTED      The chipset is unsupported by this driver.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver.
  @retval EFI_DEVICE_ERROR        Device error, driver exits abnormally.
**/
EFI_STATUS
EFIAPI
InstallSraSmmDxe (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS                      Status;
  EFI_HANDLE                      Handle;

  DEBUG ((DEBUG_INFO, "InstallSraSmmDxe() Start\n"));

  Status = gBS->LocateProtocol (&gEfiSmmCommunicationProtocolGuid, NULL, (VOID **) &mSmmCommunication);
  ASSERT_EFI_ERROR (Status);

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gPchSraProtocolGuid,
                  &mSraProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "InstallSraSmmDxe() End\n"));

  return Status;
}
