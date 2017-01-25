/**********************************************************************

Filename    :   AS3_GC.h
Content     :   
Created     :   Jan, 2010
Authors     :   Artem Bolgar, Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_GC_H
#define INC_AS3_GC_H


#include "AS3_Value.h" // Because of ValueArray
#include "Kernel/SF_ArrayPaged.h"


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
#ifdef GFC_GC_DO_TRACE
    #define GFC_GC_DEBUG_PARAMS         int level
    #define GFC_GC_DEBUG_PARAMS_DEF     int level=0
    #define GFC_GC_DEBUG_PASS_PARAMS    level+1
    #define GFC_GC_TRACE(s)             Trace(level, s)
    #define GFC_GC_COND_TRACE(c,s)      do { if(c) Trace(level, s); } while(0)
    #define GFC_GC_PRINT(s)             printf("%s\n", s)
    #define GFC_GC_PRINTF(s,p1)         printf(s, p1)
    #define GFC_GC_PARAM_START_VAL      0
#else
    #define GFC_GC_DEBUG_PARAMS
    #define GFC_GC_DEBUG_PARAMS_DEF
    #define GFC_GC_DEBUG_PASS_PARAMS
    #define GFC_GC_TRACE(s)
    #define GFC_GC_COND_TRACE(c,s)
    #define GFC_GC_PRINT(s)
    #define GFC_GC_PRINTF(s,p1)
    #define GFC_GC_PARAM_START_VAL
#endif

template <int Stat>
class RefCountCollector;

template <typename T> 
class APtr;

template <int Stat = Stat_Default_Mem>
class RefCountBaseGC : public NewOverrideBase<Stat>
{
    friend class RefCountCollector<Stat>;
    friend class Value;
    template <typename T> friend class Pickable;
    template <typename T> friend class SPtr;
    template <typename T> friend class APtr;

public:
    enum OperationGC
    {
        Operation_Release,
        Operation_MarkInCycle,
        Operation_ScanInUse,
        Operation_CollectGarbage,
        Operation_Scan,
        Operation_Disable
    };

    typedef void (*GcOp)(const RefCountBaseGC<Stat>** pchild);
    
private:
    enum
    {
        Flag_Buffered = 0x80000000u,
        Mask_State    = 0x7,
        Shift_State   = 28,
        Mask_RefCount = 0xFFFFFFF
    };
    enum States
    {
        State_InUse     = 0,
        State_InCycle   = 1,
        State_Garbage   = 2,
        State_Root      = 3,
        State_Removed   = 4
    };
    union
    {
        RefCountCollector<Stat>* pRCC;
        const RefCountBaseGC<Stat>* pNextToKill;
    };
    // Ref counter takes bits 27-0 lower bytes.
    // The most significant bit (bit 31) is flag "buffered".
    // Bits 30-28 - color (7 values)
    mutable UInt32      RefCount;
    mutable UInt32      RootIndex;

    void Increment(GFC_GC_DEBUG_PARAMS) const
    {
        SF_ASSERT((RefCount & Mask_RefCount) < Mask_RefCount);
        ++RefCount;
        GFC_GC_TRACE("Incremented ");
    }
    unsigned Decrement(GFC_GC_DEBUG_PARAMS) const
    {
        SF_ASSERT((RefCount & Mask_RefCount) > 0);
        GFC_GC_TRACE("Decrementing ");
        return ((--RefCount) & Mask_RefCount);
    }

    void MarkInCycle(GFC_GC_DEBUG_PARAMS) const;
    void ScanInUse(GFC_GC_DEBUG_PARAMS) const;
    bool CollectGarbage(GFC_GC_DEBUG_PARAMS) const;
    void Scan(GFC_GC_DEBUG_PARAMS) const;

    struct ReleaseFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            (*pchild)->Release();
        }
    };

    struct MarkInCycleFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            (*pchild)->Decrement();
            (*pchild)->MarkInCycle();
        }
    };
    struct ScanInUseFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            (*pchild)->Increment();
            if (!(*pchild)->IsInState(State_InUse))
                (*pchild)->ScanInUse();
        }
    };

    struct CollectGarbageFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            (*pchild)->CollectGarbage();
        }
    };

    struct ScanFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            (*pchild)->Scan();
        }
    };

    struct DisableFunctor
    {
        static void Call(const RefCountBaseGC<Stat>** pchild)
        {
            SF_ASSERT(pchild);
            SF_ASSERT(*pchild);
            *pchild = (RefCountBaseGC<Stat>*)((UPInt)(*pchild) + 1);
        }
    };
    
    virtual void ForEachChild_GC(GcOp op) const = 0;
    
    void ExecuteForEachChild_GC(OperationGC op) const
    {
        switch(op)
        {
        case Operation_Release: 
            ForEachChild_GC(ReleaseFunctor::Call); 
            break;
        case Operation_MarkInCycle: 
            ForEachChild_GC(MarkInCycleFunctor::Call); 
            break;
        case Operation_ScanInUse: 
            ForEachChild_GC(ScanInUseFunctor::Call); 
            break;
        case Operation_CollectGarbage: 
            ForEachChild_GC(CollectGarbageFunctor::Call); 
            break;
        case Operation_Scan: 
            ForEachChild_GC(ScanFunctor::Call); 
            break;
        case Operation_Disable: 
            ForEachChild_GC(DisableFunctor::Call); 
            break;
        }
    }

    bool IsRefCntZero() const { return (RefCount & Mask_RefCount) == 0; }

    void SetBuffered(UInt32 rootIndex) const
    { 
        RefCount |= Flag_Buffered; RootIndex = rootIndex; 
    }
    void ClearBuffered() const   { RefCount &= ~Flag_Buffered; RootIndex = ~0u; }
    bool IsBuffered() const { return (RefCount & Flag_Buffered) != 0; }

    void SetState(States c) const
    { 
        RefCount = (RefCount & ~(Mask_State << Shift_State)) | 
                   (((UInt32)c & Mask_State) << Shift_State); 
    }
    States GetState() const  { return (States)((RefCount >> Shift_State) & Mask_State); }
    bool IsInState(States c) const { return GetState() == c; }

    void Free_GC(GFC_GC_DEBUG_PARAMS) const
    {
        GFC_GC_TRACE("Freeing ");
        ExecuteForEachChild_GC(Operation_Disable);
        delete this;
    }

public:
    RefCountBaseGC(class RefCountCollector<Stat>* prcc) : pRCC(prcc), RefCount(1) { SF_ASSERT(pRCC); }

    RefCountCollector<Stat>* GetCollector() const { return pRCC; }

    // Dangerous AddRef/Release that should be used with care.
    // They recieve separate names so that they don't accidentally assigned to Ptr<>
    void    AddRef_Unsafe() const { AddRef(); }
    void    Release_Unsafe() const { Release(); }

protected:
    virtual ~RefCountBaseGC() // virtual to avoid gcc -Wall warnings
    {
    }

private:
    void AddRef() const 
    { 
        SF_ASSERT(pRCC);
        Increment(GFC_GC_PARAM_START_VAL);
        SetState(State_InUse);
    }
    void Release(GFC_GC_DEBUG_PARAMS_DEF) const;

    // copying is prohibited
    RefCountBaseGC(const RefCountBaseGC&) { }
    RefCountBaseGC& operator=(const RefCountBaseGC&) { return *this; }

#ifdef GFC_GC_DO_TRACE
    void Trace(int level, const char* text)
    {
        char s[1024] = {0};
        for (int i = 0; i < level; ++i)
            G_strcat(s, sizeof(s), "  ");
        printf ("%s%s %p, state %d, refcnt = %d\n", s, text, this, GetState(), 
            (RefCount & RefCountBaseGC<Stat>::Mask_RefCount));
}
#endif // GFC_GC_DO_TRACE
};

template <int Stat = Stat_Default_Mem>
class RefCountCollector : public RefCountBase<RefCountCollector<Stat>, Stat>
{
    friend class RefCountBaseGC<Stat>;

    ArrayPagedLH_POD<const RefCountBaseGC<Stat> *, 10, 5> Roots;
    UPInt                        FirstFreeRootIndex;
    const RefCountBaseGC<Stat>* pFirstToKill;

    enum
    {
        Flags_Collecting,
        Flags_AddingRoot
    };
    UInt8 Flags;

    void AddToKillList(const RefCountBaseGC<Stat>* proot);
public:
    struct Stats
    {
        unsigned RootsNumber;
        unsigned RootsFreedTotal;

        Stats() { RootsNumber = RootsFreedTotal = 0; }
    };
public:
    RefCountCollector():FirstFreeRootIndex(SF_MAX_UPINT), pFirstToKill(NULL), Flags(0) {}
    ~RefCountCollector() { Collect(); }

    // Perform collection. Returns 'true' if collection process was 
    // executed.
    bool Collect(Stats* pstat = NULL);

    // Returns number of roots; might be used to determine necessity
    // of call to Collect.
    UPInt GetRootsCount() const { return Roots.GetSize(); }

    // Shrinks the array for roots to a minimal possible size
    void ShrinkRoots();

    bool IsCollecting() const { return (Flags & Flags_Collecting) != 0; }
    bool IsAddingRoot() const { return (Flags & Flags_AddingRoot) != 0; }
    
private:
    RefCountCollector(const RefCountCollector&) {}
    RefCountCollector& operator=(const RefCountCollector&) { return *this; }
};

/////////////////////////////////////////////////////
// Implementations
//
template <int Stat>
inline
void RefCountBaseGC<Stat>::Release(GFC_GC_DEBUG_PARAMS) const
{
    if (IsRefCntZero())
        return;
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());

    if (Decrement(GFC_GC_DEBUG_PASS_PARAMS) == 0)
    {
        GFC_GC_TRACE("Releasing ");

        // Release all children
        ExecuteForEachChild_GC(Operation_Release);

        SetState(State_InUse);

        if (!IsBuffered())
        {
            Free_GC(GFC_GC_DEBUG_PASS_PARAMS);
        }
        else
        {
            GFC_GC_TRACE("bufferred, find it roots and free.");

            // We are going to free a buffered root. We already have stored the index of 
            // element in Roots array, lets make sure it is valid by the following ASSERT...
            SF_ASSERT(pRCC->Roots[RootIndex] == this);
            
            if (RootIndex + 1 != pRCC->Roots.GetSize())
            {
                // Now, need to make kind of "free-list". pRCC stores the index of the first
                // free root (not necessary to be physically first). This index points to
                // the element in Roots which is free. This element contains an index of the next 
                // free root, encoded by the following way: bits 31-1 contain the index, bit 0 
                // is set to 1 as an indicator that this is an index and not a pointer.
                union
                {
                    RefCountBaseGC<Stat>*  Ptr;
                    UPInt                   PtrValue;
                } u;

                // lowest bit (bit 0) set to 1 means this is an index rather than a real ptr.
                // Since all ptrs suppose to be aligned on boundary of 4, it is safe to
                // do so.
                u.PtrValue = (pRCC->FirstFreeRootIndex << 1) | 1;

                pRCC->Roots[RootIndex]   = u.Ptr;
                pRCC->FirstFreeRootIndex = RootIndex;
            }
            else
            {
                // no need in adding root to free list since this is the last one, just
                // resize.
                pRCC->Roots.Resize(RootIndex); 
            }

            Free_GC(GFC_GC_DEBUG_PASS_PARAMS);
        }
    }
    else
    {
        SF_ASSERT(!pRCC->IsCollecting());
        SF_ASSERT(!IsInState(State_Removed));
        // Possible root
        if (!IsInState(State_Root))
        {
            // Add to the array of roots, if not buffered yet
            GFC_GC_COND_TRACE(!IsBuffered(), "Adding possible root");
            SetState(State_Root);
            if (!IsBuffered())
            {
                // check if we have free roots; if so - reuse first of it.
                if (pRCC->FirstFreeRootIndex != SF_MAX_UPINT)
                {
                    SetBuffered((UInt32)pRCC->FirstFreeRootIndex);

                    // update the FirstFreeRootIndex
                    union
                    {
                        const RefCountBaseGC<Stat>*    Ptr;
                        UPInt                           UPtrValue;
                        SPInt                           SPtrValue;
                    } u;
                    u.Ptr = pRCC->Roots[pRCC->FirstFreeRootIndex];
                    SF_ASSERT(u.UPtrValue & 1);
                    u.SPtrValue >>= 1; // keep the most significant bit, needed to get 
                                       // 0xFFFFFFFF if shifting the 0xFFFFFFFF (~0u).
                    pRCC->Roots[pRCC->FirstFreeRootIndex] = this;
                    pRCC->FirstFreeRootIndex = u.UPtrValue;
                }
                else
                {
                    // no free roots, just append.
                    SetBuffered((UInt32)pRCC->Roots.GetSize()); // save root index for 
                                                               // fast access from Release
                    // Append to roots
                    pRCC->Flags |= RefCountCollector<Stat>::Flags_AddingRoot;

                    if (!pRCC->Roots.PushBackSafe(this))
                    {
                        // can't allocate memory for roots.
                        pRCC->Flags &= ~RefCountCollector<Stat>::Flags_AddingRoot;

                        // call Collect since Collect was disabled during the PushBack...
                        bool collected = pRCC->Collect();

                        // try to PushBack again
                        pRCC->Flags |= RefCountCollector<Stat>::Flags_AddingRoot;
                        if (!collected || !pRCC->Roots.PushBackSafe(this))
                        {
                            // well, still no luck... Clear "buffered", set state "InUse".
                            ClearBuffered();
                            SetState(State_InUse);
                        }
                        pRCC->Flags &= ~RefCountCollector<Stat>::Flags_AddingRoot;
                    }
                    else
                        pRCC->Flags &= ~RefCountCollector<Stat>::Flags_AddingRoot;
                }
            }
        }
    }
}

template <int Stat>
inline
void RefCountBaseGC<Stat>::MarkInCycle(GFC_GC_DEBUG_PARAMS) const
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    if (!IsInState(State_InCycle))
    {
        GFC_GC_TRACE("Marking 'incycle' ");
        SF_ASSERT(pRCC);
        SetState(State_InCycle);

        ExecuteForEachChild_GC(Operation_MarkInCycle);
    }
}

template <int Stat>
inline
void RefCountBaseGC<Stat>::ScanInUse(GFC_GC_DEBUG_PARAMS) const
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    GFC_GC_TRACE("ScanInUse ");
    SetState(State_InUse);

    ExecuteForEachChild_GC(Operation_ScanInUse);
}

template <int Stat>
inline
bool RefCountBaseGC<Stat>::CollectGarbage(GFC_GC_DEBUG_PARAMS) const
{
    if (IsInState(State_Garbage) && !IsBuffered())
    {
        SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
        GFC_GC_TRACE("Collecting garbage ");
        SetState(State_InUse);

        ExecuteForEachChild_GC(Operation_CollectGarbage);
        pRCC->AddToKillList(this);
       
        return true;
    }
    return false;
}

template <int Stat>
inline
void RefCountBaseGC<Stat>::Scan(GFC_GC_DEBUG_PARAMS) const
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    if (IsInState(State_InCycle))
    {
        if ((RefCount & Mask_RefCount) > 0)
            ScanInUse(GFC_GC_DEBUG_PASS_PARAMS);
        else
        {
            SetState(State_Garbage);
            GFC_GC_TRACE("Marking garbage ");

            ExecuteForEachChild_GC(Operation_Scan);
        }
    }
}

template <int Stat>
inline
void RefCountCollector<Stat>::AddToKillList(const RefCountBaseGC<Stat>* proot)
{
    if (!proot->IsInState(RefCountBaseGC<Stat>::State_Removed))
    {
        // !!! Hack.
        const_cast<RefCountBaseGC<Stat>*>(proot)->pNextToKill = pFirstToKill;
        pFirstToKill = proot;
        proot->SetState(RefCountBaseGC<Stat>::State_Removed);
    }
}

template <int Stat>
inline
bool RefCountCollector<Stat>::Collect(Stats* pstat)
{
    // It is forbidden to call collection if the collector is already
    // collecting or if new root is being added. So, do nothing in 
    // this case.
    if (IsCollecting() || IsAddingRoot() || Roots.GetSize() == 0)
    {
        // already collecting - do nothing.
        if (pstat)
        {
            memset(pstat, 0, sizeof(*pstat));
        }
        return false;
    }

    GFC_GC_PRINT("++++++++ Starting collecting");
    UPInt  initialNRoots     = 0;
    UPInt  totalKillListSize = 0;
    SF_ASSERT(!pFirstToKill);
    
    // In most cases this loop will make only one iteration per call.
    // But in some cases, processing of kill list may produce new roots
    // to collect. This may happen, for example, when collectible object (A)
    // contains a strong ptr to non-collectible one (B), and that non-collectible
    // object has a Ptr to another collectible object (C) with refcnt > 1.
    // When Finalize_GC for A is called, it releases the ptr on B and this cause
    // invocation of Release for C. Release for C will add the pointer to Roots,
    // since refcnt is not 1. In this case the second pass will be required.
    do 
    {
        Flags |= Flags_Collecting;
        
        UPInt i, nroots = Roots.GetSize();
        UPInt lastIdx = 0;
        UPInt lastSize = 0;
        initialNRoots += (unsigned)nroots;

        // Mark roots stage
        for (i = 0; i < nroots; ++i)
        {
            union
            {
                const RefCountBaseGC<Stat>* Ptr;
                UPInt                       PtrValue;
            } u;
            u.Ptr = Roots[i];
            if (u.PtrValue & 1) 
                continue; // free root, skip
            const RefCountBaseGC<Stat>* proot = u.Ptr;

            if (proot->IsInState(RefCountBaseGC<Stat>::State_Root))
                proot->MarkInCycle(GFC_GC_PARAM_START_VAL);
            else
            {
                proot->ClearBuffered();
                if (proot->IsInState(RefCountBaseGC<Stat>::State_InUse) && proot->IsRefCntZero())
                {
                    // shouldn't get here, since Release frees immediately now
                    SF_ASSERT(0);
                }
            }
            // compacting roots
            Roots[lastIdx++] = Roots[i];
            ++lastSize;
        }
        Roots.Resize(lastSize);
        nroots = lastSize;

        // Scan roots
        for (i = 0; i < nroots; ++i)
        {
            const RefCountBaseGC<Stat>* proot = Roots[i];
            if (proot)
                proot->Scan(GFC_GC_PARAM_START_VAL);
        }

        // Collect roots
        for (i = 0; i < nroots; ++i)
        {
            const RefCountBaseGC<Stat>* proot = Roots[i];
            if (proot)
            {
                proot->ClearBuffered();
                proot->CollectGarbage(GFC_GC_PARAM_START_VAL);
            }
        }

        // cleanup roots
        Roots.Clear();
        Flags &= ~Flags_Collecting;
        FirstFreeRootIndex = SF_MAX_UPINT;

        // Process KillList
        const RefCountBaseGC<Stat>* pcur, *pnext;
        for (i = 0, pcur = pFirstToKill; pcur; pcur = pnext, ++i)
        {
            pnext = pcur->pNextToKill;
            pcur->Free_GC(GFC_GC_PARAM_START_VAL);
        }
        totalKillListSize += i;

        // cleanup
        pFirstToKill = NULL;
    } while (Roots.GetSize() > 0);
    
    if (pstat)
    {
        // save stats. Note totalKillListSize might be greater than initialNRoots,
        // since it might free some non-root elements as well. So, just correct RootsFreedTotal
        // to avoid negative difference between RootsNumber and RootsFreedTotal.
        pstat->RootsNumber              = (unsigned)initialNRoots;
        pstat->RootsFreedTotal          = Alg::Min((unsigned)initialNRoots, (unsigned)totalKillListSize);
    }
    GFC_GC_PRINT("-------- Finished collecting\n");
    return true;
}

template <int Stat>
inline
void RefCountCollector<Stat>::ShrinkRoots()
{
     if (Roots.GetSize() == 0)
         Roots.ClearAndRelease();
}

class ASRefCountCollector : public RefCountCollector<Mem_Stat>
{
    unsigned    FrameCnt;
    unsigned    MaxRootCount;
    unsigned    PeakRootCount;
    unsigned    LastRootCount;
    unsigned    LastCollectedRoots;
    unsigned    LastPeakRootCount;
    unsigned    TotalFramesCount;
    unsigned    LastCollectionFrameNum;

    unsigned    PresetMaxRootCount;
    unsigned    MaxFramesBetweenCollections;

    bool        CollectionScheduledFlag;

    void Collect(Stats* pstat = NULL)
    {
        RefCountCollector<Mem_Stat>::Collect(pstat);
    }
public:
    ASRefCountCollector();

    void SetParams(unsigned frameBetweenCollections, unsigned maxRootCount);

    // This method should be called every frame (every full advance). 
    // It evaluates necessity of collection and performs it if necessary.
    void AdvanceFrame(unsigned* movieFrameCnt, unsigned* movieLastCollectFrame);

    // Forces collection.
    void ForceCollect();
    
    // Forces emergency collect. This method is called when memory heap cap is 
    // reached. It tries to free as much memory as possible.
    void ForceEmergencyCollect();

    void ScheduleCollect() { CollectionScheduledFlag = true; }
    void CollectIfScheduled()
    {
        if (CollectionScheduledFlag)
            ForceCollect();
    }
};

typedef RefCountBaseGC<Mem_Stat> GASRefCountBaseType;

class GASRefCountBase : public GASRefCountBaseType 
{
public:
    GASRefCountBase(ASRefCountCollector* pcc) :
      GASRefCountBaseType(pcc) {}
private:
    GASRefCountBase(const GASRefCountBase&);
};

#define GASWeakPtr Ptr

///////////////////////////////////////////////////////////////////////////
void ForEachChild_GC(const ValueArray& v, RefCountBaseGC<Mem_Stat>::GcOp op);
void ForEachChild_GC(const ValueArrayDH& v, RefCountBaseGC<Mem_Stat>::GcOp op);

void ForEachChild_GC(const Value& v, RefCountBaseGC<Mem_Stat>::GcOp op);

template <typename T, int Stat>   
inline
void ForEachChild_GC(const SPtr<T>& v, typename RefCountBaseGC<Stat>::GcOp op)
{
    if (v)
        (*op)(const_cast<const AS3::RefCountBaseGC<Stat>**>(reinterpret_cast<AS3::RefCountBaseGC<Stat>**>(v.GetRawPtr())));
}

template <typename T, int Stat>   
inline
void ForEachChild_GC_Const(const SPtr<const T>& v, typename RefCountBaseGC<Stat>::GcOp op)
{
    if (v)
        (*op)(reinterpret_cast<const AS3::RefCountBaseGC<Stat>**>(v.GetRawPtr()));
}

/////////////////////////////////////////////////////////////////////////// 
// APtr is similar to SPtr, except it can behave as a raw pointer ( no
// AddRef/Release called )

template <typename T>
class APtr
{
public:
    typedef T       ObjType;
    typedef APtr<T> SelfType;
    enum Type {Strong, Raw};

public:
    // Constructors.

    APtr(const SPtr<T>& sptr, Type t = Strong)
        : ptr((sptr.AddRef(), sptr.Get()))
        , PtrType(t)
    {
    }
    APtr(T* p = NULL, Type t = Strong)
        : ptr(p ? p->AddRef(), p : p)
        , PtrType(t)
    {
        SF_ASSERT(p == NULL || (Scaleform::UPInt)p > 4);
    }
    // Copy constructor.
    APtr(const APtr& other)
        : ptr((other.AddRef(), other.ptr))
        , PtrType(other.PtrType)
    {
    }
    ~APtr()
    {
        Release();
    }

public:
    Type GetType() const
    {
        return PtrType;
    }
    void SetTypeStrong()
    {
        if (PtrType != Strong)
        {
            PtrType = Strong;

            // Make it a strong pointer
            AddRef();
        }
    }
    void SetTypeRaw()
    {
        if (PtrType != Raw)
        {
            // Make it a raw pointer
            Release();

            PtrType = Raw;
        }
    }

public:
    SelfType& operator =(T* p)
    {
        return SetPtr(p);
    }
    SelfType& operator =(const SelfType& other)
    {
        return Set(other);
    }
    template <typename OT>
    SelfType& operator =(const SPtr<OT>& sptr)
    {
        return Set(sptr);
    }
    SelfType& SetPtr(T* p)
    {
        if (p != ptr)
        {
            SF_ASSERT(p == NULL || (Scaleform::UPInt)p > 4);

            Release();
            if (p && PtrType == Strong)
                p->AddRef();

            ptr = p;
        }

        return *this;
    }
    SelfType& Set(const SelfType& other)
    {
        if (&other != this)
        {
            Release();
            other.AddRef();

            ptr = other.ptr;
            PtrType = other.PtrType;
        }

        return *this;
    }
    template <typename OT>
    SelfType& Set(const SPtr<OT>& sptr)
    {
        if (sptr.Get() != ptr)
        {
            Release();
            // !!! This call should be here.
            //sptr.AddRef().

            ptr = sptr.Get();
            AddRef();
        }

        return *this;
    }

    T* operator->() const
    {
        return ptr;
    }
    T& operator*() const
    {
        SF_ASSERT(ptr);
        return *ptr;
    }

    // Obtain pointer reference directly, for D3D interfaces
    T*& GetRawRef() { return ptr; }
    T** GetRawPtr() const { return &ptr; }
    T* Get() const
    {
        return ptr;
    }
    // Conversion
    operator T*() const
    {
        return ptr;
    }

private:
    void AddRef() const
    {
        if (ptr && PtrType == Strong)
            ptr->AddRef();
    }
    void Release()
    {
        if (ptr && PtrType == Strong)
        {
            if ((Scaleform::UPInt)ptr & 1)
                ptr = (T*)((Scaleform::UPInt)ptr - 1);
            else
                ptr->Release();
        }
    }
private:
    mutable T*      ptr;
    mutable Type    PtrType;
};


}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_GC_H

