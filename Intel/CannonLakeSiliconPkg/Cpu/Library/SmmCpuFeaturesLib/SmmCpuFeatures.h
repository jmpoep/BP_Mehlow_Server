/** @file
  The CPU specific programming for PiSmmCpuDxeSmm module.


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

#ifndef __SMM_FEATURES_H__
#define __SMM_FEATURES_H__

#include <PiSmm.h>

#include <Protocol/SmmAccess2.h>

#include <Library/SmmCpuFeaturesLib.h>

#include <Library/PcdLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/LocalApicLib.h>
#include <Library/SynchronizationLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MtrrLib.h>
#include <Library/SocketLga775Lib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/CpuPlatformLib.h>
#include <Register/Cpuid.h>
#include <Library/HobLib.h>
#include <SaDataHob.h>

#include "SmmPpam.h"

////////
// Below definition is from IA32 SDM
////////
#define EFI_MSR_IA32_MTRR_CAP                  0xFE
#define  IA32_MTRR_SMRR_SUPPORT_BIT            BIT11
#define  IA32_MTRR_SMRR2_SUPPORT_BIT           BIT13

#define EFI_MSR_IA32_FEATURE_CONTROL           0x3A
#define  IA32_SMRR_ENABLE_BIT                  BIT3

#define  EFI_MSR_SMRR_PHYS_MASK_VALID          BIT11
#define  EFI_MSR_SMRR_MASK                     0xFFFFF000

#define EFI_MSR_NEHALEM_SMRR_PHYS_BASE         0x1F2
#define EFI_MSR_NEHALEM_SMRR_PHYS_MASK         0x1F3

#define   CPUID1_EDX_XD_SUPPORT      0x100000

#define CR0_WP                BIT16

////////
// Below definition is from CPU BWG
////////
#define EFI_MSR_HASWELL_SMRR2_PHYS_BASE        0x1F6
#define EFI_MSR_HASWELL_SMRR2_PHYS_MASK        0x1F7

#define EFI_MSR_HASWELL_SMM_MCA_CAP            0x17D
#define  SMM_PROT_MODE_BASE_BIT                BIT54
#define  TARGETED_SMI_BIT                      BIT56
#define  SMM_CPU_SVRSTR_BIT                    BIT57
#define  SMM_CODE_ACCESS_CHK_BIT               BIT58
#define  LONG_FLOW_INDICATION_BIT              BIT59

#define EFI_MSR_HASWELL_SMM_FEATURE_CONTROL    0x4E0
#define  SMM_FEATURE_CONTROL_LOCK_BIT          BIT0
#define  SMM_CPU_SAVE_EN_BIT                   BIT1
#define  SMM_CODE_CHK_EN_BIT                   BIT2

#define EFI_MSR_HASWELL_EVENT_CTL_HLT_IO       0xC1F
#define EFI_MSR_HASWELL_SMBASE                 0xC20
#define EFI_MSR_HASWELL_SMMREVID               0xC21
#define EFI_MSR_HASWELL_RIP                    0xC04
#define EFI_MSR_HASWELL_EFER                   0xC03

#define EFI_MSR_HASWELL_SMM_ENABLE             0x4E1

#define EFI_MSR_HASWELL_SMM_DELAYED            0x4E2

#define EFI_MSR_HASWELL_SMM_BLOCKED            0x4E3

#define EFI_MSR_HASWELL_IEDBASE                0xC22

#define EFI_MSR_SMM_SAVE_CONTROL               0x3e
#define SAVE_FLOATING_POINT_POINTERS           BIT0

#define EFI_MSR_THREAD_ID_INFO                 0x53

#define EFI_MSR_SMM_PROT_MODE_BASE             0x9D
#define  SMM_PROT_MODE_BASE_ENABLE_BIT         BIT0
#define  SMM_PROT_MODE_BASE_PADDR_MASK         0x00000000FFFFFF00ULL

#define PLATFORM_INFO_SMM_SAVE_CONTROL         BIT16

#define EFI_MSR_SMM_SUPOVR_STATE_LOCK          0x141
#define   EFI_MSR_SMM_SUPOVR_STATE_LOCK_PAGING_STATE_ENABLE_BIT  BIT0
#define   EFI_MSR_SMM_SUPOVR_STATE_LOCK_SMBASE_ENABLE_BIT        BIT1

#define SMM_NEHALEM_IEDBASE_OFFSET             0xFEEC

#define MSR_IA32_RTIT_CTL                      0x00000570
#define B_RTIT_CTL_TRACE_ENABLE                BIT0
#define B_RTIT_CAPABLE                         BIT25

#define IED_STRING                             "INTEL RSVD"

//
// Define for Protected Mode SMM Entry Template
//
#define SMMSEG_PROTECT_MODE_CODE_SEGMENT   0x10

#pragma pack(1)
typedef struct {
  UINT16  Limit;
  UINT32  Base;
} IA32_PROT_DESCRIPTOR;
#pragma pack()

////////
// Below section is definition for CPU SMM Feature context
////////


//
// Structure to describe CPU_SMM_FEATURE_CONTEXT
//
typedef struct {
  BOOLEAN          SmrrEnabled;
  BOOLEAN          Smrr2Enabled;
} CPU_SMM_FEATURE_CONTEXT;

//
// SMMSEG_FEATURE_ENABLES bitmap
//
#define SMMSEG_FEATURE_ENABLE                  BIT0
#define SMMSEG_FEATURE_CR4_MCE_CTL_ENABLE      BIT1

//
// Structure to describe CPU SMM Protected Mode
//
typedef struct {
  UINT32  Reserved1;
  UINT32  SmmSegFeatureEnables;   // SMM_SEG_FEATURE_ENABLES
  UINT32  GDTRLimit;
  UINT32  GDTRBaseOffset;
  UINT32  CSSelector;
  UINT32  Reserved3;
  UINT32  ESPOffset;
  UINT32  Reserved4;
  UINT32  IDTRLimit;
  UINT32  IDTRBaseOffset;
} SMMSEG;

#define  SMM_PROT_MODE_GDT_ENTRY_COUNT    27

//
// SMM PROT MODE CONTEXT (total 0x100 bytes)
//
typedef struct {
  SMMSEG                    SmmProtectedModeSMMSEG;                       // 40 bytes
  IA32_SEGMENT_DESCRIPTOR   SmmProtectedModeGdt[SMM_PROT_MODE_GDT_ENTRY_COUNT];  // 27 * 8 = 216 bytes
} SMM_PROT_MODE_CONTEXT;


//

//
// SMM CPU synchronization features available on a processor
//
typedef struct {
  BOOLEAN          TargetedSmiSupported;
  BOOLEAN          DelayIndicationSupported;
  BOOLEAN          BlockIndicationSupported;
  //
  // This processor's LOG_PROC_EN bit used in SMM_ENABLE, SMM_DELAYED, and SMM_BLOCKED MSRs
  // (introduced in Haswell processor).
  // Value of "-1" indicates this field is invalid (i.e. LOG_PROC_EN bit is not
  // supported)
  //
  UINT64           HaswellLogProcEnBit;
} SMM_CPU_SYNC_FEATURE;

///
///
///
extern BOOLEAN    mSaveStateInMsr;
extern BOOLEAN    mSmmInitSaveStateInMsr;
extern UINT8      mSmmFeatureSaveStateRegisterLma;
extern SPIN_LOCK  *mMsrSpinLock;
extern BOOLEAN    mSmmPpamEnable;
extern BOOLEAN    mSmmProcTraceEnable;
extern UINT64     gSmmSupovrStateLockData;

/**
  Read MSR or CSR based on the CPU type Register to read.

  NOTE: Since platform may uses I/O ports 0xCF8 and 0xCFC to access
        CSR, we need to use SPIN_LOCK to avoid collision on MP System.

  @param[in]  CpuIndex  The processor index.
  @param[in]  RegName   Register name.

  @return 64-bit value read from register.

**/
UINT64
SmmReadReg64 (
  IN  UINTN           CpuIndex,
  IN  SMM_REG_NAME    RegName
  );

/**
  Write MSR or CSR based on the CPU type Register to write.

  NOTE: Since platform may uses I/O ports 0xCF8 and 0xCFC to access
        CSR, we need to use SPIN_LOCK to avoid collision on MP System.

  @param[in]  CpuIndex  The processor index.
  @param[in]  RegName   Register name.
  @param[in]  RegValue  64-bit Register value.

**/
VOID
SmmWriteReg64 (
  IN  UINTN           CpuIndex,
  IN  SMM_REG_NAME    RegName,
  IN  UINT64          RegValue
  );

/**
  Initialize SMM Protected Mode IDT table.

  @param[in]  Cr3      CR3 value.
**/
VOID
InitProtectedModeIdt (
  IN UINT32  Cr3
  );

/**
  Allocate pages for code.

  @param  Pages Number of pages to be allocated.

  @return Allocated memory.
**/
VOID *
SmmFeatureAllocateCodePages (
  IN UINTN           Pages
  );

/**
  This function sets SmmSupovrStateLock MSR.

  NOTE: This function does not take effect before
  SmmCpuFeaturesCompleteSmmReadyToLock().
**/
VOID
EFIAPI
SmmSupovrStateLock (
  VOID
  );

/**
  This function fixes up the address of the global variable or function
  referred in SmiEntry assembly files to be the absoute address.
**/
VOID
EFIAPI
SmmCpuFeaturesLibSmiEntryFixupAddress (
  );

#endif
