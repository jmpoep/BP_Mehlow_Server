/** @file
  Initializes PCH Security programming in PEI

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

#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciSegmentLib.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsLpc.h>
#include <Register/PchRegsSpi.h>

/**
  Clear EISS (Enable InSMM.STS).
  The usage to clear EISS is to support set variable in PEI phase.

  NOTE: This does not work if LE bit is already set.
**/
VOID
LpcSpiClearEiss (
  VOID
  )
{
  UINT64                  LpcBaseAddress;
  UINT64                  SpiBaseAddress;

  LpcBaseAddress = PCI_SEGMENT_LIB_ADDRESS (
                     DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_LPC,
                     PCI_FUNCTION_NUMBER_PCH_LPC,
                     0
                     );
  SpiBaseAddress = PCI_SEGMENT_LIB_ADDRESS (
                     DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                     DEFAULT_PCI_BUS_NUMBER_PCH,
                     PCI_DEVICE_NUMBER_PCH_SPI,
                     PCI_FUNCTION_NUMBER_PCH_SPI,
                     0
                     );

  ASSERT ((PciSegmentRead8 (SpiBaseAddress + R_SPI_CFG_BC) & B_SPI_CFG_BC_LE) == 0);
  ASSERT ((PciSegmentRead8 (LpcBaseAddress + R_LPC_CFG_BC) & B_LPC_CFG_BC_LE) == 0);

  //
  // Since the EISS HW default is 1, need to clear it when disabled in policy
  //
  PciSegmentAnd8 (SpiBaseAddress + R_SPI_CFG_BC, (UINT8) ~(B_SPI_CFG_BC_EISS));
  PciSegmentAnd8 (LpcBaseAddress + R_LPC_CFG_BC, (UINT8) ~(B_LPC_CFG_BC_EISS));
}
