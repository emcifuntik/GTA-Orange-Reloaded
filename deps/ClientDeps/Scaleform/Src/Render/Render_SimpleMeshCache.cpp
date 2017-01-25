/**********************************************************************

Filename    :   SimpleMeshCache.cpp
Content     :   Common base for consoles MeshCache implementations.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render_SimpleMeshCache.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

//#define SF_RENDER_LOG_CACHESIZE

namespace Scaleform { namespace Render {


// ***** SimpleMeshCache

SimpleMeshCache::SimpleMeshCache(MemoryHeap* pheap,
                                 const MeshCacheParams& params,
                                 RenderSync* psync)
 : Render::MeshCache(pheap, params),   
   CacheList(getThis()),
   Allocator(pheap),
   TotalSize(0),
   pRenderSync(psync)
{
}

SimpleMeshCache::~SimpleMeshCache()
{   
}

// Allocates reserve, if any. Intended for use in Initialize implementation.
bool    SimpleMeshCache::allocateReserve()
{
    if (Params.MemReserve)
        return allocMeshBuffer(Params.MemReserve, MeshBuffer::AT_Reserve) != 0;
    return true;
}

void    SimpleMeshCache::releaseAllBuffers()
{
    ClearCache();
    while(!Buffers.IsEmpty())
        releaseMeshBuffer((SimpleMeshBuffer*)Buffers.GetFirst());
}


void    SimpleMeshCache::ClearCache()
{
    CacheList.EvictAll();
    
    // Release all dynamically allocated buffers.
    SimpleMeshBuffer* pbuffer = (SimpleMeshBuffer*)Buffers.GetFirst();
    while(!Buffers.IsNull(pbuffer))
    {
        SimpleMeshBuffer *p = pbuffer;
        pbuffer = (SimpleMeshBuffer*)pbuffer->pNext;
        if (p->GetType() == MeshBuffer::AT_Chunk)
            releaseMeshBuffer(p);
    }

    SF_ASSERT(BatchCacheItemHash.GetSize() == 0);
}

void    SimpleMeshCache::EndFrame()
{
    CacheList.EndFrame();

    // Simple Heuristic used to shrink cache. Shrink is possible once the
    // (Total_Frame_Size + LRUTailSize) exceed the allocated space by more then
    // one granularity unit. If this case, we destroy the cache buffer in the
    // order opposite to that of which it was created.

    // TBD: This may have a side effect of throwing away the current frame items
    // as well. Such effect is undesirable and can perhaps be avoided on consoles
    // with buffer data copies (copy PrevFrame content into other buffers before evict).

    UPInt totalFrameSize = CacheList.GetSlotSize(MCL_PrevFrame);
    UPInt lruTailSize    = CacheList.GetSlotSize(MCL_LRUTail);
    UPInt expectedSize   = totalFrameSize + Alg::PMin(lruTailSize, Params.LRUTailSize);
    expectedSize += expectedSize / 4; // + 25%, to account for fragmentation.

    SPInt extraSpace = TotalSize - (SPInt)expectedSize;
    if (extraSpace > (SPInt)Params.MemGranularity)
    {
        SimpleMeshBuffer* pbuffer = (SimpleMeshBuffer*)Buffers.GetLast();

        while (!Buffers.IsNull(pbuffer) && (extraSpace > (SPInt)Params.MemGranularity))
        {
            SimpleMeshBuffer* p = pbuffer;
            pbuffer = (SimpleMeshBuffer*)pbuffer->pPrev;
            if (p->GetType() != MeshBuffer::AT_Chunk)
                continue;

            extraSpace -= (SPInt)p->GetFullSize();
            releaseMeshBuffer(p);

#ifdef SF_RENDER_LOG_CACHESIZE
            LogDebugMessage(Log_Message,
                "Cache shrank to %dK. Start FrameSize = %dK, LRU = %dK\n",
                TotalSize / 1024, totalFrameSize/1024, lruTailSize/1024);
#endif
        }
    }
}

// Adds a fixed-size buffer to cache reserve; expected to be released at Release.
/*
bool    MeshCache::AddReserveBuffer(unsigned size, unsigned arena)
{
}
bool    MeshCache::ReleaseReserveBuffer(unsigned size, unsigned arena)
{
}
*/


// Allocates Vertex/Index buffer of specified size and adds it to free list.
SimpleMeshBuffer*   SimpleMeshCache::allocMeshBuffer(UPInt size, AllocType atype, unsigned arena)
{    
    SimpleMeshBuffer* pbuffer = createHWBuffer(size, atype, arena);
    if (!pbuffer)
        return 0;

    // Add the newly reserved area to the allocator.
    Allocator.AddSegment((UPInt)pbuffer->pData, pbuffer->GetSize());
    Buffers.PushBack(pbuffer);
    TotalSize += pbuffer->GetFullSize();
    return pbuffer;
}

void    SimpleMeshCache::releaseMeshBuffer(SimpleMeshBuffer* pbuffer)
{
    // Remove from Allocator; generating any necessary warnings.
    // Go through all allocation lists and Evict data items in the buffers address range.

    // TBD: There shouldn't be any MCL_InFlight items. If there are,
    //      free operations should be delayed...
    SF_ASSERT(CacheList.GetSlot(MCL_InFlight).IsEmpty());
    
    evictMeshesInBuffer(CacheList.GetSlots(), MCL_ItemCount, pbuffer);
    Allocator.RemoveSegment((UPInt)pbuffer->pData, pbuffer->GetSize());
    
    pbuffer->RemoveNode();
    TotalSize -= pbuffer->GetFullSize();
    destroyHWBuffer(pbuffer);
}


SimpleMeshBuffer* SimpleMeshCache::findBuffer(UPInt address)
{
    // TBD: This is less then ideal since it happens on every allocation....
    SimpleMeshBuffer* p = (SimpleMeshBuffer*)Buffers.GetFirst();
    while (!Buffers.IsNull(p))
    {
        UPInt bufferAddress = (UPInt)p->pData; 
        if ((address >= bufferAddress) &&
            (address < (bufferAddress + p->GetSize())))
            return p;
        p = (SimpleMeshBuffer*)p->pNext;
    }
    return 0;
}

void    SimpleMeshCache::evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                             SimpleMeshBuffer* pbuffer)
{
    for (unsigned i=0; i< count; i++)
    {
        SimpleMeshCacheItem* pitem = (SimpleMeshCacheItem*)plist[i].GetFirst();
        while (!plist[i].IsNull(pitem))
        {
            SimpleMeshCacheItem* p = pitem;
            pitem = (SimpleMeshCacheItem*)pitem->pNext;

            if (p->pBuffer == pbuffer)
                Evict(p);
        }
    }
}


bool    SimpleMeshCache::allocBuffer(UPInt* poffset, UPInt size, bool waitForCache)
{
    *poffset = Allocator.Alloc(size);
    if (*poffset != ~UPInt(0))
        return true;
   
    // If allocation failed... need to apply swapping or grow buffer.
    MeshCacheItem* pitems, *p;        

    // 1) First, apply LRU (least recently used) swapping from data stale in
    //    earlier frames. Swapping done first until under LRUTailSize if
    //    cache growth is possible; if not, swapping is done until LRU is empty.

    if ((TotalSize + SimpleMeshBuffer::MinGranularity) <= Params.MemLimit)
    {
        if (CacheList.EvictLRUTillLimit(MCL_LRUTail, Allocator,
                                        size, Params.LRUTailSize))
            goto alloc_size_available;

        // TBD: May cause spinning? Should we have two error codes?
        SF_ASSERT(size <= SimpleMeshBuffer::GetUsableSize(Params.MemGranularity));
        if (size > SimpleMeshBuffer::GetUsableSize(Params.MemGranularity))
            return false;

        UPInt allocSize = Alg::PMin(Params.MemLimit - TotalSize, Params.MemGranularity);
        if (size <= SimpleMeshBuffer::GetUsableSize(allocSize))
        {
            if (allocMeshBuffer(allocSize, MeshBuffer::AT_Chunk, 0))
            {         
#ifdef SF_RENDER_LOG_CACHESIZE
                LogDebugMessage(Log_Message, "Cache grew to %dK\n", TotalSize / 1024);
#endif
                goto alloc_size_available;
            }
        }
    }

    if (CacheList.EvictLRU(MCL_LRUTail, Allocator, size))
        goto alloc_size_available;

    
    // 2) Apply MRU (most recently used) swapping to the previous frame content.
    // TBD: We'll we assume that Fence is not needed here since PrevFrame is
    // out of the HW rendering queue by now; this needs to be verified.

    pitems = (MeshCacheItem*)CacheList.GetSlot(MCL_PrevFrame).GetFirst();
    while(!CacheList.GetSlot(MCL_PrevFrame).IsNull(pitems))
    {
        p      = pitems;
        pitems = (MeshCacheItem*)p->pNext;

        if (Evict(p, &Allocator) >= size)
            goto alloc_size_available;
    }

    // Force command buffer kick-off in case we'll be swapping
    // out the last draw command.
    pRenderSync->KickOffFences();

    // 3) Apply MRU (most recently used) swapping to the current frame content.
    // NOTE: MRU (GetFirst(), pNext iteration) gives
    //       2x improvement here with "Stars" test swapping.
    pitems = (MeshCacheItem*)CacheList.GetSlot(MCL_ThisFrame).GetFirst();
    while(!CacheList.GetSlot(MCL_ThisFrame).IsNull(pitems))
    {
        p      = pitems;
        pitems = (MeshCacheItem*)p->pNext;

        if (pRenderSync->IsPending(p->Fence))
        {   // Can't evict this item.
            if (!waitForCache)
                return false;
        }
        else if (Evict(p, &Allocator) >= size)
        {
            goto alloc_size_available;
        }          
    }

    // If MRU swapping didn't work for ThisFrame items due to them still
    // being processed by the GPU and we are being asked to wait, wait
    // until all of the fences are passed.
    if (waitForCache && !CacheList.GetSlot(MCL_ThisFrame).IsEmpty())
    {            
        pitems = (MeshCacheItem*)CacheList.GetSlot(MCL_ThisFrame).GetFirst();
        while(!CacheList.GetSlot(MCL_ThisFrame).IsNull(pitems))
        {
            p      = pitems;
            pitems = (MeshCacheItem*)p->pPrev;

            pRenderSync->WaitFence(p->Fence);
            if (Evict(p, &Allocator) >= size)
                goto alloc_size_available;
        }
    }

    return false;

    // At this point we know we have a large enough block either due to
    // swapping or buffer growth, so allocation shouldn't fail.
alloc_size_available:
    *poffset = Allocator.Alloc(size);
    SF_ASSERT(*poffset != ~UPInt(0));
    return true;
}


UPInt   SimpleMeshCache::Evict(Render::MeshCacheItem* p, AllocAddr* pallocator, MeshBase* pskipMesh)
{
    // - Free allocator data.
    AllocAddr &a = pallocator ? *pallocator : Allocator;
    UPInt freedSize = a.Free(((SimpleMeshCacheItem*)p)->AllocAddress, p->AllocSize);
    p->Destroy(pskipMesh);
    return freedSize;
}


}}; // namespace Scaleform::Render

