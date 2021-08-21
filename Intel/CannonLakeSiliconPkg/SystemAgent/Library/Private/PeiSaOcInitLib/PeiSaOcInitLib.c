/** @file
  OC System Agent Early Post initializations.

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

@par Specification Reference:
**/
#include <Private/Library/CpuOcLib.h>
#include <Library/CpuPlatformLib.h>
#include <Uefi.h>
#include <Library/PeiServicesLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Ppi/SiPolicy.h>
#include <Ppi/Wdt.h>
#include <CpuAccess.h>
#include <Private/Library/SaOcInitLib.h>
#include <Library/CpuMailboxLib.h>
#include <Library/PeiMeLib.h>
#include <Library/IoLib.h>
#include <Library/PmcLib.h>
#include <Library/MeTypeLib.h> // UP_SPS_SUPPORT

/**
  This function will perform the BCLK warm reset overclocking flow.

  @param[in] VOID

  @retval EFI_SUCCESS          Function successfully executed.

  Appropriate failure code on error.
**/
EFI_STATUS
EFIAPI
PerformBclkOcFlow (
  VOID
  )
{
  EFI_STATUS        Status;
  WDT_PPI           *WdtPei;
  UINT8             WarmResetRequired;
  UINT8             WdtTimeout;
  CPU_FAMILY        CpuFamily;
  EFI_BOOT_MODE     BootMode;

  Status = EFI_SUCCESS;
  CpuFamily = GetCpuFamily ();
  WarmResetRequired = FALSE;
  WdtTimeout = FALSE;

  if (CpuFamily == EnumCpuCnlUltUlx) {
    DEBUG((DEBUG_INFO, "(ICC) Bclk overclocking is not supported on mobile skus.\n"));
    return Status;
  }

  //
  // Locate WDT_PPI (ICC WDT PPI)
  //
  Status = PeiServicesLocatePpi (
             &gWdtPpiGuid,
             0,
             NULL,
             (VOID **) &WdtPei
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Check for WDT timeout on previous boot
  //
  WdtTimeout = WdtPei->CheckStatus();

  //
  // If no WDT timeout, ramp BCLK, otherwise, continue boot to recover.
  //
  if (WdtTimeout == FALSE) {

    Status = PeiHeciIccBclkMsg (&WarmResetRequired);
    if (EFI_ERROR (Status)) {
      DEBUG((DEBUG_WARN, "(ICC) PeiHeciIccBclkMsg failed with Status = %X\n", Status));
      return Status;
    }
    DEBUG((DEBUG_INFO, "(ICC) PerformBclkOcFlow start.\n"));

    if (WarmResetRequired == 1) {
      DEBUG((DEBUG_INFO, "(ICC) ME FW acknowledged BCLK Ramp reset is required.\n"));

      ///
      /// Check if BIOS is on S3 flow
      ///
      Status = PeiServicesGetBootMode (&BootMode);
      if ((Status == EFI_SUCCESS) && (BootMode == BOOT_ON_S3_RESUME)) {
        DEBUG((DEBUG_INFO, "(ICC) BIOS is running in the S3 resume flow. Need to update S3 scratchpad before performing warm reset.\n"));
        PmcSetBclkS3 ();
      }
      //
      // Start the Watchdog Timer
      //
      Status = WdtPei->ReloadAndStart (60);
      ASSERT_EFI_ERROR (Status);

      //
      // Warm reset is required for BCLK frequency changes. The BCLK frequency
      // is ramped while the CPU is in reset.
      //
      DEBUG ((DEBUG_INFO, "(ICC) Performing Warm Reset for BCLK frequency change.\n"));
      WdtPei->AllowKnownReset ();
      (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetWarm, EFI_SUCCESS, 0, NULL);
    }
  }

  DEBUG((DEBUG_INFO, "(ICC) No BCLK warm reset required.\n"));
  return Status;
}

/**
  Initializes Overclocking settings in the processor.

  @param[in]  OVERCLOCKING_PREMEM_CONFIG  OcConfig

  @retval EFI_SUCCESS
**/
EFI_STATUS
SaOcInit (
  IN   OVERCLOCKING_PREMEM_CONFIG  *OcConfig
  )
{
  EFI_STATUS    Status;
  OC_CAPABILITIES_ITEM OcCaps;
  VOLTAGE_FREQUENCY_ITEM CurrentVfItem;
  VOLTAGE_FREQUENCY_ITEM RequestedVfItem;
  UINT32      LibStatus;
  UINT8       DomainId;
  BOOLEAN     VfUpdateNeeded;
  WDT_PPI     *gWdtPei;
  UINT32      CurrentRealtimeMemory;

  LibStatus = 0;
  VfUpdateNeeded = FALSE;
  Status = EFI_SUCCESS;

  DEBUG((DEBUG_INFO, "SaOcInit Start \n"));

  //
  // Begin UP_SPS_SUPPORT
  //
  if (MeTypeIsSps()) {
    DEBUG ((DEBUG_INFO, "(OC) SPS detected - Overclocking initialization skipped\n"));
    return EFI_SUCCESS;
  }
  //
  // End UP_SPS_SUPPORT
  //

  if (OcConfig->OcSupport == TRUE) {
    ///
    /// Overclocking is enabled
    ///
    ZeroMem (&CurrentVfItem,sizeof (CurrentVfItem));
    ZeroMem (&RequestedVfItem,sizeof (RequestedVfItem));

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
    /// Update Realtime Memory Timings if needed
    ///
    MailboxRead(MAILBOX_TYPE_PCODE, OC_LIB_CMD_GET_REALTIME_MEMORY_STATUS, &CurrentRealtimeMemory, &LibStatus);
    if (OcConfig->RealtimeMemoryTiming != CurrentRealtimeMemory) {
      DEBUG ((DEBUG_INFO, "(OC) Current Realtime Memory Timing = %X\n", CurrentRealtimeMemory));
      DEBUG ((DEBUG_INFO, "(OC) Requested Realtime Memory Timing = %X\n", OcConfig->RealtimeMemoryTiming));
      MailboxWrite(MAILBOX_TYPE_PCODE, OC_LIB_CMD_SET_REALTIME_MEMORY_STATUS, OcConfig->RealtimeMemoryTiming, &LibStatus);
      if (LibStatus != EFI_SUCCESS) {
        DEBUG ((DEBUG_INFO, "(OC) Set Realtime Memory Timing Status failed, mailbox status = %X\n", LibStatus));
      }
    }

    ///
    /// We will loop on the CPU domains to manage the voltage/frequency settings
    ///
    for (DomainId = OC_LIB_DOMAIN_ID_GT; DomainId <= OC_LIB_DOMAIN_ID_SYSTEM_AGENT; DomainId++) {
      ///
      /// Only GT, Uncore, IOA, and IOD are valid for System Agent
      ///
      if (DomainId != OC_LIB_DOMAIN_ID_RING) {
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
            /// Need to populate the user requested settings from the policy
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
            switch (DomainId) {
              case OC_LIB_DOMAIN_ID_GT:
                RequestedVfItem.VfSettings.MaxOcRatio = (UINT8) OcConfig->GtMaxOcRatio;

                ///
                /// VoltageTarget has 2 uses and we need to update the target based
                /// on the voltagemode requested
                ///
                RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) OcConfig->GtVoltageMode;
                if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
                  RequestedVfItem.VfSettings.VoltageTarget = OcConfig->GtExtraTurboVoltage;
                }
                else {
                  RequestedVfItem.VfSettings.VoltageTarget = OcConfig->GtVoltageOverride;
                }
                RequestedVfItem.VfSettings.VoltageOffset = OcConfig->GtVoltageOffset;
                break;

              case OC_LIB_DOMAIN_ID_GT_UNSLICE:
                RequestedVfItem.VfSettings.MaxOcRatio = (UINT8) OcConfig->GtusMaxOcRatio;

                ///
                /// VoltageTarget has 2 uses and we need to update the target based
                /// on the voltagemode requested
                ///
                RequestedVfItem.VfSettings.VoltageTargetMode = (UINT8) OcConfig->GtusVoltageMode;
                if (RequestedVfItem.VfSettings.VoltageTargetMode == OC_LIB_OFFSET_ADAPTIVE) {
                  RequestedVfItem.VfSettings.VoltageTarget = OcConfig->GtusExtraTurboVoltage;
                }
                else {
                  RequestedVfItem.VfSettings.VoltageTarget = OcConfig->GtusVoltageOverride;
                }
                RequestedVfItem.VfSettings.VoltageOffset = OcConfig->GtusVoltageOffset;
                break;

              case OC_LIB_DOMAIN_ID_SYSTEM_AGENT:
                RequestedVfItem.VfSettings.VoltageOffset = OcConfig->SaVoltageOffset;
                break;
            }
            VfUpdateNeeded = (BOOLEAN) CompareMem ((VOID*)&RequestedVfItem,(VOID*)&CurrentVfItem,sizeof (VOLTAGE_FREQUENCY_ITEM));

            if (VfUpdateNeeded) {
              VfUpdateNeeded = FALSE;

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
                DEBUG ((DEBUG_WARN, "(OC) Set Voltage Frequency failed. EFI Status = %X, Library Status = %X\n", Status, LibStatus));
              }
            }
          }
          else {
            DEBUG ((DEBUG_INFO, "(OC) No OC support for this Domain = %X\n", DomainId));
          }
        }
        else {
          DEBUG ((DEBUG_WARN, "(OC) GetOcCapabilities message failed. Library Status = %X, Domain = %X\n", LibStatus, DomainId));
        }
      }
    }
  }
  ///
  /// BCLK overclocking is independent of OcSupport because it is a CSME flow.
  /// The BCLK flow should be executed regardless of overclocking support policy.
  ///
  Status = PerformBclkOcFlow();
  DEBUG((DEBUG_INFO, "(ICC) PerformBclkOcFlow status = %X.\n", Status));

  return Status;
}
