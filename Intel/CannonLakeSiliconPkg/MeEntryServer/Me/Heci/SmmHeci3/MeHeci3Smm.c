/** @file

  @copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation. <BR>

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary    and
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
**/

#include <PiSmm.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/MeTypeLib.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/MeHeci3Smm.h>
#include <Protocol/SmmReadyToLock.h>

#include <MeHeci3.h>

#include "MeHeci3Core.h"


/*****************************************************************************
 * Local definitions.
 *****************************************************************************/


/*****************************************************************************
 * Local function prototypes.
 *****************************************************************************/
EFI_STATUS EFIAPI SmmHeciInit(IN OUT SMM_ME_HECI3_PROTOCOL *pThis,
                              IN OUT UINT32                *pTimeout);
EFI_STATUS EFIAPI SmmHeciQueReset(IN OUT SMM_ME_HECI3_PROTOCOL *pThis,
                                  IN OUT UINT32                *pTimeout);
EFI_STATUS EFIAPI SmmHeciQueState(IN OUT SMM_ME_HECI3_PROTOCOL *pThis,
                                     OUT BOOLEAN               *pIsReady,
                                     OUT UINT32                *pSendQue,
                                     OUT UINT32                *pRecvQue);
EFI_STATUS EFIAPI SmmHeciRequest(IN OUT SMM_ME_HECI3_PROTOCOL *pThis,
                                 IN OUT UINT32                *pTimeout,
                                 IN     HECI_MSG_HEADER       *pReqMsg,
                                    OUT HECI_MSG_HEADER       *pRspBuf,
                                 IN     UINT32                *pBufLen);
UINT64 HeciMbarReadFull (IN OUT ME_HECI_DEVICE *pThis,
                         IN     BOOLEAN        CleanBarTypeBits);
VOID   SetHeciMbar (IN UINT64        HeciMBarIn,
                    IN ME_HECI_DEVICE *pThis);
UINT64 PrepareHeciMbar (IN ME_HECI_DEVICE *pThis);

/*****************************************************************************
 * Variables.
 *****************************************************************************/
UINT64  HeciMBar = 0;
BOOLEAN GotSmmReadyToLockEvent = FALSE;

/*****************************************************************************
 * Local functions.
 *****************************************************************************/

/**
 * Smm Ready To Lock event notification handler.
 *
 * Update HECI MBAR after PCI enumerator
 *
 * @param[in] Protocol   Points to the protocol's unique identifier.
 * @param[in] Interface  Points to the interface instance.
 * @param[in] Handle     The handle on which the interface was installed.
 *
 * @retval    EFI_SUCCESS   Notification handler runs successfully.
 *
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  EFI_STATUS                Status;
  SMM_ME_HECI3_PROTOCOL     *pSmmHeci;
  EFI_SMM_SYSTEM_TABLE2     *pSmst;
  EFI_SMM_BASE2_PROTOCOL    *pSmmBase;

  if (GotSmmReadyToLockEvent == FALSE) {
    GotSmmReadyToLockEvent = TRUE;

    Status = gBS->LocateProtocol (&gEfiSmmBase2ProtocolGuid, NULL, &pSmmBase);
    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }
    Status = pSmmBase->GetSmstLocation (pSmmBase, &pSmst);
    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }

    Status = pSmst->SmmLocateProtocol (&gSmmMeHeci3ProtocolGuid, NULL, &pSmmHeci);
    if (!EFI_ERROR (Status)) {
      // Store HECI BAR to verify if not changed upon execution
      HeciMBar = HeciMbarReadFull (&pSmmHeci->HeciDev, TRUE);
    }
  }

  return EFI_SUCCESS;
}

/*****************************************************************************
 * Public functions.
 *****************************************************************************/
/**
 * SMM HECI driver entry point.
 *
 * param[in] ImageHandle    Handle of driver image
 * param[in] pSysTable      Pointer to the system table
 *
 * return EFI_STATUS is returned.
 */
EFI_STATUS
EFIAPI
SmmHeci3EntryPoint(
  IN     EFI_HANDLE          ImageHandle,
  IN     EFI_SYSTEM_TABLE   *pSysTable)
{
  EFI_STATUS                 Status;
  BOOLEAN                    InSmm;
  EFI_SMM_BASE2_PROTOCOL    *pSmmBase;
  EFI_SMM_SYSTEM_TABLE2     *pSmst;
  SMM_ME_HECI3_PROTOCOL     *pSmmHeci;
  EFI_HANDLE                 Handle;
  VOID                      *Registration;

  if( !MeTypeIsSps() ) {
    DEBUG((DEBUG_ERROR, "[HECI-3] SMM driver not installed because of non-SPS firmware.\n"));
    return EFI_SUCCESS;
  }

  Status = gBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &pSmmBase);
  if (EFI_ERROR(Status))
  {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }
  //
  // Test if the entry point is running in SMM mode. If not just return.
  // In SMM mode install the HECI SMM service.
  //
  InSmm = FALSE;
  pSmmBase->InSmm(pSmmBase, &InSmm);
  if (!InSmm)
  {
    return EFI_UNSUPPORTED;
  }
  //
  // Create database record and add to database
  //
  Status = pSmmBase->GetSmstLocation(pSmmBase, &pSmst);
  if (EFI_ERROR (Status))
  {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }
  Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, sizeof(*pSmmHeci), (VOID *)&pSmmHeci);
  if (EFI_ERROR(Status))
  {
    ASSERT_EFI_ERROR(Status);
    return Status;
  }
  //
  // Initialize SMM HECI protocol data
  //
  pSmmHeci->HeciDev.Seg = ME_SEGMENT;
  pSmmHeci->HeciDev.Bus = ME_BUS;
  pSmmHeci->HeciDev.Dev = ME_DEV;
  pSmmHeci->HeciDev.Fun = ME_FUN_HECI3;
  pSmmHeci->HeciDev.Hidm = HECI_HIDM_MSI;
  pSmmHeci->HeciDev.Mbar = ME_HECI3_MBAR_DEFAULT;
  pSmmHeci->HeciInit = (SMM_ME_HECI3_INIT)SmmHeciInit;
  pSmmHeci->HeciQueReset = (SMM_ME_HECI3_QUE_RESET)SmmHeciQueReset;
  pSmmHeci->HeciQueState = (SMM_ME_HECI3_QUE_STATE)SmmHeciQueState;
  pSmmHeci->HeciRequest = (SMM_ME_HECI3_REQUEST)SmmHeciRequest;
  pSmmHeci->HeciSend = (SMM_ME_HECI3_SEND)HeciMsgSend;
  pSmmHeci->HeciRecv = (SMM_ME_HECI3_RECIEVE)HeciMsgRecv;
  Handle = NULL;
  //
  // Install the SMM HECI API
  //
  Status = SmmHeciInit(pSmmHeci, NULL);
  if (Status == EFI_NOT_FOUND) {
    DEBUG ((DEBUG_WARN, "[HECI-3] WARNING: Device disabled, SMM driver not installed.\n"));
    pSmst->SmmFreePool (pSmmHeci);
    return Status;
  }  
  ASSERT_EFI_ERROR (Status);
  Status = pSmst->SmmInstallProtocolInterface(&Handle, &gSmmMeHeci3ProtocolGuid,
                                                EFI_NATIVE_INTERFACE, pSmmHeci);
  ASSERT_EFI_ERROR(Status);
  if (EFI_ERROR(Status))
  {
    DEBUG((DEBUG_ERROR, "[HECI-3] ERROR: SMM driver not installed\n"));
    pSmst->SmmFreePool(pSmmHeci);
    HeciMBar = 0;
    ASSERT_EFI_ERROR (Status);
  } else {
    //
    // register SMM Ready To Lock Protocol notification
    //
    Status = pSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmReadyToLockProtocolGuid,
                    SmmReadyToLockEventNotify,
                    &Registration
                    );
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
} // SmmHeciEntryPoint()


/**
 * Initialize HECI interface.
 *
 * This function initializes Host side of HECI interface. If timeout is
 * greater than zero it also waits until ME is ready to receive messages.
 *
 * @param[in,out] pThis     Pointer to protocol structure
 * @param[in,out] pTimeout  On input timeout in ms, on exit time left
 */
EFI_STATUS EFIAPI SmmHeciInit(
  IN     SMM_ME_HECI3_PROTOCOL *pThis,
  IN OUT UINT32                *pTimeout)
{
  EFI_STATUS  Status;

  Status = HeciInit (&pThis->HeciDev, pTimeout);
  if ((!EFI_ERROR (Status)) &&
      (GotSmmReadyToLockEvent == FALSE)) {
    HeciMBar = HeciMbarReadFull (&pThis->HeciDev, TRUE);
  }

  return Status;
} // SmmHeciInit()


/**
 * Reset HECI queue.
 *
 * This function resets HECI queue. If timeout is greater than zero it also
 * waits until ME is ready to receive messages.
 *
 * @param[in,out] pThis     Pointer to protocol data
 * @param[in,out] pTimeout  On input timeout in us, on exit time left
 */
EFI_STATUS EFIAPI SmmHeciQueReset(
  IN     SMM_ME_HECI3_PROTOCOL *pThis,
  IN OUT UINT32                *pTimeout)
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT64      CurrentHeciMBar;

  CurrentHeciMBar = PrepareHeciMbar (&pThis->HeciDev);

  if (HeciMBar == HeciMbarReadFull (&pThis->HeciDev, TRUE)) {
    Status = HeciQueReset (&pThis->HeciDev, pTimeout);
  }
  SetHeciMbar (CurrentHeciMBar, &pThis->HeciDev);

  return Status;
} // SmmHeciReset()


/**
 * Get HECI queue state.
 *
 * This function reads HECI queue state. It informs whether queue is ready for
 * communication, and whether there are some dwords in send or receive queue.
 * If SmmHeciRequest() is called and queue is not empty reset is done to clear
 * it. SmmHeciQueState() may be used to detect this situation and if possible
 * delay the SMM request, so that OS driver can finish its transaction.
 *
 * @param[in,out] pThis     Pointer to protocol data
 * @param[in,out] pTimeout  On input timeout in us, on exit time left
 */
EFI_STATUS EFIAPI SmmHeciQueState(
  IN OUT SMM_ME_HECI3_PROTOCOL *pThis,
     OUT BOOLEAN               *pIsReady,
     OUT UINT32                *pSendQue,
     OUT UINT32                *pRecvQue)
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT64      CurrentHeciMBar;

  CurrentHeciMBar = PrepareHeciMbar (&pThis->HeciDev);

  if (HeciMBar == HeciMbarReadFull (&pThis->HeciDev, TRUE)) {
    Status = HeciQueState (&pThis->HeciDev, pIsReady, pSendQue, pRecvQue);
  }
  SetHeciMbar (CurrentHeciMBar, &pThis->HeciDev);

  return Status;
} // SmmHeciQueState()


/**
 * Send request message to HECI queue, wait for response if needed.
 *
 * This function writes one message to HECI queue and - if receive buffer
 * was provided and timeout is greater than zero - waits for response message.
 * Fragmentation is not supported. Reqeust and response must be unfragmented.
 * Size of receive message buffer is given in bytes in (*pBufLen) on input.
 * On exit (*pBufLen) provides the number of bytes written to the message
 * buffer. If buffer is too INT16 the message is truncated.
 *
 * @param[in]     pThis      Pointer to protocol data
 * @param[in,out] pTimeout   On input timeout in ms, on exit time left
 * @param[in]     pReqMsg    Request message
 * @param[out]    pMsgBuf    Buffer for the response message
 * @param[in,out] pBufLen    On input buffer size, on exit message, in bytes
 */
EFI_STATUS EFIAPI SmmHeciRequest(
  IN     SMM_ME_HECI3_PROTOCOL *pThis,
  IN OUT UINT32                *pTimeout,
  IN     HECI_MSG_HEADER       *pReqMsg,
     OUT HECI_MSG_HEADER       *pRspBuf,
  IN     UINT32                *pBufLen)
{
  EFI_STATUS  Status = EFI_UNSUPPORTED;
  UINT64      CurrentHeciMBar;

  CurrentHeciMBar = PrepareHeciMbar (&pThis->HeciDev);
  if (HeciMBar == HeciMbarReadFull (&pThis->HeciDev, TRUE)) {
    //
    // Send the request and wait for response if response expected
    //
    Status = HeciMsgSend (&pThis->HeciDev, pTimeout, pReqMsg);
    if (!EFI_ERROR (Status)) {
      if (pRspBuf != NULL) {
        Status = HeciMsgRecv (&pThis->HeciDev, pTimeout, pRspBuf, pBufLen);
      }
    }
    HeciQueReset (&pThis->HeciDev, pTimeout);
  }
  SetHeciMbar (CurrentHeciMBar, &pThis->HeciDev);

  return Status;
} // SmmHeciRequest()

