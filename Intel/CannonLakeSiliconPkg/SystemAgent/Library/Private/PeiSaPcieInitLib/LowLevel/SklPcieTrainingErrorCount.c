/** @file
  Error Counting for PEG training.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2016 Intel Corporation.

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

#include "SaPegLowLevel.h"
#include <SystemAgent/Library/Private/PeiSaPcieInitLib/PcieTraining.h>

///
/// Function Declarations
///
VOID
SklInitMonitor (
  IN UINT32 MchBar,
  IN UINT32 DmiBar
  );

VOID
SklTearDownMonitor (
  IN UINT32 MchBar,
  IN UINT32 DmiBar
  );

UINT32
SklEnableMonitor (
  VOID
  );

VOID
SklDisableMonitor (
  VOID
  );

VOID
SklFullMonitorReset (
  IN UINT32 MonitorPort
  );

VOID
SklProgramMonitor (
  IN UINT32 MonitorPort
  );

/**
  Open port for monitor

  @param[in]  This                          - Low level function table

  @retval Monitor Port
**/
UINT32
EFIAPI
SklOpenMonitor (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;
  UINT32                          MonitorPort;

  PciePrivate = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;

  SklInitMonitor ((UINT32) PciePrivate->MchBar, (UINT32) PciePrivate->DmiBar);
  MonitorPort = SklEnableMonitor ();
  SklFullMonitorReset (MonitorPort);
  SklProgramMonitor (MonitorPort);

  return MonitorPort;
}

/**
  Close port for monitor

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
**/
VOID
EFIAPI
SklCloseMonitor (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN UINT32                             MonitorPort
  )
{
  SA_PCIE_PRIVATE_FUNCTION_CALLS  *PciePrivate;

  PciePrivate = (SA_PCIE_PRIVATE_FUNCTION_CALLS*) This->PrivateData;
  SklFullMonitorReset (MonitorPort);
  SklDisableMonitor ();
  SklTearDownMonitor ((UINT32) PciePrivate->MchBar, (UINT32) PciePrivate->DmiBar);

  return;
}

/**
  Get Current Error Count

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
  @param[in]  PciePort                      - PCIe Root Port
**/
UINT32
EFIAPI
SklSaPcieGetErrorCount (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT32                            MonitorPort,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT32 Data32;

  Data32 = MmioRead32 (MonitorPort + (0xC + (PciePort->Function * 0x10)));

  return Data32;
}

/**
  Get Current Error Count for DMI

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
  @param[in]  PciePort                      - Unused
**/
UINT32
EFIAPI
SklSaDmiGetErrorCount (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT32                            MonitorPort,
  IN  PCIE_PORT_INFO                    *PciePort
  )
{
  UINT32 Data32;

  Data32 = MmioRead32 (MonitorPort + 0x3C);

  return Data32;
}

/**
  Clear Current Error Count for all Root Ports

  @param[in]  This                          - Low level function table
  @param[in]  MonitorPort                   - Monitor Port
**/
VOID
EFIAPI
SklSaPcieClearErrorCount (
  IN  PCIE_SI_LOW_LEVEL_FUNCTION_CALLS  *This,
  IN  UINT32                            MonitorPort
  )
{
  SklFullMonitorReset (MonitorPort);
  SklProgramMonitor (MonitorPort);

  return;
}

VOID
SklInitMonitor (
  IN UINT32 MchBar,
  IN UINT32 DmiBar
  )
{
  ///
  ///
  UINT64 Peg0BaseAddress;
  UINT64 Peg1BaseAddress;
  UINT64 Peg2BaseAddress;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, 0);
  Peg1BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 1, 0);
  Peg2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 2, 0);
  ///
  ///
  MmioWrite32 (MchBar + 0x6430, 0x00000003);
  ///
  ///
  MmioWrite32 (MchBar + 0x643C, 0x00000000);
  ///
  ///
  MmioWrite32 (MchBar + 0x6434, 0x76543210);
  ///
  ///
  MmioWrite32 (MchBar + 0x7168, 0x00000007);
  ///
  ///
  MmioWrite32 (MchBar + 0x7198, 0x0000002B);
  ///
  ///
  MmioWrite32 (MchBar + 0x7180, 0x76543210);
  ///
  ///
  MmioWrite32 (MchBar + 0x7170, 0x00000007);
  ///
  ///
  MmioWrite32 (MchBar + 0x7194, 0x00000000);
  ///
  ///
  MmioWrite32 (MchBar + 0x7184, 0x76543210);
  ///
  ///
  MmioWrite32 (DmiBar + 0xC40, 0x0000000B);
  ///
  ///
  MmioWrite32 (DmiBar + 0xCB0, 0x00000000);
  ///
  ///
  MmioWrite32 (DmiBar + 0xC44, 0x76540210);
  ///
  ///
  MmioWrite32 (MchBar + 0x718C, 0x0000000B);
  ///
  ///
  MmioWrite32 (MchBar + 0x719C, 0x00000000);
  ///
  ///
  MmioWrite32 (MchBar + 0x7188, 0x76543210);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC70, 0x00000003);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC7C, 0x00000000);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC40, 0x00000007);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xCB0, 0x00000029);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC44, 0x76543210);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xC40, 0x00000007);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xCB0, 0x00000029);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xC44, 0x76543200);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xC40, 0x0000000B);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xCB0, 0x00000000);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xC44, 0x76543010);
  ///
  ///
  MmioWrite32 (MchBar + 0x6438, 0x014A0000);

  return;
}

VOID
SklTearDownMonitor (
  IN UINT32 MchBar,
  IN UINT32 DmiBar
  )
{
  UINT64 Peg0BaseAddress;
  UINT64 Peg1BaseAddress;
  UINT64 Peg2BaseAddress;

  Peg0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 0, 0);
  Peg1BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 1, 0);
  Peg2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 1, 2, 0);
  ///
  ///
  MmioWrite32 (MchBar + 0x6430, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x643C, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x6434, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7168, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7198, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7180, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7170, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7194, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7184, 0x0);
  ///
  ///
  MmioWrite32 (DmiBar + 0xC40, 0x0);
  ///
  ///
  MmioWrite32 (DmiBar + 0xCB0, 0x0);
  ///
  ///
  MmioWrite32 (DmiBar + 0xC44, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x718C, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x719C, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x7188, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC70, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC7C, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC40, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xCB0, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg0BaseAddress + 0xC44, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xC40, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xCB0, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg1BaseAddress + 0xC44, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xC40, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xCB0, 0x0);
  ///
  ///
  PciSegmentWrite32 (Peg2BaseAddress + 0xC44, 0x0);
  ///
  ///
  MmioWrite32 (MchBar + 0x6438, 0x0);

  return;
}

UINT32
SklEnableMonitor (
  VOID
  )
{
  UINT32 Data32;

  Data32 = (PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, 0140)) & ~(07));
  ///
  ///
  MmioOr32 (Data32 + 0124, 040000);
  ///
  ///
  MmioWrite64 (Data32 + 0700020, (UINT64) SA_PEI_MONITOR_OFFSET);
  ///
  ///
  MmioOr16 (Data32 + 0700004, 02);

  return SA_PEI_MONITOR_OFFSET;
}

VOID
SklDisableMonitor (
  VOID
  )
{
  UINT32 Data32;

  Data32 = (PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, 0140)) & ~(07));
  ///
  ///
  MmioAnd16 (Data32 + 0700004, 0177775);
  ///
  ///
  MmioAnd64 (Data32 + 0700020, 07777);
  ///
  ///
  MmioAnd32 (Data32 + 0124, 037777737777);

  return;
}

VOID
SklFullMonitorReset (
  IN UINT32 MonitorPort
  )
{
  ///
  ///
  MmioWrite32 (MonitorPort + 0x0, 0x40000);
  MmioWrite32 (MonitorPort + 0x4, 0x0);
  MmioWrite32 (MonitorPort + 0x10, 0x40000);
  MmioWrite32 (MonitorPort + 0x14, 0x0);
  MmioWrite32 (MonitorPort + 0x20, 0x40000);
  MmioWrite32 (MonitorPort + 0x24, 0x0);
  MmioWrite32 (MonitorPort + 0x30, 0x40000);
  MmioWrite32 (MonitorPort + 0x34, 0x0);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x8, 0xFF000000);
  MmioWrite32 (MonitorPort + 0x18, 0xFF000000);
  MmioWrite32 (MonitorPort + 0x28, 0xFF000000);
  MmioWrite32 (MonitorPort + 0x38, 0xFF000000);
  ///
  ///
  MmioWrite32 (MonitorPort + 0xC, 0x0);
  MmioWrite32 (MonitorPort + 0x1C, 0x0);
  MmioWrite32 (MonitorPort + 0x2C, 0x0);
  MmioWrite32 (MonitorPort + 0x3C, 0x0);

  return;
}

VOID
SklProgramMonitor (
  IN       UINT32                       MonitorPort
  )
{
  ///
  ///
  ///
  ///
  MmioWrite32 (MonitorPort + 0x4, 0xA100);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x14, 0xA101);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x24, 0xA102);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x34, 0xA103);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x0, 0x110000);
  ///
  ///
  MicroSecondDelay (1 * STALL_ONE_MILLI_SECOND);

  ///
  ///
  ///
  ///
  MmioWrite32 (MonitorPort + 0x0, 0x22100);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x10, 0x22101);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x20, 0x22102);
  ///
  ///
  MmioWrite32 (MonitorPort + 0x30, 0x22103);

  return;
}
