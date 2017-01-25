/**********************************************************************

Filename    :   GFxMediaInterfaces.h
Content     :   SWF (Shockwave Flash) player library
Created     :   August, 2008
Authors     :   Maxim Didenko

Notes       :   
History     :   

Copyright   :   (c) 1998-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/


#ifndef INC_SF_GFX_MEDIAINTERFASES_H
#define INC_SF_GFX_MEDIAINTERFASES_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_RefCount.h"
#include "GFx/GFx_PlayerStats.h"

namespace Scaleform { namespace GFx {

class Render::Renderer;
class Render::Texture;
class VideoCharacter;
class Movie;

//////////////////////////////////////////////////////////////////////////
// VideoProvider Interface

// Action script 2 has two types of video providers NetStream (for playing 
// video content) and Camera (for capturing video content. 
// For now NetStream is only implemented

class VideoProvider : public RefCountBaseWeakSupport<VideoProvider, StatMV_Other_Mem>
{
public:
    virtual ~VideoProvider();

    virtual Render::Texture* GetTexture(int * pictureWidth, int* pictureHeight, 
                                 int * textureWidth, int* textureHeight) = 0;

    virtual void      Advance()            = 0;
    virtual void      Display(Render::Renderer* prenderer)  = 0;
    virtual void      Pause(bool pause)    = 0;

    virtual bool      IsActive()           = 0;
    virtual float     GetFrameTime()       = 0;

    virtual void               RegisterVideoCharacter(VideoCharacter* pvideo) = 0;
    virtual VideoCharacter* RemoveFirstVideoCharacter() = 0;

};

}} // namespace Scaleform { namespace GFx {

#endif // SF_NO_VIDEO

#endif // INC_SF_GFX_MEDIAINTERFASES_H
