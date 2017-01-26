/**********************************************************************

Filename    :   GFxAmpSendThreadCallback.cpp
Content     :   AMP server interface implementations
Created     :   January 2010
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2010 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Amp_ServerCallbacks.h"
#include "Amp_Server.h"

#ifndef SF_AMP_SERVER

namespace { char dummy; }; // to disable warning LNK4221 on PC/Xbox

#else

namespace Scaleform {
namespace GFx {
namespace AMP {


// Called from the GFxAmpThreadManager send thread
// Handles messages on the received queue
bool SendThreadCallback::OnSendLoop()
{
    return AmpServer::GetInstance().HandleNextMessage();
}

//////////////////////////////////////////////////////////////

// Constructor
StatusChangedCallback::StatusChangedCallback(Scaleform::Event* connectedEvent)
    : ConnectedEvent(connectedEvent)
{
}

// Called by GFxAmpThreadManager whenever a change in the connection status has been detected
void StatusChangedCallback::OnStatusChanged(StatusType newStatus, StatusType oldStatus, 
                                                  const char* message)
{
    SF_UNUSED(message);

    if (newStatus != oldStatus)
    {
        // Send the current paused/recording state to the player
        if (newStatus == CS_OK)
        {
            AmpServer::GetInstance().SendAppControlCaps();
            AmpServer::GetInstance().SendCurrentState();
        }

        // Signal the server that connection has been established
        // This is used so that the application can wait for a connection
        // during startup, and thus can get profiled from the first frame
        if (ConnectedEvent != NULL)
        {
            if (newStatus == CS_OK)
            {
                ConnectedEvent->SetEvent();
            }
            else
            {
                ConnectedEvent->ResetEvent();
            }
        }
    }
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif   // SF_AMP_SERVER
