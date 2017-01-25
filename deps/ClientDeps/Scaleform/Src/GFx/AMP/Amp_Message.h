/**********************************************************************

PublicHeader:   AMP
Filename    :   Amp_Message.h
Content     :   Messages sent back and forth to AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

// This file includes a number of different classes
// All of them derive from GFxMessage and represent the types of messages
// sent back and forth between AMP and the GFx application

#ifndef INCLUDE_GFX_AMP_MESSAGE_H
#define INCLUDE_GFX_AMP_MESSAGE_H

#include "Amp_ProfileFrame.h"

namespace Scaleform {
namespace GFx {
namespace AMP {

class AmpStream;
class MsgHandler;

// Generic message that includes only the type
// Although not abstract, this class is intended as a base class for concrete messages
class Message : public RefCountBase<Message, Stat_Default_Mem>, public ListNode<Message>
{
public:
    virtual ~Message() { }

    // Called by the message handler when a message of unknown type needs to be handled
    // AcceptHandler in turn calls a message-specific method of the message handler
    // This paradigm allows the message handler to process messages in an object-oriented way
    virtual bool            AcceptHandler(MsgHandler* msgHandler) const;

    // Urgent messages can be handled as they are received, bypassing the queue
    virtual bool            ShouldQueue() const     { return true; }

    // Message serialization methods
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Message printing for debugging
    virtual String          ToString() const        { return "Unknown message"; }

    // Type comparison for queue filtering
    bool        IsSameType(const Message& msg) const  { return MsgType == msg.MsgType; }

    // version for backwards-compatibility
    void        SetVersion(UInt32 version)                  { Version = version; }
    UInt32      GetVersion() const                          { return Version; }

    // Static method that reads a stream and creates a message of the appropriate type
    static Message*     CreateAndReadMessage(AmpStream& str, MemoryHeap* heap);

    // Version for backwards compatibility
    static UInt32       GetLatestVersion()          { return Version_Latest; }

protected:

    enum MessageType
    {
        Msg_None,
        Msg_Heartbeat,

        // Server to client only
        Msg_Log,
        Msg_CurrentState,
        Msg_ProfileFrame,
        Msg_SwdFile,
        Msg_SourceFile,

        // Client to server only
        Msg_SwdRequest,
        Msg_SourceRequest,
        Msg_AppControl,

        // Broadcast
        Msg_Port,

        // Image
        Msg_ImageRequest,
        Msg_ImageData,
        Msg_FontRequest,
        Msg_FontData,
    };

    enum VersionType
    {
        Version_Latest = 17
    };

    MessageType                 MsgType;  // for serialization
    UInt32                      Version;

    // Protected constructor because Message is intended to be used as a base class only
    Message(MessageType msgType = Msg_None);

    // Static helper that creates a message of the specified type
    static Message* CreateMessage(MessageType msgType, MemoryHeap* heap);
};


// Message that includes a text field
// Intended as a base class
class MessageText : public Message
{
public:

    virtual ~MessageText() { }

    // Serialization
    virtual void        Read(AmpStream& str);
    virtual void        Write(AmpStream& str) const;

    // Accessor
    const StringLH&     GetText() const;

protected:

    StringLH            TextValue;

    // Protected constructor because MessageText is intended as a base class
    MessageText(MessageType msgType, const String& textValue = "");
};

// Message that includes an integer field
// Intended as a base class
class MessageInt : public Message
{
public:
    virtual ~MessageInt() { }

    // Serialization
    virtual void        Read(AmpStream& str);
    virtual void        Write(AmpStream& str) const;

    // Accessor
    UInt32              GetValue() const;

protected:

    UInt32 BaseValue;

    // Protected constructor because MessageInt is intended as a base class
    MessageInt(MessageType msgType, UInt32 intValue = 0);
};

// Heartbeat message
// Used for connection verification
// A lack of received messages for a long time period signifies lost connection
// Not handled by the message handler, but at a lower level by the thread manager
class MessageHeartbeat : public Message
{
public:

    MessageHeartbeat();
    virtual ~MessageHeartbeat() { }

    virtual String          ToString() const        { return "Heartbeat"; }
};

// Log message
// Sent from server to client
class MessageLog : public MessageText
{
public:

    MessageLog(const String& logText = "", UInt32 logCategory = 0, const UInt64 timeStamp = 0);
    virtual ~MessageLog() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;
    virtual String          ToString() const        { return "Log"; }

    // Accessors
    UInt32                  GetLogCategory() const;
    const StringLH&         GetTimeStamp() const;

protected:

    UInt32                  LogCategory;
    StringLH                TimeStamp;
};

// Message describing AMP current state
// Sent from server to client
// The state is stored in the base class member
class MessageCurrentState : public Message
{
public:
    MessageCurrentState(const ServerState* state = NULL);
    virtual ~MessageCurrentState() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;
    virtual bool            ShouldQueue() const     { return false; }
    virtual String          ToString() const        { return "Current state"; }

    // Accessors
    const ServerState*   GetCurrentState() const   { return State; }

protected:
    Ptr<ServerState>    State;
};

// Message describing a single frame's profiling results
// Sent by server to client every frame
class MessageProfileFrame : public Message
{
public:

    MessageProfileFrame(Ptr<ProfileFrame> frameInfo = NULL);
    virtual ~MessageProfileFrame();

    // Message overrides
    virtual bool                AcceptHandler(MsgHandler* handler) const;
    virtual String              ToString() const        { return "Frame data"; }
    virtual void                Read(AmpStream& str);
    virtual void                Write(AmpStream& str) const;

    // Data Accessor
    const ProfileFrame*         GetFrameInfo() const;

protected:

    Ptr<ProfileFrame>          FrameInfo;
};


// Message with the contents of a SWD file
// The contents are not parsed but are sent in the form of an array of bytes
// Sent by server to client upon request
// The base class member holds the corresponding SWF file handle, generated by the server
class MessageSwdFile : public MessageInt
{
public:

    MessageSwdFile(UInt32 swfHandle = 0, UByte* bufferData = NULL, unsigned bufferSize = 0, 
                                                                   const char* filename = "");
    virtual ~MessageSwdFile() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "SWD file"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    const ArrayLH<UByte>&   GetFileData() const;
    const char*             GetFilename() const;

protected:

    ArrayLH<UByte>          FileData;
    StringLH                Filename;
};

// Message containing ActionScript source code
// Sent by server to client upon request
class MessageSourceFile : public Message
{
public:

    MessageSourceFile(UInt64 fileHandle = 0, UByte* bufferData = NULL, 
                      unsigned bufferSize = 0, const char* filename = "");
    virtual ~MessageSourceFile() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Source file"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    UInt64                  GetFileHandle() const;
    const ArrayLH<UByte>&   GetFileData() const;
    const char*             GetFilename() const;

protected:

    UInt64                  FileHandle;
    ArrayLH<UByte>          FileData;
    StringLH                Filename;
};



// Sent by client to server
// Base class holds the SWF handle
class MessageSwdRequest : public MessageInt
{
public:

    MessageSwdRequest(UInt32 swfHandle = 0, bool requestContents = false);
    virtual ~MessageSwdRequest() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "SWD request"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    bool                    IsRequestContents() const;

protected:

    // may just request filename for a given SWF handle, or the entire SWD file
    bool                    RequestContents; 
};

// Requests a source file corresponding to a given handle
// Sent by client to server
class MessageSourceRequest : public Message
{
public:

    MessageSourceRequest(UInt64 handle = 0, bool requestContents = false);
    virtual ~MessageSourceRequest() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Source request"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    UInt64                  GetFileHandle() const;
    bool                    IsRequestContents() const;

protected:

    UInt64                  FileHandle;
    bool                    RequestContents; 
};


// Message that controls the GFx application in some way
// Normally sent by client to server
// Server sends this to client to signify supported capabilities
// Control information stored as bits in the base class member
class MessageAppControl : public MessageInt
{
public:

    MessageAppControl(UInt32 flags = 0);
    virtual ~MessageAppControl() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "App control"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    bool                    IsToggleWireframe() const;
    void                    SetToggleWireframe(bool wireframeToggle);
    bool                    IsTogglePause() const;
    void                    SetTogglePause(bool pauseToggle);
    bool                    IsToggleAmpRecording() const;
    void                    SetToggleAmpRecording(bool recordingToggle);
    bool                    IsToggleOverdraw() const;
    void                    SetToggleOverdraw(bool overdrawToggle);
    bool                    IsToggleBatch() const;
    void                    SetToggleBatch(bool batchToggle);
    bool                    IsToggleInstructionProfile() const;
    void                    SetToggleInstructionProfile(bool instToggle);
    bool                    IsToggleFastForward() const;
    void                    SetToggleFastForward(bool ffToggle);
    bool                    IsToggleAaMode() const;
    void                    SetToggleAaMode(bool aaToggle);
    bool                    IsToggleStrokeType() const;
    void                    SetToggleStrokeType(bool strokeToggle);
    bool                    IsRestartMovie() const;
    void                    SetRestartMovie(bool restart);
    bool                    IsNextFont() const;
    void                    SetNextFont(bool next);
    bool                    IsCurveToleranceUp() const;
    void                    SetCurveToleranceUp(bool up);
    bool                    IsCurveToleranceDown() const;
    void                    SetCurveToleranceDown(bool down);
    bool                    IsForceInstructionProfile() const;
    void                    SetForceInstructionProfile(bool instProf);
    bool                    IsDebugPause() const;
    void                    SetDebugPause(bool debug);
    bool                    IsDebugStep() const;
    void                    SetDebugStep(bool debug);
    bool                    IsDebugStepIn() const;
    void                    SetDebugStepIn(bool debug);
    bool                    IsDebugStepOut() const;
    void                    SetDebugStepOut(bool debug);
    bool                    IsDebugNextMovie() const;
    void                    SetDebugNextMovie(bool debug);
    bool                    IsToggleMemReport() const;
    void                    SetToggleMemReport(bool toggle);
    bool                    IsToggleProfileFunctions() const;
    void                    SetToggleProfileFunctions(bool toggle);
    const StringLH&         GetLoadMovieFile() const;
    void                    SetLoadMovieFile(const char* fileName);

protected:

    enum OptionFlags
    {
        OF_ToggleWireframe              = 0x00000001,
        OF_TogglePause                  = 0x00000002,
        OF_ToggleAmpRecording           = 0x00000004,
        OF_ToggleOverdraw               = 0x00000008,
        OF_ToggleInstructionProfile     = 0x00000010,
        OF_ToggleFastForward            = 0x00000020,
        OF_ToggleAaMode                 = 0x00000040,
        OF_ToggleStrokeType             = 0x00000080,
        OF_RestartMovie                 = 0x00000100,
        OF_NextFont                     = 0x00000200,
        OF_CurveToleranceUp             = 0x00000400,
        OF_CurveToleranceDown           = 0x00000800,
        OF_ForceInstructionProfile      = 0x00001000,
        OF_DebugPause                   = 0x00002000,
        OF_DebugStep                    = 0x00004000,
        OF_DebugStepIn                  = 0x00008000,
        OF_DebugStepOut                 = 0x00010000,
        OF_DebugNextMovie               = 0x00020000,
        OF_ToggleMemReport              = 0x00040000,
        OF_ToggleBatch                  = 0x00080000,
        OF_ToggleProfileFunctions       = 0x00100000
    };

    StringLH                LoadMovieFile;
};

// Message containing the server listening port
// Broadcast via UDP
// The server IP address is known from the origin of the UDP packet
class MessagePort : public MessageInt
{
public:
    enum PlatformType
    {
        PlatformOther = 0,

        PlatformWindows,
        PlatformMac,
        PlatformLinux,

        PlatformXbox360,
        PlatformPs3,
        PlatformWii,
        Platform3DS,

        PlatformAndroid,
        PlatformIphone,
    };

    MessagePort(UInt32 port = 0, const char* appName = NULL, const char* FileName = NULL);
    virtual ~MessagePort() { }

    // Message overrides
    virtual bool        AcceptHandler(MsgHandler* handler) const;
    virtual void        Read(AmpStream& str);
    virtual void        Write(AmpStream& str) const;

    // Accessors
    PlatformType        GetPlatform() const;
    const StringLH&     GetAppName() const;
    const StringLH&     GetFileName() const;

protected:
    PlatformType        Platform;
    StringLH            AppName;
    StringLH            FileName;
};

// Sent by client to server
// Base class holds the Image ID
class MessageImageRequest : public MessageInt
{
public:
    MessageImageRequest(UInt32 imageId = 0);
    virtual ~MessageImageRequest() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Image request"; }
};

// Message with the contents of an image
// Sent by server to client upon request
// The base class member holds the corresponding image ID
class MessageImageData : public MessageInt
{
public:
    MessageImageData(UInt32 imageId = 0);
    virtual ~MessageImageData();

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Image data"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    AmpStream*              GetImageData() const;
    void                    SetImageData(AmpStream* imageData);

protected:
    AmpStream*              ImageDataStream;
};

// Sent by client to server
// Base class holds the Image ID
class MessageFontRequest : public MessageInt
{
public:
    MessageFontRequest(UInt32 fontId = 0);
    virtual ~MessageFontRequest() { }

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Font request"; }
};

// Message with the contents of an image
// Sent by server to client upon request
// The base class member holds the corresponding image ID
class MessageFontData : public MessageInt
{
public:
    MessageFontData(UInt32 fontId = 0);
    virtual ~MessageFontData();

    // Message overrides
    virtual bool            AcceptHandler(MsgHandler* handler) const;
    virtual String          ToString() const        { return "Font data"; }
    virtual void            Read(AmpStream& str);
    virtual void            Write(AmpStream& str) const;

    // Accessors
    AmpStream*              GetImageData() const;
    void                    SetImageData(AmpStream* imageData);

protected:
    AmpStream*              FontDataStream;
};


} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif

