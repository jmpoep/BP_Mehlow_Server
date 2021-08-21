/** @file
  AMT Config Block for PEI phase

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2017 Intel Corporation.

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
#ifndef _AMT_PEI_CONFIG_H_
#define _AMT_PEI_CONFIG_H_

#define AMT_PEI_CONFIG_REVISION 1
extern EFI_GUID gAmtPeiConfigGuid;

#pragma pack (push,1)

/**
  AMT Pei Configuration Structure.

  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Add RemoteAssistance, AmtKvmEnabled and ForcMebxSyncUp
**/
typedef struct {
  CONFIG_BLOCK_HEADER   Header;                 ///< Config Block Header
  /**
    Enable or Disable Intel Active Management Technology feature.
    If disabled, all Intel AMT features, including Alert Standard Format features, will not be supported.
       0: Disable
    <b>1: Enable.</b>
  **/
  UINT32  AmtEnabled                      : 1;
  /**
    ME WatchDog timer feature. If disabled, below WatchDogTimerOs/WatchDogTimerBios will be irrelevant.
    See WatchDogTimerOs and WatchDogTimerBios description.
    <b>0: Disable</b>
       1: Enable ME WDT if corresponding timer value is not zero.
  **/
  UINT32  WatchDog                        : 1;

  /**
    Enable or Disable Alert Standard Format feature.
    If enabled, PEI platform status will be reported to CSME.
    This setting will also determine WatchDog is enabled or not.
       0: Disable
    <b>1: Enable.</b>
  **/
  UINT32  AsfEnabled                      : 1;
  UINT32  ManageabilityMode               : 1;  ///< Manageability Mode sync with Mebx, 0: Disabled; <b>1: AMT</b>
  UINT32  FwProgress                      : 1;  ///< PET Events Progress to receive PET Events. 0: Disable; <b>1: Enable</b>
  UINT32  AmtSolEnabled                   : 1;  ///< Serial Over Lan retrieved from Mebx. The default value depends on CSME/AMT. 0: Disable, <b>1: Enable</b>
  UINT32  RemoteAssistance                : 1;  ///< Remote Assistance is enabled if platform is provisioned.  <b>0: Disable</b>, 1: Enable
  UINT32  AmtKvmEnabled                   : 1;  ///< KVM retrieved from Mebx. The default value depends on CSME/AMT. 0: Disable, <b>1: Enable</b>
  UINT32  ForcMebxSyncUp                  : 1;  ///< <b>0: No</b>; 1: Force MEBX execution
  UINT32  RsvdBits                        : 23; ///< Reserved for future use & Config block alignment
  /**
    OS WatchDog Timer
    <b>0: Disable</b> OS WDT won't be started after stopping BIOS WDT even if WatchDog is enabled.
    Non zero value - OS WDT will be started after stopping BIOS WDT if WatchDog is enabled. The timer is set
    according to the value.
  **/
  UINT16  WatchDogTimerOs;
  /**
    BIOS WatchDog Timer
    <b>0: Disable</b> BIOS WDT won't be started even if WatchDog is enabled.
    Non zero value - The BIOS WDT is set according to the value and started if WatchDog is enabled.
  **/
  UINT16  WatchDogTimerBios;
} AMT_PEI_CONFIG;

#pragma pack (pop)

#endif // _AMT_PEI_CONFIG_H_
