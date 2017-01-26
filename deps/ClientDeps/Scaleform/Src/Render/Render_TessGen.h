/**********************************************************************

Filename    :   Render_TessGen.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/
#ifndef SF_Render_TessGen_H
#define SF_Render_TessGen_H

#include "Render_Containers.h"
#include "Render_Tessellator.h"
#include "Render_Stroker.h"
#include "Render_StrokerAA.h"
#include "Render_Hairliner.h"
#include "Render_ToleranceParams.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
struct MeshGenerator
{
    LinearHeap      Heap1;
    LinearHeap      Heap2;
    LinearHeap      Heap3;
    LinearHeap      Heap4;
    Tessellator     mTess;
#ifdef SF_RENDER_ENABLE_STROKER
    Stroker         mStroker;
    StrokeSorter    mStrokeSorter;
#ifdef SF_RENDER_ENABLE_HAIRLINER
    Hairliner       mHairliner;
#endif
#ifdef SF_RENDER_ENABLE_STROKERAA
    StrokerAA       mStrokerAA;
#endif
#endif

    MeshGenerator(MemoryHeap* h) :
        Heap1(h),
        Heap2(h),
        Heap3(h),
        Heap4(h),
        mTess(&Heap1, &Heap2)
#ifdef SF_RENDER_ENABLE_STROKER
        ,mStroker(&Heap3)
        ,mStrokeSorter(&Heap4)
#ifdef SF_RENDER_ENABLE_HAIRLINER
        ,mHairliner(&Heap3)
#endif
#ifdef SF_RENDER_ENABLE_STROKERAA
        ,mStrokerAA(&Heap3)
#endif
#endif
    {
    }

    void Clear()
    {
        mTess.Clear();
#ifdef SF_RENDER_ENABLE_STROKER
        mStroker.Clear();
        mStrokeSorter.Clear();
#ifdef SF_RENDER_ENABLE_HAIRLINER
        mHairliner.Clear();
#endif
#ifdef SF_RENDER_ENABLE_STROKERAA
        mStrokerAA.Clear();
#endif
#endif
        Heap1.ClearAndRelease();
        Heap2.ClearAndRelease();
        Heap3.ClearAndRelease();
        Heap4.ClearAndRelease();
    }
};


//--------------------------------------------------------------------
struct StrokeGenerator
{
    LinearHeap      Heap1;
    LinearHeap      Heap2;
#ifdef SF_RENDER_ENABLE_STROKER
    Stroker         mStroker;
    StrokeSorter    mStrokeSorter;
#endif
    VertexPath      mPath;

    StrokeGenerator(MemoryHeap* h) : Heap1(h), Heap2(h)
#ifdef SF_RENDER_ENABLE_STROKER
        ,mStroker(&Heap1), mStrokeSorter(&Heap2),
#endif
        mPath(&Heap1)
    {}

    void Clear()
    {
#ifdef SF_RENDER_ENABLE_STROKER
        mStroker.Clear();
        mStrokeSorter.Clear();
#endif
        Heap1.ClearAndRelease();
        Heap2.ClearAndRelease();
    }
};


}} // Scaleform::Render

#endif
