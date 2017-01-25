/**********************************************************************

Filename    :   Audio.cpp
Content     :   GFx audio support
Created     :   September 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "AS2/AS2_Action.h"
#include "AS2/Audio/AS2_SoundObject.h"
#include "GFx/GFx_Audio.h"
#include "GFx/Audio/GFx_SoundTagsReader.h"

namespace Scaleform { namespace GFx {

//////////////////////////////////////////////////////////////////////////
//

Audio::Audio(
    Sound::SoundRenderer* pplayer, float maxTimeDiffernce, unsigned checkFrameInterval, SyncTypeFlags syncType) :
    AudioBase(maxTimeDiffernce, checkFrameInterval, syncType), pPlayer(pplayer), pTagsReader(NULL)
{
    pTagsReader = SF_NEW SoundTagsReader;
}

Audio::~Audio()
{
    if (pTagsReader)
        pTagsReader->Release();
}

Sound::SoundRenderer* Audio::GetRenderer() const 
{ 
    return pPlayer.GetPtr(); 
}

SoundTagsReader* Audio::GetSoundTagsReader() const
{
    return pTagsReader;
}

void Audio::RegisterASClasses(AS2::GlobalContext& gc, AS2::ASStringContext& sc)
{
    gc.AddBuiltinClassRegistry<AS2::ASBuiltin_Sound, AS2::SoundCtorFunction>(sc, gc.pGlobal);
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND
