/** @file
  SA PCIe Initialization Private Low Level functions for Coffeelake/Cannonlake DT/HALO

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

@par Specification
**/

#include "SaPegLowLevel.h"
#include <Private/Library/SaPcieDmiLib.h>
#include <Library/PchPcrLib.h>
#include <Library/SaPlatformLib.h>
#ifndef CPU_CFL
#include <Library/CpuMailboxLib.h>
#include <Register/SaRegsHostBridge.h>
#endif
#ifdef CPU_CFL
STATIC CREDIT_CONTROL_INIT_ENTRY mCreditControlInitTableX8x4x4Ptr[] = {
  {R_SA_MCHBAR_CRDTCTL0_OFFSET,  0x08241251},
  {R_SA_MCHBAR_CRDTCTL1_OFFSET,  0x08241251},
  {R_SA_MCHBAR_CRDTCTL2_OFFSET,  0x0220A412},
  {R_SA_MCHBAR_CRDTCTL3_OFFSET,  0x00000010},
  {R_SA_MCHBAR_CRDTCTL4_OFFSET,  0x000006D6},
  {R_SA_MCHBAR_CRDTCTL5_OFFSET,  0x00339DC7},
  {R_SA_MCHBAR_CRDTCTL6_OFFSET,  0x00301080},
  {R_SA_MCHBAR_CRDTCTL7_OFFSET,  0x00339DC7},
  {R_SA_MCHBAR_CRDTCTL8_OFFSET,  0x00601080},
  {R_SA_MCHBAR_CRDTCTL9_OFFSET,  0x041C7388},
  {R_SA_MCHBAR_CRDTCTL10_OFFSET, 0x00000200},
  {R_SA_MCHBAR_CRDTCTL11_OFFSET, 0x00004100},
  {R_SA_MCHBAR_CRDTCTL12_OFFSET, 0x0000048A}
};

STATIC CREDIT_CONTROL_INIT_ENTRY mCreditControlInitTableX8x8Ptr[] = {
  {R_SA_MCHBAR_CRDTCTL0_OFFSET,  0x08241051},
  {R_SA_MCHBAR_CRDTCTL1_OFFSET,  0x08241051},
  {R_SA_MCHBAR_CRDTCTL2_OFFSET,  0x02208412},
  {R_SA_MCHBAR_CRDTCTL3_OFFSET,  0x00000010},
  {R_SA_MCHBAR_CRDTCTL4_OFFSET,  0x00000758},
  {R_SA_MCHBAR_CRDTCTL5_OFFSET,  0x003039C8},
  {R_SA_MCHBAR_CRDTCTL6_OFFSET,  0x00301080},
  {R_SA_MCHBAR_CRDTCTL7_OFFSET,  0x003039C7},
  {R_SA_MCHBAR_CRDTCTL8_OFFSET,  0x00601080},
  {R_SA_MCHBAR_CRDTCTL9_OFFSET,  0x0400E388},
  {R_SA_MCHBAR_CRDTCTL10_OFFSET, 0x00000200},
  {R_SA_MCHBAR_CRDTCTL11_OFFSET, 0x00004100},
  {R_SA_MCHBAR_CRDTCTL12_OFFSET, 0x0000048A}
};
#else
STATIC CREDIT_CONTROL_INIT_ENTRY mCreditControlInitTableX8x4x4Ptr[] = {
  {R_SA_MCHBAR_CRDTCTL_PMIN_A_OFFSET,   0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PMIN_B_OFFSET,   0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_A_OFFSET,  0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PINIT_A_OFFSET,  0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PINIT_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_A_OFFSET, 0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_B_OFFSET, 0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PMAX_A_OFFSET,   0x1600826D},
  {R_SA_MCHBAR_CRDTCTL_PMAX_B_OFFSET,   0x08208401},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_A_OFFSET,  0x08008206},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_B_OFFSET,  0x00739DA1},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_A_OFFSET,  0x00048209},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_B_OFFSET,  0x00000249},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_A_OFFSET,  0x00080307},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_B_OFFSET,  0x14A28A46}
};

STATIC CREDIT_CONTROL_INIT_ENTRY mCreditControlInitTableX8x8Ptr[] = {
  {R_SA_MCHBAR_CRDTCTL_PMIN_A_OFFSET,   0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PMIN_B_OFFSET,   0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_A_OFFSET,  0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PINIT_A_OFFSET,  0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PINIT_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_A_OFFSET, 0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_B_OFFSET, 0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PMAX_A_OFFSET,   0x1600826D},
  {R_SA_MCHBAR_CRDTCTL_PMAX_B_OFFSET,   0x08050401},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_A_OFFSET,  0x08008206},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_B_OFFSET,  0x0070B5A1},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_A_OFFSET,  0x00048209},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_B_OFFSET,  0x00000209},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_A_OFFSET,  0x00080307},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_B_OFFSET,  0x14028A46}
};

STATIC CREDIT_CONTROL_INIT_ENTRY mCreditControlInitTableX16Ptr[] = {
  {R_SA_MCHBAR_CRDTCTL_PMIN_A_OFFSET,   0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PMIN_B_OFFSET,   0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_A_OFFSET,  0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPMIN_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PINIT_A_OFFSET,  0x00070212},
  {R_SA_MCHBAR_CRDTCTL_PINIT_B_OFFSET,  0x00000254},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_A_OFFSET, 0x00050212},
  {R_SA_MCHBAR_CRDTCTL_NPINIT_B_OFFSET, 0x00000254},
  {R_SA_MCHBAR_CRDTCTL_PMAX_A_OFFSET,   0x1600826D},
  {R_SA_MCHBAR_CRDTCTL_PMAX_B_OFFSET,   0x08041901},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_A_OFFSET,  0x08008206},
  {R_SA_MCHBAR_CRDTCTL_NPMAX_B_OFFSET,  0x00708741},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_A_OFFSET,  0x00048209},
  {R_SA_MCHBAR_CRDTCTL_RRMIN_B_OFFSET,  0x00000201},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_A_OFFSET,  0x00080307},
  {R_SA_MCHBAR_CRDTCTL_RRMAX_B_OFFSET,  0x14001446}
};
#endif
/**
  Determines if the PEG root ports are capable of PCIe 3.0 speed

  @param[in]  This                        - Low level function table

  @retval TRUE - PEG Root Ports are PCIe 3.0 capable
  @retval FALSE - otherwise
**/
BOOLEAN
EFIAPI
SklPegIsGen3Capable (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  )
{
  BOOLEAN   Gen3Capable;

  Gen3Capable = TRUE;
  if (PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_CAPID0_B)) & BIT20) {
    DEBUG ((DEBUG_INFO, "PEG Gen3 Fused off\n"));
    Gen3Capable = FALSE;
  }
  return Gen3Capable;
}

#ifndef CPU_CFL
/**
  Reads, modifies and writes to PCODE mail box as per the input.

  @param[in]  CrOffset                    - Config Register Offset
  @param[in]  Data32And                   - Data to Clear
  @param[in]  Data32Or                    - Data to Set
**/
EFI_STATUS
PcodeMailboxReadThenWrite(
  IN  UINT32           CrOffset,
  IN  UINT32           Data32And,
  IN  UINT32           Data32Or,
  IN  CONTROLLER_TYPE  ControllerType
  )
{
  EFI_STATUS  Status;
  UINT32      LibStatus;
  UINT32      Data32;
  UINT32      TargetBlock;

  TargetBlock = 0;
  //
  // Target IO block 0 for DMI or 1 for PCI (Bit 24 in Mailbox Command)
  //
  if (ControllerType == PEG_PORT) {
    TargetBlock = 0x1 << N_MAILBOX_TARGET_IO_BLOCK_OFFSET;
  } else {
    TargetBlock = 0x0 << N_MAILBOX_TARGET_IO_BLOCK_OFFSET;
  }
  MailboxRead (MAILBOX_TYPE_PCODE, (READ_PEG_CRIO_CR | TargetBlock | CrOffset),&Data32,&LibStatus);
  if(LibStatus != PCODE_MAILBOX_CC_SUCCESS){
    DEBUG ((DEBUG_INFO, "PEG_CRIO_CR() - MailboxRead Error = %r\n", LibStatus));
  }
  Data32 &= Data32And;
  Data32 |= Data32Or;
  Status = MailboxWrite (MAILBOX_TYPE_PCODE, (WRITE_PEG_CRIO_CR | TargetBlock | CrOffset),Data32,&LibStatus);
  if((Status != EFI_SUCCESS)||(LibStatus != PCODE_MAILBOX_CC_SUCCESS)){
    DEBUG ((DEBUG_INFO, "PEG_CRIO_CR() - MailboxWrite Error = %r\n", LibStatus));
  }
  return Status;
}
#endif

/**
  Performs AFE and RP programming that needs to be done prior to enabling the
  PCIe link and allowing it to train to active state for the first time.

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
EFIAPI
SklPegPreDetectionProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  PCIE_PEI_PREMEM_CONFIG          *PciePeiPreMemConfig;
  UINT64                          PegBaseAddress;
  UINT32                          Data32;
  UINT32                          Data32And;
  UINT32                          Data32Or;
  UINT32                          Bundle;
  UINT8                           Index;
  UINT8                           PegDev;
  UINT8                           PegFunc;
#ifndef CPU_CFL
  UINT16                          SaDid;
  UINT8                           Peg3LaneReversal;
  UINT8                           StartBundle;
  UINT32                          CrOffset;
  UINT8                           Peg0LaneReversal;
  UINT32                          HwStrap;
  CPU_STEPPING                    CpuSteppingId;
  CPU_FAMILY                      CpuFamilyId;
#endif

  PciePrivate         = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;
  PciePeiPreMemConfig = PciePrivate->PciePeiPreMemConfig;
  PegBaseAddress      = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);

#ifndef CPU_CFL
  CpuFamilyId         = GetCpuFamily ();
  CpuSteppingId       = GetCpuStepping ();

  Peg0LaneReversal  = (PciSegmentRead32 (
                          PCI_SEGMENT_LIB_ADDRESS (
                            SA_SEG_NUM,
                            SA_PEG_BUS_NUM,
                            SA_PEG_DEV_NUM,
                            SA_PEG0_FUN_NUM,
                            R_SA_PEG_PEGTST_OFFSET
                            )
                          ) >> 20) & 0x1;

  Peg3LaneReversal  = (PciSegmentRead32 (
                          PCI_SEGMENT_LIB_ADDRESS (
                            SA_SEG_NUM,
                            SA_PEG_BUS_NUM,
                            SA_PEG3_DEV_NUM,
                            SA_PEG3_FUN_NUM,
                            R_SA_PEG_PEGTST_OFFSET
                            )
                          ) >> 20) & 0x1;

  HwStrap         = (PciSegmentRead32 (PegBaseAddress + R_SA_PEG_FUSESCMN_OFFSET) >> 16) & 0x3;
#endif

#ifndef CPU_CFL
  ///
  ///
  if ((CpuFamilyId == EnumCpuCnlDtHalo) && (CpuSteppingId == EnumCnlP0)) {
    if (Peg0LaneReversal != 0) {
      Data32And = (UINT32) ~(BIT22 | BIT21 | BIT20 | BIT19 | BIT18 | BIT17 | BIT16 | BIT14 | BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8);
      Data32Or = ( (V_PH3_FS_CR_OVR << 9) | B_PH3_FS_CR_OVR_EN | (V_PH3_LF_CR_OVR << 17) | B_PH3_LF_CR_OVR_EN);
      CrOffset  = (R_SA_PEG_BND9_CRI0_CR_DWORD28_OFFSET << 8);
      PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
    }
  }
#endif
  for (Index = 0; Index < PciePortsLength; Index++) {
    PegDev         = PciePorts[Index].Device;
    PegFunc        = PciePorts[Index].Function;
    PegBaseAddress = PciePorts[Index].ConfigSpaceBase;

  ///
    ///
    ///
    /// Set PCIE_CR_REUT_OVR_CTL_0_1_0_MMR.GRCLKGTDIS [28] to 1 (for PCIE Margin Test, Default is kept 0)
    ///
  if(((PegDev == SA_PEG0_DEV_NUM)&& (PegFunc == SA_PEG0_FUN_NUM))||(PegDev == SA_PEG3_DEV_NUM)){
    Data32And = (UINT32) ~BIT28;
      Data32Or  = 0;
      PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_REUT_OVR_CTL_OFFSET, Data32And, Data32Or);
  }

    ///
    ///
    ///
    /// DCBLNC = 0
    ///
    Data32And = (UINT32) ~(BIT3 | BIT2);
    Data32Or  = 0;
    if(PegDev == SA_PEG0_DEV_NUM) {
      for (Bundle = 0; Bundle < SA_PEG0_CNT_MAX_BUNDLE; Bundle++) {
        PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_G3CTL0_OFFSET + (Bundle * BUNDLE_STEP), Data32And, Data32Or);
      }
    }
#ifndef CPU_CFL
    if (PegDev == SA_PEG3_DEV_NUM) {
      for (Bundle = 0; Bundle < SA_PEG3_CNT_MAX_BUNDLE; Bundle++) {
        PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_G3CTL0_OFFSET + (Bundle * BUNDLE_STEP), Data32And, Data32Or);
      }
    }
#endif

#ifndef CPU_CFL
  SaDid = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));
  if(SaDid == V_SA_DEVICE_ID_CNL_HALO_1){
    ///
    ///
    PciSegmentOr32 (PegBaseAddress + R_SA_PEG_OFFSET_F4, BIT27);

    ///
    ///
    /// Disable LMS Power saving

    Data32And = 0xFFFFFFFF;
    Data32Or  = BIT7;

    if(PegDev == SA_PEG3_DEV_NUM){ //PEG controller BDF=060 is not part of bifurcation, so deal with it seperately
      if(Peg3LaneReversal == 0){
        StartBundle = 0;
      } else {
        StartBundle = 1;
      }
      // MailBox Write to set  lms_pwr_dwn bit for StartBundle
      CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
      PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
    }
    if(PegDev == SA_PEG0_DEV_NUM){
      switch (PegFunc) {
          case SA_PEG0_FUN_NUM:
          switch (HwStrap) {
            case SA_PEG_x16_x0_x0:
              if (Peg0LaneReversal == 0) {
                StartBundle = 2;
              } else {
                StartBundle = 9;
              }
              // MailBox Write to set  lms_pwr_dwn bit for StartBundle
              CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
              PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
              break;
            case SA_PEG_x8_x8_x0:
            case SA_PEG_x8_x4_x4:
              if (Peg0LaneReversal == 0) {
                StartBundle = 2;
              } else {
                StartBundle = 6;
              }
              // MailBox Write to set  lms_pwr_dwn bit for StartBundle
              CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
              PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
              break;
            default:
              return; ///< Nothing to do for PEG10
          }
          break;
        case SA_PEG1_FUN_NUM:
          switch (HwStrap) {
            case SA_PEG_x8_x8_x0:
              if (Peg0LaneReversal == 0) {
                StartBundle = 6;
              } else {
                StartBundle = 2;
              }
              // MailBox Write to set  lms_pwr_dwn bit for StartBundle
              CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
              PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
              break;
            case SA_PEG_x8_x4_x4:
              if (Peg0LaneReversal == 0) {
                StartBundle = 6;
              } else {
                StartBundle = 4;
              }
              // MailBox Write to set  lms_pwr_dwn bit for StartBundle
              CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
              PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
              break;
            default:
              return; ///< Nothing to do for PEG11
          }
          break;
        case SA_PEG2_FUN_NUM:
          switch (HwStrap) {
            case SA_PEG_x8_x4_x4:
              if (Peg0LaneReversal == 0) {
                StartBundle = 8;
              } else {
                StartBundle = 2;
              }
              // MailBox Write to set  lms_pwr_dwn bit for StartBundle
              CrOffset  = ((R_SA_PEG_BND0_CRI0_CR_DWORD14_OFFSET << 8)|((StartBundle*2) << 16));
              PcodeMailboxReadThenWrite(CrOffset, Data32And, Data32Or, PEG_PORT);
              break;
            default:
              return; ///< Nothing to do for PEG12
          }
          break;
        default:
          return; ///< Invalid PEG Controller
        }//End of Switch PegFunc
    }//End of (PegDev == SA_PEG0_DEV_NUM)
    }// End of If (SaDid == V_SA_DEVICE_ID_CNL_HALO_1)
#endif

    PciSegmentOr32 (PegBaseAddress + R_SA_PEG_LTSSMC_OFFSET, (UINT32) (BIT4 | BIT3 | BIT2 | BIT1 | BIT0));
    ///
    /// Program Read-Only Write-Once Registers
    ///   R 308h [31:0]
    ///   R 314h [31:0]
    ///   R 32Ch [31:0]
    ///   R 330h [31:0]
    ///
    Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_VC0PRCA_OFFSET);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_VC0PRCA_OFFSET, Data32);
    Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_VC0NPRCA_OFFSET);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_VC0NPRCA_OFFSET, Data32);
    Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_VC1PRCA_OFFSET);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_VC1PRCA_OFFSET, Data32);
    Data32 = PciSegmentRead32 (PegBaseAddress + R_SA_PEG_VC1NPRCA_OFFSET);
    PciSegmentWrite32 (PegBaseAddress + R_SA_PEG_VC1NPRCA_OFFSET, Data32);

    ///
    ///
    ///
    /// Program Read-Write Register
    ///   R CD4h [30:24]
    ///
    Data32And = (UINT32) ~(BIT30 | BIT29 | BIT28 | BIT27 | BIT26 | BIT25 | BIT24);
    Data32Or  = 0x40 << 24;
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_G3PLINIT_OFFSET, Data32And, Data32Or);

    ///
    /// Program Peg PCI Register 208h [31:30] to 01
    ///
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_PEGCC_OFFSET,  (UINT32) ~ (BIT31 | BIT30),(UINT32) (BIT30));
  }

  ///
  /// Program PEG Recipe
  ///
  PciePrivate->PcieDmiRecipe (
                 PciePrivate->DmiBar,
                 PciePrivate->MchBar,
                 &(PciePeiPreMemConfig->PegGen3RxCtlePeaking[0]),
                 GetMaxPegBundles(),
                 (UINT8) PciePeiPreMemConfig->PegGen3RxCtleOverride,
                 FALSE
                 );
}

/**
  Configure PCIe Max Speed and Width

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports
  @param[in]  PciePortsLength             - Length of the PciePorts array
  @param[in]  Gen3Capable                 - PEG is Gen3 capable
**/
VOID
EFIAPI
SklConfigureMaxSpeedWidth (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength,
  IN  BOOLEAN                           Gen3Capable
  )
{
  PCIE_PEI_PREMEM_CONFIG  *PciePeiPreMemConfig;
  UINT64                  PegBaseAddress;
  UINT16                  LinkSpeed;
  UINT8                   Index;
  UINT8                   PegPortMaxLinkSpeed;
  UINT8                   PegBus;
  UINT8                   PegDev;
  UINT8                   PegFunc;
  UINT8                   MaxLinkWidth;

  PciePeiPreMemConfig = ((SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData)->PciePeiPreMemConfig;
  for (Index = 0; Index < PciePortsLength; Index++) {
    PegBus        = PciePorts[Index].Bus;
    PegDev        = PciePorts[Index].Device;
    PegFunc       = PciePorts[Index].Function;
    MaxLinkWidth  = PciePorts[Index].MaxPortWidth;

    ///
    /// Check if this port exists
    ///
    PegBaseAddress = PciePorts[Index].ConfigSpaceBase;
    if (PciSegmentRead16 (PegBaseAddress + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }

    ///
    /// PCIe Port Speed: 0 = Auto, 1 = Gen1, 2 = Gen2, 3 = Gen3
    ///
    PegPortMaxLinkSpeed = SaPolicyGetPegMaxLinkSpeed (&(PciePorts[Index]), PciePeiPreMemConfig);

    if (PegPortMaxLinkSpeed == PEG_AUTO) {
      LinkSpeed = (UINT16) (PciSegmentRead32 (PegBaseAddress + R_SA_PEG_LCAP_OFFSET) & 0x0F);
      DEBUG ((DEBUG_INFO, "PEG %x:%x:%x Speed: Auto %x\n", PegBus, PegDev, PegFunc,LinkSpeed));
    } else {
      LinkSpeed = PegPortMaxLinkSpeed;
      DEBUG ((DEBUG_INFO, "PEG %x:%x:%x Speed: %x\n", PegBus, PegDev, PegFunc, LinkSpeed));
    }
    ///
    /// If Gen3 is fused off, limit is Gen2
    ///
    if (Gen3Capable == FALSE) {
      if (LinkSpeed > 2) {
        LinkSpeed = 2;
      }
    }
    ///
    /// Set the requested speed in Max Link Speed in LCAP[3:0] and Target Link Speed in LCTL2[3:0].
    /// Update LCAP.MLW in the same write as it's a Write-Once field. MLW field is locked here.
	/// Configure Target Link Speed to Gen1 until Gen2 endpoint devices can be detected.
    ///
    DEBUG ((DEBUG_INFO, "PEG%x%x (%x:%x:%x) - Max Link Speed = Gen%d - Max Link Width = %d\n", PegDev, PegFunc, PegBus, PegDev, PegFunc, LinkSpeed, MaxLinkWidth));
    PciSegmentAndThenOr32 (PegBaseAddress + R_SA_PEG_LCAP_OFFSET, 0xFFFFFC00, ((UINT32) MaxLinkWidth << 4) | LinkSpeed);
#ifndef UP_SERVER_FLAG
    PciSegmentAndThenOr16 (PegBaseAddress + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), LinkSpeed);
#else
	PciSegmentAndThenOr16 (PegBaseAddress + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 1);
#endif //UP_SERVER_FLAG
  }
}

/**
  Enable RxCEM Loopback (LPBK) Mode

  @param[in]  This                        - Low level function table
  @param[in]  RxCemLoopbackLane           - Lane to use for RxCEM Testing
**/
VOID
EFIAPI
SklEnableRxCemLoopbackMode (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             RxCEMLoopbackLane,
  IN  PCIE_PORT_INFO                    *PciePorts
  )
{
  PCIE_PEI_PREMEM_CONFIG  *PciePeiPreMemConfig;
  UINT64                  Peg0BaseAddress;
  UINT8                   Lane;
  UINT32                  Data32Or;
  UINT32                  Data32And;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  PciePeiPreMemConfig = ((SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData)->PciePeiPreMemConfig;

  PciSegmentAndThenOr32 (Peg0BaseAddress + R_SA_PEG_PEGTST_OFFSET, (UINT32) ~(BIT19|BIT18|BIT17|BIT16), (RxCEMLoopbackLane & 0xF) << 16);
  if((PciePorts->Device == SA_PEG0_DEV_NUM) && (PciePorts->Function == SA_PEG0_FUN_NUM)){
    for (Lane = 0; Lane < SA_PEG0_CNT_MAX_LANE; Lane++) {
    if (Lane == RxCEMLoopbackLane) {
        PciSegmentAnd32 (Peg0BaseAddress + R_SA_PEG_AFELN0CFG0_OFFSET + (LANE_STEP * Lane), (UINT32) ~BIT9);
      } else {
        PciSegmentOr32 (Peg0BaseAddress + R_SA_PEG_AFELN0CFG0_OFFSET + (LANE_STEP * Lane), BIT9);
      }
  }
  }
  if(PciePorts->Device == SA_PEG3_DEV_NUM){
    for (Lane = 0; Lane < SA_PEG3_CNT_MAX_LANE; Lane++) {
    if (Lane == RxCEMLoopbackLane) {
        PciSegmentAnd32 (PciePorts->ConfigSpaceBase + R_SA_PEG_AFELN0CFG0_OFFSET + (LANE_STEP * Lane), (UINT32) ~BIT9);
      } else {
        PciSegmentOr32 (PciePorts->ConfigSpaceBase + R_SA_PEG_AFELN0CFG0_OFFSET + (LANE_STEP * Lane), BIT9);
      }
  }
  }
  ///
  /// Configure Protocol Awareness for testing according to policy
  ///
  if (PciePeiPreMemConfig->PegRxCemNonProtocolAwareness == 1) {
    Data32And = (UINT32) ~(0x7 << 8);
    Data32Or = 0x5 << 8;
  if((PciePorts->Device == SA_PEG0_DEV_NUM) && (PciePorts->Function == SA_PEG0_FUN_NUM)){
    PciSegmentAndThenOr32 (Peg0BaseAddress + R_SA_PEG_REUT_PH_CTR_OFFSET, Data32And, Data32Or);
  }
     if(PciePorts->Device == SA_PEG3_DEV_NUM){
    PciSegmentAndThenOr32 (PciePorts->ConfigSpaceBase + R_SA_PEG_REUT_PH_CTR_OFFSET, Data32And, Data32Or);
  }
  }
}

/**
  Disable Spread Spectrum Clocking

  @param[in]  This                        - Low level function table
**/
VOID
EFIAPI
SklDisableSpreadSpectrumClocking (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  )
{
  ///
  /// Disable spread spectrum clocking
  ///
  PchPcrAndThenOr32 (
    PID_MODPHY0,
    R_HSIO_PCR_PLL_SSC_DWORD2,
    (UINT32) ~B_HSIO_PCR_PLL_SSC_DWORD2_SSCSEN,
    0
    );
  DEBUG ((DEBUG_INFO, "Disable PEG Spread Spectrum Clocking\n"));
}


/**
  Program VRefRxDet and allow the PCIe controllers to begin training

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports Pointer
**/
VOID
EFIAPI
SklProgramVRefRxDet (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts
  )
{
  UINT32            Mask32And;
    UINT32            Data32Or;

  Mask32And = (UINT32) (~(BIT18 | BIT17 | BIT16 | BIT15 | BIT14));
    Data32Or  = (0x0F << 14) ;
    PciSegmentAndThenOr32 (PciePorts->ConfigSpaceBase + R_SA_PEG_CMNCFG7_OFFSET, Mask32And, Data32Or);
}

/**
  Clear DEFER_OC and allow the PCIe controllers to begin training

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports Pointer
**/
VOID
EFIAPI
SklClearDeferOc (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts
  )
{
  PCIE_PEI_PREMEM_CONFIG  *PciePeiPreMemConfig;

  PciePeiPreMemConfig = ((SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData)->PciePeiPreMemConfig;

  if (!SaPolicyForceDisablesPort(PciePorts, PciePeiPreMemConfig)) {
    PciSegmentAnd32 (PciePorts->ConfigSpaceBase + R_SA_PEG_AFE_PWRON_OFFSET, (UINT32) ~BIT16);
  }
}

/**
  Set DisableAutoSpeedUp bit

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  DisableAutoSpeedUp          - New value for DisableAutoSpeedUp
**/
VOID
EFIAPI
SklSetDisableAutoSpeedUp (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  BOOLEAN                           DisableAutoSpeedUp
  )
{
  if (DisableAutoSpeedUp) {
    PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_CFG5_OFFSET, BIT9);
  } else {
    PciSegmentAnd32 (PciePort->ConfigSpaceBase + R_SA_PEG_CFG5_OFFSET, (UINT32) ~(BIT9));
  }
}

/**
  Performs any additional equalization programming that needs to be done after
  initial link training and endpoint detection

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to program Phase2 for
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
EFIAPI
SklPostDetectionEqProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  PCIE_PEI_PREMEM_CONFIG          *PciePeiPreMemConfig;
  EFI_STATUS                      Status;
  UINT64                          Peg0BaseAddress;
  UINT64                          Peg3BaseAddress;
  BOOLEAN                         Peg0AnyGen3Link;
  BOOLEAN                         Peg0AnyPh3Hijack;
  BOOLEAN                         Peg3Gen3Link;
  BOOLEAN                         Peg3Ph3Hijack;
  UINT8                           Peg0LaneReversal;
  UINT8                           Peg3LaneReversal;
  UINT8                           FullSwing;
  UINT8                           PreCursor;
  UINT8                           Cursor;
  UINT8                           PostCursor;
  UINT8                           PortIndex;
  UINT8                           Lane;
  UINT8                           Peg0PresenceDetect;
  UINT8                           EqMethod;

  PciePrivate         = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;
  PciePeiPreMemConfig = PciePrivate->PciePeiPreMemConfig;
  Peg0BaseAddress     = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  Peg0LaneReversal    = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_PEGTST_OFFSET) >> 20) & 0x1;
  Peg0AnyGen3Link     = FALSE;
  Peg0AnyPh3Hijack    = FALSE;
  Peg0PresenceDetect  = 0;
  Peg3BaseAddress     = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG3_DEV_NUM, SA_PEG3_FUN_NUM, 0);
  Peg3LaneReversal    = (PciSegmentRead32 (Peg3BaseAddress + R_SA_PEG_PEGTST_OFFSET) >> 20) & 0x1;
  Peg3Gen3Link        = FALSE;
  Peg3Ph3Hijack       = FALSE;
  FullSwing           = 0;
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].SwEqData.MaxCapableSpeed >= 3) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
        Peg0AnyGen3Link = TRUE;
      } else {
        Peg3Gen3Link = TRUE;
      }
    }
    if ((PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
      Peg0PresenceDetect |= (0x1 << PciePorts[PortIndex].Function);
    }
    EqMethod = SaPolicyGetEqPhase3Method (&PciePorts[PortIndex], PciePeiPreMemConfig);
    if (SaPolicySwEqEnabledOnPort (&PciePorts[PortIndex], PciePeiPreMemConfig) ||
       (EqMethod == PH3_METHOD_STATIC)) {
      if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
        Peg0AnyPh3Hijack = TRUE;
      } else {
        Peg3Ph3Hijack = TRUE;
      }
      PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_EQCFG_OFFSET, BIT1);
      ///
      /// Clear phase2 bypass if phase2 is enabled
      ///
      if (SaPolicyGetEqPhase2Enable (&PciePorts[PortIndex], PciePeiPreMemConfig)) {
        Status = This->SetPhase2Bypass (This, &PciePorts[PortIndex], 1, FALSE);
        ASSERT_EFI_ERROR (Status);
      }
    }
  }

  ///
  /// If any Gen3 device, setup equalization values and retrain link
  ///
  if (Peg0AnyGen3Link && Peg0AnyPh3Hijack) {
    ///
    /// Program presets based upon endpoint fullswing value
    ///
    for (Lane = 0; Lane < SA_PEG0_CNT_MAX_LANE; Lane++) {
      switch (Lane) {
        case  0:
          PciePrivate->GetLinkPartnerFullSwing (This, SA_PEG0_DEV_NUM, Lane, &FullSwing);
          break;
        case  8:
          if ((Peg0PresenceDetect & BIT1) == BIT1) {
            PciePrivate->GetLinkPartnerFullSwing (This, SA_PEG0_DEV_NUM, Lane, &FullSwing);
          }
          break;
        case 12:
          if ((Peg0PresenceDetect & BIT2) == BIT2) {
            PciePrivate->GetLinkPartnerFullSwing (This, SA_PEG0_DEV_NUM, Lane, &FullSwing);
          }
          break;
        default:
          break;
      }
      GetCoefficientsFromPreset (
        PciePeiPreMemConfig->PegGen3EndPointPreset[ReverseLane (SA_PEG0_DEV_NUM, Lane, Peg0LaneReversal) ],
        FullSwing,
        &PreCursor,
        &Cursor,
        &PostCursor
        );
      PciePrivate->SetPartnerTxCoefficients (This, SA_PEG0_DEV_NUM, Lane, &PreCursor, &Cursor, &PostCursor);
    }

    ///
    /// Redo EQ
    ///
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        ///
        /// Go to Gen1
        ///
        PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 1);
        This->RetrainLink (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        This->WaitForL0 (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        ///
        /// Go to Gen3
        ///
        PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL3_OFFSET, BIT0);  ///< DOEQ
#ifdef UP_SERVER_FLAG
		if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) >= PEG_GEN3 ||
            SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == 0) {
#endif
        PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 3);
#ifdef UP_SERVER_FLAG
        } else if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == PEG_GEN2) {
          PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 2);
        }
#endif
        This->RetrainLink (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        This->WaitForL0 (This, &(PciePorts[PortIndex]));
      }
    }
  }

  if (Peg3Gen3Link && Peg3Ph3Hijack) {
    ///
    /// Program presets based upon endpoint fullswing value
    ///
    for (Lane = 0; Lane < SA_PEG3_CNT_MAX_LANE; Lane++) {
      PciePrivate->GetLinkPartnerFullSwing (This, SA_PEG3_DEV_NUM, Lane, &FullSwing);
      GetCoefficientsFromPreset (
        PciePeiPreMemConfig->PegGen3EndPointPreset[ReverseLane (SA_PEG3_DEV_NUM, Lane, Peg3LaneReversal) ],
        FullSwing,
        &PreCursor,
        &Cursor,
        &PostCursor
        );
      PciePrivate->SetPartnerTxCoefficients (This, SA_PEG3_DEV_NUM, Lane, &PreCursor, &Cursor, &PostCursor);
    }

    ///
    /// Redo EQ
    ///
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        ///
        /// Go to Gen1
        ///
        PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 1);
        This->RetrainLink (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        This->WaitForL0 (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        ///
        /// Go to Gen3
        ///
        PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL3_OFFSET, BIT0);  ///< DOEQ
#ifdef UP_SERVER_FLAG
		if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) >= PEG_GEN3 ||
            SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == 0) {
#endif
        PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 3);
#ifdef UP_SERVER_FLAG
		} else if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == PEG_GEN2) {
          PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 2);
        }
#endif
        This->RetrainLink (This, &(PciePorts[PortIndex]));
      }
    }
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if ((PciePorts[PortIndex].Device == SA_PEG3_DEV_NUM) && (PciePorts[PortIndex].EndpointPresent)) {
        This->WaitForL0 (This, &(PciePorts[PortIndex]));
      }
    }
  }
}

/**
 This function programs Equalization Phase 2/3 Bypass

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Ports to sampler calibrate
  @param[in]  PciePortsLength             - Length of the PciePorts array
**/
VOID
EFIAPI
SklEqPh2Ph3BypassProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePorts,
  IN  UINT8                             PciePortsLength
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  PCIE_PEI_PREMEM_CONFIG          *PciePeiPreMemConfig;
  EFI_STATUS                      Status;
  UINT8                           PortIndex;
  BOOLEAN                         RedoEqNeeded[SA_PEG_MAX_FUN];
  GPIO_PAD                        GpioPad;
  UINT8                           GpioLevel;

  PciePrivate   = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;
  PciePeiPreMemConfig    = PciePrivate->PciePeiPreMemConfig;
  for (PortIndex = 0; PortIndex < SA_PEG_MAX_FUN; PortIndex++) {
      RedoEqNeeded[PortIndex] = FALSE;
  }

  ///
  /// After last equalization, set PH3 bypass if needed
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) >= PEG_GEN3 ||
        SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == 0) {
      if (SaPolicySwEqEnabledOnPort (&(PciePorts[PortIndex]), PciePeiPreMemConfig) ||
          (SaPolicyGetEqPhase3Method (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == PH3_METHOD_DISABLED)) {
        RedoEqNeeded[PortIndex] = TRUE;
        PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_EQCFG_OFFSET, BIT14);
      }
    }
  }
  ///
  /// Set Ph2 Bypass if enabled by SA policy
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) >= PEG_GEN3 ||
        SaPolicyGetPegMaxLinkSpeed (&(PciePorts[PortIndex]), PciePeiPreMemConfig) == 0) {
      if (SaPolicyGetEqPhase2Enable (&(PciePorts[PortIndex]), PciePeiPreMemConfig)) {
        RedoEqNeeded[PortIndex] = TRUE;
        Status = This->SetPhase2Bypass (This, &PciePorts[PortIndex], 1, FALSE);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_WARN, "Error clearing Phase2 bypass!\n"));
        }
      } else {
        Status = This->SetPhase2Bypass (This, &PciePorts[PortIndex], 1, TRUE);
        if (EFI_ERROR (Status)) {
          DEBUG ((DEBUG_WARN, "Error setting Phase2 bypass!\n"));
        }
      }
    }
  }
  ///
  /// Redo EQ
  ///
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].EndpointPresent && RedoEqNeeded[PortIndex]) {
      ///
      /// Go to Gen1
      ///
      PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 1);
      This->RetrainLink (This, &(PciePorts[PortIndex]));
    }
  }
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].EndpointPresent && RedoEqNeeded[PortIndex]) {
      This->WaitForL0(This, &(PciePorts[PortIndex]));
    }
  }
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].EndpointPresent && RedoEqNeeded[PortIndex]) {
      ///
      /// Go to Gen3
      ///
      PciSegmentOr32 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL3_OFFSET, BIT0);  ///< DOEQ
      PciSegmentAndThenOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL2_OFFSET, (UINT16) ~(0x0F), 3);
      This->RetrainLink (This, &(PciePorts[PortIndex]));
    }
  }
  for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
    if (PciePorts[PortIndex].EndpointPresent && RedoEqNeeded[PortIndex]) {
      This->WaitForL0 (This, &(PciePorts[PortIndex]));
    }
  }


  ///
  /// Make sure the link is operating at the max speed and width, if not attempt a reset
  ///
  if (PciePeiPreMemConfig->PegGpioData.GpioSupport) {
    for (PortIndex = 0; PortIndex < PciePortsLength; PortIndex++) {
      if (RedoEqNeeded[PortIndex] &&
            (((This->GetCurrentLinkSpeed (This, &(PciePorts[PortIndex])) < PEG_GEN3) &&
              (PciePorts[PortIndex].SwEqData.MaxCapableSpeed >= PEG_GEN3)) ||
            (This->GetNegotiatedWidth (This, &(PciePorts[PortIndex])) < PciePorts[PortIndex].SwEqData.MaxCapableWidth))) {
        DEBUG ((DEBUG_INFO, "Toggling PCIe slot PERST#.\n"));
        PciSegmentOr16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, BIT4);

        if (PciePorts[PortIndex].Device == SA_PEG0_DEV_NUM) {
          GpioPad = PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.GpioPad;
          GpioLevel = (UINT8)PciePeiPreMemConfig->PegGpioData.SaPeg0ResetGpio.Active;
        } else { // PEG3
          GpioPad = PciePeiPreMemConfig->PegGpioData.SaPeg3ResetGpio.GpioPad;
          GpioLevel = (UINT8)PciePeiPreMemConfig->PegGpioData.SaPeg3ResetGpio.Active;
        }
        if (GpioLevel == 1) {
          Status = This->SetPchGpio (This, GpioPad, 1);
        } else {
          Status = This->SetPchGpio (This, GpioPad, 0);
        }
        if (!EFI_ERROR (Status)) {
          MicroSecondDelay (100 * STALL_ONE_MICRO_SECOND);
          PciSegmentAnd16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, (UINT16) ~(BIT4));
          if (GpioLevel == 1) {
            Status = This->SetPchGpio (This, GpioPad, 0);
          } else {
            Status = This->SetPchGpio (This, GpioPad, 1);
          }
        } else {
          PciSegmentAnd16 (PciePorts[PortIndex].ConfigSpaceBase + R_SA_PEG_LCTL_OFFSET, (UINT16) ~(BIT4));
        }
        if (!EFI_ERROR (Status)) {
          This->WaitForL0 (This, &(PciePorts[PortIndex]));
        }
        This->ReportPcieLinkStatus (This, &(PciePorts[PortIndex]));
      }
    }
  }
}

/**
  Program EQ Phase1 preset value

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - PCIe Root Port
  @param[in]  Direction                   - 0 = Root Port, 1 = End Point
  @param[in]  PresetValue                 - Preset value to program
  @param[in]  LogicalLane                 - Logical Lane to be configured
**/
VOID
EFIAPI
SklProgramPhase1Preset (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             Direction,
  IN  UINT8                             PresetValue,
  IN  UINT8                             LogicalLane
  )
{
  UINT32                          Data32Or;
  UINT32                          Data32And;

  //
  // Adjust the LogicalLane for PEG1 and PEG2
  //
  if (PciePort->Device == SA_PEG0_DEV_NUM) {
    switch (PciePort->Function) {
      case SA_PEG1_FUN_NUM:
        if (LogicalLane < 8) {
          DEBUG ((DEBUG_WARN, "Invalid input to ProgramPreset() function!  PegDev = %d, PegFunc=%d, Lane=%d\n", PciePort->Device, PciePort->Function, LogicalLane));
          return;
        } else {
          LogicalLane -= 8;
        }
        break;
      case SA_PEG2_FUN_NUM:
        if (LogicalLane < 12) {
          DEBUG ((DEBUG_WARN, "Invalid input to ProgramPreset() function!  PegDev = %d, PegFunc=%d, Lane=%d\n", PciePort->Device, PciePort->Function, LogicalLane));
          return;
        } else {
          LogicalLane -= 12;
        }
        break;
      default:
        break;
    }
  }
  ///
  /// RP preset goes to bits [3:0]  for even lane and [19:16] for odd lane
  /// EP preset goes to bits [11:8] for even lane and [27:24] for odd lane
  ///
  if (Direction != 0) {
    if ((LogicalLane % 2) == 0) {
      Data32And = 0xFFFFF0FF;
      Data32Or  = PresetValue << 8;
    } else {
      Data32And = 0xF0FFFFFF;
      Data32Or  = PresetValue << 24;
    }
  } else {
    if ((LogicalLane % 2) == 0) {
      Data32And = 0xFFFFFFF0;
      Data32Or  = PresetValue;
    } else {
      Data32And = 0xFFF0FFFF;
      Data32Or  = PresetValue << 16;
    }
  }

  PciSegmentAndThenOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_EQCTL0_1_OFFSET + (LogicalLane / 2) * 4, Data32And, Data32Or);

  return;
}

/**
  Power Down Unused Lanes on the given PCIe Root Port

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
**/
VOID
EFIAPI
SklPowerDownUnusedLanes (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT64                          Peg0BaseAddress;
  UINT8                           LanesToPowerDown[SA_PEG_MAX_LANE];
  UINT8                           BundlesToPowerDown[SA_PEG_MAX_BUNDLE];
  UINT8                           LanesToPowerDownLength;
  UINT8                           BundlesToPowerDownLength;
  UINT8                           Width;
  UINT8                           LaneIndex;
  UINT8                           Index;
  UINT8                           Bundle;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  if (PciePort->EndpointPresent) {
    Width = PciePort->SwEqData.MaxCapableWidth;
  } else {
    ///
    ///
    /// If endpoint is not present then we should use the disable root port
    /// sequence to shut down all bundles
    ///
    return;
  }
  if (Width <= 0) {
    ///
    /// Powering down all bundles should be done by the disable root port sequence
    ///
    return;
  }
  if ((Width % 2) > 0) {
    ///
    /// If a bundle is partially in use (for example X1 link width) leave it powered up
    ///
    Width += 1;
  }
  LanesToPowerDownLength = PciePort->MaxPortWidth - Width;
  if (LanesToPowerDownLength > PciePort->MaxPortLaneListLength) {
    ///
    /// This should never happen, just to make sure a buffer overrun is impossible
    ///
    LanesToPowerDownLength = PciePort->MaxPortLaneListLength;
  }
  if (Width > LanesToPowerDownLength) {
    ///
    /// This should never happen, just to make sure a buffer overrun is impossible
    ///
    Width = LanesToPowerDownLength;
  }
  if (PciePort->Device == SA_PEG_DEV_NUM) {
    ASSERT (LanesToPowerDownLength <= SA_PEG0_CNT_MAX_LANE);
    if (LanesToPowerDownLength > SA_PEG0_CNT_MAX_LANE) {
      return;
    }
  } else if (PciePort->Device == SA_PEG3_DEV_NUM) {
    ASSERT (LanesToPowerDownLength <= SA_PEG3_CNT_MAX_LANE);
    if (LanesToPowerDownLength > SA_PEG3_CNT_MAX_LANE) {
      return;
    }
  }
  ///
  /// Create array of lanes to power down
  ///
  LaneIndex = Width;
  for (Index = 0; ((Index < LanesToPowerDownLength) && (LaneIndex < SA_PEG0_CNT_MAX_LANE)); Index++) {
    LanesToPowerDown[Index] = PciePort->MaxPortLaneList[LaneIndex];
    LaneIndex++;
  }
  ///
  /// Convert to bundles
  ///
  GetBundleList (
    &LanesToPowerDown[0],
    LanesToPowerDownLength,
    &BundlesToPowerDown[0],
    &BundlesToPowerDownLength
    );
  ///
  /// Power down unused lanes
  ///
  if (BundlesToPowerDownLength > 0) {
    DEBUG ((
      DEBUG_INFO,
      "PCIe RP (%x:%x:%x) - Powering Down Bundles[%d:%d]\n",
      PciePort->Bus,
      PciePort->Device,
      PciePort->Function,
      BundlesToPowerDown[0],
      BundlesToPowerDown[BundlesToPowerDownLength - 1]
      ));
  } else {
    DEBUG ((
      DEBUG_INFO,
      "PCIe RP (%x:%x:%x) - All Bundles Active, Skipping Bundle Power Down\n",
      PciePort->Bus,
      PciePort->Device,
      PciePort->Function
      ));
  }
  if (BundlesToPowerDownLength > GetMaxPegBundles()) {
    BundlesToPowerDownLength = GetMaxPegBundles();
  }
  for (Index = 0; Index < BundlesToPowerDownLength; Index++) {
    Bundle = BundlesToPowerDown[Index];
    if (PciePort->Device == SA_PEG0_DEV_NUM) {
      PciSegmentOr32 (Peg0BaseAddress + R_SA_PEG_BND0SPARE_OFFSET + (Bundle * BUNDLE_STEP), BIT31);
    } else if (PciePort->Device == SA_PEG3_DEV_NUM) {
      PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_BND0SPARE_OFFSET + (Bundle * BUNDLE_STEP), BIT31);
    }
  }
}

/**
  Power Down All Lanes on the given PCIe Root Port for CFL Platform

  @param[in]  This                        - Low level function table
  @param[in]  PegDevice                   - The device number of the PEG port to power down
  @param[in]  PegFunction                 - The function number of the PEG port to power down
**/
VOID
EFIAPI
SklPowerDownAllLanes (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDevice,
  IN  UINT8                             PegFunction
  )
{
  UINT64                          Peg0BaseAddress;
  UINT32                          HwStrap;
  UINT8                           LaneReversal;
  UINT8                           StartBundle;
  UINT8                           EndBundle;
  UINT8                           Bundle;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  HwStrap         = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_FUSESCMN_OFFSET) >> 16) & 0x3;
  StartBundle     = 0;
  EndBundle       = 0;

  if (PegDevice == SA_PEG_DEV_NUM) {
    LaneReversal    = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_PEGTST_OFFSET) >> 20) & 0x1;
      switch (PegFunction) {
      case SA_PEG0_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x16_x0_x0:
            StartBundle   = 0;
            EndBundle     = 7;
            break;
          default:
            if (LaneReversal == 0) {
              StartBundle = 0;
              EndBundle   = 3;
            } else {
              StartBundle = 4;
              EndBundle   = 7;
            }
            break;
        }
        break;
      case SA_PEG1_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x8_x8_x0:
            if (LaneReversal == 0) {
              StartBundle = 4;
              EndBundle   = 7;
            } else {
              StartBundle = 0;
              EndBundle   = 3;
            }
            break;
          case SA_PEG_x8_x4_x4:
            if (LaneReversal == 0) {
              StartBundle = 4;
              EndBundle   = 5;
            } else {
              StartBundle = 2;
              EndBundle   = 3;
            }
            break;
          default:
            return; ///< Nothing to do for PEG11
        }
        break;
      case SA_PEG2_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x8_x4_x4:
            if (LaneReversal == 0) {
              StartBundle = 6;
              EndBundle   = 7;
            } else {
              StartBundle = 0;
              EndBundle   = 1;
            }
            break;
          default:
            return; ///< Nothing to do for PEG12
        }
        break;
      default:
        return; ///< Invalid PEG Controller
    }
    ///
    /// Power down unused lanes
    ///
    DEBUG ((
      DEBUG_INFO,
      "PCIe RP (%x:%x:%x) - Powering Down Bundles[%d:%d]\n",
      SA_PEG_BUS_NUM,
      PegDevice,
      PegFunction,
      StartBundle,
      EndBundle
      ));
    for (Bundle = StartBundle; Bundle <= EndBundle; Bundle++) {
      PciSegmentOr32 (Peg0BaseAddress + R_SA_PEG_BND0SPARE_OFFSET + (Bundle * BUNDLE_STEP), BIT31);
    }
  } else {
      return; ///< Invalid PEG Controller
  }
}


/**
  Power Down All Lanes on the given PCIe Root Port for CNL Platform

  @param[in]  This                        - Low level function table
  @param[in]  PegDevice                   - The device number of the PEG port to power down
  @param[in]  PegFunction                 - The function number of the PEG port to power down
**/
VOID
EFIAPI
CnlPowerDownAllLanes (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDevice,
  IN  UINT8                             PegFunction
  )
{
  UINT64                          Peg0BaseAddress;
  UINT64                          Peg3BaseAddress;
  UINT32                          HwStrap;
  UINT8                           LaneReversal;
  UINT8                           StartBundle;
  UINT8                           EndBundle;
  UINT8                           Bundle;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  Peg3BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG3_DEV_NUM, SA_PEG3_FUN_NUM, 0);
  HwStrap         = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_FUSESCMN_OFFSET) >> 16) & 0x3;
  StartBundle     = 0;
  EndBundle       = 0;

  if (PegDevice == SA_PEG_DEV_NUM) {
    LaneReversal    = (PciSegmentRead32 (Peg0BaseAddress + R_SA_PEG_PEGTST_OFFSET) >> 20) & 0x1;
      switch (PegFunction) {
      case SA_PEG0_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x16_x0_x0:
            StartBundle   = 0;
            EndBundle     = 7;
            break;
          default:
            if (LaneReversal == 0) {
              StartBundle = 0;
              EndBundle   = 3;
            } else {
              StartBundle = 4;
              EndBundle   = 7;
            }
            break;
        }
        break;
      case SA_PEG1_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x8_x8_x0:
            if (LaneReversal == 0) {
              StartBundle = 4;
              EndBundle   = 7;
            } else {
              StartBundle = 0;
              EndBundle   = 3;
            }
            break;
          case SA_PEG_x8_x4_x4:
            if (LaneReversal == 0) {
              StartBundle = 4;
              EndBundle   = 5;
            } else {
              StartBundle = 2;
              EndBundle   = 3;
            }
            break;
          default:
            return; ///< Nothing to do for PEG11
        }
        break;
      case SA_PEG2_FUN_NUM:
        switch (HwStrap) {
          case SA_PEG_x8_x4_x4:
            if (LaneReversal == 0) {
              StartBundle = 6;
              EndBundle   = 7;
            } else {
              StartBundle = 0;
              EndBundle   = 1;
            }
            break;
          default:
            return; ///< Nothing to do for PEG12
        }
        break;
      default:
        return; ///< Invalid PEG Controller
    }
    ///
    /// Power down unused lanes
    ///
    DEBUG ((
      DEBUG_INFO,
      "PCIe RP (%x:%x:%x) - Powering Down Bundles[%d:%d]\n",
      SA_PEG_BUS_NUM,
      PegDevice,
      PegFunction,
      StartBundle,
      EndBundle
      ));
    for (Bundle = StartBundle; Bundle <= EndBundle; Bundle++) {
      PciSegmentOr32 (Peg0BaseAddress + R_SA_PEG_BND0SPARE_OFFSET + (Bundle * BUNDLE_STEP), BIT31);
    }
  } else if (PegDevice == SA_PEG3_DEV_NUM) {
    StartBundle = 0;
    EndBundle   = 1;
    ///
    /// Power down unused lanes
    ///
    DEBUG ((
      DEBUG_INFO,
      "PCIe RP (%x:%x:%x) - Powering Down Bundles[%d:%d]\n",
      SA_PEG_BUS_NUM,
      PegDevice,
      PegFunction,
      StartBundle,
      EndBundle
      ));
    for (Bundle = StartBundle; Bundle <= EndBundle; Bundle++) {
      PciSegmentOr32 (Peg3BaseAddress + R_SA_PEG_BND0SPARE_OFFSET + (Bundle * BUNDLE_STEP), BIT31);
    }
  } else {
    return; ///< Invalid PEG Controller
  }
}


/**
  Sets the link width

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  LinkWidth                   - Desired Link Width
**/
VOID
EFIAPI
SklSetLinkWidth (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  UINT8                             LinkWidth
  )
{
  UINT32            Data32Or;

  Data32Or = 0;
  if (LinkWidth != 0) {
    switch (LinkWidth) {
      case 1:
        Data32Or = BIT0;
        break;
      case 2:
        Data32Or = BIT1 | BIT0;
        break;
      case 4:
        Data32Or = BIT2 | BIT1 | BIT0;
        break;
      case 8:
        Data32Or = BIT3 | BIT2 | BIT1 | BIT0;
        break;
      case 16:
      default:
        Data32Or = BIT4 | BIT3 | BIT2 | BIT1 | BIT0;
        break;
    }
    PciSegmentAndThenOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_LTSSMC_OFFSET, (UINT32) ~(BIT4 | BIT3 | BIT2 | BIT1 | BIT0), Data32Or);

    This->SetLinkDisable (This, PciePort, TRUE);
    MicroSecondDelay (STALL_ONE_MICRO_SECOND);
    This->SetLinkDisable (This, PciePort, FALSE);

    This->WaitForL0 (This, PciePort);
  }
}

/**
  Additional Programming steps that need to be performed post endpoint detection

  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
**/
VOID
EFIAPI
SklPostDetectionAdditionalProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT32                       Data32And;
  UINT32                       Data32Or;
  UINT8                        Index;

  ///
  /// Set L0SLAT[15:0] to 0x2020
  ///
  ///
  ///
  Data32And = (UINT32) ~(0xFFFF);
  Data32Or  = 0x00002020;
  PciSegmentAndThenOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_L0SLAT_OFFSET, Data32And, Data32Or);
  ///
  /// Disable PEG Debug Align Message - set 258[29] = '1b'
  ///
  PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_CFG4_OFFSET, BIT29);

  ///
  ///
  PciSegmentOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGCLKGTCMN_OFFSET, (UINT32) BIT31);

  ///
  /// Retrain the link only if VC0 negotiation is complete at this point.
  /// This is to support CLB card together with "Aways Enable PEG" option
  ///
  if (This->DataLinkLayerLinkActive (This, PciePort)) {
    This->RetrainLink (This, PciePort);
    ///
    /// Wait for Link training complete
    ///
    for (Index = 0; Index < 100; Index++) {
      if ((PciSegmentRead16 (PciePort->ConfigSpaceBase + R_SA_PEG_LSTS_OFFSET) & BIT11) != 0) {
        break;
      }
      MicroSecondDelay (STALL_ONE_MILLI_SECOND);
    }
  }
}

/**
  Perform flow control credit programming

  @param[in]  This                        - Low level function table
  @param[in]  PegDisableMask              - Bitmap of controllers to disable by function number
**/
VOID
EFIAPI
SklFlowControlCreditProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT8                             PegDisableMask
  )
{
  UINT64                    MchBar;
  UINT8                     Index;
  UINT16                    NumberOfControlCreditTableEntry;
  CREDIT_CONTROL_INIT_ENTRY *CreditControlTablePtr;

  MchBar                          = ((SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData)->MchBar;
  Index                           = 0;
  NumberOfControlCreditTableEntry = 0;
  CreditControlTablePtr           = NULL;

#ifdef CPU_CFL
  ///
  /// Select the configuration base on which PEGs are enabled.
  ///
  if (((PegDisableMask >> 2) & 0x1) == 0) {
    CreditControlTablePtr = mCreditControlInitTableX8x4x4Ptr;
    NumberOfControlCreditTableEntry = (sizeof (mCreditControlInitTableX8x4x4Ptr) / sizeof (CREDIT_CONTROL_INIT_ENTRY));
    DEBUG ((DEBUG_INFO, "Program PEG flow control credit values for x8 x4 x4 configuration\n"));
  } else if (((PegDisableMask >> 1) & 0x1) == 0) {
    CreditControlTablePtr = mCreditControlInitTableX8x8Ptr;
    NumberOfControlCreditTableEntry = (sizeof (mCreditControlInitTableX8x8Ptr) / sizeof (CREDIT_CONTROL_INIT_ENTRY));
    DEBUG ((DEBUG_INFO, "Program PEG flow control credit values for x8 x8 x0 configuration\n"));
  } else {
    FlowControlCreditProgrammingNoPegLib();
  }
#else
  ///
  /// Select the configuration base on which PEGs are enabled.
  ///
  if (((PegDisableMask >> 2) & 0x1) == 0) {
    CreditControlTablePtr = mCreditControlInitTableX8x4x4Ptr;
    NumberOfControlCreditTableEntry = (sizeof (mCreditControlInitTableX8x4x4Ptr) / sizeof (CREDIT_CONTROL_INIT_ENTRY));
    DEBUG ((DEBUG_INFO, "Program PEG flow control credit values for x8 x4 x4 configuration\n"));
  } else if (((PegDisableMask >> 1) & 0x1) == 0) {
    CreditControlTablePtr = mCreditControlInitTableX8x8Ptr;
    NumberOfControlCreditTableEntry = (sizeof (mCreditControlInitTableX8x8Ptr) / sizeof (CREDIT_CONTROL_INIT_ENTRY));
    DEBUG ((DEBUG_INFO, "Program PEG flow control credit values for x8 x8 x0 configuration\n"));
  } else if (((PegDisableMask) & 0x1) == 0) {
    CreditControlTablePtr = mCreditControlInitTableX16Ptr;
    NumberOfControlCreditTableEntry = (sizeof (mCreditControlInitTableX16Ptr) / sizeof (CREDIT_CONTROL_INIT_ENTRY));
    DEBUG ((DEBUG_INFO, "Program PEG flow control credit values for x16 x0 x0 configuration\n"));
  } else {
    FlowControlCreditProgrammingNoPegLib();
  }
#endif
  if ((CreditControlTablePtr != NULL) && (NumberOfControlCreditTableEntry > 0)) {
    for (Index = 0; Index < NumberOfControlCreditTableEntry; Index++) {
      MmioWrite32 ((UINTN) MchBar + CreditControlTablePtr[Index].RegOffset, CreditControlTablePtr[Index].Value);
    }
  }
}
#ifdef CPU_CFL
/**
  Disable or enable Icomp for a root port
  @param[in]  This                        - Low level function table
  @param[in]  PciePort                    - PCIe Root Port
  @param[in]  Disable                     - TRUE to Disable, FALSE to Enable
**/
VOID
EFIAPI
SklSetRootPortIcompDisable (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort,
  IN  BOOLEAN                           Disable
  )
{
  UINT64                          Peg0BaseAddress;
  UINT8                           RootPortBundles[SA_PEG_MAX_BUNDLE];
  UINT8                           RootPortBundlesLength;
  UINT8                           Index;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_PEG_BUS_NUM, SA_PEG0_DEV_NUM, SA_PEG0_FUN_NUM, 0);
  ///
  /// Get the enpoint bundles from the max lane list
  ///
  GetBundleList (
    &PciePort->MaxPortLaneList[0],
    PciePort->MaxPortLaneListLength,
    &RootPortBundles[0],
    &RootPortBundlesLength
    );
  if (RootPortBundlesLength > GetMaxPegBundles()) {
    RootPortBundlesLength = GetMaxPegBundles();
  }
  if (Disable) {
    DEBUG ((DEBUG_INFO, "PEG %x:%x:%x Icomp Disabled\n", PciePort->Bus, PciePort->Device, PciePort->Function));
    for (Index = 0; Index < RootPortBundlesLength; Index++) {
      PciSegmentOr32 (Peg0BaseAddress + R_SA_PEG_AFEBND0CFG5_OFFSET + (RootPortBundles[Index] * BUNDLE_STEP), BIT17);
    }
  } else {
    DEBUG ((DEBUG_INFO, "PEG %x:%x:%x Icomp Enabled\n", PciePort->Bus, PciePort->Device, PciePort->Function));
    for (Index = 0; Index < RootPortBundlesLength; Index++) {
      PciSegmentAnd32 (Peg0BaseAddress + R_SA_PEG_AFEBND0CFG5_OFFSET + (RootPortBundles[Index] * BUNDLE_STEP), (UINT32) ~BIT17);
    }
  }
}
#endif

#ifndef CPU_CFL
/**
  Program RXSQEXCTL FUSE values
  @param[in]  Value                       - Value to be programmed
**/
VOID
EFIAPI
CnlSetRootPortRXSQEXCTL (
  IN  UINT32                             Value
  )
{
  UINT8                            RootPortBundlesLength;
  UINT8                            Index;
  UINT64                           DmiBar;
  UINT32                           Data32And;
  UINT32                           Data32Or;

  PciSegmentReadBuffer (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_DMIBAR), sizeof (DmiBar), &DmiBar);
  DmiBar &= ~((UINT64) BIT0);

  RootPortBundlesLength = GetMaxDmiBundles();

  DEBUG ((DEBUG_INFO, "DMI Program RXSQEXCTL to value %x \n", Value));
  for (Index = 0; Index < RootPortBundlesLength; Index++) {
  Data32And = (UINT32)~(BIT20 | BIT19 | BIT18);
  Data32Or  = ((Value & 0x0007)<< 18);
  MmioAndThenOr32 ((UINTN) (DmiBar + R_SA_PEG_AFEBND0CFG5_OFFSET + (Index * BUNDLE_STEP)), Data32And, Data32Or);
  }
}
#endif


/**
  Program the Spine Clock Gating feature according to the configuration

  @param[in]  This                        - Low level function table
  @param[in]  PciePorts                   - Root Port to check for VC0 negotiation complete
**/
VOID
SklSpineClockGatingProgramming (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  UINT32                          Data32Or;
  UINT32                          Data32And;
  CPU_SKU                         CpuSku;

  CpuSku        = GetCpuSku ();
  PciePrivate   = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;

  if ((CpuSku == EnumCpuHalo) || (CpuSku == EnumCpuTrad)) {
    ///
    /// Configure Spine Clock Gating for PEG
    ///
    Data32And = (UINT32) ~(BIT29 | BIT28 | BIT27);
    Data32Or = 0;
    DEBUG ((DEBUG_INFO, "Configure Spine Clock Gating for PEG\n"));
    ///
    /// Write the configuration for all PEGs.
    ///
    PciSegmentAndThenOr32 (PciePort->ConfigSpaceBase + R_SA_PEG_PEGCLKGTCMN_OFFSET, Data32And, Data32Or);
    ///
    /// Configure Spine Clock Gating for DMI
    ///
    Data32And = (UINT32) ~(BIT29 | BIT28 | BIT27);
    Data32Or = 0;
    DEBUG ((DEBUG_INFO, "Configure Spine Clock Gating for DMI\n"));
    ///
    /// Write the configuration for DMI.
    ///
    MmioAndThenOr32 ((UINTN) (PciePrivate->DmiBar + R_SA_PEG_PEGCLKGTCMN_OFFSET), Data32And, Data32Or);
  }
}
typedef union {
  struct {
    UINT32  Low;
    UINT32  High;
  } Data32;
  UINT64 Data;
} UINT64_STRUCT;
/**
  This function gets the private data for the SA PCIe low level functions

  @param[in]  IN  PCIE_PEI_PREMEM_CONFIG    - PciePeiPreMemConfig
  @param[in]  IN  SA_MISC_PEI_PREMEM_CONFIG - MiscPeiPreMemConfig
  @param[out] SaPciePrivateData   - Table of function calls for SA PEG

  @retval EFI_SUCCESS - Table of function calls returned successfully
**/
EFI_STATUS
GetSklPegPrivateData (
  IN  PCIE_PEI_PREMEM_CONFIG                  *PciePeiPreMemConfig,
  IN  SA_MISC_PEI_PREMEM_CONFIG               *MiscPeiPreMemConfig,
  OUT SA_PCIE_PRIVATE_FUNCTION_CALLS          *SaPciePrivateData
  )
{
  UINT64          McBaseAddress;
  UINT64_STRUCT   Bar64;

  McBaseAddress                                         = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
  SaPciePrivateData->PciePeiPreMemConfig                = PciePeiPreMemConfig;
  Bar64.Data32.High                                     = PciSegmentRead32 (McBaseAddress + R_SA_MCHBAR + 4);
  Bar64.Data32.Low                                      = PciSegmentRead32 (McBaseAddress + R_SA_MCHBAR);
  SaPciePrivateData->MchBar                             = Bar64.Data & ~BIT0;
  Bar64.Data32.High                                     = PciSegmentRead32 (McBaseAddress + R_SA_DMIBAR + 4);
  Bar64.Data32.Low                                      = PciSegmentRead32 (McBaseAddress + R_SA_DMIBAR);
  SaPciePrivateData->DmiBar                             = Bar64.Data & ~BIT0;
#ifdef CPU_CFL
  SaPciePrivateData->GdxcBar                            = MiscPeiPreMemConfig->GdxcBar;
  SaPciePrivateData->SetRootPortIcompDisable            = SklSetRootPortIcompDisable;
#endif
  SaPciePrivateData->IsGen3Capable                      = SklPegIsGen3Capable;
  SaPciePrivateData->PreDetectionProgramming            = SklPegPreDetectionProgramming;
#ifdef CPU_CFL
  SaPciePrivateData->PcieDmiRecipe                      = SklPegDmiRecipe;
#else
  SaPciePrivateData->PcieDmiRecipe                      = CnlPegDmiRecipe;
  SaPciePrivateData->SetRootPortRXSQEXCTL               = CnlSetRootPortRXSQEXCTL;
#endif
  SaPciePrivateData->ConfigureMaxSpeedWidth             = SklConfigureMaxSpeedWidth;
  SaPciePrivateData->EnableRxCemLoopbackMode            = SklEnableRxCemLoopbackMode;
  SaPciePrivateData->ProgramVRefRxDet                   = SklProgramVRefRxDet;
  SaPciePrivateData->ClearDeferOc                       = SklClearDeferOc;
  SaPciePrivateData->SetDisableAutoSpeedUp              = SklSetDisableAutoSpeedUp;
  SaPciePrivateData->PostDetectionEqProgramming         = SklPostDetectionEqProgramming;
  SaPciePrivateData->GetLinkPartnerFullSwing            = GetLinkPartnerFullSwing;
  SaPciePrivateData->SetPartnerTxCoefficients           = SetPartnerTxCoefficients;
  SaPciePrivateData->EqPh2Ph3BypassProgramming          = SklEqPh2Ph3BypassProgramming;
  SaPciePrivateData->ProgramPhase1Preset                = SklProgramPhase1Preset;
  SaPciePrivateData->PowerDownUnusedLanes               = SklPowerDownUnusedLanes;
#ifdef CPU_CFL
  SaPciePrivateData->PowerDownAllLanes                  = SklPowerDownAllLanes;
#else
  SaPciePrivateData->PowerDownAllLanes                  = CnlPowerDownAllLanes;
#endif
  SaPciePrivateData->SetLinkWidth                       = SklSetLinkWidth;
  SaPciePrivateData->PostDetectionAdditionalProgramming = SklPostDetectionAdditionalProgramming;
  SaPciePrivateData->DisableUnusedPcieControllers       = SklDisableUnusedPcieControllers;
  SaPciePrivateData->FlowControlCreditProgramming       = SklFlowControlCreditProgramming;
  SaPciePrivateData->SpineClockGatingProgramming        = SklSpineClockGatingProgramming;
  SaPciePrivateData->DisableSpreadSpectrumClocking      = SklDisableSpreadSpectrumClocking;

  return EFI_SUCCESS;
}
