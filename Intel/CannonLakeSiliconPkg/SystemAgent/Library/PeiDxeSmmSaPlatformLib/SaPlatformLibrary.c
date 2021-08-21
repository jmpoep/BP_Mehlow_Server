/** @file
  SA Platform Lib implementation.

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
#include "SaPlatformLibrary.h"
#include <Library/PciSegmentLib.h>
#include <SaRegs.h>
#include <Library/CpuPlatformLib.h>

/**
  Determine if PCH Link is DMI/OPI

  @param[in] CpuModel             CPU model

  @retval TRUE                    DMI
  @retval FALSE                   OPI
**/
BOOLEAN
IsPchLinkDmi (
  IN CPU_FAMILY  CpuModel
  )
{
  if ((CpuModel == EnumCpuCflDtHalo) || (CpuModel == EnumCpuCnlDtHalo)) {
    return TRUE; // DMI
  }
  return FALSE;  // OPI
}


/**
  Returns the number of DMI lanes for current CPU

  @retval UINT8
**/
UINT8
GetMaxDmiLanes (
  )
{
#ifdef CPU_CFL
    return SA_DMI_CFL_MAX_LANE;
#else
  if (GetCpuFamily() == EnumCpuCnlDtHalo) { // For CNL- DT and Halo Series only,
    return SA_DMI_CNL_DT_HALO_MAX_LANE;
  } else { // For all non- CNL DT and Halo Series
    return SA_DMI_NON_CNL_DT_HALO_MAX_LANE;
  }
#endif
}


/**
  Returns the number of DMI bundles for current CPU

  @retval UINT8
**/
UINT8
GetMaxDmiBundles (
  )
{
#ifdef CPU_CFL
    return SA_DMI_CFL_MAX_BUNDLE;
#else
  if (GetCpuFamily() == EnumCpuCnlDtHalo) { // For CNL- DT and Halo Series only,
    return SA_DMI_CNL_DT_HALO_MAX_BUNDLE;
  } else { // For all non- CNL DT and Halo Series
    return SA_DMI_NON_CNL_DT_HALO_MAX_BUNDLE;
  }
#endif
}


/**
  Returns the function numbers for current CPU

  @retval UINT8
**/
UINT8
GetMaxPegFuncs (
  )
{
  if (GetCpuFamily() == EnumCpuCnlDtHalo) { // For CNL- DT and Halo Series only,
    return SA_PEG_CNL_H_MAX_FUN;
  } else { // For all non- CNL DT and Halo Series
    return SA_PEG_NON_CNL_H_MAX_FUN;
  }
}


/**
  Returns the number of PEG lanes for current CPU

  @retval UINT8
**/
UINT8
GetMaxPegLanes (
  )
{
  if (GetCpuFamily() == EnumCpuCnlDtHalo) { // For CNL- DT and Halo Series only,
    return SA_PEG_CNL_H_MAX_LANE;
  } else { // For all non- CNL DT and Halo Series
    return SA_PEG_NON_CNL_H_MAX_LANE;
  }
}


/**
  Returns the number of PEG bundles for current CPU

  @retval UINT8
**/
UINT8
GetMaxPegBundles (
  )
{
  if (GetCpuFamily() == EnumCpuCnlDtHalo) { // For CNL- DT and Halo Series only,
    return  SA_PEG_CNL_H_MAX_BUNDLE;
  } else { // For all non- CNL DT and Halo Series
    return  SA_PEG_NON_CNL_H_MAX_BUNDLE;
  }
}

/**
  Checks if PEG port is present

  @retval TRUE     PEG is presented
  @retval FALSE    PEG is not presented
**/
BOOLEAN
IsPegPresent (
  VOID
  )
{
  UINT64  PegBaseAddress;

  PegBaseAddress  = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG_DEV_NUM, 0, 0);
  if (PciSegmentRead16 (PegBaseAddress) != 0xFFFF) {
    return TRUE;
  }
  return FALSE;
}
