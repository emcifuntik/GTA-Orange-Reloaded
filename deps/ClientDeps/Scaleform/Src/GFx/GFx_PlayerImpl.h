/**********************************************************************

Filename    :   GFxPlayerImpl.h
Content     :   MovieRoot and Definition classes
Created     :   
Authors     :   Michael Antonov, Artem Bolgar, Prasad Silva

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   This file contains class declarations used in
                GFxPlayerImpl.cpp only. Declarations that need to be
                visible by other player files should be placed
                in DisplayObjectBase.h.


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_PlayerImpl_H
#define INC_SF_GFX_PlayerImpl_H

#include "GFx/GFx_Sprite.h"
#include "Render/Render_Math2D.h"
#include "Kernel/SF_File.h"

#include "GFx/GFx_DisplayList.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_Shape.h"
//#include "GFx/AS2/AS2_StringManager.h"
#include "GFx/GFx_MediaInterfaces.h"

// Font managers are allocated in sprite root nodes.
#include "GFx/GFx_FontManager.h"

#include "Render/Text/Text_Core.h" // for Text::Allocator
#include "GFx/GFx_PlayerTasks.h"

// For now
#include "GFx/GFx_MovieDef.h"

#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
#include "IME/GFxIMEImm32Dll.h"
#endif //defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(GFC_NO_IME_SUPPORT)

// This is internal file that uses SF_HEAP_NEW.
#include "Kernel/SF_HeapNew.h"

#include "GFx/GFx_Input.h"
#include "GFx/GFx_ASMovieRootBase.h"
#include "GFx/AMP/Amp_ViewStats.h"

#include "Render/Render_Context.h"
#include "GFx/GFx_DrawingContext.h"

#include "Render/Render_ScreenToWorld.h"

#define GFX_MAX_CONTROLLERS_SUPPORTED 16

namespace Scaleform {
	
#ifdef GFX_ENABLE_SOUND
	namespace Sound
	{
		class   SoundRenderer;
	}
#endif
	
namespace GFx {

// ***** Declared Classes
class MovieDefImpl;
class MovieImpl;
class SpriteDef;
// Helpers
class ImportInfo;
class SwfEvent;
class LoadQueueEntry;
class DoublePrecisionGuard;
// Tag classes
class PlaceObjectTag;
class PlaceObject2Tag;
class PlaceObject3Tag;
class RemoveObjectTag;
class RemoveObject2Tag;
class SetBackgroundColorTag;
// class GASDoAction;           - in GFxAction.cpp
// class GFxStartSoundTag;      - in GFxSound.cpp

// ***** External Classes
class Loader;

class IMECandidateListStyle;

#ifdef GFX_ENABLE_SOUND
class AudioBase;
#endif

//  ***** MovieDefRootNode

// MovieDefRootNode is maintained in MovieImpl for each MovieDefImpl; this
// node is referenced by every GFxSprite which has its own MovieDefImpl. We keep
// a distinction between imported and loadMovie based root sprites.
// 
// Purpose:
//  1) Allows us to cache temporary state of loading MovieDefs in the beginning of
//     Advance so that it is always consistent for all instances. If not done, it
//     would be possible to different instances of the same progressively loaded
//     file to be in different places in the same GfxMovieView frame.
//  2) Allows for FontManager to be shared among same-def movies.

struct MovieDefRootNode : public ListNode<MovieDefRootNode>, public NewOverrideBase<StatMV_Other_Mem>
{
    // The number of root sprites that are referencing this MovieDef. If this
    // number goes down to 0, the node is deleted.
    unsigned        SpriteRefCount;

    // MovieDef these root sprites use. No need to AddRef because GFxSprite does.
    MovieDefImpl*   pDefImpl;

    // Cache the number of frames that was loaded.
    unsigned        LoadingFrame;
    UInt32          BytesLoaded;
    // Imports don't get to rely on LoadingFrame because they are usually nested
    // and have independent frame count from import root. This LoadingFrame is
    // not necessary anyway because imports are guaranteed to be fully loaded.
    bool            ImportFlag;

    // The pointer to font manager used for a sprite. We need to keep font manager
    // here so that it can keep references to all MovieDefImpl instances for
    // all fonts obtained through GFxFontLib.
    Ptr<FontManager> pFontManager;

    MovieDefRootNode(MovieDefImpl *pdefImpl, bool importFlag = 0)
        : SpriteRefCount(1), pDefImpl(pdefImpl), ImportFlag(importFlag)
    {  }
};

class TabIndexSortFunctor
{
public:
    inline bool operator()(const InteractiveObject* a, const InteractiveObject* b) const
    {
        return (a->GetTabIndex() < b->GetTabIndex());
    }
};

class AutoTabSortFunctor
{
    enum
    {
        Epsilon = 20
    };
public:
    inline bool operator()(const InteractiveObject* a, const InteractiveObject* b) const
    {
        DisplayObjectBase::Matrix ma = a->GetLevelMatrix();
        DisplayObjectBase::Matrix mb = b->GetLevelMatrix();
        RectF aRect  = ma.EncloseTransform(a->GetFocusRect());
        RectF bRect  = mb.EncloseTransform(b->GetFocusRect());

        //@DBG
        //printf("Comparing %s with %s\n", a->GetCharacterHandle()->GetNamePath().ToCStr(), b->GetCharacterHandle()->GetNamePath().ToCStr());

        PointF centerA = aRect.Center();
        PointF centerB = bRect.Center();

        if (Alg::Abs(aRect.y1 - bRect.y1) <= Epsilon ||
            Alg::Abs(aRect.y2 - bRect.y2) <= Epsilon ||
            Alg::Abs(centerA.y - centerB.y) <= Epsilon)
        {
            // same row
            //@DBG
            //printf ("   same row, less? %d, xA = %f, xB = %f\n", int(centerA.x < centerB.x), centerA.x , centerB.x);
            return centerA.x < centerB.x;
        }
        //@DBG
        //printf ("   less? %d, yA = %f, yB = %f\n", int(centerA.y < centerB.y), centerA.y , centerB.y);
        return centerA.y < centerB.y;
    }
};


class ASIntervalTimerIntf : public RefCountBase<ASIntervalTimerIntf, StatMV_ActionScript_Mem>
{
public:
    virtual ~ASIntervalTimerIntf() {}

    virtual void            Start(MovieImpl* proot) =0;
    virtual bool            Invoke(MovieImpl* proot, float frameTime) =0;
    virtual bool            IsActive() const =0;
    virtual void            Clear() =0;
    virtual UInt64          GetNextInvokeTime() const =0;
    virtual void            SetId(int id) =0;
    virtual int             GetId() const =0;
};

// Focus related structures
struct FocusGroupDescr
{
    enum
    {
        TabableArray_Initialized        = 0x1,
        TabableArray_WithFocusEnabled   = 0x2
    };
    UInt8                               TabableArrayStatus;
    ArrayDH<Ptr<InteractiveObject>, StatMV_Other_Mem> TabableArray; 
    mutable WeakPtr<InteractiveObject>  LastFocused;
    Ptr<CharacterHandle>                ModalClip;
    UInt32                              LastFocusKeyCode;
    RectF                               LastFocusedRect;
    bool                                FocusRectShown;

    FocusGroupDescr(MemoryHeap* heap = NULL):
    TabableArrayStatus(0), TabableArray((!heap) ? Memory::GetHeapByAddress(GetThis()): heap), 
        LastFocusKeyCode(0), FocusRectShown(false) {}

    bool                IsFocused(const InteractiveObject* ch) const 
    { 
        Ptr<InteractiveObject> lch = LastFocused; return lch.GetPtr() == ch; 
    }
    void                ResetFocus() { LastFocused = NULL; }
    void                ResetFocusDirection() { LastFocusKeyCode = 0; }
    void                ResetTabableArray()
    {
        if (TabableArrayStatus & TabableArray_Initialized)
        {
            TabableArray.Resize(0);
            TabableArrayStatus = 0;
        }
    }
    Sprite* GetModalClip(MovieImpl* proot);

private:
    FocusGroupDescr* GetThis() { return this; }
};

struct ProcessFocusKeyInfo : public NewOverrideBase<StatMV_Other_Mem>
{
    FocusGroupDescr*        pFocusGroup;
    Ptr<InteractiveObject>  CurFocused;
    int                     CurFocusIdx;
    RectF                   Prev_aRect;
    UInt32                  PrevKeyCode;
    UInt32                  KeyCode;
    UInt8                   KeyboardIndex;
    UInt8                   KeysState;
    bool                    ManualFocus;
    bool                    InclFocusEnabled;
    bool                    Initialized;

    ProcessFocusKeyInfo():pFocusGroup(NULL), CurFocusIdx(-1),PrevKeyCode(0),KeyCode(0),
        KeyboardIndex(0), KeysState(0), ManualFocus(false),InclFocusEnabled(false),
        Initialized(false) {}
};

//
// ***** MovieImpl
//
// Global, shared root state for a GFxMovieSub and all its characters.
//

class MovieImpl : public Movie
{
    friend class Movie;

public:
    typedef Matrix2F Matrix;
    // Expose Value::ObjectInterface to other classes, to avoid protected.
    typedef Value::ObjectInterface ValueObjectInterface;

    // Storage for wide char conversions (VT_ConvertStringW). Stored per Value (if needed)
    struct WideStringStorage : public RefCountBase<WideStringStorage, StatMV_Other_Mem>
    {
        ASStringNode*   pNode;
        UByte           pData[1];

        WideStringStorage(ASStringNode* pnode, UPInt utf8len) : pNode(pnode) 
        {
            pNode->AddRef();
            // NOTE: pData must be guaranteed to have enough space for (wchar_t * strLen+1)
            UTF8Util::DecodeString((wchar_t*)pData, pNode->pData, utf8len);
        }
        ~WideStringStorage()                { pNode->Release(); }
        SF_INLINE const wchar_t*  ToWStr()  { return (const wchar_t*)pData; }

    private:
        // Copying is prohibited
        WideStringStorage(const WideStringStorage&);
        const WideStringStorage& operator = (const WideStringStorage&);
    };
    
    struct LevelInfo
    {
        // Level must be >= 0. -1 is used to indicate lack of a level
        // elsewhere, but that is not allowed here.
        int                     Level;
        Ptr<InteractiveObject>  pSprite;
    };
    
    // Obtains cached states. The mat is only accessed if CachedStatesFlag is not
    // set, which meas we are outside of Advance and Display.
    Log*                 GetCachedLog() const
    {
         // Do not modify CachedLogFlag; that is only for Advance/Display.
        if (!G_IsFlagSet<Flag_CachedLogFlag>(Flags))        
            pCachedLog = GetLog();
        return pCachedLog;
    }
    
#ifdef GFX_ENABLE_KEYBOARD
    // Keyboard

    const KeyboardState*    GetKeyboardState(unsigned keyboardIndex) const 
    { 
        if (keyboardIndex < GFX_MAX_KEYBOARD_SUPPORTED)
            return &KeyboardStates[keyboardIndex]; 
        return NULL;
    }
    KeyboardState*          GetKeyboardState(unsigned keyboardIndex) 
    { 
        if (keyboardIndex < GFX_MAX_KEYBOARD_SUPPORTED)
            return &KeyboardStates[keyboardIndex]; 
        return NULL;
    }
    void SetKeyboardListener(KeyboardState::IListener*);
#endif

    // Return value class - allocated after global context.
    // Used because GASString has no global or NewOverrideBase.
    struct ReturnValueHolder : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        char*     CharBuffer;
        unsigned  CharBufferSize;
        ArrayCC<ASString, StatMV_ActionScript_Mem>   StringArray;
        unsigned  StringArrayPos;

        ReturnValueHolder(ASStringManager* pmgr)
            : CharBuffer(0), CharBufferSize(0),
            StringArray(pmgr->CreateConstString("")),
            StringArrayPos(0) { }
        ~ReturnValueHolder() { if (CharBuffer) SF_FREE(CharBuffer); }

        SF_INLINE char* PreAllocateBuffer(unsigned size)
        {
            size = (size + 4095)&(~(4095));
            if (CharBufferSize < size || (CharBufferSize > size && (CharBufferSize - size) > 4096))
            {
                if (CharBuffer)
                    CharBuffer = (char*)SF_REALLOC(CharBuffer, size, StatMV_ActionScript_Mem);
                else
                    CharBuffer = (char*)SF_ALLOC(size, StatMV_ActionScript_Mem);
                CharBufferSize = size;
            }
            return CharBuffer;
        }
        SF_INLINE void ResetPos() { StringArrayPos = 0; }
        SF_INLINE void ResizeStringArray(unsigned n)
        {
            StringArray.Resize(Alg::Max(1u,n));
        }
    };

    class DragState
    {
    public:
        InteractiveObject*  pCharacter;
        bool                LockCenter;
        bool                Bound;
        // Bound coordinates
        PointF              BoundLT;
        PointF              BoundRB;
        // The difference between character origin and mouse location
        // at the time of dragStart, used and computed if LockCenter == 0.
        PointF              CenterDelta;

        DragState()
            : pCharacter(0), LockCenter(0), Bound(0), 
              BoundLT(0.0), BoundRB(0.0), CenterDelta(0.0)
            { }

        // Initializes lockCenter and mouse centering delta
        // based on the character.
        void InitCenterDelta(bool lockCenter);
    };

    // Sticky variable hash link node.
    struct StickyVarNode : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        ASString            Name;
        //void*             Value;
        StickyVarNode*      pNext;
        bool                Permanent;

/*        StickyVarNode(const GASString& name, void* pvalue, bool permanent)
            : Name(name), Value(value), pNext(0), Permanent(permanent) { }
        StickyVarNode(const StickyVarNode &node)
            : Name(node.Name), Value(node.Value), pNext(node.pNext), Permanent(node.Permanent) { }
        const StickyVarNode& operator = (const StickyVarNode &node)
            { pNext = node.pNext; Name = node.Name; Value = node.Value; Permanent = node.Permanent; return *this; }
*/
        StickyVarNode(const ASString& name, bool permanent)
            : Name(name), pNext(0), Permanent(permanent) { }
        StickyVarNode(const StickyVarNode &node)
            : Name(node.Name), pNext(node.pNext), Permanent(node.Permanent) { }
        virtual ~StickyVarNode() {}

        const StickyVarNode& operator = (const StickyVarNode &node)
        { pNext = node.pNext; Name = node.Name; Permanent = node.Permanent; return *this; }
    };

    enum FlagsType
    {
        // Set once the viewport has been specified explicitly.
        Flag_ViewportSet                    = 0x0001,

        // States are cached then this flag is set.
        Flag_CachedLogFlag                  = 0x0002,

        // Verbosity - assigned from ActionControl.
        Flag_VerboseAction                  = 0x0004,
        Flag_LogRootFilenames               = 0x0008,
        Flag_LogChildFilenames              = 0x0010,
        Flag_LogLongFilenames               = 0x0020,
        Flag_SuppressActionErrors           = 0x0040,    

        Flag_NeedMouseUpdate                = 0x0080,

        Flag_LevelClipsChanged              = 0x0100,
        // Set if Advance has not been called yet - generates warning on Display.
        Flag_AdvanceCalled                  = 0x0200,
        Flag_DirtyFlag                      = 0x0400,
        Flag_NoInvisibleAdvanceFlag         = 0x0800,
        Flag_SetCursorTypeFuncOverloaded    = 0x1000,

        // Flags for event handlers
        Flag_ContinueAnimation              = 0x2000,
        // 0x4000,
        Flag_OnEventLoadProgressCalled      = 0x8000,

        Flag_DisableFocusAutoRelByClick     = 0x010000,

        Flag_BackgroundSetByTag             = 0x020000,
        Flag_MovieIsFocused                 = 0x040000,
        Flag_OptimizedAdvanceListInvalid    = 0x080000,

        Flag_Paused                         = 0x100000,
        Flag_ExitRequested                  = 0x200000,
        Flag_AcceptAnimMovesWith3D          = 0x400000,

        // Focus-related AS extension properties
        // Disables focus release when mouse moves
        Shift_DisableFocusAutoRelease       = 22, // 0xC00000

        // Enables moving focus by arrow keys even if _focusRect is set to false
        Shift_AlwaysEnableFocusArrowKeys    = 24,

        // Enables firing onPress/onRelease even if _focusRect is set to false
        Shift_AlwaysEnableKeyboardPress     = 26,

        // Disables firing onRollOver/Out if focus is changed by arrow keys
        Shift_DisableFocusRolloverEvent     = 28,

        // Disables default focus handling by arrow and tab keys
        Shift_DisableFocusKeys              = 30
    };
#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
    GFxIMEImm32Dll      Imm32Dll;
#endif

    // *** Constructor / Destructor

    MovieImpl(MemoryHeap* pheap);
    ~MovieImpl();

    bool Init(MovieDefImpl* pmovieDef)
    {
        return pASMovieRoot->Init(pmovieDef);
    }

    ASStringManager*        GetStringManager() { return &StringManager; }//.GetStringManager(); }
    InteractiveObject*      GetMainMovie() const { return pMainMovie; }
    void                    SetMainMovie(InteractiveObject* p) { pMainMovie = p; }
    void                    RegisterAuxASClasses()  { pASMovieRoot->RegisterAuxASClasses(); }
    bool                    SetLevelMovie(int level, DisplayObjContainer *psprite);

    // Returns a movie at level, or null if no such movie exists.
    bool                    ReleaseLevelMovie(int level);

    void ClearDisplayList() { pASMovieRoot->ClearDisplayList(); }
    void ClearPlayList();

    // Non-virtual version of GetHeap(), used for efficiency.
    MemoryHeap*             GetMovieHeap() const { return pHeap; }

     
    // Finds a character given a global path. Path must 
    // include a _levelN entry as first component.
    SF_INLINE InteractiveObject*  FindTarget(const ASString& path) const;

    
    // Dragging support.
    void                SetDragState(const DragState& st)   { CurrentDragState = st; }
    void                GetDragState(DragState* st)         { *st = CurrentDragState; }
    void                StopDrag()                          { CurrentDragState.pCharacter = NULL; }
    bool                IsDraggingCharacter(const InteractiveObject* ch) const { return CurrentDragState.pCharacter == ch; }


    // Internal use in characters, etc.
    // Use this to retrieve the last state of the mouse.
    virtual void        GetMouseState(unsigned mouseIndex, float* x, float* y, unsigned* buttons);

    const MouseState* GetMouseState(unsigned mouseIndex) const
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }
    MouseState* GetMouseState(unsigned mouseIndex)
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }
    void                SetMouseCursorCount(unsigned n)
    {
        MouseCursorCount = (n <= GFX_MAX_MICE_SUPPORTED) ? n : GFX_MAX_MICE_SUPPORTED;
    }
    unsigned            GetMouseCursorCount() const
    {
        return MouseCursorCount;
    }
    virtual void        SetControllerCount(unsigned n)
    {
        ControllerCount = (n <= GFX_MAX_KEYBOARD_SUPPORTED) ? n : GFX_MAX_KEYBOARD_SUPPORTED;
    }
    virtual unsigned    GetControllerCount() const
    {
        return ControllerCount;
    }
    bool                IsMouseSupportEnabled() const { return MouseCursorCount > 0; }
    
    // Return the size of a logical GFxMovieSub pixel as
    // displayed on-screen, with the current device
    // coordinates.
    float               GetPixelScale() const               { return PixelScale; }

    // Returns time elapsed since the first Advance, in seconds (with fraction part)
    Double              GetTimeElapsed() const              { return Double(TimeElapsed)/1000; }
    // Returns time elapsed since the first Advance, in milliseconds
    UInt64              GetTimeElapsedMs() const            { return TimeElapsed; }

    float               GetFrameTime() const                { return FrameTime; }
    UInt64              GetStartTickMs() const              { return StartTickMs; }
    UInt64              GetASTimerMs() const;

    
    // Create new instance names for unnamed objects.
    ASString           CreateNewInstanceName();

    // *** Load/Unload movie support
    
    // Head of load queue.
    LoadQueueEntry*     pLoadQueueHead;
    UInt32              LastLoadQueueEntryCnt;

    // Adds load queue entry and takes ownership of it.
    void                AddLoadQueueEntry(LoadQueueEntry *pentry);
    void                AddLoadQueueEntryMT(LoadQueueEntryMT* pentryMT);

    // Processes the load queue handling load/unload instructions.  
    void                ProcessLoadQueue();


    // *** Helpers for loading images.

    typedef Loader::FileFormatType FileFormatType;
        
    // Create a loaded image MovieDef based on image resource.
    // If load states are specified, they are used for bind states. Otherwise,
    // new states are created based on pStateBag and our loader.
    MovieDefImpl*       CreateImageMovieDef(ImageResource *pimageResource, bool bilinear,
                                            const char *purl, LoadStates *pls = 0);

    ImageResource*      GetImageResourceByLinkageId(MovieDefImpl* md, const char* linkageId);

    // Fills in a file system path relative to _level0. The path
    // will contain a trailing '/' if not empty, so that relative
    // paths can be concatenated directly to it.
    // Returns 1 if the path is non-empty.
    bool                GetMainMoviePath(String *ppath) const;
    

    // *** Action List management

    // *** Movie implementation

    virtual void        SetViewport(const Viewport& viewDesc);
    virtual void        GetViewport(Viewport *pviewDesc) const;
    virtual void        SetViewScaleMode(ScaleModeType);
    virtual ScaleModeType   GetViewScaleMode() const                        { return ViewScaleMode; }
    virtual void        SetViewAlignment(AlignType);
    virtual AlignType   GetViewAlignment() const                            { return ViewAlignment; }
    virtual RectF       GetVisibleFrameRect() const                         { return TwipsToPixels(VisibleFrameRect); }
    const RectF&        GetVisibleFrameRectInTwips() const                  { return VisibleFrameRect; }

    virtual RectF       GetSafeRect() const                                 { return SafeRect; }
    virtual void        SetSafeRect(const RectF& rect)                     { SafeRect = rect; }

    void                UpdateViewport();

    virtual void        SetVerboseAction(bool verboseAction)
    { 
#if !defined(GFX_AS2_VERBOSE) && !defined(GFX_AS3_VERBOSE)
        SF_DEBUG_WARNING(1, "VerboseAction is disabled by the GFX_AS2/AS3_VERBOSE macros in GConfig.h\n");
#endif
        G_SetFlag<Flag_VerboseAction>(Flags, verboseAction); 
    }
    // Turn off/on log for ActionScript errors..
    virtual void        SetActionErrorsSuppress(bool suppressActionErrors)  { G_SetFlag<Flag_SuppressActionErrors>(Flags, suppressActionErrors); }
    // Background color.
    virtual void        SetBackgroundColor(const Color color);
    virtual void        SetBackgroundAlpha(float alpha);
    virtual float       GetBackgroundAlpha() const                  { return BackgroundColor.GetAlpha() / 255.0f; }

    bool                IsBackgroundSetByTag() const                { return G_IsFlagSet<Flag_BackgroundSetByTag>(Flags); }
    void                SetBackgroundColorByTag(const Color color) 
    { 
        SetBackgroundColor(color); 
        G_SetFlag<Flag_BackgroundSetByTag>(Flags, true); 
    }

    // Actual execution and timeline control.
    virtual float       Advance(float deltaT, unsigned frameCatchUpCount, bool capture = true);

    virtual void        Capture(bool onChangeOnly = true)
    {
        if (!onChangeOnly || RenderContext.HasChanges())
            RenderContext.Capture();
    }

    virtual const MovieDisplayHandle& GetDisplayHandle() const { return hDisplayRoot; }


    // An internal method that advances frames for movieroot's sprites
    void                AdvanceFrame(bool nextFrame, float framePos);
    void                InvalidateOptAdvanceList() { G_SetFlag<Flag_OptimizedAdvanceListInvalid>(Flags, true); }
    bool                IsOptAdvanceListInvalid() const { return G_IsFlagSet<Flag_OptimizedAdvanceListInvalid>(Flags); }
    void                ProcessUnloadQueue();
    // Releases list of unloaded characters
    void                ReleaseUnloadList();

    // Events.
    virtual unsigned    HandleEvent(const Event &event);
    virtual void        NotifyMouseState(float x, float y, unsigned buttons, unsigned mouseIndex = 0);
    virtual bool        HitTest(float x, float y, HitTestType testCond = HitTest_Shapes, unsigned controllerIdx = 0);

    /*
    virtual void        SetUserEventHandler(UserEventCallback phandler, void* puserData) 
    { 
        pUserEventHandler = phandler; 
        pUserEventHandlerData = puserData;
    }

    // FSCommand
    virtual void        SetFSCommandCallback (FSCommandCallback phandler)   { pFSCommandCallback = phandler; }
    */

    virtual void*       GetUserData() const                                 { return UserData; }
    virtual void        SetUserData(void* ud)                               { UserData = ud;  }

    virtual bool        AttachDisplayCallback(const char* pathToObject, void (SF_CDECL *callback)(void* userPtr), void* userPtr);

    // returns timerId to use with ClearIntervalTimer
    int                 AddIntervalTimer(ASIntervalTimerIntf *timer);  
    void                ClearIntervalTimer(int timerId);      
    void                ShutdownTimers();

#ifdef GFX_ENABLE_VIDEO
    void                AddVideoProvider(VideoProvider*);
    void                RemoveVideoProvider(VideoProvider*);
#endif
    
    // *** Movie implementation

    // Many of these methods delegate to pMovie methods of the same name; however,
    // they are hidden into the .cpp file because GFxSprite is not yet defined.
    
    virtual MovieDef*    GetMovieDef() const;
    
    SF_INLINE LoaderImpl*   GetLoaderImpl() const               { return pMainMovieDef->pLoaderImpl; }
    SF_INLINE MovieDefImpl* GetMovieDefImpl() const             { return pMainMovieDef; }     
    SF_INLINE void          SetMovieDefImpl(MovieDefImpl* p) { pMainMovieDef = p; }
    
    virtual unsigned    GetCurrentFrame() const;
    virtual bool        HasLooped() const;  
    virtual void        Restart();  
    virtual void        GotoFrame(unsigned targetFrameNumber);
    virtual bool        GotoLabeledFrame(const char* label, int offset = 0);
    
    virtual void        SetPause(bool pause);

    virtual void        SetPlayState(PlayState s);
    virtual PlayState   GetPlayState() const;
    virtual void        SetVisible(bool visible);
    virtual bool        GetVisible() const;
    
    void                AddStickyVariableNode(const ASString& path, StickyVarNode* pnode);
//    SF_INLINE void        AddStickyVariable(const GASString& fullPath, const Value &val, SetVarType setType);
//    SF_INLINE void        ResolveStickyVariables(InteractiveObject *pcharacter);
    void                ClearStickyVariables();

    InteractiveObject*  GetTopMostEntity(const PointF& mousePos, unsigned controllerIdx, 
        bool testAll, const InteractiveObject* ignoreMC = NULL);  

    // Profiling
    Ptr<AMP::ViewStats> AdvanceStats;

    // Obtains statistics for the movie view.
    virtual void        GetStats(StatBag* pbag, bool reset);


    virtual MemoryHeap* GetHeap() const { return pHeap; }

    // Forces to run garbage collection, if it is enabled. Does nothing otherwise.
    virtual void        ForceCollectGarbage();

    // ***** GFxStateBag implementation
    
    virtual StateBag* GetStateBagImpl() const   { return pStateBag.GetPtr(); }

    // Mark/unmark the movie as one that has "focus". This is user responsibility to set or clear
    // the movie focus. This is important for stuff like IME functions correctly.
    void                OnMovieFocus(bool set);
    virtual bool        IsMovieFocused() const { return G_IsFlagSet<Flag_MovieIsFocused>(Flags); }
    // Returns and optionally resets the "dirty flag" that indicates 
    // whether anything was changed on the stage (and need to be 
    // re-rendered) or not.
    virtual bool        GetDirtyFlag(bool doReset = true);
    void                SetDirtyFlag() { G_SetFlag<Flag_DirtyFlag>(Flags, true); }

    void                SetNoInvisibleAdvanceFlag(bool f) { G_SetFlag<Flag_NoInvisibleAdvanceFlag>(Flags, f); }
    void                ClearNoInvisibleAdvanceFlag()     { SetNoInvisibleAdvanceFlag(false); }
    bool                IsNoInvisibleAdvanceFlagSet() const { return G_IsFlagSet<Flag_NoInvisibleAdvanceFlag>(Flags); }

    void                SetContinueAnimationFlag(bool f) { G_SetFlag<Flag_ContinueAnimation>(Flags, f); }
    void                ClearContinueAnimationFlag()     { SetContinueAnimationFlag(false); }
    bool                IsContinueAnimationFlagSet() const { return G_IsFlagSet<Flag_ContinueAnimation>(Flags); }

    bool                IsVerboseAction() const        { return G_IsFlagSet<Flag_VerboseAction>(Flags); }
    bool                IsSuppressActionErrors() const { return G_IsFlagSet<Flag_SuppressActionErrors>(Flags); }
    bool                IsLogRootFilenames() const  { return G_IsFlagSet<Flag_LogRootFilenames>(Flags); }
    bool                IsLogChildFilenames() const { return G_IsFlagSet<Flag_LogChildFilenames>(Flags); }
    bool                IsLogLongFilenames() const  { return G_IsFlagSet<Flag_LogLongFilenames>(Flags); }
    bool                IsAlwaysEnableKeyboardPress() const; 
    bool                IsAlwaysEnableKeyboardPressSet() const;
    void                SetAlwaysEnableKeyboardPress(bool f);
    bool                IsDisableFocusRolloverEvent() const;
    bool                IsDisableFocusRolloverEventSet() const;
    void                SetDisableFocusRolloverEvent(bool f);
    bool                IsDisableFocusAutoRelease() const;
    bool                IsDisableFocusAutoReleaseSet() const;
    void                SetDisableFocusAutoRelease(bool f);
    bool                IsDisableFocusAutoReleaseByMouseClick() const;
    void                SetDisableFocusAutoReleaseByMouseClick(bool f);
    bool                IsAlwaysEnableFocusArrowKeys() const;
    bool                IsAlwaysEnableFocusArrowKeysSet() const;
    void                SetAlwaysEnableFocusArrowKeys(bool f);
    bool                IsDisableFocusKeys() const;
    bool                IsDisableFocusKeysSet() const;
    void                SetDisableFocusKeys(bool f);

    // Focus related functionality
    // Focus-related methods
    void                InitFocusKeyInfo(ProcessFocusKeyInfo* pfocusInfo, 
                                         const InputEventsQueue::QueueEntry::KeyEntry& keyEntry, 
                                         bool inclFocusEnabled,
                                         FocusGroupDescr* pfocusGroup = NULL);
    // Process keyboard input for focus
    void                ProcessFocusKey(Event::EventType event,
                                        const InputEventsQueue::QueueEntry::KeyEntry& keyEntry,
                                        ProcessFocusKeyInfo* pfocusInfo);
    void                FinalizeProcessFocusKey(ProcessFocusKeyInfo* pfocusInfo);

    // Internal methods that processes keyboard and mouse input
    void                ProcessInput();
    void                ProcessKeyboard(const InputEventsQueue::QueueEntry* qe, 
                                        ProcessFocusKeyInfo* focusKeyInfo);
    void                ProcessMouse(const InputEventsQueue::QueueEntry* qe, 
                                     UInt32* miceProceededMask, bool avm2);

#ifdef SF_BUILD_LOGO
    Ptr<ImageInfo>    pDummyImage;
#endif

protected:
    void                FillTabableArray(const ProcessFocusKeyInfo* pfocusInfo);
    FocusGroupDescr&    GetFocusGroup(unsigned controllerIdx) 
    { 
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED &&
            FocusGroupIndexes[controllerIdx] < GFX_MAX_CONTROLLERS_SUPPORTED);
        return FocusGroups[FocusGroupIndexes[controllerIdx]]; 
    }
    const FocusGroupDescr&  GetFocusGroup(unsigned controllerIdx) const 
    {    
        SF_ASSERT(controllerIdx < GFX_MAX_CONTROLLERS_SUPPORTED &&
            FocusGroupIndexes[controllerIdx] < GFX_MAX_CONTROLLERS_SUPPORTED);
        return FocusGroups[FocusGroupIndexes[controllerIdx]]; 
    }

    void                ResetMouseState();
    void                ResetKeyboardState();

public:
    unsigned            GetFocusGroupIndex(unsigned controllerIdx) const
    {
        return FocusGroupIndexes[controllerIdx]; 
    }
    // returns a bit-mask where each bit represents a physical controller, 
    // associated with the specified focus group.
    UInt32              GetControllerMaskByFocusGroup(unsigned focusGroupIndex);
    // Displays yellow rectangle around focused item
    void                DisplayFocusRect(const DisplayContext& context);
    // Hides yellow focus rectangle. This may happen if mouse moved.
    void                HideFocusRect(unsigned controllerIdx);
    // Returns the character currently with focus
    Ptr<InteractiveObject> GetFocusedCharacter(unsigned controllerIdx)
    { 
        return Ptr<InteractiveObject>(GetFocusGroup(controllerIdx).LastFocused); 
    }
    // Checks, is the specified item focused by the specified controller or not?
    bool                IsFocused(const InteractiveObject* ch, unsigned controllerIdx) const 
    { 
        return GetFocusGroup(controllerIdx).IsFocused(ch);
    }
    // Checks, if the 'ch' focused by ANY controller.
    bool                IsFocused(const InteractiveObject* ch) const;

    // Checks, is the specified item focused or not for keyboard input
    bool                IsKeyboardFocused(const InteractiveObject* ch, unsigned controllerIdx) const 
    { 
        return (IsFocused(ch, controllerIdx) && IsFocusRectShown(controllerIdx)); 
    }
    // Transfers focus to specified item. For movie clips and buttons the keyboard focus will not be set
    // to specified character, as well as focus rect will not be drawn.
    // return false, if focus change was prevented by NotifyOnFocusChange
    bool                SetFocusTo(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt);

    // sets LastFocused to NULL
    void                ResetFocus(unsigned controllerIdx) 
    { 
        GetFocusGroup(controllerIdx).LastFocused = NULL; 
    }
    void                ResetFocusForChar(InteractiveObject* ch);
    
    // Queue up setting of focus. ptopMostCh is might be necessary for stuff like IME to 
    // determine its state; might be NULL, if not available.
    // return false, if focus change was prevented by NotifyOnFocusChange
    bool                QueueSetFocusTo
        (InteractiveObject* ch, InteractiveObject* ptopMostCh, unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* = NULL);
    // Instantly transfers focus to specified item. For movie clips and buttons the keyboard focus will not be set
    // Instantly invokes pOldFocus->OnKillFocus, pNewFocus->OnSetFocus and Selection.bradcastMessage("onSetFocus").
    void                TransferFocus(InteractiveObject* pNewFocus, unsigned controllerIdx, FocusMovedType fmt);
    // Transfers focus to specified item. The keyboard focus transfered as well, as well as focus rect 
    // will be drawn (unless it is disabled).
    void                SetKeyboardFocusTo(InteractiveObject*, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard);
    // Returns true, if yellow focus rect CAN be shown at the time of call. This method will
    // return true, even if _focusrect = false and it is a time to show the rectangle.
    inline bool         IsFocusRectShown(unsigned controllerIdx) const
    { 
        return GetFocusGroup(controllerIdx).FocusRectShown; 
    }
    void                ResetTabableArrays();
    void                ResetFocusStates();

    void                ActivateFocusCapture(unsigned controllerIdx);
    // Sets modal movieclip. That means focus keys (TAB, arrow keys) will 
    // move focus only inside of this movieclip. To reset the modal clip set
    // to NULL.
    void                SetModalClip(Sprite* pmovie, unsigned controllerIdx);
    Sprite*             GetModalClip(unsigned controllerIdx);

    // associate a focus group with a controller.
    virtual bool        SetControllerFocusGroup(unsigned controllerIdx, unsigned logCtrlIdx);

    // returns focus group associated with a controller
    virtual unsigned    GetControllerFocusGroup(unsigned controllerIdx) const;

    unsigned            GetFocusGroupCount() const { return FocusGroupsCnt; }

    void                AddTopmostLevelCharacter(InteractiveObject*);
    void                RemoveTopmostLevelCharacter(DisplayObjectBase*);
    //void                DisplayTopmostLevelCharacters(DisplayContext &context) const;

    // Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
    void                SetIMECandidateListStyle(const IMECandidateListStyle& st);
    // Gets style of candidate list
    void                GetIMECandidateListStyle(IMECandidateListStyle* pst) const;

#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && !defined(SF_NO_IME_SUPPORT)
    // handle korean IME (core part)
    unsigned            HandleKoreanIME(const IMEEvent& imeEvent);
#endif // SF_NO_BUILTIN_KOREAN_IME

    Render::Text::Allocator*    GetTextAllocator();

    // Translates the point or rectangle in Flash coordinates to screen 
    // (window) coordinates. These methods takes into account the world matrix
    // of root, the viewport matrix and the user matrix from the renderer. 
    // Source coordinates should be in root coordinate space, in pixels.
    virtual PointF      TranslateToScreen(const PointF& p, Matrix2F* puserMatrix = 0);
    virtual RectF       TranslateToScreen(const RectF& p, Matrix2F* puserMatrix = 0);

    // Translates the point in the character's coordinate space to the point on screen (window).
    // pathToCharacter - path to a movieclip, textfield or button, i.e. "_root.hud.mc";
    // pt is in pixels, in coordinate space of the character, specified by the pathToCharacter
    // returning value is in pixels of screen.
    virtual bool        TranslateLocalToScreen(const char* pathToCharacter, 
                                   const PointF& pt, 
                                   PointF* presPt, 
                                   Matrix2F* userMatrix = 0);
    // Changes the shape of the mouse cursor. If Mouse.setCursorType is overriden by ActionScript
    // when it will be invoked. This function should be used instead of GASMouseCtorFunc::SetCursorType
    // to provide callback in user's ActionScript.
    void                ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType);

    // finds first available font manager (by traversing through root nodes).
    FontManager*        FindFontManager();

    // Read variables data to the pdata. Data is in format var1="value"&var2="value2"
    static bool         ReadLoadVariables(File* pfile, String* pdata, int* pfileLen);

    // Checks if exit was requested by ActionScript.
    virtual bool        IsExitRequested() const { return G_IsFlagSet<Flag_ExitRequested>(Flags); }
    void                RequestExit() { G_SetFlag<Flag_ExitRequested>(Flags, true); }

    // Checks if timeline animation is allowed to run when setting 3D properties, generally not for AS3
    virtual bool        AcceptAnimMovesWith3D() const { return G_IsFlagSet<Flag_AcceptAnimMovesWith3D>(Flags); }
    void                SetAcceptAnimMovesWith3D(bool b) { G_SetFlag<Flag_AcceptAnimMovesWith3D>(Flags, b); }

    // This function is similar to GFxMovieDefImpl::GetExportedResource but besides searching in local
    // it looks also in all imported moviedefs starting from level0 def.
    bool                FindExportedResource(MovieDefImpl* localDef, ResourceBindData *pdata, const String& symbol);
//     void                SetRenderRoot(Render::TreeRoot* rt) 
//     { 
//         pRenderRoot = rt; 
//         // @TEMP
//         Matrix2F m;
//         m.AppendScaling(0.05f);
//         pRenderRoot->SetMatrix(m);
//     } //@HACK
    Render::Context&    GetRenderContext() { return RenderContext; }
    Render::TreeRoot*   GetRenderRoot()    { return pRenderRoot; }

    DrawingContext*     CreateDrawingContext();

    void                AddMovieDefToKillList(MovieDefImpl*);
    void                ProcessMovieDefToKillList();

private:
    void                CheckMouseCursorType(unsigned mouseIdx, InteractiveObject* ptopMouseCharacter);
    // iterates through all drawing contexts and re-create renderer treenodes, if necessary.
    void                UpdateAllDrawingContexts();
    // clears the list of drawing context with zeroing pNext/pPrev in each node.
    void                ClearDrawingContextList();
    // update 3D view and projection after viewport changes
    void                UpdateViewAndPerspective();                         
public: // data

    static void MakeViewAndPersp3D(Matrix3F *matView, Matrix4F *matPersp, 
        float DisplayWidth, float DisplayHeight, PointF Center,
        float perspFOV=55,                        // degrees, default flash persp
        bool bInvertY=false);

    // 3D view matrix for movie
    virtual const Matrix3F *    GetViewMatrix3D() { return &ViewMat; }
    virtual void                SetViewMatrix3D(const Matrix3F& m) { ViewMat = m; }

    // 3D projection matrix for movie
    virtual const Matrix4F *    GetProjectionMatrix3D() { return &ProjectionMat; }
    virtual void                SetProjectionMatrix3D(const Matrix4F& m) { ProjectionMat = m; }

    // *** Complex object interface support

    // VM specific interface used to access complex objects (Object, etc.)
    Value::ObjectInterface*     pObjectInterface;

    // Heap used for all allocations within MovieImpl.
    MemoryHeap*                 pHeap;
    ASStringManager             StringManager;

    // Convenience pointer to _level0's Def (root in AS3). Keep this around even if _level0 unloads,
    // to avoid external crashes.
    Ptr<MovieDefImpl>           pMainMovieDef;
    // Pointer to sprite in level0 or null. Stored for convenient+efficient access.
    InteractiveObject*          pMainMovie;

    // Sorted array of currently loaded movie levels. Level 0 has
    // special significance, since it dictates viewport scale, etc. 
    // For AS3 this array always has only 1 element with Level = 0
    ArrayLH<LevelInfo, StatMV_Other_Mem>  MovieLevels;

    // A list of root MovieDefImpl objects used by all root sprites; these objects
    // can be loaded progressively in the background.
    List<MovieDefRootNode>      RootMovieDefNodes;

    Ptr<StateBagImpl>           pStateBag;

    // Renderer
    Ptr<Render::TreeRoot>       pRenderRoot;
    MovieDisplayHandle          hDisplayRoot;

    GFx::Viewport               mViewport;
    float                       PixelScale;
    // View scale values, used to adjust input mouse coordinates
    // map viewport -> movie coordinates.
    float                       ViewScaleX, ViewScaleY;
    float                       ViewOffsetX, ViewOffsetY; // in stage pixels (not viewport's ones)
    ScaleModeType               ViewScaleMode;
    AlignType                   ViewAlignment;
    RectF                       VisibleFrameRect; // rect, in swf coords (twips), visible in curr viewport
    RectF                       SafeRect;         // storage for user-define safe rect
    Matrix                      ViewportMatrix; 
    Render::ScreenToWorld       ScreenToWorld;

#ifdef GFX_ENABLE_VIDEO
    HashSet<Ptr<VideoProvider> > VideoProviders;
#endif

    // *** States cached in Advance()
    
    mutable Ptr<Log>            pCachedLog;

    // Handler pointer cached during advance.
    Ptr<UserEventHandler>       pUserEventHandler;
    Ptr<FSCommandHandler>       pFSCommandHandler;
    Ptr<ExternalInterface>      pExtIntfHandler;

    Ptr<FontManagerStates>      pFontManagerStates;

#ifdef GFX_ENABLE_SOUND
    // We don't keep them here as Ptr<> to avoid dependances from SoundRenderer
    // in another parts of GFx (like IME). We hold these pointer here only for caching propose
    AudioBase*                  pAudio;
    Sound::SoundRenderer*       pSoundRenderer;
#endif

    // *** Special reference for XML object manager
    ExternalLibPtr*             pXMLObjectManager;

    // Amount of time that has elapsed since start of playback. Used for
    // reporting in action instruction and setTimeout/setInterval. In microseconds.
    UInt64                      TimeElapsed;
    // Time remainder from previous advance, updated every frame.
    float                       TimeRemainder;
    // Cached seconds per frame; i.e., 1.0f / FrameRate.
    float                       FrameTime;

    unsigned                    ForceFrameCatchUp;

    GFx::InputEventsQueue       InputEventsQueue;

    Color                       BackgroundColor;
    MouseState                  mMouseState[GFX_MAX_MICE_SUPPORTED];
    unsigned                    MouseCursorCount;
    unsigned                    ControllerCount;
    void*                       UserData;

#ifdef GFX_ENABLE_KEYBOARD
    // Keyboard
    KeyboardState               KeyboardStates[GFX_MAX_KEYBOARD_SUPPORTED];
#endif

    ReturnValueHolder*          pRetValHolder;

    // Instance name assignment counter.
    UInt32                      InstanceNameCount;

    DragState                   CurrentDragState;   // @@ fold this into GFxMouseButtonState?

    // Sticky variable clip hash table.
    ASStringHash<StickyVarNode*>StickyVariables;


    ArrayLH<Ptr<InteractiveObject>, StatMV_Other_Mem> TopmostLevelCharacters;

    ArrayLH<Ptr<Sprite>, StatMV_Other_Mem>  SpritesWithHitArea;
    UInt64                                  StartTickMs, PauseTickMs;

    // interval timer stuff
    ArrayLH<Ptr<ASIntervalTimerIntf> ,StatMV_Other_Mem> IntervalTimers;
    int                                     LastIntervalTimerId;

    // Focus management stuff
    FocusGroupDescr                         FocusGroups[GFX_MAX_CONTROLLERS_SUPPORTED];
    unsigned                                FocusGroupsCnt;
    // Map controller index to focus group
    UInt8                                   FocusGroupIndexes[GFX_MAX_CONTROLLERS_SUPPORTED];

    // The head of the playlist.
    // Playlist head, the first child for events or frame actions execution.
    // "Playable" children characters (ASCharacters) are linked by InteractiveObject::pPlayNext
    // and InteractiveObject::pPlayPrev pointers. If pPlayPrev == NULL, then this is the first
    // element in playlist, if pPlayNext == NULL - the last one.
    InteractiveObject*                      pPlayListHead;
    InteractiveObject*                      pPlayListOptHead;
    // List of all unloaded but not destroyed yet characters.
    InteractiveObject*                      pUnloadListHead;

    UInt32                                  Flags;

    IMECandidateListStyle*                  pIMECandidateListStyle; // stored candidate list style
#if defined(SF_OS_WIN32) && defined(GFX_ENABLE_BUILTIN_KOREAN_IME) && defined(GFX_ENABLE_IME)
    GFxIMEImm32Dll                          Imm32Dll;
#endif

    LoadQueueEntryMT*                       pLoadQueueMTHead;
private:
    // A list of all drawing context to be updated at the end of Advance
    List<DrawingContext>                    DrawingContextList;

    // an array of moviedefs to be released, if Render::Context allows. When movie unloads,
    // its moviedefimpl should be put in this array and the array will release the def later, 
    // in Advance.
    struct MDKillListEntry
    {
        UInt64              FrameId;
        Ptr<MovieDefImpl>   pMovieDef;
    };
    ArrayLH<MDKillListEntry>                MovieDefKillList;
    UInt64                                  MovieDefKillFrameId;

    // should be defined last to make sure its dtor is called first.
    Render::Context                         RenderContext; 

    Matrix3F                ViewMat;
    Matrix4F                ProjectionMat;
};

// ** Inline Implementation
SF_INLINE bool MovieImpl::IsAlwaysEnableKeyboardPress() const 
{
    return G_Is3WayFlagTrue<Shift_AlwaysEnableKeyboardPress>(Flags);
}
SF_INLINE bool MovieImpl::IsAlwaysEnableKeyboardPressSet() const 
{
    return G_Is3WayFlagSet<Shift_AlwaysEnableKeyboardPress>(Flags);
}
SF_INLINE void MovieImpl::SetAlwaysEnableKeyboardPress(bool f) 
{
    G_Set3WayFlag<Shift_AlwaysEnableKeyboardPress>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusRolloverEvent() const 
{
    return G_Is3WayFlagTrue<Shift_DisableFocusRolloverEvent>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusRolloverEventSet() const 
{
    return G_Is3WayFlagSet<Shift_DisableFocusRolloverEvent>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusRolloverEvent(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusRolloverEvent>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusAutoRelease() const
{
    return G_Is3WayFlagTrue<Shift_DisableFocusAutoRelease>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusAutoReleaseSet() const
{
    return G_Is3WayFlagSet<Shift_DisableFocusAutoRelease>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusAutoRelease(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusAutoRelease>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusAutoReleaseByMouseClick() const
{
    return G_IsFlagSet<Flag_DisableFocusAutoRelByClick>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusAutoReleaseByMouseClick(bool f) 
{
    G_SetFlag<Flag_DisableFocusAutoRelByClick>(Flags, f);
}

SF_INLINE bool MovieImpl::IsAlwaysEnableFocusArrowKeys() const
{
    return G_Is3WayFlagTrue<Shift_AlwaysEnableFocusArrowKeys>(Flags);
}
SF_INLINE bool MovieImpl::IsAlwaysEnableFocusArrowKeysSet() const
{
    return G_Is3WayFlagSet<Shift_AlwaysEnableFocusArrowKeys>(Flags);
}
SF_INLINE void MovieImpl::SetAlwaysEnableFocusArrowKeys(bool f) 
{
    G_Set3WayFlag<Shift_AlwaysEnableFocusArrowKeys>(Flags, f);
}

SF_INLINE bool MovieImpl::IsDisableFocusKeys() const
{
    return G_Is3WayFlagTrue<Shift_DisableFocusKeys>(Flags);
}
SF_INLINE bool MovieImpl::IsDisableFocusKeysSet() const
{
    return G_Is3WayFlagSet<Shift_DisableFocusKeys>(Flags);
}
SF_INLINE void MovieImpl::SetDisableFocusKeys(bool f) 
{
    G_Set3WayFlag<Shift_DisableFocusKeys>(Flags, f);
}

/*SF_INLINE void MovieImpl::AddStickyVariable(const GASString& fullPath, const Value &val, SetVarType setType)
{
    pASMovieRoot->AddStickyVariable(fullPath, val, setType);
}

SF_INLINE void MovieImpl::ResolveStickyVariables(InteractiveObject *pcharacter)
{
    pASMovieRoot->ResolveStickyVariables(pcharacter);
}*/

SF_INLINE InteractiveObject* MovieImpl::FindTarget(const ASString& path) const
{
    return pASMovieRoot->FindTarget(path);
}

// ** End Inline Implementation


//!AB: This class restores high precision mode of FPU for X86 CPUs.
// Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
// leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
// be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
class DoublePrecisionGuard
{
    unsigned    fpc;
    //short       fpc;
public:

    DoublePrecisionGuard ()
    {
#if defined (SF_CC_MSVC) && defined(SF_CPU_X86)
  #if (SF_CC_MSVC >= 1400)
        // save precision mode (control word)
        _controlfp_s(&fpc, 0, 0);
        // set 53 bit precision
        unsigned _fpc;
        _controlfp_s(&_fpc, _PC_53, _MCW_PC);
  #else
        // save precision mode (control word)
        fpc = _controlfp(0,0);
        // set 53 bit precision
        _controlfp(_PC_53, _MCW_PC);
  #endif
        /*
        // save precision mode (only for X86)
        GASM fstcw fpc;
        //short _fpc = (fpc & ~0x300) | 0x300;  // 64-bit mantissa (REAL10)
        short _fpc = (fpc & ~0x300) | 0x200;  // 53-bit mantissa (REAL8)
        // set 53 bit precision
        GASM fldcw _fpc;
        */
#endif
    }

    ~DoublePrecisionGuard ()
    {
#if defined (SF_CC_MSVC) && defined (SF_CPU_X86)
  #if (SF_CC_MSVC >= 1400)
        // restore precision mode
        unsigned _fpc;
        _controlfp_s(&_fpc, fpc, _MCW_PC);
  #else
        _controlfp(fpc, _MCW_PC);
  #endif
        /*
        // restore precision mode (only for X86)
        GASM fldcw fpc;
        */
#endif
    }
};

}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_PlayerImpl_H
