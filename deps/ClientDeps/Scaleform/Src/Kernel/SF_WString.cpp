/**********************************************************************

Filename    :   WString.cpp
Content     :   Wide String buffer implementation
modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_WString.h"

namespace Scaleform {

// ***** GFxWStringBuffer class
WStringBuffer::WStringBuffer(const WStringBuffer& other)
    : pText(0), Length(0), Reserved(0,0)
{
    // Our reserve is 0. It's ok, although not efficient. Should we add
    // a different constructor?
    if (other.pText && Resize(other.Length+1))    
        memcpy(pText, other.pText, (other.Length+1)*sizeof(wchar_t));
}

WStringBuffer::~WStringBuffer()
{
    if ((pText != Reserved.pBuffer) && pText)
        SF_FREE(pText);
}

bool     WStringBuffer::Resize(UPInt size)
{    
    if ((size > Length) && (size >= Reserved.Size))
    {
        wchar_t* palloc = (wchar_t*) SF_ALLOC(sizeof(wchar_t)*(size+1), Stat_Default_Mem);
        if (palloc)
        {
            if (pText)
                memcpy(palloc, pText, (Length+1)*sizeof(wchar_t));
            palloc[size] = 0;

            if ((pText != Reserved.pBuffer) && pText)
                SF_FREE(pText);
            pText  = palloc;
            Length = size;
            return true;
        }
        return false;
    }

    if (pText)
        pText[size] = 0;
    Length = size;
    return true;
}


// Assign buffer data.
WStringBuffer& WStringBuffer::operator = (const WStringBuffer& buff)
{
    SetString(buff.pText, buff.Length);
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const String& str)
{
    UPInt size = str.GetLength();
    if (Resize(size) && size)    
        UTF8Util::DecodeString(pText, str.ToCStr(), str.GetSize());
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const char* putf8str)
{
    UPInt size = UTF8Util::GetLength(putf8str);
    if (Resize(size) && size)    
        UTF8Util::DecodeString(pText, putf8str);
    return *this;
}

WStringBuffer& WStringBuffer::operator = (const wchar_t *pstr)
{
    UPInt length = 0;
    for (const wchar_t *pstr2 = pstr; *pstr2 != 0; pstr2++)
        length++;     
    SetString(pstr, length);
    return *this;
}

void WStringBuffer::SetString(const char* putf8str, UPInt utf8Len)
{
    if (utf8Len == SF_MAX_UPINT)
        utf8Len = SFstrlen(putf8str);
    UPInt size = UTF8Util::GetLength(putf8str);
    if (Resize(size) && utf8Len)
        UTF8Util::DecodeString(pText, putf8str, utf8Len);    
}

void WStringBuffer::SetString(const wchar_t* pstr, UPInt length)
{
    if (length == SF_MAX_UPINT)
        length = SFwcslen(pstr);
    if (Resize(length) && length)
        memcpy(pText, pstr, (length+1)*sizeof(wchar_t));    
}

void WStringBuffer::StripTrailingNewLines()
{
    SPInt len = SPInt(Length);
    // check, is the content already null terminated
    if (len > 0 && pText[len -1] == 0)
        --len; //if yes, skip the '\0'
    for (SPInt i = len - 1; i >= 0 && (pText[i] == '\n' || pText[i] == '\r'); --i)
    {
        --Length;
        pText[i] = 0;
    }
}

} // Scaleform
