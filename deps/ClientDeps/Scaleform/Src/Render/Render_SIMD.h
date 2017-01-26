/**********************************************************************

Filename    :   Render_SIMD.h
Content     :   SIMD optimized functions.
Created     :   Dec 2010
Authors     :   Bart Muzzin

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_SIMD_H
#define INC_SF_Render_SIMD_H
#pragma once

#include "Kernel/SF_SIMD.h"
#include "Render/Render_TreeCacheNode.h"

namespace Scaleform { namespace Render {

#ifdef SF_SIMD_SUPPORT

// Forward declarations
template<class T> class Matrix2x4;
typedef Matrix2x4<float> Matrix2F;
template<class T> class Rect;
typedef Rect<float> RectF;

SF_SIMD_ALIGN struct Rect2F
{
    simd128f r0;
    simd128f r1;
};

// This class contains 'generic' SIMD implementations of functions, based on a baseline of functionality,
// available through a InstructionSet policy. You may specialize a function for a given instruction set, 
// by creating a template specialization of the given function. eg:
// template<> void SIMD<InstructionSet_SSE2>::matrix2F_EncloseTransform(const Matrix2F& m, RectF *pr, const RectF& r) const
// {
//      // implementation.
// }
// Doing this may be desirable, based on the availability of extra instructions on a given platform.

class SIMD
{
public:
    // Matrix2F
    static void Matrix2F_EncloseTransform(const Matrix2F& m, RectF *pr, const RectF& r);
    static void Matrix2F_SetToAppend(Matrix2F& mr, const Matrix2F& m0, const Matrix2F& m1);

    // Vertex
    static void Vertex_ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);

    // TreeCacheNode
    class FixedSizeArrayRect2F : private FixedSizeArray<Rect2F>
    {
        bool HalfRect;
    public:
        FixedSizeArrayRect2F();
        void PushBack(const RectF& r);
        bool Intersects(const RectF& bounds);
        void Clear();
    private:
        template<class IS> void pushBack(const RectF& r);
        template<class IS> bool intersects(const RectF& bounds);
    };

private:
    template<class IS> static void matrix2F_EncloseTransform(const Matrix2F& m, RectF *pr, const RectF& r);
    template<class IS> static void matrix2F_SetToAppend(Matrix2F& mr, const Matrix2F& m0, const Matrix2F& m1);
    template<class IS> static void vertex_ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);

};

#endif // SF_SIMD_SUPPORT

}}

#endif // INC_SF_Render_SIMD_H
