/**********************************************************************

Filename    :   AS3_Abc.cpp
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

#include "AS3_Abc.h"

namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc 
{

///////////////////////////////////////////////////////////////////////////
template <>
int Read16(const UInt8* data, TCodeOffset& cp)
{
    int b1 = Read8(data, cp);
    int b2 = Read8(data, cp);

    return (b2 << 8) | b1;
}

template <>
int ReadS24(const UInt8* data, TCodeOffset& cp)
{
    int b1 = Read8(data, cp);
    int b2 = Read8(data, cp);
    int b3 = Read8(data, cp);
    int r = b3 << 16 | b2 << 8 | b1;
    
    if(b3 & 0x80) 
        r = -1 - ( r ^ 0xffffff );
    
    return r;
}

template <>
int ReadU30(const UInt8* data, TCodeOffset& cp)
{
    UInt32 shift = 0;
    UInt32 s = 0;
    int nr = 0;

    while (true)
    {
        UInt8 b = static_cast<UInt8>(Read8(data, cp));
        ++nr;
        s |= (b & 127) << shift;
        shift += 7;

        if (!(b & 128) || shift >= 32)
            break;
    }

    return s;
}

template <>
Double ReadDouble(const UInt8* data, TCodeOffset& cp)
{
    // The bytes in the abc are little endian ...
    union {
        Double v;
#if (SF_BYTE_ORDER == SF_LITTLE_ENDIAN)
        struct { UInt32 lo, hi; } w;
#else
        struct { UInt32 hi, lo; } w;
#endif
    };

    // The words in memory can be little endian or big endian.
    w.lo = Read8(data, cp) | Read8(data, cp) << 8 | Read8(data, cp) << 16 | Read8(data, cp) << 24;
    w.hi = Read8(data, cp) | Read8(data, cp) << 8 | Read8(data, cp) << 16 | Read8(data, cp) << 24;

    return v;
}

template <>
String ReadString(const UInt8* data, TCodeOffset& cp, UPInt size)
{
//     StringBuffer b(size);

    // !!! No checking.
    String b((const char*)(data + cp), size);
    cp += size;
    return b;
}
template <>
StringDataPtr ReadStringPtr(const UInt8* data, TCodeOffset& cp, UPInt size)
{
    // !!! No checking.
    StringDataPtr b((const char*)(data + cp), size);
    cp += size;
    return b;
}

///////////////////////////////////////////////////////////////////////////
bool IsValidValueKind(UInt8 vk)
{
    switch(vk)
    {
    case CONSTANT_Int:
    case CONSTANT_UInt:
    case CONSTANT_Double:
    case CONSTANT_Utf8:
    case CONSTANT_True:
    case CONSTANT_False:
    case CONSTANT_Null:
    case CONSTANT_Undefined:
    case CONSTANT_Namespace:
    case CONSTANT_PackageNamespace:
    case CONSTANT_PackageInternalNs:
    case CONSTANT_ProtectedNamespace:
    case CONSTANT_ExplicitNamespace:
    case CONSTANT_StaticProtectedNs:
    case CONSTANT_PrivateNs:
        return true;
    default:
        break;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
#ifdef GFX_AS3_VERBOSE
#define OP_NAME(n) , n
#else
#define OP_NAME(n)
#endif // GFX_AS3_VERBOSE

Code::OpCodeInfo Code::opcode_info[0x100] =
{
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x00") },
    {    0,  false,   0, rt_none    OP_NAME("OP_0x01") },
    {    0,  false,   0, rt_none    OP_NAME("nop") },
    {    0,  true,   -1, rt_none    OP_NAME("throw") },
    {    1,  true,    0, rt_prop_type    OP_NAME("getsuper") },
    {    1,  true,   -2, rt_none    OP_NAME("setsuper") },
    {    1,  true,    0, rt_none    OP_NAME("dxns") },
    {    0,  true,   -1, rt_none    OP_NAME("dxnslate") },
    {    1,  false,   0, rt_none    OP_NAME("kill") },
    {    0,  false,   0, rt_none    OP_NAME("label") },
    
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x0A") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x0B") },
    {    1,  false,   0, rt_int    OP_NAME("inclocal_i2") }, // My own ...
    {    1,  false,   0, rt_int    OP_NAME("declocal_i2") }, // My own ...
    
    {    1,  true,   -2, rt_none    OP_NAME("ifnlt") },
    {    1,  true,   -2, rt_none    OP_NAME("ifnle") },
    {    1,  true,   -2, rt_none    OP_NAME("ifngt") },
    {    1,  true,   -2, rt_none    OP_NAME("ifnge") },
    {    1,  false,   0, rt_none    OP_NAME("jump") },
    {    1,  false,  -1, rt_none    OP_NAME("iftrue") }, //
    {    1,  false,  -1, rt_none    OP_NAME("iffalse") }, //
    {    1,  true,   -2, rt_none    OP_NAME("ifeq") },
    {    1,  true,   -2, rt_none    OP_NAME("ifne") },
    {    1,  true,   -2, rt_none    OP_NAME("iflt") },
    {    1,  true,   -2, rt_none    OP_NAME("ifle") },
    {    1,  true,   -2, rt_none    OP_NAME("ifgt") },
    {    1,  true,   -2, rt_none    OP_NAME("ifge") },
    {    1,  false,  -2, rt_none    OP_NAME("ifstricteq") },
    {    1,  false,  -2, rt_none    OP_NAME("ifstrictne") },
    {    2,  false,  -1, rt_none    OP_NAME("lookupswitch") },
    {    0,  false,  -1, rt_none    OP_NAME("pushwith") }, // Push on a sckope stack ...
    {    0,  false,   0, rt_none    OP_NAME("popscope") },
    {    0,  true,   -1, rt_string    OP_NAME("nextname") },
    {    0,  true,   -1, rt_uint    OP_NAME("hasnext") }, // index ...
    {    0,  false,   1, rt_object    OP_NAME("pushnull") },
    {    0,  false,   1, rt_undefined    OP_NAME("pushundefined") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x22") },
    {    0,  true,   -1, rt_prop_type    OP_NAME("nextvalue") },
    {    1,  false,   1, rt_int    OP_NAME("pushbyte") },
    {    1,  false,   1, rt_int    OP_NAME("pushshort") },
    {    0,  false,   1, rt_boolean    OP_NAME("pushtrue") },
    {    0,  false,   1, rt_boolean    OP_NAME("pushfalse") },
    {    0,  false,   1, rt_number    OP_NAME("pushnan") },
    {    0,  false,  -1, rt_none    OP_NAME("pop") },
    {    0,  false,   1, rt_eval_type    OP_NAME("dup") },
    {    0,  false,   0, rt_eval_type    OP_NAME("swap") },
    {    1,  false,   1, rt_string    OP_NAME("pushstring") },
    {    1,  false,   1, rt_int    OP_NAME("pushint") },
    {    1,  false,   1, rt_uint    OP_NAME("pushuint") },
    {    1,  false,   1, rt_number    OP_NAME("pushdouble") },
    {    0,  false,  -1, rt_none    OP_NAME("pushscope") }, // Push on a sckope stack ...
    {    1,  false,   1, rt_object    OP_NAME("pushnamespace") },
    {    2,  true,    1, rt_boolean    OP_NAME("hasnext2") },
    
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x33") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x34") },
    {    1,  false,  -1, rt_none    OP_NAME("iftrue_b") },
    {    1,  false,  -1, rt_none    OP_NAME("iffalse_b") },
    
    {   0,   true,    0, rt_unknown    OP_NAME("li8") },
    {   0,   true,    0, rt_unknown    OP_NAME("li16") },
    {   0,   true,    0, rt_unknown    OP_NAME("li32") },
    {   0,   true,    0, rt_unknown    OP_NAME("lf32") },
    {   0,   true,    0, rt_unknown    OP_NAME("lf64") },
    {   0,   true,   -2, rt_unknown    OP_NAME("si8") },
    {   0,   true,   -2, rt_unknown    OP_NAME("si16") },
    {   0,   true,   -2, rt_unknown    OP_NAME("si32") },
    {   0,   true,   -2, rt_unknown    OP_NAME("sf32") },
    {   0,   true,   -2, rt_unknown    OP_NAME("sf64") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x3F") },
    {    1,  false,   1, rt_object    OP_NAME("newfunction") }, // Tamarin throws exceptions in this opcode.
    {    1,  true,   -1, rt_any    OP_NAME("call") },
    {    1,  true,    0, rt_object    OP_NAME("construct") },
    {    2,  true,    0, rt_any    OP_NAME("callmethod") },
    {    2,  true,    0, rt_any    OP_NAME("callstatic") },
    {    2,  true,    0, rt_any    OP_NAME("callsuper") },
    {    2,  true,    0, rt_any    OP_NAME("callproperty") },
    {    0,  false,   0, rt_none    OP_NAME("returnvoid") },
    {    0,  false,  -1, rt_any    OP_NAME("returnvalue") },
    {    1,  true,   -1, rt_none    OP_NAME("constructsuper") },
    {    2,  true,    0, rt_object    OP_NAME("constructprop") },
    {   -1,  true,    0, rt_any    OP_NAME("callsuperid") },
    {    2,  true,    0, rt_any    OP_NAME("callproplex") },
    {   -1,  true,    0, rt_any    OP_NAME("callinterface") },
    {    2,  true,   -1, rt_none    OP_NAME("callsupervoid") },
    {    2,  true,   -1, rt_none    OP_NAME("callpropvoid") },
    {    0,  false,   0, rt_unknown    OP_NAME("sxi1") },
    {    0,  false,   0, rt_unknown    OP_NAME("sxi8") },
    {    0,  false,   0, rt_unknown    OP_NAME("sxi16") },
    {    1,  true,    0, rt_unknown    OP_NAME("applytype") }, // Not documented ...
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x54") },
    {    1,  false,   1, rt_object    OP_NAME("newobject") }, // Tamarin throws exceptions in this opcode.
    {    1,  false,   1, rt_object    OP_NAME("newarray") }, // Tamarin throws exceptions in this opcode.
    {    0,  false,   1, rt_object    OP_NAME("newactivation") }, // Tamarin throws exceptions in this opcode.
    {    1,  true,    0, rt_object    OP_NAME("newclass") },
    {    1,  true,    0, rt_any    OP_NAME("getdescendants") }, // ???
    {    1,  false,   1, rt_object    OP_NAME("newcatch") }, // Tamarin throws exceptions in this opcode.
    {    1,  true,    0, rt_object    OP_NAME("findpropglobalstrict") },
    {    1,  true,    0, rt_object    OP_NAME("findpropglobal") },
    {    1,  true,    1, rt_object    OP_NAME("findpropstrict") },
    {    1,  true,    1, rt_object    OP_NAME("findproperty") },
    {    1,  true,    1, rt_unknown    OP_NAME("finddef") }, // Not documented ...
    {    1,  true,    1, rt_any    OP_NAME("getlex") },
    {    1,  true,   -2, rt_none    OP_NAME("setproperty") },
    {    1,  false,   1, rt_eval_type    OP_NAME("getlocal") },
    {    1,  false,  -1, rt_none    OP_NAME("setlocal") },
    {    0,  false,   1, rt_object    OP_NAME("getglobalscope") },
    {    1,  false,   1, rt_object    OP_NAME("getscopeobject") },
    {    1,  true,    0, rt_any    OP_NAME("getproperty") }, // undefined if property not found ...
    {    1,  false,   1, rt_unknown    OP_NAME("getouterscope") }, // Not documented ...
    {    1,  true,   -2, rt_none    OP_NAME("initproperty") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x69") },
    {    1,  true,    0, rt_none    OP_NAME("deleteproperty") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x6B") },
    {    1,  true,    0, rt_prop_type    OP_NAME("getslot") },
    {    1,  true,   -2, rt_none    OP_NAME("setslot") },
    {    1,  false,   1, rt_prop_type    OP_NAME("getglobalslot") },
    {    1,  false,  -1, rt_none    OP_NAME("setglobalslot") },
    {    0,  true,    0, rt_string    OP_NAME("convert_s") },
    {    0,  true,    0, rt_string    OP_NAME("esc_xelem") },
    {    0,  true,    0, rt_string    OP_NAME("esc_xattr") },
    {    0,  true,    0, rt_int    OP_NAME("convert_i") },
    {    0,  true,    0, rt_uint    OP_NAME("convert_u") },
    {    0,  true,    0, rt_number    OP_NAME("convert_d") },
    {    0,  false,   0, rt_boolean    OP_NAME("convert_b") }, // Tamarin throws exceptions in this opcode.
    {    0,  true,    0, rt_object    OP_NAME("convert_o") },
    {    0,  true,    0, rt_object    OP_NAME("checkfilter") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x79") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7A") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7B") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7C") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7D") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7E") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0x7F") },
    {    1,  true,    0, rt_any    OP_NAME("coerce") },
    {    0,  false,   0, rt_boolean    OP_NAME("coerce_b") }, // Tamarin throws exceptions in this opcode.  // convert_b is the same operation as coerce_b
    {    0,  true,    0, rt_any    OP_NAME("coerce_a") },
    {    0,  true,    0, rt_int    OP_NAME("coerce_i") },   // convert_i is the same operation as coerce_i
    {    0,  true,    0, rt_number    OP_NAME("coerce_d") },
    {    0,  true,    0, rt_string    OP_NAME("coerce_s") },   // convert_d is the same operation as coerce_d
    {    1,  true,    0, rt_any    OP_NAME("astype") },
    {    0,  true,   -1, rt_any    OP_NAME("astypelate") },
    {    0,  true,    0, rt_uint    OP_NAME("coerce_u") },   // convert_u is the same operation as coerce_u
    {    0,  true,    0, rt_object    OP_NAME("coerce_o") },
    
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8A") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8B") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8C") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8D") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8E") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x8F") },
    {    1,  false,  -2, rt_none    OP_NAME("ifnlt_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifnle_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifngt_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifnge_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifeq_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifge_i") },
    
    {    0,  true,    0, rt_number    OP_NAME("negate") },
    {    0,  true,    0, rt_number    OP_NAME("increment") },
    {    1,  true,    0, rt_number    OP_NAME("inclocal") },
    {    0,  true,    0, rt_number    OP_NAME("decrement") },
    {    1,  true,    0, rt_none    OP_NAME("declocal") },
    {    0,  false,   0, rt_string    OP_NAME("typeof") },
    {    0,  false,   0, rt_boolean    OP_NAME("not") },
    {    0,  true,    0, rt_int    OP_NAME("bitnot") },
    
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x98") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x99") },
    {    0,  false,   0, rt_int    OP_NAME("increment_i2") },
    {    0,  false,   0, rt_int    OP_NAME("decrement_i2") },
    
    {   -1,  true,   -1, rt_unknown    OP_NAME("concat") },
    {   -1,  true,   -1, rt_number    OP_NAME("add_d") },
    
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x9C") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x9D") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x9E") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0x9F") },
    {    1,  false,  -2, rt_none    OP_NAME("ifgt_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifle_i") },
    {    1,  false,  -2, rt_none    OP_NAME("iflt_i") },
    {    1,  false,  -2, rt_none    OP_NAME("ifne_i") },
    
    {    0,  true,   -1, rt_any    OP_NAME("add") },
    {    0,  true,   -1, rt_number    OP_NAME("subtract") },
    {    0,  true,   -1, rt_number    OP_NAME("multiply") },
    {    0,  true,   -1, rt_number    OP_NAME("divide") },
    {    0,  true,   -1, rt_number    OP_NAME("modulo") },
    {    0,  true,   -1, rt_int    OP_NAME("lshift") },
    {    0,  true,   -1, rt_int    OP_NAME("rshift") },
    {    0,  true,   -1, rt_uint    OP_NAME("urshift") },
    {    0,  true,   -1, rt_int    OP_NAME("bitand") },
    {    0,  true,   -1, rt_int    OP_NAME("bitor") },
    {    0,  true,   -1, rt_int    OP_NAME("bitxor") },
    {    0,  true,   -1, rt_boolean    OP_NAME("equals") },
    {    0,  true,   -1, rt_boolean    OP_NAME("strictequals") },
    {    0,  true,   -1, rt_boolean    OP_NAME("lessthan") },
    {    0,  true,   -1, rt_boolean    OP_NAME("lessequals") },
    {    0,  true,   -1, rt_boolean    OP_NAME("greaterthan") },
    {    0,  true,   -1, rt_boolean    OP_NAME("greaterequals") },
    {    0,  true,   -1, rt_boolean    OP_NAME("instanceof") },
    {    1,  true,    0, rt_boolean    OP_NAME("istype") },
    {    0,  true,   -1, rt_boolean    OP_NAME("istypelate") },
    {    0,  true,   -1, rt_boolean    OP_NAME("in") },

    //{   -1,  false,   0, rt_none    OP_NAME("OP_0xB5") },
    {    1,  false,   0, rt_none    OP_NAME("getabsobject") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0xB6") },
    {    1,  false,   0, rt_none    OP_NAME("getabsslot") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0xB7") },
    {    1,  false,   -2, rt_none    OP_NAME("setabsslot") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0xB8") },
    {    1,  false,   -2, rt_none    OP_NAME("initabsslot") },
    //{   -1,  false,   0, rt_none    OP_NAME("OP_0xB9") },
    {    1,  false,   -2, rt_none    OP_NAME("callsupermethod") },

    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBA") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBB") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBC") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBD") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBE") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xBF") },

    {    0,  true,    0, rt_int    OP_NAME("increment_i") },
    {    0,  true,    0, rt_int    OP_NAME("decrement_i") },
    {    1,  true,    0, rt_int    OP_NAME("inclocal_i") },
    {    1,  true,    0, rt_int    OP_NAME("declocal_i") },
    {    0,  true,    0, rt_int    OP_NAME("negate_i") },
    {    0,  true,   -1, rt_int    OP_NAME("add_i") },
    {    0,  true,   -1, rt_int    OP_NAME("subtract_i") },
    {    0,  true,   -1, rt_int    OP_NAME("multiply_i") },

    {   -1,  false,   0, rt_none    OP_NAME("OP_0xC8") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xC9") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCA") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCB") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCC") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCD") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCE") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xCF") },

    {    0,  false,   1, rt_eval_type    OP_NAME("getlocal_0") },
    {    0,  false,   1, rt_eval_type    OP_NAME("getlocal_1") },
    {    0,  false,   1, rt_eval_type    OP_NAME("getlocal_2") },
    {    0,  false,   1, rt_eval_type    OP_NAME("getlocal_3") },
    {    0,  false,  -1, rt_none    OP_NAME("setlocal_0") },
    {    0,  false,  -1, rt_none    OP_NAME("setlocal_1") },
    {    0,  false,  -1, rt_none    OP_NAME("setlocal_2") },
    {    0,  false,  -1, rt_none    OP_NAME("setlocal_3") },

    {   -1,  false,   0, rt_none    OP_NAME("OP_0xD8") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xD9") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDA") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDB") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDC") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDD") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDE") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xDF") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE0") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE1") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE2") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE3") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE4") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE5") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE6") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE7") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE8") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xE9") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xEA") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xEB") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xEC") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xED") },

    {    2,  false,   0, rt_none    OP_NAME("abs_jump") },
    {    4,  true,    0, rt_none    OP_NAME("debug") }, // Tamarin throws exceptions in this opcode.
    {    1,  true,    0, rt_none    OP_NAME("debugline") }, // Tamarin throws exceptions in this opcode.
    {    1,  true,    0, rt_none    OP_NAME("debugfile") }, // Tamarin throws exceptions in this opcode.
    {    1,  false,   0, rt_none    OP_NAME("OP_0xF2") }, // op_0xF2 is used internally by the Tamarin VM.
    {    0,  false,   0, rt_unknown    OP_NAME("timestamp") }, // Not documented ...

    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF4") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF5") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF6") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF7") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF8") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xF9") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFA") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFB") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFC") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFD") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFE") },
    {   -1,  false,   0, rt_none    OP_NAME("OP_0xFF") },
    // END
};

///////////////////////////////////////////////////////////////////////////
MethodBodyInfo::MethodBodyInfo()
: file_offset(0)
, method_info_ind(-1)
, max_stack(-1)
, local_reg_count(-1)
, init_scope_depth(-1)
, max_scope_depth(-1)
{
}

///////////////////////////////////////////////////////////////////////////
MethodBodyInfo::ExceptionInfo::ExceptionInfo()
: from(-1)
, to(-1)
, target(-1)
, exc_type_ind(-1)
, var_name_ind(-1)
{
}

MethodBodyInfo::ExceptionInfo::ExceptionInfo(SInt32 _from, SInt32 _to, SInt32 _target, SInt32 _exc_type_ind, SInt32 _var_name_ind)
: from(_from)
, to(_to)
, target(_target)
, exc_type_ind(_exc_type_ind)
, var_name_ind(_var_name_ind)
{
}

///////////////////////////////////////////////////////////////////////////
bool MethodBodyInfo::Exception::FindExceptionInfo(int cp, UPInt& handler_num) const
{
    for(; handler_num < info.GetSize(); ++handler_num)
    {
        const ExceptionInfo& ei = Get(handler_num);

        if (cp >= ei.GetFrom() && cp <= ei.GetTo())
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
void File::Clear()
{
    // DataSize = 0; // We do not clear DataSize on purpose.
    MinorVersion = 0;
    MajorVersion = 0;
    Const_Pool.Clear();
    Methods.Clear();
    Metadata.Clear();
    Traits.Clear();
    AS3_Classes.Clear();
    Scripts.Clear();
    MethodBodies.Clear();
}

const MethodBodyInfo& File::GetMethodBodyInfo(int method_info_ind) const
{
    for (UPInt i = 0; i < GetMethodBody().GetSize(); ++i)
    {
        if (GetMethodBody().Get(i).GetMethodInfoInd() == method_info_ind)
            return GetMethodBody().Get(i);
    }

    {
        //SF_ASSERT(false);

        // Actually, it seems to be a valid situation.
        // MethodTable body info just doesn't exist in this SWF file ...
    }

    return GetMethodBody().Get(0);
}

}}}} // namespace Scaleform { namespace GFx { namespace AS3 { namespace Abc {


