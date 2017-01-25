/**********************************************************************

Filename    :   ImagePacker.cpp
Content     :   
Created     :   
Authors     :   Andrew Reisse

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_ImagePacker.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace GFx {

using namespace Render;


ImagePacker* ImagePackParams::Begin(ResourceId* pIdGen, ImageCreator* pic, ImageCreateInfo* pici) const
{
    return new ImagePackerImpl(this, pIdGen, pic, pici);
}

void ImagePackerImpl::Add(ResourceDataNode* presNode, ImageBase* pimage)
{
    if (pimage && (pimage->GetFormat() == Render::Image_R8G8B8A8 ||
                   pimage->GetFormat() == Render::Image_R8G8B8))
    {
        InputImage in;
        in.pImage = pimage;
        in.pResNode = presNode;
        InputImages.PushBack(in);
        InputImages.Back().ImgData.Initialize(pimage->GetFormat(), pimage->GetSize().Width, pimage->GetSize().Height,
            pimage->GetSize().Width * 4);

        if (!pimage->Decode(&InputImages.Back().ImgData))
        {
            InputImages.PopBack();
            SF_DEBUG_WARNING(1, "ImagePacker: Decode failed, not packing");
        }
    }
    else
        SF_DEBUG_WARNING(1, "ImagePacker: ImageCreator did not provide uncompressed RGBA Render::Image, not packing");
}

void ImagePackerImpl::CopyImage(ImageFormat Format, ImagePlane* pdest, const ImagePlane* psrc, const RectPacker::RectType& rect)
{
    UByte* pdestrow = pdest->GetScanline(rect.y) + 4*rect.x;
    const UByte* psrcrow = psrc->GetScanline(0);
    unsigned destpitch = pdest->Pitch;
    unsigned srcpitch = psrc->Pitch;
    bool endx = (rect.x + psrc->Width + (rect.x ? 1 : 0) == pdest->Width) ? 0 : 1;
    bool endy = (rect.y + psrc->Height + (rect.y ? 1 : 0) == pdest->Height) ? 0 : 1;

    SF_ASSERT(rect.x + psrc->Width + endx <= pdest->Width);
    SF_ASSERT(rect.y + psrc->Height + endy <= pdest->Height);

    switch(Format)
    {
    case Render::Image_R8G8B8A8:
        if (rect.x == 0)
            pdestrow -= 4;
        if (rect.y > 0)
        {
            if (rect.x > 0) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
            memcpy(pdestrow+4, psrcrow, srcpitch);
            if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
            pdestrow += destpitch;
        }
        for (unsigned j = 0; j < psrc->Height; j++, pdestrow += destpitch, psrcrow += srcpitch)
        {
            if (rect.x > 0) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
            memcpy(pdestrow+4, psrcrow, psrc->Width*4);
            if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
        }
        if (endy)
        {
            psrcrow -= srcpitch;
            if (rect.x > 0) *((UInt32*) pdestrow) = *((const UInt32*) psrcrow);
            memcpy(pdestrow+4, psrcrow, srcpitch);
            if (endx) *((UInt32*) (pdestrow+4+psrc->Width*4)) = *((const UInt32*) (psrcrow+psrc->Width*4-4));
            pdestrow += destpitch;
        }
        break;

    case Render::Image_R8G8B8:
        destpitch -= psrc->Width*4 + (rect.x ? 4 : 0);
        srcpitch -= psrc->Width*3;
        if (rect.y > 0)
        {
            if (rect.x > 0)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
                pdestrow += 4;
            }
            for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
            }
            if (endx)
            {
                pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
            }
            pdestrow += destpitch;
            psrcrow -= psrc->Width*3;
        }
        for (unsigned j = 0; j < psrc->Height; j++, pdestrow += destpitch, psrcrow += srcpitch)
        {
            if (rect.x > 0)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
                pdestrow += 4;
            }
            for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
            }
            if (endx)
            {
                pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
            }
        }
        if (endy)
        {
            psrcrow -= psrc->Pitch;
            if (rect.x > 0)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
                pdestrow += 4;
            }
            for (unsigned i = 0; i < psrc->Width; i++, psrcrow+=3, pdestrow+=4)
            {
                pdestrow[0] = psrcrow[0]; pdestrow[1] = psrcrow[1]; pdestrow[2] = psrcrow[2];  pdestrow[3] = 255;
            }
            if (endx)
            {
                pdestrow[0] = psrcrow[-3]; pdestrow[1] = psrcrow[-2]; pdestrow[2] = psrcrow[-1];  pdestrow[3] = 255;
            }
        }
        break;

    default:
        SF_DEBUG_WARNING1(1, "GFx::ImagePacker: unsupported image format %d", Format);
        break;
    }
}

void ImagePackerImpl::Finish()
{
    ImagePackParams::TextureConfig PackTextureConfig;
    pImpl->GetTextureConfig(&PackTextureConfig);
    Packer.SetWidth (2+PackTextureConfig.TextureWidth);
    Packer.SetHeight(2+PackTextureConfig.TextureHeight);
    Packer.Clear();

    for (unsigned i = 0; i < InputImages.GetSize(); i++)
        Packer.AddRect(2+InputImages[i].pImage->GetSize().Width, 2+InputImages[i].pImage->GetSize().Height, i);

    Packer.Pack();
/*
    if (mImageCreateInfo.
        .pRenderConfig)
        PackTextureConfig.SizeOptions = (mImageCreateInfo.pRenderConfig->GetRendererCapBits() & Render::Renderer::Cap_TexNonPower2) ? 
        ImagePackParams::PackSize_1 : ImagePackParams::PackSize_PowerOf2;
*/
    for (unsigned i = 0; i < Packer.GetNumPacks(); i++)
    {
        unsigned imgWidth = 0, imgHeight = 0;
        const Render::RectPacker::PackType& pack = Packer.GetPack(i);
        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImagePlane imageIn;
            InputImages[rect.Id].ImgData.GetPlane(0, &imageIn);
            Render::ImageRect irect (rect.x ? rect.x+1 : 0, rect.y ? rect.y+1 : 0, imageIn.Width, imageIn.Height);

            if (irect.x2 >= imgWidth) imgWidth = irect.x2;
            if (irect.y2 >= imgHeight) imgHeight = irect.y2;
        }
        if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_PowerOf2)
        {
            unsigned w = 1; while (w < imgWidth) { w <<= 1; }
            unsigned h = 1; while (h < imgHeight) { h <<= 1; }
            imgWidth = w;
            imgHeight = h;
        }
        else if (PackTextureConfig.SizeOptions == ImagePackParams::PackSize_4)
        {
            imgHeight += 4 - (imgHeight & 3);
            imgWidth += 4 - (imgWidth & 3);
        }

        Ptr<RawImage> pPackImage = *RawImage::Create (Image_R8G8B8A8, 1, ImageSize(imgWidth, imgHeight), 0);
        ImageData     PackImageData;
        ImagePlane    PackImagePlane0;
        pPackImage->GetImageData(&PackImageData);
        PackImageData.GetPlane(0, &PackImagePlane0);

        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImagePlane imageIn;
            InputImages[rect.Id].ImgData.GetPlane(0, &imageIn);
            CopyImage(Image_R8G8B8A8, &PackImagePlane0, &imageIn, rect);
        }

        ResourceId          textureId = pIdGen->GenerateNextId();

        //XXX mImageCreateInfo.SetTextureUsage(0);
        Ptr<ImageBase>   pimageInfo = *pImageCreator->CreateImage(mImageCreateInfo,
            Ptr<WrapperImageSource>(*new WrapperImageSource(pPackImage)));
        Ptr<ImageResource> pimageRes  = 
            *SF_HEAP_NEW(mImageCreateInfo.pHeap) ImageResource(pimageInfo.GetPtr(), Resource::Use_Bitmap);

        for (unsigned j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            Render::ImageBase* pimagein = InputImages[rect.Id].pImage;
            Render::ImageRect irect (rect.x ? rect.x+1 : 0, rect.y ? rect.y+1 : 0, pimagein->GetSize());
            ResourceDataNode *presNode = InputImages[rect.Id].pResNode;

            Ptr<Resource> pres = 
                *SF_HEAP_NEW(mImageCreateInfo.pHeap) GFxSubImageResource(pimageRes, textureId,                                                                      
                                                                      irect, mImageCreateInfo.pHeap);
            pBindData->SetResourceBindData(presNode, pres);
        }
    }
}
}} // namespace Scaleform { namespace GFx {
