/** @file
  This file contains the System Agent PCIE DSM method definition.

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

External(PINI, MethodObj) // Platform specific PCIe root port initialization
External(PPBA, MethodObj) // PCIe power budget allocation

Name(LTRV, Package(){0,0,0,0})

OperationRegion(PXCS,PCI_Config,0x00,0x480)
Field(PXCS,AnyAcc, NoLock, Preserve) {
  Offset(0),
  VDID, 32,
}

//
// Check if root port is present.
// @return 0: root port is disabled, 1: root port is enabled
//
Method(PRES) {
  If (LEqual (VDID, 0xFFFFFFFF)) {
    Return(0)
  } Else {
    Return(1)
  }
}

Name(LNRD, 0) // Last negotiated PERST# assertion delay in us

//
// Update PERST# assertion delay.
// Arg0 - New delay value in microseconds. Max is 10ms
//
// @return Last sucessfully negotiated value in us. 0 if no such value exists.
//
Method(UPRD, 1, Serialized) {
  If(LNot(LGreater(Arg0, 10000))) {
    // If the value does not exceed the limit
    // Update last negotiated value and calculate the value in ms.
    Store(Arg0, LNRD)
  }
  Return(LNRD)
}

//
// _DSM Device Specific Method
//
// Arg0: UUID Unique function identifier
// Arg1: Integer Revision Level
// Arg2: Integer Function Index (0 = Return Supported Functions)
// Arg3: Package Parameters
Method(_DSM, 4, Serialized) {
  //
  // Switch based on which unique function identifier was passed in
  //
  If (LEqual(Arg0, ToUUID ("E5C937D0-3553-4d7a-9117-EA4D19C3434D"))) {
    //
    // _DSM Definitions for Latency Tolerance Reporting
    //
    // Arguments:
    // Arg0: UUID: E5C937D0-3553-4d7a-9117-EA4D19C3434D
    // Arg1: Revision ID: 2
    // Arg2: Function Index: 6
    // Arg3: Empty Package
    //
    // Return:
    // A Package of four integers corresponding with the LTR encoding defined
    // in the PCI Express Base Specification, as follows:
    // Integer 0: Maximum Snoop Latency Scale
    // Integer 1: Maximum Snoop Latency Value
    // Integer 2: Maximum No-Snoop Latency Scale
    // Integer 3: Maximum No-Snoop Latency Value
    // These values correspond directly to the LTR Extended Capability Structure
    // fields described in the PCI Express Base Specification.
    //
    //
    // Switch by function index
    //
    Switch(ToInteger(Arg2)) {
      //
      // Function Index:0
      // Standard query - A bitmask of functions supported
      //
      Case (0) {
        Name(DSMF,Buffer(2){0,0})
        CreateBitField(DSMF,0,FUN0)
        CreateBitField(DSMF,4,FUN4)
        CreateBitField(DSMF,6,FUN6)
        CreateBitField(DSMF,8,FUN8)
        CreateBitField(DSMF,9,FUN9)
        CreateBitField(DSMF,10,FUNA)
        CreateBitField(DSMF,11,FUNB)
        //Store(0, Local0)
        Store(1,FUN0)
        if (LGreaterEqual(Arg1, 2)){          // test Arg1 for Revision ID: 2 or higher
          if (LTRS){
            //Or(Local0,0x40,Local0) // function 6
            Store(1,FUN6)
          }
          if (OBFS){
            //Or(Local0,0x10,Local0) // function 4
            Store(1,FUN4)
          }
        }
        if (LGreaterEqual(Arg1, 3)){ // test Arg1 for Revision ID: 3
          if (ECR1){
            //Or(Local0,0x100,Local0) // function 8
            Store(1,FUN8)
          }
          if (ECR1){
            //Or(Local0,0x200,Local0) } // function 9
            Store(1,FUN9)
          }
        }
        If (LGreaterEqual(Arg1, 4)) { //test Arg1 for Revision ID: 4
          If(CondRefOf(PPBA)) {
            Store(1,FUNA)
          }
          Store(1,FUNB)
        }
        //if(LNotEqual(Local0,0)) { Or(Local0,0x1,Local0) } //if any other function is supported, function0 must be supported too
        //return (Local0)
        return(DSMF)
      }
      //
      // Function Index: 4
      //
      Case(4) {
        if (LGreaterEqual(Arg1, 2)){ // test Arg1 for Revision ID: 2
          if (OBFS){
            Return (Buffer () {0,0,0,0,0,0,0,0,0,0,0,8,0,0,0,0}) // OBFF capable, offset 4[08h]
          } else {
            Return (Buffer () {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0})
          }
        }
      }
      //
      // Function Index: 6
      // LTR Extended Capability Structure
      //
      Case(6) {
        if (LGreaterEqual(Arg1, 2)){ // test Arg1 for Revision ID: 2
          if (LTRS){
            Store(And(ShiftRight(SMSL,10),7), Index(LTRV, 0))
            Store(And(SMSL,0x3FF), Index(LTRV, 1))
            Store(And(ShiftRight(SNSL,10),7), Index(LTRV, 2))
            Store(And(SNSL,0x3FF), Index(LTRV, 3))
            return (LTRV)
          } else {
            Return (0)
          }
        }
      }
      Case(8) { //ECR ACPI additions for FW latency optimizations, DSM for Avoiding Power-On Reset Delay Duplication on Sx Resume
        if(LEqual(ECR1,1)){
          if (LGreaterEqual(Arg1, 3)) { // test Arg1 for Revision ID: 3
            return (1)
          }
        }
      }
      Case(9) { //ECR ACPI additions for FW latency optimizations, DSM for Specifying Device Readiness Durations
        if(LEqual(ECR1,1)){
          if (LGreaterEqual(Arg1, 3)) { // test Arg1 for Revision ID: 3
            return(Package(5){50000,Ones,Ones,50000,Ones})
          }
        }
      }
      //
      //  Function index 10 - negotiate device auxilary power consumption.
      //
      Case(10) {
        If(CondRefOf(PPBA)) {
          Return(PPBA(Arg3))
        }
      }
      //
      // Function index 11 update delay between PME_TO_Ack and PERST# assertion
      //
      Case(11) {
        Return(UPRD(Arg3))
      }
    }
  }
  return (Buffer() {0x00})
} // End of _DSM

