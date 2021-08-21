/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2008 - 2018 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Hecicore.c

Abstract:

  Heci driver core. For Dxe Phase, determines the HECI device and initializes it.

--*/
#include <Base.h>
#include <Uefi.h>
#include <PiDxe.h>


#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>

#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/PciLib.h>

#include <Protocol/HeciSmm.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BasePrintLib/PrintLibInternal.h>
#include <Library/BaseLib.h>
#include <Library/CpuLib.h>
#include <Library/MmPciLib.h>
#include <Library/PchPcrLib.h>

#include <Protocol/Smbios.h>
#include <IndustryStandard/Smbios.h>

#include <Register/PchRegsPsf.h>    //Common definitions 
#include <Register/PchRegsPsfCnl.h> //Project specific definitions

#include <PchAccess.h>

#include <MeHeciRegs.h>
#include <HeciRegs.h>
#include <MeState.h>

#include "HeciCore.h"


extern HECI_INSTANCE_SMM  *mHeciContext;

#define HECI_SINGLE_READ_TIMEOUT 500000   // 500 ms timeout in microseconds


/*
 * Read HECI MBAR, assign default if not assigned.
 *
 * return 64-bit MBAR is returned, or NULL if HECI is not enabled.
 */
UINTN
HeciMbarRead (VOID)
{
  return HeciMbarReadFull (TRUE);
}

/*
 * Read HECI MBAR, assign default if not assigned.
 *
 * return 64-bit MBAR is returned, or NULL if HECI is not enabled.
 */
UINTN
HeciMbarReadFull (
  BOOLEAN CleanBarTypeBits
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
  
  Mbar.QWord = 0;
  Mbar.Bits.DWordL = PciRead32(HeciPciAddressBase + HECI_R_MBAR);
  if (Mbar.Bits.DWordL == 0xFFFFFFFF)
  {
    DEBUG((DEBUG_ERROR, "[HECI] ERROR: HECI-1 device disabled\n"));
    Mbar.Bits.DWordL = 0;
    /*
    MeEwlStateFailures (
      EwlSeverityFatal,
      ME_EWL_CHKP_MAJOR_NA,
      ME_EWL_CHKP_MINOR_NA,
      ME_EWL_WARN_MAJOR_HECI,
      ME_EWL_WARN_HECI_PCI
      );
      */
    goto GetOut;
  }
  if (CleanBarTypeBits == TRUE) {
    Mbar.Bits.DWordL &= 0xFFFFFFF0; // clear address type bits
  }
  Mbar.Bits.DWordH = PciRead32(HeciPciAddressBase + HECI_R_MBAR + 4);

  if ((Mbar.QWord & 0xFFFFFFFFFFFFFFF0) == 0) {
    Mbar.QWord = HECI1_MBAR_DEFAULT;
    DEBUG((DEBUG_WARN, "[HECI] WARNING: MBAR not programmed, using default 0x%08X%08X\n",
           Mbar.Bits.DWordH, Mbar.Bits.DWordL));
    PciWrite32(HeciPciAddressBase + HECI_R_MBAR + 4, Mbar.Bits.DWordH);
    PciWrite32(HeciPciAddressBase + HECI_R_MBAR, Mbar.Bits.DWordL);
  }
 GetOut:
  return (UINTN)Mbar.QWord;
}

UINT32 MmIoReadDword ( UINTN a ) {
  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;

  HeciRegHCsrPtr = (HECI_HOST_CONTROL_REGISTER *) a;
  return HeciRegHCsrPtr->ul;
}

VOID
MmIoWriteDword ( UINTN  a, UINT32 b ) {
  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;

  HeciRegHCsrPtr      = (HECI_HOST_CONTROL_REGISTER *) a;

  HeciRegHCsrPtr->ul  = b;
}

#define MMIOREADDWORD(a)      MmIoReadDword (a)
#define MMIOWRITEDWORD(a, b)  MmIoWriteDword (a, b)


UINT8 FilledSlots (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer );

EFI_STATUS OverflowCB (
  IN      UINT32                    ReadPointer,
  IN      UINT32                    WritePointer,
  IN      UINT32                    BufferDepth
 );

EFI_STATUS
WaitForMEReady (
  VOID
  );

//
// Heci driver function definitions
//
EFI_STATUS InitializeHeciPrivate(VOID)
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;
  EFI_STATUS                  Status;
  UINTN                       HeciPciAddressBase;

  Status = EFI_SUCCESS;

  do {
    HeciPciAddressBase = PCI_LIB_ADDRESS(ME_BUS, ME_DEV, ME_FUN_HECI1, 0);
    mHeciContext->PciAddressBase = HeciPciAddressBase;


    mHeciContext->DeviceInfo = PciRead16(HeciPciAddressBase + HECI_R_DID);


    if (mHeciContext->DeviceInfo == 0xFFFF) {
      Status = EFI_DEVICE_ERROR;
      break;
    }

    mHeciContext->RevisionInfo = PciRead8(HeciPciAddressBase + HECI_R_REVID);

    mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();
    if (mHeciContext->HeciMBAR == 0)
    {
      Status = EFI_DEVICE_ERROR;
      break;
    }

    PciOr8(HeciPciAddressBase + HECI_R_CMD, HECI_CMD_MSE | HECI_CMD_BME);

    PciAnd8(HeciPciAddressBase + HECI_R_HIDM, 0xFC);

    if (WaitForMEReady () != EFI_SUCCESS) {
      Status = EFI_TIMEOUT;
      break;
    }

    HeciRegHCsrPtr  = (VOID *) (UINTN) (mHeciContext->HeciMBAR + H_CSR);
    HeciRegHCsr.ul  = HeciRegHCsrPtr->ul;
    if (HeciRegHCsrPtr->r.H_RDY == 0) {
      HeciRegHCsr.r.H_RST = 0;
      HeciRegHCsr.r.H_RDY = 1;
      HeciRegHCsr.r.H_IG  = 1;
      HeciRegHCsrPtr->ul  = HeciRegHCsr.ul;
    }

  } while (EFI_ERROR (Status));

  return Status;
}

EFI_STATUS WaitForMEReady ( VOID ) {
  UINT32                 Timeout = (HECI_INIT_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;
  HECI_ME_CONTROL_REGISTER  HeciRegMeCsrHa;

  //
  //  Wait for ME ready
  //
  //
  // Check for ME ready status
  //
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  while (HeciRegMeCsrHa.r.ME_RDY_HRA == 0) {

    //
    // If ME reset occurred during the transaction reset it .
    //
    if (HeciRegMeCsrHa.r.ME_RST_HRA) {
      if (ResetHeciInterface () != EFI_SUCCESS) {
        return EFI_TIMEOUT;
      }
    }
    //
    // If 5s timeout has expired, return fail
    //
    if (Timeout-- == 0)
    {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay(HECI_WAIT_DELAY);

    HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  }
  //
  // ME ready!!!
  //
  return EFI_SUCCESS;
}

BOOLEAN CheckForHeciReset ( VOID ) {
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  HECI_ME_CONTROL_REGISTER    HeciRegMeCsrHa;

  //
  // Init Host & ME CSR
  //
  HeciRegHCsr.ul    = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);

  if ((HeciRegMeCsrHa.r.ME_RDY_HRA == 0) || (HeciRegHCsr.r.H_RDY == 0)) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS HeciInitialize ( VOID ) {
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Need to do following on ME init:
  //
  //  1) wait for ME_CSR_HA reg ME_RDY bit set
  //
  if (WaitForMEReady () != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }
  //
  //  2) setup H_CSR reg as follows:
  //     a) Make sure H_RST is clear
  //     b) Set H_RDY
  //     c) Set H_IG
  //
  HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  if (HeciRegHCsr.r.H_RDY == 0) {
    HeciRegHCsr.r.H_RST = 0;
    HeciRegHCsr.r.H_RDY = 1;
    HeciRegHCsr.r.H_IG  = 1;
    MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);
  }

  return EFI_SUCCESS;
}

EFI_STATUS  HeciReInitialize ( VOID ) {
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  EFI_STATUS                  Status;

  Status = EFI_SUCCESS;
  //
  // Need to do following on ME init:
  //
  //  1) wait for HOST_CSR_HA reg H_RDY bit set
  //
  //    if (WaitForHostReady() != EFI_SUCCESS) {
  //
  if (MeResetWait (HECI_INIT_TIMEOUT) != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }

  HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  if (HeciRegHCsr.r.H_RDY == 0) {
    Status = ResetHeciInterface ();

  }

  return Status;
}

EFI_STATUS HeciReInitialize2 ( VOID ){
  HECI_ME_CONTROL_REGISTER  HeciRegMeCsrHa;
  EFI_STATUS                Status;
  UINT32                 Timeout;
  Status = EFI_SUCCESS;

  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);

  Timeout = (HECI_INIT_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;
  while (HeciRegMeCsrHa.r.ME_RDY_HRA == 1) {
  
    if (HeciRegMeCsrHa.r.ME_RST_HRA) {
      if (ResetHeciInterface () != EFI_SUCCESS) {
        return EFI_TIMEOUT;
      }
    }

    //
    // If 5s timeout has expired, return fail
    //
    if (Timeout-- == 0)
    {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay(HECI_WAIT_DELAY);

    HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  }

  if (WaitForMEReady () != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }

  return Status;
}

EFI_STATUS HECIPacketRead (
  IN      UINT32                    Blocking,
  OUT     HECI_MSG_HEADER       *MessageHeader,
  OUT     UINT32                    *MessageData,
  IN OUT  UINT32                    *Length
  ) {
  BOOLEAN                     GotMessage;
  UINT32                      i;
  UINT32                      LengthInDwords;
  HECI_ME_CONTROL_REGISTER    HeciRegMeCsrHa;
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  UINT32                 Timeout;
  UINT32                 Timeout1;

  GotMessage = FALSE;

  HeciRegHCsr.ul      = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  HeciRegHCsr.r.H_IS  = 1;

  //
  // test for circular buffer overflow
  //
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  if (OverflowCB (
        HeciRegMeCsrHa.r.ME_CBRP_HRA,
        HeciRegMeCsrHa.r.ME_CBWP_HRA,
        HeciRegMeCsrHa.r.ME_CBD_HRA
        ) != EFI_SUCCESS) {
    //
    // if we get here, the circular buffer is overflowed
    //
    *Length = 0;
    return EFI_DEVICE_ERROR;
  }
  //
  // If NON_BLOCKING, exit if the circular buffer is empty
  //
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);;
  if ((FilledSlots(HeciRegMeCsrHa.r.ME_CBRP_HRA, HeciRegMeCsrHa.r.ME_CBWP_HRA) == 0) && !Blocking) {
    *Length = 0;
    return EFI_NO_RESPONSE;
  }

  //
  // Calculate timeout counter
  //
  Timeout = (HECI_SINGLE_READ_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;

  //
  // loop until we get a message packet
  //
  while (!GotMessage) {
    //
    // If 1s timeout has expired, return fail
    //
    if (Timeout-- == 0)
    {
       *Length = 0;
      return EFI_TIMEOUT;
    }

    //
    // Read one message from HECI buffer and advance read pointer.  Make sure
    // that we do not pass the write pointer.
    //
    HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);;
    if (FilledSlots (HeciRegMeCsrHa.r.ME_CBRP_HRA, HeciRegMeCsrHa.r.ME_CBWP_HRA) > 0) {
      //
      // Eat the HECI Message header
      //
      MessageHeader->DWord = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CB_RW);

      //
      // Compute required message length in DWORDS
      //
      LengthInDwords = ((MessageHeader->Bits.Length + 3) / 4);

      //
      // Just return success if Length is 0
      //
      if (MessageHeader->Bits.Length == 0) {
        //
        // Set Interrupt Generate bit and return
        //
        MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
        HeciRegHCsr.r.H_IG = 1;
        MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);
        *Length = 0;
        return EFI_SUCCESS;
      }
      //
      // Make sure that the message does not overflow the circular buffer.
      //
      HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
      if ((MessageHeader->Bits.Length + sizeof (HECI_MSG_HEADER)) > (HeciRegMeCsrHa.r.ME_CBD_HRA * 4)) {
        *Length = 0;
        return EFI_DEVICE_ERROR;
      }
      //
      // Make sure that the callers buffer can hold the correct number of DWORDS
      //
      if ((MessageHeader->Bits.Length) <= *Length) {

        //
        // Calculate timeout counter for inner loop
        //
        Timeout1 = (HECI_SINGLE_READ_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;

        //
        // Wait here until entire message is present in circular buffer
        //
        HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
        while (LengthInDwords > FilledSlots (HeciRegMeCsrHa.r.ME_CBRP_HRA, HeciRegMeCsrHa.r.ME_CBWP_HRA)) {
          //
          // If 1 second timeout has expired, return fail as we have not yet received a full message
          //
          if (Timeout1-- == 0)
          {
            *Length = 0;
            return EFI_TIMEOUT;
          }
          //
          // Wait before we read the register again
          //
          MicroSecondDelay(HECI_WAIT_DELAY);

          //
          // Read the register again
          //
          HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
        }
        //
        // copy rest of message
        //
        for (i = 0; i < LengthInDwords; i++) {
          MessageData[i] = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CB_RW);
        }
        //
        // Update status and length
        //
        GotMessage  = TRUE;
        *Length     = MessageHeader->Bits.Length;

      } else {
        //
        // Message packet is larger than caller's buffer
        //
        *Length = 0;
        return EFI_BUFFER_TOO_SMALL;
      }
    }
    //
    // Wait before we try to get a message again
    //
    MicroSecondDelay(HECI_WAIT_DELAY);
  }
  //
  // Read ME_CSR_HA.  If the ME_RDY bit is 0, then an ME reset occurred during the
  // transaction and the message should be discarded as bad data may have been retrieved
  // from the host's circular buffer
  //
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  if (HeciRegMeCsrHa.r.ME_RDY_HRA == 0) {
    *Length = 0;
    return EFI_DEVICE_ERROR;
  }
  //
  // Set Interrupt Generate bit
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  HeciRegHCsr.r.H_IG  = 1;
  MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);

  return EFI_SUCCESS;
}

EFI_STATUS HeciReceive (
  IN      UINT32   Blocking,
  IN OUT  UINT32  *MessageBody,
  IN OUT  UINT32  *Length
  ) {
  HECI_MSG_HEADER  PacketHeader;
  UINT32           CurrentLength;
  UINT32           MessageComplete;
  EFI_STATUS       Status;
  UINT32           PacketBuffer;
  UINT32           MeDeviceState;
  BOOLEAN          QuitFlag;

  Status          = EFI_SUCCESS;
  CurrentLength   = 0;
  MessageComplete = 0;
  QuitFlag        = FALSE;

  do {
    //
    // Enable HECI and Save the Device State
    //
    Heci1DevSaveEnable (&MeDeviceState);

    //
    // Make sure that HECI device BAR is correct and device is enabled.
    //
    mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();

    //
    // Make sure we do not have a HECI reset
    //
    if (CheckForHeciReset ()) {
      //
      // if HECI reset than try to re-init HECI
      //
      Status = HeciInitialize ();

      if (EFI_ERROR (Status)) {
        HeciDevRestore (MeDeviceState);
        Status = EFI_DEVICE_ERROR;
        break;
      }
    }
    //
    // Make sure that HECI is ready for communication.
    //
    if (WaitForMEReady () != EFI_SUCCESS) {
      HeciDevRestore (MeDeviceState);
      Status = EFI_TIMEOUT;
      break;
    }
    //
    // Set up timer for BIOS timeout.
    //
    while ((CurrentLength < *Length) && (MessageComplete == 0)) {

      PacketBuffer = *Length - CurrentLength;
      Status = HECIPacketRead (
                Blocking,
                &PacketHeader,
                (UINT32 *) &MessageBody[CurrentLength / 4],
                &PacketBuffer
                );

      //
      // Check for error condition on read
      //
      if (EFI_ERROR (Status)) {
        *Length   = 0;
        QuitFlag  = TRUE;
        break;
      }
      //
      // Get completion status from the packet header
      //
      MessageComplete = PacketHeader.Bits.MsgComplete;

      //
      // Check for zero length messages
      //
      if (PacketBuffer == 0) {
        //
        // If we are not in the middle of a message, and we see Message Complete,
        // this is a valid zero-length message.
        //
        if ((CurrentLength == 0) && (MessageComplete == 1)) {
          *Length   = 0;
          QuitFlag  = TRUE;
          break;
        } else {
          //
          // We should not expect a zero-length message packet except as described above.
          //
          *Length   = 0;
          Status    = EFI_DEVICE_ERROR;
          QuitFlag  = TRUE;
          break;
        }
      }
      //
      // Track the length of what we have read so far
      //
      CurrentLength += PacketBuffer;

    }

    if (QuitFlag == TRUE) {
      break;
    }
    //
    // If we get here the message should be complete, if it is not
    // the caller's buffer was not large enough.
    //
    if (MessageComplete == 0) {
      *Length = 0;
      Status  = EFI_BUFFER_TOO_SMALL;
    }

    if (*Length != 0) {
      *Length = CurrentLength;
    }
    //
    // Restore HECI Device State
    //
    HeciDevRestore (MeDeviceState);

  } while (EFI_ERROR (Status) && (Status != EFI_BUFFER_TOO_SMALL));

  return Status;
}

EFI_STATUS HeciSend(
  IN UINT32                  *Message,
  IN UINT32                   Length,
  IN UINT8                    HostAddress,
  IN UINT8                    MeAddress
  ) {
  UINT32                      CBLength;
  UINT32                      SendLength;
  UINT32                      CurrentLength;
  HECI_MSG_HEADER             MessageHeader;
  EFI_STATUS                  Status;
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  UINT32                      MeDeviceState;

  Status        = EFI_SUCCESS;
  CurrentLength = 0;

  do {
    //
    // Enable HECI and Save the Device State
    //
    Heci1DevSaveEnable (&MeDeviceState);

    //
    // Make sure that HECI device BAR is correct and device is enabled.
    //
    mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();

    //
    // Make sure we do not have a HECI reset
    //
    if (CheckForHeciReset ()) {
      //
      // if HECI reset than try to re-init HECI
      //
      Status = HeciInitialize ();

      if (EFI_ERROR (Status)) {
        HeciDevRestore (MeDeviceState);
        Status = EFI_DEVICE_ERROR;
        break;
      }
    }
    //
    // Make sure that HECI is ready for communication.
    //
    if (WaitForMEReady () != EFI_SUCCESS) {
      HeciDevRestore (MeDeviceState);
      Status = EFI_TIMEOUT;
      break;
    }
    //
    // Set up memory mapped registers
    //
    HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);

    //
    // Grab Circular Buffer length
    //
    CBLength = HeciRegHCsr.r.H_CBD;

    //
    // Prepare message header
    //
    MessageHeader.DWord             = 0;
    MessageHeader.Bits.MeAddress    = MeAddress;
    MessageHeader.Bits.HostAddress  = HostAddress;

    //
    // Break message up into CB-sized packets and loop until completely sent
    //
    while (Length > CurrentLength) {
      //
      // Set the Message Complete bit if this is our last packet in the message.
      // Needs to be 'less than' to account for the header.
      //
      if ((((Length - CurrentLength) + 3) / 4) < CBLength) {
        MessageHeader.Bits.MsgComplete = 1;
      }
      //
      // Calculate length for Message Header
      //    header length == smaller of circular buffer or remaining message (both account for the size of the header)
      //
      SendLength = ((CBLength < (((Length - CurrentLength) + 3) / 4)) ? ((CBLength - 1) * 4) : (Length - CurrentLength));
      MessageHeader.Bits.Length = SendLength;

      //
      // send the current packet (CurrentLength can be treated as the index into the message buffer)
      //
      Status = HeciPacketWrite (&MessageHeader, (UINT32 *) ((UINTN) Message + CurrentLength));
      if (EFI_ERROR (Status)) {
        break;
      }
      //
      // Update the length information
      //
      CurrentLength += SendLength;
    }

    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Restore HECI Device State
    //
    HeciDevRestore (MeDeviceState);

  } while (EFI_ERROR (Status));

  return Status;
}


EFI_STATUS
HeciPacketWrite (
  IN  HECI_MSG_HEADER        *MessageHeader,
  IN  UINT32                 *MessageData
  ) {
  UINT32                      Timeout;
  UINT32                      i;
  UINT32                      LengthInDwords;
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  HECI_ME_CONTROL_REGISTER    HeciRegMeCsrHa;

  if (WaitForMEReady () != EFI_SUCCESS) {
    return EFI_TIMEOUT;
  }
  //
  // Compute message length in DWORDS
  //
  LengthInDwords = ((MessageHeader->Bits.Length + 3) / 4);

  //
  // Set up timeout counter
  //
  Timeout = (HECI_SEND_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;

  //
  // Wait until there is sufficient room in the circular buffer
  // Must have room for message and message header
  //
  HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  while (LengthInDwords > (HeciRegHCsr.r.H_CBD - FilledSlots (HeciRegHCsr.r.H_CBRP, HeciRegHCsr.r.H_CBWP))) {
    //
    // If 1 second timeout has expired, return fail as the circular buffer never emptied
    //
    if (Timeout-- == 0)
    {
      return EFI_TIMEOUT;
    }
    //
    // Wait before we read the register again
    //
    MicroSecondDelay(HECI_WAIT_DELAY);

    //
    // Read Host CSR for next iteration
    //
    HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  }
  //
  // Write Message Header
  //
  MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CB_WW, MessageHeader->DWord);

  //
  // Write Message Body
  //
  for (i = 0; i < LengthInDwords; i++) {
    MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CB_WW, MessageData[i]);
  }
  //
  // Set Interrupt Generate bit
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  HeciRegHCsr.r.H_IG  = 1;
  MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);

  //
  // Test if ME Ready bit is set to 1, if set to 0 a fatal error occured during
  // the transmission of this message.
  //
  HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  if (HeciRegMeCsrHa.r.ME_RDY_HRA == 0) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciSendwACK (
  IN OUT  UINT32  *Message,
  IN      UINT32  Length,
  IN OUT  UINT32  *RecLength,
  IN      UINT8   HostAddress,
  IN      UINT8   MeAddress
  )
/*++

  Routine Description:
    Function sends one messsage through the HECI circular buffer and waits
    for the corresponding ACK message.

  Arguments:
    Message     - Pointer to the message buffer.
    SendLength  - Length of the message in bytes.
    RecLength   - Length of the message response in bytes.
    HostAddress - Address of the sending entity.
    MeAddress   - Address of the ME entity that should receive the message.

  Returns:
    EFI_STATUS

--*/
{
  EFI_STATUS  Status;
  UINT16      RetryCount;
  UINT32      TempRecLength;

  //
  // Send the message
  //
  Status = HeciSend (Message, Length, HostAddress, MeAddress);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Wait for ACK message
  //
  TempRecLength = *RecLength;
  for (RetryCount = 0; RetryCount < HECI_MAX_RETRY; RetryCount++) {
    //
    // Read Message
    //
    Status = HeciReceive(TRUE, Message, &TempRecLength);
    if (!EFI_ERROR (Status)) {
      break;
    }
    //
    // Reload receive length as it has been modified by the read function
    //
    TempRecLength = *RecLength;
  }
  //
  // Return read length and status
  //
  *RecLength = TempRecLength;
  return Status;
}


EFI_STATUS
EFIAPI
MeResetWait (
  IN  UINT32  Delay
  )
/*++

Routine Description:

  Me reset and waiting for ready

Arguments:

  Delay - The biggest waiting time

Returns:

  EFI_TIMEOUT - Time out
  EFI_SUCCESS - Me ready

--*/
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Wait for the HOST Ready bit to be cleared to signal a reset
  //
  HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  while (HeciRegHCsr.r.H_RDY == 1) {
    //
    // If timeout has expired, return fail
    //
    if (Delay-- == 0)
    {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay(1);

    HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ResetHeciInterface (
  VOID
  )
/*++

  Routine Description:
    Function forces a reinit of the heci interface by following the reset heci interface via host algorithm
    in HPS 0.90 doc 4-17-06 njy

  Arguments:
    none

  Returns:
    EFI_STATUS

--*/
{
  HECI_HOST_CONTROL_REGISTER  HeciRegHCsr;
  HECI_ME_CONTROL_REGISTER    HeciRegMeCsrHa;
  UINT32 Timeout = (HECI_INIT_TIMEOUT + HECI_WAIT_DELAY / 2) / HECI_WAIT_DELAY;

  //
  // Make sure that HECI device BAR is correct and device is enabled.
  //
  mHeciContext->HeciMBAR = CheckAndFixHeciForAccess ();

  //
  // Enable Reset
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  HeciRegHCsr.r.H_RST = 1;
  HeciRegHCsr.r.H_IG  = 1;
  MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);

  //
  // Make sure that the reset started
  //
  // HeciRegHCsr.ul = MMIOREADDWORD(mHeciContext->HeciMBAR + H_CSR);
  //
  do {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout-- == 0)
    {
       return EFI_TIMEOUT;
    }
    MicroSecondDelay(HECI_WAIT_DELAY);
    //
    // Read the ME CSR
    //
    HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  } while (HeciRegHCsr.r.H_RDY == 1);

  //
  // Wait for ME to perform reset
  //
  // HeciRegMeCsrHa.ul = MMIOREADDWORD(mHeciContext->HeciMBAR + ME_CSR_HA);
  //
  do {
    if (Timeout-- == 0)
    {
      return EFI_TIMEOUT;
    }
    MicroSecondDelay(HECI_WAIT_DELAY);
    //
    // Read the ME CSR
    //
    HeciRegMeCsrHa.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + ME_CSR_HA);
  } while (HeciRegMeCsrHa.r.ME_RDY_HRA == 0);

  //
  // Make sure IS has been signaled on the HOST side
  //
  // HeciRegHCsr.ul = MMIOREADDWORD(mHeciContext->HeciMBAR + H_CSR);
  //
  do {
    //
    // If 5 second timeout has expired, return fail
    //
    if (Timeout-- == 0)
    {
     return EFI_TIMEOUT;
    }
    MicroSecondDelay(HECI_WAIT_DELAY);
    //
    // Read the ME CSR
    //
    HeciRegHCsr.ul = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);
  } while (HeciRegHCsr.r.H_IS == 0);

  //
  // Enable host side interface
  //
  HeciRegHCsr.ul      = MMIOREADDWORD (mHeciContext->HeciMBAR + H_CSR);;
  HeciRegHCsr.r.H_RST = 0;
  HeciRegHCsr.r.H_IG  = 1;
  HeciRegHCsr.r.H_RDY = 1;
  MMIOWRITEDWORD (mHeciContext->HeciMBAR + H_CSR, HeciRegHCsr.ul);

  return EFI_SUCCESS;
}

UINT8
FilledSlots (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer
  )
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed.
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer  - Location of the read pointer.
    WritePointer - Location of the write pointer.

  Returns:
    Number of filled slots.

--*/
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  return FilledSlots;
}

EFI_STATUS
OverflowCB (
  IN  UINT32 ReadPointer,
  IN  UINT32 WritePointer,
  IN  UINT32 BufferDepth
  )
/*++

  Routine Description:
    Calculate if the circular buffer has overflowed
    Corresponds to HECI HPS (part of) section 4.2.1

  Arguments:
    ReadPointer - Value read from host/me read pointer
    WritePointer - Value read from host/me write pointer
    BufferDepth - Value read from buffer depth register

  Returns:
    EFI_STATUS

--*/
{
  UINT8 FilledSlots;

  //
  // Calculation documented in HECI HPS 0.68 section 4.2.1
  //
  FilledSlots = (((INT8) WritePointer) - ((INT8) ReadPointer));

  //
  // test for overflow
  //
  if (FilledSlots > ((UINT8) BufferDepth)) {
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciGetMeStatus (
  IN UINT32                     *MeStatus
  )
/*++

  Routine Description:
    Return ME Status

  Arguments:
    MeStatus pointer for status report

  Returns:
    EFI_STATUS

--*/
{
  HECI_FWS_REGISTER MeFirmwareStatus;
  UINT32            MeDeviceState;

  if (MeStatus == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Save HECI1 Device State and Enable it
  //
  Heci1DevSaveEnable (&MeDeviceState);

  MeFirmwareStatus.ul = MeHeciPciReadMefs1();

  if (MeFirmwareStatus.r.CurrentState == ME_STATE_NORMAL && MeFirmwareStatus.r.ErrorCode == ME_ERROR_CODE_NO_ERROR) {
    *MeStatus = ME_READY;
  } else if (MeFirmwareStatus.r.CurrentState == ME_STATE_RECOVERY) {
    *MeStatus = ME_IN_RECOVERY_MODE;
  } else {
    *MeStatus = ME_NOT_READY;
  }

  if (MeFirmwareStatus.r.FwInitComplete == ME_FIRMWARE_COMPLETED) {
    *MeStatus |= ME_FW_INIT_COMPLETE;
  }
  //
  // Save HECI Device State and Enable it
  //
  HeciDevRestore (MeDeviceState);

  //
  // DEBUG ((EFI_D_ERROR, "HECI MeStatus %X\n", MeFirmwareStatus.ul));
  //
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
HeciGetMeMode (
  IN UINT32                     *MeMode
  )
/*++

  Routine Description:
    Return ME Mode

  Arguments:
    MeMode pointer for ME Mode report

  Returns:
    EFI_STATUS

--*/
{
  HECI_FWS_REGISTER MeFirmwareStatus;
 
  if (MeMode == NULL) {
    return EFI_INVALID_PARAMETER;
  }
 
  MeFirmwareStatus.ul = MeHeciPciReadMefs1();
 
  switch (MeFirmwareStatus.r.MeOperationMode) {
    case ME_OPERATION_MODE_NORMAL:
    case ME_OPERATION_MODE_SPS: // UP_SPS_SUPPORT: changed from ME_MODE_SPS to ME_MODE_NORMAL
      *MeMode = ME_MODE_NORMAL;
      break;
 
    case ME_OPERATION_MODE_DEBUG:
      *MeMode = ME_MODE_DEBUG;
      break;
 
    case ME_OPERATION_MODE_SOFT_TEMP_DISABLE:
      *MeMode = ME_MODE_TEMP_DISABLED;
      break;
 
    case ME_OPERATION_MODE_SECOVR_JMPR:
    case ME_OPERATION_MODE_SECOVR_HECI_MSG:
      *MeMode = ME_MODE_SECOVER;
      break;
 
    default:
      *MeMode = ME_MODE_FAILED;
  }
  DEBUG ((DEBUG_ERROR, "HECI MeMode %X\n", MeFirmwareStatus.r.MeOperationMode));
 
  return EFI_SUCCESS;
}

EFI_STATUS
Heci1DevSaveEnable (
  IN OUT  UINT32 *DevState
  )
/*++

  Routine Description:
   Save HECI1 State and Enable it

  Arguments:
    DevState          - Device State Save Buffer

  Returns:
    EFI_STATUS

--*/
{

  *DevState = MmioRead32 (PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN));
  MmioWrite32 (PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI2_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN),
               *DevState & (UINT32)~B_PCH_PSFX_PCR_T0_SHDW_PCIEN_FUNDIS);

  return EFI_SUCCESS;
}

EFI_STATUS
HeciDevRestore (
  IN  UINT32 DevState
  )
/*++

  Routine Description:
  Restore HECI1&HECI2 State

  Arguments:
    DevState          - Device State Save Buffer

  Returns:
    EFI_STATUS

--*/
{
  MmioAndThenOr32 (
    PCH_PCR_ADDRESS (PID_PSF1, R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE + R_PCH_PSFX_PCR_T0_SHDW_PCIEN),
    (UINT32)~B_PCH_PSFX_PCR_T0_SHDW_PCIEN_FUNDIS, DevState);
  
  return EFI_SUCCESS;
}

