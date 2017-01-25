/**********************************************************************

Filename    :   Context.cpp
Content     :   Rendering context & tree implementation
Created     :   August 17, 2009
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_Context.h"
#include "Kernel/SF_Threads.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

/*
// Debug logic
#include <stdio.h>

enum { EntryHistorySize = 2000 };

unsigned RC_CaptureCount = 0;
unsigned RC_NextCaptureCount = 0;
unsigned RC_DisplayCaptureIndex = 0;
unsigned RC_CaptureMerge = 0;
unsigned RC_DisplayCaptureMerge = 0;

enum RecordEvent
{
    RE_Created,
    RE_Destroy_NewNode,
    RE_Destroy_WithChange,
    RE_Destroy_NoChange,
};

struct HistoryRecord
{
    Scaleform::Render::Context::Entry* pEntry;
    RecordEvent E;
    unsigned    CC, NC, DCI;
    

    void Set(Scaleform::Render::Context::Entry* e, RecordEvent re)
    {
        E = re;
        pEntry = e;
        CC = RC_CaptureCount;
        NC = RC_NextCaptureCount;
        DCI = RC_DisplayCaptureIndex;
    }

    static void AddRecord(Scaleform::Render::Context::Entry* e, RecordEvent re);
};

HistoryRecord     EntryHistory[EntryHistorySize];
Scaleform::UPInt  EntryHistoryPos = 0;

void HistoryRecord::AddRecord(Scaleform::Render::Context::Entry* e, RecordEvent re)
{
    if (EntryHistoryPos < EntryHistorySize)
    {
        EntryHistory[EntryHistoryPos].Set(e, re);
        EntryHistoryPos++;
    }
}
*/

namespace Scaleform { namespace Render { namespace ContextImpl {


//------------------------------------------------------------------------
// Snapshot represents a full rendering tree snapshot captured by the
// Context::Capture call. 

class Snapshot : public ListNode<Snapshot>,
                 public NewOverrideBase<StatRender_Context_Mem>
{
public:               
    Context*            pContext;

    // List of snapshot pages. These pages are released once
    // the snapshot it finished.
    List<SnapshotPage>  SnapshotPages;

    // Change list.
    ChangeBuffer        Changes;
    // Simple linked list of free change nodes in change buffer.
    // This is maintained to allow Create/Destroy of the same object within a frame
    // without growing change buffer.
    EntryChange*        pFreeChangeNodes;

    // Linked list changes in buffer that need propagation.
    // Note: This used to be a singly linked list with custom end-tag
    // value (Entry*)1; but exhaustive removal was too slow for some
    // Flash files (UNO.swf).
    List<Entry::PropagateNode> PropagateEntrys;

    // Destroyed entry list.
    List<Entry>         DestroyedNodes;

    
    struct HeapNode : public ListNode<HeapNode>,
                      public NewOverrideBase<StatRender_Context_Mem>
    {
        LinearHeap  ChangeHeap;
        HeapNode(MemoryHeap* pheap) : ChangeHeap(pheap) { }
    };

    // Typically a snapshot has only one heap, but we keep a list of heaps so that
    // we can merge snapshots. After a snapshot merge, both heaps can survive.    
    List<HeapNode>     Heaps;    
    

    Snapshot(Context* pcontext, MemoryHeap* pheap);
    ~Snapshot();

    EntryChange* AddChangeItem(Entry* pentry, unsigned changeBits);
    inline void  FreeChangeItem(EntryChange* pnode);

    LinearHeap& GetChangeHeap()
    {
        return Heaps.GetFirst()->ChangeHeap;
    }

    void        AddToPropagate(Entry* e)
    {
        PropagateEntrys.PushBack(&e->PNode);
    }
    void        RemoveFromPropagate(Entry* e)
    {
        SF_ASSERT(e->PNode.IsAdded());
        e->PNode.RemoveNode();
        e->PNode.Clear();
    }

    // Merges the older snapshot with a newer one. After merge,
    // the old snapshot can be deleted.
    void    Merge(Snapshot* pold);   
};


//------------------------------------------------------------------------
// ***** Context inline Implementations (Internal)

EntryPage* Entry::getEntryPage() const
{
    return (EntryPage*)( ((UPInt)this) & ~(UPInt)(EntryPageAllocAlign-1) );
}
Snapshot* Entry::getSnapshot() const
{
    return getEntryPage()->pSnapshot;
}
Context*  Entry::getContext() const
{
    return getSnapshot()->pContext;
}

Entry* Entry::PropagateNode::GetEntry()
{
    // Compute Entry address based on PNode offset.
    return (Entry*)(((UPInt)this) - (UPInt)&(((Entry*)0)->PNode));
}


Snapshot* EntryRef::GetSnapshot() const
{
    return pPage->pSnapshot;
}
SnapshotPage* EntryRef::GetSnapshotPage() const
{
    return pPage->pSnapshotPage;
}
SnapshotPage* EntryRef::GetDisplaySnapshotPage() const
{
    return pPage->pDisplaySnapshotPage;
}
void EntryRef::SetData_KeepDestroyedBit(EntryData* arg)
{
    pPage->pSnapshotPage->SetData_KeepDestroyedBit(Index, arg);
}
EntryData*& EntryRef::GetDataRef() const
{
    return pPage->pSnapshotPage->pData[Index];
}
bool EntryRef::IsDestroyed() const
{
    return pPage->pSnapshotPage->IsDestroyed(Index);
}



//------------------------------------------------------------------------
// ***** Entry
/*
void Context::Entry::Init(EntryData* pnative, EntryChange *pchange)
{
    RefCount = 1;
    pNative  = pnative;
    pRenderer= 0;
    pChange  = pchange;
    pParent  = 0;
    PNode.Clear();
    HistoryRecord::AddRecord(this, RE_Created);    
}
*/

void Entry::destroyHelper()
{
    EntryRef    ceref(this);
    Snapshot*   psnapshot = ceref.GetSnapshot();
    EntryData*  pdata     = ceref.GetData();
    EntryChange* pchange  = pChange;

    pdata->ReleaseNodes();    
    if (PNode.IsAdded())
        psnapshot->RemoveFromPropagate(this);

    if (pchange)
    {
        // If there was a change, we always destroy the item.
        //  - For NewNode, this will destroy pNative.
        //  - For other changes, this will destroy the newly created copy in the
        //    linear heap. DestroyedNodes list will be used later to destroy pNative.
        pdata->Destroy();
        
        if (pchange->IsNewNode())
        {
            if (hasRTHandle())
            {
                Lock::Locker scopeLock(psnapshot->pContext->getLock());
                psnapshot->pContext->clearRTHandle(this);
            }
            //HistoryRecord::AddRecord(this, RE_Destroy_NewNode);
            SF_ASSERT(pdata == GetNative());
            SF_FREE(pdata);
            psnapshot->pContext->Table.FreeEntry(this);
        }
        else
        {
            //HistoryRecord::AddRecord(this, RE_Destroy_WithChange);
            ceref.GetSnapshotPage()->MarkAsDestroyed(ceref.GetIndex());
            psnapshot->DestroyedNodes.PushBack(this);
        }

         psnapshot->FreeChangeItem(pchange);
    }
    else
    {
        //HistoryRecord::AddRecord(this, RE_Destroy_NoChange);
        ceref.GetSnapshotPage()->MarkAsDestroyed(ceref.GetIndex());
        psnapshot->DestroyedNodes.PushBack(this);
    }
}

void Entry::addToPropagateImpl()
{
    getSnapshot()->AddToPropagate(this);
}


EntryData* Entry::getWritableData(unsigned changeBits)
{
    EntryRef    ceref(this);
    EntryData*& pdata = ceref.GetDataRef();

    if (!pChange)
    {
        Snapshot*  psnapshot = ceref.GetSnapshot();
        // Allocate buffer space and copy-construct it; this also overwrites the
        // slot in a SnapshotPage since the pointer is used by reference.
        pdata   = pdata->ConstructCopy(psnapshot->GetChangeHeap());
        pChange = psnapshot->AddChangeItem(this, changeBits);
    }
    else
    {
        pChange->ChangeBits |= changeBits;
    }

    return pdata;
}

// ***** EntryData

EntryData::~EntryData()
{
}

// ***** EntryPage

void EntryPage::AddEntriesToList(List<Entry> *plist)
{   
    UPInt i;
    for (i = 0; i< PageEntryCount; i++)
        plist->PushBack(&Entries[i]);
}

void EntryPage::RemoveEntriesFromList(List<Entry> *plist)
{
    UPInt i;
    for (i = 0; i< PageEntryCount; i++)
        plist->Remove(&Entries[i]);
    SF_UNUSED(plist); // Quiet MSVC++.
}


//------------------------------------------------------------------------
// ***** Context::EntryTable


// Allocates a page and adds its items to free list.
bool EntryTable::AllocEntryPage()
{   
    EntryPage* ppage = (EntryPage*)
        SF_HEAP_MEMALIGN(pHeap, EntryPageAllocSize, EntryPageAllocAlign, StatRender_Context_Mem);
    if (!ppage)
        return false;

    // Must clear at least pRenderer pointer is the page; they
    // are scanned by ShutdownRendering().
    memset(ppage, 0, EntryPageAllocSize);

    // Every new page automatically gets a snapshot as well.
    ppage->pSnapshotPage = SnapshotPage::Alloc(pHeap, ppage);
    if (!ppage->pSnapshotPage)
    {
        SF_HEAP_FREE(pHeap, ppage);
        return false;
    }
    ppage->pDisplaySnapshotPage = 0;
    ppage->pSnapshot       = pActiveSnapshot;
    ppage->UseCount        = 0;
    ppage->AddEntriesToList(&FreeNodes);
    EntryPages.PushBack(ppage);
    return true;
}

void EntryTable::FreeEntryPage(EntryPage* ppage)
{
    // Lock for safety because RenderThread can also access pEntryPage pointer,
    // modified here by ClearEntryPagePointers for neighbor snapshot pages.
    // It may well be that this is not necessary because FreeEntryPage is called
    // outside of lock only when freeing new nodes; however, this ensures safety.
    Lock::Locker lock(pContext->getLock());

    ppage->RemoveNode(); // Remove from EntryPages.
    SF_ASSERT(ppage->UseCount == 0);
    ppage->RemoveEntriesFromList(&FreeNodes);

    SF_ASSERT(ppage->pSnapshotPage);
    // Clear pEntryPage pointers in neighboring snapshot pages to ensure that
    // they are not accessed by accident in destroySnaphot, nextCapture, etc.
    ppage->pSnapshotPage->ClearEntryPagePointers();
    // Remove node if we were already added to Snapshot (possible in ~Context).
    if (ppage->pSnapshotPage->pNext)
        ppage->pSnapshotPage->RemoveNode();
    ppage->pSnapshotPage->Free();
    SF_HEAP_FREE(pHeap, ppage);
}


// Allocate a node

Entry* EntryTable::AllocEntry(EntryData* pdata)
{
    // If no free space is available, allocate a new page.
    if (FreeNodes.IsEmpty() && !AllocEntryPage())
        return 0;

    Entry* p = FreeNodes.GetFirst();
    FreeNodes.Remove(p);

    EntryRef eref(p);
    eref.GetEntryPage()->UseCount++;
    eref.GetDataRef() = pdata;
    return p;
}

void EntryTable::FreeEntry(Context::Entry* p)
{ 
    FreeNodes.PushFront(p);
    EntryRef ceref(p);
    ceref.GetDataRef() = 0;

    // If all chunks in a memory block page are free,
    // release the page and all of those blocks.
    if (--ceref.GetEntryPage()->UseCount == 0)
        FreeEntryPage(ceref.GetEntryPage());
}


// 1. Get current snapshot list (return it to user for storage).
// 2. Copy the snapshot pages, setting the new set as active.
// 3. 

void EntryTable::GetActiveSnapshotPages(List<SnapshotPage>* plist)
{
    SF_ASSERT(plist->IsEmpty());
    EntryPage* ppage = EntryPages.GetFirst();
    while(!EntryPages.IsNull(ppage))
    {
        plist->PushBack(ppage->pSnapshotPage);
        ppage = (EntryPage*)ppage->pNext;
    }
}

void EntryTable::NextSnapshot(Snapshot* pnewSnapshot)
{
    // Make a full copy of each snapshot page.
    EntryPage* ppage = EntryPages.GetFirst();
    while(!EntryPages.IsNull(ppage))
    {
        ppage->pSnapshotPage =
            ppage->pSnapshotPage->Clone(pHeap, ppage->pSnapshotPage);
        ppage->pSnapshot = pnewSnapshot;
        ppage = (EntryPage*)ppage->pNext;
    }
    SetActiveSnapshot(pnewSnapshot);
}


//------------------------------------------------------------------------
// ***** SnapshotPage

SnapshotPage* SnapshotPage::Alloc(MemoryHeap* pheap, EntryPage* pentryPage)
{
    SnapshotPage* ppage = (SnapshotPage*)
        SF_HEAP_MEMALIGN(pheap, sizeof(SnapshotPage), 16, StatRender_Context_Mem);
    if (!ppage)
        return false;
#ifdef SF_BUILD_DEBUG
    memset(ppage, 0, sizeof(SnapshotPage));
#endif
    ppage->pNext      = ppage->pPrev = 0;
    ppage->pEntryPage = pentryPage;
    ppage->pOlderSnapshotPage = 0;
    ppage->pNewerSnapshotPage = 0;
    return ppage;
}

SnapshotPage* SnapshotPage::Clone(MemoryHeap* pheap, SnapshotPage* psource)
{
    SnapshotPage* ppage = (SnapshotPage*)
        SF_HEAP_MEMALIGN(pheap, sizeof(SnapshotPage), 16, StatRender_Context_Mem);
    if (!ppage)
        return 0;
    ppage->pNext      = ppage->pPrev = 0;
    ppage->pEntryPage = psource->pEntryPage;
    ppage->pOlderSnapshotPage = psource;
    ppage->pNewerSnapshotPage = 0;       
    memcpy(ppage->pData, psource->pData, sizeof(EntryData*) * PageEntryCount);

    // Old page gets a reference to the newer one.
    psource->pNewerSnapshotPage = ppage;
    return ppage;
}

void SnapshotPage::Free()
{
    
    // For safety...
    // This get used when context is deleten in Capture after Merge.
    if (pNewerSnapshotPage)
        pNewerSnapshotPage->pOlderSnapshotPage = pOlderSnapshotPage;
    if (pOlderSnapshotPage)
        pOlderSnapshotPage->pNewerSnapshotPage = pNewerSnapshotPage;
    SF_FREE(this);
}

void SnapshotPage::ClearEntryPagePointers()
{
    SnapshotPage* p = this;
    while(p->pNewerSnapshotPage)
        p = p->pNewerSnapshotPage;

    do {
        p->pEntryPage = 0;
        p = p->pOlderSnapshotPage;
    } while(p);
}


//------------------------------------------------------------------------
// ***** Snapshot implementation

Snapshot::Snapshot(Context* pcontext, MemoryHeap* pheap)
    : pContext(pcontext), pFreeChangeNodes(0)//,
    //pPropagateEntrys(Snapshot::PropagateEntryEnd())
{        
    HeapNode* pnode = SF_HEAP_NEW(pheap) HeapNode(pheap);
    Heaps.PushBack(pnode);
}

Snapshot::~Snapshot()
{
    while (!Heaps.IsEmpty())
    {
        HeapNode* pnode = Heaps.GetFirst();
        pnode->RemoveNode();
        delete pnode;
    }

    // Any snapshot pages released will live here.
    while (!SnapshotPages.IsEmpty())
    {
        SnapshotPage* pnode = SnapshotPages.GetFirst();
        pnode->RemoveNode();
        pnode->Free();
    }
}

EntryChange* Snapshot::AddChangeItem(Entry* pentry, unsigned changeBits)
{
    EntryChange* pnode;

    if (pFreeChangeNodes)
    {
        pnode            = pFreeChangeNodes;
        pFreeChangeNodes = pFreeChangeNodes->pNextFreeNode; 
    }
    else
    {
        pnode = Changes.AddItem();
        SF_ASSERT(pnode);
    }

    pnode->pNode      = pentry;    
    pnode->ChangeBits = changeBits;    
    return pnode;
}

void Snapshot::FreeChangeItem(EntryChange* pnode)
{
    pnode->pNode = 0;    
    pnode->pNextFreeNode = pFreeChangeNodes;
    pFreeChangeNodes = pnode;
}


// Merges the older snapshot with a newer one. After merge,
// the old snapshot can be deleted.
void Snapshot::Merge(Snapshot* pold)
{
    // 1. New snapshot table survives.
    // 2. Change buffers a merged by old changes added into the list,
    //    except for duplicates and destroyed nodes.
    // 3. Destroy lists are concatenated.
    // 4. Old snapshots buffer heap is kept alive as a part of us.
    //     - Memory efficiency of this can be optimized in the future
    //       by copying live items (at performance cost).


    // Merge Change buffers.
    ChangeBuffer::Page* poldPage = pold->Changes.GetFirstPage();
    while(poldPage)
    {
        unsigned iitem;
        for (iitem = 0; iitem < poldPage->GetSize(); iitem++)
        {
            EntryChange& change = poldPage->GetItem(iitem);
            if (!change.pNode)
                continue;
            
            // If the item has changed again in the new context,
            // mark it so that change item not copied. Also destroy the buffer slot.
            EntryRef      entryRef(change.pNode);
            SnapshotPage* poldSSPage = entryRef.GetSnapshotPage()->pOlderSnapshotPage;
            SF_ASSERT(poldSSPage != 0);
            EntryData*    oldEntryData = poldSSPage->pData[entryRef.GetIndex()];

            if (entryRef.GetData() != oldEntryData)
            {
                if (!change.IsNewNode())
                {
                    if (entryRef.GetCleanData() == oldEntryData)
                    {
                        // An entry was destroyed. Copy to native and update pointer, as we
                        // allow destroyed data to be accessed during change handling.
                        // Don't worry about new change list since it doesn't exist
                        // for destroyed nodes.
                        SF_ASSERT(entryRef.IsDestroyed());
                        //    printf("Merge Destroy: Node = %p, Data=%p\n",
                        //           change.pNode, entryRef.GetCleanData());

                        oldEntryData->CopyTo(change.pNode->GetNative());
                        entryRef.SetData_KeepDestroyedBit(change.pNode->GetNative());
                    }
                    // Destroy entry in the buffer and mark this change node at null.
                    oldEntryData->Destroy();
                }

                // Merge the change bits into the new change
                // IsDestroyed bit needs to be checked to ensure that DestroyedNodes 
                // entries internal pointers are not overwritten.
                // TBD: Need to make sure pChange pointer doesn't get wiped before Merge.
                if (change.pNode->GetChange() && !entryRef.IsDestroyed())
                {
                    change.pNode->GetChange()->ChangeBits |=
                        (change.ChangeBits & ~Change_Context_NewNode);
                }
            }
            else
            {
                AddChangeItem(change.pNode, change.ChangeBits);
            }
        }
        poldPage = poldPage->pNext;
    }


    // Merge destroy lists and heaps.
    DestroyedNodes.PushListToFront(pold->DestroyedNodes);
    Heaps.PushListToFront(pold->Heaps);
}





//------------------------------------------------------------------------
// ***** RTHandle

RTHandle::HandleData::HandleData(Entry* entry, Context* context)
: pContextLock(context->pCaptureLock), State(State_PreCapture), pEntry(entry)
{
}

RTHandle::HandleData::~HandleData()
{    
    Lock::Locker lock(&pContextLock->LockObject);

    if (pContextLock->pContext && pEntry)
    {
        pEntry->clearHasRTHandle();
        RemoveNode();
    }
}

RTHandle::RTHandle(Entry* entry)
{
    if (entry)
    {
        Context* context = entry->getContext();
        pData = *SF_NEW HandleData(entry, context);

        // Lock context and add to list.
        Lock::Locker lock(context->getLock());
        entry->setHasRTHandle();
        context->RTHandleList.PushBack(pData);
    }
}

RTHandle::~RTHandle()
{
}

bool RTHandle::NextCapture(RenderNotify* render)
{
    SF_AMP_SCOPE_RENDER_TIMER("RTHandle::NextCapture");
    if (!pData)
        return false;
    
    // TBD: This means that changes are applied during lock;
    //      this can be improved with re-factoring.
    Lock::Locker lock(&pData->pContextLock->LockObject);

    Context* context = pData->pContextLock->pContext;
    if (!context || (pData->State == State_Dead))
        return false;
    
   
    // Check if the object if accessible.
    // For example, if Capture want called after handle creation
    //  ... State_PreCapture
#ifdef SF_BUILD_DEBUG
    if (pData->State == State_PreCapture)
    {
        EntryRef entryRef(pData->pEntry);
        bool entryValid = false;

        if (context->pSnapshots[SS_Captured])
        {
            // This debug-only logic is expensive, but there is no other safe way to get to the
            // captured page so far, as it is not stored in EntryPage.
            List<SnapshotPage>& sspages = context->pSnapshots[SS_Captured]->SnapshotPages;
            SnapshotPage*       ppage = sspages.GetFirst();
            while(!sspages.IsNull(ppage))
            {
                if (ppage->pEntryPage == entryRef.GetEntryPage())
                {
                    if (ppage->GetCleanData(entryRef.GetIndex()))
                        entryValid = true;
                    break;
                }
                ppage = sspages.GetNext(ppage);
            }
        }
        else
        {
            SnapshotPage* page = entryRef.GetDisplaySnapshotPage();
            if (page && page->GetCleanData(entryRef.GetIndex()))
                entryValid = true;
        }
        SF_DEBUG_WARNING(!entryValid,
            "RTHandle::NextCapture error - called before Capture");
        SF_ASSERT(entryValid);
    }
#endif

    if (context->NextCapture(render, Context::Capture_OnceAFrame))
    {
        if (pData->State == State_PreCapture)
            pData->State = State_Valid;       
        return (pData->State == State_Valid);
    }
    return false;
}


Entry* RTHandle::GetRenderEntry() const
{
    return (pData && pData->State == State_Valid) ? pData->pEntry : 0;
}


//------------------------------------------------------------------------
// ***** RenderNotify implementation

RenderNotify::~RenderNotify()
{
    ReleaseAllContextData();
}

void RenderNotify::NewCapture(Context* context, bool hasChanges)
{
    SF_UNUSED(hasChanges);

    // When called, we cold Context::ScopeLock
    if (context->pRenderer != this)
    {
        context->pRenderer = this;
        ActiveContextSet.PushBack(&context->RenderNode);
    }
}

void RenderNotify::ContextReleased(Context* context)
{
    SF_ASSERT(context->pRenderer == this);
    context->RenderNode.RemoveNode();
    context->pRenderer = 0;
    context->NextCaptureCalledInFrame = false;
}

void RenderNotify::ServiceQueues()
{
    // Handle any context shut-down request.
    ContextNode* n = ActiveContextSet.GetFirst();

    while (!ActiveContextSet.IsNull(n))
    {
        ContextNode* p = n;
        n = n->pNext;
        if (p->pContext->needRenderShutdown())
            p->pContext->ShutdownRendering(this);
    }
}

void RenderNotify::ReleaseAllContextData()
{
    while(!ActiveContextSet.IsEmpty())
    {
        ActiveContextSet.GetFirst()->pContext->ShutdownRendering(this);
    }
}

void RenderNotify::EndFrameContextNotify()
{
    ContextNode* n = ActiveContextSet.GetFirst();
    while (!ActiveContextSet.IsNull(n))
    {
        n->pContext->endFrame();
        n = n->pNext;
    }
}

//------------------------------------------------------------------------
// ***** Context implementations

Context::Context(MemoryHeap* pheap)
    : pHeap(pheap), Table(getThis(), pheap), pRenderer(0),
      NextCaptureCalledInFrame(false),
      ShutdownFlag(false), pShutdownEvent(0), RenderNode(getThis())
{
    pCaptureLock = *SF_NEW ContextLock(this);

    CreateThreadId = GetCurrentThreadId();
    MultiThreadedUse = false;

    pSnapshots[SS_Active]    = SF_HEAP_NEW(pHeap) Snapshot(this, pHeap);
    pSnapshots[SS_Captured]  = 
    pSnapshots[SS_Displaying]= 
    pSnapshots[SS_Finalizing]= 0;

    FinalizedFrameId = 0;
    SnapshotFrameIds[SS_Active] = 1;
    SnapshotFrameIds[SS_Captured]   = 0;
    SnapshotFrameIds[SS_Displaying] = 0; 
    SnapshotFrameIds[SS_Finalizing] = 0;
    
    Table.SetActiveSnapshot(pSnapshots[SS_Active]);

    CaptureCalled = false;
}

Context::~Context()
{
    // Force the Display thread to release its data/shutdown access to us.
    // Note that we use blocking wait.
    Shutdown(true);

    destroySnapshot(pSnapshots[SS_Finalizing]);
    destroySnapshot(pSnapshots[SS_Displaying]);
    destroySnapshot(pSnapshots[SS_Captured]);

     if (pSnapshots[SS_Active])
     {
         // SnapshotPages must be properly initialized for destroySnapshot to work.
         Table.GetActiveSnapshotPages(&pSnapshots[SS_Active]->SnapshotPages);
         destroySnapshot(pSnapshots[SS_Active]);
     }
}

//  
Entry* Context::createEntryHelper(EntryData* pdata)
{    
    if (!pdata) return 0;

    Entry* pentry = Table.AllocEntry(pdata);
    if (!pentry)
    {
        pdata->Destroy();
        SF_FREE(pdata);
        return 0;
    }
    
    EntryChange* pchange = 
        pSnapshots[SS_Active]->AddChangeItem(pentry, (unsigned)Change_Context_NewNode);
    pentry->Init(pdata, pchange);
    pentry->AddToPropagate();

    return pentry;
}


unsigned ConstructCopyCalls = 0, DestroyCalls = 0, CopyCalls = 0;

// Destroys a snapshot from destructor.
void Context::destroySnapshot(Snapshot* p)
{
    if (!p)
        return;

    // Assign pTempPage to this snapshot's pages so we can use them for indexing.
    // TBD: This could probably be just pDisplaySnaphotPage but... pTempPage is safe.
    SnapshotPage* ppage = p->SnapshotPages.GetFirst();
    while (!p->SnapshotPages.IsNull(ppage))
    {
        if (ppage->pEntryPage)
            ppage->pEntryPage->pTempPage = ppage;
        ppage = ppage->pNext;
    }

    // Call Destroy() on all of the change items of this snapshot.
    ChangeBuffer::Page* pcbPage = p->Changes.GetFirstPage();
    while(pcbPage)
    {
        unsigned iitem;
        for (iitem = 0; iitem < pcbPage->GetSize(); iitem++)
        {
            EntryChange& change = pcbPage->GetItem(iitem);
            if (!change.pNode || change.IsNewNode())
                continue;
            // Get current snapshot's page.
            EntryRef      entryRef(change.pNode);
            SnapshotPage* ppage = entryRef.GetEntryPage()->pTempPage;
            SF_ASSERT(ppage->pData[entryRef.GetIndex()] != change.pNode->GetNative());
            ppage->pData[entryRef.GetIndex()]->Destroy();

        }
        pcbPage = pcbPage->pNext;
    }

    // Go through deleted nodes and free their native memory.
    destroyNativeNodes(p->DestroyedNodes);
    delete p;
}

void Context::destroyNativeNodes(List<Entry>& destroyList)
{
    // Go through deleted nodes and free their native memory.
    while(!destroyList.IsEmpty())
    {
        Entry* pdestroyNode = destroyList.GetFirst();
        List<Entry>::Remove(pdestroyNode);
        // Since Destroy() was not called on pNative, do it here.
        pdestroyNode->GetNative()->Destroy();
        SF_FREE(pdestroyNode->GetNative());
        pdestroyNode->ClearNative();
        Table.FreeEntry(pdestroyNode);
    }
}

// Clears out RTHandle associated with an entry, causing the handle to be "null".
// Expects lock to be held.
void Context::clearRTHandle(Entry* entry)
{
    SF_ASSERT(entry && entry->hasRTHandle());

    RTHandle::HandleData* data = RTHandleList.GetFirst();
    while(!RTHandleList.IsNull(data))
    {
        if (data->pEntry == entry)
        {
            data->RemoveNode();
            data->State = RTHandle::State_Dead;
            data->pEntry = 0;
            entry->clearHasRTHandle();
            return;
        }
        data = data->pNext;
    }    
    SF_ASSERT(0);
}

void Context::clearRTHandleList()
{    
    while(!RTHandleList.IsEmpty())
    {
        RTHandle::HandleData* data = RTHandleList.GetFirst();
        data->RemoveNode();
        data->State = RTHandle::State_Dead;
        if (data->pEntry)
            data->pEntry->clearHasRTHandle();
        data->pEntry = 0;
    }
}


bool Context::HasChanges() const
{
    return !pSnapshots[SS_Active]->Changes.IsEmpty() ||
           !pSnapshots[SS_Active]->DestroyedNodes.IsEmpty();
}

// Captures the state of the render nodes so that further
// modifications don't affect the displayed graph.
bool Context::Capture()
{
    PropagateChangesUp();

    if (ShutdownFlag)
    {
        SF_DEBUG_WARNING(1, "Render::Context::Capture failed - not allowed after Shutdown");
        return false;
    }

    Lock::Locker scopeLock(getLock());

    // Take all the finalizing nodes and process them. This finalizing nodes may change
    // pointer values in the active snapshot table, this must be done first.
    handleFinalizingSnaphot();
    
    // We fill in the pages once they are done.
    Snapshot* pactiveSnapshot = pSnapshots[SS_Active];

    Table.GetActiveSnapshotPages(&pactiveSnapshot->SnapshotPages);
  
    if (pSnapshots[SS_Captured])
    {
//        RC_CaptureMerge++;
        pactiveSnapshot->Merge(pSnapshots[SS_Captured]);
        delete pSnapshots[SS_Captured];
    }
    else
    {
//        RC_CaptureMerge = 0;
    }
    pSnapshots[SS_Captured] = pactiveSnapshot;
    SnapshotFrameIds[SS_Captured] = SnapshotFrameIds[SS_Active];

    // Clear all of the active pChange pointers, so that new changes can be tracked.
    ChangeBuffer::Page* pcbPage = pactiveSnapshot->Changes.GetFirstPage();
    while(pcbPage)
    {
        unsigned iitem;
        for (iitem = 0; iitem < pcbPage->GetSize(); iitem++)
        {
            EntryChange& change = pcbPage->GetItem(iitem);
            if (change.pNode)
                change.pNode->SetChange(0);
        }
        pcbPage = pcbPage->pNext;
    }
    
    // Change the active snapshot to the new one.
    Snapshot* pnextSnapshot = SF_HEAP_NEW(pHeap) Snapshot(this, pHeap);
    Table.NextSnapshot(pnextSnapshot);
    pSnapshots[SS_Active] = pnextSnapshot;
    SnapshotFrameIds[SS_Active]++;

    CaptureCalled = true;
    // RC_CaptureCount++;

    return true;
}

void Context::handleFinalizingSnaphot()
{
    if (pSnapshots[SS_Finalizing])
    {
        Snapshot* p = pSnapshots[SS_Finalizing];

        // Call Destroy() on all of the finalized change items; also change
        // their pointers to pNative if applicable.
        ChangeBuffer::Page* pcbPage = pSnapshots[SS_Finalizing]->Changes.GetFirstPage();
        while(pcbPage)
        {
            unsigned iitem;
            for (iitem = 0; iitem < pcbPage->GetSize(); iitem++)
            {
                EntryChange& change = pcbPage->GetItem(iitem);
                if (!change.pNode || change.IsNewNode())
                    continue;

                Entry*      pentry = change.pNode;
                EntryRef    entryRef(pentry);
                unsigned    index = entryRef.GetIndex();

                SnapshotPage* pfinalizingPage = entryRef.GetDisplaySnapshotPage()->pOlderSnapshotPage;
                SF_ASSERT(!pfinalizingPage->IsDestroyed(index));
                if (pfinalizingPage->pData[index] == entryRef.GetCleanData())
                {
                    // If active pointer didn't change since Finalize, the newest NodeData value
                    // has already been copied to pNative. Reassign the active page to do the same.
                    entryRef.SetData_KeepDestroyedBit(pentry->GetNative());

                    // If we are here, change must be 0 for non-destroyed nodes.
                    SF_ASSERT(entryRef.IsDestroyed() || (pentry->GetChange() == 0));
                    SF_ASSERT(entryRef.GetDisplaySnapshotPage()->GetCleanData(index) == pentry->GetNative());
                }

                // Destroy the change item before its buffer heap is freed.
                pfinalizingPage->pData[index]->Destroy();
            }
            pcbPage = pcbPage->pNext;
        }

        // Go through deleted nodes and free their native memory.
        destroyNativeNodes(p->DestroyedNodes);
        delete p;
        pSnapshots[SS_Finalizing] = 0;
        FinalizedFrameId = SnapshotFrameIds[SS_Finalizing];
    }
}


void Context::Shutdown(bool waitFlag)
{
    bool needWait;

    while(1)
    {
       needWait = false;
       { // Lock scope.
            Lock::Locker scopeLock(getLock());
            handleFinalizingSnaphot();
            
            ShutdownFlag = true;
            if (pRenderer && waitFlag)
            {   
                if (!MultiThreadedUse)
                    shutdownRendering_NoLock();
                else
                    needWait = true;
            }
            
            if (!needWait)
            {
                // Always remove RTHandles during lock; we do it
                // after wait finished.
                clearRTHandleList();
                // Clear pContext, causing future Handle.NextContext() to fail.
                pCaptureLock->pContext = 0;
            }
        }

        if (!needWait)
            break;
        
        SF_ASSERT(MultiThreadedUse);
        Event waitEvent;
        {  // Secondary lock scope (used to avoid always creating Event).
            Lock::Locker scopeLock(getLock());
            if (pRenderer)
                pShutdownEvent = &waitEvent;
            else
                needWait = false;
        }
        if (needWait)
            waitEvent.Wait();        
        
        // Note that is it important to loop around and re-lock CaptureLock,
        // as that would allow render thread NextCapture to exist safely.
    } // loop to re-try.
}


// Part of NextCapture that requires a lock. Sets updateSnapshot to non-zero
// if HandleChanges and destroy processing is necessary.
bool Context::nextCapture_LockScope(Snapshot** updateSnapshot,
                                    RenderNotify* pnotify, CaptureMode mode)
{
    Lock::Locker scopeLock(getLock());

    SF_ASSERT(!pRenderer || (pRenderer == pnotify));

    // Check Capture_OnceAFrame before shutdown, since if capture succeeded
    // a handle it should remain alive + accessible until EndFrame.
    if ((mode == Capture_OnceAFrame) && NextCaptureCalledInFrame)
        return true;

    if (ShutdownFlag)
    {
        clearRTHandleList();
        shutdownRendering_NoLock();
        return false;
    }

    if (CreateThreadId != GetCurrentThreadId())
        MultiThreadedUse = true;

    if (pnotify && (mode == Capture_OnceAFrame))
        NextCaptureCalledInFrame = true;
            
    if (!pSnapshots[SS_Captured])
    {
        // If no new captures were updated, use the same one.
        if (pnotify)
            pnotify->NewCapture(this, false);
        return true;
    }

    SF_ASSERT(pSnapshots[SS_Finalizing] == 0);

    // If there are items that haven't changed in the next frame, write
    // them back to pNative.
    Snapshot* pd = pSnapshots[SS_Displaying];
    if (pd)
    {
        ChangeBuffer::Page* pcbPage = pd->Changes.GetFirstPage();
        while(pcbPage)
        {
            unsigned iitem;
            for (iitem = 0; iitem < pcbPage->GetSize(); iitem++)
            {
                EntryChange&    change = pcbPage->GetItem(iitem);
                Entry*          pentry = change.pNode;
                if (!pentry || (change.IsNewNode()))
                    continue;
                EntryRef        entryRef(pentry);
                unsigned        index = entryRef.GetIndex();

                // Get captured node at same index. See if pointer
                // value in the slot is the same. If it is, not change - 
                // so we can write the data out to pNative.
                SnapshotPage* pnewSSPage = entryRef.GetDisplaySnapshotPage()->pNewerSnapshotPage;

                // Item hasn't changed, so copy it to pNative.
                // Note that we do this even for destroyed nodes (hence GetCleanData); this is
                // needed to ensure that destroying frame sees most recent data, as it may
                // still be accessed during change processing (before destroy list processing).
                EntryData* pdata = pnewSSPage->GetCleanData(index);
                if (entryRef.GetDisplaySnapshotPage()->pData[index] == pdata)
                {
                    SF_ASSERT(pdata != pentry->GetNative());
                    pdata->CopyTo(pentry->GetNative());
                    // Note that we can't call destructor yet, as pActive page may point to
                    // the data. This will be done when Finalizing snapshot is processed
                    // by the NextCapture call.

                    // Change newer pointer to pNative, but leave the finalizing one
                    // so that we can detect the change.
                    pnewSSPage->SetData_KeepDestroyedBit(index, pentry->GetNative());
                }
                else
                {
                    // Change has been overwritten by a secondary one. Secondary
                    // one will be in its own buffer, so do nothing.
                }
            }
            pcbPage = pcbPage->pNext;
        }
    }
        

    // Change 'pDisplaySnapshotPage'.
    List<SnapshotPage>& sspages = pSnapshots[SS_Captured]->SnapshotPages;
    SnapshotPage*         ppage = sspages.GetFirst();
    while(!sspages.IsNull(ppage))
    {
        if (ppage->pEntryPage)
            ppage->pEntryPage->pDisplaySnapshotPage = ppage;
        ppage = sspages.GetNext(ppage);
    }

    // Move new captured items to Display.
    pSnapshots[SS_Finalizing] = pd;
    pSnapshots[SS_Displaying] = pSnapshots[SS_Captured];
    pSnapshots[SS_Captured]   = 0;

    SnapshotFrameIds[SS_Finalizing] = SnapshotFrameIds[SS_Displaying];
    SnapshotFrameIds[SS_Displaying] = SnapshotFrameIds[SS_Captured];

    if (pnotify)
    {
        *updateSnapshot = pSnapshots[SS_Displaying];
        pnotify->NewCapture(this, true);
    }

//    RC_NextCaptureCount++;
//    RC_DisplayCaptureIndex = RC_CaptureCount;
//    RC_DisplayCaptureMerge = RC_CaptureMerge;
    return true;
}


// Captures the items from Display.
bool Context::NextCapture(RenderNotify *pnotify, CaptureMode mode)
{
    SF_AMP_SCOPE_RENDER_TIMER("Context::NextCapture");
    Snapshot* displaySnaphot = 0;
    bool      result = nextCapture_LockScope(&displaySnaphot, pnotify, mode);
    
    if (displaySnaphot)
    {
        // Notify renderer about changes that took place.
        // Entry notification is NOT done during a lock because
        // it doesn't need it and is typically time consuming.
        pnotify->EntryChanges(displaySnaphot->Changes);

        Entry* pentry = displaySnaphot->DestroyedNodes.GetFirst();
        if (!displaySnaphot->DestroyedNodes.IsNull(pentry))
        {
            do {
                pnotify->EntryDestroy(pentry);
                pentry->SetRenderDataAsBAD();
                pentry = List<Entry>::GetNext(pentry);
            } while(!displaySnaphot->DestroyedNodes.IsNull(pentry));

            // clearRTHandle() must be called during a lock.
            pentry = displaySnaphot->DestroyedNodes.GetFirst();
            Lock::Locker scopeLock(getLock());
            do {
                if (pentry->hasRTHandle())
                    clearRTHandle(pentry);
                pentry = List<Entry>::GetNext(pentry);
            } while(!displaySnaphot->DestroyedNodes.IsNull(pentry));
        }
    }

    return result;
}


void Context::ShutdownRendering(RenderNotify* pnotify)
{
    Lock::Locker scopeLock(getLock());

    if (CreateThreadId != GetCurrentThreadId())
        MultiThreadedUse = true;

    // ShutdownRendering can be called if there is no renderer,
    // or after it is being serviced.
    SF_ASSERT(!pRenderer || (pnotify == pRenderer));
    SF_UNUSED(pnotify);

    shutdownRendering_NoLock();
}

void  Context::shutdownRendering_NoLock()
{
    if (pRenderer)
    {
        // Traverse through all of the EntryTable nodes to see 
        // if any of them have pRenderer pointers, so that
        // we can clear those out.

        if (!pSnapshots[SS_Displaying])
            return;

        // Due to threading, traverse this snapshot's linked list
        // instead of the main one which belongs to Advance thread only.
        Snapshot*       pd    = pSnapshots[SS_Displaying];
        SnapshotPage*   ppage = pd->SnapshotPages.GetFirst();
        while (!pd->SnapshotPages.IsNull(ppage))
        {
            EntryPage* pentryPage = ppage->pEntryPage;
            if (pentryPage)
            {
                for (unsigned i=0; i< PageEntryCount; i++)
                {
                    Entry* pentry = &pentryPage->Entries[i];
                    if (pentry->GetRenderDataClean())
                    {
                        pRenderer->EntryFlush(pentry);
                        // Note: EntryDestroy will still be called later.
                    }
                }
            }
            ppage = ppage->pNext;
        }

        if (pShutdownEvent)
        {
            pShutdownEvent->SetEvent();
            pShutdownEvent = 0;
        }

        pRenderer->ContextReleased(this);
        //pRenderer = 0;
    }
    else
    {
        SF_ASSERT(!pShutdownEvent);
    }    
}


// Propagates changed entries up the parent chains.
// By default called automatically from within Capture.
void Context::PropagateChangesUp()
{
    Snapshot* s = pSnapshots[SS_Active];

    // Collected marked nodes into depth
    DepthUpdateArrayPOD<Entry::PropagateNode*> DepthUpdates(pHeap, 0);

    // Go through changes and assign them into depth array slots
    Entry::PropagateNode* entryNode = s->PropagateEntrys.GetFirst();
    while(!s->PropagateEntrys.IsNull(entryNode))
    {
        Entry::PropagateNode* next = entryNode->pNext;
        // Insert into DepthUpdates
        unsigned depth = entryNode->GetEntry()->CalcDepth();
        DepthUpdates.Link(depth, &entryNode->pNext, entryNode);
        // Leave pPrev with value to allow AddToPropagate for the same
        // object during PropagateUp.
        entryNode->pPrev = (Entry::PropagateNode*)1;
        entryNode = next;
    }
    // Clear list.
    s->PropagateEntrys.Clear();

    // From outer depth to inner, update parents based on bits
    Entry*   entry;
    unsigned maxUsedDepth = DepthUpdates.GetDepthUsed()-1;
    for (unsigned i = maxUsedDepth; (i+1) > 0; i--)
    {
        // Process all units of depth as needed.
        entryNode = DepthUpdates.Get(i);
        while(entryNode != 0)
        {
            entry = entryNode->GetEntry();
            if (entry->GetReadOnlyData()->PropagateUp(entry))
            {
                // If parent update is needed, add it to list.
                Entry *p = entry->pParent;
                if (p && !p->PNode.pNext)
                {                    
                    SF_ASSERT(p->CalcDepth() == (i-1));
                    DepthUpdates.Link(i-1, &p->PNode.pNext, &p->PNode);
                }
            }
            entryNode = entryNode->pNext;
            entry->PNode.Clear();
        }
    }

    SF_ASSERT(s->PropagateEntrys.IsEmpty());
}


}}} // Scaleform::Render::ContextImpl
