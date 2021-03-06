/** @file
  PCH PMC Private Library implementation for CannonLake PCH.
  All function in this library is available for PEI, DXE, and SMM,
  But do not support UEFI RUNTIME environment call.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2018 Intel Corporation.

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
#include <Uefi/UefiBaseType.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Private/Library/PmcPrivateLib.h>
#include <Library/SataLib.h>
#include <Library/PchInfoLib.h>
#include <Register/PchRegsPmc.h>

#include "PmcPrivateLibInternal.h"

/**
  This function returns PMC, PUNIT and CPU Crashlog Size allocated on PMC SSRAM
  specific for a given generation

  @param[out] PmcLogSize        Pointer to PMC CrashLog Size
  @param[out] PunitLogSize      Pointer to PUNIT CrashLog Size
  @param[out] CpuLogSize        Pointer to CPU CrashLog Size

**/
VOID
PmcSsramSocCrashLogSize (
  OUT UINT32     *PmcLogSize,
  OUT UINT32     *PunitLogSize,
  OUT UINT32     *CpuLogSize
  )
{
  *PmcLogSize   = 0xC00;  // 3KB
  *PunitLogSize = 0;  // No PUNIT Crashlog Support in CNL
  *CpuLogSize   = 0;  // No CPU Crashlog Support in CNL
}

/**
  This function disables Trace Hub by enabling power gating
**/
VOID
PmcDisableTraceHub (
  VOID
  )
{
  EFI_STATUS                Status;
  PMC_IPC_COMMAND_BUFFER    Wbuf;

  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));

  Wbuf.Buf0 = BIT0;
  Status = PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_NPK_STATE, 0, 4, &Wbuf, NULL);
  ASSERT_EFI_ERROR (Status);
}

/**
  This function enables Trace Hub by disabling power gating
**/
VOID
PmcEnableTraceHub (
  VOID
  )
{
  EFI_STATUS                Status;
  PMC_IPC_COMMAND_BUFFER    Wbuf;

  ZeroMem (&Wbuf, sizeof (PMC_IPC_COMMAND_BUFFER));

  Wbuf.Buf0 = BIT1;
  Status = PmcSendCommand (V_PMC_PWRM_IPC_CMD_COMMAND_NPK_STATE, 0, 4, &Wbuf, NULL);
  ASSERT_EFI_ERROR (Status);
}

/**
  This function is part of PMC init and configures which clock wake signals should
  set the SLOW_RING, SA, FAST_RING_CF and SLOW_RING_CF indication sent up to the CPU/PCH
**/
VOID
PmcInitClockWakeEnable (
  VOID
  )
{
  UINT32                    PchPwrmBase;

  PchPwrmBase = PmcGetPwrmBase ();
  if (IsPchLp () && (PchStepping () < PCH_B0)) {
    ///
    /// PWRMBASE + 0x1880 = 0x0
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING, 0x0);
  } else {
    ///
    /// PWRMBASE + 0x1880 = 0x2F8FBB01
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING, 0x2F8FBB01);
  }

  if (IsPchLp ()) {
    if (PchStepping () < PCH_B0) {
      ///
      /// PWRMBASE + 0x1884 = 0x0
      ///
      MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING2, 0x0);
    } else {
      ///
      /// PWRMBASE + 0x1884
      ///  PCH-LP: 0x0280C7E1
      ///
      MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING2, 0x0280C7E1);
    }
  } else {
    ///
    /// PWRMBASE + 0x1884
    ///  PCH-H:  0x0280D7E1
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING2, 0x0280D7E1);
  }

  if (IsPchLp () && (PchStepping () < PCH_B0)) {
    ///
    /// PWRMBASE + 0x1888 = 0x0
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SA, 0x0);
    ///
    /// PWRMBASE + 0x188C = 0x0
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SA2, 0x0);
  } else {
    ///
    /// PWRMBASE + 0x1888 = 0x2F8FAB01
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SA, 0x2F8FAB01);

    ///
    /// PWRMBASE + 0x188C
    ///  PCH-LP: 0x0280C7E1
    ///  PCH-H:  0x0280D7E1
    ///
    if (IsPchLp ()) {
      MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SA2, 0x0280C7E1);
    } else {
      MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SA2, 0x0280D7E1);
    }
  }

  if (IsPchLp () && (PchStepping () < PCH_B0)) {
    ///
    /// PWRMBASE + 0x1898 = 0x0
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING_CF, 0x0);
  } else {
    ///
    /// PWRMBASE + 0x1898 = 0x00018000
    ///
    MmioWrite32 (PchPwrmBase + R_PMC_PWRM_EN_CW_SLOW_RING_CF, 0x00018000);
  }
}

/**
  This function configures PWRMBASE + 0x1E00 register
**/
VOID
PmcConfigureRegPwrm1E00 (
  VOID
  )
{
  ///
  /// PWRMBASE + 0x1E00[31,30] = 1,1
  /// PWRMBASE + 0x1E00[29] = 0
  /// PWRMBASE + 0x1E00[10:6] = 0
  /// PWRMBASE + 0x1E00[3:0] = 2
  ///
  MmioAndThenOr32 (
    PmcGetPwrmBase () + R_PMC_PWRM_1E00,
    (UINT32) ~(BIT29 | (0x1F << 6) | 0xF),
    BIT31 | BIT30 | 2
    );
}

/**
  This function configures Misc PM_SYNC events settings
**/
VOID
PmcConfigurePmSyncEventsSettings (
  VOID
  )
{
  ///
  /// PWRMBASE + 0x18C0 = 0x00000A20
  ///
  MmioWrite32 (PmcGetPwrmBase () + R_PMC_PWRM_EN_MISC_EVENT, 0x00000A20);
}

/**
  This function enables all SerailIo devices.
  Static power gating disabling takes place after Global Reset, G3 or DeepSx transition.
**/
VOID
PmcEnableSerialIo (
  VOID
  )
{
  //
  // Set PWRMBASE + ST_PG_FDIS_PMC_2
  //
  MmioAnd32 (PmcGetPwrmBase () + R_PMC_PWRM_ST_PG_FDIS_PMC_2, (UINT32)~B_PMC_PWRM_ST_PG_FDIS_PMC_2_SERIALIO);
}

/**
  This function disables (static power gating) all SerailIo devices.
  For SerialIo controllers they can be power gated only if all of them are to be disabled.
  They cannot be statically power gated separately.
  For static power gating to take place Global Reset, G3 or DeepSx transition must happen.
**/
VOID
PmcStaticDisableSerialIo (
  VOID
  )
{
  //
  // Set PWRMBASE + ST_PG_FDIS_PMC_2
  //
  MmioOr32 (PmcGetPwrmBase () + R_PMC_PWRM_ST_PG_FDIS_PMC_2, B_PMC_PWRM_ST_PG_FDIS_PMC_2_SERIALIO);
}

/**
  This function checks if all SerialIo devices are statically disabled (static power gating)

  @retval SerialIo disable state
**/
BOOLEAN
PmcIsSerialIoStaticallyDisabled (
  VOID
  )
{
  //
  // Check if all SerialIo controllers are statically disabled in PMC
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_ST_PG_FDIS_PMC_2) & B_PMC_PWRM_ST_PG_FDIS_PMC_2_SERIALIO) == B_PMC_PWRM_ST_PG_FDIS_PMC_2_SERIALIO);
}

/**
  This function checks if SerialIo device is supported (not disabled by fuse)

  @retval SerialIo support state
**/
BOOLEAN
PmcIsSerialIoSupported (
  VOID
  )
{
  //
  // Get fuse info from PWRMBASE + FUSE_SS_DIS_RD_2
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_FUSE_DIS_RD_2) & B_PMC_PWRM_FUSE_DIS_RD_2_SERIALIO_FUSE_SS_DIS) == 0);
}

/**
  This function disables (non-static power gating) SCS eMMC controller and enables ModPHY SPD gating (PCH-LP only).
**/
VOID
PmcDisableScsEmmc (
  VOID
  )
{
  ASSERT (IsPchLp ());

  //
  // Set PWRMBASE + NST_PG_FDIS_1 to disable SCS Controller in PMC
  //
  MmioOr32 (PmcGetPwrmBase () + R_PMC_PWRM_NST_PG_FDIS_1, B_PCH_LP_PMC_PWRM_NST_PG_FDIS_1_EMMC_FDIS_PMC);
}

/**
  This function disables (non-static power gating) SCS SD Card controller and enables ModPHY SPD gating (PCH-LP only).
**/
VOID
PmcDisableScsSdCard (
  VOID
  )
{
  UINT32        ScsDevicePgMask;

  if (IsPchLp ()) {
    ScsDevicePgMask = B_PCH_LP_PMC_PWRM_NST_PG_FDIS_1_SDCARD_FDIS_PMC;
  } else {
    ScsDevicePgMask = B_PCH_H_PMC_PWRM_NST_PG_FDIS_1_SDCARD_FDIS_PMC;
  }

  //
  // Set PWRMBASE + NST_PG_FDIS_1 to disable SCS Controller in PMC
  //
  MmioOr32 (PmcGetPwrmBase () + R_PMC_PWRM_NST_PG_FDIS_1, ScsDevicePgMask);
}

/**
  This function disables (non-static power gating) SCS UFS controller and enables ModPHY SPD gating (PCH-LP only).

  @param[in] UfsNum     SCS UFS Device
**/
VOID
PmcDisableScsUfs (
  IN UINT32   UfsNum
  )
{
  UINT32        PchPwrmBase;

  ASSERT ((UfsNum == 0) && IsPchLp ());

  PchPwrmBase = PmcGetPwrmBase ();

  //
  // Set PWRMBASE + NST_PG_FDIS_1 to disable SCS Controller in PMC
  //
  MmioOr32 (PchPwrmBase + R_PMC_PWRM_NST_PG_FDIS_1, B_PCH_LP_PMC_PWRM_NST_PG_FDIS_1_UFS_FDIS_PMC);

  if (PmcIsModPhySusPgSupported ()) {
    //
    // Set MSPDRTREQ:
    // PchPwrmBase + R_PWRM_MODPHY_PM_CFG5[16] = 1 to enable ASL code trigger request for ModPHY SPD gating.
    //
    MmioOr32 (PchPwrmBase + R_PMC_PWRM_MODPHY_PM_CFG5, B_PMC_PWRM_MODPHY_PM_CFG5_MSPDRTREQ_UFS2);
  }
}

/**
  This function checks if SCS eMMC device is supported (not disabled by fuse)

  @retval SCS device support state
**/
BOOLEAN
PmcIsScsEmmcSupported (
  VOID
  )
{
  //
  // Get fuse info from PWRMBASE + FUSE_SS_DIS_RD_2
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_FUSE_DIS_RD_2) & B_PMC_PWRM_FUSE_DIS_RD_2_EMMC_FUSE_SS_DIS) == 0);
}

/**
  This function checks if SCS SD Card device is supported (not disabled by fuse)

  @retval SCS device support state
**/
BOOLEAN
PmcIsScsSdCardSupported (
  VOID
  )
{
  //
  // Get fuse info from PWRMBASE + FUSE_SS_DIS_RD_2
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_FUSE_DIS_RD_2) & B_PMC_PWRM_FUSE_DIS_RD_2_SDX_FUSE_SS_DIS) == 0);
}

/**
  This function checks if SCS UFS device is supported (not disabled by fuse)

  @param[in] UfsNum     SCS UFS Device

  @retval SCS device support state
**/
BOOLEAN
PmcIsScsUfsSupported (
  IN UINT32   UfsNum
  )
{
  //
  // Get fuse info from PWRMBASE + FUSE_SS_DIS_RD_2
  //
  return ((MmioRead32 (PmcGetPwrmBase () + R_PMC_PWRM_FUSE_DIS_RD_2) & B_PMC_PWRM_FUSE_DIS_RD_2_UFSX2_FUSE_SS_DIS) == 0);
}

/**
  Check if PMC supports IPC CMD 0xA9.

  @retval TRUE  PMC supports CMD A9
  @retval FALSE PMC doesn't support CMD A9
**/
BOOLEAN
PmcIsIpcCmdA9Supported (
  VOID
  )
{
  UINT32  PatchVersion;
  PCH_STEPPING  PchStep;

  PatchVersion = PmcGetFirmwarePatchVersion ();
  PchStep = PchStepping ();

  if (IsPchLp ()) {
    if (PchStep < PCH_D1 && PatchVersion >= 15) {
      return TRUE;
    } else if (PchStep >= PCH_D1 && PatchVersion >= 8) {
      return TRUE;
    }
  } else {
    if (PchStep < PCH_B0 && PatchVersion >= 13) {
      return TRUE;
    } else if (PchStep >= PCH_B0 && PatchVersion >= 12) {
      return TRUE;
    }
  }
  return FALSE;
}

