/**********************************************************************

Filename    :   AS3_CodeExecute.cpp
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

#include "AS3_CodeExecute.h"
#include "Abc/AS3_Abc.h"
#include "AS3_VM.h"
#include "AS3_VMRead.h"
#include "AS3_MovieRoot.h"
#include "Obj/AS3_Obj_Namespace.h" // We need this header for GCC Release configuration to pass linking.
#include "GFx/AMP/Amp_Server.h"

namespace Scaleform { namespace GFx { namespace AS3 
{
    
///////////////////////////////////////////////////////////////////////////
// By default execute only one top stack frame (including all function calls).
int VM::ExecuteCode(unsigned max_stack_depth)
{
    using namespace Abc;

    while (CallStack.GetSize() != 0)
    {
        State state = sExecute;
        const int call_stack_size = CallStack.GetSize();
        // !!! call_frame may be invalidated ...
        CallFrame& call_frame = CallStack.Back();
        VMAbcFile& file = call_frame.GetFile();
        unsigned int curr_offset = 0;
        const TOpCode::ValueType* code = NULL;
        TCodeOffset cp = 0;
        
        // We may already have an exception.
        // Let us store it because call_frame.GetCode() may throw it's own exception.
        const bool tmpHandleException = IsException();
        const Value tmpExceptionValue = GetExceptionValue();
        HandleException = false;
        ExceptionObj.Clean();

        const TOpCode& oc = call_frame.GetCode();

        // call_frame.GetCode() can throw exceptions.
        if (IsException())
        {
            state = sReturn;
            goto call_stack_label;
        }

        // Restore exception.
        if (tmpHandleException)
        {
            HandleException = tmpHandleException;
            ExceptionObj = tmpExceptionValue;
        }

        code = static_cast<const TOpCode::ValueType*>(oc.GetDataPtr());
        cp = call_frame.GetCodeOffset();
        
        // Exceptions ...
        if (CheckException(cp, call_frame, state))
            goto call_stack_label;

        // Regular opcodes ...              
        do
        {
            curr_offset = cp; // We need this line for a debugger for the time being ...
            
            const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(code, cp));

#ifdef USE_DEBUGGER
            // Removing the IF statement below makes code ~20% faster with MSVC ...     
            if (GetUI().NeedToCheckOpCode() && GetUI().OnOpCode(oc, curr_offset, opcode))
            {
                call_frame.SetCodeOffset(curr_offset);
                return max_stack_depth;
            }
            
            // Exception may happen during execution of GetUI().OnOpCode() because it calls toString().
            // Lets not go into opcode loop with a thrown exception.
            if (CheckException(cp, call_frame, state))
                goto call_stack_label;
#endif
                
            // Below this point call_frame may be invalidated ...
            switch (opcode)
            {
            case Code::op_nop:
                exec_nop();
                break;
            case Code::op_throw:
                {
                    // Probably, we should use *curr_offset* here ...
                    int position = exec_throw(cp, call_frame);
                    if (position < 0)
                    {
                        // Exception handling code is not in this frame ...
                        state = sReturn;
                        goto call_stack_label;
                    } else
                        cp = position;
                }
                break;
            case Code::op_getsuper:
                exec_getsuper(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setsuper:
                exec_setsuper(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_dxns:
                exec_dxns(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_dxnslate:
                exec_dxnslate();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_kill:
                exec_kill(ReadU30(code, cp));
                break;
            case Code::op_label:
                exec_label();
                break;
            case Code::op_ifnlt:
                {
                    int offset = exec_ifnlt(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifnlt_i:
                {
                    int offset = exec_ifnlt_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnle:
                {
                    int offset = exec_ifnle(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifnle_i:
                {
                    int offset = exec_ifnle_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifngt:
                {
                    int offset = exec_ifngt(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifngt_i:
                {
                    int offset = exec_ifngt_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifnge:
                {
                    int offset = exec_ifnge(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifnge_i:
                {
                    int offset = exec_ifnge_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_jump:
                {
                    int offset = exec_jump(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_iftrue:
                {
                    // No exceptions.
                    int offset = exec_iftrue(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_iftrue_b:
                {
                    int offset = exec_iftrue_b(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_iffalse:
                {
                    // No exceptions.
                    int offset = exec_iffalse(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_iffalse_b:
                {
                    int offset = exec_iffalse_b(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifeq:
                {
                    int offset = exec_ifeq(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifeq_i:
                {
                    int offset = exec_ifeq_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifne:
                {
                    int offset = exec_ifne(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifne_i:
                {
                    int offset = exec_ifne_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_iflt:
                {
                    int offset = exec_iflt(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_iflt_i:
                {
                    int offset = exec_iflt_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifle:
                {
                    int offset = exec_ifle(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifle_i:
                {
                    int offset = exec_ifle_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifgt:
                {
                    int offset = exec_ifgt(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifgt_i:
                {
                    int offset = exec_ifgt_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifge:
                {
                    int offset = exec_ifge(ReadS24(code, cp));
                    if (CheckException(cp, call_frame, state))
                        goto call_stack_label;
                    else
                        cp += offset;
                }
                break;
            case Code::op_ifge_i:
                {
                    int offset = exec_ifge_i(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifstricteq:
                {
                    int offset = exec_ifstricteq(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_ifstrictne:
                {
                    int offset = exec_ifstrictne(ReadS24(code, cp));
                    cp += offset;
                }
                break;
            case Code::op_lookupswitch:
                {
                    int base_location = curr_offset;
                    int default_offset = ReadS24(code, cp);
                    UInt32 case_count = ReadU30(code, cp);
                    
                    int index = exec_lookupswitch();
                    
                    if (index < 0 || static_cast<UInt32>(index) > case_count)
                        cp = base_location + default_offset;
                    else
                    {
                        cp += index * JumpOffsetCorrection<TOpCode::ValueType>();
                        int case_offset = ReadS24(code, cp);
                        cp = base_location + case_offset;
                    }
                }
                break;
            case Code::op_pushwith:
                exec_pushwith();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_popscope:
                exec_popscope();
                break;
            case Code::op_nextname:
                exec_nextname();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_hasnext:
                exec_hasnext();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushnull:
                exec_pushnull();
                break;
            case Code::op_pushundefined:
                exec_pushundefined();
                break;
            case Code::op_nextvalue:
                exec_nextvalue();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushbyte:
                exec_pushbyte(static_cast<UInt8>(Read8(code, cp)));
                break;
            case Code::op_pushshort:
                exec_pushshort(ReadU30(code, cp));
                break;
            case Code::op_pushtrue:
                exec_pushtrue();
                break;
            case Code::op_pushfalse:
                exec_pushfalse();
                break;
            case Code::op_pushnan:
                exec_pushnan();
                break;
            case Code::op_pop:
                exec_pop();
                break;
            case Code::op_dup:
                exec_dup();
                break;
            case Code::op_swap:
                exec_swap();
                break;
            case Code::op_pushstring:
                exec_pushstring(file, ReadU30(code, cp));
                break;
            case Code::op_pushint:
                exec_pushint(file, ReadU30(code, cp));
                break;
            case Code::op_pushuint:
                exec_pushuint(file, ReadU30(code, cp));
                break;
            case Code::op_pushdouble:
                exec_pushdouble(file, ReadU30(code, cp));
                break;
            case Code::op_pushscope:
                exec_pushscope();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_pushnamespace:
                exec_pushnamespace(file, ReadU30(code, cp));
                // ??? Exceptions ?
                break;
            case Code::op_hasnext2:
                {
                    UInt32 object_reg = ReadU30(code, cp);
                    UInt32 index_reg = ReadU30(code, cp);
                    exec_hasnext2(object_reg, index_reg);
                }
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_li8:
                exec_li8();
                break;
            case Code::op_li16:
                exec_li16();
                break;
            case Code::op_li32:
                exec_li32();
                break;
            case Code::op_lf32:
                exec_lf32();
                break;
            case Code::op_lf64:
                exec_lf64();
                break;
            case Code::op_si8:
                exec_si8();
                break;
            case Code::op_si16:
                exec_si16();
                break;
            case Code::op_si32:
                exec_si32();
                break;
            case Code::op_sf32:
                exec_sf32();
                break;
            case Code::op_sf64:
                exec_sf64();
                break;
            case Code::op_newfunction:
                exec_newfunction(ReadU30(code, cp));
                // No exceptions in exec_newfunction.
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_call:
                exec_call(ReadU30(code, cp));

                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_construct:
                exec_construct(ReadU30(code, cp));
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callmethod:
                {
                    UInt32 method_index = ReadU30(code, cp);
                    UInt32 arg_count = ReadU30(code, cp);
                    exec_callmethod(method_index, arg_count);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsupermethod:
                {
                    UInt32 method_index = ReadU30(code, cp);
                    UInt32 arg_count = ReadU30(code, cp);
                    exec_callsupermethod(method_index, arg_count);
                }

                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;

                break;
            case Code::op_callstatic:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_callstatic(file, _1, _2);
                }
            
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsuper:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_callsuper(file, _1, _2);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callproperty:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_callproperty(file, _1, _2);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_returnvoid:
                exec_returnvoid();
                state = sReturn;
                goto call_stack_label;
            case Code::op_returnvalue:
                exec_returnvalue();
                
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                
                state = sReturn;
                goto call_stack_label;
            case Code::op_constructsuper:
                exec_constructsuper(call_frame, ReadU30(code, cp));
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_constructprop:
                {
                    UInt32 mn_index = ReadU30(code, cp);
                    UInt32 arg_count = ReadU30(code, cp);
                    exec_constructprop(file, mn_index, arg_count);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callsuperid:
                exec_callsuperid();
                // ?????
                break;
            case Code::op_callproplex:
                {
                    UInt32 mn_index = ReadU30(code, cp);
                    UInt32 arg_count = ReadU30(code, cp);
                    exec_callproplex(file, mn_index, arg_count);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callinterface:
                exec_callinterface();
                // ????
                break;
            case Code::op_callsupervoid:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_callsupervoid(file, _1, _2);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_callpropvoid:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_callpropvoid(file, _1, _2);
                }
                
                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_sxi1:
                exec_sxi1();
                break;
            case Code::op_sxi8:
                exec_sxi8();
                break;
            case Code::op_sxi16:
                exec_sxi16();
                break;
            case Code::op_applytype:
                exec_applytype(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_newobject:
                exec_newobject(ReadU30(code, cp));
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newarray:
                exec_newarray(ReadU30(code, cp));
                // No exceptions in exec_newarray().
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newactivation:
                exec_newactivation(call_frame);
                // No exceptions in exec_newactivation().
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_newclass:
                exec_newclass(file, ReadU30(code, cp));

                if (CheckException(cp, call_frame, state) || NeedToStepInto(call_stack_size, cp, state))
                    goto call_stack_label;
                
                break;
            case Code::op_getdescendants:
                exec_getdescendants(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_newcatch:
                exec_newcatch(file, call_frame.GetMethodBodyInfo().GetException().Get(ReadU30(code, cp)));
                // Tamarin throws exceptions in this opcode.
                break;
            case Code::op_findpropglobalstrict:
                exec_findpropglobalstrict(ReadU30(code, cp));
                // ???
                break;
            case Code::op_findpropglobal:
                exec_findpropglobal(ReadU30(code, cp));
                // ???
                break;
            case Code::op_findpropstrict:
                exec_findpropstrict(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_findproperty:
                exec_findproperty(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_finddef:
                exec_finddef(ReadU30(code, cp));
                // ???
                break;
            case Code::op_getlex:
                exec_getlex(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setproperty:
                exec_setproperty(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getlocal:
                exec_getlocal(ReadU30(code, cp));
                break;
            case Code::op_setlocal:
                exec_setlocal(ReadU30(code, cp));
                break;
            case Code::op_getglobalscope:
                exec_getglobalscope();
                break;
            case Code::op_getscopeobject:
                exec_getscopeobject(ReadU30(code, cp));
                /* This check should be eliminated by the Verifier.
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                */
                break;
            case Code::op_getproperty:
                exec_getproperty(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getouterscope:
                exec_getouterscope(call_frame, ReadU30(code, cp));
                /* This check should be eliminated by the Verifier.
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                */
                break;
            case Code::op_initproperty:
                exec_initproperty(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_deleteproperty:
                exec_deleteproperty(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getslot:
                exec_getslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setslot:
                exec_setslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getglobalslot:
                exec_getglobalslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_setglobalslot:
                exec_setglobalslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_s:
                exec_convert_s();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_esc_xelem:
                exec_esc_xelem();
                // ??? Exceptions?
                break;
            case Code::op_esc_xattr:
                exec_esc_xattr();
                break;
            case Code::op_convert_i:
                exec_convert_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_u:
                exec_convert_u();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_d:
                exec_convert_d();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_convert_b:
                exec_convert_b();
                // Doesn't throw exceptions.
                break;
            case Code::op_convert_o:
                exec_convert_o();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_checkfilter:
                exec_checkfilter();
                // ??? Exceptions?
                break;
            case Code::op_coerce:
                exec_coerce(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_coerce_b:
                exec_coerce_b();
                break;
            case Code::op_coerce_a:
                exec_coerce_a();
                break;
            case Code::op_coerce_i:
                exec_coerce_i();
                break;
            case Code::op_coerce_d:
                exec_coerce_d();
                break;
            case Code::op_coerce_s:
                exec_coerce_s();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_astype:
                exec_astype(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_astypelate:
                exec_astypelate();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_coerce_u:
                exec_coerce_u();
                break;
            case Code::op_coerce_o:
                exec_coerce_o();
                break;
            case Code::op_negate:
                exec_negate();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment:
                exec_increment();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_inclocal:
                exec_inclocal(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_decrement:
                exec_decrement();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_declocal:
                exec_declocal(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_typeof:
                exec_typeof();
                break;
            case Code::op_not:
                exec_not();
                break;
            case Code::op_bitnot:
                exec_bitnot();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_concat:
                exec_concat();
                break;
            case Code::op_add_d:
                exec_add_d();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_add:
                exec_add();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_subtract:
                exec_subtract();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_multiply:
                exec_multiply();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_divide:
                exec_divide();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_modulo:
                exec_modulo();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_lshift:
                exec_lshift();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_rshift:
                exec_rshift();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_urshift:
                exec_urshift();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitand:
                exec_bitand();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitor:
                exec_bitor();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_bitxor:
                exec_bitxor();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_equals:
                exec_equals();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_strictequals:
                exec_strictequals();
                break;
            case Code::op_lessthan:
                exec_lessthan();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_lessequals:
                exec_lessequals();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_greaterthan:
                exec_greaterthan();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_greaterequals:
                exec_greaterequals();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_instanceof:
                exec_instanceof();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_istype:
                exec_istype(file, ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_istypelate:
                exec_istypelate();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_in:
                exec_in();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getabsobject:
                exec_getabsobject(ReadU30(code, cp));
                break;
            case Code::op_getabsslot:
                {
                    const SPInt slot_ind = ReadU30(code, cp);

                    // Get the value of a slot.
                    // slot_ind is ONE based.
                    // slot_ind - 1 is an index in a slot table.

                    // Read arguments ...
                    ReadObjectRef args(*this);

                    if (IsException() && ProcessException(cp, call_frame, state))
                        goto call_stack_label;

                    const Value _this(args.ArgObject, PickValue);
                    SF_ASSERT(_this.IsObject());
                    Object& obj = *_this.GetObject();

                    if (!obj.GetTraits().GetSlotValueUnsafe(args.ArgObject, AbsoluteIndex(slot_ind - 1), &obj) && ProcessException(cp, call_frame, state))
                        goto call_stack_label;
                }
                break;
            case Code::op_setabsslot:
                exec_setabsslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_initabsslot:
                exec_initabsslot(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment_i:
                exec_increment_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_decrement_i:
                exec_decrement_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_increment_i2:
                exec_increment_i2();
                break;
            case Code::op_decrement_i2:
                exec_decrement_i2();
                break;
            case Code::op_inclocal_i:
                exec_inclocal_i(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_inclocal_i2:
                exec_inclocal_i2(ReadU30(code, cp));
                break;
            case Code::op_declocal_i:
                exec_declocal_i(ReadU30(code, cp));
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_declocal_i2:
                exec_declocal_i2(ReadU30(code, cp));
                break;
            case Code::op_negate_i:
                exec_negate_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_add_i:
                exec_add_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_subtract_i:
                exec_subtract_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_multiply_i:
                exec_multiply_i();
                if (CheckException(cp, call_frame, state))
                    goto call_stack_label;
                break;
            case Code::op_getlocal0:
                exec_getlocal0();
                break;
            case Code::op_getlocal1:
                exec_getlocal1();
                break;
            case Code::op_getlocal2:
                exec_getlocal2();
                break;
            case Code::op_getlocal3:
                exec_getlocal3();
                break;
            case Code::op_setlocal0:
                exec_setlocal0();
                break;
            case Code::op_setlocal1:
                exec_setlocal1();
                break;
            case Code::op_setlocal2:
                exec_setlocal2();
                break;
            case Code::op_setlocal3:
                exec_setlocal3();
                break;
            case Code::op_abs_jump:
                {
                    UInt32 _1 = ReadU30(code, cp);
                    UInt32 _2 = ReadU30(code, cp);
                    exec_abs_jump(_1, _2);
                }
                break;
            case Code::op_debug:
                {
                    const UInt8 debug_type = static_cast<UInt8>(Read8(code, cp));
                    const int name_ind = ReadU30(code, cp);
                    const UInt8 dreg = static_cast<UInt8>(Read8(code, cp));
                    const int extra = ReadU30(code, cp);
                    SF_UNUSED4(debug_type, name_ind, dreg, extra);
                }
                exec_debug();
                break;
            case Code::op_debugline:
                exec_debugline(call_frame, ReadU30(code, cp));
                break;
            case Code::op_debugfile:
                exec_debugfile(call_frame, ReadU30(code, cp));
                break;
            case Code::op_0xF2:
                ReadU30(code, cp);
                // ???
                break;
            case Code::op_timestamp:
                exec_timestamp();
                break;
            }
        } while (true);


        call_stack_label:
        
        SF_AMP_CODE(SetActiveLine(0, 0);)

            // We shouldn't get here in the *Execute* state.
        SF_ASSERT(state != sExecute);
                    
        if (state != sStepInto)
        {
            // Finally drop the call frame ...
            DropCallFrame();
            
            // call_stack may be not empty at this point.
            if (--max_stack_depth == 0)
                break;
        } else
        {
            ++max_stack_depth;
        }
    }
    
    return max_stack_depth;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

