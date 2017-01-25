/**********************************************************************

Filename    :   System.cpp
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

#include "SF_System.h"
#include "SF_Threads.h"
#include "SF_Debug.h"
#include "SF_Timer.h"
#ifdef SF_AMP_SERVER
    #include "GFx/AMP/Amp_Server.h"
#endif

namespace Scaleform {

// *****  GFxSystem Implementation

static SysAllocBase*   System_pSysAlloc = 0;

// Initializes GFxSystem core, setting the global heap that is needed for GFx
// memory allocations.
void System::Init(const MemoryHeap::HeapDesc& rootHeapDesc, SysAllocBase* psysAlloc)
{
    SF_DEBUG_WARNING((System_pSysAlloc != 0), "System::Init failed - duplicate call.");

    if (!System_pSysAlloc)
    {
        Timer::initializeTimerSystem();

        bool initSuccess = psysAlloc->initHeapEngine(&rootHeapDesc);
        SF_ASSERT(initSuccess);
        SF_UNUSED(initSuccess);

        System_pSysAlloc = psysAlloc;
    }
}

void System::Destroy()
{
    SF_DEBUG_WARNING(!System_pSysAlloc, "System::Destroy failed - System not initialized.");
    if (System_pSysAlloc)
    {
        // Wait for all threads to finish; this must be done so that memory
        // allocator and all destructors finalize correctly.
#ifdef SF_ENABLE_THREADS
        Thread::FinishAllThreads();
#endif

        // Report leaks *after* AMP has been uninitialized.
        // shutdownHeapEngine() will report leaks so there is no need to do it explicitly.
        Memory::DetectMemoryLeaks();
        
        // Shutdown heap and destroy SysAlloc singleton, if any.
        System_pSysAlloc->shutdownHeapEngine();
        System_pSysAlloc = 0;
        SF_ASSERT(Memory::GetGlobalHeap() == 0);

        Timer::shutdownTimerSystem();
    }

}

} // Scaleform

