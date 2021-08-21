## @file
#  Component description file for the CannonLake SiPkg PEI libraries.
#
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 2012 - 2018 Intel Corporation.
#
#  The source code contained or described herein and all documents related to the
#  source code ("Material") are owned by Intel Corporation or its suppliers or
#  licensors. Title to the Material remains with Intel Corporation or its suppliers
#  and licensors. The Material may contain trade secrets and proprietary and
#  confidential information of Intel Corporation and its suppliers and licensors,
#  and is protected by worldwide copyright and trade secret laws and treaty
#  provisions. No part of the Material may be used, copied, reproduced, modified,
#  published, uploaded, posted, transmitted, distributed, or disclosed in any way
#  without Intel's prior express written permission.
#
#  No license under any patent, copyright, trade secret or other intellectual
#  property right is granted to or conferred upon you by disclosure or delivery
#  of the Materials, either expressly, by implication, inducement, estoppel or
#  otherwise. Any license under such intellectual property rights must be
#  express and approved by Intel in writing.
#
#  Unless otherwise agreed by Intel in writing, you may not remove or alter
#  this notice or any other notice embedded in Materials by Intel or
#  Intel's suppliers or licensors in any way.
#
#  This file contains an 'Intel Peripheral Driver' and is uniquely identified as
#  "Intel Reference Module" and is licensed for Intel CPUs and chipsets under
#  the terms of your license agreement with Intel or your vendor. This file may
#  be modified by the user, subject to additional terms of the license agreement.
#
#@par Specification Reference:
#
##

#
# Silicon Init Pei Library
#
 SiPolicyLib|$(PLATFORM_SI_PACKAGE)/Library/PeiSiPolicyLib/PeiSiPolicyLib.inf
 SiConfigBlockLib|$(PLATFORM_SI_PACKAGE)/Library/BaseSiConfigBlockLib/BaseSiConfigBlockLib.inf
 SiFviInitLib|$(PLATFORM_SI_PACKAGE)/Library/Private/PeiSiFviInitLib/PeiSiFviInitLib.inf
 PeiCpuAndPchTraceHubLib|$(PLATFORM_SI_PACKAGE)/Library/Private/PeiCpuAndPchTraceHubLib/PeiCpuAndPchTraceHubLib.inf
 StallPpiLib|$(PLATFORM_SI_PACKAGE)/Library/PeiInstallStallPpiLib/PeiStallPpiLib.inf
 SiMtrrLib|$(PLATFORM_SI_PACKAGE)/Library/SiMtrrLib/SiMtrrLib.inf

#
# Pch
#
 PchPolicyLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiPchPolicyLib/PeiPchPolicyLibCnl.inf
 PchInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiPchInitLib/PeiPchInitLibCnl.inf
 PchRcLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiPchRcLib/PeiPchRcLibCnl.inf
 PeiP2sbPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiP2sbPrivateLib/PeiP2sbPrivateLibCnl.inf
 PeiPchHdaLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiPchHdaLib/PeiPchHdaLib.inf
 PeiPchIshLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiPchIshLib/PeiPchIshLib.inf
 PeiPchPcieLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiPchPcieLib/PeiPchPcieLib.inf
 PeiPchPcieClocksLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiPchPcieClocksLib/PeiPchPcieClocksLib.inf
!if gSiPkgTokenSpaceGuid.PcdS3Enable == TRUE
 PchSmmControlLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiPchSmmControlLib/PeiPchSmmControlLib.inf
!else
 PchSmmControlLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiPchSmmControlLibNull/PeiPchSmmControlLibNull.inf
!endif
 PeiI2cMasterLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiI2cMasterLib/PeiI2cMasterLib.inf
!if gSiPkgTokenSpaceGuid.PcdOcWdtEnable == TRUE
 OcWdtLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiOcWdtLib/PeiOcWdtLib.inf
!else
 OcWdtLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiOcWdtLibNull/PeiOcWdtLibNull.inf
!endif
 ResetSystemLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiResetSystemLib/PeiResetSystemLib.inf
 PchResetLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiPchResetLib/PeiPchResetLib.inf
 SpiLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiSpiLib/PeiSpiLib.inf
 GpioHelpersLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiGpioHelpersLib/PeiGpioHelpersLib.inf
 PeiThermalLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiThermalLib/PeiThermalLibCnl.inf
 GpioNameBufferLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiGpioNameBufferLib/PeiGpioNameBufferLib.inf
 PeiCnviPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiCnviPrivateLib/PeiCnviPrivateLibCnl.inf
 PeiPchDmiLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchDmiLib/PeiPchDmiLib.inf
 PeiPmcPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPmcPrivateLib/PeiPmcPrivateLibCnl.inf
 PeiItssLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiItssLib/PeiItssLibCnl.inf
 PeiRtcLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiRtcLib/PeiRtcLib.inf
 PeiWdtLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiWdtLib/PeiWdtLib.inf
 PeiPchTraceHubLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchTraceHubLib/PeiPchTraceHubLib.inf
 PeiSmbusLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiSmbusLib/PeiSmbusLib.inf
 PeiScsLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiScsLib/PeiScsLibCnl.inf
 PeiEspiInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiEspiInitLib/PeiEspiInitLib.inf
 PeiLpcLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiLpcLib/PeiLpcLib.inf
 PeiDciLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmDciPrivateLib/PeiDciLib.inf
 PeiIshInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiIshInitLib/PeiIshInitLib.inf
 PeiSerialIoInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiSerialIoInitLib/PeiSerialIoInitLib.inf
 PeiHdaInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiHdaInitLib/PeiHdaInitLib.inf
#
# Cpu
#
 CpuCommonLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSmmCpuCommonLib/PeiDxeSmmCpuCommonLib.inf
 CpuInitLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuInitLib/PeiCpuInitLib.inf
!if gSiPkgTokenSpaceGuid.PcdBiosGuardEnable == TRUE
 BiosGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiBiosGuardLib/PeiBiosGuardLib.inf
!else
 BiosGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiBiosGuardLibNull/PeiBiosGuardLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdOverclockEnable == TRUE
 CpuOcLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcLib/PeiCpuOcLib.inf
 CpuOcInitLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcInitLib/PeiCpuOcInitLib.inf
!else
 CpuOcLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcLibNull/PeiCpuOcLibNull.inf
 CpuOcInitLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuOcInitLibNull/PeiCpuOcInitLibNull.inf
!endif
 CpuPowerMgmtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuPowerMgmtLib/PeiCpuPowerMgmtLib.inf
!if gSiPkgTokenSpaceGuid.PcdTxtEnable == TRUE
 PeiTxtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiTxtLib/PeiTxtLib.inf
!else
 PeiTxtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiTxtLibNull/PeiTxtLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdCpuPowerOnConfigEnable == TRUE
 CpuPowerOnConfigLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuPowerOnConfigLib/PeiCpuPowerOnConfigLib.inf
!else
 CpuPowerOnConfigLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiCpuPowerOnConfigLibDisable/PeiCpuPowerOnConfigLibDisable.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdSoftwareGuardEnable == TRUE
 SoftwareGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSoftwareGuardLib/PeiDxeSoftwareGuardLib.inf
!else
 SoftwareGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/BaseSoftwareGuardLibNull/BaseSoftwareGuardLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdSmbiosEnable == TRUE
 SmbiosCpuLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiSmbiosCpuLib/PeiSmbiosCpuLib.inf
!else
 SmbiosCpuLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiSmbiosCpuLibNull/PeiSmbiosCpuLibNull.inf
!endif

#
# Me
#
 MeInitLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/PeiMeInitLib/PeiMeInitLib.inf
 PeiMeLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiMeLib/PeiMeLib.inf
 PeiMePolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiMePolicyLib/PeiMePolicyLib.inf
!if gSiPkgTokenSpaceGuid.PcdSpsEnable == TRUE # UP_SPS_SUPPORT
 MeTypeLib|$(PLATFORM_SI_PACKAGE)/Me/Library/MeTypeLib/MeTypePeiLib.inf # UP_SPS_SUPPORT
!endif # UP_SPS_SUPPORT
!if gSiPkgTokenSpaceGuid.PcdAmtEnable == TRUE
 PeiAmtPolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiAmtPolicyLib/PeiAmtPolicyLib.inf
 PeiAmtLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiAmtLib/PeiAmtLib.inf
 ActiveManagementLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/PeiActiveManagementLib/PeiActiveManagementLib.inf
!else
 PeiAmtPolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiAmtPolicyLibNull/PeiAmtPolicyLibNull.inf
 PeiAmtLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiAmtLibNull/PeiAmtLibNull.inf
 ActiveManagementLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/PeiActiveManagementLibNull/PeiActiveManagementLibNull.inf
!endif

#
# SA
#
!if gSiPkgTokenSpaceGuid.PcdCflCpuEnable == TRUE
 MemoryInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/MemoryInitCfl/Library/Private/PeiMemoryInitLib/PeiMemoryInitLib.inf
!else
 MemoryInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/MemoryInit/LibraryPrivate/PeiMemoryInitLib/PeiMemoryInitLib.inf
!endif

!if gSiPkgTokenSpaceGuid.PcdSgEnable == TRUE
 SwitchableGraphicsInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSwitchableGraphicsInitLib/PeiSwitchableGraphicsInitLib.inf
!else
 SwitchableGraphicsInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSwitchableGraphicsInitLibNull/PeiSwitchableGraphicsInitLibNull.inf
!endif
 SaInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaInitLib/PeiSaInitLib.inf
!if gSiPkgTokenSpaceGuid.PcdIgdEnable == TRUE
 GraphicsInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiGraphicsInitLib/PeiGraphicsInitLib.inf
!else
 GraphicsInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiGraphicsInitLib/PeiGraphicsInitLibDisable.inf
!endif

!if gSiPkgTokenSpaceGuid.PcdPeiDisplayEnable == TRUE
 DisplayInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiDisplayInitLib/PeiDisplayInitLib.inf
!else
 DisplayInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiDisplayInitLibNull/PeiDisplayInitLibNull.inf
!endif

 PcieInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiPcieInitLib/PeiPcieInitLib.inf
!if (gSiPkgTokenSpaceGuid.PcdPegEnable == TRUE) OR (gSiPkgTokenSpaceGuid.PcdSaDmiEnable == TRUE)
 SaPcieDmiLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaPcieDmiLib/PeiSaPcieDmiLib.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdPegEnable == TRUE
 SaPcieInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaPcieInitLib/PeiSaPcieInitLib.inf
!else
 SaPcieInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaPcieInitLib/PeiSaPcieInitLibDisable.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdSaDmiEnable == TRUE
 SaDmiInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaDmiInitLib/PeiSaDmiInitLib.inf
!else
 SaDmiInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaDmiInitLibNull/PeiSaDmiInitLibNull.inf
!endif
!if (gSiPkgTokenSpaceGuid.PcdIpuEnable == TRUE) AND (gSiPkgTokenSpaceGuid.PcdCflCpuEnable == FALSE)
 IpuInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiIpuInitLib/PeiIpuInitLib.inf
!else
 IpuInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiIpuInitLibNull/PeiIpuInitLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdGnaEnable == TRUE
 GnaInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiGnaInitLib/PeiGnaInitLib.inf
!else
 GnaInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiGnaInitLibNull/PeiGnaInitLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdSaOcEnable == TRUE
 SaOcInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaOcInitLib/PeiSaOcInitLib.inf
!else
 SaOcInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiSaOcInitLibNull/PeiSaOcInitLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdVtdEnable == TRUE
 VtdInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiVtdInitLib/PeiVtdInitLib.inf
!else
 VtdInitLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiVtdInitLibNull/PeiVtdInitLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdS3Enable == TRUE
 SmmAccessLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/PeiSmmAccessLib/PeiSmmAccessLib.inf
!else
 SmmAccessLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/PeiSmmAccessLibNull/PeiSmmAccessLibNull.inf
!endif

  PeiCpuTraceHubLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/PeiCpuTraceHubLib/PeiCpuTraceHubLib.inf

  PeiSaPolicyLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/PeiSaPolicyLib/PeiSaPolicyLib.inf
#
# Cpu
#
 CpuPolicyLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiCpuPolicyLib/PeiCpuPolicyLib.inf

