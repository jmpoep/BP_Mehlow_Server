/** @file
  This files contains Pch services for RCs usage

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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

#include "PchRcLibrary.h"
#include <Ppi/Spi.h>
#include <Private/Library/PmcPrivateLib.h>

/**
  The function is used while doing CPU Only Reset, where PCH may be required
  to initialize strap data before soft reset.

  @param[in] Operation                  Get/Set Cpu Strap Set Data
  @param[in, out] CpuStrapSet           Cpu Strap Set Data

  @retval EFI_SUCCESS                   The function completed successfully.
  @exception EFI_UNSUPPORTED            The function is not supported.
**/
EFI_STATUS
EFIAPI
PchCpuStrapSet (
  IN      CPU_STRAP_OPERATION           Operation,
  IN OUT  UINT16                        *CpuStrapSet
  )
{
  EFI_STATUS        Status;
  PCH_SPI_PPI       *SpiPpi;
  UINT8             SoftStrapValue[2];

  DEBUG ((DEBUG_INFO, "PchCpuStrapSet() - Start\n"));

  switch (Operation) {
  case GetCpuStrapSetData:
    ///
    /// Get CPU Strap Settings select. 0 = from descriptor, 1 = from PCH
    ///
    if (!PmcIsSetStrapMsgInterfaceEnabled ()) {
      ///
      /// Read Strap from Flash Descriptor
      ///
      Status = PeiServicesLocatePpi (
                 &gPchSpiPpiGuid,
                 0,
                 NULL,
                 (VOID **) &SpiPpi
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "PchCpuStrapSet is not available\n"));
        return EFI_UNSUPPORTED;
      }

      Status = SpiPpi->ReadCpuSoftStrap (SpiPpi, 0, sizeof (UINT16), SoftStrapValue);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "Error reading SPI CPU Soft Strap\n"));
        return EFI_UNSUPPORTED;
      }
      *CpuStrapSet = SoftStrapValue[0] + (SoftStrapValue[1] << 8);
      return EFI_SUCCESS;
    } else {
      ///
      /// Read Strap from PCH Soft Strap.
      ///
      *CpuStrapSet = PmcGetStrapMsgData ();
    }
    break;

  case SetCpuStrapSetData:
    ///
    /// PCH BIOS Spec Section 4.3 Soft Reset Control
    /// 2. If there are CPU configuration changes, program the strap setting into the
    ///    Soft Reset Data register located in PMC and follow the steps outlined in the
    ///    "CPU Only Reset BIOS Flow" section of the Processor
    ///    BIOS Writer's Guide
    ///
    PmcSetStrapMsgData (*CpuStrapSet);
    PmcLockSetStrapMsgInterface ();
    break;

  case LockCpuStrapSetData:
    PmcLockSetStrapMsgInterface ();
    break;

  default:
    break;
  }

  DEBUG ((DEBUG_INFO, "PchCpuStrapSet() - End\n"));

  return EFI_SUCCESS;
}


/**
  Set Early Power On Configuration setting for feature change.

  @param[in] Operation                  Get or set EPOC data
  @param[in, out] CpuEPOCSet            Cpu EPOC Data

  @retval EFI_SUCCESS                   The function completed successfully.
  @exception EFI_UNSUPPORTED            The function is not supported.
**/
EFI_STATUS
EFIAPI
PchCpuEpocSet (
  IN     CPU_EPOC_OPERATION       Operation,
  IN OUT  UINT32                    *CpuEpocSet
  )
{
  DEBUG ((DEBUG_INFO, "PchCpuEpocSet() - Start\n"));

  switch (Operation) {
    case GetCpuEpocData:
      *CpuEpocSet = PmcGetCpuEpoc ();
      if (*CpuEpocSet == 0xFFFFFFFF) {
        DEBUG ((DEBUG_ERROR, "Cpu Epoc returned invalid data.\n"));
        return EFI_UNSUPPORTED;
      }
      *CpuEpocSet = *CpuEpocSet & 0x3;
      break;
    case SetCpuEpocData:
      PmcSetCpuEpoc ((PmcGetCpuEpoc () & ~0x3) | (*CpuEpocSet & 0x3));
      break;
    default:
      break;
  }

  DEBUG ((DEBUG_INFO, "PchCpuEpocSet() - End\n"));
  return EFI_SUCCESS;
}
