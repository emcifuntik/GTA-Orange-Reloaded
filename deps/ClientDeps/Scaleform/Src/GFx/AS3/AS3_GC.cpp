/**********************************************************************

Filename    :   AS3_GC.cpp
Content     :   
Created     :   Jan, 2010
Authors     :   Artem Bolgar, Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "AS3_GC.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
//
ASRefCountCollector::ASRefCountCollector()
{
    FrameCnt            = 0;
    PeakRootCount       = 0;
    LastRootCount       = 0;
    LastCollectedRoots  = 0;
    LastPeakRootCount   = 0;
    TotalFramesCount    = 0;
    LastCollectionFrameNum = 0;
    CollectionScheduledFlag = false;
    
    SetParams(~0u, ~0u);
}

void ASRefCountCollector::SetParams(unsigned frameBetweenCollections, unsigned maxRootCount)
{
    // max num of roots before collection
    if (frameBetweenCollections != ~0u)
        MaxFramesBetweenCollections = frameBetweenCollections;
    else
        MaxFramesBetweenCollections = 0; // off by default
    
    // force collection for every N-frames
    if (maxRootCount != ~0u)
        PresetMaxRootCount  = MaxRootCount = maxRootCount;
    else
        PresetMaxRootCount  = MaxRootCount = 1000;
}

//////////////////////////////////////////////////////////////////////////
// GASRefCountCollection
//
#ifdef SF_BUILD_DEBUG
//#define SF_TRACE_COLLECTIONS
#endif

void ASRefCountCollector::AdvanceFrame(unsigned* movieFrameCnt, unsigned* movieLastCollectFrame)
{
    // Is this the first advance since a collection by a different MovieView?
    if (*movieLastCollectFrame != LastCollectionFrameNum)
    {
        *movieLastCollectFrame = LastCollectionFrameNum;
        *movieFrameCnt = 1;
        return;
    }

    // Make sure we don't collect multiple times per frame
    // in the case where several views share the same GC
    if (*movieFrameCnt < FrameCnt)
    {
        // the calling MovieView is advancing the GC during a frame where
        // the GC has already been advanced more times by different MovieView
        ++(*movieFrameCnt);
        return;
    }

    unsigned curRootCount = (unsigned)GetRootsCount();
    // Do collection only every 10th frame for now
    ++TotalFramesCount;
    ++FrameCnt;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);

    // Collection occurs if:
    // 1) if number of root exceeds currently set MaxRootCount;
    // 2) if MaxFramesBetweenCollections is set to value higher than 0 and the
    //    frame counter (FrameCnt) exceeds this value, and number of roots
    //    exceeds PresetMaxRootCount.
    if ((PresetMaxRootCount != 0 && curRootCount > MaxRootCount) || 
        (MaxFramesBetweenCollections != 0 && 
        FrameCnt >= MaxFramesBetweenCollections && 
        curRootCount > PresetMaxRootCount))
    {
        ASRefCountCollector::Stats stats;
        Collect(&stats);

#ifdef SF_TRACE_COLLECTIONS        
        printf("Collect! Roots %d, MaxRoots %d, Peak %d, Roots collected %d, frames between %d ", 
            curRootCount, MaxRootCount, PeakRootCount, stats.RootsFreedTotal,
            TotalFramesCount - LastCollectionFrameNum);
#endif

        // If number of roots exceeds the preset max root count then we need to reset the PeakRootCount
        // in order to decrease currently set MaxRootCount.
        if (stats.RootsFreedTotal > PresetMaxRootCount)
        {
            PeakRootCount = curRootCount; // reset peak count
            MaxRootCount = PresetMaxRootCount;
        }

        // MaxRootCount has been updated every collection event
        //MaxRootCount = Alg::Max(PresetMaxRootCount, PeakRootCount - stats.RootsFreedTotal);
        MaxRootCount = Alg::Max(MaxRootCount, curRootCount - stats.RootsFreedTotal);

        if (PeakRootCount < (unsigned)(MaxRootCount * 0.7))
            MaxRootCount = (unsigned)(MaxRootCount * 0.7);

#ifdef SF_TRACE_COLLECTIONS        
        SF_ASSERT((int)MaxRootCount >= 0);
        printf("new maxroots %d\n", MaxRootCount);
#endif

        LastCollectionFrameNum = TotalFramesCount;

        FrameCnt          = 0;
        LastPeakRootCount = PeakRootCount;
        LastCollectedRoots= stats.RootsFreedTotal;
    }
    LastRootCount = curRootCount;
    *movieFrameCnt = FrameCnt;
    *movieLastCollectFrame = LastCollectionFrameNum;
}

void ASRefCountCollector::ForceCollect()
{
    unsigned curRootCount = (unsigned)GetRootsCount();

    ASRefCountCollector::Stats stats;
    Collect(&stats);

#ifdef SF_TRACE_COLLECTIONS        
    printf("Forced collect! Roots %d, MaxRoots %d, Peak %d, Roots collected %d\n", 
        curRootCount, MaxRootCount, PeakRootCount, stats.RootsFreedTotal);
#endif
    FrameCnt  = 0;
    PeakRootCount = Alg::Max(PeakRootCount, curRootCount);
    LastRootCount = curRootCount;
}

void ASRefCountCollector::ForceEmergencyCollect()
{
    ForceCollect();

    // DO NOT shrink roots, if this was called while in adding roots in Release
    if (!IsAddingRoot())
        ShrinkRoots();

    // Reset peak and max root counters.
    PeakRootCount = 0;
    MaxRootCount  = PresetMaxRootCount;
}


void ForEachChild_GC(const Value& v, RefCountBaseGC<Mem_Stat>::GcOp op)
{
    switch(v.GetKind())
    {
    case Value::kUndefined: // = 0
    case Value::kBoolean:   // = 1
    case Value::kInt:       // = 2
    case Value::kUInt:      // = 3
    case Value::kNumber:    // = 4
    case Value::kString:    // = 5
    case Value::kNamespace: // = 10
    case Value::kThunk:     // = 11
    case Value::kMethodInd: // = 13
        break;
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
    case Value::kThunkFunction: // = 12
        if (v.GetObject())
            (*op)((const AS3::RefCountBaseGC<Mem_Stat>**)v.AsObjectPtrPtr());

        break;
    case Value::kThunkClosure:
        if (v.GetClosure())
            (*op)((const AS3::RefCountBaseGC<Mem_Stat>**)v.AsClosurePtrPtr());

        break;
    case Value::kMethodClosure:
        if (v.GetClosure())
            (*op)((const AS3::RefCountBaseGC<Mem_Stat>**)v.AsClosurePtrPtr());

        if (v.GetFunct())
            (*op)((const AS3::RefCountBaseGC<Mem_Stat>**)v.AsObjectPtrPtr());

        break;
    default:
        break;
    }
}

void ForEachChild_GC(const ValueArray& v, RefCountBaseGC<Mem_Stat>::GcOp op)
{
    for(UPInt i = 0; i < v.GetSize(); ++i)
        AS3::ForEachChild_GC(v[i], op);
}

void ForEachChild_GC(const ValueArrayDH& v, RefCountBaseGC<Mem_Stat>::GcOp op)
{
    for(UPInt i = 0; i < v.GetSize(); ++i)
        AS3::ForEachChild_GC(v[i], op);
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


