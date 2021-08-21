/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2019 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

@file Sps.h

 SPS definitions common for various SPS modules.

**/
#ifndef _SPS_H_
#define _SPS_H_


/*****************************************************************************
 * SPS  HECI registers addresses
 */
#define SPS_REG_MEFS1    0x40   // HECI-1
#define SPS_REG_MEFS2    0x48   // HECI-1
#define SPS_REG_NMFS     0x40   // HECI-2
#define SpsHeciPciReadMefs1()  MeHeciPciReadMefs1 ()
#define SpsHeciPciReadMefs2()  MeHeciPciReadMefs2 ()
#define SpsHeciPciReadNmfs()                                                    \
        PciRead32 (PCI_LIB_ADDRESS (ME_BUS, ME_DEV, ME_FUN_HECI2, HECI_R_HFS))


/******************************************************************************
 * SPS interface on HECI status registers
 */
//
// SPS ME Firmware Status #1 Register
// This register is located in HECI-1 PCI config space at offset 40h.
// Most fields map to HECI_FWS_REGISTER in client ME firmware.
//
typedef union {
  UINT32   DWord;
  struct {
    UINT32 CurrentState      : 4,  // 0:3   Current ME firmware state
           ManufacturingMode : 1,  // 4     Platform is in Manufacturing Mode
           FptBad            : 1,  // 5     Flash Partition Table or factory defaults bad
           OperatingState    : 3,  // 6:8   ME operatiing state
           InitComplete      : 1,  // 9     Set when firmware finished initialization
           FtBupLdFlr        : 1,  // 10    ME is not able to load BRINGUP from recovery code
           UpdateInProgress  : 1,  // 11
           ErrorCode         : 4,  // 12:15 ME is not running because of fatal error
           OperatingMode     : 4,  // 16:19 This field describes the current ME operating mode
           MeResetCounter    : 4,  // 20:23 How many times ME restarted since AC cycle.
           Reserved          : 8;  // 24:31
  } Bits;
} SPS_MEFS1;

//
// SPS_MEFS1::CurrentState Values
// This field describes the current operation state of the firmware.
// The values are shared with client ME firmware.
//
#define MEFS1_CURSTATE_RESET        0  //< ME is in reset, will exit this state within 1 ms
#define MEFS1_CURSTATE_INIT         1  //< ME is initializing, will exit this state within 2 s
#define MEFS1_CURSTATE_RECOVERY     2  //< ME is in recovery mode
#define MEFS1_CURSTATE_DISABLED     4  //< ME functionality has been disabled
#define MEFS1_CURSTATE_NORMAL       5  //< ME is in normal operational state
#define MEFS1_CURSTATE_DISABLE_WAIT 6  //< Not used with SPS
#define MEFS1_CURSTATE_TRANSITION   7  //< ME is transitioning to a new operating State

//
// SPS_MEFS1::OperatingState Values
// This field describes the current operating state of ME.
//
#define MEFS1_OPSTATE_PREBOOT       0  // ME in pre-boot
#define MEFS1_OPSTATE_M0_UMA        1  // ME runs using UMA - not used with SPS
#define MEFS1_OPSTATE_M3            4  // ME runs without UMA, host in Sx
#define MEFS1_OPSTATE_M0            5  // ME runs without UMA, host in S0 -normal state for SPS
#define MEFS1_OPSTATE_BRINGUP       6  // ME in bringup
#define MEFS1_OPSTATE_M0_ERROR      7  // M0 without UMA but with error

//
// SPS_MEFS1::ErrorCode Values
// If set to non zero value the ME firmware has encountered a fatal error and stopped normal operation.
//
#define MEFS1_ERRCODE_NOERROR      0
#define MEFS1_ERRCODE_UNKNOWN      1
#define MEFS1_ERRCODE_DISABLED     2  // Not used with SPS
#define MEFS1_ERRCODE_IMAGE_FAIL   3  // No valid ME firmware image found in ME region

//
// SPS_MEFS1::OperatingMode Values
// This field describes the current operating mode of ME.
//
#define MEFS1_OPMODE_NORMAL        0  // Client firmware is running in ME
#define MEFS1_OPMODE_IGNITION      1  // Ignition firmware is running in ME
#define MEFS1_OPMODE_ALT_DISABLED  2  // ME is disabled
#define MEFS1_OPMODE_TEMP_DISABLE  3  //
#define MEFS1_OPMODE_SECOVR_JMPR   4  // Security Override activated with jumper
#define MEFS1_OPMODE_SECOVR_MSG    5  // Security Override activated with HMRFPO_ENABLE request

//
// SPS ME Firmware Status #2 Register
// This register is located in HECI-1 PCI config space at offset 48h.
//
typedef union {
  UINT32   DWord;
  struct {
    UINT32 CpuReplacedValid       : 1,  // 0
           IccStatus              : 2,  // 1:2   Status of ICC programming
           Reserved1              : 1,  // 3
           CpuReplacedSts         : 1,  // 4
           Reserved2              : 1,  // 5
           MfsFailure             : 1,  // 6     ME detected error in its configuration
           WarmResetRequest       : 1,  // 7     ME informs that host warm reset is necessary
           RecoveryCause          : 3,  // 8:10   If MEFS1.CurrentState says recovery here is reason
           EopStatus              : 1,  // 11    ME notion of EOP status
           MeTargetImageBootFault : 1,  // 12    ME could not boot active image, backup is running
           FirmwareHeartbeat      : 3,  // 13:15 ME heartbeat, changes each second
           ExtendedStatusData     :12,  // 16:23
           PmEvent                : 4,  // 24:27 Power management transition status
           ProgressCode           : 3;  // 28:30 ME firmware progress status
  } Bits;
  UINT32   ul;
  struct {
    UINT32 CpuReplacedValid       : 1,  // 0
           IccStatus              : 2,  // 1:2   Status of ICC programming
           Reserved1              : 1,  // 3
           CpuReplacedSts         : 1,  // 4
           Reserved2              : 1,  // 5
           MfsFailure             : 1,  // 6     ME detected error in its configuration
           WarmRstReqForDF        : 1,  // 7     ME informs that host warm reset is necessary
           RecoveryCause          : 3,  // 8:10   If MEFS1.CurrentState says recovery here is reason
           EopStatus              : 1,  // 11    ME notion of EOP status
           MeTargetImageBootFault : 1,  // 12    ME could not boot active image, backup is running
           FirmwareHeartbeat      : 3,  // 13:15 ME heartbeat, changes each second
           ExtendedStatusData     :12,  // 16:23
           PmEvent                : 4,  // 24:27 Power management transition status
           ProgressCode           : 3;  // 28:30 ME firmware progress status
  } r;
} SPS_MEFS2;

//
// SPS_MEFS2::RecoveryCause Values
// If SPS_MEFS1::CurrentState bits indicate that ME firmware is running in recovery mode
// these values provide the cause of this mode.
//
#define SPS_RCAUSE_RCVJMPR  0  // ME recovery jumper asserted
#define SPS_RCAUSE_MFGJMPR  1  // Security strap override jumper asserted (aka mfg jumper)
#define SPS_RCAUSE_IPMICMD  2  // Recovery forced with IPMI command
#define SPS_RCAUSE_FLASHCFG 3  // Invalid flash configuration (see ME-BIOS spec. for details)
#define SPS_RCAUSE_MEERROR  4  // ME internal error, check SEL
#define SPS_RCAUSE_UMAERROR 5  // UMA memory configuration error returned by HECI DID message

//
// SPS NM Firmware Status Register.
// This register is located in HECI-2 PCI config space at offset 40h.
//
typedef union {
  UINT32   DWord;
  struct {
    UINT32 PerfOptBoot       : 1,  // 0     Power vs performance optimized boot mode
           Cores2Disable     : 7,  // 1:7   Number of processor cores to disable in each processor
           PwrLimiting       : 1,  // 8     Set to 1 if ME is activly limiting power consumption
           SmartClstUVolt    : 1,  // 9     Set to 1 if under-voltage was seen
           SmartClstOCurr    : 1,  // 10    Set to 1 if over-current was seen
           SmartClstOTemp    : 1,  // 11    Set to 1 if over-temperature was seen
           DirectPtuExecution: 1,  // 12    Set to 1 if PTU execution is requested
           Reserved1         :18,  // 13:30
           NmEnabled         : 1;  // 31    Set to 1 if Node Manager is suported in ME
  } Bits;
} SPS_NMFS;

/*****************************************************************************
 * SPS interface using HECI messaging
 */
#define SPS_CLIENTID_BIOS 0x00

#pragma pack (1)
typedef union {
  UINT32   Data;
  struct {
    UINT32 GroupId   : 8,
           Command   : 7,
           IsResponse: 1,
           Reserved  : 8,
           Result    : 8;
  } Fields;
} MKHI_MSG_HEADER;
#pragma pack ()


/*****************************************************************************
 * ME Kernel Host Interface
 *****************************************************************************/
#define SPS_CLIENTID_ME_MKHI 0x07
//
// MKHI protocol supports several groups and several commands in each group
//
#define MKHI_GRP_GEN    0xFF  // Generic group
#define MKHI_GRP_HMRFPO 0x05  // Host ME Region Flash Protection Override
#define MKHI_GRP_NM     0x11  // Node Manager extension to MKHI
#define MKHI_GRP_MS     0x12  // Monitoring Service extension to MKHI
#define MKHI_GRP_DFUSE  0x13  // Dynamic Fusing


/******************************************************************************
 * Generic MKHI messages group
 */
#define MKHI_CMD_GET_MKHI_VERSION 0x01
#define MKHI_CMD_GET_FW_VERSION   0x02
#define MKHI_CMD_END_OF_POST      0x0C

//
// MKHI Get Version message structure
// This request provides MKHI protocol definition version.
//
#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
} MKHI_MSG_GET_MKHI_VERSION_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
  UINT16           Minor;
  UINT16           Major;
} MKHI_MSG_GET_MKHI_VERSION_RSP;
#pragma pack ()


//
// MKHI Get Firmware Version message structure
// Backup firmware version is optional, response may not contain this field
// if ME does not implement dual-image configuration.
//
//
// ME firmware version numbers structure
//
typedef struct {
  UINT16  Minor;
  UINT16  Major;
  UINT16  Build;
  UINT16  Patch;
} MKHI_FW_VERSION;

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
} MKHI_MSG_GET_FW_VERSION_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
  MKHI_FW_VERSION  Act;  // Active operatinal firmware
  MKHI_FW_VERSION  Rcv;  // Recovery firmware
  MKHI_FW_VERSION  Bkp;  // Backup operational firmwar (optional)
} MKHI_MSG_GET_FW_VERSION_RSP;
#pragma pack ()


//
// MKHI End Of POST message structure
//
#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
} MKHI_MSG_END_OF_POST_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
  UINT32           Action;
} MKHI_MSG_END_OF_POST_RSP;
#pragma pack ()

//
// MKHI HECI State Change message structure
//
#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
  UINT64           Nonce;
  UINT8            HeciId;  // HECI ID 1,2,3 - this is not function number
  UINT8            State;   // 1 for hide, or 0 for disable
  UINT16           Reserved;
} MKHI_MSG_HECI_STATE_CHANGE_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER  Mkhi;
  UINT32           Response;
} MKHI_MSG_HECI_STATE_CHANGE_RSP;
#pragma pack ()

/******************************************************************************
 * Host ME Region Flash Protoction Override MKHI messages group
 */
#define HMRFPO_CMD_MERESET   0x00
#define HMRFPO_CMD_ENABLE    0x01
#define HMRFPO_CMD_LOCK      0x02
#define HMRFPO_CMD_GETSTATUS 0x03

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT64          Nonce;
} MKHI_MSG_HMRFPO_ENABLE_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT32          FactoryBase;
  UINT32          FactoryLimit;
  UINT8           Status;
  UINT8           Reserved[3];
} MKHI_MSG_HMRFPO_ENABLE_RSP;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
} MKHI_MSG_HMRFPO_LOCK_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT64          Nonce;
  UINT32          FactoryBase;
  UINT32          FactoryLimit;
  UINT8           Status;
  UINT8           Reserved[3];
} MKHI_MSG_HMRFPO_LOCK_RSP;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
} MKHI_MSG_HMRFPO_GETSTATUS_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT8           Status;
  UINT8           Reserved[3];
} MKHI_MSG_HMRFPO_GETSTATUS_RSP;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT64              Nonce;
} MKHI_MSG_HMRFPO_MERESET_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT8               Status;
  UINT8               Reserved[3];
} MKHI_MSG_HMRFPO_MERESET_RSP;
#pragma pack ()

/******************************************************************************
 * SPS Node Manager messages
 */
#define NM_CMD_HOSTCFG   0x00
#define MAX_ACPI_PSTATES 16

#define NM_STS_SUCCESS     0
#define NM_STS_WRONGMSG    1  // Wrong message format
#define NM_STS_ALTMISS     2  // Altitude missing
#define NM_STS_PSTATESMISS 1  // P-state ratios table missing

#define MKHI_MSG_NM_HOST_CFG_VER                    1

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  union {
    UINT16        Word;
    struct {
      UINT16      TurboEn   : 1,
                  SmiOptim  : 1,
                  PowerMsmt : 1,
                  HwChange  : 1,
                  Reserved0 :10,
                  MsgVer    : 2;
    } Bits;
  } Capabilities;
  UINT8           PStatesNumber;
  UINT8           TStatesNumber;
  UINT16          MaxPower;
  UINT16          MinPower;
  UINT16          ProcNumber;
  UINT16          ProcCoresNumber;
  UINT16          ProcCoresEnabled;
  UINT16          ProcThreadsEnabled;
  UINT64          PlatformInfo;
  INT16           Altitude;
  UINT8           PStatesRatio[MAX_ACPI_PSTATES];
} MKHI_MSG_NM_HOST_CFG_REQ;
#pragma pack ()

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;
  UINT8           Status;
  UINT8           Reserved1;
  UINT16          Reserved2;
} MKHI_MSG_NM_HOST_CFG_RSP;
#pragma pack ()

#define IIO_SLOTS_MAX    3
#define RANK_BITS        4

#define MS_CMD_CUPSCFG   0x00

#pragma pack (1)
typedef struct {
  MKHI_MSG_HEADER Mkhi;            // GroupId = MKHI_GRP_MS
                                   // Command = MS_CMD_CUPSCFG
                                   // IsResponse = FALSE
                                   // Result = 0
  struct {
     UINT32       Ratio      : 6,  // Bit[5:0] = memory speed ratio
                  Reserved0  : 2,  // Bit[7:6] reserved
                  R100Mhz    : 1,  // Bit[8] = memory reference either 133Mhz (=0) or 100Mhz (=1)
                  Reserved1  : 7,  // Bit[15:9] reserved
                  C0Ranks    : 8,  // Bits[23:16] = Rank population bits of channel-0
                  C1Ranks    : 8;  // Bits[31:24] = Rank population bits of channel-1
  } MemInfo;
  UINT32          PortsNum   : 8;
  UINT32          Reserved1  : 24;
  struct {
    UINT32        Fun        : 8,  // Bit[7:0] = Function
                  Dev        : 8,  // Bit[15:8] = Device
                  Bus        : 8,  // Bit[23:16] = Bus
                  Reserved0  : 8,  // Bit[31:24] = reserved
                  LineRate   : 4,  // Bit[3:0] = LineRatePllMultiplier = 1 if Gen-1, = 2 if gen-2, = 3 if gen-3
                  LaneWidth  : 6,  // Bit[9:4] = LaneWidth  = 1 if x1, = 2 if x2, = 4 if x4, = 8 if x8, = 16 if x16
                  Reserved1  : 3,  // Bit[12:10] = reserved
                  LinkActive : 1,  // Bit[13] = LinkActive = 1 if lane is active, = 0 if not
                  Reserved2  : 18; // Bit[31:14] = reserved
  } PortInfo[IIO_SLOTS_MAX];
} MKHI_MSG_MS_CUPS_CFG_REQ;

typedef struct {
  MKHI_MSG_HEADER  Mkhi;           // GroupId = MKHI_GRP_MS
                                   // Command = MS_CMD_CUPSCFG
                                   // IsResponse = TRUE
                                   // Result =0 for success, =1 for failure
} MKHI_MSG_MS_CUPS_CFG_RSP;
#pragma pack ()


/*****************************************************************************
 * SPS messages for PMBus over HECI communication
 *****************************************************************************/

#define SPS_CMD_PMBUS_CMD_SEND_RAW_REQ        0xA
#define SPS_CMD_PMBUS_CMD_SEND_RAW_RSP        (0x80 | SPS_CMD_PMBUS_CMD_SEND_RAW_REQ)

#define SPS_CMD_PMBUS_CMD_REQ_PMBUS_DATA_SIZE 21
#define SPS_CMD_PMBUS_CMD_RSP_PMBUS_DATA_SIZE 26

#define PMBUS_CMD_STATUS_WORD 0x79

#define PMBUS_CMD_WRITE_LENGTH_READ_WORD  1
#define PMBUS_CMD_READ_LENGTH_READ_WORD   2

#define PMBUS_CMD_RESULT_SUCCESS                    0x00
#define PMBUS_CMD_RESULT_NO_DEVICE_TIMEOUT          0x80
#define PMBUS_CMD_RESULT_NOT_SERVICED               0x81
#define PMBUS_CMD_RESULT_ILLEGAL_SMBUS_ADDR_CMD     0x82
#define PMBUS_CMD_RESULT_ILLEGAL_SMBUS_ADDR_TARGET  0xA1
#define PMBUS_CMD_RESULT_PEC_ERROR                  0xA2
#define PMBUS_CMD_RESULT_UNSUPPORTED_WRITE_LENGTH   0xA5
#define PMBUS_CMD_RESULT_FRAME_LENGTH_NOT_SUPPORTED 0xA7
#define PMBUS_CMD_RESULT_MUX_COMMUNICATION_PROBLEM  0xA9
#define PMBUS_CMD_RESULT_SMBUS_TIMEOUT              0xAA

typedef enum {
  PMBUS_CMD_SEND_BYTE = 0,
  PMBUS_CMD_READ_BYTE,
  PMBUS_CMD_WRITE_BYTE,
  PMBUS_CMD_READ_WORD,
  PMBUS_CMD_WRITE_WORD,
  PMBUS_CMD_BLOCK_READ,
  PMBUS_CMD_BLOCK_WRITE,
  PMBUS_CMD_BLOCK_WRITE_READ_PROC_CALL
} PMBUS_CMD_SMBUS_TRANSACTION_TYPE;

#define PMBUS_STATUS_WORD_NO_ERRORS 0

#pragma pack (1)
typedef union {
  UINT16 Content;
  struct {
    UINT16 Unknown      :1,
           Other        :1,
           Fans         :1,
           PgStatus     :1,
           MfrSpecific  :1,
           Input        :1,
           IoutPout     :1,
           Vout         :1,
           NoneOfAbove  :1,
           Cml          :1,
           Temperature  :1,
           VinUvFault   :1,
           IoutOcFault  :1,
           VoutOvFault  :1,
           Off          :1,
           Busy         :1;
  } Bits;
} PMBUS_STATUS_WORD_MESSAGE_CONTENT;

typedef union {
  UINT8 Data;
  struct {
    UINT8 Reserved                      :1,
          SmbusMessageTransactionType   :3,
          DeviceAddressFormat           :2,
          DoNotReportPecInCc            :1,
          EnablePec                     :1;
  } Flags;
} PMBUS_CMD_FLAGS;

typedef union {
  UINT8 Data;
  struct {
    UINT8 Reserved      :1,
          SmbusAddress  :7;
  } Fields;
} PMBUS_CMD_PSU_ADDRESS;

typedef union {
  UINT8 Data;
  struct {
    UINT8 MuxMgpioIdx :6,
          Reserved    :2;
  } Fields;
} PMBUS_CMD_MUX_ADDRESS;

typedef struct {
  PMBUS_CMD_FLAGS       Flags;
  PMBUS_CMD_PSU_ADDRESS PsuAddress;
  PMBUS_CMD_MUX_ADDRESS MuxAddress;
  UINT8                 Reserved;
  UINT8                 WriteLength;
  UINT8                 ReadLength;
} PMBUS_CMD_REQUEST_PARAMETERS;

typedef struct {
  UINT8                 RequestData[SPS_CMD_PMBUS_CMD_REQ_PMBUS_DATA_SIZE];
} PMBUS_CMD_REQUEST_DATA;

typedef struct {
  UINT8                 Result;
} PMBUS_CMD_RESPONSE_PARAMETERS;

typedef struct {
  UINT8                 ResponseData[SPS_CMD_PMBUS_CMD_RSP_PMBUS_DATA_SIZE];
} PMBUS_CMD_RESPONSE_DATA;

typedef struct {
  UINT8                     Command;
  PMBUS_CMD_REQUEST_PARAMETERS    PmbusParameters;
  PMBUS_CMD_REQUEST_DATA          PmbusCommandData;
} SPS_MSG_PMBUS_CMD_SEND_RAW_REQ;

typedef struct {
    UINT8                   Command;
    PMBUS_CMD_RESPONSE_PARAMETERS PmbusParameters;
    PMBUS_CMD_RESPONSE_DATA       PmbusCommandData;
} SPS_MSG_PMBUS_CMD_SEND_RAW_RSP;
#pragma pack ()

#define SpsMsgPmbusCmdSendRawGetReqLength(WriteLength) \
  (sizeof(SPS_MSG_PMBUS_CMD_SEND_RAW_REQ) - sizeof(PMBUS_CMD_REQUEST_DATA) + (sizeof(UINT8) * WriteLength))

#define SpsMsgPmbusCmdSendRawGetRspLength(ReadLength) \
  (sizeof(SPS_MSG_PMBUS_CMD_SEND_RAW_RSP) - sizeof(PMBUS_CMD_RESPONSE_DATA) + (sizeof(UINT8) * ReadLength))

#define SpsMsgPmbusCmdSendRawNoPmbusDataRspLength() \
  (sizeof(SPS_MSG_PMBUS_CMD_SEND_RAW_RSP) - sizeof(PMBUS_CMD_RESPONSE_DATA))

#define SpsMsgPmbusCmdSendRawRspLengthVerify(ResponseLengthReceived, ResponseLengthExpected) \
  ((ResponseLengthReceived == SpsMsgPmbusCmdSendRawNoPmbusDataRspLength()) || (ResponseLengthReceived == ResponseLengthExpected))


/*****************************************************************************
 * SPS messages for ME-BIOS interface definition version
 *****************************************************************************/
#define SPS_CLIENTID_ME_SPS   0x20

#define SPS_CMD_GET_MEBIOS_INTERFACE_REQ 1
#define SPS_CMD_GET_MEBIOS_INTERFACE_RSP (0x80 | SPS_CMD_GET_MEBIOS_INTERFACE_REQ)

#pragma pack (1)
typedef struct {
  UINT8          Command;
} SPS_MSG_GET_MEBIOS_INTERFACE_REQ;

typedef struct {
    UINT8        Command;
    UINT8        VerMajor;
    UINT8        VerMinor;
    UINT32       FeatureSet;
    UINT32       FeatureSet2;
} SPS_MSG_GET_MEBIOS_INTERFACE_RSP;
#pragma pack ()

//
// BIOS must verify the SPS ME-BIOS Interface Specification version to make
// sure BIOS and ME firmware will talk the sam laguage.
// There can be different versions for recovery and operational ME firmware
// after update of operational firmware.
//
#define SPS_MEBIOS_OPR_VERSION_MIN ((0 << 8) | 0)
#define SPS_MEBIOS_OPR_VERSION_MAX ((0xFF << 8) | 0xFF)
#define SPS_MEBIOS_RCV_VERSION_MIN ((0 << 8) | 0)
#define SPS_MEBIOS_RCV_VERSION_MAX ((0xFF << 8) | 0xFF)
#define SpsMeBiosOprVersionVerify(Major, Minor)                      \
        ((((Major) << 8) | (Minor)) >= SPS_MEBIOS_OPR_VERSION_MIN && \
         (((Major) << 8) | (Minor)) <= SPS_MEBIOS_OPR_VERSION_MAX)
#define SpsMeBiosRcvVersionVerify(Major, Minor)                      \
        ((((Major) << 8) | (Minor)) >= SPS_MEBIOS_RCV_VERSION_MIN && \
         (((Major) << 8) | (Minor)) <= SPS_MEBIOS_RCV_VERSION_MAX)


/*****************************************************************************
* MCTP Bus Owner Proxy Configuration
******************************************************************************/
#define SPS_CLIENTID_ME_MCTP   0x21

#define MCPT_CMD_SET_BUS_OWNER_REQ   0x01
#define MCPT_CMD_SET_BUS_OWNER_RSP (0x80 | MCPT_CMD_SET_BUS_OWNER_REQ)
#define MCPT_CMD_SET_BUS_OWNER_RSP_SUCCESS (0x00)

#pragma pack (1)
typedef struct {
  UINT8           Command;
  UINT8           Reserved0[3];
  UINT16          PCIeAddress;
  UINT8           Location;
  UINT8           Reserved1;
} MCTP_SET_BUS_OWNER_REQ;

typedef struct {
  UINT8           Command;
  UINT8           Reserved0[2];
  UINT8           Result;
} MCTP_SET_BUS_OWNER_RSP;
#pragma pack ()

#endif // _SPS_H_
