/**********************************************************************

Filename    :   AS2/AS2_TextFormat.h
Content     :   TextFormat object functinality
Created     :   April 17, 2007
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_TEXTFORMAT_H
#define INC_SF_GFX_TEXTFORMAT_H

#include "GFxConfig.h"
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
#include "AS2/AS2_Action.h"
#include "GFx/Text/Text_Core.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {


// ***** Declared Classes
class TextFormatObject;
class TextFormatProto;
class TextFormatCtorFunction;


class TextFormatObject : public Object
{
protected:
    TextFormatObject(ASStringContext *psc, Object* pprototype)
        : Object(psc), mTextFormat(psc->GetHeap())
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        mTextFormat.~TextFormat();
        mParagraphFormat.~ParagraphFormat();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    Text::TextFormat      mTextFormat;
    Text::ParagraphFormat mParagraphFormat;

    TextFormatObject(ASStringContext* psc) 
        : Object(psc), mTextFormat(psc->GetHeap()) { SF_UNUSED(psc); }
    TextFormatObject(Environment* penv);

    ObjectType      GetObjectType() const   { return Object_TextFormat; }

    virtual bool    SetMember(Environment *penv, const ASString& name, 
        const Value& val, const PropFlags& flags = PropFlags());

    void SetTextFormat(ASStringContext* psc, const Text::TextFormat& textFmt);
    void SetParagraphFormat(ASStringContext* psc, const Text::ParagraphFormat& paraFmt);
};

class TextFormatProto : public Prototype<TextFormatObject>
{
public:
    TextFormatProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void GetTextExtent(const FnCall& fn);
};

class TextFormatCtorFunction : public CFunctionObject
{
public:
    TextFormatCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT

#endif //TEXTFORMAT

