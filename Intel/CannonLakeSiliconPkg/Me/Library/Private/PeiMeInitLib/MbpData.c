/** @file
  ME PayLoad Data implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2018 Intel Corporation.

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
#include <PiPei.h>
#include <Library/TimerLib.h>
#include <Library/DebugLib.h>
#include <Library/PerformanceLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <MeChipset.h>
#include <HeciRegs.h>
#include <PchAccess.h>
#include <IndustryStandard/Pci22.h>
#include <Ppi/SiPolicy.h>
#include <Library/MeTypeLib.h>
#include <Library/PeiMeLib.h>
#include <Ppi/MbpSensitivePpi.h>
#include <MeBiosPayloadHob.h>
#include <BupMsgs.h>
#include "MbpData.h"
#include <Library/PostCodeLib.h>

#define EVENT_REC_TOK                   "EventRec"

//
// Function Declarations
//
static MBP_SENSITIVE_PPI               mMbpSensitivePpi = {
  MBP_SENSITIVE_PPI_REVISION,
  {{{ 0 }}}
};

static EFI_PEI_PPI_DESCRIPTOR mInstallMbpSensitivePpi = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gMbpSensitivePpiGuid,
  &mMbpSensitivePpi
};

/**
  Install MbpData protocol.

  @param[in]  HeciPpi                 The pointer with Heci Ppi
  @param[out] MbpHobPtr               The pointer with MbpHob installed.
**/
VOID
InstallMbp (
  IN HECI_PPI                         *HeciPpi,
  OUT ME_BIOS_PAYLOAD_HOB             **MbpHobPtr
  )
{
  EFI_STATUS                          Status;
  ME_BIOS_PAYLOAD                     MbpData;
  ME_BIOS_PAYLOAD_SENSITIVE           MbpSensitiveData;
  ME_BIOS_PAYLOAD_HOB                 *MbpHob;


  if (MeTypeIsSps ()) {
    DEBUG ((DEBUG_INFO, "[SPS] Skip MBP initialization for SPS firmware\n"));
    return;
  }

  DEBUG((DEBUG_INFO, "ME-BIOS: MBP Entry.\n"));
  PostCode (0xE02);

  PERF_START_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8010);
  Status = PrepareMeBiosPayload (&MbpData, &MbpSensitiveData);
  PERF_END_EX (NULL, EVENT_REC_TOK, NULL, AsmReadTsc (), 0x8011);

  if (EFI_ERROR (Status)) {
    DEBUG((DEBUG_ERROR, "ME-BIOS: MBP Exit - Error by PrepareMeBiosPayload fail, Status: %r\n", Status));
    PostCode (0xE82);
    return;
  }

  if (!MeMbpHobSkipped ()) {
    ///
    /// Create MBP HOB
    ///
    Status = PeiServicesCreateHob (
               EFI_HOB_TYPE_GUID_EXTENSION,
               sizeof (ME_BIOS_PAYLOAD_HOB),
               (VOID**) &MbpHob
               );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "MeBiosPayloadHob could not be created.\n"));
      DEBUG ((DEBUG_ERROR, "ME-BIOS: MBP Exit - Error by MeBiosPayloadHob create fail, Status: %r\n", Status));
      PostCode (0xEA2);
      return;
    }

    //
    // Initialize MBP HOB
    //
    MbpHob->Header.Name   = gMeBiosPayloadHobGuid;
    MbpHob->Revision      = MBP_DATA_PROTOCOL_REVISION;
    CopyMem (&MbpHob->MeBiosPayload, &MbpData, sizeof (ME_BIOS_PAYLOAD));

    //
    // Check if selected members retrieved from FW are up to date. If not - send additional synch messages.

    if (MbpHob->MeBiosPayload.FwCapsSku.Available == FALSE) {
      //
      // FW Capabilities
      //
      Status = PeiHeciGetFwCapsSkuMsg (&MbpHob->MeBiosPayload.FwCapsSku.FwCapabilities);
      if (!EFI_ERROR (Status)) {
        MbpHob->MeBiosPayload.FwCapsSku.Available = TRUE;
      }
    }
    if (MbpHob->MeBiosPayload.FwFeaturesState.Available == FALSE) {
      Status = PeiHeciGetFwFeatureStateMsg (&MbpHob->MeBiosPayload.FwFeaturesState.FwFeatures);
      //
      // FW Features State
      //
      if (!EFI_ERROR (Status)) {
        MbpHob->MeBiosPayload.FwFeaturesState.Available = TRUE;
      }
    }
    if (MbpHob->MeBiosPayload.FwPlatType.Available == FALSE) {
      //
      // ME Platform Type
      //
      Status = PeiHeciGetPlatformTypeMsg (&MbpHob->MeBiosPayload.FwPlatType.RuleData);
      if (!EFI_ERROR (Status)) {
        MbpHob->MeBiosPayload.FwPlatType.Available = TRUE;
      }
    }

    //
    // Initialize MBP sensitive data PPI
    //
    CopyMem (&mMbpSensitivePpi.MbpSensitiveData, &MbpSensitiveData, sizeof (ME_BIOS_PAYLOAD_SENSITIVE));

    //
    // Install MBP sensitive data PPI
    //
    Status = PeiServicesInstallPpi (&mInstallMbpSensitivePpi);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "ME-BIOS: MBP Exit - Error by MbpSensitivePpi install fail, Status: %r\n", Status));
      PostCode (0xEC2);
      return;
    }
    *MbpHobPtr = MbpHob;

    //
    // Dump the Mbp data
    //
    DEBUG_CODE (
      MbpDebugPrint (MbpHob->Revision, mMbpSensitivePpi.Revision, &MbpData, &MbpSensitiveData);
      );

  }

  DEBUG ((DEBUG_INFO, "ME-BIOS: MBP Exit - Success.\n"));
  PostCode (0xE22);
}

/**
  This routine returns ME-BIOS Payload information.

  @param[out] MbpPtr              ME-BIOS Payload information.

  @retval EFI_SUCCESS             The function completed successfully.
  @retval EFI_DEVICE_ERROR        Failed to consume MBP
  @retval EFI_OUT_OF_RESOURCES    Out of resource
**/

EFI_STATUS
PrepareMeBiosPayload (
  OUT  ME_BIOS_PAYLOAD                *MbpPtr,
  OUT  ME_BIOS_PAYLOAD_SENSITIVE      *MbpSensitivePtr
  )
{
  EFI_STATUS                          Status;
  MBP_HEADER                          MbpHeader;
  UINT32                              *MbpItems;
  MBP_ITEM_HEADER                     MbpItemHeader;
  UINT32                              *Ptr;
  UINT32                              MbpItemId;
  UINT32                              MbpItemSize;
  UINT32                              i;
  UINT32                              Index;
  VOID                                *DestPtr;
  UINTN                               DestSize;
  UINT32                              *MbpItemPtr;
  HECI_HOST_CONTROL_REGISTER          HeciRegHCsr;
  volatile HECI_HOST_CONTROL_REGISTER *HeciRegHCsrPtr;
  UINT64                              HeciBaseAddress;
  BOOLEAN                             SkipMbp;

  DEBUG ((DEBUG_INFO, "PrepareMeBiosPayload () - Start\n"));

  MbpItems = AllocateZeroPool (MAX_MBP_SIZE);
  if (MbpItems == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SkipMbp = MeMbpHobSkipped ();
  Status = PeiHeciGetMbpMsg (&MbpHeader, MbpItems, SkipMbp);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "HECI get MBP error: %r!\n", Status));
    return Status;
  }
  if (SkipMbp) {
    DEBUG ((DEBUG_WARN, "Skipping MBP data due to SkipMbpHob set!\n"));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "HECI get MBP succeeded\n"));
  DEBUG ((DEBUG_INFO, "MBP header: %x\n", MbpHeader.Data));
  DEBUG ((DEBUG_INFO, "MbpSize: %x\n", MbpHeader.Fields.MbpSize));
  DEBUG ((DEBUG_INFO, "No. of Mbp Entries: %x\n", MbpHeader.Fields.NumEntries));
  DEBUG ((DEBUG_INFO, "Mbp flags: %x\n", MbpHeader.Fields.Flags));

  MbpPtr->FwCapsSku.Available                   = FALSE;
  MbpPtr->FwPlatType.Available                  = FALSE;
  MbpPtr->HwaRequest.Available                  = FALSE;
  MbpPtr->PlatBootPerfData.Available            = FALSE;
  MbpPtr->HwaRequest.Data.Fields.MediaTablePush = FALSE;
  MbpPtr->FwFeaturesState.Available             = FALSE;
  MbpPtr->UnconfigOnRtcClearState.Available     = FALSE;
  MbpPtr->ArbSvnState.Available                 = FALSE;

  MbpItemPtr = MbpItems;
  Ptr        = NULL;

  for (i = 0; i < MbpHeader.Fields.NumEntries; i++) {
    MbpItemHeader.Data = *MbpItemPtr;
    MbpItemPtr++;
    DEBUG ((DEBUG_INFO, "MBP Item %x header: %08x\n", i + 1, MbpItemHeader.Data));

    MbpItemId = (MbpItemHeader.Fields.AppId << 4) | MbpItemHeader.Fields.ItemId;
    //
    // Copy data from HECI buffer per size of each MBP item
    //
    MbpItemSize = MbpItemHeader.Fields.Length - 1;
    Ptr         = AllocateZeroPool ((MbpItemSize) * 4);
    ASSERT (Ptr);
    if (Ptr == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    for (Index = 0; Index < MbpItemSize; Index++) {
      Ptr[Index] = *MbpItemPtr;
      MbpItemPtr++;
      DEBUG ((DEBUG_INFO, "MBP Item Data: %08x\n", Ptr[Index]));
    }

    switch (MbpItemId) {
      //
      // Insensitive items
      //
      case 0x11:
        //
        // FW Version Name
        //
        DestSize = sizeof (MbpPtr->FwVersionName);
        DestPtr = &MbpPtr->FwVersionName;
        break;

      case 0x12:
        //
        // FW Capabilities
        //
        DestSize = sizeof (MbpPtr->FwCapsSku.FwCapabilities);
        DestPtr = &MbpPtr->FwCapsSku.FwCapabilities;
        MbpPtr->FwCapsSku.Available = TRUE;
        break;

      case 0x15:
        //
        // ME Platform Type
        //
        DestSize = sizeof (MbpPtr->FwPlatType.RuleData);
        DestPtr = &MbpPtr->FwPlatType.RuleData;
        MbpPtr->FwPlatType.Available = TRUE;
        break;

      case 0x17:
        ///
        /// Platform timing information
        ///
        DestSize = sizeof (MbpPtr->PlatBootPerfData.MbpPerfData);
        DestPtr = &MbpPtr->PlatBootPerfData.MbpPerfData;
        MbpPtr->PlatBootPerfData.Available = TRUE;
        break;

      case 0x18:
        //
        // ME Unconfiguration on RTC clear state
        //
        DestSize = sizeof (MbpPtr->UnconfigOnRtcClearState.UnconfigOnRtcClearData);
        DestPtr = &MbpPtr->UnconfigOnRtcClearState.UnconfigOnRtcClearData;
        MbpPtr->UnconfigOnRtcClearState.Available = TRUE;
        break;

      case 0x19:
        //
        // FW Features State
        //
        DestSize = sizeof (MbpPtr->FwFeaturesState.FwFeatures);
        DestPtr = &MbpPtr->FwFeaturesState.FwFeatures;
        MbpPtr->FwFeaturesState.Available = TRUE;
        break;

      case 0x1E:
        //
        // ARB SVN State
        //
        DestSize = sizeof (MbpPtr->ArbSvnState.ArbSvnData);
        DestPtr = &MbpPtr->ArbSvnState.ArbSvnData;
        MbpPtr->ArbSvnState.Available = TRUE;
        break;

      case 0x41:
        //
        // HWA Request
        //
        DestSize = sizeof (MbpPtr->HwaRequest.Data);
        DestPtr = &MbpPtr->HwaRequest.Data;
        MbpPtr->HwaRequest.Available = TRUE;
        break;

      case 0x51:
        //
        // ICC Profile
        //
        DestSize = sizeof (MbpPtr->IccProfile);
        DestPtr = &MbpPtr->IccProfile;
        break;

      case 0x53:
        //
        // ChipsetInit Version
        //
        DestSize = sizeof (MbpPtr->ChipsetInitVerData);
        DestPtr = &MbpPtr->ChipsetInitVerData;
        break;

        //
        // Sensitive items
        //
      case 0x14:
        //
        // Platform Key
        //
        DestSize = sizeof (MbpSensitivePtr->PlatformKey);
        DestPtr = &MbpSensitivePtr->PlatformKey;
        break;

      default:
        DEBUG ((DEBUG_WARN, "Unknown MbpItemId: %08x.\n", MbpItemId));
        //
        // default case
        //
        DestSize = 0;
        DestPtr = NULL;
        break;
    }
    if (DestPtr != NULL) {
      if ((MbpItemSize * 4) <= DestSize) {
        CopyMem (DestPtr, Ptr, (MbpItemSize * 4));
      } else {
        DEBUG ((DEBUG_INFO, "Data size is larger than destination buffer. This item is not copied.\n"));
      }
      //
      // Clear buffer
      //
      ZeroMem (Ptr, MbpItemSize * 4);
    }
  }

  //
  // Initialize memory mapped register pointers
  //
  HeciBaseAddress   = PCI_SEGMENT_LIB_ADDRESS (ME_SEGMENT, ME_BUS, ME_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, 0);
  HeciRegHCsrPtr    = (VOID *) (UINTN) ((PciSegmentRead32 (HeciBaseAddress + PCI_BASE_ADDRESSREG_OFFSET) & 0xFFFFFFF0) + H_CSR);

  //
  // Interrupt ME FW so FW knows we're done with MBP read attempt and set H_RST for ME to clear the buffer
  //
  HeciRegHCsr.ul      = HeciRegHCsrPtr->ul;
  HeciRegHCsr.r.H_IG  = 1;
  HeciRegHCsr.r.H_RST = 1;
  HeciRegHCsrPtr->ul  = HeciRegHCsr.ul;

  return Status;
}

