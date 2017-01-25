/**********************************************************************

Filename    :   ButtonDef.cpp
Content     :   Button definiton
Created     :   Nov, 2009
Authors     :   Michael Antonov, Artem Bolgar
Notes       :   

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_ButtonDef.h"
#include "GFx/GFx_LoadProcess.h"
#include "Kernel/SF_Debug.h"

#ifdef GFX_ENABLE_SOUND
#include "GFx/Audio/GFx_Sound.h" 
#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_SoundTagsReader.h"
#endif // #ifdef GFX_ENABLE_SOUND

namespace Scaleform { namespace GFx {

using Render::Text::FilterDesc;

//
// GFxButtonRecord
//

// Return true if we read a record; false if this is a null record.
bool    ButtonRecord::Read(LoadProcess* p, TagType tagType)
{
    int flags = p->ReadU8();
    if (flags == 0) 
        return false;

    Stream*  pin = p->GetStream();

    pin->LogParse("-- action record:  ");
    Flags = 0;
    if (flags & 8)
    {
        Flags |= Mask_HitTest;
        pin->LogParse("HitTest ");
    }
    if (flags & 4)
    {
        Flags |= Mask_Down;
        pin->LogParse("Down ");
    }
    if (flags & 2)
    {
        Flags |= Mask_Over;
        pin->LogParse("Over ");
    }
    if (flags & 1)
    {
        Flags |= Mask_Up;
        pin->LogParse("Up ");
    }
    pin->LogParse("\n");

    int characterId = p->ReadU16();
    CharacterId     = ResourceId(characterId);   
    Depth           = p->ReadU16(); 
    pin->ReadMatrix(&ButtonMatrix);     

    pin->LogParse("   CharId = %d, Depth = %d\n", characterId, Depth);
    pin->LogParse("   mat:\n");
    pin->LogParseClass(ButtonMatrix);

    if (tagType == 34)
    {
        pin->ReadCxformRgba(&ButtonCxform);
        pin->LogParse("   cxform:\n");
        pin->LogParseClass(ButtonCxform);
    }

    // SWF 8 features.

    // Has filters.
    if (flags & 0x10)
    {       
        pin->LogParse("   HasFilters\n");
        // This loads only Blur and DropShadow/Glow filters only.
        FilterDesc filters[FilterDesc::MaxFilters];
        unsigned numFilters = LoadFilters(pin, filters, FilterDesc::MaxFilters);
        if (numFilters)
        {
            if (!mTextFilter)
                mTextFilter = *new TextFilter;
            for (unsigned i = 0; i < numFilters; ++i)
            {
                mTextFilter->LoadFilterDesc(filters[i]);
            }
        }
    }
    // Has custom blending.
    if (flags & 0x20)
    {       
        UByte   blendMode = pin->ReadU8();
        if ((blendMode < 1) || (blendMode>14))
        {
            SF_DEBUG_WARNING(1, "ButtonRecord::Read - loaded blend mode out of range");
            blendMode = 1;
        }
        BlendMode = (Render::BlendMode) blendMode;
        pin->LogParse("   HasBlending, %d\n", (int)BlendMode);
    }
    else
    {
        BlendMode = Render::Blend_None;
    }

    // Note: 'Use Bitmap Caching' toggle does not seem to be serialized to flags, perhaps 
    // because it makes no sense in button records, since they cannot be animated (?).

    return true;
}


//
// ButtonDef
//

ButtonDef::ButtonDef()
:
#ifdef GFX_ENABLE_SOUND
    pSound(NULL),
#endif// SF_NO_SOUND
    pScale9Grid(NULL),
    Menu(false)
// Constructor.
{
}


ButtonDef::~ButtonDef()
{
#ifdef GFX_ENABLE_SOUND
    delete pSound;
#endif  // SF_NO_SOUND
    delete pScale9Grid;
}

static void SkipButtonSoundDef(LoadProcess* p)
{
    Stream *in = p->GetStream();
    for (int i = 0; i < 4; i++)
    {
        UInt16 rid = p->ReadU16();
        if (rid == 0)
            continue;
        in->ReadUInt(2);    // skip reserved bits.
        in->ReadUInt(1);
        in->ReadUInt(1);
        bool HasEnvelope = in->ReadUInt(1) ? true : false;
        bool HasLoops = in->ReadUInt(1) ? true : false;
        bool HasOutPoint = in->ReadUInt(1) ? true : false;
        bool HasInPoint = in->ReadUInt(1) ? true : false;
        if (HasInPoint) in->ReadU32();
        if (HasOutPoint) in->ReadU32();
        if (HasLoops) in->ReadU16();
        if (HasEnvelope)
        {
            int nPoints = in->ReadU8();
            for (int i=0; i < nPoints; i++)
            {
                in->ReadU32();
                in->ReadU16();
                in->ReadU16();
            }
        }
    }
}

// Initialize from the given Stream.
void    ButtonDef::Read(LoadProcess* p, TagType tagType)
{
    SF_ASSERT(tagType == Tag_ButtonCharacter ||
        tagType == Tag_ButtonSound ||
        tagType == Tag_ButtonCharacter2);

    if (tagType == Tag_ButtonCharacter)
    {
        // Old button tag.

        // Read button GFxCharacter records.
        for (;;)
        {
            ButtonRecord r;
            if (r.Read(p, tagType) == false)
            {
                // Null record; marks the end of button records.
                break;
            }

            // Search for the depth and insert in the right location. This ensures
            // that buttons are always drawn correctly.
            unsigned i;
            for(i=0; i<ButtonRecords.GetSize(); i++)
                if (ButtonRecords[i].Depth > r.Depth)
                    break;
            ButtonRecords.InsertAt(i, r);
        }

        // Read actions.
        if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
        {
            if (p->GetLoadStates()->pAS2Support)
                p->GetLoadStates()->pAS2Support->ReadButtonActions(p, this, tagType);
            else
                p->LogError("GFx_ButtonLoader - AS2 support is not installed. Actions are skipped.");
        }
        else
            p->LogError("GFx_ButtonLoader - AS3 Button shouldn't have AS2 actions. Skipped.");
    }

    else if (tagType == Tag_ButtonSound)
    {
#ifdef GFX_ENABLE_SOUND
        SF_ASSERT(pSound == NULL); // redefinition button sound is error
        AudioBase* paudio = p->GetLoadStates()->GetAudio();
        if (paudio)
        {
            SF_ASSERT(paudio->GetSoundTagsReader());
            pSound = paudio->GetSoundTagsReader()->ReadButtonSoundDef(p);
        }
        else
        {
            SkipButtonSoundDef(p);
            p->LogScriptWarning("ButtonDef::Read - Audio library is not set. Skipping sound definitions.");
        }
#else
        SkipButtonSoundDef(p);
#endif // SF_NO_SOUND

    }

    else if (tagType == Tag_ButtonCharacter2)
    {
        // Read the menu flag.
        Menu = p->ReadU8() != 0;

        int Button2_actionOffset = p->ReadU16();
        int NextActionPos = p->Tell() + Button2_actionOffset - 2;

        // Read button records.
        for (;;)
        {
            ButtonRecord r;
            if (r.Read(p, tagType) == false)
            {
                // Null record; marks the end of button records.
                break;
            }
            // Search for the depth and insert in the right location.
            unsigned i;
            for(i=0; i<ButtonRecords.GetSize(); i++)
                if (ButtonRecords[i].Depth > r.Depth)
                    break;
            ButtonRecords.InsertAt(i, r);
        }

        if (Button2_actionOffset > 0)
        {
            if (!(p->GetFileAttributes() & MovieDef::FileAttr_UseActionScript3))
            {
                if (p->GetLoadStates()->pAS2Support)
                {
                    p->SetPosition(NextActionPos);
                    p->GetLoadStates()->pAS2Support->ReadButton2ActionConditions(p, this, tagType);
                }
                else
                    p->LogError("GFx_ButtonLoader - AS2 support is not installed. Actions are skipped.");
            }
            else
                p->LogError("GFx_ButtonLoader - AS3 Button shouldn't have AS2 actions. Skipped.");
        }
    }
}

}} //namespace Scaleform::GFx {

