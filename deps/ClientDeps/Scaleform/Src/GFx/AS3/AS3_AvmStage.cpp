/**********************************************************************

Filename    :   AvmStage.cpp
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

#include "GFx/AS3/AS3_AvmStage.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Kernel/SF_MsgFormat.h"

namespace Scaleform { namespace GFx { namespace AS3 {

class FrameCounter : public InteractiveObject
{
    Ptr<DisplayObjContainer>    pRoot;
public:
    FrameCounter(MovieDefImpl* pbindingDefImpl,
                 ASMovieRootBase* pasRoot,
                 InteractiveObject* pparent, 
                 ResourceId id)
        : InteractiveObject(pbindingDefImpl, pasRoot, pparent, id)
    { 
    }
    ~FrameCounter() {}

    virtual int             CheckAdvanceStatus(bool playingNow) 
    { 
        SF_UNUSED(playingNow); 
        return 1; 
    }
    virtual void            AdvanceFrame(bool nextFrame, float framePos = 0.0f); 
    virtual unsigned        GetLoadingFrame() const { return 1; }
    virtual CharacterDef*   GetCharacterDef() const { return NULL; }

    DisplayObjContainer*    GetRoot()
    {
        return pRoot;
    }
    void                    SetRoot(DisplayObjContainer* proot)
    {
        pRoot = proot;
    }

    virtual void            ForceShutdown () {}
    void                    AddScriptableMovieClip(Sprite* pspr);

    void                    QueueFrameActions();
protected:
    void    AddChildsFrameExecutionSnapshot(
        DisplayObjContainer* pcont);
};

void FrameCounter::AdvanceFrame(bool nextFrame, float framePos) 
{
    SF_UNUSED(framePos);
    MovieRoot* asroot = ToAS3Root(GetMovieImpl());
    if (nextFrame)
    {
        // need to make sure this is the last character in the playlist
        // since it will basically execute all AS for the frame.
        SF_ASSERT(!pPlayNext);
#ifdef GFX_TRACE_DIPLAYLIST_EVERY_FRAME
        GetMovieImpl()->GetMainMovie()->CharToDisplayObjContainer_Unsafe()->DumpDisplayList(0, "");
#endif
        asroot->CheckScriptableMovieClips();

        // need to execute constructors before queueing up the frames:
        // ctors might call addFrameScript.
        asroot->ExecuteCtors();

        asroot->ExecuteActionQueue(MovieRoot::AL_Highest);
        asroot->ExecuteActionQueue(MovieRoot::AL_High);

        // traverse through playlist backward, starting from the stage 
        // ("this") and queue up frames
        QueueFrameActions();
    }
}

void FrameCounter::QueueFrameActions()
{
    // traverse through playlist backward, starting from the stage 
    // ("this") and queue up frames
    for (InteractiveObject* cur = pPlayPrev; cur; cur = cur->pPlayPrev)
    {
        if (cur->IsSprite())
        {
            AvmSprite* aspr = ToAvmSprite(cur->CharToSprite_Unsafe());
            if (aspr->IsNeedExecuteFrameFlagSet())
            {
                aspr->QueueFrameScript(aspr->GetCurrentSprite());
                aspr->SetNeedExecuteFrameFlag(false);
            }
        }
    }
}

Stage::Stage (MovieDefImpl* pbindingDefImpl, 
              ASMovieRootBase* pasRoot,
              InteractiveObject* pparent, 
              ResourceId id) : 
DisplayObjContainer(pbindingDefImpl, pasRoot, pparent, id)
{
    SetLoaded(); // mark as loaded to call OnEventUnload
}

Stage::~Stage()
{
}

void Stage::OnInsertionAsLevel(int level)
{
    FrameCounterObj = *SF_HEAP_AUTO_NEW(this) FrameCounter(
        pDefImpl, GetASMovieRoot(), NULL, ResourceId(0));
    FrameCounterObj->AddToPlayList();
    DisplayObjContainer::OnInsertionAsLevel(level);
}

void Stage::ForceShutdown ()
{
    FrameCounterObj->RemoveFromPlayList();
    ClearDisplayList();
    if (pRoot) pRoot->ForceShutdown();
    pRoot = NULL;
    DisplayObjContainer::ForceShutdown();
}

// initialize the stage
void Stage::ExecuteFrame0Events()
{
    AS3::MovieRoot* pas3Root = ToAS3Root(GetMovieImpl());

    SF_ASSERT(GetNumChildren() == 0);

//    ToAvmDisplayObj(this)->GetAS3Root()->AddJustLoadedObject(this);

    // Create a root movieclip
    Ptr<Sprite> prootMovie = *static_cast<MovieRoot*>(GetASMovieRoot())->
        CreateMovieClip(pDefImpl->GetDataDef(), pDefImpl, this, ResourceId(0), true);

    if (!prootMovie)
    {
        return ;
    }
    prootMovie->SetLoaded(); // mark as loaded to call OnEventUnload

    // Flash behaves like mouseEnabled for root is set to false;
    // however, it traces this property set to "true". 
    // Need some more investigations here.
    prootMovie->SetMouseDisabledFlag(true);

    /*// run its init actions (DoAbc tags)
    TimelineDef::Frame initActionsFrame;        
    if (pDefImpl->GetDataDef()->GetInitActions(&initActionsFrame, 0) &&
        initActionsFrame.GetTagCount() > 0)
    {
        // Need to execute these actions.
        for (unsigned i= 0; i < initActionsFrame.GetTagCount(); i++)
        {
            ExecuteTag*  e = initActionsFrame.GetTag(i);
            e->Execute(prootMovie);
        }
    }*/

    prootMovie->ExecuteInitActionFrameTags(0);

    // add the root movie as a child of the stage. OnEventLoad will
    // call CreateASInstance for the root movie.
    //mDisplayList.AddEntryAtIndex(0, prootMovie);
    prootMovie->SetName(GetMovieImpl()->GetStringManager()->CreateConstString("root1"));
    SetRoot(prootMovie);
    
    //AddChild(prootMovie);
    Ptr<Render::TreeContainer> node = GetRenderContainer();
    mDisplayList.AddEntryAtIndex(this, unsigned(mDisplayList.GetCount()), prootMovie);
    GetMovieImpl()->GetRenderRoot()->Add(node);

    prootMovie->SetParent(this);
    prootMovie->AddToPlayList();

    pas3Root->IncASFramesCnt(ToAvmDisplayObj(this)->CreateASInstance(false));
    pas3Root->IncASFramesCnt(
        ToAvmDisplayObj(prootMovie)->CreateASInstance(false));

    prootMovie->ExecuteFrame0Events();

    DisplayObjContainer::ExecuteFrame0Events();

    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_Highest);
    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_High);
    while (pas3Root->ASFramesToExecute > 0)
    {
        unsigned cnt = pas3Root->ASFramesToExecute;
        pas3Root->ASFramesToExecute = 0;
        pas3Root->pAVM->ExecuteCode(cnt);  
        if (pas3Root->pAVM->IsException())
            pas3Root->pAVM->OutputAndIgnoreException();

        //pas3Root->ASFramesToExecute = 0;
    }

    // initiate added_to_stage for root
    ToAvmSprite(prootMovie)->OnAdded(true);

    // need to force frame script execution here, since
    // AvmSprite::ExecuteFrameTags didn't do this for MainTimeline
    // sprite due to delayed initialization of its class.
    //ToAvmSprite(prootMovie)->QueueFrameScript(0);
    GetMovieImpl()->InvalidateOptAdvanceList();

    FrameCounterObj->AdvanceFrame(true, 0); 
    //AdvanceFrame(true, 0); 
    ToAS3Root(GetMovieImpl())->ExecuteActionQueue(MovieRoot::AL_Frame);
}

void Stage::ExecuteFrameTags(unsigned )
{
}

// For AS3 we don't need to recursively propagate mouse event,
// Just call OnEvent for topmost entity.
void    Stage::PropagateMouseEvent(const EventId& id)
{
    Ptr<DisplayObjContainer> thisHolder(this); // PropagateMouseEvent may release "this"; preventing.

    if (id.Id == EventId::Event_MouseMove)
    {
        // Adjust x,y of this character if it is being dragged.
        if (GetMovieImpl()->IsMouseSupportEnabled())
            DisplayObjContainer::DoMouseDrag();
    }
/*
    // Handling mouse events for stage is tricky. Stage gets mouse events
    // only if nobody else gets the event.
    MouseState* ms = GetMovieImpl()->GetMouseState(id.MouseIndex);
    SF_ASSERT(ms);
    //?if (!ms->GetTopmostEntity())
    {
        // We need to check if there is a MouseMove event handler installed on
        // stage to avoid extra work every mouse move.
        GetAvmIntObj()->OnEvent(id);
    }*/
}

ASString Stage::CreateNewInstanceName() const
{
    MovieImpl* proot = GetMovieImpl();
    return ASString(proot->GetStringManager()->CreateEmptyString());
}

void  Stage::QueueFrameActions()
{
    static_cast<FrameCounter*>(FrameCounterObj.GetPtr())->QueueFrameActions();
}

void Stage::UpdateViewAndPerspective()
{
    if ( GetRoot() )
        GetRoot()->UpdateViewAndPerspective();
}

//////////////////////////////////////////////////////////////////////////
AvmStage::~AvmStage() 
{

}

void AvmStage::OnEventLoad()
{
//    GetAS3Root()->IncASFramesCnt(CreateASInstance(false));
    //AvmDisplayObjContainer::OnEventLoad();
    //static_cast<Stage*>(pDispObj)->AdvanceFrame(true, 0);
}

void AvmStage::OnEventUnload()
{
    AvmDisplayObjContainer::OnEventUnload();
}

}}} // SF::GFx::AS3


