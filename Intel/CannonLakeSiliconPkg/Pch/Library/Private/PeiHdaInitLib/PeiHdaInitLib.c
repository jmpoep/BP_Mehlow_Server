/** @file
  Initializes the PCH HD Audio Controller and Codec.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2019 Intel Corporation.

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
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/TimerLib.h>
#include <Library/PeiServicesLib.h>
#include <IndustryStandard/Pci30.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Library/PciSegmentLib.h>
#include <Ppi/SiPolicy.h>
#include <Library/PostCodeLib.h>
#include <Library/PcdLib.h>
// For SA iDisplay Audio link configuration
#include <SaAccess.h>
#include <Private/Library/GraphicsInitLib.h>
#include <Private/Library/PchPsfPrivateLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <CpuRegs.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/PeiPchHdaLib.h>
#include <Private/Library/PeiItssLib.h>
#include <Register/PchRegs.h>
#include <Library/CnviLib.h>

#define HDA_MAX_LOOP_TIME            200
#define HDA_WAIT_PERIOD              5
#define HDA_HDA_CODEC_WAIT_PERIOD    5
#define HDA_DISP_CODEC_WAIT_PERIOD   50

#define HDA_MAX_SDI_NUMBER           3
#define HDA_MAX_SDI_MASK             ((1 << HDA_MAX_SDI_NUMBER) - 1)
#define HDA_SDI_0_HDALINK            0
#define HDA_SDI_1_HDALINK            1
#define HDA_SDI_2_IDISPLINK          2

/**
  Polling the Status bit.
  Maximum polling time (us) equals HDA_MAX_LOOP_TIME * HDA_WAIT_PERIOD.

  @param[in] StatusReg            The register address to read the status
  @param[in] PollingBitMap        The bit mapping for polling
  @param[in] PollingData          The Data for polling
  @param[in] WaitPeriod           The minimum period between sending next data

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_TIMEOUT             Polling the bit map time out
**/
EFI_STATUS
StatusPolling (
  IN      UINT32          StatusReg,
  IN      UINT32          PollingBitMap,
  IN      UINT32          PollingData,
  IN      UINT32          WaitPeriod
  )
{
  UINT32  LoopTime;

  for (LoopTime = 0; LoopTime < HDA_MAX_LOOP_TIME; LoopTime++) {
    if ((MmioRead32 (StatusReg) & PollingBitMap) == PollingData) {
      break;
    } else {
      MicroSecondDelay (WaitPeriod);
    }
  }

  if (LoopTime >= HDA_MAX_LOOP_TIME) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Checks HD Audio Controller reset state.

  @param[in] HdaBar             Memory Space Base Address

  @return BOOLEAN               TRUE if controller is in reset, FALSE otherwise
**/
BOOLEAN
IsHdaControllerReset (
  IN      UINT32                              HdaBar
  )
{
  ASSERT (HdaBar != 0);
  ///
  /// Read RESET# bit - GCTL.CRSTB (offset 08h, BIT0)
  /// TRUE:  controller in reset state (CRSTB = 0b)
  /// FALSE: controller out of reset state (CRSTB = 1b)
  ///
  return (MmioRead32 ((UINTN) (HdaBar + R_HDA_MEM_GCTL)) & B_HDA_MEM_GCTL_CRST) ? FALSE : TRUE;
}

/**
  Sets HD Audio Controller Reset state.
  In reset all state machines, memory mapped registers and links are driven to reset values.

  @param[in] HdaBar             Memory Space Base Address
  @param[in] ResetDelay         Delay needed by codec to perform self-initialization
  @param[in] AssertResetSignal  TRUE to enter (assert) reset, FALSE to exit (de-assert) reset

  @return EFI_SUCCESS           Controller changed reset state successfully
  @retval EFI_TIMEOUT           Polling RESET# bit time out
**/
EFI_STATUS
SetHdaControllerReset (
  IN      UINT32                              HdaBar,
  IN      UINT32                              ResetDelay,
  IN      BOOLEAN                             AssertResetSignal
  )
{
  UINT32 ResetBitValue;

  ASSERT (HdaBar != 0);

  DEBUG ((DEBUG_INFO, "SetHdaControllerReset() - %a Reset\n", AssertResetSignal ? "Assert" : "De-assert"));

  if (AssertResetSignal) {
    ///
    /// Enter reset state: clear RESET# bit - GCTL.CRSTB (offset 08h, BIT0) = 0b
    ///
    MmioAnd32 ((UINTN) (HdaBar + R_HDA_MEM_GCTL), (UINT32) ~(B_HDA_MEM_GCTL_CRST));
    ResetBitValue = 0;
  } else {
    ///
    /// Exit reset state: set RESET# bit - GCTL.CRSTB (offset 08h, BIT0) = 1b
    ///
    MmioOr32 ((UINTN) (HdaBar + R_HDA_MEM_GCTL), (UINT32) B_HDA_MEM_GCTL_CRST);
    ResetBitValue = 1;
  }

  ///
  /// For some combo card that will need this delay because each codec has different latency to come out from RESET.
  /// This delay can make sure all codecs be recognized by BIOS after RESET sequence.
  /// Additional delay might be required to allow codec coming out of reset prior to subsequent operations,
  /// please contact your codec vendor for detail. When clearing this bit and setting it afterward,
  /// BIOS must ensure that minimum link timing requirements (minimum RESET# assertion time, etc.) are met.
  ///
  MicroSecondDelay (ResetDelay);

  ///
  /// Poll RESET# bit until it reads back as 1 if RESET# de-asserted or 0 if asserted
  ///
  return StatusPolling (HdaBar + R_HDA_MEM_GCTL, B_HDA_MEM_GCTL_CRST, ResetBitValue, HDA_WAIT_PERIOD);
}


/**
  Send the command to the codec via the Immediate Command mechanism is written
  to the IC register

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in, out] CodecCommandData      The Codec Command to be sent to the codec
  @param[in] ReadBack                   Whether to get the response received from the codec
  @param[in] WaitPeriod                 The minimum period between sending next data

  @retval EFI_DEVICE_ERROR              Device status error, operation failed
  @retval EFI_SUCCESS                   The function completed successfully
**/
EFI_STATUS
SendCodecCommand (
  IN      UINT32          HdaBar,
  IN OUT  UINT32          *CodecCommandData,
  IN      BOOLEAN         ReadBack,
  IN      UINT32          WaitPeriod
  )
{
  EFI_STATUS  Status;

  Status = StatusPolling (HdaBar + R_HDA_MEM_ICS, (UINT16) B_HDA_MEM_ICS_ICB, (UINT16) 0, WaitPeriod);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "ICB bit is not zero before SendCodecCommand! \n"));
    return EFI_DEVICE_ERROR;
  }

  MmioWrite32 (HdaBar + R_HDA_MEM_IC, *CodecCommandData);
  MmioOr16 ((UINTN) (HdaBar + R_HDA_MEM_ICS), (UINT16) ((B_HDA_MEM_ICS_IRV | B_HDA_MEM_ICS_ICB)));

  Status = StatusPolling (HdaBar + R_HDA_MEM_ICS, (UINT16) B_HDA_MEM_ICS_ICB, (UINT16) 0, WaitPeriod);

  if (EFI_ERROR (Status)) {
    MmioAnd16 ((UINTN) (HdaBar + R_HDA_MEM_ICS), (UINT16) ~(B_HDA_MEM_ICS_ICB));
    return Status;
  }

  if (ReadBack == TRUE) {
    if ((MmioRead16 (HdaBar + R_HDA_MEM_ICS) & B_HDA_MEM_ICS_IRV) != 0) {
      *CodecCommandData = MmioRead32 (HdaBar + R_HDA_MEM_IR);
    } else {
      DEBUG ((DEBUG_ERROR, "SendCodecCommand: ReadBack fail! \n"));
      return EFI_DEVICE_ERROR;
    }
  }

  return EFI_SUCCESS;
}

/**
  Checks if connected codec supports statically switchable BCLK clock frequency.

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSdiNum                SDI number to which codec is connected
  @param[out] BclkFrequencySupportMask  Bit mask of supported frequencies

  @retval EFI_NO_RESPONSE               Verb response has not been received from codec
  @retval EFI_UNSUPPORTED               Codec does not support frequency switching
  @retval EFI_SUCCESS                   Codec supports frequency switching, the function completed successfully
**/
EFI_STATUS
GetCodecFrequencySwitchingSupport (
  IN      UINT32                             HdaBar,
  IN      UINT8                              CodecSdiNum,
  OUT     UINT32                             *BclkFrequencySupportMask
  )
{
  UINT32          BclkFreqSupport;
  EFI_STATUS      Status;

  *BclkFrequencySupportMask = 0;

  if (CodecSdiNum == HDA_SDI_2_IDISPLINK) {
    // iDisplay Audio codec does not support Get Parameter/Switching Support (F0016h) verb
    return EFI_SUCCESS;
  }

  ///
  /// Read Statically Switchable BCLK Frequency capabilities from the attached codec (VerbId = F00h, ParameterId = 16h)
  ///
  BclkFreqSupport = 0x000F0016 | (CodecSdiNum << 28);
  Status   = SendCodecCommand (HdaBar, &BclkFreqSupport, TRUE, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Reading the Codec BCLK Frequency Switching support fail!\n"));
    return EFI_NO_RESPONSE;

  }
  DEBUG ((DEBUG_INFO, "SDI#%d Verb F00h ParamtererId = 16h response = 0x%08x\n", CodecSdiNum, BclkFreqSupport));

  ///
  /// If the codec returns value of all 0s, it implies that it does not support static frequency switching.
  /// HDAudio-Link (SDIN 0/1) : BIT2 (24MHz) must be always set if supported
  ///
  if (BclkFreqSupport == 0 ||
      ((CodecSdiNum == HDA_SDI_0_HDALINK || CodecSdiNum == HDA_SDI_1_HDALINK) && ((BclkFreqSupport & BIT2) == 0))) {
    DEBUG ((DEBUG_INFO, "HDA: Codec Static Frequency Switching not supported!\n"));
    return EFI_UNSUPPORTED;
  }

  *BclkFrequencySupportMask = (BclkFreqSupport & 0x1F); // 1Fh = BITS[4:0] - 96/48/24/12/6 MHz
  DEBUG ((DEBUG_INFO, "HDA: Codec Static Frequency Switching supported - FrequencySupportMask = 0x%08x\n", *BclkFrequencySupportMask));
  return EFI_SUCCESS;
}

/**
  Get codec's current BCLK clock frequency.

  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSdiNum                SDI number to which codec is connected
  @param[out] CurrentBclkFrequency      Current BCLK frequency of codec

  @retval EFI_NO_RESPONSE               Verb response has not been received from codec
  @retval EFI_NOT_FOUND                 Could not match current codec frequency with any supported frequency
  @retval EFI_SUCCESS                   Supported frequency found, the function completed successfully
**/
EFI_STATUS
GetCodecCurrentBclkFrequency (
  IN      UINT32                              HdaBar,
  IN      UINT8                               CodecSdiNum,
  OUT     PCH_HDAUDIO_LINK_FREQUENCY          *CurrentBclkFrequency
  )
{
  UINT32          BclkFreq;
  UINT32          FreqIndex;
  EFI_STATUS      Status;

  *CurrentBclkFrequency = PchHdaLinkFreqInvalid;

  ///
  /// Read BCLK Freq from the attached codec
  ///
  BclkFreq = 0x000F3700 | (CodecSdiNum << 28);
  Status   = SendCodecCommand (HdaBar, &BclkFreq, TRUE, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Reading the Codec current BCLK Frequency fail!\n"));
    return EFI_NO_RESPONSE;
  }
  DEBUG ((DEBUG_INFO, "SDI#%d Verb F37h (GET_CCF) response = 0x%08x\n", CodecSdiNum, BclkFreq));

  for (FreqIndex = 0, Status = EFI_NOT_FOUND; FreqIndex < PchHdaLinkFreqInvalid; FreqIndex++) {
    if ((BclkFreq >> FreqIndex) & BIT0) {
      *CurrentBclkFrequency = FreqIndex;
      DEBUG ((DEBUG_INFO, "SDI#%d Codec BCLK frequency = %d\n", CodecSdiNum, *CurrentBclkFrequency));
      Status = EFI_SUCCESS;
      break;
    }
  }

  DEBUG ((DEBUG_INFO, "GetCodecCurrentBclkFrequency() Status = %r\n", Status));
  return Status;
}

/**
  Sets iDisplay Audio Codec T-Mode

  @param[in] HdaPciBase                 PCI Configuration Space Base Address
  @param[in] IDispTmode                 Display Codec T-Mode to be set
**/
VOID
SetIDisplayAudioTmode (
  IN      UINT64                              HdaPciBase,
  IN      PCH_HDAUDIO_IDISP_TMODE             RequestedIDispTmode
  )
{
  UINT32      Data32;

  DEBUG ((DEBUG_INFO, "HDA: Set iDisplay Audio T-Mode - "));
  switch (RequestedIDispTmode) {
    case PchHdaIDispMode1T:
      DEBUG ((DEBUG_INFO, "1T\n"));
      // 1T Mode: PCI Config Space + C0h[28:27_12] = 00_1b.
      Data32 = (V_HDA_CFG_SEM1_ETMODE_1T_2T << N_HDA_CFG_SEM1_ETMODE) | B_HDA_CFG_SEM1_TMODE;
      break;
    case PchHdaIDispMode2T:
      DEBUG ((DEBUG_INFO, "2T\n"));
      // 2T Mode: PCI Config Space + C0h[28:27_12] = 00_0b.
      Data32 = (V_HDA_CFG_SEM1_ETMODE_1T_2T << N_HDA_CFG_SEM1_ETMODE);
      break;
    case PchHdaIDispMode4T:
      DEBUG ((DEBUG_INFO, "4T\n"));
      // 4T Mode: PCI Config Space + C0h[28:27_12] = 01_0b.
      Data32 = (V_HDA_CFG_SEM1_ETMODE_4T << N_HDA_CFG_SEM1_ETMODE);
      break;
    case PchHdaIDispMode8T:
      DEBUG ((DEBUG_INFO, "8T\n"));
      // 8T Mode: PCI Config Space + C0h[28:27_12] = 10_0b.
      Data32 = (V_HDA_CFG_SEM1_ETMODE_8T << N_HDA_CFG_SEM1_ETMODE);
      break;
    case PchHdaIDispMode16T:
      DEBUG ((DEBUG_INFO, "16T\n"));
      // 16T Mode: PCI Config Space + C0h[28:27_12] = 11_0b.
      Data32 = (V_HDA_CFG_SEM1_ETMODE_16T << N_HDA_CFG_SEM1_ETMODE);
      break;
    default:
      DEBUG ((DEBUG_ERROR, "Unsupported iDisplay Audio T-Mode - Exit!\n"));
      Data32 = 0;
  }

  PciSegmentAndThenOr32 (HdaPciBase + R_HDA_CFG_SEM1,
    (UINT32) ~(B_HDA_CFG_SEM1_ETMODE | B_HDA_CFG_SEM1_TMODE),
    (UINT32) Data32);
}

/**
  Sets link clock frequency.
  Controller must be out of reset to set Output/Input payload registers.
  Link must be turned off before setting Link Control register, then turned on again.

  @param[in] HdaPciBase                 PCI Configuration Space Base Address
  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] LinkTypeIndex              Link index: 0 = HDA link, 1 = iDisp link
  @param[in] RequestedBclkFrequency     Clock frequency to be set
  @param[in] RequestedIDispTmode        Display Codec T-Mode to be set

  @retval EFI_NOT_READY                 Link reset cannot be asserted
  @retval EFI_DEVICE_ERROR              iDisplay link frequency switching failed
  @retval EFI_INVALID_PARAMETER         Incorrect link index given
  @retval EFI_SUCCESS                   Requested frequency set, the function completed successfully
**/
EFI_STATUS
SetCodecBclkFrequency (
  IN      UINT64                              HdaPciBase,
  IN      UINT32                              HdaBar,
  IN      UINT8                               LinkTypeIndex,
  IN      PCH_HDAUDIO_LINK_FREQUENCY          RequestedBclkFrequency,
  IN      PCH_HDAUDIO_IDISP_TMODE             RequestedIDispTmode
  )
{
  UINT16      OutputPayloadWords;
  UINT16      InputPayloadWords;
  UINT32      Data32;
  EFI_STATUS  Status;

  // Assert link RESET# before frenquency switching
  MmioAnd32 ((UINTN) (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex)), (UINT32) ~(B_HDA_MEM_LCTLX_SPA));
  Status = StatusPolling (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex), (UINT32) B_HDA_MEM_LCTLX_CPA, (UINT32) 0, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HDA: Turn off the link (SPA = 0) failed! CPA polling Status = %r\n", Status));
    return EFI_NOT_READY;
  }

  if (LinkTypeIndex == HDA_HDALINK_INDEX) {
    //
    // HDAudio-Link specific steps
    //
    DEBUG ((DEBUG_INFO, "HDA: HDA-Link Static BCLK Frequency Switch ID = %d - ", RequestedBclkFrequency));

    // For HDA-Link change OUTPAY and INPAY
    switch (RequestedBclkFrequency) {
      case PchHdaLinkFreq24MHz:
        DEBUG ((DEBUG_INFO, "24MHz\n"));
        // Output: 24MHz -> 1000 bits/frame - 40 bits (command & control) = 960 bits -> 60 words of payload
        // Input : 24MHz -> 500 bits/frame - 36 bits (response) = 464 bits -> 29 words of payload (limit to 28)
        OutputPayloadWords = 0x3C; // 60 words of payload
        InputPayloadWords  = V_HDA_MEM_INPAY_DEFAULT; // 28 words of payload
        break;
      case PchHdaLinkFreq12MHz:
        DEBUG ((DEBUG_INFO, "12MHz\n"));
        // Output: 12MHz -> 500 bits/frame - 40 bits (command & control) = 460 bits -> 28 words of payload
        // Input : 12MHz -> 250 bits/frame - 36 bits (response) = 214 bits -> 13 words of payload
        OutputPayloadWords = 0x1C; // 28 words of payload
        InputPayloadWords  = 0x0D; // 13 words of payload
        break;
      case PchHdaLinkFreq6MHz:
        DEBUG ((DEBUG_INFO, "6MHz\n"));
        // Output: 6MHz -> 250 bits/frame - 40 bits (command & control) = 210 bits -> 13 words of payload
        // Input : 6MHz -> 125 bits/frame - 36 bits (response) = 89 bits -> 5 words of payload
        OutputPayloadWords = 0x0D; // 13 words of payload
        InputPayloadWords  = 0x05; // 5 words of payload
        break;
      default:
        DEBUG ((DEBUG_INFO, "Default 24MHz\n"));
        // Default for 24MHz
        OutputPayloadWords = 0x3C; // 60 words of payload
        InputPayloadWords  = V_HDA_MEM_INPAY_DEFAULT; // 28 words of payload
        break;
    }

    // Controller must be out of reset (CRSTB = 1) to set the following registers
    MmioWrite16 ((UINTN) (HdaBar + R_HDA_MEM_OUTPAY), OutputPayloadWords);
    MmioWrite16 ((UINTN) (HdaBar + R_HDA_MEM_INPAY), InputPayloadWords);

    Data32 = MmioRead16 ((UINTN) (HdaBar + R_HDA_MEM_OUTPAY));
    DEBUG ((DEBUG_INFO, "HDA: OUTPAY (Output Payload Capability) = 0x%04x\n", Data32));
    Data32 = MmioRead16 ((UINTN) (HdaBar + R_HDA_MEM_INPAY));
    DEBUG ((DEBUG_INFO, "HDA: INPAY (Input Payload Capability) = 0x%04x\n", Data32));

  } else if (LinkTypeIndex == HDA_IDISPLINK_INDEX) {
    //
    // iDisplayAudio-Link specific steps
    //
    DEBUG ((DEBUG_INFO, "HDA: iDisplay-Link Static BCLK Frequency Switch ID = %d - ", RequestedBclkFrequency));
    DEBUG ((DEBUG_INFO, (RequestedBclkFrequency == PchHdaLinkFreq96MHz) ? "96MHz\n" : "48MHz\n"));

    if (IsAudioIDispTmodeSupported (RequestedIDispTmode)) {
      SetIDisplayAudioTmode (HdaPciBase, RequestedIDispTmode);
    }

    // Configure iDisplay link frequency on System Agent counterpart (iGFX)
    Status = ConfigureIDispAudioFrequency (RequestedBclkFrequency, RequestedIDispTmode);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "HDA SA: iDisplay-Link frequency switching failed! Status = %r\n", Status));
      return EFI_DEVICE_ERROR;
    }

  } else {
    DEBUG ((DEBUG_ERROR, "HDA: Incorrect Link Type!\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  // HDA and iDisp Link common steps
  //
  MmioAndThenOr32 ((UINTN) (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex)),
    (UINT32) ~(BIT3 | BIT2 | BIT1 | BIT0),
    (UINT32) (RequestedBclkFrequency << N_HDA_MEM_LCTLX_SCF));

  Data32 = MmioRead32 ((UINTN) (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex)));
  DEBUG ((DEBUG_INFO, "HDA: LCTLx (Link %d Control) = 0x%08x\n", LinkTypeIndex, Data32));

  // De-assert link RESET# after frenquency switching
  MmioOr32 ((UINTN) (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex)), (UINT32) B_HDA_MEM_LCTLX_SPA);
  Status = StatusPolling (HdaBar + R_HDA_MEM_LCTLX (LinkTypeIndex), (UINT32) B_HDA_MEM_LCTLX_CPA, (UINT32) B_HDA_MEM_LCTLX_CPA, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HDA: Turn on the link (SPA = 1) failed! CPA polling Status = %r\n", Status));
  }

  return EFI_SUCCESS;
}

/**
  Configures static link frequency for HD-Audio link and iDisplay link.

  @param[in] HdaConfig                  HD-A Configuration
  @param[in] HdaPciBase                 PCI Configuration Space Base Address
  @param[in] HdaBar                     Base address of Intel HD Audio memory mapped configuration registers
  @param[in] CodecSdiNum                SDI number to which codec is connected

  @retval EFI_NOT_STARTED               iDisplay link frequency switch not supported (iGfx disabled)
  @retval EFI_INVALID_PARAMETER         Incorrect SDI number given
  @retval EFI_UNSUPPORTED               Codec does not support frequency switching
  @retval EFI_NOT_FOUND                 Codec current frequency does not match with any supported frequency
  @retval EFI_DEVICE_ERROR              Codec supports requested frequency, but the switching failed (default frequency set)
  @retval EFI_SUCCESS                   Requested frequency successfully set (or correct frequency is already configured)
**/
EFI_STATUS
ConfigureLinkFrequency (
  IN      PCH_HDAUDIO_CONFIG                  *HdaConfig,
  IN      UINT64                              HdaPciBase,
  IN      UINT32                              HdaBar,
  IN      UINT8                               CodecSdiNum
  )
{
  PCH_HDAUDIO_LINK_FREQUENCY RequestedFreq;
  PCH_HDAUDIO_LINK_FREQUENCY CurrentFreq;
  UINT32                     FreqSupportMask;
  UINT8                      LinkTypeIndex;
  EFI_STATUS                 Status;

  DEBUG ((DEBUG_INFO, "ConfigureLinkFrequency() Start [SDI:%d]\n", CodecSdiNum));

  switch (CodecSdiNum) {
    case HDA_SDI_0_HDALINK:
    case HDA_SDI_1_HDALINK:
      // HDAudio link frequency change
      DEBUG ((DEBUG_INFO, "HDA: HDAudio-Link frequency configuration\n"));
      LinkTypeIndex = HDA_HDALINK_INDEX;
      RequestedFreq = HdaConfig->HdAudioLinkFrequency;

      //
      // Program INPAY register with value 1Ch (controller must be out of reset - CRSTB = 1 and before FNCFG.BCLD = 1)
      //
      MmioWrite16 ((UINTN) (HdaBar + R_HDA_MEM_INPAY), V_HDA_MEM_INPAY_DEFAULT);

      Status = GetCodecFrequencySwitchingSupport (HdaBar, CodecSdiNum, &FreqSupportMask);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      Status = GetCodecCurrentBclkFrequency (HdaBar, CodecSdiNum, &CurrentFreq);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (CurrentFreq == RequestedFreq) {
        DEBUG ((DEBUG_INFO, "HDA: Requested BCLK clock frequency matches current frequency (ID = %d) - Exit\n", CurrentFreq));
        return EFI_SUCCESS;
      }

      if ((FreqSupportMask >> RequestedFreq) & BIT0) {
        DEBUG ((DEBUG_INFO, "HDA: Requested Frequency ID = %d - Supported!\n", RequestedFreq));
        Status = SetCodecBclkFrequency (HdaPciBase, HdaBar, LinkTypeIndex, RequestedFreq, PchHdaIDispTModeInvalid);
        if (EFI_ERROR (Status)) {
          return EFI_DEVICE_ERROR;
        }

        // Delay needed after switching frequency (link reset), before sending verb to codec.
        MicroSecondDelay (HdaConfig->ResetWaitTimer);

        Status = GetCodecCurrentBclkFrequency (HdaBar, CodecSdiNum, &CurrentFreq);
        if ((Status == EFI_NOT_FOUND) || (CurrentFreq != RequestedFreq)) {
          DEBUG ((DEBUG_ERROR, "HDA: Frequency switching failed - return to default!\n"));
          SetCodecBclkFrequency (HdaPciBase, HdaBar, LinkTypeIndex, PchHdaLinkFreq24MHz, PchHdaIDispTModeInvalid);
          return EFI_DEVICE_ERROR;
        }
      }
      break;
    case HDA_SDI_2_IDISPLINK:
      // iDisplay link frequency change
      if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, PCI_VENDOR_ID_OFFSET)) == 0xFFFF) {
        DEBUG ((DEBUG_INFO, "HDA: iGFX not enabled - frequency switch for iDisplay link not supported!\n"));
        return EFI_NOT_STARTED;
      }

      LinkTypeIndex = HDA_IDISPLINK_INDEX;
      RequestedFreq = HdaConfig->IDispLinkFrequency;

      if (RequestedFreq == PchHdaLinkFreq96MHz) {
        DEBUG ((DEBUG_INFO, "HDA: iDisplay-Link default frequency selected (96MHz) - skip switching routine\n"));
        break;
      }

      DEBUG ((DEBUG_INFO, "HDA: iDisplay-Link frequency configuration\n"));
      Status = SetCodecBclkFrequency (HdaPciBase, HdaBar, LinkTypeIndex, RequestedFreq, HdaConfig->IDispLinkTmode);
      break;
    default:
      DEBUG ((DEBUG_ERROR, "HDA: Invalid SDI number for frequency configuration!\n"));
      return EFI_INVALID_PARAMETER;
  }

  // Delay required after switching frequency (link reset), before sending verb to codec.
  MicroSecondDelay (HdaConfig->ResetWaitTimer);

  DEBUG ((DEBUG_INFO, "ConfigureLinkFrequency() End\n"));
  return EFI_SUCCESS;
}

/**
  Locates HD Audio codec verb table.

  @param[in] HdaConfig        HD-A Configuration
  @param[in] VendorDeviceId   Codec vendor ID  and device ID
  @param[in] RevisionId       Codec revision ID
  @param[in] SdiNum           SDI channel number

  @return HDAUDIO_VERB_TABLE  pointer
  @retval NULL                Table not found.
**/
HDAUDIO_VERB_TABLE*
LocateVerbTable (
  IN      PCH_HDAUDIO_CONFIG      *HdaConfig,
  IN      UINT32                  VendorDeviceId,
  IN      UINT8                   RevisionId,
  IN      UINT8                   SdiNum
  )
{
  HDAUDIO_VERB_TABLE         *VerbTable;
  UINTN                      Num;

  ASSERT (VendorDeviceId != 0);

  VerbTable = NULL;

  for (Num = 0; Num < HdaConfig->VerbTableEntryNum; ++Num) {
    VerbTable = ((HDAUDIO_VERB_TABLE **) (HdaConfig->VerbTablePtr)) [Num];
    if ((VerbTable != NULL) &&
        ((((UINT32) (VerbTable->Header.VendorId) << 16) | VerbTable->Header.DeviceId) == VendorDeviceId) &&
        ((VerbTable->Header.RevisionId == 0xFF) || (VerbTable->Header.RevisionId == RevisionId)) &&
        ((VerbTable->Header.SdiNum == 0xFF) || VerbTable->Header.SdiNum == SdiNum)) {
      break;
    }
  }
  if (Num >= HdaConfig->VerbTableEntryNum) {
    VerbTable = NULL;
  }
  return VerbTable;
}


/**
  Detect active HDA codec links
    BIT2(100) -- SDI2
    BIT1(010) -- SDI1
    BIT0(001) -- SDI0

  @param[in]   HdaBar               Memory Space Base Address

  @retval      UINT8                Azalia SDI active codec mask
**/
UINT8
DetectCodecs (
  IN UINT32       HdaBar
  )
{
  UINT32     LoopTime;
  UINT8      AzaliaSdiMask;
  UINT8      TmpPollingReg;

  AzaliaSdiMask = 0;

  for (LoopTime = 0; LoopTime < HDA_MAX_LOOP_TIME; LoopTime++) {
    TmpPollingReg = (UINT8) (MmioRead8 (HdaBar + R_HDA_MEM_WAKESTS) & HDA_MAX_SDI_MASK);
    if (TmpPollingReg != 0 && (TmpPollingReg == AzaliaSdiMask)) {
      break;
    } else {
      AzaliaSdiMask = TmpPollingReg;
    }
    MicroSecondDelay (HDA_WAIT_PERIOD);
  }
  return AzaliaSdiMask;
}

/**
  For each codec, a predefined codec verb table should be programmed.

  @param[in]   HdaBar               Memory Space Base Address
  @param[in]   AzaliaSdiNum         Azalia SDI Line Number
  @param[out]  CodecVendorId        Codec Vendor Id
  @param[out]  CodecRevisionId      Codec Revision Id

  @retval EFI_SUCCESS               The function completed successfully
  @retval EFI_DEVICE_ERROR          Device status error, operation failed
**/
EFI_STATUS
GetCodecId (
  IN  UINT32     HdaBar,
  IN  UINT8      AzaliaSdiNum,
  OUT UINT32*    CodecVendorId,
  OUT UINT32*    CodecRevisionId
  )
{
  EFI_STATUS  Status;
  ///
  /// Verb:  31~28   27  26~20                   19~0
  ///         CAd    1    NID   Verb Command and data
  ///       0/1/2
  ///
  /// Read the Vendor ID/Device ID pair from the attached codec (ParameterId = 0x00)
  ///
  *CodecVendorId = 0x000F0000 | (AzaliaSdiNum << 28);
  Status = SendCodecCommand (HdaBar, CodecVendorId, TRUE, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SDI#%d Error: Reading the Codec Vendor ID/Device ID fail!\n", AzaliaSdiNum));
    return Status;
  }
  ///
  /// Read the Revision ID from the attached codec (ParameterId = 0x02)
  ///
  *CodecRevisionId = 0x000F0002 | (AzaliaSdiNum << 28);
  Status = SendCodecCommand (HdaBar, CodecRevisionId, TRUE, HDA_WAIT_PERIOD);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "SDI#%d Error: Reading the Codec Revision ID fail!\n", AzaliaSdiNum));
    return Status;
  }

  *CodecRevisionId = (*CodecRevisionId >> 8) & 0xFF;

  return EFI_SUCCESS;
}

/**
  For each codec, a predefined codec verb table should be programmed.

  @param[in]  HdaBar               Memory Space Base Address
  @param[in]  HdaConfig            HD-A Configuration
  @param[in]  AzaliaSdiNum         Azalia SDI Line Number
  @param[in]  WaitPeriod           The minimum period between sending next data
  @param[in]  CodecVendorId        Codec Vendor Id
  @param[in]  CodecRevisionId      Codec Revision Id

  @retval EFI_SUCCESS              The function completed successfully
  @retval EFI_UNSUPPORTED          Verb table for codec does not exist
**/
EFI_STATUS
MatchAndSendCodecVerbTable (
  IN  UINT32                 HdaBar,
  IN  PCH_HDAUDIO_CONFIG     *HdaConfig,
  IN  UINT8                  AzaliaSdiNum,
  IN  UINT32                 WaitPeriod,
  OUT UINT32                 CodecVendorId,
  OUT UINT32                 CodecRevisionId
  )
{
  EFI_STATUS                 Status;
  HDAUDIO_VERB_TABLE         *VerbTable;
  UINT32                     CodecCmdData;
  UINT32                     Index;

  ///
  /// Locate Verb Table and initialize detected codec
  ///
  VerbTable = LocateVerbTable (HdaConfig, CodecVendorId, (UINT8) CodecRevisionId, AzaliaSdiNum);

  if (VerbTable == NULL) {
    DEBUG ((DEBUG_ERROR, "SDI#%d Error: No matching HD-Audio codec verb table found for codec (0x%08X).\n", AzaliaSdiNum, CodecVendorId));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((DEBUG_INFO,
          "Found verb table for vendor 0x%04X devId 0x%04X "
          "rev 0x%02X (SDI:%X, size: %d dwords)\n",
          VerbTable->Header.VendorId,
          VerbTable->Header.DeviceId,
          VerbTable->Header.RevisionId,
          VerbTable->Header.SdiNum,
          VerbTable->Header.DataDwords));

  ///
  /// Send the entire list of verbs in the matching verb table one by one to the codec
  ///
  DEBUG ((DEBUG_VERBOSE, "HDA: Sending verbs to codec:\n"));
  for (Index = 0; Index < VerbTable->Header.DataDwords; ++Index) {
    CodecCmdData  = VerbTable->Data[Index];
    ASSERT ((CodecCmdData >> 28) == 0);
    ///
    /// Program CAd Field per the SDI number got during codec detection
    ///
    CodecCmdData  |= (UINT32) (AzaliaSdiNum << 28);
    Status = SendCodecCommand (HdaBar, &CodecCmdData, FALSE, WaitPeriod);
    if (EFI_ERROR (Status)) {
      ///
      /// Skip the Azalia verb table loading when find the verb table content is not
      /// properly matched with the HDA hardware, though IDs match.
      ///
      DEBUG ((DEBUG_ERROR,"Error loading verb table for Azalia Codec of 0x%08X\n", CodecVendorId));
      break;
    }
    DEBUG ((DEBUG_VERBOSE, "[%03d] 0x%08X\n", Index, CodecCmdData));
  }
  return EFI_SUCCESS;
}

/**
  For each codec, a predefined codec verb table should be programmed.
  The list contains 32-bit verbs to be sent to the corresponding codec.
  If it is not programmed, the codec uses the default verb table, which may or may not
  correspond to the platform jack information.

  @param[in] HdaConfig            HD-A Configuration
  @param[in] HdaPciBase           PCI Configuration Space Base Address
  @param[in] HdaBar               Memory Space Base Address
  @param[in] LinkType             Type of link HDA or iDisp

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_NOT_FOUND           HDA Link codec has not been discovered
**/
EFI_STATUS
DetectAndInitializeCodec (
  IN      PCH_HDAUDIO_CONFIG                  *HdaConfig,
  IN      UINT64                              HdaPciBase,
  IN      UINT32                              HdaBar,
  IN      UINT8                               LinkType
  )
{
  EFI_STATUS                                  Status;
  UINT8                                       AzaliaSdiMask;
  UINT8                                       AzaliaSdiNum;
  UINT32                                      CodecVendorId;
  UINT32                                      CodecRevisionId;
  UINT32                                      WaitPeriod;

  PostCode (0xB0F);
  WaitPeriod = HDA_HDA_CODEC_WAIT_PERIOD;
  DEBUG ((DEBUG_INFO, "DetectAndInitializeCodec() Start\n"));

  ///
  /// Detect active HDA codec links
  ///
  AzaliaSdiMask = DetectCodecs (HdaBar);

  ///
  /// Detecting if any codec is connected
  ///
  if (AzaliaSdiMask == 0) {
    ///
    /// HDA codec not detected on any link
    ///
    PostCode (0xB10);
    DEBUG ((DEBUG_ERROR, "HD-Audio Codec not detected (SDIN not connected to a codec)\n"));
    Status = EFI_NOT_FOUND;
    goto ExitInitCodec;
  }

  ///
  /// PME Enable for Audio controller, this bit is in the resume well
  ///
  if (HdaConfig->Pme == TRUE) {
    PciSegmentOr32 (HdaPciBase + R_HDA_CFG_PCS, (UINT32) B_HDA_CFG_PCS_PMEE);
  }

  for (AzaliaSdiNum = 0; AzaliaSdiNum < HDA_MAX_SDI_NUMBER; AzaliaSdiNum++) {
    switch (AzaliaSdiNum) {
      case HDA_SDI_0_HDALINK:
      case HDA_SDI_1_HDALINK:
        WaitPeriod = HDA_HDA_CODEC_WAIT_PERIOD;
        if (LinkType == HDA_IDISPLINK_INDEX) {
          DEBUG ((DEBUG_INFO, "SDI#%d: HDA codec initialization during iDisp codec initialization - abort \n", AzaliaSdiNum));
          continue;
        }
        break;
      case HDA_SDI_2_IDISPLINK:
        WaitPeriod = HDA_DISP_CODEC_WAIT_PERIOD;
        if (LinkType == HDA_HDALINK_INDEX) {
          DEBUG ((DEBUG_INFO, "SDI#%d: iDisp codec initialization during HDA codec initialization - abort \n", AzaliaSdiNum));
          continue;
        }
        break;
      default:
        DEBUG ((DEBUG_ERROR, "HDA: Invalid SDI number!\n"));
        break;
    }

    if ((AzaliaSdiMask >> AzaliaSdiNum) & 0x1) {

      ///
      /// SDIx has HD-Audio device
      ///
      DEBUG ((DEBUG_ERROR, "SDI#%d has HD-Audio device.\n", AzaliaSdiNum));

      ///
      /// PME Enable for each existing codec, these bits are in the resume well
      ///
      if (HdaConfig->Pme != FALSE) {
        MmioOr16 ((UINTN) (HdaBar + R_HDA_MEM_WAKEEN), (UINT16) ((B_HDA_MEM_WAKEEN_SDI_0 << AzaliaSdiNum)));
      }

      Status = GetCodecId (HdaBar, AzaliaSdiNum, &CodecVendorId, &CodecRevisionId);
      if (EFI_ERROR(Status)) {
        continue;
      }
      DEBUG ((DEBUG_INFO, "SDI#%d: Detected HD-Audio Codec 0x%08X rev 0x%02X\n", AzaliaSdiNum, CodecVendorId, CodecRevisionId));

      ///
      /// Link static frequency switching
      ///
      Status = ConfigureLinkFrequency (HdaConfig, HdaPciBase, HdaBar, AzaliaSdiNum);
      DEBUG ((DEBUG_INFO, "ConfigureLinkFrequency() Exit, Status = %r\n", Status));

      ///
      /// Send Verb Table if required table exist based on Codec Vendor ID and Codec Revision ID
      ///
      Status = MatchAndSendCodecVerbTable(HdaBar, HdaConfig, AzaliaSdiNum, WaitPeriod, CodecVendorId, CodecRevisionId);
      if (EFI_ERROR(Status)) {
        continue;
      }
    } else {
      ///
      /// SDIx has no HD-Audio device
      ///
      DEBUG ((DEBUG_ERROR, "SDI#%d has no HD-Audio device.\n", AzaliaSdiNum));
    }
  }

  Status = EFI_SUCCESS;

ExitInitCodec:
  DEBUG ((DEBUG_INFO, "DetectAndInitializeCodec() End, Status = %r\n", Status));
  return Status;
}

/**
  Initialize the Intel High Definition Audio Codec(s) present in the system.
  For each codec, a predefined codec verb table should be programmed.
  The list contains 32-bit verbs to be sent to the corresponding codec.
  If it is not programmed, the codec uses the default verb table, which may or may not
  correspond to the platform jack information.

  @param[in] SiPolicy             Policy
  @param[in] HdaPciBase           PCI Configuration Space Base Address
  @param[in] HdaBar               Memory Space Base Address


  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_ABORTED             Controller in reset - unable to proceed with initialization
  @retval EFI_INVALID_PARAMETER   Provided VerbTableData is null
**/
EFI_STATUS
ConfigureHdAudioCodec (
  IN  SI_POLICY_PPI             *SiPolicy,
  IN  UINT64                    HdaPciBase,
  IN  UINT32                    HdaBar
  )
{
  EFI_STATUS                    Status;
  PCH_HDAUDIO_CONFIG            *HdaConfig;

  DEBUG ((DEBUG_INFO, "ConfigureHdAudioCodec() Start\n"));

  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDALBA, HdaBar);
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDAUBA, 0);
  PciSegmentOr16 (HdaPciBase + PCI_COMMAND_OFFSET, (UINT16) EFI_PCI_COMMAND_MEMORY_SPACE);

  DEBUG ((DEBUG_INFO, "HdaPciBase = 0x%X, HdaBar = 0x%X\n", HdaPciBase, HdaBar));

  Status = GetConfigBlock ((VOID *) SiPolicy, &gHdAudioConfigGuid, (VOID *) &HdaConfig);
  ASSERT_EFI_ERROR (Status);

  if (HdaConfig->IDispCodecDisconnect) {
    ///
    /// Disconnect SDI2 signal to prevent iDisplay Audio codec enumeration
    /// Set PCR[HDA] + 534h [2] = 1b.
    ///
    DEBUG ((DEBUG_INFO, "HDA: IDisplay Audio Codec disabled - SDI2 disconnected\n"));
    PchPcrAndThenOr32 (PID_DSP, R_HDA_PCR_CDCCFG, (UINT32)~0, B_HDA_PCR_CDCCFG_DIS_SDIN2);
  } else {
    InitializeDisplayAudio ();
  }

  DetectAndInitializeCodec (HdaConfig, HdaPciBase, HdaBar, HDA_IDISPLINK_INDEX);

  if (IsHdaControllerReset (HdaBar)) {
    DEBUG ((DEBUG_ERROR, "HDA: Controller in reset state - initialization aborted!\n"));
    Status = EFI_ABORTED;
  }

  ///
  /// Clear HDBAR and disable memory map access
  ///
  PciSegmentAnd16 (HdaPciBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDALBA, 0);
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDAUBA, 0);

  DEBUG ((DEBUG_INFO, "ConfigureHdAudioCodec() End, Status = %r\n", Status));
  return Status;
}

/**
  Initialize HDA Codec and IDisp Codec

  HD Audio Codec initialization is dependent on CD clock (Display clock)
  and due to that CD clock has to be initialized before audio codec is initialized.

  @param[in] SiPolicy             Policy

**/
VOID
HdAudioInitOnEndOfPei (
  IN SI_POLICY_PPI              *SiPolicy
  )
{
  UINT64                        HdaPciBase;
  UINT32                        HdaBar;

  HdaPciBase = PCI_SEGMENT_LIB_ADDRESS (
                 DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                 DEFAULT_PCI_BUS_NUMBER_PCH,
                 PCI_DEVICE_NUMBER_PCH_HDA,
                 PCI_FUNCTION_NUMBER_PCH_HDA,
                 0
                 );

  if (IsHdAudioEnabled ()) {
    HdaBar = PcdGet32 (PcdSiliconInitTempMemBaseAddr);
    ConfigureHdAudioCodec (SiPolicy, HdaPciBase, HdaBar);
  }
}

/**
  Configures Audio Link Mode

  @param[in] HdaConfig            HD-A Configuration

**/
VOID
ConfigureAudioLinks (
  IN PCH_HDAUDIO_CONFIG   *HdaConfig
  )
{
  EFI_STATUS Status;
  UINT32     PmcSndwRcompConfig;
  BOOLEAN    IsSspEnabled;

  DEBUG ((DEBUG_INFO, "ConfigureAudioLinks() Start\n"));

  ASSERT ( !(HdaConfig->AudioLinkHda   == TRUE && HdaConfig->AudioLinkSsp0  == TRUE) );
  ASSERT ( !(HdaConfig->AudioLinkDmic0 == TRUE && HdaConfig->AudioLinkSndw4 == TRUE) );
  ASSERT ( !(HdaConfig->AudioLinkDmic1 == TRUE && HdaConfig->AudioLinkSndw3 == TRUE) );
  ASSERT ( !(HdaConfig->AudioLinkHda   == TRUE && HdaConfig->AudioLinkSndw1 == TRUE) );
  ASSERT ( !(HdaConfig->AudioLinkSsp1  == TRUE && HdaConfig->AudioLinkSndw2 == TRUE) );

  PmcSndwRcompConfig = 0;
  IsSspEnabled = FALSE;

  if (HdaConfig->AudioLinkHda && IsAudioInterfaceSupported (PchHdaLink)) {
    Status = GpioEnableHdaLink ();
    DEBUG ((DEBUG_INFO, "HDA: HD Audio Link pins configuration, Status = %r\n", Status));
  }
  if (HdaConfig->AudioLinkDmic0 && IsAudioInterfaceSupported (PchHdaDmic0)) {
    Status = GpioEnableHdaDmic (0);
    DEBUG ((DEBUG_INFO, "HDA: DMIC#0 pins configuration, Status = %r\n", Status));
  }

  if (HdaConfig->AudioLinkDmic1 && IsAudioInterfaceSupported (PchHdaDmic1)) {
    Status = GpioEnableHdaDmic (1);
    DEBUG ((DEBUG_INFO, "HDA: DMIC#1 pins configuration, Status = %r\n", Status));
  }

  if (HdaConfig->AudioLinkSsp0 && IsAudioInterfaceSupported (PchHdaSsp0)) {
    Status = GpioEnableHdaSsp (0);
    IsSspEnabled = TRUE;
    DEBUG ((DEBUG_INFO, "HDA: SSP#0 interface pins configuration, Status = %r\n", Status));
  }
  if (HdaConfig->AudioLinkSsp1 && IsAudioInterfaceSupported (PchHdaSsp1)) {
    Status = GpioEnableHdaSsp (1);
    IsSspEnabled = TRUE;
    DEBUG ((DEBUG_INFO, "HDA: SSP#1 interface pins configuration, Status = %r\n", Status));
  }

  if (HdaConfig->AudioLinkSsp2 && IsAudioInterfaceSupported (PchHdaSsp2)) {
    //
    // Cannot set GPIOs as SSP2 physical pin out if CNVi is present because
    // these pin are shared with CNVi RF interface and muxed out to RF signals by hardware default.
    //
    if (!CnviIsPresent ()) {
      Status = GpioEnableHdaSsp (2);
      IsSspEnabled = TRUE;
      DEBUG ((DEBUG_INFO, "HDA: SSP#2 interface pins configuration, Status = %r\n", Status));
    } else {
      DEBUG ((DEBUG_ERROR, "Cannot set GPIOs to SSP2 if CNVi is present.\n"));
      ASSERT (FALSE);
    }
  }

  if (IsSspEnabled) {
    Status = GpioEnableHdaSspMasterClock ();
    DEBUG ((DEBUG_INFO, "HDA: SSP Master Clock pin configuration, Status = %r\n", Status));
  }

  if (HdaConfig->AudioLinkSndw1 && IsAudioInterfaceSupported (PchHdaSndw1)) {
    Status = GpioEnableHdaSndw (0);
    DEBUG ((DEBUG_INFO, "HDA: SoundWire#1 interface pins configuration, Status = %r\n", Status));
    PmcSndwRcompConfig |= PMC_SNDW_RCOMP_SNDW1_ENABLE;
  }

  if (HdaConfig->AudioLinkSndw2 && IsAudioInterfaceSupported (PchHdaSndw2)) {
    Status = GpioEnableHdaSndw (1);
    DEBUG ((DEBUG_INFO, "HDA: SoundWire#2 interface pins configuration, Status = %r\n", Status));
    PmcSndwRcompConfig |= PMC_SNDW_RCOMP_SNDW2_ENABLE;
  }

  if (HdaConfig->AudioLinkSndw1 || HdaConfig->AudioLinkSndw2) {
    DEBUG ((DEBUG_INFO, "HDA: SoundWire Clock Buffer RCOMP Setting - "));
    if (HdaConfig->SndwBufferRcomp == PchHdaSndwActTopology) {
      DEBUG ((DEBUG_INFO, "ACT Topology (8 Ohm driver impedance)\n"));
      PmcSndwRcompConfig |= PMC_SNDW_RCOMP_ACT_TOPOLOGY;
    } else {
      DEBUG ((DEBUG_INFO, "non-ACT Topology (50 Ohm driver impedance)\n"));
      PmcSndwRcompConfig &= ~PMC_SNDW_RCOMP_ACT_TOPOLOGY;
    }

    PmcConfigureSoundWireBufferRcomp (PmcSndwRcompConfig);
  }

  if (HdaConfig->AudioLinkSndw3 && IsAudioInterfaceSupported (PchHdaSndw3)) {
    Status = GpioEnableHdaSndw (2);
    DEBUG ((DEBUG_INFO, "HDA: SoundWire#3 interface pins configuration, Status = %r\n", Status));
  }

  if (HdaConfig->AudioLinkSndw4 && IsAudioInterfaceSupported (PchHdaSndw4)) {
    Status = GpioEnableHdaSndw (3);
    DEBUG ((DEBUG_INFO, "HDA: SoundWire#4 interface pins configuration, Status = %r\n", Status));
  }
  DEBUG ((DEBUG_INFO, "ConfigureAudioLinks() End\n"));
}

/**
  Disables Intel High Definition Audio Subsystem
  (inclusive of Intel Smart Sound Technology)

  @param[in] HdaPciBase           PCI Configuration Space Base Address

**/
VOID
DisableHdAudio (
  IN      UINT64                  HdaPciBase
  )
{

  DEBUG ((DEBUG_INFO, "DisableHdAudio() Start\n"));


  ///
  /// 1. Put controller to D3 - set PCS.PS = 11b
  ///
  PciSegmentOr32 (HdaPciBase + R_HDA_CFG_PCS, B_HDA_CFG_PCS_PS);

  ///
  /// 2. Arm audio IP function disable on the PMC side.
  ///
  PmcDisableHda ();

  ///
  /// 3. Set PCR[HDA] + 530h[3] to 0
  /// Function disable step 1 - Clock Gating Enabled
  ///
  PchPcrAndThenOr32 (
    PID_DSP,
    R_HDA_PCR_FNCFG,
    (UINT32) ~B_HDA_PCR_FNCFG_CGD,
    (UINT32) 0
    );

  ///
  /// 4. Set PCR[HDA] + 530h [5,4,2,0] = 0b,1b,1b,1b
  /// Function disable step 2 - Power Gating Enabled, BIOS Configuration Lock Down, ADSP Disabled, HDA Disabled
  ///
  PchPcrAndThenOr32 (
    PID_DSP,
    R_HDA_PCR_FNCFG,
    (UINT32) ~B_HDA_PCR_FNCFG_PGD,
    (UINT32) (B_HDA_PCR_FNCFG_BCLD | B_HDA_PCR_FNCFG_ADSPD | B_HDA_PCR_FNCFG_HDASD)
    );

  ///
  /// 5. Disable HDA in PSF
  ///
  PsfDisableHdaDevice ();

  DEBUG ((DEBUG_INFO, "DisableHdAudio()\n"));
}

/**
  Initialize the Intel High Definition Audio Codec(s) present in the system.
  For each codec, a predefined codec verb table should be programmed.
  The list contains 32-bit verbs to be sent to the corresponding codec.
  If it is not programmed, the codec uses the default verb table, which may or may not
  correspond to the platform jack information.

  @param[in] SiPolicy             Policy
  @param[in] HdaPciBase           PCI Configuration Space Base Address
  @param[in] HdaBar               Memory Space Base Address


  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_ABORTED             Controller in reset - unable to proceed with initialization
  @retval EFI_INVALID_PARAMETER   Provided VerbTableData is null
**/
EFI_STATUS
ConfigureHdAudio (
  IN  SI_POLICY_PPI      *SiPolicy,
  IN  UINT64             HdaPciBase,
  IN  UINT32             HdaBar
  )
{
  EFI_STATUS                    Status;
  UINT32                        Data32AndFncfg;
  UINT32                        Data32OrFncfg;
  UINT32                        XtalFrequency;
  PCH_HDAUDIO_CONFIG            *HdaConfig;
  UINT8                         InterruptPin;
  CPU_FAMILY                    CpuFamilyId;

  DEBUG ((DEBUG_INFO, "ConfigureHdAudio() Start\n"));
  Data32AndFncfg = (UINT32) ~0;
  Data32OrFncfg  = 0;
  Status = EFI_SUCCESS;

  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDALBA, HdaBar);
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDAUBA, 0);
  PciSegmentOr16 (HdaPciBase + PCI_COMMAND_OFFSET, (UINT16) EFI_PCI_COMMAND_MEMORY_SPACE);

  DEBUG ((DEBUG_INFO, "HdaPciBase = 0x%X, HdaBar = 0x%X\n", HdaPciBase, HdaBar));

  Status = GetConfigBlock ((VOID *) SiPolicy, &gHdAudioConfigGuid, (VOID *) &HdaConfig);
  ASSERT_EFI_ERROR (Status);

  if (HdaConfig->DspEnable == TRUE) {

    if (HdaConfig->DspUaaCompliance == FALSE) {
      DEBUG ((DEBUG_INFO, "HDA: Audio DSP Enabled, SST mode - set Sub Class Code to 0x%02x\n", V_HDA_CFG_SCC_ADSP));
      //
      // Set Sub Class Code to 01 for Audio DSP enabled (SST only support)
      //
      PciSegmentWrite8 (HdaPciBase + R_HDA_CFG_SCC, (UINT8) V_HDA_CFG_SCC_ADSP);
    } else {
      DEBUG ((DEBUG_INFO, "HDA: Audio DSP Enabled, UAA mode - set Programming Interface to 0x%02x\n", V_HDA_CFG_PI_ADSP_UAA));
      //
      // Set Programming Interface to 01 for Audio DSP enabled (HDA Inbox and SST support)
      //
      PciSegmentWrite8 (HdaPciBase + R_HDA_CFG_PI, (UINT8) V_HDA_CFG_PI_ADSP_UAA);
    }

    //
    // Enable Audio DSP for operation:
    // GPROCEN = 0, ADSPxBA (BAR2) is mapped to the Intel HD Audio memory mapped configuration registers (Default),
    // GPROCEN = 1, ADSPxBA (BAR2) is mapped to the actual Audio DSP memory mapped configuration registers.
    //
    DEBUG ((DEBUG_INFO, "HDA: Set PPCTL.GPROCEN = 1 (Global Processing Enable - to enable ADSP for operation)\n"));
    MmioOr32 ((UINTN) (HdaBar + R_HDA_MEM_PPCTL), (UINT32) B_HDA_MEM_PPCTL_GPROCEN);
  } else {
    DEBUG ((DEBUG_INFO, "HDA: Audio DSP Disabled\n"));
    ///
    /// Set PCR[HDA] + 530h [2] = 1b.
    ///
    Data32OrFncfg |= B_HDA_PCR_FNCFG_ADSPD;
  }

  //
  // 1. PCI power management capability structure PC.PMES reporting PME support for D0, but HW does not generate PME in D0.
  // Program PC.PMES[15:11] = 18h before locking FNCFG.BCLD.
  //
  PciSegmentAndThenOr16 (HdaPciBase + R_HDA_CFG_PC, (UINT16) ~(0xF800), (UINT16) (V_HDA_CFG_PC_PMES << N_HDA_CFG_PC_PMES));

  ///
  /// Update PCI PMCS.PID.NEXT value from 60h (MSI) to 80h (D0i3)
  ///
  PciSegmentAndThenOr16 (HdaPciBase + R_HDA_CFG_PID, (UINT16) ~(0xFF00), (UINT16) (V_HDA_CFG_PID_NEXT_D0I3 << N_HDA_CFG_PID_NEXT));

  ///
  /// Disable Resume Trunk Clock Gating before Clock Gating is enabled
  /// Set CGCTL.RSMTCGE to 0b before claring FNCFG.CGD
  ///
  PciSegmentAnd32 (HdaPciBase + R_HDA_CFG_CGCTL, (UINT32) ~B_HDA_CFG_CGCTL_RSMTCGE);

  ///
  /// Set EM1/SEM1 register (HDABAR + 1000h / HDA PCI Config Space + C0h):
  /// 1) Set LFLCS[24] = 1b - use XTAL oscillator clock source for low frequencies link clock
  /// 2) Set FIFORDYSEL[10:9] = 11b
  /// 3) Set BBRK[31:30] = 00b
  /// 4) Set ECCWRBKD[29] = 1b
  ///
  PciSegmentAndThenOr32 (HdaPciBase + R_HDA_CFG_SEM1,
    (UINT32) ~(B_HDA_CFG_SEM1_BBRK),
    (UINT32) (B_HDA_CFG_SEM1_ECCWRBKD | B_HDA_CFG_SEM1_LFLCS | B_HDA_CFG_SEM1_FIFORDYSEL));

  //
  // Configure NodeID field for iDISPLAY Audio Link's SET_CLKOFF command
  //
  // CNL_A0 CPU: NodeID = 0x8 (HW default)
  // CNL_B0 CPU and newer: NodeID = 0xB
  CpuFamilyId = GetCpuFamily ();
  if (((CpuFamilyId == EnumCpuCnlUltUlx) && (GetCpuStepping () >= EnumCnlB0)) || (CpuFamilyId == EnumCpuCnlDtHalo)) {
    PciSegmentAndThenOr32 (
      HdaPciBase + R_HDA_CFG_SEM1,
      (UINT32)~(B_HDA_CFG_SEM1_NODEID),
      0xB
      );
  }

  ///
  /// Configures Audio Link Mode
  ///
  ConfigureAudioLinks (HdaConfig);

  ///
  /// Un-reset HDA and turn on the link
  ///
  Status = SetHdaControllerReset (HdaBar, HdaConfig->ResetWaitTimer, FALSE);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Un-reset High Definition Audio (CRSTB = 1) Time Out!\n"));
    goto ExitInitCodec;
  }

  DetectAndInitializeCodec (HdaConfig, HdaPciBase, HdaBar, HDA_HDALINK_INDEX);

  ///
  /// Set HDABAR + 104Bh[5,3] = 1b and clear bit 2,1,0
  ///
  MmioAndThenOr8 ((UINTN) (HdaBar + R_HDA_MEM_PCE), (UINT8) ~(BIT2 | BIT1 | BIT0), (UINT8) (BIT5 | BIT3));

  ///
  /// Set optimum SRAM retention energy break even - "SRMIW", PCR[DSP] + 628h[6:4]
  /// 1. if XTAL = 24 MHz, PTDC.SRMIW = 4 (64 XTAL clock = 2.67 us)
  /// 2. if XTAL = 38.4 MHz, PTDC.SRMIW = 4 (64 XTAL clock = 1.67 us)
  /// 3. if XTAL = 19.2 MHz, PTDC.SRMIW = 3 (32 XTAL clock = 1.67 us)
  ///
  XtalFrequency = (PchPcrRead32 (PID_DSP, R_HDA_PCR_STRVAL) & B_HDA_PCR_STRVAL_XOCFS) >> N_HDA_PCR_STRVAL_XOCFS;
  switch (XtalFrequency) {
    case V_HDA_PCR_STRVAL_XOCFS_24MHZ:
    case V_HDA_PCR_STRVAL_XOCFS_38_4MHZ:
      DEBUG ((DEBUG_INFO, "HDA: SRAM retention mode idle wait - 64 XTAL oscillator clocks\n"));
      PchPcrAndThenOr32 (PID_DSP, R_HDA_PCR_PTDC,
        (UINT32) ~B_HDA_PCR_PTDC_SRMIW,
        (UINT32) (V_HDA_PCR_PTDC_SRMIW_64XTAL << N_HDA_PCR_PTDC_SRMIW));
      break;
    case V_HDA_PCR_STRVAL_XOCFS_19_2MHZ:
      DEBUG ((DEBUG_INFO, "HDA: SRAM retention mode idle wait - 32 XTAL oscillator clocks\n"));
      PchPcrAndThenOr32 (PID_DSP, R_HDA_PCR_PTDC,
        (UINT32) ~B_HDA_PCR_PTDC_SRMIW,
        (UINT32) (V_HDA_PCR_PTDC_SRMIW_32XTAL << N_HDA_PCR_PTDC_SRMIW));
      break;
    default:
      DEBUG ((DEBUG_ERROR, "HDA: Invalid XTAL Oscillator Clock Frequency!\n"));
      ASSERT (TRUE);
  }

  if (IsPchLp () && (PchStepping () <= PCH_B0)) {
    PsfDisableUpstreamCompletionTrackingForHda ();
  }


  ///
  /// Set HDABAR + 1048h[6:0] = 29h
  ///
  MmioAndThenOr8 ((UINTN) (HdaBar + R_HDA_MEM_LTRC), (UINT8) ~(0x7F), (UINT8) V_HDA_MEM_LTRC_GB);

  ///
  /// Set Buffer Size Minimum Threshold to 128B.
  /// Program the VC0 cycles to use SNR = 0.
  /// Program the DPIB status write policy to always send update at the end of the HD-A 48 KHz frame period.
  /// Disable the Cx break L1 opportunistic exit.
  /// Set EM2/SEM2.BSMT [BIT27:26] to 01b
  /// Set EM2/SEM2.VC0SNR [BIT24] to 0b
  /// Set EM2/SEM2.DUM [BIT23] to 1b
  /// Set EM2/SEM2.CBL1EXEN [BIT12] to 0b
  ///
  PciSegmentAndThenOr32 (HdaPciBase + R_HDA_CFG_SEM2,
    (UINT32) ~(B_HDA_CFG_SEM2_BSMT | B_HDA_CFG_SEM2_VC0SNR | B_HDA_CFG_SEM2_CBL1EXEN),
    (UINT32) ((V_HDA_CFG_SEM2_BSMT << N_HDA_CFG_SEM2_BSMT) | B_HDA_CFG_SEM2_DUM));

  ///
  /// Set Input Stream L1 Exit Threshold 2 to 1/8 of FIFO size
  /// Set EM3L/SEM3L.ISL1EXT2 to 10b
  ///
  PciSegmentAndThenOr32 (HdaPciBase + R_HDA_CFG_SEM3L,
    (UINT32) ~B_HDA_CFG_SEM3L_ISL1EXT2,
    (UINT32) (V_HDA_CFG_SEM3L_ISL1EXT2 << N_HDA_CFG_SEM3L_ISL1EXT2));

  ///
  /// Set Output Stream L1 Exit Threshold 2 to 1/4 of FIFO size
  /// Set EM4L/SEM4L.OSL1EXT2 to 11b
  ///
  PciSegmentAndThenOr32 (HdaPciBase + R_HDA_CFG_SEM4L,
    (UINT32)~B_HDA_CFG_SEM4L_OSL1EXT2,
    (UINT32)(V_HDA_CFG_SEM4L_OSL1EXT2 << N_HDA_CFG_SEM4L_OSL1EXT2));

  if (HdaConfig->VcType == PchHdaVc1) {
    ///
    /// Enable Virtual Channel VC1
    /// Permit non snoop on VC0, PCI Config Space + C4h[24] = 1b.
    /// Enable VC1 traffic, PCR[HDA] + E4h[1] = 1b
    ///
    DEBUG ((DEBUG_INFO, "HDA: Isonchronous Support on VC1\n"));
    PciSegmentOr32 (HdaPciBase + R_HDA_CFG_SEM2, (UINT32) B_HDA_CFG_SEM2_VC0SNR);
    PchPcrAndThenOr32 (PID_DSP, R_HDA_PCR_TTCCFG, (UINT32) ~0, (UINT32) B_HDA_PCR_TTCCFG_HCDT);
  }

  ///
  /// Power Gating and Clock Gating enable
  /// Clear "PGD", PCR[DSP] + 530h[5] = 0b
  /// Clear "CGD", PCR[DSP] + 530h[3] = 0b
  ///
  DEBUG ((DEBUG_INFO, "HDA: Power Gating and Clock Gating enabled\n"));
  Data32AndFncfg &= ~(B_HDA_PCR_FNCFG_PGD | B_HDA_PCR_FNCFG_CGD);

  ///
  /// Set PCR[HDA] + 530h [4] = 1b.
  ///
  DEBUG ((DEBUG_INFO, "HDA: Configuration Lockdown\n"));
  Data32OrFncfg |= B_HDA_PCR_FNCFG_BCLD;

  //
  // Write PCR[DSP] FNCFG register only once, because BCLD bit is RWO
  //
  PchPcrAndThenOr32 (PID_DSP, R_HDA_PCR_FNCFG, Data32AndFncfg, Data32OrFncfg);

  if (HdaConfig->HdAudioLinkFrequency == PchHdaLinkFreq6MHz) {
    DEBUG ((DEBUG_INFO, "HDA: 6MHz link frequency - assert RESET# and put controller to D3\n"));
    Status = SetHdaControllerReset (HdaBar, HdaConfig->ResetWaitTimer, TRUE);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "Reset High Definition Audio (CRST = 0) Time Out!\n"));
    }
    ///
    /// Put controller to D3 - set PCS.PS = 11b
    ///
    PciSegmentOr32 (HdaPciBase + R_HDA_CFG_PCS, B_HDA_CFG_PCS_PS);
  }

  ///
  /// Configure HD Audio interrupt
  ///
  InterruptPin = ItssGetDevIntPin (SiPolicy, PCI_DEVICE_NUMBER_PCH_HDA, PCI_FUNCTION_NUMBER_PCH_HDA);

  PciSegmentWrite8 (HdaPciBase + PCI_INT_PIN_OFFSET, InterruptPin);

ExitInitCodec:

  ///
  /// Clear HDBAR and disable memory map access
  ///
  PciSegmentAnd16 (HdaPciBase + PCI_COMMAND_OFFSET, (UINT16) (~EFI_PCI_COMMAND_MEMORY_SPACE));
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDALBA, 0);
  PciSegmentWrite32 (HdaPciBase + R_HDA_CFG_HDAUBA, 0);

  DEBUG ((DEBUG_INFO, "ConfigureHdAudio() End - Status = %r\n", Status));

  return Status;
}

/**
  Initialize the Intel High Definition Audio Controller

  @param[in] SiPolicy             Policy

**/
VOID
HdAudioInit (
  IN SI_POLICY_PPI      *SiPolicy
  )
{
  UINT64                        HdaPciBase;
  UINT32                        HdaBar;

  HdaPciBase = PCI_SEGMENT_LIB_ADDRESS (
                 DEFAULT_PCI_SEGMENT_NUMBER_PCH,
                 DEFAULT_PCI_BUS_NUMBER_PCH,
                 PCI_DEVICE_NUMBER_PCH_HDA,
                 PCI_FUNCTION_NUMBER_PCH_HDA,
                 0
                 );

  if (IsHdAudioEnabled ()) {
    ///
    /// System BIOS should ensure that the High Definition Audio HDBAR HDA PCI offset 10-17h contains a valid address value
    /// and is enabled by setting HDA PCI offset 04h[1].
    ///
    HdaBar = PcdGet32 (PcdSiliconInitTempMemBaseAddr);
    ConfigureHdAudio (SiPolicy, HdaPciBase, HdaBar);
  } else {
    DisableHdAudio (HdaPciBase);
  }
}


