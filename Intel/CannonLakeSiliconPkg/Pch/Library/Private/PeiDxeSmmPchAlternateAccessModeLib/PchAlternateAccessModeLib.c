/** @file
  PCH alternate access mode library.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PchPcrLib.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsItss.h>


/**
  This function can be called to enable/disable Alternate Access Mode
  @note, the alternate access mode does not support to read NMI_EN# bit.

  @param[in] AmeCtrl              If TRUE, enable Alternate Access Mode.
                                  If FALSE, disable Alternate Access Mode.

**/
VOID
EFIAPI
PchAlternateAccessMode (
  IN  BOOLEAN       AmeCtrl
  )
{
  UINT32  Data32Or;
  UINT32  Data32And;

  Data32Or  = 0;
  Data32And = 0xFFFFFFFF;

  if (AmeCtrl == TRUE) {
    ///
    /// Enable Alternate Access Mode
    ///
    Data32Or  = (UINT32) (B_ITSS_PCR_GIC_AME);
  }

  if (AmeCtrl == FALSE) {
    ///
    /// Disable Alternate Access Mode
    ///
    Data32And = (UINT32) ~(B_ITSS_PCR_GIC_AME);
  }

  ///
  /// Program Alternate Access Mode Enable bit
  ///
  PchPcrAndThenOr32 (
    PID_ITSS, R_ITSS_PCR_GIC,
    Data32And,
    Data32Or
    );
}
