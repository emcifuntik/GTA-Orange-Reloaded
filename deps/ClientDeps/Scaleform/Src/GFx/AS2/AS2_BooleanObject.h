/**********************************************************************

Filename    :   AS2/AS2_BooleanObject.h
Content     :   Boolean object functinality
Created     :   April 10, 2006
Authors     :   

Notes       :   
History     :   

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_BOOLEAN_H
#define INC_SF_GFX_BOOLEAN_H

#include "AS2/AS2_Action.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class BooleanObject;
class BooleanProto;
class BooleanCtorFunction;


class BooleanObject : public Object
{
protected:
    bool bValue;
    
    void CommonInit (Environment* penv);

    BooleanObject(ASStringContext* psc, Object* pprototype) : 
        Object(psc), bValue (0) 
    { 
        Set__proto__(psc, pprototype);
    }
public:
    BooleanObject(Environment* penv);
    BooleanObject(Environment* penv, bool val);
    virtual const char* GetTextValue(Environment* penv = 0) const;

    ObjectType      GetObjectType() const   { return Object_Boolean; }

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value    GetValue() const;

    //
    // This method is used to invoke some methods for primitive number,
    // like it is possible to invoke toString (radix) or valueOf even for non-object Boolean.
    //
    static bool InvokePrimitiveMethod(const FnCall& fnCall, const ASString& methodName);
};

class BooleanProto : public Prototype<BooleanObject>
{
public:
    BooleanProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);
};

class BooleanCtorFunction : public CFunctionObject
{
public:
    BooleanCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment* penv) const;        

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2
#endif //BOOLEAN

