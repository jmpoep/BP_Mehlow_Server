/** @file
  DCI private library.
  All functions from this library are available in PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2018 Intel Corporation.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsDci.h>

/**
  Return DCI Debug Enabled status.

  @retval TRUE   DCI Debug is enabled
  @retval FALSE  DCI Debug is disabled
**/
BOOLEAN
IsDciDebugEnabled (
  VOID
  )
{
  UINT16 Data;

  Data = PchPcrRead16 (PID_DCI, R_DCI_PCR_ECTRL);

  if (Data & B_DCI_PCR_ECTRL_DBG_EN) {
    DEBUG ((DEBUG_INFO, "DCI Debug Enabled\n"));
    return TRUE;
  }

  DEBUG ((DEBUG_INFO, "DCI Debug Disabled\n"));
  return FALSE;
}

/**
  Return USB2 DbC enable status.

  @retval TRUE   USB2 DbC is enabled
  @retval FALSE  USB2 DbC is disabled
**/
BOOLEAN
IsUsb2DbcDebugEnabled (
  VOID
  )
{
  UINT16 Data;

  Data  = PchPcrRead16 (PID_DCI, R_DCI_PCR_ECTRL);
  Data &= (B_DCI_PCR_ECTRL_DBG_EN | B_DCI_PCR_ECTRL_USB2DBCEN);

  // If ECTRL[8, 5] = 1b, 1b, then USB2 DbC is enabled.
  if (Data == (UINT16)(B_DCI_PCR_ECTRL_DBG_EN | B_DCI_PCR_ECTRL_USB2DBCEN)) {
    DEBUG ((DEBUG_INFO, "USB2 DbC is enabled\n"));
    return TRUE;
  }

  DEBUG ((DEBUG_INFO, "USB2 DbC is disabled\n"));
  return FALSE;
}

/**
  Return DbC connected status.

  @retval TRUE   DbC is connected
  @retval FALSE  DbC is disconnected
**/
BOOLEAN
IsDbcConnected (
  VOID
  )
{
  return ((PchPcrRead32 (PID_DCI, R_DCI_PCR_EARBCTRL) & B_DCI_PCR_EARBCTRL_ARB_GNT_DBC) != 0);
}

/**
  Return DCI OOB connected status.

  @retval TRUE   DCI OOB is connected
  @retval FALSE  DCI OOB is disconnected
**/
BOOLEAN
IsDciOobConnected (
  VOID
  )
{
  return ((PchPcrRead32 (PID_DCI, R_DCI_PCR_EARBCTRL) & B_DCI_PCR_EARBCTRL_ARB_GNT_DCI_OOB) != 0);
}