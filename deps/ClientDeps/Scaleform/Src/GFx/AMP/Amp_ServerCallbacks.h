/**********************************************************************

Filename    :   Amp_ServerCallbacks.h
Content     :   AMP server interface implementations
Created     :   January 2010
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2010 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_GFXAMP_SEND_THREAD_CALLBACK_H
#define INC_GFXAMP_SEND_THREAD_CALLBACK_H

#include "Kernel/SF_RefCount.h"
#include "Amp_Interfaces.h"

namespace Scaleform {
    class Event;
namespace GFx {
namespace AMP {

// SendThreadCallback::OnSendLoop is called once per "frame" 
// from the GFxAmpThreadManager send thread
//
class SendThreadCallback : 
    public RefCountBase<SendThreadCallback, Stat_Default_Mem>, 
    public SendInterface
{
public:
    virtual ~SendThreadCallback() { }
    virtual bool OnSendLoop();
};

// StatusChangedCallback::OnStatusChanged is called by ThreadManager 
// whenever a change in the connection status has been detected
//
class StatusChangedCallback : 
    public RefCountBase<StatusChangedCallback, Stat_Default_Mem>, 
    public ConnStatusInterface
{
public:
    StatusChangedCallback(Scaleform::Event* connectedEvent = NULL);
    virtual ~StatusChangedCallback() { }
    virtual void OnStatusChanged(StatusType newStatus, StatusType oldStatus, const char* message);
protected:
    Scaleform::Event* ConnectedEvent;   // notifies GFx that connection has been established
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
