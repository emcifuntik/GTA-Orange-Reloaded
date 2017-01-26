/**********************************************************************

Filename    :   Render_SIMD.cpp
Content     :   
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render/Render_SIMD.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_TreeCacheNode.h"

namespace Scaleform { namespace Render {

#ifdef SF_SIMD_SUPPORT

#if defined (SF_OS_WIN32)

// On PC, we need code to detect the features of the CPU, to determine 
// which SIMD implementation should be used.

class CPUCapsX86
{
public:
    static int  CPUInfo[4];
    static bool CPUID_Called;

    CPUCapsX86() { Detect(); }
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

int  CPUCapsX86::CPUInfo[4]   = {0,0,0,0};
bool CPUCapsX86::CPUID_Called = false;
static CPUCapsX86 CPUCaps;

//---------------------------------------------------------------------------------------

class InstructionSet_SSE2
{
public:
    // Loads a value from aligned memory, and returns it.
    static simd128f LoadAligned( const float * p )
    {
        return _mm_load_ps(p);
    }

    // Loads a value from aligned memory, and returns it.
    static simd128i LoadAligned( const simd128i * p )
    {
        return _mm_loadu_si128(p);
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, simd128f v )
    {
        return _mm_store_ps(p, v);
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( simd128i * p, simd128i v )
    {
        return _mm_stream_si128(p, v);
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static simd128f Constant( )
    {
        // When initializing unions which contain arrays (which __m128i is), the the initializers are cast to the first member in the union.
        static simd128i v = { (i0&0xFF000000 >> 24), (i0&0x00FF0000 >> 16), (i0&0x0000FF00 >> 8), (i0&0x000000FF ), 
                              (i1&0xFF000000 >> 24), (i1&0x00FF0000 >> 16), (i1&0x0000FF00 >> 8), (i1&0x000000FF ), 
                              (i2&0xFF000000 >> 24), (i2&0x00FF0000 >> 16), (i2&0x0000FF00 >> 8), (i2&0x000000FF ), 
                              (i3&0xFF000000 >> 24), (i3&0x00FF0000 >> 16), (i3&0x0000FF00 >> 8), (i3&0x000000FF ) };
        return *(simd128f*)&v;
    }

    // Sets the two most significant elements (in r0) to the given values.
    static simd128f SetHigh( simd128f r0, float f0, float f1 )
    {
        r0.m128_f32[2] = f0;
        r0.m128_f32[3] = f1;
        return r0;
    }

    // Sets the two least significant elements (in r0) to the given values.
    static simd128f SetLow( simd128f r0, float f0, float f1)
    {
        r0.m128_f32[0] = f0;
        r0.m128_f32[1] = f1;
        return r0;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static simd128i Set1(UInt16 v)
    {
        return _mm_set1_epi16(v);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static simd128f Shuffle( simd128f r0, simd128f r1 )
    {
        return _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(f0, f1, f2, f3));
    }

    // Interleaves the two low components of r0 and r1.
    static simd128f UnpackLo( simd128f r0, simd128f r1 )
    {
        return _mm_unpacklo_ps(r0, r1);
    }

    // Interleaves the two high components of r0 and r1.
    static simd128f UnpackHi( simd128f r0, simd128f r1 )
    {
        return _mm_unpackhi_ps(r0, r1);
    }

    // Multiplies two input registers and returns the result.
    static simd128f Multiply( simd128f r0, simd128f r1 )
    {
        return _mm_mul_ps(r0, r1);
    }

    // Adds to input registers and returns the result.
    static simd128f Add( simd128f r0, simd128f r1 )
    {
        return _mm_add_ps(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static simd128i Add16( simd128i r0, simd128i r1 )
    {
        return _mm_add_epi16(r0, r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static simd128f MultiplyAdd( simd128f r0, simd128f r1, simd128f r2 )
    {
        return Add( Multiply(r0, r1), r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static simd128f Min( simd128f r0, simd128f r1 )
    {
        return _mm_min_ps(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static simd128f Max( simd128f r0, simd128f r1 )
    {
        return _mm_max_ps(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static simd128f And( simd128f r0, simd128f r1 )
    {
        return _mm_and_ps( r0, r1 );
    }

    // Element-wise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static simd128f CompareGE( simd128f r0, simd128f r1 )
    {
        return _mm_cmpge_ps(r0, r1);
    }

    // Gathers the MSB (sign) bit of each element in r0, and returns them in the LSB
    // four bits of the return. The rest of the return bits are zero.
    static int MoveMask( simd128f r0 )
    {
        return _mm_movemask_ps(r0);
    }
};

#elif defined (SF_OS_XBOX360)

//---------------------------------------------------------------------------------------
#define _PERMUTE_SINGLE_MASK(name, f0, f1, f2, f3) \
    static const __vector4i imask##name = {  \
    ((f0)*4+3) | ((f0)*4 +2) << 8 | ((f0)*4 +1) << 16 | ((f0)*4 +0) << 24, \
    ((f1)*4+3) | ((f1)*4 +2) << 8 | ((f1)*4 +1) << 16 | ((f1)*4 +0) << 24, \
    ((f2)*4+3) | ((f2)*4 +2) << 8 | ((f2)*4 +1) << 16 | ((f2)*4 +0) << 24, \
    ((f3)*4+3) | ((f3)*4 +2) << 8 | ((f3)*4 +1) << 16 | ((f3)*4 +0) << 24 }; \
    static const __vector4& ##name = (__vector4&)imask##name;

class InstructionSet_X360VMX
{
public:
    // Loads a value from aligned memory, and returns it.
    static simd128f LoadAligned( const float * p )
    {
        return *(simd128f*)p;
    }

    // Loads a value from aligned memory, and returns it.
    static simd128i LoadAligned( const simd128i * p )
    {
        return *(p);
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, simd128f v )
    {
        *(simd128f*)p = v;
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( simd128i * p, simd128i v )
    {
        // Note: not actually streaming. __stvx uses the L2 cache.
        *p = v;
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static simd128f Constant( )
    {
        static simd128i c = { i0, i1, i2, i3 };
        return *(simd128f*)&c;
    }

    // Sets the two most significant elements (in r0) to the given values.
    static simd128f SetHigh( simd128f r0, float f0, float f1 )
    {
        r0.vector4_f32[2] = f0;
        r0.vector4_f32[3] = f1;
        return r0;
    }

    // Sets the two least significant elements (in r0) to the given values.
    static simd128f SetLow( simd128f r0, float f0, float f1)
    {
        r0.vector4_f32[0] = f0;
        r0.vector4_f32[1] = f1;
        return r0;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static simd128i Set1(UInt16 v)
    {
        // Better way?
        static const __vector4i mask = { 0x01000100, 0x01000100, 0x01000100, 0x01000100 };
        __vector4i t;
        t[0] = v;
        simd128i r;
        *(simd128f*)&r = __vperm(*(simd128f*)t, __vzero(), *(simd128f*)mask );
        return (r);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static simd128f Shuffle( simd128f r0, simd128f r1 )
    {
        // Counter intuitive reversal of order.
        _PERMUTE_SINGLE_MASK(mask, f3, f2, f1+4, f0+4);
        return __vperm(r0, r1, mask);
    }

    // Interleaves the two low components of r0 and r1.
    static simd128f UnpackLo( simd128f r0, simd128f r1 )
    {
        _PERMUTE_SINGLE_MASK(mask, 0, 4, 1, 5);
        return __vperm(r0, r1, mask);
    }

    // Interleaves the two high components of r0 and r1.
    static simd128f UnpackHi( simd128f r0, simd128f r1 )
    {
        _PERMUTE_SINGLE_MASK(mask, 2, 6, 3, 7 );
        return __vperm(r0, r1, mask);
    }

    // Multiplies two input registers and returns the result.
    static simd128f Multiply( simd128f r0, simd128f r1 )
    {
        return __vmulfp(r0, r1);
    }

    // Adds to input registers and returns the result.
    static simd128f Add( simd128f r0, simd128f r1 )
    {
        return __vaddfp(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static simd128i Add16( simd128i r0, simd128i r1 )
    {
        return *(simd128i*)&__vaddshs(*(simd128f*)&r0, *(simd128f*)&r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static simd128f MultiplyAdd( simd128f r0, simd128f r1, simd128f r2 )
    {
        return __vmaddfp(r0, r1, r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static simd128f Min( simd128f r0, simd128f r1 )
    {
        return __vminfp(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static simd128f Max( simd128f r0, simd128f r1 )
    {
        return __vmaxfp(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static simd128f And( simd128f r0, simd128f r1 )
    {
        return __vand( r0, r1 );
    }

    // Elementwise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static simd128f CompareGE( simd128f r0, simd128f r1 )
    {
        return __vcmpgefp(r0, r1);
    }

    // Gathers the MSB (sign) bit of each element in r0, and returns them in the LSB
    // four bits of the return. The rest of the return bits are zero.
    static int MoveMask( simd128f r0 )
    {
        // X360 doesn't have an easy instruction for this, the following is a little ugly.
        simd128f t0 = __vcmpgefp(r0, __vzero());
        simd128f t1 = __vpkswss(t0, t0);
        simd128f t2 = __vpkswss(t1, t1);
        UINT mask = ~((*(__vector4i*)&t2))[0];
        mask = (mask&0x80000000) >> 31 | (mask&0x00800000) >> 22 | (mask&0x00008000) >> 13 | (mask&0x00000080) >> 4;
        return (int)mask;
    }
};
#elif defined(SF_OS_PS3)

#define _PERMUTE_SINGLE_MASK(name, f0, f1, f2, f3) \
    static const __vector4i imask##name = {  \
    ((f0)*4+3) | ((f0)*4 +2) << 8 | ((f0)*4 +1) << 16 | ((f0)*4 +0) << 24, \
    ((f1)*4+3) | ((f1)*4 +2) << 8 | ((f1)*4 +1) << 16 | ((f1)*4 +0) << 24, \
    ((f2)*4+3) | ((f2)*4 +2) << 8 | ((f2)*4 +1) << 16 | ((f2)*4 +0) << 24, \
    ((f3)*4+3) | ((f3)*4 +2) << 8 | ((f3)*4 +1) << 16 | ((f3)*4 +0) << 24 }; \
    static const __vector4& ##name = (__vector4&)imask##name;

//---------------------------------------------------------------------------------------
class InstructionSet_PS3
{
public:
    // Loads a value from aligned memory, and returns it.
    static simd128f LoadAligned( const float * p )
    {
        return vec_lvx( 0, (simd128f*)p );
    }

    // Loads a value from aligned memory, and returns it.
    static simd128i LoadAligned( const simd128i * p )
    {
        return vec_ld( 0, p );
    }

    // Stores a value to aligned memory.
    static void StoreAligned( float * p, simd128f v )
    {
        vec_st(v, p, 0);
    }

    // Stores a values to aligned memory, without using the cache.
    static void StreamAligned( simd128i * p, simd128i v )
    {
        // Note: not actually streaming.
        vec_st(v, p, 0);
    }

    // Creates a 4 single-precision floating point constant from 4 unsigned integers.
    template< unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3 >
    static simd128f Constant( )
    {
        static simd128i c = { i0, i1, i2, i3 };
        return *(simd128f*)&c;
    }

    // Sets the two most significant elements (in r0) to the given values.
    static simd128f SetHigh( simd128f r0, float f0, float f1 )
    {
        vec_insert( f0, r0, 2);
        vec_insert( f1, r0, 3);
        return r0;
    }

    // Sets the two least significant elements (in r0) to the given values.
    static simd128f SetLow( simd128f r0, float f0, float f1)
    {
        vec_insert( f0, r0, 0);
        vec_insert( f1, r0, 1);
        return r0;
    }

    // Sets 8 16-bit integer values in the register to the input.
    static simd128i Set1(UInt16 v)
    {
        return vec_splat_u16(v);
    }

    // Takes two registers, selects two elements from the first, and two from the second.
    // all template arguments are in the range 0-3, f0 and f1 refer to the elements in r0,
    // f2 and f3 refer to the elements in r1.
    template <int f0, int f1, int f2, int f3>
    static simd128f Shuffle( simd128f r0, simd128f r1 )
    {
        // Counter intuitive reversal of order.
        _PERMUTE_SINGLE_MASK(mask, f3, f2, f1+4, f0+4);
        return vec_perm(r0, r1, mask);
    }

    // Interleaves the two low components of r0 and r1.
    static simd128f UnpackLo( simd128f r0, simd128f r1 )
    {
        return vec_mergel(r0, r1);
    }

    // Interleaves the two high components of r0 and r1.
    static simd128f UnpackHi( simd128f r0, simd128f r1 )
    {
        return vec_mergeh(r0, r1);
    }

    // Multiplies two input registers and returns the result.
    static simd128f Multiply( simd128f r0, simd128f r1 )
    {
        // No muliply instruction - use madd, w/addend = 0.
        static simd128f zero = Constant<0,0,0,0>();
        return vec_madd(r0, r1, zero);
    }

    // Adds to input registers and returns the result.
    static simd128f Add( simd128f r0, simd128f r1 )
    {
        return vec_add(r0, r1);
    }

    // Adds 16-bit integer elements in two registers together
    static simd128i Add16( simd128i r0, simd128i r1 )
    {
        return *(simd128i*)&vec_add(*(__vector signed short *)&r0, *(__vector signed short *)&r1);
    }

    // Multiplies r0 and r1 then adds r2.
    static simd128f MultiplyAdd( simd128f r0, simd128f r1, simd128f r2 )
    {
        return vec_madd(r0, r1, r2 );
    }

    // Computes minimum component-wise values between r0 and r1.
    static simd128f Min( simd128f r0, simd128f r1 )
    {
        return vec_min(r0, r1);
    }

    // Computes maximum component-wise values between r0 and r1.
    static simd128f Max( simd128f r0, simd128f r1 )
    {
        return vec_max(r0, r1);
    }

    // Returns the bitwise and of the two input registers.
    static simd128f And( simd128f r0, simd128f r1 )
    {
        return vec_and( r0, r1 );
    }

    // Elementwise comparison of two registers, returns r0.i >= r1.i ? 0xFFFFFFFF : 0;
    static simd128f CompareGE( simd128f r0, simd128f r1 )
    {
        return vec_cmpge(r0, r1);
    }

    // Gathers the MSB (sign) bit of each element in r0, and returns them in the LSB
    // four bits of the return. The rest of the return bits are zero.
    static int MoveMask( simd128f r0 )
    {
        // PS3 doesn't have an easy instruction for this, the following is a little ugly.
        static simd128f zero = Constant<0,0,0,0>();
        simd128i t0 = vec_cmpge(r0, zero);
        __vector unsigned short t1 = vec_packs(t0, t0);
        __vector unsigned char  t2 = vec_packs(t1, t1);
        UINT mask = ~(vec_extract(*(simd128i*)t2, 0));
        mask = (mask&0x80000000) >> 31 | (mask&0x00800000) >> 22 | (mask&0x00008000) >> 13 | (mask&0x00000080) >> 4;
        return (int)mask;
    }
};

#endif


//---------------------------------------------------------------------------------------
void SIMD::Matrix2F_EncloseTransform(const Matrix2F& m, RectF *pr, const RectF& r)
{
#ifdef SF_STATIC_SIMD
    matrix2F_EncloseTransform<SF_STATIC_SIMD>(m,pr,r);
#elif defined(SF_OS_WIN32)
    // Only support SSE2, which should be available on all modern PCs.
    SF_ASSERT(CPUCaps.HasSSE2());
    matrix2F_EncloseTransform<InstructionSet_SSE2>(m,pr,r);
#endif
}

void SIMD::Matrix2F_SetToAppend(Matrix2F& mr, const Matrix2F& m0, const Matrix2F& m1)
{
#ifdef SF_STATIC_SIMD
    matrix2F_SetToAppend<SF_STATIC_SIMD>(mr,m0,m1);
#elif defined(SF_OS_WIN32)
    // Only support SSE2, which should be available on all modern PCs.
    SF_ASSERT(CPUCaps.HasSSE2());
    matrix2F_SetToAppend<InstructionSet_SSE2>(mr,m0,m1);
#endif
}

void SIMD::Vertex_ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta)
{
#ifdef SF_STATIC_SIMD
    vertex_ConvertIndices<SF_STATIC_SIMD>(pdest, psource, count, delta);
#elif defined(SF_OS_WIN32)
    // Only support SSE2, which should be available on all modern PCs.
    SF_ASSERT(CPUCaps.HasSSE2());
    vertex_ConvertIndices<InstructionSet_SSE2>(pdest, psource, count, delta);
#endif
}


//---------------------------------------------------------------------------------------
// ** Implementations

template< class IS >
void SIMD::matrix2F_EncloseTransform( const Matrix2F& m, RectF *pr, const RectF& r )
{
    simd128f rect   = IS::LoadAligned(&r.x1);    
    simd128f m0     = IS::LoadAligned(m.M[0]);
    simd128f m1     = IS::LoadAligned(m.M[1]);

    simd128f y1y1x1x1 = IS::UnpackLo(rect, rect);
    simd128f y2y2x2x2 = IS::UnpackHi(rect, rect);    
    simd128f add_yyxx = IS::Shuffle<3, 3, 3, 3>(m0, m1);

    simd128f m11_01_10_00 = IS::UnpackLo(m0, m1);

    simd128f m11y1_m01y1_m10x1_m00x1 = IS::Multiply(y1y1x1x1, m11_01_10_00);
    simd128f m11y2_m01y2_m10x2_m00x2 = IS::Multiply(y2y2x2x2, m11_01_10_00);

    simd128f m10x2_m10x1_m00x2_m00x1 = IS::UnpackLo(m11y1_m01y1_m10x1_m00x1, m11y2_m01y2_m10x2_m00x2);
    simd128f m11y2_m11y1_m01y2_m01y1 = IS::UnpackHi(m11y1_m01y1_m10x1_m00x1, m11y2_m01y2_m10x2_m00x2);
    simd128f m10x1_m10x2_m00x1_m00x2 = IS::Shuffle<2,3,0,1>(m10x2_m10x1_m00x2_m00x1, m10x2_m10x1_m00x2_m00x1);

    simd128f yAyBxAxB = IS::Add(m10x2_m10x1_m00x2_m00x1, m11y2_m11y1_m01y2_m01y1);
    simd128f yCyDxCxD = IS::Add(m10x1_m10x2_m00x1_m00x2, m11y2_m11y1_m01y2_m01y1);

    simd128f min0_yyxx = IS::Min(yAyBxAxB, yCyDxCxD);
    simd128f max0_yyxx = IS::Max(yAyBxAxB, yCyDxCxD);

    simd128f min_yyxx = IS::Min(min0_yyxx, IS::Shuffle<2,3,0,1>(min0_yyxx, min0_yyxx));
    simd128f max_yyxx = IS::Max(max0_yyxx, IS::Shuffle<2,3,0,1>(max0_yyxx, max0_yyxx));

    simd128f yxyx       = IS::Shuffle<2,0,2,0>(min_yyxx, max_yyxx);
    simd128f add_yxyx   = IS::Shuffle<2,0,2,0>(add_yyxx, add_yyxx);
    simd128f result     = IS::Add(yxyx, add_yxyx);

    IS::StoreAligned(&pr->x1, result);
}


template< class IS >
void SIMD::matrix2F_SetToAppend( Matrix2F& mr, const Matrix2F& m0, const Matrix2F& m1 )
{
    simd128f c0001 = IS::Constant<0,0,0,0xFFFFFFFF>();
    simd128f m1v0  = IS::LoadAligned(m1.M[0]);
    simd128f m1v1  = IS::LoadAligned(m1.M[1]);
    simd128f m0v0  = IS::LoadAligned(m0.M[0]);
    simd128f m0v1  = IS::LoadAligned(m0.M[1]);

    simd128f res_v0 = IS::And( m1v0, c0001 );
    simd128f res_v1 = IS::And( m1v1, c0001 );

    simd128f t0 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v0, m1v0));
    simd128f t1 = IS::Multiply( m0v0, IS::Shuffle<0,0,0,0>(m1v1, m1v1));

    simd128f t2 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v0, m1v0), t0);
    simd128f t3 = IS::MultiplyAdd( m0v1, IS::Shuffle<1,1,1,1>(m1v1, m1v1), t1);

    IS::StoreAligned(&mr.M[0][0], IS::Add(res_v0, t2));
    IS::StoreAligned(&mr.M[1][0], IS::Add(res_v1, t3));
}

// SSE-Intrinsic version of ConvertIndicies. This is significantly faster,
// on vertex-copy bound app can improve whole-app performance by 10%.
template< class IS >
void SIMD::vertex_ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta)
{
    UInt16* sourceEnd = psource + count;
    UPInt   destAlignBegin = (((UPInt)pdest) + 15) & ~(UPInt)15;
    UPInt   destAlignEnd   = ((UPInt)(pdest + count)) & ~(UPInt)15;

    if (destAlignBegin < destAlignEnd)
    {
        // Copy heading bytes before alignment.
        while(pdest < (UInt16*)destAlignBegin)
            *(pdest++) = (*psource++) + delta;

        // Expand delta into each word.
        simd128i mdelta = IS::Set1( delta );

        do
        {
            simd128i val = IS::LoadAligned((simd128i*)psource);
            // Streaming improves performance here over store,
            // as we typically write to HW index buffer.
            IS::StreamAligned((simd128i*)pdest, IS::Add16(val, mdelta));
            psource += sizeof(simd128i) / sizeof(UInt16);
            pdest   += sizeof(simd128i) / sizeof(UInt16);
        } while(pdest < (UInt16*)destAlignEnd);
    }

    // Copy tail remainder.
    while(psource < sourceEnd)
        *(pdest++) = (*psource++) + delta;
}

// This implementation is roughly +50% faster on X86 then stock Intersects.
// For frogger.swf (Ctrl+G), this improves framerate 895 -> 960fps.
SIMD::FixedSizeArrayRect2F::FixedSizeArrayRect2F() : HalfRect(false) 
{

}

void SIMD::FixedSizeArrayRect2F::PushBack(const RectF& r)
{
#ifdef SF_STATIC_SIMD
    pushBack<SF_STATIC_SIMD>(r);
#elif defined(SF_OS_WIN32)
    // Only support SSE2, which should be available on all modern PCs.
    SF_ASSERT(CPUCaps.HasSSE2());
    pushBack<InstructionSet_SSE2>(r);
#endif
}

bool SIMD::FixedSizeArrayRect2F::Intersects(const RectF& bounds)
{
#ifdef SF_STATIC_SIMD
    return intersects<SF_STATIC_SIMD>(bounds);
#elif defined(SF_OS_WIN32)
    // Only support SSE2, which should be available on all modern PCs.
    SF_ASSERT(CPUCaps.HasSSE2());
    return intersects<InstructionSet_SSE2>(bounds);
#endif
}


template< class IS >
void SIMD::FixedSizeArrayRect2F::pushBack(const RectF& r)
{
    if (HalfRect)
    {
        Rect2F& t = pData[Size-1];
        t.r0 = IS::SetHigh( t.r0, r.x1, r.y1 );
        t.r1 = IS::SetHigh( t.r1, r.x2, r.y2 );
        HalfRect = false;
    }
    else
    {
        // Allow for fast growth; ok since its temp store.
        if (Size == Reserve)
            grow(Size * 2);
        Rect2F& t = pData[Size];
        Size++;
        t.r0 = IS::SetLow( t.r0, r.x1, r.y1 );
        t.r1 = IS::SetLow( t.r1, r.x2, r.y2 );
        HalfRect = true;
    }
}

template< class IS >
bool SIMD::FixedSizeArrayRect2F::intersects(const RectF& bounds)
{
    if (!bounds.IsEmpty())
    {
        simd128f bound_y2x2y1x1 = IS::LoadAligned(&bounds.x1);
        simd128f b_y1x1y1x1 = IS::Shuffle<1,0,1,0>(bound_y2x2y1x1, bound_y2x2y1x1);
        simd128f b_y2x2y2x2 = IS::Shuffle<3,2,3,2>(bound_y2x2y1x1, bound_y2x2y1x1);

        UPInt i = 0;
        UPInt count = GetSize();
        if (HalfRect)
            count--;

        for (i=0; i<count; i++)
        {
            Rect2F* pr = &operator[](i);
            simd128f r1_y1x1_r0_y1x1 = pr->r0;
            simd128f r1_y2x2_r0_y2x2 = pr->r1;

            int mask0 = IS::MoveMask(IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2));
            int mask1 = IS::MoveMask(IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2));
            int commonMask = mask0 | (mask1 << 4);

            if (((commonMask & 0x33) == 0) || ((commonMask & 0xCC) == 0))
                return true;
        }

        if (HalfRect)
        {
            Rect2F* pr = &operator[](i);
            simd128f r1_y1x1_r0_y1x1 = pr->r0;
            simd128f r1_y2x2_r0_y2x2 = pr->r1;

            int mask0 = IS::MoveMask(IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2));
            int mask1 = IS::MoveMask(IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2));
            int commonMask = mask0 | (mask1 << 4);
            if ((commonMask & 0x33) == 0)
                return true;
        }
    }
    return false;
}

void SIMD::FixedSizeArrayRect2F::Clear()
{
    FixedSizeArray<Rect2F>::Clear();
    HalfRect = false;
}

#endif // SF_SIMD_SUPPORT

}} // Scaleform::Render
