/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**
Copyright (c) 2015 - 2019 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file SpsSmm.c

  SPS SMM driver.
**/


//
// Standard header files.
//
#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/MeTypeLib.h>
#include <Library/PciLib.h>

//
// Protocols
//
#include <Protocol/SpsSmmHmrfpoProtocol.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/HeciSmm.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/HeciProtocol.h>

//
// Driver specific headers.
//
#include <HeciRegs.h>
#include <MeHeciRegs.h>
#include <MkhiMsgs.h>
#include <Sps.h>
#include <MePolicyHob.h>
#include <SpsSmm.h>
#include <MeDefines.h>


// global to store Nonce value read by ME HMRFPO_LOCK request
UINT64 gNonceValue = 0;

#if SPS_TESTMENU_FLAG
//global to store disable sending HmrfpoLock message to ME
STATIC UINT8  gHmrfpoLockEnabled    = TRUE;
STATIC UINT8  gHmrfpoEnableEnabled  = FALSE;
#endif // SPS_TESTMENU_FLAG
//global used to disable HECI before OS boot
#define HECI_MAX_INTERFACES   4

struct {
  UINT8 HeciId;
  UINT8 HeciState;
} gHeciHideInMe[HECI_MAX_INTERFACES];

#define HECI1_IX 1
#define HECI2_IX 2
#define HECI3_IX 3
#define HECI4_IX 4

#pragma pack (1)
typedef struct {
  HECI_MSG_HEADER Hdr;
  union {
    MKHI_MSG_HECI_STATE_CHANGE_REQ HeciStateReq;
    MKHI_MSG_HECI_STATE_CHANGE_RSP HeciStateRsp;
  } HeciMsg;
} HECI_MSG_STATE_CHANGE;
#pragma pack ()

/** SPS SMM driver entry point.

  @param[in] ImageHandle    Handle of driver image
  @param[in] pSysTable      Pointer to the system table

  @return EFI_STATUS is returned.
 */
EFI_STATUS
EFIAPI
SpsSmmEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *pSysTable
  )
{

#if !defined(SIMICS_FLAG)

  EFI_STATUS                Status;
  SMM_HECI_PROTOCOL         *pSmmHeci;
  EFI_SMM_BASE2_PROTOCOL    *pSmmBase;
  EFI_HANDLE                Handle;
  BOOLEAN                   InSmm;
  SPS_SMM_HMRFPO_PROTOCOL   *pSpsSmmHmrfpo;
  SPS_MEFS1                 MeFs1;
  ME_PEI_PREMEM_CONFIG	    *MePeiPreMemConfig;
  EFI_HOB_GUID_TYPE         *GuidHob;
  VOID                      *Registration;
  

  DEBUG ((DEBUG_INFO, "[SPS_SMM] SpsSmmEntryPoint started.\n"));

  Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, &pSmmBase);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Can't locate gEfiSmmBase2Protocol (%r).\n", Status));
    return Status;
  }

  // Test if the entry point is running in SMM mode. If not just return.
  InSmm = FALSE;
  pSmmBase->InSmm (pSmmBase, &InSmm);
  if (!InSmm) {
    return EFI_UNSUPPORTED;
  }

  // Check if ME SPS firmware is running
  if (!MeTypeIsSps ()) {
    MeFs1.DWord = MeHeci1PciRead32 (SPS_REG_MEFS1);
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] Non SPS firmware in ME. Driver will be unloaded. (MEFS1: 0x%08X)\n", MeFs1.DWord));

    /*
     * This is workaround for bug in efi core: unload of smm driver cause exception
     */
    //return EFI_UNSUPPORTED;
    return EFI_SUCCESS;
  }

  // Find HECI SMM protocol
  pSmmHeci = NULL;
  Status = gSmst->SmmLocateProtocol (&gSmmHeciProtocolGuid, NULL, &pSmmHeci);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI SMM protocol not found (%r)\n", Status));
    return Status;
  }

  Status = gSmst->SmmAllocatePool (EfiRuntimeServicesData, sizeof (SPS_SMM_HMRFPO_PROTOCOL), (VOID*)&pSpsSmmHmrfpo);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Can't allocate memory for SpsSmmHmrfpo protocol!. Status=%r\n", Status));
    return Status;
  }

  //init ME versions
  RetrieveMeVersions (pSmmHeci, pSpsSmmHmrfpo);

  //install SpsSmmHmrfpo Protocol
  pSpsSmmHmrfpo->Lock      = (HMRFPO_LOCK_FUNC)Hmrfpo_Lock;
  pSpsSmmHmrfpo->Enable    = (HMRFPO_ENABLE_FUNC)Hmrfpo_Enable;
  pSpsSmmHmrfpo->MeReset   = (HMRFPO_MERESET_FUNC)Hmrfpo_MeReset;

  Handle = NULL;
  Status = gSmst->SmmInstallProtocolInterface (
                    &Handle,
                    &gSpsSmmHmrfpoProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    pSpsSmmHmrfpo
                    );

  ASSERT_EFI_ERROR (Status);

  GuidHob = GetFirstGuidHob (&gMePreMemPolicyHobGuid);
  if (GuidHob != NULL) {
    MePeiPreMemConfig = GET_GUID_HOB_DATA (GuidHob);
    if (MePeiPreMemConfig == NULL) {
      gSmst->SmmUninstallProtocolInterface (
                          &Handle,
                          &gSpsSmmHmrfpoProtocolGuid,
                          pSpsSmmHmrfpo
                          );
      gSmst->SmmFreePool (pSpsSmmHmrfpo);
      DEBUG ((EFI_D_ERROR, "[SPS] ERROR: Get policy from HOB failed (%r)\n", Status));
      return EFI_NOT_READY;
    }
  } else {
    gSmst->SmmUninstallProtocolInterface (
                        &Handle,
                        &gSpsSmmHmrfpoProtocolGuid,
                        pSpsSmmHmrfpo
                        );
    gSmst->SmmFreePool (pSpsSmmHmrfpo);
    DEBUG ((EFI_D_ERROR, "[SPS] ERROR: Get HOB failed (%r)\n", Status));
    return EFI_NOT_READY;
  }

#if SPS_TESTMENU_FLAG
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "[SPS_SMM] ERROR: Cannot get ME configuration (%r)\n"
                          "          Using hardcoded defaults\n", Status));
      //set default values
      gHmrfpoLockEnabled    = TRUE;
      gHmrfpoEnableEnabled  = FALSE;
      gHeciHideInMe[0].HeciId    = HECI1_IX;
      gHeciHideInMe[0].HeciState = ME_SPS_HECI_IN_ME_OFF;
      gHeciHideInMe[1].HeciId    = HECI2_IX;
      gHeciHideInMe[1].HeciState = ME_SPS_HECI_IN_ME_OFF;
      gHeciHideInMe[2].HeciId    = HECI3_IX;
      gHeciHideInMe[2].HeciState = ME_SPS_HECI_IN_ME_OFF;
      gHeciHideInMe[3].HeciId    = HECI4_IX;
      gHeciHideInMe[3].HeciState = ME_SPS_HECI_IN_ME_OFF;
    } else {
      gHmrfpoLockEnabled    = (UINT8)MePeiPreMemConfig->MeHmrfpoLockEnabled;
      gHmrfpoEnableEnabled  = (UINT8)MePeiPreMemConfig->MeHmrfpoEnableEnabled;
      gHeciHideInMe[0].HeciId    = HECI1_IX;
      gHeciHideInMe[0].HeciState = (UINT8)MePeiPreMemConfig->Heci1HideInMe;
      gHeciHideInMe[1].HeciId    = HECI2_IX;
      gHeciHideInMe[1].HeciState = (UINT8)MePeiPreMemConfig->Heci2HideInMe;
      gHeciHideInMe[2].HeciId    = HECI3_IX;
      gHeciHideInMe[2].HeciState = (UINT8)MePeiPreMemConfig->Heci3HideInMe;
      gHeciHideInMe[3].HeciId    = HECI4_IX;
      gHeciHideInMe[3].HeciState = (UINT8)MePeiPreMemConfig->Heci4HideInMe;
    }

  // Send HmrfpoLock to get Nonce value
  if (gHmrfpoLockEnabled) {

#endif // SPS_TESTMENU_FLAG
    gNonceValue = 0;
    HeciReq_HmrfpoLock (pSmmHeci, &gNonceValue);
    if (gNonceValue != 0) {
      DEBUG ((DEBUG_INFO, "[SPS_SMM] Nonce saved.\n"));
    } else {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Can't get nonce!\n"));
    }

    //Decide if callbacks will be needed
    if (gHeciHideInMe[0].HeciState != ME_SPS_HECI_IN_ME_OFF ||
        gHeciHideInMe[1].HeciState != ME_SPS_HECI_IN_ME_OFF ||
        gHeciHideInMe[3].HeciState != ME_SPS_HECI_IN_ME_OFF ||
        gHeciHideInMe[2].HeciState != ME_SPS_HECI_IN_ME_OFF
    ) {
      //HECI needs to be disabled at the end of boot - register callbacks to do it
    
      //Register callback on ExitBootServices event
      Registration = NULL;
      Status = gSmst->SmmRegisterProtocolNotify (
                       &gEdkiiSmmExitBootServicesProtocolGuid,
                       SpsSmmExitBootServicesCallback,
                       &Registration
                       );
    
      if (!EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "[SPS_SMM] Callback on ExitBootServices event registered .\n"));
      } else {
        DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Can't register callback on ExitBootServices event.\n"));
        ASSERT_EFI_ERROR (Status);
      }
    
      //Register callback on LegacyBoot event
      Registration = NULL;
      Status = gSmst->SmmRegisterProtocolNotify (
                       &gEdkiiSmmLegacyBootProtocolGuid,
                       SpsSmmExitBootServicesCallback,
                       &Registration
                       );
    
      if (!EFI_ERROR (Status)) {
        DEBUG ((DEBUG_INFO, "[SPS_SMM] Callback on LegacyBoot event registered .\n"));
      } else {
        DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Can't register callback on ExitBootServices event.\n"));
        ASSERT_EFI_ERROR (Status);
      }
    }
    

#if SPS_TESTMENU_FLAG

    // send HECI enable if needed
    if (gHmrfpoEnableEnabled) {
      DEBUG ((DEBUG_INFO, "[SPS_SMM] HmrfpoEnable needs to be send after HmrfpoLock.\n"));
      HeciReq_HmrfpoEnable (pSmmHeci, gNonceValue);
    }
  } else {
    // if MeHmrfpoLockEnabled is set to FALSE in setup don't send any HMRFPO_LOCK message - ME updated tool needs to get nonce...
    gNonceValue = 0;
    DEBUG ((DEBUG_INFO, "[SPS_SMM] HMRFPO_LOCK disabled in setup.\n"));
  }

#endif // SPS_TESTMENU_FLAG

  DEBUG ((DEBUG_INFO, "[SPS_SMM] SpsSmmEntryPoint ends.\n"));

#endif // !SIMICS_FLAG

  return EFI_SUCCESS;
} // SpsSmmEntryPoint


/** Procedure to send HMRFPO_LOCK HECI Request

    @param[in]    *pSmmHeci - pointer to SMM HECI driver
    @param[in,out] Nonce - buffer to store nonce value from HECI message response
    @return EFI_STATUS is returned.
*/
EFI_STATUS
HeciReq_HmrfpoLock (
  IN     SMM_HECI_PROTOCOL *pSmmHeci,
  IN OUT UINT64            *Nonce
  )
{
  UINT32     RspLen;
  EFI_STATUS Status;
  UINT8      Retries;
  SPS_MEFS1  MeFs1;

  union {
    HECI_MSG_HMRFPO_LOCK_REQ Req;
    HECI_MSG_HMRFPO_LOCK_RSP Rsp;
  } HeciMsg;

  DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending HMRFPO_LOCK, MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1)));

  MeFs1.DWord = MeHeci1PciRead32 (SPS_REG_MEFS1);
  if (MeFs1.Bits.CurrentState == MEFS1_CURSTATE_RECOVERY) {
    // Construct HECI HMRFPO_LOCK request
    ZeroMem (&HeciMsg, sizeof (HeciMsg));
    HeciMsg.Req.Mkhi.Data = HECI_MSG_LOCK_REQ_MKHI_HDR;

    Status = pSmmHeci->SendMsg ((UINT32*)&HeciMsg.Req,
                                sizeof (HeciMsg.Req),
                                SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_LOCK sending failed (%r)\n", Status));
      pSmmHeci->ResetHeci ();
    }
  }

  for (Retries = 3; Retries--; ) {
    // Construct HECI HMRFPO_LOCK request
    ZeroMem (&HeciMsg, sizeof (HeciMsg));
    HeciMsg.Req.Mkhi.Data = HECI_MSG_LOCK_REQ_MKHI_HDR;

    RspLen = sizeof (HeciMsg.Rsp);
    Status = pSmmHeci->SendwACK ((UINT32*)&HeciMsg.Req,
                                 sizeof (HeciMsg.Req), &RspLen,
                                 SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);

    DEBUG ((DEBUG_INFO, "[SPS_SMM] MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1)));

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] HMRFPO_LOCK send failed (%r)\n", Status));
      continue;
    }

    if (HeciMsg.Rsp.Mkhi.Data != HECI_MSG_LOCK_RSP_MKHI_HDR) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Invalid HMRFPO_LOCK response (MKHI: 0x%08X)\n",
              HeciMsg.Rsp.Mkhi.Data));

      if (Retries > 1) {
        pSmmHeci->ResetHeci ();
      }
      Status = EFI_UNSUPPORTED;
      continue;
    }

    if (HeciMsg.Rsp.Status != 0) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_LOCK failed (cause: %d)\n",
              HeciMsg.Rsp.Status));

      Status = EFI_UNSUPPORTED;
      continue;
    }
    break;
  }

  pSmmHeci->ResetHeci ();
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[SPS_SMM] HMRFPO_LOCK reports success\n"));

    if (Nonce != NULL) {
      *Nonce = HeciMsg.Rsp.Nonce; //save the nonce value
    }
  }

  return Status;
}

/** Procedure to send HMRFPO_ENABLE HECI Request

    Please note: After sending HmrfpoEnable message ME needs to reset itself
    - this means that this is highly probable that this function fails at first few tries.
    This is recommended to call this function in a loop a few times until success is returned.
    Please refer to ME-Bios spec. for more details.

    @param[in]    *pSmmHeci pointer to SMM HECI driver
    @param[in]    *Nonce    Nonce value for current session

    @return EFI_STATUS is returned.
*/
EFI_STATUS
HeciReq_HmrfpoEnable (
  IN SMM_HECI_PROTOCOL *pSmmHeci,
  IN UINT64            Nonce
  )
{
  UINT32     RspLen;
  EFI_STATUS Status;
  UINT8      Retries;
  SPS_MEFS1  MeFs1;

  union {
    HECI_MSG_HMRFPO_ENABLE_REQ Req;
    HECI_MSG_HMRFPO_ENABLE_RSP Rsp;
  } HeciMsg;

  DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending HMRFPO_ENABLE, MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1) ));

  MeFs1.DWord = MeHeci1PciRead32 (SPS_REG_MEFS1);
  if (MeFs1.Bits.CurrentState == MEFS1_CURSTATE_NORMAL) {
    // Construct HMRFPO_ENABLE request message
    ZeroMem (&HeciMsg, sizeof (HeciMsg));
    HeciMsg.Req.Mkhi.Data = HECI_MSG_ENABLE_REQ_MKHI_HDR;
    HeciMsg.Req.Nonce = Nonce;

    Status = pSmmHeci->SendMsg ((UINT32*)&HeciMsg.Req,
                                sizeof (HeciMsg.Req),
                                SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_ENABLE sending failed (%r)\n", Status));
      pSmmHeci->ResetHeci ();
    }
  }

  for (Retries = 3; Retries--; ) {
    // Construct HMRFPO_ENABLE request message
    ZeroMem (&HeciMsg, sizeof (HeciMsg));
    HeciMsg.Req.Mkhi.Data = HECI_MSG_ENABLE_REQ_MKHI_HDR;
    HeciMsg.Req.Nonce = Nonce;
    RspLen = sizeof (HeciMsg.Rsp);

    Status = pSmmHeci->SendwACK ((UINT32*)&HeciMsg.Req,
                                 sizeof (HeciMsg.Req), &RspLen,
                                 SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);

    DEBUG ((DEBUG_INFO, "[SPS_SMM] MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1)));

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_ENABLE sending failed (%r)\n", Status));
      continue;
    }

    if (HeciMsg.Rsp.Mkhi.Data != HECI_MSG_ENABLE_RSP_MKHI_HDR) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Invalid HMRFPO_ENABLE response (MKHI: 0x%08X)\n",
          HeciMsg.Rsp.Mkhi.Data));
      if (Retries > 1) {
        pSmmHeci->ResetHeci ();
      }
      Status = EFI_UNSUPPORTED;
      continue;
    }

    if (HeciMsg.Rsp.Status != 0) {
      DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_ENABLE refused (cause: %d)\n",
          HeciMsg.Rsp.Status));
      Status = EFI_UNSUPPORTED;
      continue;
    }
    break;
  }

  pSmmHeci->ResetHeci ();
  if (!EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[SPS_SMM] HMRFPO_ENABLE reports success.\n"));
  }
  return Status;
}

/** Procedure to send HMRFPO_RESET HECI Request

    Please note: HmrfpoReset message make ME to reset itself
    - this means that this is highly probable that this function return fail (timeout occurs).

    Please refer to ME-Bios spec. for more details.

    @param[in]    *pSmmHeci pointer to SMM HECI driver
    @param[in]    *Nonce    Nonce value for current session

    @return EFI_STATUS is returned.
*/
EFI_STATUS
HeciReq_HmrfpoMeReset (
  IN SMM_HECI_PROTOCOL *pSmmHeci,
  IN UINT64            Nonce
  )
{

  UINT32     RspLen;
  EFI_STATUS Status;

  union {
    HECI_MSG_HMRFPO_MERESET_REQ Req;
    HECI_MSG_HMRFPO_MERESET_RSP Rsp;
  } HeciMsg;

  DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending HMRFPO_MERESET, MEFS1: 0x%08X\n",
    MeHeci1PciRead32 (SPS_REG_MEFS1)));


  // Construct HMRFPO_MERESET request message
  ZeroMem (&HeciMsg, sizeof (HeciMsg));

  HeciMsg.Req.Mkhi.Data = HECI_MSG_MERESET_REQ_MKHI_HDR;
  HeciMsg.Req.Nonce = Nonce;

  RspLen = sizeof (HeciMsg.Rsp);
  Status = pSmmHeci->SendwACK ((UINT32*)&HeciMsg.Req,
                              sizeof (HeciMsg.Req), &RspLen,
                              SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);

  DEBUG ((DEBUG_INFO, "[SPS_SMM] MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1)));
  pSmmHeci->ResetHeci ();

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] HMRFPO_MERESET send failed (%r)\n", Status));
    return Status;
  }

  if (HeciMsg.Rsp.Mkhi.Data != HECI_MSG_MERESET_RSP_MKHI_HDR) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Invalid HMRFPO_MERESET response (MKHI: 0x%08X)\n",
           HeciMsg.Rsp.Mkhi.Data));
    return EFI_UNSUPPORTED;
  }

  if (HeciMsg.Rsp.Status != 0) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HMRFPO_MERESET refused (cause: %d)\n",
           HeciMsg.Rsp.Status));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO, "[SPS_SMM] HMRFPO_MERESET reports success.\n"));
  return EFI_SUCCESS;
}


/**
  Function which implements SpsSmmHmrfpo Protocol Lock function

  @return EFI_STATUS is returned.
*/
EFI_STATUS
Hmrfpo_Lock (VOID)
{
  SMM_HECI_PROTOCOL *pSmmHeci;
  EFI_STATUS        Status;

#if SPS_TESTMENU_FLAG
  if (!gHmrfpoLockEnabled) {
    DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending Hmrfpo_Lock message is disabled in setup\n"));
    return EFI_UNSUPPORTED;
  }
#endif // SPS_TESTMENU_FLAG

  Status = gSmst->SmmLocateProtocol (&gSmmHeciProtocolGuid, NULL, &pSmmHeci);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI SMM protocol not found (%r)\n", Status));
    return Status;
  }

  Status = HeciReq_HmrfpoLock (pSmmHeci, NULL);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HeciReq_HmrfpoLock command returns error (%r)\n", Status));
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  Function which implements SpsSmmHmrfpo Protocol MeReset function

  @return EFI_STATUS is returned.
*/
EFI_STATUS
Hmrfpo_MeReset (VOID)
{
  SMM_HECI_PROTOCOL *pSmmHeci;
  EFI_STATUS        Status;

  if (gNonceValue == 0) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Nonce value unknown. Operation not supported\n"));
    return EFI_UNSUPPORTED;
  }

  Status = gSmst->SmmLocateProtocol (&gSmmHeciProtocolGuid, NULL, &pSmmHeci);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI SMM protocol not found (%r)\n", Status));
    return Status;
  }

  Status = HeciReq_HmrfpoMeReset (pSmmHeci, gNonceValue);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HeciReq_HmrfpoMeReset command returns error (%r)\n", Status));
    return Status;
  }
  return EFI_SUCCESS;
}

/**
  Function which implements SpsSmmHmrfpo Protocol Enable function

  @return EFI_STATUS is returned.
*/
EFI_STATUS
Hmrfpo_Enable (VOID)
{
  SMM_HECI_PROTOCOL *pSmmHeci;
  EFI_STATUS        Status;

  if (gNonceValue == 0) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Nonce value unknown. Operation not supported\n"));
    return EFI_UNSUPPORTED;
  }

  Status = gSmst->SmmLocateProtocol (&gSmmHeciProtocolGuid, NULL, &pSmmHeci);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI SMM protocol not found (%r)\n", Status));
    return Status;
  }

  Status = HeciReq_HmrfpoEnable (pSmmHeci, gNonceValue);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HeciReq_HmrfpoEnable command returns error (%r)\n", Status));
    return Status;
  }
  return EFI_SUCCESS;
}


/** Function set the ME versions red by MKHI messages

  @param[in,out]  pSmsSmmHmrfpo -
  @return         EFI_STATUS
*/
VOID RetrieveMeVersions (
  IN     SMM_HECI_PROTOCOL       *pSmmHeci,
  IN OUT SPS_SMM_HMRFPO_PROTOCOL *pSpsSmmHmrfpo
  )
{
  EFI_STATUS Status;
  UINT32     RspLen;
  union {
    HECI_MSG_MKHI_GET_FW_VERSION_REQ Req;
    HECI_MSG_MKHI_GET_FW_VERSION_RSP Rsp;
  } HeciMsg;

  //
  // Send MKHI_GET_FW_VERSION request to ME
  //
  DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending MKHI_GET_FW_VERSION, expect success\n"));

  // Construct MKHI_GET_FW_VERSION request
  ZeroMem (&HeciMsg, sizeof (HeciMsg));

  HeciMsg.Req.Mkhi.Data = HECI_MSG_GET_FW_VERSION_REQ_MKHI_HDR;

  RspLen = sizeof (HeciMsg.Rsp);
  Status = pSmmHeci->SendwACK ((UINT32*)&HeciMsg.Req,
                              sizeof (HeciMsg.Req), &RspLen,
                              SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);


  DEBUG ((DEBUG_INFO, "[SPS_SMM] MEFS1: 0x%08X\n", MeHeci1PciRead32 (SPS_REG_MEFS1)));

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[ME] ERROR: Cannot send MKHI_GET_FW_VERSION (%r)\n",Status));

  } else if (HeciMsg.Rsp.Mkhi.Data != HECI_MSG_GET_FW_VERSION_RSP_MKHI_HDR) {
    DEBUG ((DEBUG_ERROR, "[ME] ERROR: Invalid MKHI_GET_FW_VERSION response (MKHI: 0x%X)\n",
              HeciMsg.Rsp.Mkhi.Data));

  } else {
    // no errors - copy data from HECI message

    // Active operational firmware
    pSpsSmmHmrfpo->ActiveFwVersion.Minor  = HeciMsg.Rsp.Act.Minor;
    pSpsSmmHmrfpo->ActiveFwVersion.Major  = HeciMsg.Rsp.Act.Major;
    pSpsSmmHmrfpo->ActiveFwVersion.Build  = HeciMsg.Rsp.Act.Build;
    pSpsSmmHmrfpo->ActiveFwVersion.Patch  = HeciMsg.Rsp.Act.Patch;

    // Recovery firmware
    pSpsSmmHmrfpo->RecoveryFwVersion.Minor  = HeciMsg.Rsp.Rcv.Minor;
    pSpsSmmHmrfpo->RecoveryFwVersion.Major  = HeciMsg.Rsp.Rcv.Major;
    pSpsSmmHmrfpo->RecoveryFwVersion.Build  = HeciMsg.Rsp.Rcv.Build;
    pSpsSmmHmrfpo->RecoveryFwVersion.Patch  = HeciMsg.Rsp.Rcv.Patch;

    //
    // ME in dual-image configuration provides the version of the backup image yet
    //
    if (RspLen > sizeof (HeciMsg.Rsp) - sizeof (HeciMsg.Rsp.Bkp)) {

      // Backup operational firmware (optional)
      pSpsSmmHmrfpo->BackupFwVersion.Minor  = HeciMsg.Rsp.Bkp.Minor;
      pSpsSmmHmrfpo->BackupFwVersion.Major  = HeciMsg.Rsp.Bkp.Major;
      pSpsSmmHmrfpo->BackupFwVersion.Build  = HeciMsg.Rsp.Bkp.Build;
      pSpsSmmHmrfpo->BackupFwVersion.Patch  = HeciMsg.Rsp.Bkp.Patch;
    }
  }
}

/**
  This code is called on ExitBootServices event (or LegacyBoot as well)

  @param[in] Protocol   Points to the protocol's unique identifier
  @param[in] Interface  Points to the interface instance
  @param[in] Handle     The handle on which the interface was installed

  @retval EFI_SUCCESS   always success successfully
*/
EFI_STATUS
EFIAPI
SpsSmmExitBootServicesCallback (
  IN CONST EFI_GUID     *Protocol,
  IN VOID               *Interface,
  IN EFI_HANDLE         Handle
  )
{
  EFI_STATUS            Status;
  SMM_HECI_PROTOCOL     *pSmmHeci;
  UINT8                 Index;
  UINT32                RspLen;
  HECI_MSG_STATE_CHANGE HeciStateChange;

  DEBUG ((DEBUG_INFO, "[SPS_SMM] SpsSmmExitBootServicesCallback enter\n"));

  // Find HECI SMM protocol
  pSmmHeci = NULL;
  Status = gSmst->SmmLocateProtocol (&gSmmHeciProtocolGuid, NULL, &pSmmHeci);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI SMM protocol not found (%r)\n", Status));
    return Status;
  }

  //
  // Send HECI_STATE_CHANGE if enabled in ME policy
  //

  for (Index = HECI_MAX_INTERFACES; Index--;) {
    if (gHeciHideInMe[Index].HeciState != ME_SPS_HECI_IN_ME_OFF) {
      HeciStateChange.HeciMsg.HeciStateReq.Mkhi.Data = 0x00000DFF;
      HeciStateChange.HeciMsg.HeciStateReq.Nonce = gNonceValue;
      HeciStateChange.HeciMsg.HeciStateReq.HeciId = gHeciHideInMe[Index].HeciId;
      HeciStateChange.HeciMsg.HeciStateReq.State = gHeciHideInMe[Index].HeciState & 1;
      RspLen = sizeof (HeciStateChange.HeciMsg);
      DEBUG ((DEBUG_INFO, "[SPS_SMM] Sending HECI_CHANGE_STATE to ME to %a HECI-%d\n",
        (HeciStateChange.HeciMsg.HeciStateReq.State) ? "hide" : "disable", HeciStateChange.HeciMsg.HeciStateReq.HeciId));

      Status = pSmmHeci->SendMsg ((UINT32*)&(HeciStateChange.HeciMsg.HeciStateReq),
        sizeof (HeciStateChange.HeciMsg.HeciStateReq), SPS_CLIENTID_BIOS, SPS_CLIENTID_ME_MKHI);
      if (!EFI_ERROR (Status)) {
        //
        // We don't expect response if it is HECI-1 hidden/disabled.
        // ME executes the operation before we can read response, so it fails.
        //
        if (Index == 0) {
          HeciStateChange.HeciMsg.HeciStateRsp.Mkhi.Data = 0x00008DFF;
          HeciStateChange.HeciMsg.HeciStateRsp.Response = 0;
        } else {
          Status = pSmmHeci->ReadMsg (TRUE, (UINT32*)&(HeciStateChange.HeciMsg.HeciStateRsp), &RspLen);
        }
      }
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Cannot send HECI_CHANGE_STATE (%r)\n", Status));
      } else if (HeciStateChange.HeciMsg.HeciStateRsp.Mkhi.Data != 0x00008DFF) {
        DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: Invalid HECI_CHANGE_STATE response (MKHI: 0x%X)\n",
          HeciStateChange.HeciMsg.HeciStateRsp.Mkhi.Data));
      } else if (HeciStateChange.HeciMsg.HeciStateRsp.Response != 0) {
        DEBUG ((DEBUG_ERROR, "[SPS_SMM] ERROR: HECI_CHANGE_STATE for HECI-%d failed (%d)\n",
          gHeciHideInMe[Index].HeciId, HeciStateChange.HeciMsg.HeciStateRsp.Response));
      }
    }
  } // for (Index...)
  if (gHeciHideInMe[HECI1_IX].HeciState != ME_SPS_HECI_IN_ME_DISABLE) {
    pSmmHeci->ResetHeci ();
  }

  DEBUG ((DEBUG_INFO, "[SPS_SMM] SpsSmmExitBootServicesCallback exit\n"));
  return EFI_SUCCESS;
}
