/**********************************************************************

Filename    :   GFx/AS3/AS3Support.cpp
Content     :   Implementation of AS3 support state
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/GFx_LoadProcess.h"
#include "GFx/GFx_AS3Support.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_Button.h"
#include "GFx/GFx_Sprite.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_StaticText.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/GFx_TagLoaders.h"
#include "GFx/AS3/AS3_AbcDataBuffer.h"
#include "GFx/AS3/AS3_AvmSprite.h"
#include "GFx/AS3/AS3_AvmButton.h"
#include "GFx/AS3/AS3_AvmTextField.h"
#include "GFx/AS3/AS3_AvmBitmap.h"
#include "GFx/AS3/AS3_AvmStaticText.h"
#include "GFx/AMP/Amp_Server.h"

namespace Scaleform { namespace GFx {

namespace AS3
{

// Implementation of DoAbc SWF9+ tag (tag that contains ABC bytecode
// data).
class DoAbc : public ExecuteTag
{
public:
    Ptr<AbcDataBuffer> pAbc;

    virtual void    Execute(DisplayObjContainer* m)
    {
        SF_ASSERT(m);

        AvmSprite* avm = ToAvmSprite(m);
        if (!avm->GetAS3Root()->ExecuteAbc(pAbc, m->GetResourceMovieDef()))
        {
            if (avm->GetMovieImpl()->GetLog())
            {
                avm->GetMovieImpl()->GetLog()->
                    LogError("Can't execute ABC from %s", m->GetResourceMovieDef()->GetFileURL());
            }
        }

//         const GASEnvironment *penv = avm->GetASEnvironment();
//         if (pBuf && !pBuf->IsNull())
//         {
//             Ptr<GASActionBuffer> pbuff =
//                 *SF_HEAP_NEW(penv->GetHeap()) GASActionBuffer(penv->GetSC(), pBuf);
//             avm->AddActionBuffer(pbuff.GetPtr());
//         }
    }

    virtual void    ExecuteWithPriority(DisplayObjContainer* m, ActionPriority::Priority ) 
    { 
        Execute(m);
    }
    // Don't override because actions should not be replayed when seeking the Sprite.
    //void  ExecuteState(Sprite* m) {}

    // Tell the caller that we are an action tag.
    virtual bool    IsActionTag() const
    {
        return true;
    }

    void Trace(const char* str)
    {
        SF_UNUSED(str);
#ifdef SF_BUILD_DEBUG
        printf("  %s actions, file %s\n", str, pAbc->GetFileName());
#endif
    }
};

void    SF_STDCALL SymbolClassLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_UNUSED(tagInfo);
    SF_ASSERT(tagInfo.TagType == Tag_SymbolClass);

    Stream* pin     = p->GetStream();
    unsigned numOfRecs  = pin->ReadU16();

    p->LogParse("  SymbolClassLoader: num = %d\n", numOfRecs);

    for (unsigned i = 0; i < numOfRecs; ++i)
    {
        UInt16      id = pin->ReadU16();
        StringDH    name(p->GetLoadHeap());
        pin->ReadString(&name);

        p->LogParse("       id = %d, symbol = '%s'\n", id, name.ToCStr());

        ResourceId     rid(id);
        ResourceHandle hres;

        // id == 0 is a special case; in this case the name should
        // be associated with the root sprite (MainTimeline)
        if (id == 0 || p->GetResourceHandle(&hres, rid))
        {
            // Add export to the list.
            p->ExportResource(name, rid, hres);

            // Should we check export types?
            // This may be impossible with handles.
        }
        else         
        {
            // This is really a debug error, since we expect Flash files to
            // be consistent and include their exported characters.
            // SF_DEBUG_WARNING1(1, "Export loader failed to export resource '%s'",
            //                       psymbolName );
            p->LogError("SymbolClassLoader can't find Resource with id = %d, name = '%s'", 
                        id, name.ToCStr());
        }        
    }
}

void    SF_STDCALL DoAbcLoader(LoadProcess* p, const TagInfo& tagInfo)
{
    SF_ASSERT(tagInfo.TagType == Tag_DoAbc);
    Stream* pin     = p->GetStream();
    UInt32  flags   = pin->ReadU32();
    StringDH name(p->GetLoadHeap());
    pin->ReadString(&name);
    int     dataOffset  = pin->Tell();
    unsigned    dataSize    = tagInfo.TagLength - (dataOffset - tagInfo.TagDataOffset);

#ifdef GFX_VERBOSE_PARSE
    p->LogParse("  DoAbcLoader: flags = 0x%X, name = '%s', dataSize = %d\n", 
        flags, name.ToCStr(), dataSize);
    pin->LogBytes(dataSize);
    pin->SetPosition(pin->Tell() - dataSize);
#endif

    void*   pbuf = 
        SF_HEAP_ALLOC(Memory::GetGlobalHeap(), dataSize + sizeof(AbcDataBuffer) - 1, StatMD_ActionOps_Mem);
    Ptr<AbcDataBuffer> pabc = *new (pbuf) AbcDataBuffer(name, flags, dataSize);
    if (pin->ReadToBuffer(pabc->Data, pabc->DataSize) != (int)pabc->DataSize)
    {
        pin->LogError("Can't read completely ABCData at offset %d",
                       tagInfo.TagOffset);
        return;
    }

#ifdef SF_AMP_SERVER
    pabc->SwdHandle = p->GetLoadTaskData()->GetSwdHandle();
    pabc->SwfOffset = dataOffset;

    if (p->GetProcessInfo().Header.SWFFlags & MovieInfo::SWF_Compressed)
    {
        // Add the size of the 8-byte header, 
        // since that is not included in the compressed stream
        pabc->SwfOffset += 8;
    }

    //AmpServer::GetInstance().AddSwf(pabc->SwdHandle, "", p->GetFileURL());
#endif  // SF_AMP_SERVER

    DoAbc* da = p->AllocTag<DoAbc>();
    da->pAbc = pabc;
    p->AddInitAction(ResourceId(0), da);
    //p->AddExecuteTag(da);
    //p->GetDataDef_Unsafe()->GetPlaylist()
}

} // namespace AS3


AS3Support::AS3Support() 
: ASSupport(State_AS3Support) 
{
    RegisterTagLoader(Tag_SymbolClass,  AS3::SymbolClassLoader);
    RegisterTagLoader(Tag_DoAbc,        AS3::DoAbcLoader);
}

MovieImpl* AS3Support::CreateMovie(MemoryContext* memContext)
{
    AS3::MemoryContextImpl* memContextImpl = static_cast<AS3::MemoryContextImpl*>(memContext);
    MemoryHeap* pheap = memContextImpl->Heap;
    MovieImpl* pmovie           = SF_HEAP_NEW(pheap) MovieImpl(pheap);
    Ptr<ASMovieRootBase> pasmr = *SF_HEAP_NEW(pheap) AS3::MovieRoot(memContextImpl, pmovie, this);
    return pmovie;
}

MemoryContext* AS3Support::CreateMemoryContext(const char* heapName, 
                                               const MemoryParams& memParams,
                                               bool debugHeap)
{
    unsigned heapFlags = debugHeap ? MemoryHeap::Heap_UserDebug : 0;
    MemoryHeap::HeapDesc desc = memParams.Desc;
    desc.Flags |= heapFlags;
    desc.Flags |= MemoryHeap::Heap_FastTinyBlocks;
    desc.Flags |= MemoryHeap::Heap_ThreadUnsafe;
    // disable Heap_ThreadUnsafe for AMP, 
    // since we get all memory reports from display thread
    SF_AMP_CODE(desc.Flags &= ~MemoryHeap::Heap_ThreadUnsafe;)
    desc.HeapId = HeapId_MovieView;
    desc.Limit  = AS3::MemoryContextImpl::HeapLimit::INITIAL_DYNAMIC_LIMIT; // 128K is the initial dynamic limit

    MemoryHeap* heap = Memory::GetGlobalHeap()->CreateHeap(heapName, desc);
    AS3::MemoryContextImpl* memContext = SF_HEAP_NEW(heap) AS3::MemoryContextImpl();
    memContext->Heap = heap;
#ifdef GFX_AS_ENABLE_GC
    memContext->ASGC = *SF_HEAP_NEW(heap) AS3::ASRefCountCollector();
    memContext->ASGC->SetParams(memParams.FramesBetweenCollections, memParams.MaxCollectionRoots);
#endif
    memContext->StringMgr = *SF_HEAP_NEW(heap) ASStringManager(heap);

    memContext->LimHandler.UserLevelLimit        = memParams.Desc.Limit;
    memContext->LimHandler.HeapLimitMultiplier   = memParams.HeapLimitMultiplier;

    heap->SetLimitHandler(&memContext->LimHandler);
    heap->ReleaseOnFree(memContext);

    return memContext;
}



DisplayObjectBase* AS3Support::CreateCharacterInstance
    (MovieImpl* proot, 
    CharacterDef* pcharDef, 
    InteractiveObject* pparent, 
    ResourceId rid, 
    MovieDefImpl *pbindingImpl,
    CharacterDef::CharacterDefType type)
{
    //MovieImpl* proot = pparent->GetMovieImpl();    
    switch ((type != CharacterDef::Unknown) ? type : pcharDef->GetType())
    {
    case CharacterDef::Shape:
        {
            return SF_HEAP_NEW(proot->GetMovieHeap()) 
                AS3::ShapeObject(pcharDef, proot->pASMovieRoot, pparent, rid);
        }

    case CharacterDef::Sprite:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(Sprite) + 3) & ~3) + sizeof(AS3::AvmMovieClip),
                StatMV_ActionScript_Mem);
            Sprite* pspr = new (pm) 
                Sprite(static_cast<SpriteDef*>(pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);
            AS3::AvmMovieClip* avmSpr = new (pm + ((sizeof(Sprite) + 3) & ~3)) AS3::AvmMovieClip(pspr);
//            pspr->BindAvmObj(avmSpr);
            SF_UNUSED(avmSpr);
            return pspr;
        }
    case CharacterDef::EmptySprite:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(Sprite) + 3) & ~3) + sizeof(AS3::AvmSprite),
                StatMV_ActionScript_Mem);
            Sprite* pspr = new (pm) 
                Sprite(static_cast<SpriteDef*>(pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);
            AS3::AvmSprite* avmSpr = new (pm + ((sizeof(Sprite) + 3) & ~3)) AS3::AvmSprite(pspr);
            //            pspr->BindAvmObj(avmSpr);
            SF_UNUSED(avmSpr);
            return pspr;
        }
    case CharacterDef::Button:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(Button) + 3) & ~3) + sizeof(AS3::AvmButton),
                StatMV_ActionScript_Mem);
            Button* pbtn = new (pm)
                Button(static_cast<ButtonDef*>
                (pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);
            AS3::AvmButton* avmBtn = new (pm + ((sizeof(Button) + 3) & ~3)) AS3::AvmButton(pbtn);
            SF_UNUSED(avmBtn);
            return pbtn;
        }
    case CharacterDef::TextField:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(TextField) + 3) & ~3) + sizeof(AS3::AvmTextField),
                StatMV_ActionScript_Mem);
            TextField* ptxt = new (pm)
                TextField(static_cast<TextFieldDef*>
                (pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);
            AS3::AvmTextField* avmTxt = new (pm + ((sizeof(TextField) + 3) & ~3)) AS3::AvmTextField(ptxt);
            SF_UNUSED(avmTxt);
            return ptxt;
        }
    case CharacterDef::StaticText:
        {
            UByte* pm = (UByte*) SF_HEAP_ALLOC(proot->GetMovieHeap(), 
                ((sizeof(StaticTextCharacter) + 3) & ~3) + sizeof(AS3::AvmStaticText),
                StatMV_ActionScript_Mem);
            StaticTextCharacter* ptxt = new (pm)
                StaticTextCharacter(static_cast<StaticTextDef*>
                (pcharDef), pbindingImpl, proot->pASMovieRoot, pparent, rid);
            AS3::AvmStaticText* avmTxt = new (pm + ((sizeof(StaticTextCharacter) + 3) & ~3)) AS3::AvmStaticText(ptxt);
            SF_UNUSED(avmTxt);
            return ptxt;
        }
    case CharacterDef::Bitmap:
        {
            return SF_HEAP_NEW(proot->GetMovieHeap()) 
                AS3::AvmBitmap(proot->pASMovieRoot, pbindingImpl, pparent, rid);
        }
    default:;
    }
    SF_ASSERT(0); // unknown type!
    return NULL;

}

void  AS3Support::DoActions()
{

}


PlaceObject2Tag* AS3Support::AllocPlaceObject2Tag(LoadProcess* p, UPInt dataSz, UInt8)
{
    return p->AllocTag<PlaceObject2Tag>(dataSz);
}

PlaceObject3Tag* AS3Support::AllocPlaceObject3Tag(LoadProcess* p, UPInt dataSz)
{
    return p->AllocTag<PlaceObject3Tag>(dataSz);
}

RemoveObjectTag* AS3Support::AllocRemoveObjectTag(LoadProcess* p)
{
    return p->AllocTag<RemoveObjectTag>();
}

RemoveObject2Tag* AS3Support::AllocRemoveObject2Tag(LoadProcess* p)
{
    return p->AllocTag<RemoveObject2Tag>();
}

}} // namespace Scaleform { namespace GFx {
