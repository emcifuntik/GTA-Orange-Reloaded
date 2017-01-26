/**********************************************************************

Filename    :   SoundTagsReader.h
Content     :   GFx audio support
Created     :   February, 2009
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_SOUNDTAGSREADER_H
#define INC_SF_GFX_SOUNDTAGSREADER_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "GFx/Audio/GFx_Sound.h"

namespace Scaleform { namespace GFx {

class LoadProcess;
class ButtonSoundDef;

//////////////////////////////////////////////////////////////////////////
//

class SoundTagsReader : public RefCountBaseNTS<SoundTagsReader,StatMD_Sounds_Mem>
{
public:
    virtual ButtonSoundDef* ReadButtonSoundDef(LoadProcess* p);
    virtual void ReadDefineSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadStartSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadButtonSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadDefineExternalSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadDefineExternalStreamSoundTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadSoundStreamHeadTag(LoadProcess* p, const TagInfo& tagInfo);
    virtual void ReadSoundStreamBlockTag(LoadProcess* p, const TagInfo& tagInfo);
};

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_SOUND

#endif // INC_SF_GFX_SOUNDTAGSREADER_H
