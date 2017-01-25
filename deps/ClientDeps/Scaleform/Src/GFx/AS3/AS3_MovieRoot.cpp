/**********************************************************************

Filename    :   GFx/AS3/MovieRoot.cpp
Content     :   Implementation of AS3 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov, Prasad Silva

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/AS3_AbcDataBuffer.h"
#include "GFx/GFx_ASString.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/AS3/Obj/AS3_Obj_Function.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObjectContainer.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Loader.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_LoaderInfo.h"
#include "GFx/AS3/AS3_AsString.h"
#include "Kernel/SF_MsgFormat.h"
#include "GFx/AS3/Obj/AS3_Obj_Array.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLRequest.h"
#include "GFx/AS3/Obj/Net/AS3_Obj_Net_URLLoader.h"
#include "GFx/AS3/AS3_AvmBitmap.h"

namespace Scaleform { namespace GFx { namespace AS3 {
///

ASVM::ASVM(MovieRoot* pmr, FlashUI& _ui, FileLoader& loader, AS3::StringManager& sm, ASRefCountCollector& gc)
: VM(_ui, loader, sm, gc), pMovieRoot(pmr), ExtensionsEnabled(false)
{
    GraphicsClass = GetClass("flash.display.Graphics");
    TransformClass = GetClass("flash.geom.Transform");
	MatrixClass = GetClass("flash.geom.Matrix");
	ColorTransformClass = GetClass("flash.geom.ColorTransform");
    EventClass = GetClass("flash.events.Event");
    MouseEventClass = GetClass("flash.events.MouseEvent");
    KeyboardEventClass = GetClass("flash.events.KeyboardEvent");
    FocusEventClass = GetClass("flash.events.FocusEvent");
    TextEventClass = GetClass("flash.events.TextEvent");
    
    MouseEventExClass = GetClass("scaleform.gfx.MouseEventEx");
    KeyboardEventExClass = GetClass("scaleform.gfx.KeyboardEventEx");
    FocusEventExClass = GetClass("scaleform.gfx.FocusEventEx");
    TextEventExClass = GetClass("scaleform.gfx.TextEventEx");

    TimerEventClass = GetClass("flash.events.TimerEvent");
    ProgressEventClass = GetClass("flash.events.ProgressEvent");
    PointClass = GetClass("flash.geom.Point");
    RectangleClass = GetClass("flash.geom.Rectangle");
    TextFormatClass = GetClass("flash.text.TextFormat");
    EventDispatcherClass = GetClass("flash.events.EventDispatcher");


    SF_ASSERT(GraphicsClass);
	SF_ASSERT(TransformClass);
	SF_ASSERT(MatrixClass);
	SF_ASSERT(ColorTransformClass);
    SF_ASSERT(EventClass);
    SF_ASSERT(MouseEventClass);
    SF_ASSERT(KeyboardEventClass);
    SF_ASSERT(FocusEventClass);
    SF_ASSERT(TextEventClass);

    SF_ASSERT(MouseEventExClass);
    SF_ASSERT(KeyboardEventExClass);
    SF_ASSERT(FocusEventExClass);
    SF_ASSERT(TextEventExClass);

    SF_ASSERT(TimerEventClass);
    SF_ASSERT(ProgressEventClass);
    SF_ASSERT(PointClass);
    SF_ASSERT(RectangleClass);
    SF_ASSERT(TextFormatClass);
    SF_ASSERT(EventDispatcherClass);
}

AMP::ViewStats* ASVM::GetAdvanceStats() const
{
#ifdef SF_AMP_SERVER
    return pMovieRoot->GetMovieImpl()->AdvanceStats;
#else
    return NULL;
#endif
}


bool ASVM::_constructInstance(SPtr<Object> &pobj, Object* classObj,
                              unsigned argc, const Value* argv)
{
    Value v;
    classObj->Construct(v, argc, argv, true);

    if (!IsException() && !v.IsNullOrUndefined())
    {
        pobj = v.GetObject();
        return true;
    }        
    pobj = NULL;
    if (IsException())
        IgnoreException();
    return false;
}

// returns a moviedef for the last abc in call stack
MovieDefImpl* ASVM::GetResourceMovieDef(Instances::Object* instance) const
{
    VMAbcFile* vmabc = instance->GetTraits().GetFilePtr();
    if (vmabc)
    {
        const AbcFileWithMovieDef& abc = 
            static_cast<const AbcFileWithMovieDef&>(vmabc->GetAbcFile());
        // null is allowed!
//        SF_ASSERT(abc.pDefImpl);
        return abc.pDefImpl;
    }
    else
    {
        // call stack is empty - get the main moviedef
        return pMovieRoot->GetMovieDefImpl();
    }
}

//////////////////////////////////////////////////////////////////////////
MovieRoot::MovieRoot(MemoryContextImpl* memContext, MovieImpl* pmovie, ASSupport* pas) 
: 
    ASMovieRootBase(pas),
    MemContext(memContext),
    NumAdvancesSinceCollection(0),
    LastCollectionFrame(0),
    ActionQueue(memContext->Heap),
    BuiltinsMgr(pmovie->GetStringManager()),
    StageInvalidated(false)
{
    CheckClipsCnt = 0;    
    ASFramesToExecute = 0;
    pMovieImpl = pmovie;
    pMovieImpl->SetASMovieRoot(this);
    CreateObjectInterface(pmovie);
    pmovie->SetKeyboardListener(this);
    pInvokeAliases     = NULL;

    pmovie->SetDisableFocusAutoRelease(true);
    pmovie->SetDisableFocusRolloverEvent(true);
}

MovieRoot::~MovieRoot()
{
}

bool MovieRoot::Init(MovieDefImpl* pmovieDef) 
{ 
    // do we need to create VM always? SWF files with no AS are possible.
    // But there are some problems with handling input or focus w/o VM:
    // we either need to check for presence of AS3 objects/VM always or
    // to create them on fly. @TODO
    CheckAvm();

    // Create stage first
    CreateStage(pmovieDef);

    pMovieImpl->SetLevelMovie(0, pStage);

    // In case permanent variables were assigned.. check them.
    ResolveStickyVariables(pStage);
    return true;
}

void  MovieRoot::Restart()
{
}

bool MovieRoot::CheckAvm()
{
    if (!pAVM)
    {
        // turn on VM tracing
        ActionControl* pac = pMovieImpl->GetActionControl();
        if (pac != NULL)
        {
            SetNeedToCheckOpCode((pac->GetActionFlags() & ActionControl::Action_Verbose) != 0);
        }
        else
        {
            SetNeedToCheckOpCode(false);
        }
        SetState(FlashUI::sStep);

        pAVM = SF_HEAP_NEW(GetMovieHeap()) ASVM(this, *this, *this, *GetStringManager(), *(MemContext->ASGC));
        pAVM->ExecuteCode();
    }
    return (pAVM.GetPtr() != NULL);
}

bool MovieRoot::ExecuteAbc(const AbcDataBuffer* pabc, MovieDefImpl* pdefImpl)
{
    // find already existing AbcFile first
    for (UPInt i = 0; i < AbcFiles.GetSize(); ++i)
    {
        ASVM::AbcFileWithMovieDef* file = 
            static_cast<ASVM::AbcFileWithMovieDef*>(AbcFiles[i].GetPtr());
        if (file->pAbcData == pabc)
            return true; // it is already loaded!
    }
    AutoPtr<Abc::Reader> pReader = SF_HEAP_NEW(GetMovieHeap()) Abc::Reader(pabc->Data, pabc->DataSize);
    AutoPtr<Abc::File>  pAbcFile = SF_HEAP_NEW(GetMovieHeap()) ASVM::AbcFileWithMovieDef(pdefImpl, pabc);

    pAbcFile->SetSource(pdefImpl->GetFileURL());
    pAbcFile->SetDataSize(pabc->DataSize);
    SF_AMP_CODE(pAbcFile->SetSwdHandle(pabc->SwdHandle);)
    SF_AMP_CODE(pAbcFile->SetSwfFileOffset(pabc->SwfOffset);)

    bool result = pReader->Read(*pAbcFile);
    if (result)
    {
        MovieRoot::CheckAvm();
        AbcFiles.PushBack(pAbcFile); //@TODO: more intellectual way of storing, 
        pAVM->LoadFile(*pAbcFile.GetPtr()).DoNotCheck();
        if (pAVM->IsException())
        {
            pAVM->OutputAndIgnoreException();
            result = false;
        }

        // what if swf was unloaded and then loaded again? duplicated abc file?
    }
    return result;
}

void MovieRoot::RemoveAbc(MovieDefImpl* )
{
    //@TODO?
    // should it also remove a ptr to MovieDefImpl from LoadedMovieDefs array?
//     for (UPInt i = 0; i < AbcFiles.GetSize(); ++i)
//     {
//         ASVM::AbcFileWithMovieDef* file = 
//             static_cast<ASVM::AbcFileWithMovieDef*>(AbcFiles[i].GetPtr());
//         if (file->pDefImpl == def)
//             file->pDefImpl = NULL; //???
//     }
}

// FlashUI methods
void MovieRoot::Output(FlashUI::OutputMessageType type, const char* msg)
{
    Log* log = pMovieImpl->GetCachedLog();
    if (!log)
        return;

    LogMessageId logId;

    switch(type)
    {
    case FlashUI::Output_Action:    logId = Log_Action; break;
    case FlashUI::Output_Error:     logId = Log_ScriptError; break;
    case FlashUI::Output_Warning:   logId = Log_ScriptWarning; break;
    case FlashUI::Output_Message:
    default:
        logId = Log_Message;
    }
    char    buffStr[2000];
    size_t  len = SFstrlen(msg);
    size_t  szToCopy = (len < sizeof(buffStr)) ? len : sizeof(buffStr) - 1;

    // replace '\r' by '\n'
    SFstrncpy(buffStr, sizeof(buffStr), msg, szToCopy);
    buffStr[szToCopy] = '\0';
    //     for (char* p = buffStr; *p != 0; ++p)
    //     {
    //         if (*p == '\r')
    //             *p = '\n';
    //     }
    if (len < sizeof(buffStr))
        log->LogMessageById(logId, "%s", buffStr);
    else
        log->LogMessageById(logId, "%s ...<truncated>", buffStr);
}


bool MovieRoot::OnOpCode(const Abc::TOpCode& code, Abc::TCodeOffset offset, Abc::Code::OpCode opcode)
{   
    SF_UNUSED3(code, offset, opcode);
#ifdef GFX_AS3_VERBOSE
    // Trace VM state and opcode.
    AS3::TraceVM(*pAVM, "\t");
    AS3::TraceOpCode(*this, *pAVM, offset);
#endif
    return false;
}

#ifdef USE_WORDCODE
bool MovieRoot::OnOpCode(const ArrayLH_POD<UInt8>& code, Abc::TCodeOffset offset, Abc::Code::OpCode opcode)
{
    SF_UNUSED3(code, offset, opcode);
    return false;
}
#endif

UPInt MovieRoot::GetSize() const
{
    return AbcFiles.GetSize();
}

const AS3::Abc::File& MovieRoot::GetFile(UPInt n)
{
    return *AbcFiles[n];
}

const AS3::Abc::File* MovieRoot::GetFile(const ASString&)
{
    return NULL;
}

MovieRoot::MouseState::DoubleClickInfo& MovieRoot::MouseState::GetDoubleClickInfo(unsigned buttonMask)
{
    unsigned idx; 
    if (buttonMask > (1<<2))
        idx = (unsigned)logf((float)buttonMask);
    else
    {
        static const unsigned ll[] = { 0, 1, 2, 0, 3 };
        idx = ll[buttonMask];
    }
    SF_ASSERT(idx < sizeof(DblClick)/sizeof(DblClick[0]));
    return DblClick[idx];
}

void    MovieRoot::NotifyOnResize()
{
	QueueEventChain(EventId::Event_Resize);
}

void    MovieRoot::GenerateMouseEvents(unsigned mouseIndex)
{
    GFx::MouseState& ms = *pMovieImpl->GetMouseState(mouseIndex);
    SF_ASSERT(&ms);
    Ptr<InteractiveObject>  ActiveEntity  = ms.GetActiveEntity();
    Ptr<InteractiveObject>  TopmostEntity = ms.GetTopmostEntity();

    Ptr<InteractiveObject>  PrevActiveEntity = ActiveEntity;

    KeyboardState* keybSt = GetMovieImpl()->GetKeyboardState(mouseIndex);

    // If topmostEntity is NULL then mouse cursor is above the stage. Flash generates
    // mouse events for stage as a target is this case. 
    InteractiveObject*      activeEntitySafe = 
        (ActiveEntity) ? ActiveEntity.GetPtr() : (InteractiveObject*)pStage.GetPtr();
    InteractiveObject*      
        topmostEntitySafe = (TopmostEntity) ? TopmostEntity.GetPtr() : (InteractiveObject*)pStage.GetPtr();
    SF_ASSERT(activeEntitySafe);
    SF_ASSERT(topmostEntitySafe);
    if (ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left)
    {
        // Mouse button was down.
        // Handle onDragOut, onDragOver
        if (!ms.IsMouseInsideEntityLast())
        {
            if (TopmostEntity == ActiveEntity)
            {
                // onDragOver
                if (ActiveEntity)  
                    ActiveEntity->OnMouseEvent(ButtonEventId
                    (EventId::Event_DragOver, mouseIndex, ActiveEntity->IncrementRollOverCnt()));

                ms.SetMouseInsideEntityLast(true);
            }
        }
        else
        {
            // MouseInsideEntityLast == true
            if (TopmostEntity != ActiveEntity)
            {
                // onDragOut
                if (ActiveEntity)
                {
                    ActiveEntity->OnMouseEvent(ButtonEventId
                        (EventId::Event_DragOut, mouseIndex, ActiveEntity->DecrementRollOverCnt()));
                }
                ms.SetMouseInsideEntityLast(false);
            }
        }

#if 0  
        // Handle trackAsMenu dragOver
        if (!ActiveEntity
            || ActiveEntity->GetTrackAsMenu())
        {
            if (TopmostEntity
                && TopmostEntity != ActiveEntity
                && TopmostEntity->GetTrackAsMenu())
            {
                // Transfer to topmost entity, dragOver
                ActiveEntity = TopmostEntity;
                ActiveEntity->OnMouseEvent(ButtonEventId(EventId::Event_DragOver, 
                    mouseIndex, 
                    0/*ActiveEntity->IncrementRollOverCnt()*/));
                ms.SetMouseInsideEntityLast(true);
            }
        }
#endif

        // Handle onRelease, onReleaseOutside
        if ((ms.GetButtonsState() & GFx::MouseState::MouseButton_Left) == 0)
        {
            MouseState::DoubleClickInfo& dci = mMouseState[mouseIndex].GetDoubleClickInfo(MouseState::MouseButton_Left);
            if (ms.GetMouseButtonDownEntity())
                ms.GetMouseButtonDownEntity()->OnMouseEvent(ButtonEventId(EventId::Event_ReleaseOutside, mouseIndex));
//            if (ms.IsMouseInsideEntityLast())
            {
                topmostEntitySafe->OnMouseEvent(ButtonEventId(EventId::Event_MouseUp, mouseIndex));
                bool dblClick = false;
                UInt32 tm = (UInt32)(Timer::GetTicks() / 1000);
                if (dci.PrevClickTime != 0)
                {
                    PointF pt = ms.GetLastPosition();
                    int ax = (int)TwipsToPixels(pt.x);
                    int ay = (int)TwipsToPixels(pt.y);
                    if (tm <= dci.PrevClickTime + GFX_DOUBLE_CLICK_TIME_MS && 
                        (int)TwipsToPixels(dci.PrevPosition.x) == ax && 
                        (int)TwipsToPixels(dci.PrevPosition.y) == ay)
                    {
                        dblClick = true;
                        //printf("DOUBLE CLICK!\n");
                    }
                }
                if (dblClick && ToAvmInteractiveObj(topmostEntitySafe)->IsDoubleClickEnabled())
                {
                    ButtonEventId evt(EventId::Event_DoubleClick, mouseIndex);
                    evt.KeysState = keybSt->GetKeyModifiers();
                    topmostEntitySafe->OnMouseEvent(evt);
                }
                // 10/19/2010, NFM: Only fire Click if current ActiveEntity == ActiveEntity when the last MouseDown occurred.
                else if (ActiveEntity == ms.GetMouseButtonDownEntity()) 
                {
                    ButtonEventId evt(EventId::Event_Click, mouseIndex);
                    evt.KeysState = keybSt->GetKeyModifiers();
                    topmostEntitySafe->OnMouseEvent(evt);                    
                }
                dci.PrevClickTime = tm;
                dci.PrevPosition  = ms.GetLastPosition();
            }
//             else
//             {
//                 if (!activeEntitySafe->GetTrackAsMenu())
//                     activeEntitySafe->OnMouseEvent(ButtonEventId(EventId::Event_ReleaseOutside, mouseIndex));
//                 // Clear, to prevent Event_RollOut event below.
//                 dci.PrevClickTime = 0;
//             }
            ms.SetMouseButtonDownEntity(NULL);
        }
    }

    bool trackAsMenu = (ActiveEntity) ? ActiveEntity->GetTrackAsMenu() : false;
    if (ms.GetWheelDelta() != 0)
    {
        ButtonEventId evt(EventId::Event_MouseWheel, mouseIndex);
        evt.KeysState       = keybSt->GetKeyModifiers();
        evt.MouseWheelDelta = (SInt8)ms.GetWheelDelta();
        topmostEntitySafe->OnMouseEvent(evt);
    }

    if (ms.IsMouseMoved())
    {
        ButtonEventId evt(EventId::Event_MouseMove, mouseIndex);
        evt.KeysState       = keybSt->GetKeyModifiers();
        evt.MouseWheelDelta = (SInt8)ms.GetWheelDelta();
        topmostEntitySafe->OnMouseEvent(evt);
    }
    if (topmostEntitySafe->GetTrackAsMenu())
        trackAsMenu = true;

    //printf("%d %x %x %p %p\n", mouseIndex, ms.GetButtonsState(), ms.GetPrevButtonsState(), TopmostEntity.GetPtr(), ActiveEntity.GetPtr());

    // 10/19/2010, NFM: Commented out LeftMouseButton || TrackAsMenu check fix a bug where 
    //                  ROLL_OVER/MOUSE_OVER/MOUSE_OUT/ROLL_OUT were not fired when the left Mouse button was pressed.
    // if ((ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left) == 0 || 
    //     trackAsMenu)
    //{
        // Mouse button was up.
        //printf("%p %p\n", TopmostEntity.GetPtr(), ActiveEntity.GetPtr());

        // New active entity is whatever is below the mouse right now.
    if (TopmostEntity != ActiveEntity)
    {
        // rollOut
        UPInt rosSz = mMouseState[mouseIndex].RolloverStack.GetSize();
        SPInt i;
        for (i = (SPInt)rosSz - 1; i >= 0; --i)
        {
            // check the stack of already "rollovered" characters.
            // If the new character is not the child of the one in the stack - 
            // fire rollOut and remove from stack. Stop, when the element char
            // actually is the parent of ActiveEntity.
            if (!TopmostEntity || (mMouseState[mouseIndex].RolloverStack[i] != TopmostEntity &&
                !mMouseState[mouseIndex].RolloverStack[i]->IsAncestor(TopmostEntity)))
            {
                ButtonEventId evt
                    (EventId::Event_RollOut, mouseIndex, mMouseState[mouseIndex].RolloverStack[i]->DecrementRollOverCnt());
                evt.KeysState = keybSt->GetKeyModifiers();
                mMouseState[mouseIndex].RolloverStack[i]->OnMouseEvent(evt);              
            }
            else
                break;
        }
        mMouseState[mouseIndex].RolloverStack.PopBack(rosSz - i - 1);

        if (ActiveEntity)
        {
            // mouseOut
            // check if the new entity is the child
            mMouseState[mouseIndex].LastMouseOverObj = ActiveEntity;
            ButtonEventId evt
                (EventId::Event_MouseOut, mouseIndex, ToAvmInteractiveObj(ActiveEntity)->DecrementMouseOverCnt());
            evt.KeysState = keybSt->GetKeyModifiers();
            ActiveEntity->OnMouseEvent(evt);   
        }

        ActiveEntity = TopmostEntity;

        if (ActiveEntity)
        {
            // rollOver
            bool fireRollover = true;
            UPInt rosSz = mMouseState[mouseIndex].RolloverStack.GetSize();
            if (rosSz)
            {
                for (SPInt i = (SPInt)rosSz - 1; i >= 0; --i)
                {
                    if (mMouseState[mouseIndex].RolloverStack[i] == ActiveEntity)
                    {
                        fireRollover = false;
                        break;
                    }
                }
            }
            if (fireRollover)
            {
                // Seems to me rollover/rollout are fired for each parent up to the "root" (not for stage)
                // This happens anyway on capture and bubble stage, but Flash fires rollover/out
                // for each parent separately (having each parent as a "target").
                Ptr<InteractiveObject> cur = ActiveEntity;
                InteractiveObject* stage = GetStage();
                UPInt idx = mMouseState[mouseIndex].RolloverStack.GetSize();
                for (; cur && cur != stage; cur = cur->GetParent())
                {
                    if (idx != 0)
                    {
                        // check if we already have the the char in the stack; 
                        // if so, stop here.
                        if (mMouseState[mouseIndex].RolloverStack[idx - 1] == cur)
                            break;
                    }
                    ButtonEventId evt
                        (EventId::Event_RollOver, mouseIndex, cur->IncrementRollOverCnt());
                    evt.KeysState = keybSt->GetKeyModifiers();
                    cur->OnMouseEvent(evt);
                    mMouseState[mouseIndex].RolloverStack.InsertAt(idx, cur);
                }
            }
            
            // mouseOver
            ButtonEventId evt
                (EventId::Event_MouseOver, mouseIndex, ToAvmInteractiveObj(ActiveEntity)->IncrementMouseOverCnt());
            evt.KeysState = keybSt->GetKeyModifiers();
            ActiveEntity->OnMouseEvent(evt);
            mMouseState[mouseIndex].LastMouseOverObj = ActiveEntity;
        }

        ms.SetMouseInsideEntityLast(true);
    }

    // mouse button press
    if ((ms.GetPrevButtonsState() & GFx::MouseState::MouseButton_Left) == 0)
    {
        if (ms.GetButtonsState() & GFx::MouseState::MouseButton_Left)
        {
            // onPress
            ButtonEventId evt(EventId::Event_MouseDown, mouseIndex);
            evt.KeysState = keybSt->GetKeyModifiers();
            activeEntitySafe->OnMouseEvent(evt);
            // 10/19/2010, NFM: Store this entity so we can check against it for a Click event.
            ms.SetMouseButtonDownEntity(ActiveEntity);

            ms.SetMouseInsideEntityLast(true);
        }
    }

    // } NFM: From old if (!MouseLeftButton || TrackAsMenu) statement.

    // Write The (possibly modified) Ptr copies back
    // into the state struct.
    ms.SetActiveEntity(ActiveEntity);
}

SPtr<Instances::Event> MovieRoot::CreateEventObject(const ASString& type, bool bubbles, bool cancelable)
{
    CheckAvm();
    Classes::EventDispatcher& evtDispClass = 
        static_cast<Classes::EventDispatcher&>(*GetAVM()->EventDispatcherClass.Get());
    return evtDispClass.CreateEventObject(type, bubbles, cancelable);
}

void    MovieRoot::DoActions()
{
    SF_AMP_SCOPE_TIMER_ID(pMovieImpl->AdvanceStats, "MovieRoot::DoActions", Amp_Native_Function_Id_DoActions);
    ExecuteCtors();
    ExecuteActionQueues();
}

void    MovieRoot::ExecuteCtors()
{
    // execute any scheduled AS stack frames (most likely AS ctors)
    if (ASFramesToExecute > 0)
    {
        pAVM->ExecuteCode(ASFramesToExecute);  
        if (pAVM->IsException())
            pAVM->IgnoreException();

        ASFramesToExecute = 0;
    }
}

void    MovieRoot::ExecuteActionQueue(ActionLevel lvl)
{
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueIterator iter(lvl, &ActionQueue);
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void    MovieRoot::ExecuteActionQueue(ActionLevel lvl, void* rootEntry)
{
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    MovieRoot::ActionQueueIterator iter(lvl, &ActionQueue, 
        static_cast<ActionEntry*>(rootEntry));
    const ActionEntry* paction;
    while((paction = iter.getNext()) != NULL)
    {
        paction->Execute(this);
    }
}

void    MovieRoot::ExecuteActionQueues()
{
    //ForceCollect();
    for (unsigned i = 0; i < AL_Count_; ++i)
    {   
        ExecuteActionQueue(ActionLevel(i));
    }
    //ForceCollect();
}

void    MovieRoot::QueueFrameActions()
{
    pStage->QueueFrameActions();
}

void MovieRoot::ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType) 
{
    // extensions disabled; just call static method GASMouseCtorFunction::SetCursorType
    SF_ASSERT(pMovieImpl);

    if (pMovieImpl->pUserEventHandler)
    {
        pMovieImpl->pUserEventHandler->HandleEvent(pMovieImpl, 
            MouseCursorEvent((MouseCursorEvent::CursorShapeType)newCursorType, mouseIdx));
    }
}

bool MovieRoot::NotifyOnFocusChange(InteractiveObject* curFocused, 
                                    InteractiveObject* toBeFocused, 
                                    unsigned controllerIdx, 
                                    FocusMovedType fmt,
                                    ProcessFocusKeyInfo* pfocusKeyInfo)
{
    // call OnFocusChange for stage if there is no currently focused object
    AvmInteractiveObj* obj = (curFocused) ? ToAvmInteractiveObj(curFocused) : ToAvmInteractiveObj(pStage);
    return obj->OnFocusChange(toBeFocused, controllerIdx, fmt, pfocusKeyInfo);
}

void MovieRoot::NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt) 
{
    // do we need to queue it like in AS2? (!AB)
    GetMovieImpl()->TransferFocus(ch, controllerIdx, fmt);
}

void MovieRoot::NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, unsigned controllerIdx) 
{ 
    SF_UNUSED3(curFocused, pNewFocus, controllerIdx); 
}

Sprite* MovieRoot::CreateMovieClip(TimelineDef* pdef, 
                                MovieDefImpl* pdefImpl,
                                InteractiveObject* parent,
                                ResourceId id, 
                                bool loadedSeparately)
{
    DisplayObjectBase* pdo = GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), pdef, parent, id, pdefImpl, CharacterDef::Sprite);
    SF_ASSERT(pdo->IsInteractiveObject() && pdo->IsSprite());
    Sprite* pspr = pdo->CharToSprite_Unsafe();
    pspr->SetLoadedSeparately(loadedSeparately);
    pspr->SetMovieClipFlag();
    return pspr;
}

Sprite* MovieRoot::CreateSprite(TimelineDef* pdef, 
                                MovieDefImpl* pdefImpl,
                                InteractiveObject* parent,
                                ResourceId id, 
                                bool loadedSeparately)
{
    DisplayObjectBase* pdo = GetASSupport()->CreateCharacterInstance
        (GetMovieImpl(), pdef, parent, id, pdefImpl, CharacterDef::EmptySprite);
    SF_ASSERT(pdo->IsInteractiveObject() && pdo->IsSprite());
    Sprite* pspr = pdo->CharToSprite_Unsafe();
    pspr->SetLoadedSeparately(loadedSeparately);
    return pspr;
}

AvmStage* MovieRoot::CreateStage(MovieDefImpl* pdefImpl)
{
    SF_ASSERT(!pStage);
    UByte* pm = (UByte*) SF_HEAP_ALLOC(GetMovieHeap(), 
        ((sizeof(Stage) + 3) & ~3) + sizeof(AS3::AvmStage),
        StatMV_ActionScript_Mem);
    pStage = *new (pm) Stage(pdefImpl, this, NULL, ResourceId(ResourceId::InvalidId));
    AvmStage* avmStage = new (pm + ((sizeof(Stage) + 3) & ~3)) AvmStage(pStage);
    InsertEmptyAction(AL_Highest)->SetAction(pStage, EventId::Event_Load);
    return avmStage;
}

void MovieRoot::AdvanceFrame(bool nextFrame)
{
    if (IsStageInvalidated())
    {
        // fire RENDER events
        QueueEventChain(EventId::Event_Render);
        ValidateStage();
        DoActions(); // execute queued up events right here
    }

    if (nextFrame)
    {
        // if collection was scheduled by System.gc - collect
        if (MemContext->ASGC)
        {
            MemContext->ASGC->CollectIfScheduled();
#ifdef GFX_AS_ENABLE_GC
            MemContext->ASGC->AdvanceFrame(&NumAdvancesSinceCollection, &LastCollectionFrame);
#endif // SF_NO_GC
        }
    }
}

void MovieRoot::ClearDisplayList()
{
    // It is important to destroy the sprite before the global context,
    // so that is is not used from OnEvent(unload) in sprite destructor
    // NOTE: We clear the list here first because users can store pointers in _global,
    // which would cause pMovie assignment to not release it early (avoid "aeon.swf" crash).

    GetMainMovie()->ClearDisplayList();
    GetMainMovie()->OnEventUnload();
    GetMainMovie()->ForceShutdown();

    /*for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
        static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr())->ClearDisplayList();
    for (i = pMovieImpl->MovieLevels.GetSize(); i > 0; i--)
    {
        Sprite* pspr = static_cast<Sprite*>(pMovieImpl->MovieLevels[i-1].pSprite.GetPtr());
        pspr->OnEventUnload();
        pspr->ForceShutdown();
    }*/
    // Release all refs.
    pMovieImpl->MovieLevels.Clear();
}


// forces garbage collection (if GC is enabled)
void MovieRoot::ForceCollect()
{
    MemContext->ASGC->ForceCollect();
}

// forces emergency garbage collection (if GC is enabled). This method is called
// when heap is overflown. 
void MovieRoot::ForceEmergencyCollect()
{
    MemContext->ASGC->ForceEmergencyCollect();
}

void MovieRoot::Shutdown()
{
    // clean up sprites that we hold in NewSprites hash
    for (NewSpritesHashType::Iterator it = NewSprites.Begin(); it != NewSprites.End(); ++it)
    {
        Ptr<DisplayObjContainer> spr = it->Second;
        // we can remove unreferenced, stopped, unlinked sprite from the list
        spr->RemoveFromPlayList();
        it.RemoveAlt(spr);
    }
    NewSprites.Clear();
    GetMovieImpl()->ClearPlayList();

    ActionQueue.Clear();
    pStage = NULL;
    ForceCollect();

    // Reset invoke aliases set by ExternalInterface.addCallback
    delete pInvokeAliases; pInvokeAliases = NULL;
    AbcFiles.Clear();

    // before releasing AVM we need to release all Values.
    ExternalIntfRetVal.SetUndefined();

    pAVM = NULL;
}

void MovieRoot::OnKeyDown(InteractiveObject *pmovie, UInt32 code, UByte ascii, 
                          UInt32 wcharCode, UInt8 keyboardIndex) 
{
    SF_UNUSED2(pmovie, keyboardIndex);

    Ptr<InteractiveObject> focused = pMovieImpl->GetFocusedCharacter(keyboardIndex);
    Instances::DisplayObject* as3dobj = (focused) ? 
        ToAvmInteractiveObj(focused)->GetAS3Obj() : ToAvmDisplayObjContainer(pStage)->GetAS3Obj();
    if (as3dobj)
        as3dobj->Dispatch(EventId(EventId::Event_KeyDown, code, ascii, wcharCode, keyboardIndex), focused);
}       
void MovieRoot::OnKeyUp(InteractiveObject *pmovie, UInt32 code, UByte ascii, 
                        UInt32 wcharCode, UInt8 keyboardIndex) 
{
    SF_UNUSED2(pmovie, keyboardIndex);

    Ptr<InteractiveObject> focused = pMovieImpl->GetFocusedCharacter(keyboardIndex);
    Instances::DisplayObject* as3dobj = (focused) ? 
        ToAvmInteractiveObj(focused)->GetAS3Obj() : ToAvmDisplayObjContainer(pStage)->GetAS3Obj();
    if (as3dobj)
        as3dobj->Dispatch(EventId(EventId::Event_KeyUp, code, ascii, wcharCode, keyboardIndex), focused);
}
void MovieRoot::Update(UInt32 code, UByte ascii, UInt32 wcharCode, UInt8 keyboardIndex) 
{
    SF_UNUSED4(code, ascii, wcharCode, keyboardIndex);
}

const ASString& GetEventString(const StringManager& mgr, const EventId& evt)
{
    switch(evt.Id)
    {
    case EventId::Event_EnterFrame:
        return mgr.GetBuiltin(AS3Builtin_enterFrame);
    case EventId::Event_Press:
        return mgr.GetBuiltin(AS3Builtin_mouseDown);
    case EventId::Event_Release:
        return mgr.GetBuiltin(AS3Builtin_mouseUp);
//     case EventId::Event_MouseDown:
//         return mgr.GetBuiltin(AS3Builtin_mouseDown);
//     case EventId::Event_MouseUp:
//         return mgr.GetBuiltin(AS3Builtin_mouseUp);
    }
    return mgr.GetBuiltin(AS3Builtin_empty_);
}

void MovieRoot::OnMovieFocus(bool set)
{
    if (set)
        mEventChains.Dispatch(EventId::Event_Activate);
    else
        mEventChains.Dispatch(EventId::Event_Deactivate);
    //ExecuteActivateDeactivateQueue((set) ? EventId::Event_Activate : EventId::Event_Deactivate);
}

// called BEFORE processing complete (non-partial) AdvanceFrame.
void MovieRoot::OnNextFrame()
{
    // Here we need to queue
    //CheckScriptableMovieClips();

    //FrameExecSnapshot s(GetMovieImpl()->GetHeap());
    //MakeFrameExecutionSnapshot();

    // Run enterframes
    // Execute the actions in the action list, in the given environment.
    // NOTE: Actions can add other actions to the end of the list while running.
    mEventChains.QueueEvents(EventId::Event_EnterFrame);
    
    // queue up frames
    //pStage->AddChildsFrameExecutionSnapshot(pStage);
    //ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_Frame);

    // execute controller events
    //ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_ControllerEvents);
}


void MovieRoot::SetExternalInterfaceRetVal(const GFx::Value& retVal)
{
	GFxValue2ASValue(retVal, &ExternalIntfRetVal);
}

// Extracts the last token from a path and returns both parts (delimiter is '.')
// Returns false if fullPath is the name
bool MovieRoot::ExtractPathAndName(const char* fullPath, ASString* ppath, ASString* pname)
{
    // Search for colon.
    int         colonIndex = -1;
    const char* cstr = fullPath;
    const char* p;

    if ((p = SFstrrchr(cstr, '.')) != 0)
        colonIndex = int(p - cstr);
    else
        return false;

    // Make the subparts.
    *pname = GetStringManager()->CreateString(fullPath + colonIndex + 1);
    *ppath = GetStringManager()->CreateString(fullPath, colonIndex);

    return true;
}


void MovieRoot::AddStickyVariable(const ASString& path, const ASString& name, 
                                  const Value &val, Movie::SetVarType setType)
{
    if (name.IsEmpty())
        return;

    StickyVarNode* pnode;
    if ((pnode = SF_HEAP_NEW(GetMovieHeap()) StickyVarNode(name, val, (setType == Movie::SV_Permanent))) != 0)
        pMovieImpl->AddStickyVariableNode(path, pnode);
}


void    MovieRoot::ResolveStickyVariables(InteractiveObject *pch)
{
    SF_ASSERT(pch);

    const ASString &path  = pch->GetCharacterHandle()->GetNamePath();
    MovieImpl::StickyVarNode* _pnode = 0;
    StickyVarNode*   p, *pnext, *pnode;
    //AvmCharacter* pcharacter = ToAvmCharacter(pch);
    Object* pobj = ToAvmInteractiveObj(pch)->GetAS3Obj();

    if (pMovieImpl->StickyVariables.Get(path, &_pnode))
    {       
        pnode = static_cast<StickyVarNode*>(_pnode);
        // If found, add variables.     
        // We also remove sticky nodes while keeping permanent ones.
        StickyVarNode*  ppermanent = 0;
        StickyVarNode*  ppermanentTail = 0;

        p = pnode;

        while (p)
        {
            //pcharacter->SetMember(pcharacter->GetASEnvironment(), p->Name, p->mValue);
            Multiname propname(pAVM->GetPublicNamespace(), Value(p->Name));
            pobj->SetProperty(propname, p->mValue);
            pnext = static_cast<StickyVarNode*>(p->pNext);

            if (p->Permanent)
            {
                // If node is permanent, create a permanent-only linked list.
                if (!ppermanent)
                {
                    ppermanent = p;
                }
                else
                {
                    ppermanentTail->pNext = p;
                }

                ppermanentTail = p;
                p->pNext = 0;
            }
            else
            {
                // If not permanent, delete this node.
                delete p;
            }

            p = pnext;
        }


        if (ppermanent)
        {
            // If permanent list was formed, keep it.
            if (ppermanent != pnode)
                pMovieImpl->StickyVariables.Set(path, ppermanent);
        }
        else
        {
            // Otherwise delete hash key.
            pMovieImpl->StickyVariables.Remove(path);
        }
    }
}


// -------------------------------------------------------------------------------------------

// Extracts tokens that are '.' delimited from an AS2-like path. Path is expected to be null
// terminated.
class PathTokenizer
{
    mutable StringDataPtr   Path;
    mutable StringDataPtr   Token;
public:
    PathTokenizer(const char* ppath) : Path(ppath) {}
    bool                    HasToken() const { return Path.GetSize() > 0; }
    const StringDataPtr&    NextToken() const
    {
        SF_ASSERT(HasToken());

        Token = Path.GetNextToken('.');
        Path.TrimLeft( Token.GetSize() );
        if (Path.ToCStr()[0] == '.') Path.TrimLeft(1);

        return Token;
    }    
};


bool MovieRoot::GetASVariableAtPath(Value* pval, const char* ppathToVar) const
{
    if (!ppathToVar) return false;
    const_cast<MovieRoot*>(this)->CheckAvm(); // revise const_cast
    SF_ASSERT(!pAVM->IsException());
    pval->SetUndefined();
    Value resolvedVal;
    // Set the stage to be the current root
    AvmDisplayObjContainer*  pstage = GetAvmMainMovie();
    SF_ASSERT(pstage);
    resolvedVal = Value(pstage->GetAS3Obj());

    // Check in global objects first
    Multiname prop(*pAVM, ppathToVar);
    PropRef globalProp = FindScopeProperty(*pAVM, 0, pAVM->GetGlobalObjects(), prop);
    if (globalProp.IsFound())
    {
        CheckResult gcheck = globalProp.GetSlotValue(*pAVM, *pval);
        SF_ASSERT(gcheck && !pAVM->IsException());
        return gcheck && !pAVM->IsException();
    }

    // Check for known prefixes and disregard them for further path resolving
    // Certain cases target root1, else stage is used as root
    bool isroot1 = false;
    const char* ppath = ppathToVar;
    /*
    if      (!SFstrncmp(ppath, "_root.", 6))    { ppath += 6; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_root\0", 6))   { ppath += 5; isroot1 = true; }
    else if (!SFstrncmp(ppath, "root.", 5))     { ppath += 5; isroot1 = true; }
    else if (!SFstrncmp(ppath, "root\0", 5))    { ppath += 4; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_level0.", 8))  { ppath += 8; isroot1 = true; }
    else if (!SFstrncmp(ppath, "_level0\0", 8)) { ppath += 7; isroot1 = true; }
    else if (!SFstrncmp(ppath, "stage.", 6))    { ppath += 6; }
    else if (!SFstrncmp(ppath, "stage\0", 6))   { ppath += 6; }
    */
    unsigned offset = 0;
    bool r1allow = true;
    switch (ppath[0])
    {
    case '_':
        if      (!SFstrncmp(ppath+1, "root", 4)) { offset += 5; }
        else if (!SFstrncmp(ppath+1, "level0", 5)) { offset += 7; }
        break;
    case 'r': if (!SFstrncmp(ppath+1, "oot", 3)) { offset += 4; } break;
    case 's': if (!SFstrncmp(ppath+1, "tage", 4)) { offset += 5; r1allow = false; } break;
    }
    switch (ppath[offset])
    {
    case '.':  { offset++; }
    case '\0': { isroot1 = r1allow && true; ppath += offset; }
    }    

    // Get root1 and set it as the current root if needed
    if (isroot1)
    {
        DisplayObjectBase* proot1 = 
            pstage->GetDisplayList().GetDisplayObjectByName(GetBuiltinsMgr().CreateConstString("root1"));
        SF_ASSERT(proot1 && proot1->IsDisplayObjContainer());
        resolvedVal = 
            Value(ToAvmDisplayObjContainer(proot1->CharToDisplayObjContainer())->GetAS3Obj());
    }

    // Parse and traverse objects starting from display list root
    PathTokenizer pt(ppath);
    while (pt.HasToken())
    {
        const StringDataPtr& token = pt.NextToken();
        // Get property of current root element
        SF_ASSERT(resolvedVal.IsObject());
        ASString propName = GetBuiltinsMgr().CreateString(token.ToCStr(), token.GetSize());
        Multiname prop(pAVM->GetPublicNamespace(), Value(propName));
        Value subVal;
        Object* presolvedObj = resolvedVal.GetObject();
        if (!presolvedObj)
        {
            pval->SetUndefined();
            if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
            return false;
        }
        if (!presolvedObj->GetProperty(prop, subVal))
        {
            // If property not found, element may be in display list
            // Check if object is a display object container
            if (AreDisplayObjectContainerTraits(presolvedObj->GetTraitsType()))
            {
                AvmDisplayObjContainer* pcurr = 
                    ToAvmDisplayObjContainer(static_cast<Instances::DisplayObjectContainer*>(
                        presolvedObj)->pDispObj->CharToDisplayObjContainer());
                AS3::Instances::DisplayObject* pchild = pcurr->GetAS3ChildByName(propName);
                if (pchild)
                {
                    SF_ASSERT(pchild->pDispObj);
                    subVal = Value(pchild);
                }
                else
                {
                    if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
                    return false;
                }
            }
        }        
        resolvedVal = subVal;
    }

    *pval = resolvedVal;
    SF_ASSERT(!pAVM->IsException());
    return true;
}

bool MovieRoot::GetVariable(GFx::Value *pval, const char* ppathToVar) const
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to GetVariable");   
    SF_DEBUG_ASSERT(pval, "Error: NULL pval passed to GetVariable");

    Value resolvedVal;

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    pval->SetUndefined();
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        ASValue2GFxValue(resolvedVal, pval);
        return true;
    }
    return false;
}

bool MovieRoot::SetVariable(const char* ppathToVar, const GFx::Value& value, Movie::SetVarType setType /* = Movie::SV_Sticky */)
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to SetVariable");   

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Note: SetVariable cannot be used to get access to the global object(s). An arbitrary
    //       variable/path (not targetted to root/_root/stage) will be processed under the 
    //       'root' scope automatically.

    // Extract path and name
    ASString path = GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_);
    ASString name = GetBuiltinsMgr().GetBuiltin(AS3Builtin_empty_);
    if (!ExtractPathAndName(ppathToVar, &path, &name))
    {
        // Path extraction failed; ppathToVar is pointing to a root var
        name = GetStringManager()->CreateString(ppathToVar);
        path = GetStringManager()->CreateString("root");
    }
    // Get path (parent) var
    Value parentVal;
    if (!GetASVariableAtPath(&parentVal, path.ToCStr()))
        return false;
    // Put property name into path var
    SF_ASSERT(parentVal.IsObject());
    Object* pparentObj = parentVal.GetObject();
    Multiname prop(pAVM->GetPublicNamespace(), Value(name));
    Value putval;
    GFxValue2ASValue(value, &putval);
    bool setResult = pparentObj->SetProperty(prop, putval);

    if ( (!setResult && (setType != Movie::SV_Normal)) ||
        (setType == Movie::SV_Permanent) )
    {
        // Store in sticky hash.
        AddStickyVariable(path, name, putval, setType);
    }

    if (pAVM->IsException()) pAVM->OutputAndIgnoreException();
    return setResult;
}

unsigned MovieRoot::GetVariableArraySize(const char* ppathToVar)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resolvedVal;
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        SF_ASSERT(resolvedVal.IsObject());
        Object* pobj = resolvedVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array)
        {
            Instances::Array* parr = static_cast<Instances::Array*>(pobj);
            return parr->GetSize();
        }
    }
    return 0;
}

bool MovieRoot::GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, void* pdata, unsigned count)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    Value resolvedVal;
    if (GetASVariableAtPath(&resolvedVal, ppathToVar))
    {
        if (!resolvedVal.IsObject()) return false;
        Object* pobj = resolvedVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array)
        {
            pMovieImpl->pRetValHolder->ResetPos();
            pMovieImpl->pRetValHolder->ResizeStringArray(0);

            Instances::Array* parray = static_cast<Instances::Array*>(pobj);
            unsigned arrSize = parray->GetSize();
            switch (type)
            {
            case Movie::SA_Int:
                {
                    int* parr = static_cast<int*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (int)val.AsInt();
                        else
                            parr[i] = 0;
                    }
                }
            case Movie::SA_Float:
                {
                    float* parr = static_cast<float*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (float)val.AsNumber();
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Double:
                {
                    Double* parr = static_cast<Double*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                            parr[i] = (Double)val.AsNumber();
                        else
                            parr[i] = 0;
                    }
                }
                break;
            case Movie::SA_Value:
                {
                    GFx::Value* parr = static_cast<GFx::Value*>(pdata);
                    for (unsigned i = 0, n = Alg::Min(arrSize, count); i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        GFx::Value* pdestVal = &parr[i];
                        pdestVal->SetUndefined();
                        if (!val.IsUndefined())
                            ASValue2GFxValue(val, pdestVal);
                        else
                            pdestVal->SetUndefined();
                    }
                }
                break;
            case Movie::SA_String:
                {
                    const char** parr = static_cast<const char**>(pdata);
                    unsigned n = Alg::Min(arrSize, count);
                    pMovieImpl->pRetValHolder->ResizeStringArray(n);
                    for (unsigned i = 0; i < n; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                        {
                            ASString str = val.AsString();
                            parr[i] = str.ToCStr();
                            pMovieImpl->pRetValHolder->StringArray[pMovieImpl->pRetValHolder->StringArrayPos++] = str;
                        }
                        else
                            parr[i] = NULL;
                    }
                }
                break;
            case Movie::SA_StringW:
                {
                    const wchar_t** parr = static_cast<const wchar_t**>(pdata);
                    // pre-calculate the size of required buffer
                    unsigned i, bufSize = 0;
                    unsigned n = Alg::Min(arrSize, count);
                    pMovieImpl->pRetValHolder->ResizeStringArray(n);
                    for (i = 0; i < arrSize; ++i)
                    {
                        const Value& val = parray->At(i+index);
                        if (!val.IsUndefined())
                        {
                            ASString str = val.AsString();
                            pMovieImpl->pRetValHolder->StringArray[i] = str;
                            bufSize += str.GetLength()+1;
                        }
                    }
                    wchar_t* pwBuffer = (wchar_t*)pMovieImpl->pRetValHolder->PreAllocateBuffer(bufSize * sizeof(wchar_t));

                    for (i = 0; i < n; ++i)
                    {
                        const char* psrcStr = pMovieImpl->pRetValHolder->StringArray[i].ToCStr();
                        wchar_t* pwdstStr = pwBuffer;
                        UInt32 code;
                        while ((code = UTF8Util::DecodeNextChar(&psrcStr)) != 0)
                        {
                            *pwBuffer++ = (wchar_t)code;
                        }
                        *pwBuffer++ = 0;

                        parr[i] = pwdstStr;
                    }
                    pMovieImpl->pRetValHolder->ResizeStringArray(0);
                }
                break;
            }
            return true;
        }
    }
    return false;
}


bool MovieRoot::SetVariableArraySize(const char* ppathToVar, unsigned count, Movie::SetVarType setType)
{
    // TODO: Check VM

    Value retVal;
    if (GetASVariableAtPath(&retVal, ppathToVar) && retVal.IsObject())
    {
        Object* pobj = retVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array)
        {
            // Array already exists - just resize
            Instances::Array* parray;
            parray = static_cast<Instances::Array*>(pobj);
            if (count != unsigned(parray->GetSize()))
                parray->Resize(count);
            return true;
        }
    }

    // no array found - allocate new one, resize and set
    Pickable<Instances::Array> parray = pAVM->MakeArray();
    parray->Resize(count);
    
    Value val(parray);
    GFx::Value gfxval;
    ASValue2GFxValue(val, &gfxval);
    return SetVariable(ppathToVar, gfxval, setType);
}


bool MovieRoot::SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                 unsigned index, const void* pdata, unsigned count, Movie::SetVarType setType)
{
    // TODO: Check VM

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    AS3::SPtr<Instances::Array> parray;
    Value retVal;
    if(GetASVariableAtPath(&retVal, ppathToVar) && retVal.IsObject())
    {
        Object* pobj = retVal.GetObject();
        if (pobj && pobj->GetTraitsType() == Traits_Array)
            parray = static_cast<Instances::Array*>(pobj);
    }

    bool found = true;
    if (!parray) 
    {
        parray = pAVM->MakeArray();
        found = false;
    }
    if (count + index > unsigned(parray->GetSize()))
        parray->Resize(count + index);

    switch(type)
    {
    case Movie::SA_Int:
        {
            const SInt32* parr = static_cast<const SInt32*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(parr[i]));
            }
            break;
        }
    case Movie::SA_Float:
        {
            const float* parr = static_cast<const float*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(Value::Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Double:
        {
            const Double* parr = static_cast<const Double*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(Value::Number(parr[i])));
            }
            break;
        }
    case Movie::SA_Value:
        {
            const GFx::Value* parr = static_cast<const GFx::Value*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                Value asVal;
                GFxValue2ASValue(parr[i], &asVal);
                parray->Set(i + index, asVal);
            }
            break;
        }
    case Movie::SA_String:
        {
            const char* const* parr = static_cast<const char* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    case Movie::SA_StringW:
        {
            const wchar_t* const* parr = static_cast<const wchar_t* const*>(pdata);
            for (unsigned i = 0; i < count; ++i)
            {
                parray->Set(i + index, Value(GetStringManager()->CreateString(parr[i])));
            }
            break;
        }
    }

    if (!found)
    {
        Value val(parray, PickValue);
        GFx::Value gfxval;
        ASValue2GFxValue(val, &gfxval);
        return SetVariable(ppathToVar, gfxval, setType);
    }
    return true;
}


bool MovieRoot::IsAvailable(const char* ppathToVar) const
{
    // TODO: Check VM

    SF_DEBUG_ASSERT(ppathToVar, "Error: NULL ppathToVar passed to IsAvailable");   

    Value resolvedVal;
    return GetASVariableAtPath(&resolvedVal, ppathToVar);
}

bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult,
                       const GFx::Value* pargs, unsigned numArgs)
{   
    SF_AMP_SCOPE_TIMER_ID(pMovieImpl->AdvanceStats, "MovieRoot::Invoke", Amp_Native_Function_Id_Invoke);

    // TODO: Check VM

    SF_DEBUG_ASSERT(pmethodName, "Error: NULL pmethodName passed to Invoke");   

    bool retVal;
    Value resultVal;

    // There should not be an exception in the VM
    SF_ASSERT(!pAVM->IsException());

    // Setup invoke parameters
    // ** Create storage
    enum { NumValuesOnStack = 10 };
    void* argArrayOnStack[NumValuesOnStack*((sizeof(GFx::Value)+sizeof(void*))/sizeof(void*))];
    Value* pargArray;
    if (numArgs > NumValuesOnStack)
        pargArray = (Value*)
        SF_HEAP_AUTO_ALLOC_ID(this, sizeof(Value) * numArgs, GFx::StatMV_ActionScript_Mem);
    else
        pargArray = (Value*)argArrayOnStack;
    // ** Convert to AS3::Value
    for (unsigned i = 0, n = numArgs; i < n; ++i)
    {
        const GFx::Value& val = pargs[i];
        Value* pdestVal = Scaleform::Construct<Value>(&pargArray[i]);
        GFxValue2ASValue(val, pdestVal);
    }

    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Try to resolve invoke alias
    Value* paliasFunc;
    if (pInvokeAliases && (paliasFunc = ResolveInvokeAlias(pmethodName)) != NULL)
        pAVM->Execute(*paliasFunc, Value::GetUndefined(), resultVal, numArgs, pargArray);
    else
    {
        Value resolvedVal;
        if (GetASVariableAtPath(&resolvedVal, pmethodName))
            pAVM->Execute(resolvedVal, Value::GetUndefined(), resultVal, numArgs, pargArray);
    }

    retVal = !pAVM->IsException();
    if (pAVM->IsException()) pAVM->OutputAndIgnoreException();

    // Convert result to GFx::Value
    if (retVal && presult)
        ASValue2GFxValue(resultVal, presult);

    // Destruct elements in AS3::Value params array
    for (unsigned i = 0, n = numArgs; i < n; ++i)
        pargArray[i].~Value();
    if (numArgs > sizeof(argArrayOnStack)/sizeof(argArrayOnStack[0]))
        SF_FREE(pargArray);

    return retVal;
}


bool MovieRoot::Invoke(const char* pmethodName, GFx::Value *presult, const char* pargFmt, ...)
{
    // Delegate to InvokeArgs
    va_list args;
    va_start(args, pargFmt);
    bool retVal = InvokeArgs(pmethodName, presult, pargFmt, args);
    va_end(args);
    return retVal;
}



void MovieRoot::ParseValueArguments(Array<Value>& arr, const char* pmethodName, 
                                    const char* pargFmt, va_list args) const
{
    LogState* plog = GetLogState();
    arr.Clear();
    if (pargFmt)
    {
        Value arg;
        const char* p = pargFmt;
        for (;;)
        {
            char    c = *p++;
            if (c == 0)
            {
                // End of args.
                break;
            }
            else if (c == '%')
            {
                c = *p++;
                // Here's an arg.
                if (c == 'd')
                {
                    // Integer.
                    arg.SetSInt32(va_arg(args, int));
                    arr.PushBack(arg);
                }
                else if (c == 'u')
                {
                    // Undefined.
                    arg.SetUndefined();
                    arr.PushBack(arg);
                }
                else if (c == 'n')
                {
                    // Null.
                    arg.SetNull();
                    arr.PushBack(arg);
                }
                else if (c == 'b')
                {
                    // Boolean
                    arg.SetBool(bool(va_arg(args, int) != 0));
                    arr.PushBack(arg);
                }
                else if (c == 'f')
                {
                    // Double    
                    arg.SetNumber((Value::Number)va_arg(args, double));
                    arr.PushBack(arg);

                    // MA: What about float? C specification states that "float converts to
                    // double by the standard argument promotion", so this works. But,
                    // what happens on PS2?  Do we needs '%hf' specifier for float?
                }
                else if (c == 'h')
                {
                    c = *p++;
                    if (c == 'f')
                    {
                        // AB: %hf will be treated as double too, since C spec states
                        // that float is converted to double.
                        // Double
                        arg.SetNumber((Value::Number)va_arg(args, double));
                        arr.PushBack(arg);
                        SF_DEBUG_WARNING2(1, "ParseArguments('%s','%s') - '%%hf' will be treated as double", 
                            pmethodName,
                            pargFmt);
                    }
                    else
                    {
                        if (plog)
                            plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%h%c'",
                                pmethodName,
                                pargFmt,
                                c);
                    }
                }
                else if (c == 's')
                {
                    // String                
                    arr.PushBack(GetBuiltinsMgr().CreateString(va_arg(args, const char *)));
                }
                else if (c == 'l')
                {
                    c = *p++;
                    if (c == 's')
                    {
                        // Wide string.
                        arr.PushBack(GetBuiltinsMgr().CreateString(va_arg(args, const wchar_t *)));
                    }
                    else
                    {
                        if (plog)
                            plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%l%c'",
                                pmethodName,
                                pargFmt,
                                c);
                    }
                }
                else
                {
                    // Invalid fmt, warn.
                    if (plog)
                        plog->LogScriptError("ParseArguments('%s','%s') - invalid format '%%%c'",
                            pmethodName,
                            pargFmt,
                            c);
                }
            }
            else
            {
                // Invalid arg; warn.                
                if (plog)
                    plog->LogScriptError("ParseArguments('%s','%s') - invalid char '%c'",
                        pmethodName,
                        pargFmt,
                        c);
            }
            // skip all whitespaces
            for (c = *p; c != '\0' && (c == ' ' || c == '\t' || c == ','); c = *++p) 
                ;
        }
    }
}

bool MovieRoot::InvokeArgs(const char* pmethodName, GFx::Value *presult, const char* pargFmt, va_list args)
{
    // TODO: Check VM
    SF_AMP_SCOPE_TIMER_ID(pMovieImpl->AdvanceStats, "MovieRoot::InvokeArgs", Amp_Native_Function_Id_InvokeArgs);

    bool retVal;
    Value resultVal;

    SF_DEBUG_ASSERT(pmethodName, "Error: NULL pmethodName passed to InvokeArgs");
    SF_DEBUG_ASSERT(pargFmt, "Error: NULL pargFmt passed to InvokeArgs");

    // There should not be an exception in the VM
    SF_ASSERT(!pAVM->IsException());

    // Parse args into AS3::Value.s
    Array<Value> valArray;
    ParseValueArguments(valArray, pmethodName, pargFmt, args);
    
    unsigned numArgs = valArray.GetSize();
    Value* pargArray = &valArray[0];
    
    // Need to restore high precision mode of FPU for X86 CPUs.
    // Direct3D may set the Mantissa Precision Control Bits to 24-bit (by default 53-bits) and this 
    // leads to bad precision of FP arithmetic. For example, the result of 0.0123456789 + 1.0 will 
    // be 1.0123456789 with 53-bit mantissa mode and 1.012345671653 with 24-bit mode.
    DoublePrecisionGuard dpg;

    // Try to resolve invoke alias
    Value* paliasFunc;
    if (pInvokeAliases && (paliasFunc = ResolveInvokeAlias(pmethodName)) != NULL)
        pAVM->Execute(*paliasFunc, Value::GetUndefined(), resultVal, numArgs, pargArray);
    else
    {
        Value resolvedVal;
        if (GetASVariableAtPath(&resolvedVal, pmethodName))
            pAVM->Execute(resolvedVal, Value::GetUndefined(), resultVal, numArgs, pargArray);
    }

    retVal = !pAVM->IsException();
    if (pAVM->IsException()) pAVM->IgnoreException();

    // Convert result to GFx::Value
    if (retVal && presult)
        ASValue2GFxValue(resultVal, presult);

    return retVal;
}


// *** Action List management
MovieRoot::ActionQueueType::ActionQueueType(MemoryHeap *pheap)
: pHeap(pheap)
{
    ModId = 1;
    pFreeEntry = NULL;
    FreeEntriesCount = 0;
}

MovieRoot::ActionQueueType::~ActionQueueType()
{
    Clear();

    // free pFreeEntry chain
    for(ActionEntry* pcur = pFreeEntry; pcur; )
    {
        ActionEntry* pnext = pcur->pNextEntry;
        delete pcur;
        pcur = pnext;
    }
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::GetNewEntry()
{
    ActionEntry *p = 0;
    if (pFreeEntry)
    {
        p = pFreeEntry;
        pFreeEntry = pFreeEntry->pNextEntry;
        p->pNextEntry = 0;
        --FreeEntriesCount;
    }
    else
    {
        if ((p = SF_HEAP_NEW(GetMovieHeap()) ActionEntry)==0)
            return 0;
    }
    return p;
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::InsertEntry(ActionLevel lvl)
{
    ActionEntry *p = GetNewEntry();

    ActionQueueEntry& actionQueueEntry = Entries[lvl];

    // Insert and move insert pointer to us.
    if (actionQueueEntry.pInsertEntry == 0) // insert at very beginning
    {
        p->pNextEntry                   = actionQueueEntry.pActionRoot;
        actionQueueEntry.pActionRoot    = p;
    }
    else
    {
        p->pNextEntry = actionQueueEntry.pInsertEntry->pNextEntry;
        actionQueueEntry.pInsertEntry->pNextEntry = p;
    }
    actionQueueEntry.pInsertEntry   = p;
    if (p->pNextEntry == NULL)
        actionQueueEntry.pLastEntry = p;
    ++ModId;

    // Done
    return p;
}

MovieRoot::ActionEntry*  MovieRoot::ActionQueueType::PrependEntry(ActionLevel lvl)
{
    ActionEntry *p = GetNewEntry();

    ActionQueueEntry& actionQueueEntry = Entries[lvl];
    
    // insert at very beginning
    p->pNextEntry                   = actionQueueEntry.pActionRoot;
    actionQueueEntry.pActionRoot    = p;
    if (actionQueueEntry.pInsertEntry == 0) 
        actionQueueEntry.pInsertEntry   = p;
    if (p->pNextEntry == NULL)
        actionQueueEntry.pLastEntry = p;
    ++ModId;

    // Done
    return p;
}

MovieRoot::ActionEntry* MovieRoot::ActionQueueType::FindEntry(
    ActionLevel lvl, const ActionEntry& entry)
{
    ActionQueueEntry& queue = Entries[lvl];
    ActionEntry* pentry = queue.pActionRoot;
    for (; pentry != NULL; pentry = pentry->pNextEntry)
    {
        if (*pentry == entry)
            return pentry;
    }
    return NULL;
}

void MovieRoot::ActionQueueType::AddToFreeList(MovieRoot::ActionEntry* pentry)
{
    pentry->ClearAction();
    if (FreeEntriesCount < 50)
    {
        pentry->pNextEntry = pFreeEntry;
        pFreeEntry = pentry;
        ++FreeEntriesCount;
    }
    else
        delete pentry;
}

void    MovieRoot::ActionQueueType::Clear()
{   
    for (unsigned i = 0; i < AL_Count_; ++i)
    {
        MovieRoot::ActionQueueIterator iter((ActionLevel)i, this);
        while(iter.getNext())
            ;
    }
}

MovieRoot::ActionQueueIterator::ActionQueueIterator(
    ActionLevel lvl, MovieRoot::ActionQueueType* pactionQueue, ActionEntry* rootEntry)
{
    SF_ASSERT(pactionQueue);
    pActionQueue    = pactionQueue;
    Level           = lvl;
    ModId           = 0;
    pLastEntry      = 0;
    pRootEntry      = rootEntry;
    pCurEntry       = (pRootEntry) ? pRootEntry->pNextEntry : pActionQueue->Entries[Level].pActionRoot;
}

MovieRoot::ActionQueueIterator::~ActionQueueIterator()
{
    if (pLastEntry)
        pActionQueue->AddToFreeList(pLastEntry); // release entry
}

const MovieRoot::ActionEntry* MovieRoot::ActionQueueIterator::getNext()
{
    if (pActionQueue->ModId != ModId)
    {
        // new actions were added - restart
        ModId = pActionQueue->ModId;
        pCurEntry = (pRootEntry) ? pRootEntry->pNextEntry : pActionQueue->Entries[Level].pActionRoot;
    }
   
    ActionEntry* pentry = pCurEntry;
    if (pCurEntry)
    {
        if (pCurEntry == pActionQueue->Entries[Level].pInsertEntry)
        {
            if (pCurEntry->pNextEntry)
                pActionQueue->Entries[Level].pInsertEntry = pCurEntry->pNextEntry;
            else
                pActionQueue->Entries[Level].pInsertEntry = pRootEntry;
        }
        if (pCurEntry == pActionQueue->Entries[Level].pActionRoot)
            pActionQueue->Entries[Level].pActionRoot = pCurEntry->pNextEntry;
        else if (pRootEntry)
            pRootEntry->pNextEntry = pCurEntry->pNextEntry;
        pCurEntry = pCurEntry->pNextEntry;
    }
    if (pActionQueue->Entries[Level].pActionRoot == NULL)
    {
        pActionQueue->Entries[Level].pInsertEntry = NULL;
        pActionQueue->Entries[Level].pLastEntry = NULL;
    }
    if (pLastEntry)
    {
        pLastEntry->pNextEntry = NULL;
        pActionQueue->AddToFreeList(pLastEntry); // release entry
        // queue is modified; in the case if there other iterators we need to 
        // update ModId to force those iterators to restart.
        ++pActionQueue->ModId;
        ModId = pActionQueue->ModId;
    }
    pLastEntry = pentry;
    return pentry;
}


// Executes actions in this entry
void    MovieRoot::ActionEntry::Execute(MovieRoot *proot) const
{   
    SF_UNUSED(proot);
    // pCharacter might be null, if only InsertEmptyAction was used, without
    // subsequent call to SetAction.
    if (!pCharacter || pCharacter->IsUnloaded())
        return;

    switch(Type)
    {
    case Entry_Event:       
        //ToAvmCharacter(pCharacter)->ExecuteEvent(EventId);      
        ToAvmDisplayObj(pCharacter)->FireEvent(mEventId);
        break;

    case Entry_Function:        
        {
            SF_ASSERT(proot->pAVM);
            Value _this(ToAvmDisplayObj(pCharacter)->GetAS3Obj());
            Value result;
            proot->pAVM->Execute_IgnoreException(Function, _this, result, 0, NULL);
        }
//        ToAvmCharacter(pCharacter)->ExecuteFunction(Function, FunctionParams);      
        break;

    case Entry_None:
        break;
    }
}

void    MovieRoot::AddInvokeAlias(const ASString& alias, const Value& closure)
{
    if (!pInvokeAliases)
        pInvokeAliases = new ASStringHash<Value>();
    pInvokeAliases->Set(alias, closure);
}

Value* MovieRoot::ResolveInvokeAlias(const char* pstr)
{
    SF_ASSERT(GetStringManager());
    if (pInvokeAliases)
    {
        ASString str = GetStringManager()->CreateString(pstr);
        return pInvokeAliases->Get(str);
    }
    return NULL;
}

void MovieRoot::AddScriptableMovieClip(DisplayObjContainer* pspr)
{
    if (!pspr->GetParent())
    {
        //pStage->InsertChildToPlayList(pspr);
        pspr->AddToPlayList();
        pspr->ModifyOptimizedPlayList();
    }
    // All sprites created by new and that has timelines 
    // are stored in special storage. Their timelines are executed
    // regardless to the fact they are attached to display list or not.
    NewSprites.Add(pspr, Ptr<DisplayObjContainer>(pspr));
}

void MovieRoot::CheckScriptableMovieClips()
{
    // check only every 100 call. Need to think about more intelligent way.
    // If a sprite, created by AS operator "new", stopped, detached from its
    // parent (or never was attached) and refcount == 1 (means, it is held by
    // the hash "NewSprites" only) then we can remove the sprite permanently from
    // the hash.
    //if (CheckClipsCnt++ >= 100)
    {
        for (NewSpritesHashType::Iterator it = NewSprites.Begin(); it != NewSprites.End(); ++it)
        {
            DisplayObjContainer* spr = it->Second;
            if (spr->GetPlayState() == State_Stopped && 
                !spr->GetParent() &&
                spr->GetRefCount() == 1)
            {
                // we can remove unreferenced, stopped, unlinked sprite from the list
                spr->RemoveFromPlayList();
                it.RemoveAlt(spr);
            }
        }
        CheckClipsCnt = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
void EventChains::AddToChain(EventId::IdCode evtId, DisplayObject* obj)
{
    AutoPtr<Chain>* ppchain;
    Chain* chain;
    if ((ppchain = Chains.Get(evtId)) == NULL)
    {
        // create new array
        chain = SF_HEAP_AUTO_NEW(this) Chain();
        Chains.Add(evtId, chain);
    }
    else
        chain = ppchain->GetPtr();
    SF_ASSERT(chain);
    if (FindObjectIndexInChain(chain, obj) == SF_MAX_UPINT)
        chain->PushBack(obj);
}

void EventChains::RemoveFromChain(EventId::IdCode evtId, DisplayObject* obj)
{
    AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        UPInt i = FindObjectIndexInChain(chain, obj);
        if (i != SF_MAX_UPINT)
        {
            // nullify; remove in Dispatch to avoid
            // concurrent modification.
            (*chain)[i] = NULL;
        }
    }
}

const ArrayLH<Ptr<DisplayObject> >* EventChains::GetChain(EventId::IdCode evtId) const
{
    const AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
        return ppchain->GetPtr();
    return NULL;
}

void EventChains::Dispatch(EventId::IdCode evtId)
{
   AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        for (UPInt i = 0; i < chain->GetSize();)
        {
            if (!(*chain)[i])
            {
                // remove empty entries
                chain->RemoveAt(i);
                continue;
            }
            EventId evt(evtId);
            ToAvmDisplayObj((*chain)[i])->FireEvent(evt);
            ++i;
        }
    }
}

void EventChains::QueueEvents(EventId::IdCode evtId)
{
    AutoPtr<Chain>* ppchain = Chains.Get(evtId);
    if (ppchain)
    {
        Chain* chain = ppchain->GetPtr();
        for (UPInt i = 0; i < chain->GetSize();)
        {
            if (!(*chain)[i])
            {
                // remove empty entries
                chain->RemoveAt(i);
                continue;
            }
            EventId evt(evtId);
            ToAvmDisplayObj((*chain)[i])->OnEvent(evt);
            ++i;
        }
    }
}

UPInt EventChains::FindObjectIndexInChain(Chain* chain, DisplayObject* obj)
{
    for (UPInt i = 0, n = chain->GetSize(); i < n; ++i)
    {
        if ((*chain)[i] == obj)
            return i;
    }
    return SF_MAX_UPINT;
}

// ****************************************************************************
// Adds load queue entry based on parsed url and target path 
// (Load/unload Movie)
//
LoadQueueEntry::LoadQueueEntry(Instances::URLRequest* request, Instances::Loader* loader, LoadMethod method, bool quietOpen)
: GFx::LoadQueueEntry((request)?request->GetUrl().ToCStr():"", method, false, quietOpen)
{ 
    mLoader     = loader;
    mURLRequest = request;
    FirstExec   = true;
}

LoadQueueEntry::LoadQueueEntry(Instances::URLRequest* request, Instances::URLLoader* loader, LoadMethod method, bool quietOpen) 
: GFx::LoadQueueEntry((request)?request->GetUrl().ToCStr():"", method, true, quietOpen)
{
    mURLLoader  = loader;
    mURLRequest = request;
    FirstExec   = true;
}   

LoadQueueEntry::~LoadQueueEntry()
{
}

LoadQueueEntryMT_LoadMovie::LoadQueueEntryMT_LoadMovie(
    LoadQueueEntry* pqueueEntry, MovieImpl* pmovieRoot)
: GFx::LoadQueueEntryMT(pqueueEntry, pmovieRoot)
{
    CharSwitched        = false;
    FirstFrameLoaded    = false;
    BytesLoaded         = 0;

    bool stripped       = false;
    /*if (pqueueEntry->pCharacter)
    {
        // This is loadMovie, not level.
        SF_ASSERT(qe->Level == -1);
        SF_ASSERT(!(qe->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag));

        // Make sure we have character targets and parents.
        Ptr<DisplayObject> poldChar = pqueueEntry->pCharacter->ResolveCharacter(pMovieRoot);
        if (poldChar)
            stripped = (poldChar->GetResourceMovieDef()->GetSWFFlags() 
            & MovieInfo::SWF_Stripped) != 0;
    }
    else if (pqueueEntry->Level != -1)
    {
        MovieRoot* as2mr = static_cast<MovieRoot*>(pMovieRoot->pASMovieRoot);
        SF_ASSERT(qe->Type & GFxAS2LoadQueueEntry::LTF_LevelFlag);
        if (as2mr->GetLevelMovie(qe->Level))
            stripped = ((as2mr->GetLevelMovie(qe->Level)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
        else if (as2mr->GetLevelMovie(0))
            stripped = ((as2mr->GetLevelMovie(0)->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
    }*/
    MovieRoot* root = ToAS3Root(pMovieImpl);
    if (root->GetMainMovie())
    {
        stripped = ((root->GetMainMovie()->GetResourceMovieDef()->GetSWFFlags()
            & MovieInfo::SWF_Stripped) != 0);
    }
    pPreloadTask = *SF_NEW MoviePreloadTask(pMovieImpl, pqueueEntry->URL, stripped, pqueueEntry->QuietOpen);
    pMovieImpl->GetTaskManager()->AddTask(pPreloadTask);
}

LoadQueueEntryMT_LoadMovie::~LoadQueueEntryMT_LoadMovie()
{
}

// Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
// Returns true if the movie is loaded completely or in the case of errors.
bool LoadQueueEntryMT_LoadMovie::LoadFinished()
{
    bool btaskDone = pPreloadTask->IsDone();
    // If canceled, wait until task is done
    // [PPS] Change this logic to call AbandonTask and 
    // handle it appropriately inside the task to die
    // instantly
    if (pQueueEntry->Canceled && btaskDone)
    {
        return true;
    }

    //bool extensions = false;

    LoadQueueEntry* qe = static_cast<LoadQueueEntry*>(pQueueEntry);
    MovieRoot*   root = ToAS3Root(pMovieImpl);
    if (btaskDone)
    {
        // Preload task is finished by now. Check if a movedef is created
        MovieDefImpl* pdefImpl = pPreloadTask->GetMoiveDefImpl();
        if (!pdefImpl)
        {
            // A moviedef was not created. This means that loader couldn't load a file. 
            // Creating an empty movie and set it instead of the old one.

            //printf("!!! ERR LoadFinished!\n");
            qe->mLoader->ExecuteErrorEvent(qe->mURLRequest->GetUrl().ToCStr());
            qe->FirstExec = false;
        }
        else
        {
            if (qe->FirstExec)
            {
                qe->mLoader->ExecuteOpenEvent();
                qe->FirstExec = false;

                root->LoadedMovieDefs.Set(pdefImpl);
            }
            // A moviedef was created successfully.
            if (!CharSwitched)
            {
                //Replace the old character with the new sprite. This operation should be done only once
                //per loading movie.
                DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                SF_UNUSED(pparent);
//                 if (qe->Level != -1)
//                 {
//                     pMovieImpl->ReleaseLevelMovie(qe->Level);
//                     NewCharId = ResourceId(); // Assign invalid id.
//                 } 
//                 else if (qe->pCharacter)
//                 {
//                     pparent = pOldChar->GetParent();
//                     // make sure that the parent of the character that we want to
//                     // replace is still alive.
//                     if (!pparent)
//                         return true;
//                 }
                ResourceId NewCharId = ResourceId(); // Assign invalid id.
                CharSwitched = true;
            }

            // The number of loaded bytes has changed since the last iteration and we have a movie loader listener
            // sent a progress notification.
            if (BytesLoaded != pdefImpl->GetBytesLoaded())
            {
                // get Loader
                BytesLoaded = pdefImpl->GetBytesLoaded();
                qe->mLoader->ExecuteProgressEvent(BytesLoaded, pdefImpl->GetFileBytes());
            }

            // when we loaded the first frame process its actions.
            if (!FirstFrameLoaded && ((pdefImpl->pBindData->GetBindState() & MovieDefImpl::BSF_Frame1Loaded) != 0))
            {
                // First frame is loaded.
                if (pdefImpl->GetDataDef()->MovieType == MovieDataDef::MT_Flash)
                {

                    // This branch will be executed when gfxExtensions == true, to 
                    // produce different (better) behavior during the loading. If extensions
                    // are on then we can start the loading movie after the first frame is loaded.
                    // Flash, for some reasons, waits until whole movie is loaded.

                    DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                    ResourceId newCharId = ResourceId(0); // root element always has 0 res id.
                    Ptr<Sprite> pnewChar;
                    if (pdefImpl)
                    {
                        // CharId of loaded clip is the same as the old one
                        // Also mark clip as loadedSeparately, so that _lockroot works.
                        pnewChar = *static_cast<Sprite*>(root->GetASSupport()->CreateCharacterInstance(
                            pMovieImpl, pdefImpl->GetDataDef(), 
                            pparent, newCharId, pdefImpl, CharacterDef::Sprite));
                        pnewChar->SetLoadedSeparately();
                        pnewChar->SetTimelineObjectFlag(true);
                        pnewChar->OnEventLoad();

                        pnewChar->AddToPlayList();
                        // Run actions on Frame 0. This should be done before calling OnLoadInit
                        pnewChar->ExecuteFrame0Events();
                        ToAvmInteractiveObj(pnewChar)->CreateASInstance(true);

                        //root->InsertEmptyAction(MovieRoot::AL_Highest)->SetAction(pnewChar, EventId::Event_Initialize);
                        //root->InsertEmptyAction(MovieRoot::AL_High)->SetAction(pnewChar, EventId::Event_Construct);

                        ToAvmDisplayObjContainer(pparent)->AddChild(pnewChar);
                        //pparent->ReplaceChildCharacter(poldChar, pnewChar);
                        // In case permanent variables were assigned.. check them.
                        root->ResolveStickyVariables(pnewChar);

                        // need to update the optimized play list
                        pnewChar->ModifyOptimizedPlayListLocal<Sprite>();
                        //pnewChar->ExecuteFrame0Events();
                        root->DoActions();
                    }
                    
                    // Fire "init" event
                    qe->mLoader->ExecuteInitEvent(pnewChar);
                }
                FirstFrameLoaded = true;
            }
            //printf("!!! LoadFinished! %s\n", qe->mURLRequest->GetUrl().ToCStr());

            // If the movie is not completely loaded yet try it on the next iteration.
            if ((pdefImpl->pBindData->GetBindState() 
                & (MovieDefImpl::BS_InProgress | MovieDefImpl::BS_Finished)) < MovieDefImpl::BS_Finished)
                return false;
            if (pdefImpl->pBindData->GetBindState() & MovieDefImpl::BS_Finished)
            {
                //printf("!!! Finished! %d == %d\n", pdefImpl->GetDataDef()->MovieType, MovieDataDef::MT_Image);
                if (pdefImpl->GetDataDef()->MovieType == MovieDataDef::MT_Image)
                {
                    // bitmap
                    DisplayObjContainer* pparent = qe->mLoader->GetDisplayObjContainer();
                    ResourceId NewCharId = ResourceId(); // Assign invalid id.
                    Ptr<AS3::AvmBitmap> pnewChar = *static_cast<AS3::AvmBitmap*>(root->GetASSupport()->CreateCharacterInstance(
                        root->GetMovieImpl(), pdefImpl->GetDataDef(), 
                        pparent, NewCharId, pdefImpl, CharacterDef::Bitmap));

                    ResourceHandle rh;
                    if (pdefImpl->GetDataDef()->GetResourceHandle(&rh, ResourceId(CharacterDef::CharId_ImageMovieDef_ImageResource)))
                    {
                        Resource* pres = rh.GetResource(&pdefImpl->GetResourceBinding());
                        //printf("!!! Res: %p\n", pres);
                        if (pres)
                        {
                            if (pres->GetResourceType() == Resource::RT_Image)
                            {
                                //printf("!!! SetImage\n");
                                pnewChar->SetImage(static_cast<ImageResource*>(pres));
                            }
                        }
                    }

                    ToAvmDisplayObjContainer(pparent)->AddChild(pnewChar);
                    // Fire "init" event
                    qe->mLoader->ExecuteInitEvent(pnewChar);
                }

                // fire "complete" event
                qe->mLoader->ExecuteCompleteEvent();
            } 
            else 
            {
                // A error happened during the movie loading 
                qe->mLoader->ExecuteErrorEvent(qe->mURLRequest->GetUrl().ToCStr());
                qe->FirstExec = false;
            }
        }
        // Loading finished. Remove the item from the queue.
        return true;
    }
    else
    {
        if (qe->FirstExec)
        {
            qe->mLoader->ExecuteOpenEvent();
            qe->FirstExec = false;
        }
    }   
    return false;
}

bool LoadQueueEntryMT_LoadMovie::IsPreloadingFinished()
{
    return pPreloadTask->IsDone();
}


// *** Load/Unload movie support
void    MovieRoot::AddLoadQueueEntryMT(LoadQueueEntry* pentry)
{
    LoadQueueEntryMT* pentryMT = NULL;
    if (pentry->Type & LoadQueueEntry::LTF_VarsFlag)
    {
#ifdef GFX_AS2_ENABLE_LOADVARS
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadVars(pentry, pMovieImpl);
#endif
    }
    /*else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_XMLFlag)
    {
#ifdef GFX_ENABLE_XML
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadXML(pentry, this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->XMLHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->XMLHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByXMLASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    } 
    else if (pentry->Type & GFxAS2LoadQueueEntry::LTF_CSSFlag)
    {
#ifdef GFX_ENABLE_CSS
        if (pentry->URL.GetLength())
        {
            pentryMT = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntryMT_LoadCSS(pentry,this);
            GFxAS2LoadQueueEntry* as2entry = static_cast<GFxAS2LoadQueueEntry*>(pentryMT->GetQueueEntry());
            if (pentryMT && as2entry->CSSHolder.ASObj.ToObject(NULL))
            {
                Object* pxmlobj = as2entry->CSSHolder.ASObj.ToObject(NULL);
                LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
                while (ptail)    
                {
                    GFxAS2LoadQueueEntry* as2tail = static_cast<GFxAS2LoadQueueEntry*>(ptail->GetQueueEntry());
                    as2tail->CancelByCSSASObjPtr(pxmlobj);
                    ptail = ptail->GetNext();
                }
            }
        }
#endif
    }*/
    else 
    {
        pentryMT = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntryMT_LoadMovie(pentry, pMovieImpl);
        if (pentryMT)
        {
            LoadQueueEntry* asentry = static_cast<LoadQueueEntry*>(pentryMT->GetQueueEntry());
            LoadQueueEntryMT *ptail = pMovieImpl->pLoadQueueMTHead;
            while (ptail)    
            {
                LoadQueueEntry* astail = static_cast<LoadQueueEntry*>(ptail->GetQueueEntry());
                astail->Cancel(asentry);
                ptail = ptail->GetNext();
            }
        }
    }
    if (!pentryMT)
    {
        delete pentry;
        return;
    }
    pMovieImpl->AddLoadQueueEntryMT(pentryMT);
}

/*
void    MovieRoot::AddLoadQueueEntry(const char* ptarget, const char* purl, Environment* env,
                                     LoadQueueEntry::LoadMethod method,
                                     MovieClipLoader* pmovieClipLoader)
{
    GFxAS2LoadQueueEntry* pentry        = 0;
    int                   level         = -1;
    InteractiveObject*    ptargetChar   = env ? env->FindTarget(GetStringManager()->CreateString(ptarget)) : 
        static_cast<InteractiveObject*>(FindTarget(GetStringManager()->CreateString(ptarget)));
    Sprite*          ptargetSprite = ptargetChar ? ptargetChar->CharToSprite() : 0;  

    // If target leads to level, use level loading option.
    if (ptargetSprite)  
    {
        level = ToAvmSprite(ptargetSprite)->GetLevel();
        if (level != -1)
        {
            ptargetSprite = 0;
            ptargetChar   = 0;
        }       
    }

    // Otherwise, we have a real target.
    if (ptargetChar)
    {
        pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(ptargetChar->GetCharacterHandle(), purl, method);
    }
    else
    {
        // It must be a level or bad target. 
        if (level == -1)
        {
            // Decode level name.
            const char* ptail = "";
            level = ParseLevelName(ptarget, &ptail, GetLevelMovie(0)->IsCaseSensitive());
            if (*ptail != 0)
                level = -1; // path must end at _levelN.
        }

        if (level != -1)
        {
            pentry = SF_HEAP_NEW(GetMovieHeap()) GFxAS2LoadQueueEntry(level, purl, method);
        }
    }

    if (pentry) 
    {
        pentry->MovieClipLoaderHolder.SetAsObject(pmovieClipLoader);
        AddMovieLoadQueueEntry(pentry);
    }
}*/

// ****************************************************************************
// Adds load queue entry based on parsed url and target character 
// (Load/unload Movie)
//
void MovieRoot::AddNewLoadQueueEntry(Instances::URLRequest* urlRequest,
                                     Instances::Loader* loader,
                                     LoadQueueEntry::LoadMethod method)
{   
    SF_ASSERT(loader);

    LoadQueueEntry*pentry = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntry(urlRequest, loader, method);
    if (pentry)
    {
        bool    userImageProtocol = 0;

        // Check to see if URL is a user image substitute.
        // pentry->URL.GetLength() == 0 means we got here from unloadMovie
        if (pentry->URL.GetLength() > 0 && (pentry->URL[0] == 'i' || pentry->URL[0] == 'I'))
        {
            String urlLowerCase = pentry->URL.ToLower();

            if (urlLowerCase.Substring(0, 6) == "img://" || urlLowerCase.Substring(0, 8) == "imgps://")
                userImageProtocol = 1;
        }

        if ( pentry->URL.GetLength() > 0 && !userImageProtocol && pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

void MovieRoot::AddNewLoadQueueEntry(Instances::URLRequest* urlRequest,
                                     Instances::URLLoader* loader,
                                     LoadQueueEntry::LoadMethod method)
{
    SF_ASSERT(urlRequest && loader);

    LoadQueueEntry*pentry = SF_HEAP_NEW(GetMovieHeap()) LoadQueueEntry(urlRequest, loader, method);
    if (pentry)
    {
        pentry->Type = LoadQueueEntry::LT_LoadData;
        if (pentry->URL.GetLength() > 0 && pMovieImpl->GetTaskManager())
            AddLoadQueueEntryMT(pentry);
        else
            AddLoadQueueEntry(pentry);
    }
}

// Fills in a file system path relative to _level0.
bool MovieRoot::GetRootFilePath(String *ppath) const
{       
    if (!GetMainMovie())
    {
        ppath->Clear();
        return false;
    }

    // Get URL.
    *ppath = GetMovieDefImpl()->GetFileURL();

    // Extract path by clipping off file name.
    if (!URLBuilder::ExtractFilePath(ppath))
    {
        ppath->Clear();
        return false;
    }
    return true;
}

Text::Allocator* MovieRoot::GetTextAllocator()
{
    if (!MemContext->TextAllocator)
    {
        MemContext->TextAllocator = *SF_HEAP_NEW(MemContext->Heap) Text::Allocator(MemContext->Heap);
    }
    return MemContext->TextAllocator;
}

void MovieRoot::ProcessLoadQueueEntry(GFx::LoadQueueEntry *pentry, LoadStates* pls)
{
    LoadQueueEntry& entry = static_cast<LoadQueueEntry&>(*pentry);
    if (pentry->Type & LoadQueueEntry::LTF_VarsFlag)
    {
        // loading data using URLLoader
        bool userVarsProtocol = false;
        bool error = false;

        // @TODO logic to determine if a user protocol was used

        String data;
        int    fileLen = 0;
        if (userVarsProtocol)
        {
            // @TODO
        }
        else
        {
            String rootPath;
            GetRootFilePath(&rootPath);
            URLBuilder::LocationInfo loc(URLBuilder::File_LoadVars, pentry->URL, rootPath);
            String                   fileName;
            pls->BuildURL(&fileName, loc);

            // File loading protocol
            Ptr<File> pfile;
            pfile = *pls->OpenFile(fileName.ToCStr());
            if (pfile)
            {
                if (entry.mURLLoader->IsLoadingVariables())
                    MovieImpl::ReadLoadVariables(pfile, &data, &fileLen);
            }
            else
            {
                char buf[1024];
                SFsprintf(buf, sizeof(buf), "Can't open %s", fileName.ToCStr());
                entry.mURLLoader->ExecuteErrorEvent(buf);
                error = true;
            }
        }
        if (!error)
        {
            if (entry.mURLLoader->IsLoadingVariables())
            {
                entry.mURLLoader->SetVariablesDataString(data.ToCStr());
            }
            else {} //@TODO other dataTypes
            entry.mURLLoader->ExecuteOpenEvent();
            entry.mURLLoader->ExecuteProgressEvent(fileLen, fileLen);
            entry.mURLLoader->ExecuteCompleteEvent();
        }
    }
    else
    {
        // loading a movie/bitmap using Loader
        String              url(pentry->URL), urlStrGfx;
        bool                stripped = false;
        LogState*           plog = pls->GetLogState();
        Ptr<MovieDefImpl>   pmovieDef;
        unsigned            loadFlags = GetMovieDefImpl()->GetLoadFlags();
        int                 filelength = 0;

        if (GetMainMovie())
        {
            stripped = ((GetMainMovie()->GetResourceMovieDef()->GetSWFFlags()
                & MovieInfo::SWF_Stripped) != 0);
        }
        if (url.GetLength())
        {               
            bool    bilinearImage     = 0;
            bool    userImageProtocol = 0;

            // Check to see if URL is a user image substitute.
            if (url[0] == 'i' || url[0] == 'I')
            {
                String urlLowerCase = url.ToLower();

                if (urlLowerCase.Substring(0, 6) == "img://")
                    userImageProtocol = bilinearImage = 1;
                else if (urlLowerCase.Substring(0, 8) == "imgps://")
                    userImageProtocol = 1;
            }

            if (userImageProtocol)
            {
               //@IMG
//                 Ptr<ImageLoader> pimageLoader = pMovieImpl->pStateBag->GetImageLoader();
//                 // Create image through image callback
//                 Ptr<ImageResource> pimageRes =
//                     *LoaderImpl::LoadMovieImage(url, pimageLoader, plog, GetMovieHeap());
// 
//                 if (pimageRes)
//                 {
//                     pmovieDef = *pMovieImpl->CreateImageMovieDef(pimageRes, bilinearImage,
//                         url.ToCStr(), pls);
//                 }
//                 else 
//                 {
//                     if (plog)
//                     {
//                         plog->LogScriptWarning(
//                             "LoadMovieImageCallback failed to load image \"%s\"", url.ToCStr());
//                     }
//                     entry.mLoader->ExecuteErrorEvent(url.ToCStr());
//                 }
            }
            else
            {
                // Load the movie, first trying the .gfx file if necessary.
                // Our file loading can automatically detect and load images as well,
                // so we take advantage of that by passing the Loader::LoadImageFiles flag.

                // TBD: We will need to do something with delayed loading here in the future;
                // i.e. if the CreateMovie is executing in a different thread it will need to
                // post messages when it is done so that the handlers can be called.

                String rootPath;
                GetRootFilePath(&rootPath);

                unsigned lf = loadFlags | Loader::LoadImageFiles | Loader::LoadWaitCompletion;
                if (pentry->QuietOpen)
                    lf |= Loader::LoadQuietOpen;

                if (urlStrGfx.GetLength() > 0)
                {
                    URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                        urlStrGfx, rootPath);

                    pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
                }
                if (!pmovieDef)
                {
                    URLBuilder::LocationInfo loc(URLBuilder::File_LoadMovie,
                        url, rootPath);

                    pmovieDef = *LoaderImpl::CreateMovie_LoadState(pls, loc, lf);
                }

                if (pmovieDef)
                {
                    // Record file length for progress report below. Once we do
                    // threaded loading this will change.
                    filelength = pmovieDef->GetFileBytes();
                }
                else 
                {
                    if (plog && !pentry->QuietOpen)
                    {
                        plog->LogScriptWarning("Failed loading URL \"%s\"", url.ToCStr());
                    }
                    entry.mLoader->ExecuteErrorEvent(url.ToCStr());
                }
            }

            LoadedMovieDefs.Set(pmovieDef);

            entry.mLoader->ExecuteOpenEvent();

            DisplayObjContainer* pparent = entry.mLoader->GetDisplayObjContainer();
            ResourceId newCharId = ResourceId(); // Assign invalid id.
            Ptr<DisplayObject> pnewChar;
            if (pmovieDef && pmovieDef->GetDataDef()->MovieType == MovieDataDef::MT_Image)
            {
                // bitmap
                Ptr<AS3::AvmBitmap> pnewBmpChar = *static_cast<AS3::AvmBitmap*>(GetASSupport()->CreateCharacterInstance(
                    GetMovieImpl(), pmovieDef->GetDataDef(), 
                    pparent, newCharId, pmovieDef, CharacterDef::Bitmap));

                ResourceHandle rh;
                if (pmovieDef->GetDataDef()->GetResourceHandle(&rh, ResourceId(CharacterDef::CharId_ImageMovieDef_ImageResource)))
                {
                    Resource* pres = rh.GetResource(&pmovieDef->GetResourceBinding());
                    if (pres)
                    {
                        if (pres->GetResourceType() == Resource::RT_Image)
                        {
                            pnewBmpChar->SetImage(static_cast<ImageResource*>(pres));
                        }
                    }
                }

                ToAvmDisplayObjContainer(pparent)->AddChild(pnewBmpChar);
                pnewChar = pnewBmpChar;
            }
            else
            {
                // movie
                Ptr<Sprite> pnewSprite;
                newCharId = ResourceId(0); // root element always has 0 res id.
                if (pmovieDef)
                {
                    entry.mLoader->ExecuteProgressEvent(pmovieDef->GetFileBytes(), pmovieDef->GetFileBytes());
                    // CharId of loaded clip is the same as the old one
                    // Also mark clip as loadedSeparately, so that _lockroot works.
                    pnewSprite = *static_cast<Sprite*>(GetASSupport()->CreateCharacterInstance(
                        GetMovieImpl(), pmovieDef->GetDataDef(), 
                        NULL, newCharId, pmovieDef, CharacterDef::Sprite));
                    pnewSprite->SetLoadedSeparately();
                    pnewSprite->SetTimelineObjectFlag(true);
                    pnewSprite->OnEventLoad();

                    pnewSprite->AddToPlayList();
                    // Run actions on Frame 0. This should be done before calling OnLoadInit
                    pnewSprite->ExecuteFrame0Events();

                    ToAvmInteractiveObj(pnewSprite)->CreateASInstance(true);

                    // And attach a new character in place of an old one.
                    if (pnewSprite)
                    {
                        //AB: we need to replicate the same CreateFrame, Depth and the Name.
                        // Along with replicated Id this is important to maintain the loaded
                        // movie on the stage. Otherwise, once the parent clip reaches the end
                        // of its timeline the original movieclip will be recreated. Flash
                        // doesn't recreate the original movie clip in this case, and it also
                        // doesn't stop timeline animation for the target movieclip. The only way
                        // to achieve both of these features is to copy Id, CreateFrame, Depth and
                        // Name from the old character to the new one.
                        // See last_to_first_frame_mc_no_reloadX.fla and 
                        // test_timeline_anim_after_loadmovie.swf
                        //pnewSprite->SetCreateFrame(poldChar->GetCreateFrame());
                        //pnewSprite->SetDepth(poldChar->GetDepth());
                        //if (!poldChar->HasInstanceBasedName())
                        //    pnewSprite->SetName(poldChar->GetName());

                        ToAvmDisplayObjContainer(pparent)->AddChild(pnewSprite);
                        //pparent->ReplaceChildCharacter(poldChar, pnewSprite);
                        // In case permanent variables were assigned.. check them.
                        ResolveStickyVariables(pnewSprite);

                        // need to update the optimized play list
                        pnewSprite->ModifyOptimizedPlayListLocal<Sprite>();
                    }
                }
                bool charIsLoadedSuccessfully = (pnewSprite.GetPtr() != NULL);
                if (charIsLoadedSuccessfully)
                {
                    DoActions();
                } 
                pnewChar = pnewSprite;
            }
            entry.mLoader->ExecuteInitEvent(pnewChar);
            entry.mLoader->ExecuteCompleteEvent();
        } 
    }
}

void MovieRoot::ProcessLoadVarsMT
(GFx::LoadQueueEntry *pentry, LoadStates* , const String& data, UPInt fileLen, bool succeeded)
{
    LoadQueueEntry& entry = static_cast<LoadQueueEntry&>(*pentry);
    if (!succeeded)
    {
        char buf[1024];
        SFsprintf(buf, sizeof(buf), "Can't open %s", entry.mURLRequest->GetUrl().ToCStr());
        entry.mURLLoader->ExecuteErrorEvent(buf);
    }
    else
    {
        if (entry.mURLLoader->IsLoadingVariables())
        {
            entry.mURLLoader->SetVariablesDataString(data.ToCStr());
        }
        else {} //@TODO other dataTypes
        entry.mURLLoader->ExecuteOpenEvent();
        entry.mURLLoader->ExecuteProgressEvent(fileLen, fileLen);
        entry.mURLLoader->ExecuteCompleteEvent();
    }
}

/*
OnExceedLimit and OnFreeSegment are used to control MovieView's memory heap growth.
OnExceedLimit is called then the pre-set heap limit it exceeded.
OnFreeSegment is called then a chunk of memory is freed (not less than heap granularity).

User can set two parameters: UserLevelLimit (in bytes) and HeapLimitMultiplier (0..1].

The heap has initial pre-set limit 128K (so called "dynamic" limit). When this limit
is exceeded then OnExceedLimit is called. In this case it is necessary to determine:
either to collect to free up space, or to expand the heap.  The heuristic used to make the 
decision to collect or expand is taken from the Boehm-Demers-Weiser (BDW) garbage 
collector and memory allocator.
The BDW algorithm is (pseudo-code):

if (allocs since collect >= heap footprint * HeapLimitMultiplier)
collect
else
expand(heap footprint + overlimit + heap footprint * HeapLimitMultiplier)

The default value for HeapLimitMultiplier is 0.25.
Thus, it will collect only if allocated memory since the last collection is more
than 25% (default value of HeapLimitMultiplier) of the current heap size. Otherwise,
it will expand the limit up to requested size plus 25% of the heap size.

If collection occurred, it flushes all possible caches (text/paragraph formats, for example) and 
invoke garbage collector for ActionScript.

If user has specified UserLevelLimit then the algorithm above works the same way
up to that limit. If heap limit exceeds the UserLevelLimit then collection will be
invoked regardless to number of allocations since last collect. The dynamic heap limit
will be set to heap's footprint after collection + delta required for the requested allocation.

OnFreeSegment call reduces the limit by the freeing size.
*/
bool MemoryContextImpl::HeapLimit::OnExceedLimit(MemoryHeap* heap, UPInt overLimit)
{
    UPInt footprint = heap->GetFootprint();
    UPInt heapLimit = heap->GetLimit();
#ifdef SF_TRACE_COLLECTIONS        
    printf("\n! Limit is exceeded. Used: %u, footprint: %u, lim: %u, over limit: %u\n", 
        (unsigned)heap->GetUsedSpace(), (unsigned)footprint, (unsigned)heapLimit, (unsigned)overLimit);
#endif

    SPInt allocsSinceLastCollect = (SPInt)footprint - LastCollectionFootprint;
    UPInt newLimit = (heapLimit + overLimit) + (UPInt)(footprint * HeapLimitMultiplier);

    if (allocsSinceLastCollect >= (SPInt)(footprint * HeapLimitMultiplier) ||
        (UserLevelLimit != 0 && newLimit > UserLevelLimit))
    {
        Collect(heap);

        if (UserLevelLimit != 0 && newLimit > UserLevelLimit)
        {
            // check, if user limit is specified. If so, and if it is exceeded
            // then increase the limit just for absolutely required delta to minimize
            // the heap growth.
            SF_ASSERT(LastCollectionFootprint <= footprint);
            if (overLimit > (footprint - LastCollectionFootprint))
            {
                CurrentLimit = heapLimit + (overLimit - (footprint - LastCollectionFootprint));
                heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                printf("-        UserLimit exceeded. increasing limit up to: %u (%u)\n", 
                    (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                CurrentLimit = heap->GetLimit(); // take an actual value of the limit
            }
            else
            {
                // even though limit is not changed - set it to heap again to make sure
                // the acutual heap's limit is set correctly.
                heap->SetLimit(CurrentLimit);

#ifdef SF_TRACE_COLLECTIONS        
                printf("-        no limit increase is necessary. Current limit is %u (%u)\n", 
                    (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif

                CurrentLimit = heap->GetLimit(); // take an actual value of the limit
            }
        }
    }
    else
    {
        heap->SetLimit(newLimit);

#ifdef SF_TRACE_COLLECTIONS        
        printf("-    increasing limit up to: %u (%u)\n", (unsigned)newLimit, (unsigned)heap->GetLimit());
#endif

        CurrentLimit = heap->GetLimit(); // take an actual value of the limit
    }

    return true;
}
void MemoryContextImpl::HeapLimit::OnFreeSegment(MemoryHeap* heap, UPInt freeingSize)
{
    UPInt oldLimit = CurrentLimit;
    if (oldLimit > UserLevelLimit)
    {
        if (oldLimit > freeingSize)
        {
            CurrentLimit = oldLimit - freeingSize;
            heap->SetLimit(CurrentLimit);
#ifdef SF_TRACE_COLLECTIONS        
            printf("!!   reducing limit from %u to %u (%u)\n", (unsigned)oldLimit, (unsigned)CurrentLimit, (unsigned)heap->GetLimit());
#endif
        }
    }
}
void MemoryContextImpl::HeapLimit::Collect(MemoryHeap* heap)
{
    // The collection part.
    // We may release cached text format objects from Text::Allocator...
    if (MemContext->TextAllocator)
    {
        MemContext->TextAllocator->FlushTextFormatCache(true);
        MemContext->TextAllocator->FlushParagraphFormatCache(true);
    }
#ifdef GFX_AS_ENABLE_GC
    // If garbage collection is used - force collect. Use ForceEmergencyCollect - it
    // guarantees that no new allocations will be made during the collection.
    MemContext->ASGC->ForceEmergencyCollect();
#endif // SF_NO_GC
    LastCollectionFootprint = heap->GetFootprint(); 

#ifdef SF_TRACE_COLLECTIONS        
    printf("+    footprint after collection: %u, used mem: %u\n", 
        (unsigned)heap->GetFootprint(), (unsigned)heap->GetUsedSpace());
#endif
}
void MemoryContextImpl::HeapLimit::Reset(MemoryHeap* heap)
{
    Collect(heap);
    heap->SetLimit(INITIAL_DYNAMIC_LIMIT);   // reset to initial 128 
    CurrentLimit = heap->GetLimit(); // take an actual value of the limit
}


}}} // SF::GFx::AS3


