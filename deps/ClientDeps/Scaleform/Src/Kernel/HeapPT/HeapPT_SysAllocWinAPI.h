/**********************************************************************

PublicHeader:   Kernel
Platform    :   WinAPI, XBox360
Filename    :   HeapPT_SysAllocWinAPI.h
Content     :   Win32/XBox360 System Allocator
Created     :   2009
Authors     :   Maxim Shemanarev

Notes       :   Win32 System Allocator that uses VirtualAlloc
                and VirualFree.

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_HeapPT_SysAllocWinAPI_H
#define INC_SF_Kernel_HeapPT_SysAllocWinAPI_H

#include "../SF_SysAlloc.h"
#if (defined SF_OS_XBOX360 || defined SF_OS_WIN32)
namespace Scaleform
{

// ***** SysMemMapperWinAPI
//
// Memory mapper for Windows API. Uses VirtualAlloc/VirtualFree.
//------------------------------------------------------------------------
class SysMemMapperWinAPI : public SysMemMapper
{
public:
    virtual UPInt   GetPageSize() const;

    // Reserve and release address space. The functions must not allocate
    // any actual memory. The size is typically very large, such as 128 
    // megabytes. ReserveAddrSpace() returns a pointer to the reserved
    // space; there is no usable memory is allocated.
    //--------------------------------------------------------------------
    virtual void*   ReserveAddrSpace(UPInt size);
    virtual bool    ReleaseAddrSpace(void* ptr, UPInt size);

    // Map and unmap memory pages to allocate actual memory. The caller 
    // guarantees the integrity of the calls. That is, the the size is 
    // always a multiple of GetPageSize() and the ptr...ptr+size is always 
    // within the reserved address space. Also it's guaranteed that there 
    // will be no MapPages call for already mapped memory and no UnmapPages 
    // for not mapped memory.
    //--------------------------------------------------------------------
    virtual void*   MapPages(void* ptr, UPInt size);
    virtual bool    UnmapPages(void* ptr, UPInt size);
};


// ***** SysAllocWinAPI
//------------------------------------------------------------------------

// SysAllocWinAPI provides a default system allocator implementation for Windows
// and Xbox360 platforms, relying on VirtualAlloc and VirtualFree APIs.
//
// If possible, we recommend that developers use this interface directly instead
// of providing their own SysAlloc implementation. Since SysAllocWinAPI relies on
// the low-level OS functions, it intelligently considers both system page size and
// granularity, allowing it to be particularly alignment friendly. Due to this alignment
// efficiency, SysAllocWinAPI does not lose any memory on 4K alignment required
// internally by MemoryHeap. No alignment overhead means that SysAllocWinAPI uses
// memory and address space with maximum efficiency, making it the best choice for
// the GFx system allocator on Microsoft platforms.

class SysAllocWinAPI : public SysAllocBase_SingletonSupport<SysAllocWinAPI, SysAllocPaged>
{
    // Default segment size to reserve address space. Actual segments
    // can have different size, for example, if a larger continuous
    // block is requested. Or, in case of bad fragmentation, when 
    // ReserveAddrSpace() fails, the mapper can reduce segment size.
    enum { SegSize = 128*1024*1024 };
public:

    // Initializes system allocator.
    SF_EXPORT       SysAllocWinAPI(UPInt granularity = 64*1024, UPInt segSize = SegSize);

    virtual void    GetInfo(Info* i) const;
    virtual void*   Alloc(UPInt size, UPInt align);
    virtual bool    ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align);
    virtual bool    Free(void* ptr, UPInt size, UPInt align);

    virtual UPInt   GetFootprint() const;
    virtual UPInt   GetUsedSpace() const;

    SF_EXPORT const UInt32* GetBitSet(UPInt seg) const; // DBG
    virtual UPInt           GetBase() const;            // DBG

private:
    SysMemMapperWinAPI      Mapper;
    class SysAllocMapper*   pAllocator;
    UPInt                   PrivateData[128];

};

} // Scaleform

#endif      // (defined SF_OS_XBOX360 || defined SF_OS_WIN32)
#endif
