/**********************************************************************

Filename    :   GFxASGlowFilter.h
Content     :   GlowFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_GASGlowFilter_H
#define INC_GASGlowFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "AS2/AS2_BitmapFilter.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class GlowFilterObject;
class GlowFilterProto;
class GlowFilterCtorFunction;


// 
// Actionscript GlowFilter object declaration
//

class GlowFilterObject : public BitmapFilterObject
{
    friend class GlowFilterProto;
    friend class GlowFilterCtorFunction;

protected:

//     GlowFilterObject();
//     GlowFilterObject(const GlowFilterObject& obj) : BitmapFilterObject(obj) {}
    GlowFilterObject(ASStringContext *psc, Object* pprototype)
        : BitmapFilterObject(psc, pprototype)
    { 
    }

    virtual             ~GlowFilterObject();

public:

    GlowFilterObject(Environment* penv);

    virtual ObjectType  GetObjectType() const { return Object_GlowFilter; }

    virtual bool        SetMember(Environment* penv, 
        const ASString& name, const Value& val, 
        const PropFlags& flags = PropFlags());

    virtual bool        GetMember(Environment* penv, 
        const ASString& name, Value* val);
};


class GlowFilterProto : public Prototype<GlowFilterObject>
{
public:
    GlowFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);

    static const NameFunction FunctionTable[];

    //
    // Default BitmapFilter object functions
    //
    static void         Clone(const FnCall& fn);
};


class GlowFilterCtorFunction : public CFunctionObject
{
public:
    GlowFilterCtorFunction(ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment* penv) const;

    static FunctionRef Register(GlobalContext* pgc);
};


}}} //SF::GFx::AS2
#endif  // SF_NO_FXPLAYER_AS_FILTERS

#endif  // INC_GASGlowFilter_H
