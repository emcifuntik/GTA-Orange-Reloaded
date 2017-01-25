/**********************************************************************

Filename    :   AS3_Traits.h
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

#ifndef INC_AS3_Traits_H
#define INC_AS3_Traits_H

#include "AS3_Object.h"
#include "Kernel/SF_AutoPtr.h"


namespace Scaleform { namespace GFx { namespace AS3
{
    
///////////////////////////////////////////////////////////////////////////
template <typename T>
inline
SlotInfo::DataType GetSlotInfoType(const T&)
{
    return SlotInfo::DT_Object;
}
template <>
inline
SlotInfo::DataType GetSlotInfoType<bool>(const bool&)
{
    return SlotInfo::DT_Boolean;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_MWERKS) && !defined(SF_CC_ARM)
template <>
inline
SlotInfo::DataType GetSlotInfoType<int>(const int&)
{
    return SlotInfo::DT_Int;
}
#endif
template <>
inline
SlotInfo::DataType GetSlotInfoType<SInt32>(const SInt32&)
{
	return SlotInfo::DT_Int;
}
#if !defined(SF_CC_GNU) && !defined(SF_CC_SNC) && !defined(SF_CC_MWERKS) && !defined(SF_CC_ARM)
template <>
inline
SlotInfo::DataType GetSlotInfoType<unsigned int>(const unsigned int&)
{
    return SlotInfo::DT_UInt;
}
#endif
template <>
inline
SlotInfo::DataType GetSlotInfoType<UInt32>(const UInt32&)
{
	return SlotInfo::DT_UInt;
}
template <>
inline
SlotInfo::DataType GetSlotInfoType<Value::Number>(const Value::Number&)
{
    return SlotInfo::DT_Number;
}
template <>
inline
SlotInfo::DataType GetSlotInfoType<Value>(const Value&)
{
    //return SlotInfo::DT_ValueArray;
    return SlotInfo::DT_Value;
}
template <>
inline
SlotInfo::DataType GetSlotInfoType<String>(const String&)
{
    return SlotInfo::DT_String;
}
template <>
inline
SlotInfo::DataType GetSlotInfoType<ASString>(const ASString&)
{
    return SlotInfo::DT_String;
}

///////////////////////////////////////////////////////////////////////////
inline
void SlotInfo::DeleteDynamic(Object& obj) const
{
    SF_ASSERT(ValueInd.IsValid());

    // That shouldn't work with slots ...
    // That is supposed to work with dynamic properties only ...
    // Instance data ...
    obj.DeleteDynamicSlotValue(ValueInd);
}

///////////////////////////////////////////////////////////////////////////
class Slots //: public NewOverrideBase<Mem_Stat>
{
    friend class Object; // Because of GetSlots()

public:
    Slots& operator =(const Slots& other)
    {
        if (this != &other)
        {
            Assign(other);
        }

        return *this;
    }

public:
    void Assign(const Slots& other)
    {
        SlotContainer = other.SlotContainer;
    }
    bool IsValidIndex(AbsoluteIndex ind) const
    {
        return ind.IsValid() && ind.Get() < SlotContainer.GetSize();
    }
	UPInt GetSlotInfoNum() const
    {
        return SlotContainer.GetSize();
    }
    ASString GetName(AbsoluteIndex ind) const
    {
        SF_ASSERT(ind.IsValid() && ind.Get() < SlotContainer.GetSize());
        return ASString(SlotContainer.GetKey(ind));
    }
    const SlotContainerType& GetSlots() const
    {
        return SlotContainer;
    }
    const SlotInfo& GetSlotInfo(AbsoluteIndex ind) const
    {
        SF_ASSERT(ind.IsValid() && ind.Get()< SlotContainer.GetSize());
        
        return SlotContainer[ind];
    }
    const SlotInfo& GetSlotInfo(const ASString& name) const 
    {
//         SF_ASSERT2(slots.Find(name) >= 0, "Slot with name '" + name + "' wasn't found among slots.");
        SF_ASSERT(SlotContainer.Find(name).IsValid());
        
        return SlotContainer.Get(name);
    }
    
    bool HasSlot(const ASString& name) const
    {
        return SlotContainer.Find(name).IsValid();
    }
    bool SlotsAreBound2Values() const
    {
        AbsoluteIndex size(SlotContainer.GetSize());

        for(AbsoluteIndex i(0); i < size; ++i)
        {
            // Ignore DT_Code (and others) ...
            const SlotInfo::DataType bt = SlotContainer[i].GetBindingType();
            switch (bt)
            {
            case SlotInfo::DT_Unknown: // Not bound part of a getter/setter.
            case SlotInfo::DT_Code:
            case SlotInfo::DT_Get:
            case SlotInfo::DT_Set:
            case SlotInfo::DT_GetSet:
                continue;
            default:
            	break;
            }
            
            if (!SlotContainer[i].IsBound2Value())
                return false;
        }
        
        return true;
    }

public:
    // Obtains a slot by index nane + ns, returning null if not found.
    SlotInfo* FindSlotInfo(const ASString& name, const Instances::Namespace& ns)
    {
        AbsoluteIndex ind = FindSlotInfoIndex(name, ns);

        return (ind.IsValid() ? &SlotContainer[ind] : NULL);
    }
    const SlotInfo* FindSlotInfo(const ASString& name, const Instances::Namespace& ns) const
    {
        AbsoluteIndex ind = FindSlotInfoIndex(name, ns);

        return (ind.IsValid() ? &SlotContainer[ind] : NULL);
    }

protected:
    AbsoluteIndex FindSlotInfoIndex(const ASString& name, const Instances::Namespace& ns) const;

    SlotContainerType& GetSlots()
    {
        return SlotContainer;
    }
    SlotInfo& GetSlotInfo(AbsoluteIndex ind)
    {
        SF_ASSERT(ind.IsValid());
        
        return SlotContainer[ind];
    }
    SlotInfo& GetTopSlotInfo()
    {
        return SlotContainer.Back();
    }
    void SetSlotInfo(AbsoluteIndex ind, const ASString& name, const SlotInfo& v);
    void AddSlotInfo(const ASString& name, const SlotInfo& v)
    {
        SlotContainer.Add(name, v);
    }
    void Swap(SlotContainerType& other)
    {
        SlotContainer.Swap(other);
    }

    // FindAdd finds a name, also considering namespace of the slot.
    SlotInfo& FindAddSlotInfo(const ASString& name, const SlotInfo& v);

private:
    SlotContainerType SlotContainer;
}; // class Slots

/////////////////////////////////////////////////////////////////cls/////////
struct TClassMemberInfo
{
    unsigned Const:1;
    SlotInfo::DataType Dt:5; // One bit can be saved.
    unsigned Offset:10;
    const char* Name;
    // const Abc::TraitInfo* TI; // ????
};

/////////////////////////////////////////////////////////////////cls/////////
// Forward declaration

namespace Classes
{
	class ClassClass;
    class UserDefined;
}

namespace Instances
{
    class GlobalObjectScript;
}

namespace InstanceTraits
{
    class Traits;
}

namespace ClassTraits
{
    class UserDefined;
}

class GC;
class VM;

///////////////////////////////////////////////////////////////////////////
class Traits : public GASRefCountBase, public Slots
{
    friend class Classes::UserDefined; // Because of StoreScopeStack().
    friend class ClassTraits::UserDefined; // Because of StoreScopeStack().
    friend class ClassTraits::Traits; // Required for ClassTraits::ClassClass.
    friend class InstanceTraits::Traits; // Because of OwnSlotNumber.
    friend class Instances::GlobalObjectCPP; // Because of Add2VT().
    friend class Instances::GlobalObjectScript; // Because of SetupSlotValues().
    
public:
    typedef Traits Self;

public:
    enum ObjectType {SealedObject, DynamicObject};

    // "c" stands for a "constructor" - a class, which will create new objects 
    // (actually, created by these traits) It is more like a back-reference ...
    // "pt" - traits of the base class.
    // VM is not *const* on purpose (because of ExecuteCode() ) ...
    Traits(VM& _vm, Class& c, const Traits* pt, ObjectType ot);
    ~Traits();

private:
    // Special case for the ClassTraits::ClassClass object ...
    // VM is not *const* on purpose (because of ExecuteCode() ) ...
    Traits(VM& _vm);
        
public:
    // Interface ...
    
    // This method is not const because it is supposed to pass non-const "this"
    // into a newly created object ...
    // This method returns Value instead of Object* because primitive types 
    // have more compact representations.
    // Object initialization should be done by the Super() method.
    virtual void MakeObject(Value& result) = 0;

    // Only instance traits can be global.
    virtual bool IsGlobal() const;

#ifdef CONSTRUCT_IN_TRAITS
    virtual void Construct(Value& _this, unsigned argc, const Value* argv, bool extCall = false);
    virtual void PreInit(const Value& _this) const;
    virtual void PostInit(const Value& _this, unsigned argc, const Value* argv) const;
#endif
    
public:
    // Constructor may be set up on demand.
    bool HasConstructorSetup() const
    {
        return pConstructor.Get() != NULL;
    }
    const Class& GetConstructor() const;
    Class& GetConstructor();

    //
    const Traits* GetParent() const
    {
        return pParent;
    }

    //
    bool IsParentOf(const Traits& other) const;

    //
    const Class& GetClass() const
    {
        return GetConstructor();
    }
    Class& GetClass()
    {
        return GetConstructor();
    }

    //
    BuiltinTraitsType GetTraitsType() const { return TraitsType; }

    //
    enum QNameFormat { qnfWithColons, qnfWithDot };
    virtual ASString GetName() const = 0;
    virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const = 0;

    ASString GetName(AbsoluteIndex ind) const
    {
        return Slots::GetName(ind);
    }

public:
    bool IsDynamic() const
    {
        return OT == DynamicObject;
    }
    bool IsSealed() const
    {
        return !IsDynamic();
    }
    bool IsArrayLike() const
    {
        return ArrayLike;
    }
    bool IsClassTraits() const
    {
        return TraitsType == Traits_Class;
    }
    bool IsInterface() const
    {
        return IsInterface_;
    }

public:
    // Should never be NULL.
    Instances::Namespace& GetPublicNamespace() const;
    // May be NULL.
    const Instances::Namespace* GetProtectedNamespace() const
    {
        return ProtectedNamespace;
    }
    // Should never be NULL.
    const Instances::Namespace& GetPrivateNamespace();

public:
    VM& GetVM() const
    {
        SF_ASSERT(pVM);
        return *pVM;
    }
    const VTable& GetVT() const
    {
        return *pVTable;
    }
    virtual VMAbcFile* GetFilePtr() const;
    bool IsUserDefined() const { return (GetFilePtr() != NULL); }
    StringManager& GetStringManager() const;
    GC& GetGC() const;
    Traits& GetSelf()
    {
        return *this;
    }
    // Get number of fixed slots, which are stored in an array of Values.
    UPInt GetFixedValueSlotNumber() const
    {
        return FixedValueSlotNumber;
    }
    const ScopeStackType& GetStoredScopeStack() const;

    ASString GetASString(const char *pstr) const;
    ASString GetASString(const char *pstr, UPInt length) const;
    ASString GetASString(const String& str) const;

public:
    ///
    // Can throw exceptions.
    // Return false in case of a problem or an exception.
    CheckResult GetSlotValue(Value& value, const ASString& name, const Instances::Namespace& ns, Object* const obj = NULL) const 
    {
        const SlotInfo* si = Slots::FindSlotInfo(name, ns);

        return si != NULL && si->GetSlotValue(value, obj);
    }
    CheckResult GetSlotValueUnsafe(Value& value, const ASString& name, const Instances::Namespace& ns, Object* const obj = NULL) const 
    {
        const SlotInfo* si = Slots::FindSlotInfo(name, ns);

        return si != NULL && si->GetSlotValueUnsafe(value, obj);
    }
    // Can throw exceptions.
    // Return false in case of a problem or an exception.
    CheckResult GetSlotValue(Value& value, AbsoluteIndex ind, Object* const obj = NULL) const 
    {
        return GetSlotInfo(ind).GetSlotValue(value, obj);
    }
    CheckResult GetSlotValueUnsafe(Value& value, AbsoluteIndex ind, Object* const obj = NULL) const 
    {
        return GetSlotInfo(ind).GetSlotValueUnsafe(value, obj);
    }
    
    ///
    // Can throw exceptions.        
    CheckResult SetSlotValue(const ASString& name, const Instances::Namespace& ns, const Value& v, Object* const obj = NULL)
    {
        SlotInfo* si = Slots::FindSlotInfo(name, ns);
        
        return si != NULL && si->SetSlotValue(GetVM(), v, obj);
    }
    // Can throw exceptions.        
    CheckResult SetSlotValue(AbsoluteIndex ind, const Value& v, Object* const obj = NULL)
    {
        return GetSlotInfo(ind).SetSlotValue(GetVM(), v, obj);
    }
    
public:
    const SlotInfo* FindSlotInfo(const ASString& name, const Instances::Namespace& ns) const
    {
        return Slots::FindSlotInfo(name, ns);
    }
    const SlotInfo* FindSlotInfo(const Multiname& mn) const 
    {
        return Slots::FindSlotInfo(mn.GetName(), mn.GetNamespace());
    }
    const SlotInfo* FindSlotInfo(VMAbcFile& file, const Abc::Multiname& mn) const;
    const SlotInfo& GetSlotInfo(AbsoluteIndex ind) const
    {
        return Slots::GetSlotInfo(ind);
    }

protected:
    SlotInfo* FindSlotInfo(const ASString& name, const Instances::Namespace& ns)
    {
        return Slots::FindSlotInfo(name, ns);
    }
    SlotInfo& GetSlotInfo(AbsoluteIndex ind)
    {
        return Slots::GetSlotInfo(ind);
    }

protected:
    void SetSlot(AbsoluteIndex index, const ASString& name, Pickable<const Instances::Namespace> ns,
                 const ClassTraits::Traits* tr, SlotInfo::DataType dt, 
                 AbsoluteIndex offset, bool _const = false);
    void SetSlot(AbsoluteIndex index, const ASString& name, Pickable<const Instances::Namespace> ns,
                 VMAbcFile& file, const Abc::TraitInfo& ti, SlotInfo::DataType dt, 
                 AbsoluteIndex offset, bool _const = false)
    {
        SF_ASSERT(index.Get() < GetSlots().GetSize());

        SetSlotInfo(
            index,
            name,
            SlotInfo(
                ns,
                file,
                ti,
                (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
                DEBUG_ARG(name.GetNode())
                )
            );

        SlotInfo& si = GetSlotInfo(index);
        si.Bind(dt, offset);
    }

    void AddSlot(const ASString& name, Pickable<const Instances::Namespace> ns,
                 const ClassTraits::Traits* tr, SlotInfo::DataType dt, 
                 AbsoluteIndex offset, bool _const);
    void AddSlot(const ASString& name, Pickable<const Instances::Namespace> ns,
                 VMAbcFile& file, const Abc::TraitInfo& ti, SlotInfo::DataType dt, 
                 AbsoluteIndex offset, bool _const)
    {
        AddSlot(
            name, 
            SlotInfo(
                ns, 
                file,
                ti,
                (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
                DEBUG_ARG(name.GetNode())
                )
            );
        GetTopSlotInfo().Bind(dt, offset);
    }

    void AddSlot(
        const ASString& name, Pickable<const Instances::Namespace> ns,
        SlotInfo::DataType dt, 
        AbsoluteIndex offset,
        bool _const = false
        )
    {
        AddSlot(
            name, 
            SlotInfo(
                ns, 
                NULL,
                (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
                DEBUG_ARG(name.GetNode())
            )
        );
        GetTopSlotInfo().Bind(dt, offset);
    }

public:
    void AddSlot(
        const char* name, 
        Pickable<const Instances::Namespace> ns,
        SlotInfo::DataType dt, 
        AbsoluteIndex offset,
        bool _const = false
        )
    {
        AddSlot(GetStringManager().CreateConstString(name), ns, dt, offset, _const);
    }

    void AddSlot(RelativeIndex ind, const ASString& name, Pickable<const Instances::Namespace> ns,
                 const ClassTraits::Traits* tr, SlotInfo::DataType dt, AbsoluteIndex offset,
                 bool _const = false);
    void AddSlot(RelativeIndex ind, const ASString& name, Pickable<const Instances::Namespace> ns,
        VMAbcFile& file, const Abc::TraitInfo& ti, SlotInfo::DataType dt, AbsoluteIndex offset,
        bool _const = false);

    // Add a bunch of traits ...
    void AddSlotsByPos(const Abc::HasTraits& traits, VMAbcFile& file);
    
public:
    void AddSlotValue(
        const ASString& name, 
        Pickable<const Instances::Namespace> ns
        )
    {
        AddSlot(name, ns, SlotInfo::DT_ValueArray, AbsoluteIndex(FixedValueSlotNumber++), false);
    }
    void AddSlotValue(
        const ASString& name, 
        Pickable<const Instances::Namespace> ns,
        const ClassTraits::Traits* tr
        )
    {
        AddSlot(name, ns, tr, SlotInfo::DT_ValueArray, AbsoluteIndex(FixedValueSlotNumber++), false);
    }
    
public:
    // Set up values of slots ...
    // This method is not *const* because we can change slot values for virtual methods (VTable) ...
    virtual void SetupSlotValues(Object& /*for_obj*/) const {}

public:
    GlobalSlotIndex GetNextPropertyIndex(GlobalSlotIndex ind) const
    {
        while (ind.Get() < GetSlots().GetSize())
        {
            ++ind;
            
            if (GetSlotInfo(AbsoluteIndex(ind)).GetAttrs() & SlotInfo::aDontEnum)
                continue;

            return ind;
        }
        
        return GlobalSlotIndex(0);
    }
    
    AbsoluteIndex GetFirstOwnSlotIndex() const
    {
        return FirstOwnSlotInd;
    }
    UPInt GetOwnSlotNumber() const
    {
        return OwnSlotNumber;
    }

public:
    virtual void ForEachChild_GC(GcOp op) const;
    void ForEachChild_GC(const Object& obj, GcOp op) const;
    
protected:
	// We need Classes::ClassClass to create new classes.
	Classes::ClassClass& GetClassClass() const;
    const Value& GetGlobalObject() const;
    // SetConstructor() is supposed to be called from ClassClass only ...
    void SetConstructor(Class* pc)
    {
        SF_ASSERT(pConstructor == NULL);
        pConstructor = pc;
    }
//     const Abc::ConstPool& GetConstPool() const;

    void SetArrayLike(bool flag = true)
    {
        ArrayLike = flag;
    }
    void SetIsInterface(bool flag = true)
    {
        IsInterface_ = flag;
    }


protected:
    void AddSlotsByName(const Abc::HasTraits& t, VMAbcFile& file);

    // Set up values of slots ...
    void SetupSlotValues(VMAbcFile& file, const Abc::HasTraits& t, Object& for_obj) const;
    
    // Adds alias-slots for methods declared in the interface.
    // Must be called after actual definitions were added (AddSlotsByName).
    // ClassTraits and InstanceTraits have *interfaces*.
    // Can throw exceptions.
    void AddInterfaceSlots(VMAbcFile& file, const Abc::ClassInfo& class_info);

protected:
    virtual void InitOnDemand() const;
    void SetProtectedNamespace(Pickable<Instances::Namespace> ns);

protected:
    VTable& GetVT()
    {
        return *pVTable;
    }
    
    void Add2VT(const ThunkInfo& func);
    void SetTraitsType(BuiltinTraitsType tt)
    {
        TraitsType = tt;
    }

private:
    void AddSlot(const ASString& name, const SlotInfo& slot)
    {
        AddSlotInfo(name, slot);

        // Accounting ...
        ++OwnSlotNumber;
    }

    void Add2VTableStoredScopeStack(const Value& o);
    void StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss);

    // Add Virtual Table value ...
    void Add2VT(
        VTable& vt, 
        SlotInfo& si, 
        const Value& v, 
        SlotInfo::DataType new_bt
        );

    // This method should be used only in case of UserDefined traits.
    void SetParentTraits(const Traits* pt)
    {
        SF_ASSERT(pParent.Get() == NULL);
        pParent.SetPtr(pt);
    }

protected:
    // It true object behaves as an array.
    bool ArrayLike;
    // This class is interface.
    bool IsInterface_;
    // Index of the very first not inherited slot ...
    AbsoluteIndex   FirstOwnSlotInd;
    UPInt           OwnSlotNumber;
    // Number of fixed slots, which are stored in an array of Values.
    UPInt           FixedValueSlotNumber;
    
private:
    VM*                 pVM;
    ObjectType          OT;
    // It is used to speed up type info retrieval.
    BuiltinTraitsType   TraitsType;
    
    // A class, which created this object ...
    // constructor is *not const* on purpose ...
    SPtr<Class>         pConstructor;
    SPtr<const Traits>  pParent;
    AutoPtr<VTable>     pVTable;
    
    // !!! Those two should go into InstanceTraits.
    SPtr<Instances::Namespace> ProtectedNamespace;
    SPtr<Instances::Namespace> PrivateNamespace;
}; // class Traits

namespace ClassTraits
{
    class Traits : public AS3::Traits
    {
        friend class AS3::VM; // Because of SetInstanceTraits().
        friend class ClassClass; // Because of Traits(VM& vm) ...

    public:
        Traits(VM& vm, const ClassTraits::Traits* pt);

    public:
        virtual void ForEachChild_GC(GcOp op) const;
        virtual void MakeObject(Value& result);
        void RegisterSlots();

        const Traits* GetParent() const
        {
            return static_cast<const Traits*>(AS3::Traits::GetParent());
        }
        InstanceTraits::Traits& GetInstanceTraits() const
        {
            SF_ASSERT(IsValid());
            return *ITraits;
        }
        bool IsValid() const
        {
            return ITraits.Get() != NULL;
        }

    public:
        // From documentation to the *istype* opcode:
        // let valueType refer to *this* type. If valueType is the same as otherType, result
        // is true. If otherType is a base type of valueType, or an implemented interface of valueType,
        // then result is true. Otherwise result is set to false.
        bool IsParentTypeOf(const Traits& other) const;
        bool IsOfType(const Traits& other) const
        {
            return other.IsParentTypeOf(*this);
        }

    public:
        // Coerce *value* to this type (value of this class) ...
        virtual bool Coerce(const Value& value, Value& result) const;
        bool CoerceValue(const Value& value, Value& result) const;

        virtual ASString GetName() const;
        virtual ASString GetQualifiedName(QNameFormat f = qnfWithColons) const;

    protected:
        void SetInstanceTraits(InstanceTraits::Traits& itr);
        template <typename OT>
        void SetInstanceTraits(Pickable<OT> itr)
        {
            SetIsInterface(itr->IsInterface());
            ITraits = itr;
        }

    private:
        // This constructor is called by ClassClass only ...
        explicit Traits(VM& vm);

    private:
        // Instance traits are presented in Class traits only.
        SPtr<InstanceTraits::Traits>    ITraits;
    };
}

///////////////////////////////////////////////////////////////////////////
inline
const ClassTraits::Traits& Class::GetClassTraits() const
{
    return static_cast<const ClassTraits::Traits&>(GetTraits());
}

inline
ClassTraits::Traits& Class::GetClassTraits()
{
    return static_cast<ClassTraits::Traits&>(GetTraits());
}

inline
const InstanceTraits::Traits& Class::GetInstanceTraits() const
{
    return GetClassTraits().GetInstanceTraits();
}

inline
InstanceTraits::Traits& Class::GetInstanceTraits()
{
    return GetClassTraits().GetInstanceTraits();
}

inline
InstanceTraits::Traits& Class::GetNewObjectTraits() const
{
    return GetClassTraits().GetInstanceTraits();
}

///////////////////////////////////////////////////////////////////////////
inline
const VTable& Object::GetVT() const
{
    return GetTraits().GetVT();
}

inline
void Object::SetTraits(Traits& t)
{
    pTraits.SetPtr(&t);
}

inline
BuiltinTraitsType Object::GetTraitsType() const
{
    return GetTraits().GetTraitsType();
}

inline
bool Object::IsDynamic() const
{
    return GetTraits().IsDynamic();
}

inline
const Class& Object::GetClass() const
{
    return GetTraits().GetConstructor();
}

inline
Class& Object::GetClass()
{
    return GetTraits().GetConstructor();
}

inline 
VM& Object::GetVM() const
{
    return GetTraits().GetVM();
}

inline 
CheckResult Object::GetSlotValue(SlotIndex ind, Value& value)
{
	return GetTraits().GetSlotValue(value, AbsoluteIndex(ind, GetTraits()), this);
}

inline 
CheckResult Object::GetSlotValueUnsafe(SlotIndex ind, Value& value)
{
    return GetTraits().GetSlotValueUnsafe(value, AbsoluteIndex(ind, GetTraits()), this);
}

inline 
CheckResult Object::GetSlotValue(GlobalSlotIndex ind, Value& value)
{
    return GetTraits().GetSlotValue(value, AbsoluteIndex(ind), this);
}

inline 
CheckResult Object::GetSlotValueUnsafe(GlobalSlotIndex ind, Value& value)
{
    return GetTraits().GetSlotValueUnsafe(value, AbsoluteIndex(ind), this);
}

///////////////////////////////////////////////////////////////////////////
inline
AbsoluteIndex::AbsoluteIndex(RelativeIndex ind, const Traits& t)
: Index(ind.Get() + t.GetFirstOwnSlotIndex().Get())
{
}

inline
AbsoluteIndex::AbsoluteIndex(SlotIndex ind, const Traits& t)
: Index(ind.Get() - 1 + t.GetFirstOwnSlotIndex().Get())
{
}

inline
AbsoluteIndex::AbsoluteIndex(GlobalSlotIndex ind)
: Index(ind.Get() - 1)
{
}

///////////////////////////////////////////////////////////////////////////
inline
RelativeIndex::RelativeIndex(AbsoluteIndex ind, const Traits& t)
: Index(ind.Get() - t.GetFirstOwnSlotIndex().Get())
{
}

///////////////////////////////////////////////////////////////////////////
inline
SlotIndex::SlotIndex(GlobalSlotIndex ind, const Traits& t)
: Index(ind.Get() - t.GetFirstOwnSlotIndex().Get())
{
    SF_ASSERT(ind.Get() > t.GetFirstOwnSlotIndex().Get());
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_Traits_H

