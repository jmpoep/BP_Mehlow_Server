/** @file
  This file contain OEM/Platform implementation for TXT in DXE phase.
  It represents an abstract outline of the steps required during DXE
  for enabling TXT.  Each individual step is further abstracted behind
  a function call interface.  This is intended to minimize the need to
  modify this file when porting TXT to future platforms.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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

#include <Private/Library/PchXhciLib.h>
#include "Txt.h"
#include "TxtDxeLib.h"

/**
  This function gets called before/after run the SCHECK function, intend to avoid platform specific SMIs to interfere in BIOS POST
  if BIOS SMM MP services or synchonization code is not well considered that some of APs are not waken up from Wait-for-SIPI state.
  Function should preserve original SMI enabling setting in augument is TRUE, and then restore it in augurment is FALSE.
  The caller is DisableSmiSources() in TxtDxeLib.c

  @param[in] TxtDxeCtx        - Point to TXT_DXE_LIB_CONTEXT structure
  @param[in] Operation        = TRUE    - Calling before SCHECK to saved and disable platform specific SMIs setting
                              = FALSE   - Calling after SCHECK to restore platform specific SMIs setting

  @retval EFI_SUCCESS   - Always.
  **/
EFI_STATUS
EFIAPI
TxtDxeOemDisableSmi (
  IN TXT_DXE_LIB_CONTEXT *TxtDxeCtx,
  IN BOOLEAN             Operation
  )
{
  STATIC UINT32 SavedSmiControlXhci;

  if (Operation == TRUE) {
    ///
    /// Save and disable OEM/Platform specific SMIs
    ///
    /// Disable XHCI SMIs before giving control to TXT ACM. XHCI controller generating SMI during TXT ACM results in USB kbd issue
    ///
    SavedSmiControlXhci = PchXhciLegacySmiEnGet ();
    PchXhciLegacySmiEnSet (0);
  } else {
    ///
    /// Restore or re-enable OEM/Platform specific SMIs
    ///
    /// Enable EHCI SMIs before giving control to TXT ACM.
    ///
    PchXhciLegacySmiEnSet (SavedSmiControlXhci);
  }

  return EFI_SUCCESS;
}
