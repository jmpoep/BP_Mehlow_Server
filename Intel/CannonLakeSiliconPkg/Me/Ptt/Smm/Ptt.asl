/** @file
  The TPM2 definition block in ACPI table for TrEE physical presence
  and MemoryClear.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2012 - 2017 Intel Corporation.

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

DefinitionBlock (
  "Tpm.aml",
  "SSDT",
  2,
  "Intel_",
  "Tpm2Tabl",
  0x1000
  )
{

  Scope (\_SB)
  {
    Device (TPM)
    {
      //
      // Define _HID, "PNP0C31" is defined in
      // "Secure Startup-FVE and TPM Admin BIOS and Platform Requirements"
      //
      Name (_HID, "MSFT0101")

      //
      // Readable name of this device, don't know if this way is correct yet
      //
      Name (_STR, Unicode ("TPM 2.0 Device"))

      //
      // Return the resource consumed by TPM device
      //
      Name (_CRS, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0xFED40000, 0x5000)
      })

       //
      // Operational region for Smi port access
      //
      OperationRegion (SMIP, SystemIO, 0xB2, 1)
      Field (SMIP, ByteAcc, NoLock, Preserve)
      {
          IOB2, 8
      }

      OperationRegion (FHCI, SystemMemory, 0xFED40000, 0x1000)
      Field (FHCI, AnyAcc, NoLock, Preserve)
      {
        Offset(0x04),
        HERR, 32,
        Offset (0x40),
        CREQ, 32,
        Offset (0x44),
        CSTS, 32,
        Offset (0x4C),
        HCMD, 32,
      }
      include ("PttNvs.asl")

      Method (PTS, 1, Serialized)
      {
        //
        // Detect Sx state for MOR, only S4, S5 need to handle
        //
        If (LAnd (LLess (Arg0, 6), LGreater (Arg0, 3)))
        {
          //
          // Bit4 -- DisableAutoDetect. 0 -- Firmware MAY autodetect.
          //
          If (LNot (And (MORD, 0x10)))
          {
            //
            // Triggle the SMI through ACPI _PTS method.
            //
            Store (0x02, MCIP)

            //
            // Triggle the SMI interrupt
            //
            Store (MCIN, IOB2)
          }
        }
        Return (0)
      }

      Method (_STA, 0)
      {
        Return (0x0f)
      }

      //
      // TCG Hardware Information
      //
      Method (HINF, 3, Serialized, 0, {BuffObj, PkgObj}, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger(Arg1))
        {
          Case (0)
          {
            //
            // Standard query
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Return failure if no TPM present
            //
            Name(TPMV, Package () {0x01, Package () {0x1, 0x20}})
            if (LEqual (_STA (), 0x00))
            {
              Return (Package () {0x00})
            }

            //
            // Return TPM version
            //
            Return (TPMV)
          }
          Default {BreakPoint}
        }
        Return (Buffer () {0})
      }

      Name(TPM2, Package (0x02){
        Zero,
        Zero
      })

      Name(TPM3, Package (0x03){
        Zero,
        Zero,
        Zero
      })

      //
      // TCG Physical Presence Interface
      //
      Method (TPPI, 3, Serialized, 0, {BuffObj, PkgObj, IntObj, StrObj}, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger(Arg1))
        {
          Case (0)
          {
            //
            // Standard query, supports function 1-8
            //
            Return (Buffer () {0xFF, 0x01})
          }
          Case (1)
          {
            //
            // a) Get Physical Presence Interface Version
            //
            Return ("$PV")
          }
          Case (2)
          {
            //
            // b) Submit TPM Operation Request to Pre-OS Environment
            //

            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)
            Store (0x02, PPIP)

            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)
            Return (FRET)


          }
          Case (3)
          {
            //
            // c) Get Pending TPM Operation Requested By the OS
            //

            Store (PPRQ, Index (TPM2, 0x01))
            Return (TPM2)
          }
          Case (4)
          {
            //
            // d) Get Platform-Specific Action to Transition to Pre-OS Environment
            //
            Return (2)
          }
          Case (5)
          {
            //
            // e) Return TPM Operation Response to OS Environment
            //
            Store (0x05, PPIP)

            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)

            Store (LPPR, Index (TPM3, 0x01))
            Store (PPRP, Index (TPM3, 0x02))

            Return (TPM3)
          }
          Case (6)
          {

            //
            // f) Submit preferred user language (Not implemented)
            //

            Return (3)

          }
          Case (7)
          {
            //
            // g) Submit TPM Operation Request to Pre-OS Environment 2
            //
            Store (7, PPIP)
            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)
            Store (0, PPRM)
            If (LEqual (PPRQ, 23)) {
              Store (DerefOf (Index (Arg2, 0x01)), PPRM)
            }

            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)
            Return (FRET)
          }
          Case (8)
          {
            //
            // e) Get User Confirmation Status for Operation
            //
            Store (8, PPIP)
            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)

            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)

            Return (FRET)
          }

          Default {BreakPoint}
        }
        Return (1)
      }

      Method (TMCI, 3, Serialized, 0, IntObj, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger (Arg1))
        {
          Case (0)
          {
            //
            // Standard query, supports function 1-1
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Save the Operation Value of the Request to MORD (reserved memory)
            //
            Store (DerefOf (Index (Arg2, 0x00)), MORD)

            //
            // Triggle the SMI through ACPI _DSM method.
            //
            Store (0x01, MCIP)

            //
            // Triggle the SMI interrupt
            //
            Store (MCIN, IOB2)
            Return (MRET)
          }
          Default {BreakPoint}
        }
        Return (1)
      }

      Method (TSMI, 3, Serialized, 0, IntObj, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        Name (WTME, 0)
        //
        // Switch by function index
        //
        Switch (ToInteger (Arg1))
        {
          Case (0)
          {
            //
            // Standard query, supports function 1-1
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Request for TPM to go to command ready
            //
            Store (0x00000001, CREQ)

            //
            // Wait for tpm to clear tpmidle
            //
            While(LAND(LLessEqual(WTME, 200),LNotEqual(And(CREQ,0x00000001),0)))
            {
              Sleep(1)
              Increment(WTME)
            }
            //
            // Trigger Command Interrupt
            //
            Store (0x00000002, HCMD)

            Return (0)
          }
          Default {BreakPoint}
        }
        Return (1)
      }

      Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
      {

        //
        // TCG Hardware Information
        //
        If(LEqual(Arg0, ToUUID ("cf8e16a5-c1e8-4e25-b712-4f54a96702c8")))
        {
          Return (HINF (Arg1, Arg2, Arg3))
        }

        //
        // TCG Physical Presence Interface
        //
        If(LEqual(Arg0, ToUUID ("3dddfaa6-361b-4eb4-a424-8d10089d1653")))
        {
          Return (TPPI (Arg1, Arg2, Arg3))
        }

        //
        // TCG Memory Clear Interface
        //
        If(LEqual(Arg0, ToUUID ("376054ed-cc13-4675-901c-4756d7f2d45d")))
        {
          Return (TMCI (Arg1, Arg2, Arg3))
        }

        //
        // TPM2 ACPI Start Method
        //
        If(LEqual(Arg0, ToUUID ("6bbf6cab-5463-4714-b7cd-f0203c0368d4")))
        {
          Return (TSMI (Arg1, Arg2, Arg3))
        }

        Return (Buffer () {0})
      }
    }
  }
}
