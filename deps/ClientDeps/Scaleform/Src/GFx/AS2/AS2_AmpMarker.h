/**********************************************************************

Filename    :   AS2/AS2_AmpMarker.h
Content     :   Implementation of marker class for AMP
Created     :   May, 2010
Authors     :   Alex Mantzaris

Notes       :   
History     :   

Copyright   :   (c) 1998-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_GAS_AMP_MARKER_H
#define INC_GAS_AMP_MARKER_H

#include "GFxConfig.h"
#include "AS2/AS2_Object.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** External Classes
class ArrayObject;
class Environment;

class AmpMarker : public Object
{
protected:
    AmpMarker(ASStringContext *sc, Object* prototype)
        : Object(sc)
    { 
        Set__proto__(sc, prototype); // this ctor is used for prototype obj only
    }

    void commonInit(Environment* env);

public:
    AmpMarker(Environment* env);
};

class AmpMarkerProto : public Prototype<AmpMarker>
{
public:
    AmpMarkerProto(ASStringContext* sc, Object* prototype, const FunctionRef& constructor);

};

//
// AMP Marker static class
//
// A constructor function object for Object
class AmpMarkerCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void AddMarker(const FnCall& fn);

public:
    AmpMarkerCtorFunction(ASStringContext *sc);

    static void GlobalCtor(const FnCall& fn);
    virtual Object* CreateNewObject(Environment*) const { return 0; }

    virtual bool GetMember(Environment* penv, const ASString& name, Value* pval);
    virtual bool SetMember(Environment *penv, const ASString& name, const Value& val, const PropFlags& flags = PropFlags());

    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif // INC_GAS_AMP_MARKER_H
