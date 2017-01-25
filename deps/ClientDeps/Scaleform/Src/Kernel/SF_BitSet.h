/**********************************************************************

PublicHeader:   None
Filename    :   BitSet.h
Content     :   Template implementation for a simple BitSet
Created     :   Feb, 2010
Authors     :   Artem Bolgar
Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_BitSet_H
#define INC_SF_Kernel_BitSet_H

#include "SF_Allocator.h"

#undef new

namespace Scaleform {

// A simple non-expandable basic bitset class
template <class Allocator>
class FixedBitSetBase
{
public:
    FixedBitSetBase(const void* pheapAddr, unsigned bitsCount)
        : BitsCount(bitsCount)
    {
        unsigned dataLen = (BitsCount + 7)/8;
        pData = (UByte*)Allocator::Alloc(pheapAddr, dataLen, __FILE__, __LINE__);
        memset(pData, 0, dataLen);
    }
    ~FixedBitSetBase()
    {
        Allocator::Free(pData);
    }

    void Set(unsigned bitIndex) 
    { 
        SF_ASSERT(bitIndex < BitsCount);
        pData[bitIndex >> 3] |= (1 << (bitIndex & 3));
    }

    void Set(unsigned bitIndex, bool s) 
    { 
        (s) ? Set(bitIndex) : Clear(bitIndex);
    }

    void Clear(unsigned bitIndex) 
    { 
        SF_ASSERT(bitIndex < BitsCount);
        pData[bitIndex >> 3] &= ~(1 << (bitIndex & 3));
    }

    bool IsSet(unsigned bitIndex) const
    { 
        SF_ASSERT(bitIndex < BitsCount);
        return (pData[bitIndex >> 3] & (1 << (bitIndex & 3))) != 0;
    }
    bool operator[](unsigned bitIndex) const { return IsSet(bitIndex); }
protected:
    UByte*  pData;
    unsigned    BitsCount;
};

template<int SID=Stat_Default_Mem>
class FixedBitSetLH : public FixedBitSetBase<AllocatorLH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorLH<UByte, SID> >     BaseType;

protected:
    void* getThis() { return this; }
public:
    FixedBitSetLH(unsigned bitsCount) : BaseType(getThis(), bitsCount) {}
};

template<int SID=Stat_Default_Mem>
class FixedBitSetDH : public FixedBitSetBase<AllocatorDH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorDH<UByte, SID> >     BaseType;

    FixedBitSetDH(MemoryHeap* pheap, unsigned bitsCount) : BaseType(pheap, bitsCount) {}
};

template<int SID=Stat_Default_Mem>
class FixedBitSetGH : public FixedBitSetBase<AllocatorGH<UByte, SID> >
{
public:
    typedef FixedBitSetBase<AllocatorGH<UByte, SID> >     BaseType;

    FixedBitSetGH(unsigned bitsCount) : BaseType(Memory::GetGlobalHeap(), bitsCount) {}
};

} // Scaleform

#endif // INC_SF_Kernel_BitSet_H
