/** @file
  This file contains functions to get DDR IO Data Offsets
  used memory training.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2017 Intel Corporation.

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

#include "McAddress.h"
#include "MrcInterface.h"
#include "MrcDdrIoOffsets.h"
#include "MrcHalRegisterAccess.h"
///
/// Local Functions
///
/*
  This function returns the offset to access specific Channel/Strobe of DataControl0.

  @params[in]  Channel - 0-based index of Channel to access.
  @params[in]  Byte    - 0-based index of Byte to access.

  @retval UINT32 - CR offset
*/
static
UINT32
DataControl0Offset (
  IN  UINT32  const   Channel,
  IN  UINT32  const   Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL0_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset  = DDRDATACH0_CR_DDRCRDATACONTROL0_CNL_REG;
    Offset += (DDRDATACH1_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL0_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG) * Strobe;
  }

  return Offset;
}

/*
  This function returns the offset to access specific Channel/Strobe of DataControl1.

  @params[in]  Channel - 0-based index of Channel to access.
  @params[in]  Byte    - 0-based index of Byte to access.

  @retval UINT32 - CR offset
*/
static
UINT32
DataControl1Offset (
  IN  UINT32  const   Channel,
  IN  UINT32  const   Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL1_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset  = DDRDATACH0_CR_DDRCRDATACONTROL1_CNL_REG;
    Offset += (DDRDATACH1_CR_DDRCRDATACONTROL1_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL1_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL1_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL1_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL1_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL1_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL1_CNL_REG) * Strobe;
  }

  return Offset;
}

/*
  This function returns the offset to access specific Channel/Strobe of DataControl2.

  @params[in]  Channel - 0-based index of Channel to access.
  @params[in]  Byte    - 0-based index of Byte to access.

  @retval UINT32 - CR offset
*/
static
UINT32
DataControl2Offset (
  IN  UINT32  const Channel,
  IN  UINT32  const Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL2_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset  = DDRDATACH0_CR_DDRCRDATACONTROL2_CNL_REG;
    Offset += (DDRDATACH1_CR_DDRCRDATACONTROL2_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL2_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL2_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL2_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL2_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL2_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL2_CNL_REG) * Strobe;
  }

  return Offset;
}

/*
  This function returns the offset to access specific Channel/Strobe of DataControl3.

  @params[in]  Channel - 0-based index of Channel to access.
  @params[in]  Byte    - 0-based index of Byte to access.

  @retval UINT32 - CR offset
*/
static
UINT32
DataControl3Offset (
  IN  UINT32  const Channel,
  IN  UINT32  const Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL3_CNL_REG;
  }
  else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset = DDRDATACH0_CR_DDRCRDATACONTROL3_CNL_REG;
    Offset += (DDRDATACH1_CR_DDRCRDATACONTROL3_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL3_CNL_REG) * Channel;
  }
  else {
    Offset = DDRDATA0CH0_CR_DDRCRDATACONTROL3_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL3_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL3_CNL_REG) * Channel +
      (DDRDATA1CH0_CR_DDRCRDATACONTROL3_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL3_CNL_REG) * Strobe;
  }

  return Offset;
}

/*
  This function returns the offset to access specific Channel/Strobe of DataControl4.

  @params[in]  Channel - 0-based index of Channel to access.
  @params[in]  Byte    - 0-based index of Byte to access.

  @retval UINT32 - CR offset
*/
static
UINT32
DataControl4Offset (
  IN  UINT32  const Channel,
  IN  UINT32  const Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL4_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset = DDRDATACH0_CR_DDRCRDATACONTROL4_CNL_REG +
              (DDRDATACH1_CR_DDRCRDATACONTROL4_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL4_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL4_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL4_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL4_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL4_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL4_CNL_REG) * Strobe;
  }

  return Offset;
}

static
UINT32
DataControl5Offset (
  IN  UINT32  const Channel,
  IN  UINT32  const Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL5_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset = DDRDATACH0_CR_DDRCRDATACONTROL5_CNL_REG +
              (DDRDATACH1_CR_DDRCRDATACONTROL5_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL5_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL5_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL5_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL5_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL5_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL5_CNL_REG) * Strobe;
  }

  return Offset;
}

static
UINT32
DataControl6Offset (
  IN  UINT32  const Channel,
  IN  UINT32  const Strobe
  )
{
  UINT32 Offset;

  if (Channel >= MAX_CHANNEL) {
    // Overall Broadcast
    Offset = DDRDATA_CR_DDRCRDATACONTROL6_CNL_REG;
  } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
    // Channel Broadcast
    Offset = DDRDATACH0_CR_DDRCRDATACONTROL6_CNL_REG +
              (DDRDATACH1_CR_DDRCRDATACONTROL6_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL6_CNL_REG) * Channel;
  } else {
    Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL6_CNL_REG;
    Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL6_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL6_CNL_REG) * Channel +
              (DDRDATA1CH0_CR_DDRCRDATACONTROL6_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL6_CNL_REG) * Strobe;
  }

  return Offset;
}

static
UINT32
WriteConfigOffset (
  IN  UINT32  const Channel,
  IN  UINT32  const SubCh
  )
{
  UINT32 Offset;

  Offset  = MCMISCS_WRITECFGCH0_CNL_REG;
  Offset += ((MCMISCS_WRITECFGCH1_CNL_REG - MCMISCS_WRITECFGCH0_CNL_REG) * SubCh) +
            ((MCMISCS_WRITECFGCH2_CNL_REG - MCMISCS_WRITECFGCH0_CNL_REG) * Channel);

  return Offset;
}

/**
  Function used to get the CR Offset for Data Read Groups.

  @param[in]  Group       - DDRIO group being accessed.
  @param[in]  Socket      - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel     - DDR Channel Number within the processor socket (0-based).
  @param[in]  SubChannel  - DDR SubChannel Number (0-based).
  @param[in]  Rank        - Rank index in the channel. (0-based).
  @param[in]  Strobe      - Dqs data group within the rank (0-based).
  @param[in]  Bit         - Bit index within the data group (0-based).
  @param[in]  FreqIndex   - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
GetDdrIoDataReadOffsets (
  IN  MrcParameters   *MrcData,
  IN  GSM_GT          Group,
  IN  UINT32          Socket,
  IN  UINT32          Channel,
  IN  UINT32          SubChannel,
  IN  UINT32          Rank,
  IN  UINT32          Strobe,
  IN  UINT32          Bit,
  IN  UINT32          FreqIndex
  )
{
  UINT32  Offset;

  Offset  = MRC_UINT32_MAX;

  switch (Group) {
    case RxRankMuxDelay:
      // Only Rank0 register has this field.
      Rank = 0;
      // Fall Through
    case RecEnDelay:
    case RxDqsPDelay:
    case RxDqsNDelay:
    case RxEq:
      if (Channel >= MAX_CHANNEL) {
        // Channel/Strobe Broadcast
        Offset = DDRDATA_CR_RXTRAINRANK0_CNL_REG;
        Offset += ((DDRDATA_CR_RXTRAINRANK1_CNL_REG - DDRDATA_CR_RXTRAINRANK0_CNL_REG) * Rank);
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_RXTRAINRANK0_CNL_REG;
        Offset += ((DDRDATACH1_CR_RXTRAINRANK0_CNL_REG - DDRDATACH0_CR_RXTRAINRANK0_CNL_REG) * Channel) +
          ((DDRDATACH0_CR_RXTRAINRANK1_CNL_REG - DDRDATACH0_CR_RXTRAINRANK0_CNL_REG) * Rank);
      } else {
        Offset = DDRDATA0CH0_CR_RXTRAINRANK0_CNL_REG +
          ((DDRDATA0CH1_CR_RXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_RXTRAINRANK0_CNL_REG) * Channel) +
          ((DDRDATA0CH0_CR_RXTRAINRANK1_CNL_REG - DDRDATA0CH0_CR_RXTRAINRANK0_CNL_REG) * Rank) +
          ((DDRDATA1CH0_CR_RXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_RXTRAINRANK0_CNL_REG) * Strobe);
      }
      break;

    case RxDqBitDelay:
      if (Channel >= MAX_CHANNEL) {
        // Channel/Strobe Broadcast
        Offset = DDRDATA_CR_RXPERBITRANK0_CNL_REG;
        Offset += (DDRDATA_CR_RXPERBITRANK1_CNL_REG - DDRDATA_CR_RXPERBITRANK0_CNL_REG) * Rank;
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_RXPERBITRANK0_CNL_REG;
        Offset += (DDRDATACH1_CR_RXPERBITRANK0_CNL_REG - DDRDATACH0_CR_RXPERBITRANK0_CNL_REG) * Channel +
          (DDRDATACH0_CR_RXPERBITRANK1_CNL_REG - DDRDATACH0_CR_RXPERBITRANK0_CNL_REG) * Rank;
      } else {
        Offset = DDRDATA0CH0_CR_RXPERBITRANK0_CNL_REG +
          ((DDRDATA0CH1_CR_RXPERBITRANK0_CNL_REG - DDRDATA0CH0_CR_RXPERBITRANK0_CNL_REG) * Channel) +
          ((DDRDATA0CH0_CR_RXPERBITRANK1_CNL_REG - DDRDATA0CH0_CR_RXPERBITRANK0_CNL_REG) * Rank) +
          ((DDRDATA1CH0_CR_RXPERBITRANK0_CNL_REG - DDRDATA0CH0_CR_RXPERBITRANK0_CNL_REG) * Strobe);
      }
      break;

    case RoundTripDelay:
      Offset = CH0_CR_SC_ROUNDTRIP_LATENCY_CNL_REG +
        ((CH1_CR_SC_ROUNDTRIP_LATENCY_CNL_REG - CH0_CR_SC_ROUNDTRIP_LATENCY_CNL_REG) * Channel);
      break;

    case RxFlybyDelay:
      Offset  = MCMISCS_READCFGCHACHB0_CNL_REG;
      Offset += (MCMISCS_READCFGCHACHB1_CNL_REG - MCMISCS_READCFGCHACHB0_CNL_REG) *Channel;
      break;

    case RxFifoRdEnTclDelay:
      Offset  = MCMISCS_READCFGCH0_CNL_REG;
      Offset += ((MCMISCS_READCFGCH1_CNL_REG - MCMISCS_READCFGCH0_CNL_REG) * SubChannel) +
                ((MCMISCS_READCFGCH2_CNL_REG - MCMISCS_READCFGCH0_CNL_REG) * Channel);
      break;

    case RxFifoRdEnFlybyDelay:
      Offset  = MCMISCS_RXDQFIFORDENCHACHB0_CNL_REG;
      Offset += (MCMISCS_RXDQFIFORDENCHACHB1_CNL_REG - MCMISCS_RXDQFIFORDENCHACHB0_CNL_REG) * Channel;
      break;

    case RxIoTclDelay:
    case RxDqDataValidDclkDelay:
    case RxDqDataValidQclkDelay:
      Offset  = MCMISCS_READCFGCH0_CNL_REG;
      Offset += ((MCMISCS_READCFGCH1_CNL_REG - MCMISCS_READCFGCH0_CNL_REG) * SubChannel) +
                ((MCMISCS_READCFGCH2_CNL_REG - MCMISCS_READCFGCH0_CNL_REG) * Channel);
      break;

    case SenseAmpDelay:
    case SenseAmpDelayLSB:
    case SenseAmpDuration:
    case McOdtDelay:
    case McOdtDelayLSB:
    case McOdtDuration:
    case RxBiasIComp:
      Offset = DataControl1Offset (Channel, Strobe);
      break;

    case McOdtDelayMSB:
    case SenseAmpDelayMSB:
      Offset = DataControl0Offset (Channel, Strobe);
      break;

    case RxDqsAmpOffset:
    case RxCben:
      Offset = DataControl2Offset (Channel, Strobe);
      break;

    case CBEnAmp1:
      Offset = DataControl3Offset(Channel, Strobe);
      break;

    case RxBiasRCompLsb:
      Offset = DataControl4Offset (Channel, Strobe);
      break;

    case RxVref:
      Offset = DataControl5Offset (Channel, Strobe);
      break;

    case RxBiasRCompMsb:
      Offset = DataControl6Offset (Channel, Strobe);
      break;

    case RxVoc:
      if (Channel >= MAX_CHANNEL) {
        // Channel/Strobe Broadcast
        Offset = DDRDATA_CR_RXOFFSETVDQRANK0_CNL_REG;
        Offset += ((DDRDATA_CR_RXOFFSETVDQRANK1_CNL_REG - DDRDATA_CR_RXOFFSETVDQRANK0_CNL_REG) * Rank);
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_RXOFFSETVDQRANK0_CNL_REG;
        Offset += ((DDRDATACH1_CR_RXOFFSETVDQRANK0_CNL_REG - DDRDATACH0_CR_RXOFFSETVDQRANK0_CNL_REG) * Channel) +
          ((DDRDATACH0_CR_RXOFFSETVDQRANK1_CNL_REG - DDRDATACH0_CR_RXOFFSETVDQRANK0_CNL_REG) * Rank);
      } else {
        Offset = DDRDATA0CH0_CR_RXOFFSETVDQRANK0_CNL_REG +
          ((DDRDATA0CH1_CR_RXOFFSETVDQRANK0_CNL_REG - DDRDATA0CH0_CR_RXOFFSETVDQRANK0_CNL_REG) * Channel) +
          ((DDRDATA0CH0_CR_RXOFFSETVDQRANK1_CNL_REG - DDRDATA0CH0_CR_RXOFFSETVDQRANK0_CNL_REG) * Rank) +
          ((DDRDATA1CH0_CR_RXOFFSETVDQRANK0_CNL_REG - DDRDATA0CH0_CR_RXOFFSETVDQRANK0_CNL_REG) * Strobe);
      }
      break;

    default:
      break;
  }

  return Offset;
}

/**
  Function used to get the CR Offset for Write Data Groups.

  @param[in]  Group       - DDRIO group being accessed.
  @param[in]  Socket      - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel     - DDR Channel Number within the processor socket (0-based).
  @param[in]  SubChannel  - DDR SubChannel Number in the channel(0-based).
  @param[in]  Rank        - Rank index in the channel. (0-based).
  @param[in]  Strobe      - Dqs data group within the rank (0-based).
  @param[in]  Bit         - Bit index within the data group (0-based).
  @param[in]  FreqIndex   - Index supporting multiple operating frequencies. (Not used in Core MRC)

  @retval CR Offset.
**/
UINT32
GetDdrIoDataWriteOffsets (
  IN  MrcParameters   *MrcData,
  IN  GSM_GT          Group,
  IN  UINT32          Socket,
  IN  UINT32          Channel,
  IN  UINT32          SubChannel,
  IN  UINT32          Rank,
  IN  UINT32          Strobe,
  IN  UINT32          Bit,
  IN  UINT32          FreqIndex
  )
{
  UINT32 Offset;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case TxRankMuxDelay:
      // This field only apply to rank zero.
      Rank = 0;
      // Fall Through
    case TxDqsDelay:
    case TxDqDelay:
    case TxEq:
      if (Channel >= MAX_CHANNEL) {
        // Channel/Strobe Broadcast
        Offset = DDRDATA_CR_TXTRAINRANK0_CNL_REG;
        Offset += ((DDRDATA_CR_TXTRAINRANK1_CNL_REG - DDRDATA_CR_TXTRAINRANK0_CNL_REG) * Rank);
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_TXTRAINRANK0_CNL_REG;
        Offset += ((DDRDATACH1_CR_TXTRAINRANK0_CNL_REG - DDRDATACH0_CR_TXTRAINRANK0_CNL_REG) * Channel) +
          ((DDRDATACH0_CR_TXTRAINRANK1_CNL_REG - DDRDATACH0_CR_TXTRAINRANK0_CNL_REG) * Rank);
      } else {
        Offset = DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG +
          ((DDRDATA0CH1_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Channel) +
          ((DDRDATA0CH0_CR_TXTRAINRANK1_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Rank) +
          ((DDRDATA1CH0_CR_TXTRAINRANK0_CNL_REG - DDRDATA0CH0_CR_TXTRAINRANK0_CNL_REG) * Strobe);
      }
      break;

    case TxDqBitDelay:
      if (Channel >= MAX_CHANNEL) {
        // Channel/Strobe Broadcast
        Offset = DDRDATA_CR_TXPERBITRANK0_CNL_REG;
        Offset += ((DDRDATA_CR_TXPERBITRANK1_CNL_REG - DDRDATA_CR_TXPERBITRANK0_CNL_REG) * Rank);
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_TXPERBITRANK0_CNL_REG;
        Offset += ((DDRDATACH1_CR_TXPERBITRANK0_CNL_REG - DDRDATACH0_CR_TXPERBITRANK0_CNL_REG) * Channel) +
          ((DDRDATACH0_CR_TXPERBITRANK1_CNL_REG - DDRDATACH0_CR_TXPERBITRANK0_CNL_REG) * Rank);
      } else {
        Offset = DDRDATA0CH0_CR_TXPERBITRANK0_CNL_REG +
          ((DDRDATA0CH1_CR_TXPERBITRANK0_CNL_REG - DDRDATA0CH0_CR_TXPERBITRANK0_CNL_REG) * Channel) +
          ((DDRDATA0CH0_CR_TXPERBITRANK1_CNL_REG - DDRDATA0CH0_CR_TXPERBITRANK0_CNL_REG) * Rank) +
          ((DDRDATA1CH0_CR_TXPERBITRANK0_CNL_REG - DDRDATA0CH0_CR_TXPERBITRANK0_CNL_REG) * Strobe);
      }
      break;

    case TxVref:
      Offset = DDRVREF_CR_DDRCRVREFADJUST1_CNL_REG;
      break;

    case TxDqFifoRdEnFlybyDelay:
      Offset  = MCMISCS_WRITECFGCHACHB0_CNL_REG;
      Offset += (MCMISCS_WRITECFGCHACHB1_CNL_REG - MCMISCS_WRITECFGCHACHB0_CNL_REG) * Channel;
      break;

    case TxDqFifoWrEnTcwlDelay:
    case TxDqFifoRdEnTcwlDelay:
      Offset = WriteConfigOffset (Channel, SubChannel);
      break;

    default:
      break;
    }

  return Offset;
}

/**
  Function used to get the CR Offset for training offset and comp offset Groups.

  @param[in]  MrcData   - Global MRC data structure.
  @param[in]  Group     - DDRIO group being accessed.
  @param[in]  Socket    - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel   - DDR Channel Number within the processor socket (0-based).
  @param[in]  Rank      - Rank index in the channel. (0-based).
  @param[in]  Strobe    - Dqs data group within the rank (0-based).
  @param[in]  Bit       - Bit index within the data group (0-based).
  @param[in]  FreqIndex - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
GetDdrIoDataTrainOffsets (
  IN MrcParameters    *MrcData,
  IN  GSM_GT          Group,
  IN  UINT32          Socket,
  IN  UINT32          Channel,
  IN  UINT32          Rank,
  IN  UINT32          Strobe,
  IN  UINT32          Bit,
  IN  UINT32          FreqIndex
  )
{
  UINT32 Offset;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case RecEnOffset:
    case RxDqsOffset:
    case RxVrefOffset:
    case TxDqsOffset:
    case TxDqOffset:
      if (Channel >= MAX_CHANNEL) {
        // Overall Broadcast
        Offset = DDRDATA_CR_DDRCRDATAOFFSETTRAIN_CNL_REG;
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset = DDRDATACH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG;
        Offset += ((DDRDATACH1_CR_DDRCRDATAOFFSETTRAIN_CNL_REG - DDRDATACH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG) * Channel);
      } else {
        // Specific Channel and Strobe
        Offset = DDRDATA0CH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG +
          ((DDRDATA0CH1_CR_DDRCRDATAOFFSETTRAIN_CNL_REG - DDRDATA0CH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG) * Channel) +
          ((DDRDATA1CH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG - DDRDATA0CH0_CR_DDRCRDATAOFFSETTRAIN_CNL_REG) * Strobe);
      }
      break;

    case CompOffsetAll:
    case TxTcoCompOffset:
      if (Channel >= MAX_CHANNEL) {
        // Controller Broadcast
        Offset = DDRDATA_CR_DDRCRDATAOFFSETCOMP_CNL_REG;
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset  = DDRDATACH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG;
        Offset += ((DDRDATACH1_CR_DDRCRDATAOFFSETCOMP_CNL_REG - DDRDATACH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG) * Channel);
      } else {
        // Specific Channel and Strobe
        Offset = DDRDATA0CH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG +
          ((DDRDATA0CH1_CR_DDRCRDATAOFFSETCOMP_CNL_REG - DDRDATA0CH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG) * Channel) +
          ((DDRDATA1CH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG - DDRDATA0CH0_CR_DDRCRDATAOFFSETCOMP_CNL_REG) * Strobe);
      }
      break;

    default:
      break;
  }

  return Offset;
}

/**
  Function used to get the CR Offset for DDR IO Configuration settings.

  @param[in]  MrcData     - Global MRC data structure.
  @param[in]  Group       - DDRIO group being accessed.
  @param[in]  Socket      - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel     - DDR Channel Number within the processor socket (0-based).
  @param[in]  SubChannel  - DDR SubChannel within the channel Number(0-based).
  @param[in]  Rank        - Rank index in the channel. (0-based).
  @param[in]  Strobe      - Dqs data group within the rank (0-based).
  @param[in]  Bit         - Bit index within the data group (0-based).
  @param[in]  FreqIndex   - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
MrcGetDdrIoConfigOffsets (
  IN MrcParameters    *MrcData,
  IN  GSM_GT          Group,
  IN  UINT32          Socket,
  IN  UINT32          Channel,
  IN  UINT32          SubChannel,
  IN  UINT32          Rank,
  IN  UINT32          Strobe,
  IN  UINT32          Bit,
  IN  UINT32          FreqIndex
  )
{
  UINT32 Offset;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case GsmIocWlWakeCyc:
    case GsmIocWlSleepCyc:
    case GsmIocForceCmpUpdt:
    case GsmIocWlLatency:
    case GsmIocNoDqInterleave:
    case GsmIocScramLpMode:
    case GsmIocScramDdr4Mode:
    case GsmIocDisClkGate:
    case GsmIocDllWlEn:
    case GsmIocDisDataIdlClkGate:
    case GsmIoc4ChMode:
    case GsmIocScramLp4Mode:
    case GsmIocChNotPop:
    case GsmIocDisIosfSbClkGate:
    case GsmIocEccEn:
      Offset = DDRSCRAM_CR_DDRMISCCONTROL0_CNL_REG;
      break;

    case GsmIocIoReset:
      Offset = DDRSCRAM_CR_DDRMISCCONTROL1_CNL_REG;
      break;

    case GsmIocDataVccddqMode:
    case GsmIocDataOdtStaticDis:
    case GsmIocDataCtlLpMode:
    case GsmIocLpEarlyRLeak:
    case GsmIocOdtSampExtEn:
    case GsmIocTxOn:
    case GsmIocSenseAmpMode:
    case GsmIocReadLevelMode:
    case GsmIocWriteLevelMode:
    case GsmIocReadDqDqsMode:
    case GsmIocForceOdtOn:
    case GsmIocReadPreambleEn:
    case GsmIocRxPiPwrDnDis:
    case GsmIocTxPiPwrDnDis:
    case GsmIocTxDisable:
    case GsmIocLowSupEnDqDqs:
    case GsmIocInternalClocksOn:
      if (Channel >= MAX_CHANNEL) {
        // Overall Broadcast
        Offset = DDRDATA_CR_DDRCRDATACONTROL0_CNL_REG;
      } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
        // Rank Broadcast
        Offset  = DDRDATACH0_CR_DDRCRDATACONTROL0_CNL_REG;
        Offset += (DDRDATACH1_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATACH0_CR_DDRCRDATACONTROL0_CNL_REG) * Channel;
      } else {
        Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG;
        Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG) * Channel +
                  (DDRDATA1CH0_CR_DDRCRDATACONTROL0_CNL_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL0_CNL_REG) * Strobe;
      }
      break;

    case GsmIocDllMask:
    case GsmIocStrongWeakLeaker:
    case GsmIocSdllSegmentDisable:
    case GsmIocDllWeakLock:
    case GsmIocDataRefPi:
      Offset = DataControl1Offset (Channel, Strobe);
      break;

    case GsmIocLeakerComp:
    case GsmIocLongWrPreambleLp4:
    case GsmIocDataCtlLp4Mode:
    case GsmIocVssOdtEn:
    case GsmIocRxTypeSelect:
    case GsmIocRxBiasSel:
    case GsmIocRxDeskewCal:
    case GsmIocTxDeskewCal:
      Offset = DataControl4Offset (Channel, Strobe);
      break;

    case GsmIocRxClkStg:
    case GsmIocDataVddqOdtEn:
    case GsmIocDataVttOdtEn:
    case GsmIocRxVrefMFC:
    case GsmIocVrefPwrDnEn:
    case GsmIocDataCtlDdr4Mode:
    case GsmIocRxDeskewDis:
    case GsmIocTxDeskewDis:
    case GsmIocRxStaggerCtl:
    case GsmIocConstZTxEqEn:
    case GsmIocTxEqDis:
    case GsmIocForceBiasOn:
    case GsmIocForceRxAmpOn:
    case GsmIocDqSlewDlyByPass:
    case GsmIocWlLongDelEn:
    case GsmIocLpLongOdtEn:
      Offset = DataControl2Offset (Channel, Strobe);
      break;
/*
    case DataControl3:
        if (Channel >= MAX_CHANNEL) {
          // Overall Broadcast
          Offset = DDRDATA_CR_DDRCRDATACONTROL3_CNL_REG;
        } else if (Strobe >= MAX_SDRAM_IN_DIMM) {
          // Rank Broadcast
          Offset  = DDRDATACH0_CR_DDRCRDATACONTROL3_CNL_REG;
          Offset += (DDRDATACH1_CR_DDRCRDATACONTROL3_REG - DDRDATACH0_CR_DDRCRDATACONTROL3_REG) * Channel;
        } else {
          Offset  = DDRDATA0CH0_CR_DDRCRDATACONTROL3_REG;
          Offset += (DDRDATA0CH1_CR_DDRCRDATACONTROL3_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL3_REG) * Channel +
                    (DDRDATA1CH0_CR_DDRCRDATACONTROL3_REG - DDRDATA0CH0_CR_DDRCRDATACONTROL3_REG) * Strobe;
        }
        break;
*/
    case GsmIocLp4DqsMaskPulseCnt:
    case GsmIocLp4DqsPulseCnt:
    case GsmIocLp4DqsPulseEn:
    case GsmIocLp4ForceDqsNOn:
        Offset = DataControl5Offset (Channel, Strobe);
        break;

    case GsmIocRankOverrideEn:
    case GsmIocRankOverrideVal:
      Offset = DataControl6Offset (Channel, Strobe);
      break;

    case GsmIocTxVrefConverge:
    case GsmIocCmdVrefConverge:
      Offset = DDRVREF_CR_DDRCRVREFADJUST2_CNL_REG;
      break;

    case GsmIocCompOdtStaticDis:
    case GsmIocDqOdtUpDnOff:
    case GsmIocFixOdtD:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL0_CNL_REG;
      break;

    case GsmIocCompClkOn:
    case GsmIocCompVccddqHi:
    case GsmIocCompVccddqLo:
    case GsmIocDisableQuickComp:
    case GsmIocSinStep:
    case GsmIocSinStepAdv:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL1_CNL_REG;
      break;

    case GsmIocCompVddqOdtEn:
    case GsmIocCompVttOdtEn:
    case GsmIocVttPanicCompUpMult:
    case GsmIocVttPanicCompDnMult:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL2_CNL_REG;
      break;

    case GsmIocCmdDrvVref200ohm:
      Offset = DDRPHY_COMP_CR_DDRCRDATACOMP2_CNL_REG;
      break;

    case GsmIocOdtOverride:
    case GsmIocOdtOn:
    case GsmIocMprTrainDdrOn:
      if (Channel >= MAX_CHANNEL) {
        // Overall Broadcast
        Offset = BC_CR_REUT_CH_MISC_ODT_CTRL_CNL_REG;
      } else {
        // Channel
        Offset = (Channel) ? CH1_CR_REUT_CH_MISC_ODT_CTRL_CNL_REG : CH0_CR_REUT_CH_MISC_ODT_CTRL_CNL_REG;
      }
      break;

    case GsmIocCkeOverride:
    case GsmIocEnStartTestSync:
    case GsmIocCkeOn:
      if (Channel >= MAX_CHANNEL) {
        // Overall Broadcast
        Offset = BC_CR_REUT_CH_MISC_CKE_CTRL_CNL_REG;
      } else {
        // Channel
        Offset = (Channel) ? CH1_CR_REUT_CH_MISC_CKE_CTRL_CNL_REG : CH0_CR_REUT_CH_MISC_CKE_CTRL_CNL_REG;
      }
      break;

    case GsmIocRptChRepClkOn:
    case GsmIocCmdAnlgEnGraceCnt:
    case GsmIocTxAnlgEnGraceCnt:
    case GsmIocTxDqFifoRdEnPerRankDelDis:
      Offset = WriteConfigOffset (Channel, SubChannel);
      break;

    default:
      break;
    }

  return Offset;
}

/**
  Function used to get the CR Offset for DDR IO Command Groups.

  @param[in]  MrcData     - Global MRC data structure.
  @param[in]  Group       - DDRIO group being accessed.
  @param[in]  Socket      - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel     - DDR Channel Number within the processor socket (0-based).
  @param[in]  GroupIndex  - Index for command group types that specify indicies (0-based).
  @param[in]  FreqIndex   - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset.
**/
UINT32
GetDdrIoCommandOffsets (
  IN MrcParameters    *MrcData,
  IN  GSM_GT          Group,
  IN  UINT32          Socket,
  IN  UINT32          Channel,
  IN  UINT32          GroupIndex,
  IN  UINT32          FreqIndex
  )
{
  UINT32 Offset;

  static const UINT32 CtlGrpBase[MRC_CTL_GRP_MAX] = {
    DDRCTLCH0_CR_DDRCRCTLPICODING_CNL_REG, DDRCTLCH0_CR_DDRCRCTLPICODING_CNL_REG,
    DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG, DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG};
  static const UINT32 CkeGrpBase[MRC_CTL_GRP_MAX] = {
    DDRCKECH0_CR_DDRCRCTLPICODING_CNL_REG, DDRCKECH0_CR_DDRCRCTLPICODING_CNL_REG,
    DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG, DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG};
  static const UINT32 CmdGrpBaseCnl[MRC_CMD_GRP_MAX] = {
    DDRCMDACH0_CR_DDRCRCMDPICODING_CNL_REG, DDRCMDACH0_CR_DDRCRCMDPICODING_CNL_REG,
    DDRCMDBCH0_CR_DDRCRCMDPICODING_CNL_REG, DDRCMDBCH0_CR_DDRCRCMDPICODING_CNL_REG,
    DDRCKECH0_CR_DDRCRCTLPICODING_CNL_REG};
  static const UINT32 ClkGrpBaseCnl[MRC_CLK_GRP_MAX] = {
    DDRCTLCH0_CR_DDRCRCTLPICODING_CNL_REG, DDRCTLCH0_CR_DDRCRCTLPICODING_CNL_REG,
    DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG, DDRCTLCLKCKE0_CR_DDRCRCTLPICODING_CNL_REG};
  static const UINT32 CmdCompOffset[MAX_SUB_CHANNEL] = {
    DDRCMDACH0_CR_DDRCRCMDCOMPOFFSET_CNL_REG, DDRCMDBCH0_CR_DDRCRCMDCOMPOFFSET_CNL_REG};

  static const UINT32 ClkCmdCtlInc = 0x100;

  Offset = MRC_UINT32_MAX;

  switch (Group) {
    case CtlGrpPi:
      if (GroupIndex < MRC_CTL_GRP_MAX) {
        Offset = CtlGrpBase[GroupIndex] + (ClkCmdCtlInc * Channel);
      }
      break;

    case CkeGrpPi:
      if (GroupIndex < MRC_CKE_GRP_MAX) {
        Offset = CkeGrpBase[GroupIndex] + (ClkCmdCtlInc * Channel);
      }
      break;

    case CmdGrpPi:
      if (GroupIndex < MRC_CMD_GRP_MAX) {
        Offset  = CmdGrpBaseCnl[GroupIndex];
        Offset += (ClkCmdCtlInc * Channel);
      }
      break;

    case ClkGrpPi:
      if (GroupIndex < MRC_CLK_GRP_MAX) {
        Offset  = ClkGrpBaseCnl[GroupIndex];
        Offset += (ClkCmdCtlInc * Channel);
      }
      break;

    case CmdVref:
      Offset = DDRVREF_CR_DDRCRVREFADJUST1_CNL_REG;
      break;

    case CmdRCompDrvDownOffset:
    case CmdRCompDrvUpOffset:
    case CmdTcoCompOffset:
    case CmdSCompOffset:
      // Cmd Comp Offset to a specific Channel and Sub-Channel
      if (GroupIndex < MAX_SUB_CHANNEL) {
        Offset = CmdCompOffset[GroupIndex];
        Offset += (ClkCmdCtlInc * Channel);
      }
      break;

    case CtlRCompDrvDownOffset:
    case CtlRCompDrvUpOffset:
    case CtlTcoCompOffset:
    case CtlSCompOffset:
      // Ctl Comp Offset to a specific Channel
      Offset = DDRCTLCH0_CR_DDRCRCTLCOMPOFFSET_CNL_REG +
        ((DDRCTLCH1_CR_DDRCRCTLCOMPOFFSET_CNL_REG - DDRCTLCH0_CR_DDRCRCTLCOMPOFFSET_CNL_REG) * Channel);
      break;

    case CkeRCompDrvDownOffset:
    case CkeRCompDrvUpOffset:
    case CkeTcoCompOffset:
    case CkeSCompOffset:
      // Cke Comp Offset to a specific Channel
      Offset = DDRCKECH0_CR_DDRCRCTLCOMPOFFSET_CNL_REG +
        ((DDRCKECH1_CR_DDRCRCTLCOMPOFFSET_CNL_REG - DDRCKECH0_CR_DDRCRCTLCOMPOFFSET_CNL_REG) * Channel);
      break;

    case ClkRCompDrvDownOffset:
    case ClkRCompDrvUpOffset:
    case ClkTcoCompOffset:
    case ClkSCompOffset:
    case ClkCompOnTheFlyUpdtEn:
      // Clk Comp Offset Specific Channel
      Offset = DDRCLKCH0_CR_DDRCRCLKCOMPOFFSET_CNL_REG +
        ((DDRCLKCH1_CR_DDRCRCLKCOMPOFFSET_CNL_REG - DDRCLKCH0_CR_DDRCRCLKCOMPOFFSET_CNL_REG) * Channel);
      break;


    case CtlGrp0:
    case CtlGrp1:
    case CtlGrp2:
    case CtlGrp3:
    case CtlGrp4:
    case CmdGrp0:
    case CmdGrp1:
    case CmdGrp2:
    case CkAll:
    case CmdAll:
    case CtlAll:
    default:
      break;
    }

  return Offset;
}

/**
  This function handles getting the register offset for the requested parameter.
  It will determine multicast by the parameter exceeding the MAX of the
  Socket/Channel/Rank/Strobe/Bit.

  @param[in]  MrcData     - Pointer to global data.
  @param[in]  Group       - DDRIO group to access.
  @param[in]  Socket      - Processor socket in the system (0-based).  Not used in Core MRC.
  @param[in]  Channel     - DDR Channel Number within the processor socket (0-based).
  @param[in]  SubChannel  - DDR SubChannel Number(0-based).
  @param[in]  Rank        - Rank number within a channel (0-based).
  @param[in]  Strobe      - Dqs data group within the rank (0-based).
  @param[in]  Bit         - Bit index within the data group (0-based).
  @param[in]  FreqIndex   - Index supporting multiple operating frequencies. (Not used in Client CPU's)

  @retval CR Offset
**/
UINT32
MrcGetDdrIoRegOffset (
  IN  MrcParameters *const  MrcData,
  IN  GSM_GT  const         Group,
  IN  UINT32  const         Socket,
  IN  UINT32  const         Channel,
  IN  UINT32  const         SubChannel,
  IN  UINT32  const         Rank,
  IN  UINT32  const         Strobe,
  IN  UINT32  const         Bit,
  IN  UINT32  const         FreqIndex
  )
{
  UINT32          Offset = MRC_UINT32_MAX;

  switch (Group) {
    case RecEnDelay:
    case RxDqsPDelay:
    case RxDqsNDelay:
    case RxVref:
    case RxEq:
    case RxDqBitDelay:
    case RoundTripDelay:
    case RxFlybyDelay:
    case RxIoTclDelay:
    case RxFifoRdEnFlybyDelay:
    case RxFifoRdEnTclDelay:
    case RxDqDataValidDclkDelay:
    case RxDqDataValidQclkDelay:
    case SenseAmpDelay:
    case SenseAmpDelayLSB:
    case SenseAmpDelayMSB:
    case SenseAmpDuration:
    case McOdtDelay:
    case McOdtDelayLSB:
    case McOdtDelayMSB:
    case McOdtDuration:
    case RxBiasIComp:
    case RxBiasRCompLsb:
    case RxBiasRCompMsb:
    case RxDqsAmpOffset:
    case RxCben:
    case RxVoc:
    case CBEnAmp1:
    case RxRankMuxDelay:
      Offset = GetDdrIoDataReadOffsets (MrcData, Group, Socket, Channel, SubChannel, Rank, Strobe, Bit, FreqIndex);
      break;

    case TxDqsDelay:
    case TxDqDelay:
    case TxEq:
    case TxRankMuxDelay:
    case TxDqBitDelay:
    case TxVref:
    case TxDqFifoWrEnTcwlDelay:
    case TxDqFifoRdEnTcwlDelay:
    case TxDqFifoRdEnFlybyDelay:
      Offset = GetDdrIoDataWriteOffsets (MrcData, Group, Socket, Channel, SubChannel, Rank, Strobe, Bit, FreqIndex);
      break;

    case RecEnOffset:
    case RxDqsOffset:
    case RxVrefOffset:
    case TxDqsOffset:
    case TxDqOffset:
    case TxTcoCompOffset:
      Offset = GetDdrIoDataTrainOffsets (MrcData, Group, Socket, Channel, Rank, Strobe, Bit, FreqIndex);
      break;

    case CtlGrpPi:
    case CkeGrpPi:
    case CmdGrpPi:
    case ClkGrpPi:
    case CmdVref:
    case CmdRCompDrvDownOffset:
    case CmdRCompDrvUpOffset:
    case CmdTcoCompOffset:
    case CmdSCompOffset:
    case CtlRCompDrvDownOffset:
    case CtlRCompDrvUpOffset:
    case CtlTcoCompOffset:
    case CtlSCompOffset:
    case ClkRCompDrvDownOffset:
    case ClkRCompDrvUpOffset:
    case ClkTcoCompOffset:
    case ClkSCompOffset:
    case ClkCompOnTheFlyUpdtEn:
    case CkeRCompDrvDownOffset:
    case CkeRCompDrvUpOffset:
    case CkeTcoCompOffset:
    case CkeSCompOffset:
      Offset = GetDdrIoCommandOffsets (MrcData, Group, Socket, Channel, Strobe, FreqIndex);
      break;

    case TxSlewRate:
    case DqScompPC:
    case CmdSlewRate:
    case CmdScompPC:
    case CtlSlewRate:
    case CtlScompPC:
    case ClkSlewRate:
    case ClkScompPC:
    case TcoCmdOffset:
    case DqRcompStatLegs:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL1_CNL_REG;
      break;

    case TxRonUp:
    case TxRonDn:
    case SCompCodeDq:
    case TxTco:
      Offset = DDRPHY_COMP_CR_DDRCRDATACOMP0_CNL_REG;
      break;

    case WrDSCodeUpCmd:
    case WrDSCodeDnCmd:
    case SCompCodeCmd:
    case TcoCompCodeCmd:
      Offset = DDRPHY_COMP_CR_DDRCRCMDCOMP_CNL_REG;
      break;

    case WrDSCodeUpCtl:
    case WrDSCodeDnCtl:
    case SCompCodeCtl:
    case TcoCompCodeCtl:
      Offset = DDRPHY_COMP_CR_DDRCRCTLCOMP_CNL_REG;
      break;

    case WrDSCodeUpClk:
    case WrDSCodeDnClk:
    case SCompCodeClk:
    case TcoCompCodeClk:
      Offset = DDRPHY_COMP_CR_DDRCRCLKCOMP_CNL_REG;
      break;

    case DqDrvVref:
    case DqOdtVref:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL0_CNL_REG;
      break;

    case CmdDrvVref:
    case CtlDrvVref:
    case ClkDrvVref:
      Offset = DDRPHY_COMP_CR_DDRCRCOMPCTL3_CNL_REG;
      break;

    case CompRcompOdtUp:
    case CompRcompOdtDn:
      Offset = DDRPHY_COMP_CR_DDRCRDATACOMP1_CNL_REG;
      break;

    default:
      break;
  }

  return Offset;
}
