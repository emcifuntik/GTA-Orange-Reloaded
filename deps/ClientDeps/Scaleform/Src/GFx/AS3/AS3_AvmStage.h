/**********************************************************************

Filename    :   AvmStage.h
Content     :   Implementation of AS3-dependent part of Stage.
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFx_AS3_AvmStage_H
#define INC_SF_GFx_AS3_AvmStage_H

#include "GFx/AS3/AS3_AvmDisplayObjContainer.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_DisplayObjContainer.h"
#include "GFx/GFx_Sprite.h"

namespace Scaleform {
namespace GFx {

namespace AS3 {

class MovieRoot;

class FrameExecSnapshot
{
    ArrayDH<Ptr<Sprite> > mArray;
public:
    FrameExecSnapshot(MemoryHeap* pheap) : mArray(pheap) {}

    UPInt GetSize() const { return mArray.GetSize(); }
    Sprite* operator[](UPInt i) const { return mArray[i]; }

    void PushBack(Sprite* d)
    {
        mArray.PushBack(d);
    }
};


// The Stage class. This is a top level container in AS3 hierarchy.
class Stage : public DisplayObjContainer
{
    Ptr<DisplayObjContainer>    pRoot;
    Ptr<InteractiveObject>      FrameCounterObj;
public:
    Stage(MovieDefImpl* pbindingDefImpl, 
        ASMovieRootBase* pasRoot,
        InteractiveObject* pparent, 
        ResourceId id);
    ~Stage();

    virtual unsigned        GetLoadingFrame() const { return 1; }

    // initialize the stage
    virtual void            ExecuteFrame0Events();
    virtual void            ExecuteFrameTags(unsigned frame);
    virtual void            OnInsertionAsLevel(int level);

    virtual void            UpdateViewAndPerspective();

    DisplayObjContainer*    GetRoot()
    {
        return pRoot;
    }
    void                    SetRoot(DisplayObjContainer* proot)
    {
        pRoot = proot;
    }

    virtual void            ForceShutdown ();

    virtual InteractiveObject* GetTopParent(bool ignoreLockRoot = false) const 
    { 
        if (!pParentChar)
            return const_cast<Stage*>(this);
        return pParentChar->GetTopParent(ignoreLockRoot);
    }

    void                    QueueFrameActions();
protected:
    void                    PropagateMouseEvent(const EventId& id);

    virtual ASString        CreateNewInstanceName() const;
};

// AVM-dependable part of Stage class.
class AvmStage : 
    public AvmDisplayObjContainer
{
public:
    AvmStage(DisplayObjContainer* pobj) : AvmDisplayObjContainer(pobj) {}
    virtual ~AvmStage();

    virtual void                OnEventLoad();
    virtual void                OnEventUnload();
    //virtual void                OnRemoved() {}
    virtual const char*         GetDefaultASClassName() const { return "flash.display.Stage"; }

    virtual bool                IsStage() const { return true; }
};


}}} // SF::GFx::AS3
#endif // INC_SF_GFx_AS3_AvmStage_H

