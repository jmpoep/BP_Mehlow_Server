/** @file
  CPU Platform Lib implementation.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2018 Intel Corporation.

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
#include "CpuPlatformLibrary.h"
#include <SaAccess.h>
#include <MeChipset.h>

#define SKIP_MICROCODE_CHECKSUM_CHECK 1
#define C6DRAM_ENABLE 1
#define C6DRAM_DISABLE 0

/**
  Return CPU Family ID

  @retval CPU_FAMILY              CPU Family ID
**/
CPU_FAMILY
EFIAPI
GetCpuFamily (
  VOID
  )
{
  EFI_CPUID_REGISTER Cpuid;
  ///
  /// Read the CPUID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  return ((CPU_FAMILY) (Cpuid.RegEax & CPUID_FULL_FAMILY_MODEL));
}

/**
  Return Cpu stepping type

  @retval UINT8                   Cpu stepping type
**/
CPU_STEPPING
EFIAPI
GetCpuStepping (
  VOID
  )
{
  EFI_CPUID_REGISTER Cpuid;
  ///
  /// Read the CPUID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  return ((CPU_STEPPING) (Cpuid.RegEax & CPUID_FULL_STEPPING));
}

/**
  Return CPU Sku

  @retval UINT8              CPU Sku
**/
UINT8
EFIAPI
GetCpuSku (
  VOID
  )
{
  UINT8              CpuType;
  UINT16             CpuDid;
  UINT32             CpuFamilyModel;
  EFI_CPUID_REGISTER Cpuid;
  BOOLEAN            SkuFound;

  SkuFound  = TRUE;
  CpuType   = EnumCpuUnknown;

  ///
  /// Read the CPUID & DID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  CpuFamilyModel = Cpuid.RegEax & CPUID_FULL_FAMILY_MODEL;
  CpuDid = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));

  switch (CpuFamilyModel) {
#ifdef CPU_CFL
    case CPUID_FULL_FAMILY_MODEL_COFFEELAKE_ULT_ULX:
      switch (CpuDid) {
        case V_SA_DEVICE_ID_KBL_MB_ULT_1:    // KBL ULT OPI
        case V_SA_DEVICE_ID_CFL_ULT_1:       // CFL ULT
        case V_SA_DEVICE_ID_CFL_ULT_2:       // CFL ULT
        case V_SA_DEVICE_ID_CFL_ULT_3:       // CFL ULT
        case V_SA_DEVICE_ID_CFL_ULT_4:       // CFL ULT
        case V_SA_DEVICE_ID_CFL_ULT_5:       // CFL ULT
          CpuType = EnumCpuUlt;
          break;

        default:
          SkuFound = FALSE;
          break;
      }
      break;

    case CPUID_FULL_FAMILY_MODEL_COFFEELAKE_DT_HALO:
      switch (CpuDid) {

        case V_SA_DEVICE_ID_KBL_DT_2:      // DT
        case V_SA_DEVICE_ID_KBL_SVR_2:     // Server
        case V_SA_DEVICE_ID_CFL_DT_1:      // DT
        case V_SA_DEVICE_ID_CFL_DT_2:      // DT
        case V_SA_DEVICE_ID_CFL_DT_3:      // DT
        case V_SA_DEVICE_ID_CFL_DT_4:      // DT
        case V_SA_DEVICE_ID_CFL_WS_1:      // WorkStation
        case V_SA_DEVICE_ID_CFL_WS_2:      // Workstation
        case V_SA_DEVICE_ID_CFL_WS_3:      // Workstation
        case V_SA_DEVICE_ID_CFL_SVR_1:     // Server
        case V_SA_DEVICE_ID_CFL_SVR_2:     // Server
        case V_SA_DEVICE_ID_CFL_SVR_3:     // Server
          CpuType = EnumCpuTrad;
          break;

        case V_SA_DEVICE_ID_KBL_HALO_2:    // Halo
        case V_SA_DEVICE_ID_CFL_HALO_1:    // Halo
        case V_SA_DEVICE_ID_CFL_HALO_2:    // Halo
        case V_SA_DEVICE_ID_CFL_HALO_3:    // Halo
        case V_SA_DEVICE_ID_CFL_HALO_IOT_1: // Halo IOT
          CpuType = EnumCpuHalo;
          break;

        default:
          SkuFound = FALSE;
          break;
      }
      break;

    default:
      SkuFound = FALSE;
      break;
#else
    case CPUID_FULL_FAMILY_MODEL_CANNONLAKE_ULT_ULX:
      switch (CpuDid) {
        case V_SA_DEVICE_ID_CNL_MB_ULT_1:    // ULT OPI
        case V_SA_DEVICE_ID_CNL_MB_ULT_2:    // Ult (4+3e)
          CpuType = EnumCpuUlt;
          break;

        case V_SA_DEVICE_ID_CNL_MB_ULX_1:    // ULX 2+2
        case V_SA_DEVICE_ID_CNL_MB_ULX_2:    // ULX OPI
          CpuType = EnumCpuUlx;
          break;

        default:
          SkuFound = FALSE;
          break;
      }
      break;

    case CPUID_FULL_FAMILY_MODEL_CANNONLAKE_DT_HALO:
      switch (CpuDid) {
        case V_SA_DEVICE_ID_CNL_DT_1:      // DT (6+2)
        case V_SA_DEVICE_ID_CNL_DT_2:      // DT (4+1)
          CpuType = EnumCpuTrad;
          break;

        case V_SA_DEVICE_ID_CNL_HALO_1:    // Halo (6+2)
        case V_SA_DEVICE_ID_CNL_HALO_2:    // Halo (8+2)
        case V_SA_DEVICE_ID_CNL_HALO_3:    // Halo (4+2)
          CpuType = EnumCpuHalo;
          break;

        default:
          SkuFound = FALSE;
          break;
      }
      break;
#endif // CPU_CFL
  }
#ifdef CFL_SIMICS
  CpuType = EnumCpuTrad;
#else
  if (!SkuFound) {
    DEBUG ((DEBUG_ERROR, "Unsupported CPU SKU, Device ID: 0x%02X, CPUID: 0x%08X!\n", CpuDid, CpuFamilyModel));
    ASSERT (FALSE);
  }
#endif

  return CpuType;
}

/**
  Return CPU Identifier used to identify various CPU types

  @retval CPU_OVERRIDE_IDENTIFIER           CPU Identifier
**/
CPU_OVERRIDE_IDENTIFIER
EFIAPI
GetCpuIdentifier (
  VOID
  )
{
  CPU_OVERRIDE_IDENTIFIER CpuIdentifier;
  CPU_SKU CpuSku;
  CPU_FAMILY  CpuFamily;
  UINT16 PackageTdp;
  UINT16 PackageTdpWatt;
  UINT16 TempPackageTdp;
  MSR_REGISTER TempMsr;
  MSR_REGISTER PackagePowerSkuUnitMsr;
  UINT8 ProcessorPowerUnit;
#ifdef CPU_CFL
  UINT16  CpuDid;
  UINT16  GtDid;

  CpuDid = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_MC_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));
  GtDid = PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, SA_IGD_DEV, SA_MC_FUN, R_SA_MC_DEVICE_ID));
#endif

  ///
  /// Initialize local variables
  ///
  CpuSku    = GetCpuSku ();
  CpuFamily = GetCpuFamily ();
  CpuIdentifier = EnumUnknownCpuId;

  ///
  /// Find Package TDP value in 1/100 Watt units
  ///
  TempMsr.Qword                 = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU);
  PackagePowerSkuUnitMsr.Qword  = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);
  ProcessorPowerUnit           = (PackagePowerSkuUnitMsr.Bytes.FirstByte & PACKAGE_POWER_UNIT_MASK);
  if (ProcessorPowerUnit == 0) {
    ProcessorPowerUnit = 1;
  } else {
    ProcessorPowerUnit = (UINT8) LShiftU64 (2, (ProcessorPowerUnit - 1));
  }
  TempPackageTdp = (UINT16) (TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK);
  PackageTdpWatt = (UINT16) DivU64x32 (TempPackageTdp, ProcessorPowerUnit);


  PackageTdp = (PackageTdpWatt * 100);
  if ((TempPackageTdp % ProcessorPowerUnit) !=0) {
    PackageTdp += ((TempPackageTdp % ProcessorPowerUnit) * 100) / ProcessorPowerUnit;
  }

  ///
  /// Logic to determine the CPU Identifier
  ///
  switch(CpuSku) {
    case EnumCpuUlx:
      if (CpuFamily == EnumCpuCnlUltUlx) {
        //
        //  CNL-Y
        //
        CpuIdentifier = EnumCnlY9Watt22CpuId;
      }
      break;

    case EnumCpuUlt:
      if (CpuFamily == EnumCpuCflUltUlx) {
#ifdef CPU_CFL
        if ((PackageTdp == CPU_TDP_28_WATTS) && (CpuDid == V_SA_DEVICE_ID_CFL_ULT_1)) {
          ///
          ///  CFL-U 4+3e
          ///
          CpuIdentifier = EnumCflU28Watt43eCpuId;
        } else if ((PackageTdp == CPU_TDP_28_WATTS) && (CpuDid == V_SA_DEVICE_ID_CFL_ULT_6)) {
          ///
          ///  CFL- U 2+3e
          ///
          CpuIdentifier = EnumCflU28Watt23eCpuId;
        } else if (PackageTdp == CPU_TDP_15_WATTS) {
            if (((CpuDid == V_SA_DEVICE_ID_CFL_ULT_1) || (CpuDid == V_SA_DEVICE_ID_CFL_ULT_3))
              && ((GtDid == V_SA_PCI_DEV_2_GT2_CFL_ULT_1_ID) || (GtDid == V_SA_PCI_DEV_2_GT2_CFL_ULT_3_ID))) {
               ///
               ///  WHL U 4+2f
               ///
               CpuIdentifier = EnumWhlU15Watt42fCpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_ULT_4) {
                ///
                ///  WHL U 2+2
                ///
                CpuIdentifier = EnumWhlU15Watt22fCpuId;
                if (GtDid == V_SA_PCI_DEV_2_GT1_CFL_ULT_1_ID) {
                  ///
                  ///  WHL U 2f+1f
                  ///
                  CpuIdentifier = EnumWhlU15Watt2f1fCpuId;
                }
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_ULT_1) {
              ///
              ///  CFL UR 4+3e
              ///
              CpuIdentifier = EnumCflUR15Watt43eCpuId;
            }
        }
#endif
      } else if (CpuFamily == EnumCpuCnlUltUlx) {
        ///
        ///  CNL-U 2+2
        ///
        if (PackageTdp == CPU_TDP_15_WATTS) {
          CpuIdentifier = EnumCnlU15Watt22CpuId;
        }
      }
      break;

#ifdef CPU_CFL
    case EnumCpuTrad:
      ///
      /// CFL-S
      ///
      if (CpuFamily == EnumCpuCflDtHalo) {
        switch (PackageTdp) {
          case CPU_TDP_35_WATTS:
            ///
            ///  35 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_3) {
              ///
              /// 2+2
              ///
              CpuIdentifier = EnumCflS35Watt22CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2
              ///
              CpuIdentifier = EnumCflS35Watt42CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_1) {
              ///
              /// 6+2
              ///
              CpuIdentifier = EnumCflS35Watt62CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_4 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_3 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_2) {
              ///
              /// 8+2
              ///
              CpuIdentifier = EnumCflS35Watt82CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_54_WATTS:
            ///
            ///  54 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_3) {
              ///
              /// 2+2
              ///
              CpuIdentifier = EnumCflS54Watt22CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_58_WATTS:
            ///
            ///  58 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_3) {
              ///
              /// 2+2
              ///
              CpuIdentifier = EnumCflS58Watt22CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_62_WATTS:
            ///
            ///  65 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2
              ///
              CpuIdentifier = EnumCflS62Watt42CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
            break;

          case CPU_TDP_65_WATTS:
            ///
            ///  65 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2
              ///
              CpuIdentifier = EnumCflS65Watt42CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_1) {
              ///
              /// 6+2
              ///
              CpuIdentifier = EnumCflS65Watt62CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_4 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_3 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_2) {
              ///
              /// 8+2
              ///
              CpuIdentifier = EnumCflS65Watt82CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_71_WATTS:
            ///
            ///  71 Watt Workstation
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2
              ///
              CpuIdentifier = EnumCflS71Watt42CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_83_WATTS:
            ///
            ///  83 Watt Workstation
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2
              ///
              CpuIdentifier = EnumCflS83Watt42CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_80_WATTS:
            ///
            ///  80 Watt Workstation
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_1 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_1 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_1) {
              ///
              /// 6+2
              ///
              CpuIdentifier = EnumCflS80Watt62CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_4 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_3 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_2) {
              ///
              /// 8+2
              ///
              CpuIdentifier = EnumCflS80Watt82CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;

          case CPU_TDP_91_WATTS:
          case CPU_TDP_95_WATTS:
            ///
            ///  95 Watts
            ///
            if (CpuDid == V_SA_DEVICE_ID_CFL_DT_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_WS_2 ||
                CpuDid == V_SA_DEVICE_ID_CFL_SVR_3) {
              ///
              /// 4+2 TDP reports as 91W
              ///
              CpuIdentifier = EnumCflS95Watt42CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_1 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_1) {
              ///
              /// 6+2, 95W
              ///
              CpuIdentifier = EnumCflS95Watt62CpuId;
            } else if (CpuDid == V_SA_DEVICE_ID_CFL_DT_4 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_WS_3 ||
                       CpuDid == V_SA_DEVICE_ID_CFL_SVR_2) {
              ///
              /// 8+2, 95W
              ///
              CpuIdentifier = EnumCflS95Watt82CpuId;
            } else {
              CpuIdentifier = EnumUnknownCpuId;
            }
          break;
          }
        }
      break;
#endif

    case EnumCpuHalo:
#ifdef CPU_CFL
      if (CpuFamily == EnumCpuCflDtHalo) {
        ///
        /// CFL-H
        ///
        if (PackageTdp == CPU_TDP_45_WATTS) {
          ///
          /// 45 Watt
          ///
          if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_2) {
            ///
            /// 4f+2
            ///
            CpuIdentifier = EnumCflH45Watt4f2CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_1) {
            ///
            /// 6+2
            ///
            CpuIdentifier = EnumCflH45Watt62CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_3) {
            ///
            /// 8+2
            ///
            CpuIdentifier = EnumCflH45Watt82CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_IOT_1) {
            ///
            /// 6+2 (IOT)
            ///
            CpuIdentifier = EnumCflH45Watt62CpuId;
          } else {
            CpuIdentifier = EnumUnknownCpuId;
          }
        } else if (PackageTdp == CPU_TDP_65_WATTS) {
          ///
          /// 65 Watt
          ///
          if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_1) {
            ///
            /// 6+2
            ///
            CpuIdentifier = EnumCflH65Watt62CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_3) {
            ///
            /// 8+2
            ///
            CpuIdentifier = EnumCflH65Watt82CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_IOT_1) {
            ///
            /// 6+2 (IOT)
            ///
            CpuIdentifier = EnumCflH65Watt62CpuId;
          } else if (CpuDid == V_SA_DEVICE_ID_CFL_HALO_2) {
            ///
            /// 4+2
            ///
            CpuIdentifier = EnumCflH65Watt42CpuId;
          } else {
            CpuIdentifier = EnumUnknownCpuId;
          }
        }
      }
#else
      if (CpuFamily == EnumCpuCnlDtHalo) {
        ///
        /// CNL-H
        ///
        if (PackageTdp == CPU_TDP_45_WATTS) {
          CpuIdentifier = EnumCnlH45Watt82CpuId;
        } else if (PackageTdp == CPU_TDP_65_WATTS) {
          CpuIdentifier = EnumCnlH65Watt82CpuId;
        } else if (PackageTdp == CPU_TDP_95_WATTS) {
          CpuIdentifier = EnumCnlH95Watt82CpuId;
        } else if (PackageTdp == CPU_TDP_120_WATTS) {
          CpuIdentifier = EnumCnlH120Watt82CpuId;
        } else {
          CpuIdentifier = EnumUnknownCpuId;
        }
      }
#endif
      break;

    default:
      CpuIdentifier = EnumUnknownCpuId;
      break;
    }

  return CpuIdentifier;
}

/**
  Returns the processor microcode revision of the processor installed in the system.

  @retval Processor Microcode Revision
**/
UINT32
GetCpuUcodeRevision (
  VOID
  )
{
  AsmWriteMsr64 (MSR_IA32_BIOS_SIGN_ID, 0);
  AsmCpuid (CPUID_VERSION_INFO, NULL, NULL, NULL, NULL);
  return (UINT32) RShiftU64 (AsmReadMsr64 (MSR_IA32_BIOS_SIGN_ID), 32);
}

/**
  Verify the DWORD type checksum

  @param[in] ChecksumAddr  - The start address to be checkumed
  @param[in] ChecksumLen   - The length of data to be checksumed

  @retval EFI_SUCCESS           - Checksum correct
  @retval EFI_CRC_ERROR         - Checksum incorrect
**/
EFI_STATUS
Checksum32Verify (
  IN UINT32 *ChecksumAddr,
  IN UINT32 ChecksumLen
  )
{
#if SKIP_MICROCODE_CHECKSUM_CHECK
  return EFI_SUCCESS;
#else
  UINT32 Checksum;
  UINT32 Index;

  Checksum = 0;

  for (Index = 0; Index < ChecksumLen; Index++) {
    Checksum += ChecksumAddr[Index];
  }

  return (Checksum == 0) ? EFI_SUCCESS : EFI_CRC_ERROR;
#endif
}

/**
  This function checks the MCU revision to decide if BIOS needs to load
  microcode.

  @param[in] MicrocodePointer - Microcode in memory
  @param[in] Revision         - Current CPU microcode revision

  @retval EFI_SUCCESS - BIOS needs to load microcode
  @retval EFI_ABORTED - Don't need to update microcode
**/
EFI_STATUS
CheckMcuRevision (
  IN CPU_MICROCODE_HEADER *MicrocodePointer,
  IN UINT32               Revision
  )
{
  EFI_STATUS Status;
  Status = EFI_ABORTED;

  if ((MicrocodePointer->UpdateRevision & 0x80000000) ||
      (MicrocodePointer->UpdateRevision > Revision) ||
      (Revision == 0)) {
    Status = EFI_SUCCESS;
  }

  return Status;
}

/**
  Check if this microcode is correct one for processor

  @param[in] Cpuid               - processor CPUID
  @param[in] MicrocodeEntryPoint - entry point of microcode
  @param[in] Revision            - revision of microcode

  @retval CorrectMicrocode if this microcode is correct
**/
BOOLEAN
CheckMicrocode (
  IN UINT32               Cpuid,
  IN CPU_MICROCODE_HEADER *MicrocodeEntryPoint,
  IN UINT32               *Revision
  )
{
  EFI_STATUS                          Status;
  UINT8                               ExtendedIndex;
  MSR_IA32_PLATFORM_ID_REGISTER       Msr;
  UINT32                              ExtendedTableLength;
  UINT32                              ExtendedTableCount;
  BOOLEAN                             CorrectMicrocode;
  CPU_MICROCODE_EXTENDED_TABLE        *ExtendedTable;
  CPU_MICROCODE_EXTENDED_TABLE_HEADER *ExtendedTableHeader;

  Status              = EFI_NOT_FOUND;
  ExtendedTableLength = 0;
  CorrectMicrocode    = FALSE;

  if (MicrocodeEntryPoint == NULL) {
    return FALSE;
  }

  Msr.Uint64 = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);

  ///
  /// Check if the microcode is for the Cpu and the version is newer
  /// and the update can be processed on the platform
  ///
  if ((MicrocodeEntryPoint->HeaderVersion == 0x00000001) &&
      !EFI_ERROR (CheckMcuRevision (MicrocodeEntryPoint, *Revision))
      ) {
    if ((MicrocodeEntryPoint->ProcessorId == Cpuid) && (MicrocodeEntryPoint->ProcessorFlags & (1 << (UINT8) Msr.Bits.PlatformId))) {
      if (MicrocodeEntryPoint->DataSize == 0) {
        Status = Checksum32Verify ((UINT32 *) MicrocodeEntryPoint, 2048 / sizeof (UINT32));
      } else {
        Status = Checksum32Verify (
                   (UINT32 *) MicrocodeEntryPoint,
                   (MicrocodeEntryPoint->DataSize + sizeof (CPU_MICROCODE_HEADER)) / sizeof (UINT32)
                   );
      }

      if (!EFI_ERROR (Status)) {
        CorrectMicrocode = TRUE;
      }
    } else if ((MicrocodeEntryPoint->DataSize != 0)) {
      ///
      /// Check the  Extended Signature if the entended signature exist
      /// Only the data size != 0 the extended signature may exist
      ///
      ExtendedTableLength = MicrocodeEntryPoint->TotalSize - (MicrocodeEntryPoint->DataSize + sizeof (CPU_MICROCODE_HEADER));
      if (ExtendedTableLength != 0) {
        ///
        /// Extended Table exist, check if the CPU in support list
        ///
        ExtendedTableHeader = (CPU_MICROCODE_EXTENDED_TABLE_HEADER *) ((UINT8 *) (MicrocodeEntryPoint) + MicrocodeEntryPoint->DataSize + 48);
        ///
        /// Calulate Extended Checksum
        ///
        if ((ExtendedTableLength % 4) == 0) {
          Status = Checksum32Verify ((UINT32 *) ExtendedTableHeader, ExtendedTableLength / sizeof (UINT32));
          if (!EFI_ERROR (Status)) {
            ///
            /// Checksum correct
            ///
            ExtendedTableCount  = ExtendedTableHeader->ExtendedSignatureCount;
            ExtendedTable       = (CPU_MICROCODE_EXTENDED_TABLE *) (ExtendedTableHeader + 1);
            for (ExtendedIndex = 0; ExtendedIndex < ExtendedTableCount; ExtendedIndex++) {
              ///
              /// Verify Header
              ///
              if ((ExtendedTable->ProcessorSignature == Cpuid) && (ExtendedTable->ProcessorFlag & (1 << (UINT8) Msr.Bits.PlatformId))) {
                Status = Checksum32Verify (
                           (UINT32 *) ExtendedTable,
                           sizeof (CPU_MICROCODE_EXTENDED_TABLE) / sizeof (UINT32)
                           );
                if (!EFI_ERROR (Status)) {
                  ///
                  /// Find one
                  ///
                  CorrectMicrocode = TRUE;
                  break;
                }
              }

              ExtendedTable++;
            }
          }
        }
      }
    }
  }

  return CorrectMicrocode;
}

/**
  This function is to program Trace Hub ACPI base address to processor's MSR TRACE_HUB_STH_ACPIBAR_BASE.

  @param[in]  TraceHubAcpiBase - Base address of Trace Hub ACPI Base address
**/
VOID
EFIAPI
CpuWriteTraceHubAcpiBase (
  IN UINT64  TraceHubAcpiBase
  )
{
  //
  // Check the pass in Trace Hub ACPI base if 256KB alignment.
  //
  if ((TraceHubAcpiBase & (UINT64) V_MSR_TRACE_HUB_STH_ACPIBAR_BASE_MASK) != 0) {
    ASSERT ((TraceHubAcpiBase & (UINT64) V_MSR_TRACE_HUB_STH_ACPIBAR_BASE_MASK) == 0);
    return;
  }

  ///
  /// Set MSR TRACE_HUB_STH_ACPIBAR_BASE[0] LOCK bit for the AET packets to be directed to NPK MMIO.
  ///
  AsmWriteMsr64 (MSR_TRACE_HUB_STH_ACPIBAR_BASE, TraceHubAcpiBase | B_MSR_TRACE_HUB_STH_ACPIBAR_BASE_LOCK);

  return;
}

/**
  Check on the processor if SGX is supported.

  @dot
    digraph G {
      subgraph cluster_c0 {
        node [shape = box];
          b1[label="Read CPUID(EAX=7,ECX=0):EBX[2] \nto check SGX feature" fontsize=12 style=filled color=lightblue];
          b2[label="Return TRUE" fontsize=12 style=filled color=lightblue];
          b3[label="Return FALSE" fontsize=12 style=filled color=lightblue];

        node [shape = ellipse];
          e1[label="Start" fontsize=12 style=filled color=lightblue];
          e2[label="End" fontsize=12 style=filled color=lightblue];

        node [shape = diamond,style=filled,color=lightblue];
          d1[label="Are SGX feature supported and \nPRMRR configuration enabled" fontsize=12];

        label = "IsSgxSupported Flow"; fontsize=15; fontcolor=black; color=lightblue;
        e1 -> b1
        b1 -> d1
        d1 -> b2 [label="Yes" fontsize=9]
        d1 -> b3 [label="No" fontsize=9]
        b2 -> e2
        b3 -> e2

      }
    }
  @enddot

  @retval TRUE  if SGX supported
  @retval FALSE if SGX is not supported
**/
BOOLEAN
IsSgxSupported (
  VOID
  )
{
  EFI_CPUID_REGISTER CpuidRegs;

  //
  // Processor support SGX feature by reading CPUID.(EAX=7,ECX=0):EBX[2]
  //
  AsmCpuidEx (CPUID_STRUCTURED_EXTENDED_FEATURE_FLAGS, 0, &CpuidRegs.RegEax,&CpuidRegs.RegEbx,&CpuidRegs.RegEcx,&CpuidRegs.RegEdx);

  ///
  /// SGX feature is supported with CPUID.(EAX=7,ECX=0):EBX[2]=1
  /// PRMRR configuration enabled, MSR IA32_MTRRCAP (FEh) [12] == 1
  ///
  if (((CpuidRegs.RegEbx & BIT2)) && (AsmReadMsr64 (MSR_IA32_MTRRCAP) & BIT12)) {
    return TRUE;
  }
  return FALSE;
}

/**
  Detect if C6DRAM supported or not by reading it from PCODE mailbox
  If C6DRAM is supported, configures whether it is enabled based on input parameter.

  @param[in]  EnableC6Dram - Policy setting for C6DRAM

  @retval TRUE - Supported
  @retval FALSE - Not Supported
**/
BOOLEAN
IsC6dramSupported (
  IN UINT32    EnableC6Dram
  )
{
  UINT32       LibStatus;
  UINT32       C6DramStatus;
  EFI_STATUS   Status;

  LibStatus    = 0x0;

  ///
  /// For C6DRAM, PCODE mailbox returns fuse_c6dram_en && C6DRAM_ENABLE.
  /// In order to read only the fuse_c6dram_en status, BIOS must send C6DRAM_ENABLE = 1 to the mailbox, and read the value.
  /// Then, if C6DRAM is disabled by policy, C6DRAM_DISABLE = 0 is sent to the mailbox.
  ///
  DEBUG ((DEBUG_INFO, "Reading fuse_c6dram_en status, first writing 0x1D command to Pcode mailbox, then reading the answer from the mailbox\n"));
  Status = MailboxWrite(MAILBOX_TYPE_PCODE, MAILBOX_BIOS_ALLOW_C6DRAM_CMD, C6DRAM_ENABLE , &LibStatus);
  if (Status != EFI_SUCCESS || LibStatus != EFI_SUCCESS) {
    DEBUG ((DEBUG_ERROR, "Mailbox write command failed unexpectedly, C6DRAM is not supported. LibStatus = %x , Mailbox command return status %r \n", LibStatus , Status));
    return FALSE;
  } else {
    Status = MailboxRead(MAILBOX_TYPE_PCODE, MAILBOX_BIOS_ALLOW_C6DRAM_CMD, &C6DramStatus, &LibStatus);
    if (Status != EFI_SUCCESS || LibStatus != EFI_SUCCESS) {
      DEBUG ((DEBUG_ERROR, "Mailbox read command failed unexpectedly, C6DRAM is not supported. LibStatus = %x , Mailbox command return status %r \n", LibStatus , Status));
      return FALSE;
    } else {
      if(C6DramStatus == C6DRAM_ENABLE && EnableC6Dram == C6DRAM_DISABLE){
        ///
        /// BIOS enabled C6DRAM to check hardware support. Now disabling due to policy setting.
        ///
        MailboxWrite(MAILBOX_TYPE_PCODE, MAILBOX_BIOS_ALLOW_C6DRAM_CMD, C6DRAM_DISABLE , &LibStatus);
      }
      DEBUG ((DEBUG_INFO, "fuse_c6dram_en status %x \n", C6DramStatus));
      return (BOOLEAN) C6DramStatus;
    }
  }
}

/**
  Get processor generation

  @retval CPU_GENERATION  Returns the executing thread's processor generation.
**/
CPU_GENERATION
GetCpuGeneration (
  VOID
  )
{
  EFI_CPUID_REGISTER Cpuid;
  CPU_FAMILY         CpuFamilyModel;
  CPU_GENERATION     CpuGeneration;

  CpuGeneration = EnumCflCpu;
  ///
  /// Read the CPUID information
  ///
  AsmCpuid (CPUID_VERSION_INFO, &Cpuid.RegEax, &Cpuid.RegEbx, &Cpuid.RegEcx, &Cpuid.RegEdx);
  CpuFamilyModel = (CPU_FAMILY) (Cpuid.RegEax & CPUID_FULL_FAMILY_MODEL);

  switch (CpuFamilyModel) {
    case EnumCpuCflUltUlx:
    case EnumCpuCflDtHalo:
      CpuGeneration = EnumCflCpu;
      break;

    case EnumCpuCnlUltUlx:
    case EnumCpuCnlDtHalo:
      CpuGeneration = EnumCnlCpu;
      break;

    default:
      CpuGeneration = EnumCpuUnknownGeneration;
      ASSERT (FALSE);
      break;
  }

  return CpuGeneration;
}

/**
  Check if Disable CPU Debug (DCD) bit is set from FIT CPU Debugging [Disabled].
  If it is set, CPU probe mode is disabled.

  @retval TRUE    DCD is set
  @retval FALSE   DCD is clear
**/
BOOLEAN
IsCpuDebugDisabled (
  VOID
  )
{
  UINT32                  MeFwSts6;

  MeFwSts6 = PciSegmentRead32 (
               PCI_SEGMENT_LIB_ADDRESS (
                 ME_SEGMENT,
                 ME_BUS,
                 ME_DEVICE_NUMBER,
                 HECI_FUNCTION_NUMBER,
                 R_ME_HFS_6
                 )
               );
  if (MeFwSts6 & B_ME_HFS_6_DCD) {
    return TRUE;
  }
  return FALSE;
}

/**
  Is BIOS GUARD enabled.

  @retval TRUE   BIOS GUARD is supported and enabled.
  @retval FALSE  BIOS GUARD is disabled.
**/
BOOLEAN
IsBiosGuardEnabled (
  VOID
  )
{
  if (AsmReadMsr64 (MSR_PLATFORM_INFO) & B_MSR_PLATFORM_INFO_BIOSGUARD_AVAIL) {
    if (AsmReadMsr64 (MSR_PLAT_FRMW_PROT_CTRL) & B_MSR_PLAT_FRMW_PROT_CTRL_EN) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Is Whiskey Lake CPU.

  @retval TRUE  The CPUID corresponds with a Whiskey Lake CPU
  @retval FALSE The CPUID does not correspond with a Whiskey Lake CPU
**/
BOOLEAN
IsWhlCpu (
  VOID
  )
{
  CPU_FAMILY    CpuFamily;
  CPU_STEPPING  CpuStepping;

  CpuFamily   = GetCpuFamily ();
  CpuStepping = GetCpuStepping ();

  //
  // Check if it is Whiskey Lake CPU
  //
  if ((CpuFamily == EnumCpuCflUltUlx) && ((CpuStepping == EnumCflW0) || (CpuStepping == EnumCflV0))) {
    return TRUE;
  }

  return FALSE;
}

/**
  Is Nifty Rock feature supported.

  @retval TRUE   Nifty Rock feature is supported.
  @retval FALSE  Nifty Rock feature is not supported.
**/
BOOLEAN
IsNiftyRockSupported (
  VOID
  )
{
  CPU_FAMILY    CpuFamily;
  CPU_STEPPING  CpuStepping;

  CpuFamily   = GetCpuFamily ();
  CpuStepping = GetCpuStepping ();

  //
  // Check if it is Whiskey Lake CPU
  //
  if (IsWhlCpu ()) {
    return TRUE;
  }

  //
  // Check if it is CFL-S 8+2 or CFL-H 8+2
  //
  if ((CpuStepping >= EnumCflP0) && (CpuFamily == EnumCpuCflDtHalo)) {
    return TRUE;
  }

  return FALSE;
}