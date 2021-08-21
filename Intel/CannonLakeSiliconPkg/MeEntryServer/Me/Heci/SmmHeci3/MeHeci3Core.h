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

#ifndef _ME_HECI3_CORE_H_
#define _ME_HECI3_CORE_H_

#define HECI_TIMEOUT_UNIT 1000  // Timout unit in us for the HECI functions


/**
 * Initialize HECI interface.
 *
 * This function initializes Host side of HECI interface. If timeout is
 * greater than zero it also waits until ME is ready to receive messages.
 *
 * @param[in,out] pThis     Pointer to HECI device structure
 * @param[in,out] pTimeout  On input timeout in ms, on exit time left
 */
EFI_STATUS EFIAPI
HeciInit(
  IN OUT ME_HECI_DEVICE   *pThis,
  IN OUT UINT32           *pTimeout);

/**
 * Set HECI interrupt enable bit in HECI CSR.
 *
 * This function sets HECI interrupt enable bit. It may enable or disable HECI
 * interrupt.
 *
 * NOTE: It is dedicated for SMM. DXE and PEI drivers must not use interrupt.
 *
 * @param[in,out] pThis      Pointer to HECI device structure
 * @param[in]     IntEnabled TRUE enables HECI interrupt, FALSE disables
 */
EFI_STATUS EFIAPI
HeciIntEnable(
  IN OUT ME_HECI_DEVICE   *pThis,
     OUT BOOLEAN          IntEnabled);

/**
 * Get HECI interrupt state.
 *
 * This function reads HECI interrupt state. Whether it is enabled and whether
 * it is currently signalled.
 *
 * @param[in,out] pThis       Pointer to HECI device structure
 * @param[out]    pIntEnabled Is HECI interrupt enabled
 * @param[out]    pIntStatus  Is HECI interrupt signalled
 */
EFI_STATUS EFIAPI
HeciIntState(
  IN OUT ME_HECI_DEVICE   *pThis,
     OUT BOOLEAN          *pIntEnabled,
     OUT BOOLEAN          *pIntStatus);

/**
 * Reset HECI interface.
 *
 * This function resets HECI queue. If timeout is greater than zero it also
 * waits until ME is ready to receive messages.
 *
 * @param[in,out] pThis      Pointer to HECI device structure
 * @param[in,out] pTimeout   On input timeout in ms, on exit time left
 */
EFI_STATUS EFIAPI
HeciQueReset(
  IN OUT ME_HECI_DEVICE   *pThis,
  IN OUT UINT32           *pTimeout);

/**
 * Get HECI queue state.
 *
 * This function reads HECI queue state. Whether it is ready for communication
 * and whether there are any messages in the queue.
 *
 * @param[in,out] pThis     Pointer to HECI device structure
 * @param[out]    pIsReady  Is HECI ready for communication
 * @param[out]    pSendQue  Number of DWords in send queue
 * @param[out]    pRecvQue  Number of DWords in receive queue
 */
EFI_STATUS EFIAPI
HeciQueState(
  IN OUT ME_HECI_DEVICE   *pThis,
     OUT BOOLEAN          *pIsReady,
     OUT UINT32           *pSendQue,
     OUT UINT32           *pRecvQue);

/**
 * Write one message to HECI queue.
 *
 * This function puts one message to HECI queue. If timeout is greater than
 * zero the function may wait for space in the queue. This function handles
 * only messages shorter than HECI queue length. Fragmentation of large
 * messages must be done by upper layer protocol.
 *
 * @param[in,out] pThis      Pointer to HECI device structure
 * @param[in,out] pTimeout   On input timeout in ms, on exit time left
 * @param[in]     pMessage   The header of the message to send
 */
EFI_STATUS EFIAPI
HeciMsgSend(
  IN OUT ME_HECI_DEVICE     *pThis,
  IN OUT UINT32             *pTimeout,
  IN     HECI_MSG_HEADER    *pMessage);

/**
 * Read one message from HECI queue.
 *
 * This function reads one message from HECI queue. If timeout is greater than
 * zero the function may wait for the message. Size of message buffer is given
 * in bytes in (*pBufLen) on input. On exit (*pBufLen) provides the number of
 * bytes written to the message buffer. If buffer is too INT16 the message
 * is truncated.
 *
 * @param[in,out] pThis      Pointer to HECI device structure
 * @param[in,out] pTimeout   On input timeout in ms, on exit time left
 * @param[out]    pMsgBuf    Buffer for the received message
 * @param[in,out] pBufLen    On input buffer size, on exit message, in bytes
 */
EFI_STATUS EFIAPI
HeciMsgRecv(
  IN OUT ME_HECI_DEVICE     *pThis,
  IN OUT UINT32             *pTimeout,
     OUT HECI_MSG_HEADER    *pMsgBuf,
  IN     UINT32             *pBufLen);


#endif // _ME_HECI3_CORE_H_

