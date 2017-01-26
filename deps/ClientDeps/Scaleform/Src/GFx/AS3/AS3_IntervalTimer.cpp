/**********************************************************************

Filename    :   AS3/IntervalTimer.cpp
Content     :   
Created     :   Feb, 2010
Authors     :   Artyom Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_IntervalTimer.h"

namespace Scaleform { namespace GFx { namespace AS3 {

IntervalTimer::IntervalTimer(const Value& function, unsigned delay, bool timeOut) :
    Function(function), Interval(delay), CurrentCount(0), RepeatCount(0), InvokeTime(0), Id(0), 
    Active(true), Timeout(timeOut)
{
}

IntervalTimer::IntervalTimer
(Instances::Timer* timerObj, unsigned delay, unsigned curCount, unsigned repeatCount) 
:
    TimerObj(timerObj), Interval(delay), CurrentCount(curCount), RepeatCount(repeatCount), 
    InvokeTime(0), Id(0), Active(true), Timeout(false)
{
}

void IntervalTimer::SetArguments(unsigned num, const Value* argArr)
{
    for (unsigned i = 0; i < num; ++i)
    {
        Params.PushBack(argArr[i]);
    }
}

void IntervalTimer::Start(MovieImpl* proot)
{
    SF_ASSERT(proot);

    UInt64 startTime = proot->GetTimeElapsedMs();
    InvokeTime = startTime + Interval;
}

bool IntervalTimer::Invoke(MovieImpl* proot, float frameTime)
{
    SF_AMP_SCOPE_TIMER(proot->AdvanceStats, "IntervalTimer::Invoke");

    SF_ASSERT(proot);
    if (!Active) return false;
    MovieRoot* asroot = ToAS3Root(proot);
    //GFx::AS2::MovieRoot* asroot = static_cast<GFx::AS2::MovieRoot*>(proot->pASMovieRoot);

    UInt64 currentTime  = proot->GetTimeElapsedMs();
    bool retval         = false;
    if (currentTime >= InvokeTime)
    {
        if (TimerObj)
        {
            // invoke Timer's listeners
            if (RepeatCount == 0 || CurrentCount < RepeatCount)
            {
                ++CurrentCount;
                TimerObj->ExecuteEvent();
            }
        }
        else
        {
            // invoke a function
            Value _this;
            Value result;
            asroot->GetAVM()->Execute_IgnoreException(
                Function, _this, result, Params.GetSize(), Params.GetDataPtr());            
        }

        if (Timeout)
        {
            // clear the timeout timer
            Active = false;
        }
        else
        {
            // set new invoke time
            UInt64 interval = GetNextInterval(currentTime, UInt64(frameTime * 1000));
            if (interval > 0)
                InvokeTime += interval;
            else
                InvokeTime = currentTime; //?
        }
        retval = true;
    }
    return retval;
}

UInt64 IntervalTimer::GetNextInterval(UInt64 currentTime, UInt64 frameTime) const
{
    if (RepeatCount != 0 && CurrentCount >= RepeatCount)
        return 0;

    int interval; 
    if (Interval < frameTime/10) // make sure to have not more than 10 calls a frame
        interval = unsigned(frameTime/10);
    else
        interval = Interval;
    if (interval == 0)
        return 0;
    return (((currentTime - InvokeTime) + interval)/interval)*interval;
}

void IntervalTimer::Clear()
{
    Active   = false;
    TimerObj = NULL; // remove circular reference
}

}}} //SF::GFx::AS3
