/**@file
  ACPI DSDT table for SCS Controllers

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2014 - 2019 Intel Corporation.

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

#include <Register/PchRegsScs.h>
#include <Register/PchRegsPcr.h>
#include <Register/PchRegsScsSd.h>
#include <Register/PchRegsScsCnl.h>
#include <PcieRegs.h>

#include <PchReservedResources.h>

External(\XBAS)

Scope(\_SB.PCI0) {

  //
  // DLL restore work around function.
  // When our integrated SD controllers exits D3 it will loose
  // the auto tuning settings it got after driver executed auto tuning
  // flow. Since OS drivers do not typically rerun the auto tuning flow on
  // D3 exit this could lead to potential problems with SD operation. To avoid
  // this BIOS saves the auto tuning DLL value and writes it to DLL value register
  // that is used when no auto tuning flow has been executed. This should guarantee correct
  // controller operation even if auto tuning has not been executed.
  // This function assumes:
  // 1. OS driver only runs the tuning once in the platform runtime
  // 2. The auto tune value that was established back then is good enough to
  //   suffice for entire platform runtime. Changes due to time and temperature are assumed to be insignificant
  // 3. The result of the auto tuning is not 0. 0 is used to identify D3 entries where OS driver didn't execute
  //    the tuning and restore should be skipped.
  //
  // @param[in] Arg0  SD controller bus number
  // @param[in] Arg1  SD controller device number
  // @param[in] Arg2  SD controller function number
  // @param[in] Arg3  MMIO address to use during restore flow
  // @param[in] Arg4  Size of the MMIO space
  //
  Method (DLLR, 5, Serialized) {
    ADBG ("SD DLL restore flow")

    Name (TEMP, 0)

    Name (EMPB, 0)
    Store (\XBAS, EMPB)
    Or(EMPB, ShiftLeft(Arg0, 20), EMPB)
    Or(EMPB, ShiftLeft(Arg1, 15), EMPB)
    Or(EMPB, ShiftLeft(Arg2, 12), EMPB)

    //
    // This is just a regular PCI access to SD controller.
    // We have to use SystemMemory region since PCI region access
    // can be blocked by OSPM if PCI_Config is used
    //
    OperationRegion (EMPC, SystemMemory, EMPB, 0x100)
    Field (EMPC, DWordAcc, NoLock, Preserve) {
      Offset(PCI_COMMAND_OFFSET),
         , 1,
      MSE, 1,
      Offset(R_PCI_BAR0_OFFSET),
      BAR0, 64,
      Offset(R_SCS_CFG_PCS),
      PSTA,32
    }

    Name (OPST, 0)
    Store (PSTA, OPST) // Save power status to restore it later

    //
    // At this point controller can be in D3Hot state and we have to transition it to D0 first.
    // to be able to access it's MMIO
    //
    And(PSTA, 0xFFFFFFFC, PSTA)
    Store(PSTA, TEMP) // Read back is neccessary for D-state transition to take effect

    Name (OMSE, 0)
    Store (MSE, OMSE) // Save MSE value to restore it later
    //
    // We can't assume that OSPM left a valid memory address for SD controller when it transitions it to D3
    // for that reason we save whatever value is in BAR0 and use one of our reserved ranges to run the restore flow.
    //
    Store (0, MSE)

    Name (OBAR, 0)
    Store (BAR0, OBAR) // Save BAR value to restore it later

    Store (Arg3, BAR0)
    Store (BAR0, TEMP) // Readback required to catch new BAR
    Store (1, MSE)

    //
    // @NOTE: Even though DLL registers are supposed to be
    // accessible via PCR space on real silicon access to that space
    // is failing and we have to do access via MMIO
    //
    OperationRegion (EMMI, SystemMemory, Arg3, Arg4)
    Field (EMMI, DWordAcc, NoLock, Preserve) {
      Offset(R_SCS_MEM_RX_CMD_DATA_DLL_CNTL2),
      FDLL, 8,
      Offset(R_SCS_MEM_AUTO_TUNING),
      ADLL, 8
    }

    Name (FDLV, 0)
    Name (ADLV, 0)

    Store (FDLL, FDLV)
    Store (ADLL, ADLV)

    ADBG (Concatenate ("Fixed DLL value ", ToHexString(FDLV)))
    ADBG (Concatenate ("Auto DLL Value ", ToHexString(ADLV)))

    //
    // Assume that if auto tuning DLL value is 0 the auto tuning flow
    // has not been executed and DLL restore should be skipped.
    //
    If(LNotEqual (ADLV, 0)) {
      ADBG ("Auto tuning executed, restoring values")

      //
      // We have to multiply auto tuned value by 2 to
      // adjust the value to the encoding of the fixed DLL
      // register.
      //
      Multiply (ADLV, 2, ADLV)
      Store (ADLV, FDLL)
    }

    Store (0, MSE)
    Store (OBAR, BAR0)
    Store (OMSE, MSE)
    Store (OPST, PSTA)
    Store(PSTA, TEMP) // Read back is neccessary for D-state transition to take effect
  }

  //
  // SCS UFS (PCI Mode)
  //
  Device(PUFS) {
    Name(_ADR, 0x00120005)
    Name (_DDN, "Intel(R) UFS Controller")

    Device (CARD) {
      Name (_ADR, 0x00000008) // Slot 0, Function 8
      Method(_RMV, 0x0, NotSerialized) { Return (0) } // Device cannot be removed
    }
  }

  //
  // SCS eMMC (PCI Mode)
  //
  Device(PEMC) {
    Name(_ADR, 0x001A0000)

    Method (_STA) {
      If (LEqual (\EMCE, 0)) {
        //
        // This will prevent OSPM from running
        // methods like _PSx for this device.
        //
        Return (0)
      } Else {
        //
        // Report as fully functional and to be shown in UI.
        //
        Return (0xF)
      }
    }


    // Memory Region to access to the eMMC PCI Configuration Space
    OperationRegion(SCSR, PCI_Config, 0x00, 0x100)
    Field(SCSR, WordAcc, NoLock, Preserve) {
      Offset(R_SCS_CFG_PCS),         // 0x84, PMCSR - Power Management Control and Status
      PSTA,32,
      Offset(R_SCS_CFG_PG_CONFIG),   // 0xA2, Device PG config
          , 2,
      PGEN, 1         // [BIT2] PGE - PG Enable
    }

    Method(_PS0, 0, Serialized) {
      Stall (50) // Sleep 50us

      Store(0, PGEN) // Disable PG

      //
      // On some platforms it has been observed that
      // transactions to SD and eMMC are failing due to
      // the OCP timeout being too low. This timeout doesn't
      // generate any interrupt so software can't react to this failure.
      // We disable the OCP timeout on D0 entry to avoid dropping transactions
      // to SD and eMMC. This write has to be done on D0 entry since power gating in D3
      // will reset it to default state.
      //
      PCRA (PID_EMMC, R_SCS_PCR_1C20, 0x0)
      PCRA (PID_EMMC, R_SCS_PCR_4820, 0x0)

      And(PSTA, 0xFFFFFFFC, PSTA) // Set BIT[1:0] = 00b - Power State D0
      Store(PSTA, TEMP) // Read Back PMCSR
    }

    Method(_PS3, 0, Serialized) {

      //
      // @NOTE: Run this work around before enabling power gating.
      //
      DLLR (
        0,
        PCI_DEVICE_NUMBER_PCH_CNL_SCS_EMMC,
        PCI_FUNCTION_NUMBER_PCH_CNL_SCS_EMMC,
        PCH_EMMC_BASE_ADDRESS,
        PCH_EMMC_MMIO_SIZE
        )

      Store(1, PGEN) // Enable PG

      Or(PSTA, 0x3, PSTA) // Set BIT[1:0] = 11b - Power State D3
      Store(PSTA, TEMP) // Read Back PMCSR
    }

    // _DSM x86 Device Specific Method
    // Arg0: UUID Unique function identifier
    // Arg1: Integer Revision Level
    // Arg2: Integer Function Index (0 = Return Supported Functions)
    // Arg3: Package Parameters
    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      If(PCIC(Arg0)) { return(PCID(Arg0,Arg1,Arg2,Arg3)) }

      ADBG(Concatenate("EMH4=",ToDecimalString(EMH4)))
      // Check the UUID
      If(LEqual(Arg0, ToUUID("f6c13ea5-65cd-461f-ab7a-29f7e8d5bd61"))) {
        // Check the revision
        If(LGreaterEqual(Arg1, Zero)) {
          //Switch statement based on the function index.
          Switch(ToInteger(Arg2)) {
            //
            // Function Index 0 the return value is a buffer containing
            // one bit for each function index, starting with zero.
            // Bit 0 - Indicates whether there is support for any functions other than function 0.
            // Bit 1 - Indicates support to clear power control register
            // Bit 2 - Indicates support to set power control register
            // Bit 3 - Indicates support to set 1.8V signalling
            // Bit 4 - Indicates support to set 3.3V signalling
            // Bit 5 - Indicates support for HS200 mode
            // Bit 6 - Indicates support for HS400 mode
            // Bit 9 - Indicates eMMC I/O Driver Strength
            //
            // For eMMC we have to support functions for
            // HS200 and HS400 and I/O Driver Strength [BIT9, BIT6, BIT5]
            //
            Case(0) {
              //
              // The return value 0x261 (BIT6 set) for HS400 enabled and
              // 0x221 (BIT6 not set) when HS400 is disabled in Setup menu.
              //
              If(LEqual(EMH4, 1)) {
                Return(Buffer() {0x61, 0x02}) // HS400 support enabled
              }
              Return(Buffer() {0x21, 0x02})   // HS400 support disabled
            }

            //
            // Function index 5 - corresponds to HS200 mode
            // Return value from this function is used to program
            // the UHS Mode Select bits in Host Control 2.
            // 011b - corresponds to SDR104 and according to the
            // SD Host Controller Spec and this value is overloaded
            // to program the controller to select HS200 mode for eMMC.
            //
            Case(5) {
              Return(Buffer() {0x3})
            }

            //
            // Function index 6 - corresponds to HS400 mode
            // Return value from this function is used to program
            // the UHS Mode Select bits in Host Control 2.
            // 101b is a reserved value according to the SD Host
            // Controller Spec and we use this value for HS400 mode
            // selection.
            //
            Case(6) {
              Return(Buffer() {0x5})
            }

            //
            // Function index 9 - corresponds to I/O Driver Strength
            // Return value from this function represents the values
            // of the Driver Strength selection
            // (eMMC 5.01 Specification JESD84-B50.1, Table 186)
            // that shall be programmed by the Host Driver (OS)
            // as part of the Initialization flows.
            //
            Case(9) {
              Switch(EMDS) {
                Case(0x0) { Return(Buffer() {0x0}) } // 50 Ohm
                Case(0x1) { Return(Buffer() {0x1}) } // 33 Ohm
                Case(0x4) { Return(Buffer() {0x4}) } // 40 Ohm
              }
            }
          } // End - Switch(Arg2)
        }
      } // End - UUID check
      Return(Buffer() {0x0})
    } // End - _DSM

    Device (CARD) {
      Name (_ADR, 0x00000008)
      Method(_RMV, 0x0, NotSerialized) { Return (0) } // Device cannot be removed
    }
  } // Device(PEMC)

  //
  // SCS SD Card (PCI Mode)
  //
  Device(PSDC) {
    Name(_ADR, 0x00140005)

    Method (_STA) {
      If (LEqual (\SDCE, 0)) {
        //
        // This will prevent OSPM from running
        // methods like _PSx for this device.
        //
        Return (0)
      } Else {
        //
        // Report as fully functional and to be shown in UI.
        //
        Return (0xF)
      }
    }

    Method (_CRS, 0, NotSerialized) {
      //Wake from SD card events while host controller in D3
      Name (SBFI, ResourceTemplate () {
          GpioInt (Edge, ActiveBoth, SharedAndWake, PullNone, 10000, "\\_SB.PCI0.GPI0", 0x00, ResourceConsumer, INTG) {0} // SD CARD DETECT GpioInt
          GpioIo (Shared, PullDefault, 0, 0, IoRestrictionInputOnly, "\\_SB.PCI0.GPI0",,, GPIO) {0} // SD CARD DETECT GpioIo
      })

      CreateWordField (SBFI, INTG._PIN, SDIP)
      CreateWordField (SBFI, GPIO._PIN, SDGP)

      If(LEqual(PCHS, PCHL)) {
        Store (GNUM(GPIO_CNL_LP_VGPIO39), SDIP)
        Store (GNUM(GPIO_CNL_LP_VGPIO39), SDGP)
      } Else {
        Store (GNUM(GPIO_CNL_H_VGPIO6), SDIP)
        Store (GNUM(GPIO_CNL_H_VGPIO6), SDGP)
      }
      Return (SBFI)
    }


    Name(_S0W, 3) // D3hot capable

    // Memory Region to access to the SDHC PCI Configuration Space
    OperationRegion(SCSR, PCI_Config, 0x00, 0x100)
    Field(SCSR, WordAcc, NoLock, Preserve) {
      Offset(R_SCS_CFG_PCS),         // 0x84, PMCSR - Power Management Control and Status
      PSTA,32,
      Offset(R_SCS_CFG_PG_CONFIG),   // 0xA2, Device PG config
          , 2,
      PGEN, 1         // [BIT2] PGE - PG Enable
    }

    //
    // SD PWREN# w/a method for controller entering D3Hot.
    // SD host controller doesn't respect the PWREN# invert setting
    // after it power gates in the D3 state. This causes PWREN# to assert after
    // controller enters D3Hot which might lead to SD card damage when inserted.
    // Work around is to assign ownership to GPIO controller and de-assert the PWREN#.
    //
    // Arg0 - If true switch PWREN# control to GPIO and disable the power
    //
    Method (SDPW, 1, Serialized) {
      If (LNot (\SDPH)) {
        Return ()
      }
      Name (GPIO, 0)
      If (LEqual (PCHS, PCHL)) {
        Store (GPIO_CNL_LP_GPP_A17, GPIO)
      } Else {
        Store (GPIO_CNL_H_GPP_A17, GPIO)
      }

      If (LEqual (Arg0, 0)) {
        // Assign GPIO control back to SD host controller
        // by setting pad mode to native function 1
        SPMV (GPIO, 1)
      } Else {
        // Assign control to GPIO and disable the power to the device
        // This function assumes that the PWREN# is active high since
        // it is the only failing scenario.
        SGOV (GPIO, 0)
        SPMV (GPIO, 0)
      }
    }

    Method(_PS0, 0, Serialized) {
      Store(0, PGEN) // Disable PG

      //
      // On some platforms it has been observed that
      // transactions to SD and eMMC are failing due to
      // the OCP timeout being too low. This timeout doesn't
      // generate any interrupt so software can't react to this failure.
      // We disable the OCP timeout on D0 entry to avoid dropping transactions
      // to SD and eMMC. This write has to be done on D0 entry since power gating in D3
      // will reset it to default state.
      //
      PCRA (PID_SDX, R_SCS_PCR_1C20, 0x0)
      PCRA (PID_SDX, R_SCS_PCR_4820, 0x0)

      And(PSTA, 0xFFFFFFFC, PSTA) // Set BIT[1:0] = 00b - Power State D0
      Store(PSTA, TEMP) // Read Back PMCSR
      SDPW (0)
    }

    Method(_PS3, 0, Serialized) {
      SDPW (1)

      Store(1, PGEN) // Enable PG

      Or(PSTA, 0x3, PSTA) // Set BIT[1:0] = 11b - Power State D3
      Store(PSTA, TEMP) // Read Back PMCSR
    }

    // _DSM x86 Device Specific Method
    // Arg0: UUID Unique function identifier
    // Arg1: Integer Revision Level
    // Arg2: Integer Function Index (0 = Return Supported Functions)
    // Arg3: Package Parameters
    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      If(PCIC(Arg0)) { return(PCID(Arg0,Arg1,Arg2,Arg3)) }

      // Check the UUID
      If(LEqual(Arg0, ToUUID("f6c13ea5-65cd-461f-ab7a-29f7e8d5bd61"))) {
        // Check the revision
        If(LGreaterEqual(Arg1, Zero)) {
          // Switch statement based on the function index.
          Switch(ToInteger(Arg2)) {
            //
            // Function Index 0 the return value is a buffer containing
            // one bit for each function index, starting with zero.
            // Bit 0 - Indicates whether there is support for any functions other than function 0.
            // Bit 1 - Indicates support to clear power control register
            // Bit 2 - Indicates support to set power control register
            // Bit 3 - Indicates support to set 1.8V signalling
            // Bit 4 - Indicates support to set 3.3V signalling
            // Bit 5 - Indicates support for HS200 mode
            // Bit 6 - Indicates support for HS400 mode
            // Bit 9 - Indicates eMMC I/O Driver Strength
            //
            // For SD we have to support functions to
            // set 1.8V signalling and 3.3V signalling [BIT4, BIT3]
            //
            Case(0) {
              Return (Buffer() {0x19})
            }

            //
            // Function Index 3: Set 1.8v signalling.
            // We put a sleep of 100ms in this method to
            // work around a known issue with detecting
            // UHS SD card on PCH. This is to compensate
            // for the SD VR slowness.
            //
            Case(3) {
              Sleep (100) // Sleep 100ms
              Return(Buffer(){0x00})
            }

            //
            // Function Index 4: Set 3.3v signalling.
            // We put a sleep of 100ms in this method to
            // work around a known issue with detecting
            // UHS SD card on PCH. This is to compensate
            // for the SD VR slowness.
            //
            Case(4) {
              Sleep (100) // Sleep 100ms
              Return(Buffer(){0x00})
            }
          } // End - Switch(Arg2)
        }
      } // End - UUID check
      Return(Buffer() {0x0})
    } // End _DSM
  }
}

