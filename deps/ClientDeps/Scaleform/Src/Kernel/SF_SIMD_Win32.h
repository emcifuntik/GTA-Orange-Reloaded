/**********************************************************************

Filename    :   SF_SIMD_Win32.h
Content     :   SIMD instruction set(s) for Win32.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_SF_SIMD_WIN32_H
#define INC_SF_SF_SIMD_WIN32_H
#pragma once

#if defined(SF_ENABLE_SIMD) && defined(SF_OS_WIN32)

#include <emmintrin.h>
#include <intrin.h>

#pragma warning (disable : 4324) // warning C4324: 'struct' : structure was padded due to __declspec(align())
#define SF_SIMD_ALIGN __declspec(align(16)) 

namespace Scaleform { namespace SIMD { 

typedef __m128 Vector4f;
typedef __m128i Vector4i;

// Separated, because there may be multiple instructions sets (but not currently).
namespace SSE { 

class InstructionSet
{
public:
    // Loads a value from aligned memory, and returns it.
    static Vector4f LoadAligned( const float * p )
    {
        return _mm_load_ps(p);
    }

    // Loads a value from aligned memory, and returns it.
    static Vector4i LoadAligned( const Vector4i * p )
    {
        return _mm_loadu_si128(p);
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, Vector4f v )
    {
        return _mm_store_ps(p, v);
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( Vector4i * p, Vector4i v )
    {
        return _mm_stream_si128(p, v);
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static Vector4f Constant( )
    {
        // When initializing unions which contain arrays (which __m128i is), the the initializers are cast to the first member in the union.
        static Vector4i v = { (i0&0xFF000000 >> 24), (i0&0x00FF0000 >> 16), (i0&0x0000FF00 >> 8), (i0&0x000000FF ), 
            (i1&0xFF000000 >> 24), (i1&0x00FF0000 >> 16), (i1&0x0000FF00 >> 8), (i1&0x000000FF ), 
            (i2&0xFF000000 >> 24), (i2&0x00FF0000 >> 16), (i2&0x0000FF00 >> 8), (i2&0x000000FF ), 
            (i3&0xFF000000 >> 24), (i3&0x00FF0000 >> 16), (i3&0x0000FF00 >> 8), (i3&0x000000FF ) };
        return *(Vector4f*)&v;
    }

    // Sets the two most significant elements (in r0) to the given values.
    static Vector4f SetHigh( Vector4f r0, float f0, float f1 )
    {
        r0.m128_f32[2] = f0;
        r0.m128_f32[3] = f1;
        return r0;
    }

    // Sets the two least significant elements (in r0) to the given values.
    static Vector4f SetLow( Vector4f r0, float f0, float f1)
    {
        r0.m128_f32[0] = f0;
        r0.m128_f32[1] = f1;
        return r0;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static Vector4i Set1(UInt16 v)
    {
        return _mm_set1_epi16(v);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static Vector4f Shuffle( Vector4f r0, Vector4f r1 )
    {
        return _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(f0, f1, f2, f3));
    }

    // Interleaves the two low components of r0 and r1.
    static Vector4f UnpackLo( Vector4f r0, Vector4f r1 )
    {
        return _mm_unpacklo_ps(r0, r1);
    }

    // Interleaves the two high components of r0 and r1.
    static Vector4f UnpackHi( Vector4f r0, Vector4f r1 )
    {
        return _mm_unpackhi_ps(r0, r1);
    }

    // Multiplies two input registers and returns the result.
    static Vector4f Multiply( Vector4f r0, Vector4f r1 )
    {
        return _mm_mul_ps(r0, r1);
    }

    // Adds to input registers and returns the result.
    static Vector4f Add( Vector4f r0, Vector4f r1 )
    {
        return _mm_add_ps(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static Vector4i Add16( Vector4i r0, Vector4i r1 )
    {
        return _mm_add_epi16(r0, r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static Vector4f MultiplyAdd( Vector4f r0, Vector4f r1, Vector4f r2 )
    {
        return Add( Multiply(r0, r1), r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static Vector4f Min( Vector4f r0, Vector4f r1 )
    {
        return _mm_min_ps(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static Vector4f Max( Vector4f r0, Vector4f r1 )
    {
        return _mm_max_ps(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static Vector4f And( Vector4f r0, Vector4f r1 )
    {
        return _mm_and_ps( r0, r1 );
    }

    // Element-wise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static Vector4f CompareGE( Vector4f r0, Vector4f r1 )
    {
        return _mm_cmpge_ps(r0, r1);
    }

    // Gathers the MSB (sign) bit of each element in r0, and returns them in the LSB
    // four bits of the return. The rest of the return bits are zero.
    static int MoveMask( Vector4f r0 )
    {
        return _mm_movemask_ps(r0);
    }
};

} // SSE

// On PC, we need code to detect the features of the CPU, to determine 
// which SIMD implementation should be used.
class CPUCaps
{
public:
    static int  CPUInfo[4];
    static bool CPUID_Called;

    CPUCaps() { Detect(); }
    static bool CPUDectected() { return CPUID_Called; }
    static void DetectImpl()
    {
        __cpuid(CPUInfo,1);
        CPUID_Called = true;
    }

    static void Detect()
    {
        if (!CPUDectected())
            DetectImpl();
    }

    // 26th bit is SSE2.
    static bool HasSSE2() { return (CPUInfo[3] & (1 << 25)) != 0; }
};

extern CPUCaps CPUCaps;


// Define the default instruction set.
typedef SSE::InstructionSet IS;

}} // Scaleform::SIMD

#endif // SF_ENABLE_SIMD && SF_OS_WIN32

#endif // INC_SF_SF_SIMD_WIN32_H