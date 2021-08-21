/** @file
  This code supports a private implementation of the Legacy Region protocol.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#ifndef _LEGACY_REGION_H_
#define _LEGACY_REGION_H_

#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Protocol/LegacyRegion2.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/Cpu.h>
#include <IndustryStandard/Pci22.h>
#include <SaAccess.h>


#define LEGACY_REGION_INSTANCE_SIGNATURE  SIGNATURE_32 ('R', 'E', 'G', 'N')
#define LEGACY_REGION_DECODE_ROM  3


///
/// PAM registers granularity is 16 KB
///
#define PAM_GRANULARITY     0x4000
#define PAM_UNLOCK          0x0000
#define PAM_LOCK            0x0001
#define PAM_BOOTLOCK        0x0002

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      Handle;
  EFI_LEGACY_REGION2_PROTOCOL     LegacyRegion;
  EFI_HANDLE                      ImageHandle;

  ///
  /// Protocol for PAM register access
  ///
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo;
} LEGACY_REGION_INSTANCE;

#define LEGACY_REGION_INSTANCE_FROM_THIS  (this) CR (this, \
                                                     LEGACY_REGION_INSTANCE, \
                                                     LegacyRegion, \
                                                     LEGACY_REGION_INSTANCE_SIGNATURE \
        )

/**
  Install Driver to produce Legacy Region protocol.

  @param[in] ImageHandle             Handle for the image of this driver

  @retval EFI_SUCCESS - Legacy Region protocol installed
  @retval Other       - No protocol installed, unload driver.
**/
EFI_STATUS
LegacyRegionInstall (
  IN EFI_HANDLE           ImageHandle
  );
#endif
