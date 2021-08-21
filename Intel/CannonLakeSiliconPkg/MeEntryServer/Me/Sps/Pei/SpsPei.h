/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/

/*++

Copyright (c) 2009-2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

File Name:

  SpsPei.h

Abstract:

  General SPS PEI definitions

--*/
#ifndef _SPSPEI_H_
#define _SPSPEI_H_

#define LPC_BUS       0
#define LPC_DEV       31
#define LPC_FUN       0


#define WDT_ADDRESS_OFFSET 0x54
//
// ICC Watchdog Timer bit definitions
//
#define OC_WDT_RLD            (1 << 31)
#define OC_WDT_ICCSURV_STS    (1 << 25)
#define OC_WDT_NO_ICCSURV_STS (1 << 24)
#define OC_WDT_EN             (1 << 14)
#define OC_WDT_ICCSURV        (1 << 13)
#define OC_WDT_CTL_LCK        (1 << 12)
#define OC_WDT_CTL_TOV_MASK   (0x3FF)
#define OC_WDT_FAILURE_STS    (1 << 23)
#define OC_WDT_UNXP_RESET_STS (1 << 22)
#define OC_WDT_AFTER_POST     (0x3F0000)


EFI_STATUS
UpdateTheTimeStampCounteronHOB (  
  CONST EFI_PEI_SERVICES  **PeiServices, 
  UINT8                   PeiPhase 
  );

UINT64 
EFIAPI
ReadTimeStampCounter (
   VOID
   );
  
#endif // _SPSPEI_H_

