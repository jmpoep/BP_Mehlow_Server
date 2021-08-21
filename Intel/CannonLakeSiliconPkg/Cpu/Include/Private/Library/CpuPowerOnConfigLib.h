/** @file
  Header file for CpuPowerOnConfig PEI Library implementation.

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

@par Specification
**/

#ifndef _CPU_POWER_ON_CONFIG_LIB_H_
#define _CPU_POWER_ON_CONFIG_LIB_H_

#include <Ppi/SiPolicy.h>

///
/// Structure for CPU Strap settings
///
typedef struct {
  UINT32 HtDisabled              :  1; ///< Intel HT Technology Disable.
  UINT32 NumberOfActiveCores     :  3; ///< Number of Active Cores.
  UINT32 Reserved1               :  1;
  UINT32 Bist                    :  1; ///< Built In Self Test (BIST) initiation.
  UINT32 FlexRatio               :  6; ///< Flex Ratio.
  UINT32 BootRatio               :  1; ///< Processor boot ratio; When set allows the processor to power up in maximum non-turbo ratio from the following boot.
  UINT32 JtagC10PowerGateDisable :  1; ///< JTAG Power gate disable.
  UINT32 Reserved2               :  1;
  UINT32 DpSscMarginEnable       :  1; ///< Enable DisplayPort SSC range reduction
  UINT32 Reserved3               : 16;
} CPU_STRAP_SET;

///
/// Structure for Early Power on Configuration (EPOC) for Coffee Lake.
///
typedef union {
  struct {
  UINT32 Fclk      : 2;  ///< Bits 1:0 - 00 = normal operation, 01 = 1 GHz, 10 = 400 Mhz, 11 = Reserved
  UINT32 Reserved  : 30; ///< Bits 31:2 - Reserved
  } Bits;
  UINT32 Uint32;
} CPU_EPOC_SET;

/**
  Set Cpu Strap and Set Early Power On Configuration setting for feature change.

  @param[in] SiPreMemPolicyPpi - Address of the SI_PREMEM_POLICY_PPI.

  @retval EFI_SUCCESS       - Function completed successfully.
**/
EFI_STATUS
SetCpuStrapAndEarlyPowerOnConfig (
  IN SI_PREMEM_POLICY_PPI       *SiPreMemPolicyPpi
  );

#endif
