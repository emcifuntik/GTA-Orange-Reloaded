/**********************************************************************

Filename    :   TreeCacheNode.cpp
Content     :   Cached version of tree node classes
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

#include "Render/Render_TreeCacheNode.h"
#include "Render/Renderer2DImpl.h"
#include "Render/Render_SIMD.h"

#include "Kernel/SF_HeapNew.h"

bool Check_Address = false;
unsigned BundlePatternFrameId = 0;
unsigned BuildPatternCount = 0;

namespace SFR = Scaleform::Render;

// For crash debugging
volatile const SFR::TreeCacheNode  *TCN_This = 0;
volatile const SFR::TreeNode::NodeData *TCN_ThisData = 0;
volatile const SFR::TreeNode* TCN_ChildNode = 0;
volatile const SFR::TreeCacheNode *TCN_Child = 0;
volatile const SFR::TreeCacheNode *TCN_Parent = 0;
volatile Scaleform::UPInt TCN_Index = 0;



namespace Scaleform { namespace Render {

// ***** TreeCacheNode

TreeCacheNode::TreeCacheNode(TreeNode* pnode, Renderer2DImpl* prenderer2D, unsigned flags)
 : pRoot(0),  pNode(pnode), pRenderer2D(prenderer2D),
   pParent(0), pMask(0),
   Depth(0), Flags((UInt16)flags), UpdateFlags(Change_State_Effect_Mask),
   pNextUpdate(0)
{   
    SF_ASSERT((flags & ~NF_CacheNode_Mask) == 0);
    // Clear nodes to mark that Parent is not a container.
    pPrev = pNext = 0;
}

TreeCacheNode::~TreeCacheNode()
{
    // Destroyed nodes need to remove themselves from parent,
    // in case HandleChanges is called on it.    

    if (pPrev)
        RemoveNode();
    else if (pParent)
    {
        SF_ASSERT(pParent->GetMask() == this);
        pParent->ClearMask();
    }
    
    if (pMask)
    {
        // We may still have pMask if it was moved a new invisible subtree,
        // for example. Don't leave it dangling.
        SF_ASSERT(pMask->pParent == this);
        pMask->RemoveFromParent();
    }
}

// Helper Renderer2DImpl accessor to avoid including that header
HAL* TreeCacheNode::GetHAL() const
{
    return pRenderer2D->GetHAL();
}
PrimitiveFillManager& TreeCacheNode::GetPrimitiveFillManager() const
{
    return pRenderer2D->GetPrimitiveFillManager();
}
MatrixPool& TreeCacheNode::GetMatrixPool() const
{
    return pRenderer2D->GetMatrixPool();
}


void TreeCacheNode::UpdateInsertIntoParent(TreeCacheNode* pparent,
                                           TreeCacheNode* pinsert,
                                           const TreeNode::NodeData* nodeData,
                                           UInt16 depth)
{
    TreeCacheNode* oldParent = GetParent();
    if ((oldParent != pparent) ||
        (pinsert && pparent->GetMask() == this) || // Adding to container what was its mask earlier.
        (!pinsert && pparent->GetMask() != this))  // Setting as mask what was a child earlier.
    {
        if (oldParent)
        {
            RemoveFromParent();
            if (oldParent->pRoot && oldParent->IsPatternChainValid())
                oldParent->pRoot->AddToUpdate(oldParent, Update_Pattern);
        }

        // Insert into us.
        if (pinsert)
            pinsert->InsertNodeAfter(this);
        // nodeData can be null for Shape Layer.
        if (nodeData)
        {
            if (nodeData->IsMaskNode())
            {
                // If parent had a different mask, remove it.
                if (pparent->GetMask() && (pparent->GetMask() != this))
                    pparent->GetMask()->RemoveFromParent();
                SF_ASSERT(pinsert == 0);
                SF_ASSERT(pparent->GetNodeData()->GetState<MaskNodeState>()
                                 ->GetNode()->GetRenderData() == this);
                pparent->SetMask(this);
                SetFlags(GetFlags()|NF_MaskNode);
                propagateMaskFlag(NF_PartOfMask);
            }
            if (nodeData->GetState<Scale9State>())
                propagateScale9Flag(NF_PartOfScale9);
        }
        SetParent(pparent);
    }

    // Verify pParent insertion and Depth.
    // - Depth is needed even if there is no pRoot since is allows us to
    //   propagate bounds, keeping them properly cached.
    Depth = depth;
    pRoot = pparent->pRoot;

    // Call UpdateChildSubtree to build child layers.
    // This is a virtual function that will do nothing for leaf nodes.
    UpdateChildSubtree(nodeData, depth + 1);
}

// Removed node from a cached parent child list, also updating the Primitive.
// Used by HandleChanges.
void TreeCacheNode::RemoveFromParent()
{
    if (pPrev)
    {
        SF_ASSERT(pParent);
        RemoveNode();
    }
    else if (pParent)
    {
        SF_ASSERT(pParent->GetMask() == this);
        pParent->ClearMask();
        SetFlags(GetFlags() & ~NF_MaskNode);
    }
    // Clear pPrev, pNext to mark no parent container.
    pNext   = pPrev = 0;
    pParent = 0;
    Depth   = 0;

    // Clear NF_PartOfMask flag in subtree.
    if (GetFlags() & NF_PartOfMask)
        propagateMaskFlag(0);
    if (GetFlags() & NF_PartOfScale9)
        propagateScale9Flag(0); // propagate will check flag presence.
    
    // This nulls out pRoot/CachedChildPattern in us and subtree, which effectively
    // disables matrix, cxform and pattern updates.
    HandleRemoveNode();
}

// Removes this node by clearing out pRoot in the subtree.
void TreeCacheNode::HandleRemoveNode()
{
    pRoot = 0;
    if (HasMask())
        GetMask()->HandleRemoveNode();
}


void TreeCacheNode::UpdateChildSubtree(const TreeNode::NodeData* data, UInt16 depth)
{   
    // NF_PartOfMask flag update is moved into UpdateChildSubtree because it
    // includes custom change-detection logic in TreeCacheShapeLayer.
    unsigned newFlags = (GetFlags() & ~NF_PartOfMask) |
                        NF_MaskNode_MapTo_PartOfMask(GetFlags());
    if (data->GetState<Scale9State>())
        newFlags |= NF_PartOfScale9;
    if (GetParent())
        newFlags |= (GetParent()->GetFlags() & (NF_PartOfMask|NF_PartOfScale9));
    if (data->Is3D())
        newFlags |= NF_3D;
    SetFlags(newFlags);

    updateMaskCache(data, depth);
}

void TreeCacheNode::updateMaskCache(const TreeNode::NodeData* data, UInt16 depth,
                                    bool updateSubtree)
{
    TCN_This = this;
    TCN_ThisData = data;

    // Mask can be added, removed, or replaced.
    if (data->HasMask())
    {
        TreeNode* maskNode = data->GetState<MaskNodeState>()->GetNode();

        TCN_ChildNode = maskNode;
        
        if (updateSubtree)
        {
            // Regular update for pRoot, Depth, etc called from UpdateChildSubtree
            maskNode->GetDisplayData()->updateCache(this, 0, maskNode, depth);
        }
        else
        {
            // Mask state-change triggered update.
            TreeCacheNode* newMask = (TreeCacheNode*)maskNode->GetRenderData();

            if ((newMask != GetMask()) || !GetMask())
            {   
                // Update should remove non-matching pMask in this.
                newMask = maskNode->GetDisplayData()->updateCache(this, 0, maskNode, depth);
                SF_UNUSED(newMask);
                SF_ASSERT((GetMask() == newMask) && (GetFlags()&NF_HasMask));
                SF_ASSERT(newMask->GetFlags() & NF_MaskNode);
                goto update_on_mask_change;
            }
        }
    }
    else if (HasMask())
    {   // Mask removed.
        GetMask()->RemoveFromParent();
        SF_ASSERT(!GetMask());

    update_on_mask_change:
        if (pRoot && IsPatternChainValid())
            pRoot->AddToUpdate(this, Update_Pattern);
    }
}

void TreeCacheNode::updateMaskTransform(TransformArgs& t,
                                        TransformFlags flags)
{
    if (HasMask())
    {
        const TreeNode::NodeData* data = GetMask()->GetNodeData();
        if ( !(flags & TF_Has3D) )
            t.Mat.Prepend(data->M2D());
        else
            t.PrependMatrix3D(data->M3D());
        GetMask()->UpdateTransform(data, t, flags);
    }
}

void TreeCacheNode::updateEffectChain(BundleEntryRange* contentChain)
{
    if (HasMask())
    {
        BundleEntryRange maskPattern(Range_Empty);
        GetMask()->GetPatternChain(&maskPattern);
        Effects.UpdateBundleChain(contentChain, &maskPattern);
    }
    else
    {
        Effects.UpdateBundleChain(contentChain, 0);
    }
}


void TreeCacheNode::propagateMaskFlag(unsigned partOfMask)
{    
    ApplyFlags(NF_PartOfMask, partOfMask);
}
void TreeCacheNode::propagateScale9Flag(unsigned partOfScale9)
{    
    if (GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    ApplyFlags(NF_PartOfScale9, partOfScale9);
}


// Updates culling for this class and returns new needCull flag.
TransformFlags TreeCacheNode::updateCulling(
    const TreeNode::NodeData* data, const TransformArgs& t,
    RectF* cullRect, TransformFlags flags)
{
    SF_ASSERT(t.CullRect == *cullRect);
    bool culled = false;
    if (flags & TF_NeedCull || flags & TF_Has3D )
    {
        RectF bounds;

        // Update Mask Matrix (in effect) + do mask culling.
        if (HasMask())
        {
            Matrix2F        maskAreaMatrix;
            MaskEffect*     maskEffect = Effects.GetMaskEffect();
            MaskEffectState mes;

            // Update MaskEffect state, including its matrix.
            if (maskEffect)
            {
                mes = calcMaskBounds(&bounds, &maskAreaMatrix, t.GetMatrix3D(flags),                    
                    maskEffect->GetEffectState());
                if (maskEffect->UpdateMatrix(mes, maskAreaMatrix))
                {
                    UpdateFlags |= Change_State_MaskNode;
                    addParentToDepthPatternUpdate();
                }
            }
            else
            {
                mes = calcMaskBounds(&bounds, &maskAreaMatrix, t.Mat, MES_NoMask);
            }

            // Adjust cullRect.
            if (mes != MES_NoMask)
            {
                if (mes == MES_Culled)
                    culled = true;
                else
                {   // If view cullRect became empty, we are culled.
                    t.Mat.EncloseTransform(&bounds, bounds);
                    if (!cullRect->IntersectRect(cullRect, bounds))
                        culled = true;
                }
            }
        }

        if ((flags & TF_CullCxform) && !culled)
        {
            // Cull object if its Alpha is perceivably at 0.
            // This is only done for leaf shapes, i.e. the ones that have TF_CullCxform
            // bit passed in an argument. We can't do this for containers since child
            // cxforms can be increased again through additive channel.
            if ((fabs(t.Cx.M[3][0]) < 0.001f) && (fabs(t.Cx.M[3][1]) < (1.0f/255.0f)) &&
                !(GetFlags() & NF_PartOfMask))
                culled = true;
        }

        if (!culled)
        {
            t.Mat.EncloseTransform(&bounds, data->AproxLocalBounds);
            culled = !cullRect->Intersects(bounds);
        }
    }

    if (culled && !(flags & TF_Has3D))
    {
        if (!(GetFlags() & NF_Culled))
        {
            SetFlags(GetFlags()|NF_Culled);
            addParentToDepthPatternUpdate();
        }

        // Remove NeedCull flag for children, since the container was culled.
        flags = (TransformFlags)(flags & ~TF_NeedCull);
    }
    else
    {
        if (GetFlags() & NF_Culled)
        {
            SetFlags(GetFlags()&~NF_Culled);
            addParentToDepthPatternUpdate();
        }
    }

    if (HasMask())
    {
        if ( !(flags & TF_Has3D) )
        {
            TransformArgs args(*cullRect, t.Mat, t.Cx);
            updateMaskTransform(args, flags);
        }
        else
        {
            TransformArgs args(*cullRect, t.GetMatrix3D(flags), t.Cx);
            args.Mat = Matrix2F::Identity;
            updateMaskTransform(args, flags);
        }
    }

    return flags;
}

static float scaleX(int left, int width, float x)
{
    return left + width * (x+1) / 2.f;
}
static float scaleY(int top, int height, float y)
{
    return top + height * (-y+1) / 2.f;
}


// Calculates bounds of the mask in local coordinates. Return false if
// no adjustment is necessary.
bool TreeCacheNode::calcChildMaskBounds(RectF *bounds, TreeCacheNode* child)
{
    SF_ASSERT(child->HasMask() && (child->pParent == this));
               
    TreeCacheNode*            maskNodeCache= child->pMask;
    const TreeNode::NodeData* maskNodeData = maskNodeCache->GetNodeData();    
    if (!maskNodeData->IsVisible() || maskNodeData->AproxParentBounds.IsEmpty())
        return false;

    // Figure out if there is there any 3D in the tree (in which case we need to project the bounds into clip space).
    bool has3d = maskNodeData->Is3D();
    TreeCacheNode * pparent = this;
    Matrix4F viewproj;
    while ( pparent && !has3d )
    {
        has3d |= pparent->GetNodeData()->Is3D();
        pparent = pparent->GetParent();
    }

    if ( has3d )
    {
        SF_ASSERT( pRoot->GetNodeData()->HasViewport() );
        const Viewport & vp = pRoot->GetNodeData()->VP;
        bounds->x1 = scaleX(vp.Left, vp.Width, -1);
        bounds->y1 = scaleY(vp.Top, vp.Height, -1);
        bounds->x2 = scaleX(vp.Left, vp.Width, 1);
        bounds->y2 = scaleY(vp.Top, vp.Height, 1);     // negate top and bottom to invert Y    
        return true;
    }

    // TBD: When is TreeCacheNode created for masks?
    //  --> Attached: at parent attachment (to pRoot)
    //  --> Non-Attached: at time of pRoot initialization.
    // Conclusion: all masks should have a root.

    const TreeNode::NodeData* childData = child->GetNodeData();

    // Transform mask into our coordinate system. Transform logic has
    // optimized special cases for direct child/sibling masks.
    Matrix m(maskNodeData->M2D());
    if (maskNodeCache->GetParent() != this)
    {        
        if (maskNodeCache->GetParent() == child)
            m.Append(childData->M2D());
        else
        {   // TBD: Not necessary if Mask is always a child of parent.
            maskNodeCache->CalcViewMatrix(&m);
            Matrix m2, m3;
            CalcViewMatrix(&m3);
            m2.SetInverse(m3);
            m.Append(m3);
        }
    }

    *bounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);
    return true;
}



// Pixel-area thresholds for mask modes; multiple constans to apply hysteresis.
const float MaskClipAreasThresholds[MES_Entry_Count] =
{
    23500.0f,
    23500.0f,     
    17000.0f, // Low threshold (if already clipped, stay so).
    30000.0f  // High threshold (if already combinable, stay so).
};

MaskEffectState TreeCacheNode::calcMaskBounds(RectF* maskBounds,
                                              Matrix2F* boundAreaMatrix,
                                              const Matrix3F& viewMatrix,
                                              MaskEffectState oldState)
{
    SF_ASSERT(HasMask());
    // TBD: Assumes mask parent is this.
    SF_ASSERT(GetMask()->GetParent() == this);

    const TreeNode::NodeData* data = GetNodeData();    
    const TreeNode::NodeData* maskNodeData = GetMask()->GetNodeData();
    if (!maskNodeData->IsVisible() || maskNodeData->AproxParentBounds.IsEmpty())
        return MES_NoMask;

    // Figure out if there is there any 3D in the tree (in which case we need to project the bounds into clip space).
    bool has3d = maskNodeData->Is3D();
    TreeCacheNode * pparent = this;
    Matrix4F viewproj;
    while ( pparent && !has3d )
    {
        has3d |= pparent->GetNodeData()->Is3D();
        pparent = pparent->GetParent();
    }

    // 2D case.
    if ( !has3d )
    {
        Matrix2F viewMatrix2D;
        viewMatrix2D.SetMatrix(viewMatrix);

        // TBD: When is TreeCacheNode created for masks?
        //  --> Attached: at parent attachment (to pRoot)
        //  --> Non-Attached: at time of pRoot initialization.
        // Conclusion: all masks should have a root.    
        
        // Transform mask into our coordinate system. 
        // TBD: Transform logic has optimized special cases for direct child/sibling masks.
        Matrix2F m(maskNodeData->M2D());
        *maskBounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);

        //  Determine intersection; cull all content if there is no common area.
        RectF intersect, boundsUnion;
        if (!maskBounds->IntersectRect(&intersect, data->AproxLocalBounds))
            return MES_Culled;

        maskBounds->UnionRect(&boundsUnion, data->AproxLocalBounds);

        RectF    viewUnionBounds = viewMatrix2D.EncloseTransform(boundsUnion);
        float    fraction        = intersect.Area() / boundsUnion.Area();
        float    viewArea        = viewUnionBounds.Area();
        Matrix2F boundsMatrix;
        MaskEffectState newState;
        float    differenceArea = viewArea * (1.0f - fraction);

        // Decide mask clip/combine mode based on the pixel area difference
        // between union and intersection. Use hysteresis thresholds to avoid
        // extraneous mode switches.    
        if (differenceArea > MaskClipAreasThresholds[oldState])
        {
            newState = MES_Clipped;
            // Matrix that maps unit rect {0,0, 1,1} to bounds.
            // Clipped area matrix is always in viewport coordinates, meaning that the
            // following holds: (m.Shx()== 0.0f) && (m.Shy() == 0.0f). This is necessary
            // allow clipped mask to be fully erased in HAL by transforming a unit rectangle.
            RectF viewClipRect;
            viewMatrix.EncloseTransform(&viewClipRect, intersect);
            // Expand to enclose pixel (could be significant with FSAA).
            viewClipRect.x1 = floorf(viewClipRect.x1);
            viewClipRect.y1 = floorf(viewClipRect.y1);
            viewClipRect.x2 = ceilf(viewClipRect.x2);
            viewClipRect.y2 = ceilf(viewClipRect.y2);
            boundAreaMatrix->SetMatrix(viewClipRect.Width(), 0.0f, viewClipRect.x1,
                0.0f, viewClipRect.Height(), viewClipRect.y1);
        }
        else
        {
            // Combined mask; we'll clear the whole local area.
            newState = MES_Combinable;
            boundAreaMatrix->SetMatrix(boundsUnion.Width(), 0.0f, boundsUnion.x1,
                0.0f, boundsUnion.Height(), boundsUnion.y1);
            boundAreaMatrix->Append(viewMatrix2D);
        }
        return newState;
    }
    else
    {
        // 3D case.
        if ( maskNodeData->Is3D() )
        {
            Matrix3F m(maskNodeData->M3D());
            *maskBounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);
        }
        else
        {
            Matrix2F m(maskNodeData->M2D());
            *maskBounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);
        }

        // View and project matrices always come from the root.
        const TreeNode::NodeData* pRootData = pRoot->GetNodeData();
        const ProjectionMatrix3DState* projState = pRootData->GetState<ProjectionMatrix3DState>();
        SF_ASSERT(projState);

        const ViewMatrix3DState* viewState = pRootData->GetState<ViewMatrix3DState>();
        SF_ASSERT(viewState);

        viewproj = Matrix4F(projState->GetProjectionMatrix3D()->M) * Matrix4F(viewState->GetViewMatrix3D()->M);

        // Transform into clip space.
        SF_ASSERT( pRoot->GetNodeData()->HasViewport() );
        const Viewport & vp = pRoot->GetNodeData()->VP;
        RectF clipBounds;
        Matrix4F wvp = viewproj * viewMatrix;
        wvp.EncloseTransformHomogeneous(&clipBounds, *maskBounds);

        // Scale to VP coordinates.
        maskBounds->x1 = scaleX(vp.Left, vp.Width, clipBounds.x1);
        maskBounds->y1 = scaleY(vp.Top, vp.Height, clipBounds.y2);
        maskBounds->x2 = scaleX(vp.Left, vp.Width, clipBounds.x2);
        maskBounds->y2 = scaleY(vp.Top, vp.Height, clipBounds.y1);     // negate top and bottom to invert Y

        // Save the results.
        //boundAreaMatrix->SetMatrix(maskBounds->Width(), 0.0f, maskBounds->x1, 0.0f, maskBounds->Height(), maskBounds->y1);
        boundAreaMatrix->SetMatrix((float)vp.Width, 0.0f, (float)vp.Left, 0.0f, (float)vp.Height, (float)vp.Top);
        return MES_Combinable;
    }
}


void TreeCacheNode::UpdateBundlePattern()
{
    // Mask update not possible locally because chain isn't known.
}


void TreeCacheNode::CalcViewMatrix(Matrix* pviewMatrix) const
{
    // CalcViewMatrix shouldn't be called on TreeCacheNodes that don't have
    // associated TreeNodes (such as TreeShapeCacheLayer).
    SF_ASSERT(pNode);

    const TreeCacheNode* p = this;
    *pviewMatrix = GetNodeData()->M2D();

    while(p->GetParent())
    {
        p = p->GetParent();
        pviewMatrix->Append(p->GetNodeData()->M2D());
    }
}

void TreeCacheNode::CalcViewMatrix(Matrix3F* pviewMatrix) const
{
    // CalcViewMatrix shouldn't be called on TreeCacheNodes that don't have
    // associated TreeNodes (such as TreeShapeCacheLayer).
    SF_ASSERT(pNode);

    const TreeCacheNode* p = this;
    *pviewMatrix = GetNodeData()->M3D();

    while(p->GetParent())
    {
        p = p->GetParent();
        pviewMatrix->Append(p->GetNodeData()->M3D());
    }
}

/*

//------------------------------------------------------------------------
void TreeCacheNode::CalcScale9GridParameters(RectF* s9gRect, Matrix2F* shapeMtx) const
{
    SF_ASSERT(pScale9Bounds.GetPtr());

    const TreeCacheNode* node = this;
    const TreeCacheNode* s9gNode = 0;
    while(node)
    {
        const TreeNode::NodeData* data = node->GetNodeData();
        const Scale9State* s9s = data->GetState<Scale9State>();
        if (s9s)
        {
            s9gNode = node;
            pScale9Bounds->S9gMtx = data->M2D();
            *s9gRect = s9s->GetRect();
        }
        node = node->GetParent();
    }

    SF_ASSERT(s9gNode);

    node = this;
    for(;;)
    {
        const TreeNode::NodeData* data = node->GetNodeData();
        shapeMtx->Append(data->M2D());
        if (node == s9gNode)
            break;
        node = node->GetParent();
    }
}
*/

// ***** Cached Nodes Handling 

TreeCacheContainer::~TreeCacheContainer()
{
    // Set all child items, if still alive, to have null pParent pointer.
    TreeCacheNode* p = Children.GetFirst();
    while(!Children.IsNull(p))
    {
        SF_ASSERT(p->GetParent() == this);
        TreeCacheNode* next = p->pNext;
        p->SetParent(0);
        p->pPrev = p->pNext = 0;
        p = next;
    }
}



//------------------------------------------------------------------------
// UpdateTransform - updates and propagates Matrix and Cxform values
// down the tree. Also performs culling & mask updates.
// For nested layer, these will receive matrix and node from TreeCacheShape.
void TreeCacheContainer::UpdateTransform(const TreeNode::NodeData* pbaseData,
                                         const TransformArgs& t,
                                         TransformFlags flags)
{    
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheContainer::UpdateTransform");
    TransformArgs args(t.CullRect); // Assign CullRect only.
    flags = updateCulling(pbaseData, t, &args.CullRect, flags);

    SortParentBounds = pbaseData->AproxParentBounds;
    SetFlags(GetFlags()&~NF_ExpandedBounds);

    // Traverse child list and not pbaseData->Children array because the
    // later may have nodes with no cache (in case we are in destroying phase).
    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {        
        const TreeNode::NodeData* pchildData = child->GetNodeData();
        TransformFlags            childFlags = (TransformFlags)
                                    (flags | (child->UpdateFlags & (Change_CxForm|Change_Matrix)));

        child->UpdateFlags &= ~(Change_Matrix|Change_CxForm); 

        // Concat transforms:
        // if child mat is 2D then append it to cur 2D mat, set has2D flag (start or continue a 2D sequence)
        // if child mat is 3D then,
        //      if cur has2D then append child 3D to cur 2D (converted to 3D), clear has2D
        //      else append child 3D to cur 3D
        //      set has3D
        if (!pchildData->Is3D())
        {
            // child is 2D
            if (flags & TF_Has2D)
                args.Mat.SetToAppend(pchildData->M2D(), t.Mat);         // child append t or  (t * child)
            else
                args.Mat = pchildData->M2D();
            childFlags = (TransformFlags)(childFlags | TF_Has2D);
            if (flags & TF_Has3D)
                args.SetMatrix3D(t.GetMatrix3D(TF_Has3D));       // carry along the 3D transform if necessary
        }
        else
        {   // child is 3D            
            if (flags & TF_Has2D)
		{
	            args.SetMatrix3D(t.GetMatrix3D(flags) * pchildData->M3D());
                childFlags = (TransformFlags)(childFlags & ~TF_Has2D);  // clear 2D flag since 2D has been appended to 3D
		}
		else	
                if (flags & TF_Has3D)
                    args.SetMatrix3D(t.GetMatrix3D(flags) * pchildData->M3D());           // append child 3D to cur 3D, continue a 3D sequence		
            childFlags = (TransformFlags)(childFlags | TF_Has3D);
        }

        args.Cx.SetToAppend(pchildData->Cx, t.Cx);
        child->UpdateTransform(pchildData, args, childFlags);
        child = child->pNext;
    }
}


//------------------------------------------------------------------------
/*
RectF TreeCacheContainer::CalcScale9GridBounds(const Matrix2F& m)
{
    RectF r(0);
    RectF tmp;
    Matrix m2;

    const NodeData* pdata = GetNodeData();

    UPInt childCount = pdata->Children.GetSize();
    for(UPInt i = 0; i < childCount; i++)
    {
        TreeNode*                 pchildNode = pdata->Children[i];
        TreeCacheNode*            pchildCache= (TreeCacheNode*)pchildNode->GetRenderData();
        const TreeNode::NodeData* pchildData = pchildNode->GetDisplayData();
        m2 = m;
        m2.Prepend(pchildData->M2D());
        tmp = pchildCache->CalcScale9GridBounds(m2);

        if (!tmp.IsEmpty())
        {
            if (!r.IsEmpty())
                r.Union(tmp);
            else
                r = tmp;
        }
    }
    return r;
}
*/

//------------------------------------------------------------------------
// 
void    TreeCacheContainer::HandleChanges(unsigned changeBits)
{
    SF_ASSERT(pNode);
    if (!(changeBits & (Change_ChildInsert|Change_ChildRemove)))
        return;

    // Go through child data array and verify that it matches our list.
    // Remove/add items as necessary.
    bool            change = false;
    const NodeData* pdata = GetNodeData();
    UPInt           childCount = pdata->Children.GetSize();
    unsigned        index;
    TreeCacheNode*  pcache1 = Children.GetFirst();    

    for(index = 0; index < childCount; index++)
    {
        TreeNode*      pnode0  = pdata->Children.GetAt(index);
        TreeCacheNode* pcache0 = (TreeCacheNode*)pnode0->GetRenderData();

        // If item matches, move on.
        if (pcache0 == pcache1)
        {
            pcache1 = pcache1->pNext;
            continue;
        }

        // If item doesn't have us as parent or isn't a part of our Children
        // list, as in (pPrev == 0) for masks, re-root it.
        if (!pcache0 || (pcache0->GetParent() != this) || !pcache0->pPrev)
        {
            if (pcache0 && pcache0->GetParent())
            {
                TreeCacheNode* oldParent = pcache0->GetParent();
                // This branch was removed elsewhere in the tree and re-rooted to us!!
                //  handle that remove...
                pcache0->RemoveFromParent();
                if (oldParent->pRoot)
                    oldParent->pRoot->AddToUpdate(oldParent, Update_Pattern);
            }
            
            // Insertion; insert content ... it can either be a sprite or a shape!
            TreeCacheNode* pinserted = 
                pnode0->GetDisplayData()->updateCache(this, pcache1->pPrev, pnode0, Depth + 1);

            if (pinserted && pinserted->pRoot)
                pinserted->pRoot->AddToUpdate(pinserted, Change_Matrix | 
                                                         Change_CxForm);
            change = true;

            // It is possible (although highly rare) that insert operation will
            // remove our next child, adding it somewhere in the subtree.
            // If this happens, re-run the loop.
            if (!Children.IsNull(pcache1) && (pcache1->GetParent() != this))
            {   // Reset.
                pcache1 = Children.GetFirst();
                index   = (unsigned)-1;
            }

            // no pcache1 advance
            continue;
        }

        // If we are here, (pcache0->pParent == this).
        // If means that either that
        //   (a) the pcache1 item was removed from the list/relocated or
        //   (b) the pcache0 item was moved here from a different location in our list.
        SF_ASSERT(!Children.IsNull(pcache1));
        
        // Just remove the current and future items until we hit pcache0 or reach
        // the end of the cached item list.
        do {
            TreeCacheNode* pcache1Next = pcache1->pNext;
            SF_ASSERT(pcache1->pParent == this);
            pcache1->RemoveFromParent();
            pcache1 = pcache1Next;

        } while((pcache1 != pcache0) && !Children.IsNull(pcache1));
        
        change = true;
               
        // Don't increment index, since we need to loop around to insert this index item
        index--;
    }

    // Remove remaining items, if any.
    while (!Children.IsNull(pcache1))
    {
        TreeCacheNode* pcache1Next = pcache1->pNext;
        SF_ASSERT(pcache1->pParent == this);
        pcache1->RemoveFromParent();
        pcache1 = pcache1Next;
        change = true;
    }

    if (change && IsPatternChainValid())
    {
        SF_ASSERT(pRoot); // implied not null by CachedChildPattern validity.
        pRoot->AddToUpdate(this, Update_Pattern);
    }
}


// Removes this node by clearing out pRoot, so that node matrix propagation doesn't happen
void TreeCacheContainer::HandleRemoveNode()
{
    TreeCacheNode::HandleRemoveNode();
    //pRoot = 0;
    CachedChildPattern.Clear(Range_Invalid);

    TreeCacheNode* p = Children.GetFirst();
    while (!Children.IsNull(p))
    {
        p->HandleRemoveNode();
        p = p->pNext;
    }
}


// TBD: UpdateChildSubtree processing may be somewhat incorrect if a tree is being added that
// had several modifications done to it in the same frame. If that is the case, HandleChanges
// will rearrange the subtree leafs again...
void TreeCacheContainer::UpdateChildSubtree(const TreeNode::NodeData *pdata, UInt16 depth)
{
    TreeCacheNode::UpdateChildSubtree(pdata, depth);
    // Insertion pointer for new nodes as we go.
    TreeCacheNode*        pnewInsert = Children.GetFirst()->pPrev;
    const TreeNodeArray & children = static_cast<const NodeData*>(pdata)->Children;

    for(UPInt i=0; i<children.GetSize(); i++)
    {
        TreeNode* pchildNode = children[i];
        pnewInsert = 
            pchildNode->GetDisplayData()->updateCache(this, pnewInsert, pchildNode, depth);
    }
}

void TreeCacheContainer::propagateMaskFlag(unsigned partOfMask)
{
    ApplyFlags(NF_PartOfMask, partOfMask);    

    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        // State of a branch under NF_MaskNode flag can't change,
        // so don't go there.
        if (!(child->IsMaskNode()))
            child->propagateMaskFlag(partOfMask);
        child = child->pNext;
    }
}

void TreeCacheContainer::propagateScale9Flag(unsigned partOfScale9)
{
    if (GetNodeData()->GetState<Scale9State>())
        partOfScale9 |= NF_PartOfScale9;
    // No need to recurse down if the flag is already set correctly.
    if ((GetFlags() & NF_PartOfScale9) == partOfScale9)
        return;

    ApplyFlags(NF_PartOfScale9, partOfScale9);

    TreeCacheNode* child = Children.GetFirst();
    while(!Children.IsNull(child))
    {
        child->propagateScale9Flag(partOfScale9);
        child = child->pNext;
    }
}

bool FixedSizeArrayRectF::Intersects(const RectF& bounds)
{
    if (!bounds.IsEmpty())
    {
        for (UPInt i=0; i<GetSize(); i++)
        {
            RectF& r = operator[](i);
            bool noIntersection = (r.x1 >= bounds.x2) || (bounds.x1 >= r.x2) ||
                                  (r.y1 >= bounds.y2) || (bounds.y1 >= r.y2);
            if (!noIntersection)
                return true;
        }
    }
    return false;
}    

#ifdef SF_ENABLE_SIMD
// This implementation is roughly +50% faster on X86 then stock Intersects.
// For frogger.swf (Ctrl+G), this improves framerate 895 -> 960fps.

void FixedSizeArrayRect2F::PushBack(const RectF& r)
{
    using namespace Scaleform::SIMD;
    if (HalfRect)
    {
        Rect2F& t = pData[Size-1];
        t.r0 = IS::SetHigh( t.r0, r.x1, r.y1 );
        t.r1 = IS::SetHigh( t.r1, r.x2, r.y2 );
        HalfRect = false;
    }
    else
    {
        // Allow for fast growth; ok since its temp store.
        if (Size == Reserve)
        {
            grow(Size * 2);
        }
        Rect2F& t = pData[Size];
        Size++;
        t.r0 = IS::SetLow( t.r0, r.x1, r.y1 );
        t.r1 = IS::SetLow( t.r1, r.x2, r.y2 );
        HalfRect = true;
    }
}

bool FixedSizeArrayRect2F::Intersects(const RectF& bounds)
{
    using namespace Scaleform::SIMD;
    if (!bounds.IsEmpty())
    {
        Vector4f bound_y2x2y1x1 = IS::LoadAligned(&bounds.x1);
        Vector4f b_y1x1y1x1 = IS::Shuffle<1,0,1,0>(bound_y2x2y1x1, bound_y2x2y1x1);
        Vector4f b_y2x2y2x2 = IS::Shuffle<3,2,3,2>(bound_y2x2y1x1, bound_y2x2y1x1);

        UPInt i = 0;
        UPInt count = GetSize();
        if (HalfRect)
            count--;

        for (i=0; i<count; i++)
        {
            Rect2F* pr = &operator[](i);
            Vector4f r1_y1x1_r0_y1x1 = pr->r0;
            Vector4f r1_y2x2_r0_y2x2 = pr->r1;

            int mask0 = IS::MoveMask(IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2));
            int mask1 = IS::MoveMask(IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2));
            int commonMask = mask0 | (mask1 << 4);

            if (((commonMask & 0x33) == 0) || ((commonMask & 0xCC) == 0))
                return true;
        }

        if (HalfRect)
        {
            Rect2F* pr = &operator[](i);
            Vector4f r1_y1x1_r0_y1x1 = pr->r0;
            Vector4f r1_y2x2_r0_y2x2 = pr->r1;

            int mask0 = IS::MoveMask(IS::CompareGE(r1_y1x1_r0_y1x1, b_y2x2y2x2));
            int mask1 = IS::MoveMask(IS::CompareGE(b_y1x1y1x1, r1_y2x2_r0_y2x2));
            int commonMask = mask0 | (mask1 << 4);
            if ((commonMask & 0x33) == 0)
                return true;
        }
    }
    return false;
}

void FixedSizeArrayRect2F::Clear()
{
    FixedSizeArray<Rect2F>::Clear();
    HalfRect = false;
}
#endif // SF_ENABLE_SIMD

void TreeCacheContainer::BuildChildPattern(BundleEntryRange* pattern)
{
    FixedSizeArrayRectFImpl prevRects;
    
    SF_ASSERT(pRoot);
    unsigned childIndex = 0;

    pattern->Clear(Range_Empty);
    BundleEntryRangeMatcher prevPattern(Range_Empty);
    bool             expandBounds = false;
    RectF            expandedBounds;

    TreeCacheNode* child = Children.GetFirst();
    for (; !Children.IsNull(child); child = child->pNext, childIndex++)
    {        
        // Get existing chain
        BundleEntryRange childPattern(Range_Empty);
        SF_ASSERT(child->Depth != 0);
        if (!child->GetPatternChain(&childPattern))
            continue;
        //childPattern.VerifyValidChildDepth(chainIndex);

        // Limit on match length. If we are 3D, children cannot join to the previous chain.
        if ((child->GetFlags() & NF_3D) || childPattern.GetLength() > Bundle_MergeLengthLimit)
        {
            pattern->Append(prevPattern);
            pattern->Append(childPattern);
            prevPattern.Clear(Range_Empty);
            prevRects.Clear();
            continue;
        }

        RectF bounds;
        bool  match = false;

        if (child->pNode)
        {
            bounds = child->SortParentBounds;
            //const TreeCacheNode::NodeData* data = child->GetNodeData();
            //bounds = data->AproxParentBounds;
            // data->M2D().EncloseTransform(data->AproxLocalBounds);
        }

        // Expand bounds to accommodate mask.
        if (child->GetFlags() & (NF_ExpandedBounds|NF_HasMask))
        {
            if (child->HasMask())
            {
                // Expand by mask.
                RectF maskBounds;
                if (calcChildMaskBounds(&maskBounds, child))
                {
                    if (expandBounds)
                        expandedBounds = maskBounds;
                    else
                    {
                        expandBounds = true;
                        expandedBounds.UnionRect(&expandedBounds, maskBounds);
                    }
                    if (!bounds.IsEmpty())
                        bounds.UnionRect(&bounds, maskBounds);
                    else
                        bounds = maskBounds;
                }
            }

            if (expandBounds)
                expandedBounds = bounds;
            else
            {
                expandBounds = true;
                expandedBounds.UnionRect(&expandedBounds, bounds);
            }
        }

        if (!prevPattern.IsEmpty())
        {
            // Don't do overlap test if we're 3D - it isn't accurate.
            bool matchNotAllowed = ((child->GetFlags() & NF_3D) == 0) && prevRects.Intersects(bounds);

            // See if new pattern is sub-pattern of the old one
            if (!matchNotAllowed)
            {
                match = prevPattern.Match(childPattern);
                // TBD: Chain on match?
            }
            else
            {
                // If pattern is one-unit long and key is identical, overlaps are ok
                if ((prevPattern.GetLength() == 1) && (childPattern.GetLength() == 1) &&
                    (prevPattern.GetFirstKey().MatchSingleItemOverlap(childPattern.GetFirstKey())))
                    match = true;
            }
        }

        if (match)
        {
            // On match, concatenate our chains to those of child.
            prevPattern.MergeMatchingChains(childPattern, Depth);
        }
        else
        {
            // If no match took place
            pattern->Append(prevPattern);
            prevPattern = childPattern;
            prevRects.Clear();
        }

        if (!bounds.IsEmpty())
            prevRects.PushBack(bounds);
    }

    pattern->Append(prevPattern);


    // Expand our bounds to account for child expansion (masks), if necessary.
    // This will ensure that we are properly sorted.
    if (expandBounds)
    {
        SF_ASSERT(pNode);
        const TreeNode::NodeData* data = pNode->GetDisplayData();
        if (!data->AproxLocalBounds.IsEmpty())
            expandedBounds.UnionRect(&expandedBounds, data->AproxLocalBounds);
        data->M2D().EncloseTransform(&expandedBounds, expandedBounds);

        if (expandedBounds != SortParentBounds)
        {
            SortParentBounds = expandedBounds;
            SetFlags(GetFlags()|NF_ExpandedBounds);
        }
    }

    BuildPatternCount++;
}


// Make sure that all items are valid
void VerifyPattern(BundleEntryRange& pattern)
{
    unsigned       count = 0;
    BundleIterator ibundle = pattern.GetIterator();
    
    while(ibundle)
    {   
        // Alias chain, stripping merged nodes that we originally
        // connected higher up in the tree.
        BundleEntry*   chain = ibundle;
        TreeCacheNode* pcacheNode = ibundle->pSourceNode;

        SF_ASSERT(pcacheNode->pRoot);
        SF_UNUSED(pcacheNode);
        if (!pcacheNode->pRoot)
            printf("Null pRoot in %p - %p node\n", ibundle.GetEntry(), pcacheNode);

        unsigned chainCount = 0;
        while(chain->pChain)
        {   
            SF_ASSERT(chain->pSourceNode->pRoot);
            if (!chain->pSourceNode->pRoot)
                printf("Null pRoot in %p - %p node - at chain loop %d\n",
                       ibundle.GetEntry(), ibundle->pSourceNode, chainCount);
            chain = chain->pChain;
            chainCount++;
        }

        // Check validity of possible advance.
        if (ibundle.GetEntry() != pattern.GetLast())
        {
            if (!ibundle->HasNextPattern())
            {
                printf("Frame %d: Null NextPattern after %p, count = %d\n",
                       BundlePatternFrameId, ibundle.GetEntry(), count);
                TreeCacheNode* parent = pcacheNode->GetParent();
                printf("SourceNode = %p, parent = %p\n", pcacheNode, parent);
                while(parent)
                {
                    printf("%p Depth = %d\n", parent, parent->Depth);
                    //printf("%p Updated at frame %d, Depth = %d\n",
                    //          parent, parent->BuildFrameId, parent->Depth);
                    parent = parent->GetParent();
                }
            }
        }

        ibundle++;
        count++;
    }
}


// Verified/Rebuilds the bundle pattern for this container,
// adding parent to update list if necessary.

void TreeCacheContainer::UpdateBundlePattern()
{
    // Not that invalid patterns are processed later above in top-down
    // fashion, so skip them if they were added by mistake.
    if (!IsPatternChainValid())
        return;
    SF_ASSERT(pRoot);

    /*
    if (BundlePatternFrameId == 546751)
    {
        if ((this == (void*)0x1e13e90) || (this == (void*)0x1e61590))
        {
            printf("UpdateBundlePattern(%p) { Depth = %d; CCP.Length = %d }\n",
                   this, Depth, CachedChildPattern.GetLength());
        }
    }
    */

    // Build an alias list pattern
    BundleEntryRange pattern(Range_Invalid);
    BuildChildPattern(&pattern);

    if (!Effects.IsEmpty() && !pattern.IsEmpty())
    {   
        // We always rebuild this chain, even if pattern and CachedChildPattern values match.
        // The later can't be relied upon because child item could've been re-rooted deeper
        // in subtree, which would yield same pattern but with broken pNextPattern node.
        BundleEntryRange chain(pattern);        
        updateEffectChain(&chain);
    }

    // See if parent update is necessary. It is needed if:
    //  1. First/Last pointer doesn't match.
    //  2. Pattern old/new Length <= limit and pattern structure has changed.
    //  3. Pattern old/new Length <= limit and at least one of its tail chain nodes don't match.

    // Invisible nodes don't require parent update notification, since they
    // don't contribute bundles. Parent notification takes place externally
    // when visibility changes.
    if (IsDrawn())
    {
        if ((CachedChildPattern.GetLength() <= Bundle_MergeLengthLimit) ||
            (pattern.GetLength() <= Bundle_MergeLengthLimit) ||
            (CachedChildPattern.GetFirst() != pattern.GetFirst()) ||
            (CachedChildPattern.GetLast() != pattern.GetLast()) )
        {  
            addParentToDepthPatternUpdate();
        }
        else
        {
            
            if (CachedChildPattern.GetLength() != pattern.GetLength())
            {
                if (GetParent() && GetParent()->IsPatternChainValid() &&
                    !(GetParent()->UpdateFlags & Update_Pattern_Mask))
                {
                    // Propagate length change; this will very rarely happen.
                    addParentToDepthPatternUpdate();

              //      printf("Node %p at d=%d, Length %d -> %d\n", this, Depth,
              //             CachedChildPattern.GetLength(), pattern.GetLength());
                }
            }
        }
    }

    CachedChildPattern = pattern;
}


bool TreeCacheContainer::GetPatternChain(BundleEntryRange* range)
{
    // Invisible and/or culled objects don't contribute bundles to parent.
    if (!IsDrawn())
    {
        range->Clear(Range_Empty);
        return false;
    }
    
    bool     oldChainValid     = CachedChildPattern.IsValid();
    unsigned updateEffectsMask = UpdateFlags & Change_State_Effect_Mask;
    
    if (oldChainValid && !updateEffectsMask)
    {
        if (!CachedChildPattern.IsEmpty())
        {
            if (!Effects.GetRange(range))
                *range = CachedChildPattern;
            range->StripChainsByDepth(Depth);
        }
    }
    else
    {
        if (oldChainValid)
            CachedChildPattern.StripChainsByDepth(Depth);
        else
            BuildChildPattern(&CachedChildPattern);
        *range = CachedChildPattern;

        // Update the effects.
        if (updateEffectsMask)
        {
            Effects.UpdateEffects(this, updateEffectsMask);
            UpdateFlags &= ~Change_State_Effect_Mask;
        }
        if (!CachedChildPattern.IsEmpty())
            updateEffectChain(range);
    }

    return range->GetLength() != 0;
}

bool TreeCacheContainer::IsPatternChainValid() const
{
    return CachedChildPattern.IsValid();
}



//------------------------------------------------------------------------
// ***** TreeCacheRoot Implementation

void TreeCacheRoot::ChainUpdatesByDepth()
{
    TreeCacheNode *p0, *p = pUpdateList;
    pUpdateList = 0;

    while (p)
    {
        p0 = p->pNextUpdate;
        DepthUpdates.Link(p->Depth, &p->pNextUpdate, p);
        p = p0;
    }
}


void TreeCacheRoot::UpdateTreeData()
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheRoot::UpdateTreeData");

   // SF_ASSERT(BundlePatternFrameId != 13994);

    // And update all Transforms (Matrices, Cxforms) as necessary.
    // Data is propagated top-down, so we visit low-depth indices first.    
    unsigned depthUsed = DepthUpdates.GetDepthUsed();
    unsigned i;
    for (i = 0; i < depthUsed; i++)
    {
        TreeCacheNode* pnodeCache = DepthUpdates.Get(i);
        while(pnodeCache)
        {
            unsigned transformChangeFlags =
                pnodeCache->UpdateFlags & (Change_Matrix|Change_CxForm);
            
            if (pnodeCache->UpdateFlags & Change_State_Scale9)
            {
                TreeCacheNode* p = pnodeCache->GetParent();
                // propagateScale9Flag will check nodes own Scale9 state.
                pnodeCache->propagateScale9Flag(p ? (p->GetFlags() & NF_PartOfScale9) : 0);
                transformChangeFlags |= Change_Matrix;
            }

            if (pnodeCache->UpdateFlags & Change_3D)
            {
                pnodeCache->ApplyFlags(NF_3D, pnodeCache->GetNodeData()->Is3D() ? NF_3D : 0 );
                transformChangeFlags |= Change_Matrix;
            }

            // Update transform.
            if (transformChangeFlags)
            {
                SF_ASSERT(pnodeCache->pNode);
                
                // Compute cumulative ancestor Matrix/Cxform.
                const TreeNode::NodeData* pdata = pnodeCache->GetNodeData();
                TransformFlags flags = (TransformFlags)
                    (transformChangeFlags | (ViewValid ? TF_NeedCull : 0));
                TransformArgs  args;
                if (pdata->Is3D())
                {
                    args = TransformArgs(ViewCullRect, Matrix2F::Identity, pdata->M3D(), pdata->Cx);
                    flags = (TransformFlags)(flags | TF_Has3D);
                }
                else
                {
                    args = TransformArgs(ViewCullRect, pdata->M2D(), Matrix3F::Identity, pdata->Cx);
                    flags = (TransformFlags)(flags | TF_Has2D);
                }                

                // We need to compute both Matrix and Cxform even if one of the flags
                // is not set in case one of child objects does need that attribute
                // updated as well.
                TreeCacheNode* p = pnodeCache->GetParent();
                while(p)
                {
                    if (!p->IsDrawn())
                        flags = (TransformFlags)(flags & ~TF_NeedCull);
                    const TreeNode::NodeData* data = p->GetNodeData();

                    if (flags & TF_Has3D)
                        // accumulate in 3D
                        args.AppendMatrix3D(data->Is3D() ? data->M3D() : data->M2D());
                    else
                    {   // 2D case
                        if (data->Is3D())
                        {   // switch to 3D accumulation
                            args.SetMatrix3D(args.Mat);
                            args.AppendMatrix3D(data->M3D());
                            args.Mat.SetIdentity();
                            flags = (TransformFlags)(flags & ~TF_Has2D);
                            flags = (TransformFlags)(flags | TF_Has3D);
                        }
                        else
                            args.Mat.Append(data->M2D());
                    }
                    args.Cx.Append(data->Cx);
                    p = p->GetParent();
                }
                pnodeCache->UpdateTransform(pdata, args, flags);
                pnodeCache->UpdateFlags &= ~(Change_Matrix|Change_CxForm);
            }

            pnodeCache = pnodeCache->pNextUpdate;
        }
    }

    // Update patterns - this needs to be done from top to bottom.
    // Refresh depthUsed since higher depth updates could've been queued up
    // due to culling.
    depthUsed = DepthUpdates.GetDepthUsed();
    for (i = depthUsed; i != 0; i--)
    {
        TreeCacheNode* pnodeCache = DepthUpdates.Get(i-1);
        while(pnodeCache)
        {
            if (pnodeCache->UpdateFlags & Update_Pattern_Mask)
            {
                // Rebuild/verify pattern for this node.
                pnodeCache->UpdateBundlePattern();
                pnodeCache->UpdateFlags &= ~Update_Pattern_Mask;
            }

            pnodeCache->UpdateFlags &= ~Update_InList;
            pnodeCache = pnodeCache->pNextUpdate;
        }
    }

    // Debug only.
    // VerifyPattern(CachedChildPattern);
    
    DepthUpdates.Clear();

    // Update all root bundle chains.
    BundleIterator ibundles = CachedChildPattern.GetIterator();
    while(ibundles)
    {
        ibundles->UpdateBundleEntry(this, pRenderer2D);
        ibundles++;
    }


    BundlePatternFrameId++;
    //if ((BundlePatternFrameId & 0xFFF) == 0)
    //    printf("Frame %x\n", BundlePatternFrameId);
}


void TreeCacheRoot::Draw()
{
    SF_AMP_SCOPE_RENDER_TIMER("TreeCacheRoot::Draw");

    if (!IsDrawn())
        return;

    const TreeRoot::NodeData* data = GetNodeData();
    if (data->HasViewport())
    {
        SF_AMP_SCOPE_RENDER_TIMER("HAL::BeginDisplay");
        pRenderer2D->GetHAL()->BeginDisplay(data->BGColor, data->VP);
    }

    // MOOSE TODO - handle view and projection matrix states correctly
    const TreeNode::NodeData* pNodeData = pNode->GetDisplayData();
    const ProjectionMatrix3DState* projState = pNodeData->GetState<ProjectionMatrix3DState>();
    if(projState)
    {
        SF_AMP_SCOPE_RENDER_TIMER("HAL::SetProj3D");
        pRenderer2D->GetHAL()->SetProj3D(projState->GetProjectionMatrix3D()->M);
    }
    
    const ViewMatrix3DState* viewState = pNodeData->GetState<ViewMatrix3DState>();
    if(viewState)
    {
        SF_AMP_SCOPE_RENDER_TIMER("HAL::SetView3D");
        pRenderer2D->GetHAL()->SetView3D(viewState->GetViewMatrix3D()->M);
    }

    BundleIterator ibundles = CachedChildPattern.GetIterator();
    while(ibundles)
    {
        ibundles->DrawBundleEntry(pRenderer2D);
        ibundles++;
    }

    if (data->HasViewport())
    {
        SF_AMP_SCOPE_RENDER_TIMER("HAL::EndDisplay");
        pRenderer2D->GetHAL()->EndDisplay();
    }
}


//------------------------------------------------------------------------
// ***** TreeCacheMeshBase implementation

TreeCacheMeshBase::TreeCacheMeshBase(TreeNode* pnode, const SortKey& key,
                                     Renderer2DImpl* prenderer2D, unsigned flags)
:   TreeCacheNode(pnode, prenderer2D, flags),
    SorterShapeNode(GetThis(), key)
{    
}

TreeCacheMeshBase::~TreeCacheMeshBase()
{    
}

// updates computed final HMatrix
void TreeCacheMeshBase::ComputeFinalMatrix(const TransformArgs& t, TransformFlags flags)
{
    // Initialize HW Matrix/Cxform.
    if (M.IsNull())
    {   // Don't allocate Cxform data initially if not used.
        unsigned matrixFormatBits = HMatrix::Has_2D;
        if (!(t.Cx == Cxform::Identity))
            matrixFormatBits |= HMatrix::Has_Cxform;

        if (flags & TF_Has3D)
        {
            matrixFormatBits |= HMatrix::Has_3D;
            M = GetMatrixPool().CreateMatrix(t.GetMatrix3D(flags), t.Cx, matrixFormatBits);
        }
        else
            M = GetMatrixPool().CreateMatrix(t.Mat, t.Cx, matrixFormatBits);
    }
    else
    {
        if (flags & TF_Has3D)
            M.SetMatrix3D(t.GetMatrix3D(flags));
        else
            M.SetMatrix2D(t.Mat);
        if (flags & TF_Cxform)
            M.SetCxform(t.Cx);
    }
}

// TBD: Handle shape pointer change
void TreeCacheMeshBase::HandleChanges(unsigned changeBits)
{
    SF_UNUSED(changeBits);
}

bool TreeCacheMeshBase::GetPatternChain(BundleEntryRange* range)
{    
    if (!IsDrawn())
    {
        range->Clear(Range_Empty);
        return false;
    }

    range->SetToEntry(&SorterShapeNode);
    SorterShapeNode.Removed = false;

    // We request chain when rebuilding a bundle; so mark it for verify.
    if (Effects.IsEmpty() && !(UpdateFlags & Change_State_Effect_Mask))
    {
        // Optimized "strip chains" version for once item.
        if (SorterShapeNode.pBundle)
            SorterShapeNode.pBundle->SetNeedsUpdate();
        // Note: We do NOT clear pNextPattern on purpose!
        SorterShapeNode.pChain = 0;
        SorterShapeNode.ChainHeight = 0;
    }
    else
    {
        if (UpdateFlags & Change_State_Effect_Mask)
        {
            Effects.UpdateEffects(this, UpdateFlags & Change_State_Effect_Mask);
            updateEffectChain(range);
            UpdateFlags &= ~Change_State_Effect_Mask;
        }
        else
        {
            Effects.GetRange(range);
        }
        // Always clear all chains + notify bundles, since this is
        // a leaf node shape.
        // TBD: May not be true for mask that may have mergeable chain entrys (?)
        //range->StripChains();
        range->StripChainsByDepth(Depth);
    }
  
    return true;
}

void TreeCacheMeshBase::UpdateBundlePattern()
{   
    if (HasMask() && !Effects.IsEmpty())
    {
        BundleEntryRange chain(Range_Empty);
        chain.SetToEntry(&SorterShapeNode);
        updateEffectChain(&chain);
    }
}

}} // Scaleform::Render
