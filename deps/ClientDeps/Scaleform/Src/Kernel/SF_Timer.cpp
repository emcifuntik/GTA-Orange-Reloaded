/**********************************************************************

Filename    :   Timer.cpp
Content     :   Provides static functions for precise timing
Created     :   June 28, 2005
Authors     :   

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "SF_Timer.h"

#if defined (SF_OS_WIN32)
#include <windows.h>

#elif defined(SF_OS_XBOX360)
#include <xtl.h>

#elif defined(SF_OS_PS3)
#include <sys/sys_time.h>

#elif defined(SF_OS_WII)
#include <revolution/os.h>

#elif defined(SF_OS_3DS)
#include <nn/fnd.h>
#include <nn/os.h>

#else
#include <sys/time.h>
#endif

namespace Scaleform {

//------------------------------------------------------------------------
// ***** Timer Class

UInt64 Timer::GetProfileTicks()
{
    return (GetRawTicks() * MksPerSecond) / GetRawFrequency();
}
Double Timer::GetProfileSeconds()
{
    static UInt64 StartTime = GetProfileTicks();
    return TicksToSeconds(GetProfileTicks()-StartTime);
}


//------------------------------------------------------------------------
// *** Win32, Xbox, Xbox360 Specific Timer

#if (defined (SF_OS_WIN32) || defined (SF_OS_XBOX) || defined (SF_OS_XBOX360) || defined(SF_OS_WINCE))

CRITICAL_SECTION WinAPI_GetTimeCS;
volatile UInt32  WinAPI_OldTime = 0;
volatile UInt32  WinAPI_WrapCounter = 0;


UInt32 Timer::GetTicksMs()
{
#if defined (SF_OS_XBOX360) 
    return GetTickCount();
#else
    return timeGetTime();
#endif
}

UInt64 Timer::GetTicks()
{
#if defined (SF_OS_XBOX360) 
    return GetProfileTicks();
#else
    
    DWORD  ticks = timeGetTime();
    UInt64 result;

    // On Win32 QueryPerformanceFrequency is unreliable due to SMP and
    // performance levels, so use this logic to detect wrapping and track
    // high bits.
    ::EnterCriticalSection(&WinAPI_GetTimeCS);

    if (WinAPI_OldTime > ticks)
        WinAPI_WrapCounter++;
    WinAPI_OldTime = ticks;

    result = (UInt64(WinAPI_WrapCounter) << 32) | ticks;
    ::LeaveCriticalSection(&WinAPI_GetTimeCS);

    return result * MksPerMs;
#endif    
}

UInt64 Timer::GetRawTicks()
{
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
}

UInt64 Timer::GetRawFrequency()
{
    static UInt64 perfFreq = 0;
    if (perfFreq == 0)
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        perfFreq = freq.QuadPart;
    }
    return perfFreq;
}

void Timer::initializeTimerSystem()
{
    InitializeCriticalSection(&WinAPI_GetTimeCS);
}
void Timer::shutdownTimerSystem()
{
    DeleteCriticalSection(&WinAPI_GetTimeCS);
}

#else   // !SF_OS_WIN32


//------------------------------------------------------------------------
// *** Other OS Specific Timer     

UInt32 Timer::GetTicksMs()
{
    return (UInt32)(GetProfileTicks() / 1000);
}
// The profile ticks implementation is just fine for a normal timer.
UInt64 Timer::GetTicks()
{
    return GetProfileTicks();
}

void Timer::initializeTimerSystem()
{
}
void Timer::shutdownTimerSystem()
{
}


// System specific GetProfileTicks

#if defined(SF_OS_PS3)

UInt64 Timer::GetRawTicks()
{
    // read time
    UInt64 ticks;
#ifdef SF_CC_SNC
    ticks = __mftb();
#else
    asm volatile ("mftb %0" : "=r"(ticks));
#endif
    return ticks;
}

static bool   bInitialized = false;
static UInt64 iPerfFreq;

UInt64 Timer::GetRawFrequency()
{
    if (!bInitialized)
    {
        iPerfFreq = sys_time_get_timebase_frequency();
    }
    return iPerfFreq;
}

#elif defined(SF_OS_WII)

UInt64 Timer::GetRawTicks()
{
    return OSTicksToMicroseconds(OSGetTime());
}

UInt64 Timer::GetRawFrequency()
{
    return MksPerSecond;
}

#elif defined(SF_OS_3DS)

UInt64 Timer::GetRawTicks()
{
    return nn::fnd::TimeSpan(nn::os::Tick::GetSystemCurrent()).GetMicroSeconds();
}

UInt64 Timer::GetRawFrequency()
{
    return MksPerSecond;
}

// Standard or Other OS 
#else

UInt64  Timer::GetRawTicks()
{
    // TODO: prefer rdtsc when available?

    // Return microseconds.
    struct timeval tv;
    UInt64 result;

    gettimeofday(&tv, 0);

    result = (UInt64)tv.tv_sec * 1000000;
    result += tv.tv_usec;

    return result;
}

UInt64 Timer::GetRawFrequency()
{
    return MksPerSecond;
}

#endif // GetProfileTicks


#endif  // !SF_OS_WIN32



} // Scaleform

