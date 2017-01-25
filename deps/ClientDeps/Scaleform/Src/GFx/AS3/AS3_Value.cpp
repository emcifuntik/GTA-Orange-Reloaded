/**********************************************************************

Filename    :   AS3_Value.cpp
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

#include "AS3_Value.h"
#include "Kernel/SF_MsgFormat.h"
#include "Obj/AS3_Obj_Namespace.h"

namespace Scaleform { namespace GFx { namespace AS3
{

///////////////////////////////////////////////////////////////////////////
Value::Value(const ASString& v)
: Flags(kString)
, value(v.GetNode())
{
    value.VS._1.VStr->AddRef();
}

Value::Value(ASStringNode* v)
: Flags(kString)
, value(v)
{
    //SF_ASSERT(v != NULL);
    if (v)
        v->AddRef();
    else
        SetKind(kUndefined);
}

void Value::Assign(const Value& other)
{
    if (&other != this)
    {
        Release();
        Flags = other.Flags;
        Bonus = other.Bonus;
        value = other.value;
        AddRef();
    }
}

void Value::AssignUnsafe(const Value& other)
{
    if (&other != this)
    {
        SF_ASSERT(!IsRefCounted());

        Flags = other.Flags;
        Bonus = other.Bonus;
        value = other.value;
        AddRef();
    }
}

void Value::Assign(const ASString& v)
{
    Release();
    SetKind(kString);
    value = v.GetNode();
    value.VS._1.VStr->AddRef();
}

void Value::AssignUnsafe(const ASString& v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kString);
    value = v.GetNode();
    value.VS._1.VStr->AddRef();
}

void Value::Assign(ASStringNode* v)
{
    Release();
    SetKind(kString);
    value = v;
    if (value.VS._1.VStr)
        value.VS._1.VStr->AddRef();
    else
        SetKind(kUndefined);
}

void Value::AssignUnsafe(ASStringNode* v)
{
    SF_ASSERT(!IsRefCounted());
    SetKind(kString);
    value = v;
    if (value.VS._1.VStr)
        value.VS._1.VStr->AddRef();
    else
        SetKind(kUndefined);
}

void Value::Swap(Value& other)
{
    const UInt32 tmpFlags = other.Flags;
    const Extra tmpBonus = other.Bonus;
    const VU tmpValue = other.value;

    other.Flags = Flags;
    other.Bonus = Bonus;
    other.value = value;

    Flags = tmpFlags;
    Bonus = tmpBonus;
    value = tmpValue;
}

void Value::Pick(Value& other)
{
    Release();

    Flags = other.Flags;
    Bonus = other.Bonus;
    Flags = other.Flags;

    other.SetKind(kUndefined);
}

void Value::PickUnsafe(Value& other)
{
    SF_ASSERT(!IsRefCounted());

    Flags = other.Flags;
    Bonus = other.Bonus;
    value = other.value;

    other.SetKind(kUndefined);
}

const Value& Value::GetUndefined()
{
    static Value v;

    return v;
}

const Value& Value::GetNull()
{
    static Value v((Object*)NULL);

    return v;
}

Value::ObjectTag Value::GetObjectTag() const
{
    switch (GetKind())
    {
    case kFunction:
        return otFunction;
    case kObject:
        return otObject;
    case kClass:
        return otClass;
    case kNamespace:
        return otNamespace;
    default:
        break;
    }

    return otInvalid;
}

bool Value::Convert2Boolean() const
{
    bool result = false;;

    switch(GetKind())
    {
    case kUndefined:
        result = false;
        break;
    case kBoolean:
        result = value;
        break;
    case kInt:
        result = value.VS._1.VInt != 0;
        break;
    case kUInt:
        result = value.VS._1.VUInt != 0;
        break;
    case kNumber:
        if (IsNaN() || IsPOSITIVE_ZERO() || IsNEGATIVE_ZERO())
            result = false;
        else
            result = true;
        break;
    case kString:
        result = value.VS._1.VStr->Size != 0;
        break;
    case kNamespace:
        result = false;
        break;
    case kFunction:
    case kObject:
    case kClass:
    case kThunkFunction:
    case kMethodClosure:
        if (IsNull())
            result = false;
        else
            result = true;
        break;
    case kThunkClosure:
        // In case of ThunkClosure we always have a thunk.
        result = false;
        break;
    case kThunk:
    case kMethodInd:
        result = true;
        break;
    }

    return result;
}

CheckResult Value::Convert2Number(Value::Number& result) const
{
    switch(GetKind())
    {
    case kNumber:
        result = value;
        break;
    case kUndefined:
        result = NumberUtil::NaN();
        break;
    case kBoolean:
        result = value.VS._1.VBool ? 1.0 : NumberUtil::POSITIVE_ZERO();
        break;
    case kInt:
        result = static_cast<Number>(value.VS._1.VInt);
        break;
    case kUInt:
        result = static_cast<Number>(value.VS._1.VUInt);
        break;
    case kString:
        {
            const ASString str(value.VS._1.VStr);           
            if (str.IsEmpty())
            {
                result = NumberUtil::POSITIVE_ZERO();
                break;
            }
            else
            {
                UInt32 len = str.GetSize();
                UInt32 offset;
                Double d = NumberUtil::StringToDouble(str.ToCStr(), len, &offset);
                if (NumberUtil::IsNaN(d) || d == 0)
                {
                    if (offset == len) //should be empty string, so result is actually 0;
                    {
                        result = NumberUtil::POSITIVE_ZERO();
                        break;
                    }
                    else
                    {
                        d = NumberUtil::StringToInt(str.ToCStr(), len, 0, &offset);
                    }
                }
                //Check if all following characters are white space and/or line terminators
                UInt32 index = UTF8Util::GetByteIndex(ASUtils::SkipWhiteSpace(str.ToCStr()+offset), str.ToCStr()+offset, len - offset);
                if ( index + offset >= len)
                {
                    result = d;
                    break;
                }
            }

            result = NumberUtil::NaN();
            break;
        }
    default:
        if (IsNull())
            result = NumberUtil::POSITIVE_ZERO();
        else
        {
            Value v;

            /*
            Apply the following steps:
            1. Call ToPrimitive(input argument, hint Number).
            2. Call ToNumber(Result(1)).
            3. Return Result(2).
            */
            if (!Convert2PrimitiveValue(v, hintNumber))
                return false;

            if (!v.Convert2Number(result))
                return false;
        }

        break;
    }

    return true;
}
    
CheckResult Value::Convert2Int32(SInt32& result) const
{
    switch (GetKind())
    {
    case kUndefined:
    case kThunk:
    case kThunkFunction:
        result = 0;
        break;
    case kInt:
    case kMethodInd:
        result = value;
        break;
    case kBoolean:
        result = (AsBool() ? 1 : 0);
        break;
    case kUInt:
        result = value.VS._1.VUInt;
        break;

    case kNumber:
    case kString:
        {
            Number n;
            if ((GetKind()) == kNumber)
                n = AsNumber();
            else if (!Convert2Number(n))
                return false;
       
            if (NumberUtil::IsNaNOrInfinity(n) || NumberUtil::IsNEGATIVE_ZERO(n) || NumberUtil::IsPOSITIVE_ZERO(n))
                result = 0;
            else
            {
                SInt32 sign = ( n < 0 ) ? -1 : 1;
                Number posint = sign * ::fmod(::floor( n * sign ), (UInt64)1 << 32);
                if (posint >= ((UInt64)1 << 31))
                    result = (SInt32)(posint - ((UInt64)1 << 32));
                else
                    result = (UInt32)posint;
            }
        }
        break;

    default:
        if (IsNull())
            result = 0;
        else
        {
            Value v;

            if (!Convert2PrimitiveValue(v))
                return false;

            if (!v.Convert2Int32(result))
                return false;
        }
    }

    return true;
}

CheckResult Value::Convert2UInt32(UInt32& result) const
{
    switch(GetKind())
    {
    case kUndefined:
    case kThunk:
    case kThunkFunction:
        result = 0;
        break;
    case kUInt:
        result = value.operator UInt32();
        break;
    case kInt:
        result = static_cast<UInt32>(AsInt());
        break;
    case kBoolean:
        result = (AsBool() ? 1 : 0);
        break;

    case kNumber:
    case kString:
        {
            Number n;
            if ((GetKind()) == kNumber)
                n = AsNumber();
            else if (!Convert2Number(n))
                return false;

            if (NumberUtil::IsNaNOrInfinity(n) || NumberUtil::IsNEGATIVE_ZERO(n) || NumberUtil::IsPOSITIVE_ZERO(n))
                result = 0;
            else
            {
                SInt32 sign = ( n < 0 ) ? -1 : 1;
                result =(UInt32) (sign * ::fmod(::floor( n * sign ), (UInt64)1 << 32));
            }
        }

        break;
    default:
        if (IsNull())
            result = 0;
        else
        {
            Value v;

            if (!Convert2PrimitiveValue(v))
                return false;

            if (!v.Convert2UInt32(result))
                return false;
        }
    }

    return true;
}

// Current implementation of double formatting, approximating ECMA 262-5
// section 9.8.1 (ToString rules). TBD: Move into utilities file.

// Shouldn't be static. Used in AS3_Obj_Number.
UPInt SF_ECMA_dtostr(char* buffer, int bufflen, Double val)
{   
    // Use fast code path for integers.
    int intVal = (int)val;
    if (val == (Double)intVal)
    {
        SFitoa(intVal, buffer, bufflen, 10);
        return SFstrlen(buffer);
    }

    SF_ASSERT(bufflen >= 40);

    // Check for NaN/Infinity and format.
    if (NumberUtil::IsNaN(val))
    {
        memcpy(buffer, "NaN", 4);
        return 3;
    }
    if (NumberUtil::IsPOSITIVE_INFINITY(val))
    {
        memcpy(buffer, "Infinity", sizeof("Infinity"));
        return sizeof("Infinity")-1;
    }
    if (NumberUtil::IsNEGATIVE_INFINITY(val))
    {
        memcpy(buffer, "-Infinity", sizeof("-Infinity"));
        return sizeof("-Infinity")-1;
    }    

    char  temp[40] = { 0 };
    char  *d, *s;
    // For positive-exponent values, ECMA 9.8.1 (ToString) rules require
    // 21 digits of precision; this doesn't seem to apply for negative
    // exponent values. Limit negative exponent value to 16 digits, since
    // that is the maximum value precisely represented by double.
    // Higher then 16 decimal value can result in false representations such
    // as "0.0012345" printed as "0.001234499999999".
    char   format[] = "%.16g";
    Double absVal = fabs(val);

    if ((absVal >= 1.0e16) && (absVal < 1.0e21))
    {
        format[3] = '7';
        if (absVal >= 1.0e17)
        {
            format[3] = '8';
            if (absVal >= 1.0e18)
            {
                format[3] = '9';
                if (absVal >= 1.0e19)
                {
                    format[2] = '2';
                    format[3] = '0';
                    if (absVal >= 1.0e20)
                    {
                        format[2] = '2';
                        format[3] = '1';
                    }
                }
            }
        }
    }

    const int nc1 = (int)SFsprintf(temp, sizeof(temp), format, val);

    // Get rid of a trailing number.
    {
        // Check for exponent part.
        bool have_e = false;
        for (int i = 0; i < nc1; ++i)
        {
            if (temp[i] == 'e')
            {
                have_e = true;
                break;
            }
        }

        if (!have_e)
        {
            int dot_pos = 0;
            bool have_dot = false;

            // Look for dot.
            for (; temp[dot_pos] != 0; ++dot_pos)
            {
                char c = temp[dot_pos];
                if (c == '.' || c == ',')
                {
                    have_dot = true;
                    break;
                }
            }

            if (have_dot && temp[nc1 - 1] != '0')
            {
                int i = nc1 - 2;
                for (; i > dot_pos; --i)
                {
                    if (temp[i] != '0')
                        break;
                }

                // We must have at least one not zero digit after a dot.
                if (i < nc1 - 2 && i != dot_pos)
                    temp[i + 1] = 0;
            }
        }
    }

    // This code patches the resulting string to match ECMA:
    //  1) "," -> "."; possible due to locale settings
    //  2) "e-005" -> "e-5"; no leading exponent zeros generated by sprintf
    //  3) "1.2345E-006" -> "0.0000012345"; Up to 6 leading zeros are needed

    bool negativeExponent = false;
    int  exponentValue = 0;

    for (d = buffer, s = temp; *s != 0; s++)
    {
        if (*s == ',')
        {
            *d++ = '.';
            continue;
        }

        if (*s == 'e')
        {
            // Exponent, read it.
            *d++ = *s++;

            // Read sign
            if (*s == '-' || *s == '+')
            {
                if (*s == '-')
                    negativeExponent = true;
                *d++ = *s++;
            }

            // Skip 0s of exponent, then copy it
            while (*s == '0')
                s++;

            // Copy exponent.
            while((*s >='0') && (*s <= '9'))
            {
                exponentValue = exponentValue * 10 + (*s - '0');
                *d++ = *s++;
            }
            break;
        }

        *d++ = *s;
    }

    // See if we need to re-format small negative exponent with forward-0s
    // to match ECMA 262 section 9.8.1 item point 7, as follows:    
    // Given, s * 10 ^ (n-k), where k is the number of decimal digits
    // ... 
    // 7. "If 0 < n <= 21, return the String consisting of the most significant n digits of
    //    the decimal representation of s, followed by a decimal point '.', followed by the
    //    remaining k-n digits of the decimal representation of s.
    // 8. If -6 < n <= 0, return the String consisting of the character '0', followed by the
    //    decimal point '.', followed by -n occurrences

    if (negativeExponent && (exponentValue > 0) && (exponentValue <= 6))
    {
        // Start over, re-scanning string & re-assigning digits
        s = temp; d = buffer;
        if (*s == '-')
        {
            s++;
            d++;
        }

        // We should start with non-zero digit and a dot. This check is probably 
        // not necessary since exponents won't have multiple digits in front, but do it
        // for safety it just in case we have some strange printf implementation...
        if ((*s >= '1') && (*s <= '9') && (d[1] == '.'))
        {
            // Insert 0s
            *d++ = '0';
            *d++ = '.';
            for (int zeroCount = 1; zeroCount < exponentValue; zeroCount++)
                *d++ = '0';

            // Now, copy available digits while skipping '.'.
            while(*s != 'e')
            {
                SF_ASSERT(*s != 0);
                if ((*s >= '0') && (*s <= '9'))
                {
                    *d++ = *s;
                }
                s++;
            }
        }
    }

    // Terminate string.
    *d = 0;    
    return d - buffer;
}

CheckResult Value::Convert2String(ASString& result, StringManager& sm) const
{
    switch(GetKind())
    {
    case kUndefined:
        result = sm.GetBuiltin(AS3Builtin_undefined);            
        break;
    case kThunk:
    case kThunkClosure:
        result = sm.CreateConstString("function Function() {}");
        break;
    case kBoolean:
        result = sm.GetBuiltin(value.VS._1.VBool ? AS3Builtin_true : AS3Builtin_false);
        break;
    case kInt:
        result = sm.CreateString(Scaleform::AsString(operator SInt32()));
        break;
    case kUInt:
        result = sm.CreateString(Scaleform::AsString(operator UInt32()));
        break;
    case kNumber:
        {
            char  buffer[40];
            UPInt length = SF_ECMA_dtostr(buffer, sizeof(buffer), value.VNumber);
            result = sm.CreateString(buffer, length);
        }      
        break;
    case kNamespace:
        result = AsNamespace().GetUri();
        break;
    case kFunction:
    case kClass:
    case kObject:
    case kThunkFunction:
    case kMethodClosure:
        if (IsNull())
            result = sm.GetBuiltin(AS3Builtin_null);
        else
        {
            Value v;

            /*
            Apply the following steps:
            Call ToPrimitive(input argument, hint String).
            Call ToString(Result(1)).
            Return Result(2).
            */
            if (!Convert2PrimitiveValue(v, hintString))
                return false;

            if (!v.Convert2String(result, sm))
                return false;
        }

        break;
    case kInstanceTraits:
    case kClassTraits:
        result = sm.GetBuiltin(AS3Builtin_empty_);
        break;
    default:
        result.AssignNode(value.VS._1.VStr);
        break;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
CheckResult LessThan(bool& result, const Value& l, const Value& r)
{
    Value val_res;
    const bool rc = AbstractLessThan(val_res, l, r);

    if (rc)
    {
        if (val_res.GetKind() == Value::kUndefined)
            result = false;
        else
            result = val_res;
    }

    return rc;
}

CheckResult GreaterThan(bool& result, const Value& l, const Value& r)
{
    Value val_res;
    // This call is different from LessThan() ...
    const bool rc = AbstractLessThan(val_res, r, l);

    if (rc)
    {
        if (val_res.GetKind() == Value::kUndefined)
            result = false;
        else
            result = val_res;
    }

    return rc;
}

CheckResult LessThanOrEqual(bool& result, const Value& l, const Value& r)
{
    Value val_res;
    // This call is different from LessThan() ...
    const bool rc = AbstractLessThan(val_res, r, l);

    if (rc)
    {
        if (val_res.GetKind() == Value::kUndefined || val_res.AsBool() == true)
            result = false;
        else
            result = true;
    }

    return rc;
}

CheckResult GreaterThanOrEqual(bool& result, const Value& l, const Value& r)
{
    Value val_res;
    // This call is different from LessThan() ...
    const bool rc = AbstractLessThan(val_res, l, r);

    if (rc)
    {
        if (val_res.GetKind() == Value::kUndefined || val_res.AsBool() == true)
            result = false;
        else
            result = true;
    }

    return rc;
}

///////////////////////////////////////////////////////////////////////////
CheckResult Add(StringManager& sm, Value& result, const Value& l, const Value& r)
{
    // We cannot restrict "result" to deal with non-reference counted values only
    // because "result" can be a value on op-stack.
    //SF_ASSERT(!result.IsRefCounted());

    // Strings.
    if (l.IsString() || r.IsString())
    {
        ASString left(sm.CreateEmptyString());
        ASString right(sm.CreateEmptyString());

        if (!l.Convert2String(left, sm) || !r.Convert2String(right, sm))
            // Exception
            return false;

        result = left + right;
        return true;
    }

    // Operands are primitives but strings.
    if (l.IsPrimitive() && r.IsPrimitive())
    {
        // Convert to Number.
        Value::Number l_num = 0.0;
        Value::Number r_num = 0.0;

        if (!l.Convert2Number(l_num) || !r.Convert2Number(r_num))
            // Exception
            return false;

        result.SetNumber(l_num + r_num);
        return true;
    }

    // Experimental code. No prorogation to Number.
    /*
    // Operands are primitives but strings.
    switch (l.GetKind())
    {
    case Value::kInt:
        switch (r.GetKind())
        {
        case Value::kInt:
            {
                Value::Number n = l.AsInt() + r.AsInt();

                if (n >= SF_MIN_UINT32 && n <= SF_MAX_UINT32)
                    result.SetUInt32(static_cast<UInt32>(n));
                else if (n >= SF_MIN_SINT32 && n <= SF_MAX_SINT32)
                    result.SetSInt32(static_cast<SInt32>(n));
                else
                    result.SetNumber(n);
            }
            return true;
        case Value::kUInt:
            {
                Value::Number n = l.AsInt() + r.AsUInt();

                if (n >= SF_MIN_UINT32 && n <= SF_MAX_UINT32)
                    result.SetUInt32(static_cast<UInt32>(n));
                else if (n >= SF_MIN_SINT32 && n <= SF_MAX_SINT32)
                    result.SetUInt32(static_cast<SInt32>(n));
                else
                    result.SetNumber(n);
            }
            return true;
        case Value::kNumber:
            result.SetNumber(l.AsInt() + r.AsNumber());
            return true;
        default:
            break;
        }
        break;
    case Value::kUInt:
        switch (r.GetKind())
        {
        case Value::kInt:
            {
                Value::Number n = l.AsUInt() + r.AsInt();

                if (n >= SF_MIN_UINT32 && n <= SF_MAX_UINT32)
                    result.SetUInt32(static_cast<UInt32>(n));
                else if (n >= SF_MIN_SINT32 && n <= SF_MAX_SINT32)
                    result.SetSInt32(static_cast<SInt32>(n));
                else
                    result.SetNumber(n);
            }
            return true;
        case Value::kUInt:
            {
                Value::Number n = l.AsUInt() + r.AsUInt();

                // There is no way it can be SInt32.
                if (n >= SF_MIN_UINT32 && n <= SF_MAX_UINT32)
                    result.SetUInt32(static_cast<UInt32>(n));
                else
                    result.SetNumber(n);
            }
            return true;
        case Value::kNumber:
            result.SetNumber(l.AsUInt() + r.AsNumber());
            return true;
        default:
            break;
        }
        break;
    case Value::kNumber:
        result.SetNumber(l.AsNumber() + r.AsNumber());
        return true;
    default:
        // Objects and booleans will be handled in the next block.
        break;
    }
    */

    // Convert to primitive values.
    Value l_prim;
    Value r_prim;

    if (!l.Convert2PrimitiveValue(l_prim) || !r.Convert2PrimitiveValue(r_prim))
        // Exception
        return false;

    return Add(sm, result, l_prim, r_prim);
}

///////////////////////////////////////////////////////////////////////////
CheckResult Subtract(Value& result, const Value& l, const Value& r)
{
    Value::Number left;
    Value::Number right = 0.0;

    if (!l.Convert2Number(left) || !r.Convert2Number(right))
        // Exception
        return false;

    result.SetNumber(left - right);

    return true;
}

///////////////////////////////////////////////////////////////////////////
CheckResult Multiply(Value& result, const Value& l, const Value& r)
{
    Value::Number left;
    Value::Number right = 0.0;

    if (!l.Convert2Number(left) || !r.Convert2Number(right))
        // Exception
        return false;

    result.SetNumber(left * right);

    return true;
}

///////////////////////////////////////////////////////////////////////////
// Increment treats numeric values as Number.
void Increment(Value& l)
{
	switch (l.GetKind())
	{
	case Value::kInt:
        if (l.AsInt() == SF_MAX_SINT32)
            // Switch to unsigned values.
            l.SetUInt32Unsafe((UInt32)l.AsInt() + 1);
        else
            l.AsInt() += 1;
		break;
	case Value::kUInt:
        if (l.AsUInt() == SF_MAX_UINT32)
            // Switch to Number values.
            l.SetNumberUnsafe((Value::Number)l.AsUInt() + 1);
        else
		    l.AsUInt() += 1;
		break;
	case Value::kNumber:
		l.AsNumber() += 1;
		break;
	default:
        {
            Value::Number num;
            if (l.Convert2Number(num))
                l.SetNumber(num + 1);
        }
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
// Decrement treats numeric values as Number.
void Decrement(Value& l)
{
	switch (l.GetKind())
	{
	case Value::kInt:
        if (l.AsInt() == SF_MIN_SINT32)
            // Switch to Number values.
            l.SetNumberUnsafe((Value::Number)l.AsInt() - 1);
        else
		    l.AsInt() -= 1;
		break;
	case Value::kUInt:
        if (l.AsUInt() == SF_MIN_UINT32)
            // Switch to signed values.
            l.SetSInt32Unsafe(-1);
        else
		    l.AsUInt() -= 1;
		break;
	case Value::kNumber:
		l.AsNumber() -= 1;
		break;
	default:
        {
            Value::Number num;
            if (l.Convert2Number(num))
                l.SetNumber(num - 1);
        }
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
// Negate treats numeric values as Number.
void Negate(Value& l)
{
	switch (l.GetKind())
	{
	case Value::kInt:
        if (l.AsInt() != 0)
        {
            l.AsInt() = -l.AsInt();
            break;
        }

        l.SetNumberUnsafe(0.0);
        // No break on purpose.
	case Value::kNumber:
		l.AsNumber() = -l.AsNumber();
		break;
	default:
        {
            Value::Number num;
            if (l.Convert2Number(num))
                l.SetNumber(-num);
        }
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
CheckResult AbstractLessThan(Value& result, const Value& l, const Value& r)
{
    Value _1;
    Value _2;

    if (!l.Convert2PrimitiveValue(_1, Value::hintNumber))
        return false;

    if (!r.Convert2PrimitiveValue(_2, Value::hintNumber))
        return false;

    Value::KindType _1k = _1.GetKind();
    Value::KindType _2k = _2.GetKind();

    if (_1k == Value::kInt && _2k == Value::kInt)
    {
        result.SetBool(_1.AsInt() < _2.AsInt());
        return true;
    }

    if (_1k == Value::kUInt && _2k == Value::kUInt)
    {
        result.SetBool(_1.AsUInt() < _2.AsUInt());
        return true;
    }

    if (_1k == Value::kString && _2k == Value::kString)
    {
        const ASString str1 = _1;
        const ASString str2 = _2;

        // Our ASString < operator matches ECMA-262 section 11.8.5.16-20.
        // We don't want to use strcmp() here since it improperly handles \0s in string.
        result.SetBool(str1 < str2);
        return true;
    }

    // (ECMA-262 section 11.8.5.4)
    if (!_1.ToNumberValue())
        return false;
    // (ECMA-262 section 11.8.5.5)
    if (!_2.ToNumberValue())
        return false;

    // (ECMA-262 section 11.8.5.6 - 7)
    if (_1.IsNaN() || _2.IsNaN())
    {
        result.SetUndefined();
        return true;
    }

    // (ECMA-262 section 11.8.5.8)
    if (_1.AsNumber() == _2.AsNumber())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.9)
    if (_1.IsPOSITIVE_ZERO() && _2.IsNEGATIVE_ZERO())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.10)
    if (_1.IsNEGATIVE_ZERO() && _2.IsPOSITIVE_ZERO())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.10)
    if (_1.IsNEGATIVE_ZERO() && _2.IsPOSITIVE_ZERO())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.11)
    if (_1.IsPOSITIVE_INFINITY())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.12)
    if (_2.IsPOSITIVE_INFINITY())
    {
        result.SetBool(true);
        return true;
    }

    // (ECMA-262 section 11.8.5.13)
    if (_2.IsNEGATIVE_INFINITY())
    {
        result.SetBool(false);
        return true;
    }

    // (ECMA-262 section 11.8.5.14)
    if (_1.IsNEGATIVE_INFINITY())
    {
        result.SetBool(true);
        return true;
    }

    // (ECMA-262 section 11.8.5.15)
    result.SetBool(_1.AsNumber() < _2.AsNumber());

    return true;
}

///////////////////////////////////////////////////////////////////////////
bool StrictEqual(const Value& x, const Value& y)
{
    // (ECMA-262 section 11.9.6.1)
    if (x.GetKind() != y.GetKind())
    {
        if (x.IsNumeric() && y.IsNumeric())
        {
            if (x.IsUInt())
            {
                switch (y.GetKind())
                {
                case Value::kInt:
                    if (y.AsInt() < 0)
                        return false;

                    return x.AsUInt() == static_cast<UInt32>(y.AsInt());
                case Value::kNumber:
                    return x.AsUInt() == y.AsNumber();
                default:
                    break;
                }
            }
            else if(x.IsInt())
            {
                switch (y.GetKind())
                {
                case Value::kUInt:
                    if (x.AsInt() < 0)
                        return false;

                    return static_cast<UInt32>(x.AsInt()) == y.AsUInt();
                case Value::kNumber:
                    return x.AsInt() == y.AsNumber();
                default:
                    break;
                }
            }
            else if(x.IsNumber())
            {
                switch (y.GetKind())
                {
                case Value::kInt:
                    return x.AsNumber() == y.AsInt();
                case Value::kUInt:
                    return x.AsNumber() == y.AsUInt();
                default:
                    break;
                }
            }
        } 
        else
            return false;
    }

    switch(x.GetKind())
    {
    case Value::kUndefined:
        // (ECMA-262 section 11.9.6.2)
        return true;
    case Value::kBoolean:
        // (ECMA-262 section 11.9.6.12)
        return x.AsBool() == y.AsBool();
    case Value::kInt:
        return x.AsInt() == y.AsInt();
    case Value::kUInt:
        return x.AsUInt() == y.AsUInt();
    case Value::kNumber:
        // (ECMA-262 section 11.9.6.5 - 6)
        if (x.IsNaN() || y.IsNaN())
            return false;
        // (ECMA-262 section 11.9.6.7)
        if (x.AsNumber() == y.AsNumber())
            return true;
        // (ECMA-262 section 11.9.6.8)
        if (x.IsPOSITIVE_ZERO() && y.IsNEGATIVE_ZERO())
            return true;
        // (ECMA-262 section 11.9.6.9)
        if (x.IsNEGATIVE_ZERO() && y.IsPOSITIVE_ZERO())
            return true;
        return false;
    case Value::kString:
        // (ECMA-262 section 11.9.6.11)
        return x.AsStringNode() == y.AsStringNode();
    case Value::kNamespace:
        return &x.AsNamespace() == &y.AsNamespace();
    case Value::kFunction:
    case Value::kObject:
    case Value::kClass:
    case Value::kThunkFunction:
        // (ECMA-262 section 11.9.6.3)
        if (x.IsNull() || y.IsNull())
            // This is strange.
            return true;

        if (x.GetObject() == y.GetObject() || AreJoined(x, y))
            return true;

        break;
    case Value::kMethodClosure:
        // (ECMA-262 section 11.9.6.3)
        if (x.IsNull() || y.IsNull())
            // This is strange.
            return true;

        if (x.GetClosure() == y.GetClosure() || AreJoined(x, y))
        {
            if (x.GetFunct() == y.GetFunct())
                return true;
        }

        break;
    case Value::kThunkClosure:
        // (ECMA-262 section 11.9.6.3)
        if (x.IsNull() || y.IsNull())
            // This is strange.
            return true;

        if (x.GetClosure() == y.GetClosure() || AreJoined(x, y))
        {
            if (&x.GetThunkFunct() == &y.GetThunkFunct())
                return true;
        }

        break;
    case Value::kThunk:
	return &x.AsThunk() == &y.AsThunk();
    case Value::kMethodInd:
        return x.GetMethodInd() == y.GetMethodInd() && &x.GetAbcFile() == &y.GetAbcFile();
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////
CheckResult AbstractEqual(bool& result, const Value& l, const Value& r)
{
    if (l.GetKind() == r.GetKind())
    {
        switch(l.GetKind())
        {
        case Value::kUndefined:
            result = true;
            return true;
        case Value::kNumber:
            if (l.IsNaN() || r.IsNaN())
            {
                result = false;
                return true;
            }

            if (l.AsNumber() == r.AsNumber())
            {
                result = true;
                return true;
            }

            result = false;
            return true;
        case Value::kString:
            result = l.AsStringNode() == r.AsStringNode();
            return true;
        case Value::kBoolean:
            result = l.AsBool() == r.AsBool();
            return true;
        case Value::kInt:
            result = l.AsInt() == r.AsInt();
            return true;
        case Value::kUInt:
            result = l.AsUInt() == r.AsUInt();
            return true;
        case Value::kThunk:
            result = &l.AsThunk() == &r.AsThunk();
            return true;
        case Value::kMethodClosure:
            result = l.GetClosure() == r.GetClosure() && l.GetFunct() == r.GetFunct();
            return true;
        case Value::kThunkClosure:
            result = l.GetClosure() == r.GetClosure() && &l.GetThunkFunct() == &r.GetThunkFunct();
            return true;
        default:
            break;
        }

        result = (l.GetObject() == r.GetObject() || AreJoined(l, r));

        return true;
    }
    else
    {
        // Combination: Undefined + Null
        {
            if ((l.IsUndefined() && r.IsNull()) ||
                (r.IsUndefined() && l.IsNull()))
            {
                result = true;
                return true;
            }
        }

        // Combination: Number + String
        {
            if (l.IsNumber() && r.IsString())
            {
                Value::Number v;
                if (!r.Convert2Number(v))
                    // Exception.
                    return false;

                result = (l.AsNumber() == v);
                return true;
            }

            if (l.IsString() && r.IsNumber())
            {
                Value::Number v;
                if (!l.Convert2Number(v))
                    // Exception.
                    return false;

                result = (v == r.AsNumber());
                return true;
            }
        }

        // Combination: Undefined + Object
        // !!! NOT IN standard. May not work correctly.
        {
            if (l.IsUndefined() && r.IsObject())
            {
                result = r.IsNull();
                return true;
            }

            if (l.IsObject() && r.IsUndefined())
            {
                result = l.IsNull();
                return true;
            }
        }

        // Combination: Number/String + Object
        {
            if ((l.IsString() || l.IsNumber()) && r.IsObject())
            {
                if (r.IsNull())
                {
                    result = false;
                    return true;
                }

                Value v;
                if (!r.Convert2PrimitiveValue(v))
                    // Exception.
                    return false;

                return AbstractEqual(result, l, v);
            }

            if (l.IsObject() && (r.IsString() || r.IsNumber()))
            {
                if (l.IsNull())
                {
                    result = false;
                    return true;
                }

                Value v;
                if (!l.Convert2PrimitiveValue(v))
                    // Exception.
                    return false;

                return AbstractEqual(result, v, r);
            }
        }

        // Combination: Thunk
        {
//             if (l.IsThunk())
//             {
//                 if (r.IsNullOrUndefined())
//                     return l.AsThunk() == NULL;
// 
//                 return false;
//             } 
//             else if (r.IsThunk())
//             {
//                 if (l.IsNullOrUndefined())
//                     return r.AsThunk() == NULL;
// 
//                 return false;
//             }
            if (l.IsThunk() || r.IsThunk())
            {
                result = false;
                return true;
            }
        }

        // Combination: MethodClosure
        {
            if (l.IsMethodClosure() || r.IsMethodClosure())
            {
                result = false;
                return true;
            }
        }

        // Combination: ThunkClosure
        {
            if (l.IsThunkClosure() || r.IsThunkClosure())
            {
                result = false;
                return true;
            }
        }

        // Combination: Not Number
        {
            if (l.GetKind() != Value::kNumber)
            {
                Value::Number v;
                if (!l.Convert2Number(v))
                    return false;

                return AbstractEqual(result, Value(v), r);
            }

            if (r.GetKind() != Value::kNumber)
            {
                Value::Number v;
                if (!r.Convert2Number(v))
                    return false;

                return AbstractEqual(result, l, Value(v));
            }
        }
    }

    // No exceptions so far.
    return true;
}

Value GetAbsObject(UPInt addr)
{
    Value result;
    Value::ObjectTag tag = (Value::ObjectTag)(addr & 3);

    addr -= tag;
    switch (tag)
    {
    case Value::otObject:
        result = reinterpret_cast<Object*>(addr);
        break;
    case Value::otClass:
        result = reinterpret_cast<Class*>(addr);
        break;
    case Value::otFunction:
        result = reinterpret_cast<Instances::Function*>(addr);
        break;
    case Value::otNamespace:
        result = reinterpret_cast<Instances::Namespace*>(addr);
        break;
    default:
        break;
    }

    return result;
}

}}} // namespace Scaleform { namespace GFx { namespace AS3 {

