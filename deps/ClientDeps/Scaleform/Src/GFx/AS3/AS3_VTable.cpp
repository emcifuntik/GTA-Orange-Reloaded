/**********************************************************************

Filename    :   AS3_VTable.cpp
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

#include "AS3_VTable.h"
#include "AS3_VM.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
VTable::VTable(VM& _vm)
: pVM(&_vm)
, InitScope(_vm.GetMemoryHeap())
{
    const ScopeStackType& ss = pVM->GetScopeStack();
    const UPInt size = ss.GetSize();

    for (UPInt i = pVM->GetScopeStackBaseInd(); i < size; ++i)
        InitScope.PushBack(ss[i]);
}

VTable::VTable(const VTable& parent)
: pVM(parent.pVM)
, VTMethods(parent.VTMethods) // Deep copy.
, InitScope(parent.GetVM().GetMemoryHeap())
{
    const ScopeStackType& ss = pVM->GetScopeStack();
    const UPInt size = ss.GetSize();

    for (UPInt i = pVM->GetScopeStackBaseInd(); i < size; ++i)
        InitScope.PushBack(ss[i]);
}

void VTable::GetMethod(Value& value, AbsoluteIndex ind, Object& _this) const
{
    const Value& m = VTMethods[ind.Get()];
    
    switch(m.GetKind())
    {
    case Value::kThunk:
//         // !!! This is a hack ...
//         // Actually, *Traits* is a hack ...
//         value = GetVM().ClassMethodClosure->MakeInstance(
//             (InstanceTraits::Traits&)const_cast<const Object&>(_this).GetTraits(), 
//             _this, 
//             m.AsThunk()
//             );

        // Make a ThunkClosure.
        value = Value(&_this, m.AsThunk());
        return;
    case Value::kFunction:
        // Make a MethodClosure.
        value = Value(&_this, const_cast<Instances::Function*>(&m.AsFunction()));
        return;
//     case Value::kMethodInd:
//         // This case should be handled by SetMethod/AddMethod.
//         value = GetVM().ClassMethodClosure->MakeInstance2(m.GetMethodInd(), Value(&_this), GetStoredScopeStack());
    default:
        break;
    }
    
    value = m;
}

void VTable::SetMethod(AbsoluteIndex ind, const Value& m, SlotInfo::DataType dt, const Traits& ot DEBUG_ARG(const ASString& name))
{
    Value* vt_value = NULL;
    
    switch(dt)
    {
    case SlotInfo::DT_Code:
    case SlotInfo::DT_Get:
        vt_value = &VTMethods[ind.Get()];
        break;
    case SlotInfo::DT_Set:
        vt_value = &VTMethods[ind.Get() + 1];
        break;
    default:
        SF_ASSERT(false);
        break;
    }
    
    if (m.GetKind() == Value::kMethodInd)
        // Make a function object instead.
        *vt_value = GetVM().GetClassFunction().MakeInstance2(m.GetAbcFile(), m.GetMethodInd(), Value::GetUndefined(), GetStoredScopeStack(), ot DEBUG_ARG(name));
    else
        *vt_value = m;

    // DO NOT delete code below.
    /*
    // This code is an optimization. Unfortunately, it doesn't fork for the time being
    // because Value::kMethodInd misses stored scope stack.
    // This assert is not correct here because Set method can be called for Setters/Getters.
    // SF_ASSERT(m.GetKind() != Value::kMethodInd);
    *vt_value = m;
    */
}

AbsoluteIndex VTable::AddMethod(const Value& m, SlotInfo::DataType dt, const Traits& ot DEBUG_ARG(const ASString& name))
{
    int ind;
    
    switch(dt)
    {
    case SlotInfo::DT_Code:
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        break;
    case SlotInfo::DT_Get:
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        // Place for a setter.
        VTMethods.PushDefault();
        break;
    case SlotInfo::DT_Set:
        // Place for a getter.
        VTMethods.PushDefault();
        ind = VTMethods.GetSize();
        VTMethods.PushDefault();
        break;
    default:
        SF_ASSERT(false);
        ind = 0;
        break;
    }
    
    if (m.GetKind() == Value::kMethodInd)
        // Make a function object instead.
        VTMethods[ind] = GetVM().GetClassMethodClosure().MakeInstance2(m.GetAbcFile(), m.GetMethodInd(), Value::GetUndefined(), GetStoredScopeStack(), ot DEBUG_ARG(name));
    else
        VTMethods[ind] = m;
    
    return AbsoluteIndex(VTMethods.GetSize() - (dt == SlotInfo::DT_Code ? 1 : 2));
}

void VTable::Execute(AbsoluteIndex ind, const Value& _this, unsigned argc, const Value* argv, bool discard_result)
{
    SF_ASSERT(ind.IsValid());
    const Value& v = VTMethods[ind.Get()];
    
    GetVM().Execute(v, _this, argc, argv, discard_result);
}

void VTable::StoreScopeStack(const UPInt baseSSInd, const ScopeStackType& ss)
{
    const UPInt size = ss.GetSize();

    for (UPInt i = baseSSInd; i < size; ++i)
        InitScope.PushBack(ss[i]);

    // Populate functions with a new Scope Stack.
    for (UPInt i = 0; i < VTMethods.GetSize(); ++i)
    {
        Value& v = VTMethods[i];

        if (v.GetKind() == Value::kFunction)
            v.AsFunction().StoreScopeStack(baseSSInd, ss);
    }
}

void VTable::Add2StoredScopeStack(const Value& o)
{
    InitScope.PushBack(o);

    for (UPInt i = 0; i < VTMethods.GetSize(); ++i)
    {
        Value& v = VTMethods[i];

        if (v.GetKind() == Value::kFunction)
            v.AsFunction().Add2StoredScopeStack(o);
    }
}
}}} // namespace Scaleform { namespace GFx { namespace AS3 {

