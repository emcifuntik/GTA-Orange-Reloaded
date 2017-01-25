/**********************************************************************

Filename    :   AvmDisplayObjContainer.h
Content     :   Implementation of AS3-dependent part of DisplayObjectContainer.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_AS3_AvmDisplayObjContainer_H
#define INC_AS3_AvmDisplayObjContainer_H

#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_DisplayObjContainer.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

class MovieRoot;

class AvmDisplayObjContainer : 
    public AvmInteractiveObj,
    public AvmDisplayObjContainerBase
{
public:
    AvmDisplayObjContainer(DisplayObjContainer* pobj) 
        : AvmInteractiveObj(pobj) {}
    virtual ~AvmDisplayObjContainer();

    //***** AvmDisplayObjBase implementation
    virtual class AvmInteractiveObjBase* ToAvmInteractiveObjBase() 
    { 
        return static_cast<AvmInteractiveObj*>(this); 
    }
    virtual class AvmSpriteBase*        ToAvmSpriteBase() { return NULL; }
    virtual class AvmButtonBase*        ToAvmButttonBase() { return NULL; }
    virtual class AvmTextFieldBase*     ToAvmTextFieldBase() { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase() { return this; }
    virtual void                        OnEventLoad()
    {
        AvmInteractiveObj::OnEventLoad();
    }
    virtual void                        OnEventUnload()
    {
        AvmInteractiveObj::OnEventUnload();
    }
    virtual bool                        OnUnloading(bool mayRemove)
    {
        return AvmInteractiveObj::OnUnloading(mayRemove);
    }

    //***** AvmInteractiveObjectBase implementation
    virtual bool                        ActsAsButton() const
    {
        return false;
    }
    virtual bool                        IsLevelMovie() const { return false; }
    virtual void                        ForceShutdown() {}
    virtual unsigned                    GetCursorType() const 
    {
        return AvmInteractiveObj::GetCursorType();
    }

    virtual void                        ExecuteInitActionFrameTags(unsigned) {}
    // Used for root-level sprites.
    virtual void                        ExecuteFrame0Events() {}
    virtual void                        ExecuteFrameTags(unsigned) {}
    virtual void                        CloneInternalData(const InteractiveObject* ) {}

    virtual bool                        HasEventHandler(const EventId& ) const { return false; }

    virtual bool                        OnEvent(const EventId& id) 
    { 
        return AvmInteractiveObj::OnEvent(id); 
    }
    virtual void                        FillTabableArray(InteractiveObject::FillTabableParams* params);
    //virtual bool            IsTabable() const = 0;
    // returns true, if yellow focus rect is enabled for the character
    //    virtual bool            IsFocusRectEnabled() const =0;
    // should return true, if focus may be set to this character by keyboard or
    // ActionScript.
    virtual bool                        IsFocusEnabled() const { return false; }

    virtual void                        CopyPhysicalProperties(const InteractiveObject *poldChar)
    {
        AvmInteractiveObj::CopyPhysicalProperties(poldChar);
    }
    // Determines the absolute path of the character.
    virtual const char*                 GetAbsolutePath(String *ppath) const
    {
        return AvmInteractiveObj::GetAbsolutePath(ppath);
    }
    virtual bool                        IsTabable() const 
    { 
        return AvmInteractiveObj::IsTabable();
    }
    virtual bool                DoesAcceptMouseFocus() const
    {
        return AvmInteractiveObj::DoesAcceptMouseFocus();
    }
    virtual void                        OnFocus
        (InteractiveObject::FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
        unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
    }
    virtual void                        OnGettingKeyboardFocus(unsigned controllerIdx, FocusMovedType fmt)
    {
        AvmInteractiveObj::OnGettingKeyboardFocus(controllerIdx, fmt);
    }
    virtual bool                        OnKeyEvent(const EventId& id, int* pkeyMask)
    {
        return AvmInteractiveObj::OnKeyEvent(id, pkeyMask);
    }
    virtual bool                        OnLosingKeyboardFocus
        (InteractiveObject* newFocusCh, unsigned controllerIdx, FocusMovedType fmt = GFx_FocusMovedByKeyboard)
    {
        return AvmInteractiveObj::OnLosingKeyboardFocus(newFocusCh, controllerIdx, fmt);
    }

    virtual const char*                 GetDefaultASClassName() const 
    { 
        return "flash.display.DisplayObjectContainer"; 
    }

    DisplayObjContainer*                GetDisplayObjContainer() 
    {
        return static_cast<DisplayObjContainer*>(GetDispObj());
    }
    const DisplayObjContainer*          GetDisplayObjContainer() const
    {
        return static_cast<const DisplayObjContainer*>(GetDispObj());
    }
    DisplayList&                        GetDisplayList() 
    { 
        return GetDisplayObjContainer()->GetDisplayList();
    }
    const DisplayList&                  GetDisplayList() const
    { 
        return GetDisplayObjContainer()->GetDisplayList();
    }
    unsigned                            GetNumChildren() const
    {
        return GetDisplayObjContainer()->GetNumChildren();
    }

    // adds a child at the end of DisplayList (on top of each other)
    void                AddChild(DisplayObjectBase* ch);
    // adds a child at the appropriate index. Index should be in the range 0..GetNumChildren()
    // Check should be done externally before calling this method.
    void                AddChildAt(DisplayObjectBase* ch, unsigned index);
    virtual InteractiveObject* FindInsertToPlayList(InteractiveObject* ch);
    // Removes child. Returns the removed child
    DisplayObjectBase*  RemoveChild(DisplayObjectBase* ch);
    // Removes child at the index. Returns the removed child
    DisplayObjectBase*  RemoveChildAt(unsigned index);
    // Sets index of existing child. Returns false if not found. Index should be in
    // correct range 0..GetNumChildren()-1
    bool                SetChildIndex(DisplayObjectBase*, unsigned index);

    // Returns AS3 object (GetChildAt returns GFx::DisplayObject)
    SPtr<Instances::DisplayObject> GetAS3ChildAt(unsigned index);
    // Returns AS3 object (GetChildByName returns GFx::DisplayObject)
    SPtr<Instances::DisplayObject> GetAS3ChildByName(const ASString& name);

    // Swaps children in display list. Returns false if at least one of them is not found
    // in the display list
    bool                SwapChildren(DisplayObjectBase*, DisplayObjectBase*);
    // Swaps children at indexes. Indexes should be in correct range 0..GetNumChildren()-1
    bool                SwapChildrenAt(unsigned i1, unsigned i2);

    // propagates events to all children (including itself).
    virtual void        PropagateEvent(const Instances::Event&, bool inclChildren);

    // fill an array with objects under the point. The point is in container's coord space.
    bool                GetObjectsUnderPoint(ArrayDH<Ptr<DisplayObjectBase> >* destArray, const PointF& pt) const;
};

SF_INLINE AvmDisplayObjContainer* ToAvmDisplayObjContainer(DisplayObjContainer* po)
{
    if (!po)
        return NULL;
    SF_ASSERT(po->IsAVM2());
    SF_ASSERT(po->GetAvmObjImpl());
    return static_cast<AvmDisplayObjContainer*>(po->GetAvmObjImpl()->ToAvmDispContainerBase());
}

SF_INLINE const AvmDisplayObjContainer* ToAvmDisplayObjContainer(const DisplayObjContainer* po)
{
    return const_cast<const AvmDisplayObjContainer*>(ToAvmDisplayObjContainer(const_cast<DisplayObjContainer*>(po)));
}

}}} // SF::GFx::AS3
#endif // INC_AS3_AvmDisplayObjContainer_H

