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
#include "GFx/AS3/AS3_AvmDisplayObj.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "Render/Render_TreeShape.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_EventDispatcher.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_Event.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {


// Constructor.
AvmDisplayObj::AvmDisplayObj(DisplayObject* pdispObj) 
: 
    pDispObj(pdispObj),
//     pAS3Obj(NULL),
    Flags(0)
{
    pClassName     = NULL;
    pdispObj->BindAvmObj(this);
    pAS3Obj.SetTypeRaw();
}

AvmDisplayObj::~AvmDisplayObj()
{
    SF_FREE((void*)pClassName);
//     if (pAS3Obj && (Flags & Flag_StrongAS3Ptr))
//         pAS3Obj->Release();
}

void AvmDisplayObj::OnEventLoad()
{
    // added to displaylist, make pAS3Obj ptr strong
    pAS3Obj.SetTypeStrong();
// 
//     GetAS3Root()->InsertEmptyAction(
//         MovieRoot::AL_EnterFrame)->SetAction(pDispObj, EventId::Event_Added);
}

bool AvmDisplayObj::OnUnloading(bool m)       
{ 
    // Check if "removed" or "removedFromStage" handler is installed. If yes, then do the following:
    // 1) Add Event_Removed into action queue
    // 2) Set depth to -2, meaning it is being unloaded.
    // 3) still return "true", meaning there is no need to move the char into "limbo" space in 
    // display list.
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj && (
        as3obj->HasEventHandler(GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removed), false) || 
        as3obj->HasEventHandler(GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removedFromStage), false)))
    {
        GetAS3Root()->InsertEmptyAction(
            MovieRoot::AL_EnterFrame)->SetAction(pDispObj, EventId::Event_Removed);
        
        pDispObj->SetDepth(-2); // mark as "unloading"

        // check, if we need to fire removeFromStage
        if (IsStageAccessible())
        {
            // if stage is still accessible then this means that we are
            // detaching the obj from the stage and need to fire "removeFromStage".
            Flags |= Flag_RemovedFromStage;
        }
    }
    return m; 
}

void AvmDisplayObj::OnEventUnload()
{
    if (!pDispObj->IsUnloadQueuedUp())
    {
        if (!pDispObj->IsInstanceBasedNameFlagSet() && pDispObj->IsTimelineObjectFlagSet())
        {
            if (GetAS3Parent())
            {
                const ASString& name = GetName();
                Value v;
                // get old value of "name" property first
                GetAS3Parent()->GetProperty(Multiname(GetAS3Root()->GetAVM()->GetPublicNamespace(), name), v).DoNotCheck();
                if (GetAS3Root()->GetAVM()->IsException())
                    GetAS3Root()->GetAVM()->OutputAndIgnoreException();
                // compare it with AS3 obj; reset it to null only if ptr is equal to as3obj. If not then the property probably
                // has already been changed.
                else if (v.IsObject() && v.GetObject() == GetAS3Obj())
                {
                    GetAS3Parent()->SetProperty(Multiname(GetAS3Root()->GetAVM()->GetPublicNamespace(), name), Value::GetNull()).DoNotCheck();
                    if (GetAS3Root()->GetAVM()->IsException())
                        GetAS3Root()->GetAVM()->OutputAndIgnoreException();
                }
            }
        }
        // need to weaken a reference to AS3 object when this object is removed
        // from display list. This will avoid circular reference.
        OnDetachFromTimeline();
    }
    else
    {
        // if char is marked "unloading" (see "OnUnloading") then we need to 
        // clear out Unloaded flag so the queued up "removed" event will be 
        // executed.
        pDispObj->SetUnloaded(false);
    }
}

bool AvmDisplayObj::HasEventHandler(const EventId& id) const
{
    if (pAS3Obj)
    {
        switch (id.Id)
        {
        case EventId::Event_EnterFrame:
            {
                // We return always false here; HasEventHandler is used
                // by CheckAdvanceStatus to determine either it is necessary to
                // put display object into optimized playlist or not. In AS3
                // enterFrame event doesn't affect optimized playlist since
                // it is executed separately.
//                 if (pAS3Obj->MayHaveEnterFrameHandler())
//                 {
//                     return pAS3Obj->HasEventHandler(
//                         GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_enterFrame),
//                         false);
//                 }
                break;
            }
        default: SF_ASSERT(0); // not implemented
        }
    }
    return false;
}

void AvmDisplayObj::PropagateEvent(const Instances::Event& evtProto, bool)
{
    SPtr<Instances::Event> evt = evtProto.Clone();
    SF_ASSERT(evt);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj)
    {
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
}

ASVM* AvmDisplayObj::GetAVM()
{
    GetAS3Root()->CheckAvm();
    return GetAS3Root()->GetAVM();
}
void AvmDisplayObj::FireEvent(const EventId& id)
{
    switch(id.Id)
    {
    case EventId::Event_Load:
        pDispObj->OnEventLoad();
        break;
    case EventId::Event_Added:
        {
            //        printf("Added! %s\n", GetName().ToCStr());
            const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_added);
            Instances::DisplayObject* as3obj = GetAS3Obj();
            if (as3obj && as3obj->HasEventHandler(evtName, false) && !GetAVM()->IsException())
            {
                SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
                SF_ASSERT(evt);
                evt->Target = as3obj;
                as3obj->Dispatch(evt, GetDispObj());
            }
            // check if we need to fire addedToStage
            if (IsStageAccessible() && !GetAVM()->IsException())
            {
                // fire it for each existing child
                SPtr<Instances::Event> evt = GetAS3Root()->CreateEventObject(
                    GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_addedToStage), false, false);
                SF_ASSERT(evt);
                PropagateEvent(*evt, false);
            }
        }
        break;
    case EventId::Event_Removed:
        {
            pDispObj->SetDepth(0); // remove "unloading" status

            ///        printf("Removed! %s\n", GetName().ToCStr());
            const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removed);
            Instances::DisplayObject* as3obj = GetAS3Obj();
            if (as3obj && as3obj->HasEventHandler(evtName, false) && !GetAVM()->IsException())
            {
                SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
                SF_ASSERT(evt);
                evt->Target = as3obj;
                as3obj->Dispatch(evt, GetDispObj());
            }
            // check if we need to fire removedFromStage
            if ((Flags & Flag_RemovedFromStage) && !GetAVM()->IsException())
            {
                SPtr<Instances::Event> evt = GetAS3Root()->CreateEventObject(
                    GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removedFromStage), false, false);
                SF_ASSERT(evt);
                PropagateEvent(*evt, false);
            }
            pDispObj->OnEventUnload();
        }

        break;
//     case EventId::Event_Unload:
//         {
//             // Event_Unload might be fired only when unloading was delayed. 
//             // Unloading is delayed for sprites with frame scripts. In the case
//             // when the sprite with frame script is removed from a stage, it
//             // shouldn't be removed from the playlist; instead, it is just marked
//             // as "unloading" to let the last frame script to be executed, and this
//             // event is put into the event queue. So, here we need to finalize unloading,
//             // removed it from displaylist, playlist and release all references to it.
//             if (GetParent() && GetParent()->IsDisplayObjContainer())
//             {
//                 DisplayObjContainer* cont = GetParent()->CharToDisplayObjContainer_Unsafe();
//                 SPInt i = cont->GetDisplayList().FindDisplayIndex(pDispObj);
//                 if (i >= 0)
//                 {
//                     cont->GetDisplayList().RemoveEntryAtIndex(UPInt(i));
//                 }
//             }
//             pDispObj->OnEventUnload();
//             break;
//        }
    case EventId::Event_Construct: // construct AS3 instance
        {
            //printf("construct %s\n", GetName().ToCStr());
            CallCtor(true);
            //CreateASInstance(true);
        }
        break;
    case EventId::Event_Initialize: // init as3 property
        {
            if (pDispObj->IsTimelineObjectFlagSet())
            {
                //@DBG
                //printf("$initialize %s\n", GetName().ToCStr());
                CreateASInstanceNoCtor();
                if (!pDispObj->IsInstanceBasedNameFlagSet() && GetAS3Parent())
                {
                    const ASString& name = GetName();
                    //SF_ASSERT(name != "start_btn");
                    // !!! SetProperty() can throw exceptions. Exception checking is disabled by the DoNotCheck() call.
                    GetAS3Parent()->SetProperty(Multiname(GetAS3Root()->GetAVM()->GetPublicNamespace(), name), Value(GetAS3Obj())).DoNotCheck();
                    if (GetAS3Root()->GetAVM()->IsException())
                        GetAS3Root()->GetAVM()->OutputAndIgnoreException();
                }
            }
        }
        break;
    case EventId::Event_EnterFrame:
        if (pAS3Obj)
        {
            if (pAS3Obj->MayHaveEnterFrameHandler())
            {
                pAS3Obj->DispatchToTarget(
                    GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_enterFrame),
                    pAS3Obj,
                    false);
            }
        }
        break;
    case EventId::Event_Activate:
        if (pAS3Obj && pAS3Obj->MayHaveActivateHandler())
        {
            pAS3Obj->Dispatch(id, pDispObj);
        }
        break;
    case EventId::Event_Deactivate:
        if (pAS3Obj && pAS3Obj->MayHaveDeactivateHandler())
        {
            pAS3Obj->Dispatch(id, pDispObj);
        }
        break;
    case EventId::Event_Render:
        if (pAS3Obj && pAS3Obj->MayHaveRenderHandler())
        {
            pAS3Obj->Dispatch(id, pDispObj);
        }
        break;
    }
}

// OnAdded is called when object is added to the stage by ActionScript 
// (using addChild/addChildAt)
void AvmDisplayObj::OnAdded(bool byTimeline)
{
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_added);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj && as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
    // check if we need to fire addedToStage
    if (IsStageAccessible() && !GetAVM()->IsException())
    {
        // fire it for each existing child
        SPtr<Instances::Event> evt = GetAS3Root()->CreateEventObject(
            GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_addedToStage), false, false);
        SF_ASSERT(evt);
        PropagateEvent(*evt, !byTimeline);
    }
}

// OnRemoved is called when object is removed from the stage by ActionScript 
// (using removeChild/removeChildAt)
void AvmDisplayObj::OnRemoved(bool byTimeline)
{
    ///        printf("Removed! %s\n", GetName().ToCStr());
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removed);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj && as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
    // check if we need to fire removedFromStage
    if (IsStageAccessible() && !GetAVM()->IsException())
    {
        SPtr<Instances::Event> evt = GetAS3Root()->CreateEventObject(
            GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_removedFromStage), false, false);
        SF_ASSERT(evt);
        PropagateEvent(*evt, !byTimeline);
    }
}


// method called on any event: enterframe, mouse, key.
// This method should only put event in appropriate level
// of action queue, DO NOT execute code right in it!
bool AvmDisplayObj::OnEvent(const EventId& id)
{
    switch (id.Id)
    {
    case EventId::Event_EnterFrame:
        {
            if (pAS3Obj)
            {
                GetAS3Root()->InsertEmptyAction(
                    MovieRoot::AL_EnterFrame)->SetAction(pDispObj, id);
            }
            break;
        }
    case EventId::Event_Render:
        {
            if (pAS3Obj)
            {
                GetAS3Root()->InsertEmptyAction(
                    MovieRoot::AL_Render)->SetAction(pDispObj, id);
            }
            break;
        }
	
    default:;
    }
    return true;
}
void AvmDisplayObj::ReleaseAS3Obj()    
{ 
    pAS3Obj.SetPtr(NULL);
}

void AvmDisplayObj::OnDetachFromTimeline()
{
    // need to weaken a reference to AS3 object when this object is removed
    // from display list. This will avoid circular reference.
    pAS3Obj.SetTypeRaw();
}

void AvmDisplayObj::AssignAS3Obj(Instances::DisplayObject* pas3obj)
{
//     if (pAS3Obj && (Flags & Flag_StrongAS3Ptr))
//         pAS3Obj->Release();
// 
//     pAS3Obj = pas3obj;
// 
//     if (pAS3Obj && (Flags & Flag_StrongAS3Ptr))
//         pAS3Obj->AddRef();

    pAS3Obj.SetPtr(pas3obj);
}

// Determines the absolute path of the character.
const char* AvmDisplayObj::GetAbsolutePath(String *ppath) const
{
    InteractiveObject* pparent = GetParent();
    if (pparent)
    {
        SF_ASSERT(pparent != pDispObj);
        pparent->GetAbsolutePath(ppath);
        *ppath += ".";
        *ppath += GetName().ToCStr();
    }
    return ppath->ToCStr();
}

MovieRoot* AvmDisplayObj::GetAS3Root() const 
{ 
    return static_cast<MovieRoot*>(pDispObj->GetASMovieRoot()); 
}

String& AvmDisplayObj::GetASClassName(String* className) const
{
    SF_ASSERT(className);
    if (HasClassName())
    {
        // if the obj was created with using classname (imported, and PlaceObject3 with classname was used)
        // then we need to return the classname (anyway, GetId will return InvalidId).
        *className = GetClassName();
    }
    else
    {
        const String* psymbolName = pDispObj->GetResourceMovieDef()->GetNameOfExportedResource(pDispObj->GetId());
        if (!psymbolName)
            *className = GetDefaultASClassName();
        else
            *className = *psymbolName;
    }
    return *className;
}

unsigned AvmDisplayObj::CreateASInstance(bool execute)
{
    if (CreateASInstanceNoCtor())
        return CallCtor(execute);
    return 0;
/*
    //return NULL;
    if (pAS3Obj)
        return 0;
    MovieRoot* proot = GetAS3Root();
    VM* avm = proot->GetAVM();
    if (!avm)
    {
        SF_DEBUG_MESSAGE(1, "AS3 AVM is not created, meaning no DoAbc tags executed. Can't create AS instance.");
        return 0;
    }
    SF_ASSERT(avm);
    if (avm)
    {
        String className;
        GetASClassName(&className);

//!        printf("Class: %s\n", className.ToCStr());

        Value v;
        const bool need2execute = avm->Construct(className, v);
        if (!avm->IsException() && !v.IsNullOrUndefined())
        {
            Instances::DisplayObject& as3iobj = static_cast<Instances::DisplayObject&>(*v.GetObject());
            as3iobj.pDispObj    = pDispObj;
            pAS3Obj.SetPtr(&as3iobj);
            pAS3Obj.SetTypeStrong();
            if (need2execute && execute)
            {
                avm->ExecuteCode();
                return 0;
            }
            return need2execute ? 1 : 0;
        }
        else
        {
            if (avm->IsException())
                avm->OutputAndIgnoreException();            
        }
    }
    return 0;*/
}

bool AvmDisplayObj::CreateASInstanceNoCtor()
{
    //return NULL;
    if (pAS3Obj)
        return false;
    MovieRoot* proot = GetAS3Root();
    VM* avm = proot->GetAVM();
    if (!avm)
    {
        SF_DEBUG_MESSAGE(1, "AS3 AVM is not created, meaning no DoAbc tags executed. Can't create AS instance.");
        return false;
    }
    SF_ASSERT(avm);
    if (avm)
    {
        String className;
        GetASClassName(&className);

//!        printf("Class: %s\n", className.ToCStr());

        Value _this;
        Value value;
        // We may not use GetClass() because GetClassUnsafe() can initialize 
        // scripts as a side effect.
        if (!avm->GetClassUnsafe(className, value))
        {
            if (avm->IsException()) avm->OutputAndIgnoreException();
            return false;
        }
        SF_ASSERT(!value.IsNull());
        SF_ASSERT(value.IsClass());
        Class& _class = value.AsClass();

        /* Future development.
        // Doesn't work yet.
        Value _this;
        Class* value = avm->GetClass(className);
        if (value == NULL)
        {
            if (avm->IsException()) avm->OutputAndIgnoreException();
            return false;
        }
        Class& _class = *value;
        */

#ifdef CONSTRUCT_IN_TRAITS
        InstanceTraits::Traits& tr = _class.GetInstanceTraits();
        tr.MakeObject(_this);
        tr.PreInit(_this);
#else
        _class.GetNewObjectTraits().MakeObject(_this);
        _class.PreInit(_this);
#endif

        Instances::DisplayObject& as3iobj = static_cast<Instances::DisplayObject&>(*_this.GetObject());
        as3iobj.pDispObj    = pDispObj;
        pAS3Obj.SetPtr(&as3iobj);
        pAS3Obj.SetTypeStrong();

        _this.GetObject()->InitInstance(true);
        return true;
    }
    return false;
}

unsigned AvmDisplayObj::CallCtor(bool execute)
{
    if (!pAS3Obj)
        return 0;
    const Class& _class = 
        const_cast<const Instances::DisplayObject*>(pAS3Obj.Get())->GetClass();
    Value _this(pAS3Obj);

    MovieRoot* proot = GetAS3Root();
    VM* avm = proot->GetAVM();
    if (!avm)
    {
        SF_DEBUG_MESSAGE(1, "AS3 AVM is not created, meaning no DoAbc tags executed. Can't create AS instance.");
        return 0;
    }
    UPInt stackSzBefore = avm->GetCallStack().GetSize();

#ifdef CONSTRUCT_IN_TRAITS
    _class.GetInstanceTraits().PostInit(_this, 0, NULL);
#else
    _class.PostInit(_this, 0, NULL);
#endif

    if (avm->GetCallStack().GetSize() > stackSzBefore)
    {
        // need to execute code
        if (execute)
        {
            avm->ExecuteCode();
            return 0;
        }
        else
            return 1;
    }
    return 0;
}

MemoryHeap* AvmDisplayObj::GetHeap() const
{
    return GetAS3Root()->GetMovieHeap();
}

DisplayObjContainer* AvmDisplayObj::GetRoot()
{
    Stage* stage = GetAS3Root()->GetStage();
    if (pDispObj == stage)
        return stage;
    return stage->GetRoot();
}

Stage* AvmDisplayObj::GetStage()
{
    return GetAS3Root()->GetStage();    
}

bool AvmDisplayObj::IsStageAccessible() const
{
    if (pDispObj->IsInteractiveObject() && 
        ToAvmInteractiveObj(pDispObj->CharToInteractiveObject_Unsafe())->IsStage())
        return true;
    AvmInteractiveObj* p = GetAvmParent();
    for (; p && p->GetAvmParent(); p = p->GetAvmParent())
        ;
    if (p)
    {
        return p->IsStage();
    }
    return false;
}

AvmInteractiveObj* AvmDisplayObj::GetAvmParent() const
{
    return ToAvmInteractiveObj(GetParent());
}

Instances::DisplayObject* AvmDisplayObj::GetAS3Parent() const 
{
    return (GetAvmParent()) ? GetAvmParent()->GetAS3Obj() : NULL; 
}

void AvmDisplayObj::InitClassName(const char* className)
{
    SF_ASSERT(!pClassName); // should be called only once!
    if (!pClassName)
    {
        UPInt len = SFstrlen(className);
        char* p = (char*)SF_HEAP_AUTO_ALLOC(GetDispObj(), len + 1);
        memcpy(p, className, len + 1);
        pClassName = p;
    }
}

bool AvmDisplayObj::GetObjectsUnderPoint(ArrayDH<Ptr<DisplayObjectBase> >* destArray, 
                                         const PointF& pt) const 
{ 
    if (pDispObj->PointTestLocal(pt, DisplayObjectBase::HitTest_TestShape))
    {
        // PPS: A push front op matches Flash player behavior; requires optimization
        //destArray->PushBack(pDispObj);
        destArray->InsertAt(0, pDispObj);

        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////
// ShapeObject
ShapeObject::ShapeObject(        
        CharacterDef* pdef,  
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id) : 
    DisplayObject(pasRoot, pparent, id),
    AvmDisplayObj(GetThis()),
    pDef(static_cast<ShapeBaseCharacterDef*>(pdef))
{
    SF_ASSERT(pDef);
    SF_ASSERT(pDef->GetType() == CharacterDef::Shape);
}

DrawingContext* ShapeObject::GetDrawingContext()
{
    if (!pDrawing)
    {
        pDrawing = *GetMovieImpl()->CreateDrawingContext();
        Render::TreeContainer* c = ConvertToTreeContainer();

        // Drawing API replaces existing shape, so we need to delete old node
        c->Remove(0, 1);

        c->Add(pDrawing->GetTreeNode());
    }
    return pDrawing;
}

DisplayObjectBase::TopMostResult ShapeObject::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr)
{   
    if (pDrawing)
    {
        Render::PointF p;          
        TransformPointToLocal(p, pt);

        if (pDrawing->DefPointTestLocal(p, true, this))
        {
            // need to return parent (since it is an InteractiveObject)
            pdescr->pResult = GetParent();
            return TopMost_Found;
        }
    }
    else
    {
        pdescr->pResult = GetTopMostMouseEntityDef(pDef, pt, pdescr->TestAll, pdescr->pIgnoreMC);
        if (pdescr->pResult)
            return TopMost_Continue;
    }

    return TopMost_FoundNothing;
}

bool ShapeObject::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{     
    if (pDrawing && pDrawing->DefPointTestLocal(pt, hitTestMask & HitTest_TestShape, this))
        return true;
    return pDef->DefPointTestLocal(pt, hitTestMask & HitTest_TestShape, this);
}

// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  ShapeObject::GetBounds(const Matrix &transform) const
{
    RectF r;

    if (pDrawing)
    {
        RectF tempRect;
        pDrawing->ComputeBound(&tempRect);
        if (!tempRect.IsEmpty())
        {
            tempRect = transform.EncloseTransform(tempRect);
            if (!r.IsEmpty())
                r.Union(tempRect);
            else
                r = tempRect;
        }
    }
    else
        r = RectF(transform.EncloseTransform(pDef->GetBoundsLocal()));

    return r;
}

Ptr<Render::TreeNode> ShapeObject::CreateRenderNode(Render::Context& context) const
{
    return pDef->CreateTreeShape(context, GetMovieImpl()->GetMovieDefImpl());
}

float ShapeObject::GetRatio() const
{
    if (pRenNode)
    {
        if (pRenNode->GetReadOnlyData()->GetType() == Render::TreeNode::NodeData::ET_Shape)
        {
            return static_cast<Render::TreeShape*>(pRenNode.GetPtr())->GetMorphRatio();
        }
    }
    return 0.f; 
}

void  ShapeObject::SetRatio(float f)
{
    Render::TreeNode* n = GetRenderNode();
    if (n && n->GetReadOnlyData()->GetType() == Render::TreeNode::NodeData::ET_Shape)
    {
        static_cast<Render::TreeShape*>(pRenNode.GetPtr())->SetMorphRatio(f);
    }
}

}}} // SF::GFx::AS3


