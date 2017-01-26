/**********************************************************************

PublicHeader:   GFx
Filename    :   GFx_ImageSupport.h
Content     :   
Created     :   Sep, 2010
Authors     :   Artem Bolgar

Notes       :   Redesigned to use inherited state objects for GFx 2.0.

Copyright   :   (c) 2005-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef SF_GFx_ImageSupport_H
#define SF_GFx_ImageSupport_H

#include "GFx/GFx_Loader.h"
#include "Render/ImageFiles/JPEG_ImageFile.h"

namespace Scaleform { namespace GFx {

class ZlibSupportBase;
using Render::ImageData;
using Render::ImageSize;
using Render::ImageFormat;
using Render::ImageUpdateSync;

// A memory image of JPEG + Zlibed alpha channel.
class MemoryBufferJpegImageWithZlibAlphas : public Render::JPEG::MemoryBufferImage
{
    Ptr<ZlibSupportBase> ZLib;
    unsigned ZlibAlphaOffset;
public:
    MemoryBufferJpegImageWithZlibAlphas(ZlibSupportBase* zlib, unsigned alphaPos, ImageFormat format, const ImageSize& size, unsigned use,
        unsigned textureArena, ImageUpdateSync* sync, 
        File* file, SInt64 filePos, UPInt length = 0)
        : Render::JPEG::MemoryBufferImage(format,size, use, textureArena, sync,
        file, filePos, length), ZLib(zlib), ZlibAlphaOffset(alphaPos)
    { }

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
};

// Image source for Zlib-compressed swf image
class ZlibImageSource : public Render::FileImageSource
{
public:
    enum SourceBitmapDataFormat
    {
        ColorMappedRGB,
        RGB16,
        RGB24,
        ColorMappedRGBA,
        RGBA
    };
protected:
    Ptr<ZlibSupportBase>    Zlib;
    UInt16                  ColorTableSize;
    SourceBitmapDataFormat  BitmapFormatId;
public:
    ZlibImageSource(ZlibSupportBase* zlib, File* file, const ImageSize& size, 
                    SourceBitmapDataFormat bmpFormatId, ImageFormat format = Render::Image_None, 
                    UInt16 colorTableSize = 0, UInt64 uncompressedLen = 0);

    ~ZlibImageSource() {}

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
    // Creates an image out of source that is compatible with argument Use,
    // updateSync and other settings.
    virtual Render::Image* CreateCompatibleImage(const Render::ImageCreateArgs& args);
    virtual unsigned    GetMipmapCount() const { return 1;}
};

// A memory image of lossless zlib-compressed image
class MemoryBufferZlibImage : public Render::MemoryBufferImage
{
public:
    typedef ZlibImageSource::SourceBitmapDataFormat SourceBitmapDataFormat;
protected:
    Ptr<ZlibSupportBase>    Zlib;
    UInt16                  ColorTableSize;
    SourceBitmapDataFormat  BitmapFormatId;
public:
    MemoryBufferZlibImage(ZlibSupportBase* zlib, ImageFormat format, const ImageSize& size, SourceBitmapDataFormat bmpFormatId, 
        UInt16 colorTableSize, unsigned use, unsigned textureArena, ImageUpdateSync* sync, 
        File* file, SInt64 filePos, UPInt length = 0)
        : Render::MemoryBufferImage(format,size, use, textureArena, sync, file, filePos, length),
        Zlib(zlib), ColorTableSize(colorTableSize), BitmapFormatId(bmpFormatId)
    { }

    virtual bool Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const;
};

}} // Scaleform::GFx

#endif
