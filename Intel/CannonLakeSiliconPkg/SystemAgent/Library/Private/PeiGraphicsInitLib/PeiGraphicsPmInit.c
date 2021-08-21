/** @file
  PEIM to initialize IGFX PM

@copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2018 Intel Corporation.

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
#include <Library/PeiServicesLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciSegmentLib.h>
#include <Library/PciExpressLib.h>
#include <IndustryStandard/Pci.h>
#include <Private/Library/GraphicsInitLib.h>
#include <Private/Library/SaInitLib.h>
#include <Library/TimerLib.h>
#include <CpuRegs.h>
#include <Library/CpuPlatformLib.h>
#include <SaAccess.h>
#include <PchAccess.h>

///
/// Driver Consumed PPI Prototypes
///
#include <ConfigBlock/GraphicsPeiPreMemConfig.h>
#include <SaPolicyCommon.h>

GLOBAL_REMOVE_IF_UNREFERENCED BOOT_SCRIPT_REGISTER_SETTING  gSaGtRC6Registers[] = {
  {0x0,  0xA090,  0x7FFFFFFF,  0x0},
  //
  // RC1e - RC6/6p - RC6pp Wake Rate Limits
  //
  {0x0,  0xA098,  0x0,  0x03E80000},
  {0x0,  0xA0A8,  0x0,  0x0001E848},
  {0x0,  0xA0AC,  0x0,  0x00000019},
  //
  // Render/Video/Blitter Idle Max Count
  //
  {0x0,  0x2054,  0x0,  0xA},
  {0x0,  0x12054, 0x0,  0xA},
  {0x0,  0x22054, 0x0,  0xA},
  {0x0,  0x1A054, 0x0,  0xA},
  {0x0,  0x1C054, 0x0,  0xA},
  //
  // Enable Idle Messages
  //
  {0x0,  0x2050,  0x0,  0x00010000},
  {0x0,  0x12050, 0x0,  0x00010000},
  {0x0,  0x22050, 0x0,  0x00010000},
  {0x0,  0x1a050, 0x0,  0x00010000},
  {0x0,  0x1c050, 0x0,  0x00010000},
  {0x0,  0xC3E4,  0x0,  0xA},
  //
  // Unblock Ack to Busy
  //
  {0x0,  0xA0B0,  0x0,  0x0},
  //
  // RC Sleep / RCx Thresholds
  //
  {0x0,  0xA0B8,  0x0,  0x0000C350},
  //
  // RP Settings
  //
  {0x0,  0xA010,  0x0,  0x000F4240},
  {0x0,  0xA014,  0x0,  0x12060000},
  {0x0,  0xA02C,  0x0,  0x0000E808},
  {0x0,  0xA030,  0x0,  0x0003BD08},
  {0x0,  0xA068,  0x0,  0x000101D0},
  {0x0,  0xA06C,  0x0,  0x00055730},
  {0x0,  0xA070,  0x0,  0x0000000A},
  {0x0,  0xA168,  0x0,  0x00000006},
  {0x0,  0xA024,  0x0,  0x00000592}
};

GLOBAL_REMOVE_IF_UNREFERENCED BOOT_SCRIPT_REGISTER_SETTING  gSaGtSecurityRegisters[] = {
  {0x0,  0x4150,  0x0,         0x80040003},
  {0x0,  0x4154,  0x0,         0x800507FC},
  {0x0,  0x4158,  0x0,         0x800508D3},
  {0x0,  0x415C,  0x0,         0x800BFFFC},
  {0x0,  0x4160,  0x0,         0x80114001},
  {0x0,  0x4164,  0x0,         0x80117FFC},
  {0x0,  0x4168,  0x0,         0x80138001},
  {0x0,  0x416C,  0x0,         0x80147FFC},
  {0x0,  0x4170,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x4174,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x4178,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x417C,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x4180,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x4184,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x4188,  0x7FFFFFFF,  0x80000000},
  {0x0,  0x418C,  0x7FFFFFFF,  0x80000000}
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT32 gSpcLock[] = {
#ifdef CPU_CFL
  0x24608,
  0x2460C,
  0x24610,
#endif
  0x24688,
  0x2468C,
  0x24690,
  0x24708,
  0x2470C,
  0x24710,
  0x24788,
  0x2478C,
  0x24790,
  0x24008,
#ifdef CPU_CFL
  0x24088,
  0x2408C,
  0x24090,
  0x24108,
  0x2410C,
  0x24110,
#endif
  0x24188,
  0x2418C,
  0x24190,
  0x24408,
  0x2440C,
  0x24410,
  0x24488,
  0x2448C,
  0x24490,
  0x24508,
  0x2450C,
  0x24510
#ifdef CPU_CFL
  ,
  0x24208,
  0x2420C,
  0x24210,
  0x24288,
  0x2428C,
  0x24290
#endif
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT32 gSpcLockCnlLp[] = {
  0x24088,
  0x2408C,
  0x24090
};

GLOBAL_REMOVE_IF_UNREFERENCED UINT32 gSpcLockCnlH[] = {
  0x25208,
  0x2520C,
  0x25210,
  0x25288,
  0x2528C,
  0x25290,
  0x24208,
  0x2420C,
  0x24210,
  0x24288,
  0x2428C,
  0x24290
};

/**
  Initialize GT PowerManagement of SystemAgent.

  @param[in] GtConfig         - Instance of GRAPHICS_PEI_CONFIG
  @param[in] GttMmAdr            - Base Address of IGFX MMIO BAR
  @param[in] MchBarBase          - Base Address of MCH_BAR

  @retval EFI_SUCCESS           - GT Power Management initialization complete
  @retval EFI_INVALID_PARAMETER - The input parameter is invalid
**/
EFI_STATUS
PmInit (
  IN       GRAPHICS_PEI_CONFIG          *GtConfig,
  IN       UINT32                       GttMmAdr,
  IN       UINT32                       MchBarBase
  )
{
  UINT32            RegOffset;
  UINT32            Data32;
  UINT32            Data32Mask;
  UINT32            Result;
  UINT8             i;
  UINT32            Data32And;
  UINT32            Data32Or;
  CPU_STEPPING      CpuSteppingId;
  CPU_FAMILY        CpuFamilyId;
  CPU_GENERATION    CpuGeneration;
#ifndef CPU_CFL
  UINT32            GMSSizeSelector;
  UINT32            GMSSize;
  UINT32            GMSBase;
  UINT32            DoorbellCtxBase;
  UINT32            RC6CXTBASE;
  UINT64            McD0BaseAddress;

  McD0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
#endif
  CpuSteppingId   = GetCpuStepping ();
  CpuFamilyId     = GetCpuFamily();
  CpuGeneration   = GetCpuGeneration();

  if ((GttMmAdr == 0) || (MchBarBase == 0) || (GtConfig == NULL)) {
    DEBUG ((DEBUG_WARN, "Invalid parameters for PmInit\n"));
    return EFI_INVALID_PARAMETER;
  }

  DEBUG ((DEBUG_INFO, "Initializing GT PM Enabling, register configuration starting here...... \n"));

#ifndef CPU_CFL
  ///
  /// 1aa. Enable all GTI-Uncore clock gating
  ///
  RegOffset                     = 0xD08;
  Data32                        = 0x0;
  MmioWrite32 (GttMmAdr + RegOffset, Data32);
#else
  ///
  /// 1a. Set RC6 Context Location
  ///
  RegOffset                     = 0xD40;
  Data32                        = 0x80000000;
  MmioWrite32 (GttMmAdr + RegOffset, Data32);
#endif

  ///
  /// 1b. Set Context Base
  ///
#ifdef CPU_CFL
  Data32                        = 0x00000001;
#else
  ///
  /// Must set to a physical address within GT stolen Memory WOPCM, at least 32KB from the top.
  /// This range must be coordinated with other uses (like GuC and PAVP);
  /// It is expected that the upper 24KB of stolen memory is the proper location
  /// Also set bit 2:0 to 111b
  ///
  GMSSizeSelector = PciSegmentRead32 (McD0BaseAddress + R_SA_GGC);
  GMSSizeSelector = (GMSSizeSelector & B_SA_GGC_GMS_MASK) >> N_SA_GGC_GMS_OFFSET;
  //
  // Graphics Stolen Size
  // Graphics Stolen size above 64MB has a granularity of 32MB increments
  // GMS values below 240 correspond to Sizes 32 * GSMValue
  // Graphics Stolen size below 64MB has a higher granularity and can be set in 4MB increments
  // GMS values ranging from 240-254 correspond to sizes 4MB to 60MB (excluding 32MB) which is 4*(GSMValue-239)
  //
  if (GMSSizeSelector < 240 ) {
    GMSSize = (UINT32) GMSSizeSelector * 32;
  } else {
    GMSSize = 4 * (GMSSizeSelector - 239);
  }
  DEBUG ((DEBUG_INFO, "GMSSize: %dMB\n",GMSSize));
  GMSBase = PciSegmentRead32 (McD0BaseAddress + R_SA_BDSM) & B_SA_BDSM_BDSM_MASK;
  DEBUG ((DEBUG_INFO, "GMSBase read from R_SA_BDSM: 0x%x\n",GMSBase));

  //
  //RC6CXTBASE: the algorithm is BDSM+BDSM_SIZE-RC6CTXBASE_SIZE if WOPCM is not enabled.  If WOPCM is enabled, it should be WOPCMBASE+WOPCM_SIZE-RC6CTXBASE_SIZE.
  //In current design, WOPCM region : WOPCMbase (PAVPC bits 31:20) to DSMtop (BDSM + GMS) with PCME (PAVPC bit 0) = 1
  //so we can use RC6CXTBASE = DSMtop (BDSM + GMS) -RC6CTXBASE_SIZE in either case
  //
  RC6CXTBASE      = GMSBase + GMSSize * 0x100000 - RC6CTXBASE_SIZE;
  DoorbellCtxBase = (UINT32) (RC6CXTBASE - 0x1000);

  ///
  /// Programming Doorbell Context base Low and High
  ///
  Data32          = DoorbellCtxBase | BIT0;
  MmioWrite32 (GttMmAdr + 0xDC8, Data32);
  MmioWrite32 (GttMmAdr + 0xDCC, 0);

  ///
  /// Programming RC6 Context Base
  ///
  Data32                        = RC6CXTBASE | BIT0;
  DEBUG ((DEBUG_INFO, "RC6 Context Base: 0x%x\n", RC6CXTBASE));
#endif
  RegOffset                     = 0xD48;
  MmioWrite32 (GttMmAdr + RegOffset, Data32);

  if (GtConfig->PmSupport) {

#ifndef CPU_CFL
    ///
    /// Programming Crystal Clock and lock register by setting Bit31.
    ///
    RegOffset = 0xD00;
    Data32    = MmioRead32 (GttMmAdr + RegOffset);
    ///
    /// Programm [2:1] = 11 if BIT3 is 0 (indicate ref clock is 19.2Mhz)
    /// Programm [2:1] = 10 if BIT3 is 1 (indicate ref clock is 24Mhz)
    ///
    Data32 &= (BIT3);
    if ((Data32 >>3) == 0) {
      Data32Or = 0x6;
    } else {
      Data32Or = 0x4;
    }

    Data32Or |= BIT31;
    Data32And = (UINT32) ~(BIT2 | BIT1);
    MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
#endif

    ///
    /// Enable Force Wake
    ///
    RegOffset                     = 0xA188;
    Data32                        = 0x00010001;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll to verify Force Wake Acknowledge Bit
    ///
    RegOffset                     = 0x130044;
    Data32Mask                    = BIT0;
    Result                        = 1;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);
    DEBUG ((DEBUG_INFO, "Poll to verify Force Wake Acknowledge Bit, Result = 1\n"));

    ///
    /// Workaround for A0: Disable Tesselation DOP gating by setting bit 19 of MMIO register 20A0h.
    ///
    if ((CpuFamilyId == EnumCpuCnlUltUlx) && (CpuSteppingId <= EnumCnlA0)) {
      MmioOr32 (GttMmAdr + 0x20A0, BIT19);
    }

    if ((CpuFamilyId == EnumCpuCnlUltUlx) || (CpuFamilyId == EnumCpuCnlDtHalo)) {
      ///
      /// W/a: Disable pinned GTT read protection mechanism by default in pinner defeatured system
      ///
      MmioOr32 (GttMmAdr + 0x182810, BIT0);
      ///
      /// W/a: Set LP display DDR read cycles as non-snooped by setting LPSNOOP bit.
      ///
      MmioOr32 (GttMmAdr + 0x10100C, BIT0);
      ///
      /// Enable Dynamic clock gating of PSF clock feature is disabled by default and requires BIOS programming to enable it.
      ///
      MmioOr32 (GttMmAdr + 0x101038, BIT1);
    }

    ///
    /// GT Security Resgister programming.
    ///
    if (CpuGeneration != EnumCnlCpu) {
      for (i = 0; i < sizeof (gSaGtSecurityRegisters) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
        RegOffset    = gSaGtSecurityRegisters[i].Offset;
        Data32And    = gSaGtSecurityRegisters[i].AndMask;
        Data32Or     = gSaGtSecurityRegisters[i].OrMask;

        MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      }
    }

    ///
    /// Enabling Push Bus Metric Counter
    ///
    RegOffset                     = 0xA250;
    Data32                        = 0x000001FF;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Configuring Push Bus Shift
    ///
    RegOffset                     = 0xA25C;
    Data32                        = 0x00000010;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Pushbus Metric Control
    ///
    RegOffset                     = 0xA248;
    Data32                        = 0x80000004;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Program GfxPause Register
    ///
    RegOffset                     = 0xA000;
    Data32                        = 0x00070020;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// GPM Control
    ///
    RegOffset                     = 0xA180;
#ifdef CPU_CFL
    Data32                        = 0xC5200000;
#else
    Data32                        = 0x81200000;
#endif
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// ECO_BUSRST
    ///
    RegOffset                     = 0xA194;
#ifdef CPU_CFL
    Data32Or                      = BIT5;
#else
    Data32Or                      = BIT5 | BIT7 | BIT31;
    if ((CpuGeneration == EnumCnlCpu) && (CpuSteppingId >= EnumCnlD0)) {
      Data32Or |= BIT8;
    }
#endif
    MmioOr32 (GttMmAdr + RegOffset, Data32Or);

    ///
    /// Enable DOP clock gating.
    ///
#ifdef CPU_CFL
    Data32 = 0x000007FD;
#else
    Data32 = 0x00000FFF;
#endif

    RegOffset                     = 0x9424;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);
    DEBUG ((DEBUG_INFO, "Enabled DOP clock gating \n"));

#ifndef CPU_CFL
    ///
    /// Enable Unit Level Clock Gating
    ///
    RegOffset                     = 0x9430;
    Data32                        = 0x00000000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    RegOffset                     = 0x9044;
    Data32                        = 0xC0000000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

#else
    ///
    /// Enable unit level clock gates
    ///
    RegOffset                     = 0x9400;
    Data32                        = 0x00000000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    RegOffset                     = 0x9404;
    Data32                        = 0x40401000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    RegOffset                     = 0x9408;
    Data32                        = 0x00000000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    RegOffset                     = 0x940C;
    Data32                        = 0x02000001;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);
#endif

    ///
    /// Program GT Normal Frequency Request
    ///
    Data32 = 0x03018000;

    MmioWrite32 (GttMmAdr + 0xA008, Data32);

    ///
    /// Program Video Frequency Request
    ///
    Data32 = 0x0C800000;


    MmioWrite32 (GttMmAdr + 0xA00C, Data32);

    ///
    /// Program Media Force Wake. Set this before enabling power gating.
    ///
    RegOffset                     = 0xA270;
    Data32                        = 0x00010001;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll for Media Force Wake acknowledge.
    ///
    RegOffset                     = 0x0D88;
    Data32Mask                    = BIT0;
    Result                        = 1;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);

    ///
    /// Render Force Wake. Set this before enabling power gating.
    ///
    RegOffset                     = 0xA278;
    Data32                        = 0x00010001;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll for Render Force Wake acknowledge.
    ///
    RegOffset                     = 0x0D84;
    Data32Mask                    = BIT0;
    Result                        = 1;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);


    ///
    /// CdynMax Clamping Feature for higher frequency
    ///
    if (GtConfig->CdynmaxClampEnable) {
      ///
      /// a. Program the event weights into GT MMIO registers 0x8C04, 0x8C08 and 0x8C0C (Iccp_CDYNMAX_EVTWTx)
      /// b. Program the EI window in GT MMIO register 0x8C00 (CDYNMAX_CFG0)
      /// c. Program the clamping thresholds and the associated delta values in MMIO registers 0x8C10, 0x8C14, 0x8C18 and 0x8C1C (Iccp_CDYNA_CLAMP_THRx)
      /// d. Program the Panic threshold values in MMIO register 0x8C00.
      /// e. Program the threshold Compare Shift Value in MMIO register 0x8C1C
#ifdef CPU_CFL
      /// f. Set the required clamping level (clamped/Unclamped) in register in GT MMIO register 0xA218 (Iccp_Request_level)
      /// g. Enable GT ICCP feature via GT MMIO register 0xA214 (Iccp_Feature_Enable). The below settings are of 86% Clamping Threshold
#else
      /// f. Set the Lock Bit for SPMunit Registers
      /// g. Set the required clamping level (clamped/Unclamped) in register in GT MMIO register 0xA218 (Iccp_Request_level)
      /// h. Enable GT ICCP feature via GT MMIO register 0xA214 (Iccp_Feature_Enable). The below settings are of 86% Clamping Threshold
#endif
      ///
      DEBUG ((DEBUG_INFO, "Cdynmax Clamp Feature Enabled\n"));

      RegOffset                     = 0x8C04;
#ifdef CPU_CFL
      Data32                        = 0x29609FFF;
#else
      Data32                        = 0x0C60A2FF;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C04: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      RegOffset                     = 0x8C08;
#ifdef CPU_CFL
      Data32                        = 0x0603090F;
#else
      Data32                        = 0x0703070D;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C08: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      RegOffset                     = 0x8C0C;
#ifdef CPU_CFL
      Data32And                     = 0xFFFFFFC0;
      Data32Or                      = 0x18;
#else
      Data32And                     = 0xFF000000;
      Data32Or                      = 0x5171C;
#endif
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0x8C0C: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
      /// b. EI - Evaluation Interval 8C00[15:0]]= 13 (256ns/20 = 12.8)
#ifdef CPU_CFL
      /// e.Panic Threshold value 8C00[31:24]= 160, Progam ClampDis Threshold 8C00[23:16]= 84
#else
      /// d.Panic Threshold value 8C00[31:24]= 146, Progam ClampDis Threshold 8C00[23:16]= 71
#endif
      ///
      RegOffset                     = 0x8C00;
#ifdef CPU_CFL
      Data32                        = 0xA054000D;
#else
      Data32                        = 0x9247000D;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C00: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
      /// c. Clamping Threshold and Deltas
      ///
      RegOffset                     = 0x8C10;
#ifdef CPU_CFL
      Data32                        = 0xF078E051;
#else
      Data32                        = 0xF06EE04A;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C10: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      RegOffset                     = 0x8C14;
#ifdef CPU_CFL
      Data32                        = 0xFC9AF88C;
#else
      Data32                        = 0xFC8CF880;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C14: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      RegOffset                     = 0x8C18;
#ifdef CPU_CFL
      Data32                        = 0x08A300A0;
#else
      Data32                        = 0x08950092;
#endif
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "0x8C18: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
      /// e.Threshold Compare Shift Value in MMIO register 0x8C1C
      ///
      RegOffset                     = 0x8C1C;
#ifdef CPU_CFL
      Data32And                     = 0xF8000000;
      Data32Or                      = 0x022010A9;
#else
      Data32And                     = 0xC0000000;
      Data32Or                      = 0x1220109B;
#endif
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0x8C1C: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
#ifdef CPU_CFL
      /// f. License Request Level - GT MMIO Register address: 0xA218 [0:0] 1 - Clamped Mode; 0 - Non-Clamped Mode
#else
      /// g. License Request Level - GT MMIO Register address: 0xA218 [0:0] 1 - Clamped Mode; 0 - Non-Clamped Mode
#endif
      ///
      RegOffset                     = 0xA218;
      Data32And                     = 0xFFFFFFFE;
      Data32Or                      = BIT0;
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0xA218: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
#ifdef CPU_CFL
      /// g. Enabling the feature GT MMIO Register address: 0xA214 [0:0] IccP Feature Enable 0b = Feature is disabled (default) 1b = Feature is enabled
      ///  [31:31]   IccP Lock Keep it locked for non-Halo for SPM and GPM unit Keep it unlocked for Halo
#else
      /// h. Enabling the feature GT MMIO Register address: 0xA214 [0:0] IccP Feature Enable 0b = Feature is disabled (default) 1b = Feature is enabled
      /// i. Iccp Feature Lock 0xA214 [31:31] =1 , 1b = Feature is locked for SPM and GPM unit  0b = Feature Unlocked.
#endif
      ///
      RegOffset                     = 0xA214;
      Data32And                     = 0x7FFFFFFE;
#ifdef CPU_CFL
      Data32Or                      = BIT0;
#else
      Data32Or                      = (BIT31 | BIT0);
#endif
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0xA214: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      ///
      /// Programming for 50% CheckPoint
      /// a. Enable 50% CheckPoint 0x8C20[1] = 0
      /// b. 0x8C20[3:2] = 50% checkpoint programmable bubble count. 0x8C20[3:2] = 11b for 86% CT
      /// c. Enable Max Ratio to be 11/16 0x8C20[4] = 0
      /// d. Progam ClampDis Threshold 8C00[23:16] (done above)
      ///
      RegOffset                     = 0x8C20;
      Data32And                     = 0xFFFFFFE1;
      Data32Or                      = 0x0C;
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
#ifndef CPU_CFL
      ///
      /// Lock Bit for SPMunit Registers 0x8C20 [9:9]
      ///
      MmioOr32 (GttMmAdr + RegOffset, BIT9);
#endif
      DEBUG ((DEBUG_INFO, "0x8C20: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

    } else {
      ///
#ifdef CPU_CFL
      /// For non-Halo 4+4e parts disable this feature and lock it
      /// Disable the feature 0xA214 [0:0]  0b = Feature is disabled 1b = Feature is enabled
      /// [31:31]   IccP Lock Keep it locked for non-Halo for SPM and GPM unit Keep it unlocked for Halo
#else
      /// Set the Lock Bit for SPMunit Registers
      /// Set the required clamping level as Unclamped in register 0xA218[0]
      /// Disable the feature 0xA214 [0:0]  0b = Feature is disabled 1b = Feature is enabled
      /// Keep it locked 0xA214 [31:31]  IccP Lock bit : 1= Keep it locked for all part.
#endif
      ///
      DEBUG ((DEBUG_INFO, "Cdynmax Clamp Feature Disabled\n"));

#ifndef CPU_CFL
      ///
      /// Lock Bit for SPMunit Registers
      ///
      RegOffset                     = 0x8C20;
      Data32And                     = (UINT32) ~(BIT9);
      Data32Or                      = BIT9;
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0x8C20: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));
#endif

      ///
      /// License Request Level - Non-Clamp Mode: 0xA218 [0:0] 1 - Clamped Mode; 0 - Non-Clamped Mode
      ///
      RegOffset                     = 0xA218;
      Data32And                     = (UINT32) ~(BIT0);
      Data32Or                      = 0;
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0xA218: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));

      RegOffset                     = 0xA214;
      Data32And                     = 0x7FFFFFFE;
      Data32Or                      = BIT31;
      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
      DEBUG ((DEBUG_INFO, "0xA214: 0x%x\n", MmioRead32 (GttMmAdr + RegOffset)));
    }

    ///
    /// RC6 Settings
    ///
    for (i = 0; i < sizeof (gSaGtRC6Registers) / sizeof (BOOT_SCRIPT_REGISTER_SETTING); ++i) {
      RegOffset                     = gSaGtRC6Registers[i].Offset;
      Data32And                     = gSaGtRC6Registers[i].AndMask;
      Data32Or                      = gSaGtRC6Registers[i].OrMask;

      MmioAndThenOr32 (GttMmAdr + RegOffset, Data32And, Data32Or);
    }

    if (((CpuFamilyId == EnumCpuCnlUltUlx) && (CpuSteppingId >= EnumCnlC0)) ||
        ((CpuFamilyId == EnumCpuCnlDtHalo) && (CpuSteppingId >= EnumCnlC0))) {
      MmioWrite32 (GttMmAdr + 0xA09C, 0x00280028);
      MmioWrite32 (GttMmAdr + 0xA0A0, 0x00000028);
    } else {
      MmioWrite32 (GttMmAdr + 0xA09C, 0x00280000);
    }

    ///
    /// HW RC6 Control Settings
    ///
    Data32 = 0;

    if (GtConfig->RenderStandby) {
      Data32 = 0x88040000;
    }

    MmioWrite32 (GttMmAdr + 0xA090, Data32);

    ///
    /// RC6 Settings
    ///
    ///
    /// Wait for Mailbox ready
    ///
    Data32Mask  = BIT31;
    Result      = 0;

    PollGtReady (GttMmAdr, 0x138124, Data32Mask, Result);

    ///
    /// Mailbox Data  - RC6 VIDS
    ///
    Data32 = 0x0;
    MmioWrite32 (GttMmAdr + 0x138128, Data32);

    ///
    /// Mailbox Command
    ///
    Data32                      = 0x80000004;
    MmioWrite32 (GttMmAdr + 0x138124, Data32);

    ///
    /// Wait for Mailbox ready
    ///
    Data32Mask  = BIT31;
    Result      = 0;

    PollGtReady (GttMmAdr, 0x138124, Data32Mask, Result);

    ///
    /// Enable PM Interrupts
    ///
    Data32                      = 0x03000076;
    MmioWrite32 (GttMmAdr + 0x4402C, Data32);

    ///
    /// SPC Register Lock.
    ///
      DEBUG ((DEBUG_INFO, "GT Slice/Subslice Fuse value: %x\n", MmioRead32 (GttMmAdr + 0x9120)));
      if ((MmioRead32 (GttMmAdr + 0x9120) & B_SA_GT_SLICE_0_ENABLE_BIT) == 0) {
        ///
        /// Slice 0 disabled, set 0xFDC to valid slice
        ///
#ifndef CPU_CFL
        if ((MmioRead32 (GttMmAdr + 0x9120) & B_SA_GT_SLICE_1_ENABLE_BIT) == 0) {
          Data32Or = BIT27;  //slice 2
        } else {
#endif
        Data32Or = BIT26;  //slice 1
#ifndef CPU_CFL
        }
#endif
        Data32And = (UINT32) ~(BIT27 | BIT26);
        MmioAndThenOr32 (GttMmAdr + 0x0FDC, Data32And, Data32Or);
      }
      if ((MmioRead32 (GttMmAdr + 0x9120) & B_SA_GT_SUB_SLICE_0_ENABLE_BIT) == B_SA_GT_SUB_SLICE_0_ENABLE_BIT) {
        ///
        /// SubSlice 0 disabled, set 0xFDC to SubSlice 1
        ///
        Data32And = (UINT32) ~(BIT25 | BIT24);
        Data32Or = BIT24;
        MmioAndThenOr32 (GttMmAdr + 0x0FDC, Data32And, Data32Or);
      }

      Data32Or = 0x80000000;
      for (i = 0; i < sizeof(gSpcLock) / sizeof(UINT32); ++i) {
        RegOffset = gSpcLock[i];
        MmioOr32(GttMmAdr + RegOffset, Data32Or);
      }
      ///
      /// CNL LP & H based SPC Register Lock.
      ///
      if ((CpuGeneration == EnumCnlCpu) && (CpuFamilyId == EnumCpuCnlUltUlx)){
        for (i = 0; i < sizeof(gSpcLockCnlLp) / sizeof(UINT32); ++i) {
          RegOffset = gSpcLockCnlLp[i];
          MmioOr32(GttMmAdr + RegOffset, Data32Or);
        }
      } else if ((CpuGeneration == EnumCnlCpu) && (CpuFamilyId == EnumCpuCnlDtHalo)) {
        for (i = 0; i < sizeof(gSpcLockCnlH) / sizeof(UINT32); ++i) {
          RegOffset = gSpcLockCnlH[i];
          MmioOr32(GttMmAdr + RegOffset, Data32Or);
        }
      }

    ///
    /// Enabling to enter RC6 state in idle mode.
    ///
    Data32 = 0;
    if (GtConfig->RenderStandby) {
      RegOffset                     = 0xA094;
      Data32                        = 0x00040000;
      MmioWrite32 (GttMmAdr + RegOffset, Data32);
      DEBUG ((DEBUG_INFO, "Entered RC6 state in idle mode\n"));
    }

    ///
    /// Media Force Wake
    ///
    RegOffset                     = 0xA270;
    Data32                        = 0x00010000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll for Media Force Wake acknowledge.
    ///
    RegOffset                     = 0x0D88;
    Data32Mask                    = BIT0;
    Result                        = 0;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);

    ///
    /// Render Force Wake
    ///
    RegOffset                     = 0xA278;
    Data32                        = 0x00010000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll for Render Force Wake acknowledge.
    ///
    RegOffset                     = 0x0D84;
    Data32Mask                    = BIT0;
    Result                        = 0;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);

    ///
    /// Clear offset 0xA188 [31:0] to clear the force wake enable
    ///
    RegOffset                     = 0xA188;
    Data32                        = 0x00010000;
    MmioWrite32 (GttMmAdr + RegOffset, Data32);

    ///
    /// Poll until clearing is cleared to verify the force wake acknowledge.
    ///
    RegOffset                     = 0x130044;
    Data32Mask                    = BIT0;
    Result                        = 0;
    PollGtReady (GttMmAdr, RegOffset, Data32Mask, Result);
  }

  DEBUG ((DEBUG_INFO, "All register programming done for GT PM Init. Exiting.\n"));
  return EFI_SUCCESS;
}

/**
  Program PSMI registers.

  @param[in] GRAPHICS_PEI_CONFIG             *GtConfig
  @param[in] GRAPHICS_PEI_PREMEM_CONFIG      *GtPreMemConfig

  @retval EFI_SUCCESS     - PSMI registers programmed.
**/
EFI_STATUS
ProgramPsmiRegs (
  IN       GRAPHICS_PEI_CONFIG             *GtConfig,
  IN       GRAPHICS_PEI_PREMEM_CONFIG      *GtPreMemConfig
  )
{
  UINT64       McD0BaseAddress;
  UINT64       McD2BaseAddress;
  UINT64       PsmiBase;
  UINT32       *PsmiBaseAddr;
  UINT32       *PsmiLimitAddr;
  UINT64       PsmiLimit;
  UINT32       PavpMemSizeInMeg;
  UINT32       PsmiRegionSize;
  UINT32       GraphicsStolenSize;
  UINT32       PavpMemSize;
  UINT32       GttMmAdr;
  UINT32       GSMBase;

  McD0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
  McD2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0);

  GttMmAdr = (PciSegmentRead32 (McD2BaseAddress + R_SA_IGD_GTTMMADR)) & 0xFFFFFFF0;

  if (GttMmAdr == 0) {
    GttMmAdr = GtPreMemConfig->GttMmAdr;
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, (UINT32) (GttMmAdr & 0xFFFFFFFF));
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR + 4, 0);
  }

  if (!IgfxCmdRegEnabled()) {
    ///
    /// Enable Bus Master and Memory access on 0:2:0
    ///
    PciSegmentOr16 (McD2BaseAddress + R_SA_IGD_CMD, (BIT2 | BIT1));
  }

  ///
  /// If device 0:2:0 (Internal Graphics Device, or GT) is not enabled, skip programming PSMI registers
  ///
  if ((PciSegmentRead16 (McD2BaseAddress + R_SA_IGD_VID) != 0xFFFF) && (GtPreMemConfig->GtPsmiSupport == 1)) {
    DEBUG ((DEBUG_INFO, "Programming PSMI Registers\n"));
    ///
    /// PsmiBase is GSM Base plus GSM Size.
    ///
    GSMBase = PciSegmentRead32 (McD0BaseAddress + R_SA_BDSM) & B_SA_BDSM_BDSM_MASK;

    if (GtPreMemConfig->IgdDvmt50PreAlloc < 240) {
      GraphicsStolenSize = (32 * GtPreMemConfig->IgdDvmt50PreAlloc) << 20;
    } else {
      GraphicsStolenSize = (4 * (GtPreMemConfig->IgdDvmt50PreAlloc - 239)) << 20;
    }
    PsmiBase = (UINT64) GSMBase + (UINT64) GraphicsStolenSize;
    PsmiBaseAddr = (UINT32 *) &PsmiBase;

    ///
    /// Psmi Limit is Psmibase plus Psmi size and subtract PAVP size.
    ///
    PavpMemSizeInMeg = 1 << ((PciSegmentRead32 (McD0BaseAddress + R_SA_PAVPC) & B_SA_PAVPC_SIZE_MASK) >> 7);
    PavpMemSize = PavpMemSizeInMeg << 20;
    PsmiRegionSize = (32 + 256 * GtPreMemConfig->PsmiRegionSize) << 20;
    PsmiLimit = (UINT64) PsmiBase + (UINT64) PsmiRegionSize - PavpMemSize;
    PsmiLimitAddr = (UINT32 *) &PsmiLimit;
    ///
    /// Program PSMI Base and PSMI Limit and Lock.
    ///
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMIBASE_OFFSET + 4, *(PsmiBaseAddr + 1));
    //MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMIBASE_OFFSET, (*PsmiBaseAddr & B_SA_PSMIBASE_LSB_MASK) | B_SA_PSMI_LOCK_MASK);

    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMILIMIT_OFFSET + 4, *(PsmiLimitAddr + 1));
    //MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMILIMIT_OFFSET, (*PsmiLimitAddr & B_SA_PSMILIMIT_LSB_MASK) | B_SA_PSMI_LOCK_MASK);
  } /*else if (GtPreMemConfig->GtPsmiSupport == 0) {
    ///
    /// Program Non-Existent memony location for security when PSMI not in use.
    ///
    PsmiBase = 0xFFFFFFFFFFFFFFFF;
    PsmiBaseAddr = (UINT32 *) &PsmiBase;
    PsmiLimit = 0;
    PsmiLimitAddr = (UINT32 *) &PsmiLimit;
    ///
    /// Program PSMI Base and PSMI Limit and Lock.
    ///
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMIBASE_OFFSET + 4, *(PsmiBaseAddr + 1));
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMIBASE_OFFSET, (*PsmiBaseAddr & B_SA_PSMIBASE_LSB_MASK) | B_SA_PSMI_LOCK_MASK);

    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMILIMIT_OFFSET + 4, *(PsmiLimitAddr + 1));
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_PSMILIMIT_OFFSET, (*PsmiLimitAddr & B_SA_PSMILIMIT_LSB_MASK) | B_SA_PSMI_LOCK_MASK);
  }*/
  return EFI_SUCCESS;
}

/**
  Initialize PAVP feature of SystemAgent.

  @param[in] GRAPHICS_PEI_CONFIG             *GtConfig
  @param[in] SA_MISC_PEI_CONFIG              *MiscPeiConfig

  @retval EFI_SUCCESS     - PAVP initialization complete
  @retval EFI_UNSUPPORTED - iGFX is not present so PAVP not supported
**/
EFI_STATUS
PavpInit (
  IN       GRAPHICS_PEI_CONFIG             *GtConfig,
  IN       SA_MISC_PEI_CONFIG              *MiscPeiConfig
  )
{

  UINT32       PcmBase;
  UINT64       McD0BaseAddress;
  UINT64       McD2BaseAddress;
  UINT32       Pavpc;
  PcmBase = 0;
  McD0BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, 0);
  McD2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0);
  Pavpc = PciSegmentRead32 (McD0BaseAddress + R_SA_PAVPC);


  ///
  /// If device 0:2:0 (Internal Graphics Device, or GT) is not enabled, skip PAVP
  ///
  if (PciSegmentRead16 (McD2BaseAddress + R_SA_IGD_VID) != 0xFFFF) {
    DEBUG ((DEBUG_INFO, "Initializing PAVP\n"));
    Pavpc &= (UINT32) ~(B_SA_PAVPC_HVYMODSEL_MASK | B_SA_PAVPC_PCMBASE_MASK | B_SA_PAVPC_PAVPE_MASK | B_SA_PAVPC_PCME_MASK);
    Pavpc &= (UINT32) ~(BIT8 | BIT7);
    PcmBase = ((UINT32) RShiftU64 ((PciSegmentRead32 (McD0BaseAddress +R_SA_TOLUD)), 20)) - PAVP_PCM_SIZE_1_MB;
    Pavpc |= (UINT32) LShiftU64 (PcmBase, 20);
    if (GtConfig->PavpEnable == 1)  {
      Pavpc |= B_SA_PAVPC_PAVPE_MASK;
    }
    Pavpc |= B_SA_PAVPC_PCME_MASK;

    Pavpc |= BIT6;
  }
  ///
  /// Lock PAVPC Register
  ///
  Pavpc |= B_SA_PAVPC_PAVPLCK_MASK;
  PciSegmentWrite32 (McD0BaseAddress + R_SA_PAVPC, Pavpc);

  return EFI_SUCCESS;
}

/**
  Poll Run busy clear

  @param[in] Base    - Base address of MMIO
  @param[in] Timeout - Timeout value in microsecond

  @retval TRUE       - Run Busy bit is clear
  @retval FALSE      - Run Busy bit is still set
**/
BOOLEAN
PollRunBusyClear (
  IN    UINT64           Base,
  IN    UINT32           Timeout
  )
{
  UINT32  Value;
  BOOLEAN Status = FALSE;

  //
  // Make timeout an exact multiple of 10 to avoid infinite loop
  //
  if ((Timeout) % 10 != 0) {
    Timeout = (Timeout) + 10 - ((Timeout) % 10);
  }

  while (Timeout != 0) {
    Value = MmioRead32 ((UINTN) Base + 0x138124);
    if (Value & BIT31) {
      //
      // Wait for 10us and try again.
      //
      DEBUG ((DEBUG_INFO, "Interface register run busy bit is still set. Trying again \n"));
      MicroSecondDelay (MAILBOX_WAITTIME);
      Timeout = Timeout - MAILBOX_WAITTIME;
    } else {
      Status = TRUE;
      break;
    }
  }
  ASSERT ((Timeout != 0));

  return Status;
}

/**
  Program the max Cd Clock supported by the platform

  @param[in] GtConfig            - Instance of GRAPHICS_PEI_CONFIG
  @param[in] GttMmAdr            - Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            - CD Clock value programmed.
  @retval EFI_INVALID_PARAMETER  - The input parameter is invalid

**/
EFI_STATUS
ProgramCdClkReg (
  IN       GRAPHICS_PEI_CONFIG          *GtConfig,
  IN       UINT32                       GttMmAdr
  )
{
  UINT32         Data32Or;

#ifdef CPU_CFL
  ///
  /// CDCLK_CTL - GttMmAdr + 0x46000
  /// CdClock 1: 450Mhz   - [10:0] = 0x382
  /// CdClock 2: 540Mhz   - [10:0] = 0x436
  /// CdClock 0: 337.5Mhz - [10:0] = 0x2A1
  /// CdClock 3: 675Mhz   - [10:0] = 0x544
  ///
  switch (GtConfig->CdClock) {
    case 0 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_337_5;
      break;
    case 1 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_450;
      break;
    case 2 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_540;
      break;
    case 3 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_675;
      break;
    default:
      return EFI_INVALID_PARAMETER;
  }
#else
  ///
  /// For CNL, CDCLK_CTL - GttMmAdr + 0x46000
  /// CdClock 0: 168Mhz - [10:0] = 0x14E
  /// CdClock 1: 336Mhz - [10:0] = 0x29E
  /// CdClock 2: 528Mhz - [10:0] = 0x41E
  ///
  switch (GtConfig->CdClock) {
    case 0 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_168;
      break;
    case 1 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_336;
      break;
    case 2 :
      Data32Or = V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_528;
      break;
    default:
      return EFI_INVALID_PARAMETER;
  }
#endif // CPU_CFL
  //
  // Program CDCLK register with user selected value so that GOP can read and initialize CD Clock.
  //
  MmioAndThenOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, 0xFFFFF800, Data32Or);

  return EFI_SUCCESS;
}


/**
  Initialize the full CD clock as per Bspec sequence.

  @param[in] GtConfig            - Instance of GRAPHICS_PEI_CONFIG
  @param[in] GtPreMemConfig      - Instance of GRAPHICS_PEI_PREMEM_CONFIG

  @retval EFI_SUCCESS            - CD Clock Init successful.
  @retval EFI_INVALID_PARAMETER  - The input parameter is invalid
  @retval EFI_UNSUPPORTED        - iGfx is not present.
**/
EFI_STATUS
CdClkInit (
  IN  GRAPHICS_PEI_CONFIG             *GtConfig,
  IN  GRAPHICS_PEI_PREMEM_CONFIG      *GtPreMemConfig
  )
{
  UINT32         Data32Or;
  UINT16         WaitTime;
  UINT64         McD2BaseAddress;
  UINT32         GttMmAdr;
  UINT32         VoltageLevel;
#ifdef CPU_CFL
  UINT8          DpModeLinkRate;
  UINT32         CdClkRegValue;
#endif //CPU_CFL

  WaitTime = DISPLAY_CDCLK_TIMEOUT;
  McD2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0);
  if (PciSegmentRead16 (McD2BaseAddress + R_SA_IGD_VID) == 0xFFFF) {
    DEBUG ((DEBUG_INFO, "iGFX not enabled - Exit!\n"));
    return EFI_UNSUPPORTED;
  }

  if (GtConfig->SkipS3CdClockInit) {
    return EFI_SUCCESS;
  }

  GttMmAdr = (PciSegmentRead32 (McD2BaseAddress + R_SA_IGD_GTTMMADR)) & 0xFFFFFFF0;
  if (GttMmAdr == 0) {
    GttMmAdr = GtPreMemConfig->GttMmAdr;
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, (UINT32) (GttMmAdr & 0xFFFFFFFF));
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR + 4, 0);
  }

  if (!IgfxCmdRegEnabled()) {
    ///
    /// Enable Bus Master and Memory access on 0:2:0
    ///
    PciSegmentOr16 (McD2BaseAddress + R_SA_IGD_CMD, (BIT2 | BIT1));
  }


  //
  // Initialize full CDCLK sequence if not initialzed by PEIM Gfx.
  //
  if (!(MmioRead32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET) & B_SA_CDCLK_PLL_ENABLE_BIT)) {
#ifdef CPU_CFL
    ///
    /// CDCLK_CTL - GttMmAdr + 0x46000
    /// CdClock 1: [27:26] = 00b; 450    Mhz - [10:0] = 0x382
    /// CdClock 2: [27:26] = 01b; 540    Mhz - [10:0] = 0x436
    /// CdClock 0: [27:26] = 10b; 337.5  Mhz - [10:0] = 0x2A1
    /// CdClock 3: [27:26] = 11b; 675    Mhz - [10:0] = 0x544
    ///
    switch (GtConfig->CdClock) {
      case 0 :
        Data32Or = V_SA_CDCLK_CTL_CD_FREQ_SELECT_2 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_337_5;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_0;
        break;
      case 1 :
        Data32Or = V_SA_CDCLK_CTL_CD_FREQ_SELECT_0 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_450;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_1;
        break;
      case 2 :
        Data32Or = V_SA_CDCLK_CTL_CD_FREQ_SELECT_1 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_540;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_2;
        break;
      case 3 :
        Data32Or = V_SA_CDCLK_CTL_CD_FREQ_SELECT_3 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_675;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_3;
        break;
      default:
        return EFI_INVALID_PARAMETER;
    }
#else
    ///
    /// For CNL, CDCLK_CTL - GttMmAdr + 0x46000
    /// CdClock 0: [23:22] = 10b; 168Mhz - [10:0] = 001 0100 1110 = 0x14E
    /// CdClock 1: [23:22] = 00b; 336Mhz - [10:0] = 010 1001 1110 = 0x29E
    /// CdClock 2: [23:22] = 00b; 528Mhz - [10:0] = 100 0001 1110 = 0x41E
    ///
    switch (GtConfig->CdClock) {
      case 0 :
        Data32Or = V_SA_CDCLK_CTL_CD2X_DIVIDE_BY_2 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_168;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_0;
        break;
      case 1 :
        Data32Or = V_SA_CDCLK_CTL_CD2X_DIVIDE_BY_1 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_336;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_1;
        break;
      case 2 :
        Data32Or = V_SA_CDCLK_CTL_CD2X_DIVIDE_BY_1 | V_SA_CDCLK_CTL_CD_FREQ_DECIMAL_528;
        VoltageLevel = V_SA_GTTMMADR_MAILBOX_DATA_LOW_VOLTAGE_LEVEL_2;
        break;
      default:
        return EFI_INVALID_PARAMETER;
    }
#endif // CPU_CFL
    //
    // Enable Display Power Well
    //
    EnablePowerWell (GttMmAdr);
    //
    // Inform Power control of upcoming frequency change
    //
    PollRunBusyClear (GttMmAdr, MAILBOX_TIMEOUT); // Poll run-busy before start

    while (WaitTime != 0) { //3ms loop
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_DATA_LOW_OFFSET, 0x00000003);  // mailbox_low       = 0x00000003
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_DATA_HIGH_OFFSET, 0x00000000); // mailbox_high      = 0x00000000
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_INTERFACE_OFFSET, 0x80000007); // mailbox Interface = 0x80000007
      PollRunBusyClear (GttMmAdr, MAILBOX_TIMEOUT);   // Poll Run Busy cleared
      //
      // Check for MailBox Data read status successful
      //
      if ((MmioRead32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_DATA_LOW_OFFSET) & BIT0) == 1) {
        DEBUG ((DEBUG_INFO, "Mailbox Data low read Successfull \n"));
        break;
      }
      MicroSecondDelay (MAILBOX_WAITTIME);
      WaitTime = WaitTime - MAILBOX_WAITTIME;
    }
    //
    // 3ms Timeout
    //
    if (WaitTime == 0) {
      DEBUG ((DEBUG_INFO, "CDCLK initialization failed , not changing CDCLK \n"));
    } else {
      DEBUG ((DEBUG_INFO, "Enabling CDCLK  \n"));
      //
      // Enable CDCLK PLL and change the CDCLK_CTL register
      //
#ifndef CPU_CFL
      if (((MmioRead32 (GttMmAdr + R_SA_GTTMMADR_DSSM_OFFSET)) & BIT31) == 1) {
        // Program CD Clock PLL Ratio with reference clock frequency = 24MHz
        MmioAndThenOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_GTTMMADR_CDCLK_PLL_RATIO_MASK, V_SA_CDCLK_PLL_RATIO_REF_CLK_24_MHZ);
      } else {
        // Program CD Clock PLL Ration with reference clock frequency = 19.2MHz
        MmioAndThenOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_GTTMMADR_CDCLK_PLL_RATIO_MASK, V_SA_CDCLK_PLL_RATIO_REF_CLK_19_2MHZ); 
      }
#endif

#ifdef CPU_CFL
      ///
      /// If DPLL_CTRL1 (0x6C058) BIT[3:1] == 100b or 101b (2.16 GHz or 4.32 GHz), toggle the frequency select to get the PLL to recover.
      ///
      CdClkRegValue = MmioRead32(GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET);
      DpModeLinkRate = (((MmioRead32(GttMmAdr + R_SA_GTTMMADR_DPLL_CTRL_OFFSET)) >> 1) & 0x07);
      if (DpModeLinkRate == 0x04 || DpModeLinkRate == 0x05) {
        MmioOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, BIT19);
        MmioOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_CDCLK_PLL_ENABLE_BIT);
        PollGtReady (GttMmAdr, R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_CDCLK_PLL_LOCK_BIT, B_SA_CDCLK_PLL_LOCK_BIT);

        MmioAnd32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, (UINT32)(~(BIT27 | BIT26)));
        MmioOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, (UINT32)((CdClkRegValue & (BIT27 | BIT26))));
        MmioAnd32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, (UINT32)(~BIT19));
      } else {
#endif //CPU_CFL
      MmioOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_CDCLK_PLL_ENABLE_BIT);
      PollGtReady (GttMmAdr, R_SA_GTTMMADR_CDCLK_PLL_ENABLE_OFFSET, B_SA_CDCLK_PLL_LOCK_BIT, B_SA_CDCLK_PLL_LOCK_BIT);
#ifdef CPU_CFL
      }
#endif //CPU_CFL
      MmioAndThenOr32 (GttMmAdr + R_SA_GTTMMADR_CDCLK_CTL_OFFSET, B_SA_GT_CD_CLK_FREQ_MASK, Data32Or);
      //
      //Inform Power controller of the selected freq
      //
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_DATA_LOW_OFFSET, VoltageLevel);
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_DATA_HIGH_OFFSET, 0x00000000);
      MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_MAILBOX_INTERFACE_OFFSET, 0x80000007);
    }
  }
  return EFI_SUCCESS;
}

/**
Initialize GT Power management

  @param[in] GRAPHICS_PEI_PREMEM_CONFIG      GtPreMemConfig
  @param[in] GRAPHICS_PEI_CONFIG             GtConfig

  @retval EFI_SUCCESS          - GT Power management initialization complete
**/
EFI_STATUS
GraphicsPmInit (
  IN       GRAPHICS_PEI_PREMEM_CONFIG      *GtPreMemConfig,
  IN       GRAPHICS_PEI_CONFIG             *GtConfig
  )
{
  UINT32                LoGTBaseAddress;
  UINT32                HiGTBaseAddress;
  UINT64                McD2BaseAddress;
  UINT32                GttMmAdr;
  UINT32                GmAdr;
  UINT32                MchBarBase;
  UINT8                 Msac;
#ifdef CPU_CFL
  UINT32                Data32;
  UINT32                Data32H;
#endif

  DEBUG ((DEBUG_INFO, " iGfx Power management start.\n"));

  GttMmAdr   = 0;
  MchBarBase = 0;
  MchBarBase = PciSegmentRead32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, R_SA_MCHBAR));
  MchBarBase = MchBarBase & ~BIT0;

  ///
  /// If device 0:2:0 (Internal Graphics Device, or GT) is enabled, then Program GTTMMADR,
  ///
  McD2BaseAddress = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IGD_BUS, SA_IGD_DEV, SA_IGD_FUN_0, 0);
  if (PciSegmentRead16 (McD2BaseAddress + R_SA_IGD_VID) != 0xFFFF) {
    ///
    /// Program GT PM Settings if GttMmAdr allocation is Successful
    ///
    GttMmAdr                          = GtPreMemConfig->GttMmAdr;
    LoGTBaseAddress                   = (UINT32) (GttMmAdr & 0xFFFFFFFF);
    HiGTBaseAddress                   = 0;
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR, LoGTBaseAddress);
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GTTMMADR + 4, HiGTBaseAddress);

    Msac = PciSegmentRead8 (McD2BaseAddress + R_SA_IGD_MSAC_OFFSET);
    PciSegmentAndThenOr8 (McD2BaseAddress + R_SA_IGD_MSAC_OFFSET, (UINT8) ~(BIT4 + BIT3 + BIT2 + BIT1 + BIT0), SA_GT_APERTURE_SIZE_256MB);

    GmAdr = GtPreMemConfig->GmAdr;
    PciSegmentWrite32 (McD2BaseAddress + R_SA_IGD_GMADR, GmAdr);

    if (!IgfxCmdRegEnabled()) {
      ///
      /// Enable Bus Master and Memory access on 0:2:0
      ///
      PciSegmentOr16 (McD2BaseAddress + R_SA_IGD_CMD, (BIT2 | BIT1));
    }

    ///
    /// Program GT frequency
    /// Note: User requested frequency takes precedence than DisableTurboGt
    ///
    if ((GtConfig->DisableTurboGt == 1) && (GtConfig->GtFreqMax == 0xFF)) {
      ///
      /// Read bits[15:8] and limit the GtFrequency to Rp1
      ///
      MmioWrite8 ((MchBarBase + 0x5994), (UINT8)(((MmioRead32 (MchBarBase + 0x5998)) >> 0x8) & 0xFF));
      DEBUG ((DEBUG_INFO, "Disabling Turbo Gt - Programmed to frequency (in units of 50MHz): 0x%x \n", MmioRead8 (MchBarBase + 0x5994)));
    } else if (GtConfig->GtFreqMax != 0xFF) {
      ///
      /// Program user requested GT frequency
      ///
      MmioWrite8 ((MchBarBase + 0x5994), (UINT8) GtConfig->GtFreqMax);
      DEBUG ((DEBUG_INFO, "Max frequency programmed by user in MchBar 0x5994 is (to be multiplied by 50 for MHz): 0x%x  \n", MmioRead8 (MchBarBase + 0x5994)));
    }

    ///
    /// PmInit Initialization
    ///
    DEBUG ((DEBUG_INFO, "Initializing GT PowerManagement\n"));
    PmInit (GtConfig, GttMmAdr, MchBarBase);

    ///
    /// Program CD clock value based on Policy
    ///
    ProgramCdClkReg (GtConfig, GttMmAdr);

#ifdef CPU_CFL
    DEBUG ((DEBUG_INFO, "Configuring iTouch Source Registers Doorbell and GSA_Touch \n"));
    ///
    /// Configure iTouch Doorbell Source Registers
    /// Configure Doorbell Register 0x124828 BDF bits[63:48] with Bus Device Function of DoorBell Source HECI Device 0/22/4 and lock BDF bit[40]
    /// Configure GSA_Touch Register 0x1300B4 BDF bits[31:16] with Bus Device Function of DoorBell Source HECI Device 0/22/4 and lock BDF bit[0]
    ///
    Data32H = MmioRead32 (GttMmAdr + R_SA_GTTMMADR_GTDOORBELL_OFFSET + 4); // Higher 32 bits of 0x124828
    Data32H = ((Data32H & 0x0000FFFF) | (PCI_EXPRESS_LIB_ADDRESS (GSA_TOUCH_BUS, GSA_TOUCH_DEV, GSA_TOUCH_FUN, 0) << 4) | BIT8); // Bus 0, Device 22, Func 4 and lock bit

    Data32 = MmioRead32 (GttMmAdr + R_SA_GTTMMADR_GSA_TOUCH_OFFSET);
    Data32 = ((Data32 & 0x0000FFFF) | (PCI_EXPRESS_LIB_ADDRESS (GSA_TOUCH_BUS, GSA_TOUCH_DEV, GSA_TOUCH_FUN, 0) << 4) | B_SA_GTTMMADR_GSA_TOUCH_LOCK); // Bus 0, Device 22, Func 4 and lock bit
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_GTDOORBELL_OFFSET + 4, Data32H);
    MmioWrite32 (GttMmAdr + R_SA_GTTMMADR_GSA_TOUCH_OFFSET, Data32);
#endif

    ///
    /// Clear GttMmAdr, GmAdr
    ///
    PciSegmentWrite8 (McD2BaseAddress + R_SA_IGD_MSAC_OFFSET, Msac);
  }
  DEBUG ((DEBUG_INFO, "iGfx Power management end.\n"));
  return EFI_SUCCESS;
}


/**
  Enables Power Well 1 for platform

  @param[in] GttMmAdr            - Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            - Power well 1 Enabled
  @retval EFI_UNSUPPORTED        - Power well 1 programming Failed
  @retval EFI_TIMEOUT            - Timed out
**/
EFI_STATUS
EnablePowerWell1 (
  IN  UINT32     GttMmAdr
  )
{
  EFI_STATUS  Status;
  //
  // Poll for PG0 Fuse distribution status
  //
  Status = PollGtReady (GttMmAdr, R_SA_GTTMMADR_FUSE_STATUS_OFFSET, B_SA_GTTMMADR_FUSE_STATUS_PG0_DIST_STATUS, B_SA_GTTMMADR_FUSE_STATUS_PG0_DIST_STATUS);
  if (Status != EFI_SUCCESS) {
    return EFI_UNSUPPORTED;
  }
  //
  // Enable PG1
  //
  MmioOr32 (GttMmAdr + R_SA_GTTMMADR_PWR_WELL_CTL_OFFSET, R_SA_GTTMMADR_PWR_WELL_CTL_PG_1_ENABLE);
  //
  // Poll for PG1 state
  //
  Status = PollGtReady (GttMmAdr, R_SA_GTTMMADR_PWR_WELL_CTL_OFFSET, R_SA_GTTMMADR_PWR_WELL_CTL_PG_1_STATE, R_SA_GTTMMADR_PWR_WELL_CTL_PG_1_STATE);
  return Status;
}

/**
  Enables Power Well 2 for platform

  @param[in] GttMmAdr            - Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            - Power well 2 Enabled
  @retval EFI_UNSUPPORTED        - Power well 2 programming Failed
  @retval EFI_TIMEOUT            - Timed out
**/
EFI_STATUS
EnablePowerWell2 (
  IN  UINT32     GttMmAdr
  )
{
  EFI_STATUS  Status;
  //
  // Poll Fuse PG1 distribution status
  //
  Status = PollGtReady (GttMmAdr, R_SA_GTTMMADR_FUSE_STATUS_OFFSET, B_SA_GTTMMADR_FUSE_STATUS_PG1_DIST_STATUS, B_SA_GTTMMADR_FUSE_STATUS_PG1_DIST_STATUS);
  if (Status != EFI_SUCCESS) {
    return EFI_UNSUPPORTED;
  }
  //
  // Enable PG2
  //
  MmioOr32 (GttMmAdr + R_SA_GTTMMADR_PWR_WELL_CTL_OFFSET, R_SA_GTTMMADR_PWR_WELL_CTL_PG_2_ENABLE);
  //
  // Poll for PG2 state
  //
  Status = PollGtReady (GttMmAdr, R_SA_GTTMMADR_PWR_WELL_CTL_OFFSET, R_SA_GTTMMADR_PWR_WELL_CTL_PG_2_STATE, R_SA_GTTMMADR_PWR_WELL_CTL_PG_2_STATE);
  return Status;
}


/**
  Program the Display Power Wells supported by platform

  @param[in] GttMmAdr            - Base Address of IGFX MMIO BAR

  @retval EFI_SUCCESS            - Power well programming finished successfully
  @retval EFI_UNSUPPORTED        - Power well programming failed
  @retval EFI_TIMEOUT            - Timed out
**/
EFI_STATUS
EnablePowerWell (
  IN  UINT32     GttMmAdr
)
{
  EFI_STATUS        Status;

  DEBUG ((DEBUG_INFO, "EnablePowerWell Started !\n"));
  //
  // Enable the power well 1
  //
  Status = EnablePowerWell1 (GttMmAdr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "EnablePowerWell1 () has failed!\n"));
    return Status;
  }
  //
  // Enable power well 2
  //
  Status = EnablePowerWell2 (GttMmAdr);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "EnablePowerWell2 () has failed!\n"));
    return Status;
  }

  DEBUG ((DEBUG_INFO, "EnablePowerWell Successfull \n"));
  return EFI_SUCCESS;
}
