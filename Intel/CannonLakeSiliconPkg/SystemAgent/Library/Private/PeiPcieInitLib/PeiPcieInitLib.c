/** @file
  PCIe Initialization Library High Level Generic Functions

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2016 Intel Corporation.

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

#include <Private/Library/PcieInitLib.h>

/**
  Waits for the Data Link Layer on all given root ports to reach the DL_Active state.
  The user passes a fail mask that indicates which root ports to check.  The function
  will update the fail mask to indicate which root ports successfully trained.

  The fail mask is a bitmap based on PciePorts array indices.  The array must be
  of length 8 or greater since the PciePorts array can have at most 256 entries.

  @param[in]      PcieAccess              - Low level function table
  @param[in]      PciePorts               - PCIe Root Ports to wait for
  @param[in]      PciePortsLength         - Length of the PciePorts array
  @param[in]      ForceCheck              - TRUE to ignore current FailMask and check all root ports
  @param[in, out] FailMask                - Bitmap of root ports to check.  Returns bitmap indicating which root ports
                                            failed to reach DL_Active.  Array must be of length 8 or greater!
**/
VOID
EFIAPI
WaitForDataLinkLayerLinkActiveOnAllPorts (
  IN      PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN      PCIE_PORT_INFO                    *PciePorts,
  IN      UINT8                             PciePortsLength,
  IN      BOOLEAN                           ForceCheck,
  IN OUT  UINT32                            *FailMask///[PCIE_ROOT_PORT_BITMAP_LENGTH];
  )
{
  UINT32  CompleteMask[PCIE_ROOT_PORT_BITMAP_LENGTH];
  UINT8   Index;
  UINT8   Stalls;
  BOOLEAN AllLinksDone;

  for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
    CompleteMask[Index] = 0x0;
  }
  ///
  /// If force check is true, initialize bitmap to check all ports
  ///
  if (ForceCheck) {
    for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
      FailMask[Index] = 0xFFFFFFFF;
    }
    for (Index = 0; Index < PciePortsLength; Index++) {
      if (PciePorts[Index].Device == SA_PEG0_DEV_NUM) {
        FailMask[Index / 32] &= (UINT32) ~(0x1 << (PciePorts[Index].Function));
      }
      if (PciePorts[Index].Device == SA_PEG3_DEV_NUM) {
        FailMask[Index / 32] &= (UINT32) ~(B_PCIE_PEG3_BITMAP);
      }
    }
  }

  ///
  /// Wait for VC0 Negotiation to complete on all given root ports
  ///
  for (Stalls = 0; Stalls < 100; Stalls++) {
    for (Index = 0; Index < PciePortsLength; Index++) {
      ///
      /// Test if current port is neither in fail mask nor previously tested good.
      ///
      if ((PciePorts[Index].Device == SA_PEG0_DEV_NUM) &&
          ((FailMask[Index / 32] & (0x1 << (PciePorts[Index].Function))) |
           (CompleteMask[Index / 32] & (0x1 << (PciePorts[Index].Function)))) == 0) {
        DEBUG ((DEBUG_INFO, " PCIe RP (%x:%x:%x) - checking\n", PciePorts[Index].Bus, PciePorts[Index].Device, PciePorts[Index].Function));
        ///
        /// Check if VC0 negotiation is complete
        ///
        if (PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[Index]))) {
          ///
          /// Record as a good link
          ///
          DEBUG ((DEBUG_INFO, "  VC negotiation is complete\n"));
          CompleteMask[Index / 32] |= (0x1 << (PciePorts[Index].Function));
        }
      }
      if ((PciePorts[Index].Device == SA_PEG3_DEV_NUM) &&
          ((FailMask[Index / 32] & (B_PCIE_PEG3_BITMAP)) |
           (CompleteMask[Index / 32] & (B_PCIE_PEG3_BITMAP))) == 0) {
        DEBUG ((DEBUG_INFO, " PCIe RP (%x:%x:%x) - checking\n", PciePorts[Index].Bus, PciePorts[Index].Device, PciePorts[Index].Function));
        ///
        /// Check if VC0 negotiation is complete
        ///
        if (PcieAccess->DataLinkLayerLinkActive (PcieAccess, &(PciePorts[Index]))) {
          ///
          /// Record as a good link
          ///
          DEBUG ((DEBUG_INFO, "  VC negotiation is complete\n"));
          CompleteMask[Index / 32] |= (B_PCIE_PEG3_BITMAP);
        }
      }
    }
    ///
    /// If all links accounted for then exit
    ///
    AllLinksDone = TRUE;
    for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
      if ((FailMask[Index] | CompleteMask[Index] ) != 0xFFFFFFFF) {
        AllLinksDone = FALSE;
        break;
      }
    }
    if (AllLinksDone) {
      break;
    }
    MicroSecondDelay (STALL_ONE_MILLI_SECOND);
  }
  DEBUG ((DEBUG_INFO, "Total Stall: %d msec\n", (UINT32) Stalls));
  ///
  /// Update Fail Mask
  ///
  for (Index = 0; Index < PCIE_ROOT_PORT_BITMAP_LENGTH; Index++) {
    FailMask[Index] = (UINT32) ~CompleteMask[Index];
  }
}

/**
  This function prints the time required for DL_Active to be set. Quits after 100 msec.

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
**/
VOID
WaitForDataLinkLayerLinkActive (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT32  MsecWait;
  BOOLEAN DllActive;

  MsecWait  = 0;
  DllActive = PcieAccess->DataLinkLayerLinkActive (PcieAccess, PciePort);

  while (!(DllActive) && (MsecWait < 100)) {
    MsecWait++;
    MicroSecondDelay (STALL_ONE_MILLI_SECOND);
    DllActive = PcieAccess->DataLinkLayerLinkActive (PcieAccess, PciePort);
  }
  DEBUG ((DEBUG_INFO, "PCIe RP (%x:%x:%x) - DL_Active = %x after %d msec.\n",
          (UINTN) PciePort->Bus, (UINTN) PciePort->Device, (UINTN) PciePort->Function, (UINTN) DllActive, MsecWait));
}

/**
  Computes the Pre-Cursor, Cursor, and Post-Cursor from a preset

  @param[in]  Preset                      - Preset to compute coefficients for
  @param[in]  FullSwing                   - The full swing of the transmitter
  @param[out] PreCursor                   - Computed Pre-Cursor
  @param[out] Cursor                      - Computed Cursor
  @param[out] PostCursor                  - Computed Post-Cursor
**/
VOID
GetCoefficientsFromPreset (
  IN  UINT8                             Preset,
  IN  UINT8                             FullSwing,
  OUT UINT8                             *PreCursor,
  OUT UINT8                             *Cursor,
  OUT UINT8                             *PostCursor
  )
{
  INT32   PreCursorMilli;
  INT32   PostCursorMilli;

  PreCursorMilli  = 0;
  PostCursorMilli = 0;

  ///
  /// Get starting values from Table 4-16 of the PCIe Base Spec v3.0
  ///
  switch (Preset) {
    case  0:
      PreCursorMilli  =    0;
      PostCursorMilli = -250;
      break;

    case  1:
      PreCursorMilli  =    0;
      PostCursorMilli = -167;
      break;

    case  2:
      PreCursorMilli  =    0;
      PostCursorMilli = -200;
      break;

    case  3:
      PreCursorMilli  =    0;
      PostCursorMilli = -125;
      break;

    case  4:
      PreCursorMilli  =    0;
      PostCursorMilli =    0;
      break;

    case  5:
      PreCursorMilli  = -100;
      PostCursorMilli =    0;
      break;

    case  6:
      PreCursorMilli  = -125;
      PostCursorMilli =    0;
      break;

    case  7:
      PreCursorMilli  = -100;
      PostCursorMilli = -200;
      break;

    case  8:
      PreCursorMilli  = -125;
      PostCursorMilli = -125;
      break;

    case  9:
      PreCursorMilli  = -166;
      PostCursorMilli =    0;
      break;

    case 10:  ///< P10 is unsupported
    default:
      PreCursorMilli  = -100;
      PostCursorMilli = -200;
      DEBUG ((DEBUG_WARN, "GetCoefficientsFromPreset(): Unsupported Preset Requested: P%d. Using P7.\n", Preset));
      break;
  }

  ///
  /// Convert to absolute values
  ///
  if (PreCursorMilli < 0) {
    PreCursorMilli *= -1;
  }
  if (PostCursorMilli < 0) {
    PostCursorMilli *= -1;
  }

  ///
  /// Apply FullSwing
  ///
  PreCursorMilli  *= FullSwing;
  PostCursorMilli *= FullSwing;

  ///
  /// Convert to integers
  ///
  *PreCursor  = (( PreCursorMilli % 1000) >= 500) ? (UINT8) (( PreCursorMilli / 1000) + 1) : (UINT8) ( PreCursorMilli / 1000);
  *PostCursor = ((PostCursorMilli % 1000) >= 500) ? (UINT8) ((PostCursorMilli / 1000) + 1) : (UINT8) (PostCursorMilli / 1000);
  *Cursor     = FullSwing - (*PreCursor) - (*PostCursor);

  return;
}

/**
  Checks for link speed and width downgrades

  @param[in]  PcieAccess                  - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  OriginalLinkSpeed           - Original Speed of the Link
  @param[in]  OriginalLinkWidth           - Original Width of the Link
**/
BOOLEAN
LinkIsDowngraded (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *PcieAccess,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             OriginalLinkSpeed,
  IN  UINT8                             OriginalLinkWidth
  )
{
  BOOLEAN   IsDowngraded;

  IsDowngraded = FALSE;

  if (OriginalLinkSpeed != PcieAccess->GetCurrentLinkSpeed (PcieAccess, PciePort)) {
    DEBUG ((DEBUG_INFO, "Link speed downgrade detected\n"));
    IsDowngraded = TRUE;
  }
  if (OriginalLinkWidth != PcieAccess->GetNegotiatedWidth (PcieAccess, PciePort)) {
    DEBUG ((DEBUG_INFO, "Link width downgrade detected\n"));
    IsDowngraded = TRUE;
  }
  return IsDowngraded;
}

