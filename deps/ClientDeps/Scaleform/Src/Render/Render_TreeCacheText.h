/**********************************************************************

Filename    :   Render_TreeCacheText.h
Content     :   TreeText Cache implementation header
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

#ifndef SF_Render_TreeCacheText_H
#define SF_Render_TreeCacheText_H

#include "Render_TreeText.h"
#include "Render_TreeCacheNode.h"
#include "Render_TextMeshProvider.h"

namespace Scaleform { namespace Render {

class TextMeshProvider;

//------------------------------------------------------------------------
// ***** TreeCacheText

class TreeCacheText : public TreeCacheMeshBase
{
friend class TreeCacheShape;
public:
    // Construct TreeCacheShapeLayer.
    //  pnode may be null if we are a child under 
    TreeCacheText(TreeText* pnode, Renderer2DImpl* prenderer2D, unsigned flags);
    ~TreeCacheText();
    

    // Inlines/node types overloaded in derived class for proper return type.
    typedef TreeText::NodeData NodeData;
    inline TreeText*       GetNode() const { return (TreeText*)pNode; }
    inline const NodeData* GetNodeData() const { return GetNode()->GetDisplayData(); }


    void            UpdateCache(TreeCacheContainer* pparent, UInt16 depth);

    virtual void    UpdateChildSubtree(const TreeNode::NodeData*, UInt16 depth);
    
    // For nested layer, these will receive matrix and node from TreeCacheShape.
    virtual void    UpdateTransform(const TreeNode::NodeData* pdata,
                                    const TransformArgs& t, TransformFlags flags);
    virtual void    HandleChanges(unsigned changeBits);

    virtual void    propagateMaskFlag(unsigned partOfMask);

    //virtual RectF   CalcScale9GridBounds(const Matrix2F &m);
    //virtual void    UpdateScale9GridShapeMatrix();

    // Assigns an inserts a mesh into a specified location.
    virtual MeshBase* GetMesh() { SF_ASSERT(0); return 0; }
    virtual void      RemoveMesh() {}

    // TBD: Used by TextPrimitiveBundle instead of GetMesh()..
    TextMeshProvider* GetMeshProvider();
    TextMeshProvider* CreateMeshProvider();

    inline Bundle* GetBundle() const
    {
        return SorterShapeNode.pBundle.GetPtr();
    }

    // Used by TextPrimitiveBundle.
    TreeCacheText*   pNextNoBatch;

protected:
    TextMeshProvider TMProvider;
};


}} // Scaleform::Render

#endif
