/** @file
  This file contains GPIO name library implementation specific to CNL

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
#include <Base.h>
#include <Library/BaseLib.h>
#include <Uefi/UefiBaseType.h>
#include <Library/DebugLib.h>
#include <Library/PchInfoLib.h>
#include <Private/Library/GpioPrivateLib.h>
#include <GpioPinsCnlLp.h>
#include <GpioPinsCnlH.h>

STATIC CONST CHAR8*  mGpioGppaNames[] = {
  "ESPI_CLK_LOOPBK"
};

STATIC CONST CHAR8*  mGpioGppbNames[] = {
  "GSPI0_CLK_LOOPBK",
  "GSPI1_CLK_LOOPBK"
};

STATIC CONST CHAR8*  mGpioGpdNames[] = {
  "SLP_LANB",
  "SLP_SUSB",
  "SLP_WAKEB",
  "SLP_DRAM_RESETB"
};

STATIC CONST CHAR8*  mGpioGppiNames[] = {
  "SYS_PWROK",
  "SYS_RESETB",
  "MLK_RSTB"
};

STATIC CONST CHAR8*  mGpioSpiNames[] = {
  "SPI0_IO_2",
  "SPI0_IO_3",
  "SPI0_MOSI_IO_0",
  "SPI0_MOSI_IO_1",
  "SPI0_TPM_CSB",
  "SPI0_FLASH_0_CSB",
  "SPI0_FLASH_1_CSB",
  "SPI0_CLK",
  "SPI0_CLK_LOOPBK"
};

STATIC CONST CHAR8*  mGpioAzaNames[] = {
  "HDA_BCLK",
  "HDA_RSTB",
  "HDA_SYNC",
  "HDA_SDO",
  "HDA_SDI_0",
  "HDA_SDI_1",
  "SSP1_SFRM",
  "SSP1_TXD"
};

STATIC CONST CHAR8*  mGpioJtagNames[] = {
  "JTAG_TDO",
  "JTAGX",
  "PRDYB",
  "PREQB",
  "CPU_TRSTB",
  "JTAG_TDI",
  "JTAG_TMS",
  "JTAG_TCK",
  "ITP_PMODE"
};

STATIC CONST CHAR8*  mGpioHvmosNames[] = {
  "HVMOS_L_BKLTEN",
  "HVMOS_L_BKLCTL",
  "HVMOS_L_VDDEN",
  "HVMOS_SYS_PWROK",
  "HVMOS_SYS_RESETB",
  "HVMOS_MLK_RSTB"
};

STATIC CONST CHAR8*  mGpioCpuNames[] = {
  "HDACPU_SDI",
  "HDACPU_SDO",
  "HDACPU_SCLK",
  "PM_SYNC",
  "PECI",
  "CPUPWRGD",
  "THRMTRIPB",
  "PLTRST_CPUB",
  "PM_DOWN",
  "TRIGGER_IN",
  "TRIGGER_OUT"
};

STATIC CONST GPIO_GROUP_NAME_INFO  mPchLpGroupDescriptors[] = {
  GPIO_GROUP_NAME("GPP_A", GPIO_CNL_LP_ESPI_CLK_LOOPBK, mGpioGppaNames),
  GPIO_GROUP_NAME("GPP_B", GPIO_CNL_LP_GSPI0_CLK_LOOPBK, mGpioGppbNames),
  GPIO_GROUP_NAME_BASIC("GPP_C"),
  GPIO_GROUP_NAME_BASIC("GPP_D"),
  GPIO_GROUP_NAME_BASIC("GPP_E"),
  GPIO_GROUP_NAME_BASIC("GPP_F"),
  GPIO_GROUP_NAME_BASIC("GPP_G"),
  GPIO_GROUP_NAME_BASIC("GPP_H"),
  GPIO_GROUP_NAME("GPD", GPIO_CNL_LP_SLP_LANB, mGpioGpdNames),
  GPIO_GROUP_NAME_BASIC("VGPIO"),
  GPIO_GROUP_NAME("SPI", GPIO_CNL_LP_SPI0_IO_2, mGpioSpiNames),
  GPIO_GROUP_NAME("AZA", GPIO_CNL_LP_HDA_BCLK, mGpioAzaNames),
  GPIO_GROUP_NAME("CPU", GPIO_CNL_LP_HDACPU_SDI, mGpioCpuNames),
  GPIO_GROUP_NAME("JTAG", GPIO_CNL_LP_JTAG_TDO, mGpioJtagNames),
  GPIO_GROUP_NAME("HVMOS", GPIO_CNL_LP_HVMOS_L_BKLTEN, mGpioHvmosNames)
};

STATIC CONST GPIO_GROUP_NAME_INFO  mPchHGroupDescriptors[] = {
  GPIO_GROUP_NAME("GPP_A", GPIO_CNL_H_ESPI_CLK_LOOPBK, mGpioGppaNames),
  GPIO_GROUP_NAME("GPP_B", GPIO_CNL_H_GSPI0_CLK_LOOPBK, mGpioGppbNames),
  GPIO_GROUP_NAME_BASIC("GPP_C"),
  GPIO_GROUP_NAME_BASIC("GPP_D"),
  GPIO_GROUP_NAME_BASIC("GPP_E"),
  GPIO_GROUP_NAME_BASIC("GPP_F"),
  GPIO_GROUP_NAME_BASIC("GPP_G"),
  GPIO_GROUP_NAME_BASIC("GPP_H"),
  GPIO_GROUP_NAME("GPP_I", GPIO_CNL_H_SYS_PWROK, mGpioGppiNames),
  GPIO_GROUP_NAME_BASIC("GPP_J"),
  GPIO_GROUP_NAME_BASIC("GPP_K"),
  GPIO_GROUP_NAME("GPD", GPIO_CNL_H_SLP_LANB, mGpioGpdNames),
  GPIO_GROUP_NAME_BASIC("VGPIO"),
  GPIO_GROUP_NAME("SPI", GPIO_CNL_H_SPI0_IO_2, mGpioSpiNames),
  GPIO_GROUP_NAME("AZA", GPIO_CNL_H_HDA_BCLK, mGpioAzaNames),
  GPIO_GROUP_NAME("CPU", GPIO_CNL_H_HDACPU_SDI, mGpioCpuNames),
  GPIO_GROUP_NAME("JTAG", GPIO_CNL_H_JTAG_TDO, mGpioJtagNames),
};

/**
  Returns GPIO_GROUP_NAME_INFO corresponding to the given GpioPad

  @param[in] GroupIndex  Group index

  @retval GPIO_GROUP_NAME_INFO*  Pointer to the GPIO_GROUP_NAME_INFO
  @reval  NULL                   If no group descriptor was found
**/
CONST
GPIO_GROUP_NAME_INFO*
GpioGetGroupNameInfo (
  IN UINT32  GroupIndex
  )
{
  if (IsPchLp ()) {
    if (GroupIndex < ARRAY_SIZE (mPchLpGroupDescriptors)) {
      return &mPchLpGroupDescriptors[GroupIndex];
    } else {
      ASSERT (FALSE);
      return NULL;
    }
  } else {
    if (GroupIndex < ARRAY_SIZE (mPchHGroupDescriptors)) {
      return &mPchHGroupDescriptors[GroupIndex];
    } else {
      ASSERT (FALSE);
      return NULL;
    }
  }
}

