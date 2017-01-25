/**********************************************************************

PublicHeader:   Kernel
Filename    :   SF_AmpInterface.h
Content     :   AMP Interface
Created     :   2010
Authors     :   Alex Mantzaris

Notes       :   Interface to the Analyzer for Memory and Performance

Copyright   :   (c) 1998-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Kernel_AmpInterface_H
#define INC_SF_Kernel_AmpInterface_H

#include "SF_RefCount.h"
#include "SF_String.h"
#include "SF_Log.h"

namespace Scaleform {

namespace Render { 
    class AmpRenderer; 
    class Renderer2D; 
}

namespace GFx { 
    class Loader;
    class MovieImpl;
    namespace AMP {
        class AppControlInterface;
        class ServerState;
        class MessageAppControl;
        class SocketImplFactory;
    }
}

enum AmpNativeFunctionId
{
    Amp_Native_Function_Id_Invalid = -1,

    Amp_Native_Function_Id_Advance,
    Amp_Native_Function_Id_AdvanceFrame,
    Amp_Native_Function_Id_DoActions,
    Amp_Native_Function_Id_ProcessInput,
    Amp_Native_Function_Id_ProcessMouse,

    Amp_Native_Function_Id_Display,
    Amp_Native_Function_Id_Tessellate,
    Amp_Native_Function_Id_GlyphTextureMapper_Create,

    Amp_Native_Function_Id_GetVariable,
    Amp_Native_Function_Id_GetVariableArray,
    Amp_Native_Function_Id_GetVariableArraySize,
    Amp_Native_Function_Id_SetVariable,
    Amp_Native_Function_Id_SetVariableArray,
    Amp_Native_Function_Id_SetVariableArraySize,
    Amp_Native_Function_Id_Invoke,
    Amp_Native_Function_Id_InvokeAlias,
    Amp_Native_Function_Id_InvokeArgs,
    Amp_Native_Function_Id_InvokeAliasArgs,

    Amp_Native_Function_Id_Begin_ObjectInterface,
    Amp_Native_Function_Id_ObjectInterface_HasMember,
    Amp_Native_Function_Id_ObjectInterface_GetMember,
    Amp_Native_Function_Id_ObjectInterface_SetMember,
    Amp_Native_Function_Id_ObjectInterface_Invoke,
    Amp_Native_Function_Id_ObjectInterface_DeleteMember,
    Amp_Native_Function_Id_ObjectInterface_VisitMembers,
    Amp_Native_Function_Id_ObjectInterface_GetArraySize,
    Amp_Native_Function_Id_ObjectInterface_SetArraySize,
    Amp_Native_Function_Id_ObjectInterface_GetElement,
    Amp_Native_Function_Id_ObjectInterface_SetElement,
    Amp_Native_Function_Id_ObjectInterface_VisitElements,
    Amp_Native_Function_Id_ObjectInterface_PushBack,
    Amp_Native_Function_Id_ObjectInterface_PopBack,
    Amp_Native_Function_Id_ObjectInterface_RemoveElements,
    Amp_Native_Function_Id_ObjectInterface_GetDisplayMatrix,
    Amp_Native_Function_Id_ObjectInterface_SetDisplayMatrix,
    Amp_Native_Function_Id_ObjectInterface_GetMatrix3D,
    Amp_Native_Function_Id_ObjectInterface_SetMatrix3D,
    Amp_Native_Function_Id_ObjectInterface_IsDisplayObjectActive,
    Amp_Native_Function_Id_ObjectInterface_GetDisplayInfo,
    Amp_Native_Function_Id_ObjectInterface_SetDisplayInfo,
    Amp_Native_Function_Id_ObjectInterface_SetText,
    Amp_Native_Function_Id_ObjectInterface_GetText,
    Amp_Native_Function_Id_ObjectInterface_GotoAndPlay,
    Amp_Native_Function_Id_ObjectInterface_GetCxform,
    Amp_Native_Function_Id_ObjectInterface_SetCxform,
    Amp_Native_Function_Id_ObjectInterface_CreateEmptyMovieClip,
    Amp_Native_Function_Id_ObjectInterface_AttachMovie,
    Amp_Native_Function_Id_End_ObjectInterface,

    Amp_Num_Native_Function_Ids
};

class AmpStats;

// AmpInterface is the interface for AMP
// Having an interface in the Kernel eliminates the dependency on AMP
// an instance of a class implementing this interface is typically 
// created on application startup and passed into System or GFx::System constructor.
class AmpServer
{
public:
    virtual ~AmpServer() { }

    // AdvanceFrame needs to be called once per frame
    // It is called from GRenderer::EndFrame
    virtual void AdvanceFrame() = 0;

    static AmpServer& GetInstance();

    // Custom callback that handles application-specific messages
    virtual void        SetAppControlCallback(GFx::AMP::AppControlInterface* callback) = 0;

    virtual bool        IsValidConnection() const = 0;
    virtual UInt32      GetCurrentState() const = 0;
    virtual void        SetConnectedApp(const char* playerTitle) = 0;
    virtual void        SetAaMode(const char* aaMode) = 0;
    virtual void        SetStrokeType(const char* strokeType) = 0;
    virtual void        SetCurrentLocale(const char* locale) = 0;
    virtual void        SetCurveTolerance(float tolerance) = 0;
    virtual void        UpdateState(const GFx::AMP::ServerState* state) = 0;
    virtual bool        IsEnabled() const = 0;
    virtual bool        IsPaused() const = 0;
    virtual bool        IsProfiling() const = 0;
    virtual bool        IsInstructionProfiling() const = 0;
    virtual bool        IsInstructionSampling() const = 0;
    virtual bool        IsProfileFunctions() const = 0;
    virtual void        SetAppControlCaps(const GFx::AMP::MessageAppControl* caps) = 0;

    // Configuration options
    virtual void        SetListeningPort(UInt32 port) = 0;
    virtual void        SetBroadcastPort(UInt32 port) = 0;
    virtual void        SetNoBroadcast() { SetBroadcastPort(0); } //  call before connection is opened
    virtual void        SetConnectionWaitTime(unsigned waitTimeMilliseconds) = 0;
    virtual void        SetHeapLimit(UPInt memLimit) = 0;
    virtual void        SetInitSocketLib(bool initSocketLib) = 0;
    virtual void        SetSocketImplFactory(GFx::AMP::SocketImplFactory* socketFactory) = 0;

    // Message handler
    virtual bool        HandleNextMessage() = 0;

    // Specific messages
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType) = 0;
    virtual void    SendCurrentState() = 0;
    virtual void    SendAppControlCaps() = 0;

    // AMP keeps track of active Movie Views
    virtual void        AddMovie(GFx::MovieImpl* movie) = 0;
    virtual void        RemoveMovie(GFx::MovieImpl* movie) = 0;

    // AMP keeps track of active Loaders (for renderer access)
    virtual void        AddLoader(GFx::Loader* loader) = 0;
    virtual void        RemoveLoader(GFx::Loader* loader) = 0;

    // AMP renderer is used to render overdraw
    virtual void        AddAmpRenderer(Render::AmpRenderer* renderer) = 0;
    virtual void        SetRenderer(Render::Renderer2D* renderer) = 0;

    // AMP server generates unique handles for each SWF
    // The 16-byte SWD debug ID is not used because it is not appropriate as a hash key
    // AMP keeps a map of SWD handles to actual SWD ID and filename
    virtual UInt32      GetNextSwdHandle() const = 0;
    virtual void        AddSwf(UInt32 swdHandle, const char* swdId, const char* filename) = 0;
    virtual String      GetSwdId(UInt32 handle) const = 0;
    virtual String      GetSwdFilename(UInt32 handle) const = 0;

    virtual void    AddSourceFile(UInt64 fileHandle, const char* fileName) = 0;

    // Renderer statistics
    virtual AmpStats*   GetDisplayStats() = 0;

protected:
    static AmpServer* AmpServerSingleton;
};

class AmpStats : public RefCountBase<AmpStats, Stat_Default_Mem>
{
public:
    virtual ~AmpStats() { }

    virtual void    NativePushCallstack(const char* functionName, AmpNativeFunctionId functionId) = 0;
    virtual void    NativePopCallstack(UInt64 time) = 0;
};

// This class keeps track of function execution time and call stack
// Time starts counting in constructor and stops in destructor
// Updates the view stats object with the results
class AmpFunctionTimer
{
public:
    AmpFunctionTimer(AmpStats* ampStats, const char* functionName, AmpNativeFunctionId functionId = Amp_Native_Function_Id_Invalid) : 
            StartTicks(0), Stats(ampStats)
    { 
        if (functionId == Amp_Native_Function_Id_Invalid)
        {
            if (!AmpServer::GetInstance().IsProfiling()
                || !AmpServer::GetInstance().IsProfileFunctions())
            {
                Stats = NULL;
            }
        }
        if (Stats != NULL)
        {
            Stats->NativePushCallstack(functionName, functionId);
            StartTicks = Timer::GetRawTicks();
        }
    }
    ~AmpFunctionTimer()
    {
        if (Stats != NULL)
        {
            Stats->NativePopCallstack(Timer::GetRawTicks() - StartTicks);
        }
    }
private:
    UInt64              StartTicks;
    AmpStats*           Stats;
};

// Some macros for frequently-used methods
#ifdef SF_AMP_SERVER
#define SF_AMP_SCOPE_TIMER_ID(ampStats, functionName, functionId)    AmpFunctionTimer _amp_timer_##functionId((ampStats), (functionName), (functionId))
#define SF_AMP_SCOPE_TIMER(ampStats, functionName)    AmpFunctionTimer _amp_timer_((ampStats), (functionName))
#define SF_AMP_SCOPE_RENDER_TIMER_ID(functionName, functionId)    AmpFunctionTimer _amp_timer_##functionId(AmpServer::GetInstance().GetDisplayStats(), (functionName), (functionId))
#define SF_AMP_SCOPE_RENDER_TIMER(functionName)    AmpFunctionTimer _amp_timer_(AmpServer::GetInstance().GetDisplayStats(), (functionName))
#define SF_AMP_CODE(x) x
#else
#define SF_AMP_SCOPE_TIMER_ID(ampStats, functionName, functionId)
#define SF_AMP_SCOPE_TIMER(ampStats, functionName)
#define SF_AMP_SCOPE_RENDER_TIMER_ID(functionName, functionId)
#define SF_AMP_SCOPE_RENDER_TIMER(functionName)
#define SF_AMP_CODE(x)
#endif


}  // namespace Scaleform

#endif  //  INC_SF_Kernel_AmpInterface_H

