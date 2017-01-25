/**********************************************************************

Filename    :   Amp_ThreadMgr.cpp
Content     :   Manages the socket threads
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef SF_ENABLE_THREADS

#include "Amp_ThreadMgr.h"
#include "Amp_Stream.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_Debug.h"

#include <time.h>

#define MICROSECONDS_PER_SECOND 1000000

namespace Scaleform {
namespace GFx {
namespace AMP {


// Static 
// Send thread function
int ThreadMgr::SocketThreadLoop(Thread* sendThread, void* param)
{
    SF_UNUSED(sendThread);

    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    while (threadMgr->SendReceiveLoop())
    {
        // SendLoop exited but we are not shutting down
        // re-try to establish connection

        Thread::Sleep(1);  // seconds
    }

    return 0;
}

int ThreadMgr::BroadcastThreadLoop(Thread*, void* param)
{
    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    threadMgr->BroadcastLoop();

    return 0;
}

int ThreadMgr::BroadcastRecvThreadLoop(Thread*, void* param)
{
    ThreadMgr* threadMgr = static_cast<ThreadMgr*>(param);
    if (threadMgr == NULL)
    {
        return 1;
    }

    threadMgr->BroadcastRecvLoop();

    return 0;
}

// Constructor
ThreadMgr::ThreadMgr(MsgHandler* msgHandler, 
                                 SendInterface* sendCallback, 
                                 ConnStatusInterface* connectionCallback,
                                 Scaleform::Event* sendQueueWaitEvent,
                                 SocketImplFactory* socketFactory) : 
    SocketThread(NULL),
    BroadcastThread(NULL),
    BroadcastRecvThread(NULL),
    Port(0),
    BroadcastPort(0),
    BroadcastRecvPort(0),
    Server(true),
    Sock(msgHandler->IsInitSocketLib(), socketFactory),
    HeartbeatIntervalMillisecs(DefaultHeartbeatIntervalMillisecs),
    Exiting(true),
    LastSendHeartbeat(0),
    LastRcvdHeartbeat(0),
    ConnectionStatus(ConnStatusInterface::CS_Idle),
    LastConnected(false),
    SendRate(0),
    ReceiveRate(0),
    MsgReceivedQueue(NULL, 0),
    MsgSendQueue(sendQueueWaitEvent, 90),
    SendQueueWaitEvent(sendQueueWaitEvent),
    MsgVersion(Message::GetLatestVersion()),
    SendCallback(sendCallback),
    ConnectionChangedCallback(connectionCallback),
    Handler(msgHandler),
    SocketFactory(socketFactory)
{
    Sock.SetLock(&SocketLock);
}

// Destructor
ThreadMgr::~ThreadMgr()
{
    UninitAmp();
}

// Initialization
// Returns true if successful
// A NULL IP address means that we are going to be a server
bool ThreadMgr::InitAmp(const char* address, UInt32 port, UInt32 broadcastPort)
{
    Lock::Locker locker(&InitLock);

    // Have we already been initialized?
    if (IsRunning())
    {
        if ((IsServer() ? address == NULL : IpAddress == address) && port == Port)
        {
            // Same connection information - done
            return true;
        }
        else
        {
            // Different connection information - start over
            UninitAmp();
        }
    }

    Exiting = false;
    Port = port;
    BroadcastPort = broadcastPort;
    Server = (address == NULL);
    if (!IsServer())
    {
        IpAddress = address;
    }

    // Start the send thread
    if (Port != 0)
    {
        SocketThread = *SF_HEAP_AUTO_NEW(this) Thread(ThreadMgr::SocketThreadLoop, (void*) this);
        if (!SocketThread || !SocketThread->Start())
        {
            return false;
        }
    }

    StartBroadcastRecv(BroadcastRecvPort);

    return true;
}

// Cleanup
void ThreadMgr::UninitAmp()
{
    Lock::Locker locker(&InitLock);

    SetExiting();

    // wait until threads are done
    if (BroadcastThread)
    {
        BroadcastThread->Wait();
        BroadcastThread = NULL;
    }

    if (BroadcastRecvThread)
    {
        BroadcastRecvThread->Wait(); 
        BroadcastRecvThread = NULL;
    }

    if (SocketThread)
    {
        SocketThread->Wait();
        SocketThread = NULL;
    }

    MsgSendQueue.Clear();
    MsgReceivedQueue.Clear();
}

// Initialize the broadcast thread
void ThreadMgr::StartBroadcastRecv(UInt32 port)
{
    BroadcastRecvPort = port;
    if (BroadcastRecvPort != 0 && !BroadcastRecvThread)
    {
        BroadcastRecvThread = *SF_HEAP_AUTO_NEW(this) Thread(
                                        ThreadMgr::BroadcastRecvThreadLoop, (void*) this);
        if (BroadcastRecvThread)
        {
            Lock::Locker locker(&StatusLock);
            Exiting = false;

            BroadcastRecvThread->Start();
        }
    }
}

void ThreadMgr::SetBroadcastInfo(const char* connectedApp, const char* connectedFile)
{
    Lock::Locker locker(&BroadcastInfoLock);
    BroadcastApp = connectedApp;
    BroadcastFile = connectedFile;
}

void ThreadMgr::SetHeartbeatInterval(UInt32 heartbeatInterval)
{
    HeartbeatIntervalMillisecs = heartbeatInterval;
}

// Returns true when there is a thread still running
bool ThreadMgr::IsRunning() const
{
    if (SocketThread && !SocketThread->IsFinished())
    {
        return true;
    }

    return false;
}

// Thread-safe status accessor
bool ThreadMgr::IsExiting() const
{
    Lock::Locker locker(&StatusLock);
    return Exiting;
}

// Signals the threads that we are shutting down
void ThreadMgr::SetExiting()
{
    Lock::Locker locker(&StatusLock);
    Exiting = true;
    Sock.Destroy();
}

// Keep track of last message received for connection state
void ThreadMgr::UpdateLastReceivedTime(bool updateStatus)
{
    if (updateStatus)
    {
        UpdateStatus(ConnStatusInterface::CS_OK, "");
    }

    Lock::Locker locker(&StatusLock);
    LastRcvdHeartbeat = Timer::GetTicks();
}

// Create the socket
// Returns true on success
bool ThreadMgr::SocketConnect(String* errorMsg)
{
    Lock::Locker kLocker(&StatusLock);

    // Check for exit
    // This needs to be inside SocketConnect for thread safety
    // Otherwise we could exit and destroy the socket after entering this function
    if (IsExiting())
    {
        Sock.Destroy();
        return false;
    }

    UpdateStatus(ConnStatusInterface::CS_Connecting, "");

    if (IsServer())
    {
        if (!Sock.Create(NULL, Port, errorMsg))
        {
            SetExiting();  // Something is terribly wrong, don't keep trying
            return false;
        }
    }
    else
    {
        if (!Sock.Create(IpAddress.ToCStr(), Port, errorMsg))
        {
            //
            // PPS: Remove setting CS_Failed since SendLoop tries to reconnect forever. 
            //      Semantics for CS_Failed should be defined, since the UI is expected
            //      to handle this status differently than CS_Connecting.
            //
            // if (errorMsg != NULL)
            // {
            //    UpdateStatus(GFxAmpConnStatusInterface::CS_Failed, *errorMsg);
            // }
            return false;
        }

        // If the client succeeds in creating the socket, it means we have a valid connection
        // Update the received time even though we have not yet received a message

        // We don't do this for the server until a connection has been accepted
        UpdateLastReceivedTime(false);
    }

    if (BroadcastPort != 0)
    {
        if (!BroadcastThread)
        {
            // Start the broadcast thread
            BroadcastThread = *SF_HEAP_AUTO_NEW(this) Thread(ThreadMgr::BroadcastThreadLoop, (void*) this);
            if (BroadcastThread)
            {
                BroadcastThread->Start();
            }
        }
    }

    return true;
}

ConnStatusInterface::StatusType ThreadMgr::GetConnectionStatus() const
{
    Lock::Locker locker(&StatusLock);
    return ConnectionStatus;
}

bool ThreadMgr::IsValidSocket()
{
    Lock::Locker locker(&StatusLock);
    return Sock.IsValid();
}

// Socket status
// Notifies connection interface on loss of connection
// Sends a log message on connection
// We are considered connected if we have received a message 
// in the last 2 * HeartbeatIntervalMillisecs
bool ThreadMgr::IsValidConnection()
{
    Lock::Locker locker(&StatusLock);

    UInt64 ticks = Timer::GetTicks();

    bool connected = (LastRcvdHeartbeat != 0);
    if (HeartbeatIntervalMillisecs > 0)
    {
        UInt64 deltaTicks = ticks - LastRcvdHeartbeat;
        connected = (deltaTicks < 2 * HeartbeatIntervalMillisecs * 1000);
    }

    //SF_DEBUG_MESSAGE1(!connected, "Lost connection after %d milliseconds", (ticks - LastRcvdHeartbeat) / 1000);

    if (!connected)
    {
        // Notify that we have lost connection
        UpdateStatus(ConnStatusInterface::CS_Connecting, NULL);
    }

    if (!LastConnected && connected)
    {
        // Send a log message to announce that we have connected
        String strMessage;
        SPrintF(strMessage, "Established AMP connection on port %d\n", Port);
        SendLog(strMessage, Log_Message);
    }
    LastConnected = connected;

    return connected;
}

// For different behavior between server and client sockets
bool ThreadMgr::IsServer() const
{
    return Server;
}

// Reads a message from the queue
// Returns true if a message was retrieved
Message* ThreadMgr::RetrieveMessageFromSendQueue()
{
    UInt64 ticks = Timer::GetTicks();

    // Recover the next message, if any
    Message* msg = MsgSendQueue.PopFront();

    if (msg == NULL)
    {
        // Nothing to send. Should we send a heartbeat?
        UInt64 deltaTicks = ticks - LastSendHeartbeat;
        if (HeartbeatIntervalMillisecs > 0 && deltaTicks >  HeartbeatIntervalMillisecs * 1000)
        {
            msg = SF_HEAP_AUTO_NEW(this) MessageHeartbeat();
        }
    }

    if (msg != NULL)
    {
        LastSendHeartbeat = ticks;  // update the last message send time
    }

    return msg;
}

// Socket thread loop
// Returns true if exited with a broken connection
// Returns false if shutting down
bool ThreadMgr::SendReceiveLoop()
{
    if (SendQueueWaitEvent != NULL)
    {
        SendQueueWaitEvent->SetEvent();
    }

    // Create a socket
    while (SocketConnect())
    {
        // wait until a connection has been established with the client
        if (Sock.Accept(1))
        {
            Sock.SetBlocking(false);

            // Send a heartbeat message first for version checking
            MsgVersion = Message::GetLatestVersion();
            SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageHeartbeat());

            Ptr<AmpStream> strmReceived = *SF_HEAP_AUTO_NEW(this) AmpStream();
            char bufferReceived[BufferSize];

            // Signal to the connection status that we are good to go
            UpdateLastReceivedTime(true);

            UInt64 lastSampleTime = Timer::GetProfileTicks();
            UInt32 bytesSent = 0;
            UInt32 bytesReceived = 0;
            Ptr<AmpStream> streamSend = *SF_HEAP_AUTO_NEW(this) AmpStream();
            UPInt streamSendDataLeft = streamSend->GetBufferSize();
            const char* sendBuffer = NULL;

            // Keep sending messages from the send queue
            while (!IsExiting() && IsValidConnection() && !Sock.CheckAbort())
            {
                bool actionPerformed = false;

                // Check for a callback
                if (SendCallback != NULL)
                {
                    if (SendCallback->OnSendLoop())
                    {
                        actionPerformed = true;
                    }
                }

                if (streamSendDataLeft == 0)
                {
                    // Retrieve the next message from the send queue
                    Ptr<Message> msg = *RetrieveMessageFromSendQueue();
                    if (msg)
                    {
                        streamSend = *SF_HEAP_AUTO_NEW(this) AmpStream();
                        msg->SetVersion(MsgVersion);
                        msg->Write(*streamSend);
                        streamSendDataLeft = streamSend->GetBufferSize();
                        sendBuffer = reinterpret_cast<const char*>(streamSend->GetBuffer());
                    }
                }

                if (streamSendDataLeft > 0)
                {
                    UPInt nextSendSize;
                    if (streamSendDataLeft <= BufferSize)
                    {
                        // The message fits in one packet
                        nextSendSize = streamSendDataLeft;
                    }
                    else
                    {
                        // The message does not fit. 
                        // Send the first BUFFER_SIZE bytes in this packet, 
                        // and the rest in the next one
                        nextSendSize = BufferSize;
                    }

                    // Send the packet
                    int sentBytes = Sock.Send(sendBuffer, nextSendSize);
                    if (sentBytes > 0)
                    {
                        //SF_DEBUG_MESSAGE3(true, "Sent %d/%d of %s", sentBytes, streamDataSizeLeft, msg->ToString().ToCStr());
                        bytesSent += sentBytes;
                        sendBuffer += sentBytes;
                        streamSendDataLeft -= sentBytes;
                        actionPerformed = true;
                    }
                    else if (sentBytes < 0)
                    {
                        // Failed to send. Retry
                    }
                }

                // Check for incoming messages and place on the received queue
                int packetSize = Sock.Receive(bufferReceived, BufferSize);
                if (packetSize > 0)
                {
                    bytesReceived += packetSize;
                    actionPerformed = true;

                    // add packet to previously-received incomplete data
                    strmReceived->Append(reinterpret_cast<UByte*>(bufferReceived), packetSize);

                    //SF_DEBUG_MESSAGE3(true, "Received %u/%u bytes (%u)", packetSize, strmReceived->FirstMessageSize(), strmReceived->GetBufferSize());
                }

                UPInt readBufferSize = strmReceived->GetBufferSize();
                if (readBufferSize > 0 && readBufferSize >= strmReceived->FirstMessageSize())
                {
                    UpdateLastReceivedTime(false);

                    // Read the stream and create a message
                    Message* msg = Message::CreateAndReadMessage(*strmReceived, Memory::GetHeapByAddress(this));

                    // Update the buffer to take into account what we already read
                    strmReceived->PopFirstMessage();

                    if (msg != NULL)
                    {
                        if (msg->GetVersion() < MsgVersion)
                        {
                            MsgVersion = msg->GetVersion();
                        }

                        //SF_DEBUG_MESSAGE1(true, "Received %s", msg->ToString().ToCStr());

                        if (msg->ShouldQueue() || Handler == NULL)
                        {
                            // Place the new message on the received queue
                            MsgReceivedQueue.PushBack(msg);
                        }
                        else
                        {
                            // Handle immediately
                            // bypass the queue
                            msg->AcceptHandler(Handler);
                            msg->Release();
                        }
                    }
                    else
                    {
                        SF_DEBUG_MESSAGE(1, "Corrupt message received");
                    }
                }

                if (!actionPerformed)
                {
                    Thread::MSleep(10);  // Don't hog the CPU
                }

                // Update the byte rates
                UInt64 nextTicks = Timer::GetProfileTicks();
                UInt32 deltaTicks = static_cast<UInt32>(nextTicks - lastSampleTime);
                if (deltaTicks > MICROSECONDS_PER_SECOND) // one second
                {
                    SendRate = bytesSent * MICROSECONDS_PER_SECOND / deltaTicks;
                    ReceiveRate = bytesReceived * MICROSECONDS_PER_SECOND / deltaTicks;
                    lastSampleTime = nextTicks;
                    bytesSent = 0;
                    bytesReceived = 0;
                    //SF_DEBUG_MESSAGE2(true, "Send: %d bytes/s, Receive: %d bytes/s", SendRate, ReceiveRate);
                }                
            }
        }

        Sock.Destroy();
    }

    return !IsExiting();
}

// UDP socket broadcast loop
bool ThreadMgr::BroadcastLoop()
{
    BroadcastSocket broadcastSocket(Handler->IsInitSocketLib(), SocketFactory);

    if (!broadcastSocket.Create(BroadcastPort, true))
    {
        return false;
    }

    while (!IsExiting())
    {
        if (!IsValidConnection())
        {
            Ptr<AmpStream> stream = *SF_HEAP_AUTO_NEW(this) AmpStream();
            BroadcastInfoLock.DoLock();
            Ptr<MessagePort> msg = *SF_HEAP_AUTO_NEW(this) MessagePort(Port, BroadcastApp, BroadcastFile);
            BroadcastInfoLock.Unlock();
            msg->Write(*stream);

            broadcastSocket.Broadcast(
                reinterpret_cast<const char*>(stream->GetBuffer()), 
                stream->GetBufferSize());
        }

        Thread::Sleep(1);
    }

    return true;
}

// UDP socket broadcast receive loop
bool ThreadMgr::BroadcastRecvLoop()
{
    BroadcastSocket broadcastSocket(Handler->IsInitSocketLib(), SocketFactory);
    char bufferReceived[BufferSize];

    if (!broadcastSocket.Create(BroadcastRecvPort, false))
    {
        return false;
    }

    while (!IsExiting())
    {
        int packetSize = broadcastSocket.Receive(bufferReceived, BufferSize);
        if (packetSize > 0)
        {
            Ptr<AmpStream> strmReceived = *SF_HEAP_AUTO_NEW(this) AmpStream(
                                            reinterpret_cast<UByte*>(bufferReceived), packetSize);
            SF_ASSERT(strmReceived->FirstMessageSize() == static_cast<UPInt>(packetSize));
            if (strmReceived->FirstMessageSize() == static_cast<UPInt>(packetSize))
            {
                UInt32 port;
                UInt32 address;
                char name[256];
                broadcastSocket.GetName(&port, &address, name, 256);
                Handler->SetRecvAddress(port, address, name);

                // Read the stream and create a message
                Ptr<Message> msg = *Message::CreateAndReadMessage(
                                                *strmReceived, Memory::GetHeapByAddress(this));
                if (msg)
                {
                    msg->AcceptHandler(Handler);
                }
            }
        }
        else
        {
            Thread::MSleep(100);
        }
    }

    return true;
}

// Places a message on the send queue
// after serializing it into a stream
void ThreadMgr::SendAmpMessage(Message* msg)
{
    MsgSendQueue.PushBack(msg);
}

// Sends a log message
// This is a convenience function that just calls SendAmpMessage
void ThreadMgr::SendLog(const String& messageText, LogMessageId messageType)
{
    // create time stamp for log message
    time_t rawTime;
    time(&rawTime);

    // Send a message with the appropriate category for filtering
    SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageLog(messageText, messageType, rawTime));
}

// Retrieves a message from the received queue
// Returns NULL if there is no message to retrieve
Ptr<Message> ThreadMgr::GetNextReceivedMessage()
{
    return *MsgReceivedQueue.PopFront();
}

// Clears all messages of the specified type from the received queue
// If the input parameter is NULL, the it clears all the messages
void ThreadMgr::ClearReceivedMessages(const Message* msg)
{
    MsgReceivedQueue.ClearMsgType(msg);
}

// Notify the callback object that the connection status has changed
void ThreadMgr::UpdateStatus(ConnStatusInterface::StatusType status, const char* statusMsg)
{
    Lock::Locker lock(&StatusLock);

    if (ConnectionStatus != status)
    {
        ConnStatusInterface::StatusType oldStatus = ConnectionStatus;
        ConnectionStatus = status;

        if (ConnectionChangedCallback != NULL)
        {
            ConnectionChangedCallback->OnStatusChanged(status, oldStatus, statusMsg);
        }
    }
}

///////////////////////////////////////////////////////////////////

ThreadMgr::MsgQueue::MsgQueue(Scaleform::Event* sizeEvent, UInt32 sizeCheckHysterisisPercent) : 
    Size(0), 
    SizeEvent(sizeEvent),
    SizeCheckHysterisisPercent(sizeCheckHysterisisPercent)
{
    if (SizeEvent != NULL)
    {
        SizeEvent->SetEvent();
    }
}


void ThreadMgr::MsgQueue::PushBack(Message* msg)
{
    Lock::Locker locker(&QueueLock);
    Queue.PushBack(msg);
    ++Size;
    CheckSize(Memory::GetHeapByAddress(msg));
}

Message* ThreadMgr::MsgQueue::PopFront()
{
    Message* msg = NULL;
    Lock::Locker locker(&QueueLock);
    if (!Queue.IsEmpty())
    {
        msg = Queue.GetFirst();
        MemoryHeap* heap = Memory::GetHeapByAddress(msg);
        Queue.Remove(msg);
        --Size;
        CheckSize(heap);
    }
    return msg;
}

void ThreadMgr::MsgQueue::Clear()
{
    Lock::Locker locker(&QueueLock);
    while (!Queue.IsEmpty())
    {
        Message* msg = Queue.GetFirst();
        Queue.Remove(msg);
        msg->Release();
    }
    Size = 0;
    if (SizeEvent != NULL)
    {
        SizeEvent->SetEvent();
    }
}

void ThreadMgr::MsgQueue::ClearMsgType(const Message* msg)
{
    Lock::Locker locker(&QueueLock);
    Message* msgQueued = Queue.GetFirst();
    UPInt queueInitSize = Size;
    for (UPInt i = 0; i < queueInitSize; ++i)
    {
        if (msg == NULL || msgQueued->IsSameType(*msg))
        {
            Message* msgNext = Queue.GetNext(msgQueued);
            MemoryHeap* heap = Memory::GetHeapByAddress(msgNext);
            Queue.Remove(msgQueued);
            msgQueued->Release();
            msgQueued = msgNext;
            --Size;
            CheckSize(heap);
        }
    }    
}

UInt32 ThreadMgr::MsgQueue::GetSize() const
{
    return Size;
}

void ThreadMgr::MsgQueue::CheckSize(MemoryHeap* heap)
{
    if (SizeEvent != NULL && heap->GetLimit() > 0)
    {
        if (100 * heap->GetFootprint() < heap->GetLimit() * SizeCheckHysterisisPercent)
        {
            SizeEvent->SetEvent();
        }
        else if (heap->GetFootprint() > heap->GetLimit())
        {
            SizeEvent->ResetEvent();
        }
    }
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // GFC_NO_THREADSUPPORT
