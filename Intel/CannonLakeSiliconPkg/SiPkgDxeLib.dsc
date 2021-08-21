# @file
#  Component description file for the CannonLake SiPkg DXE libraries.
#
# @copyright
#  INTEL CONFIDENTIAL
#  Copyright 2012 - 2019 Intel Corporation.
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
# Silicon Init Dxe Library
#

#
# Common
#
!if gSiPkgTokenSpaceGuid.PcdAcpiEnable == TRUE
 AslUpdateLib|$(PLATFORM_SI_PACKAGE)/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf
!else
 AslUpdateLib|$(PLATFORM_SI_PACKAGE)/Library/DxeAslUpdateLibNull/DxeAslUpdateLibNull.inf
!endif
 SiFviLib|$(PLATFORM_SI_PACKAGE)/Library/DxeSiFviLib/DxeSiFviLib.inf
 SiFviInitLib|$(PLATFORM_SI_PACKAGE)/Library/Private/DxeSiFviInitLib/DxeSiFviInitLib.inf
 SiConfigBlockLib|$(PLATFORM_SI_PACKAGE)/Library/BaseSiConfigBlockLib/BaseSiConfigBlockLib.inf

#
# Cpu
#
 CpuCommonLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSmmCpuCommonLib/PeiDxeSmmCpuCommonLib.inf
!if gSiPkgTokenSpaceGuid.PcdSoftwareGuardEnable == TRUE
 SoftwareGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSoftwareGuardLib/PeiDxeSoftwareGuardLib.inf
!else
 SoftwareGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/BaseSoftwareGuardLibNull/BaseSoftwareGuardLibNull.inf
!endif
 SmmCpuFeaturesLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/SmmCpuFeaturesLib/SmmCpuFeaturesLib.inf

#
# Pch
#
 PchHdaLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/DxePchHdaLib/DxePchHdaLib.inf
 ResetSystemLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/DxeResetSystemLib/DxeResetSystemLib.inf
 DxePchPolicyLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/DxePchPolicyLib/DxePchPolicyLib.inf
 GpioHelpersLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/BaseGpioHelpersLibNull/BaseGpioHelpersLibNull.inf
 GpioNameBufferLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/DxeGpioNameBufferLib/DxeGpioNameBufferLib.inf
 SmmPchPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/SmmPchPrivateLib/SmmPchPrivateLib.inf
 TopSwapLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/SmmTopSwapLib/SmmTopSwapLib.inf
 DxeHdaNhltLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/DxeHdaNhltLib/DxeHdaNhltLib.inf

#
# Me
#
!if gSiPkgTokenSpaceGuid.PcdAmtEnable == TRUE
 DxeAmtLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAmtLib/DxeAmtLib.inf
 DxeAmtPolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAmtPolicyLib/DxeAmtPolicyLib.inf
 DxeAsfLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAsfLib/DxeAsfLib.inf
!else
 DxeAmtLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAmtLibNull/DxeAmtLibNull.inf
 DxeAmtPolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAmtPolicyLibNull/DxeAmtPolicyLibNull.inf
 DxeAsfLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeAsfLibNull/DxeAsfLibNull.inf
!endif
 DxeMeLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeMeLib/DxeMeLib.inf
 MbpDataLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeMbpDataLib/DxeMbpDataLib.inf
 DxeMePolicyLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeMePolicyLib/DxeMePolicyLib.inf
!if gSiPkgTokenSpaceGuid.PcdSpsEnable == TRUE # UP_SPS_SUPPORT
 MeTypeLib|$(PLATFORM_SI_PACKAGE)/Me/Library/MeTypeLib/MeTypeDxeLib.inf # UP_SPS_SUPPORT
!endif # UP_SPS_SUPPORT
 PttHeciLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxePttHeciLib/DxePttHeciLib.inf
!if gSiPkgTokenSpaceGuid.PcdIntegratedTouchEnable == TRUE
 DxeTouchHeciMsgsLib|$(PLATFORM_SI_PACKAGE)/Me/Library/DxeTouchHeciMsgsLib/DxeTouchHeciMsgsLib.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdJhiEnable == TRUE
 DxeJhiSupportLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/DxeJhiSupportLib/DxeJhiSupportLib.inf
 DxeBeihaiLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/DxeBeihaiLib/DxeBeihaiLib.inf
!endif

#
# SystemAgent
#
 DxeSaPolicyLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/DxeSaPolicyLib/DxeSaPolicyLib.inf
 SaPcieLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/DxeSmmSaPcieLib/DxeSmmSaPcieLib.inf
!if gSiPkgTokenSpaceGuid.PcdSmbiosEnable == TRUE
 SmbiosMemoryLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/DxeSmbiosMemoryLib/DxeSmbiosMemoryLib.inf
!else
 SmbiosMemoryLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/Private/DxeSmbiosMemoryLibNull/DxeSmbiosMemoryLibNull.inf
!endif
 PciHostBridgeLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/DxePciHostBridgeLib/DxePciHostBridgeLib.inf
