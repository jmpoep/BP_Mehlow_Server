/** @file
  This file contains functions that initializes PCI Express Root Ports of PCH.

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
#include "PchInitPei.h"
#include <Uefi/UefiBaseType.h>
#include <SaPolicyCommon.h>
#include <Private/Library/PchDmiLib.h>
#include <Private/Library/PeiHsioLib.h>
#include <Library/PeiPchPcieLib.h>
#include <Register/PchRegs.h>
#include <Library/PeiMeLib.h>
#include <Library/MemoryAllocationLib.h>
#include <PchResetPlatformSpecific.h>
#include <Private/Ppi/TetonGlacierPpi.h>
#include <Private/Library/PeiHybridStorageLib.h>

#define LTSSM_POLL_INTERVAL       10u // in microseconds, period for polling port state during SW EQ
#define RECOVERY_TIME_THRESHOLD   40  // in percent, how much time can SW EQ spend in recovery during a single step

#define LINK_ACTIVE_POLL_INTERVAL 100     // in microseconds
#define LINK_ACTIVE_POLL_TIMEOUT  1000000 // in microseconds

#define TETON_GLACIER_VENDOR_ID       0x8086
#define TETON_GLACIER_DEVICE_ID       0x0975
#define INVALID_ROOT_PORT             0xFF
#define INVALID_ROOT_PORT_CONTROLLER  0xFF
#define BUS_NUMBER_FOR_IMR 0x00
#define STATIC_CONFIG      0x1
#define DYNAMIC_CONFIG     0x2

/**
  Device information structure
**/
typedef struct {
  UINT16  Vid;
  UINT16  Did;
  UINT8   MaxLinkSpeed;
} PCIE_DEVICE_INFO;

/**
  PCIe controller configuration strings.
**/
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8* mPcieControllerConfigName[] = {
  "4x1",
  "1x2-2x1",
  "2x2",
  "1x4"
};

/**
  Returns the PCIe controller configuration (4x1, 1x2-2x1, 2x2, 1x4)

  @param[in] ControllerIndex        Number of PCIe controller (0 based)

  @retval PCIe controller configuration
**/
PCIE_CONTROLLER_CONFIG
GetPcieControllerConfig (
  IN     UINT32        ControllerIndex
  )
{
  EFI_STATUS              Status;
  UINT32                  Data32;
  PCIE_CONTROLLER_CONFIG  Config;
  UINT32                  FirstRp;

  FirstRp = ControllerIndex * PCH_PCIE_CONTROLLER_PORTS;

  Status = PchSbiRpPciRead32 (FirstRp, R_PCH_PCIE_CFG_STRPFUSECFG, &Data32);
  ASSERT_EFI_ERROR (Status);

  Config = ((Data32 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);
  DEBUG ((DEBUG_INFO, "PCIe SP%c is %a\n", (UINTN) ('A' + ControllerIndex), mPcieControllerConfigName[Config]));
  return Config;
}

/**
  Programs Isolated Memory Region feature.
  IMR is programmed in a PCH rootport, based on data retrieved from CPU registers.

  @param[in] RpIndex     Root Port Number (0-based)
  @param[in] Rs3Bus      Bus number for IMR. All PCIE data on RS3 will be identified by this bus number
**/
VOID
EnablePcieImr (
  UINT32 RpIndex,
  UINT8  Rs3Bus
  )
{
  UINT32       ImrBaseLow;
  UINT32       ImrBaseHigh;
  UINT32       ImrLimitLow;
  UINT32       ImrLimitHigh;
  PCI_IMR_HOB  *PciImrHob;
  PCH_SBI_PID  ControllerPid;
  UINT32       Data32;
  UINT64       ImrLimit;

  DEBUG ((DEBUG_INFO, "EnablePcieImr: RP %d, bus %d\n", RpIndex, Rs3Bus));

  PciImrHob = NULL;
  PciImrHob = GetFirstGuidHob (&gPciImrHobGuid);
  if (PciImrHob == NULL) {
    DEBUG ((DEBUG_INFO, "EnablePcieImr: no HOB\n"));
    return;
  }
  //
  // Sanity check - don't program PCIe IMR if base address is 0
  //
  if (PciImrHob->PciImrBase == 0) {
    DEBUG ((DEBUG_ERROR, "PcieImr base address is 0, IMR programming skipped!\n"));
    return;
  }
  ImrLimit = PciImrHob->PciImrBase + (PchGetPcieImrSize () << 20);
  ImrBaseLow   = (UINT32) RShiftU64 ((PciImrHob->PciImrBase & 0xFFF00000), 20);
  ImrBaseHigh  = (UINT32) RShiftU64 (PciImrHob->PciImrBase, 32);
  ImrLimitLow  = (UINT32) RShiftU64 ((ImrLimit & 0xFFF00000), 20);
  ImrLimitHigh = (UINT32) RShiftU64 (ImrLimit, 32);
  //
  // IMR base & limit registers in PCH contain bits 63:20 of adress, divided into upper (64:32) and lower (31:20) parts
  // That means bits 19:10 are ignored and addresses are aligned to 1MB.
  //
  ControllerPid = PchGetPcieControllerSbiPid (RpIndex / PCH_PCIE_CONTROLLER_PORTS);


  Data32 = Rs3Bus | (ImrBaseLow << N_SPX_PCR_IMRAMBL_IAMB) | (ImrLimitLow << N_SPX_PCR_IMRAMBL_IAML);
  PchPcrWrite32 (ControllerPid, R_SPX_PCR_IMRAMBL, Data32);
  PchPcrWrite32 (ControllerPid, R_SPX_PCR_IMRAMBU32, ImrBaseHigh);
  PchPcrWrite32 (ControllerPid, R_SPX_PCR_IMRAMLU32, ImrLimitHigh);
  PchPcrWrite32 (ControllerPid, R_SPX_PCR_IMRAMLE, (BIT0 << (RpIndex % PCH_PCIE_CONTROLLER_PORTS)) | B_SPX_PCR_IMRAMLE_SRL);
  PsfSetRs3Bus (Rs3Bus);

  DEBUG ((DEBUG_INFO, "IMR registers: PID %x, +10=%08x, +14=%08x, +18=%08x, +1c=%08x %d\n",
    ControllerPid,
    PchPcrRead32(ControllerPid, R_SPX_PCR_IMRAMBL),
    PchPcrRead32(ControllerPid, R_SPX_PCR_IMRAMBU32),
    PchPcrRead32(ControllerPid, R_SPX_PCR_IMRAMLU32),
    PchPcrRead32(ControllerPid, R_SPX_PCR_IMRAMLE)
    ));

}

/**
  This function assigns bus number to PCIe bus .

  @param[in] RpIndex     Root Port index
**/
VOID
AssignTemporaryBus (
  IN UINT64 RpBase,
  IN UINT8  TempPciBus
  )
{
  UINT64 EpBase;
  //
  // Assign bus numbers to the root port
  //
  PciSegmentAndThenOr32 (
    RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
    (UINT32) ~B_PCI_BRIDGE_BNUM_SBBN_SCBN,
    ((UINT32) (TempPciBus << 8)) | ((UINT32) (TempPciBus << 16))
    );
  //
  // A config write is required in order for the device to re-capture the Bus number,
  // according to PCI Express Base Specification, 2.2.6.2
  // Write to a read-only register VendorID to not cause any side effects.
  //
  EpBase  = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, TempPciBus, 0, 0, 0);
  PciSegmentWrite16 (EpBase + PCI_VENDOR_ID_OFFSET, 0);
}

/**
  Clear temp bus usage.

  @param[in] RpBase     Root Port PCI base address
**/
VOID
ClearBus (
  IN UINT64 RpBase
  )
{
  PciSegmentAnd32 (
    RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
    (UINT32) ~B_PCI_BRIDGE_BNUM_SBBN_SCBN
    );
}

/**
  This function sets Common Clock Mode bit in rootport and endpoint connected to it, if both sides support it.
  This bit influences rootport's Gen3 training and should be set before Gen3 software equalization is attempted.
  It does not attempt to set CCC in further links behind rootport

  @param[in] RpIndex     Root Port index
**/
VOID
EnableCommonClock (
  IN UINT32 RpIndex,
  IN UINT8  TempPciBus
  )
{
  UINTN  RpDevice;
  UINTN  RpFunction;
  UINT64 RpBase;
  UINT64 EpBase;
  UINT8  Func;
  UINT8  EpPcieCapOffset;
  GetPchPcieRpDevFun (RpIndex, &RpDevice, &RpFunction);
  RpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, RpDevice, RpFunction, 0);
  AssignTemporaryBus (RpBase, TempPciBus);
  EpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, TempPciBus, 0, 0, 0);
  EpPcieCapOffset = PcieBaseFindCapId (EpBase, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (GetScc (RpBase, R_PCH_PCIE_CFG_CLIST) && (EpPcieCapOffset != 0) && GetScc (EpBase, EpPcieCapOffset)) {
    EnableCcc (RpBase, R_PCH_PCIE_CFG_CLIST);
    EnableCcc (EpBase, EpPcieCapOffset);
    if (IsMultifunctionDevice (EpBase)) {
      for (Func = 1; Func <= PCI_MAX_FUNC; Func++) {
        EpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, TempPciBus, 0, Func, 0);
        EnableCcc (EpBase, PcieBaseFindCapId (EpBase, EFI_PCI_CAPABILITY_ID_PCIEXP));
      }
    }
    RetrainLink (RpBase, R_PCH_PCIE_CFG_CLIST, TRUE);
  }
  ClearBus (RpBase);
}
/**
  This function determines whether root port is configured in non-common clock mode.
  Result is based on the NCC soft-strap setting.

  @param[in] RpBase      Root Port pci segment base address

  @retval TRUE           Port in NCC SSC mode.
  @retval FALSE          Port not in NCC SSC mode.
**/
BOOLEAN
IsPcieNcc (
  IN     UINT64  RpBase
  )
{
  if (PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_SCC) {
    return FALSE;
  } else {
    return TRUE;
  }
}

/**
  Determines whether PCIe link is active

  @param[in] RpBase    Root Port base address
  @retval Link Active state
**/
STATIC
BOOLEAN
IsLinkActive (
  UINT64  RpBase
  )
{
  return !! (PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LA);
}

/**
  This function checks if de-emphasis needs to be changed from default for a given rootport

  @param[in] PortIndex    Root Port number
  @param[in] PcieConfig   Pointer to a PCH_PCIE_CONFIG that provides the platform setting
  @param[in] DevInfo      Information on device that is connected to rootport

  @retval TRUE            De-emphasis needs to be changed
  @retval FALSE           No need to change de-emphasis
**/
BOOLEAN
NeedDecreasedDeEmphasis (
  IN PCIE_DEVICE_INFO      DevInfo
  )
{
  //
  // Intel WiGig devices
  //
  if (DevInfo.Vid == V_PCH_INTEL_VENDOR_ID && DevInfo.Did == 0x093C) {
    return TRUE;
  }
  return FALSE;
}

/**
  Programs presets-to-coefficients mapping for hardware equalization.
  It should not be performed for ports without Gen3 capability
  Programming takes less than 2us; timeout is implemented just in case.

  @param[in] RpBase       Root Port pci segment base address
**/
VOID
PresetToCoefficientMapping (
  UINT64 RpBase
  )
{
  UINT32 TimeoutUs;

  if (GetMaxLinkSpeed (RpBase) < V_PCIE_LCAP_MLS_GEN3) {
    return;
  }
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, B_PCH_PCIE_CFG_EQCFG1_HPCMQE);
  TimeoutUs = 1000;
  while (TimeoutUs-- > 0) {
    if ((PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EQCFG1) & B_PCH_PCIE_CFG_EQCFG1_HPCMQE) == 0) {
      return;
    }
    MicroSecondDelay (1);
  }
  ASSERT (FALSE);
}

/**
  Detect whether CLKREQ# is supported by the platform and device.

  Assumes device presence. Device will pull CLKREQ# low until CPM is enabled.
  Test pad state to verify that the signal is correctly connected.
  This function will change pad mode to GPIO!

  @param[in] RootPortConfig      Root port configuration
  @param[in] DevicePresent       Determines whether there is a device on the port

  @retval TRUE if supported, FALSE otherwise.
**/
BOOLEAN
PchPcieDetectClkreq (
  IN       UINT32          RpIndex,
  IN CONST PCH_PCIE_CONFIG *PcieConfig
  )
{
  if (!IsClkReqAssigned (PchClockUsagePchPcie0 + RpIndex)) {
    return FALSE;
  }
  if (PcieConfig->RootPort[RpIndex].ClkReqDetect &&
      EFI_ERROR (CheckClkReq (PchClockUsagePchPcie0 + RpIndex))) {
    DEBUG ((DEBUG_INFO, "CLKREQ is not Low, disabling power management for RP %d.\n", RpIndex));
    return FALSE;
  }
  return TRUE;
}

/**
  Disables the root port. Depending on 2nd param, port's PCI config space may be left visible
  to prevent function swapping

  Use sideband access unless the port is still available.

  @param[in] PortIndex          Root Port Number
  @param[in] KeepPortVisible    Should the port' interface be left visible on PCI Bus0
**/
VOID
PchDisableRootPort (
  IN UINT8   RpIndex,
  IN BOOLEAN KeepPortVisible
  )
{
  UINT32      Data32;
  UINT32      LoopTime;
  UINT32      TargetState;
  UINT32      LinkActive;

  DEBUG ((DEBUG_INFO, "PchDisableRootPort(%d) Start\n", RpIndex + 1));

  Data32 = 0;

  PchSbiRpPciRead32 (RpIndex, (R_PCH_PCIE_CFG_LSTS-2), &Data32);//access LSTS using dword-aligned read
  LinkActive = (Data32 >> 16) & B_PCIE_LSTS_LA;

  if (LinkActive) {
    ///
    /// If device is present disable the link.
    ///
    DEBUG ((DEBUG_INFO, "Disabling the link.\n"));
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_LCTL, ~0u, B_PCIE_LCTL_LD);
  } else {
    ///
    /// Otherwise if device is not present perform the following steps using sideband access:
    /// 1.  Set B0:Dxx:Fn:338h[26] = 1b
    /// 2.  Poll B0:Dxx:Fn:328h[31:24] until 0x1 with 50ms timeout
    /// 3.  Set B0:Dxx:Fn +408h[27] =1b
    ///

    DEBUG ((DEBUG_INFO, "Stopping the port.\n"));
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCIEALC, ~0u, B_PCH_PCIE_CFG_PCIEALC_BLKDQDA);

    TargetState = V_PCH_PCIE_CFG_PCIESTS1_LTSMSTATE_DETRDY;
    for (LoopTime = 0; LoopTime < 5000; LoopTime++) {
      PchSbiRpPciRead32 (RpIndex, R_PCH_PCIE_CFG_PCIESTS1, &Data32);
      if (((Data32 & B_PCH_PCIE_CFG_PCIESTS1_LTSMSTATE) >> N_PCH_PCIE_CFG_PCIESTS1_LTSMSTATE) == TargetState) {
        break;
      }
      MicroSecondDelay (10);
    }

  }
  ///
  /// Set offset 408h[27] to 1b to disable squelch.
  ///
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PHYCTL4, ~0u, B_PCH_PCIE_CFG_PHYCTL4_SQDIS);

  ///
  /// Make port disappear from PCI bus
  ///
  if (!KeepPortVisible) {
    DEBUG ((DEBUG_INFO, "Hiding the port\n"));
    ///
    /// PCIe RP IOSF Sideband register offset 0x00[19:16], depending on the port that is Function Disabled
    /// Access it by offset 0x02[4:0] to avoid RWO bit
    ///
    PchPcrAndThenOr8 (
      GetRpSbiPid (RpIndex),
      R_SPX_PCR_PCD + 0x02,
      0x0F,
      (UINT8) (1 << (RpIndex % 4))
      );
    ///
    /// Then disable the port in PSF
    ///
    PsfDisablePcieRootPort (RpIndex);
  }
  ///
  /// Function disable PCIE port in PMC
  ///
  PmcDisablePcieRootPort (RpIndex);
  DisableClock (PchClockUsagePchPcie0 + RpIndex);
  DEBUG ((DEBUG_INFO, "PchDisableRootPort() End\n"));
}

/**
  This function creates Capability and Extended Capability List

  @param[in] RpIndex         Root Port index
  @param[in] RpBase          Root Port pci segment base address
  @param[in] PcieRpConfig    Pointer to a PCH_PCIE_CONFIG that provides the platform setting

**/
VOID
InitCapabilityList (
  IN UINT32                           RpIndex,
  IN UINT64                           RpBase,
  IN CONST PCH_PCIE_ROOT_PORT_CONFIG  *PcieRpConfig
  )
{
  UINT32      Data32;
  UINT16      Data16;
  UINT8       Data8;
  UINT16      NextCap;

  ///
  /// Build Capability linked list
  /// 1.  Read and write back to capability registers 34h, 41h, 81h and 91h using byte access.
  /// 2.  Program NSR, A4h[3] = 0b
  ///
  Data8 = PciSegmentRead8 (RpBase + PCI_CAPBILITY_POINTER_OFFSET);
  PciSegmentWrite8 (RpBase + PCI_CAPBILITY_POINTER_OFFSET, Data8);

  Data16 = PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_CLIST);
  PciSegmentWrite16 (RpBase + R_PCH_PCIE_CFG_CLIST, Data16);

  Data16 = PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_MID);
  PciSegmentWrite16 (RpBase + R_PCH_PCIE_CFG_MID, Data16);

  Data16 = PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_SVCAP);
  PciSegmentWrite16 (RpBase + R_PCH_PCIE_CFG_SVCAP, Data16);

  Data32 = PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_PMCS);
  Data32 &= (UINT32) ~(B_PCIE_PMCS_NSR);
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_PMCS, Data32);

  /*
  a. NEXT_CAP = 0h
  */
  NextCap     = V_PCIE_EXCAP_NCO_LISTEND;


  /*
  b. If Downstream Port Containment is enabled, then
    1. Set Next Capability Offset, Dxx:Fn +250h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +250h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +250h[15:0] = 001h
    4. NEXT_CAP = 250h
    ELSE, set Dxx:Fn +250h [31:0] = 0

  */
  Data32 = 0;
  if (PcieRpConfig->DpcEnabled == TRUE) {
    Data32  = (V_PCH_PCIE_CFG_EX_DPCECH_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_DPC_CID;
    Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
    NextCap = R_PCH_PCIE_CFG_EX_DPCECH;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_DPCECH, Data32);
  /*
    Set DPC capabilities
  */
  if (PcieRpConfig->DpcEnabled == TRUE) {
    Data32 = PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_DPCECH + R_PCIE_EX_DPC_CAP_OFFSET);
    Data32 &= ~BIT5;
    if (PcieRpConfig->RpDpcExtensionsEnabled == TRUE) {
      Data32 |= BIT5;
    }
  } else {
    Data32 = 0;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_DPCECH + R_PCIE_EX_DPC_CAP_OFFSET, Data32);

  /*
  c. If the RP is GEN3 capable (by fuse and BIOS policy), enable Secondary PCI Express Extended Capability
    1. Set Next Capability Offset, Dxx:Fn +220h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +220h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +220h[15:0] = 0019h
    4. NEXT_CAP = 220h
    ELSE, set Dxx:Fn +220h [31:0] = 0
  */

  Data32 = 0;
  if (GetMaxLinkSpeed (RpBase) >= V_PCIE_LCAP_MLS_GEN3) {
    Data32  = (V_PCH_PCIE_CFG_EX_SPEECH_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_SPE_CID;
    Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
    NextCap = R_PCH_PCIE_CFG_EX_SPEECH;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_SPEECH, Data32);

  /*
  d. If support L1 Sub-State
    1. Set Next Capability Offset, Dxx:Fn +200h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +200h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +200h[15:0] = 001Eh
    4. Read and write back to Dxx:Fn +204h
    5. Refer to section 8.3 for other requirements (Not implemented here)
    6. NEXT_CAP = 200h
    ELSE, set Dxx:Fn +200h [31:0] = 0, and read and write back to Dxx:Fn +204h
  */

  Data32 = 0;
  if (IsClkReqAssigned (PchClockUsagePchPcie0 + RpIndex) &&
      (PcieRpConfig->L1Substates != PchPcieL1SubstatesDisabled)) {
    Data32  = (V_PCH_PCIE_CFG_EX_L1S_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_L1S_CID;
    Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
    NextCap = R_PCH_PCIE_CFG_EX_L1SECH;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_L1SECH, Data32);

  Data32 = PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_L1SCAP);
  if (PcieRpConfig->L1Substates == PchPcieL1SubstatesDisabled) {
    Data32 &= (UINT32) ~(0x1F);
  } else if (PcieRpConfig->L1Substates == PchPcieL1SubstatesL1_1) {
    Data32 &= (UINT32) ~(BIT0 | BIT2);
  }
  //
  // Set TpowerOn capability to 44us
  //
  Data32 &= ~(B_PCIE_EX_L1SCAP_PTV | B_PCIE_EX_L1SCAP_PTPOS);
  Data32 |=  (22 << N_PCIE_EX_L1SCAP_PTV) | (V_PCIE_EX_L1SCAP_PTPOS_2us << N_PCIE_EX_L1SCAP_PTPOS);
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_L1SCAP, Data32);

  /*
  e. If support PTM
    1. Set Next Capability Offset, Dxx:Fn +150h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +140h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +140h[15:0] = 001Fh
    4. Read and write back to Dxx:Fn +144h
    5. NEXT_CAP = 140h
    ELSE, set Dxx:Fn +150h [31:0] = 0
    In both cases: read Dxx:Fn + 154h, set BIT1 and BIT2 then write it back
  */
  Data32 = 0;
  if (PcieRpConfig->PtmEnabled == TRUE) {
    Data32 = (V_PCH_PCIE_CFG_EX_PTM_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_PTM_CID;
    Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
    NextCap = R_PCH_PCIE_CFG_EX_PTMECH;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_PTMECH, Data32);
  Data32 = PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_PTMCAPR);
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_PTMCAPR, (Data32 | B_PCIE_EX_PTMCAP_PTMRC | B_PCIE_EX_PTMCAP_PTMRSPC));

  /*
  f. If support ACS
    1. Set Next Capability Offset, Dxx:Fn +140h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +140h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +140h[15:0] = 000Dh
    4. Read and write back to Dxx:Fn +144h
    5. NEXT_CAP = 140h
    ELSE, set Dxx:Fn +140h [31:0] = 0, and read and write back to Dxx:Fn +144h
  */
  Data32 = 0;
  if (PcieRpConfig->AcsEnabled == TRUE) {
    Data32 = (V_PCH_PCIE_CFG_EX_ACS_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_ACS_CID;
    Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
    NextCap = R_PCH_PCIE_CFG_EX_ACSECH;
  }
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_ACSECH, Data32);

  Data32 = PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_ACSCAPR);
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_ACSCAPR, Data32);

  /*
  g. If support Advanced Error Reporting
    1. Set Next Capability Offset, Dxx:Fn +100h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +100h[19:16] = 1h
    3. Set Capability ID, Dxx:Fn +100h[15:0] = 0001h
    ELSE
    1. Set Next Capability Offset, Dxx:Fn +100h[31:20] = NEXT_CAP
    2. Set Capability Version, Dxx:Fn +100h[19:16]  = 0h
    3. Set Capability ID, Dxx:Fn +100h[15:10]  = 0000h
  */

  Data32 = 0;
  if (PcieRpConfig->AdvancedErrorReporting) {
    Data32 = (V_PCH_PCIE_CFG_EX_AEC_CV << N_PCIE_EXCAP_CV) | V_PCIE_EX_AEC_CID;
  }
  Data32 |= (NextCap << N_PCIE_EXCAP_NCO);
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_EX_AECH, Data32);

  //
  // Mask Unexpected Completion uncorrectable error
  //
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EX_UEM, B_PCIE_EX_UEM_UC);
}

/**
  The function to change the root port speed based on policy

  @param[in] SiPolicyPpi The SI Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
**/
EFI_STATUS
PchPcieRpSpeedChange (
  IN CONST SI_POLICY_PPI           *SiPolicyPpi,
  IN BOOLEAN                        *Gen3DeviceFound
  )
{
  EFI_STATUS            Status;
  UINTN                 PortIndex;
  UINTN                 PchMaxPciePortNum;
  UINT64                PciRootPortRegBase[PCH_MAX_PCIE_ROOT_PORTS];
  UINTN                 RpDev;
  UINTN                 RpFunc;
  UINTN                 LinkRetrainedBitmap;
  UINTN                 TimeoutCount;
  UINT32                MaxLinkSpeed;
  PCH_PCIE_CONFIG       *PcieRpConfig;
  PCH_SATA_CONFIG       *SataConfig;

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gSataConfigGuid, (VOID *) &SataConfig);
  ASSERT_EFI_ERROR (Status);

  PchMaxPciePortNum = GetPchMaxPciePortNum ();
  //
  // Since we are using the root port base many times, it is best to cache them.
  //
  for (PortIndex = 0; PortIndex < PchMaxPciePortNum; PortIndex++) {
    PciRootPortRegBase[PortIndex] = 0;
    Status = GetPchPcieRpDevFun (PortIndex, &RpDev, &RpFunc);
    if (EFI_ERROR (Status)) {
      ASSERT (FALSE);
      continue;
    }
    PciRootPortRegBase[PortIndex] = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, (UINT32) RpDev, (UINT32) RpFunc, 0);
  }

  ///
  /// PCH BIOS Spec Section 8.14 Additional PCI Express* Programming Steps
  /// NOTE: Detection of Non-Complaint PCI Express Devices
  ///
  LinkRetrainedBitmap = 0;
  for (PortIndex = 0; PortIndex < PchMaxPciePortNum; PortIndex++) {
    if (PciRootPortRegBase[PortIndex] == 0) {
      continue;
    }
    if (PciSegmentRead16 (PciRootPortRegBase[PortIndex] + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }

    MaxLinkSpeed = GetMaxLinkSpeed (PciRootPortRegBase[PortIndex]);

    if (MaxLinkSpeed > 1) {
      PciSegmentAndThenOr16 (
        PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LCTL2,
        (UINT16) ~B_PCIE_LCTL2_TLS,
        (UINT16) MaxLinkSpeed
        );
      if (IsLinkActive (PciRootPortRegBase[PortIndex])) {
        //
        // Retrain the link if device is present
        //
        PciSegmentOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
        LinkRetrainedBitmap |= (1u << PortIndex);
      }
    }
  }

  //
  // 15 ms timeout while checking for link active on retrained link
  //
  for (TimeoutCount = 0; ((LinkRetrainedBitmap != 0) && (TimeoutCount < 150)); TimeoutCount++) {
    //
    // Delay 100 us
    //
    MicroSecondDelay (100);
    //
    // Check for remaining root port which was link retrained
    //
    for (PortIndex = 0; PortIndex < PchMaxPciePortNum; PortIndex++) {
      if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
        //
        // If the link is active, clear the bitmap
        //
        if (PciSegmentRead16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LA) {
          LinkRetrainedBitmap &= ~(1u << PortIndex);
        }
      }
    }
  }

  //
  // If 15 ms has timeout, and some link are not active, train to gen1
  //
  if (LinkRetrainedBitmap != 0) {
    for (PortIndex = 0; PortIndex < PchMaxPciePortNum; PortIndex++) {
      if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
        //
        // Set TLS to gen1
        //
        PciSegmentAndThenOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LCTL2,
          (UINT16) ~(B_PCIE_LCTL2_TLS),
          V_PCIE_LCTL2_TLS_GEN1);
        //
        // Retrain link
        //
        PciSegmentOr16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
      }
    }

    //
    // Wait for retrain completion or timeout in 15ms. Do not expect failure as
    // port was detected and trained as Gen1 earlier
    //
    for (TimeoutCount = 0; ((LinkRetrainedBitmap != 0) && (TimeoutCount < 150)); TimeoutCount++) {
      //
      // Delay 100 us
      //
      MicroSecondDelay (100);
      //
      // Check for remaining root port which was link retrained
      //
      for (PortIndex = 0; PortIndex < PchMaxPciePortNum; PortIndex++) {
        if ((LinkRetrainedBitmap & (1u << PortIndex)) != 0) {
          //
          // If the link is active, clear the bitmap
          //
          if (PciSegmentRead16 (PciRootPortRegBase[PortIndex] + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LA) {
            LinkRetrainedBitmap &= ~(1u << PortIndex);
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

/*
  Checks if given rootport has an endpoint connected

  @param[in] DeviceBase       PCI segment base address of root port

  @retval                     TRUE if endpoint is connected
  @retval                     FALSE if no endpoint was detected
*/
BOOLEAN
IsEndpointConnected (
  UINT64 DeviceBase
  )
{
  return !!(PciSegmentRead16 (DeviceBase + R_PCH_PCIE_CFG_SLSTS) & B_PCIE_SLSTS_PDS);
}

/*
  Some PCIe devices may take long time between they become detected and form a link.
  This function waits until all enabled, non-empty rootports form a link or until a timeout

  @param[in] MaxRootPorts      number of rootports
  @param[in] DisabledPortMask  mask of rootprots that don't need to be considered
*/
VOID
WaitForLinkActive (
  UINT32 MaxRootPorts,
  UINT32 DisabledPortMask
  )
{
  UINT32 PortMask;
  UINT32 Index;
  UINT32 Time;
  UINT64 RpBase;

  Time = 0;
  //
  // Set a bit in PortMask for each rootport that exists and isn't going to be disabled
  //
  PortMask = (0x1 << MaxRootPorts) - 1;
  PortMask &= ~DisabledPortMask;

  DEBUG ((DEBUG_INFO, "WaitForLinkActive, RP mask to wait for = 0x%08x\n", PortMask));
  while (Time < LINK_ACTIVE_POLL_TIMEOUT) {
    for (Index = 0; Index < MaxRootPorts; Index ++) {
      if (!(PortMask & (BIT0 << Index))) {
        continue;
      }
      RpBase = PchPcieBase (Index);
      //
      // if PDS is not set or if LA is set then this rootport is done - clear it from mask
      //
      if (!IsEndpointConnected (RpBase) || IsLinkActive (RpBase)) {
        PortMask &= ~ (BIT0 << Index);
      }
    }
    if (PortMask == 0x0) {
      DEBUG ((DEBUG_INFO, "WaitForLinkActive, all RPs done, lost %dms waiting\n", Time/1000));
      return;
    }
    MicroSecondDelay (LINK_ACTIVE_POLL_INTERVAL);
    Time += LINK_ACTIVE_POLL_INTERVAL;
  }

  DEBUG ((DEBUG_WARN, "WaitForLinkActive, timeout with the following RPs still not done: 0x%08x\n", PortMask));
}

/**
  Get information about the endpoint

  @param[in]  RpBase      Root port pci segment base address
  @param[in]  TempPciBus  Temporary bus number
  @param[out] DeviceInfo  Device information structure

  @raturn TRUE if device was found, FALSE otherwise
**/
BOOLEAN
GetDeviceInfo (
  IN  UINT64            RpBase,
  IN  UINT8             TempPciBus,
  OUT PCIE_DEVICE_INFO  *DeviceInfo
  )
{
  UINT64                  EpBase;
  UINT32                  Data32;
  UINT8                   EpPcieCapPtr;
  UINT8                   EpLinkSpeed;

  DeviceInfo->Vid = 0xFFFF;
  DeviceInfo->Did = 0xFFFF;
  DeviceInfo->MaxLinkSpeed = 0;

  //
  // Check for device presence
  //
  if (!IsEndpointConnected (RpBase)) {
    return FALSE;
  }

  //
  // Assign temporary bus numbers to the root port
  //
  PciSegmentAndThenOr32 (
    RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
    (UINT32) ~B_PCI_BRIDGE_BNUM_SBBN_SCBN,
    ((UINT32) (TempPciBus << 8)) | ((UINT32) (TempPciBus << 16))
    );

  //
  // A config write is required in order for the device to re-capture the Bus number,
  // according to PCI Express Base Specification, 2.2.6.2
  // Write to a read-only register VendorID to not cause any side effects.
  //
  EpBase  = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, TempPciBus, 0, 0, 0);
  PciSegmentWrite16 (EpBase + PCI_VENDOR_ID_OFFSET, 0);

  Data32 = PciSegmentRead32 (EpBase + PCI_VENDOR_ID_OFFSET);
  DeviceInfo->Vid = (UINT16) (Data32 & 0xFFFF);
  DeviceInfo->Did = (UINT16) (Data32 >> 16);

  EpLinkSpeed = 0;
  EpPcieCapPtr = PcieFindCapId (DEFAULT_PCI_SEGMENT_NUMBER_PCH, TempPciBus, 0, 0, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (EpPcieCapPtr != 0) {
    EpLinkSpeed = PciSegmentRead8 (EpBase + EpPcieCapPtr + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_MLS;
  }
  DeviceInfo->MaxLinkSpeed = EpLinkSpeed;

  //
  // Clear bus numbers
  //
  PciSegmentAnd32 (RpBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, (UINT32) ~B_PCI_BRIDGE_BNUM_SBBN_SCBN);

  DEBUG ((DEBUG_INFO, "VID: %04X DID: %04X  MLS: %d\n",
          DeviceInfo->Vid, DeviceInfo->Did, DeviceInfo->MaxLinkSpeed));

  return (Data32 != 0xFFFFFFFF);
}

/**
  Program controller power management settings.
  This settings are relevant to all ports including disabled ports.
  All registers are located in the first port of the controller.
  Use sideband access since primary may not be available.

  @param[in]  RpIndex               The root port to be initialized (zero based).
  @param[in]  TrunkClockGateEn      Indicates whether trunk clock gating is to be enabled,
                                    requieres all controller ports to have dedicated CLKREQ#
                                    or to be disabled.
**/
VOID
PchPcieConfigureControllerBasePowerManagement (
  IN  UINT32   RpIndex,
  IN  BOOLEAN  TrunkClockGateEn
  )
{
  UINT32      Data32And;
  UINT32      Data32Or;

  DEBUG ((DEBUG_INFO, "PchPcieConfigureControllerBasePowerManagement(%d)\n", RpIndex + 1));

  ASSERT ((RpIndex % PCH_PCIE_CONTROLLER_PORTS) == 0);

  ///
  /// Set E1h[7,5,4,2] to 1111b   (R_PCH_PCIE_CFG_RPDCGEN)
  /// Set E2h[4] to 1b            (R_PCH_PCIE_CFG_RPPGEN)
  /// Set E1h[6] to 1b if all ports on the controller support CLKREQ#   (R_PCH_PCIE_CFG_RPDCGEN)
  ///
  Data32Or  = (B_PCH_PCIE_CFG_RPDCGEN_RPSCGEN | B_PCH_PCIE_CFG_RPDCGEN_LCLKREQEN |
               B_PCH_PCIE_CFG_RPDCGEN_BBCLKREQEN | B_PCH_PCIE_CFG_RPDCGEN_SRDBCGEN) << 8;
  Data32Or |= B_PCH_PCIE_CFG_RPPGEN_SEOSCGE << 16;
  if (TrunkClockGateEn) {
    DEBUG ((DEBUG_INFO, "Setting PTOCGE\n"));
    Data32Or |= (B_PCH_PCIE_CFG_RPDCGEN_PTOCGE << 8);
  }
  PchSbiRpPciAndThenOr32 (RpIndex, 0xE0, ~0u, Data32Or);

  ///
  /// Set E8h[17,15] to [1,1]
  ///
  Data32Or = B_PCH_PCIE_CFG_PWRCTL_WPDMPGEP | B_PCH_PCIE_CFG_PWRCTL_DBUPI;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PWRCTL, ~0u, Data32Or);
  ///
  /// Set F5h[1:0] to 11b  (R_PCH_PCIE_CFG_PHYCTL2)
  /// Set F7h[3:2] = 00b   (R_PCH_PCIE_CFG_IOSFSBCS)
  ///
  Data32And = (UINT32) ~(B_PCH_PCIE_CFG_IOSFSBCS_SIID << 24);
  Data32Or = (B_PCH_PCIE_CFG_PHYCTL2_PXPG3PLLOFFEN | B_PCH_PCIE_CFG_PHYCTL2_PXPG2PLLOFFEN) << 8;
  PchSbiRpPciAndThenOr32 (RpIndex, 0xF4, Data32And, Data32Or);

  ///
  /// Set 424h[11] to 1b
  ///
  Data32Or = B_PCH_PCIE_CFG_PCIEPMECTL2_PHYCLPGE;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCIEPMECTL2, ~0u, Data32Or);

  ///
  /// Set 428h[5] to 1b
  /// Set 428h[0] to 0b
  Data32And = (UINT32) ~B_PCH_PCIE_CFG_PCE_PMCRE;
  Data32Or = B_PCH_PCIE_CFG_PCE_HAE;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCE, Data32And, Data32Or);
}


/**
  Configure power management settings whcih are applicable to both enabled and disabled ports.
  This settings are relevant to all ports including disabled ports.
  Use sideband access since primary may not be available.

  @param[in]  RpIndex               The root port to be initialized (zero based).
  @param[in]  PhyLanePgEnable       Indicates whether PHY lane power gating is to be enabled,
                                    requires CLKREQ# to supported by the port or the port to be disabled.
  @param[in]  TbtWorkAround         TBT status indicator.
**/
VOID
PchPcieConfigurePortBasePowerManagement (
  IN  UINT32   RpIndex,
  IN  BOOLEAN  PhyLanePgEnable,
  IN  BOOLEAN  TbtWorkAround
  )
{

  UINT32      Data32;
  UINT32      Data32And;
  UINT32      Data32Or;

  DEBUG ((DEBUG_INFO, "PchPcieConfigureBasePowerManagement(%d) Start\n", RpIndex + 1));

  ///
  /// Set E1h[1:0] = 11b    (R_PCH_PCIE_CFG_RPDCGEN)
  ///
  Data32Or = (B_PCH_PCIE_CFG_RPDCGEN_RPDLCGEN | B_PCH_PCIE_CFG_RPDCGEN_RPDBCGEN) << 8;
  PchSbiRpPciAndThenOr32 (RpIndex, 0xE0, ~0u, Data32Or);

  ///
  /// Set F7h[6] to 1b     (R_PCH_PCIE_CFG_IOSFSBCS)
  ///
  Data32Or = B_PCH_PCIE_CFG_IOSFSBCS_SCPTCGE << 24;
  Data32And = (UINT32) ~(BIT8);
  if (TbtWorkAround) {
    PchSbiRpPciAndThenOr32(RpIndex, 0xF4, Data32And, Data32Or);
  } else {
    PchSbiRpPciAndThenOr32(RpIndex, 0xF4, ~0u, Data32Or);
  }

  DEBUG_CODE_BEGIN ();
  //
  // Ensure PHYCLPGE is set before DLSULPPGE and FDPPGE
  //
  PchSbiRpPciRead32 (PchGetPcieFirstPortIndex (RpIndex), R_PCH_PCIE_CFG_PCIEPMECTL2, &Data32);
  ASSERT ((Data32 & B_PCH_PCIE_CFG_PCIEPMECTL2_PHYCLPGE) != 0);
  DEBUG_CODE_END ();

  ///
  /// Set 420h[31] = 1b
  /// If CLKREQ# is supported or port is disabled set 420h[30,29] to 11b.
  /// 420h[29] (DLSULDLSD) and 420h[0] must be set if DLSULPPGE is set or PTOCGE is set.
  /// Assume that if PTOCGE is set CLKREQ is supported on this port.
  /// L1.LOW is disabled; if all conditions are met, it will be enabled later.
  ///
  Data32Or  = B_PCH_PCIE_CFG_PCIEPMECTL_FDPPGE;
  Data32And = (UINT32) ~(B_PCH_PCIE_CFG_PCIEPMECTL_L1LE);
  if (PhyLanePgEnable) {
    DEBUG ((DEBUG_INFO, "Setting DLSULPPGE+DLSULDLSD.\n"));
    Data32Or |= B_PCH_PCIE_CFG_PCIEPMECTL_DLSULPPGE | B_PCH_PCIE_CFG_PCIEPMECTL_DLSULDLSD | B_PCH_PCIE_CFG_PCIEPMECTL_L1FSOE;
  }
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCIEPMECTL, Data32And, Data32Or);

  ///
  /// Set 424h[8,7] to 11b
  ///
  Data32Or = B_PCH_PCIE_CFG_PCIEPMECTL2_FDCPGE | B_PCH_PCIE_CFG_PCIEPMECTL2_DETSCPGE | B_PCH_PCIE_CFG_PCIEPMECTL2_DISSCPGE;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCIEPMECTL2, ~0u, Data32Or);
}

/**
  Set Gen3 coefficient list entry.

  @param[in] RpBase      Root Port pci segment base address
  @param[in] ListEntry   ListEntry (0-9)
  @param[in] Cm          C-1
  @param[in] Cp          C+1
**/
VOID
PcieSetCoeffList (
  UINT64 RpBase,
  UINT32 ListEntry,
  UINT32 Cm,
  UINT32 Cp
  )
{
  UINT32  PreReg;
  UINT32  PostReg;
  UINT32  PreField;
  UINT32  PostField;
  UINT32  Data32And;
  UINT32  Data32Or;

  ASSERT (ListEntry < 10);
  ASSERT ((Cm & ~0x3F) == 0);
  ASSERT ((Cp & ~0x3F) == 0);

  PreReg    = ((ListEntry * 2))     / 5;
  PreField  = ((ListEntry * 2))     % 5;
  PostReg   = ((ListEntry * 2) + 1) / 5;
  PostField = ((ListEntry * 2) + 1) % 5;

  ASSERT (PreReg  < 4);
  ASSERT (PostReg < 4);

  Data32And = (UINT32) ~(0x3F << (6 * PreField));
  Data32Or  = (Cm << (6 * PreField));
  ASSERT ((Data32And & Data32Or) == 0);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_RTPCL1 + (PreReg  * 4), Data32And, Data32Or);

  Data32And = (UINT32) ~(0x3F << (6 * PostField));
  Data32Or  = (Cp << (6 * PostField));
  ASSERT ((Data32And & Data32Or) == 0);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_RTPCL1 + (PostReg * 4), Data32And, Data32Or);

  DEBUG ((DEBUG_INFO, "Port %d list %d: (%d,%d)\n",
          PciePortNum (RpBase), ListEntry, Cm, Cp));
}

/**
  Gen3 Remote transmitter coefficient override

  @param[in] RpBase    Root Port pci segment base address
  @param[in] RpLaneIndex    Root Port Lane (0-3)
  @param[in] Cm        C-1
  @param[in] Cp        C+1
**/
STATIC
VOID
PcieSetCoefficients (
  UINT64 RpBase,
  UINT32 RpLaneIndex,
  UINT32 Cm,
  UINT32 Cp
  )
{
  UINT32  Mask[2];
  UINT32  Data[2];

  static UINT8 CmShift[] = { 0, 12, 24,  6 };
  static UINT8 CmReg  [] = { 0,  0,  0,  1 };
  static UINT8 CpShift[] = { 6, 18,  0, 12 };
  static UINT8 CpReg  [] = { 0,  0,  1,  1 };

  ASSERT (RpLaneIndex < 4);
  ASSERT ((Cm & ~0x3F) == 0);
  ASSERT ((Cp & ~0x3F) == 0);

  Mask[0] = 0;
  Mask[1] = 0;
  Data[0] = 0;
  Data[1] = 0;

  Mask[CmReg[RpLaneIndex]] |= 0x3Fu << CmShift[RpLaneIndex];
  Data[CmReg[RpLaneIndex]] |= Cm    << CmShift[RpLaneIndex];
  Mask[CpReg[RpLaneIndex]] |= 0x3Fu << CpShift[RpLaneIndex];
  Data[CpReg[RpLaneIndex]] |= Cp    << CpShift[RpLaneIndex];

  DEBUG ((DEBUG_INFO, "Port %d.%d use coefficients (%d, %d)\n",
          PciePortNum (RpBase), RpLaneIndex, Cm, Cp));

  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_RTPCL1, ~Mask[0], Data[0] | B_PCH_PCIE_CFG_RTPCL1_PCM);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_RTPCL2, ~Mask[1], Data[1]);
  //
  // Remote Transmitter Preset Coefficient Override Enable
  //
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, B_PCH_PCIE_CFG_EQCFG1_RTPCOE);
}

/**
  Reset and enable Recovery Entry and Idle Framing Error Count

  @param[in] RpBase    Root Port base address
**/
STATIC
VOID
ResetErrorCounts (
  UINT64 RpBase
  )
{
  PciSegmentAnd32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, (UINT32) ~B_PCH_PCIE_CFG_EQCFG1_REIFECE);
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, B_PCH_PCIE_CFG_EQCFG1_REIFECE);
}

/**
  Get max link width.

  @param[in] RpBase    Root Port base address
  @retval Max link width
**/
STATIC
UINT8
GetMaxLinkWidth (
  UINT64  RpBase
  )
{
  UINT8  LinkWidth;
  LinkWidth = (UINT8) ((PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_LCAP) & B_PCIE_LCAP_MLW) >> N_PCIE_LCAP_MLW);
  ASSERT (LinkWidth <= 4);
  if (LinkWidth > 4) {
    LinkWidth = 4;
  }
  return LinkWidth;
}



/**
  Populate HW EQ coefficient search list.
  @param[in] RpBase    Root Port base address
  @param[in] Params    Equalization parameters
**/
VOID
InitializeCoeffList (
        UINT64           RpBase,
  CONST PCH_PCIE_CONFIG  *PcieRpConfig
  )
{
  UINT32 Index;

  for (Index = 0; Index < PCH_PCIE_SWEQ_COEFFS_MAX; ++Index) {
    PcieSetCoeffList (
      RpBase,
      Index,
      PcieRpConfig->SwEqCoeffList[Index].Cm,
      PcieRpConfig->SwEqCoeffList[Index].Cp
      );
  }

  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_RTPCL1, B_PCH_PCIE_CFG_RTPCL1_PCM);

  // Total number of coefficients
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG2, (UINT32) ~B_PCH_PCIE_CFG_EQCFG2_HAPCSB, 4 << N_PCH_PCIE_CFG_EQCFG2_HAPCSB);
  // Number of coefficients per iteration
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_HAEQ,   (UINT32) ~B_PCH_PCIE_CFG_HAEQ_HAPCCPI,  4 << N_PCH_PCIE_CFG_HAEQ_HAPCCPI);

  for (Index = 0; Index < 4; ++Index) {
    DEBUG ((DEBUG_INFO, "RTPCL%d = 0x%08x\n", Index, PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_RTPCL1 + (Index * 4))));
  }
}

/**
  Configures rootport for hardware Gen3 link equalization.
  @param[in] Eq       Equaliztion context structure
  @param[in] Params   Equalization parameters
**/
VOID
DoGen3HardwareEq (
  UINT64                RpBase,
  CONST PCH_PCIE_CONFIG *Params
  )
{
  UINT32 Data32Or;
  UINT32 Data32And;

  DEBUG ((DEBUG_INFO, "DoGen3HardwareEq\n"));

  DEBUG ((DEBUG_INFO, "LSTS2: 0x%04x\n", PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS2)));

  //
  // Clear Remote Transmitter Preset Coefficient Override Enable
  //
  PciSegmentAnd32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, (UINT32) ~(B_PCH_PCIE_CFG_EQCFG1_RTPCOE |
                                                    B_PCH_PCIE_CFG_EQCFG1_TUPP));

  InitializeCoeffList (RpBase, Params);

  Data32And = (UINT32) ~(B_PCH_PCIE_CFG_EQCFG2_PCET |
                         B_PCH_PCIE_CFG_EQCFG2_REWMET);
  Data32Or  = (4 << N_PCH_PCIE_CFG_EQCFG2_PCET) | 0x02; // REWMET = 2 (4 errors)

  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG2, Data32And, Data32Or);
  PciSegmentAnd32 (RpBase + R_PCH_PCIE_CFG_HAEQ, (UINT32) ~B_PCH_PCIE_CFG_HAEQ_MACFOMC); // Clear MACFOMC

  DEBUG ((DEBUG_INFO, "EQCFG1: 0x%08x\n", PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EQCFG1)));
  DEBUG ((DEBUG_INFO, "EQCFG2: 0x%08x\n", PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EQCFG2)));
  DEBUG ((DEBUG_INFO, "HAEQ:   0x%08x\n", PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_HAEQ)));
  DEBUG ((DEBUG_INFO, "L01EC:  0x%08x\n", PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_L01EC)));
  DEBUG ((DEBUG_INFO, "L23EC:  0x%08x\n", PciSegmentRead32 (RpBase + R_PCH_PCIE_CFG_EX_L23EC)));

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_LCTL2, (UINT16) ~B_PCIE_LCTL2_TLS, 3);
}

/**
  Configures rootport for static Gen3 link equalization.
  @param[in] Eq       Equaliztion context structure
  @param[in] Param    Fixed coefficients to use
**/
VOID
DoGen3StaticEq (
  UINT32                        RpIndex,
  CONST PCH_PCIE_EQ_PARAM       *Param
  )
{
  UINT32  RpLaneIndex;
  UINT32  PcieLane;
  UINT64  RpBase;

  DEBUG ((DEBUG_INFO, "DoGen3StaticEq\n"));

  RpBase = PchPcieBase (RpIndex);
  //
  // Bypass EQ Phase 3
  //
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, B_PCH_PCIE_CFG_EQCFG1_RTLEPCEB);

  for (RpLaneIndex = 0; RpLaneIndex < GetMaxLinkWidth (RpBase); ++RpLaneIndex) {
    PcieLane = PchPciePhysicalLane ( RpIndex, RpLaneIndex);
    PcieSetCoefficients (RpBase, RpLaneIndex, Param[PcieLane].Cm, Param[PcieLane].Cp);
  }

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_LCTL2, (UINT16) ~B_PCIE_LCTL2_TLS, 3);
}

/**
  Perform software link equaliztion (coefficient search).
  @param[in] RpIndex      Port index
  @param[in] SiPolicy     The SI Policy
  @param[in] TempPciBus   Temp bus number
**/
VOID
Gen3LinkEqualize (
  UINT32                 RpIndex,
  CONST SI_POLICY_PPI    *SiPolicy,
  UINT8                  TempPciBus,
  BOOLEAN                Gen3DeviceFound

  )
{
  UINT64                   RpBase;
  PCH_PCIE_EQ_METHOD       EqMethod;
  EFI_STATUS               Status;
  CONST PCH_PCIE_CONFIG    *PcieRpConfig;

  DEBUG ((DEBUG_INFO, "Gen3LinkEqualize\n"));

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);

  RpBase = PchPcieBase (RpIndex);

  ASSERT (GetLinkSpeed (RpBase) < 3);

  EqMethod = PcieRpConfig->RootPort[RpIndex].Gen3EqPh3Method;

  //
  // If both rootport and endpoint support Common Clock config, set it before equalization
  //
  EnableCommonClock (RpIndex, TempPciBus);

  if (EqMethod == PchPcieEqHardware || EqMethod == PchPcieEqDefault) {
    DoGen3HardwareEq (RpBase, PcieRpConfig);
    if (Gen3DeviceFound) {
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EX_LCTL3, B_PCIE_EX_LCTL3_PE);
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
    }
  } else if (EqMethod == PchPcieEqStaticCoeff) {
    DoGen3StaticEq (RpIndex, PcieRpConfig->EqPh3LaneParam);
    if (Gen3DeviceFound) {
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EX_LCTL3, B_PCIE_EX_LCTL3_PE);
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
    }
  } else {
    DEBUG ((DEBUG_INFO, "Invalid EqMethod %d\n", EqMethod));
    ASSERT (FALSE);
  }
  ResetErrorCounts (RpBase);
}

/**
  Initialize non-common clock port.
  Ports with NCC configuration need to have their mPHY lanes reconfigured by BIOS before
  endpoint detection can start. Reconfiguration is instant, but detection may take up to
  100ms. In order to save as much time as possible, this reconfiguration should be executed
  in PEI pre-mem, so that detection happens in parallel with memory init
  @param[in] RpIndex    Root Port index
**/
VOID
PcieInitNccPort (
  IN UINT32   RpIndex
  )
{
  UINT64     RpBase;
  UINT32     RpLaneIndex;
  UINT32     MaxLinkWidth;
  HSIO_LANE  HsioLane;
  UINT8      FiaLane;

  DEBUG ((DEBUG_INFO, "PcieInitNccPort(%d)\n", RpIndex+1));

  RpBase = PchPcieBase (RpIndex);
  MaxLinkWidth = GetMaxLinkWidth (RpBase);
  for (RpLaneIndex = 0; RpLaneIndex < MaxLinkWidth; ++RpLaneIndex) {
    if (PchFiaGetPcieRootPortLaneNum (RpIndex, RpLaneIndex, &FiaLane)) {
      HsioGetLane (FiaLane, &HsioLane);
      HsioPcieNccLaneInit (&HsioLane);
    }
  }
  PciSegmentAnd32 (RpBase + R_PCH_PCIE_CFG_PCIEALC, (UINT32) ~B_PCH_PCIE_CFG_PCIEALC_BLKDQDA);
}

/**
  Verify whether the PCIe port does own all lanes according to the port width.
  @param[in] RpBase    Root Port base address
**/
BOOLEAN
IsPciePortOwningLanes (
  IN     UINT64   RpBase
  )
{
  UINT32     MaxLinkWidth;
  UINT32     RpLaneIndex;
  UINT32     RpIndex;

  RpIndex      = PciePortIndex (RpBase);
  MaxLinkWidth = GetMaxLinkWidth (RpBase);
  for (RpLaneIndex = 0; RpLaneIndex < MaxLinkWidth; ++RpLaneIndex) {
    if (!PchFiaIsPcieRootPortLaneConnected (RpIndex, RpLaneIndex)) {
      return FALSE;
    }
  }
  return TRUE;
}

/**
  Check for device presence with timeout.

  @param[in]     RpBase      Root Port base address
  @param[in]     TimeoutUs   Timeout in microseconds
  @param[in,out] Timer       Timer value, must be initialized to zero
                             before the first call of this function.
**/
BOOLEAN
PchPcieIsDevicePresent (
  IN     UINT64  RpBase,
  IN     UINT32  TimeoutUs,
  IN OUT UINT32  *Timer
  )
{
  while (TRUE) {
    if (IsEndpointConnected (RpBase)) {
      return TRUE;
    }
    if (*Timer < TimeoutUs) {
      MicroSecondDelay (10);
      *Timer += 10;
    } else {
      break;
    }
  }
  return FALSE;
}

/**
  Checks if given rootport should be left visible even though disabled, in order to avoid PCIE rootport swapping

  @param[in] RpIndex           rootport number
  @param[in] RpDisableMask     bitmask of all disabled rootports
  @param[in] PciExpressConfig  PCIe policy configuration

  @retval TRUE  port should be kept visible despite being disabled
  @retval FALSE port should be disabled and hidden

**/
BOOLEAN
IsPortForceVisible (
  IN UINT8                 RpIndex,
  IN UINT32                RpDisableMask,
  IN CONST PCH_PCIE_CONFIG *PciExpressConfig
  )
{
  UINT32 FunctionsEnabledPerDevice;
  UINT32 RpEnabledMask;

  //
  // only rootports mapped to Function0 are relevant for preventing rootport swap
  //
  if ((PciExpressConfig->RpFunctionSwap == 1) || (RpIndex % 8 != 0)) {
    return FALSE;
  }
  //
  // set a bit for each port that exists and isn't disabled
  //
  RpEnabledMask = (1u << GetPchMaxPciePortNum ()) - 1;
  RpEnabledMask &= (~RpDisableMask);

  FunctionsEnabledPerDevice = (RpEnabledMask >> ((RpIndex/8)*8)) & 0xFF;
  if (FunctionsEnabledPerDevice != 0) {
    return TRUE;
  }
  return FALSE;
}

/**
  Configure root port function number mapping

**/
VOID
PchConfigureRpfnMapping (
  VOID
  )
{
  UINT8                                 PortIndex;
  UINT8                                 OriginalFuncZeroRp;
  UINT8                                 MaxPciePortNum;
  UINTN                                 DevNum;
  UINTN                                 FuncNum;
  UINT64                                RpBase;
  UINT32                                ControllerPcd[PCH_MAX_PCIE_CONTROLLERS];
  UINT32                                PcieControllers;
  UINT32                                ControllerIndex;
  UINT32                                FirstController;
  PCH_SBI_PID                           ControllerPid;

  DEBUG ((DEBUG_INFO,"PchConfigureRpfnMapping () Start\n"));
  ZeroMem (ControllerPcd, sizeof (ControllerPcd));
  MaxPciePortNum = GetPchMaxPciePortNum ();

  PcieControllers = GetPchMaxPcieControllerNum ();

  for (ControllerIndex = 0; ControllerIndex < PcieControllers; ++ControllerIndex) {
    ControllerPcd[ControllerIndex] = PchPcrRead32 (PchGetPcieControllerSbiPid (ControllerIndex), R_SPX_PCR_PCD);
    DEBUG ((DEBUG_INFO, "SP%c = 0x%08x\n", 'A' + ControllerIndex, ControllerPcd[ControllerIndex]));
  }

  ///
  /// Configure root port function number mapping
  ///
  for (PortIndex = 0; PortIndex < MaxPciePortNum; ) {
    GetPchPcieRpDevFun (PortIndex, &DevNum, &FuncNum);
    RpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, DevNum, FuncNum, 0);
    //
    // Search for first enabled function
    //
    if (PciSegmentRead16 (RpBase) != 0xFFFF) {
      if (FuncNum != 0) {
        //
        // First enabled root port that is not function zero will be swapped with function zero on the same device
        // RP PCD register must sync with PSF RP function config register
        //
        ControllerIndex    = PortIndex / 4;
        OriginalFuncZeroRp = (PortIndex / 8) * 8;
        FirstController    = OriginalFuncZeroRp / 4;

        //
        // The enabled root port becomes function zero
        //
        ControllerPcd[ControllerIndex] &= (UINT32) ~(B_SPX_PCR_PCD_RP1FN << ((PortIndex % 4) * S_SPX_PCR_PCD_RP_FIELD));
        ControllerPcd[ControllerIndex] |= 0u;
        //
        // Origianl function zero on the same device takes the numer of the current port
        //
        ControllerPcd[FirstController] &= (UINT32) ~B_SPX_PCR_PCD_RP1FN;
        ControllerPcd[FirstController] |= (UINT32) FuncNum;

        //
        // Program PSF1 RP function config register.
        //
        PsfSetPcieFunction (OriginalFuncZeroRp, (UINT32) FuncNum);
        PsfSetPcieFunction (PortIndex, 0);
      }
      //
      // Once enabled root port was found move to next PCI device
      //
      PortIndex = ((PortIndex / 8) + 1) * 8;
      continue;
    }
    //
    // Continue search for first enabled root port
    //
    PortIndex++;
  }

  //
  // Write to PCD and lock the register
  //
  for (ControllerIndex = 0; ControllerIndex < PcieControllers; ++ControllerIndex) {
    ControllerPid = PchGetPcieControllerSbiPid (ControllerIndex);
    PchPcrWrite32 (ControllerPid, R_SPX_PCR_PCD, ControllerPcd[ControllerIndex] | B_SPX_PCR_PCD_SRL);
    DEBUG ((DEBUG_INFO, "SP%c = 0x%08x\n", 'A' + ControllerIndex, PchPcrRead32 (ControllerPid, R_SPX_PCR_PCD)));
  }
}

/**
  Checks integrity of Policy settings for all rootports.
  Triggers assert if anything is wrong. For debug builds only

  @param[in] PciExpressConfig     Pointer to PCH_PCIE_CONFIG instance
**/
VOID
PchPciePolicySanityCheck (
  IN OUT PCH_PCIE_CONFIG *PciExpressConfig
  )
{
  UINT8                       RpIndex;
  PCH_PCIE_ROOT_PORT_CONFIG   *RpConfig;

  for (RpIndex = 0; RpIndex < GetPchMaxPciePortNum (); RpIndex++) {
    RpConfig  = &PciExpressConfig->RootPort[RpIndex];
    //
    // Ports with hotplug support must have SlotImplemented bit set
    //
    ASSERT (!RpConfig->HotPlug || RpConfig->SlotImplemented);
  }
}

/**
  Perform power management configuration when TG is connected

  @param[in] RpIndex                 Index of rootport
**/
STATIC
VOID
TgPowerManagementConfiguration (
  UINT8                               RpIndex
  )
{
  UINT32 Data32Or;

  DEBUG ((DEBUG_INFO, "TgPowerManagementConfiguration () Start\n"));
  Data32Or = (B_PCH_PCIE_CFG_RPDCGEN_PTOCGE << 8);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_SPR, ~0u, Data32Or);
  DEBUG ((DEBUG_INFO, "TgPowerManagementConfiguration () End\n"));
}

/**
  Search for TG and override softstraps if necessasary
**/
STATIC
VOID
TgDynamicDetectionAndConfig (
  VOID
  )
{
  UINT8                               MaxPciePortNum;
  UINT8                               RpIndex;
  UINT8                               ConfigIndex;
  UINT64                              RpBase;
  PCIE_DEVICE_INFO                    DevInfo;
  UINT8                               TempPciBus;
  UINT16                              NumOfControllers;
  BOOLEAN                             TgOverrideSts;
  UINT8                               TgLocation;
  PCH_RESET_DATA                      ResetData;
  EFI_STATUS                          Status;
  TETON_GLACIER_CFG_PPI               *TetonGlacierCfg;
  UINT8                               Index;

  DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig() Start\n"));
  TgLocation = INVALID_ROOT_PORT;
  ConfigIndex = INVALID_ROOT_PORT_CONTROLLER;
  TgOverrideSts = FALSE;
  MaxPciePortNum = GetPchMaxPciePortNum ();
  TempPciBus = PcdGet8 (PcdSiliconInitTempPciBusMin);
  NumOfControllers = GetPchMaxPcieControllerNum ();
  Status = PeiServicesLocatePpi (
             &gTetonGlacierCfgPpiGuid,
             0,
             NULL,
             (VOID **) &TetonGlacierCfg
             );
  if (Status == EFI_NOT_FOUND) {
    DEBUG ((DEBUG_ERROR, "Teton Glacier Configuration Data Ppi Not found.\n"));
    return;
  }
  for (Index = 0; Index < NumOfControllers; Index ++) {
    if (TetonGlacierCfg->ControllerCfg[Index] == PcieOverride2x2) {
      TgOverrideSts = TRUE;
      TgLocation = Index * PCH_PCIE_CONTROLLER_PORTS;
      break;
    }
  }
  CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
  StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
  //checking all Pcie controllers for TG connection
  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex = RpIndex + 4) {
    DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig Checking RpIndex %x\n",(RpIndex + 1)));
    RpBase = PchPcieBase (RpIndex);
    GetDeviceInfo (RpBase, TempPciBus, &DevInfo);
    if (IsHybridStorageDevice (DevInfo.Vid, DevInfo.Did)) {
      DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig found TG\n"));
      if (TgOverrideSts && (RpIndex == TgLocation)) {
        //Teton Glacier found already performing override for the correct controller so exit from loop
        DEBUG ((DEBUG_INFO, "TG found Override being done on correct root port continuing with boot\n"));
        TgPowerManagementConfiguration (RpIndex);
        break;
      } else {
        //Teton Glacier found but location is different from earlier,
        //Send Heci message to perform softstrapoverride on correct controller and exit from loop
        DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig sending set SoftStrap  override message with index %x \n", (RpIndex + 1)));
        if (IsPchH ()) {
          if (RpIndex == 8) {
            ConfigIndex = 2; // Controller3
          } else if (RpIndex == 16) {
            ConfigIndex = 4; // Controller5
          } else if (RpIndex == 20) {
            ConfigIndex = 5; // Controller6
          }
        } else {
          if (RpIndex == 4) {
            ConfigIndex = 1; // Controller2
          } else if (RpIndex == 8) {
            ConfigIndex = 2; // Controller3
          } else if (RpIndex == 12) {
            ConfigIndex = 3; // Controller4
          }
        }
        Status = PeiHeciOverrideSoftStrapMsg (NumOfControllers, ConfigIndex, PcieOverride2x2);
        if (Status == EFI_SUCCESS) {
          DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig TG found for first time overriding SoftStrap and scheduling reset\n"));
          SiScheduleResetSetType (EfiResetPlatformSpecific, &ResetData);
          break;
        }
      }
    } else {
      //Teton Glacier not found on platform, check if SoftStrap override already being done
      if (TgOverrideSts && (RpIndex == TgLocation)) {
        //
        // Teton glacier not present but SoftStrap override being done,
        // Send Heci message to clear SoftStrap override and exit from loop
        //
        Status = PeiHeciOverrideSoftStrapMsg (NumOfControllers, INVALID_ROOT_PORT, PcieOverrideNone);
        if (Status == EFI_SUCCESS) {
          DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig TG removed clearing SoftStrap override and scheduling reset\n"));
          SiScheduleResetSetType (EfiResetPlatformSpecific, &ResetData);
          break;
        }
      } else {
        DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig TG not found on controller and no SoftStrap override being done, continuing with boot\n"));
      }
    }
  }
  DEBUG ((DEBUG_INFO, "TgDynamicDetectionAndConfig() End\n"));
}
/**
  Perform Initialization of the Downstream Root Ports.

  @param[in] SiPolicy             The SI Policy PPI
**/
VOID
PchInitRootPorts (
  IN CONST SI_POLICY_PPI     *SiPolicy
  )
{
  PCH_PCIE_CONFIG            *PciExpressConfig;
  SI_PREMEM_POLICY_PPI       *SiPreMemPolicyPpi;
  EFI_STATUS                 Status;
  UINT8                      RpIndex;
  UINT64                     RpBase;
  UINTN                      RpDevice;
  UINTN                      RpFunction;
  UINT8                      MaxPciePortNum;
  UINT32                     RpDisableMask;
  UINT32                     RpClkreqMask;
  UINT32                     Timer;
  UINT32                     DetectTimeoutUs;
  BOOLEAN                    Gen3DeviceFound[PCH_MAX_PCIE_ROOT_PORTS];
  BOOLEAN                    KeepPortVisible;
  UINT8                      TempPciBusMin;
  UINT8                      TempPciBusMax;
  BOOLEAN                    TbtWorkAround;
  TETON_GLACIER_CONFIG       *TetonGlacierConfig;

  DEBUG ((DEBUG_INFO, "PchInitRootPorts() Start\n"));

  TempPciBusMin = PcdGet8 (PcdSiliconInitTempPciBusMin);
  TempPciBusMax = PcdGet8 (PcdSiliconInitTempPciBusMax);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gPcieRpConfigGuid, (VOID *) &PciExpressConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gTetonGlacierConfigGuid, (VOID *) &TetonGlacierConfig);
  ASSERT_EFI_ERROR(Status);

  DEBUG_CODE_BEGIN ();
  PchPciePolicySanityCheck (PciExpressConfig);
  DEBUG_CODE_END ();

  Timer            = 0;
  MaxPciePortNum   = GetPchMaxPciePortNum ();
  RpDisableMask    = 0;
  RpClkreqMask     = 0;

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);

  PchFiaSetClockOutputDelay ();

  if (PchIsPcieImrEnabled ()) {
    EnablePcieImr (PchGetPcieImrPortNumber (), BUS_NUMBER_FOR_IMR);
  }


  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex++) {
    RpBase = PchPcieBase (RpIndex);

    //
    // Enable CLKREQ# regardless of port being available/enabled to allow clock gating.
    //
    if (IsClkReqAssigned (PchClockUsagePchPcie0 + RpIndex)) {
      RpClkreqMask |= (BIT0 << RpIndex);
    }

    //
    // Determine available ports based on lane ownership and port configuration (x1/x2/x4)
    // Root ports can be already disabled by PchEarlyDisabledDeviceHandling
    //
    if ((PciSegmentRead16 (RpBase) == 0xFFFF) ||
        (IsPciePortOwningLanes (RpBase) == FALSE)) {
      RpDisableMask |= (BIT0 << RpIndex);
      continue;
    }
    //
    // Program preset-coefficient mapping for all Gen3 capable ports that own their lanes
    // This must happen before power management is configured
    //
    PresetToCoefficientMapping (RpBase);

    ///
    /// Set the Slot Implemented Bit.
    /// PCH BIOS Spec section 8.2.3, The System BIOS must
    /// initialize the "Slot Implemented" bit of the PCI Express* Capabilities Register,
    /// XCAP Dxx:Fn:42h[8] of each available and enabled downstream root port.
    /// Ports with hotplug capability must have SI bit set
    /// The register is write-once so must be written even if we're not going to set SI, in order to lock it.
    ///
    /// This must happen before code reads PresenceDetectState, because PDS is invalid unless SI is set
    ///
    if (PciExpressConfig->RootPort[RpIndex].SlotImplemented || PciExpressConfig->RootPort[RpIndex].HotPlug) {
      PciSegmentOr16 (RpBase + R_PCH_PCIE_CFG_XCAP, B_PCIE_XCAP_SI);
    } else {
      PciSegmentAnd16 (RpBase + R_PCH_PCIE_CFG_XCAP, (UINT16)(~B_PCIE_XCAP_SI));
    }

    ///
    /// For non-hotplug ports disable the port if there is no device present.
    ///
    DetectTimeoutUs = PciExpressConfig->RootPort[RpIndex].DetectTimeoutMs * 1000;
    if (PchPcieIsDevicePresent (RpBase, DetectTimeoutUs, &Timer)) {
      DEBUG ((DEBUG_INFO, "Port %d has a device attached.\n", RpIndex + 1));
      //
      // At this point in boot, CLKREQ pad is still configured as GP input and doesnt' block clock generation
      // regardless of input state. Before switching it to native mode when it will start gating clock, we
      // verify if CLKREQ is really connected. If not, pad will not switch and power management
      // will be disabled in rootport.
      // By the time this code runs device can't have CPM or L1 substates enabled, so it is guaranteed to pull ClkReq down.
      // If ClkReq is detected to be high anyway, it means ClkReq is not connected correctly.
      // Checking pad's input value is primarily a measure to prevent problems with long cards inserted into short
      // open-ended PCIe slots on motherboards which route PRSNT signal to CLKREQ. Such config causes CLKREQ signal to float.
      //
      if (!PchPcieDetectClkreq (RpIndex, PciExpressConfig)) {
        RpClkreqMask &= ~(BIT0 << RpIndex);
      }
    } else {
      if (PciExpressConfig->RootPort[RpIndex].HotPlug == FALSE) {
        RpDisableMask |= (BIT0 << RpIndex);
      }
    }
  }

  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex++) {
    if (RpClkreqMask & (BIT0 << RpIndex)) {
      //
      // Enabled CLKREQ# pad if supported to allow clock gating regardless of port being enabled.
      //
      EnableClkReq (PchClockUsagePchPcie0 + RpIndex);
    }

    ///
    /// Configure power management applicable to all port including disabled ports.
    ///
    if (PciExpressConfig->DisableRootPortClockGating == FALSE) {
      if ((RpIndex % PCH_PCIE_CONTROLLER_PORTS) == 0) {
        //
        // TrunkClockGateEn depends on each of the controller ports supporting CLKREQ# or being disabled.
        //
        PchPcieConfigureControllerBasePowerManagement (
          RpIndex,
          (((RpClkreqMask | RpDisableMask) & (0xFu << RpIndex)) == (0xFu << RpIndex))
          );
      }
      //
      // PhyLanePgEnable depends on the port supporting CLKREQ# or being disabled.
      //
      TbtWorkAround = PciExpressConfig->RootPort[RpIndex].PcieRootPortGen2PllL1CgDisable ? TRUE : FALSE;
      PchPcieConfigurePortBasePowerManagement (
        RpIndex,
        (((RpClkreqMask | RpDisableMask) & (BIT0 << RpIndex)) != 0),
        TbtWorkAround
        );
    }
  }

  //
  // Wait for all ports with PresenceDetect=1 to form a link
  // Having an active link is necessary to access and configure the endpoint
  // We cannot use results of PchPcieIsDevicePresent() because it checks PDS only and may include
  // PCIe cards that never form a link, such as compliance load boards.
  //
  WaitForLinkActive (MaxPciePortNum, RpDisableMask);
  ///
  /// For each controller set Initialize Transaction Layer Receiver Control on Link Down
  /// and Initialize Link Layer Receiver Control on Link Down.
  /// Use sideband access in case 1st port of a controller is disabled
  ///
  for (RpIndex = 0; RpIndex < MaxPciePortNum; ++RpIndex) {
    if ((RpIndex % PCH_PCIE_CONTROLLER_PORTS) == 0) {
      PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PCIEALC, ~0u, B_PCH_PCIE_CFG_PCIEALC_ITLRCLD | B_PCH_PCIE_CFG_PCIEALC_ILLRCLD);
    }
  }
  //
  // PTM programming happens per controller
  // Lanes 0-5 have different mPhy type and require different programming than the rest
  // For 2nd controller which is composed of both types of mPhy lanes, the lower values must be used
  //
  RpIndex = 0;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC1, 0x0, 0x240B2B07);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC2, 0x0, 0x1B09200C);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC3, 0x0, 0x170B180B);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC4, 0x0, 0x190B1C09);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC5, (UINT32)(~0xFFFF), 0x190B);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMECFG, (UINT32)(~0x1C0F7F), 0x40052);
  RpIndex = 4;
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC1, 0x0, 0x240B2B07);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC2, 0x0, 0x1B08200C);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC3, 0x0, 0x160B180B);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC4, 0x0, 0x180A1B08);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC5, (UINT32)(~0xFFFF), 0x180C);
  PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMECFG, (UINT32)(~0x1C0F7F), 0x40052);
  for (RpIndex = 8; RpIndex < MaxPciePortNum; RpIndex += PCH_PCIE_CONTROLLER_PORTS) {
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC1, 0x0, 0x250C2C08);
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC2, 0x0, 0x1C08210C);
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC3, 0x0, 0x160C180B);
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC4, 0x0, 0x180A1B08);
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMPSDC5, (UINT32)(~0xFFFF), 0x180C);
    PchSbiRpPciAndThenOr32 (RpIndex, R_PCH_PCIE_CFG_PTMECFG, (UINT32)(~0x1C0F7F), 0x40052);
  }

  for (RpIndex = 0; RpIndex < MaxPciePortNum; ++RpIndex) {
    if (RpDisableMask & (BIT0 << RpIndex)) {
      KeepPortVisible = IsPortForceVisible (RpIndex, RpDisableMask, PciExpressConfig);
      PchDisableRootPort (RpIndex, KeepPortVisible);
    } else {
      PchInitSingleRootPort (
        RpIndex,
        SiPolicy,
        SiPreMemPolicyPpi,
        TempPciBusMin,
        &Gen3DeviceFound[RpIndex]
        );
      ///
      /// Initialize downstream devices
      ///
      GetPchPcieRpDevFun (RpIndex, &RpDevice, &RpFunction);
      RootportDownstreamConfiguration (
        DEFAULT_PCI_SEGMENT_NUMBER_PCH,
        DEFAULT_PCI_BUS_NUMBER_PCH,
        (UINT8) RpDevice,
        (UINT8) RpFunction,
        TempPciBusMin,
        TempPciBusMax
        );
    }
  }
  ///
  /// Clear GPE0 Register PCI_EXP_STS and HOT_PLUG_STS by writing 1
  ///
  IoWrite32 (
    PmcGetAcpiBase () + R_ACPI_IO_GPE0_STS_127_96,
    B_ACPI_IO_GPE0_STS_127_96_PCI_EXP | B_ACPI_IO_GPE0_STS_127_96_HOT_PLUG
    );

  ///
  /// If SCI is enabled in any port, Set BIOS_PCI_EXP_EN bit, PMC PCI offset A0h[10],
  /// to globally enable the setting of the PCI_EXP_STS bit by a PCI Express* PME event.
  ///
  for (RpIndex = 0; RpIndex < MaxPciePortNum; RpIndex++) {
    if (PciExpressConfig->RootPort[RpIndex].PmSci) {
      PmcEnablePciExpressPmeEvents ();
      break;
    }
  }

  ///
  /// PCH BIOS Spec Section 8.2.9
  /// Enable PCIe Relaxed Order to always allow downstream completions to pass posted writes.
  /// To enable this feature configure DMI and PSF:
  ///
  PchDmiEnablePcieRelaxedOrder ();
  PsfEnablePcieRelaxedOrder ();

  ///
  ///Check and configure Teton Glacier
  ///
  if (TetonGlacierConfig->TetonGlacierMode == DYNAMIC_CONFIG) {
    TgDynamicDetectionAndConfig ();
  }
  //
  // Program the root port target link speed based on policy.
  //
  Status = PchPcieRpSpeedChange (SiPolicy, Gen3DeviceFound);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "PchInitRootPorts() End\n"));
}

/**
  This function provides BIOS workaround for WiFi device cannot enter S0i3 state due to LCTL.ECPM bit is cleared.
  This workaound is applied for Intel Wireless-AC 9260(Thunder Peak) and Intel Wireless-AX 22260(Cyclone Peak).
  This function does speed change earlier, so that Endpoint will be in correct state by the time
  RootportDownstreamConfiguration () is executed.

  @param[in] DevInfo Information on device that is connected to rootport
  @param[in] Speed   PCIe root port policy speed setting
  @param[in] RpBase  Root Port base address
**/
STATIC
VOID
WifiLinkSpeedSyncWorkaround (
  IN PCIE_DEVICE_INFO DevInfo,
  IN UINT8            Speed,
  IN UINT64           RpBase
  )
{
  UINTN TimeoutCount;

  if ((DevInfo.Vid == V_PCH_INTEL_VENDOR_ID) &&
      ((DevInfo.Did == 0x2526) || (DevInfo.Did == 0x2723)) &&
      (Speed != PchPcieGen1)) {
    PciSegmentAndThenOr16 (
      RpBase + R_PCH_PCIE_CFG_LCTL2,
      (UINT16) ~B_PCIE_LCTL2_TLS,
      (UINT16) DevInfo.MaxLinkSpeed
      );

    // Retrain the Link
    PciSegmentOr16 (RpBase + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
    // 100 ms timeout while checking for link training is completed.
    for (TimeoutCount = 0; TimeoutCount < 1000; TimeoutCount++) {
      // Delay 100 us
      MicroSecondDelay (100);
      if ((PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LT) == 0) {
        break;
      }
    }
    // 100 ms timeout while checking for link active on retrained link
    for (TimeoutCount = 0; TimeoutCount < 1000; TimeoutCount++) {
      // Delay 100 us
      MicroSecondDelay (100);
      if (PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LA) {
        break;
      }
    }
  }
}

/**
  Performs mandatory Root Port Initialization.
  This function is silicon-specific and configures proprietary registers.

  @param[in]  PortIndex               The root port to be initialized (zero based)
  @param[in]  SiPolicy                The SI Policy PPI
  @param[in]  SiPreMemPolicyPpi       The SI PreMem Policy PPI
  @param[in]  TempPciBus              The temporary Bus number for root port initialization
  @param[out] Gen3DeviceFound         Reports if there's Gen3 capable endpoint connected to this rootport
**/
VOID
PchInitSingleRootPort (
  IN  UINT8                                     PortIndex,
  IN  CONST SI_POLICY_PPI                       *SiPolicy,
  IN  SI_PREMEM_POLICY_PPI                      *SiPreMemPolicyPpi,
  IN  UINT8                                     TempPciBus,
  OUT BOOLEAN                                   *Gen3DeviceFound
  )
{
  EFI_STATUS                        Status;
  UINT64                            RpBase;
  UINT32                            Data32Or;
  UINT32                            Data32And;
  UINT16                            Data16;
  UINT16                            Data16Or;
  UINT16                            Data16And;
  UINT8                             Data8Or;
  UINT8                             Data8And;
  PCH_PCIE_CONFIG                   *PcieRpConfig;
  CONST PCH_PCIE_ROOT_PORT_CONFIG   *RootPortConfig;
  VTD_CONFIG                        *VtdConfig;
  UINT32                            Tls;
  PCIE_DEVICE_INFO                  DevInfo;
  UINT32                            Uptp;
  UINT32                            Dptp;
  UINT8                             RpLinkSpeed;
  UINT32                            RpMaxPayloadCapability;
  UINT8                             InterruptPin;
  UINTN                             RpDevice;
  UINTN                             RpFunction;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gVtdConfigGuid, (VOID *) &VtdConfig);
  ASSERT_EFI_ERROR (Status);

  RootPortConfig  = &PcieRpConfig->RootPort[PortIndex];

  DEBUG ((DEBUG_INFO, "PchInitSingleRootPort (%d) Start \n", PortIndex + 1));
  RpBase = PchPcieBase (PortIndex);
  GetPchPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);

  Tls = PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LCTL2) & B_PCIE_LCTL2_TLS;
  ASSERT (Tls < V_PCIE_LCTL2_TLS_GEN3);

  /// PCH BIOS Spec Section 8.2.10 Completion Retry Status Replay Enable
  /// Following reset it is possible for a device to terminate the
  /// configuration request but indicate that it is temporarily unable to process it,
  /// but in the future. The device will return the Configuration Request Retry Status.
  /// By setting the Completion Retry Status Replay Enable, Dxx:Fn + 320h[22],
  /// the RP will re-issue the request on receiving such status.
  /// The BIOS shall set this bit before first configuration access to the endpoint.
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_PCIECFG2, B_PCH_PCIE_CFG_PCIECFG2_CRSREN);
  //
  // Set speed capability in rootport
  //
  Data8And = (UINT8)(~((UINT8)(B_PCH_PCIE_CFG_MPC_PCIESD >> 8)));
  Data8Or = 0;


  switch (RootPortConfig->PcieSpeed) {
    case PchPcieGen1:
      Data8Or |= (V_PCH_PCIE_CFG_MPC_PCIESD_GEN1 << (N_PCH_PCIE_CFG_MPC_PCIESD - 8));
      break;
    case PchPcieGen2:
      Data8Or |= (V_PCH_PCIE_CFG_MPC_PCIESD_GEN2 << (N_PCH_PCIE_CFG_MPC_PCIESD - 8));
      break;
    case PchPcieGen3:
    case PchPcieAuto:
      break;
  }
  PciSegmentAndThenOr8 (RpBase + R_PCH_PCIE_CFG_MPC + 1, Data8And, Data8Or);

  GetDeviceInfo (RpBase, TempPciBus, &DevInfo);
  *Gen3DeviceFound = ((GetMaxLinkSpeed (RpBase) >= 3) && (DevInfo.MaxLinkSpeed >= 3));

  if ( NeedDecreasedDeEmphasis (DevInfo)) {
    PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_LCTL2, B_PCIE_LCTL2_SD);
  }
  WifiLinkSpeedSyncWorkaround (DevInfo, RootPortConfig->PcieSpeed, RpBase);
  ///
  /// If only 128B max payload is supported set CCFG.UNRS to 0.
  ///
  /// If peer writes are supported set max payload size supported to 128B, clear CCFG.UPMWPD
  /// and program all the PCH Root Ports such that upstream posted writes and upstream non-posted requests
  /// are split at 128B boundary by setting CCFG fields: UPSD to 0, CCFG.UPRS to 000b and UNSD to 0, UNRS to 000b
  ///
  Data32And = ~0u;
  Data32Or  = 0;
  RpMaxPayloadCapability = PchPcieMaxPayload256;
  if (RootPortConfig->MaxPayload == PchPcieMaxPayload128 ||
      PcieRpConfig->EnablePeerMemoryWrite) {
    RpMaxPayloadCapability = PchPcieMaxPayload128;

    Data32And &= (UINT32) ~(B_PCH_PCIE_CFG_CCFG_UNSD | B_PCH_PCIE_CFG_CCFG_UNRS);
    Data32Or  |= (UINT32)  (V_PCH_PCIE_CFG_CCFG_UNRS_128B << N_PCH_PCIE_CFG_CCFG_UNRS);

    if (PcieRpConfig->EnablePeerMemoryWrite) {
      Data32And &= (UINT32) ~(B_PCH_PCIE_CFG_CCFG_UPMWPD |
                              B_PCH_PCIE_CFG_CCFG_UPSD | B_PCH_PCIE_CFG_CCFG_UPRS);
      Data32Or  |= (UINT32)  (V_PCH_PCIE_CFG_CCFG_UPRS_128B << N_PCH_PCIE_CFG_CCFG_UPRS);
    }
  }
  ASSERT (RootPortConfig->MaxPayload < PchPcieMaxPayloadMax);
  ///
  /// Set B0:Dxx:Fn + D0h [13:12] to 01b
  ///
  Data32And &= (UINT32) ~B_PCH_PCIE_CFG_CCFG_UNRD;
  Data32Or  |=  (1u << N_PCH_PCIE_CFG_CCFG_UNRD);

  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_CCFG, Data32And, Data32Or);

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_DCAP, (UINT16) ~B_PCIE_DCAP_MPS, (UINT16)RpMaxPayloadCapability);


  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// If B0:Dxx:Fn + 400h is programmed, BIOS will also program B0:Dxx:Fn + 404h [1:0] = 11b,
  /// to enable these override values.
  /// - Fn refers to the function number of the root port that has a device attached to it.
  /// - Default override value for B0:Dxx:Fn + 400h should be 880F880Fh
  /// - Also set 404h[2] to lock down the configuration
  /// - Refer to table below for the 404h[3] policy bit behavior.
  /// Done in PcieSetPm()
  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// Program B0:Dxx:Fn + 64h [11] = 1b
  ///
  Data32Or = 0;
  Data32And = ~0u;
  if (RootPortConfig->LtrEnable == TRUE) {
    Data32Or |= B_PCIE_DCAP2_LTRMS;
  } else {
    Data32And &= (UINT32) ~(B_PCIE_DCAP2_LTRMS);
  }
  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// Optimized Buffer Flush/Fill (OBFF) is not supported.
  /// Program B0:Dxx:Fn + 64h [19:18] = 0h
  ///
  Data32And &= (UINT32) ~B_PCIE_DCAP2_OBFFS;
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_DCAP2, Data32And, Data32Or);
  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// Program B0:Dxx:Fn + 68h [10] = 1b
  ///
  Data16 = PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_DCTL2);
  if (RootPortConfig->LtrEnable == TRUE) {
    Data16 |= B_PCIE_DCTL2_LTREN;
  } else {
    Data16 &= (UINT16) ~(B_PCIE_DCTL2_LTREN);
  }
  PciSegmentWrite16 (RpBase + R_PCH_PCIE_CFG_DCTL2, Data16);

  ///
  /// PCH BIOS Spec Section 8.15.1 Power Optimizer Configuration
  /// Step 3 done in PchPciExpressHelpersLibrary.c ConfigureLtr
  ///

  ///
  /// Set Dxx:Fn + 300h[23:00] = 0B75FA7h
  /// Set Dxx:Fn + 304h[11:00] = 0C97h
  ///
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_PCIERTP1, ~0x00FFFFFFu, 0x00B75FA7);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_PCIERTP2, ~0x00000FFFu, 0x00000C97);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set Dxx:Fn + 318h [31:16] = 1414h (Gen2 and Gen1 Active State L0s Preparation Latency)
  ///
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_PCIEL0SC, ~0xFFFF0000u, 0x14140000);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// 1.  Program Dxx:Fn + E8h[20, 1] to [1, 1]
  ///
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_PWRCTL, B_PCH_PCIE_CFG_PWRCTL_LTSSMRTC |
                  B_PCH_PCIE_CFG_PWRCTL_RPL1SQPOL);
  ///
  /// 2.  Program Dxx:Fn + 320h[27, 30] to [1,1]
  /// Enable PCIe Relaxed Order to always allow downstream completions to pass posted writes,
  /// 3.  Set B0:Dxx:Fn:320h[24:23] = 11b
  /// Set PME timeout to 10ms, by
  /// 4.  Set B0:Dxx:Fn:320h[21:20] = 01b
  ///

  Data32And = (UINT32) ~B_PCH_PCIE_CFG_PCIECFG2_PMET;
  Data32Or  = B_PCH_PCIE_CFG_PCIECFG2_LBWSSTE |
    B_PCH_PCIE_CFG_PCIECFG2_RLLG3R |
    B_PCH_PCIE_CFG_PCIECFG2_CROAOV |
    B_PCH_PCIE_CFG_PCIECFG2_CROAOE |
    (V_PCH_PCIE_CFG_PCIECFG2_PMET << N_PCH_PCIE_CFG_PCIECFG2_PMET);

  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_PCIECFG2, Data32And, Data32Or);

  ///
  /// Enable squelch by programming Dxx:Fn + 324h[25, 24, 5] to [0, 0, 1]
  /// Enable Completion Time-Out Non-Fatal Advisory Error, Dxx:Fn + 324h[14] = 1b
  ///
  ///
  Data32And = (UINT32) ~(B_PCH_PCIE_CFG_PCIEDBG_LGCLKSQEXITDBTIMERS);
  Data32Or  = B_PCH_PCIE_CFG_PCIEDBG_CTONFAE | B_PCH_PCIE_CFG_PCIEDBG_SPCE;
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_PCIEDBG, Data32And, Data32Or);

  ///
  ///
  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Program Dxx:Fn + 424h [6, 5, 4] = [1, 1, 1]
  ///
  PciSegmentOr32 (
    RpBase + R_PCH_PCIE_CFG_PCIEPMECTL2,
    (B_PCH_PCIE_CFG_PCIEPMECTL2_L23RDYSCPGE |
     B_PCH_PCIE_CFG_PCIEPMECTL2_L1SCPGE)
    );
  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// If Dxx:Fn + F5h[0] = 1b or step 3 is TRUE, set Dxx:Fn + 4Ch[17:15] = 4h
  /// Else set Dxx:Fn + 4Ch[17:15] = 010b
  ///
  Data32And = (UINT32) (~B_PCIE_LCAP_EL1);
  Data32Or = 4 << N_PCIE_LCAP_EL1;

  ///
  /// Set LCAP APMS according to platform policy.
  ///
  if (RootPortConfig->Aspm < PchPcieAspmAutoConfig) {
    Data32And &= (UINT32) ~B_PCIE_LCAP_APMS;
    Data32Or  |= RootPortConfig->Aspm << N_PCIE_LCAP_APMS;
  } else {
    Data32Or  |= B_PCIE_LCAP_APMS_L0S | B_PCIE_LCAP_APMS_L1;
  }

  if (IsCnlPch () && IsPchLp () && (PchStepping () == PCH_B1)) {
    Data32And &= (UINT32) ~B_PCIE_LCAP_APMS_L0S;
    Data32Or  &= (UINT32) ~B_PCIE_LCAP_APMS_L0S;
  }
  //
  // The EL1, ASPMOC and APMS of LCAP are RWO, must program all together.
  //
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_LCAP, Data32And, Data32Or);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Configure PCI Express Number of Fast Training Sequence for each port
  /// 1.  Set Dxx:Fn + 314h [31:24, 23:16, 15:8, 7:0] to [7Eh, 70h, 3Fh, 38h]
  /// 2.  Set Dxx:Fn + 478h [15:8, 7:0] to [3Dh, 2Ch]
  ///
  PciSegmentWrite32 (RpBase + R_PCH_PCIE_CFG_PCIENFTS, 0x7E703F38);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_G3L0SCTL, ~0x0000FFFFu, 0x00003D2C);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set MPC.IRRCE, Dxx:Fn + D8h[25] = 1b using byte access
  /// For system that support MCTP over PCIE set
  /// Set PCIE RP PCI offset D8h[27] = 1b
  /// Set PCIE RP PCI offset D8h[3] = 1b
  ///
  Data8And = (UINT8) (~(B_PCH_PCIE_CFG_MPC_IRRCE | B_PCH_PCIE_CFG_MPC_MMBNCE) >> 24);
  Data8Or = B_PCH_PCIE_CFG_MPC_MMBNCE >> 24;
  if (VtdConfig->VtdDisable) {
    Data8Or |= B_PCH_PCIE_CFG_MPC_IRRCE >> 24;
  }
  PciSegmentAndThenOr8 (RpBase + R_PCH_PCIE_CFG_MPC + 3, Data8And, Data8Or);

  Data8And = (UINT8) ~(B_PCH_PCIE_CFG_MPC_MCTPSE);
  Data8Or  = B_PCH_PCIE_CFG_MPC_MCTPSE;
  PciSegmentAndThenOr8 (RpBase + R_PCH_PCIE_CFG_MPC, Data8And, Data8Or);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set PCIE RP PCI offset F5h[7:4] = 0000b
  ///
  PciSegmentAnd8 (RpBase + R_PCH_PCIE_CFG_PHYCTL2, (UINT8) ~(B_PCH_PCIE_CFG_PHYCTL2_TDFT | B_PCH_PCIE_CFG_PHYCTL2_TXCFGCHGWAIT));

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Enable PME_TO Time-Out Policy, Dxx:Fn + E2h[6] =1b
  ///
  PciSegmentOr8 (RpBase + R_PCH_PCIE_CFG_RPPGEN, B_PCH_PCIE_CFG_RPPGEN_PTOTOP);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Configure Transmitter Preset for each Upstream and Downstream Port Lane:
  /// 1.  Set L01EC.DPL0TP, Dxx:Fn + 22Ch[3:0]    = 7
  /// 2.  Set L01EC.UPL0TP, Dxx:Fn + 22Ch[11:8]   = 5
  /// 3.  Set L01EC.DPL1TP, Dxx:Fn + 22Ch[19:16]  = 7
  /// 4.  Set L01EC.UPL1TP, Dxx:Fn + 22Ch[27:24]  = 5
  /// 5.  Set L23EC.DPL2TP, Dxx:Fn + 230h[3:0]    = 7
  /// 6.  Set L23EC.UPL2TP, Dxx:Fn + 230h[11:8]   = 5
  /// 7.  Set L23EC.DPL3TP, Dxx:Fn + 230h[19:16]  = 7
  /// 8.  Set L23EC.UPL3TP, Dxx:Fn + 230h[27:24]  = 5
  ///
  Uptp = RootPortConfig->Uptp;
  Dptp = RootPortConfig->Dptp;
  Data32And = (UINT32) ~(B_PCIE_EX_L01EC_UPL1TP | B_PCIE_EX_L01EC_DPL1TP | B_PCIE_EX_L01EC_UPL0TP | B_PCIE_EX_L01EC_DPL0TP);
  Data32Or = ((Uptp << N_PCIE_EX_L01EC_UPL1TP) |
              (Dptp << N_PCIE_EX_L01EC_DPL1TP) |
              (Uptp << N_PCIE_EX_L01EC_UPL0TP) |
              (Dptp << N_PCIE_EX_L01EC_DPL0TP));
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_EX_L01EC, Data32And, Data32Or);

  Data32And = (UINT32) ~(B_PCIE_EX_L23EC_UPL3TP | B_PCIE_EX_L23EC_DPL3TP | B_PCIE_EX_L23EC_UPL2TP | B_PCIE_EX_L23EC_DPL2TP);
  Data32Or = ((Uptp << N_PCIE_EX_L23EC_UPL3TP) |
              (Dptp << N_PCIE_EX_L23EC_DPL3TP) |
              (Uptp << N_PCIE_EX_L23EC_UPL2TP) |
              (Dptp << N_PCIE_EX_L23EC_DPL2TP));
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_EX_L23EC, Data32And, Data32Or);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Enable EQ TS2 in Recovery Receiver Config, Dxx:Fn + 450h[7]= 1b
  ///
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EQCFG1, B_PCH_PCIE_CFG_EQCFG1_EQTS2IRRC);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// If there is no IOAPIC behind the root port, set EOI Forwarding Disable bit (PCIE RP PCI offset D4h[1]) to 1b.
  /// For Empty Hot Plug Slot, set is done in PchInitSingleRootPort ()
  ///

  ///
  /// System bios should initiate link retrain for all slots that has card populated after register restoration.
  /// Done in PchPciExpressHelpersLibrary.c PchPcieInitRootPortDownstreamDevices ()
  ///

  ///
  /// Configure Extended Synch
  ///
  if (RootPortConfig->ExtSync) {
    Data16And = (UINT16) ~0;
    Data16Or  = B_PCIE_LCTL_ES;
  } else {
    Data16And = (UINT16) (~B_PCIE_LCTL_ES);
    Data16Or  = 0;
  }
  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_LCTL, Data16And, Data16Or);

  ///
  /// Configure Completion Timeout
  ///
  Data16And = (UINT16) ~(B_PCIE_DCTL2_CTD | B_PCIE_DCTL2_CTV);
  Data16Or  = 0;
  if (RootPortConfig->CompletionTimeout == PchPcieCompletionTO_Disabled) {
    Data16Or = B_PCIE_DCTL2_CTD;
  } else {
    switch (RootPortConfig->CompletionTimeout) {
      case PchPcieCompletionTO_Default:
        Data16Or = V_PCIE_DCTL2_CTV_DEFAULT;
        break;

      case PchPcieCompletionTO_16_55ms:
        Data16Or = V_PCIE_DCTL2_CTV_40MS_50MS;
        break;

      case PchPcieCompletionTO_65_210ms:
        Data16Or = V_PCIE_DCTL2_CTV_160MS_170MS;
        break;

      case PchPcieCompletionTO_260_900ms:
        Data16Or = V_PCIE_DCTL2_CTV_400MS_500MS;
        break;

      case PchPcieCompletionTO_1_3P5s:
        Data16Or = V_PCIE_DCTL2_CTV_1P6S_1P7S;
        break;

      default:
        Data16Or = 0;
        break;
    }
  }

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_DCTL2, Data16And, Data16Or);

  ///
  /// For Root Port Slots Numbering on the CRBs.
  ///
  Data32Or  = 0;
  Data32And = (UINT32) (~(B_PCIE_SLCAP_SLV | B_PCIE_SLCAP_SLS | B_PCIE_SLCAP_PSN));
  ///
  /// PCH BIOS Spec section 8.8.2.1
  /// Note: If Hot Plug is supported, then write a 1 to the Hot Plug Capable (bit6) and Hot Plug
  /// Surprise (bit5) in the Slot Capabilities register, PCIE RP PCI offset 54h. Otherwise,
  /// write 0 to the bits PCIe Hot Plug SCI Enable
  ///
  Data32And &= (UINT32) (~(B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS));
  if (RootPortConfig->HotPlug) {
    Data32Or |= B_PCIE_SLCAP_HPC | B_PCIE_SLCAP_HPS;
  }
  ///
  /// Get the width from LCAP
  /// Slot Type  X1  X2/X4/X8/X16
  /// Default     10W   25W
  /// The slot power consumption and allocation is platform specific. Please refer to the
  /// "PCI Express* Card Electromechanical (CEM) Spec" for details.
  ///
  if (RootPortConfig->SlotPowerLimitValue != 0) {
    Data32Or |= (UINT32) (RootPortConfig->SlotPowerLimitValue << N_PCIE_SLCAP_SLV);
    Data32Or |= (UINT32) (RootPortConfig->SlotPowerLimitScale << N_PCIE_SLCAP_SLS);
  } else {
    if (GetMaxLinkWidth (RpBase) == 1) {
      Data32Or |= (UINT32) (100 << N_PCIE_SLCAP_SLV);
      Data32Or |= (UINT32) (1 << N_PCIE_SLCAP_SLS);
    } else if (GetMaxLinkWidth (RpBase) >= 2) {
      Data32Or |= (UINT32) (250 << N_PCIE_SLCAP_SLV);
      Data32Or |= (UINT32) (1 << N_PCIE_SLCAP_SLS);
    }
  }

  ///
  /// PCH BIOS Spec section 8.2.4
  /// Initialize Physical Slot Number for Root Ports
  ///
  Data32Or |= (UINT32) (RootPortConfig->PhysicalSlotNumber << N_PCIE_SLCAP_PSN);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_SLCAP, Data32And, Data32Or);

  InitCapabilityList (PortIndex, RpBase, RootPortConfig);

  //
  // All actions involving LinkDisable must finish before anything is programmed on endpoint side
  // because LinkDisable resets endpoint
  //

  ///
  /// Perform equalization for Gen3 capable ports
  ///
  if (GetMaxLinkSpeed (RpBase) >= 3) {
    Gen3LinkEqualize (PortIndex, SiPolicy, TempPciBus, *Gen3DeviceFound);
  }
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set "Link Speed Training Policy", Dxx:Fn + D4h[6] to 1.
  /// Make sure this is after mod-PHY related programming is completed.
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_MPC2, B_PCH_PCIE_CFG_MPC2_LSTP);

  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Step 29 If Transmitter Half Swing is enabled, program the following sequence
  /// a. Ensure that the link is in L0.
  /// b. Program the Link Disable bit (0x50[4]) to 1b.
  /// c. Program the Analog PHY Transmitter Voltage Swing bit (0xE8[13]) to set the transmitter swing to half/full swing
  /// d. Program the Link Disable bit (0x50[4]) to 0b.
  /// BIOS can only enable this on SKU where GEN3 capability is disabled on that port
  RpLinkSpeed   = PciSegmentRead8 (RpBase + R_PCH_PCIE_CFG_LCAP) & B_PCIE_LCAP_MLS;
  if (RpLinkSpeed < V_PCIE_LCAP_MLS_GEN3 && RootPortConfig->TransmitterHalfSwing) {
    PciSegmentOr8 (RpBase + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_LD);
    while (IsLinkActive (RpBase)) {
      // wait until link becomes inactive before changing swing
    }
    PciSegmentOr16 (RpBase + R_PCH_PCIE_CFG_PWRCTL, B_PCH_PCIE_CFG_PWRCTL_TXSWING);
    PciSegmentAnd8 (RpBase + R_PCH_PCIE_CFG_LCTL, (UINT8) ~(B_PCIE_LCTL_LD));
  }
  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set "Poisoned TLP Non-Fatal Advisory Error Enable", Dxx:Fn + D4h[12] to 1
  ///
  Data32Or = B_PCH_PCIE_CFG_MPC2_PTNFAE;
  PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_MPC2, Data32Or);

  //
  // L1LOW LTR threshold latency value
  //
  PciSegmentAndThenOr32 (
    RpBase + R_PCH_PCIE_CFG_PCIEPMECTL,
    (UINT32) ~B_PCH_PCIE_CFG_PCIEPMECTL_L1LTRTLV,
    (V_PCH_PCIE_CFG_PCIEPMECTL_L1LTRTLV << N_PCH_PCIE_CFG_PCIEPMECTL_L1LTRTLV)
    );

  ///
  /// Additional configurations
  ///
  ///
  /// Configure Error Reporting policy in the Device Control Register
  ///
  Data16And = (UINT16) (~(B_PCIE_DCTL_URE | B_PCIE_DCTL_FEE | B_PCIE_DCTL_NFE | B_PCIE_DCTL_CEE));
  Data16Or  = 0;

  if (RootPortConfig->UnsupportedRequestReport) {
    Data16Or |= B_PCIE_DCTL_URE;
  }

  if (RootPortConfig->FatalErrorReport) {
    Data16Or |= B_PCIE_DCTL_FEE;
  }

  if (RootPortConfig->NoFatalErrorReport) {
    Data16Or |= B_PCIE_DCTL_NFE;
  }

  if (RootPortConfig->CorrectableErrorReport) {
    Data16Or |= B_PCIE_DCTL_CEE;
  }

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_DCTL, Data16And, Data16Or);

  ///
  /// Configure Interrupt / Error reporting in R_PCH_PCIE_CFG_RCTL
  ///
  Data16And = (UINT16) (~(B_PCIE_RCTL_SFE | B_PCIE_RCTL_SNE | B_PCIE_RCTL_SCE));
  Data16Or  = 0;

  if (RootPortConfig->SystemErrorOnFatalError) {
    Data16Or |= B_PCIE_RCTL_SFE;
  }

  if (RootPortConfig->SystemErrorOnNonFatalError) {
    Data16Or |= B_PCIE_RCTL_SNE;
  }

  if (RootPortConfig->SystemErrorOnCorrectableError) {
    Data16Or |= B_PCIE_RCTL_SCE;
  }

  PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_RCTL, Data16And, Data16Or);

  ///
  /// Root PCI-E Powermanagement SCI Enable
  ///
  if (RootPortConfig->PmSci) {
    ///
    /// PCH BIOS Spec section 8.7.3 BIOS Enabling of Intel PCH PCI Express* PME SCI Generation
    /// Step 1
    /// Make sure that PME Interrupt Enable bit, Dxx:Fn:Reg 5Ch[3] is cleared
    ///
    PciSegmentAnd16 (RpBase + R_PCH_PCIE_CFG_RCTL, (UINT16) (~B_PCIE_RCTL_PIE));

    ///
    /// Step 2
    /// Program Misc Port Config (MPC) register at PCI config space offset
    /// D8h as follows:
    /// Set Power Management SCI Enable bit, Dxx:Fn:Reg D8h[31]
    /// Clear Power Management SMI Enable bit, Dxx:Fn:Reg D8h[0]
    /// Use Byte Access to avoid RWO bit
    ///
    PciSegmentAnd8 (RpBase + R_PCH_PCIE_CFG_MPC, (UINT8) (~B_PCH_PCIE_CFG_MPC_PMME));
    PciSegmentOr8 ((RpBase + R_PCH_PCIE_CFG_MPC + 3), (UINT8) (B_PCH_PCIE_CFG_MPC_PMCE >> 24));
  }
  if (RootPortConfig->HotPlug) {
    ///
    /// PCH BIOS Spec section 8.8.2.1
    /// Step 1
    /// Clear following status bits, by writing 1b to them, in the Slot
    /// Status register at offset 1Ah of PCI Express Capability structure:
    /// Presence Detect Changed (bit3)
    ///
    PciSegmentAnd16 (RpBase + R_PCH_PCIE_CFG_SLSTS, B_PCIE_SLSTS_PDC);
    ///
    /// Step 2
    /// Program the following bits in Slot Control register at offset 18h
    /// of PCI Express* Capability structure:
    /// Presence Detect Changed Enable (bit3) = 1b
    /// Hot Plug Interrupt Enable (bit5) = 0b
    ///
    PciSegmentAndThenOr16 (RpBase + R_PCH_PCIE_CFG_SLCTL, (UINT16) (~B_PCIE_SLCTL_HPE), B_PCIE_SLCTL_PDE);
    ///
    /// Step 3
    /// Program Misc Port Config (MPC) register at PCI config space offset
    /// D8h as follows:
    /// Hot Plug SCI Enable (HPCE, bit30) = 1b
    /// Hot Plug SMI Enable (HPME, bit1) = 0b
    /// Use byte access to avoid premature locking BIT23, SRL
    ///
    PciSegmentAnd8 (RpBase + R_PCH_PCIE_CFG_MPC, (UINT8)(~B_PCH_PCIE_CFG_MPC_HPME) );
    PciSegmentOr8 (RpBase + R_PCH_PCIE_CFG_MPC + 3, B_PCH_PCIE_CFG_MPC_HPCE >> 24);
    ///
    /// PCH BIOS Spec section 8.9
    /// BIOS should mask the reporting of Completion timeout (CT) errors by setting
    /// the uncorrectable Error Mask register PCIE RP PCI offset 108h[14].
    ///
    PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EX_UEM, B_PCIE_EX_UEM_CT);
  }

  ///
  /// PCH BIOS Spec Section 8.10 PCI Bus Emulation & Port80 Decode Support
  /// The I/O cycles within the 80h-8Fh range can be explicitly claimed
  /// by the PCIe RP by setting MPC.P8XDE, PCI offset D8h[26] = 1 (using byte access)
  /// BIOS must also configure the corresponding DMI registers GCS.RPR and GCS.RPRDID
  /// to enable DMI to forward the Port8x cycles to the corresponding PCIe RP
  ///
  if (PcieRpConfig->EnablePort8xhDecode && (PortIndex == (UINT8)PcieRpConfig->PchPciePort8xhDecodePortIndex)) {
    PciSegmentOr8 (RpBase + R_PCH_PCIE_CFG_MPC + 3, (UINT8) (B_PCH_PCIE_CFG_MPC_P8XDE >> 24));
    PchIoPort80DecodeSet (PortIndex);
  }
  //
  // Initialize R/WO Registers that described in PCH BIOS Spec
  //
  ///
  /// SRL bit is write-once and lock, so SRL, UCEL and CCEL must be programmed together
  /// otherwise UCEL/CCEL programming would lock SRL prematurely in wrong state
  ///
  /// PCH BIOS Spec Section 8.15 Additional PCI Express* Programming Steps
  /// Set Common Clock Exit Latency,      Dxx:Fn + D8h[17:15] = 4h
  /// Set Non-common Clock Exit Latency,  Dxx:Fn + D8h[20:18] = 7h
  ///
  Data32And = ~(UINT32) (B_PCH_PCIE_CFG_MPC_UCEL | B_PCH_PCIE_CFG_MPC_CCEL);
  Data32Or  = (7 << N_PCH_PCIE_CFG_MPC_UCEL) | (4<< N_PCH_PCIE_CFG_MPC_CCEL);


  Data32Or |= B_PCH_PCIE_CFG_MPC_SRL;

  PciSegmentAndThenOr32(RpBase + R_PCH_PCIE_CFG_MPC, Data32And, Data32Or);

  //
  // Check if SRL bit actually got programmed. If not, then it means some code accessed MPC register earlier and locked it
  //
  ASSERT((PciSegmentRead32(RpBase + R_PCH_PCIE_CFG_MPC) & B_PCH_PCIE_CFG_MPC_SRL) == B_PCH_PCIE_CFG_MPC_SRL);

  if (PcieRpConfig->RootPort[PortIndex].PcieRootPortGen2PllL1CgDisable == 1) {
    PciSegmentAnd32 (RpBase + R_PCH_PCIE_CFG_PCIEPMECTL2, (UINT32)~(B_PCH_PCIE_CFG_PCIEPMECTL2_L1SCPGE));
  }

  ///
  /// Configure Root Port interrupt
  ///
  InterruptPin = ItssGetDevIntPin (SiPolicy, (UINT8)RpDevice, (UINT8)RpFunction);

  Data32And = (UINT32) ~B_PCH_PCIE_CFG_STRPFUSECFG_PXIP;
  Data32Or = (UINT32) (InterruptPin << N_PCH_PCIE_CFG_STRPFUSECFG_PXIP);
  PciSegmentAndThenOr32 (RpBase + R_PCH_PCIE_CFG_STRPFUSECFG, Data32And, Data32Or);
}

/**
  Hide rootports disabled by policy. This needs to be done in premem,
  because graphics init from SystemAgent code depends on those ports
  being already hidden

  @param[in] PcieRpPreMemConfig   Platform policy
**/
VOID
EarlyPcieRpDisabling (
  IN PCH_PCIE_RP_PREMEM_CONFIG *PcieRpPreMemConfig
  )
{
  UINT32 RpIndex;
  for (RpIndex = 0; RpIndex < GetPchMaxPciePortNum (); RpIndex++) {
    //
    // Disable RP 20-23 for CNL PCH-H A0 Stepping
    //
    if (IsCnlPch () && IsPchH () && (PchStepping() == PCH_A0) && (RpIndex >= 20) && (RpIndex <= 23)) {
      PsfDisablePcieRootPort (RpIndex);
      continue;
    }
    if ((PcieRpPreMemConfig->RpEnabledMask & (UINT32) (1 << RpIndex)) == 0) {
      PsfDisablePcieRootPort (RpIndex);
    }
  }
}

/**
  Initializes ports with NonCommonClock configuration. Such ports can't detect endpoints
  before NCC init ends. To prevent boot delays, NCC handling should happen in pre-mem
  rather than just before endpoint detection in post-mem
**/
VOID
EarlyPcieNccHandling (
  VOID
  )
{
  UINT32 RpIndex;

  for (RpIndex = 0; RpIndex < GetPchMaxPciePortNum (); RpIndex++) {
    if (IsPcieNcc (PchPcieBase (RpIndex))) {
      PcieInitNccPort (RpIndex);
    }
  }
}

/**
  Configure PCIe Grant Counts
**/
VOID
PchConfigurePcieGrantCounts (
  VOID
  )
{
   UINT32                ControllerMax;
   UINT32                Controller;
   PSF_PCIE_CTRL_CONFIG  PsfPcieCtrlConfigTable[PCH_MAX_PCIE_CONTROLLERS];

   ControllerMax = GetPchMaxPcieControllerNum ();

  for (Controller = 0; Controller < ControllerMax; Controller++) {
    switch (GetPcieControllerConfig (Controller)) {
      case Pcie4x1: //Pcie4x1
        PsfPcieCtrlConfigTable[Controller] = PsfPcieCtrl4x1;
        break;
      case Pcie1x2_2x1: //Pcie1x2_2x1
        PsfPcieCtrlConfigTable[Controller] = PsfPcieCtrl1x2_2x1;
        break;
      case Pcie2x2: //Pcie2x2
        PsfPcieCtrlConfigTable[Controller] = PsfPcieCtrl2x2;
        break;
      case Pcie1x4: //Pcie1x4
        PsfPcieCtrlConfigTable[Controller] = PsfPcieCtrl1x4;
        break;
      default:
        ASSERT (FALSE);
    }
  }

  PsfConfigurePcieGrantCounts (PsfPcieCtrlConfigTable, ControllerMax);
}
