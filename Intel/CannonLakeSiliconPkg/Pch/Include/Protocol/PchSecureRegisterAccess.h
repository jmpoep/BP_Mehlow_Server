/** @file
  This file defines the PCH Secure Register Access Protocol.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2018 Intel Corporation.

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
#ifndef _PCH_SECURE_REGISTER_ACCESS_H_
#define _PCH_SECURE_REGISTER_ACCESS_H_

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                   gPchSraProtocolGuid;

/**
  Execute USB2 read.

  @param[in] Offset                     Offset of the SBI message
  @param[out] Data32                    Read data
  @param[out] Response                  Response

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_DEVICE_ERROR              Transaction fail
  @retval EFI_INVALID_PARAMETER         Invalid parameter
  @retval EFI_TIMEOUT                   Timeout while waiting for response
**/
typedef
EFI_STATUS
(EFIAPI *SRA_USB2_READ) (
  IN     UINT64                         Offset,
  OUT    UINT32                         *Data32,
  OUT    UINT8                          *Response
  );

/**
  Execute USB2 write.

  @param[in] Offset                     Offset of the SBI message
  @param[in] Data32                     Write data
  @param[out] Response                  Response

  @retval EFI_SUCCESS                   Successfully completed.
  @retval EFI_DEVICE_ERROR              Transaction fail
  @retval EFI_INVALID_PARAMETER         Invalid parameter
  @retval EFI_TIMEOUT                   Timeout while waiting for response
**/
typedef
EFI_STATUS
(EFIAPI *SRA_USB2_WRITE) (
  IN     UINT64                         Offset,
  IN     UINT32                         Data32,
  OUT    UINT8                          *Response
  );

/**
  The protocol is for PCH secure register access.
**/
typedef struct {
  /**
    This member specifies the revision of this structure. This field is used to
    indicate backwards compatible changes to the protocol.
  **/
  UINT8                             Revision;
  SRA_USB2_READ                     Usb2Read;
  SRA_USB2_WRITE                    Usb2Write;
} PCH_SRA_PROTOCOL;

/**
  PCH SRA PROTOCOL revision number

  Revision 1:   Initial version
**/
#define PCH_SRA_REVISION       1

#endif
