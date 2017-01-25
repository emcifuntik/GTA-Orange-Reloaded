/**********************************************************************

Filename    :   AS3_Object.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_Object_H
#define INC_AS3_Object_H


#include "AS3_Value.h"
#include "AS3_Slot.h"
#include "Kernel/SF_AutoPtr.h"

///////////////////////////////////////////////////////////////////////////////
#define MEMBER(clss, mbr) (((clss *)1)->mbr)
#define OFFSETOF(clss, mbr) ((UPInt)&(MEMBER(clss, mbr)) - 1)
#define NUMBEROF(a) sizeof(a) / sizeof(a[0])

//#define CONSTRUCT_IN_TRAITS
//#define VERSION_AIR

namespace Scaleform { namespace GFx { namespace AS3 
{
    
///////////////////////////////////////////////////////////////////////////
typedef ArrayLH<Abc::Multiname> MultinamesType;

///////////////////////////////////////////////////////////////////////////
namespace Instances
{
    class Object;
    class Array;
    class Function;
}

///////////////////////////////////////////////////////////////////////////
template <typename T>
inline
typename Clean<T>::type DefaultValue(StringManager&)
{
    return NULL;
}

template <>
inline
void DefaultValue<void>(StringManager&)
{
}

template <>
inline
bool DefaultValue<bool>(StringManager&)
{
    return false;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM)
template <>
inline
int DefaultValue<int>(StringManager&)
{
    return 0;
}
#endif
template <>
inline
SInt32 DefaultValue<SInt32>(StringManager&)
{
	return 0;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_OS_WII) && !defined(SF_CC_ARM)
template <>
inline
unsigned int DefaultValue<unsigned int>(StringManager&)
{
    return 0;
}
#endif
template <>
inline
UInt32 DefaultValue<UInt32>(StringManager&)
{
	return 0;
}

template <>
inline
Value::Number DefaultValue<Value::Number>(StringManager&)
{
    return NumberUtil::NaN();
}
template <>
inline
Value::Number DefaultValue<const Value::Number&>(StringManager&)
{
    return NumberUtil::NaN();
}

template <>
inline
ASStringNode* DefaultValue<ASStringNode*>(StringManager& sm)
{
    return sm.GetEmptyStringNode();
}

// !!! Do not delete this !!!
template <>
inline
String DefaultValue<String>(StringManager&)
{
    return String();
}
template <>
inline
String DefaultValue<const String&>(StringManager&)
{
    return String();
}

template <>
inline
ASString DefaultValue<ASString>(StringManager& sm)
{
    // No problem with RefCount.
    return sm.CreateEmptyString();
}
template <>
inline
ASString DefaultValue<const ASString&>(StringManager& sm)
{
    // No problem with RefCount.
    return sm.CreateEmptyString();
}

template <>
inline
Value DefaultValue<Value>(StringManager&)
{
    return Value::GetUndefined();
}
template <>
inline
Value DefaultValue<const Value&>(StringManager&)
{
    return Value::GetUndefined();
}

template <>
inline
AS3::Object* DefaultValue<AS3::Object*>(StringManager&)
{
    return NULL;
}

template <>
inline
Instances::Function* DefaultValue<Instances::Function*>(StringManager&)
{
    return NULL;
}

///////////////////////////////////////////////////////////////////////////
namespace Impl
{

    // Convert<FROM,TO> is a class specialization used to convert Value to
    // is component primitive types and back. This class is used to implement the
    // Convert() template function used for reading argument values Thunks.
    // Typically Value -> type conversions are used for reading arguments,
    // while type -> Value conversion is used for the return type.
    //
    // Specializations such as <Value, ASString>, <Value, unsigned> are defined for
    // each primitive type; furthermore, additional conversions are defined in each
    // each object implementation header, converting Value to Instances::ObjectType
    // through a static cast from Instances::Object.
    
    template <typename FROM, typename TO>
    class Convert
    {
    };
    
    template <>
    class Convert<Value, Value>
    {
    public:
        typedef Value TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            to = from;
        }
    };
    
    template <>
    class Convert<Value, String>
    {
    public:
        typedef String TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager& sm)
        {
            ASString str(sm.CreateEmptyString());
            if (from.Convert2String(str, sm))
                to.AssignString(str.ToCStr(), str.GetSize());
        }
    };
    
    template <>
    class Convert<Value, ASString>
    {
    public:
        typedef ASString TO;
        typedef Value FROM;

    public:
        Convert(TO& to, const FROM& from, StringManager& sm)
        {
            from.Convert2String(to, sm).DoNotCheck();
        }
    };

    template <>
    class Convert<Value, bool>
    {
    public:
        typedef bool TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            to = from.Convert2Boolean();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM)
    template <>
    class Convert<Value, int>
    {
    public:
        typedef int TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            SInt32 v;
            if (from.Convert2Int32(v))
                to = v;
        }
    };
#endif

    template <>
    class Convert<Value, SInt32>
    {
    public:
        typedef SInt32 TO;
        typedef Value FROM;

    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            from.Convert2Int32(to).DoNotCheck();
        }
    };

#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_ARM)
    template <>
    class Convert<Value, unsigned int>
    {
    public:
        typedef unsigned int TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            UInt32 v;
            if (from.Convert2UInt32(v))
                to = v;
        }
    };
#endif

    template <>
    class Convert<Value, UInt32>
    {
    public:
        typedef UInt32 TO;
        typedef Value FROM;

    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            from.Convert2UInt32(to).DoNotCheck();
        }
    };
    
    template <>
    class Convert<Value, Value::Number>
    {
    public:
        typedef Value::Number TO;
        typedef Value FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            from.Convert2Number(to).DoNotCheck();
        }
    };
    
    template <class T>
    class Convert<Value, T>
    {
    public:
        typedef T TO;
        typedef Value FROM;

    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            // This *if* statement is required only in case when REPORT_NOT_IMPLEMENTED
            // is not defined.
            if (from.IsUndefined())
                to = T();
            else
                to = (TO)(from.GetObject());
        }
    };

    template <typename FROM>
    class Convert<FROM, Value>
    {
    public:
        typedef Value TO;
        
    public:
        Convert(TO& to, FROM from, StringManager&)
        {
            to = Value(from);
        }
    };
    
    template <>
    class Convert<ASString, Value>
    {
    public:
        typedef Value TO;
        typedef ASString FROM;
        
    public:
        Convert(TO& to, const FROM& from, StringManager&)
        {
            to = Value(from);
        }
    };
    
}

///////////////////////////////////////////////////////////////////////////
template <typename TO, typename FROM>
inline
void Convert(TO& to, const FROM& from, StringManager& sm)
{
    Impl::Convert<FROM, TO>(to, from, sm);
}

///////////////////////////////////////////////////////////////////////////
// RT stands for Result Type. It can be of type Value, which can hold multiple
// value types.
// VT stands for Value Type. This is what we want to be hold by Value. If RT
// is not a Value, VT and RT should be the same.
// If argument is not available, default value of ResultType is returned.
template <typename VT, typename RT>
inline
void ReadArgValueDef(unsigned n, RT& result, unsigned argc, const Value* argv, StringManager& sm)
{
    if (argc > n)
        AS3::Convert(result, argv[n], sm);
    else
        result = RT(AS3::template DefaultValue<VT>(sm));
}

// Similar to ReadArgValueDef()
// If argument is not available, result won't be changed.
template <typename VT, typename RT>
inline
void ReadArgValue(unsigned n, RT& result, unsigned argc, const Value* argv, StringManager& sm)
{
    if (argc > n)
        AS3::Convert(result, argv[n], sm);
}

// Reads an argument at index by converting it to the ResultType. If argument
// is not available, default value of ResultType is returned.
template <typename RT>
inline
void ReadArgDef(unsigned n, RT& result, unsigned argc, const Value* argv, StringManager& sm)
{
    if (argc > n)
        AS3::Convert(result, argv[n], sm);
    else
        result = AS3::template DefaultValue<RT>(sm);
}

// Similar to ReadArgDef()
// If argument is not available, result won't be changed.
template <typename RT>
inline
void ReadArg(unsigned n, RT& result, unsigned argc, const Value* argv, StringManager& sm)
{
    if (argc > n)
        AS3::Convert(result, argv[n], sm);
}

///////////////////////////////////////////////////////////////////////////

// ThunkFunk0-6 are thunk function generator classes.
// The actual implementation of static Func() is provided in VM.h; its
// implementation is resposible for reading the arguments from argv and
// calling the pointer-to-member.

template <typename T, size_t N, typename R>
struct ThunkFunc0
{
    typedef void (T::*TMethod)(R&);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R> 
typename ThunkFunc0<T, N, R>::TMethod ThunkFunc0<T, N, R>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0>
struct ThunkFunc1
{
    typedef void (T::*TMethod)(R&, A0);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R, typename A0> 
typename ThunkFunc1<T, N, R, A0>::TMethod ThunkFunc1<T, N, R, A0>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1>
struct ThunkFunc2
{
    typedef void (T::*TMethod)(R&, A0, A1);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1> 
typename ThunkFunc2<T, N, R, A0, A1>::TMethod ThunkFunc2<T, N, R, A0, A1>::Method = NULL;

// Specialization for variable-argument length function call.
template <typename T, size_t N>
struct ThunkFunc2<T, N, Value, unsigned, const Value*>
{
    typedef void (T::*TMethod)(Value&, unsigned, const Value*);

    static void Func(VM& /*vm*/, const Value& obj, Value& result, unsigned argc, const Value* argv)
    {
        T& _this = static_cast<T&>(*(obj.IsClosure() ? obj.GetClosure() : obj.GetObject()));

        (_this.*Method)(result, argc, argv);
    }

    static TMethod Method;
};

template <typename T, size_t N> 
typename ThunkFunc2<T, N, Value, unsigned, const Value*>::TMethod ThunkFunc2<T, N, Value, unsigned, const Value*>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2>
struct ThunkFunc3
{
    typedef void (T::*TMethod)(R&, A0, A1, A2);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2> 
typename ThunkFunc3<T, N, R, A0, A1, A2>::TMethod
ThunkFunc3<T, N, R, A0, A1, A2>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3>
struct ThunkFunc4
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2, typename A3> 
typename ThunkFunc4<T, N, R, A0, A1, A2, A3>::TMethod
ThunkFunc4<T, N, R, A0, A1, A2, A3>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4>
struct ThunkFunc5
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3, A4);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4> 
typename ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::TMethod
ThunkFunc5<T, N, R, A0, A1, A2, A3, A4>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4, typename A5>
struct ThunkFunc6
{
    typedef void (T::*TMethod)(R&, A0, A1, A2, A3, A4, A5);

    static void Func(VM& vm, const Value& obj, Value& result, unsigned argc, const Value* argv);
    static TMethod Method;
};

template <typename T, size_t N, typename R, typename A0, typename A1, typename A2,
typename A3, typename A4, typename A5> 
typename ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::TMethod
ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5>::Method = NULL;

///////////////////////////////////////////////////////////////////////////
template <size_t N, typename T, typename R>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&))
{
    typedef AS3::ThunkFunc0<T, N, R> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0))
{
    typedef AS3::ThunkFunc1<T, N, R, A0> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0, typename A1>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0, A1))
{
    typedef AS3::ThunkFunc2<T, N, R, A0, A1> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0, typename A1, typename A2>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0, A1, A2))
{
    typedef AS3::ThunkFunc3<T, N, R, A0, A1, A2> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0, typename A1, typename A2, typename A3>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0, A1, A2, A3))
{
    typedef AS3::ThunkFunc4<T, N, R, A0, A1, A2, A3> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0, A1, A2, A3, A4))
{
    typedef AS3::ThunkFunc5<T, N, R, A0, A1, A2, A3, A4> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

template <size_t N, typename T, typename R, typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
inline
ThunkInfo::TThunkFunc MakeThunkFunc(void (T::*m)(R&, A0, A1, A2, A3, A4, A5))
{
    typedef AS3::ThunkFunc6<T, N, R, A0, A1, A2, A3, A4, A5> TFunc;

    TFunc::Method = m;
    return TFunc::Func;
}

///////////////////////////////////////////////////////////////////////////
Value GetDetailValue(VMAbcFile& file, const Abc::ValueDetail& d);

///////////////////////////////////////////////////////////////////////////
class NamespaceSet : public GASRefCountBase
{
public:
    NamespaceSet(VM& vm);
    ~NamespaceSet();
    
public:
    bool Contains(const Instances::Namespace& ns) const;
    
public:
    const ValueArray& GetNamespaces() const
    {
        return Namespaces;
    }
    void Add(Instances::Namespace& ns);
    
public:
    virtual void ForEachChild_GC(GcOp op) const;
    
private:
    ValueArray Namespaces;
};

///////////////////////////////////////////////////////////////////////////
// Multiname should be completely initialized by the time we use it.
// So, the "run-time" part is irrelevant for this class.

namespace TR
{
    class StackReader;
}

class Multiname : public Abc::HasMultinameKind
{
    friend class StackReader;
    friend class TR::StackReader;
    
public:
    typedef Abc::MultinameKind MultinameKind;
    
public:
    Multiname(Instances::Namespace& ns, const Value& name = Value::GetUndefined());
    Multiname(NamespaceSet& nss, const Value& name = Value::GetUndefined());
    Multiname(VMAbcFile& file, const Abc::Multiname& mn);
    // Creates a multi-name from a sting with includes a namespace,
    // as in "flash.display.Stage", where "flash.display" is a namespace.
    Multiname(const VM& vm, const char* qualifiedTypeName);

public:
    bool IsAnyType() const
    {
        return Name.IsNullOrUndefined() || (Name.IsString() && Name.AsString().IsEmpty());
    }

    bool IsAnyNamespace() const
    {
        return IsQName() && Obj.Get() == NULL;
    }

public:
    bool ContainsNamespace(const Instances::Namespace& ns) const;

    const Value& GetName() const
    {
        return Name;
    }
    Instances::Namespace& GetNamespace() const;
    NamespaceSet& GetNamespaceSet() const;

protected:
    // Used by op-stack Read() to resolve name components.
    void SetRTNamespace(Instances::Namespace& ns);

    void SetRTName(const Value& nameVal)
    {
        SF_ASSERT(IsNameLate());
        Name = nameVal;
        setKind((MultinameKind)(GetKind() & ~Abc::MN_NameLate));
    }
    
private:
    SPtr<GASRefCountBase> Obj;

    // Name is a value because of Array's indices.
    Value Name;
};

///////////////////////////////////////////////////////////////////////////
// PropRef is a property reference within Object, combining object
// pointer with a SlotInfo in it. Returned by various FindProperty methods.

class PropRef
{
public:
    PropRef()
    : pSI(NULL)
    {
    }
    PropRef(Object* _object, const SlotInfo* _si)
        : V(_object)
        , pSI(_si)
    {
    }
    PropRef(const Value& v, const SlotInfo* _si)
        : V(v)
        , pSI(_si)
    {
    }
    
public:
    operator bool() const
    {
        return !V.IsUndefined();
    }
    
    bool IsFound() const
    {
        return !V.IsUndefined();
    }
    const Value& GetValue() const
    {
        return V;
    }
    Value& GetValue()
    {
        return V;
    }
    const SlotInfo* GetSlotInfo() const
    {
        return pSI;
    }
    // Get name for debugging purposes.
    DEBUG_CODE(ASString GetName() const;)
    // Can throw exceptions.
    CheckResult GetSlotValue(VM& vm, Value& value) const
    {
        SF_ASSERT(pSI);
        return pSI->GetSlotValue(vm, value, V);
    }
    // Can throw exceptions.
    CheckResult GetSlotValueUnsafe(VM& vm, Value& value) const
    {
        SF_ASSERT(pSI);
        return pSI->GetSlotValueUnsafe(vm, value, V);
    }
    // Can throw exceptions.
    CheckResult GetSlotValue(VM& vm, Value& value, const VTable& vt) const
    {
        SF_ASSERT(pSI);
        return pSI->GetSlotValue(vm, value, V, &vt);
    }
    CheckResult GetSlotValueUnsafe(VM& vm, Value& value, const VTable& vt) const
    {
        SF_ASSERT(pSI);
        return pSI->GetSlotValueUnsafe(vm, value, V, &vt);
    }
    // Can throw exceptions.        
    CheckResult SetSlotValue(VM& vm, const Value& value) const
    {
        SF_ASSERT(pSI);
        return const_cast<SlotInfo*>(pSI)->SetSlotValue(vm, value, V);
    }
    
private:
    Value           V;
    const SlotInfo* pSI;
};


///////////////////////////////////////////////////////////////////////////

// These are the different Built-in traits types that Object instance
// can have. These can be used for efficient TypeOf testing for built-in
// classes.

enum BuiltinTraitsType
{
    // *** VM Types
    Traits_Unknown,
    Traits_Class,
    Traits_Namespace,
    Traits_Array,
    Traits_Vector_object,

    // *** Flash-required types that need fast checking.
    
    // Display object types (must be listed within _Begin/_End range)
    Traits_DisplayObject_Begin,
    Traits_DisplayObject = Traits_DisplayObject_Begin,    
    Traits_InteractiveObject,
    Traits_SimpleButton,
    Traits_Shape,
    Traits_MorphShape,
    Traits_AVM1Movie,
    Traits_Bitmap,    
    // Following items must be derived from DisplayObjectContainer.
    Traits_DisplayObjectContainer,
    Traits_Sprite,
    Traits_MovieClip,
    Traits_Stage,
    Traits_Loader,    
    Traits_DisplayObject_End
};


///////////////////////////////////////////////////////////////////////////
// Forward declarations.

namespace ClassTraits
{
    class Traits;
    class Function;
    class MethodClosure;
}

namespace Instances
{
    class GlobalObject;
    class GlobalObjectCPP;
}

namespace InstanceTraits
{
    class UserDefined;
}

///////////////////////////////////////////////////////////////////////////
class VTable;
enum FindPropAttr { FindGet, FindSet };

///////////////////////////////////////////////////////////////////////////
const SlotInfo* 
FindDynamicSlot(const SlotContainerType& sc, const ASString& name, const Instances::Namespace& ns);

// Future development. Not finished yet.
const SlotInfo* 
FindDynamicSlotStrict(const SlotContainerType& sc, const ASString& name, const Instances::Namespace& ns);

const SlotInfo* 
FindFixedSlot(const Traits& t, const ASString& name, const Instances::Namespace& ns, UPInt& index, Object* obj);

const ClassTraits::Traits* 
FindFixedTraits(const Traits& t, const ASString& name, const Instances::Namespace& ns, UPInt& index);

const SlotInfo* 
FindDynamicSlot(VM& vm, const SlotContainerType& sc, const Multiname& mn);

// Pass an object if this function is called on an object.
// *index* is a slot index in given traits. Not valid if function returns NULL.
const SlotInfo* 
FindFixedSlot(VM& vm, const Traits& t, const Multiname& mn, UPInt& index, Object* obj);

#if 0
const SlotInfo* 
FindFixedSlot(VMAbcFile& file, const Abc::Multiname& mn, const Traits& t, UPInt& index, Object* obj);
#endif

const ClassTraits::Traits* 
FindFixedTraits(VM& vm, const Traits& t, const Multiname& mn);

///////////////////////////////////////////////////////////////////////////
// Seems to be similar to the ScriptObject in Tamarin ...
class Object : public GASRefCountBase
{
    friend class VM; // Because of Class& GetClass().
    friend class SlotInfo; // Because of GetDynamicValue().
    friend class Class; // Because of AddDynamicFunc().
    friend class Instance; // Because of Object(VM& vm);
    friend class InstanceTraits::UserDefined; // Because of SetTraits().
    
public:
    // "t" - traits used to create this object ...
    explicit Object(Traits& t);
    virtual ~Object();

private:
    // This constructor should be used only by Namespace.
    Object(VM& vm);
    
public:
    // (dynamic properties are always in the public namespace in
    // ActionScript 3.0; a run-time error is signaled if an attempt is add a non-public property).
    
    // ECMA [[Put]]
    // Can throw exceptions.
    virtual CheckResult SetProperty(const Multiname& prop_name, const Value& value);
    // Can throw exceptions.
    // This method will return false if property wasn't found or we've got an exception.
    virtual CheckResult GetProperty(const Multiname& prop_name, Value& value);

    // ECMA [[Delete]]
    // Return "true" on success ...
    // Can throw exceptions.
    virtual CheckResult DeleteProperty(const Multiname& prop_name);
    
    // ECMA [[DefaultValue]]
    // Can throw exceptions.
	void GetDefaultValue(Value& result, Value::Hint hint = Value::hintNone);
    
    // called as a function ...
    // "Call" can be called on any object.
    // Can throw exceptions.
    virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);
    // It is a delayed version of the C++ constructor.
    virtual void Super(unsigned argc, const Value* argv);
    
    // ECMA [[Construct]]
    // called as constructor, a la new ClassName() ...
    virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false) = 0;

    // invoked after instance is created and traits are initialized.
    virtual void InitInstance(bool /*extCall*/ = false) {}
    
    virtual ASString GetName() const = 0;

public:
    // Dynamic attributes are always in the public namespace ...
    void AddDynamicSlotValuePair(const ASString& prop_name, const Value& v,
        SlotInfo::Attribute a = SlotInfo::aNone);
    virtual void AddDynamicSlotValuePair(const Multiname& mn, const Value& v,
        SlotInfo::Attribute a = SlotInfo::aNone);
    // Can throw exceptions.
    CheckResult DeleteDynamicSlotValuePair(const Multiname& mn);
    // Delete/set undefined dynamic value.
    void DeleteDynamicSlotValue(AbsoluteIndex ind);

protected:
    // Analogue to Add2VT(), but adds function to dynamic values.
    void AddDynamicFunc(const ThunkInfo& func);

    ///////////////
    // The method below is used with Global Object only.
    ///////////////

    // Add to fixed slots.
    const SlotInfo& AddFixedSlotValuePair(const ASString& prop_name, Pickable<const Instances::Namespace> ns,
        const ClassTraits::Traits* tr, const Value& v, UPInt& index);

public:
	// This is a wrapper, which will execute "valueOf"
	// It can throw exceptions.
	void ValueOf(Value& result);

    ASRefCountCollector *GetCollector() const
    {
        return reinterpret_cast<ASRefCountCollector*>(GASRefCountBase::GetCollector());
    }
    
    virtual void ForEachChild_GC(GcOp op) const;
    void ForEachChild_GC_NoValues(GcOp op) const;
    void ForEachChild_GC_NoTraits(GcOp op) const;

    const Class& GetClass() const;
    
    virtual const ScopeStackType* GetStoredScope() const { return NULL; }

    bool IsDynamic() const;
    
public:
    virtual Value GetNextPropertyName(GlobalSlotIndex ind) const;

    // GetNextPropertyIndex is used for hasnext/hasnext2 opcode
    // implementation, which iterates on the object.
    // If there are no more properties, then the result index is 0,
    // which is an invalid SlotIndex.    
    virtual GlobalSlotIndex GetNextPropertyIndex(GlobalSlotIndex ind) const;

    // Get the value of the property that is at position index + 1.
    // Can throw exceptions.
    virtual void GetNextPropertyValue(Value& value, GlobalSlotIndex ind);

public:
    // These slot related methods are provided for convenience.

    // Can throw exceptions.
    CheckResult GetSlotValue(SlotIndex ind, Value& value);
    CheckResult GetSlotValueUnsafe(SlotIndex ind, Value& value);
    CheckResult GetSlotValue(GlobalSlotIndex ind, Value& value);
    CheckResult GetSlotValueUnsafe(GlobalSlotIndex ind, Value& value);

    // Set a value in this object referenced by a slot.
    // Can throw exceptions.        
    CheckResult SetSlotValue(SlotIndex ind, const Value& value);

public:
    // Object's Traits are not supposed to be changed elsewhere ...
    const Traits& GetTraits() const
    {
        SF_ASSERT(pTraits.Get());
        return *pTraits;
    }
    // Non-const version is public because we need to create instances of
    // classes outside of traits them self.
    // Non-const GetTraits() is not safe, but we need it.
    Traits& GetTraits()
    {
        SF_ASSERT(pTraits.Get());
        return *pTraits;
    }

    VM& GetVM() const;

    BuiltinTraitsType GetTraitsType() const;

    const VTable& GetVT() const;

public:
    ASString GetASString(const char *pstr) const;
    ASString GetASString(const char *pstr, UPInt length) const;
    ASString GetASString(const String& str) const;

public:
    // These methods are safe to be public.

    // AS3 methods.
    void constructor(Value& result);

public:
    // !!! This method should not be used in Object::hasOwnProperty() because it
    // looks into static properties and into a prototype chain.
    // This method is not const because it can cause on demand object initialization.
    PropRef FindProperty(const Multiname& mn, FindPropAttr attr = FindGet);

    // Searches traits on object; searchTraits are typically instance traits
    // of this object, but may also be traits for a base (when searching super).
    const SlotInfo* FindFixedSlot(const ASString& name, 
                                  const Instances::Namespace& ns,
                                  UPInt& index)
    {
        return AS3::FindFixedSlot(GetTraits(), name, ns, index, this);
    }

    const SlotInfo* FindDynamicSlot(const ASString& name, 
                                    const Instances::Namespace& ns) const
    {
        return AS3::FindDynamicSlot(GetDynamicSlots(), name, ns);
    }
    virtual const SlotInfo* FindDynamicSlot(const Multiname& mn) const
    {
        return AS3::FindDynamicSlot(GetVM(), GetDynamicSlots(), mn);
    }

    const SlotContainerType& GetDynamicSlots() const
    {
        return DynSlots;
    }

public:
    virtual const SlotInfo* InitializeOnDemand(
        const SlotInfo* si,
        const ASString& name,
        const Instances::Namespace& ns,
        UPInt& index);
    
protected:    
    const ValueArray& GetValues() const
    {
        return Values;
    }
    
    Class& GetClass();
    StringManager& GetStringManager() const;
    const Value& GetGlobalObject() const;
    bool IsException() const;

protected:    
    // Assign value "v" to Values into first available spot.
    // Return index in Values where "v" was assigned.
    AbsoluteIndex SetValue(const Value& v);
    // Assign value "v" to Values at index ind.
    void SetValue(AbsoluteIndex ind, const Value& value);

//protected:
public:
    // Can throw exceptions.
    void ExecuteProperty(const ASString& name, Value& result, unsigned argc, const Value* argv);
	void ExecuteValue(Value& value, Value& result, unsigned argc, const Value* argv);
    
private:
    Object(const Object&);

private:
    void AdjustNumOfValues();
    void SetTraits(Traits& t);

private:
    void Set(SPtr<Object>& o, const Value& v);
    void Set(Value& value, const Value& v);

    const Value& GetDynamicValue(AbsoluteIndex ind) const
    {
        SF_ASSERT(ind.IsValid() && static_cast<UPInt>(ind.Get()) < Values.GetSize());
        return Values[ind.Get()];
    }
    Value& GetDynamicValue(AbsoluteIndex ind)
    {
        SF_ASSERT(ind.IsValid() && static_cast<UPInt>(ind.Get()) < Values.GetSize());
        return Values[ind.Get()];
    }

    // Can throw exceptions.
    CheckResult GetSlotValue(const ASString& prop_name, Instances::Namespace& ns, Value& value);
    CheckResult GetSlotValueUnsafe(const ASString& prop_name, Instances::Namespace& ns, Value& value);

private:
    //
    SPtr<Traits> pTraits;
    
    /////////////
    // Data, which is related only to dynamic objects.
    /////////////

    // Values are shared between fixed slots (in user defined classes) and
    // dynamic slots of a particular object.
    ValueArray Values;
    // Slot info, which is specific to dynamic objects.
    SlotContainerType DynSlots;
    // FirstFreeInd is used to optimize add/remove operations with Values.
    UPInt FirstFreeInd;
}; // class Object

inline 
Value::operator SPtr<Object>() const
{
    SF_ASSERT(IsObject());
    return SPtr<Object>(static_cast<Object*>(value));
}

///////////////////////////////////////////////////////////////////////////
PropRef FindScopeProperty(VM& vm, const UPInt baseSSInd, const ScopeStackType& scope_stack, const Multiname& mn);
const ClassTraits::Traits* FindScopeTraits(VM& vm, const UPInt baseSSInd, const ScopeStackType& scope_stack, const Multiname& mn);

///////////////////////////////////////////////////////////////////////////
// Try to locate own property. If that is not possible, then look into stored scope chain (if available).
// This function is similar to Object::FindProperty().
// The difference is that it can handle primitive values.
// It can throw exceptions.
PropRef FindObjProperty(VM& vm, const Value& value, const Multiname& mn, FindPropAttr attr = FindGet);
PropRef FindPropertyWith(VM& vm, const Value& value, const Multiname& mn, FindPropAttr attr = FindGet);
DEBUG_CODE(inline ASString PropRef::GetName() const { return V.GetObject()->GetName() + "::" + pSI->GetName(); })

// This function will find a function with name prop_name on value _this and execute it.
// Return false if function is not found or exception is thrown.
CheckResult ExecuteProperty(VM& vm, const Multiname& prop_name, const Value& _this, Value& result, unsigned argc, const Value* argv);

// Put property value.
// This method is similar to Object::SetProperty().
// It can throw exceptions.
CheckResult SetProperty(VM& vm, const Value& _this, const Multiname& prop_name, const Value& value);
// Get property value.
// It can throw exceptions.
// This method will return false if property wasn't found or we've got an exception.
CheckResult GetProperty(VM& vm, const Value& _this, const Multiname& prop_name, Value& value);
CheckResult GetPropertyUnsafe(VM& vm, const Value& _this, const Multiname& prop_name, Value& value);

///////////////////////////////////////////////////////////////////////////
// Can throw exceptions.
// ot - Origination Traits.
CheckResult GetSuperProperty(VM& vm, const Traits* ot, Value& result, const Value& _this, const Multiname& mn);
// Can throw exceptions.        
// ot - Origination Traits.
CheckResult SetSuperProperty(VM& vm, const Traits* ot, const Value& object, const Multiname& mn, const Value& value);

///////////////////////////////////////////////////////////////////////////
// Can throw exceptions.
// This function will run "toString" method on _this.
CheckResult ToString(VM& vm, const Value& _this, ASString& result);

///////////////////////////////////////////////////////////////////////////
namespace InstanceTraits
{
    class Traits;
}

// Class is a synonym for datatype ...
// Static part of the object ...
// !!! May not be moveable ...
// ??? It is a base class for all Classes. Theoretically, it is supposed to be abstract ...
class Class : public Object
{
    friend class ClassTraits::Traits;
    
public:
    typedef Class ClassType;

public:
    // "t" are traits used to create this Class ...
    // "base" is not *const* because Class is Object and we need to 
    // be able to change properties of Objects ...
    Class(const ASString& n, ClassTraits::Traits& t, Class* parent);
    virtual ~Class();
    
public:
    // Implementation ...
    
#ifdef CONSTRUCT_IN_TRAITS
    // ECMA [[Construct]]
    // called as constructor, a la new ClassName() ...
    virtual void Construct(Value& result, unsigned argc, const Value* argv, bool extCall = false);
#else
    virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
    virtual void PreInit(const Value& _this) const;
    virtual void PostInit(const Value& _this, unsigned argc, const Value* argv) const;
#endif

    // Get a class for a generic.
    virtual Class& ApplyTypeArgs(unsigned argc, const Value* argv);

    virtual ASString GetName() const;

    virtual void ForEachChild_GC(GcOp op) const;
    
public:
    // Return Traits needed to create new objects ...
    InstanceTraits::Traits& GetNewObjectTraits() const;

    Class* GetParentClass() const
    {
        return ParentClass;
    }
    
    Object& GetPrototype();

    const ClassTraits::Traits& GetClassTraits() const;
    ClassTraits::Traits& GetClassTraits();
    const InstanceTraits::Traits& GetInstanceTraits() const;
    InstanceTraits::Traits& GetInstanceTraits();

public:
    // Type conversion collection of methods ...

    virtual void Call(const Value& _this, Value& result, unsigned argc, const Value* const argv);

    typedef Value (Class::*ConvertFunc)(const Value& v) const;

    // Add methods from VTable to a prototype object.
    // Raw methods are added, no proxies.
    void InitPrototypeFromVTable(AS3::Object& obj, ConvertFunc f = &Class::ConvertCopy) const;
    // Run methods through a CheckType proxy.
    void InitPrototypeFromVTableCheckType(AS3::Object& obj) const;

    // InitPrototype() has to be overloaded if prototype methods do not match regular methods.
    // For example, in the Object instance.
    virtual void InitPrototype(AS3::Object& obj) const;
    // Create a prototype object.
    virtual Pickable<Object> MakePrototype() const;

protected:
    // Add "constructor" to the prototype object.
    void AddConstructor(AS3::Object& obj) const;

    void SetPrototype(Pickable<AS3::Object> p)
    {
        SF_ASSERT(!pPrototype.Get());
        pPrototype = p;
        SF_ASSERT(pPrototype.Get());
    }
    // Object::AddDynamicFunc() is protected. So, we need a wrapper.
    static void AddDynamicFunc(Object& obj, const ThunkInfo& func)
    {
        obj.AddDynamicFunc(func);
    }
    
private:
    // Converters.
    Value ConvertCopy(const Value& v) const;
    Value ConvertCheckType(const Value& v) const;

private:
    // AS3 methods.
    void lengthGet(SInt32& result);
    void prototypeGet(Value& result);
    void toString(ASString& result);

    // This member is not *const* because Class is Object and we need to 
    // be able to change properties of Objects ...
    SPtr<Class>                 ParentClass; 
    const ASString              Name;
        
protected:
    SInt32 Length;
    
private:
    SPtr<Object> pPrototype;
}; // class Class

///////////////////////////////////////////////////////////////////////////
// Forward declaration.

// This is a C++ trait-like function.
// template <class T>
// const char* GetClassName();
// 
// template <class T>
// inline
// const char* GetClassName(T*) { return GetClassName<T>(); }

namespace Classes
{
    class None;

    // This is a C++ trait class for a class.
    template <typename T>
    struct Info
    {
        static const char* GetName();
        static const char* GetPkgName();
        static const char* GetParentName();
        static const char* GetParentPkgName();
        static Pickable<ClassTraits::Traits> MakeClassTraits(VM& vm, const ClassTraits::Traits& parent);
    };

    template <>
    struct Info<None>
    {
        static const char* GetName();
        static const char* GetPkgName();
    };
}

namespace Impl
{

    // C - class
    // M - method id
    // N - arg number
    // T - arg type
    template <class C, int M, int N, typename T>
    inline
    typename Clean<T>::type GetMethodDefArg(AS3::StringManager& sm)
    {
        return DefaultValue<T>(sm);
    }

}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Object_H

