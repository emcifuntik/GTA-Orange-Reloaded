/**********************************************************************

PublicHeader:   Render
Filename    :   TextureUtil.h
Content     :   Mipmap/Image conversion and generation functions
                used for texture implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_TextureUtil_H
#define INC_SF_Render_TextureUtil_H

#include "Render_Image.h"
#include "Render_ResizeImage.h"


namespace Scaleform { namespace Render {


// ***** Rescale Support

typedef ResizeImageType ImageRescaleType;

ImageRescaleType SF_STDCALL GetImageFormatRescaleType(ImageFormat format);
void    SF_STDCALL  RescaleImageData(ImageData& dest, ImageData& src,
                                     ImageRescaleType resizeType);


// ***** Scan-line Conversion

void    SF_STDCALL  ConvertImagePlane(const ImagePlane &dplane, const ImagePlane &splane,
                                      ImageFormat format, unsigned formatPlaneIndex,
                                      Image::CopyScanlineFunc copyScanline,
                                      Palette* pcolorMap, void* scanlineArg = 0);

void    SF_STDCALL  ConvertImageData(ImageData& dest, ImageData& src,
                                     Image::CopyScanlineFunc copyScanline,
                                     void* scanlineArg = 0);


// ***** Software Mip-map Generation

// Makes a next MipmapLevel based on image format.
// Source and destination ImagePlane(s) are allowed to be the same.

void    SF_STDCALL  GenerateMipLevel(ImagePlane& dplane, ImagePlane& splane,
                                     ImageFormat format, unsigned formatPlaneIndex);


// Obtains an image scaline conversion function which converts between the specified default formats.
Image::CopyScanlineFunc SF_STDCALL GetImageConvertFunc(ImageFormat destFormat, ImageFormat sourceFormat);

// ***** General scan-line conversion functions

void    SF_STDCALL  Image_CopyScanline32_SwapBR(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL    Image_CopyScanline32_RGBA_ARGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_RGBA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_BGRA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_ABGR(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline24_Extend_RGB_ARGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_BGRA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_RGBA_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);
void    SF_STDCALL  Image_CopyScanline32_Retract_ARGB_RGB(UByte* pd, const UByte* ps, UPInt size, Palette*, void*);

}};  // namespace Scaleform::Render

#endif
