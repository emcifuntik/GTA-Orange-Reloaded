/**********************************************************************

Filename    :   Amp_Message.cpp
Content     :   Messages sent back and forth to AMP
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Amp_Message.h"
#include "Kernel/SF_HeapNew.h"
#include "Amp_Stream.h"
#include "Amp_Interfaces.h"

namespace Scaleform {
namespace GFx {
namespace AMP {


// static factory method
Message* Message::CreateMessage(MessageType msgType, MemoryHeap* heap)
{
    Message* message = NULL;
    switch (msgType)
    {
    case Msg_Heartbeat:
        message = SF_HEAP_NEW(heap) MessageHeartbeat();
        break;
    case Msg_Log:
        message = SF_HEAP_NEW(heap) MessageLog();
        break;
    case Msg_CurrentState:
        message = SF_HEAP_NEW(heap) MessageCurrentState();
        break;
    case Msg_ProfileFrame:
        message = SF_HEAP_NEW(heap) MessageProfileFrame();
        break;
    case Msg_SwdFile:
        message = SF_HEAP_NEW(heap) MessageSwdFile();
        break;
    case Msg_SourceFile:
        message = SF_HEAP_NEW(heap) MessageSourceFile();
        break;
    case Msg_SwdRequest:
        message = SF_HEAP_NEW(heap) MessageSwdRequest();
        break;
    case Msg_SourceRequest:
        message = SF_HEAP_NEW(heap) MessageSourceRequest();
        break;
    case Msg_AppControl:
        message = SF_HEAP_NEW(heap) MessageAppControl();
        break;
    case Msg_Port:
        message = SF_HEAP_NEW(heap) MessagePort();
        break;
    case Msg_ImageRequest:
        message = SF_HEAP_NEW(heap) MessageImageRequest();
        break;
    case Msg_ImageData:
        message = SF_HEAP_NEW(heap) MessageImageData();
        break;
    case Msg_FontRequest:
        message = SF_HEAP_NEW(heap) MessageFontRequest();
        break;
    case Msg_FontData:
        message = SF_HEAP_NEW(heap) MessageFontData();
        break;
    default:
        break;
    }
    return message;
}

// static factory method that extracts data from the stream and creates a new message
Message* Message::CreateAndReadMessage(AmpStream& str, MemoryHeap* heap)
{
    MessageType msgType = static_cast<MessageType>(str.ReadUByte());
    UInt32 msgVersion = str.ReadUInt32();
    if (msgVersion > Version_Latest)
    {
        return NULL;
    }

    str.Rewind();

    Message* message = CreateMessage(msgType, heap);
    if (message != NULL)
    {
        message->Read(str);
    }

    return message;
}

// Constructor
Message::Message(MessageType msgType) : MsgType(msgType), Version(Version_Latest) 
{
}

// Virtual method called by the message handler when a message of unknown type needs to be handled
// AcceptHandler in turn calls a message-specific method of the message handler
// This paradigm allows the message handler to process messages in an object-oriented way
// Returns true if the client handles this message
bool Message::AcceptHandler(MsgHandler* handler) const
{
    SF_UNUSED(handler);
    return false;
}

// Serialization
void Message::Read(AmpStream& str)
{
    MsgType = static_cast<MessageType>(str.ReadUByte());
    Version = str.ReadUInt32();
}

// Serialization
void Message::Write(AmpStream& str) const
{
    str.WriteUByte(static_cast<UByte>(MsgType));
    str.WriteUInt32(Version);
}

//////////////////////////////////////////////////////////////////

// Constructor
MessageText::MessageText(MessageType msgType, const String& textValue) : 
    Message(msgType),
    TextValue(textValue)
{
}

// Serialization
void MessageText::Read(AmpStream& str)
{
    Message::Read(str);
    str.ReadString(&TextValue);
}

// Serialization
void MessageText::Write(AmpStream& str) const
{
    Message::Write(str);
    str.WriteString(TextValue);
}

// Text field accessor
const StringLH& MessageText::GetText() const
{
    return TextValue;
}

//////////////////////////////////////////////////////////////////

// Constructor
MessageInt::MessageInt(MessageType msgType, UInt32 intValue) : 
    Message(msgType),
    BaseValue(intValue)
{
}

// Serialization
void MessageInt::Read(AmpStream& str)
{
    Message::Read(str);
    BaseValue = str.ReadUInt32();
}

// Serialization
void MessageInt::Write(AmpStream& str) const
{
    Message::Write(str);
    str.WriteUInt32(BaseValue);
}

// Text field accessor
UInt32 MessageInt::GetValue() const
{
    return BaseValue;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageHeartbeat::MessageHeartbeat() : Message(Msg_Heartbeat)
{
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageLog::MessageLog(const String& logText, UInt32 logCategory, const UInt64 timeStamp) : 
    MessageText(Msg_Log, logText),
    LogCategory(logCategory)
{
    static const int textSize = 9;
    char timeStampText[textSize];
    UInt32 seconds = static_cast<UInt32>(timeStamp % 60);
    UInt32 minutes = static_cast<UInt32>((timeStamp / 60) % 60);
    UInt32 hours = static_cast<UInt32>((timeStamp / 3600) % 24);
    SFsprintf(timeStampText, textSize, "%02u:%02u:%02u", hours, minutes, seconds);
    TimeStamp = timeStampText;
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageLog::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleLog(this);
}

// Serialization
void MessageLog::Read(AmpStream& str)
{
    MessageText::Read(str);
    LogCategory = str.ReadUInt32();
    str.ReadString(&TimeStamp);

    if (Version <= 2)
    {
        for (int i = 0; i < 128; ++i)
        {
            str.ReadUInt32();
        }
    }
}

// Serialization
void MessageLog::Write(AmpStream& str) const
{
    MessageText::Write(str);
    str.WriteUInt32(LogCategory);
    str.WriteString(TimeStamp);

    if (Version <= 2)
    {
        for (int i = 0; i < 128; ++i)
        {
            str.WriteUInt32(0);
        }
    }
}

// Accessor
// Category of the log message. Error, warning, informational, etc
UInt32 MessageLog::GetLogCategory() const
{
    return LogCategory;
}

// Accessor
const StringLH& MessageLog::GetTimeStamp() const
{
    return TimeStamp;
}

/////////////////////////////////////////////////////////////////////////

// Constructor
MessageCurrentState::MessageCurrentState(const ServerState* state) : 
    Message(Msg_CurrentState)
{
    State = *SF_HEAP_AUTO_NEW(this) ServerState();
    if (state != NULL)
    {
        *State = *state;
    }
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageCurrentState::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleCurrentState(this);
}

// Serialization
void MessageCurrentState::Read(AmpStream& str)
{
    Message::Read(str);
    State->Read(str, Version);
}

// Serialization
void MessageCurrentState::Write(AmpStream& str) const
{
    Message::Write(str);
    State->Write(str, Version);
}

/////////////////////////////////////////////////////////////////////////

// Constructor
MessageProfileFrame::MessageProfileFrame(Ptr<ProfileFrame> frameInfo) : 
    Message(Msg_ProfileFrame), 
    FrameInfo(frameInfo)
{
}

MessageProfileFrame::~MessageProfileFrame()
{
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageProfileFrame::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleProfileFrame(this);
}

// Serialization
void MessageProfileFrame::Read(AmpStream& str)
{
    Message::Read(str);
    FrameInfo = *SF_HEAP_AUTO_NEW(this) ProfileFrame();
    FrameInfo->Read(str, Version);
}

// Serialization
void MessageProfileFrame::Write(AmpStream& str) const
{
    Message::Write(str);
    SF_ASSERT(FrameInfo);
    FrameInfo->Write(str, Version);
}

// Accessor
const ProfileFrame* MessageProfileFrame::GetFrameInfo() const
{
    return FrameInfo;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSwdFile::MessageSwdFile(UInt32 swfHandle, UByte* bufferData, 
                               unsigned bufferSize, const char* filename) : 
    MessageInt(Msg_SwdFile, swfHandle), 
    Filename(filename)
{
    FileData.Resize(bufferSize);
    for (UPInt i = 0; i < bufferSize; ++i)
    {
        FileData[i] = *(bufferData + i);
    }
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageSwdFile::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleSwdFile(this);
}

// Serialization
void MessageSwdFile::Read(AmpStream& str)
{
    MessageInt::Read(str);
    UInt32 dataSize = str.ReadUInt32();
    FileData.Resize(dataSize);
    for (UInt32 i = 0; i < dataSize; ++i)
    {
        FileData[i] = str.ReadUByte();
    }
    str.ReadString(&Filename);
}

// Serialization
void MessageSwdFile::Write(AmpStream& str) const
{
    MessageInt::Write(str);

    str.WriteUInt32(static_cast<UInt32>(FileData.GetSize()));
    for (UInt32 i = 0; i < FileData.GetSize(); ++i)
    {
        str.WriteUByte(FileData[i]);
    }
    str.WriteString(Filename);
}


// Accessor
const ArrayLH<UByte>& MessageSwdFile::GetFileData() const
{
    return FileData;
}

// Accessor
const char* MessageSwdFile::GetFilename() const
{
    return Filename;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSourceFile::MessageSourceFile(UInt64 fileHandle, UByte* bufferData, 
                                     unsigned bufferSize, const char* filename) : 
    Message(Msg_SourceFile), 
    FileHandle(fileHandle),
    Filename(filename)
{
    FileData.Resize(bufferSize);
    for (UPInt i = 0; i < bufferSize; ++i)
    {
        FileData[i] = *(bufferData + i);
    }
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageSourceFile::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleSourceFile(this);
}

// Serialization
void MessageSourceFile::Read(AmpStream& str)
{
    Message::Read(str);
    FileHandle = str.ReadUInt64();
    UInt32 dataSize = str.ReadUInt32();
    FileData.Resize(dataSize);
    for (UInt32 i = 0; i < dataSize; ++i)
    {
        FileData[i] = str.ReadUByte();
    }
    str.ReadString(&Filename);
}

// Serialization
void MessageSourceFile::Write(AmpStream& str) const
{
    Message::Write(str);

    str.WriteUInt64(FileHandle);
    str.WriteUInt32(static_cast<UInt32>(FileData.GetSize()));
    for (UInt32 i = 0; i < FileData.GetSize(); ++i)
    {
        str.WriteUByte(FileData[i]);
    }
    str.WriteString(Filename);
}


// Accessor
UInt64 MessageSourceFile::GetFileHandle() const
{
    return FileHandle;
}

// Accessor
const ArrayLH<UByte>& MessageSourceFile::GetFileData() const
{
    return FileData;
}

// Accessor
const char* MessageSourceFile::GetFilename() const
{
    return Filename;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSwdRequest::MessageSwdRequest(UInt32 swfHandle, bool requestContents) : 
    MessageInt(Msg_SwdRequest, swfHandle), 
    RequestContents(requestContents)
{
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageSwdRequest::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleSwdRequest(this);
}

// Serialization
void MessageSwdRequest::Read(AmpStream& str)
{
    MessageInt::Read(str);
    RequestContents = (str.ReadUByte() != 0);
}

// Serialization
void MessageSwdRequest::Write(AmpStream& str) const
{
    MessageInt::Write(str);
    str.WriteUByte(RequestContents ? 1 : 0);
}

// Accessor
bool MessageSwdRequest::IsRequestContents() const
{
    return RequestContents;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageSourceRequest::MessageSourceRequest(UInt64 handle, bool requestContents) : 
    Message(Msg_SourceRequest), 
    FileHandle(handle),
    RequestContents(requestContents)
{
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageSourceRequest::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleSourceRequest(this);
}

// Serialization
void MessageSourceRequest::Read(AmpStream& str)
{
    Message::Read(str);
    FileHandle = str.ReadUInt64();
    RequestContents = (str.ReadUByte() != 0);
}

// Serialization
void MessageSourceRequest::Write(AmpStream& str) const
{
    Message::Write(str);
    str.WriteUInt64(FileHandle);
    str.WriteUByte(RequestContents ? 1 : 0);
}

// Accessor
UInt64 MessageSourceRequest::GetFileHandle() const
{
    return FileHandle;
}

// Accessor
bool MessageSourceRequest::IsRequestContents() const
{
    return RequestContents;
}


////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////

// Constructor
MessageAppControl::MessageAppControl(UInt32 flags) : 
    MessageInt(Msg_AppControl, flags)
{
}

// Virtual method called from Server::HandleNextMessage
// Returns true if the client handles this message
bool MessageAppControl::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleAppControl(this);
}

// Serialization
void MessageAppControl::Read(AmpStream& str)
{
    MessageInt::Read(str);
    str.ReadString(&LoadMovieFile);
}

// Serialization
void MessageAppControl::Write(AmpStream& str) const
{
    MessageInt::Write(str);
    str.WriteString(LoadMovieFile);
}

// Setting this flag toggles wireframe mode on the app
bool MessageAppControl::IsToggleWireframe() const
{
    return (BaseValue & OF_ToggleWireframe) != 0;
}

// Setting this flag toggles wireframe mode on the app
void MessageAppControl::SetToggleWireframe(bool wireframeToggle)
{
    if (wireframeToggle)
    {
        BaseValue |= OF_ToggleWireframe;
    }
    else
    {
        BaseValue &= (~OF_ToggleWireframe);
    }
}

// Setting this flag toggles pause on the movie
bool MessageAppControl::IsTogglePause() const
{
    return (BaseValue & OF_TogglePause) != 0;
}

// Setting this flag toggles pause on the movie
void MessageAppControl::SetTogglePause(bool pauseToggle)
{
    if (pauseToggle)
    {
        BaseValue |= OF_TogglePause;
    }
    else
    {
        BaseValue &= (~OF_TogglePause);
    }
}

// Setting this flag toggles fast forward on the movie
bool MessageAppControl::IsToggleFastForward() const
{
    return (BaseValue & OF_ToggleFastForward) != 0;
}

// Setting this flag toggles fast forward on the movie
void MessageAppControl::SetToggleFastForward(bool ffToggle)
{
    if (ffToggle)
    {
        BaseValue |= OF_ToggleFastForward;
    }
    else
    {
        BaseValue &= (~OF_ToggleFastForward);
    }
}

// Setting this flag toggles between edge anti-aliasing modes
bool MessageAppControl::IsToggleAaMode() const
{
    return (BaseValue & OF_ToggleAaMode) != 0;
}

// Setting this flag toggles between edge anti-aliasing modes
void MessageAppControl::SetToggleAaMode(bool aaToggle)
{
    if (aaToggle)
    {
        BaseValue |= OF_ToggleAaMode;
    }
    else
    {
        BaseValue &= (~OF_ToggleAaMode);
    }
}

// Setting this flag toggles between stroke types
bool MessageAppControl::IsToggleStrokeType() const
{
    return (BaseValue & OF_ToggleStrokeType) != 0;
}

// Setting this flag toggles between stroke types
void MessageAppControl::SetToggleStrokeType(bool strokeToggle)
{
    if (strokeToggle)
    {
        BaseValue |= OF_ToggleStrokeType;
    }
    else
    {
        BaseValue &= (~OF_ToggleStrokeType);
    }
}

// Setting this flag restarts the flash movie on the app
bool MessageAppControl::IsRestartMovie() const
{
    return (BaseValue & OF_RestartMovie) != 0;
}

// Setting this flag restarts the flash movie on the app
void MessageAppControl::SetRestartMovie(bool restart)
{
    if (restart)
    {
        BaseValue |= OF_RestartMovie;
    }
    else
    {
        BaseValue &= (~OF_RestartMovie);
    }
}

// Setting this flag cycles through the font configurations 
bool MessageAppControl::IsNextFont() const
{
    return (BaseValue & OF_NextFont) != 0;
}

// Setting this flag cycles through the font configurations 
void MessageAppControl::SetNextFont(bool next)
{
    if (next)
    {
        BaseValue |= OF_NextFont;
    }
    else
    {
        BaseValue &= (~OF_NextFont);
    }
}

// Setting this flag increases the curve tolerance
bool MessageAppControl::IsCurveToleranceUp() const
{
    return (BaseValue & OF_CurveToleranceUp) != 0;
}

// Setting this flag increases the curve tolerance
void MessageAppControl::SetCurveToleranceUp(bool up)
{
    if (up)
    {
        BaseValue |= OF_CurveToleranceUp;
    }
    else
    {
        BaseValue &= (~OF_CurveToleranceUp);
    }
}

// Setting this flag decreases the curve tolerance
bool MessageAppControl::IsCurveToleranceDown() const
{
    return (BaseValue & OF_CurveToleranceDown) != 0;
}

// Setting this flag decreases the curve tolerance
void MessageAppControl::SetCurveToleranceDown(bool down)
{
    if (down)
    {
        BaseValue |= OF_CurveToleranceDown;
    }
    else
    {
        BaseValue &= (~OF_CurveToleranceDown);
    }
}

bool MessageAppControl::IsForceInstructionProfile() const
{
    return (BaseValue & OF_CurveToleranceDown) != 0;
}

void MessageAppControl::SetForceInstructionProfile(bool instProf)
{
    if (instProf)
    {
        BaseValue |= OF_ForceInstructionProfile;
    }
    else
    {
        BaseValue &= (~OF_ForceInstructionProfile);
    }
}

bool MessageAppControl::IsDebugPause() const
{
    return (BaseValue & OF_DebugPause) != 0;
}

void MessageAppControl::SetDebugPause(bool debug)
{
    if (debug)
    {
        BaseValue |= OF_DebugPause;
    }
    else
    {
        BaseValue &= (~OF_DebugPause);
    }
}

bool MessageAppControl::IsDebugStep() const
{
    return (BaseValue & OF_DebugStep) != 0;
}

void MessageAppControl::SetDebugStep(bool debug)
{
    if (debug)
    {
        BaseValue |= OF_DebugStep;
    }
    else
    {
        BaseValue &= (~OF_DebugStep);
    }
}

bool MessageAppControl::IsDebugStepIn() const
{
    return (BaseValue & OF_DebugStepIn) != 0;
}

void MessageAppControl::SetDebugStepIn(bool debug)
{
    if (debug)
    {
        BaseValue |= OF_DebugStepIn;
    }
    else
    {
        BaseValue &= (~OF_DebugStepIn);
    }
}

bool MessageAppControl::IsDebugStepOut() const
{
    return (BaseValue & OF_DebugStepOut) != 0;
}

void MessageAppControl::SetDebugStepOut(bool debug)
{
    if (debug)
    {
        BaseValue |= OF_DebugStepOut;
    }
    else
    {
        BaseValue &= (~OF_DebugStepOut);
    }
}

bool MessageAppControl::IsDebugNextMovie() const
{
    return (BaseValue & OF_DebugNextMovie) != 0;
}

void MessageAppControl::SetDebugNextMovie(bool debug)
{
    if (debug)
    {
        BaseValue |= OF_DebugNextMovie;
    }
    else
    {
        BaseValue &= (~OF_DebugNextMovie);
    }
}

bool MessageAppControl::IsToggleMemReport() const
{
    return (BaseValue & OF_ToggleMemReport) != 0;

}

void MessageAppControl::SetToggleMemReport(bool toggle)
{
    if (toggle)
    {
        BaseValue |= OF_ToggleMemReport;
    }
    else
    {
        BaseValue &= (~OF_ToggleMemReport);
    }
}

bool MessageAppControl::IsToggleProfileFunctions() const
{
    return (BaseValue & OF_ToggleProfileFunctions) != 0;

}

void MessageAppControl::SetToggleProfileFunctions(bool toggle)
{
    if (toggle)
    {
        BaseValue |= OF_ToggleProfileFunctions;
    }
    else
    {
        BaseValue &= (~OF_ToggleProfileFunctions);
    }
}

// Setting this flag toggles amp sending profile messages every frame
bool MessageAppControl::IsToggleAmpRecording() const
{
    return (BaseValue & OF_ToggleAmpRecording) != 0;
}

// Setting this flag toggles amp sending profile messages every frame
void MessageAppControl::SetToggleAmpRecording(bool recordingToggle)
{
    if (recordingToggle)
    {
        BaseValue |= OF_ToggleAmpRecording;
    }
    else
    {
        BaseValue &= (~OF_ToggleAmpRecording);
    }
}

// Setting this flag toggles amp renderer to draw hot spots
bool MessageAppControl::IsToggleOverdraw() const
{
    return (BaseValue & OF_ToggleOverdraw) != 0;
}

// Setting this flag toggles amp renderer to draw hot spots
void MessageAppControl::SetToggleOverdraw(bool overdrawToggle)
{
    if (overdrawToggle)
    {
        BaseValue |= OF_ToggleOverdraw;
    }
    else
    {
        BaseValue &= (~OF_ToggleOverdraw);
    }
}

// Setting this flag toggles amp renderer to draw hot spots
bool MessageAppControl::IsToggleBatch() const
{
    return (BaseValue & OF_ToggleBatch) != 0;
}

// Setting this flag toggles amp renderer to draw hot spots
void MessageAppControl::SetToggleBatch(bool batchToggle)
{
    if (batchToggle)
    {
        BaseValue |= OF_ToggleBatch;
    }
    else
    {
        BaseValue &= (~OF_ToggleBatch);
    }
}

// Setting this flag toggles instruction profiling for per-line timings
bool MessageAppControl::IsToggleInstructionProfile() const
{
    return (BaseValue & OF_ToggleInstructionProfile) != 0;
}

// Setting this flag toggles instruction profiling for per-line timings
void MessageAppControl::SetToggleInstructionProfile(bool instToggle)
{
    if (instToggle)
    {
        BaseValue |= OF_ToggleInstructionProfile;
    }
    else
    {
        BaseValue &= (~OF_ToggleInstructionProfile);
    }
}


// Accessor
const StringLH& MessageAppControl::GetLoadMovieFile() const
{
    return LoadMovieFile;
}

// Accessor
void MessageAppControl::SetLoadMovieFile(const char* pcFileName)
{
    LoadMovieFile = pcFileName;
}

///////////////////////////////////////////////////////////////////////////////////

MessagePort::MessagePort(UInt32 port, const char* appName, const char* fileName) :
    MessageInt(Msg_Port, port)
{
    if (appName != NULL)
    {
        AppName = appName;
    }
    if (fileName != NULL)
    {
        FileName = fileName;
    }
#if defined(SF_OS_WIN32)
    Platform = PlatformWindows;
#elif defined(SF_OS_LINUX)
    Platform = PlatformLinux;
#elif defined(SF_OS_MAC)
    Platform = PlatformMac;

#elif defined(SF_OS_PS3)
    Platform = PlatformPs3;
#elif defined(SF_OS_XBOX360)
    Platform = PlatformXbox360;
#elif defined(SF_OS_WII)
    Platform = PlatformWii;
#elif defined(SF_OS_3DS)
    Platform = Platform3DS;

#elif defined(SF_OS_ANDROID)
    Platform = PlatformAndroid;
#elif defined(SF_OS_IPHONE)
    Platform = PlatformIphone;

#else
    Platform = PlatformOther;
#endif
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessagePort::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandlePort(this);
}

// Serialization
void MessagePort::Read(AmpStream& str)
{
    MessageInt::Read(str);
    str.ReadString(&AppName);
    if (Version >= 5)
    {
        Platform = static_cast<PlatformType>(str.ReadUInt32());
        str.ReadString(&FileName);
    }
}

// Serialization
void MessagePort::Write(AmpStream& str) const
{
    MessageInt::Write(str);
    str.WriteString(AppName);
    if (Version >= 5)
    {
        str.WriteUInt32(Platform);
        str.WriteString(FileName);
    }
}

// Accessor
const StringLH& MessagePort::GetAppName() const
{
    return AppName;
}

// Accessor
const StringLH& MessagePort::GetFileName() const
{
    return FileName;
}

// Accessor
MessagePort::PlatformType MessagePort::GetPlatform() const
{
    return Platform;
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageImageRequest::MessageImageRequest(UInt32 imageId) : 
    MessageInt(Msg_ImageRequest, imageId)
{
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageImageRequest::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleImageRequest(this);
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageImageData::MessageImageData(UInt32 imageId) : 
    MessageInt(Msg_ImageData, imageId)
{
    ImageDataStream = SF_HEAP_AUTO_NEW(this) AmpStream();
}

// Destructor
MessageImageData::~MessageImageData()
{
    ImageDataStream->Release();
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageImageData::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleImageData(this);
}

// Serialization
void MessageImageData::Read(AmpStream& str)
{
    MessageInt::Read(str);
    str.ReadStream(ImageDataStream);
}

// Serialization
void MessageImageData::Write(AmpStream& str) const
{
    MessageInt::Write(str);
    str.WriteStream(*ImageDataStream);
}


// Accessor
AmpStream* MessageImageData::GetImageData() const
{
    return ImageDataStream;
}

void MessageImageData::SetImageData(AmpStream* imageData)
{
    ImageDataStream->Release();
    ImageDataStream = imageData;
    ImageDataStream->AddRef();
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageFontRequest::MessageFontRequest(UInt32 fontId) : 
    MessageInt(Msg_FontRequest, fontId)
{
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageFontRequest::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleFontRequest(this);
}

////////////////////////////////////////////////////////////////////

// Constructor
MessageFontData::MessageFontData(UInt32 fontId) : 
    MessageInt(Msg_FontData, fontId)
{
    FontDataStream = SF_HEAP_AUTO_NEW(this) AmpStream();
}

// Destructor
MessageFontData::~MessageFontData()
{
    FontDataStream->Release();
}

// Virtual method called from the message handler
// Returns true if the client handles this message
bool MessageFontData::AcceptHandler(MsgHandler* handler) const
{
    return handler->HandleFontData(this);
}

// Serialization
void MessageFontData::Read(AmpStream& str)
{
    MessageInt::Read(str);
    str.ReadStream(FontDataStream);
}

// Serialization
void MessageFontData::Write(AmpStream& str) const
{
    MessageInt::Write(str);
    str.WriteStream(*FontDataStream);
}


// Accessor
AmpStream* MessageFontData::GetImageData() const
{
    return FontDataStream;
}

void MessageFontData::SetImageData(AmpStream* imageData)
{
    FontDataStream->Release();
    FontDataStream = imageData;
    FontDataStream->AddRef();
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

