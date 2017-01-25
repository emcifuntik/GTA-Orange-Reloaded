/**********************************************************************

Filename    :   GFxInput.h
Content     :   Input handling
Created     :   
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   This file contains class declarations used in
GFxPlayerImpl.cpp only. Declarations that need to be
visible by other player files should be placed
in DisplayObject.h.


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_SF_GFX_Input_H
#define INC_SF_GFX_Input_H

#include "GFx/GFx_InteractiveObject.h"

#define GFX_MAX_MICE_SUPPORTED 4

#define GFX_DOUBLE_CLICK_TIME_MS 300  // in ms


namespace Scaleform { namespace GFx {

// ****

class InputEventsQueueEntry
{
public:
    typedef UInt16 ButtonsStateType;
    enum MouseButtonMask
    {
        MouseButton_Left   = 1,
        MouseButton_Right  = 2,
        MouseButton_Middle = 4,

        MouseButton_MaxNum = (sizeof(ButtonsStateType)*8),
        MouseButton_LastButton = (1 << (MouseButton_MaxNum - 1)),   // 0x8000
        MouseButton_AllMask    = ((1 << MouseButton_MaxNum) - 1)    // 0xFFFF
    };
    enum MouseButtonFlags
    {
        MouseButton_Released = 0x80,  // indicates mouse button released
        MouseButton_Move     = 0x40,  // indicates mouse moved
        MouseButton_Wheel    = 0x20
    };

    enum QueueEntryType
    {
        QE_Mouse,
        QE_Key
    } t;
    struct MouseEntry
    {
        float               PosX, PosY;     // mouse coords, in twips, when button event happened
        ButtonsStateType    ButtonsState; // bit-mask for buttons (see MouseButtonMask)
        SInt8               WheelScrollDelta; // delta for mouse wheel
        UInt8               Flags;        // see MouseButtonFlags
        UInt8               MouseIndex;

        PointF GetPosition() const   { return PointF(PosX, PosY); }
        unsigned    GetMouseIndex() const { return MouseIndex; }

        bool IsButtonsStateChanged() const  { return ButtonsState != 0; }
        bool IsAnyButtonPressed() const  
        { 
            return !(Flags & (MouseButton_Move | MouseButton_Released)) && 
                ButtonsState != 0; 
        }
        bool IsAnyButtonReleased() const 
        { 
            return !(Flags & MouseButton_Move) && 
                (Flags & MouseButton_Released) && ButtonsState != 0; 
        }
        bool IsMoved() const          { return (Flags & MouseButton_Move) != 0; }
        bool IsLeftButton() const     { return (ButtonsState & MouseButton_Left) != 0; }
        bool IsRightButton() const    { return (ButtonsState & MouseButton_Right) != 0; }
        bool IsMiddleButton() const   { return (ButtonsState & MouseButton_Middle) != 0; }
        // button is specified as index: 0 - left, 1 - right, 2 - middle, 3 - .... etc
        bool IsIndexedButton(unsigned index) const { return (ButtonsState & (1 << index)) != 0; }
        // buttons are specified as a bit mask
        bool AreButtons(unsigned mask) const { return (ButtonsState & mask) != 0; }
        bool IsMouseWheel() const     { return (Flags & MouseButton_Wheel) != 0; }
    };
    struct KeyEntry
    {
        UInt32              WcharCode;
        UInt32              Code;
        UByte               AsciiCode;
        UInt8               KeysState;
        UInt8               KeyboardIndex;
        bool                KeyIsDown;
    };
    union Entry
    {
        MouseEntry mouseEntry;
        KeyEntry   keyEntry;
    } u;

    bool IsMouseEntry() const { return t == QE_Mouse; }
    bool IsKeyEntry() const { return t == QE_Key; }

    MouseEntry& GetMouseEntry()             { return u.mouseEntry; }
    const MouseEntry& GetMouseEntry() const { return u.mouseEntry; }

    KeyEntry& GetKeyEntry()             { return u.keyEntry; }
    const KeyEntry& GetKeyEntry() const { return u.keyEntry; }
};

// This class provides queue for input events, such as keyboard and mouse (mice)
// events
class InputEventsQueue
{
public:
    typedef InputEventsQueueEntry                    QueueEntry;
    typedef InputEventsQueueEntry::ButtonsStateType  ButtonsStateType;
    typedef InputEventsQueueEntry::MouseButtonMask   MouseButtonMask;
    typedef InputEventsQueueEntry::MouseButtonFlags  MouseButtonFlags;
protected:
    enum
    {
        Queue_Length = 100
    };
    QueueEntry  Queue[Queue_Length];
    UPInt       StartPos;
    UPInt       UsedEntries;
    PointF      LastMousePos[GFX_MAX_MICE_SUPPORTED];
    UInt32      LastMousePosMask;

    const QueueEntry* PeekLastQueueEntry() const; 
    QueueEntry* PeekLastQueueEntry()
    {
        return const_cast<QueueEntry*>(const_cast<const InputEventsQueue*>(this)->PeekLastQueueEntry());
    }
    QueueEntry* AddEmptyQueueEntry();
    bool IsAnyMouseMoved() const;
public:
    InputEventsQueue();

    SF_INLINE bool IsQueueEmpty() const { return UsedEntries == 0 && LastMousePosMask == 0; }
    const QueueEntry* GetEntry();
    void ResetQueue() { UsedEntries = 0; }

    // mouse related methods
    void AddMouseMove(unsigned mouseIndex, const PointF& pos); 
    void AddMouseButtonEvent(unsigned mouseIndex, const PointF& pos, unsigned buttonsSt, unsigned flags);
    void AddMouseWheel(unsigned mouseIndex, const PointF& pos, int delta);

    SF_INLINE void AddMousePress(unsigned mouseIndex, const PointF& pos, unsigned buttonsSt)
    {   
        AddMouseButtonEvent(mouseIndex, pos, buttonsSt, 0);
    }
    SF_INLINE void AddMouseRelease(unsigned mouseIndex, const PointF& pos, unsigned buttonsSt)
    {
        AddMouseButtonEvent(mouseIndex, pos, buttonsSt, QueueEntry::MouseButton_Released);
    }

    // keyboard related events
    void AddKeyEvent(UInt32 code, UByte ascii, UInt32 wcharCode, bool isKeyDown, 
                     KeyModifiers specialKeysState = 0, UInt8 keyboardIndex = 0);
    void AddKeyDown(UInt32 code, UByte ascii, KeyModifiers specialKeysState = 0,
                    UInt8 keyboardIndex = 0)
    {
        AddKeyEvent(code, ascii, 0, true, specialKeysState, keyboardIndex);
    }
    void AddKeyUp  (UInt32 code, UByte ascii, KeyModifiers specialKeysState = 0, 
        UInt8 keyboardIndex = 0)
    {
        AddKeyEvent(code, ascii, 0, false, specialKeysState, keyboardIndex);
    }
    void AddCharTyped(UInt32 wcharCode, UInt8 keyboardIndex = 0)
    {
        AddKeyEvent(0, 0, wcharCode, true, 0, keyboardIndex);
    }
};


// ***** MouseState
// This class keeps mouse states and queue of mouse events, such as down/up, wheel, move.
// Also, this class tracks topmost entities for generating button events later.
// If player supports more than one mouse, then implementation should contain
// corresponding number of MouseState's instances.
class MouseState
{
public:
    typedef UInt16 ButtonsStateType;
    enum MouseButtonMask
    {
        MouseButton_Left   = 1,
        MouseButton_Right  = 2,
        MouseButton_Middle = 4,

        MouseButton_MaxNum = (sizeof(ButtonsStateType)*8),
        MouseButton_LastButton = (1 << (MouseButton_MaxNum - 1)),   // 0x8000
        MouseButton_AllMask    = ((1 << MouseButton_MaxNum) - 1)    // 0xFFFF
    };
    enum MouseButtonFlags
    {
        MouseButton_Released = 0x80,  // indicates mouse button released
        MouseButton_Move     = 0x40,  // indicates mouse moved
        MouseButton_Wheel    = 0x20
    };
protected:
    mutable WeakPtr<InteractiveObject>    TopmostEntity;  // what's underneath the mouse right now
    mutable WeakPtr<InteractiveObject>    PrevTopmostEntity; // previous topmost entity
    mutable WeakPtr<InteractiveObject>    ActiveEntity;   // entity that currently owns the mouse pointer    
    mutable WeakPtr<InteractiveObject>    MouseButtonDownEntity; // if the mouse button is pressed, the active entity when the mouse button was pressed.

    unsigned    CurButtonsState;  // current mouse buttons state, pressed buttons marked by bits set
    unsigned    PrevButtonsState; // previous mouse buttons state, pressed buttons marked by bits set
    PointF      LastPosition;

    unsigned    mPresetCursorType;  // preset type of cursor; ~0u, if there is none.
    unsigned    CursorType;         // current type of cursor
    int         WheelDelta;

    bool        TopmostEntityIsNull           :1; // indicates if topmost ent is null or not
    bool        PrevTopmostEntityWasNull      :1;
    bool        MouseInsideEntityLast         :1;
    bool        MouseMoved                    :1; // indicates mouse was moved
    bool        Activated                     :1; // is mouse activated

public:
    MouseState();

    void ResetState();

    void UpdateState(const InputEventsQueue::QueueEntry&);

    SF_INLINE bool IsMouseMoved() const  { return MouseMoved; }

    void     SetCursorType(unsigned curs) 
    { 
        CursorType = (mPresetCursorType == ~0u) ? curs : mPresetCursorType;
    }
    unsigned GetCursorType() const       { return CursorType; }
    void     PresetCursorType(unsigned curs) 
    { 
        CursorType = mPresetCursorType = curs;
    }
    unsigned GetPresetCursorType() const { return mPresetCursorType; }

    PointF GetLastPosition() const 
    { 
        return LastPosition;
    }
    unsigned GetButtonsState() const { return CurButtonsState; }
    unsigned GetPrevButtonsState() const { return PrevButtonsState; }

    Ptr<InteractiveObject> GetActiveEntity() const  
    { 
        return *const_cast<WeakPtr<InteractiveObject>* >(&ActiveEntity); 
    }
    Ptr<InteractiveObject> GetTopmostEntity() const 
    { 
        return *const_cast<WeakPtr<InteractiveObject>* >(&TopmostEntity); 
    }
    Ptr<InteractiveObject> GetMouseButtonDownEntity() const  
    { 
        return *const_cast<WeakPtr<InteractiveObject>* >(&MouseButtonDownEntity); 
    }

    void SetActiveEntity(InteractiveObject* pch)
    {
        ActiveEntity = pch;
    }
    void SetTopmostEntity(InteractiveObject* pch)
    {
        PrevTopmostEntity           = TopmostEntity;
        PrevTopmostEntityWasNull    = TopmostEntityIsNull;
        TopmostEntity               = pch;
        TopmostEntityIsNull         = (pch == NULL);
    }
    void SetMouseButtonDownEntity(InteractiveObject* pch)
    {
        MouseButtonDownEntity = pch;
    }
    bool IsTopmostEntityChanged() const
    {
        Ptr<InteractiveObject> ptop  = TopmostEntity;
        Ptr<InteractiveObject> pprev = PrevTopmostEntity;
        return (ptop != pprev) ||
            (!ptop && !TopmostEntityIsNull) ||
            (!pprev && !PrevTopmostEntityWasNull);
    }
    void SetMouseInsideEntityLast(bool f) { MouseInsideEntityLast = f; }
    bool IsMouseInsideEntityLast() const { return MouseInsideEntityLast; }

    bool IsActivated() const { return Activated; }

    void ResetPrevButtonsState() { PrevButtonsState = CurButtonsState; }
    void ResetMouseMovedState() { MouseMoved = false; }

    int GetWheelDelta() const { return WheelDelta; }
};

// ***** EventId

// Describes various ActionScript events that can be fired of and that
// users can install handlers for through on() and onClipEvent() handlers.
// Events include button and movie clip mouse actions, keyboard handling,
// load & unload notifications, etc. 

class EventId
{
public:
    // These must match the function names in EventId::GetFunctionName()
    enum IdCode
    {
        Event_Invalid,

        // These are for buttons & sprites.
        Event_Load              = 0x000001,
        Event_EnterFrame        = 0x000002,
        Event_Unload            = 0x000004,
        Event_MouseMove         = 0x000008,
        Event_MouseDown         = 0x000010,
        Event_MouseUp           = 0x000020,
        Event_KeyDown           = 0x000040,
        Event_KeyUp             = 0x000080,
        Event_Data              = 0x000100,
        Event_Initialize        = 0x000200,

        // button events
        Event_Press             = 0x000400,
        Event_Release           = 0x000800,
        Event_ReleaseOutside    = 0x001000,
        Event_RollOver          = 0x002000,
        Event_RollOut           = 0x004000,
        Event_DragOver          = 0x008000,
        Event_DragOut           = 0x010000,
        Event_KeyPress          = 0x020000,

        Event_Construct         = 0x040000,

        Event_PressAux          = 0x080000,
        Event_ReleaseAux        = 0x100000,
        Event_ReleaseOutsideAux = 0x200000,
        Event_DragOverAux       = 0x400000,
        Event_DragOutAux        = 0x800000,

        Event_AuxEventMask      = 0xF80000,
        Event_ButtonEventsMask  = 0xFBFC00,

        //Event_LastCombined      = Event_Construct,
        Event_LastCombined      = Event_DragOutAux,
        Event_NextAfterCombined = (Event_LastCombined << 1),

        // These are for the MoveClipLoader ActionScript only
        Event_LoadStart         = Event_NextAfterCombined,
        Event_LoadError,
        Event_LoadProgress,
        Event_LoadInit,

        // These are for the XMLSocket ActionScript only
        Event_SockClose,
        Event_SockConnect,
        Event_SockData,
        Event_SockXML,

        // These are for the XML ActionScript only
        Event_XMLLoad,
        Event_XMLData,

        Event_MouseOver,
        Event_MouseOut,
        Event_Click,
        Event_DoubleClick,
        Event_MouseWheel,
        Event_Added,
        Event_Removed,
        Event_Activate,
        Event_Deactivate,
        Event_Render,
		Event_Resize,

        Event_End,
        Event_COUNT = 20 + (Event_End - Event_NextAfterCombined) + 5  // PPS: 5 for Aux events
    };

    UInt32              Id;
    UInt32              WcharCode;
    UInt32              KeyCode;
    union
    {
        UByte           AsciiCode;
        UInt8           ButtonId;
    };
    UInt8               RollOverCnt;
    union
    {
        SInt8           ControllerIndex;
        SInt8           MouseIndex;
        SInt8           KeyboardIndex;
    };
    KeyModifiers    KeysState;
    SInt8               MouseWheelDelta;

    EventId() : Id(Event_Invalid), WcharCode(0), KeyCode(Key::None),AsciiCode(0),
        RollOverCnt(0), MouseWheelDelta(0) { ControllerIndex = -1; }

    EventId(UInt32 id)
        : Id(id), WcharCode(0), KeyCode(Key::None), AsciiCode(0), 
        RollOverCnt(0), MouseWheelDelta(0)
    {
        ControllerIndex = -1;
        // For the button key events, you must supply a keycode.
        // Otherwise, don't.
        //!SF_ASSERT((KeyCode == Key::None && (Id != Event_KeyPress)) || 
        //!     (KeyCode != Key::None && (Id == Event_KeyPress)));
    }

    EventId(UInt32 id, UInt32 c, UByte ascii, UInt32 wcharCode = 0,
        UInt8 keyboardIndex = 0)
        : Id(id), WcharCode(wcharCode), KeyCode(c), AsciiCode(ascii), 
        RollOverCnt(0), MouseWheelDelta(0)
    {
        ControllerIndex = (UInt8)keyboardIndex;
        // For the button key events, you must supply a keycode.
        // Otherwise, don't.
        //!SF_ASSERT((KeyCode == Key::None && (Id != Event_KeyPress)) || 
        //!     (KeyCode != Key::None && (Id == Event_KeyPress)));
    }

    SF_INLINE bool IsButtonEvent() const
    { return (Id & Event_ButtonEventsMask) != 0; }

    SF_INLINE bool IsKeyEvent() const
    { return (Id & Event_KeyDown) || (Id & Event_KeyUp); }

    bool    operator==(const EventId& id) const 
    { 
        return Id == id.Id && (!(Id & Event_KeyPress) || (KeyCode == id.KeyCode)); 
    }

    EventId& operator=(const EventId& o)
    {
        Id              = o.Id;
        WcharCode       = o.WcharCode;
        KeyCode         = o.KeyCode;
        AsciiCode       = o.AsciiCode;
        RollOverCnt     = o.RollOverCnt;
        ControllerIndex = o.ControllerIndex;
        KeysState       = o.KeysState;
        MouseWheelDelta = o.MouseWheelDelta;
        return *this;
    }

    // Return the name of a method-handler function corresponding to this event.
    //ASString           GetFunctionName(ASStringManager *psm) const;
    //ASBuiltinType      GetFunctionNameBuiltinType() const;

    // converts keyCode/asciiCode from this event to the on(keyPress <>) format
    int                 ConvertToButtonKeyCode () const;

    UPInt               HashCode() const
    {
        UPInt hash = Id;
        if (Id & Event_KeyPress) 
            hash ^= KeyCode;
        return hash;
    }

    unsigned GetEventsCount() const { return Alg::Cardinality(Id); }
};

// This event id should be used for button events, like onRollOver, onRollOut, etc
class ButtonEventId : public EventId
{
public:
    ButtonEventId(UInt32 id, int mouseIndex = -1, unsigned rollOverCnt = 0, UInt8 buttonId = 0) :
      EventId(id)
      {
          ButtonId    = buttonId;
          MouseIndex  = (SInt8)mouseIndex;
          RollOverCnt = (UInt8)rollOverCnt;
      }
};

struct EventIdHashFunctor
{
    UPInt operator()(const EventId& data) const 
    { 
        return data.HashCode();
    }
};


}} // namespace Scaleform::GFx

#endif // INC_SF_GFX_Input_H

