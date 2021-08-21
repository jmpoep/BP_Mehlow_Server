#ifndef _CPU_INFO_DATA
#define _CPU_INFO_DATA 

#include <HeciRegs.h>
#include <Library/CpuLib.h>

typedef struct {
   UINT32 TdpHigh;          //bits 63 .. 32 of Turbo Power current limit MSR
   UINT32 TdpLow;           //bits 31 ..  0 of Turbo Power current limit MSR
   UINT32 PlatformInfoHigh; //bits 63 .. 32 of Platform Info  MSR
   UINT32 PlatformInfoLow;  //bits 31 ..  0 of Platform Info  MSR
} PerCPUInfo;


#define NM_PSTATES_CMD 0x0
#define MKHII_NM_GROUP_ID 0x0

#define CPUID_FULL_FAMILY_MODEL_SANDYBRIDGE  0x000206A0  // Sandy Bridge
#define CPUID_FULL_FAMILY_MODEL_JAKETOWN     0x000206D0  // Jaketown

typedef struct {
   UINT8          GroupID;
   HBM_COMMAND    Command;
   UINT8          Rsvd[3];
   UINT8          Capabilities;  
   UINT8          PStates;
   UINT8          TStates;
   UINT16         MaxPower;
   UINT16         MinPower;
   UINT8          CpuInstalled; 
   UINT8          CpuThreads;
   UINT8          Padding[2];
   PerCPUInfo     CpuInfo;
} CPU_DISCOVERY;


typedef  union {
   UINT32 Data;
   struct {
      UINT16   Max;
      UINT16   Low;
   }r; 
} POWER_LIMIT;

typedef union {
   UINT64 Data;
   struct {
      UINT32 High;
      UINT32 Low;
   }r;
} UINT32_MSR;


EFI_STATUS  GatherCPUInfoData ( 
    EFI_HOB_TYPE_SPS_INFO *PeiInfo, 
    SYSTEM_CONFIGURATION *SetupData  );

UINT8 GetCpuInstalled ( );

UINT8
GetCpuMaxNumberOfLogicalCores (
  VOID
  );

UINT32  GetCpuFusedCores( EFI_BOOT_SERVICES * BS );

#define PLATFORM_INFO   0xCE

#define RATIO_STEP_SIZE 1


#define MAX_PSTATES_RECOMMENDED 0x10

#define MAX_GET_T_STATES            0x10  // This should be (-1) ?
#define NON_MODULATED               0x80;
#define HERMOSA_BEACH_MAX_T_STATES  0x0f
#define MAX_T_STATES_AVAILABLE	    0x0f
#define MIN_T_STATES_AVAILABLE	    0x07
#define TSTATES_NOT_SUPPORTED       0

#define TURBO_POWER_CURRENT_LIMIT 0x1AD
#define PLATFORM_INFO             0xCE


#endif
