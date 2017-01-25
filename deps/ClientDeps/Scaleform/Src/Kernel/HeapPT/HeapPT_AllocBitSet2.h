/**********************************************************************

Filename    :   HeapPT_AllocBitSet2.h
Content     :   Bit-set based allocator, 2 bits per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Kernel_HeapPT_AllocBitSet2_H
#define INC_SF_Kernel_HeapPT_AllocBitSet2_H

#include "HeapPT_FreeBin.h"
#include "../SF_HeapBitSet2.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
class AllocBitSet2
{
public:
    AllocBitSet2(UPInt minAlignShift);

    void Reset() { Bin.Reset(); }

    static UInt32* GetBitSet(const HeapSegment* seg)
    {
        return (UInt32*)(((UByte*)seg) + sizeof(HeapSegment));
    }

    UPInt GetBitSetWords(UPInt dataSize) const
    {
        return BitSet2::GetBitSetSize(dataSize, MinAlignShift);
    }

    UPInt GetBitSetBytes(UPInt dataSize) const
    {
        return GetBitSetWords(dataSize) * sizeof(UInt32);
    }

    void  InitSegment(HeapSegment* seg);
    void  ReleaseSegment(HeapSegment* seg);

    void* Alloc(UPInt size, HeapSegment** allocSeg);
    void* Alloc(UPInt size, UPInt alignSize, HeapSegment** allocSeg);

    void  Free(HeapSegment* seg, void* ptr);
    void* ReallocInPlace(HeapSegment* seg, void* oldPtr, 
                         UPInt newSize, UPInt* oldSize);

    UPInt GetUsableSize(const HeapSegment* seg, const void* ptr) const;
    UPInt GetAlignShift(const HeapSegment* seg, const void* ptr, UPInt size) const;

    UPInt GetTotalFreeSpace() const
    {
        return Bin.GetTotalFreeSpace(MinAlignShift);
    }

    void VisitMem(MemVisitor* visitor, MemVisitor::Category cat) const
    {
        Bin.VisitMem(visitor, MinAlignShift, cat);
    }

    void VisitUnused(SegVisitor* visitor, unsigned cat) const
    {
        Bin.VisitUnused(visitor, MinAlignShift, cat);
    }

    void CheckIntegrity() const
    {
        Bin.CheckIntegrity(MinAlignShift);
    }

private:
    UPInt   MinAlignShift;
    UPInt   MinAlignMask;
    FreeBin Bin;
};

}} // Scaleform::Heap


#endif
