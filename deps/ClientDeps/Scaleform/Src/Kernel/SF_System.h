/**********************************************************************

PublicHeader:   Kernel
Filename    :   SF_System.h
Content     :   General kernel initalization/cleanup, including that
                of the memory allocator.
Created     :   Ferbruary 5, 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_System_H
#define INC_SF_Kernel_System_H

#include "SF_Memory.h"

namespace Scaleform {

// ***** System Core Initialization class

// System initialization must take place before any other SF_Kernel objects are used;
// this is done my calling System::Init(). Among other things, this is necessary to
// initialize the memory allocator. Similarly, System::Destroy must be
// called before program exist for proper clenup. Both of these tasks can be achieved by
// simply creating System object first, allowing its constructor/destructor do the work.

// Note that for GFx use this class super-seeded by the GFxSystem class, which
// should be used instead.

class System
{
public:

    // Two default argument constructors, to allow specifying SF_SysAllocPaged with and without
    // HeapDesc for the root heap.
    SF_EXPORT System(SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton())
    {
        Init(psysAlloc);
    }
    SF_EXPORT System(const MemoryHeap::HeapDesc& rootHeapDesc,
            SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton())
    {
        Init(rootHeapDesc, psysAlloc);
    }

    SF_EXPORT ~System()
    {
        Destroy();
    }

    // Initializes System core, setting the global heap that is needed for GFx
    // memory allocations. Users can override memory heap implementation by passing
    // a different memory heap here.   
    SF_EXPORT static void SF_CDECL Init(const MemoryHeap::HeapDesc& rootHeapDesc,
                     SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton());
    
    SF_EXPORT static void SF_CDECL Init(
                     SysAllocBase* psysAlloc = SF_SYSALLOC_DEFAULT_CLASS::InitSystemSingleton())
    {
        Init(MemoryHeap::RootHeapDesc(), psysAlloc);
    }

    // De-initializes System more, finalizing the threading system and destroying
    // the global memory allocator.
    SF_EXPORT static void SF_CDECL Destroy();
};

} // Scaleform

#endif
