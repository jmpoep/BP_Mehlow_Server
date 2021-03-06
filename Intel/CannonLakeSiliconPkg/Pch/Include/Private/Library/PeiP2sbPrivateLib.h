/** @file
  Header file for PeiP2sbPrivateLib.

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
#ifndef _PEI_P2SB_PRIVATE_LIB_H_
#define _PEI_P2SB_PRIVATE_LIB_H_

#include <Ppi/SiPolicy.h>

/**
  Get P2SB pci segment address.

  @retval P2SB pci segment base address
**/
UINT64
P2sbPciBase (
  VOID
  );

/**
  Lock SAI access from P2SB before any 3rd code execution.
**/
VOID
P2sbSaiLock (
  VOID
  );

/**
  Check SBREG readiness.

  @retval TRUE                SBREG is ready
  @retval FALSE               SBREG is not ready
**/
BOOLEAN
P2sbIsSbregReady (
  VOID
  );

/**
  Internal function performing HPET initin early PEI phase
**/
VOID
P2sbHpetInit (
  VOID
  );

/**
 Early init P2SB configuration
**/
VOID
P2sbEarlyConfig (
  VOID
  );

/**
  The function performs P2SB initialization.

  @param[in] SiPolicy         The SI Policy PPI instance
**/
VOID
P2sbConfigure (
  IN  SI_POLICY_PPI           *SiPolicy
  );

/**
  The function performs P2SB lock programming.

  @param[in] SiPolicy         The SI Policy PPI instance
**/
VOID
P2sbLock (
  IN  SI_POLICY_PPI           *SiPolicy
  );

#endif // _PEI_P2SB_PRIVATE_LIB_H_
