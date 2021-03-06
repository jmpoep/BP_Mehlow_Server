/** @file
  PCH Pcie SMM Driver Entry

@copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2018 Intel Corporation.

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
#include "PchInitSmm.h"
#include <PcieRegs.h>
#include <Register/PchRegs.h>
#include <Register/PchRegsPcie.h>

GLOBAL_REMOVE_IF_UNREFERENCED PCH_PCIE_DEVICE_OVERRIDE    *mDevAspmOverride;
GLOBAL_REMOVE_IF_UNREFERENCED UINT32                      mNumOfDevAspmOverride;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mPchBusNumber;
//
// @note:
// These temp bus numbers cannot be used in runtime (hot-plug).
// These can be used only during boot.
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mTempRootPortBusNumMin;
GLOBAL_REMOVE_IF_UNREFERENCED UINT8                       mTempRootPortBusNumMax;

GLOBAL_REMOVE_IF_UNREFERENCED PCH_PCIE_ROOT_PORT_CONFIG   mPcieRootPortConfig[PCH_MAX_PCIE_ROOT_PORTS];

GLOBAL_REMOVE_IF_UNREFERENCED BOOLEAN                     mPciePmTrapExecuted = FALSE;

extern EFI_GUID gPchDeviceTableHobGuid;

/**
  Program Common Clock and ASPM of Downstream Devices

  @param[in] PortIndex                  Pcie Root Port Number
  @param[in] RpDevice                   Pcie Root Pci Device Number
  @param[in] RpFunction                 Pcie Root Pci Function Number
**/
STATIC
VOID
PchPcieSmi (
  IN  UINT8                             PortIndex,
  IN  UINT8                             RpDevice,
  IN  UINT8                             RpFunction
  )
{
  UINT8                 SecBus;
  UINT8                 SubBus;
  UINT64                RpBase;
  UINT64                EpBase;
  UINT8                 EpPcieCapPtr;
  UINT8                 EpMaxSpeed;
  BOOLEAN               DownstreamDevicePresent;
  UINT32                Timeout;

  RpBase   = PCI_SEGMENT_LIB_ADDRESS (
               DEFAULT_PCI_SEGMENT_NUMBER_PCH,
               mPchBusNumber,
               (UINT32) RpDevice,
               (UINT32) RpFunction,
               0
               );

  if (PciSegmentRead16 (RpBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    return;
  }
  //
  // Check presence detect state. Here the endpoint must be detected using PDS rather than
  // the usual LinkActive check, because PDS changes immediately and LA takes a few milliseconds to stabilize
  //
  DownstreamDevicePresent = !!(PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_SLSTS) & B_PCIE_SLSTS_PDS);

  if (DownstreamDevicePresent) {
    ///
    /// Make sure the link is active before trying to talk to device behind it
    /// Wait up to 100ms, according to PCIE spec chapter 6.7.3.3
    ///
    Timeout = 100 * 1000;
    while ((PciSegmentRead16 (RpBase + R_PCH_PCIE_CFG_LSTS) & B_PCIE_LSTS_LA) == 0 ) {
      MicroSecondDelay (10);
      Timeout-=10;
      if (Timeout == 0) {
        return;
      }
    }
    SecBus  = PciSegmentRead8 (RpBase + PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET);
    SubBus  = PciSegmentRead8 (RpBase + PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET);
    ASSERT (SecBus != 0 && SubBus != 0);
    RootportDownstreamConfiguration (
      DEFAULT_PCI_SEGMENT_NUMBER_PCH,
      DEFAULT_PCI_BUS_NUMBER_PCH,
      RpDevice,
      RpFunction,
      mTempRootPortBusNumMin,
      mTempRootPortBusNumMax
      );
    RootportDownstreamPmConfiguration (
      DEFAULT_PCI_SEGMENT_NUMBER_PCH,
      DEFAULT_PCI_BUS_NUMBER_PCH,
      RpDevice,
      RpFunction,
      mTempRootPortBusNumMin,
      mTempRootPortBusNumMax,
      &mPcieRootPortConfig[PortIndex],
      mNumOfDevAspmOverride,
      mDevAspmOverride
    );
    //
    // Perform Equalization
    //
    EpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, SecBus, 0, 0, 0);
    EpPcieCapPtr = PcieFindCapId (DEFAULT_PCI_SEGMENT_NUMBER_PCH, SecBus, 0, 0, EFI_PCI_CAPABILITY_ID_PCIEXP);
    EpMaxSpeed = PciSegmentRead8 (EpBase + EpPcieCapPtr + R_PCIE_LCAP_OFFSET) & B_PCIE_LCAP_MLS;
    if (EpMaxSpeed >= 3) {
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_EX_LCTL3, B_PCIE_EX_LCTL3_PE);
      PciSegmentOr32 (RpBase + R_PCH_PCIE_CFG_LCTL, B_PCIE_LCTL_RL);
    }
  }
}

/**
  PCIE Hotplug SMI call back function for each Root port

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.
**/
VOID
EFIAPI
PchPcieSmiRpHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  PCH_PCIE_SMI_RP_CONTEXT           *RpContext
  )
{
  PchPcieSmi (RpContext->RpIndex, RpContext->DevNum, RpContext->FuncNum);
}

/**
  PCIE Link Active State Change Hotplug SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.
**/
VOID
EFIAPI
PchPcieLinkActiveStateChange (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  PCH_PCIE_SMI_RP_CONTEXT           *RpContext
  )
{
  return;
}

/**
  PCIE Link Equalization Request SMI call back function for all Root ports

  @param[in] DispatchHandle             Handle of this dispatch function
  @param[in] RpContext                  Rootport context, which contains RootPort Index,
                                        and RootPort PCI BDF.
**/
VOID
EFIAPI
PchPcieLinkEqHandlerFunction (
  IN  EFI_HANDLE                        DispatchHandle,
  IN  PCH_PCIE_SMI_RP_CONTEXT           *RpContext
  )
{
  ///
  /// From PCI Express specification, the PCIe device can request for Link Equalization. When the
  /// Link Equalization is requested by the device, an SMI will be generated  by PCIe RP when
  /// enabled and the SMI subroutine would invoke the Software Preset/Coefficient Search
  /// software to re-equalize the link.
  ///

  return;

}

/**
  An IoTrap callback to config PCIE power management settings
**/
VOID
PchPciePmIoTrapSmiCallback (
  VOID
  )
{
  UINT32                                    PortIndex;
  UINT64                                    RpBase;
  UINT8                                     MaxPciePortNum;
  UINTN                                     RpDevice;
  UINTN                                     RpFunction;

  MaxPciePortNum                   = GetPchMaxPciePortNum ();

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetPchPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, (UINT32) RpDevice, (UINT32) RpFunction, 0);

    if (PciSegmentRead16 (RpBase) != 0xFFFF) {
      RootportDownstreamPmConfiguration (
        DEFAULT_PCI_SEGMENT_NUMBER_PCH,
        DEFAULT_PCI_BUS_NUMBER_PCH,
        (UINT8)RpDevice,
        (UINT8)RpFunction,
        mTempRootPortBusNumMin,
        mTempRootPortBusNumMax,
        &mPcieRootPortConfig[PortIndex],
        mNumOfDevAspmOverride,
        mDevAspmOverride
      );

    }
  }
}

/**
  An IoTrap callback to config PCIE power management settings

  @param[in] DispatchHandle  - The handle of this callback, obtained when registering
  @param[in] DispatchContext - Pointer to the EFI_SMM_IO_TRAP_DISPATCH_CALLBACK_CONTEXT

**/
VOID
EFIAPI
PchPcieIoTrapSmiCallback (
  IN  EFI_HANDLE                            DispatchHandle,
  IN  EFI_SMM_IO_TRAP_CONTEXT                *CallbackContext,
  IN OUT VOID                               *CommBuffer,
  IN OUT UINTN                              *CommBufferSize
  )
{
  if (CallbackContext->WriteData == PciePmTrap) {
    if (mPciePmTrapExecuted == FALSE) {
      PchPciePmIoTrapSmiCallback ();
      mPciePmTrapExecuted = TRUE;
    }
  } else {
    ASSERT_EFI_ERROR (EFI_INVALID_PARAMETER);
  }
}

/**
  This function clear the Io trap executed flag before enter S3

  @param[in] Handle    Handle of the callback
  @param[in] Context   The dispatch context

  @retval EFI_SUCCESS  PCH register saved
**/
EFI_STATUS
EFIAPI
PchPcieS3EntryCallBack (
  IN  EFI_HANDLE                        Handle,
  IN CONST VOID                    *Context OPTIONAL,
  IN OUT VOID                      *CommBuffer OPTIONAL,
  IN OUT UINTN                     *CommBufferSize OPTIONAL
  )
{
  mPciePmTrapExecuted = FALSE;
  return EFI_SUCCESS;
}
/**
  Register PCIE Hotplug SMI dispatch function to handle Hotplug enabling

  @param[in] ImageHandle          The image handle of this module
  @param[in] SystemTable          The EFI System Table

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
InitializePchPcieSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
{
  EFI_STATUS                            Status;
  UINT8                                 PortIndex;
  UINT8                                 Data8;
  UINT32                                Data32Or;
  UINT32                                Data32And;
  UINT64                                RpBase;
  UINTN                                 RpDevice;
  UINTN                                 RpFunction;
  EFI_HANDLE                            PcieHandle;
  PCH_PCIE_SMI_DISPATCH_PROTOCOL        *PchPcieSmiDispatchProtocol;
  EFI_HOB_GUID_TYPE*                    Hob;
  UINT32                                DevTableSize;
  EFI_HANDLE                            PchIoTrapHandle;
  EFI_SMM_IO_TRAP_REGISTER_CONTEXT      PchIoTrapContext;
  EFI_SMM_SX_REGISTER_CONTEXT           SxDispatchContext;
  PCH_PCIE_IOTRAP_PROTOCOL              *PchPcieIoTrapProtocol;
  EFI_HANDLE                            SxDispatchHandle;
  UINT8                                 MaxPciePortNum;

  DEBUG ((DEBUG_INFO, "InitializePchPcieSmm () Start\n"));

  MaxPciePortNum    = GetPchMaxPciePortNum ();

  //
  // Locate Pch Pcie Smi Dispatch Protocol
  //
  Status = gSmst->SmmLocateProtocol (&gPchPcieSmiDispatchProtocolGuid, NULL, (VOID**)&PchPcieSmiDispatchProtocol);
  ASSERT_EFI_ERROR (Status);

  mPchBusNumber = DEFAULT_PCI_BUS_NUMBER_PCH;
  mTempRootPortBusNumMin = PcdGet8 (PcdSiliconInitTempPciBusMin);
  mTempRootPortBusNumMax = PcdGet8 (PcdSiliconInitTempPciBusMax);

  ASSERT (sizeof mPcieRootPortConfig == sizeof mPchConfigHob->PcieRp.RootPort);
  CopyMem (
    mPcieRootPortConfig,
    &(mPchConfigHob->PcieRp.RootPort),
    sizeof (mPcieRootPortConfig)
    );

  mDevAspmOverride                  = NULL;
  mNumOfDevAspmOverride             = 0;

  Hob = GetFirstGuidHob (&gPchDeviceTableHobGuid);
  if (Hob != NULL) {
    DevTableSize = GET_GUID_HOB_DATA_SIZE (Hob);
    ASSERT ((DevTableSize % sizeof (PCH_PCIE_DEVICE_OVERRIDE)) == 0);
    mNumOfDevAspmOverride = DevTableSize / sizeof (PCH_PCIE_DEVICE_OVERRIDE);
    DEBUG ((DEBUG_INFO, "Found PcieDeviceTable HOB (%d entries)\n", mNumOfDevAspmOverride));
    Status = gSmst->SmmAllocatePool (
                      EfiRuntimeServicesData,
                      DevTableSize,
                      (VOID **) &mDevAspmOverride
                      );
    CopyMem (mDevAspmOverride, GET_GUID_HOB_DATA (Hob), DevTableSize);
  }

  //
  // Throught all PCIE root port function and register the SMI Handler for enabled ports.
  //
  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    GetPchPcieRpDevFun (PortIndex, &RpDevice, &RpFunction);
    RpBase = PCI_SEGMENT_LIB_ADDRESS (DEFAULT_PCI_SEGMENT_NUMBER_PCH, DEFAULT_PCI_BUS_NUMBER_PCH, (UINT32) RpDevice, (UINT32) RpFunction, 0);
    //
    // Skip the root port function which is not enabled
    //
    if (PciSegmentRead32 (RpBase) == 0xFFFFFFFF) {
      continue;
    }

    //
    // Register SMI Handlers for Hot Plug and Link Active State Change
    //
    Data8 = PciSegmentRead8 (RpBase + R_PCH_PCIE_CFG_SLCAP);
    if (Data8 & B_PCIE_SLCAP_HPC) {
      PcieHandle = NULL;
      Status = PchPcieSmiDispatchProtocol->HotPlugRegister (
                                             PchPcieSmiDispatchProtocol,
                                             PchPcieSmiRpHandlerFunction,
                                             PortIndex,
                                             &PcieHandle
                                             );
      ASSERT_EFI_ERROR (Status);

      Status = PchPcieSmiDispatchProtocol->LinkActiveRegister (
                                             PchPcieSmiDispatchProtocol,
                                             PchPcieLinkActiveStateChange,
                                             PortIndex,
                                             &PcieHandle
                                             );
      ASSERT_EFI_ERROR (Status);

      Data32Or  = B_PCH_PCIE_CFG_MPC_HPME;
      Data32And = (UINT32) ~B_PCH_PCIE_CFG_MPC_HPME;
      S3BootScriptSaveMemReadWrite (
        S3BootScriptWidthUint32,
        PcdGet64 (PcdPciExpressBaseAddress) + RpBase + R_PCH_PCIE_CFG_MPC,
        &Data32Or,  /// Data to be ORed
        &Data32And  /// Data to be ANDed
        );
    }

    //
    // Register SMI Handler for Link Equalization Request from Gen 3 Devices.
    //
    Data8 = PciSegmentRead8 (RpBase + R_PCH_PCIE_CFG_LCAP);
    if ((Data8 & B_PCIE_LCAP_MLS) == V_PCIE_LCAP_MLS_GEN3) {
      Status = PchPcieSmiDispatchProtocol->LinkEqRegister (
                                             PchPcieSmiDispatchProtocol,
                                             PchPcieLinkEqHandlerFunction,
                                             PortIndex,
                                             &PcieHandle
                                             );
      ASSERT_EFI_ERROR (Status);
    }
  }

  ASSERT_EFI_ERROR (Status);

  PchIoTrapContext.Type     = WriteTrap;
  PchIoTrapContext.Length   = 4;
  PchIoTrapContext.Address  = 0;
  Status = mPchIoTrap->Register (
                         mPchIoTrap,
                         (EFI_SMM_HANDLER_ENTRY_POINT2) PchPcieIoTrapSmiCallback,
                         &PchIoTrapContext,
                         &PchIoTrapHandle
                         );
  ASSERT_EFI_ERROR (Status);

  //
  // Install the PCH Pcie IoTrap protocol
  //
  (gBS->AllocatePool) (EfiBootServicesData, sizeof (PCH_PCIE_IOTRAP_PROTOCOL), (VOID **)&PchPcieIoTrapProtocol);
  PchPcieIoTrapProtocol->PcieTrapAddress = PchIoTrapContext.Address;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gPchPcieIoTrapProtocolGuid,
                  PchPcieIoTrapProtocol,
                  NULL
                  );

  //
  // Register the callback for S3 entry
  //
  SxDispatchContext.Type  = SxS3;
  SxDispatchContext.Phase = SxEntry;
  Status = mSxDispatch->Register (
                          mSxDispatch,
                          PchPcieS3EntryCallBack,
                          &SxDispatchContext,
                          &SxDispatchHandle
                          );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "InitializePchPcieSmm, IoTrap @ %x () End\n", PchIoTrapContext.Address));

  return EFI_SUCCESS;
}
