/** @file
  This file contains functions to get CPGC Offsets
  used memory training.

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

@par Specification Reference:
**/
#include "MrcInterface.h"
#include "MrcCpgcOffsets.h"
#include "McAddress.h"

/**
  This returns the read or write offset for the Cacheline Mux Pattern Buffer.

  @param[in]  MrcData   - Pointer to global MRC data.
  @param[in]  Direction - Specifies which traffic Mux to program.
  @param[in]  Index     - 0-based index specifying which Mux to program.

  @retval - MCHBAR Register Offset.
*/
UINT32
MrcGetClMuxPbOffset (
  IN  MrcParameters   *const  MrcData,
  IN  MRC_CL_MUX_DIR  const   Direction,
  IN  UINT32          const   Index
  )
{
  UINT32  Offset;

  if (Direction == MrcClMuxRd) {
    Offset = REUT_PG_PAT_CL_MUX_RD_PB_0_CNL_REG;
  } else {
    Offset = REUT_PG_PAT_CL_MUX_WR_PB_0_CNL_REG;
  }

  // CNL have same offset delta's for both Rd and Wr.
  Offset += ((REUT_PG_PAT_CL_MUX_RD_PB_1_CNL_REG - REUT_PG_PAT_CL_MUX_RD_PB_0_CNL_REG) * Index);

  return Offset;
}

/**
  This function returns the offset of the CADB Pattern control.

  @param[in]  MrcData - Pointer to global MRC data.
  @param[in]  Channel - 0-based index specifying which channel control to program.

  @retval - MCHBAR Register Offset.
**/
UINT32
MrcGetReutChPatCadbCtlOffset (
  IN  MrcParameters *const  MrcData,
  IN  UINT32        const   Channel
  )
{
  UINT32  Offset;

  Offset  = REUT_CH_PAT_CADB_CTRL_0_CNL_REG;
  Offset += ((REUT_CH_PAT_CADB_CTRL_1_CNL_REG - REUT_CH_PAT_CADB_CTRL_0_CNL_REG) * Channel);

  return Offset;
}

/**
  This function returns the offset of the error control register.

  @param[in]  MrcData - Pointer to global MRC data.
  @param[in]  Channel - 0-based index specifying which channel control to program.

  @retval - MCHBAR Register Offset.
**/
UINT32
MrcGetTestErrCtlOffset (
  IN  MrcParameters *const  MrcData,
  IN  UINT32                Channel
  )
{
  UINT32 Offset;

  Offset  = REUT_CH_ERR_CTL_0_CNL_REG;
  Offset += (REUT_CH_ERR_CTL_1_CNL_REG - REUT_CH_ERR_CTL_0_CNL_REG) * Channel;

  return Offset;
}

/**
  This function returns the offset to the Pattern Inversion control register.

  @param[in]  MrcData - Pointer to global MRC data.
  @param[in]  Channel - 0-based index specifying which channel control to program.

  @retval - MCHBAR Register Offset.
**/
UINT32
MrcGetPatInvOffset (
  IN  MrcParameters *const  MrcData,
  IN  UINT32        const   Channel
  )
{
  UINT32  Offset;

  Offset = REUT_PG_PAT_DATA_INV_CNL_REG;

  return Offset;
}

/**
  This function returns the register offset for the ECC error mask register.

  @param[in]  MrcData - Pointer to global MRC data.
  @param[in]  Channel - 0-based index specifying which channel control to program.

  @retval - MCHBAR Register Offset.
**/
UINT32
MrcGetEccErrMskOffset (
  IN  MrcParameters *const  MrcData,
  IN  UINT32        const   Channel
  )
{
  UINT32  Offset;

  Offset = REUT_CH_ERR_ECC_MASK_0_CNL_REG +
    ((REUT_CH_ERR_ECC_MASK_1_CNL_REG - REUT_CH_ERR_ECC_MASK_0_CNL_REG) * Channel);

  return Offset;
}

