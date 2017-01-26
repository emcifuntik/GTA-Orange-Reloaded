/**********************************************************************

Filename    :   Amp_ThreadMgr.h
Content     :   Manages the socket threads
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_GFX_AMP_THREAD_MGR_H
#define INC_GFX_AMP_THREAD_MGR_H

#include "Amp_Socket.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_String.h"
#include "Amp_Interfaces.h"
#include "GFx/GFx_Log.h"

namespace Scaleform {
    // Forward declarations
    class String;
    class Lock;
    class Event;
namespace GFx {
namespace AMP {

// Manages the socket connection threads for AMP
// An instance of this class is contained both in the AMP server singleton and in the AMP client
// The caller can pass in callback objects for notification
// The SendInterface::OnSendLoop gets called from the socket thread for app-specific processing
// The ConnStatusInterface::OnStatusChanged gets called whenever the connection status is updated
// A MsgHandler can be specified for immediate message handling, bypassing the received queue (for AS debugger)
class ThreadMgr : public RefCountBase<ThreadMgr, Stat_Default_Mem>
{
public:
    ThreadMgr(MsgHandler* msgHandler, SendInterface* sendCallback, 
              ConnStatusInterface* connectionCallback, Scaleform::Event* sendQueueWaitEvent,
              SocketImplFactory* socketFactory);
    virtual ~ThreadMgr();

    // Initialize AMP - Starts the threads for sending and receiving messages
    bool                InitAmp(const char* ipAddress, UInt32 port, UInt32 broadcastPort);
    // Uninitialize AMP - Performs thread cleanup
    void                UninitAmp();
    // Initialize the broadcast receive thread
    void                StartBroadcastRecv(UInt32 port);
    // Set the connected app
    void                SetBroadcastInfo(const char* connectedApp, const char* connectedFile);
    // Heartbeat interval
    void                SetHeartbeatInterval(UInt32 heartbeatInterval);

    // Accessors
    const StringLH&                         GetAddress() const  { return IpAddress; }
    UInt32                                  GetPort() const     { return Port; }
    ConnStatusInterface::StatusType         GetConnectionStatus() const;
    const char*                             GetConnectionMessage() const;
    UInt32                                  GetReceivedQueueSize() const { return MsgReceivedQueue.GetSize(); }
    UInt32                                  GetSendQueueSize() const { return MsgSendQueue.GetSize(); }
    UInt32                                  GetSendRate() const { return SendRate; }
    UInt32                                  GetReceiveRate() const { return ReceiveRate; }

    // Has a socket been successfully created? Thread-safe
    bool                IsValidSocket();
    // Has a message been received from the other end recently? Thread-safe
    bool                IsValidConnection();
    // Place a message on the queue to be sent.  Thread-safe
    void                SendAmpMessage(Message* msg);
    // Send a log message.  Thread-safe
    void                SendLog(const String& logMessage, LogMessageId messageType);
    // Retrieve the next received message from the queue.  Thread-safe
    Ptr<Message>        GetNextReceivedMessage();
    // Clear all messages of the specified type from the received queue
    void                ClearReceivedMessages(const Message* msg);

private:
    
    enum 
    {
        // maximum packet size - larger messages are broken up
        BufferSize = 512,  

        // If no message has been sent for this long, we send a heartbeat message
#ifdef SF_OS_WII
        DefaultHeartbeatIntervalMillisecs = 0, // 0 means no heartbeat
#else
        DefaultHeartbeatIntervalMillisecs = 1000,
#endif
    };

    // MsgQueue encapsulates a message queue
    // It is a thread-safe list that keeps track of its size
    class MsgQueue
    {
    public:
        MsgQueue(Scaleform::Event* sizeEvent, UInt32 sizeCheckHysterisisPercent);
        void            PushBack(Message* msg);
        Message*        PopFront();
        void            Clear();
        void            ClearMsgType(const Message* msg);
        UInt32          GetSize() const;

    private:

        Lock                    QueueLock;
        List<Message>           Queue;
        AtomicInt<UInt32>       Size;
        Scaleform::Event*       SizeEvent;
        UInt32                  SizeCheckHysterisisPercent;

        void                    CheckSize(MemoryHeap* heap);
    };


    Ptr<Thread>         SocketThread;  // Takes messages from the send queue and sends them
    Ptr<Thread>         BroadcastThread;  // Broadcasts listener socket IP address and port 
    Ptr<Thread>         BroadcastRecvThread;  // Listens for broadcast messages 
    UInt32              Port;
    Lock                BroadcastInfoLock;
    UInt32              BroadcastPort;
    StringLH            BroadcastApp;
    StringLH            BroadcastFile;
    UInt32              BroadcastRecvPort;
    StringLH            IpAddress;
    bool                Server;  // Server or Client?
    Lock                SocketLock;
    Socket              Sock;
    UInt32              HeartbeatIntervalMillisecs;

    // Status is checked from send and received threads
    mutable Lock                        InitLock;
    mutable Lock                        StatusLock;
    bool                                Exiting;
    UInt64                              LastSendHeartbeat;
    UInt64                              LastRcvdHeartbeat;
    ConnStatusInterface::StatusType     ConnectionStatus;
    bool                                LastConnected;
    AtomicInt<UInt32>                   SendRate;
    AtomicInt<UInt32>                   ReceiveRate;

    // Message queues
    MsgQueue                            MsgReceivedQueue;
    MsgQueue                            MsgSendQueue;
    Scaleform::Event*                   SendQueueWaitEvent;
    UInt32                              MsgVersion;

    // Callback interfaces
    SendInterface*                      SendCallback;         // send thread callback
    ConnStatusInterface*                ConnectionChangedCallback; // connection changed callback
    MsgHandler*                         Handler;           // message handler for bypassing the received queue
    SocketImplFactory*                  SocketFactory;

    // Thread functions
    static int          SocketThreadLoop(Thread* sendThread, void* param);
    static int          BroadcastThreadLoop(Thread* broadcastThread, void* param);
    static int          BroadcastRecvThreadLoop(Thread* broadcastThread, void* param);

    // Private helper functions
    bool                IsServer() const;
    bool                IsRunning() const;
    bool                IsExiting() const;
    void                SetExiting();
    void                UpdateLastReceivedTime(bool updateStatus);
    bool                SocketConnect(String* errorMsg = NULL);
    Message*            RetrieveMessageFromSendQueue();
    bool                SendReceiveLoop();
    bool                BroadcastLoop();
    bool                BroadcastRecvLoop();
    void                UpdateStatus(ConnStatusInterface::StatusType eStatus, const char* pcMessage);
};

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif // INC_GFX_AMP_THREAD_MGR_H
