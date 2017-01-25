/**********************************************************************

Filename    :   Sprite.h
Content     :   Implementation of MovieClip character
Created     :   
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_SF_GFX_SPRITE_H
#define INC_SF_GFX_SPRITE_H

#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/GFx_DrawingContext.h"
#include "GFx/GFx_SpriteDef.h"
#include "GFx/GFx_ASSoundIntf.h"
#include "Sound/Sound_SoundRenderer.h"

namespace Scaleform { namespace GFx {

class FontManager;
struct MovieDefRootNode;

class StaticTextSnapshotData;

// An interface for implementation of AS-specific part of Sprite
class AvmSpriteBase : public AvmDisplayObjContainerBase
{
public:
    virtual DisplayObjectBase*  AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame,
        UInt32 addFlags,
        CharacterCreateInfo* pcharCreateOverride,
        InteractiveObject* origChar) =0;
    virtual void AdvanceFrame(bool nextFrame, float framePos) =0;
    virtual void CloneInternalData(const InteractiveObject* src) =0;
    virtual void ExecuteFrame0Events() =0;
    virtual void ExecuteFrameTags(unsigned frame) =0;
    virtual void ExecuteInitActionFrameTags(unsigned frame) =0;
    virtual void ForceShutdown() =0;
    virtual int  GetLevel() const        { return -1; }
    virtual bool IsLevelMovie() const =0;

#ifndef SF_NO_IME_SUPPORT
	virtual bool CheckCandidateListLevel(int) const {return false;}
#endif
};

// AS-independent part of Sprite implementation
class Sprite : public DisplayObjContainer
{
    typedef TimelineDef::Frame  Frame;
    typedef Matrix2F            Matrix;
    typedef Cxform              Cxform;

    // Data source for playlist (either SpriteDef of GFxMovieDataDef).
    Ptr<TimelineDef>    pDef;

    PlayState           PlayStatePriv;
    unsigned            CurrentFrame;

    // A node used by every root sprite in a tree so that they can be
    // scanned and initialized efficiently to a thread-synched Advance.
    // Null for all non-root sprites.
    MovieDefRootNode*   pRootNode;

#if defined(GFX_ENABLE_SOUND)
public:
    // keep a track of active sound playing in this sprite
    struct ActiveSoundItem : public RefCountBaseNTS<ActiveSoundItem, StatMV_Other_Mem>
    {
        ActiveSoundItem();
        ~ActiveSoundItem();

        Ptr<Sound::SoundChannel> pChannel;          // active channel
        ASSoundIntf*             pSoundObject;      // AS sound object which created this sound
        // it can be NULL for sound which were created 
        // by a sound object (event,streaming,video sounds)
        // it is used for calling Sound.onSoundComplete handler
        SoundResource*           pResource;         // resource from the swf resource lib 
        // could be NULL for stream and video sounds.
        // the object will be addrefed manually to avoid compile dependances
        // it is used for stoping sounds by linkage name
    };
    // list of active sound and sound attributes and sound objects
    struct ActiveSounds : public NewOverrideBase<StatMV_Other_Mem>
    {
        ActiveSounds();
        ~ActiveSounds();

        int                             Volume;         // volume for this movie clip
        int                             Pan;            // specifying the left-right balance for a sound (-100 -- 0 -- 100)
        ArrayLH<Ptr<ActiveSoundItem> >  Sounds;         // list of active sound channels
        ArrayLH<ASSoundIntf*>           ASSounds;       // list of all AS sound objects attached to this sprite
        Ptr<Sound::SoundChannel>        pStreamSound;   // streaming sound of this movie clip
    };

private:
    // active sound started in this sprite
    ActiveSounds*           pActiveSounds;
    int                     FindActiveSound(ActiveSoundItem* item);

#endif // GFX_ENABLE_SOUND

    Ptr<DrawingContext>     pDrawingAPI;

    // hitArea for this sprite
    Ptr<CharacterHandle>    pHitAreaHandle;
    // Pointer to sprite which has this sprite as hit area  
    Sprite*                 pHitAreaHolder; 

    // Transient holder for user renderer data set from Actionscript
//@REN
//     struct UserRendererData : public NewOverrideBase<StatMV_ActionScript_Mem>
//     {           
//         ASString                StringVal;
//         float                   FloatVal;
//         float                   MatrixVal[16];
//         Render::Renderer::UserData UserData;
// 
//         UserRendererData(ASStringManager* psm)
//             : StringVal(psm->CreateString((const char*)NULL)), FloatVal(0.0f) { }
//     };
//     UserRendererData*       pUserData;

    enum
    {
        Flags_UpdateFrame       = 0x1,
        Flags_HasLoopedPriv     = 0x2,
        Flags_Unloaded          = 0x4, // Set once Unload has been called.
        Flags_OnEventLoadCalled = 0x8,

        // This flag is set if sprite was loaded with loadMovie.
        // If so, _lockroot applies.
        Flags_LoadedSeparately  = 0x10,
        Flags_LockRoot          = 0x20,
        Flags_SpriteDef         = 0x40,
        Flags_OnLoadReqd        = 0x80
    };
    UByte                   Flags;

    // focus related stuff
    Bool3W                  FocusEnabled;
protected:
    void AssignRootNode(bool importFlag = false);
    void SetLoadedSeparatelyFlag(bool v = true) { Flags = (UByte)((v) ? (Flags | Flags_LoadedSeparately): (Flags & (~Flags_LoadedSeparately))); }

public:
    void SetUpdateFrame(bool v = true)      { Flags = (UByte)((v) ? (Flags | Flags_UpdateFrame)     : (Flags & (~Flags_UpdateFrame))); }
    void SetHasLoopedPriv(bool v = true)    { Flags = (UByte)((v) ? (Flags | Flags_HasLoopedPriv)   : (Flags & (~Flags_HasLoopedPriv))); }
    void SetOnEventLoadCalled(bool v = true){ Flags = (UByte)((v) ? (Flags | Flags_OnEventLoadCalled)   : (Flags & (~Flags_OnEventLoadCalled))); }
    void SetLockRoot(bool v = true)         { Flags = (UByte)((v) ? (Flags | Flags_LockRoot)        : (Flags & (~Flags_LockRoot))); }
    void SetLoadedSeparately(bool v = true);
    void SetOnLoadReqd(bool v = true)       { Flags = (UByte)((v) ? (Flags | Flags_OnLoadReqd)      : (Flags & (~Flags_OnLoadReqd))); }

    bool IsUpdateFrame() const              { return (Flags & Flags_UpdateFrame) != 0; }
    bool IsOnEventLoadCalled() const        { return (Flags & Flags_OnEventLoadCalled) != 0; }
    bool IsLockRoot() const                 { return (Flags & Flags_LockRoot) != 0; }
    bool IsLoadedSeparately() const         { return (Flags & Flags_LoadedSeparately) != 0; }
    bool IsOnLoadReqd() const               { return (Flags & Flags_OnLoadReqd) != 0; }

    void                CloneInternalData(const InteractiveObject* src);

    void                SetDirtyFlag();
    const Bool3W        GetFocusEnabledFlag() const { return FocusEnabled; }
    void                SetFocusEnabledFlag(bool b) { FocusEnabled = b; }
    void                UndefineFocusEnabledFlag()  { FocusEnabled = Bool3W(); }
public: 

    enum MouseState
    {
        UP = 0,
        DOWN,
        OVER
    };
    MouseState          MouseStatePriv;

public:
    // Both pdef (play-list) and pdefImpl (binding) pointers must be provided.
    Sprite(TimelineDef* pdef, MovieDefImpl* pdefImpl,
        ASMovieRootBase* pr, InteractiveObject* parent,
        ResourceId id, bool loadedSeparately = false);

    virtual ~Sprite();

    // *** implementation of DisplayObjectBase
    virtual void                AdvanceFrame(bool nextFrame, float framePos);
    virtual FontManager*        GetFontManager() const;
    virtual CharacterDef*       GetCharacterDef() const        { return pDef.GetPtr(); }
    // Bounds computation.
    // "transform" matrix describes the transform applied to parent and us,
    // including the object's matrix itself. This means that if transform is
    // identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
    virtual RectF               GetBounds(const Matrix &transform) const;       
    virtual DrawingContext*     GetDrawingContext();
    // Returns character bounds excluding stroke
    virtual RectF               GetRectBounds(const Matrix &transform) const;       
    // Return the topmost entity that the given point covers.  NULL if none.
    // Coords are in parent's frame.    
    virtual TopMostResult       GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);
    virtual InteractiveObject*  GetTopParent(bool ignoreLockRoot = false) const;
    virtual Render::TreeContainer* GetRenderContainer();
    virtual CharacterDef::CharacterDefType GetType() const { return CharacterDef::Sprite; }
    virtual bool                PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;
    virtual void                SetName(const ASString& name);

    // *** implementation of InteractiveObject
    // returns rectangle for focusrect, in local coords
    virtual RectF               GetFocusRect() const;
    virtual void                ExecuteFrame0Events();
    virtual void                ForceShutdown ();
    virtual bool                HasLooped() const { return (Flags & Flags_HasLoopedPriv) != 0; }
    // reports focusEnabled property state (used for Selection.setFocus())
    virtual bool                IsFocusEnabled() const;
    // Handle a button event.
    virtual bool                OnMouseEvent(const EventId& id);
    // Dispatch event Handler(s), if any.
    virtual bool                OnEvent(const EventId& id);
    virtual void                OnEventUnload();
    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed)
    virtual void                OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt);
    // Dispatch key event
    virtual bool                OnKeyEvent(const EventId& id, int* pkeyMask);
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    virtual bool                OnLosingKeyboardFocus(InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt);
    virtual bool                OnUnloading();
    // Do the events That (appear to) happen as the Character
    // loads.  frame1 tags and actions are Executed (even
    // before Advance() is called).  Then the onLoad event
    // is triggered.
    // Note: the default OnEventLoad logic has been moved to DefaultOnEventLoad
    // method. This method should be called by the AvmSprite at the right time:
    // looks like AS2 and AS3 perform default action in different moments of time.
    //virtual void                OnEventLoad();
    // Set state changed flags
    virtual void                SetStateChangeFlags(UInt8 flags);
//     virtual Sprite*             ToSprite() { return this; }
//     virtual const Sprite*       ToSprite() const { return this; }

    // *** implementation of DisplayObjContainer
    // Add an object to the display list.
    // CreateFrame specifies frame used as 'CreateFrame' for the new object (def: ~0 == CurrentFrame).
    virtual DisplayObjectBase*      AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame = SF_MAX_UINT,
        UInt32 addFlags = 0,
        CharacterCreateInfo* pcharCreateOverride = 0,
        InteractiveObject* origChar = 0);
    // Updates the transform properties of the object at the specified depth.   
    //void            MoveDisplayObject(const CharPosInfo &pos);
    virtual void                CreateAndReplaceDisplayObject
        (const CharPosInfo &pos, const ASString& name, DisplayObjectBase** newChar);
    // Remove the object at the specified depth.
    // If id != -1, then only remove the object at depth with matching id.
    //void                      RemoveDisplayObject(int depth, ResourceId id);
    // Remove *this* object from its parent.
    //void                      RemoveDisplayObject() { ::RemoveDisplayObject(); }
    // Movie Loading support
    virtual bool                ReplaceChildCharacterOnLoad
        (InteractiveObject *poldChar, InteractiveObject *pnewChar);
    virtual bool                ReplaceChildCharacter
        (InteractiveObject *poldChar, InteractiveObject *pnewChar);


    // *** native methods
    // Returns 0 if nothing to do
    // 1 - if need to add to optimized play list
    // -1 - if need to remove from optimized play list
    int                         CheckAdvanceStatus(bool playingNow);

    // This method contains default onLoad actions. Might be called
    // from AvmSprite::OnEventLoad to perform default actions.
    void                        DefaultOnEventLoad();

//    void                        Display(DisplayContext &context);

    // Execute the tags associated with the specified frame.
    // noRemove is specified if Removal tags are not to be executed (causing warning). Useful
    // for executing tags in the target frame after a backwards seek.
    // frame is 0-based
    void                        ExecuteFrameTags(unsigned frame);
    // Executes init action tags only for the frame.
    void                        ExecuteInitActionFrameTags(unsigned frame)
    {
        if (HasAvmObject())
            GetAvmSprite()->ExecuteInitActionFrameTags(frame);
    }
    void                        ExecuteImportedInitActions(MovieDef* psourceMovie);

    unsigned                    GetCurrentFrame() const { return CurrentFrame; }
    unsigned                    GetFrameCount() const   { return pDef->GetFrameCount(); } 
    UInt32                      GetBytesLoaded() const;
    virtual Sprite*             GetHitArea() const;
    virtual Sprite*             GetHitAreaHolder() const {return pHitAreaHolder;}
    CharacterHandle*            GetHitAreaHandle() const {return pHitAreaHandle;}
    int                         GetHitAreaIndex();
    unsigned                    GetLoadingFrame() const;
#ifdef GFX_AS_ENABLE_TEXTSNAPSHOT
    void                        GetTextSnapshot(StaticTextSnapshotData* pdata) const;
#endif
    // Returns 0-based frame #
    bool                        GetLabeledFrame
        (const char* label, unsigned* frameNumber, bool translateNumbers = 1)
    {
        return pDef->GetLabeledFrame(label, frameNumber, translateNumbers);
    }
//     virtual DisplayObject*      GetMask() const;
//     virtual DisplayObject*      GetMaskOwner() const;
    PlayState                   GetPlayState() const        
    { 
        return (IsMovieClip()) ? PlayStatePriv : State_Stopped; 
    }
    SpriteDef*                  GetSpriteDef() 
    { 
        return (Flags & Flags_SpriteDef) ? static_cast<SpriteDef*>(pDef.GetPtr()) : NULL; 
    }
    TimelineDef*                GetDef() const        { return pDef.GetPtr(); } // non-virtual
    InteractiveObject*          GetDisplayObjectByName(const ASString& name, bool caseSensitive = true)
    {
        return mDisplayList.GetDisplayObjectByName(name, caseSensitive);
    }
    // Set the sprite state at the specified frame number.
    // 0-based frame numbers!!  (in contrast to ActionScript and Flash MX)
    void                        GotoFrame(unsigned targetFrameNumber);
    // Look up the labeled frame, and jump to it.
    bool                        GotoLabeledFrame(const char* label, int offset = 0);

    // Increment CurrentFrame, and take care of looping.
    void                        IncrementFrameAndCheckForLoop();
    bool                        IsLevelMovie() const;
    int                         GetLevel() const;

    // Special event handler; ensures that unload is called on child items in timely manner.
    // Do the events that happen when there is XML data waiting
    // on the XML socket connection.
    virtual void                OnEventXmlsocketOnxml();    
    // Do the events That (appear to) happen on a specified interval.
    virtual void                OnEventIntervalTimer(); 
    // Do the events that happen as a MovieClip (swf 7 only) loads.
    virtual void                OnEventLoadProgress();
    virtual void                OnIntervalTimer(void *timer);

    void                        PropagateNoAdvanceGlobalFlag();
    void                        PropagateNoAdvanceLocalFlag();

    void                        Restart();

    void                        SetHitAreaHolder(Sprite* phitAreaHolder) {pHitAreaHolder = phitAreaHolder ;}
    void                        SetHitArea (Sprite* phitArea);
    void                        SetRendererString(ASString str);
    void                        SetRendererFloat(float num);
    void                        SetRendererMatrix(float *m, unsigned count = 16);
    void                        SetPause(bool pause);
    void                        SetPlayState(PlayState s);
    // this method is called only with threaded loading to set correct values
    // when the first has just been loaded
    void                        SetRootNodeLoadingStat(unsigned bytesLoaded, unsigned loadingFrame);
    virtual void                SetVisible(bool visible);

#ifndef SF_NO_IME_SUPPORT
    void                        SetIMECandidateListFont(FontResource* pfont);
#endif //#ifdef SF_NO_IME_SUPPORT

    bool                        SwapDepths(int depth1, int depth2, unsigned frame)
    {
        return mDisplayList.SwapDepths(this, depth1, depth2, frame);
    }
    // Combine the flags to avoid a conditional. It would be faster with a macro.
    SF_INLINE int               Transition(int a, int b) const
    {
        return (a << 2) | b;
    }

#if defined(GFX_ENABLE_SOUND) 
    // check if load sound is already playing
    bool              IsSoundPlaying(ASSoundIntf* psobj);
    // add an active sound
    void              AddActiveSound(Sound::SoundChannel* pchan, ASSoundIntf* psobj, SoundResource* pres);
    // add a streaming sound
    void              SetStreamingSound(Sound::SoundChannel* pchan);
    // return a streaming sound
    Sound::SoundChannel* GetStreamingSound() const;
    // Detach an AS sound object from this sprite
    void              DetachSoundObject(ASSoundIntf*);
    // Return a sound pan which is saved inside this object.
    // It is used just for displaying this value
    int               GetSoundPan();
    // Return a calculated sound volume.
    float             GetRealSoundPan();
    // Save the new sound volume and propagate it to the child movieclip
    void              SetSoundPan(int volume);
    // Return a sound volume which is saved inside this object.
    // It is used just for displaying this value
    int               GetSoundVolume();
    // Return a calculated sound volume.
    float             GetRealSoundVolume();
    // Save the new sound volume and propagate it to the child movieclip
    void              SetSoundVolume(int volume);
    // Stop all active sound in this movieclip and all child clips
    virtual void      StopActiveSounds();
    // Stop all active sound in this movieclip and all child clips
    //void              StopActiveSounds(const String& name);
    // Stop all with the give resource in this sprite with the given linkage name
    void              StopActiveSounds(SoundResource* pres);
    // update the volume of all sound started by this sprite
    void              UpdateActiveSoundVolume();
    // update the pan of all sound started by this sprite
    void              UpdateActiveSoundPan();
    // release an active sound which is attached to this sprite. It is used to pass
    // active sounds between movieclips with MovieClip.attachAudio method
    ActiveSoundItem*  ReleaseActiveSound(Sound::SoundChannel*);
    // attach an active sound released by ReleaseActiveSound mehtod.
    void              AttachActiveSound(ActiveSoundItem*);
    // register an AS sound object which is attached to this sprite
    void              AttachSoundObject(ASSoundIntf*);
    // return an active sound playing position
    float             GetActiveSoundPosition(ASSoundIntf* psobj);
    // stop streaming sound
    void              StopStreamSound();
    // Go through the list of active sounds and remove sounds that are not 
    // playing anymore (will call Sound.onSoundComplete event handler)
    // this method should be called from AdvenceFrame.
    void              CheckActiveSounds();
#endif // GFX_ENABLE_SOUND

    virtual void UpdateViewAndPerspective();
    virtual bool Is3D() const;   // returns true if sprite contains 3D characters

private:
    SF_INLINE MemoryHeap*       GetMovieHeap() const; // impl in cpp
    SF_INLINE AvmSpriteBase*    GetAvmSprite()
    {
        return GetAvmObjImpl()->ToAvmSpriteBase();
    }
    SF_INLINE const AvmSpriteBase*  GetAvmSprite() const
    {
        return const_cast<const AvmSpriteBase*>(const_cast<AvmDisplayObjBase*>(GetAvmObjImpl())->ToAvmSpriteBase());
    }
};

SF_INLINE
Sprite* DisplayObjectBase::CharToSprite()
{
    return IsSprite() ? static_cast<Sprite*>(this) : 0;
}

SF_INLINE
Sprite* DisplayObjectBase::CharToSprite_Unsafe()
{
    SF_ASSERT(IsSprite());
    return static_cast<Sprite*>(this);
}

SF_INLINE
const Sprite* DisplayObjectBase::CharToSprite() const 
{
    return IsSprite() ? static_cast<const Sprite*>(this) : 0;
}

SF_INLINE
const Sprite* DisplayObjectBase::CharToSprite_Unsafe() const 
{
    SF_ASSERT(IsSprite());
    return static_cast<const Sprite*>(this);
}


}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_SPRITE_H

