/**********************************************************************

Filename    :   ImagePacker.h
Content     :   
Created     :   
Authors     :   Andrew Reisse

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_IMAGEPACKER_H
#define INC_SF_GFX_IMAGEPACKER_H

#include "GFx/GFx_MovieDef.h"
#include "GFx/GFx_ImageResource.h"
#include "Render/Render_RectPacker.h"

namespace Scaleform { namespace GFx {

// Track state of packer while binding a movie.

class ImagePacker : public RefCountBaseNTS<ImagePacker,Stat_Default_Mem>
{
public:
    virtual void SetBindData(MovieDefImpl::BindTaskData* pbinddata) = 0;
    virtual void Add(ResourceDataNode* presNode, Render::ImageBase* pimage) = 0;
    virtual void Finish() = 0;
};


class ImagePackerImpl : public ImagePacker
{
    struct InputImage
    {
        ResourceDataNode    *pResNode;                  // needed to update binding with subreference to packed texture
        Render::ImageBase   *pImage;
        Render::ImageData    ImgData;
    };
    ArrayLH<InputImage>             InputImages;
    const ImagePackParams*          pImpl;
    ResourceId*                     pIdGen;             // shared with FontTextures
    Render::RectPacker              Packer;
    ImageCreator*                   pImageCreator;
    ImageCreateInfo                 mImageCreateInfo;
    MovieDefImpl::BindTaskData*     pBindData;

    void CopyImage(Render::ImageFormat format, Render::ImagePlane* pdest, const Render::ImagePlane* psrc, const Render::RectPacker::RectType& rect);

public:
    ImagePackerImpl(const ImagePackParams* pimpl, ResourceId* pidgen, ImageCreator* pic, ImageCreateInfo* pici)
        : pImpl(pimpl), pIdGen(pidgen), pImageCreator(pic), mImageCreateInfo(*pici) { }

    virtual void SetBindData(MovieDefImpl::BindTaskData* pbinddata) { pBindData = pbinddata; }
    virtual void Add(ResourceDataNode* presNode, Render::ImageBase* pimage);
    virtual void Finish();
};

}} // namespace Scaleform { namespace GFx {

#endif
