/** @file
  Definition of PCIe BDAT schemas

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
#ifndef PCIE_TRAINING_BDAT_H_
#define PCIE_TRAINING_BDAT_H_

#include <Bdat4.h>

extern EFI_GUID gSchemaListGuid;
extern EFI_GUID gEqPhase3SchemaGuid;
extern EFI_GUID gScoreSchemaGuid;
extern EFI_GUID gPortMarginGuid;
extern EFI_GUID gLaneMarginGuid;
extern EFI_GUID gJitterTolerenceGuid;
extern EFI_GUID gVocMarginGuid;

#define BDAT_PCIE_MAX_LINK_WIDTH     32

#pragma pack(push, 1)

///
/// Common Structure Definitions
///
typedef struct {
  UINT8                             Bus;
  UINT8                             Device;
  UINT8                             Function;
  UINT8                             Reserved;
} BDAT_PCI_DEVICE;

typedef union {
  UINT32                            Data;
  struct {
    UINT16                          VendorId;
    UINT16                          DeviceId;
  } Ids;
} BDAT_PCI_DEVICE_ID;

///
/// Structure definitions for the BDAT_PCIE_EQ_PHASE3_STRUCTURE
///
typedef struct {
  UINT8                             Major;
  UINT8                             Minor;
  UINT8                             Rev;
  UINT8                             Build;
} BDAT_PCIE_CODE_VERSION;

typedef struct {
  BDAT_PCI_DEVICE                   RootPort;
  BDAT_PCI_DEVICE                   PhyPort;
  BDAT_PCI_DEVICE_ID                EndpointId;
  UINT8                             LinkIsGen3Capable;
  UINT8                             Reserved1;
  UINT8                             Reserved2;
  UINT8                             Reserved3;
} BDAT_PCIE_ROOT_PORT;

typedef struct {
  UINT8                             PhysicalLane;
  UINT8                             LogicalLane;
  UINT8                             BestPreset;
  UINT8                             Reserved;
} BDAT_PCIE_LANE_BEST_PRESET;

typedef struct {
  BDAT_PCIE_ROOT_PORT               Port;
  UINT8                             LaneCount;
  UINT8                             Reserved1;
  UINT8                             Reserved2;
  UINT8                             Reserved3;
  BDAT_PCIE_LANE_BEST_PRESET        Lanes[BDAT_PCIE_MAX_LINK_WIDTH];
} BDAT_PCIE_PORT_BEST_PRESET;

typedef struct {
  BDAT_SCHEMA_HEADER_STRUCTURE      SchemaHeader;
  BDAT_PCIE_CODE_VERSION            CodeVersion;
  UINT16                            RootPortCount;
  UINT16                            Reserved;
//BDAT_PCIE_PORT_BEST_PRESET        Ports[RootPortCount];  <Dynamically sized
} BDAT_PCIE_EQ_PHASE3_STRUCTURE;

///
/// Structure definitions for BDAT_PCIE_PRESET_SCORE_STRUCTURE
///
typedef struct {
  UINT8                             PhysicalLane;
  UINT8                             Reserved1;
  UINT8                             Reserved2;
  UINT8                             Reserved3;
  INT32                             Score;
} BDAT_PCIE_PRESET_LANE_SCORE;

typedef struct {
  UINT8                             Preset;
  UINT8                             LaneCount;
  UINT8                             Reserved1;
  UINT8                             Reserved2;
  BDAT_PCI_DEVICE                   PhyPort;
  BDAT_PCIE_PRESET_LANE_SCORE       Lanes[BDAT_PCIE_MAX_LINK_WIDTH];
} BDAT_PCIE_PRESET_SCORE;

typedef struct {
  BDAT_SCHEMA_HEADER_STRUCTURE      SchemaHeader;
  UINT16                            ScoreCount;
  UINT16                            Reserved;
//BDAT_PCIE_PRESET_SCORE            Scores[ScoreCount];  <Dynamically sized
} BDAT_PCIE_PRESET_SCORE_STRUCTURE;

///
/// Structure definitions for BDAT_PCIE_PORT_MARGIN_STRUCTURE
///
typedef struct {
  BDAT_PCI_DEVICE                   RootPort;
  EFI_GUID                          MarginType;
  INT32                             HighSideMargin;
  INT32                             LowSideMargin;
} BDAT_PCIE_PORT_MARGIN;

typedef struct {
  BDAT_SCHEMA_HEADER_STRUCTURE      SchemaHeader;
  UINT16                            MarginCount;
  UINT16                            Reserved;
//BDAT_PCIE_PORT_MARGIN             Margins[MarginCount];  <Dynamically sized
} BDAT_PCIE_PORT_MARGIN_STRUCTURE;


///
/// HOB definitions for moving data to DXE
///
typedef struct {
  EFI_HOB_GUID_TYPE                 HobGuidType;
  BDAT_PCIE_EQ_PHASE3_STRUCTURE     EqPhase3Schema;
} BDAT_PCIE_EQ_PHASE3_HOB;

typedef struct {
  EFI_HOB_GUID_TYPE                 HobGuidType;
  BDAT_PCIE_PRESET_SCORE_STRUCTURE  ScoreSchema;
} BDAT_PCIE_PRESET_SCORE_HOB;

typedef struct {
  EFI_HOB_GUID_TYPE                 HobGuidType;
  BDAT_PCIE_PORT_MARGIN_STRUCTURE   PortSchema;
} BDAT_PCIE_PORT_MARGIN_HOB;


#pragma pack (pop)

#endif
