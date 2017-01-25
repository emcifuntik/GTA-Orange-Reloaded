/**********************************************************************

Platform    :   WinAPI, XBox360
Filename    :   SysAllocWinAPI.cpp
Content     :   Win32 System Allocator
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

#include "HeapPT_SysAllocWinAPI.h"
#include "HeapPT_SysAllocMapper.h"
#include "../SF_HeapNew.h"
#ifndef SF_OS_XBOX360
#include <windows.h>
#endif

namespace Scaleform {

//#include <stdio.h>   // DBG
//#include "SF_Memory.h" // DBG


//------------------------------------------------------------------------
UPInt SysMemMapperWinAPI::GetPageSize() const
{
#ifdef SF_OS_XBOX360
    return 4096;
#else
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);
    return info.dwPageSize;
#endif
}

//------------------------------------------------------------------------
void* SysMemMapperWinAPI::ReserveAddrSpace(UPInt size)
{
    return ::VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

//------------------------------------------------------------------------
bool SysMemMapperWinAPI::ReleaseAddrSpace(void* ptr, UPInt)
{
    return ::VirtualFree(ptr, 0, MEM_RELEASE) != 0;
}

//------------------------------------------------------------------------
void* SysMemMapperWinAPI::MapPages(void* ptr, UPInt size)
{
    return ::VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE);
}

//------------------------------------------------------------------------
bool SysMemMapperWinAPI::UnmapPages(void* ptr, UPInt size)
{
    return ::VirtualFree(ptr, size, MEM_DECOMMIT) != 0;
}

//------------------------------------------------------------------------
SysAllocWinAPI::SysAllocWinAPI(UPInt granularity, UPInt segSize) :
    Mapper(),
    pAllocator(::new(PrivateData) SysAllocMapper(&Mapper, segSize, granularity))
{
    SF_COMPILER_ASSERT(sizeof(PrivateData) >= sizeof(SysAllocMapper));
}

//------------------------------------------------------------------------
void SysAllocWinAPI::GetInfo(Info* i) const
{
    pAllocator->GetInfo(i);
}

//------------------------------------------------------------------------
void* SysAllocWinAPI::Alloc(UPInt size, UPInt align)
{
    return pAllocator->Alloc(size, align);
}

//------------------------------------------------------------------------
bool SysAllocWinAPI::ReallocInPlace(void* oldPtr, UPInt oldSize, UPInt newSize, UPInt align)
{
    return pAllocator->ReallocInPlace(oldPtr, oldSize, newSize, align);
}

//------------------------------------------------------------------------
bool SysAllocWinAPI::Free(void* ptr, UPInt size, UPInt align)
{
    return pAllocator->Free(ptr, size, align);
}

//------------------------------------------------------------------------
UPInt SysAllocWinAPI::GetFootprint() const 
{ 
    return pAllocator->GetFootprint(); 
}

//------------------------------------------------------------------------
UPInt SysAllocWinAPI::GetUsedSpace() const 
{ 
    return pAllocator->GetUsedSpace(); 
}

//------------------------------------------------------------------------
const UInt32* SysAllocWinAPI::GetBitSet(UPInt seg) const // DBG
{
    return pAllocator->GetBitSet(seg);
}

UPInt SysAllocWinAPI::GetBase() const
{
    return pAllocator->GetBase();
}

} // Scaleform
