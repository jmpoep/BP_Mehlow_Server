/** @file
This file contains doxygen commands definitions for creating
ClientSiliconPkg override documentation

@copyright
  INTEL CONFIDENTIAL
  Copyright 2016 - 2018 Intel Corporation.

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

@par Specification
**/

/**
 @page clientsipkgoverride

  ClientSiliconPkg Override tags (SIPO-xxxx) document issues that Royal Park team has resolved with an override of Royal Park.

  The Active SIPO table lists every issue which is currently resolved with an override. It is a catalog of all such existing overrides.

  The Retired SIPO table lists issues which previously required such overrides. It is purely historical as these overrides have all been deleted (are no longer needed).
<BR>
<CENTER>
<STRONG>Active SIPOs</STRONG>
<BR>in CannonlakeSiliconPkg/Override
</CENTER>

<table border="1" style="width:1200px">
<tr>
  <th>Tag</th>
  <th>Client HSD</th>
  <th>BP HSD</th>
  <th>Status / Planned EOL</th>
  <th>Description</th>
</tr>
  <tr> <td>SIPO-KBL-0001</td> <td></td>    <td></td>        <td></td>           <td>1504005677:Enabling Performance measurement for FSP Binary - Moving PeiFpdtPerformanceLib from ClientCommonPkg to ClientSiliconPkg (as needed by FSP binary) by renaming the library as PerformanceLibFpdt to follow the standard naming</td> </tr>

  <tr> <td>SIPO-KBL-0002</td> <td>ClientHSD</td>    <td>N/A</td>        <td>EOL once HstiFeatureBit.h is taken back to ClientSiliconPkg</td>           <td>1208123075:HSTI to capture the SMM_FEATURE_CONTROL SMM Code access enables. SECT is a feature that enables the trap on SMM code access violation. This violation occurs when there is a code access from SMRAM to outside SMRAM. There are checks in the HSTI to verify if this MSR is locked. However there has to be checks to ensure that the MSR is enabled to be compliant with security checks.HSD is to request a check on the enable bit being set on MSR 4E0 in addition to the lock bit.</td> </tr>
  <tr> <td>SIPO-CNL-0002</td> <td>220816850</td>  <td></td>  <td></td> <td>Shadow PEIMs to reduce S3 resume times.</td></tr>
  <tr> <td>SIPO-CNL-0009</td> <td>2007331427</td> <td>1375669</td>  <td></td> <td>EFI_PEI_MP_SERVICES_PPI crashed when calling in EndOfPei</td></tr>
</table>

<BR>
<CENTER>
<STRONG>Retired SIPOs</STRONG>
<BR>formerly in CannonlakeSiliconPkg/Override
</CENTER>

<table border="1" style="width:1200px">
<tr>
  <th>Tag</th>
  <th>Client HSD</th>
  <th>BP HSD</th>
  <th>Actual EOL</th>
  <th>Description</th>
</tr>
  <tr> <td>SIPO-CNL-0001</td> <td>220545591</td>  <td></td>  <td></td>           <td>CORE</td>       <td>Perform complete initialization when enable AP</td> </tr>
  <tr> <td>SIPO-CNL-0004</td> <td>1604518882</td> <td></td>  <td>Fixed with SIPO-CNL-0006</td> <td>While Running S3 Cycle, System hung at POSTCODE 9C18 is observed.</td></tr>
  <tr> <td>SIPO-CNL-0005</td> <td>1604531888</td> <td></td>  <td>Fixed with SIPO-CNL-0006</td> <td>While Running S4 and Warm-reset Cycle, System hung at POSTCODE 9C18 is observed.</td></tr>
  <tr> <td>SIPO-CNL-0003</td> <td>220818230</td>  <td>1375528</td>  <td>PiSmmCpuDxeSmm override moved to ClientCommonPkg</td> <td>Initialize APs in SMM on S3 resume with only one INIT-SIPI-SIPI instead of 2.</td></tr>
  <tr> <td>SIPO-CNL-0006</td> <td>1604583200</td> <td></td>  <td></td> <td>While Running S3 Cycle, System hung at POSTCODE 9C18 is observed.</td></tr>
  <tr> <td>SIPO-CNL-0007</td> <td>2201619209</td> <td>1375606</td>  <td>PiSmmCpuDxeSmm override moved to ClientCommonPkg</td> <td>If ins/outs is executing on a thread, a page fault will occur if SMI is generated.</td></tr>
  <tr> <td>SIPO-CNL-0008</td> <td>1504706940</td> <td>1375548</td>  <td>PiSmmCpuDxeSmm override moved to ClientCommonPkg</td> <td>Fix Linker illegal text relocation issue for Xcode toolchain.</td></tr>
  </table>

**/

