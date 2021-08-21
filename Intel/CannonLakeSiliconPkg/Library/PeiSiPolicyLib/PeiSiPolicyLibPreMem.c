/** @file
  This file is PeiSiPolicyLib library creates default settings of RC
  Policy and installs RC Policy PPI.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2017 Intel Corporation.

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
#include "PeiSiPolicyLibrary.h"
#include <Base.h>
#include <Private/Library/PeiCpuAndPchTraceHubLib.h>

/**
  SiCreatePreMemConfigBlocks creates the config blocksg of Silicon PREMEM Policy.
  It allocates and zero out buffer, and fills in the Intel default settings.

  @param[out] SiPreMemPolicyPpi   The pointer to get Silicon Policy PPI instance

  @retval EFI_SUCCESS             The policy default is initialized.
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiCreatePreMemConfigBlocks (
  OUT  SI_PREMEM_POLICY_PPI **SiPreMemPolicyPpi
  )
{
  UINT16               TotalBlockSize;
  EFI_STATUS           Status;
  SI_PREMEM_POLICY_PPI *SiPreMemPolicy;
  UINT16               RequiredSize;

  SiPreMemPolicy = NULL;
  //
  // TotalBlockSize = Pch , SA, ME and CPU config block size.
  //
  TotalBlockSize = PchGetPreMemConfigBlockTotalSize () +
                   MeGetConfigBlockTotalSizePreMem () +
                   SaGetConfigBlockTotalSizePreMem () +
                   CpuGetPreMemConfigBlockTotalSize ();
  DEBUG ((DEBUG_INFO, "TotalBlockSize = 0x%x\n", TotalBlockSize));

  RequiredSize = sizeof (CONFIG_BLOCK_TABLE_HEADER) + TotalBlockSize;

  Status = CreateConfigBlockTable (RequiredSize, (VOID *)&SiPreMemPolicy);
  ASSERT_EFI_ERROR (Status);

  //
  // General initialization
  //
  SiPreMemPolicy->TableHeader.Header.Revision = SI_PREMEM_POLICY_REVISION;
  //
  // Add config blocks.
  //
  Status = PchAddPreMemConfigBlocks ((VOID *) SiPreMemPolicy);
  ASSERT_EFI_ERROR (Status);
  Status = MeAddConfigBlocksPreMem ((VOID *) SiPreMemPolicy);
  ASSERT_EFI_ERROR (Status);
  Status = SaAddConfigBlocksPreMem ((VOID *) SiPreMemPolicy);
  ASSERT_EFI_ERROR (Status);
  Status = CpuAddPreMemConfigBlocks ((VOID *) SiPreMemPolicy);
  ASSERT_EFI_ERROR (Status);
  //
  // Assignment for returning SaInitPolicy config block base address
  //
  *SiPreMemPolicyPpi = SiPreMemPolicy;
  return Status;
}

/**
  SiPreMemInstallPolicyPpi installs SiPreMemPolicyPpi.
  While installed, RC assumes the Policy is ready and finalized. So please update and override
  any setting before calling this function.

  @param[in] SiPreMemPolicyPpi   The pointer to Silicon Policy PPI instance

  @retval EFI_SUCCESS            The policy is installed.
  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to create buffer
**/
EFI_STATUS
EFIAPI
SiPreMemInstallPolicyPpi (
  IN  SI_PREMEM_POLICY_PPI *SiPolicyPreMemPpi
  )
{
  EFI_STATUS             Status;
  EFI_PEI_PPI_DESCRIPTOR *SiPolicyPreMemPpiDesc;

  SiPolicyPreMemPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  if (SiPolicyPreMemPpiDesc == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  SiPolicyPreMemPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  SiPolicyPreMemPpiDesc->Guid  = &gSiPreMemPolicyPpiGuid;
  SiPolicyPreMemPpiDesc->Ppi   = SiPolicyPreMemPpi;

  //
  // Print whole PCH_POLICY_PPI and serial out.
  //
  PchPreMemPrintPolicyPpi (SiPolicyPreMemPpi);
  //
  // Print ME config blocks and serial out.
  //
  MePrintPolicyPpiPreMem (SiPolicyPreMemPpi);
  //
  // Print whole SI_POLICY_PPI and serial out.
  //
  SaPrintPolicyPpiPreMem (SiPolicyPreMemPpi);
  //
  // Print whole CPU of SI_PREMEM_POLICY_PPI and serial out.
  //
  CpuPreMemPrintPolicy (SiPolicyPreMemPpi);
  //
  // Install Silicon Policy PPI
  //
  Status = PeiServicesInstallPpi (SiPolicyPreMemPpiDesc);
  ASSERT_EFI_ERROR (Status);
  return Status;
}

/**
  Calculate total trace buffer size and make it power of two, eliminate the total size within 512MB
  Please ensure CPU and PCH trace hub policies are configured before calling.

  @param[in] SiPreMemPolicyPpi               The pointer to get Silicon Policy PPI instance

  @retval UINT32                             Total size of trace buffers
**/
UINT32
TraceHubCalculateTotalBufferSize (
  IN SI_PREMEM_POLICY_PPI         *SiPreMemPolicyPpi
  )
{
  PCH_TRACE_HUB_PREMEM_CONFIG     *PchTraceHubPreMemConfig;
#ifndef CPU_CFL
  CPU_TRACE_HUB_CONFIG            *CpuTraceHubConfig;
#endif
  EFI_STATUS                      Status;
  UINT32                          TotalTraceBufferSize;
  UINT32                          TraceHubBufferSizeTab[8] = {0, SIZE_1MB, SIZE_8MB, SIZE_64MB, SIZE_128MB, SIZE_256MB, SIZE_512MB, SIZE_1GB};

  TotalTraceBufferSize = 0;
#ifndef CPU_CFL
  Status = GetConfigBlock((VOID *) SiPreMemPolicyPpi, &gCpuTraceHubConfigGuid, (VOID *) &CpuTraceHubConfig);
  ASSERT_EFI_ERROR(Status);
#endif
  Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gPchTraceHubPreMemConfigGuid, (VOID *) &PchTraceHubPreMemConfig);
  ASSERT_EFI_ERROR (Status);

  if (
#ifndef CPU_CFL
      (CpuTraceHubConfig->CpuTraceHubMemReg0Size >= TraceBufferMax) || (CpuTraceHubConfig->CpuTraceHubMemReg1Size >= TraceBufferMax) ||
#endif
      (PchTraceHubPreMemConfig->MemReg0Size >= TraceBufferMax) || (PchTraceHubPreMemConfig->MemReg1Size >= TraceBufferMax)
      ) {
    DEBUG ((DEBUG_ERROR, "Illegal Trace Hub size policy input, should be within the range 0~6, skip allocate trace buffers\n"));
    return 0;
  }
  //
  // Trace Hub mode is not disabled or SCRPD0[24] is set, reserve trace hub memory
  //
#ifndef CPU_CFL
  if ((CpuTraceHubConfig->EnableMode != TraceHubModeDisabled) || (IsDebuggerInUse (CpuTraceHub))) {
    TotalTraceBufferSize += TraceHubBufferSizeTab[CpuTraceHubConfig->CpuTraceHubMemReg0Size] + TraceHubBufferSizeTab[CpuTraceHubConfig->CpuTraceHubMemReg1Size];
  }
#endif
  if ((PchTraceHubPreMemConfig->EnableMode != TraceHubModeDisabled) || (IsDebuggerInUse (PchTraceHub))) {
    TotalTraceBufferSize += TraceHubBufferSizeTab[PchTraceHubPreMemConfig->MemReg0Size] + TraceHubBufferSizeTab[PchTraceHubPreMemConfig->MemReg1Size];
  }

  if (TotalTraceBufferSize > SIZE_512MB) {
    DEBUG ((DEBUG_ERROR, "Total Trace Hub Memory size over limited 512MB, reset trace hub memory region size to default\n"));
    TotalTraceBufferSize = SIZE_64MB + SIZE_64MB;
    DEBUG ((DEBUG_ERROR, "Enforce PCH TH mem reg 0/1 to 64MB/64MB\n"));
#ifndef CPU_CFL
    TotalTraceBufferSize += SIZE_128MB + SIZE_128MB;
    DEBUG ((DEBUG_ERROR, "Enforce CPU TH mem reg 0/1 to 128MB/128MB\n"));
#endif
  }
  //
  // make the total size to be powoer of 2, to ensure use least MTRR when set cache
  //
  if (TotalTraceBufferSize > GetPowerOfTwo32 (TotalTraceBufferSize)) {
    TotalTraceBufferSize = 2 * GetPowerOfTwo32 (TotalTraceBufferSize);
  }
  return TotalTraceBufferSize;
}
