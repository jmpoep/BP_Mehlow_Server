/** @file
  Initializes PCH USB Controllers.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2018 Intel Corporation.

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
#include "PchUsb.h"
#include <Ppi/Spi.h>
#include <Private/Library/PeiHsioLib.h>
#include <Private/Library/UsbInitLib.h>
#include <Library/PostCodeLib.h>
#include <Library/UsbLib.h>
#include <Register/RegsUsb.h>

/**
  Tune the USB 3.0 signals quality.

  @param[in]  UsbConfig           The USB_CONFIG policy instance
  @param[in]  XhciPciBase         XHCI PCI CFG Base Address
**/
STATIC
VOID
XhciUsb3HsioTuning (
  IN  USB_CONFIG      *UsbConfig,
  IN  CONST UINT64    XhciPciBase
  )
{
  HSIO_LANE             HsioLane;
  UINT8                 Index;
  UINT8                 LaneNum;
  UINT32                Data32And;
  UINT32                Data32Or;

  for (Index = 0; Index < GetPchXhciMaxUsb3PortNum (); Index++) {
    if ((UsbConfig->PortUsb30[Index].HsioTxDeEmphEnable == FALSE) &&
        (UsbConfig->PortUsb30[Index].HsioTxDownscaleAmpEnable == FALSE)) {
      continue;
    }

    if (PchFiaGetUsb3LaneNum (Index, &LaneNum)) {
      HsioGetLane (LaneNum, &HsioLane);
      //
      //Step 1: Make changes to any of the TX (Transmit) ModPHY Register Bit Fields
      //
      //
      // USB 3.0 TX Output -3.5dB De-Emphasis Adjustment Setting (ow2tapgen2deemph3p5)
      // HSIO_TX_DWORD5[21:16]
      //
      if (UsbConfig->PortUsb30[Index].HsioTxDeEmphEnable == TRUE) {
        HsioLaneAndThenOr32 (
          &HsioLane,
          R_HSIO_PCR_TX_DWORD5,
          (UINT32) ~B_HSIO_PCR_TX_DWORD5_OW2TAPGEN2DEEMPH3P5_5_0,
          (UINT32) (UsbConfig->PortUsb30[Index].HsioTxDeEmph << N_HSIO_PCR_TX_DWORD5_OW2TAPGEN2DEEMPH3P5_5_0)
          );
        HsioChipsetInitSusWrite32 (
          HsioLane.Pid,
          R_HSIO_PCR_TX_DWORD5,
          HsioLaneRead32 (&HsioLane, R_HSIO_PCR_TX_DWORD5)
          );
      }

      //
      // USB 3.0 TX Output Downscale Amplitude Adjustment (orate01margin)
      // HSIO_TX_DWORD8[21:16]
      //
      if (UsbConfig->PortUsb30[Index].HsioTxDownscaleAmpEnable == TRUE) {
        HsioLaneAndThenOr32 (
          &HsioLane,
          R_HSIO_PCR_TX_DWORD8,
          (UINT32) ~B_HSIO_PCR_TX_DWORD8_ORATE01MARGIN_5_0,
          (UINT32) (UsbConfig->PortUsb30[Index].HsioTxDownscaleAmp << N_HSIO_PCR_TX_DWORD8_ORATE01MARGIN_5_0)
          );
        HsioChipsetInitSusWrite32 (
          HsioLane.Pid,
          R_HSIO_PCR_TX_DWORD8,
          HsioLaneRead32 (&HsioLane, R_HSIO_PCR_TX_DWORD8)
          );
      }

      //
      // Signed Magnatude number added to the CTLE code.(ctle_adapt_offset_cfg_4_0)
      // HSIO_RX_DWORD25 [20:16]
      //
      if (UsbConfig->PortUsb30HsioRx[Index].HsioCtrlAdaptOffsetCfgEnable == TRUE) {
        HsioLaneAndThenOr32 (
          &HsioLane,
          R_HSIO_PCR_RX_DWORD25,
          (UINT32) ~B_HSIO_PCR_RX_DWORD25_CTLE_ADAPT_OFFSET_CFG_4_0,
          (UINT32) (UsbConfig->PortUsb30HsioRx[Index].HsioCtrlAdaptOffsetCfg << N_HSIO_PCR_RX_DWORD25_CTLE_ADAPT_OFFSET_CFG_4_0)
          );
        HsioChipsetInitSusWrite32 (
          HsioLane.Pid,
          R_HSIO_PCR_RX_DWORD25,
          HsioLaneRead32 (&HsioLane, R_HSIO_PCR_RX_DWORD25)
          );
      }

      Data32And = ~0u;
      Data32Or  = 0;

      //
      // LFPS filter select for n (filter_sel_n_2_0)
      // HSIO_RX_DWORD51 [29:27]
      //
      if (UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelNEnable == TRUE) {
        Data32And &= (UINT32)~B_HSIO_PCR_RX_DWORD51_FILTER_SEL_N_2_0;
        Data32Or  |= (UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelN << N_HSIO_PCR_RX_DWORD51_FILTER_SEL_N_2_0);
      }

      //
      // LFPS filter select for p (filter_sel_p_2_0)
      // HSIO_RX_DWORD51 [26:24]
      //
      if (UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelPEnable == TRUE) {
        Data32And &= (UINT32)~B_HSIO_PCR_RX_DWORD51_FILTER_SEL_P_2_0;
        Data32Or  |= (UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelP << N_HSIO_PCR_RX_DWORD51_FILTER_SEL_P_2_0);
      }

      //
      // Controls the input offset (olfpscfgpullupdwnres_sus_usb_2_0)
      // HSIO_RX_DWORD51 [2:0]
      //
      if (UsbConfig->PortUsb30HsioRx[Index].HsioOlfpsCfgPullUpDwnResEnable == TRUE) {
        Data32And &= (UINT32)~B_HSIO_PCR_RX_DWORD51_OLFPSCFGPULLUPDWNRES_SUS_USB_2_0;
        Data32Or  |= UsbConfig->PortUsb30HsioRx[Index].HsioOlfpsCfgPullUpDwnRes;
      }

      if (UsbConfig->PortUsb30HsioRx[Index].HsioOlfpsCfgPullUpDwnResEnable ||
          UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelNEnable ||
          UsbConfig->PortUsb30HsioRx[Index].HsioFilterSelPEnable) {
        HsioLaneAndThenOr32 (
          &HsioLane,
          R_HSIO_PCR_RX_DWORD51,
          Data32And,
          Data32Or
          );
        HsioChipsetInitSusWrite32 (
          HsioLane.Pid,
          R_HSIO_PCR_RX_DWORD51,
          HsioLaneRead32 (&HsioLane, R_HSIO_PCR_RX_DWORD51)
          );
      }

      //
      // Step 2: Clear HSIO_TX_DWORD19 Bit[1] (o_calcinit bit) to 0b
      //
      HsioLaneAndThenOr32 (
        &HsioLane,
        R_HSIO_PCR_TX_DWORD19,
        (UINT32) ~(BIT1),
        (UINT32) (0)
        );
      HsioChipsetInitSusWrite32 (
        HsioLane.Pid,
        R_HSIO_PCR_TX_DWORD19, 
        HsioLaneRead32 (&HsioLane, R_HSIO_PCR_TX_DWORD19)
        );

      //
      // Step 3: Set HSIO_TX_DWORD19 Bit[1] (o_calcinit) to 1b
      //
      HsioLaneAndThenOr32 (
        &HsioLane,
        R_HSIO_PCR_TX_DWORD19,
        (UINT32) ~(0),
        (UINT32) (BIT1)
        );
      HsioChipsetInitSusWrite32 (
        HsioLane.Pid,
        R_HSIO_PCR_TX_DWORD19,
        HsioLaneRead32 (&HsioLane, R_HSIO_PCR_TX_DWORD19)
        );
    }
  }
}

/*
  Performs PCH USB Controllers initialization

  @param[in] SiPolicy     The Silicon Policy PPI instance
*/
VOID
PchUsbConfigure (
  IN  SI_POLICY_PPI               *SiPolicy
  )
{
  EFI_STATUS    Status;
  USB_CONFIG    *UsbConfig;
  UINT64        ControllerPciMmBase;
  UINT8         InterruptPin;
#ifndef CFL_SIMICS
  UINT8         Irq;
  UINT32        Data32Or;
  UINT32        Data32And;
#endif

  Status = GetConfigBlock ((VOID *) SiPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  ControllerPciMmBase = PCI_SEGMENT_LIB_ADDRESS (
                          DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                          DEFAULT_PCI_BUS_NUMBER_PCH,
                          PCI_DEVICE_NUMBER_PCH_XHCI,
                          PCI_FUNCTION_NUMBER_PCH_XHCI,
                          0
                          );

  //
  // Tune the USB 2.0 high-speed signals quality.
  //
  PostCode (0xB04);
  Usb2AfeProgramming (UsbConfig);

  //
  // Configure USB3 ModPHY turning.
  //
  PostCode (0xB05);
  XhciUsb3HsioTuning (UsbConfig, ControllerPciMmBase);

  //
  // Configure PCH xHCI
  //
  PostCode (0xB06);
  XhciConfigure (UsbConfig, ControllerPciMmBase);

  if (PciSegmentRead16 (ControllerPciMmBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
    ///
    /// Configure xHCI interrupt
    ///
    InterruptPin = ItssGetDevIntPin (SiPolicy, PCI_DEVICE_NUMBER_PCH_XHCI, PCI_FUNCTION_NUMBER_PCH_XHCI);

    PciSegmentWrite8 (ControllerPciMmBase + PCI_INT_PIN_OFFSET, InterruptPin);
  }

  //
  // Set all necessary lock bits in xHCI controller
  //
  XhciLockConfiguration (UsbConfig, ControllerPciMmBase);

#ifndef CFL_SIMICS
  //
  // Configure PCH OTG (xDCI)
  //
  PostCode (0xB08);
  ControllerPciMmBase = PCI_SEGMENT_LIB_ADDRESS (
                          DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                          DEFAULT_PCI_BUS_NUMBER_PCH,
                          PCI_DEVICE_NUMBER_PCH_XDCI,
                          PCI_FUNCTION_NUMBER_PCH_XDCI,
                          0
                          );

  XdciConfigure (UsbConfig, ControllerPciMmBase);

  if (PciSegmentRead16 (ControllerPciMmBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
    ///
    /// Configure xDCI interrupt
    ///
    ItssGetDevIntConfig (
      SiPolicy,
      PCI_DEVICE_NUMBER_PCH_XDCI,
      PCI_FUNCTION_NUMBER_PCH_XDCI,
      &InterruptPin,
      &Irq
      );

    //
    // Set Interrupt Pin and IRQ number
    //
    Data32Or = (UINT32) ((InterruptPin << N_OTG_PCR_PCICFGCTRL_INT_PIN) |
                          (Irq << N_OTG_PCR_PCICFGCTRL_PCI_IRQ));
    Data32And =~(UINT32) (B_OTG_PCR_PCICFGCTRL_PCI_IRQ | B_OTG_PCR_PCICFGCTRL_ACPI_IRQ | B_OTG_PCR_PCICFGCTRL_INT_PIN | B_OTG_PCR_PCICFGCTRL_ACPI_INT_EN);

    PchPcrAndThenOr32 (PID_OTG, R_OTG_PCR_PCICFGCTRL1, Data32And, Data32Or);
  }
#endif
}

/*
  Finishes previously started PCH USB initialization

  @param[in] SiPolicy     The Silicon Policy PPI instance
*/
VOID
PchUsbAfterConfigure (
  IN  SI_POLICY_PPI               *SiPolicy
  )
{
  EFI_STATUS    Status;
  USB_CONFIG    *UsbConfig;
  UINT64        XhciPciMmBase;

  Status = GetConfigBlock ((VOID *) SiPolicy, &gUsbConfigGuid, (VOID *) &UsbConfig);
  ASSERT_EFI_ERROR (Status);

  XhciPciMmBase = PCI_SEGMENT_LIB_ADDRESS (
                    DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                    DEFAULT_PCI_BUS_NUMBER_PCH,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );

  XhciConfigureAfterInit (UsbConfig, XhciPciMmBase);

}

