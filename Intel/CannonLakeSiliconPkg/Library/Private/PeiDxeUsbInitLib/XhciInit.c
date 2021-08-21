/** @file
  Library for USB controller initialization for both CPU and PCH.

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
#include <Private/Library/UsbInitLib.h>

// General includes
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/BaseMemoryLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Register/RegsUsb.h>
#include <Library/PcdLib.h>
#include <Library/UsbLib.h>

// PCH specific includes
#include <Library/PchPcrLib.h>
#include <Private/Library/PchPsfPrivateLib.h>
#include <Library/PchInfoLib.h>
#include <Library/PchSbiAccessLib.h>

// SA specific includes
#include <Library/CpuPlatformLib.h>

#include <Library/ResetSystemLib.h>
#include <Library/HobLib.h>
#include <Private/Library/PeiHsioLib.h>


#define PORT_RESET_TIMEOUT      12000  //< 12000 * 10 us = 120 ms timeout for xHCI port reset, according to USB3 spec

// xHC Latency Tolerance Parameters used during initialization process
#define V_XHCI_LTR_HIT_1    0x03B60366
#define V_XHCI_LTR_HIT_2    0x00050002
#define V_XHCI_LTR_MIT_1    0x012C010E
#define V_XHCI_LTR_MIT_2    0x00050002
#define V_XHCI_LTR_LIT_1    0x003C0036
#define V_XHCI_LTR_LIT_2    0x00050002

// xHCI controller DID for USB IP Version detection
#define V_XHCI_DID_PCH_LP_V16_0     0x9DED
#define V_XHCI_DID_PCH_H_V16_0      0xA36D
#define V_XHCI_DID_CPU_LP_V17_0     0x8A13
#define V_XHCI_DID_PCH_LP_V17_1     0x34ED
#define V_XHCI_DID_PCH_N_V17_1      0x38ED
#define V_XHCI_DID_PCH_H_V17_1      0x3DED
#define V_XHCI_DID_PCH_B_V18_0      0x98ED
#define V_XHCI_DID_PCH_LP_V18_0     0x41AD

// USB IP Version enumeration
typedef enum {
  V16_0 = 0,
  V17_0,
  V17_1_A,
  V17_1_B,
  V18_0
} USB_IP_VERSION;

// xHCI controller information structure
typedef struct {
  BOOLEAN          OnSouth;
  USB_IP_VERSION   IpVersion;       // Version of USB IP (value according to USB_IP_VERSION enumeration
  BOOLEAN          IsCpuStepA0;     // Flag for applying programming in specific steps for CPU
  UINT16           DeviceId;        // Host Controller device ID
  UINT32           UsbLtrHigh;      // High Idle Time Control LTR parameters
  UINT32           UsbLtrMid;       // Medium Idle Time Control LTR parameters
  UINT32           UsbLtrLow;       // Low Idle Time Control LTR parameters
} USB_CONTROLLER_INFO;

/*
  Helper function to return supported number of USB3 ports reported by Host Controller

  @param[in]  XhciMmioBase      Memory BAR of the xHCI controller
*/
STATIC
UINT32
GetUsb3PortCount (
  IN UINTN     XhciMmioBase
  )
{
  UINT32  PortCount;

  PortCount = ((MmioRead32 (XhciMmioBase + R_XHCI_MEM_XECP_SUPP_USB3_2) & B_XHCI_MEM_XECP_SUPP_USBX_2_CPC) >> N_XHCI_MEM_XECP_SUPP_USBX_2_CPC);
  ASSERT (PortCount <= MAX_USB3_PORTS);

  return (MIN (PortCount, MAX_USB3_PORTS));
}

/*
  Helper function to return supported number of USB2 ports reported by Host Controller

  @param[in]  XhciMmioBase      Memory BAR of the xHCI controller
*/
STATIC
UINT32
GetUsb2PortCount (
  IN UINTN     XhciMmioBase
  )
{
  UINT32  PortCount;

  PortCount = ((MmioRead32 (XhciMmioBase + R_XHCI_MEM_XECP_SUPP_USB2_2) & B_XHCI_MEM_XECP_SUPP_USBX_2_CPC) >> N_XHCI_MEM_XECP_SUPP_USBX_2_CPC);
  ASSERT (PortCount <= MAX_USB2_PORTS);

  return (MIN (PortCount, MAX_USB2_PORTS));
}

/**
  Returns USB3 PortSC register address base offset in xHCI MMIO space which
  can be later used for iteration through all USB3 PortSC registers

  @param[in]  XhciMmioBase      Memory BAR of the xHCI controller

  @retval     UINT32            Calculated PortSC register offset
**/
STATIC
UINT32
GetUsb3PortScBase (
  IN UINT32     XhciMmioBase
  )
{
  UINT32   Usb2PortCount;

  Usb2PortCount = GetUsb2PortCount (XhciMmioBase);
  return (R_XHCI_MEM_PORTSC_START_OFFSET + (Usb2PortCount * S_XHCI_MEM_PORTSC_PORT_SPACING));
}

/**
  Helper function for gathering of xHCI controller features and infromation

  @param[in]  XhciPciBase         xHCI PCI config space address
  @param[out] HcInfo              xHCI controller information structure
**/
STATIC
VOID
GetXhciControllerInfo (
  IN  UINT64                XhciPciBase,
  OUT USB_CONTROLLER_INFO   *HcInfo
  )
{
  UINT16                ControllerDid;

  ZeroMem (HcInfo, sizeof (USB_CONTROLLER_INFO));

  // Check device DID from given PCI config space
  ControllerDid     = PciSegmentRead16 (XhciPciBase + PCI_DEVICE_ID_OFFSET);
  HcInfo->DeviceId  = ControllerDid;

  switch (HcInfo->DeviceId) {
    case V_XHCI_DID_PCH_LP_V16_0:
    case V_XHCI_DID_PCH_H_V16_0:
      HcInfo->OnSouth   = TRUE;
      HcInfo->IpVersion = V16_0;
      break;

    case V_XHCI_DID_CPU_LP_V17_0:
      HcInfo->OnSouth   = FALSE;
      HcInfo->IpVersion = V17_0;
      break;

    case V_XHCI_DID_PCH_LP_V17_1:
      HcInfo->OnSouth   = TRUE;
      HcInfo->IpVersion = V17_1_A;
      break;

    case V_XHCI_DID_PCH_N_V17_1:
    case V_XHCI_DID_PCH_H_V17_1:
      HcInfo->OnSouth   = TRUE;
      HcInfo->IpVersion = V17_1_B;
      break;

    case V_XHCI_DID_PCH_B_V18_0:
    case V_XHCI_DID_PCH_LP_V18_0:
      HcInfo->OnSouth   = TRUE;
      HcInfo->IpVersion = V18_0;
      break;

    default:
      DEBUG ((DEBUG_ERROR, "Trying to configure unknown xHCI controller with DID 0x%4X\n", HcInfo->DeviceId));
      ASSERT (FALSE);
      break;
  }

  if ((HcInfo->IpVersion == V16_0) ||
      (HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V17_1)) {
    HcInfo->UsbLtrHigh = V_XHCI_LTR_HIT_1;
    HcInfo->UsbLtrMid  = V_XHCI_LTR_MIT_1;
    HcInfo->UsbLtrLow  = V_XHCI_LTR_LIT_1;
  } else {
    HcInfo->UsbLtrHigh = V_XHCI_LTR_HIT_2;
    HcInfo->UsbLtrMid  = V_XHCI_LTR_MIT_2;
    HcInfo->UsbLtrLow  = V_XHCI_LTR_LIT_2;
  }

  DEBUG ((DEBUG_INFO, "xHCI Controller DID: 0x%4X\n", ControllerDid));
  DEBUG ((DEBUG_INFO, "Params:\n UsbIpVersion: 0x%2X, Device: 0x%4X\n", HcInfo->IpVersion, HcInfo->DeviceId));
}

/**
  Function for checking if one of the USB3 ports is used for debug purposes

  @param[in]  XhciMmioBase      Memory BAR of the xHCI controller
  @param[out] RetDebugEnable    Debug 0 indexed port number if found, otherwise 0xFF returned

  @retval     BOOLEAN           TRUE if debug port found on one of the ports
**/
STATIC
BOOLEAN
GetDebugPortIndex (
  IN  UINT32      XhciMmioBase,
  OUT UINT32       *RetDebugPortSsIndex
  )
{
  UINT32          DebugPortSsIndex;
  UINT32          SsPortCount;
  UINT32          HsPortCount;
  UINT32          DebugPort;
  UINT32          CapabilityPointer;
  UINT32          Capability;
  BOOLEAN         DebugEnable;

  HsPortCount     = GetUsb2PortCount (XhciMmioBase);
  SsPortCount     = GetUsb3PortCount (XhciMmioBase);

  //
  // Get debug enable status in order to skip some XHCI init which
  // may break XHCI debug
  //
  CapabilityPointer = (UINT32) (XhciMmioBase + (MmioRead32 (XhciMmioBase + R_XHCI_MEM_HCCPARAMS1) >> 16) * 4);
  DebugEnable       = FALSE;
  DebugPortSsIndex  = 0xFF;
  Capability        = MmioRead32 (CapabilityPointer);

  DEBUG ((DEBUG_INFO, "XHCI Capability Pointer = 0x%x\n", CapabilityPointer));

  while (TRUE) {
    if ((Capability & B_XHCI_MEM_CAPABILITY_ID) == V_XHCI_MEM_DBC_DCID) {
      //
      // Check DCR bit in DCCTRL register (Debug Capability Base + 20h), if set, debug device is running
      //
      if ((MmioRead32 (CapabilityPointer + R_XHCI_MEM_DBC_DCCTRL) & B_XHCI_MEM_DBC_DCCTRL_DCR) != 0) {
        DebugEnable = TRUE;
        //
        // Get debug port number [24:31] in DCST register which starts from 1
        //
        DebugPort = (MmioRead32 (CapabilityPointer + R_XHCI_MEM_DBC_DCST) >> N_XHCI_MEM_DBC_DCST_DBG_PORT_NUMBER);
        //
        // Veryfing if debug port number falls within Super Speed port boundaries
        //
        if (DebugPort > HsPortCount) {
          if (DebugPort - HsPortCount < SsPortCount) {
            //
            // Translate to 0-based super speed port numbering
            //
            DebugPortSsIndex = DebugPort - HsPortCount - 1;
            DEBUG ((DEBUG_INFO, "DebugPortSsIndex = ?%d\n", DebugPortSsIndex));
            break;
          }
        }
      }
    }
    if ((((Capability & B_XHCI_MEM_CAPABILITY_NEXT_CAP_PTR) >> N_XHCI_MEM_CAPABILITY_NEXT_CAP_PTR) & B_XHCI_MEM_CAPABILITY_ID) == 0) {
      //
      // Reached the end of list, quit
      //
      break;
    }
    CapabilityPointer += ((Capability & B_XHCI_MEM_CAPABILITY_NEXT_CAP_PTR) >> N_XHCI_MEM_CAPABILITY_NEXT_CAP_PTR) * 4;
    Capability = MmioRead32 (CapabilityPointer);
  }

  *RetDebugPortSsIndex = DebugPortSsIndex;

  return DebugEnable;
}

/*
  Helper function to initiate reset on USB3 ports

  @param[in]  XhciMmioBase      Memory BAR of the xHCI controller
*/
STATIC
VOID
XhciInitiatePortReset (
  IN  UINT32           XhciMmioBase
  )
{
  BOOLEAN     DebugEnable;
  UINT32      PortSCxUsb3Base;
  UINT32      DebugPortSsIndex;
  UINT32      UsbPort;
  UINT32      SsPortCount;
  //
  // Perform WPR on USB3 port except for the port has DBC enabled.
  //
  //
  // Get debug enable status in order to skip some XHCI init which
  // may break XHCI debug
  //
  DebugEnable     = FALSE;
  PortSCxUsb3Base = GetUsb3PortScBase (XhciMmioBase);
  SsPortCount     = GetUsb3PortCount (XhciMmioBase);

  //
  // Perform WPR on USB3 port except for the port has DBC enabled.
  //
  DebugEnable = GetDebugPortIndex (XhciMmioBase, &DebugPortSsIndex);

  //
  // Initiate warm reset to all USB3 ports except for the USB3 port which has Dbc enabled
  //
  for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {
    if ((DebugEnable) && (UsbPort == DebugPortSsIndex)) {
      continue;
    }
    MmioAndThenOr32 (
      XhciMmioBase + (PortSCxUsb3Base + (UsbPort * S_XHCI_MEM_PORTSC_PORT_SPACING)),
      (UINT32) ~ (B_XHCI_MEM_PORTSCXUSB3_PED),
      B_XHCI_MEM_PORTSCXUSB3_WPR
      );
  }
}

/**
  Program and enable XHCI Memory Space

  @param[in] XhciPciBase          XHCI PCI Base Address
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
**/
STATIC
VOID
XhciMemorySpaceOpen (
  IN  UINT64                      XhciPciBase,
  IN  UINT32                      XhciMmioBase
  )
{
  //
  // Assign memory resources
  //
  PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_BAR0, XhciMmioBase);
  PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_BAR0 + 4, 0);

  PciSegmentOr16 (
    XhciPciBase + PCI_COMMAND_OFFSET,
    (UINT16) (EFI_PCI_COMMAND_MEMORY_SPACE)
    );
}

/**
  Clear and disable XHCI Memory Space

  @param[in] XhciPciBase          XHCI PCI Base Address
**/
STATIC
VOID
XhciMemorySpaceClose (
  IN  UINT64                      XhciPciBase
  )
{
  //
  // Clear memory resources
  //
  PciSegmentAnd16 (
    XhciPciBase + PCI_COMMAND_OFFSET,
    (UINT16) ~(EFI_PCI_COMMAND_MEMORY_SPACE)
    );

  PciSegmentWrite32 ((XhciPciBase + R_XHCI_CFG_BAR0), 0);
  PciSegmentWrite32 ((XhciPciBase + R_XHCI_CFG_BAR0 + 4), 0);
}

/**
  Performs USB3 warm port reset.

  @param[in] XhciMmioBase         XHCI Mmio Base Address
**/
STATIC
VOID
Usb3WarmPortReset (
  IN  UINT32    XhciMmioBase
  )
{
  UINT32          SsPortCount;
  UINTN           PortSCxUsb3Base;
  UINT32          Data32;
  UINTN           Timeout;
  UINT32          DebugPortSsIndex;
  UINT32          UsbPort;
  BOOLEAN         DebugEnable;

  DebugEnable     = FALSE;
  PortSCxUsb3Base = GetUsb3PortScBase (XhciMmioBase);
  SsPortCount     = GetUsb3PortCount (XhciMmioBase);

  //
  // Perform WPR on USB3 port except for the port has DBC enabled.
  //
  DebugEnable = GetDebugPortIndex (XhciMmioBase, &DebugPortSsIndex);

  //
  // Poll for warm reset bit to be cleared or timeout at 120ms
  //
  Timeout = 0;
  while (TRUE) {
    Data32 = 0;
    for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {
      if ((DebugEnable) && (UsbPort == DebugPortSsIndex)) {
        continue;
      }
      Data32 |= MmioRead32 (XhciMmioBase + (PortSCxUsb3Base + (UsbPort * S_XHCI_MEM_PORTSC_PORT_SPACING)));
    }
    if (((Data32 & B_XHCI_MEM_PORTSCXUSB3_PR) == 0) || (Timeout > PORT_RESET_TIMEOUT)) {
      break;
    }
    MicroSecondDelay (10);
    Timeout++;
  }

  if (Timeout > PORT_RESET_TIMEOUT) {
    DEBUG ((DEBUG_WARN, "Waiting for USB3 port reset has timed out\n"));
  }

  //
  // Clear all the port's status by program xHCIBAR + PortSC offset [23:17] to 1111111b.
  //
  for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {
    if ((DebugEnable) && (UsbPort == DebugPortSsIndex)) {
      continue;
    }
    MmioAndThenOr32 (
      XhciMmioBase + (PortSCxUsb3Base + (UsbPort * S_XHCI_MEM_PORTSC_PORT_SPACING)),
      (UINT32) ~ (B_XHCI_MEM_PORTSCXUSB3_PED),
      B_XHCI_MEM_PORTSCXUSB3_CEC |
      B_XHCI_MEM_PORTSCXUSB3_PLC |
      B_XHCI_MEM_PORTSCXUSB3_PRC |
      B_XHCI_MEM_PORTSCXUSB3_OCC |
      B_XHCI_MEM_PORTSCXUSB3_WRC |
      B_XHCI_MEM_PORTSCXUSB3_PEC |
      B_XHCI_MEM_PORTSCXUSB3_CSC
      );
  }
}

/**
  Setup XHCI Over-Current Mapping

  @param[in] HcInfo               xHCI controller information structure
  @param[in] UsbConfig            The policy for USB configuration
  @param[in] XhciPciBase          XHCI PCI Base Address
  @param[in] XhciMmioBase         XHCI Memory Bar base address
**/
STATIC
VOID
XhciOverCurrentMapping (
  IN  CONST USB_CONTROLLER_INFO   *HcInfo,
  IN  CONST USB_CONFIG            *UsbConfig,
  IN  UINT64                      XhciPciBase,
  IN  UINT32                      XhciMmioBase
  )
{
  UINT32                   Index;
  UINT32                   U2OcMBuf[PCH_USB_OC_PINS_MAX];
  UINT32                   U3OcMBuf[PCH_USB_OC_PINS_MAX];
  UINT32                   OcPin;
  UINT32                   OcPinsUsedMask;

  if (!UsbConfig->OverCurrentEnable) {
    //
    // Clear Over-Current registers to switch off Over-Current detection.
    //
    DEBUG ((DEBUG_WARN, "Clear Over-Current registers to enable OBS pins usage\n"));

    for (Index = 0; Index < PCH_USB_OC_PINS_MAX; Index++) {
      if (HcInfo->IpVersion >= V17_1_B) {
        MmioWrite32 (XhciMmioBase + R_XHCI_MEM_U2OCM + (Index * 4), 0);
        MmioWrite32 (XhciMmioBase + R_XHCI_MEM_U3OCM + (Index * 4), 0);
      } else {
        PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_U2OCM + (Index * 4), 0);
        PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_U3OCM + (Index * 4), 0);
      }
    }

    //
    // Exit function after clearing Overcurrent mapping
    //
    return;
  }

  ZeroMem (U2OcMBuf, sizeof (U2OcMBuf));
  ZeroMem (U3OcMBuf, sizeof (U3OcMBuf));
  OcPinsUsedMask = 0;

  for (Index = 0; Index < GetPchUsb2MaxPhysicalPortNum (); Index++) {
    if (UsbConfig->PortUsb20[Index].OverCurrentPin != UsbOverCurrentPinSkip) {
      OcPin = UsbConfig->PortUsb20[Index].OverCurrentPin;
      ASSERT (OcPin < PCH_USB_OC_PINS_MAX);
      U2OcMBuf[OcPin] |= (UINT32) (BIT0 << Index);
      OcPinsUsedMask |= (UINT32) (BIT0 << OcPin);
    }
  }

  for (Index = 0; Index < GetPchXhciMaxUsb3PortNum (); Index++) {
    if (UsbConfig->PortUsb30[Index].OverCurrentPin != UsbOverCurrentPinSkip) {
      OcPin = UsbConfig->PortUsb30[Index].OverCurrentPin;
      ASSERT (OcPin < PCH_USB_OC_PINS_MAX);
      U3OcMBuf[OcPin] |= (UINT32) (BIT0 << Index);
      OcPinsUsedMask |= (UINT32) (BIT0 << OcPin);
    }
  }

  for (Index = 0; Index < PCH_USB_OC_PINS_MAX; Index++) {
    if (HcInfo->IpVersion >= V17_1_B) {
      MmioWrite32 (XhciMmioBase + R_XHCI_MEM_U2OCM + (Index * 4), U2OcMBuf[Index]);
      MmioWrite32 (XhciMmioBase + R_XHCI_MEM_U3OCM + (Index * 4), U3OcMBuf[Index]);
    } else {
      PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_U2OCM + (Index * 4), U2OcMBuf[Index]);
      PciSegmentWrite32 (XhciPciBase + R_XHCI_CFG_U3OCM + (Index * 4), U3OcMBuf[Index]);
    }
  }

  for (Index = 0; Index < PCH_USB_OC_PINS_MAX; Index++) {
    if ((OcPinsUsedMask >> Index) & BIT0) {
      GpioEnableUsbOverCurrent (Index);
    }
  }
}

/**
  Configure xHCI after initialization

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciBase         XHCI PCI CFG Base Address
**/
VOID
XhciConfigureAfterInit (
  IN  USB_CONFIG      *UsbConfig,
  IN  UINT64          XhciPciBase
  )
{
  USB_CONTROLLER_INFO   HcInfo;
  UINT32                XhciMmioBase;

  GetXhciControllerInfo (XhciPciBase, &HcInfo);

  XhciMmioBase = PcdGet32 (PcdSiliconInitTempMemBaseAddr);

  //
  // Assign memory resources
  //
  XhciMemorySpaceOpen (
    XhciPciBase,
    XhciMmioBase
    );

  Usb3WarmPortReset (XhciMmioBase);

  //
  // Clear memory resources
  //
  XhciMemorySpaceClose (
    XhciPciBase
    );
}

/**
  Performs basic configuration of USB3 (xHCI) controller.

  @param[in]  HcInfo              xHCI controller information structure
  @param[in]  XhciPciBase         xHCI PCI config space address
  @param[in]  XhciMmioBase        Memory base address of xHCI Controller
  @param[in]  UsbConfig           Instance of USB_CONFIG Config Block
**/
STATIC
VOID
XhciHcInit (
  IN  USB_CONTROLLER_INFO   *HcInfo,
  IN  UINT64                XhciPciBase,
  IN  UINT32                XhciMmioBase,
  IN  USB_CONFIG            *UsbConfig
  )
{
  UINT32          Data32;
  UINT32          Data32Or;
  UINT32          Data32And;
  UINT16          Data16Or;
  UINT32          HsPortCount;
  UINT32          SsPortCount;
  UINT32          XhciUsb3Pdo;
  UINT32          XhciUsb3PortsMask;

  DEBUG ((DEBUG_INFO, "XhciHcInit XhciMmio Base = 0x%x\n", XhciMmioBase));

  HsPortCount = GetUsb2PortCount (XhciMmioBase);
  SsPortCount = GetUsb3PortCount (XhciMmioBase);

  DEBUG ((DEBUG_INFO, "Number of supported Super Speed Ports  = %d\n", SsPortCount));
  DEBUG ((DEBUG_INFO, "Number of supported High Speed Ports   = %d\n", HsPortCount));

  //
  //  XHCC1 - XHC System Bus Configuration 1
  //  Address Offset: 0x40
  //  Value: [21:19] 110b, [18] 1b, [8] 1b
  //  Writes to this registers needs to be performed per bytes to avoid touching bit 31
  //  Bit 31 is used to lock RW/L bits and can be writen once.
  //
  PciSegmentOr8 (
    XhciPciBase + R_XHCI_CFG_XHCC1 + 1,
    (UINT32) (BIT0)
    );
  PciSegmentOr8 (
    XhciPciBase + R_XHCI_CFG_XHCC1 + 2,
    (UINT32) (BIT5 | BIT4 | BIT2)
    );

  //
  //  DBGDEV_CTRL_REG1 - Debug Device Control Register 1
  //  Address Offset: 0x8754
  //  Value: [9] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_DBGDEV_CTRL_REG1),
    (UINT32) (BIT9)
    );

  //
  //  XHCC2 - XHC System Bus Configuration 2
  //  Address Offset: 0x44
  //  Value: [22:21] 11b, [20:14] 0x7F, [11] 0b, [10] 1b, [9:8] 10b, [7:6] 10b, [5:3] 001b, [0] 1b
  //  xHCI on North and USB IP V16.0 [25:23] 111b [2:1] 11b
  //
  Data32Or  = (BIT22 | BIT21 | 0x7F << N_XHCI_CFG_XHCC2_UNPPA | BIT10 | BIT9 | BIT7 | BIT3 | BIT0);
  Data32And = (UINT32)~(BIT11 | BIT8 | BIT6 | BIT5 | BIT4);
  if (!HcInfo->OnSouth || (HcInfo->IpVersion == V16_0)) {
    Data32Or |= (BIT25 | BIT24 | BIT23 | BIT2 | BIT1);
  }
  PciSegmentAndThenOr32 (
    XhciPciBase + R_XHCI_CFG_XHCC2,
    Data32And,
    Data32Or
    );

  //
  //  PMCTRL2 - Power Management Control 2
  //  Address Offset: 0x8468
  //  Value: [5] 1b, [0] 1b
  //  This bits must be set before setting USB_SRAM_PG_EN (bit27) in XHCLKGTEN
  //  For CNP-LP Ax steppings:
  //    [5] 0b, [0] 0b
  //
  if ((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) && (PchStepping () < PCH_B0)) {
    MmioAnd32 (
      (XhciMmioBase + R_XHCI_MEM_PMCTRL2),
      (UINT32)~(BIT5 | BIT0)
      );
  } else {
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_PMCTRL2),
      (UINT32) (BIT5 | BIT0)
      );
  }

  //
  //  XHCLKGTEN - Clock Gating
  //  Address Offset: 0x50
  //  Value: 0x0FDC6D7F
  //  For devices with IP V17.1 or newer excluding device with id 0x34ED
  //    [17:16] 11b, [6] 0b
  //
  Data32 = 0x0FDC6D7F;
  if ((HcInfo->IpVersion >= V17_1_B) ||
     ((HcInfo->IpVersion == V17_0) && !HcInfo->IsCpuStepA0)) {
    Data32 |= (BIT17 | BIT16);
    Data32 &= (UINT32)~(BIT6);
  }
  PciSegmentWrite32 (
    (XhciPciBase + R_XHCI_CFG_XHCLKGTEN),
    Data32
    );

  //
  //  MSI_NEXT - Next Item Pointer
  //  Address Offset: 0x81
  //  Value: [7:0] 90h
  //
  PciSegmentWrite8 (
    (XhciPciBase + R_XHCI_CFG_MSI_NEXT),
    0x90
    );

  //
  //  PCE - Power Control Enables
  //  Address Offset: 0xA2
  //  Value: [3] 1b, [2] 1b, [1] 1b, [0] 0b
  //  For IP newer than v16.0  [5] 1b
  //
  Data16Or = (UINT16) (BIT3 | BIT2 | BIT1);
  if (HcInfo->IpVersion > V16_0) {
    Data16Or |= BIT5;
  }
  PciSegmentAndThenOr16 (
    (XhciPciBase + R_XHCI_CFG_PCE),
    (UINT16)~(BIT0),
    Data16Or
    );

  //
  //  HSCFG2 - High Speed Configuration 2
  //  Address Offset: 0xA4
  //  Value: [15:11] 00011b
  //
  PciSegmentAndThenOr32 (
    (XhciPciBase + R_XHCI_CFG_HSCFG2),
    (UINT32) ~(BIT15 | BIT14 | BIT13),
    (UINT32) (BIT12 | BIT11)
    );

  //
  //  SSCFG1 - SuperSpeed Configuration 1
  //  Address Offset: 0xA8
  //  Value: [17] 1b, [14] 1b
  //
  PciSegmentOr32 (
    (XhciPciBase + R_XHCI_CFG_SSCFG1),
    (UINT32) (BIT17 | BIT14)
    );

  //
  //  HSCFG1 - High Speed Configuration 1
  //  Address offset: 0xAC
  //  Value: [19:18] 00b
  //
  PciSegmentAnd32 (
    (XhciPciBase + R_XHCI_CFG_HSCFG1),
    (UINT32)~(BIT19 | BIT18)
    );

  //
  //  XHCC3 - XHC System Bus Configuration 3
  //  Address Offset: 0xFC
  //  Value: [4] 1b
  //
  PciSegmentOr32 (
    (XhciPciBase + R_XHCI_CFG_XHCC3),
    (UINT32) (BIT4)
    );

  if (HcInfo->IpVersion == V16_0) {
    //
    //  HCIVERSION - Host Controller Interface Version Number
    //  Address Offset: 0x02
    //  Value: [15:0] = 0110h
    //  Only for IP V16.0
    //
    MmioAndThenOr32 (
      (XhciMmioBase + R_XHCI_MEM_CAPLENGTH),
      (UINT32) B_XHCI_MEM_HCIVERSION,
      (UINT32) (V_XHCI_MEM_HCIVERSION << N_XHCI_MEM_HCIVERSION)
      );
  }

  //
  //  HCSPARAMS3 - Structural Parameters 3
  //  Address Offset: 0x0C
  //  Value: [31:16] = A0h, [7:0] = 0Ah
  //  For USB IP V16 [31:16] = 396h
  //
  Data32Or = 0x00A0000A;
  if (HcInfo->IpVersion == V16_0) {
    Data32Or = (0x0396 << N_XHCI_MEM_HCSPARAMS3_U2DEL) | (Data32Or & B_XHCI_MEM_HCSPARAMS3_U1DEL);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HCSPARAMS3),
    B_XHCI_MEM_HCSPARAMS3,
    Data32Or
    );

  //
  //  HCCPARAMS1 - Capability Parameters 1
  //  Address Offset: 0x10
  //  Value: For USB IP V16 [11] 0b else [11] 1b
  //
  if (HcInfo->IpVersion == V16_0) {
    MmioAnd32 (
      (XhciMmioBase + R_XHCI_MEM_HCCPARAMS1),
      (UINT32)~(BIT11)
      );
  } else {
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_HCCPARAMS1),
      (UINT32) (BIT11)
      );
  }

  //
  //  HCCPARAMS2 - Capability Parameters 2
  //  Address Offset: 0x1C
  //  Value: [6] 1b, [4] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_HCCPARAMS2),
    (UINT32) (BIT6 | BIT4)
    );

  //
  //  XECP_CMDM_CTRL_REG1 - Command Manager Control 1
  //  Address Offset: 0x818C
  //  Value: [20] 0b, [16] 1b, [8] 0b
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_XECP_CMDM_CTRL_REG1),
    (UINT32)~(BIT20 | BIT8),
    (UINT32) (BIT16)
    );

  //
  //  XECP_CMDM_CTRL_REG2 - Command Manager Control 2
  //  Address offset: 0x8190
  //  Value: [14] 0b
  //
  MmioAnd32 (
    (XhciMmioBase + R_XHCI_MEM_XECP_CMDM_CTRL_REG2),
    (UINT32)~(BIT14)
    );

  //
  //  XECP_CMDM_CTRL_REG3 - Command Manager Control 3
  //  Address offset: 0x8194
  //  Value: [25] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_XECP_CMDM_CTRL_REG3),
    (UINT32) (BIT25)
    );

  //
  //  HOST_CTRL_ODMA_REG - Host Control ODMA Register
  //  Address Offset: 0x8098
  //  Value: [2:1] 00b
  //
  MmioAnd32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_ODMA_REG),
    (UINT32)~(BIT2 | BIT1)
    );

  //
  //  PMCTRL - Power Management Control
  //  Address Offset: 0x80A4
  //  Value: [31:29] 010b, [27] 1b, [25:22] 0100b, [16] 1b, [15:8] 50h, [3] 1b, [2] 1b
  //  Additionaly for Device with ID 0x98ED [23:22] 11b
  //
  Data32Or = (UINT32) (BIT30 | BIT27 | BIT24 | BIT16 | (0x50 << N_XHCI_MEM_PMCTRL_SSU3LPFS_DET) | BIT3 | BIT2);
  if (HcInfo->DeviceId == V_XHCI_DID_PCH_B_V18_0) {
    Data32Or |= (UINT32) (BIT23 | BIT22);
  }
  if (HcInfo->IsCpuStepA0 ||
     ((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V17_1) && (PchStepping () < PCH_B0))) {
    Data32Or |= BIT17;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_PMCTRL),
    (UINT32) ~(BIT31 | BIT29 | BIT25 | BIT23 | BIT22 | B_XHCI_MEM_PMCTRL_SSU3LFPS_DET),
    Data32Or
    );

  //
  //  AUX_CTRL_REG3 - Aux PM Control 3 Register
  //  Address Offset: 0x81C8
  //  Value: [6] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_AUX_CTRL_REG3),
    (UINT32) (BIT6)
    );

  //
  //  SSPE - Super Speed Port Enables
  //  Address Offset: 0x80B8
  //  Value: [30] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_SSPE),
    (UINT32) (BIT30)
    );

  //
  //  AUX_CTRL_REG1 - AUX Power Management Control
  //  Address Offset: 0x80E0
  //  Value: [16] 0b, [9] 0b, [6] 1b
  //  For CNP Ax steppings [16] 1b
  //
  Data32Or = (UINT32) (BIT6);
  if (((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) || (HcInfo->DeviceId == V_XHCI_DID_PCH_H_V16_0)) && (PchStepping () < PCH_B0)) {
    Data32Or |= BIT16;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_AUX_CTRL_REG1),
    (UINT32) ~(BIT16 | BIT9),
    Data32Or
    );

  //
  //  HOST_CTRL_SCH_REG - Host Control Scheduler
  //  Address Offset: 0x8094
  //  Value: [23] 1b, [22] 1b, [21] 0b, [14] 0b, [6] 1b
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SCH_REG),
    (UINT32)~(BIT21 | BIT14),
    (UINT32) (BIT23 | BIT22 | BIT6)
    );

  //
  //  HOST_CTRL_PORT_LINK_REG - SuperSpeed Port Link Control
  //  Address Offset: 0x80EC
  //  Value: [19] 0b, [17] 1b
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_PORT_LINK_REG),
    (UINT32)~(BIT19),
    (UINT32) (BIT17)
    );

  //
  //  USB2_LINK_MGR_CTRL_REG1 - USB2 Port Link Control 1, 2, 3, 4
  //  Address Offset: 0x80F0
  //  [127:96] is mapped to DW4 at offset 80FCh-80FFh [25] 1b
  //  [31:0]   is mapped to DW1 at offset 80F0h-80F3h [20] 0b
  //  For xHCI on south [22] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_USB2_LINK_MGR_CTRL_REG1_DW4),
    (UINT32) (BIT25)
    );
  Data32And = (UINT32) ~(BIT20);
  Data32Or  = 0u;
  if (HcInfo->OnSouth) {
    Data32 |= BIT22;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_USB2_LINK_MGR_CTRL_REG1_DW1),
    Data32And,
    Data32Or
    );

  //
  //  HOST_CTRL_BW_CTRL_REG - Host Controller Bandwith Control Register
  //  Address Offset: 0x8100
  //  Value: [21] 1b [20] 1b
  //  Program only for IPs V16.0, V18.0 or for devices with IDs:
  //    0x8A13, 0x34ED or 0x38ED
  //
  if ((HcInfo->IpVersion <= V17_1_A) ||
      (HcInfo->DeviceId == V_XHCI_DID_PCH_N_V17_1)) {
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_BW_CTRL_REG),
      (UINT32) (BIT20 | BIT21)
      );
  }

  //
  //  HOST_IF_CTRL_REG - Host Controller Interface Control Register
  //  Address Offset: 0x8108
  //  Value: [30] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_IF_CTRL_REG),
    (UINT32) (BIT30)
    );

  //
  //  HOST_CTRL_TRM_REG2 - Host Controller Transfer Manager Control 2
  //  Address Offset: 0x8110
  //  Value: [20] 1b, [11] 1b, [2] 0b,
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_TRM_REG2),
    (UINT32)~(BIT2),
    (UINT32) (BIT20 | BIT11)
    );

  //
  //  HOST_CTRL_BW_MAX_REG - MAX BW Control Reg 4
  //  Address Offset: 0x8128
  //  Value: V16_0 [47:36] - 0xFFF for CNP A0, 0x528 (default) for rest
  //  Value: [23:12] 0x753
  //
  if (HcInfo->OnSouth) {
    if (((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) ||
         (HcInfo->DeviceId == V_XHCI_DID_PCH_H_V16_0)) && (PchStepping () == PCH_A0)) {
      MmioOr32 (
        (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_BW_MAX_REG + 4),
        (UINT32) (0xFFF << 4)
        );
    }
    MmioAndThenOr32 (
      (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_BW_MAX_REG),
      (UINT32)~(B_XHCI_MEM_HOST_CTRL_BW_MAX_REG_MAX_HS_BW),
      (UINT32) (0x753 << N_XHCI_MEM_HOST_CTRL_BW_MAX_REG_MAX_HS_BW)
      );
  }

  //
  //  AUX_CTRL_REG2 - Aux PM Control Register 2
  //  Address Offset: 0x8154
  //  Value: [31] 1b, [21] 0b, [13] 1b
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_AUX_CTRL_REG2),
    (UINT32) ~(BIT21),
    (UINT32) (BIT31 | BIT13)
    );

  //
  //  AUXCLKCTL - xHCI Aux Clock Control Register
  //  Address Offset: 0x816C
  //  Value: [19:16] 1111b, [14] 1b, [13:12] 00b, [11:8] 0h, [5:2] 1111b
  //  For Device with ID 0x9DED and PCH stepping B1 [19:18] 00b
  //
  Data32Or = (UINT32) (BIT17 | BIT16 | BIT14 | BIT5 | BIT4 | BIT3 | BIT2);
  if ((HcInfo->DeviceId != V_XHCI_DID_PCH_LP_V16_0) || (PchStepping () != PCH_B1)) {
    Data32Or |= (BIT19 | BIT18);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_AUXCLKCTL),
    (UINT32) ~(BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8),
    Data32Or
    );

  //
  //  HOST_IF_PWR_CTRL_REG0 - Power Scheduler Control 0
  //  Address Offset: 0x8140
  //  Value: [31:24] 0xFF, [23:12] 0x00F, [11:0] 0x03C
  //
  MmioWrite32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_IF_PWR_CTRL_REG0),
    0xFF00F03C
    );

  //
  //  HOST_IF_PWR_CTRL_REG1 - Power Scheduler Control 1
  //  Address Offset: 0x8144
  //  Value: [8] 0b
  //  For devices on south with IP V17.1 or newer excluding device with ID 0x34ED
  //    [24] 1b
  //  Only for USB V16.0
  //    [8] 1b
  //
  Data32And = (UINT32)~(BIT8);
  Data32Or  = 0;
  if ((HcInfo->IpVersion >= V17_1_B) && HcInfo->OnSouth) {
    Data32Or = (UINT32) (BIT24);
  }
  if (HcInfo->IpVersion == V16_0) {
    Data32And |= BIT8;
    Data32Or  |= BIT8;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_IF_PWR_CTRL_REG1),
    Data32And,
    Data32Or
    );

  //
  //  USBLPM - USB LPM Parameters
  //  Address Offset: 0x8170
  //  Value: [18:16] 000b
  //  For USB IP V16 [9:0] 0000000000b
  //
  Data32And = (UINT32)~(BIT18 | BIT17 | BIT16);
  if (HcInfo->IpVersion == V16_0) {
    Data32And &= (UINT32)~(BIT9 | BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
  }
  MmioAnd32 (
    (XhciMmioBase + R_XHCI_MEM_USBLPM),
    Data32And
    );

  //
  //  xHC Latency Tolerance Parameters - LTV Control
  //  Address Offset: 0x8174
  //  Value: [30]   1b
  //         [24]   1b
  //         [11:0] 0xC0A (for xHCI in PCH)
  //
  Data32And = (UINT32)~(0);
  Data32Or  = (BIT30 | BIT24);

  if (HcInfo->OnSouth) {
    Data32And = (UINT32)~(0xFFF);
    Data32Or |= 0xC0A;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_XHCLTVCTL),
    Data32And,
    Data32Or
    );

  //
  //  xHC Latency Tolerance Parameters - High Idle Time Control
  //  Address Offset: 0x817C
  //
  //  For USB IP v16 or device with ID 0x34ED
  //    Value - 0x033200A3
  //  else
  //    Value - 0x00050002
  //
  MmioWrite32 (
    (XhciMmioBase + R_XHCI_MEM_LTVHIT),
    HcInfo->UsbLtrHigh
    );

  //
  //  xHC Latency Tolerance Parameters - Medium Idle Time Control
  //  Address Offset: 0x8180
  //
  //  For USB IP v16 or device with ID 0x34ED
  //    Value - 0x00CB0028
  //  else
  //    Value - 0x00050002
  //
  MmioWrite32 (
    (XhciMmioBase + R_XHCI_MEM_LTVMIT),
    HcInfo->UsbLtrMid
    );

  //
  //  xHC Latency Tolerance Parameters - Low Idle Time Control
  //  Address Offset: 0x8184
  //
  //  For USB IP v16 or device with ID 0x34ED
  //    Value - 0x0099001E
  //  else
  //    Value - 0x00050002
  //
  MmioWrite32 (
    (XhciMmioBase + R_XHCI_MEM_LTVLIT),
    HcInfo->UsbLtrLow
    );

  //
  //  Host Controller Misc Reg
  //  Address Offset: 0x80B0
  //  Value: [24] 0b, [23] 1b, [18:16] 000b
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_MISC_REG),
    (UINT32)~(BIT24 | BIT18| BIT17 | BIT16),
    (UINT32) (BIT23)
    );

  //
  //  Host Controller Misc Reg 2
  //  Address Offset: 0x80B4
  //  Value: [7] 1b, [5] 0b
  //  For USB IP V16 [28] 0b, [2] 0b else [28] 1b, [2] 1b
  //
  Data32And = (UINT32)~(BIT5);
  Data32Or  = (UINT32) (BIT7);
  if (HcInfo->IpVersion == V16_0) {
    Data32And &= (UINT32)~(BIT28);
  } else {
    Data32Or  |= BIT28 | BIT2;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_MISC_REG_2),
    Data32And,
    Data32Or
    );

  //
  //  HOST_BW_OV_HS_REG - High Speed TT Bandwidth Overhead
  //  Address Offset: 0x80C8
  //  Value:  For xHCI in PCH [23:12] 00Ah
  //          [11:0] 0h
  //
  Data32And = (UINT32)~(B_XHCI_MEM_HOST_BW_OV_HS_REG_OVHD_HSTTBW);
  Data32Or  = 0;
  if (HcInfo->OnSouth) {
    Data32And  &= (UINT32)~(B_XHCI_MEM_HOST_BW_OV_HS_REG_OVHD_HSBW);
    Data32Or   |= (0xA << N_XHCI_MEM_HOST_BW_OV_HS_REG_OVHD_HSBW);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_BW_OV_HS_REG),
    Data32And,
    Data32Or
    );

  //
  //  THROTT - XHCI Throttle Control
  //  Address Offset: 0x819C
  //  Value: [20] 1b, [14:12] 111b, [11:8] 0x3, [7:4] 0x7, [3:0] 0xD
  //  For IP V16.0  [16] 1b
  //  For IP V17.1 and earlier [20] 1b
  //
  Data32Or = (UINT32) (BIT14 | BIT13 | BIT12 | BIT9 | BIT8 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT0);
  if (HcInfo->IpVersion == V16_0) {
    Data32Or |= (BIT16);
  }
  if (HcInfo->IpVersion < V18_0) {
    Data32Or |= (BIT20);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_THROTT),
    (UINT32) ~(BIT11 | BIT10 | BIT7 | BIT1),
    Data32Or
    );

  //
  //  THROTT2 - XHCI Throttle Control2
  //  Address Offset: 0x81B4
  //  Value: [31:0] 0h
  //
  MmioWrite32 (
    (XhciMmioBase + R_XHCI_MEM_THROTT2),
    0x0
    );

  //
  //  LFPSONCOUNT - LFPS On Count
  //  Address Offset: 0x81B8
  //  Value: [18] 0b only for device with ID 0x8A13 A0 stepping
  //
  if ((HcInfo->DeviceId == V_XHCI_DID_CPU_LP_V17_0) && HcInfo->IsCpuStepA0) {
    MmioAnd32 (
      (XhciMmioBase + R_XHCI_MEM_LFPSONCOUNT),
      (UINT32)~(BIT18)
      );
  }

  //
  //  D0I2CTRL - D0I2 Control Register
  //  Address Offset: 0x81BC
  //  Value: [31] 1b, [29:26] 4h, [25:22] 0h, [21] 0b, [20:16] 4h, [15:4] 20h, [2:0] 0h
  //  For USB IP V18.0
  //    [25:22] 1h, [20:16] 0Fh
  //  For devices with IDs: 0x9DED, 0xA36D, 0x8A13 (only A0 stepping) and 0x34ED
  //    [3] 1b
  //
  Data32Or = (UINT32) (BIT31
           | (0x4 << N_XHCI_MEM_D0I2CTRL_D0I2_MIN_RESIDENCY)
           | (0x20 << N_XHCI_MEM_D0I2CTRL_MSI_IDLE_THRESHOLD));
  if (HcInfo->IpVersion >= V18_0) {
    Data32Or |= (0x1 << N_XHCI_MEM_D0I2CTRL_D0I2_ENTRY_HYSTERESIS_TIMER
               | 0xF << N_XHCI_MEM_D0I2CTRL_MSID0I2PWT);
  } else {
    Data32Or |= (0x4 << N_XHCI_MEM_D0I2CTRL_MSID0I2PWT);
  }
  if ((HcInfo->IpVersion == V16_0) ||
      (HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V17_1) ||
      ((HcInfo->DeviceId == V_XHCI_DID_CPU_LP_V17_0) && HcInfo->IsCpuStepA0)) {
    Data32Or |= BIT3;
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_D0I2CTRL),
    (UINT32)~(B_XHCI_MEM_D0I2CTRL),
    Data32Or
    );

  //
  //  D0i2SchAlarmCtrl - D0i2 Scheduler Alarm Control Register
  //  Address Offset: 0x81C0
  //  Value: [28:16] 2Dh, [12:0] 0Fh
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_D0I2SCH_ALARM_CTRL),
    (UINT32)~(B_XHCI_MEM_D0I2SCH_ALARM_CTRL),
    (UINT32) ((0x2D << N_XHCI_MEM_D0I2SCH_ALARM_CTRL_D0I2IT) | 0xF)
    );

  //
  //  USB2PMCTRL - USB2 Power Management Control
  //  Address Offset: 0x81C4
  //  Value: [11] 1b, [10:8] 001b
  //  For devices with ID different than 0x98ED
  //    [3:2] 10b, [1:0] 10b
  //
  Data32Or  = (UINT32) (BIT11 | BIT8);
  Data32And = (UINT32)~(BIT10 | BIT9);
  if (HcInfo->DeviceId != V_XHCI_DID_PCH_B_V18_0) {
    Data32Or  |= (BIT3 | BIT1);
    Data32And &= (UINT32)~(BIT2 | BIT0);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_USB2PMCTRL),
    Data32And,
    Data32Or
    );

  //
  //  TRBPRFCTRLREG1 - TRB Prefetch Control Register 1
  //  Address Offset: 0x81D0
  //  Value: [23] 1b, [2] 1b, [0] 0b
  //  For CNP-LP Ax steppings [25] 1b else [25] 0b
  //
  Data32And = (UINT32)~(BIT0);
  Data32Or  = (UINT32) (BIT23 | BIT2);
  if (!((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) && (PchStepping () < PCH_B0))) {
    Data32And &= (UINT32)~(BIT25);
  }
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_TRBPRFCTRLREG1),
    Data32And,
    Data32Or
    );

  //
  //  TRBPRFCACHEINVREG - TRB Prefetch Cache Invalidation Register 1
  //  Address Offset: 0x81D8
  //  Value: [23:17] 7Fh
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_TRBPRFCACHEINVREG),
    (0x7F << N_XHCI_MEM_TRBPRFCACHEINVREG_EN_TRB_FLUSH)
    );

  //
  //  HOST_CTRL_SUS_LINK_PORT_REG
  //  Address Offset: 0x81F8
  //  Value: [8:7] 1h
  //
  MmioAndThenOr32 (
    (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SUS_LINK_PORT_REG),
    (UINT32) ~(BIT8 | BIT7),
    (UINT32) (BIT7)
    );

  if (HcInfo->OnSouth == TRUE) {
    //
    //  HOST_CTRL_EARLY_DBG_REG
    //  Address Offset: 0x81FC
    //  Value: [0]0b [1]1b
    //  For Device with ID 0x9DED and PCH stepping B1 [6] 1b
    //
    Data32Or = (UINT32) (BIT1);
    if ((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) && (PchStepping () == PCH_B1)) {
      Data32Or |= BIT6;
    }
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_EARLY_DBG_REG),
      Data32Or
      );
  }

  if (HcInfo->IpVersion > V16_0) {
    //
    //  MULT_IN_SCH_POLICY - Multiple IN Scheduler Policy Register
    //  Address Offset: 0x82A0
    //  Applies only to IP V17.0 and newer
    //  Value:  [5:4] 1h (default) for 0x81A3, 0x34ED and 0x38ED else 3h
    //
    if ((HcInfo->DeviceId != V_XHCI_DID_CPU_LP_V17_0) &&
        (HcInfo->DeviceId != V_XHCI_DID_PCH_LP_V17_1) &&
        (HcInfo->DeviceId != V_XHCI_DID_PCH_N_V17_1)) {
      MmioOr32 (
        (XhciMmioBase + R_XHCI_MEM_MULT_IN_SCH_POLICY),
        (UINT32) (BIT5 | BIT4)
        );
    }

    //
    //  MULT_IN_FAIRNESS_POLICY_1 - Fairness Policy Register 1
    //  Address Offset: 0x82A4
    //  Applies only to IP V17.0 and newer
    //  Value:  [6:4] 7h only if Device ID is not 0x81A3, 0x34ED and 0x38ED
    //
    if ((HcInfo->DeviceId != V_XHCI_DID_CPU_LP_V17_0) &&
        (HcInfo->DeviceId != V_XHCI_DID_PCH_LP_V17_1) &&
        (HcInfo->DeviceId != V_XHCI_DID_PCH_N_V17_1)) {
      MmioOr32 (
        (XhciMmioBase + R_XHCI_MEM_MULT_IN_FAIRNESS_POLICY_1),
        (UINT32) (BIT6 | BIT5 | BIT4)
        );
    }

    //
    //  PMREQ Control Register
    //  Address Offset: 0x83D0
    //  Value: [15] 1b
    //  Applies only to IP V17.0 and newer
    //  Additionally for device with ID 0x34ED
    //    [12] 1b
    //
    Data32Or = (UINT32) (BIT15);
    if (HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V17_1) {
      Data32Or |= BIT12;
    }
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_PMREQ_CTRL_REG),
      Data32Or
      );

    if ((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V17_1) ||
       ((HcInfo->DeviceId == V_XHCI_DID_CPU_LP_V17_0) && (HcInfo->IsCpuStepA0))) {
      //
      //  Enhanced Clock Gate Control Policy Reguster
      //  Address Offset: 0x83D8
      //  Applies only to devices with IDs 0x8A13 or 0x34ED
      //  Values: [3:2] 11b
      //
      MmioOr32 (
        (XhciMmioBase + R_XHCI_MEM_ENH_CLK_GATE_CTRL),
        (UINT32) (BIT3 | BIT2)
        );
    }
  }

  //
  //  DBCCTL - DBC Control
  //  Address Offset: 0x8760
  //  Values: [6:2] 1Fh [0] 1b
  //
  MmioOr32 (
    (XhciMmioBase + R_XHCI_MEM_DBC_DBCCTL),
    (UINT32) ((0x1F << N_XHCI_MEM_DBC_DBCCTL_DISC_RXD_CNT) | BIT0)
    );

  //
  //  HOST_CTRL_SSP_LINK_REG2
  //  Address Offset: 0x8E68
  //  Value:  [24:23] 3h for CNP Ax else 0h
  //          [4]     1b for CNP Ax else 0b
  //
  if (((HcInfo->DeviceId == V_XHCI_DID_PCH_LP_V16_0) ||
       (HcInfo->DeviceId == V_XHCI_DID_PCH_H_V16_0)) && (PchStepping () < PCH_B0)) {
    MmioOr32 (
      (XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_LINK_REG2),
      (UINT32) (BIT24 | BIT23 | BIT4)
      );
  }

  //
  //  HOST_CTRL_SSP_LFPS_REG2
  //  Address Offset: 0x8E74
  //  Value: [22:18] 3h
  //
  MmioAndThenOr32 (
    XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_LFPS_REG2,
    (UINT32) ~(0x7C0000),
    (UINT32) (BIT19 | BIT18)
    );

  //
  //  HOST_CTRL_SSP_LFPS_REG3
  //  Address Offset: 0x8E78
  //  Value: [4:0] 3h
  //
  MmioAndThenOr32 (
    XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_LFPS_REG3,
    (UINT32) ~(0x1F),
    (UINT32) (BIT1 | BIT0)
    );

  //
  //  HOST_CTRL_SSP_LFPS_REG4
  //  Address Offset: 0x8E7C
  //  This only applies to USB IP v16.0
  //  If USB Compliance Mode disabled
  //    [23:17] = 0x4, [16:14] = 0x6
  //  Else
  //    [23:17] = 0x3C, [16:14] = 0x2
  //
  if (HcInfo->IpVersion == V16_0) {
    Data32And = (UINT32) ~(0xFFC000);
    if (UsbConfig->EnableComplianceMode == FALSE) {
      Data32Or = (UINT32) ((0x4 << 17) | (0x6 << 14));
    } else {
      Data32Or = (UINT32) ((0x3C << 17) | (0x2 << 14));
    }
    MmioAndThenOr32 (
      XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_LFPS_REG4,
      Data32And,
      Data32Or
      );
  }

  //
  //  HOST_CTRL_SSP_CONFIG_REG1
  //  Address Offset: 0x8E80
  //  Value: [29] 1b
  //
  MmioOr32 (
    XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_CONFIG_REG1,
    (UINT32) (BIT29)
    );

  //
  //  HOST_CTRL_USB3_RECAL
  //  Address Offset: 0x8E84
  //  Value:
  //  For IP V16.0  [30:28] = 111b, [19:18] = 10b
  //  else if xHCI on North [31] = 0b otherwise leave default value
  //
  Data32And = ~0u;
  Data32Or  = 0;
  if (HcInfo->IpVersion < V17_0) {
    Data32And = (UINT32)~(BIT18);
    Data32Or  = (UINT32) (BIT30 | BIT29 | BIT28 | BIT19);
  } else {
    if (!HcInfo->OnSouth) {
      Data32And = (UINT32)~(BIT31);
    }
  }
  MmioAndThenOr32 (
    XhciMmioBase + R_XHCI_MEM_HOST_CTRL_USB3_RECAL,
    Data32And,
    Data32Or
    );

  if (HcInfo->IpVersion == V16_0) {
    //
    //  HOST_CTRL_SSP_CONFIG_REG2
    //  Address Offset: 0x8E9C
    //  Value: [27] 1b
    //  This only applies to USB IP v16.0
    //
    MmioOr32 (
      XhciMmioBase + R_XHCI_MEM_HOST_CTRL_SSP_CONFIG_REG2,
      BIT27
      );
  }

  if (HcInfo->IpVersion >= V18_0) {
    //
    //  AUDIO_OFFLOAD_CTR
    //  Address Offset: 0x91F4
    //  This only applies to USB IP V18.0 (PCH only)
    //  Value: [31] 1b, [05:03] 001b
    //
    if (HcInfo->OnSouth) {
      MmioOr32 (
        (XhciMmioBase + R_XHCI_MEM_AUDIO_OFFLOAD_CTR),
        (UINT32) (BIT31 | BIT3)
        );
    }
  }

  //
  //  Set 1 to enable Super Speed Ports terminations on enabled ports only (PDO = 0)
  //  Required for Deep S3
  //
  XhciUsb3PortsMask = (UINT32)((1 << SsPortCount) - 1);
  XhciUsb3Pdo = MmioRead32 (XhciMmioBase + R_PCH_XHCI_MEM_USB3PDO) & XhciUsb3PortsMask;
  Data32 = (~XhciUsb3Pdo) & XhciUsb3PortsMask;

  MmioAndThenOr32 (
    XhciMmioBase + R_XHCI_MEM_SSPE,
    (UINT32)~(XhciUsb3PortsMask),
    Data32
    );

  //
  //  SSIC related programming
  //
  MmioOr32(
    XhciMmioBase + R_XHCI_MEM_SSIC_CONF_REG2_PORT_1,
    (UINT32) (B_XHCI_MEM_SSIC_CONF_REG2_PORT_UNUSED | B_XHCI_MEM_SSIC_CONF_REG2_PROG_DONE)
    );

  MmioOr32(
    XhciMmioBase + R_XHCI_MEM_SSIC_CONF_REG2_PORT_2,
    (UINT32) (B_XHCI_MEM_SSIC_CONF_REG2_PORT_UNUSED | B_XHCI_MEM_SSIC_CONF_REG2_PROG_DONE)
    );
}

/**
  Locks xHCI configuration by setting the proper lock bits in controller

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciBase         xHCI PCI config space address
**/
VOID
XhciLockConfiguration (
  IN  USB_CONFIG            *UsbConfig,
  IN  UINT64                XhciPciBase
  )
{
  //
  // After xHCI is initialized, BIOS should lock the xHCI configuration registers to RO.
  // This prevent any unintended changes. There is also a lockdown feature for OverCurrent
  // registers. BIOS should set these bits to lock down the settings prior to end of POST.
  // 1. Set Access Control bit at XHCI PCI offset 40h[31] to 1b to lock xHCI register settings.
  // 2. Set OC Configuration Done bit at XHCI PCI offset 44h[31] to lock overcurrent mappings from
  //    further changes.
  //
  if (UsbConfig->XhciOcLock) {
    PciSegmentOr32 (XhciPciBase + R_XHCI_CFG_XHCC2, (UINT32) (B_XHCI_CFG_XHCC2_OCCFDONE));
  }

  //
  // XHCI PCI offset 40h is write once register.
  // Unsupported Request Detected bit is write clear
  //
  PciSegmentAndThenOr32 (
    XhciPciBase + R_XHCI_CFG_XHCC1,
    (UINT32) ~(B_XHCI_CFG_XHCC1_URD),
    (UINT32) (B_XHCI_CFG_XHCC1_ACCTRL)
    );
}

/**
  Common entry point for PCH and CPU xHCI controller

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciBase         xHCI PCI config space address
**/
VOID
XhciConfigure (
  IN  USB_CONFIG  *UsbConfig,
  IN  UINT64      XhciPciBase
  )
{
  UINT32                Usb2DisabledPorts;
  UINT32                Usb2PortCount;
  UINT32                Usb3DisabledPorts;
  UINT32                Usb3PortCount;
  UINT32                Index;
  USB_CONTROLLER_INFO   HcInfo;
  UINT32                XhciMmioBase;
  EFI_STATUS            Status;

  if (PciSegmentRead16 (XhciPciBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "xHCI: PCI device NOT found. XhciPciBase 0x%8X\n", XhciPciBase));
    return;
  }

  DEBUG ((DEBUG_INFO, "XhciConfigure() - Start\n"));

  GetXhciControllerInfo (XhciPciBase, &HcInfo);

  XhciMmioBase = PcdGet32 (PcdSiliconInitTempMemBaseAddr);

  //
  // Assign memory resources
  //
  XhciMemorySpaceOpen (
    XhciPciBase,
    XhciMmioBase
    );

  //
  // Disable Compliance Mode
  // It must be done before clock gating is configured.
  //
  if (UsbConfig->EnableComplianceMode == FALSE) {
    MmioOr32 (
      XhciMmioBase + R_XHCI_MEM_HOST_CTRL_PORT_LINK_REG,
      (UINT32) (BIT0)
      );
  }

  //
  // Program Xhci Port Disable Override.
  //
  if (HcInfo.OnSouth && UsbConfig->PdoProgramming) {
    DEBUG ((DEBUG_INFO, "xHCI: PEI phase PDO programming\n"));

    Usb2DisabledPorts = 0;
    Usb2PortCount = GetUsb2PortCount (XhciMmioBase);
    for (Index = 0; Index < Usb2PortCount; Index++) {
      if (UsbConfig->PortUsb20[Index].Enable == FALSE) {
        Usb2DisabledPorts |= (BIT0 << Index);
      }
    }
    Usb3DisabledPorts = 0;
    Usb3PortCount = GetUsb3PortCount (XhciMmioBase);
    for (Index = 0; Index < Usb3PortCount; Index++) {
      if (UsbConfig->PortUsb30[Index].Enable == FALSE) {
        Usb3DisabledPorts |= (BIT0 << Index);
      }
    }

    Status = UsbDisablePorts (
               XhciMmioBase,
               Usb2DisabledPorts,
               Usb3DisabledPorts
               );

    //
    // If PDO register is locked, reset platform to unlock it
    //
    if (EFI_ERROR (Status)) {
      ResetWarm ();
    }
  }

  XhciHcInit (
    &HcInfo,
    XhciPciBase,
    XhciMmioBase,
    UsbConfig
    );

  //
  //  Initiate USB ports reset after Host Controller initialization is done
  //
  if (GetBootModeHob () != BOOT_ON_S3_RESUME) {
    XhciInitiatePortReset (XhciMmioBase);
  }

  //
  // Setup USB Over-Current Mapping.
  //
  XhciOverCurrentMapping (
    &HcInfo,
    UsbConfig,
    XhciPciBase,
    XhciMmioBase
    );

  //
  // Clear memory resources
  //
  XhciMemorySpaceClose (XhciPciBase);

  DEBUG ((DEBUG_INFO, "XhciConfigure() - End\n"));
}

/**
  Programs USB2 AFE register with requested data

  @param[in]  Offset            USB2 AFE register offset
  @param[in]  Data32And         Mask with bits to clear
  @param[in]  Data32Or          Mask with bits to set
**/
STATIC
VOID
Usb2AfeProgramRegister (
  IN  UINT16  Offset,
  IN  UINT32  Data32And,
  IN  UINT32  Data32Or
  )
{
  EFI_STATUS      Status;
  UINT32          Data32;
  UINT32          Expected;
  UINT8           Response;

  //
  // Check for actual data in AFE register and verify accessibility
  //
  Status = PchSbiExecution (
             PID_USB2,
             Offset,
             PrivateControlRead,
             FALSE,
             &Data32,
             &Response
             );
  if (EFI_ERROR (Status)) {
    if ((PchPcrRead32 (PID_DCI, R_DCI_PCR_ECTRL) & B_DCI_PCR_ECTRL_USB2DBCEN) != 0) {
      return;
    }

    ASSERT_EFI_ERROR (Status);
    //
    // Due to error happening data wasn't read properly so cannot rely on it
    //
    return;
  }

  //
  // Data from register was successfully read so change it accordingly
  //
  Expected = (Data32 & Data32And) | Data32Or;
  DEBUG ((DEBUG_INFO, "Programming USB2 AFE register 0x%4X to 0x%8X\n", Offset, Expected));

  Status = PchSbiExecution (
             PID_USB2,
             Offset,
             PrivateControlWrite,
             TRUE,
             &Expected,
             &Response
             );
  ASSERT_EFI_ERROR (Status);

  HsioChipsetInitSusWrite32 (PID_USB2, Offset, Expected);

  DEBUG_CODE_BEGIN ();
  UINT32          Mask;
  //
  // Sanity check for potential data mismatch between written value and expected one
  //
  Status = PchSbiExecution (
             PID_USB2,
             Offset,
             PrivateControlRead,
             FALSE,
             &Data32,
             &Response
             );

  ASSERT_EFI_ERROR (Status);
  // Mask for checking only modified bits
  Mask = ((~Data32And) | Data32Or);
  if (Offset == R_USB2_PCR_GLOBAL_PORT) {
    Mask &= (UINT32)~(BIT24); // UPLLS - Writes of 0b has no effect. Reads always return a 0b.
  }
  ASSERT ((Expected & Mask) == (Data32 & Mask));
  DEBUG_CODE_END ();
}


/**
  Tune the USB 2.0 high-speed signals quality.

  @param[in]  UsbConfig           The USB_CONFIG policy instance
**/
VOID
Usb2AfeProgramming (
  IN  USB_CONFIG   *UsbConfig
  )
{
  UINT32          Data32And;
  UINT32          Data32Or;
  UINT32          Data32;
  UINT8           PortIndex;
  UINT16          PortStatusOffset;
  UINT8           DbcPort;
  UINT8           Retry;
  UINT8           PpLane;
  UINT8           MaxUsbPortNumber;
  UINT16          Offset;

  DbcPort = 0xFF;

  DEBUG ((DEBUG_INFO, "xHCI: Usb2AfeProgramming Start\n"));

  //
  // Disable USB2 PHY Core PG AFE tuning
  //
  PmcUsb2CorePhyPowerGatingDisable ();

  //
  // USB2 Global Port
  // Address Offset: 0x4001
  // Bit [29] 1b, [25] 1b, [24] 1b, [13] 1b, [12] 1b, [11] 1b, [3] 0b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_GLOBAL_PORT,
    (UINT32)~(BIT3),
    (BIT29 | BIT25 | BIT24 | BIT13 | BIT12 | BIT11));

  //
  // USB2 Global Port 2
  // Address Offset: 0x402C
  // Bit [27] 1b, [22] 0b, [9] 1b, [8] 1b, [7] 1b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_GLOBAL_PORT_2,
    (UINT32)~(BIT22),
    (BIT27 | BIT9 | BIT8 | BIT7));

  //
  // USB2 COMPBG
  // Address Offset: 0x7F04
  // Bit [15] 1b, [14:13] 10b, [12:11] 00b, [10:9] 11b, [8:7] 01b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_CFG_COMPBG,
    (UINT32)~(BIT13 | BIT12 | BIT11 | BIT8),
    (BIT15 | BIT14 | BIT10 | BIT9 | BIT7));

  //
  // GLB ADP VBUS COMP REG
  // Address Offset: 0x402B
  // Bit [22] 1b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_GLB_ADP_VBUS_REG,
    (UINT32)~0,
    (BIT22));

  //
  // SFRCONFIG_0
  // Address Offset: 0x702C
  // Bit [9:8] 10b [7:4] 0001b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_SFRCONFIG_0,
    (UINT32)~(BIT8 | BIT7 | BIT6 | BIT5),
    (BIT9 | BIT4));

  //
  // USB2 PLL1
  // Address Offset: 0x7F02
  // Bit [30:29] 10b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_PLL1,
    (UINT32)~(BIT29),
    (BIT30));

  //
  // USB2 PLLDIVRATIOS_0
  // Address Offset: 0x7000
  // Bit [31:24] 01010000b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_PLLDIVRATIOS_0,
    (UINT32)~(BIT31 | BIT29 | BIT27 | BIT26 | BIT25 | BIT24),
    (BIT30 | BIT28));

  //
  // CONFIG_0
  // Address Offset: 0x7008
  // Bit [17:13] 11001b [12:9] 1101b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_CONFIG_0,
    (UINT32)~(BIT15 | BIT14 | BIT10),
    (BIT17 | BIT16 | BIT13 | BIT12 | BIT11 | BIT9));

  //
  // CONFIG_3
  // Address Offset: 0x7014
  // Bit [31:24] 47h [23:16] BBh [15:8] 3Eh [7:1] Fh
  //
  Usb2AfeProgramRegister (R_USB2_PCR_CONFIG_3,
    (UINT32)~(0xFFFFFFFE),
    0x47BB3E1E);

  //
  // DFT_1
  // Address Offset: 0x7024
  // Bit [15:14] 11b [13:12] 11b
  //
  Usb2AfeProgramRegister (R_USB2_PCR_DFT_1,
    (UINT32)~0,
    (BIT15 | BIT14 | BIT13 | BIT12));

  MaxUsbPortNumber = GetPchUsb2MaxPhysicalPortNum ();
  Data32 = PchPcrRead32 (PID_DCI, R_DCI_PCR_ECTRL);

  // Check if DbC is enabled and USB2 DBC is used
  if ((Data32 & (B_DCI_PCR_ECTRL_DBG_EN | B_DCI_PCR_ECTRL_USB2DBCEN)) != 0) {
    // Need to check few times for DbC port allowing bits to settle
    for (Retry = 0; Retry < 3; Retry++) {
      for (PortIndex = 0; PortIndex < MaxUsbPortNumber; PortIndex++) {
        // Check port status in DAP registers
        PortStatusOffset = R_XHCI_PCR_DAP_USB2PORT_STATUS_0 + (PortIndex * S_XHCI_MEM_PORTSC_PORT_SPACING);
        Data32 = PchPcrRead32 (PID_XHCI, PortStatusOffset);

        // Check if operating state says it's DbC port
        if ((Data32 & B_XHCI_PCR_DAP_USB2PORT_STATUS_0_OS) == V_XHCI_PCR_DAP_USB2PORT_STATUS_0_OS_DBC) {
          DEBUG ((DEBUG_INFO, "Port %d is used as a debug port\n", PortIndex));
          break;
        }
      }

      if (PortIndex < MaxUsbPortNumber) { // Found USB2 DbC Port index
        DbcPort = PortIndex + 1;          // Adding 1 to accomodate for next loop lane numbering
        break;
      }

      MicroSecondDelay (10);              // Wait 10us for bits to settle and retry
    }
  }

  // Numbering is starting from 1 to make sure port specific offset
  // is correctly calculated based on layout of registers
  for (PpLane = 1; PpLane <= MaxUsbPortNumber; PpLane++) {
    DEBUG ((DEBUG_INFO, "### PpLane %d\n", PpLane));
    if (PpLane == DbcPort) {              // Don't apply USB2 AFE configuration to port used as DbC connection
      continue;
    }

    //
    // USB2 PER PORT
    // Address Offset: 0x4000 + (N * 0x100) - where N is lane number
    // Bit[14:8] according to recommendation for current board design
    // Bit[31:26] = 111111b, [4] 1b
    //
    Offset = R_USB2_PCR_PP_LANE_BASE_ADDR | V_USB2_PCR_PER_PORT | (PpLane << 8);

    Data32And = (UINT32) ~(BIT14 | BIT13 | BIT12 | BIT11 | BIT10 | BIT9 | BIT8);
    Data32Or  = (((UsbConfig->PortUsb20[PpLane - 1].Afe.Pehalfbit) & 0x1) << 14) |
                (((UsbConfig->PortUsb20[PpLane - 1].Afe.Petxiset) & 0x7)  << 11) |
                (((UsbConfig->PortUsb20[PpLane - 1].Afe.Txiset) & 0x7)    << 8);
    Data32Or |= (BIT31 | BIT30 | BIT29 | BIT28 | BIT27 | BIT26 | BIT4);
    Usb2AfeProgramRegister (Offset, Data32And, Data32Or);

    //
    // USB2 PER PORT 2
    // Address Offset: 0x4026 + (N * 0x100) - where N is lane number
    // [31] 0b, [30] 1b, [25] 1b, [24:23] according to recommendation for current board design, [1:0] 11b
    //
    Offset = R_USB2_PCR_PP_LANE_BASE_ADDR | V_USB2_PCR_PER_PORT_2 | (PpLane << 8);

    Data32And = (UINT32) ~(BIT31 | BIT24 | BIT23);
    Data32Or  = (((UsbConfig->PortUsb20[PpLane - 1].Afe.Predeemp) & 0x3) << 23);
    Data32Or |= (BIT30 | BIT25 | BIT1 | BIT0);
    Usb2AfeProgramRegister (Offset, Data32And, Data32Or);

    //
    // UTMI MISC REG PER PORT
    // Address Offset: 0x4008 + (N * 0x100) - where N is lane number
    // [13:12] 01b, [11] 1b [10:9] 01b, [7] 1b
    //
    Offset = R_USB2_PCR_PP_LANE_BASE_ADDR | V_USB2_PCR_UTMI_MISC_PER_PORT | (PpLane << 8);

    Data32And = (UINT32) ~(BIT13 | BIT10);
    Data32Or  = (BIT12 | BIT11 | BIT9 | BIT7);
    Usb2AfeProgramRegister (Offset, Data32And, Data32Or);
  }
}
