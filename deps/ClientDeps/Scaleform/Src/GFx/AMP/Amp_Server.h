/**********************************************************************

PublicHeader:   AMP
Filename    :   Amp_Server.h
Content     :   Class encapsulating communication with AMP
                Embedded in application to be profiled
Created     :   December 2009
Authors     :   Alex Mantzaris

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INCLUDE_FX_AMP_SERVER_H
#define INCLUDE_FX_AMP_SERVER_H

#include "Amp_Interfaces.h"
#include "Kernel/SF_AmpInterface.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_ImageResource.h"
#include "Render/Renderer2D.h"
#include "Render/Render_HAL.h"

namespace Scaleform {

namespace Render
{
    // Forward declaration
    class AmpRenderer;
}

namespace GFx {

namespace AMP {

class ThreadMgr;
class SendThreadCallback;
class StatusChangedCallback;
class ViewStats;
class SocketImplFactory;

// AMP server states
enum ServerStateType
{
    Amp_Default =                   0x00000000,
    Amp_Paused =                    0x00000001,
    Amp_Disabled =                  0x00000002,
    Amp_ProfileInstructions =       0x00000004,
    Amp_RenderOverdraw =            0x00000008,
    Amp_InstructionSampling =       0x00000010,
    Amp_MemoryReports =             0x00000020,
    Amp_ProfileFunctions =          0x00000040,
    Amp_App_Paused =                0x00001000,
    Amp_App_Wireframe =             0x00002000,
    Amp_App_FastForward =           0x00004000,
    Amp_RenderBatch =               0x00008000,
};

#define GFX_AMP_BROADCAST_PORT 7533

#ifdef SF_AMP_SERVER

// Server encapsulates the communication of a GFx application with an AMP client
// It is a singleton so that it can be easily accessed throughout GFx
// The object has several functions:
// - it keeps track of statistics per frame
// - it manages the socket connection with the client
// - it handles received messages
// The server needs to be thread-safe, as it can be accessed from anywhere in GFx
class Server : public MsgHandler, public AmpServer, public RefCountBase<Server, Stat_Default_Mem>
{
public:
    // Singleton initialization and uninitialization
    // Called from System
    static void     Init();
    static void     Uninit();

    // Connection status
    virtual bool    IsValidConnection() const;
    virtual UInt32  GetCurrentState() const;
    virtual void    SetConnectedApp(const char* playerTitle);
    virtual void    SetAaMode(const char* aaMode);
    virtual void    SetStrokeType(const char* strokeType);
    virtual void    SetCurrentLocale(const char* locale);
    virtual void    SetCurveTolerance(float tolerance);
    virtual void    UpdateState(const ServerState* state);
    virtual bool    IsEnabled() const               { return !IsState(Amp_Disabled); }
    virtual bool    IsPaused() const                { return IsState(Amp_Paused); }
    virtual bool    IsProfiling() const;
    virtual bool    IsInstructionProfiling() const  { return IsState(Amp_ProfileInstructions); }
    virtual bool    IsInstructionSampling() const   { return IsState(Amp_InstructionSampling); }
    virtual bool    IsProfileFunctions() const      { return IsState(Amp_ProfileFunctions); }
    virtual void    SetAppControlCaps(const MessageAppControl* caps);

    // Configuration options
    virtual void    SetListeningPort(UInt32 port);
    virtual void    SetBroadcastPort(UInt32 port);
    virtual void    SetConnectionWaitTime(unsigned waitTimeMilliseconds);
    virtual void    SetHeapLimit(UPInt memLimit);
    virtual void    SetInitSocketLib(bool initSocketLib);
    virtual void    SetSocketImplFactory(SocketImplFactory* socketFactory);

    // AdvanceFrame needs to be called once per frame
    // It is called from GRenderer::EndFrame
    virtual void    AdvanceFrame();

    // Custom callback that handles application-specific messages
    virtual void    SetAppControlCallback(AppControlInterface* callback);

    // Message handler
    virtual bool    HandleNextMessage();

    // Specific messages
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType);
    virtual void    SendCurrentState();
    virtual void    SendAppControlCaps();

    // GFxAmpMsgHandler interface implementation
    virtual bool    HandleAppControl(const MessageAppControl* message);
    virtual bool    HandleSwdRequest(const MessageSwdRequest* message);
    virtual bool    HandleSourceRequest(const MessageSourceRequest* message);
    virtual bool    HandleImageRequest(const MessageImageRequest* message);
    virtual bool    HandleFontRequest(const MessageFontRequest* message);
    virtual bool    IsInitSocketLib() const { return InitSocketLib; }

    // AMP keeps track of active Movie Views
    virtual void    AddMovie(MovieImpl* movie);
    virtual void    RemoveMovie(MovieImpl* movie);
    void            RefreshMovieStats();

    // AMP keeps track of active Loaders (for renderer access)
    void            AddLoader(Loader* loader);
    void            RemoveLoader(Loader* loader);

    // AMP renderer is used to render overdraw
    virtual void    AddAmpRenderer(Render::AmpRenderer* renderer);
    // Set the renderer to be profiled (only one supported)
    virtual void    SetRenderer(Render::Renderer2D* renderer);

    // AMP server generates unique handles for each SWF
    // The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
    // AMP keeps a map of SWD handles to actual SWD ID and filename
    virtual UInt32  GetNextSwdHandle() const;
    virtual void    AddSwf(UInt32 swdHandle, const char* swdId, const char* filename);
    virtual String  GetSwdId(UInt32 handle) const;
    virtual String  GetSwdFilename(UInt32 handle) const;

    // ActionScript source code file name for a given handle
    virtual void    AddSourceFile(UInt64 fileHandle, const char* fileName);

    // Renderer statistics
    virtual AmpStats*   GetDisplayStats();

private:

    // Struct that holds loaded SWF information
    struct SwdInfo : public RefCountBase<SwdInfo, Stat_Default_Mem>
    {
        StringLH    SwdId;
        StringLH    Filename;
    };
    typedef HashLH<UInt32, Ptr<SwdInfo> > SwdMap;

    // Struct that holds AS file information
    struct SourceFileInfo : public RefCountBase<SourceFileInfo, Stat_Default_Mem>
    {
        StringLH Filename;
    };
    typedef HashLH<UInt64, Ptr<SourceFileInfo> > SourceFileMap;

    struct ViewProfile : public RefCountBase<ViewProfile, Stat_Default_Mem>
    {
        ViewProfile(MovieImpl* movie);
        void CollectStats(ProfileFrame* frameProfile, UPInt index);
        void ClearStats();
        Ptr<ViewStats>      AdvanceTimings;
    };

    struct RenderProfile : public RefCountBase<RenderProfile, Stat_Default_Mem>
    {
        RenderProfile();
        void CollectStats(ProfileFrame* frameProfile);
        void ClearStats();
        Ptr<ViewStats>      DisplayTimings;
    };

    // Member variables
    ServerState                     CurrentState;   // Paused, Disabled, etc
    mutable Lock                    CurrentStateLock;
    UInt32                          ToggleState;   // Paused, Disabled, etc
    mutable Lock                    ToggleStateLock;
    UInt32                          Port;           // For socket connection to client
    UInt32                          BroadcastPort;  // For broadcasting IP address to AMP
    Ptr<ThreadMgr>                  SocketThreadMgr;      // Socket threads
    mutable Lock                    ConnectionLock;
    mutable Lock                    FrameDataLock;
    ArrayLH<MovieImpl*>             Movies;
    ArrayLH< Ptr<ViewProfile> >     MovieStats;
    mutable Lock                    MovieLock;
    ArrayLH<Loader*>                Loaders;
    ArrayLH<Render::AmpRenderer*>   AmpRenderers;
    Render::Renderer2D*             CurrentRenderer;
    Ptr<RenderProfile>              RenderStats;
    mutable Lock                    LoaderLock;
    SwdMap                          HandleToSwdIdMap; // Map of unique handle to SWF information
    mutable Lock                    SwfLock;
    SourceFileMap                   HandleToSourceFileMap; 
    mutable Lock                    SourceFileLock;
    Scaleform::Event                ConnectedEvent; // Used to suspend GFx until connected to AMP
    Scaleform::Event                SendingEvent; // Used to suspend GFx until message queue is empty
    unsigned                        ConnectionWaitDelay;  // milliseconds
    bool                            InitSocketLib;  // Initialize socket library?
    SocketImplFactory*              SocketFactory;
    mutable bool                    Profiling;

    // Handles app requests from AMP client
    AppControlInterface*            AppControlCallback;
    // Callback from Send Thread
    Ptr<SendThreadCallback>         SendCallback;
    // Callback for connection status change
    Ptr<StatusChangedCallback>      StatusCallback;

    // Caps so that Client knows what app control functionality is supported
    Ptr<MessageAppControl>          AppControlCaps;

    // private constructor 
    // Create singleton with Server::Init
    Server();
    ~Server();

    bool        IsMemReports() const            { return IsState(Amp_MemoryReports); }
    void        WaitForAmpConnection(unsigned maxDelayMilliseconds = SF_WAIT_INFINITE);
    bool        IsState(ServerStateType state) const;
    void        SetState(ServerStateType state, bool stateValue);
    void        ToggleAmpState(UInt32 toggleState);
    String      GetSourceFilename(UInt64 handle) const;
    bool        GetProfilingState() const;
    void        UpdateProfilingState();

    // Internal helper methods
    bool        OpenConnection();
    void        CloseConnection();
    bool        IsSocketCreated() const;

    void        CollectMemoryData(ProfileFrame* frameProfile);
    void        CollectMovieData(ProfileFrame* frameProfile);
    void        ClearMovieData();
    void        CollectRendererData(ProfileFrame* frameProfile);
    void        ClearRendererData();
    void        SendFrameStats();
    void        CollectRendererStats(ProfileFrame* frameProfile, const Render::HAL::Stats& stats);

    typedef StringHash< Array< Ptr<ImageResource> > > HeapResourceMap;
    typedef StringHash< Array<String> > FontResourceMap;
    void        CollectImageData(HeapResourceMap* resourceMap, FontResourceMap* fontMap);

    Ptr<ViewStats>  GetDebugPausedMovie() const;
};

#endif  // SF_AMP_SERVER

} // namespace AMP
} // namespace GFx
} // namespace Scaleform

#endif  // INCLUDE_FX_AMP_SERVER_H

