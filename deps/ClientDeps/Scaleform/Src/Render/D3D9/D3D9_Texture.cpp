/**********************************************************************

Filename    :   D3D9_Texture.cpp
Content     :   D3D9 Texture and TextureManager implementation
Created     :   January 2010
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "D3D9_Texture.h"
#include "Render\Render_TextureUtil.h"
#include "Kernel\SF_Debug.h"
//#include "Kernel\SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace D3D9 {


Texture::Texture(TextureManagerLocks* pmanagerLocks, const TextureFormat* pformat,
                 unsigned mipLevels, const ImageSize& size, unsigned use,
                 unsigned arena, ImageBase* pimage)
 : pManagerLocks(pmanagerLocks), pImage(pimage),
   pFormat(pformat), ImgSize(size), MipLevels((UByte)mipLevels), Use((UInt16)use),
   TextureFlags(0), Type(Type_Managed), State(State_InitPending), pMap(0)
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

void Texture::GetUVGenMatrix(Matrix2F* mat) const
{
    // UV Scaling rules are as follows:
    //  - If image got scaled, divide by original size.
    //  - If image is padded, divide by new texture size.
    const ImageSize& sz = (TextureFlags & TF_Rescale) ? ImgSize : Texture0.Size;
    *mat = Matrix2F::Scaling(1.0f / (float)sz.Width, 1.0f / (float)sz.Height);
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


bool IsD3DFormatRescaleCompatible(D3DFORMAT format,
                                  ImageFormat *ptargetImageFormat,
                                  ResizeImageType* presizeType)
{
    switch(format)
    {
        case D3DFMT_A8R8G8B8:
            *ptargetImageFormat = Image_B8G8R8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case D3DFMT_A8B8G8R8:
            *ptargetImageFormat = Image_R8G8B8A8;
            *presizeType        = ResizeRgbaToRgba;
            return true;
        case D3DFMT_A8:
            *ptargetImageFormat = Image_A8;
            *presizeType        = ResizeGray;
            return true;
        default:
            break;
    }
    return false;
}


bool IsD3DFormatMipGenCompatible(D3DFORMAT format)
{
    switch(format)
    {
    case D3DFMT_A8R8G8B8:
    case D3DFMT_A8B8G8R8:
    case D3DFMT_A8:
        return true;
    default:
        break;
    }
    return false;
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
        if (!pmanager->Caps.NonPow2Supported(Use))
        {
            ImageSize roundedSize = ImageSize_RoundUpPow2(tdesc.Size);
            if (roundedSize != tdesc.Size)
            {
                tdesc.Size = roundedSize;
                resize = true;
            }
        }
    }

    if (resize && (Use & ImageUse_Wrap))
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
    DWORD    d3dUsage = 0;
    D3DPOOL  d3dPool  = D3DPOOL_MANAGED;

    if (Use & ImageUse_GenMipmaps)
    {
        SF_ASSERT(MipLevels == 1);
        if (!pFormat->CanAutoGenMipmaps())
        {            
            TextureFlags |= TF_SWMipGen;
            // If using SW MipGen, determine how many mip-levels we should have.
            allocMipLevels = 31;
            for (itex = 0; itex < TextureCount; itex++)
                allocMipLevels = Alg::Min(allocMipLevels,
                                          ImageSize_MipLevelCount(pTextures[itex].Size));
            MipLevels = (UByte)allocMipLevels;
        }
        else
        {
            d3dUsage |= D3DUSAGE_AUTOGENMIPMAP;
            allocMipLevels = 0;
        }
    }

    // MA: Gamebryo-compatibility work-around from GFx 3.x, TBD whether still necessary.
    // For some reason we need to specify levelsNeeded-1, otherwise surface accesses
    // may crash (when running with Gamebryo). So, 256x256 texture has levelCount of 8 (not 9).
    // There is no problem with dropping one level unless user intends to Map it (not typical).
    if ((allocMipLevels > 1) && ((Use & ImageUse_Map_Mask) == 0))
    {
        allocMipLevels--;
        MipLevels--;
    }

    // Only use Dynamic textures for updatable/mappable textures.
    // Also, since Dynamic textures can be lost, don't allow them if ImageUse_InitOnly
    // is not specified.
    bool    allowDynamicTexture = ((Use & ImageUse_InitOnly) != 0) &&
                                  ((Use & (ImageUse_PartialUpdate | ImageUse_Map_Mask)) != 0);
    if (!pmanager->Caps.DynamicTexAllowed())
        allowDynamicTexture = false;

    if (allowDynamicTexture)
    {
        d3dUsage |= D3DUSAGE_DYNAMIC;
        d3dPool  = D3DPOOL_DEFAULT;
        Type = Type_Dynamic;
    }

    // Create textures
    for (itex = 0; itex < TextureCount; itex++)
    {
        HWTextureDesc& tdesc = pTextures[itex];

        if (S_OK != pmanager->pDevice->CreateTexture(
                        tdesc.Size.Width, tdesc.Size.Height, allocMipLevels,
                        d3dUsage, pFormat->GetD3DFormat(), d3dPool, &tdesc.pTexture, 0) )
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - IDirect3DTexture9::CreateTexture failed");
            // Texture creation failed, release all textures and fail.
initialize_texture_fail_after_create:
            ReleaseHWTextures();
            State = State_InitFailed;
            return false;
        }
    }

    // Upload image content to texture, if any.
    if (pImage && !Update())
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - couldn't initialize texture");
        goto initialize_texture_fail_after_create;
    }

    State = State_Valid;
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
    ImageData       *pdecodeTarget = 0, *prescaleTarget = 0;
    ImageData       imageData1, imageData2;
    Ptr<RawImage>   pimage1, pimage2;
    unsigned        sourceMipLevels = GetMipmapCount(); // May be different from MipLevels    

    // Texture update proceeds in four (optional) steps:
    //   1. Image::Decode - Done unless rescaling directly from RawImage.
    //   2. Rescale       - Done if non-pow2 textures are not supported as necessary.
    //   3. Convert       - Needed if conversion wasn't applied in Decode.
    //   4. SW Mipmap Gen - Loop to generate SW mip-maps, may also have a convert step.

    // Although Decode can do scan-line conversion, Convert step is necessary
    // because our Rescale and GenerateMipLevel functions don't support all D3D9
    // destination formats. If non-supported format is encountered, conversion
    // is delayed till after rescale (which, of course, requires an extra image buffer).

    ImageFormat      rescaleBuffFromat = format;
    ImageRescaleType rescaleType = ResizeNone;
    
    if (rescale && !IsD3DFormatRescaleCompatible(pFormat->pMapping->D3DFormat,
                                                 &rescaleBuffFromat, &rescaleType))
        convert = true;
    if (swMipGen && !IsD3DFormatMipGenCompatible(pFormat->pMapping->D3DFormat))
        convert = true;
   
    
    // *** 1. Decode from source pImage to Image1/MappedTexture

    Lock::Locker  imageLock(&pManagerLocks->ImageLock);

    if (!pImage)
        return false;

    // Decode is not needed if RawImage is used directly as a source for rescale.
    if (! ((pImage->GetImageType() == Image::Type_RawImage) && rescale) )
    {
        // Determine decodeTarget -> Image1 if rescale / convert will take place
        if (rescale || convert)
        {
            pimage1 = *RawImage::Create(rescaleBuffFromat, sourceMipLevels, ImgSize, 0);
            if (!pimage1) return false;
            pimage1->GetImageData(&imageData1);
            imageData1.Format = (ImageFormat)(format | ImageFormat_Convertible);
            pdecodeTarget = &imageData1;
        }
        else
        {
            if (!pmanager->mapTexture(this))
                return false;
            pdecodeTarget = &pMap->Data;
        }

        // Decode to decode_Target (Image1 or MappedTexture)
        pImage->Decode(pdecodeTarget, 
            convert ? &Image::CopyScanlineDefault : pFormat->pMapping->CopyFunc);
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
        if (convert)
        {
            pimage2 = *RawImage::Create(format, sourceMipLevels, pTextures[0].Size, 0);
            if (!pimage2) return false;
            pimage2->GetImageData(&imageData2);
            prescaleTarget = &imageData2;
        }
        else
        {
            if (!pmanager->mapTexture(this))
                return false;
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
        if (!pmanager->mapTexture(this))
            return false;
        ConvertImageData(pMap->Data, *psource, pFormat->pMapping->CopyFunc);
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
                                      pFormat->pMapping->CopyFunc, psource->GetColorMap());
                    splane.Width  = dplane.Width;
                    splane.Height = dplane.Height;
                }
            }
        }
    }

    pmanager->unmapTexture(this);
    return true;
}


void Texture::ReleaseHWTextures()
{
    TextureManager* pmanager = GetManager();
    bool useKillList = !pmanager->Caps.IsMultiThreaded() &&
                       (GetCurrentThreadId() != pmanager->RenderThreadId);    

    for (unsigned itex = 0; itex < TextureCount; itex++)
    {
        IDirect3DTexture9* ptexture = pTextures[itex].pTexture;
        if (ptexture)
        {
            if (useKillList)
                pmanager->D3DTextureKillList.PushBack(ptexture);
            else
                ptexture->Release();
        }        
        pTextures[itex].pTexture = 0;
    }
}

// Obtains lock & stop using the image
void    Texture::ImageLost()
{
    Lock::Locker lock(&pManagerLocks->ImageLock);
    pImage = 0;
}


static D3DTEXTUREFILTERTYPE Texture_FilterLookup[Sample_Count] = 
{
    D3DTEXF_POINT,    
    D3DTEXF_LINEAR
};
static D3DTEXTUREADDRESS  Texture_AddressLookup[Wrap_Count] = 
{
    D3DTADDRESS_WRAP,
    D3DTADDRESS_CLAMP
};

// Applies a texture to device starting at pstageIndex, advances index
void    Texture::ApplyTexture(DWORD stageIndex, ImageFillMode fm)
{
    IDirect3DDevice9* pdevice = GetManager()->pDevice;

    D3DTEXTUREFILTERTYPE filter = 
        Texture_FilterLookup[fm.GetSampleMode() >> Sample_Shift];
    D3DTEXTUREADDRESS    address = Texture_AddressLookup[fm.GetWrapMode()];

    for (unsigned i = 0; i < TextureCount; i++, stageIndex++)
    {
        pdevice->SetTexture(stageIndex, pTextures[i].pTexture);
        GetManager()->SetSamplerState(stageIndex, filter, address);
    }
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
                        desc.DestRect.x1 * pFormat->pMapping->BytesPerPixel;

        splane.SetSize(desc.DestRect.GetSize());
        dplane.SetSize(desc.DestRect.GetSize());
        ConvertImagePlane(dplane, splane, format, desc.PlaneIndex,
                          pFormat->pMapping->CopyFunc, 0);
    }

    GetManager()->unmapTexture(this);
    return true;
}

#ifdef SF_AMP_SERVER
bool Texture::Copy(ImageData* pdata)
{
    Image::CopyScanlineFunc puncopyFunc = pFormat->pMapping->UncopyFunc;
    if ( !GetManager() || pFormat->pMapping->Format != pdata->Format || !puncopyFunc)
    {
        // - No texture manager, OR 
        // - Output format is different from the source input format of this texture (unexpected, because
        //   we should be copying back into the image's original source format) OR
        // - We don't know how to uncopy this format.
        return false;
    }

    // Map the texture.
    bool alreadyMapped = (pMap != 0);
    unsigned mipCount = GetMipmapCount();
    if (!alreadyMapped && !GetManager()->mapTexture(this, 0, mipCount))
    {
        SF_DEBUG_WARNING(1, "Texture::Copy failed - couldn't map texture");
        return false;
    }
    SF_ASSERT(pMap);

    // Copy the planes into pdata, using the reverse copy function.
    SF_ASSERT( pdata->GetPlaneCount() == pMap->Data.GetPlaneCount() );
    int ip;
    for ( ip = 0; ip < pdata->RawPlaneCount; ip++ )
    {
        ImagePlane splane, dplane;
        pdata->GetPlane(ip, &dplane);
        pMap->Data.GetPlane(ip, &splane);

        ConvertImagePlane(dplane, splane, GetFormat(), ip, puncopyFunc, 0);
    }

    // Unmap the texture, if we mapped it.
    if ( !alreadyMapped )
        GetManager()->unmapTexture(this);

    return true;
}
#endif

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

    pTexture      = ptexture;
    StartMipLevel = mipLevel;
    LevelCount    = levelCount;

    bool     failedLock   = false;
    unsigned textureCount = ptexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = pTexture->pTextures[itex];
        ImagePlane              plane(tdesc.Size, 0);

        for(unsigned i = 0; i < StartMipLevel; i++)
            plane.SetNextMipSize();

        for (unsigned level = 0; level < levelCount; level++)
        {
            D3DLOCKED_RECT lr;
            // TBD: D3DLOCK_DISCARD may be useful for full-screen DYNAMIC
            // texture updates.

            if (!failedLock &&
                (tdesc.pTexture->LockRect(level + StartMipLevel, &lr, 0, 0) == D3D_OK))
            {   
                plane.Pitch    = lr.Pitch;
                plane.pData    = (UByte*)lr.pBits;
                plane.DataSize = ImageData::GetMipLevelSize(Data.GetFormat(), plane.GetSize(), level); 
                //lr.Pitch * plane.Height;
            }
            else
            {
                plane.Pitch    = 0;
                plane.pData    = 0;
                plane.DataSize = 0;
                failedLock = true;
            }

            Data.SetPlane(level * textureCount + itex, plane);
            // Prepare for next level.
            plane.SetNextMipSize();
        }
    }

    if (failedLock)
    {
        SF_DEBUG_ERROR(1, "D3D9::MappedTexture::Map failed - LockRect failure");
        Unmap();
        return false;
    }

    pTexture->pMap = this;
    return true;
}

void MappedTexture::Unmap()
{
    unsigned textureCount = pTexture->TextureCount;

    for (unsigned itex = 0; itex < textureCount; itex++)
    {
        Texture::HWTextureDesc &tdesc = pTexture->pTextures[itex];
        ImagePlane plane;

        for (unsigned level = 0; level < LevelCount; level++)
        {
            Data.GetPlane(level * textureCount + itex, &plane);
            if (plane.pData)
            {
                tdesc.pTexture->UnlockRect(level + StartMipLevel);
                plane.pData = 0;
            }
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

TextureManager::TextureManager(IDirect3DDevice9* pdevice,
                               D3DCapFlags capFlags)
 : pDevice(pdevice), Caps(capFlags)
{
    RenderThreadId = Caps.IsMultiThreaded() ? 0 : GetCurrentThreadId();
    initTextureFormats();
    pLocks = *SF_HEAP_AUTO_NEW(this) TextureManagerLocks(this);

    for ( int i = 0; i < MaximumStages; i++ )
    {
        AddressMode[i] = D3DTADDRESS_FORCE_DWORD;
        FilterType[i] = D3DTEXF_FORCE_DWORD;
    }
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

// ***** D3D9 Format mapping and conversion functions

void SF_STDCALL D3D9_CopyScanline8_Extend_A8_A8L8(UByte* pd, const UByte* ps, UPInt size,
                                                  Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0];
    }
}

void SF_STDCALL D3D9_CopyScanline8_Extend_A8_A4R4G4B4(UByte* pd, const UByte* ps, UPInt size,
                                                      Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd+=2, ps++)
    {        
        pd[0] = 255;
        pd[1] = ps[0] | 0x0f; // Copy high 4 bits; low bits set to F.
    }
}

void SF_STDCALL D3D9_CopyScanline16_Retract_A8L8_A8(UByte* pd, const UByte* ps, UPInt size,
                                                    Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {        
        pd[0] = ps[1];
    }
}

void SF_STDCALL D3D9_CopyScanline16_Retract_A4R4G4B4_A8(UByte* pd, const UByte* ps, UPInt size,
                                        Palette*, void*)
{
    for (UPInt i = 0; i< size; i++, pd++, ps+=2)
    {        
        pd[0] = ps[1] & ~0x0f; // Copy high 4 bits; zero low bits.
    }
}


// Image to Texture format conversion and mapping table,
// organized by the order of preferred image conversions.

static TextureFormat::Mapping D3D9TextureFormatMapping[] = 
{
    // Warning: Different versions of the same ImageFormat must go right after each-other,
    // as initTextureFormats relies on that fact to skip them during detection.
    { Image_R8G8B8A8,   D3DFMT_A8B8G8R8, 4, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_R8G8B8A8,   D3DFMT_A8R8G8B8, 4, false, &Image_CopyScanline32_SwapBR,            &Image_CopyScanline32_SwapBR },

    { Image_B8G8R8A8,   D3DFMT_A8R8G8B8, 4, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_B8G8R8A8,   D3DFMT_A8B8G8R8, 4, false, &Image_CopyScanline32_SwapBR,            &Image_CopyScanline32_SwapBR },

    { Image_R8G8B8,     D3DFMT_A8R8G8B8, 4, false, &Image_CopyScanline24_Extend_RGB_BGRA,   &Image_CopyScanline32_Retract_BGRA_RGB },
    { Image_B8G8R8,     D3DFMT_A8R8G8B8, 4, false, &Image_CopyScanline24_Extend_RGB_RGBA,   &Image_CopyScanline32_Retract_RGBA_RGB },

    { Image_A8,         D3DFMT_A8,       1, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_A8,         D3DFMT_A8L8,     2, false, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_A8,         D3DFMT_A4R4G4B4, 2, false, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_DXT1,       D3DFMT_DXT1,     0, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT3,       D3DFMT_DXT3,     0, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},
    { Image_DXT5,       D3DFMT_DXT5,     0, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault},

    { Image_Y8_U2_V2,   D3DFMT_A8,       1, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2,   D3DFMT_A8L8,     2, false, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_Y8_U2_V2,   D3DFMT_A4R4G4B4, 2, false, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_Y8_U2_V2_A8,D3DFMT_A8,       1, true,  &Image::CopyScanlineDefault,             &Image::CopyScanlineDefault },
    { Image_Y8_U2_V2_A8,D3DFMT_A8L8,     1, false, &D3D9_CopyScanline8_Extend_A8_A8L8,      &D3D9_CopyScanline16_Retract_A8L8_A8 },
    { Image_Y8_U2_V2_A8,D3DFMT_A4R4G4B4, 2, false, &D3D9_CopyScanline8_Extend_A8_A4R4G4B4,  &D3D9_CopyScanline16_Retract_A4R4G4B4_A8 },

    { Image_None,       D3DFMT_UNKNOWN,  0, false, 0, 0 }
};


void        TextureManager::initTextureFormats()
{
    Ptr<IDirect3D9>               pd3d9;
    Ptr<IDirect3DSurface9>        pbackSurface;
    D3DCAPS9                      caps;
    D3DDEVICE_CREATION_PARAMETERS cp;
    D3DSURFACE_DESC               surfaceDesc;
    
    pDevice->GetDirect3D(&pd3d9.GetRawRef());    
    pDevice->GetCreationParameters(&cp);
    pDevice->GetDeviceCaps(&caps);
    pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pbackSurface.GetRawRef());
    pbackSurface->GetDesc(&surfaceDesc);
    
    TextureFormat::Mapping* pmapping;
  
    for (pmapping = D3D9TextureFormatMapping; pmapping->Format != Image_None; pmapping++)
    {
        // See if format is supported.
        if (D3D_OK ==
            pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                     surfaceDesc.Format, 0,
                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat))
        {
            TextureFormat tf = { pmapping, 0 };
            // And now check its capabilities to assign extra Usage.
            if ((caps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) &&
                (D3D_OK ==  pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                                     surfaceDesc.Format, D3DUSAGE_AUTOGENMIPMAP,
                                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat)))
            {
                tf.D3DUsage |= D3DUSAGE_AUTOGENMIPMAP;
            }

            if ((caps.Caps2 & D3DCAPS2_DYNAMICTEXTURES) &&
                (D3D_OK ==  pd3d9->CheckDeviceFormat(cp.AdapterOrdinal, cp.DeviceType,
                                                     surfaceDesc.Format, D3DUSAGE_DYNAMIC,
                                                     D3DRTYPE_TEXTURE, pmapping->D3DFormat)))
            {
                tf.D3DUsage |= D3DUSAGE_DYNAMIC;
            }

            TextureFormats.PushBack(tf);

            // If format added, skip additional mappings for it.
            while ((pmapping+1)->Format == pmapping->Format)
                pmapping++;
        }
    }
}

const TextureFormat*  TextureManager::getTextureFormat(ImageFormat format) const
{
    for (unsigned i = 0; i< TextureFormats.GetSize(); i++)    
        if (TextureFormats[i].GetImageFormat() == format)
            return &TextureFormats[i];
    return 0;
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
    for (unsigned i = 0; i<D3DTextureKillList.GetSize(); i++)
        D3DTextureKillList[i]->Release();
    D3DTextureKillList.Clear();
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

void    TextureManager::ServiceQueues()
{
    Mutex::Locker lock(&pLocks->TextureMutex);
    processTextureKillList();
    processInitTextures();
    processImageUpdates();
}

void TextureManager::SetSamplerState( unsigned stage, D3DTEXTUREFILTERTYPE filter, D3DTEXTUREADDRESS address )
{
    if ( AddressMode[stage] != address )
    {
        pDevice->SetSamplerState(stage, D3DSAMP_ADDRESSU, address);
        pDevice->SetSamplerState(stage, D3DSAMP_ADDRESSV, address);
        AddressMode[stage] = address;
    }
    if ( FilterType[stage] != filter )
    {
        pDevice->SetSamplerState(stage, D3DSAMP_MINFILTER, filter );
        pDevice->SetSamplerState(stage, D3DSAMP_MAGFILTER, filter );
        pDevice->SetSamplerState(stage, D3DSAMP_MIPFILTER, filter );
        FilterType[stage] = filter;
    }
}

Render::Texture* TextureManager::CreateTexture(ImageFormat format, unsigned mipLevels,
                                               const ImageSize& size,
                                               unsigned use, unsigned arena, ImageBase* pimage)
{
    if (pimage) 
    {
        if (format != pimage->GetFormatNoConv())
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - source image format mismatch");
            return 0;
        }
        if ((pimage->GetImageType() == Image::Type_ImageBase) && !(use & ImageUse_InitOnly))
        {
            SF_DEBUG_ERROR(1, "CreateTexture failed - ImageUse_InitOnly expected when using ImageBase class");
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
        SF_DEBUG_ERROR(1, "CreateTexture failed - unsupported format");
        return 0;
    }
    if (ImageData::IsFormatCompressed(format) &&
        (use & (ImageUse_GenMipmaps|ImageUse_Map_Mask|ImageUse_PartialUpdate)))
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - unsupported Use flags for compressed format");
        return 0;
    }
    if (((use & ImageUse_Map_Mask) != 0) && !ptformat->pMapping->Mappable)
    {
        SF_DEBUG_ERROR(1, "CreateTexture failed - format not mappable");
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

    if (Caps.IsMultiThreaded() || (GetCurrentThreadId() == RenderThreadId))
    {
        // Before initializing texture, process previous requests, if any.
        if (!Caps.IsMultiThreaded())
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
    if (use & ImageUse_InitOnly)
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
    // ImageUse_InitOnly (ImageUse_NoDataLoss alias) ok while textures are Managed
    unsigned use = ImageUse_InitOnly | ImageUse_Update;
    if (!ImageData::IsFormatCompressed(format))
        use |= ImageUse_PartialUpdate | ImageUse_GenMipmaps;

    const TextureFormat* ptformat = getTextureFormat(format);
    if (!ptformat)
        return 0;
    if (ptformat->pMapping->Mappable)
        use |= ImageUse_MapInUpdate;
    return use;   
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


}}};  // namespace Scaleform::Render::D3D9

