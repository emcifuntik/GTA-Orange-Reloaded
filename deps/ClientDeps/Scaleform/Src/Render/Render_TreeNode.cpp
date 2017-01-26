/**********************************************************************

Filename    :   TreeNode.cpp
Content     :   Rendering tree node classes implementation
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

#include "Render_TreeNode.h"
#include "Render_TreeCacheNode.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// ***** TreeNode

void TreeNode::NodeData::ReleaseNodes() const
{
    if (HasMask())
    {
        removeThisAsMaskOwner(this);
        // Clearing NF_HasMask Probably not needed since we are destroying node...
        // Could clearing it also affect data seen by another thread? Best be safe.
        //Flags &= ~NF_HasMask;
    }
    States.ReleaseNodes();
}

void TreeNode::SetMatrix(const Matrix2F& m)
{
    // If they are switching from 3D to 2D, we need to reflect those changes.
    unsigned flags = Change_Matrix;
    if ( !GetReadOnlyData()->Is3D() )
        flags |= Change_3D;

    NodeData* pdata = GetWritableData(flags); // |Change_AproxBounds
    pdata->SetMatrix(m);
    /*
    RectF r = m.EncloseTransform(pdata->AproxLocalBounds);
    if (r != pdata->AproxParentBounds)
    {
        pdata = GetWritableData(Change_AproxBounds);
        pdata->AproxParentBounds = r;
        if (pParent)
            pParent->AddToPropagate();
  
    } */
    
    // Update our own bounds, then parent if needed.
    AddToPropagate();
}

void TreeNode::SetMatrix3D(const Matrix3F& m)
{
    // If they are switching from 2D to 3D, we need to reflect those changes.
    unsigned flags = Change_Matrix;
    if ( !GetReadOnlyData()->Is3D() )
        flags |= Change_3D;

    NodeData* pdata = GetWritableData(flags); // |Change_AproxBounds
    pdata->SetMatrix3D(m);
    /*
    RectF r = m.EncloseTransform(pdata->AproxLocalBounds);
    if (r != pdata->AproxParentBounds)
    {
        pdata = GetWritableData(Change_AproxBounds);
        pdata->AproxParentBounds = r;
        if (pParent)
            pParent->AddToPropagate();
  
    } */
    
    // Update our own bounds, then parent if needed.
    AddToPropagate();
}

void TreeNode::CalcViewMatrix(Matrix2F *m) const
{    
    *m = GetReadOnlyData()->M2D();
    appendAncestorMatrices(m);
}

void TreeNode::appendAncestorMatrices(Matrix2F *m) const
{
    const TreeNode* p = this;
    while(p->GetParent())
    {
        p = p->GetParent();
        m->Append(p->GetReadOnlyData()->M2D());
    }
}


void TreeNode::SetVisible(bool visible)
{
    if (IsVisible() != visible)
    {
        NodeData* pdata = GetWritableData(Change_Visible);
        pdata->SetVisible(visible);

        // Invisible items retain their bounds, but parent doesn't
        // consider them in bounds computation. This means that
        // we require parent update in bound computation.
        if (Entry::GetParent())
            Entry::GetParent()->AddToPropagate();
    }
}

const State* TreeNode::GetState(StateType state) const
{
    return GetReadOnlyData()->States.GetState(state);
}
void TreeNode::SetState(State& s, unsigned changeFlags)
{
    NodeData* pdata = GetWritableData(changeFlags);

    if (s.GetType() == State_MaskNode)
    {
        MaskNodeState& mns = (MaskNodeState&)s;
        if (!setThisAsMaskOwner(pdata, mns.GetNode()))
            return;      
    }
    pdata->States.SetState(s);
}

bool TreeNode::RemoveState(StateType state, unsigned changeFlags)
{
    NodeData* pdata = GetWritableData(changeFlags);
    
    if (state == State_MaskNode)
    {
        if (!removeThisAsMaskOwner(pdata))
            return false;
        pdata->Flags &= ~NF_HasMask;
    }
    return pdata->States.RemoveState(state);
}

void TreeNode::SetScale9Grid(const RectF& rect)
{
    NodeData* pdata = GetWritableData(Change_State_Scale9);
    if (rect.x1 < rect.x2 && rect.y1 < rect.y2)
    {
        Ptr<Scale9GridRect> s9gr = *SF_HEAP_AUTO_NEW(this) Scale9GridRect;
        if (s9gr)
        {
            s9gr->Scale9 = rect;
            pdata->States.SetState(&Scale9State::InterfaceImpl, s9gr.GetPtr());
        }
    }
    else
    {
        pdata->States.RemoveState(State_Scale9);
    }
}

void TreeNode::SetViewMatrix3D(const Matrix3F& mat3D)
{
    // allocate space and copy
    NodeData* pdata = GetWritableData(Change_State_ViewMatrix3D);
    Ptr<Matrix3FRef> pmat3D = *SF_HEAP_AUTO_NEW(this) Matrix3FRef;
    *pmat3D = mat3D;    
    pdata->States.SetState(&ViewMatrix3DState::InterfaceImpl, pmat3D.GetPtr());
}

void TreeNode::SetProjectionMatrix3D(const Matrix4F& mat3D)
{
    // allocate space and copy
    NodeData* pdata = GetWritableData(Change_State_ProjectionMatrix3D);
    Ptr<Matrix4FRef> pmat3D = *SF_HEAP_AUTO_NEW(this) Matrix4FRef;
    *pmat3D = mat3D;    
    pdata->States.SetState(&ProjectionMatrix3DState::InterfaceImpl, pmat3D.GetPtr());
}

void TreeNode::SetBlendMode(BlendMode mode)
{
    NodeData* pdata = GetWritableData(Change_State_BlendMode);
    if (mode != Blend_None)
        pdata->States.SetState(&BlendState::InterfaceImpl, (void*)mode);
    else
        pdata->States.RemoveState(State_BlendMode);
}

void TreeNode::SetMaskNode(TreeNode* node)
{
    NodeData* pdata = GetWritableData(Change_State_MaskNode);
    if (node)
    {
        if (setThisAsMaskOwner(pdata, node))
            pdata->States.SetState(&MaskNodeState::InterfaceImpl, (void*)node);
    }
    else
    {
        if (removeThisAsMaskOwner(pdata))
        {
            pdata->States.RemoveState(State_MaskNode);
            pdata->Flags &= ~NF_HasMask;
        }
    }
    AddToPropagate();
}

bool TreeNode::setThisAsMaskOwner(NodeData* thisData, TreeNode* node)
{
    NodeData* pmaskData = node->GetWritableData(Change_IsMask);
#ifdef SF_BUILD_DEBUG
    // Error / warn if mask already has a different owner.
    if (pmaskData->IsMaskNode() && (pmaskData->GetMaskOwner() != this))
    {
        SF_DEBUG_WARNING1(1, "TreeNode::SetMaskNode failed - node %p already has owner", node);
        return false;
    }
    if (node->GetParent() && (node->GetParent() != this))
    {
        SF_DEBUG_WARNING1(1, "TreeNode::SetMaskNode failed - node %p already has parent", node);
        return false;
    }
#endif
    // Record the us as owner in mask state.
    node->SetParent(this);
    pmaskData->Flags |= NF_MaskNode;
    pmaskData->States.SetState(&Internal_MaskOwnerState::InterfaceImpl, (void*)this);
    thisData->Flags |= NF_HasMask;    
    // TBD: Mask node's parent shouldn't include it in bounds?
    //if (node->pParent)
    //    node->GetParent()->AddToPropagate();
    return true;
}

bool TreeNode::removeThisAsMaskOwner(const NodeData* thisData)
{
    const MaskNodeState* mns = thisData->States.GetState<MaskNodeState>();
    if (mns)
    {
        NodeData* pmaskData = mns->GetNode()->GetWritableData(Change_IsMask);
        mns->GetNode()->SetParent(0);
        SF_ASSERT(pmaskData->IsMaskNode());
        // TBD: Check owner match for debug?
        pmaskData->Flags &= ~NF_MaskNode;
        pmaskData->States.RemoveState(State_Internal_MaskOwner);
        return true;
    }
    return false;
}


//--------------------------------------------------------------------
// ***** TreeNodeArray implementation

TreeNodeArray::ArrayData TreeNodeArray::NullArrayData = { 1, 0, {0} };

TreeNodeArray::ArrayData*  TreeNodeArray::AllocData2(
                       TreeNode** pnodes1, UPInt count1,
                       TreeNode** pnodes2, UPInt count2,
                       UPInt extra)
{
    UPInt       size = sizeof(ArrayData) + sizeof(TreeNode*) * (count1 + count2 + extra - 1);
    ArrayData*  pdata = (ArrayData*)SF_HEAP_AUTO_ALLOC(this, size);
    pdata->RefCount = 1;
    pdata->Size     = count1 + count2 + extra;

    TreeNode** p = pdata->Nodes;
    if (count1)
    {
        memcpy(p, pnodes1, sizeof(TreeNode*) * count1);
        p += count1;
    }
    if (count2)
    {
        memcpy(p, pnodes2, sizeof(TreeNode*) * count2);
        //p += count2;
    }
    return pdata;
}

TreeNodeArray::ArrayData*  TreeNodeArray::AllocData3(
                       TreeNode** pnodes1, UPInt count1,
                       TreeNode** pnodes2, UPInt count2,
                       TreeNode** pnodes3, UPInt count3)
{
    ArrayData*  pdata = AllocData2(pnodes1, count1, pnodes2, count2, count3);
    if (count3)
        memcpy(pdata->Nodes + count1 + count2, pnodes3, sizeof(TreeNode*) * count3);
    return pdata;
}


//--------------------------------------------------------------------
// ***** TreeContainer implementation

bool TreeContainer::Insert(UPInt index, TreeNode** pnodes, UPInt count)
{
    NodeData* pdata  = GetWritableData(Change_ChildInsert);
    bool      result = pdata->Children.Insert(index, pnodes, count);
    if (result)
    {
        TreeNode**pn = pnodes;
        UPInt     c  = count;
        while(c--)
        {
            SF_ASSERT((*pn)->GetParent() == 0);
            (*pn)->SetParent(this);
            (*pn)->AddRef();
            pn++;
        }
        AddToPropagate();
    }
    return result;
}

void TreeContainer::Remove(UPInt index, UPInt count)
{
    if (count == 0)
        return;
    NodeData* pdata = GetWritableData(Change_ChildRemove);
    // TBD: Need to check return code!
    TreeNode** pn = pdata->Children.GetMultipleAt(index);
    UPInt      c  = count;
    while(c--)
    {
        SF_ASSERT((*pn)->GetParent() == this);
        (*pn)->SetParent(0);
        (*pn)->Release();
        pn++;
    }
    AddToPropagate();
    //Release_Multiple(pdata->Children.GetMultipleAt(index), count);
    bool  result = pdata->Children.Remove(index, count);
    SF_ASSERT(result != false);
    result;
}


void TreeContainer::NodeData::ReleaseNodes() const
{
    if (Children.GetSize())
    {
        TreeNode** pn = Children.GetMultipleAt(0);
        UPInt      c  = Children.GetSize();
        while(c--)
        {
            SF_ASSERT((*pn)->GetParent());
            (*pn)->SetParent(0);
            (*pn)->Release();
            pn++;
        }
    }
    BaseClass::ReleaseNodes();
}

bool TreeContainer::NodeData::PropagateUp(Context::Entry* entry) const
{
    TreeContainer* pc = (TreeContainer*)entry;

    UPInt   count = Children.GetSize();
    RectF   bounds, parentBounds;
    bool    boundsEmpty = true;

    for (UPInt i = 0; i< count; i++)
    {
        TreeNode* child = Children.GetAt(i);
        const TreeNode::NodeData* childData = child->GetReadOnlyData();

        if (childData->IsVisible() && !childData->AproxParentBounds.IsEmpty())
        {
            //RectF r = childData->M.EncloseTransform(childData->AproxLocalBounds);
            if (boundsEmpty)
            {
                bounds = childData->AproxParentBounds;
                boundsEmpty = false;
            }
            else
            {
                bounds.UnionRect(&bounds, childData->AproxParentBounds);
            }
        }
    }
    
    // Doesn't work because  
    //if (HasMask())
    //    boundsEmpty = expandByMaskBounds(pc, &bounds, boundsEmpty);

    if (!boundsEmpty)
        parentBounds = M2D().EncloseTransform(bounds);

    if ((bounds != AproxLocalBounds) || (parentBounds != AproxParentBounds))
    {
        // Bounds change should cause Matrix change (for now)
        //  - needed for culling on this object to be recomputed.
        //  - TBD: May be this can be partial update.... such as parent cull check and done.
        NodeData* wdata = pc->GetWritableData(Change_AproxBounds | Change_Matrix);
        wdata->AproxLocalBounds = bounds;
        wdata->AproxParentBounds= parentBounds;

        // Mark mask parent as having changed bounds+matrix update, as it is
        // needed since currently mask is not apart of computed bounds.
        // Matrix update ensures mask bounds/bounds matrix is updated too.
        // TBD: Is this not enough? What about parent matrix change?
        // Doesn't work well, since mask node bounds don't necessarily change above
        // when child mask moves.
        //if (IsMaskNode())
        //    pc->GetParent()->GetWritableData(Change_AproxBounds | Change_Matrix);

        return IsVisible();
    }
    return false;
}

/*
// Computes mask bounds and expands bounds by it; returns the
// new boundsEmpty flag.
bool TreeContainer::NodeData::expandByMaskBounds(const TreeNode *thisNode,
                                                 RectF* bounds, bool boundsEmpty) const
{
    TreeNode*                 maskNode = GetState<MaskNodeState>()->GetNode();
    const TreeNode::NodeData* maskNodeData = maskNode->GetReadOnlyData();

    if (!maskNodeData->IsVisible() || maskNodeData->AproxParentBounds.IsEmpty())
        return boundsEmpty;

    // Transform mask into our coordinate system. Transform logic has
    // optimized special cases for direct child/sibling masks.
    Matrix2F m(maskNodeData->M);

    if (maskNode->GetParent() != thisNode)
    {
        Matrix2F m2;
        if (maskNode->GetParent() == thisNode->GetParent())
            m2.SetInverse(M);
        else
        {
            Matrix2F m3;
            maskNode->appendAncestorMatrices(&m);
            thisNode->CalcViewMatrix(&m3);
            m2.SetInverse(m3);
        }
        m.Append(m2);
    }

    RectF maskBounds = m.EncloseTransform(maskNodeData->AproxLocalBounds);

    if (!boundsEmpty)
        bounds->UnionRect(bounds, maskBounds);
    else
        *bounds = maskBounds;

    return true;
}
*/

TreeCacheNode*  TreeContainer::NodeData::updateCache(TreeCacheNode* pparent,
                                                     TreeCacheNode* pinsert,
                                                     TreeNode* pnode, UInt16 depth) const
{
    // Create & insert node if it doesn't already exist.
    TreeContainer*      pcontainer = (TreeContainer*)pnode;
    TreeCacheContainer* pcache = (TreeCacheContainer*)pnode->GetRenderData();
    if (!pcache)
    {        
        // Pass NF_MaskNode|NF_PartOfMask in constructor for consistency with Shape,
        // which needs it to create proper SortKey.
        unsigned cnodeFlags = (GetFlags() & NF_Visible);
        if (IsMaskNode())
            cnodeFlags |= NF_MaskNode|NF_PartOfMask;
        if (Is3D())
            cnodeFlags |= NF_3D;

        pcache = SF_HEAP_AUTO_NEW(pparent)
                 TreeCacheContainer(pcontainer, pparent->pRenderer2D, cnodeFlags);
        pnode->SetRenderData(pcache);
        if (!pcache)
            return 0;
    }

    // Insert node if not yet in parent. Update pRoot, Depth, pParent and child subtree. 
    pcache->UpdateInsertIntoParent(pparent, pinsert, this, depth);
    return pcache;    
}


}} // Scaleform::Render

