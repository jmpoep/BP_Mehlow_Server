/** @file
  This files contains Pch services for RCs usage

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

#ifndef _PCH_RC_LIB_H_
#define _PCH_RC_LIB_H_

/**
  Type definitions for CPU STRAP OPERATION.
**/
typedef enum _CPU_STRAP_OPERATION
{
  GetCpuStrapSetData,
  SetCpuStrapSetData,
  LockCpuStrapSetData
} CPU_STRAP_OPERATION;

/**
  Type definitions for CPU EPOC OPERATION.
**/
typedef enum _CPU_EPOC_OPERATION
{
  GetCpuEpocData,
  SetCpuEpocData
} CPU_EPOC_OPERATION;

/**
  The function is used while doing CPU Only Reset, where PCH may be required
  to initialize strap data before soft reset.

  @param[in] Operation                  Get/Set Cpu Strap Set Data
  @param[in, out] CpuStrapSet           Cpu Strap Set Data

  @retval EFI_SUCCESS                   The function completed successfully.
  @exception EFI_UNSUPPORTED            The function is not supported.
**/
EFI_STATUS
EFIAPI
PchCpuStrapSet (
  IN      CPU_STRAP_OPERATION           Operation,
  IN OUT  UINT16                        *CpuStrapSet
  );

/**
  Set Early Power On Configuration setting for feature change.

  @param[in] Operation                  Get or set EPOC data
  @param[in, out] CpuEPOCSet            Cpu EPOC Data

  @retval EFI_SUCCESS                   The function completed successfully.
  @exception EFI_UNSUPPORTED            The function is not supported.
**/
EFI_STATUS
EFIAPI
PchCpuEpocSet (
  IN     CPU_EPOC_OPERATION       Operation,
  IN OUT  UINT32                    *CpuEpocSet
  );

#endif
