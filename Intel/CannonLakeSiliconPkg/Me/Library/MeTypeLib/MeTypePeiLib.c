/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c)  2017 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

@file:
  MeTypePeiLib.c

@brief:
  Me Type Lib implementation.

**/
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/PeiServicesLib.h>
#include <PiDxe.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include "MeTypeLibInternal.h"
#include <MeChipset.h>
#include <BupMsgs.h>
#include <Ppi/HeciPpi.h>
#include <Library/HobLib.h>
#include <MkhiMsgs.h>
#include <MeiBusMsg.h>

/**

  @brief
  Get Me FW type from ME via HECI

  @param[in] None

  @retval On Error:                ME_TYPE_UNDEF
  @retval On Success:              ME_TYPE_SPS
  @retval On Success:              ME_TYPE_AMT
  @retval On Success:              ME_TYPE_DISABLED

**/
ON_BOARD_ME_TYPE
PeiHeciGetMeTypeMsg (
  VOID
  )
{
  EFI_STATUS                      Status;
  UINT32                          Length;
  UINT32                          RecvLength;
  HECI_PPI                        *HeciPpi;
  GEN_GET_ME_TYPE_BUFFER          MsgGenGetMeType;
  UINT32                          MeMode;
  ON_BOARD_ME_TYPE                RetVal = ME_TYPE_UNDEF;

  DEBUG ((EFI_D_INFO, "HECI: PeiHeciGetMeTypeMsg() start\n"));

  Status = PeiServicesLocatePpi (
            &gHeciPpiGuid,            // GUID
            0,                        // INSTANCE
            NULL,                     // EFI_PEI_PPI_DESCRIPTOR
            (VOID **) &HeciPpi        // PPI
            );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    return EFI_NOT_FOUND;
  }

  Status = HeciPpi->GetMeMode (&MeMode);
  if (EFI_ERROR (Status) || (MeMode != ME_MODE_NORMAL)) {
    return EFI_UNSUPPORTED;
  }

  MsgGenGetMeType.Request.MKHIHeader.Data               = 0;
  MsgGenGetMeType.Request.MKHIHeader.Fields.GroupId     = BUP_COMMON_GROUP_ID;
  MsgGenGetMeType.Request.MKHIHeader.Fields.Command     = BUP_GET_ME_TYPE;
  MsgGenGetMeType.Request.MKHIHeader.Fields.IsResponse  = 0;
  Length                                                = sizeof (GEN_GET_FW_TYPE);
  RecvLength                                            = sizeof (GEN_GET_FW_TYPE_ACK);

  ///
  /// Send Get Platform Type Request to ME
  ///
  Status = HeciPpi->SendwAck (
                  HECI1_DEVICE,
                  (UINT32 *) &MsgGenGetMeType,
                  Length,
                  &RecvLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_MKHI_MESSAGE_ADDR
                  );
  if (!EFI_ERROR(Status) &&
      (MsgGenGetMeType.Response.MKHIHeader.Fields.IsResponse == 1) &&
      (MsgGenGetMeType.Response.MKHIHeader.Fields.GroupId == BUP_COMMON_GROUP_ID) &&
      (MsgGenGetMeType.Response.MKHIHeader.Fields.Command == BUP_GET_ME_TYPE)) {
    if (RecvLength != sizeof (GEN_GET_FW_TYPE_ACK)) {
      DEBUG ((DEBUG_ERROR, "HECI: PeiHeciGetMeTypeMsg() - Response: wrong length = %d\n",
        RecvLength));
    } else {
      DEBUG ((DEBUG_INFO, "HECI: PeiHeciGetMeTypeMsg() - OK: FwType = %d, FwSubType = %d\n",
        MsgGenGetMeType.Response.FwTypeData.FwType, MsgGenGetMeType.Response.FwTypeData.FwSubType));
      RetVal = MsgGenGetMeType.Response.FwTypeData.FwType;
      if (RetVal == METYPE_SPS_EPO) {
        DEBUG ((DEBUG_INFO, "HECI: PeiHeciGetMeTypeMsg() Use MeType SPS for SPS-EPO\n"));
        RetVal = METYPE_SPS;
      } else if ( (RetVal == METYPE_SPS) && 
          (MsgGenGetMeType.Response.FwTypeData.FwSubType == MESUBTYPE_SPS_TYPE_SV)) {
        DEBUG ((DEBUG_INFO, "HECI: PeiHeciGetMeTypeMsg() DFX FW discovered\n"));
        RetVal = METYPE_DFX;
      }
    }
  }

  DEBUG ((DEBUG_INFO, "HECI: PeiHeciGetMeTypeMsg() Status is %r, returns %d\n",
    Status, RetVal));

  return RetVal;
}

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
  HECI_PPI            *HeciPpi;
  ME_FW_TYPE_HOB      *MeTypeHob;
  EFI_STATUS          Status = EFI_NOT_FOUND;

  DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() Start (PEI)\n"));

  MeTypeHob = GetFirstGuidHob (&gMeTypeHobGuid);
  if (MeTypeHob == NULL) {
    Status = PeiServicesLocatePpi (
            &gHeciPpiGuid,            // GUID
            0,                        // INSTANCE
            NULL,                     // EFI_PEI_PPI_DESCRIPTOR
            (VOID **) &HeciPpi        // PPI
            );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR(Status)) {
      return ME_TYPE_UNDEF;
    }
    Status = HeciPpi->InitializeHeci(HECI1_DEVICE);
    if (EFI_ERROR(Status)) {
      DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() Cannot initialize HECI: %r\n", Status));
      return ME_TYPE_UNDEF;
    }

    RetVal = PeiHeciGetMeTypeMsg();
    if (RetVal != ME_TYPE_UNDEF) {
      DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() Create MeType HOB for ME: %d\n", RetVal));
      Status = PeiServicesCreateHob ( EFI_HOB_TYPE_GUID_EXTENSION,
                                      sizeof(ME_FW_TYPE_HOB),
                                      &MeTypeHob );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() Set MeType HOB info to: %d\n", RetVal));
        MeTypeHob->EfiHobGuidType.Name = gMeTypeHobGuid;
        MeTypeHob->MeType = RetVal;
      }
      RetVal = ME_TYPE_UNDEF;
    }
  }

  MeTypeHob = GetFirstGuidHob (&gMeTypeHobGuid);
  if (MeTypeHob != NULL) {
    RetVal = MeTypeHob->MeType;
  }

  DEBUG ((DEBUG_ERROR, "HECI: GetMeTypeFromHob() returns %d\n", RetVal));

  return RetVal;
}
