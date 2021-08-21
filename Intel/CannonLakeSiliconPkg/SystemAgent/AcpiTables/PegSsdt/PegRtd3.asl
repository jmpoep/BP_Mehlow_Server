/** @file
  This file contains the device definitions of the SystemAgent
  PCIE ACPI Reference Code.

 @copyright
  INTEL CONFIDENTIAL
  Copyright 1999 - 2017 Intel Corporation.

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

#include <PchAccess.h>

Scope (\_SB.PCI0) {

    //
    // Name: RTEN
    // Description: Function to Enable the link for RTD3 [RCTL.L22DT]
    // Input: PEG Index
    // Return: Nothing
    //
    Method(RTEN,1)
    {
      If (LEqual(Arg0, 0))
      {
        Store(1, Q0L0) /// Set L23_Rdy to Detect Transition  (L23R2DT)
        Sleep(16)
        Store(0, Local0)
        /// Wait up to 12 ms for transition to Detect
        While(Q0L0) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (0, P0RM)

        //Program AFEOVR.RXSQDETOVR : Additional Power savings: Set 0:1:0 0xc20 BIT4 = 0 & BIT5 = 0
        Store (0, P0AP)
      }
      ElseIf (LEqual(Arg0, 1))
      {
        Store(1, Q1L0) /// Set L23_Rdy to Detect Transition  (L23R2DT)
        Sleep(16)
        Store(0, Local0)
        /// Wait up to 12 ms for transition to Detect
        While(Q1L0) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (0, P1RM)

        //Program AFEOVR.RXSQDETOVR : Additional Power savings: Set 0:1:1 0xc20 BIT4 = 0 & BIT5 = 0
        Store (0, P1AP)
      }
      ElseIf (LEqual(Arg0, 2))
      {
        Store(1, Q2L0) /// Set L23_Rdy to Detect Transition  (L23R2DT)
        Sleep(16)
        Store(0, Local0)
        /// Wait up to 12 ms for transition to Detect
        While(Q2L0) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (0, P2RM)

        //Program AFEOVR.RXSQDETOVR : Additional Power savings: Set 0:1:2 0xc20 BIT4 = 0 & BIT5 = 0
        Store (0, P2AP)
      }
#ifndef CPU_CFL
      ElseIf (LEqual(Arg0, 3))
      {
        Store(1, Q3L0) /// Set L23_Rdy to Detect Transition  (L23R2DT)
        Sleep(16)
        Store(0, Local0)
        /// Wait up to 12 ms for transition to Detect
        While(Q3L0) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (0, P3RM)

        //Program AFEOVR.RXSQDETOVR : Additional Power savings: Set 0:6:0 0xc20 BIT4 = 0 & BIT5 = 0
        Store (0, P3AP)
      }
#endif
    } // End of Method(RTEN,1)

    //
    // Name: RTDS
    // Description: Function to Disable link for RTD3 [RCTL.L23ER]
    // Input: PEG Index
    // Return: Nothing
    //
    Method(RTDS,1)
    {
      If (LEqual(Arg0, 0))
      {
        /// Set L23_Rdy Entry Request (L23ER)
        Store(1, Q0L2)
        Sleep(16)
        Store(0, Local0)
        While(Q0L2) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (1, P0RM)

        //Program AFEOVR.RXSQDETOVR: Set 0:1:0 0xc20 BIT4 = 1 & BIT5 = 1: 11 = Squelch detector output overridden to 1
        Store (3, P0AP)
      }
      ElseIf (LEqual(Arg0, 1))
      {
        /// Set L23_Rdy Entry Request (L23ER)
        Store(1, Q1L2)
        Sleep(16)
        Store(0, Local0)
        While(Q1L2) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (1, P1RM)

        //Program AFEOVR.RXSQDETOVR: Set 0:1:1 0xc20 BIT4 = 1 & BIT5 = 1: 11 = Squelch detector output overridden to 1
        Store (3, P1AP)
      }
      ElseIf (LEqual(Arg0, 2))
      {
        /// Set L23_Rdy Entry Request (L23ER)
        Store(1, Q2L2)
        Sleep(16)
        Store(0, Local0)
        While(Q2L2) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (1, P2RM)

        //Program AFEOVR.RXSQDETOVR: Set 0:1:2 0xc20 BIT4 = 1 & BIT5 = 1: 11 = Squelch detector output overridden to 1
        Store (3, P2AP)
      }
#ifndef CPU_CFL
      ElseIf (LEqual(Arg0, 3))
      {
        /// Set L23_Rdy Entry Request (L23ER)
        Store(1, Q3L2)
        Sleep(16)
        Store(0, Local0)
        While(Q3L2) {
          If(Lgreater(Local0, 4))
          {
            Break
          }
          Sleep(16)
          Increment(Local0)
        }
        //override "robust squelch mechanism"
        Store (1, P3RM)

        //Program AFEOVR.RXSQDETOVR: Set 0:6:0 0xc20 BIT4 = 1 & BIT5 = 1: 11 = Squelch detector output overridden to 1
        Store (3, P3AP)
      }
#endif
    } // End of Method(RTDS,1)


} // End of Scope (\_SB.PCI0)
