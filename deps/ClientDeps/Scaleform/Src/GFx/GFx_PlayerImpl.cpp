/**********************************************************************

Filename    :   GFxPlayerImpl.cpp
Content     :   MovieRoot and Definition classes
Created     :   
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "Kernel/SF_File.h"
#include "Render/Render_Image.h"
//#include "Render/Render_JPEGUtil.h"
#include "Kernel/SF_Alg.h"

//#include "Render/Render_Renderer.h"
#include "Kernel/SF_UTF8Util.h"

#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_LoadProcess.h"
//#include "GFx/GFx_RenderGen.h"

#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Log.h"
#include "GFx/GFx_MorphCharacter.h"
#include "GFx/GFx_Shape.h"
#include "GFx/GFx_Stream.h"
//#include "GFx/GFx_Styles.h"
#include "GFx/GFx_DisplayList.h"

#include "GFx/GFx_ASUtils.h"

//#include "GFx/GFx_DisplayContext.h"

#include "GFx/IME/GFx_IMEManager.h"

#include "GFx/GFx_Audio.h"
#include "GFx/GFx_VideoBase.h"
#include "Kernel/SF_MsgFormat.h"

#include "GFx/GFx_PlayerStats.h"
#include "GFx/AMP/Amp_Server.h"

#include <string.h> // for memset
#include <float.h>
#include <stdlib.h>
#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

#ifdef SF_BUILD_LOGO
#include "GFxLogo.cpp"
#endif

// Increment this when the cache data format changes.
#define CACHE_FILE_VERSION  4

//#define SF_ADVANCE_EXECUTION_TRACE

namespace Scaleform { namespace GFx {

// ***** Statistics declarations

// MovieDef Memory.
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMD_Mem, "MovieDef", Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_CharDefs_Mem,  "CharDefs", StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_ShapeData_Mem, "ShapeData",StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Tags_Mem,      "Tags",     StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Fonts_Mem,     "Fonts",    StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Images_Mem,    "Images",   StatMD_Mem)
#ifdef GFX_ENABLE_SOUND
SF_DECLARE_MEMORY_STAT(StatMD_Sounds_Mem,    "Sounds",   StatMD_Mem)
#endif
SF_DECLARE_MEMORY_STAT(StatMD_ActionOps_Mem, "ActionOps",   StatMD_Mem)
SF_DECLARE_MEMORY_STAT(StatMD_Other_Mem,     "MD_Other",   StatMD_Mem)
// Load timings.
SF_DECLARE_TIMER_STAT_SUM_GROUP(StatMD_Time, "Time",     StatGroup_Default)
SF_DECLARE_TIMER_STAT(StatMD_Load_Tks,       "Load",     StatMD_Time)
SF_DECLARE_TIMER_STAT(StatMD_Bind_Tks,       "Bind",     StatMD_Time)
      
// MovieView Memory.
SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatMV_Mem,   "MovieView",    Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_MovieClip_Mem,       "MovieClip",    StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_ActionScript_Mem,    "ActionScript", StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_Text_Mem,            "Text",         StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_XML_Mem,             "XML",          StatMV_Mem)
SF_DECLARE_MEMORY_STAT(StatMV_Other_Mem,           "MV_Other",     StatMV_Mem)

SF_DECLARE_MEMORY_STAT(StatIME_Mem,                "IME",          Stat_Mem)

SF_DECLARE_MEMORY_STAT_AUTOSUM_GROUP(StatFC_Mem,   "FontCache",    Stat_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_Batch_Mem,           "Batches",      StatFC_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_GlyphCache_Mem,      "GlyphCache",   StatFC_Mem)
SF_DECLARE_MEMORY_STAT(StatFC_Other_Mem,           "FC_Other",     StatFC_Mem)


// MovieView Timings.
SF_DECLARE_TIMER_STAT_AUTOSUM_GROUP(StatMV_Tks,            "Ticks",          StatGroup_Default)
SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Advance_Tks,    "Advance",        StatMV_Tks)
SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Action_Tks,     "Action",         StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Seek_Tks,       "Seek",           StatMV_Action_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Timeline_Tks,   "Timeline",       StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Input_Tks,      "Input",          StatMV_Advance_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Mouse_Tks,      "Mouse",          StatMV_Input_Tks)

SF_DECLARE_TIMER_STAT_AUTOSUM_GROUP(StatMV_ScriptCommunication_Tks, "Script Communication",  StatMV_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_GetVariable_Tks,"GetVariable",    StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_SetVariable_Tks,"SetVariable",    StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Invoke_Tks,     "Invoke",         StatMV_ScriptCommunication_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_InvokeAction_Tks, "InvokeAction", StatMV_Invoke_Tks)

SF_DECLARE_TIMER_STAT_SUM_GROUP(    StatMV_Display_Tks,    "Display",        StatMV_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_Tessellate_Tks,    "Tessellate",  StatMV_Display_Tks)
SF_DECLARE_TIMER_STAT(              StatMV_GradientGen_Tks,    "GradientGen",StatMV_Display_Tks)

// MovieView Counters.
SF_DECLARE_COUNTER_STAT_SUM_GROUP(StatMV_Counters, "Counters",     StatGroup_Default)
SF_DECLARE_COUNTER_STAT(StatMV_Invoke_Cnt,         "Invoke",       StatMV_Counters)
SF_DECLARE_COUNTER_STAT(StatMV_MCAdvance_Cnt,      "MCAdvance",    StatMV_Counters)
SF_DECLARE_COUNTER_STAT(StatMV_Tessellate_Cnt,     "Tessellate",   StatMV_Counters)


//
// ***** Movie
//
Movie::Movie() : pASMovieRoot(NULL)
{
}

Movie::~Movie()
{
    SF_ASSERT(!pASMovieRoot);
}

void Movie::SetASMovieRoot(ASMovieRootBase* pasmgr)
{
    SF_ASSERT(!pASMovieRoot); // set only once!
    SF_ASSERT(pasmgr);        // not NULL
    pASMovieRoot = pasmgr;
    pASMovieRoot->AddRef();
}

int Movie::GetAVMVersion() const
{
    return pASMovieRoot->GetAVMVersion();
}

bool Movie::GetVariable(Value *pval, const char* ppathToVar) const
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariable", Amp_Native_Function_Id_GetVariable);
    return pASMovieRoot->GetVariable(pval, ppathToVar);
}

bool Movie::SetVariable(const char* ppathToVar, const Value& value, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariable", Amp_Native_Function_Id_SetVariable);
    return pASMovieRoot->SetVariable(ppathToVar, value, setType);
}

bool Movie::SetVariableArray(SetArrayType type, const char* ppathToVar,
                                        unsigned index, const void* pdata, unsigned count, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariableArray", Amp_Native_Function_Id_SetVariableArray);
    return pASMovieRoot->SetVariableArray(type, ppathToVar, index, pdata, count, setType);
}

bool Movie::SetVariableArraySize(const char* ppathToVar, unsigned count, SetVarType setType)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::SetVariableArraySize", Amp_Native_Function_Id_SetVariableArraySize);
    return pASMovieRoot->SetVariableArraySize(ppathToVar, count, setType);
}

unsigned Movie::GetVariableArraySize(const char* ppathToVar)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariableArraySize", Amp_Native_Function_Id_GetVariableArraySize);
    return pASMovieRoot->GetVariableArraySize(ppathToVar);
}

bool Movie::GetVariableArray(SetArrayType type, const char* ppathToVar,
                                        unsigned index, void* pdata, unsigned count)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::GetVariableArray", Amp_Native_Function_Id_GetVariableArray);
    return pASMovieRoot->GetVariableArray(type, ppathToVar, index, pdata, count);
}

bool Movie::Invoke(const char* pmethodName, Value *presult, const Value* pargs, unsigned numArgs)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::Invoke", Amp_Native_Function_Id_Invoke);
    return pASMovieRoot->Invoke(pmethodName, presult, pargs, numArgs);
}

bool Movie::Invoke(const char* pmethodName, Value *presult, const char* pargFmt, ...)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::Invoke", Amp_Native_Function_Id_Invoke);
    va_list args;
    bool retVal;
    va_start(args, pargFmt);
    retVal = pASMovieRoot->InvokeArgs(pmethodName, presult, pargFmt, args);
    va_end(args);
    return retVal;
}

bool Movie::InvokeArgs(const char* pmethodName, Value *presult, const char* pargFmt, va_list args)
{
    SF_AMP_SCOPE_TIMER_ID(pASMovieRoot->GetMovieImpl()->AdvanceStats, "Movie::InvokeArgs", Amp_Native_Function_Id_InvokeArgs);
    return pASMovieRoot->InvokeArgs(pmethodName, presult, pargFmt, args);
}

void Movie::CreateString(Value* pvalue, const char* pstring)
{ 
    pASMovieRoot->CreateString(pvalue, pstring); 
}
void Movie::CreateStringW(Value* pvalue, const wchar_t* pstring)
{ 
    pASMovieRoot->CreateStringW(pvalue, pstring); 
}
void Movie::CreateObject(Value* pvalue, const char* className,
                                 const Value* pargs, unsigned nargs)
{ 
    pASMovieRoot->CreateObject(pvalue, className, pargs, nargs); 
}
void Movie::CreateArray(Value* pvalue)
{ 
    pASMovieRoot->CreateArray(pvalue); 
}
void Movie::CreateFunction(Value* pvalue, FunctionHandler* pfc, void* puserData /* = NULL */)
{
    pASMovieRoot->CreateFunction(pvalue, pfc, puserData);
}

bool Movie::IsAvailable(const char* ppathToVar) const
{ 
    return pASMovieRoot->IsAvailable(ppathToVar); 
}

void Movie::SetExternalInterfaceRetVal(const Value& v)
{ 
    pASMovieRoot->SetExternalInterfaceRetVal(v); 
}

void Movie::Release()
{
    if ((AtomicOps<int>::ExchangeAdd_NoSync(&RefCount, -1) - 1) == 0)
    {
        // Since deleting the memory context releases the heap,
        // we AddRef the memory context so that it doesn't get deleted before this object
        Ptr<MemoryContext> memContext =  pASMovieRoot->GetMemoryContext();
        delete this;
        // now it's safe to delete the context
    }
}


//--------------------------------------------------------------------
//
// ***** MovieImpl
//
// Global, shared root state for a GFxMovieSub and all its characters.
MovieImpl::MovieImpl(MemoryHeap* pheap)
:       
    RenderContext(Memory::GetGlobalHeap()),
    pHeap(pheap),
    StringManager(pHeap),
    PixelScale(1.0f),
    VisibleFrameRect(0, 0, 0, 0),
    SafeRect(0, 0, 0, 0),
    pXMLObjectManager(NULL),
    BackgroundColor(0, 0, 0, 255),  
#if defined(GFX_MOUSE_SUPPORT_ENABLED) && GFX_MOUSE_SUPPORT_ENABLED != 0
    MouseCursorCount(1),
#else
    MouseCursorCount(0),
#endif
    ControllerCount(1),
    UserData(NULL),
    Flags(0),
    MovieDefKillFrameId(0)
{
    pRenderRoot = *RenderContext.CreateEntry<Render::TreeRoot>();
    hDisplayRoot= MovieDisplayHandle(pRenderRoot);
    // Capture to ensure that hDisplayHandle is immediately safe to pass to 
    // the render thread.
    RenderContext.Capture();

    pMainMovie  = 0;    

    G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);
    G_SetFlag<Flag_MovieIsFocused>(Flags, true);
    G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

    pPlayListHead = pPlayListOptHead = NULL;

    // Make sure all our allocations occur in the nested heap.
    TimeElapsed     = 0;
    TimeRemainder   = 0.0f;
    // Correct FrameTime is assigned to (1.0f / pMainMovieDef->GetFrameRate())
    // when _level0 is set/changed.
    FrameTime       = 1.0f / 12.0f;
    ForceFrameCatchUp = 0;

    // No entries in load queue.
    pLoadQueueHead  = 0;

    pLoadQueueMTHead = 0;

    // Create a delegated shared state and ensure that it has a log.
    // Delegation is configured later to Level0Def's shared state.
    pStateBag       = *SF_HEAP_NEW(pHeap) StateBagImpl(0);

#ifdef GFX_ENABLE_KEYBOARD
    for (UInt8 i = 0; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i)
        KeyboardStates[i].SetKeyboardIndex(i);
#endif
    pRetValHolder       =  SF_HEAP_NEW(pHeap) ReturnValueHolder(&StringManager);

    pFontManagerStates  = *SF_HEAP_NEW(pHeap) FontManagerStates(pStateBag);

    InstanceNameCount  = 0;

    // Viewport: un-initialized by default.
    ViewScaleX = 1.0f;
    ViewScaleY = 1.0f;
    ViewOffsetX = ViewOffsetY = 0;
    ViewScaleMode = SM_ShowAll;
    ViewAlignment = Align_Center;

    // Focus
    FocusGroupsCnt          = 1;
    memset(FocusGroupIndexes, 0, sizeof(FocusGroupIndexes));

    LastIntervalTimerId = 0;

    pIMECandidateListStyle  = NULL;

    StartTickMs = Timer::GetTicks()/1000;
    PauseTickMs = 0;

    SafeRect.Clear();

#ifdef GFX_ENABLE_SOUND
    pAudio = NULL;
    pSoundRenderer = NULL;
#endif

    // Initialized within VM-specific MovieRoot constructor.
    pObjectInterface = 0;

    LastLoadQueueEntryCnt   = 0;
    pUnloadListHead         = NULL;

    bool ampEnabled = false;
    SF_AMP_CODE(ampEnabled = AmpServer::GetInstance().IsEnabled();)
    if (ampEnabled)
    {
        AdvanceStats = *SF_HEAP_AUTO_NEW(&AmpServer::GetInstance()) AMP::ViewStats();
        AmpServer::GetInstance().AddMovie(this);
    }
    else
    {
        AdvanceStats = *SF_HEAP_NEW(pHeap) AMP::ViewStats();
    }
}

MovieImpl::~MovieImpl()
{
    ClearDrawingContextList();
    SpritesWithHitArea.Clear();
    SF_AMP_CODE(AmpServer::GetInstance().RemoveMovie(this);)

    ProcessUnloadQueue();
    pRenderRoot = NULL;

#ifndef SF_NO_IME_SUPPORT
    // clean up IME manager
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        if (pIMEManager->IsMovieActive(this))
            pIMEManager->ClearActiveMovie();
    }
    delete pIMECandidateListStyle;
#endif //#ifdef SF_NO_IME_SUPPORT
    pFontManagerStates = NULL;
    // Set our log to String Manager so that it can report leaks if they happened.
//?????    GetStringManager()->SetLeakReportLog(GetLog(), GetMovieDef()->GetFileURL());

    ShutdownTimers();

#ifdef GFX_ENABLE_VIDEO
    VideoProviders.Clear();
#endif

#ifdef GFX_ENABLE_SOUND
    InteractiveObject* psprite = GetMainMovie();
    if (psprite)
        psprite->StopActiveSounds();
#endif

    InvalidateOptAdvanceList();
    ClearDisplayList();

    ClearStickyVariables();

    delete pRetValHolder;

    // clean up threaded load queue before freeing
    LoadQueueEntryMT* plq = pLoadQueueMTHead;
    unsigned plqCount = 0;
    while (plq)
    {
        plqCount++;
        plq->Cancel();
        plq = plq->pNext;
    }
    // wait until all threaded loaders exit
    // this is required to avoid losing the movieroot
    // heap before the load tasks complete.
    unsigned plqDoneCount = 0;
    while (plqCount > plqDoneCount)
    {
        plqDoneCount = 0;
        plq = pLoadQueueMTHead;
        while (plq)
        {
            if (plq->LoadFinished())
                plqDoneCount++;
            plq = plq->pNext;
        }
    }

    // free load queue
    while(pLoadQueueHead)
    {
        // Remove from queue.
        LoadQueueEntry *pentry = pLoadQueueHead;
        pLoadQueueHead = pentry->pNext;
        delete pentry;
    }
    while(pLoadQueueMTHead)
    {
        LoadQueueEntryMT *pentry = pLoadQueueMTHead;
        pLoadQueueMTHead = pentry->pNext;
        delete pentry;
    }
    
#ifdef SF_BUILD_DEBUG
    //
    // Check if there are GFx::Values still holding refs to AS objects.
    // Warn the developer if there are any because this condition
    // will lead to crash where the Value dtor, etc. will be
    // accessing freed memory.
    //
    if (pObjectInterface && pObjectInterface->HasTaggedValues())
    {
        SF_ASSERT(0); //@TODO!
        pObjectInterface->DumpTaggedValues();
        SF_DEBUG_MESSAGE(1, "");
        SF_DEBUG_ERROR(1, "There are GFx::Values still holding references to AS Objects. If not released\n"
                           "before the MovieImpl that owns the AS objects dies, then a crash will occur\n"
                           "due to the Value trying refer to freed memory. (When the MovieImpl dies,\n"
                           "it drops the heap that contains all AS objects.) Make sure all GFx::Values that\n"
                           "hold AS object references are released before MovieImpl dies.\n");
        SF_ASSERT(0);
    }
#endif
    delete pObjectInterface;

    pASMovieRoot->Shutdown();
    pASMovieRoot->Release();
    pASMovieRoot = NULL;
    hDisplayRoot.Clear();
    pRenderRoot  = NULL;
    RenderContext.Shutdown(true);
}

void MovieImpl::ClearPlayList()
{
    // clear playlist
    for (InteractiveObject* cur = pPlayListHead, *next; cur; cur = next)
    {
        next = cur->pPlayNext;
        cur->pPlayNext = cur->pPlayPrev = cur->pPlayNextOpt = NULL;
    }
    pPlayListHead = pPlayListOptHead = NULL;
}

// Fills in a file system path relative to _level0.
bool MovieImpl::GetMainMoviePath(String *ppath) const
{       
    if (!pMainMovie)
    {
        ppath->Clear();
        return false;
    }

    // Get URL.
    *ppath = pMainMovieDef->GetFileURL();

    // Extract path by clipping off file name.
    if (!URLBuilder::ExtractFilePath(ppath))
    {
        ppath->Clear();
        return false;
    }
    return true;
}


#ifdef SF_BUILD_DEBUG
//#define SF_TRACE_COLLECTIONS
#endif


// *** Load/Unload movie support

// ****************************************************************************
// Adds load queue entry and takes ownership of it.
//
void    MovieImpl::AddLoadQueueEntry(LoadQueueEntry *pentry)
{
    pentry->EntryTime = ++LastLoadQueueEntryCnt;
    if (!pLoadQueueHead)
    {
        pLoadQueueHead = pentry;
    }
    else
    {
        // Find tail.
        LoadQueueEntry *ptail = pLoadQueueHead;
        while (ptail->pNext)    
            ptail = ptail->pNext;   

        // Insert at tail.
        ptail->pNext = pentry;
    }   
}

void    MovieImpl::AddLoadQueueEntryMT(LoadQueueEntryMT* pentryMT)
{
    pentryMT->pQueueEntry->EntryTime = ++LastLoadQueueEntryCnt;
    if (!pLoadQueueMTHead)
    {
        pLoadQueueMTHead = pentryMT;
    }
    else
    {
        // Find tail.
        LoadQueueEntryMT *ptail = pLoadQueueMTHead;
        while (ptail->GetNext())    
            ptail = ptail->GetNext();   

        // Insert at tail.
        ptail->pNext = pentryMT;
        pentryMT->pPrev = ptail;
    }   
}


MovieDefImpl*  MovieImpl::CreateImageMovieDef(
                        ImageResource *pimageResource, bool bilinear,
                        const char *purl, LoadStates *pls)
{   
    MovieDefImpl*    pdefImpl = 0; 
    Ptr<LoadStates> plsRef;

    // If load states were not passed, create them. This is necessary if
    // we are being called from sprite's attachBitmap call.
    if (!pls)
    {
        plsRef = *SF_NEW LoadStates(GetLoaderImpl(), pStateBag);
        pls = plsRef.GetPtr();
    }

    // Create a loaded image sprite based on imageInfo.
    if (pimageResource)
    {
        // No file opener here, since it was not used (ok since file openers are
        // used as a key only anyway). Technically this is not necessary since
        // image 'DataDefs' are not stored in the library right now, but the
        // argument must be passed for consistency.
        ResourceKey createKey = MovieDataDef::CreateMovieFileKey(purl, 0, 0, 0, 0);
        MemoryHeap* pheap       = GetMovieHeap();

        // Create a MovieDataDef containing our image (and an internal corresponding ShapeDef).
        Ptr<MovieDataDef> pimageMovieDataDef =
            *MovieDataDef::Create(createKey, MovieDataDef::MT_Image, purl, pheap);

        if (!pimageMovieDataDef)
            return pdefImpl;        
        pimageMovieDataDef->InitImageFileMovieDef(0, pimageResource, GetImageCreator(), bilinear);

        // We can alter load states since they are custom for out caller.
        // (ProcessLoadQueue creates a copy of LoadStates)
        pls->SetRelativePathForDataDef(pimageMovieDataDef);

        // TBD: Should we use pStateBag or its delegate??       
        pdefImpl = SF_HEAP_NEW(pheap) MovieDefImpl(pimageMovieDataDef,
                                                    pls->GetBindStates(), pls->pLoaderImpl,
                                                    Loader::LoadAll,
                                                    pStateBag->GetDelegate(),
                                                    pheap, true); // Images are fully loaded.
    }
    return pdefImpl;
}


// Processes the load queue handling load/unload instructions.  
void    MovieImpl::ProcessLoadQueue()
{
    while(pLoadQueueHead)
    {
        // Remove from queue.
        LoadQueueEntry *pentry = pLoadQueueHead;
        pLoadQueueHead = pentry->pNext;

        // Capture load states - constructor copies their values from pStateBag;
        // this means that states used for 'loadMovie' can be overridden per movie view.
        // This means that state inheritance chain (Loader -> MovieDefImpl -> MovieImpl)
        // takes effect here, so setting new states in any of above can have effect
        // on loadMovie calls. Note that this is different from import logic, which
        // always uses exactly same states as importee. Such distinction is by design.
        Ptr<LoadStates>  pls = *new LoadStates(GetLoaderImpl(), pStateBag);

        pASMovieRoot->ProcessLoadQueueEntry(pentry, pls);

        delete pentry;
    }

    LoadQueueEntryMT* pentry = pLoadQueueMTHead;
    // First we need to make sure that our queue does not have any tasks with 
    // unfinished preloading task (which usually is just searching and opening a file). 
    // We need this check to avoid a race condition like this: let say we load 
    // A.swf into _level0 and B.swf into _level1 and proloading for A.swf takes more 
    // time then for B.swf . In this case B.swf will be loaded first and then unloaded 
    // by loading A.swf into _level0. This happens because we unload a level just after preloading
    // has finished and unloading of _level0 unloads all everything in the swf movie
    while(pentry)
    {
        if (!pentry->IsPreloadingFinished())
            return;
        pentry = pentry->pNext;
    }
    // Check status of all movies that are loading through the TaskManager if we have one.
    pentry = pLoadQueueMTHead;
    while(pentry)
    {
        if (pentry->LoadFinished()) 
        {
            LoadQueueEntryMT* next = pentry->pNext;
            if (next)
                next->pPrev = pentry->pPrev;
            if (pentry->pPrev)
                pentry->pPrev->pNext = next;
            if (pLoadQueueMTHead == pentry) 
                pLoadQueueMTHead = next;
            delete pentry;
            pentry = next;
        } 
        else 
        {
            pentry = pentry->pNext;
        }
    }
}


// *** Drag State support


void MovieImpl::DragState::InitCenterDelta(bool lockCenter)
{
    LockCenter = lockCenter;

    if (!LockCenter)
    {
        typedef Matrix2F   Matrix;                     

        // We are not centering on the object, so record relative delta         
        Matrix          parentWorldMat;
        InteractiveObject*   pchar = pCharacter;
        if (pchar->GetParent())
            parentWorldMat = pchar->GetParent()->GetWorldMatrix();

        // Mouse location
        const MouseState* pmouseState = pchar->GetMovieImpl()->GetMouseState(0); // only mouse id = 0 is supp
        SF_ASSERT(pmouseState);
        PointF worldMouse(pmouseState->GetLastPosition());

        PointF parentMouse;
        // Transform to parent coordinates [world -> parent]
        parentWorldMat.TransformByInverse(&parentMouse, worldMouse);

        // Calculate the relative mouse offset, so that we can apply adjustment
        // before bounds checks.
        Matrix  local = pchar->GetMatrix();
        CenterDelta.x = local.Tx() - parentMouse.x;
        CenterDelta.y = local.Ty() - parentMouse.y;
    }
}


// *** MovieImpl's Movie interface implementation


void    MovieImpl::SetViewport(const Viewport& viewDesc)
{
    if (memcmp(&viewDesc, &mViewport, sizeof(mViewport)) == 0)
        return;

    int   prevLeft = mViewport.Left;
    int   prevTop  = mViewport.Top;
    int   prevWidth = mViewport.Width;
    int   prevHeight = mViewport.Height;
    float prevScale = mViewport.Scale;
    float prevRatio = mViewport.AspectRatio;

    G_SetFlag<Flag_ViewportSet>(Flags, true);
    mViewport        = viewDesc;

    if (mViewport.BufferHeight <= 0 || mViewport.BufferWidth <= 0)
        SF_DEBUG_WARNING(1, "MovieImpl::SetViewport: buffer size not set");

    RectF prevVisRect = VisibleFrameRect;
    UpdateViewport();

    if (prevVisRect != VisibleFrameRect ||
        (ViewScaleMode == SM_NoScale && 
            (prevWidth != mViewport.Width || prevHeight != mViewport.Height || 
             prevLeft  != mViewport.Left  || prevTop    != mViewport.Top ||
             prevScale != mViewport.Scale || prevRatio != mViewport.AspectRatio)) ||
        (ViewScaleMode != SM_ExactFit && 
            (prevWidth != mViewport.Width || 
             prevHeight != mViewport.Height || prevRatio != mViewport.AspectRatio)))
    {
        pASMovieRoot->NotifyOnResize();
    }
    pRenderRoot->SetViewport(mViewport);
    pRenderRoot->SetBackgroundColor(BackgroundColor);

    // MOOSE TODO, 3D doesn't use this, it would be better if this was part of renderHal Viewport matrix
    pRenderRoot->SetMatrix(ViewportMatrix);

}

void  MovieImpl::SetBackgroundColor(const Color color)      
{ 
    BackgroundColor = color; 
    pRenderRoot->SetBackgroundColor(BackgroundColor);
}

void  MovieImpl::SetBackgroundAlpha(float alpha)             
{ 
    BackgroundColor.SetAlpha( Alg::Clamp<UByte>((UByte)(alpha*255.0f), 0, 255) ); 
    pRenderRoot->SetBackgroundColor(BackgroundColor);
}

void    MovieImpl::UpdateViewport()
{
    RectF prevVisibleFrameRect = VisibleFrameRect;
    float prevViewOffsetX       = ViewOffsetX;
    float prevViewOffsetY       = ViewOffsetY;
    float prevViewScaleX        = ViewScaleX;
    float prevViewScaleY        = ViewScaleY;
    float prevPixelScale        = PixelScale;

    // calculate frame rect to be displayed and update    
    if (pMainMovieDef)
    {
        // We need to calculate VisibleFrameRect depending on scaleMode and alignment. Alignment is significant only for NoScale mode; 
        // otherwise it is ignored. 
        // Renderer will use VisibleFrameRect to inscribe it into Viewport's rectangle. VisibleFrameRect coordinates are in twips,
        // Viewport's coordinates are in pixels. For simplest case, for ExactFit mode VisibleFrameRect is equal to FrameSize, thus,
        // viewport is filled by the whole scene (the original aspect ratio is ignored in this case, the aspect ratio of viewport
        // is used instead).
        // ViewOffsetX and ViewOffsetY is calculated in stage pixels (not viewport's ones).

        // mViewport rectangle recalculated to twips.
        RectF viewportRect(PixelsToTwips(float(mViewport.Left)), 
            PixelsToTwips(float(mViewport.Top)), 
            PixelsToTwips(float(mViewport.Left + mViewport.Width)), 
            PixelsToTwips(float(mViewport.Top + mViewport.Height)));
        const float viewWidth = viewportRect.Width();
        const float viewHeight = viewportRect.Height();
        const float frameWidth = pMainMovieDef->GetFrameRectInTwips().Width();
        const float frameHeight = pMainMovieDef->GetFrameRectInTwips().Height();
        switch(ViewScaleMode)
        {
        case SM_NoScale:
            {
                // apply mViewport.Scale and .AspectRatio first
                const float scaledViewWidth  = viewWidth * mViewport.AspectRatio * mViewport.Scale;
                const float scaledViewHieght = viewHeight * mViewport.Scale;

                // calculate a VisibleFrameRect
                switch(ViewAlignment)
                {
                case Align_Center:
                    // We need to round centering to pixels in noScale mode, otherwise we lose pixel center alignment.
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                case Align_TopLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_TopCenter:
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_TopRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = 0;
                    break;
                case Align_BottomLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_BottomCenter:
                    VisibleFrameRect.x1 = (float)PixelsToTwips((int)TwipsToPixels(frameWidth/2 - scaledViewWidth/2));
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_BottomRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = frameHeight - scaledViewHieght;
                    break;
                case Align_CenterLeft:
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                case Align_CenterRight:
                    VisibleFrameRect.x1 = frameWidth - scaledViewWidth;
                    VisibleFrameRect.y1  = (float)PixelsToTwips((int)TwipsToPixels(frameHeight/2 - scaledViewHieght/2));
                    break;
                }
                VisibleFrameRect.SetWidth(scaledViewWidth);
                VisibleFrameRect.SetHeight(scaledViewHieght);
                ViewOffsetX = TwipsToPixels(VisibleFrameRect.x1);
                ViewOffsetY = TwipsToPixels(VisibleFrameRect.y1);
                ViewScaleX = mViewport.AspectRatio * mViewport.Scale;
                ViewScaleY = mViewport.Scale;
                break;
            }
        case SM_ShowAll: 
        case SM_NoBorder: 
            {
                const float viewWidthWithRatio = viewWidth * mViewport.AspectRatio;
                // For ShowAll and NoBorder we need to apply AspectRatio to viewWidth in order
                // to calculate correct VisibleFrameRect and scales.
                // Scale is ignored for these modes.
                if ((ViewScaleMode == SM_ShowAll && viewWidthWithRatio/frameWidth < viewHeight/frameHeight) ||
                    (ViewScaleMode == SM_NoBorder && viewWidthWithRatio/frameWidth > viewHeight/frameHeight))
                {
                    float visibleHeight = frameWidth * viewHeight / viewWidthWithRatio;    
                    VisibleFrameRect.x1 = 0;
                    VisibleFrameRect.y1 = frameHeight/2 - visibleHeight/2;
                    VisibleFrameRect.SetWidth(frameWidth);
                    VisibleFrameRect.SetHeight(visibleHeight);

                    ViewOffsetX = 0;
                    ViewOffsetY = TwipsToPixels(VisibleFrameRect.y1);
                    ViewScaleX = viewWidth ? (frameWidth / viewWidth) : 0.0f;
                    ViewScaleY = ViewScaleX / mViewport.AspectRatio;
                }
                else 
                {
                    float visibleWidth = frameHeight * viewWidthWithRatio / viewHeight;    
                    VisibleFrameRect.x1 = frameWidth/2 - visibleWidth/2;
                    VisibleFrameRect.y1 = 0;
                    VisibleFrameRect.SetWidth(visibleWidth);
                    VisibleFrameRect.SetHeight(frameHeight);

                    ViewOffsetX = TwipsToPixels(VisibleFrameRect.x1);
                    ViewOffsetY = 0;
                    ViewScaleY = viewHeight ? (frameHeight / viewHeight) : 0.0f;
                    ViewScaleX = ViewScaleY * mViewport.AspectRatio;
                }
            }
            break;
        case SM_ExactFit: 
            // AspectRatio and Scale is ignored for this mode.
            VisibleFrameRect.x1 = VisibleFrameRect.y1 = 0;
            VisibleFrameRect.SetWidth(frameWidth);
            VisibleFrameRect.SetHeight(frameHeight);
            ViewOffsetX = ViewOffsetY = 0;
            ViewScaleX = viewWidth  ? (VisibleFrameRect.Width() / viewWidth) : 0.0f;
            ViewScaleY = viewHeight ? (VisibleFrameRect.Height() / viewHeight) : 0.0f;
            break;
        }
        PixelScale = Alg::Max((ViewScaleX == 0) ? 0.005f : 1.0f/ViewScaleX, 
                           (ViewScaleY == 0) ? 0.005f : 1.0f/ViewScaleY);    
    }
    else
    {
        ViewOffsetX = ViewOffsetY = 0;
        ViewScaleX = ViewScaleY = 1.0f;
        PixelScale = SF_TWIPS_TO_PIXELS(20.0f);
    }
    //  RendererInfo.ViewportScale = PixelScale;

    ViewportMatrix = Render::Matrix2F::Translation(-VisibleFrameRect.x1, -VisibleFrameRect.y1);
    ViewportMatrix.AppendScaling(mViewport.Width / VisibleFrameRect.Width(),
                                 mViewport.Height / VisibleFrameRect.Height());
    //ViewportMatrix.AppendTranslation(float(mViewport.Left)+ViewOffsetX, float(mViewport.Top)+ViewOffsetY);
    //ViewportMatrix.AppendTranslation(-ViewOffsetX, -ViewOffsetY);
  //  RendererInfo.ViewportScale = PixelScale;

    if (prevVisibleFrameRect != VisibleFrameRect || prevViewOffsetX != ViewOffsetX ||
        prevViewOffsetY      != ViewOffsetY      || prevViewScaleX  != ViewScaleX  ||
        prevViewScaleY       != ViewScaleY       || prevPixelScale  != PixelScale)
    {
        SetDirtyFlag();
        UpdateViewAndPerspective();
    }
}

void MovieImpl::MakeViewAndPersp3D(Matrix3F *matView, Matrix4F *matPersp, 
                                   float DisplayWidth, float DisplayHeight, PointF Center,
                                   float perspFOV,                        // degrees
                                   bool bInvertY)
{
    const float nearZ = 1;
    const float farZ  = 100000;

    // calculate view matrix
    float fovYAngle = (float)SF_DEGTORAD(perspFOV);
    float pixelPerUnitRatio = DisplayWidth*.5f;    
    float eyeZ = pixelPerUnitRatio/tanf(fovYAngle/2.f);

    // compute view matrix
    Point3F vEyePt( Center.x, Center.y, (-eyeZ < -farZ) ? -farZ : -eyeZ);
    Point3F vLookatPt(Center.x, Center.y, 0);
    Point3F vUpVec( 0, bInvertY ? 1.f : -1.f, 0 );
    if (bInvertY)
        matView->ViewLH(vEyePt, vLookatPt, vUpVec );
    else
        matView->ViewRH(vEyePt, vLookatPt, vUpVec );

    // compute perspective matrix    
    if (bInvertY) 
        matPersp->PerspectiveFocalLengthLH(eyeZ, DisplayWidth, DisplayHeight, nearZ, farZ);
    else
        matPersp->PerspectiveFocalLengthRH(eyeZ, DisplayWidth, DisplayHeight, nearZ, farZ);
}

// recompute view and perspective if necessary (usually when visible frame rect changes)
void MovieImpl::UpdateViewAndPerspective()        
{ 
    if (pRenderRoot)
    {
        Matrix3F view;
        Matrix4F proj;
        
        RectF vfr = GetVisibleFrameRectInTwips();
        float displayWidth  = fabsf(vfr.Width());     
        float displayHeight = fabsf(vfr.Height());      
        float centerX       = (vfr.x1 + vfr.x2)/2.f;
        float centerY       = (vfr.y1 + vfr.y2)/2.f;

        MakeViewAndPersp3D(&view, &proj, displayWidth, displayHeight, PointF(centerX, centerY)); 

        // store at movie level
        SetViewMatrix3D(view);
        SetProjectionMatrix3D(proj);

        // account for 2D viewport mat set at the root
        Matrix3F viewportInv = Matrix3F::Translation(VisibleFrameRect.x1, VisibleFrameRect.y1, 0) * 
            Matrix3F::Scaling(1.0f / (mViewport.Width / VisibleFrameRect.Width()),
            1.f / (mViewport.Height / VisibleFrameRect.Height()), 1.f);
        view =  view * viewportInv;

        pRenderRoot->SetViewMatrix3D(view);
        pRenderRoot->SetProjectionMatrix3D(proj);
    }
#if 0
    for (unsigned movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++) 
    {
        InteractiveObject* pmovie = MovieLevels[movieIndex].pSprite;
        if (pmovie)
            pmovie->UpdateViewAndPerspective();
    }
#endif
}

void    MovieImpl::GetViewport(Viewport *pviewDesc) const
{
    *pviewDesc = mViewport;
}

void    MovieImpl::SetViewScaleMode(ScaleModeType scaleMode)
{
    ViewScaleMode = scaleMode;
    SetViewport(mViewport);
}

void    MovieImpl::SetViewAlignment(AlignType align)
{
    ViewAlignment = align;
    SetViewport(mViewport);
}

InteractiveObject* MovieImpl::GetTopMostEntity(const PointF& mousePos, 
                                               unsigned controllerIdx, 
                                               bool testAll, 
                                               const InteractiveObject* ignoreMC)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::GetTopMostEntity");

    float w = VisibleFrameRect.Width();
    float h = VisibleFrameRect.Height();
    float nsx = 2.f * ((mousePos.x - PixelsToTwips(ViewOffsetX)) / w) - 1.0f;
    float nsy = 2.f * ((mousePos.y - PixelsToTwips(ViewOffsetY)) / h) - 1.0f;
    ScreenToWorld.SetNormalizedScreenCoords(nsx, nsy);
    ScreenToWorld.SetProjection(*GetProjectionMatrix3D());
    ScreenToWorld.SetView(*GetViewMatrix3D());

    InteractiveObject* ptopMouseCharacter = 0;
    // look for chars marked as topmostLevel first.
    for (int i = (int)TopmostLevelCharacters.GetSize() - 1; i >= 0; --i)
    {
        DisplayObjectBase* pch = TopmostLevelCharacters[i];
        SF_ASSERT(pch);

        Matrix2F matrix(pch->GetParent()->GetWorldMatrix());
        PointF pp = matrix.TransformByInverse(mousePos);

        // need to test with 'testAll' = true first to detect any overlappings
        DisplayObjectBase::TopMostDescr descr;
        descr.pIgnoreMC = ignoreMC;
        descr.TestAll   = testAll;
        descr.ControllerIdx = controllerIdx;
        if (pch->GetTopMostMouseEntity(pp, &descr) == DisplayObjectBase::TopMost_Found)
        {
            SF_ASSERT(descr.pResult);
            ptopMouseCharacter = descr.pResult;
            break;
        }
    }
    if (!ptopMouseCharacter)
    {        
        for (int movieIndex = (int)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
        {   
            InteractiveObject* pmovie  = MovieLevels[movieIndex-1].pSprite;
            DisplayObjectBase::TopMostDescr descr;
            descr.pIgnoreMC = ignoreMC;
            descr.TestAll   = testAll;
            descr.ControllerIdx = controllerIdx;
            if (pmovie->GetTopMostMouseEntity(mousePos, &descr) == DisplayObjectBase::TopMost_Found)
            {
                SF_ASSERT(descr.pResult);
                ptopMouseCharacter = descr.pResult;
                break;
            }
        }
    }
    return ptopMouseCharacter;
}

template<typename T>
struct PtrGuard
{
    PtrGuard(T** ptr) : Ptr(ptr) {}
    ~PtrGuard() { if(*Ptr)  {  (*Ptr)->Release();  *Ptr = NULL; } }
    T** Ptr;
};

float MovieImpl::Advance(float deltaT, unsigned frameCatchUpCount, bool capture)
{    
    if (G_IsFlagSet<Flag_Paused>(Flags))
    {   // Call capture anyway, since Invoke could've changed things.
        if (capture)
            MovieImpl::Capture();
        return 0.05f;
    }

    if (!pMainMovie)
    {   // Call capture to clear out content in case Invoke caused movie unload.
        if (capture)
            MovieImpl::Capture();
        return pMainMovieDef ? (1.0f / pMainMovieDef->GetFrameRate()) : 0.0f;
    }
    // DBG
    //printf("MovieImpl::Advance %d   -------------------\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());

    ProcessMovieDefToKillList();

#if defined(SF_ENABLE_STATS)
    AmpFunctionTimer advanceTimer(AdvanceStats, "MovieImpl::Advance", Amp_Native_Function_Id_Advance);
#endif

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Index used in iterating through root movies.
    unsigned   movieIndex;

    if (deltaT < 0.0f)
    {
        SF_DEBUG_WARNING(1, "Movie::Advance - negative deltaT argument specified");
        deltaT = 0.0f;
    }

    G_SetFlag<Flag_AdvanceCalled>(Flags, true);

    // Advance - Cache states for faster non-locked access.
    Ptr<ActionControl>           pac;

    State*                        pstates[10]    = {0,0,0,0,0,0,0,0,0,0};
    const static State::StateType stateQuery[10] =
    { State::State_UserEventHandler, State::State_FSCommandHandler,
      State::State_ExternalInterface, State::State_ActionControl,
      State::State_Log, State::State_FontLib, State::State_FontMap,
      State::State_FontProvider, State::State_Translator,
      State::State_Audio // Not obtained if no sound.
    };
    // Get multiple states at once to avoid extra locking.
#ifdef GFX_ENABLE_SOUND   
    pStateBag->GetStatesAddRef(pstates, stateQuery, 10);
#else
    pStateBag->GetStatesAddRef(pstates, stateQuery, 9);
#endif
  
    Ptr<LogState> logState;

    pUserEventHandler   = *(UserEventHandler*)  pstates[0];
    pFSCommandHandler   = *(FSCommandHandler*)  pstates[1];
    pExtIntfHandler     = *(ExternalInterface*) pstates[2];
    pac                 = *(ActionControl*)     pstates[3];
    logState            = *(LogState*)          pstates[4];
    pCachedLog = logState ? logState->GetLog() : 0;

    // Check if FontLib, FontMap, FontProvider or Translator is changed
    UInt8 stateChangeFlag = 0u;
    {
        Ptr<FontLib>      fl = *(FontLib*)            pstates[5];
        Ptr<FontMap>      fm = *(FontMap*)            pstates[6];
        Ptr<FontProvider> fp = *(FontProvider*)       pstates[7];
        Ptr<Translator>   tr = *(Translator*)         pstates[8];
        stateChangeFlag = pFontManagerStates->CheckStateChange(fl,fm,fp,tr);
    }

    // Set flag indicating that the log is cached.
    G_SetFlag<Flag_CachedLogFlag>(Flags, true);
    
    if (pac)
    {
        G_SetFlag<Flag_VerboseAction>(Flags, (pac->GetActionFlags() & ActionControl::Action_Verbose) != 0);
        G_SetFlag<Flag_SuppressActionErrors>(Flags, (pac->GetActionFlags() & ActionControl::Action_ErrorSuppress) != 0);
        G_SetFlag<Flag_LogRootFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LogRootFilenames) != 0);
        G_SetFlag<Flag_LogLongFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LongFilenames) != 0);
        G_SetFlag<Flag_LogChildFilenames>(Flags, (pac->GetActionFlags() & ActionControl::Action_LogChildFilenames) != 0);
    }
    else
    {
        G_SetFlag<Flag_VerboseAction>(Flags, 0);
        G_SetFlag<Flag_SuppressActionErrors>(Flags, 0);
        G_SetFlag<Flag_LogRootFilenames>(Flags, 0);
        G_SetFlag<Flag_LogLongFilenames>(Flags, 0);
        G_SetFlag<Flag_LogChildFilenames>(Flags, 0);
    }

#ifdef GFX_ENABLE_SOUND
    pAudio = (AudioBase*) pstates[9];
    PtrGuard<AudioBase> as_ptr(&pAudio);
    if (pAudio)
    {
        pSoundRenderer = pAudio->GetRenderer();
        if(pSoundRenderer)
            pSoundRenderer->AddRef();
    }
    PtrGuard<Sound::SoundRenderer> sr_ptr(&pSoundRenderer);
#endif    
    // Capture frames loaded so that the value of a loaded frame will be consistent
    // for all root nodes that share the same MovieDefImpl. This ensures that if
    // there 'loadMovie' is used several times on the same progressively loaded SWF,
    // all instances will see the same progress state.

    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {        
        // Bytes loaded must always be grabbed before the frame, since there is a data
        // race between value returned 'getBytesLoaded' and the frame we can seek to.
        if (!pdefNode->ImportFlag)
        {
            pdefNode->BytesLoaded  = pdefNode->pDefImpl->GetBytesLoaded();
            pdefNode->LoadingFrame = pdefNode->pDefImpl->GetLoadingFrame();
        }

        // if FontLib, FontMap, FontProvider or Translator is changed we need to 
        // to clean FontManager cache
        if (stateChangeFlag)
            pdefNode->pFontManager->CleanCache();

        pdefNode = pdefNode->pNext;
    }

    // Notify TextField that states changed
    if (stateChangeFlag)
    {
        for (unsigned i = 0; i < MovieLevels.GetSize(); i++)
            MovieLevels[i].pSprite->SetStateChangeFlags(stateChangeFlag);
    }

    // Shadowed value from GFxSprite::GetLoadingFrame would have been grabbed in
    // the RootMovieDefNodes traversal above, so it must come after it.
    if ((pMainMovie->GetLoadingFrame() == 0))
    {
        if (capture)
            MovieImpl::Capture();
        G_SetFlag<Flag_CachedLogFlag>(Flags, 0);
        return 0;
    }

    // Execute loading/frame0 events for root level clips, if necessary.
    if (G_IsFlagSet<Flag_LevelClipsChanged>(Flags) && (pMainMovie->GetLoadingFrame() > 0))
    {
        G_SetFlag<Flag_LevelClipsChanged>(Flags, false);      
        // Queue up load events, tags and actions for frame 0.
        for (movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
        {
            // ExecuteFrame0Events does an internal check, so this will only have an effect once.
            MovieLevels[movieIndex-1].pSprite->ExecuteFrame0Events();
        }

        // It is important to execute frame 0 events here before we get to AdvanceFrame() below,
        // so that actions like stop() are applied in timely manner.
        pASMovieRoot->DoActions();
        ProcessUnloadQueue();
        ProcessLoadQueue();
    }

    // *** Advance the frame based on time

    TimeElapsed += UInt64(deltaT * 1000);
    TimeRemainder += deltaT;    

//if (TimeRemainder >= FrameTime)
//    MovieLevels[0].pSprite->DumpDisplayList(0, "_LEVEL");

    UInt64 advanceStart = Timer::GetProfileTicks();
    float minDelta = FrameTime;

    if (IntervalTimers.GetSize() > 0)
    {
        UPInt i, n;
        unsigned needCompress = 0;
        for (i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
        {
            if (IntervalTimers[i] && IntervalTimers[i]->IsActive())
            {
                IntervalTimers[i]->Invoke(this, FrameTime);

                float delta = float((IntervalTimers[i]->GetNextInvokeTime() - TimeElapsed))/1000.f;
                if (delta < minDelta)
                    minDelta = delta;
            }
            else
                ++needCompress;
        }
        if (needCompress)
        {
            n = IntervalTimers.GetSize(); // size could be changed after Invoke
            unsigned j;
            // remove empty entries
            for (i = j = 0; i < n; ++i)
            {
                if (!IntervalTimers[j] || !IntervalTimers[j]->IsActive())
                {
                    IntervalTimers[j]->Clear();
                    IntervalTimers.RemoveAt(j);
                }
                else
                    ++j;
            }
        }
    }
#ifdef GFX_ENABLE_VIDEO
    if (VideoProviders.GetSize() > 0)
    {
        Array<VideoProvider*> remove_list;
        for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
        {
            Ptr<VideoProvider> pvideo = *it;
            // we have to call Progress method here and not from VideoObject AdvanceFrame method 
            // because there could be videos that are not attached to any VideoObject but still they 
            // have to progress (only sound in this case is produced)
            pvideo->Advance();
            if (pvideo->IsActive())
            {
                if( pvideo->GetFrameTime() < minDelta)
                    minDelta = pvideo->GetFrameTime();
            }
            else
                remove_list.PushBack(pvideo);
        }
        for(UPInt i = 0; i < remove_list.GetSize(); ++i)
            VideoProviders.Remove(remove_list[i]);
    }
#endif
#ifdef GFX_ENABLE_SOUND
    if (pSoundRenderer)
    {
        float delta = pSoundRenderer->Update();
        if (delta < minDelta)
            minDelta = delta;
    }
#endif

    ProcessInput();
   
    if (TimeRemainder >= FrameTime)
    {
        // Mouse, keyboard and timer handlers above may queue up actions which MUST be executed before the
        // Advance. For example, if a mouse event contains "gotoAndPlay(n)" action and Advance is
        // going to queue up a frame with "stop" action, then that gotoAndPlay will be incorrectly stopped.
        // By executing actions before the Advance we avoid queuing the stop-frame and gotoAndPlay will
        // work correctly. (AB, 12/13/06)
        pASMovieRoot->DoActions();

        bool frameCatchUp = frameCatchUpCount > 0 || ForceFrameCatchUp > 0;
        do
        {
            if (frameCatchUp)
                TimeRemainder -= FrameTime;
            else
                TimeRemainder = (float)fmod(TimeRemainder, FrameTime);

            float framePos = (TimeRemainder >= FrameTime) ? 0.0f : (TimeRemainder / FrameTime);

            // Advance a frame.
            {
                AdvanceFrame(1, framePos);
#ifdef GFX_ENABLE_VIDEO
                // we need to call advance for video players here also because when we synchronize
                // swf timeline and video frames we need to notify video players that our the timer has changed.
                if (VideoProviders.GetSize() > 0)
                {
                    Array<VideoProvider*> remove_list;
                    for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
                    {
                        Ptr<VideoProvider> pvideo = *it;
                        // we have to call Progress method here and not from VideoObject AdvanceFrame method 
                        // because there could be videos that are not attached to any VideoObject but still they 
                        // have to progress (only sound in this case is produced)
                        pvideo->Advance();
                        if (pvideo->IsActive())
                        {
                            if( pvideo->GetFrameTime() < minDelta)
                                minDelta = pvideo->GetFrameTime();
                        }
                        else
                            remove_list.PushBack(pvideo);
                    }
                    for(UPInt i = 0; i < remove_list.GetSize(); ++i)
                        VideoProviders.Remove(remove_list[i]);
                }
#endif
            }

            // Execute actions queued up due to actions and mouse.
            pASMovieRoot->DoActions();
            ProcessUnloadQueue();
            ProcessLoadQueue();
            if (ForceFrameCatchUp > 0)
                ForceFrameCatchUp--;

        } while((frameCatchUpCount-- > 0 && TimeRemainder >= FrameTime) || ForceFrameCatchUp > 0);
        // Force GetTopmostMouse update in next Advance so that buttons detect change, if any.
        G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);

        // Let refcount collector to do its job
        pASMovieRoot->AdvanceFrame(true);
    }
    else
    {
        // Fractional advance only.
        float framePos = TimeRemainder / FrameTime;
        AdvanceFrame(0, framePos);

        TimeRemainder = (float)fmod(TimeRemainder, FrameTime);

        // Technically AdvanceFrame(0) above should not generate any actions.
        // However, we need to execute actions queued up due to mouse.
        pASMovieRoot->DoActions();
        ProcessUnloadQueue();
        ProcessLoadQueue();
        pASMovieRoot->AdvanceFrame(false);
    }    

    UpdateAllDrawingContexts();

    ResetTabableArrays();
    G_SetFlag<Flag_CachedLogFlag>(Flags, false);

    // DBG
    //printf("MovieImpl::Advance %d   ^^^^^^^^^^^^^^^^^\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());
    UInt64 advanceStop = Timer::GetProfileTicks();
    minDelta -= (advanceStop - advanceStart)/1000000.0f;
    if (minDelta < 0.0f)
        minDelta = 0.0f;

    if (capture)
        MovieImpl::Capture();
    return Alg::Min(minDelta, FrameTime - TimeRemainder);
}


void MovieImpl::ProcessInput()
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessInput", Amp_Native_Function_Id_ProcessInput);

#if defined(GFX_ENABLE_KEYBOARD) || defined(GFX_ENABLE_MOUSE)
    // *** Handle keyboard / mice
    if (pMainMovie) 
    {
        // mouse event handling is different between AVM 1 and 2: for AS3 (avm2)
        // we need to call GetTopMostEntity with the parameter "testAll" set to true, thus,
        // it will test all objects, not only ones having button events.
        bool avm2 = (pASMovieRoot->GetAVMVersion() == 2);
        SF_UNUSED(avm2);

        ProcessFocusKeyInfo focusKeyInfo;
        UInt32              miceProceededMask = 0;
        UInt32              miceSupportedMask = ((1U << MouseCursorCount) - 1);

        while (!InputEventsQueue.IsQueueEmpty())
        {
            const InputEventsQueue::QueueEntry* qe = InputEventsQueue.GetEntry();
            if (qe->IsKeyEntry())
            {
                ProcessKeyboard(qe, &focusKeyInfo);
            }
            else if (qe->IsMouseEntry())
            {
                ProcessMouse(qe, &miceProceededMask, avm2);
            }
        }

        if (G_IsFlagSet<Flag_NeedMouseUpdate>(Flags) && (miceProceededMask & miceSupportedMask) != miceSupportedMask)
        {
            for (unsigned mi = 0, mask = 1; mi < MouseCursorCount; ++mi, mask <<= 1)
            {
                if (!(miceProceededMask & mask))
                {
                    if (!mMouseState[mi].IsActivated())
                        continue; // this mouse is not activated yet
                    mMouseState[mi].ResetPrevButtonsState();
                    Ptr<InteractiveObject> ptopMouseCharacter = 
                        GetTopMostEntity(mMouseState[mi].GetLastPosition(), mi, avm2);
                    mMouseState[mi].SetTopmostEntity(ptopMouseCharacter);    

                    // check for necessity of changing cursor
                    CheckMouseCursorType(mi, ptopMouseCharacter);

                    pASMovieRoot->GenerateMouseEvents(mi);
                }
            }
        }

        FinalizeProcessFocusKey(&focusKeyInfo);
        G_SetFlag<Flag_NeedMouseUpdate>(Flags, false);
    }
#endif // GFX_ENABLE_KEYBOARD || GFX_ENABLE_MOUSE
}

void MovieImpl::ProcessKeyboard(const InputEventsQueue::QueueEntry* qe, 
                                ProcessFocusKeyInfo* focusKeyInfo)
{
#ifdef GFX_ENABLE_KEYBOARD

    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::ProcessKeyboard");

    int                 keyMask = 0;
    const InputEventsQueue::QueueEntry::KeyEntry& keyEntry = qe->GetKeyEntry();
    // keyboard
    if (keyEntry.Code != 0)
    {
        // key down / up
        EventId::IdCode eventIdCode;
        Event::EventType event;
        if (keyEntry.KeyIsDown)
        {
            eventIdCode = EventId::Event_KeyDown;
            event       = Event::KeyDown;
        }
        else
        {
            eventIdCode = EventId::Event_KeyUp;
            event       = Event::KeyUp;
        }
        EventId eventId
            ((UByte)eventIdCode, 
            keyEntry.Code, 
            keyEntry.AsciiCode, 
            keyEntry.WcharCode, keyEntry.KeyboardIndex);
        eventId.KeysState = keyEntry.KeysState;

        //printf ("Key %d (ASCII %d) is %s\n", code, 
        // asciiCode, (event == Event::KeyDown) ? "Down":"Up");

        for (unsigned movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
            MovieLevels[movieIndex-1].pSprite->PropagateKeyEvent(eventId, &keyMask);

        const KeyboardState* keyboardState = GetKeyboardState(keyEntry.KeyboardIndex);
        SF_ASSERT(keyboardState);
        keyboardState->NotifyListeners
            (pMainMovie, 
            keyEntry.Code, 
            keyEntry.AsciiCode, 
            keyEntry.WcharCode, 
            event);

        if (!IsDisableFocusKeys())
            ProcessFocusKey(event, keyEntry, focusKeyInfo);
    }
    else if (keyEntry.WcharCode != 0)
    {
        // char
        FocusGroupDescr& focusGroup = GetFocusGroup(keyEntry.KeyboardIndex);
        Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;
        if (curFocused)
        {
            curFocused->OnCharEvent(keyEntry.WcharCode, keyEntry.KeyboardIndex);
        }
    }
#else
    SF_UNUSED(psc);
#endif //SF_NO_KEYBOARD_SUPPORT

}

void MovieImpl::ProcessMouse(const InputEventsQueue::QueueEntry* qe, UInt32* miceProceededMask, bool avm2)
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::ProcessMouse", Amp_Native_Function_Id_ProcessMouse);

    // Proceed mouse
    *miceProceededMask |= (1 << qe->GetMouseEntry().MouseIndex);

    unsigned mi = qe->GetMouseEntry().MouseIndex;
    mMouseState[mi].UpdateState(*qe);

    Ptr<InteractiveObject> ptopMouseCharacter = 
        GetTopMostEntity(qe->GetMouseEntry().GetPosition(), mi, avm2);
    mMouseState[mi].SetTopmostEntity(ptopMouseCharacter);    

    /*//Debug code
    static unsigned lastState = 0;
    printf("Mouse Position: %5.2f, %5.2f\n", qe->GetMouseEntry().GetPosition().x, qe->GetMouseEntry().GetPosition().y );
    if (mMouseState[mi].GetButtonsState() != (lastState & 1)) 
    {
        if (ptopMouseCharacter)  //?
            printf("! %s\n", ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
        else
            printf("! NONE!\n");
    } 
    if (ptopMouseCharacter)  //?
    {
        printf("? %s\n", ptopMouseCharacter->GetCharacterHandle()->GetNamePath().ToCStr());
    }
    lastState = mMouseState[mi].GetButtonsState();
	*/

#ifndef SF_NO_IME_SUPPORT
    unsigned buttonsState = mMouseState[mi].GetButtonsState();
    // notify IME about mouse down/up
    if (qe->GetMouseEntry().IsButtonsStateChanged())
    {
        Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
        if (pIMEManager && pIMEManager->IsMovieActive(this))
        {
            if (qe->GetMouseEntry().IsAnyButtonPressed())
                pIMEManager->OnMouseDown(this, buttonsState, ptopMouseCharacter);
            else
                pIMEManager->OnMouseUp(this, buttonsState, ptopMouseCharacter);
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT

    // Send mouse events.
    EventId::IdCode buttonEvent = EventId::Event_Invalid;
    if (qe->GetMouseEntry().IsButtonsStateChanged() && qe->GetMouseEntry().IsLeftButton())
    {
        buttonEvent = (qe->GetMouseEntry().IsAnyButtonPressed()) ?
            EventId::Event_MouseDown : EventId::Event_MouseUp;
    }

    for (unsigned movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
    {
        InteractiveObject* pmovie = MovieLevels[movieIndex-1].pSprite;
        ButtonEventId evt(buttonEvent, mi);
        evt.MouseWheelDelta = qe->GetMouseEntry().WheelScrollDelta;

        // Send mouse up/down events.
        if (buttonEvent != EventId::Event_Invalid)
            pmovie->PropagateMouseEvent(ButtonEventId(buttonEvent, mi));
        // Send move.
        if (mMouseState[mi].IsMouseMoved())
        {
            pmovie->PropagateMouseEvent(ButtonEventId(EventId::Event_MouseMove, mi));
        }
    }
    if ((!IsDisableFocusAutoReleaseByMouseClick() && qe->GetMouseEntry().IsButtonsStateChanged()) ||
        (!IsDisableFocusAutoRelease() && mMouseState[mi].IsMouseMoved()))
    {
        HideFocusRect(mi);
    }

    if (qe->GetMouseEntry().IsMouseWheel())
    {
        // no need to propagate the mouse wheel, just sent 
        if (ptopMouseCharacter)
            ptopMouseCharacter->OnMouseWheelEvent(qe->GetMouseEntry().WheelScrollDelta);
    }

    pASMovieRoot->NotifyMouseEvent(qe, mMouseState[mi], mi);

    // check for necessity of changing cursor
    CheckMouseCursorType(mi, ptopMouseCharacter);

    // Send onKillFocus, if necessary
    if (qe->GetMouseEntry().IsAnyButtonPressed() && qe->GetMouseEntry().IsLeftButton())
    {
        Ptr<InteractiveObject> curFocused = GetFocusGroup(mi).LastFocused;
        if (ptopMouseCharacter != curFocused)
        {
            QueueSetFocusTo(ptopMouseCharacter, ptopMouseCharacter, mi, GFx::GFx_FocusMovedByMouse);
        }
    }

    //!AB: button's events, such as onPress/onRelease should be fired AFTER
    // appropriate mouse events (onMouseDown/onMouseUp).
    pASMovieRoot->GenerateMouseEvents(mi);
}

void MovieImpl::AdvanceFrame(bool nextFrame, float framePos)
{
    SF_AMP_SCOPE_TIMER_ID(AdvanceStats, "MovieImpl::AdvanceFrame", Amp_Native_Function_Id_AdvanceFrame);

#ifdef SF_ADVANCE_EXECUTION_TRACE
    bool first = true;
#endif
    if (nextFrame)
        pASMovieRoot->OnNextFrame();

    // If optimized advance list is invalidated then we need to 
    // recreate it. We need a separate loop iteration, since call to Advance
    // may modify the optimized advance list and this may cause incorrect behavior.
    if (1)//(IsOptAdvanceListInvalid())
    {
        pPlayListOptHead = NULL;
        G_SetFlag<Flag_OptimizedAdvanceListInvalid>(Flags, false);
        //Ptr<InteractiveObject> pnextCh;// = NULL;
        InteractiveObject* pnextCh;// = NULL;
        for (InteractiveObject* pcurCh = pPlayListHead; pcurCh; pcurCh = pnextCh)
        {
            pnextCh = pcurCh->pPlayNext;
            pcurCh->ClearOptAdvancedListFlag();
            pcurCh->pPlayNextOpt = NULL;

            // "unloaded" chars should be already removed.
            SF_ASSERT(!pcurCh->IsUnloaded());

            if (pcurCh->CanAdvanceChar())
            {
                if (pcurCh->CheckAdvanceStatus(false) == 1)
                    pcurCh->AddToOptimizedPlayList();
                if (nextFrame || pcurCh->IsReqPartialAdvanceFlagSet())
                {
#ifdef SF_ADVANCE_EXECUTION_TRACE
                    if (first)
                        printf("\n");
                    first = false;
                    String s;
                    //printf("!------------- %s\n", pcurCh->GetCharacterHandle()->GetNamePath().ToCStr());
                    printf("!------------- %s\n", pcurCh->GetAbsolutePath(&s));
#endif

                    pcurCh->AdvanceFrame(nextFrame, framePos);          
                }
            }
        }
    }
    else
    {
        InteractiveObject* pnextCh = NULL;
        for (InteractiveObject* pcurCh = pPlayListOptHead; pcurCh; pcurCh = pnextCh)
        {
            SF_ASSERT(!pcurCh->IsUnloaded());
            SF_ASSERT(pcurCh->IsOptAdvancedListFlagSet());

            pnextCh = pcurCh->pPlayNextOpt;

            if (pcurCh->CanAdvanceChar() && (nextFrame || pcurCh->IsReqPartialAdvanceFlagSet()))
            {
#ifdef SF_ADVANCE_EXECUTION_TRACE
                if (first)
                    printf("\n");
                first = false;
                printf("+------------- %s\n", pcurCh->GetCharacterHandle()->GetNamePath().ToCStr());
#endif
                pcurCh->AdvanceFrame(nextFrame, framePos);          
            }
        }
    }

#ifdef SF_ADVANCE_EXECUTION_TRACE
    if (!first)
        printf("\n");
#endif
}

void MovieImpl::CheckMouseCursorType(unsigned mouseIdx, InteractiveObject* ptopMouseCharacter)
{
    if (mMouseState[mouseIdx].IsTopmostEntityChanged())
    {
        unsigned newCursorType = MouseCursorEvent::ARROW;
        if (ptopMouseCharacter)
            newCursorType = ptopMouseCharacter->GetCursorType();
        ChangeMouseCursorType(mouseIdx, newCursorType);
    }
}

void MovieImpl::ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType)
{
    unsigned prevCursor = mMouseState[mouseIdx].GetCursorType();
    mMouseState[mouseIdx].SetCursorType(newCursorType);
    if (prevCursor != mMouseState[mouseIdx].GetCursorType())
        pASMovieRoot->ChangeMouseCursorType(mouseIdx,  mMouseState[mouseIdx].GetCursorType());
}

unsigned  MovieImpl::HandleEvent(const Event &event)
{
    if (!IsMovieFocused() && event.Type != Event::SetFocus)
        return HE_NotHandled;

    union
    {
        const Event *        pevent;
        const KeyEvent *     pkeyEvent;
        const MouseEvent *   pmouseEvent;
        const CharEvent *    pcharEvent;
#ifndef SF_NO_IME_SUPPORT
        const IMEEvent *     pimeEvent;
#endif
        const SetFocusEvent* pfocusEvent;
    };
    pevent = &event;

    // handle set focus event for movie
    if (event.Type == Event::SetFocus)
    {
#ifdef GFX_ENABLE_KEYBOARD
        // Handle special keys state, if initialized
        for (unsigned i = 0; i < GFX_MAX_KEYBOARD_SUPPORTED; ++i)
        {
            if (pfocusEvent->KeysStates[i].IsInitialized())
            {
                KeyboardState* keyboardState = GetKeyboardState(i);
                SF_ASSERT(keyboardState);
                keyboardState->SetKeyToggled(Key::NumLock, 
                    pfocusEvent->KeysStates[i].IsNumToggled());
                keyboardState->SetKeyToggled(Key::CapsLock, 
                    pfocusEvent->KeysStates[i].IsCapsToggled());
                keyboardState->SetKeyToggled(Key::ScrollLock, 
                    pfocusEvent->KeysStates[i].IsScrollToggled());
            }
        }
#endif // GFX_ENABLE_KEYBOARD
        OnMovieFocus(true);
        return HE_Handled;
    }
    else if (event.Type == Event::KeyDown || event.Type == Event::KeyUp)
    {
#ifdef GFX_ENABLE_KEYBOARD
        // Handle special keys state, if initialized
        KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
        SF_DEBUG_WARNING(!keyboardState, "KeyEvent contains wrong index of keyboard");
        if (keyboardState && pkeyEvent->Modifiers.IsInitialized())
        {
            keyboardState->SetKeyToggled(Key::NumLock, 
                pkeyEvent->Modifiers.IsNumToggled());
            keyboardState->SetKeyToggled(Key::CapsLock, 
                pkeyEvent->Modifiers.IsCapsToggled());
            keyboardState->SetKeyToggled(Key::ScrollLock, 
                pkeyEvent->Modifiers.IsScrollToggled());
        }
#endif // GFX_ENABLE_KEYBOARD
    }

    // Process the event type

    switch(event.Type)
    {
        // Mouse.
    case Event::MouseMove:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
                      PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseMove(pmouseEvent->MouseIndex, pt);
            return HE_Completed;
        }
        break;

    case Event::MouseUp:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
                      PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseRelease(pmouseEvent->MouseIndex, pt, (1 << pmouseEvent->Button));
            return HE_Completed;
        }
        break;

    case Event::MouseDown:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
                      PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMousePress(pmouseEvent->MouseIndex, pt, (1 << pmouseEvent->Button));
            return HE_Completed;
        }
        break;

    case Event::MouseWheel:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(MouseEvent),
            "MouseEvent class required for mouse events");
        if (pmouseEvent->MouseIndex < MouseCursorCount)
        {
            PointF pt(PixelsToTwips(pmouseEvent->x * ViewScaleX + ViewOffsetX),
                      PixelsToTwips(pmouseEvent->y * ViewScaleY + ViewOffsetY));
            InputEventsQueue.AddMouseWheel(pmouseEvent->MouseIndex, pt, (int)pmouseEvent->ScrollDelta);
            return HE_Completed;
        }
        break;


#ifdef GFX_ENABLE_KEYBOARD
        // Keyboard.
    case Event::KeyDown:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(KeyEvent),
                "KeyEvent class required for keyboard events");
            KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
            if (keyboardState)
            {
                keyboardState->SetKeyDown(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                          pkeyEvent->Modifiers);
            }
            InputEventsQueue.AddKeyDown((short)pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                        pkeyEvent->Modifiers, pkeyEvent->KeyboardIndex);
            if (pkeyEvent->WcharCode >= 32 && pkeyEvent->WcharCode != 127)
            {
                InputEventsQueue.AddCharTyped(pkeyEvent->WcharCode, pkeyEvent->KeyboardIndex);
            }
            return HE_Completed;
        }

    case Event::KeyUp:
        {
            SF_DEBUG_WARNING(event.EventClassSize != sizeof(KeyEvent),
                "KeyEvent class required for keyboard events");
            KeyboardState* keyboardState = GetKeyboardState(pkeyEvent->KeyboardIndex);
            if (keyboardState)
            {
                keyboardState->SetKeyUp(pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                                        pkeyEvent->Modifiers);
            }
            InputEventsQueue.AddKeyUp((short)pkeyEvent->KeyCode, pkeyEvent->AsciiCode, 
                pkeyEvent->Modifiers, pkeyEvent->KeyboardIndex);
            return HE_Completed;
        }

    case Event::Char:
        SF_DEBUG_WARNING(event.EventClassSize != sizeof(CharEvent),
            "CharEvent class required for keyboard events");
        SF_DEBUG_WARNING(!GetKeyboardState(pcharEvent->KeyboardIndex), 
            "GFxCharEvent contains wrong index of keyboard");
        InputEventsQueue.AddCharTyped(pcharEvent->WcharCode, pcharEvent->KeyboardIndex);
        return HE_Completed;
#else
    case Event::KeyDown:
    case Event::KeyUp:
    case Event::CharEvent:
        SF_DEBUG_WARNING(1, "Keyboard support is disabled due to SF_NO_KEYBOARD_SUPPORT macro.");
        break;
#endif //SF_NO_KEYBOARD_SUPPORT

#ifndef SF_NO_IME_SUPPORT
    case Event::IME:
        {
            // IME events should be handled immediately
            Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
            if (pIMEManager)
            {
                return pIMEManager->HandleIMEEvent(this, *pimeEvent);
            }
#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
            else
            {
                return HandleKoreanIME(*pimeEvent);
            }
#endif // SF_NO_BUILTIN_KOREAN_IME
        }
        break;
#endif //#ifdef SF_NO_IME_SUPPORT

    case Event::KillFocus:
        OnMovieFocus(false);
        break;

    default:
        break;
    }
    return HE_NotHandled;
}

// The host app uses this to tell the GFxMovieSub where the
// user's mouse pointer is.
// Note, this function is deprecated, use HandleEvent instead.
void    MovieImpl::NotifyMouseState(float x, float y, unsigned buttons, unsigned mouseIndex)
{
    float mx  = (x * ViewScaleX + ViewOffsetX);
    float my  = (y * ViewScaleY + ViewOffsetY);
    PointF     pt(PixelsToTwips(mx), PixelsToTwips(my));

    if (mouseIndex < MouseCursorCount)
    {
        //mMouseState[mouseIndex].ResetQueue();
        //PointF p = mMouseState[mouseIndex].GetLastPosition();
        //if (pt != p)
        InputEventsQueue.AddMouseMove(mouseIndex, pt);
        unsigned lastButtons = mMouseState[mouseIndex].GetButtonsState();
        for (unsigned i = 0, mask = 1; i < MouseState::MouseButton_MaxNum; i++, mask <<= 1)
        {
            if ((buttons & mask) && !(lastButtons & mask))
                InputEventsQueue.AddMousePress(mouseIndex, pt, (buttons & mask));
            else if ((lastButtons & mask) && !(buttons & mask))
                InputEventsQueue.AddMouseRelease(mouseIndex, pt, (lastButtons & mask));
        }
    }
}

void MovieImpl::GetMouseState(unsigned mouseIndex, float* x, float* y, unsigned* buttons)
{
    SF_ASSERT(mouseIndex < GFX_MAX_MICE_SUPPORTED);
    if (mouseIndex < MouseCursorCount)
    {
        PointF p = mMouseState[mouseIndex].GetLastPosition();
        // recalculate coords back to window
        p.x = (TwipsToPixels(p.x) - ViewOffsetX) / ViewScaleX;
        p.y = (TwipsToPixels(p.y) - ViewOffsetY) / ViewScaleY;

        if (x)
            *x = p.x;
        if (y)
            *y = p.y;
        if (buttons)
            *buttons = mMouseState[mouseIndex].GetButtonsState();
    }
}

bool    MovieImpl::HitTest(float x, float y, HitTestType testCond, unsigned controllerIdx)
{
    SF_AMP_SCOPE_TIMER(AdvanceStats, "MovieImpl::HitTest");

    float mx  = (x * ViewScaleX + ViewOffsetX);
    float my  = (y * ViewScaleY + ViewOffsetY);

    unsigned movieIndex;
    for (movieIndex = (unsigned)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
    {
        InteractiveObject*  pmovie  = MovieLevels[movieIndex-1].pSprite;

        RectF      movieLocalBounds = pmovie->GetBounds(Matrix2F());

        PointF     pt(PixelsToTwips(mx), PixelsToTwips(my));

        PointF     ptMv;
        pmovie->TransformPointToLocal(ptMv, pt);

        if (movieLocalBounds.Contains(ptMv) || pmovie->Is3D() )
        {
            switch(testCond)
            {
            case HitTest_Bounds:
                if (pmovie->PointTestLocal(ptMv, 0))
                    return 1;
                break;
            case HitTest_Shapes:
                if (pmovie->PointTestLocal(ptMv, DisplayObjectBase::HitTest_TestShape))
                    return 1;
                break;
            case HitTest_ButtonEvents:
                {
                    DisplayObjectBase::TopMostDescr descr;
                    descr.ControllerIdx = controllerIdx;
                    if (pmovie->GetTopMostMouseEntity(ptMv, &descr) == DisplayObjectBase::TopMost_Found)
                        return 1;
                    break;
                }
            case HitTest_ShapesNoInvisible:
                if (pmovie->PointTestLocal(ptMv, 
                    DisplayObjectBase::HitTest_TestShape | DisplayObjectBase::HitTest_IgnoreInvisible))
                    return 1;
                break;
            }
        }
    }
    return 0;
}

int    MovieImpl::AddIntervalTimer(ASIntervalTimerIntf *timer)
{
    timer->SetId(++LastIntervalTimerId);
    IntervalTimers.PushBack(timer);
    return LastIntervalTimerId;
}

void    MovieImpl::ClearIntervalTimer(int timerId)
{
    for (UPInt i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
    {
        if (IntervalTimers[i] && IntervalTimers[i]->GetId() == timerId)
        {
            // do not remove the timer's array's entry here, just null it;
            // it will be removed later in Advance.
            IntervalTimers[i]->Clear();
            return;
        }
    }
}

void    MovieImpl::ShutdownTimers()
{
    for (UPInt i = 0, n = IntervalTimers.GetSize(); i < n; ++i)
    {
        IntervalTimers[i]->Clear();
    }
    IntervalTimers.Clear();
}

#ifdef GFX_ENABLE_VIDEO
void MovieImpl::AddVideoProvider(VideoProvider* pvideo)
{
    if (pvideo)
        VideoProviders.Add(pvideo);
}
void MovieImpl::RemoveVideoProvider(VideoProvider* pvideo)
{
    VideoProviders.Remove(pvideo);
}
#endif

// *** MovieImpl's Movie interface implementation

// These methods mostly delegate to _level0 movie.

MovieDef*    MovieImpl::GetMovieDef() const
{   
    return pMainMovieDef;
}
unsigned    MovieImpl::GetCurrentFrame() const
{
    return pMainMovie ? pMainMovie->GetCurrentFrame() : 0;
}
bool    MovieImpl::HasLooped() const
{
    return pMainMovie ? pMainMovie->HasLooped() : 0; 
}

// Destroys a level and its resources.
bool    MovieImpl::ReleaseLevelMovie(int level)
{
    if (level == 0)
    {
        StopDrag();

        ShutdownTimers();

        // Not sure if this unload order is OK
        while (MovieLevels.GetSize())
        {
            InteractiveObject* plevel = MovieLevels[MovieLevels.GetSize() - 1].pSprite;
            plevel->OnEventUnload();
            pASMovieRoot->DoActions();
            plevel->ForceShutdown();
            MovieLevels.RemoveAt(MovieLevels.GetSize() - 1);
        }

        // Clear vars.
        pMainMovie = NULL;   
        FrameTime    = 1.0f / 12.0f;        
        // Keep GetMovieDef() till next load so that users don't get null from GetMovieDef().
        //  SetMovieDef(NULL);
        //  pStateBag->SetDelegate(0);

        G_SetFlag<Flag_LevelClipsChanged>(Flags, true);
        return 1;
    }

    // Exhaustive for now; could do binary.
    for (unsigned i = 0; i < MovieLevels.GetSize(); i++)
    {
        if (MovieLevels[i].Level == level)
        {
            Ptr<InteractiveObject> plevel = MovieLevels[i].pSprite;

            // Inform old character of unloading.
            plevel->OnEventUnload();
            pASMovieRoot->DoActions();            

            // TBD: Is this right, or should local frames persist till level0 unload?
            plevel->ForceShutdown(); 

            // Remove us.
            MovieLevels.RemoveAt(i);

            G_SetFlag<Flag_LevelClipsChanged>(Flags, true);
            return 1;
        }
    }   
    return 0;
}

void    MovieImpl::Restart()                         
{       
//    pASMovieRoot->Restart();
    if (pMainMovie)
    {
        ProcessUnloadQueue();

        Ptr<MovieDefImpl> prootMovieDef = pMainMovie->GetResourceMovieDef();

        // It is important to destroy the sprite before the global context,
        // so that is is not used from OnEvent(unload) in sprite destructor
        // NOTE: We clear the list here first because users can store pointers in _global,
        // which would cause pMovie assignment to not release it early (avoid "aeon.swf" crash).
        int i;
        for (i = (int)MovieLevels.GetSize() - 1; i >= 0; i--)
            ReleaseLevelMovie(i);
        // Release all refs.
        MovieLevels.Clear();

        // clean up threaded load queue before freeing
        LoadQueueEntryMT* plq = pLoadQueueMTHead;
        unsigned plqCount = 0;
        while (plq)
        {
            plqCount++;
            plq->Cancel();
            plq = plq->pNext;
        }
        // wait until all threaded loaders exit
        // this is required to avoid losing the movieroot
        // heap before the load tasks complete.
        unsigned plqDoneCount = 0;
        while (plqCount > plqDoneCount)
        {
            plqDoneCount = 0;
            plq = pLoadQueueMTHead;
            while (plq)
            {
                if (plq->LoadFinished())
                    plqDoneCount++;
                plq = plq->pNext;
            }
        }

        // free load queue
        while(pLoadQueueHead)
        {
            // Remove from queue.
            LoadQueueEntry *pentry = pLoadQueueHead;
            pLoadQueueHead = pentry->pNext;
            delete pentry;
        }
        while(pLoadQueueMTHead)
        {
            LoadQueueEntryMT *pentry = pLoadQueueMTHead;
            pLoadQueueMTHead = pentry->pNext;
            delete pentry;
        }
        pLoadQueueHead      = 0;
        pLoadQueueMTHead    = 0;

        InvalidateOptAdvanceList();
        pPlayListHead = pPlayListOptHead = NULL;

#ifndef SF_NO_IME_SUPPORT
        // clean up IME manager
        bool wasIMEActive = false;

        Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
        if (pIMEManager)
        {
            if (pIMEManager->IsMovieActive(this))
            {
                wasIMEActive = true;
                pIMEManager->SetActiveMovie(NULL);
            }
        }

        // delete params of candidate list so they could be recreated
        delete pIMECandidateListStyle;
        pIMECandidateListStyle = NULL;

#endif //#ifdef SF_NO_IME_SUPPORT

#ifdef GFX_ENABLE_VIDEO
        VideoProviders.Clear();
#endif
#ifdef GFX_ENABLE_SOUND
        InteractiveObject* psprite = GetMainMovie();
        if (psprite)
            psprite->CharToSprite()->StopActiveSounds();
#endif
        //ReleaseUnloadList();
        pASMovieRoot->Shutdown();

        delete pRetValHolder;pRetValHolder = NULL;

        // Reset focus states
        ResetFocusStates();

        G_Set3WayFlag<Shift_DisableFocusAutoRelease>(Flags, 0);
        G_Set3WayFlag<Shift_AlwaysEnableFocusArrowKeys>(Flags, 0);
        G_Set3WayFlag<Shift_AlwaysEnableKeyboardPress>(Flags, 0);
        G_Set3WayFlag<Shift_DisableFocusRolloverEvent>(Flags, 0);
        G_Set3WayFlag<Shift_DisableFocusKeys>(Flags, 0);
        ResetMouseState();

        // Recreate global context

        // this collect should collect everything from old global context; do not
        // remove!
        pASMovieRoot->ForceCollect();
        pRetValHolder = SF_HEAP_NEW(pHeap) ReturnValueHolder(&StringManager);

        pASMovieRoot->Init(prootMovieDef);

        if (!pMainMovie)
        {
            return;
        }

        // reset mouse state and cursor
        if (pUserEventHandler)
        {
            for (unsigned i = 0; i < MouseCursorCount; ++i)
            {
                pUserEventHandler->HandleEvent(this, MouseCursorEvent(Event::DoShowMouse, i));
                pUserEventHandler->HandleEvent(this, MouseCursorEvent(MouseCursorEvent::ARROW, i));
            }
        }

        // reset keyboard state
        ResetKeyboardState();

#ifndef SF_NO_IME_SUPPORT
        if (wasIMEActive)
            pIMEManager->SetActiveMovie(this);
#endif //#ifdef SF_NO_IME_SUPPORT

        Advance(0.0f, 0);
        SetDirtyFlag();

        pASMovieRoot->ForceCollect();
    }
}

void    MovieImpl::ResetMouseState()
{
    for (unsigned i = 0; i < sizeof(mMouseState)/sizeof(mMouseState[0]); ++i)
    {
        mMouseState[i].ResetState();
    }
}

void    MovieImpl::ResetKeyboardState()
{
#ifdef GFX_ENABLE_KEYBOARD
    for (unsigned i = 0; i < sizeof(KeyboardStates)/sizeof(KeyboardStates[0]); ++i)
    {
        KeyboardStates[i].ResetState();
    }
#endif
}

#ifdef GFX_ENABLE_KEYBOARD
void    MovieImpl::SetKeyboardListener(KeyboardState::IListener* l)
{
    for (unsigned i = 0; i < sizeof(KeyboardStates)/sizeof(KeyboardStates[0]); ++i)
    {
        KeyboardStates[i].SetListener(l);
    }
}
#endif

void    MovieImpl::GotoFrame(unsigned targetFrameNumber) 
{   // 0-based!!
    if (pMainMovie) pMainMovie->GotoFrame(targetFrameNumber); 
}

bool    MovieImpl::GotoLabeledFrame(const char* label, int offset)
{
    if (!pMainMovie)
        return 0;

    unsigned    targetFrame = SF_MAX_UINT;
    if (pMainMovieDef->GetDataDef()->GetLabeledFrame(label, &targetFrame, 0))
    {
        GotoFrame((unsigned)((int)targetFrame + offset));
        return 1;
    }
    else
    {
        Ptr<LogState> log = GetLogState();
        if (log)
            log->LogScriptError("MovieImpl::GotoLabeledFrame('%s') unknown label", label);
        return 0;
    }
}

#if 0
void    MovieImpl::Display()
{
    // If there is no _level0, there are no other levels either.
    if (!pMainMovie)
        return;
    
#if defined(SF_ENABLE_STATS)
    AmpFunctionTimer displayTimer(DisplayStats, AMP::NativeCodeSwdHandle, Amp_Native_Function_Id_Display);
#endif

    // Warn in case the user called Display() before Advance().
    SF_DEBUG_WARNING(!G_IsFlagSet<Flag_AdvanceCalled>(Flags), "Movie::Display called before Advance - no rendering will take place");

    unsigned    movieIndex;
    bool    haveVisibleMovie = 0;

    for (movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++)
    {
        if (MovieLevels[movieIndex].pSprite->GetVisible())
        {
            haveVisibleMovie = 1;
            break;
        }
    }

    // Avoid rendering calls if no movies are visible.
    if (!haveVisibleMovie)
        return;
    // DBG
    //printf("MovieImpl::Display %d   -----------------------\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());

    bool ampProfiling = false;
#ifdef SF_AMP_SERVER
    ampProfiling = AMP::Server::GetInstance().IsProfiling();
#endif

    DisplayContext context(pStateBag, 
                              pMainMovieDef->GetWeakLib(),
                              &pMainMovieDef->GetResourceBinding(),
                              GetPixelScale(),
                              ViewportMatrix, ampProfiling ? DisplayStats : NULL);

    //Matrix2D matrix = Render::Matrix2D::Identity;
    //context.pParentMatrix = &matrix;

    // Need to check RenderConfig before GetRenderer.
//@REN 
//     if (!context.GetRenderConfig() || !context.GetRenderer())
//     {
//         SF_DEBUG_WARNING(1, "Movie::Display failed, no renderer specified");
//         return;
//     }
// 
//     // Cache log to avoid expensive state access.
//     G_SetFlag<Flag_CachedLogFlag>(Flags, true);
//     pCachedLog    = context.pLog;
// 
//     context.GetRenderer()->BeginDisplay(   
//         BackgroundColor,
//         mViewport,
//         VisibleFrameRect.x1, VisibleFrameRect.x2,
//         VisibleFrameRect.y1, VisibleFrameRect.y2);
// 
// #ifdef GFX_ENABLE_VIDEO
//     if (VideoProviders.GetSize() > 0)
//     {
//         for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
//         {
//             Ptr<VideoProvider> pvideo = *it;
//             // we have to call Progress method here and not from VideoObject AdvanceFrame method 
//             // because there could be videos that are not attached to any VideoObject but still they 
//             // have to progress (only sound in this case is produced)
//             pvideo->Display(context.GetRenderer());
//         }
//     }
// #endif
// 
//     // Display all levels. _level0 is at the bottom.
//     for (movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++)
//     {
//         InteractiveObject* pmovie = MovieLevels[movieIndex].pSprite;
//         pmovie->Display(context);
//     }
//     DisplayTopmostLevelCharacters(context);
// 
//     // Display focus rect
//     DisplayFocusRect(context);
// 
//     context.GetRenderer()->EndDisplay();
// 
//     G_SetFlag<Flag_CachedLogFlag>(Flags, false);
// 
//     context.pMeshCacheManager->ClearKillList();
//     context.pMeshCacheManager->AcquireFrameQueue();
// 
//     if (context.pMeshCacheManager->GetRenderGenMemLimit())
//     {
//         // Release RenderGen memory if necessary.
//         RenderGen* rg = context.pMeshCacheManager->GetRenderGen();
//         if (rg->GetNumBytes() > context.pMeshCacheManager->GetRenderGenMemLimit())
//         {
//             rg->ClearAndRelease();
//         }
//     }
//     // DBG
//     //printf("MovieImpl::Display %d   ^^^^^^^^^^^^^^^^^^^^^\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());
// 
// #ifdef SF_BUILD_LOGO
//     GFx_DisplayLogo(context.GetRenderer(), mViewport, pDummyImage);
// #endif
    if (!context.GetRenderConfig() || !context.GetRenderer())
    {
        SF_DEBUG_WARNING(1, "Movie::Display failed, no renderer specified");
        return;
    }

    // Cache log to avoid expensive state access.
    G_SetFlag<Flag_CachedLogFlag>(Flags, true);
    pCachedLog    = context.pLog;

    context.GetRenderer()->BeginDisplay(   
        BackgroundColor,
        Viewport,
        VisibleFrameRect.x1, VisibleFrameRect.x2,
        VisibleFrameRect.y1, VisibleFrameRect.y2);

#ifdef GFX_ENABLE_VIDEO
    if (VideoProviders.GetSize() > 0)
    {
        for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
        {
            Ptr<VideoProvider> pvideo = *it;
            // we have to call Progress method here and not from VideoObject AdvanceFrame method 
            // because there could be videos that are not attached to any VideoObject but still they 
            // have to progress (only sound in this case is produced)
            pvideo->Display(context.GetRenderer());
        }
    }
#endif

    // Display all levels. _level0 is at the bottom.
    for (movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++)
    {
        InteractiveObject* pmovie = MovieLevels[movieIndex].pSprite;
        pmovie->Display(context);
    }
    DisplayTopmostLevelCharacters(context);

    // Display focus rect
    DisplayFocusRect(context);

    context.GetRenderer()->EndDisplay();

    G_SetFlag<Flag_CachedLogFlag>(Flags, false);

    context.pMeshCacheManager->ClearKillList();
    context.pMeshCacheManager->AcquireFrameQueue();

    if (context.pMeshCacheManager->GetRenderGenMemLimit())
    {
        // Release RenderGen memory if necessary.
        RenderGen* rg = context.pMeshCacheManager->GetRenderGen();
        if (rg->GetNumBytes() > context.pMeshCacheManager->GetRenderGenMemLimit())
        {
            rg->ClearAndRelease();
        }
    }
    // DBG
    //printf("MovieImpl::Display %d   ^^^^^^^^^^^^^^^^^^^^^\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());

#ifdef SF_BUILD_LOGO
    GFx_DisplayLogo(context.GetRenderer(), Viewport, pDummyImage);
#endif
//@REN 
//     if (!context.GetRenderConfig() || !context.GetRenderer())
//     {
//         SF_DEBUG_WARNING(1, "Movie::Display failed, no renderer specified");
//         return;
//     }
// 
//     // Cache log to avoid expensive state access.
//     G_SetFlag<Flag_CachedLogFlag>(Flags, true);
//     pCachedLog = context.pLog;
// 
//     context.GetRenderer()->BeginDisplay(   
//         BackgroundColor,
//         mViewport,
//         VisibleFrameRect.x1, VisibleFrameRect.x2,
//         VisibleFrameRect.y1, VisibleFrameRect.y2);
// 
// #ifdef GFX_ENABLE_VIDEO
//     if (VideoProviders.GetSize() > 0)
//     {
//         for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
//         {
//             Ptr<VideoProvider> pvideo = *it;
//             // we have to call Progress method here and not from VideoObject AdvanceFrame method 
//             // because there could be videos that are not attached to any VideoObject but still they 
//             // have to progress (only sound in this case is produced)
//             pvideo->Display(context.GetRenderer());
//         }
//     }
// #endif
// 
//     // Display all levels. _level0 is at the bottom.
//     for (movieIndex = 0; movieIndex < MovieLevels.GetSize(); movieIndex++)
//     {
//         InteractiveObject* pmovie = MovieLevels[movieIndex].pSprite;
//         pmovie->Display(context);
//     }
//     DisplayTopmostLevelCharacters(context);
// 
//     // Display focus rect
//     DisplayFocusRect(context);
// 
//     context.GetRenderer()->EndDisplay();
// 
//     G_SetFlag<Flag_CachedLogFlag>(Flags, false);
// 
//     context.pMeshCacheManager->ClearKillList();
//     context.pMeshCacheManager->AcquireFrameQueue();
// 
//     if (context.pMeshCacheManager->GetRenderGenMemLimit())
//     {
//         // Release RenderGen memory if necessary.
//         RenderGen* rg = context.pMeshCacheManager->GetRenderGen();
//         if (rg->GetNumBytes() > context.pMeshCacheManager->GetRenderGenMemLimit())
//         {
//             rg->ClearAndRelease();
//         }
//     }
//     // DBG
//     //printf("MovieImpl::Display %d   ^^^^^^^^^^^^^^^^^^^^^\n", pMainMovie->GetLevelMovie(0)->ToSprite()->GetCurrentFrame());
// 
// #ifdef SF_BUILD_LOGO
//     GFx_DisplayLogo(context.GetRenderer(), mViewport, pDummyImage);
// #endif
}
#endif

void    MovieImpl::SetPause(bool pause)
{
    if ((G_IsFlagSet<Flag_Paused>(Flags) && pause) || (!G_IsFlagSet<Flag_Paused>(Flags) && !pause))
        return;
    G_SetFlag<Flag_Paused>(Flags, pause);
    if (pause)
        PauseTickMs = Timer::GetTicks()/1000;
    else
        StartTickMs += (Timer::GetTicks()/1000 - PauseTickMs); 

    InteractiveObject* pnextCh = NULL;
    for (InteractiveObject* pcurCh = pPlayListHead; pcurCh; pcurCh = pnextCh)
    {
        pnextCh = pcurCh->pPlayNext;
        pcurCh->SetPause(pause);
    }
#ifdef GFX_ENABLE_VIDEO
    if (VideoProviders.GetSize() > 0)
    {
        Array<VideoProvider*> remove_list;
        for(HashSet<Ptr<VideoProvider> >::Iterator it = VideoProviders.Begin(); it != VideoProviders.End(); ++it)
        {
            Ptr<VideoProvider> pvideo = *it;
            pvideo->Pause(pause);
        }
    }
#endif
}

void    MovieImpl::SetPlayState(PlayState s)
{   
    if (pMainMovie)
        pMainMovie->SetPlayState(s); 
}
PlayState MovieImpl::GetPlayState() const
{   
    if (!pMainMovie) return GFx::State_Stopped;
    return pMainMovie->GetPlayState(); 
}

void    MovieImpl::SetVisible(bool visible)
{
    if (pMainMovie) pMainMovie->SetVisible(visible);
}
bool    MovieImpl::GetVisible() const
{
    return pMainMovie ? pMainMovie->GetVisible() : 0;
}

void MovieImpl::AddStickyVariableNode(const ASString& path, StickyVarNode* p)
{
    SF_ASSERT(p);
    MovieImpl::StickyVarNode* pnode = 0;

    if (StickyVariables.Get(path, &pnode) && pnode)
    {
        MovieImpl::StickyVarNode* pcur  = pnode;
        MovieImpl::StickyVarNode* pprev = NULL;

        while (pcur)
        {
            // If there is a name match, replace the node.
            if (pcur->Name == p->Name)
            {
                p->pNext = pcur->pNext;
                if (pprev)
                    pprev->pNext = p;
//                 p->Value     = val;
//                 if (!p->Permanent) // Permanent sticks.
//                     p->Permanent = (setType == Movie::SV_Permanent);
                return;
            }
            pprev = pcur;
            pcur = pcur->pNext;
        };

        // Not found, add new node
        // Link prev node to us: order does not matter.
        p->pNext = pnode->pNext;
        pnode->pNext = p;
        
        // Done.
        return;
    }

    // Save node.
    StickyVariables.Set(path, p);
}
// 
// void    MovieImpl::ResolveStickyVariables(InteractiveObject *pcharacter)
// {
//     pASMovieRoot->ResolveStickyVariables(pcharacter);
// }

void    MovieImpl::ClearStickyVariables()
{
    ASStringHash<StickyVarNode*>::Iterator ihash = StickyVariables.Begin();

    // Travers hash and delete nodes.
    for( ;ihash != StickyVariables.End(); ++ihash)
    {
        StickyVarNode* pnode = ihash->Second;
        StickyVarNode* p;

        while (pnode)
        {
            p = pnode;
            pnode = p->pNext;
            delete p;
        }
    }

    StickyVariables.Clear();
}

void        MovieImpl::OnMovieFocus(bool set)
{
#ifdef GFX_ENABLE_KEYBOARD
    if (!set)
    {
        ResetKeyboardState();
        ResetMouseState();
    }
#endif //#ifdef GFX_ENABLE_KEYBOARD
    G_SetFlag<Flag_MovieIsFocused>(Flags, set);
#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        pIMEManager->SetActiveMovie(this);
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    pASMovieRoot->OnMovieFocus(set);
}

void MovieImpl::ResetTabableArrays()
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        FocusGroups[i].ResetTabableArray();
    }
}

void MovieImpl::ResetFocusStates()
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        FocusGroups[i].ModalClip.Clear();
        FocusGroups[i].ResetTabableArray();
        FocusGroups[i].LastFocusKeyCode = 0;
    }
}

bool MovieImpl::SetControllerFocusGroup(unsigned controllerIdx, unsigned focusGroupIndex)
{
    if (controllerIdx >= GFX_MAX_CONTROLLERS_SUPPORTED || 
        focusGroupIndex >= GFX_MAX_CONTROLLERS_SUPPORTED)
        return false;
    FocusGroupIndexes[controllerIdx] = (UInt8)focusGroupIndex;
    if (focusGroupIndex >= FocusGroupsCnt)
        FocusGroupsCnt = focusGroupIndex + 1;
    return true;
}

unsigned MovieImpl::GetControllerFocusGroup(unsigned controllerIdx) const
{
    if (controllerIdx >= GFX_MAX_CONTROLLERS_SUPPORTED)
        return false;
    return FocusGroupIndexes[controllerIdx];
}

void MovieImpl::FillTabableArray(const ProcessFocusKeyInfo* pfocusInfo)
{
    SF_ASSERT(pfocusInfo);

    FocusGroupDescr& focusGroup = *pfocusInfo->pFocusGroup;
    if (pfocusInfo->InclFocusEnabled && 
        (focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized) && 
        !(focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_WithFocusEnabled))
        focusGroup.ResetTabableArray(); // need to refill array with focusEnabled chars
    if (!(focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized))
    {
        Sprite* modalClip       = NULL;
        //bool tabIndexed         = false;
        InteractiveObject::FillTabableParams p;
        p.Array = &focusGroup.TabableArray;
        p.InclFocusEnabled = pfocusInfo->InclFocusEnabled;
        if ((modalClip = focusGroup.GetModalClip(this)) == NULL)
        {
            // fill array by focusable characters
            for (int movieIndex = (int)MovieLevels.GetSize(); movieIndex > 0; movieIndex--)
            {
                if (MovieLevels[movieIndex-1].pSprite->IsDisplayObjContainer())
                MovieLevels[movieIndex-1].pSprite->CharToDisplayObjContainer_Unsafe()->FillTabableArray(&p);
            }
        }
        else
        {
            // fill array by focusable characters but only by children of ModalClip
            modalClip->FillTabableArray(&p);
        }

        if (p.TabIndexed)
        {
            // sort by tabIndex if tabIndexed == true
            static TabIndexSortFunctor sf;
            Alg::QuickSort(focusGroup.TabableArray, sf);
        }
        else
        {
            // sort for automatic order
            static AutoTabSortFunctor sf;
            Alg::QuickSort(focusGroup.TabableArray, sf);
        }
        focusGroup.TabableArrayStatus = FocusGroupDescr::TabableArray_Initialized;
        if (pfocusInfo->InclFocusEnabled)
            focusGroup.TabableArrayStatus |= FocusGroupDescr::TabableArray_WithFocusEnabled;
    }
}

// Focus management
void MovieImpl::InitFocusKeyInfo(ProcessFocusKeyInfo* pfocusInfo, 
                                 const InputEventsQueue::QueueEntry::KeyEntry& keyEntry,
                                 bool inclFocusEnabled,
                                 FocusGroupDescr* pfocusGroup)
{
    if (!pfocusInfo->Initialized)
    {
        FocusGroupDescr& focusGroup = (!pfocusGroup) ? GetFocusGroup(keyEntry.KeyboardIndex) : *pfocusGroup;
        pfocusInfo->pFocusGroup     = &focusGroup;
        pfocusInfo->PrevKeyCode     = focusGroup.LastFocusKeyCode;
        pfocusInfo->Prev_aRect      = focusGroup.LastFocusedRect;
        pfocusInfo->InclFocusEnabled= inclFocusEnabled;
        pfocusInfo->ManualFocus     = false;
        pfocusInfo->KeyboardIndex   = keyEntry.KeyboardIndex;
        pfocusInfo->KeyCode         = keyEntry.Code;
        pfocusInfo->KeysState       = keyEntry.KeysState;

        FillTabableArray(pfocusInfo);

        //@DBG Debug code, do not remove!
        //for(int ii = 0; ii < tabableArraySize; ++ii)
        //{
        //printf("Focus[%d] = %s\n", ii, pfocusInfo->TabableArray[ii]->GetCharacterHandle()->GetNamePath().ToCStr());
        //}

        pfocusInfo->CurFocusIdx = -1;
        pfocusInfo->CurFocused  = focusGroup.LastFocused;
        if (pfocusInfo->CurFocused)
        {
            // find the index of currently focused
            for (UPInt i = 0; i < focusGroup.TabableArray.GetSize(); ++i)
            {
                if (focusGroup.TabableArray[i] == pfocusInfo->CurFocused)
                {
                    pfocusInfo->CurFocusIdx = (int)i;
                    break;
                }
            }
        }
        pfocusInfo->Initialized = true;
    }
}

void MovieImpl::ProcessFocusKey(Event::EventType event, 
                                const InputEventsQueue::QueueEntry::KeyEntry& keyEntry, 
                                ProcessFocusKeyInfo* pfocusInfo)
{
    if (event == Event::KeyDown)
    {
        if (keyEntry.Code == Key::Tab || 
            ((IsFocusRectShown(keyEntry.KeyboardIndex) || pfocusInfo->ManualFocus) && 
            (keyEntry.Code == Key::Left || keyEntry.Code == Key::Right || 
             keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)))
        {
            // init focus info if it is not initialized yet
            InitFocusKeyInfo(pfocusInfo, keyEntry, false);
            const FocusGroupDescr& focusGroup = *pfocusInfo->pFocusGroup;
            int tabableArraySize = (int)focusGroup.TabableArray.GetSize();
            if (keyEntry.Code == Key::Tab)
            {
                int cnt;
                int curFocusIdx = pfocusInfo->CurFocusIdx;
                pfocusInfo->CurFocusIdx = -1;
                for (cnt = 0; cnt < tabableArraySize; ++cnt)
                {
                    if (KeyModifiers(keyEntry.KeysState).IsShiftPressed())
                    {
                        if (--curFocusIdx < 0)  
                            curFocusIdx = tabableArraySize-1;
                    }
                    else
                    {
                        if (++curFocusIdx >= tabableArraySize)
                            curFocusIdx = 0;
                    }
                    // check if candidate is tabable, or if manual focus mode is on. Break, if so.
                    if (focusGroup.TabableArray[curFocusIdx] && 
                        (pfocusInfo->InclFocusEnabled || focusGroup.TabableArray[curFocusIdx]->IsTabable()) &&
                        focusGroup.TabableArray[curFocusIdx]->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                    {
                        pfocusInfo->CurFocusIdx = curFocusIdx;
                        break;
                    }
                } 
                SetDirtyFlag();
            }
            else if (pfocusInfo->CurFocused)
            {
                if (pfocusInfo->CurFocused->IsFocusRectEnabled() || IsAlwaysEnableFocusArrowKeys() || pfocusInfo->ManualFocus)
                {
                    DisplayObjectBase::Matrix ma = pfocusInfo->CurFocused->GetLevelMatrix();
                    // aRect represents the rectangle of currently focused character.
                    RectF aRect  = ma.EncloseTransform(pfocusInfo->CurFocused->GetFocusRect());

                    // We need to adjust the existing source rectangle (aRect). If we have
                    // saved previous rectangle and the direction if focus moving is the same
                    // (for example, we pressed the up/down arrow key again), then we will use
                    // x1/x2 coordinates from the saved rectangle to maintain the same Y-axis.
                    // Same will happen if we move right/left, we will use original y1/y2
                    // to keep X-axis.
                    if (pfocusInfo->PrevKeyCode == keyEntry.Code)
                    {
                        if (keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)
                        {
                            aRect.x1      = pfocusInfo->Prev_aRect.x1;
                            aRect.x2     = pfocusInfo->Prev_aRect.x2;
                        }
                        else if (keyEntry.Code == Key::Right || keyEntry.Code == Key::Left)
                        {
                            aRect.y1       = pfocusInfo->Prev_aRect.y1;
                            aRect.y2    = pfocusInfo->Prev_aRect.y2;
                        }
                    }
                    else
                    {
                        pfocusInfo->Prev_aRect  = aRect;
                        pfocusInfo->PrevKeyCode = keyEntry.Code;
                    }

                    if (keyEntry.Code == Key::Right || keyEntry.Code == Key::Left)
                    {
                        SetDirtyFlag();
                        int newFocusIdx = pfocusInfo->CurFocusIdx;
                        RectF newFocusRect;
                        newFocusRect.x1 = newFocusRect.x2 = (keyEntry.Code == Key::Right) ? float(INT_MAX) : float(INT_MIN);
                        newFocusRect.y1 = newFocusRect.y2 = float(INT_MAX);
                        // find nearest from right or left side

                        bool hitStipe = false;
                        for (int i = 0; i < tabableArraySize - 1; ++i)
                        {
                            newFocusIdx = (keyEntry.Code == Key::Right) ? newFocusIdx + 1 : newFocusIdx - 1;
                            if (newFocusIdx >= tabableArraySize)
                                newFocusIdx = 0;
                            else if (newFocusIdx < 0)
                                newFocusIdx = tabableArraySize - 1;
                            Ptr<InteractiveObject> b = focusGroup.TabableArray[newFocusIdx];
                            if ((!pfocusInfo->InclFocusEnabled && !b->IsTabable()) ||
                                !b->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                            {
                                // If this is not for manual focus and not tabable - ignore.
                                continue;
                            }
                            DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
                            RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

                            bool  curHitStripe    = false;
                            // check the "stripe zone"

                            RectF stripeRect;
                            if (keyEntry.Code == Key::Right)
                                stripeRect = RectF(aRect.x2+1, aRect.y1, SF_MAX_FLOAT, aRect.y2);
                            else  // x1
                                stripeRect = RectF(SF_MIN_FLOAT, aRect.y1, aRect.x1-1, aRect.y2);
                            // Check, if the current character ("b") is in the stripe zone or not.
                            if (bRect.Intersects(stripeRect))
                            {
                                RectF intersectionRect = bRect;
                                intersectionRect.Intersect(stripeRect);
                                //@DBG
                                //printf("Intersection height is %f\n", intersectionRect.Height());
                                if (intersectionRect.Height() >= 40) // 2 pixels threshold
                                    curHitStripe    = true;
                            }
                            if (curHitStripe)
                            {
                                if (!hitStipe)
                                { // first hit - save the current char index and rect
                                    pfocusInfo->CurFocusIdx = newFocusIdx;
                                    newFocusRect            = bRect;
                                    hitStipe                = true;

                                    //@DBG
                                    //printf("InitFocus = %s\n",  
                                    //    b->GetCharacterHandle()->GetNamePath().ToCStr());
                                    continue;
                                }
                            }
                            // if we already hit stripe once - ignore all chars NOT in the stripe zone
                            if (hitStipe && !curHitStripe)
                                continue;

                            PointF vector1, vector2;
                            // lets calculate the distance from the adjusted rectangle of currently
                            // focused character to the character currently being iterated, and compare
                            // the distance with the distance to existing candidate.
                            // The distance being calculated is from the middle of the right side (if "right" 
                            // key is down) to the middle of the left side, or other way around.
                            if (keyEntry.Code == Key::Right)
                            {
                                PointF ptBeg(aRect.x2, aRect.y1 + aRect.Height()/2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.HClamp  (aRect.x2, bRect.x2);
                                newrect.HClamp(aRect.x2, newFocusRect.x2);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Width())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1, bRect.y1 + bRect.Height()/2);
                                PointF ptEnd2(newrect.x1, newFocusRect.y1 + newFocusRect.Height()/2);

                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.x < 0) // negative, means it is not at the right
                                    continue;
                            }
                            else // left
                            {
                                PointF ptBeg(aRect.x1, aRect.y1 + aRect.Height()/2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.HClamp  (bRect.x1, aRect.x1);
                                newrect.HClamp(newFocusRect.x1, aRect.x1);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Width())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x2, bRect.y1 + bRect.Height()/2);
                                PointF ptEnd2(newrect.x2, newFocusRect.y1 + newFocusRect.Height()/2);

                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.x > 0) // positive, means it is not at the left
                                    continue;
                            }
                            //@DBG
                            //printf("Checking for %s, vec1(%d,%d), vec2(%d,%d)\n",  
                            //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                            //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                            // Check, if the character in the "stripe-zone". If yes - check, is the new char
                            // closer by the 'x' coordinate or not. If 'x' coordinate is the same - check if it is 
                            // close by the 'y' coordinate. Update 'newFocus' item if so.
                            // If stripe is not hit, then just measure the distance to the new char and
                            // update the 'newFocus' item if it is closer than previous one.
                            if ((hitStipe && 
                                 (Alg::Abs(vector1.x) < Alg::Abs(vector2.x) || (vector1.x == vector2.x && Alg::Abs(vector1.y) < Alg::Abs(vector2.y)))) || 
                                (!hitStipe &&
                                (vector1.x*vector1.x + vector1.y*vector1.y < 
                                vector2.x*vector2.x + vector2.y*vector2.y)))
                            {
                                //@DBG
                                //printf("   newFocus = %s, vec1(%d,%d), vec2(%d,%d)\n",  
                                //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                                //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                                pfocusInfo->CurFocusIdx = newFocusIdx;
                                newFocusRect            = bRect;
                            }
                        }
                    }
                    else if (keyEntry.Code == Key::Up || keyEntry.Code == Key::Down)
                    {
                        SetDirtyFlag();
                        int newFocusIdx = pfocusInfo->CurFocusIdx;
                        RectF newFocusRect(0);
                        newFocusRect.x1 = float(INT_MAX);
                        newFocusRect.y1 = (keyEntry.Code == Key::Down) ? float(INT_MAX) : float(INT_MIN);
                        // find nearest from top and bottom side
                        // The logic is as follows:
                        // 1. The highest priority characters are ones, which boundary rectangles intersect with the "stripe zone" 
                        // above or below the currently selected character. I.e. for Down key the "stripe zone" will be 
                        // ((aRect.x1, aRect.x2), (aRect.y2, Infinity)).
                        //   a) if there are more than one characters in the "stripe zone", then the best candidate should
                        //      have shortest distance by Y axis.
                        // 2. Otherwise, the closest character will be chosen by comparing Y-distance and only then X-distance.
                        bool hitStipe = false;

                        for (int i = 0; i < tabableArraySize - 1; ++i)
                        {
                            newFocusIdx = (keyEntry.Code == Key::Down) ? newFocusIdx + 1 : newFocusIdx - 1;
                            if (newFocusIdx >= tabableArraySize)
                                newFocusIdx = 0;
                            else if (newFocusIdx < 0)
                                newFocusIdx = tabableArraySize - 1;
                            Ptr<InteractiveObject> b  = focusGroup.TabableArray[newFocusIdx];
                            if ((!pfocusInfo->InclFocusEnabled && !b->IsTabable()) ||
                                !b->IsFocusAllowed(this, pfocusInfo->KeyboardIndex))
                            {
                                // If this is not for manual focus and not tabable - ignore.
                                continue;
                            }
                            DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
                            RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

                            bool  curHitStripe    = false;
                            // check the "stripe zone"
                            RectF stripeRect;
                            if (keyEntry.Code == Key::Down)
                                stripeRect = RectF(aRect.x1, aRect.y2 + 1, aRect.x2, SF_MAX_FLOAT);
                            else // Up
                                stripeRect = RectF(aRect.x1, SF_MIN_FLOAT, aRect.x2, aRect.y1 - 1);
                            //@DBG
                            //printf("bRect = %s\n",  b->GetCharacterHandle()->GetNamePath().ToCStr());
                            if (bRect.Intersects(stripeRect))
                            {
                                RectF intersectionRect = bRect;
                                intersectionRect.Intersect(stripeRect);
                                //@DBG
                                //printf("Intersection width is %f\n", intersectionRect.Width());
                                if (intersectionRect.Width() >= 40) // 2 pixels threshold
                                    curHitStripe    = true;
                            }
                            //@DBG
                            //printf ("curHitStripe is %d\n", (int)curHitStripe);
                            if (curHitStripe)
                            {
                                if (!hitStipe)
                                { // first hit - save the current char index and rect
                                    pfocusInfo->CurFocusIdx = newFocusIdx;
                                    newFocusRect = bRect;
                                    hitStipe = true;

                                    //@DBG
                                    //printf("InitFocus = %s\n",  
                                    //    b->GetCharacterHandle()->GetNamePath().ToCStr());
                                    continue;
                                }
                            }
                            // if we already hit stripe once - ignore all chars NOT in the stripe zone
                            if (hitStipe && !curHitStripe)
                                continue;

                            PointF vector1, vector2;
                            // lets calculate the distance from the adjusted rectangle of currently
                            // focused character to the character currently being iterated, and compare
                            // the distance with the distance to existing candidate.
                            // The distance being calculated is from the middle of the top side (if "up" 
                            // key is down) to the middle of the bottom side, or other way around.
                            if (keyEntry.Code == Key::Up)
                            {
                                PointF ptBeg(aRect.x1 + aRect.Width()/2, aRect.y1);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.VClamp  (bRect.y1, aRect.y1);
                                newrect.VClamp(newFocusRect.y1, aRect.y1);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Height())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1 + brect.Width()/2, brect.y2);
                                PointF ptEnd2(newrect.x1 + newrect.Width()/2, newrect.y2);
                                
                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.y > 0) // positive, means it is not at the top
                                    continue;
                            }
                            else // down
                            {
                                PointF ptBeg(aRect.x1 + aRect.Width()/2, aRect.y2);
                                RectF brect(bRect), newrect(newFocusRect);
                                brect.VClamp  (aRect.y2, bRect.y2);
                                newrect.VClamp(aRect.y2, newFocusRect.y2);
                                if (!brect.IsNormal() || Alg::IRound(TwipsToPixels(brect.Height())) <= 3) // threshold 3 pixels
                                    continue;
                                PointF ptEnd1(brect.x1 + brect.Width()/2, brect.y1);
                                PointF ptEnd2(newrect.x1 + newrect.Width()/2, newrect.y1);
                                
                                vector1.x = floorf(TwipsToPixels(ptEnd1.x - ptBeg.x)); 
                                vector1.y = floorf(TwipsToPixels(ptEnd1.y - ptBeg.y));
                                vector2.x = floorf(TwipsToPixels(ptEnd2.x - ptBeg.x)); 
                                vector2.y = floorf(TwipsToPixels(ptEnd2.y - ptBeg.y));
                                if (vector1.y < 0) // negative, means it is not at the down
                                    continue;
                            }
                            //@DBG
                            //printf("Checking for %s, vec1(%d,%d), vec2(%d,%d)\n",  
                            //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                            //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                            // Check, if the character in the "stripe-zone". If yes - check, is the new char
                            // closer by the 'y' coordinate or not. If 'x' coordinate is the same - check if it is 
                            // close by the 'x' coordinate. Update 'newFocus' item if so.
                            // If stripe is not hit, then just measure the distance to the new char and
                            // update the 'newFocus' item if it is closer than previous one.
                            if ((hitStipe && 
                                (Alg::Abs(vector1.y) < Alg::Abs(vector2.y) || (vector1.y == vector2.y && Alg::Abs(vector1.x) < Alg::Abs(vector2.x)))) || 
                                (!hitStipe &&
                                (vector1.x*vector1.x + vector1.y*vector1.y < 
                                vector2.x*vector2.x + vector2.y*vector2.y)))
                            {
                                //@DBG
                                //printf("   newFocus = %s, vec1(%d,%d), vec2(%d,%d)\n",  
                                //    b->GetCharacterHandle()->GetNamePath().ToCStr(),
                                //    (int)vector1.x, (int)vector1.y, (int)vector2.x, (int)vector2.y);
                                pfocusInfo->CurFocusIdx = newFocusIdx;
                                newFocusRect = bRect;
                            }
                        }
                    }
                }
            }
            if (pfocusInfo->CurFocusIdx >= 0 && pfocusInfo->CurFocusIdx < tabableArraySize)
            {
                pfocusInfo->CurFocused = focusGroup.TabableArray[pfocusInfo->CurFocusIdx];
            }
            else
            {
                pfocusInfo->CurFocused = NULL;
            }
        }
    }
}

void MovieImpl::FinalizeProcessFocusKey(ProcessFocusKeyInfo* pfocusInfo)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(pfocusInfo->KeyboardIndex);
    if (pfocusInfo && pfocusInfo->Initialized && 
        (focusGroup.TabableArrayStatus & FocusGroupDescr::TabableArray_Initialized))
    {
        InteractiveObject* psetFocusToCh;
        if (pfocusInfo->CurFocusIdx >= 0 && pfocusInfo->CurFocusIdx < (int)focusGroup.TabableArray.GetSize())
            psetFocusToCh = focusGroup.TabableArray[pfocusInfo->CurFocusIdx];
        else
        {
            // if CurFocusIdx is out of the TabableArray then do nothing. This may happen
            // when TabableArray is empty; in this case we do not need to transfer focus
            // anywhere.
            return; 
        }

        Ptr<InteractiveObject> lastFocused = focusGroup.LastFocused;

        if (lastFocused != psetFocusToCh)
        {
            // keep tracking direction of focus movement
            focusGroup.LastFocusKeyCode = pfocusInfo->PrevKeyCode;
            focusGroup.LastFocusedRect  = pfocusInfo->Prev_aRect;

            QueueSetFocusTo(psetFocusToCh, NULL, pfocusInfo->KeyboardIndex, 
                GFx_FocusMovedByKeyboard, pfocusInfo);
        }
        if (!psetFocusToCh || psetFocusToCh->GetType() != CharacterDef::TextField)
            focusGroup.FocusRectShown = true;
        else
            focusGroup.FocusRectShown = false;
        SetDirtyFlag();
    }
}

void MovieImpl::ActivateFocusCapture(unsigned controllerIdx)
{
    ProcessFocusKeyInfo focusKeyInfo;
    InputEventsQueue::QueueEntry::KeyEntry keyEntry;
    keyEntry.Code = Key::Tab;
    keyEntry.KeysState = 0;
    keyEntry.KeyboardIndex = (UInt8)controllerIdx;
    ProcessFocusKey(KeyEvent::KeyDown, keyEntry, &focusKeyInfo);
    FinalizeProcessFocusKey(&focusKeyInfo);
}

void MovieImpl::SetModalClip(Sprite* pmovie, unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);

    if (!pmovie)
        focusGroup.ModalClip = NULL;
    else
        focusGroup.ModalClip = pmovie->GetCharacterHandle();
}

Sprite* MovieImpl::GetModalClip(unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    return focusGroup.GetModalClip(this);
}

UInt32 MovieImpl::GetControllerMaskByFocusGroup(unsigned focusGroupIndex)
{
    UInt32 v = 0;
    for (unsigned f = 0, mask = 0x1; f < GFX_MAX_CONTROLLERS_SUPPORTED; ++f, mask <<= 1)
    {
        if (FocusGroupIndexes[f] == focusGroupIndex)
            v |= mask;
    }
    return v;
}

// @REN
/*
void MovieImpl::DisplayFocusRect(const DisplayContext& context)
{
    for (unsigned f = 0; f < FocusGroupsCnt; ++f)
    {
        Ptr<InteractiveObject> curFocused = FocusGroups[f].LastFocused;
        if (curFocused  && FocusGroups[f].FocusRectShown && curFocused->IsFocusRectEnabled())
        {
            Matrix2F   mat = curFocused->GetWorldMatrix();         
            RectF focusLocalRect = curFocused->GetFocusRect();

            if (focusLocalRect.IsNull())
                return;
            // Do viewport culling if bounds are available.
            if (!VisibleFrameRect.Intersects(mat.EncloseTransform(focusLocalRect)))
                if (!(context.GetRenderFlags() & RenderConfig::RF_NoViewCull))
                    return;

            Render::Renderer* prenderer   = context.GetRenderer();

            RectF focusWorldRect = mat.EncloseTransform(focusLocalRect);

            prenderer->SetCxform(Cxform()); // reset color transform for focus rect
            prenderer->SetMatrix(Matrix2F()); // reset matrix to work in world coords


            PointF         coords[4];
            static const UInt16 indices[24] = { 
                0, 1, 2,    2, 1, 3,    2, 4, 5,    5, 4, 6,    7, 3, 8,    8, 3, 9,    5, 9, 10,   10, 9, 11 
            };
#define LIMIT_COORD(c) ((c>32767)?32767.f:((c < -32768)?-32768.f:c))
#define LIMITED_POINT(p) PointF(LIMIT_COORD(p.x), LIMIT_COORD(p.y))

            coords[0] = LIMITED_POINT(focusWorldRect.TopLeft());
            coords[1] = LIMITED_POINT(focusWorldRect.TopRight());
            coords[2] = LIMITED_POINT(focusWorldRect.BottomRight());
            coords[3] = LIMITED_POINT(focusWorldRect.BottomLeft());

            Render::Renderer::VertexXY16i icoords[12];
            // Strip (fill in)
            icoords[0].x = (SInt16) coords[0].x;      icoords[0].y = (SInt16) coords[0].y;        // 0
            icoords[1].x = (SInt16) coords[1].x;      icoords[1].y = (SInt16) coords[1].y;        // 1
            icoords[2].x = (SInt16) coords[0].x;      icoords[2].y = (SInt16) coords[0].y + 40;   // 2
            icoords[3].x = (SInt16) coords[1].x;      icoords[3].y = (SInt16) coords[1].y + 40;   // 3
            icoords[4].x = (SInt16) coords[0].x + 40; icoords[4].y = (SInt16) coords[0].y + 40;   // 4
            icoords[5].x = (SInt16) coords[3].x;      icoords[5].y = (SInt16) coords[3].y - 40;   // 5
            icoords[6].x = (SInt16) coords[3].x + 40; icoords[6].y = (SInt16) coords[3].y - 40;   // 6
            icoords[7].x = (SInt16) coords[1].x - 40; icoords[7].y = (SInt16) coords[1].y + 40;   // 7
            icoords[8].x = (SInt16) coords[2].x - 40; icoords[8].y = (SInt16) coords[2].y - 40;   // 8
            icoords[9].x = (SInt16) coords[2].x;      icoords[9].y = (SInt16) coords[2].y - 40;   // 9 
            icoords[10].x = (SInt16) coords[3].x;     icoords[10].y = (SInt16) coords[3].y;       // 10
            icoords[11].x = (SInt16) coords[2].x;     icoords[11].y = (SInt16) coords[2].y;       // 11

            prenderer->SetVertexData(icoords, 12, Render::Renderer::Vertex_XY16i);

            UInt32 colorValue = 0xFFFF00;
            colorValue ^= (f * 0x1080D0);

            prenderer->FillStyleColor(colorValue | 0xFF000000);

            // Fill the inside
            prenderer->SetIndexData(indices, 24, Render::Renderer::Index_16);
            prenderer->DrawIndexedTriList(0, 0, 12, 0, 8);

            // Done
            prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
            prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
        }
    }
}
*/
void MovieImpl::HideFocusRect(unsigned controllerIdx)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    if (focusGroup.FocusRectShown)
    {
        Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;
        if (curFocused && curFocused->GetParent())
            if (!curFocused->OnLosingKeyboardFocus(NULL, controllerIdx))
                return; // focus loss was prevented
    }
    focusGroup.FocusRectShown = false;
}

bool MovieImpl::SetFocusTo(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt)
{
    // the order of events, if Selection.setFocus is invoked is as follows:
    // Instantly:
    // 1. curFocus.onKillFocus, curFocus = oldFocus
    // 2. curFocus = newFocus
    // 3. curFocus.onSetFocus, curFocus = newFocus
    // 4. Selection focus listeners, curFocus = newFocus
    // Queued:
    // 5. oldFocus.onRollOut, curFocus = newFocus
    // 6. newFocus.onRollOver, curFocus = newFocus
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == ch) return false;

    //?if (!pASMovieRoot->NotifyOnFocusChange(curFocused, ch, controllerIdx, fmt))
    //?    return false;

    if (curFocused && curFocused->GetParent())
    {
        // queue onRollOut, step 5 (since it is queued up, we may do it before TransferFocus)
        if (!curFocused->OnLosingKeyboardFocus(ch, controllerIdx, fmt))
            return false; // focus loss was prevented
    }

    // Do instant focus transfer (steps 1-4)
    TransferFocus(ch, controllerIdx, fmt);

    // invoke onSetFocus for newly set LastFocused
    if (ch)
    {
        // queue onRollOver, step 6
        ch->OnGettingKeyboardFocus(controllerIdx,  fmt);
    }
    return true;
}

bool MovieImpl::QueueSetFocusTo(InteractiveObject* ch, InteractiveObject* ptopMostCh, 
                                unsigned controllerIdx, FocusMovedType fmt,
                                ProcessFocusKeyInfo* pfocusKeyInfo)
{
    // the order of events, if focus key is pressed is as follows:
    // 1. curFocus.onRollOut, curFocus = oldFocus
    // 2. newFocus.onRollOver, curFocus = oldFocus
    // 3. curFocus.onKillFocus, curFocus = oldFocus
    // 4. curFocus = newFocus
    // 5. curFocus.onSetFocus, curFocus = newFocus
    // 6. Selection focus listeners, curFocus = newFocus
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == ch) 
        return false;

#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        // report about focus change to IME. IME may prevent focus changing by
        // returning previously focused character or grant by returning new
        // focusing one.
        ch = pIMEManager->HandleFocus(this, curFocused, ch, ptopMostCh);
        if (curFocused == ch) 
            return false;
    }
#else
    SF_UNUSED(ptopMostCh);
#endif //#ifdef SF_NO_IME_SUPPORT

    if (!pASMovieRoot->NotifyOnFocusChange(curFocused, ch, controllerIdx, fmt, pfocusKeyInfo))
        return false;

    if (ch && (fmt == GFx_FocusMovedByMouse && !ch->DoesAcceptMouseFocus()))
        ch = NULL;

    if (curFocused && curFocused->GetParent())
    {
        // queue onRollOut (step 1)
        if (!curFocused->OnLosingKeyboardFocus(ch, controllerIdx, fmt))
            return false; // if focus loss was prevented - return
    }

    // invoke onSetFocus for newly set LastFocused
    if (ch)
    {
        // queue onRollOver (step 2)
        ch->OnGettingKeyboardFocus(controllerIdx, fmt);
    }

    pASMovieRoot->NotifyQueueSetFocus(ch, controllerIdx, fmt);
    return true;
}

// Instantly transfers focus w/o any queuing
void MovieImpl::TransferFocus(InteractiveObject* pNewFocus, unsigned controllerIdx, FocusMovedType fmt)
{
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);
    Ptr<InteractiveObject> curFocused = focusGroup.LastFocused;

    if (curFocused == pNewFocus) return;

    int avmVersion = pASMovieRoot->GetAVMVersion();

    // AVM1 (AS 1.0/2.0) and AVM2 (AS 3.0) behaves differently while changing the focus.
    // In AS2 onKillFocus event is delivered BEFORE currently focused item is changed
    // (thus, Selection.getFocus() called from onKillFocus returns old focused item).
    // In AS3 focusOut event is dispatched AFTER focused item is changed
    // (thus, stage.focus contains newly focused character already).

    if (avmVersion == 2) // AS 3.0
        focusGroup.LastFocused = pNewFocus;

    if (curFocused && curFocused->GetParent())
    {
        // invoke onKillFocus for LastFocused
        curFocused->OnFocus(InteractiveObject::KillFocus, pNewFocus, controllerIdx, fmt);
    }

    if (avmVersion == 1) // AS 1.0/2.0
        focusGroup.LastFocused = pNewFocus;

    // invoke onSetFocus for newly set LastFocused
    if (pNewFocus)
    {
        pNewFocus->OnFocus(InteractiveObject::SetFocus, curFocused, controllerIdx, fmt);
    }

    pASMovieRoot->NotifyTransferFocus(curFocused, pNewFocus, controllerIdx);
}


void MovieImpl::SetKeyboardFocusTo(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt)
{
#ifndef SF_NO_IME_SUPPORT
    Ptr<IMEManagerBase> pIMEManager = GetIMEManager();
    if (pIMEManager)
    {
        // report about focus change to IME. IME may prevent focus changing by
        // returning previously focused character or grant by returning new
        // focusing one.
    //    Ptr<InteractiveObject> curFocused = LastFocused;
    //    ch = pIMEManager->HandleFocus(this, curFocused, ch, NULL);
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    FocusGroupDescr& focusGroup = GetFocusGroup(controllerIdx);

    if (!ch || ch->GetType() != CharacterDef::TextField)
        focusGroup.FocusRectShown = true;
    else
        focusGroup.FocusRectShown = false;
    
    focusGroup.ResetFocusDirection();
    if (!SetFocusTo(ch, controllerIdx, fmt))
        return;
    // here is a difference with the Flash: in Flash, if you do Selection.setFocus on invisible character or
    // character with invisible parent then Flash sets the focus on it and shows the focusRect.
    // GFxPlayer sets the focus, but doesn't show the focusRect. Probably, it shouldn't set focus at all.
    // (AB, 02/22/07).
    if (focusGroup.FocusRectShown)
    {
        InteractiveObject* cur = ch;
        for (; cur && cur->GetVisible(); cur = cur->GetParent())
            ;
        focusGroup.FocusRectShown = !cur;
    }
}

void MovieImpl::ResetFocusForChar(InteractiveObject* ch)
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        if (FocusGroups[i].IsFocused(ch))
        {
            // send focus out event
            Ptr<InteractiveObject> curFocused = FocusGroups[i].LastFocused;
            if (curFocused)
            {
                UInt32    m = GetControllerMaskByFocusGroup(i);
                unsigned cc = GetControllerCount();
                for (unsigned j = 0; m != 0 && j < cc; ++j, m >>= 1)
                    SetFocusTo(NULL, j, GFx_FocusMovedByKeyboard);
            }
            FocusGroups[i].ResetFocus();
        }
    }
}

bool MovieImpl::IsFocused(const InteractiveObject* ch) const
{
    for (unsigned i = 0; i < FocusGroupsCnt; ++i)
    {
        if (FocusGroups[i].IsFocused(ch))
            return true;
    }
    return false;
}


void MovieImpl::AddTopmostLevelCharacter(InteractiveObject* pch)
{
    SF_ASSERT(pch);
    // do not allow to mark _root/_levelN sprites as topmostLevel...
    if (pch->IsSprite() && pch->CharToSprite()->IsLevelMovie())
        return;

    // make sure this character is not in the list yet
    // we need insert the character at the right pos to maintain original order
    // of topmostLevel characters.
    // So, storing the char with lowest original depth first and the highest last.
    // All characters for higher _level will be stored in front of characters for lower _level.
    
    UPInt i = 0;
    if (TopmostLevelCharacters.GetSize() > 0)
    {
        ArrayDH<DisplayObject*> chParents(GetHeap());
        ArrayDH<DisplayObject*> curParents(GetHeap());
        // fill array of parents for the passed char
        InteractiveObject* pchTopPar = NULL;
        for (InteractiveObject* ppar = pch; ppar; ppar = ppar->GetParent())
        {
            chParents.PushBack(ppar);
            pchTopPar = ppar;
        }

        // find the position according to depth and level. Exhaustive search for now.
        for (UPInt n = TopmostLevelCharacters.GetSize(); i < n; ++i)
        {
            if (TopmostLevelCharacters[i] == pch)
                return;

            // fill array of parent for the current char
            curParents.Resize(0);
            InteractiveObject* pcurTopPar = NULL;
            for (InteractiveObject* ppar = TopmostLevelCharacters[i]; ppar; ppar = ppar->GetParent())
            {
                curParents.PushBack(ppar);
                pcurTopPar = ppar;
            }

            if (pcurTopPar == pchTopPar)
            {
                // compare parents, starting from the top ones
                SPInt chParIdx  = (SPInt)chParents.GetSize()-1;
                SPInt curParIdx = (SPInt)curParents.GetSize()-1;
                bool found = false, cancel_iteration = false;
                for(;chParIdx >= 0 && curParIdx >= 0; --chParIdx, --curParIdx)
                {
                    if (chParents[chParIdx] != curParents[curParIdx])
                    {
                        // parents are different: compare depths of first different parents then.
                        // Note, 'parents' arrays contain the characters itselves, so, there is no 
                        // need to test them separately.
                        SF_ASSERT(chParents[chParIdx]->GetDepth() != curParents[curParIdx]->GetDepth());
                        if (chParents[chParIdx]->GetDepth() < curParents[curParIdx]->GetDepth())
                            found = true;
                        else
                            cancel_iteration = true;
                        break;
                    }
                }
                if (found)
                    break;
                else if (cancel_iteration)
                    continue;
            }
            else
            {
                SF_ASSERT(pchTopPar->CharToSprite_Unsafe()->IsLevelMovie());
                SF_ASSERT(pcurTopPar->CharToSprite_Unsafe()->IsLevelMovie());
                SF_ASSERT(pcurTopPar->CharToSprite_Unsafe()->GetLevel() != 
                          pchTopPar->CharToSprite_Unsafe()->GetLevel());

                // different levels, compare their numbers
                if (pcurTopPar->CharToSprite_Unsafe()->GetLevel() > pchTopPar->CharToSprite_Unsafe()->GetLevel())
                    break; // stop, if we are done with our level
            }
        }
    }
    TopmostLevelCharacters.InsertAt(i, pch);
}

void MovieImpl::RemoveTopmostLevelCharacter(DisplayObjectBase* pch)
{
    for (UPInt i = 0, n = TopmostLevelCharacters.GetSize(); i < n; ++i)
    {
        if (TopmostLevelCharacters[i] == pch)
        {
            TopmostLevelCharacters.RemoveAt(i);
            return;
        }
    }
}

// void MovieImpl::DisplayTopmostLevelCharacters(DisplayContext &context) const
// {
//     Matrix2F* pom  = context.pParentMatrix;
//     Cxform* pocf = context.pParentCxform;
//     for (UPInt i = 0, n = TopmostLevelCharacters.GetSize(); i < n; ++i)
//     {
//         DisplayObjectBase* pch = TopmostLevelCharacters[i];
//         SF_ASSERT(pch);
//         Matrix2F       m = pch->GetParent()->GetWorldMatrix();
//         Cxform      cf = pch->GetParent()->GetWorldCxform();
//         
//         context.pParentMatrix = &m;
//         context.pParentCxform = &cf;
//         pch->Display(context);
//     }
//     context.pParentMatrix = pom;
//     context.pParentCxform = pocf;
// }

// Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
void MovieImpl::SetIMECandidateListStyle(const IMECandidateListStyle& st)
{
#ifndef SF_NO_IME_SUPPORT
    if (!pIMECandidateListStyle)
        pIMECandidateListStyle = SF_HEAP_NEW(pHeap) IMECandidateListStyle(st);
    else
        *pIMECandidateListStyle = st;
#else
    SF_UNUSED(st);
#endif //#ifdef SF_NO_IME_SUPPORT
}

// Gets style of candidate list
void MovieImpl::GetIMECandidateListStyle(IMECandidateListStyle* pst) const
{
#ifndef SF_NO_IME_SUPPORT
    if (!pIMECandidateListStyle)
        *pst = IMECandidateListStyle();
    else
        *pst = *pIMECandidateListStyle;
#else
    SF_UNUSED(pst);
#endif //#ifdef SF_NO_IME_SUPPORT
}

bool MovieImpl::GetDirtyFlag(bool doReset)
{
    bool dirtyFlag = G_IsFlagSet<Flag_DirtyFlag>(Flags);
    if (doReset)
        G_SetFlag<Flag_DirtyFlag>(Flags, false);
    return dirtyFlag;
}

Render::Text::Allocator* MovieImpl::GetTextAllocator()
{
    return pASMovieRoot->GetTextAllocator();
}

UInt64 MovieImpl::GetASTimerMs() const
{
    TestStream* pts = GetTestStream();
    UInt64 timerMs;
    if (pts)
    {
        if (pts->TestStatus == TestStream::Record)
        {
            timerMs = Timer::GetTicks()/1000 - StartTickMs;
            LongFormatter f(timerMs);
            f.Convert();
            pts->SetParameter("timer", f.ToCStr());
        }
        else
        {
           String tstr;
           pts->GetParameter("timer", &tstr);
           timerMs = SFatouq(tstr.ToCStr());
        }

    }
    else
        timerMs = Timer::GetTicks()/1000 - StartTickMs;
    return timerMs;
}

PointF MovieImpl::TranslateToScreen(const PointF& p, Matrix2F* puserMatrix)
{
    Matrix2F worldMatrix   = pMainMovie->GetWorldMatrix();
    Matrix2F mat           = ViewportMatrix;

    if (puserMatrix)
        mat.Prepend(*puserMatrix);
    mat.Prepend(worldMatrix);
    return mat.Transform(PixelsToTwips(p));
}

RectF  MovieImpl::TranslateToScreen(const RectF& r, Matrix2F* puserMatrix)
{
    Matrix2F worldMatrix   = pMainMovie->GetWorldMatrix();
    Matrix2F mat           = ViewportMatrix;

    if (puserMatrix)
        mat.Prepend(*puserMatrix);
    mat.Prepend(worldMatrix);
    return mat.EncloseTransform(PixelsToTwips(r));
}

// pathToCharacter - path to a character, i.e. "_root.hud.mc";
// pt is in pixels, in coordinate space of the character specified by the pathToCharacter
// returning value is in pixels of screen.
bool MovieImpl::TranslateLocalToScreen(const char* pathToMovieClip, 
                                       const PointF& pt, 
                                       PointF* presPt, 
                                       Matrix2F* puserMatrix)
{
    SF_ASSERT(0); // @TODO! redo
    SF_UNUSED4(pathToMovieClip, pt, presPt, puserMatrix);
#if 0
    SF_ASSERT(presPt);
    // context of MovieImpl
    GASEnvironment* penv = GetLevelMovie(0)->GetASEnvironment();
    Value res;
    bool found = penv->GetVariable(penv->CreateString(pathToCharacter), &res);
    if (!found)
        return false;// oops.
    InteractiveObject* pchar = res.ToASCharacter(penv);
    // now we can get matrix from the character and translate the point
    if (!pchar)
        return false; // oops.
    // note, point coords should be in twips here!
    PointF pointInTwips = PixelsToTwips(pt);

    Renderer::Matrix worldMatrix   = pchar->GetWorldMatrix();
    Renderer::Matrix mat           = ViewportMatrix;

    mat.Prepend(userMatrix);
    mat.Prepend(worldMatrix);
    *presPt = mat.Transform(pointInTwips);
#endif
    return true;
}

// Obtains statistics for the movie view.
void    MovieImpl::GetStats(StatBag* pbag, bool reset)
{
    AdvanceStats->GetStats(pbag, reset);
}

void   MovieImpl::ForceCollectGarbage() 
{ 
    pASMovieRoot->ForceCollect();
}

// Sets a movie at level; used for initialization.
bool   MovieImpl::SetLevelMovie(int level, DisplayObjContainer *psprite)
{
    unsigned i = 0;
    SF_ASSERT(level >= 0);
    SF_ASSERT(psprite);

    for (; i < MovieLevels.GetSize(); i++)
    {
        if (MovieLevels[i].Level >= level)
        {           
            if (MovieLevels[i].Level == level)
            {
                SF_DEBUG_WARNING1(1, "MovieImpl::SetLevelMovie fails, level %d already occupied", level);
                return 0;
            }           
            // Found insert spot.
            break;              
        }
    }
    G_SetFlag<Flag_LevelClipsChanged>(Flags, true);

    // Insert the item.
    MovieImpl::LevelInfo li;
    li.Level  = level;
    li.pSprite= psprite;
    MovieLevels.InsertAt(i, li);

    psprite->OnInsertionAsLevel(level);

    if (level == 0)
    {
        SetMainMovie(psprite);
        SetMovieDefImpl(psprite->GetResourceMovieDef());
        if (pMainMovieDef)
        {
            AdvanceStats->SetName(pMainMovieDef->GetFileURL());
        }
        pStateBag->SetDelegate(pMainMovieDef->pStateBag);
        // Frame timing
        FrameTime    = 1.0f / GetMovieDef()->GetFrameRate();

        if (!G_IsFlagSet<Flag_ViewportSet>(Flags))
        {
            MovieDefImpl* pdef = psprite->GetResourceMovieDef();
            Viewport desc((int)pdef->GetWidth(), (int)pdef->GetHeight(), 0,0, (int)pdef->GetWidth(), (int)pdef->GetHeight());
            SetViewport(desc);
        }
    }

    G_SetFlag<Flag_NeedMouseUpdate>(Flags, true);
    return 1;
}

// Create new instance names for unnamed objects.
ASString    MovieImpl::CreateNewInstanceName()
{
    InstanceNameCount++; // Start at 0, so first value is 1.
    char pbuffer[48] = { 0 };

    Format(StringDataPtr(pbuffer, sizeof(pbuffer)), "instance{0}", InstanceNameCount);

    return StringManager.CreateString(pbuffer);
}

ImageResource* MovieImpl::GetImageResourceByLinkageId(MovieDefImpl* md, const char* linkageId)
{
    Ptr<ImageResource> pimageRes;
    if (md) // can it be NULL?
    {
        SF_ASSERT(linkageId);
        bool    userImageProtocol = 0;

        // Check to see if URL is a user image substitute.
        if (linkageId[0] == 'i' || linkageId[0] == 'I')
        {
            String urlLowerCase = String(linkageId).ToLower();

            if (urlLowerCase.Substring(0, 6) == "img://")
                userImageProtocol = true;
            else if (urlLowerCase.Substring(0, 8) == "imgps://")
                userImageProtocol = true;
        }

        if (userImageProtocol)
        {
            //@IMG
//             // Create image through image callback
//             StateBag* pss       = pStateBag;
//             MemoryHeap* pheap   = GetMovieHeap();
//             pimageRes = 
//                 *LoaderImpl::LoadMovieImage(linkageId,
//                 pss->GetImageLoader(), pss->GetLog(), pheap);
        }
        else
        {
            // Get exported resource for linkageId and verify that it is an image.
            ResourceBindData resBindData;
            if (!FindExportedResource(md, &resBindData, linkageId))
                return NULL;
            SF_ASSERT(resBindData.pResource.GetPtr() != 0);
            // Must check resource type, since users can theoretically pass other resource ids.
            if (resBindData.pResource->GetResourceType() == Resource::RT_Image)
            {
                pimageRes = (ImageResource*)resBindData.pResource.GetPtr();
            }
        }
    }
    return pimageRes;
}

// ****************************************************************************
// Helper function to load data from a file
//
bool    MovieImpl::ReadLoadVariables(File* pfile, String* pdata, int* pfileLen)
{
    SF_ASSERT(pfile);
    SF_ASSERT(pdata);
    SF_ASSERT(pfileLen);

    if ((*pfileLen = pfile->GetLength()) == 0)
        return false;

    String str;

    UByte* td = (UByte*) SF_ALLOC(*pfileLen, Stat_Default_Mem);
    pfile->Read(td, *pfileLen);

    wchar_t*            wcsptr = NULL;
    UByte*              ptextData = td;
    int                 textLength = *pfileLen;

    // the following converts byte stream to appropriate endianness
    // for UTF16/UCS2 (wide char format)
    UInt16* prefix16 = (UInt16*)td;
    if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFFFE)) // little endian
    {
        prefix16++;
        ptextData = (UByte*)prefix16;
        textLength = (*pfileLen / 2) - 1;
        if (sizeof(wchar_t) == 2)
        {
            for (int i=0; i < textLength; i++)
                prefix16[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
        }
        else
        {
            // special case: create an aux buffer to hold the data
            wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), Stat_Default_Mem);
            for (int i=0; i < textLength; i++)
                wcsptr[i] = (wchar_t)Alg::ByteUtil::LEToSystem(prefix16[i]);
            ptextData = (UByte*)wcsptr;
        }
        str.AppendString( (const wchar_t*)ptextData, textLength );
    }
    else if (prefix16[0] == Alg::ByteUtil::BEToSystem((UInt16)0xFEFF)) // big endian
    {
        prefix16++;
        ptextData = (UByte*)prefix16;
        textLength = (*pfileLen / 2) - 1;
        if (sizeof(wchar_t) == 2)
        {
            for (int i=0; i < textLength; i++)
                prefix16[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
        }
        else
        {
            wcsptr = (wchar_t*) SF_ALLOC(textLength * sizeof(wchar_t), Stat_Default_Mem);
            for (int i=0; i < textLength; i++)
                wcsptr[i] = Alg::ByteUtil::BEToSystem(prefix16[i]);
            ptextData = (UByte*)wcsptr;
        }
        str.AppendString( (const wchar_t*)ptextData, textLength );
    }
    else if (*pfileLen > 2 && td[0] == 0xEF && td[1] == 0xBB && td[2] == 0xBF)
    {
        // UTF-8 with explicit BOM
        ptextData += 3;
        textLength -= 3;
        str.AppendString( (const char*)ptextData, textLength );
    }
    else
    {
        str.AppendString( (const char*)ptextData, textLength );
    }

    if (wcsptr)
        SF_FREE(wcsptr);

    // following works directly on bytes
    ASUtils::Unescape(str.ToCStr(), str.GetSize(), pdata);

    SF_FREE(td);
    return true;
}

/*

// In 

void MovieImpl::CreateString(Value* pvalue, const char* pstring)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED2(pvalue, pstring);
}

void MovieImpl::CreateStringW(Value* pvalue, const wchar_t* pstring)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED2(pvalue, pstring);
}

void MovieImpl::CreateObject(Value* pvalue, const char* className, 
                  const Value* pargs, unsigned nargs)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED4(pvalue, className, pargs, nargs);
}
void MovieImpl::CreateArray(Value* pvalue)
{
    SF_ASSERT(0); //@TODO!
    SF_UNUSED1(pvalue);
}
*/


bool MovieImpl::AttachDisplayCallback
    (const char* pathToObject, void (SF_CDECL *callback)(void* userPtr), void* userPtr)
{
    return pASMovieRoot->AttachDisplayCallback(pathToObject, callback, userPtr);
}

void MovieImpl::ProcessUnloadQueue()
{
    // process unload queue
    if (pUnloadListHead)
    {
        InteractiveObject* pnextCh = NULL;
        for (InteractiveObject* pcurCh = pUnloadListHead; pcurCh; pcurCh = pnextCh)
        {
            SF_ASSERT(pcurCh->IsUnloaded());
            SF_ASSERT(!pcurCh->IsOptAdvancedListFlagSet());
#ifdef SF_BUILD_DEBUG
            InteractiveObject* psavedParent = pcurCh->GetParent();
#endif

            pnextCh = pcurCh->pNextUnloaded;
            pcurCh->pNextUnloaded = NULL;
            pcurCh->OnEventUnload();

            // Remove from parent's display list
            InteractiveObject* pparent = pcurCh->GetParent();

#ifdef SF_BUILD_DEBUG
            SF_ASSERT(psavedParent == pparent);
#endif

            if (pparent)
                pparent->RemoveDisplayObject(pcurCh);

            pcurCh->Release();
        }
        pUnloadListHead = NULL;
    }
}

void MovieImpl::ReleaseUnloadList()
{
//     InteractiveObject* pnextCh = NULL;
//     for (InteractiveObject* pcurCh = pUnloadListHead; pcurCh; pcurCh = pnextCh)
//     {
//         SF_ASSERT(pcurCh->IsUnloaded());
//         SF_ASSERT(!pcurCh->IsOptAdvancedListFlagSet());
// 
//         pnextCh = pcurCh->pNextUnloaded;
//         pcurCh->pNextUnloaded = NULL;
//         pcurCh->Release();
//     }
//     pUnloadListHead = NULL;
}

// finds first available font manager (by traversing through root nodes).
FontManager* MovieImpl::FindFontManager()
{
    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {   
        if (pdefNode->pFontManager)
            return pdefNode->pFontManager;

        pdefNode = pdefNode->pNext;
    }
    return NULL; // not found
}

Sprite* FocusGroupDescr::GetModalClip(MovieImpl* proot)
{
    if (ModalClip)
    {
        Ptr<DisplayObject> modalChar = ModalClip->ResolveCharacter(proot);
        if (modalChar)
        {
            SF_ASSERT(modalChar->IsSprite());
            return modalChar->CharToSprite_Unsafe();
        }
    }
    return NULL;
}

// ***** GFxValue definitions

String GFx::Value::ToString() const
{
    String retVal;

    switch(GetType())
    {
    case VT_Undefined:
        {
            retVal = String("undefined");
        }
        break;
    case VT_Null:
        {  
            retVal = String("null");
        }
        break;
    case VT_Boolean:
        {  
            retVal = String(mValue.BValue ? "true" : "false");
        }
        break;
    case VT_Number:
        {
            char buf[NumberUtil::TOSTRING_BUF_SIZE];
            retVal = String(NumberUtil::ToString(mValue.NValue, buf, sizeof(buf))); // Precision is 10 by default.
        }
        break;
    case VT_String:
        {
            retVal = String(GetString());
        }
        break;
    case VT_StringW:
        {
            retVal = String(GetStringW());
        }
        break;
    case VT_Object:
    case VT_Array:
    case VT_DisplayObject:
        {               
            pObjectInterface->ToString(&retVal, *this);
        }
        break;
    default:
        {
            retVal = "<bad type>";
            SF_ASSERT(0);
        }
    }
    return retVal;
}

const wchar_t* GFx::Value::ToStringW(wchar_t* pwstr, UPInt len) const
{
    switch(GetType())
    {
    case VT_StringW:
        {
            SFwcscpy(pwstr, len, GetStringW());
            return pwstr;
        }
        break;
    default:
        {
            UTF8Util::DecodeString(pwstr, ToString().ToCStr());   
        }
        break;
    }
    return pwstr;
}

#ifdef SF_BUILD_DEBUG
void Value::ObjectInterface::DumpTaggedValues() const
{
    SF_DEBUG_MESSAGE(1, "** Begin Tagged GFx::Values Dump **");
    const Value* data = ExternalObjRefs.GetFirst();
    while (!ExternalObjRefs.IsNull(data))
    {
        const char* ptypestr = NULL;
        switch (data->GetType())
        {
        case Value::VT_Array: ptypestr = "Array"; break;
        case Value::VT_DisplayObject: ptypestr = "DispObj"; break;
        default: ptypestr = "Object";
        }
        SF_DEBUG_MESSAGE2(1, "> [%s] : %p", ptypestr, data);
        data = ExternalObjRefs.GetNext(data);
    }
    SF_DEBUG_MESSAGE(1, "** End Tagged GFx::Values Dump **");
}
#endif

bool MovieImpl::FindExportedResource(MovieDefImpl* localDef, ResourceBindData *presBindData, const String& symbol)
{
    if (localDef->GetExportedResource(presBindData, symbol))
        return true;

    MovieDefImpl* curDef = localDef;

    // not found in local def - look through "import parents" (movies which import the local one directly or indirectly)
    // Thus, if a moviedef was loaded by loadMovie it should look only inside the movie and ITS imports,
    // it shouldn't go to the movie that actually loaded it.
    MovieDefRootNode *pdefNode = RootMovieDefNodes.GetFirst();
    while(!RootMovieDefNodes.IsNull(pdefNode))
    {      
        if (pdefNode->pDefImpl != localDef && pdefNode->pDefImpl->DoesDirectlyImport(curDef))
        {
            if (pdefNode->pDefImpl->GetExportedResource(presBindData, symbol))
                return true;
            curDef = pdefNode->pDefImpl;
        }
        pdefNode = pdefNode->pNext;
    }
    return false;
}

DrawingContext* MovieImpl::CreateDrawingContext()
{
    DrawingContext* dc = SF_HEAP_NEW(pHeap) DrawingContext(pHeap, GetRenderContext(), GetImageCreator());
    DrawingContextList.PushBack(dc);
    return dc;
}

void MovieImpl::UpdateAllDrawingContexts()
{
    DrawingContext* curDC = DrawingContextList.GetFirst();
    for (; !DrawingContextList.IsNull(curDC); curDC = curDC->pNext)
    {
        if (curDC->IsDirty())
        {
            curDC->UpdateRenderNode();
        }
    }
}

void MovieImpl::ClearDrawingContextList()
{
    DrawingContext* curDC = DrawingContextList.GetFirst();
    DrawingContext* pnext;
    for (; !DrawingContextList.IsNull(curDC); curDC = pnext)
    {
        pnext = curDC->pNext;
        curDC->pPrev = curDC->pNext = NULL;
    }
    DrawingContextList.Clear();
}

void MovieImpl::AddMovieDefToKillList(MovieDefImpl* md)
{
    if (md)
    {
        MDKillListEntry e;
        e.FrameId   = RenderContext.GetFrameId();
        e.pMovieDef = md;
        MovieDefKillList.PushBack(e);
    }
}

void MovieImpl::ProcessMovieDefToKillList()
{
    if (MovieDefKillList.GetSize() > 0)
    {
        UInt64 finFrId = RenderContext.GetFinalizedFrameId();
        for (UPInt i = 0; i < MovieDefKillList.GetSize();)
        {
            if (finFrId >= MovieDefKillList[i].FrameId)
            {
                MovieDefKillList.RemoveAt(i);
            }
            else
                ++i;
        }
    }
}

}} // namespace Scaleform::GFx
