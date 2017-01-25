/**********************************************************************

Filename    :   HeapMH_AllocEngine.h
Content     :   The main allocation engine
            :   
Created     :   2009
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_HeapMH_AllocEngine_H
#define INC_SF_Kernel_HeapMH_AllocEngine_H

#include "../SF_Atomic.h"
#include "HeapMH_Root.h"
#include "HeapMH_AllocBitSet2.h"

namespace Scaleform { namespace HeapMH {

//------------------------------------------------------------------------
class AllocEngineMH
{
    typedef List<PageMH> PageListType;
public:
    AllocEngineMH(SysAlloc*     sysAlloc,
                  MemoryHeapMH* heap,
                  UPInt         minAlignSize=16,
                  UPInt         limit=0);
    ~AllocEngineMH();

    bool IsValid() const { return true; }

    void FreeAll();

    UPInt SetLimit(UPInt lim)            { return Limit = lim; }
    void  SetLimitHandler(void* handler) { pLimHandler = handler; }

    void*   Alloc(UPInt size, PageInfoMH* info);
    void*   Alloc(UPInt size, UPInt alignSize, PageInfoMH* info);

    void*   ReallocInPage(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo);
    void*   ReallocInNode(NodeMH* node, void* oldPtr, UPInt newSize, PageInfoMH* newInfo);
    void*   ReallocGeneral(PageMH* page, void* oldPtr, UPInt newSize, PageInfoMH* newInfo);
    void*   Realloc(void* oldPtr, UPInt newSize);
    void    Free(PageMH* page, void* ptr);
    void    Free(NodeMH* node, void* ptr);
    void    Free(void* ptr);

    void    GetPageInfo(PageMH* page, PageInfoMH* info) const;
    void    GetPageInfo(NodeMH* node, PageInfoMH* info) const;
    void    GetPageInfoWithSize(PageMH* page, const void* ptr, PageInfoMH* info) const;
    void    GetPageInfoWithSize(NodeMH* node, const void* ptr, PageInfoMH* info) const;

    UPInt   GetFootprint() const { return Footprint; }
    UPInt   GetUsedSpace() const { return UsedSpace; }

    UPInt   GetUsableSize(void* ptr);

private:
    PageMH* allocPage(bool* limHandlerOK);
    void*   allocDirect(UPInt size, UPInt alignSize, bool* limHandlerOK, PageInfoMH* info);
    void    freePage(PageMH* page);

    void*   allocFromPage(UPInt size, PageInfoMH* info);
    void*   allocFromPage(UPInt size, UPInt alignSize, PageInfoMH* info);

    SysAlloc*       pSysAlloc;
    MemoryHeapMH*   pHeap;
    UPInt           MinAlignSize;
    AllocBitSet2MH  Allocator;
    PageListType    Pages;
    UPInt           Footprint;
    UPInt           UsedSpace;
    UPInt           Limit;
    void*           pLimHandler;
    UPInt           UseCount;
};

}} // Scaleform::HeapMH

#endif

