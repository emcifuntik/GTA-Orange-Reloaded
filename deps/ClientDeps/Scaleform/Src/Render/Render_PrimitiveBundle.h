/**********************************************************************

Filename    :   Render_PrimitiveBundle.h
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

#ifndef INC_SF_Render_PrimitiveBundle_H
#define INC_SF_Render_PrimitiveBundle_H

#include "Render_TreeCacheNode.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {



// ***** DrawableBundle


class DrawableBundle : public Bundle
{
protected:
    TreeCacheRoot*  pRoot;    
    Renderer2DImpl* pRenderer2D;

public:
    DrawableBundle(TreeCacheRoot *proot, Renderer2DImpl* pr2d)
        : Bundle(0), pRoot(proot), pRenderer2D(pr2d)
    { }

#ifndef SF_CC_ARM
    inline
#endif
    HAL*  GetHAL() const;
    inline Renderer2DImpl* GetRenderer() const { return pRenderer2D; }

    // Interface used by TreeCacheShape/ShapeLayer
    // Needed because ComplexPrimitiveBundle represents its data differently.
    virtual void    UpdateMesh(BundleEntry* entry) = 0;

    //virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    //virtual void    RemoveEntries(UPInt index, UPInt count);
};


//--------------------------------------------------------------------
// ***** PrimitiveBundle

// PrimitiveBundle maintains a primitive that a BundleLayer tree corresponds to,
// with the top of the tree being pTop. In-order traversal of the tree should yield
// shapes in the order of MatrixSourceNodes.

class PrimitiveBundle : public DrawableBundle
{
public:    
    Primitive Prim;
    
    // Insertions & removals should manipulate the Primitive and Matrices
    PrimitiveBundle(TreeCacheRoot *proot, const SortKey& key, Renderer2DImpl* pr2d);

    void    Draw()
    {        
        GetHAL()->Draw(&Prim);
    }

    virtual void    UpdateMesh(BundleEntry* entry);
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);

    void    CheckMatrixConsistency();   
};


// ***** ComplexPrimitiveBundle

class ComplexPrimitiveBundle : public Bundle, public RenderQueueItem::Interface
{
public:
    ComplexPrimitiveBundle();
    ~ComplexPrimitiveBundle();

    void    Draw(HAL* hal);
    virtual void    UpdateMesh(BundleEntry* entry);
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
    
    // RenderQueueItem::Interface implementation
    virtual QIPrepareResult  Prepare(RenderQueueItem&, RenderQueueProcessor&, bool);
    virtual void             EmitToHAL(RenderQueueItem&, RenderQueueProcessor&);

private:

    struct InstanceEntry
    {
        HMatrix          M;
        Ptr<ComplexMesh> pMesh;
    };
    
    UPInt   countConsecutiveMeshesAtIndex(UPInt i) const
    {
        UPInt        j = i, count = Instances.GetSize();
        ComplexMesh* mesh = Instances[i].pMesh;
        while((j < (count-1)) && (Instances[j+1].pMesh == mesh))
            j++;
        return j-i+1;
    }

    // For now, keep one primitive per element
    ArrayLH<InstanceEntry> Instances;
};



// ***** MaskBundle

// MaskBunle maintains MaskPrimitive and updates it by pulling mask dat from nodes.

class MaskBundle : public Bundle
{
public:    
    MaskPrimitive Prim;

    // Insertions & removals should manipulate the Primitive and Matrices
    MaskBundle(HAL* hal, MaskPrimitive::MaskAreaType maskType);

    void    Draw(HAL* hal)
    {
        hal->Draw(&Prim);
    }
    
    virtual void    InsertEntry(UPInt index, BundleEntry* shape);
    virtual void    RemoveEntries(UPInt index, UPInt count);
};



}} // Scaleform::Render

#endif

