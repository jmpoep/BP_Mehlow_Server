/** @file
  This code supports a the private implementation
  of the Legacy Interrupt protocol.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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
#include <Protocol/LegacyInterrupt.h>
#include <PiDxe.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PchPcrLib.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsLpc.h>
#include <Register/PchRegsItss.h>
#include <Private/PchConfigHob.h>

#define MAX_PIRQ_NUMBER 8

//
// Handle for the Legacy Interrupt Protocol instance produced by this driver
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_HANDLE                    mLegacyInterruptHandle = NULL;

//
// Module Global:
//  Since this driver will only ever produce one instance of the Private Data
//  protocol you are not required to dynamically allocate the PrivateData.
//

/**
  Return the number of PIRQs supported by this chipset.

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[out] NumberPirqs         The pointer which point to the max IRQ number supported by this PCH.

  @retval EFI_SUCCESS             Legacy BIOS protocol installed
**/
EFI_STATUS
EFIAPI
GetNumberPirqs (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *NumberPirqs
  )
{
  *NumberPirqs = MAX_PIRQ_NUMBER;

  return EFI_SUCCESS;
}

/**
  Return PCI location of this device. $PIR table requires this info.

  @param[in] This                 Protocol instance pointer.
  @param[out] Bus                 PCI Bus
  @param[out] Device              PCI Device
  @param[out] Function            PCI Function

  @retval EFI_SUCCESS             Bus/Device/Function returned
**/
EFI_STATUS
EFIAPI
GetLocation (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *Bus,
  OUT UINT8                          *Device,
  OUT UINT8                          *Function
  )
{
  *Bus      = DEFAULT_PCI_BUS_NUMBER_PCH;
  *Device   = PCI_DEVICE_NUMBER_PCH_LPC;
  *Function = PCI_FUNCTION_NUMBER_PCH_LPC;

  return EFI_SUCCESS;
}

/**
  Read the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[out] PirqData            Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number
  @retval EFI_DEVICE_ERROR        Operation was unsuccessful
**/
EFI_STATUS
EFIAPI
ReadPirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  OUT UINT8                          *PirqData
  )
{
  EFI_PEI_HOB_POINTERS   HobPtr;
  static PCH_CONFIG_HOB  *PchConfigHob = NULL;

  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

  *PirqData = PchPcrRead8 (PID_ITSS, R_ITSS_PCR_PIRQA_ROUT + PirqNumber) & 0x7F;

  if (*PirqData == 0x7F) {
    if (PchConfigHob == NULL) {
      HobPtr.Guid  = GetFirstGuidHob (&gPchConfigHobGuid);
      if (HobPtr.Guid == NULL) {
        ASSERT (FALSE);
        return EFI_DEVICE_ERROR;
      }
      PchConfigHob = (PCH_CONFIG_HOB *) GET_GUID_HOB_DATA (HobPtr.Guid);
    }

    if (PirqNumber < ARRAY_SIZE (PchConfigHob->Interrupt.PxRcConfig)) {
      *PirqData = PchConfigHob->Interrupt.PxRcConfig[PirqNumber];
      return EFI_SUCCESS;
    }

    DEBUG ((DEBUG_ERROR, "ReadPirq ERROR, PirqData = 0x%x\n", *PirqData));
    return EFI_INVALID_PARAMETER;
  }

  return EFI_SUCCESS;
}

/**
  Write the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[in] PirqData             Value to write

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number
  @retval EFI_DEVICE_ERROR        Operation was unsuccessful
**/
EFI_STATUS
EFIAPI
WritePirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  IN  UINT8                          PirqData
  )
{
  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }
  PchPcrWrite8 (PID_ITSS, R_ITSS_PCR_PIRQA_ROUT + PirqNumber, PirqData);

  if ((PchPcrRead8 (PID_ITSS, R_ITSS_PCR_PIRQA_ROUT + PirqNumber) & 0x7F) != PirqData) {
    DEBUG ((DEBUG_ERROR, "WritePirq ERROR\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

//
// The Legacy Interrupt Protocol instance produced by this driver
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_LEGACY_INTERRUPT_PROTOCOL mLegacyInterrupt = {
  GetNumberPirqs,
  GetLocation,
  ReadPirq,
  WritePirq
};

/**
  <b>LegacyInterrupt DXE Module Entry Point</b>\n
  - <b>Introduction</b>\n
    The LegacyInterrupt module is a DXE driver which provides a standard way for
    other drivers to get/set the PIRQ/legacy IRQ mappings.

  - @pre
    PCH PCR base address configured

  - @result
    The LegacyInterrupt driver produces EFI_LEGACY_INTERRUPT_PROTOCOL which is
    documented in the Compatibility Support Module Specification.

  @param[in] ImageHandle          Handle for this drivers loaded image protocol.
  @param[in] SystemTable          EFI system table.

  @retval EFI_SUCCESS             Legacy Interrupt protocol installed
  @retval Other                   No protocol installed, unload driver.
**/
EFI_STATUS
EFIAPI
LegacyInterruptInstall (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS  Status;

  DEBUG ((DEBUG_INFO, "LegacyInterruptInstall() Start\n"));

  //
  // Make sure the Legacy Interrupt Protocol is not already installed in the system
  //
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiLegacyInterruptProtocolGuid);

  //
  // Make a new handle and install the protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mLegacyInterruptHandle,
                  &gEfiLegacyInterruptProtocolGuid,
                  &mLegacyInterrupt,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "LegacyInterruptInstall() End\n"));
  return Status;
}
