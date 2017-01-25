/**********************************************************************

Filename    :   CharacterDef.cpp
Content     :   Implementation of basic CharacterDef functionality.
Created     :   
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :  

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_CharacterDef.h"

// For GASExecutTag().
#include "GFx/GFx_MovieDef.h"

namespace Scaleform { namespace GFx {

// ***** CharacterDef

// Create a most basic instance through GFxGenericCharacter
// GFxCharacter*   CharacterDef::CreateCharacterInstance(InteractiveObject* parent, ResourceId rid,
//                                                          MovieDefImpl *pbindingImpl)
// {
//     SF_UNUSED(pbindingImpl);
//     return SF_HEAP_AUTO_NEW(parent) GFxGenericCharacter(this, parent, rid);
// }

void    CharacterDef::Display(DisplayContext &, DisplayObjectBase* pinstanceInfo)  
{ 
    SF_UNUSED(pinstanceInfo); 
}


// *****  TimelineDef

// Calls destructors on all of the tag objects.
void    TimelineDef::Frame::DestroyTags()
{
    for (unsigned i=0; i<TagCount; i++)
        pTagPtrList[i]->~ExecuteTag();
    // Clear count and pointers.
    pTagPtrList = 0;
    TagCount    = 0;
}

}} // namespace Scaleform::GFx
