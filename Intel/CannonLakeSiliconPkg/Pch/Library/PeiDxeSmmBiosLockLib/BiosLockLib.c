/** @file
  Bios Lock library.

  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

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
#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/S3BootScriptLib.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsLpc.h>
#include <Register/PchRegsSpi.h>

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
  )
{
  UINT64  LpcBaseAddress;
  UINT64  SpiBaseAddress;

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

  ///
  /// PCH BIOS Spec Flash Security Recommendation
  ///
  /// BIOS needs to enable the BIOS Lock Enable (BLE) feature of the PCH by setting
  /// SPI/eSPI/LPC PCI offset DCh[1] = 1b.
  /// When this bit is set, attempts to write the Write Protect Disable (WPD) bit
  /// in PCH will cause a SMI which will allow the BIOS to verify that the write is
  /// from a valid source.
  /// Remember that BIOS needs to set SPI/LPC/eSPI PCI Offset DC [0] = 0b to enable
  /// BIOS region protection before exiting the SMI handler.
  /// Also, TCO_EN bit needs to be set (SMI_EN Register, ABASE + 30h[13] = 1b) to keep
  /// BLE feature enabled after booting to the OS.
  /// Intel requires that BIOS enables the Lock Enable (LE) feature of the PCH to
  /// ensure SMM protection of flash.
  /// RC installs a default SMI handler that clears WPD.
  /// There could be additional SMI handler to log such attempt if desired.
  ///
  /// BIOS needs to enable the "Enable in SMM.STS" (EISS) feature of the PCH by setting
  /// SPI PCI offset DCh[5] = 1b for SPI or setting eSPI PCI offset DCh[5] = 1b for eSPI.
  /// When this bit is set, the BIOS region is not writable until SMM sets the InSMM.STS bit,
  /// to ensure BIOS can only be modified from SMM. Please refer to CPU BWG for more details
  /// on InSMM.STS bit.
  /// Intel requires that BIOS enables the Lock Enable (LE) feature of the PCH to ensure
  /// SMM protection of flash.
  /// SPI PCI offset DCh[1] = 1b for SPI or setting eSPI PCI offset DCh[1] = 1b for eSPI.
  /// When this bit is set, EISS is locked down.
  ///
  PciSegmentOr8 (SpiBaseAddress + R_SPI_CFG_BC, B_SPI_CFG_BC_EISS | B_SPI_CFG_BC_LE);
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint8,
    PcdGet64 (PcdPciExpressBaseAddress) + SpiBaseAddress + R_SPI_CFG_BC,
    1,
    (VOID *) (UINTN) (PcdGet64 (PcdPciExpressBaseAddress) + SpiBaseAddress + R_SPI_CFG_BC)
    );
  PciSegmentOr8 (LpcBaseAddress + R_LPC_CFG_BC, B_LPC_CFG_BC_EISS | B_LPC_CFG_BC_LE);
  S3BootScriptSaveMemWrite (
    S3BootScriptWidthUint8,
    PcdGet64 (PcdPciExpressBaseAddress) + LpcBaseAddress + R_LPC_CFG_BC,
    1,
    (VOID *) (UINTN) (PcdGet64 (PcdPciExpressBaseAddress) + LpcBaseAddress + R_LPC_CFG_BC)
    );
}
