/**********************************************************************

Filename    :   RefCountCollector.h
Content     :   Fast general purpose refcount-based garbage collector functions
Created     :   November 25, 2008
Authors     :   Artem Bolgar, Maxim Shemanarev

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFxl_AS2_RefCountCollector_H
#define INC_SF_GFxl_AS2_RefCountCollector_H

#include "Kernel/SF_Alg.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayPaged.h"

#ifdef SF_BUILD_DEBUG
//#define SF_GC_DO_TRACE
#endif

#ifdef SF_GC_DO_TRACE
#define SF_GC_DEBUG_PARAMS         int level
#define SF_GC_DEBUG_PARAMS_DEF     int level=0
#define SF_GC_DEBUG_PASS_PARAMS    level+1
#define SF_GC_TRACE(s)             Trace(level, s)
#define SF_GC_COND_TRACE(c,s)      do { if(c) Trace(level, s); } while(0)
#define SF_GC_PRINT(s)             printf("%s\n", s)
#define SF_GC_PRINTF(s,p1)         printf(s, p1)
#define SF_GC_PARAM_START_VAL      0
#else
#define SF_GC_DEBUG_PARAMS
#define SF_GC_DEBUG_PARAMS_DEF
#define SF_GC_DEBUG_PASS_PARAMS
#define SF_GC_TRACE(s)
#define SF_GC_COND_TRACE(c,s)
#define SF_GC_PRINT(s)
#define SF_GC_PRINTF(s,p1)
#define SF_GC_PARAM_START_VAL
#endif

namespace Scaleform {
namespace GFx {
namespace AS2 {

template <int Stat>
class RefCountCollector;

// This is a base class for all "refcount-collectables" classes.
// This class might be used instead of regular reference counter
// base class (RefCountBase) to provide functionality similar to
// garbage collection. It is still based on reference counters, however,
// it is possible to detect and free circular references. Use 
// RefCountCollector.Collect method to perform cycle references collection.
// Each instance of the "collectable" class may have so called "children".
// "Children" are strong pointers ("addref-ed", Ptr might be used) to
// another "collectable" objects. A "collectable" object may also contain
// strong or weak pointers on "non-collectable" objects.
//
// Destructors for "collectable" object should NEVER be called. The virtual
// method Finalize_GC will be invoked instead. However, the exact moment
// of this call is undefined.
// There is a limitation of what is possible to do inside the Finalize_GC.
// It is forbidden to touch any of "collectable" children. These pointers
// might be already invalid at the moment. Thus, no Release, no dtor calls
// for "collectables" from inside of the Finalize_GC (dtors are forbidden at all).
// For "non-collectables", such as regular non-refcounted objects, destructors
// should be called implicitly. For regular refcounted members Release functions
// should be called.
//
// Another limitation of Finalize_GC: it should not, directly or indirectly,
// cause invocation of Release for any "collectable" object, since "Release" 
// may cause adding new roots in root array and the system might be already in 
// collecting state. The only exception from this limitation is when it is guaranteed 
// that the refcount == 1 before Release is called. In this case, no roots will be
// added and the object will be destroyed instantly.
//
// To provide "children" to refcount collector it is necessary to add several
// methods in implementation of the "collectable" object. The first method is:
//
//     virtual void ExecuteForEachChild_GC(OperationGC operation) const;
//
// The implementation of this method is quite simple, for example:
//    void ASObject::ExecuteForEachChild_GC(OperationGC operation) const
//    {
//        ASRefCountBaseType::CallForEachChild<ASObject>(operation);
//    }
// Another method that is required to be defined is a template one, ForEachChild_GC:
//     template <class Functor> void ForEachChild_GC(SF_GC_DEBUG_PARAMS_DEF) const;
// Note, this method should be public or its class should be a friend to RefCountBaseGC<..>.
// The ForEachChild_GC should call static Functior::Call(RefCountBaseGC*) method for
// each "collectable child".
// This template method might be implemented inline, in the header file, or in CPP. 
// If the method is implemented in CPP then the source file should have the following
// statement right next to ForEachChild_GC implementation:
//      SF_GC_PREGEN_FOR_EACH_CHILD(ClassName)

template <int Stat = Stat_Default_Mem>
class RefCountBaseGC : public NewOverrideBase<Stat>
{
    friend class RefCountCollector<Stat>;
public:
    enum OperationGC
    {
        Operation_Release,
        Operation_MarkInCycle,
        Operation_ScanInUse,
        Operation_CollectGarbage,
        Operation_Scan
    };
protected:
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
        RefCountBaseGC<Stat>* pNextToKill;
    };
    // Ref counter takes bits 27-0 lower bytes.
    // The most significant bit (bit 31) is flag "buffered".
    // Bits 30-28 - color (7 values)
    UInt32              RefCount;
    UInt32              RootIndex;

    SF_INLINE void Increment(SF_GC_DEBUG_PARAMS)
    {
        SF_ASSERT((RefCount & Mask_RefCount) < Mask_RefCount);
        ++RefCount;
        SF_GC_TRACE("Incremented ");
    }
    SF_INLINE unsigned Decrement(SF_GC_DEBUG_PARAMS)
    {
        SF_ASSERT((RefCount & Mask_RefCount) > 0);
        SF_GC_TRACE("Decrementing ");
        return ((--RefCount) & Mask_RefCount);
    }

    void MarkInCycle(SF_GC_DEBUG_PARAMS);
    void ScanInUse(SF_GC_DEBUG_PARAMS);
    bool CollectGarbage(SF_GC_DEBUG_PARAMS);
    void Scan(SF_GC_DEBUG_PARAMS);

    struct ReleaseFunctor
    {
        SF_INLINE static void Call(RefCountBaseGC<Stat>* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Release();
        }
    };

    struct MarkInCycleFunctor
    {
        SF_INLINE static void Call(RefCountBaseGC<Stat>* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Decrement();
            pchild->MarkInCycle();
        }
    };
    struct ScanInUseFunctor
    {
        SF_INLINE static void Call(RefCountBaseGC<Stat>* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Increment();
            if (!pchild->IsInState(State_InUse))
                pchild->ScanInUse();
        }
    };

    struct CollectGarbageFunctor
    {
        SF_INLINE static void Call(RefCountBaseGC<Stat>* pchild)
        {
            SF_ASSERT(pchild);
            pchild->CollectGarbage();
        }
    };

    struct ScanFunctor
    {
        SF_INLINE static void Call(RefCountBaseGC<Stat>* pchild)
        {
            SF_ASSERT(pchild);
            pchild->Scan();
        }
    };

    // This template method is a kind of dispatcher function. It is called
    // from the virtual method ExecuteForEachChild_GC for every "collectable"
    // class that has "collectable" children. The "collectable" class should
    // also have a template method ForEachChild_GC.
    template <class ClassImpl>
    SF_INLINE void CallForEachChild(OperationGC op) const
    {
        switch(op)
        {
        case Operation_Release: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<ReleaseFunctor>(); 
            break;
        case Operation_MarkInCycle: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<MarkInCycleFunctor>(); 
            break;
        case Operation_ScanInUse: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<ScanInUseFunctor>(); 
            break;
        case Operation_CollectGarbage: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<CollectGarbageFunctor>(); 
            break;
        case Operation_Scan: 
            static_cast<const ClassImpl*>(this)->template ForEachChild_GC<ScanFunctor>(); 
            break;
        }
    }

    SF_INLINE bool IsRefCntZero() const { return (RefCount & Mask_RefCount) == 0; }

    SF_INLINE void SetBuffered(UInt32 rootIndex) 
    { 
        RefCount |= Flag_Buffered; RootIndex = rootIndex; 
    }
    SF_INLINE void ClearBuffered()    { RefCount &= ~Flag_Buffered; RootIndex = ~0u; }
    SF_INLINE bool IsBuffered() const { return (RefCount & Flag_Buffered) != 0; }

    SF_INLINE void SetState(States c)  
    { 
        RefCount = (RefCount & ~(Mask_State << Shift_State)) | 
                   (((UInt32)c & Mask_State) << Shift_State); 
    }
    SF_INLINE States GetState() const  { return (States)((RefCount >> Shift_State) & Mask_State); }
    SF_INLINE bool IsInState(States c) const { return GetState() == c; }

    SF_INLINE void Free_GC(SF_GC_DEBUG_PARAMS)
    {
        SF_GC_TRACE("Freeing ");
        Finalize_GC();
        SF_FREE(this);
    }

    // Virtual methods, which should be overloaded by the inherited classes.
    virtual void ExecuteForEachChild_GC(OperationGC operation) const { SF_UNUSED(operation); }
    virtual void Finalize_GC() = 0;

public:
    RefCountBaseGC(class RefCountCollector<Stat>* prcc) : pRCC(prcc), RefCount(1) { SF_ASSERT(pRCC); }

    void AddRef() 
    { 
        SF_ASSERT(pRCC);
        Increment(SF_GC_PARAM_START_VAL);
        SetState(State_InUse);
    }
    void Release(SF_GC_DEBUG_PARAMS_DEF);

    SF_INLINE RefCountCollector<Stat>* GetCollector() { return pRCC; }
 
protected:
    virtual ~RefCountBaseGC() // virtual to avoid gcc -Wall warnings
    {
        // destructor should never be invoked; Finalize_GC should be 
        // invoked instead.
        SF_ASSERT(0);
    }
private:
    // copying is prohibited
    RefCountBaseGC(const RefCountBaseGC&) { }
    RefCountBaseGC& operator=(const RefCountBaseGC&) { return *this; }

#ifdef SF_GC_DO_TRACE
    void Trace(int level, const char* text)
    {
        char s[1024] = {0};
        for (int i = 0; i < level; ++i)
            SFstrcat(s, sizeof(s), "  ");
        printf ("%s%s %p, state %d, refcnt = %d\n", s, text, this, GetState(), 
            (RefCount & RefCountBaseGC<Stat>::Mask_RefCount));
    }
#endif // SF_GC_DO_TRACE
};

// This macro should be used if it is necessary to pre-generate ForEachChild_GC
// template methods. Usually, this is necessary if it is impossible to put 
// inlined implementation of ForEachChild_GC into the header file. In this case,
// the header might contain only template declaration:
//
//        template <class Functor> void ForEachChild_GC(SF_GC_DEBUG_PARAMS_DEF) const;
// and the implementation might be placed into the CPP file. Right next to the 
// implementation this macro should be placed in the CPP file to pre-generate
// all necessary variants of the template.
#define SF_GC_PREGEN_FOR_EACH_CHILD(Class) \
template void Class::ForEachChild_GC<ASRefCountBaseType::ReleaseFunctor>() const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::MarkInCycleFunctor>() const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::ScanInUseFunctor>() const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::CollectGarbageFunctor>() const; \
template void Class::ForEachChild_GC<ASRefCountBaseType::ScanFunctor>() const;

// This class performs "refcount collection" (other words - garbage collection).
// See comments above how to organize "collectable" classes. The collector
// may allocate memory only during the "Release" calls for possible roots.
// No allocations are done during the Collect call.
template <int Stat = Stat_Default_Mem>
class RefCountCollector : public RefCountBase<RefCountCollector<Stat>, Stat>
{
    friend class RefCountBaseGC<Stat>;

    ArrayPagedLH_POD<RefCountBaseGC<Stat> *, 10, 5>     Roots;
    UPInt                                               FirstFreeRootIndex;
    RefCountBaseGC<Stat>*                               pFirstToKill;

    enum
    {
        Flags_Collecting,
        Flags_AddingRoot
    };
    UInt8 Flags;

    void AddToKillList(RefCountBaseGC<Stat>* proot);
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
void RefCountBaseGC<Stat>::Release(SF_GC_DEBUG_PARAMS)
{
    if (IsRefCntZero())
        return;
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());

    if (Decrement(SF_GC_DEBUG_PASS_PARAMS) == 0)
    {
        SF_GC_TRACE("Releasing ");

        // Release all children
        ExecuteForEachChild_GC(Operation_Release);

        SetState(State_InUse);

        if (!IsBuffered())
        {
            Free_GC(SF_GC_DEBUG_PASS_PARAMS);
        }
        else
        {
            SF_GC_TRACE("bufferred, find it roots and free.");

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
                    RefCountBaseGC<Stat>*   Ptr;
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

            Free_GC(SF_GC_DEBUG_PASS_PARAMS);
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
            SF_GC_COND_TRACE(!IsBuffered(), "Adding possible root");
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
                        RefCountBaseGC<Stat>*   Ptr;
                        UPInt                   UPtrValue;
                        SPInt                   SPtrValue;
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
void RefCountBaseGC<Stat>::MarkInCycle(SF_GC_DEBUG_PARAMS)
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    if (!IsInState(State_InCycle))
    {
        SF_GC_TRACE("Marking 'incycle' ");
        SF_ASSERT(pRCC);
        SetState(State_InCycle);

        ExecuteForEachChild_GC(Operation_MarkInCycle);
    }
}

template <int Stat>
void RefCountBaseGC<Stat>::ScanInUse(SF_GC_DEBUG_PARAMS)
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    SF_GC_TRACE("ScanInUse ");
    SetState(State_InUse);

    ExecuteForEachChild_GC(Operation_ScanInUse);
}

template <int Stat>
bool RefCountBaseGC<Stat>::CollectGarbage(SF_GC_DEBUG_PARAMS)
{
    if (IsInState(State_Garbage) && !IsBuffered())
    {
        SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
        SF_GC_TRACE("Collecting garbage ");
        SetState(State_InUse);

        ExecuteForEachChild_GC(Operation_CollectGarbage);
        pRCC->AddToKillList(this);
       
        return true;
    }
    return false;
}

template <int Stat>
void RefCountBaseGC<Stat>::Scan(SF_GC_DEBUG_PARAMS)
{
    SF_ASSERT(pRCC && pRCC->Roots.GetSize() == pRCC->Roots.GetSize());
    if (IsInState(State_InCycle))
    {
        if ((RefCount & Mask_RefCount) > 0)
            ScanInUse(SF_GC_DEBUG_PASS_PARAMS);
        else
        {
            SetState(State_Garbage);
            SF_GC_TRACE("Marking garbage ");

            ExecuteForEachChild_GC(Operation_Scan);
        }
    }
}

template <int Stat>
SF_INLINE void RefCountCollector<Stat>::AddToKillList(RefCountBaseGC<Stat>* proot)
{
    if (!proot->IsInState(RefCountBaseGC<Stat>::State_Removed))
    {
        proot->pNextToKill = pFirstToKill;
        pFirstToKill = proot;
        proot->SetState(RefCountBaseGC<Stat>::State_Removed);
    }
}

template <int Stat>
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

    SF_GC_PRINT("++++++++ Starting collecting");
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
                RefCountBaseGC<Stat>*   Ptr;
                UPInt                   PtrValue;
            } u;
            u.Ptr = Roots[i];
            if (u.PtrValue & 1) 
                continue; // free root, skip
            RefCountBaseGC<Stat>* proot = u.Ptr;

            if (proot->IsInState(RefCountBaseGC<Stat>::State_Root))
                proot->MarkInCycle(SF_GC_PARAM_START_VAL);
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
            RefCountBaseGC<Stat>* proot = Roots[i];
            if (proot)
                proot->Scan(SF_GC_PARAM_START_VAL);
        }

        // Collect roots
        for (i = 0; i < nroots; ++i)
        {
            RefCountBaseGC<Stat>* proot = Roots[i];
            if (proot)
            {
                proot->ClearBuffered();
                proot->CollectGarbage(SF_GC_PARAM_START_VAL);
            }
        }

        // cleanup roots
        Roots.Resize(0);
        Flags &= ~Flags_Collecting;
        FirstFreeRootIndex = SF_MAX_UPINT;

        // Process KillList
        RefCountBaseGC<Stat>* pcur, *pnext;
        for (i = 0, pcur = pFirstToKill; pcur; pcur = pnext, ++i)
        {
            pnext = pcur->pNextToKill;
            pcur->Free_GC(SF_GC_PARAM_START_VAL);
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
        pstat->RootsFreedTotal          = (unsigned)Alg::PMin(initialNRoots, totalKillListSize);
    }
    SF_GC_PRINT("-------- Finished collecting\n");
    return true;
}

template <int Stat>
void RefCountCollector<Stat>::ShrinkRoots()
{
    if (Roots.GetSize() == 0)
        Roots.ClearAndRelease();
}

}}} // Scaleform::GFx::AS2

#endif
