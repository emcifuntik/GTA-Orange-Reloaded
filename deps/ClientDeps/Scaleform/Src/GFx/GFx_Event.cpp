/**********************************************************************

Filename    :   Event.cpp
Content     :   
Created     :   
Authors     :   

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include <string.h>
#include "GFx_Event.h"

namespace Scaleform { namespace GFx {

#ifdef GFX_ENABLE_KEYBOARD

KeyboardState::KeyQueue::KeyQueue ()
{
    ResetState();
}

void KeyboardState::KeyQueue::ResetState()
{
    PutIdx = 0;
    GetIdx = 0;
    Count = 0;
    memset (&Buffer, 0, sizeof(Buffer));
}

void KeyboardState::KeyQueue::Put (short code, UByte ascii, UInt32 wcharCode, Event::EventType event, KeyModifiers specialKeysState) 
{
    if (Count < KeyQueueSize)
    {
        Buffer[PutIdx].code             = code;
        Buffer[PutIdx].ascii            = ascii;
        Buffer[PutIdx].wcharCode        = wcharCode;
        Buffer[PutIdx].event            = event;
        Buffer[PutIdx].keyMods = specialKeysState;
        if (++PutIdx >= KeyQueueSize)
            PutIdx = 0;
        ++Count;
    }
}
bool KeyboardState::KeyQueue::Get (short* code, UByte* ascii, UInt32* wcharCode, Event::EventType* event, KeyModifiers* specialKeysState)
{
    if (Count > 0) 
    {
        *code               = Buffer[GetIdx].code;
        *ascii              = Buffer[GetIdx].ascii;
        *wcharCode          = Buffer[GetIdx].wcharCode;
        *event              = Buffer[GetIdx].event;
        if (specialKeysState) // specialKeysState is optional
            *specialKeysState   = Buffer[GetIdx].keyMods;
        if (++GetIdx >= KeyQueueSize)
            GetIdx = 0;
        --Count;
        return true;
    }
    return false;
}

KeyboardState::KeyboardState()
{
    pListener = NULL;
    KeyboardIndex = 0;
    memset(Keymap, 0, sizeof(Keymap));
    memset(Toggled, 0, sizeof(Toggled));
}

void KeyboardState::ResetState()
{
    KeyQueue.ResetState();
    memset(Keymap, 0, sizeof(Keymap));
    memset(Toggled, 0, sizeof(Toggled));
}

bool KeyboardState::IsKeyDown(int code) const
{
    if (code < 0 || code >= Key::KeyCount) return false;

    int ByteIndex = code >> 3;
    int BitIndex = code - (ByteIndex << 3);
    int mask = 1 << BitIndex;

    SF_ASSERT(ByteIndex >= 0 && ByteIndex < int(sizeof(Keymap)/sizeof(Keymap[0])));

    if (Keymap[ByteIndex] & mask)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool KeyboardState::IsKeyToggled(int code) const
{
    switch (code)
    {
        case Key::NumLock:
            return Toggled[0];
        case Key::CapsLock:
            return Toggled[1];
        case Key::ScrollLock:
            return Toggled[2];
    }
    return false;
}

KeyModifiers KeyboardState::GetKeyModifiers() const
{
    KeyModifiers mods;
    mods.SetAltPressed(IsKeyDown(Key::Alt));
    mods.SetCtrlPressed(IsKeyDown(Key::Control));
    mods.SetShiftPressed(IsKeyDown(Key::Shift));
    mods.SetNumToggled(Toggled[0]);
    mods.SetCapsToggled(Toggled[1]);
    mods.SetScrollToggled(Toggled[2]);
    return mods;
}

void KeyboardState::SetKeyToggled(int code, bool toggle)
{
    switch (code)
    {
        case Key::NumLock:
            Toggled[0] = toggle;
            break;
        case Key::CapsLock:
            Toggled[1] = toggle;
            break;
        case Key::ScrollLock:
            Toggled[2] = toggle;
            break;
    }
}

void KeyboardState::SetKeyDown(int code, UByte ascii, KeyModifiers mods)
{
    if (code < 0 || code >= Key::KeyCount) return;

    int ByteIndex = code >> 3;
    int BitIndex = code - (ByteIndex << 3);
    int mask = 1 << BitIndex;

    SF_ASSERT(ByteIndex >= 0 && ByteIndex < int(sizeof(Keymap)/sizeof(Keymap[0])));

    Keymap[ByteIndex] |= mask;

    KeyQueue.Put(short(code), ascii, 0, Event::KeyDown, mods);
}

void KeyboardState::SetKeyUp(int code, UByte ascii, KeyModifiers mods)
{
    if (code < 0 || code >= Key::KeyCount) return;

    int ByteIndex = code >> 3;
    int BitIndex = code - (ByteIndex << 3);
    int mask = 1 << BitIndex;

    SF_ASSERT(ByteIndex >= 0 && ByteIndex < int(sizeof(Keymap)/sizeof(Keymap[0])));

    Keymap[ByteIndex] &= ~mask;

    KeyQueue.Put (short(code), ascii, 0, Event::KeyUp, mods);
}

void KeyboardState::SetChar(UInt32 wcharCode)
{
    KeyQueue.Put(0, 0, wcharCode, Event::Char, 0);
}

bool KeyboardState::GetQueueEntry
(short* code, UByte* ascii, UInt32* wcharCode, Event::EventType* event, KeyModifiers* mods)
{
    return KeyQueue.Get (code, ascii, wcharCode, event, mods);
}

void KeyboardState::NotifyListeners
(InteractiveObject *pmovie, UInt32 code, UByte ascii, UInt32 wcharCode, Event::EventType event) const
{
    // notify listeners
    // Do we need mods here? (AB)
    if (pListener)
    {
        if (event == Event::KeyDown) 
            pListener->OnKeyDown(pmovie, code, ascii, wcharCode, KeyboardIndex);
        else if (event == Event::KeyUp) 
            pListener->OnKeyUp(pmovie, code, ascii, wcharCode, KeyboardIndex);
    }
}

void KeyboardState::UpdateListeners(UInt32 code, UByte ascii, UInt32 wcharCode)
{
    // notify listeners
    // Do we need mods here? (AB)
    if (pListener)
        pListener->Update(code, ascii, wcharCode, KeyboardIndex);
}

void KeyboardState::CleanupListener()
// Remove dead entries in the listeners list.  (Since
// we use WeakPtr's, listeners can disappear without
// notice.)
{
    pListener = NULL;
}

void KeyboardState::SetListener(IListener* listener)
{
    CleanupListener();
    pListener = listener;
}

#endif //GFX_ENABLE_KEYBOARD

}} // Scaleform::GFx
