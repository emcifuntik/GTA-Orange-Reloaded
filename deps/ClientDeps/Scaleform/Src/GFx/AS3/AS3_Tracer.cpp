/**********************************************************************

Filename    :   AS3_Tracer.cpp
Content     :   
Created     :   Sep, 2010
Authors     :   Sergey Sikorskiy

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "AS3_Tracer.h"
#include "AS3_AsString.h"
#include "AS3_VTable.h"
#include "Obj/AS3_Obj_Array.h"
#include "Obj/AS3_Obj_UserDefined.h"
#include "Obj/Vec/AS3_Obj_Vec_Vector_object.h"
#include "Obj/AS3_Obj_int.h"
#include "Obj/AS3_Obj_uint.h"
#include "Obj/AS3_Obj_Number.h"
#include "Obj/AS3_Obj_Boolean.h"
#include "Obj/AS3_Obj_String.h"

#define NEW_CODE
#define MULTINAME_OPTIMIZATION

namespace Scaleform { namespace GFx { namespace AS3 
{

#ifdef USE_WORDCODE

namespace TR 
{

    ///////////////////////////////////////////////////////////////////////////
    void StackReader::CheckObject(const Value& v) const
    {
        SF_UNUSED1(v);

        /* Disable checks temporarily till we implement all operations with stack.
        if (v.IsNull())
        {
            // A TypeError is thrown if obj is null or undefined.
            GetVM().ThrowVerifyError(VM::eConvertNullToObjectError DEBUG_ARG("Cannot access a property or method of a null object reference"));
        }
        else if (v.IsUndefined())
        {
            GetVM().ThrowVerifyError(VM::eConvertUndefinedToObjectError DEBUG_ARG("A term is undefined and has no properties"));
        }
        */
    }

    int StackReader::Read(Multiname& obj) const
    {
        switch (obj.GetKind())
        {
        case Abc::MN_QName:
        case Abc::MN_QNameA:
            // QName is not read from stack; both name and namespase known at compile-time.
            break;
        case Abc::MN_RTQName:
        case Abc::MN_RTQNameA:
            // Name is known at compile time, Namespace is resolved at runtime.

            /*
            // For maniacs.
            CheckObject(GetState().BackOpValue());
            if (GetVM().IsException())
                break;
            */

            // !!! We should get a Namespace here.
            // It is up to verifier to check that.
            // In our case we will just get "any namespace".
            GetState().PopOpValue();
            return 1;
        case Abc::MN_RTQNameL:
        case Abc::MN_RTQNameLA:
            // Both name and namespace are resolved at runtime.
            // We will get "any name" here.
            GetState().PopOpValue();

            /*
            // For maniacs.
            CheckObject(GetState().BackOpValue());
            if (GetVM().IsException())
                break;
            */

            // !!! We should get a Namespace here.
            // It is up to verifier to check that.
            // In our case we will just get "any namespace".
            //GetState().PopOpValue();
            obj.SetRTName(GetState().PopOpValue());
            return 2;
        case Abc::MN_Multiname:
        case Abc::MN_MultinameA:
            // MInfo is not supposed to be read from stack.
            break;
        case Abc::MN_MultinameL:
        case Abc::MN_MultinameLA:
            // Namespace set is known at compile time, Name is resolved at runtime.        
            // name_ind is a string, and we will get an int or an uint as an index instead;
            // We will get "any name" here.
            //GetState().PopOpValue();
            obj.SetRTName(GetState().PopOpValue());
            return 1;
        case Abc::MN_Typename:
            // Recursive!!!?
            GetVM().GetUI().Output(FlashUI::Output_Warning, "Reading chained multiname in itself.");
            return Read(obj);
        default:
            SF_ASSERT(false);
            break;
        }

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    ReadArgs::ReadArgs(VM& vm, State& s, UInt32 arg_count )
        : StackReader(vm, s)
        , ArgNum(arg_count)
        , CallArgs(GetVM().GetMemoryHeap())
    {
        if (arg_count)
        {
            if (arg_count <= eFixedArrSize)
            {
                unsigned j = 0;
                for (UPInt i = GetOpStack().GetSize() - arg_count; i < GetOpStack().GetSize(); ++i, ++j)
                {
                    FixedArr[j] = GetState().GetOpValue(i);
                }
            }
            else
            {
                for (UPInt i = GetOpStack().GetSize() - arg_count; i < GetOpStack().GetSize(); ++i)
                {
                    CallArgs.PushBack(GetState().GetOpValue(i));
                }
            }

#ifdef DISABLE_TRACING
            ; // Nothing
#else
            GetOpStack().Resize(GetOpStack().GetSize() - arg_count);
#endif
        }

        HaveRead(arg_count);
    }

    ///////////////////////////////////////////////////////////////////////////////
    void State::exec_getsuper(UInt32 index)
    {
        PushCode(index);

        TR::ReadMnObject args(GetFile(), *this, index);

    //     if (IsException())
    //         return;

    //     Value value;
    //     // MA: eReadSealedError or eWriteOplyError possible in property
    //     if (!GetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), value, args.ArgObject, args.ArgMN))
    //         return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't find parent property"));

        // We can do better than this.
        PushOp(Value());
    }

    void State::exec_setsuper(UInt32 index) 
    {
        PushCode(index);

        TR::ReadValueMnObject args(GetFile(), *this, index);

    //     if (IsException())
    //         return;

    //     if (!SetSuperProperty(*this, GetCurrCallFrame().GetOriginationTraits(), args.ArgObject, args.ArgMN, args.ArgValue))
    //         return ThrowReferenceError(VM::eNotImplementedYet DEBUG_ARG("Couldn't set parent property"));
    }

    void State::exec_construct(UInt32 arg_count)
    {
        PushCode(arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);

        PushOp(args.ArgObject);
    }

    void State::exec_constructsuper(UInt32 arg_count)
    {
        PushCode(arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);
    }

    void State::exec_constructprop(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);

#ifndef DISABLE_TRACING
        if (args.ArgObject.GetKind() == Value::kInstanceTraits)
        {
            PushOp(Value(args.ArgObject.GetInstanceTraits()));
            return;
        }

        if (!args.ArgMN.IsRunTime())
        {
            // Ignore run-time multinames for the time being.
            VM& vm = GetVM();

            // Find a property.
            const ClassTraits::Traits* tr = FindScopeTraits(vm, 0, GetScopeStack(), args.ArgMN);

            // Look in Saved Scope.
            /* No Saved Scope at this time.
            if (!tr)
            {
                const ScopeStackType* ss = CallStack.Back().GetSavedScope();

                if (ss)
                    tr = FindScopeTraits(vm, *ss, args.ArgMN);
            }
            */

            // Look in Global Objects.
            // !!! WARNING:
            // !!! We are not supposed to search in Global Objects with this opcode.
            if (!tr)
                tr = FindScopeTraits(vm, 0, vm.GetGlobalObjects(), args.ArgMN);

            if (tr)
            {
                // Push found object ...
                PushOp(Value(tr->GetInstanceTraits()));
                return;
            }
        }
#endif

        PushOp(Value(GetObjectType()));
    }

    InstanceTraits::Traits& State::GetFunctType(const Value& value) const
    {
        switch (value.GetKind())
        {
        case Value::kThunk:
            return GetVM().GetClassTraitsFunction().GetThunkTraits();
        case Value::kThunkClosure:
            return GetVM().GetClassTraitsFunction().GetThunkFunctionTraits();
        default:
            break;
        }

        return GetVM().GetClassTraitsFunction().GetInstanceTraits();
    }

    InstanceTraits::Traits& State::GetFunctReturnType(const Value& value) const
    {
        switch (value.GetKind())
        {
        case Value::kFunction:
            {
                const Instances::Function& f = value.AsFunction();
                const ClassTraits::Traits* ctr = GetVM().Resolve2ClassTraits(f.GetFile(), f.GetReturnType());

                if (ctr)
                    return ctr->GetInstanceTraits();
            }
            break;
        case Value::kMethodClosure:
            {
                Instances::Function* f = value.GetFunct();
                if (f)
                {
                    const ClassTraits::Traits* ctr = GetVM().Resolve2ClassTraits(f->GetFile(), f->GetReturnType());

                    if (ctr)
                        return ctr->GetInstanceTraits();
                }
            }
            break;
        default:
            break;
        }

        return GetObjectType();
    }

    Value State::GetPropertyType(const Traits& obj_traits, const SlotInfo& si) const
    {
        if (si.IsCode() && si.GetValueInd().IsValid() && !obj_traits.IsInterface())
        {
            if (si.GetBindingType() == SlotInfo::DT_Code)
                // Retrieve function type.
                return GetFunctType(obj_traits.GetVT().Get(si.GetValueInd()));
            else
                // Retrieve function type.
                return GetFunctReturnType(obj_traits.GetVT().Get(si.GetValueInd()));
        }
        else if (si.IsClass())
            return const_cast<ClassTraits::Traits&>(const_cast<SlotInfo&>(si).GetDataType(GetVM()));

        return const_cast<SlotInfo&>(si).GetDataType(GetVM()).GetInstanceTraits();
    }

    void State::exec_call(UInt32 arg_count)
    {
        PushCode(arg_count);

        TR::ReadArgsObjectValue args(GetVM(), *this, arg_count);

        PushOp(Value(GetFunctReturnType(args.value)));
    }

    void State::exec_callmethod(UInt32 method_index, UInt32 arg_count)
    {
        PushCode(method_index);
        PushCode(arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);

        // We can do better than this.
        //PushOp(Value());
        PushOp(Value(GetObjectType()));
    }

    void State::exec_callstatic(UInt32 method_index, UInt32 arg_count)
    {
        PushCode(method_index);
        PushCode(arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);

        // We can do better than this.
        //PushOp(Value());
        PushOp(Value(GetObjectType()));
    }

    void State::exec_callsuper(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);

        // We can do better than this.
        //PushOp(Value());
        PushOp(Value(GetObjectType()));
    }

    void State::exec_callproperty(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);

        PushOp(Value(GetObjectType()));
    }

    void State::exec_callproplex(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);

#ifndef DISABLE_TRACING
        /*
        // Ignore run-time multinames for the time being.
        if (!args.ArgMN.IsRunTime())
        {
            VM& vm = GetVM();

            // Find a property.
            const ClassTraits::Traits* tr = FindScopeTraits(vm, 0, GetScopeStack(), args.ArgMN);

            // Look in Saved Scope.
            if (!tr)
            {
                const ScopeStackType* ss = GetSavedScope();

                if (ss)
                    tr = FindScopeTraits(vm, *ss, args.ArgMN);
            }

            // Look in Global Objects.
            // !!! WARNING:
            // !!! We are not supposed to search in Global Objects with this opcode.
            if (!tr)
                tr = FindScopeTraits(vm, vm.GetGlobalObjects(), args.ArgMN);

            if (tr)
            {
                // Push found object ...
                PushOp(Value(tr->GetInstanceTraits()));
                return;
            }
        }
        */
#endif

        PushOp(Value(GetObjectType()));
    }

    void State::exec_callsupervoid(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);
    }

    void State::exec_callpropvoid(UInt32 mn_index, UInt32 arg_count)
    {
        PushCode(mn_index);
        PushCode(arg_count);

        TR::ReadArgsMnObject args(GetFile(), *this, arg_count, mn_index);
    }

    void State::exec_applytype(UInt32 arg_count)
    {
        PushCode(arg_count);

        TR::ReadArgsObject args(GetVM(), *this, arg_count);

        // Result is a type.
        PushOp(Value(GetClassType()));
    }

    void State::exec_newobject(UInt32 arg_count)
    {
        PushCode(arg_count);

        for (UInt32 i = 0; i < arg_count; ++i)
        {
            PopOp(); // value
            PopOp(); // name
        }

        PushOp(Value(GetObjectType()));
    }

    void State::exec_newarray(UInt32 arr_size)
    {
        PushCode(arr_size);

        for (UInt32 i = 0; i < arr_size; ++i)
        {
            PopOp(); // value
        }

        PushOp(Value(GetArrayType()));
    }

    void State::exec_getdescendants(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnObject args(GetFile(), *this, mn_index);

        // !!! This is a wrong result !!!
        PushOp(Value(GetObjectType(), true));
    }

    void State::exec_newcatch(UInt32 v)
    {
        PushCode(v);

        PushOp(Value(GetObjectType()));
    }

    PropRef State::FindProp(const Multiname& mn, ScopeType& stype, UPInt& scope_index, UPInt& slot_index)
    {
        // Ignore run-time multinames for the time being.
        if (!mn.IsRunTime())
        {
            VM& vm = GetVM();

            // 1) Check scope stack.
            {
                for (UPInt i = GetScopeStack().GetSize(); i > 0; --i)
                {
                    scope_index = i - 1;
                    const Value& v = GetScopeStack()[scope_index];
                    const Traits* tr = GetValueTraits(v);

                    if (v.GetWith())
                        return PropRef();

                    // Skip global object.
                    if (tr && !tr->IsGlobal())
                    {
                        const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_index, NULL);
                        if (si)
                        {
                            stype = stScopeStack;
                            return PropRef(v, si);
                        }
                    }
                }
            }

            // 2) Look in Saved Scope.
            // Saved Scope has classes, Activation objects, regular object, and a global object.
            // No primitive types so far.
            if (GetSavedScope())
            {
                const ValueArrayDH& ss = *GetSavedScope();
                for (UPInt i = ss.GetSize(); i > 0 ; --i)
                {
                    scope_index = i - 1;
                    const Value& v = ss[scope_index];
                    const Traits& tr = vm.GetValueTraits(v);

                    if (v.GetWith())
                        return PropRef();

                    /* Search in global objects has the same code.
                    if (tr.IsGlobal())
                        // We will deal with global objects later.
                        continue;
                    */

                    const SlotInfo* si = FindFixedSlot(vm, tr, mn, slot_index, NULL);

                    if (si)
                    {
                        // We found it

                        stype = stStoredScope;
                        return PropRef(v, si);
                    }
                }
            }

            // Check global objects. We are looking for classes.
            // We know about all registered class objects.
//             {
//                 const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(mn);
//                 if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
//                 {
//                     // Not correct. This should be a global object.
//                     // This is just a class.
//                     value = Value(&ctr->GetInstanceTraits().GetClass());
// 
//                     return stGlobalObject;
//                 }
//             }

            PropRef prop = FindScopeProperty(vm, 0, vm.GetGlobalObjects(), mn);
            if (prop)
            {
                stype = stGlobalObject;
                return prop;
            }
        }

        return PropRef();
    }

    void State::exec_findpropstrict(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMn args(GetFile(), *this, mn_index);

#ifndef DISABLE_TRACING
        UPInt scope_index;
        UPInt slot_index;
        ScopeType stype;
        const PropRef prop = FindProp(args.ArgMN, stype, scope_index, slot_index);

        if (prop)
        {
            PushOp(prop.GetValue());
            return;
        }
#endif

        PushOp(Value(GetObjectType()));
    }

    void State::exec_findproperty(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMn args(GetFile(), *this, mn_index);

#ifndef DISABLE_TRACING
        UPInt scope_index;
        UPInt slot_index;
        ScopeType stype;
        const PropRef prop = FindProp(args.ArgMN, stype, scope_index, slot_index);

        if (prop)
        {
            PushOp(prop.GetValue());
            return;
        }
#endif

        // Can be null.
        PushOp(Value(GetObjectType(), true));
    }

    void State::exec_getlex(UInt32 mn_index)
    {
        PushCode(mn_index);

        //TR::ReadMnCt args(GetFile(), *this, mn_index); // ReadMnCt() doesn't change stack state.

        PushOp(Value(GetObjectType()));
    }

    void State::exec_getabsobject(InstanceTraits::Traits& tr)
    {
        // It is a replacement opcode, so, we do not have to store mn_index.
        //TR::ReadMnCt args(GetFile(), *this, mn_index); // ReadMnCt() doesn't change stack state.

        PushOp(Value(tr));
    }

    void State::exec_setproperty(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadValueMnObject args(GetFile(), *this, mn_index);
    }

    void State::exec_getproperty(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnObject args(GetFile(), *this, mn_index);

#ifndef DISABLE_TRACING
        // Ignore run-time multinames.
        if (!args.ArgMN.IsRunTime())
        {
            const Traits* tr = GetValueTraits(args.ArgObject);
            if (tr)
            {
                // The property with the name specified by the multiname will be resolved in object, and the
                // value of that property will be pushed onto the stack. If the property is unresolved,
                // undefined is pushed onto the stack.

                UPInt index = 0;
                const SlotInfo* si = FindFixedSlot(GetVM(), *tr, args.ArgMN, index, (args.ArgObject.IsObject() ? args.ArgObject.GetObject() : NULL));
                if (si)
                {
                    PushOp(GetPropertyType(*tr, *si));
                    return;
                }
            }
        }

        PushOp(Value(GetObjectType()));
#endif
    }

    void State::exec_newactivation()
    {
        InstanceTraits::Activation& it = GetVM().GetActivationInstanceTraits(GetFile(), GetTracer().GetMethodBodyInfo() DEBUG_ARG(GetTracer().GetName()));

        PushOp(Value(it));
    }

    void State::exec_kill(UInt32 index)
    {
        PushCode(index);

        SetRegister(AbsoluteIndex(index), Value::GetUndefined());
    }

    void State::exec_pushbyte(SInt32 v)
    {
        PushCode(v);
        PushOp(GetSIntType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_i);
    }

    void State::exec_pushshort(SInt32 v)
    {
        PushCode(v);
        PushOp(GetSIntType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_i);
    }

    void State::exec_pushstring(SInt32 v)
    {
        PushCode(v);
        PushOp(GetStringType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_s);
    }

    void State::exec_pushint(SInt32 v)
    {
        PushCode(v);
        PushOp(GetSIntType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_i);
    }

    void State::exec_pushuint(SInt32 v)
    {
        PushCode(v);
        PushOp(GetUIntType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_u);
    }

    void State::exec_pushdouble(SInt32 v)
    {
        PushCode(v);
        PushOp(GetNumberType());
        GetTracer().SkipNextOpcode(bcp, Abc::Code::op_convert_d);
    }

    void State::exec_hasnext2(UInt32 object_reg, UInt32 index_reg)
    {
        PushCode(object_reg);
        PushCode(index_reg);

#ifndef DISABLE_TRACING
        AbsoluteIndex reg_ind(index_reg); // Value must be of type int.
        //AbsoluteIndex obj_ind(object_reg); // Can be set to null.

        // A kind of verification.
        // reg_ind can be undefined.
        SF_ASSERT(GetRegister(reg_ind).IsUndefined() || GetValueTraits(GetRegister(reg_ind)) == &GetSIntType());
#endif

        PushOp(Value(GetBooleanType()));
    }

    void State::exec_nextname()
    {
        PopOp(); // index: int
        PopOp(); // obj

        PushOp(Value::GetUndefined()); // name
    }

    void State::exec_hasnext()
    {
        PopOp(); // cur_index: int
        PopOp(); // obj

        PushOp(GetSIntType()); // next_index
    }

    void State::exec_nextvalue()
    {
        PopOp(); // index: int
        PopOp(); // obj: Value

        PushOp(Value(GetObjectType())); // next_value: Value, not null.
    }

    void State::exec_coerce(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnCtValue args(GetFile(), *this, mn_index);

        const ClassTraits::Traits* ctr = GetVM().Resolve2ClassTraits(GetFile(), args.ArgMN);

        if (ctr == NULL)
            return GetVM().ThrowVerifyError(VM::eClassNotFoundError);

        PushOp(Value(ctr->GetInstanceTraits()));
    }

    void State::exec_astypelate()
    {
        PopOp(); // class
        Value value = PopOpValue(); // value

        if (value.GetKind() == Value::kInstanceTraits)
            PushOp(value.GetInstanceTraits(), true);
        else if (value.GetKind() == Value::kClassTraits)
            PushOp(value.GetClassTraits(), true);
        else
            PushOp(Value());
    }

    void State::exec_add()
    {
        const Value _2 = PopOpValue(); // value2
        const Value _1 = PopOpValue(); // value1

        Tracer& tracer = GetTracer();
        const Traits* _1tr = tracer.GetValueTraits(_1);
        const Traits* _2tr = tracer.GetValueTraits(_2);

        // Strings.
        if (_1tr == &GetStringType() || _2tr == &GetStringType())
        {
            PushOp(GetStringType());
            return;
        }

        // Operands are primitives but strings.
        if (tracer.IsPrimitiveType(*_1tr) && tracer.IsPrimitiveType(*_2tr))
        {
            PushOp(GetNumberType());
            return;
        }

        // Result should be either String or Number, so, let us make it of Object type.
        PushOp(Value(GetObjectType()));
    }

    void State::exec_istype(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnCt args(GetFile(), *this, mn_index);

        ConvertOpTo(GetBooleanType());
    }

    void State::exec_pushscope()
    {
        // A TypeError is thrown if value is null or undefined.

        // It is safe to use BackOpValue() this way because op stack won't be midified.
        PushScope(BackOpValue());
        PopOp();
    }

    void State::exec_initproperty(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadValueMnObject args(GetFile(), *this, mn_index);
    }

    void State::exec_deleteproperty(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnObject args(GetFile(), *this, mn_index);

        PushOp(Value(GetBooleanType()));
    }

    void State::exec_getslot(UInt32 slot_index)
    {
        PushCode(slot_index);

        TR::ReadObject args(GetVM(), *this);

        Tracer& tracer = GetTracer();
        const Traits* vtr = tracer.GetValueTraits(args.ArgObject);

        if (vtr)
        {
            /*
            // Original code
            // Slot's indices begin from ONE.
            if (slot_index <= vtr->GetSlots().GetSize())
            {
                const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));

                PushOp(Value(ctr.GetInstanceTraits()));
            }
            else
            {
                // !!! Check that vtr is a dynamic type.
                PushOp(Value(GetObjectType()));
            }

            return;
            */

            // Slot's indices begin from ONE.
//             if (slot_index <= vtr->GetSlots().GetSize())
//             {
//                 const SlotInfo& si = vtr->GetSlotInfo(AbsoluteIndex(SlotIndex(slot_index), *vtr));
// 
//                 /* 
//                 // Attempt 1.
//                 // Class is not initialized yet because script wasn't executed yet.
//                 if (si.IsClassOrConst() && args.ArgObject.IsObject())
//                 {
//                     // In case this is a class or a const we can get a real value.
//                     // It is already initialized.
// 
//                     Value value;
// 
//                     if (si.GetSlotValueUnsafe(GetVM(), value, args.ArgObject.GetObject()))
//                     {
//                         PushOp(value);
//                         return;
//                     }
//                 }
//                 */
// 
//                 /*
//                 // Attempt 2.
//                 // Class is not initialized yet because script wasn't executed yet.
//                 if (si.IsClass())
//                 {
//                     const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));
// 
//                     PushOp(Value(&ctr.GetInstanceTraits().GetClass()));
//                     return;
//                 }
//                 else
//                 {
//                     // Use InstanceTraits.
// 
//                     const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));
// 
//                     PushOp(Value(ctr.GetInstanceTraits()));
//                     return;
//                 }
//                 */
//             }
// 
//             // !!! Check that vtr is a dynamic type.
//             PushOp(Value(GetObjectType()));
// 
//             return;

            // Slot's indices begin from ONE.
            if (slot_index <= vtr->GetSlots().GetSize())
            {
                const SlotInfo& si = vtr->GetSlotInfo(AbsoluteIndex(SlotIndex(slot_index), *vtr));
                const ClassTraits::Traits& ctr = tracer.GetSlotCTraits(*vtr, SlotIndex(slot_index));

                if (si.IsClass())
                    PushOp(Value(const_cast<ClassTraits::Traits&>(ctr)));
                else
                    PushOp(Value(ctr.GetInstanceTraits()));
            }
            else
            {
                // !!! Check that vtr is a dynamic type.
                PushOp(Value(GetObjectType()));
            }

            return;
        }

        PushOp(Value());
    }

    void State::exec_setslot(UInt32 slot_index)
    {
        PushCode(slot_index);

        TR::ReadValueObject args(GetVM(), *this);
    }

    void State::exec_astype(UInt32 mn_index)
    {
        PushCode(mn_index);

        TR::ReadMnCtValue args(GetFile(), *this, mn_index);
    }

    void State::exec_newclass(UInt32 v)
    {
        PushCode(v);

        PopOp(); // Base Class: Object or null

        const Abc::ClassInfo& class_info = GetFile().GetClasses().Get(v);
        const ClassTraits::UserDefined& ctr = GetVM().GetUserDefinedTraits(GetFile(), class_info);
        InstanceTraits::Traits& itr = ctr.GetInstanceTraits();

        PushOp(Value(itr));
    }

    State::State( const State& other ) : pTracer(other.pTracer)
        , bcp(other.bcp)
        , Registers(other.Registers)
        , OpStack(other.OpStack)
        , ScopeStack(other.ScopeStack)
    {
    }

    State::State( MemoryHeap* heap, Tracer& tr, Abc::TCodeOffset cp, const ValueArrayDH& r ) : pTracer(&tr)
        , bcp(cp)
        , Registers(r)
        , OpStack(heap)
        , ScopeStack(heap)
    {
    }

    State::State( Tracer& tr, Abc::TCodeOffset cp, const ValueArrayDH& r, const ValueArrayDH& o, const ValueArrayDH& s ) : pTracer(&tr)
        , bcp(cp)
        , Registers(r)
        , OpStack(o)
        , ScopeStack(s)
    {
    }

}

///////////////////////////////////////////////////////////////////////////////
Tracer::Tracer(
    MemoryHeap* heap,
    const CallFrame& cf
    )
    : Heap(heap)
    , CF(cf)
    , bcode(cf.GetMethodBodyInfo().GetCode().GetCode())
    , wcode(cf.GetMethodBodyInfo().wcode)
    , done(false)
    , curr_offset(0)
    , code(bcode.GetDataPtr())
    , code_end(bcode.GetSize())
    , opcode_pos(heap)
    , recalculate_pos(heap)
    , long_offset(heap)
    , loc_reg(heap)
    , States(heap)
    , PrintOffset(0)
    , CurrBlock(NULL)
{
    // Switch on State tracing
    {
        SetNeedToCheckOpCode(GetVM().GetUI().NeedToCheckOpCode());
        SetState(FlashUI::sStep);
        PrintOffset = GetVM().GetCallStack().GetSize();

        if (PrintOffset)
            --PrintOffset;
    }

    wcode.Clear();
    wcode.Reserve(bcode.GetSize());
    long_offset.Resize(bcode.GetSize());

    // Initialize registers ...
    {
        // Initialize *this*

        /*
        // Version 1.
        // Doesn't work in case of custom this.
        {
            Value& _this = CF.GetRegister(0);

            if (_this.IsObject() && _this.GetObject()->GetName() == "global")
                loc_reg.PushBack(_this);
            else if (_this.IsClass())
                loc_reg.PushBack(_this.AsClass().GetClassTraits());
            else
                loc_reg.PushBack(GetVM().GetInstanceTraits(_this));
        }
        */

        // Version 2.
        {
            Traits* ot = const_cast<Traits*>(CF.GetOriginationTraits());

            // We MAY NOT use real object for *this* because this may be passed
            // at run time and can be of different type
            if (ot)
            {
                if (ot->IsClassTraits())
                    loc_reg.PushBack(static_cast<ClassTraits::Traits&>(*ot));
                else
                    loc_reg.PushBack(static_cast<InstanceTraits::Traits&>(*ot));
            }
            else
                loc_reg.PushBack(GetObjectType());
        }

        /*
        // It doesn't work.
        // We MAY NOT use real object for *this* because this may be passed
        // at run time and can be of different type

        // Version 3.
        {
            Traits* ot = const_cast<Traits*>(CF.GetOriginationTraits());

            if (ot)
            {
                if (ot->IsGlobal())
                    loc_reg.PushBack(CF.GetRegister(0));
                else if (ot->IsClassTraits())
                    loc_reg.PushBack(static_cast<ClassTraits::Traits&>(*ot));
                else
                    loc_reg.PushBack(static_cast<InstanceTraits::Traits&>(*ot));
            }
            else
                loc_reg.PushBack(GetObjectType());
        }
        */

        ///////////
        // Set up the rest of the registers.
        ///////////

        const Abc::MethodInfo& mi = GetMethodBodyInfo().GetMethodInfo(GetAbcFile().GetMethods());
        const SInt32 maxInd = GetMethodBodyInfo().GetMaxLocalRegisterIndex();
//         maxInd -= (mi.NeedRest() || mi.NeedArguments()) ? 1 : 0;
//         const SInt32 paramCount = mi.GetParamCount();

        // Hack for Function::apply() and Function::call() (not done yet)

        for(SInt32 i = 1; i < maxInd; ++i)
            loc_reg.PushBack(Value::GetUndefined());

        // Get register's types.
        const UPInt param_count = mi.GetParamCount();
        UPInt reg = 1;

        // Registers 1 through method_info.param_count holds parameter values coerced to the declared types
        // of the parameters. If fewer than method_body_info.local_count values are supplied to the call then
        // the remaining values are either the values provided by default value declarations (optional arguments)
        // or the value undefined.

        for(UPInt i = 0; i < param_count; ++i)
        {
            const Abc::Multiname& amn = mi.GetParamType(GetAbcFile().GetConstPool(), i);
            const AS3::Multiname mn(GetFile(), amn);
            const ClassTraits::Traits* tr = GetVM().Resolve2ClassTraits(mn);

            if (tr)
                loc_reg[reg++] = tr->GetInstanceTraits();
            else
            {
                // That shouldn't happen.
                SF_ASSERT(false);
                loc_reg[reg++] = Value::GetUndefined();
            }
        }

        if (mi.NeedRest())
        {
            // If NEED_REST is set in method_info.flags, the method_info.param_count+1 register is set up to
            // reference an array that holds the superfluous arguments.

            loc_reg[reg++] = GetVM().GetClassTraitsArray().GetInstanceTraits();
        } else if (mi.NeedArguments())
        {
            // If NEED_ARGUMENTS is set in method_info.flags, the method_info.param_count+1 register is set up
            // to reference an “arguments” object that holds all the actual arguments: see ECMA-262 for more
            // information. (The AVM2 is not strictly compatible with ECMA-262; it creates an Array object for
            // the “arguments” object, whereas ECMA-262 requires a plain Object.)

            // "arguments" is a regular Array. There is no an explicit "arguments" class.
            loc_reg[reg++] = GetVM().GetClassTraitsArray().GetInstanceTraits();
        }
    }

    // Create initial state/block.
    {
        States.PushBack(TR::State(GetHeap(), *this, 0, loc_reg));
        if (CF.IsAdjustedStack())
            States.Back().GetOpStack().PushBack(loc_reg[0]);
        Blocks.PushBack(SF_HEAP_NEW(GetHeap()) TR::Block(0, 0));
    }
}

Tracer::~Tracer()
{
    // Free list elements.
    {
        TR::Block* block = Blocks.GetFirst();
        while(!Blocks.IsNull(block))
        {
            TR::Block* next = Blocks.GetNext(block);
            SF_FREE(block);
            block = next;
        }
        Blocks.Clear();
    }
}

void Tracer::EmitCode()
{
    using namespace Abc;

    // Sometimes compiler generates dummy code, which produces
    // absolute address outside of a code block. Such opcodes should
    // be dropped from a newly generated code. TBD.

    if (done)
        return;

#ifdef USE_DEBUGGER
    FlashUI& ui = GetUI();

    if (ui.NeedToCheckOpCode())
    {
        MakeOffset(ui, GetPrintOffset());

        ui.Output(FlashUI::Output_Action, "=== Start optimization ");
        DEBUG_CODE(ui.Output(FlashUI::Output_Action, GetName().ToCStr());)
        ui.Output(FlashUI::Output_Action, " ===\n");
    }
#endif

    const MethodBodyInfo::Exception& e = GetMethodBodyInfo().exception;

    // Create blocks for exceptions if any.
    {
        const TR::State* pState = &GetState(GetFirstBlock());

        for (UPInt i = 0; i < e.GetSize(); ++i)
        {
            const MethodBodyInfo::ExceptionInfo& ei = e.Get(i);

            // A "from" block is uninitialized, but AddBlock() will mark it as initialized.
            // We will need to reset this flag.
            TR::Block* try_block = AddBlock(*pState, ei.GetFrom(), false, false);

            // pState can be invalidated in AddBlock().
            pState = &GetState(GetFirstBlock());

            if (try_block)
            {
                // Reset the "initialized" flag.
                try_block->SetInitialized(false);
                try_block->SetTryBlock();
            }

            // ei.GetTo() is inclusive.
//             if (ei.GetTo() != ei.GetTargetPos())
//                 AddBlock(ei.GetTo());

            // !!! ei.GetTargetPos() can be a dead block, if there is no throw.
            // If there is no explicit throw opcode we need to simulate it,
            // otherwise stack will be broken.
            // We can specify only the beginning of the block.
            // catch_block is already initialized.
            // A "target" block is initialized.
            TR::Block* catch_block = AddBlock(*pState, ei.GetTargetPos(), false, false);

            // pState can be invalidated in AddBlock().
            pState = &GetState(GetFirstBlock());

            if (catch_block)
            {
                catch_block->SetCatchBlock();

                TR::State& state = GetState(*catch_block);
                const SInt32 et_ind = ei.GetExceptionTypeInd();

                if (et_ind == 0)
                {
                    // This handler can handle any type of exceptions ...
                    state.PushOp(Value(GetObjectType()));
                }
                else
                {
                    const ClassTraits::Traits* etr = GetVM().Resolve2ClassTraits(GetFile(), GetFile().GetConstPool().GetMultiname(et_ind));
                    SF_ASSERT(etr);

                    if (etr)
                    {
                        state.PushOp(Value(etr->GetInstanceTraits()));
                    }
                }
            }
        }
    }

    TraceBlock(0, GetFirstBlock());

    if (GetVM().IsException())
        return;

#ifdef USE_DEBUGGER
    if (ui.NeedToCheckOpCode())
    {
        MakeOffset(ui, GetPrintOffset());

        ui.Output(FlashUI::Output_Action, "=== Finish optimization ");
        DEBUG_CODE(ui.Output(FlashUI::Output_Action, GetName().ToCStr());)
        ui.Output(FlashUI::Output_Action, " ===\n");
    }
#endif

    // Recalculate positions.
    for(UPInt i = 0; i < recalculate_pos.GetSize(); ++i)
    {
        // Get position in wcode where an absolute address is stored.
        const Recalculate& r = recalculate_pos[i];
        const Abc::TCodeOffset pos = r.pos;
        const TOpCode::ValueType absolute_address = wcode[pos];

        // Fix relative offset.
        // Convert it back from absolute to relative.
        TOpCode::ValueType new_address = 0;

        // It is possible that compiler generates dummy code, which addresses
        // code outside of a code segment. Let's set address to ZERO in such
        // case.
        if (absolute_address < long_offset.GetSize())
        {
            // long version for debugging.
            new_address = static_cast<TOpCode::ValueType>(long_offset[absolute_address]);
            new_address -= static_cast<TOpCode::ValueType>(pos);
            new_address += r.base;
        }

        SF_ASSERT(new_address < wcode.GetSize());
        wcode[pos] = new_address;
    }

    // Create exception info for word code.
    {
        const MethodBodyInfo::Exception& e = GetMethodBodyInfo().exception;
        MethodBodyInfo::Exception& ne = GetMethodBodyInfo().wexception;
        for (UPInt i = 0; i < e.GetSize(); ++i)
        {
            const MethodBodyInfo::ExceptionInfo& ei = e.Get(i);

            ne.info.PushBack(MethodBodyInfo::ExceptionInfo(
                static_cast<SInt32>(long_offset[ei.GetFrom()]),
                static_cast<SInt32>(long_offset[ei.GetTo()]),
                static_cast<SInt32>(long_offset[ei.GetTargetPos()]),
                ei.GetExceptionTypeInd(),
                ei.GetVariableNameInd()
                ));
        }
    }

    done = true;
}

void Tracer::TraceBlock(Abc::TCodeOffset bcp, const TR::Block& initBlock)
{
    using namespace Abc;

    TR::Block*& pBlock = CurrBlock;
    pBlock = GetBlock(bcp);
    SF_ASSERT(pBlock);

    if (pBlock->IsInitializedBlock())
    {
        // Block is already initialized. We just need to merge states
        // and, probably, retrace it.
        MergeBlock(*pBlock, initBlock);
    }
    else
    {
        // Block is not yet initialized and traced.
        InitializeBlock(*pBlock, initBlock);
    }

    TR::State* pState = &GetState(*pBlock);

    /* Do not handle this for the time being.
    UPInt tryInd = 0;
    UPInt tryBegin = SF_MAX_UPINT;
    UPInt tryEnd = SF_MAX_UPINT;

    // Try block related stuff.
    {
        const MethodBodyInfo::Exception& e = GetMethodBodyInfo().exception;
        if (tryInd < e.GetSize())
        {
            const MethodBodyInfo::ExceptionInfo& ei = e.Get(tryInd++);
            tryBegin = ei.GetFrom();
            tryEnd = ei.GetTo();
        }
    }
    */

#ifdef USE_DEBUGGER
    FlashUI& ui = GetUI();
#endif

    while (bcp < code_end)
    {
        // Check for next block
        {
            const TR::Block* prevBlock = pBlock;
            TR::Block* nextBlock = GetBlock(bcp);

            if (nextBlock && nextBlock->GetFrom() == bcp)
            {
                pBlock = nextBlock;
                pState = &GetState(*pBlock);

                if (pBlock->IsInitializedBlock())
                    // Do we need to merge all the time?
                    MergeBlock(*pBlock, *prevBlock);
                else
                    InitializeBlock(*pBlock, *prevBlock);
            }
        }


        // Handle dead blocks
        {
            SF_ASSERT(pBlock);
            while (pBlock->IsDeadBlock() && bcp < code_end)
            {
                // This call is mandatory to maintain long_offset consistency.
                RegisterOpCode(bcp);

                const UInt8 byte_code = static_cast<UInt8>(Read8(code, bcp));
                const Code::OpCode opcode = static_cast<Code::OpCode>(byte_code);

#ifdef USE_DEBUGGER
                if (ui.NeedToCheckOpCode())
                {
                    MakeOffset(ui, GetPrintOffset());
                    ui.Output(FlashUI::Output_Action, "Skip opcode below.\n");
                    ui.OnOpCode(bcode, curr_offset, opcode);
                }
#endif
                switch (opcode)
                {
                case Code::op_ifnlt:
                case Code::op_ifnle:
                case Code::op_ifngt:
                case Code::op_ifnge:
                case Code::op_ifeq:
                case Code::op_ifge:
                case Code::op_ifgt:
                case Code::op_ifle:
                case Code::op_iflt:
                case Code::op_ifne:
                case Code::op_ifstricteq:
                case Code::op_ifstrictne:
                case Code::op_iftrue:
                case Code::op_iffalse:
                case Code::op_jump:
                    ReadS24(code, bcp);
                    break;
                case Code::op_lookupswitch:
                    {
                        ReadS24(code, bcp);
                        int case_count = ReadU30(code, bcp);

                        for(int i = 0; i <= case_count; ++i)
                            ReadS24(code, bcp);
                    }
                    break;
                case Code::op_debug:
                    Read8(code, bcp);
                    ReadU30(code, bcp);
                    Read8(code, bcp);
                    ReadU30(code, bcp);
                    break;
                default:
                    switch (Code::GetOpCodeInfo(opcode).operandCount)
                    {
                    case 2:
                        ReadU30(code, bcp);
                        // No break on purpose.
                    case 1:
                        ReadU30(code, bcp);
                        break;
                    default:
                        break;
                    }
                }

                // Check for next block
                {
                    const TR::Block* prevBlock = pBlock;
                    TR::Block* nextBlock = GetBlock(bcp);

                    if (nextBlock && nextBlock->GetFrom() == bcp)
                    {
                        pBlock = nextBlock;
                        pState = &GetState(*pBlock);

                        if (pBlock->IsInitializedBlock())
                            // Do we need to merge all the time?
                            MergeBlock(*pBlock, *prevBlock);
                        else
                            InitializeBlock(*pBlock, *prevBlock);
                    }
                }
            }

            if (bcp >= code_end)
                break;
        }

        // Try block related stuff.
        // Should be evaluated before an opcode.
        /* Do not handle this for the time being.
        if (tryBegin != SF_MAX_UPINT && bcp > tryEnd)
        {
            const MethodBodyInfo::Exception& e = GetMethodBodyInfo().exception;
            if (tryInd < e.GetSize())
            {
                // Get a new try block.

                const MethodBodyInfo::ExceptionInfo& ei = e.Get(tryInd++);
                tryBegin = ei.GetFrom();
                tryEnd = ei.GetTo();
            }
            else
            {
                // No more try blocks.
                tryBegin = SF_MAX_UPINT;
                tryEnd = SF_MAX_UPINT;
            }
        }

        const bool insideOfTry = (bcp >= tryBegin && bcp <= tryEnd);
        */

        // This call is mandatory to maintain long_offset consistency.
        RegisterOpCode(bcp);

        const UInt8 byte_code = static_cast<UInt8>(Read8(code, bcp));
        const Code::OpCode opcode = static_cast<Code::OpCode>(byte_code);

        // Try to substitute with a strictly typed opcode ...
        // Opcode can be replaced ...
        if (SubstituteOpCode(bcp, *pState, byte_code))
            continue;
        else
            wcode.PushBack(byte_code);


        // Fix offsets and remove useless opcodes ...
        /* Works not correctly. Eats next op.
        if (oi.rt == Code::rt_int && oi.operandCount <= 0)
        {
            SkipNextOpcode(Code::op_convert_i);
        } else if (oi.rt == Code::rt_boolean && oi.operandCount <= 0)
        {
            SkipNextOpcode(Code::op_convert_b);
        } else if (oi.rt == Code::rt_number && oi.operandCount <= 0)
        {
            SkipNextOpcode(Code::op_convert_d);
        } else
        */
        {
#ifdef USE_DEBUGGER
            if (ui.NeedToCheckOpCode() && ui.OnOpCode(bcode, curr_offset, opcode))
            {
                // That should never happen.
                SF_ASSERT(false);
            }
#endif

            // We do not need to handle new wcode here ...
            switch (opcode)
            {
            case Code::op_throw:
                /* DO NOT delete this code.
                // It looks like it is not enough to put logic below only in op_throw.
                {
                    Code::OpCode noc = GetNextOpCode();
                    bool deadBlock = (noc != Code::op_label && noc != Code::op_jump);

                    AddBlock(bcp, deadBlock); // Block after throw. Possible dead block.
                }
                */

                AddBlock(*pState, bcp, true); // Block after throw. Possible dead block.

                // pState can be invalidated in AddBlock().
                pState = &GetState(*pBlock);

                pState->exec_throw();
                break;
            case Code::op_getsuper:
                pState->exec_getsuper(ReadU30(code, bcp));
                break;
            case Code::op_setsuper:
                pState->exec_setsuper(ReadU30(code, bcp));
                break;
            case Code::op_dxns:
                pState->exec_dxns(ReadU30(code, bcp));
                break;
            case Code::op_dxnslate:
                pState->exec_dxnslate();
                break;
            case Code::op_kill:
                pState->exec_kill(ReadU30(code, bcp));
                break;
            case Code::op_label:
                AddBlock(*pState, curr_offset); // Block starts from label.

                // pState can be invalidated in AddBlock().
                pState = &GetState(*pBlock);

                pState->exec_label();
                break;
            case Code::op_ifnlt:
            case Code::op_ifnle:
            case Code::op_ifngt:
            case Code::op_ifnge:
            case Code::op_ifeq:
            case Code::op_ifge:
            case Code::op_ifgt:
            case Code::op_ifle:
            case Code::op_iflt:
            case Code::op_ifne:
            case Code::op_ifstricteq:
            case Code::op_ifstrictne:
                {
                    // Relative
                    const int offset = ReadS24(code, bcp);

                    pState->PopOp();
                    pState->PopOp();
                    // Relative
                    StoreOffset(bcp, *pState, offset);

                    // pState can be invalidated in StoreOffset().
                    pState = &GetState(*pBlock);
                }
                break;
            case Code::op_iftrue:
            case Code::op_iffalse:
                {
                    // Relative
                    const int offset = ReadS24(code, bcp);

                    pState->PopOp();
                    StoreOffset(bcp, *pState, offset);

                    // pState can be invalidated in StoreOffset().
                    pState = &GetState(*pBlock);
                }
                break;
            case Code::op_jump:
                {
                    const int offset = ReadS24(code, bcp);

                    // No manipulation with stack.
                    StoreOffset(bcp, *pState, offset);

                    // pState can be invalidated in StoreOffset().
                    pState = &GetState(*pBlock);

                    AddBlock(*pState, bcp, true); // Code immediately following the jump. Possible dead block.

                    // pState can be invalidated in AddBlock().
                    pState = &GetState(*pBlock);

                    //bcp += offset;
                }
                break;
            case Code::op_lookupswitch:
                {
                    pState->exec_lookupswitch();

                    Abc::TCodeOffset base_location = curr_offset;
                    int default_offset = ReadS24(code, bcp);
                    StoreOffset(bcp, *pState, static_cast<SInt32>(base_location + default_offset - bcp), 1);

                    // pState can be invalidated in StoreOffset().
                    pState = &GetState(*pBlock);

                    int case_count = ReadU30(code, bcp);
                    wcode.PushBack(case_count);

                    for(int i = 0; i <= case_count; ++i)
                    {
                        const int offset = ReadS24(code, bcp);

                        StoreOffset(bcp, *pState, static_cast<SInt32>(base_location + offset - bcp), i + 3);

                        // pState can be invalidated in StoreOffset().
                        pState = &GetState(*pBlock);
                    }
                }
                AddBlock(*pState, bcp, true); // Code immediately following the lookupswitch. Possible dead block.

                // pState can be invalidated in AddBlock().
                pState = &GetState(*pBlock);

                break;
            case Code::op_pushwith:
                pState->exec_pushwith();
                break;
            case Code::op_popscope:
                pState->exec_popscope();
                break;
            case Code::op_nextname:
                pState->exec_nextname();
                break;
            case Code::op_hasnext:
                pState->exec_hasnext();
                break;
            case Code::op_pushnull:
                pState->exec_pushnull();
                break;
            case Code::op_pushundefined:
                pState->exec_pushundefined();
                break;
            case Code::op_nextvalue:
                pState->exec_nextvalue();
                break;
            case Code::op_pushbyte:
                pState->exec_pushbyte(Read8(code, bcp));
                break;
            case Code::op_pushshort:
                pState->exec_pushshort(ReadU30(code, bcp));
                break;
            case Code::op_pushtrue:
            case Code::op_pushfalse:
                pState->exec_push_bool();
                break;
            case Code::op_pushnan:
                pState->exec_pushnan();
                break;
            case Code::op_pop:
                pState->exec_pop();
                break;
            case Code::op_dup:
                pState->exec_dup();
                break;
            case Code::op_swap:
                pState->exec_swap();
                break;
            case Code::op_pushstring:
                pState->exec_pushstring(ReadU30(code, bcp));
                break;
            case Code::op_pushint:
                pState->exec_pushint(ReadU30(code, bcp));
                break;
            case Code::op_pushuint:
                pState->exec_pushuint(ReadU30(code, bcp));
                break;
            case Code::op_pushdouble:
                pState->exec_pushdouble(ReadU30(code, bcp));
                break;
            case Code::op_pushscope:
                pState->exec_pushscope();
                break;
            case Code::op_pushnamespace:
                pState->exec_pushnamespace(ReadU30(code, bcp));
                // ??? Exceptions ?
                break;
            case Code::op_hasnext2:
                {
                    UInt32 object_reg = ReadU30(code, bcp);
                    UInt32 index_reg = ReadU30(code, bcp);
                    pState->exec_hasnext2(object_reg, index_reg);
                }
                break;
            case Code::op_li8:
                pState->exec_li8();
                break;
            case Code::op_li16:
                pState->exec_li16();
                break;
            case Code::op_li32:
                pState->exec_li32();
                break;
            case Code::op_lf32:
                pState->exec_lf32();
                break;
            case Code::op_lf64:
                pState->exec_lf64();
                break;
            case Code::op_si8:
                pState->exec_si8();
                break;
            case Code::op_si16:
                pState->exec_si16();
                break;
            case Code::op_si32:
                pState->exec_si32();
                break;
            case Code::op_sf32:
                pState->exec_sf32();
                break;
            case Code::op_sf64:
                pState->exec_sf64();
                break;
            case Code::op_newfunction:
                pState->exec_newfunction(ReadU30(code, bcp));
                break;
            case Code::op_call:
                pState->exec_call(ReadU30(code, bcp));
                break;
            case Code::op_construct:
                pState->exec_construct(ReadU30(code, bcp));
                break;
            case Code::op_callmethod:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callmethod(_1, _2);
                }
                break;
            case Code::op_callstatic:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callstatic(_1, _2);
                }
                break;
            case Code::op_callsuper:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callsuper(_1, _2);
                }
                break;
            case Code::op_callproperty:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callproperty(_1, _2);
                }
                break;
            case Code::op_returnvoid:
            case Code::op_returnvalue:
                // Possible dead block.
                AddBlock(*pState, bcp, true);

                // pState can be invalidated in AddBlock().
                pState = &GetState(*pBlock);

                // Exceptions are possible, but we can ignore them here.

                // Stop tracing.
                //goto call_stack_label;
                break;
            case Code::op_constructsuper:
                pState->exec_constructsuper(ReadU30(code, bcp));
                break;
            case Code::op_constructprop:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_constructprop(_1, _2);
                }
                break;
            case Code::op_callsuperid:
                pState->exec_callsuperid();
                // Exceptions ?????
                break;
            case Code::op_callproplex:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callproplex(_1, _2);
                }
                break;
            case Code::op_callinterface:
                pState->exec_callinterface();
                // Exceptions ????
                break;
            case Code::op_callsupervoid:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callsupervoid(_1, _2);
                }
                break;
            case Code::op_callpropvoid:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_callpropvoid(_1, _2);
                }
                break;
            case Code::op_sxi1:
                pState->exec_sxi1();
                break;
            case Code::op_sxi8:
                pState->exec_sxi8();
                break;
            case Code::op_sxi16:
                pState->exec_sxi16();
                break;
            case Code::op_applytype:
                pState->exec_applytype(ReadU30(code, bcp));
                break;
            case Code::op_newobject:
                pState->exec_newobject(ReadU30(code, bcp));
                break;
            case Code::op_newarray:
                pState->exec_newarray(ReadU30(code, bcp));
                break;
            case Code::op_newactivation:
                pState->exec_newactivation();
                break;
            case Code::op_newclass:
                pState->exec_newclass(ReadU30(code, bcp));
                break;
            case Code::op_getdescendants:
                pState->exec_getdescendants(ReadU30(code, bcp));
                break;
            case Code::op_newcatch:
                pState->exec_newcatch(ReadU30(code, bcp));
                break;
            case Code::op_findpropglobalstrict:
                pState->exec_findpropglobalstrict(ReadU30(code, bcp));
                break;
            case Code::op_findpropglobal:
                pState->exec_findpropglobal(ReadU30(code, bcp));
                break;
            case Code::op_findpropstrict:
                pState->exec_findpropstrict(ReadU30(code, bcp));
                break;
            case Code::op_findproperty:
                pState->exec_findproperty(ReadU30(code, bcp));
                break;
            case Code::op_finddef:
                pState->exec_finddef(ReadU30(code, bcp));
                break;
            case Code::op_getlex:
                pState->exec_getlex(ReadU30(code, bcp));
                break;
            case Code::op_setproperty:
                pState->exec_setproperty(ReadU30(code, bcp));
                break;
            case Code::op_setlocal:
                pState->exec_setlocal(ReadU30(code, bcp));
                break;
            case Code::op_getglobalscope:
                pState->exec_getglobalscope();
                break;
            case Code::op_getscopeobject:
                pState->exec_getscopeobject(ReadU30(code, bcp));
                break;
            case Code::op_getproperty:
                pState->exec_getproperty(ReadU30(code, bcp));
                break;
            case Code::op_getouterscope:
                pState->exec_getouterscope(ReadU30(code, bcp));
                break;
            case Code::op_initproperty:
                pState->exec_initproperty(ReadU30(code, bcp));
                break;
            case Code::op_deleteproperty:
                pState->exec_deleteproperty(ReadU30(code, bcp));
                break;
            case Code::op_getslot:
                pState->exec_getslot(ReadU30(code, bcp));
                break;
            case Code::op_setslot:
                pState->exec_setslot(ReadU30(code, bcp));
                break;
            case Code::op_getglobalslot:
                pState->exec_getglobalslot(ReadU30(code, bcp));
                break;
            case Code::op_setglobalslot:
                pState->exec_setglobalslot(ReadU30(code, bcp));
                break;
            case Code::op_convert_s:
                pState->exec_convert_s();
                break;
            case Code::op_esc_xelem:
                pState->exec_esc_xelem();
                break;
            case Code::op_esc_xattr:
                pState->exec_esc_xattr();
                break;
            case Code::op_convert_i:
                pState->exec_convert_i();
                break;
            case Code::op_convert_u:
                pState->exec_convert_u();
                break;
            case Code::op_convert_d:
                pState->exec_convert_d();
                break;
            case Code::op_convert_b:
                pState->exec_convert_b();
                break;
            case Code::op_convert_o:
                pState->exec_convert_o();
                break;
            case Code::op_checkfilter:
                pState->exec_checkfilter();
                break;
            case Code::op_coerce:
                pState->exec_coerce(ReadU30(code, bcp));
                break;
            case Code::op_coerce_b:
                pState->exec_coerce_b();
                break;
            case Code::op_coerce_a:
                pState->exec_coerce_a();
                break;
            case Code::op_coerce_i:
                pState->exec_coerce_i();
                break;
            case Code::op_coerce_d:
                pState->exec_coerce_d();
                break;
            case Code::op_coerce_s:
                pState->exec_coerce_s();
                break;
            case Code::op_astype:
                pState->exec_astype(ReadU30(code, bcp));
                break;
            case Code::op_astypelate:
                pState->exec_astypelate();
                break;
            case Code::op_coerce_u:
                pState->exec_coerce_u();
                break;
            case Code::op_coerce_o:
                pState->exec_coerce_o();
                break;
            case Code::op_negate:
                pState->exec_negate();
                break;
            case Code::op_increment:
                pState->exec_increment();
                break;
            case Code::op_inclocal:
                pState->exec_inclocal(ReadU30(code, bcp));
                break;
            case Code::op_decrement:
                pState->exec_decrement();
                break;
            case Code::op_declocal:
                pState->exec_declocal(ReadU30(code, bcp));
                break;
            case Code::op_typeof:
                pState->exec_typeof();
                break;
            case Code::op_not:
                pState->exec_not();
                break;
            case Code::op_bitnot:
                pState->exec_bitnot();
                break;
            case Code::op_concat:
                pState->exec_concat();
                break;
            case Code::op_add_d:
                pState->exec_add_d();
                break;
            case Code::op_add:
                pState->exec_add();
                break;
            case Code::op_subtract:
                pState->exec_subtract();
                break;
            case Code::op_multiply:
                pState->exec_multiply();
                break;
            case Code::op_divide:
                pState->exec_divide();
                break;
            case Code::op_modulo:
                pState->exec_modulo();
                break;
            case Code::op_lshift:
                pState->exec_lshift();
                break;
            case Code::op_rshift:
                pState->exec_rshift();
                break;
            case Code::op_urshift:
                pState->exec_urshift();
                break;
            case Code::op_bitand:
                pState->exec_bitand();
                break;
            case Code::op_bitor:
                pState->exec_bitor();
                break;
            case Code::op_bitxor:
                pState->exec_bitxor();
                break;
            case Code::op_equals:
            case Code::op_strictequals:
            case Code::op_lessthan:
            case Code::op_lessequals:
            case Code::op_greaterthan:
            case Code::op_greaterequals:
            case Code::op_istypelate:
                pState->PopOp(); // value2
                pState->ConvertOpTo(GetBooleanType());
                break;
            case Code::op_instanceof:
                pState->exec_instanceof();
                break;
            case Code::op_istype:
                pState->exec_istype(ReadU30(code, bcp));
                break;
            case Code::op_in:
                pState->exec_in();
                break;
            case Code::op_increment_i:
            case Code::op_decrement_i:
            case Code::op_negate_i:
            case Code::op_add_i:
            case Code::op_subtract_i:
            case Code::op_multiply_i:
                pState->ConvertOpTo(GetSIntType());
                break;
            case Code::op_inclocal_i:
                pState->exec_inclocal_i(ReadU30(code, bcp));
                break;
            case Code::op_declocal_i:
                pState->exec_declocal_i(ReadU30(code, bcp));
                break;
            case Code::op_getlocal:
                pState->exec_getlocal(ReadU30(code, bcp));
                break;
            case Code::op_getlocal0:
                pState->exec_getlocal0();
                break;
            case Code::op_getlocal1:
                pState->exec_getlocal1();
                break;
            case Code::op_getlocal2:
                pState->exec_getlocal2();
                break;
            case Code::op_getlocal3:
                pState->exec_getlocal3();
                break;
            case Code::op_setlocal0:
                pState->exec_setlocal0();
                break;
            case Code::op_setlocal1:
                pState->exec_setlocal1();
                break;
            case Code::op_setlocal2:
                pState->exec_setlocal2();
                break;
            case Code::op_setlocal3:
                pState->exec_setlocal3();
                break;
            case Code::op_abs_jump:
                {
                    UInt32 _1 = ReadU30(code, bcp);
                    UInt32 _2 = ReadU30(code, bcp);
                    pState->exec_abs_jump(_1, _2);
                }

                AddBlock(*pState, bcp, true); // Block, which begins after jump. Possible dead block.

                // pState can be invalidated in AddBlock().
                pState = &GetState(*pBlock);

                break;
            case Code::op_debug:
                wcode.PushBack(Read8(code, bcp));
                wcode.PushBack(ReadU30(code, bcp));
                wcode.PushBack(Read8(code, bcp));
                wcode.PushBack(ReadU30(code, bcp));
                break;
            case Code::op_debugline:
            case Code::op_debugfile:
                wcode.PushBack(ReadU30(code, bcp));
                break;
            case Code::op_0xF2:
                ReadU30(code, bcp);
                // ???
                break;
            case Code::op_timestamp:
                pState->exec_timestamp();
                break;
            default:
                // This part should be gone eventually.
                switch (Code::GetOpCodeInfo(opcode).operandCount)
                {
                case 1:
                    wcode.PushBack(ReadU30(code, bcp));
                    break;
                case 2:
                    wcode.PushBack(ReadU30(code, bcp));
                    wcode.PushBack(ReadU30(code, bcp));
                    break;
                default:
                    break;
                }
            }
        }

        // Exception related.
        /* Let's skip it for the time being.
        if (oi.canThrow && insideOfTry)
        {
            AddBlock(bcp);

            // pState can be invalidated in AddBlock().
            pState = &GetState(*pBlock);
        }
        */

        if (GetVM().IsException())
            return;
    }
}

TR::Block* Tracer::GetBlock(UPInt offset)
{
    TR::Block* block = Blocks.GetLast();

    // Skip blocks with bigger offset.
    while (block != NULL && offset < block->GetFrom())
    {
        block = Blocks.GetPrev(block);
    }

    return block;
}

void Tracer::StoreOffset(Abc::TCodeOffset bcp, const TR::State& st, SInt32 offset, int base)
{
    using namespace Abc;

    if (offset < 0)
    {
        /* DO NOT delete this code.
        // This check proves that each back reference is marked with the Code::op_label.
        {
            TCodeOffset ccp = cp;
            const Code::OpCode offset_opcode = static_cast<Code::OpCode>(Read8(code + offset, ccp));
            SF_UNUSED(offset_opcode);
            SF_ASSERT(offset_opcode == Code::op_label);
        }
        */

        // We already know a new address. We just need to get it and store it.
        TCodeOffset new_offset = long_offset[bcp + offset] - wcode.GetSize() + base;
        //SF_ASSERT((wcode.GetSize() + new_offset) >= 0);
        wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(new_offset));
    } else
    {
        AddBlock(st, bcp + offset);

        // A new address is not known at this time because it is in front of current code pointer.
        // Translate offset to an absolute address and store it instead of a relative address.
        wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(bcp + offset));
        // Store a position in wcode where we stored this absolute address, which we will need 
        // to recalculate later.
        recalculate_pos.PushBack(Recalculate(wcode.GetSize() - 1, base));
    }
}

void Tracer::SkipOpCode(Abc::TCodeOffset& opcode_cp, Abc::TCodeOffset new_cp)
{
    // Store pos in case it is referenced.
    opcode_pos.PushBack(opcode_cp);
    long_offset[opcode_cp] = wcode.GetSize();
    // This assert triggers in several tests.
    //SF_ASSERT(long_offset[opcode_cp] < 100000);

    // Change CP.
    opcode_cp = new_cp;
}

void Tracer::RegisterOpCode(Abc::TCodeOffset opcode_cp)
{
    curr_offset = opcode_cp;
    opcode_pos.PushBack(opcode_cp);
    long_offset[opcode_cp] = wcode.GetSize();
    // This assert triggers in several tests.
    //SF_ASSERT(long_offset[opcode_cp] < 100000);
}

void Tracer::SkipNextOpcode(Abc::TCodeOffset& bcp, Abc::Code::OpCode opcode)
{
    using namespace Abc;

    TCodeOffset ccp = bcp;
    const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(code, ccp));

    if (next_opcode == opcode)
        SkipOpCode(bcp, ccp);
}

void Tracer::SkipNextConvert(Abc::TCodeOffset& bcp, Abc::Code::ResultType type)
{
    using namespace Abc;

    TCodeOffset ccp = bcp;
    const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(code, ccp));
    bool skip = false;

    switch(next_opcode)
    {
    case Code::op_convert_i:
        skip = type == Code::rt_int;
        break;
    case Code::op_convert_u:
        skip = type == Code::rt_uint;
        break;
    case Code::op_convert_d:
        skip = type == Code::rt_number;
        break;
    case Code::op_convert_s:
        skip = type == Code::rt_string;
        break;
    case Code::op_convert_b:
        skip = type == Code::rt_boolean;
        break;
    case Code::op_convert_o:
        skip = type == Code::rt_object;
        break;
    default:
        break;
    }

    if (skip)
        SkipOpCode(bcp, ccp);
}

Abc::Code::OpCode Tracer::GetPrevOpCode(UPInt opcode_num_diff) const
{
    using namespace Abc;

    if (opcode_pos.GetSize() > opcode_num_diff)
    {
        TCodeOffset ccp = opcode_pos[opcode_pos.GetSize() - opcode_num_diff - 1];
        const Code::OpCode prev_opcode = static_cast<Code::OpCode>(Read8(code, ccp));

        return prev_opcode;
    }

    return Code::op_nop;
}

Abc::Code::OpCode Tracer::GetPrevOpCode2(UPInt offset) const
{
    using namespace Abc;

    UPInt i = Alg::LowerBound(opcode_pos, offset);
    TCodeOffset ccp = opcode_pos[i - 1];
    const Code::OpCode prev_opcode = static_cast<Code::OpCode>(Read8(code, ccp));

    return prev_opcode;
}

bool Tracer::MatchPrevRT(TR::State& st, Abc::Code::ResultType t1, UPInt opcode_offset) const
{
    using namespace Abc;

    const Code::OpCode opcode = GetPrevOpCode(opcode_offset);
    int reg_num = -1;

    switch(opcode)
    {
    case Code::op_getlocal:
        {
            TCodeOffset ccp = opcode_pos[opcode_pos.GetSize() - opcode_offset - 1] + 1;
            reg_num = ReadU30(code, ccp);
        }
        break;
    case Code::op_getlocal0:
        reg_num = 0;
        break;
    case Code::op_getlocal1:
        reg_num = 1;
        break;
    case Code::op_getlocal2:
        reg_num = 2;
        break;
    case Code::op_getlocal3:
        reg_num = 3;
        break;
    case Code::op_getslot:
        if (GetPrevOpCode(opcode_offset + 1) == Code::op_getglobalscope)
            return GetSlotType(GetNextU30(opcode_offset), GetHasTraits()) == t1; // !!! Can be a mistake.
        break;
    default:
        break;
    }

    if (reg_num >= 0)
        return AsResultType(st.GetRegister(AbsoluteIndex(reg_num))) == t1;

    return Code::GetOpCodeInfo(opcode).rt == t1;
}

bool Tracer::MatchPrevRT(TR::State& st, Abc::Code::ResultType t1, Abc::Code::ResultType t2, UPInt opcode_offset) const
{
    return MatchPrevRT(st, t2, opcode_offset + 0) && MatchPrevRT(st, t1, opcode_offset + 1);
}

static
void MergeValues(Value& to, const Value& from)
{
    //to = from;

    if (to.IsUndefined())
        to = from;

//     if (to.IsUndefined() && !from.IsUndefined())
//         to = from;
    // if from.IsUndefined() - do nothing.
}

static
void MergeLists(ValueArrayDH& to, const ValueArrayDH& from, bool check_size = true)
{
    if (check_size && to.GetSize() != from.GetSize())
        return;

    for (UPInt i = 0; i < from.GetSize(); ++i)
    {
        if (i >= to.GetSize())
            to.PushBack(from[i]);
        else
            MergeValues(to[i], from[i]);
    }
}

void Tracer::InitializeBlock(TR::Block& to, const TR::Block& from)
{
    SF_ASSERT(!to.IsInitializedBlock());

    const TR::State& from_st = GetState(from);
    TR::State& to_st = GetState(to);

    MergeLists(to_st.GetOpStack(), from_st.GetOpStack(), false);
    MergeLists(to_st.GetScopeStack(), from_st.GetScopeStack(), false);
    MergeLists(to_st.GetRegisters(), from_st.GetRegisters());

    to.SetInitialized();
}

void Tracer::MergeBlock(TR::Block& to, const TR::Block& from)
{
    SF_ASSERT(to.IsInitializedBlock());

    const TR::State& from_st = GetState(from);
    TR::State& to_st = GetState(to);

    MergeLists(to_st.GetOpStack(), from_st.GetOpStack());
    MergeLists(to_st.GetScopeStack(), from_st.GetScopeStack());
    MergeLists(to_st.GetRegisters(), from_st.GetRegisters());
}

int Tracer::GetNextU30(UPInt opcode_offset) const
{
    using namespace Abc;

    TCodeOffset ccp = opcode_pos[opcode_pos.GetSize() - opcode_offset - 1];

    return ReadU30(code, ++ccp);
}

Abc::Code::ResultType Tracer::GetSlotType(int ind, const Abc::HasTraits* ht) const
{
    using namespace Abc;

    if (ht)
    {
        for(UPInt i = 0; i < ht->GetTraitsCount(); ++i)
        {
            const TraitInfo& ti = ht->GetTraitInfo(GetAbcFile().GetTraits(), AbsoluteIndex(i));

            if (ti.IsSlotOrConst())
            {
                if (ti.GetSlotID() == ind)
                    return AsResultType(ti.GetTypeName(GetAbcFile()));
            }
        }
    }

    return Code::rt_unknown;
}

Abc::Code::ResultType Tracer::AsResultType(const Abc::Multiname& mn) const
{
    using namespace Abc;

    const String& name = mn.GetName(GetAbcFile().GetConstPool());

    if (name == "int")
        return Code::rt_int;
    else if (name == "uint")
        return Code::rt_uint;
    else if (name == "Number")
        return Code::rt_number;
    else if (name == "String")
        return Code::rt_string;
    else if (name == "Boolean")
        return Code::rt_boolean;
    else
        return Code::rt_object;

    //return Code::rt_unknown;
}

Abc::Code::ResultType Tracer::AsResultType (const Value& v) const
{
    using namespace Abc;

    if (v.IsNull())
        return Code::rt_object;

    if (v.IsUndefined())
        return Code::rt_undefined;

    const AS3::Traits* tr = NULL;

    if (v.GetKind() != Value::kInstanceTraits)
        return Code::rt_unknown;

    tr = &v.GetInstanceTraits();

    if (tr == &GetClassType())
        return Code::rt_any;

    if (tr == &GetBooleanType())
        return Code::rt_boolean;

    if (tr == &GetSIntType())
        return Code::rt_int;

    if (tr == &GetUIntType())
        return Code::rt_uint;

    if (tr == &GetNumberType())
        return Code::rt_number;

    if (tr == &GetStringType())
        return Code::rt_string;

    return Code::rt_object;
}

Value Tracer::AsValueType(Abc::Code::ResultType rt) const
{
    using namespace Abc;

    switch (rt)
    {
    case Code::rt_unknown:
    case Code::rt_none:
        break;
    case Code::rt_any:
        return GetClassType();
    case Code::rt_undefined:
        break;
    case Code::rt_boolean:
        return GetBooleanType();
    case Code::rt_int:
        return GetSIntType();
    case Code::rt_uint:
        return GetUIntType();
    case Code::rt_number:
        return GetNumberType();
    case Code::rt_string:
        return GetStringType();
    case Code::rt_object:
        return GetObjectType();
    case Code::rt_prop_type:
    case Code::rt_eval_type:
    default:
        break;
    }

    return Value::GetUndefined();
}

bool Tracer::SubstituteOpCode(Abc::TCodeOffset& bcp, TR::State& st, UInt8 byte_code)
{
    using namespace Abc;

    const Code::OpCode opcode = static_cast<Code::OpCode>(byte_code);

#ifdef USE_DEBUGGER
    FlashUI& ui = GetUI();;
#endif

    // Try to substitute with a strictly typed opcode ...

    switch(opcode)
    {
#if 0 // This code breaks several tests.
        // This part of optimization is more or less safe.
    case Code::op_ifnlt:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifnlt_i;
        break;
    case Code::op_ifnle:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifnle_i;
        break;
    case Code::op_ifngt:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifngt_i;
        break;
    case Code::op_ifnge:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifnge_i;
        break;
    case Code::op_iftrue:
        if (MatchPrevRT(st, Code::rt_boolean))
            byte_code = Code::op_iftrue_b;
        break;
    case Code::op_iffalse:
        if (MatchPrevRT(st, Code::rt_boolean))
            byte_code = Code::op_iffalse_b;
        break;
    case Code::op_ifeq:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifeq_i;
        break;
    case Code::op_ifne:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifne_i;
        break;
    case Code::op_iflt:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_iflt_i;
        break;
    case Code::op_ifle:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifle_i;
        break;
    case Code::op_ifgt:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifgt_i;
        break;
    case Code::op_ifge:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifge_i;
        break;
    case Code::op_ifstricteq:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifeq_i;
        break;
    case Code::op_ifstrictne:
        if (MatchPrevRT(st, Code::rt_int, Code::rt_int))
            byte_code = Code::op_ifne_i;
        break;
    case Code::op_increment_i:
        if (MatchPrevRT(st, Code::rt_int))
            byte_code = Code::op_increment_i2;
        break;
    case Code::op_decrement_i:
        if (MatchPrevRT(st, Code::rt_int))
            byte_code = Code::op_decrement_i2;
        break;
#endif
    case Code::op_getglobalscope:
        {
            TCodeOffset ccp = bcp;
#ifdef USE_DEBUGGER
            TCodeOffset next_offset = ccp;
#endif
            const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(code, ccp));
            int drop_num = 0;

            if (next_opcode == Code::op_getslot)
            {
#ifdef USE_DEBUGGER
                if (ui.NeedToCheckOpCode())
                {
                    ui.OnOpCode(bcode, curr_offset, opcode);
                }
#endif
                // Simulate prev opcode.
                st.exec_getglobalscope();
#ifdef USE_DEBUGGER
                if (ui.NeedToCheckOpCode())
                {
                    ui.OnOpCode(bcode, next_offset, next_opcode);
                }
#endif
                wcode.PushBack(Code::op_getglobalslot);
                // Simulate prev opcode.
                st.exec_getslot(ReadU30(code, ccp));

                // Skip prev combination ...
                SkipOpCode(bcp, ccp);

                return true;
            } else if (next_opcode == Code::op_swap)
            {
                opcode_pos.PushBack(ccp);
                ++drop_num;

#ifdef USE_DEBUGGER
                TCodeOffset next_offset2 = ccp;
#endif
                const Code::OpCode next_opcode2 = static_cast<Code::OpCode>(Read8(code, ccp));

                if (next_opcode2 == Code::op_setslot)
                {
                    wcode.PushBack(Code::op_setglobalslot);

#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, curr_offset, opcode);
                    }
#endif
                    // Simulate a set of opcodes below.
                    st.exec_getglobalscope();
#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, next_offset, next_opcode);
                    }
#endif
                    st.exec_swap();
#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, next_offset2, next_opcode2);
                    }
#endif
                    st.exec_setslot(ReadU30(code, ccp));


                    // Instead of simulating a simple one below.
                    //exec_setglobalslot(ReadU30(code, ccp));

                    // Skip prev combination ...
                    SkipOpCode(bcp, ccp);

                    return true;
                } else
                {
                    opcode_pos.PopBack(drop_num);
                }
            }
        }
        break;
    case Code::op_getlocal:
        {
            TCodeOffset ccp = bcp;
            const int reg_num = ReadU30(code, ccp);
#if 1
            Code::ResultType type = AsResultType(st.GetRegister(AbsoluteIndex(reg_num)));
#else
            Code::ResultType type = Code::rt_unknown;
#endif

#ifdef USE_DEBUGGER
            TCodeOffset next_offset = ccp;
#endif
            const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(code, ccp));
            bool check_setlocal = false;
            int drop_num = 0;

            switch(next_opcode)
            {
            case Code::op_increment:
            case Code::op_decrement:
            case Code::op_increment_i:
            case Code::op_decrement_i:
                check_setlocal = true;
                break;
            default:
                break;
            }

            if (check_setlocal)
            {
#ifdef USE_DEBUGGER
                if (ui.NeedToCheckOpCode())
                {
                    ui.OnOpCode(bcode, curr_offset, opcode);
                    ui.OnOpCode(bcode, next_offset, next_opcode);
                }
#endif

                opcode_pos.PushBack(ccp);
                ++drop_num;

#ifdef USE_DEBUGGER
                TCodeOffset current_offset = ccp;
#endif
                Code::OpCode current_opcode = static_cast<Code::OpCode>(Read8(code, ccp));

                if (current_opcode == Code::op_convert_i)
                {
                    type = Code::rt_int;

                    opcode_pos.PushBack(ccp);
                    ++drop_num;
#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, current_offset, current_opcode);
                    }
                    current_offset = ccp;
#endif
                    current_opcode = static_cast<Code::OpCode>(Read8(code, ccp));
                }

                if (current_opcode == Code::op_setlocal)
                {
                    if (reg_num != ReadU30(code, ccp))
                        break;

                    switch(next_opcode)
                    {
                    case Code::op_increment:
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, current_offset, current_opcode);
                        }
#endif
                        if (type == Code::rt_int)
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_inclocal_i2);
                        }
                        else
                        {
                            st.exec_convert_reg_d(reg_num);
                            wcode.PushBack(Code::op_inclocal);
                        }
                        wcode.PushBack(reg_num);

                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    case Code::op_decrement:
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, current_offset, current_opcode);
                        }
#endif
                        if (type == Code::rt_int)
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_declocal_i2);
                        }
                        else
                        {
                            st.exec_convert_reg_d(reg_num);
                            wcode.PushBack(Code::op_declocal);
                        }
                        wcode.PushBack(reg_num);

                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    case Code::op_increment_i:
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, current_offset, current_opcode);
                        }
#endif
                        if (type == Code::rt_int)
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_inclocal_i2);
                        }
                        else
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_inclocal_i);
                        }
                        wcode.PushBack(reg_num);

                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    case Code::op_decrement_i:
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, current_offset, current_opcode);
                        }
#endif
                        if (type == Code::rt_int)
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_declocal_i2);
                        }
                        else
                        {
                            st.exec_convert_reg_i(reg_num);
                            wcode.PushBack(Code::op_declocal_i);
                        }
                        wcode.PushBack(reg_num);

                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    default:
                        break;
                    }
                } else
                {
                    opcode_pos.PopBack(drop_num);
                }
            }
        }
        break;
#ifdef MULTINAME_OPTIMIZATION
    case Code::op_initproperty:
    case Code::op_setproperty:
    case Code::op_setsuper:
        {
            using namespace Abc;

            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);

#ifdef USE_DEBUGGER
            if (ui.NeedToCheckOpCode())
            {
                ui.OnOpCode(bcode, curr_offset, Code::op_nop);
            }
#endif

            TR::ReadValueMnObject args(GetFile(), st, mn_index);

            const bool super_tr = (opcode == Code::op_setsuper);
            const Traits* tr = GetValueTraits(args.ArgObject, super_tr);

            // EmitSetProperty() always returns true.
            if (EmitSetProperty(opcode, st, tr, args.ArgMN, mn_index))
            {
                // Skip prev combination ...
                SkipOpCode(bcp, ccp);
                return true;
            }
        }
        break;
    case Code::op_getproperty:
    case Code::op_getsuper:
        {
            using namespace Abc;

            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);

#ifdef USE_DEBUGGER
            if (ui.NeedToCheckOpCode())
            {
                ui.OnOpCode(bcode, curr_offset, Code::op_nop);
            }
#endif

            TR::ReadMnObject args(GetFile(), st, mn_index);

            const bool super_tr = (opcode == Code::op_getsuper);
            const Traits* tr = GetValueTraits(args.ArgObject, super_tr);

            // EmitGetProperty() always returns true.
            if (EmitGetProperty(opcode, st, tr, args.ArgMN, mn_index))
            {
                // Skip prev combination ...
                SkipOpCode(bcp, ccp);
                return true;
            }
        }
        break;
    case Code::op_findpropstrict:
        /*
        // Doesn't work yet.
        {
            // Try to eliminate findpropstrict + getproperty
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);
            const Code::OpCode next_opcode = static_cast<Code::OpCode>(Read8(code, ccp));

            if (next_opcode == Code::op_getproperty)
            {
                const int get_mn_index = ReadU30(code, ccp);
                
                if (mn_index == get_mn_index)
                {
                    // This is the same as getlex.
                    if (EmitFindProperty(st, mn_index, true DEBUG_ARG(true)))
                    {
                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    }
                }
            }
        }
        */
        // No break on purpose.
    case Code::op_findproperty:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);

            if (EmitFindProperty(st, mn_index))
            {
                // Skip prev combination ...
                SkipOpCode(bcp, ccp);
                return true;
            }
        }
        break;
    case Code::op_getlex:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);

            if (EmitFindProperty(st, mn_index, true))
            {
#ifdef USE_DEBUGGER
                if (ui.NeedToCheckOpCode())
                {
                    ui.OnOpCode(bcode, curr_offset, opcode);
                }
#endif

                // Skip prev combination ...
                SkipOpCode(bcp, ccp);

                return true;
            }
        }
        break;
    case Code::op_callproplex:
    case Code::op_callpropvoid:
    case Code::op_callproperty: // Optimization of op_callproperty breaks three tests.
    case Code::op_callsuper:
    case Code::op_callsupervoid:
        {
            // Call a property.

            TCodeOffset ccp = bcp;

            UInt32 mn_index = ReadU30(code, ccp);
            UInt32 arg_count = ReadU30(code, ccp);

#ifdef USE_DEBUGGER
            if (ui.NeedToCheckOpCode())
            {
                ui.OnOpCode(bcode, curr_offset, opcode);
            }
#endif

            // Read arguments ...
            TR::ReadArgsMnObject args(GetFile(), st, arg_count, mn_index);

            /*
            if (IsException())
                return false;
            */

            const bool super_tr = (opcode == Code::op_callsuper || opcode == Code::op_callsupervoid);
            const Traits* tr = GetValueTraits(args.ArgObject, super_tr);

            // EmitCall() always returns true.
            if (EmitCall(opcode, st, tr, args.ArgMN, mn_index, arg_count))
            {
                // Skip prev combination ...
                SkipOpCode(bcp, ccp);
                return true;
            }

        }
        break;
    case Code::op_coerce:
        {
            TCodeOffset ccp = bcp;
            const int mn_index = ReadU30(code, ccp);
            TR::ReadMnCtValue args(GetFile(), st, mn_index);
            const ClassTraits::Traits* to = GetVM().Resolve2ClassTraits(GetFile(), args.ArgMN);
            const Traits* value_tr = GetValueTraits(args.value);

            if (to != NULL && value_tr != NULL)
            {
                if (to == value_tr || to->GetInstanceTraits().IsParentOf(*value_tr))
                {

#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, curr_offset, opcode);
                    }
#endif

                    // TBD: It, probably, should be st.PushOp(Value(*GetValueTraits(args.value)));
                    // It will be a more precise type.
                    st.PushOp(Value(to->GetInstanceTraits()));

                    // Skip prev combination ...
                    SkipOpCode(bcp, ccp);

                    return true;
                }
            }

            // Restore back opstack state.
            st.PushOp(args.value);
            return false;
        }
        break;
    case Code::op_constructprop:
        {
            TCodeOffset ccp = bcp;
            UInt32 mn_index = ReadU30(code, ccp);
            UInt32 arg_count = ReadU30(code, ccp);

            // We can optimize case with arg_count == 0 
            // because we do not have any more values on stack.
            if (arg_count > 0)
                return false;

            // Read arguments ...
            TR::ReadMnObject args(GetFile(), st, mn_index);

            // Ignore run-time multinames.
            if (!args.ArgMN.IsRunTime())
            {
                // It might be a class name.
                if (args.ArgMN.IsQName())
                {
                    const ClassTraits::Traits* ctr = GetVM().Resolve2ClassTraits(args.ArgMN);

                    // Check that we are completely initialized.
                    if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
                    {
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, curr_offset, opcode);
                        }
#endif

                        InstanceTraits::Traits& itr = ctr->GetInstanceTraits();

                        // Emit getabsobject.
                        EmitGetAbsObject(Value(&itr.GetClass()), true);

                        // Emit *construct*.
                        st.PushCode(Code::op_construct);
                        st.PushCode(arg_count); // arg_count == 0

                        // Result type.
                        st.PushOp(itr);

                        // Skip prev combination ...
                        SkipOpCode(bcp, ccp);

                        return true;
                    }
                }

                // Try to replace constructprop with getabsslot + construct.
                {
                    const Traits* tr = GetValueTraits(args.ArgObject);

                    // For some reason this optimization doesn't work on global objects.
                    if (tr && !tr->IsGlobal())
                    {
                        UPInt slot_index = 0;
                        const SlotInfo* si = FindFixedSlot(GetVM(), *tr, args.ArgMN, slot_index, NULL);

                        if (si)
                        {
                            bool got_result = false;

                            if (si->IsCode())
                            {
                                if (si->GetValueInd().IsValid() && !tr->IsInterface())
                                {
                                    // Emit op_callmethod.
                                    st.PushCode(Code::op_callmethod);
                                    st.PushCode(static_cast<UInt32>(si->GetValueInd().Get()));
                                    st.PushCode(0);

                                    got_result = true;
                                }
                            }
                            else
                            {
                                EmitGetAbsSlot(slot_index);

                                got_result = true;
                            }

                            if (got_result)
                            {
#ifdef USE_DEBUGGER
                                if (ui.NeedToCheckOpCode())
                                {
                                    ui.OnOpCode(bcode, curr_offset, opcode);
                                }
#endif

                                // Emit *construct*.
                                st.PushCode(Code::op_construct);
                                st.PushCode(arg_count); // arg_count == 0

                                // Result type.
                                st.PushOp(st.GetPropertyType(*tr, *si));

                                // Skip prev combination ...
                                SkipOpCode(bcp, ccp);

                                return true;
                            }
                        }
                    }
                }
            }

            // No luck. Put an object back on stack.
            st.PushOp(args.ArgObject);

            return false;
        }
        break;
#endif
    default:
        break;
    }

    return false;
}

bool Tracer::EmitFindProperty(TR::State& st, const int mn_index, bool get_prop DEBUG_ARG(bool show_getprop))
{
    using namespace Abc;
    using namespace TR;

    const Abc::Multiname& mn = GetAbcFile().GetConstPool().GetMultiname(mn_index);
    AS3::Multiname as3_mn(GetFile(), mn);
    UPInt slot_ind = 0;
    UPInt scope_ind = 0;
    State::ScopeType stype;
#ifdef USE_DEBUGGER
    FlashUI& ui = GetUI();;
#endif

    PropRef prop = st.FindProp(as3_mn, stype, scope_ind, slot_ind);

    if (prop)
    {
        VM& vm = GetVM();
        SF_ASSERT(prop.GetSlotInfo());
        const SlotInfo& si = *prop.GetSlotInfo();
        const Traits* tr = GetValueTraits(prop.GetValue());
        SF_ASSERT(tr);

        switch (stype)
        {
        case State::stScopeStack:
        case State::stStoredScope:
            if (stype == State::stScopeStack)
            {
                // Let's replace it with op_getscopeobject.
                wcode.PushBack(Code::op_getscopeobject);
                wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(scope_ind));
            } else if (!EmitGetAbsObject(prop.GetValue()))
            {
                // Activation object is always different.
                // It would be a bad idea to store address to it.
                // Regular object can be garbage collected by the time we refer to it.
                // Let's replace it with op_getouterscope.
                EmitGetOuterScope(scope_ind);
            }

            if (get_prop)
            {
                bool emitted = false;

                // !!! ATTN: We MAY NOT skip DT_Set here.
                if (si.IsCode())
                {
                    if (si.GetValueInd().IsValid() && !tr->IsInterface())
                    {
                        if (si.GetBindingType() == SlotInfo::DT_Code)
                        {
                            EmitGetAbsSlot(slot_ind);

                            // Retrieve function type.
                            st.PushOp(st.GetFunctType(tr->GetVT().Get(si.GetValueInd())));
                        }
                        else
                        {
                            // Emit op_callmethod.
                            st.PushCode(Code::op_callmethod);
                            st.PushCode(static_cast<UInt32>(si.GetValueInd().Get()));
                            st.PushCode(0);

                            // Retrieve function return type. In case of SlotInfo::DT_Code we need to retrieve function type.
                            st.PushOp(Value(st.GetFunctReturnType(tr->GetVT().Get(si.GetValueInd()))));
                        }

                        emitted = true;
                    }
                }

                if (!emitted)
                    EmitGetAbsSlot(slot_ind);
            }

#ifdef USE_DEBUGGER
            if (ui.NeedToCheckOpCode())
            {
                ui.OnOpCode(bcode, curr_offset, Code::op_nop);
            }
#endif
            if (get_prop)
            {
                // Push value type.
                st.PushOp(st.GetPropertyType(*tr, si));

#ifdef USE_DEBUGGER
                if (show_getprop && ui.NeedToCheckOpCode())
                {
                    Abc::TCodeOffset ccp = curr_offset;

                    Read8(bcode.GetDataPtr(), ccp);
                    ReadU30(bcode.GetDataPtr(), ccp);
                    ui.OnOpCode(bcode, ccp, Code::op_nop);
                }
#endif
            }
            else
                // Push object, which has this slot.
                st.PushOp(prop.GetValue());

            return true;
        case State::stGlobalObject:
            // Check global objects. We are looking for classes.
            // We know about all registered class objects.
            if (get_prop)
            {
                const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(as3_mn);
                if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
                {
                    // This is just a class.
                    Value value = Value(&ctr->GetInstanceTraits().GetClass());

#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, curr_offset, Code::op_nop);
                    }
#endif

                    st.PushOp(value);

#ifdef USE_DEBUGGER
                    if (show_getprop && ui.NeedToCheckOpCode())
                    {
                        Abc::TCodeOffset ccp = curr_offset;

                        Read8(bcode.GetDataPtr(), ccp);
                        ReadU30(bcode.GetDataPtr(), ccp);
                        ui.OnOpCode(bcode, ccp, Code::op_nop);
                    }
#endif
                    // Emit getabsobject.
                    return EmitGetAbsObject(value);
                }
            }

            if (get_prop)
            {
                // It looks like there is no need to handle getters in global objects.
                if (!si.IsCode())
                {
                    Value value;

                    // Special treatment for global objects because classes are not initialized yet.
                    if (GetValueTraits(st.GetRegister(AbsoluteIndex(0)))->IsGlobal())
                    {
                        // Property was found on a global object. We can use this fact.

                        // Emit getabsobject.
                        EmitGetAbsObject(prop.GetValue());
                        // Get absolute slot on a global object.
                        EmitGetAbsSlot(slot_ind);
#ifdef USE_DEBUGGER
                        if (ui.NeedToCheckOpCode())
                        {
                            ui.OnOpCode(bcode, curr_offset, Code::op_nop);
                        }
#endif
                        st.PushOp(st.GetPropertyType(*GetValueTraits(prop.GetValue()), si));
#ifdef USE_DEBUGGER
                        if (show_getprop && ui.NeedToCheckOpCode())
                        {
                            Abc::TCodeOffset ccp = curr_offset;

                            Read8(bcode.GetDataPtr(), ccp);
                            ReadU30(bcode.GetDataPtr(), ccp);
                            ui.OnOpCode(bcode, ccp, Code::op_nop);
                        }
#endif

                        return true;
                    }
                    else if (prop.GetSlotValueUnsafe(GetVM(), value))
                    {
                        // We can get real value. Value is not going to change.
                        // (This is true for classes and constants)
                        const bool rc = EmitGetSlot(value, slot_ind);
                        if (rc)
                        {
#ifdef USE_DEBUGGER
                            if (ui.NeedToCheckOpCode())
                            {
                                ui.OnOpCode(bcode, curr_offset, Code::op_nop);
                            }
#endif
                            st.PushOp(value);
#ifdef USE_DEBUGGER
                            if (show_getprop && ui.NeedToCheckOpCode())
                            {
                                Abc::TCodeOffset ccp = curr_offset;

                                Read8(bcode.GetDataPtr(), ccp);
                                ReadU30(bcode.GetDataPtr(), ccp);
                                ui.OnOpCode(bcode, ccp, Code::op_nop);
                            }
#endif
                        }

                        return rc;
                    }
                }
            }
            else
            {
                const Value& value = prop.GetValue();

                const bool rc = EmitGetSlot(value, slot_ind);
                if (rc)
                {
#ifdef USE_DEBUGGER
                    if (ui.NeedToCheckOpCode())
                    {
                        ui.OnOpCode(bcode, curr_offset, Code::op_nop);
                    }
#endif
                    st.PushOp(value);
#ifdef USE_DEBUGGER
                    if (show_getprop && ui.NeedToCheckOpCode())
                    {
                        Abc::TCodeOffset ccp = curr_offset;

                        Read8(bcode.GetDataPtr(), ccp);
                        ReadU30(bcode.GetDataPtr(), ccp);
                        ui.OnOpCode(bcode, ccp, Code::op_nop);
                    }
#endif
                }

                return rc;
            }

            break;
        }
    }

    return false;
}

void Tracer::EmitGetAbsSlot(const UPInt index)
{
    using namespace Abc;

    wcode.PushBack(Code::op_getabsslot); // !!! Not just op_getslot.
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(gsind.Get()));
}

void Tracer::EmitSetAbsSlot(const UPInt index)
{
    using namespace Abc;

    wcode.PushBack(Code::op_setabsslot); // !!! Not just op_setslot.
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(gsind.Get()));
}

void Tracer::EmitInitAbsSlot(const UPInt index)
{
    using namespace Abc;

    wcode.PushBack(Code::op_initabsslot); // !!! Not just op_setslot.
    const GlobalSlotIndex gsind = GlobalSlotIndex(AbsoluteIndex(index));
    SF_ASSERT(gsind.IsValid());
    wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(gsind.Get()));
}

void Tracer::EmitGetValue(const Value& value, const UPInt index, bool objOnStack)
{
    using namespace Abc;

    const UPInt origCodeSize = wcode.GetSize();

    switch (value.GetKind())
    {
    case Value::kUndefined:
        if (objOnStack)
            wcode.PushBack(Code::op_pop);
        wcode.PushBack(Code::op_pushundefined);
        break;
    case Value::kBoolean:
        if (objOnStack)
            wcode.PushBack(Code::op_pop);
        wcode.PushBack(value.AsBool() ? Code::op_pushtrue : Code::op_pushfalse);
        break;
        /* TBD: pushint/pushuint
    case Value::kInt:
    case Value::kUInt:
        */
    case Value::kNumber:
        if (value.IsNaN())
        {
            if (objOnStack)
                wcode.PushBack(Code::op_pop);
            wcode.PushBack(Code::op_pushnan);
        }
        break;
        /* 
        // TBD: Strings are reference counted. Unless we have this string in 
        // a ConstPool there is no way to optimize access to this string.
        // As an alternative, we can keep a list of strings, used in a function metainfo.
    case Value::kString:
        break;
        */
        /*
        // Life time of objects and namespaces is not known.
        // TBD: We can keep a list of used objects in a function metainfo.
    case Value::kObject:
    case Value::kFunction:
    case Value::kNamespace:
        */
    case Value::kObject:
    case Value::kClass:
        EmitGetAbsObject(value, objOnStack);
        break;
    default:
        break;
    }

    if (origCodeSize == wcode.GetSize())
    {
        // We haven't added any new opcode.
        // Let's fall back to EmitGetSlot().
        EmitGetAbsSlot(index);
    }
}

bool Tracer::EmitGetProperty(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const int mn_index)
{
    using namespace Abc;

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime())
    {
        if (tr)
        {
            VM& vm = GetVM();
            UPInt slot_ind = 0;

            if (mn.GetName().IsTraits())
            {
                if (tr->IsArrayLike() && IsNumericType(mn.GetName()))
                {
                    const AS3::Multiname mn_getValue(vm.GetPublicNamespace(), vm.GetStringManager().CreateConstString("getValue"));
                    const SlotInfo* si = FindFixedSlot(vm, *tr, mn_getValue, slot_ind, NULL);
                    if (si && si->GetValueInd().IsValid())
                    {
                        // It looks like there is no reason to try to call callsupermethod here.

                        // Emit op_callmethod.
                        st.PushCode(Code::op_callmethod);
                        st.PushCode(static_cast<UInt32>(si->GetValueInd().Get()));
                        st.PushCode(1);

                        // Retrieve type of elements.
                        InstanceTraits::Traits* elem_tr = NULL;
                        if (tr == &GetVectorSIntType())
                            elem_tr = &st.GetSIntType();
                        else if (tr == &GetVectorUIntType())
                            elem_tr = &st.GetUIntType();
                        else if (tr == &GetVectorNumberType())
                            elem_tr = &st.GetNumberType();
                        else if (tr == &GetVectorStringType())
                            elem_tr = &st.GetStringType();
                        else if (tr->GetTraitsType() == Traits_Vector_object)
                        {
                            const InstanceTraits::Traits* itr = static_cast<const InstanceTraits::Traits*>(tr);

                            if (itr && itr->HasConstructorSetup())
                                elem_tr = &static_cast<const ClassTraits::Vector_object&>(itr->GetClass().GetClassTraits()).GetEnclosedClassTraits().GetInstanceTraits();
                            else
                                elem_tr = &st.GetObjectType();
                        }
                        else
                            elem_tr = &st.GetObjectType();

                        st.PushOp(*elem_tr);

                        return true;
                    }
                }

                // Fall back.
                {
                    st.PushOp(Value(GetObjectType()));

                    // Restore back old opcode.
                    st.PushCode(opcode);
                    st.PushCode(mn_index);

                    return true;
                }
            }

            if (tr->IsGlobal())
            {
                // In case this is just a class.
                // !!! This block breaks tests "ecma3/FunctionObjects\e15_3_4__1_rt.abc"
                // and "ecma3/FunctionObjects\e15_3_4_rt.abc"
                {
                    const ClassTraits::Traits* ctr = vm.GetRegisteredClassTraits(mn);
                    if (ctr && ctr->IsValid() && ctr->GetInstanceTraits().HasConstructorSetup())
                    {
                        // This is just a class.
                        Value value = Value(&ctr->GetInstanceTraits().GetClass());

                        st.PushOp(value);

                        // Emit getabsobject.
                        // Object is on stack. This is the difference from EmitFindProperty().
                        const bool result = EmitGetAbsObject(value, true);
                        if (result)
                            return true;
                    }
                }

#if 0
                // !!! It looks like we shouldn't optimize access to Global Object.
                // This block should generate an op_getslot opcode.
                // !!! It still brakes "as3\Definitions\Packages\NoPackageName.as"
                {
                    const SlotInfo* si = FindFixedSlot(vm, *tr, args.ArgMN, slot_ind, NULL);

                    if (si)
                    {
                        if (!si->IsCode())
                        {
                            // Put object back on stack.
                            st.PushOp(obj_value);

                            // Object is on stack. Let's keep it there.
                            EmitGetAbsSlot(slot_ind);

                            return true;

//                                 Value value;
// 
//                                 // We can get real value. Value is not going to change.
//                                 // (This is true for classes, global objects and constants)
//                                 if (si->GetSlotValueUnsafe(GetVM(), value, obj_value))
//                                 {
//                                     // Object is on stack. This is the difference from EmitFindProperty().
//                                     const bool rc = EmitGetSlot(value, slot_ind, true);
//                                     EmitGetAbsSlot(slot_ind);
// 
//                                     if (rc)
//                                     {
// #ifdef USE_DEBUGGER
//                                         if (ui.NeedToCheckOpCode())
//                                         {
//                                             ui.OnOpCode(bcode, curr_offset, Code::op_nop);
//                                         }
// #endif
//                                         st.PushOp(value);
//                                     }
// 
//                                     return rc;
//                                 }
                        }
                    }
                }
#endif

            }
            else
            {
                const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_ind, NULL);

                if (si)
                {
                    // !!! ATTN: We MAY NOT skip DT_Set here.
                    if (si->IsCode())
                    {
                        if (si->GetValueInd().IsValid() && !tr->IsInterface())
                        {
                            if (si->GetBindingType() == SlotInfo::DT_Code)
                            {
                                EmitGetAbsSlot(slot_ind);

                                // Retrieve function type.
                                st.PushOp(st.GetFunctType(tr->GetVT().Get(si->GetValueInd())));

                                return true;
                            }
                            else
                            {
                                if (opcode == Code::op_getsuper)
                                    // Emit op_callsupermethod.
                                    st.PushCode(Code::op_callsupermethod);
                                else
                                    // Emit op_callmethod.
                                    st.PushCode(Code::op_callmethod);

                                st.PushCode(static_cast<UInt32>(si->GetValueInd().Get()));
                                st.PushCode(0);

                                // Retrieve function return type. In case of SlotInfo::DT_Code we need to retrieve function type.
                                st.PushOp(Value(st.GetFunctReturnType(tr->GetVT().Get(si->GetValueInd()))));

                                return true;
                            }
                        }
                    }
                    else
                    {
                        st.PushOp(st.GetPropertyType(*tr, *si));

                        EmitGetAbsSlot(slot_ind);

                        return true;
                    }
                }
            }


            // Default implementation.
            {
                // The property with the name specified by the multiname will be resolved in object, and the
                // value of that property will be pushed onto the stack. If the property is unresolved,
                // undefined is pushed onto the stack.

                UPInt slot_ind = 0;
                const SlotInfo* si = FindFixedSlot(GetVM(), *tr, mn, slot_ind, NULL);
                if (si)
                {
                    st.PushOp(st.GetPropertyType(*tr, *si));

                    // Restore back old opcode.
                    st.PushCode(opcode);
                    st.PushCode(mn_index);

                    return true;
                }
            }
        }

    }

    st.PushOp(Value(GetObjectType()));

    // Restore back old opcode.
    st.PushCode(opcode);
    st.PushCode(mn_index);

    return true;
}

bool Tracer::EmitSetProperty(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const int mn_index)
{
    using namespace Abc;

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime())
    {
        if (tr && !tr->IsGlobal())
        {
            VM& vm = GetVM();
            UPInt slot_ind = 0;
            const SlotInfo* si = FindFixedSlot(vm, *tr, mn, slot_ind, NULL);

            if (si)
            {
                if (si->IsCode())
                {
                    if (si->GetValueInd().IsValid() && !tr->IsInterface())
                    {
                        const SlotInfo::DataType dt = si->GetBindingType();

                        if (dt == SlotInfo::DT_Set || dt == SlotInfo::DT_GetSet)
                        {
                            if (opcode == Code::op_setsuper)
                                // Emit op_callsupermethod.
                                st.PushCode(Code::op_callsupermethod);
                            else
                                // Emit op_callmethod.
                                st.PushCode(Code::op_callmethod);

                            st.PushCode(static_cast<UInt32>(si->GetValueInd().Get() + 1));
                            st.PushCode(1);

                            st.PushCode(Code::op_pop);

                            return true;
                        }
                    }
                }
                else
                {
                    if (opcode == Code::op_initproperty)
                        EmitInitAbsSlot(slot_ind);
                    else
                        EmitSetAbsSlot(slot_ind);

                    return true;
                }
            }
        }
    }

    // Restore back old opcode.
    st.PushCode(opcode);
    st.PushCode(mn_index);

    return true;
}

bool Tracer::EmitCall(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const UInt32 mn_index, const UInt32 arg_count)
{
    using namespace Abc;

    // Ignore run-time multinames for the time being.
    if (!mn.IsRunTime())
    {
        UPInt slot_index = 0;

        if (tr && !tr->IsInterface())
        {
            const SlotInfo* si = FindFixedSlot(GetVM(), *tr, mn, slot_index, NULL);

            if (si && si->IsCode())
            {
                if (si->GetValueInd().IsValid())
                {
                    switch (opcode)
                    {
                    case Code::op_callsuper:
                    case Code::op_callsupervoid:
                        // Emit op_callsupermethod.
                        st.PushCode(Code::op_callsupermethod);
                        break;
                    default:
                        // Emit op_callmethod.
                        st.PushCode(Code::op_callmethod);
                        break;
                    }

                    st.PushCode(static_cast<UInt32>(si->GetValueInd().Get()));
                    st.PushCode(arg_count);

                    if (opcode == Code::op_callpropvoid || opcode == Code::op_callsupervoid)
                        st.PushCode(Code::op_pop);
                    else
                        // Retrieve function type.
                        st.PushOp(Value(st.GetFunctReturnType(tr->GetVT().Get(si->GetValueInd()))));

                    return true;
                }
            }
        }
    }

    // Try to replace with a *convert* opcode.
    if (arg_count == 1 && mn.IsQName())
    {
        VM& vm = GetVM();
        const ClassTraits::Traits* ctr = vm.Resolve2ClassTraits(mn);
        const UPInt orig_size = wcode.GetSize();

        if (ctr)
        {
            if (ctr == &vm.GetClassTraitsInt())
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_convert_i);

                st.PushOp(GetSIntType());
            }
            else if (ctr == &vm.GetClassTraitsUInt())
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_convert_u);

                st.PushOp(GetUIntType());
            }
            else if (ctr == &vm.GetClassTraitsNumber())
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_convert_d);

                st.PushOp(GetNumberType());
            }
            else if (ctr == &vm.GetClassTraitsBoolean())
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_convert_b);

                st.PushOp(GetBooleanType());
            }
            else if (ctr == &vm.GetClassTraitsString())
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_convert_s);

                st.PushOp(GetStringType());
            }
            /* DO NOT delete
            // coerce causes problems in Test\AS3\WebSWF\box2d_21a_flash10\Examples\PhysTest.swf
            else
            {
                st.PushCode(Code::op_swap);
                st.PushCode(Code::op_pop);

                st.PushCode(Code::op_coerce);
                st.PushCode(mn_index);

                st.PushOp(ctr->GetInstanceTraits());
            }
            */

            if (orig_size != wcode.GetSize())
            {
                // New opcode was generated.
                if (opcode == Code::op_callpropvoid || opcode == Code::op_callsupervoid)
                {
                    st.PushCode(Code::op_pop); // result
                    // ??? Should we pop two times.
                    //st.PushCode(Code::op_pop); // function
                }

                return true;
            }
        }
    }

    // 
    {
        // There is nothing we can do. Emit original op_callproperty.
        st.PushCode(opcode);
        st.PushCode(mn_index);
        st.PushCode(arg_count);

        if (opcode != Code::op_callpropvoid || opcode != Code::op_callsupervoid)
            st.PushOp(Value(GetObjectType()));
    }

    return true;
}

bool Tracer::EmitGetAbsObject(const Value& value, bool objOnStack)
{
    using namespace Abc;

    bool result = false;
    if (value.IsClass() || GetVM().GetValueTraits(value).IsGlobal())
    {
        Value::ObjectTag tag = value.GetObjectTag();

        if (objOnStack)
            wcode.PushBack(Code::op_pop);

        wcode.PushBack(Code::op_getabsobject);
        wcode.PushBack(reinterpret_cast<unsigned long>(value.GetObject()) + tag);

        result = true;
    }

    return result;
}

bool Tracer::EmitGetSlot(const Value& value, const UPInt index, bool objOnStack)
{
    using namespace Abc;

    if (value.IsTraits())
    {
        EmitGetAbsSlot(index);

        return true;
    }
    else
    {
        switch (value.GetKind())
        {
        case Value::kUndefined:
            if (objOnStack)
                wcode.PushBack(Code::op_pop);
            wcode.PushBack(Code::op_pushundefined);
            return true;
        case Value::kBoolean:
            if (objOnStack)
                wcode.PushBack(Code::op_pop);
            wcode.PushBack(value.AsBool() ? Code::op_pushtrue : Code::op_pushfalse);
            return true;
            /* TBD: pushint/pushuint
        case Value::kInt:
        case Value::kUInt:
            */
        case Value::kNumber:
            if (value.IsNaN())
            {
                if (objOnStack)
                    wcode.PushBack(Code::op_pop);
                wcode.PushBack(Code::op_pushnan);
                return true;
            }
            break;
            /* TBD: We can introduce an ObjectTag for strings.
        case Value::kString:
            break;
            */
        /*
        // Life time of objects and namespaces is not known.
        // TBD: We can keep a list of used objects in a function metainfo.
        case Value::kObject:
        case Value::kFunction:
        case Value::kNamespace:
        */
        case Value::kObject:
        case Value::kClass:
            return EmitGetAbsObject(value, objOnStack);
        default:
            break;
        }
    }

    return false;
}

void Tracer::EmitGetOuterScope(const UPInt index)
{
    using namespace Abc;

    wcode.PushBack(Code::op_getouterscope);
    wcode.PushBack(static_cast<Abc::TOpCode::ValueType>(index));
}

void Tracer::Output(OutputMessageType type, const char* msg)
{
    GetFile().GetVM().GetUI().Output(type, msg);
}

bool Tracer::OnOpCode(const Abc::TOpCode& code, Abc::TCodeOffset offset, Abc::Code::OpCode opcode)
{
    SF_UNUSED3(code, offset, opcode);
    return false;
}

bool Tracer::OnOpCode(const ArrayLH_POD<UInt8>& code,
                      Abc::TCodeOffset offset, Abc::Code::OpCode opcode)
{
    SF_UNUSED3(code, offset, opcode);
#ifdef GFX_AS3_VERBOSE
    // Trace trace state and opcode.
    AS3::TraceVM(*this, "\t");
    // We trace original byte code.
    AS3::TraceOpCode(*this, bcode, offset);
#endif

    return false;
}

TR::Block* Tracer::AddBlock(const TR::State& st, UPInt from, bool dead, bool checkOpCode)
{
    using namespace Abc;

    if (from >= code_end)
        // No blocks after code_end.
        return NULL;

    TR::Block* block = GetBlock(from);
    SF_ASSERT(block);

    if (block && from == block->GetFrom())
    {
        // A block already exists.
        // The "dead" flag will be ignored here.

        return block;
    }

    // We need to create a new block.

    //
    //const bool not_initialized = dead;

    // Check current opcode.
    if (checkOpCode)
    {
        // !!! We can store prev_opcode and pass as an argument. That would save time.
        const Code::OpCode curr_opcode = GetCurrOpCode(from);

        switch (curr_opcode)
        {
        case Code::op_label:
        case Code::op_debug:
        case Code::op_debugline:
        case Code::op_debugfile:
            // If there is a label, then this block is not dead.
            dead = false;
            break;
        case Code::op_jump:
            {
                const Code::OpCode prev_opcode = GetPrevOpCode2(from);

                if (prev_opcode == Code::op_throw)
                    dead = false;
            }
            break;
        default:
            break;
        }
    }

    const UPInt si = States.GetSize();
    TR::State new_st(st);
    States.PushBack(new_st);
    // st can be invalidated at this point.
    TR::Block* newNode = SF_HEAP_NEW(GetHeap()) TR::Block(si, from, dead);
    block->InsertNodeAfter(newNode);

    //if (not_initialized)
    if (dead)
    {
        // So far we do not check opcode only in exception blocks.
        newNode->SetInitialized(!dead);
    }

    return newNode;
}

const Traits* Tracer::GetValueTraits(const Value& v, const bool super_tr) const
{
    const Traits* result = NULL;

    if (super_tr)
        result = GetOriginationTraits();

    if (result == NULL)
    {
        switch (v.GetKind())
        {
        case Value::kUndefined:
            break;
        case Value::kInstanceTraits:
            result = &v.GetInstanceTraits();
            break;
        case Value::kClassTraits:
            result = &v.GetClassTraits();
            break;
        default:
            result = &GetVM().GetValueTraits(v);
        }

        // A hack.
        if (result && result == &GetVM().GetClassTraitsClassClass())
            result = &GetVM().GetClassTraitsObject();
    }

    if (super_tr && result)
        result = result->GetParent();

    return result;
}

const InstanceTraits::Traits* Tracer::GetInstanceTraits(const Value& v) const
{
    const InstanceTraits::Traits* result = NULL;

    switch (v.GetKind())
    {
    case Value::kUndefined:
        break;
    case Value::kInstanceTraits:
        result = &v.GetInstanceTraits();
        break;
    case Value::kClassTraits:
        result = &v.GetClassTraits().GetInstanceTraits();
        break;
    default:
        result = &GetVM().GetInstanceTraits(v);
    }

    // A hack.
    if (result && result == &GetVM().GetClassTraitsClassClass().GetInstanceTraits())
        result = &GetVM().GetClassTraitsObject().GetInstanceTraits();

    return result;
}

Scaleform::GFx::AS3::Value Tracer::GetGlobalObject(const TR::State& st) const
{
    const ScopeStackType* sss = GetSavedScope();

    if (sss && !sss->IsEmpty())
        return sss->At(0);

    return st.GetRegister(AbsoluteIndex(0));
}

const ClassTraits::Traits& Tracer::GetSlotCTraits(const Traits& tr, SlotIndex ind)
{
    return const_cast<SlotInfo&>(tr.GetSlotInfo(AbsoluteIndex(ind, tr))).GetDataType(tr.GetVM());
}

bool Tracer::IsPrimitiveType(const Traits& tr) const
{
    // ??? kUndefined = 0, // = 0
    return  &tr == &GetBooleanType() ||
            &tr == &GetSIntType() ||
            &tr == &GetUIntType() ||
            &tr == &GetNumberType() ||
            &tr == &GetStringType();
}

bool Tracer::IsNumericType(const Value& v) const
{
    const Traits* tr = GetValueTraits(v);

    return  tr == &GetSIntType() ||
            tr == &GetUIntType() ||
            tr == &GetNumberType();
}

///////////////////////////////////////////////////////////////////////////////
namespace Abc {
    const TOpCode& MethodBodyInfo::GetOpCode(const CallFrame& cf) const
    {
        
        if (wcode.IsEmpty())
        {
            Tracer wc(cf.GetVM().GetMemoryHeap(), cf);
            wc.EmitCode();
        }

        return wcode;
    }
} // namespace Abc {


#endif

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

