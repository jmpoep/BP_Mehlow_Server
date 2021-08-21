/** @file
  VT-d policy definitions.

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

@par Specification Reference:
**/
#ifndef _VTD_CONFIG_H_
#define _VTD_CONFIG_H_

#pragma pack(push, 1)

#define VTD_CONFIG_REVISION 2

/**
  The data elements should be initialized by a Platform Module.
  The data structure is for VT-d driver initialization\n
  <b>Revision 1</b>:
  - Initial version.
  <b>Revision 2</b>:
  - Add DMA_CONTROL_GUARANTEE bit in the DMAR table
**/
typedef struct {
  CONFIG_BLOCK_HEADER  Header;                      ///< Offset 0-27 Config Block Header
  /**
    Offset 28:0 :
    VT-D Support can be verified by reading CAP ID register as expalined in BIOS Spec.
    This policy is for debug purpose only.
    If VT-D is not supported, all other policies in this config block will be ignored.
    <b>0 = To use Vt-d</b>;
    1 = Avoids programming Vtd bars, Vtd overrides and DMAR table.
  **/
  UINT32        VtdDisable               : 1;
  UINT32        X2ApicOptOut             : 1;       ///< Offset 28:1 :This field is used to enable the X2APIC_OPT_OUT bit in the DMAR table. 1=Enable/Set and <b>0=Disable/Clear</b>
  UINT32        DmaControlGuarantee      : 1;       ///< Offset 28:2 :This field is used to enable the DMA_CONTROL_GUARANTEE bit in the DMAR table. 1=Enable/Set and <b>0=Disable/Clear</b>
  UINT32        RsvdBits0                : 29;      ///< Offset 28:3 :Reserved bits for future use
  UINT32        BaseAddress[SA_VTD_ENGINE_NUMBER];  ///< Offset 32: This field is used to describe the base addresses for VT-d function: <b>BaseAddress[0]=0xFED90000, BaseAddress[1]=0xFED92000, BaseAddress[2]=0xFED91000 </b>
} VTD_CONFIG;
#pragma pack(pop)

#endif   //  _VTD_CONFIG_H_
