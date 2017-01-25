/**********************************************************************

Filename    :   SF_SIMD.h
Content     :   Defines and includes for SIMD functionality.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_SF_SIMD_H
#define INC_SF_SF_SIMD_H
#pragma once

#include "Kernel/SF_Types.h"

// Forward defines.
// Vector4f         typedef for 128 bit register for floating point data.
// Vector4i         typedef for 128 bit register for integer data.
// SF_SIMD_ALIGN    required alignment attribute for any data used by SIMD operations.
// IS               typedef for the default SIMD instruction set for a platform. This should
//                  be something that is minimally supported on the platform.

#if defined(SF_ENABLE_SIMD) && defined (SF_OS_WIN32) 
    #include "SF_SIMD_Win32.h"
#elif defined(SF_ENABLE_SIMD) && defined(SF_OS_XBOX360)
    #include "SF_SIMD_X360.h"
#elif defined(SF_ENABLE_SIMD) && defined(SF_OS_PS3)
    #include "SF_SIMD_PS3.h"
#else
    #define SF_SIMD_ALIGN       // No alignment for non-SIMD platforms.
    #ifdef SF_ENABLE_SIMD
        #undef SF_ENABLE_SIMD   // This platform doesn't support SIMD, so force disable it.
    #endif
#endif


#endif // INC_SF_SF_SIMD_H
