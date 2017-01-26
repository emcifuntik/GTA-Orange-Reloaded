/**********************************************************************

Filename    :   Render_PrimitiveBundle.cpp
Content     :   PrimitiveBundle classes, SortKey and sorting support
Created     :
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

#include "Render_PrimitiveBundle.h"
#include "Renderer2DImpl.h" // for GetHAL() inline
#include "Render_Queue.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

// ***** DrawableBundle
//

// ***** PrimitiveBundle

// Insertions & removals should manipulate the Primitive and Matrices
PrimitiveBundle::PrimitiveBundle(TreeCacheRoot *proot, const SortKey& key,
                                 Renderer2DImpl* pr2d)
: DrawableBundle(proot, pr2d), Prim(GetHAL(), key.GetPrimitiveFill())
{    
}

void    PrimitiveBundle::CheckMatrixConsistency()
{
    unsigned count = Prim.GetMeshCount();

    for (unsigned i = 0; i< count; i++)
    {
        // Compute parent matrix.
        TreeCacheMeshBase* pshape = static_cast<TreeCacheMeshBase*>(Entries[i]->pSourceNode);
        Matrix             m(pshape->GetMatrixSafe());
        TreeCacheNode*     p = pshape->GetParent();

        while(p)
        {
            m.Append(p->GetNodeData()->M2D());
            p = p->GetParent();
        }        

        const Matrix &m2 = Prim.GetMatrix(i).GetMatrix2D();
        if (m2 != m)
            SF_ASSERT(0);
    }
}

void PrimitiveBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    DrawableBundle::InsertEntry(index, entry);
    TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
    SF_ASSERT(!source->M.IsNull()); 
    Prim.Insert((unsigned)index, (Mesh*)source->GetMesh(), source->M);
}

void PrimitiveBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    DrawableBundle::RemoveEntries(index, count);
}

void PrimitiveBundle::UpdateMesh(BundleEntry* entry)
{
    UPInt index;
    if (findEntryIndex(&index, entry))
    {
        TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
        Prim.SetMesh((unsigned)index, (Mesh*)source->GetMesh());
    }
}


//--------------------------------------------------------------------
// ***** Complex Primitive Bundle

ComplexPrimitiveBundle::ComplexPrimitiveBundle() : Bundle(0)
{
}
ComplexPrimitiveBundle::~ComplexPrimitiveBundle()
{
}

void ComplexPrimitiveBundle::Draw(HAL* hal)
{   
    // We need to emit every ComplexMesh as a separate draw call, since it is
    // a unit of caching that may need to be Prepared and emitted separately.
    // To do this, we count the numbed of consecutive meshes and pass a start
    // index for each new mesh start.

    UPInt count = Instances.GetSize();
    for (UPInt i = 0; i < count; i += countConsecutiveMeshesAtIndex(i))
        hal->Draw(this, (void*)(i)); // Pass index; count will be re-computed.
}


void ComplexPrimitiveBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    
    TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
    ComplexMesh *      complexMesh = (ComplexMesh*)source->GetMesh();
    SF_ASSERT(!source->M.IsNull());

    InstanceEntry e = { source->M, complexMesh };
    Instances.InsertAt(index, e);   
}

void ComplexPrimitiveBundle::RemoveEntries(UPInt index, UPInt count)
{
    Instances.RemoveMultipleAt(index, count);
    Bundle::RemoveEntries(index, count);
}

void ComplexPrimitiveBundle::UpdateMesh(BundleEntry* entry)
{
    UPInt index;
    if (findEntryIndex(&index, entry))
    {
        TreeCacheMeshBase* source = (TreeCacheMeshBase*)entry->pSourceNode;
        Instances[index].pMesh = (ComplexMesh*)source->GetMesh();
    }
}


RenderQueueItem::QIPrepareResult
ComplexPrimitiveBundle::Prepare(RenderQueueItem& item,
                                RenderQueueProcessor& qp, bool waitForCache)
{
    // Upload into cache if not there already.
    UPInt        index = (UPInt)item.GetData();
    ComplexMesh* mesh = Instances[index].pMesh;

    bool prepareResult = qp.GetMeshCache()->PrepareComplexMesh(mesh, waitForCache);

    // We use PrepareItemBuffer::IsProcessing(this) state as a flag, setting it only if 
    // PrepareComplexPrimitive failed initially. In this case, we want to return QIP_NeedCache
    // once, which would force any items in render queue before us to be flushed,
    // freeing up cache for another Prepare attempt.
    // This means that our logic is as follows:
    //  - If we succeed the first time, just emit Done.
    //  - When we fail the first time, we set StartProcessing(this), causing a flush.
    //  - When we fail the second time, clear IsProcessing and return Done. In this
    //    case Emit will simply skip us if there is no MeshBatchData.

    if (prepareResult || qp.GetPrepareItemBuffer()->IsProcessing(&item))
    {
        qp.GetPrepareItemBuffer()->StartProcessing(0);
        return RenderQueueItem::QIP_Done;
    }

    qp.GetPrepareItemBuffer()->StartProcessing(&item);
    return RenderQueueItem::QIP_NeedCache;
}

void ComplexPrimitiveBundle::EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
{
    // Execute rendering command on HAL...
    UPInt        index = (UPInt)item.GetData();
    ComplexMesh* mesh = Instances[index].pMesh;

    if (!qp.GetPrepareItemBuffer()->IsProcessing(&item) && mesh)
    {
        // Re-count the number of instances what we combined,
        // this is the same loop as was used to determine the index.
        UPInt  instanceCount = countConsecutiveMeshesAtIndex(index);
        StrideArray<HMatrix> matrices(&Instances[index].M, instanceCount,
                                      sizeof(InstanceEntry));
        qp.GetHAL()->DrawProcessedComplexMeshes(mesh, matrices);
    }
}


//--------------------------------------------------------------------
// ***** MaskBundle

// Insertions & removals should manipulate the Primitive and Matrices
MaskBundle::MaskBundle(HAL* hal, MaskPrimitive::MaskAreaType maskType)
 : Bundle(0), Prim(hal, maskType)
{
}

void MaskBundle::InsertEntry(UPInt index, BundleEntry* entry)
{
    Bundle::InsertEntry(index, entry);
    // Obtains "Mask Clear" matrix rectangle.
    MaskEffect* maskEffect = entry->pSourceNode->Effects.GetMaskEffect();
    SF_ASSERT(maskEffect);
    Prim.Insert(index, maskEffect->GetBoundsMatrix());
}

void MaskBundle::RemoveEntries(UPInt index, UPInt count)
{
    Prim.Remove((unsigned)index, (unsigned)count);
    Bundle::RemoveEntries(index, count);
}



//--------------------------------------------------------------------
// ***** SortKey for Primitive/ComplexPrimitive implementation

class SKI_Primitive : public SortKeyInterface_RefCountImpl
{
public:
    SKI_Primitive()
        : SortKeyInterface_RefCountImpl(SortKey_PrimitiveFill, SKF_MatchNoOverlap|SKF_MatchSingleOverlap)
    { }

    static SKI_Primitive Instance;

    virtual bool UpdateBundleEntry(SortKeyData d, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl* r)
    {
        if (!p->pBundle)
        {
            SortKey ourKey((PrimitiveFill*)d);
            Ptr<DrawableBundle> bundle = *SF_HEAP_AUTO_NEW(tr) PrimitiveBundle(tr, ourKey, r);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl*)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_Primitive::DrawBundleEntry");
        if (p->pBundle)
            ((PrimitiveBundle*)p->pBundle.GetPtr())->Draw();
    }
};

class SKI_ComplexPrimitive : public SortKeyInterface
{
public: 
    SKI_ComplexPrimitive()
        : SortKeyInterface(SortKey_MeshProvider, SKF_MatchNoOverlap)
    { }

    static SKI_ComplexPrimitive Instance;

    // Use custom Ref-Counting since MeshProvider doesn't necessarily
    // start with RefCountImpl* base.
    virtual void AddRef(SortKeyData p)  { if (p) ((MeshProvider*)p)->AddRef(); }
    virtual void Release(SortKeyData p) { if (p) ((MeshProvider*)p)->Release(); }

    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl*)
    {
        if (!p->pBundle)
        {
            Ptr<ComplexPrimitiveBundle> bundle = 
                *SF_HEAP_AUTO_NEW(tr) ComplexPrimitiveBundle();
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_ComplexPrimitive::DrawBundleEntry");
        if (p->pBundle)
        {
            ComplexPrimitiveBundle* bundle = (ComplexPrimitiveBundle*)p->pBundle.GetPtr();
            bundle->Draw(r->GetHAL());
        }
    }
};


SKI_Primitive        SKI_Primitive::Instance;
SKI_ComplexPrimitive SKI_ComplexPrimitive::Instance;

SortKey::SortKey(PrimitiveFill* fill)
{
    initKey(&SKI_Primitive::Instance, fill);
}

SortKey::SortKey(MeshProvider* fill)
{
    initKey(&SKI_ComplexPrimitive::Instance, fill);
}



//--------------------------------------------------------------------
// ***** SortKey interfaces and constructors for Masks

class SKI_MaskStart : public SortKeyInterface
{
public:
    SKI_MaskStart(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    static SKI_MaskStart Combinable_Instance;
    static SKI_MaskStart Clipped_Instance;

    virtual bool UpdateBundleEntry(SortKeyData, BundleEntry* p,
                                   TreeCacheRoot* tr, Renderer2DImpl* r)
    {
        if (!p->pBundle)
        {
            MaskPrimitive::MaskAreaType maskType = 
                (Type == SortKey_MaskStartClipped) ?
                MaskPrimitive::Mask_Clipped : MaskPrimitive::Mask_Combinable;

            Ptr<MaskBundle> bundle = *SF_HEAP_AUTO_NEW(tr) MaskBundle(r->GetHAL(), maskType);
            p->SetBundle(bundle, 0);
        }
        return p->pBundle.GetPtr() != 0;
    }

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if (other.GetType() == SortKey_MaskEnd)
        {
            SF_ASSERT(SortKeyMask_End == (UPInt)other.GetData());
            return SKT_Replace;
        }
        return SKT_Skip;
    }

    virtual void DrawBundleEntry(SortKeyData, BundleEntry* p, Renderer2DImpl* r)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_MaskStart::DrawBundleEntry");
        if (p->pBundle)
        {
            MaskBundle *bundle = (MaskBundle*)p->pBundle.GetPtr();
            bundle->Draw(r->GetHAL());
        }
    }
};


class SKI_MaskEnd : public SortKeyInterface
{
public:
    SKI_MaskEnd(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    class RQII_EndMask : public RenderQueueItem::Interface
    {
    public:
        virtual void EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
        {
            HAL*      hal = qp.GetHAL();
            SortKeyMaskType maskType = (SortKeyMaskType)(UPInt)item.GetData();
            if (maskType == SortKeyMask_End)
                hal->EndMaskSubmit();
            else if (maskType == SortKeyMask_Pop)
                hal->PopMask();
        }        
    };

    static RQII_EndMask RQII_Instance;
    static SKI_MaskEnd  Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData data, const SortKey& other)
    {
        if (other.GetType() == SortKey_MaskEnd)
        {
            SF_ASSERT(SortKeyMask_End == (UPInt)data);
            SF_ASSERT(SortKeyMask_Pop == (UPInt)other.GetData());
            return SKT_End;
        }
        SF_UNUSED(data);
        return SKT_Skip;
    }

    virtual void DrawBundleEntry(SortKeyData data, BundleEntry*, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_MaskEnd::DrawBundleEntry");
        RenderQueueItem rqi(&RQII_Instance, data);
        r2d->GetHAL()->Draw(rqi);
    }
};

// Static SortKeyInterface instances for masks.
SKI_MaskStart SKI_MaskStart::Combinable_Instance(SortKey_MaskStart,
                                                SKF_MatchNoOverlap|SKF_RangeStart);
SKI_MaskStart SKI_MaskStart::Clipped_Instance(SortKey_MaskStartClipped, SKF_RangeStart);
SKI_MaskEnd   SKI_MaskEnd::Instance(SortKey_MaskEnd, SKF_MatchNoOverlap);

SKI_MaskEnd::RQII_EndMask SKI_MaskEnd::RQII_Instance;

static SortKeyInterface* SortKeyMaskInterfaces[SortKeyMask_Item_Count] =
{
    &SKI_MaskStart::Combinable_Instance,
    &SKI_MaskStart::Clipped_Instance,
    &SKI_MaskEnd::Instance,
    &SKI_MaskEnd::Instance
};

SortKey::SortKey(SortKeyMaskType maskType)
{    
    initKey(SortKeyMaskInterfaces[maskType], (SortKeyData)maskType);

    // Checks for array above.
    SF_COMPILER_ASSERT(SortKeyMask_Push == 0);
    SF_COMPILER_ASSERT(SortKeyMask_Pop == 3);
}



//--------------------------------------------------------------------
// ***** SortKey Interfaces for BlendMode

class SKI_BlendMode : public SortKeyInterface
{
public:
    SKI_BlendMode(SortKeyType type, unsigned flags) : SortKeyInterface(type, flags)
    { }

    class RQII_BlendMode : public RenderQueueItem::Interface
    {
    public:
        virtual void EmitToHAL(RenderQueueItem& item, RenderQueueProcessor& qp)
        {
            HAL* hal = qp.GetHAL();
            UPInt      data = (UPInt)item.GetData();

            // Push/Pop blend mode.
            if (data == 0xFFFFFFFF)
                hal->PopBlendMode();
            else
                hal->PushBlendMode((BlendMode)data);
        }
    };

    static RQII_BlendMode RQII_Instance;
    static SKI_BlendMode  Start_Instance;
    static SKI_BlendMode  End_Instance;

    virtual SortKeyTransition GetRangeTransition(SortKeyData, const SortKey& other)
    {
        if ((other.GetType() == SortKey_BlendModeEnd) && (Type == SortKey_BlendModeStart))
            return SKT_End;
        return SKT_Skip;
    }

    virtual void DrawBundleEntry(SortKeyData data, BundleEntry*, Renderer2DImpl* r2d)
    {
        SF_AMP_SCOPE_RENDER_TIMER("SKI_BlendMode::DrawBundleEntry");
        RenderQueueItem rqi(&RQII_Instance, data);
        r2d->GetHAL()->Draw(rqi);
    }
};


// Keys
SKI_BlendMode SKI_BlendMode::Start_Instance(SortKey_BlendModeStart, SKF_RangeStart|
                                                                    SKF_MatchNoOverlap|SKF_MatchSingleOverlap);
SKI_BlendMode SKI_BlendMode::End_Instance(SortKey_BlendModeEnd, SKF_MatchNoOverlap|SKF_MatchSingleOverlap);

SKI_BlendMode::RQII_BlendMode SKI_BlendMode::RQII_Instance;


// Blend Mode SortKey constructor.
SortKey::SortKey(SortKeyType blendKeyType, BlendMode mode)
{    
    SF_ASSERT((blendKeyType == SortKey_BlendModeStart) || (blendKeyType == SortKey_BlendModeEnd));
    
    if (blendKeyType == SortKey_BlendModeStart)
        initKey(&SKI_BlendMode::Start_Instance, (SortKeyData)mode);
    else
        initKey(&SKI_BlendMode::End_Instance, (SortKeyData)0xFFFFFFFF);
}


}} // Scaleform::Render
