/** @file
  SwitchableGraphics Pei driver.
  This Pei driver initialize GPIO programming
  for the platform.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2017 Intel Corporation.

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
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PciSegmentLib.h>
#include <CpuRegs.h>
#include <PchAccess.h>
#include <SaAccess.h>
#include <PcieRegs.h>
#include <Private/Library/SwitchableGraphicsInitLib.h>
#include <Library/PeiServicesLib.h>
#include <Private/SaConfigHob.h>
#include <Library/GpioLib.h>
#include <Library/GpioNativeLib.h>


/**
  Initialize the SwitchableGraphics support (PEI).

  @param[in]
  IN   SWITCHABLE_GRAPHICS_CONFIG        *SgGpioData,
  IN   SA_MISC_PEI_PREMEM_CONFIG         *MiscPeiPreMemConfig to access the SwitchableGraphicsInit related information
**/
VOID
SwitchableGraphicsInit (
  IN   SWITCHABLE_GRAPHICS_CONFIG        *SgGpioData,
  IN   SA_MISC_PEI_PREMEM_CONFIG         *MiscPeiPreMemConfig
  )
{
  SA_CONFIG_HOB             *SaConfigHob;

  DEBUG ((DEBUG_INFO, "SG:: SwitchableGraphics PEI initialization Starting\n"));
  ///
  /// Get SaConfigHob HOB
  ///
  SaConfigHob = NULL;
  SaConfigHob = (SA_CONFIG_HOB *) GetFirstGuidHob (&gSaConfigHobGuid);

  if (SaConfigHob != NULL) {
    SaConfigHob->SaRtd3.SgMode               = MiscPeiPreMemConfig->SgMode;
    SaConfigHob->SaRtd3.DelayAfterHoldReset  = MiscPeiPreMemConfig->SgDelayAfterHoldReset;
    SaConfigHob->SaRtd3.DelayAfterPwrEn      = MiscPeiPreMemConfig->SgDelayAfterPwrEn;

    ///
    /// GPIO Assigned from policy
    ///
    SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.GpioSupport = SgGpioData->SaRtd3Pcie0Gpio.GpioSupport;
    SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.GpioSupport = SgGpioData->SaRtd3Pcie1Gpio.GpioSupport;
    SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.GpioSupport = SgGpioData->SaRtd3Pcie2Gpio.GpioSupport;
#ifndef CPU_CFL
    SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.GpioSupport = SgGpioData->SaRtd3Pcie3Gpio.GpioSupport;
#endif

    ///
    /// GPIO for PCIe 0
    ///
    if (SgGpioData->SaRtd3Pcie0Gpio.GpioSupport != NotSupported) {
      if (SgGpioData->SaRtd3Pcie0Gpio.GpioSupport == PchGpio) {
        SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.HoldRst.GpioNo   = SgGpioData->SaRtd3Pcie0Gpio.HoldRst.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.PwrEnable.GpioNo = SgGpioData->SaRtd3Pcie0Gpio.PwrEnable.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.WakeGpioNo       = SgGpioData->SaRtd3Pcie0Gpio.WakeGpioNo;
      }
      SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.HoldRst.ExpanderNo   = SgGpioData->SaRtd3Pcie0Gpio.HoldRst.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.PwrEnable.ExpanderNo = SgGpioData->SaRtd3Pcie0Gpio.PwrEnable.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.HoldRst.Active       = SgGpioData->SaRtd3Pcie0Gpio.HoldRst.Active;
      SaConfigHob->SaRtd3.Rtd3Pcie0Gpio.PwrEnable.Active     = SgGpioData->SaRtd3Pcie0Gpio.PwrEnable.Active;
      SaConfigHob->SaRtd3.RootPortIndex = SgGpioData->RootPortIndex;
    }

    ///
    /// GPIO for PCIe 1
    ///
    if (SgGpioData->SaRtd3Pcie1Gpio.GpioSupport != NotSupported) {
      if (SgGpioData->SaRtd3Pcie1Gpio.GpioSupport == PchGpio) {
        SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.HoldRst.GpioNo   = SgGpioData->SaRtd3Pcie1Gpio.HoldRst.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.PwrEnable.GpioNo = SgGpioData->SaRtd3Pcie1Gpio.PwrEnable.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.WakeGpioNo       = SgGpioData->SaRtd3Pcie1Gpio.WakeGpioNo;
      }
      SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.HoldRst.ExpanderNo   = SgGpioData->SaRtd3Pcie1Gpio.HoldRst.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.PwrEnable.ExpanderNo = SgGpioData->SaRtd3Pcie1Gpio.PwrEnable.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.HoldRst.Active       = SgGpioData->SaRtd3Pcie1Gpio.HoldRst.Active;
      SaConfigHob->SaRtd3.Rtd3Pcie1Gpio.PwrEnable.Active     = SgGpioData->SaRtd3Pcie1Gpio.PwrEnable.Active;
    }

    ///
    /// GPIO for PCIe 2
    ///
    if (SgGpioData->SaRtd3Pcie2Gpio.GpioSupport != NotSupported) {
      if (SgGpioData->SaRtd3Pcie2Gpio.GpioSupport == PchGpio) {
        SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.HoldRst.GpioNo   = SgGpioData->SaRtd3Pcie2Gpio.HoldRst.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.PwrEnable.GpioNo = SgGpioData->SaRtd3Pcie2Gpio.PwrEnable.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.WakeGpioNo       = SgGpioData->SaRtd3Pcie2Gpio.WakeGpioNo;
      }
      SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.HoldRst.ExpanderNo   = SgGpioData->SaRtd3Pcie2Gpio.HoldRst.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.PwrEnable.ExpanderNo = SgGpioData->SaRtd3Pcie2Gpio.PwrEnable.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.HoldRst.Active       = SgGpioData->SaRtd3Pcie2Gpio.HoldRst.Active;
      SaConfigHob->SaRtd3.Rtd3Pcie2Gpio.PwrEnable.Active     = SgGpioData->SaRtd3Pcie2Gpio.PwrEnable.Active;
    }

#ifndef CPU_CFL
    ///
    /// GPIO for PCIe 3
    ///
    if (SgGpioData->SaRtd3Pcie2Gpio.GpioSupport != NotSupported) {
      if (SgGpioData->SaRtd3Pcie2Gpio.GpioSupport == PchGpio) {
        SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.HoldRst.GpioNo = SgGpioData->SaRtd3Pcie3Gpio.HoldRst.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.PwrEnable.GpioNo = SgGpioData->SaRtd3Pcie2Gpio.PwrEnable.GpioNo;
        SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.WakeGpioNo = SgGpioData->SaRtd3Pcie3Gpio.WakeGpioNo;
      }
      SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.HoldRst.ExpanderNo = SgGpioData->SaRtd3Pcie3Gpio.HoldRst.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.PwrEnable.ExpanderNo = SgGpioData->SaRtd3Pcie3Gpio.PwrEnable.ExpanderNo;
      SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.HoldRst.Active = SgGpioData->SaRtd3Pcie3Gpio.HoldRst.Active;
      SaConfigHob->SaRtd3.Rtd3Pcie3Gpio.PwrEnable.Active = SgGpioData->SaRtd3Pcie3Gpio.PwrEnable.Active;
    }
#endif
  }

  DEBUG ((DEBUG_INFO, "SG:: SwitchableGraphics PEI initialization Ends\n"));
}
