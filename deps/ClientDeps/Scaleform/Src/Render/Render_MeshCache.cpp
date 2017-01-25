/**********************************************************************

Filename    :   MeshCache.h
Content     :   New renderer Mesh Cache implementation
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render_MeshCache.h"
#include "Kernel/SF_HeapNew.h"

#include <stdio.h>


namespace Scaleform { namespace Render {
  

// ***** MeshCacheItem

void MeshCacheItemUseNode::SetMeshItem(MeshCacheItem* p)
{             
    if (pMeshItem)
        RemoveNode();
    pMeshItem = p;
    if (p)
        p->PrimitiveBatches.PushFront(this);
}


MeshCacheItem*  MeshCacheItem::Create(
                    MeshCacheItem::MeshType type,
                    MeshCacheListSet* pcacheList, UPInt classSize,
                    MeshBaseContent& content,
                    UPInt allocSize,
                    unsigned vertexCount, unsigned indexCount)
{
    classSize = (classSize + sizeof(UPInt)-1) & ~(sizeof(UPInt)-1);
    
    const unsigned meshCount = content.GetMeshCount();
    MeshCache*     pcache = pcacheList->GetCache();
    MeshCacheItem* p = (MeshCacheItem*)
        SF_HEAP_ALLOC(pcache->pHeap,
                      classSize + meshCount * sizeof(MeshBase*),
                      StatRender_MeshBatch_Mem);
    if (!p) return 0;
    
    p->Type         = type;
    p->pCacheList   = pcacheList;
    p->HashKey      = content.HashKey;
    p->MeshCount    = meshCount;
    p->pMeshes      = (MeshBase**)(((UPInt)p) + classSize);
    for (unsigned i = 0; i < meshCount; i++)
        p->pMeshes[i] = content[i];
 
    // GList is not initialized by default, so do it here.
    p->PrimitiveBatches.Clear();
    
    p->AllocSize     = allocSize;
    p->VertexCount   = vertexCount;
    p->IndexCount    = indexCount;
    p->Fence         = 0;

    /* Target-Specific logic; implemented in MeshCacheItem of derived class. 
    p->AllocOffset   = allocOffset;
    p->AllocSize     = allocSize;
    p->VBAllocOffset = vertexOffset;
    p->IBAllocOffset = indexOffset;
    */
    
    if (type == Mesh_Regular)
    {
        // We used Add_NoDuplicate to allow batch MeshCacheItem eviction in ~Mesh to
        // work properly, as it can only evict each mesh once.
        //  - Also, this keeps the list smaller.
        for (unsigned i=0; i< meshCount; i++)
            ((Mesh*)content[i])->CacheItems.Add_NoDuplicate(p);
        pcache->BatchCacheItemHash.Add(p);
    }
    else
    {
        // Complex meshes are not placed in Hash, since they
        // have a single direct pointer to MeshCacheItem.
        SF_ASSERT(type == Mesh_Complex);
        ((ComplexMesh*)p->pMeshes[0])->pCacheMeshItem = p;
    }
    
    // Add item to cache caching list so that Remove() can be called on it later.
    pcacheList->PushFront(MCL_Uncached, p);    
    //pcacheList->Slots[MCL]

    return p;
}


// Removes all references to us from MeshCache and PrimitiveBatch objects and frees
// batch memory. Exception: Doesn't try to remove reference from our pskipMesh. 
void    MeshCacheItem::Destroy(MeshBase* pskipMesh)
{
    // - Remove from cache list.
    // - Remove from hash table.
    // - Clear out references in all PrimitiveBatch structures that reference us.
    // - Update meshes that have pointers to use.
    // - Free our memory.

    pCacheList->RemoveNode(this);

    MeshCacheItemUseNode* puseNode = PrimitiveBatches.GetFirst();
    while (!PrimitiveBatches.IsNull(puseNode))
    {
        puseNode->pMeshItem = 0;
        puseNode = puseNode->pNext;
    }
    PrimitiveBatches.Clear();


    if (Type == Mesh_Regular)
    {
        GetCache()->BatchCacheItemHash.Remove(this);

        // A mesh can be killed by user-eviction inside of RemoveMeshCacheItem call;
        // this can cause a crash in case our pMeshes array has duplicates
        // which are not matched inside of Mesh::CacheItems array. 
        // We AddRef/Release meshes to avoid this early death problem.
        unsigned i;
        for (i = 0; i< MeshCount; i++)
            if (pMeshes[i] != pskipMesh)
                pMeshes[i]->AddRef();

        for (i = 0; i< MeshCount; i++)
            if (pMeshes[i] != pskipMesh)
                ((Mesh*)pMeshes[i])->RemoveMeshCacheItem(this);

        for (i = 0; i< MeshCount; i++)
            if (pMeshes[i] != pskipMesh)
                pMeshes[i]->Release();
    }
    else
    {
        SF_ASSERT(MeshCount == 1);
        SF_ASSERT(Type == Mesh_Complex);
        if (pMeshes[0] != pskipMesh)
            ((ComplexMesh*)pMeshes[0])->RemoveMeshCacheItem(this);
    }
    
    SF_FREE(this);
}

// Find MeshCacheItem from which mesh data can be copied; used on consoles where
// video memory is readable.
MeshCacheItem* MeshCacheItem::FindMeshSourceBatch(Mesh *pmesh,
                                                  unsigned *pprevVertexCount,
                                                  unsigned *pprevIndexCount,
                                                  MeshCacheItem* pskipBatch)
{
    // If mesh is not in a staging buffer, it must be a part of a batch cached somewhere else.
    SF_ASSERT(pmesh->CacheItems.GetSize() != 0);            

    // Find the batch that has our mesh, then our mesh location in that batch
    // for the purposes of copying vertex and index data.
    MeshCacheItem*  psourceCacheItem = 0;
    MeshCacheItem** pmeshBuffer = pmesh->CacheItems.GetBuffer();

    for(unsigned bm = 0; bm < pmesh->CacheItems.GetSize(); bm++)
    {
        if ((pmeshBuffer[bm] != 0) && (pmeshBuffer[bm] != pskipBatch))
        {
            psourceCacheItem = pmeshBuffer[bm];
            break;
        }
    }
    SF_ASSERT(psourceCacheItem != 0);
    SF_ASSERT(psourceCacheItem->Type == Mesh_Regular);

    // Find our mesh's VB/IB offsets within this batches data chunk.
    unsigned    imesh;
    *pprevVertexCount = 0;
    *pprevIndexCount  = 0;
    for (imesh = 0; imesh < psourceCacheItem->MeshCount; imesh++)
    {                
        MeshBase* psourceMesh = psourceCacheItem->pMeshes[imesh];
        if (pmesh == psourceMesh)
            break;
        *pprevVertexCount += psourceMesh->VertexCount;
        *pprevIndexCount  += psourceMesh->IndexCount;
    }
    SF_ASSERT(imesh != psourceCacheItem->MeshCount);

    return psourceCacheItem;
}



// Informs us that a mesh containing us is deleted. This call
// results in this MeshCacheItem also being deleted.
void MeshCacheItem::NotifyMeshRelease(MeshBase* pmesh)
{   
    pCacheList->GetCache()->Evict(this, 0, pmesh);
}

MeshUseStatus MeshCacheItem::GetUseStatus() const
{
    return pCacheList->GetCache()->GetItemUseStatus(this);
}

//------------------------------------------------------------------------
// ***** MeshCacheListSet

bool    MeshCacheListSet::EvictLRU(ListSlot& list, AllocAddr& a, UPInt size)
{
    MeshCacheItem* pitems, *p;

    pitems = list.GetLast();
    while(!list.IsNull(pitems))
    {
        p      = pitems;
        pitems = p->pPrev;
        if (pCache->Evict(p, &a) >= size)
            return true;
    }
    return false;
}

bool    MeshCacheListSet::EvictLRUTillLimit(ListSlot& list, AllocAddr& a, UPInt size, UPInt limit)
{
    MeshCacheItem* pitems, *p;
    
    pitems = list.GetLast();
    while(!list.IsNull(pitems) && (list.Size > limit))
    {
        p      = pitems;
        pitems = p->pPrev;
        if (pCache->Evict(p, &a) >= size)
            return true;
    }
    return false;
}

void    MeshCacheListSet::EvictAll()
{
    unsigned i;
    for (i = 0; i < MCL_ItemCount; i ++)
    {
        while(!Slots[i].IsEmpty())
            pCache->Evict(Slots[i].GetFirst());
        SF_ASSERT(Slots[i].Size == 0);
    }
}

void    MeshCacheListSet::EndFrame()
{
    SF_ASSERT(Slots[MCL_InFlight].IsEmpty());
    PushListToFront(MCL_LRUTail, MCL_PrevFrame);
    PushListToFront(MCL_PrevFrame, MCL_ThisFrame);
    SF_ASSERT(Slots[MCL_ThisFrame].IsEmpty());
}




// ***** Mesh Output

class MeshVertexOutput : public VertexOutput
{
public:
    typedef MeshCache::MeshResult MeshResult;

    MeshVertexOutput(Mesh *mesh, MeshCache* cache, 
                     const VertexFormat* sourceFormat,
                     const VertexFormat* singleFormat,
                     const VertexFormat* batchFormat,
                     bool waitForCache)
        : pCache(cache), WaitForCache(waitForCache),
          pMesh(mesh),
          pSourceFormat(sourceFormat),
          pSingleFormat(singleFormat),  pBatchFormat(batchFormat),
          Result(MeshResult::Fail_General),
          pVertexDataStart(0), pIndexDataStart(0)
    {        
    }

    const MeshResult& GetResult() const { return Result; }

    virtual bool BeginOutput(const Fill* fills, unsigned fillCount,
                             const Matrix2F& vertexMatrix)
    {
        //SF_ASSERT(fillCount == 1);
        SF_UNUSED(fillCount);
        //const VertexFormat* pformat = pMesh->Data.pInterface->GetVertexFormat(hmesh);
        //const VertexFormat* pformat = pMesh->GetVertexFormat();
        // TBD: Verify format matching

        // Determine if this is a "large mesh", allocated directly.
        // If large mesh is already set, we don't need batch format.
        if (pMesh->LargeMesh ||
            (pBatchFormat && (fills->VertexCount * pBatchFormat->Size >
                              pCache->Params.NoBatchVerticesSizeThreshold)))
        {
            Render::MeshCacheItem* batchData;
            MeshCacheItem::MeshBaseContent mc((MeshBase**)&pMesh, 1);
            UPInt vertexBufferSize = fills->VertexCount * pSingleFormat->Size;

            // Assign mesh sizes; AllocateMesh does this as well.
            pMesh->VertexCount = fills->VertexCount;
            pMesh->IndexCount  = fills->IndexCount;
            pMesh->LargeMesh   = true;

            MeshCache::AllocResult allocState = 
                pCache->AllocCacheItem(&batchData, &pVertexDataStart, &pIndexDataStart,
                                       MeshCacheItem::Mesh_Regular, mc,
                                       vertexBufferSize,
                                       fills->VertexCount, fills->IndexCount,
                                       WaitForCache);

            if (allocState == MeshCache::Alloc_Success)
            {
                setResult(MeshResult::Success_LargeMesh);
                pCache->MoveToCacheListFront(MCL_ThisFrame, batchData);
            }
            else if (allocState == MeshCache::Alloc_Fail)
            {
                setResult(MeshResult::Fail_LargeMesh_NeedCache);
            }
        }
        else
        {
            SF_ASSERT(!pMesh->LargeMesh);

            // Batching allocates space in staging buffer.
            if (getStagingBuffer()->AllocateMesh(pMesh, fills->VertexCount,
                                                 pSourceFormat->Size, fills->IndexCount))
                setResult(MeshResult::Success_Staging);
            else
                setResult(MeshResult::Fail_Staging_NoBuffer);
        }

        pMesh->VertexMatrix = vertexMatrix;
        return Result.Succeded();
    }

    virtual void EndOutput()
    {
        // We could do some error checking here to make sure the
        // entire data range was uploaded...
    }

    virtual void SetVertices(unsigned fillIndex, unsigned vertexOffset,
                             void* vertices, unsigned vertexCount)
    {
        if ((fillIndex != 0) || !Result.Succeded())
            return;        
        SF_ASSERT(pSourceFormat && (fillIndex == 0));
        SF_UNUSED(fillIndex);

        if (!Result.IsLargeMesh())
        {
            UByte* pv = getStagingBufferDest() + pMesh->StagingBufferOffset;
            memcpy(pv + pSourceFormat->Size * vertexOffset,
                   vertices, vertexCount * pSourceFormat->Size);
        }
        else
        {
            // Do copy-conversion to mesh buffer.
            UByte* dest = pVertexDataStart + vertexOffset * pSingleFormat->Size;
            ConvertVertices_Buffered(*pSourceFormat, vertices,
                                     *pSingleFormat, dest, vertexCount);
        }
    }


    virtual void SetIndices(unsigned fillIndex, unsigned indexOffset,
                            UInt16* indices, unsigned indexCount)
    {
        if ((fillIndex != 0) || !Result.Succeded())
            return;
        SF_ASSERT(pSourceFormat && (fillIndex == 0));
        SF_UNUSED(fillIndex);

        IndexType* dest;

        if (!Result.IsLargeMesh())
            dest = (IndexType*)(getStagingBufferDest() + pMesh->StagingBufferIndexOffset);
        else
            dest = pIndexDataStart;

        memcpy(dest + indexOffset, indices, sizeof(IndexType) * indexCount);
    }

private:    
    MeshStagingBuffer*  getStagingBuffer()     { return &pCache->StagingBuffer; }
    UByte*              getStagingBufferDest() { return pCache->StagingBuffer.GetBuffer(); }
    void                setResult(MeshResult::ResultType result) { Result.Value = result; }

    MeshCache*          pCache;
    bool                WaitForCache;
    Mesh*               pMesh;    
    const VertexFormat* pSourceFormat;
    // Destination formats we generate for single/batch meshes.
    const VertexFormat* pSingleFormat;
    const VertexFormat* pBatchFormat;

    // Result state for directly-allocated meshes.
    MeshCache::MeshResult Result;
    UByte*              pVertexDataStart;
    IndexType*          pIndexDataStart;
};


//------------------------------------------------------------------------
// ***** MeshStagingBuffer

MeshStagingBuffer::MeshStagingBuffer()
: pBuffer(0), BufferSize(0), TotalPinnedSize(0), PinSizeLimit(0)
{  
}

MeshStagingBuffer::~MeshStagingBuffer()
{
    Reset();
}

bool MeshStagingBuffer::Initialize(MemoryHeap* pheap, UPInt size)
{
    if (pBuffer && (size != BufferSize))
        Reset();
    pBuffer = (UByte*)SF_HEAP_ALLOC(pheap, size, StatRender_Buffers_Mem);
    if (!pBuffer)
        return false;
    BufferSize  = size;
    TotalPinnedSize = 0;
    PinSizeLimit = size / 2;
    return true;
}

void MeshStagingBuffer::Reset()
{
    if (!pBuffer)
        return;

    // Remove all nodes and mark then as not having staging data.
    while (!MeshList.IsEmpty())
    {
        MeshStagingNode *p = MeshList.GetFirst();
        p->StagingBufferOffset      = 0;
        p->StagingBufferIndexOffset = 0;
        p->RemoveNode();
        // We must call RemoveNode first, since OnStagingNodeEvict may remove 
        // other meshes through callback in MeshKey, corrupting pPrev/pNext.
        p->OnStagingNodeEvict();
    }

    SF_FREE(pBuffer);
    pBuffer    = 0;
    BufferSize = 0;
    TotalPinnedSize = 0;
}


// Allocated space in the circular buffer, while potentially swapping out
// unpinned meshes.
bool   MeshStagingBuffer::AllocateBufferSpace(UPInt * poffset, UPInt size)
{
    if (size > PinSizeLimit)
        return false;

    // Advance head so that it doesn't bump into tail.
    SF_ASSERT(size <= (BufferSize - TotalPinnedSize));

    while(!MeshList.IsEmpty())
    {
        MeshStagingNode* phead = MeshList.GetFirst();
        MeshStagingNode* ptail = MeshList.GetLast();

        UPInt head = phead->StagingBufferOffset + phead->StagingBufferSize;
        UPInt tail = ptail->StagingBufferOffset;

        UPInt spaceAvailable = BufferSize - head;

        if (head <= tail)
        {
            spaceAvailable = tail - head;
        }
        else
        {
            UPInt spaceAvailable2 = BufferSize - head;
            if (spaceAvailable2 >= size)
            {
                *poffset = head;
                return true;
            }
            spaceAvailable = tail;
        }

        if (spaceAvailable >= size)
        {
            *poffset = tail - spaceAvailable;
            return true;
        }

        // Evict tail, or shuffle tail up front if it is pinned.
        ptail->RemoveNode();

        if (ptail->PinCount)
        {
            if (spaceAvailable)
            {                    
                memcpy(pBuffer + ptail->StagingBufferOffset - spaceAvailable,
                       pBuffer + ptail->StagingBufferOffset, ptail->StagingBufferSize);
                ptail->StagingBufferIndexOffset -= spaceAvailable;
                ptail->StagingBufferOffset      -= spaceAvailable;
            }
            MeshList.PushFront(ptail);
        }
        else
        {
            // Tail was evicted.
            //ptail->StagingBufferSize = 0;
            ptail->OnStagingNodeEvict();
        }
    }

    // Mesh list is empty, allocate from beginning.
    *poffset = 0;
    return true;   
}

bool    MeshStagingBuffer::AllocateMesh(Mesh *pmesh, UPInt vertexCount, UPInt vertexSize,
                                        UPInt indexCount)
{
    UPInt       verticesSize = vertexCount * vertexSize;
    UPInt       indicesSize  = indexCount * sizeof(IndexType);
    // Round up sizes to 16.
    UPInt       totalSize    = ((verticesSize + indicesSize) + 15) & ~15;

    // This allocation should only fail if mesh is too big. We can find
    // alternative means of handling this in the future.
    UPInt       offset;

    if (!AllocateBufferSpace(&offset, totalSize))
        return false;

    pmesh->StagingBufferOffset      = offset;
    pmesh->StagingBufferSize        = totalSize;
    pmesh->StagingBufferIndexOffset = offset + verticesSize;
    pmesh->VertexCount = (unsigned)vertexCount;
    pmesh->IndexCount  = (unsigned)indexCount;

    //printf("Staging alloc: mesh=%p SOffset = %d vcount=%d icount=%d\n",
    //       pmesh, offset, vertexCount, indexCount);

    // Add mesh to head of the buffer.
    MeshList.PushFront(pmesh);
    return true;
}



// ***** MeshCache::StagingBufferPrep


MeshCache::StagingBufferPrep::StagingBufferPrep(MeshCache* cache,
                                                MeshCacheItem::MeshContent &mc,
                                                const VertexFormat* format,
                                                bool canCopyData,
                                                MeshCacheItem * skipBatch)
    : pCache(cache), MC(mc)
{
    MeshStagingBuffer& sbuffer = cache->StagingBuffer;
    const unsigned     meshCount = mc.GetMeshCount();
    SF_ASSERT(meshCount <= 256);

    unsigned imesh;

    // With buffer data allocate, generate the meshes we need if they
    // are not already in a staging buffer or a part of another mesh.

    // b) Lock all meshes that do exist.
    for (imesh = 0; imesh < meshCount; imesh++)
    {
        Mesh* pmesh = MC[imesh];
        if (sbuffer.IsCached(pmesh))
        {
            sbuffer.PinMesh(pmesh);
            PinnedFlagArray[imesh] = true;
        }
        else
        {
            PinnedFlagArray[imesh] = false;
        }
    }

    // c) Generate meshes that don't exist.
    if (canCopyData)
    {
        for (imesh = 0; imesh < meshCount; imesh++)
        {
            Mesh* pmesh = MC[imesh];

			// Ensure that if we aren't pinned, that we have a valid cache item to copy from. skipBatch 
			// is intended to be the new cache item to be copied into, so we cannot copy from it, if it is the only one.
            if (!PinnedFlagArray[imesh] && (pmesh->CacheItems.GetSize() == 0 || 
                (pmesh->CacheItems.GetSize() == 1 && pmesh->CacheItems[0] == skipBatch)))
            {
                if (!sbuffer.IsCached(pmesh))
                {
                    MeshResult mr = pCache->GenerateMesh(pmesh, format, 0,0, false);
                    // Any mesh that fit into a staging buffer before
                    // should fit into it now.
                    SF_ASSERT(mr.Succeded() || !mr.IsLargeMesh());
                }
                sbuffer.PinMesh(pmesh);
            }
        }
    }
    else
    {
        for (imesh = 0; imesh < meshCount; imesh++)
        {
            if (!PinnedFlagArray[imesh])
            {
                Mesh* pmesh = MC[imesh];
                if (!sbuffer.IsCached(pmesh))
                {
                    MeshResult mr = pCache->GenerateMesh(pmesh, format, 0,0, false);
                    // Any mesh that fit into a staging buffer before
                    // should fit into it now.
                    SF_ASSERT(mr.Succeded() || !mr.IsLargeMesh());
                }
                sbuffer.PinMesh(pmesh);
            }
        }
    }
}

MeshCache::StagingBufferPrep::~StagingBufferPrep()
{
    // Unpin meshes!
    MeshStagingBuffer& sbuffer = pCache->StagingBuffer;
    for (unsigned imesh = 0; imesh < MC.GetMeshCount(); imesh++)
    {
        if (sbuffer.IsCached(MC[imesh]))
            sbuffer.UnpinMesh(MC[imesh]);
    }
}


//------------------------------------------------------------------------
// ***** Complex mesh version

// ComplexMeshVertexOutput allocates vertices directly from cache, an copies
// them there (unlike MeshVertexOutput, which goes through staging buffer).

class ComplexMeshVertexOutput : public VertexOutput
{
public:
    typedef MeshCache::AllocResult AllocResult;

    ComplexMeshVertexOutput(ComplexMesh *mesh, MeshCache* cache,
                        HAL* hal, bool waitForCache)
    : pMesh(mesh), pCache(cache), pHAL(hal), WaitForCache(waitForCache),
  AllocState(MeshCache::Alloc_StateError)
    {
    }

    AllocResult GetResult() const { return AllocState; }

    virtual bool    BeginOutput(const Fill* fills, unsigned fillCount,
                                const Matrix2F& vertexMatrix)
    {
    // Compute the size of vertex and index buffers, then allocate.
    // This alloc converts fills into target.
    UPInt    vertexBufferSize;
    unsigned vertexCount, indexCount;
    if (!pMesh->InitFillRecords(fills, fillCount, vertexMatrix, pHAL,
                                &vertexBufferSize, &vertexCount, &indexCount))
        return false;

    Render::MeshCacheItem* batchData;
    MeshCacheItem::MeshBaseContent mc((MeshBase**)&pMesh, 1);
            
    AllocState = pCache->AllocCacheItem(&batchData, &pVertexDataStart, &pIndexDataStart,
                                       MeshCacheItem::Mesh_Complex, mc,
                                       vertexBufferSize, vertexCount, indexCount,
                                       WaitForCache);
    pFills = fills;
    return (AllocState == MeshCache::Alloc_Success);
    }

    virtual void    EndOutput()
    {        
        pCache->PostUpdateMesh( pMesh->GetCacheItem() );
    }

    virtual void    SetVertices(unsigned fillIndex, unsigned vertexOffset,
                                void* vertices, unsigned vertexCount)
    {
    if (AllocState != MeshCache::Alloc_Success)
        return;        
    const ComplexMesh::FillRecord* record = pMesh->GetFillRecords() + fillIndex;
    SF_ASSERT(fillIndex < pMesh->GetFillRecordCount());
    SF_ASSERT((vertexOffset + vertexCount) <= record->VertexCount);

    // Do copy-conversion.
    UByte* dest = pVertexDataStart + record->VertexByteOffset +
                  vertexOffset * record->pFormats[0]->Size;
    ConvertVertices_Buffered(*pFills[fillIndex].pFormat, vertices,
                             *record->pFormats[0], dest, vertexCount);
    }

    virtual void    SetIndices(unsigned fillIndex, unsigned indexOffset,
                               UInt16* indices, unsigned indexCount)
    {
    if (AllocState != MeshCache::Alloc_Success)
        return;        
    const ComplexMesh::FillRecord* record = pMesh->GetFillRecords() + fillIndex;
    SF_ASSERT(fillIndex < pMesh->GetFillRecordCount());
    SF_ASSERT((indexOffset + indexCount) <= record->IndexCount);
    // Copy indices.
    IndexType* dest = pIndexDataStart + record->IndexOffset + indexOffset;
    memcpy(dest, indices, sizeof(IndexType) * indexCount);
    }

private:
    ComplexMesh* pMesh;
    MeshCache*   pCache;
    HAL*         pHAL;
    bool         WaitForCache;
    const Fill*  pFills;
    AllocResult  AllocState;
    UByte*       pVertexDataStart;
    IndexType*   pIndexDataStart;
};


//------------------------------------------------------------------------
// ***** MeshCache

MeshCache::MeshCache(MemoryHeap *pheap, const MeshCacheParams& params)
: pHeap(pheap), Params(params), pRQCaches(0)
{
    // TBD: Need to init MaxMatrixIndex based on HW capabilities.
}

MeshCache::~MeshCache()
{
}

void MeshCache::SetRQCacheInterface(RQCacheInterface* rqCaches)
{
    if (pRQCaches != rqCaches)
    {
        if (pRQCaches)
        {
            pRQCaches->ClearCacheLocked(Cache_Mesh);
            pRQCaches->SetCache(Cache_Mesh, 0);
        }
        pRQCaches = rqCaches;
        if (rqCaches)
        {
            pRQCaches->SetCache(Cache_Mesh, this);
            if (AreBuffersLocked())
                pRQCaches->SetCacheLocked(Cache_Mesh);
        }
    }
}

MeshUseStatus MeshCache::GetItemUseStatus(const MeshCacheItem* item)
{
    // Default implementation doesn't consider Fences; it's good enough for D3D9, etc.
    switch (item->GetListType())
    {    
    case MCL_LRUTail:   return MUS_LRUTail;
    case MCL_PrevFrame: return MUS_PrevFrame;
    case MCL_ThisFrame: return MUS_ThisFrame;
    case MCL_InFlight:  return MUS_InUse;
    case MCL_Uncached:
        break;
    }
    return MUS_Uncached;
}


MeshCache::MeshResult MeshCache::GenerateMesh(
                             Mesh* mesh,
                             const VertexFormat* sourceFormat,
                             const VertexFormat* singleFormat,
                             const VertexFormat* batchFormat,
                             bool waitForCache)
{
    MeshVertexOutput out(mesh, this,
                         sourceFormat, singleFormat, batchFormat,
                         waitForCache);
    mesh->GetProvider()->GetData(mesh, &out, mesh->GetMeshGenFlags());
    return out.GetResult();
}


bool MeshCache::PrepareComplexMesh(ComplexMesh* mesh, bool waitForCache)
{
    // Prepares the whole primitive as a unit.    
    if (!mesh || mesh->GetCacheItem())
        return true;

    // Determine tessellation size, tessellate.
    ComplexMeshVertexOutput out(mesh, this, mesh->GetHAL(), waitForCache);
    if (!mesh->GetProvider()->GetData(mesh, &out, mesh->GetMeshGenFlags()))
    {
        // Return true in case of tessellate failure so that no 'retry' is attempted.
        return true;
    }
    if (out.GetResult() == Alloc_Fail)
        return false;

    // Move pMeshItem closer in mesh cache to InFlight slot, to ensure
    // that it doesn't get swapped out.
    if (mesh->GetCacheItem())
    {
        SF_ASSERT(out.GetResult() == Alloc_Success);
        MoveToCacheListFront(MCL_InFlight, mesh->GetCacheItem());
    }

    // Both Alloc_Success and Alloc_StateError return true - means ok to Emit.
    return true;
}


}}; // namespace Scaleform::Render

