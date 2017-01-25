/**********************************************************************

Filename    :   AS3_Value.h
Content     :   
Created     :   Jan, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_Value_H
#define INC_AS3_Value_H

#include "Abc/AS3_Abc_Type.h"
#include "GFx/GFx_ASUtils.h" // For NumberUtil

///////////////////////////////////////////////////////////////////////////////
// Forward declaration.

///////////////////////////////////////////////////////////////////////////////
namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
// Forward declaration ...
class Object;
class Class;
class VM;
class VMAbcFile;
class Value;

namespace Instances
{
    class Function;
    class ThunkFunction;
    class Namespace;
}

namespace InstanceTraits
{
    class Traits;
}

namespace ClassTraits
{
    class Traits;
}

///////////////////////////////////////////////////////////////////////////
// CheckResult is a helper class that represents a boolean value, which MUST be 
// checked. CheckResult is used primarily to to ensure that exceptions
// are checked for after an operation that might've thrown an exception.

class CheckResult
{
public:
    CheckResult(bool r)
        : Result(r)

#ifdef SF_BUILD_DEBUG
        , Checked(false)
#endif
    {
    }
    CheckResult(const CheckResult& other)
        : Result(other.Result)

#ifdef SF_BUILD_DEBUG
        , Checked(other.Checked)
#endif
    {
#ifdef SF_BUILD_DEBUG
        // Pick semantic.
        other.Checked = true;
#endif
    }
    ~CheckResult()
    {
        SF_ASSERT(Checked);
    }

public:
    void DoNotCheck() const
    {
#ifdef SF_BUILD_DEBUG
        Checked = true;
#endif
    }
    operator bool() const
    {
#ifdef SF_BUILD_DEBUG
        Checked = true;
#endif

        return Result;
    }
    bool operator ==(bool v) const
    {
#ifdef SF_BUILD_DEBUG
        Checked = true;
#endif

        return Result == v;
    }
    bool operator !() const
    {
#ifdef SF_BUILD_DEBUG
        Checked = true;
#endif

        return !Result;
    }

    CheckResult& operator =(bool v)
    {
        Result = v;
        return *this;
    }

private:
    CheckResult& operator =(const CheckResult& other);

private:
    bool Result;

#ifdef SF_BUILD_DEBUG
    mutable bool Checked;
#endif
};

///////////////////////////////////////////////////////////////////////////
// Type of code used with virtual table.
enum TCodeType 
{
    CT_Method = 0U,
    CT_Get = 1U,
    CT_Set = 2U
};

struct ThunkInfo 
{
    typedef void (*TThunkFunc)(VM& vm, const Value& _this, Value& result, unsigned argc, const Value* argv);

    TThunkFunc Method;

    const char* PackageName;
    const char* ClassName;
    const char* Name;

    //Abc::NamespaceKind NsKind;
    const char* NamespaceName;

    TCodeType CodeType_:3; // One bit can be saved.

    unsigned MinArgNum_:3;
    unsigned MaxArgNum_:10;

    TCodeType GetCodeType() const
    {
        return CodeType_;
    }
    unsigned GetMinArgNum() const
    {
        return MinArgNum_;
    }
    unsigned GetMaxArgNum() const
    {
        return MaxArgNum_;
    }
};

///////////////////////////////////////////////////////////////////////////
class Value
{
public:
    typedef Double Number;
    
    enum KindType
    {
        kUndefined = 0,
        kBoolean,
        kInt,
        kUInt,
        kNumber,

        kThunk,
        // kMethodInd is redundant at this time. It is transformed into
        // Instances::Function in Add2VT()/VTable::SetMethod() call.
        // Method index could be used by itself, but in this case we need to
        // figure out how to pass/store its file and initialization scope
        // for a method.
        kMethodInd,

        // Value kinds used by the Tracer.
        kInstanceTraits,
        kClassTraits,

        // Below are ref-counted tags.
        kString,
        kFunction,
        kObject,
        kClass,
        kNamespace,
        // ThunkFunction is a Function object, which stores a Thunk.
        kThunkFunction,
        kMethodClosure,
        kThunkClosure,
    };
    enum ObjectTag {otInvalid = -1, otObject = 0, otClass, otFunction, otNamespace};
    enum Hint {hintNone, hintNumber, hintString};
    enum TraceValueType {Valid, Null, Undefined}; // Need to remove "Undefined"

public:
    Value(const Value& other)
    : Flags(other.Flags)
    , Bonus(other.Bonus)
    , value(other.value)
    {
        other.AddRef();
    }
    
    Value()
    : Flags(kUndefined)
    {
    }
    Value(SInt32 v, VMAbcFile& f)
    : Flags(kMethodInd)
    , value(v, f)
    {
    }
    explicit Value(bool v)
    : Flags(kBoolean)
    , value(v)
    {
    }
    explicit Value(SInt32 v)
    : Flags(kInt)
    , value(v)
    {
    }
#if defined(SF_64BIT_POINTERS) || defined(SF_CC_MSVC)
    explicit Value(UPInt v)
    : Flags(kUInt)
    , value(static_cast<UInt32>(v))
    {
		SF_ASSERT(v <= SF_MAX_UINT32);
    }
#endif
    explicit Value(UInt32 v)
    : Flags(kUInt)
    , value(v)
    {
    }
    explicit Value(Number v)
    : Flags(kNumber)
    , value(v)
    {
    }
    Value(const ThunkInfo& v)
    : Flags(kThunk)
    , value(v)
    {
    }
    Value(const ASString& v);
    Value(ASStringNode* v);

    // This constructor is declared explicit to prevent unexpected type convention.
    template <typename OT>
    explicit Value(const SPtr<OT>& v) : Flags(kUndefined) { AssignUnsafe(v); }
    // Pick semantic.
    template <typename OT>
    Value(SPtr<OT>& v, PickType) : Flags(kUndefined) { PickUnsafe(v); }


    explicit Value(Object* v);
    explicit Value(Class* v);
    explicit Value(Instances::Function* v);
    explicit Value(Instances::ThunkFunction* v);
    explicit Value(Instances::Namespace* v);

    Value(Object* p, Instances::Function* f) 
        : Flags(kMethodClosure)
        , value(p, f)
    {
        SF_ASSERT(p);
        SF_ASSERT(f);
        AddRef();
    }
    Value(Object* p, const ThunkInfo& f) 
        : Flags(kThunkClosure)
        , value(p, f)
    { 
        SF_ASSERT(p);
        AddRef();
    }

    ~Value()
    {
        Release();
    }

public:
    // Tracer related.
    Value(InstanceTraits::Traits& tr, bool null = false)
        : Flags(kInstanceTraits)
        , Bonus(tr)
    {
        SetTraceValueType(null ? Null : Valid);
        SetWith(false);
    }
    Value(ClassTraits::Traits& tr, bool null = false)
        : Flags(kClassTraits)
        , Bonus(tr)
    {
        SetTraceValueType(null ? Null : Valid);
        SetWith(false);
    }

public:
    // Pick constructors. They are not declared explicit on purpose.

    // They save one AddRef and one Release (which is expensive) during 
    // destruction of the "other" value.

    Value(Value& other, PickType)
        : Flags(other.Flags)
        , Bonus(other.Bonus)
        , value(other.value)
    {
        other.SetFlags(0); // Clean up all flags.
    }
    Value(Object* p, Instances::Function* f, PickType) 
        : Flags(kMethodClosure)
        , value(p, f)
    {
        SF_ASSERT(p);
        SF_ASSERT(f);
        // No AddRef() is necessary.
    }
    Value(Object* p, const ThunkInfo& f, PickType) 
        : Flags(kThunkClosure)
        , value(p, f)
    { 
        SF_ASSERT(p);
        // No AddRef() is necessary.
    }

    template <typename OT>
    Value(Pickable<OT> v) { PickUnsafe(v); }

public:
    ///
    void Assign(const Value& other);
    void AssignUnsafe(const Value& other);

    ///
    void Assign(const ThunkInfo& v)
    {
        Release();
        SetKind(kThunk);
        value = v;
    }
    void AssignUnsafe(const ThunkInfo& v)
    {
        SF_ASSERT(!IsRefCounted());

        SetKind(kThunk);
        value = v;
    }

    ///
    void Assign(const ASString& v);
    void AssignUnsafe(const ASString& v);

    ///
    void Assign(ASStringNode* v);
    void AssignUnsafe(ASStringNode* v);

    ///
    template <typename OT>
    void Assign(const SPtr<OT>& v) { Assign(v.Get()); }
    template <typename OT>
    void AssignUnsafe(const SPtr<OT>& v) { AssignUnsafe(v.Get()); }


    ///
    void Assign(Object* v);
    void AssignUnsafe(Object* v);

    ///
    void Assign(Class* v);
    void AssignUnsafe(Class* v);

    ///
    void Assign(Instances::Function* v);
    void AssignUnsafe(Instances::Function* v);

    ///
    void Assign(Instances::ThunkFunction* v);
    void AssignUnsafe(Instances::ThunkFunction* v);

    ///
    void Assign(Instances::Namespace* v);
    void AssignUnsafe(Instances::Namespace* v);

public:
    // Assignment operators below eliminate a temporary Value object
    // and an expensive Release() call.
    Value& operator =(const Value& other) { Assign(other); return *this; }
    Value& operator =(const ThunkInfo& v) { Assign(v); return *this; }
    Value& operator =(const ASString& v) { Assign(v); return *this; }
    Value& operator =(ASStringNode* v) { Assign(v); return *this; }
    Value& operator =(Object* v) { Assign(v); return *this; }
    Value& operator =(Class* v) { Assign(v); return *this; }
    Value& operator =(Instances::Function* v) { Assign(v); return *this; }
    Value& operator =(Instances::ThunkFunction* v) { Assign(v); return *this; }
    Value& operator =(Instances::Namespace* v) { Assign(v); return *this; }
    template <typename OT>
    Value& operator =(const SPtr<OT>& v) { Assign(v); return *this; }

public:
    // Pick value from another value.
    // It saves one AddRef/Release.

    ///
    template <typename OT>
    void Pick(SPtr<OT>& v)
    {
        Pick(v.Get());
        v.pObject = NULL;
    }
    template <typename OT>
    void PickUnsafe(SPtr<OT>& v)
    {
        PickUnsafe(v.Get());
        v.pObject = NULL;
    }
    template <typename OT>
    void Pick(Pickable<OT> v) { Pick(v.GetPtr()); }
    template <typename OT>
    void PickUnsafe(Pickable<OT> v) { PickUnsafe(v.GetPtr()); }

    ///
    void Pick(Value& other);
    void PickUnsafe(Value& other);

    ///
    void Pick(Object* v);
    void PickUnsafe(Object* v);

    ///
    void Pick(Class* v);
    void PickUnsafe(Class* v);

    ///
    void Pick(Instances::Function* v);
    void PickUnsafe(Instances::Function* v);

    ///
    void Pick(Instances::ThunkFunction* v);
    void PickUnsafe(Instances::ThunkFunction* v);

    ///
    void Pick(Instances::Namespace* v);
    void PickUnsafe(Instances::Namespace* v);

public:
    template <typename OT>
    Value& operator =(Pickable<OT> v) { Pick(v); return *this; }

public:
    // A bunch of "Set" functions, which are developed to avoid implicit type conversion.
    // (They correspond to explicit constructors.)
    // You can call "Unsafe" version if you know for sure that previous value is not
    // reference countable. (This is done for optimization)
    void SetUndefined()
    {
        Release();
        SetKind(kUndefined);
    }
    void SetUndefinedUnsafe()
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kUndefined);
    }
    void SetNull()
    {
        Release();
        SetKind(kObject);
        value = (Object*)NULL;
    }
    void SetNullUnsafe()
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kObject);
        value = (Object*)NULL;
    }
    void SetBool(bool v)
    {
        Release();
        SetKind(kBoolean);
        value = v;
    }
    void SetBoolUnsafe(bool v)
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kBoolean);
        value = v;
    }
    void SetSInt32(SInt32 v)
    {
        Release();
        SetKind(kInt);
        value = v;
    }
    void SetSInt32Unsafe(SInt32 v)
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kInt);
        value = v;
    }
    void SetUPInt(UPInt v)
    {
        SF_ASSERT(v <= SF_MAX_UINT32);
        Release();
        SetKind(kUInt);
        value = static_cast<UInt32>(v);
    }
    void SetUPIntUnsafe(UPInt v)
    {
        SF_ASSERT(v <= SF_MAX_UINT32);
        SF_ASSERT(!IsRefCounted());
        SetKind(kUInt);
        value = static_cast<UInt32>(v);
    }
    void SetUInt32(UInt32 v)
    {
        Release();
        SetKind(kUInt);
        value = v;
    }
    void SetUInt32Unsafe(UInt32 v)
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kUInt);
        value = v;
    }
    void SetNumber(Number v)
    {
        Release();
        SetKind(kNumber);
        value = v;
    }
    void SetNumberUnsafe(Number v)
    {
        SF_ASSERT(!IsRefCounted());
        SetKind(kNumber);
        value = v;
    }    

    void SetWith(bool v = true)
    {
        // Conditionally set or clear bits without branching.
        Flags = (Flags & ~withMask) | (-static_cast<SInt32>(v) & withMask);
    }

    // This method is optimized for performance.
    // No AddRef/Release will be called.
    void Swap(Value& other);

public:
    ///
    operator ASStringNode*() const
    {
        SF_ASSERT(GetKind() == kString);
        return value;
    }
    operator ASString() const
    {
        SF_ASSERT(GetKind() == kString);
        return ASString(value);
    }

    ///
    operator bool&()
    {
        SF_ASSERT(GetKind() == kBoolean);
        return value;
    }
    operator bool() const
    {
        SF_ASSERT(GetKind() == kBoolean);
        return value;
    }

    ///
    operator SInt32&()
    {
        SF_ASSERT(GetKind() == kInt);
        return value;
    }
    operator SInt32() const
    {
        SF_ASSERT(GetKind() == kInt);
        return value;
    }

    ///
    operator UInt32&()
    {
        SF_ASSERT(GetKind() == kUInt);
        return value;
    }
    operator UInt32() const
    {
        SF_ASSERT(GetKind() == kUInt);
        return value.operator UInt32();
    }

    ///
    operator Number&()
    {
        SF_ASSERT(GetKind() == kNumber);
        return value;
    }
    operator Number() const
    {
        SF_ASSERT(GetKind() == kNumber);
        return value;
    }

    ///
    operator SPtr<Object>() const;

    ///
    operator Object&() const
    {
        SF_ASSERT(IsObject());
        SF_ASSERT(static_cast<Object*>(value));
        return *static_cast<Object*>(value);
    }

    ///
    operator Class&() const
    {
        SF_ASSERT(IsClass());
        SF_ASSERT(static_cast<Class*>(value));
        return *static_cast<Class*>(value);
    }

    ///
    operator Instances::Function&() const
    {
        SF_ASSERT(IsFunction());
        SF_ASSERT(static_cast<Instances::Function*>(value));
        return *static_cast<Instances::Function*>(value);
    }

    ///
    operator const ThunkInfo&() const
    {
        SF_ASSERT(IsThunk());
        return value;
    }

    ///
    operator Instances::ThunkFunction&() const
    {
        SF_ASSERT(IsThunkFunction());
        SF_ASSERT(static_cast<Instances::ThunkFunction*>(value));
        return *static_cast<Instances::ThunkFunction*>(value);
    }

    ///
    operator Instances::Namespace&() const
    {
        SF_ASSERT(IsNamespace());
        SF_ASSERT(static_cast<Instances::Namespace*>(value));
        return *static_cast<Instances::Namespace*>(value);
    }

    // !!! Return pointer.
    Instances::Function* GetFunct() const
    {
        SF_ASSERT(IsMethodClosure());
        return value.GetFunct();
    }

    const ThunkInfo& GetThunkFunct() const
    {
        SF_ASSERT(IsThunkClosure());
        return value.GetThunkFunct();
    }
    
public:
    // Convert to not Value data types.

    //
    bool& AsBool()
    {
        SF_ASSERT(GetKind() == kBoolean);
        return value;
    }
    bool AsBool() const
    {
        SF_ASSERT(GetKind() == kBoolean);
        return value;
    }

    //
    SInt32& AsInt()
    {
        SF_ASSERT(GetKind() == kInt);
        return value;
    }
    SInt32 AsInt() const
    {
        SF_ASSERT(GetKind() == kInt);
        return value;
    }

    //
    UInt32& AsUInt()
    {
        SF_ASSERT(GetKind() == kUInt);
        return value;
    }
    UInt32 AsUInt() const
    {
        SF_ASSERT(GetKind() == kUInt);
        return value.operator UInt32();
    }

    //
    Number& AsNumber()
    {
        SF_ASSERT(GetKind() == kNumber);
        return value;
    }
    Number AsNumber() const
    {
        SF_ASSERT(GetKind() == kNumber);
        return value;
    }

	//
    ASStringNode* AsStringNode() const
    {
        SF_ASSERT(GetKind() == kString);
        return value;
    }
    ASString AsString() const
    {
        SF_ASSERT(GetKind() == kString);
        return ASString(value);
    }

    // Although value itself is const it may contain a non-const Object.
    Object* GetObject() const
    {
        SF_ASSERT(IsObject());
        return value.VS._1.VObj;
    }
    Object* GetClosure() const
    {
        SF_ASSERT(GetKind() == kThunkClosure || GetKind() == kMethodClosure);
        return value.VS._2.VObj;
    }
    Object** AsObjectPtrPtr() const
    {
        switch (GetKind())
        {
        case kFunction:
        case kClass:
        case kThunkFunction:
        case kObject:
        case kMethodClosure:
            // Let's cheat on constness.
            return const_cast<Object**>(&value.VS._1.VObj);
        default:
            break;
        }

        SF_ASSERT(false);
        return NULL;
    }
    Object** AsClosurePtrPtr() const
    {
        SF_ASSERT(GetKind() == kThunkClosure || GetKind() == kMethodClosure);
        // Let's cheat on constness.
        return const_cast<Object**>(&value.VS._2.VObj);
    }
    
	//
    Class& AsClass()
    {
        SF_ASSERT(IsClass());
        SF_ASSERT(value.VS._1.VClass);
        return *value.VS._1.VClass;
    }
    const Class& AsClass() const
    {
        SF_ASSERT(IsClass());
        SF_ASSERT(value.VS._1.VClass);
        return *value.VS._1.VClass;
    }
    
	//
    Instances::Function& AsFunction()
    {
        SF_ASSERT(IsFunction());
        SF_ASSERT(static_cast<Instances::Function*>(value));
        return *static_cast<Instances::Function*>(value);
    }
    const Instances::Function& AsFunction() const
    {
        SF_ASSERT(IsFunction());
        SF_ASSERT(static_cast<const Instances::Function*>(value));
        return *static_cast<const Instances::Function*>(value);
    }

	//
    const ThunkInfo& AsThunk() const
    {
        SF_ASSERT(IsThunk());
        return value;
    }

	//
    const Instances::ThunkFunction& AsThunkFunction() const
    {
        SF_ASSERT(IsThunkFunction());
        SF_ASSERT(static_cast<const Instances::ThunkFunction*>(value));
        return *static_cast<const Instances::ThunkFunction*>(value);
    }

	//
    const Instances::Namespace& AsNamespace() const
    {
        SF_ASSERT(IsNamespace());
        SF_ASSERT(static_cast<const Instances::Namespace*>(value));
        return *static_cast<const Instances::Namespace*>(value);
    }

    VMAbcFile& GetAbcFile() const
    {
        SF_ASSERT(IsMethodInd());
        return value.GetAbcFile();
    }

public:
    static const Value& GetUndefined();
    static const Value& GetNull();

public:
    // Masks without Traits.
    //enum { kindMask = 0x0F, valueTypeMask = 0x30, valueTypeOffset = 4, withMask = 0x80 };
    // Masks with Traits.
    enum { kindMask = 0x1F, valueTypeMask = 0x60, valueTypeOffset = 5, withMask = 0x100 };

    KindType GetKind() const
    {
        return static_cast<KindType>(Flags & kindMask);
    }

    ObjectTag GetObjectTag() const;
    ASStringNode* GetStringNode() const
    {
        SF_ASSERT(GetKind() == kString);
        return value;
    }
    SInt32 GetMethodInd() const
    {
        SF_ASSERT(GetKind() == kMethodInd);
        return value;
    }

    // Methods related to the Tracer.
    TraceValueType GetTraceValueType() const { return static_cast<TraceValueType>((Flags & valueTypeMask) >> valueTypeOffset); }
    bool GetWith() const { return (Flags & withMask) > 0; }
    bool IsTraceNull() const { return GetTraceValueType() == Null; }
    InstanceTraits::Traits& GetInstanceTraits() const
    {
        SF_ASSERT(GetKind() == kInstanceTraits);
        SF_ASSERT(Bonus.ITr);
        return *Bonus.ITr;
    }
    ClassTraits::Traits& GetClassTraits() const
    {
        SF_ASSERT(GetKind() == kClassTraits);
        SF_ASSERT(Bonus.CTr);
        return *Bonus.CTr;
    }
    bool IsTraceNullOrUndefined() const { return IsTraceNull() || IsUndefined(); }

public:
    // (ECMA-262 section 9.1)
    // Can throw exceptions.
    // Return false in case of exception.
    // result will not be modified in case of exception.
    CheckResult Convert2PrimitiveValue(Value& result, Hint hint = hintNone) const;
    // Can throw exceptions.
    // Return false in case of exception.
    // Value will not be modified in case of exception.
    CheckResult ToPrimitiveValue()
    {
        return Convert2PrimitiveValue(*this);
    }

    // (ECMA-262 section 9.2)
    // Doesn't throw exceptions.
    bool Convert2Boolean() const;
    // Doesn't throw exceptions.
    void ToBooleanValue()
    {
        SetBool(Convert2Boolean());
    }
    
    // (ECMA-262 section 9.3)
    // Can throw exceptions.
    // Return false in case of exception.
    CheckResult Convert2Number(Number& result) const;
    // Can throw exceptions.
    // value will not be modified in case of exception.
    CheckResult ToNumberValue()
    {
        CheckResult rc = true;
        Number result;

        if (Convert2Number(result))
            SetNumber(result);
        else
            rc = false;

        return rc;
    }
    
    // (ECMA-262 section 9.5)
    // Can throw exceptions.
    // Return false in case of exception.
    CheckResult Convert2Int32(SInt32& result) const;
    // Can throw exceptions.
    CheckResult ToInt32Value()
    {
        bool rc = true;
        SInt32 r;

        if (Convert2Int32(r))
            SetSInt32(r);
        else
            rc = false;

        return rc;
    }
    
    // (ECMA-262 section 9.6)
    CheckResult Convert2UInt32(UInt32& result) const;
    // Can throw exceptions.
    CheckResult ToUInt32Value()
    {
        CheckResult rc = true;
        UInt32 r;

        if (Convert2UInt32(r))
            SetUInt32(r);
        else
            rc = false;

        return rc;
    }
    
    // (ECMA-262 section 9.8)
    // Can throw exceptions.
    // Return false in case of exception.
    CheckResult Convert2String(ASString& result, StringManager& sm) const;
    // Can throw exceptions.
    CheckResult ToStringValue(StringManager& sm)
    {
        CheckResult rc = true;
        ASString r = sm.CreateEmptyString();

        if (Convert2String(r, sm))
            *this = r;
        else
            rc = false;

        return rc;
    }
    
    void Clean()
    {
        Release();
        SetKind(kUndefined);
    }
    
public:
    template <typename T> static T& ToType(Value& v, bool& success);
    
public:
    bool IsNull() const 
    { 
        if (IsObject())
            return value.VS._1.VObj == NULL;

        return false;
    }
    bool IsInt() const { return GetKind() == kInt; }
    bool IsBool() const { return GetKind() == kBoolean; }
    bool IsUInt() const { return GetKind() == kUInt; }
    bool IsNumber() const { return GetKind() == kNumber; }
    bool IsString() const { return GetKind() == kString; }
    bool IsUndefined() const { return GetKind() == kUndefined; }
    bool IsNullOrUndefined() const { return GetKind() == kUndefined || IsNull(); }
    bool IsFunction() const { return GetKind() == kFunction; }
    bool IsObjectStrict() const { return GetKind() == kObject; }
    bool IsClass() const { return GetKind() == kClass; }
//     bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsNamespace() || IsMethodClosure() || IsThunkClosure(); }
    bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsMethodClosure(); }
    bool IsThunk() const { return GetKind() == kThunk; }
    bool IsThunkFunction() const { return GetKind() == kThunkFunction; }
    bool IsNamespace() const { return GetKind() == kNamespace; }
    bool IsMethodClosure() const { return GetKind() == kMethodClosure; }
    bool IsThunkClosure() const { return GetKind() == kThunkClosure; }
    bool IsClosure() const { return IsThunkClosure() || IsMethodClosure(); }
    bool IsMethodInd() const { return GetKind() == kMethodInd; }
    bool IsCallable() const { return IsMethodClosure() || IsThunkClosure() || IsFunction() || IsThunk() || IsThunkFunction() || IsMethodInd() || GetKind() == kObject || GetKind() == kClass; }
    bool IsNumeric() const { return IsNumber() || IsInt() || IsUInt(); }
    bool IsPrimitive() const { const int t = (int)GetKind(); return (t < (int)kThunk || t == kString); }
    bool IsRefCounted() const { return (int)GetKind() > (int)kClassTraits; }
    bool IsTraits() const { return GetKind() == kInstanceTraits || GetKind() == kClassTraits; }
            
    bool IsNaN() const;
    bool IsPOSITIVE_INFINITY() const;
    bool IsNEGATIVE_INFINITY() const;
    bool IsNaNOrInfinity() const;
    bool IsPOSITIVE_ZERO() const;
    bool IsNEGATIVE_ZERO() const;
    bool IsNaN_OR_NP_ZERO() const;

private:
    void AddRefInternal() const;
    void ReleaseInternal();

    void AddRef() const
    {
        if (IsRefCounted())
            AddRefInternal();
    }
    void Release()
    {
        if (IsRefCounted())
            ReleaseInternal();
    }

private:
    //
    static UInt32 GetFlags(KindType v)
    {
        return static_cast<UInt32>(v);
    }

    //
    void SetFlags(UInt32 flags)
    {
        Flags = flags;
    }
    void SetKind(KindType v)
    {
        // Four bits.
        Flags = (Flags & ~kindMask) | (static_cast<UInt32>(v));
    }
    void SetTraceValueType(TraceValueType v)
    {
        // Two bits.
        Flags = (Flags & ~valueTypeMask) | (static_cast<UInt32>(v) << valueTypeOffset);
    }

private:
    // Size: one word.
    union Extra
    {
        Extra() : ITr(NULL) {}
        Extra(InstanceTraits::Traits& tr) : ITr(&tr) {}
        Extra(ClassTraits::Traits& tr) : CTr(&tr) {}

        InstanceTraits::Traits* ITr;
        ClassTraits::Traits* CTr;
    };

    // Size: one word.
    union V1U
    {
        bool VBool;
        SInt32 VInt;
        UInt32 VUInt;
        ASStringNode* VStr;
        Object* VObj;
        Class* VClass;
        Instances::Function* VFunct;
        const ThunkInfo* VThunk;
        Instances::ThunkFunction* VThunkFunct;
        Instances::Namespace* VNs;
    };

    // Size: one word.
    union V2U
    {
        Object* VObj;
        VMAbcFile* VAbcFile;
    };

    // Size: two words.
    struct VStruct
    {
        V1U _1;
        V2U _2;
    };

    // Size: two words.
    // Both parts have to be initialized because they will be used to calculate a hash key.
    union VU
    {
    public:
        VU() {} // No initialization for performance reason.
        VU(bool v) { VS._1.VBool = v; }
        VU(SInt32 v) { VS._1.VInt = v; }
        VU(UInt32 v) { VS._1.VUInt = v; }
        VU(Number v) { VNumber = v; }
        VU(ASStringNode* v) { VS._1.VStr = v; }
        VU(Object* v) { VS._1.VObj = v; }
        VU(Class* v) { VS._1.VClass = v; }
        VU(Instances::Function* v) { VS._1.VFunct = v; }
        VU(const ThunkInfo& v) { VS._1.VThunk = &v; }
        VU(Instances::ThunkFunction* v) { VS._1.VThunkFunct = v; }
        VU(Instances::Namespace* v) { VS._1.VNs = v; }

        VU(Object* p, Instances::Function* f) 
        { 
            VS._2.VObj = p; 
            VS._1.VFunct = f; 
        }
        VU(Object* p, const ThunkInfo& f) 
        { 
            VS._2.VObj = p; 
            VS._1.VThunk = &f; 
        }
        VU(SInt32 v, VMAbcFile& f)
        {
            VS._2.VAbcFile = &f;
            VS._1.VInt = v;
        }

    public:
        ///
        operator bool&() { return VS._1.VBool; }
        operator bool() const { return VS._1.VBool; }

        ///
        operator SInt32&() { return VS._1.VInt; }
        operator SInt32() const { return VS._1.VInt; }

        ///
        operator UInt32&() { return VS._1.VUInt; }
        operator UInt32() const { return VS._1.VUInt; }

#if !defined(SF_CC_SNC) && !defined(SF_CC_MWERKS) && !defined(SF_CC_ARM)

        ///
        operator UPInt() const { return VS._1.VUInt; }
#endif

        ///
        operator Number&() { return VNumber; }
        operator Number() const { return VNumber; }

        ///
        operator ASStringNode*() const { return VS._1.VStr; }

        ///
        operator Object*() const { return VS._1.VObj; }

        ///
        operator Class*() const { return VS._1.VClass; }

        ///
        operator Instances::Function*() const { return VS._1.VFunct; }

        ///
        operator const ThunkInfo&() const { return *VS._1.VThunk; }

        ///
        operator Instances::ThunkFunction*() const { return VS._1.VThunkFunct; }

        ///
        operator Instances::Namespace*() const { return VS._1.VNs; }

        ///
        Instances::Function* GetFunct() const { return VS._1.VFunct; }
        const ThunkInfo& GetThunkFunct() const { return *VS._1.VThunk; }
        VMAbcFile& GetAbcFile() const { return *VS._2.VAbcFile; }

    public:
        Number VNumber;
        VStruct VS;
    };

private:
    UInt32  Flags;
    Extra   Bonus;
    VU      value;
};

typedef ArrayLH<Value> ValueArray;
typedef ArrayDH<Value> ValueArrayDH;
typedef ValueArrayDH   ScopeStackType;

template <> 
inline 
Value& Value::ToType<Value>(Value& v, bool& /*success*/) { return v; }

template <> 
inline 
bool& Value::ToType<bool>(Value& v, bool& success) 
{
    if (success)
    {
        v.ToBooleanValue();
        return v;
    }

    static bool tmp;
    return tmp;
}

template <> 
inline 
SInt32& Value::ToType<SInt32>(Value& v, bool& success) 
{
    static SInt32 tmp;
    success = success && v.ToInt32Value();
    return success ? v.AsInt() : tmp;
}

template <> 
inline 
UInt32& Value::ToType<UInt32>(Value& v, bool& success) 
{
    static UInt32 tmp;
    success = success && v.ToUInt32Value();
    return success ? v.AsUInt() : tmp;
}

template <> 
inline 
Value::Number& Value::ToType<Value::Number>(Value& v, bool& success) 
{
    static Value::Number tmp;
    success = success && v.ToNumberValue();
    return success ? v.AsNumber() : tmp;
}

inline
bool Value::IsNaN() const
{
    SF_ASSERT(GetKind() == kNumber);

    return NumberUtil::IsNaN(value.VNumber);
}

inline
bool Value::IsPOSITIVE_INFINITY() const
{
    SF_ASSERT(GetKind() == kNumber);

    return NumberUtil::IsPOSITIVE_INFINITY(value.VNumber);
}

inline
bool Value::IsNEGATIVE_INFINITY() const
{
    SF_ASSERT(GetKind() == kNumber);

    return NumberUtil::IsNEGATIVE_INFINITY(value.VNumber);
}

inline
bool Value::IsNaNOrInfinity() const
{
    SF_ASSERT(GetKind() == kNumber);

    return NumberUtil::IsNaNOrInfinity(value.VNumber);
}

inline
bool Value::IsPOSITIVE_ZERO() const
{
    SF_ASSERT(GetKind() == kNumber);
    
    //return (memcmp(&value.v_number, GFxPOSITIVE_ZERO_Bytes,  sizeof(value.v_number)) == 0);
    // !!! Optimization !!!
    //return value.VLong == SF_UINT64(0x0);
    return NumberUtil::IsPOSITIVE_ZERO(value.VNumber);
}

inline
bool Value::IsNEGATIVE_ZERO() const
{
    SF_ASSERT(GetKind() == kNumber);
    
    //return (memcmp(&value.v_number, GFxNEGATIVE_ZERO_Bytes,  sizeof(value.v_number)) == 0);
    // !!! Optimization !!!
    //return value.VLong == SF_UINT64(0x8000000000000000);
    return NumberUtil::IsNEGATIVE_ZERO(value.VNumber);
}

inline
bool Value::IsNaN_OR_NP_ZERO() const
{
    SF_ASSERT(GetKind() == kNumber);
    
    return IsNaN() || IsPOSITIVE_ZERO() || IsNEGATIVE_ZERO();
}

inline
Value::Number MakeValueNumber(double v)
{
#ifdef SF_NO_DOUBLE
    return NumberUtil::ConvertDouble2Float(v);
#else
    return v;
#endif
}

// (ECMA-262 section 11.6.1)
// Can throw exceptions.
CheckResult Add(StringManager& sm, Value& result, const Value& l, const Value& r);

// Can throw exceptions.
CheckResult Subtract(Value& result, const Value& l, const Value& r);

// Can throw exceptions.
CheckResult Multiply(Value& result, const Value& l, const Value& r);


// Can throw exceptions
// Increment treats numeric values as Number.
void Increment(Value& l);
// Can throw exceptions
// Decrement treats numeric values as Number.
void Decrement(Value& l);
// Can throw exceptions
// Negate treats numeric values as Number.
void Negate(Value& l);

// (ECMA-262 section 13.1.2)
// Not implemented yet ...
inline
bool AreJoined(const Value& /*l*/, const Value& /*r*/)
{
    return false;
}

// (ECMA-262 section 11.9.6)
// No exceptions.
bool StrictEqual(const Value& x, const Value& y);

// (ECMA-262 section 11.9.3)
// Can throw exceptions.
// Return false in case of exception.
CheckResult AbstractEqual(bool& result, const Value& l, const Value& r);

// (ECMA-262 section 11.8.5)
// Can throw exceptions.
// Return false in case of exception.
CheckResult AbstractLessThan(Value& result, const Value& l, const Value& r);

// (ECMA-262 section 11.8.1)
// Can throw exceptions.
// Return false in case of exception.
CheckResult LessThan(bool& result, const Value& l, const Value& r);

// (ECMA-262 section 11.8.2)
// Can throw exceptions.
// Return false in case of exception.
CheckResult GreaterThan(bool& result, const Value& l, const Value& r);

// (ECMA-262 section 11.8.3)
// Can throw exceptions.
// Return false in case of exception.
CheckResult LessThanOrEqual(bool& result, const Value& l, const Value& r);

// (ECMA-262 section 11.8.4)
// Can throw exceptions.
// Return false in case of exception.
CheckResult GreaterThanOrEqual(bool& result, const Value& l, const Value& r);


// Retrieve an object stored at absolute address "addr".
// "addr" stores an ObjectTag in the lowest two bits.
Value GetAbsObject(UPInt addr);

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


namespace Scaleform { namespace Alg {

// Specialization for Swap()
template <>
inline
void Swap<GFx::AS3::Value>(GFx::AS3::Value& l, GFx::AS3::Value& r)
{
    l.Swap(r);
}

}} //namespace Scaleform { namespace Alg {

#endif // INC_AS3_Value_H
