/** @file
  This file contains functions that initialize PCI Express clock sources in PCH.

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

#include <Library/SiPolicyLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Private/Library/PeiPchPcieClocksLib.h>
#include <Library/PchInfoLib.h>
#include <Library/PchPcrLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Private/Library/PchFiaLib.h>
#include <Library/TimerLib.h>
#include <Library/PeiMeLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Private/Ppi/TetonGlacierPpi.h>

#define  CYCLE_ROUTER_0    0x0
#define  CYCLE_ROUTER_1    0x1
#define  CYCLE_ROUTER_2    0x2
#define  STATIC_CONFIG     0x1
#define  DYNAMIC_CONFIG    0x2

PCH_PCIE_CLOCK       *mClockPolicy = NULL;

/**
  Retrieves and returns PCIe Clocks Policy.
  After 1st call the result is cached to speed up following requests.

  @retval  pointer to PCIe Clocks Policy
**/
STATIC
PCH_PCIE_CLOCK*
GetClocksPolicy (
  VOID
  )
{
  EFI_STATUS               Status;
  SI_POLICY_PPI            *SiPolicyPpi;
  PCH_PCIE_CONFIG          *PchPciePeiConfig;

  if (mClockPolicy == NULL) {
    //
    // Get Policy settings through the SiPolicy PPI
    //
    Status = PeiServicesLocatePpi (
               &gSiPolicyPpiGuid,
               0,
               NULL,
               (VOID **) &SiPolicyPpi
               );
    ASSERT_EFI_ERROR (Status);
    Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gPcieRpConfigGuid, (VOID *) &PchPciePeiConfig);
    ASSERT_EFI_ERROR (Status);
    mClockPolicy = &(PchPciePeiConfig->PcieClock[0]);
  }
  return mClockPolicy;
}

/**
  Retrieves and returns Teton Glacier Policy.

  @retval  pointer to Teton Glacier Policy
**/
STATIC
TETON_GLACIER_CONFIG*
GetTetonGlacierPolicy (
  VOID
  )
{
  EFI_STATUS               Status;
  SI_POLICY_PPI            *SiPolicyPpi;
  TETON_GLACIER_CONFIG     *TetonGlacierConfig;

  //
  // Get Policy settings through the SiPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicyPpi
             );
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock ((VOID *) SiPolicyPpi, &gTetonGlacierConfigGuid, (VOID *) &TetonGlacierConfig);
  ASSERT_EFI_ERROR (Status);
  return TetonGlacierConfig;
}

/**
  Overrides Clkreq when Teton Glacier is connected to the system

  @param[in,out] *ClkReqMap           Pointer to Clock request map
  @param[in]      CycleRouter         Cycle Router to which Teton Glacier is connected
**/
STATIC
VOID
TetonGlacierClkReqOverride (
  IN UINT8  *ClkReqMap,
  IN UINT8  CycleRouter
  )
{
  if (IsPchH ()) {
    switch (CycleRouter) {
      case (CYCLE_ROUTER_0):
          ClkReqMap[PchClockUsagePchPcie10] = ClkReqMap[PchClockUsagePchPcie8];
          break;
        case (CYCLE_ROUTER_1):
          ClkReqMap[PchClockUsagePchPcie22] = ClkReqMap[PchClockUsagePchPcie20];
          break;
        case (CYCLE_ROUTER_2):
          ClkReqMap[PchClockUsagePchPcie18] = ClkReqMap[PchClockUsagePchPcie16];
    }
  } else {
    switch (CycleRouter) {
      case (CYCLE_ROUTER_0):
        ClkReqMap[PchClockUsagePchPcie6] = ClkReqMap[PchClockUsagePchPcie4];
        break;
      case (CYCLE_ROUTER_2):
        ClkReqMap[PchClockUsagePchPcie14] = ClkReqMap[PchClockUsagePchPcie12];
        break;
      case (CYCLE_ROUTER_1):
        ClkReqMap[PchClockUsagePchPcie10] = ClkReqMap[PchClockUsagePchPcie8];
    }
  }
}
/**
  Assign CLKREQ# to PCH PCIe ports
**/
VOID
PchConfigurePchPciePortsClkReqMapping (
  VOID
  )
{
  UINT8                               MaxPciePortNum;
  UINT8                               PortIndex;
  UINT8                               ClkReqMap[PCH_MAX_PCIE_ROOT_PORTS];
  UINT8                               ClkReqNum;
  TETON_GLACIER_CONFIG                *TetonGlacierConfig;
  UINT8                               TetonGlacierCycleRouter;
  UINT8                               ControllerConfig[PCH_MAX_PCIE_CONTROLLERS];
  UINT16                              NumOfControllers;
  BOOLEAN                             TgPresent;
  EFI_PEI_PPI_DESCRIPTOR              *TetonGlacierConfigPpiDesc;;
  TETON_GLACIER_CFG_PPI               *TetonGlacierCfg;
  EFI_STATUS                          Status;

  MaxPciePortNum = GetPchMaxPciePortNum ();
  ASSERT (MaxPciePortNum <= PCH_MAX_PCIE_ROOT_PORTS);

  for (PortIndex = 0; PortIndex < MaxPciePortNum; PortIndex++) {
    ClkReqNum = FindClkReqForUsage (PchClockUsagePchPcie0 + PortIndex);
    if (ClkReqNum == PCH_PCIE_NO_SUCH_CLOCK) {
      ClkReqMap[PortIndex] = PCH_FIA_NO_CLKREQ;
    } else {
      ClkReqMap[PortIndex] = ClkReqNum;
    }
  }
  //
  // Check Teton Glacier Support
  //
  TetonGlacierConfig = GetTetonGlacierPolicy ();
  TetonGlacierCycleRouter = (UINT8) TetonGlacierConfig->TetonGlacierCR;
  NumOfControllers = GetPchMaxPcieControllerNum ();
  ZeroMem (ControllerConfig, sizeof (ControllerConfig));

  if (TetonGlacierConfig->TetonGlacierMode == DYNAMIC_CONFIG) {
    Status = PeiHeciGetSoftStrpConfigMsg (NumOfControllers, ControllerConfig);
    if (Status == EFI_SUCCESS) {
      if (IsPchH ()) {
        TgPresent = ((ControllerConfig[2] == PcieOverride2x2) || (ControllerConfig[4] == PcieOverride2x2) || (ControllerConfig[5] == PcieOverride2x2));
      } else {
        TgPresent = ((ControllerConfig[1] == PcieOverride2x2) || (ControllerConfig[2] == PcieOverride2x2) || (ControllerConfig[3] == PcieOverride2x2));
      }
      Status = PeiServicesLocatePpi (
                 &gTetonGlacierCfgPpiGuid,
                 0,
                 NULL,
                 (VOID **) &TetonGlacierCfg
                 );
      if (Status == EFI_NOT_FOUND) {
        TetonGlacierCfg = (TETON_GLACIER_CFG_PPI *) AllocateZeroPool (sizeof (TETON_GLACIER_CFG_PPI));
        if (TetonGlacierCfg == NULL) {
          return;
        }
        CopyMem (TetonGlacierCfg, ControllerConfig, PCH_MAX_PCIE_CONTROLLERS * sizeof (UINT8));
        TetonGlacierConfigPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
        if (TetonGlacierConfigPpiDesc == NULL) {
          ASSERT(FALSE);
          return;
        }
        TetonGlacierConfigPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
        TetonGlacierConfigPpiDesc->Guid = &gTetonGlacierCfgPpiGuid;
        TetonGlacierConfigPpiDesc->Ppi = TetonGlacierCfg;
        DEBUG ((DEBUG_INFO, "Installing Teton Glacier configuration PPI\n"));
        Status = PeiServicesInstallPpi (TetonGlacierConfigPpiDesc);
        ASSERT_EFI_ERROR (Status);
      }
      if (TgPresent) {
        if (IsPchH ()) {
          if (ControllerConfig[2] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_0;
          } else if (ControllerConfig[4] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_2;
          } else if (ControllerConfig[5] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_1;
          }
          DEBUG ((DEBUG_INFO, "Performing TetonGlacierClkReqOverride for CR %x \n", (TetonGlacierCycleRouter + 1) ));
          TetonGlacierClkReqOverride (ClkReqMap, TetonGlacierCycleRouter);
        } else {
          if (ControllerConfig[1] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_0;
          } else if (ControllerConfig[2] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_1;
          } else if (ControllerConfig[3] == PcieOverride2x2) {
            TetonGlacierCycleRouter = CYCLE_ROUTER_2;
          }
          DEBUG ((DEBUG_INFO, "Performing TetonGlacierClkReqOverride for CR %x \n", (TetonGlacierCycleRouter + 1)));
          TetonGlacierClkReqOverride (ClkReqMap, TetonGlacierCycleRouter);
        }
      }
    }
  }
  FreePool (ControllerConfig);
  if (TetonGlacierConfig->TetonGlacierMode == STATIC_CONFIG) {
    //
    //Override clkreq mapping for the correct root port based on cycle router selected when teton glacier is connected
    //
    TetonGlacierClkReqOverride (ClkReqMap,TetonGlacierCycleRouter);
  }
  PchFiaAssignPchPciePortsClkReq (ClkReqMap, MaxPciePortNum);
}

/**
  Assign CLKREQ# to GbE
**/
VOID
PchConfigureGbeClkReqMapping (
  VOID
  )
{
  UINT8  ClkReqNum;

  ClkReqNum = FindClkReqForUsage (PchClockUsageLan);
  if (ClkReqNum == PCH_PCIE_NO_SUCH_CLOCK) {
    PchFiaAssignGbeClkReq (PCH_FIA_NO_CLKREQ);
  } else {
    PchFiaAssignGbeClkReq (ClkReqNum);
  }
}

/**
  Assign CLKREQ# to controllers
  Assignment is based on the platform policy.
**/
VOID
PchConfigureClkreqMapping (
  VOID
  )
{
  PchConfigurePchPciePortsClkReqMapping ();
  PchConfigureGbeClkReqMapping ();
}

/**
  Checks current state of ClkReq GPIO pad that was assigned to given port.
  This function returns error when port has no Clock or ClkReq assigned
  or if GPIO pad is not owned by BIOS. If there's no error, pad's current input value is returned.

  @param[in] ClockUsage     PCIe port for which ClkReq must be checked
  @retval EFI_UNSUPPORTED   no ClkReq assigned or input value can't be read
  @retval EFI_DEVICE_ERROR  wrong pad state, high
  @retval EFI_SUCCESS       ClkReq is in correct state, low
**/
EFI_STATUS
CheckClkReq (
  PCH_PCIE_CLOCK_USAGE ClockUsage
  )
{
  UINT32      ClkReqNumber;
  GPIO_PAD    ClkReqPad;
  GPIO_CONFIG PadConfig;
  EFI_STATUS  Status;
  UINT32      GpioValue;

  ClkReqNumber = FindClkReqForUsage (ClockUsage);
  if (ClkReqNumber == PCH_PCIE_NO_SUCH_CLOCK || ClkReqNumber >= GetPchMaxPcieClockNum ()) {
    return EFI_UNSUPPORTED;
  }
  ClkReqPad = GpioGetClkreqPad (ClkReqNumber);

  ZeroMem (&PadConfig, sizeof (PadConfig));
  PadConfig.PadMode      = GpioPadModeGpio;
  PadConfig.Direction    = GpioDirIn;
  GpioSetPadConfig (ClkReqPad, &PadConfig);
  //
  // Wait some time to make sure GPIO pad was reconfigured and its input value propagated to internal logic
  //
  MicroSecondDelay (5);

  Status = GpioGetInputValue (ClkReqPad, &GpioValue);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  if (GpioValue != 0) {
    return EFI_DEVICE_ERROR;
  }
  return EFI_SUCCESS;
}

/**
  Overrides ClkUsage when Teton Glacier is connected to the system

  @param[in,out] *ClkUsagePtr        Pointer to the number of PCIe port to check ClkReq for
  @param[in]      CycleRouter        CycleRouter to which Teton Glacier is connected
**/
STATIC
VOID
TetonGlacierClkUsageOverride (
  IN PCH_PCIE_CLOCK_USAGE  *ClkUsagePtr,
  IN UINT8                 CycleRouter
  )
{
  PCH_PCIE_CLOCK_USAGE ClkUsageValue;
  ClkUsageValue = *ClkUsagePtr;
  if (IsPchH ()) {
    if ((CycleRouter == CYCLE_ROUTER_0) && (ClkUsageValue >= PchClockUsagePchPcie8 && ClkUsageValue <= PchClockUsagePchPcie11)) {
      *ClkUsagePtr = PchClockUsagePchPcie8;
    } else if ((CycleRouter == CYCLE_ROUTER_2) && (ClkUsageValue >= PchClockUsagePchPcie16 && ClkUsageValue <= PchClockUsagePchPcie19)) {
      *ClkUsagePtr = PchClockUsagePchPcie16;
    } else if ((CycleRouter == CYCLE_ROUTER_1) && (ClkUsageValue >= PchClockUsagePchPcie20 && ClkUsageValue <= PchClockUsagePchPcie23)) {
      *ClkUsagePtr = PchClockUsagePchPcie20;
    }
  }

  if (IsPchLp ()) {
    if ((CycleRouter == CYCLE_ROUTER_0) && (ClkUsageValue >= PchClockUsagePchPcie4 && ClkUsageValue <= PchClockUsagePchPcie7)) {
      *ClkUsagePtr = PchClockUsagePchPcie4;
    } else if ((CycleRouter == CYCLE_ROUTER_1) && (ClkUsageValue >= PchClockUsagePchPcie8 && ClkUsageValue <= PchClockUsagePchPcie11)) {
      *ClkUsagePtr = PchClockUsagePchPcie8;
    } else if ((CycleRouter == CYCLE_ROUTER_2) && (ClkUsageValue >= PchClockUsagePchPcie12 && ClkUsageValue <= PchClockUsagePchPcie15 )) {
      *ClkUsagePtr = PchClockUsagePchPcie12;
    }
  }
}
/**
  Checks if given PCIe port is assigned with Clock Request signal

  @param[in]  ClockUsage  type and number of PCIe port to check ClkReq for
  @retval     TRUE        there's a ClkReq pad corresponding to given Port
  @retval     FALSE       there's no ClkReq pad corresponding to given Port
**/
BOOLEAN
IsClkReqAssigned (
  PCH_PCIE_CLOCK_USAGE ClockUsage
  )
{
  UINT32                              ClkNumber;
  UINT32                              ClkReqNumber;
  UINT8                               TetonGlacierCycleRouter;
  TETON_GLACIER_CONFIG                *TetonGlacierConfig;
  TETON_GLACIER_CFG_PPI               *TetonGlacierCfg;
  EFI_STATUS                          Status;
  BOOLEAN                             TgPresent;

  //
  // Check Teton Glacier Support
  //
  TetonGlacierConfig = GetTetonGlacierPolicy ();
  TetonGlacierCycleRouter = (UINT8) TetonGlacierConfig->TetonGlacierCR;

  if (TetonGlacierConfig->TetonGlacierMode == DYNAMIC_CONFIG) {
    Status = PeiServicesLocatePpi (
               &gTetonGlacierCfgPpiGuid,
               0,
               NULL,
               (VOID **)&TetonGlacierCfg
               );
    if (Status == EFI_NOT_FOUND) {
      DEBUG ((DEBUG_ERROR, "Teton Glacier Configuration Data Ppi Not found.\n"));
    }
    if (IsPchH ()) {
      TgPresent = ((TetonGlacierCfg->ControllerCfg[2] == PcieOverride2x2) || (TetonGlacierCfg->ControllerCfg[4] == PcieOverride2x2) || (TetonGlacierCfg->ControllerCfg[5] == PcieOverride2x2));
    } else {
      TgPresent = ((TetonGlacierCfg->ControllerCfg[1] == PcieOverride2x2) || (TetonGlacierCfg->ControllerCfg[2] == PcieOverride2x2) || (TetonGlacierCfg->ControllerCfg[3] == PcieOverride2x2));
    }
    if (TgPresent) {
      if (IsPchH ()) {
        if (TetonGlacierCfg->ControllerCfg[2] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_0;
        } else if (TetonGlacierCfg->ControllerCfg[4] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_2;
        } else if (TetonGlacierCfg->ControllerCfg[5] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_1;
        }
      } else {
        if (TetonGlacierCfg->ControllerCfg[1] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_0;
        } else if (TetonGlacierCfg->ControllerCfg[2] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_1;
        } else if (TetonGlacierCfg->ControllerCfg[3] == PcieOverride2x2) {
          TetonGlacierCycleRouter = CYCLE_ROUTER_2;
        }
      }
      TetonGlacierClkUsageOverride (&ClockUsage, TetonGlacierCycleRouter);
    }
  }

  if (TetonGlacierConfig->TetonGlacierMode == STATIC_CONFIG) {
    //
    //Override ClkUsage for the correct pcie root port when Teton Glacier is connected to the system based on the cycle router to which it is connected
    //
    TetonGlacierClkUsageOverride (&ClockUsage, TetonGlacierCycleRouter);
  }
  ClkNumber = ClockUsageToClockNumber (GetClocksPolicy (), ClockUsage);
  if (ClkNumber == PCH_PCIE_NO_SUCH_CLOCK) {
    return FALSE;
  }
  ClkReqNumber = ClockNumberToClkReqNumber (GetClocksPolicy (), ClkNumber);
  if (ClkReqNumber == PCH_PCIE_NO_SUCH_CLOCK) {
    return FALSE;
  }
  return TRUE;
}

/**
  Enables CLK REQ for given PCIe port
  If given port has a clock source assigned and that clock source has a clock request signal,
  then the corresponding GPIO pad is configured into ClkReq mode

  @param[in]  ClockUsage      type and number of PCIe port
  @retval     EFI_SUCCESS     Clock Request was successfully enabled
  @retval     EFI_UNSUPPORTED there's no ClkReq pad corresponding to given Port
**/
EFI_STATUS
EnableClkReq (
  PCH_PCIE_CLOCK_USAGE ClockUsage
  )
{
  UINT32 ClkNumber;
  UINT32 ClkReqNumber;

  ClkNumber = ClockUsageToClockNumber (GetClocksPolicy (), ClockUsage);
  if (ClkNumber == PCH_PCIE_NO_SUCH_CLOCK) {
    return EFI_UNSUPPORTED;
  }
  ClkReqNumber = ClockNumberToClkReqNumber (GetClocksPolicy (), ClkNumber);
  if (ClkReqNumber == PCH_PCIE_NO_SUCH_CLOCK || ClkReqNumber >= GetPchMaxPcieClockNum ()) {
    return EFI_UNSUPPORTED;
  }
  GpioEnableClkreq (ClkReqNumber);
  return EFI_SUCCESS;
}

/**
  Disables one PCIe clock.

  @param[in] ClockUsage    type and number of PCIe port for which Clock should be disabled
**/
VOID
DisableClock (
  PCH_PCIE_CLOCK_USAGE ClockUsage
  )
{
  UINT32 ClkNumber;

  ClkNumber = ClockUsageToClockNumber (GetClocksPolicy (), ClockUsage);
  if (ClkNumber == PCH_PCIE_NO_SUCH_CLOCK) {
    return;
  }
  PmcSetPcieClockEnableMask ((BIT0 << ClkNumber), 0);
}

/**
  Disables all PCIe clocks which are not used according to Policy
**/
VOID
DisableUnusedPcieClocks (
  VOID
  )
{
  UINT32          Index;
  PCH_PCIE_CLOCK* ClocksPolicy;

  ClocksPolicy = GetClocksPolicy ();
  for (Index = 0; Index < GetPchMaxPcieClockNum (); Index++) {
    if (ClocksPolicy[Index].Usage == PchClockUsageNotUsed) {
      PmcSetPcieClockEnableMask ( (BIT0 << Index), 0);
    }
  }
}

/**
  This function scans PCH PCIE configuration and finds CLK_REQ signal that corresponds to given usage.

  @param[in]  ClockUsage   purpose of this CLK_SRC signal, either root port index or LAN
  @retval     number of CLK_REQ signal if any is assigned, PCH_PCIE_NO_SUCH_CLOCK otherwise
**/
UINT8
FindClkReqForUsage (
  PCH_PCIE_CLOCK_USAGE  ClockUsage
  )
{
  UINT32 Index;
  PCH_PCIE_CLOCK* ClocksPolicy;

  ClocksPolicy = GetClocksPolicy ();
  for (Index = 0; Index < GetPchMaxPcieClockNum (); Index++) {
    if (ClocksPolicy[Index].Usage == ClockUsage) {
      if (ClocksPolicy[Index].ClkReq < GetPchMaxPcieClockNum ()) {
        return ClocksPolicy[Index].ClkReq;
      } else {
        return PCH_PCIE_NO_SUCH_CLOCK;
      }
    }
  }
  return PCH_PCIE_NO_SUCH_CLOCK;
}

/**
  This function scans PCH PCIE configuration and finds clock number that corresponds to given usage.
  If there's no clock assigned, it will return PCH_PCIE_NO_SUCH_CLOCK

  @param[in]  ClocksPolicy  PCIe clocks configuration policy structure
  @param[in]  ClockUsage   user of clock, either PCH PCIe port, CPU PCIe port or LAN
  @retval     number of clock if any is assigned, PCH_PCIE_NO_SUCH_CLOCK otherwise
**/
UINT8
ClockUsageToClockNumber (
  CONST PCH_PCIE_CLOCK* ClocksPolicy,
  PCH_PCIE_CLOCK_USAGE  ClockUsage
  )
{
  UINT8 Index;

  for (Index = 0; Index < GetPchMaxPcieClockNum (); Index++) {
    if (ClocksPolicy[Index].Usage == ClockUsage) {
      return Index;
    }
  }
  return 0xFF;
}

/**
  This function scans PCH PCIE configuration and finds Clock Request signal that corresponds to given Clock Source.
  If there's no CLK_REQ, it will return 0xFF

  @param[in]  ClocksPolicy  PCIe clocks configuration policy structure
  @param[in]  ClkSrcNumber  purpose of this CLK_SRC signal, either PCH PCIe port, CPU PCIe port or LAN
  @retval     number of CLK_REQ signal if any is assigned, PCH_PCIE_NO_SUCH_CLOCK otherwise
**/
UINT8
ClockNumberToClkReqNumber (
  CONST PCH_PCIE_CLOCK* ClocksPolicy,
  UINT32                ClkSrcNumber
  )
{
  if (ClocksPolicy[ClkSrcNumber].ClkReq < GetPchMaxPcieClockNum ()) {
    return ClocksPolicy[ClkSrcNumber].ClkReq;
  } else {
    return PCH_PCIE_NO_SUCH_CLOCK;
  }
}

