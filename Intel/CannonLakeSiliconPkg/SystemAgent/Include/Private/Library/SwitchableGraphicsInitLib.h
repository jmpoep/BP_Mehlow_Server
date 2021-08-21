/** @file
  Header file for the SwitchableGraphics Pei driver.

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
#ifndef _SWITCHABLE_GRAPHICS_PEI_H_
#define _SWITCHABLE_GRAPHICS_PEI_H_

#include <SaPolicyCommon.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef HIGH
#define HIGH  1
#endif

#ifndef LOW
#define LOW 0
#endif

#define SG_DELAY_HOLD_RST    100 * STALL_ONE_MILLI_SECOND
#define SG_DELAY_PWR_ENABLE  300 * STALL_ONE_MILLI_SECOND

/**
  Initialize the SwitchableGraphics support (PEI).

  @param[in] PCIE_PEI_PREMEM_CONFIG      *PciePeiPreMemConfig to access the PCIe related settings
  @param[in] SA_MISC_PEI_PREMEM_CONFIG   *MiscPeiPreMemConfig to access the Misc settings
**/
VOID
SwitchableGraphicsInit (
  IN   SWITCHABLE_GRAPHICS_CONFIG        *SgGpioData,
  IN   SA_MISC_PEI_PREMEM_CONFIG         *MiscPeiPreMemConfig
  );
#endif
