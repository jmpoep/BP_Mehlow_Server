/** @file
  Initializes PCH Storage and Communications Subsystem Controllers.

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
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/TimerLib.h>
#include <Library/PciSegmentLib.h>
#include <Protocol/BlockIo.h>
#include "PchInit.h"
#include <Library/PchInfoLib.h>
#include <ConfigBlock/ScsConfig.h>
#include <IndustryStandard/Pci30.h>
#include <Register/PchRegsScsSd.h>

//
// HS400 Tuning Definitions
//
#define RX_STROBE_DLL1_TAP_MAX_RANGE          39
#define RX_STROBE_DLL1_TAP_MIN_RANGE          0
#define RX_STROBE_DLL1_TAP_MIN_MEPT           5
#define RX_STROBE_DLL1_TAP_MAX_MEPT           16
#define TX_DATA_DLL_TAP_MAX_RANGE             79
#define TX_DATA_DLL_TAP_MIN_RANGE             0
#define TX_DATA_DLL_TAP_MIN_MEPT              4
#define TX_DATA_DLL_TAP_MAX_MEPT              22

//
// Command Definitions
//
#define CMD6                                  6
#define CMD8                                  8
#define CMD13                                 13
#define CMD31                                 31
#define SWITCH                                CMD6
#define SEND_EXT_CSD                          CMD8
#define SEND_STATUS                           CMD13
#define SEND_WRITE_PROT_TYPE                  CMD31
#define WRITE_BYTE_MODE                       3
#define BLOCK_LENGTH                          512
#define TIMEOUT_COMMAND                       100
#define TIMEOUT_DATA                          5000
#define HS_TIMING_INDEX                       185
#define BUS_WIDTH_INDEX                       183

//
// Card Status Definitions
//
#define SWITCH_ERROR                          BIT7
#define ERASE_RESET                           BIT13
#define WP_ERASE_SKIP                         BIT15
#define CID_CSD_OVERWRITE                     BIT16
#define ERROR                                 BIT19
#define CC_ERROR                              BIT20
#define CARD_ECC_FAILED                       BIT21
#define ILLEGAL_COMMAND                       BIT22
#define COM_CRC_ERROR                         BIT23
#define LOCK_UNLOCK_FAILED                    BIT24
#define CARD_IS_LOCKED                        BIT25
#define WP_VIOLATION                          BIT26
#define ERASE_PARAM                           BIT27
#define ERASE_SEQ_ERROR                       BIT28
#define BLOCK_LEN_ERROR                       BIT29
#define ADDRESS_MISALIGN                      BIT30
#define ADDRESS_OUT_OF_RANGE                  BIT31

typedef enum {
  Hs200 = 0,
  Hs400,
  DDR50,
  SDR25
} EMMC_MODE;

typedef enum {
  RxDll1 = 0,
  RxDll2
} RX_STROBE_DLL_REG;

typedef enum {
  NotAvailable = 0,
  Passed,
  Failed
} BLOCK_READ_WRITE_STATUS;

typedef enum {
  ResponseNo = 0,
  ResponseR1,
  ResponseR1b,
  ResponseR2,
  ResponseR3,
  ResponseR4,
  ResponseR5,
  ResponseR5b,
  ResponseR6,
  ResponseR7
} RESPONSE_TYPE;

typedef enum {
  NoData = 0,
  InData,
  OutData
} TRANSFER_TYPE;

typedef struct {
  UINT32  CmdSet:              3;
  UINT32  Reserved0:           5;
  UINT32  Value:               8;
  UINT32  Index:               8;
  UINT32  Access:              2;
  UINT32  Reserved1:           6;
} SWITCH_ARGUMENT;

//
// PCH_EMMC_TUNING PROTOCOL Global Variable
//
GLOBAL_REMOVE_IF_UNREFERENCED PCH_EMMC_TUNING_PROTOCOL PchEmmcTuningProtocol = {
  ConfigureEmmcHs400Mode
};

/**
  Set max clock frequency of the host, the actual frequency may not be the same
  as MaxFrequency. It depends on the max frequency the host can support, divider,
  and host speed mode.

  @param[in] This                 Pointer to EFI_SD_HOST_IO_PROTOCOL
  @param[in] MaxFrequency         Max frequency in HZ

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             The timeout time expired.
**/
EFI_STATUS
EFIAPI
SetClockFrequency (
  IN UINTN                       EmmcBaseAddress,
  IN  UINT32                     MaxFrequency
  )
{
  UINT16                 Data16;
  UINT32                 Frequency;
  UINT32                 Divider = 0;
  UINT32                 TimeOutCount;
  UINT32                 CurrentClockInKHz;

  DEBUG ((DEBUG_INFO, "SetClockFrequency: BaseClockInMHz = %d \n", 200));

  Frequency = (200 * 1000 * 1000) / MaxFrequency;
  DEBUG ((DEBUG_INFO, "SetClockFrequency: FrequencyInHz = %d \n", Frequency));

  if ((200 * 1000 * 1000 % MaxFrequency) != 0) {
    Frequency += 1;
  }

  Divider = 1;
  while (Frequency > Divider) {
    Divider = Divider * 2;
  }
  if (Divider >= 0x400) {
    Divider = 0x200;
  }
  Divider = Divider >> 1;

  DEBUG ((DEBUG_INFO, "SetClockFrequency: after shift: Base Clock Divider = 0x%x \n", Divider));

  CurrentClockInKHz = (200 * 1000);
  if (Divider != 0) {
    CurrentClockInKHz = CurrentClockInKHz / (Divider * 2);
  }
  //
  //Set frequency
  //  Bit[15:8] SDCLK Frequency Select at offset 2Ch
  //    80h - base clock divided by 256
  //    40h - base clock divided by 128
  //    20h - base clock divided by 64
  //    10h - base clock divided by 32
  //    08h - base clock divided by 16
  //    04h - base clock divided by 8
  //    02h - base clock divided by 4
  //    01h - base clock divided by 2
  //    00h - Highest Frequency the target support (10MHz-63MHz)
  //
  //  Bit [07:06] are assigned to bit 09-08 of clock divider in SDCLK Frequency Select on SD controller 3.0
  //

  Data16 = (UINT16) ((Divider & 0xFF) << 8 | (((Divider & 0xFF00) >> 8) << 6));

  DEBUG ((DEBUG_INFO,
          "SetClockFrequency: base=%dMHz, clkctl=0x%04x, f=%dKHz\n",
          200,
          Data16,
          CurrentClockInKHz
          ));
  DEBUG ((DEBUG_INFO, "SetClockFrequency: set MMIO_CLKCTL value = 0x%x \n", Data16));

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL, Data16);
  Data16 |= BIT0;

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL, Data16);
  TimeOutCount = 1000;
  do {
    Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL);
    MicroSecondDelay (100);
    TimeOutCount--;
    if (TimeOutCount == 0) {
      DEBUG ((DEBUG_INFO, "SetClockFrequency: Timeout\n"));
      return EFI_TIMEOUT;
    }
  } while ((Data16 & BIT1) != BIT1);

  Data16 |= BIT2;
  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL, Data16);
  return EFI_SUCCESS;
}

/**
  Get Error Reason from Host

  @param[in] CommandIndex         Command Index which return error
  @param[in] ErrorCode            Command Error Code

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Command Timeout Error
  @retval EFI_TIMEOUT             Data Timeout Error
  @retval EFI_CRC_ERROR           Command or Data CRC Error
  @retval EFI_DEVICE_ERROR        Command End Bit Error
                                  Command Index Error
                                  Data End Bit Error
                                  Current Limit Error
                                  Auto CMD12 Error
                                  ADMA Error
**/
STATIC
EFI_STATUS
EmmcGetErrorReason (
  IN  UINT16    CommandIndex,
  IN  UINT16    ErrorCode
  )
{
  EFI_STATUS    Status;

  Status = EFI_DEVICE_ERROR;

  DEBUG ((DEBUG_ERROR, "[%2d] -- ", CommandIndex));

  if (ErrorCode & BIT0) {
    Status = EFI_TIMEOUT;
    DEBUG ((DEBUG_ERROR, "Command Timeout Error\n"));
  }

  if (ErrorCode & BIT1) {
    Status = EFI_CRC_ERROR;
    DEBUG ((DEBUG_ERROR, "Command CRC Error\n"));
  }

  if (ErrorCode & BIT2) {
    DEBUG ((DEBUG_ERROR, "Command End Bit Error\n"));
  }

  if (ErrorCode & BIT3) {
    DEBUG ((DEBUG_ERROR, "Command Index Error\n"));
  }

  return Status;
}

/**
  Reset the host CMD and DATA Line

  @param[in] EmmcBaseAddress      Base address of MMIO register

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             The timeout time expired.
**/
EFI_STATUS
EmmcReset (
  IN UINTN               EmmcBaseAddress
  )
{
  UINT8                  Data8;
  UINT8                  ResetType;
  UINT16                 SaveClkCtl;
  UINT32                 TimeOutCount;

  ResetType = (B_SCS_MEM_SWRST_CMDLINE | B_SCS_MEM_SWRST_DATALINE);

  //
  // To improve eMMC stability, we zero the R_SCS_MEM_CLKCTL register and
  // stall for 50 microsecond before reseting the controller. We
  // restore the register setting following the reset operation.
  //
  SaveClkCtl = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL);
  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL, 0);
  MicroSecondDelay (50);
  //
  // Reset the SD host controller
  //
  MmioWrite8 (EmmcBaseAddress + R_SCS_MEM_SWRST, ResetType);

  TimeOutCount  = 1000; // 1 second timeout
  do {
    MicroSecondDelay (1 * 1000);

    TimeOutCount --;

    Data8 = MmioRead8 (EmmcBaseAddress + R_SCS_MEM_SWRST);
    if ((Data8 & ResetType) == 0) {
      break;
    }
  } while (TimeOutCount > 0);

  //
  // We now restore the R_SCS_MEM_CLKCTL register which we set to 0 above.
  //
  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_CLKCTL, SaveClkCtl);

  if (TimeOutCount == 0) {
    DEBUG ((DEBUG_ERROR, "EmmcReset: Time out \n"));
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check card status, print the DEBUG info and check the error

  @param[in] CardStatus           Status got from card status register

  @retval EFI_SUCCESS             The data was read from or written to the PCI controller.
  @retval EFI_DEVICE_ERROR        Device failed during operation
**/
EFI_STATUS
EmmcCheckCardStatus (
  IN  UINT32    CardStatus
  )
{

  EFI_STATUS   Status;
  Status = EFI_SUCCESS;
  DEBUG ((DEBUG_ERROR, "CardStatus:"));

  if (CardStatus & ADDRESS_OUT_OF_RANGE) {
    DEBUG ((DEBUG_ERROR, " ADDRESS_OUT_OF_RANGE"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ADDRESS_MISALIGN) {
    DEBUG ((DEBUG_ERROR, " ADDRESS_MISALIGN"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & BLOCK_LEN_ERROR) {
    DEBUG ((DEBUG_ERROR, " BLOCK_LEN_ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ERASE_SEQ_ERROR) {
    DEBUG ((DEBUG_ERROR, " ERASE_SEQ_ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ERASE_PARAM) {
    DEBUG ((DEBUG_ERROR, " ERASE_PARAM"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & WP_VIOLATION) {
    DEBUG ((DEBUG_ERROR, " WP_VIOLATION"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & CARD_IS_LOCKED) {
    DEBUG ((DEBUG_ERROR, " CARD_IS_LOCKED"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & LOCK_UNLOCK_FAILED) {
    DEBUG ((DEBUG_ERROR, " LOCK_UNLOCK_FAILED"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & COM_CRC_ERROR) {
    DEBUG ((DEBUG_ERROR, " COM_CRC_ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ILLEGAL_COMMAND) {
    DEBUG ((DEBUG_ERROR, " ILLEGAL_COMMAND"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & CARD_ECC_FAILED) {
    DEBUG ((DEBUG_ERROR, " CARD_ECC_FAILED"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & CC_ERROR) {
    DEBUG ((DEBUG_ERROR, " CC_ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ERROR) {
    DEBUG ((DEBUG_ERROR, " ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & CID_CSD_OVERWRITE) {
    DEBUG ((DEBUG_ERROR, " CID_CSD_OVERWRITE"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & WP_ERASE_SKIP) {
    DEBUG ((DEBUG_ERROR, " WP_ERASE_SKIP"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & ERASE_RESET) {
    DEBUG ((DEBUG_ERROR, " ERASE_RESET"));
    Status = EFI_DEVICE_ERROR;
  }

  if (CardStatus & SWITCH_ERROR) {
    DEBUG ((DEBUG_ERROR, " SWITCH_ERROR"));
    Status = EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_ERROR, " \n"));

  return Status;
}
/**
  The main function used to send the command to the card inserted into the SD/MMC host
  slot. It will assemble the arguments to set the command register and wait for the command
  and transfer completed until timeout. Then it will read the response register to fill
  the ResponseData.

  @param[in]  EmmcBaseAddress      Base address of MMIO register
  @param[in]  CommandIndex         The command index to set the command index field of command register
  @param[in]  Argument             Command argument to set the argument field of command register
  @param[in]  DataType             TRANSFER_TYPE, indicates no data, data in or data out
  @param[in]  Buffer               Contains the data read from / write to the device
  @param[in]  BufferSize           The size of the buffer
  @param[in]  ResponseType         RESPONSE_TYPE
  @param[in]  TimeOut              Time out value in 1 ms unit
  @param[out] ResponseData         Depending on the ResponseType, such as CSD or card status

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   A parameter was incorrect.
  @retval EFI_OUT_OF_RESOURCES    A resource has run out.
  @retval EFI_TIMEOUT             The timeout time expired.
  @retval EFI_DEVICE_ERROR        The physical device reported an error while attempting the operation
**/
EFI_STATUS
SendCommand (
  IN      UINTN                      EmmcBaseAddress,
  IN      UINT16                     CommandIndex,
  IN      UINT32                     Argument,
  IN      TRANSFER_TYPE              DataType,
  IN      UINT8                      *Buffer, OPTIONAL
  IN      UINT32                     BufferSize,
  IN      RESPONSE_TYPE              ResponseType,
  IN      UINT32                     TimeOut,
  OUT     UINT32                     *ResponseData
  )
{
  EFI_STATUS            Status;
  UINT32                ResponseDataCount;
  UINT16                Data16;
  UINT32                Data32;
  UINT64                Data64;
  UINT32                BlockLength;
  UINT32                Index;
  BOOLEAN               CommandCompleted;
  BOOLEAN               BufferReadReady;
  INT32                 Timeout;

  Status             = EFI_SUCCESS;
  BlockLength        = BLOCK_LENGTH;

  if (Buffer != NULL && DataType == NoData) {
    Status = EFI_INVALID_PARAMETER;
    return Status;
  }

  //
  // Check CMD INHIBIT and DATA INHIBIT before send command
  //
  Timeout = 1000;
  do {
    Data32 = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_PSTATE);
    MicroSecondDelay (100);
  } while ((Timeout-- > 0) && (Data32 & BIT0));

  Timeout = 1000;
  do {
    Data32 = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_PSTATE);
    MicroSecondDelay (100);
  } while ((Timeout-- > 0) && (Data32 & BIT1));


  //
  // Enable Interrupts
  //
  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_NINTEN, B_SCS_MEM_NINTEN_MASK);


  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_ERINTEN, B_SCS_MEM_ERINTEN_MASK);


  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_NINTSIGNEN, B_SCS_MEM_NINTSIGNEN_MASK);


  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_ERINTSIGNEN, B_SCS_MEM_ERINTSIGNEN_MASK);

  //
  // Clear status bits
  //
  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS, B_SCS_MEM_NINTSTS_CLEAR_MASK);

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_ERINTSTS, B_SCS_MEM_ERINTSTS_CLEAR_MASK);

  if (Buffer != NULL) {
    Data16 = 0;
    if (BufferSize <= BlockLength) {
      Data16 |= BufferSize;
    } else {
      Data16 |= BlockLength;
    }
  } else {
    Data16 = 0;
  }

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_BLKSZ, Data16);

  if (Buffer != NULL) {
    if (BufferSize <= BlockLength) {
      Data16 = 1;
    } else {
      Data16 = (UINT16) (BufferSize / BlockLength);
    }
  } else {
    Data16 = 0;
  }

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_BLKCNT, Data16);

  //
  // Argument
  //
  MmioWrite32 (EmmcBaseAddress + R_SCS_MEM_CMDARG, Argument);

  //
  // Transfer Mode
  //
  Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_XFRMODE);

  //
  // Data Transfer Direction Select
  //
  Data16 = 0;
  if (DataType == InData) {
    Data16 |= B_SCS_MEM_XFRMODE_DATA_TRANS_DIR;
  }

  if (CommandIndex == SEND_EXT_CSD) {
    Data16 |= B_SCS_MEM_XFRMODE_BLKCNT_EN;
  }

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_XFRMODE, Data16);
  //
  //Command
  //
  //ResponseTypeSelect    IndexCheck    CRCCheck    ResponseType
  //  00                     0            0           NoResponse
  //  01                     0            1           R2
  //  10                     0            0           R3, R4
  //  10                     1            1           R1, R5, R6, R7
  //  11                     1            1           R1b, R5b
  //
  switch (ResponseType) {
    case ResponseNo:
      Data16 = (CommandIndex << 8);
      ResponseDataCount = 0;
      break;

    case ResponseR1:
    case ResponseR5:
    case ResponseR6:
    case ResponseR7:
      Data16 = (CommandIndex << 8) | V_SCS_MEM_SDCMD_RESP_TYPE_SEL_RESP48 |
        B_SCS_MEM_SDCMD_CMD_INDEX_CHECK_EN | B_SCS_MEM_SDCMD_CMD_CRC_CHECK_EN;
      ResponseDataCount = 1;
      break;

    case ResponseR1b:
    case ResponseR5b:
      Data16 = (CommandIndex << 8) | V_SCS_MEM_SDCMD_RESP_TYPE_SEL_RESP48_CHK |
        B_SCS_MEM_SDCMD_CMD_INDEX_CHECK_EN | B_SCS_MEM_SDCMD_CMD_CRC_CHECK_EN;
      ResponseDataCount = 1;
      break;

    case ResponseR2:
      Data16 = (CommandIndex << 8) | V_SCS_MEM_SDCMD_RESP_TYPE_SEL_RESP136 |
        B_SCS_MEM_SDCMD_CMD_CRC_CHECK_EN;
      ResponseDataCount = 4;
      break;

    case ResponseR3:
    case ResponseR4:
      Data16 = (CommandIndex << 8) | V_SCS_MEM_SDCMD_RESP_TYPE_SEL_RESP48;
      ResponseDataCount = 1;
      break;

    default:
      ASSERT (0);
      Status = EFI_INVALID_PARAMETER;
      return Status;
  }

  if (DataType != NoData) {
    Data16 |= B_SCS_MEM_SDCMD_DATA_PRESENT_SEL;
  }

  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_SDCMD, Data16);
  CommandCompleted = FALSE;
  BufferReadReady = FALSE;
  TimeOut = 1000;
  do {
    Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_ERINTSTS);
    if ((Data16 & B_SCS_MEM_ERINTSTS_MASK) != 0) {
      Status = EmmcGetErrorReason (CommandIndex, Data16);
      return Status;
    }

    Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS) & 0x1ff;
    if (Data16 & B_SCS_MEM_NINTSTS_CMD_COMPLETE) {
      //
      // Command completed
      //
      CommandCompleted = TRUE;

      MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS, B_SCS_MEM_NINTSTS_CMD_COMPLETE);
      Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS);
      if ((DataType == NoData) & (ResponseType != ResponseR1b)) {
        break;
      }
    }

    if ((CommandCompleted) && (ResponseType == ResponseR1b)) {
      Data32 = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_PSTATE);
      if (Data32 & B_SCS_MEM_PSTATE_DAT0) {
        break;
      }
    }

    if ((CommandCompleted) && (Buffer!= NULL)) {
      if (!(Data16 & B_SCS_MEM_NINTSTS_CMD_COMPLETE)) {

        if (Data16 & B_SCS_MEM_NINTSTS_BUF_READ_READY_INTR) {
          BufferReadReady = TRUE;
          MmioOr16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS, B_SCS_MEM_NINTSTS_BUF_READ_READY_INTR);
          Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS);
        }

        if (BufferReadReady) {
          if (!(Data16 & B_SCS_MEM_NINTSTS_BUF_READ_READY_INTR)) {
            for (Index = 0; Index < BufferSize; Index =  Index + 4) {
              Data32 = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_BUFDATAPORT);
              if ((Index + 4) < BufferSize) {
                CopyMem ((Buffer + Index), &Data32, 4);
              } else {
                CopyMem ((Buffer + Index), &Data32, (BufferSize - Index));
              }
            }
          }
          Data16 = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS);
        }

        if (Data16 & B_SCS_MEM_NINTSTS_TRANSFER_COMPLETE) {
          MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_NINTSTS, B_SCS_MEM_NINTSTS_TRANSFER_COMPLETE);
          break;
        }
      }
    }
    MicroSecondDelay (1*1000);

    TimeOut --;

  } while (TimeOut > 0);

  if (TimeOut == 0) {
    Status = EFI_TIMEOUT;
    return Status;
  }

  if (ResponseData != NULL) {
    UINT32 *ResDataPtr = NULL;

    ResDataPtr = ResponseData;
    for (Index = 0; Index < ResponseDataCount; Index++) {
      *ResDataPtr = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_RESP + Index * 4);
      ResDataPtr++;
    }

    if (ResponseType == ResponseR2) {
      //
      // Adjustment for R2 response
      //
      Data32 = 1;
      for (Index = 0; Index < ResponseDataCount; Index++) {
        Data64 = LShiftU64 (*ResponseData, 8);
        *ResponseData = (UINT32) ((Data64 & 0xFFFFFFFF) | Data32);
        Data32 =  (UINT32) RShiftU64 (Data64, 32);
        ResponseData++;
      }
    }
  }
  return Status;
}

/**
  This main function is to send command to Emmc

  @param[in]  EmmcBaseAddress      Base address of MMIO register
  @param[in]  CommandIndex         The command index to set the command index field of command register
  @param[in]  Argument             Command argument to set the argument field of command register
  @param[in]  DataType             TRANSFER_TYPE, indicates no data, data in or data out
  @param[in]  Buffer               Contains the data read from / write to the device
  @param[in]  BufferSize           The size of the buffer
  @param[in]  ResponseType         RESPONSE_TYPE
  @param[in]  TimeOut              Time out value in 1 ms unit
  @param[out] ResponseData         Depending on the ResponseType, such as CSD or card status

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   A parameter was incorrect.
  @retval EFI_OUT_OF_RESOURCES    A resource has run out.
  @retval EFI_TIMEOUT             The timeout time expired.
  @retval EFI_DEVICE_ERROR        The physical device reported an error while attempting the operation
**/
EFI_STATUS
EmmcSendCommand (
  IN      UINTN                      EmmcBaseAddress,
  IN      UINT16                     CommandIndex,
  IN      UINT32                     Argument,
  IN      TRANSFER_TYPE              DataType,
  IN      UINT8                      *Buffer, OPTIONAL
  IN      UINT32                     BufferSize,
  IN      RESPONSE_TYPE              ResponseType,
  IN      UINT32                     TimeOut,
  OUT     UINT32                     *ResponseData
  )
{
  EFI_STATUS    Status;

  Status = SendCommand (
             EmmcBaseAddress,
             CommandIndex,
             Argument,
             DataType,
             Buffer,
             BufferSize,
             ResponseType,
             TimeOut,
             ResponseData
             );

  if (!EFI_ERROR (Status)) {
    if (ResponseType == ResponseR1 || ResponseType == ResponseR1b) {
      ASSERT (ResponseData != NULL);
      Status = EmmcCheckCardStatus (*ResponseData);
      if (EFI_ERROR (Status)) {
        EmmcReset (EmmcBaseAddress);
      }
    }
  } else {
    //
    // Reset Host Controller CMD and DATA
    //
    EmmcReset (EmmcBaseAddress);
  }

  return Status;
}
/**
  Set Tx Data Delay Control 1

  @param[in] EmmcBaseAddress      Base address of MMIO register
  @param[in] Value                Value (0 - 79)

  @retval VOID
**/
VOID
EmmcSetTxDllCtrl1 (
  IN UINTN                      EmmcBaseAddress,
  IN UINT8                      Value
  )
{
  MmioAndThenOr8 (EmmcBaseAddress + (R_SCS_MEM_TX_DATA_DLL_CNTL1 + 1), 0, Value);
}

/**
  Set Rx Strobe Delay Control DLL1 for HS400

  @param[in] EmmcBaseAddress      Base address of MMIO register
  @param[in] RxDll                To program RxDll1 or RxDll2 register
  @param[in] Value                Value (0 - 39)

  @retval VOID
**/
VOID
EmmcSetRxDllCtrl (
  IN UINTN                      EmmcBaseAddress,
  IN UINT8                      RxDll,
  IN UINT8                      Value
  )
{
  if (RxDll == RxDll1) {
    MmioAndThenOr8 (EmmcBaseAddress + (R_SCS_MEM_RX_STROBE_DLL_CNTL + 1), 0, Value);
  } else {
    MmioAndThenOr8 (EmmcBaseAddress + R_SCS_MEM_RX_STROBE_DLL_CNTL, 0, Value);
  }
}

/**
  Disable eMMC Host HS400 Support

  @param[in] EmmcBaseAddress      Base address of MMIO register

  @retval N/A
**/
VOID
EmmcHostHs400Disabled (
  IN UINTN                      EmmcBaseAddress
  )
{
  MmioAnd32 (EmmcBaseAddress + R_SCS_MEM_CAP_BYPASS_REG1, (UINT32) ~B_SCS_MEM_CAP_BYPASS_REG1_HS400);
}
/**
  Set Host Mode

  @param[in] EmmcBaseAddress      Base address of MMIO register
  @param[in] Mode                 Set Host Mode, 0: HS200, 1: HS400

  @retval N/A
**/
VOID
EmmcSetHostMode (
  IN UINTN                      EmmcBaseAddress,
  IN UINT32                     Mode
  )
{
  UINT16        ModeSet;
  UINT32        HostCapabilities;

  ModeSet = MmioRead16 (EmmcBaseAddress + R_SCS_MEM_HOST_CTL2);
  HostCapabilities = MmioRead32 (EmmcBaseAddress + R_SCS_MEM_CAP2);

  ModeSet &= ~B_SCS_MEM_HOST_CTL2_MODE_MASK;
  if ((Mode == Hs200) && (HostCapabilities & B_SCS_MEM_CAP2_SDR104_SUPPORT)) {
    ModeSet |= V_SCS_MEM_HOST_CTL2_MODE_SDR104;
  } else if ((Mode == Hs400) && (HostCapabilities & B_SCS_MEM_CAP2_HS400_SUPPORT)) {
    ModeSet |= V_SCS_MEM_HOST_CTL2_MODE_HS400;
  } else if (Mode == DDR50) {
    ModeSet |= V_SCS_MEM_HOST_CTL2_MODE_DDR50;
  } else if (Mode == SDR25) {
    ModeSet |= V_SCS_MEM_HOST_CTL2_MODE_SDR25;
  }


  MmioWrite16 (EmmcBaseAddress + R_SCS_MEM_HOST_CTL2, ModeSet);
}

/**
  To select eMMC card operating mode HS200/HS400

  @param[in] EmmcInfo                    A pointer to EMMC_INFO structure
  @param[in] EmmcBaseAddress             Base address of MMIO register
  @param[in] EmmcMode                    To select HS200 or HS400 mode

  @retval EFI_SUCCESS                    Emmc Mode Select successful.
  @retval EFI_INVALID_PARAMETER          A parameter was incorrect.
**/
EFI_STATUS
EmmcModeSelection (
  IN EMMC_INFO                          *EmmcInfo,
  IN UINTN                              EmmcBaseAddress,
  IN EMMC_MODE                          EmmcMode
  )
{
  SWITCH_ARGUMENT             SwitchArgument;
  UINT32                      CardStatus;
  UINT32                      HsTimingValue;
  UINT32                      BusWidthValue;
  EFI_STATUS                  Status;

  Status = EFI_SUCCESS;


  if (EmmcMode == Hs200) {
    HsTimingValue = 2;
    BusWidthValue = EmmcInfo->HS200BusWidth;
  } else {
    switch (mPchConfigHob->Scs.ScsEmmcHs400DriverStrength) {
      case DriverStrength33Ohm:
        HsTimingValue = 0x13;
        break;
      case DriverStrength40Ohm:
        HsTimingValue = 0x43;
        break;
      case DriverStrength50Ohm:
        HsTimingValue = 0x03;
        break;
      default:
        HsTimingValue = 0x13;
    }

    BusWidthValue = 6;
  }
  SetClockFrequency (EmmcBaseAddress, (50 * 1000 * 1000)); // Set 50MHz
  //
  // 1. Set HS_TIMING to 0x01 for High Speed interface timing. This is required prior DDR 8 bit bus width setting (CMD6)
  //
  ZeroMem (&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = 1;
  SwitchArgument.Index  = HS_TIMING_INDEX;
  SwitchArgument.Access = WRITE_BYTE_MODE;
  Status  = EmmcSendCommand (
              EmmcBaseAddress,
              SWITCH,
              *(UINT32*)&SwitchArgument,
              NoData,
              NULL,
              0,
              ResponseR1b,
              TIMEOUT_COMMAND,
              &CardStatus
              );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EmmcSetHostMode (EmmcBaseAddress, SDR25);

  //
  // 2. Set desired bus width to DDR 8 bit CMD6
  //
  ZeroMem (&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = BusWidthValue;
  SwitchArgument.Index  = BUS_WIDTH_INDEX;
  SwitchArgument.Access = WRITE_BYTE_MODE;
  Status  = EmmcSendCommand (
              EmmcBaseAddress,
              SWITCH,
              *(UINT32*)&SwitchArgument,
              NoData,
              NULL,
              0,
              ResponseR1b,
              TIMEOUT_COMMAND,
              &CardStatus
              );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  EmmcSetHostMode (EmmcBaseAddress, DDR50);

  //
  // 3. Set HS400 bit and Driver Strentgh value in HS_TIMING (CMD6)
  //
  ZeroMem (&SwitchArgument, sizeof (SWITCH_ARGUMENT));
  SwitchArgument.CmdSet = 0;
  SwitchArgument.Value  = HsTimingValue;
  SwitchArgument.Index  = HS_TIMING_INDEX;
  SwitchArgument.Access = WRITE_BYTE_MODE;
  Status  = EmmcSendCommand (
              EmmcBaseAddress,
              SWITCH,
              *(UINT32*)&SwitchArgument,
              NoData,
              NULL,
              0,
              ResponseR1b,
              TIMEOUT_COMMAND,
              &CardStatus
              );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (EmmcMode == Hs200) {
    EmmcSetHostMode (EmmcBaseAddress, Hs200);
  } else {
    EmmcSetHostMode (EmmcBaseAddress, Hs400);
  }
  SetClockFrequency (EmmcBaseAddress, (200 * 1000 * 1000)); // Set 200MHz


  return EFI_SUCCESS;
}

/**
  To perform HS400 Rx Data Path Training

  @param[in]     EmmcInfo                    A pointer to EMMC_INFO structure
  @param[in]     BlockIo                     A pointer to EFI_BLOCK_IO_PROTOCOL structure
  @param[in]     EmmcBaseAddress             Base address of MMIO register
  @param[in/out] EmmcTuningData              A pointer to EMMC_TUNING_DATA structure

  @retval EFI_SUCCESS                    HS400 Rx Data Path Training is successful.
  @retval EFI_OUT_OF_RESOURCES           The request could not be completed due to a lack of resources.
  @retval EFI_INVALID_PARAMETER          A parameter was incorrect.
  @retval EFI_DEVICE_ERROR               Hardware Error
  @retval EFI_NO_MEDIA                   No media
  @retval EFI_MEDIA_CHANGED              Media Change
  @retval EFI_BAD_BUFFER_SIZE            Buffer size is bad
  @retval EFI_CRC_ERROR                  Command or Data CRC Error
**/
EFI_STATUS
EmmcRxHs400Tuning (
  IN EMMC_INFO                    *EmmcInfo,
  IN EFI_BLOCK_IO_PROTOCOL        *BlockIo,
  IN UINTN                        EmmcBaseAddress,
  IN OUT EMMC_TUNING_DATA         *EmmcTuningData
  )
{
  UINT8                     *Buffer;
  UINT8                     DllCount;
  UINT8                     DllMax;
  UINT8                     DllMin;
  UINT8                     Smin;
  UINT8                     Smax;
  UINT8                     Sopt;
  EFI_STATUS                Status;
  BLOCK_READ_WRITE_STATUS   FirstRead;

  Status = EFI_SUCCESS;

  Smin = RX_STROBE_DLL1_TAP_MIN_MEPT;
  Smax = RX_STROBE_DLL1_TAP_MAX_MEPT;
  Buffer = (VOID *) AllocateZeroPool (BlockIo->Media->BlockSize);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  //
  // 1. Read Tuning Block that used at HS200 Tuning
  //
  Status = BlockIo->ReadBlocks (
                      BlockIo,
                      BlockIo->Media->MediaId,
                      EmmcInfo->Lba,
                      BlockIo->Media->BlockSize,
                      Buffer
                      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // 2. Move to HS400 Mode
  //
  Status = EmmcModeSelection (EmmcInfo, EmmcBaseAddress, Hs400);
  if (EFI_ERROR (Status)) {
    goto Exit;
  }
  //
  // 3. Set Rx Strobe DLL1 to the Minimal Expected Passing Tap (Smin)
  //    Offset 830h: Rx Strobe Delay DLL 1(HS400 Mode), bits [14:8]
  //    Set Rx Data Strobe DLL2 to middle point 0x13
  //    Offset 830h: Rx Strobe Delay DLL 2(HS400 Mode), bits [6:0]
  DllCount = RX_STROBE_DLL1_TAP_MIN_MEPT;
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll1, DllCount);
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll2, DllCount);

  //
  // 4. Read the block that was stored
  // 5. If CRC fails on first read, increase the DLL Step and repeat block read until passed
  //    Else if CRC passed on first read, decrease the DLL Step and repeat block read until failed
  // 6. Store the Rx Path min DLL passing step number
  //
  FirstRead = NotAvailable;
  DllMax    = RX_STROBE_DLL1_TAP_MAX_RANGE;
  DllMin    = RX_STROBE_DLL1_TAP_MIN_RANGE;
  while ((DllCount <= DllMax) && (DllCount >= DllMin)) {
    Status = BlockIo->ReadBlocks (
                        BlockIo,
                        BlockIo->Media->MediaId,
                        EmmcInfo->Lba,
                        BlockIo->Media->BlockSize,
                        Buffer
                        );

    if (Status == EFI_SUCCESS) {
      if (FirstRead == NotAvailable) {
        FirstRead = Passed;
      } else if (FirstRead == Failed) {
        Smin = DllCount;
        break;
      }
      if (DllCount == RX_STROBE_DLL1_TAP_MIN_RANGE) {
        Smin = RX_STROBE_DLL1_TAP_MIN_RANGE;
        break;
      }
    } else if (Status == EFI_CRC_ERROR) { // Rely on the driver to return ReadBlocks status on CRC error
      if (FirstRead == NotAvailable) {
        FirstRead = Failed;
      } else if (FirstRead == Passed) {
        Smin = DllCount + 1;
        break;
      }
      if (DllCount == RX_STROBE_DLL1_TAP_MAX_RANGE) {
        Status = EFI_CRC_ERROR;
        goto Exit;
      }
    } else {
      goto Exit;
    }

    if (FirstRead == Failed) {
      DllCount++;
    } else {
      DllCount--;
    }
    EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll1, DllCount);
    EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll2, DllCount);
  }


  //
  // 7. Set the Rx Strobe DLL1 to the Maximal Expected Passing Tap (Smax)
  // Offset 830h: Rx Strobe Delay DLL 1(HS400 Mode), bits [14:8]
  // 8. Read the block that was stored
  // 9. If CRC fails on first read, decrease the DLL Step and repeat step 8 until pass
  //    Else if CRC passed on first read, increase the DLL Step and repeat step 8 until failed
  //
  DllCount = RX_STROBE_DLL1_TAP_MAX_MEPT;
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll1, DllCount);
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll2, DllCount);

  FirstRead = NotAvailable;
  DllMax    = RX_STROBE_DLL1_TAP_MAX_RANGE;
  DllMin    = RX_STROBE_DLL1_TAP_MIN_RANGE;
  while ((DllCount <= DllMax) && (DllCount >= DllMin)) {
    Status = BlockIo->ReadBlocks (
                        BlockIo,
                        BlockIo->Media->MediaId,
                        EmmcInfo->Lba,
                        BlockIo->Media->BlockSize,
                        Buffer
                        );
    if (Status == EFI_SUCCESS) {
      if (FirstRead == NotAvailable) {
        FirstRead = Passed;
      } else if (FirstRead == Failed) {
        Smax = DllCount;
        break;
      }
      if (DllCount == RX_STROBE_DLL1_TAP_MAX_RANGE) {
        Smax = DllCount;
        break;
      }
    } else if (Status == EFI_CRC_ERROR) { // Rely on the driver to return ReadBlocks status on CRC error
      if (FirstRead == NotAvailable) {
        FirstRead = Failed;
      } else if (FirstRead == Passed) {
        Smax = DllCount - 1;
        break;
      }
      if (DllCount == RX_STROBE_DLL1_TAP_MIN_RANGE) {
        Status = EFI_CRC_ERROR;
        goto Exit;
      }
    } else {
      goto Exit;
    }
    if (FirstRead == Failed) {
      DllCount--;
    } else {
      DllCount++;
    }
    EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll1, DllCount);
    EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll2, DllCount);
  }
  //
  // 10. Store the Rx Path max DLL Passing Step number
  //
  //
  // 11. Compute the Rx DLL Optimal Point (Sopt) = (Smax - Smin)/2 + Smin
  //
  Sopt = (Smax - Smin) / 2 + Smin;
  //
  // 12. Store the Rx DLL optimal value (Sopt)
  //
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll1, Sopt);
  EmmcSetRxDllCtrl (EmmcBaseAddress, RxDll2, Sopt);

  Status = EFI_SUCCESS;
  EmmcTuningData->Hs400RxStrobe1Dll  = Sopt;

Exit:
  FreePool (Buffer);
  return Status;
}

/**
  To perform HS400 Tx Data Path Training

  @param[in] EmmcInfo                       A pointer to EMMC_INFO structure
  @param[in] EmmcBaseAddress                Base address of MMIO register
  @param[in] BlockIo                        A pointer to EFI_BLOCK_IO_PROTOCOL structure
  @param[in/out] EmmcTuningData             A pointer to EMMC_TUNING_DATA structure

  @retval EFI_SUCCESS                    HS400 Rx Data Path Training is successful.
  @retval EFI_OUT_OF_RESOURCES           The request could not be completed due to a lack of resources.
  @retval EFI_INVALID_PARAMETER          A parameter was incorrect.
  @retval EFI_DEVICE_ERROR               Hardware Error
  @retval EFI_NO_MEDIA                   No media
  @retval EFI_MEDIA_CHANGED              Media Change
  @retval EFI_BAD_BUFFER_SIZE            Buffer size is bad
  @retval EFI_CRC_ERROR                  Command or Data CRC Error
**/
EFI_STATUS
EmmcTxHs400Tuning (
  IN  EMMC_INFO                   *EmmcInfo,
  IN  EFI_BLOCK_IO_PROTOCOL       *BlockIo,
  IN  UINTN                       EmmcBaseAddress,
  IN OUT EMMC_TUNING_DATA         *EmmcTuningData
  )
{
  UINT8                    *Buffer;
  UINT8                     DllCount;
  UINT8                     DllMax;
  UINT8                     DllMin;
  UINT8                     Smin;
  UINT8                     Smax;
  UINT8                     Sopt;
  UINT8                     N;
  EFI_STATUS                Status;
  BLOCK_READ_WRITE_STATUS   FirstWrite;

  Status = EFI_SUCCESS;

  Smin = TX_DATA_DLL_TAP_MIN_MEPT;
  Smax = TX_DATA_DLL_TAP_MAX_MEPT;
  N = 0;

  Buffer = (VOID *) AllocateZeroPool (BlockIo->Media->BlockSize);
  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  //
  // 1. Read Tuning Block that used at Rx HS400 Tuning
  //
  Status = BlockIo->ReadBlocks (
                      BlockIo,
                      BlockIo->Media->MediaId,
                      EmmcInfo->Lba,
                      BlockIo->Media->BlockSize,
                      Buffer
                      );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }
  //
  // 2. Set Tx Data DLL1 to the Minimal Expected Passing Tap (Smin)
  // Offset 824h: Tx Data Delay Control 1
  // Tx Data Delay (HS400 Mode), BIT[14:8]
  //
  DllCount = TX_DATA_DLL_TAP_MIN_MEPT;
  EmmcSetTxDllCtrl1 (EmmcBaseAddress, DllCount);

  //
  // 2. Write Single Block
  //
  FirstWrite = NotAvailable;
  DllMax     = TX_DATA_DLL_TAP_MAX_RANGE;
  DllMin     = TX_DATA_DLL_TAP_MIN_RANGE;
  while ((DllCount <= DllMax) && (DllCount >= DllMin)) {
    Status = BlockIo->WriteBlocks (
                        BlockIo,
                        BlockIo->Media->MediaId,
                        EmmcInfo->Lba,
                        BlockIo->Media->BlockSize,
                        Buffer
                        );

    if (Status == EFI_SUCCESS) {
      if (FirstWrite == NotAvailable) {
        FirstWrite = Passed;
      } else if (FirstWrite == Failed) {
        Smin = DllCount;
        break;
      }
      if (DllCount == TX_DATA_DLL_TAP_MIN_RANGE) {
        Smin = TX_DATA_DLL_TAP_MIN_RANGE;
        break;
      }
      //
      // 3. If CRC fails increment DLL Step and repeat step 2
      //
    } else if (Status == EFI_CRC_ERROR) { // Rely on the driver to return ReadBlocks status on CRC error
      if (FirstWrite == NotAvailable) {
        FirstWrite = Failed;
      } else if (FirstWrite == Passed) {
        Smin = DllCount + 1;
        break;
      }
      if (DllCount == TX_DATA_DLL_TAP_MAX_RANGE) {
        goto Exit;
      }
    } else {
      goto Exit;
    }
    if (FirstWrite == Failed) {
      DllCount++;
    } else {
      DllCount--;
    }
    EmmcSetTxDllCtrl1 (EmmcBaseAddress, DllCount);
  }
  //
  // 4. Store the Tx Path min DLL passing step number
  //
  //
  // 5. Set the DLL to max expected passing step (Smax)
  //
  DllCount = TX_DATA_DLL_TAP_MAX_MEPT;
  EmmcSetTxDllCtrl1 (EmmcBaseAddress, DllCount);
  //
  // 6. Write Single Block
  //
  FirstWrite = NotAvailable;
  DllMax     = TX_DATA_DLL_TAP_MAX_RANGE;
  DllMin     = TX_DATA_DLL_TAP_MIN_RANGE;
  while ((DllCount <= DllMax) && (DllCount >= DllMin)) {
    Status = BlockIo->WriteBlocks (
                        BlockIo,
                        BlockIo->Media->MediaId,
                        EmmcInfo->Lba,
                        BlockIo->Media->BlockSize,
                        Buffer
                        );

    if (Status == EFI_SUCCESS) {
      if (FirstWrite == NotAvailable) {
        FirstWrite = Passed;
      } else if (FirstWrite == Failed) {
        Smax = DllCount;
        break;
      }
      if (DllCount == TX_DATA_DLL_TAP_MAX_RANGE) {
        Smax = TX_DATA_DLL_TAP_MAX_RANGE;
        break;
      }
      //
      // 7. If CRC fails decrement DLL Step and repeat step 6
      //
    } else if (Status == EFI_CRC_ERROR) { // Rely on the driver to return ReadBlocks status on CRC error
      if (FirstWrite == NotAvailable) {
        FirstWrite = Failed;
      } else if (FirstWrite == Passed) {
        Smax = DllCount - 1;
        break;
      }
      if (DllCount == TX_DATA_DLL_TAP_MIN_RANGE) {
        goto Exit;
      }
    } else {
      goto Exit;
    }
    if (FirstWrite == Failed) {
      DllCount--;
    } else {
      DllCount++;
    }
    EmmcSetTxDllCtrl1 (EmmcBaseAddress, DllCount);
  }
  //
  // 8. Store the DLL passing step number (Smax)
  //
  DEBUG ((DEBUG_INFO, "Smax = 0x%x\n", Smax));
  N = Smax - Smin;
  if (N <= 0) {
    Status = EFI_DEVICE_ERROR;
    goto Exit;
  }
  //
  // 9. Compute the Tx DLL Optimal point (Sopt) = (Smax - Smin) / 2 + Smin
  //
  Sopt = (Smax - Smin) / 2 + Smin;
  //
  // 10. Store the Tx Strobe DLL Optimal point value
  //
  EmmcSetTxDllCtrl1 (EmmcBaseAddress, Sopt);

  Status = EFI_SUCCESS;
  EmmcTuningData->Hs400TxDataDll  = Sopt;

Exit:
  FreePool (Buffer);
  return Status;
}

/**
  To perform write protection checking on the address to write

  @param[in] EmmcInfo                    A pointer to EMMC_INFO structure
  @param[in] BlockIo                     A pointer to EFI_BLOCK_IO_PROTOCOL structure
  @param[in] EmmcBaseAddress             Base address of MMIO register

  @retval EFI_SUCCESS                    HS400 Rx Data Path Training is successful.
  @retval EFI_OUT_OF_RESOURCES           The request could not be completed due to a lack of resources.
  @retval EFI_INVALID_PARAMETER          A parameter was incorrect.
  @retval EFI_DEVICE_ERROR               Hardware Error
  @retval EFI_NO_MEDIA                   No media
  @retval EFI_MEDIA_CHANGED              Media Change
  @retval EFI_BAD_BUFFER_SIZE            Buffer size is bad
  @retval EFI_CRC_ERROR                  Command or Data CRC Error
**/
EFI_STATUS
EmmcWriteProtectCheck (
  IN EMMC_INFO                      *EmmcInfo,
  IN EFI_BLOCK_IO_PROTOCOL          *BlockIo,
  IN UINTN                          EmmcBaseAddress
  )
{
  EFI_STATUS      Status;
  UINT32          DataSize;
  UINT32          CardStatus;
  UINT32          Address;
  UINT32          Timeout;
  UINT16          CommandIndex;
  UINT8           *Buffer;

  Status          = EFI_SUCCESS;
  DataSize        = 8;
  CommandIndex    = SEND_WRITE_PROT_TYPE;
  Timeout         = TIMEOUT_DATA;

  Buffer = (VOID *) AllocateZeroPool (DataSize);
  Address = (UINT32) DivU64x32 (MultU64x32 (EmmcInfo->Lba, BlockIo->Media->BlockSize), 512);


  if (BlockIo->Media->ReadOnly == TRUE) {
    Status = EFI_WRITE_PROTECTED;
    goto Exit;
  }

  if (Buffer == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  Status = EmmcSendCommand (
             EmmcBaseAddress,
             CommandIndex,
             Address,
             InData,
             Buffer,
             DataSize,
             ResponseR1,
             Timeout,
             &CardStatus
             );

  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  if ((Buffer[7] & (BIT0 | BIT1)) !=0) {
    Status = EFI_WRITE_PROTECTED;
    goto Exit;
  }


Exit:
  FreePool (Buffer);
  return Status;
}

/**
  Configure eMMC in HS400 Mode

  @param[in] This                         A pointer to PCH_EMMC_TUNING_PROTOCOL structure
  @param[in] Revision                     Revision parameter used to verify the layout of EMMC_INFO and TUNINGDATA.
  @param[in] EmmcInfo                     A pointer to EMMC_INFO structure
  @param[out] EmmcTuningData              A pointer to EMMC_TUNING_DATA structure

  @retval EFI_SUCCESS                     The function completed successfully
  @retval EFI_NOT_FOUND                   The item was not found
  @retval EFI_OUT_OF_RESOURCES            The request could not be completed due to a lack of resources.
  @retval EFI_INVALID_PARAMETER           A parameter was incorrect.
  @retval EFI_DEVICE_ERROR                Hardware Error
  @retval EFI_NO_MEDIA                    No media
  @retval EFI_MEDIA_CHANGED               Media Change
  @retval EFI_BAD_BUFFER_SIZE             Buffer size is bad
  @retval EFI_CRC_ERROR                   Command or Data CRC Error
**/
EFI_STATUS
EFIAPI
ConfigureEmmcHs400Mode (
  IN  PCH_EMMC_TUNING_PROTOCOL          *This,
  IN  UINT8                             Revision,
  IN  EMMC_INFO                         *EmmcInfo,
  OUT EMMC_TUNING_DATA                  *EmmcTuningData
  )
{
  EFI_BLOCK_IO_PROTOCOL         *BlockIo;
  EFI_STATUS                    Status;
  EFI_STATUS                    ModeStatus;
  UINTN                         EmmcBaseAddress;
  UINT64                        EmmcPciBaseAddress;
  ModeStatus =  EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "ConfigureEmmcHs400Mode() Start\n"));
  EmmcTuningData->Hs400DataValid = FALSE;
  //
  // Check PCH_EMMC_TUNING_PROTOCOL_REVISION
  //
  if (Revision != PCH_EMMC_TUNING_PROTOCOL_REVISION) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: PCH eMMC Tuning Protocol Revision Not Match! Tuning Aborted\n"));
    return EFI_UNSUPPORTED;
  }
  //
  // Get eMMC Host Controller Mmio base register
  //
  EmmcPciBaseAddress = ScsGetEmmcBaseAddress ();

  if (PciSegmentRead16 (EmmcPciBaseAddress + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: eMMC Host Controller Unavailable!\n"));
    return EFI_UNSUPPORTED;
  }

  //
  // Assume BAR is ready since it's executed after blockio ready, handling 64bit BAR in DXE is not required
  //
  EmmcBaseAddress = PciSegmentRead32 (EmmcPciBaseAddress + PCI_BASE_ADDRESSREG_OFFSET) & 0xFFFFF000;
  //
  // Enable Memory Decode
  //
  if ((PciSegmentRead8 (EmmcPciBaseAddress + PCI_COMMAND_OFFSET) & EFI_PCI_COMMAND_MEMORY_SPACE) == 0) {
    PciSegmentOr8 (EmmcPciBaseAddress + PCI_COMMAND_OFFSET, EFI_PCI_COMMAND_MEMORY_SPACE);
  }

  if (PchStepping () < PCH_B0) {
    DEBUG ((DEBUG_INFO, "[CNP Ax] Bypass HS400 mode tuning, set device to HS400 mode\n"));

    Status = EmmcModeSelection (EmmcInfo, EmmcBaseAddress, Hs400);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: eMMC HS400 Mode Selection Failed!\n"));
    }

    DEBUG ((DEBUG_INFO, "[CNP Ax] ConfigureEmmcHs400Mode() End\n"));
    return Status;
  }

  if (mPchConfigHob->Scs.ScsEmmcHs400TuningRequired == FALSE) {
    if (mPchConfigHob->Scs.ScsEmmcHs400DllDataValid == TRUE) {
      DEBUG ((DEBUG_INFO, "ConfigureEmmcHs400Mode: SCS eMMC 5.0 HS400 Tuning Not Required, set device to HS400 mode\n"));
      Status = EmmcModeSelection (EmmcInfo, EmmcBaseAddress, Hs400);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: eMMC HS400 Mode Selection Failed!\n"));
      }
      return Status;
    } else {
      DEBUG ((DEBUG_INFO, "ConfigureEmmcHs400Mode: SCS eMMC 5.0 HS400 Mode Selection Not Required.\n"));
      return EFI_ABORTED;
    }
  }
  //
  // Handle Platform Emmc Info Protocol for Efi Block Io Protocol
  //
  Status = gBS->HandleProtocol (
                  EmmcInfo->PartitionHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID**) &BlockIo
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: BlockIo: Platform Emmc Info Protocol Handle Not Found!\n"));
    EmmcHostHs400Disabled (EmmcBaseAddress);
    return Status;
  }

  //
  // Write Protection checking on the region
  //
  Status = EmmcWriteProtectCheck (EmmcInfo, BlockIo, EmmcBaseAddress);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: Region is write protected! HS400 Tuning Abort!\n"));
    EmmcHostHs400Disabled (EmmcBaseAddress);
    return Status;
  }
  //
  // Rx HS400 Auto Tuning
  //
  Status = EmmcRxHs400Tuning (EmmcInfo, BlockIo, EmmcBaseAddress, EmmcTuningData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: Rx HS400 Auto Tuning Failed!\n"));
    ModeStatus = EmmcModeSelection (EmmcInfo, EmmcBaseAddress, Hs200);
    if (EFI_ERROR (ModeStatus)) {
      DEBUG ((DEBUG_ERROR, "EmmcTxHs400Tuning: eMMC HS200 Mode Selection Failed!\n"));
    }
    EmmcTuningData->Hs400DataValid = FALSE;
    return Status;
  }

  //
  // Tx HS400 Auto Tuning
  //
  Status = EmmcTxHs400Tuning (EmmcInfo, BlockIo, EmmcBaseAddress, EmmcTuningData);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ConfigureEmmcHs400Mode: Tx HS400 Auto Tuning Failed!\n"));
    ModeStatus = EmmcModeSelection (EmmcInfo, EmmcBaseAddress, Hs200);
    if (EFI_ERROR (ModeStatus)) {
      DEBUG ((DEBUG_ERROR, "EmmcTxHs400Tuning: eMMC HS200 Mode Selection Failed!\n"));
    }
    EmmcTuningData->Hs400DataValid = FALSE;
    return Status;
  }
  //
  // Set HS400 Data Valid Tuning Bit to TRUE
  //
  EmmcTuningData->Hs400DataValid = TRUE;

  DEBUG ((DEBUG_INFO, "ConfigureEmmcHs400Mode() End\n"));

  return Status;
}

/**
  Install PCH EMMC TUNING PROTOCOL

**/
VOID
InstallPchEmmcTuningProtocol (
  VOID
  )
{

  EFI_HANDLE                      Handle;
  EFI_STATUS                      Status;

  Handle = NULL;

  ///
  /// For normal boot flow
  /// 1. If ScsEmmcEnabled and ScsEmmcHs400Enabled policy set,
  ///    a) Set ScsEmmcHs400TuningRequired policy to state tuning required in PEI,
  ///       - if RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion, a battery failure or CMOS clear.(Boot with default settings)
  ///       - if non-volatile variable 'Hs400TuningData' does not exist
  ///    b) RC installed Pch Emmc Tuning Protocol regardless of ScsEmmcHs400TuningRequired policy setting.in DXE
  ///    c) If boot with default settings after CMOS cleared, platform delete variable 'Hs400TuningData' in DXE
  /// 2. Once RC successfully installed Pch Emmc Tuning Protocol, it will be used to perform EmmcTune for Hs400.
  /// 3. Then, platform must set the variable with returned EmmcTuningData no matter tuning pass of fail
  /// 4. Platform shall set variable 'Hs400TuningData' for one time only or after CMOS clear
  ///
  /// For fast boot flow
  /// 1. If ScsEmmcEnabled and ScsEmmcHs400Enabled policy set,
  ///    a) Set ScsEmmcHs400TuningRequired policy to state tuning not required, if non-volatile variable 'Hs400TuningData' exist
  ///    b) RC installed Pch Emmc Tuning Protocol regardless of ScsEmmcHs400TuningRequired policy setting in DXE
  /// 2. Once RC successfully installed Pch Emmc Tuning Protocol, it will be used to perform EmmcTune
  /// 3. Since ScsEmmcHs400TuningRequired state tuning not required, RC will not perform Emmc Hs400 Tuning but just set the device to operate in HS400 mode if data is valid
  /// 4. Platform shall not set variable 'Hs400TuningData'
  ///
  //
  // Install PchEmmcTuningProtocol Protocol
  //
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  &gPchEmmcTuningProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &PchEmmcTuningProtocol
                  );
  ASSERT_EFI_ERROR (Status);
}
