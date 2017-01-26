/**********************************************************************

Filename    :   AS3_Object.cpp
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
#include "AS3_VM.h"
#include "AS3_VTable.h"
#include "Obj/AS3_Obj_Function.h"
#include "Obj/AS3_Obj_Error.h"
#include "Obj/AS3_Obj_Namespace.h"
#include "AS3_AsString.h" // For logs.
#include "GFx/GFx_ASUtils.h" // for ASNumberUtil::IsNaN, etc


#ifdef si_ptr
#undef si_ptr
#endif

namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
Value GetDetailValue(VMAbcFile& file, const Abc::ValueDetail& d)
{   
    const int value_ind = d.GetIndex();
    
    // If we have a value ...
    if (value_ind > 0)
    {
        switch(d.GetKind())
        {
        case Abc::CONSTANT_Int:
            return Value(file.GetConstPool().GetInt(value_ind));
        case Abc::CONSTANT_UInt:
            return Value(file.GetConstPool().GetUInt(value_ind));
        case Abc::CONSTANT_Double:
            return Value(MakeValueNumber(file.GetConstPool().GetDouble(value_ind)));
        case Abc::CONSTANT_Utf8:
            return Value(
                file.GetVM().GetStringManager().CreateString(
                    file.GetConstPool().GetString(AbsoluteIndex(value_ind))
                ));
        case Abc::CONSTANT_True:
            return Value(true);
        case Abc::CONSTANT_False:
            return Value(false);
        case Abc::CONSTANT_Null:
            return Value::GetNull();
        case Abc::CONSTANT_Undefined:
            return Value::GetUndefined();
        case Abc::CONSTANT_Namespace:
        case Abc::CONSTANT_PackageNamespace:
        case Abc::CONSTANT_PackageInternalNs:
        case Abc::CONSTANT_ProtectedNamespace:
        case Abc::CONSTANT_ExplicitNamespace:
        case Abc::CONSTANT_StaticProtectedNs:
        case Abc::CONSTANT_PrivateNs:
            return Value(&file.GetInternedNamespace(d.GetIndex()));
        default:
            SF_ASSERT(false);
            break;
        }
    }
    
    return Value::GetUndefined();
}

///////////////////////////////////////////////////////////////////////////
// ??? Can be a method ...
bool HasPublicNamespace(const Abc::ConstPool& cp, const Abc::Multiname& mn)
{
    // QName case ...
    if (mn.IsQName())
        return mn.GetNamespace(cp).IsPublic();
    
    // Multiname case ...
    const Abc::NamespaceSetInfo& nss = mn.GetNamespaceSetInfo(cp);
    for(UPInt i = 0; i < nss.GetSize(); ++i)
    {
        if (nss.Get(cp, i).IsPublic())
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
// Value related section.

void Value::AddRefInternal() const
{
    KindType k = GetKind();
    switch(k)
    {
    case kString:
        value.VS._1.VStr->AddRef();

        break;
    case kNamespace:
        if (value.VS._1.VNs)
            value.VS._1.VNs->AddRef();

        break;
    case kMethodClosure:
        if (value.VS._2.VObj)
            value.VS._2.VObj->AddRef();

        if (value.VS._1.VFunct)
            value.VS._1.VFunct->AddRef();

        break;
    case kClass:
    case kFunction:
    case kThunkFunction:
    case kObject:
        if (value.VS._1.VObj)
            value.VS._1.VObj->AddRef();

        break;
    case kThunkClosure:
        if (value.VS._2.VObj)
            value.VS._2.VObj->AddRef();

        break;
    default:
        break;
    }
}

void Value::ReleaseInternal()
{
    switch(GetKind())
    {
    case kString:
        value.VS._1.VStr->Release();
        break;
    case kNamespace:
        if ((UPInt)value.VS._1.VNs & 1)
            value.VS._1.VNs = (Instances::Namespace*)((UPInt)value.VS._1.VNs - 1);
        else if (value.VS._1.VNs)
            value.VS._1.VNs->Release();

        break;
    case kMethodClosure:
        // Object.
        if ((UPInt)value.VS._2.VObj & 1)
            value.VS._2.VObj = (Object*)((UPInt)value.VS._2.VObj - 1);
        else if (value.VS._2.VObj)
            value.VS._2.VObj->Release();

        // Function.
        if ((UPInt)value.VS._1.VFunct & 1)
            value.VS._1.VFunct = (Instances::Function*)((UPInt)value.VS._1.VFunct - 1);
        else if (value.VS._1.VFunct)
            value.VS._1.VFunct->Release();

        break;
    case kClass:
    case kFunction:
    case kThunkFunction:
    case kObject:
        // Object.
        if ((UPInt)value.VS._1.VObj & 1)
            value.VS._1.VObj = (Object*)((UPInt)value.VS._1.VObj - 1);
        else if (value.VS._1.VObj)
            value.VS._1.VObj->Release();

        break;
    case kThunkClosure:
        // Object.
        if ((UPInt)value.VS._2.VObj & 1)
            value.VS._2.VObj = (Object*)((UPInt)value.VS._2.VObj - 1);
        else if (value.VS._2.VObj)
            value.VS._2.VObj->Release();

        break;
    default:
        break;
    }
}

CheckResult Value::Convert2PrimitiveValue(Value& result, Hint hint) const
{
	if (!IsPrimitive() && !IsNull())
    {
        if (IsThunk())
        {
            // A hack.
            //result = sm.CreateConstString("function Function() {}");
            result.SetNumber(0);

            // No exceptions.
            return true;
        }

        // !!! GetDefaultValue() can throw exceptions.
        GetObject()->GetDefaultValue(result, hint);

        if (GetObject()->GetVM().IsException())
            return false;
    }
	else
		result = *this;

    // No exceptions.
    return true;
}

///////////////////////////////////////////////////////////////////////////
NamespaceSet::NamespaceSet(VM& vm)
: GASRefCountBase(&vm.GetGC())
{
}

void NamespaceSet::ForEachChild_GC(GcOp op) const
{
    AS3::ForEachChild_GC(Namespaces, op);
}
    
bool NamespaceSet::Contains(const Instances::Namespace& ns) const
{
    for(UPInt i = 0; i < Namespaces.GetSize(); ++i)
    {
        if (&Namespaces[i].AsNamespace() == &ns)
            return true;
    }
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
Multiname::Multiname(Instances::Namespace& ns, const Value& name)
: Abc::HasMultinameKind(Abc::MN_QName)
, Obj(&ns)
, Name(name)
{
    SF_ASSERT(IsQName());
}

Multiname::Multiname(NamespaceSet& nss, const Value& name)
: Abc::HasMultinameKind(Abc::MN_Multiname)
, Obj(&nss)
, Name(name)
{
    SF_ASSERT(IsMultiname());
}

Multiname::Multiname(VMAbcFile& file, const Abc::Multiname& mn)
: Abc::HasMultinameKind(mn.GetKind())
{
    if (&mn != &file.GetConstPool().GetAnyType())
        Name = file.GetInternedString(mn.GetNameInd());

    // Ignore run-time multinames for the time being.
    if (mn.IsQName() && !mn.IsRunTime())
        // !!! It looks like we should use multiname here.
        //Obj = &file.GetInternedNamespace(mn);
        // !!! We do not handle "Any Namespace" here.
        Obj = &file.GetInternedNamespace(mn.GetNamespaceInd());
    else if (mn.IsMultiname())
        Obj = &file.GetInternedNamespaceSet(mn.GetNamespaceSetInd());
    else
    {
        // RTQName receive null object.
    }
}

Multiname::Multiname(const VM& vm, const char* qualifiedTypeName)
: Abc::HasMultinameKind(Abc::MN_QName)
{
    // Parse "flash.display::Stage" or "flash.display.Stage" into namespace + typename.
    StringDataPtr str(qualifiedTypeName);
    bool          colon = true;
    SPInt         dotIndex = str.FindLastChar(':');

    if (dotIndex < 0)
    {
        colon = false;
        dotIndex = str.FindLastChar('.');
    }

    const char*   name = str.GetTrimLeft(dotIndex + 1).ToCStr();

    if (dotIndex > 0)
    {
        Obj = vm.MakeInternedNamespace(Abc::NS_Public,
            vm.GetStringManager().CreateString(qualifiedTypeName, 
            (colon ? Alg::Max(0, (int)dotIndex - 1) : (int)dotIndex)));
    }
    else
        Obj = &vm.GetPublicNamespace();

    Name = vm.GetStringManager().CreateString(name);
}

bool Multiname::ContainsNamespace(const Instances::Namespace& ns) const
{
    if (!IsMultiname())
        return *static_cast<Instances::Namespace*>(Obj.Get()) == ns;
    return static_cast<NamespaceSet*>(Obj.Get())->Contains(ns);
}

// Used by op-stack Read() to resolve name components.
void Multiname::SetRTNamespace(Instances::Namespace& ns)
{
    SF_ASSERT(IsRunTime());
    Obj = &ns;
    setNamespaceBits(Abc::MN_NS_Qualified);
}

Instances::Namespace& Multiname::GetNamespace() const
{
    SF_ASSERT(IsQName());
    SF_ASSERT(Obj.Get());
    return *static_cast<Instances::Namespace*>(Obj.Get());
}

NamespaceSet& Multiname::GetNamespaceSet() const
{
    SF_ASSERT(IsMultiname());
    SF_ASSERT(Obj.Get());
    return *static_cast<NamespaceSet*>(Obj.Get());
}

///////////////////////////////////////////////////////////////////////////
PropRef FindScopeProperty(
    VM& vm,
    const UPInt baseSSInd,
    const ScopeStackType& scope_stack,
    const Multiname& mn
    )
{
    // [2.3.6]
    // Typically, the order of the search for resolving multinames is the object’s declared traits, 
    // its dynamic properties, and finally the prototype chain.
    // The dynamic properties and prototype chain search will only happen if the
    // multiname contains the public namespace (dynamic properties are always in the public namespace in
    // ActionScript 3.0; a run-time error is signaled if an attempt is add a non-public property).
    
    PropRef prop;
            
    // Look into the scope stack ...
    for (UPInt i = scope_stack.GetSize(); i > baseSSInd ; --i)
    {
        const Value& v = scope_stack[i - 1];

        /* For maniacs
        if (v.IsNullOrUndefined())
            continue;
        */

        prop = FindPropertyWith(vm, v, mn);
        
        if (prop)
            // For RVO sake ...
            break;
    }
    
    return prop;
}

const ClassTraits::Traits* FindScopeTraits(VM& vm, const UPInt baseSSInd, const ScopeStackType& scope_stack, const Multiname& mn)
{
    const ClassTraits::Traits* result = NULL;

    for (UPInt i = scope_stack.GetSize(); i > baseSSInd ; --i)
    {
        const Value& v = scope_stack[i - 1];
        const Traits& tr = vm.GetValueTraits(v);

        result = FindFixedTraits(vm, tr, mn);
        
        if (result)
            // For RVO sake ...
            break;
    }

    return result;
}

const SlotInfo* FindDynamicSlot(const SlotContainerType& sc, 
                                const ASString& name, const Instances::Namespace& ns)
{
    const SlotInfo* sli_ptr = NULL;

    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    if (values != NULL)
    {
        // Find a property with the same name and check namespace after that ...
        for (UPInt i = values->GetSize(); i > 0; --i)
        {            
            const SlotInfo& si = sc[AbsoluteIndex(values->At(i - 1))];
            const Instances::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns == ns)
            {
                sli_ptr = &si;
                break;
            }
        }
    }

    return sli_ptr;
}

// Future development.
const SlotInfo* FindDynamicSlotStrict(const SlotContainerType& sc, 
                                      const ASString& name, const Instances::Namespace& ns)
{
    const SlotInfo* sli_ptr = NULL;

    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    if (values != NULL)
    {
        // Find a property with the same name and check namespace after that ...
        for (UPInt i = values->GetSize(); i > 0; --i)
        {            
            const SlotInfo& si = sc[AbsoluteIndex(values->At(i - 1))];
            const Instances::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns == ns)
            {
                sli_ptr = &si;
                break;
            }
        }
    }

    return sli_ptr;
}

const SlotInfo* FindFixedSlot(const Traits& t, const ASString& name,
                              const Instances::Namespace& ns, UPInt& index, Object* obj)
{
    const SlotInfo* sli_ptr = NULL;
    const SlotContainerType& sc = t.GetSlots();

    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    if (values != NULL)
    {
        // Find a property with the same name and check namespace after that ...
        for (UPInt i = values->GetSize(); i > 0; --i)
        {
            index = values->At(i - 1);
            const SlotInfo& si = sc[AbsoluteIndex(index)];
            const Instances::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns.GetKind() == ns.GetKind())
            {
                bool found = false;

                switch (ns.GetKind())
                {
                case Abc::NS_Protected:
                    found = true;
                    break;
                case Abc::NS_Private:
                    found = (&ns == &cur_ns);
                    break;
                default:
                    found = (cur_ns.GetUri() == ns.GetUri());
                    break;
                }

                if (found)
                {
                    sli_ptr = &si;
                    break;
                }
            }
        }
    }

    // if (!sli_ptr && obj) will break VM.
    if (obj)
        sli_ptr = obj->InitializeOnDemand(sli_ptr, name, ns, index);

    return sli_ptr;
}

const ClassTraits::Traits* 
FindFixedTraits(const Traits& t, const ASString& name, const Instances::Namespace& ns, UPInt& index)
{
    const ClassTraits::Traits* result = NULL;

    const SlotInfo* si = FindFixedSlot(t, name, ns, index, NULL);
    if (si)
        result = &const_cast<SlotInfo*>(si)->GetDataType(t.GetVM());

    return result;
}

const SlotInfo* FindDynamicSlot(VM& vm, const SlotContainerType& sc, const Multiname& mn)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name, sm))
        // Exception.
        return NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
        return FindDynamicSlot(sc, name, mn.GetNamespace());

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const ValueArray& namespaces = mn.GetNamespaceSet().GetNamespaces();
    const SlotInfo* si = NULL;
    for(UPInt i = 0; i < namespaces.GetSize(); ++i)
    {
        si = FindDynamicSlot(sc, name, namespaces[i].AsNamespace());

        if (si)
            break;
    }

    return si;
}

const SlotInfo* FindFixedSlot(VM& vm, const Traits& t, const Multiname& mn, UPInt& index, Object* obj)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name, sm))
        // Exception.
        return NULL;

    const SlotInfo* si_ptr = NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
        return AS3::FindFixedSlot(t, name, mn.GetNamespace(), index, obj);

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const ValueArray& namespaces = mn.GetNamespaceSet().GetNamespaces();

    const SlotContainerType& sc = t.GetSlots();
    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    for(UPInt i = 0; i < namespaces.GetSize(); ++i)
    {
        //si_ptr = AS3::FindFixedSlot(GetTraits(), name, namespaces[i].AsNamespace());
        const Instances::Namespace& ns = namespaces[i].AsNamespace();

        if (values != NULL)
        {
            // Find a property with the same name and check namespace after that ...
            for (UPInt i = values->GetSize(); i > 0; --i)
            {
                index = values->At(i - 1);
                const SlotInfo& si = sc[AbsoluteIndex(index)];
                const Instances::Namespace& cur_ns = si.GetNamespace();

                if (cur_ns.GetKind() == ns.GetKind())
                {
                    bool found = false;

                    switch (ns.GetKind())
                    {
                    case Abc::NS_Protected:
                        found = true;
                        break;
                    case Abc::NS_Private:
                        found = (&ns == &cur_ns);

                        // Future development.
                        //                         if (!found && cur_ns.IsParentOf(ns))
                        //                         {
                        //                             // Private namespaces always have owners.
                        //                             vm.ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG("Attempt to access private property in a base class"));
                        //                             return NULL;
                        //                         }
                        break;
                    default:
                        found = (cur_ns.GetUri() == ns.GetUri());
                        break;
                    }

                    if (found)
                    {
                        si_ptr = &si;
                        break;
                    }
                }
            }
        }

        // if (!si_ptr && obj) will break VM.
        if (obj)
            si_ptr = obj->InitializeOnDemand(si_ptr, name, ns, index);

        if (si_ptr)
            break;
    }

    return si_ptr;
}

#if 0
const SlotInfo* FindFixedSlot(VMAbcFile& file, const Abc::Multiname& mn, const Traits& t, UPInt& index, Object* obj)
{
    const ASString name = file.GetInternedString(mn.GetNameInd());
    const SlotInfo* si_ptr = NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
    {
        const Instances::Namespace& ns = file.GetInternedNamespace(mn.GetNamespaceInd());

        return AS3::FindFixedSlot(t, name, ns, index, obj);
    }

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const NamespaceSet& nss = file.GetInternedNamespaceSet(mn.GetNamespaceSetInd());
    const ValueArray& namespaces = nss.GetNamespaces();

    const SlotContainerType& sc = t.GetSlots();
    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    for(UPInt i = 0; i < namespaces.GetSize(); ++i)
    {
        //si_ptr = AS3::FindFixedSlot(GetTraits(), name, namespaces[i].AsNamespace());
        const Instances::Namespace& ns = namespaces[i].AsNamespace();

        if (values != NULL)
        {
            // Find a property with the same name and check namespace after that ...
            for (UPInt i = values->GetSize(); i > 0; --i)
            {
                index = values->At(i - 1);
                const SlotInfo& si = sc[AbsoluteIndex(index)];
                const Instances::Namespace& cur_ns = si.GetNamespace();

                if (cur_ns.GetKind() == ns.GetKind())
                {
                    bool found = false;

                    switch (ns.GetKind())
                    {
                    case Abc::NS_Protected:
                        found = true;
                        break;
                    case Abc::NS_Private:
                        found = (&ns == &cur_ns);

// Future development.
//                         if (!found && cur_ns.IsParentOf(ns))
//                         {
//                             // Private namespaces always have owners.
//                             vm.ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG("Attempt to access private property in a base class"));
//                             return NULL;
//                         }
                        break;
                    default:
                        found = (cur_ns.GetUri() == ns.GetUri());
                        break;
                    }

                    if (found)
                    {
                        si_ptr = &si;
                        break;
                    }
                }
            }
        }

        // if (!si_ptr && obj) will break VM.
        if (obj)
            si_ptr = obj->InitializeOnDemand(si_ptr, name, ns, index);

        if (si_ptr)
            break;
    }

    return si_ptr;
}
#endif

const ClassTraits::Traits* FindFixedTraits(VM& vm, const Traits& t, const Multiname& mn)
{
    const Value& vn = mn.GetName();
    StringManager& sm = vm.GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name, sm))
        // Exception.
        return NULL;

    const ClassTraits::Traits* result = NULL;

    // QName case ...
    // [2.3.6] QName will resolve to the property with the same name and
    // namespace as the QName
    if (mn.IsQName())
    {
        UPInt index = 0;
        return AS3::FindFixedTraits(t, name, mn.GetNamespace(), index);
    }

    // Multiname case ...
    // [2.3.6] object is searched for any properties whose name is the same as
    // the multinames name, and whose namespace matches any of the namespaces in the multinames namespace set.
    // Since the multiname may have more than one namespace, there could be multiple properties that match the
    // multiname. If there are multiple properties that match a TypeError is raised
    const ValueArray& namespaces = mn.GetNamespaceSet().GetNamespaces();

    const SlotContainerType& sc = t.GetSlots();
    const SlotContainerType::SetType::ValueType* values = sc.FindValues(name);

    for(UPInt i = 0; i < namespaces.GetSize(); ++i)
    {
        //si_ptr = AS3::FindFixedSlot(GetTraits(), name, namespaces[i].AsNamespace());
        const Instances::Namespace& ns = namespaces[i].AsNamespace();

        if (values != NULL)
        {
            // Find a property with the same name and check namespace after that ...
            for (UPInt i = values->GetSize(); i > 0; --i)
            {
                const SlotInfo& si = sc[AbsoluteIndex(values->At(i - 1))];
                const Instances::Namespace& cur_ns = si.GetNamespace();

                if (cur_ns.GetKind() == ns.GetKind())
                {
                    bool found = false;

                    switch (ns.GetKind())
                    {
                    case Abc::NS_Protected:
                        found = true;
                        break;
                    case Abc::NS_Private:
                        found = (&ns == &cur_ns);

// Future development.
//                         if (!found && cur_ns.IsParentOf(ns))
//                         {
//                             // Private namespaces always have owners.
//                             vm.ThrowReferenceError(VM::ePropertyNotFoundError DEBUG_ARG("Attempt to access private property in a base class"));
//                             return NULL;
//                         }
                        break;
                    default:
                        found = (cur_ns.GetUri() == ns.GetUri());
                        break;
                    }

                    if (found)
                    {
                        result = &const_cast<SlotInfo&>(si).GetDataType(vm);
                        break;
                    }
                }
            }
        }

        if (!result)
            result = vm.Resolve2ClassTraits(name, ns);

        if (result)
            break;
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
PropRef FindObjProperty(VM& vm, const Value& value, const Multiname& mn, FindPropAttr attr)
{
    PropRef result;
    //const bool is_object = value.IsObject() && value.GetObject() != NULL;
    //bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsNamespace() || IsMethodClosure() || IsThunkClosure(); }

    // This is different from Value::IsObject().
    bool is_object = false;
    switch (value.GetKind())
    {
    case Value::kUndefined: // = 0
    case Value::kBoolean:   // = 1
    case Value::kInt:       // = 2
    case Value::kUInt:      // = 3
    case Value::kNumber:    // = 4
    case Value::kString:    // = 5
    case Value::kNamespace: // = 10
        break;
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
    case Value::kMethodClosure:  // = 14
        // null should be handled outside of this function. 
        SF_ASSERT(value.GetObject() != NULL);
        is_object = true;
        break;
    case Value::kThunk:     // = 11
    case Value::kThunkFunction: // = 12
    case Value::kMethodInd:  // = 13
        // kThunkClosure contains object ...
    case Value::kThunkClosure:  // = 15
        break;
    }

    const Traits& t = vm.GetValueTraits(value);
    const SlotInfo* si;

    // Search in fixed slots.
    UPInt index = 0;
    si = FindFixedSlot(vm, t, mn, index, (is_object ? value.GetObject() : NULL));

    if (si)
        return PropRef(value, si);

    if (!mn.ContainsNamespace(vm.GetPublicNamespace()))
        return result;

    // Search in dynamic slots.
    if (is_object && t.IsDynamic())
    {
        si = value.GetObject()->FindDynamicSlot(mn);
        if (si)
            return PropRef(value, si);
    }

    if (attr == FindGet)
    {
        // Search in a prototype chain.
        {
            const Traits* cur_tr = &t;
            while (cur_tr)
            {
                Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

                result = proto.FindProperty(mn, attr);
                if (result)
                    break;

                cur_tr = cur_tr->GetParent();
            }
        }

        // Search in a stored scope.
        if (!result && (value.IsFunction() || value.IsMethodClosure()))
        {
            const Instances::Function& f = value.IsFunction() ? value.AsFunction() : *value.GetFunct();

            result = FindScopeProperty(vm, 0, f.GetInitScope(), mn);
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
/// Very similar to FindProperty2.
PropRef FindPropertyWith(VM& vm, const Value& value, const Multiname& mn, FindPropAttr attr)
{
    PropRef result;
    //const bool is_object = value.IsObject() && value.GetObject() != NULL;
    //bool IsObject() const { return IsObjectStrict() || IsClass() || IsFunction() || IsThunkFunction() || IsNamespace() || IsMethodClosure() || IsThunkClosure(); }

    // This is different from Value::IsObject().
    bool is_object = false;
    switch (value.GetKind())
    {
    case Value::kUndefined: // = 0
    case Value::kBoolean:   // = 1
    case Value::kInt:       // = 2
    case Value::kUInt:      // = 3
    case Value::kNumber:    // = 4
    case Value::kString:    // = 5
    case Value::kNamespace: // = 10
        break;
    case Value::kFunction:  // = 6
    case Value::kObject:    // = 8
    case Value::kClass:     // = 9
    case Value::kMethodClosure:  // = 14
        // null should be handled outside of this function. 
        SF_ASSERT(value.GetObject() != NULL);
        is_object = true;
        break;
    case Value::kThunk:     // = 11
    case Value::kThunkFunction: // = 12
    case Value::kMethodInd:  // = 13
        // kThunkClosure contains object ...
    case Value::kThunkClosure:  // = 15
        break;
    }

    const Traits& t = vm.GetValueTraits(value);
    const SlotInfo* si;

    // Search in fixed slots.
    UPInt index = 0;
    si = FindFixedSlot(vm, t, mn, index, (is_object ? value.GetObject() : NULL));

    if (si)
        return PropRef(value, si);

    if (!(t.IsGlobal() || value.GetWith()))
        return result;

    // Search in dynamic slots.
    if (is_object && t.IsDynamic())
    {
        si = value.GetObject()->FindDynamicSlot(mn);
        if (si)
            return PropRef(value, si);
    }

    if (attr == FindGet)
    {
        // Search in a prototype chain.
        {
            const Traits* cur_tr = &t;
            while (cur_tr)
            {
                Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

                result = proto.FindProperty(mn, attr);
                if (result)
                    break;

                cur_tr = cur_tr->GetParent();
            }
        }

        // Search in a stored scope.
        if (!result && (value.IsFunction() || value.IsMethodClosure()))
        {
            const Instances::Function& f = value.IsFunction() ? value.AsFunction() : *value.GetFunct();

            result = FindScopeProperty(vm, 0, f.GetInitScope(), mn);
        }
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
CheckResult ExecuteProperty(VM& vm, const Multiname& prop_name, const Value& _this, Value& result, unsigned argc, const Value* argv)
{
    PropRef prop = AS3::FindObjProperty(vm, _this, prop_name);
    if (prop)
    {
        Value funct;

        if (prop.GetSlotValueUnsafe(vm, funct))
        {
            if (funct.IsNullOrUndefined())
                // A TypeError is thrown if obj is null or undefined.
                vm.ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Object is null or undefined"));
            else
            {
                vm.Execute(funct, _this, result, argc, argv);
                return !vm.IsException();
            }
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
Object::Object(Traits& t)
: GASRefCountBase(&t.GetVM().GetGC())
, pTraits(&t)
, FirstFreeInd(0)
{
    AdjustNumOfValues();
}

Object::Object(VM& vm)
: GASRefCountBase(&vm.GetGC())
, pTraits(NULL)
, FirstFreeInd(0)
{
}


Object::~Object()
{
    // Accounting.
    //--traits->ActiveObjNum;
    
    // Not always correct for the time being.
    //SF_ASSERT(counter == 0);
}

void Object::AdjustNumOfValues()
{
    // "slot_num" - number of predefined slots in an object ...
    const UPInt slot_num = GetTraits().GetFixedValueSlotNumber();

    if (slot_num)
    {
        if (FirstFreeInd == Values.GetSize())
            // Adjust FirstFreeInd
            FirstFreeInd += slot_num;

        Values.Resize(Values.GetSize() + slot_num);
    }
}

// Trait based version. It doesn't work for some reason.
//  void Object::ForEachChild_GC(GcOp op) const
//  {
//      traits->ForEachChild_GC(*this, op);
//      (*op)(const_cast<const AS3::GRefCountBaseGC**>(reinterpret_cast<AS3::GRefCountBaseGC**>(traits.GetRawPtr())));
//  }

// Regular version.
void Object::ForEachChild_GC(GcOp op) const
{
    AS3::ForEachChild_GC(Values, op);
    AS3::ForEachChild_GC<Traits, Mem_Stat>(pTraits, op);

//     if (pVTable)
//         pVTable->ForEachChild_GC(op);
}

void Object::ForEachChild_GC_NoValues(GcOp op) const
{
    AS3::ForEachChild_GC<Traits, Mem_Stat>(pTraits, op);

//     if (pVTable)
//         pVTable->ForEachChild_GC(op);
}

void Object::ForEachChild_GC_NoTraits(GcOp op) const
{
    AS3::ForEachChild_GC(Values, op);

//     if (pVTable)
//         pVTable->ForEachChild_GC(op);
}

void Object::ValueOf(Value& result)
{
	ExecuteProperty(GetStringManager().GetBuiltin(AS3Builtin_valueOf), result, 0, NULL);
}

CheckResult Object::SetSlotValue(SlotIndex ind, const Value& value)
{
    return GetTraits().SetSlotValue(AbsoluteIndex(ind, GetTraits()), value, this);
}

void Object::GetDefaultValue(Value& result, Value::Hint hint)
{
    StringManager &sm = GetStringManager();

	if (hint == Value::hintNone)
	{
		if (GetClass().GetName() == sm.GetBuiltin(AS3Builtin_Date))
			hint = Value::hintString;
		else
			hint = Value::hintNumber;
	}

	Value value;

	if (hint == Value::hintString)
	{
		// toString part.
		{
			if (!GetSlotValueUnsafe(sm.GetBuiltin(AS3Builtin_toString), GetVM().GetPublicNamespace(), value))
				return;

			if (value.IsCallable())
				ExecuteValue(value, result, 0, NULL);

			if (GetVM().IsException() || result.IsPrimitive())
				return;
		}

		// valueOf part.
		{
			if (!GetSlotValueUnsafe(sm.GetBuiltin(AS3Builtin_valueOf), GetVM().GetPublicNamespace(), value))
				return;

			if (value.IsCallable())
				ExecuteValue(value, result, 0, NULL);

			if (GetVM().IsException() || result.IsPrimitive())
				return;
		}
	}
	else
	{
		// valueOf part.
		{
			if (!GetSlotValueUnsafe(sm.GetBuiltin(AS3Builtin_valueOf), GetVM().GetPublicNamespace(), value))
				return;

			if (value.IsCallable())
				ExecuteValue(value, result, 0, NULL);

			if (GetVM().IsException() || result.IsPrimitive())
				return;
		}

		// toString part.
		{
			if (!GetSlotValueUnsafe(sm.GetBuiltin(AS3Builtin_toString), GetVM().GetPublicNamespace(), value))
				return;

			if (value.IsCallable())
				ExecuteValue(value, result, 0, NULL);

			if (GetVM().IsException() || result.IsPrimitive())
				return;
		}
	}

	return GetVM().ThrowTypeError(VM::eNotImplementedYet);
}

void Object::SetValue(AbsoluteIndex ind, const Value& value)
{
    SF_ASSERT(ind.IsValid());
    
    if (ind.Get() >= Values.GetSize())
    {
        if (FirstFreeInd == Values.GetSize())
            // Adjust FirstFreeInd.
            FirstFreeInd = ind.Get() + 1;

        Values.Resize(ind.Get() + 1);
    }

	Set(Values[ind.Get()], value);
}
    
PropRef Object::FindProperty(const Multiname& mn, FindPropAttr attr)
{
    const SlotInfo* si = NULL;

    UPInt index = 0;
    si = AS3::FindFixedSlot(GetVM(), GetTraits(), mn, index, this);
    if (si)
        return PropRef(this, si);

    // Check dynamic slots.
    if (IsDynamic())
    {
        si = FindDynamicSlot(mn);
        if (si)
            return PropRef(this, si);
    }

    PropRef result;
    if (attr == FindGet)
    {
        // Search in a prototype chain.
        const Traits* cur_tr = &GetTraits();
        while (cur_tr)
        {
            Object& proto = const_cast<Class&>(cur_tr->GetClass()).GetPrototype();

            if (&proto == this)
                break;

            result = proto.FindProperty(mn, attr);

            if (result)
                break;

            cur_tr = cur_tr->GetParent();
        }
    }

    return result;
}

void Object::Set(SPtr<Object>& o, const Value& v)
{
    SF_ASSERT(v.IsObject());
    o = v.GetObject();
}

void Object::Set(Value& value, const Value& v)
{
    value = v;
}

void Object::ExecuteProperty(const ASString& name, Value& result, unsigned argc, const Value* argv)
{
    const Multiname mn(GetVM().GetPublicNamespace(), name);
    PropRef prop = AS3::FindObjProperty(GetVM(), Value(this), mn);
    
    if (prop)
    {
        Value _this;
        
        if (!prop.GetSlotValueUnsafe(GetVM(), _this))
            return;
        
        if (_this.IsNullOrUndefined())
        {
            // A TypeError is thrown if obj is null or undefined.
            GetVM().ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Object is null or undefined"));
            return;
        }

		ExecuteValue(_this, result, argc, argv);

        // !!! Code below is not correct.
        //GetVM().Execute(prop.GetValue(), prop.GetObject(), result, argc, argv);
    } else
    {
        // Assert for the time being.
        SF_ASSERT(false);
        GetVM().ThrowReferenceError(VM::eNotImplementedYet);
    }
}

void Object::AddDynamicSlotValuePair(const ASString& prop_name, const Value& v, SlotInfo::Attribute a)
{
	// We do not need to check for the type compatibility and do type 
	// conversion for dynamic attributes because they can be of any type.

    // It is not correct to check FixedValueSlotNumber here because we deal with
    // Dynamic Slots here.
    // SF_ASSERT(Values.GetSize() == GetTraits().GetFixedValueSlotNumber());

    const AbsoluteIndex value_ind = SetValue(v);
        
    // Add traits info.
    // Dynamic properties are always public.
	// And they can be of any type.
    // !!! It is not correct to call FindAdd() here. (NEED EXPLANATION)
    AbsoluteIndex slot_ind = DynSlots.Add(
        prop_name, 
        SlotInfo(
            GetVM().MakePublicNamespace(), // Dynamic properties are always public.
            NULL,
            a
            )
        );
    
    // Bind a trait to a value ...
    DynSlots[slot_ind].Bind(SlotInfo::DT_ValueArray, value_ind);
}

void Object::AddDynamicSlotValuePair(const Multiname& mn, const Value& v, SlotInfo::Attribute a)
{
    // Name can be passed as any data type.
    // Let's convert it to a string;
    const Value& vn = mn.GetName();
    StringManager& sm = GetStringManager();
    ASString name = sm.CreateEmptyString();
    if (!vn.Convert2String(name, sm))
        // Exception.
        return;

    AddDynamicSlotValuePair(name, v, a);
}

CheckResult Object::DeleteDynamicSlotValuePair(const Multiname& mn)
{
    // Convert to string.
    ASString str_name(GetStringManager().CreateEmptyString());
    if (!mn.GetName().Convert2String(str_name, GetStringManager()))
        // Exception.
        return false;

    // Remove values
    {
        const SlotContainerType::SetType::ValueType* indSet = DynSlots.FindValues(str_name);

        if (indSet)
        {
            for (UPInt i = 0; i < indSet->GetSize(); ++i)
            {
                const SlotInfo& si = DynSlots[AbsoluteIndex(indSet->At(i))];
                si.DeleteDynamic(*this);
            }
        }
    }

    // Remove key
    //const int ind = DynSlots.Find(mn.GetName().AsString());
    const UPInt n = DynSlots.RemoveKey(str_name);

    return n > 0;
}

void Object::DeleteDynamicSlotValue(AbsoluteIndex ind)
{
    SF_ASSERT(ind.IsValid());
    Values[ind.Get()].SetUndefined();

    if (ind.Get() < FirstFreeInd)
        FirstFreeInd = ind.Get();
}

void Object::Call( const Value& _this, Value& result, unsigned argc, const Value* const argv )
{
    // Do nothing by default;
    SF_UNUSED4(result, _this, argc, argv);
}

void Object::Super(unsigned /*argc*/, const Value* /*argv*/)
{
    // Do nothing by default;
}

void Object::constructor(Value& result)
{
    result = &GetClass();
}

inline
bool IsDeletedKey(ASStringNode* key)
{
    return key == NULL;
}

GlobalSlotIndex Object::GetNextPropertyIndex(GlobalSlotIndex ind) const
{
    UPInt n = GetTraits().GetSlots().GetSize();
    GlobalSlotIndex result(0);

    if (ind.Get() < n)
    {
        // Static slots.
        result = GetTraits().GetNextPropertyIndex(ind);

        if (result.IsValid())
            return result;
        else
            // Adjust index to work with dynamic slots.
            ind = GlobalSlotIndex(n);
    }

    // Dynamic slots.
    while (ind.Get() - n < DynSlots.GetSize())
    {
        ++ind;

        // Check for SlotInfo::aDontEnum and deleted slots.
        AbsoluteIndex aind(ind);
        if (IsDeletedKey(DynSlots.GetKey(aind - n)) || DynSlots[aind - n].IsDontEnum())
            continue;

        return ind;
    }

    return GlobalSlotIndex(0);
}

Value Object::GetNextPropertyName(GlobalSlotIndex ind) const
{
    if (ind.IsValid())
    {
        UPInt static_slot_num = GetTraits().GetSlotInfoNum();

        if (ind > static_slot_num)
            return DynSlots.GetKey(AbsoluteIndex(ind - static_slot_num));
        else
            return GetTraits().GetName(AbsoluteIndex(ind));
    }

    SF_ASSERT(false);
    return Value::GetUndefined();
}

void Object::GetNextPropertyValue(Value& value, GlobalSlotIndex ind)
{
    if (ind.IsValid())
    {
        UPInt static_slot_num = GetTraits().GetSlotInfoNum();

        if (ind > static_slot_num)
            // !!! Should we check result of GetSlotValue() here?
            DynSlots[AbsoluteIndex(ind - static_slot_num)].GetSlotValue(value, this).DoNotCheck();
        else
            GetTraits().GetSlotValue(value, AbsoluteIndex(ind), this);
    }
    else
        SF_ASSERT(false);
}

AbsoluteIndex Object::SetValue(const Value& v)
{
    UPInt value_ind;
    if (FirstFreeInd == Values.GetSize())
    {
        // No free values.

        // Add value.
        value_ind = Values.GetSize();
        Values.PushBack(v);
        FirstFreeInd = Values.GetSize();
    } else
    {
        value_ind = FirstFreeInd;
        Values[value_ind] = v;

        // Find next FirstFreeInd.
        // There is no way to find next free value for the time being.
        FirstFreeInd = Values.GetSize();
    }

    return AbsoluteIndex(value_ind);
}

const SlotInfo& Object::AddFixedSlotValuePair(const ASString& prop_name, Pickable<const Instances::Namespace> ns,
    const ClassTraits::Traits* tr, const Value& v, UPInt& index)
{
    Values.PushBack(v);
    FirstFreeInd = Values.GetSize();

    // Add to Traits and bind ...
    GetTraits().AddSlotValue(prop_name, ns, tr);
    // No dynamic slots should be added at this point.
    SF_ASSERT(Values.GetSize() == GetTraits().GetFixedValueSlotNumber());

    index = GetTraits().GetSlots().GetSize() - 1;
    return GetTraits().GetTopSlotInfo();

}
  
const SlotInfo* Object::InitializeOnDemand(
    const SlotInfo* si,
    const ASString&,
    const Instances::Namespace&,
    UPInt&)
{
    return si;
}

///////////////////////////////////////////////////////////////////////////
CheckResult ToString(VM& vm, const Value& _this, ASString& result)
{
    if (_this.IsNullOrUndefined())
    {
        if (!_this.Convert2String(result, vm.GetStringManager()))
            return false;
    }
    else
    {
        Value r;
        Multiname name(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("toString"));

        if (!ExecuteProperty(vm, name, _this, r, 0, NULL))
            return false;

        if (!r.IsString() && !r.Convert2String(result, vm.GetStringManager()))
            // exception
            return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
CheckResult GetSuperProperty(VM& vm, const Traits* ot, Value& result, const Value& _this, const Multiname& mn)
{
    const Traits& tr = (ot ? *ot : vm.GetValueTraits(_this));

    if (tr.GetParent())
    {
        // Search object using base class traits.
        UPInt index = 0;
        const Traits& parent_tr = *tr.GetParent();

        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_tr,
            mn,
            index,
            NULL
            );

        // Retrieve value using base class Virtual Table.
        return si && si->GetSlotValue(vm, result, _this, &parent_tr.GetVT());

    }

    /* Old version/ Let's keep it for historic reason.
    const Class& obj_class = tr.GetClass();
    
    if (obj_class.GetParentClass())
    {
        Class& parent_class = *obj_class.GetParentClass();
        const InstanceTraits::Traits& parent_traits = const_cast<const InstanceTraits::Traits&>(parent_class.GetNewObjectTraits());

        // Search object using base classes instance traits.
        UPInt index = 0;
        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_traits,
            mn,
            index,
            &parent_class
            );

        return si && si->GetSlotValue(vm, result, _this, &parent_traits.GetVT());
    }
    */
    
    return false;
}

CheckResult SetSuperProperty(VM& vm, const Traits* ot, const Value& _this, const Multiname& mn, const Value& value)
{
    const Traits& tr = (ot ? *ot : vm.GetValueTraits(_this));

    if (tr.GetParent())
    {
        // Search object using base class traits.
        UPInt index = 0;
        const Traits& parent_tr = *tr.GetParent();

        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_tr, 
            mn,
            index,
            NULL
            );

        // Set value using base class Virtual Table.
        return si && const_cast<SlotInfo*>(si)->SetSlotValue(vm, value, _this, &parent_tr.GetVT());
    }

    /* Old version/ Let's keep it for historic reason.
    const Class& obj_class = tr.GetClass();
    
    if (obj_class.GetParentClass())
    {
        Class& parent_class = *obj_class.GetParentClass();
        const InstanceTraits::Traits& parent_traits = const_cast<const InstanceTraits::Traits&>(parent_class.GetNewObjectTraits());

        // Search object using base classes instance traits.
        UPInt index = 0;
        const SlotInfo* si = FindFixedSlot(
            vm, 
            parent_traits, 
            mn,
            index,
            &parent_class
            );

        return si && const_cast<SlotInfo*>(si)->SetSlotValue(vm, value, _this, &parent_traits.GetVT());
    }
    */
    
    return false;
}

///////////////////////////////////////////////////////////////////////////
Class::Class(const ASString& n, ClassTraits::Traits& t, Class* parent)
: Object(t)
, ParentClass(parent)
, Name(n)
, Length(1)
{
}

Class::~Class()
{
    // All prototypes should be initialized. At least in the end.
    // Initialize prototypes on demand.
    //SF_ASSERT(pPrototype.Get());
}

#ifdef CONSTRUCT_IN_TRAITS
void Class::Construct(Value& result, unsigned argc, const Value* argv, bool extCall)
{
    GetInstanceTraits().Construct(result, argc, argv, extCall);
}
#else
void Class::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
{
    GetNewObjectTraits().MakeObject(_this);
    PreInit(_this);
    _this.GetObject()->InitInstance(extCall);
    PostInit(_this, argc, argv);
}

void Class::PreInit(const Value& /*_this*/) const
{
    // Do nothing by default.
}

void Class::PostInit(const Value& _this, unsigned argc, const Value* argv) const
{
    _this.GetObject()->Super(argc, argv);
}
#endif


ASString Class::GetName() const
{
    return Name;
}

// Regular version.
void Class::ForEachChild_GC(GcOp op) const
{
    Object::ForEachChild_GC(op);

    // Instance traits are handled  in parent classes.
    
    AS3::ForEachChild_GC<Class, Mem_Stat>(ParentClass, op);
    AS3::ForEachChild_GC<Object, Mem_Stat>(pPrototype, op);
}

void Class::InitPrototypeFromVTable(AS3::Object& obj, ConvertFunc f) const
{
    const InstanceTraits::Traits& t = GetClassTraits().GetInstanceTraits();
    const VTable& vt = t.GetVT();
    const SlotContainerType& sc = t.GetSlots();

    for (UPInt i = t.GetFirstOwnSlotIndex().Get(); i < sc.GetSize(); ++i)
    {
        const AbsoluteIndex abs_i(i);
        const SlotInfo& si = sc[abs_i];

        if (si.GetBindingType() == SlotInfo::DT_Code)
        {
            ASStringNode* sn = sc.GetKey(abs_i);

            obj.AddDynamicSlotValuePair(
                ASString(sn), 
                (this->*f)(vt.Get(si.GetValueInd())), 
                SlotInfo::aDontEnum
                );
        }
    }
}

void Class::InitPrototypeFromVTableCheckType(AS3::Object& obj) const
{
    InitPrototypeFromVTable(obj, &Class::ConvertCheckType);
}

void Class::InitPrototype(AS3::Object& obj) const
{
    if (GetParentClass())
        GetParentClass()->InitPrototype(obj);

    InitPrototypeFromVTableCheckType(obj);
    AddConstructor(obj);
}

void Class::AddConstructor(AS3::Object& obj) const
{
    // Add "constructor" to the prototype object.
    obj.AddDynamicSlotValuePair(
        GetVM().GetStringManager().CreateConstString("constructor"),
        Value(const_cast<Class*>(this)),
        SlotInfo::aDontEnum
        );
}

Pickable<Object> Class::MakePrototype() const
{
    return GetVM().MakeObject();
}

Value Class::ConvertCopy(const Value& v) const
{
    // Just copy the value;
    return v;
}

Value Class::ConvertCheckType(const Value& v) const
{
    return GetVM().GetClassFunction().MakeCheckTypeInstance(*this, v.AsThunk());
}

void Class::lengthGet(SInt32& result)
{
    result = Length;
}

Object& Class::GetPrototype()
{
    if (pPrototype.Get() == NULL)
    {
        pPrototype = MakePrototype();
        InitPrototype(*pPrototype);
    }

    return *pPrototype;
}

void Class::prototypeGet(Value& result)
{
    result = &GetPrototype();
}

void Class::toString(ASString& result)
{
    result = GetASString("[class " + GetName() + "]");
}

Class& Class::ApplyTypeArgs(unsigned argc, const Value* argv)
{
    SF_UNUSED2(argc, argv);
    return *this;
}

void Class::Call(const Value& /*_this*/, Value& result, unsigned argc, const Value* const argv)
{
    // explicit coercion of a class object.
    if (argc == 1)
    {
        const Value& v = argv[0];
        //const Class& cl = GetValueTraits().GetVM().GetClass(v);
        //const Abc::ClassInfo& ci = GetInstanceTraits().GetClassInfo();

        if (GetClassTraits().Coerce(v, result))
            return;
        else
            return GetVM().ThrowTypeError(VM::eNotImplementedYet DEBUG_ARG("Cannot coerce value."));
    }

    return GetVM().ThrowRangeError(VM::eCoerceArgumentCountError DEBUG_ARG("One argument is required."));
}

///////////////////////////////////////////////////////////////////////////////
NamespaceSet::~NamespaceSet()
{
}

void NamespaceSet::Add(Instances::Namespace& ns)
{
    Namespaces.PushBack(Value(&ns));
}

///////////////////////////////////////////////////////////////////////////
Value::Value(Object* v)
: Flags(kObject)
, value(v)
{
    if (v)
    {
        v->AddRef();
    }
}

Value::Value(Class* v)
: Flags(kClass)
, value(v)
{
    if (v)
        v->AddRef();
}

void  Value::Assign(Object* v)
{
    Release();
    SetKind(kObject);
    value = v;
    if (value.VS._1.VObj)
    {
      //  SF_ASSERT(value.VS._1.VObj->GetTraitsType() != Traits_Class);
        value.VS._1.VObj->AddRef();
    }
}

void  Value::AssignUnsafe(Object* v)
{
    SetKind(kObject);
    value = v;
    if (value.VS._1.VObj)
    {
        //  SF_ASSERT(value.VS._1.VObj->GetTraitsType() != Traits_Class);
        value.VS._1.VObj->AddRef();
    }
}

void Value::Assign(Class* v)
{
    Release();
    SetKind(kClass);
    value = v;
    if (value.VS._1.VClass)
        value.VS._1.VClass->AddRef();
}

void Value::AssignUnsafe(Class* v)
{
    SetKind(kClass);
    value = v;
    if (value.VS._1.VClass)
        value.VS._1.VClass->AddRef();
}

void  Value::Pick(Object* v)
{
    Release();
    SetKind(kObject);
    value = v;
    // No AddRef() is necessary.
}

void  Value::PickUnsafe(Object* v)
{
    SetKind(kObject);
    value = v;
    // No AddRef() is necessary.
}

void Value::Pick(Class* v)
{
    Release();
    SetKind(kClass);
    value = v;
    // No AddRef() is necessary.
}

void Value::PickUnsafe(Class* v)
{
    SetKind(kClass);
    value = v;
    // No AddRef() is necessary.
}

///////////////////////////////////////////////////////////////////////////
namespace Classes
{
    //const char* Info<Class>::GetName() { return "Class"; }
    //const char* Info<Class>::GetPkgName() { return ""; }
    template<>
    struct Info<Class>
    {
        const char* GetName() { return "Class"; }
        const char* GetPkgName() { return ""; }
    };

    const char* Info<None>::GetName() { return ""; }
    const char* Info<None>::GetPkgName() { return ""; }
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

