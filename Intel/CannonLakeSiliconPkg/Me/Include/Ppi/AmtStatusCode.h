/** @file
  Header file for AMT Status Code support at PEI phase

@copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2016 Intel Corporation.

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
#ifndef _AMT_STATUS_CODE_PPI_H_
#define _AMT_STATUS_CODE_PPI_H_

///
/// Intel AMT PEI Status Code PPI GUID
/// This driver produces interface to let PEI Status Code generic driver report status to
/// Intel AMT, so that Intel AMT PET message can be sent out in PEI phase.
///
extern EFI_GUID gAmtStatusCodePpiGuid;

///
/// Revision
///
#define AMT_STATUS_CODE_PPI_REVISION  1

typedef struct _AMT_STATUS_CODE_PPI  AMT_STATUS_CODE_PPI;

/**
  Provides an interface that a software module can call to report an ASF PEI status code.
  The extension to report status code to Intel AMT, so that Intel AMT PET message will
  be sent out in PEI.

  @param[in] This                 This interface.
  @param[in] Type                 Indicates the type of status code being reported.
  @param[in] Value                Describes the current status of a hardware or software entity.
                                  This included information about the class and subclass that is
                                  used to classify the entity as well as an operation.
  @param[in] Instance             The enumeration of a hardware or software entity within
                                  the system. Valid instance numbers start with 1.
  @param[in] CallerId             This optional parameter may be used to identify the caller.
                                  This parameter allows the status code driver to apply different
                                  rules to different callers.
  @param[in] Data                 This optional parameter may be used to pass additional data.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_DEVICE_ERROR        The function should not be completed due to a device error.
**/
typedef
EFI_STATUS
(EFIAPI *AMT_STATUS_CODE_PPI_REPORT_STATUS_CODE) (
  IN  AMT_STATUS_CODE_PPI         *This,
  IN  EFI_STATUS_CODE_TYPE        Type,
  IN  EFI_STATUS_CODE_VALUE       Value,
  IN  UINT32                      Instance,
  IN  EFI_GUID                    *CallerId OPTIONAL,
  IN  EFI_STATUS_CODE_DATA        *Data OPTIONAL
  );

///
/// Intel AMT PEI Status Code PPI
/// The PEI platform status code driver should be responsible to use this interface to report
/// status code to Intel AMT. For example, memory init, memory init done, chassis intrusion.
/// So that Intel AMT driver will send corresponding PET message out.
///
struct _AMT_STATUS_CODE_PPI {
  ///
  /// The extension to report status code to Intel AMT, so that Intel AMT PET message will
  /// be sent out in PEI.
  ///
  AMT_STATUS_CODE_PPI_REPORT_STATUS_CODE  ReportStatusCode;
};

/// 1) If AMT driver is not ready yet, StatusCode driver can save information

#endif
