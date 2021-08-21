/** @file
  Prototype of the SiPolicyLib library.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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
#ifndef _SI_POLICY_LIB_H_
#define _SI_POLICY_LIB_H_

#include <Ppi/SiPolicy.h>

/**
  Print whole SI_PREMEM_POLICY_PPI and serial out.

  @param[in] SiPreMemPolicyPpi          The RC PREMEM Policy PPI instance
**/
VOID
EFIAPI
SiPreMemPrintPolicyPpi (
  IN  SI_PREMEM_POLICY_PPI          *SiPreMemPolicyPpi
  );

/**
  Print whole SI_POLICY_PPI and serial out.

  @param[in] SiPolicyPpi          The RC Policy PPI instance
**/
VOID
EFIAPI
SiPrintPolicyPpi (
  IN  SI_POLICY_PPI          *SiPolicyPpi
  );

/**
  SiCreatePreMemConfigBlocks creates the config blocksg of Silicon Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] SiPreMemPolicyPpi   The pointer to get Silicon PREMEM Policy PPI instance

  @retval EFI_SUCCESS             The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiCreatePreMemConfigBlocks (
  OUT  SI_PREMEM_POLICY_PPI         **SiPreMemPolicyPpi
  );

/**
  SiCreateConfigBlocks creates the config blocksg of Silicon Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] SiPolicyPpi         The pointer to get Silicon Policy PPI instance

  @retval EFI_SUCCESS             The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiCreateConfigBlocks (
  OUT  SI_POLICY_PPI         **SiPolicyPpi
  );

/**
  SiPreMemInstallPolicyPpi installs SiPreMemPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiPreMemPolicyPpi   The pointer to Silicon PREMEM Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiPreMemInstallPolicyPpi (
  IN  SI_PREMEM_POLICY_PPI          *SiPreMemPolicyPpi
  );

/**
  SiInstallPolicyPpi installs SiPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiPolicyPpi         The pointer to Silicon Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiInstallPolicyPpi (
  IN  SI_POLICY_PPI          *SiPolicyPpi
  );

/**
  Print out all silicon policy information.

  @param[in] SiPolicyPpi         The pointer to Silicon Policy PPI instance

  @retval none
**/
VOID
DumpSiPolicy (
  IN  SI_POLICY_PPI *SiPolicyPpi
  );

/**
  Calculate total trace buffer size and make it power of two, eliminate the total size within 512MB
  Please ensure CPU and PCH trace hub policy are configured before calling.

  @param[in] SiPreMemPolicyPpi               The pointer to get Silicon Policy PPI instance

  @retval UINT32                             Total size of trace buffers
**/
UINT32
TraceHubCalculateTotalBufferSize (
  IN SI_PREMEM_POLICY_PPI         *SiPreMemPolicyPpi
  );
#endif // _SI_PREMEM_POLICY_LIB_H_
