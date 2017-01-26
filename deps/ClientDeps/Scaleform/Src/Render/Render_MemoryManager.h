/**********************************************************************

PublicHeader:   Render
Filename    :   Render_MemoryManager.h
Content     :   Base MemoryManager used by console RenderHALs.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_MemoryManager_H
#define INC_SF_Render_MemoryManager_H

// Include Image.h since it defines TextureManager.
#include "Render_Image.h"

namespace Scaleform { namespace Render {


// MemoryType describes the type of memory allocation being requested.
// Memory_Normal should be implemented on all platforms, while platform-specific
// types only need to be handled on platforms for which they are defined.

enum MemoryType
{
    Memory_Normal,
    Memory_X360_Physical,
    Memory_PS3_MainVideo,
    Memory_PS3_RSX
};

// ***** MemoryManager

// Renderer MemoryManager is responsible for allocating and freeing video memory,
// including textures. Implementation of this class will typically only be provided
// on consoles, as PC needs to allocate different resources separately.
// The only unified allocation interface is provided by TextureManager, which
// should work with all memory managers.

// TBD:
// In the future this API can be extended with additional functionality
// to support alternative memory management strategies. Possible functionality:
//  - Make allocations movable, requiring them to be locked for access.
//  - If so, report policy: QuickLock, ReadableBuffer, Movable
//  - Notify of Unload Complete (can be used to defrag).
//  - API to move buffer memory (in case of shrink).

class MemoryManager : public NewOverrideBase<Stat_Default_Mem>
{
public:
    virtual ~MemoryManager() {}

    // *** Alloc/Free

    // Allocates renderer memory of specified type.
    virtual void*   Alloc(UPInt size, UPInt align, MemoryType type, unsigned arena = 0) = 0;
    virtual void    Free(void* pmem, UPInt size) = 0;

    // *** Arena Management

    // Arenas used to organize video memory; every allocation is passed
    // arena integer to which it belongs. Each allowed memory types has
    // its own meaning of arena id.  Arena 0 is automatically created for all
    // memory types.
    // Developers can implement CreateArena() to always succeed, while
    // using one shared memory range for all allocations.
    virtual bool    CreateArena(unsigned arena, MemoryType type, UPInt limit)
    { SF_UNUSED3(arena, type, limit); return true; }
    virtual void    DestroyArena(unsigned arena) { SF_UNUSED(arena); }
    // TBD: --> We can have "Render Thread Lag" for cleanup.        
};


}}; // namespace Scaleform::Render

#endif // INC_SF_Render_MemoryManager_H
