/** @file
  Additional programming steps for disabling PEG controller.

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

@par Specification Reference:
**/
#include "SaPegLowLevel.h"
#include <Private/Library/SaPcieInitLib.h>
#include <Library/CpuPlatformLib.h>

/**
Program flow control credits for configurations where PEG is disabled or x16x0x0.
**/
VOID
FlowControlCreditProgrammingNoPegLib (
  VOID
  )
{
}
/**
  Disable Unused PEG Controllers

  @param[in]  This                        - Low level function table
  @param[in]  PegDisableMask              - Bitmap of controllers to disable by function number
**/
VOID
EFIAPI
SklDisableUnusedPcieControllers (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDisableMask,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  UINTN                           McBaseAddress;
  UINTN                           Peg0BaseAddress;
  UINT16                          LoopCount;
  UINT32                          AndData32;
  CPU_SKU                         CpuSku;
  CPU_GENERATION                  CpuGeneration;
  BOOLEAN                         DisableLinkFunc0;
  BOOLEAN                         FunctionExists;
  BOOLEAN                         PowerGatingSupported;
  BOOLEAN                         DisablePegPort;
  UINT32                          StateTracker;

  PciePrivate      = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;
  DisableLinkFunc0 = FALSE;
  McBaseAddress    = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
  Peg0BaseAddress  = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  CpuSku           = GetCpuSku ();
  CpuGeneration    = GetCpuGeneration ();
  DisablePegPort   = FALSE;
  ///
  /// PEG10 must be enabled if PEG11 and/or PEG12 are enabled
  ///
  if (((PegDisableMask & B_PEG1_DISABLE_MASK) == 0) || ((PegDisableMask & B_PEG2_DISABLE_MASK) == 0)) {
    if ((PegDisableMask & B_PEG0_DISABLE_MASK) != 0) {
      DisableLinkFunc0 = TRUE;
    }
    PegDisableMask &= (UINT8) ~(B_PEG0_DISABLE_MASK);
  }

  if (PciSegmentRead16 (PciePort->ConfigSpaceBase + R_SA_PEG_VID_OFFSET) == 0xFFFF) {
    FunctionExists = FALSE;
    DEBUG ((DEBUG_WARN, "PCIe RP (0:%x:%x) Disabled.\n", PciePort->Device, PciePort->Function));
  } else {
    FunctionExists = TRUE;
  }

  if ((PciePort->Device == SA_PEG3_DEV_NUM)
     && (PegDisableMask & B_PEG3_DISABLE_MASK)
     && FunctionExists
     )
  {
    DisablePegPort = TRUE;
  }
  if ((PciePort->Device == SA_PEG_DEV_NUM)
     && ((((PegDisableMask >> (PciePort->Function)) & 0x1) != 0)
     || ((PciePort->Function == 0) && DisableLinkFunc0))
     && FunctionExists == TRUE
     )
  {
    DisablePegPort = TRUE;
  }

  if (DisablePegPort == TRUE) {
    if ((CpuGeneration == EnumCnlCpu) && (GetCpuStepping () == EnumCnlP0)) {
      ///
      ///
      PciSegmentAnd32 (PciePort->ConfigSpaceBase + R_SA_PEG_AFEOVR_OFFSET, (UINT32) ~(BIT18 | BIT19));
      PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGCOMLCGCTRL_OFFSET, BIT30);
      PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_OFFSET_F0, BIT3);
      MicroSecondDelay (10000 * STALL_ONE_MICRO_SECOND);
      PciSegmentAnd32 (PciePort->ConfigSpaceBase + R_SA_PEG_OFFSET_F0, (UINT32) ~(BIT3));
    }
    ///
    /// Set PEG Controller.R D20h [30] = 1
    ///
    PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGCOMLCGCTRL_OFFSET, BIT30);
    ///
    /// Poll until PEG Controller.R D20h [31] = 1
    ///
    StateTracker = 0;
    for (LoopCount = 0; LoopCount < (100 * 10); LoopCount++) {
      StateTracker = PciSegmentRead32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGCOMLCGCTRL_OFFSET) & BIT31;
      if (StateTracker == BIT31) {
        break;
      }
      MicroSecondDelay (STALL_ONE_MICRO_SECOND * 2); //2usec
    }
    ///
    /// Check if successful, if not then abort flow for this controller
    ///
    if (StateTracker != BIT31) {
      PegDisableMask |= (1 << PciePort->Function);
      if (PciePort->Device == SA_PEG3_DEV_NUM) {
        PegDisableMask |= B_PEG3_DISABLE_MASK;
      } else {
        PegDisableMask |= (B_PEG0_DISABLE_MASK << PciePort->Function);
      }
      DEBUG ((DEBUG_WARN, "PCIe RP (0:%x:%x) PEGCOMLCGCTRL operation Timeout.\n", PciePort->Device, PciePort->Function));
      return;
    }
    ///
    /// DisableLink. Set PEG Controller.R 0B0h [4] (LD (Link Disable) bit in Link Control Register
    ///
    PciSegmentOr8 (PciePort->ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, BIT4);

    if (CpuGeneration == EnumCnlCpu) {
      ///
      /// Poll until PEG Controller.R 464h = 0 or 2
      ///
      StateTracker = 0;
      for (LoopCount = 0; LoopCount < (100 * 10); LoopCount++) {
        if (PciePort->Device == SA_PEG3_DEV_NUM) {
          StateTracker = (PciSegmentRead32 (PciePort->ConfigSpaceBase + R_SA_PEG_REUT_PH1_PIS_OFFSET)) & 0x3F;
        } else {
          StateTracker = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_REUT_PH1_PIS_OFFSET) >> (8 * (PciePort->Function))) & 0x3F;
        }
        if (StateTracker == 0x0 || StateTracker == 0x2) {
          break;
        }
        MicroSecondDelay (STALL_ONE_MICRO_SECOND * 2); //2usec
      }
      ///
      /// Check if successful, if not then abort flow for this controller
      ///
      if (StateTracker != 0x0 && StateTracker != 0x2) {
        if (PciePort->Device == SA_PEG3_DEV_NUM) {
          PegDisableMask |= B_PEG3_DISABLE_MASK;
        } else {
          PegDisableMask |= (B_PEG0_DISABLE_MASK << PciePort->Function);
        }
        DEBUG ((DEBUG_WARN, "PCIe RP (0:1:%x) Timeout disabling link.\n", PciePort->Function));
        return;
      }
    }
    ///
    /// Program AFEOVR.RXSQDETOVR
    /// PCIe link disable for Switchable GFx
    /// Additional Power savings: Set PEG Controller 0xC20 BIT4 = 1 & BIT5 = 1
    ///
    PciSegmentOr8 (PciePort->ConfigSpaceBase + R_SA_PEG_AFEOVR_OFFSET, (UINT8) (BIT5 | BIT4));
    ///
    /// Power Down All Lanes
    ///
    PciePrivate->PowerDownAllLanes (This, PciePort->Device, PciePort->Function);
    ///
    /// Disable Controller
    ///
    if (CpuGeneration == EnumCnlCpu) {
      ///
      /// Set PEG Controller.R D10h [0] = 1
      ///
      PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_DEBUP2_OFFSET, BIT0);
    }
    if (PciePort->Device == SA_PEG3_DEV_NUM) {
      if (PegDisableMask & B_PEG3_DISABLE_MASK) {
        ///
        /// Clear D0.F0.R 054h (DEVEN) enable bit for PEG3.
        ///
        AndData32 = (UINT32) ~(BIT13);
        PciSegmentAnd32 (McBaseAddress + R_SA_DEVEN, AndData32);
        DEBUG ((DEBUG_WARN, "PCIe RP (0:%x:%x) Disabled.\n", PciePort->Device, PciePort->Function));
      }
    } else {
        if (((PegDisableMask >> PciePort->Function) & 0x1) != 0) {
        ///
        /// Clear D0.F0.R 054h (DEVEN) enable bit for PEG0/1/2.
        ///
        AndData32 = (UINT32) ~(BIT3 >> PciePort->Function);
        PciSegmentAnd32 (McBaseAddress + R_SA_DEVEN, AndData32);
        DEBUG ((DEBUG_WARN, "PCIe RP (0:%x:%x) Disabled.\n", PciePort->Device, PciePort->Function));
      }
    }
  }
  ///
  /// If all PEG Controllers are disabled, disable PEG
  ///
  if ((PegDisableMask & V_PEG_DISABLE_MASK) == V_PEG_DISABLE_MASK) {
    PowerGatingSupported = FALSE;
    if ((CpuSku  == EnumCpuHalo) || (CpuSku  == EnumCpuTrad)) {
      PowerGatingSupported = TRUE;
    }

    if (PowerGatingSupported) {
      DEBUG ((DEBUG_INFO, "No PEG Root Ports active, Power Gating PEG\n"));
      MmioOr32 (((UINTN) PciePrivate->MchBar) + R_SA_MCHBAR_BIOS_RESET_CPL_OFFSET, BIT3);
    } else {
      DEBUG ((DEBUG_WARN, "PEG Power Gate not supported on this stepping\n"));
    }
  }
}

/**
  This function gets the private data for the SA PCIe low level functions

  @param[in]  PCIE_PEI_PREMEM_CONFIG     - PciePeiPreMemConfig
  @param[in]  SA_MISC_PEI_PREMEM_CONFIG  - MiscPeiPreMemConfig
  @param[out] SaPciePrivateData          - Table of function calls for SA PEG

  @retval EFI_SUCCESS - Table of function calls returned successfully
**/
EFI_STATUS
GetSaPciePrivateData (
  IN  PCIE_PEI_PREMEM_CONFIG                  *PciePeiPreMemConfig,
  IN  SA_MISC_PEI_PREMEM_CONFIG               *MiscPeiPreMemConfig,
  OUT SA_PCIE_PRIVATE_FUNCTION_CALLS          *SaPciePrivateData
  )
{
  CPU_SKU  CpuSku;

  CpuSku = GetCpuSku ();

  if ((CpuSku == EnumCpuHalo) || (CpuSku == EnumCpuTrad)) {
    return GetSklPegPrivateData (
             PciePeiPreMemConfig,
             MiscPeiPreMemConfig,
             SaPciePrivateData
             );
  }
  return EFI_UNSUPPORTED;
}
