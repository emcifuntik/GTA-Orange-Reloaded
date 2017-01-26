/**********************************************************************

Filename    :   ASUtils.h
Content     :   AS Utility functinality
Created     :   Sep 21, 2009
Authors     :   Artyom Bolgar, Sergey Sikorskiy

Notes       :   
History     :   

Copyright   :   (c) 1998-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_ASUtilS_H
#define INC_SF_GFX_ASUtilS_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_String.h"

namespace Scaleform { namespace GFx {

// This namespace contains some static AS-related helper functions
namespace ASUtils
{
    // Escape functions - src length required.
    void EscapeWithMask(const char* psrc, UPInt length, String* pescapedStr, const unsigned* escapeMask);
    void EscapePath(const char* psrc, UPInt length, String* pescapedStr);
    void Escape(const char* psrc, UPInt length, String *pescapedStr);
    void Unescape(const char* psrc, UPInt length, String *punescapedStr);

    // Used in AS3 parseInt, parseDouble and ConvertToNumber
    UInt32 SkipWhiteSpace( const String& str);

    namespace AS3
    {
        // psrc must be an UTF8 encoded string.
        void EscapeWithMask(const char* psrc, UPInt length, String& escapedStr, const unsigned* escapeMask);
        // psrc must be an UTF8 encoded string.
        void Escape(const char* psrc, UPInt length, String& escapedStr);
        // psrc must be an UTF8 encoded string.
        void Unescape(const char* psrc, UPInt length, String& unescapedStr);
    }
}

// This namespace contains some floating point number-related helper functions
namespace NumberUtil {

    // This function is necessary for platforms, which do not have hardware
    // support for double.
    float           ConvertDouble2Float(double v);

    Double SF_CDECL NaN();
    Double SF_CDECL POSITIVE_INFINITY();
    Double SF_CDECL NEGATIVE_INFINITY();
    Double SF_CDECL MIN_VALUE();
    Double SF_CDECL MAX_VALUE();
    Double SF_CDECL POSITIVE_ZERO();
    Double SF_CDECL NEGATIVE_ZERO();

#ifndef SF_NO_DOUBLE
    inline bool SF_STDCALL IsNaN(Double v)
    {
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            Double  D;
        } u;
        u.D = v;
        return ((u.I & SF_UINT64(0x7FF0000000000000)) == SF_UINT64(0x7FF0000000000000) && (u.I & SF_UINT64(0xFFFFFFFFFFFFF)));
    }
    inline bool SF_STDCALL IsPOSITIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            Double  D;
        } u;
        u.D = v;
        return (u.I == SF_UINT64(0x7FF0000000000000));
    }
    inline bool SF_STDCALL IsNEGATIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            Double  D;
        } u;
        u.D = v;
        return (u.I == SF_UINT64(0xFFF0000000000000));
    }
    inline bool SF_STDCALL IsNaNOrInfinity(Double v)
    {
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt64));
        union
        {
            UInt64  I;
            Double  D;
        } u;
        u.D = v;
        return ((u.I & SF_UINT64(0x7FF0000000000000)) == SF_UINT64(0x7FF0000000000000));
    }
#else
    inline bool SF_STDCALL IsNaN(Double v)
    {
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt32));
        union
        {
            UInt32  I;
            float   F;
        } u;
        u.F = v;
        return ((u.I & 0x7F800000u) == 0x7F800000u && (u.I & 0x7FFFFFu));
    }
    inline bool SF_STDCALL IsPOSITIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt32));
        union
        {
            UInt32  I;
            float   F;
        } u;
        u.F = v;
        return (u.I == 0x7F800000u);
    }
    inline bool SF_STDCALL IsNEGATIVE_INFINITY(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt32));
        union
        {
            UInt32  I;
            float   F;
        } u;
        u.F = v;
        return (u.I == 0xFF800000u);
    }
    inline bool SF_STDCALL IsNaNOrInfinity(Double v) 
    { 
        SF_COMPILER_ASSERT(sizeof(Double) == sizeof(UInt32));
        union
        {
            UInt32  I;
            float   F;
        } u;
        u.F = v;
        return ((u.I & 0x7F800000u) == 0x7F800000u);
    }
#endif

    bool SF_STDCALL IsPOSITIVE_ZERO(Double v);
    bool SF_STDCALL IsNEGATIVE_ZERO(Double v);

    enum 
    {
        TOSTRING_BUF_SIZE = 64
    };
    // radixPrecision: > 0  - radix
    //                 < 0  - precision (= -radixPrecision)
    // Returns: a pointer to the first character in the destStr buffer.
    // The recommended size for the destStr buffer is TOSTRING_BUF_SIZE.
    // IMPORTANT: use returned pointer rather than the buffer itself: ToString & IntToString
    // fill buffer from the end to the beginning and DO NOT move the resulting string to the
    // beginning of the buffer; thus, the resulting string may start not from the beginning of
    // the buffer.
    const char* SF_STDCALL ToString(Double value, char destStr[], size_t destStrSize, int radixPrecision = 10);
    // radix - 2, 8, 10, 16
    const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize, int radix);
    // radix is always 10
    const char* SF_STDCALL IntToString(SInt32 value, char destStr[], size_t destStrSize);

    //ECMA-262 implementations
    Double SF_STDCALL StringToInt(const char* str, UInt32 strLen, SInt32 radix, UInt32 *endIndex);
    Double SF_STDCALL StringToDouble(const char* str, UInt32 strLen,  UInt32 *endIndex);
} // NumberUtil

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_ASUtilS_H
