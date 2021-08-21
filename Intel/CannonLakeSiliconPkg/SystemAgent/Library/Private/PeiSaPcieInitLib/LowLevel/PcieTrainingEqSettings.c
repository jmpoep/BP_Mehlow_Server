/**@file

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2017 Intel Corporation.

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
/*++ @file
  This file adds equalization setting support.

@copyright
  Copyright (c) 2013 - 2014 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
#include "SaPegLowLevel.h"

/**
  Get Full Swing value for EndPoint Transmitter

  @param[in]  This                        - Low level function table
  @param[in]  PegDev                      - Device Number
  @param[in]  Lane                        - Physical Lane Number
  @param[out] FullSwing                   - Full Swing value
**/
VOID
EFIAPI
GetLinkPartnerFullSwing (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDev,
  IN  UINT8                             Lane,
  OUT UINT8                             *FullSwing
  )
{
  UINT64  PegBaseAddress;
  UINT32  Data32;

  PegBaseAddress = 0;

  if (PegDev == SA_PEG0_DEV_NUM) {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
    if (Lane > 15) {
      ASSERT (Lane <= 15);
      Lane = 0;
    }
  }
  if (PegDev == SA_PEG3_DEV_NUM) {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG3_DEV_NUM, SA_PEG3_FUN_NUM, 0);
    if (Lane > 3) {
      ASSERT (Lane <= 3);
      Lane = 0;
    }
  }

  if (PegBaseAddress == 0) {
    ASSERT (PegBaseAddress != 0);
    return;
  }

  Data32 = BIT25 | BIT23 | (Lane << 19) | BIT18;
  PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_EQPH3_OFFSET, Data32);
  Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_EQPH3_OFFSET);
  PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_EQPH3_OFFSET, 0);

  *FullSwing = (Data32 >> 6) & 0x3F;

  return;
}

/**
  Sets the Phase 3 Hijack Equalization Coefficients

  @param[in]  This                        - Low level function table
  @param[in]  PegDev                      - Device Number
  @param[in]  Lane                        - Physical Lane Number
  @param[in]  PreCursor                   - Computed Pre-Cursor
  @param[in]  Cursor                      - Computed Cursor
  @param[in]  PostCursor                  - Computed Post-Cursor
**/
VOID
EFIAPI
SetPartnerTxCoefficients (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDev,
  IN  UINT8                             Lane,
  IN  UINT8                             *PreCursor,
  IN  UINT8                             *Cursor,
  IN  UINT8                             *PostCursor
  )
{
  UINT64  PegBaseAddress;
  UINT32  Data32;

  PegBaseAddress = 0;

  if (PegDev == SA_PEG0_DEV_NUM) {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
    if (Lane > 15) {
      ASSERT (Lane <= 15);
      return;
    }
  }
  if (PegDev == SA_PEG3_DEV_NUM) {
    PegBaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG3_DEV_NUM, SA_PEG3_FUN_NUM, 0);
    if (Lane > 3) {
      ASSERT (Lane <= 3);
      return;
    }
  }

  if (PegBaseAddress == 0) {
    ASSERT (PegBaseAddress != 0);
    return;
  }

  if ((*Cursor) > 63) {
    ASSERT ((*Cursor) <= 63);
    return;
  }
  if ((*PreCursor) > 63) {
    ASSERT ((*PreCursor) <= 63);
    return;
  }
  if ((*PostCursor) > 63) {
    ASSERT ((*PostCursor) <= 63);
    return;
  }

  Data32 = (Lane << 19) | BIT18 | (*Cursor << 12) | (*PreCursor << 6) | (*PostCursor);
  PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_EQPH3_OFFSET, Data32);
  PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_EQPH3_OFFSET, 0);
  return;
}
