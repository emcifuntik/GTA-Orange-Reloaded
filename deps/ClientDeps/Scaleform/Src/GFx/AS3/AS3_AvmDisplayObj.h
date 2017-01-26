/**********************************************************************

Filename    :   AvmDisplayObj.h
Content     :   Implementation of AS3-dependent part of DisplayObject and Shape.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_AS3_AvmDisplayObj_H
#define INC_AS3_AvmDisplayObj_H

#include "GFx/GFx_ShapeSwf.h"
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_DisplayObject.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_Event.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes
class MovieImpl;
class MovieDataDef;
class MovieDefImpl;
struct TextFilter;
class DisplayObjContainer;

namespace AS3 {

class MovieRoot;
class Stage;
class AvmDisplayObjContainer;
class AvmInteractiveObj;
class ASVM;

// An AS3-dependable part of displaylist's DisplayObject.
class AvmDisplayObj : 
    public NewOverrideBase<StatMV_ActionScript_Mem>,
    public AvmDisplayObjBase
{
protected:
    DisplayObject*                  pDispObj;
    APtr<Instances::DisplayObject>  pAS3Obj; // might be strong and weak ptr (see Flag_StrongAS3Ptr)
    const char*                     pClassName; // a classname used to create the obj; Id == Invalid in this case.

    enum {
        Flag_RemovedFromStage   = 0x01
    };
    UByte                       Flags;

private:
    // implementations of ASInteractiveObjBase
    virtual void Bind(DisplayObject* po)
    {
        //SF_ASSERT(po->IsInteractiveObject() && static_cast<InteractiveObject*>(po)->IsAVM1());
        pDispObj = po;
    }

public:
    // Constructor.
    AvmDisplayObj(DisplayObject* pdispObj);
    ~AvmDisplayObj();

    virtual class AvmInteractiveObjBase*    ToAvmInteractiveObjBase() { return NULL; }
    virtual class AvmSpriteBase*            ToAvmSpriteBase()         { return NULL; }
    virtual class AvmButtonBase*            ToAvmButttonBase()        { return NULL; }
    virtual class AvmTextFieldBase*         ToAvmTextFieldBase()      { return NULL; }
    virtual class AvmDisplayObjContainerBase* ToAvmDispContainerBase(){ return NULL; }

    // Determines the absolute path of the character.
    virtual const char*                     GetAbsolutePath(String *ppath) const;
    virtual bool                            HasEventHandler(const EventId& id) const;
    // method called on any event: enterframe, mouse, key.
    // This method should only put event in appropriate level
    // of action queue, DO NOT execute code right in it!
    virtual bool                            OnEvent(const EventId& id);
    // This method is called when display object is added to display list
    virtual void                            OnEventLoad();
    // This method is called when object is removed from display list and onEventLoad was called
    virtual void                            OnEventUnload();
    // OnRemoved is called when object is removed from the stage by ActionScript 
    // (using removeChild/removeChildAt)
    // It is also called when object is removed from display list and onEventLoad wasn't called
    // (instead, OnAdded was).
    virtual void                            OnRemoved(bool byTimeline);

//    virtual bool                            OnUnloading(bool m)       { return m; }
    virtual bool                            OnUnloading(bool m);

    DisplayObject*                          GetDispObj() const { return pDispObj; }
    MovieRoot*                              GetAS3Root() const;
    MemoryHeap*                             GetHeap() const;

    InteractiveObject*                      GetParent() const 
    { 
        SF_ASSERT(!pDispObj->GetParent() || pDispObj->GetParent()->IsInteractiveObject());
        return pDispObj->GetParent(); 
    }
    AvmInteractiveObj*                      GetAvmParent() const;
    Instances::DisplayObject*               GetAS3Parent() const;
    ASString                                GetName() const  { return pDispObj->GetName(); }

    // creates instance of AS class for the object
    // returns 1 if needs ExecuteCode, 0 - otherwise
    virtual unsigned                        CreateASInstance(bool execute);
    bool                                    CreateASInstanceNoCtor();
    unsigned                                CallCtor(bool execute);
    virtual const char*                     GetDefaultASClassName() const { return "flash.display.DisplayObject"; }
    virtual String&                         GetASClassName(String* className) const;
    Instances::DisplayObject*               GetAS3Obj() const  { return pAS3Obj; }
    ASVM*                                   GetAVM();
    void                                    ReleaseAS3Obj();
    void                                    OnDetachFromTimeline();
    void                                    AssignAS3Obj(Instances::DisplayObject*);

    DisplayObjContainer*                    GetRoot();
    Stage*                                  GetStage();
    bool                                    IsStageAccessible() const;

    // actually executes the event
    void                                    FireEvent(const EventId& id);
    // propagates events to all children (including itself).
    virtual void                            PropagateEvent(const Instances::Event&, bool inclChildren = true);

    // OnAdded is called when object is added to the stage by ActionScript 
    // (using addChild/addChildAt). See also: OnRemoved
    virtual void                            OnAdded(bool byTimeline);

    void                                    InitClassName(const char*);
    bool                                    HasClassName() const { return !!pClassName; }
    const char*                             GetClassName() const { return pClassName;  }

    virtual bool                            GetObjectsUnderPoint
        (ArrayDH<Ptr<DisplayObjectBase> >* destArray, const PointF& pt) const;


    void*   operator new        (UPInt , void *ptr)   { return ptr; }
    void    operator delete     (void *, void *) { }
    void    operator delete     (void *) { SF_ASSERT(false); }
};

// This class represents a DisplayList's object "Shape"
// (analog of GenericCharacter in AS2). AVM-part is AvmDisplayObj.
class ShapeObject : public DisplayObject, public AvmDisplayObj
{
    Ptr<ShapeBaseCharacterDef>  pDef;
    Ptr<DrawingContext>         pDrawing;

    ShapeObject* GetThis() { return this; }
public:
    // Constructor.
    ShapeObject(        
        CharacterDef* pdef,  
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id);

    virtual CharacterDef* GetCharacterDef() const
    {
        return pDef;
    }

//    virtual void    Display(DisplayContext &context);

    // These are used for finding bounds, width and height.
    virtual RectF  GetBounds(const Matrix &transform) const;
    virtual RectF  GetRectBounds(const Matrix &transform) const
    {       
        return transform.EncloseTransform(pDef->GetRectBoundsLocal());
    }   

    virtual bool    PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    virtual DrawingContext* GetDrawingContext();

    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;

    // AvmDisplayObj methods
    virtual const char*                     GetDefaultASClassName() const { return "flash.display.Shape"; }
    InteractiveObject*                      GetParent() const 
    { 
        return DisplayObject::GetParent(); 
    }

    virtual float           GetRatio() const;
    virtual void            SetRatio(float);

    SF_MEMORY_REDEFINE_NEW(ShapeObject, StatMV_ActionScript_Mem)
};

SF_INLINE AvmDisplayObj* ToAvmDisplayObj(DisplayObject* po)
{
    if (!po)
        return NULL;
    SF_ASSERT(po->IsAVM2());
    SF_ASSERT(po->GetAvmObjImpl());
    return static_cast<AvmDisplayObj*>(po->GetAvmObjImpl());
}

SF_INLINE const AvmDisplayObj* ToAvmDisplayObj(const DisplayObject* po)
{
    return const_cast<const AvmDisplayObj*>(ToAvmDisplayObj(const_cast<DisplayObject*>(po)));
}

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmDisplayObj_H
