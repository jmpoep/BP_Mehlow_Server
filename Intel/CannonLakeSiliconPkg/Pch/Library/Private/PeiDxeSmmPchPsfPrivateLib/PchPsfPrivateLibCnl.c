/** @file
  This file contains internal PSF routines for PCH PSF CNL lib usage

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2017 Intel Corporation.

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
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PchPcrLib.h>
#include <Library/PchInfoLib.h>
#include <Library/SataLib.h>
#include <Library/BaseMemoryLib.h>
#include <Register/PchRegsPsf.h>
#include <Register/PchRegsPsfCnl.h>
#include <Register/PchRegsPcie.h>
#include "PchPsfPrivateLibInternal.h"

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpSerialIoI2cPsfRegs[] =
{
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C0_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C1_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C2_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C3_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C4_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_I2C5_REG_BASE
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHSerialIoI2cPsfRegs[] =
{
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C0_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C1_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C2_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_I2C3_REG_BASE
};

/**
  Return PSF_PORT for SerialIO I2C device

  @param[in] I2cNum  Serial IO I2C device (I2C0, I2C1, ....)

  @retval  PsfPort   PSF PORT structure for SerialIO I2C device
**/
PSF_PORT
PsfSerialIoI2cPort (
  IN UINT32  I2cNum
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    if (I2cNum < ARRAY_SIZE(mPchLpSerialIoI2cPsfRegs)) {
      PsfPort.RegBase = mPchLpSerialIoI2cPsfRegs[I2cNum];
      return PsfPort;
    }
  } else {
    if (I2cNum < ARRAY_SIZE(mPchHSerialIoI2cPsfRegs)) {
      PsfPort.RegBase = mPchHSerialIoI2cPsfRegs[I2cNum];
      return PsfPort;
    }
  }

  ASSERT(FALSE);
  return PSF_PORT_NULL;
}

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpSerialIoSpiPsfRegs[] =
{
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI0_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI1_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SPI2_REG_BASE
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHSerialIoSpiPsfRegs[] =
{
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI0_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI1_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SPI2_REG_BASE
};

/**
  Return PSF_PORT for SerialIO SPI device

  @param[in] SpiNum  Serial IO SPI device (SPI0, SPI1, ....)

  @retval  PsfPort   PSF PORT structure for SerialIO SPI device
**/
PSF_PORT
PsfSerialIoSpiPort (
  IN UINT32  SpiNum
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    if (SpiNum < ARRAY_SIZE(mPchLpSerialIoSpiPsfRegs)) {
      PsfPort.RegBase = mPchLpSerialIoSpiPsfRegs[SpiNum];
      return PsfPort;
    }
  } else {
    if (SpiNum < ARRAY_SIZE(mPchHSerialIoSpiPsfRegs)) {
      PsfPort.RegBase = mPchHSerialIoSpiPsfRegs[SpiNum];
      return PsfPort;
    }
  }

  ASSERT(FALSE);
  return PSF_PORT_NULL;
}

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpSerialIoUartPsfRegs[] =
{
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART0_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART1_REG_BASE,
  R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_UART2_REG_BASE
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHSerialIoUartPsfRegs[] =
{
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART0_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART1_REG_BASE,
  R_CNL_PCH_H_PSF3_PCR_T0_SHDW_UART2_REG_BASE
};

/**
  Return PSF_PORT for SerialIO UART device

  @param[in] UartNum  Serial IO UART device (UART0, UART1, ....)

  @retval  PsfPort    PSF PORT structure for SerialIO UART device
**/
PSF_PORT
PsfSerialIoUartPort (
  IN UINT32  UartNum
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    if (UartNum < ARRAY_SIZE(mPchLpSerialIoUartPsfRegs)) {
      PsfPort.RegBase = mPchLpSerialIoUartPsfRegs[UartNum];
      return PsfPort;
    }
  } else {
    if (UartNum < ARRAY_SIZE(mPchHSerialIoUartPsfRegs)) {
      PsfPort.RegBase = mPchHSerialIoUartPsfRegs[UartNum];
      return PsfPort;
    }
  }

  ASSERT(FALSE);
  return PSF_PORT_NULL;
}

/**
  Return PSF_PORT for TraceHub device

  @retval    PsfPort         PSF PORT structure for TraceHub device
**/
PSF_PORT
PsfTraceHubPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF2;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_TRACE_HUB_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF2_PCR_T0_SHDW_TRACE_HUB_REG_BASE;
  }
  return PsfPort;
}

/**
  Return PSF_PORT for TraceHub ACPI device

  @retval    PsfPort         PSF PORT structure for TraceHub ACPI device
**/
PSF_PORT
PsfTraceHubAcpiDevPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF2;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_TRACE_HUB_ACPI_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF2_PCR_T0_SHDW_TRACE_HUB_ACPI_REG_BASE;
  }
  return PsfPort;
}

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpHeciPsfRegBase[] =
{
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI1_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI2_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_HECI4_REG_BASE
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHHeciPsfRegBase[] =
{
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI1_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI2_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_HECI4_REG_BASE
};

/**
  Return PSF_PORT for HECI device

  @param[in] HeciDevice      HECIx Device (HECI1-4)

  @retval    PsfPort         PSF PORT structure for HECI device
**/
PSF_PORT
PsfHeciPort (
  IN UINT8      HeciDevice
  )
{
  PSF_PORT PsfPort;

  if ((HeciDevice < 1) || (HeciDevice > 4)) {
    ASSERT (FALSE);
    return PSF_PORT_NULL;
  }

  PsfPort.PsfPid = PID_PSF1;

  if (IsPchLp ()) {
    PsfPort.RegBase = mPchLpHeciPsfRegBase[HeciDevice - 1];
  } else {
    PsfPort.RegBase = mPchHHeciPsfRegBase[HeciDevice - 1];
  }
  return PsfPort;
}

/**
  Return PSF_PORT for SOL device

  @retval    PsfPort         PSF PORT structure for SOL device
**/
PSF_PORT
PsfSolPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF1;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_KT_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF1_PCR_T0_SHDW_KT_REG_BASE;
  }
  return PsfPort;
}

/**
  Disable IDER device at PSF level
**/
VOID
PsfDisableIderDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF1;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_IDER_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF1_PCR_T0_SHDW_IDER_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable HDAudio device at PSF level
**/
VOID
PsfDisableHdaDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_AUD_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_AUD_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable xDCI device at PSF level
**/
VOID
PsfDisableXdciDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF2;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_OTG_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF2_PCR_T0_SHDW_OTG_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable xHCI device at PSF level
**/
VOID
PsfDisableXhciDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF2;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_XHCI_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF2_PCR_T0_SHDW_XHCI_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable xHCI VTIO Phantom device at PSF level
**/
VOID
PsfDisableXhciVtioDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  if (IsPchLp () && (PchStepping () < PCH_B0)) {
    //
    // xHCI VTIO Phantom device did not exist on CNL-PCH-LP-A0
    //
    return;
  }

  PsfPort.PsfPid = PID_PSF2;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_XHCI_VTIO_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF2_PCR_T0_SHDW_XHCI_VTIO_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable SATA device at PSF level

  @param[in]  SataCtrlIndex     SATA controller index
**/
VOID
PsfDisableSataDevice (
  IN UINT32     SataCtrlIndex
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF1;

  if (IsCdfPch ()) {
    ASSERT (SataCtrlIndex < 3);
    if (SataCtrlIndex == SATA_1_CONTROLLER_INDEX) {
      PsfPort.RegBase = R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_1_REG_BASE;
    } else if (SataCtrlIndex == SATA_2_CONTROLLER_INDEX) {
      PsfPort.RegBase = R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_2_REG_BASE;
    } else if (SataCtrlIndex == SATA_3_CONTROLLER_INDEX) {
      PsfPort.RegBase = R_CDF_PCH_PSF1_PCR_T0_SHDW_SATA_3_REG_BASE;
    } else {
      ASSERT (FALSE);
      PsfPort.RegBase = 0;
    }
  } else {
    if (IsPchLp ()) {
      PsfPort.RegBase = R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_SATA_REG_BASE;
    } else {
      PsfPort.RegBase = R_CNL_PCH_H_PSF1_PCR_T0_SHDW_SATA_REG_BASE;
    }
  }

  PsfDisableDevice (PsfPort);
}

/**
  Program PSF grant counts for SATA
  Call this before SATA ports are accessed for enumeration
**/
VOID
PsfConfigureSataGrantCounts (
  VOID
  )
{
  //
  // PCH-LP:
  //   Set PSF_4_DEV_GNTCNT_RELOAD_DGCR2-5.GNTCNTRELOAD to 0x1
  //     VR    -> PSF_4_DEV_GNTCNT_RELOAD_DGCR2
  //     VS0-2 -> PSF_4_DEV_GNTCNT_RELOAD_DGCR3-5
  // PCH-H:
  //   Set PSF_4_DEV_GNTCNT_RELOAD_DGCR2-10.GNTCNTRELOAD to 0x1
  //     VR    -> PSF_4_DEV_GNTCNT_RELOAD_DGCR2
  //     VS0-7 -> PSF_4_DEV_GNTCNT_RELOAD_DGCR3-10
  //
  UINT16  Dgcr0Addr;
  UINT8   DgcrMinIndex;
  UINT8   DgcrMaxIndex;
  UINT8   DgcrIndex;

  if (IsPchLp ()) {
    Dgcr0Addr = R_CNL_PCH_LP_PSF4_PCR_DEV_GNTCNT_RELOAD_DGCR0;
    DgcrMinIndex = 2;
    DgcrMaxIndex = 5;
  } else {
    Dgcr0Addr = R_CNL_PCH_H_PSF4_PCR_DEV_GNTCNT_RELOAD_DGCR0;
    DgcrMinIndex = 2;
    DgcrMaxIndex = 10;
  }

  for (DgcrIndex = DgcrMinIndex; DgcrIndex <= DgcrMaxIndex; DgcrIndex++) {
    PchPcrAndThenOr32 (
      PID_PSF4,
      Dgcr0Addr  + (DgcrIndex * S_PCH_PSFX_PCR_DEV_GNTCNT_RELOAD_DGCR),
      (UINT32) ~B_PCH_PSFX_PCR_DEV_GNTCNT_RELOAD_DGCR_GNT_CNT_RELOAD,
      0x1
      );
  }
}

/**
  Return PSF_PORT for ISH device

  @retval    PsfPort         PSF PORT structure for ISH device
**/
PSF_PORT
PsfIshPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_ISH_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_ISH_REG_BASE;
  }
  return PsfPort;
}

/**
  Disable GbE device at PSF level
**/
VOID
PsfDisableGbeDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_GBE_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_GBE_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Disable SMBUS device at PSF level
**/
VOID
PsfDisableSmbusDevice (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SMBUS_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SMBUS_REG_BASE;
  }

  PsfDisableDevice (PsfPort);
}

/**
  Return PSF_PORT for SCS eMMC device

  @retval    PsfPort      PSF PORT structure for SCS eMMC device
**/
PSF_PORT
PsfScsEmmcPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_EMMC_REG_BASE;
    PsfPort.PsfPid = PID_PSF2;
    return PsfPort;
  }

  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

/**
  Return PSF_PORT for SCS SD Card device

  @retval    PsfPort      PSF PORT structure for SCS SD Card device
**/
PSF_PORT
PsfScsSdCardPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_SDCARD_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_SDCARD_REG_BASE;
  }
  return PsfPort;
}

/**
  Return PSF_PORT for SCS UFS device

  @param[in] UfsNum       UFS Device

  @retval    PsfPort      PSF PORT structure for SCS UFS device
**/
PSF_PORT
PsfScsUfsPort (
  IN UINT32  UfsNum
  )
{
  PSF_PORT PsfPort;

  if (IsPchLp () && (UfsNum == 0)) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF2_PCR_T0_SHDW_UFS_REG_BASE;
    PsfPort.PsfPid = PID_PSF2;
    return PsfPort;
  }

  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

//
// RS0 PSF1_T1_SHDW_PCIExx_REG_BASE registers for PCH-LP
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpPsf1PcieRegBase[] =
{
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE01_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE02_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE03_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE04_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE05_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE06_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE07_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE08_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE09_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE10_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE11_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE12_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE13_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE14_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE15_RS0_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T1_SHDW_PCIE16_RS0_REG_BASE
};

//
// RS3 PSF1_T0_SHDW_PCIExx_REG_BASE registers for PCH-LP
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpPsf1Rs3PcieRegBase[] =
{
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE01_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE02_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE03_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE04_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE05_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE06_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE07_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE08_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE09_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE10_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE11_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE12_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE13_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE14_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE15_RS3_REG_BASE,
  R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_PCIE16_RS3_REG_BASE
};

//
// RS0 PSF1_T1_SHDW_PCIExx_REG_BASE registers for PCH-H
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHPsf1PcieRegBase[] =
{
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE01_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE02_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE03_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE04_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE05_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE06_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE07_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE08_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE09_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE10_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE11_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE12_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE13_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE14_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE15_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE16_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE17_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE18_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE19_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE20_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE21_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE22_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE23_RS0_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T1_SHDW_PCIE24_RS0_REG_BASE
};

//
// RS3 PSF1_T0_SHDW_PCIExx_REG_BASE registers for PCH-H
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHPsf1Rs3PcieRegBase[] =
{
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE01_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE02_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE03_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE04_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE05_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE06_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE07_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE08_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE09_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE10_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE11_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE12_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE13_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE14_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE15_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE16_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE17_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE18_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE19_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE20_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE21_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE22_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE23_RS3_REG_BASE,
  R_CNL_PCH_H_PSF1_PCR_T0_SHDW_PCIE24_RS3_REG_BASE
};

GLOBAL_REMOVE_IF_UNREFERENCED PSF_PORT mPchHPsfPcieSecondLevelPort[] =
{
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE01_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE02_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE03_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE04_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE05_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE06_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE07_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE08_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE09_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE10_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE11_REG_BASE},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_SHDW_PCIE12_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE13_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE14_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE15_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE16_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE17_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE18_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE19_REG_BASE},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_SHDW_PCIE20_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE21_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE22_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE23_REG_BASE},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_SHDW_PCIE24_REG_BASE}
};

/**
  Return second level PSF_PORT to which PCIE Root Port device is connected (directly)

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe
**/
PSF_PORT
PsfPcieSecondLevelPort (
  IN UINT32  RpIndex
  )
{
  if (IsPchH ()) {
    if (RpIndex < ARRAY_SIZE(mPchHPsfPcieSecondLevelPort)) {
      return mPchHPsfPcieSecondLevelPort[RpIndex];
    }
    ASSERT (FALSE);
  }
  return PSF_PORT_NULL;
}

/**
  Return PSF_PORT at root PSF level to which PCIe Root Port device is connected

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe

**/
PSF_PORT
PsfRootPciePort (
  IN UINT32  RpIndex
  )
{
  PSF_PORT PsfPort;

  if (IsPchLp ()) {
    if (RpIndex < ARRAY_SIZE(mPchLpPsf1PcieRegBase)) {
      PsfPort.PsfPid = PID_PSF1;
      PsfPort.RegBase = mPchLpPsf1PcieRegBase[RpIndex];
      return PsfPort;
    }
  } else {
    if (RpIndex < ARRAY_SIZE(mPchHPsf1PcieRegBase)) {
      PsfPort.PsfPid = PID_PSF1;
      PsfPort.RegBase = mPchHPsf1PcieRegBase[RpIndex];
      return PsfPort;
    }
  }
  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

/**
  Return RS3 PSF_PORT at root PSF level to which PCIe Root Port device is connected

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe
**/
PSF_PORT
PsfRootRs3PciePort (
  IN UINT32  RpIndex
  )
{
  PSF_PORT PsfPort;

  if (IsPchLp ()) {
    if (RpIndex < ARRAY_SIZE(mPchLpPsf1Rs3PcieRegBase)) {
      PsfPort.PsfPid = PID_PSF1;
      PsfPort.RegBase = mPchLpPsf1Rs3PcieRegBase[RpIndex];
      return PsfPort;
    }
  } else {
    if (RpIndex < ARRAY_SIZE(mPchHPsf1Rs3PcieRegBase)) {
      PsfPort.PsfPid = PID_PSF1;
      PsfPort.RegBase = mPchHPsf1Rs3PcieRegBase[RpIndex];
      return PsfPort;
    }
  }
  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_GRANT_COUNT_REG mPchHGntCntRegs[] = {
    {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF6_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}, //SPA
    {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF8_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}, //SPB
    {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF6_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}, //SPC
    {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF7_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}, //SPD
    {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF7_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}, //SPE
    {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_DEV_GNTCNT_RELOAD_DGCR0, R_CNL_PCH_H_PSF8_PCR_TARGET_GNTCNT_RELOAD_PG1_TGT0}  //SPF
  };

/**
  Grant count regs data for PSF that is directly connected to PCIe Root Ports

  @param[in]  Controller     PCIe Root Port Controller index (0 based)
  @param[out] GrantCountReg  Structure with PSF Grant Count register data
**/
VOID
PsfPcieGrantCountBaseReg (
  IN  UINT8                Controller,
  OUT PSF_GRANT_COUNT_REG  *GrantCountReg
  )
{
  if (IsPchLp ()) {
    GrantCountReg->PsfPid = PID_PSF1;
    GrantCountReg->DevGntCnt0Base = R_CNL_PCH_LP_PSF1_DEV_GNTCNT_RELOAD_DGCR0;
    GrantCountReg->TargetGntCntPg1Tgt0Base = R_CNL_PCH_LP_PSF1_TARGET_GNTCNT_RELOAD_PG1_TGT0;
  } else {
    if (Controller < ARRAY_SIZE(mPchHGntCntRegs)) {
      GrantCountReg->PsfPid = mPchHGntCntRegs[Controller].PsfPid;
      GrantCountReg->DevGntCnt0Base = mPchHGntCntRegs[Controller].DevGntCnt0Base;
      GrantCountReg->TargetGntCntPg1Tgt0Base = mPchHGntCntRegs[Controller].TargetGntCntPg1Tgt0Base;
    }
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 mDgcrLp [][PCH_PCIE_CONTROLLER_PORTS] = {
    {  8,  9, 10, 11 }, //SPA
    { 12, 13, 14, 15 }, //SPB
    { 16, 17, 18, 19 }, //SPC
    { 20, 21, 22, 23 }  //SPD
  };

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 mPgTgtLp [][PCH_PCIE_CONTROLLER_PORTS] = {
    {  0,  1,  2,  3 }, //SPA
    {  8,  9, 10, 11 }, //SPB
    { 16, 17, 18, 19 }, //SPC
    { 24, 25, 26, 27 }  //SPD
  };

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 mDgcrH [][PCH_PCIE_CONTROLLER_PORTS] = {
    {  2,  3,  4,  5 }, //SPA
    {  2,  3,  4,  5 }, //SPB
    {  6,  7,  8,  9 }, //SPC
    {  2,  3,  4,  5 }, //SPD
    {  6,  7,  8,  9 }, //SPE
    {  6,  7,  8,  9 }  //SPF
  };

GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 mPgTgtH [][PCH_PCIE_CONTROLLER_PORTS] = {
    {  0,  1,  2,  3 }, //SPA
    {  0,  1,  2,  3 }, //SPB
    {  4,  5,  6,  7 }, //SPC
    {  0,  1,  2,  3 }, //SPD
    {  4,  5,  6,  7 }, //SPE
    {  4,  5,  6,  7 }  //SPF
  };

/**
  Get Grant Count number (Device Grant Count and Target Grant Count)
  for PSF that is directly connected to PCIe Root Ports

  @param[in]  Controller    PCIe Root Port Controller index
  @param[in]  Channel       PCIe Root Port Channel index
  @param[out] DgcrNo        Device Grant Count number
  @param[out] PgTgtNo       Target Grant Count number
**/
VOID
PsfPcieGrantCountNumber (
  IN  UINT8 Controller,
  IN  UINT8 Channel,
  OUT UINT8 *DgcrNo,
  OUT UINT8 *PgTgtNo
  )
{
  if (IsPchLp ()) {
    if ((Channel < ARRAY_SIZE(mDgcrLp[0])) && (Controller < ARRAY_SIZE(mDgcrLp))) {
      *DgcrNo = mDgcrLp[Controller][Channel];
      *PgTgtNo = mPgTgtLp[Controller][Channel];
      return;
    }
  } else {
    if ((Channel < ARRAY_SIZE(mDgcrH[0])) && (Controller < ARRAY_SIZE(mDgcrH))) {
      *DgcrNo = mDgcrH[Controller][Channel];
      *PgTgtNo = mPgTgtH[Controller][Channel];
      return;
    }
  }
  ASSERT (FALSE);
}

/**
  Grant count regs data for a given PSF-to-PSF port.

  @param[in] PsfTopoPort         PSF-to-PSF port

  @param[out] GrantCountReg      Structure with PSF Grant Count register data
**/
VOID
PsfSegmentGrantCountBaseReg (
  IN PSF_TOPO_PORT         PsfTopoPort,
  OUT PSF_GRANT_COUNT_REG  *GrantCountReg
  )
{
  if (IsPchH () &&
     (PsfTopoPort.PsfId == 1) &&
     ((PsfTopoPort.PortId == 0) || (PsfTopoPort.PortId == 1) || (PsfTopoPort.PortId == 2))) {
    GrantCountReg->PsfPid = PID_PSF1;
    GrantCountReg->DevGntCnt0Base = R_CNL_PCH_H_PSF1_DEV_GNTCNT_RELOAD_DGCR0;
    GrantCountReg->TargetGntCntPg1Tgt0Base = R_CNL_PCH_H_PSF1_TARGET_GNTCNT_RELOAD_PG1_TGT0;
  } else {
    ASSERT (FALSE);
  }
}

/**
  Grant Count number (Device Grant Count and Target Grant Count) for a given PSF-to-PSF port.

  @param[in] PsfTopoPort         PSF-to-PSF port
  @param[out] DgcrNo             Device Grant Count number
  @param[out] PgTgtNo            Target Grant Count number
**/
VOID
PsfSegmentGrantCountNumber (
  IN PSF_TOPO_PORT PsfTopoPort,
  OUT UINT8        *DgcrNo,
  OUT UINT8        *PgTgtNo
  )
{
  *DgcrNo = 0;
  *PgTgtNo = 0;

  ASSERT (IsPchH () && (PsfTopoPort.PsfId == 1));

  switch (PsfTopoPort.PortId) {
    case 0: //PSF6
      *DgcrNo = 5;
      *PgTgtNo = 0;
      break;
    case 1: //PSF7
      *DgcrNo = 6;
      *PgTgtNo = 1;
      break;
    case 2: //PSF8
      *DgcrNo = 7;
      *PgTgtNo = 2;
      break;
    default:
      ASSERT (FALSE);
      break;
  }
}

//
// PCH-LP PSF topology for PCIe
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchLpPsf1PcieTreeTopo[] = {
  //{PsfId, Port}, PortType, Child, PcieCtrlIndex
  {{1, 0}, PsfPcieCtrlPort, NULL, {0}}, // SPA
  {{1, 2}, PsfPcieCtrlPort, NULL, {1}}, // SPB
  {{1, 4}, PsfPcieCtrlPort, NULL, {2}}, // SPC
  {{1, 6}, PsfPcieCtrlPort, NULL, {3}}, // SPD
  PSF_TOPOLOGY_END
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchLpPsfPcieTreeTopo[] = {
  {{0, 0}, PsfToPsfPort, mPchLpPsf1PcieTreeTopo}, // Abstract root of PSF topology
  PSF_TOPOLOGY_END
};

//
// PCH-H PSF topology for PCIe
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchHPsf6PcieTreeTopo[] = {
  //{PsfId, Port}, PortType, Child, PcieCtrlIndex
  {{6, 0}, PsfPcieCtrlPort, NULL, {0}}, // SPA
  {{6, 1}, PsfPcieCtrlPort, NULL, {2}}, // SPC
  PSF_TOPOLOGY_END
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchHPsf7PcieTreeTopo[] = {
  //{PsfId, Port}, PortType, Child, PcieCtrlIndex
  {{7, 0}, PsfPcieCtrlPort, NULL, {3}}, // SPD
  {{7, 1}, PsfPcieCtrlPort, NULL, {4}}, // SPE
  PSF_TOPOLOGY_END
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchHPsf8PcieTreeTopo[] = {
  //{PsfId, Port}, PortType, Child, PcieCtrlIndex
  {{8, 0}, PsfPcieCtrlPort, NULL, {1}}, // SPB
  {{8, 1}, PsfPcieCtrlPort, NULL, {5}}, // SPF
  PSF_TOPOLOGY_END
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchHPsf1PcieTreeTopo[] = {
  //{PsfId, Port}, PortType, Child;
  {{1, 0}, PsfToPsfPort, mPchHPsf6PcieTreeTopo},
  {{1, 1}, PsfToPsfPort, mPchHPsf7PcieTreeTopo},
  {{1, 2}, PsfToPsfPort, mPchHPsf8PcieTreeTopo},
  PSF_TOPOLOGY_END
};

GLOBAL_REMOVE_IF_UNREFERENCED CONST PSF_TOPOLOGY mPchHPsfPcieTreeTopo[] = {
  {{0, 0}, PsfToPsfPort, mPchHPsf1PcieTreeTopo}, // Abstract root of PSF topology
  PSF_TOPOLOGY_END
};

/**
  Get PSF Pcie Tree topology

  @param[in] PsfTopology          PSF Port from PSF PCIe tree topology

  @retval PsfTopology             PSF PCIe tree topology
**/
CONST PSF_TOPOLOGY*
PsfGetRootPciePsfTopology (
  VOID
  )
{
  if (IsPchLp ()) {
    return mPchLpPsfPcieTreeTopo;
  } else {
    return mPchHPsfPcieTreeTopo;
  }
}

/**
  Get EOI register data for given PSF ID

  @param[in]  PsfId           PSF ID (1 - PSF1, 2 - PSF2, ...)
  @param[out] EoiTargetBase   EOI Target register
  @param[out] EoiControlBase  EOI Control register

  @retval MaxTargets          Number of supported targets

**/
UINT8
PsfEoiRegData (
  UINT32        PsfId,
  UINT16        *EoiTargetBase,
  UINT16        *EoiControlBase
  )
{
  UINT8  MaxTargets;

  MaxTargets = 0;
  *EoiTargetBase = 0;
  *EoiControlBase = 0;

  switch (PsfId) {
    case 1:
      if (IsPchLp ()) {
        *EoiTargetBase = R_CNL_PCH_LP_PSF1_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
        *EoiControlBase = R_CNL_PCH_LP_PSF1_PCR_PSF_MC_CONTROL_MCAST0_EOI;
        MaxTargets = 17;
      } else {
        *EoiTargetBase = R_CNL_PCH_H_PSF1_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
        *EoiControlBase = R_CNL_PCH_H_PSF1_PCR_PSF_MC_CONTROL_MCAST0_EOI;
        MaxTargets = 7;
      }
      break;

    case 3:
      *EoiTargetBase = R_CNL_PCH_PSF3_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
      *EoiControlBase = R_CNL_PCH_PSF3_PCR_PSF_MC_CONTROL_MCAST0_EOI;
      MaxTargets = 1;
      break;

    case 6:
      if (IsPchH ()) {
        *EoiTargetBase = R_CNL_PCH_H_PSF6_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
        *EoiControlBase = R_CNL_PCH_H_PSF6_PCR_PSF_MC_CONTROL_MCAST0_EOI;
        MaxTargets = 8;
      }
      break;

    case 7:
      if (IsPchH ()) {
        *EoiTargetBase = R_CNL_PCH_H_PSF7_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
        *EoiControlBase = R_CNL_PCH_H_PSF7_PCR_PSF_MC_CONTROL_MCAST0_EOI;
        MaxTargets = 8;
      }
      break;

    case 8:
      if (IsPchH ()) {
        *EoiTargetBase = R_CNL_PCH_H_PSF8_PCR_PSF_MC_AGENT_MCAST0_TGT0_EOI;
        *EoiControlBase = R_CNL_PCH_H_PSF8_PCR_PSF_MC_CONTROL_MCAST0_EOI;
        MaxTargets = 8;
      }
      break;
  }
  return MaxTargets;
}

/**
  Get MCTP register data for given PSF ID

  @param[in]  PsfId            PSF ID (1 - PSF1, 2 - PSF2, ...)
  @param[out] MctpTargetBase   MCTP Target register
  @param[out] MctpControlBase  MCTP Control register

  @retval MaxTargets           Number of supported targets

**/
UINT8
PsfMctpRegData (
  UINT32        PsfId,
  UINT16        *MctpTargetBase,
  UINT16        *MctpControlBase
  )
{
  UINT8  MaxTargets;

  MaxTargets = 0;
  *MctpTargetBase = 0;
  *MctpControlBase = 0;

  switch (PsfId) {
    case 1:
      if (IsPchLp ()) {
        *MctpTargetBase = R_CNL_PCH_LP_PSF1_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1;
        *MctpControlBase = R_CNL_PCH_LP_PSF1_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1;
        MaxTargets = 17;
      } else {
        *MctpTargetBase = R_CNL_PCH_H_PSF1_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1;
        *MctpControlBase = R_CNL_PCH_H_PSF1_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1;
        MaxTargets = 7;
      }
      break;

    case 6:
      if (IsPchH ()) {
        *MctpTargetBase = R_CNL_PCH_H_PSF6_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1;
        *MctpControlBase = R_CNL_PCH_H_PSF6_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1;
        MaxTargets = 8;
      }
      break;

    case 7:
      if (IsPchH ()) {
        *MctpTargetBase = R_CNL_PCH_H_PSF7_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1;
        *MctpControlBase = R_CNL_PCH_H_PSF7_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1;
        MaxTargets = 8;
      }
      break;

    case 8:
      if (IsPchH ()) {
        *MctpTargetBase = R_CNL_PCH_H_PSF8_PCR_PSF_MC_AGENT_MCAST1_RS0_TGT0_MCTP1;
        *MctpControlBase = R_CNL_PCH_H_PSF8_PCR_PSF_MC_CONTROL_MCAST1_RS0_MCTP1;
        MaxTargets = 8;
      }
      break;
  }
  return MaxTargets;
}

/**
  P2SB PSF port Destination ID (psf_id:port_group_id:port_id:channel_id)

  @retval P2SB Destination ID
**/
PSF_PORT_DEST_ID
PsfP2sbDestinationId (
  VOID
  )
{
  PSF_PORT_DEST_ID P2sbTarget;

  if (IsPchLp ()) {
    P2sbTarget.RegVal = V_CNL_PCH_LP_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_P2SB;
  } else {
    P2sbTarget.RegVal = V_CNL_PCH_H_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_P2SB;
  }
  return P2sbTarget;
}

/**
  DMI PSF port Destination ID (psf_id:port_group_id:port_id:channel_id)

  @retval DMI Destination ID
**/
PSF_PORT_DEST_ID
PsfDmiDestinationId (
  VOID
  )
{
  PSF_PORT_DEST_ID DmiTarget;

  DmiTarget.RegVal = V_CNL_PCH_PSFX_PCR_PSF_MC_AGENT_MCAST_TGT_DMI;

  return DmiTarget;
}

GLOBAL_REMOVE_IF_UNREFERENCED PSF_PORT_DEST_ID PchLpRpDestId[] =
{
  {0x18000}, {0x18001}, {0x18002}, {0x18003}, // SPA: PSF1, PortID = 0
  {0x18200}, {0x18201}, {0x18202}, {0x18203}, // SPB: PSF1, PortID = 2
  {0x18400}, {0x18401}, {0x18402}, {0x18403}, // SPC: PSF1, PortID = 4
  {0x18600}, {0x18601}, {0x18602}, {0x18603}  // SPD: PSF1, PortID = 6
};

GLOBAL_REMOVE_IF_UNREFERENCED PSF_PORT_DEST_ID PchHRpDestId[] =
{
  {0x68000}, {0x68001}, {0x68002}, {0x68003}, // SPA: PSF6, PortID = 0
  {0x88000}, {0x88001}, {0x88002}, {0x88003}, // SPB: PSF8, PortID = 0
  {0x68100}, {0x68101}, {0x68102}, {0x68103}, // SPC: PSF6, PortID = 1
  {0x78000}, {0x78001}, {0x78002}, {0x78003}, // SPD: PSF7, PortID = 0
  {0x78100}, {0x78101}, {0x78102}, {0x78103}, // SPE: PSF7, PortID = 1
  {0x88100}, {0x88101}, {0x88102}, {0x88103}  // SPF: PSF8, PortID = 1
};

/**
  PCIe PSF port destination ID (psf_id:port_group_id:port_id:channel_id)

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval Destination ID
**/
PSF_PORT_DEST_ID
PsfPcieDestinationId (
  IN UINT32  RpIndex
  )
{
  if (IsPchLp ()) {
    if (RpIndex < ARRAY_SIZE(PchLpRpDestId)) {
      return PchLpRpDestId[RpIndex];
    }
  } else {
    if (RpIndex < ARRAY_SIZE(PchHRpDestId)) {
      return PchHRpDestId[RpIndex];
    }
  }
  ASSERT (FALSE);
  return (PSF_PORT_DEST_ID){0};
}

//
// RS0 PSF1_T1_AGENT_FUNCTION_CONFIG_SPx_Dxx_Fx registers for PCH-LP
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpPsf1PcieFuncCfgBase[] =
{
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F0,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F1,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F2,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F3,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F4,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F5,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F6,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F7,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F0,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F1,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F2,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F3,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F4,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F5,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F6,
  R_CNL_PCH_LP_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F7
};
//
// RS3 PSF1_T0_AGENT_FUNCTION_CONFIG_SPx_Dxx_Fx registers for PCH-LP
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchLpPsf1Rs3PcieFuncCfgBase[] =
{
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F0,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F1,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F2,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F3,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F4,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F5,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F6,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F7,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F0,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F1,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F2,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F3,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F4,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F5,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F6,
  R_CNL_PCH_LP_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F7
};

//
// RS0 PSF1_T1_AGENT_FUNCTION_CONFIG_SPx_Dxx_Fx registers for PCH-H
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHPsf1PcieFuncCfgBase[] =
{
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F0,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F1,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F2,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_RS0_D28_F3,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F4,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F5,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F6,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_RS0_D28_F7,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F0,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F1,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F2,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_RS0_D29_F3,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F4,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F5,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F6,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_RS0_D29_F7,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F0,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F1,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F2,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_RS0_D27_F3,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F4,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F5,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F6,
  R_CNL_PCH_H_PSF1_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_RS0_D27_F7
};

//
// RS3 PSF1_T0_AGENT_FUNCTION_CONFIG_SPx_Dxx_Fx registers for PCH-H
//
GLOBAL_REMOVE_IF_UNREFERENCED UINT16 mPchHPsf1Rs3PcieFuncCfgBase[] =
{
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F0,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F1,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F2,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPA_RS3_D28_F3,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F4,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F5,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F6,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPB_RS3_D28_F7,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F0,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F1,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F2,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPC_RS3_D29_F3,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F4,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F5,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F6,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPD_RS3_D29_F7,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F0,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F1,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F2,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPE_RS3_D27_F3,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F4,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F5,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F6,
  R_CNL_PCH_H_PSF1_PCR_T0_AGENT_FUNCTION_CONFIG_SPF_RS3_D27_F7
};

GLOBAL_REMOVE_IF_UNREFERENCED PSF_PORT mPchHPsfPcieFuncCfgSecondLevelPort[] =
{
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F0},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F1},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F2},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPA_D28_F3},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F4},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F5},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F6},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPB_D28_F7},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F0},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F1},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F2},
  {PID_PSF6, R_CNL_PCH_H_PSF6_PCR_T1_AGENT_FUNCTION_CONFIG_SPC_D29_F3},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F4},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F5},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F6},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPD_D29_F7},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F0},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F1},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F2},
  {PID_PSF7, R_CNL_PCH_H_PSF7_PCR_T1_AGENT_FUNCTION_CONFIG_SPE_D27_F3},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F4},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F5},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F6},
  {PID_PSF8, R_CNL_PCH_H_PSF8_PCR_T1_AGENT_FUNCTION_CONFIG_SPF_D27_F7}
};

/**
  Return the PSF (Root level) Function Config PSF_PORT for PCIe Root Port

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe Function Config
**/
PSF_PORT
PsfRootPcieFunctionConfigPort (
  IN UINT32  RpIndex
  )
{
  PSF_PORT  PsfPort;

  PsfPort.PsfPid = PID_PSF1;

  if (IsPchLp ()) {
    if (RpIndex < ARRAY_SIZE(mPchLpPsf1PcieFuncCfgBase)) {
      PsfPort.RegBase = mPchLpPsf1PcieFuncCfgBase[RpIndex];
      return PsfPort;
    }
  } else {
    if (RpIndex < ARRAY_SIZE(mPchHPsf1PcieFuncCfgBase)) {
      PsfPort.RegBase = mPchHPsf1PcieFuncCfgBase[RpIndex];
      return PsfPort;
    }
  }
  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

/**
  Return the PSF (Root level) RS3 Function Config PSF_PORT for PCIe Root Port

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe Function Config
**/
PSF_PORT
PsfRootRs3PcieFunctionConfigPort (
  IN UINT32  RpIndex
  )
{
  PSF_PORT  PsfPort;

  PsfPort.PsfPid = PID_PSF1;

  if (IsPchLp ()) {
    if (RpIndex < ARRAY_SIZE(mPchLpPsf1Rs3PcieFuncCfgBase)) {
      PsfPort.RegBase = mPchLpPsf1Rs3PcieFuncCfgBase[RpIndex];
      return PsfPort;
    }
  } else {
    if (RpIndex < ARRAY_SIZE(mPchHPsf1Rs3PcieFuncCfgBase)) {
      PsfPort.RegBase = mPchHPsf1Rs3PcieFuncCfgBase[RpIndex];
      return PsfPort;
    }
  }
  ASSERT (FALSE);
  return PSF_PORT_NULL;
}

/**
  Return the PSF Function Config Second Level PSF_PORT for PCIe Root Port

  @param[in] RpIndex        PCIe Root Port Index (0 based)

  @retval    PsfPort        PSF PORT structure for PCIe Function Config
**/
PSF_PORT
PsfPcieFunctionConfigSecondLevelPort (
  IN UINT32  RpIndex
  )
{
  if (IsPchH ()) {
    if (RpIndex < ARRAY_SIZE(mPchHPsfPcieFuncCfgSecondLevelPort)) {
      return mPchHPsfPcieFuncCfgSecondLevelPort[RpIndex];
    }
    ASSERT (FALSE);
  }

  return PSF_PORT_NULL;
}

/**
  This function enables PCIe Relaxed Order in PSF
**/
VOID
PsfEnablePcieRelaxedOrder (
  VOID
  )
{
  //
  // PCH BIOS Spec Section 8.2.9
  //
  PchPcrAndThenOr32 (
    PID_PSF1,
    R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG0_PORT0,
    ~0u,
    BIT1
    );

  if (IsPchH ()) {
    PchPcrAndThenOr32 (
      PID_PSF1,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT0,
      ~0u,
      BIT2
      );
    PchPcrAndThenOr32 (
      PID_PSF1,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT1,
      ~0u,
      BIT2
      );
    PchPcrAndThenOr32 (
      PID_PSF1,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG1_PORT2,
      ~0u,
      BIT2
      );
    PchPcrAndThenOr32 (
      PID_PSF6,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG0_PORT0,
      ~0u,
      BIT1
      );
    PchPcrAndThenOr32 (
      PID_PSF7,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG0_PORT0,
      ~0u,
      BIT1
      );
    PchPcrAndThenOr32 (
      PID_PSF8,
      R_CNL_PCH_PSF_PCR_PORT_CONFIG_PG0_PORT0,
      ~0u,
      BIT1
      );
  }
}

/**
  Return PSF_PORT for CNVi device

  @retval    PsfPort         PSF PORT structure for CNVi device
**/
PSF_PORT
PsfCnviPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_CNVI_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_CNVI_REG_BASE;
  }

  return PsfPort;
}

/**
  Return PSF_PORT for PMC device

  @retval    PsfPort         PSF PORT structure for PMC device
**/
PSF_PORT
PsfPmcPort (
  VOID
  )
{
  PSF_PORT PsfPort;

  PsfPort.PsfPid = PID_PSF3;

  if (IsPchLp ()) {
    PsfPort.RegBase = R_CNL_PCH_LP_PSF3_PCR_T0_SHDW_PMC_REG_BASE;
  } else {
    PsfPort.RegBase = R_CNL_PCH_H_PSF3_PCR_T0_SHDW_PMC_REG_BASE;
  }
  return PsfPort;
}

GLOBAL_REMOVE_IF_UNREFERENCED PSF_SEGMENT mPchLpPsfTable[] =
{
  {1, PID_PSF1},
  {2, PID_PSF2},
  {3, PID_PSF3},
  {4, PID_PSF4},
  {5, PID_CSME_PSF}
};

GLOBAL_REMOVE_IF_UNREFERENCED PSF_SEGMENT mPchHPsfTable[]  =
{
  {1, PID_PSF1},
  {2, PID_PSF2},
  {3, PID_PSF3},
  {4, PID_PSF4},
  {5, PID_CSME_PSF},
  {6, PID_PSF6},
  {7, PID_PSF7},
  {8, PID_PSF8}
};

/**
  Get list of supported PSF segments.

  @param[out] PsfTable        Array of supported PSF segments
  @param[out] PsfTableLength  Length of PsfTable
**/
VOID
PsfSegments (
  OUT PSF_SEGMENT  **PsfTable,
  OUT UINT32       *PsfTableLength
  )
{
  if (IsPchLp ()) {
    *PsfTable = mPchLpPsfTable;
    *PsfTableLength = ARRAY_SIZE(mPchLpPsfTable);
  } else {
    *PsfTable = mPchHPsfTable;
    *PsfTableLength = ARRAY_SIZE(mPchHPsfTable);
  }
}

/**
  Disable D3:F0 device at PSF level for PCH-LP
**/
VOID
PsfDisableD3F0 (
  VOID
  )
{
  PSF_PORT  PsfPort;

  PsfPort.PsfPid = PID_PSF1;
  PsfPort.RegBase = R_CNL_PCH_LP_PSF1_PCR_T0_SHDW_NVM_D3_F0_BASE;

  PsfDisableDevice (PsfPort);
}

/**
  Disable PSF upstream completion tracking for HDAudio on PCH-LP
**/
VOID
PsfDisableUpstreamCompletionTrackingForHda (
  VOID
  )
{
  PchPcrAndThenOr32 (PID_PSF1, R_CNL_PCH_LP_PSF1_PCR_PSF_PORT_CONFIG_PG1_PORT7, (UINT32)~B_PCH_PSFX_PCR_PORT_CONFIG_PGX_PORTX_ISM_CPL_TRACK, 0);
  PchPcrAndThenOr32 (PID_PSF2, R_CNL_PCH_LP_PSF2_PCR_PSF_PORT_CONFIG_PG1_PORT4, (UINT32)~B_PCH_PSFX_PCR_PORT_CONFIG_PGX_PORTX_ISM_CPL_TRACK, 0);
  PchPcrAndThenOr32 (PID_PSF3, R_CNL_PCH_LP_PSF3_PCR_PSF_PORT_CONFIG_PG1_PORT0, (UINT32)~B_PCH_PSFX_PCR_PORT_CONFIG_PGX_PORTX_ISM_CPL_TRACK, 0);
}

/**
  Check if MCTP is supported

  @retval TRUE              MCTP is supported
          FALSE             MCTP is not supported
**/
BOOLEAN
PsfIsMctpSupported (
  VOID
  )
{
  return TRUE;
}

