/** @file
  PEI DCI Init library.

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

#include <Base.h>
#include <Uefi/UefiBaseType.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Register/PchRegsDci.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <PchResetPlatformSpecific.h>
#include <Library/PostCodeLib.h>
#include <Ppi/SiPolicy.h>

/**
  Perform DCI configuration.

  @param[in] SiPreMemPolicyPpi          The SI PREMEM Policy PPI instance
**/
VOID
DciConfiguration (
  IN  SI_PREMEM_POLICY_PPI             *SiPreMemPolicyPpi
  )
{
  EFI_STATUS                            Status;
  PCH_DCI_PREMEM_CONFIG                 *DciPreMemConfig;
  UINT32                                Data32And;
  UINT32                                Data32Or;
  PCH_RESET_DATA                        ResetData;
  UINT32                                EctrlOrg;
  UINT32                                EctrlNew;

  Data32And = ~0u;
  Data32Or = 0;
  EctrlOrg = PchPcrRead32 (PID_DCI, R_DCI_PCR_ECTRL);

  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gDciPreMemConfigGuid, (VOID *) &DciPreMemConfig);
  ASSERT_EFI_ERROR (Status);


  if ((PchGeneration () == CNL_PCH) && IsPchLp () && (PchStepping () < PCH_B0)) {
    ///
    /// Set PCR[DCI] + 30h bit[5] to 1, and clear 30h [2,1,0] = 0
    ///
    PchPcrAndThenOr32 (
      PID_DCI, R_DCI_PCR_PCE,
      (UINT32) ~(B_DCI_PCR_PCE_D3HE | B_DCI_PCR_PCE_I3E | B_DCI_PCR_PCE_PMCRE),
      B_DCI_PCR_PCE_HAE
      );
    ///
    /// BIOS need to set DCI PCR offset 0x08[19,18,17,16]=0
    ///
    PchPcrAndThenOr32 (PID_DCI, R_DCI_PCR_ECKPWRCTL, (UINT32) ~(BIT19 | BIT18 | BIT17 | BIT16), 0);
  } else {
    ///
    /// Configure USB DBC mode according to debug probe type
    ///
    switch (DciPreMemConfig->PlatformDebugConsent) {
      case ProbeTypeDciOob:
      case ProbeTypeXdp3:
        if (!PcdGetBool (PcdSimicsEnable)) {
          //
          // Disable USB DbC
          //
          Data32And &= (UINT32)~(B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN);
        }
        break;

      case ProbeTypeDciOobDbc:
      case ProbeTypeUsb2Dbc:
        //
        // Enable USB2 DbC
        //
        Data32Or |= (B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN); // for USB2DBC = 1 and USB3DBC = 0 is the illigal case, USB3DBC must be set
        break;

      case ProbeTypeUsb3Dbc:
        //
        // Enable USB3 DbC
        //
        Data32And &= (UINT32)~(B_DCI_PCR_ECTRL_USB2DBCEN);
        Data32Or |= B_DCI_PCR_ECTRL_USB3DBCEN;
        break;

      case ProbeTypeDisabled:
        //
        // Comply with HW value, keep it unchanged
        //
        Data32And &= (UINT32)~(B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN);
        Data32Or |= (EctrlOrg & (B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN));
        break;

      default:
        //
        // Keep silicon register defaults
        //
        break;
    }

    switch (DciPreMemConfig->DciUsb3TypecUfpDbg) {
      case Usb3TcDbgDisabled:
        Data32And &= (UINT32)~B_DCI_PCR_ECTRL_UFP2DFP;
        break;

      case Usb3TcDbgEnabled:
        Data32Or |= B_DCI_PCR_ECTRL_UFP2DFP;
        break;

      case Usb3TcDbgNoChange:
        Data32And &= (UINT32)~B_DCI_PCR_ECTRL_UFP2DFP;
        Data32Or |= (EctrlOrg & B_DCI_PCR_ECTRL_UFP2DFP);
        break;

      default:
        break;
    }
  }
  ///
  /// Configure PLATFORM_DEBUG_EN_USER, set DCI_EN for user opt-in debug enabled
  ///
  if (DciPreMemConfig->PlatformDebugConsent != ProbeTypeDisabled) {
    Data32Or |= B_DCI_PCR_ECTRL_HDCIEN;
  } else {
    Data32And &= (UINT32)~B_DCI_PCR_ECTRL_HDCIEN;
  }

  PchPcrAndThenOr32 (PID_DCI, R_DCI_PCR_ECTRL, Data32And, Data32Or);
  ///
  /// Set DCI EN Lock
  ///
  PchPcrAndThenOr32 (PID_DCI, R_DCI_PCR_ECTRL, ~0u, B_DCI_PCR_ECTRL_HDCIEN_LOCK);

  EctrlNew = PchPcrRead32 (PID_DCI, R_DCI_PCR_ECTRL);
  if (!PcdGetBool (PcdSimicsEnable)) {
    ///
    /// if DBG_EN bit is inconsistent from previous boot, which is reflection of HEEN, trigger global reset to ensure ePOC update at the same time.
    /// if USB2DBCEN / USB2DBCEN / UFP2DFP bits are set differently from previous boot, trigger global reset to ensure PMC update at the same time.
    ///
    if ((EctrlOrg & (B_DCI_PCR_ECTRL_DBG_EN | B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN | B_DCI_PCR_ECTRL_UFP2DFP)) !=
        (EctrlNew & (B_DCI_PCR_ECTRL_DBG_EN | B_DCI_PCR_ECTRL_USB2DBCEN | B_DCI_PCR_ECTRL_USB3DBCEN | B_DCI_PCR_ECTRL_UFP2DFP))) {
      PostCode (0xB4B);
      DEBUG ((DEBUG_INFO, "ECTRL Old = 0x%08x\nECTRL New = 0x%08x\n", EctrlOrg, EctrlNew));
      DEBUG ((DEBUG_INFO, "PchDciConfiguration - global reset due to user opt-in debug state change.\n"));
      CopyMem (&ResetData.Guid, &gPchGlobalResetGuid, sizeof (EFI_GUID));
      StrCpyS (ResetData.Description, PCH_RESET_DATA_STRING_MAX_LENGTH, PCH_PLATFORM_SPECIFIC_RESET_STRING);
      (*GetPeiServicesTablePointer ())->ResetSystem2 (EfiResetPlatformSpecific, EFI_SUCCESS, sizeof (PCH_RESET_DATA), &ResetData);
    }
  }
}