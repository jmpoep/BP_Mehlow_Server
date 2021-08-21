/** @file
  PCH PCIE root port library.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchInfoLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchPcieRpLib.h>
#include <PcieRegs.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsClk.h>
#include <Register/PchRegsPcie.h>
#include <Register/PchRegsPcr.h>

GLOBAL_REMOVE_IF_UNREFERENCED CONST PCH_PCIE_CONTROLLER_INFO mPchPcieControllerInfo[] = {
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, PID_SPA,  0 },
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, PID_SPB,  4 },
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, PID_SPC,  8 },
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, PID_SPD, 12 },
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, PID_SPE, 16 }, // PCH-H only
  { PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, PID_SPF, 20 }  // PCH-H only
};
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT32 mPchPcieControllerInfoSize = sizeof (mPchPcieControllerInfo) / sizeof (mPchPcieControllerInfo[0]);

GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPchLpRstPcieStorageSupportedPort[] = {
  RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID,   // RP1..RP4
  RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,         // RP5..RP8
  RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2,         // RP9..RP12
  RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3          // RP13..RP16
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT8 mPchHRstPcieStorageSupportedPort[] = {
  RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID,   // RP1..RP4
  RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID,   // RP5..RP8
  RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,       RST_PCIE_STORAGE_CR_1,         // RP9..RP12
  RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID, RST_PCIE_STORAGE_CR_INVALID,   // RP13..RP16
  RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3,       RST_PCIE_STORAGE_CR_3,         // RP17..RP20
  RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2,       RST_PCIE_STORAGE_CR_2          // RP21..RP24
};

/**
  Get Pch Pcie Root Port Device and Function Number by Root Port physical Number

  @param[in]  RpNumber              Root port physical number. (0-based)
  @param[out] RpDev                 Return corresponding root port device number.
  @param[out] RpFun                 Return corresponding root port function number.

  @retval     EFI_SUCCESS           Root port device and function is retrieved
  @retval     EFI_INVALID_PARAMETER RpNumber is invalid
**/
EFI_STATUS
EFIAPI
GetPchPcieRpDevFun (
  IN  UINTN   RpNumber,
  OUT UINTN   *RpDev,
  OUT UINTN   *RpFun
  )
{
  UINTN       Index;
  UINTN       FuncIndex;
  UINT32      PciePcd;

  if (RpNumber >= GetPchMaxPciePortNum ()) {
    DEBUG ((DEBUG_ERROR, "GetPchPcieRpDevFun invalid RpNumber %x", RpNumber));
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }

  Index = RpNumber / PCH_PCIE_CONTROLLER_PORTS;
  FuncIndex = RpNumber - mPchPcieControllerInfo[Index].RpNumBase;
  *RpDev = mPchPcieControllerInfo[Index].DevNum;
  PciePcd = PchPcrRead32 (mPchPcieControllerInfo[Index].Pid, R_SPX_PCR_PCD);
  *RpFun = (PciePcd >> (FuncIndex * S_SPX_PCR_PCD_RP_FIELD)) & B_SPX_PCR_PCD_RP1FN;

  return EFI_SUCCESS;
}

/**
  Get Root Port physical Number by Pch Pcie Root Port Device and Function Number

  @param[in]  RpDev                 Root port device number.
  @param[in]  RpFun                 Root port function number.
  @param[out] RpNumber              Return corresponding physical Root Port index (0-based)

  @retval     EFI_SUCCESS           Physical root port is retrieved
**/
EFI_STATUS
EFIAPI
GetPchPcieRpNumber (
  IN  UINTN   RpDev,
  IN  UINTN   RpFun,
  OUT UINTN   *RpNumber
  )
{
  UINT64 RpBase;

  RpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, RpDev, RpFun, 0);
  *RpNumber = (PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_LCAP) >> N_PCH_PCIE_CFG_LCAP_PN) -1;
  return EFI_SUCCESS;
}

/**
  Gets pci segment base address of PCIe root port.

  @param RpIndex    Root Port Index (0 based)
  @return PCIe port base address.
**/
UINT64
PchPcieBase (
  IN  UINT32   RpIndex
  )
{
  UINTN       RpDevice;
  UINTN       RpFunction;

  GetPchPcieRpDevFun (RpIndex, &RpDevice, &RpFunction);

  return PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, (UINT32) RpDevice, (UINT32) RpFunction, 0);
}

/**
  Checks which CLK_REQ signal is assigned to given CLK_SRC

  @param ClkSrc    number of CLK_SRC, 0-based
  @retval          number of CLK_REQ, 0-based
**/
UINT8
GetClkReqForClkSrc (
  IN UINT8 ClkSrc
  )
{
  UINT32 ClkMuxRegister;
  UINT32 ClkReq;

  ASSERT (ClkSrc < GetPchMaxPcieClockNum ());
  if (ClkSrc < S_ICLK_PCR_MUXSEL_FIELDS_PER_REG) {
    ClkMuxRegister = PchPcrRead32 (PID_ICLK, R_ICLK_PCR_CMU_ONE_DWORD25);
    ClkReq = (ClkMuxRegister >> (ClkSrc * S_ICLK_PCR_MUXSEL_BITS_PER_FIELD)) & B_ICLK_PCR_MUXSEL_BITS_PER_FIELD;
  } else {
    ClkMuxRegister = PchPcrRead32 (PID_ICLK, R_ICLK_PCR_CMU_ONE_DWORD26);
    ClkReq = (ClkMuxRegister >> ((ClkSrc - S_ICLK_PCR_MUXSEL_FIELDS_PER_REG) * S_ICLK_PCR_MUXSEL_BITS_PER_FIELD)) & B_ICLK_PCR_MUXSEL_BITS_PER_FIELD;
  }
  DEBUG ((DEBUG_INFO, "GetClkReqForClkSrc %d->%d, register %08x\n", ClkSrc, ClkReq, ClkMuxRegister));
  return (UINT8)ClkReq;
}

/**
  Determines whether L0s is supported on current stepping.

  @return TRUE if L0s is supported, FALSE otherwise
**/
BOOLEAN
PchIsPcieL0sSupported (
  VOID
  )
{
  return TRUE;
}

/**
  Some early PCH steppings require Native ASPM to be disabled due to hardware issues:
   - RxL0s exit causes recovery
   - Disabling PCIe L0s capability disables L1
  Use this function to determine affected steppings.

  @return TRUE if Native ASPM is supported, FALSE otherwise
**/
BOOLEAN
PchIsPcieNativeAspmSupported (
  VOID
  )
{
  return PchIsPcieL0sSupported ();
}

/**
  Check the RST PCIe Storage Cycle Router number according to the root port number and PCH type

  @param[in] RootPortNum  Root Port Number

  @return  The RST PCIe Storage Cycle Router Number
**/
UINT8
RstGetCycleRouterNumber (
  IN  UINT32                   RootPortNum
  )
{
  if (IsPchLp ()) {
    if (RootPortNum < ARRAY_SIZE (mPchLpRstPcieStorageSupportedPort)) {
      return mPchLpRstPcieStorageSupportedPort[RootPortNum];
    }
  } else if (IsPchH ()) {
    if (RootPortNum < ARRAY_SIZE (mPchHRstPcieStorageSupportedPort)) {
      return mPchHRstPcieStorageSupportedPort[RootPortNum];
    }
  }
  return RST_PCIE_STORAGE_CR_INVALID;
}

