/**@file

 @copyright
  INTEL CONFIDENTIAL
  Copyright 2017 Intel Corporation.

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

#include "Register/PchRegsClk.h"

Scope(\_SB)
{
  // IsCLK PCH register for clock settings
  OperationRegion (ICLK, SystemMemory, Add(SBRG, Add(ShiftLeft(PID_ICLK, 16), R_ICLK_PCR_CAMERA1)), 0x82)
  Field(ICLK,AnyAcc,Lock,Preserve)
  {
    CLK1, 8,
    Offset(0x80),
    CLK2, 8,
  }

  //
  // Number Of Clocks
  //
  Method(NCLK, 0x0, NotSerialized)
  {
    Return (2) // IMGCLKOUT_0, IMGCLKOUT_1
  }

  //
  // Clock Control
  //
  Method(CLKC, 0x2, NotSerialized)
  {
    //
    // Arg0 - Clock number (0:IMGCLKOUT_0, etc)
    // Arg1 - Desired state (0:Disable, 1:Enable)
    //
    Switch(Arg0)
    {
      Case (0)
      {
        Store(CLK1, Local0)
        Store(Or(And(Local0, Not(B_ICLK_PCR_REQUEST)), ShiftLeft(Arg1, 1)), CLK1)
      }
      Case (1)
      {
        Store(CLK2, Local0)
        Store(Or(And(Local0, Not(B_ICLK_PCR_REQUEST)), ShiftLeft(Arg1, 1)), CLK2)
      }
    }
  }

  //
  // Clock Frequency
  //
  Method(CLKF, 0x2, NotSerialized)
  {
    //
    // Arg0 - Clock number (0:IMGCLKOUT_0, etc)
    // Arg1 - Clock frequency (0:24MHz, 1:19.2MHz)
    //
    Switch(Arg0)
    {
      Case (0)
      {
        Store(CLK1, Local0)
        Store(Or(And(Local0, Not(B_ICLK_PCR_FREQUENCY)), Arg1), CLK1)
      }
      Case (1)
      {
        Store(CLK2, Local0)
        Store(Or(And(Local0, Not(B_ICLK_PCR_FREQUENCY)), Arg1), CLK2)
      }
    }
  }
} // \_SB