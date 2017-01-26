/**********************************************************************

Filename    :   Render_States.cpp
Content     :   Implementation of different render states used for render
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

#include "Render_States.h"
#include "Render_TreeNode.h"

namespace Scaleform { namespace Render {


State::Interface_Value StateNone_InterfaceImpl(State_None);

void StateData::InitDefaultStates_ForceLink()
{
}

//--------------------------------------------------------------------

// ***** BlendState
BlendState::Interface BlendState::InterfaceImpl(State_BlendMode);

// ***** Scale9State
Scale9State::Interface Scale9State::InterfaceImpl(State_Scale9);

// ***** ViewMatrix3DState
ViewMatrix3DState::Interface ViewMatrix3DState::InterfaceImpl(State_ViewMatrix3D);

// ***** ProjectionMatrix3DState
ProjectionMatrix3DState::Interface ProjectionMatrix3DState::InterfaceImpl(State_ProjectionMatrix3D);

// ***** MaskNodeState

MaskNodeState::Interface MaskNodeState::InterfaceImpl;

void MaskNodeState::Interface::AddRef(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->AddRef();
}
void MaskNodeState::Interface::Release(void* data, RefBehaviour b)
{
    if (b != Ref_NoTreeNode)
        ((TreeNode*)data)->Release();
}

Internal_MaskOwnerState::Interface Internal_MaskOwnerState::InterfaceImpl(State_Internal_MaskOwner);


}} // Scaleform::Render
