/**********************************************************************

Filename    :   GL_Texture.cpp
Content     :   GL Texture and TextureManager implementation
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "GL_Texture.h"
#include "Render/Render_TextureUtil.h"
#include "Kernel/SF_Debug.h"

#if defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ExtensionMacros.h"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#endif

namespace Scaleform { namespace Render { namespace GL {


Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
    unsigned mipLevels, const ImageSize& size, unsigned use,
    unsigned arena, ImageBase* pimage)
    : pManagerLocks(pmanagerLocks), pImage(pimage),
    pFormat(pformat), ImgSize(size), MipLevels((UByte)mipLevels), Use((UInt16)use),
    TextureFlags(0), State(State_InitPending), pMap(0)
{
    SF_UNUSED(arena);

    // Not in linked list yet.
    pNext = pPrev = 0;

    TextureCount = (UByte) pformat->GetPlaneCount();
    if (TextureCount > 1)
    {
        pTextures = (HWTextureDesc*)
            SF_HEAP_AUTO_ALLOC(this, sizeof(HWTextureDesc) * TextureCount);
    }
    else
    {
        pTextures = &Texture0;
    }
    memset(pTextures, 0, sizeof(HWTextureDesc) * TextureCount);
}


Texture::~Texture()
{
    //  pImage must be null, since ImageLost had to be called externally.
    SF_ASSERT(pImage == 0);

    Mutex::Locker  lock(&pManagerLocks->TextureMutex);

    if ((State == State_Valid) || (State == State_Lost))
    {
        // pManagerLocks->pManager should still be valid for these states.
        SF_ASSERT(pManagerLocks->pManager);
        RemoveNode();
        pNext = pPrev = 0;
        // If not on Render thread, add HW textures to queue.
        ReleaseHWTextures();
    }

    if ((pTextures != &Texture0) && pTextures)
        SF_FREE(pTextures);
}

// LoseManager is called from ~TextureManager, cleaning out most of the
// texture internal state. Clean-out needs to be done during a Lock,
// in case ~Texture is called due to release in another thread.
void    Texture::LoseManager()
{        
    SF_ASSERT(pMap == 0);

    Lock::Locker lock(&pManagerLocks->ImageLock);

    RemoveNode();
    pNext = pPrev = 0;

    ReleaseHWTextures();
    State   = State_Dead;    
    pFormat = 0; // Users can't access Format any more ?

    if (pImage)
    {
        // TextureLost may release 'this' Texture, so do it last.
        SF_ASSERT(pImage->GetImageType() != Image::Type_ImageBase);
        Image* pimage = (Image*)pImage;
        pImage = 0;
        pimage->TextureLost(Image::TLR_ManagerDestroyed);
    }
}


Render::TextureManager* Texture::GetTextureManager() const
{
    return pManagerLocks->pManager;
}


bool Texture::Initialize()
{    
    bool            resize  = false;
    ImageFormat     format  = GetImageFormat();
    TextureManager* pmanager= GetManager();
    unsigned        itex;

    // Determine sizes of textures.
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];
        tdesc.Size = ImageData::GetFormatPlaneSize(format, ImgSize, itex);
        if (!pmanager->IsNonPow2Supported(format, Use))
        {
            ImageSize roundedSize = ImageSize_RoundUpPow2(tdesc.Size);
            if (roundedSize != tdesc.Size)
            {
                tdesc.Size = roundedSize;
                resize = true;
            }
        }        
    }

    // XXX padding to power of 2 not supported in core yet
    if (resize /*&& (Use & ImageUse_Wrap)*/)
    {        
        if (ImageData::IsFormatCompressed(format))
        {        
            SF_DEBUG_ERROR(1, 
                "CreateTexture failed - Can't rescale compressed Wrappable image to Pow2");
            State = State_InitFailed;
            return false;
        }
        TextureFlags |= TF_Rescale;
    }

    // Determine how many mipLevels we should have and whether we can
    // auto-generate them or not.
    unsigned allocMipLevels = MipLevels;
    bool genMipmaps = 0, allocMapBuffer = 0;

    if (Use & ImageUse_MapLocal)
        allocMapBuffer = 1;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        if (!(pmanager->GetTextureUseCaps(format) & ImageUse_GenMipmaps))
        {            
            TextureFlags |= TF_SWMipGen;
            // If using SW MipGen, determine how many mip-levels we should have.
            allocMipLevels = 31;
            for (itex = 0; itex < TextureCount; itex++)
                allocMipLevels = Alg::Min(allocMipLevels, ImageSize_MipLevelCount(pTextures[itex].Size));
            MipLevels = (UByte)allocMipLevels;
        }
        else
        {
            genMipmaps = 1;
            allocMipLevels = 1;
        }
    }

    if (allocMapBuffer)
    {
        pBackingImage = *RawImage::Create(format, MipLevels, pTextures[0].Size, 0);
    }

    LastMinFilter = LastAddress = 0;

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        glGenTextures(1, &tdesc.TexId);
        glBindTexture(GL_TEXTURE_2D, tdesc.TexId);

#if defined(GL_TEXTURE_MAX_LEVEL)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, MipLevels-1);
#endif

        if (!ImageData::IsFormatCompressed(format))
        {
            glTexImage2D(GL_TEXTURE_2D, 0, pFormat->GLColors, tdesc.Size.Width, tdesc.Size.Height, 0, pFormat->GLFormat, pFormat->GLData, 0);

            unsigned width = tdesc.Size.Width;
            unsigned height = tdesc.Size.Height;
            for (unsigned i = 1; i < allocMipLevels; i++)
            {
                width >>= 1;
                height >>= 1;
                if (width < 1)
                    width = 1;
                if (height < 1)
                    height = 1;
                glTexImage2D(GL_TEXTURE_2D, i, pFormat->GLColors, width, height, 0, pFormat->GLFormat, pFormat->GLData, 0);
            }
        }
    }

    if (genMipmaps)
    {
        glGenerateMipmapEXT(GL_TEXTURE_2D);
    }

    // Upload image content to texture, if any.
    if (pImage && !Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture contents");

        ReleaseHWTextures();
        State = State_InitFailed;
        return false;
    }

    State = State_Valid;
    return true;
}

bool Texture::Upload(unsigned itex, unsigned level, const ImagePlane& plane)
{
    glBindTexture(GL_TEXTURE_2D, pTextures[itex].TexId);

    if (ImageData::IsFormatCompressed(pFormat->Format))
    {
        glCompressedTexImage2D(GL_TEXTURE_2D, level, pFormat->GLColors, plane.Width, plane.Height, pFormat->GLFormat, pFormat->GLData, plane.pData);
        return true;
    }

    if (plane.Width * pFormat->BytesPerPixel == plane.Pitch)
    {
        if (plane.Width == pTextures[itex].Size.Width && plane.Height == pTextures[itex].Size.Height)
            glTexImage2D(GL_TEXTURE_2D, level, pFormat->GLColors, plane.Width, plane.Height, 0, pFormat->GLFormat, pFormat->GLData, plane.pData);
        else
            glTexSubImage2D(GL_TEXTURE_2D, level, 0,0, plane.Width, plane.Height, pFormat->GLFormat, pFormat->GLData, plane.pData);
    }
#if (defined(GL_UNPACK_ROW_LENGTH) && defined(GL_UNPACK_ALIGNMENT))
    else if (plane.Pitch == ((3 + plane.Width * pFormat->BytesPerPixel) & ~3))
    {
        glPixelStorei(GL_UNPACK_ROW_LENGTH, plane.Width);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

        if (plane.Width == pTextures[itex].Size.Width && plane.Height == pTextures[itex].Size.Height)
            glTexImage2D(GL_TEXTURE_2D, level, pFormat->GLColors, plane.Width, plane.Height, 0, pFormat->GLFormat, pFormat->GLData, plane.pData);
        else
            glTexSubImage2D(GL_TEXTURE_2D, level, 0,0, plane.Width, plane.Height, pFormat->GLFormat, pFormat->GLData, plane.pData);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
#endif
    else
    {
        if (!pBackingImage)
        {
            SF_DEBUG_WARNING(1, "GL::Texture - irregular size, cannot use upload, making texture mappable");
            MakeMappable();
        }

        unsigned width = pTextures[itex].Size.Width;
        unsigned height = pTextures[itex].Size.Height;

        for (unsigned i = 1; i < level; i++)
        {
            width >>= 1;
            height >>= 1;
            if (width < 1)
                width = 1;
            if (height < 1)
                height = 1;
        }

        UpdateDesc upd;
        upd.DestRect = ImageRect(0,0, width, height);
        upd.PlaneIndex = itex;
        upd.SourcePlane = plane;

        Update(&upd, 1, level);
    }

    return true;
}

bool Texture::Update()
{
    ImageFormat     format   = GetImageFormat();
    TextureManager* pmanager = GetManager();
    bool            rescale  = (TextureFlags & TF_Rescale) ? true : false;
    bool            swMipGen = (TextureFlags & TF_SWMipGen) ? true : false;
    bool            convert  = false;
    ImageData       *psource;
    ImageData       *pdecodeTarget = 0, *prescaleTarget = 0, *pconvertTarget = 0;
    ImageData       imageData1, imageData2;
    Ptr<RawImage>   pimage1, pimage2;
    unsigned        sourceMipLevels = GetMipmapCount(); // May be different from MipLevels    

    // Texture update proceeds in four (optional) steps:
    //   1. Image::Decode - Done unless rescaling directly from RawImage.
    //   2. Rescale       - Done if non-pow2 textures are not supported as necessary.
    //   3. Convert       - Needed if conversion wasn't applied in Decode.
    //   4. SW Mipmap Gen - Loop to generate SW mip-maps, may also have a convert step.

    // Although Decode can do scan-line conversion, Convert step is necessary
    // because our Rescale and GenerateMipLevel functions don't support all GL
    // destination formats. If non-supported format is encountered, conversion
    // is delayed till after rescale (which, of course, requires an extra image buffer).

    ImageFormat      rescaleBuffFromat = pFormat->ConvFormat;
    ImageRescaleType rescaleType = ResizeNone;

    if (rescale)
    {
        if (pFormat->BytesPerPixel == 4)
            rescaleType = ResizeRgbaToRgba;
        else if (pFormat->BytesPerPixel == 1)
            rescaleType = ResizeGray;
        else
        {
            rescaleBuffFromat = Image_R8G8B8A8;
            convert = true;
        }
    }
    if (swMipGen && !(pFormat->BytesPerPixel == 4 || pFormat->BytesPerPixel == 1))
        convert = true;

    // *** 1. Decode from source pImage to Image1/MappedTexture

    Lock::Locker  imageLock(&pManagerLocks->ImageLock);

    if (!pImage)
        return false;

    // Decode is not needed if RawImage is used directly as a source for rescale.
    if (! ((pImage->GetImageType() == Image::Type_RawImage) && rescale) )
    {
        // Determine decodeTarget -> Image1 if rescale / convert will take place
        if (rescale || convert || !pmanager->mapTexture(this))
        {
            pimage1 = *RawImage::Create(rescaleBuffFromat, sourceMipLevels, ImgSize, 0);
            if (!pimage1) return false;
            pimage1->GetImageData(&imageData1);
            imageData1.Format = (ImageFormat)(format | ImageFormat_Convertible);
            pdecodeTarget = &imageData1;
        }
        else
        {
            pdecodeTarget = &pMap->Data;
        }

        // Decode to decode_Target (Image1 or MappedTexture)
        pImage->Decode(pdecodeTarget, 
            convert ? &Image::CopyScanlineDefault : pFormat->CopyFunc);
        psource = pdecodeTarget;
    }
    else
    {
        ((RawImage*)pImage)->GetImageData(&imageData1);
        psource = &imageData1;
    }

    // *** 2. Rescale - from source to Image2/MappedTexture

    if (rescale)
    {
        if (convert || !pmanager->mapTexture(this))
        {
            pimage2 = *RawImage::Create(pFormat->ConvFormat, sourceMipLevels, pTextures[0].Size, 0);
            if (!pimage2) return false;
            pimage2->GetImageData(&imageData2);
            prescaleTarget = &imageData2;
        }
        else
        {
            prescaleTarget = &pMap->Data;
        }

        if (rescaleType == ResizeNone)
        {
            rescaleType = GetImageFormatRescaleType(format);
            SF_ASSERT(rescaleType != ResizeNone);
        }        
        RescaleImageData(*prescaleTarget, *psource, rescaleType);
        psource = prescaleTarget;
    }

    // *** 3. Convert - from temp source to MappedTexture
    if (convert)
    {
        if (pmanager->mapTexture(this))
            pconvertTarget = &pMap->Data;
        else if (prescaleTarget)
            pconvertTarget = prescaleTarget;
        else
        {
            pimage2 = *RawImage::Create(format, sourceMipLevels, pTextures[0].Size, 0);
            if (!pimage2) return false;
            pimage2->GetImageData(&imageData2);
            pconvertTarget = &imageData2;
        }

        ConvertImageData(*pconvertTarget, *psource, pFormat->CopyFunc);
        psource = pconvertTarget;
    }

    // *** 4. Generate Mip-Levels

    if (swMipGen)
    {        
        unsigned formatPlaneCount = ImageData::GetFormatPlaneCount(format);
        SF_ASSERT(sourceMipLevels == 1);

        // For each level, generate next mip-map from source to target.
        // Source may be either RawImage, Image1/2, or even MappedTexture itself.
        // Target will be Image1/2 if conversion is needed, MappedTexture otherwise.

        for (unsigned iplane=0; iplane < formatPlaneCount; iplane++)
        {
            ImagePlane splane, tplane;
            psource->GetMipLevelPlane(0, iplane, &splane);

            for (unsigned level = 1; level < MipLevels; level++)
            {
                pMap->Data.GetMipLevelPlane(level, iplane, &tplane);

                if (!convert)
                {
                    GenerateMipLevel(tplane, splane, format, iplane);
                    // If we generated directly into MappedTexture,
                    // texture will be used as source for next iteration.
                    splane = tplane;
                }
                else
                {
                    // Extra conversion step means, source has only one level.
                    // We reuse it through GenerateMipLevel, which allows source
                    // and destination to be the same.
                    ImagePlane dplane(splane);
                    dplane.SetNextMipSize();
                    GenerateMipLevel(dplane, splane, format, iplane);
                    ConvertImagePlane(tplane, dplane, format, iplane,
                        pFormat->CopyFunc, psource->GetColorMap());
                    splane.Width  = dplane.Width;
                    splane.Height = dplane.Height;
                }
            }
        }
    }

    if (psource == &pMap->Data)
        pmanager->unmapTexture(this);
    else
    {
        for (unsigned itex = 0; itex < TextureCount; itex++)
            for (unsigned level = 0; level < MipLevels; level++)
            {
                ImagePlane plane;
                psource->GetPlane(level * TextureCount + itex, &plane);
                Upload(itex, level, plane);
            }
    }

    return true;
}


void Texture::ReleaseHWTextures()
{
    TextureManager* pmanager = GetManager();
    bool useKillList = !(pmanager->IsMultiThreaded() || GetCurrentThreadId() == pmanager->RenderThreadId);

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        GLuint TexId = pTextures[itex].TexId;
        if (TexId)
        {
            if (useKillList)
                pmanager->GLTextureKillList.PushBack(TexId);
            else
                glDeleteTextures(1, &TexId);
        }        
        pTextures[itex].TexId = 0;
    }
}

// Obtains lock & stop using the image
void    Texture::ImageLost()
{
    Lock::Locker lock(&pManagerLocks->ImageLock);
    pImage = 0;
}

bool    Texture::Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT((Use & ImageUse_Map_Mask) != 0);
    SF_ASSERT(!pMap);

    if (levelCount == 0)
        levelCount = MipLevels - mipLevel;

    if (!GetManager()->mapTexture(this, mipLevel, levelCount))
    {
        SF_DEBUG_WARNING(1, "Texture::Map failed - couldn't map texture");
        return false;
    }

    pdata->Initialize(GetImageFormat(), levelCount,
        pMap->Data.pPlanes, pMap->Data.RawPlaneCount, true);
    pdata->Use = Use;
    return true;
}

bool    Texture::Unmap()
{
    if (!pMap) return false;
    GetManager()->unmapTexture(this);
    return true;
}


bool    Texture::Update(const UpdateDesc* updates, unsigned count, unsigned mipLevel)
{
    // use map and update entire texture if necessary for performance
    if (pBackingImage)
    {
domap:
        bool inUnmap = pMap != 0;

        if (!GetManager()->mapTexture(this, mipLevel, 1))
        {
            SF_DEBUG_WARNING(1, "Texture::Update failed - couldn't map texture");
            return false;
        }

        ImageFormat format = GetImageFormat(); 
        ImagePlane  dplane;

        for (unsigned i = 0; i < count; i++)
        {
            const UpdateDesc &desc = updates[i];
            ImagePlane        splane(desc.SourcePlane);

            pMap->Data.GetPlane(desc.PlaneIndex, &dplane);
            dplane.pData += desc.DestRect.y1 * dplane.Pitch +
                desc.DestRect.x1 * pFormat->BytesPerPixel;

            splane.SetSize(desc.DestRect.GetSize());
            dplane.SetSize(desc.DestRect.GetSize());
            ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                pFormat->CopyFunc, 0);
        }

        if (!inUnmap)
            GetManager()->unmapTexture(this);
        return true;
    }

    // partial update
    bool convert = pFormat->CopyFunc != 0;

    for (unsigned i = 0; i < count; i++)
    {
        const UpdateDesc &desc = updates[i];
        ImagePlane        splane(desc.SourcePlane);

#if (defined(GL_UNPACK_ROW_LENGTH) && defined(GL_UNPACK_ALIGNMENT))
        if (!convert && splane.Pitch == splane.Width * pFormat->BytesPerPixel)
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pFormat->GLFormat, pFormat->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        }
        else if (!convert && splane.Pitch == ((3 + splane.Width * pFormat->BytesPerPixel) & ~3))
        {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, splane.Width);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pFormat->GLFormat, pFormat->GLData, splane.pData);

            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        }
        else
#endif
        if (!convert && splane.Pitch == desc.DestRect.Width() * pFormat->BytesPerPixel)
        {
            glTexSubImage2D(GL_TEXTURE_2D, mipLevel,
                desc.DestRect.x1, desc.DestRect.y1, desc.DestRect.Width(), desc.DestRect.Height(),
                pFormat->GLFormat, pFormat->GLData, splane.pData);
        }
        else
        {
            SF_DEBUG_WARNING(1, "GL::Texture - irregular size, cannot use update, making texture mappable");

            MakeMappable();
            goto domap;
        }
    }

    return true;
}

void Texture::MakeMappable()
{
    pBackingImage = *RawImage::Create(pFormat->Format, MipLevels, pTextures[0].Size, 0);
}

// ***** MappedTexture

bool MappedTexture::Map(Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(!IsMapped());
    SF_ASSERT((mipLevel + levelCount) <= ptexture->MipLevels);

    // Initialize Data as efficiently as possible.
    if (levelCount <= PlaneReserveSize)
        Data.Initialize(ptexture->GetImageFormat(), levelCount, Planes, levelCount, true);
    else if (!Data.Initialize(ptexture->GetImageFormat(), levelCount, true))
        return false;
    if (!ptexture->pBackingImage)
        return false;

    pTexture      = ptexture;
    StartMipLevel = mipLevel;
    LevelCount    = levelCount;

    unsigned textureCount = ptexture->TextureCount;
    ImageData BackingData;
    ptexture->pBackingImage->GetImageData(&BackingData);

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        ImagePlane              plane;

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        for (unsigned level = 0; level < levelCount; level++)
        {
            BackingData.GetMipLevelPlane(level + StartMipLevel, itex, &plane);
            Data.SetPlane(level * textureCount + itex, plane);
        }
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap()
{
    unsigned textureCount = pTexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        ImagePlane plane;

        for (unsigned level = 0; level < LevelCount; level++)
        {
            Data.GetPlane(level * textureCount + itex, &plane);

            if (plane.pData)
                pTexture->Upload(itex, level + StartMipLevel, plane);
        }
    }

    pTexture->pMap = 0;
    pTexture       = 0;
    StartMipLevel  = 0;
    LevelCount     = 0;    
}


// TempSurfaceSet - represent a set of System-memory surfaces used
// to upload data into Default textures. We keep a minimum number
// of these alive and re-use them.
// ---> If we always use managed textures, we don't have to worry about this.
/*
struct TempSurfaceSet : public List<TempSurfaceSet>
{
D3DFORMAT   Format;
unsigned    PlaneCount;
ImageSize   Size;

// Texture which created us, if any.
// When that texture is destroyed, so will this surface set.
Texture*    pTexture;

struct TexturePair
{
IDirect3DTexture9* pPrimary;
IDirect3DTexture9* pMip;
};

TexturePair     Set[1];
};

*/

// ***** ImageUpdateQueue

ImageUpdateQueue::~ImageUpdateQueue()
{
    for (unsigned i = 0; i<Queue.GetSize(); i++)
    {
        if (Queue[i] & 1)
        {
            Image* pimage = (Image*)(Queue[i] & ~(UpdateRef)1);
            pimage->Release();
        }
        else
        {
            ImageUpdate *pupdate = (ImageUpdate*) Queue[i];
            pupdate->Release();
        }
    }
}

void    ImageUpdateQueue::ProcessUpdates(Render::TextureManager* pmanager)
{
    for (unsigned i = 0; i<Queue.GetSize(); i++)
    {
        if (Queue[i] & 1)
        {
            Image*           pimage   = (Image*)(Queue[i] & ~(UpdateRef)1);
            Render::Texture* ptexture = pimage->GetTexture(pmanager);
            if (ptexture)
                ptexture->Update();
            pimage->Release();
        }
        else
        {
            ImageUpdate *pupdate = (ImageUpdate*) Queue[i];
            pupdate->HandleUpdate(pmanager);
            pupdate->Release();
        }
    }
    Queue.Clear();
}


// ***** TextureManager

TextureManager::TextureManager()
{
    pLocks = *SF_HEAP_AUTO_NEW(this) TextureManagerLocks(this);
}

TextureManager::~TextureManager()
{   
    Mutex::Locker lock(&pLocks->TextureMutex);

    // InitTextureQueue MUST be empty, or there was a thread
    // service problem.
    SF_ASSERT(TextureInitQueue.IsEmpty());
    processTextureKillList();    

    // Notify all textures
    while (!Textures.IsEmpty())
        Textures.GetFirst()->LoseManager();

    pLocks->pManager = 0;
}

// ***** GL Format mapping and conversion functions

void SF_STDCALL GL_CopyScanline8_Extend_A_LA(UByte* pd, const UByte* ps, UPInt size, Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[1] = ps[0];
        pd[0] = 255;
    }
}

// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

enum TexDrawMode
{
    FixFn = 1,
    Shader = 2,
    Both = 3
};

static TextureFormat GLTextureFormatMapping[] = 
{
#ifdef SF_OS_PS3
    { Image_R8G8B8A8, Image_R8G8B8A8, GL_RGBA,            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, true,  Both,  NULL,               &Image::CopyScanlineDefault },
    { Image_B8G8R8A8, Image_R8G8B8A8, GL_RGBA,            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, false, Both,  NULL,               &Image_CopyScanline32_SwapBR },
    { Image_R8G8B8,   Image_R8G8B8A8, GL_RGBA,            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, false, Both,  NULL,               &Image_CopyScanline24_Extend_RGB_RGBA },
    { Image_B8G8R8,   Image_R8G8B8A8, GL_RGBA,            GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV, 4, false, Both,  NULL,               &Image_CopyScanline24_Extend_RGB_RGBA },
#else
    { Image_R8G8B8A8, Image_R8G8B8A8, GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, true,  Both,  NULL,               &Image::CopyScanlineDefault },
    { Image_R8G8B8A8, Image_R8G8B8A8, GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, false, Both,  "EXT_bgra",         &Image_CopyScanline32_SwapBR },
    { Image_B8G8R8A8, Image_R8G8B8A8, GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, true,  Both,  "EXT_bgra",         &Image::CopyScanlineDefault },
    { Image_B8G8R8A8, Image_R8G8B8A8, GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, false, Both,  NULL,               &Image_CopyScanline32_SwapBR },

#if !defined(GL_ES_VERSION_2_0)
    { Image_R8G8B8,   Image_R8G8B8,   GL_RGB,             GL_RGB,             GL_UNSIGNED_BYTE, 3, true,  Both,  NULL,               &Image::CopyScanlineDefault },
#endif
    { Image_R8G8B8,   Image_R8G8B8A8, GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, false, Both,  NULL,               &Image_CopyScanline24_Extend_RGB_RGBA },
    { Image_R8G8B8,   Image_R8G8B8A8, GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, false, Both,  "EXT_bgra",         &Image_CopyScanline24_Extend_RGB_BGRA },

#if !defined(GL_ES_VERSION_2_0)
    { Image_B8G8R8,   Image_R8G8B8,   GL_RGB,             GL_BGR_EXT,         GL_UNSIGNED_BYTE, 3, true,  Both,  "EXT_bgra",         &Image::CopyScanlineDefault },
#endif
    { Image_B8G8R8,   Image_R8G8B8A8, GL_RGBA,            GL_BGRA_EXT,        GL_UNSIGNED_BYTE, 4, false, Both,  "EXT_bgra",         &Image_CopyScanline24_Extend_RGB_RGBA },
    { Image_B8G8R8,   Image_R8G8B8A8, GL_RGBA,            GL_RGBA,            GL_UNSIGNED_BYTE, 4, false, Both,  NULL,               &Image_CopyScanline24_Extend_RGB_BGRA },
#endif

    { Image_A8,       Image_A8,       GL_ALPHA,           GL_ALPHA,           GL_UNSIGNED_BYTE, 1, true,  FixFn, NULL,               &Image::CopyScanlineDefault },
    { Image_A8,       Image_A8,       GL_LUMINANCE,       GL_LUMINANCE,       GL_UNSIGNED_BYTE, 1, true,  Shader,NULL,               &Image::CopyScanlineDefault },
//    { Image_A8,       Image_A8L8,     GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 2, false, Both,  NULL,               &GL_CopyScanline8_Extend_A_LA },

#if !defined(GL_ES_VERSION_2_0)
    { Image_DXT1, Image_DXT1, GL_RGB,  GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_UNSIGNED_BYTE,  0, true, Both, "EXT_s3tc",   &Image::CopyScanlineDefault },
    { Image_DXT3, Image_DXT3, GL_RGBA, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_UNSIGNED_BYTE, 0, true, Both, "EXT_s3tc",   &Image::CopyScanlineDefault },
    { Image_DXT5, Image_DXT5, GL_RGBA, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, GL_UNSIGNED_BYTE, 0, true, Both, "EXT_s3tc",   &Image::CopyScanlineDefault },
#endif

    { Image_None, Image_None,      0,  0, false, 0 }
};


static bool CheckExtension (const char *exts, const char *name)
{
    const char *p = strstr(exts, name);
    return (p && (p[strlen(name)] == 0 || p[strlen(name)] == ' '));
}

const TextureFormat*  TextureManager::getTextureFormat(ImageFormat format) const
{
    for (unsigned i = 0; i< TextureFormats.GetSize(); i++)    
        if (TextureFormats[i]->Format == format)
            return TextureFormats[i];
    return 0;
}

void TextureManager::Initialize(HAL* phal)
{
    RenderThreadId = GetCurrentThreadId();
    pHal = phal;
    Caps = 0;

    const char *glexts = (const char *) glGetString(GL_EXTENSIONS);

#if defined(GL_ES_VERSION_2_0)
    Caps |= TC_NonPower2Limited;
    if (CheckExtension(glexts, "GL_OES_texture_npot"))
        Caps |= TC_NonPower2Full;
    if (CheckExtension(glexts, "GL_OES_required_internalformat") &&
        CheckExtension(glexts, "GL_IMG_texture_format_BGRA8888"))
        Caps |= TC_UseBgra;
#else
    // XXX
    Caps |= TC_NonPower2Full|TC_NonPower2Limited;
#endif

    TextureFormats.Clear();
    TextureFormat* pmapping;

    for (pmapping = GLTextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        // See if format is supported.
#if defined(GL_ES_VERSION_2_0)
        if ((Caps & TC_UseBgra) && pmapping->GLFormat == GL_RGBA)
            continue;
#else
        if (pmapping->Extension == NULL || CheckExtension(glexts, pmapping->Extension))
#endif
        {
            TextureFormats.PushBack(pmapping);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}


MappedTexture* TextureManager::mapTexture(Texture* ptexture, unsigned mipLevel, unsigned levelCount)
{
    MappedTexture* pmap;

    if (!MappedTexture0.IsMapped())
        pmap = &MappedTexture0;
    else
    {
        pmap = SF_HEAP_AUTO_NEW(this) MappedTexture;
        if (!pmap) return 0;
    }

    if (pmap->Map(ptexture, mipLevel, levelCount))
        return pmap;
    if (pmap != &MappedTexture0)
        delete pmap;
    return 0;  
}

void           TextureManager::unmapTexture(Texture *ptexture)
{
    MappedTexture *pmap = ptexture->pMap;
    pmap->Unmap();
    if (pmap != &MappedTexture0)
        delete pmap;
}

void    TextureManager::processTextureKillList()
{
    glDeleteTextures(GLTextureKillList.GetSize(), GLTextureKillList.GetDataPtr());
    GLTextureKillList.Clear();
}

void    TextureManager::processInitTextures()
{
    // TextureMutex lock expected externally.
    //Mutex::Locker lock(&TextureMutex);

    if (!TextureInitQueue.IsEmpty())
    {
        do {
            Texture* ptexture = TextureInitQueue.GetFirst();
            ptexture->RemoveNode();
            ptexture->pPrev = ptexture->pNext = 0;
            if (ptexture->Initialize())
                Textures.PushBack(ptexture);

        } while (!TextureInitQueue.IsEmpty());
        pLocks->TextureInitWC.NotifyAll();
    }
}

void    TextureManager::ProcessQueues()
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    processTextureKillList();
    processInitTextures();
    processImageUpdates();
}


Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
    const ImageSize& size, unsigned use, unsigned arena, ImageBase* pimage)
{
    if (pimage) 
    {
        if (format != pimage->GetFormatNoConv())
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - source image format mismatch");
            return 0;
        }
    }
    else
    {
        if (!(use & (ImageUse_PartialUpdate|ImageUse_PartialUpdate|ImageUse_Map_Mask)))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - no source image for read-only ImageUse");
            return 0;
        }
    }

    const TextureFormat* ptformat = getTextureFormat(format);
    if (!ptformat)
    {
        SF_DEBUG_ERROR1(1, "CreateTexture failed - unsupported format %d", format);
        return 0;
    }
    if (ImageData::IsFormatCompressed(format) &&
        (use & (ImageUse_GenMipmaps|ImageUse_Map_Mask|ImageUse_PartialUpdate)))
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - unsupported Use flags for compressed format");
        return 0;
    }
    if (((use & ImageUse_Map_Mask) != 0) && !ptformat->Mappable)
    {
        SF_DEBUG_ERROR1(1, "CreateTexture failed - format %d not mappable", format);
        return 0;
    }

    Texture* ptexture = 
        SF_HEAP_AUTO_NEW(this) Texture(pLocks, ptformat, mipLevels, size, use, arena, pimage);
    if (!ptexture)
        return false;
    if (!ptexture->IsValid())
    {
        ptexture->Release();
        return false;
    }

    Mutex::Locker lock(&pLocks->TextureMutex);

    if (IsMultiThreaded() || (GetCurrentThreadId() == RenderThreadId))
    {
        // Before initializing texture, process previous requests, if any.
        if (!IsMultiThreaded())
        {
            processTextureKillList();
            processInitTextures();
        }
        if (ptexture->Initialize())
            Textures.PushBack(ptexture);
    }
    else
    {
        TextureInitQueue.PushBack(ptexture);
        while(ptexture->State == Texture::State_InitPending)
            pLocks->TextureInitWC.Wait(&pLocks->TextureMutex);
    }

    // Clear out 'pImage' reference if it's not supposed to be kept. It is safe to do this
    // without ImageLock because texture hasn't been returned yet, so this is the only
    // thread which has access to it.
    if (use & ImageUse_NoDataLoss)
        ptexture->pImage = 0;

    // If texture was properly initialized, it would've been added to list.
    if (ptexture->State == Texture::State_InitFailed)
    {
        ptexture->Release();
        return 0;
    }
    return ptexture;
}

unsigned TextureManager::GetTextureUseCaps(ImageFormat format)
{
    unsigned use = ImageUse_NoDataLoss | ImageUse_Update;
    if (!ImageData::IsFormatCompressed(format))
        use |= ImageUse_PartialUpdate | ImageUse_GenMipmaps;

    const TextureFormat* ptformat = getTextureFormat(format);
    if (!ptformat)
        return 0;
    if (ptformat->Mappable)
        use |= ImageUse_MapInUpdate;
    return use;   
}

bool TextureManager::IsNonPow2Supported(ImageFormat, UInt16 use)
{
    unsigned need = TC_NonPower2Limited;
    if (use & (ImageUse_Wrap|ImageUse_GenMipmaps))
        need |= TC_NonPower2Full;

    return ((Caps & need) == need) != 0;
}


void    TextureManager::UpdateImage(Image* pimage)
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    ImageUpdates.Add(pimage);
}
void    TextureManager::UpdateImage(ImageUpdate* pupdate)
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    ImageUpdates.Add(pupdate);
}


}}};  // namespace Scaleform::Render::GL

