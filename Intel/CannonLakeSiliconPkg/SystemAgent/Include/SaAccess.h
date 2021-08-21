/** @file
  Macros to simplify and abstract the interface to PCI configuration.

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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
#ifndef _SAACCESS_H_
#define _SAACCESS_H_

#include "SaRegs.h"
#include "SaCommonDefinitions.h"

///
/// SystemAgent Base Address definition
///
#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND  1
#endif
#ifndef STALL_ONE_MILLI_SECOND
#define STALL_ONE_MILLI_SECOND  1000
#endif

//
// SA Segement Number
//
#define SA_SEG_NUM         0x00

#define V_SA_DEVICE_ID_INVALID 0xFFFF

///
/// Controller Type
///
typedef enum {
  DMI_PORT = 0x0,
  PEG_PORT = 0x1,
} CONTROLLER_TYPE;

///
/// The value before AutoConfig match the setting of PCI Express Base Specification 1.1, please be careful for adding new feature
///
typedef enum {
  PcieAspmDisabled,
  PcieAspmL0s,
  PcieAspmL1,
  PcieAspmL0sL1,
  PcieAspmAutoConfig,
  PcieAspmMax
} SA_PCIE_ASPM_CONFIG;

///
/// SgMode settings
///
typedef enum {
  SgModeDisabled = 0,
  SgModeReserved,
  SgModeMuxless,
  SgModeDgpu,
  SgModeMax
} SG_MODE;

//
// Macros that judge which type a device ID belongs to
//
#ifdef CPU_CFL
#define IS_SA_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_KBL_MB_ULT_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_ULT_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_ULT_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_ULT_3) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_ULT_4) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_ULT_5) \
    )
#else
#define IS_SA_DEVICE_ID_MOBILE(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_CNL_MB_ULT_1) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_MB_ULT_2) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_MB_ULX_1) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_MB_ULX_2) \
    )
#endif

///
/// Device IDs that are Desktop specific B0:D0:F0
///
#ifdef CPU_CFL
#define IS_SA_DEVICE_ID_DESKTOP(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_KBL_DT_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_DT_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_DT_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_DT_3) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_DT_4) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_WS_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_WS_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_WS_3) \
    )
#else
#define IS_SA_DEVICE_ID_DESKTOP(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_CNL_DT_1) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_DT_2) \
    )
#endif

///
/// Device IDS that are Server specific B0:D0:F0
///
#ifdef CPU_CFL
#define IS_SA_DEVICE_ID_SERVER(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_KBL_SVR_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_SVR_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_SVR_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_SVR_3) \
    )
#else
#define IS_SA_DEVICE_ID_SERVER(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_INVALID) \
    )
#endif

///
/// Device IDs that are Halo specific B0:D0:F0
///
#ifdef CPU_CFL
#define IS_SA_DEVICE_ID_HALO(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_KBL_HALO_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_HALO_1) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_HALO_2) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_HALO_3) || \
      (DeviceId == V_SA_DEVICE_ID_CFL_HALO_IOT_1) \
    )
#else
#define IS_SA_DEVICE_ID_HALO(DeviceId) \
    ( \
      (DeviceId == V_SA_DEVICE_ID_CNL_HALO_1) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_HALO_2) || \
      (DeviceId == V_SA_DEVICE_ID_CNL_HALO_3) \
    )
#endif

#ifdef CPU_CFL
#endif
#endif
