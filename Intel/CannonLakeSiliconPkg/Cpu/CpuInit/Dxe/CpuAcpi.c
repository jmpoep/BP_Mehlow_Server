/** @file
  Cpu driver, which initializes ACPI

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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

#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/CpuNvsArea.h>
#include "CpuInitDxe.h"

GLOBAL_REMOVE_IF_UNREFERENCED CPU_NVS_AREA_PROTOCOL CpuNvsAreaProtocol;

/**
  Initialize Cpu Nvs Area Protocol

  @param[in] ImageHandle - Image handle of the loaded driver

  @retval EFI_SUCCESS           - thread can be successfully created
**/
EFI_STATUS
EFIAPI
CpuAcpiInit (
  IN EFI_HANDLE       ImageHandle
  )
{
  EFI_STATUS        Status;

  ///
  /// Get Cpu Nvs protocol pointer
  ///
  CpuNvsAreaProtocol.Area = (CPU_NVS_AREA *) (UINTN) mCpuInitDataHob->CpuGnvsPointer;
  CpuNvsAreaProtocol.Area->DtsIoTrapAddress  = 0x810;
  CpuNvsAreaProtocol.Area->DtsIoTrapLength   = 4;
  CpuNvsAreaProtocol.Area->BiosGuardIoTrapAddress = 0x1000;
  CpuNvsAreaProtocol.Area->BiosGuardIoTrapLength  = 4;
  CpuNvsAreaProtocol.Area->DtsAcpiEnable  = 0;

  ///
  /// Install Cpu Power management GlobalNVS Area protocol
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gCpuNvsAreaProtocolGuid,
                  &CpuNvsAreaProtocol,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}
