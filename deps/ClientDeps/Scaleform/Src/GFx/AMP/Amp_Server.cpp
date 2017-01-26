/**********************************************************************

Filename    :   Server.cpp
Content     :   Class encapsulating communication with AMP
                Embedded in application to be profiled
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"

#ifdef SF_AMP_SERVER

#include "Amp_Server.h"
#include "Kernel/SF_HeapNew.h"
#include "Amp_ThreadMgr.h"
#include "GFx/GFx_ASMovieRootBase.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Amp_ViewStats.h"
#include "Amp_ServerCallbacks.h"
#include "Amp_Visitors.h"
#include "Amp_Stream.h"
#include "Kernel/SF_MsgFormat.h"
#include "Kernel/SF_MemItem.h"
#include "Render/ImageFiles/PNG_ImageFile.h"
#include "Render/Renderer2DImpl.h"
#include "Render/Render_GlyphCache.h"


namespace Scaleform {
namespace GFx {
namespace AMP {

// Static
// AMP Server initialization
void Server::Init()
{
    MemoryHeap* heap = Memory::GetGlobalHeap()->CreateHeap("AMP", MemoryHeap::Heap_UserDebug);
    heap->SetLimit(1024 * 1024);
    Server* serverInstance = SF_HEAP_NEW(heap) Server();
    AmpServerSingleton = serverInstance;
    heap->ReleaseOnFree(serverInstance);
}

// Static
// AMP Server uninitialization
void Server::Uninit()
{
    static_cast<Server*>(AmpServerSingleton)->Release();
}

////////////////////////////////////////////////

// Returns true if there is a valid socket connection to the AMP client
bool Server::IsValidConnection() const
{
    return (SocketThreadMgr && SocketThreadMgr->IsValidConnection());
}

// State accessor
// Checks for paused or disabled state
bool Server::IsState(ServerStateType state) const
{
    Lock::Locker locker(&CurrentStateLock);
    return ((CurrentState.StateFlags & state) != 0);
}

// State accessor
// Sets the specified state (paused, disabled, etc)
void Server::SetState(ServerStateType state, bool stateValue)
{
    if (stateValue != IsState(state))
    {
        ToggleAmpState(state);
    }
}

// Toggles the specified state, allowing multiple states to be set or unset at once
void Server::ToggleAmpState(UInt32 toggleState)
{
    Lock::Locker locker(&CurrentStateLock);
    CurrentState.StateFlags ^= toggleState;

    if (toggleState != 0)
    {
        if ((Amp_Disabled & toggleState) != 0)
        {
            if (IsState(Amp_Disabled))
            {
                CloseConnection();
            }
            else
            {
                OpenConnection();
            }            
        }

        UpdateProfilingState();
        SendCurrentState();
    }
}

UInt32 Server::GetCurrentState() const
{
    Lock::Locker locker(&CurrentStateLock);
    return CurrentState.StateFlags;
}


void Server::SetConnectedApp(const char* playerTitle)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.ConnectedApp != playerTitle)
    {
        CurrentState.ConnectedApp = playerTitle;
        SendCurrentState();
    }
}

void Server::SetAaMode(const char* aaMode)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.AaMode != aaMode)
    {
        CurrentState.AaMode = aaMode;
        SendCurrentState();
    }
}

void Server::SetStrokeType(const char* strokeType)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.StrokeType != strokeType)
    {
        CurrentState.StrokeType = strokeType;
        SendCurrentState();
    }
}

void Server::SetCurrentLocale(const char* locale)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState.CurrentLocale != locale)
    {
        CurrentState.CurrentLocale = locale;
        SendCurrentState();
    }
}

void Server::SetCurveTolerance(float tolerance)
{
    Lock::Locker locker(&CurrentStateLock);
    if (Alg::Abs(tolerance - CurrentState.CurveTolerance) > 0.001f)
    {
        CurrentState.CurveTolerance = tolerance;
        SendCurrentState();
    }
}

// Updates the state and sends it to AMP
// Useful because not all state components are exposed through individual accessors
// Also, allows changing many components at once, sending only one update message, 
// for example during initialization
void Server::UpdateState(const ServerState* state)
{
    Lock::Locker locker(&CurrentStateLock);
    if (CurrentState != *state)
    {
        CurrentState = *state;
        SendCurrentState();
    }
}


// Can change port so that AMP can distinguish between instances of GFx running on the same address
void Server::SetListeningPort(UInt32 port)
{
    if (Port != port)
    {
        Port = port;
        if (IsSocketCreated())
        {
            CloseConnection();
            OpenConnection();
        }
    }
}

// Set the port where UDP broadcast messages are sent and received
// Has no effect after server had already started
void Server::SetBroadcastPort(UInt32 port)
{
    BroadcastPort = port;
}

// Returns true if currently connected to AMP and sending stats
bool Server::GetProfilingState() const
{ 
    if (!IsEnabled())
    {
        return false;
    }

    if (IsPaused())
    {
        return false;
    }

    if (!SocketThreadMgr || !SocketThreadMgr->IsValidSocket())
    {
        return false;
    }

    if (!IsValidConnection())
    {
        return false;
    }

    return true;
}

// Returns the cached value of GetProfilingState
// The cached value gets updated when you call UpdateProfilingState
bool Server::IsProfiling() const
{ 
    return Profiling;
}

void Server::UpdateProfilingState()
{
    Profiling = GetProfilingState();
}

// Wait until connected, or until the specified time interval has passed
// Used so that an application can start being profiled from the first frame
// since establishing connection can take a little while to establish
void Server::WaitForAmpConnection(unsigned maxDelayMilliseconds)
{
    ConnectedEvent.Wait(maxDelayMilliseconds);
}

// Sets the AMP capabilities supported by the current GFx application
// These capabilities, such as wireframe mode, are implemented outside of GFx
// and are application-specific
void Server::SetAppControlCaps(const MessageAppControl* caps)
{
    AppControlCaps = *SF_HEAP_AUTO_NEW(this) MessageAppControl(caps->GetValue());
    AppControlCaps->SetLoadMovieFile(caps->GetLoadMovieFile());
}

// The server can wait for a connection before proceeding
// This is useful when you want to profile the startup
void Server::SetConnectionWaitTime(unsigned waitTimeMilliseconds)
{
    ConnectionWaitDelay = waitTimeMilliseconds;
}

// If AMP is the only system using sockets, it will initialize the library 
// and release it when it's done.
// SetInitSocketLib(false) means that AMP is going to be using a previously-initialized socket library
void Server::SetInitSocketLib(bool initSocketLib)
{
    InitSocketLib = initSocketLib;
}

// Set the AMP socket implementation factory
// Must be called before a connection has been opened
void Server::SetSocketImplFactory(SocketImplFactory* socketFactory)
{
    SocketFactory = socketFactory;
}

// Sets the AMP heap limit in bytes
// When this limit is exceeded, GFx is paused until any pending messages have been sent to AMP,
// thus reducing the heap memory
void Server::SetHeapLimit(UPInt memLimit)
{
    Memory::GetHeapByAddress(this)->SetLimit(memLimit);
}


// Called once per frame by the app
// It is called from GRenderer::EndFrame
// Updates all statistics
void Server::AdvanceFrame()
{
    UpdateProfilingState();

    // Don't bother updating if we have no client connected
    if (IsProfiling())
    {
        SendFrameStats();
    }
    else
    {
        // Timing stats need to be cleared every frame when not recording
        // Otherwise times will accumulate

        ClearMovieData();
        ClearRendererData();
    }

    Lock::Locker locker(&ToggleStateLock);
    if (ToggleState != 0)
    {
        ToggleAmpState(ToggleState);
        ToggleState = 0;
    }

    // Wait for AMP thread to catch up
    if (IsProfiling())
    {
        SendingEvent.Wait(1000);
    }
}

// Optional callback to handle app control messages
void Server::SetAppControlCallback(AppControlInterface* callback)
{
    AppControlCallback = callback;
}

// Called from the app to give a chance to handle new socket messages
// Returns true if a message was found and handled
bool Server::HandleNextMessage()
{
    if (SocketThreadMgr)
    {
        Ptr<Message> msg = SocketThreadMgr->GetNextReceivedMessage();
        if (msg)
        {
            return msg->AcceptHandler(this);
        }
    }

    return false;
}

// Sends a log message to AMP client
// called directly from the log object
void Server::SendLog(const char* message, int msgLength, LogMessageId msgType)
{
    if (SocketThreadMgr && SocketThreadMgr->IsValidConnection())
    {
        SocketThreadMgr->SendLog(String(message, msgLength), msgType);
    }
}

// Sends the current state to the AMP client so the UI can be properly updated
void Server::SendCurrentState()
{
    if (SocketThreadMgr)
    {
        Lock::Locker locker(&CurrentStateLock);
        SocketThreadMgr->SetBroadcastInfo(CurrentState.ConnectedApp, CurrentState.ConnectedFile);
        Ptr<ViewStats> currentMovie = GetDebugPausedMovie();
        if (currentMovie)
        {
            currentMovie->GetActiveLine(&CurrentState.CurrentFileId, &CurrentState.CurrentLineNumber);            
        }
        MessageCurrentState* state = SF_HEAP_AUTO_NEW(this) MessageCurrentState(&CurrentState);
        SocketThreadMgr->SendAmpMessage(state);
    }
}

// Sends the supported AMP capabilities (for example, wireframe mode) to the AMP client
void Server::SendAppControlCaps()
{
    SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageAppControl(AppControlCaps->GetValue()));
}

// Handles an app control request message by forwarding it to the callback object
bool Server::HandleAppControl(const MessageAppControl* msg)
{
    Lock::Locker locker(&ToggleStateLock);
    ToggleState = 0;
    if (msg->IsToggleAmpRecording())
    {
        ToggleState |= Amp_Paused;
    }
    if (msg->IsToggleInstructionProfile())
    {
        ToggleState |= Amp_ProfileInstructions;
    }
    if (msg->IsToggleMemReport())
    {
        ToggleState |= Amp_MemoryReports;
    }
    if (msg->IsToggleProfileFunctions())
    {
        ToggleState |= Amp_ProfileFunctions;
    }

    if (msg->IsForceInstructionProfile() && !IsInstructionProfiling())
    {
        ToggleState |= Amp_ProfileInstructions;
    }

    if (msg->IsDebugPause())
    {
        Lock::Locker lock(&MovieLock);
        Ptr<ViewStats> pausedMovie = GetDebugPausedMovie();
        if (pausedMovie)
        {
            pausedMovie->DebugGo();
        }
        else
        {
            if (MovieStats.GetSize() > 0)
            {
                MovieStats[0]->AdvanceTimings->DebugPause();
            }
        }
    }

    {
        Lock::Locker lock(&MovieLock);
        Ptr<ViewStats> pausedMovie = GetDebugPausedMovie();
        if (pausedMovie)
        {
            if (msg->IsDebugNextMovie())
            {
                for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
                {
                    if (MovieStats[i]->AdvanceTimings == pausedMovie)
                    {
                        UPInt next = (i == MovieStats.GetSize() - 1 ? 0 : i + 1);
                        MovieStats[next]->AdvanceTimings->DebugPause();
                        pausedMovie->DebugGo();
                    }
                }
            }
            else if (msg->IsDebugStep())
            {
                pausedMovie->DebugStep(0);
            }
            else if (msg->IsDebugStepIn())
            {
                pausedMovie->DebugStep(1);
            }
            else if (msg->IsDebugStepOut())
            {
                pausedMovie->DebugStep(-1);
            }
        }
    }

    if (AppControlCallback != NULL)
    {
        // Forward to custom handler
        AppControlCallback->HandleAmpRequest(msg);
    }

    return true;
}

// The server has no knowledge of the SWD format
// but the SWD may be located on the server computer and not the client
// Therefore the server first tries to load the SWD locally 
// If it succeeds, it sends the data, without parsing it, to the client
// If no SWD is found, then it sends the file name for the requested SWD handle
bool Server::HandleSwdRequest(const MessageSwdRequest* msg)
{
    // Get the first file opener found
    Ptr<FileOpenerBase> fileOpener;
    LoaderLock.DoLock();
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        fileOpener = Loaders[i]->GetFileOpener();
        if (fileOpener)
        {
            break;
        }
    }
    LoaderLock.Unlock();

    if (fileOpener)
    {
        Lock::Locker locker(&SwfLock);

        UInt32 handle = msg->GetValue();
        SwdMap::Iterator it = HandleToSwdIdMap.Find(handle);
        if (it != HandleToSwdIdMap.End())
        {
            String filename = GetSwdFilename(handle);
            if (!filename.IsEmpty() && msg->IsRequestContents())
            {
                // Convert .swf into .swd
                String swdFilename = filename;
                UPInt iLength = swdFilename.GetLength();
                if (iLength > 4)
                {
                    String extension = swdFilename.Substring(iLength - 4, iLength);
                    if (extension == ".swf" || extension == ".gfx")
                    {
                        swdFilename = swdFilename.Substring(0, iLength - 4);
                    }
                }
                swdFilename += ".swd";

                // Try to read the SWD
                bool sent = false;

                Ptr<File> swdFile = *fileOpener->OpenFile(swdFilename);
                if (swdFile && swdFile->GetLength() > 0)
                {
                    Array<UByte> fileData(swdFile->GetLength());
                    if (swdFile->Read(&fileData[0], (int)fileData.GetSize()) == swdFile->GetLength())
                    {
                        // File read. Send the data to the client
                        SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageSwdFile(handle, 
                            &fileData[0], (UInt32)fileData.GetSize(), swdFilename));
                        sent = true;
                    }
                    else
                    {
                        SF_ASSERT(false); // corrupt SWD
                    }
                    swdFile->Close();
                }

                if (!sent)
                {
                    // Send just the SWD file name so the client can try to load it
                    SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageSwdFile(handle, NULL, 0, swdFilename));
                }
            }
        }
    }

    return true;
}

bool Server::HandleSourceRequest(const MessageSourceRequest* msg)
{
    // Get the first file opener found
    Ptr<FileOpenerBase> fileOpener;
    LoaderLock.DoLock();
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        fileOpener = Loaders[i]->GetFileOpener();
        if (fileOpener)
        {
            break;
        }
    }
    LoaderLock.Unlock();

    if (fileOpener)
    {
        Lock::Locker locker(&SourceFileLock);

        UInt64 handle = msg->GetFileHandle();
        SourceFileMap::Iterator it = HandleToSourceFileMap.Find(handle);
        if (it != HandleToSourceFileMap.End())
        {
            String filename = GetSourceFilename(handle);
            if (!filename.IsEmpty() && msg->IsRequestContents())
            {
                // Try to read the file
                bool sent = false;

                Ptr<File> sourceFile = *fileOpener->OpenFile(filename);
                if (sourceFile && sourceFile->GetLength() > 0)
                {
                    Array<UByte> fileData(sourceFile->GetLength());
                    if (sourceFile->Read(&fileData[0], (int)fileData.GetSize()) == sourceFile->GetLength())
                    {
                        // File read. Send the data to the client
                        SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageSourceFile(handle, 
                            &fileData[0], (UInt32)fileData.GetSize(), filename));
                        sent = true;
                    }
                    else
                    {
                        SF_ASSERT(false); // corrupt file
                    }
                    sourceFile->Close();
                }

                if (!sent)
                {
                    // Send just the source file name so the client can try to load it
                    SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageSourceFile(handle, NULL, 0, filename));
                }
            }
        }
    }

    return true;
}

bool Server::HandleImageRequest(const MessageImageRequest* msg)
{
    SF_UNUSED(msg);

    HeapResourceMap imageMap;
    CollectImageData(&imageMap, NULL);

    for (HeapResourceMap::Iterator it = imageMap.Begin(); it != imageMap.End(); ++it)
    {
        for (UPInt j = 0; j < it->Second.GetSize(); ++j)
        {
            ImageResource* imgRes = it->Second[j];
            if (imgRes->GetImageId() == msg->GetValue())
            {
                Ptr<ImageBase> srcImage = imgRes->GetImage();
                if (srcImage && srcImage->GetImageType() > ImageBase::Type_Image)
                {    
#ifdef SF_ENABLE_LIBPNG 
                    MessageImageData* dataMsg;
                    dataMsg = SF_HEAP_AUTO_NEW(this) MessageImageData(msg->GetValue());
                    Ptr<AmpStream> imageFile = *SF_HEAP_AUTO_NEW(this) AmpStream();
                    if (Render::PNG::FileWriter::WriteImage(imageFile, static_cast<Image*>(srcImage.GetPtr())))
                    {
                        dataMsg->SetImageData(imageFile);
                        SocketThreadMgr->SendAmpMessage(dataMsg);
                    }
                    else
                    {
                        dataMsg->Release();
                    }
#endif                          
                }
                return true;
            }
        }
    }

    return true;
}

bool Server::HandleFontRequest(const MessageFontRequest* msg)
{
    SF_UNUSED(msg);

    return true;
}


// Adds a movie to the list of movies to be profiled
// Called from GFxMovieRoot constructor
void Server::AddMovie(MovieImpl* movie)
{
    // Don't profile movies on the debug heaps (such as the AMP HUD)
    if (((movie->GetMovieHeap()->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        // Open the socket connection, if needed
        if (!IsSocketCreated())
        {
            OpenConnection();
        }

        Lock::Locker locker(&MovieLock);
        Movies.PushBack(movie);

        // Add the stats immediately
        MovieStats.PushBack(*SF_HEAP_AUTO_NEW(this) ViewProfile(movie));
    }
}

// Removes a movie from the list of movies to be profiled
// Called from GFxMovieRoot destructor
void Server::RemoveMovie(MovieImpl* movie)
{
    if (((movie->GetMovieHeap()->GetFlags() & MemoryHeap::Heap_UserDebug) == 0))
    {
        Lock::Locker locker(&MovieLock);
        for (UPInt i = 0; i < Movies.GetSize(); ++i)
        {
            if (Movies[i] == movie)
            {
                Movies.RemoveAt(i);
                // Don't remove the stats yet
                // wait until they have reported
                break;
            }
        }

        // If there are no more movies left to profile, close the socket connection
        if (Movies.GetSize() == 0)
        {
            CloseConnection();        
        }
    }
}

// When a movie is unloaded, the stats are kept alive by AMP
// until they can be reported to the client
// RefreshMovieStats re-syncs the movie stats with the loaded movies
// after the statistics have been reported
void Server::RefreshMovieStats()
{
    Lock::Locker locker(&MovieLock);
    MovieStats.Clear();
    for (UPInt i = 0; i < Movies.GetSize(); ++i)
    {
        MovieStats.PushBack(*SF_HEAP_AUTO_NEW(this) ViewProfile(Movies[i]));
    }
}


// Adds a loader to the list of active loaders
// Called from the GFxLoader constructor
void Server::AddLoader(Loader* loader)
{
    Lock::Locker locker(&LoaderLock);
    Loaders.PushBack(loader);
}

// Removes a loader from the list of active loaders 
// Called from the GFxLoader destructor
void Server::RemoveLoader(Loader* loader)
{
    Lock::Locker locker(&LoaderLock);
    for (UPInt i = 0; i < Loaders.GetSize(); ++i)
    {
        if (Loaders[i] == loader)
        {
            Loaders.RemoveAt(i);
            break;
        }
    }
}

// AMP renderer is used to render overdraw
void Server::AddAmpRenderer(Render::AmpRenderer* renderer)
{
    Lock::Locker locker(&LoaderLock);
    if (renderer != NULL)
    {
        AmpRenderers.PushBack(renderer);
    }
}

// Set the renderer to be profiled (only one supported)
void Server::SetRenderer(Render::Renderer2D* renderer)
{
    CurrentRenderer = renderer;
}


// AMP server generates unique handles for each SWF
// The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
// AMP keeps a map of SWD handles to actual SWD ID and filename
UInt32 Server::GetNextSwdHandle() const
{
    static UInt32 lastSwdHandle = NativeCodeSwdHandle;
    Lock::Locker locker(&SwfLock);
    ++lastSwdHandle;
    return lastSwdHandle;
}

// Called whenever a Debug ID tag is found when reading a SWF
// Creates a new entry in the SWD map
void Server::AddSwf(UInt32 swdHandle, const char* swdId, const char* filename)
{
    SwdInfo* pInfo = SF_HEAP_AUTO_NEW(this) SwdInfo();
    pInfo->SwdId = swdId;
    pInfo->Filename = filename;

    Lock::Locker locker(&SwfLock);
    HandleToSwdIdMap.Set(swdHandle, *pInfo);
}

// Retrieves the SWD debug ID for a given handle
String Server::GetSwdId(UInt32 handle) const
{
    Lock::Locker locker(&SwfLock);
    SwdMap::ConstIterator it = HandleToSwdIdMap.Find(handle);
    if (it == HandleToSwdIdMap.End())
    {
        return "";
    }
    return it->Second->SwdId;
}

// Retrieves the SWF filename for a given handle
String Server::GetSwdFilename(UInt32 handle) const
{
    Lock::Locker locker(&SwfLock);
    SwdMap::ConstIterator it = HandleToSwdIdMap.Find(handle);
    if (it == HandleToSwdIdMap.End())
    {
        return "";
    }
    return it->Second->Filename;
}

// Retrieves the source filename for a given handle
String Server::GetSourceFilename(UInt64 handle) const
{
    Lock::Locker locker(&SourceFileLock);
    SourceFileMap::ConstIterator it = HandleToSourceFileMap.Find(handle);
    if (it == HandleToSourceFileMap.End())
    {
        return "";
    }
    return it->Second->Filename;
}

void Server::AddSourceFile(UInt64 fileHandle, const char* fileName)
{
    Lock::Locker locker(&SourceFileLock);
    Ptr<SourceFileInfo> fileInfo = *SF_HEAP_AUTO_NEW(this) SourceFileInfo();
    fileInfo->Filename = fileName;
    HandleToSourceFileMap.Set(fileHandle, fileInfo);
}

AmpStats* Server::GetDisplayStats()
{
    return RenderStats->DisplayTimings;
}

// Constructor
// Called only from Server::Init
Server::Server() :
    ToggleState(0),
    Port(7534),
    BroadcastPort(GFX_AMP_BROADCAST_PORT),
    SocketThreadMgr(NULL),
    CurrentRenderer(NULL),
    ConnectionWaitDelay(0),
    InitSocketLib(true),
    SocketFactory(NULL),
    Profiling(false),
    AppControlCallback(NULL)

{
    SendCallback = *SF_HEAP_AUTO_NEW(this) SendThreadCallback();
    StatusCallback = *SF_HEAP_AUTO_NEW(this) StatusChangedCallback(&ConnectedEvent);
    AppControlCaps = *SF_HEAP_AUTO_NEW(this) MessageAppControl();
    RenderStats = *SF_HEAP_AUTO_NEW(this) RenderProfile();
}

// Destructor
Server::~Server()
{
    SocketThreadMgr = NULL;
}

// Start the socket threads
// Returns true if successful
bool Server::OpenConnection()
{
    if (!IsState(Amp_Disabled))
    {
        ConnectionLock.DoLock();
        if (!SocketThreadMgr)
        {
            SocketThreadMgr = *SF_HEAP_AUTO_NEW(this) ThreadMgr(this, SendCallback, 
                StatusCallback, &SendingEvent, SocketFactory);
        }
        ConnectionLock.Unlock();

        if (!SocketThreadMgr || !SocketThreadMgr->InitAmp(NULL, Port, BroadcastPort))
        {
            return false;
        }

        UpdateProfilingState();

        WaitForAmpConnection(ConnectionWaitDelay);
    }

    return true;
}

// Stops the socket threads
void Server::CloseConnection()
{
    MovieLock.DoLock();
    for (UPInt i = 0; i < Movies.GetSize(); ++i)
    {
        MovieStats[i]->AdvanceTimings->DebugGo();
    }
    MovieLock.Unlock();

    Lock::Locker locker(&ConnectionLock);
    SocketThreadMgr = NULL;
    UpdateProfilingState();
}

// Returns true if we are listening for a connection
bool Server::IsSocketCreated() const
{
    if (!SocketThreadMgr)
    {
        return false;
    }
    return true;
}

// Collects all the memory data per frame
// Populates the ProfileFrame object, which is then sent to the client
void Server::CollectMemoryData(ProfileFrame* frameProfile)
{
    MemoryHeap::RootStats stats;
    Memory::pGlobalHeap->GetRootStats(&stats);

    frameProfile->TotalMemory = (UInt32)(stats.SysMemFootprint - stats.UserDebugFootprint - stats.DebugInfoFootprint);
    UInt32 MemoryComponents = 0;

    HeapResourceMap imageMap;
    FontResourceMap fontMap;
    CollectImageData(&imageMap, &fontMap);

    // Images
    for (HeapResourceMap::Iterator it = imageMap.Begin(); it != imageMap.End(); ++it)
    {
        for (UPInt j = 0; j < it->Second.GetSize(); ++j)
        {
            ImageResource* imgRes = it->Second[j];
            Render::ImageBase* img = imgRes->GetImage();
            if (img != NULL)
            {
                Ptr<ImageInfo> imgInfo = *SF_HEAP_AUTO_NEW(this) ImageInfo();
                imgInfo->Id = img->GetImageId();
                imgInfo->Bytes = static_cast<UInt32>(imgRes->GetBytes());
                imgInfo->HeapName = it->First;
                imgInfo->External = true;  // images are in video memory

                String imgName;
                String fileURL(imgRes->GetFileURL());
                if (!fileURL.IsEmpty())
                { 
                    imgName += fileURL + " ";
                }
                if (imgRes->GetResourceUse() == Resource::Use_Gradient)
                {
                    imgName += "Gradient";
                }
                else if (imgRes->GetResourceUse() == Resource::Use_Bitmap)
                {
                    imgName += "Bitmap";
                }

                UInt32 imageWidth = imgRes->GetWidth();
                UInt32 imageHeight = imgRes->GetHeight();

                if (imgRes->GetImage() && imgRes->GetImage()->GetImageType() == ImageBase::Type_SubImage)
                {
                    SubImage* subImg = static_cast<SubImage*>(imgRes->GetImage());
                    imgInfo->AtlasTop = subImg->GetRect().y1;
                    imgInfo->AtlasBottom = subImg->GetRect().y2;
                    imgInfo->AtlasLeft = subImg->GetRect().x1;
                    imgInfo->AtlasRight = subImg->GetRect().x2;
                    imgInfo->AtlasId = subImg->GetBaseImageId();
                    imageWidth = imgInfo->AtlasRight - imgInfo->AtlasLeft;
                    imageHeight = imgInfo->AtlasBottom - imgInfo->AtlasTop;
                }

                String imgFormat;
                switch (imgRes->GetImageFormat())
                {
                case Render::Image_R8G8B8A8:
                    imgFormat = "R8G8B8A8";
                    break;
                case Render::Image_B8G8R8A8:
                    imgFormat = "B8G8R8A8";
                    break;
                case Render::Image_R8G8B8:
                    imgFormat = "R8G8B8";
                    break;
                case Render::Image_B8G8R8:
                    imgFormat = "B8G8R8";
                    break;
                case Render::Image_A8:
                    imgFormat = "A8";
                    break;
                case Render::Image_DXT1:
                    imgFormat = "DXT1";
                    break;
                case Render::Image_DXT3:
                    imgFormat = "DXT3";
                    break;
                case Render::Image_DXT5:
                    imgFormat = "DXT5";
                    break;
                default:
                    break;
                }

                Format(imgInfo->Name, "{1}x{2} {0} ({3})", 
                    imgName.ToCStr(), 
                    imageWidth, 
                    imageHeight,
                    imgFormat);

                frameProfile->ImageList.PushBack(imgInfo);
            }
        }
    }

    if (IsMemReports())
    {
        Memory::pGlobalHeap->MemReport(frameProfile->MemoryByStatId, MemoryHeap::MemReportHeapDetailed);
        Memory::pGlobalHeap->MemReport(frameProfile->MemoryByHeap, MemoryHeap::MemReportFull);
        frameProfile->ImageMemory = frameProfile->MemoryByStatId->SumValues("Image");
        frameProfile->VideoMemory = frameProfile->MemoryByStatId->SumValues("Video");
        frameProfile->SoundMemory = frameProfile->MemoryByStatId->SumValues("Sound");
        frameProfile->MovieDataMemory = frameProfile->MemoryByStatId->SumValues("Movie Data Heaps");
        frameProfile->MovieViewMemory = frameProfile->MemoryByStatId->SumValues("Movie View Heaps");
        frameProfile->MeshCacheMemory = frameProfile->MemoryByStatId->SumValues("Renderer");
    }
    else
    {
        Ptr<MemItem> memItem = *SF_HEAP_AUTO_NEW(this) MemItem(0);
        Memory::pGlobalHeap->MemReport(memItem, MemoryHeap::MemReportBrief);
        String toggleMessage("Toggle the <img src='instr' width='13' height='13' align='baseline' vspace='-4'> button to enable detailed memory reports");
        frameProfile->MemoryByStatId->Name = toggleMessage;
        frameProfile->MemoryByHeap->Name = toggleMessage;
        for (UPInt i = 0; i < memItem->Children.GetSize(); ++i)
        {
            MemItem* childItem = memItem->Children[i];
            if (childItem->Name == "Image")
            {
                frameProfile->ImageMemory = childItem->Value;
            }
            else if (childItem->Name == "Video")
            {
                frameProfile->VideoMemory = childItem->Value;
            }
            else if (childItem->Name == "Sound")
            {
                frameProfile->SoundMemory = childItem->Value;
            }
            else if (childItem->Name == "Movie Data")
            {
                frameProfile->MovieDataMemory = childItem->Value;
            }
            else if (childItem->Name == "Movie View")
            {
                frameProfile->MovieViewMemory = childItem->Value;
            }
            else if (childItem->Name == "Renderer")
            {
                frameProfile->MeshCacheMemory = childItem->Value;
            }
        }
    }

    MemoryComponents += frameProfile->ImageMemory;
    MemoryComponents += frameProfile->VideoMemory;
    MemoryComponents += frameProfile->SoundMemory;
    MemoryComponents += frameProfile->MovieDataMemory;
    MemoryComponents += frameProfile->MovieViewMemory;
    MemoryComponents += frameProfile->MeshCacheMemory;

    const MemItem* unused = frameProfile->MemoryByStatId->SearchForName("Unused Memory");
    if (unused != NULL)
    {
        MemoryComponents += unused->Value;
    }

    frameProfile->OtherMemory = Alg::Max(frameProfile->TotalMemory, MemoryComponents) - MemoryComponents;

    UInt32 lastItemId = frameProfile->MemoryByStatId->GetMaxId();

    // Fonts
    FontResourceMap::Iterator itFont;
    for (itFont = fontMap.Begin(); itFont != fontMap.End(); ++itFont)
    {
        Ptr<MemItem> fontMovie = *SF_HEAP_AUTO_NEW(this) MemItem(++lastItemId);
        fontMovie->Name = itFont->First;
        fontMovie->StartExpanded = true;
        for (UPInt j = 0; j < itFont->Second.GetSize(); ++j)
        {
            fontMovie->AddChild(++lastItemId, itFont->Second[j]);
        }
        frameProfile->Fonts->Children.PushBack(fontMovie);
    }

    frameProfile->Fonts->StartExpanded = true;
}

// Collects the image information from the ResourceLib and MovieDefs
// Output is a map of heap name to an array of images
void Server::CollectImageData(HeapResourceMap* resourceMap, FontResourceMap* fontMap)
{
    HashSet<UInt32> resLibImages;
    if (resourceMap != NULL)
    {
        Lock::Locker lock(&LoaderLock);

        // Get all the unique resourceLibs from the registered loaders
        Array<ResourceLib*> resourceLibs;
        for (UPInt i = 0; i < Loaders.GetSize(); ++i)
        {
            ResourceLib* resourceLib = Loaders[i]->GetResourceLib();
            if (resourceLib != NULL)
            {
                bool resLibFound = false;
                for (UPInt j  = 0; j < resourceLibs.GetSize(); ++j)
                {
                    if (resourceLibs[j] == resourceLib)
                    {
                        resLibFound = true;
                        break;
                    }
                }
                if (!resLibFound)
                {
                    resourceLibs.PushBack(resourceLib);
                }
            }
        }

        // Get the images for each resourceLib
        for (UPInt i = 0; i < resourceLibs.GetSize(); ++i)
        {
            MemoryHeap* heap = resourceLibs[i]->GetImageHeap();
            String heapName("[Heap] ");
            heapName += heap->GetName();
            HeapResourceMap::Iterator it = resourceMap->FindCaseInsensitive(heapName);
            if (it == resourceMap->End())
            {
                resourceMap->SetCaseInsensitive(heapName, Array< Ptr<ImageResource> >() );
                it = resourceMap->FindCaseInsensitive(heapName);
            }

            Array< Ptr<Resource> > resources;
            resourceLibs[i]->GetResourceArray(&resources);
            for (UPInt i = 0; i < resources.GetSize(); ++i)
            {
                if (resources[i]->GetResourceType() == Resource::RT_Image)
                {
                    ImageResource* imgRes = static_cast<ImageResource*>(resources[i].GetPtr());
                    it->Second.PushBack(imgRes);
                    resLibImages.Set(imgRes->GetImageId());
                }
            }
        }
    }

    {
        Lock::Locker lock(&MovieLock);

        // Get all the unique movieDefs from the registered Movies
        Array<MovieDef*> movieDefs;
        for (UPInt i = 0; i < Movies.GetSize(); ++i)
        {
            MovieDef* movieDef = Movies[i]->GetMovieDef();
            if (movieDef != NULL)
            {
                bool movieDefFound = false;
                for (UPInt j  = 0; j < movieDefs.GetSize(); ++j)
                {
                    if (movieDefs[j] == movieDef)
                    {
                        movieDefFound = true;
                        break;
                    }
                }
                if (!movieDefFound)
                {
                    movieDefs.PushBack(movieDef);
                }
            }
        }

        // Get the images for each movieDef
        for (UPInt i = 0; i < movieDefs.GetSize(); ++i)
        {
            if (resourceMap != NULL)
            {
                ImageVisitor imgVisitor;
                movieDefs[i]->VisitResources(&imgVisitor, MovieDef::ResVisit_AllImages);       

                MemoryHeap* heap = movieDefs[i]->GetImageHeap();

                MemoryHeap::HeapInfo kInfo;
                heap->GetHeapInfo(&kInfo);
                String heapName("[Heap] ");
                heapName += heap->GetName();
                if (kInfo.pParent != NULL && kInfo.pParent != Memory::GetGlobalHeap())
                {
                    heapName += kInfo.pParent->GetName();
                }

                HeapResourceMap::Iterator it = resourceMap->FindCaseInsensitive(heapName);
                if (it == resourceMap->End())
                {
                    resourceMap->SetCaseInsensitive(heapName, Array< Ptr<ImageResource> >());
                    it = resourceMap->FindCaseInsensitive(heapName);
                }
                for (UPInt j = 0; j < imgVisitor.Images.GetSize(); ++j)
                {
                    if (resLibImages.Find(imgVisitor.Images[j]->GetImageId()) == resLibImages.End())
                    {
                        it->Second.PushBack(imgVisitor.Images[j]);
                    }
                }
            }

            if (fontMap != NULL)
            {
                FontVisitor fontVisitor;
                movieDefs[i]->VisitResources(&fontVisitor, MovieDef::ResVisit_Fonts);

                String movieName(movieDefs[i]->GetFileURL());
                FontResourceMap::Iterator itFont = fontMap->FindCaseInsensitive(movieName);
                if (itFont == fontMap->End())
                {
                    fontMap->SetCaseInsensitive(movieName, fontVisitor.Fonts);
                }
                else
                {
                    itFont->Second.Append(fontVisitor.Fonts);
                }
            }
        }
    }
}


// Collects all the movie data per frame
// Populates the ProfileFrame object, which is then sent to the client
void Server::CollectMovieData(ProfileFrame* frameProfile)
{
    Lock::Locker lock(&MovieLock);
    frameProfile->MovieStats.Resize(MovieStats.GetSize());
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->CollectStats(frameProfile, i);
    }

    ClearMovieData();
}

// Clears all the movie data
// Clearing needs to happen even when stats are not being reported
// to avoid them accumulating and therefore being wrong for the first frame
void Server::ClearMovieData()
{
    RefreshMovieStats();  // free stats that belong to unloaded movies

    Lock::Locker lock(&MovieLock);
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        MovieStats[i]->ClearStats();
    }
}

// Populates the ProfileFrame with renderer stats
void Server::CollectRendererData(ProfileFrame* frameProfile)
{
    RenderStats->CollectStats(frameProfile);

    Render::HAL::Stats stats;
    CurrentRenderer->GetHAL()->GetStats(&stats, false);
    CollectRendererStats(frameProfile, stats);

    Render::GlyphCache* glyphCache = CurrentRenderer->GetImpl()->GetGlyphCache();
    frameProfile->RasterizedGlyphCount = glyphCache->GetNumRasterizedGlyphs();
    frameProfile->FontTextureCount = glyphCache->GetNumTextures();
    unsigned totalArea = glyphCache->ComputeTotalArea();
    if (totalArea != 0)
    {
        frameProfile->FontFill = 100 * glyphCache->ComputeUsedArea() / totalArea;
    }

    /*

    for (UPInt i = 0; i < meshCaches.GetSize(); ++i)
    {
        frameProfile->StrokeCount += meshCaches[i]->GetNumStrokes();
        frameProfile->MeshThrashing += meshCaches[i]->GetMeshThrashing();
    }

    */

    ClearRendererData();
}

void Server::CollectRendererStats(ProfileFrame* frameProfile, const Render::HAL::Stats& stats)
{
    frameProfile->MeshCount += stats.Meshes;
    frameProfile->TriangleCount += stats.Triangles;
    frameProfile->DrawPrimitiveCount += stats.Primitives;
}

// Clears all the renderer statistics
// Clearing needs to happen even when stats are not being reported
// to avoid them accumulating and therefore being wrong for the first frame
void Server::ClearRendererData()
{
    RenderStats->ClearStats();
    Render::HAL::Stats stats;
    CurrentRenderer->GetHAL()->GetStats(&stats, true);
}

// Sends all the statistics for the current frame to the AMP client
void Server::SendFrameStats()
{
    Lock::Locker locker(&FrameDataLock);

    ProfileFrame* frameProfile = SF_HEAP_AUTO_NEW(this) ProfileFrame();

    static UInt64 lastTick = 0;
    static UInt32 frameCounter = 0;
    static UInt32 lastFps = 0;
    frameProfile->TimeStamp = Timer::GetProfileTicks();
    UInt64 iDeltaTicks = frameProfile->TimeStamp - lastTick;
    ++frameCounter;
    if (iDeltaTicks > 1000000)
    {
        lastFps = static_cast<UInt32>(frameCounter * 1000000 / iDeltaTicks);
        frameCounter = 0;
        lastTick = frameProfile->TimeStamp;
    }
    frameProfile->FramesPerSecond = lastFps;

    // Send the active SWD handles so the client knows to load the appropriate SWD
    SwfLock.DoLock();
    for (SwdMap::Iterator it = HandleToSwdIdMap.Begin(); it != HandleToSwdIdMap.End(); ++it)
    {
        frameProfile->SwdHandles.PushBack(it->First);
    }
    SwfLock.Unlock();

    // Send the active files so the client knows to load them
    SourceFileLock.DoLock();
    for (SourceFileMap::Iterator it = HandleToSourceFileMap.Begin(); it != HandleToSourceFileMap.End(); ++it)
    {
        frameProfile->FileHandles.PushBack(it->First);
    }
    SourceFileLock.Unlock();
    
//     Uncomment for marker testing
//     if (frameProfile->TimeStamp % 10 == 0 && MovieStats.GetSize() > 0)
//     {
//         MovieStats[0]->AdvanceTimings->AddMarker("test");
//     }

    CollectMovieData(frameProfile);
    CollectRendererData(frameProfile);
    CollectMemoryData(frameProfile);

    SocketThreadMgr->SendAmpMessage(SF_HEAP_AUTO_NEW(this) MessageProfileFrame(*frameProfile));
}

Ptr<ViewStats> Server::GetDebugPausedMovie() const
{
    Lock::Locker lock(&MovieLock);
    for (UPInt i = 0; i < MovieStats.GetSize(); ++i)
    {
        if (MovieStats[i]->AdvanceTimings->IsDebugPaused())
        {
            return MovieStats[i]->AdvanceTimings;
        }
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////

Server::ViewProfile::ViewProfile(MovieImpl* movie) : 
    AdvanceTimings(movie->AdvanceStats)
{
}

void Server::ViewProfile::CollectStats(ProfileFrame* frameProfile, UPInt index)
{
    MovieProfile* stats = SF_HEAP_AUTO_NEW(this) MovieProfile(); 

    AdvanceTimings->CollectTimingStats(frameProfile);
    AdvanceTimings->CollectAmpInstructionStats(stats);
    AdvanceTimings->CollectAmpFunctionStats(stats);
    AdvanceTimings->CollectAmpSourceLineStats(stats);
    AdvanceTimings->CollectMarkers(stats);

    stats->ViewHandle = AdvanceTimings->GetViewHandle();
    stats->MinFrame = stats->MaxFrame = AdvanceTimings->GetCurrentFrame();
    stats->ViewName = AdvanceTimings->GetName();
    stats->Version = AdvanceTimings->GetVersion();
    stats->Width = AdvanceTimings->GetWidth();
    stats->Height = AdvanceTimings->GetHeight();
    stats->FrameRate = AdvanceTimings->GetFrameRate();
    stats->FrameCount = AdvanceTimings->GetFrameCount();

    frameProfile->MovieStats[index] = *stats;        

    // Update gradient fill by counting how many times FillStyle::GetGradientFillTexture was called
//     for (UPInt j = 0; j < stats->FunctionStats->FunctionTimings.GetSize(); ++j)
//     {
//         MovieFunctionStats::FuncStats& funcStats = stats->FunctionStats->FunctionTimings[j];
//         UInt64 key = NativeCodeSwdHandle;
//         key <<= 32;
//         key += Func_GFxFillStyle_GetGradientFillTexture;
//         if (funcStats.FunctionId == key)
//         {
//             frameProfile->GradientFillCount += funcStats.TimesCalled;
//         }
//     }
}

void Server::ViewProfile::ClearStats()
{
    AdvanceTimings->ClearAmpFunctionStats();
    AdvanceTimings->ClearAmpInstructionStats();
    AdvanceTimings->ClearAmpSourceLineStats();
    AdvanceTimings->ClearMarkers();
}

//////////////////////////////////////////////////////////////////////////////

Server::RenderProfile::RenderProfile()
{
    DisplayTimings = *SF_HEAP_AUTO_NEW(this) ViewStats();
}

void Server::RenderProfile::CollectStats(ProfileFrame* frameProfile)
{
    Ptr<MovieProfile> stats = *SF_HEAP_AUTO_NEW(this) MovieProfile(); 

    DisplayTimings->CollectTimingStats(frameProfile);
    DisplayTimings->CollectAmpFunctionStats(stats);

    frameProfile->DisplayStats = stats->FunctionStats;        
}

void Server::RenderProfile::ClearStats()
{
    DisplayTimings->ClearAmpFunctionStats();
}

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // SF_AMP_SERVER


