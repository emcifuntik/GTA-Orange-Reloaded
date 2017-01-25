/**********************************************************************

Filename    :   AvmStaticText.h
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
#ifndef INC_AS3_AvmStaticText_H
#define INC_AS3_AvmStaticText_H

#include "GFx/AS3/AS3_AvmDisplayObj.h"
#include "GFx/GFx_StaticText.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes

namespace AS3 {

// An AS3-dependable part of displaylist's DisplayObject.
class AvmStaticText : public AvmDisplayObj
{
public:
    // Constructor.
    AvmStaticText(StaticTextCharacter*);
    ~AvmStaticText();

    virtual const char* GetDefaultASClassName() const { return "flash.text.StaticText"; }
    virtual String&     GetASClassName(String* className) const
    {
        SF_ASSERT(className);
        // Always create "flash.display.Bitmap" class, since
        // the custom AS class is attached to BitmapData and not to Bitmap.
        *className = AvmStaticText::GetDefaultASClassName();
        return *className;
    }
};

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmStaticText_H
