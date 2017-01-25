/**********************************************************************

Filename    :   AvmInteractiveObj.h
Content     :   Implementation of AS3-dependent part of InteractiveObject.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/AS3/AS3_AvmInteractiveObj.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_InteractiveObject.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_FocusEvent.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {


// Constructor.
AvmInteractiveObj::AvmInteractiveObj(InteractiveObject* pdispObj) 
:
    AvmDisplayObj(pdispObj),
    MouseOverCnt(0),
    Flags(0)
{
}

AvmInteractiveObj::~AvmInteractiveObj()
{
}


void AvmInteractiveObj::OnEventLoad()
{
    // moved in AS3::InteractiveObject::Super
//     GetAvmParent()->InsertChildToPlayList(GetIntObj());
//     GetIntObj()->ModifyOptimizedPlayList();

// 
//     //CreateASInstance();//?? delayed creation?
//     if (pDispObj->IsTimelineObjectFlagSet())
//     {
//         GetAS3Root()->InsertEmptyAction(MovieRoot::AL_Highest)->SetAction(pDispObj, EventId::Event_Initialize);
//         GetAS3Root()->InsertEmptyAction(MovieRoot::AL_High)->SetAction(pDispObj, EventId::Event_Construct);
//         printf("??? %s\n", GetName().ToCStr());
//         //GetAS3Root()->IncASFramesCnt(CreateASInstance(false));
//     }
    AvmDisplayObj::OnEventLoad();
}

void AvmInteractiveObj::OnEventUnload()
{
    AvmDisplayObj::OnEventUnload();
}

bool AvmInteractiveObj::OnEvent(const EventId& id)
{
    switch (id.Id)
    {
    case EventId::Event_Click:
    case EventId::Event_DoubleClick:
    case EventId::Event_MouseOver:
    case EventId::Event_MouseOut:
    case EventId::Event_RollOver:
    case EventId::Event_RollOut:
    case EventId::Event_MouseMove:
    case EventId::Event_MouseDown:
    case EventId::Event_MouseUp:
	case EventId::Event_Resize:
        if (pAS3Obj)
        {
            // notify listeners
            pAS3Obj->Dispatch(id, GetIntObj());
        }
        break;
    default:
        return AvmDisplayObj::OnEvent(id);
    }
    return true;
}

void AvmInteractiveObj::CloneInternalData(const InteractiveObject*)
{
}

// Remove the object with the specified name.
// void    AvmInteractiveObj::RemoveDisplayObject()
// {
//     if (!GetParent())
//         return;
//     Sprite* pparent = GetParent()->CharToSprite();
//     if (!pparent)
//         return; 
//     pparent->RemoveDisplayObject(pDispObj->GetDepth(), pDispObj->GetId());
// }


void    AvmInteractiveObj::CopyPhysicalProperties(const InteractiveObject *)
{
}

bool AvmInteractiveObj::DoesAcceptMouseFocus() const
{
    return true;
}

void AvmInteractiveObj::OnFocus(InteractiveObject::FocusEventType evt, 
                                InteractiveObject* oldOrNewFocusCh, 
                                unsigned controllerIdx,
                                FocusMovedType)
{
    SF_ASSERT(pAS3Obj);
    SF_UNUSED(controllerIdx);
    const char* focusName = (evt == InteractiveObject::KillFocus) ? "focusOut" : "focusIn";

    SF_ASSERT(!oldOrNewFocusCh || ToAvmInteractiveObj(oldOrNewFocusCh)->GetAS3Obj());
    SF_ASSERT(pAS3Obj);
    Instances::DisplayObject* relatedObj = (oldOrNewFocusCh) ? 
        ToAvmInteractiveObj(oldOrNewFocusCh)->GetAS3Obj() : NULL;

    SPtr<Instances::FocusEvent> focusEvt = 
        pAS3Obj->CreateFocusEventObject(GetMovieImpl()->GetStringManager()->CreateConstString(focusName), 
        relatedObj, controllerIdx);
    pAS3Obj->Dispatch(focusEvt.Get(), pDispObj);
}

bool AvmInteractiveObj::OnFocusChange
    (InteractiveObject* toBeFocused, unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* pfocusKeyInfo)
{
    SF_ASSERT(fmt != GFx_FocusMovedByKeyboard || pfocusKeyInfo);
    const char* focusName = (fmt == GFx_FocusMovedByKeyboard) ? "keyFocusChange" : "mouseFocusChange";
    
    SF_ASSERT(!toBeFocused || ToAvmInteractiveObj(toBeFocused)->GetAS3Obj());
    SF_ASSERT(pAS3Obj);
    SF_UNUSED(controllerIdx);
    Instances::DisplayObject* relatedObj = (toBeFocused) ? 
        ToAvmInteractiveObj(toBeFocused)->GetAS3Obj() : NULL;

    SPtr<Instances::FocusEvent> focusEvt = 
        pAS3Obj->CreateFocusEventObject(GetMovieImpl()->GetStringManager()->CreateConstString(focusName), 
        relatedObj, controllerIdx);

    focusEvt->Cancelable = true;
    if (fmt == GFx_FocusMovedByKeyboard)
    {
        focusEvt->KeyCode = pfocusKeyInfo->KeyCode;
        focusEvt->ShiftKey = KeyModifiers(pfocusKeyInfo->KeysState).IsShiftPressed();
    }
    pAS3Obj->Dispatch(focusEvt.Get(), pDispObj);
    return !focusEvt->IsDefaultPrevented();
}

InteractiveObject* AvmInteractiveObj::GetTopParent(bool ignoreLockRoot) const
{
    InteractiveObject* pparent = GetParent();
    if (!pparent || !pparent->IsAVM2())
        return static_cast<InteractiveObject*>(pDispObj); // already top parent
    return pparent->GetTopParent(ignoreLockRoot);
}

AvmInteractiveObj* AvmInteractiveObj::GetAvmTopParent(bool ignoreLockRoot) const
{
    if (!GetParent()->IsAVM2())
        return NULL;

    return static_cast<AvmInteractiveObj*>(
        GetParent()->GetTopParent(ignoreLockRoot)->GetAvmIntObj());
}

bool    AvmInteractiveObj::IsTabable() const
{
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) 
        return false;
    if (GetIntObj()->IsTabEnabledFlagDefined())
        return GetIntObj()->IsTabEnabledFlagTrue();
    else if (GetIntObj()->GetTabIndex() > 0)
        return true;
    return false;
}

bool AvmInteractiveObj::ActsAsButton() const    
{ 
//     if (IsLevelMovie() || !IsEnabledFlagSet()/* || !GetParent() || !GetParent()->GetParent()*/)
//         return false; // _levelN should NOT act as a button no matter what.
    //     const GASMovieClipObject* asObj = (ASMovieClipObj) ? ASMovieClipObj.GetPtr() :
    //         static_cast<const GASMovieClipObject*>(Get__proto__());
    //     if (asObj) 
    //         return asObj->ActsAsButton(); 
    if (pAS3Obj)
    {
        return pAS3Obj->HasButtonHandlers();
    }
    return true;//false;
}

InteractiveObject* AvmInteractiveObj::FindInsertToPlayList(InteractiveObject* ch)
{
    InteractiveObject* parent = ch->GetParent();
    if (parent && parent->pPlayPrev)
    {
        // insert before the parent
        //ch->InsertToPlayListAfter(parent->pPlayPrev);
        return parent->pPlayPrev;
    }
    else
        //ch->AddToPlayList();
        return NULL;
}

void AvmInteractiveObj::MoveBranchInPlayList()
{
    InteractiveObject* p = GetIntObj();
    //SF_ASSERT(p->IsInPlayList());
    InteractiveObject* pend = p;

    // save the previous in the playlist
    InteractiveObject* pbefore = p->pPlayNext;
    InteractiveObject* pafter = NULL;

    // now look for the last item in the branch
    InteractiveObject* pcur = p->pPlayPrev;
    InteractiveObject* parpar = p;
    for (; pcur ; pcur = pcur->pPlayPrev)
    {
        // check the parent chain.
        InteractiveObject* c = pcur;
        for (; c; c = c->GetParent())
        {
            if (c == parpar)
            {
                // still have the parent...
                break;
            }
        }
        if (!c)
        {
            // there is no parent in the parent chain!
            pafter = pcur;
            break;
        }
    }
    // connect pbefore->pPlayPrev and pafter->pPlayNext, thus excluding the branch
    // from the playlist....
    if (pafter)
    {
        pend   = pafter->pPlayNext;
        pafter->pPlayNext = pbefore;
    }
    else
    {
        if (pbefore)
        {
            pend = GetMovieImpl()->pPlayListHead;
            GetMovieImpl()->pPlayListHead = pbefore;
        }
        else
            pend = p;
    }
    if (pbefore)
        pbefore->pPlayPrev  = pafter;

    SF_ASSERT(p && pend);
    p->pPlayNext    = NULL;
    pend->pPlayPrev = NULL;

    // now need to insert the branch [pend->p] into the playlist at new location
    InteractiveObject* parent = GetParent();
    if (parent)
    {
        InteractiveObject* pafter = ToAvmInteractiveObj(parent)->FindInsertToPlayList(p);
        if (pafter)
        {
            p->pPlayNext      = pafter->pPlayNext;
            if (p->pPlayNext)
                p->pPlayNext->pPlayPrev = p;

            pafter->pPlayNext = pend;
            pend->pPlayPrev   = pafter;
        }
        else
            parent = NULL;
    }
    if (!parent)
    {
        // no parent, insert at the head of playlist
        InteractiveObject* phead = GetMovieImpl()->pPlayListHead;
        if (phead)
        {
            p->pPlayNext = phead;
            phead->pPlayPrev = p;
        }
        GetMovieImpl()->pPlayListHead = pend;
    }
    GetMovieImpl()->InvalidateOptAdvanceList();
}

void AvmInteractiveObj::InsertChildToPlayList(InteractiveObject* ch)
{
    SF_ASSERT(!ch->IsDisplayObjContainer() || ch->CharToDisplayObjContainer_Unsafe()->GetNumChildren() == 0);
    InteractiveObject* pafter = FindInsertToPlayList(ch);
    if (pafter)
        ch->InsertToPlayListAfter(pafter);
    else
        ch->AddToPlayList();
}

}}} // SF::GFx::AS3


