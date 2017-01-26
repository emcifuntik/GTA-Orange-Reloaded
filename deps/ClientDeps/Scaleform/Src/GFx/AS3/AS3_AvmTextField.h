/**********************************************************************

Filename    :   AS2_AvmTextField.h
Content     :   Implementation of AS3 part of TextField character
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFx_AS3_AvmTextField_H
#define INC_SF_GFx_AS3_AvmTextField_H

#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "GFx/GFx_TextFieldDef.h"
#include "GFx/GFx_TextField.h"
#include "GFx/Text/Text_StyleSheet.h"

class TextFieldObject;

namespace Scaleform {
namespace GFx {
namespace AS3 {

// This class represents AS2-dependent part of GFx::TextField 
// character class.
class AvmTextField : public AvmInteractiveObj, public AvmTextFieldBase
{
#ifdef GFX_ENABLE_CSS
    struct CSSHolder : public TextField::CSSHolderBase
    {
    };
#endif //GFX_ENABLE_CSS
public:
    AvmTextField(TextField* pbutton);

    //*** AvmDisplayObjBase
    virtual const char* GetDefaultASClassName() const { return "flash.text.TextField"; }
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmInteractiveObj*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return this; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return NULL; }
    virtual bool                        OnEvent(const EventId& id)
    {
        return AvmDisplayObj::OnEvent(id);
    }
    virtual void                        OnEventLoad();
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

    //*** AvmInteractiveObjBase implementation
    virtual bool            ActsAsButton() const { return false; }
    virtual void            CloneInternalData(const InteractiveObject* src)
    {
        AvmInteractiveObj::CloneInternalData(src);
    }
    virtual void            CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmInteractiveObj::CopyPhysicalProperties(poldChar);
    }
    // should return true, if focus may be transfered to this character by clicking on
    // left mouse button. An example of such character is selectable textfield.
    virtual bool            DoesAcceptMouseFocus() const 
    { 
        return true; 
    }
    virtual const char*     GetAbsolutePath(String *ppath) const
    {
        return AvmInteractiveObj::GetAbsolutePath(ppath);
    }
    virtual unsigned        GetCursorType() const { return 0; }

    virtual void            OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
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
    virtual bool            IsTabable() const;

    // *** AvmTextFieldBase implementation
    virtual bool            HasStyleSheet() const;
    virtual void            NotifyChanged();
    virtual bool            OnMouseEvent(const EventId& event);
    // returns false, if default action should be prevented
    virtual bool            OnCharEvent(wchar_t wcharCode, unsigned controllerIdx);
    virtual void            OnScroll();
    virtual bool            UpdateTextFromVariable() { return false; }
    virtual void            UpdateVariable() {}

    // *** domestic
    const Text::StyleManager* GetStyleSheet() const;

    SF_INLINE TextField*      GetTextField() const
    {
        SF_ASSERT(pDispObj); 
        SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
        return static_cast<TextField*>(pDispObj);
    }
    TextFieldDef*           GetTextFieldDef() const;
    void                    SetDirtyFlag() { GetTextField()->SetDirtyFlag(); }
    void                    UpdateAutosizeSettings();
};

SF_INLINE AvmTextField* ToAvmTextField(InteractiveObject* po)
{
    SF_ASSERT(po->IsAVM1() && po->GetType() == CharacterDef::TextField);
    return static_cast<AvmTextField*>(static_cast<AvmInteractiveObj*>(po->GetAvmIntObj()));
}


}}} // SF::GFx::AS3

#endif //INC_SF_GFx_AS3_AvmTextField_H
