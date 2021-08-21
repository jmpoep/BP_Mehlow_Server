/** @file

  @copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2018 Intel Corporation. <BR>

  The source code contained or described herein and all documents related to the
  source code ("Material") are owned by Intel Corporation or its suppliers or
  licensors. Title to the Material remains with Intel Corporation or its suppliers
  and licensors. The Material may contain trade secrets and proprietary    and
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
**/

#ifndef _SPS_HW_CHANGE_PPI_H_
#define _SPS_HW_CHANGE_PPI_H_

extern EFI_GUID gSpsHwChangePpiGuid;

typedef struct _SPS_HW_CHANGE_PPI_ SPS_HW_CHANGE_PPI;


/*****************************************************************************
 @brief
  This function sets power measurement suport status and hardware configuration
  change status to be sent to ME. It is needed when platform power measurement
  using Power/Thermal Utility (PTU) Option ROM is supported.

  NOTE: This function can be used only when Node Manager is enabled in ME.

  @param[in] pThis            Pointer to this PPI
  @param[in] PwrMsmtSupport   True if power measurement using PTU is supported,
                              otherwise false.
  @param[in] HwCfgChanged     True if hardware configuration changed since
                              last boot, otherwise false.

  @return EFI_STATUS is returned.
**/
typedef EFI_STATUS (EFIAPI *SPS_HW_CHANGE_SET_STATUS) (
  IN     SPS_HW_CHANGE_PPI *pThis,
  IN     BOOLEAN            PwrMsmtSupport,
  IN     BOOLEAN            HwCfgChanged);

/*
 * EFI protocol for hardware configuration change monitoring for ME.
 *
 * This protocol defines operations needed to inform Node Manager in ME about:
 * - Host support for power consumption measurement using NM PTU Option ROM and
 * - change in hardware configuration that may infuence power consumption.
 * The later triggers power measurement procedure when the first says that
 * measurement is supported.
 */
typedef struct _SPS_HW_CHANGE_PPI_ {
  SPS_HW_CHANGE_SET_STATUS  SpsHwChangeSetStatus;
} SPS_HW_CHANGE_PPI;

#endif // _SPS_HW_CHANGE_PPI_H_

