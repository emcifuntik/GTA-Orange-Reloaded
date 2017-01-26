/**********************************************************************

Filename    :   HeapPT_DebugInfo.h
Content     :   Debug and statistics implementation.
Created     :   July 14, 2008
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_HeapPT_DebugInfo_H
#define INC_SF_Kernel_HeapPT_DebugInfo_H

#include "../SF_HeapTypes.h"

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

#include "../SF_List.h"
#include "../SF_Atomic.h"
#include "../SF_AllocInfo.h"
#include "HeapPT_Granulator.h"
#include "HeapPT_PageTable.h"

namespace Scaleform { namespace HeapPT {

class AllocEngine;

using namespace Heap;

//------------------------------------------------------------------------
template<class T>
struct DebugDataPool : ListNode<DebugDataPool<T> >
{
    typedef DebugDataPool<T>   SelfType;

    DebugDataPool() : UseCount(0) {}

    static UPInt GetSize(UPInt bytes)
    { 
        return (bytes - sizeof(SelfType)) / sizeof(T); 
    }

    T* GetElement(UPInt i) 
    {
        return (T*)((UByte*)this + sizeof(SelfType) + i*sizeof(T));
    }
    UPInt   UseCount;
};


//------------------------------------------------------------------------
struct DebugNode : ListNode<DebugNode>
{
    enum
    {
        // Initially DebugData is created, as a single linked list
        // of nodes. After exceeding PageUpperLimit it is converted into
        // DebugPage. When DataCount gets less then PageLowerLimit
        // the DebugPage is converted back to DebugData.
        // It works like a classical Schmitt trigger with a hysteresis 
        // loop:
        //           LowerLimit
        //               /-------<-------+-----Page----
        //               |               | 
        //               |   Hysteresis  |
        //               |      Loop     |
        //               |               |
        //  ----Chain----+------->-------/
        //                           UpperLimit
        //----------------------------------------------------------------
        PageUpperLimit = 12,
        PageLowerLimit = 6,
        NodePageFlag   = 0x8000U,
        NodePageMask   = 0x7FFFU,
        ChainLimit     = 10
    };

    UPInt IsPage() const 
    { 
        return DataCount & UPInt(NodePageFlag); 
    }

    UPInt GetDataCount() const 
    {
        return DataCount & UPInt(NodePageMask);
    }

    UPInt Increment()
    {
        return (++DataCount) & UPInt(NodePageMask);
    }

    UPInt Decrement()
    {
        return (--DataCount) & UPInt(NodePageMask);
    }

    UPIntHalf DataCount;
    UPIntHalf ChainCount;
};


//------------------------------------------------------------------------
struct DebugData : DebugNode
{
    typedef DebugDataPool<DebugData> DataPool;

    void Clear()
    {
        DataCount   = 0;
        ChainCount  = 0;
        pParent     = 0;
        pNextData   = 0;
        RefCount    = 0;
        Address     = 0;
        Size        = 0;
        memset(&Info, 0, sizeof(Info));
    }

    void MessUp()
    {
        memset(&DataCount,  0xFE, sizeof(DataCount));
        memset(&ChainCount, 0xFE, sizeof(ChainCount));
        memset(&pParent,    0xFE, sizeof(pParent));
        memset(&pNextData,  0xFE, sizeof(pNextData));
        memset(&RefCount,   0xFE, sizeof(RefCount));
        memset(&Address,    0xFE, sizeof(Address));
        memset(&Size,       0xFE, sizeof(Size));
        memset(&Info,       0xFE, sizeof(Info));
    }


    DataPool*       pDataPool;
    DebugData*      pParent;
    DebugData*      pNextData;      // Next data entry in chain
    UPInt           RefCount;
    UPInt           Address;
    UPInt           Size;
    AllocInfo       Info;
};



//------------------------------------------------------------------------
struct DebugPage : DebugNode
{
    typedef DebugDataPool<DebugPage> DataPool;

    enum 
    {
        PageShift   = 5,
        PageSize    = 1 << PageShift, // Must not exceed the number of bits in UPInt

        AddrShift   = Heap_PageShift - PageShift,
        AddrSize    = 1 << AddrShift,
        AddrMask    = AddrSize-1
    }; 

    void Clear()
    {
        DataCount   = UPInt(NodePageFlag);
        ChainCount  = 0;
        Mask        = 0;
        memset(pData, 0, sizeof(pData));
    }

    void MessUp()
    {
        memset(&DataCount,  0xFE, sizeof(DataCount));
        memset(&ChainCount, 0xFE, sizeof(ChainCount));
        memset(&Mask,       0xFE, sizeof(Mask));
        memset(pData,       0xFE, sizeof(pData));
    }

    DataPool*   pDataPool;
    UPInt       Mask;
    DebugData*  pData[PageSize];
};




//------------------------------------------------------------------------
class DebugStorage
{
    enum 
    { 
        PoolSize        = Heap_DebugAllocPoolSize,
        AddrShift       = DebugPage::AddrShift,
        AddrSize        = DebugPage::AddrSize,
        AddrMask        = DebugPage::AddrMask,
        PageUpperLimit  = DebugNode::PageUpperLimit,
        PageLowerLimit  = DebugNode::PageLowerLimit,
    };

    typedef DebugDataPool<DebugData> DataPoolType;
    typedef DebugDataPool<DebugPage> PagePoolType;

public:
    struct DebugDataPtr
    {
        DebugDataPtr() : pNode(0), Index(~UPInt(0)), pSelf(0), pPrev(0) {}
        DebugNode*  pNode;
        UPInt       Index;
        DebugData*  pSelf;
        DebugData*  pPrev;
    };

    DebugStorage(Granulator* alloc, LockSafe* rootLocker);

    unsigned GetStatId(UPInt parentAddr, const AllocInfo* info);

    bool AddAlloc(UPInt parentAddr, bool autoHeap, UPInt thisAddr, 
                  UPInt size, UPInt usable, const AllocInfo* info);

    void RemoveAlloc(UPInt addr, UPInt usable);

    void RelinkAlloc(DebugDataPtr* ptr, UPInt oldAddr, 
                     UPInt newAddr, UPInt newSize, UPInt usable);

    void CheckDataTail(const DebugDataPtr* ptr, UPInt usable);

    void FreeAll();

    void GetDebugData(UPInt addr, DebugDataPtr* ptr);

    void UnlinkAlloc(UPInt addr, DebugDataPtr* ptr);

    void GetStats(AllocEngine* allocator, StatBag* bag) const;

    const DebugData* GetFirstEntry() const;
    const DebugData* GetNextEntry(const DebugData* entry) const;

    void VisitMem(MemVisitor* visitor, unsigned flags) const;

    bool DumpMemoryLeaks(const char* heapName);

    void UltimateCheck();

    UPInt GetUsedSpace() const;

private:
    bool        allocDataPool();
    void        freeDataPool(DataPoolType* pool);
    bool        allocPagePool();
    void        freePagePool(PagePoolType* pool);

    DebugPage*  allocDebugPage();
    void        freeDebugPage(DebugPage* page);

    DebugData*  allocDebugData();
    void        freeDebugData(DebugData* data);
    void        unlinkDebugData(DebugDataPtr* ptr);
    void        linkDebugData(DebugData* data);
    void        findInChainWithin(DebugData* chain, UPInt addr, DebugDataPtr* ptr);
    void        findInChainExact(DebugData* chain, UPInt addr, DebugDataPtr* ptr);
    void        findInNodeWithin(DebugNode* node, UPInt idx, UPInt addr, DebugDataPtr* ptr);
    void        findDebugData(UPInt addr, bool autoHeap, DebugDataPtr* ret);
    void        linkToPage(DebugPage* page, DebugData* data);
    void        convertToChain(DebugPage* page, DebugData* oldData);
    bool        convertToPage(DebugData* chain, DebugData* newData);
    void        fillDataTail(DebugData* data, UPInt usable);
    void        reportViolation(DebugData* data, const char* msg);

    template<class T>
    void visitMem(const List<T>& lst, UPInt dataSize, 
                  MemVisitor* visitor,
                  MemVisitor::Category cat, 
                  HeapSegment* seg) const
    {
        const T* ptr = lst.GetFirst();
        while(!lst.IsNull(ptr))
        {
            const TreeSeg* allocSeg = pAllocator->GetAllocSegment(ptr);
            seg->pData    = allocSeg->Buffer;
            seg->DataSize = allocSeg->Size;
            visitor->Visit(seg, UPInt(ptr), dataSize, cat);
            ptr = (const T*)(ptr->pNext);
        }
    }

    Granulator*         pAllocator;
    List<DataPoolType>  DataPools;
    List<PagePoolType>  PagePools;
    List<DebugData>     UsedDataList;
    List<DebugData>     FreeDataList;
    List<DebugPage>     FreePageList;
    LockSafe*           pRootLocker;
};

}} // Scaleform::Heap

#endif // SF_MEMORY_ENABLE_DEBUG_INFO

#endif

