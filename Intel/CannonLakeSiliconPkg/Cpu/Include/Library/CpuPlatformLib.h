/** @file
  Header file for CpuPlatform Lib.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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

#ifndef _CPU_PLATFORM_LIB_H_
#define _CPU_PLATFORM_LIB_H_

#include <CpuRegs.h>
#include <CpuDataStruct.h>
#include <CpuPowerMgmt.h>

/**
  Check CPU Type of the platform

  @retval CPU_FAMILY              CPU type
**/
CPU_FAMILY
EFIAPI
GetCpuFamily (
  VOID
  );

/**
  Return Cpu stepping type

  @retval CPU_STEPPING                   Cpu stepping type
**/
CPU_STEPPING
EFIAPI
GetCpuStepping (
  VOID
  );

/**
  Return CPU Sku

  @retval UINT8              CPU Sku
**/
UINT8
EFIAPI
GetCpuSku (
  VOID
  );

/**
  Return CPU Identifier used to identify various CPU types

  @retval CPU_OVERRIDE_IDENTIFIER           CPU Identifier
**/
CPU_OVERRIDE_IDENTIFIER
EFIAPI
GetCpuIdentifier (
  VOID
  );

/**
  Returns the processor microcode revision of the processor installed in the system.

  @retval Processor Microcode Revision
**/
UINT32
GetCpuUcodeRevision (
  VOID
  );

/**
  Check if this microcode is correct one for processor

  @param[in] Cpuid               - processor CPUID
  @param[in] MicrocodeEntryPoint - entry point of microcode
  @param[in] Revision            - revision of microcode

  @retval CorrectMicrocode if this microcode is correct
**/
BOOLEAN
CheckMicrocode (
  IN UINT32               Cpuid,
  IN CPU_MICROCODE_HEADER *MicrocodeEntryPoint,
  IN UINT32               *Revision
  );

/**
  This function is to program Trace Hub ACPI base address to processor's MSR TRACE_HUB_STH_ACPIBAR_BASE.

  @param[in]  TraceHubAcpiBase - Base address of Trace Hub ACPI Base address
**/
VOID
EFIAPI
CpuWriteTraceHubAcpiBase (
  IN UINT64  TraceHubAcpiBase
  );

/**
  Check on the processor if SGX is supported.

  @retval True if SGX supported or FALSE if not
**/
BOOLEAN
IsSgxSupported (
  VOID
  );

/**
  Detect if C6DRAM supported or not by reading it from PCODE mailbox
  If C6DRAM is supported, configures whether it is enabled based on input parameter.

  @param[in]  EnableC6Dram - Policy setting for C6DRAM

  @retval TRUE - Supported
  @retval FALSE - Not Supported
**/
BOOLEAN
IsC6dramSupported (
  IN UINT32       EnableC6Dram
  );

/**
  Get processor generation

  @retval CPU_GENERATION  Returns the executing thread's processor generation.
**/
CPU_GENERATION
GetCpuGeneration (
  VOID
  );

/**
  Check if Disable CPU Debug (DCD) bit is set from FIT CPU Debugging [Disabled].
  If it is set, CPU probe mode is disabled.

  @retval TRUE    DCD is set
  @retval FALSE   DCD is clear
**/
BOOLEAN
IsCpuDebugDisabled (
  VOID
  );

/**
  Is BIOS GUARD enabled.

  @retval TRUE   BIOS GUARD is supported and enabled.
  @retval FALSE  BIOS GUARD is disabled.
**/
BOOLEAN
IsBiosGuardEnabled (
  VOID
  );

/**
  Is Whiskey Lake CPU.

  @retval TRUE  The CPUID corresponds with a Whiskey Lake CPU
  @retval FALSE The CPUID does not correspond with a Whiskey Lake CPU
**/
BOOLEAN
IsWhlCpu (
  VOID
  );

/**
  Is Nifty Rock feature supported.

  @retval TRUE   Nifty Rock feature is supported.
  @retval FALSE  Nifty Rock feature is not supported.
**/
BOOLEAN
IsNiftyRockSupported (
  VOID
  );

#endif
