/** @file
    Source code file for Silicon Init Post Memory module.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2013 - 2018 Intel Corporation.

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

@par Specification
**/

#include "SiInit.h"
#include <Library/PerformanceLib.h>
#include <MeChipset.h>
#include <Library/PchInfoLib.h>
#include <Private/Library/PeiP2sbPrivateLib.h>
#include <Private/Library/PeiHdaInitLib.h>


STATIC SVID_SID_INIT_ENTRY mCdfSsidTablePtr[] = {
  //
  // SA Device(s)
  //
  {{{PCI_SVID_OFFSET,    SA_MC_FUN,        SA_MC_DEV,        SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG0_FUN_NUM,  SA_PEG0_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG1_FUN_NUM,  SA_PEG1_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG2_FUN_NUM,  SA_PEG2_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG3_FUN_NUM,  SA_PEG3_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_IGD_FUN_0,     SA_IGD_DEV,       SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_IPU_FUN_NUM,   SA_IPU_DEV_NUM,   SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_GNA_FUN_NUM,   SA_GNA_DEV_NUM,   SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_PCI_FUN_0,     SA_PCI_DEV_4,     SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_PCI_FUN_0,     SA_PCI_DEV_7,     SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  //
  // PCH Device(s)
  //
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_LPC,               PCI_DEVICE_NUMBER_PCH_LPC,           DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_P2SB,              PCI_DEVICE_NUMBER_PCH_P2SB,          DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_PMC,               PCI_DEVICE_NUMBER_PCH_PMC,           DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_HDA,               PCI_DEVICE_NUMBER_PCH_HDA,           DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_CDF_PCH_SATA_1,        PCI_DEVICE_NUMBER_CDF_PCH_SATA_1,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_CDF_PCH_SATA_2,        PCI_DEVICE_NUMBER_CDF_PCH_SATA_2,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_CDF_PCH_SATA_3,        PCI_DEVICE_NUMBER_CDF_PCH_SATA_3,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_SMBUS,             PCI_DEVICE_NUMBER_PCH_SMBUS,         DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_SPI,               PCI_DEVICE_NUMBER_PCH_SPI,           DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_TRACE_HUB,         PCI_DEVICE_NUMBER_PCH_TRACE_HUB,     DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_XHCI,              PCI_DEVICE_NUMBER_PCH_XHCI,          DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_XDCI,              PCI_DEVICE_NUMBER_PCH_XDCI,          DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_THERMAL,           PCI_DEVICE_NUMBER_PCH_THERMAL,       DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_5,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_6,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_7,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_8,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_9,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_10, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_11, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_12, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
};

STATIC SVID_SID_INIT_ENTRY mSsidTablePtr[] = {
  //
  // SA Device(s)
  //
  {{{PCI_SVID_OFFSET,    SA_MC_FUN,        SA_MC_DEV,        SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG0_FUN_NUM,  SA_PEG0_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG1_FUN_NUM,  SA_PEG1_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG2_FUN_NUM,  SA_PEG2_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{R_SA_PEG_SS_OFFSET, SA_PEG3_FUN_NUM,  SA_PEG3_DEV_NUM,  SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_IGD_FUN_0,     SA_IGD_DEV,       SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_IPU_FUN_NUM,   SA_IPU_DEV_NUM,   SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_GNA_FUN_NUM,   SA_GNA_DEV_NUM,   SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_PCI_FUN_0,     SA_PCI_DEV_4,     SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    SA_PCI_FUN_0,     SA_PCI_DEV_7,     SA_MC_BUS, 0, SA_SEG_NUM, 0}}, {0, 0},0},
  //
  // PCH Device(s)
  //
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_LPC,     PCI_DEVICE_NUMBER_PCH_LPC,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_P2SB,    PCI_DEVICE_NUMBER_PCH_P2SB,   DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_PMC,     PCI_DEVICE_NUMBER_PCH_PMC,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_HDA,     PCI_DEVICE_NUMBER_PCH_HDA,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_SATA,    PCI_DEVICE_NUMBER_PCH_SATA,   DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_SMBUS,   PCI_DEVICE_NUMBER_PCH_SMBUS,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_SPI,     PCI_DEVICE_NUMBER_PCH_SPI,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  //
  // Skip PCH LAN controller
  // PCH LAN SVID/SID may be loaded automatically from the NVM Word 0Ch/0Bh upon power up or reset
  // depending on the "Load Subsystem ID" bit field in NVM word 0Ah
  //
  //{{{PCI_SVID_OFFSET,    PCI_FUNCTION_NUMBER_PCH_LAN,     PCI_DEVICE_NUMBER_PCH_LAN,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_TRACE_HUB,       PCI_DEVICE_NUMBER_PCH_TRACE_HUB,        DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART0, PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART0,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART1, PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART1,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI0,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI0,   DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI1,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI1,   DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_CNL_SCS_EMMC,        PCI_DEVICE_NUMBER_PCH_CNL_SCS_EMMC,         DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< SCS EMMC, SKL PCH-LP only
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_CNL_SCS_UFS,         PCI_DEVICE_NUMBER_PCH_CNL_SCS_UFS,          DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_SPI2,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_SPI2,   DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_CNL_SCS_SDCARD, PCI_DEVICE_NUMBER_PCH_CNL_SCS_SDCARD, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< SCS SD Card, SKL PCH-LP only
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_XHCI,       PCI_DEVICE_NUMBER_PCH_XHCI,       DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_XDCI,       PCI_DEVICE_NUMBER_PCH_XDCI,       DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_THERMAL,    PCI_DEVICE_NUMBER_PCH_THERMAL,    DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_ISH,               PCI_DEVICE_NUMBER_PCH_ISH,           DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_5,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_6,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_7,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_8,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_1, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_9,  PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_10, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_11, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_12, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_13, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #13, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_14, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #14, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_15, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #15, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_16, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #16, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_17, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #17, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_18, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #18, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_19, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #19, SKL-H and KBL-H only
  {{{R_PCH_PCIE_CFG_SVID,  PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_20, PCI_DEVICE_NUMBER_PCH_PCIE_DEVICE_3, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}, ///< PCI Express Root Port #20, SKL-H and KBL-H only
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C0,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C0,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C1,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C1,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C2,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C2,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C3,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C3,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_UART2, PCI_DEVICE_NUMBER_PCH_SERIAL_IO_UART2, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C5,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C5,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  PCI_FUNCTION_NUMBER_PCH_SERIAL_IO_I2C4,  PCI_DEVICE_NUMBER_PCH_SERIAL_IO_I2C4,  DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  //
  // ME Device(s)
  //
  {{{PCI_SVID_OFFSET,  HECI_FUNCTION_NUMBER,  ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  HECI2_FUNCTION_NUMBER, ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  IDER_FUNCTION_NUMBER,  ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  SOL_FUNCTION_NUMBER,   ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  HECI3_FUNCTION_NUMBER, ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0},
  {{{PCI_SVID_OFFSET,  HECI4_FUNCTION_NUMBER, ME_DEVICE_NUMBER, DEFAULT_PCI_BUS_NUMBER_PCH, 0, DEFAULT_PCI_SEGMENT_NUMBER_PCH, 0}}, {0, 0},0}
};

#ifndef FSP_FLAG
EFI_PEI_PPI_DESCRIPTOR mEndOfSiInit = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI |EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEndOfSiInitPpiGuid,
  NULL
};
#endif // FSP_FLAG

#ifndef FSP_FLAG
GLOBAL_REMOVE_IF_UNREFERENCED EFI_PEI_NOTIFY_DESCRIPTOR  mSiInitNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEndOfSiInitPpiGuid,
    SiInitOnEndOfPei
  }
};
#else
GLOBAL_REMOVE_IF_UNREFERENCED EFI_PEI_NOTIFY_DESCRIPTOR  mSiInitNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiEndOfPeiSignalPpiGuid,
    SiInitOnEndOfPei
  }
};
#endif // FSP_FLAG

static EFI_PEI_NOTIFY_DESCRIPTOR  mSiInitPostMemNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gSiPolicyPpiGuid,
    SiInitPostMemOnPolicy
  }
};

/**
  Silicon Init End of PEI callback function. This is the last change before entering DXE and OS when S3 resume.

  @param[in] PeiServices   - Pointer to PEI Services Table.
  @param[in] NotifyDesc    - Pointer to the descriptor for the Notification event that
                             caused this function to execute.
  @param[in] Ppi           - Pointer to the PPI data associated with this function.

  @retval EFI_STATUS       - Always return EFI_SUCCESS
**/
EFI_STATUS
SiInitOnEndOfPei (
  IN EFI_PEI_SERVICES                   **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR          *NotifyDesc,
  IN VOID                               *Ppi
  )
{
  EFI_STATUS             Status;
  SI_POLICY_PPI          *SiPolicy;
  SI_CONFIG              *SiConfig;
  CPU_CONFIG             *CpuConfig;
  EFI_BOOT_MODE          BootMode;
  CPU_CONFIG_LIB_PREMEM_CONFIG       *CpuConfigLibPreMemConfig;
  SI_PREMEM_POLICY_PPI               *SiPreMemPolicyPpi;

  //
  // Get Policy settings through the SiPolicy PPI
  //
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicy
             );
  if (Status != EFI_SUCCESS) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **)&SiPreMemPolicyPpi
             );
  if (Status != EFI_SUCCESS) {
    ASSERT (FALSE);
    return EFI_NOT_FOUND;
  }

  Status = GetConfigBlock ((VOID *) SiPolicy, &gSiConfigGuid, (VOID *) &SiConfig);
  ASSERT_EFI_ERROR (Status);
  Status = GetConfigBlock((VOID *)SiPolicy, &gCpuConfigGuid, (VOID *)&CpuConfig);
  ASSERT_EFI_ERROR(Status);
  Status = GetConfigBlock ((VOID *)SiPreMemPolicyPpi, &gCpuConfigLibPreMemConfigGuid, (VOID *)&CpuConfigLibPreMemConfig);
  ASSERT_EFI_ERROR (Status);


  DEBUG ((DEBUG_INFO, "SiInitOnEndOfPei - Start\n"));

  //
  // Initializes PCH after End of Pei
  //
  PchOnEndOfPei ();
  //
  // Execute before P2SB lock to ensure hiding trace hub thru PSF is valid.
  //
  ConfigureMscForCpuAndPchTraceHub (SiConfig->TraceHubMemBase);
  //
  // Configure P2SB at the end of EndOfPei
  // This must be done before POSTBOOT_SAI programming.
  //
  P2sbLock (SiPolicy);

#ifndef CPU_CFL
  if (CpuConfigLibPreMemConfig->SkipMpInit == 0) {
    //
    // Set BIOS DONE MSR on all Cores
    //
    SetBiosDone ((CONST EFI_PEI_SERVICES **) PeiServices);
  }
#else
  //
  // Set PCH DMI HOSTIA_POSTBOOT_SAI with CFL CPU.
  //
  PchDmiEnablePostBootSai ();
  //
  // Lock SAI access from P2SB after PostBootSai and before any 3rd code execution.
  //
  P2sbSaiLock ();
#endif

  //
  // Do necessary PCH configuration just after POSTBOOT_SAI switch
  //
  PchDmiConfigAfterPostBootSai ();

  #ifndef CPU_CFL
  CpuInitAtEndOfPei ((CONST EFI_PEI_SERVICES **) PeiServices);
  #endif

  //
  // Set BIOS_RESET_CPL to indicate BIOS initialization completed
  //
  PERF_START_EX (&gPerfSaResetPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4090);
  SaResetComplete ();
  PERF_END_EX (&gPerfSaResetPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4091);

  PERF_START_EX (&gPerfHdaPostMemGuid, NULL, NULL, AsmReadTsc (), 0x40D0);
  HdAudioInitOnEndOfPei (SiPolicy);
  PERF_END_EX (&gPerfHdaPostMemGuid, NULL, NULL, AsmReadTsc (), 0x40D1);

  //
  // Initialize power management after RESET_CPL at post-memory phase.
  //
  if (CpuConfigLibPreMemConfig->SkipMpInit == 0) {
    PERF_START_EX (&gPerfCpuPowerMgmtGuid, NULL, NULL, AsmReadTsc (), 0x40A0);
    CpuPowerMgmtInit ((CONST EFI_PEI_SERVICES **) PeiServices);
    PERF_END_EX (&gPerfCpuPowerMgmtGuid, NULL, NULL, AsmReadTsc (), 0x40A1);
  }

  MeOnEndOfPei ();
  SaOnEndOfPei ();

  //
  // Build FVI Info HOB in normal boot
  //
  Status = PeiServicesGetBootMode (&BootMode);
  if ((Status == EFI_SUCCESS) && (BootMode != BOOT_ON_S3_RESUME)) {
    BuildFviInfoHob ();
  }

  InitializeSmbiosCpuHobs ();

  DEBUG ((DEBUG_INFO, "SiInitOnEndOfPei - End\n"));
  return EFI_SUCCESS;
}

/**
    Program SA devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID).

**/

VOID
ProgramSvidSid (
  VOID
  )
{
  UINT8                         Index;
  UINT8                         SgMode;
  UINT16                        NumberOfSsidTableEntry;
  SVID_SID_INIT_ENTRY           *SsidTablePtr;
  EFI_STATUS                    Status;
  SI_PREMEM_POLICY_PPI          *SiPreMemPolicyPpi;
  SA_MISC_PEI_PREMEM_CONFIG     *MiscPeiPreMemConfig;
  SI_POLICY_PPI                 *SiPolicy;
  SI_CONFIG                     *SiConfig;
  UINT32                        Data32;
  UINT16                        DataSvid;
  UINT16                        DataSsid;
  UINT64                        IgdDeviceSvidAddr;

  SgMode = 0;
  MiscPeiPreMemConfig = NULL;
  SiPreMemPolicyPpi = NULL;

  IgdDeviceSvidAddr = (UINT64) PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_IGD_DEV, SA_IGD_FUN_0, PCI_SVID_OFFSET);

  Status = PeiServicesLocatePpi (
             &gSiPreMemPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPreMemPolicyPpi
             );

  if ((Status == EFI_SUCCESS) && (SiPreMemPolicyPpi != NULL)) {
    MiscPeiPreMemConfig = NULL;
    Status = GetConfigBlock ((VOID *) SiPreMemPolicyPpi, &gSaMiscPeiPreMemConfigGuid, (VOID *) &MiscPeiPreMemConfig);
    if (MiscPeiPreMemConfig != NULL) {
      SgMode = MiscPeiPreMemConfig->SgMode;
    }
  }

  //
  // Set Table ptr and Table count to default value
  //
  SsidTablePtr = mSsidTablePtr;
  NumberOfSsidTableEntry = (sizeof (mSsidTablePtr) / sizeof (SVID_SID_INIT_ENTRY));

  if (IsCdfPch ()) {
    SsidTablePtr = mCdfSsidTablePtr;
    NumberOfSsidTableEntry = (sizeof (mCdfSsidTablePtr) / sizeof (SVID_SID_INIT_ENTRY));
  }
  //
  // Locate SiPolicyPpi
  //
  SiPolicy = NULL;
  Status = PeiServicesLocatePpi (
             &gSiPolicyPpiGuid,
             0,
             NULL,
             (VOID **) &SiPolicy
             );
  if ((Status == EFI_SUCCESS) && (SiPolicy != NULL)) {
    Status = GetConfigBlock ((VOID *) SiPolicy, &gSiConfigGuid, (VOID *) &SiConfig);
    if ((Status == EFI_SUCCESS) &&
       (SiConfig != NULL) &&
       (SiConfig->SsidTablePtr != 0) &&
       (SiConfig->NumberOfSsidTableEntry != 0)) {
      //
      // Use SiPolicy Table and table counts
      //
      SsidTablePtr = (SVID_SID_INIT_ENTRY*) SiConfig->SsidTablePtr;
      NumberOfSsidTableEntry = MAX_DEVICE_COUNT;
      //
      // Verify number of devices
      //
      if ((SiConfig->NumberOfSsidTableEntry < MAX_DEVICE_COUNT)) {
        NumberOfSsidTableEntry = SiConfig->NumberOfSsidTableEntry;
      }
    }
  }

  for (Index = 0; Index < NumberOfSsidTableEntry; Index++) {
    ///
    /// Skip if the device is disabled
    ///
    if (PciSegmentRead32 (SsidTablePtr[Index].Address.SegBusDevFuncRegister & (~0xFFF)) == 0xFFFFFFFF) {
      continue;
    }
    ///
    /// Program SA devices Subsystem Vendor Identifier (SVID)
    ///
    DataSvid = SsidTablePtr[Index].SvidSidValue.SubSystemVendorId;
    if (DataSvid == 0) {
      DataSvid = DEFAULT_SSVID;
    }
    //
    // GET SID
    //
    DataSsid = SsidTablePtr[Index].SvidSidValue.SubSystemId;
    if (DataSsid == 0) {
      DataSsid = DEFAULT_SSDID;
    }
    Data32 = (DataSsid << 16) | DataSvid;
    if (SsidTablePtr[Index].Address.SegBusDevFuncRegister != IgdDeviceSvidAddr) {
      PciSegmentWrite32 (
        SsidTablePtr[Index].Address.SegBusDevFuncRegister,
        Data32
        );
    } else {
      if ((SsidTablePtr[Index].SvidSidValue.SubSystemId == 0) && ((SgMode == 0) || (SgMode == 3))) {
        ///
        /// Program SubsystemID for IGFX When IGD is enabled
        ///
        Data32 = (Data32 & 0x0000FFFF) | (IGD_SSID << 16);
        DEBUG ((DEBUG_INFO, "IGD:: Program SDID [Subsystem ID] for IGFX: 0x%x\n", Data32));

      } else if ((SsidTablePtr[Index].SvidSidValue.SubSystemId == 0) && (SgMode == 2)) {
          ///
          /// Program SubsystemID for IGFX When SG is enabled
          ///
          Data32 = (Data32 & 0x0000FFFF) | (SG_SSID << 16);
          DEBUG ((DEBUG_INFO, "SG:: Program SDID [Subsystem ID] for IGFX: 0x%x\n", Data32));
      } else {
        ///
        /// Program SubsystemID for IGFX
        ///
          DEBUG ((DEBUG_INFO, "Program SDID [Subsystem ID] for IGFX: 0x%x\n", Data32));
      }
      PciSegmentWrite32 (
        SsidTablePtr[Index].Address.SegBusDevFuncRegister,
        Data32
        );
    }
  }

  return;
}

/**
  Slicon Initializes after PostMem phase Policy PPI produced,
  All required polices must be installed before the callback

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  SiPolicy PPI.

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_UNSUPPORTED         The function failed to locate SiPolicy
**/
EFI_STATUS
EFIAPI
SiInitPostMemOnPolicy (
  IN  EFI_PEI_SERVICES             **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN  VOID                         *Ppi
  )
{
  SI_POLICY_PPI             *SiPolicy;
  VOID                      *HobPtr;
  EFI_STATUS                Status;
  CPU_CONFIG                *CpuConfig;
  SI_CONFIG                 *SiConfig;
  GRAPHICS_PEI_CONFIG       *GtConfig;


  DEBUG ((DEBUG_INFO, "SiInit () - Start\n"));
  SiPolicy = (SI_POLICY_PPI*) Ppi;

  Status = GetConfigBlock((VOID *)SiPolicy, &gGraphicsPeiConfigGuid, (VOID *)&GtConfig);
  ASSERT_EFI_ERROR(Status);
  Status = GetConfigBlock((VOID *)SiPolicy, &gCpuConfigGuid, (VOID *)&CpuConfig);
  ASSERT_EFI_ERROR(Status);
  Status = GetConfigBlock ((VOID *) SiPolicy, &gSiConfigGuid, (VOID *) &SiConfig);
  ASSERT_EFI_ERROR(Status);

  HobPtr = BuildGuidDataHob (&gSiConfigHobGuid, SiConfig, sizeof (SI_CONFIG));
  ASSERT (HobPtr != 0);

  //
  // SA Early Post Mem initialization
  //
  SaEarlyInitPostMem  (GtConfig);

  ///
  /// SubsystemID programming on Internal Devices.
  ///
  DEBUG ((DEBUG_INFO, "SubsystemID programming on Internal Devices\n"));
  ProgramSvidSid ();

  //
  // Perform ME post mem init. Done before PCH Init as PCH Init required MBP data
  //
  PERF_START_EX (&gPerfMePostMemGuid, NULL, NULL, AsmReadTsc (), 0x40B0);
  MePostMemInit ();
  PERF_END_EX (&gPerfMePostMemGuid, NULL, NULL, AsmReadTsc (), 0x40B1);

  //
  // Initializes PCH after memory services initialized
  //
  PERF_START_EX (&gPerfPchPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4020);
  PchInit (SiPolicy);
  PERF_END_EX (&gPerfPchPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4021);

  //
  // SA Post Mem initialization
  //
  PERF_START_EX (&gPerfSaPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4030);
  SaInit ();
  PERF_END_EX (&gPerfSaPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4031);

#ifdef CPU_CFL
  //
  // SA Security Lock down after all initialization done
  //
  PERF_START_EX (&gPerfSaSecLockPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4050);
  SaSecurityLock ();
  PERF_END_EX (&gPerfSaSecLockPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4051);
#endif


  //
  // Initialize processor features, performance and power management features,
  // BIOS GUARD, and overclocking etc features before RESET_CPL at post-memory phase.
  //
  PERF_START_EX (&gPerfCpuPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4080);
  CpuInit ((CONST EFI_PEI_SERVICES **) PeiServices, SiPolicy);
  PERF_END_EX (&gPerfCpuPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4081);

  //
  // Perform AMT post mem init
  //
  PERF_START_EX (&gPerfAmtPostMemGuid, NULL, NULL, AsmReadTsc (), 0x40C0);
  AmtPostMemInit ();
  PERF_END_EX (&gPerfAmtPostMemGuid, NULL, NULL, AsmReadTsc (), 0x40C1);

#ifndef CPU_CFL
  //
  // SA Security Lock down after all initialization done
  //
  PERF_START_EX (&gPerfSaSecLockPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4050);
  SaSecurityLock ();
  PERF_END_EX (&gPerfSaSecLockPostMemGuid, NULL, NULL, AsmReadTsc (), 0x4051);
#endif

  //

  //
  // Install EndOfPei callback function.
  //
  Status = PeiServicesNotifyPpi (mSiInitNotifyList);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((DEBUG_INFO, "SiInit () - End\n"));

#ifndef FSP_FLAG
  //
  // End of SiInit notification event
  //
  Status = PeiServicesInstallPpi (&mEndOfSiInit);
  ASSERT_EFI_ERROR (Status);
#endif // FSP_FLAG

  return EFI_SUCCESS;
}

/**
  Silicon Initializes after memory services initialized

  @param[in] FileHandle           The file handle of the file, Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The function completes successfully
**/
EFI_STATUS
EFIAPI
SiInit (
  IN  EFI_PEI_FILE_HANDLE               FileHandle,
  IN CONST EFI_PEI_SERVICES             **PeiServices
  )
{
  EFI_STATUS                Status;

  //
  // Install PostMem phase OnPolicyInstalled callback function.
  //
  Status = PeiServicesNotifyPpi (mSiInitPostMemNotifyList);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
