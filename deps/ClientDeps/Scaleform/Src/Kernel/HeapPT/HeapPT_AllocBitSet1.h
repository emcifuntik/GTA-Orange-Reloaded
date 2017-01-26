/**********************************************************************

Filename    :   HeapPT_AllocBitSet1.h
Content     :   Bit-set based allocator, 1 bit per block.

Created     :   2009
Authors     :   Maxim Shemanarev

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

#ifndef INC_SF_Kernel_HeapPT_AllocBitSet1_H
#define INC_SF_Kernel_HeapPT_AllocBitSet1_H

#include "HeapPT_FreeBin.h"

namespace Scaleform { namespace HeapPT {

//------------------------------------------------------------------------
class AllocBitSet1
{
public:
    AllocBitSet1(UPInt minAlignShift);

    static UInt32* GetBitSet(const HeapSegment* seg)
    {
        return (UInt32*)(((UByte*)seg) + sizeof(HeapSegment));
    }

    UPInt GetBitSetWords(UPInt dataSize) const
    {
        UPInt blocks = (dataSize + MinAlignMask) >> MinAlignShift;
        return (blocks + 31) >> 5;
    }

    UPInt GetBitSetBytes(UPInt dataSize) const
    {
        return GetBitSetWords(dataSize) * sizeof(UInt32);
    }

    void  InitSegment(HeapSegment* seg);
    void  ReleaseSegment(HeapSegment* seg);

    UPInt AlignSize(UPInt size) const 
    { 
        return (size + MinAlignMask) & ~MinAlignMask; 
    }

    void* Alloc(UPInt size, HeapSegment** allocSeg);
    void  Free(HeapSegment* seg, void* ptr, UPInt size);

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

private:
    SF_INLINE static void clrBit(UInt32* bitSet, UPInt idx)
    {
        bitSet[idx >> 5] &= ~(UInt32(1) << (idx & 31));
    }

    SF_INLINE static void setBit(UInt32* bitSet, UPInt idx)
    {
        bitSet[idx >> 5] |= UInt32(1) << (idx & 31);
    }

    SF_INLINE static void markBusy(UInt32* bitSet, UPInt start, UPInt size)
    {
        setBit(bitSet, start);
        setBit(bitSet, start+size-1);
    }

    SF_INLINE static void markFree(UInt32* bitSet, UPInt start, UPInt size)
    {
        clrBit(bitSet, start);
        clrBit(bitSet, start+size-1);
    }

    SF_INLINE static bool isZero(const UInt32* bitSet, UPInt idx)
    {
        return (bitSet[idx >> 5] & (UInt32(1) << (idx & 31))) == 0;
    }

    UPInt   MinAlignShift;
    UPInt   MinAlignMask;
    FreeBin Bin;
};


}} // Scaleform::Heap

#endif
