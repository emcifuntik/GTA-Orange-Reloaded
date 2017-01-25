/**********************************************************************

Filename    :   Render_TreeShape.cpp
Content     :   TreeShape represents a Flash shape node in a tree.
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

#include "Render_TreeShape.h"
#include "Render_TreeCacheNode.h"

#include "Render_PrimitiveBundle.h" // For GetBundle inline
#include "Renderer2DImpl.h"

namespace Scaleform { namespace Render {

class TreeCacheShape;
class TreeCacheShapeLayer;


//------------------------------------------------------------------------
// ***** TreeCacheShape

// TreeCacheShape contains cached TreeShape data built out of
// child TreeCacheShapeLayer objects.
//   - It is only created if MeshProvider numLayers > 1;
//     otherwise TreeCacheShapeLayer is used directly for efficiency.

class TreeCacheShape : public TreeCacheContainer
{
public:
    TreeCacheShape(TreeShape* pnode, Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheShape();

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeShape::NodeData NodeData;
    inline TreeShape*      GetNode() const { return (TreeShape*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }

    inline ShapeMeshProvider* GetMeshProvider() const { return GetNodeData()->pMeshProvider; }

    // TreeCacheNode API.
    virtual void    HandleChanges(unsigned changeBits);
    //  virtual void    HandleRemoveNode();
    virtual void    UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth);

    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);
};


//------------------------------------------------------------------------
// ***** TreeCacheShapeLayer

// TreeCacheShapeLayer is a single Layer of a shape, which may be a shape,
// complex shape or a stroke.
//  - For LayerCount > 1, TreeCacheShapeLayers are child items under TreeCacheShape.
//  - For LayerCount == 1, TreeCacheShapeLayer lives directly in the rendering tree.

class TreeCacheShapeLayer : public TreeCacheMeshBase
{
friend class TreeCacheShape;
public:
    // Construct TreeCacheShapeLayer.
    //  pnode may be null if we are a child under 
    TreeCacheShapeLayer(TreeShape* pnode, const SortKey& key, unsigned drawLayer,
                        Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheShapeLayer();
    
    bool    IsComplexShape() const { return ComplexShape; }
    bool    HasParentShapeCache() const { return pNode == 0; }

    const TreeShape::NodeData* GetShapeNodeData() const
    { 
        TreeNode* node = HasParentShapeCache() ? GetParent()->pNode : pNode;
        return (const TreeShape::NodeData*)node->GetDisplayData();
    }

    ShapeMeshProvider* GetMeshProvider() const
    { 
        return GetShapeNodeData()->pMeshProvider;
    }

    inline float GetMorphRatio() const
    { 
        return GetShapeNodeData()->MorphRatio;
    }

    // Creates a new TreeCacheShapeLayer.
    //  - shapeNode may be null if we are creating leaf node under TreeCacheShape parent.
    static TreeCacheNode* Create(TreeCacheNode* pparent,
                                 ShapeMeshProvider* provider, unsigned drawLayer,
                                 unsigned flags, TreeShape* shapeNode = 0);

    static SortKey        CreateSortKey(TreeCacheNode* refNode,
                                        ShapeMeshProvider* provider, unsigned drawLayer,
                                        unsigned flags, Ptr<Image>* gradientImage);

    unsigned  calcMeshGenFlags() const
    {
        unsigned meshGenFlags = (GetFlags() & NF_PartOfMask) ? Mesh_Mask : Mesh_EdgeAA;
        if (GetFlags() & NF_PartOfScale9)
            meshGenFlags |= Mesh_Scale9;
        return meshGenFlags;
    }

    void            getShapeMatrixFrom3D(const TreeShape::NodeData* nd, Matrix2F* m) const;

    // Re-assigned SortKey; called due to flag change (Mask/NoMask).
    void            updateSortKey();

    void            UpdateCache(TreeCacheContainer* pparent, UInt16 depth);
    
    virtual void    HandleChanges(unsigned changeBits);
    virtual void    UpdateChildSubtree(const TreeNode::NodeData*, UInt16 depth);
    
    // For nested layer, these will receive matrix and node from TreeCacheShape.
    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);

    virtual void    propagateMaskFlag(unsigned partOfMask);
    virtual void    propagateScale9Flag(unsigned partOfScale9);

    // Assigns an inserts a mesh into a specified location.    
    virtual MeshBase* GetMesh();
    virtual void    RemoveMesh();

    inline Bundle* GetBundle() const
    {
        return SorterShapeNode.pBundle.GetPtr();
    }

private:

    unsigned   getLayer() const { return Layer; }

    // Computes mesh key value for matrix.
    unsigned   calcMeshKey(const ShapeMeshProvider* pmeshProvider, 
                           const Matrix& m, unsigned drawLayer, float* keyData,
                           Scale9GridData* s9g, float morphRatio) const;

    // Updates pMeshKey and its mesh to work for viewMatrix. New MeshKey is
    // created if it didn't exists already. Returns true if update took place.
    // Fills in *meshKeyFlags (mandatory).
    bool       updateMeshKey(Renderer2DImpl* r2D, ShapeMeshProvider* provider, float morphRatio,
                             const Matrix& viewMatrix, unsigned meshGenFlags, 
                             unsigned *meshKeyFlags = 0);

    Ptr<MeshKey>    pMeshKey;
    Ptr<Image>      pGradient;
    bool            ComplexShape;
    unsigned        Layer;
};



//------------------------------------------------------------------------
// ***** TreeShape::NodeData Implementation

bool TreeShape::NodeData::PropagateUp(Entry* entry) const
{        
    RectF    bounds, parentBounds;

    // TO DO: Force Update bounds when the shape changes!!!
    if (AproxLocalBounds.IsEmpty())
    {
        if (pMeshProvider)
        {
            // TO DO: Consider correct computation of stroke bounds that requires 
            // StrokeGenerator and ToleranceParam.
            bounds = pMeshProvider->GetIdentityBounds();
        }
    }
    else
    {
        bounds = AproxLocalBounds;
    }

    if (!bounds.IsEmpty())
        parentBounds = M2D().EncloseTransform(bounds);
    
    if ((bounds != AproxLocalBounds) || (parentBounds != bounds))
    {
        TreeShape* pc = (TreeShape*)entry;
        NodeData* d = pc->GetWritableData(Change_AproxBounds);
        d->AproxLocalBounds = bounds;
        d->AproxParentBounds = parentBounds;
        return IsVisible();
    }
    return false;   
}

TreeCacheNode* TreeShape::NodeData::updateCache(TreeCacheNode* pparent,
                                                TreeCacheNode* pinsert,
                                                TreeNode* pnode, UInt16 depth) const
{
    // Create & insert node if it doesn't already exist.
    TreeShape*          pshape = (TreeShape*)pnode;
    TreeCacheNode*      pcache = (TreeCacheNode*)pnode->GetRenderData();
    if (!pcache)
    {
        ShapeMeshProvider*  meshProvider = pshape->GetDisplayData()->pMeshProvider;
        unsigned            numLayers = meshProvider->GetLayerCount();
        
        // Pass NF_MaskNode|NF_PartOfMask to create proper SortKey; it is
        // also adjusted in Insert.
        unsigned cnodeFlags = (Flags & (NF_Visible|NF_MaskNode)) |
                              NF_MaskNode_MapTo_PartOfMask(Flags);
        cnodeFlags |= (pparent->GetFlags() & (NF_PartOfMask|NF_PartOfScale9));
        cnodeFlags |= (Flags & (NF_3D));
        if (GetState<Scale9State>())
            cnodeFlags |= NF_PartOfScale9;

        // For numLayers == 1, create TreeCacheShapeLayer directly.
        // If numLayers != 1, create TreeCacheShape with child layers.
        // numLayers == 0 is handled as an empty TreeCacheShape container.
        if (numLayers != 1)
            pcache = SF_HEAP_AUTO_NEW(pparent)
                     TreeCacheShape(pshape, pparent->pRenderer2D, cnodeFlags);
        else
            pcache = TreeCacheShapeLayer::Create(pparent, meshProvider, 0, cnodeFlags, pshape);
        
        if (!pcache) return 0;
        pnode->SetRenderData(pcache);
    }

    pcache->UpdateInsertIntoParent(pparent, pinsert, this, depth);    
    return pcache;
}



//------------------------------------------------------------------------
// ***** TreeCacheShape Implementation

TreeCacheShape::TreeCacheShape(TreeShape* pnode, Renderer2DImpl* prenderer2D, unsigned flags)
: TreeCacheContainer(pnode, prenderer2D, flags)
{
}

TreeCacheShape::~TreeCacheShape()
{        
    // Since cache Layers have no nodes, delete them explicitly.
    while (!Children.IsEmpty())
    {
        TreeCacheNode* p = Children.GetFirst();
        p->RemoveNode();
        p->pPrev = 0;
        p->SetParent(0);
        delete p;
    }
}

//------------------------------------------------------------------------
void TreeCacheShape::UpdateTransform(const TreeNode::NodeData* shapeData,
                                     const TransformArgs& t, TransformFlags flags)
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheShape::UpdateTransform");

    TransformArgs args(t.CullRect, t.Mat, t.Cx);
    if (flags & TF_Has3D)
        args.SetMatrix3D(t.GetMatrix3D(TF_Has3D));

    updateCulling(shapeData, t, &args.CullRect, (TransformFlags) (flags | TF_CullCxform));

    SortParentBounds = shapeData->AproxParentBounds;
    SetFlags(GetFlags()&~NF_ExpandedBounds);

    // Child shape layers don't need individual culling because group
    // culling was just done above.
    flags = (TransformFlags) (flags & ~TF_NeedCull);

    TreeCacheNode* pnode = Children.GetFirst();
    while (!Children.IsNull(pnode))
    {
        // Avoid virtual call since we know child node type.
        TreeCacheShapeLayer* p = (TreeCacheShapeLayer*)pnode;
        p->TreeCacheShapeLayer::UpdateTransform(shapeData, args, flags);
        // TreeCacheShapeLayer doesn't have NeedUpdate flag.
        pnode = pnode->pNext;
    }
}


//------------------------------------------------------------------------
void TreeCacheShape::HandleChanges(unsigned changeBits)
{
    if ((changeBits & Change_MorphRatio) && pRoot)
    {
        // Force a call to UpdateTransform, which will detect ratio change in updateMesh.
        pRoot->AddToUpdate(this, Change_Matrix);
    }

    // TO DO: Recreate Layers due to SetShape()

    // Since the structure of the shape can change (1 layer vs. more layers),
    // this object may need to be destroyed & re-created.
    // Potentially implement as:
    //   - Remove from parent.
    //   - Update parent to insert new item here.
    //   - Delete this (and return immediately).
    //  Renderer2DImpl::EntryChanges would need updating to handle deleted pcache;
    //  perhaps this function can return a replacement? TBD.

    // TreeCacheShapeLayer would need a similar implementation as well.
}


//------------------------------------------------------------------------
void TreeCacheShape::UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth)
{
    TreeCacheNode::UpdateChildSubtree(pdata, depth);

    // Insertion pointer for new nodes as we go.
    ShapeMeshProvider* meshProvider = static_cast<const NodeData*>(pdata)->pMeshProvider;
    unsigned layerCount = meshProvider->GetLayerCount();
    unsigned drawLayer  = 0;

    if (Children.IsEmpty())
    {
        TreeCacheNode*  pnewInsert = Children.GetFirst()->pPrev;
    
        for(drawLayer = 0; drawLayer < layerCount; ++drawLayer)
        {
            TreeCacheNode* cache =
                TreeCacheShapeLayer::Create(this, meshProvider, drawLayer, NF_Visible);
            if (cache)
            {                
                cache->UpdateInsertIntoParent(this, pnewInsert, 0, depth);
                pnewInsert = cache;
            }
        }
    }
    else
    {
        TreeCacheNode*  p = Children.GetFirst();
        while(!Children.IsNull(p))
        {
            ((TreeCacheShapeLayer*)p)->UpdateCache(this, depth);
            p = p->pNext;
        }
    }
}


//------------------------------------------------------------------------
// ***** TreeCacheShapeLayer implementation

TreeCacheShapeLayer::TreeCacheShapeLayer(TreeShape* node, const SortKey& key, unsigned drawLayer,
                                         Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheMeshBase(node, key, prenderer2D, flags),
    ComplexShape(key.GetType() == SortKey_MeshProvider), Layer(drawLayer)
{
}

TreeCacheShapeLayer::~TreeCacheShapeLayer()
{
}


//------------------------------------------------------------------------
TreeCacheNode* TreeCacheShapeLayer::Create(TreeCacheNode* pparent,
                                           ShapeMeshProvider* provider, unsigned drawLayer,
                                           unsigned flags, TreeShape* shapeNode)
{   
    Ptr<Image> gradient(0);
    SortKey    key(CreateSortKey(pparent, provider, drawLayer, flags, &gradient));
    
    TreeCacheShapeLayer* pcache = 
        SF_HEAP_AUTO_NEW(pparent)
        TreeCacheShapeLayer(shapeNode, key, drawLayer, pparent->pRenderer2D, flags);
    if (!pcache)
        return 0;

    pcache->pGradient = gradient;
    return pcache;
}


SortKey TreeCacheShapeLayer::CreateSortKey(TreeCacheNode* refNode,
                                           ShapeMeshProvider* provider, unsigned drawLayer,
                                           unsigned flags, Ptr<Image>* gradientImage)
{
    unsigned meshGenFlags = Mesh_EdgeAA;
    if (flags & NF_PartOfMask)
        meshGenFlags = Mesh_Mask; // Mask replaces EdgeAA
    if (flags & NF_PartOfScale9)
        meshGenFlags |= Mesh_Scale9;

    if (provider->GetFillCount(drawLayer, meshGenFlags) > 1)
    {
        // ComplexMesh key, based on MeshProvider.
        return SortKey(provider);
    }

    // Simple Mesh key, based on PrimitiveFill.
    TextureManager*       mng = refNode->GetHAL()->GetTextureManager();
    PrimitiveFillManager& fillManager = refNode->GetPrimitiveFillManager();
    FillData   fillData;    
    provider->GetFillData(&fillData, drawLayer, 0, meshGenFlags);
    Ptr<PrimitiveFill> fill = *fillManager.CreateFill(fillData, gradientImage, mng);
    return SortKey(fill.GetPtr());
}


void TreeCacheShapeLayer::updateSortKey()
{
    // Sort key changes in response to Flags; currently to enable/disable mask.
    SorterShapeNode.ClearBundle();
    SorterShapeNode.Key = CreateSortKey(this, GetMeshProvider(), getLayer(),
                                        GetFlags(), &pGradient);
    ComplexShape = (SorterShapeNode.Key.GetType() == SortKey_MeshProvider);
    
    pMeshKey.Clear();

    if (pRoot && GetParent())
        pRoot->AddToUpdate(GetParent(), Update_Pattern|Change_Matrix);    
}


void TreeCacheShapeLayer::UpdateCache(TreeCacheContainer* pparent, UInt16 depth)
{  
    SF_ASSERT(pParent == pparent);
    Depth = depth;
    pRoot = pparent->pRoot;
}

void TreeCacheShapeLayer::HandleChanges(unsigned changeBits)
{
    if ((changeBits & Change_MorphRatio) && pRoot)
    {
        // Force a call to UpdateTransform, which will detect ratio change in updateMesh.
        pRoot->AddToUpdate(this, Change_Matrix);
    }
}

void TreeCacheShapeLayer::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{
    // We override UpdateChildSubtree so that sortKey can be updated if necessary.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9)) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());    
    if (data && data->GetState<Scale9State>())
        newFlags |= NF_PartOfScale9;
    if (data && data->Is3D())
        newFlags |= NF_3D;

    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        updateSortKey();
    }
    if (data)
        updateMaskCache(data, depth);
}


//------------------------------------------------------------------------
static unsigned GetMatrixFlip(const Matrix2F& m)
{
    unsigned flags = 0;
    float x1 = 1;
    float y1 = 0;
    float x2 = 0;
    float y2 = 1;
    m.Transform2x2(&x1, &y1);
    m.Transform2x2(&x2, &y2);
    if (Math2D::CrossProduct(0, 0, x2, y2, x1, y1) <  0) flags |= MeshKey::KF_FlipX;
    if (Math2D::CrossProduct(0, 0, x1, y1, x2, y2) >= 0) flags |= MeshKey::KF_FlipY;
//printf("%f %d%d\n", m.Sx(), (flags & MeshKey::KF_FlipX) != 0, (flags & MeshKey::KF_FlipY) != 0);
    return flags;
}


// Returns mesh key flags.
//------------------------------------------------------------------------
unsigned TreeCacheShapeLayer::calcMeshKey(const ShapeMeshProvider* pmeshProvider, 
                                          const Matrix& m, unsigned drawLayer, float* keyData, 
                                          Scale9GridData* s9g, float morphRatio) const
{
    unsigned flags = MeshKey::KF_Fill;

    // TO DO: Try to optimize, calculate it in TreeCacheShape.
    if (GetFlags() & NF_PartOfScale9)
    {
        const TreeCacheNode* thisNode = this;
        if (HasParentShapeCache())
            thisNode = GetParent();

        Matrix2F shapeMtx;
        while(thisNode)
        {
            const NodeData* nd = thisNode->GetNodeData();
            const Scale9State* s9 = nd->GetState<Scale9State>();
            if (s9)
            {
                s9g->S9Rect    = s9->GetRect();
                s9g->Bounds    = nd->AproxLocalBounds;
                s9g->ShapeMtx  = shapeMtx;
                s9g->Scale9Mtx = nd->M2D();
                s9g->ViewMtx   = m;
                flags |= MeshKey::KF_Scale9Grid;
                break;
            }
            shapeMtx.Append(nd->M2D());
            thisNode = thisNode->GetParent();
        }
    }

    bool validKey = true;

    if (flags & MeshKey::KF_Scale9Grid)
    {
        s9g->MakeMeshKey(keyData);
    }
    else
    {
        validKey = MeshKey::CalcMatrixKey(m, keyData, 0);
        unsigned strokeStyle = pmeshProvider->GetLayerStroke(drawLayer);
        if (strokeStyle)
        {
            flags = MeshKey::KF_Stroke;

            StrokeStyleType ss;
            pmeshProvider->GetStrokeStyle(strokeStyle, &ss, 0);
            if (ss.Flags & StrokeFlag_StrokeHinting)
            {
                flags = MeshKey::KF_StrokeHinted;
            }
        }
    }

    if (!validKey) 
        flags |= MeshKey::KF_Degenerate;

    flags |= GetMatrixFlip(m);

    keyData[MeshKey::GetKeySize(flags) - 1] = morphRatio;
    return flags;
}

// Updates pMeshKey and its mesh to work for viewMatrix. New MeshKey is
// created if it didn't exists already. Returns true if update took place.
//------------------------------------------------------------------------
bool TreeCacheShapeLayer::updateMeshKey(Renderer2DImpl* r2D, ShapeMeshProvider* provider,
                                        float morphRatio, const Matrix& viewMatrix, 
                                        unsigned meshGenFlags, unsigned *meshKeyFlags)
{
    float keyData[MeshKey::MaxKeySize];

    const ToleranceParams& cfg = r2D->GetToleranceParams();
    unsigned drawLayer = getLayer();
    unsigned f2 = 0;
    if (meshGenFlags & Mesh_EdgeAA) f2 |= MeshKey::KF_EdgeAA;
    if (meshGenFlags & Mesh_Mask)   f2 |= MeshKey::KF_Mask;    

    Scale9GridData s9g;
//s9g.S9Rect   = RectF();  // DBG
//s9g.Bounds   = RectF();
//s9g.ShapeMtx = Matrix2F();
//s9g.Scale9Mtx= Matrix2F();
//s9g.ViewMtx  = Matrix2F();

    unsigned flags = calcMeshKey(provider, viewMatrix, drawLayer, keyData, &s9g, morphRatio) | f2;
    if (meshKeyFlags)
        *meshKeyFlags = flags;
    
    MeshKeyManager* mkmanager = r2D->GetMeshKeyManager();
    MeshKey*        newKey;


    if (pMeshKey)
    {
        if (pMeshKey->Match(drawLayer, flags, keyData, cfg))
            return false;
        newKey = mkmanager->CreateMatchingKey(pMeshKey->GetKeySet(),
                                              drawLayer, flags, keyData, cfg);
    }
    else
    {
        newKey = mkmanager->CreateMatchingKey(provider, drawLayer, flags, keyData, cfg);
    }

    if (!newKey)
        return false;
    SF_ASSERT(newKey != pMeshKey);

    if (!newKey->pMesh)
    {
        Ptr<MeshBase> pmesh;
        if (ComplexShape)
        {
            newKey->pMesh = *SF_HEAP_AUTO_NEW(this)
                ComplexMesh(r2D, newKey->GetKeySet(), &r2D->GetPrimitiveFillManager(),
                            viewMatrix, morphRatio, drawLayer, meshGenFlags);
        } // ...don't remove curlies
        else
        {
            newKey->pMesh = *SF_HEAP_AUTO_NEW(this)
                Mesh(r2D, newKey->GetKeySet(), viewMatrix, morphRatio, drawLayer, meshGenFlags);
        }

        if (!newKey->pMesh)
        {
            newKey->Release();
            return false;
        }

        if (flags & MeshKey::KF_Scale9Grid)
        {
            Ptr<Scale9GridData> p = *SF_HEAP_AUTO_NEW(this) Scale9GridData(s9g);
            newKey->pMesh->SetScale9Grid(p);
        }
    }

    pMeshKey = *newKey;
    return true;
}


static float scaleX(int left, int width, float x)
{
    return left + width * (x+1) / 2.f;
}
static float scaleY(int top, int height, float y)
{
    return top + height * (-y+1) / 2.f;
}



//------------------------------------------------------------------------
// Compute 2D matrix for tessellation
void TreeCacheShapeLayer::getShapeMatrixFrom3D(const TreeShape::NodeData* nd, Matrix2F* mat) const
{
    SF_ASSERT(M.Has3D());
    RectF bounds = nd->pMeshProvider->GetIdentityBounds();

    const TreeNode::NodeData* pRootData = pRoot->GetNodeData();
    const ProjectionMatrix3DState* projState = pRootData->GetState<ProjectionMatrix3DState>();
    SF_ASSERT(projState);

    const ViewMatrix3DState* viewState = pRootData->GetState<ViewMatrix3DState>();
    SF_ASSERT(viewState);

    Matrix4F viewproj = Matrix4F(projState->GetProjectionMatrix3D()->M) * Matrix4F(viewState->GetViewMatrix3D()->M);

    Matrix4F m = viewproj * M.GetMatrix3D();
    const Viewport & vp = pRoot->GetNodeData()->VP;

    Point3F p1 = m.TransformHomogeneous(Point3F(bounds.x1, bounds.y1, 0));
    Point3F p2 = m.TransformHomogeneous(Point3F(bounds.x2, bounds.y1, 0));
    Point3F p3 = m.TransformHomogeneous(Point3F(bounds.x2, bounds.y2, 0));

    p1.x = scaleX(vp.Left, vp.Width,  p1.x);
    p1.y = scaleY(vp.Top,  vp.Height, p1.y);
    p2.x = scaleX(vp.Left, vp.Width,  p2.x);
    p2.y = scaleY(vp.Top,  vp.Height, p2.y);
    p3.x = scaleX(vp.Left, vp.Width,  p3.x);
    p3.y = scaleY(vp.Top,  vp.Height, p3.y);

    float coord[6] = { p1.x, p1.y, p2.x, p2.y, p3.x, p3.y };
    mat->SetRectToParl(bounds.x1, bounds.y1, bounds.x2, bounds.y2, coord);

    if (fabsf(mat->GetDeterminant()) < 0.001f) // TO DO: Move constants to ToleranceParams
       *mat = Matrix2F::Scaling(mat->GetScale());

    unsigned strokeStyle = nd->pMeshProvider->GetLayerStroke(getLayer());
    if (strokeStyle)
    {
        float s = mat->GetScale();
        if (s < 0.005f) // TO DO: Move constants to ToleranceParams
            s = 0.005f;
        *mat = Matrix2F::Scaling(s);
    }


    // Test version 1
    //Matrix2F mat = (flags & TF_Has3D) ? Matrix2F(t.Mat3D) : t.Mat;	// Note: 3D -> 2D matrix conversion 
    //float sx = mat.GetXScale();
    //float sy = mat.GetYScale();
    //if (sx < 0.001f || sy < 0.001f)
    //{
    //    if (sx < 0.001f) sx = 0.001f;
    //    if (sy < 0.001f) sy = 0.001f;
    //    mat = Matrix2F::Scaling(sx, sy);
    //}

    // Test version 2
    //Matrix2F mat = t.Mat;
    //if (flags & TF_Has3D) 
    //{
    //    float m00 = t.Mat3D.M[0][0];
    //    float m01 = t.Mat3D.M[0][1];
    //    float m10 = t.Mat3D.M[1][0];
    //    float m11 = t.Mat3D.M[1][1];
    //    float sx = sqrtf(m00*m00 + m10*m10);
    //    float sy = sqrtf(m11*m11 + m01*m01);
    //    if (sx < 0.01f) 
    //        sx = 0.01f;
    //    if (sy < 0.01f) 
    //        sy = 0.01f;
    //    mat = Matrix2F::Scaling(sx, sy);
    //}

    // Test version 3 (uniform scaling)
    //Matrix2F mat = t.Mat;
    //if (flags & TF_Has3D) 
    //{
    //    float sx = t.Mat3D.M[0][0] + t.Mat3D.M[0][1];
    //    float sy = t.Mat3D.M[1][0] + t.Mat3D.M[1][1];
    //    mat = Matrix2F::Scaling(sqrtf(sx*sx + sy*sy) * 0.707106781f);
    //}
}



//------------------------------------------------------------------------
// Updates computed final matrix
void TreeCacheShapeLayer::UpdateTransform(const TreeNode::NodeData* nodeData,
                                          const TransformArgs& t,
                                          TransformFlags flags)
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheShapeLayer::UpdateTransform");

    RectF cullRect(t.CullRect);
    updateCulling(nodeData, t, &cullRect, (TransformFlags) (flags | TF_CullCxform));
    SortParentBounds = nodeData->AproxParentBounds;
    SetFlags(GetFlags() & ~NF_ExpandedBounds );

    // updates computed final HMatrix
    ComputeFinalMatrix(t, flags);

    // If we were updating Cxform only, skip the rest of the logic, which
    // depends only on matrix. (TBD: Handle Cxform alpha change).
    if (!(flags & TF_Matrix))
        return;

    const TreeShape::NodeData* shapeNodeData = (const TreeShape::NodeData*)nodeData;    
    Bundle* bundle = GetBundle();
    if (!bundle)
        return;
    SF_ASSERT(pMeshKey);

    unsigned meshKeyFlags = 0;
    bool     updateMesh = false;


    Matrix2F mat(Matrix2F::NoInit);
    if (flags & TF_Has3D)
        getShapeMatrixFrom3D(shapeNodeData, &mat);
    else
        mat = t.Mat;

    if (updateMeshKey(pRenderer2D, shapeNodeData->pMeshProvider, shapeNodeData->MorphRatio,
        mat, calcMeshGenFlags(), &meshKeyFlags))
    {
//        SF_ASSERT(pRoot);        
        updateMesh = true;

        if (shapeNodeData->MorphRatio != 0)
        {
            // TO DO: Revise GetFillMatrix for morphing.
            if (!ComplexShape && SorterShapeNode.pBundle)
            {            
                PrimitiveFill* fill = 
                    ((PrimitiveBundle*)SorterShapeNode.pBundle.GetPtr())->Prim.GetFill();

                Texture* texture = fill->GetTexture(0);
                if (texture)
                {
                    Matrix tm;
                    GetMeshProvider()->GetFillMatrix(pMeshKey->pMesh, &tm, getLayer(), 0, calcMeshGenFlags());

                    SF_ASSERT(!M.IsNull());
                    //if (M.IsNull())
                    //    M = GetMatrixPool().CreateMatrix(m);
                    M.SetTextureMatrix(tm, 0);
                }
            }
        }
    }
    // TBD: This adjustment should really be done before (!bundle) return
    //      above, but before updateMeshKey. Else its not applied in first frame...
    //      Currently updateMeshKey isn't called immediately to avoid allocation
    //      for hidden objects.
    // TO DO: revise, consider offset in the MeshKey
    if (M.Has3D() == 0 && (meshKeyFlags & MeshKey::KF_KeyTypeMask) == MeshKey::KF_StrokeHinted)
    {
        Matrix m2 = t.Mat;
        m2.Tx() = floorf(m2.Tx() + ((m2.Tx() < 0) ? -0.5f : +0.5f));
        m2.Ty() = floorf(m2.Ty() + ((m2.Ty() < 0) ? -0.5f : +0.5f));
        M.SetMatrix2D(m2);
    }

    if (updateMesh)
        bundle->UpdateMesh(&SorterShapeNode);
}


// Returns an a mesh, typically before insertion into a specified location by Bundle.
//------------------------------------------------------------------------
MeshBase* TreeCacheShapeLayer::GetMesh()
{    
//    SF_ASSERT(pRoot);
    if (!pMeshKey || !pMeshKey->pMesh)
    {
        Matrix2F mat(Matrix2F::NoInit);
        unsigned meshGenFlags = calcMeshGenFlags();

        SF_ASSERT(!M.IsNull());

        if (M.Has3D())
            getShapeMatrixFrom3D(GetShapeNodeData(), &mat);
        else
            mat = M.GetMatrix2D();

        //(HasParentShapeCache() ? GetParent() : (TreeCacheNode*)this)->CalcViewMatrix(&m);
        updateMeshKey(pRenderer2D, GetMeshProvider(), GetMorphRatio(), mat, meshGenFlags);

        // Simple shapes only!
        if (!ComplexShape && SorterShapeNode.pBundle)
        {            
            PrimitiveFill* fill = 
                ((PrimitiveBundle*)SorterShapeNode.pBundle.GetPtr())->Prim.GetFill();

            Texture* texture = fill->GetTexture(0);
            if (texture)
            {
                Matrix tm;
                GetMeshProvider()->GetFillMatrix(pMeshKey->pMesh, &tm, getLayer(), 0, meshGenFlags);

                //if (M.IsNull())
                //    M = GetMatrixPool().CreateMatrix(mat);
                M.SetTextureMatrix(tm, 0);
            }
        }
    }

    return pMeshKey ? pMeshKey->pMesh.GetPtr() : 0;
}

//------------------------------------------------------------------------
void TreeCacheShapeLayer::RemoveMesh()
{    
    pMeshKey = 0;
    // Removal of mesh from primitive can be done by group externally.
}


void TreeCacheShapeLayer::propagateMaskFlag(unsigned partOfMask)
{        
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) | partOfMask;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        updateSortKey(); // Mask can change SortKey; add to update...
    }
}

void TreeCacheShapeLayer::propagateScale9Flag(unsigned partOfScale9)
{    
    if (GetNode() && GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    unsigned newFlags = (GetFlags() & ~NF_PartOfScale9) | partOfScale9;
    if (newFlags != GetFlags())
    {
        SetFlags(newFlags);
        // Scale9 can change SortKey, since its passed to GetFillData()
        updateSortKey(); 
    }
}


// 1. Check UpdateLayerMatrix    - DONE
// 2. Check InsertMesh           - DONE
// 3. Fix handling pNode == 0    - DONE
// 4. Implement HandleChanges()


}} // Scaleform::Render
