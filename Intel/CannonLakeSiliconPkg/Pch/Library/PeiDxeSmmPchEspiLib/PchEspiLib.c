/** @file
  This file contains routines for eSPI

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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
#include <Library/BaseMemoryLib.h>
#include <Library/PchEspiLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Library/TimerLib.h>
#include <PchLimits.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsLpc.h>

#define CHANNEL_RESET_TIMEOUT     100   ///< Channel reset timeout in us after which to report error
#define SLAVE_CHANNELS_MAX        7     ///< Max number of channels

//
// eSPI Slave registers
//
#define R_ESPI_SLAVE_GENCAP               0x08      ///< General Capabilities and Configurations
#define B_ESPI_SLAVE_GENCAP_SUPPCHAN      0xFF      ///< Channels supported bit mask
#define R_ESPI_SLAVE_CHACAP_BASE          0x10      ///< Base address from which channel Cap and Conf registers start on slave
#define S_ESPI_SLAVE_CHACAP_OFFSET        0x10      ///< Offset for each channel from base
#define B_ESPI_SLAVE_CHACAP_CHEN          BIT0      ///< Slave Channel enable bit
#define B_ESPI_SLAVE_CHACAP_CHRDY         BIT1      ///< Slave Channel ready bit

/**
  Checks if second slave capability is enabled

  @retval TRUE      There's second slave
  @retval FALSE     There's no second slave
**/
BOOLEAN
IsEspiSecondSlaveSupported (
  VOID
  )
{
  return (IsPchH () && ((PchPcrRead32 (PID_ESPISPI, R_ESPI_PCR_SOFTSTRAPS) & R_ESPI_PCR_SOFTSTRAPS_CS1_EN) != 0));
}

/**
  Checks in slave General Capabilities register if it supports channel with requested number

  @param[in]  SlaveId         Id of slave to check
  @param[in]  ChannelNumber   Number of channel of which to check

  @retval TRUE      Channel with requested number is supported by slave device
  @retval FALSE     Channel with requested number is not supported by slave device
**/
BOOLEAN
IsEspiSlaveChannelSupported (
  UINT8   SlaveId,
  UINT8   ChannelNumber
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;
  UINT8       SupportedChannels;

  Status = PchEspiSlaveGetConfig (SlaveId, R_ESPI_SLAVE_GENCAP, &Data32);
  if (EFI_ERROR (Status)) {
    return FALSE;
  }
  SupportedChannels = (UINT8) (Data32 & B_ESPI_SLAVE_GENCAP_SUPPCHAN);

  DEBUG ((DEBUG_INFO, "Slave %d supported channels 0x%4X\n", SlaveId, SupportedChannels));

  if (ChannelNumber > SLAVE_CHANNELS_MAX || !(SupportedChannels & (BIT0 << ChannelNumber))) {
    // Incorrect channel number was specified. Either exceeded max or Slave doesn't support that channel.
    return FALSE;
  }

  return TRUE;
}

/**
  Is eSPI enabled in strap.

  @retval TRUE          Espi is enabled in strap
  @retval FALSE         Espi is disabled in strap
**/
BOOLEAN
IsEspiEnabled (
  VOID
  )
{
  return (PchPcrRead32 (PID_ESPISPI, R_ESPI_PCR_CFG_VAL) & B_ESPI_PCR_CFG_VAL_ESPI_EN) != 0;
}

/**
  eSPI helper function to clear slave configuration register status

  @retval EFI_SUCCESS Write to private config space succeed
  @retval others      Read / Write failed
**/
STATIC
VOID
EspiClearScrs (
  VOID
  )
{
  PchPcrAndThenOr32 (
    PID_ESPISPI,
    R_ESPI_PCR_SLV_CFG_REG_CTL,
    (UINT32) ~0,
     B_ESPI_PCR_SLV_CFG_REG_CTL_SCRS
     );
}

/**
  eSPI helper function to poll slave configuration register enable for 0
  and to check for slave configuration register status

  @retval EFI_SUCCESS       Enable bit is zero and no error in status bits
  @retval EFI_DEVICE_ERROR  Error in SCRS
  @retval others            Read / Write to private config space failed
**/
STATIC
EFI_STATUS
EspiPollScreAndCheckScrs (
  VOID
  )
{
  UINT32     ScrStat;

  do {
    ScrStat = PchPcrRead32 (PID_ESPISPI, R_ESPI_PCR_SLV_CFG_REG_CTL);
  } while ((ScrStat & B_ESPI_PCR_SLV_CFG_REG_CTL_SCRE) != 0);

  ScrStat = (ScrStat & B_ESPI_PCR_SLV_CFG_REG_CTL_SCRS) >> N_ESPI_PCR_SLV_CFG_REG_CTL_SCRS;
  if (ScrStat != V_ESPI_PCR_SLV_CFG_REG_CTL_SCRS_NOERR) {
    DEBUG ((DEBUG_ERROR, "eSPI slave config register status (error) is %x \n", ScrStat));
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

typedef enum {
  EspiSlaveOperationConfigRead,
  EspiSlaveOperationConfigWrite,
  EspiSlaveOperationStatusRead,
  EspiSlaveOperationInBandReset
} ESPI_SLAVE_OPERATION;

/**
  Helper library to do all the operations regards to eSPI slave

  @param[in]      SlaveId         eSPI Slave ID
  @param[in]      SlaveAddress    Slave address to be put in R_ESPI_PCR_SLV_CFG_REG_CTL[11:0]
  @param[in]      SlaveOperation  Based on ESPI_SLAVE_OPERATION
  @param[in,out]  Data

  @retval EFI_SUCCESS           Operation succeed
  @retval EFI_INVALID_PARAMETER Slave ID is not supported or SlaveId 1 is used in PCH_LP
  @retval EFI_INVALID_PARAMETER Slave configuration register address exceed maximum allowed
  @retval EFI_INVALID_PARAMETER Slave configuration register address is not DWord aligned
  @retval EFI_ACCESS_DENIED     eSPI Slave write to address range 0 to 0x7FF has been locked
  @retval EFI_DEVICE_ERROR      Error in SCRS during polling stage of operation
**/
STATIC
EFI_STATUS
EspiSlaveOperationHelper (
  IN     UINT32               SlaveId,
  IN     UINT32               SlaveAddress,
  IN     ESPI_SLAVE_OPERATION SlaveOperation,
  IN OUT UINT32               *Data
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;

  //
  // Check the SlaveId is 0 or 1
  //
  if (SlaveId >= PCH_MAX_ESPI_SLAVES) {
    DEBUG ((DEBUG_ERROR, "eSPI Slave ID of %d or more is not accepted \n", PCH_MAX_ESPI_SLAVES));
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check if SlaveId 1 is used, it is a PCH_H
  //
  if ((SlaveId == 1) && (IsPchLp ())) {
    DEBUG ((DEBUG_ERROR, "eSPI Slave ID of 1 is only available on PCH_H \n"));
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check the address is not more then 0xFFF
  //
  if (SlaveAddress > B_ESPI_PCR_SLV_CFG_REG_CTL_SCRA) {
    DEBUG ((DEBUG_ERROR, "eSPI Slave address must be less than 0x%x \n", (B_ESPI_PCR_SLV_CFG_REG_CTL_SCRA + 1)));
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check the address is DWord aligned
  //
  if ((SlaveAddress & 0x3) != 0) {
    DEBUG ((DEBUG_ERROR, "eSPI Slave address must be DWord aligned \n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check if write is allowed
  //
  if ((SlaveOperation == EspiSlaveOperationConfigWrite) &&
      (SlaveAddress <= 0x7FF)) {

    //
    // If the SLCRR is not set in corresponding slave, we will check the lock bit
    //
    Data32 = PchPcrRead32 (PID_ESPISPI, (UINT16) (R_ESPI_PCR_LNKERR_SLV0 + (SlaveId * S_ESPI_PCR_LNKERR_SLV0)));
    if ((Data32 & B_ESPI_PCR_LNKERR_SLV0_SLCRR) == 0) {

      Data32 = PchPcrRead32 (PID_ESPISPI, (UINT16) R_ESPI_PCR_SLV_CFG_REG_CTL);
      if ((Data32 & B_ESPI_PCR_SLV_CFG_REG_CTL_SBLCL) != 0) {
        DEBUG ((DEBUG_ERROR, "eSPI Slave write to address range 0 to 0x7FF has been locked \n"));
        return EFI_ACCESS_DENIED;
      }
    }
  }

  //
  // Input check done, now go through all the processes
  //
   EspiClearScrs ();

  if (SlaveOperation == EspiSlaveOperationConfigWrite) {
    PchPcrWrite32 (
      PID_ESPISPI,
      (UINT16) R_ESPI_PCR_SLV_CFG_REG_DATA,
      *Data
      );
  }

  PchPcrAndThenOr32 (
    PID_ESPISPI,
    (UINT16) R_ESPI_PCR_SLV_CFG_REG_CTL,
    (UINT32) ~(B_ESPI_PCR_SLV_CFG_REG_CTL_SID | B_ESPI_PCR_SLV_CFG_REG_CTL_SCRT | B_ESPI_PCR_SLV_CFG_REG_CTL_SCRA),
    (B_ESPI_PCR_SLV_CFG_REG_CTL_SCRE |
     (SlaveId << N_ESPI_PCR_SLV_CFG_REG_CTL_SID) |
     (((UINT32) SlaveOperation) << N_ESPI_PCR_SLV_CFG_REG_CTL_SCRT) |
     SlaveAddress
     )
    );

  Status = EspiPollScreAndCheckScrs ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((SlaveOperation == EspiSlaveOperationConfigRead) || (SlaveOperation == EspiSlaveOperationStatusRead)) {
    Data32 = PchPcrRead32 (
               PID_ESPISPI,
               (UINT16) R_ESPI_PCR_SLV_CFG_REG_DATA
               );
    if (SlaveOperation == EspiSlaveOperationStatusRead) {
      *Data = Data32 & 0xFFFF;
    } else {
      *Data = Data32;
    }
  }

  return EFI_SUCCESS;
}

/**
  Get configuration from eSPI slave

  @param[in]  SlaveId       eSPI slave ID
  @param[in]  SlaveAddress  Slave Configuration Register Address
  @param[out] OutData       Configuration data read

  @retval EFI_SUCCESS           Operation succeed
  @retval EFI_INVALID_PARAMETER Slave ID is not supported
  @retval EFI_INVALID_PARAMETER Slave ID is not supported or SlaveId 1 is used in PCH_LP
  @retval EFI_INVALID_PARAMETER Slave configuration register address exceed maximum allowed
  @retval EFI_INVALID_PARAMETER Slave configuration register address is not DWord aligned
  @retval EFI_DEVICE_ERROR      Error in SCRS during polling stage of operation
**/
EFI_STATUS
PchEspiSlaveGetConfig (
  IN  UINT32 SlaveId,
  IN  UINT32 SlaveAddress,
  OUT UINT32 *OutData
  )
{
  //
  // 1. Clear status from previous transaction by writing 111b to status in SCRS, PCR[eSPI] + 4000h [30:28]
  // 2. Program SLV_CFG_REG_CTL with the right value (Bit[31]=01, Bit [20:19]=<SlvID>, Bit [17:16] = 00b, Bit[11:0] = <addr_xxx>.
  // 3. Poll the SCRE (PCR[eSPI] +4000h [31]) to be set back to 0
  // 4. Check the transaction status in SCRS (bits [30:28])
  // 5. Read SLV_CFG_REG_DATA.
  //
  return EspiSlaveOperationHelper (SlaveId, SlaveAddress, EspiSlaveOperationConfigRead, OutData);
}

/**
  Set eSPI slave configuration

  Note: A Set_Configuration must always be followed by a Get_Configuration in order to ensure
  that the internal state of the eSPI-MC is consistent with the Slave's register settings.

  @param[in]  SlaveId       eSPI slave ID
  @param[in]  SlaveAddress  Slave Configuration Register Address
  @param[in]  InData        Configuration data to write

  @retval EFI_SUCCESS           Operation succeed
  @retval EFI_INVALID_PARAMETER Slave ID is not supported or SlaveId 1 is used in PCH_LP
  @retval EFI_INVALID_PARAMETER Slave configuration register address exceed maximum allowed
  @retval EFI_INVALID_PARAMETER Slave configuration register address is not DWord aligned
  @retval EFI_ACCESS_DENIED     eSPI Slave write to address range 0 to 0x7FF has been locked
  @retval EFI_DEVICE_ERROR      Error in SCRS during polling stage of operation
**/
EFI_STATUS
PchEspiSlaveSetConfig (
  IN  UINT32 SlaveId,
  IN  UINT32 SlaveAddress,
  IN  UINT32 InData
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;

  //
  // 1. Clear status from previous transaction by writing 111b to status in SCRS, PCR[eSPI] + 4000h [30:28]
  // 2. Program SLV_CFG_REG_DATA with the write value.
  // 3. Program SLV_CFG_REG_CTL with the right value (Bit[31]=01, Bit [20:19]=<SlvID>, Bit [17:16] = 01b, Bit[11:0] = <addr_xxx>.
  // 4. Poll the SCRE (PCR[eSPI] +4000h [31]) to be set back to 0
  // 5. Check the transaction status in SCRS (bits [30:28])
  //
  Status = EspiSlaveOperationHelper (SlaveId, SlaveAddress, EspiSlaveOperationConfigWrite, &InData);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Status = PchEspiSlaveGetConfig (SlaveId, SlaveAddress, &Data32);
  return Status;
}

/**
  Get status from eSPI slave

  @param[in]  SlaveId       eSPI slave ID
  @param[out] OutData       Configuration data read

  @retval EFI_SUCCESS           Operation succeed
  @retval EFI_INVALID_PARAMETER Slave ID is not supported or SlaveId 1 is used in PCH_LP
  @retval EFI_DEVICE_ERROR      Error in SCRS during polling stage of operation
**/
EFI_STATUS
PchEspiSlaveGetStatus (
  IN  UINT32 SlaveId,
  OUT UINT16 *OutData
  )
{
  EFI_STATUS  Status;
  UINT32      TempOutData;

  TempOutData = 0;

  //
  // 1. Clear status from previous transaction by writing 111b to status in SCRS, PCR[eSPI] + 4000h [30:28]
  // 2. Program SLV_CFG_REG_CTL with the right value (Bit[31]=01, Bit [20:19]=<SlvID>, Bit [17:16] = 10b, Bit[11:0] = <addr_xxx>.
  // 3. Poll the SCRE (PCR[eSPI] +4000h [31]) to be set back to 0
  // 4. Check the transaction status in SCRS (bits [30:28])
  // 5. Read SLV_CFG_REG_DATA [15:0].
  //
  Status = EspiSlaveOperationHelper (SlaveId, 0, EspiSlaveOperationStatusRead, &TempOutData);
  *OutData = (UINT16) TempOutData;

  return Status;
}

/**
  eSPI slave in-band reset

  @param[in]  SlaveId           eSPI slave ID

  @retval EFI_SUCCESS           Operation succeed
  @retval EFI_INVALID_PARAMETER Slave ID is not supported or SlaveId 1 is used in PCH_LP
  @retval EFI_DEVICE_ERROR      Error in SCRS during polling stage of operation
**/
EFI_STATUS
PchEspiSlaveInBandReset (
  IN  UINT32 SlaveId
  )
{
  //
  // 1. Clear status from previous transaction by writing 111b to status in SCRS, PCR[eSPI] + 4000h [30:28]
  // 2. Program SLV_CFG_REG_CTL with the right value (Bit[31]=01, Bit [20:19]=<SlvID>, Bit [17:16] = 11b).
  // 3. Poll the SCRE (PCR[eSPI] +4000h [31]) to be set back to 0
  // 4. Check the transaction status in SCRS (bits [30:28])
  //
  return EspiSlaveOperationHelper (SlaveId, 0, EspiSlaveOperationInBandReset, NULL);
}

/**
  eSPI Slave channel reset helper function

  @param[in]  SlaveId           eSPI slave ID
  @param[in]  ChannelNumber     Number of channel to reset

  @retval     EFI_SUCCESS       Operation succeeded
  @retval     EFI_UNSUPPORTED   Slave doesn't support that channel or invalid number specified
  @retval     EFI_TIMEOUT       Operation has timeouted
**/
EFI_STATUS
PchEspiSlaveChannelReset (
  IN  UINT8   SlaveId,
  IN  UINT8   ChannelNumber
  )
{
  UINT8       Timeout;
  UINT32      Data32;
  UINT32      SlaveChannelAddress;
  BOOLEAN     SlaveBmeSet;
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "eSPI slave %d channel %d reset\n", SlaveId, ChannelNumber));

  Timeout = CHANNEL_RESET_TIMEOUT;
  SlaveBmeSet = FALSE;

  if (!IsEspiSlaveChannelSupported (SlaveId, ChannelNumber)) {
    // Incorrect channel number was specified. Either exceeded max or Slave doesn't support that channel.
    DEBUG ((DEBUG_ERROR, "Channel %d is not valid channel number for slave %d!\n", ChannelNumber, SlaveId));
    return EFI_UNSUPPORTED;
  }

  // Calculating slave channel address
  SlaveChannelAddress = R_ESPI_SLAVE_CHACAP_BASE + (S_ESPI_SLAVE_CHACAP_OFFSET * ChannelNumber);

  // If we're resetting Peripheral Channel then we need to disable Bus Mastering first and reenable after reset
  if (ChannelNumber == 0) {
    Status = PchEspiSlaveGetConfig (SlaveId, SlaveChannelAddress, &Data32);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    if ((Data32 & B_ESPI_SLAVE_BME) != 0) {
      Data32 &= ~(B_ESPI_SLAVE_BME);
      Status = PchEspiSlaveSetConfig (SlaveId, SlaveChannelAddress, Data32);
      if (EFI_ERROR (Status)) {
        return Status;
      }
      SlaveBmeSet = TRUE;
    }
  }

  // Disable channel
  Status = PchEspiSlaveGetConfig (SlaveId, SlaveChannelAddress, &Data32);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Data32 &= ~(B_ESPI_SLAVE_CHACAP_CHEN);
  Status = PchEspiSlaveSetConfig (SlaveId, SlaveChannelAddress, Data32);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  // Enable channel
  Status = PchEspiSlaveGetConfig (SlaveId, SlaveChannelAddress, &Data32);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Data32 |= B_ESPI_SLAVE_CHACAP_CHEN;
  Status = PchEspiSlaveSetConfig (SlaveId, SlaveChannelAddress, Data32);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  DEBUG ((DEBUG_INFO, "Waiting for Channel Ready bit\n"));
  // Wait until channel is ready by polling Channel Ready bit
  while (((Data32 & B_ESPI_SLAVE_CHACAP_CHRDY) == 0) && (Timeout > 0)) {
    Status = PchEspiSlaveGetConfig (SlaveId, SlaveChannelAddress, &Data32);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    MicroSecondDelay (1);
    --Timeout;
  }

  if (Timeout == 0) {
    // The waiting for channel to be ready has timed out
    DEBUG ((DEBUG_ERROR, "The operation of channel %d reset for slave %d has timed out!\n", ChannelNumber, SlaveId));
    return EFI_TIMEOUT;
  }

  if (ChannelNumber == 0 && SlaveBmeSet) {
    Status = PchEspiSlaveGetConfig (SlaveId, SlaveChannelAddress, &Data32);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Data32 |= B_ESPI_SLAVE_BME;
    Status = PchEspiSlaveSetConfig (SlaveId, SlaveChannelAddress, Data32);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}
