/**********************************************************************

PublicHeader:   GFx
Filename    :   VideoBase.h
Content     :   GFx video interface
Created     :   June 2008
Authors     :   Maxim Didenko, Vladislav Merker

Copyright   :   (c) 2006-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_VIDEOBASE_H
#define INC_SF_GFX_VIDEOBASE_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_VIDEO

#include "Kernel/SF_Memory.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

namespace AS2 {
    class GlobalContext;
    class ASStringContext;
}
class Object;
class LoadProcess;
class Sprite;
class VideoPlayer;

namespace Video {

// ***** VideoBase
//
// This interface has four responsibilities:
// 1. Create an instance of a VideoPlayer interface when GFx library requires
//    a video play back. Each video requires its own VideoPlayer instance.
// 2. Attach the audio from the video source to a MovieClip object.
// 3. Register all Action Script objects that are required for video playback.
// 4. Read video stream tags from SWF file.
//
// This interface provides two ways for implementing background loading of 
// game data while a video file is playing.
// 1. Using a callback object which will notify a user when a video system needs 
//    to read a new chunk of video data and when the read operation has finished.
// 2. Using IsIORequired method to detect if the reading of video data is required 
//    and EnbaleIO method to disable/enable video read operations.
//
class VideoBase : public State
{
public:
    VideoBase() : State(State_Video) {}
    virtual ~VideoBase() {};

    // Create an instance of a video player.
    virtual VideoPlayer*    CreateVideoPlayer(MemoryHeap*, TaskManager*, FileOpenerBase*, Log*) = 0;

    // Callback interface for notifying the application then video a read operation
    // is required and when it has finished.
    class ReadCallback : public RefCountBase<ReadCallback, Stat_Video_Mem>
    {
    public:
        virtual ~ReadCallback() {}

        // Notify then a video read operation is required. Upon receiving this notification
        // the application should stop all its disk IO operations (and wait until they are really
        // finished). After returning form this method the video system will start file read 
        // operation immediately.
        virtual void OnReadRequested() = 0;
        // Notify then the video read operation has finished. Upon receiving this notification 
        // the application can resume its disk IO operations until it receives OnReadRequested 
        // notification.
        virtual void OnReadCompleted() = 0;
    };
    // Set a read call back instance to the video system.
    virtual void            SetReadCallback(ReadCallback*) = 0;

    // Query a video system if it need to perform any data read operation
    // as soon as this function returns true the application should immediately
    // stop all its disk IO operations and enable video read operations.
    virtual bool            IsIORequired() const = 0;
    // Enable/Disable video read operations.
    virtual void            EnableIO(bool) = 0;

    // Attach the audio from the video source (NetStream AS call) to a MovieClip
    // which holds this video source.
    virtual void            AttachAudio(Object*, Sprite*) = 0;

    virtual void            RegisterASClasses(AS2::GlobalContext&, AS2::ASStringContext&) = 0;
    virtual void            ReadDefineVideoStreamTag(LoadProcess*, const TagInfo&) = 0;

    // Get a pointer to the memory heap used by video system.
    virtual MemoryHeap*     GetHeap() const = 0;
};

} // namespace Scaleform::GFx::Video

inline void StateBag::SetVideo(VideoBase* ptr)
{ 
    SetState(State::State_Video, ptr); 
}

inline Ptr<VideoBase> StateBag::GetVideo() const
{ 
    return *(VideoBase*) GetStateAddRef(State::State_Video); 
}

}} // namespace Scaleform::GFx

#endif // GFX_ENABLE_VIDEO

#endif // INC_SF_GFX_VIDEOBASE_H
