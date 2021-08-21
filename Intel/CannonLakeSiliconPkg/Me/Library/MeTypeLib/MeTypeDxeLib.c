/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2017 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

@file:
  MeTypeDxeLib.c

@brief:
  Me Type Lib implementation.

**/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <PiDxe.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include "PchAccess.h"
#include "HeciRegs.h"
#include "Library/MeTypeLib.h"
#include "MeTypeLibInternal.h"
#include "MeFwHob.h"
#include <Library/HobLib.h>
#include <Library/PeiMeLib.h>

/**
  Function returns ME Type read from HOB /if it is not present/
  If HOB is not present it tries to create it.

  @param[in] None

  @retval On Error:                ME_TYPE_UNDEF
  @retval On Success:              ME_TYPE_SPS
  @retval On Success:              ME_TYPE_AMT
  @retval On Success:              ME_TYPE_DISABLED
**/

ON_BOARD_ME_TYPE
GetMeTypeFromHob (
  VOID
  )
{
  ON_BOARD_ME_TYPE    RetVal = ME_TYPE_UNDEF;
  ME_FW_TYPE_HOB      *MeTypeHob;

  DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() Start (DXE)\n"));

  MeTypeHob = GetFirstGuidHob (&gMeTypeHobGuid);
  if (MeTypeHob != NULL) {
    RetVal = MeTypeHob->MeType;
  }

  DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() returns %d\n", RetVal));

  return RetVal;
}
