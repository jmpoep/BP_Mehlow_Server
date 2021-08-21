/** @file
  Policy definition for System Agent overclocking Config Block

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
#ifndef _OVERCLOCKING_PREMEM_CONFIG__H_
#define _OVERCLOCKING_PREMEM_CONFIG__H_
#pragma pack(push, 1)

#define SA_OVERCLOCKING_CONFIG_REVISION 2

/**
 Defines the overclocking configuration parameters for System Agent.\n
  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Add GT unslice support.

**/
typedef struct {
  CONFIG_BLOCK_HEADER  Header;    ///< Offset 0-27 Config Block Header
  /**
  Offset 28:0 :
  Enable disable of SA overclocking mailbox commands.
  If disabled, or if PcdSaOcEnable is disabled, all other policies in this config block are ignored.
  <b>0=Disable</b>,
  1=Enable
  **/
  UINT32  OcSupport             : 1;
  UINT32  GtVoltageMode         : 1;    ///< Offset 28:1 :Specifies whether GT voltage is operating in Adaptive or Override mode: <b>0=Adaptive</b>, 1=Override
  UINT32  RealtimeMemoryTiming  : 1;    ///< Offset 28:2 :Enable/Disable the message sent to the CPU to allow realtime memory timing changes after MRC_DONE. <b>0=Disable</b>, 1=Enable
  UINT32  GtusVoltageMode       : 1;    ///< Offset 28:3 :Specifies whether GT unslice voltage is operating in Adaptive or Override mode: <b>0=Adaptive</b>, 1=Override
  UINT32  RsvdBits0             : 28;   ///< Offset 28:4 - 31 :Reserved for future use
  UINT8   GtMaxOcRatio;                 ///< Offset 32 Maximum GT turbo ratio override: 0=Minimal, 255=Maximum, <b>0=AUTO</b>
  UINT8   Rsvd0;                        ///< Offset 33 Reserved for DWORD alignment
  INT16   GtVoltageOffset;              ///< Offset 34 The voltage offset applied to GT slice. Valid range from -1000mv to 1000mv: <b>0=Minimal</b>, 1000=Maximum
  UINT16  GtVoltageOverride;            ///< Offset 36 The GT voltage override which is applied to the entire range of GT frequencies <b>0=Default</b>
  UINT16  GtExtraTurboVoltage;          ///< Offset 38 The adaptive voltage applied during turbo frequencies. Valid range from 0 to 2000mV: <b>0=Minimal</b>, 2000=Maximum
  INT16   SaVoltageOffset;              ///< Offset 40 The voltage offset applied to the SA. Valid range from -1000mv to 1000mv: <b>0=Default</b>
  INT16   GtusVoltageOffset;            ///< Offset 42 The voltage offset applied to GT unslice. Valid range from -1000mv to 1000mv: <b>0=Minimal</b>, 1000=Maximum
  UINT16  GtusVoltageOverride;          ///< Offset 44 The GT unslice voltage override which is applied to the entire range of GT frequencies: <b>0=Default</b>
  UINT16  GtusExtraTurboVoltage;        ///< Offset 46 The adaptive voltage applied during turbo frequencies. Valid range from 0 to 2000mV: <b>0=Default</b>
  UINT8   GtusMaxOcRatio;               ///< Offset 48 Maximum GTus turbo ratio override: 0=Minimal, 6=Maximum, <b>0=AUTO</b>
  UINT8   Rsvd1[3];                     ///< Offset 49-51 Reserved for DWORD alignment
} OVERCLOCKING_PREMEM_CONFIG;
#pragma pack(pop)

#endif // _OVERCLOCKING_CONFIG_H_
