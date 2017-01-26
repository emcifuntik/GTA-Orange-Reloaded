/**********************************************************************

PublicHeader:   None
Filename    :   WString.h
Content     :   Wide String buffer string implementation
modification).
Created     :   April 27, 2007
Authors     :   Ankur Mohan, Michael Antonov

Notes       :
History     :

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDIN
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_WString_H
#define INC_SF_Kernel_WString_H

#include "SF_String.h"

namespace Scaleform {
// ***** GFxWStringBuffer Class

// A class used to store a string buffer of wchar_t. This is used by GFxTranslator
// interface, or when a variable size Unicode character buffer is necessary.

class WStringBuffer
{
public:

    // Reservation header, keeps size and reservation pointer. This structure allows
    // us to not have the rest of the class be a template.
    struct ReserveHeader
    {
        wchar_t* pBuffer;
        UPInt    Size;

        ReserveHeader() : pBuffer(0), Size(0) { }
        ReserveHeader(wchar_t* pbuffer, UPInt size) : pBuffer(pbuffer), Size(size) { }
        ReserveHeader(const ReserveHeader &other) : pBuffer(other.pBuffer), Size(other.Size) { }
    };
    // Buffer data reservation object. Creates a buffer of fixed size,
    // must be passed on constructor.
    template<unsigned int size>
    struct Reserve : public ReserveHeader
    {
        wchar_t  Buffer[size];
        Reserve() : ReserveHeader(Buffer, size) { }
    };

private:
    wchar_t*      pText;
    UPInt         Length;
    ReserveHeader Reserved;

public:

    WStringBuffer()
        : pText(0), Length(0), Reserved(0,0) { }
    WStringBuffer(const ReserveHeader& reserve)
        : pText(reserve.pBuffer), Length(0), Reserved(reserve) { }
    WStringBuffer(const WStringBuffer& other);

    ~WStringBuffer();

    // Resize the buffer to desired size.
    bool     Resize(UPInt size);
    // Resets buffer contents to empty.
    void     Clear() { Resize(0); }

    inline const wchar_t* ToWStr() const { return (pText != 0) ? pText : L""; }
    inline UPInt    GetLength() const    { return Length; }
    inline wchar_t* GetBuffer() const    { return pText; }

    // Character access and mutation.
    inline wchar_t operator [] (UPInt index) const { SF_ASSERT(pText && (index <= Length)); return pText[index]; }
    inline wchar_t& operator [] (UPInt index)            { SF_ASSERT(pText && (index <= Length)); return pText[index]; }

    // Assign buffer data.
    WStringBuffer& operator = (const WStringBuffer& buff);
    WStringBuffer& operator = (const String& str);
    WStringBuffer& operator = (const wchar_t *pstr);
    WStringBuffer& operator = (const char* putf8str);

    void ResizeInternal(UPInt _size, bool flag = false);

    void SetString(const wchar_t* pstr, UPInt length = SF_MAX_UPINT);
    void SetString(const char* putf8str, UPInt utf8Len = SF_MAX_UPINT);

    void StripTrailingNewLines();
};


template <int sz>
class WString_Reserve
{
protected:
    WStringBuffer::Reserve<sz> Res;
};

template <int sz>
class WStringBufferReserve : WString_Reserve<sz>, public WStringBuffer
{   
public:
    WStringBufferReserve(const char* putf8str) : WString_Reserve<sz>(), WStringBuffer(WString_Reserve<sz>::Res) 
    { 
        *(WStringBuffer*)this = putf8str; 
    }
    WStringBufferReserve(const String &str) : WString_Reserve<sz>(), WStringBuffer(WString_Reserve<sz>::Res) 
    { 
        *(WStringBuffer*)this = str; 
    }
};

} // Scaleform

#endif
