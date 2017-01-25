/**********************************************************************

Filename    :   HeapPT_Bookkeeper.h
Content     :   Bookkeeping allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Internal allocator used to store bookkeeping information.

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_HeapPT_Bookkeeper_H
#define INC_SF_Kernel_HeapPT_Bookkeeper_H

#include "../SF_List.h"
#include "../SF_SysAlloc.h"
#include "HeapPT_PageTable.h"
#include "HeapPT_AllocBitSet1.h"

namespace Scaleform { namespace HeapPT {

using namespace Heap;

// ***** Bookkeeper
//
//  Internal allocator used to store bookkeeping information.
//------------------------------------------------------------------------
class Bookkeeper
{
public:
    typedef List<HeapSegment> SegmentListType;

    Bookkeeper(SysAllocPaged* sysAlloc, UPInt granularity = Heap_PageSize);

    SysAllocPaged* GetSysAlloc() { return pSysAlloc; }

    void*  Alloc(UPInt size);
    void   Free(void* ptr, UPInt size);

    void   VisitMem(MemVisitor* visitor, unsigned flags) const;
    void   VisitSegments(SegVisitor* visitor) const;

    UPInt  GetFootprint() const { return Footprint; }
    UPInt  GetUsedSpace() const { return Footprint - Allocator.GetTotalFreeSpace(); }

private:
    UPInt           getHeaderSize(UPInt dataSize) const;
    HeapSegment*    allocSegment(UPInt size);
    void            freeSegment(HeapSegment* seg);

    SysAllocPaged*      pSysAlloc;
    UPInt               Granularity;
    SegmentListType     SegmentList;
    AllocBitSet1        Allocator;
    UPInt               Footprint;
};

}} //Scaleform::Heap

#endif
