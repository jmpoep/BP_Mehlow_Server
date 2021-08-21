/** @file
  Main implementation source file for the support of caching the "Setup" variable.

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

  This file contains 'Framework Code' and is licensed as such under the terms
  of your license agreement with Intel or your vendor. This file may not be
  modified, except as allowed by additional terms of your license agreement.

@par Specification Reference:
**/

#include <Library/BaseLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <SystemAgent/Include/SaAccess.h>
#include <SystemAgent/Include/Register/SaRegsHostBridge.h>

#ifdef CPU_CFL
#define BIOS_POST_CODE_PCU_REG            0x00005428
#else
#define BIOS_POST_CODE_PCU_REG            0x00005824
#endif

/**
  Write the postcode value into a scratch pad register

  @param[in]  Postcode - The postcode value to save to scratchpad register

  @retval     None
**/
VOID
SetPostcodeToScratchpadReg (
  UINT32 Postcode
  )
{
  UINT32     MchBar;

  MchBar = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MCHBAR)) &~BIT0;
  DEBUG ((DEBUG_INFO, "Port80 forward FSP POSTCODE=<%02x>\n", Postcode));
  MmioWrite32 ((MchBar + BIOS_POST_CODE_PCU_REG), Postcode);
  return;
}

