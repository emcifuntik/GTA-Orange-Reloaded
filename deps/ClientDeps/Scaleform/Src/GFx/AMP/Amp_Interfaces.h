/**********************************************************************

PublicHeader:   AMP
Filename    :   Amp_Interfaces.h
Content     :   Interfaces for customizing the behavior of AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INCLUDE_GFX_AMP_INTERFACES_H
#define INCLUDE_GFX_AMP_INTERFACES_H

#include "Amp_Message.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

// 
// Custom AMP behavior is achieved by overriding the classes in this file
//

// Both the AMP client and AMP server are derived from MsgHandler
// so that messages can invoke the proper handler function without knowledge of the 
// concrete handler class, and so that messages can be handled differently
// in the client and server
class MsgHandler
{
public:
    MsgHandler() : RecvPort(0), RecvAddress(0)                          { }
    virtual ~MsgHandler()                                               { }
    virtual bool    HandleSwdFile(const MessageSwdFile*)                { return false; }
    virtual bool    HandleSwdRequest(const MessageSwdRequest*)          { return false; }
    virtual bool    HandleSourceFile(const MessageSourceFile*)          { return false; }
    virtual bool    HandleSourceRequest(const MessageSourceRequest*)    { return false; }
    virtual bool    HandleProfileFrame(const MessageProfileFrame*)      { return false; }
    virtual bool    HandleCurrentState(const MessageCurrentState*)      { return false; }
    virtual bool    HandleLog(const MessageLog*)                        { return false; }
    virtual bool    HandleAppControl(const MessageAppControl*)          { return false; }
    virtual bool    HandlePort(const MessagePort*)                      { return false; }
    virtual bool    HandleImageRequest(const MessageImageRequest*)      { return false; }
    virtual bool    HandleImageData(const MessageImageData*)            { return false; }
    virtual bool    HandleFontRequest(const MessageFontRequest*)        { return false; }
    virtual bool    HandleFontData(const MessageFontData*)              { return false; }
    void SetRecvAddress(UInt32 port, UInt32 address, char* name)
    {
        RecvPort = port;
        RecvAddress = address;
        RecvName = name;
    }
    virtual bool    IsInitSocketLib() const                             { return true; }
protected:
    UInt32  RecvPort;
    UInt32  RecvAddress;
    String  RecvName;
};

// AppControlInterface::HandleAmpRequest is called by Server
// whenever a MessageAppControl has been received
class AppControlInterface
{
public:
    virtual ~AppControlInterface() { }
    virtual bool HandleAmpRequest(const MessageAppControl* pMessage) = 0;
};

// SendInterface::OnSendLoop is called once per "frame" 
// from the ThreadManager send thread
class SendInterface
{
public:
    virtual ~SendInterface() { }
    virtual bool OnSendLoop() = 0;
};

// ConnStatusInterface::OnStatusChanged is called by ThreadManager 
// whenever a change in the connection status has been detected
class ConnStatusInterface
{
public:
    enum StatusType
    {
        CS_Idle         = 0x0,
        CS_Connecting   = 0x1,
        CS_OK           = 0x2,
        CS_Failed       = 0x3,
    };

    virtual ~ConnStatusInterface() { }
    virtual void OnStatusChanged(StatusType newStatus, StatusType oldStatus, const char* message) = 0;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif
