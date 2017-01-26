/**********************************************************************

Filename    :   SimpleMeshCache.h
Content     :   Implementation of mesh cache with single Allocator,
                intended as a base class for console mesh caches.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_SimpleMeshCache_H
#define INC_SF_Render_SimpleMeshCache_H

#include "Render_MeshCache.h"

namespace Scaleform { namespace Render {

class SimpleMeshBuffer;
class SimpleMeshCacheItem;
class SimpleMeshCache;


// SimpleMeshBuffer is a base MeshBuffer class that any MeshCache implementation
// that is derived from SimpleMeshCache should use.

class SimpleMeshBuffer : public Render::MeshBuffer
{
    friend class SimpleMeshCache;
public:

    enum {
        MinGranularity  = 16*1024,
        // We pass a smaller size to the Allocator to prevent it from merging
        // blocks which can be reported consecutively by PhysicalAlloc.
        // Users should still allocate and map full original size to the buffer.
        AllocSizeDelta  = sizeof(UPInt)
    };
   
    SimpleMeshBuffer(UPInt size, AllocType type, unsigned arena)
        : Render::MeshBuffer(size - AllocSizeDelta, type, arena)
    {        
        SF_ASSERT(size > AllocSizeDelta);
    }

    inline UPInt   GetFullSize() const { return GetSize() + AllocSizeDelta; }

    // Returns user maximum allocatable size given a specified buffer size.    
    inline static UPInt GetUsableSize(UPInt size) { return size - AllocSizeDelta; }
};


// SimpleMeshCacheItem stores an additional MeshBuffer pointer and
// and allocation/vertex offset data, in addition to base MeshCacheItem content.
// This class should be used as a base instead of Render::MeshCacheItem by
// all SimpleMeshCache derived mesh cache implementations.

class SimpleMeshCacheItem : public Render::MeshCacheItem
{
    friend class SimpleMeshCache;
protected:
    SimpleMeshBuffer* pBuffer;
    // Global memory address of allocation.
    UPInt             AllocAddress;
    // Start of vertex and index buffer regions. In some back ends, us as X360,
    // this is measured in bytes from the start of buffer; in others it can
    // be an absolute memory location.
    UPInt             VertexOffset, IndexOffset;

    // Format of the vetex data within the buffer.
    const VertexFormat * pFormat;

public:
    static SimpleMeshCacheItem* Create(MeshType type,
                                       MeshCacheListSet* pcacheList, UPInt classSize,
                                       MeshBaseContent& mc, SimpleMeshBuffer* pbuffer,
                                       UPInt allocAddress, UPInt allocSize,
                                       UPInt vertexOffset, unsigned vertexCount,
                                       UPInt indexOffset, unsigned indexCount, const VertexFormat * pfmt )
    {
        SimpleMeshCacheItem* p = (SimpleMeshCacheItem*)
            Render::MeshCacheItem::Create(type, pcacheList, classSize,
                                          mc, allocSize, vertexCount, indexCount);
        if (p)
        {
            p->pBuffer      = pbuffer;
            p->AllocAddress = allocAddress;
            p->VertexOffset = vertexOffset;
            p->IndexOffset  = indexOffset;
            p->pFormat      = pfmt;
        }
        return p;
    }

	SimpleMeshBuffer*   GetBuffer() const { return pBuffer; }
    UPInt			    GetAllocAddr() const { return AllocAddress; }
    UPInt			    GetVertexOffset() const { return VertexOffset; }
    UPInt			    GetIndexOffset() const { return IndexOffset; }
    const VertexFormat* GetVertexFormat() const { return pFormat; }
};


// Rendering synchronization class.
// Used by SimpleMeshCache::allocBuffer() to know when different cache
// buffers have completed rendering and can thus be discarded.

class RenderSync
{
public:
    virtual ~RenderSync() { }

    virtual void    BeginFrame() { }
    virtual void    EndFrame() { }

    // Trigger all queued-up HW rendering commands. After this is executed,
    // we can wait for fences. If this is not called, fence waiting may fail.
    virtual void    KickOffFences()         = 0;

    // Write a fence into the command buffer and return a value to pass to WaitFence.
    virtual UPInt   SetFence()              = 0;

    // Check if all drawing prior to the specified fence is complete.
    virtual bool    IsPending(UPInt f)     = 0;
    // Wait for (at least) all drawing prior to the specified fence
    // to complete before returning.
    virtual void    WaitFence(UPInt f)     = 0;
};



// SimpleMeshCache is a partial mesh cache implementation used to simplify
// platform-specific MeshCache implementation on consoles. It has the following
// characteristics:
//  - Supports allocation of multiple MeshBuffer blocks, delegating the actual
//    allocation to the createHWBuffer()/destroyHWBuffer() virtual functions.
//  - Handles grow/shrink of the cache.
//  - Implements allocBuffer() function that should be used by the PreparePrimitive
//    implementation in the derived class.
//  - Used passed RenderSync object to implement fencing.

class SimpleMeshCache : public Render::MeshCache
{   
protected:
    typedef MeshBuffer::AllocType AllocType;

    MeshCacheListSet        CacheList;
    AllocAddr               Allocator;
    UPInt                   TotalSize;
    List<Render::MeshBuffer> Buffers; // Use base MeshBuffer to avoid List<> problems.
    RenderSync*             pRenderSync;
    
    inline MeshCache*   getThis() { return this; }

    // Allocates Vertex/Index buffer of specified size and adds it to free list.
    SimpleMeshBuffer*   allocMeshBuffer(UPInt size, AllocType atype, unsigned arena = 0);
    void                releaseMeshBuffer(SimpleMeshBuffer* pbuffer);

    // Allocates reserve buffer, if any. Intended for use in Initialize implementation.
    bool                allocateReserve();
    // Cleared cache and releases all buffers including reserve. Intended for
    // use in MeshCache::Reset implementation.
    void                releaseAllBuffers();    

    void                evictMeshesInBuffer(MeshCacheListSet::ListSlot* plist, UPInt count,
                                            SimpleMeshBuffer* pbuffer);

    // Allocates a number of bytes in the specified buffer, while evicting LRU data.
    // Buffer can contain either vertex and/or index data.
    bool                allocBuffer(UPInt* poffset, UPInt size, bool waitForCache);
    // Finds which buffer address offset returned by allocBuffer() belongs to...
    SimpleMeshBuffer*   findBuffer(UPInt address);


    // *** Virtual HW API to be implemented by derived class

    // Creates a SimpleMeshBuffer derived class and initializes its pData with
    // hw-allocated memory block of size.
    virtual SimpleMeshBuffer* createHWBuffer(UPInt size, AllocType atype, unsigned arena) = 0;
    // Frees buffers memory and deletes HW buffer.
    virtual void              destroyHWBuffer(SimpleMeshBuffer* pbufer) = 0;


public:
    SimpleMeshCache(MemoryHeap* pheap,
                    const MeshCacheParams& params,
                    RenderSync* psync);
    ~SimpleMeshCache();

    virtual void    ClearCache();
    virtual void    EndFrame();

    // Adds a fixed-size buffer to cache reserve; expected to be released at Release.
    //virtual bool    AddReserveBuffer(unsigned size, unsigned arena = 0);
    //virtual bool    ReleaseReserveBuffer(unsigned size, unsigned arena = 0);
    
    virtual UPInt   Evict(Render::MeshCacheItem* p, AllocAddr* pallocator = 0, MeshBase* pmesh = 0);
};


}};  // namespace Scaleform::Render

#endif
