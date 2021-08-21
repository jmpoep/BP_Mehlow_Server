/** @file
  This header file provides common definitions just for System Agent using to avoid including extra module's file.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2016 Intel Corporation.

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
#ifndef _SA_COMMON_DEFINITIONS_H_
#define _SA_COMMON_DEFINITIONS_H_

#define ERROR_BY_16     (0xEE15)
#define ERROR_NOT_BY_16 (0xED15)

#define MAX_PCIE_ASPM_OVERRIDE       500
#define MAX_PCIE_LTR_OVERRIDE        500

#define DISABLED  0
#define ENABLED   1

#define SA_VTD_ENGINE_NUMBER        3

///
///  Common code version reporting structure
///
typedef struct {
  UINT8 Major;  ///< Major version number
  UINT8 Minor;  ///< Minor version number
  UINT8 Rev;    ///< Revision number
  UINT8 Build;  ///< Build number
} CODE_VERSION;
#endif
