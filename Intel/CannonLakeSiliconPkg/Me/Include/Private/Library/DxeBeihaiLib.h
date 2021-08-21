/** @file
  Header file for Beihai Library.

@copyright
  INTEL CONFIDENTIAL
  Copyright 2015 - 2016 Intel Corporation.

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

#ifndef __DXE_BEIHAI_LIB_H__
#define __DXE_BEIHAI_LIB_H__

#include <Protocol/JhiProtocol.h>
#include <Private/JhiInternal.h>

#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif

//
// bh_shared_errcode
//
typedef int BH_RET; //BH_RET should have values defined following

//Following are error code number(integer value) across Beihai host and firmware.

#define BH_SUCCESS                              0x0

/////BHP specific error code section:           0x000
#define BPE_NOT_INIT                            0x001
#define BPE_SERVICE_UNAVAILABLE                 0x002
#define BPE_INTERNAL_ERROR                      0x003
#define BPE_COMMS_ERROR                         0x004
#define BPE_OUT_OF_MEMORY                       0x005
#define BPE_INVALID_PARAMS                      0x006
#define BPE_MESSAGE_TOO_SHORT                   0x007
#define BPE_MESSAGE_ILLEGAL                     0x008
#define BPE_NO_CONNECTION_TO_FIRMWARE           0x009
#define BPE_NOT_IMPLEMENT                       0x00A
#define BPE_OUT_OF_RESOURCE                     0x00B
#define BPE_INITIALIZED_ALREADY                 0x00C
#define BPE_CONNECT_FAILED                      0x00D
//////////////////////////////////////////////////

//General error code section for Beihai on FW:  0x100
#define BHE_OUT_OF_MEMORY                       0x101
/* Bad parameters to native */
#define BHE_BAD_PARAMETER                       0x102
#define BHE_INSUFFICIENT_BUFFER                 0x103
#define BHE_MUTEX_INIT_FAIL                     0x104
#define BHE_COND_INIT_FAIL                      0x105
/* Watchdog time out */
#define BHE_WD_TIMEOUT                          0x106
#define BHE_FAILED                              0x107
#define BHE_INVALID_HANDLE                      0x108
/* IPC error code */
#define BHE_IPC_ERR_DEFAULT                     0x109
#define BHE_IPC_ERR_PLATFORM                    0x10A
#define BHE_IPC_SRV_INIT_FAIL                   0x10B
//////////////////////////////////////////////////

//VM communication error code section:          0x200
#define BHE_MAILBOX_NOT_FOUND                   0x201
#define BHE_APPLET_CRASHED                      BHE_MAILBOX_NOT_FOUND
#define BHE_MSG_QUEUE_IS_FULL                   0x202
/* Mailbox is denied by firewall */
#define BHE_MAILBOX_DENIED                      0x203
//////////////////////////////////////////////////

//VM InternalAppletCommunication error          0x240
#define BHE_IAC_INTERNAL_SESSION_NUM_EXCEED     0x241
#define BHE_IAC_CLIENT_SLOT_FULL                0x242
#define BHE_IAC_SERVICETA_EXITED                0x243
#define BHE_IAC_EXIST_INTERNAL_SESSION          0x244
#define BHE_IAC_SERVICETA_UNCAUGHT_EXCEPTION    0x245
#define BHE_IAC_SERVICE_SESSION_NOT_FOUND       0x246
#define BHE_IAC_SERVICE_HOST_SESSION_NUM_EXCEED 0x247
//////////////////////////////////////////////////

//Firmware thread/mutex error code section:     0x280
#define BHE_THREAD_ERROR                        0x281
#define BHE_THREAD_TIMED_OUT                    0x282
//////////////////////////////////////////////////

//Applet manager error code section:            0x300
/* JEFF file load fail, OOM or file format error not distinct by VM*/
#define BHE_LOAD_JEFF_FAIL                      0x303
/* Request operation on a package, but it does not exist.*/
#define BHE_PACKAGE_NOT_FOUND                   0x304
/* Uninstall package fail because of live session exist.*/
#define BHE_EXIST_LIVE_SESSION                  0x305
/* VM instance init fail when create session.*/
#define BHE_VM_INSTANCE_INIT_FAIL               0x306
/* Query applet property that Beihai does not support.*/
#define BHE_QUERY_PROP_NOT_SUPPORT              0x307
/* Incorrect Beihai package format */
#define BHE_INVALID_BPK_FILE                    0x308
/* Download a package which has already exists in app manager*/
#define BHE_PACKAGE_EXIST                       0x309
/* VM instance not found */
#define BHE_VM_INSTNACE_NOT_FOUND               0x312
/* JDWP agent starting fail */
#define BHE_STARTING_JDWP_FAIL                  0x313
/* Group access checking fail*/
#define BHE_GROUP_CHECK_FAIL                    0x314
/* package SDID dose not equal to the effective one in app manager*/
#define BHE_SDID_UNMATCH                        0x315
#define BHE_APPPACK_UNINITED                    0x316
#define BHE_SESSION_NUM_EXCEED                  0x317
/* TA package verify failure */
#define BHE_TA_PACKAGE_HASH_VERIFY_FAIL         0x318
/*SDID has not been reset to invalid one
#define BHE_SDID_NOT_RESET                      0x316
*/
/* Switch SD for IVM*/
#define BHE_SWITCH_ISD                          0x319
//////////////////////////////////////////////////

//VM Applet instance error code section:        0x400
#define BHE_APPLET_GENERIC                      0x400
#define BHE_UNCAUGHT_EXCEPTION                  0x401
/* Bad parameters to applet */
#define BHE_APPLET_BAD_PARAMETER                0x402
/* Small response buffer */
#define BHE_APPLET_SMALL_BUFFER                 0x403
/* Bad state */
#define BHE_BAD_STATE                           0x404
//////////////////////////////////////////////////

/*TODO: Should be removed these UI error code when integrate with ME 9 */
#define BHE_UI_EXCEPTION                        0x501
#define BHE_UI_ILLEGAL_USE                      0x502
#define BHE_UI_ILLEGAL_PARAMETER                0x503
#define BHE_UI_NOT_INITIALIZED                  0x504
#define BHE_UI_NOT_SUPPORTED                    0x505
#define BHE_UI_OUT_OF_RESOURCES                 0x506
//////////////////////////////////////////////////

//BeiHai VMInternalError code section:          0x600
#define BHE_UNKOWN                              0x602
#define BHE_MAGIC_UNMATCH                       0x603
#define BHE_UNIMPLEMENTED                       0x604
#define BHE_INTR                                0x605
#define BHE_CLOSED                              0x606
#define BHE_BUFFER_OVERFLOW                     0x607   /* TODO: no used error, should remove*/
#define BHE_NOT_SUPPORTED                       0x608
#define BHE_WEAR_OUT_VIOLATION                  0x609
#define BHE_NOT_FOUND                           0x610
#define BHE_INVALID_PARAMS                      0x611
#define BHE_ACCESS_DENIED                       0x612
#define BHE_INVALID                             0x614
#define BHE_TIMEOUT                             0x615
//////////////////////////////////////////////////

//SDM specific error code section:              0x800
#define BHE_SDM_FAILED                          0x800
#define BHE_SDM_NOT_FOUND                       0x801
#define BHE_SDM_ALREADY_EXIST                   0x803
#define BHE_SDM_TATYPE_MISMATCH                 0x804
#define BHE_SDM_TA_NUMBER_LIMIT                 0x805
#define BHE_SDM_SIGNAGURE_VERIFY_FAIL           0x806
#define BHE_SDM_PERMGROUP_CHECK_FAIL            0x807
#define BHE_SDM_INSTALL_CONDITION_FAIL          0x808
#define BHE_SDM_SVN_CHECK_FAIL                  0x809
#define BHE_SDM_TA_DB_NO_FREE_SLOT              0x80A
#define BHE_SDM_SD_DB_NO_FREE_SLOT              0x80B
#define BHE_SDM_SVL_DB_NO_FREE_SLOT             0x80C
#define BHE_SDM_SVL_CHECK_FAIL                  0x80D
#define BHE_SDM_DB_READ_FAIL                    0x80E
#define BHE_SDM_DB_WRITE_FAIL                   0x80F
// ......
//////////////////////////////////////////////////

//Launcher specific error code section:         0x900
#define BHE_LAUNCHER_INIT_FAILED                0x901
#define BHE_SD_NOT_INSTALLED                    0x902
#define BHE_NTA_NOT_INSTALLED                   0x903
#define BHE_PROCESS_SPAWN_FAILED                0x904
#define BHE_PROCESS_KILL_FAILED                 0x905
#define BHE_PROCESS_ALREADY_RUNNING             0x906
#define BHE_PROCESS_IN_TERMINATING              0x907
#define BHE_PROCESS_NOT_EXIST                   0x908
#define BHE_PLATFORM_API_ERR                    0x909
#define BHE_PROCESS_NUM_EXCEED                  0x09A
//////////////////////////////////////////////////

//BeihaiHAL Layer error code section: 0x1000,0x2000
//  .... reserved here, defined in CSG BeihaiStatusHAL.h
//////////////////////////////////////////////////


//
// BeihaiStatusHAL
//
typedef enum _BH_STATUS
{
   HAL_SUCCESS                      = 0x00000000,

   HAL_TIMED_OUT                    = 0x00001001,
   HAL_FAILURE                      = 0x00001002,
   HAL_OUT_OF_RESOURCES             = 0x00001003,
   HAL_OUT_OF_MEMORY                = 0x00001004,
   HAL_BUFFER_TOO_SMALL             = 0x00001005,
   HAL_INVALID_HANDLE               = 0x00001006,
   HAL_NOT_INITIALIZED              = 0x00001007,
   HAL_INVALID_PARAMS               = 0x00001008,
   HAL_NOT_SUPPORTED                = 0x00001009,
   HAL_NO_EVENTS                    = 0x0000100A,
   HAL_NOT_READY                    = 0x0000100B,
   HAL_CONNECTION_CLOSED            = 0x0000100C,
   // ...etc

   HAL_INTERNAL_ERROR               = 0x00001100,
   HAL_ILLEGAL_FORMAT               = 0x00001101,
   HAL_LINKER_ERROR                 = 0x00001102,
   HAL_VERIFIER_ERROR               = 0x00001103,

   // User defined applet & session errors to be returned to the host (should be exposed also in the host DLL)
   HAL_FW_VERSION_MISMATCH          = 0x00002000,
   HAL_ILLEGAL_SIGNATURE            = 0x00002001,
   HAL_ILLEGAL_POLICY_SECTION       = 0x00002002,
   HAL_OUT_OF_STORAGE               = 0x00002003,
   HAL_UNSUPPORTED_PLATFORM_TYPE    = 0x00002004,
   HAL_UNSUPPORTED_CPU_TYPE         = 0x00002005,
   HAL_UNSUPPORTED_PCH_TYPE         = 0x00002006,
   HAL_UNSUPPORTED_FEATURE_SET      = 0x00002007,
   HAL_ILLEGAL_VERSION              = 0x00002008,
   HAL_ALREADY_INSTALLED            = 0x00002009,
   HAL_MISSING_POLICY               = 0x00002010,
   HAL_ILLEGAL_PLATFORM_ID          = 0x00002011,
   HAL_UNSUPPORTED_API_LEVEL        = 0x00002012,
   HAL_LIBRARY_VERSION_MISMATCH     = 0x00002013

   // ... etc

} BH_STATUS;


//
// bh_shared_types
//
typedef INT8  BH_I8;
typedef UINT8 BH_U8;
typedef INT16 BH_I16;
typedef UINT16 BH_U16;
typedef INT32 BH_I32;
typedef UINT32 BH_U32;
typedef INT64 BH_I64;
typedef UINT64 BH_U64;

typedef BH_U64 BH_GROUP;
#define BH_GUID_LENGTH 16
#define BH_MAX_PACK_HASH_LEN 32

typedef struct {
  BH_U8 data[BH_MAX_PACK_HASH_LEN];
} BH_PACK_HASH;

typedef struct {
  BH_U8 data[BH_GUID_LENGTH];
} BH_TAID;
#define BH_TAID_LEN sizeof(BH_TAID)

typedef struct {
  BH_U8 data[BH_GUID_LENGTH];
} BH_SDID;

#define BH_SDID_LEN sizeof(BH_SDID)


/*
Firmware properties are formatted as "type\0key\0value\0"
Example: "string\0name\0Tom\0int\0Age\013\0"
*/
struct _bh_ta_install_condition_list_t { //same as BH_PROP_LIST, and needs discussion with CSG
  BH_U32 num;
  BH_U32 len;
  BH_I8 data[0];
};


//
// bhp_exp
//
typedef BOOLEAN bool;
#ifndef true
#define true TRUE
#endif
#ifndef false
#define false FALSE
#endif


typedef void* SD_SESSION_HANDLE; //SD session handle
typedef void* JAVATA_SESSION_HANDLE; //Java TA session handle

typedef int (*PFN_BHP_TRANSPORT_SEND)    (unsigned int handle, unsigned char* buffer, unsigned int length);
typedef int (*PFN_BHP_TRANSPORT_RECEIVE) (unsigned int handle, unsigned char* buffer, unsigned int* length);
typedef int (*PFN_BHP_TRANSPORT_CONNECT) (int heci_port, unsigned int* handle);
typedef int (*PFN_BHP_TRANSPORT_CLOSE)  (unsigned int handle);

typedef struct
{
    PFN_BHP_TRANSPORT_SEND  pfnSend;
    PFN_BHP_TRANSPORT_RECEIVE pfnRecv;
    PFN_BHP_TRANSPORT_CONNECT pfnConnect; //connect to specified heci_guid in FW
    PFN_BHP_TRANSPORT_CLOSE pfnClose; //close the connection
} BHP_TRANSPORT;

typedef struct _BHP_INFO {
  //
  // FW info
  //
  UINT16      Major;
  UINT16      Minor;
  UINT16      Hotfix;
  UINT16      Build;

  //
  // PluginType
  //
  UINT16      PluginType;
} BHP_INFO;

/**
 * Invoke this function before using other API.
 * It will try to connect ME processes(Launcher, SDM and I-VM), and create receiving threads
 * for those process and do other initialization.
 *
 *
 * @return BH_SUCCESS if succuss
 *
 * @return BPE_NO_CONNECTION_TO_FIRMWARE if failed to HECI initialation
 * @return BPE_INTERNAL_ERROR if receiver thread cannot be created or other internal failure
 */
BH_RET BHP_Init (const BHP_TRANSPORT* transport, BHP_INFO *info);


/**
 * Invoke this function before exiting.
 * If BHP_Init is not called, this function will do nothing.
 * If anything goes wrong, please call this function to release resources.
 *
 * @return BH_SUCCESS if success
 */
BH_RET BHP_Deinit (void);

/**
 * Send Reset command to SDM, Launcher and VM, to let them enter initial state.
 * This function will be blocked until receiving all the responses.
 *
 * @return BH_SUCCESS if success.
 */
BH_RET BHP_Reset (void);

/**
 * Open SD(Security Domain) Session.
 *
 * @param SD_ID [IN] the SD ID(UUID) to be opened.
 * @param handle [OUT] the returned session handle.
 *
 * @return BH_SUCCESS if succuss
 *
 */
BH_RET BHP_OpenSDSession(const char* SD_ID, SD_SESSION_HANDLE* pSession);

/**
 * close specified sd session.
 * @param handle [IN] the sd session handle.
 *
 * @return BH_SUCCESS if success
 */
BH_RET BHP_CloseSDSession(const SD_SESSION_HANDLE handle);

/**
  * send admin cmd package to SD.
  * The admin cmd pkg could be for installsd, uninstallsd, install_javata,uninstall_javata,
  * install_nativeta, uninstall_nativeta.
  *
  * @param handle [IN] the sd session handle.
  * @param cmd_pkg [IN] admin cmd package.
  *        If it is Install Java/Native TA CMD, it shall include both ACP and the according Java/Native TA package.
  *        The two parts shall be concatenated continuously.
  *        The pkg_len parameter shall be the total length of these two parts.
  * @param pkg_len [IN] admin cmd package length.
  *
  * @return BH_SUCCESS if success
  */
BH_RET BHP_SendAdminCmdPkg(const SD_SESSION_HANDLE handle, const char* cmd_pkg, unsigned int pkg_len);

/**
 * Open Session to specified Java TA.
 * The Firmware side might need to spawn the VM process, and create the TA instance.
 * This function will also connect to the VM process's heci address.
 * This function will block until VM replied the response.
 * Please call BHP_Deinit() to clean up when anything goes wrong.
 *
 * @param pSession [OUT] the ta session handle, which is used in the function BHP_SendAndRecv.
 * @param TA_Id [IN] the applet ID (UUID) to create session.
 * @param TA_pkg [in] TA binary package, i.e, .bpk data.
 * @param pkg_len [in] The length of TA binary package in bytes.
 * @param init_param [IN] the input buffer of the CreateSession command.
 * @param param_len [IN] the length of init_param in bytes
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_OpenTASession (JAVATA_SESSION_HANDLE* pSession, const char *TA_Id,  const char* TA_pkg, unsigned int pkg_len,
                    const char* init_param, unsigned int param_len);

/**
 * Send a CloseTASession command to VM to close the specified Java TA session.
 * This function will be blocked until VM replies the response.
 *
 * @param pSession [IN] the java ta session handle to close.
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_CloseTASession (const JAVATA_SESSION_HANDLE handle);

/**
 * Send a ForceCloseSession command to VM. This function will be blocked until VM replies the response.
 *
 * @param pSession [IN] the session handle to close.
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ForceCloseTASession (const JAVATA_SESSION_HANDLE handle);

/**
 * Send a SendAndRecv command to VM. This function will be blocked until VM replies the response.
 *
 * @param pSession [IN] the java ta session handle.
 * @param nCommandId [IN] the command ID.
 * @param input [IN] the input buffer to be sent to TA.
 * @param length [IN] the length of input buffer.
 * @param output [OUT] the pointer to output buffer.
 * @param output_length [IN/OUT] the expected maximum length of output buffer / the actually length of output buffer.
 * @param pResponseCode [OUT] the command result, which is set by IntelApplet.setResponseCode()
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_SendAndRecv (const JAVATA_SESSION_HANDLE handle, int nCommandId, const void* input, unsigned int length,
    void** output, unsigned int* output_length, int* pResponseCode);

/**
 * Send a SendAndRecvInternal command to VM. This function will be blocked until VM replies the response.
 *
 * @param pSession [IN] the java ta session handle.
 * @param what [IN] the message type.
 * @param nCommandId [IN] the command ID.
 * @param input [IN] the input buffer to be sent to TA.
 * @param length [IN] the length of input buffer.
 * @param output [OUT] the pointer to output buffer.
 * @param output_length [IN/OUT] the expected maximum length of output buffer / the actually length of output buffer.
 * @param pResponseCode [OUT] the command result, which is set by IntelApplet.setResponseCode()
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_SendAndRecvInternal (const JAVATA_SESSION_HANDLE handle, int what, int nCommandId,
    const void* input, unsigned int length, void** output, unsigned int* output_length, int* pResponseCode);

/**
 * Send a ListTAProperties command to VM. This function return all property names of the specific TA.
 * This function only works for the TA whose TA package is already downloaded to VM.
 *
 * @param appId[IN]  the TA uuid string to query
 * @param count[OUT]  the number of TA properties
 * @param properties[OUT] the result list of property names
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ListTAProperties (const char* appId, unsigned int *count, char*** properties);

/**
 * Send a QueryTAProperty command to VM. This function will be blocked until VM replies the response.
 * This function only works for the TA whose TA package is already downloaded to VM.
 *
 * @param appId [IN] the TA ID to Query.
 * @param property [IN] the property name to be queried.
 * @param length [IN] the length of property.
 * @param output [OUT] the pointer to output buffer, which is allocated by BHP and should be freed after used.
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_QueryTAProperty (const char *appId, const void* property, unsigned int length, char** output);

/**
 * Send a ListTASessions comand to VM. This function return count and array of Session Handle if success.
 *
 * @param appId [IN] Java TA ID to query
 * @param count [OUT] count of Sessions
 * @param pSessions [OUT] Array of Session Handles. Allocated by BHP,  and should be freed after used.
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ListTASessions (const char* appId, unsigned int* count, JAVATA_SESSION_HANDLE** pSessions);

/**
 * Functions to get the list of downloaded TA Packages in Firmware for specific SD.
 *   It will list both JavaTA and Native TA.
 *
 * @param SD_ID [IN] the specific SD UUID string
 * @param count [OUT] number of packages
 * @param appIdStrs [OUT] String Array of TA UUID. Allocated by bhp, please free it after usage.
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ListDownloadedTAs (const char* SD_ID, unsigned int *count, char*** appIdStrs);

/**
 * List intalled SD information.
 * It will send listSD cmd to SDM process.
 *
 * @param handle [IN] SD Session Handle
 * @param count [OUT] count of the sd ids.
 * @param sdIdStrs [OUT] installed sd UUID strings. It is allocated inside this function,
 *        and caller should be responsible to free it using BHP_Free().
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ListInstalledSDs(const SD_SESSION_HANDLE handle, unsigned int* count, char*** sdIdStrs);

/**
 * List intalled TA in specified SD.
 * It will send listTA cmd to SDM process.
 *
 * @param handle [IN] SD Session Handle
 * @param SD_ID [IN] sd UUID string.
 * @param count [OUT] count of the ta UUIDs.
 * @param appIdStrs [OUT] installed ta UUID strings. It is allocated inside this function,
 *        and caller should be responsible to free it using BHP_Free().
 *
 * @return BH_SUCCESS if success
 *
 */
BH_RET BHP_ListInstalledTAs(const SD_SESSION_HANDLE handle, const char* SD_ID, unsigned int * count, char*** appIdStrs);

/**
 * Function to free memory allocated in BHP
 *
 * @param ptr of buffer to free
 */
void BHP_Free (void* ptr);

/**
  * Run native TA.
  * It will download the TA package to firmware, then native TA will be spawned.
  *
  * @param TA_ID [IN] the TA UUID.
  * @param TA_pkg [IN] The native ta package.
  * @param pkg_len [IN] the length of ta package in bytes.
  *
  * @return BH_SUCCESS if success
  *
  */
BH_RET BHP_RunNativeTA(const char* TA_ID, const char* TA_pkg, unsigned int pkg_len);

/**
  * Stop Native TA.
  * It will send StopNativeTA cmd to Launcher, which will kill the native TA process.
  *
  * @param TA_ID [IN]  the TA UUID.
  *
  * @return BH_SUCCESS if success
  *
  */
BH_RET BHP_StopNativeTA(const char* TA_ID);


//
// bh_acp_format
//
/**
 * @brief  BHP package format definition
 * Ins is short for install
 * Sig is short for signature
 * AC is short for Admin Command
 * TA is short for Trusted Application
 * NTA is for nativeTA and JTA is for JavaTA
 * SD is short for Security Domain
 * Check CSE Beihai HLD for more details of the keyword
 *
 * Each section has to be aligned on 4 bytes
 * Changes applied on them should be consitent with
 * package_generator(BPKT) and package_parser(admin_pack lib)
 * ACPackage
 *   |--ACPackHeader--|--ACInsCond--|--data--|--ACSignature--|
 *   Use idx_XXX to reference condition/data
 * The package that ACPackage.data contains:
 *   AC Install SD Package structure:
 *     |--ACInsSDHeader--|--ACInsGroups--|--ACName--|
 *   AC Uninstall SD
 *     |--BH_SDID--|
 *   AC Install JTA Package structure:
 *     |--ACInsJTAHeader--|--ACInsHash--|--ACInsGroups--|
 *     |--ACInsReasons(post)--|--ACInsReasons(reg)--|--ACName--|
 *   AC Intall Native TA Package structure:
 *     |--ACInsNTAHeader--|--ACInsHash--|--ACInsGroups--|
 *     |--ACInsMetadata--|--ACName--|
 *   AC UninstallTA
 *     |--BH_TAID, ACInsCond--|
 */

//make sure those limitation values are adjusted to real world
#define BH_MAX_ACP_INS_REASONS_LENGTH 1024
#define BH_MAX_ACP_USED_SERVICES 20
#define BH_MAX_ACP_SVL_RECORDS 40
#define BH_MAX_ACP_PORPS_LENGTH 2048
#define BH_MAX_ACP_SIG_EXT_LENGTH 544
#define BH_MAX_ACP_NTA_METADATA_LENGTH 5120

typedef char* TEMP_SIG_KEY;
typedef BH_U32 TEMP_SIG_SIG;
typedef char* TEMP_COND;
typedef BH_U32 TEMP_HASH;

enum AC_CMD_ID {
  AC_CMD_INVALID,
  AC_INSTALL_SD,
  AC_UNINSTALL_SD,
  AC_INSTALL_JTA,
  AC_UNINSTALL_JTA,
  AC_INSTALL_NTA,
  AC_UNINSTALL_NTA,
  AC_UPDATE_SVL,
  AC_INSTALL_JTA_PROP,
  AC_CMD_NUM
};

enum AC_TA_TYPE {
  AC_TA_TYPE_BOTH,
  AC_TA_TYPE_JAVA,
  AC_TA_TYPE_NATIVE,
};

/*TBD:
typedef struct {
    BH_U32 enc_alg;
    BH_U32 len;
    char data[0];
} ACEncryption;
*/

#pragma pack(1)

typedef struct {
  /*ACP Header*/
  BH_U8 magic[4];
  BH_U8 version;
  BH_U8 little_endian;
  BH_U16 reserved;
  BH_U32 size;/*total package size in byte except signature*/
  BH_U32 cmd_id;
  BH_U32 svn;

  /*Section Index*/
  BH_U32 idx_num;
  BH_U32 idx_condition;
  /*TBD: BH_U32 idx_encrypt;*/
  BH_U32 idx_data;
} ACPackHeader;


typedef struct {
  BH_U8 len;/*the size of data in byte*/
  BH_I8 data[0];
} ACName;

typedef struct {
  BH_TAID ta_id;
  BH_U32 ta_svn;
} ACSV;

struct BH_SV_LIST{
  BH_U32 num; //number of sv items
  ACSV data[0];
};

struct BH_TAID_LIST {
  BH_U32 num;
  BH_TAID list[0];
};

/*
Firmware properties are formatted as "type\0key\0value\0"
Example: "string\0name\0Tom\0int\0Age\013\0"
*/
struct BH_PROP_LIST {
  BH_U32 num; //number of properties
  BH_U32 len; //the size of data in byte
  BH_I8 data[0];
};

typedef struct BH_PROP_LIST ACProp;
typedef struct BH_TAID_LIST ACTAIDList;
typedef struct BH_SV_LIST ACSVList;
typedef struct {
  BH_U32 len; /*the amount of items in data*/
  BH_U8 data[0];
} ACInsHash;

typedef struct {
  /*NOTE: len means the amount of items in data,
  when counting total bytes, you need to multiply it with sizeof(data[0]) */
  BH_U32 len;
  BH_U32 data[0];
} ACInsReasons;

typedef struct {
  BH_U32 len; /*the amount of items in data*/
  BH_U8 data[0];
} ACInsMetadata;

typedef struct {
  BH_SDID sd_id;
  BH_U32 sd_svn;
  BH_U16 ssd_num;
  BH_U8 ta_type;
  BH_U8 reserved;
  BH_U16 max_ta_can_install;
  BH_U16 max_ta_can_run;
  BH_U32 flash_quota;
  BH_U64 ac_groups;
  BH_I8 sd_name[32];
} ACInsSDHeader;

#pragma pack()

//it should match the key type with BeihaiHAL.h
enum SIG_KEY_TYPE {UNKNOWN = 0, INTEL_PLATFORM_KEY, DAL_KEY_1, INTEL_KEY_Manifest_V1, AC_SIG_KEY_OTHER};

#define AC_SIG_KEY_LEN 260

#pragma pack(1)

typedef struct {
  BH_U16 sig_alg;
  BH_U16 sig_key_type;
  /*sig_data covers sig_key and sig_manifest,
    parser of signature should be reponsible to extract them
    according to sig_alg and sig_key_type*/
  BH_I8 sig_key[AC_SIG_KEY_LEN];
} ACInsSDSigKey;

#define AC_MAX_SIGNATURE_LEN 256
typedef struct {
  BH_U8 sig_magic[4];
  BH_I8 sig_version;
  BH_U8 reserved;
  BH_U16 sig_len;
  BH_SDID sig_sd;
  BH_U16 sig_alg;
  BH_U16 sig_key_type;
  BH_I8 sig_key_manifest[AC_SIG_KEY_LEN];
  /*sig_data covers signing result
    signature verification API should be reponsible to extract them
    according to sig_alg and sig_key_type.
    Its max length is AC_MAX_SIGNATURE_LEN*/
  BH_I8 sig_data[0];
} ACSig;

/*
below structures are the parsing result that application layer should use
they are in-memory representation of admin packages
*/
typedef struct {
  ACPackHeader *head;
  /*the type of data depends on head->cmd_id*/
  char data[0];
  /*--ACSignature is appendeded after command package*/
} ACPack;

typedef struct {
  ACProp* ins_cond;
  ACInsSDHeader* head;
  ACInsSDSigKey* sig_key;
} ACInsSDPack;

typedef struct {
  BH_SDID* p_sdid;
} ACUnsSDPack;

typedef struct {
  BH_TAID ta_id;
  BH_U32 ta_svn;
  BH_U8 hash_alg_type;
  BH_U8 ta_reserved[3];
  BH_PACK_HASH hash;
} ACInsTAHeader; /*header struct shared between JTA and NTA*/

typedef ACInsTAHeader ACInsNTAHeader;
typedef ACInsTAHeader ACInsJTAHeader;

typedef struct {
  ACProp* ins_cond;
  ACInsJTAHeader* head;
} ACInsJTAPack;

typedef struct {
  BH_U32 mem_quota;
  BH_U8 ta_encrypted;
  BH_U8 padding;
  BH_U16 allowed_inter_session_num;
  BH_U64 ac_groups;
  BH_U32 timeout;
} ACInsJTAPropHeader;

typedef struct {
  ACInsJTAPropHeader* head;
  ACInsReasons *post_reasons;
  ACInsReasons *reg_reasons;
  ACProp *prop;
  ACTAIDList *used_service_list;
} ACInsJTAProp;


typedef struct {
  ACProp* ins_cond;
  ACInsNTAHeader* head;
  ACInsMetadata* mdata;
} ACInsNTAPack;

typedef struct {
  BH_TAID* p_taid;
} ACUnsTAPack;

typedef struct {
  ACProp* ins_cond;
  ACSVList* sv_list;
} ACUpdateSVLPack;

#pragma pack()



//
// bh_acp_exp
//
#pragma pack(1)

typedef struct {
  ACPackHeader* head;
  ACInsSDPack cmd_pack;
  ACSig* sign;
} ACInsSDPackExt;

typedef struct {
  ACPackHeader* head;
  ACUnsSDPack cmd_pack;
  ACSig* sign;
} ACUnsSDPackExt;

typedef struct {
  ACPackHeader* head;
  ACInsJTAPack cmd_pack;
  ACSig* sign;
  char* ta_pack;
} ACInsJTAPackExt;

typedef struct {
  ACPackHeader* head;
  ACInsNTAPack cmd_pack;
  ACSig* sign;
  char* ta_pack;
} ACInsNTAPackExt;

typedef struct {
  ACPackHeader *head;
  ACUnsTAPack cmd_pack;
  ACSig* sign;
} ACUnsTAPackExt;

typedef struct {
  ACInsJTAProp cmd_pack;
  char* jeff_pack;
} ACInsJTAPropExt;

typedef struct {
  ACPackHeader* head;
  ACUpdateSVLPack cmd_pack;
  ACSig* sign;
} ACUpdateSVLPackExt;

typedef struct {
  ACProp *props;
} ACTAProps;

#pragma pack()

BH_RET ACP_pload_ins_sd(const void* raw_data, unsigned size, ACInsSDPackExt *pack);
BH_RET ACP_pload_uns_sd(const void* raw_data, unsigned size, ACUnsSDPackExt *pack);
BH_RET ACP_pload_ins_jta(const void* raw_data, unsigned size, ACInsJTAPackExt *pack);
BH_RET ACP_pload_ins_nta(const void* raw_data, unsigned size, ACInsNTAPackExt *pack);
BH_RET ACP_pload_uns_jta(const void* raw_data, unsigned size, ACUnsTAPackExt *pack);
BH_RET ACP_pload_uns_nta(const void* raw_data, unsigned size, ACUnsTAPackExt *pack);
BH_RET ACP_pload_ins_jta_prop(const void* raw_data, unsigned size, ACInsJTAPropExt *pack);
BH_RET ACP_get_cmd_id(const void* raw_data, unsigned size, int* cmd_id);
BH_RET ACP_pload_update_svl(const void *raw_data, unsigned size, ACUpdateSVLPackExt*pack);


//
// JhiPlugin
//

/**
  Initialize Beihai library.

  @param[in] info                 The pointer to the buffer of BHP_INFO.

  @retval JHI_SUCCESS             Initialize Beihai library successfully.
  @retval others                  Initialize Beihai library failed with error code.
**/
UINT32
JHI_Plugin_Init (
  OUT BHP_INFO *info
);

/**
  De-initialize Beihai library.

  @retval JHI_SUCCESS             De-initialize Beihai library successfully.
  @retval others                  De-initialize Beihai library failed with error code.
**/
UINT32
JHI_Plugin_DeInit (VOID);

/**
  Download an applet to Beihai library.

  @param[in] pAppId               The AppId.
  @param[in] pAppBlob             The pointer to the buffer of applet.
  @param[in] AppSize              The size of applet.

  @retval JHI_SUCCESS             Download an applet successfully.
  @retval others                  Download an applet failed with error code.
**/
UINT32
JHI_Plugin_DownloadApplet (
  IN CONST CHAR8  *pAppId,
  IN UINT8        *pAppBlob,
  IN unsigned int AppSize
);

/**
  Unload an applet from Beihai library.

  @param[in] pAppId               The AppId.

  @retval JHI_SUCCESS             Unload an applet successfully.
  @retval others                  Unload an applet failed with error code.
**/
UINT32
JHI_Plugin_UnloadApplet (
  IN CONST CHAR8 *AppId
);

/**
  Create a session for an applet to Beihai library.

  @param[in] pAppId               The AppId.
  @param[out] pSession            The session handle.
  @param[in] pAppBlob             The pointer to the content of applet.
  @param[in] BlobSize             The size of the applet.
  @param[in] SessionID            The session ID for the sessin.
  @param[in] InitBuffer           The pointer to the buffer for session initialization.

  @retval JHI_SUCCESS             Create a session successfully.
  @retval others                  Create a session failed with error code.
**/
UINT32
JHI_Plugin_CreateSession (
  IN CONST CHAR8        *AppId,
  OUT VM_SESSION_HANDLE *pSession,
  IN CONST UINT8        *pAppBlob,
  IN unsigned int       BlobSize,
  IN JHI_SESSION_ID     SessionID,
  IN DATA_BUFFER        *InitBuffer
);

/**
  Close a session for an applet to Beihai library.

  @param[in] pSession             The session handle.

  @retval JHI_SUCCESS             Close a session successfully.
  @retval others                  Close a session failed with error code.
**/
UINT32
JHI_Plugin_CloseSession (
  IN VM_SESSION_HANDLE* pSession
);

/**
  Command, data transition to an applet by session.

  @param[in] Session              The session handle.
  @param[in] nCommandId           The command ID.
  @param[in] pIOBuffer            The pointer to the buffer for this command.
  @param[out] pResponseCode       The pointer to the buffer of response code.

  @retval JHI_SUCCESS             Transition successfully.
  @retval others                  Transition failed with error code.
**/
UINT32
JHI_Plugin_SendAndRecv (
  IN VM_SESSION_HANDLE  Session,
  IN INT32              nCommandId,
  IN JVM_COMM_BUFFER    *pIOBuffer,
  OUT INT32             *pResponseCode
);

/**
  Get property of an applet.

  @param[in] pAppId               The AppId.
  @param[in, out] pIOBuffer       The pointer to the buffer of property and return value.

  @retval JHI_SUCCESS             Get property successfully.
  @retval others                  Get property failed with error code.
**/
UINT32
JHI_Plugin_GetAppletProperty (
  IN CONST CHAR8          *AppId,
  IN OUT JVM_COMM_BUFFER  *pIOBuffer
);

#endif // __DXE_BEIHAI_LIB_H__

