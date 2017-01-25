/**********************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageCreator.h
Content     :   
Created     :   June 21, 2005
Authors     :   Michael Antonov

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   (c) 2005-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_File.h"

#include "Render/Render_Image.h"
#include "Render/Render_ImageFiles.h"
#include "GFx_ImageCreator.h"

namespace Scaleform { namespace GFx {

using Render::ImageFileReader;
using Render::ImageCreateArgs;
    
ImageCreator::ImageCreator(TextureManager* textureManager) 
: State(State_ImageCreator), pTextureManager(textureManager) 
{ 
}

// Looks up image for "img://" protocol.
Image* ImageCreator::LoadProtocolImage(const ImageCreateInfo& info, const String& url)
{
    SF_UNUSED2(info, url);
    return 0;
}

// Loads image from file.
Image* ImageCreator::LoadImage(const ImageCreateInfo& info, const String& url)
{
    ImageFileHandlerRegistry* registry = info.GetImageFileHandlerRegistry();
    if (!registry || !info.GetFileOpener())
        return 0;

    Ptr<File> file = *info.GetFileOpener()->OpenFile(url);

    ImageFileReader* reader;
    ImageCreateArgs args;
    if (registry->DetectFormat(&reader, file) != Render::ImageFile_Unknown)
    {
        // Get image source.
        args.pHeap      = info.pHeap;
        args.Use        = info.Use;
        args.pManager   = pTextureManager;

        Ptr<ImageSource> source = *reader->ReadImageSource(file, args);
        if (source)
        {
            return CreateImage(info, source);
        }
    }

    return registry->ReadImage(file, args);
}

// Loads exported image file.
Image* ImageCreator::LoadExportedImage(const ImageCreateExportInfo& info, const String& url)
{
    Image* result = LoadImage(info, url);

    // Fall-back trying ".dds", ".tga"
    if (!result && url.HasExtension())
    {
        if (url.GetExtension().ToLower() != ".dds")
        {
            String url2 = url;
            url2.StripExtension();
            url2 += ".dds";
            result = LoadImage(info, url2);
        }
    }

    return result;
}

// Creates image
//  - Returns image that matches expected protocol.
Image* ImageCreator::CreateImage(const ImageCreateInfo& info, ImageSource* source)
{
    ImageCreateArgs args;
    args.pHeap = info.pHeap;
    args.Use = info.Use;
    args.pManager = pTextureManager;

    return source->CreateCompatibleImage(args);

}

void ImageFileHandlerRegistry::AddHandler(ImageFileHandler* handler)
{
    Render::ImageFileHandlerRegistry::AddHandler(handler);
}

}} // Scaleform::GFx
