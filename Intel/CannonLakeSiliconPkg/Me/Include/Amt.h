/** @file
  Header file for common Active Management Technology defines.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#ifndef _AMT_H_
#define _AMT_H_

///
/// Intel Internet Assigned Numbers Authority Manufacturer ID
/// (The firmware sends 0x57010000 for decimal value 343)
///
#define INTEL_IANA_SWAP32(x)      ((((x) & 0xff) << 24) | (((x) & 0xff00) << 8) | \
                                    (((x) & 0xff0000) >> 8) | (((x) & 0xff000000) >> 24))
#define ASF_INTEL_IANA            0x00000157
#define ASF_INTEL_CONVERTED_IANA  INTEL_IANA_SWAP32 (ASF_INTEL_IANA)  ///< 0X57010000, received from ME
///
/// Intel OEM Special Command
///
#define ASF_INTEL_OEM_CMD 0xC1

///
/// Intel OEM Parameters 16 bit OEM Parameter values
///
#define USE_SOL                 BIT0 ///< 0000 0000 0000 0001 - bit 0, use SOL on the next boot
//
// Intel OEM Command 16 bit special command parameter values
//
#define USE_STORAGE_REDIR       BIT0 ///< 0000 0000 0000 0001 - bit 0 Parameter 2 will be used to indicate the channel
#define ENFORCE_SECURE_BOOT     BIT1 ///< 0000 0000 0000 0010 - bit 1 Enforce secure boot over Storage Redirection
#define REFLASH_BIOS            BIT2 ///< 0000 0000 0000 0100 - bit 2
#define BIOS_SETUP              BIT3 ///< 0000 0000 0000 1000 - bit 3
#define BIOS_PAUSE              BIT4 ///< 0000 0000 0001 0000 - bit 4
#define USE_KVM                 BIT5 ///< 0000 0000 0010 0000 - bit 5
#define ENABLE_SECURE_ERASE     BIT6 ///< 0000 0000 0100 0000 - bit 6

#define STORAGE_REDIRECTION_BOOT_DEVICE_MASK   0x0300
#define STORAGE_REDIRECTION_BOOT_DEVICE_SHIFT  8

#endif
