/**********************************************************************

Filename    :   AS3/IntervalTimer.h
Content     :   SWF (Shockwave Flash) player library
Created     :   Feb, 2010
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFx_AS3_IntervalTimer_H
#define INC_SF_GFx_AS3_IntervalTimer_H

#include "GFx/GFx_Log.h"
#include "GFx/GFx_InteractiveObject.h"
#include "Kernel/SF_Timer.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/AS3/Obj/Utils/AS3_Obj_Utils_Timer.h"

namespace Scaleform { namespace GFx { namespace AS3 {

class IntervalTimer : public ASIntervalTimerIntf
{
    Value                   Function;
    SPtr<Instances::Timer>  TimerObj;
    ArrayLH<Value>          Params;
    unsigned                Interval;
    unsigned                CurrentCount;
    unsigned                RepeatCount;
    UInt64                  InvokeTime;
    int                     Id;
    bool                    Active;
    bool                    Timeout; // indicates to invoke timer only once

    UInt64                  GetNextInterval(UInt64 currentTime, UInt64 frameTime) const;
public:
    IntervalTimer(const Value& function, unsigned delay, bool timeOut = false);
    IntervalTimer(Instances::Timer* timerObj, unsigned delay, unsigned curCount, unsigned repeatCount);

    virtual void            Start(MovieImpl* proot);
    virtual bool            Invoke(MovieImpl* proot, float frameTime);
    virtual bool            IsActive() const { return Active; }
    virtual void            Clear();
    virtual UInt64          GetNextInvokeTime() const { return InvokeTime; }
    virtual void            SetId(int id) { Id = id; }
    virtual int             GetId() const { return Id; }

    void                    SetArguments(unsigned num, const Value* argArr);

    unsigned                GetCurrentCount() const { return CurrentCount; }
};

}}} //SF::GFx::AS3

#endif // INC_SF_GFx_AS3_IntervalTimer_H
