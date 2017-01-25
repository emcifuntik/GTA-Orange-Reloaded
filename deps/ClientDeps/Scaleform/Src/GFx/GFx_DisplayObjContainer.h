/**********************************************************************

Filename    :   DisplayObjContainer.h
Content     :   Implementation of DisplayObjContainer
Created     :   
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef INC_SF_GFX_DISPLAYOBJCONTAINER_H
#define INC_SF_GFX_DISPLAYOBJCONTAINER_H

#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_DisplayList.h"

namespace Scaleform { namespace GFx {

class AvmDisplayObjContainerBase : public AvmInteractiveObjBase 
{
public:
    virtual void    FillTabableArray(InteractiveObject::FillTabableParams* params) =0;
};

// Implementation of DisplayObjectContainer class. This is a base class
// for Sprite. It contains DisplayList.
class DisplayObjContainer : public InteractiveObject
{
protected:
    DisplayList     mDisplayList;    

public:
    DisplayObjContainer
        (MovieDefImpl* pbindingDefImpl, 
         ASMovieRootBase* pasRoot,
         InteractiveObject* pparent, 
         ResourceId id);
    virtual ~DisplayObjContainer();


    //*** DisplayObjectBase overloaded methods
    // Return character bounds in specified coordinate space.
    virtual RectF           GetBounds(const Matrix &t) const     
    { 
        return mDisplayList.GetBounds(t); 
    }
    virtual CharacterDef*   GetCharacterDef() const { return NULL; }

    // Returns character bounds excluding stroke
    virtual RectF           GetRectBounds(const Matrix &t) const 
    { 
        return mDisplayList.GetRectBounds(t); 
    }

    //virtual void            Display(DisplayContext &context);

    //*** domestic methods
    // adds a child at the end of DisplayList (on top of each other)
    //void                AddChild(DisplayObjectBase* ch);

    // adds a child at the appropriate index. Index should be in the range 0..GetNumChildren()
    // Check should be done externally before calling this method.
    //void                AddChildAt(DisplayObjectBase* ch, unsigned index);
        
    virtual DisplayObjectBase*  AddDisplayObject(
        const CharPosInfo &pos, 
        const ASString& name,
        const ArrayLH<SwfEvent*, StatMD_Tags_Mem>* peventHandlers,
        const void *pinitSource,
        unsigned createFrame = SF_MAX_UINT,
        UInt32 addFlags = 0,
        CharacterCreateInfo* pcharCreateOverride = 0,
        InteractiveObject* origChar = 0)
    {
        SF_ASSERT(0); // shouldn't be called!
        SF_UNUSED7(pos, name, peventHandlers, pinitSource, createFrame, addFlags, pcharCreateOverride);
        SF_UNUSED(origChar);
        return NULL;
    }

    void                ClearDisplayList()
    {
        mDisplayList.Clear(this);
        SetDirtyFlag();
    }
    // Determines whether the specified display object is a child (or grandchild, or grandgrand...)
    // of the DisplayObjectContainer instance or the instance itself.
    bool                Contains(DisplayObjectBase* ch) const;

    virtual void        ExecuteImportedInitActions(MovieDef*) {}

    virtual Sprite*     GetHitArea() const { return NULL; }
    virtual Sprite*     GetHitAreaHolder() const { return NULL; }

    // For debugging -- return the id of the DisplayObjectBase at the specified depth.
    // Return 0 if nobody's home.
    DisplayObjectBase*  GetCharacterAtDepth(int depth);
    DisplayObjectBase*  GetCharacterAtDepth(int depth, 
                                                bool *markedForRemove);
    DisplayObjectBase*  GetChildAt(unsigned index) const;
    DisplayObjectBase*  GetChildByName(const ASString& name) const
    {
        return mDisplayList.GetDisplayObjectByName(name);
    }
    // Returns index of the child display object. Returns ~0u if specified
    // display object is not child.
    SPInt               GetChildIndex(DisplayObjectBase* ch) const
    {
        return mDisplayList.FindDisplayIndex(ch);
    }
    
    // Return -1 if nobody's home.
    ResourceId          GetIdAtDepth(int depth);
    // Returns maximum used depth, -1 if no depth is available.
    int                 GetLargestDepthInUse() const { return mDisplayList.GetLargestDepthInUse();}
    unsigned            GetNumChildren() const { return (unsigned)mDisplayList.GetCount(); }

    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    virtual void        OnInsertionAsLevel(int level);

    void                MoveDisplayObject(const CharPosInfo &pos); 
    virtual void        CreateAndReplaceDisplayObject(const CharPosInfo &pos, const ASString& name, DisplayObjectBase** newChar);

    virtual bool        PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Propagates an event to all children.
    virtual void        PropagateMouseEvent(const EventId& id);
    virtual void        PropagateKeyEvent(const EventId& id, int* pkeyMask);
    virtual void        PropagateFocusGroupMask(unsigned mask);

    void                ReplaceDisplayObject(const CharPosInfo &pos, DisplayObjectBase* ch, const ASString& name);
    void                RemoveDisplayObject(int depth, ResourceId id); 
    void                RemoveDisplayObject() { InteractiveObject::RemoveDisplayObject(); }
    virtual void        RemoveDisplayObject(DisplayObjectBase* ch);

    // Removes child. Returns the removed child
    //DisplayObjectBase*  RemoveChild(DisplayObjectBase* ch);
    // Removes child at the index. Returns the removed child
    //DisplayObjectBase*  RemoveChildAt(unsigned index);
    // Sets index of existing child. Returns false if not found. Index should be in
    // correct range 0..GetNumChildren()-1
    //bool                SetChildIndex(DisplayObjectBase*, unsigned index);

    // Swaps children in display list. Returns false if at least one of them is not found
    // in the display list
    //bool                SwapChildren(DisplayObjectBase*, DisplayObjectBase*);
    // Swaps children at indexes. Indexes should be in correct range 0..GetNumChildren()-1
    //bool                SwapChildrenAt(unsigned i1, unsigned i2);

    SF_INLINE void      VisitChildren(DisplayList::MemberVisitor *pvisitor) const
    {
        mDisplayList.VisitMembers(pvisitor);
    }
    DisplayList&        GetDisplayList()        { return mDisplayList; }
    const DisplayList&  GetDisplayList() const  { return mDisplayList; }
    //virtual void        InsertBranchToPlayList(InteractiveObject* pbegin, InteractiveObject* pend);

    void                FillTabableArray(InteractiveObject::FillTabableParams* params);

#ifdef SF_BUILD_DEBUG
    void                DumpDisplayList(int indent, const char* title = NULL);
#else
    inline void         DumpDisplayList(int , const char*  = NULL) {}
#endif

    void                CalcDisplayListHitTestMaskArray(
        Array<UByte> *phitTest, const Render::PointF &p, bool testShape) const;
private:
    SF_INLINE AvmDisplayObjContainerBase*  GetAvmDispContainer()
    {
        return GetAvmObjImpl()->ToAvmDispContainerBase();
    }
};

// conversion methods 
SF_INLINE
DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer()
{
    return IsDisplayObjContainer() ? static_cast<DisplayObjContainer*>(this) : 0;
}

SF_INLINE
DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer_Unsafe()
{
    SF_ASSERT(IsDisplayObjContainer());
    return static_cast<DisplayObjContainer*>(this);
}

SF_INLINE
const DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer() const 
{
    return IsDisplayObjContainer() ? static_cast<const DisplayObjContainer*>(this) : 0;
}

SF_INLINE
const DisplayObjContainer* DisplayObjectBase::CharToDisplayObjContainer_Unsafe() const 
{
    SF_ASSERT(IsDisplayObjContainer());
    return static_cast<const DisplayObjContainer*>(this);
}

SF_INLINE
DisplayObjectBase* DisplayObjContainer::GetCharacterAtDepth(int depth, 
                                                            bool *markedForRemove)
{
    return mDisplayList.GetDisplayObjectAtDepth(depth, markedForRemove);
}


}} // namespace Scaleform::GFx
#endif //INC_SF_GFX_DISPLAYOBJCONTAINER_H
