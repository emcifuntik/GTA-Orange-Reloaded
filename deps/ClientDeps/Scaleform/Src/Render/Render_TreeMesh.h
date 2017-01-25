/**********************************************************************

Filename    :   TreeMesh.h
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

#ifndef INC_SF_Render_TreeMesh_H
#define INC_SF_Render_TreeMesh_H

#include "Render_TreeNode.h"
#include "Render_Primitive.h"


namespace Scaleform { namespace Render {


class TreeMesh : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
    public:
        NodeData() : ContextData_ImplMixin<NodeData, TreeNode::NodeData>(ET_Mesh)
        { }
        NodeData(MeshProvider* provider)
            : ContextData_ImplMixin<NodeData, TreeNode::NodeData>(ET_Mesh),
              pShape(provider)
        { }
        ~NodeData()
        {
        }
        Ptr<MeshProvider> pShape;

        virtual bool            PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    inline MeshProvider* GetShape() const 
    { 
        return GetReadOnlyData()->pShape; 
    }

    inline void       SetShape(MeshProvider* pshape) 
    { 
        GetWritableData(Change_Shape)->pShape = pshape;
        AddToPropagate();
    }
};


}} // Scaleform::Render

#endif
