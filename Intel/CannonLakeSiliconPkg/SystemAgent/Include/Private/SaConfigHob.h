/** @file
  The GUID definition for SaConfigHob

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2018 Intel Corporation.

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
#ifndef _SA_CONFIG_HOB_H_
#define _SA_CONFIG_HOB_H_

#include <SaAccess.h>
#include <Base.h>

extern EFI_GUID gSaConfigHobGuid;

#pragma pack (push,1)
///
/// DPR Directory Types
///
typedef enum {
  EnumDprDirectoryTxt   = 0,
  EnumDprDirectoryBiosGuard
} DPR_DIRECTORY_ELEMENT;

#define DPR_DIRECTORY_TYPE_TXT          0x01      ///< DPR directory type - TXT
#define DPR_DIRECTORY_TYPE_BIOSGUARD    0x02      ///< DPR directory type - BIOS Guard
#define DPR_DIRECTORY_MAX               2         ///< DPR Maximum Size

///
/// DPR directory entry definition
///
typedef struct {
  UINT8   Type;          ///< DPR Directory Type
  UINT8   Size;          ///< DPR Size in MB
  UINT32  PhysBase;      ///< Must be 4K aligned (bits 11..0 must be clear)
  UINT16  Reserved;      ///< Must be 0
} DPR_DIRECTORY_ENTRY;

///
/// The data elements should be initialized by a Platform Module.
/// The data structure is for VT-d driver initialization
///
typedef struct {
  BOOLEAN               VtdDisable;                        ///< 1 = Avoids programming Vtd bars, Vtd overrides and DMAR table
  UINT32                BaseAddress[SA_VTD_ENGINE_NUMBER]; ///< This field is used to describe the base addresses for VT-d function
  BOOLEAN               X2ApicOptOut;                      ///< This field is used to enable the X2APIC_OPT_OUT bit in the DMAR table. <b>1=Enable/Set</b> and 0=Disable/Clear
  BOOLEAN               InterruptRemappingSupport;         ///< This field is used to indicate Interrupt Remapping supported or not
} SA_VTD_CONFIGURATION_HOB;

///
/// SA GPIO Data Structure
///
typedef struct {
  UINT8   ExpanderNo; ///< =Expander No For I2C based GPIO
  UINT32  GpioNo;     ///< GPIO pad
  BOOLEAN Active;     ///< 0=Active Low; 1=Active High
} SA_GPIO;

///
/// SA PCIE RTD3 GPIO Data Structure
///
typedef struct {
  UINT8              GpioSupport;      ///< 0=Not Supported; 1=PCH based; 2=I2C Based
  SA_GPIO            HoldRst;          ///< Offset 8 This field contain PCIe HLD RESET GPIO value and level information
  SA_GPIO            PwrEnable;        ///< This field contain PCIe PWR Enable GPIO value and level information
  UINT32             WakeGpioNo;       ///< This field contain PCIe RTD3 Device Wake GPIO number
} PCIE_RTD3_GPIO;

///
/// SG Info HOB
///
typedef struct {
  SG_MODE           SgMode;
  UINT8             RootPortIndex;
  PCIE_RTD3_GPIO    Rtd3Pcie0Gpio;
  PCIE_RTD3_GPIO    Rtd3Pcie1Gpio;
  PCIE_RTD3_GPIO    Rtd3Pcie2Gpio;
#ifndef CPU_CFL
  PCIE_RTD3_GPIO    Rtd3Pcie3Gpio;
#endif
  UINT16            DelayAfterPwrEn;
  UINT16            DelayAfterHoldReset;
} SA_RTD3;

///
/// System Agent Config Hob
///
typedef struct {
  EFI_HOB_GUID_TYPE        EfiHobGuidType;                           ///< GUID Hob type structure for gSaConfigHobGuid
  DPR_DIRECTORY_ENTRY      DprDirectory[DPR_DIRECTORY_MAX];          ///< DPR directory entry definition
  BOOLEAN                  InitPcieAspmAfterOprom;                   ///< 1=initialize PCIe ASPM after Oprom; 0=before (This will be set basing on policy)
  SA_RTD3                  SaRtd3;                                   ///< SG Info HOB
  UINT8                    ApertureSize;                             ///< Aperture size value
  UINT8                    IpuAcpiMode;                              ///< IPU ACPI mode: 0=Disabled, 1=IGFX Child device, 2=ACPI device
  SA_VTD_CONFIGURATION_HOB VtdData;                                  ///< VT-d Data HOB
  BOOLEAN                  CridEnable;                               ///< This field inidicates if CRID is enabled or disabled (to support Intel(R) SIPP)
  BOOLEAN                  SkipPamLock;                              ///< 0=All PAM registers will be locked in System Agent code, 1=Do not lock PAM registers in System Agent code.
  UINT8                    PowerDownUnusedBundles[SA_PEG_MAX_FUN];   ///< PCIe power down unused bundles support
  UINT8                    PegMaxPayload[SA_PEG_MAX_FUN];            ///< PEG Max Pay Load Size (0xFF: Auto, 0:128B, 1:256B)
} SA_CONFIG_HOB;
#pragma pack (pop)
#endif
