## @file
#  Component description file for the CannonLake SiPkg both Pei and Dxe libraries DSC file.
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
# Silicon Init Common Library
#

#
# Set PCH generation according PCD.
# The DEFINE will be used to select PCH library INF file corresponding to PCH generation
#
DEFINE  PCH = Cnl

#
# Common
#
 MmPciLib|$(PLATFORM_SI_PACKAGE)/Library/PeiDxeSmmMmPciLib/PeiDxeSmmMmPciLib.inf
 UsbLib|$(PLATFORM_SI_PACKAGE)/Library/PeiDxeSmmUsbLib/PeiDxeSmmUsbLib.inf
 UsbInitLib|$(PLATFORM_SI_PACKAGE)/Library/Private/PeiDxeUsbInitLib/PeiDxeUsbInitLib.inf

#
# Cpu
#
 SecCpuLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/SecCpuLib/SecCpuLib.inf
 TxtLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeTxtLib/PeiDxeTxtLib.inf
 CpuPlatformLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmCpuPlatformLib/PeiDxeSmmCpuPlatformLib.inf

!if gSiPkgTokenSpaceGuid.PcdBootGuardEnable == TRUE
 BootGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmBootGuardLib/PeiDxeSmmBootGuardLib.inf
!else
 BootGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmBootGuardLibNull/PeiDxeSmmBootGuardLibNull.inf
!endif

 SoftwareGuardLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/Private/PeiDxeSoftwareGuardLib/PeiDxeSoftwareGuardLib.inf
 CpuMailboxLib|$(PLATFORM_SI_PACKAGE)/Cpu/Library/PeiDxeSmmCpuMailboxLib/PeiDxeSmmCpuMailboxLib.inf

#
# Me
#
!if gSiPkgTokenSpaceGuid.PcdSpsEnable == FALSE # UP_SPS_SUPPORT
 MeTypeLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiDxeMeTypeLib/PeiDxeMeTypeLib.inf
!endif # UP_SPS_SUPPORT
 PttPtpLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiDxePttPtpLib/PeiDxePttPtpLib.inf
 MeChipsetLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiDxeMeChipsetLib/PeiDxeMeChipsetLib.inf
 MeShowBufferLib|$(PLATFORM_SI_PACKAGE)/Me/Library/PeiDxeMeShowBufferLib/PeiDxeMeShowBufferLib.inf
 MeFwStsLib|$(PLATFORM_SI_PACKAGE)/Me/Library/BaseMeFwStsLib/BaseMeFwStsLib.inf
 HeciInitLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/PeiDxeHeciInitLib/PeiDxeHeciInitLib.inf
 AlertStandardFormatLib|$(PLATFORM_SI_PACKAGE)/Me/Library/Private/PeiDxeAlertStandardFormatLib/PeiDxeAlertStandardFormatLib.inf
#
# Pch
#
 SecPchLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/SecPchLib/SecPchLib.inf
 PchCycleDecodingLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchCycleDecodingLib/PeiDxeSmmPchCycleDecodingLib.inf
 PchGbeLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchGbeLib/PeiDxeSmmPchGbeLib.inf
 GbeMdiLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmGbeMdiLib/PeiDxeSmmGbeMdiLib.inf
 PchInfoLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchInfoLib/PeiDxeSmmPchInfoLib$(PCH).inf
 SataLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmSataLib/PeiDxeSmmSataLib$(PCH).inf
 PchPcieRpLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchPcieRpLib/PeiDxeSmmPchPcieRpLib.inf
 PchPcrLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchPcrLib/PeiDxeSmmPchPcrLib.inf
 PmcLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPmcLib/PeiDxeSmmPmcLib.inf

 PchSbiAccessLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchSbiAccessLib/PeiDxeSmmPchSbiAccessLib.inf
 GpioLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmGpioLib/PeiDxeSmmGpioLib.inf
!if gSiPkgTokenSpaceGuid.PcdTraceHubEnable == TRUE
 PchTraceHubLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchTraceHubLib/PeiDxeSmmPchTraceHubLib.inf
!else
 PchTraceHubLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/BasePchTraceHubLibNull/BasePchTraceHubLibNull.inf
!endif
!if gSiPkgTokenSpaceGuid.PcdSerialIoUartEnable == TRUE
 PchSerialIoUartLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchSerialIoUartLib/PeiDxeSmmPchSerialIoUartLib.inf
!else
 PchSerialIoUartLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/BasePchSerialIoUartLibNull/BasePchSerialIoUartLibNull.inf
!endif
 PchSerialIoLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchSerialIoLib/PeiDxeSmmPchSerialIoLibCnl.inf
 PchEspiLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchEspiLib/PeiDxeSmmPchEspiLib.inf
 PchWdtCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmPchWdtCommonLib/PeiDxeSmmPchWdtCommonLib.inf
 ResetSystemLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/BaseResetSystemLib/BaseResetSystemLib.inf
 SmbusLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/BaseSmbusLib/BaseSmbusLib.inf
 BiosLockLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmBiosLockLib/PeiDxeSmmBiosLockLib.inf
 CnviLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmCnviLib/PeiDxeSmmCnviLib.inf
 ItssLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/PeiDxeSmmItssLib/PeiDxeSmmItssLib.inf
 #private
 PchPciExpressHelpersLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchPciExpressHelpersLib/PeiDxeSmmPchPciExpressHelpersLib.inf
 PchInitCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchInitCommonLib/PeiDxeSmmPchInitCommonLib.inf
 PchSmbusCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchSmbusCommonLib/PeiDxeSmmPchSmbusCommonLib.inf
 PchSpiCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/BasePchSpiCommonLib/BasePchSpiCommonLib.inf
 GpioPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmGpioPrivateLib/PeiDxeSmmGpioPrivateLibCnl.inf
 PchAlternateAccessModeLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchAlternateAccessModeLib/PeiDxeSmmPchAlternateAccessModeLib.inf
 PchXhciLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchXhciLib/PeiDxeSmmPchXhciLib.inf
 PeiDxeI2cMasterCommonLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeI2cMasterCommonLib/PeiDxeI2cMasterCommonLib.inf
 PchPsfPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchPsfPrivateLib/PeiDxeSmmPchPsfPrivateLib$(PCH).inf
 PeiRstPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiRstPrivateLib/PeiRstPrivateLib.inf
 PeiSataPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiSataPrivateLib/PeiSataPrivateLib.inf
 PeiGbeInitLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiGbeInitLib/PeiGbeInitLib.inf
 PeiHsioLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiHsioLib/PeiHsioLibCnl.inf
 PmcPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPmcPrivateLib/PeiDxeSmmPmcPrivateLibCnl.inf
 PmcPrivateLibWithS3|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPmcPrivateLib/PeiDxeSmmPmcPrivateLibWithS3.inf
 PchFiaLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchFiaLib/PeiDxeSmmPchFiaLib$(PCH).inf
 PchDmiLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchDmiLib/PeiDxeSmmPchDmiLib.inf
 PchDmiWithS3Lib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmPchDmiLib/PeiDxeSmmPchDmiWithS3Lib.inf
 DciPrivateLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiDxeSmmDciPrivateLib/PeiDxeSmmDciPrivateLib.inf
 SiScheduleResetLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/BaseSiScheduleResetLib/BaseSiScheduleResetLib.inf
 PeiHybridStorageLib|$(PLATFORM_SI_PACKAGE)/Pch/Library/Private/PeiHybridStorageLib/PeiHybridStorageLib.inf

#
# SA
#
 SecSaLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/SecSaLib/SecSaLib.inf
 SaPlatformLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/Library/PeiDxeSmmSaPlatformLib/PeiDxeSmmSaPlatformLib.inf

#
# Memory
#
!if gSiPkgTokenSpaceGuid.PcdCflCpuEnable == TRUE
 MemoryAddressEncodeLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/MemoryInitCfl/Library/PeiDxeSmmMemAddrEncodeLib/PeiDxeSmmMemAddrEncodeLib.inf
!else
 MemoryAddressEncodeLib|$(PLATFORM_SI_PACKAGE)/SystemAgent/MemoryInit/Library/PeiDxeSmmMemAddrEncodeLib/PeiDxeSmmMemAddrEncodeLib.inf
!endif

