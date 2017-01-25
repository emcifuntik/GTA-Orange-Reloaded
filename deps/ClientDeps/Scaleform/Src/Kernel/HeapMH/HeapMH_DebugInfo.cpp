/**********************************************************************

Filename    :   HeapMH_DebugInfo.cpp
Content     :   Debug and statistics implementation.
Created     :   July 14, 2008
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "../SF_Debug.h"
#include "../SF_Stats.h"
#include "HeapMH_DebugInfo.h"
#include "HeapMH_AllocEngine.h"

#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
DebugStorageMH::DebugStorageMH(SysAlloc* alloc, LockSafe* rootLocker) : 
    pAllocator(alloc), 
    pRootLocker(rootLocker)
{}


//------------------------------------------------------------------------
bool DebugStorageMH::allocDataPool()
{
    void* poolBuf = 0; 
    {
        LockSafe::Locker rl(pRootLocker);
        poolBuf = pAllocator->Alloc(PoolSize, sizeof(void*));
    }

    if (poolBuf)
    {
        //memset(poolBuf, 0xCC, PoolSize);
        DataPoolType* pool = ::new (poolBuf) DataPoolType;
        DataPools.PushFront(pool);

        UPInt size = pool->GetSize(PoolSize);
        for(UPInt i = 0; i < size; ++i)
        {
            DebugDataMH* data = pool->GetElement(i);
            data->pDataPool = pool;
            FreeDataList.PushBack(data);
        }
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
void DebugStorageMH::freeDataPool(DataPoolType* pool)
{
    UPInt size = pool->GetSize(PoolSize);
    for(UPInt i = 0; i < size; ++i)
    {
        FreeDataList.Remove(pool->GetElement(i));
    }
    DataPools.Remove(pool);
    {
        LockSafe::Locker rl(pRootLocker);
        //memset(pool, 0xFE, PoolSize);
        pAllocator->Free(pool, PoolSize, sizeof(void*));
    }
}

//------------------------------------------------------------------------
DebugDataMH* DebugStorageMH::allocDebugData()
{
    if (FreeDataList.IsEmpty())
    {
        if (!allocDataPool())
            return 0;
    }
    SF_ASSERT(!FreeDataList.IsEmpty());

    DebugDataMH* data = FreeDataList.GetFirst();
    FreeDataList.Remove(data);
    data->Clear();
    UsedDataList.PushBack(data);
    data->pDataPool->UseCount++;
    return data;
}


//------------------------------------------------------------------------
void DebugStorageMH::freeDebugData(DebugDataMH* data)
{
    //data->Clear();
    FreeDataList.PushFront(data);
    if (--data->pDataPool->UseCount == 0)
    {
        freeDataPool(data->pDataPool);
    }
}


//------------------------------------------------------------------------
DebugDataMH* DebugStorageMH::getDebugData(PageInfoMH* page)
{
    for (int i = 0; i < 3; ++i)
    {
        if (page->DebugHeaders[i])
        {
            return *(page->DebugHeaders[i]);
        }
    }
    return 0;
}

//------------------------------------------------------------------------
void DebugStorageMH::setDebugData(PageInfoMH* page, DebugDataMH* data)
{
    for (int i = 0; i < 3; ++i)
    {
        if (page->DebugHeaders[i])
        {
            *(page->DebugHeaders[i]) = data;
        }
    }
}


//------------------------------------------------------------------------
void DebugStorageMH::unlinkDebugData(PageInfoMH* page, DebugDataPtr* ptr)
{
    ptr->pData->DataCount--;
    if (ptr->pPrev)
    {
        ptr->pPrev->pNextData = ptr->pSelf->pNextData;
    }
    else
    {
        if (ptr->pSelf->pNextData)
        {
            ptr->pSelf->pNextData->DataCount = ptr->pData->DataCount;
        }
        setDebugData(page, ptr->pSelf->pNextData);
    }
}



//------------------------------------------------------------------------
void DebugStorageMH::linkDebugData(PageInfoMH* page, DebugDataMH* data)
{
    DebugDataMH* node = getDebugData(page);
    if (node == 0)
    {
        data->pNextData = 0;
        node = data;
    }

    if (data != node)
    {
        data->pNextData = (DebugDataMH*)node;
        data->DataCount = node->DataCount;
    }
    setDebugData(page, data);
}



//------------------------------------------------------------------------
void DebugStorageMH::findInChainWithin(DebugDataMH* chain, UPInt addr, DebugDataPtr* ptr)
{
    DebugDataMH* prev = 0;
    while (chain)
    {
        UPInt end = chain->Address + chain->Size;
        if (addr >= chain->Address && addr < end)
        {
            ptr->pSelf = chain;
            ptr->pPrev = prev;
            return;
        }
        prev  = chain;
        chain = chain->pNextData;
    }
}


//------------------------------------------------------------------------
void DebugStorageMH::findInChainExact(DebugDataMH* chain, UPInt addr, DebugDataPtr* ptr)
{
    DebugDataMH* prev = 0;
    while (chain)
    {
        if (addr == chain->Address)
        {
            ptr->pSelf = chain;
            ptr->pPrev = prev;
            return;
        }
        prev  = chain;
        chain = chain->pNextData;
    }
}


//------------------------------------------------------------------------
void DebugStorageMH::findDebugData(PageInfoMH* page, UPInt addr, DebugDataPtr* ptr)
{
    DebugDataMH* node = getDebugData(page);
    ptr->pData = node;
    findInChainWithin(node, addr, ptr);
}


//------------------------------------------------------------------------
void DebugStorageMH::fillDataTail(DebugDataMH* data, UPInt usable)
{
    UByte* p = (UByte*)data->Address;
    for (UPInt i = data->Size; i < usable; ++i)
    {
        p[i] = UByte(129 + i);
    }
}

//------------------------------------------------------------------------
void DebugStorageMH::CheckDataTail(const DebugDataPtr* ptr, UPInt usable)
{
    const UByte* p = (const UByte*)ptr->pSelf->Address;
    for (UPInt i = ptr->pSelf->Size; i < usable; ++i)
    {
        if (p[i] != UByte(129 + i))
        {
            reportViolation(ptr->pSelf, " Corruption");
            SF_ASSERT(0);
        }
    }
}

//------------------------------------------------------------------------
unsigned DebugStorageMH::GetStatId(PageInfoMH* page, UPInt parentAddr, const AllocInfo* info)
{
    AllocInfo i2 = info ? *info : AllocInfo();
    if (i2.StatId != Stat_Default_Mem)
    {
        return i2.StatId;
    }
    DebugDataPtr parent;
    findDebugData(page, parentAddr, &parent);
    if (parent.pSelf == 0)
    {
        return Stat_Default_Mem;
    }
    return parent.pSelf->Info.StatId;
}


//------------------------------------------------------------------------
bool DebugStorageMH::AddAlloc(UPInt thisAddr, UPInt size, PageInfoMH* page, const AllocInfo* info)
{
    SF_ASSERT(thisAddr);

    DebugDataMH* data = allocDebugData();
    if (data)
    {
        data->RefCount  = 1;
        data->Address   = thisAddr;
        data->Size      = size;
        data->Info      = info ? *info : AllocInfo();

        linkDebugData(page, data);
        fillDataTail(data, page->UsableSize);
        return true;
    }
    return false;
}


//------------------------------------------------------------------------
bool DebugStorageMH::AddAlloc(PageInfoMH* parentInfo, UPInt parentAddr, UPInt thisAddr, 
                              UPInt size, PageInfoMH* page, const AllocInfo* info)
{
    SF_ASSERT(thisAddr);

    DebugDataMH* data = allocDebugData();
    if (data)
    {
        data->RefCount  = 1;
        data->Address   = thisAddr;
        data->Size      = size;
        data->Info      = info ? *info : AllocInfo();

        linkDebugData(page, data);
        fillDataTail(data, page->UsableSize);

//if (usable > size)      // DBG Simulate memory corruption
//    ((char*)data->Address)[size] = 0;

        DebugDataPtr parent;
        findDebugData(parentInfo, parentAddr, &parent);
        SF_ASSERT(parent.pSelf != 0);
        if (data->Info.StatId == Stat_Default_Mem)
        {
            data->Info.StatId = parent.pSelf->Info.StatId;
        }

        // Prevent from creating too long parent chains in 
        // the following scenario:
        //
        //    p1 = heap.Alloc(. . .);
        //    for(. . .)
        //    {
        //        p2 = AllocAutoHeap(p1, . . .);
        //        Free(p1);
        //        p1 = p2;
        //    }
        //
        // When the chain gets longer than ChainLimit, just 
        // connect it to the grandparent, interrupting the chain. 
        // The condition "DebugNode::ChainLimit+1" means that 
        // we allow one more element in the chain to be able to detect 
        // whether or not the chain was interrupted: 
        // Interruption has occurred if ChainCount > ChainLimit.
        //-----------------------------
        if (parent.pSelf->ChainCount > DebugDataMH::ChainLimit+1)
        {
            parent.pSelf = parent.pSelf->pParent;
            SF_ASSERT(parent.pSelf);
        }
        data->pParent    = parent.pSelf;
        data->ChainCount = parent.pSelf->ChainCount + 1;
        parent.pSelf->RefCount++;
        return true;
    }
    return false;
}

//------------------------------------------------------------------------
void DebugStorageMH::RemoveAlloc(UPInt addr, PageInfoMH* page)
{
    SF_ASSERT(addr);
    DebugDataPtr ptr;
    GetDebugData(addr, page, &ptr);
    CheckDataTail(&ptr, page->UsableSize);
    SF_ASSERT(ptr.pSelf->Address == addr);  // Must be exact address.

    UsedDataList.Remove(ptr.pSelf);
    unlinkDebugData(page, &ptr);

    DebugDataMH* data = ptr.pSelf;
    while (data && --data->RefCount == 0)
    {
        DebugDataMH* parent = data->pParent;
        freeDebugData(data);
        data = parent;
    }
}

//------------------------------------------------------------------------
void DebugStorageMH::UnlinkAlloc(UPInt addr, PageInfoMH* page, DebugDataPtr* ptr)
{
    GetDebugData(addr, page, ptr);
    unlinkDebugData(page, ptr);
}

//------------------------------------------------------------------------
void DebugStorageMH::RelinkAlloc(DebugDataPtr* ptr, UPInt oldAddr, 
                                 UPInt newAddr, UPInt newSize,
                                 PageInfoMH* newInfo)
{
    SF_ASSERT(ptr->pSelf && oldAddr && newAddr);
    
    ptr->pSelf->Size = newSize;
    ptr->pSelf->Address   = newAddr;
    ptr->pSelf->DataCount = 0;
    linkDebugData(newInfo, ptr->pSelf);
    fillDataTail(ptr->pSelf, newInfo->UsableSize);

#ifndef SF_BUILD_DEBUG
    SF_UNUSED(oldAddr);
#endif
}



//------------------------------------------------------------------------
void DebugStorageMH::GetDebugData(UPInt addr, PageInfoMH* page, DebugDataPtr* ptr)
{
    findDebugData(page, addr, ptr);
    SF_ASSERT(ptr->pSelf);
}

//------------------------------------------------------------------------
void DebugStorageMH::FreeAll()
{
    while (!DataPools.IsEmpty())
    {
        DataPoolType* pool = DataPools.GetFirst();
        DataPools.Remove(pool);
        pAllocator->Free(pool, PoolSize, 0);
    }

    UsedDataList.Clear();
    FreeDataList.Clear();
}

//------------------------------------------------------------------------
void DebugStorageMH::GetStats(AllocEngineMH* allocator, StatBag* bag) const
{
#ifdef SF_ENABLE_STATS
    // Must be locked in MemoryHeap.
    const DebugDataMH* data = UsedDataList.GetFirst();
    while (!UsedDataList.IsNull(data))
    {
        bag->IncrementMemoryStat(data->Info.StatId,
                                 data->Size,
                                 allocator->GetUsableSize((void*)data->Address));
        data = (const DebugDataMH*)data->pNext;
    }
#else
    SF_UNUSED2(allocator, bag);
#endif
}

//------------------------------------------------------------------------
const DebugDataMH* DebugStorageMH::GetFirstEntry() const
{
    return UsedDataList.IsEmpty() ? 0 : UsedDataList.GetFirst();
}

//------------------------------------------------------------------------
const DebugDataMH* DebugStorageMH::GetNextEntry(const DebugDataMH* entry) const
{
    const DebugDataMH* next = (const DebugDataMH*)entry->pNext;
    return UsedDataList.IsNull(next) ? 0 : next;
}

//------------------------------------------------------------------------
static void MemCpyClean(char* dest, const char* src, int sz)
{
    int i = 0;
    for (; i < sz; i++)
    {
        char c = src[i];
        if (c < 32 || c > 126)
            c = '.';
        dest[i] = c;
    }
    dest[i] = 0;
}

//------------------------------------------------------------------------
void DebugStorageMH::reportViolation(DebugDataMH* data, const char* msg)
{
    char memdump[33];

    MemCpyClean(memdump, 
               (const char*)(data->Address), 
               (data->Size < 32) ? int(data->Size) : 32);

    SF_DEBUG_MESSAGE6(data->Info.pFileName,
                      "%s(%d) :%s %d bytes @0x%8.8X (contents:'%s')\n", 
                      data->Info.pFileName, 
                      data->Info.Line,
                      msg,
                      data->Size, 
                      data->Address, 
                      memdump);

    SF_DEBUG_MESSAGE2(!data->Info.pFileName,
                      "%s in unknown file: %d bytes \n",
                      msg,
                      data->Size);
    
    const DebugDataMH* parent = data->pParent;

    unsigned int i2 = 1;
    char indentBuf[256];
    while(parent)
    {
        if (i2 > sizeof(indentBuf) - 1)
            i2 = sizeof(indentBuf) - 1;
        memset(indentBuf, '+', i2);
        indentBuf[i2] = 0;

        SF_DEBUG_MESSAGE6(parent->Info.pFileName,
                          "%s(%d) : %s Parent of alloc @0x%8.8X is (%d bytes @0x%8.8X)\n",
                          parent->Info.pFileName, 
                          parent->Info.Line,
                          indentBuf,
                          data->Address,
                          parent->Size,
                          parent->Address);

        SF_DEBUG_MESSAGE4(!parent->Info.pFileName,
                          "%s Unknown file: Parent of alloc @0x%8.8X is (%d bytes @0x%8.8X)\n", 
                          indentBuf,
                          data->Address,
                          parent->Size,
                          parent->Address);

        parent = parent->pParent;
        ++i2;
    }

#ifndef SF_BUILD_DEBUG
    SF_UNUSED(msg);
#endif
}

//------------------------------------------------------------------------
bool DebugStorageMH::DumpMemoryLeaks(const char* heapName)
{
    bool ret = false;
    UPInt leakedBytes = 0;
    UPInt leakedAllocs = 0;

#ifndef SF_BUILD_DEBUG
    SF_UNUSED(heapName);
#endif

    DebugDataMH* data = UsedDataList.GetFirst();
    SF_DEBUG_ERROR1(!UsedDataList.IsNull(data), 
                    "Memory leaks detected in heap '%s'!", heapName);
    SF_UNUSED(heapName);

    while (!UsedDataList.IsNull(data))
    {
        ret = true;
        reportViolation(data, " Leak");
        leakedBytes += data->Size;
        leakedAllocs++;
        data = (DebugDataMH*)data->pNext;
    }
    SF_DEBUG_ERROR2(ret, "Total memory leaked: %d bytes in %d allocations", 
                    leakedBytes, 
                    leakedAllocs);
    return ret;
}


//------------------------------------------------------------------------
void DebugStorageMH::UltimateCheck()
{
}


//------------------------------------------------------------------------
UPInt DebugStorageMH::GetUsedSpace() const
{
    UPInt usedSpace = 0;
    const DataPoolType* data = DataPools.GetFirst();
    while(!DataPools.IsNull(data))
    {
        usedSpace += PoolSize;
        data = data->pNext;
    }
    return usedSpace;
}

}} // Scaleform::HeapMH

#endif
