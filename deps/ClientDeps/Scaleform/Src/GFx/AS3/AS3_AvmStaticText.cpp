/**********************************************************************

Filename    :   AvmStaticText.cpp
Content     :   Implementation of timeline part of StaticText.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/AS3/AS3_AvmStaticText.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "GFx/AS3/Obj/Text/AS3_Obj_Text_StaticText.h"

namespace Scaleform { namespace GFx { namespace AS3 {

AvmStaticText::AvmStaticText(StaticTextCharacter* pch)
:   AvmDisplayObj(pch)
{
}

AvmStaticText::~AvmStaticText()
{
}

}}} // SF::GFx::AS3
