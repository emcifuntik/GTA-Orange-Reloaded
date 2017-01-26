/**********************************************************************

PublicHeader:   Kernel
Filename    :   HeapMH_SysAllocMalloc.h
Content     :   System Allocator Interface
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Interface to the system allocator.

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_HeapMH_SysAllocMalloc_H
#define INC_SF_Kernel_HeapMH_SysAllocMalloc_H

#include "../SF_SysAlloc.h"

#if defined(SF_OS_WIN32) || defined(SF_OS_WINCE) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
#include <malloc.h>
#else
#include <stdlib.h>
#endif

namespace Scaleform {

class SysAllocMalloc : public SysAllocBase_SingletonSupport<SysAllocMalloc, SysAlloc>
{
public:
    SysAllocMalloc() {}
    virtual ~SysAllocMalloc() {}

#if defined(SF_OS_WIN32) || defined(SF_OS_WINCE) || defined(SF_OS_XBOX) || defined(SF_OS_XBOX360)
    virtual void* Alloc(UPInt size, UPInt align)
    {
        return _aligned_malloc(size, align);
    }

    virtual void  Free(void* ptr, UPInt size, UPInt align)
    {
        SF_UNUSED2(size, align);
        _aligned_free(ptr);
    }

    virtual void* Realloc(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
    {
        SF_UNUSED(oldSize);
        return _aligned_realloc(oldPtr, newSize, align);
    }
#else
    virtual void* Alloc(UPInt size, UPInt align)
    {
        UPInt ptr = (UPInt)malloc(size+align);
        UPInt aligned = 0;
        if (ptr)
        {
            aligned = (UPInt(ptr) + align-1) & ~(align-1);
            if (aligned == ptr) 
                aligned += align;
            *(((UPInt*)aligned)-1) = aligned-ptr;
        }
        return (void*)aligned;
    }

    virtual void  Free(void* ptr, UPInt size,  UPInt align)
    {
        UPInt src = UPInt(ptr) - *(((UPInt*)ptr)-1);
        free((void*)src);
    }

    virtual void* Realloc(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
    {
        void* newPtr = Alloc(newSize, align);
        if (newPtr)
        {
            memcpy(newPtr, oldPtr, (newSize < oldSize) ? newSize : oldSize);
            Free(oldPtr, oldSize, align);
        }
        return newPtr;
    }
#endif
};


} // Scaleform

#endif
