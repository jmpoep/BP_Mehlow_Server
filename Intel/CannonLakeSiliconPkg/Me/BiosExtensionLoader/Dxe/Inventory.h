/** @file
  Include for AMT Bios Extentions Loader Inventory Functions

@copyright
  INTEL CONFIDENTIAL
  Copyright 2005 - 2016 Intel Corporation.

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
#ifndef _INVENTORY_H_
#define _INVENTORY_H_

#include <Protocol/IdeControllerInit.h>
#include <Protocol/DiskInfo.h>
#include <Protocol/NvmExpressPassthru.h>
#include <Protocol/AtaPassThru.h>

///
/// Common part of the PCI configuration space header for devices, P2P bridges,
/// and cardbus bridges
///
typedef struct {
  UINT16  VendorId;
  UINT16  DeviceId;

  UINT16  Command;
  UINT16  Status;

  UINT8   RevisionId;
  UINT8   ClassCode[3];

  UINT8   CacheLineSize;
  UINT8   PrimaryLatencyTimer;
  UINT8   HeaderType;
  UINT8   BIST;

} PCI_COMMON_HEADER;

///
/// PCI configuration space header for devices(after the common part)
///
typedef struct {
  UINT32  Bar[6];           ///< Base Address Registers
  UINT32  CardBusCISPtr;    ///< CardBus CIS Pointer
  UINT16  SubVendorId;      ///< Subsystem Vendor ID
  UINT16  SubSystemId;      ///< Subsystem ID
  UINT32  ROMBar;           ///< Expansion ROM Base Address
  UINT8   CapabilitiesPtr;  ///< Capabilities Pointer
  UINT8   Reserved[3];

  UINT32  Reserved1;

  UINT8   InterruptLine;    ///< Interrupt Line
  UINT8   InterruptPin;     ///< Interrupt Pin
  UINT8   MinGnt;           ///< Min_Gnt
  UINT8   MaxLat;           ///< Max_Lat
} PCI_DEVICE_HEADER;

///
/// PCI configuration space header for pci-to-pci bridges(after the common part)
///
typedef struct {
  UINT32  Bar[2];                 ///< Base Address Registers
  UINT8   PrimaryBus;             ///< Primary Bus Number
  UINT8   SecondaryBus;           ///< Secondary Bus Number
  UINT8   SubordinateBus;         ///< Subordinate Bus Number
  UINT8   SecondaryLatencyTimer;  ///< Secondary Latency Timer
  UINT8   IoBase;                 ///< I/O Base
  UINT8   IoLimit;                ///< I/O Limit
  UINT16  SecondaryStatus;        ///< Secondary Status
  UINT16  MemoryBase;             ///< Memory Base
  UINT16  MemoryLimit;            ///< Memory Limit
  UINT16  PrefetchableMemBase;    ///< Pre-fetchable Memory Base
  UINT16  PrefetchableMemLimit;   ///< Pre-fetchable Memory Limit
  UINT32  PrefetchableBaseUpper;  ///< Pre-fetchable Base Upper 32 bits
  UINT32  PrefetchableLimitUpper; ///< Pre-fetchable Limit Upper 32 bits
  UINT16  IoBaseUpper;            ///< I/O Base Upper 16 bits
  UINT16  IoLimitUpper;           ///< I/O Limit Upper 16 bits
  UINT8   CapabilitiesPtr;        ///< Capabilities Pointer
  UINT8   Reserved[3];

  UINT32  ROMBar;                 ///< Expansion ROM Base Address
  UINT8   InterruptLine;          ///< Interrupt Line
  UINT8   InterruptPin;           ///< Interrupt Pin
  UINT16  BridgeControl;          ///< Bridge Control
} PCI_BRIDGE_HEADER;

///
/// PCI configuration space header for cardbus bridges(after the common part)
///
typedef struct {
  UINT32  CardBusSocketReg;       ///< Cardus Socket/ExCA Base Address Register
  UINT16  Reserved;
  UINT16  SecondaryStatus;        ///< Secondary Status
  UINT8   PciBusNumber;           ///< PCI Bus Number
  UINT8   CardBusBusNumber;       ///< CardBus Bus Number
  UINT8   SubordinateBusNumber;   ///< Subordinate Bus Number
  UINT8   CardBusLatencyTimer;    ///< CardBus Latency Timer
  UINT32  MemoryBase0;            ///< Memory Base Register 0
  UINT32  MemoryLimit0;           ///< Memory Limit Register 0
  UINT32  MemoryBase1;
  UINT32  MemoryLimit1;
  UINT32  IoBase0;
  UINT32  IoLimit0;               ///< I/O Base Register 0
  UINT32  IoBase1;                ///< I/O Limit Register 0
  UINT32  IoLimit1;
  UINT8   InterruptLine;          ///< Interrupt Line
  UINT8   InterruptPin;           ///< Interrupt Pin
  UINT16  BridgeControl;          ///< Bridge Control
} PCI_CARDBUS_HEADER;

typedef struct {
  PCI_COMMON_HEADER Common;
  union {
    PCI_DEVICE_HEADER   Device;
    PCI_BRIDGE_HEADER   Bridge;
    PCI_CARDBUS_HEADER  CardBus;
  } NonCommon;
  UINT32  Data[48];
} PCI_CONFIG_SPACE;


#define NVME_GENERIC_TIMEOUT            5000000   ///< us
#define NVME_ADMIN_IDENTIFY_CMD         0x06
#define NVME_CONTROLLER_ID              0

#define NVME_ALL_NAMESPACES             0xFFFFFFFF

#define NVME_IDENTIFY_NAMESPACE_STRUCT  0
#define NVME_IDENTIFY_CONTROLLER_STRUCT 1


typedef struct {
  UINT16 Ms;                /* Metadata Size */
  UINT8  Lbads;             /* LBA Data Size */
  UINT8  Rp:2;              /* Relative Performance */
    #define LBAF_RP_BEST      00b
    #define LBAF_RP_BETTER    01b
    #define LBAF_RP_GOOD      10b
    #define LBAF_RP_DEGRADED  11b
  UINT8  Rsvd1:6;           /* Reserved as of Nvm Express 1.1 Spec */
} NVME_LBAFORMAT;

//
// Identify Namespace Data
//
typedef struct {
  //
  // NVM Command Set Specific
  //
  UINT64 Nsze;                /* Namespace Size (total number of blocks in formatted namespace) */
  UINT64 Ncap;                /* Namespace Capacity (max number of logical blocks) */
  UINT64 Nuse;                /* Namespace Utilization */
  UINT8  Nsfeat;              /* Namespace Features */
  UINT8  Nlbaf;               /* Number of LBA Formats */
  UINT8  Flbas;               /* Formatted LBA size */
  UINT8  Mc;                  /* Metadata Capabilities */
  UINT8  Dpc;                 /* End-to-end Data Protection capabilities */
  UINT8  Dps;                 /* End-to-end Data Protection Type Settings */
  UINT8  Nmic;                /* Namespace Multi-path I/O and Namespace Sharing Capabilities */
  UINT8  Rescap;              /* Reservation Capabilities */
  UINT8  Rsvd1[88];           /* Reserved as of Nvm Express 1.1 Spec */
  UINT64 Eui64;               /* IEEE Extended Unique Identifier */
  //
  // LBA Format
  //
  NVME_LBAFORMAT LbaFormat[16];

  UINT8 Rsvd2[192];           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8 VendorData[3712];     /* Vendor specific data */
} NVME_ADMIN_NAMESPACE_DATA;

typedef struct {
  UINT16 Mp;                /* Maximum Power */
  UINT8  Rsvd1;             /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Mps:1;             /* Max Power Scale */
  UINT8  Nops:1;            /* Non-Operational State */
  UINT8  Rsvd2:6;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Enlat;             /* Entry Latency */
  UINT32 Exlat;             /* Exit Latency */
  UINT8  Rrt:5;             /* Relative Read Throughput */
  UINT8  Rsvd3:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rrl:5;             /* Relative Read Leatency */
  UINT8  Rsvd4:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwt:5;             /* Relative Write Throughput */
  UINT8  Rsvd5:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rwl:5;             /* Relative Write Leatency */
  UINT8  Rsvd6:3;           /* Reserved as of Nvm Express 1.1 Spec */
  UINT8  Rsvd7[16];         /* Reserved as of Nvm Express 1.1 Spec */
} NVME_PSDESCRIPTOR;

//
//  Identify Controller Data
//
typedef struct {
  //
  // Controller Capabilities and Features 0-255
  //
  UINT16 Vid;                 /* PCI Vendor ID */
  UINT16 Ssvid;               /* PCI sub-system vendor ID */
  UINT8  Sn[20];              /* Product serial number */

  UINT8  Mn[40];              /* Proeduct model number */
  UINT8  Fr[8];               /* Firmware Revision */
  UINT8  Rab;                 /* Recommended Arbitration Burst */
  UINT8  Ieee_oui[3];         /* Organization Unique Identifier */
  UINT8  Cmic;                /* Multi-interface Capabilities */
  UINT8  Mdts;                /* Maximum Data Transfer Size */
  UINT8  Cntlid[2];           /* Controller ID */
  UINT8  Rsvd1[176];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Admin Command Set Attributes
  //
  UINT16 Oacs;                /* Optional Admin Command Support */
    #define NAMESPACE_MANAGEMENT_SUPPORTED  BIT3
    #define FW_DOWNLOAD_ACTIVATE_SUPPORTED  BIT2
    #define FORMAT_NVM_SUPPORTED            BIT1
    #define SECURITY_SEND_RECEIVE_SUPPORTED BIT0
  UINT8  Acl;                 /* Abort Command Limit */
  UINT8  Aerl;                /* Async Event Request Limit */
  UINT8  Frmw;                /* Firmware updates */
  UINT8  Lpa;                 /* Log Page Attributes */
  UINT8  Elpe;                /* Error Log Page Entries */
  UINT8  Npss;                /* Number of Power States Support */
  UINT8  Avscc;               /* Admin Vendor Specific Command Configuration */
  UINT8  Apsta;               /* Autonomous Power State Transition Attributes */
  UINT8  Rsvd2[246];          /* Reserved as of Nvm Express 1.1 Spec */

  //
  // NVM Command Set Attributes
  //
  UINT8  Sqes;                /* Submission Queue Entry Size */
  UINT8  Cqes;                /* Completion Queue Entry Size */
  UINT16 Rsvd3;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Nn;                  /* Number of Namespaces */
  UINT16 Oncs;                /* Optional NVM Command Support */
  UINT16 Fuses;               /* Fused Operation Support */
  UINT8  Fna;                 /* Format NVM Attributes */
  UINT8  Vwc;                 /* Volatile Write Cache */
  UINT16 Awun;                /* Atomic Write Unit Normal */
  UINT16 Awupf;               /* Atomic Write Unit Power Fail */
  UINT8  Nvscc;               /* NVM Vendor Specific Command Configuration */
  UINT8  Rsvd4;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT16 Acwu;                /* Atomic Compare & Write Unit */
  UINT16 Rsvd5;               /* Reserved as of Nvm Express 1.1 Spec */
  UINT32 Sgls;                /* SGL Support  */
  UINT8  Rsvd6[164];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // I/O Command set Attributes
  //
  UINT8 Rsvd7[1344];          /* Reserved as of Nvm Express 1.1 Spec */
  //
  // Power State Descriptors
  //
  NVME_PSDESCRIPTOR PsDescriptor[32];

  UINT8  VendorData[1024];    /* Vendor specific data */
} NVME_ADMIN_CONTROLLER_DATA;

/**
  AMT only need to know removable PCI device information.


  @retval EFI_SUCCESS             mAmtPciFru will be update.
  @retval EFI_OUT_OF_RESOURCES    System on-board device list is larger than
                                  MAX_FILTER_OUT_DEVICE_NUMBER supported.
**/
EFI_STATUS
BuildPciFru (
  VOID
  );

/**
  Collect all media devices. First look for Pass Thru Protocols for ATA and NVMe drives.
  Check device paths afterwards and look for the messaging device paths.
**/
VOID
EFIAPI
BuildMediaList (
  VOID
  );
#endif // _INVENTORY_H_
