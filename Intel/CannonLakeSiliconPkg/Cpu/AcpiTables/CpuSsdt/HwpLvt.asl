/** @file
  This file contains the HWP GPE Handler ASL code.
  Method HL62 should be called by platform ASL SW SCI event .

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

#include "CpuPowerMgmt.h"

DefinitionBlock (
  "HwpLvt.aml",
  "SSDT",
  2,
  "PmRef",
  "HwpLvt",
  0x3000
  )
{
External(\_SB.PR00, DeviceObj)
External(\_SB.PR01, ProcessorObj)
External(\_SB.PR02, ProcessorObj)
External(\_SB.PR03, ProcessorObj)
External(\_SB.PR04, ProcessorObj)
External(\_SB.PR05, ProcessorObj)
External(\_SB.PR06, ProcessorObj)
External(\_SB.PR07, ProcessorObj)
External(\_SB.PR08, ProcessorObj)
External(\_SB.PR09, ProcessorObj)
External(\_SB.PR10, ProcessorObj)
External(\_SB.PR11, ProcessorObj)
External(\_SB.PR12, ProcessorObj)
External(\_SB.PR13, ProcessorObj)
External(\_SB.PR14, ProcessorObj)
External(\_SB.PR15, ProcessorObj)
External(\TCNT, FieldUnitObj)
External(\_SB.CFGD, FieldUnitObj)
External(\_SB.OSCP, IntObj)

Scope(\_GPE) {

  //
  // HLVT : HwP Handler for SCI _L62
  //

  Method(HLVT, 0, Serialized)
  {
    Name(NTVL, 0x83) // NotifyValue
    //
    // If Intel Turbo Boost Max Technology 3.0 is present AND
    // \_SB.OSCP[12] = Platform-Wide OS Capable for Intel Turbo Boost Max Technology 3.0
    //
    If (LAnd(And(\_SB.CFGD, PPM_TURBO_BOOST_MAX), And(\_SB.OSCP, 0x1000))) {
      Store(0x85, NTVL)
    }

    Switch (ToInteger(TCNT)) {
      Case(16) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
        Notify(\_SB.PR07, NTVL)
        Notify(\_SB.PR08, NTVL)
        Notify(\_SB.PR09, NTVL)
        Notify(\_SB.PR10, NTVL)
        Notify(\_SB.PR11, NTVL)
        Notify(\_SB.PR12, NTVL)
        Notify(\_SB.PR13, NTVL)
        Notify(\_SB.PR14, NTVL)
        Notify(\_SB.PR15, NTVL)
      }
      Case(14) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
        Notify(\_SB.PR07, NTVL)
        Notify(\_SB.PR08, NTVL)
        Notify(\_SB.PR09, NTVL)
        Notify(\_SB.PR10, NTVL)
        Notify(\_SB.PR11, NTVL)
        Notify(\_SB.PR12, NTVL)
        Notify(\_SB.PR13, NTVL)
      }
      Case(12) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
        Notify(\_SB.PR07, NTVL)
        Notify(\_SB.PR08, NTVL)
        Notify(\_SB.PR09, NTVL)
        Notify(\_SB.PR10, NTVL)
        Notify(\_SB.PR11, NTVL)
      }
      Case(10) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
        Notify(\_SB.PR07, NTVL)
        Notify(\_SB.PR08, NTVL)
        Notify(\_SB.PR09, NTVL)
      }
      Case(8) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
        Notify(\_SB.PR07, NTVL)
      }
      Case(7) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
        Notify(\_SB.PR06, NTVL)
      }
      Case(6) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
        Notify(\_SB.PR05, NTVL)
      }
      Case(5) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
        Notify(\_SB.PR04, NTVL)
      }
      Case(4) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
        Notify(\_SB.PR03, NTVL)
      }
      Case(3) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
        Notify(\_SB.PR02, NTVL)
      }
      Case(2) {
        Notify(\_SB.PR00, NTVL)
        Notify(\_SB.PR01, NTVL)
      }
      Default {
        Notify(\_SB.PR00, NTVL)
      }
    }
  }
} //end Scope(\_GPE)
}// end of definition block
