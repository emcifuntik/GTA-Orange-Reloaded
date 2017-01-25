/**********************************************************************

Filename    :   Amp_Socket.h
Content     :   Socket wrapper class

Created     :   July 03, 2009
Authors     :   Boris Rayskiy, Alex Mantzaris

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_GFX_SOCKET_H
#define INC_GFX_SOCKET_H

#include "Kernel/SF_Types.h"

namespace Scaleform {
class String;
class Lock;
namespace GFx {
namespace AMP {

class SocketInterface
{
public:
    virtual ~SocketInterface() { }
    virtual bool CreateStream() = 0;
    virtual bool CreateDatagram() = 0;
    virtual bool Bind() = 0;
    virtual bool Listen(int) = 0;
    virtual bool Connect() = 0;
    virtual bool Accept(int timeout) = 0;
    virtual int Send(const char*, UPInt) const = 0;
    virtual int Receive(char*, int) const = 0;
    virtual int SendBroadcast(const char*, UPInt) const = 0;
    virtual int ReceiveBroadcast(char*, int) const = 0;
    virtual void SetListenPort(UInt32) = 0;
    virtual void SetBroadcastPort(UInt32) = 0;
    virtual void SetAddress(UInt32, const char*) = 0;
    virtual void SetBlocking(bool) = 0;
    virtual void SetBroadcast(bool) = 0;
    virtual void GetName(UInt32*, UInt32*, char*, UInt32) = 0;
    virtual bool Shutdown() = 0;
    virtual bool Startup() = 0;
    virtual void Cleanup() = 0;
    virtual int GetLastError() const = 0;
    virtual bool IsValid() const = 0;
    virtual bool CheckAbort() const = 0;
};

class SocketImplFactory
{
public:
    virtual ~SocketImplFactory() { }
    virtual SocketInterface* Create() = 0;
    virtual void Destroy(SocketInterface* socketImpl) = 0;
};

class Socket
{
public:
    Socket(bool initLib, SocketImplFactory* socketImplFactory);
    ~Socket();

    // Create a socket, either to a specific address and port (client), or a listener (server)
    bool    Create(const char* ipAddress, UInt32 port, String* errorMsg);
    // Shuts down and performs cleanup
    void    Destroy();
    // Wait until an incoming connection is accepted (only relevant for server sockets)
    bool    Accept(int timeout);
    // Send a packet over the network
    int     Send(const char* dataBuffer, UPInt dataBufferSize) const;
    // Receive a packet over the network (blocks)
    int     Receive(char* dataBuffer, int dataSize) const;
    // Test the outgoing connection
    bool    IsConnected() const;
    // Set blocking
    void    SetBlocking(bool blocking);
    // Set a lock for multithreaded access
    void    SetLock(Lock* lock);
    // Is the socket valid?
    bool IsValid() const;
    // Check whether a reconnection should happen
    bool    CheckAbort() const;

private:
    enum  { SocketListenBacklog = 10 };  // Number of simultaneous connections (server only)

    SocketImplFactory*  SocketFactory;
    SocketInterface*    SocketImpl;
    bool                IsServer;
    bool                InitLib;

#ifdef SF_ENABLE_THREADS
    Lock*               CreateLock;
#endif

    // Private helper functions
    bool    Shutdown();
    bool    Startup();
    void    Cleanup();
};

class BroadcastSocket
{
public:
    BroadcastSocket(bool initLib, SocketImplFactory* socketImplFactory);
    ~BroadcastSocket();
    bool    Create(UInt32 port, bool broadcast);
    void    Destroy();
    int     Broadcast(const char* dataBuffer, UPInt dataBufferSize) const;
    int     Receive(char* dataBuffer, int dataSize) const;
    void    GetName(UInt32* port, UInt32* address, char* name, UInt32 nameSize) const;

private:
    SocketImplFactory*  SocketFactory;
    SocketInterface*    SocketImpl;
    bool                InitLib;
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif // INC_GFX_SOCKET_H
