/**********************************************************************

Filename    :   AS2/AS2_Color.h
Content     :   Text character implementation
Created     :   
Authors     :   

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_COLOR_H
#define INC_SF_GFX_COLOR_H

#include "GFxConfig.h"
#ifdef GFX_AS2_ENABLE_COLOR
#include "GFx/GFx_InteractiveObject.h"
#include "AS2/AS2_Object.h"

#include "AS2/AS2_Action.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class ColorObject;
class ColorProto;


// ActionScript Color object

class ColorObject : public Object
{
    friend class ColorProto;
    friend class ColorCtorFunction;

    WeakPtr<InteractiveObject> pCharacter;
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        pCharacter.~WeakPtr<InteractiveObject>();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
protected:
    ColorObject(ASStringContext *psc, Object* pprototype) // for prototype 
        : Object(psc)
    { 
        Set__proto__(psc, pprototype);
    }
public:
    ColorObject(Environment* penv, InteractiveObject *pcharacter)
        : Object(penv), pCharacter(pcharacter)
    {        
        Set__proto__(penv->GetSC(), penv->GetPrototype(ASBuiltin_Color));
    }
    void               SetTarget(InteractiveObject *pcharacter) { pCharacter = pcharacter; }
    virtual ObjectType GetObjectType() const   { return Object_Color; }
};

class ColorProto : public Prototype<ColorObject>
{
public:
    ColorProto(ASStringContext *psc, Object* prototype, const FunctionRef& constructor);

    // NOTE: SetTransform allows for negative color transforms (inverse mul/color subtract?).
    // TODO: Such transforms probably won't work in renderer -> needs testing!

    static void SetTransform(const FnCall& fn);
    static void GetTransform(const FnCall& fn);
    static void SetRGB(const FnCall& fn);
    static void GetRGB(const FnCall& fn);
};

class ColorCtorFunction : public CFunctionObject
{
public:
    ColorCtorFunction(ASStringContext *psc) 
        : CFunctionObject(psc, GlobalCtor) {}

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};
#endif //#ifdef GFX_AS2_ENABLE_COLOR
}}} //SF::GFx::AS2

#endif // INC_SF_GFX_COLOR_H

