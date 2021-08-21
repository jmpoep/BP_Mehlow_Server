/** @file
  This file will perform SA PCIE Root Complex initialization.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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
#include <Private/Library/SaPcieInitLib.h>
#include <Library/PciSegmentLib.h>
#include <PchAccess.h>
#include <CpuRegs.h>
#include <CpuAccess.h>
#include <Library/CpuPlatformLib.h>
#include <Library/SaPlatformLib.h>
#include <Ppi/SiPolicy.h>
#include <Library/PeiServicesLib.h>
#include <Library/ConfigBlockLib.h>
#include <ConfigBlock/PchGeneralConfig.h>

///
/// Global variables
///
typedef struct {
  UINT8 Bus;
  UINT8 Device;
  UINT8 Function;
  UINT8 Slot;
  UINT8 Bus2;
  UINT8 Device2;
  UINT8 Function2;
} PEG_PORT_DEVICE;

GLOBAL_REMOVE_IF_UNREFERENCED PEG_PORT_DEVICE PegDeviceTable[] = {
  ///
  ///          Bus,           Device,         Function, Slot, Bus2, Device2, Function2
  ///
  { SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM,    1,    2,       0,         0 },
  { SA_PEG_BUS_NUM, SA_PEG1_DEV_NUM, SA_PEG1_FUN_NUM,    2,    3,       0,         0 },
  { SA_PEG_BUS_NUM, SA_PEG2_DEV_NUM, SA_PEG2_FUN_NUM,    3,    4,       0,         0 },
  { SA_PEG_BUS_NUM, SA_PEG3_DEV_NUM, SA_PEG3_FUN_NUM,    4,    5,       0,         0 }
};

///
/// Functions
///
/**
    Perform Egress Port 0 Initialization.

    @param[in] EgressPortBar   - EPBAR Address

    @retval EFI_SUCCESS        - Egress Port 0 initialization successed.
**/
EFI_STATUS
Cid1EgressPort0Init (
  IN  UINT32      EgressPortBar
  )
{
  UINT32  Data32And;
  UINT32  Data32Or;
  UINT8   Data8And;
  UINT8   Data8Or;
  UINT8   BitMask;
  UINT8   BitValue;

  ///
  /// Egress Port Configuration
  ///
  /// Egress Port Virtual Channel 0 Configuration
  ///   System BIOS must insure that only TC0 is mapped to VC0.
  ///   a. Set the Egress Port Register EPBAR offset 014h[7:1]=0000000b
  ///
  Data32And = BIT0;
  Data32Or  = 0;
  MmioAnd32 (EgressPortBar + 0x14, Data32And);

  ///
  /// System BIOS must program the extended VC count field.
  ///   b. Set the Egress Port Register EPBAR offset 004h[2:0]=001b
  ///
  Data8And  = (UINT8) ~(0x07);
  Data8Or   = BIT0;
  MmioAndThenOr8 (EgressPortBar + 0x4, Data8And, Data8Or);

  ///
  /// Egress Port VC1 Configuration
  ///   a. Assign Virtual Channel ID 1 to VC1: by programming PXEPBAR Offset 020[26:24] = '001b'
  ///   b. Select and map TC1 to VC1: by programming PXPEPBAR Offset 020h[7:1] = '0000001b'
  ///
  Data32And = (UINT32) ~(0x070000FE);
  Data32Or  = ((0x01 << 24) + BIT1);
  MmioAndThenOr32 (EgressPortBar + 0x20, Data32And, Data32Or);

  ///
  /// c. Enable VC1 (no hardware behind this bit, s/w compatibility flag only) BIT31
  ///    Program EXPEPBAR Offset 020h[31]=1
  ///
  Data32And = 0xFFFFFFFF;
  Data32Or  = BIT31;
  MmioOr32 (EgressPortBar + 0x20, Data32Or);

  ///
  /// d. Poll the VC1 Negotiation Pending bit until it reads 0:
  ///    Read the Egress Port Register EPBAR Offset 026h until [1]==0
  ///
  BitMask   = (UINT8) BIT1;
  BitValue  = 0;
  while ((MmioRead8 (EgressPortBar + 0x26) & BitMask) != BitValue) {
  };

  return EFI_SUCCESS;
}

/**
    Conditionally perform PEG Port Initialization.

    @param[in]   PCIE_PEI_PREMEM_CONFIG      PciePeiPreMemConfig,
    @param[in]   PCIE_PEI_CONFIG             PciePeiConfig,

    @retval EFI_SUCCESS            - PEG Port initialization successed.
**/
EFI_STATUS
Cid1PegPortInit (
  IN       PCIE_PEI_PREMEM_CONFIG     *PciePeiPreMemConfig,
  IN       PCIE_PEI_CONFIG            *PciePeiConfig
  )
{
  UINT32                       Data32;
  UINT32                       Data32Or;
  UINT32                       Data32And;
  UINT16                       Data16;
  UINT16                       Data16Or;
  UINT8                        Data8;
  UINT64                       PegBaseAddress;
  UINT8                        Bus;
  UINT8                        Dev;
  UINT8                        Func;
  UINT8                        PegComplete;
  CPU_SKU                      CpuSku;
  UINT8                        HwStrap;

  CpuSku   = GetCpuSku ();

  ///
  /// Read HwStrap Register - PEG1CFGSEL D1.R 504h [17:16]
  ///
  HwStrap = (UINT8) ((PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, R_SA_PEG_FUSESCMN_OFFSET)) & (BIT17 + BIT16)) >> 16);

  ///
  /// Scan/initialize PEG devices based on HW strapping.
  ///
  for (PegComplete = 0; PegComplete < ((sizeof (PegDeviceTable)) / (sizeof (PEG_PORT_DEVICE))); PegComplete++) {
    ///
    /// Get Peg Device BDF
    ///
    Bus             = PegDeviceTable[PegComplete].Bus;
    Dev             = PegDeviceTable[PegComplete].Device;
    Func            = PegDeviceTable[PegComplete].Function;

    PegBaseAddress  = (UINT64) PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, Bus, Dev, Func, 0);
    ///
    /// Check if the PEG is Enabled. Since Graphics initialization has already
    /// occurred, simply check for PEG presence.
    ///
    if (PciSegmentRead16 (PegBaseAddress + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
      ///
      /// PEG port enable and visible
      ///
      ///
      /// Initialize Slot Implemented for PCI Express Port
      ///
      Data16Or  = BIT8;

      PciSegmentOr16 (PegBaseAddress + R_SA_PEG_CAP_OFFSET, Data16Or);

      ///
      /// Initialize "Physical Slot Number" and Slot Power Limit for PCI Express Port
      /// Note: this register is a write once.
      ///
      Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_SLOTCAP_OFFSET);

      ///
      /// Set [31:19] for Physical Slot Number
      ///
      Data32 &= (UINT32) ~(0x1FFF << 19);
      Data32 |= ((UINT32) (PciePeiConfig->PegPhysicalSlotNumber[PegComplete] & 0x1FFF)) << 19;
      ///
      /// Set [14:7] for Power Limit Value: 75 Watts (Default)
      ///
      Data32 &= (UINT32) ~(0xFF << 7);
      Data32 |= ((UINT32) (PciePeiConfig->PegSlotPowerLimitValue[PegComplete])) << 7;
      ///
      /// [16:15] for Power Limit Scal: 1.0 Watt Scale
      ///
      Data32 &= (UINT32) ~(0x3 << 15);
      Data32 |= ((UINT32) (PciePeiConfig->PegSlotPowerLimitScale[PegComplete] & 0x3)) << 15;

      PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_SLOTCAP_OFFSET, Data32);

      ///
      /// Additional Programming Steps
      ///
      /// Set PEG D.F.R 006h [15:0] = 0FFFFh
      ///
      Data32And = ~(0xFFFF0000);
      Data32Or  = 0xFFFF0000;

      PciSegmentAndThenOr32 (PegBaseAddress + 0x4, Data32And, Data32Or);

      ///
      /// Set PEG D.F.R 01Eh [15:0] = 0FFFFh
      ///
      Data32And = ~(0xFFFF0000);
      Data32Or  = 0xFFFF0000;

      PciSegmentAndThenOr32 (PegBaseAddress + 0x1C, Data32And, Data32Or);

      ///
      /// Set PEG D.F.R 0AAh [15:0] = 0FFFFh
      ///
      ///
      ///
      Data32And = 0xFFFF;
      Data32Or  = 0xFFFF;

      PciSegmentOr16 (PegBaseAddress + 0xAA, (UINT16) Data32Or);

      ///
      /// Set PEG D.F.R 1C4h [31:0] = 0FFFFFFFFh
      ///
      Data32 = 0xFFFFFFFF;

      PciSegmentOr32 (PegBaseAddress + R_SA_PEG_PEGUESTS_OFFSET, Data32);

      ///
      /// Set PEG D.F.R 1D0h [31:0] = 0FFFFFFFFh
      ///
      Data32 = 0xFFFFFFFF;

      PciSegmentOr32 (PegBaseAddress + R_SA_PEG_PEGCESTS_OFFSET, Data32);

      ///
      /// Set PEG D.F.R 1F0h [31:0] = 0FFFFFFFFh
      ///
      Data32 = 0xFFFFFFFF;

      PciSegmentOr32 (PegBaseAddress + 0x1F0, Data32);
      ///
      /// Set BIT19 of DCAP2 register of the PEG port to enable OBFF support using WAKE# signaling
      ///
      if ((CpuSku == EnumCpuHalo) || (CpuSku == EnumCpuTrad)) {
        Data32 = (UINT32) BIT19;
        PciSegmentOr32 (PegBaseAddress + R_SA_PEG_DCAP2_OFFSET, Data32);
      }
      ///
      /// Complete Common Port and Endpoint Configuration
      ///
      ///
      /// Virtual Channel Configuration of PCI Express Port
      /// Set the VC0RCTL register D1:F0 Offset 114h [7:1] = 7Fh
      ///
      Data32And = 0xFFFFFF01;
      Data32Or  = BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1;

      PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_VC0RCTL0_OFFSET, Data32And, Data32Or);
      ///
      /// 6.8 Additional Programming Steps before Enabling ASPM for PEG device
      ///
      AdditionalPEGProgramStepsBeforeASPM (Bus, Dev, Func, PciePeiPreMemConfig, PciePeiConfig);

      ///
      /// 6.10 Interrupt Routing for PCI Express*
      /// It is recommened to re-route the legacy interrupts (INTA -> INTB,C,D)
      /// to avoid overcrowded INTA. ACPI PRT needs update.
      /// The ACPI _PRT() methods for PEG controllers must match the legacy interrupt routing.
      ///
      if ((Dev == 1) && (Func == 1)) {
        Data32And = (UINT32) ~(BIT25 | BIT24);
        Data32Or  = BIT24 | BIT25;
        PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_CFG4_OFFSET, Data32And, Data32Or);
      }

      if ((Dev == 1) && (Func == 2)) {
        Data32And = (UINT32) ~(BIT25 | BIT24);
        Data32Or  = BIT25;
        PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_CFG4_OFFSET, Data32And, Data32Or);
      }
    }

    ///
    /// Lock offset 3Dh for Interrupt Pin
    ///
    ///
    ///
    Data8 = PciSegmentRead8 (PegBaseAddress + PCI_INT_PIN_OFFSET);
    PciSegmentWrite8 (PegBaseAddress + PCI_INT_PIN_OFFSET, Data8);

    ///
    /// Lock DCAP register
    ///
    ///
    ///
    Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_DCAP_OFFSET);
    ///
    /// Override PEG MPS if user chose a lower value
    ///
    if ((PciePeiConfig->PegMaxPayload[PegComplete] != PEG_MPS_AUTO) && (PciePeiConfig->PegMaxPayload[PegComplete] <= (Data32 & (BIT2|BIT1|BIT0)))) {
      Data16 = PciSegmentRead16 (PegBaseAddress + R_SA_PEG_DCTL_OFFSET);
      ///
      /// Set Max Payload Size
      ///
      Data16 &= ~(BIT7 | BIT6 | BIT5);
      Data16 |= (PciePeiConfig->PegMaxPayload[PegComplete] << 5);
      PciSegmentWrite16 (PegBaseAddress + R_SA_PEG_DCTL_OFFSET, Data16);
    }
    PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_DCAP_OFFSET, Data32);

    ///
    ///
    if ((Bus == 0) && (Dev == 1) && (Func == 0)) {
      Data32  = PciSegmentRead32 (PegBaseAddress + 0xCD0);
      Data32 |= BIT11;
      PciSegmentWrite32 (PegBaseAddress + 0xCD0, Data32);
    }

    if ((HwStrap == SA_PEG_x16_x0_x0) && (PegComplete == 0)) {
      break;
    }
    if ((HwStrap == SA_PEG_x8_x8_x0) && (PegComplete == 1)) {
      break;
    }
    ///
    ///
    if ((HwStrap == SA_PEG_x8_x4_x4) && (PegComplete == 2)) {
      break;
    }
  }

  return EFI_SUCCESS;
}

/**
    DMI Port Initialization for both CID1 (Port 1 in MCH) and CID2 (Port 0 in PCH).

    @param[in] DmiBar              - DMIBAR Address
    @param[in] PCIE_PEI_CONFIG     - PciePeiConfig

    @retval EFI_SUCCESS            - DMI Port initialization successed.
**/
EFI_STATUS
Cid1Cid2DmiPortInit (
  IN  UINT32                       DmiBar,
  IN  PCIE_PEI_CONFIG              *PciePeiConfig
  )
{
  UINT32             Data32Or;
  UINT8              Data8And;
  UINT8              Data8Or;
  CPU_FAMILY         CpuFamilyId;
  SI_POLICY_PPI      *SiPolicy;
  EFI_STATUS         Status;
  PCH_GENERAL_CONFIG *PchGeneralConfig;
  UINT8              DmiAspm;

  CpuFamilyId = GetCpuFamily ();
  DmiAspm = PciePeiConfig->DmiAspm;

  //
  // Get Policy settings through the SiPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicy
             );
  if ((Status == EFI_SUCCESS) && (SiPolicy != NULL)) {
    Status = GetConfigBlock ((VOID *) SiPolicy, &gPchGeneralConfigGuid, (VOID *) &PchGeneralConfig);
    ASSERT_EFI_ERROR (Status);
    if (PchGeneralConfig->LegacyIoLowLatency) {
      DmiAspm = PcieAspmDisabled;
    }
  }

  ///
  /// Set DMIBAR Offset 1C4h [31:0] = 0FFFFFFFFh
  ///
  Data32Or                = 0xFFFFFFFF;
  MmioWrite32 (DmiBar + 0x1C4, 0xFFFFFFFF);

  ///
  /// Set DMIBAR Offset 1D0h [31:0] = 0FFFFFFFFh
  ///
  Data32Or                = 0xFFFFFFFF;
  MmioWrite32 (DmiBar + 0x1D0, 0xFFFFFFFF);

  if (IsPchLinkDmi (CpuFamilyId)) {
    ///
    /// Set L0SLAT[31] to 1 before enabling ASPM
    ///
    Data32Or  = (UINT32)   BIT31;
    MmioOr32 (DmiBar + R_SA_PEG_L0SLAT_OFFSET, Data32Or);

    ///
    /// Enable 'Active State PM'. DMILCTL register at DMIBAR 088h [1:0] = '11b'.
    /// Based on the policy:
    ///
    ///
    /// Set default value ASPM disabled.
    ///
    Data8And  = 0xFC;
    Data8Or   = 0x00;

    if (DmiAspm == PcieAspmAutoConfig ||
        DmiAspm == PcieAspmL0sL1
        ) {
      ///
      /// Enable ASPM = L0s and L1 Entry
      ///
      Data8Or   = 0x03;
    } else if (DmiAspm == PcieAspmL0s) {
      ///
      /// Enable ASPM = L0s
      ///
      Data8Or   = 0x01;
    } else if (DmiAspm == PcieAspmL1) {
      ///
      /// Enable ASPM = L1
      ///
      Data8Or   = 0x02;
    }
    MmioAndThenOr8 (DmiBar + 0x88, Data8And, Data8Or);

    if (PciePeiConfig->DmiExtSync == ENABLED) {
      ///
      /// Enable Extended Synch
      ///
      Data8Or   = 0x10;
      MmioOr8 (DmiBar + 0x88, Data8Or);
    }

    if (PciePeiConfig->DmiIot == ENABLED) {
      ///
      /// if DMI Iot is enabled, set DMIBAR offset 0xD34 = 0x44
      ///
      Data8Or               = 0x44;
      MmioWrite8 (DmiBar + 0xD34, 0x44);
    }
  }

  return EFI_SUCCESS;
}

/**
    Perform Root Complex Topology Initialization for CID1.

    @param[in] EgressPortBar     - EPBAR Address
    @param[in] DmiBar            - DMIBAR Address

    @retval EFI_SUCCESS          - Root Complex topology initialization for CID1 successed.
**/
EFI_STATUS
Cid1TopologyInit (
  IN  UINT32       EgressPortBar,
  IN  UINT32       DmiBar
  )
{
  UINT32  Data32;
  UINT32  Data32And;
  UINT32  Data32Or;
  UINT32  DwordReg;
  UINT64  McD1Base;
  UINT64  McD1F1Base;
  UINT64  McD1F2Base;

  McD1Base    = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, 0);
  McD1F1Base  = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 1, 0);
  McD1F2Base  = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 2, 0);

  ///
  /// Set the CID1 Egress Port 0 Topology
  ///
  ///
  /// Step 1, Set the SA Component ID = 1.
  ///
  Data32And = 0xFF00FFFF;
  Data32Or  = BIT16;
  MmioAndThenOr32 (EgressPortBar + 0x44, Data32And, Data32Or);

  ///
  /// Step 2, Set link 1 Target Component ID and valid (Bit 0 = 1b).
  /// Set the Link 1 TCID = 1 (Bits 23:16 = 01h).
  ///
  Data32And = 0xFF00FFFF;
  Data32Or  = (BIT16 | BIT0);
  MmioAndThenOr32 (EgressPortBar + 0x50, Data32And, Data32Or);

  ///
  /// Step 3, Set Link 1 to Reference the DMI RCRB (Bits 31:0 = DMI Base).
  ///
  Data32 = (UINT32) DmiBar;
  MmioWrite32 ((UINTN) EgressPortBar + 0x58, Data32);
  Data32 = (UINT32) 0x00;
  MmioWrite32 ((UINTN) EgressPortBar + 0x58 + 4, Data32);

  ///
  /// CID1 Egress port Root Topology and PciExpress Port (PEG Ports) Topology
  /// Programming only if PEG devices are enabled.
  ///
  /// Step 1 PCI Express Enabled Check
  /// Check and Configure CID1 root and Device 1 function 0
  ///
  if (PciSegmentRead16 (McD1Base + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
    ///
    /// Step 4, Set Link 2 to Reference the Device 1 function 0.
    ///
    Data32 = (UINT32) 0x8000;
    MmioWrite32 ((UINTN) EgressPortBar + 0x68, Data32);
    Data32 = (UINT32) 0x00;
    MmioWrite32 ((UINTN) EgressPortBar + 0x68 + 4, Data32);

    ///
    /// Step 5, Set link 2 Target Component ID and valid bit(Offset 60h, Bit 0 = 1b)
    /// Set the Link 2 TCID = 1 (Bits 23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    MmioAndThenOr32 (EgressPortBar + 0x60, Data32And, Data32Or);

    ///
    /// Set the CID1 PCI Express* Port Root Topology
    ///
    /// Step 2 Set the Link 1 CID = 1 144h(23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = BIT16;
    PciSegmentAndThenOr32 ((UINTN) McD1Base + 0x144, Data32And, Data32Or);

    ///
    /// Step 3. Set Link 1 to Reference the SA EP RCRB (Bits 31:0 = EPBAR).
    ///
    Data32 = (UINT32) EgressPortBar;
    PciSegmentWrite32 ((UINTN) McD1Base + 0x158, Data32);
    Data32 = (UINT32) 0x00;
    PciSegmentWrite32 ((UINTN) McD1Base + 0x158 + 4, Data32);

    ///
    /// Step 4 Set link 1 Target Component ID and valid bit(Offset 150h, Bit 0 = 1b)
    /// Set the Link 1 TCID = 1 (Bits 23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    PciSegmentAndThenOr32 ((UINTN) McD1Base + 0x150, Data32And, Data32Or);

    ///
    /// Step 5 Program Read-Only Write-Once Registers
    /// D1.F0.R 0C4h [31:0]
    ///
    Data32And             = 0xFFFFFFFF;
    Data32Or              = (UINT32) 0x00;
    Data32    = PciSegmentRead32 (McD1Base + 0xC4);
    PciSegmentWrite32 (McD1Base + 0xC4, Data32);
  }
  ///
  /// Check and Configure CID1 root and Device 1 function 1
  ///
  if (PciSegmentRead16 (McD1F1Base + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
    ///
    /// Step 6, Set Link 3 to Reference the Device 1 function 1.
    ///
    Data32 = (UINT32) 0x9000;
    MmioWrite32 ((UINTN) EgressPortBar + 0x78, Data32);
    Data32 = (UINT32) 0x00;
    MmioWrite32 ((UINTN) EgressPortBar + 0x78 + 4, Data32);

    ///
    /// Step 7. Set the Link 3 Target Component ID and Valid Bit 70h [0]
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    MmioAndThenOr32 (EgressPortBar + 0x70, Data32And, Data32Or);

    ///
    /// Step 2 Set the Link 1 CID = 1 144h(23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = BIT16;
    PciSegmentAndThenOr32 ((UINTN) McD1F1Base + 0x144, Data32And, Data32Or);

    ///
    /// Step 3 Set Link 1 to Reference the IMC EP RCRB (Bits 31:0 = EPBAR).
    ///
    Data32 = (UINT32) EgressPortBar;
    PciSegmentWrite32 ((UINTN) McD1F1Base + 0x158, Data32);
    Data32 = (UINT32) 0x00;
    PciSegmentWrite32 ((UINTN) McD1F1Base + 0x158 + 4, Data32);

    ///
    /// Step 4 Set link 1 Target Component ID and valid bit(Offset 150h, Bit 0 = 1b)
    /// Set the Link 1 TCID = 1 (Bits 23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    PciSegmentAndThenOr32 ((UINTN) McD1F1Base + 0x150, Data32And, Data32Or);

    ///
    /// Step 5 Program Read-Only Write-Once Registers
    ///
    Data32And               = 0xFFFFFFFF;
    Data32Or                = (UINT32) 0x00;
    Data32    = PciSegmentRead32 (McD1F1Base + 0xC4);
    PciSegmentWrite32 (McD1F1Base + 0xC4, Data32);
  }
  ///
  /// Check and Configure CID1 root and Device 1 function 2
  ///
  if (PciSegmentRead16 (McD1F2Base + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
    ///
    /// Step 8, Set Link 4 to Reference the Device 1 function 2.
    ///
    Data32 = (UINT32) 0xA000;
    MmioWrite32 ((UINTN) EgressPortBar + 0x88, Data32);
    Data32 = (UINT32) 0x00;
    MmioWrite32 ((UINTN) EgressPortBar + 0x88 + 4, Data32);

    ///
    /// Step 9, Set the Link 4 Target Component ID and Valid Bit 80h [0]
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    MmioAndThenOr32 (EgressPortBar + 0x80, Data32And, Data32Or);

    ///
    /// Step 2. Set the Link 1 CID = 1 144h(23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = BIT16;
    PciSegmentAndThenOr32 ((UINTN) McD1F2Base + 0x144, Data32And, Data32Or);

    ///
    /// Step 3. Set Link 1 to Reference the IMC EP RCRB (Bits 31:0 = EPBAR).
    ///
    Data32 = (UINT32) EgressPortBar;
    PciSegmentWrite32 ((UINTN) McD1F2Base + 0x158, Data32);
    Data32 = (UINT32) 0x00;
    PciSegmentWrite32 ((UINTN) McD1F2Base + 0x158 + 4, Data32);

    ///
    /// Step 4. Set link 1 Target Component ID and valid bit(Offset 150h, Bit 0 = 1b)
    /// Set the Link 1 TCID = 1 (Bits 23:16 = 01h).
    ///
    Data32And = 0xFF00FFFF;
    Data32Or  = (BIT16 | BIT0);
    PciSegmentAndThenOr32 ((UINTN) McD1F2Base + 0x150, Data32And, Data32Or);

    ///
    /// Step 5. Program Read-Only Write-Once Registers
    ///
    Data32And               = 0xFFFFFFFF;
    Data32Or                = (UINT32) 0x00;
    Data32    = PciSegmentRead32 (McD1F2Base + 0xC4);
    PciSegmentWrite32 (McD1F2Base + 0xC4, Data32);
  }

  ///
  /// Set the CID1 DMI Port Root Topology
  ///
  /// Step 1 Set the CID = 1 ( Bits 23:16 = 01h).
  ///
  Data32And = 0xFF00FFFF;
  Data32Or  = BIT16;
  MmioAndThenOr32 (DmiBar + 0x44, Data32And, Data32Or);

  ///
  /// Step 2.  Set Link 1 Target Port Number = 0 (Bits 31:24 = 00h).
  /// Step 3.  Set Link 1 TCID = 2 (Bits 23:16 = 02h).
  /// Step 4.  Set Link 1 as valid (Bit 0 = 1b).
  ///
  Data32And = 0x0000FFFF;
  Data32Or  = (BIT17 + BIT0);
  MmioAndThenOr32 (DmiBar + 0x50, Data32And, Data32Or);

  ///
  /// Step 5, Set Link 1 to Reference the PCH RCRB
  ///

  // SPT TODO: Replace with right code during PCIe cleanup

  Data32                      = (UINT32) 0x00;
  MmioWrite32 (DmiBar + (0x58 + 4), Data32);

  ///
  /// Step 6, Set Link 2 to Reference the IMC EP (Bits 31:0 = EP).
  ///
  Data32 = (UINT32) EgressPortBar;
  MmioWrite32 ((UINTN) DmiBar + 0x68, Data32);
  Data32 = (UINT32) 0x00;
  MmioWrite32 ((UINTN) DmiBar + 0x68 + 4, Data32);

  ///
  /// Step 7, Set link 2 Target Component ID and valid Bit(Bit 0 = 1b)
  /// Set the Link 2 TCID = 1 (Bits 23:16 = 01h).
  ///
  Data32And = 0xFF00FFFF;
  Data32Or  = (BIT16 | BIT0);
  MmioAndThenOr32 (DmiBar + 0x60, Data32And, Data32Or);

  ///
  /// Step 8. Program RO and Write-Once Registers
  /// DMIBAR Offset 004h [31:0]
  /// DMIBAR Offset 084h [31:0]
  ///
  DwordReg = MmioRead32 (DmiBar + 0x4);
  MmioWrite32 (DmiBar + 0x4, DwordReg);

  DwordReg = MmioRead32 (DmiBar + 0x84);
  MmioWrite32 (DmiBar + 0x84, DwordReg);

  return EFI_SUCCESS;
}

/**
  Perform Root Complex Topology Initialization for CID2.
  Note: This sequence follows PCH BIOS specification Ver 0.5 section 8.3
        Root Complex Topology Programming

  @param[in] DmiBar            - DMIBAR Address

  @retval EFI_SUCCESS          - Root Complex topology initialization for CID2 successed.
**/
EFI_STATUS
Cid2TopologyInit (
  IN UINT32   DmiBar
  )
{
  ///
  /// PCH BIOS specification Ver 0.5 section 8.3 Note 1,2
  /// program a value into this Component ID field to determine the Component ID of
  /// ICH8, and this value must be different from the Component ID value of the MCH.
  /// Set the CID = 2 (Offset 104h, Bits 23:16 = 02h).
  ///

  // SPT TODO: Replace with right code during PCIe cleanup

  ///
  /// PCH BIOS specification Ver 0.5 section 8.3 Note 3,4
  /// Note 3: This Target Port # field must be filled in by System BIOS with the Port
  /// # of the RCRB DMI link in the MCH.
  /// Note 4: This Target CID field must be filled in by System BIOS with the
  /// Component ID of the MCH.
  /// Set the Link 1 Target Port Number = 1 (Offset 110h, bits 31:24 = 01h).
  /// Set the Link 1 Target Component ID = 1 (Offset 110h, bits 23:16 = 01h).
  ///

  // SPT TODO: Replace with right code during PCIe cleanup

  ///
  /// PCH BIOS specification Ver 0.5 section 8.3 Note 5
  /// Fill the Base Address field with the same base address of the RCRB DMI link in
  /// the MCH, This register field is located at RCBA+ 0118h[63:0],
  /// and will be locked once written until the next reset.
  ///

  // SPT TODO: Replace with right code during PCIe cleanup

  return EFI_SUCCESS;
}
