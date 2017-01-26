/**********************************************************************

Filename    :   HeapMH_AllocBitSet2.h
Content     :   "Magic-header based" Bit-set based allocator, 2 bits 
                per block.

Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Kernel_HeapMH_AllocBitSet2_H
#define INC_SF_Kernel_HeapMH_AllocBitSet2_H

#include "../SF_HeapBitSet2.h"
#include "HeapMH_FreeBin.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
class AllocBitSet2MH
{
public:
    AllocBitSet2MH();

    void Reset() { Bin.Reset(); }

    void  InitPage(PageMH* page, UInt32 index);
    void  ReleasePage(UByte* start);

    void* Alloc(UPInt size, MagicHeadersInfo* headers);
    void* Alloc(UPInt size, UPInt alignSize, MagicHeadersInfo* headers);
    void* ReallocInPlace(PageMH* page, void* oldPtr, UPInt newSize, UPInt* oldSize, MagicHeadersInfo* headers);
    void  Free(PageMH* page, void* ptr, MagicHeadersInfo* headers, UPInt* oldBytes);

    UPInt GetUsableSize(const PageMH* page, const void* ptr) const;

private:
    ListBinMH Bin;
};

}} // Scaleform::HeapMH


#endif
