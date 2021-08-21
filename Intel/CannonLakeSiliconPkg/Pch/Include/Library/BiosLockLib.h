/** @file
  Header file for BiosLockLib.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 Intel Corporation.

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
#ifndef _BIOSLOCK_LIB_H_
#define _BIOSLOCK_LIB_H_

/**
  Enable BIOS lock. This will set the LE (Lock Enable) and EISS (Enable In SMM.STS).
  When this is set, attempts to write the WPD (Write Protect Disable) bit in PCH
  will cause a SMI which will allow the BIOS to verify that the write is from a valid source.

  Bios should always enable LockDownConfig.BiosLock policy to set Bios Lock bit in FRC.
  If capsule udpate is enabled, it's expected to not do BiosLock by setting BiosLock policy disable
  so it can udpate BIOS region.
  After flash update, it should utilize this lib to do BiosLock for security.
**/
VOID
BiosLockEnable (
  VOID
  );

#endif // _BIOSLOCK_LIB_H_
