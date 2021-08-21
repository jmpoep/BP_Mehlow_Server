/** @file
  Memory DXE Policy definitions

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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
#ifndef _MEMORY_DXE_CONFIG_H_
#define _MEMORY_DXE_CONFIG_H_

#pragma pack(push, 1)

#define MEMORY_DXE_CONFIG_REVISION 1

/**
  The Memory Configuration includes DIMM SPD address Map and DIMM Slot Mechanical present bit map.
  The data elements should be initialized by a Platform Module.\n
  <b>Revision 1</b>:
  - Initial version.
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                   ///< Offset 0-27: Config Block Header
/**
  Offset 28:
  Dimm SPD address
  Only Server support 2 channels * 3 slots per channel = 6 sockets totally
  The Desktop and mobile only support 2 channels * 2 slots per channel = 4 sockets totally
  So there is mapping rule here for Desktop and mobile that there are no more 4 DIMMS totally in a system:
    Channel A/ Slot 0 --> Dimm 0 --> SpdAddressTable[0]
    Channel A/ Slot 1 --> Dimm 1 --> SpdAddressTable[1]
    Channel B/ Slot 0 --> Dimm 2 --> SpdAddressTable[2]
    Channel B/ Slot 1 --> Dimm 3 --> SpdAddressTable[3]
  Refer to SmbiosMemory.c for use
  If change the mapping rule, please update the Revision number.
**/
  UINT8                 *SpdAddressTable;
/**
  Offset 36:
  Channel A DIMM Slot Mechanical present bit map, bit 0 -> DIMM 0, bit 1 -> DIMM1, ...
  if the bit is 1, the related DIMM slot is present.
  E.g. if channel A has 2 DIMMs,  ChannelASlotMap = 0x03;
  E.g. if channel A has only 1 DIMMs,  ChannelASlotMap = 0x01;
  Refer to SmbiosMemory.c
**/
  UINT8                 ChannelASlotMap;
/**
  Offset 37:
  Channel B DIMM Slot Mechanical present bit map, bit 0 -> DIMM 0, bit 1 -> DIMM1, ...
  if the bit is 1, the related DIMM slot is present.
  E.g. if channel B has 2 DIMMs,  ChannelBSlotMap = 0x03;
  E.g. if channel B has only 1 DIMMs,  ChannelBSlotMap = 0x01;
  Refer to SmbiosMemory.c
**/
  UINT8                 ChannelBSlotMap;
  UINT8                 MrcTimeMeasure;   ///< Offset 38: MRC execution time measurement: <b>0=Disable</b>, 1=Enable
  UINT8                 MrcFastBoot;      ///< Offset 39: Fast boot: 0=Disable, <b>1=Enable</b>
} MEMORY_DXE_CONFIG;
#pragma pack(pop)

#endif // _MEMORY_DXE_CONFIG_H_
