/** @file
  Header file for PCH Initialization Driver.

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
#ifndef _PCH_INIT_DXE_H_
#define _PCH_INIT_DXE_H_

#include <Protocol/PchEmmcTuning.h>
#include <SiConfigHob.h>
#include <Private/PchConfigHob.h>
#include <Private/Protocol/PchNvsArea.h>

//
// Data definitions
//
extern EFI_HANDLE               mImageHandle;

//
// Pch NVS area definition
//
extern PCH_NVS_AREA_PROTOCOL    mPchNvsAreaProtocol;

extern PCH_CONFIG_HOB           *mPchConfigHob;
extern SI_CONFIG_HOB_DATA       *mSiConfigHobData;

//
// Function Prototype
//

//
// Local function prototypes
//
/**
  Initialize the PCH device according to the PCH Policy HOB
  and install PCH info instance.

**/
VOID
InitializePchDevice (
  VOID
  );

/**
  Common PchInit Module Entry Point
**/
VOID
PchInitEntryPointCommon (
  VOID
  );

/**
  Common PCH initialization on PCI enumeration complete.
**/
VOID
PchOnPciEnumCompleteCommon (
  VOID
  );

/**
  Configures Serial IO Controllers

**/
EFI_STATUS
ConfigureSerialIoAtBoot (
  VOID
  );

/**
  Creates device handles for SerialIo devices in ACPI mode

**/
VOID
CreateSerialIoHandles (
  VOID
  );

/**
  Mark memory used by SerialIo devices in ACPI mode as allocated

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
AllocateSerialIoMemory (
  VOID
  );

/**
  Puts all SerialIo controllers (except UARTs in debug mode) in D3.
  Clears MemoryEnable for all PCI-mode controllers on S3 resume
**/
VOID
ConfigureSerialIoAtS3Resume (
  VOID
  );

/**
  Update ASL definitions for SerialIo devices.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
UpdateSerialIoAcpiData (
  VOID
  );

/**
  Initialize PCIE SRC clocks in ICC subsystem

  @param[in] GbePortNumber        Number of PCIE rootport assigned to GbE adapter

**/
VOID
ConfigurePchPcieClocks (
  IN UINTN                        GbePortNumber
  );

/**
  Initialize Intel High Definition Audio ACPI Tables

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_LOAD_ERROR          ACPI table cannot be installed
  @retval EFI_UNSUPPORTED         ACPI table not set because DSP is disabled
**/
EFI_STATUS
PchHdAudioAcpiInit (
  VOID
  );

/**
  Configure eMMC in HS400 Mode

  @param[in] This                         A pointer to PCH_EMMC_TUNING_PROTOCOL structure
  @param[in] Revision                     Revision parameter used to verify the layout of EMMC_INFO and TUNINGDATA.
  @param[in] EmmcInfo                     A pointer to EMMC_INFO structure
  @param[out] EmmcTuningData              A pointer to EMMC_TUNING_DATA structure

  @retval EFI_SUCCESS                     The function completed successfully
  @retval EFI_NOT_FOUND                   The item was not found
  @retval EFI_OUT_OF_RESOURCES            The request could not be completed due to a lack of resources.
  @retval EFI_INVALID_PARAMETER           A parameter was incorrect.
  @retval EFI_DEVICE_ERROR                Hardware Error
  @retval EFI_NO_MEDIA                    No media
  @retval EFI_MEDIA_CHANGED               Media Change
  @retval EFI_BAD_BUFFER_SIZE             Buffer size is bad
  @retval EFI_CRC_ERROR                   Command or Data CRC Error
**/
EFI_STATUS
EFIAPI
ConfigureEmmcHs400Mode (
  IN  PCH_EMMC_TUNING_PROTOCOL          *This,
  IN  UINT8                             Revision,
  IN  EMMC_INFO                         *EmmcInfo,
  OUT EMMC_TUNING_DATA                  *EmmcTuningData
  );

/**
  Install PCH EMMC TUNING PROTOCOL

**/
VOID
InstallPchEmmcTuningProtocol (
  VOID
  );

/**
  Get eMMC PCI cfg space address

  @return UINT64  PCI base address
**/
UINT64
ScsGetEmmcBaseAddress (
  VOID
  );

/**
  Perform the remaining configuration on PCH SATA to perform device detection,
  then set the SATA SPD and PxE corresponding, and set the Register Lock on PCH SATA

  @retval None
**/
VOID
ConfigurePchSataOnEndOfDxe (
  VOID
  );

/**
  Update ASL data for CNVI Device.

  @retval EFI_SUCCESS             The function completed successfully
**/
EFI_STATUS
UpdateCnviAcpiData (
   VOID
   );

/**
  Initialize Pch acpi
  @param[in] ImageHandle          Handle for the image of this driver

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver
**/
EFI_STATUS
PchAcpiInit (
  IN EFI_HANDLE         ImageHandle
  );

/**
  Update ASL object before Boot

  @retval EFI_STATUS
  @retval EFI_NOT_READY         The Acpi protocols are not ready.
**/
EFI_STATUS
PchUpdateNvsArea (
  VOID
  );

/**
  Initialize PCH Nvs Area opeartion region.

**/
VOID
PatchPchNvsAreaAddress (
  VOID
  );

#endif // _PCH_INIT_DXE_H_
