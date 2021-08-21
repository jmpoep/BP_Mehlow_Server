/** @file
  Initializes/updates PCH related HOBs in PEI

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 - 2018 Intel Corporation.

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

#include <Library/PchGbeLib.h>
#include "PchInitPei.h"

/**
  The function update pch info hob in the end of PchInit.
**/
VOID
BuildPchInfoHob (
  VOID
  )
{
  EFI_STATUS  Status;
  UINT32      StrapFuseCfg1;
  UINT32      StrapFuseCfg5;
  UINT32      StrapFuseCfg9;
  UINT32      StrapFuseCfg13;
  UINT32      StrapFuseCfg17;
  UINT32      StrapFuseCfg21;
  UINT32      MaxCtrl;
  UINT32      CtrlIndex;
  UINT32      RpIndex;
  UINT32      SpxPcd;
  PCH_INFO_HOB *PchInfoHob;
  BOOLEAN     CridSupport;
  UINT8       OrgRid;
  UINT8       NewRid;
  UINT32      LaneReversal1;
  UINT32      LaneReversal2;
  UINT32      LaneReversal3;
  UINT32      LaneReversal4;
  UINT32      LaneReversal5;
  UINT32      LaneReversal6;

  PchInfoHob = BuildGuidHob (&gPchInfoHobGuid, sizeof (PCH_INFO_HOB));
  ASSERT (PchInfoHob != 0);
  if (PchInfoHob == NULL) {
    return;
  }

  PchInfoHob->Revision = PCH_INFO_HOB_REVISION;

  Status = PchSbiRpPciRead32 (0, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg1);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (4, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg5);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (8, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg9);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (12, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg13);
  ASSERT_EFI_ERROR (Status);

  Status = PchSbiRpPciRead32 (0, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal1);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (4, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal2);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (8, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal3);
  ASSERT_EFI_ERROR (Status);
  Status = PchSbiRpPciRead32 (12, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal4);
  ASSERT_EFI_ERROR (Status);

  PchInfoHob->PciePortFuses = 0;
  PchInfoHob->PcieControllerCfg[0] = (UINT8) ((StrapFuseCfg1 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);
  PchInfoHob->PcieControllerCfg[1] = (UINT8) ((StrapFuseCfg5 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);
  PchInfoHob->PcieControllerCfg[2] = (UINT8) ((StrapFuseCfg9 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);
  PchInfoHob->PcieControllerCfg[3] = (UINT8) ((StrapFuseCfg13 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);

  PchInfoHob->PcieControllerLaneReversal[0] = (UINT8) ((LaneReversal1 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);
  PchInfoHob->PcieControllerLaneReversal[1] = (UINT8) ((LaneReversal2 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);
  PchInfoHob->PcieControllerLaneReversal[2] = (UINT8) ((LaneReversal3 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);
  PchInfoHob->PcieControllerLaneReversal[3] = (UINT8) ((LaneReversal4 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);

  if (IsPchH ()) {
    Status = PchSbiRpPciRead32 (16, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg17);
    ASSERT_EFI_ERROR (Status);
    Status = PchSbiRpPciRead32 (20, R_PCH_PCIE_CFG_STRPFUSECFG, &StrapFuseCfg21);
    ASSERT_EFI_ERROR (Status);

    Status = PchSbiRpPciRead32 (16, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal5);
    ASSERT_EFI_ERROR (Status);
    Status = PchSbiRpPciRead32 (20, R_PCH_PCIE_CFG_PCIEDBG, &LaneReversal6);
    ASSERT_EFI_ERROR (Status);

    PchInfoHob->PcieControllerCfg[4] = (UINT8) ((StrapFuseCfg17 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);
    PchInfoHob->PcieControllerCfg[5] = (UINT8) ((StrapFuseCfg21 & B_PCH_PCIE_CFG_STRPFUSECFG_RPC) >> N_PCH_PCIE_CFG_STRPFUSECFG_RPC);

    PchInfoHob->PcieControllerLaneReversal[4] = (UINT8) ((LaneReversal5 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);
    PchInfoHob->PcieControllerLaneReversal[5] = (UINT8) ((LaneReversal6 & B_PCH_PCIE_CFG_PCIEDBG_LR) != 0);
  }

  PchInfoHob->PciePortLaneEnabled = 0;
  for (RpIndex = 0; RpIndex < GetPchMaxPciePortNum (); RpIndex++) {
    if (PchFiaIsPcieRootPortLaneConnected (RpIndex, 0)) {
      PchInfoHob->PciePortLaneEnabled |= BIT0 << RpIndex;
    }
  }

  //
  // Get PCIE Port disable value for each RP
  //
  MaxCtrl = GetPchMaxPcieControllerNum ();

  for (CtrlIndex = 0; CtrlIndex < MaxCtrl; CtrlIndex++) {
    SpxPcd = PchPcrRead32 (PchGetPcieControllerSbiPid (CtrlIndex), R_SPX_PCR_PCD);
    for (RpIndex = 0; RpIndex < PCH_PCIE_CONTROLLER_PORTS; RpIndex++) {
      if ((SpxPcd & (B_SPX_PCR_PCD_P1D << RpIndex)) != 0) {
        PchInfoHob->PciePortFuses |= BIT0 << (RpIndex + CtrlIndex * PCH_PCIE_CONTROLLER_PORTS);
      }
    }
  }

  if (PchIsGbePresent ()) {
    PchInfoHob->GbePciePortNumber  = (UINT8)PchFiaGetGbePortNumber ();
  } else {
    PchInfoHob->GbePciePortNumber  = 0;
  }

  PchInfoHob->HpetBusNum    = V_P2SB_CFG_HBDF_BUS;
  PchInfoHob->HpetDevNum    = V_P2SB_CFG_HBDF_DEV;
  PchInfoHob->HpetFuncNum   = V_P2SB_CFG_HBDF_FUNC;
  PchInfoHob->IoApicBusNum  = V_P2SB_CFG_IBDF_BUS;
  PchInfoHob->IoApicDevNum  = V_P2SB_CFG_IBDF_DEV;
  PchInfoHob->IoApicFuncNum = V_P2SB_CFG_IBDF_FUNC;

  //
  // This must be done before PMC hidden.
  //
  CridSupport = FALSE;
  OrgRid      = 0;
  NewRid      = 0;
  Status = PmcDetectCrid0 (&CridSupport, &OrgRid, &NewRid);
  DEBUG ((DEBUG_INFO, "CridSupport = %x %x %x\n", CridSupport, OrgRid, NewRid));
  if (!EFI_ERROR (Status)) {
    PchInfoHob->CridSupport = CridSupport;
    PchInfoHob->CridOrgRid  = OrgRid;
    PchInfoHob->CridNewRid  = NewRid;
  }
}

/**
  The function update pch config hob in the end of PchInit.

  @param[in]      SiPolicy               The SI Policy PPI instance
**/
VOID
BuildPchConfigHob (
  IN     SI_POLICY_PPI          *SiPolicy
  )
{
  PCH_CONFIG_HOB              *PchConfigHob;
  SI_PREMEM_POLICY_PPI        *SiPreMemPolicy;
  PCH_SMBUS_PREMEM_CONFIG     *SmbusPreMemConfig;
  PCH_GENERAL_CONFIG          *PchGeneralConfig;
  PCH_INTERRUPT_CONFIG        *InterruptConfig;
  PCH_SERIAL_IO_CONFIG        *SerialIoConfig;
  PCH_PCIE_CONFIG             *PcieRpConfig;
  PCH_SCS_CONFIG              *ScsConfig;
  PCH_CNVI_CONFIG             *CnviConfig;
  PCH_HDAUDIO_CONFIG          *HdAudioConfig;
  PCH_DMI_CONFIG              *DmiConfig;
  PCH_LOCK_DOWN_CONFIG        *LockDownConfig;
  PCH_PM_CONFIG               *PmConfig;
  PCH_SATA_CONFIG             *SataConfig;
  PCH_FLASH_PROTECTION_CONFIG *FlashProtectionConfig;
  PCH_LAN_CONFIG              *LanConfig;
  PCH_TRACE_HUB_PREMEM_CONFIG *PchTraceHubPreMemConfig;
  EFI_STATUS                  Status;
  UINT32                      SataCtrlIndex;
  PCH_ESPI_CONFIG             *EspiConfig;

  PchConfigHob = BuildGuidHob (&gPchConfigHobGuid, sizeof (PCH_CONFIG_HOB));
  ASSERT (PchConfigHob != 0);
  if (PchConfigHob == NULL) {
    return;
  }

  //
  // Get Policy settings through the SiPreMemPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicy
             );
  if (Status != EFI_SUCCESS) {
    //
    // SI_PREMEM_POLICY_PPI must be installed at this point
    //
    ASSERT (FALSE);
    return;
  }
  Status = GetConfigBlock ((VOID *) SiPreMemPolicy, &gSmbusPreMemConfigGuid, (VOID *) &SmbusPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPreMemPolicy, &gPchTraceHubPreMemConfigGuid, (VOID *) &PchTraceHubPreMemConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gPchGeneralConfigGuid, (VOID *) &PchGeneralConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gInterruptConfigGuid, (VOID *) &InterruptConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gSerialIoConfigGuid, (VOID *) &SerialIoConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gScsConfigGuid, (VOID *) &ScsConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gCnviConfigGuid, (VOID *) &CnviConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gHdAudioConfigGuid, (VOID *) &HdAudioConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gDmiConfigGuid, (VOID *) &DmiConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gLockDownConfigGuid, (VOID *) &LockDownConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gPmConfigGuid, (VOID *) &PmConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gFlashProtectionConfigGuid, (VOID *) &FlashProtectionConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gLanConfigGuid, (VOID *) &LanConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gEspiConfigGuid, (VOID *) &EspiConfig);
  ASSERT_EFI_ERROR (Status);


  PchConfigHob->Smbus.NumRsvdSmbusAddresses = SmbusPreMemConfig->NumRsvdSmbusAddresses;
  CopyMem (
    PchConfigHob->Smbus.RsvdSmbusAddressTable,
    SmbusPreMemConfig->RsvdSmbusAddressTable,
    sizeof (PchConfigHob->Smbus.RsvdSmbusAddressTable)
    );

  PchConfigHob->General.Crid = PchGeneralConfig->Crid;
  PchConfigHob->Interrupt.NumOfDevIntConfig = InterruptConfig->NumOfDevIntConfig;
  PchConfigHob->Interrupt.GpioIrqRoute = InterruptConfig->GpioIrqRoute;
  CopyMem (
    PchConfigHob->Interrupt.PxRcConfig,
    InterruptConfig->PxRcConfig,
    sizeof (PchConfigHob->Interrupt.PxRcConfig)
    );
  CopyMem (
    PchConfigHob->Interrupt.DevIntConfig,
    InterruptConfig->DevIntConfig,
    sizeof (PchConfigHob->Interrupt.DevIntConfig)
    );

  CopyMem (
    PchConfigHob->SerialIo.DevMode,
    SerialIoConfig->DevMode,
    sizeof (PchConfigHob->SerialIo.DevMode)
    );
  PchConfigHob->SerialIo.DebugUartNumber = SerialIoConfig->DebugUartNumber;
  PchConfigHob->SerialIo.EnableDebugUartAfterPost = SerialIoConfig->EnableDebugUartAfterPost;
  CopyMem (
    PchConfigHob->PcieRp.RootPort,
    PcieRpConfig->RootPort,
    sizeof (PchConfigHob->PcieRp.RootPort)
    );
  PchConfigHob->Scs.ScsEmmcEnabled = ScsConfig->ScsEmmcEnabled;
  PchConfigHob->Scs.ScsEmmcHs400Enabled = ScsConfig->ScsEmmcHs400Enabled;
  PchConfigHob->Scs.ScsEmmcHs400TuningRequired = ScsConfig->ScsEmmcHs400TuningRequired;
  PchConfigHob->Scs.ScsEmmcHs400DllDataValid = ScsConfig->ScsEmmcHs400DllDataValid;
  PchConfigHob->Scs.ScsEmmcHs400DriverStrength = ScsConfig->ScsEmmcHs400DriverStrength;
  PchConfigHob->Scs.ScsSdPowerEnableActiveHigh = ScsConfig->SdCardPowerEnableActiveHigh;
  PchConfigHob->Scs.ScsSdCardEnabled = ScsConfig->ScsSdcardEnabled;
  PchConfigHob->Cnvi.Mode = CnviConfig->Mode;
  PchConfigHob->HdAudio.DspEnable = HdAudioConfig->DspEnable;
  PchConfigHob->HdAudio.AudioLinkSndw1 = HdAudioConfig->AudioLinkSndw1;
  PchConfigHob->HdAudio.AudioLinkSndw2 = HdAudioConfig->AudioLinkSndw2;
  PchConfigHob->HdAudio.AudioLinkSndw3 = HdAudioConfig->AudioLinkSndw3;
  PchConfigHob->HdAudio.AudioLinkSndw4 = HdAudioConfig->AudioLinkSndw4;
  PchConfigHob->HdAudio.CodecSxWakeCapability = HdAudioConfig->CodecSxWakeCapability;
  PchConfigHob->LockDown.GlobalSmi = LockDownConfig->GlobalSmi;
  PchConfigHob->LockDown.BiosInterface = LockDownConfig->BiosInterface;
  PchConfigHob->LockDown.RtcMemoryLock = LockDownConfig->RtcMemoryLock;
  PchConfigHob->LockDown.BiosLock = LockDownConfig->BiosLock;
  PchConfigHob->Pm.SlpS0VmRuntimeControl = PmConfig->SlpS0VmRuntimeControl;
  PchConfigHob->Pm.SlpS0Vm070VSupport    = PmConfig->SlpS0Vm070VSupport;
  PchConfigHob->Pm.SlpS0Vm075VSupport    = PmConfig->SlpS0Vm075VSupport;
  PchConfigHob->Pm.PsOnEnable            = PmConfig->PsOnEnable;
  PchConfigHob->Espi.BmeMasterSlaveEnabled = EspiConfig->BmeMasterSlaveEnabled;
  for (SataCtrlIndex = 0; SataCtrlIndex < GetPchMaxSataControllerNum (); SataCtrlIndex++) {
    SataConfig = GetPchSataConfig (SiPolicy, SataCtrlIndex);
    PchConfigHob->Sata[SataCtrlIndex].Enable = SataConfig->Enable;
    PchConfigHob->Sata[SataCtrlIndex].TestMode = SataConfig->TestMode;
    CopyMem (
      PchConfigHob->Sata[SataCtrlIndex].PortSettings,
      SataConfig->PortSettings,
      sizeof (PchConfigHob->Sata[SataCtrlIndex].PortSettings)
      );
    // RST PCIe Storage Remapping supported only by fist SATA controller
    if (SataCtrlIndex == SATA_1_CONTROLLER_INDEX) {
      CopyMem (
        PchConfigHob->Sata[SataCtrlIndex].RstPcieStorageRemap,
        SataConfig->RstPcieStorageRemap,
        sizeof (PchConfigHob->Sata[SataCtrlIndex].RstPcieStorageRemap)
        );
    }
  }
  CopyMem (
    PchConfigHob->ProtectRange,
    FlashProtectionConfig->ProtectRange,
    sizeof (PchConfigHob->ProtectRange)
    );

  PchConfigHob->PchTraceHub.PchTraceHubMode = PchTraceHubPreMemConfig->EnableMode;

}

/**
  Build PchDeviceTableHob

  @param[in] SiPolicy     The Silicon Policy PPI instance

  @retval None
**/
VOID
BuildPcieDeviceTableHob (
  IN  SI_POLICY_PPI  *SiPolicy
  )
{
  EFI_STATUS                            Status;
  UINTN                                 Count;
  VOID                                  *HobPtr;
  PCH_PCIE_DEVICE_OVERRIDE              *PcieDeviceTable;
  PCH_PCIE_CONFIG                       *PcieRpConfig;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gPcieRpConfigGuid, (VOID *) &PcieRpConfig);
  ASSERT_EFI_ERROR (Status);
  PcieDeviceTable = (PCH_PCIE_DEVICE_OVERRIDE *) PcieRpConfig->PcieDeviceOverrideTablePtr;
  if (PcieDeviceTable != NULL) {
    for (Count = 0; PcieDeviceTable[Count].DeviceId != 0; Count++) {
    }
    DEBUG ((DEBUG_INFO, "PCH Installing PcieDeviceTable HOB (%d entries)\n", Count));
    HobPtr = BuildGuidDataHob (
               &gPchDeviceTableHobGuid,
               PcieDeviceTable,
               Count * sizeof (PCH_PCIE_DEVICE_OVERRIDE)
               );
    ASSERT (HobPtr != 0);
  }
}

