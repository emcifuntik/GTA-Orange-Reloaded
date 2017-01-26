/**********************************************************************

Filename    :   SoundRenderer.cpp
Content     :   Sound renderer interface
Created     :   November, 2008
Authors     :   Andrew Reisse, Maxim Didenko, Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Sound_SoundRenderer.h"

#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace Sound {

SoundInfo::~SoundInfo()
{
}

SoundSample* SoundInfo::GetSoundSample(SoundRenderer *pplayer)
{
    if (pSoundSample && pSoundSample->IsDataValid())
    {
        SF_ASSERT(pSoundSample->GetSoundRenderer() == pplayer);
        return pSoundSample;
    }

    if (pSound)
    {
        if ((pSoundSample = *pplayer->CreateSampleFromData(pSound)))
        {
//            pTexture->AddChangeHandler(this);
        }
    }
    else
    {
        SF_DEBUG_WARNING(1, "SoundInfo::GetSoundSample failed, data not available.");
    }
    return pSoundSample;
}

bool SoundInfo::SetSoundData(SoundDataBase* ps)
{
    pSound = ps;
    pSoundSample = NULL;
    return 1;
}

unsigned SoundInfo::GetRate() const
{
    if (pSound)
        return pSound->GetRate();
    return 0;
}

void SoundInfo::ReleaseResource()
{
    pSoundSample = NULL;
}

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND
