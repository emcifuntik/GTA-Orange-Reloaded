/**********************************************************************
PublicHeader:   GFx
Filename    :   GFx/AS2/AS2Support.h
Content     :   Implementation of AS2 support state
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#ifndef SF_GFX_AS2_SUPPORT_H
#define SF_GFX_AS2_SUPPORT_H
#include "GFx/GFx_Types.h"
#include "GFx/GFx_Resource.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

class MovieDefImpl;
class MovieImpl;
class PlaceObject2Tag;
class PlaceObject3Tag;
class RemoveObjectTag;
class RemoveObject2Tag;

class CharacterDef;
class InteractiveObject;
class DisplayObjectBase;
class MemoryContext;

// AS2 support implementation. This class implements ASSupport interface
// for AVM1 (AS 1.0/2.0)
class AS2Support : public ASSupport
{
public:
    AS2Support() : ASSupport(State_AS2Support) {}

    virtual MovieImpl* CreateMovie(MemoryContext* memContext);

    virtual MemoryContext* CreateMemoryContext(const char* heapName, 
                                               const MemoryParams& memParams, 
                                               bool debugHeap);

    // Creates a new instance of a character, ownership is passed to creator (character is
    // not automatically added to parent as a child; that is responsibility of the caller).
    // Default behavior is to create a GenericCharacter.
    //  - pbindingImpl argument provides the MovieDefImpl object with binding tables
    //    necessary for the character instance to function. Since in the new implementation
    //    loaded data is separated from instances, CharacterDef will not know what
    //    what MovieDefImpl it is associated with, so this data must be passed.
    //    This value is significant when creating GFxSprite objects taken from an import,
    //    since their MovieDefImpl is different from pparent->GetResourceMovieDef().
    virtual DisplayObjectBase* CreateCharacterInstance
        (MovieImpl* proot,
        CharacterDef* pcharDef, 
        InteractiveObject* pparent, 
        ResourceId rid, 
        MovieDefImpl *pbindingImpl,
        CharacterDef::CharacterDefType type = CharacterDef::Unknown);

    virtual void  DoActions();
    virtual UByte GetAVMVersion() const { return 1; }

    virtual PlaceObject2Tag*  AllocPlaceObject2Tag(LoadProcess*, UPInt dataSz, UInt8 swfVersion);
    virtual PlaceObject3Tag*  AllocPlaceObject3Tag(LoadProcess*, UPInt dataSz);
    virtual RemoveObjectTag*  AllocRemoveObjectTag(LoadProcess* p);
    virtual RemoveObject2Tag* AllocRemoveObject2Tag(LoadProcess* p);

    virtual void DoActionLoader(LoadProcess* p, const TagInfo& tagInfo);
    virtual void DoInitActionLoader(LoadProcess* p, const TagInfo& tagInfo);

    virtual void ReadButtonActions
        (LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType);
    virtual void ReadButton2ActionConditions
        (LoadProcess* p, ButtonDef* pbuttonDef, TagType tagType);
};
}} // namespace Scaleform::GFx

#endif //SF_GFX_AS2_SUPPORT_H
