/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/

/**

Copyright (c) 2014 - 2017 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

@file:

  MeTypeLib.h

@brief:

  HECI Library

**/
#ifndef _ME_TYPE_DEFS_H_
#define _ME_TYPE_DEFS_H_

// definitions for use in hfr files
#define METYPE_SPS      0x01
#define METYPE_SPS_EPO  0x02
#define METYPE_AMT      0x03
#define METYPE_DFX      0x0F
#define METYPE_UNDEF    0xEE
#define METYPE_DISABLED 0xFF


// if METYPE == METYPE_SPS
#define MESUBTYPE_SPS_TYPE_E5          0x01
#define MESUBTYPE_SPS_TYPE_E3          0x02
#define MESUBTYPE_SPS_TYPE_SV          0x03

// if METYPE == METYPE_SPS_EPO
#define MESUBTYPE_SPS_TYPE_EPO         0x01

// if METYPE == METYPE_AMT
#define MESUBTYPE_AMT_TYPE_CLIENT      0x01
#define MESUBTYPE_AMT_TYPE_HEDT        0x02
#define MESUBTYPE_AMT_TYPE_WS          0x03


#endif // _ME_TYPE_DEFS_H_
