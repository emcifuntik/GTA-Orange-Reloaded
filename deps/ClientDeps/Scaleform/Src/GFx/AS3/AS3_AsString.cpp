/**********************************************************************

Filename    :   AS3_AsString.cpp
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

#include "AS3_AsString.h"
#include "AS3_VM.h"
#include "AS3_Tracer.h"
#include "Obj/AS3_Obj_Namespace.h"


namespace Scaleform
{

///////////////////////////////////////////////////////////////////////////
// Specialization for formatting of ASString.

class ASStrFormatter : public StrFormatter
{
public:
    ASStrFormatter(MsgFormat& f, const GFx::ASStringNode* v)
        : StrFormatter(f, StringDataPtr(v->pData, v->Size))
    {
    }
    ASStrFormatter(MsgFormat& f, const GFx::ASString& v)
        : StrFormatter(f, StringDataPtr(v.ToCStr(), v.GetSize()))
    {
    }
};

template <>
struct FmtInfo<const GFx::ASStringNode*>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<GFx::ASStringNode*>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<const GFx::ASString>
{
    typedef ASStrFormatter formatter;
};

template <>
struct FmtInfo<GFx::ASString>
{
    typedef ASStrFormatter formatter;
};

}


namespace Scaleform { namespace GFx { namespace AS3 
{

///////////////////////////////////////////////////////////////////////////
const char* AsString(const Abc::NamespaceKind kind)
{
    using namespace Abc;

    switch (kind)
    {
    case NS_Public:
        return "public";
    case NS_Protected:
        return "protected";
    case NS_StaticProtected:
        return "static protected";
    case NS_Private:
        return "private";
    case NS_Explicit:
        return "explicit";
    case NS_PackageInternal:
        return "package internal";
    default:
        break;
    }

    SF_ASSERT(false);
    return "Invalid Namespace type";
}

///////////////////////////////////////////////////////////////////////////
ASString AsString(const Value& value, StringManager& sm)
{
    switch(value.GetKind())
    {
//     case Value::kUndefined:
//         return sm.CreateConstString("undefined");
//     case Value::kNull:
//         return sm.CreateConstString("null");
//     case Value::kBoolean:
//         return sm.CreateConstString(value.AsBool() ? "true" : "false");
//     case Value::kInt:
//         return sm.CreateString(Scaleform::AsString(value.AsInt()));
//     case Value::kUInt:
//         return sm.CreateString(Scaleform::AsString(value.AsUInt()));
//     case Value::kNumber:
//         return sm.CreateString(Scaleform::AsString(value.AsNumber()));
//     case Value::kString:
//         return value.AsString();

    case Value::kUndefined:
    case Value::kBoolean:
    case Value::kInt:
    case Value::kUInt:
    case Value::kNumber:
    case Value::kString:
        {
            ASString r = sm.CreateEmptyString();
            value.Convert2String(r, sm).DoNotCheck();
            return r;
        }
    case Value::kThunk:
        return sm.CreateConstString("thunk");
    case Value::kThunkFunction:
        return sm.CreateConstString("thunk function");
    case Value::kMethodInd:
        return sm.CreateString("method ind: " + Scaleform::AsString(value.GetMethodInd()));

        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        return sm.CreateConstString("function Function() {}");
    case Value::kObject:
        if (value.GetObject())
            return sm.CreateConstString("[object ") + value.GetObject()->GetName() + "]";
        else
            return sm.GetBuiltin(AS3Builtin_null);
    case Value::kClass:
        if (value.GetObject())
            return sm.CreateConstString("[class ") + value.GetObject()->GetName() + "]";
        else
            return sm.GetBuiltin(AS3Builtin_null);
    case Value::kNamespace:
        {
            const Instances::Namespace& ns = value.AsNamespace();
            ASString result = sm.CreateConstString(AsString(ns.GetKind()));

            if (!ns.GetUri().IsEmpty())
            {
                result += " ";
                result += ns.GetUri();
            }
            return result;
        }
        // DO NOT delete this code.
        //      case Value::kFunction:
        //      case Value::kXML:
        //      case Value::kObject:
        //      case Value::kClass:
        //          return AsString(value.AsObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kMethodClosure:
        return sm.CreateConstString("MethodClosure");
    case Value::kThunkClosure:
        return sm.CreateConstString("ThunkClosure");
    }

    SF_ASSERT(false);
    return sm.CreateEmptyString();
}


///////////////////////////////////////////////////////////////////////////
// const char* AsString(const Abc::TraitInfo::Type type)
// {
//     switch(type)
//     {
//     case Abc::TraitInfo::tSlot:
//         return "Slot";
//     case Abc::TraitInfo::tMethod:
//         return "Method";
//     case Abc::TraitInfo::tGetter:
//         return "Getter";
//     case Abc::TraitInfo::tSetter:
//         return "Setter";
//     case Abc::TraitInfo::tClass:
//         return "Class";
//     case Abc::TraitInfo::tFunction:
//         return "Function";
//     case Abc::TraitInfo::tConst:
//         return "Const";
//     }
// 
//     SF_ASSERT(false);
//     return "unknown";
// }

///////////////////////////////////////////////////////////////////////////
String AsString(const ASStringNode* node)
{
    if (node)
        return String(node->pData, node->Size);

    return String();
}

///////////////////////////////////////////////////////////////////////////
// String AsString(const Value& value, const VM& vm)
// {
//     String result;
//     
//     result.AppendString(AsString(value));
//     
//     // Display class traits ...
//     {
//         const Traits& as3_traits = vm.GetClasses(value).GetTraits();
//         if (!as3_traits.GetSlots().IsEmpty())
//         {
//             result.AppendString(" Class Traits [");
//             result.AppendString(AsString(as3_traits));
//             result.AppendString("]");
//         }
//     }
//     
//     // Display instance traits ...
//     if (value.IsObject())
//     {
//         Object& obj = value.AsObject();
//         const Traits& as3_traits = const_cast<const Object&>(obj).GetTraits();
//         
//         if (!as3_traits.GetSlots().IsEmpty())
//         {
//             result.AppendString(" Instance Traits [");
//             result.AppendString(AsString(as3_traits, &obj));
//             result.AppendString("]");
//         }
//     }
//     
//     return result;
// }

///////////////////////////////////////////////////////////////////////////
// const char* AsString(const Value::KindType kind)
// {
//     switch(kind)
//     {
//     case Value::kUndefined:
//         return "Undefined";
//     case Value::kNull:
//         return "Null";
//     case Value::kBoolean:
//         return "Boolean";
//     case Value::kInt:
//         return "Int";
//     case Value::kUInt:
//         return "unsigned";
//     case Value::kNumber:
//         return "Number";
//     case Value::kString:
//         return "String";
//     case Value::kFunction:
//         return "Function";
//     case Value::kXML:
//         return "XML";
//     case Value::kObject:
//         return "Object";
//     case Value::kClass:
//         return "Class";
//     case Value::kNamespace:
//         return "Namespace";
//     default:
//         break;
//     }
//     
//     return NULL;
// }

///////////////////////////////////////////////////////////////////////////
// String AsString(const ValueArray& values, const VM& vm)
// {
//     String result;
// 
//     for(UPInt i = 0; i < values.GetSize(); ++i)
//     {
//         if (i > 0)
//             result.AppendString(", ");
// 
//         result.AppendString(AsString(values[i], vm));
//     }
// 
//     return result;
// }

#ifdef GFX_AS3_VERBOSE

///////////////////////////////////////////////////////////////////////////
String AsString(const Value& value)
{
    if (value.IsNull())
        return "null";

    switch(value.GetKind())
    {
    case Value::kUndefined:
        return "undefined";
    case Value::kBoolean:
        return value.AsBool() ? "true" : "false";
    case Value::kInt:
        return Scaleform::AsString(value.AsInt());
    case Value::kUInt:
        return Scaleform::AsString(value.AsUInt());
    case Value::kNumber:
        if (value.IsNaN())
            return "NaN";
        if (value.IsPOSITIVE_ZERO() || value.IsNEGATIVE_ZERO())
            return "0";
        if (value.IsPOSITIVE_INFINITY() || value.IsNEGATIVE_INFINITY())
            return "0";

        return Scaleform::AsString(value.AsNumber());
    case Value::kString:
        return "\"" + String(value.AsString().ToCStr()) + "\"";
    case Value::kThunk:
        return "thunk";
    case Value::kThunkFunction:
        return "thunk function";
    case Value::kMethodInd:
        return "method ind: " + Scaleform::AsString(value.GetMethodInd());
        
        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        return "function Function() {}";
    case Value::kObject:
        if (value.GetObject())
            return "[object " + value.GetObject()->GetName() + "]";
        else
            return "null";
    case Value::kClass:
        if (value.GetObject())
            return "[class " + value.GetObject()->GetName() + "]";
        else
            return "null";
    case Value::kNamespace:
        {
            const Instances::Namespace& ns = value.AsNamespace();
            String result(AsString(ns.GetKind()));

            if (!ns.GetUri().IsEmpty())
            {
                result += " ";
                result += ns.GetUri().ToCStr();
            }
            return result;
        }
        // DO NOT delete this code.
//      case Value::kFunction:
//      case Value::kXML:
//      case Value::kObject:
//      case Value::kClass:
//          return AsString(value.AsObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kMethodClosure:
        return "MethodClosure";
    case Value::kThunkClosure:
        return "ThunkClosure";
    }
    
    SF_ASSERT(false);
    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsStringDebug(const Object* obj)
{
    String sink;

    if (obj == NULL)
        sink = "null";
    else
        Format(sink, "{0}@{1:x}", obj->GetName(), UPInt(obj));

    return sink;
}

String AsStringDebug(const Value& value)
{
    switch(value.GetKind())
    {
    case Value::kUndefined:
        return "undefined";
    case Value::kBoolean:
        return value.AsBool() ? "true" : "false";
    case Value::kInt:
        return Scaleform::AsString(value.AsInt());
    case Value::kUInt:
        return Scaleform::AsString(value.AsUInt());
    case Value::kNumber:
        if (value.IsNaN())
            return "NaN";
        if (value.IsPOSITIVE_ZERO() || value.IsNEGATIVE_ZERO())
            return "0";
        if (value.IsPOSITIVE_INFINITY() || value.IsNEGATIVE_INFINITY())
            return "0";

        return Scaleform::AsString(value.AsNumber());
    case Value::kString:
        return "\"" + String(value.AsString().ToCStr()) + "\"";
    case Value::kThunk:
        {
            String sink;
            Format(sink, "thunk@{0:x}", UPInt(&value.AsThunk()));
            return sink;
        }
        break;
    case Value::kThunkFunction:
        {
            String sink;

            if (value.IsNull())
                sink = "null";
            else
                Format(sink, "thunk function@{0}@{1:x}", value.GetObject()->GetName(), UPInt(&value.AsThunkFunction()));

            return sink;
        }
        break;
    case Value::kMethodInd:
        return "method ind: " + Scaleform::AsString(value.GetMethodInd());

        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        {
            String sink;

            if (value.IsNull())
                sink = "null";
            else
                Format(sink, "function@{0:x}", UPInt(&value.AsFunction()));

            return sink;
        }
    case Value::kObject:
        {
            String sink;

            if (value.IsNull())
                sink = "null";
            else
                Format(sink, "{0}@{1:x}", value.GetObject()->GetName(), UPInt(value.GetObject()));

            return sink;
        }
    case Value::kClass:
        if (value.GetObject())
            return value.GetObject()->GetName().AppendChar('$').ToCStr();
        else
            return "null";
    case Value::kNamespace:
        {
            const Instances::Namespace& ns = value.AsNamespace();
            String result(AsString(ns.GetKind()));

            if (!ns.GetUri().IsEmpty())
            {
                result += " ";
                result += ns.GetUri().ToCStr();
            }
            return result;
        }
        // DO NOT delete this code.
        //      case Value::kFunction:
        //      case Value::kXML:
        //      case Value::kObject:
        //      case Value::kClass:
        //          return AsString(value.GetObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kMethodClosure:
        {
            String sink;
            
            if (value.GetClosure())
                Format(sink, "MethodClosure@{0}@{1:x}@{2:x}", value.GetClosure()->GetName(), UPInt(value.GetClosure()), UPInt(value.GetFunct()));
            else
                sink = "null";

            return sink;
        }
//         return "MethodClosure";
        break;
    case Value::kThunkClosure:
        {
            String sink;

            if (value.GetClosure())
                Format(sink, "ThunkClosure@{0}@{1:x}@{2:x}", value.GetClosure()->GetName(), UPInt(value.GetClosure()), UPInt(&value.GetThunkFunct()));
            else
                sink = "null";

            return sink;
        }
        break;
    }

    SF_ASSERT(false);
    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsString(const ValueArray& values)
{
    String result;
    
    for(UPInt i = 0; i < values.GetSize(); ++i)
    {
        if (i > 0)
            result.AppendString(", ");
        
        result.AppendString(AsString(values[i]));
    }
    
    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsStringDebug(const ValueArray& values, UPInt from)
{
    String result;

    for(UPInt i = from; i < values.GetSize(); ++i)
    {
        if (i > from)
            result.AppendString(", ");

        result.AppendString(AsStringDebug(values[i]));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsStringDebug(const ArrayPagedLH<Value>& values, UPInt from)
{
    String result;

    for(UPInt i = from; i < values.GetSize(); ++i)
    {
        if (i > from)
            result.AppendString(", ");

        result.AppendString(AsStringDebug(values[i]));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
void Output(FlashUI& ui, SPInt v)
{
    LongFormatter f(v);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

void Output(FlashUI& ui, UPInt v, bool as_hex)
{
    LongFormatter f(v);

    if (as_hex)
        f.SetBase(16);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

void Output(FlashUI& ui, Value::Number v)
{
    DoubleFormatter f(v);

    f.Convert();
    ui.Output(FlashUI::Output_Action, f.ToCStr());
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const ArrayPagedLH<Value>& values, UPInt from)
{
    for(UPInt i = from; i < values.GetSize(); ++i)
    {
        if (i > from)
            ui.Output(FlashUI::Output_Action, ", ");

        Output(vm, ui, values[i]);
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const ArrayDH<Value>& values, UPInt from)
{
    for(UPInt i = from; i < values.GetSize(); ++i)
    {
        if (i > from)
            ui.Output(FlashUI::Output_Action, ", ");

        Output(vm, ui, values[i]);
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(const VM& vm, FlashUI& ui, const Value& value)
{
    switch(value.GetKind())
    {
    case Value::kUndefined:
        ui.Output(FlashUI::Output_Action, "undefined");
        break;
    case Value::kBoolean:
        ui.Output(FlashUI::Output_Action, value.AsBool() ? "true" : "false");
        break;
    case Value::kInt:
        Output(ui, static_cast<SPInt>(value.AsInt()));
        break;
    case Value::kUInt:
        Output(ui, static_cast<UPInt>(value.AsUInt()));
        break;
    case Value::kNumber:
        if (value.IsNaN())
        {
            ui.Output(FlashUI::Output_Action, "NaN");
            break;
        }

        if (value.IsPOSITIVE_ZERO())
        {
            ui.Output(FlashUI::Output_Action, "+0");
            break;
        }

        if (value.IsNEGATIVE_ZERO())
        {
            ui.Output(FlashUI::Output_Action, "-0");
            break;
        }

        if (value.IsPOSITIVE_INFINITY())
        {
            ui.Output(FlashUI::Output_Action, "+Infinity");
            break;
        }

        if (value.IsNEGATIVE_INFINITY())
        {
            ui.Output(FlashUI::Output_Action, "-Infinity");
            break;
        }

        Output(ui, value.AsNumber());
        break;
    case Value::kString:
        ui.Output(FlashUI::Output_Action, "\"");
        ui.Output(FlashUI::Output_Action, value.AsString().ToCStr());
        ui.Output(FlashUI::Output_Action, "\"");
        break;
    case Value::kThunk:
        ui.Output(FlashUI::Output_Action, "thunk@");
        Output(ui, reinterpret_cast<UPInt>(&value.AsThunk()), true);
        break;
    case Value::kThunkFunction:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            ui.Output(FlashUI::Output_Action, "thunk function@");
            ui.Output(FlashUI::Output_Action, value.GetObject()->GetName().ToCStr());
            Output(ui, reinterpret_cast<UPInt>(&value.AsThunkFunction()), true);
        }
        break;
    case Value::kMethodInd:
        ui.Output(FlashUI::Output_Action, "method ind: ");
        Output(ui, static_cast<SPInt>(value.GetMethodInd()));
        break;

        // !!! Code below is a hack ...
        // We should call commented out code instead.
    case Value::kFunction:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            ui.Output(FlashUI::Output_Action, "function@");
            Output(ui, reinterpret_cast<UPInt>(&value.AsFunction()), true);
        }
        break;
    case Value::kObject:
        if (value.IsNull())
            ui.Output(FlashUI::Output_Action, "null");
        else
        {
            ui.Output(FlashUI::Output_Action, value.GetObject()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(value.GetObject()), true);
        }
        break;
    case Value::kClass:
        if (value.GetObject())
        {
            ui.Output(FlashUI::Output_Action, value.GetObject()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "$");
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kNamespace:
        {
            const Instances::Namespace& ns = value.AsNamespace();

            ui.Output(FlashUI::Output_Action, AsString(ns.GetKind()));
            if (!ns.GetUri().IsEmpty())
            {
                ui.Output(FlashUI::Output_Action, " ");
                ui.Output(FlashUI::Output_Action, ns.GetUri().ToCStr());
            }
        }
        break;
    // DO NOT delete this code.
    //      case Value::kFunction:
    //      case Value::kXML:
    //      case Value::kObject:
    //      case Value::kClass:
    //          return AsString(value.GetObject()) + " (" + AsString(value.GetKind()) + ")";
    case Value::kMethodClosure:
        if (value.GetClosure())
        {
            ui.Output(FlashUI::Output_Action, "MethodClosure@");
            ui.Output(FlashUI::Output_Action, value.GetClosure()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(value.GetClosure()), true);
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(value.GetFunct()), true);
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kThunkClosure:
        if (value.GetClosure())
        {
            ui.Output(FlashUI::Output_Action, "ThunkClosure@");
            ui.Output(FlashUI::Output_Action, value.GetClosure()->GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(value.GetClosure()), true);
            ui.Output(FlashUI::Output_Action, "@");
            Output(ui, reinterpret_cast<UPInt>(&value.GetThunkFunct()), true);
        }
        else
            ui.Output(FlashUI::Output_Action, "null");
        break;
    case Value::kInstanceTraits:
        {
            InstanceTraits::Traits* itr = &value.GetInstanceTraits();

            if (itr == &vm.GetITraitsVoid())
                ui.Output(FlashUI::Output_Action, "void");
            else if (itr == &vm.GetITraitsNull())
                ui.Output(FlashUI::Output_Action, "null");
            else
            {
                // This is trace-related info.
                ui.Output(FlashUI::Output_Action, value.GetInstanceTraits().GetName().ToCStr());
                ui.Output(FlashUI::Output_Action, "$");
            }
        }
        break;
    case Value::kClassTraits:
        {
            // This is trace-related info.
            ui.Output(FlashUI::Output_Action, value.GetClassTraits().GetName().ToCStr());
            ui.Output(FlashUI::Output_Action, "$$");
        }
        break;
    }
}

///////////////////////////////////////////////////////////////////////////
void Output(FlashUI& ui, Abc::TCodeOffset offset, const CallFrame& cf)
{
    using namespace Abc;

//     const NamespaceDisplayType mdt = dComplete;
    const File& file = cf.GetFile().GetAbcFile();

    const TOpCode& oc = cf.GetMethodBodyInfo().GetOpCode(cf);
    const TOpCode::ValueType* code = static_cast<const TOpCode::ValueType*>(oc.GetDataPtr());

    Output(cf.GetVM(), ui, file, code, offset, cf.GetName()->pData);

    /*
    const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(code, offset));

    Output(ui, UInt32(offset - 1));
    ui.Output(FlashUI::Output_Action, " ");
    ui.Output(FlashUI::Output_Action, Code::GetOpCodeInfo(opcode).name);
    ui.Output(FlashUI::Output_Action, " ");

    switch (opcode)
    {
    case Code::op_getabsobject:
        ui.Output(FlashUI::Output_Action, "obj: ");
        Output(ui, GetAbsObject(ReadU30(code, offset)));
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
        if (cf.GetName())
        {
            ui.Output(FlashUI::Output_Action, "// ");
            ui.Output(FlashUI::Output_Action, cf.GetName()->pData);
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

    if (offset == 1 && cf.GetName())
    {
        ui.Output(FlashUI::Output_Action, "// ");
        ui.Output(FlashUI::Output_Action, cf.GetName()->pData);
    }
    */
}

///////////////////////////////////////////////////////////////////////////
String AsString(const ValueArrayDH& values)
{
    String result;

    for(UPInt i = 0; i < values.GetSize(); ++i)
    {
        if (i > 0)
            result.AppendString(", ");

        result.AppendString(AsString(values[i]));
    }

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsStringDebug(const ValueArrayDH& values, UPInt from)
{
    String result;

    for(UPInt i = from; i < values.GetSize(); ++i)
    {
        if (i > from)
            result.AppendString(", ");

        result.AppendString(AsStringDebug(values[i]));
    }

    return result;
}

String AsString(const Instances::Namespace& ns, NamespaceDisplayType ndt)
{
    ASString str = ns.GetUri();
    String Uri(str.ToCStr(), str.GetSize());

    using namespace Abc;

    switch(ndt)
    {
    case dComplete:
        return String(AsString(ns.GetKind())) + (Uri.IsEmpty() ? String() : String(" " + Uri));
    case dDeclarationOnly:
    case dShort:
        switch(ns.GetKind())
        {
        case NS_Public:
        case NS_Explicit:
            // User-defined namespace ...
            return Uri;
            //        case NamespaceInfo::NS_Package: // This is treated as NS_Public
            //            return empty_string;
        case NS_PackageInternal:
            return "internal";
        case NS_Protected:
            return "protected";
        case NS_StaticProtected:
            return "static protected";
        case NS_Private:
            return "private";
        default:
            SF_ASSERT(false);
            break;
        }

        return Uri;
    }

    return String();
}

String AsString(const NamespaceSet& nss)
{
    return "[" + AsString(nss.GetNamespaces()) + "]";
}

#endif

///////////////////////////////////////////////////////////////////////////
// String AsString(Object& object)
// {
//     ASString str = object.ToString(); 
//     return String(str.ToCStr(), str.GetSize());
// }

// String AsString(Object& obj)
// {
//     String result;
//     
//     result.AppendString(AsString(obj));
//     
//     // Display class traits ...
//     {
//         const Traits& as3_traits = const_cast<const Object&>(obj).GetClasses().GetTraits();
//         if (!as3_traits.GetSlots().IsEmpty())
//         {
//             result.AppendString(" Class Traits [");
//             result.AppendString(AsString(as3_traits));
//             result.AppendString("]");
//         }
//     }
//     
//     // Display instance traits ...
//     {
//         const Traits& as3_traits = const_cast<const Object&>(obj).GetTraits();
//         
//         if (!as3_traits.GetSlots().IsEmpty())
//         {
//             result.AppendString(" Instance Traits [");
//             result.AppendString(AsString(as3_traits, &obj));
//             result.AppendString("]");
//         }
//     }
//     
//     return result;
// }

///////////////////////////////////////////////////////////////////////////
// String AsString(const Traits& t, const Abc::ConstPool& cp)
// {
//     String result;
//     const SlotContainerType& s = t.GetSlots();
//     
//     result.AppendString("[&");
//     for(UPInt i = 0; i < s.GetSize(); ++i)
//     {
//         ASStringNode* key_node = s.GetKey(i);
//         String key(key_node->pData, key_node->Size);
// 
//         result 
//             << "-|" 
//             << key 
//             << ": " 
//             << AsString(s[i], cp) 
//             << "&"
//             ;
//     }
//     result << "]";
//     
//     return result;
// }

// String AsString(const Traits& traits, AS3::Object* const obj)
// {
//     String result;
//     const SlotContainerType& slots = traits.GetSlots();
//     
//     for(UPInt i = 0; i < slots.GetSize(); ++i)
//     {
//         const SlotInfo& si = slots[i];
//         Value slot_value;
//         
//         si.GetValue(slot_value, obj);
//         
//         if (obj->GetVM().IsException())
//             break;
//         
//         if (i > 0)
//             result << ", ";
//         
//         result 
//             << "{" 
//             << i + 1 
//             << ", " 
//             << AsString(slots.GetKey(i)) 
//             << ", " 
//             << AsString(si.GetType())
//             << ", " 
//             << si.GetDataType()
//             << ", " 
//             << AsString(slot_value)
//             << "}" 
//             ;
//     }
//     
//     return result;
// }

///////////////////////////////////////////////////////////////////////////
// String AsString(const SlotInfo& info, const Abc::ConstPool& cp)
// {
//     String result;
//     
//     result << AsString(cp, info.GetNamespace());
//     
//     return result;
// }

// String AsString(const NamespaceSet& /*nss*/)
// {
//     return String();
// }


#ifdef GFX_AS3_VERBOSE

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::NamespaceInfo& info, NamespaceDisplayType ndt)
{
    using namespace Abc;

    static const String empty_string;

    switch(ndt)
    {
    case dComplete:
        return String(AsString(info.GetKind())) + (info.GetNameURI().IsEmpty() ? String() : String(" " + info.GetNameURI()));
    case dDeclarationOnly:
        switch(info.GetKind())
        {
        case NS_Public:
        case NS_Explicit:
            // User-defined namespace ...
            return info.GetNameURI().ToCStr();
//        case NamespaceInfo::NS_Package: // This is treated as NS_Public
//            return empty_string;
        case NS_PackageInternal:
            return "internal";
        case NS_Protected:
            return "protected";
        case NS_StaticProtected:
            return "static protected";
        case NS_Private:
            return "private";
        default:
            SF_ASSERT(false);
            break;
        }

        return info.GetNameURI().ToCStr();
    case dShort:
        // ???
        if (info.HasKind())
        {
            if (info.IsUserDefined())
                return info.GetNameURI().ToCStr();
        }
        else
            return "noname";

        break;
    }

    return empty_string;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::NamespaceSetInfo& info, NamespaceDisplayType ndt)
{
    String result("[");

    for(UPInt i = 0; i < info.GetSize(); ++i)
    {
        // Optimization for empty strings ...
        const String tmp = AsString(info.Get(cp, i), ndt);

        if(i > 0 && !tmp.IsEmpty() && result.GetSize() > 1)
            result += ", ";

        if (!tmp.IsEmpty())
            result += tmp;
    }

    return result + "]";
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::Multiname& info)
{
    switch(info.GetKind())
    {
    case Abc::MN_QName:
    case Abc::MN_QNameA:
        {
            String result;

            result += AsString(info.GetNamespace(cp), dComplete);

            if (!result.IsEmpty())
                result += "::";

            result += info.GetName(cp).ToCStr();

            return result;
        }
        break;
    case Abc::MN_RTQName:
    case Abc::MN_RTQNameA:
        return info.GetName(cp).ToCStr();
    case Abc::MN_RTQNameL:
    case Abc::MN_RTQNameLA:
        return "";
    case Abc::MN_Multiname:
    case Abc::MN_MultinameA:
        {
            String result;

            result = AsString(cp, info.GetNamespaceSetInfo(cp));

            if (!result.IsEmpty())
                result += "::";

            result += info.GetName(cp).ToCStr();

            return result;
        }
        break;
    case Abc::MN_MultinameL:
    case Abc::MN_MultinameLA:
        return AsString(cp, info.GetNamespaceSetInfo(cp));
    default:
	break;
    }

    SF_ASSERT(false);

    // To avoid compiler warnings ...
    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::ValueDetail& detail)
{
    using namespace Abc;

    const int vindex = detail.GetIndex();

    switch(detail.GetKind())
    {
    case CONSTANT_Int:
        return Scaleform::AsString(cp.GetInt(vindex));
    case CONSTANT_UInt:
        return Scaleform::AsString((unsigned int)cp.GetUInt(vindex));
    case CONSTANT_Double:
        return Scaleform::AsString(cp.GetDouble(vindex));
    case CONSTANT_Utf8:
        return cp.GetString(AbsoluteIndex(vindex)).ToCStr();
    case CONSTANT_True:
        return "true";
    case CONSTANT_False:
        return "false";
    case CONSTANT_Null:
        return "null";
    case CONSTANT_Undefined:
        return "Undefined";
    case CONSTANT_Namespace:
    case CONSTANT_PackageNamespace:
    case CONSTANT_PackageInternalNs:
    case CONSTANT_ProtectedNamespace:
    case CONSTANT_ExplicitNamespace:
    case CONSTANT_StaticProtectedNs:
    case CONSTANT_PrivateNs:
        return AsString(cp.GetNamespace(vindex));
    }

    return String();
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::MethodInfo& info)
{
    String result;

    //result += info.GetName();
    result += "(";

    for(UPInt i = 0; i < info.GetParamCount(); ++i)
    {
        if(i > 0)
            result += ", ";

        if(info.HasParamNames())
        {
            result += info.GetParamName(i).ToCStr();
            result += ":";
        }

        result += AsString(cp, info.GetParamType(cp, i));

        if(info.HasOptionalParams() && i >= (info.GetParamCount() - info.GetOptionalParamCount()))
            result += " = " + AsString(cp, info.GetOptionalParam(i - (info.GetParamCount() - info.GetOptionalParamCount())));
    }

    result += ") : ";
    result += AsString(cp, info.GetReturnType(cp));

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsStringFlag(const Abc::Instance& info)
{
    String result;

    if (!info.IsSealed())
        result += "dynamic ";

    if (info.IsFinal())
        result += "final ";

    if (info.IsInterface())
        result += "interface ";
    else
        result += "class ";

    // It is almost always presented ...		
#if 0
    if (info.HasProtectedNamespace())
        result += "(has protected namespace) ";
#endif

    return result;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Abc::ConstPool& cp, const Abc::ClassTable& as3_class, int ind)
{
    String str;
    const Abc::Instance& inst = as3_class.Get(ind).GetInstanceInfo();

    // Build a header ...
    str += AsStringFlag(inst) + AsString(cp, inst.GetName(cp));

    if (inst.HasSuperClass())
        str += " extends " + AsString(cp, inst.GetSuperClassName(cp));

    if (inst.GetInterfaces().GetSize() > 0)
    {
        str += " implements ";

        for(AbsoluteIndex i(0); i.Get() < inst.GetInterfaces().GetSize(); ++i)
        {
            if (i.Get() > 0)
                str += ", ";

            str += AsString(cp, inst.GetInterfaces().GetName(cp, i));
        }
    }

    // !!! Do not display traits and an initializer ...
#if 0
    // Static info ...
    const ClassInfo& info = as3_class.GetInfo(ind);
    str += AsString(info);

    // Part of Instance info is already displayed ...
    str += AsString(inst);
#endif

    return str;
}

///////////////////////////////////////////////////////////////////////////
String AsString(const Multiname& mn, NamespaceDisplayType ndt)
{
    switch(mn.GetKind())
    {
    case Abc::MN_QName:
    case Abc::MN_QNameA:
        {
            String result;

            result += AsString(mn.GetNamespace(), ndt);

            if (!result.IsEmpty())
                result += "::";

            result += AsString(mn.GetName());

            return result;
        }
        break;
    case Abc::MN_RTQName:
    case Abc::MN_RTQNameA:
        return AsString(mn.GetName());
    case Abc::MN_RTQNameL:
    case Abc::MN_RTQNameLA:
        return "";
    case Abc::MN_Multiname:
    case Abc::MN_MultinameA:
        {
            String result;

            if (ndt != dShort)
                result = AsString(mn.GetNamespaceSet());

            if (!result.IsEmpty())
                result += "::";

            result += AsString(mn.GetName());

            return result;
        }
        break;
    case Abc::MN_MultinameL:
    case Abc::MN_MultinameLA:
        return AsString(mn.GetNamespaceSet());
    default:
	break;
    }

    SF_ASSERT(false);
    return String();

//     return AsString(mn.GetName());
}

///////////////////////////////////////////////////////////////////////////
String AsString(Abc::TCodeOffset offset, const CallFrame& cf)
{
    using namespace Abc;

    String result;

//     const NamespaceDisplayType mdt = dComplete;
    const File& file = cf.GetFile().GetAbcFile();

    const TOpCode& oc = cf.GetMethodBodyInfo().GetOpCode(cf);
    const TOpCode::ValueType* code = static_cast<const TOpCode::ValueType*>(oc.GetDataPtr());

    const ConstPool& cp = file.GetConstPool();

    const Code::OpCode opcode = static_cast<Code::OpCode>(Read8(code, offset));

    const String prefix = Scaleform::AsString(offset - 1) + " " + Code::GetOpCodeInfo(opcode).name;

    switch (opcode)
    {
    case Code::op_getabsobject:
        result << "obj: " << GetAbsObject(ReadU30(code, offset));
        break;
    case Code::op_getsuper:
    case Code::op_setsuper:
        result << AsString(cp, cp.GetMultiname(ReadU30(code, offset)));
        break;
    case Code::op_dxns:
        result << cp.GetString(AbsoluteIndex(ReadU30(code, offset)));
        break;
    case Code::op_inclocal:
    case Code::op_declocal:
    case Code::op_inclocal_i:
    case Code::op_declocal_i:
    case Code::op_inclocal_i2:
    case Code::op_declocal_i2:
    case Code::op_kill:
        result << "reg " << Scaleform::AsString(ReadU30(code, offset));
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
        result << "offset: " << Scaleform::AsString(ReadS24(code, offset) + JumpOffsetCorrection<UInt8>() + 1);
        break;
    case Code::op_debugfile:
    case Code::op_pushstring:
        result << "'" << cp.GetString(AbsoluteIndex(ReadU30(code, offset))) << "'";
        break;
    case Code::op_pushbyte:
        result << Scaleform::AsString(Read8(code, offset));
        break;
    case Code::op_pushint:
        result << Scaleform::AsString(cp.GetInt(ReadU30(code, offset)));
        break;
    case Code::op_pushuint:
        result << Scaleform::AsString(cp.GetUInt(ReadU30(code, offset)));
        break;
    case Code::op_pushdouble:
        result << Scaleform::AsString(cp.GetDouble(ReadU30(code, offset)));
        break;
    case Code::op_pushnamespace:
        result << AsString(cp.GetNamespace(ReadU30(code, offset)));
        break;
    case Code::op_hasnext2:
        {
            UInt32 object_reg = ReadU30(code, offset);
            UInt32 index_reg = ReadU30(code, offset);
            result << "object_reg: " << Scaleform::AsString(object_reg) << " index_reg: " << Scaleform::AsString(index_reg);
        }
        break;
    case Code::op_newfunction:
        result << AsString(cp, file.GetMethods().Get(ReadU30(code, offset)));
        break;
    case Code::op_callmethod:
    case Code::op_callsupermethod:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            result << "method index: " << Scaleform::AsString(index) << " argc=" << Scaleform::AsString(arg_count);
        }
    case Code::op_callstatic:
        {
            int index = ReadU30(code, offset);
            int arg_count = ReadU30(code, offset);

            result << AsString(cp, file.GetMethods().Get(index)) << " argc=" << Scaleform::AsString(arg_count);
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

            result << AsString(cp, cp.GetMultiname(index)) << " argc=" << Scaleform::AsString(arg_count);
        }
        break;
    case Code::op_call:
    case Code::op_construct:
    case Code::op_constructsuper:
    case Code::op_newobject:
    case Code::op_newarray:
        result << "argc=" << Scaleform::AsString(ReadU30(code, offset));
        break;
    case Code::op_newclass:
        // result << AsString(GetClasses().GetInfo(bc.ReadU30()));
        result << AsString(cp, file.GetClasses(), ReadU30(code, offset));
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
        result << AsString(cp, cp.GetMultiname(ReadU30(code, offset)));
        break;
    case Code::op_getlocal:
    case Code::op_setlocal:
        result << "reg: " << Scaleform::AsString(ReadU30(code, offset));
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

            result << "default: " << Scaleform::AsString(default_offset) << " maxcase: " << Scaleform::AsString(case_count);
            for(int i = 0; i <= case_count; ++i)
                result << " " << Scaleform::AsString(ReadS24(code, offset));
        }
        break;
    case Code::op_debug:
        {
            const UInt8 debug_type = static_cast<UInt8>(Read8(code, offset));
            const int name_ind = ReadU30(code, offset);
            const UInt8 dreg = static_cast<UInt8>(Read8(code, offset));
            const int extra = ReadU30(code, offset);
            SF_UNUSED3(debug_type, extra, dreg);

            result << "name: " << cp.GetString(AbsoluteIndex(name_ind));
        }
        break;
    case Code::op_returnvalue:
    case Code::op_returnvoid:
        DEBUG_CODE(if (cf.GetName())result << "// " << cf.GetName());
        break;
    default:
        switch (Code::GetOpCodeInfo(opcode).operandCount)
        {
        case 1:
            result << Scaleform::AsString(ReadU30(code, offset));
            break;
        case 2:
            {
                int _1 = ReadU30(code, offset);
                int _2 = ReadU30(code, offset);
                result << Scaleform::AsString(_1) << ' ' << Scaleform::AsString(_2);
            }
            break;
        default:
            break;
        }
    }

    DEBUG_CODE(if (offset == 1 && cf.GetName()) result << "// " << cf.GetName());

    if (result.IsEmpty())
        result = prefix;
    else
        result = prefix + " " + result;

    return result;
}

static const char* offsets[11] = {
	"",
	"\t",
	"\t\t",
	"\t\t\t",
	"\t\t\t\t",
	"\t\t\t\t\t",
	"\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t",
	"\t\t\t\t\t\t\t\t\t\t",
};

void MakeOffset(FlashUI& ui, UPInt offset)
{
    while (offset)
    {
        const UPInt n = Alg::Min<UPInt>(offset, 10);
        ui.Output(FlashUI::Output_Action, offsets[n]);
        offset -= n;
    }
}

static void TraceF(const VM& vm, FlashUI& ui, UPInt offset, const char* prefix, const char* name, const ArrayPagedLH<Value>& values, UPInt from = 0)
{
    ui.Output(FlashUI::Output_Action, prefix);
    MakeOffset(ui, offset);
    ui.Output(FlashUI::Output_Action, name);
    ui.Output(FlashUI::Output_Action, ": ");
    Output(vm, ui, values, from);
    ui.Output(FlashUI::Output_Action, "\n");
}

static void TraceF(const VM& vm, FlashUI& ui, UPInt offset, const char* prefix, const char* name, const ArrayDH<Value>& values, UPInt from = 0)
{
    ui.Output(FlashUI::Output_Action, prefix);
    MakeOffset(ui, offset);
    ui.Output(FlashUI::Output_Action, name);
    ui.Output(FlashUI::Output_Action, ": ");
    Output(vm, ui, values, from);
    ui.Output(FlashUI::Output_Action, "\n");
}

#ifdef USE_WORDCODE
// static void TraceF(FlashUI& ui, int offset, const char* prefix, const char* name, const TR::ValueArrayDH& values, UPInt from = 0)
// {
//     ui.Output(FlashUI::Output_Action, prefix);
//     MakeOffset(ui, offset);
//     ui.Output(FlashUI::Output_Action, name);
//     ui.Output(FlashUI::Output_Action, ": ");
//     Output(ui, values, from);
//     ui.Output(FlashUI::Output_Action, "\n");
// }

void TraceVM(Tracer& tr, const char* prefix)
{
    FlashUI& ui = tr.GetUI();
    VM& vm = tr.GetVM();
    const Tracer& ctr = const_cast<const Tracer&>(tr);
    const TR::Block& block = ctr.GetCurrBlock();
    const TR::State& st = ctr.GetState(block);
    const UPInt offset = tr.GetPrintOffset();

    TraceF(vm, ui, offset, prefix, "stack", st.GetOpStack());
    TraceF(vm, ui, offset, prefix, "scope", st.GetScopeStack());
    TraceF(vm, ui, offset, prefix, "local", st.GetRegisters());
    if (tr.GetSavedScope())
        TraceF(vm, ui, offset, prefix, "stored scope", *tr.GetSavedScope());
}

void TraceOpCode(Tracer& tr, const ArrayLH_POD<UInt8>& code, Abc::TCodeOffset offset)
{
    FlashUI& ui = tr.GetUI();
    const Abc::File& file = tr.GetFile().GetAbcFile();

    MakeOffset(ui, tr.GetPrintOffset());
    // We do not know the name at this point.
    AS3::Output(tr.GetVM(), ui, file, code.GetDataPtr(), offset, NULL);
    ui.Output(FlashUI::Output_Action, "\n");
}
#endif

void TraceVM(const VM& vm, const char* prefix)
{
    FlashUI& ui = vm.GetUI();
    UPInt offset = vm.GetCallStack().GetSize();
    if (offset)
	    --offset;

    TraceF(vm, ui, offset, prefix, "stack", vm.GetOpStack(), vm.GetCurrCallFrame().GetOriginalStackSize());
    TraceF(vm, ui, offset, prefix, "scope", vm.GetScopeStack(), vm.GetCurrCallFrame().GetScopeStackBaseInd());
    TraceF(vm, ui, offset, prefix, "local", vm.GetRegisters(), vm.GetCurrCallFrame().GetRegBaseInd());
    if (vm.GetCurrCallFrame().GetSavedScope())
	TraceF(vm, ui, offset, prefix, "stored scope", *vm.GetCurrCallFrame().GetSavedScope());
    //if (vm.GetCurrCallFrame().GetOriginationTraits())
	//TraceF(vm.GetUI(), offset, prefix, "stored traits", AsStringDebug(Value((Class*)&vm.GetCurrCallFrame().GetOriginationTraits()->GetClass())));
}

void TraceOpCode(FlashUI& ui, const VM& vm, Abc::TCodeOffset offset)
{
    if (vm.GetCallStack().GetSize() == 0)
        return;

    MakeOffset(ui, vm.GetCallStack().GetSize() - 1);
    Output(ui, offset, vm.GetCallStack().Back());
    ui.Output(FlashUI::Output_Action, "\n");
}

#endif


}}} // namespace Scaleform { namespace GFx { namespace AS3 {


