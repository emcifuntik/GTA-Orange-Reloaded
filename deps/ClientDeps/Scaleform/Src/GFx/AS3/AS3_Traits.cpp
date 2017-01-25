/**********************************************************************

Filename    :   AS3_Traits.cpp
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

#include "AS3_Traits.h"
#include "AS3_VTable.h"
#include "AS3_VM.h"
#include "Obj/AS3_Obj_Namespace.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
void Slots::SetSlotInfo(AbsoluteIndex ind, const ASString& name, const SlotInfo& v)
{
    SF_ASSERT(ind.IsValid());
    
    SlotContainer[ind] = v;
    SlotContainer.SetKey(ind, name);
}

AbsoluteIndex Slots::FindSlotInfoIndex(const ASString& name, const Instances::Namespace& ns) const
{
    // Namespace must match for find to succeed, since an object
    // can have identically named slots from different namespaces.
    const SlotContainerType::SetType::ValueType *vt = SlotContainer.FindValues(name);
    if (vt)
    {
        for (UPInt i = 0; i < vt->GetSize(); i++)
        {
            const AbsoluteIndex index((*vt)[i]);
            const SlotInfo& si = SlotContainer[index];
            const Instances::Namespace& cur_ns = si.GetNamespace();

            if (cur_ns.GetKind() == ns.GetKind())
            {
                switch (ns.GetKind())
                {
                case Abc::NS_Protected:
                    return index;
                case Abc::NS_Private:
                    if (&ns == &cur_ns)
                        return index;
                    break;
                default:
                    if (cur_ns.GetUri() == ns.GetUri())
                        return index;
                    break;
                }
            }

        }
    }

    return AbsoluteIndex(-1);
}

SlotInfo& Slots::FindAddSlotInfo(const ASString& name, const SlotInfo& v)
{    
    SlotInfo *slotInfo = FindSlotInfo(name, v.GetNamespace());
    if (slotInfo)
        return *slotInfo;
    AbsoluteIndex ind = SlotContainer.Add(name, v);
    return SlotContainer[ind];
}

///////////////////////////////////////////////////////////////////////////
Traits::Traits(VM& _vm, Class& c, const Traits* pt, ObjectType ot)
: GASRefCountBase(&_vm.GetGC())
, ArrayLike(pt ? pt->IsArrayLike() : false) // Inherit *ArrayLike* from the parent.
, IsInterface_(false)
, FirstOwnSlotInd(0)
, OwnSlotNumber(0)
, FixedValueSlotNumber(0)
, pVM(&_vm)
, OT(ot)
, TraitsType(pt ? pt->GetTraitsType() : Traits_Unknown)
, pConstructor(&c)
, pParent(pt)
{
    // Initialize vtable ...
    if (pt)
        // pVTable is AutoPtr.
        pVTable = SF_HEAP_NEW(_vm.GetMemoryHeap()) VTable(pt->GetVT());
    else
        // pVTable is AutoPtr.
        pVTable = SF_HEAP_NEW(_vm.GetMemoryHeap()) VTable(_vm);
    
    FirstOwnSlotInd += OwnSlotNumber;
}

Traits::Traits(VM& _vm)
: GASRefCountBase(&_vm.GetGC())
, ArrayLike(false)
, IsInterface_(false)
, FirstOwnSlotInd(0)
, OwnSlotNumber(0)
, FixedValueSlotNumber(0)
, pVM(&_vm)
, OT(SealedObject)
, TraitsType(Traits_Unknown)
, pConstructor(NULL)
, pParent(NULL)
// pVTable is AutoPtr.
, pVTable(SF_HEAP_NEW(_vm.GetMemoryHeap()) VTable(_vm))
{
    FirstOwnSlotInd += OwnSlotNumber;
}

Traits::~Traits()
{
}

bool Traits::IsGlobal() const
{
    return false;
}

#ifdef CONSTRUCT_IN_TRAITS
void Traits::Construct(Value& _this, unsigned argc, const Value* argv, bool extCall)
{
    MakeObject(_this);
    PreInit(_this);
    _this.GetObject()->InitInstance(extCall);
    PostInit(_this, argc, argv);
}

void Traits::PreInit(const Value& /*_this*/) const
{
    // Do nothing by default.
}

void Traits::PostInit(const Value& _this, unsigned argc, const Value* argv) const
{
    _this.GetObject()->Super(argc, argv);
}
#endif

void Traits::AddSlotsByName(const Abc::HasTraits& t, VMAbcFile& file)
{
    for(AbsoluteIndex i(0); i.Get() < t.GetTraitsCount(); ++i)
    {
        const Abc::TraitInfo& trait_info = t.GetTraitInfo(file.GetTraits(), i);
        const Abc::Multiname& mn = trait_info.GetName(file.GetConstPool());
        
        SF_ASSERT(mn.IsQName());
        
        // What is actually a data type ???
        SlotInfo::DataType binding_type = SlotInfo::DT_ValueArray;
        bool _const = false;
        
        switch(trait_info.GetType())
        {
        case Abc::TraitInfo::tConst:
            _const = true;
        case Abc::TraitInfo::tSlot:
            break;
        case Abc::TraitInfo::tMethod:
            binding_type = SlotInfo::DT_Code;
            break;
        case Abc::TraitInfo::tGetter:
            binding_type = SlotInfo::DT_Get;
            break;
        case Abc::TraitInfo::tSetter:
            binding_type = SlotInfo::DT_Set;
            break;
        case Abc::TraitInfo::tClass:
            break;
        case Abc::TraitInfo::tFunction:
            binding_type = SlotInfo::DT_Code;
            break;
        default:
            SF_ASSERT(false);
            break;
        }

        const ASString mn_name = file.GetInternedString(mn.GetNameInd());

        /* DO NOT delete this code.
        // Try to resolve type in place.
        // This doesn't work because of dependencies.
        const ClassTraits::Traits* tr = NULL;

        if (trait_info.IsData())
        {
            const Abc::Multiname& tmn = trait_info.GetTypeName(file.GetAbcFile());
            tr = GetVM().Resolve2ClassTraits(file, tmn);
            Multiname debug_mn(file, tmn);
            SF_ASSERT(tr);
        }
        */

        // Add a new/replace slot ...
        // We do not use ClassTraits::Traits here because we want to postpone name resolution.
        // Otherwise we will get problems with dependencies.
        SlotInfo& si = FindAddSlotInfo(
            mn_name,
            SlotInfo(
                Pickable<Instances::Namespace>(&file.GetInternedNamespace(mn.GetNamespaceInd()), PickValue),
                file,
                trait_info,
                (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
                DEBUG_ARG((GetQualifiedName() + "/" + mn_name).GetNode())
                )
            );
            
        // Bind.
        if (trait_info.HasMethodInd())
            // Initialize a virtual table ...
            // Add2VT will also bind the slot.
            Add2VT(GetVT(), si, Value(trait_info.GetMethodInd(), file), binding_type);
        else
            si.Bind(binding_type, AbsoluteIndex(FixedValueSlotNumber++));
    }
}

void Traits::SetupSlotValues(VMAbcFile& file, const Abc::HasTraits& t, Object& for_obj) const
{
    using namespace Abc;

    InitializerGuard __(GetVM());

    // *t* is relative because it is specific to the object and does not include inheritance ...
    //for(RelativeIndex rel_ind(0); rel_ind.Get() < t.GetTraitsCount(); ++rel_ind)
    // *i* is absolute for Abc::HasTraits, but it is relative in current traits.
    for(AbsoluteIndex i(0); i.Get() < t.GetTraitsCount(); ++i)
    {
        const TraitInfo& trait_info = t.GetTraitInfo(file.GetTraits(), i);
        
        switch(trait_info.GetType())
        {
        case TraitInfo::tSlot:
        case TraitInfo::tConst:
        {
            Abc::Multiname mn = trait_info.GetName(file.GetConstPool());
            SlotInfo* si = const_cast<SlotInfo*>(FindSlotInfo(file, mn));

            if (si)
            {
                // If we have a value ...
                if (trait_info.HasDefaultValue())
                {
                    si->SetSlotValue(GetVM(), GetDetailValue(file, trait_info.GetDefaultValue()), &for_obj).DoNotCheck();
                }
                else
                {
                    Abc::Multiname mn = trait_info.GetTypeName(file.GetAbcFile());

                    si->SetSlotValue(GetVM(), GetVM().GetDefaultValue(file, mn), &for_obj).DoNotCheck();
                }
            }
            else
                SF_ASSERT(false);
        }
            break;
        case TraitInfo::tMethod:
        case TraitInfo::tGetter:
        case TraitInfo::tSetter:
        {
            // !!!
            // When creating the new function object the scope stack used is the current scope stack when
            // this instruction is executed, and the body is the method_body entry that references the
            // specified method_info entry.
//              const int method_ind = trait_info.AsMethod().GetMethodInd();
//              const Value value = GetVM().ClassMethodClosure->MakeInstance(method_ind, for_obj);
//
//              AddVT(GetVT(), si, value);
        }
            break;
        case TraitInfo::tFunction:
        {
            // !!!
            // When creating the new function object the scope stack used is the current scope stack when
            // this instruction is executed, and the body is the method_body entry that references the
            // specified method_info entry.
//              const int method_ind = trait_info.AsFunction().GetMethodInd();
//              const Value value = GetVM().ClassFunction->MakeInstance(method_ind, for_obj);
//              
//              AddVT(GetVT(), si, value);
        }
            break;
        case TraitInfo::tClass:
            //SF_ASSERT(false);
            // It looks like nothing to initialize.
            break;
        default:
            SF_ASSERT(false);
            break;
        }
    }
}

void Traits::AddInterfaceSlots(VMAbcFile& file, const Abc::ClassInfo& class_info)
{
    const Abc::Instance& instance = class_info.GetInstanceInfo();
    const Abc::Instance::Interfaces& interfaces = instance.GetInterfaces();
    VM& vm = GetVM();

    for (UPInt i = 0; i < interfaces.GetSize(); ++i)
    {
        const Multiname interfaceMN(file, interfaces.GetName(file.GetConstPool(), AbsoluteIndex(i)));

        // DO NOT RegisterImplementedInterface() on purpose.
        // We will save some memory by using an overloaded method SupportsInterface()
        //RegisterImplementedInterface(interfaceMN);

        const ClassTraits::Traits* ctraits = vm.GetRegisteredClassTraits(interfaceMN);
        if (!ctraits)
            ctraits = AS3::FindScopeTraits(GetVM(), 0, GetVM().GetGlobalObjects(), interfaceMN);

        if (!ctraits)
            return GetVM().ThrowVerifyError(VM::eNotImplementedError);

        if (!ctraits->IsValid())
            return GetVM().ThrowVerifyError(VM::eNotImplementedError);

        const InstanceTraits::Traits& itraits = ctraits->GetInstanceTraits();
        const SlotContainerType&      islotContainer = itraits.GetSlots();

        for (UPInt j = 0; j < islotContainer.GetSize(); ++j)
        {
            AbsoluteIndex   interfaceSlotIndex(j);
            ASString        interfaceSlotName(islotContainer.GetKey(interfaceSlotIndex));
            const SlotInfo& interfaceSlot = islotContainer[interfaceSlotIndex];

            // For each interface slot, we see if has already been added. If it is, as is the
            // case for 'constructor' and perhaps multiple interface use, no change is necessary.
            // If not, we find the method declared in 'public' namespace of the class and
            // create an alias to it for interface namespace.

            SlotInfo* thisSlot = FindSlotInfo(interfaceSlotName, interfaceSlot.GetNamespace());
            if (!thisSlot)
            {
                thisSlot = FindSlotInfo(interfaceSlotName, vm.GetPublicNamespace());
                if (thisSlot)
                {
                    // Interface slot was added/overridden in public namespace.
                    // Let us add an alias.
                    // What should we do about protected and private namespaces.

                    // Just thisSlot->setNamespace() below won't work.
                    //thisSlot->setNamespace(interfaceSlot.GetNamespace());

                    SlotInfo newSlot(*thisSlot);
                    newSlot.setNamespace(interfaceSlot.GetNamespace());
                    AddSlotInfo(interfaceSlotName, newSlot);
                }
                else
                {
                    if (instance.IsInterface())
                    {
                        // Add this slot/code.
                        SlotInfo::DataType binding_type = interfaceSlot.GetBindingType();
                        SlotInfo& si = FindAddSlotInfo(
                            interfaceSlotName, 
                            interfaceSlot
                            );

                        // Initialize a virtual table ...
                        // Add2VT will also bind the slot.
                        // Add2VT(GetVT(), si, vt_value, binding_type);

                        // Code in this block is a part of the Add2VT() method.
                        {
                            switch (binding_type)
                            {
                            case SlotInfo::DT_Code:
                            case SlotInfo::DT_Get:
                                {
                                    const Value& vt_value = itraits.GetVT().Get(interfaceSlot.GetValueInd());

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type, *this DEBUG_ARG(si.GetName())));
                                }
                                break;
                            case SlotInfo::DT_Set:
                                {
                                    const Value& vt_value = itraits.GetVT().Get(interfaceSlot.GetValueInd() + 1);

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type, *this DEBUG_ARG(si.GetName())));
                                }
                                break;
                            case SlotInfo::DT_GetSet:
                                // Add DT_Get part.
                                {
                                    binding_type = SlotInfo::DT_Get;
                                    const Value& vt_value = itraits.GetVT().Get(interfaceSlot.GetValueInd());

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type, *this DEBUG_ARG(si.GetName())));
                                }
                                // Add DT_Set part.
                                {
                                    binding_type = SlotInfo::DT_Set;
                                    const Value& vt_value = itraits.GetVT().Get(interfaceSlot.GetValueInd() + 1);

                                    // Add a new record to the virtual table ...
                                    si.Bind(binding_type, GetVT().AddMethod(vt_value, binding_type, *this DEBUG_ARG(si.GetName())));
                                }
                                break;
                            default:
                                SF_ASSERT(false);
                            };

                            ++OwnSlotNumber;
                        }
                    }
                    else
                    {
                        // Class instance doesn't implement all traits of the interface!
                        // TBD: message with class, interface, and method name.
                        return GetVM().ThrowVerifyError(VM::eNotImplementedError);
                    }
                }
            }
            /* For debugging.
            else
                SF_ASSERT(false);
            */
        }

        // Do the same thing recursively for parent interfaces, if we have any.
        // The loop enough is not enough to propagate all the traits through interface
        // chain, since it only aliases slots that are overridden in the derived interface.
        if (itraits.GetParent())
        {
            const Abc::ClassInfo* baseiClassInfo = itraits.GetParent()->GetClassInfo();

            if (baseiClassInfo)        
                AddInterfaceSlots(file, *baseiClassInfo);
        }
    }
}

void Traits::ForEachChild_GC(GcOp op) const
{
    AS3::ForEachChild_GC<Class, Mem_Stat>(pConstructor, op);
    AS3::ForEachChild_GC_Const<const Traits, Mem_Stat>(pParent, op);
    
    pVTable->ForEachChild_GC(op);

    AS3::ForEachChild_GC<Instances::Namespace, Mem_Stat>(ProtectedNamespace, op);
    AS3::ForEachChild_GC<Instances::Namespace, Mem_Stat>(PrivateNamespace, op);
    AS3::ForEachChild_GC(GetSlots(), op);
}

void Traits::ForEachChild_GC(const Object& obj, GcOp op) const
{
    AbsoluteIndex count(GetSlotInfoNum());
    
    for(AbsoluteIndex i(0); i < count; ++i)
        GetSlotInfo(i).ForEachChild_GC(obj, op);
}

void Traits::AddSlotsByPos(const Abc::HasTraits& traits, VMAbcFile& file)
{
    // Slots, which have an ID.
    for(AbsoluteIndex i(0); i.Get() < traits.GetTraitsCount(); ++i)
    {
        const Abc::TraitInfo& trait_info = traits.GetTraitInfo(file.GetTraits(), i);

        // Filter out slots with no ID.
        if (trait_info.HasSlotID())
        {
            const Abc::Multiname& mn = trait_info.GetName(file.GetConstPool());
            // slot_id == 0 means "auto-assign slot" ...
            const SlotIndex slot_id(trait_info.GetSlotID());
            
            // Filter out slots with ID set to 0.
            if (!slot_id.IsValid())
                continue;

            SlotInfo::DataType binding_type = SlotInfo::DT_ValueArray;
            const AbsoluteIndex ind(trait_info.HasMethodInd() ? -1 : static_cast<SPInt>(FixedValueSlotNumber++));
            bool _const = false;

            // Get binding_type.
            switch(trait_info.GetType())
            {
            case Abc::TraitInfo::tConst:
                _const = true;
            case Abc::TraitInfo::tSlot:
                break;
            case Abc::TraitInfo::tMethod:
                binding_type = SlotInfo::DT_Code;
                break;
            case Abc::TraitInfo::tGetter:
                binding_type = SlotInfo::DT_Get;
                break;
            case Abc::TraitInfo::tSetter:
                binding_type = SlotInfo::DT_Set;
                break;
            case Abc::TraitInfo::tClass:
                break;
            case Abc::TraitInfo::tFunction:
                binding_type = SlotInfo::DT_Code;
                break;
            default:
                SF_ASSERT(false);
                break;
            }

            /* DO NOT delete this code.
            // Try to resolve type in place.
            // This doesn't work because of dependencies.
            const ClassTraits::Traits* tr = NULL;
            if (trait_info.IsData())
            {
                const Abc::Multiname& tmn = trait_info.GetTypeName(file.GetAbcFile());
                tr = GetVM().Resolve2ClassTraits(file, tmn);
                SF_ASSERT(tr);
            }
            */

            // We do not use ClassTraits::Traits here because we want to postpone name resolution.
            // Otherwise we will get problems with dependencies.
            AddSlot(
                RelativeIndex(slot_id), 
                file.GetInternedString(mn.GetNameInd()),
                Pickable<Instances::Namespace>(&file.GetInternedNamespace(mn), PickValue),
                file,
                trait_info,
                binding_type, 
                ind,
                _const
                );

            if (trait_info.HasMethodInd())
            {
                // Initialize a virtual table ...
                // Add2VT will also bind the slot.
                Add2VT(GetVT(), GetTopSlotInfo(), Value(trait_info.GetMethodInd(), file), binding_type);
            }
        }
    }

    // Slots, which do not have an ID.
    for(AbsoluteIndex i(0); i.Get() < traits.GetTraitsCount(); ++i)
    {
        const Abc::TraitInfo& trait_info = traits.GetTraitInfo(file.GetTraits(), i);

        // Filter out slots with ID.
        if (trait_info.HasSlotID())
        {
            // slot_id == 0 means "auto-assign slot" ...
            const SlotIndex slot_id(trait_info.GetSlotID());

            if (slot_id.IsValid())
                // This case should be already handled.
                continue;
        }

        SlotInfo::DataType binding_type = SlotInfo::DT_ValueArray;
        const Abc::Multiname& mn = trait_info.GetName(file.GetConstPool());
        const AbsoluteIndex ind(trait_info.HasMethodInd() ? -1 : static_cast<SPInt>(FixedValueSlotNumber++));
        bool _const = false;

        // Get binding_type.
        switch(trait_info.GetType())
        {
        case Abc::TraitInfo::tConst:
            _const = true;
        case Abc::TraitInfo::tSlot:
            break;
        case Abc::TraitInfo::tMethod:
            binding_type = SlotInfo::DT_Code;
            break;
        case Abc::TraitInfo::tGetter:
            binding_type = SlotInfo::DT_Get;
            break;
        case Abc::TraitInfo::tSetter:
            binding_type = SlotInfo::DT_Set;
            break;
        case Abc::TraitInfo::tClass:
            break;
        case Abc::TraitInfo::tFunction:
            binding_type = SlotInfo::DT_Code;
            break;
        default:
            SF_ASSERT(false);
            break;
        }

        // Auto-assign a slot number ...

        /* DO NOT delete this code.
        // Try to resolve type in place.
        // This doesn't work because of dependencies.
        const ClassTraits::Traits* tr = NULL;
        if (trait_info.IsData())
        {
            const Abc::Multiname& tmn = trait_info.GetTypeName(file.GetAbcFile());
            tr = GetVM().Resolve2ClassTraits(file, tmn);
            SF_ASSERT(tr);
        }
        */

        // We do not use ClassTraits::Traits here because we want to postpone name resolution.
        // Otherwise we will get problems with dependencies.
        AddSlot(
            file.GetInternedString(mn.GetNameInd()),
            Pickable<Instances::Namespace>(&file.GetInternedNamespace(mn.GetNamespaceInd()), PickValue), 
            file,
            trait_info,
            binding_type, 
            ind,
            _const
            );

        if (trait_info.HasMethodInd())
        {
            // Initialize a virtual table ...
            // Add2VT will also bind the slot.
            Add2VT(GetVT(), GetTopSlotInfo(), Value(trait_info.GetMethodInd(), file), binding_type);
        }
    }
}

// new_bt - new binding type.
void Traits::Add2VT(VTable& vt, SlotInfo& si, const Value& v, SlotInfo::DataType new_bt)
{
    SF_ASSERT(new_bt == SlotInfo::DT_Code || new_bt == SlotInfo::DT_Get || new_bt == SlotInfo::DT_Set);
    AbsoluteIndex method_ind = si.GetValueInd();
    
    if (method_ind.IsValid())
    {
        // bt - binding type.
        const SlotInfo::DataType bt = si.GetBindingType();
        
        // Check for possible problems.
        if (bt == SlotInfo::DT_Code && new_bt != bt)
        {
            // We do not want to assign a Getter/Setter value on top of regular code.
            SF_ASSERT(false);
            return;
        } else if ((bt == SlotInfo::DT_GetSet || bt == SlotInfo::DT_Get || bt == SlotInfo::DT_Set) && new_bt == SlotInfo::DT_Code)
        {
            // Code instead of a Getter/Setter.
            SF_ASSERT(false);
            return;
        }
        
        // Set the value.
        vt.SetMethod(method_ind, v, new_bt, *this DEBUG_ARG(si.GetName()));
        
        // Rebind if necessary.
        if ((bt == SlotInfo::DT_Get && new_bt == SlotInfo::DT_Set) || (bt == SlotInfo::DT_Set && new_bt == SlotInfo::DT_Get))
            si.Bind(SlotInfo::DT_GetSet, method_ind);
    } else
    {
        // Add a new record to the virtual table ...
        si.Bind(new_bt, vt.AddMethod(v, new_bt, *this DEBUG_ARG(si.GetName())));
        
        // DO NOT delete code below.
        // Not correct.
        //OwnSlotNumber += (new_bt == SlotInfo::DT_Code ? 1 : 2);

        // It will be only one slot but it may have two records in virtual table.
        ++OwnSlotNumber;
    }
}

Instances::Namespace& Traits::GetPublicNamespace() const
{
    return GetVM().GetPublicNamespace();
}

const Instances::Namespace& Traits::GetPrivateNamespace()
{
    if (!PrivateNamespace)
    {
        PrivateNamespace = GetVM().MakeNamespace(Abc::NS_Private);
        PrivateNamespace->SetOwner(*this);
    }
    
    return *PrivateNamespace;
}

void Traits::InitOnDemand() const
{
    ; // Do nothing.
}

void Traits::SetProtectedNamespace(Pickable<Instances::Namespace> ns)
{
    ProtectedNamespace = ns;
    ProtectedNamespace->SetOwner(*this);
}

void Traits::Add2VTableStoredScopeStack(const Value& o)
{
    GetVT().Add2StoredScopeStack(o);
}

void Traits::StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss)
{
    GetVT().StoreScopeStack(baseSSInd, ss);
}

const ScopeStackType& Traits::GetStoredScopeStack() const
{
    return GetVT().GetStoredScopeStack();
}

void Traits::AddSlot(RelativeIndex ind, const ASString& name, 
                     Pickable<const Instances::Namespace> ns,
                     const ClassTraits::Traits* tr, SlotInfo::DataType dt, 
                     AbsoluteIndex offset, bool _const)
{
    const AbsoluteIndex index(ind, *this);

    while (index.Get() > GetSlots().GetSize())
    {
        // Use empty string as a key for dummy records ...
        AddSlot(GetStringManager().CreateEmptyString(), SlotInfo());
    }

    if (index.Get() == GetSlots().GetSize())
        // Add data ...
        AddSlot(name, ns, tr, dt, offset, _const);
    else
        // Set data ...
        SetSlot(index, name, ns, tr, dt, offset, _const);
}

void Traits::AddSlot(RelativeIndex ind, const ASString& name, Pickable<const Instances::Namespace> ns,
             VMAbcFile& file, const Abc::TraitInfo& ti, SlotInfo::DataType dt, AbsoluteIndex offset,
             bool _const)
{
    const AbsoluteIndex index(ind, *this);

    while (index.Get() > GetSlots().GetSize())
    {
        // Use empty string as a key for dummy records ...
        AddSlot(GetStringManager().CreateEmptyString(), SlotInfo());
    }

    if (index.Get() == GetSlots().GetSize())
        // Add data ...
        AddSlot(name, ns, file, ti, dt, offset, _const);
    else
        // Set data ...
        SetSlot(index, name, ns, file, ti, dt, offset, _const);
}

void Traits::AddSlot(const ASString& name, Pickable<const Instances::Namespace> ns, const ClassTraits::Traits* tr, SlotInfo::DataType dt, AbsoluteIndex offset, bool _const)
{
    AddSlot(
        name, 
        SlotInfo(
            ns, 
            tr,
            (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
            DEBUG_ARG(name.GetNode())
            )
        );
    GetTopSlotInfo().Bind(dt, offset);
}

void Traits::Add2VT(const ThunkInfo& func)
{
    ASString method_name = GetStringManager().CreateConstString(func.Name);
    Pickable<Instances::Namespace> ns;
    const bool empty_ns_name = func.NamespaceName == NULL || *func.NamespaceName == 0;

    if (!empty_ns_name)
    {
        if (func.NamespaceName == NS_AS3 || strcmp(func.NamespaceName, NS_AS3) == 0)
            ns = Pickable<Instances::Namespace>(&GetVM().GetAS3Namespace(), PickValue);
        else
            ns = GetVM().MakeInternedNamespace(Abc::NS_Public, func.NamespaceName);
    }
    else
    {
        //ns = &GetVM().GetPublicNamespace();
        const bool empty_class_name = func.ClassName == NULL || *func.ClassName == 0;

        if (empty_class_name)
        {
            const bool empty_pkg_name = func.PackageName == NULL || *func.PackageName == 0;
            if (empty_pkg_name)
                ns = GetVM().MakePublicNamespace();
            else
                ns = GetVM().MakeInternedNamespace(Abc::NS_Public, func.PackageName);
        }
        else
            ns = GetVM().MakePublicNamespace();

        /* DO NOT delete this code.
        if (!func.PackageName || *func.PackageName == 0)
            ns = &GetVM().GetPublicNamespace();
        else
            ns = &GetVM().MakeInternedNamespace(func.NsKind, func.PackageName);
        */

        /* DO NOT delete this code.
        if (!func.PackageName && !func.ClassName)
            ns = &GetVM().GetPublicNamespace();
        else
        {
            ASString ns_name = GetVM().GetStringManager().CreateEmptyString();
            if (func.PackageName)
                ns_name += func.PackageName;

            if (func.ClassName)
            {
                if (func.PackageName)
                    ns_name += ".";

                ns_name += func.ClassName;
            }

            ns = &GetVM().MakeInternedNamespace(func.NsKind, ns_name);
        }
        */
    }

    SlotInfo& si = FindAddSlotInfo(
        method_name,
        SlotInfo(
            ns, 
            NULL,
            SlotInfo::aDontEnum 
            DEBUG_ARG(method_name.GetNode())
        )
    );

    Add2VT(GetVT(), si, func, GetDataType(func.GetCodeType()));
}

VMAbcFile* Traits::GetFilePtr() const
{
    return NULL;
}

bool Traits::IsParentOf(const Traits& other) const
{
    const Traits* ot = &other;

    while (ot->GetParent())
    {
        ot = ot->GetParent();

        if (ot == this)
            return true;
    }

    return false;
}

void Traits::SetSlot(AbsoluteIndex index, const ASString& name, Pickable<const Instances::Namespace> ns, const ClassTraits::Traits* tr, SlotInfo::DataType dt, AbsoluteIndex offset, bool _const /*= false*/)
{
    SF_ASSERT(index.Get() < GetSlots().GetSize());

//     const Abc::Multiname& mn = ti.GetTypeName(file.GetAbcFile());
//     const ClassTraits::Traits* tr = GetVM().Resolve2ClassTraits(file, mn);;

    SetSlotInfo(
        index,
        name,
        SlotInfo(
            ns,
            tr,
            (_const ? (SlotInfo::aDontEnum | SlotInfo::aReadOnly) : SlotInfo::aDontEnum)
            DEBUG_ARG(name.GetNode())
            )
        );

    SlotInfo& si = GetSlotInfo(index);
    si.Bind(dt, offset);
}

const Class& Traits::GetConstructor() const
{
    if (!pConstructor.Get())
    {
        InitOnDemand();
        SF_ASSERT(pConstructor.Get());
    }

    return *pConstructor;
}

Class& Traits::GetConstructor()
{
    if (!pConstructor.Get())
    {
        InitOnDemand();
        SF_ASSERT(pConstructor.Get());
    }

    return *pConstructor;
}

namespace ClassTraits
{

    Traits::Traits(VM& vm, const ClassTraits::Traits* pt)
        : AS3::Traits(vm, vm.GetClassClass(), pt, DynamicObject) // Classes are Dynamic Objects.
    {
        TraitsType = Traits_Class;
        RegisterSlots();
        SF_ASSERT(SlotsAreBound2Values());
    }

    Traits::Traits(VM& vm)
        : AS3::Traits(vm)
    {
        TraitsType = Traits_Class;
//         RegisterSlots();
//         SF_ASSERT(SlotsAreBound2Values());
    }

    void Traits::ForEachChild_GC(GcOp op) const
    {
        AS3::Traits::ForEachChild_GC(op);

        AS3::ForEachChild_GC<InstanceTraits::Traits, Mem_Stat>(ITraits, op);
    }

    void Traits::MakeObject(Value& result)
    {
        result = &GetInstanceTraits().GetClass();
    }

    void Traits::RegisterSlots()
    {
        /* DO NOT delete this code
        // length as a property.
        AddSlot(
        "length", 
        GetPublicNamespace(),
        NULL,
        SlotInfo::DT_Int, // We cannot use GetSlotInfoType() here.
        AbsoluteIndex(OFFSETOF(Class, Length))
        );
        // Prototype as a property.
        AddSlot(
        "prototype", 
        GetPublicNamespace(),
        NULL,
        SlotInfo::DT_Object, // We cannot use GetSlotInfoType() here.
        AbsoluteIndex(OFFSETOF(Class, pPrototype))
        );

        // Newer version.
        DefFixed(&Class::prototypeGet, "", "", "prototype", NULL, 0, 0, CT_Get).
        Def(&Class::lengthGet, "", "", "length", NULL, 0, 0, CT_Get).
        Def(&Class::toString, "", "", "toString", NULL, 0, 0);
        */

        static ThunkInfo ti[] = {
            {&Instances::Object::AS3isPrototypeOf, "", "Object", "isPrototypeOf", NS_AS3, CT_Method, 1, 1},
            {&Instances::Object::AS3hasOwnProperty, "", "Object", "hasOwnProperty", NS_AS3, CT_Method, 1, 1},
            {&Instances::Object::AS3propertyIsEnumerable, "", "Object", "propertyIsEnumerable", NS_AS3, CT_Method, 1, 1},
        };
        for (unsigned i = 0; i < NUMBEROF(ti); ++i)
            Add2VT(ti[i]);

        static ThunkInfo f[] = {
            {MakeThunkFunc<__LINE__>(&Class::prototypeGet), "", "", "prototype", NULL, CT_Get, 0, 0},
            {MakeThunkFunc<__LINE__>(&Class::lengthGet), "", "", "length", NULL, CT_Get, 0, 0},
            {MakeThunkFunc<__LINE__>(&Class::toString), "", "", "toString", NULL, CT_Method, 0, 0},
        };
        for (unsigned i = 0; i < NUMBEROF(f); ++i)
            Add2VT(f[i]);

        // FirstOwnSlotInd += own_slot_number;
        FirstOwnSlotInd += 3 + 3;
    }

    bool Traits::IsParentTypeOf(const Traits& other) const
    {
        // let valueType refer to *this* type. If valueType is the same as otherType, result
        // is true. If otherType is a base type of valueType, or an implemented interface of valueType,
        // then result is true. Otherwise result is set to false.

        if (this == &other)
            return true;

        const Traits* tr = &other;

        while (tr)
        {
            // Check implemented interfaces.
            const InstanceTraits::Traits& itr = tr->GetInstanceTraits();
            if (itr.SupportsInterface(*this))
                return true;

            tr = tr->GetParent();
            if (tr == this)
                return true;
        }

        return false;
    }

    bool Traits::Coerce(const Value& value, Value& result) const
    {
        return CoerceValue(value, result);
    }

    bool Traits::CoerceValue(const Value& value, Value& result) const
    {
        // Primitives can be coerced to their own return types and Object,
        // as well as Number/unsigned when appropriate.
        // TBD: IsNumber/etc needs to be verified for integers.

        switch (value.GetKind())
        {
        case Value::kUndefined:
            if (GetVM().IsClassClass(*this))
                result = value;
            else
                result.SetNull();
            break;
        case Value::kInt:
            if (!GetVM().IsObject(*this) && !GetVM().IsNumber(*this) && !GetVM().IsInt(*this))
                return false;
            result.SetNumber(value.AsInt());
            break;
        case Value::kUInt:
            if (!GetVM().IsObject(*this) && !GetVM().IsNumber(*this) && !GetVM().IsUint(*this))
                return false;
            result.SetNumber(value.AsUInt());
            break;
        case Value::kThunk:
        case Value::kMethodInd:
            if (!GetVM().IsObject(*this) && !GetVM().IsFunction(*this))
                return false;
            result = value;
            break;

        case Value::kBoolean:
            if (!GetVM().IsObject(*this) && !GetVM().IsBoolean(*this))
                return false;
            result = value;
            break;

        case Value::kNumber:
            if (!GetVM().IsObject(*this) && !GetVM().IsNumber(*this))
                return false;
            result = value;
            break;

        case Value::kString:
            if (!GetVM().IsObject(*this) && !GetVM().IsString(*this))
                return false;
            result = value;
            break;
        case Value::kNamespace:
            if (!GetVM().IsObject(*this) && !GetVM().IsNamespace(*this))
                return false;
            result = value;
            break;
        case Value::kFunction:
        case Value::kObject:
        case Value::kThunkFunction:
            // We do not need to get classes of primitive objects because they all are final.
            if (value.GetObject())
            {
                const Traits& other = GetVM().GetClassTraits(value);

                if (IsParentTypeOf(other))
                    result = value;
                else
                    return false;
            }
            else
                // null
                result = value;

            break;
        case Value::kThunkClosure:
            // I have no idea how to coerce a thunk.
            return false;
            break;
        case Value::kClass:
            // We do not need to get classes of primitive objects because they all are final.
            if (IsParentTypeOf(value.AsClass().GetClassTraits()))
                result = value;
            else
                return false;

            break;
        case Value::kMethodClosure:
            if (value.GetFunct())
            {
                const Instances::Function* funct = value.GetFunct();
                SF_ASSERT(funct);
                const Class& cl = funct->GetClass();
                // No declaration of Instances::Function at this point.
                const Traits& other = cl.GetClassTraits();

                if (IsParentTypeOf(other))
                    result = value;
                else
                    return false;
            }
            // Theoretically, there should be no null here.
        }

        return true;
    }

    ASString Traits::GetName() const
    {
        return GetInstanceTraits().GetName();
    }
    ASString Traits::GetQualifiedName(QNameFormat f) const
    {
        return GetInstanceTraits().GetQualifiedName(f);
    }

} // namespace ClassTraits

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

