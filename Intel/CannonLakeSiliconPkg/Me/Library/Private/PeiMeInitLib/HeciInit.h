/** @file
  Framework PEIM to provide Heci.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2008 - 2016 Intel Corporation.

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
#ifndef _HECI_INIT_H_
#define _HECI_INIT_H_

#include <PiPei.h>
///
/// Driver Consumed PPI Prototypes
///
#include <Ppi/MemoryDiscovered.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/PeiServicesLib.h>
#include <Ppi/MemoryDiscovered.h>

#include <IndustryStandard/Pci22.h>
#include <Library/PciSegmentLib.h>
#include <Library/MeTypeLib.h>
#include <HeciRegs.h>
#include <MeChipset.h>
#include <HeciRegs.h>
#include <MeState.h>
#include <CoreBiosMsg.h>
#include <PchAccess.h>
#include <Library/PmcLib.h>
#include <Library/TimerLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>
#include <Library/MemoryAllocationLib.h>

//
// Driver Consumed PPI Prototypes
//
#include <Ppi/HeciPpi.h>
#include <Ppi/SiPolicy.h>

//
// Header for common HECI driver prototypes
//
#include <Private/Library/HeciInitLib.h>
#include <Library/MeChipsetLib.h>
#include <Library/PeiMeLib.h>

//
// Prototypes
//

/**
  Internal function performing Heci PPIs init needed in PEI phase

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_DEVICE_ERROR        ME FPT is bad
**/
EFI_STATUS
InstallHeciPpi (
  VOID
  );

/**
  Internal function performing PM register initialization for Me
**/
VOID
MePmInit (
  VOID
  );

#endif // _HECI_INIT_H_
