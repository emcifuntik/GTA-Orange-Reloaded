/**********************************************************************

PublicHeader:   Kernel
Filename    :   SysAllocMalloc.h
Content     :   Malloc System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   System Allocator that uses regular malloc/free

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_SysAllocMalloc_H
#define INC_SF_Kernel_SysAllocMalloc_H

#include "../SF_SysAlloc.h"

namespace Scaleform {

// ***** SysAllocPagedMalloc
//
//------------------------------------------------------------------------
class SysAllocPagedMalloc : public SysAllocBase_SingletonSupport<SysAllocPagedMalloc, SysAllocPaged>
{
public:
    enum { MinGranularity = 64*1024 };

    SF_EXPORT SysAllocPagedMalloc(UPInt granularity = MinGranularity);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual UPInt   GetFootprint() const { return Footprint; }
    virtual UPInt   GetUsedSpace() const { return Footprint; }

    virtual UPInt   GetBase() const; // DBG

private:
    UPInt   Granularity;
    UPInt   Footprint;
    UPInt   Base; // DBG
};

} // Scaleform

#endif
