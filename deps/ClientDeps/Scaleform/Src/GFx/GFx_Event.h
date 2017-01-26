/**********************************************************************

PublicHeader:   GFx
Filename    :   Event.h
Content     :   Public interface for SWF Movie input events
Created     :   June 21, 2005
Authors     :   Michael Antonov

Notes       :   

Copyright   :   (c) 2005-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX__Event_H
#define INC_SF_GFX__Event_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Stats.h"
#include "Kernel/SF_Alg.h"
#include "Kernel/SF_KeyCodes.h"

#include <stdlib.h>
#include <string.h>

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class Event;
class MouseEvent;
class KeyEvent;
class CharEvent;
class KeyboardState;

// Forward declarations.
class InteractiveObject;

// ***** Debug check support

// This expression will only be avaluated in debug mode.
#ifdef SF_BUILD_DEBUG
#define SF_DEBUG_EXPR(arg) arg
#else
#define SF_DEBUG_EXPR(arg)
#endif

// ***** Event classes


// Event is a base class for input event notification. Objects of this type
// can be passed to Movie::HandleEvent to inform the movie of various input
// actions. Events that have specific subclasses, such as MouseEvent, must
// use those subclasses and NOT the base Event class.

class Event : public NewOverrideBase<Stat_Default_Mem>
{
public:
    enum EventType
    {
        Unknown,       
        // Informative events sent to the player.
        MouseMove,
        MouseDown,
        MouseUp,
        MouseWheel,
        KeyDown,
        KeyUp,
        SceneResize,
        SetFocus,
        KillFocus,

        // Action events, to be handled by user.
        DoShowMouse,
        DoHideMouse,
        DoSetMouseCursor,

        Char,
        IME
    };

    // What kind of event this is.
    EventType       Type;
    // State of special keys
    KeyModifiers    Modifiers;

    // Size of class, used in debug build to verify that
    // appropriate classes are used for messages.
    SF_DEBUG_EXPR(unsigned EventClassSize);


    Event(EventType eventType = Unknown)
    {
        Type = eventType;
        SF_DEBUG_EXPR(EventClassSize = sizeof(Event));
    }
    Event(EventType eventType, KeyModifiers keysState)
    {
        Type = eventType;
        Modifiers = keysState;
        SF_DEBUG_EXPR(EventClassSize = sizeof(Event));
    }
};


class MouseEvent : public Event
{
public:
    float    x, y;
    float    ScrollDelta;
    unsigned Button;
    unsigned MouseIndex;

    MouseEvent() : Event()
    {
        Button = 0; x = 0; y = 0; ScrollDelta = 0.0f; MouseIndex = 0;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
    // User event constructor - x, y must always be filled in.
    MouseEvent(EventType eventType, unsigned button,
        float xpos, float ypos, float scrollVal = 0.0f, unsigned mouseIdx = 0)
        : Event(eventType)
    {
        Button = button; x = xpos; y = ypos; ScrollDelta = scrollVal;
        MouseIndex = mouseIdx;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
    MouseEvent(EventType eventType, unsigned mouseIdx)
        : Event(eventType)
    {
        Button = 0; x = y = 0; ScrollDelta = 0;
        MouseIndex = mouseIdx;
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseEvent));
    }
};

class MouseCursorEvent : public Event
{
public:
    enum CursorShapeType
    {
        ARROW = 0,
        HAND  = 1,
        IBEAM = 2,
        BUTTON = 3
    };
    unsigned mCursorShape;
    unsigned MouseIndex;

    MouseCursorEvent() : Event(Event::DoSetMouseCursor), mCursorShape(HAND), 
        MouseIndex(0)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }
    MouseCursorEvent(CursorShapeType cursorShape, unsigned mouseIndex)
        : Event(Event::DoSetMouseCursor), mCursorShape(cursorShape), 
        MouseIndex(mouseIndex)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }
    MouseCursorEvent(EventType eventType, unsigned mouseIndex)
        : Event(eventType), mCursorShape(ARROW), 
        MouseIndex(mouseIndex)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(MouseCursorEvent));
    }

    CursorShapeType GetCursorShape() const { return (CursorShapeType)mCursorShape; }
};

class KeyEvent : public Event
{
public:
    // Key, and whether it was pressed up or down
    Key::Code     KeyCode;
    // ASCII code. 0, if unavailable.
    UByte       AsciiCode; // left for compatibility reasons only
    unsigned    WcharCode; // left for compatibility reasons only

    // The index of the physical keyboard controller.
    UInt8       KeyboardIndex;

    KeyEvent(EventType eventType = Unknown, Key::Code code = Key::None, 
             UByte asciiCode = 0, UInt32 wcharCode = 0,
             UInt8 keyboardIndex = 0)
        : Event(eventType), KeyCode (code), AsciiCode (asciiCode), WcharCode(wcharCode),
        KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(KeyEvent));
    }

    KeyEvent(EventType eventType, Key::Code code, UByte asciiCode, UInt32 wcharCode,
             KeyModifiers mods, UInt8 keyboardIndex = 0)
        : Event(eventType, mods), KeyCode (code), AsciiCode (asciiCode), WcharCode(wcharCode),
          KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(KeyEvent));
    }
};

class CharEvent : public Event
{
public:
    UInt32              WcharCode;
    // The index of the physical keyboard controller.
    UInt8               KeyboardIndex;

    CharEvent(UInt32 wcharCode, UInt8 keyboardIndex = 0)
        : Event(Char), WcharCode(wcharCode), KeyboardIndex(keyboardIndex)
    { 
        SF_DEBUG_EXPR(EventClassSize = sizeof(CharEvent));
    }
};

#ifdef GFX_ENABLE_KEYBOARD
// Keyboard state and queue class. 
class KeyboardState : public RefCountBase<KeyboardState, Stat_Default_Mem>
{
public:
    class IListener
    {
    public:
        virtual ~IListener() {}
        virtual void OnKeyDown(InteractiveObject *pmovie, UInt32 code, UByte ascii,
                               UInt32 wcharCode, UInt8 keyboardIndex) = 0;
        virtual void OnKeyUp(InteractiveObject *pmovie, UInt32 code, UByte ascii, 
                             UInt32 wcharCode, UInt8 keyboardIndex) = 0;
        virtual void Update(UInt32 code, UByte ascii, UInt32 wcharCode, 
                            UInt8 keyboardIndex)= 0;
    };

protected:
    IListener*          pListener;

    class KeyQueue 
    {
    private:
        enum { KeyQueueSize = 100 };
        struct KeyRecord 
        {
            UInt32              wcharCode;
            short               code;
            Event::EventType    event;
            UByte               ascii;
            KeyModifiers        keyMods;
        }               Buffer[KeyQueueSize];
        unsigned        PutIdx;
        unsigned        GetIdx;
        unsigned        Count;
    public:
        KeyQueue ();

        void Put(short code, UByte ascii, UInt32 wcharCode, Event::EventType event, KeyModifiers mods = 0);
        bool Get(short* code, UByte* ascii, UInt32* wcharCode, Event::EventType* event, KeyModifiers* mods = 0);

        bool IsEmpty () const { return Count == 0; }

        void ResetState();

    } KeyQueue;

    UInt8               KeyboardIndex;

    UByte               Keymap[Key::KeyCount / 8 + 1];   // bit-Array
    bool                Toggled[3];
public:

    KeyboardState();

    void    SetKeyboardIndex(UInt8 ki) { SF_ASSERT(ki < GFX_MAX_KEYBOARD_SUPPORTED); KeyboardIndex = ki; }
    UInt8   GetKeyboardIndex() const   { return KeyboardIndex; }

    bool    IsKeyDown(int code) const;
    bool    IsKeyToggled(int code) const;
    void    SetKeyToggled(int code, bool toggle);
    void    SetKeyDown(int code, UByte ascii, KeyModifiers mods = 0);
    void    SetKeyUp(int code, UByte ascii, KeyModifiers mods = 0);
    void    SetChar(UInt32 wcharCode);

    bool    IsQueueEmpty() const { return KeyQueue.IsEmpty(); }
    bool    GetQueueEntry(short* code, 
                          UByte* ascii, 
                          UInt32* wcharCode, 
                          Event::EventType* event, 
                          KeyModifiers* mods = 0);

    void    CleanupListener();
    void    SetListener(IListener* listener);

    void    NotifyListeners(InteractiveObject *pmovie, 
                            UInt32 code, 
                            UByte ascii, 
                            UInt32 wcharCode, 
                            Event::EventType event) const;
    void    UpdateListeners(UInt32 code, UByte ascii, UInt32 wcharCode);

    void    ResetState();

    KeyModifiers GetKeyModifiers() const;
};
#endif //GFX_ENABLE_KEYBOARD

class IMEEvent : public Event
{
public:
    enum IMEEventType
    {
        IME_Default,
        IME_PreProcessKeyboard
    } IMEEvtType;

    IMEEvent(IMEEventType t = IME_Default) : Event(IME), IMEEvtType(t)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEEvent));
    }
};

class IMEWin32Event : public IMEEvent
{
public:
    unsigned Message;  // Win32 message id (WM_<>)
    UPInt    WParam;
    UPInt    LParam;
    UPInt    hWND;
    int      Options;

    IMEWin32Event(IMEEventType t = IME_Default) : IMEEvent(t), 
        Message(0), WParam(0), LParam(0), hWND(0), Options(0)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEWin32Event));
    }
    IMEWin32Event(IMEEventType t, UPInt hwnd, unsigned win32msg, UPInt wp, UPInt lp, int options = 0) : IMEEvent(t),
        Message(win32msg), WParam(wp), LParam(lp), hWND(hwnd), Options(options)
    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEWin32Event));
    }
};

class IMEXBox360Event : public IMEEvent
{
public:
    // Unpacked XINPUT_KEYSTROKE structure
    UInt16   VirtualKey;
    wchar_t  Unicode;
    UInt16   Flags;
    UInt8    UserIndex;
    UInt8    HidCode;
    void*    pLastKey;
    unsigned eType; 

    IMEXBox360Event(UInt16 virtualKey, wchar_t unicode, UInt16 flags, UInt8 userIndex, UInt8 hidCode, void* plastKey,
        unsigned etype) : IMEEvent(IME_Default), VirtualKey(virtualKey), Unicode(unicode), Flags(flags),
        UserIndex(userIndex), HidCode(hidCode), pLastKey(plastKey), eType(etype)

    {
        SF_DEBUG_EXPR(EventClassSize = sizeof(IMEXBox360Event));
    }
};

class IMEPS3Event : public IMEEvent
{
public:

    IMEPS3Event(): KeyCode(0), CharCode(0), Modifiers(0), LastKeyCode(0), LastCharCode(0), LastModifiers(0){};
    IMEPS3Event(unsigned keyCode, unsigned code, unsigned mods, unsigned lastKeyCode, unsigned lastCharCode, unsigned lastMods)
    {
        KeyCode         = keyCode;
        CharCode        = code;
        Modifiers       = mods;
        LastKeyCode     = lastKeyCode;
        LastCharCode    = lastCharCode;
        LastModifiers   = lastMods;
    };
    unsigned KeyCode;
    unsigned CharCode;
    unsigned Modifiers;
    mutable unsigned LastKeyCode;
    mutable unsigned LastCharCode;
    mutable unsigned LastModifiers; 
};

class IMEMacEvent : public IMEEvent
{
public:

    IMEMacEvent(): compositionString(NULL), attributes(NULL), eType(0){};
    IMEMacEvent(char* compString, char* attrib, unsigned etype)
    {
        compositionString   = compString;
        attributes          = attrib;
        eType               = etype;
    };

    char*     compositionString;
    char*     attributes;
    unsigned  eType;
};

// Event to use when movie's window is getting focus. Special key state
// should be provided in order to maintain correct state of keys such as
// CapsLock, NumLock, ScrollLock.
class SetFocusEvent : public Event
{
public:
    // State of special keys
    KeyModifiers KeysStates[GFX_MAX_KEYBOARD_SUPPORTED];

    SetFocusEvent(KeyModifiers specialKeysState) : 
      Event(SetFocus, specialKeysState) {}

      SetFocusEvent(unsigned numKeyboards, KeyModifiers* specialKeysStates) 
          : Event(SetFocus) 
      {
          for (unsigned i = 0, n = Alg::Min(unsigned(GFX_MAX_KEYBOARD_SUPPORTED), numKeyboards); i < n; ++i)
              KeysStates[i] = specialKeysStates[i];
      }
};

}} // Scaleform::GFx

#endif
