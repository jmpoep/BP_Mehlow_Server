/** @file
  SA PCIe Initialization Low Level functions

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

@par Specification
**/

#include "SaPegLowLevel.h"

/**
  This function gets the low level functions for the SA PCIe interface

  @param[in]  PCIE_PEI_PREMEM_CONFIG - PciePeiPreMemConfig
  @param[out] SaPciePrivateData      - Table of function calls for SA PEG

  @retval EFI_SUCCESS - Table of function calls returned successfully
**/
EFI_STATUS
GetSaPcieLowLevelFunctionCalls (
  IN  PCIE_PEI_PREMEM_CONFIG                  *PciePeiPreMemConfig,
  OUT PCIE_SI_LOW_LEVEL_FUNCTION_CALLS        *SaPcieLowLevelFunctionCalls
  )
{
  CPU_SKU  CpuSku;

  CpuSku = GetCpuSku ();

  if ((CpuSku == EnumCpuHalo) || (CpuSku == EnumCpuTrad)) {
    return GetSklPegLowLevelFunctionCalls (
             PciePeiPreMemConfig,
             SaPcieLowLevelFunctionCalls
             );
  }
  return EFI_UNSUPPORTED;
}
