/**********************************************************************

Filename    :   GL_MeshCache.cpp
Content     :   GL Mesh Cache implementation
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "GL_MeshCache.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_HeapNew.h"

#if defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ExtensionMacros.h"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#endif

//#define SF_RENDER_LOG_CACHESIZE

namespace Scaleform { namespace Render { namespace GL {


MeshBuffer::~MeshBuffer()
{
    if (Buffer)
        glDeleteBuffers(1, &Buffer);
}

bool MeshBuffer::DoMap()
{
    SF_ASSERT(!pData);
    glBindBuffer(Type, Buffer);
    pData = glMapBuffer(Type, GL_WRITE_ONLY);
    return pData != 0;
}

void MeshBuffer::Unmap()
{
    if (pData)
    {
        glBindBuffer(Type, Buffer);
        bool result = glUnmapBuffer(Type); // XXX - data loss can occur here
        SF_ASSERT(result);
        pData = 0;
    }
}

bool MeshBuffer::allocBuffer()
{
    if (Buffer)
        glDeleteBuffers(1, &Buffer);
    glGenBuffers(1, &Buffer);

    // Binding to the array or element target at creation is supposed to let drivers that need
    // separate vertex/index storage to know what the buffer will be used for.
    glBindBuffer(Type, Buffer);
    glBufferData(Type, Size, 0, GL_STATIC_DRAW);
    return 1;
}


// Helpers used to initialize default granularity sizes,
// splitting VB/Index size by 5/9.
inline UPInt calcVBGranularity(UPInt granularity)
{
    return (((granularity >> 4) * 5) / 9) << 4;
}
inline UPInt calcIBGranularity(UPInt granularity, UPInt vbGranularity)
{
    return (((granularity >> 4) - (vbGranularity >> 4)) << 4);
}


//------------------------------------------------------------------------
// ***** MeshCache

MeshCache::MeshCache(MemoryHeap* pheap, const MeshCacheParams& params)
  : Render::MeshCache(pheap, params),
    pHal(0), CacheList(getThis()),
    VertexBuffers(GL_ARRAY_BUFFER, pheap, calcVBGranularity(params.MemGranularity)),
    IndexBuffers(GL_ELEMENT_ARRAY_BUFFER, pheap,
                 calcIBGranularity(params.MemGranularity, VertexBuffers.GetGranularity())),
    Mapped(false), VBSizeEvictedInMap(0),
    MaskEraseBatchVertexBuffer(0)
{
}

MeshCache::~MeshCache()
{
    Reset();
}

// Initializes MeshCache for operation, including allocation of the reserve
// buffer. Typically called from SetVideoMode.
bool    MeshCache::Initialize(HAL* phal)
{
    if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
        return false;

    pHal = phal;
    UseSeparateIndexBuffers = true;

    if (!createInstancingVertexBuffer() ||
        !createMaskEraseBatchVertexBuffer())
    {
        Reset();
        return false;
    }

    if (Params.MemReserve &&
        !allocCacheBuffers(Params.MemReserve, MeshBuffer::AT_Reserve))
    {
        Reset();
        return false;
    }

    return true;
}

void MeshCache::Reset()
{
    if (pHal)
    {
        destroyBuffers();
        if (MaskEraseBatchVertexBuffer)
            glDeleteBuffers(1, &MaskEraseBatchVertexBuffer);
        MaskEraseBatchVertexBuffer = 0;
        pHal = 0;
    }

    StagingBuffer.Reset();
}

void MeshCache::ClearCache()
{
    destroyBuffers(MeshBuffer::AT_Chunk);
    SF_ASSERT(BatchCacheItemHash.GetSize() == 0);
}

void MeshCache::destroyBuffers(MeshBuffer::AllocType at)
{
    // TBD: Evict everything first!
    CacheList.EvictAll();
    VertexBuffers.DestroyBuffers(at);
    IndexBuffers.DestroyBuffers(at);
    ChunkBuffers.Clear();
}

bool MeshCache::SetParams(const MeshCacheParams& argParams)
{
    MeshCacheParams params(argParams);
    adjustMeshCacheParams(&params);

    if (pHal)
    {
        CacheList.EvictAll();

        if (Params.StagingBufferSize != params.StagingBufferSize)
        {
            if (!StagingBuffer.Initialize(pHeap, params.StagingBufferSize))
            {
                if (!StagingBuffer.Initialize(pHeap, Params.StagingBufferSize))
                {
                    SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore StagingBuffer after fail");
                }
                return false;
            }
        }

        if ((Params.MemReserve != params.MemReserve) ||
            (Params.MemGranularity != params.MemGranularity))
        {
            destroyBuffers();

            // Allocate new reserve. If not possible, restore previous one and fail.
            if (params.MemReserve &&
                !allocCacheBuffers(params.MemReserve, MeshBuffer::AT_Reserve))
            {
                if (Params.MemReserve &&
                    !allocCacheBuffers(Params.MemReserve, MeshBuffer::AT_Reserve))
                {
                    SF_DEBUG_ERROR(1, "MeshCache::SetParams - couldn't restore Reserve after fail");
                }
                return false;
            }

            VertexBuffers.SetGranularity(calcVBGranularity(params.MemGranularity));
            IndexBuffers.SetGranularity(calcIBGranularity(params.MemGranularity,
                                        VertexBuffers.GetGranularity()));
        }
    }
    Params = params;
    return true;
}

void MeshCache::adjustMeshCacheParams(MeshCacheParams* p)
{
    // TBD: Detect/record HW instancing capability.

    if (p->MaxBatchInstances > SF_RENDER_GL_INSTANCE_MATRICES)
        p->MaxBatchInstances = SF_RENDER_GL_INSTANCE_MATRICES;
    if (p->VBLockEvictSizeLimit < 1024 * 256)
        p->VBLockEvictSizeLimit = 1024 * 256;

    UPInt maxStagingItemSize = p->MaxVerticesSizeInBatch +
                               sizeof(UInt16) * p->MaxIndicesInBatch;
    if (maxStagingItemSize * 2 > p->StagingBufferSize)
        p->StagingBufferSize = maxStagingItemSize * 2;
}


void MeshCache::EndFrame()
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

    SPInt extraSpace = getTotalSize() - (SPInt)expectedSize;
    if (extraSpace > (SPInt)Params.MemGranularity)
    {        
        while (!ChunkBuffers.IsEmpty() && (extraSpace > (SPInt)Params.MemGranularity))
        {
            MeshBuffer* p = (MeshBuffer*)ChunkBuffers.GetLast();
            p->RemoveNode();
            extraSpace -= (SPInt)p->GetSize();

            MeshBufferSet&  mbs = (p->GetBufferType() == GL_ARRAY_BUFFER) ?
                                  (MeshBufferSet&)VertexBuffers : (MeshBufferSet&)IndexBuffers;
            // Evict first!
            evictMeshesInBuffer(CacheList.GetSlots(), MCL_ItemCount, p);
            mbs.DestroyBuffer(p);

#ifdef SF_RENDER_LOG_CACHESIZE
            LogDebugMessage(Log_Message,
                "Cache shrank to %dK. Start FrameSize = %dK, LRU = %dK\n",
                getTotalSize() / 1024, totalFrameSize/1024, lruTailSize/1024);
#endif
        }
    }    
}


// Adds a fixed-size buffer to cache reserve; expected to be released at Release.
/*
bool MeshCache::AddReserveBuffer(unsigned size, unsigned arena)
{
}
bool MeshCache::ReleaseReserveBuffer(unsigned size, unsigned arena)
{
}
*/


// Allocates Vertex/Index buffer of specified size and adds it to free list.
bool MeshCache::allocCacheBuffers(UPInt size, MeshBuffer::AllocType type, unsigned arena)
{
    if (UseSeparateIndexBuffers)
    {
        UPInt vbsize = calcVBGranularity(size);
        UPInt ibsize = calcIBGranularity(size, vbsize);

        MeshBuffer *pvb = VertexBuffers.CreateBuffer(vbsize, type, arena, pHeap, pHal);
        if (!pvb)
            return false;
        MeshBuffer   *pib = IndexBuffers.CreateBuffer(ibsize, type, arena, pHeap, pHal);
        if (!pib)
        {
            VertexBuffers.DestroyBuffer(pvb);
            return false;
        }
    }
    else
    {
        MeshBuffer *pb = VertexBuffers.CreateBuffer(size, type, arena, pHeap, pHal);
        if (!pb)
            return false;
    }

#ifdef SF_RENDER_LOG_CACHESIZE
    LogDebugMessage(Log_Message, "Cache grew to %dK\n", getTotalSize() / 1024);
#endif
    return true;
}


bool MeshCache::createInstancingVertexBuffer()
{
    return true;
}

bool MeshCache::createMaskEraseBatchVertexBuffer()
{
    VertexXY16iAlpha pbuffer[6 * MaxEraseBatchCount];

    // For now we create a buffer with a list of const values so that it can be
    // used to carry matrix index. TBD: Perhaps there is a shader value we can use instead?
    for(unsigned i = 0; i< MaxEraseBatchCount; i++)
    {
        // This assumes Alpha in first byte. Effect may depend on byte order and
        // ShaderManager vertex format mapping (offset assigned for VET_Instance8
        // for ShaderManager::registerVertexFormat).
        pbuffer[i * 6 + 0].x  = 0;
        pbuffer[i * 6 + 0].y  = 1;
        pbuffer[i * 6 + 0].Alpha[0] = (UByte)i;
        pbuffer[i * 6 + 1].x  = 0;
        pbuffer[i * 6 + 1].y  = 0;
        pbuffer[i * 6 + 1].Alpha[0] = (UByte)i;
        pbuffer[i * 6 + 2].x  = 1;
        pbuffer[i * 6 + 2].y  = 0;
        pbuffer[i * 6 + 2].Alpha[0] = (UByte)i;

        pbuffer[i * 6 + 3].x  = 0;
        pbuffer[i * 6 + 3].y  = 1;
        pbuffer[i * 6 + 3].Alpha[0] = (UByte)i;
        pbuffer[i * 6 + 4].x  = 1;
        pbuffer[i * 6 + 4].y  = 0;
        pbuffer[i * 6 + 4].Alpha[0] = (UByte)i;
        pbuffer[i * 6 + 5].x  = 1;
        pbuffer[i * 6 + 5].y  = 1;
        pbuffer[i * 6 + 5].Alpha[0] = (UByte)i;
    }

    glGenBuffers(1, &MaskEraseBatchVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, MaskEraseBatchVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pbuffer), pbuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}


bool MeshCache::LockBuffers()
{
    SF_ASSERT(!Mapped);
    Mapped = true;
    VBSizeEvictedInMap = 0;
    if (pRQCaches)
        pRQCaches->SetCacheLocked(Cache_Mesh);
    return true;
}

void MeshCache::UnlockBuffers()
{
    SF_ASSERT(Mapped != 0); 
    MappedBuffers.UnmapAll();
    Mapped = false;
    if (pRQCaches)
        pRQCaches->ClearCacheLocked(Cache_Mesh);
}


void MeshCache::evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
    MeshBuffer* pbuffer)
{
    for (unsigned i=0; i< count; i++)
    {
        MeshCacheItem* pitem = (MeshCacheItem*)plist[i].GetFirst();
        while (!plist[i].IsNull(pitem))
        {
            MeshCacheItem* p = pitem;
            pitem = (MeshCacheItem*)pitem->pNext;

            if ((p->pVertexBuffer == pbuffer) || (p->pIndexBuffer == pbuffer))
                Evict(p);
        }
    }
}


UPInt MeshCache::Evict(Render::MeshCacheItem* pbatch, AllocAddr* pallocator, MeshBase* pskipMesh)
{
    MeshCacheItem* p = (MeshCacheItem*)pbatch;
    // - Free allocator data.
    UPInt vbfree = VertexBuffers.Free(p->VBAllocSize, p->pVertexBuffer, p->VBAllocOffset);
    UPInt ibfree = IndexBuffers.Free(p->IBAllocSize, p->pIndexBuffer, p->IBAllocOffset);
    UPInt freedSize = (&VertexBuffers.GetAllocator() == pallocator) ? vbfree : ibfree;
    
    VBSizeEvictedInMap += (unsigned)  p->VBAllocSize;
    p->Destroy(pskipMesh);
    return freedSize;
}


// Allocates the buffer, while evicting LRU data.
bool MeshCache::allocBuffer(UPInt* poffset, MeshBuffer** pbuffer,
                            MeshBufferSet& mbs, UPInt size, bool waitForCache)
{
    SF_UNUSED(waitForCache);

    if (mbs.Alloc(size, pbuffer, poffset))
        return true;

    // If allocation failed... need to apply swapping or grow buffer.
    MeshCacheItem* pitems, *p;

    // 1) First, apply LRU (least recently used) swapping from data stale in
    //    earlier frames until the total size 

    if ((getTotalSize() + MinSupportedGranularity) <= Params.MemLimit)
    {
        if (CacheList.EvictLRUTillLimit(MCL_LRUTail, mbs.GetAllocator(),
                                        size, Params.LRUTailSize))
                                        goto alloc_size_available;

        // TBD: May cause spinning? Should we have two error codes?
        SF_ASSERT(size <= mbs.GetGranularity());
        if (size > mbs.GetGranularity())
            return false;

        UPInt allocSize = Alg::PMin(Params.MemLimit - getTotalSize(), mbs.GetGranularity());
        if (size <= allocSize)
        {
            MeshBuffer* pbuff = mbs.CreateBuffer(allocSize, MeshBuffer::AT_Chunk, 0, pHeap, pHal);
            if (pbuff)
            {
                ChunkBuffers.PushBack(pbuff);
#ifdef SF_RENDER_LOG_CACHESIZE
                LogDebugMessage(Log_Message, "Cache grew to %dK\n", getTotalSize() / 1024);
#endif
                goto alloc_size_available;
            }
        }
    }

    if (CacheList.EvictLRU(MCL_LRUTail, mbs.GetAllocator(), size))
        goto alloc_size_available;

    if (VBSizeEvictedInMap > Params.VBLockEvictSizeLimit)
        return false;

    // 2) Apply MRU (most recently used) swapping to the current frame content.
    // NOTE: MRU (GetFirst(), pNext iteration) gives
    //       2x improvement here with "Stars" test swapping.
    pitems = (MeshCacheItem*)CacheList.GetSlot(MCL_PrevFrame).GetFirst();
    while(!CacheList.GetSlot(MCL_PrevFrame).IsNull(pitems))
    {
        p      = pitems;
        pitems = (MeshCacheItem*)p->pNext;

        if (Evict(p, &mbs.GetAllocator()) >= size)
            goto alloc_size_available;
    }

    pitems = (MeshCacheItem*)CacheList.GetSlot(MCL_ThisFrame).GetFirst();
    while(!CacheList.GetSlot(MCL_ThisFrame).IsNull(pitems))
    {
        p      = pitems;
        pitems = (MeshCacheItem*)p->pNext;

        if (Evict(p, &mbs.GetAllocator()) >= size)
            goto alloc_size_available;
    }
    return false;

    // At this point we know we have a large enough block either due to
    // swapping or buffer growth, so allocation shouldn't fail.
alloc_size_available:
    if (!mbs.Alloc(size, pbuffer, poffset))
    {
        SF_ASSERT(0);
        return false;
    }

    return true;
}


// Generates meshes and uploads them to buffers.
bool MeshCache::PreparePrimitive(PrimitiveBatch* pbatch,
                                 MeshCacheItem::MeshContent &mc,
                                 bool waitForCache)
{
    Primitive* prim = pbatch->GetPrimitive();

    if (mc.IsLargeMesh())
    {
        SF_ASSERT(mc.GetMeshCount() == 1);
        MeshResult mr = GenerateMesh(mc[0], prim->GetVertexFormat(),
                                     pbatch->pFormat, 0, waitForCache);

        if (mr.Succeded())
            pbatch->SetCacheItem(mc[0]->CacheItems[0]);
        // Return 'false' if we just need more cache, to flush and retry.
        if (mr == MeshResult::Fail_LargeMesh_NeedCache)
            return false;
        return true;
    }

    // NOTE: We always know that meshes in one batch fit into Mesh Staging Cache.
    unsigned totalVertexCount, totalIndexCount;
    pbatch->CalcMeshSizes(&totalVertexCount, &totalIndexCount);

    SF_ASSERT(pbatch->pFormat);

    Render::MeshCacheItem* batchData = 0;
    unsigned       destVertexSize = pbatch->pFormat->Size;
    UByte*         pvertexDataStart;
    IndexType*     pindexDataStart;
    AllocResult    allocResult;

    allocResult = AllocCacheItem(&batchData, &pvertexDataStart, &pindexDataStart,
                                 MeshCacheItem::Mesh_Regular, mc,
                                 totalVertexCount * destVertexSize,
                                 totalVertexCount, totalIndexCount, waitForCache);
    if (allocResult != Alloc_Success)
    {
        // Return 'true' for state error (we can't recover by swapping cache and re-trying).
        return (allocResult == Alloc_Fail) ? false : true;
    }

    pbatch->SetCacheItem(batchData);

    // Prepare and Pin mesh data with the StagingBuffer.
    StagingBufferPrep meshPrep(this, mc, prim->GetVertexFormat(), false);

    // Copy meshes into the Vertex/Index buffers.

    // All the meshes have been pinned, so we can
    // go through them and copy them into buffers
    UByte*      pstagingBuffer   = StagingBuffer.GetBuffer();
    const VertexFormat* pvf      = prim->GetVertexFormat();
    const VertexFormat* pdvf     = pbatch->pFormat;

    unsigned    i;
    unsigned    indexStart = 0;

    for(i = 0; i< mc.GetMeshCount(); i++)
    {
        Mesh* pmesh = mc[i];
        SF_ASSERT(pmesh->StagingBufferSize != 0);

        // Convert vertices and initialize them to the running index
        // within this primitive.
        void*   convertArgArray[1] = { &i };
        ConvertVertices_Buffered(*pvf, pstagingBuffer + pmesh->StagingBufferOffset,
                                 *pdvf, pvertexDataStart,
                                 pmesh->VertexCount, &convertArgArray[0]);
        // Copy and assign indices.
        ConvertIndices(pindexDataStart,
            (IndexType*)(pstagingBuffer + pmesh->StagingBufferIndexOffset),
            pmesh->IndexCount, (IndexType)indexStart);

        pvertexDataStart += pmesh->VertexCount * destVertexSize;
        pindexDataStart  += pmesh->IndexCount;
        indexStart       += pmesh->VertexCount;
    }

    // ~StagingBufferPrep will Unpin meshes in the staging buffer.
    return true;
}

MeshCache::AllocResult
MeshCache::AllocCacheItem(Render::MeshCacheItem** pdata,
                          UByte** pvertexDataStart, IndexType** pindexDataStart,
                          MeshCacheItem::MeshType meshType,
                          MeshCacheItem::MeshBaseContent &mc,
                          UPInt vertexBufferSize,
                          unsigned vertexCount, unsigned indexCount,
                          bool waitForCache)
{
    if (!AreBuffersMapped() && !LockBuffers())
        return Alloc_StateError;

    // Compute and allocate appropriate VB/IB space.
    UPInt       vbOffset = 0, ibOffset = 0;
    MeshBuffer  *pvb = 0, *pib = 0;
    UByte       *pvdata, *pidata;
    MeshCache::AllocResult failType = Alloc_Fail;

    if (!allocBuffer(&vbOffset, &pvb, VertexBuffers,
                     vertexBufferSize, waitForCache))
    {
handle_alloc_fail:
        if (pvb) VertexBuffers.Free(vertexBufferSize, pvb, vbOffset);
        if (pib) IndexBuffers.Free(indexCount * sizeof(IndexType), pib, ibOffset);        
        return failType;
    }
    if (!allocBuffer(&ibOffset, &pib, IndexBuffers,
                     indexCount * sizeof(IndexType), waitForCache))
        goto handle_alloc_fail;

    pvdata = pvb->Map(MappedBuffers);
    pidata = pib->Map(MappedBuffers);

    if (!pvdata || !pidata)
        goto handle_alloc_fail;

    // Create new MeshCacheItem; add it to hash.
    *pdata = MeshCacheItem::Create(meshType, &CacheList, mc, pvb, pib,
                    (unsigned)vbOffset, vertexBufferSize, vertexCount,
                    (unsigned)ibOffset, indexCount * sizeof(IndexType), indexCount);

    if (!*pdata)
    {
        // Memory error; free buffers, skip mesh.
        SF_ASSERT(0);
        failType = Alloc_StateError;
        goto handle_alloc_fail;
    }

    *pvertexDataStart = pvdata + vbOffset;
    *pindexDataStart  = (IndexType*)(pidata + ibOffset);
    return Alloc_Success;
}


}}}; // namespace Scaleform::Render::GL

