/**********************************************************************

PublicHeader:   Kernel
Filename    :   HeapNew.h
Content     :   Defines heap allocation macros such as SF_HEAP_NEW, etc.
Created     :   September 9, 2008
Authors     :   Michael Antonov, Maxim Shemanarev

Notes       :   This file should be included in internal headers and
                source files but NOT the public headers because it
                #undefines operator new. It is separated from Memory
                for this reason.

Copyright   :   (c) 2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_Memory.h"

// Operator 'new' must NOT be defined, or the macros below will not compile
// correctly (compiler would complain about the number of macro arguments).
#undef new

#ifndef INC_SF_Kernel_HeapNew_H
#define INC_SF_Kernel_HeapNew_H


// Define heap macros
#ifdef SF_MEMORY_ENABLE_DEBUG_INFO

#define SF_HEAP_NEW(heap)        new(heap,__FILE__,__LINE__)
#define SF_HEAP_NEW_ID(heap,id)  new(heap, id, __FILE__,__LINE__)
#define SF_HEAP_AUTO_NEW(addr)   new((MemAddressPtr)((void*)addr),__FILE__,__LINE__)
// Global new
#define SF_NEW                    new(__FILE__,__LINE__)

#else

#define SF_HEAP_NEW(pheap)        new(pheap)
#define SF_HEAP_NEW_ID(pheap,id)  new(pheap,id)
#define SF_HEAP_AUTO_NEW(paddr)   new((MemAddressPtr)((void*)paddr))
#define SF_NEW                    new

#endif 


#endif // INC_SF_HEAPNEW_H
