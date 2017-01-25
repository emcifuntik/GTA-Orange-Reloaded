/**********************************************************************

Filename    :   InteractiveObject.cpp
Content     :   InteractiveObject
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_PlayerImpl.h"

namespace Scaleform { namespace GFx {

// SF_INLINE AvmInteractiveObjBase* InteractiveObject::GetAvmIntObj() const
// {
//     return static_cast<AvmInteractiveObjBase*>(pAvmObjImpl.GetPtr());
// }

// Constructor.
InteractiveObject::InteractiveObject(
    MovieDefImpl* pbindingDefImpl,
    ASMovieRootBase* pasRoot,
    InteractiveObject* pparent, 
    ResourceId id)
                               :
    DisplayObject(pasRoot, pparent, id),
    pDefImpl(pbindingDefImpl),
    Flags(0),
    TabIndex (0),
    RollOverCnt(0),
    pDisplayCallback(NULL),
    DisplayCallbackUserPtr(NULL)
{
    FocusGroupMask = 0;
    SetInteractiveObjectFlag();
    pPlayNext = pPlayPrev = NULL;
    pPlayNextOpt = NULL;

    SetEnabledFlag();
}

InteractiveObject::~InteractiveObject()
{
    RemoveFromPlayList();
//    SF_ASSERT(!pPlayNext && !pPlayPrev); // actually already should be removed
//     if (pNameHandle)
//         pNameHandle->ReleaseCharacter();
}

Ptr<Render::TreeNode> InteractiveObject::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeContainer> node = *context.CreateEntry<Render::TreeContainer>();
    return node.GetPtr();
}

Render::TreeContainer* InteractiveObject::GetRenderContainer()
{
    // it is guaranteed that pRenNode is a TreeContainer!
    return static_cast<Render::TreeContainer*>(GetRenderNode());
}

MovieDefImpl* InteractiveObject::GetResourceMovieDef() const
{
    // Return the definition where binding takes place.
    return pDefImpl.GetPtr();
}

void InteractiveObject::SetNextUnloaded(InteractiveObject* punlObj)
{
    pNextUnloaded = punlObj;
    SetUnloaded();
}

void InteractiveObject::OnEventLoad()
{
    DisplayObject::OnEventLoad();
}

void InteractiveObject::OnEventUnload()
{
    SetUnloading();
    MovieImpl* proot = GetMovieImpl();
    RemoveFromPlayList();

    if (proot->IsDraggingCharacter(this))
        proot->StopDrag();

    // Need to check, is this character currently focused or not. If yes, we need
    // to reset a pointer to currently focused character. OnEventUnload might be
    // called from the sprite's destructor (so refcnt == 0 but weak ref is not set to NULL yet)
    if (proot)
        proot->ResetFocusForChar(this);

//     // need to release the character to avoid accidental reusing unloaded character.
//     if (pNameHandle)
//         pNameHandle->ReleaseCharacter();

    DisplayObject::OnEventUnload();
}

bool InteractiveObject::OnUnloading()
{
    bool rv = DisplayObject::OnUnloading();
    return rv;
}

bool InteractiveObject::IsInPlayList() const 
{ 
    return pPlayNext || pPlayPrev ||  GetMovieImpl()->pPlayListHead == this; 
}

void InteractiveObject::AddToPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    SF_ASSERT(proot->pPlayListHead != this);
    SF_ASSERT(!pPlayNext && !pPlayPrev);

    //@DBG
//     String s;
//     GetAbsolutePath(&s);
//     printf("=------------- %s\n", s.ToCStr()); //DBG

    // insert at the head of playlist
    if (proot->pPlayListHead)
    {
        proot->pPlayListHead->pPlayPrev = this;
        pPlayNext                       = proot->pPlayListHead;
    }
    proot->pPlayListHead                = this;
    proot->SetDirtyFlag();
}

void InteractiveObject::InsertToPlayListAfter(InteractiveObject* pafterCh)
{
    MovieImpl* proot = GetMovieImpl();
    SF_ASSERT(proot->pPlayListHead != this);
    SF_ASSERT(!pPlayNext && !pPlayPrev);
    
    SF_ASSERT(pafterCh); // use AddToPlayList if pafterCh == NULL

    SF_ASSERT(pafterCh->pPlayPrev || proot->pPlayListHead == pafterCh);
    
    this->pPlayPrev     = pafterCh;
    this->pPlayNext     = pafterCh->pPlayNext;
    pafterCh->pPlayNext = this;
    if (this->pPlayNext)
        this->pPlayNext->pPlayPrev = this;

    proot->SetDirtyFlag();
}

void InteractiveObject::AddToOptimizedPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    if (IsOptAdvancedListFlagSet() || proot->IsOptAdvanceListInvalid() || IsUnloaded() || IsUnloading())
        return;
    SF_ASSERT(IsInPlayList());
    SF_ASSERT(proot->pPlayListHead == this || pPlayPrev);
    SF_ASSERT(proot->pPlayListOptHead != this && !pPlayNextOpt);

    // find the place in the optimized playlist
    InteractiveObject* pcur = pPlayPrev;
    for (; pcur && !pcur->IsOptAdvancedListFlagSet(); pcur = pcur->pPlayPrev)
        ;
    if (pcur)
    {
        pPlayNextOpt        = pcur->pPlayNextOpt;
        pcur->pPlayNextOpt  = this;
    }
    else
    {
        pPlayNextOpt            = proot->pPlayListOptHead;
        proot->pPlayListOptHead = this;
    }
    SetOptAdvancedListFlag();
    proot->SetDirtyFlag();
}

void InteractiveObject::RemoveFromPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    SF_ASSERT(proot);

    RemoveFromOptimizedPlayList();

    if (pPlayNext)
        pPlayNext->pPlayPrev = pPlayPrev;

    if (pPlayPrev)
        pPlayPrev->pPlayNext = pPlayNext;
    else if (proot->pPlayListHead == this)
        proot->pPlayListHead = pPlayNext;
    
    pPlayNext = pPlayPrev = NULL;
    proot->SetDirtyFlag();
}

void InteractiveObject::RemoveFromOptimizedPlayList()
{
    MovieImpl* proot = GetMovieImpl();
    if (IsOptAdvancedListFlagSet())
    {
        if (!proot->IsOptAdvanceListInvalid())
        {
            SF_ASSERT(proot);

            // find previous element in optimized list
            InteractiveObject* pcur = pPlayPrev;
            for (; pcur && !pcur->IsOptAdvancedListFlagSet(); pcur = pcur->pPlayPrev)
                ;
            if (pcur)
                pcur->pPlayNextOpt = pPlayNextOpt;
            else
            {
                SF_ASSERT(proot->pPlayListOptHead == this);
                proot->pPlayListOptHead = pPlayNextOpt;
            }
        }
        pPlayNextOpt = NULL;
        ClearOptAdvancedListFlag();
        proot->SetDirtyFlag();
    }
}

void InteractiveObject::CloneInternalData(const InteractiveObject* src)
{
    SF_ASSERT(src);
    if (src->pGeomData)
        SetGeomData(*src->pGeomData);
    if (HasAvmObject())
        GetAvmIntObj()->CloneInternalData(src);
}

// Implement mouse-dragging for this pMovie.
void    InteractiveObject::DoMouseDrag()
{
    MovieImpl::DragState     st;
    MovieImpl*   proot = GetMovieImpl();
    proot->GetDragState(&st);

    if (this == st.pCharacter)
    {
        // We're being dragged!
        Render::PointF worldMouse = proot->GetMouseState(0)->GetLastPosition();
        Render::PointF parentMouse;
        Matrix  parentWorldMat;
        if (pParent)
            parentWorldMat = pParent->GetWorldMatrix();

        parentWorldMat.TransformByInverse(&parentMouse, worldMouse);
        // if (!st.LockCenter) is not necessary, because then st.CenterDelta == 0.
        parentMouse += st.CenterDelta;

        if (st.Bound)
        {           
            // Clamp mouse coords within a defined rectangle
            parentMouse.x = Alg::Clamp(parentMouse.x, st.BoundLT.x, st.BoundRB.x);
            parentMouse.y = Alg::Clamp(parentMouse.y, st.BoundLT.y, st.BoundRB.y);
        }

        // Once touched, object is no longer animated by the timeline
        SetAcceptAnimMoves(0);

        // Place our origin so that it coincides with the mouse coords
        // in our parent frame.
        SetX(TwipsToPixels(Double(parentMouse.x)));
        SetY(TwipsToPixels(Double(parentMouse.y)));

        //Matrix    local = GetMatrix();
        //local.Tx() = parentMouse.x;
        //local.Ty() = parentMouse.y;
        //SetMatrix(local);

    }
}

void    InteractiveObject::CopyPhysicalProperties(const InteractiveObject *poldChar)
{
    // Copy physical properties, used by loadMovie().
    SetDepth(poldChar->GetDepth());
    SetCxform(poldChar->GetCxform());
    SetMatrix(poldChar->GetMatrix());
    if (poldChar->pGeomData)
        SetGeomData(*poldChar->pGeomData);

    if (HasAvmObject())
        GetAvmIntObj()->CopyPhysicalProperties(poldChar);
}

void    InteractiveObject::MoveNameHandle(InteractiveObject *poldChar)
{
    // Re-link all ActionScript references.
    pNameHandle = poldChar->pNameHandle;
    poldChar->pNameHandle = 0;
    if (pNameHandle)
        pNameHandle->pCharacter = this;
}

bool InteractiveObject::IsFocusRectEnabled() const
{
    if (IsFocusRectFlagDefined())
        return IsFocusRectFlagTrue();
    //!AB: _focusrect seems to ignore lockroot. that is why we specify "true" as a parameter
    // of GetASRootMovie.
    InteractiveObject* prootMovie = GetTopParent(true);
    if (prootMovie != this)
        return prootMovie->IsFocusRectEnabled();
    return true;
}

bool InteractiveObject::DoesAcceptMouseFocus() const
{ 
    return GetAvmIntObj()->DoesAcceptMouseFocus();
}

void InteractiveObject::OnFocus(FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
                                unsigned controllerIdx, FocusMovedType fmt)
{
    if (HasAvmObject())
        GetAvmIntObj()->OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);
}

bool InteractiveObject::IsTabable() const
{
    if (HasAvmObject())
        return GetAvmIntObj()->IsTabable();
    return false;
}

// Determines the absolute path of the character.
// void    InteractiveObject::GetAbsolutePath(String *ppath) const
// {
//     if (pParent)
//     {
//         SF_ASSERT(pParent != this);
//         pParent->GetAbsolutePath(ppath);
//         *ppath += ".";
//         *ppath += GetName().ToCStr();
//     }
// }

ASStringManager* InteractiveObject::GetStringManager() const
{
    return GetMovieImpl()->GetStringManager();
}

// StringManager* InteractiveObject::GetBuiltinStringManager() const
// {
//     return GetMovieImpl()->GetBuiltinStringManager();
// }

// void InteractiveObject::SetDirtyFlag()
// {
//     GetMovieImpl()->SetDirtyFlag();
// }
// 
unsigned InteractiveObject::GetCursorType() const
{
    if (HasAvmObject())
        return GetAvmIntObj()->GetCursorType();
    return MouseCursorEvent::ARROW;
}


// int InteractiveObject::GetAVMVersion() const
// {
//     return (pDefImpl->GetFileAttributes() & GFxMovieDef::FileAttr_UseActionScript3) ? 2 : 1;
// }

// Remove the object with the specified name.
void  InteractiveObject::RemoveDisplayObject()
{
    if (!GetParent())
        return;
    DisplayObjContainer* pparent = GetParent()->CharToDisplayObjContainer();
    if (!pparent)
        return; 
    pparent->RemoveDisplayObject(GetDepth(), GetId());
}


UInt16 InteractiveObject::GetFocusGroupMask() const
{
    if (FocusGroupMask == 0)
        return GetParent()->GetFocusGroupMask();
    return FocusGroupMask;
}

UInt16 InteractiveObject::GetFocusGroupMask()
{
    if (FocusGroupMask == 0 && GetParent())
        FocusGroupMask = GetParent()->GetFocusGroupMask();
    return FocusGroupMask;
}

bool   InteractiveObject::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const
{
    unsigned focusIdx = proot->GetFocusGroupIndex(controllerIdx);
    return (GetFocusGroupMask() & (1 << focusIdx)) != 0;
}

bool   InteractiveObject::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx)
{
    unsigned focusIdx = proot->GetFocusGroupIndex(controllerIdx);
    return (GetFocusGroupMask() & (1 << focusIdx)) != 0;
}

/*
// ***** CharacterHandle
CharacterHandle::CharacterHandle(const ASString& name, InteractiveObject *pparent, InteractiveObject* pcharacter)
: Name(name), NamePath(name.GetManager()->CreateEmptyString()), OriginalName(name)
{
    RefCount    = 1;
    pCharacter  = pcharacter;    

    if (pcharacter->IsAVM1())
    {
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);
    }
}

CharacterHandle::~CharacterHandle()
{
}

// Release a character reference, used when character dies
void    CharacterHandle::ReleaseCharacter()
{
    pCharacter = 0;
}


// Changes the name.
void    CharacterHandle::ChangeName(const ASString& name, InteractiveObject *pparent)
{
    if (pparent->IsAVM1())
    {
        Name = name;
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);

        // Do we need to update paths in all parents ??
    }
}

// Resolve the character, considering path if necessary.
InteractiveObject* CharacterHandle::ResolveCharacter(MovieImpl *proot) const
{
    if (pCharacter)
        return pCharacter;
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

InteractiveObject* CharacterHandle::ForceResolveCharacter(MovieImpl *proot) const
{
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

void CharacterHandle::ResetName(StringManager* psm)
{
    Name     = psm->GetBuiltin(ASBuiltin_empty_);
    NamePath = Name; 
}
*/
}} // Scaleform::GFx
