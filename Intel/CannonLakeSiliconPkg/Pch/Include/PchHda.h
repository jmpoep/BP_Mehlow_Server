/** @file
  Header file for HD Audio configuration.

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

#ifndef _PCH_HDA_H_
#define _PCH_HDA_H_

enum PCH_HDAUDIO_VC_TYPE {
  PchHdaVc0 = 0,
  PchHdaVc1 = 1
};

enum PCH_HDAUDIO_SNDW_BUFFER_RCOMP_TOPOLOGY {
  PchHdaSndwNonActTopology = 0,
  PchHdaSndwActTopology    = 1
};

enum PCH_HDAUDIO_DMIC_TYPE {
  PchHdaDmicDisabled = 0,
  PchHdaDmic2chArray = 1,
  PchHdaDmic4chArray = 2,
  PchHdaDmic1chArray = 3
};

typedef enum {
  PchHdaLinkFreq6MHz  = 0,
  PchHdaLinkFreq12MHz = 1,
  PchHdaLinkFreq24MHz = 2,
  PchHdaLinkFreq48MHz = 3,
  PchHdaLinkFreq96MHz = 4,
  PchHdaLinkFreqInvalid
} PCH_HDAUDIO_LINK_FREQUENCY;

typedef enum  {
  PchHdaIDispMode2T  = 0,
  PchHdaIDispMode1T  = 1,
  PchHdaIDispMode4T  = 2,
  PchHdaIDispMode8T  = 3,
  PchHdaIDispMode16T = 4,
  PchHdaIDispTModeInvalid
} PCH_HDAUDIO_IDISP_TMODE;

typedef enum  {
  PchHdaLink      = 0,
  PchHdaIDispLink = 1,
  PchHdaDmic0     = 2,
  PchHdaDmic1     = 3,
  PchHdaSsp0      = 4,
  PchHdaSsp1      = 5,
  PchHdaSsp2      = 6,
  PchHdaSsp3      = 7,
  PchHdaSsp4      = 8,
  PchHdaSsp5      = 9,
  PchHdaSndw1     = 10,
  PchHdaSndw2     = 11,
  PchHdaSndw3     = 12,
  PchHdaSndw4     = 13,
  PchHdaLinkUnsupported
} PCH_HDAUDIO_LINK_TYPE;

#endif // _PCH_HDA_H_
