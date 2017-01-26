/**********************************************************************

Filename    :   SoundEvent.h
Content     :   SoundEvent interface
Created     :   March 2010
Authors     :   Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_GSOUNDEVENT_H
#define INC_GSOUNDEVENT_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_SOUND

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_String.h"

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundEvent : public RefCountBase<SoundEvent,Stat_Sound_Mem>
{
public:
    SoundEvent() : pUserData(NULL) {}
    virtual ~SoundEvent() {}

    virtual void PostEvent(String event, String eventId = "") = 0;
    virtual void SetParam (String param, float paramValue, String eventId = "") = 0;

    void* pUserData;
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif // INC_GSOUNDEVENT_H
