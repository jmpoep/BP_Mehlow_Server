/** @file
 Common configurations for CPU and PCH trace hub

@copyright
  INTEL CONFIDENTIAL
  Copyright 2017 Intel Corporation.

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
#ifndef _TRACE_HUB_COMMON_CONFIG_H_
#define _TRACE_HUB_COMMON_CONFIG_H_

///
/// The TRACE_HUB_ENABLE_MODE describes the desired TraceHub mode of operation
///
typedef enum {
  TraceHubModeDisabled       = 0,       ///< TraceHub Disabled
  TraceHubModeTargetDebugger = 1,       ///< TraceHub Target Debugger mode, debug on target device itself, config to PCI mode
  TraceHubModeHostDebugger   = 2,       ///< TraceHub Host Debugger mode, debugged by host with cable attached, config to ACPI mode
  TraceHubModeMax
} TRACE_HUB_ENABLE_MODE;

///
/// The TRACE_BUFFER_SIZE describes the desired TraceHub buffer size
///
typedef enum {
  TraceBufferNone,
  TraceBuffer1M,
  TraceBuffer8M,
  TraceBuffer64M,
  TraceBuffer128M,
  TraceBuffer256M,
  TraceBuffer512M,
  TraceBufferMax
} TRACE_BUFFER_SIZE;

#endif
