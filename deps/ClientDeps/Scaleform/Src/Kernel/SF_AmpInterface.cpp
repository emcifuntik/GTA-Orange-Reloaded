/**********************************************************************

PublicHeader:   Kernel
Filename    :   SF_AmpInterface.cpp
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

#include "SF_AmpInterface.h"
#include "SF_HeapNew.h"

namespace Scaleform {

class DefaultAmpServer : public AmpServer
{
public:
    virtual void    AdvanceFrame() { }
    virtual void    SetAppControlCallback(GFx::AMP::AppControlInterface* callback) { SF_UNUSED(callback); }
    virtual bool    IsValidConnection() const { return false; }
    virtual UInt32  GetCurrentState() const { return 0; }
    virtual void    SetConnectedApp(const char* playerTitle) { SF_UNUSED(playerTitle); }
    virtual void    SetAaMode(const char* aaMode) { SF_UNUSED(aaMode); }
    virtual void    SetStrokeType(const char* strokeType) { SF_UNUSED(strokeType); }
    virtual void    SetCurrentLocale(const char* locale) { SF_UNUSED(locale); }
    virtual void    SetCurveTolerance(float tolerance) { SF_UNUSED(tolerance); }
    virtual void    UpdateState(const GFx::AMP::ServerState* state) { SF_UNUSED(state); }
    virtual bool    IsEnabled() const { return false; }
    virtual bool    IsPaused() const { return false; }
    virtual bool    IsProfiling() const { return false; }
    virtual bool    IsInstructionProfiling() const { return false; }
    virtual bool    IsInstructionSampling() const { return false; }
    virtual bool    IsProfileFunctions() const  { return false; }
    virtual void    SetAppControlCaps(const GFx::AMP::MessageAppControl* caps){ SF_UNUSED(caps); }
    virtual bool    HandleNextMessage() { return false; }
    virtual void    SendLog(const char* message, int messageLength, LogMessageId msgType) { SF_UNUSED3(message, messageLength, msgType); }
    virtual void    SendCurrentState() { }
    virtual void    SendAppControlCaps() { }
    virtual void    SetListeningPort(UInt32 port) { SF_UNUSED(port); }
    virtual void    SetBroadcastPort(UInt32 port) { SF_UNUSED(port); }
    virtual void    SetConnectionWaitTime(unsigned waitTimeMilliseconds) { SF_UNUSED(waitTimeMilliseconds); }
    virtual void    SetHeapLimit(UPInt memLimit) { SF_UNUSED(memLimit); }
    virtual void    SetInitSocketLib(bool initSocketLib) { SF_UNUSED(initSocketLib); }
    virtual void    SetSocketImplFactory(GFx::AMP::SocketImplFactory* socketFactory) { SF_UNUSED(socketFactory); }
    virtual void    AddMovie(GFx::MovieImpl* movie) { SF_UNUSED(movie); }
    virtual void    RemoveMovie(GFx::MovieImpl* movie) { SF_UNUSED(movie); }
    virtual void    AddLoader(GFx::Loader* loader) { SF_UNUSED(loader); }
    virtual void    RemoveLoader(GFx::Loader* loader) { SF_UNUSED(loader); }
    virtual void    AddAmpRenderer(Render::AmpRenderer* renderer) { SF_UNUSED(renderer); }
    virtual void    SetRenderer(Render::Renderer2D* renderer) { SF_UNUSED(renderer); }
    virtual UInt32  GetNextSwdHandle() const { return 0; }
    virtual void    AddSwf(UInt32 swdHandle, const char* swdId, const char* filename) { SF_UNUSED3(swdHandle, swdId, filename); }
    virtual String  GetSwdId(UInt32 handle) const { SF_UNUSED(handle); return ""; }
    virtual String  GetSwdFilename(UInt32 handle) const { SF_UNUSED(handle); return ""; }
    virtual AmpStats* GetDisplayStats() { return NULL; }
    virtual void    AddSourceFile(UInt64 fileHandle, const char* fileName) { SF_UNUSED2(fileHandle, fileName); }
};


// Static
AmpServer* AmpServer::AmpServerSingleton = NULL;

// Static
AmpServer& AmpServer::GetInstance()
{
    if (AmpServerSingleton == NULL)
    {
        static DefaultAmpServer ampSingleton;
        AmpServerSingleton = &ampSingleton; 
    }
    return *AmpServerSingleton;
}


}  // namespace Scaleform

