/**********************************************************************

Filename    :   AS2_StringObject.h
Content     :   Implementation of AS2 String class
Created     :   
Authors     :   Maxim Shemanarev, Artem Bolgar

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_SF_GFX_STRINGOBJECT_H
#define INC_SF_GFX_STRINGOBJECT_H

#include "AS2/AS2_Action.h"
#include "AS2/AS2_ObjectProto.h"

namespace Scaleform { namespace GFx { namespace AS2 {

// ***** Declared Classes
class StringObject;
class StringProto;
class StringCtorFunction;

// ***** External Classes
class ArrayObject;
class Environment;



class StringObject : public Object
{
protected:
    ASString sValue;

    void commonInit (Environment* penv);
    
    StringObject(ASStringContext *psc, Object* pprototype) :
        Object(psc), sValue(psc->GetBuiltin(ASBuiltin_empty_))
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }
#ifdef GFX_AS_ENABLE_GC
protected:
    virtual void Finalize_GC()
    {
        sValue.~ASString();
        Object::Finalize_GC();
    }
#endif //SF_NO_GC
public:
    StringObject(Environment* penv);
    StringObject(Environment* penv, const ASString& val);

    bool                GetMemberRaw(ASStringContext *psc, const ASString& name, Value* val);
    const char*         GetTextValue(Environment* =0) const;
    ObjectType          GetObjectType() const   { return Object_String; }

    const ASString&     GetString() const               { return sValue; }
    void                SetString(const ASString& val) { sValue = val; }

    virtual void        SetValue(Environment* penv, const Value&);
    virtual Value       GetValue() const;
};

class StringProto : public Prototype<StringObject>
{
public:
    StringProto(ASStringContext *psc, Object* pprototype, const FunctionRef& constructor);

    static void GlobalCtor(const FnCall& fn);

    static void StringCharAt(const FnCall& fn);
    static void StringCharCodeAt(const FnCall& fn);
    static void StringConcat(const FnCall& fn);
    static void StringIndexOf(const FnCall& fn);
    static void StringLastIndexOf(const FnCall& fn);
    static void StringLocaleCompare(const FnCall& fn);
    static void StringSlice(const FnCall& fn);
    static void StringSplit(const FnCall& fn);        
    static void StringSubstr(const FnCall& fn);
    static void StringSubstring(const FnCall& fn);
    static void StringToLowerCase(const FnCall& fn);
    static void StringToString(const FnCall& fn);
    static void StringToUpperCase(const FnCall& fn);
    static void StringValueOf(const FnCall& fn);

    // Helper methods.
    static ASString StringSubstring(const ASString& str, 
                                     int start, int length);
    static Ptr<ArrayObject> StringSplit(Environment* penv, const ASString& str,
                                            const char* delimiters, int limit = 0x3FFFFFFF);

    // Creates a ASString based on two char* pointers
    static ASString CreateStringFromCStr(ASStringContext* psc, const char* start, const char* end = 0);
};

class StringCtorFunction : public CFunctionObject
{
    static const NameFunction StaticFunctionTable[];

    static void StringFromCharCode(const FnCall& fn);
public:
    StringCtorFunction(ASStringContext *psc);

    virtual Object* CreateNewObject(Environment *penv) const;

    static void GlobalCtor(const FnCall& fn);
    static FunctionRef Register(GlobalContext* pgc);
};

}}} // SF::GFx::AS2

#endif // INC_SF_GFX_STRINGOBJECT_H
