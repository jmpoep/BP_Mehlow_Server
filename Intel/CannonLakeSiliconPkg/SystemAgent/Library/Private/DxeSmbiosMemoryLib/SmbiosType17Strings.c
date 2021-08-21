/** @file
  This library will determine memory configuration information from the chipset
  and memory and create SMBIOS memory structures appropriately.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2016 Intel Corporation.

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
#include "SmbiosMemory.h"

GLOBAL_REMOVE_IF_UNREFERENCED CHAR8 *DimmToDevLocator[] = {
  "ChannelA-DIMM0",
  "ChannelA-DIMM1",
  "ChannelB-DIMM0",
  "ChannelB-DIMM1"
};

GLOBAL_REMOVE_IF_UNREFERENCED CHAR8 *DimmToBankLocator[] = {
  "BANK 0",
  "BANK 1",
  "BANK 2",
  "BANK 3"
};

GLOBAL_REMOVE_IF_UNREFERENCED MEMORY_MODULE_MANUFACTURE_LIST MemoryModuleManufactureList[] = {
  {0,    0x2c, "Micron"},
  {0,    0xad, "SK Hynix"},
  {0,    0xce, "Samsung"},
  {1,    0x4f, "Transcend"},
  {1,    0x98, "Kingston"},
  {2,    0xfe, "Elpida"},
  {0xff, 0xff, 0}
};
