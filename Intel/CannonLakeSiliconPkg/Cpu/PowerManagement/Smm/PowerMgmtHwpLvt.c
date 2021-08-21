/** @file
  This is the SMM driver HWP SMI interrupt

@copyright
  INTEL CONFIDENTIAL
  Copyright 2011 - 2017 Intel Corporation.

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

#include <PchAccess.h>
#include <CpuAccess.h>
#include "PowerMgmtSmm.h"
#include <Private/PowerMgmtNvsStruct.h>
#include "CpuDataStruct.h"
#include <Protocol/SmmIoTrapDispatch2.h>
#include <Library/IoLib.h>

GLOBAL_REMOVE_IF_UNREFERENCED CPU_NVS_AREA *mCpuNvsAreaPtr;
STATIC UINT32 HwpSmiHandlerWasSet = 0;
/**
  Checks for a HWP interrupt by reading MSR.

  This function must be MP safe.

  @param[in] Buffer    Pointer to Buffer -return HWP status
**/
VOID
EFIAPI
HwpLvtEventCheckMsr (
  IN VOID *Buffer
  )
{
  UINT32  *HwpEvent;
  MSR_REGISTER   MsrData;
  ///
  /// Cast to enhance readability.
  ///
  HwpEvent = (UINT32 *) Buffer;
  MsrData.Qword = AsmReadMsr64 (MSR_IA32_HWP_STATUS);
  if (MsrData.Dwords.Low & (B_HWP_CHANGE_TO_GUARANTEED)) {
    *HwpEvent = 1;
  }
  return;
}

/**
  Clear HWP interrupt Status

  @param[in] Buffer
**/
VOID
EFIAPI
HwpLvtEventClearStatus (
  IN VOID *Buffer
  )
{
  MSR_REGISTER   MsrData;

  MsrData.Qword = AsmReadMsr64 (MSR_IA32_HWP_STATUS);
  MsrData.Dwords.Low &= (~(B_HWP_CHANGE_TO_GUARANTEED));
  AsmWriteMsr64 (MSR_IA32_HWP_STATUS,MsrData.Qword);
  return;
}

/**
  Generates a _GPE._L62 SCI to an ACPI OS.
**/
VOID
HwpSetSwGpeSts (
  VOID
  )
{
  ///
  /// Check SCI enable
  ///
  if (PmcIsSciEnabled ()) {
    ///
    /// Enable SCI assertion by setting SW GPE
    /// This will generate a SW SCI event in PCH
    ///
    PmcTriggerSwGpe ();
    ///
    /// Set Interrupt status to indicate HWP based interrupt
    ///
    mCpuNvsAreaPtr->HwpInterruptStatus = 1;
  }
}

/**
  Enable/Disable HWP Interrupt


  @param[in] Buffer    Pointer to Buffer -return HWP status
**/
VOID
EFIAPI
EnableDisableHwpInterrupt (
  IN VOID *HwpInterruptEnableFlag
  )
{
  MSR_REGISTER  Ia32HwpInterrupt;
  ///
  /// Set/Clear bits 0 and 1 of MSR IA32_HWP_INTERRUPT (773h) to enable/disable HWP interrupts
  ///
  Ia32HwpInterrupt.Qword = AsmReadMsr64 (MSR_IA32_HWP_INTERRUPT);
  if ( *((UINT32 *) HwpInterruptEnableFlag) == 1) {
    Ia32HwpInterrupt.Dwords.Low |= B_IA32_HWP_CHANGE_TO_GUARANTEED;
  } else {
    Ia32HwpInterrupt.Dwords.Low &= (~B_IA32_HWP_CHANGE_TO_GUARANTEED);
  }
  AsmWriteMsr64 (MSR_IA32_HWP_INTERRUPT, Ia32HwpInterrupt.Qword);
}

/**
  HWP EPP Disable
**/
VOID
EFIAPI
HwpEppDisable (
  VOID
  )
{
  MSR_REGISTER  MsrMiscPwrMgmt;
  ///
  /// Clear bit 12 of MSR MSR_MISC_PWR_MGMT (1AAh) to disable HWP EPP.
  /// Only need to clear bit 12 if bit 12 is already set to avoid redundant writes.
  ///
  MsrMiscPwrMgmt.Qword = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
  if (MsrMiscPwrMgmt.Dwords.Low & B_MISC_PWR_MGMT_ENABLE_HWP_EPP) {
    MsrMiscPwrMgmt.Dwords.Low &= (~B_MISC_PWR_MGMT_ENABLE_HWP_EPP);
    AsmWriteMsr64 (MSR_MISC_PWR_MGMT, MsrMiscPwrMgmt.Qword);
  }
}


/**
  SMI handler to handle HWP CPU Local APIC SMI
  for HWP LVT interrupt

  @param[in] SmmImageHandle        Image handle returned by the SMM driver.
  @param[in] CommunicationBuffer   Pointer to the buffer that contains the communication Message
  @param[in] SourceSize            Size of the memory image to be used for handler.

  @retval EFI_SUCCESS           Callback Function Executed
**/
EFI_STATUS
EFIAPI
HwpLvtSmiCallback (
  IN EFI_HANDLE  SmmImageHandle,
  IN CONST VOID  *ContextData,         OPTIONAL
  IN OUT VOID    *CommunicationBuffer, OPTIONAL
  IN OUT UINTN   *SourceSize           OPTIONAL
  )
{
  UINT32  HwpEvent;
  UINT32  HwpInterrupt;
  MSR_REGISTER   MsrData;

  ///
  /// Check first if HWP is enabled before changing HWP related MSR's
  ///
  MsrData.Qword = AsmReadMsr64 (MSR_IA32_PM_ENABLE);
  if ((MsrData.Dwords.Low & B_IA32_PM_ENABLE_HWP_ENABLE) && HwpSmiHandlerWasSet) {
    ///
    /// Check whether SMI is due to HWP interrupt.
    ///
    HwpEvent = 0;
    RunOnAllLogicalProcessors (HwpLvtEventCheckMsr, &HwpEvent);

    if (HwpEvent == 1) {
      ///
      /// Disable HWP Interrupts
      ///
      HwpInterrupt = 0;
      RunOnAllLogicalProcessors (EnableDisableHwpInterrupt,&HwpInterrupt);
      ///
      /// If HWP interrupt Assert SCI
      ///
      HwpSetSwGpeSts ();
      ///
      /// Clear the HWP Interrupt Status
      ///
      RunOnAllLogicalProcessors (HwpLvtEventClearStatus,NULL);
      ///
      /// Clear EPP Flag
      ///
      HwpEppDisable ();
      ///
      /// Enable HWP Interrupts
      ///
      HwpInterrupt = 1;
      RunOnAllLogicalProcessors (EnableDisableHwpInterrupt,&HwpInterrupt);
    }
  }

  return EFI_SUCCESS;
}

/**
  Enable xApic LVT Thermal interrupt

  @param[in] Buffer    Pointer to Buffer -return HWP status
**/
VOID
EFIAPI
EnableXapicLvt (
  IN VOID *Buffer
  )
{
  UINT32                      ApicThermalValue;
  BOOLEAN                     x2ApicEnabled;
  MSR_IA32_APIC_BASE_REGISTER Msr;

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_APIC_BASE);
  x2ApicEnabled = (BOOLEAN) ((Msr.Bits.EXTD == 1) && (Msr.Bits.EN == 1));

  ///
  /// Configure the Local APIC to generate an SMI on Thermal events.  First,
  /// Clear BIT16, BIT10-BIT8, BIT7-BIT0.  Then, set BIT9 (delivery mode).
  /// Don't enable the interrupt if it's already enabled
  ///
  if (x2ApicEnabled) {
    ApicThermalValue = (UINT32) AsmReadMsr64 (MSR_IA32_X2APIC_LVT_THERMAL);
  } else {
    ApicThermalValue = *(UINT32 *) (UINTN) MMIO_LOCAL_APIC_THERMAL_DEF;
  }

  if ((ApicThermalValue & (B_INTERRUPT_MASK | B_DELIVERY_MODE | B_VECTOR)) != V_MODE_SMI) {
    HwpSmiHandlerWasSet = TRUE;
    ApicThermalValue = (ApicThermalValue &~(B_INTERRUPT_MASK | B_DELIVERY_MODE | B_VECTOR)) | V_MODE_SMI;
    if (x2ApicEnabled) {
      AsmWriteMsr64 (MSR_IA32_X2APIC_LVT_THERMAL, ApicThermalValue);
    } else {
      *(UINT32 *) (UINTN) (MMIO_LOCAL_APIC_THERMAL_DEF) = (UINT32) ApicThermalValue;
    }
  }
}

/**
  Software SMI callback for TPM physical presence which is called from ACPI method.

  Caution: This function may receive untrusted input.
  Variable and ACPINvs are external input, so this function will validate
  its data structure to be valid value.

  @param[in]      DispatchHandle  The unique handle assigned to this handler by SmiHandlerRegister().
  @param[in]      Context         Points to an optional handler context which was specified when the
                                  handler was registered.

  @retval EFI_SUCCESS             The interrupt was handled successfully.
**/
EFI_STATUS
EFIAPI
HwpNotificationCallback (
  IN EFI_HANDLE                  DispatchHandle,
  IN EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext
  )
{

  RunOnAllLogicalProcessors (EnableXapicLvt, NULL);

  return EFI_SUCCESS;
}

/**
  Initialize the HWP SMI Handler and IO trap.
  @retval EFI_SUCCESS   The driver installes/initialized correctly.
  @retval EFI_NOT_FOUND CPU Data HOB not available.
**/
EFI_STATUS
EFIAPI
InitPowerMgmtHwpLvt (
   VOID
  )
{
  EFI_STATUS             Status;
  EFI_HANDLE             Handle;
  EFI_SMM_SW_DISPATCH2_PROTOCOL  *SwDispatch;
  EFI_SMM_SW_REGISTER_CONTEXT    SwContext;
  EFI_HANDLE                     SwHandle;
  CPU_NVS_AREA_PROTOCOL   *CpuNvsAreaProtocol;

  ///
  /// Locate Cpu Nvs area
  ///
  Status = gBS->LocateProtocol (&gCpuNvsAreaProtocolGuid, NULL, (VOID **) &CpuNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);
  mCpuNvsAreaPtr = CpuNvsAreaProtocol->Area;

  ///
  /// Register an SMI handler to handle the Thermal LVT SMI events
  ///
  if ((mCpuNvsAreaPtr->PpmFlags & PPM_HWP) && (mCpuNvsAreaPtr->PpmFlags & PPM_HWP_LVT)) {
    Status = gSmst->SmiHandlerRegister (HwpLvtSmiCallback, NULL, &Handle);
    ASSERT_EFI_ERROR (Status);
  }

  ///
  /// Register HWP notifications SMI handler
  ///
  Status = gSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, (VOID**)&SwDispatch);
  ASSERT_EFI_ERROR (Status);
  SwContext.SwSmiInputValue = mCpuNvsAreaPtr->HwpSmi;
  Status = SwDispatch->Register (SwDispatch, (EFI_SMM_HANDLER_ENTRY_POINT2) HwpNotificationCallback, &SwContext, &SwHandle);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}
