/** @file
  Print whole PCH_PREMEM_POLICY_PPI

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
#include "PeiPchPolicyLibrary.h"

/**
  Print PCH_GENERAL_PREMEM_CONFIG and serial out.

  @param[in] PchGeneralPreMemConfig     Pointer to a PCH_GENERAL_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintGeneralPreMemConfig (
  IN CONST PCH_GENERAL_PREMEM_CONFIG    *PchGeneralPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ PCH General PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, " Port80Route= %x\n", PchGeneralPreMemConfig->Port80Route));
}

/**
  Print PCH_DCI_PREMEM_CONFIG and serial out.

  @param[in] DciPreMemConfig            Pointer to a PCH_DCI_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintDciPreMemConfig (
  IN CONST PCH_DCI_PREMEM_CONFIG        *DciPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ PCH DCI PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "PlatformDebugConsent = %x\n", DciPreMemConfig->PlatformDebugConsent));
  DEBUG ((DEBUG_INFO, "DciUsb3TypecUfpDbg = %x\n", DciPreMemConfig->DciUsb3TypecUfpDbg));
}

/**
  Print PCH_WDT_PREMEM_CONFIG and serial out.

  @param[in] WdtPreMemConfig            Pointer to a PCH_WDT_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintWdtPreMemConfig (
  IN CONST PCH_WDT_PREMEM_CONFIG               *WdtPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ PCH WDT PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "DisableAndLock= %x\n", WdtPreMemConfig->DisableAndLock));
}

/**
  Print PCH_TRACE_HUB_CONFIG and serial out.

  @param[in] TraceHubConfig             Pointer to a PCH_TRACE_HUB_CONFIG that provides the platform setting

**/
VOID
PchPrintTraceHubPreMemConfig (
  IN CONST PCH_TRACE_HUB_PREMEM_CONFIG         *PchTraceHubPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ PCH TraceHub PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "EnableMode= %x\n", PchTraceHubPreMemConfig->EnableMode));
  DEBUG ((DEBUG_INFO, "MemReg0Size= %x\n", PchTraceHubPreMemConfig->MemReg0Size));
  DEBUG ((DEBUG_INFO, "MemReg1Size= %x\n", PchTraceHubPreMemConfig->MemReg1Size));
}

/**
  Print PCH_SMBUS_CONFIG and serial out.

  @param[in] SmbusConfig         Pointer to a PCH_SMBUS_CONFIG that provides the platform setting

**/
VOID
PchPrintSmbusPreMemConfig (
  IN CONST PCH_SMBUS_PREMEM_CONFIG    *SmbusPreMemConfig
  )
{
  UINT32 Index;

  DEBUG ((DEBUG_INFO, "------------------ PCH SMBUS PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, " Enable= %x\n", SmbusPreMemConfig->Enable));
  DEBUG ((DEBUG_INFO, " ArpEnable= %x\n", SmbusPreMemConfig->ArpEnable));
  DEBUG ((DEBUG_INFO, " DynamicPowerGating= %x\n", SmbusPreMemConfig->DynamicPowerGating));
  DEBUG ((DEBUG_INFO, " SpdWriteDisable= %x\n", SmbusPreMemConfig->SpdWriteDisable));
  DEBUG ((DEBUG_INFO, " SmbAlertEnable= %x\n", SmbusPreMemConfig->SmbAlertEnable));
  DEBUG ((DEBUG_INFO, " SmbusIoBase= %x\n", SmbusPreMemConfig->SmbusIoBase));
  DEBUG ((DEBUG_INFO, " NumRsvdSmbusAddresses= %x\n", SmbusPreMemConfig->NumRsvdSmbusAddresses));
  DEBUG ((DEBUG_INFO, " RsvdSmbusAddressTable= {"));
  for (Index = 0; Index < SmbusPreMemConfig->NumRsvdSmbusAddresses; ++Index) {
    DEBUG ((DEBUG_INFO, " %02xh", SmbusPreMemConfig->RsvdSmbusAddressTable[Index]));
  }
  DEBUG ((DEBUG_INFO, " }\n"));
}

/**
  Print PCH_LPC_PREMEM_CONFIG and serial out.

  @param[in] LpcPreMemConfig                  Pointer to a PCH_LPC_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintLpcPreMemConfig (
  IN CONST PCH_LPC_PREMEM_CONFIG              *LpcPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ PCH LPC PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, "EnhancePort8xhDecoding= %x\n", LpcPreMemConfig->EnhancePort8xhDecoding));
}

/**
  Print PCH_HSIO_PCIE_PREMEM_CONFIG and serial out.

  @param[in] HsioPciePreMemConfig     Pointer to a PCH_HSIO_PCIE_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintHsioPciePreMemConfig (
  IN CONST PCH_HSIO_PCIE_PREMEM_CONFIG *HsioPciePreMemConfig
  )
{
  UINT32 Index;

  DEBUG ((DEBUG_INFO, "------------------ HSIO PCIE PreMem Config ------------------\n"));
  for (Index = 0; Index < GetPchMaxPciePortNum (); Index++) {
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioRxSetCtleEnable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioRxSetCtleEnable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioRxSetCtle= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioRxSetCtle));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen1DownscaleAmpEnable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen1DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen1DownscaleAmp= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen1DownscaleAmp));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DownscaleAmpEnable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DownscaleAmp= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DownscaleAmp));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen3DownscaleAmpEnable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen3DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen3DownscaleAmp= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen3DownscaleAmp));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen1DeEmphEnable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen1DeEmphEnable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen1DeEmph= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen1DeEmph));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DeEmph3p5Enable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DeEmph3p5Enable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DeEmph3p5= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DeEmph3p5));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DeEmph6p0Enable= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DeEmph6p0Enable));
    DEBUG ((DEBUG_INFO, " RootPort[%d] HsioTxGen2DeEmph6p0= %x\n", Index, HsioPciePreMemConfig->Lane[Index].HsioTxGen2DeEmph6p0));
  }
}

/**
  Print PCH_HSIO_SATA_PREMEM_CONFIG and serial out.

  @param[in] SataCtrlIndex            SATA controller index
  @param[in] HsioSataPreMemConfig     Pointer to a PCH_HSIO_SATA_PREMEM_CONFIG that provides the platform setting
**/
VOID
PchPrintHsioSataPreMemConfig (
  IN UINT8                             SataCtrlIndex,
  IN CONST PCH_HSIO_SATA_PREMEM_CONFIG *HsioSataPreMemConfig
  )
{
  UINT32 Index;

  DEBUG ((DEBUG_INFO, "---------------- HSIO SATA PreMem Config for controller %d ----------------\n", SataCtrlIndex));
  for (Index = 0; Index < GetPchMaxSataPortNum (SataCtrlIndex); Index++) {
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen1EqBoostMagEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen1EqBoostMagEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen1EqBoostMag= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen1EqBoostMag));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen2EqBoostMagEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen2EqBoostMagEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen2EqBoostMag= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen2EqBoostMag));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen3EqBoostMagEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen3EqBoostMagEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioRxGen3EqBoostMag= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioRxGen3EqBoostMag));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen1DownscaleAmpEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen1DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen1DownscaleAmp= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen1DownscaleAmp));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen2DownscaleAmpEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen2DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen2DownscaleAmp= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen2DownscaleAmp));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen3DownscaleAmpEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen3DownscaleAmpEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen3DownscaleAmp= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen3DownscaleAmp));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen1DeEmphEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen1DeEmphEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen1DeEmph= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen1DeEmph));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen2DeEmphEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen2DeEmphEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen2DeEmph= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen2DeEmph));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen3DeEmphEnable= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen3DeEmphEnable));
    DEBUG ((DEBUG_INFO, " PortSettings[%d] HsioTxGen3DeEmph= %x\n", Index, HsioSataPreMemConfig->PortLane[Index].HsioTxGen3DeEmph));
  }
}

/**
  Print PCH_PCIE_RP_PREMEM_CONFIG and serial out.

  @param[in] PcieRpPreMemConfig        Pointer to a PCH_PCIE_RP_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintPcieRpPreMemConfig (
  IN CONST PCH_PCIE_RP_PREMEM_CONFIG    *PcieRpPreMemConfig
  )
{
  UINT32 Index;
  DEBUG ((DEBUG_INFO, "------------------ PCH PCIe RP PreMem Config ------------------\n"));

  for (Index = 0; Index < GetPchMaxPciePortNum (); Index++) {
    DEBUG ((DEBUG_INFO, " Port[%d] RpEnabled= %x\n", Index, (PcieRpPreMemConfig->RpEnabledMask & (UINT32) (1 << Index)) != 0 ));
  }
  DEBUG ((DEBUG_INFO, " PcieImrEnabled= %x\n", PcieRpPreMemConfig->PcieImrEnabled));
  DEBUG ((DEBUG_INFO, " PcieImrSize= %d MB\n", PcieRpPreMemConfig->PcieImrSize));
  DEBUG ((DEBUG_INFO, " ImrRpSelection= %d\n", PcieRpPreMemConfig->ImrRpSelection));
}

/**
  Print PCH_HDAUDIO_PREMEM_CONFIG and serial out.

  @param[in] LpcPreMemConfig                  Pointer to a PCH_HDAUDIO_PREMEM_CONFIG that provides the platform setting

**/
VOID
PchPrintHdAudioPreMemConfig (
  IN CONST PCH_HDAUDIO_PREMEM_CONFIG *HdaPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ HD Audio PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, " Enable= %x\n", HdaPreMemConfig->Enable));
}

/**
  Print PCH_ISH_PREMEM_CONFIG  and serial out.

  @param[in] IshPreMemConfig                  Pointer to a PCH_ISH_PREMEM_CONFIG  that provides the platform setting

**/
VOID
PchPrintIshPreMemConfig (
  IN CONST PCH_ISH_PREMEM_CONFIG *IshPreMemConfig
  )
{
  DEBUG ((DEBUG_INFO, "------------------ ISH PreMem Config ------------------\n"));
  DEBUG ((DEBUG_INFO, " Enable= %x\n", IshPreMemConfig->Enable));
}


/**
  Print whole PCH_POLICY_PPI and serial out.

  @param[in] SiPreMemPolicyPpi    The RC PREMEM Policy PPI instance

**/
VOID
EFIAPI
PchPreMemPrintPolicyPpi (
  IN  SI_PREMEM_POLICY_PPI     *SiPreMemPolicyPpi
  )
{
  DEBUG_CODE_BEGIN ();
  EFI_STATUS                      Status;
  PCH_GENERAL_PREMEM_CONFIG       *PchGeneralPreMemConfig;
  PCH_DCI_PREMEM_CONFIG           *DciPreMemConfig;
  PCH_WDT_PREMEM_CONFIG           *WdtPreMemConfig;
  PCH_TRACE_HUB_PREMEM_CONFIG     *PchTraceHubPreMemConfig;
  PCH_SMBUS_PREMEM_CONFIG         *SmbusPreMemConfig;
  PCH_LPC_PREMEM_CONFIG           *LpcPreMemConfig;
  PCH_HSIO_PCIE_PREMEM_CONFIG     *HsioPciePreMemConfig;
  PCH_HSIO_SATA_PREMEM_CONFIG     *HsioSataPreMemConfig;
  PCH_PCIE_RP_PREMEM_CONFIG       *PcieRpPreMemConfig;
  PCH_HDAUDIO_PREMEM_CONFIG       *HdaPreMemConfig;
  PCH_ISH_PREMEM_CONFIG           *IshPreMemConfig;
  UINT8                           SataCtrlIndex;

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPchGeneralPreMemConfigGuid, (VOID *) &PchGeneralPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gDciPreMemConfigGuid, (VOID *) &DciPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gWatchDogPreMemConfigGuid, (VOID *) &WdtPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPchTraceHubPreMemConfigGuid, (VOID *) &PchTraceHubPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gSmbusPreMemConfigGuid, (VOID *) &SmbusPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gLpcPreMemConfigGuid, (VOID *) &LpcPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gHsioPciePreMemConfigGuid, (VOID *) &HsioPciePreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPcieRpPreMemConfigGuid, (VOID *) &PcieRpPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gHdAudioPreMemConfigGuid, (VOID *) &HdaPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gIshPreMemConfigGuid, (VOID *) &IshPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  DEBUG ((DEBUG_INFO, "------------------------ PCH Print PreMemPolicy Start ------------------------\n"));
  DEBUG ((DEBUG_INFO, " Revision= %x\n", SiPreMemPolicyPpi->TableHeader.Header.Revision));

  PchPrintGeneralPreMemConfig (PchGeneralPreMemConfig);
  PchPrintDciPreMemConfig (DciPreMemConfig);
  PchPrintWdtPreMemConfig (WdtPreMemConfig);
  PchPrintTraceHubPreMemConfig (PchTraceHubPreMemConfig);
  PchPrintSmbusPreMemConfig (SmbusPreMemConfig);
  PchPrintLpcPreMemConfig (LpcPreMemConfig);
  PchPrintHsioPciePreMemConfig (HsioPciePreMemConfig);
  for (SataCtrlIndex = 0; SataCtrlIndex < GetPchMaxSataControllerNum (); SataCtrlIndex++) {
    HsioSataPreMemConfig = GetPchHsioSataPreMemConfig (SiPreMemPolicyPpi, SataCtrlIndex);
    PchPrintHsioSataPreMemConfig (SataCtrlIndex, HsioSataPreMemConfig);
  }
  PchPrintPcieRpPreMemConfig (PcieRpPreMemConfig);
  PchPrintHdAudioPreMemConfig (HdaPreMemConfig);
  PchPrintIshPreMemConfig (IshPreMemConfig);

  DEBUG ((DEBUG_INFO, "------------------------ PCH Print PreMemPolicy End --------------------------\n"));
  DEBUG_CODE_END ();
}

