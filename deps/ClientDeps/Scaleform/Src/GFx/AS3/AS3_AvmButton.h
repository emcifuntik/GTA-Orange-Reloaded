/**********************************************************************

Filename    :   AS2_AvmButton.h
Content     :   Implementation of AS3-dependent part of Button character 
                and its AS3 Button class.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_AS3_AvmButton_H
#define INC_AS3_AvmButton_H

#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_Tags.h"
#include "GFx/GFx_ButtonDef.h"
#include "GFx/GFx_Button.h"
#include "GFx/AS3/AS3_AvmInteractiveObj.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

// This class represents AS2-dependent part of GFx::Button 
// character class.
class AvmButton : public AvmInteractiveObj, public AvmButtonBase
{
public:
    AvmButton(Button* pbutton);

    //*** AvmDisplayObjBase
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmInteractiveObj*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return this; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }
    virtual bool                        OnEvent(const EventId& id)
    {
        return AvmInteractiveObj::OnEvent(id);
    }
    virtual void                        OnEventLoad()
    {
        AvmInteractiveObj::OnEventLoad();
    }
    virtual void                        OnEventUnload()
    {
        AvmInteractiveObj::OnEventUnload();
    }
//     virtual void                        OnRemoved()
//     {
//         AvmInteractiveObj::OnRemoved();
//     }
    virtual bool                        OnUnloading(bool)
    {
        return true;
    }
    virtual bool                        HasEventHandler(const EventId& id) const
    {
        return AvmDisplayObj::HasEventHandler(id);
    }

    //*** AvmInteractiveObjBase
    virtual bool            ActsAsButton() const { return true; }
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmInteractiveObj::GetAbsolutePath(ppath);
    }
    virtual unsigned        GetCursorType() const;
    virtual void            CloneInternalData(const InteractiveObject* src)
    {
        AvmInteractiveObj::CloneInternalData(src);
    }
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmInteractiveObj::CopyPhysicalProperties(poldChar);
    }

    virtual bool            DoesAcceptMouseFocus() const
    {
        return true;
    }
    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }

    // invoked when item is going to get focus (Selection.setFocus is invoked, or TAB is pressed).
    virtual void            OnGettingKeyboardFocus(unsigned /*controllerIdx*/, FocusMovedType) {}
    // Special event handler; key down and up
    // See also PropagateKeyEvent, KeyMask
    virtual bool            OnKeyEvent(const EventId&, int*) { return false; }
    // invoked when focused item is about to lose keyboard focus input (mouse moved, for example)
    // if returns false, focus will not be transfered.
    virtual bool            OnLosingKeyboardFocus
        (InteractiveObject*, unsigned , FocusMovedType = GFx_FocusMovedByKeyboard) { return true; }

    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool            IsFocusEnabled() const  { return true; }
    virtual bool            IsTabable() const       { return AvmInteractiveObj::IsTabable(); }

    //*** AvmButtonBase
    virtual void            CreateCharacters();
    //virtual void ConstructCharacter(InteractiveObject*, const ButtonRecord*) =0;
    //virtual void            ConstructCharacter(InteractiveObject*, const ButtonRecord* rec);
    virtual bool            OnMouseEvent(const EventId& evt);
    virtual bool            OnButtonKeyEvent(const EventId& id, int* pkeyMask); 
    virtual void            SwitchState(ButtonRecord::MouseState);

    virtual const char*     GetDefaultASClassName() const { return "flash.display.SimpleButton"; }

    DisplayObject*          GetUpStateObject() const;
    void                    SetUpStateObject(DisplayObject* ch);

    DisplayObject*          GetOverStateObject() const;
    void                    SetOverStateObject(DisplayObject* ch);

    DisplayObject*          GetDownStateObject() const;
    void                    SetDownStateObject(DisplayObject* ch);

    DisplayObject*          GetHitStateObject() const;
    void                    SetHitStateObject(DisplayObject* ch);

private:
    ButtonDef*              GetButtonDef() const;
    Button*                 GetButton() const { return static_cast<Button*>(pDispObj); }
};

}}} // SF::GFx::AS3


#endif // INC_AS3_AvmButton_H

