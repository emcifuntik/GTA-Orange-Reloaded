/**********************************************************************

Filename    :   AS3_Slot.cpp
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

#include "AS3_Object.h"
#include "AS3_VTable.h"
#include "AS3_VM.h"
#include "AS3_Slot.h"
#include "Obj/AS3_Obj_Namespace.h"


namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
SlotInfo::SlotInfo()
: Attr(aDontEnum)
, CTraits(NULL)
, File(NULL)
, TI(NULL)
, BindingType(DT_Unknown)
, ValueInd(-1)
{
}

SlotInfo::SlotInfo(
    Pickable<const Instances::Namespace> ns, 
    const ClassTraits::Traits* ctraits,
    int a
    DEBUG_ARG(const Ptr<ASStringNode>& name)
    )
: Attr(a)
, pNs(ns)
, CTraits(ctraits)
, File(NULL)
, TI(NULL)
, BindingType(DT_Unknown)
, ValueInd(-1)
DEBUG_ARG(Name(name))
{
}

SlotInfo::SlotInfo(
     Pickable<const Instances::Namespace> ns,
     VMAbcFile& file,
     const Abc::TraitInfo& ti,
     int a
     DEBUG_ARG(const Ptr<ASStringNode>& name)
     )
: Attr(a)
, pNs(ns)
, File(&file)
, TI(&ti)
, BindingType(DT_Unknown)
, ValueInd(-1)
DEBUG_ARG(Name(name))
{
}

SlotInfo::SlotInfo(const SlotInfo& other)
: Attr(other.Attr)
, pNs(other.pNs)
, CTraits(other.CTraits)
, File(other.File)
, TI(other.TI)
, BindingType(other.BindingType)
, ValueInd(other.ValueInd)
DEBUG_ARG(Name(other.Name))
{
}

SlotInfo::~SlotInfo()
{
}

SlotInfo& SlotInfo::operator =(const SlotInfo& other)
{
    if (this != &other)
    {
        Attr = other.Attr;
        pNs = other.pNs;
        CTraits = other.CTraits;
        File = other.File;
        TI = other.TI;
        BindingType = other.BindingType;
        ValueInd = other.ValueInd;
        DEBUG_CODE(Name = other.Name;)
    }
    
    return *this;
}

bool SlotInfo::IsConst() const
{
    using namespace Abc;

    return IsReadonly() || (TI && (TI->GetType() == TraitInfo::tConst));
}

bool SlotInfo::IsClass() const
{
    using namespace Abc;

    return TI && (TI->GetType() == TraitInfo::tClass);
}

bool SlotInfo::IsClassOrConst() const
{
    using namespace Abc;

    bool result = IsReadonly();
    if (!result && TI)
    {
        const TraitInfo::Type type = TI->GetType();
        result = (type == TraitInfo::tClass || type == TraitInfo::tConst);
    }

    return result;
}

CheckResult SlotInfo::GetSlotValue(VM& vm, Value& value, const Value& _this, const VTable* vt) const
{
    SF_ASSERT(ValueInd.IsValid());

    switch(BindingType)
    {
    case DT_ValueArray:
        value = _this.GetObject()->GetDynamicValue(ValueInd);
        break;
    case DT_Value:
        value = *(Value*)((char*)_this.GetObject() + ValueInd.Get());
        break;
    case DT_Object:
        {
            SPtr<Object>& o = *(SPtr<Object>*)((char*)_this.GetObject() + ValueInd.Get());

            value = o;
        }
        break;
    case DT_Boolean:
        value.SetBool(*(bool*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Int:
        value.SetSInt32(*(SInt32*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_UInt:
        value.SetUInt32(*(UInt32*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Number:
        value.SetNumber(*(Value::Number*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_String:
        value = *(ASString*)((char*)_this.GetObject() + ValueInd.Get());
        break;
    case DT_Code:
        if (vt == NULL)
            vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

        // Original version.
        if (_this.IsObject())
            vt->GetMethod(value, ValueInd, *_this.GetObject());
        else
            value = vt->Get(ValueInd);

        break;
    case DT_Get:
    case DT_GetSet:
        {
            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

            const Value& getter = vt->Get(ValueInd);

            if (getter.IsCallable())
                vm.Execute(getter, _this, value, 0, NULL);
            else
            {
                vm.ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Set:
        // Error.
        vm.ThrowReferenceError(VM::eWriteOnlyError DEBUG_ARG("Property doesn't have a getter method defined."));
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::GetSlotValueUnsafe(VM& vm, Value& value, const Value& _this, const VTable* vt) const
{
    SF_ASSERT(ValueInd.IsValid());

    switch(BindingType)
    {
    case DT_ValueArray:
        value.AssignUnsafe(_this.GetObject()->GetDynamicValue(ValueInd));
        break;
    case DT_Value:
        value.AssignUnsafe(*(Value*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Object:
        {
            SPtr<Object>& o = *(SPtr<Object>*)((char*)_this.GetObject() + ValueInd.Get());

            value.AssignUnsafe(o);
        }
        break;
    case DT_Boolean:
        value.SetBoolUnsafe(*(bool*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Int:
        value.SetSInt32Unsafe(*(SInt32*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_UInt:
        value.SetUInt32Unsafe(*(UInt32*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Number:
        value.SetNumberUnsafe(*(Value::Number*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_String:
        value.AssignUnsafe(*(ASString*)((char*)_this.GetObject() + ValueInd.Get()));
        break;
    case DT_Code:
        if (vt == NULL)
            vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

        // Original version.
        if (_this.IsObject())
            vt->GetMethod(value, ValueInd, *_this.GetObject());
        else
            value.AssignUnsafe(vt->Get(ValueInd));

        break;
    case DT_Get:
    case DT_GetSet:
        {
            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

            const Value& getter = vt->Get(ValueInd);

            if (getter.IsCallable())
                vm.Execute(getter, _this, value, 0, NULL);
            else
            {
                vm.ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Set:
        // Error.
        vm.ThrowReferenceError(VM::eWriteOnlyError DEBUG_ARG("Property doesn't have a getter method defined."));
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::GetSlotValue(Value& value, Object* obj) const
{
    SF_ASSERT(ValueInd.IsValid());
    SF_ASSERT(obj);
    
    switch(BindingType)
    {
    case DT_ValueArray:
        // !!! Should be SlotIndex ...
        value = obj->GetDynamicValue(ValueInd);
        break;
    case DT_Value:
        value = *(Value*)((char*)obj + ValueInd.Get());
        break;
    case DT_Object:
    {
        SPtr<Object>& o = *(SPtr<Object>*)((char*)obj + ValueInd.Get());
        
        value = o;
    }
        break;
    case DT_Boolean:
        value.SetBool(*(bool*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Int:
        value.SetSInt32(*(SInt32*)((char*)obj + ValueInd.Get()));
        break;
    case DT_UInt:
        value.SetUInt32(*(UInt32*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Number:
        value.SetNumber(*(Value::Number*)((char*)obj + ValueInd.Get()));
        break;
    case DT_String:
        value = *(ASString*)((char*)obj + ValueInd.Get());
        break;
    case DT_Code:
        // Be careful in case of a closure. "obj" is used to create a methods closure.
        obj->GetVT().GetMethod(value, ValueInd, *obj);
        break;
    case DT_Get:
    case DT_GetSet:
        {
            const Value& getter = obj->GetVT().Get(ValueInd);
            
            if (getter.IsCallable())
                obj->GetVM().Execute(getter, Value(obj), value, 0, NULL);
            else
            {
                obj->GetVM().ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Set:
        // Error.
        obj->GetVM().ThrowReferenceError(VM::eWriteOnlyError DEBUG_ARG("Property doesn't have a getter method defined."));
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::GetSlotValueUnsafe(Value& value, Object* obj) const
{
    SF_ASSERT(ValueInd.IsValid());
    SF_ASSERT(obj);

    switch(BindingType)
    {
    case DT_ValueArray:
        // !!! Should be SlotIndex ...
        value.AssignUnsafe(obj->GetDynamicValue(ValueInd));
        break;
    case DT_Value:
        value.AssignUnsafe(*(Value*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Object:
        {
            SPtr<Object>& o = *(SPtr<Object>*)((char*)obj + ValueInd.Get());

            value.AssignUnsafe(o);
        }
        break;
    case DT_Boolean:
        value.SetBoolUnsafe(*(bool*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Int:
        value.SetSInt32Unsafe(*(SInt32*)((char*)obj + ValueInd.Get()));
        break;
    case DT_UInt:
        value.SetUInt32Unsafe(*(UInt32*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Number:
        value.SetNumberUnsafe(*(Value::Number*)((char*)obj + ValueInd.Get()));
        break;
    case DT_String:
        value.AssignUnsafe(*(ASString*)((char*)obj + ValueInd.Get()));
        break;
    case DT_Code:
        // Be careful in case of a closure. "obj" is used to create a methods closure.
        obj->GetVT().GetMethod(value, ValueInd, *obj);
        break;
    case DT_Get:
    case DT_GetSet:
        {
            const Value& getter = obj->GetVT().Get(ValueInd);

            if (getter.IsCallable())
                obj->GetVM().Execute(getter, Value(obj), value, 0, NULL);
            else
            {
                obj->GetVM().ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Set:
        // Error.
        obj->GetVM().ThrowReferenceError(VM::eWriteOnlyError DEBUG_ARG("Property doesn't have a getter method defined."));
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::SetSlotValue(VM& vm, const Value& v, const Value& _this, const VTable* vt)
{
    SF_ASSERT(ValueInd.IsValid());

    // A ReferenceError is thrown if the property is const
    if (IsReadonly() && !vm.IsInInitializer())
    {
        // const properties can be assigned in initializers.
        vm.ThrowReferenceError(VM::eConstWriteError DEBUG_ARG("Property is const."));
        return false;
    }

    // Coerce to a slot data type.
    Value r;
    if (
        v.IsClass() || 
        v.IsFunction() || // GetDataType() for functions doesn't work.
        BindingType == DT_Get ||
        BindingType == DT_Set ||
        BindingType == DT_GetSet
        )
    {
        // Do not coerce classes.
        // Do not coerce Functions for the time being.
        r = v;
    } 
    else
    {
        if (!GetDataType(vm).Coerce(v, r))
        {
            vm.ThrowTypeError(VM::eCheckTypeFailedError);
            return false;
        }
    }

    switch(BindingType)
    {
    case DT_ValueArray:
        _this.GetObject()->SetValue(ValueInd, r);
        break;
    case DT_Value:
        _this.GetObject()->Set(*(Value*)((char*)_this.GetObject() + ValueInd.Get()), r);
        break;
    case DT_Object:
        _this.GetObject()->Set(*(SPtr<Object>*)((char*)_this.GetObject() + ValueInd.Get()), r);
        break;
    case DT_Boolean:
        *(bool*)((char*)_this.GetObject() + ValueInd.Get()) = r.AsBool();
        break;
    case DT_Int:
        *(SInt32*)((char*)_this.GetObject() + ValueInd.Get()) = r.AsInt();
        break;
    case DT_UInt:
        *(unsigned int*)((char*)_this.GetObject() + ValueInd.Get()) = r.AsUInt();
        break;
    case DT_Number:
        *(Value::Number*)((char*)_this.GetObject() + ValueInd.Get()) = r.AsNumber();
        break;
    case DT_String:
        *(ASString*)((char*)_this.GetObject() + ValueInd.Get()) = r.AsString();
        break;
    case DT_Get:
        // Error.
        vm.ThrowReferenceError(VM::eConstWriteError DEBUG_ARG("Property doesn't have a setter method defined."));
        return false;
    case DT_Set:
    case DT_GetSet:
        {
            if (vt == NULL)
                vt = &const_cast<const Traits&>(vm.GetValueTraits(_this)).GetVT();

            const Value& setter = vt->Get(AbsoluteIndex(ValueInd.Get() + 1));

            if (setter.IsCallable())
            {
                Value result;

                vm.Execute(setter, _this, result, 1, &r);
                if (vm.IsException())
                    return false;
            } else
            {
                vm.ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Code:
        //if (_this.IsPrimitive() || vm.GetValueTraits(_this).IsSealed())
        if (_this.IsPrimitive())
        {
            // It looks like Tamarin works this way.
            // Primitive value is a sealed object.
            vm.ThrowReferenceError(
                VM::eWriteSealedError 
                DEBUG_ARG("Property is unresolved and object is not dynamic"));
        }
        else
        {
            // Sealed objects, which are not primitives seem to throw eCannotAssignToMethodError.
            vm.ThrowReferenceError(VM::eCannotAssignToMethodError DEBUG_ARG("Cannot assign to a method in virtual table."));
        }
        return false;
    default:
        return false;
    }

    return true;
}

CheckResult SlotInfo::SetSlotValue(VM& vm, const Value& v, Object* const obj)
{
    SF_ASSERT(ValueInd.IsValid());
    SF_ASSERT(obj);

    // A ReferenceError is thrown if the property is const
    if (IsReadonly() && !vm.IsInInitializer())
    {
        // const properties can be assigned in initializers.
        vm.ThrowReferenceError(VM::eConstWriteError DEBUG_ARG("Property is const."));
        return false;
    }

    // Coerce to a slot data type.
    Value r;
    if (
        v.IsClass() || 
        //v.IsFunction() || // GetDataType() for functions doesn't work.
        BindingType == DT_Get ||
        BindingType == DT_Set ||
        BindingType == DT_GetSet
        )
    {
        // Do not coerce classes.
        // Do not coerce Functions for the time being.
        r = v;
    } 
    else
    {
        if (!GetDataType(vm).Coerce(v, r))
        {
            vm.ThrowTypeError(VM::eCheckTypeFailedError);
            return false;
        }
    }
    
    switch(BindingType)
    {
    case DT_ValueArray:
        obj->SetValue(ValueInd, r);
        break;
    case DT_Value:
        obj->Set(*(Value*)((char*)obj + ValueInd.Get()), r);
        break;
    case DT_Object:
        obj->Set(*(SPtr<Object>*)((char*)obj + ValueInd.Get()), r);
        break;
    case DT_Boolean:
        *(bool*)((char*)obj + ValueInd.Get()) = r.AsBool();
        break;
    case DT_Int:
        *(SInt32*)((char*)obj + ValueInd.Get()) = r.AsInt();
        break;
    case DT_UInt:
        *(unsigned int*)((char*)obj + ValueInd.Get()) = r.AsUInt();
        break;
    case DT_Number:
        *(Value::Number*)((char*)obj + ValueInd.Get()) = r.AsNumber();
        break;
    case DT_String:
        // In case of Abc::StringID ...
        *(ASString*)((char*)obj + ValueInd.Get()) = r.AsString();
        break;
    case DT_Get:
        // Error.
        vm.ThrowReferenceError(VM::eConstWriteError DEBUG_ARG("Property doesn't have a setter method defined."));
        break;
    case DT_Set:
    case DT_GetSet:
        {
            const Value& setter = obj->GetVT().Get(AbsoluteIndex(ValueInd.Get() + 1));
            
            if (setter.IsCallable())
            {
                Value result;
                
                vm.Execute(setter, Value(obj), result, 1, &r);
                if (vm.IsException())
                    return false;
            } else
            {
                vm.ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Function object expected."));
                return false;
            }
        }
        break;
    case DT_Code:
        vm.ThrowReferenceError(VM::eCannotAssignToMethodError DEBUG_ARG("Cannot assign to a method in virtual table."));
        return false;
    default:
        return false;
    }

    return true;
}

void SlotInfo::ForEachChild_GC(const Object& obj, RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    switch(BindingType)
    {
    case DT_ValueArray:
        AS3::ForEachChild_GC(obj.Values[ValueInd.Get()], op);
        break;
    case DT_Value:
        AS3::ForEachChild_GC(*(Value*)((char*)&obj + ValueInd.Get()), op);
        break;
    case DT_Object:
        {
            SPtr<Object>& ptr = *(SPtr<Object>*)((char*)&obj + ValueInd.Get());
            
            if (ptr)
                ptr->ForEachChild_GC(op);
        }
        break;
    case DT_Boolean:
    case DT_Int:
    case DT_UInt:
    case DT_Number:
    case DT_String:
    case DT_Get:
    case DT_Set:
    case DT_GetSet:
    case DT_Code:
        break;
    default:
        SF_ASSERT(false);
        break;
    }
}

const ClassTraits::Traits& SlotInfo::GetDataType(VM& vm)
{
    if (CTraits.Get() == NULL)
    {
        if (File != NULL && TI != NULL)
        {
            // Delayed type resolution.
            if (TI->IsData())
            {
                const Abc::Multiname& mn = TI->GetTypeName(File->GetAbcFile());
                const Multiname as3_nm(*File, mn);

                CTraits.SetPtr(vm.Resolve2ClassTraits(as3_nm));
                SF_ASSERT(CTraits.Get());
            }
            else
                // !!! We probably should get result type here.
                CTraits.SetPtr(&vm.GetClassTraitsFunction());
        }
        else
        {
            switch (BindingType)
            {
            case DT_Unknown:
            case DT_ValueArray:
            case DT_Value:
            case DT_Object:
                CTraits.SetPtr(&vm.GetClassTraitsObject());
                break;
            case DT_Boolean:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsBoolean());
                break;
            case DT_Int:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsInt());
                break;
            case DT_UInt:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsUInt());
                break;
            case DT_Number:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsNumber());
                break;
            case DT_String:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsString());
                break;
            case DT_Code:
            case DT_Get:
            case DT_Set:
            case DT_GetSet:
                CTraits.SetPtr((const ClassTraits::Traits*)&vm.GetClassTraitsFunction());
                break;
            }
        }
    }

    SF_ASSERT(CTraits.Get());
    return *CTraits;
}

void SlotInfo::ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    AS3::ForEachChild_GC_Const<const Instances::Namespace, Mem_Stat>(pNs, op);
	AS3::ForEachChild_GC_Const<const ClassTraits::Traits, Mem_Stat>(CTraits, op);
}

void SlotInfo::setNamespace(const Instances::Namespace& ns)
{
    pNs = &ns;
}

///////////////////////////////////////////////////////////////////////////////
SlotContainerType::SlotContainerType()
: Size(0)
, FirstFreeInd(0)
{
}

SlotContainerType::SelfType& SlotContainerType::operator=( const SlotContainerType::SelfType& other )
{
    if (this != &other)
    {
        // Set Size to a temporary value ZERO to prevent possible issues with 
        // Garbage Collector.
        Size = 0;

        FirstFreeInd = other.FirstFreeInd;
        VArray = other.VArray;
        KArray = other.KArray;
        KVHashSet = other.KVHashSet;

        // Size MUST be changed after all arrays are changed because changing 
        // of array's size may call Garbage Collector, which will visit all 
        // elements of arrays.
        Size = other.Size;
    }

    return *this;
}

void SlotContainerType::Clear()
{
    // Order of changing Size and calling of Clear() is not important here because
    // Garbage Collector is not supposed to be called in this case.

    Size = 0;
    FirstFreeInd = 0;
    VArray.Clear();
    KArray.Clear();
    KVHashSet.Clear();
}

void SlotContainerType::ForEachChild_GC(RefCountBaseGC<Mem_Stat>::GcOp op) const
{
    // We MAY NOT use member Size here because of the Garbage Collector.
    // VArray and Size can be easily changed not synchronously.
    const UPInt size = VArray.GetSize();

    for(UPInt i = 0; i < size; ++i)
        VArray[i].ForEachChild_GC(op);
}

void SlotContainerType::SetKey(AbsoluteIndex ind, const ASString& k)
{
    const UPInt index = ind.Get();

    if (KArray[index] == k.GetNode())
        // The same key.
        return;

    // Remove old value index.
    {
        SetType::Iterator it = KVHashSet.Find(KArray[index]);
        IndSetType& indSet = it->Second;
        const UPInt size = indSet.GetSize();
        for (UPInt i = 0; i < size; ++i)
        {
            if (indSet[i] == index)
            {
                // We found reference to the old key.
                // Remove it.
                indSet.RemoveAt(i);
                break;
            }
        }
    }

    // Set new value.
    {
        ASStringNode* node = k.GetNode();
        KArray[index] = node;
        SetType::ValueType* indSet = KVHashSet.Get(node);

        if (indSet == NULL)
        {
            IndSetType indSet2(Memory::GetHeapByAddress(this));
            indSet2.PushBack(index);
            KVHashSet.Add(node, indSet2);
        }
        else
            indSet->PushBack(index);
    }
}

Scaleform::UPInt SlotContainerType::RemoveKey( const ASString& k )
{
    SetType::Iterator it = KVHashSet.Find(k.GetNode());

    if (it.IsEnd())
        return 0;

    IndSetType& indSet = it->Second;
    Alg::QuickSort(indSet);
    const UPInt size = indSet.GetSize();
    UPInt result;

    for (UPInt i = size; i > 0; --i)
    {
        // Unlink.
        result = indSet[i - 1];
        KArray[result] = NULL;
        VArray[result] = ValueType();

        if (result < FirstFreeInd)
            FirstFreeInd = result;
    }

    //it.Remove(); // Doesn't compile.
    //KVHashSet.Remove(k.GetNode());
    indSet.Clear(); // Clear instead of KVHashSet.Remove(k.GetNode()).

    return size;
}

Scaleform::GFx::AS3::AbsoluteIndex SlotContainerType::Add( const ASString& k, const ValueType& v )
{
    UPInt result;

    if (FirstFreeInd == Size)
    {
        // No free indices.
        // We need to add new record.

        result = VArray.GetSize();
        KArray.PushBack(k.GetNode());
        VArray.PushBack(v);

        // Size MUST be changed after all PushBack() calls because PushBack()
        // may call Garbage Collector, which will visit all elements of arrays.

        ++FirstFreeInd;
        ++Size;
    }
    else
    {
        // We can reuse erased records.
        // Size doesn't change.

        result = FirstFreeInd;

        // Assign value.
        KArray[result] = k.GetNode();
        VArray[result] = v;

        // Find next FirstFreeInd.
        for (++FirstFreeInd; FirstFreeInd < Size; ++FirstFreeInd)
        {
            if (KArray[FirstFreeInd].GetPtr() == NULL)
                break;
        }
    }

    SetType::Iterator it = KVHashSet.Find(k.GetNode());
    if (it.IsEnd())
    {
        IndSetType indSet(Memory::GetHeapByAddress(this));
        indSet.PushBack(result);
        KVHashSet.Add(k.GetNode(), indSet);
    } else
        it->Second.PushBack(result);

    return AbsoluteIndex(result);
}

void SlotContainerType::Swap( SelfType& other )
{
    // We want to prevent all possible problems related to the Garbage Collector.

    //const UPInt tmp_size = Alg::Min(Size, other.Size);
    const UPInt tmp_size = 0;
    const UPInt this_size = Size;
    const UPInt other_size = other.Size;

    // Set Size to a temporary value.
    Size = tmp_size;
    other.Size = tmp_size;

    // Swap arrays.
    Alg::Swap(FirstFreeInd, other.FirstFreeInd);
    Alg::Swap(VArray, other.VArray);
    Alg::Swap(KArray, other.KArray);
    Alg::Swap(KVHashSet, other.KVHashSet);

    // Change Size after we swapped arrays.
    Size = other_size;
    other.Size = this_size;
}

Scaleform::GFx::AS3::AbsoluteIndex SlotContainerType::FindAdd( const ASString& k, const ValueType& v )
{
    AbsoluteIndex result(-1);
    SetType::ValueType* indSet = KVHashSet.Get(k.GetNode());

    if (indSet == NULL)
        result = Add(k, v);
    else if (indSet->IsEmpty())
        result = Add(k, v);
    else
        result = AbsoluteIndex(indSet->Front());

    return result;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


