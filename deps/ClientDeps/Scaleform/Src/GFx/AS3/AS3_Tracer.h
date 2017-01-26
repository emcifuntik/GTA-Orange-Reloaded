/**********************************************************************

Filename    :   AS3_Tracer.h
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

#ifndef INC_AS3_TRACER_H
#define INC_AS3_TRACER_H

#include "Kernel/SF_List.h"

#include "AS3_VM.h"
#include "AS3_FlashUI.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
#ifdef USE_WORDCODE
//#define DISABLE_TRACING

class Tracer;

namespace TR
{
    ///////////////////////////////////////////////////////////////////////////
    // Moveable.
    class Block : public NewOverrideBase<Mem_Stat>, public ListNode<Block>
    {
        friend class AS3::Tracer;

    public:
        Block(UPInt si, UPInt f, bool dead = false) 
            : IsCatch(false)
            , IsTry(false)
            , IsDead(dead)
            , IsInitialized(true)
            , From(f)
            , StateInd(si)
        {
        }

    public:
        //
        bool IsInitializedBlock() const
        {
            return IsInitialized;
        }

        //
        bool IsCatchBlock() const
        {
            return IsCatch;
        }
        void SetCatchBlock(bool flag = true)
        {
            IsCatch = flag;
        }

        //
        bool IsTryBlock() const
        {
            return IsTry;
        }
        void SetTryBlock(bool flag = true)
        {
            IsTry = flag;
        }

        //
        bool IsDeadBlock() const
        {
            return IsDead;
        }
        void SetDeadBlock(bool flag = true)
        {
            IsDead = flag;
        }

        //
        UPInt GetFrom() const
        {
            return From;
        }

        //
        UPInt GetStateInd() const
        {
            return StateInd;
        }

    private:
        void SetInitialized(bool flag = true)
        {
            IsInitialized = flag;
        }

    private:
        bool IsCatch;
        bool IsTry;
        bool IsDead;
        bool IsInitialized;
        UPInt From; // Inclusive
        UPInt StateInd;
    }; // class Block

    ///////////////////////////////////////////////////////////////////////////

    class StackReader;

    class State
    {
        friend class AS3::Tracer;
        friend class StackReader;

    public:
        State(const State& other);
        State(MemoryHeap* heap, Tracer& tr, Abc::TCodeOffset cp, const ValueArrayDH& r);
        State(Tracer& tr, Abc::TCodeOffset cp, const ValueArrayDH& r, const ValueArrayDH& o, const ValueArrayDH& s);

    public:
        enum ScopeType {stScopeStack, stStoredScope, stGlobalObject};

    public:
        const ValueArrayDH& GetRegisters() const
        {
            return Registers;
        }
        const ValueArrayDH& GetOpStack() const
        {
            return OpStack;
        }
        const ValueArrayDH& GetScopeStack() const
        {
            return ScopeStack;
        }

    public:
        ///
        const Value& GetOpValue(UPInt ind)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(ind);
            return Value::GetUndefined();
#else
            SF_ASSERT(ind < OpStack.GetSize());
            return OpStack[ind];
#endif
        }

        ///
        const Value& BackOpValue()
        {
#ifdef DISABLE_TRACING
            return Value::GetUndefined();
#else
            return OpStack.Back();
#endif
        }
        void SetBackOpValue(const Value& v)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(v);
#else
            OpStack.Back().Assign(v);
#endif
        }
        void SetBackOpValueUnsafe(const Value& v)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(v);
#else
            OpStack.Back().AssignUnsafe(v);
#endif
        }

        ///
        Value PopOpValue()
        {
#ifdef DISABLE_TRACING
            return Value();
#else
            return OpStack.Pop();
#endif
        }
        void PopOp(UPInt count = 1)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(count);
#else
            OpStack.PopBack(count);
#endif
        }
        void PushOp(const Value& v)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(v);
#else
            OpStack.PushBack(v);
#endif
        }
        void PushOp(InstanceTraits::Traits& tr, bool null = false)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED2(tr, null);
#else
            OpStack.PushBack(Value(tr, null));
#endif
        }
        void PushOp(ClassTraits::Traits& tr, bool null = false)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED2(tr, null);
#else
            OpStack.PushBack(Value(tr, null));
#endif
        }
        void ConvertOpTo(InstanceTraits::Traits& tr, bool null = false)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED2(tr, null);
#else
            OpStack.Back() = Value(tr, null);
#endif
        }
        void SwapOp()
        {
#ifdef DISABLE_TRACING
            ; // Nothing
#else
            // Optimized version ...
            SPInt num      = OpStack.GetSize();
            const Value _2 = OpStack[--num];

            OpStack[num] = OpStack[num - 1];
            OpStack[--num] = _2;
#endif
        }

        ///
        const Value& GetScopeValue(UPInt ind)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(ind);
            return Value::GetUndefined();
#else
            SF_ASSERT(ind < ScopeStack.GetSize());
            return ScopeStack[ind];
#endif
        }
        void PopScope(UPInt count = 1)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(count);
#else
            ScopeStack.PopBack(count);
#endif
        }
        void PushScope(const Value& v)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED(v);
#else
            ScopeStack.PushBack(v);
#endif
        }

        ///
        void SetRegister(AbsoluteIndex index, const Value& v)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED2(index, v);
#else
            Registers[index.Get()] = v;
#endif
        }
        const Value& GetRegister(AbsoluteIndex index) const
        {
#ifdef DISABLE_TRACING
            SF_UNUSED1(index);
            return Value::GetUndefined();
#else
            return Registers[index.Get()];
#endif
        }
        void ConvertRegisterTo(AbsoluteIndex index, InstanceTraits::Traits& tr, bool null = false)
        {
#ifdef DISABLE_TRACING
            SF_UNUSED3(index, tr, null);
#else
            Registers[index.Get()] = Value(tr, null);
#endif
        }

    public:
        const Tracer& GetTracer() const
        {
            return *pTracer;
        }
        Abc::TCodeOffset GetCP() const
        {
            return bcp;
        }

        ValueArrayDH& GetRegisters()
        {
            return Registers;
        }
        ValueArrayDH& GetOpStack()
        {
            return OpStack;
        }
        ValueArrayDH& GetScopeStack()
        {
            return ScopeStack;
        }

    public:

        void exec_throw() 
        { 
            // Get exception object from the stack ...
            //GetState().PopOp(); // Index
        }
        void exec_getsuper(UInt32 index);
        void exec_setsuper(UInt32 index);

        void exec_dxns(UInt32 index) 
        {
            PushCode(index);

            // Sets the default XML namespace.

            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);

            // index is a u30 that must be an index into the string constant pool. The string at index is used
            // as the uri for the default XML namespace for this method.

            // A VerifyError is thrown if dxns is used in a method that does not have the SETS_DXNS flag set.
        }
        void exec_dxnslate()
        {
            // Sets the default XML namespace with a value determined at runtime.

            PopOp();

            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);

            // The top value on the stack is popped, converted to a string, and that string is used as the uri
            // for the default XML namespace for this method.

            // A VerifyError is thrown if dxnslate is used in a method that does not have the SETS_DXNS flag set.
        }
        void exec_kill(UInt32 index);
        void exec_label() 
        { 
            // Do nothing 
        }

        void exec_lookupswitch() 
        {
            // Value on the stack must be of type integer ...
            PopOp(); // Index
        }

        void exec_pushwith()
        {
            // A TypeError is thrown if scope_obj is null or undefined.

            PushScope(PopOpValue());
            // Set up the *scope* flag.
            GetScopeStack().Back().SetWith();
        }
        void exec_popscope()
        {
            PopScope();
        }
        void exec_nextname();
        void exec_hasnext();
        void exec_pushnull()
        {
            PushOp(Value(GetNullType(), true));
        }
        void exec_pushundefined()
        {
            PushOp(Value(GetVoidType()));
        }
        void exec_nextvalue();

        void exec_pushbyte(SInt32 v);
        void exec_pushshort(SInt32 v);
        void exec_push_bool()
        {
            PushOp(Value(GetBooleanType()));
        }
        void exec_pushnan()
        {
            PushOp(Value(GetNumberType()));
        }
        void exec_pop()
        {
            PopOp();
        }
        void exec_dup() 
        {
            // This sequence of calls is necessary to prevent problems related
            // to Array reallocation.
            PushOp(Value::GetUndefined());
            const Value& v = GetOpValue(GetOpStack().GetSize() - 2);
            SetBackOpValueUnsafe(v);
        }
        void exec_swap() 
        {
            SwapOp();
        }
        void exec_pushstring(SInt32 v);
        void exec_pushint(SInt32 v);
        void exec_pushuint(SInt32 v);
        void exec_pushdouble(SInt32 v);
        void exec_pushscope();
        void exec_pushnamespace(UInt32 v) 
        {
            PushCode(v);
            PushOp(GetNamespaceType());
        }
        void exec_hasnext2(UInt32 object_reg, UInt32 index_reg);
        void exec_li8() 
        {
            // Load data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_li16() 
        {
            // Load data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_li32() 
        {
            // Load data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_lf32() 
        {
            // Load data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_lf64() 
        {
            // Load data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_si8() 
        {
            // Store data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_si16() 
        {
            // Store data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_si32() 
        {
            // Store data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_sf32() 
        {
            // Store data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_sf64() 
        {
            // Store data ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_newfunction(UInt32 method_ind)
        {
            PushCode(method_ind);

            PushOp(GetFunctionType());
        }
        void exec_call(UInt32 arg_count);
        void exec_construct(UInt32 arg_count);
        void exec_callmethod(UInt32 method_index, UInt32 arg_count);
        void exec_callstatic(UInt32 method_index, UInt32 arg_count);
        void exec_callsuper(UInt32 mn_index, UInt32 arg_count);
        void exec_callproperty(UInt32 mn_index, UInt32 arg_count);
        void exec_constructsuper(UInt32 arg_count);
        void exec_constructprop(UInt32 mn_index, UInt32 arg_count);
        void exec_callsuperid()
        {
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_callproplex(UInt32 mn_index, UInt32 arg_count);
        void exec_callinterface()
        {
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_callsupervoid(UInt32 mn_index, UInt32 arg_count);
        void exec_callpropvoid(UInt32 mn_index, UInt32 arg_count);
        void exec_sxi1() 
        {
            // Sign extend ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_sxi8() 
        {
            // Sign extend ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_sxi16() 
        {
            // Sign extend ... FP10 ...
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_applytype(UInt32 arg_count);
        void exec_newobject(UInt32 arg_count);
        void exec_newarray(UInt32 arr_size);
        void exec_newactivation();
        void exec_newclass(UInt32 v);
        void exec_getdescendants(UInt32 index);
        void exec_newcatch(UInt32 v);
        void exec_findpropglobalstrict(UInt32 v) 
        {
            PushCode(v);

            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_findpropglobal(UInt32 v)
        {
            PushCode(v);

            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_findpropstrict(UInt32 mn_index);
        void exec_findproperty(UInt32 mn_index);
        void exec_finddef(UInt32 v) 
        {
            PushCode(v);

            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_getlex(UInt32 mn_index);
        void exec_getabsobject(InstanceTraits::Traits& tr);
        void exec_setproperty(UInt32 mn_index);
        void exec_getlocal(UInt32 reg_num) 
        {
            PushCode(reg_num);
            const Value& reg_value = GetRegister(AbsoluteIndex(reg_num));

            PushOp(reg_value);

#ifdef UNSAFE_OPTIMIZATION
            SkipNextConvert(reg_value);
#endif
        }
        void exec_setlocal(UInt32 v) 
        {
            PushCode(v);

            SetRegister(AbsoluteIndex(v), PopOpValue());
        }
        void exec_getglobalscope();
        void exec_getscopeobject(UInt32 scope_index) 
        {
            PushCode(scope_index);

            PushOp(GetScopeValue(scope_index));
            // Clean up the *with* flag.
            GetOpStack().Back().SetWith(false);
        }
        void exec_getproperty(UInt32 mn_index);
        void exec_getouterscope(UInt32 v) 
        {
            PushCode(v);

            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_initproperty(UInt32 mn_index);
        void exec_deleteproperty(UInt32 mn_index);
        void exec_getslot(UInt32 slot_index);
        void exec_setslot(UInt32 slot_index);
        void exec_getglobalslot(UInt32 slot_index) 
        {
            PushCode(slot_index);

            // We can do better than this.
            PushOp(Value());
        }
        void exec_setglobalslot(UInt32 slot_index) 
        {
            PushCode(slot_index);

            // We can do better than this.
            PopOp();
        }
        void exec_convert_s() 
        {
            ConvertOpTo(GetStringType());
        }
        void exec_esc_xelem() 
        {
            ConvertOpTo(GetStringType());

            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_esc_xattr() 
        {
            ConvertOpTo(GetStringType());

            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_convert_i() 
        {
            ConvertOpTo(GetSIntType());
        }
        // Artificial command.
        void exec_convert_reg_i(UInt32 reg_num) 
        {
            // reg_num should be stored in wcode outside of this method.
            ConvertRegisterTo(AbsoluteIndex(reg_num), GetSIntType());
        }
        void exec_convert_u()
        {
            ConvertOpTo(GetUIntType());
        }
        void exec_convert_d() 
        {
            ConvertOpTo(GetNumberType());
        }
        // Artificial command.
        void exec_convert_reg_d(UInt32 reg_num) 
        {
            // reg_num should be stored in wcode outside of this method.
            ConvertRegisterTo(AbsoluteIndex(reg_num), GetNumberType());
        }
        void exec_convert_b() 
        {
            ConvertOpTo(GetBooleanType());
        }
        void exec_convert_o() 
        {
            // Error if value.IsNullOrUndefined() || !value.IsObjectStrict()
        }
        void exec_checkfilter() 
        {
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_coerce(UInt32 mn_index);
        void exec_coerce_b() 
        {
            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_coerce_a() 
        {
            // Coerce a value to the any type.
            // Indicates to the verifier that the value on the stack is of the any type (*). Does nothing to value.
        }
        void exec_coerce_i() 
        {
            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_coerce_d() 
        {
            // Not documented ...
            // Not implemented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_coerce_s() 
        {
            ConvertOpTo(GetStringType());
        }
        void exec_astype(UInt32 mn_index);
        void exec_astypelate();
        void exec_coerce_u() 
        {
            ConvertOpTo(GetUIntType());
        }
        void exec_coerce_o() 
        {
            // Not documented ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_negate() 
        {
            ConvertOpTo(GetNumberType());
        }
        void exec_increment()
        {
            ConvertOpTo(GetNumberType());
        }
        void exec_inclocal(UInt32 v)
        {
            ConvertRegisterTo(AbsoluteIndex(v), GetNumberType());
        }
        void exec_decrement()
        {
            ConvertOpTo(GetNumberType());
        }
        void exec_declocal(UInt32 v)
        {
            ConvertRegisterTo(AbsoluteIndex(v), GetNumberType());
        }
        void exec_typeof()
        {
            ConvertOpTo(GetStringType());
        }
        void exec_not() 
        {
            ConvertOpTo(GetBooleanType());
        }
        void exec_bitnot()
        {
            ConvertOpTo(GetSIntType());
        }
        void exec_concat() 
        {
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_add_d() 
        {
            PopOp(); // value2
            ConvertOpTo(GetNumberType());
        }
        void exec_add();
        void exec_subtract() 
        {
            PopOp(); // value2
            ConvertOpTo(GetNumberType());
        }
        void exec_multiply() 
        {
            PopOp(); // value2
            ConvertOpTo(GetNumberType());
        }
        void exec_divide()
        {
            PopOp(); // value2
            ConvertOpTo(GetNumberType());
        }
        void exec_modulo() 
        {
            PopOp(); // value2
            ConvertOpTo(GetNumberType());
        }
        void exec_lshift()
        {
            // (ECMA-262 section 11.7.1)
            // Documentation says that stack._1 should be SInt32

            PopOp(); // value2
            ConvertOpTo(GetSIntType());
        }
        void exec_rshift()
        {
            // The right shift is sign extended, resulting in a signed 32-bit integer. See
            // ECMA-262 section 11.7.2
            // (C++) The type of the result is the same as the type of the left operand.

            PopOp(); // value2
            ConvertOpTo(GetSIntType());
        }
        void exec_urshift() 
        {
            PopOp(); // value2
            ConvertOpTo(GetUIntType());
        }
        void exec_bitand() 
        {
            PopOp(); // value2
            ConvertOpTo(GetSIntType());
        }
        void exec_bitor() 
        {
            PopOp(); // value2
            ConvertOpTo(GetSIntType());
        }
        void exec_bitxor()
        {
            PopOp(); // value2
            ConvertOpTo(GetSIntType());
        }
        void exec_instanceof()
        {
            PopOp(); // type
            ConvertOpTo(GetBooleanType());
        }
        void exec_istype(UInt32 mn_index);
        void exec_in()
        {
            PopOp(); // obj
            ConvertOpTo(GetBooleanType());
        }
        void exec_inclocal_i(UInt32 v)
        {
            PushCode(v);

            ConvertRegisterTo(AbsoluteIndex(v), GetSIntType());
        }
        void exec_declocal_i(UInt32 v) 
        {
            PushCode(v);

            ConvertRegisterTo(AbsoluteIndex(v), GetSIntType());
        }
        void exec_getlocal0() 
        {
            const Value& reg_value = GetRegister(AbsoluteIndex(0));

            PushOp(reg_value);

#ifdef UNSAFE_OPTIMIZATION
            SkipNextConvert(reg_value);
#endif
        }
        void exec_getlocal1()
        {
            const Value& reg_value = GetRegister(AbsoluteIndex(1));

            PushOp(reg_value);

#ifdef UNSAFE_OPTIMIZATION
            SkipNextConvert(reg_value);
#endif
        }   
        void exec_getlocal2()
        {
            const Value& reg_value = GetRegister(AbsoluteIndex(2));

            PushOp(reg_value);

#ifdef UNSAFE_OPTIMIZATION
            SkipNextConvert(reg_value);
#endif
        }
        void exec_getlocal3()
        {
            const Value& reg_value = GetRegister(AbsoluteIndex(3));

            PushOp(reg_value);

#ifdef UNSAFE_OPTIMIZATION
            SkipNextConvert(reg_value);
#endif
        }
        void exec_setlocal0()
        {
            SetRegister(AbsoluteIndex(0), PopOpValue());
        }
        void exec_setlocal1()
        {
            SetRegister(AbsoluteIndex(1), PopOpValue());
        }
        void exec_setlocal2() 
        {
            SetRegister(AbsoluteIndex(2), PopOpValue());
        }
        void exec_setlocal3()
        {
            SetRegister(AbsoluteIndex(3), PopOpValue());
        }
        void exec_abs_jump(UInt32 v1, UInt32 v2) 
        {
            PushCode(v1);
            PushCode(v2);

            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }
        void exec_timestamp() 
        {
            // Not documented ...
            // Not implemented yet ...
            GetVM().ThrowVerifyError(VM::eNotImplementedError);
        }

    private:
        InstanceTraits::Traits& GetObjectType() const;
        InstanceTraits::Traits& GetClassType() const;
        InstanceTraits::Traits& GetFunctionType() const;
        InstanceTraits::Traits& GetArrayType() const;
        InstanceTraits::Traits& GetStringType() const;
        InstanceTraits::Traits& GetNumberType() const;
        InstanceTraits::Traits& GetSIntType() const;
        InstanceTraits::Traits& GetUIntType() const;
        InstanceTraits::Traits& GetBooleanType() const;
        InstanceTraits::Traits& GetNamespaceType() const;
        InstanceTraits::Traits& GetVectorSIntType() const;
        InstanceTraits::Traits& GetVectorUIntType() const;
        InstanceTraits::Traits& GetVectorNumberType() const;
        InstanceTraits::Traits& GetVectorStringType() const;
        InstanceTraits::Traits& GetNullType() const;
        InstanceTraits::Traits& GetVoidType() const;

        InstanceTraits::Traits& GetFunctType(const Value& value) const;
        InstanceTraits::Traits& GetFunctReturnType(const Value& value) const;
        Value GetPropertyType(const Traits& obj_traits, const SlotInfo& si) const;

    private:
        VMAbcFile& GetFile() const;
        VM& GetVM() const;
        Tracer& GetTracer()
        {
            return *pTracer;
        }

    private:
        State& operator=(const State&);

    private:
        void PushCode(UInt32 code) const;
        const Traits* GetValueTraits(const Value& v);
        const ScopeStackType* GetSavedScope() const;
        PropRef FindProp(const Multiname& mn, ScopeType& stype, UPInt& scope_index, UPInt& slot_index);

    private:
        Tracer* pTracer;
        Abc::TCodeOffset bcp;
        ValueArrayDH Registers;
        ValueArrayDH OpStack;
        ValueArrayDH ScopeStack;
    };

    ///////////////////////////////////////////////////////////////////////////
    // This class is very similar to one, which reads real values.
    class StackReader
    {
    public:
        typedef StackReader SelfType;

        StackReader(VM& vm, State& s)
            : VMRef(vm)
            , StateRef(s)
            , Num(0)
        {
        }

    protected:
        // How many extra records we have read ...
        void HaveRead(int _n)
        {
            Num += _n;
        }
        int GetRead() const
        {
            return Num;
        }

    protected:
        VM& GetVM() const
        {
            return VMRef;
        }
        State& GetState() const
        {
            return StateRef;
        }

        ValueArrayDH& GetOpStack() const
        {
            return GetState().OpStack;
        }
        Value PopOpValue()
        {
            return GetState().PopOpValue();
        }

        void CheckObject(const Value& v) const;

        int Read(Multiname& obj) const;

    private:
        SelfType& operator =(const SelfType&);

    private:
        VM&     VMRef;
        State&  StateRef;
        int     Num;
    };

    class MnReader
    {
    public:
        MnReader(VMAbcFile& file)
            : File(file)
        {
        }

    public:
        VMAbcFile& GetFile() const
        {
            return File;
        }
        const Abc::Multiname& GetMultiname(UPInt ind) const
        {
            return GetFile().GetConstPool().GetMultiname(ind);
        }

    private:
        MnReader& operator =(const MnReader&);

    private:
        VMAbcFile& File;
    };

    // ReadArgs classes are used to read arguments from stack in various call opcodes,
    // including callproperty, callmethod, callsuper, construct, etc.

    struct ReadArgs : public StackReader
    {
        typedef ReadArgs SelfType;

        ReadArgs(VM& vm, State& s, UInt32 arg_count);

        const Value* GetCallArgs() const
        {
            if (ArgNum > CallArgs.GetSize())
                return FixedArr;
            else
                return CallArgs.GetDataPtr();
        }
        unsigned GetCallArgsNum() const
        {
            return ArgNum;
        }

    private:
        SelfType& operator =(const SelfType&);

    private:
        enum {eFixedArrSize = 8};

        const unsigned  ArgNum;
        Value      FixedArr[eFixedArrSize];
        ValueArrayDH    CallArgs;
    };

    struct ReadArgsObject : public ReadArgs
    {
        typedef ReadArgsObject SelfType;

        ReadArgsObject(VM& vm, State& s, UInt32 arg_count)
            : ReadArgs(vm, s, arg_count)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadArgsObjectValue : public ReadArgsObject
    {
        typedef ReadArgsObjectValue SelfType;

        ReadArgsObjectValue(VM& vm, State& s, UInt32 arg_count)
            : ReadArgsObject(vm, s, arg_count)
            , value(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value value;
    };

    struct ReadArgsMn : public ReadArgs, public MnReader
    {
        typedef ReadArgsMn SelfType;

        ReadArgsMn(VMAbcFile& file, State& s, UInt32 arg_count, UInt32 mn_index)
            : ReadArgs(file.GetVM(), s, arg_count)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadArgsMnObject : public ReadArgsMn
    {
        typedef ReadArgsMnObject SelfType;

        ReadArgsMnObject(VMAbcFile& file, State& s, UInt32 arg_count, UInt32 mn_index)
            : ReadArgsMn(file, s, arg_count, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMn : public StackReader, public MnReader
    {
        typedef ReadMn SelfType;

        ReadMn(VMAbcFile& file, State& s, UInt32 mn_index)
            : StackReader(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    // Read compile-time multiname ...
    struct ReadMnCt : public StackReader, public MnReader
    {
        typedef ReadMnCt SelfType;

        ReadMnCt(VMAbcFile& file, State& s, UInt32 mn_index)
            : StackReader(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(GetMultiname(mn_index))
        {
            if (!ArgMN.IsCompileTime())
            {
                // The multiname at index must not be a runtime multiname.
                SF_ASSERT(false);
            }
        }

        const Abc::Multiname& ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValue : public StackReader
    {
        typedef ReadValue SelfType;

        ReadValue(VM& vm, State& s)
            : StackReader(vm, s)
            , ArgValue(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value ArgValue;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadObject : public StackReader
    {
        typedef ReadObject SelfType;

        ReadObject(VM& vm, State& s)
            : StackReader(vm, s)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMnCtValue : public ReadMnCt
    {
        typedef ReadMnCtValue SelfType;

        ReadMnCtValue(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadMnCt(file, s, mn_index)
            , value(s.PopOpValue())
        {
            HaveRead(1);
        }

        const Value value;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueObject : public ReadValue
    {
        typedef ReadValueObject SelfType;

        ReadValueObject(VM& vm, State& s)
            : ReadValue(vm, s)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueMn : public ReadValue, public MnReader
    {
        typedef ReadValueMn SelfType;

        ReadValueMn(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadValue(file.GetVM(), s)
            , MnReader(file)
            , ArgMN(file, GetMultiname(mn_index))
        {
            HaveRead(Read(ArgMN));
        }

        Multiname ArgMN;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadValueMnObject : public ReadValueMn
    {
        typedef ReadValueMnObject SelfType;

        ReadValueMnObject(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadValueMn(file, s, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

    struct ReadMnObject : public ReadMn
    {
        typedef ReadMnObject SelfType;

        ReadMnObject(VMAbcFile& file, State& s, UInt32 mn_index)
            : ReadMn(file, s, mn_index)
            , ArgObject(s.PopOpValue())
        {
            HaveRead(1);

            CheckObject(ArgObject);
        }

        const Value ArgObject;

    private:
        SelfType& operator =(const SelfType&);
    };

} // namespace TR

// Tracer is similar to the VM in the way that it tries to "execute" opcode.
// The main difference is that it doesn't care about real values. It does 
// care only about value's types.
// As a side effect it generates optimized byte code (it is actually word code).

class Tracer : FlashUI
{
    friend class TR::State;

public:
    Tracer(MemoryHeap* heap, const CallFrame& cf);
    ~Tracer();

public:
    // Can throw exceptions.
    void EmitCode();

public:
    //
    MemoryHeap* GetHeap() const
    {
        return Heap;
    }

    //
    const TR::Block& GetFirstBlock() const
    {
        SF_ASSERT(!Blocks.IsEmpty());
        return *Blocks.GetFirst();
    }
    const TR::Block& GetCurrBlock() const
    {
        SF_ASSERT(CurrBlock);
        return *CurrBlock;
    }

    //
    VMAbcFile& GetFile() const
    {
        return CF.GetFile();
    }

    //
    const Abc::MethodBodyInfo& GetMethodBodyInfo() const
    {
        return CF.GetMethodBodyInfo();
    }

    const ScopeStackType* GetSavedScope() const
    {
        return CF.GetSavedScope();
    }
    const Abc::HasTraits* GetHasTraits() const
    {
        return CF.GetHasTraits();
    }
    const Traits* GetOriginationTraits() const
    {
        return CF.GetOriginationTraits();
    }

    const Abc::File& GetAbcFile() const
    {
        return GetFile().GetAbcFile();
    }

    VM& GetVM() const
    {
        return GetFile().GetVM();
    }

    const Traits* GetValueTraits(const Value& v, const bool super_tr = false) const;
    const InstanceTraits::Traits* GetInstanceTraits(const Value& v) const;

    UPInt GetPrintOffset() const
    {
        return PrintOffset;
    }

    const TR::State& GetState(const TR::Block& b) const
    {
        return States[b.GetStateInd()];
    }

    DEBUG_CODE(ASString GetName() const { return ASString(CF.GetName());} )

public:
    void InitializeBlock(TR::Block& to, const TR::Block& from);
    void MergeBlock(TR::Block& to, const TR::Block& from);

private:
    int GetNextU30(UPInt opcode_offset = 0) const;
    // It will create a block if necessary.
    void StoreOffset(Abc::TCodeOffset bcp, const TR::State& st, SInt32 offset, int base = -1);
    void SkipNextOpcode(Abc::TCodeOffset& bcp, Abc::Code::OpCode opcode);
    void SkipNextConvert(Abc::TCodeOffset& bcp, Abc::Code::ResultType type);
    void SkipNextConvert(Abc::TCodeOffset& bcp, const Value& value)
    {
        SkipNextConvert(bcp, AsResultType(value));
    }

    void SkipOpCode(Abc::TCodeOffset& opcode_cp, Abc::TCodeOffset new_cp);
    void RegisterOpCode(Abc::TCodeOffset opcode_cp);

    Abc::Code::OpCode GetCurrOpCode(UPInt opcode_offset) const
    {
        using namespace Abc;
        TCodeOffset ccp = opcode_offset;
        return static_cast<Code::OpCode>(Read8(code, ccp));

    }
    Abc::Code::OpCode GetNextOpCode(Abc::TCodeOffset bcp) const
    {
        using namespace Abc;
        TCodeOffset ccp = bcp;
        return static_cast<Code::OpCode>(Read8(code, ccp));
    }
    Abc::Code::OpCode GetPrevOpCode(UPInt opcode_num_diff = 1) const;
    // offset - absolute byte offset.
    Abc::Code::OpCode GetPrevOpCode2(UPInt offset) const;
    // Return true if no further processing is needed ...
    bool SubstituteOpCode(Abc::TCodeOffset& bcp, TR::State& st, UInt8 byte_code);
    bool EmitFindProperty(TR::State& st, const int mn_index, bool get_prop = false DEBUG_ARG(bool show_getprop = false));

    void EmitGetAbsSlot(const UPInt index);
    void EmitSetAbsSlot(const UPInt index);
    void EmitInitAbsSlot(const UPInt index);
    bool EmitGetAbsObject(const Value& value, bool objOnStack = false);

    bool EmitGetSlot(const Value& value, const UPInt index, bool objOnStack = false);

    void EmitGetOuterScope(const UPInt index);

    // EmitGetValue is an optimized version of EmitGetAbsSlot(const UPInt index).
    // Sometimes we just know an exact value. This function will fall back to
    // EmitGetAbsSlot() in case we cannot use value.
    void EmitGetValue(const Value& value, const UPInt index, bool objOnStack = false);

    bool EmitGetProperty(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const int mn_index);
    bool EmitSetProperty(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const int mn_index);
    bool EmitCall(const Abc::Code::OpCode opcode, TR::State& st, const Traits* tr, const Multiname& mn, const UInt32 mn_index, const UInt32 arg_count);

    Abc::Code::ResultType GetSlotType(int ind, const Abc::HasTraits* ht) const;
    Abc::Code::ResultType AsResultType (const Abc::Multiname& mn) const;

    bool MatchPrevRT(TR::State& st, Abc::Code::ResultType t1, UPInt opcode_offset = 1) const;
    bool MatchPrevRT(TR::State& st, Abc::Code::ResultType t1, Abc::Code::ResultType t2, UPInt opcode_offset = 1) const;

    // Translate offset into a block.
    TR::Block* GetBlock(UPInt offset);
    // Can throw exceptions.
    void TraceBlock(Abc::TCodeOffset bcp, const TR::Block& initBlock);

private:
    InstanceTraits::Traits& GetObjectType() const
    {
        return GetVM().GetClassTraitsObject().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetClassType() const
    {
        return GetVM().GetClassTraitsClassClass().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetFunctionType() const
    {
        return GetVM().GetClassTraitsFunction().GetInstanceTraits();
    }
    InstanceTraits::Traits& GetArrayType() const
    {
        return GetVM().GetITraitsArray();
    }
    InstanceTraits::Traits& GetStringType() const
    {
        return GetVM().GetITraitsString();
    }
    InstanceTraits::Traits& GetNumberType() const
    {
        return GetVM().GetITraitsNumber();
    }
    InstanceTraits::Traits& GetSIntType() const
    {
        return GetVM().GetITraitsSInt();
    }
    InstanceTraits::Traits& GetUIntType() const
    {
        return GetVM().GetITraitsUInt();
    }
    InstanceTraits::Traits& GetBooleanType() const
    {
        return GetVM().GetITraitsBoolean();
    }
    InstanceTraits::Traits& GetNamespaceType() const
    {
        return GetVM().GetITraitsNamespace();
    }
    InstanceTraits::Traits& GetVectorSIntType() const
    {
        return GetVM().GetITraitsVectorSInt();
    }
    InstanceTraits::Traits& GetVectorUIntType() const
    {
        return GetVM().GetITraitsVectorUInt();
    }
    InstanceTraits::Traits& GetVectorNumberType() const
    {
        return GetVM().GetITraitsVectorNumber();
    }
    InstanceTraits::Traits& GetVectorStringType() const
    {
        return GetVM().GetITraitsVectorString();
    }
//     InstanceTraits::Traits& GetVectorObjectType() const
//     {
//         GetVM().GetClassTraitsVectorObject().GetInstanceTraits();
//     }

    InstanceTraits::Traits& GetNullType() const
    {
        return GetVM().GetITraitsNull();
    }
    InstanceTraits::Traits& GetVoidType() const
    {
        return GetVM().GetITraitsVoid();
    }

public:
    bool IsPrimitiveType(const Traits& tr) const;
    bool IsNumericType(const Value& v) const;

private:
    Tracer& operator =(const Tracer&);

private:
    struct Recalculate
    {
        Recalculate(Abc::TCodeOffset p, int b = -1)
            : pos(p)
            , base(b)
        {
            SF_ASSERT(pos > 0);
            //SF_ASSERT(base <= 0);
        }
        Abc::TCodeOffset pos;
        // offset of a command base relative to "pos".
        // "base" uses target coordinates.
        int base;
    };

    TR::State& GetState(const TR::Block& b)
    {
        return States[b.GetStateInd()];
    }

private:
    Abc::Code::ResultType AsResultType (const Value& v) const;
    Value AsValueType(Abc::Code::ResultType rt) const;

private:
    bool IsException() const
    {
        return GetVM().IsException();
    }

    Value GetGlobalObject(const TR::State& st) const;
    static const ClassTraits::Traits& GetSlotCTraits(const Traits& tr, SlotIndex ind);

    // Return NULL if a valid block cannot be created.
    TR::Block* AddBlock(const TR::State& st, UPInt from, bool dead = false, bool checkOpCode = true);

public:
    // UI related stuff.

    FlashUI& GetUI() { return *this; }

private:
    virtual void Output(OutputMessageType type, const char* msg);  
    virtual bool OnOpCode(const Abc::TOpCode& code, Abc::TCodeOffset offset, Abc::Code::OpCode opcode);
    virtual bool OnOpCode(const ArrayLH_POD<UInt8>& code, Abc::TCodeOffset offset, Abc::Code::OpCode opcode);

private:
    MemoryHeap* Heap;
    const CallFrame& CF;
    const ArrayLH_POD<UInt8>& bcode;
    Abc::TOpCode& wcode;

    bool done;      
    Abc::TCodeOffset curr_offset;
    //Abc::TCodeOffset cp;
    const UInt8* code;
    const Abc::TCodeOffset code_end;
    // Original opcode
    ArrayDH_POD<Abc::TCodeOffset> opcode_pos;
    // Positions, which require postponed recalculation.
    ArrayDH_POD<Recalculate> recalculate_pos;
    // Map of original cp to a new (long) cp.
    ArrayDH_POD<Abc::TCodeOffset> long_offset;

    // Registers.
    ValueArrayDH loc_reg;
    ArrayDH<TR::State> States;
    List<TR::Block> Blocks;
    UPInt PrintOffset;
    //UPInt BlockInd;
    TR::Block* CurrBlock;
}; // class Tracer

namespace TR
{
    inline
    InstanceTraits::Traits& State::GetObjectType() const
    {
        return GetTracer().GetObjectType();
    }

    inline
    InstanceTraits::Traits& State::GetClassType() const
    {
        return GetTracer().GetClassType();
    }

    inline
    InstanceTraits::Traits& State::GetFunctionType() const
    {
        return GetTracer().GetFunctionType();
    }

    inline
    InstanceTraits::Traits& State::GetArrayType() const
    {
        return GetTracer().GetArrayType();
    }

    inline
    InstanceTraits::Traits& State::GetStringType() const
    {
        return GetTracer().GetStringType();
    }

    inline
    InstanceTraits::Traits& State::GetNumberType() const
    {
        return GetTracer().GetNumberType();
    }

    inline
    InstanceTraits::Traits& State::GetSIntType() const
    {
        return GetTracer().GetSIntType();
    }

    inline
    InstanceTraits::Traits& State::GetUIntType() const
    {
        return GetTracer().GetUIntType();
    }

    inline
    InstanceTraits::Traits& State::GetBooleanType() const
    {
        return GetTracer().GetBooleanType();
    }

    inline
    InstanceTraits::Traits& State::GetNamespaceType() const
    {
        return GetTracer().GetNamespaceType();
    }

    inline
    InstanceTraits::Traits& State::GetVectorSIntType() const
    {
        return GetTracer().GetVectorSIntType();
    }

    inline
    InstanceTraits::Traits& State::GetVectorUIntType() const
    {
        return GetTracer().GetVectorUIntType();
    }

    inline
    InstanceTraits::Traits& State::GetVectorNumberType() const
    {
        return GetTracer().GetVectorNumberType();
    }

    inline
    InstanceTraits::Traits& State::GetVectorStringType() const
    {
        return GetTracer().GetVectorStringType();
    }

    inline
    InstanceTraits::Traits& State::GetNullType() const
    {
        return GetTracer().GetNullType();
    }

    inline
    InstanceTraits::Traits& State::GetVoidType() const
    {
        return GetTracer().GetVoidType();
    }

    inline
    void State::exec_getglobalscope() 
    {
        PushOp(GetTracer().GetGlobalObject(*this));
    }

    inline
    void State::PushCode(UInt32 code) const
    {
        pTracer->wcode.PushBack(code);
    }

    inline
    const Traits* State::GetValueTraits(const Value& v)
    {
        return GetTracer().GetValueTraits(v);
    }

    inline
    const ScopeStackType* State::GetSavedScope() const
    {
        return GetTracer().GetSavedScope();
    }

    inline
    VMAbcFile& State::GetFile() const
    {
        return GetTracer().GetFile();
    }

    inline
    VM& State::GetVM() const
    {
        return GetTracer().GetVM();
    }

} // namespace TR

#endif


}}} // namespace Scaleform { namespace GFx { namespace AS3 {

#endif // INC_AS3_TRACER_H

