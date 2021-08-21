/**

Copyright (c) 2017, Intel Corporation.

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.


@file InitSpsMe.c

**/

#include <Uefi.h>
#include <Library/IoLib.h>
#include <Library/PchCycleDecodingLib.h>
#include <Library/PmcLib.h>
#include <PchAccess.h>

EFI_STATUS
EFIAPI
SpsMeSecEntrypoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
    UINT32    PchPwrmBase;
    UINT32    Data32;

    PchPwrmBase = PmcGetPwrmBase ();
    Data32 = MmioRead32 (PchPwrmBase + R_PMC_PWRM_PRSTS);
      if (Data32 & B_PMC_PWRM_PRSTS_ME_WAKE_STS) {
      MmioOr32 (PchPwrmBase + R_PMC_PWRM_PRSTS, B_PMC_PWRM_PRSTS_ME_WAKE_STS);
    }

    return EFI_SUCCESS;
}
