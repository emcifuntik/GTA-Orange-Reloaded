/**********************************************************************

Filename    :   Render_TreeCacheText.cpp
Content     :   TreeText Cache implementation
Created     :   2009-2010
Authors     :   Maxim Shemanarev, Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Render_TreeCacheText.h"

#include "Render_PrimitiveBundle.h" // For GetBundle inline
#include "Render_TextMeshProvider.h"
#include "Renderer2DImpl.h"

#include <stdio.h> // DBG

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
// ***** TreeCacheText implementation

TreeCacheText::TreeCacheText(TreeText* node, Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheMeshBase(node, SortKey(SortKeyText), prenderer2D, flags),
    pNextNoBatch(0), TMProvider(pRenderer2D->GetGlyphCache())
{
}

TreeCacheText::~TreeCacheText()
{
}



void TreeCacheText::UpdateCache(TreeCacheContainer* pparent, UInt16 depth)
{  
    SF_ASSERT(pParent == pparent);
    Depth = depth;
    pRoot = pparent->pRoot;

}

void TreeCacheText::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{
    // We override UpdateChildSubtree so that sortKey can be updated if necessary.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9)) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());
    if (data && data->Is3D())
        newFlags |= NF_3D;

    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
    }
    if (data)
        updateMaskCache(data, depth);
}



//------------------------------------------------------------------------
// Updates computed final matrix
void TreeCacheText::UpdateTransform(const TreeNode::NodeData* nodeData,
                                    const TransformArgs& t,
                                    TransformFlags flags)
{
    RectF cullRect(t.CullRect);
    updateCulling(nodeData, t, &cullRect, (TransformFlags) (flags | TF_CullCxform));
    SortParentBounds = nodeData->AproxParentBounds;
    SetFlags(GetFlags() & ~NF_ExpandedBounds );

    // Updates computed final HMatrix
    ComputeFinalMatrix(t, flags);

    // If we were updating Cxform only, skip the rest of the logic, which
    // depends only on matrix. (TBD: Handle Cxform alpha change).
    if (!(flags & TF_Matrix))
        return;

    // TO DO: Adjust matrix (snap) if necessary
    //    // TBD: This adjustment should really be done before (!bundle) return
    ////      above, but before updateMeshKey. Else its not applied in first frame...
    ////      Currently updateMeshKey isn't called immediately to avoid allocation
    ////      for hidden objects.
    //// TO DO: revise, consider offset in the MeshKey
    //if ((meshKeyFlags & MeshKey::KF_KeyTypeMask) == MeshKey::KF_StrokeHinted)
    //{
    //    Matrix m2 = t.Mat;
    //    m2.Tx() = floorf(m2.Tx() + ((m2.Tx() < 0) ? -0.5f : +0.5f));
    //    m2.Ty() = floorf(m2.Ty() + ((m2.Ty() < 0) ? -0.5f : +0.5f));
    //    M.SetMatrix2D(m2);
    //}

    const TreeText::NodeData* shapeNodeData = (const TreeText::NodeData*)nodeData;    
    Bundle* bundle = GetBundle();
    if (!bundle)
        return;

    TreeCacheNode* thisNode = this;
    bool updateMesh = false;

    if (!TMProvider.IsCreated())
    {
        // M.SetMatrix2D // if necessary
        return;
    }


    // TO DO: Verify
    //if (thisNode->pScale9Bounds)
    //{
    //    if (!thisNode->pScale9Bounds->IsValid())
    //    {
    //        RectF b = thisNode->pScale9Bounds->pOwner->CalcScale9GridBounds(Matrix2F());
    //        thisNode->pScale9Bounds->SetBounds(b);
    //    }
    //    thisNode->UpdateScale9GridShapeMatrix();
    //
    //    // TO DO: Process Scale9Grid 
    //}

    // TO DO: Handle Mesh_EdgeAA
   // unsigned meshGenFlags = (GetFlags() & NF_PartOfMask) ? Mesh_Mask : Mesh_EdgeAA;


    // Making decision. Check to see if it's necessary to rebuild TextMeshProvider.
    // Matrix M.GetMatrix2D/SetMatrix2D: final rendering matrix in screen space
    if (TMProvider.NeedsUpdate(M.GetMatrix2D()))
    {
        updateMesh = true;
    }
    // TO DO: Possibly other conditions

    if (updateMesh)
    {
        bundle->UpdateMesh(&SorterShapeNode);
        TMProvider.Clear();
    }
}


//------------------------------------------------------------------------
void TreeCacheText::HandleChanges(unsigned changeBits)
{
    if (changeBits & Change_TextLayout)
    {
        Bundle* bundle = GetBundle();
        if (bundle)
            bundle->UpdateMesh(&SorterShapeNode);

        TMProvider.Clear();
    }
}




void TreeCacheText::propagateMaskFlag(unsigned partOfMask)
{        
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) | partOfMask;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
      //  updateSortKey(); // Mask can change SortKey; add to update...
    }
}

TextMeshProvider* TreeCacheText::GetMeshProvider()
{
    return TMProvider.IsCreated() ? &TMProvider : 0;
}

TextMeshProvider* TreeCacheText::CreateMeshProvider() 
{ 
    SF_ASSERT(!TMProvider.IsCreated());

  //  GlyphCache* cache = pRenderer2D->GetGlyphCache();
  //  pProvider = *SF_HEAP_AUTO_NEW(this) TextMeshProvider(cache);

    const TreeText::NodeData* nodeData = GetNodeData();
    const TextLayout* layout = nodeData->pLayout;

    //SF_ASSERT(layout);
    if (layout)
    {
        unsigned meshGenFlags = (GetFlags() & NF_PartOfMask) ? Mesh_Mask : Mesh_EdgeAA;
        if (GetFlags() & NF_PartOfScale9)
            meshGenFlags |= Mesh_Scale9;
        TMProvider.CreateMeshData(layout, pRenderer2D, M, meshGenFlags);
    }
    // Create may have failed.
    return TMProvider.IsCreated() ? &TMProvider : 0;
}




}} // Scaleform::Render
