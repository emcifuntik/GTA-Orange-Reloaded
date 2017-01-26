/**********************************************************************

PublicHeader:   Kernel
Filename    :   Math.h
Content     :   Math inline functions and macro definitions
Created     :   July 7, 2005
Authors     :   Michael Antonov, Maxim Shemanarev
Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_Math_H
#define INC_SF_Kernel_Math_H

#include "SF_Types.h"

#ifdef SF_MATH_H
#include SF_MATH_H
#elif defined(SF_OS_WINCE)
#include <cmath>
#elif defined(SF_CC_RENESAS)
#include <math.h>
#include <mathf.h>
#else
#include <math.h>
#include <stdlib.h>
#endif


// *** Useful math constants

#define SF_MATH_EPSILON                ((Double)1.0e-12)                   // epsilon 
#define SF_MATH_E                      ((Double)2.7182818284590452354)     // e 
#define SF_MATH_LOG2E                  ((Double)1.4426950408889634074)     // log_2 e 
#define SF_MATH_LOG10E                 ((Double)0.43429448190325182765)    // log_10 e 
#define SF_MATH_LN2                    ((Double)0.69314718055994530942)    // log_e 2 
#define SF_MATH_LN10                   ((Double)2.30258509299404568402)    // log_e 10 
#define SF_MATH_PI                     ((Double)3.14159265358979323846)    // pi 
#define SF_MATH_PI_2                   ((Double)1.57079632679489661923)    // pi/2 
#define SF_MATH_PI_3                   ((Double)1.04719755119659774615)    // pi/3 
#define SF_MATH_PI_4                   ((Double)0.78539816339744830962)    // pi/4 
#define SF_MATH_1_PI                   ((Double)0.31830988618379067154)    // 1/pi 
#define SF_MATH_2_PI                   ((Double)0.63661977236758134308)    // 2/pi 
#define SF_MATH_2_SQRTPI               ((Double)1.12837916709551257390)    // 2/sqrt(pi) 
#define SF_MATH_SQRT2                  ((Double)1.41421356237309504880)    // sqrt(2) 
#define SF_MATH_SQRT1_2                ((Double)0.70710678118654752440)    // 1/sqrt(2) 
#define SF_MATH_DEGTORAD               ((Double)0.01745329251994329577)    // pi/180 
#define SF_MATH_TAN_PI_8               ((Double)0.41421356237309504880)    // tan(pi/8)
#define SF_MATH_SIN_PI_4               ((Double)0.70710678118654752440)    // sin(pi/4)

// Degrees to Radians
#define SF_DEGTORAD(d)                 ((d)*(SF_MATH_PI)/((Double)180.0))
#define SF_RADTODEG(r)                 ((r)*((Double)180.0)/(SF_MATH_PI))

#endif // INC_SF_Kernel_Math_H
