/**********************************************************************

Filename    :   GFx/AS3/AS2_MovieRoot.h
Content     :   Implementation of AS2 part of MovieImpl
Created     :   
Authors     :   Artem Bolgar, Michael Antonov

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFx_AS3_MovieRoot_H
#define INC_SF_GFx_AS3_MovieRoot_H

#include "GFx/GFx_Types.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_AS3Support.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_AvmStage.h"
#include "GFx/AS3/AS3_VM.h"
#include "GFx/GFx_Input.h"
#include "GFx/Text/Text_Core.h"

#include "GFx/AS3/AS3_Value.h"

#include "GFx/GFx_MediaInterfaces.h"

#ifdef GFX_ENABLE_XML
#include "GFx/XML/XML_Support.h"
#endif

#ifdef GFX_ENABLE_CSS
//#include "GFx/AS3/AS3_StyleSheet.h"
#endif

namespace Scaleform {
namespace GFx {

class TimelineDef;
class LogState;

namespace AS3 {

namespace Instances
{
    class URLRequest;
    class Loader;
    class URLLoader;
}

class AbcDataBuffer;

//  ****** MouseListener

// class MouseListener
// {
// public:
//     virtual ~MouseListener() {}
// 
//     virtual void OnMouseMove(GASEnvironment *penv, unsigned mouseIndex) const = 0;
//     virtual void OnMouseDown(GASEnvironment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const = 0;
//     virtual void OnMouseUp(GASEnvironment *penv, unsigned mouseIndex, unsigned button, InteractiveObject* ptarget) const   = 0;
//     virtual void OnMouseWheel(GASEnvironment *penv, unsigned mouseIndex, int sdelta, InteractiveObject* ptarget) const = 0;
// 
//     virtual bool IsEmpty() const = 0;
// };
class LoadQueueEntry : public GFx::LoadQueueEntry
{
public:
    SPtr<Instances::Loader>     mLoader;
    SPtr<Instances::URLLoader>  mURLLoader;
    SPtr<Instances::URLRequest> mURLRequest;
    bool                        FirstExec;
    //int                  Level;
    //Ptr<CharacterHandle> pCharacter;
    // movie clip loader and variables loader should be held by Value because 
    // there is a cross-reference inside of their instances: loader._listener[0] = loader. 
    // Value can release such cross-referenced objects w/o memory leak, whereas Ptr can't.
//    Value                MovieClipLoaderHolder;
//    Value                LoadVarsHolder;
// #ifdef GFX_ENABLE_XML
//     // the following structure is used to load xml files. the Value holds the
//     // actionscript object (explanation above). The loader is kept as a thread safe
//     // reference. this seperation is required because the loader object is used
//     // inside the loading thread in progressive loading mode. this causes problems
//     // with the Value reference.
//     struct XMLHolderType
//     {
//         Value                   ASObj;
//         Ptr<GFxASXMLFileLoader> Loader;
//     };
//     XMLHolderType       XMLHolder;
// #endif

// #ifdef GFX_ENABLE_CSS
//     // the following structure is used to load xml files. the Value holds the
//     // actionscript object (explanation above). The loader is kept as a thread safe
//     // reference. this seperation is required because the loader object is used
//     // inside the loading thread in progressive loading mode. this causes problems
//     // with the Value reference.
//     struct CSSHolderType
//     {
//         Value                   ASObj;
//         Ptr<GFxASCSSFileLoader> Loader;
//     };
//     CSSHolderType               CSSHolder;
// #endif

    // Constructor helper.      
    LoadQueueEntry(Instances::URLRequest* request, 
                   Instances::Loader* loader, 
                   LoadMethod method, bool quietOpen = false);
    LoadQueueEntry(Instances::URLRequest* request, 
                   Instances::URLLoader* loader, 
                   LoadMethod method, bool quietOpen = false);
    virtual ~LoadQueueEntry();

    void Cancel(LoadQueueEntry* e)
    {
        if (mLoader && mLoader == e->mLoader)
            Canceled = true;
        else if (mURLLoader && mURLLoader == e->mURLLoader)
            Canceled = true;
    }

// #ifdef GFX_ENABLE_XML
//     void CancelByXMLASObjPtr(Object* pxmlobj)
//     {
//         if (!XMLHolder.ASObj.IsUndefined() && pxmlobj == XMLHolder.ASObj.ToObject(NULL))
//             Canceled = true;
//     }
// #endif
// #ifdef GFX_ENABLE_CSS
//     void CancelByCSSASObjPtr(Object* pobj)
//     {
//         if (!CSSHolder.ASObj.IsUndefined() && pobj == CSSHolder.ASObj.ToObject(NULL))
//             Canceled = true;
//     }
// #endif
};

//  ****** GFxLoadingMovieEntry
class LoadQueueEntryMT_LoadMovie : public GFx::LoadQueueEntryMT
{
    friend class GFx::MovieImpl;

    Ptr<MoviePreloadTask>   pPreloadTask;
//    Ptr<Sprite>              pNewChar;
//    bool                     FirstCheck;
//    Ptr<InteractiveObject>   pOldChar;      
//    ResourceId               NewCharId;
    bool                     CharSwitched;
    unsigned                 BytesLoaded;
    bool                     FirstFrameLoaded;
public:
    LoadQueueEntryMT_LoadMovie(LoadQueueEntry* pqueueEntry, GFx::MovieImpl* pmovieRoot);
    ~LoadQueueEntryMT_LoadMovie();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
    bool IsPreloadingFinished();

};

// Memory context implementation specific to AS2
class MemoryContextImpl : public MemoryContext
{
public:
    MemoryHeap*                 Heap;
#ifdef GFX_AS_ENABLE_GC 
    Ptr<ASRefCountCollector>    ASGC;
#endif
    Ptr<ASStringManager>        StringMgr;
    Ptr<Text::Allocator>        TextAllocator;

    struct HeapLimit : MemoryHeap::LimitHandler
    {
        enum
        {
            INITIAL_DYNAMIC_LIMIT = (128*1024)
        };

        MemoryContextImpl*  MemContext;
        UPInt               UserLevelLimit;
        UPInt               LastCollectionFootprint;
        UPInt               CurrentLimit;
        float               HeapLimitMultiplier;

        HeapLimit() :  MemContext(NULL), UserLevelLimit(0), LastCollectionFootprint(0), 
            CurrentLimit(0), HeapLimitMultiplier(0.25) {}

        void Collect(MemoryHeap* heap);
        void Reset(MemoryHeap* heap);

        virtual bool OnExceedLimit(MemoryHeap* heap, UPInt overLimit);
        virtual void OnFreeSegment(MemoryHeap* heap, UPInt freeingSize);
    } LimHandler;

    MemoryContextImpl() : Heap(NULL)
    {
        LimHandler.MemContext = this;
    }
    ~MemoryContextImpl()
    {
        Heap->SetLimitHandler(NULL);
    }
};


#if 0
#ifdef GFX_ENABLE_XML
// ****** GFxLoadXMLTask
// Reads a file with loadXML data on a separate thread
class GFxAS2LoadXMLTask : public Task
{
public:
    GFxAS2LoadXMLTask(LoadStates* pls, const String& level0Path, const String& url, 
        GFxAS2LoadQueueEntry::XMLHolderType xmlholder);

    virtual void    Execute();

    bool IsDone() const;

private:
    Ptr<LoadStates>         pLoadStates;
    String                  Level0Path;
    String                  Url;
    Ptr<GFxASXMLFileLoader> pXMLLoader;

    volatile unsigned       Done;
};

class GFxAS2LoadQueueEntryMT_LoadXML : public LoadQueueEntryMT
{
    Ptr<GFxAS2LoadXMLTask> pTask;
    Ptr<LoadStates>        pLoadStates;
    MovieRoot*             pASMovieRoot;

public:
    GFxAS2LoadQueueEntryMT_LoadXML(LoadQueueEntry* pqueueEntry, MovieRoot* pmovieRoot);
    ~GFxAS2LoadQueueEntryMT_LoadXML();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
};
#endif


#ifdef GFX_ENABLE_CSS
// ****** GFxLoadCSSTask
// Reads a file with loadXML data on a separate thread
class GFxAS2LoadCSSTask : public Task
{
public:
    GFxAS2LoadCSSTask(LoadStates* pls, const String& level0Path, const String& url, 
        LoadQueueEntry::CSSHolderType xmlholder);

    virtual void    Execute();

    bool IsDone() const;

private:
    Ptr<LoadStates>         pLoadStates;
    String                  Level0Path;
    String                  Url;
    Ptr<ASCSSFileLoader> pLoader;

    volatile unsigned       Done;
};

class GFxAS2LoadQueueEntryMT_LoadCSS : public LoadQueueEntryMT
{
    Ptr<GFxLoadCSSTask> pTask;
    Ptr<LoadStates>     pLoadStates;

public:
    GFxAS2LoadQueueEntryMT_LoadCSS(LoadQueueEntry* pqueueEntry, GFx::MovieImpl* pmovieRoot);
    ~GFxAS2LoadQueueEntryMT_LoadCSS();

    // Check if a movie is loaded. Returns false in the case if the movie is still being loaded.
    // Returns true if the movie is loaded completely or in the case of errors.
    bool LoadFinished();
};
#endif

#endif // #if 0

// Extension of AS3::VM to hold pMovieRoot and to provide access
// to it from C++ implementations of AS3 classes.
class ASVM : public VM
{
protected:
    MovieRoot*          pMovieRoot;
public:
    SPtr<Object>        GraphicsClass;
	SPtr<Object>        TransformClass;
	SPtr<Object>		MatrixClass;
	SPtr<Object>        ColorTransformClass;
    SPtr<Object>        EventClass;
    SPtr<Object>        MouseEventClass;
    SPtr<Object>        MouseEventExClass;
    SPtr<Object>        KeyboardEventClass;
    SPtr<Object>        KeyboardEventExClass;
    SPtr<Object>        FocusEventClass;
    SPtr<Object>        FocusEventExClass;
    SPtr<Object>        TextEventClass;
    SPtr<Object>        TextEventExClass;
    SPtr<Object>        TimerEventClass;
    SPtr<Object>        ProgressEventClass;
    SPtr<Object>        PointClass;
    SPtr<Object>        RectangleClass;
    SPtr<Object>        TextFormatClass;
    SPtr<Object>        EventDispatcherClass;

    bool                ExtensionsEnabled;

    class AbcFileWithMovieDef : public Abc::File
    {
    public:
        MovieDefImpl*           pDefImpl;
        const AbcDataBuffer*    pAbcData;

        AbcFileWithMovieDef(MovieDefImpl* pdefImpl, const AbcDataBuffer* data)
            : pDefImpl(pdefImpl), pAbcData(data) {}
    };

private:
    bool                    _constructInstance(SPtr<Object> &pobj, Object* classObj,
                                               unsigned argc, const Value* argv);
public:
    ASVM(MovieRoot* pmr, FlashUI& _ui, FileLoader& loader, AS3::StringManager& sm, ASRefCountCollector& gc);

    MovieRoot*              GetMovieRoot() const { return pMovieRoot; }
    inline GFx::MovieImpl*  GetMovieImpl() const;

    inline MemoryHeap*      GetHeap() const;
    
    virtual AMP::ViewStats* GetAdvanceStats() const;
    inline Log*             GetLog() const;
    inline LogState*        GetLogState() const;


    inline const ASString&  GetBuiltin(BuiltinType btype) const;

    template <class T>
    bool                    ConstructInstance(SPtr<T> &pobj, Object* classObj,
                                              unsigned argc = 0, const Value* argv = 0)
    {
        SF_ASSERT(((UPInt)static_cast<Object*>((T*)0)) == 0);
        SF_ASSERT(!IsException());
        return _constructInstance(
            reinterpret_cast<SPtr<Object>&>(pobj), classObj, argc, argv);
    }

    // returns a moviedef for the last abc in call stack
    MovieDefImpl*           GetResourceMovieDef(Instances::Object* instance) const;
};

// This class represents chains of display object, which
// need to receive certain events. An example of such chain - 
// enterFrame event. It is necessary to have a list (array) of 
// all objects that can receive enterFrame event.
// Another events like this are 'activate', 'deactivate', 'render'
class EventChains
{
    struct Chain : ArrayLH<Ptr<DisplayObject> > {};
    HashIdentityLH<int, AutoPtr<Chain> > Chains;

    UPInt FindObjectIndexInChain(Chain* chain, DisplayObject* obj);

public:
    void AddToChain(EventId::IdCode evtId, DisplayObject* obj);
    void RemoveFromChain(EventId::IdCode evtId, DisplayObject* obj);
    const ArrayLH<Ptr<DisplayObject> >* GetChain(EventId::IdCode evtId) const;
    void Dispatch(EventId::IdCode evtId);
    void QueueEvents(EventId::IdCode evtId);
};

// Implementation of ASMovieRootBase for AS 3.0. The pointer to this instance
// is stored inside of MovieImpl.
class MovieRoot :   public ASMovieRootBase, 
                    public FlashUI, 
                    public FileLoader,
                    public KeyboardState::IListener
{
    friend class AvmSprite;
    friend class Stage;
    friend class LoadQueueEntryMT_LoadMovie;
protected:
    struct MemContextPtr : public Ptr<MemoryContextImpl>
    {
        MemContextPtr(MemoryContextImpl* p) : Ptr<MemoryContextImpl>(p) {}
        MemContextPtr(const MemContextPtr& p) : Ptr<MemoryContextImpl>(p) {}
        MemContextPtr(Ptr<MemoryContextImpl>& p) : Ptr<MemoryContextImpl>(p) {}
        ~MemContextPtr()
        {
            // Make sure no uncollected objects left in garbage collector.
            // Should be called after VM is destroyed but before MovieRoot does.
            GetPtr()->ASGC->ForceCollect();
        }
    };
    // Memory heap context
    MemContextPtr       MemContext; // must be declared first!
    AutoPtr<ASVM>       pAVM;       // must be declared second!
public:
    enum ActionLevel
    {
        AL_Highest      = 0,
        AL_High,
        AL_EnterFrame,
        AL_Frame,
        AL_ControllerEvents,
        AL_Render,

        AL_Count_,
        AL_Lowest      = AL_Count_ - 1
    };
    struct MouseState
    {
        enum
        {
            MouseButton_Left = GFx::MouseState::MouseButton_Left,
            MouseButton_Middle = GFx::MouseState::MouseButton_Middle,
            MouseButton_Right = GFx::MouseState::MouseButton_Right,
            MouseButton_MaxNum = GFx::MouseState::MouseButton_MaxNum
        };

        ArrayLH<Ptr<InteractiveObject>, StatMV_ActionScript_Mem>    RolloverStack;
        Ptr<InteractiveObject>                                      LastMouseOverObj;
        struct DoubleClickInfo
        {
            UInt32 PrevClickTime;
            PointF PrevPosition;

            DoubleClickInfo():PrevClickTime(0) { }
        }                                                           DblClick[MouseButton_MaxNum];

        DoubleClickInfo& GetDoubleClickInfo(unsigned buttonMask);
    };

    // Return value storage for ExternalInterface.call.
    Value                   ExternalIntfRetVal;

protected:

    // Keep track of the number of advances for shared GC
    unsigned                NumAdvancesSinceCollection;
    unsigned                LastCollectionFrame;

    ASStringHash<Value>*    pInvokeAliases; // aliases set by ExtIntf.addCallback

protected:
    // Action queue is stored as a singly linked list queue. List nodes must be traversed
    // in order for execution. New actions are inserted at the insert location, which is
    // commonly the end; however, in some cases insert location can be modified to allow
    // insertion of items before other items.
    // Action list to be executed 
    struct ActionEntry : public NewOverrideBase<StatMV_ActionScript_Mem>
    {
        enum EntryType
        {
            Entry_None,
            Entry_Event,        // 
            Entry_Function
        };

        ActionEntry*        pNextEntry;
        EntryType           Type;
        Ptr<DisplayObject>  pCharacter;
        EventId             mEventId;
        Value               Function;

        SF_INLINE ActionEntry();
        SF_INLINE ActionEntry(const ActionEntry &src);
        SF_INLINE const ActionEntry& operator = (const ActionEntry &src);

        // Helper constructors
        //SF_INLINE ActionEntry(InteractiveObject *pcharacter, GASActionBuffer* pbuffer);
        SF_INLINE ActionEntry(DisplayObject *pcharacter, const EventId& id);
        //SF_INLINE ActionEntry(InteractiveObject *pcharacter, const GASFunctionRef& function, const GASValueArray* params = 0);
        //SF_INLINE ActionEntry(InteractiveObject *pcharacter, const GASCFunctionPtr function, const GASValueArray* params = 0);

        SF_INLINE void SetAction(DisplayObject *pcharacter, const EventId& id);
        SF_INLINE void SetAction(DisplayObject *pcharacter, const Value& v);
        SF_INLINE void ClearAction();

        // Executes actions in this entry
        void    Execute(MovieRoot *proot) const;

        bool operator==(const ActionEntry&) const;
    };
    struct ActionQueueEntry
    {
        // This is a root of an action list. Root node action is 
        // always Entry_None and thus does not have to be executed.
        ActionEntry*    pActionRoot;

        // This is an action insert location. In a beginning, &ActionRoot, afterwards
        // points to the node after which new actions are added.
        ActionEntry*    pInsertEntry;

        // Pointer to a last entry
        ActionEntry*    pLastEntry;

        ActionQueueEntry() { Reset(); }
        inline void Reset() { pActionRoot = pLastEntry = pInsertEntry = NULL; }
    };
    struct ActionQueueType
    {
        ActionQueueEntry    Entries[AL_Count_];
        // This is a modification id to track changes in queue during its execution
        int                 ModId;
        // This is a linked list of un-allocated entries.   
        ActionEntry*        pFreeEntry;
        unsigned            FreeEntriesCount;

        MemoryHeap*         pHeap;

        ActionQueueType(MemoryHeap* pheap);
        ~ActionQueueType();

        MemoryHeap*                 GetMovieHeap() const { return pHeap; }

        ActionEntry*                GetNewEntry();
        ActionEntry*                InsertEntry(ActionLevel lvl);
        ActionEntry*                PrependEntry(ActionLevel lvl);
        void                        AddToFreeList(ActionEntry*);
        void                        Clear();
        ActionEntry*                GetInsertEntry(ActionLevel lvl) const
        {
            return Entries[lvl].pInsertEntry;
        }
        ActionEntry*                SetInsertEntry(
            ActionLevel lvl, ActionEntry* pinsertEntry)
        {
            ActionEntry* pie = Entries[lvl].pInsertEntry;
            Entries[lvl].pInsertEntry = pinsertEntry;
            return pie;
        }
        ActionEntry*                FindEntry(ActionLevel lvl, const ActionEntry&);
    };
    struct ActionQueueIterator
    {
        int              ModId;
        ActionQueueType* pActionQueue;
        ActionEntry*     pCurEntry;
        ActionEntry*     pRootEntry;
        ActionEntry*     pLastEntry;
        ActionLevel      Level;

        ActionQueueIterator(ActionLevel lvl, ActionQueueType*, ActionEntry* rootEntry = NULL);
        ~ActionQueueIterator();

        const ActionEntry*          getNext();
    };

    ArrayLH<AutoPtr<Abc::File> >    AbcFiles;
    ActionQueueType                 ActionQueue;

    EventChains                     mEventChains;

//     ArrayLH<Ptr<DisplayObject> >    EnterFrameQueue;
// 
//     struct ActivateInfo
//     {
//         Ptr<DisplayObject>  Obj;
//         EventId::IdCode     Evt;
// 
//         ActivateInfo() {}
//         ActivateInfo(DisplayObject* d, EventId::IdCode e) : Obj(d), Evt(e) {}
//     };
//     ArrayLH<ActivateInfo>           ActivateDeactivateQueue;

    Ptr<Stage>                      pStage;
    StringManager                   BuiltinsMgr;

    ArrayLH<Ptr<DisplayObject> >    JustLoadedObjects;
    unsigned                        ASFramesToExecute;

    MouseState                      mMouseState[GFX_MAX_MICE_SUPPORTED];
    bool                            StageInvalidated;

    // AS2 implementation of StickyVarNode
    struct StickyVarNode : public GFx::MovieImpl::StickyVarNode
    {
        Value mValue;

        StickyVarNode(const ASString& name, const Value& value, bool permanent)
            : GFx::MovieImpl::StickyVarNode(name, permanent), mValue(value) { }
        StickyVarNode(const StickyVarNode &node)
            : GFx::MovieImpl::StickyVarNode(node), mValue(node.mValue) { }
        const StickyVarNode& operator = (const StickyVarNode &node)
        { GFx::MovieImpl::StickyVarNode::operator =(node); mValue = node.mValue; return *this; }
    };

    // A holder for sprites with timelines created by ActionScript ("new")
    typedef HashIdentityLH<DisplayObjContainer*, Ptr<DisplayObjContainer> > NewSpritesHashType;
    NewSpritesHashType              NewSprites;
    unsigned                        CheckClipsCnt;

    HashSetLH<Ptr<MovieDefImpl> >   LoadedMovieDefs;
public:

    MovieRoot(MemoryContextImpl* memContext, GFx::MovieImpl* pmovie, ASSupport* pas);
    ~MovieRoot();

    SF_INLINE MovieDefImpl*     GetMovieDefImpl() const { return pMovieImpl->GetMovieDefImpl(); }
    SF_INLINE void              SetMovieDefImpl(MovieDefImpl* p) const { pMovieImpl->SetMovieDefImpl(p); }
    SF_INLINE MemoryHeap*       GetMovieHeap() const { return pMovieImpl->GetMovieHeap(); }
    SF_INLINE AS3::StringManager* GetStringManager() { return &BuiltinsMgr; } // revise @TODO GetBuiltinsMgr?
    SF_INLINE Log*              GetLog() const { return pMovieImpl->GetLog(); }
    SF_INLINE LogState*         GetLogState() const { return pMovieImpl->GetLogState(); }
    const StringManager&        GetBuiltinsMgr() const { return BuiltinsMgr; } // revise @TODO GetStringManager?

    const MouseState* GetAS3MouseState(unsigned mouseIndex) const
    {
        if (mouseIndex >= GFX_MAX_MICE_SUPPORTED)
            return NULL;
        return &mMouseState[mouseIndex];
    }

    // fills path to the root swf file
    bool GetRootFilePath(String *ppath) const;

    // Load queue
protected:
    // Adds load queue entry based on parsed url and target path.
    SF_INLINE void      AddLoadQueueEntry(LoadQueueEntry *pentry)
    {
        pMovieImpl->AddLoadQueueEntry(pentry);
    }
    void                AddLoadQueueEntryMT(LoadQueueEntry *pqueueEntry);
//    void                AddMovieLoadQueueEntry(LoadQueueEntry* pentry);
//     void                AddLoadQueueEntry(const char* ptarget, const char* purl, Environment* env,
//         LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None,
//         MovieClipLoader* pmovieClipLoader = NULL);
public:
    void                AddNewLoadQueueEntry(
        Instances::URLRequest* urlRequest,
        Instances::Loader* loader,
        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);

    void                AddNewLoadQueueEntry(
        Instances::URLRequest* urlRequest,
        Instances::URLLoader* loader,
        LoadQueueEntry::LoadMethod method = LoadQueueEntry::LM_None);

    // Creates an GFx::Value::ObjectInterface from MovieImpl; called from constructor.
    void CreateObjectInterface(GFx::MovieImpl* movie);

    void AddJustLoadedObject(DisplayObject* dobj) { JustLoadedObjects.PushBack(dobj); }

    // Inserts an empty action and returns a pointer to it. Should call SetAction afterwards.   
    ActionEntry*        InsertEmptyAction(ActionLevel lvl) 
    { 
        return ActionQueue.InsertEntry(lvl); 
    }
    // Prepends (inserts at the begining) an empty action
    ActionEntry*        PrependEmptyAction(ActionLevel lvl) 
    { 
        return ActionQueue.PrependEntry(lvl); 
    }
    void*               GetActionQueueInsertionPoint(ActionLevel lvl) const
    {
        return ActionQueue.GetInsertEntry(lvl);
    }
    void                ExecuteActionQueue(ActionLevel lvl);
    // starts exectution from rootEntry->pNextEntry
    void                ExecuteActionQueue(ActionLevel lvl, void* rootEntry);
    void                ExecuteCtors();
    void                ExecuteActionQueues();
    // queue all planned frame actions; it does not execute them, use ExecuteActionQueues.
    void                QueueFrameActions();

    // FlashUI methods
    virtual void Output(FlashUI::OutputMessageType type, const char* msg);    
    virtual bool OnOpCode(const Abc::TOpCode& code,
                          Abc::TCodeOffset offset, Abc::Code::OpCode opcode);
#ifdef USE_WORDCODE
    virtual bool OnOpCode(const ArrayLH_POD<UInt8>& code,
                          Abc::TCodeOffset offset, Abc::Code::OpCode opcode);
#endif

    // FileLoader methods
    virtual UPInt GetSize() const;
    virtual const AS3::Abc::File& GetFile(UPInt n);
    virtual const AS3::Abc::File* GetFile(const ASString& name);
    void                RemoveAbc(MovieDefImpl*);

    //     virtual void        AddStickyVariable
    //         (const GASString& fullPath, const GFxValue &val, Movie::SetVarType setType) =0;
    virtual void        AdvanceFrame(bool nextFrame);
    virtual bool        AttachDisplayCallback(const char* ppathToObject,
        void (SF_CDECL *callback)(void* puser), void* puser) { SF_UNUSED3(ppathToObject, callback, puser); return false; }
    // Check for AVM; create it, if it is not created yet.
    virtual bool        CheckAvm();
    virtual void        ChangeMouseCursorType(unsigned mouseIdx, unsigned newCursorType);
    virtual void        ClearDisplayList();
    virtual void        DoActions();
    virtual InteractiveObject* FindTarget(const ASString& path) const { SF_UNUSED(path); return 0; }
    // forces garbage collection (if GC is enabled)
    virtual void        ForceCollect();
    // forces emergency garbage collection (if GC is enabled). This method is called
    // when heap is overflown. 
    virtual void        ForceEmergencyCollect();
    // Generate button events (onRollOver, onPress, etc)
    virtual void        GenerateMouseEvents(unsigned mouseIndex);

    virtual bool        Init(MovieDefImpl* pmovieDef);
    virtual void        NotifyMouseEvent(const InputEventsQueueEntry* qe, const GFx::MouseState& ms, int mi) 
    { SF_UNUSED3(qe, ms, mi); }
    virtual void        NotifyOnResize();
    virtual void        NotifyQueueSetFocus(InteractiveObject* ch, unsigned controllerIdx, FocusMovedType fmt);
    virtual void        NotifyTransferFocus(InteractiveObject* curFocused, InteractiveObject* pNewFocus, unsigned controllerIdx);
    virtual bool        NotifyOnFocusChange(InteractiveObject* curFocused, InteractiveObject* toBeFocused, 
                                            unsigned controllerIdx, FocusMovedType fmt, ProcessFocusKeyInfo* pfocusKeyInfo = NULL);

    // invoked when whole movie view gets or loses focus
    virtual void        OnMovieFocus(bool set);
    // called BEFORE processing complete (non-partial) AdvanceFrame.
    virtual void        OnNextFrame(); 

    virtual MemoryContext* GetMemoryContext() const
    {
        return MemContext;
    }

    virtual Text::Allocator* GetTextAllocator();

    virtual void        ProcessLoadQueueEntry(GFx::LoadQueueEntry *pentry, LoadStates* pls);
    virtual void        ProcessLoadVarsMT
        (GFx::LoadQueueEntry *pentry, LoadStates* pls, const String& data, 
        UPInt fileLen, bool succeeded);

    // Register AS classes defined in aux libraries
    virtual void        RegisterAuxASClasses() {}
    virtual void        ResolveStickyVariables(InteractiveObject *pcharacter);
    virtual void        Restart();
    virtual void        SetExternalInterfaceRetVal(const GFx::Value&);
    virtual void        SetMemoryParams(unsigned frameBetweenCollections, unsigned maxRootCount) 
    { SF_UNUSED2(frameBetweenCollections, maxRootCount); }
    virtual void        Shutdown();

    // Value conversion for external communication
    void                GFxValue2ASValue(const GFx::Value& gfxVal, Value* pdestVal);
    void                ASValue2GFxValue(const Value& value, GFx::Value* pdestVal) const;

    // External API
    virtual void        CreateString(GFx::Value* pvalue, const char* pstring);
    virtual void        CreateStringW(GFx::Value* pvalue, const wchar_t* pstring);
    virtual void        CreateObject(GFx::Value* pvalue, const char* className = NULL,
                                     const GFx::Value* pargs = NULL, unsigned nargs = 0);
    virtual void        CreateArray(GFx::Value* pvalue);
    virtual void        CreateFunction(GFx::Value* pvalue, GFx::FunctionHandler* pfc, 
                                     void* puserData = NULL);

    virtual bool        GetVariable(GFx::Value *pval, const char* ppathToVar) const;
    virtual bool        SetVariable(const char* ppathToVar, const GFx::Value& value, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        GetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                    unsigned index, void* pdata, unsigned count);
    virtual unsigned    GetVariableArraySize(const char* ppathToVar);
    virtual bool        SetVariableArray(Movie::SetArrayType type, const char* ppathToVar,
                                    unsigned index, const void* pdata, unsigned count, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        SetVariableArraySize(const char* ppathToVar, unsigned count, 
                                    Movie::SetVarType setType = Movie::SV_Sticky);
    virtual bool        IsAvailable(const char* ppathToVar) const;
    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, 
                                    const GFx::Value* pargs, unsigned numArgs); 
    virtual bool        Invoke(const char* pmethodName, GFx::Value *presult, 
                                    const char* pargFmt, ...);
    virtual bool        InvokeArgs(const char* pmethodName, GFx::Value *presult, 
                                   const char* pargFmt, va_list args);

    // KeyboardState::IListener implementation
    // KeyboardState::IListener methods
    virtual void        OnKeyDown(InteractiveObject *pmovie, UInt32 code, UByte ascii, 
                                  UInt32 wcharCode, UInt8 keyboardIndex);
    virtual void        OnKeyUp(InteractiveObject *pmovie, UInt32 code, UByte ascii, 
                                UInt32 wcharCode, UInt8 keyboardIndex);
    virtual void        Update(UInt32 code, UByte ascii, UInt32 wcharCode, UInt8 keyboardIndex);

    Sprite*             CreateMovieClip(TimelineDef* pdef, MovieDefImpl* pdefImpl,
                                        InteractiveObject* parent, ResourceId id, 
                                        bool loadedSeparately = false);

    Sprite*             CreateSprite(TimelineDef* pdef, MovieDefImpl* pdefImpl,
                                     InteractiveObject* parent, ResourceId id, 
                                     bool loadedSeparately = false);

    AvmStage*           CreateStage(MovieDefImpl* pdefImpl);

    DisplayObjContainer*    GetMainMovie() const 
    { 
        return static_cast<DisplayObjContainer*>(pMovieImpl->GetMainMovie()); 
    }
    AvmDisplayObjContainer* GetAvmMainMovie() const 
    {
        return ToAvmDisplayObjContainer(GetMainMovie());
    }
    DisplayObjContainer* GetMainTimeline() const
    {
        return GetStage()->GetRoot();
    }
    bool                ExecuteAbc(const AbcDataBuffer*, MovieDefImpl* pdefImpl);
    ASVM*               GetAVM() const { return pAVM.GetPtr(); }
    SPtr<Instances::Event> CreateEventObject(const ASString& type, bool bubbles, bool cancelable);

    Stage*              GetStage() const { return pStage; }
    void                IncASFramesCnt(unsigned i) { ASFramesToExecute += i; }

    void                AddToEventChain(EventId::IdCode evtId, DisplayObject* obj)
    {
        mEventChains.AddToChain(evtId, obj);
    }
    void                RemoveFromEventChain(EventId::IdCode evtId, DisplayObject* obj)
    {
        mEventChains.RemoveFromChain(evtId, obj);
    }
    void                QueueEventChain(EventId::IdCode evtId)
    {
        mEventChains.QueueEvents(evtId);
    }

    // forces RENDER events to fire up
    void                InvalidateStage() { StageInvalidated = true; }
    void                ValidateStage()   { StageInvalidated = false; }
    bool                IsStageInvalidated() const { return StageInvalidated; }

    void                AddInvokeAlias(const ASString& alias, const Value& closure);
    Value*              ResolveInvokeAlias(const char* pstr);

    void                AddScriptableMovieClip(DisplayObjContainer*);
    void                CheckScriptableMovieClips();
protected:

    void                AddStickyVariable(const ASString& path, const ASString& name, 
                                          const Value &val, Movie::SetVarType setType);
    bool                ExtractPathAndName(const char* fullPath, ASString* ppath, 
                                           ASString* pname);

    bool                GetASVariableAtPath(Value* pval, const char* ppathToVar) const;
    void                ParseValueArguments(Array<Value>& arr, const char* pmethodName, 
                                            const char* pargFmt, va_list args) const;
};

// ** Inline Implementation


SF_INLINE MovieRoot::ActionEntry::ActionEntry()
{ 
    pNextEntry = 0;
    Type = Entry_None; 
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(const MovieRoot::ActionEntry &src)
{
    pNextEntry      = src.pNextEntry;
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    mEventId        = src.mEventId;
}

SF_INLINE const MovieRoot::ActionEntry& 
MovieRoot::ActionEntry::operator = (const MovieRoot::ActionEntry &src)
{           
    pNextEntry      = src.pNextEntry;
    Type            = src.Type;
    pCharacter      = src.pCharacter;
    mEventId        = src.mEventId;
    return *this;
}

SF_INLINE MovieRoot::ActionEntry::ActionEntry(DisplayObject *pcharacter, const EventId& id)
{
    pNextEntry      = 0;
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    mEventId        = id;
}

// SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, const GASFunctionRef& function, const GASValueArray* params)
// {
//     pNextEntry      = 0;
//     Type            = Entry_Function;
//     pCharacter      = pcharacter;
//     pActionBuffer   = 0;
//     Function        = function;
//     if (params)
//         FunctionParams = *params;
//     SessionId       = 0;
// }

// SF_INLINE MovieRoot::ActionEntry::ActionEntry(InteractiveObject *pcharacter, const GASCFunctionPtr function, const GASValueArray* params)
// {
//     pNextEntry      = 0;
//     Type            = Entry_CFunction;
//     pCharacter      = pcharacter;
//     pActionBuffer   = 0;
//     CFunction        = function;
//     if (params)
//         FunctionParams = *params;
//     SessionId       = 0;
// }

// SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, GASActionBuffer* pbuffer)
// {
//     Type            = Entry_Buffer;
//     pCharacter      = pcharacter;
//     pActionBuffer   = pbuffer;
//     mEventId.Id     = EventId::Event_Invalid;
// }

SF_INLINE void MovieRoot::ActionEntry::SetAction(DisplayObject *pcharacter, const EventId& id)
{
    Type            = Entry_Event;
    pCharacter      = pcharacter;
    mEventId        = id;
}

SF_INLINE void MovieRoot::ActionEntry::SetAction(DisplayObject *pcharacter, const Value& v)
{
    Type            = Entry_Function;
    pCharacter      = pcharacter;
    Function        = v;
}

// SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, const GASFunctionRef& function, const GASValueArray* params)
// {
//     Type            = Entry_Function;
//     pCharacter      = pcharacter;
//     pActionBuffer   = 0;
//     Function        = function;
//     if (params)
//         FunctionParams = *params;
// }
// 
// SF_INLINE void MovieRoot::ActionEntry::SetAction(InteractiveObject *pcharacter, const GASCFunctionPtr function, const GASValueArray* params)
// {
//     Type            = Entry_CFunction;
//     pCharacter      = pcharacter;
//     pActionBuffer   = 0;
//     CFunction       = function;
//     if (params)
//         FunctionParams = *params;
// }

SF_INLINE bool MovieRoot::ActionEntry::operator==(const MovieRoot::ActionEntry& e) const
{
    return Type == e.Type && pCharacter == e.pCharacter &&  mEventId == e.mEventId;
}

SF_INLINE void MovieRoot::ActionEntry::ClearAction()
{
    Type            = Entry_None;
    pCharacter      = 0;
    Function.SetUndefined();
}

SF_INLINE MovieRoot* ToAS3Root(Sprite* spr)
{
    SF_ASSERT(spr && spr->GetASMovieRoot() && spr->GetAVMVersion() == 2);
    return static_cast<MovieRoot*>(spr->GetASMovieRoot());
}

SF_INLINE MovieRoot* ToAS3Root(GFx::MovieImpl* pm)
{
    SF_ASSERT(pm->pASMovieRoot && pm->pASMovieRoot->GetAVMVersion() == 2);
    return static_cast<MovieRoot*>(pm->pASMovieRoot);
}

GFx::MovieImpl* ASVM::GetMovieImpl() const
{ 
    return pMovieRoot->GetMovieImpl(); 
}

MemoryHeap* ASVM::GetHeap() const
{
    return pMovieRoot->GetMovieHeap();
}

inline LogState* ASVM::GetLogState() const 
{ 
    SF_ASSERT(pMovieRoot); return pMovieRoot->GetLogState(); 
}
inline Log* ASVM::GetLog() const 
{ 
    SF_ASSERT(pMovieRoot); return pMovieRoot->GetLog(); 
}

inline const ASString& ASVM::GetBuiltin(BuiltinType btype) const
{
    return GetMovieRoot()->GetBuiltinsMgr().GetBuiltin(btype);
}

const ASString& GetEventString(const StringManager& mgr, const EventId& evt);

}}} // Scaleform::GFx::AS3

#endif // INC_SF_GFx_AS3_MovieRoot_H

