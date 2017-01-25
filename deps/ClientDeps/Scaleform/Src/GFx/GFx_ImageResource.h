/**********************************************************************

PublicHeader:   GFx
Filename    :   ImageResource.h
Content     :   Image resource representation for GFxPlayer
Created     :   January 30, 2007
Authors     :   Michael Antonov

Notes       :   

Copyright   :   (c) 2005-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_IMAGERESOURCE_H
#define INC_SF_GFX_IMAGERESOURCE_H

#include "GFx/GFx_Resource.h"
#include "GFx/GFx_Loader.h"

#include "Kernel/SF_HeapNew.h"
#include "Render/Render_Image.h"

namespace Scaleform { namespace GFx {

// ***** ImageResource - image resource representation

struct ImageFileInfo : public ResourceFileInfo
{
    UInt16                  TargetWidth;
    UInt16                  TargetHeight;
    // Store Use, so that that we can pass it to ImageCreator 
    Resource::ResourceUse   Use;
    String                  ExportName;

    ImageFileInfo()
    {
        TargetHeight = TargetHeight = 0;
        Use = Resource::Use_Bitmap;
    }
    ImageFileInfo(const ImageFileInfo& other) : ResourceFileInfo(other)
    {
        TargetWidth = other.TargetWidth;
        TargetHeight = other.TargetHeight;
        Use = other.Use;
    }
};


class ImageResource : public Resource
{
protected:

    Ptr<Render::ImageBase>  pImage;
    // Key used to look up / resolve this object (contains file data).
    ResourceKey             Key;
    // What the resource is used for
    ResourceUse             UseType;

public:
    ImageResource(void* , ResourceUse use = Use_Bitmap)
    {
        //pImageInfo = pimage;
        UseType    = use;
    }
    ImageResource(Render::ImageBase* pimage, ResourceUse use = Use_Bitmap)
    {
        pImage     = pimage;
        UseType    = use;
    }
    ImageResource(Render::ImageBase* pimage, const ResourceKey& key, ResourceUse use = Use_Bitmap)
    {
        pImage      = pimage;
        Key         = key;
        UseType     = use;
    }

    // Returns the referenced image. All Render::ImageInfoBase derived classes are created
    // by ImageCreator::CreateImage or Render::ImageInfoBase::CreateSubImage. If the user
    // overrides both of those functions to return a custom class, they can use type-casts
    // to access they custom data structures.
    inline Render::ImageBase*  GetImage() const
    {
        return pImage.GetPtr();
    }
    inline void         SetImage(Render::ImageBase* pimage)
    {
        pImage = pimage;
    }

    inline unsigned    GetWidth() const     { return pImage ? pImage->GetSize().Width : 0;  }
    inline unsigned    GetHeight() const    { return pImage ? pImage->GetSize().Height : 0;  }

#ifdef SF_AMP_SERVER
    inline UPInt       GetBytes() const         { return pImage ? pImage->GetBytes() : 0;  }
    inline UPInt       IsExternal() const       { return pImage ? pImage->IsExternal() : false;  }
    inline UInt32      GetImageId() const       { return pImage ? pImage->GetImageId() : 0; }
    inline Render::ImageFormat GetImageFormat() const   { return pImage ? pImage->GetFormat() : Render::Image_None; }
#endif

    virtual ResourceId GetBaseImageId() { return ResourceId(0); }

    // Ref file info, if any.
    /*
        
    // MA: We may need to bring this back, however,
    // we can to do this by obtaining the info from key.
    // Instead, we would need to store it locally.

    const GFxImageFileInfo* GetFileInfo() const
    {
        // Image files
        return (const GFxImageFileInfo*)Key.GetFileInfo();
    }
    */

    const char* GetFileURL() const
    {
        // Image files
        return Key.GetFileURL();
    }

    // GImplement Resource interface.
    virtual ResourceKey GetKey()                        { return Key; }
    virtual ResourceUse GetImageUse()                   { return UseType; }
    virtual unsigned    GetResourceTypeCode() const     { return MakeTypeCode(RT_Image, UseType); }


    // *** Methods for creating image keys

    // Create a key for an image file.    
    static  ResourceKey  CreateImageFileKey(ImageFileInfo* pfileInfo,
                                               FileOpener* pfileOpener,
                                               ImageCreator* pimageCreator,
                                               MemoryHeap* pimageHeap);
};


struct SubImageResourceInfo : public RefCountBaseNTS<SubImageResourceInfo, Stat_Default_Mem>
{
    ResourceId          ImageId;
    Ptr<ImageResource>  Image;
    Render::ImageRect   Rect;

    UPInt  GetHashCode() const;
};

class SubImageResource : public ImageResource
{
    Render::ImageRect   Rect;
    ResourceId          BaseImageId;

public:
    SubImageResource(ImageResource* pbase, ResourceId baseid,
                        const Render::ImageRect& rect, MemoryHeap* pheap)
    :   ImageResource(
            Ptr<Render::Image>(*SF_HEAP_AUTO_NEW(pbase->GetImage()) Render::SubImage(static_cast<Render::Image*>(pbase->GetImage()), rect))),
        Rect(rect), BaseImageId(baseid)
    {
        SF_UNUSED2(pheap,pbase);
    }

    virtual ResourceId GetBaseImageId() { return BaseImageId; }
};

}} // namespace Scaleform { namespace GFx {

#endif

