/** @file
  This code provides a initialization of intel VT-d (Virtualization Technology for Directed I/O).

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
#include <Uefi/UefiBaseType.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PostCodeLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PciSegmentLib.h>
#include <SaAccess.h>
#include <Private/Library/VtdInitLib.h>
#include <Library/CpuPlatformLib.h>
#include <Private/Library/PchPsfPrivateLib.h>
#include <CpuRegs.h>
#include <Private/SaConfigHob.h>
#include <ConfigBlock/SiConfig.h>
#include <Library/BootGuardLib.h>

extern EFI_GUID gSaConfigHobGuid;

/**
  Configure VT-d Base and capabilities in Pre-Mem.

  @param[in]   VTD_CONFIG                 VTD config block from SA Policy PPI

  @retval EFI_SUCCESS     - Vtd initialization complete
  @exception EFI_UNSUPPORTED - Vtd is not enabled by policy
**/
EFI_STATUS
VtdInitPreMem (
  IN       VTD_CONFIG                        *Vtd
  )
{
  UINTN          i;
  UINT64         McD0BaseAddress;
  UINTN          MchBar;
  UINT32         Data32Or;
  UINT32         MsrValue;
  UINT32         VtdBaseAddress3;
  UINT32         AcmPolicySts;

  McD0BaseAddress             = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, 0);
  MchBar                      = PciSegmentRead32 (McD0BaseAddress + R_SA_MCHBAR) &~BIT0;


   if (IsBootGuardSupported()){
     MsrValue = (UINT32) AsmReadMsr64 (MSR_BOOT_GUARD_SACM_INFO);
     if ((MsrValue & B_BOOT_GUARD_SACM_INFO_NEM_ENABLED) == 1) {
        AcmPolicySts = MmioRead32 (MMIO_ACM_POLICY_STATUS );
        if  (AcmPolicySts & (BIT29)){
          //dma protection is enabled
          DEBUG ((DEBUG_INFO, "[Boot Guard]Clear DMA protectedtion set by BTG ACM\n"));
          VtdBaseAddress3 = MmioRead32 (MchBar + R_SA_MCHBAR_VTD3_OFFSET) &~BIT0;
          if (VtdBaseAddress3 > 0){
            MmioWrite32 ((VtdBaseAddress3 + R_SA_VTD_PMEN_OFFSET) , 0);
            //Read-back
            MmioRead32 (VtdBaseAddress3 + R_SA_VTD_PMEN_OFFSET);
            MmioWrite32 ((VtdBaseAddress3 + R_SA_VTD_PLMBASE_OFFSET), 0);
            MmioWrite32 ((VtdBaseAddress3 + R_SA_VTD_PLMLIMIT_OFFSET), 0);
            MmioWrite32 ((MchBar + R_SA_MCHBAR_VTD3_OFFSET), 0);
           } //VtdBaseAddress3 > 0
         } // AcmPolicySts & (BIT29)
       }  //NEM enabled by SACM
    } // IsBootGuardSupported()

  if ((Vtd->VtdDisable) || (PciSegmentRead32 (McD0BaseAddress + R_SA_MC_CAPID0_A_OFFSET) & BIT23)) {
    //
    // W/A: BIOS is required to always set IPUVTDBAR_LOW.VTD_ENABLE in CNL B0 (even if VTD is disabled).
    //
    if ((GetCpuFamily () == EnumCpuCnlUltUlx) && (GetCpuStepping () == EnumCnlB0)) {
      PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, R_SA_VTD_IPU_UBAR), 0);
      PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, R_SA_VTD_IPU_LBAR), BIT0);
    }
    DEBUG ((DEBUG_WARN, "VTd disabled or no capability!\n"));
    return EFI_UNSUPPORTED;
  }
  ///
  /// Check SA supports VTD and VTD is enabled in setup menu
  ///
  DEBUG ((DEBUG_INFO, "VTd enabled\n"));

  ///
  /// Enable VTd in PCH
  ///
  PchPsfEnableVtd ();

  ///
  /// Program Remap Engine Base Address
  ///
  i = 0;

  ///
  /// Configure VTD1 BAR
  /// Skip GFXVTBAR if IGD is disabled
  ///
  if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 2, 0, PCI_VENDOR_ID_OFFSET)) != 0xFFFF) {
    Data32Or = Vtd->BaseAddress[i];
    Data32Or |= 0x1;
    MmioWrite32 (MchBar + R_SA_MCHBAR_VTD1_OFFSET, Data32Or);
  }
  i++;

#ifndef CPU_CFL
  ///
  /// Configure VTD2 BAR
  /// Skip IPUVTBAR if IPU is disabled
  ///
  if (PciSegmentRead16(PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, PCI_VENDOR_ID_OFFSET)) != V_SA_DEVICE_ID_INVALID) {
    //
    // Workaround for Blue Screen only on CNL Simics package
    //
    if (PcdGetBool (PcdSimicsEnable) == FALSE) {
      Data32Or = Vtd->BaseAddress[i];
      Data32Or |= 0x1;
      PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, R_SA_VTD_IPU_UBAR), 0);
      PciSegmentWrite32 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, R_SA_VTD_IPU_LBAR), Data32Or);
      MmioWrite32 (MchBar + R_SA_MCHBAR_VTD2_HIGH_OFFSET, 0);
      MmioWrite32 (MchBar + R_SA_MCHBAR_VTD2_LOW_OFFSET, Data32Or);
    }
  }
#endif
  i++;

  ///
  /// Configure VTD3 BAR
  ///
  Data32Or = Vtd->BaseAddress[i];
  Data32Or |= 0x1;
  MmioWrite32 (MchBar + R_SA_MCHBAR_VTD3_OFFSET, Data32Or);

  return EFI_SUCCESS;
}

/**
  Configure VT-d Base and capabilities for IPs available in PostMem

  @param[in]   VTD_CONFIG                 VTD config block from SA Policy PPI

  @retval EFI_SUCCESS     - Vtd initialization complete
  @exception EFI_UNSUPPORTED - Vtd is not enabled by policy
**/
EFI_STATUS
VtdInitPostMem (
  IN       VTD_CONFIG                        *Vtd
  )
{
  UINTN          i;
  UINT64         McD0BaseAddress;
  UINTN          MchBar;
  UINT32         VtdBase;
  BOOLEAN        VtdIntRemapSupport [SA_VTD_ENGINE_NUMBER];
  UINT32         VtBarReg [SA_VTD_ENGINE_NUMBER];
  SA_CONFIG_HOB  *SaConfigHob;
#ifdef CPU_CFL
  UINT32         VtdRegOffsetFF0;
  UINT32         VtdRegOffsetFF4;
#else
  BOOLEAN        IpuExisted;
#endif
  BOOLEAN        IgdExisted;

  SaConfigHob       = NULL;
  SaConfigHob = GetFirstGuidHob (&gSaConfigHobGuid);
  if (SaConfigHob == NULL) {
    DEBUG ((DEBUG_WARN, "Failed to retrieve SaConfigHob for VT-d! VT-d cannot be enabled!\n"));
    return EFI_UNSUPPORTED;
  }

  McD0BaseAddress             = PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 0, 0, 0);
  MchBar                      = PciSegmentRead32 (McD0BaseAddress + R_SA_MCHBAR) &~BIT0;
  VtBarReg[0]                 = R_SA_MCHBAR_VTD1_OFFSET;
#ifndef CPU_CFL
  VtBarReg[1]                 = R_SA_MCHBAR_VTD2_LOW_OFFSET;
#endif
  VtBarReg[2]                 = R_SA_MCHBAR_VTD3_OFFSET;

#ifndef CPU_CFL
  IpuExisted                  = FALSE;
#endif
  IgdExisted                  = FALSE;


  ///
  /// Check IGD existed or not
  ///
  if (PciSegmentRead16 (PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_MC_BUS, 2, 0, PCI_VENDOR_ID_OFFSET)) != 0xFFFF) {
    IgdExisted = TRUE;
  }

#ifndef CPU_CFL
  ///
  /// Check IPU existed or not
  ///
  if (PciSegmentRead16(PCI_SEGMENT_LIB_ADDRESS (SA_SEG_NUM, SA_IPU_BUS_NUM, SA_IPU_DEV_NUM, SA_IPU_FUN_NUM, PCI_VENDOR_ID_OFFSET)) != V_SA_DEVICE_ID_INVALID) {
    IpuExisted = TRUE;
  }
#endif

  for (i = 0; i < SA_VTD_ENGINE_NUMBER; i++) {
    //
    // Set InterruptRemappingSupport default to FALSE
    //
    VtdIntRemapSupport[i] = FALSE;
#ifdef CPU_CFL
    ///
    /// Skip VtBarReg[1] since CFL doens't support VTD for IPU
    ///
    if (i == 1) {
      continue;
    }
#endif
    VtdBase = MmioRead32 (MchBar + VtBarReg[i]) & 0xfffffffe;

    ///
    /// skip if the VT bar is 0
    ///
    if (VtdBase == 0) {
      continue;
    }

#ifdef CPU_CFL
    ///
    /// Initialize register value before overrides
    /// Register settings will be overridden basing on requirements
    /// All registers will be updated in the end of the loop (for register locking sequence consideration)
    ///
    VtdRegOffsetFF4 = MmioRead32 (VtdBase + 0xFF4);
    VtdRegOffsetFF0 = MmioRead32 (VtdBase + 0xFF0);
    if ((VtdRegOffsetFF0 & BIT31) == BIT31) {
      DEBUG ((DEBUG_INFO, "Lock bit of this VT engine has been set. Skip initialization.\n"));
      continue;
    }
    ///
    /// Overrides
    ///
    ///
    /// Set lock bit
    ///
    VtdRegOffsetFF0 |= BIT31;

    if (i == 0) {
      MmioWrite32 (VtdBase + 0x100, 0x50A);
    }

    ///
    /// Program all register after all overrides applied
    ///
    MmioWrite32 (VtdBase + 0xFF4, VtdRegOffsetFF4);
    MmioWrite32 (VtdBase + 0xFF0, VtdRegOffsetFF0);
#endif

    ///
    /// Check IR status and update the InterruptRemappingSupport
    ///
    if (MmioRead32 (VtdBase + VTD_ECAP_REG) & IR) {
      VtdIntRemapSupport[i] = TRUE;
    }
  }

  //
  // Set IR support default as false
  //
  SaConfigHob->VtdData.InterruptRemappingSupport = FALSE;
#ifdef CPU_CFL
  if (IgdExisted == TRUE) {
    if (VtdIntRemapSupport[0] == TRUE  && VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  } else {
    if (VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  }
#else
  if (IgdExisted == TRUE && IpuExisted == TRUE) {
    if (VtdIntRemapSupport[0] == TRUE && VtdIntRemapSupport[1] == TRUE && VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  } else if (IgdExisted == TRUE && IpuExisted == FALSE) {
    if (VtdIntRemapSupport[0] == TRUE && VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  } else if (IgdExisted == FALSE && IpuExisted == TRUE) {
    if (VtdIntRemapSupport[1] == TRUE && VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  } else {
    if (VtdIntRemapSupport[2] == TRUE) {
      SaConfigHob->VtdData.InterruptRemappingSupport = TRUE;
    }
  }
#endif

  return EFI_SUCCESS;
}
