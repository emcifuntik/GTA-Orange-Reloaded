/**********************************************************************

PublicHeader:   Render
Filename    :   Render_States.h
Content     :   Declarations of different render states used for render
                tree node configuration.
Created     :   June 24, 2010
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_States_H
#define INC_SF_Render_States_H

#include "Render_StateBag.h"
#include "Render_Scale9Grid.h"
#include "Render_Matrix3x4.h"
#include "Render_Matrix4x4.h"

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
// ***** BlendState

// Blend modes supported by Flash.
enum BlendMode
{
    Blend_None          = 0,    // Same as Normal, but means that BlendMode was not set.
    Blend_Normal        = 1,
    Blend_Layer         = 2,
    Blend_Multiply      = 3,
    Blend_Screen        = 4,
    Blend_Lighten       = 5,
    Blend_Darken        = 6,
    Blend_Difference    = 7,
    Blend_Add           = 8,
    Blend_Subtract      = 9,
    Blend_Invert        = 10,
    Blend_Alpha         = 11,
    Blend_Erase         = 12,
    Blend_Overlay       = 13,
    Blend_HardLight     = 14
};

class BlendState : public State
{
public:    
    typedef State::Interface_Value Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_BlendMode; }

    BlendState(BlendMode mode) : State(&InterfaceImpl, (void*)mode, NoAddRef) { }

    BlendMode GetBlendMode() const    { return (BlendMode)DataValue; }
    // TBD: Should this be read-only?
    //void      SetBlendMode(BlendMode mode)  { DataValue = mode; }
};


//--------------------------------------------------------------------
// ***** Scale9State

class Scale9State : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_Scale9; }

    Scale9State(Scale9GridRect* sgrect) : State(&InterfaceImpl, (void*)sgrect)
    { SF_ASSERT(sgrect);}

    RectF GetRect() const
    { 
        Scale9GridRect* sgrect = (Scale9GridRect*)GetData();
        return sgrect->Scale9;
    }
};

//--------------------------------------------------------------------
// ***** MaskNodeState

class TreeNode;

class MaskNodeState : public State
{
public:
    class Interface : public State::Interface
    {
    public:
        Interface() : State::Interface(State_MaskNode) { }
        virtual void AddRef(void* data, RefBehaviour b);
        virtual void Release(void* data, RefBehaviour b);
    };
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_MaskNode; }

    MaskNodeState(TreeNode* node) : State(&InterfaceImpl, (void*)node)
    { SF_ASSERT(node);}

    TreeNode* GetNode() const { return (TreeNode*)pData; }
};

//--------------------------------------------------------------------
// ***** ViewMatrix3DState

class ViewMatrix3DState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_ViewMatrix3D; }

    ViewMatrix3DState(const Matrix3F* mat3D) : State(&InterfaceImpl, (void*)mat3D)
    { SF_ASSERT(mat3D);}

    const Matrix3FRef *GetViewMatrix3D() const    { return (Matrix3FRef*)GetData(); }
};

//--------------------------------------------------------------------
// ***** ProjectionMatrix3DState

class ProjectionMatrix3DState : public State
{
public:
    typedef State::Interface_RefCountImpl Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_ProjectionMatrix3D; }

    ProjectionMatrix3DState(const Matrix4F* mat3D) : State(&InterfaceImpl, (void*)mat3D)
    { SF_ASSERT(mat3D);}

    const Matrix4FRef *GetProjectionMatrix3D() const    { return (Matrix4FRef*)GetData(); }
};

// MaskOwner - This stores a pointer to mask owner TreeNode, without AddRef.
class Internal_MaskOwnerState : public State
{
public:    
    typedef State::Interface_Value Interface;
    static Interface InterfaceImpl;
    static StateType GetType_Static() { return State_Internal_MaskOwner; }

    Internal_MaskOwnerState(TreeNode* owner) : State(&InterfaceImpl, (void*)owner, NoAddRef) { }

    TreeNode* GetMaskOwner() const  { return (TreeNode*)DataValue; }
};


}} // Scaleform::Render

#endif
