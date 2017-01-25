/**********************************************************************

Filename    :   Renderer2DImpl.h
Content     :   Renderer2D internal implementation header
Created     :   November 3, 2010
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

#ifndef INC_SF_Render_Renderer2DImpl_H
#define INC_SF_Render_Renderer2DImpl_H

#include "Renderer2D.h"

//#include "Render_Containers.h"
//#include "Render_TreeCacheNode.h"
#include "Render_PrimitiveBundle.h"
#include "Render_HAL.h"
#include "Render_TessGen.h"
#include "Render_MeshKey.h"

#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// ***** Renderer2DImpl
 
/*
    Sorted Renderer2D implementation optimizes rendering by keeping cached
    versions of the rendering tree nodes and their resulting primitives. The cached tree nodes
    and primitive bundles are created on the first Draw call for a given TreeRoot; future
    rendering calls are then proceed by traversing the cached Primitive structures and not
    the entire rendering tree, greatly improving performance. Since the RenderContext tracks
    changes made to the tree nodes, Renderer2D processes these changes before every frame,
    ensuring the displayed tree is correct. Being able to process these change lists
    efficiently, that is with minimal object traversal and modification, is critical to
    good performance during animation.

    Since best rendering performance is achieved by batching multiple DrawPrimitive calls
    into one, rendering data structures are designed to combine consecutive Shapes of the
    same Key (representing a unique rendering state) into one HAL primitive. This combining
    is achieved by Sorter namespace classes, which maintain sorted list of Bundle objects,
    with each Bundle corresponding to one or more consecutive primitives of the same key.
    
*/

class Renderer2DImpl : public NewOverrideBase<StatRender_Mem>,
                       public Context::RenderNotify, public HALNotify
{
    HAL*                    pHal;
    MeshGenerator           MeshGen;
    StrokeGenerator         StrokeGen;
    ToleranceParams         Tolerances;
    PrimitiveFillManager    FillManager;
    MatrixPool              MPool;
    Ptr<MeshKeyManager>     pMeshKeyManager;
    Ptr<GlyphCache>         pGlyphCache;
    GlyphCacheParams        mGlyphCacheParam;

    void* getThis() { return this; }

public:
    Renderer2DImpl(HAL* phal);
    ~Renderer2DImpl();

    HAL*                    GetHAL()        const { return pHal; }
    MeshGenerator*          GetMeshGen()          { return &MeshGen; }
    MeshKeyManager*         GetMeshKeyManager() const { return pMeshKeyManager; }
    GlyphCache*             GetGlyphCache() const { return pGlyphCache; }
    StrokeGenerator*        GetStrokeGen()        { return &StrokeGen; }

    const ToleranceParams&  GetToleranceParams() const { return Tolerances; }
    void                    SetToleranceParams(const ToleranceParams& params) { Tolerances=params; }

    MeshCacheConfig*        GetMeshCacheConfig() const;

    // Call this function to make the glyph cache work.
    void    SetGlyphCacheParam(const GlyphCacheParams& param);
    
    // Delegated interface.
    bool    BeginFrame();
    void    EndFrame();
    bool    BeginScene()    { return pHal->BeginScene(); }
    void    EndScene()      { pHal->EndScene(); }

    void    BeginDisplay(Color backgroundColor, const Viewport& viewport);   
    void    EndDisplay();

    // Renders a display tree starting with TreeRoot. TreeRoot must've
    // been allocated from the specified context.
    void    Draw(TreeRoot *pnode);
  
    PrimitiveFillManager&   GetPrimitiveFillManager() { return FillManager; }
    MatrixPool&             GetMatrixPool() { return MPool; }

protected:
    // Viewport and its HW matrix being used.
    Viewport            VP;

    // All created roots.
    List<TreeCacheNode> RenderRoots;

    // *** Context::RenderNotify implementation
    // Lifetime detection
    virtual void    EntryDestroy(Context::Entry*);
    virtual void    EntryFlush(Context::Entry*);
    virtual void    NewCapture() { }
    // Handle changes in display lists
    virtual void    EntryChanges(Context::ChangeBuffer&);

    // *** HALNotify implementation
    virtual void    OnHALEvent(HALNotifyType type);
};


inline HAL* DrawableBundle::GetHAL() const
{
    return pRenderer2D->GetHAL();
}

}} // Scaleform::Render

#endif
