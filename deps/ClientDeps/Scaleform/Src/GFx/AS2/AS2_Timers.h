/**********************************************************************

Filename    :   AS2/AS2_Timers.h
Content     :   SWF (Shockwave Flash) player library
Created     :   August, 2006
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_TIMERS_H
#define INC_SF_GFX_TIMERS_H

#include "GFx/GFx_Log.h"
#include "GFx/AS2/AS2_Action.h"
#include "GFx/GFx_InteractiveObject.h"
#include "Kernel/SF_Timer.h"

namespace Scaleform { namespace GFx { namespace AS2 {

class IntervalTimer : public ASIntervalTimerIntf //, public GNewOverrideBase<StatMV_ActionScript_Mem>
{
    FunctionRef                 Function;
    GASWeakPtr<Object>          pObject;
    WeakPtr<InteractiveObject>  Character;
    ASString                    MethodName;
    ValueArray                  Params;
    unsigned                    Interval;
    UInt64                      InvokeTime;
    int                         Id;
    Ptr<CharacterHandle>        LevelHandle; // used to store handle on _levelN to get environment
    bool                        Active;
    bool                        Timeout; // indicates to invoke timer only once

    UInt64           GetNextInterval(UInt64 currentTime, UInt64 frameTime) const;
public:
    IntervalTimer(const FunctionRef& function, ASStringContext* psc);
    IntervalTimer(Object* object, const ASString& methodName);
    IntervalTimer(InteractiveObject* character, const ASString& methodName);

    void            Start(MovieImpl* proot);
    bool            Invoke(MovieImpl* proot, float frameTime);
    inline bool     IsActive() const { return Active; }
    void            Clear();
    inline UInt64   GetNextInvokeTime() const { return InvokeTime; }

    inline void SetId(int id) { Id = id; }
    inline int  GetId() const { return Id; }

    static void SetInterval(const FnCall& fn);
    static void ClearInterval(const FnCall& fn);

    static void SetTimeout(const FnCall& fn);
    static void ClearTimeout(const FnCall& fn);

private:
    static void Set(const FnCall& fn, bool timeout);
};

}}} //SF::GFx::AS2
#endif // INC_SF_GFX_TIMERS_H
