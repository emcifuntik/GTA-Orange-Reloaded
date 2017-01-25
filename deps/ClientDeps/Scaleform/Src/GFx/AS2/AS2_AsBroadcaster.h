/**********************************************************************

Filename    :   AS2/AS2_AsBroadcaster.h
Content     :   Implementation of AsBroadcaster class
Created     :   October, 2006
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_SF_GFX_ASBROADCASTER_H
#define INC_SF_GFX_ASBROADCASTER_H

#include "AS2/AS2_Action.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class AsBroadcaster;
class AsBroadcasterProto;
class AsBroadcasterCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class AsBroadcaster : public Object
{
protected:
    void commonInit (Environment* penv);
    
    AsBroadcaster (ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
public:
    AsBroadcaster (Environment* penv);

    virtual ObjectType GetObjectType() const   { return Object_AsBroadcaster; }

    static bool Initialize(ASStringContext* psc, ObjectInterface* pobj);
    static bool InitializeProto(ASStringContext* psc, ObjectInterface* pobj);
    static bool InitializeInstance(ASStringContext* psc, ObjectInterface* pobj);
    static bool AddListener(Environment* penv, ObjectInterface* pthis, ObjectInterface* plistener);
    static bool RemoveListener(Environment* penv, ObjectInterface* pthis, ObjectInterface* plistener);
    static bool BroadcastMessage(Environment* penv, 
                                 ObjectInterface* pthis, 
                                 const ASString& eventName, 
                                 int nArgs, 
                                 int firstArgBottomIndex);
    
    class InvokeCallback
    {
    public:
        virtual ~InvokeCallback() {}
        virtual void Invoke(Environment* penv, ObjectInterface* pthis, const FunctionRef& method) =0;
    };
    static bool BroadcastMessageWithCallback
        (Environment* penv, ObjectInterface* pthis, const ASString& eventName, InvokeCallback* pcallback);
};

class AsBroadcasterProto : public Prototype<AsBroadcaster>
{
public:
    AsBroadcasterProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void AddListener(const FnCall& fn);
    static void BroadcastMessage(const FnCall& fn);
    static void RemoveListener(const FnCall& fn);
};

class AsBroadcasterCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void Initialize (const FnCall& fn);
public:
    AsBroadcasterCtorFunction (ASStringContext *psc);

    static void GlobalCtor(const FnCall& fn);

    virtual Object* CreateNewObject(Environment* penv) const;    

    static FunctionRef Register(GlobalContext* pgc);
};


}}} //SF::GFx::AS2
#endif // INC_SF_GFX_ASBROADCASTER_H
