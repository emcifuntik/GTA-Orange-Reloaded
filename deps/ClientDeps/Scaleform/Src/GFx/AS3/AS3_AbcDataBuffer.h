/**********************************************************************

Filename    :   AbcDataBuffer.h
Content     :   Implementation of buffer that holds ABC data from SWF
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_AS3_AbcDataBuffer_H
#define INC_AS3_AbcDataBuffer_H

#include "GFx/GFx_Types.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

class AbcDataBuffer : public RefCountBase<AbcDataBuffer, StatMD_ActionOps_Mem>
{
public:
    String      Name;
    unsigned    DataSize;
    UByte       Flags;
    SF_AMP_CODE(UInt32 SwdHandle;)
    SF_AMP_CODE(int SwfOffset;)

    enum 
    {
        Flags_None      = 0x0,
        Flags_LazyInit  = 0x1
    };
#ifdef SF_BUILD_DEBUG
    StringLH    FileName;
#endif
    UByte       Data[1];

    AbcDataBuffer(const String& name, unsigned flags, unsigned dataSize) 
        : Name(name), DataSize(dataSize), Flags((UByte)flags)
#ifdef SF_AMP_SERVER
        , SwdHandle(0), SwfOffset(0)
#endif
    {}

    void*   operator new        (UPInt n, void *ptr)   { SF_UNUSED(n); return ptr; }
    void    operator delete     (void *ptr, void *ptr2) { SF_UNUSED2(ptr,ptr2); }

    bool                IsNull() const          { return DataSize == 0; }
    unsigned            GetLength() const       { return DataSize; }
    const UByte*        GetBufferPtr() const    { return (IsNull()) ? NULL : Data; }

    const char*         GetFileName() const    
    { 
#ifdef SF_BUILD_DEBUG
        return FileName.ToCStr();
#else
        return NULL;
#endif
    }

    SF_MEMORY_REDEFINE_NEW(AbcDataBuffer, StatMD_ActionOps_Mem);                                  \
};

}}} // Scaleform::GFx::AS3

#endif // INC_AS3_AbcDataBuffer_H

