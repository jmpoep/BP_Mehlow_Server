/** @file
  PCH LPC Library

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

#include <Uefi/UefiBaseType.h>
#include <Ppi/SiPolicy.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PchEspiLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Private/Library/PeiLpcLib.h>
#include <Private/Library/PchDmiLib.h>
#include <ConfigBlock/SerialIrqConfig.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsLpc.h>
#include <Library/PchInfoLib.h>

/**
  Returns PCH LPC device PCI base address.

  @retval                   PCH LPC PCI base address.
**/
STATIC
UINT64
LpcPciBase (
  VOID
  )
{
  return PCI_SEGMENT_LIB_ADDRESS (
           DEFAULT_PCI_SEGMENT_NUMBER_PCH,
           DEFAULT_PCI_BUS_NUMBER_PCH,
           PCI_DEVICE_NUMBER_PCH_LPC,
           PCI_FUNCTION_NUMBER_PCH_LPC,
           0
           );
}

/**
  Configure LPC power management.

  @param[in] SiPolicy                  The SI Policy instance
**/
VOID
LpcConfigurePm (
  IN  SI_POLICY_PPI                    *SiPolicy
  )
{
  UINT64                                PciLpcRegBase;
  EFI_STATUS                            Status;
  PCH_PM_CONFIG                         *PmConfig;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);

  PciLpcRegBase = LpcPciBase ();

  ///
  /// BIOS need to set LPC PCR 0x341C[13,3:0] to all 1's and [8,9,10] = 0's
  ///
  PchPcrAndThenOr32 (PID_LPC, R_LPC_PCR_PRC, (UINT32) ~(BIT10 | BIT9 | BIT8), (BIT13 | BIT3 | BIT2 | BIT1 | BIT0));

  if (IsEspiEnabled ()) {
    ///
    /// BIOS needs to set LPC PCR 0x3418[2] to 1 if eSPI mode is enabled
    ///
    PchPcrAndThenOr32 (PID_LPC, R_LPC_PCR_GCFD, 0xFFFFFFFF, B_LPC_PCR_GCFD_SRVR_CLKRUN_EN);
  } else {
    ///
    /// The PCLKVLD_CFG does need to be configured to "10" (2 clock cycle delay).
    ///
    PciSegmentOr16 (PciLpcRegBase + R_LPC_CFG_PCC, 2 << N_LPC_CFG_PCC_PCLKVLD_CFG);
    ///
    /// System BIOS is also required to set following bit.
    /// PCI CLKRUN# Enable" bit (LPC PCI offset E0h[0]) = 1b
    /// Below setting only applicable when in LPC mode
    ///
    PciSegmentAnd16 (PciLpcRegBase + R_LPC_CFG_PCC, (UINT16) (~B_LPC_CFG_PCC_CLKRUN_EN));
    if (PmConfig->LpcClockRun) {
      PciSegmentOr16 (PciLpcRegBase + R_LPC_CFG_PCC, B_LPC_CFG_PCC_CLKRUN_EN);
    }
  }
}

/**
  Configure LPC device on early PEI.
**/
VOID
LpcOnEarlyPeiConfigure (
  VOID
  )
{
  DEBUG ((DEBUG_INFO, "LpcOnEarlyPeiConfigure() \n"));

  //
  // Enhance port 8xh LPC decoding.
  // The can be disable by policy EnhancePort8xhDecoding.
  //
  PchLpcGenIoRangeSet (0x80, 0x10);
}

/**
  Configure LPC device on Policy callback.

  @param[in] SiPreMemPolicyPpi         The SI PreMem Policy PPI instance
**/
VOID
LpcOnPolicyConfigure (
  IN  SI_PREMEM_POLICY_PPI             *SiPreMemPolicyPpi
  )
{
  PCH_LPC_GEN_IO_RANGE_LIST             LpcGenIoRangeList;
  UINT64                                LpcBase;
  UINTN                                 Index;
  EFI_STATUS                            Status;
  PCH_LPC_PREMEM_CONFIG                 *LpcPreMemConfig;

  DEBUG ((DEBUG_INFO, "LpcOnPolicyConfigure()\n"));

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gLpcPreMemConfigGuid, (VOID *) &LpcPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  //
  // If EnhancePort8xhDecoding is disabled, clear LPC and DMI LPC general IO range register.
  //
  if (LpcPreMemConfig->EnhancePort8xhDecoding == 0) {
    DEBUG ((DEBUG_INFO, "Disable EnhancePort8xhDecoding\n"));
    SetMem (&LpcGenIoRangeList, sizeof (PCH_LPC_GEN_IO_RANGE_LIST), 0);
    PchLpcGenIoRangeGet (&LpcGenIoRangeList);
    for (Index = 0; Index < PCH_LPC_GEN_IO_RANGE_MAX; Index++) {
      if ((LpcGenIoRangeList.Range[Index].BaseAddr == 0x80) &&
          (LpcGenIoRangeList.Range[Index].Length   == 0x10)) {
        //
        // Clear the LPC general IO range register and DMI LPC general IO range register.
        //
        LpcBase = LpcPciBase ();
        PciSegmentWrite32 (
          LpcBase + R_LPC_CFG_GEN1_DEC + Index * 4,
          0
          );
        PchDmiClearLpcGenIoRange (Index);
        break;
      }
    }
  }
}

/**
  The function performs Serial IRQ specific programming.

  @param[in] SiPolicyPpi               The SI Policy PPI instance
**/
VOID
LpcConfigureSerialIrq (
  IN  SI_POLICY_PPI                    *SiPolicyPpi
  )
{
  UINT64                               PciLpcRegBase;
  UINT8                                RegData8;
  EFI_STATUS                           Status;
  PCH_LPC_SIRQ_CONFIG                  *SerialIrqConfig;

  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gSerialIrqConfigGuid, (VOID *) &SerialIrqConfig);
  ASSERT_EFI_ERROR (Status);

  if (SerialIrqConfig->SirqEnable == FALSE) {
    return;
  }

  PciLpcRegBase = LpcPciBase ();
  RegData8        = 0;

  ///
  /// PCH BIOS Spec Section 6.3 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function in LPC PCI offset 64h[7] and
  /// select continuous or quiet mode, LPC PCI offset 64h[6].
  /// PCH requires that the System BIOS first set the SERIRQ logic to continuous mode operation for at least one frame
  /// before switching it into quiet mode operation. This operation should be performed during the normal boot sequence
  /// as well as a resume from STR (S3).
  ///
  RegData8  = PciSegmentRead8 (PciLpcRegBase + R_LPC_CFG_SERIRQ_CNT);
  RegData8  &= (UINT8) ~(B_LPC_CFG_SERIRQ_CNT_SIRQEN | B_LPC_CFG_SERIRQ_CNT_SFPW);

  switch (SerialIrqConfig->StartFramePulse) {
    case PchSfpw8Clk:
      RegData8 |= V_LPC_CFG_SERIRQ_CNT_SFPW_8CLK;
      break;

    case PchSfpw6Clk:
      RegData8 |= V_LPC_CFG_SERIRQ_CNT_SFPW_6CLK;
      break;

    case PchSfpw4Clk:
    default:
      RegData8 |= V_LPC_CFG_SERIRQ_CNT_SFPW_4CLK;
      break;
  }
  ///
  /// Set the SERIRQ logic to continuous mode
  ///
  RegData8 |= (UINT8) (B_LPC_CFG_SERIRQ_CNT_SIRQEN | B_LPC_CFG_SERIRQ_CNT_SIRQMD);

  PciSegmentWrite8 (PciLpcRegBase + R_LPC_CFG_SERIRQ_CNT, RegData8);

  ///
  /// PCH BIOS Spec Section 6.3 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function
  /// in LPC PCI offset 64h[7] and select continuous or quiet mode, LPC PCI offset 64h[6].
  ///
  if (SerialIrqConfig->SirqMode == PchQuietMode) {
    PciSegmentAnd8 (PciLpcRegBase + R_LPC_CFG_SERIRQ_CNT, (UINT8) ~B_LPC_CFG_SERIRQ_CNT_SIRQMD);
  }
}

