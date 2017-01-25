/**********************************************************************

PublicHeader:   Sound_FMOD
Filename    :   SoundRendererFMOD.h
Content     :   SoundRenderer FMOD Ex implementation
Created     :   November, 2008
Authors     :   Andrew Reisse, Maxim Didenko, Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_GSOUNDRENDERERMOD_H
#define INC_GSOUNDRENDERERMOD_H

#include "Sound_SoundRenderer.h"
#ifdef GFX_ENABLE_SOUND

namespace FMOD
{
    class System;
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    class EventSystem;
#endif
}

namespace Scaleform { namespace Sound {

//////////////////////////////////////////////////////////////////////////
//

class SoundRendererFMOD : public SoundRenderer
{
public:
    static SoundRendererFMOD* SF_CDECL CreateSoundRenderer();

    virtual FMOD::System* GetFMODSystem() = 0;
#if defined(GFX_SOUND_FMOD_DESIGNER) && (defined(SF_OS_WIN32) || defined(SF_OS_MAC))
    virtual FMOD::EventSystem* GetFMODEventSystem() = 0;
#endif

    // Initialize FMOD sound renderer
    // if call_fmod_update is true them the SoundRenderer will call FMOD::System::update() method
    // from its Update() method
    // if threaded_update is true then the SoundRenderer will create a separate thread for retrieving 
    // audio data from video files. If this parameter is false audio data from video file will be retrieved
    // on the main thread from Update() method (which is called from GFxMovieRoot::Advance method)
    // (Xbox360 only) processor_core parameter allows specifying a processor core which will be used for 
    // SoundRenderer thread if it is created.
    virtual bool Initialize(FMOD::System* pd, bool call_fmod_update = true, bool threaded_update = true
#ifdef SF_OS_XBOX360
                          , int processor_core = 4
#endif
                           ) = 0;

    // Finalize can be called to stop SoundRenderer thread and release all internal objects if this is 
    // needs to be done before the SoundRenderer is destructed.
    virtual void Finalize() = 0;
};

}} // Scaleform::Sound

#endif // GFX_ENABLE_SOUND

#endif
