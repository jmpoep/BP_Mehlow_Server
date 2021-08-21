/** @file
  Header file for PCH RESET Common Library.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2016 Intel Corporation.

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
#ifndef _PCH_RESET_COMMON_LIB_H_
#define _PCH_RESET_COMMON_LIB_H_
#include <Uefi/UefiSpec.h>
#include <Protocol/PchReset.h>
///
/// Private data structure definitions for the driver
///
#define PCH_RESET_SIGNATURE SIGNATURE_32 ('I', 'E', 'R', 'S')

typedef struct {
  UINT32              Signature;
  EFI_HANDLE          Handle;
  union {
    PCH_RESET_PPI       PchResetPpi;
    PCH_RESET_PROTOCOL  PchResetProtocol;
  }PchResetInterface;
  UINT32              PchPwrmBase;
  UINT16              PchAcpiBase;
  UINT64              PchPmcBase;
} PCH_RESET_INSTANCE;

//
// Function prototypes used by the Pch Reset ppi/protocol.
//
/**
  Initialize an Pch Reset ppi/protocol instance.

  @param[in] PchResetInstance     Pointer to PchResetInstance to initialize

  @retval EFI_SUCCESS             The protocol instance was properly initialized
  @exception EFI_UNSUPPORTED      The PCH is not supported by this module
**/
EFI_STATUS
PchResetConstructor (
  PCH_RESET_INSTANCE          *PchResetInstance
  );

/**
  Execute Pch Reset from the host controller.
  @param[in] PchResetInstance     Pointer to PchResetInstance to initialize
  @param[in] PchResetType         Pch Reset Types which includes ColdReset, WarmReset,
                                  ShutdownReset, GlobalReset

  @retval EFI_SUCCESS             Successfully completed.
  @retval EFI_INVALID_PARAMETER   If ResetType is invalid.
**/
EFI_STATUS
PchReset (
  IN PCH_RESET_INSTANCE *PchResetInstance,
  IN PCH_RESET_TYPE     PchResetType
  );
#endif
