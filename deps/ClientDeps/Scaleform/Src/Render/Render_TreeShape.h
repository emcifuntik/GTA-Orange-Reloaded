/**********************************************************************

Filename    :   TreeShape.h
Content     :   TreeShape represents a Flash shape node in a tree.
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef SF_Render_TreeShape_H
#define SF_Render_TreeShape_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_List.h"
#include "Render_TreeNode.h"
#include "Render_ShapeMeshProvider.h"

namespace Scaleform { namespace Render {


class TreeShape : public TreeNode
{
public:

    class NodeData : public ContextData_ImplMixin<NodeData, TreeNode::NodeData>
    {
    public:
        NodeData() : ContextData_ImplMixin<NodeData, TreeNode::NodeData>(ET_Shape), MorphRatio(0)
        { }
        NodeData(ShapeMeshProvider* meshProvider)
            :  ContextData_ImplMixin<NodeData, TreeNode::NodeData>(ET_Shape),
              pMeshProvider(meshProvider), MorphRatio(0)
        { }
        ~NodeData()
        {
        }
        Ptr<ShapeMeshProvider> pMeshProvider;
        float                  MorphRatio;

        virtual bool            PropagateUp(Entry* entry) const;

        virtual TreeCacheNode*  updateCache(TreeCacheNode* pparent, TreeCacheNode* pinsert,
                                            TreeNode* pnode, UInt16 depth) const;
    };

    SF_RENDER_CONTEXT_ENTRY_INLINES(NodeData)

    ShapeMeshProvider* GetShape() const 
    { 
        return GetReadOnlyData()->pMeshProvider;
    }

    void SetShape(ShapeMeshProvider* pshape) 
    { 
        GetWritableData(Change_Shape)->pMeshProvider = pshape;
        AddToPropagate();
    }

    float GetMorphRatio() const
    {
        return GetReadOnlyData()->MorphRatio;
    }

    void SetMorphRatio(float ratio)
    {
        GetWritableData(Change_MorphRatio)->MorphRatio = ratio;
    }


};


}} // Scaleform::Render

#endif
