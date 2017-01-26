/**********************************************************************

PublicHeader:   Render
Filename    :   Render_TreeNode.h
Content     :   Declarations of different rendering tree nodes.
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

#ifndef INC_SF_Render_TreeNode_H
#define INC_SF_Render_TreeNode_H

#include "Render_Context.h"

#include "Render_CxForm.h"
#include "Render_Matrix2x4.h"
#include "Render_Matrix3x4.h"
#include "Render_Containers.h"
#include "Render_Gradients.h"
#include "Render_Viewport.h"
#include "Render_States.h"

namespace Scaleform { namespace Render {

// A transformable node in the tree
class TreeContainer;

// Declarations needed for TreeNode::NodeData::updateCache.
class TreeCacheNode;
class TreeCacheContainer;


class TreeNode : public Context::Entry
{
public:

    class NodeData : public Context::EntryData
    {
    private:
        // This can be interpreted as either a 2D or 3D matrix (indicated by Is3D())
        // Use M3D(), M2D() accessors, (can't use unions since Matrix classes contain non-trivial ctors)
        Matrix3F    M34;          

    public:
        NodeData(Context::EntryData::EntryType type = ET_Base)
            : Context::EntryData(type, NF_Visible) { }

        StateBag    States;        
        Cxform      Cx;
        // Propagated LocalBounds approximation. This is not precise since it uses
        // cumulatively enclosed rectangles (not the precise matrix). Intended for
        // use with hit-testing, culling, masking and batch bounds checks.
        RectF    AproxLocalBounds; 
        RectF    AproxParentBounds;
        

        // updateCache - Internal function used for node implementation. Needs to handle 3 cases:
        //  1. Renderer cache node is not created. Create the proper node and process/cache subtree,
        //     adding items into DP.
        //  2. Render cache node created, but has no parent. This means cached node has been removed and
        //     is now being re-added. pParent, pRoot and other variables need to be initialized, DPs inserted.
        //  3. Render node is created and is part of parent. In this case, we are verifying a child node
        //     of a subtree after an ancestor (ex, parent) was re-added. We don't need to do insert/remove,
        //     but we do need to update pRoot/Depth and insert DPs.
        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const = 0;

        virtual void ReleaseNodes() const;

        void  SetVisible(bool visible)
        {
            Flags = (UInt16)((Flags & ~NF_Visible) | (visible ? NF_Visible : 0));
        }
        bool  IsVisible() const     { return (Flags & NF_Visible) != 0; }

        bool  IsMaskNode() const    { return (Flags & NF_MaskNode) != 0; }
        bool  HasMask() const       { return (Flags & NF_HasMask) != 0; }
        bool  Is3D() const          { return (Flags & NF_3D) != 0; }
        void  Clear3D()             { Flags &= ~NF_3D; }  
        unsigned GetFlags() const   { return Flags; }

        template<class S>
        const S* GetState() const   { return States.GetState<S>(); }

        TreeNode* GetMaskOwner() const
        {
            const Internal_MaskOwnerState* ownerState = GetState<Internal_MaskOwnerState>();
            return ownerState ? ownerState->GetMaskOwner() : 0;
        }       

        // Matrix 2D/3D accessors
        const Matrix2F &M2D() const { return *(Matrix2F *)&M34; }
        Matrix2F &M2D()             { return *(Matrix2F *)&M34; }

        const Matrix3F &M3D() const { return M34; }
        Matrix3F &M3D()             { return M34; }        

        void SetMatrix(const Matrix2F &mat)     { *(Matrix2F *)&M34 = mat; }
        void SetMatrix3D(const Matrix3F &mat)   { M34 = mat; Flags |= NF_3D; }
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    void            SetMatrix(const Matrix2F& m); 
    const Matrix2F&   M2D() const
    {
        return GetReadOnlyData()->M2D();
    }

    void            SetMatrix3D(const Matrix3F& m); 
    const Matrix3F& M3D() const
    {
        return GetReadOnlyData()->M3D();
    }

    void        SetViewMatrix3D(const Matrix3F& m);
    bool GetViewMatrix3D(Matrix3F *mat) const
    {        
        const ViewMatrix3DState* state = GetState<ViewMatrix3DState>();
        if (!state)
            return false;
        *mat = Matrix3F(state->GetViewMatrix3D()->M);
        return true;
    }

    void        SetProjectionMatrix3D(const Matrix4F& m);
    bool GetProjectionMatrix3D(Matrix4F *mat) const
    {        
        const ProjectionMatrix3DState* state = GetState<ProjectionMatrix3DState>();
        if (!state)
            return false;

        *mat = Matrix4F(state->GetProjectionMatrix3D()->M);
        return true;
    }

    // Calculates view matrix of this tree node; i.e. the matrix that would
    // transform local coordinates to the coordinates of the root tree node, which
    // typically matches viewport.
    void            CalcViewMatrix(Matrix2F *m) const;


    void            SetCxform(const Cxform& cx)
    {
        NodeData* pdata = GetWritableData(Change_CxForm);
        pdata->Cx = cx;
    }
    const Cxform&   GetCxform() const
    {        
        return GetReadOnlyData()->Cx;
    }

    void            SetVisible(bool visible);
    bool            IsVisible() const
    {
        return GetReadOnlyData()->IsVisible();
    }
    unsigned        GetFlags() const
    {
        return GetReadOnlyData()->GetFlags();
    }

    // General state access.
    const State* GetState(StateType state) const;
    void         SetState(State& s, unsigned changeFlags);
    bool         RemoveState(StateType state, unsigned changeFlags);

    template<class S>
    const S*     GetState() const
    {
        return static_cast<const S*>(GetState(S::GetType_Static()));
    }

    void        SetScale9Grid(const RectF& rect);
    RectF       GetScale9Grid() const
    {        
        const Scale9State* state = GetState<Scale9State>();
        return state ? state->GetRect() : RectF(0);
    }
    
    void        SetBlendMode(BlendMode mode);
    BlendMode   GetBlendMode() const
    {
        const BlendState* state = GetState<BlendState>();
        return state ? state->GetBlendMode() : Blend_None;
    }

    void        SetMaskNode(TreeNode* node);
    TreeNode*   GetMaskNode() const
    {
        const MaskNodeState* state = GetState<MaskNodeState>();
        return state ? state->GetNode() : 0;
    }

    // Treat this TreeNode as 2D only
    void        Clear3D()           { GetWritableData(Change_Matrix)->Clear3D(); }
    // Determine if this TreeNode has a 3D transform
    bool        Is3D() const        { return GetReadOnlyData()->Is3D(); }
    // Determine if this TreeNode is itself a mask node (root of mask applied elsewhere).
    bool        IsMaskNode() const  { return GetReadOnlyData()->IsMaskNode(); }
    // Determine if this node has a mask applied (call GetMaskNode for actual node).
    bool        HasMask() const     { return GetReadOnlyData()->HasMask(); }

    bool        IsPartOfMask()
    {
        TreeNode* node = this;
        while(node)
        {
            if (node->IsMaskNode())
                return true;
            node = node->GetParent();
        }
        return false;
    }

    inline TreeNode*  GetParent() const;


    // Internal CalcViewMatrix helper.
    void        appendAncestorMatrices(Matrix2F *m) const;

protected:

    friend class NodeData;
    bool        setThisAsMaskOwner(NodeData* thisData, TreeNode* node);
    static bool removeThisAsMaskOwner(const NodeData* thisData);

    // Helpers for sprite, etc
    static void AddRef_Multiple(TreeNode** pnodes, UPInt count)
    {
        while(count--) (*(pnodes++))->AddRef();
    }
    static void Release_Multiple(TreeNode** pnodes, UPInt count)
    {
        while(count--) (*(pnodes++))->Release();
    }
};



//--------------------------------------------------------------------
// Copy-on-write array used for nodes.

class TreeNodeArray
{
public:

    struct ArrayData
    {
        volatile int    RefCount;
        UPInt           Size;
        TreeNode*       Nodes[1];

        void AddRef()
        {
            AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, 1);
        }
        void Release()
        {
            if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
                SF_FREE(this);
        }
    };

    static ArrayData NullArrayData;
    ArrayData*       pData;

    ArrayData*  AllocData2(TreeNode** pnodes1, UPInt count1,
                           TreeNode** pnodes2, UPInt count2,
                           UPInt extra = 0);
    
    ArrayData*  AllocData3(TreeNode** pnodes1, UPInt count1,
                           TreeNode** pnodes2, UPInt count2,
                           TreeNode** pnodes3, UPInt count3);

public:

    TreeNodeArray()
    {
        pData = &NullArrayData;
        pData->AddRef();
    }
    TreeNodeArray(const TreeNodeArray& src)
    {
        src.pData->AddRef();
        pData = src.pData;
    }
    ~TreeNodeArray()
    {
        pData->Release();
    }

    void operator = (const TreeNodeArray& src)
    {
        src.pData->AddRef();
        pData->Release();
        pData = src.pData;
    }

    bool        Insert(UPInt index, TreeNode** pnodes, UPInt count)
    {
        SF_ASSERT(index <= pData->Size);
        ArrayData* pnewData = AllocData3(pData->Nodes, index, pnodes, count,
                                         pData->Nodes + index, pData->Size - index);
        if (pnewData)
        {
            pData->Release();
            pData = pnewData;
            return true;
        }
        return false;
    }

    bool        Remove(UPInt index, UPInt count = 1)
    {
        SF_ASSERT(index <= pData->Size);
        SF_ASSERT((count + index) <= pData->Size);

        ArrayData* pnewData = AllocData2(pData->Nodes, index,
                                         pData->Nodes + index + count,
                                         pData->Size - index - count);
        if (pnewData)
        {
            pData->Release();
            pData = pnewData;
            return true;
        }
        return false;
    }
    
    TreeNode* GetAt(UPInt index) const
    {
        SF_ASSERT(index < GetSize());
        return pData->Nodes[index];
    }   
    UPInt       GetSize() const
    {
        return pData->Size;
    }

    TreeNode* operator [] (UPInt index) const
    {
        SF_ASSERT(index < GetSize());
        return pData->Nodes[index];
    }

    TreeNode** GetMultipleAt(UPInt index) const
    {
        SF_ASSERT(index < GetSize());
        return &pData->Nodes[index];
    }  
};


//--------------------------------------------------------------------
// ***** TreeContainer


class TreeContainer : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
        typedef ContextData_ImplMixin<NodeData, TreeNode::NodeData> BaseClass;
    protected:
        NodeData(Context::EntryData::EntryType type)
            :  BaseClass(type) { }
    public:
        NodeData() : BaseClass(ET_Container) { }
        
        TreeNodeArray Children;


        // Needed for Entry implementation.
        virtual void        ReleaseNodes() const;
        virtual bool        PropagateUp(Entry* entry) const;

        // Computes mask bounds and expands bounds by it; returns the
        // new boundsEmpty flag.
     //   bool                expandByMaskBounds(const TreeNode *thisNode,
     //                                          RectF* bounds, bool boundsEmpty) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    bool        Insert(UPInt index, TreeNode** pnodes, UPInt count);
    bool        Insert(UPInt index, TreeNode* pnode) { return Insert(index, &pnode, 1); }
    void        Remove(UPInt index, UPInt count);
    bool        Add(TreeNode* node) { return Insert(GetSize(), node); }

    // Child access

    TreeNode* GetAt(UPInt index) const
    {     
        return GetReadOnlyData()->Children.GetAt(index);
    }
    UPInt       GetSize() const
    {        
        return GetReadOnlyData()->Children.GetSize();
    }
   
};

inline TreeNode* TreeNode::GetParent() const
{
    return static_cast<TreeNode*>(Context::Entry::GetParent());
}


//--------------------------------------------------------------------

// Special root node for rendering.
//
// TreeRoot can optionally apply it own Viewport and clear it with background color.
// Viewport is only applied if it is valid (off by default); background is only
// cleared if its alpha channel is not 0.

class TreeRoot : public TreeContainer
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeContainer::NodeData>
    {
    public:
        NodeData() :  ContextData_ImplMixin<NodeData, TreeContainer::NodeData>(ET_Root) { }
        
        // Viewport and background color for it.
        Viewport VP;
        Color    BGColor;

        Color   GetBackgroundColor() const { return BGColor; }
        bool    HasViewport() const        { return VP.IsValid(); }

    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    // Returns 'true' of TreeRoot has a valid viewport that is used.

    bool HasViewport() const
    {
        return GetReadOnlyData()->HasViewport();
    }
    const Viewport& GetViewport() const
    {
        return GetReadOnlyData()->VP;
    }
    Color           GetBackgroundColor() const
    {
        return GetReadOnlyData()->GetBackgroundColor();
    }

    // Sets a viewport to use for rendering; if this is set, BeginDisplay will be
    // called with these values.
    void SetViewport(const Viewport& vp)
    {
        NodeData* data = GetWritableData(Change_Viewport);
        data->VP = vp;
    }   
    void SetBackgroundColor(const Color& color)
    {
        NodeData* data = GetWritableData(Change_Viewport);
        data->BGColor = color;
    }
};


// Convenience typedef for DisplayHandle for TreeRoot.
typedef DisplayHandle<TreeRoot> TreeRootDisplayHandle;


}} // Scaleform::Render

#endif
