/** @file
  OC CPU Early Post initializations.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2018 Intel Corporation.

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

@par Specification
**/

#include <Private/Library/CpuCommonLib.h>
#include <Private/Library/CpuOcLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/CpuMailboxLib.h>
#include <Library/ConfigBlockLib.h>
#include <Library/PostCodeLib.h>
#include <Library/CpuPlatformLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/PmcLib.h>

/**
  Initializes Ring related overclocking settings.

  @param[in] CpuOverClockingConfig      Pointer to CPU Overclocking Policy instance

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
CpuOcInitRingSettings (
  IN CPU_OVERCLOCKING_PREMEM_CONFIG *CpuOverClockingConfig
  )
{
  EFI_STATUS              Status;
  UINT32                  LibStatus;
  UINT32                  MailboxData;
  UINT32                  CurrentRingDownBin;

  ///
  /// Program Ring Downbin feature if needed
  ///
  Status = MailboxRead(MAILBOX_TYPE_OC, OC_LIB_CMD_GET_MISC_TURBO_CONTROL, &MailboxData, &LibStatus);
  if ((Status == EFI_SUCCESS) && (LibStatus == PCODE_MAILBOX_CC_SUCCESS)) {
    ///
    /// Ring Downbin mailbox command uses reverse encoding. 0 - Enable Ring Downbin, 1 - Disable Ring Downbin
    ///
    CurrentRingDownBin = (~MailboxData & MISC_TURBO_RING_DOWNBIN_MASK) >> MISC_TURBO_RING_DOWNBIN_OFFSET;
    if (CpuOverClockingConfig->RingDownBin != CurrentRingDownBin) {
      DEBUG ((DEBUG_INFO, "(OC) Updating Ring Downbin = %X\n", CpuOverClockingConfig->RingDownBin));
      MailboxData = (~CpuOverClockingConfig->RingDownBin << MISC_TURBO_RING_DOWNBIN_OFFSET) & MISC_TURBO_RING_DOWNBIN_MASK;
      MailboxWrite(MAILBOX_TYPE_OC, OC_LIB_CMD_SET_MISC_TURBO_CONTROL, MailboxData, &LibStatus);
      if (LibStatus != EFI_SUCCESS) {
        DEBUG ((DEBUG_INFO, "(OC) Ring Downbin message failed, mailbox status = %X\n", LibStatus));
      }
    }
  }

  return Status;
}

/**
  Initializes IA/Core related overclocking settings.

  @param[in] CpuOverClockingConfig      Pointer to CPU Overclocking Policy instance

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
CpuOcInitCoreSettings(
IN CPU_OVERCLOCKING_PREMEM_CONFIG *CpuOverClockingConfig,
OUT BOOLEAN *ResetRequired
  )
{
  EFI_STATUS          Status;
  UINT32              LibStatus;
  AVX_RATIO_ITEM      CurrentAvxRatioOffset;
  AVX_RATIO_ITEM      RequestedAvxRatioOffset;
  UINT32              CurrentPllOffset;
  UINT32              RequestedPllOffset;
  UINT32              MailboxReadCommand;
  UINT32              MailboxWriteCommand;
  UINT32              MailboxData;
  UINT32              MiscGlobalConfigData;
  UINT8               DomainId;
  UINT32              CurrentTjMaxOffset;
  UINT32              RequestedTjMaxOffset;

  ///
  /// Configure the Pll Voltage offsets for each CPU domain
  ///

  //
  // Initialize
  //
  RequestedPllOffset  = 0x00;
  MailboxReadCommand  = 0x00;
  MailboxWriteCommand = 0x00;
  ZeroMem (&RequestedAvxRatioOffset, sizeof (AVX_RATIO_ITEM));

  for (DomainId = OC_LIB_DOMAIN_ID_IA_CORE; DomainId <= OC_LIB_DOMAIN_ID_MEMORY_CONTROLLER; DomainId++ ) {

    switch (DomainId) {
      case OC_LIB_DOMAIN_ID_IA_CORE:
        RequestedPllOffset = CpuOverClockingConfig->CorePllVoltageOffset;
        MailboxReadCommand = READ_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_IA_CORE << 8);
        MailboxWriteCommand = WRITE_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_IA_CORE << 8);
      break;

      case OC_LIB_DOMAIN_ID_GT:
      case OC_LIB_DOMAIN_ID_GT_UNSLICE:
        RequestedPllOffset = CpuOverClockingConfig->GtPllVoltageOffset;
        MailboxReadCommand = READ_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_GT << 8);
        MailboxWriteCommand = WRITE_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_GT << 8);
      break;

      case OC_LIB_DOMAIN_ID_RING:
        RequestedPllOffset = CpuOverClockingConfig->RingPllVoltageOffset;
        MailboxReadCommand = READ_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_RING << 8);
        MailboxWriteCommand = WRITE_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_RING << 8);
      break;

      case OC_LIB_DOMAIN_ID_SYSTEM_AGENT:
        RequestedPllOffset = CpuOverClockingConfig->SaPllVoltageOffset;
        MailboxReadCommand = READ_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_SYSTEM_AGENT << 8);
        MailboxWriteCommand = WRITE_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_SYSTEM_AGENT << 8);
      break;

      case OC_LIB_DOMAIN_ID_MEMORY_CONTROLLER:
        RequestedPllOffset = CpuOverClockingConfig->McPllVoltageOffset;
        MailboxReadCommand = READ_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_MEMORY_CONTROLLER << 8);
        MailboxWriteCommand = WRITE_PLL_VOLTAGE_OFFSET_CMD | (OC_LIB_DOMAIN_ID_MEMORY_CONTROLLER << 8);
      break;

      default:
      break;
    }

    ///
    /// Read the current PLL voltage offset
    ///
    MailboxRead(MAILBOX_TYPE_PCODE, MailboxReadCommand, &CurrentPllOffset, &LibStatus);
    if (LibStatus == PCODE_MAILBOX_CC_SUCCESS) {
      ///
      /// If needed, write the new PLL voltage offset
      ///
      if (RequestedPllOffset != CurrentPllOffset) {
        DEBUG ((DEBUG_INFO, "(OC) Current Pll offset for domain %d  = %X\n", DomainId, CurrentPllOffset));
        DEBUG ((DEBUG_INFO, "(OC) Requested Pll offset for domain %d = %X\n",DomainId, RequestedPllOffset));
        MailboxWrite(MAILBOX_TYPE_PCODE, MailboxWriteCommand, RequestedPllOffset, &LibStatus);
        if (LibStatus != EFI_SUCCESS) {
          DEBUG ((DEBUG_INFO, "(OC) Pll message failed for Domain %d, mailbox status = %X\n", DomainId, LibStatus));
        } else {
          *ResetRequired = TRUE;
        }
      }
    }
  }

  ///
  /// Get the current AVX ratio from OC mailbox
  ///
  Status = GetAvxRatioOffset(&CurrentAvxRatioOffset, &LibStatus);
  if (LibStatus == OC_LIB_COMPLETION_CODE_SUCCESS) {
    DEBUG ((DEBUG_INFO, "(OC) Current AVX2 Ratio Offset   = %X\n", CurrentAvxRatioOffset.Avx2Ratio));
    DEBUG ((DEBUG_INFO, "(OC) Requested AVX2 Ratio Offset = %X\n", CpuOverClockingConfig->Avx2RatioOffset));
    RequestedAvxRatioOffset.Avx2Ratio = (UINT8)CpuOverClockingConfig->Avx2RatioOffset;


#ifndef CPU_CFL
    DEBUG ((DEBUG_INFO, "(OC) Current AVX3 Ratio Offset   = %X\n", CurrentAvxRatioOffset.Avx3Ratio));
    DEBUG ((DEBUG_INFO, "(OC) Requested AVX3 Ratio Offset = %X\n", CpuOverClockingConfig->Avx3RatioOffset));
    RequestedAvxRatioOffset.Avx3Ratio = (UINT8)CpuOverClockingConfig->Avx3RatioOffset;
#endif

    if (CompareMem ((VOID *)&RequestedAvxRatioOffset,(VOID *)&CurrentAvxRatioOffset,sizeof (AVX_RATIO_ITEM))) {
      ///
      /// Set the requested AVX ratio to OC mailbox
      ///
      Status = SetAvxRatioOffset(RequestedAvxRatioOffset, &LibStatus);
      if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
        DEBUG ((DEBUG_ERROR, "(OC) Set AVX ratio offset failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
      }
    }
  } else {
    DEBUG ((DEBUG_INFO, "(OC) Get AVX Ratio offset failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
  }

  ///
  /// Read the current TjMax Offset
  ///
  MailboxRead(MAILBOX_TYPE_PCODE, MAILBOX_READ_OC_TJ_MAX_OFFSET_CMD, &CurrentTjMaxOffset, &LibStatus);
  if (LibStatus == PCODE_MAILBOX_CC_SUCCESS) {
    RequestedTjMaxOffset = (UINT8)CpuOverClockingConfig->TjMaxOffset;
    //
    // If needed, write the new TjMax Offset.
    //
    if (RequestedTjMaxOffset != CurrentTjMaxOffset) {
      DEBUG((DEBUG_INFO, "(OC) Current TjMax Offset  = %X\n", CurrentTjMaxOffset));
      DEBUG((DEBUG_INFO, "(OC) Requested TjMax Offset  = %X\n", RequestedTjMaxOffset));

      //
      // Check if TjMax Offset is within acceptable range of 10 to Max TjMax Offset.
      //
      if ((RequestedTjMaxOffset >= MIN_TJ_MAX_OFFSET) && (RequestedTjMaxOffset <= MAX_TJ_MAX_OFFSET)) {
        MailboxWrite(MAILBOX_TYPE_PCODE, MAILBOX_WRITE_OC_TJ_MAX_OFFSET_CMD, RequestedTjMaxOffset, &LibStatus);
        if (LibStatus != EFI_SUCCESS) {
          DEBUG((DEBUG_ERROR, "(OC) Failed to change TjMax Offset to %X, mailbox status = %X\n", RequestedTjMaxOffset, LibStatus));
        } else {
          *ResetRequired = TRUE;
        }
      } else {
        DEBUG((DEBUG_INFO, "(OC) Requested TjMax Offset of %d is out of range and will not be applied. Valid Range 10 to 63.\n", RequestedTjMaxOffset));
      }
    } 
  }else {
    DEBUG((DEBUG_INFO, "(OC) Failed to read TjMaxOffset message. LibStatus = %d\n", LibStatus));
  }

  ///
  /// Misc Global Config programming
  ///
#ifdef CPU_CFL
  MiscGlobalConfigData = (UINT32)((CpuOverClockingConfig->BclkAdaptiveVoltage << MISC_GLOBAL_BCLK_ADAPTIVE_OFFSET) & BIT2);
#else
  ///
  /// Fivr options are reverse encoded in the config block defintion
  /// We need to convert to the mailbox format before writing
  ///
  MiscGlobalConfigData = (UINT32)(((CpuOverClockingConfig->BclkAdaptiveVoltage << MISC_GLOBAL_BCLK_ADAPTIVE_OFFSET) |
                                 ((~CpuOverClockingConfig->FivrEfficiency & BIT0) << MISC_GLOBAL_FIVR_EFFICIENCY_OFFSET) |
                                 (~CpuOverClockingConfig->FivrFaults & BIT0)) & MISC_GLOBAL_CFG_COMMAND_MASK);
#endif
  DEBUG ((DEBUG_INFO, "(OC) Set Misc Global Config = %X\n", MiscGlobalConfigData));
  Status = MailboxWrite(MAILBOX_TYPE_OC, OC_LIB_CMD_SET_MISC_GLOBAL_CONFIG, MiscGlobalConfigData, &LibStatus);
  if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
    DEBUG ((DEBUG_ERROR, "(OC) Set Misc Global Config Failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
  }

  ///
  /// Program Thermal Velocity Boost (TVB) features
  ///
  Status = MailboxRead(MAILBOX_TYPE_OC, OC_LIB_CMD_GET_MISC_TURBO_CONTROL, &MailboxData, &LibStatus);
  if ((Status == EFI_SUCCESS) && (LibStatus == PCODE_MAILBOX_CC_SUCCESS)) {
    ///
    /// TVB Ratio Clipping and TVB Voltage Optimization programming.
    /// Mailbox encoding is reversed,0 = enable, 1 = disable.
    ///
    MailboxData = (UINT32)((MailboxData & MISC_TURBO_RING_DOWNBIN_MASK) |
                        (((~CpuOverClockingConfig->TvbRatioClipping << MISC_TURBO_TVB_RATIO_OFFSET) & MISC_TURBO_TVB_RATIO_MASK) |
                        ((~CpuOverClockingConfig->TvbVoltageOptimization << MISC_TURBO_TVB_VOLTAGE_OFFSET) & MISC_TURBO_TVB_VOLTAGE_MASK)));
    DEBUG ((DEBUG_INFO, "(OC) Updating Thermal Velocity Boost settings, RatioClipping = %X, VoltageOptimization = %X\n", CpuOverClockingConfig->TvbRatioClipping, CpuOverClockingConfig->TvbVoltageOptimization));
    MailboxWrite(MAILBOX_TYPE_OC, OC_LIB_CMD_SET_MISC_TURBO_CONTROL, MailboxData, &LibStatus);
    if (LibStatus != EFI_SUCCESS) {
      DEBUG ((DEBUG_INFO, "(OC) Thermal Velocity Boost message failed, mailbox status = %X\n", LibStatus));
    }
  }

  return Status;
}

/**
  Initializes CPU domain voltage settings for overclocking.

  @param[in] CpuOverClockingConfig      Pointer to CPU Overclocking Policy instance

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
CpuOcInitVoltageSettings (
  IN CPU_OVERCLOCKING_PREMEM_CONFIG *CpuOverClockingConfig
  )
{
  EFI_STATUS              Status;
  OC_CAPABILITIES_ITEM    OcCaps;
  VOLTAGE_FREQUENCY_ITEM  CurrentVfItem;
  VOLTAGE_FREQUENCY_ITEM  RequestedVfItem;
  UINT32                  LibStatus;
  UINT8                   DomainId;
  UINT8                   MinDomainId;
  UINT8                   MaxDomainId;
  WDT_PPI                 *gWdtPei;

  MinDomainId = OC_LIB_DOMAIN_ID_DDR;
  MaxDomainId = OC_LIB_DOMAIN_ID_SYSTEM_AGENT;
  Status = EFI_SUCCESS;

  //
  // Locate WDT_PPI (ICC WDT PPI)
  //
  Status = PeiServicesLocatePpi (
             &gWdtPpiGuid,
             0,
             NULL,
             (VOID **) &gWdtPei
             );
  ASSERT_EFI_ERROR (Status);

  ///
  /// We will loop on the CPU domains to manage the voltage/frequency settings
  ///
  for (DomainId = MinDomainId; DomainId < MaxDomainId; DomainId++) {
    ///
    /// Only IA_CORE and RING is valid for CPU Core
    ///
    if ((DomainId == OC_LIB_DOMAIN_ID_IA_CORE) || (DomainId == OC_LIB_DOMAIN_ID_RING)) {
      ///
      /// Get OC Capabilities of the domain
      ///
      ZeroMem (&OcCaps,sizeof (OcCaps));
      OcCaps.DomainId = DomainId;
      Status = GetOcCapabilities (&OcCaps,&LibStatus);

      if (LibStatus == OC_LIB_COMPLETION_CODE_SUCCESS) {
        ///
        /// If any OC is supported on this domain, then proceed
        ///
        if (OcCaps.RatioOcSupported || OcCaps.VoltageOverridesSupported || OcCaps.VoltageOffsetSupported) {
          ///
          /// Need to populate the user requested settings from the Policy
          /// to determine if OC changes are desired.
          ///
          ZeroMem (&CurrentVfItem,sizeof (CurrentVfItem));
          CurrentVfItem.DomainId = DomainId;

          ///
          /// Get a copy of the current domain VfSettings from the Mailbox Library
          ///
          Status = GetVoltageFrequencyItem (&CurrentVfItem,&LibStatus);
          if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
            continue;
          }

          ///
          /// Populate the user requested VfSettings struct
          ///
          ZeroMem (&RequestedVfItem,sizeof (RequestedVfItem));
          RequestedVfItem.DomainId = DomainId;
          if (DomainId == OC_LIB_DOMAIN_ID_IA_CORE) {
            RequestedVfItem.VfSettings.MaxOcRatio = (UINT8) CpuOverClockingConfig->CoreMaxOcRatio;
          } else if (DomainId == OC_LIB_DOMAIN_ID_RING) {
            RequestedVfItem.VfSettings.MaxOcRatio = (UINT8) CpuOverClockingConfig->RingMaxOcRatio;
          }

          ///
          /// VoltageTarget has 2 uses and we need to update the target based
          /// on the voltagemode requested
          ///
          if (DomainId == OC_LIB_DOMAIN_ID_IA_CORE) {
            RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) CpuOverClockingConfig->CoreVoltageMode;
            if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->CoreVoltageAdaptive;
            } else {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->CoreVoltageOverride;
            }
            RequestedVfItem.VfSettings.VoltageOffset = CpuOverClockingConfig->CoreVoltageOffset;
          } else if (DomainId == OC_LIB_DOMAIN_ID_RING) {
#ifdef CPU_CFL
            ///
            /// CFL CPU shares the Core and Ring VR, so Ring voltage settings must equal Core voltage settings.
            /// Ring Max OC Ratio can be applied separately since it is not tied to the Ring VR.
            ///
            RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) CpuOverClockingConfig->CoreVoltageMode;
            if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->CoreVoltageAdaptive;
            } else {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->CoreVoltageOverride;
            }
            RequestedVfItem.VfSettings.VoltageOffset = CpuOverClockingConfig->CoreVoltageOffset;
#else
            ///
            /// Update Ring Voltage settings
            ///
            RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) CpuOverClockingConfig->RingVoltageMode;
            if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->RingVoltageAdaptive;
            } else {
              RequestedVfItem.VfSettings.VoltageTarget = CpuOverClockingConfig->RingVoltageOverride;
            }
            RequestedVfItem.VfSettings.VoltageOffset = CpuOverClockingConfig->RingVoltageOffset;
#endif
          }


          ///
          /// Compare current settings with user requested settings to see if changes are needed
          ///
          if (CompareMem ((VOID *)&RequestedVfItem,(VOID *)&CurrentVfItem,sizeof (VOLTAGE_FREQUENCY_ITEM))) {
            ///
            /// Arm watchdog timer for OC changes
            ///
            Status = gWdtPei->ReloadAndStart (WDT_TIMEOUT_BETWEEN_PEI_DXE);

            ///
            /// Need to update the requested voltage/frequency values
            ///
            DEBUG ((DEBUG_INFO, "(OC) Set Voltage Frequency for Domain = %X\n", DomainId));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.MaxOcRatio     = %X\n", RequestedVfItem.VfSettings.MaxOcRatio));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.TargetMode     = %X\n", RequestedVfItem.VfSettings.VoltageTargetMode));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.VoltageTarget  = %X\n", RequestedVfItem.VfSettings.VoltageTarget));
            DEBUG ((DEBUG_INFO, "(OC) RequestedVfItem.VfSettings.VoltageOffset  = %X\n", RequestedVfItem.VfSettings.VoltageOffset));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.MaxOcRatio       = %X\n", CurrentVfItem.VfSettings.MaxOcRatio));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.TargetMode       = %X\n", CurrentVfItem.VfSettings.VoltageTargetMode));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.VoltageTarget    = %X\n", CurrentVfItem.VfSettings.VoltageTarget));
            DEBUG ((DEBUG_INFO, "(OC) CurrentVfItem.VfSettings.VoltageOffset    = %X\n", CurrentVfItem.VfSettings.VoltageOffset));

            Status = SetVoltageFrequencyItem (RequestedVfItem,&LibStatus);
            if ((Status != EFI_SUCCESS) || (LibStatus != OC_LIB_COMPLETION_CODE_SUCCESS)) {
              DEBUG ((DEBUG_ERROR, "(OC) Set Core Voltage Frequency failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
            }
          }
        }
      } else {
        DEBUG ((DEBUG_ERROR, "(OC) GetOcCapabilities message failed. Library Status = %X, Domain = %X\n", LibStatus, DomainId));
      }
    }
  }

  return Status;
}

/**
  Initializes pre-memory Overclocking settings in the processor.

  @param[in] OverclockingtConfig      Pointer to Policy protocol instance

  @retval EFI_SUCCESS
**/
EFI_STATUS
EFIAPI
CpuOcInitPreMem (
  IN SI_PREMEM_POLICY_PPI *SiPreMemPolicyPpi
  )
{
  EFI_STATUS                     Status;
  CPU_OVERCLOCKING_PREMEM_CONFIG *CpuOverClockingConfig;
  BOOLEAN                        ResetRequired;
  EFI_BOOT_MODE                  BootMode;

  DEBUG((DEBUG_INFO, "CpuOcInitPreMem Start \n"));
  PostCode (0xC26);

  ResetRequired = FALSE;
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gCpuOverclockingPreMemConfigGuid, (VOID *) &CpuOverClockingConfig);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Check for Overclocking support
  ///
  if (CpuOverClockingConfig->OcSupport == 0) {
    DEBUG ((DEBUG_INFO, "(OC) Overclocking is disabled. Bypassing CPU core overclocking flow.\n"));
    return EFI_SUCCESS;
  }

  ///
  /// Configure Ring related settings
  ///
  Status = CpuOcInitRingSettings(CpuOverClockingConfig);

  ///
  /// Configure Core related settings
  ///
  Status = CpuOcInitCoreSettings(CpuOverClockingConfig, &ResetRequired);

  ///
  /// Configure OC voltage settings
  ///
  Status = CpuOcInitVoltageSettings(CpuOverClockingConfig);


  ///
  /// Perform Cold Reset if required by Core Pll voltage offset changes
  ///
  if (ResetRequired == TRUE) {
    ///
    /// Check if BIOS is on S3 flow
    ///
    Status = PeiServicesGetBootMode (&BootMode);
    if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
      DEBUG((DEBUG_INFO, "(OC) BIOS is running in the S3 resume flow. Need to update S3 scratchpad before performing warm reset.\n"));
      PmcSetBclkS3 ();
    }
    DEBUG ((DEBUG_INFO, "(OC) Perform Warm Reset\n"));
    DEBUG((DEBUG_INFO, "CpuOcInit Reset \n"));
    PostCode (0xC27);
    PerformWarmOrColdReset (WARM_RESET);
  }

  DEBUG((DEBUG_INFO, "CpuOcInitPreMem End\n"));
  return Status;
}

