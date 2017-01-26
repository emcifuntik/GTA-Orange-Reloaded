/**********************************************************************

Filename    :   TreeMesh.cpp
Content     :   Defines TreeMesh - TreeNode that displayes triangles.
Created     :   December 19, 2009
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

#include "Render_TreeMesh.h"
#include "Render_TreeCacheNode.h"
#include "Render_PrimitiveBundle.h"

namespace Scaleform { namespace Render {


// Cache Node that provides a way of generating custom triangle meshes

class TreeCacheMesh : public TreeCacheMeshBase
{
public:
    Ptr<Mesh> pMesh;

    TreeCacheMesh(TreeMesh* pnode, const SortKey& key,
                  Renderer2DImpl* prenderer2D, unsigned flags)
        : TreeCacheMeshBase(pnode, key, prenderer2D, flags)
    {
    }
    ~TreeCacheMesh() { }
    
    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeMesh::NodeData NodeData;
    inline TreeMesh*       GetNode() const { return (TreeMesh*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }


    virtual void    UpdateTransform(const TreeNode::NodeData* data,
                                    const TransformArgs& t,
                                    TransformFlags flags)
    {
        RectF cullRect(t.CullRect);
        updateCulling(data, t, &cullRect, (TransformFlags) (flags|TF_CullCxform));
        SortParentBounds = data->AproxParentBounds;
        SetFlags(GetFlags()&~NF_ExpandedBounds);

        if (M.IsNull())
        {   // Don't allocate Cxform data initially if not used.
            unsigned matrixFormatBits = HMatrix::Has_2D;
            if (!(t.Cx == Cxform::Identity))
                matrixFormatBits |= HMatrix::Has_Cxform;
            M = GetMatrixPool().CreateMatrix(t.Mat, t.Cx, matrixFormatBits);
        }
        else
        {
            M.SetMatrix2D(t.Mat);
            M.SetCxform(t.Cx);
        }
    }

    // Obtains a mesh, creating it if necessary.
    virtual MeshBase* GetMesh()
    {
        SF_ASSERT(pRoot);

        if (!pMesh)
        {
            const NodeData* pdata = GetNodeData();
            MeshProvider*   pmeshProvider = pdata->pShape.GetPtr();
            pMesh = *SF_HEAP_AUTO_NEW(this) Mesh(pRenderer2D, pmeshProvider, Matrix2F());
        }
        return pMesh.GetPtr();
    }

    virtual void RemoveMesh()
    {
    }

};


bool TreeMesh::NodeData::PropagateUp(Context::Entry* entry) const
{
    RectF bounds, parentBounds;

    if (pShape)
        bounds = pShape->GetIdentityBounds();
    if (!bounds.IsEmpty())
        parentBounds = M2D().EncloseTransform(bounds);

    if ((bounds != AproxLocalBounds) || (parentBounds != AproxParentBounds))
    {
        TreeMesh* pm = (TreeMesh*)entry;
        NodeData* d = pm->GetWritableData(Change_AproxBounds);
        d->AproxLocalBounds = bounds;
        d->AproxParentBounds= parentBounds;
        return IsVisible(); // Only update parent for visible children.
    }
    return false;   
}

TreeCacheNode*  TreeMesh::NodeData::updateCache(TreeCacheNode* pparent,
                                                TreeCacheNode* pinsert,
                                                TreeNode* pnode, UInt16 depth) const
{
    TreeCacheMesh* pcache = (TreeCacheMesh*)pnode->GetRenderData();
    if (!pcache)
    {
        TreeMesh* mesh = (TreeMesh*)pnode;
        
        TextureManager*       mng = pparent->GetHAL()->GetTextureManager();
        PrimitiveFillManager& fillManager = pparent->GetPrimitiveFillManager();
        FillData              fillData;

        // TBD: Don't set Mesh_Mask flag yet since then we'd have to be switching
        // meshes based on mask state (an cache/re-generate them, etc).
        mesh->GetDisplayData()->pShape->GetFillData(&fillData, 0, 0, Mesh_EdgeAA);

        Ptr<Image> gradient(0);
        Ptr<PrimitiveFill> fill = *fillManager.CreateFill(fillData, &gradient, mng);
        // For now we don't save gradient here...
        SF_ASSERT(!gradient.GetPtr());
        
        unsigned cnodeFlags = (GetFlags() & (NF_Visible|NF_MaskNode)) |
                               NF_MaskNode_MapTo_PartOfMask(GetFlags());
        pcache = SF_HEAP_AUTO_NEW(pparent)
            TreeCacheMesh((TreeMesh*)pnode, SortKey(fill), pparent->pRenderer2D, cnodeFlags);
        pnode->SetRenderData(pcache);
    }
    
    pcache->UpdateInsertIntoParent(pparent, pinsert, this, depth);
    return pcache;
}

}} // Scaleform::Render
