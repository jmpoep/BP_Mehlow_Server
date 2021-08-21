/** @file
  Copies the memory related timing and configuration information into the
  Compatible BIOS data (BDAT) table.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2010 - 2017 Intel Corporation.

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
#ifndef _MrcRmtData_h_
#define _MrcRmtData_h_

#include "MrcTypes.h"

#define VDD_1_350             1350                      ///< VDD in millivolts
#define VDD_1_500             1500                      ///< VDD in millivolts
#define PI_STEP_BASE          2048                      ///< Magic number from spec
#define PI_STEP_INTERVAL      128                       ///< tCK is split into this amount of intervals
#define PI_STEP               ((PI_STEP_BASE) / (PI_STEP_INTERVAL))
#define VREF_STEP_BASE        100                       ///< Magic number from spec
#define TX_VREF_STEP          7800                      ///< TX Vref step in microvolts
#define TX_VREF(VDD)          (((TX_VREF_STEP) * (VREF_STEP_BASE)) / (VDD)) ///< VDD passed in is in millivolts
#define RX_VREF_STEP          8000                      ///< TX Vref step in microvolts
#define RX_VREF(VDD)          (((RX_VREF_STEP) * (VREF_STEP_BASE)) / (VDD)) ///< VDD passed in is in millivolts
#define CA_VREF_STEP          8000                      ///< TX Vref step in microvolts
#define CA_VREF(VDD)          (((CA_VREF_STEP) * (VREF_STEP_BASE)) / (VDD)) ///< VDD passed in is in millivolts

#define MAX_SPD_RMT           512                       ///< The maximum amount of data, in bytes, in an SPD structure.
#define RMT_PRIMARY_VERSION   4                         ///< The BDAT structure that is currently supported.
#define RMT_SECONDARY_VERSION 0                         ///< The BDAT structure that is currently supported.
#define MAX_MODE_REGISTER     7                         ///< Number of mode registers
#define MAX_DRAM_DEVICE       9                         ///< Maximum number of memory devices
#define MAX_2D_EYE_TYPE       2                         ///< Maximum number of supported Margin 2D Eye Types
#define MAX_2D_EYE_OFFSETS    7                         ///< Number of 2D Eye Offsets

//
// Warning: Bdat4.h has its own copy of this #define
// make sure to change it in both places
//
#define MAX_SCHEMA_LIST_LENGTH (10)


#ifdef BDAT_SUPPORT
/*
  BSSA result Memory Schema GUID
  {8F4E928-0F5F-46D4-8410-479FDA279DB6}
*/
extern EFI_GUID gSsaBiosResultsGuid;
/*
  RMT Results Metadata GUID
  {02CB1552-D659-4232-B51F-CAB1E11FCA87}
*/
extern EFI_GUID gRmtResultMetadataGuid;
/*
  RMT Results Columns GUID
  {0E60A1EB-331F-42A1-9DE7-453E84761154}
*/
extern EFI_GUID gRmtResultColumnsGuid;

/*
Margin2D Results Metadata GUID
{48265582-8E49-4AC7-AA06-E1B9A74C9716}
*/
extern EFI_GUID gMargin2DResultMetadataGuid;
/*
Margin2D Results Columns GUID
{91A449EC-8A4A-4736-AD71-A3F6F6D752D9}
*/
extern EFI_GUID gMargin2DResultColumnsGuid;

#endif
/*
 GUID for Schema List HOB
 This is private GUID used by MemoryInit internally.
 {3047C2AC-5E8E-4C55-A1CB-EAAD0A88861B}
*/
extern EFI_GUID gMrcSchemaListHobGuid;

#pragma pack(push, 1)


///
/// SSA results buffer header.
///
typedef struct {
  UINT32  Revision;
  BOOLEAN TransferMode;
  struct {
    UINT32   Reserved;
    UINT32   MetadataSize;
    EFI_GUID MetadataType;
  } MdBlock;
  struct {
    UINT32   Reserved;
    EFI_GUID ResultType;
    UINT32   ResultElementSize;
    INT32    ResultCapacity;
    INT32    ResultElementCount;
  } RsBlock;
} RESULTS_DATA_HDR;

// start auto-generated by the BSSA CCK sourced from the result xml files.
typedef enum {
  DisableScrambler = 0,
  EnableScrambler = 1,
  DontTouchScrambler = 2,
  SCRAMBLER_OVERRIDE_MODE_DELIM = MRC_INT32_MAX
} SCRAMBLER_OVERRIDE_MODE;

typedef struct _RMT_RESULT_METADATA {
  BOOLEAN EnableCtlAllMargin;
  UINT16 SinglesBurstLength;
  UINT32 SinglesLoopCount;
  UINT16 TurnaroundsBurstLength;
  UINT32 TurnaroundsLoopCount;
  SCRAMBLER_OVERRIDE_MODE ScramblerOverrideMode;
  UINT8 PiStepUnit[2];
  UINT16 RxVrefStepUnit[2];
  UINT16 TxVrefStepUnit[2][2];
  UINT16 CmdVrefStepUnit[2][2];
  UINT8 MajorVer;
  UINT8 MinorVer;
  UINT8 RevVer;
  UINT32 BuildVer;
  UINT16 ResultEleCount;
} RMT_RESULT_METADATA;

typedef enum {
  RankResultType0 = 0,
  RankResultType1 = 1,
  RankResultType2 = 2,
  RankResultType3 = 3,
  ByteResultType = 4,
  LaneResultType = 5,
  TurnaroundResultType = 6,
  ParamLimits0ResultType = 7,
  ParamLimits1ResultType = 8,
  ParamLimits2ResultType = 9,
  ParamLimits3ResultType = 10,
  ResultTypeMax = 31,
  RMT_RESULT_TYPE_DELIM = MRC_INT32_MAX
} RMT_RESULT_TYPE;

typedef struct _RMT_RESULT_ROW_HEADER {
  UINT32 ResultType : 5;
  UINT32 Socket : 3;
  UINT32 Controller : 2;
  UINT32 Channel : 3;
  UINT32 DimmA : 1;
  UINT32 RankA : 3;
  UINT32 DimmB : 1;
  UINT32 RankB : 3;
  UINT32 Lane : 8;
  UINT32 IoLevel : 1;
  UINT32 Reserved : 2;
} RMT_RESULT_ROW_HEADER;

typedef struct _RMT_RESULT_COLUMNS {
  RMT_RESULT_ROW_HEADER Header;
  UINT8 Margin[4][2];
} RMT_RESULT_COLUMNS;

typedef struct _MARGIN_2D_RESULT_METADATA {
  UINT16 BurstLength;
  UINT32 LoopCount;
  UINT8 MajorVer;
  UINT8 MinorVer;
  UINT8 RevVer;
  UINT32 BuildVer;
  UINT16 ResultEleCount;
} MARGIN_2D_RESULT_METADATA;

typedef enum _MARGIN_2D_EYE_TYPE{
  RxDqsRxVrefEyeType = 0,
  TxDqTxVrefEyeType = 1,
  //CmdCmdVrefEyeType = 2,
  MARGIN_2D_EYE_TYPE_DELIM = MRC_INT32_MAX
} MARGIN_2D_EYE_TYPE;

typedef enum _MARGIN_PARAM_TYPE {
  TimingMarginParamType = 0,
  VotageMarginParamType = 1,
  MARGIN_PARAM_TYPE_DELIM = MRC_INT32_MAX
} MARGIN_PARAM_TYPE;

typedef struct _MARGIN_2D_RESULT_COLUMNS {
  UINT8 Controller;
  UINT8 Channel;
  UINT8 Dimm;
  UINT8 Rank;
  UINT8 Lane;
  UINT8 EyeType;
  UINT8 OuterMarginParamType;
  INT16 OuterOffset;
  UINT8 LeftMargin;
  UINT8 RightMargin;
} MARGIN_2D_RESULT_COLUMNS;

// end of auto-generated by the BSSA CCK sourced from the result xml files.

typedef struct _BASE_RMT_RESULT {
  RESULTS_DATA_HDR              ResultsHeader;
  RMT_RESULT_METADATA           Metadata;
  RMT_RESULT_COLUMNS            Rows[1];
} BASE_RMT_RESULT;

typedef struct _BASE_MARGIN_2D_RESULT {
  RESULTS_DATA_HDR              ResultsHeader;
  MARGIN_2D_RESULT_METADATA   Metadata;
  MARGIN_2D_RESULT_COLUMNS    Rows[1];
} BASE_MARGIN_2D_RESULT;


typedef enum {
  RankMarginToolType = 0,
  RankMarginToolPerBitType = 1,
  Margin2DType = 2,
  MRC_BDAT_SCHEMA_TYPE_DELIMITER = MRC_INT32_MAX
} MRC_BDAT_SCHEMA_TYPE;

typedef struct {
  UINT32                      Data1;
  UINT16                      Data2;
  UINT16                      Data3;
  UINT8                       Data4[8];
} BDAT_EFI_GUID;

typedef struct {
  UINT16  HobType;
  UINT16  HobLength;
  UINT32  Reserved;
} BDAT_HOB_GENERIC_HEADER;

typedef struct {
  BDAT_HOB_GENERIC_HEADER  Header;
  BDAT_EFI_GUID            Name;
  ///
  /// Guid specific data goes here
  ///
} BDAT_HOB_GUID_TYPE;

typedef struct {
  BDAT_EFI_GUID               SchemaId;                         ///< The GUID uniquely identifies the format of the data contained within the structure.
  UINT32                      DataSize;                         ///< The total size of the memory block, including both the header as well as the schema specific data.
  UINT16                      Crc16;                            ///< Crc16 is computed in the same manner as the field in the BDAT_HEADER_STRUCTURE.
} MRC_BDAT_SCHEMA_HEADER_STRUCTURE;

typedef struct {
  MRC_BDAT_SCHEMA_HEADER_STRUCTURE SchemaHeader;                ///< The schema header.
  BASE_RMT_RESULT          RMT_RESULTS_WITH_META_COLUMNS;
} BDAT_MEMORY_DATA_STRUCTURE;

typedef struct {
  BDAT_HOB_GUID_TYPE          HobGuidType;
  BDAT_MEMORY_DATA_STRUCTURE  MemorySchema;
} BDAT_MEMORY_DATA_HOB;

#pragma pack (pop)

typedef struct {
  BDAT_HOB_GUID_TYPE          HobGuidType;
  UINT16                      SchemaHobCount;
  UINT16                      Reserved;
  BDAT_EFI_GUID               SchemaHobGuids[MAX_SCHEMA_LIST_LENGTH];
} MRC_BDAT_SCHEMA_LIST_HOB;

#endif //_MrcRmtData_h_
