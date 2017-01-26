/**********************************************************************

Filename    :   AS3_AsString.h
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

#ifndef INC_AS3_AsString_H
#define INC_AS3_AsString_H

#include "Kernel/SF_MsgFormat.h"
#include "AS3_Value.h"
#include "AS3_FlashUI.h"
#include "Abc/AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 
{

// namespace Abc
// {
//     class ConstPool;
// }

///////////////////////////////////////////////////////////////////////////
// Forward declaration.
class Object;
class Traits;
class VM;
class SlotInfo;
class Multiname;
class NamespaceSet;
class CallFrame;
class Namespace;

///////////////////////////////////////////////////////////////////////////
enum NamespaceDisplayType 
{
    dComplete,
    dDeclarationOnly,
    dShort
};

///////////////////////////////////////////////////////////////////////////
const char* AsString(const Abc::NamespaceKind kind);
String AsString(const ASStringNode* node);
ASString AsString(const Value& value, StringManager& sm);

///////////////////////////////////////////////////////////////////////////
// const char* AsString(const Abc::TraitInfo::Type type);

///////////////////////////////////////////////////////////////////////////
// String AsString(const Value& value, const VM& vm);
// const char* AsString(const Value::KindType kind);

///////////////////////////////////////////////////////////////////////////
// String AsString(const ValueArray& values, const VM& vm);

#ifdef GFX_AS3_VERBOSE

class Tracer;
class FlashUI;

void MakeOffset(FlashUI& ui, UPInt offset);

String AsStringDebug(const Object* obj);

String AsString(const Value& value);
String AsStringDebug(const Value& value);

String AsString(const ValueArray& values);
String AsStringDebug(const ValueArray& values, UPInt from = 0);
String AsStringDebug(const ArrayPagedLH<Value>& values, UPInt from = 0);

void Output(FlashUI& ui, SPInt v);
void Output(FlashUI& ui, UPInt v, bool as_hex = false);
void Output(FlashUI& ui, Value::Number v);
void Output(const VM& vm, FlashUI& ui, const ArrayPagedLH<Value>& values, UPInt from = 0);
void Output(const VM& vm, FlashUI& ui, const ArrayDH<Value>& values, UPInt from = 0);
void Output(const VM& vm, FlashUI& ui, const Value& value);
void Output(FlashUI& ui, Abc::TCodeOffset offset, const CallFrame& cf);
void TraceOpCode(FlashUI& ui, const VM& vm, Abc::TCodeOffset offset);
void TraceOpCode(Tracer& tr, const ArrayLH_POD<UInt8>& code, Abc::TCodeOffset offset);

String AsString(const ValueArrayDH& values);
String AsStringDebug(const ValueArrayDH& values, UPInt from = 0);

String AsString(Abc::TCodeOffset offset, const CallFrame& cf);

void TraceVM(const VM& vm, const char* prefix = "");
void TraceVM(Tracer& tr, const char* prefix = "");

String AsString(const Abc::NamespaceInfo& info, NamespaceDisplayType ndt = dShort);

String AsString(const Abc::ConstPool& cp, const Abc::NamespaceSetInfo& info, NamespaceDisplayType ndt = dComplete);

String AsString(const Abc::ConstPool& cp, const Abc::Multiname& info);
String AsString(const Multiname& mn, NamespaceDisplayType ndt = dComplete);

String AsString(const Abc::ConstPool& cp, const Abc::ValueDetail& detail);
String AsString(const Abc::ConstPool& cp, const Abc::MethodInfo& info);

String AsStringFlag(const Abc::Instance& info);
String AsString(const Abc::ConstPool& cp, const Abc::ClassTable& as3_class, int ind);

String AsString(const Namespace& ns, NamespaceDisplayType ndt);
String AsString(const NamespaceSet& nss);

template <typename T>
void Output(const VM& vm, FlashUI& ui, const Abc::File& file, const T* code, Abc::TCodeOffset offset, const char* cf_name)
{
    using namespace Abc;

    const Abc::ConstPool& cp = file.GetConstPool();
    const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(code, offset));

    Output(ui, UPInt(offset - 1));
    ui.Output(FlashUI::Output_Action, " ");
    ui.Output(FlashUI::Output_Action, Code::GetOpCodeInfo(opcode).name);
    ui.Output(FlashUI::Output_Action, " ");

    switch (opcode)
    {
    case Code::op_getabsobject:
        ui.Output(FlashUI::Output_Action, "obj: ");
        Output(vm, ui, GetAbsObject(ReadU30(code, offset)));
        break;
    case Code::op_getsuper:
    case Code::op_setsuper:
        ui.Output(FlashUI::Output_Action, AsString(cp, cp.GetMultiname(ReadU30(code, offset))));
        break;
    case Code::op_dxns:
        ui.Output(FlashUI::Output_Action, cp.GetString(AbsoluteIndex(ReadU30(code, offset))));
        break;
    case Code::op_inclocal:
    case Code::op_declocal:
    case Code::op_inclocal_i:
    case Code::op_declocal_i:
    case Code::op_inclocal_i2:
    case Code::op_declocal_i2:
    case Code::op_kill:
        ui.Output(FlashUI::Output_Action, "reg ");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
    case Code::op_ifnlt:
    case Code::op_ifnle:
    case Code::op_ifngt:
    case Code::op_ifnge:

    case Code::op_ifnlt_i:
    case Code::op_ifnle_i:
    case Code::op_ifngt_i:
    case Code::op_ifnge_i:

    case Code::op_jump:

    case Code::op_iftrue:
    case Code::op_iffalse:

    case Code::op_iftrue_b:
    case Code::op_iffalse_b:

    case Code::op_ifeq:
    case Code::op_ifge:
    case Code::op_ifgt:
    case Code::op_ifle:
    case Code::op_iflt:
    case Code::op_ifne:

    case Code::op_ifeq_i:
    case Code::op_ifge_i:
    case Code::op_ifgt_i:
    case Code::op_ifle_i:
    case Code::op_iflt_i:
    case Code::op_ifne_i:

    case Code::op_ifstricteq:
    case Code::op_ifstrictne:
        // 4 stands for size of a command plus size of an offset ...
        ui.Output(FlashUI::Output_Action, "offset: ");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadS24(code, offset) + JumpOffsetCorrection<UInt8>() + 1));
        break;
    case Code::op_debugfile:
    case Code::op_pushstring:
        ui.Output(FlashUI::Output_Action, "'");
        ui.Output(FlashUI::Output_Action, cp.GetString(AbsoluteIndex(ReadU30(code, offset))));
        ui.Output(FlashUI::Output_Action, "'");
        break;
    case Code::op_pushbyte:
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(Read8(code, offset)));
        break;
    case Code::op_pushint:
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetInt(ReadU30(code, offset))));
        break;
    case Code::op_pushuint:
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetUInt(ReadU30(code, offset))));
        break;
    case Code::op_pushdouble:
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(cp.GetDouble(ReadU30(code, offset))));
        break;
    case Code::op_pushnamespace:
        ui.Output(FlashUI::Output_Action, AsString(cp.GetNamespace(ReadU30(code, offset))));
        break;
    case Code::op_hasnext2:
        {
            UInt32 object_reg = ReadU30(code, offset);
            UInt32 index_reg = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, "object_reg: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(object_reg));
            ui.Output(FlashUI::Output_Action, " index_reg: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index_reg));
        }
        break;
    case Code::op_newfunction:
        ui.Output(FlashUI::Output_Action, AsString(cp, file.GetMethods().Get(ReadU30(code, offset))));
        break;
    case Code::op_callmethod:
    case Code::op_callsupermethod:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, "ind: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(index));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));
        }
        break;
    case Code::op_callstatic:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, AsString(cp, file.GetMethods().Get(index)));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));
        }
        break;
    case Code::op_callproperty:
    case Code::op_callpropvoid:
    case Code::op_callproplex:
    case Code::op_callsuper:
    case Code::op_callsupervoid:
    case Code::op_constructprop:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            ui.Output(FlashUI::Output_Action, AsString(cp, cp.GetMultiname(index)));
            ui.Output(FlashUI::Output_Action, " argc=");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(arg_count));
        }
        break;
    case Code::op_call:
    case Code::op_construct:
    case Code::op_constructsuper:
    case Code::op_newobject:
    case Code::op_newarray:
        ui.Output(FlashUI::Output_Action, "argc=");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
    case Code::op_newclass:
        ui.Output(FlashUI::Output_Action, AsString(cp, file.GetClasses(), ReadU30(code, offset)));
        break;
    case Code::op_getdescendants:
    case Code::op_getproperty:
    case Code::op_setproperty:
    case Code::op_initproperty:
    case Code::op_findpropstrict:
    case Code::op_findproperty:
    case Code::op_finddef:
    case Code::op_deleteproperty:
    case Code::op_istype:
    case Code::op_coerce:
    case Code::op_astype:
    case Code::op_getlex:
        ui.Output(FlashUI::Output_Action, AsString(cp, cp.GetMultiname(ReadU30(code, offset))));
        break;
    case Code::op_getlocal:
    case Code::op_setlocal:
        ui.Output(FlashUI::Output_Action, "reg: ");
        ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
        break;
        // case op_newcatch:
        // bc.ReadU30();
        // // index is a u30 that must be an index of an exception_info structure for this method.
        // // result << AsString(cp, cp.GetMultiname(bc.ReadU30()));
        // break;
    case Code::op_lookupswitch:
        {
            int default_offset = ReadS24(code, offset);
            int case_count = ReadU30(code, offset);
            //int case_offset;

            ui.Output(FlashUI::Output_Action, "default: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(default_offset));
            ui.Output(FlashUI::Output_Action, " maxcase: ");
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(case_count));
            for(int i = 0; i <= case_count; ++i)
            {
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadS24(code, offset)));
            }
        }
        break;
    case Code::op_debug:
        {
            const UInt8 debug_type = static_cast<UInt8>(Read8(code, offset));
            const int name_ind = ReadU30(code, offset);
            const UInt8 dreg = static_cast<UInt8>(Read8(code, offset));
            const int extra = ReadU30(code, offset);
            SF_UNUSED3(debug_type, extra, dreg);

            ui.Output(FlashUI::Output_Action, "name: ");
            ui.Output(FlashUI::Output_Action, cp.GetString(AbsoluteIndex(name_ind)));
        }
        break;
    case Code::op_returnvalue:
    case Code::op_returnvoid:
        if (cf_name)
        {
            ui.Output(FlashUI::Output_Action, "// ");
            ui.Output(FlashUI::Output_Action, cf_name);
        }
        break;
    default:
        switch (Code::GetOpCodeInfo(opcode).operandCount)
        {
        case 1:
            ui.Output(FlashUI::Output_Action, Scaleform::AsString(ReadU30(code, offset)));
            break;
        case 2:
            {
                int _1 = ReadU30(code, offset);
                int _2 = ReadU30(code, offset);

                ui.Output(FlashUI::Output_Action, Scaleform::AsString(_1));
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, Scaleform::AsString(_2));
            }
            break;
        default:
            break;
        }
    }

    if (offset == 1 && cf_name)
    {
        ui.Output(FlashUI::Output_Action, "// ");
        ui.Output(FlashUI::Output_Action, cf_name);
    }
}

#endif

///////////////////////////////////////////////////////////////////////////
// String AsString(const SlotInfo& info, const Abc::ConstPool& cp);

///////////////////////////////////////////////////////////////////////////
// String AsString(Object& object);
// String AsString(Object& obj);

///////////////////////////////////////////////////////////////////////////
// String AsString(const Traits& t, const Abc::ConstPool& cp);
// String AsString(const Traits& traits, Object* const obj = NULL);

///////////////////////////////////////////////////////////////////////////
// String AsString(const Abc::ConstPool& cp, const Instances::Namespace& ns);

}}} // namespace Scaleform { namespace GFx { namespace AS3 {


#endif // INC_AS3_AsString_H

