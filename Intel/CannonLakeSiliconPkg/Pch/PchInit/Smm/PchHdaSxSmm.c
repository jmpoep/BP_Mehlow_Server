/** @file
  PCH HD Audio Sx handler implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

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

@par Specification
**/

#include "PchInitSmm.h"
#include <Register/PchRegs.h>
#include <Register/PchRegsHda.h>

GLOBAL_REMOVE_IF_UNREFERENCED UINT64  mPchHdaPciBase;
GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN mPchHdaCodecSxWakeCapability = FALSE;

/**
  Updates Codec Sx Wake Capability setting: disabled/enabled
**/
VOID
UpdateHdaCodecSxWakeCapabilitySetting (
  VOID
  )
{
  mPchHdaPciBase = PCI_SEGMENT_LIB_ADDRESS (
                    DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                    DEFAULT_PCI_BUS_NUMBER_PCH,
                    PCI_DEVICE_NUMBER_PCH_HDA,
                    PCI_FUNCTION_NUMBER_PCH_HDA,
                    0
                    );

  if ((mPchConfigHob->HdAudio.CodecSxWakeCapability == FALSE) ||
      (PciSegmentRead16 (mPchHdaPciBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF)) {
    mPchHdaCodecSxWakeCapability = FALSE;
  } else {
    mPchHdaCodecSxWakeCapability = TRUE;
  }
}

/**
  HDA Controller Configurations Prior to Entering Sx
**/
VOID
PchHdaSxCallback (
  VOID
  )
{
  if (mPchHdaCodecSxWakeCapability == FALSE) {
    return;
  }

  PciSegmentOr32 (mPchHdaPciBase + R_HDA_CFG_PGCTL, (UINT32) B_HDA_CFG_PGCTL_CTLPGD);
}
