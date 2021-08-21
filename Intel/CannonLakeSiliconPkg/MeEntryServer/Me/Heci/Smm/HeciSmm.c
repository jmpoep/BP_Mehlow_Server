/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2008 - 2019 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciSmm.c

Abstract:

  SMM HECI driver

--*/

#include <Library/UefiLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>

#include <Protocol/SmmBase2.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>

#include <Protocol/Smmbase.h>

#include <MeHeciRegs.h>
#include <HeciRegs.h>
#include "HeciCore.h"


EFI_SMM_BASE2_PROTOCOL *mSmmBase2;
HECI_INSTANCE_SMM     *mHeciContext;
UINT64                  HeciMBar = 0;
BOOLEAN                 GotSmmReadyToLockEvent = FALSE;

/**
  Function sets HECI MBAR to input value.

  @param[in] HeciMBarIn           HECI MBAR to set

  @retval    NONE
**/
VOID
SetHeciMbar (
  IN UINT64 HeciMBarIn
  )
{
  UINTN  HeciPciAddressBase;
  union {
    UINT64   QWord;
    struct {
      UINT32 DWordL;
      UINT32 DWordH;
    } Bits;
  } Mbar;

  HeciPciAddressBase = mHeciContext->PciAddressBase;
    
  if (PciRead32 (HeciPciAddressBase + HECI_R_MBAR) == 0xFFFFFFFF) {
    DEBUG ((DEBUG_WARN, "[HECI] Warning: SetHeciMbar (): Can't set (HECI-1 device disabled)\n"));
    return;
  }
  Mbar.QWord = HeciMBarIn;

  // Store HECI MBAR
  DEBUG ((DEBUG_INFO, "[HECI] SetHeciMbar (): 0x%08X%08X\n", Mbar.Bits.DWordH, Mbar.Bits.DWordL));
  PciWrite32 (HeciPciAddressBase + HECI_R_MBAR + 4, Mbar.Bits.DWordH);
  PciWrite32 (HeciPciAddressBase + HECI_R_MBAR, Mbar.Bits.DWordL);
}

/**
  Function prepares HECI MBAR to use in SMM
  1. Before ReadyToLock event stores current MBAR in local variable
  2. Reads current MBAR to return to caller.
     Caller is responsible to restore original MBAR value before SMM exit
  3. Set HECI MBAR to safe value known before ReadyToLock event

  @param[in] NONE
  @retval    HeciMBar             Original HECI MBAR, before preparation for SMM

**/
UINTN
PrepareHeciMbar (
  VOID
  )
{
  UINT64 CurrentHeciMBar;

  if (GotSmmReadyToLockEvent == FALSE) {
    HeciMBar = (UINT64) HeciMbarReadFull (FALSE);
  }
  CurrentHeciMBar = (UINT64) HeciMbarReadFull (FALSE);
  SetHeciMbar (HeciMBar);

  return CurrentHeciMBar;
}

/**
  Function sends one message through the HECI circular buffer and waits
  for the corresponding ACK message.
  Before sending message check if HECI MBAR is not changed

  @param[in] HeciDev              The HECI device to be accessed.
  @param[in][out] Message         Pointer to the message buffer.
  @param[in] Length               Length of the message in bytes.
  @param[in][out] RecLength       Length of the message response in bytes.
  @param[in] HostAddress          Address of the sending entity.
  @param[in] MeAddress            Address of the ME entity that should receive the message.

  @retval EFI_SUCCESS             Command succeeded
  @retval EFI_DEVICE_ERROR        HECI Device error, command aborts abnormally
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_BUFFER_TOO_SMALL    Message Buffer is too small for the Acknowledge
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support send this message through this HECI
**/
EFI_STATUS
EFIAPI
HeciSendwAckWithCheck (
//  IN      HECI_DEVICE             HeciDev,
  IN OUT  UINT32                 *Message,
  IN      UINT32                  Length,
  IN OUT  UINT32                 *RecLength,
  IN      UINT8                   HostAddress,
  IN      UINT8                   MeAddress
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  UINT64     CurrentHeciMBar;

  CurrentHeciMBar = PrepareHeciMbar ();

  if (HeciMBar ==  HeciMbarReadFull (FALSE)) {
    Status = HeciSendwACK (/*HeciDev,*/
      Message,
      Length,
      RecLength,
      HostAddress,
      MeAddress
    );
  }

  SetHeciMbar (CurrentHeciMBar);

  return Status;
}

/**
  Reads a message from the ME across HECI. This function can only be used after invoking HeciSend() first.
  Before sending message check if HECI MBAR is not changed

  @param[in] HeciDev              The HECI device to be accessed.
  @param[in] Blocking             Used to determine if the read is BLOCKING or NON_BLOCKING.
  @param[in][out] MessageBody     Pointer to a buffer used to receive a message.
  @param[in][out] Length          Pointer to the length of the buffer on input and the length
                                  of the message on return. (in bytes)

  @retval EFI_SUCCESS             One message packet read.
  @retval EFI_DEVICE_ERROR        Failed to initialize HECI or zero-length message packet read
  @retval EFI_TIMEOUT             HECI is not ready for communication
  @retval EFI_BUFFER_TOO_SMALL    The caller's buffer was not large enough
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support this message through this HECI
**/
EFI_STATUS
EFIAPI
HeciReceiveWithCheck (
//  IN      HECI_DEVICE             HeciDev,
  IN      UINT32                  Blocking,
  IN OUT  UINT32                 *MessageBody,
  IN OUT  UINT32                 *Length
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  UINTN      CurrentHeciMBar;
  

  CurrentHeciMBar = PrepareHeciMbar ();

  if (HeciMBar == HeciMbarReadFull (FALSE)) {
    Status = HeciReceive (/*HeciDev,*/
      Blocking,
      MessageBody,
      Length
    );
  }

  SetHeciMbar (CurrentHeciMBar);

  return Status;
}

/**
  Function sends one message (of any length) through the HECI circular buffer.
  Before sending message check if HECI MBAR is not changed

  @param[in] HeciDev              The HECI device to be accessed.
  @param[in] Message              Pointer to the message data to be sent.
  @param[in] Length               Length of the message in bytes.
  @param[in] HostAddress          The address of the host processor.
  @param[in] MeAddress            Address of the ME subsystem the message is being sent to.

  @retval EFI_SUCCESS             One message packet sent.
  @retval EFI_DEVICE_ERROR        Failed to initialize HECI
  @retval EFI_TIMEOUT             HECI is not ready for communication
  @retval EFI_UNSUPPORTED         Current ME mode doesn't support send this message through this HECI
**/
EFI_STATUS
EFIAPI
HeciSendWithCheck (
  IN UINT32                      *Message,
  IN UINT32                       Length,
  IN UINT8                        HostAddress,
  IN UINT8                        MeAddress
  )
{
  EFI_STATUS Status = EFI_UNSUPPORTED;
  UINTN      CurrentHeciMBar;

  CurrentHeciMBar = PrepareHeciMbar ();

  if (HeciMBar == HeciMbarReadFull (FALSE)) {
    Status = HeciSend (/*HeciDev,*/
      Message,
      Length,
      HostAddress,
      MeAddress
    );
  }

  SetHeciMbar (CurrentHeciMBar);

  return Status;
}

/**
  Determines if the HECI device is present and, if present, initializes it for
  use by the BIOS.
  After initialization store HECI MBAR

  @param[in] HeciDev              The HECI device to be accessed.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_DEVICE_ERROR        No HECI device
  @retval EFI_TIMEOUT             HECI does not return the buffer before timeout
  @retval EFI_UNSUPPORTED         HECI MSG is unsupported
**/
EFI_STATUS
EFIAPI
HeciInitializeWithCheck ( void 
  /*IN HECI_DEVICE              HeciDev*/)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = HeciInitialize ();

  if ((!EFI_ERROR (Status)) &&
      (GotSmmReadyToLockEvent == FALSE)) {
    HeciMBar = HeciMbarReadFull (FALSE);
  }

  return Status;
}


/**
  Heci Re-initializes it for Host
  After Re-initialization store HECI MBAR

  @param[in] HeciDev          The HECI device to be accessed.

  @retval EFI_TIMEOUT         ME is not ready
  @retval EFI_STATUS          Status code returned by ResetHeciInterface
**/
EFI_STATUS
EFIAPI
HeciReInitializeWithCheck ( VOID
  /*IN HECI_DEVICE              HeciDev*/)
{
  EFI_STATUS Status = EFI_UNSUPPORTED;

  Status = HeciReInitialize ();

  if ((!EFI_ERROR (Status)) &&
      (GotSmmReadyToLockEvent == FALSE)) {
    HeciMBar = HeciMbarReadFull (FALSE);
  }

  return Status;
}

/**
  Smm Ready To Lock event notification handler.

  Update HECI MBAR after PCI enumerator

  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  // Check for multiple times handler trigger
  if (GotSmmReadyToLockEvent == FALSE) {
    // Store HECI BAR to verify if not changed upon execution
    HeciMBar = HeciMbarReadFull (FALSE);

    // Lock HeciMBar variable
    GotSmmReadyToLockEvent = TRUE;
  }

  return EFI_SUCCESS;
}


UINT64 CheckAndFixHeciForAccess ( VOID ){
  UINTN  HeciPciAddressBase;
  UINT32 HeciMBAR[2];

  HeciPciAddressBase = mHeciContext->PciAddressBase;

  //
  // Read HECI_MBAR in case it has changed
  //
  HeciMBAR[0] = PciRead32 (HeciPciAddressBase + HECI_R_MBAR) & 0xFFFFFFF0;
  if ((PciRead32 (HeciPciAddressBase + HECI_R_MBAR) & 0x6) == 0x4) {
    HeciMBAR[1] = PciRead32 (HeciPciAddressBase + (HECI_R_MBAR + 4));
  } else {
    HeciMBAR[1] = 0;
  }

  //
  // Check if HECI_MBAR is disabled
  //
  if ((PciRead8 (HeciPciAddressBase + HECI_R_CMD) & (HECI_CMD_MSE | HECI_CMD_BME)) != (HECI_CMD_MSE | HECI_CMD_BME))
  {
    //
    // If cmd reg in pci cfg space is not turned on turn it on.
    //
    PciOr8 (HeciPciAddressBase + HECI_R_CMD, HECI_CMD_MSE | HECI_CMD_BME);
  }

  return *((UINT64 *)HeciMBAR);
}

EFI_STATUS HeciSmmEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
){
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;
  BOOLEAN     InSmm;
  VOID        *Registration;

  Status = SystemTable->BootServices->LocateProtocol (
                                        &gEfiSmmBase2ProtocolGuid,
                                        NULL,
                                        (VOID **)&mSmmBase2
                                        );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Test if the entry point is running in SMM mode. If not just return.
  // In SMM mode install the HECI SMM service.
  //
  InSmm = FALSE;
  mSmmBase2->InSmm(mSmmBase2, &InSmm);
  if (!InSmm)  {
    return EFI_UNSUPPORTED;
  }

  //
  // Create database record and add to database
  //
  Status = gSmst->SmmAllocatePool (
                          EfiRuntimeServicesData, 
                          sizeof (HECI_INSTANCE_SMM), 
                          &(VOID *)mHeciContext);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mHeciContext->Handle = ImageHandle;

  //
  // Initialize HECI protocol pointers
  //
  mHeciContext->HeciCtlr.ResetHeci    = ResetHeciInterface;
  mHeciContext->HeciCtlr.SendwACK     = HeciSendwAckWithCheck;
  mHeciContext->HeciCtlr.ReadMsg      = HeciReceiveWithCheck;
  mHeciContext->HeciCtlr.SendMsg      = HeciSendWithCheck;
  mHeciContext->HeciCtlr.InitHeci     = HeciInitializeWithCheck;
  mHeciContext->HeciCtlr.ReInitHeci   = HeciReInitializeWithCheck;
  mHeciContext->HeciCtlr.MeResetWait  = MeResetWait;
  mHeciContext->HeciCtlr.GetMeStatus  = HeciGetMeStatus;
  mHeciContext->HeciCtlr.GetMeMode    = HeciGetMeMode;
  //
  // Initialize the HECI device
  //
  Status = InitializeHeciPrivate ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // Store HECI BAR to verify if not changed upon execution
  HeciMBar = HeciMbarReadFull (FALSE);  

  //
  // Install the HECI interface
  //
  Handle = NULL;
  Status = gSmst->SmmInstallProtocolInterface (
                &Handle,
                &gSmmHeciProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mHeciContext->HeciCtlr
               );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    gSmst->SmmFreePool(mHeciContext);
    mHeciContext = NULL;
    return Status;
  }

  //
  // register SMM Ready To Lock Protocol notification
  //
  Status = gSmst->SmmRegisterProtocolNotify (
                  &gEfiSmmReadyToLockProtocolGuid,
                  SmmReadyToLockEventNotify,
                  &Registration
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
