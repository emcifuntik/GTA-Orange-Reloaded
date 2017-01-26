/**********************************************************************

Filename    :   Render_Image.cpp
Content     :   Image implementation
Created     :   December 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render_Image.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {


// **** ImageSize Helpers

ImageSize SF_STDCALL ImageSize_RoundUpPow2(ImageSize sz)
{
    ImageSize size(1);
    while (size.Width  < sz.Width)  { size.Width <<= 1; }
    while (size.Height < sz.Height) { size.Height <<= 1; }
    return size;
}

ImageSize SF_STDCALL ImageSize_NextMipSize(ImageSize sz)
{    
    sz.Width  = Alg::Max<UInt32>(1, sz.Width/2);
    sz.Height = Alg::Max<UInt32>(1, sz.Height/2);
    return sz;
}

unsigned SF_STDCALL ImageSize_MipLevelCount(ImageSize sz)
{
    unsigned levels = 1;
    while((sz.Width > 1) && (sz.Height > 1))
    {
        sz = ImageSize_NextMipSize(sz);
        levels++;
    }
    return levels;
}


// ***** ImagePlane

    
void    ImagePlane::GetMipLevel(ImageFormat format, unsigned level,
                                ImagePlane* p, unsigned plane) const
{
    *p = *this;
    UPInt levelSize = 0, totalLevelSize = 0;

    for(unsigned lv = level; lv != 0; lv--)
    {
        UPInt levelSize = ImageData::GetMipLevelSize(format, p->GetSize(), plane);        
        p->pData += levelSize;
        p->SetNextMipSize();
        p->Pitch  = ImageData::GetFormatPitch(format, p->Width, plane);

        totalLevelSize += levelSize;
        SF_ASSERT(totalLevelSize <= DataSize);
    }
    p->DataSize = DataSize - totalLevelSize + levelSize;
}

// ***** Palette

Palette* SF_STDCALL Palette::Create(unsigned colorCount,
                                                bool hasAlpha, MemoryHeap* pheap)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();
    SF_ASSERT(colorCount < 0x10000);
    Palette* pmap = (Palette*)
        SF_HEAP_ALLOC(pheap, sizeof(Palette) + (colorCount-1)*sizeof(Color),
                      Stat_Default_Mem);
    if (pmap)
    {
        pmap->RefCount    = 1;
        pmap->ColorCount  = (UInt16)colorCount;
        pmap->HasAlphaFlag= hasAlpha;
        memset(pmap->Colors, 0, sizeof(Color)*colorCount);
    }
    return pmap;
}


//--------------------------------------------------------------------
// ***** ImageData

bool    ImageData::allocPlanes(ImageFormat format, unsigned mipLevelCount,
                               bool separateMipmaps)
{
    SF_ASSERT((pPlanes == &Plane0) && (!(Flags & Flag_AllocPlanes)));
    SF_ASSERT(mipLevelCount < 256); // LevelCount is UByte.

    // GetFormat plane count
    unsigned fmtPlanes = GetFormatPlaneCount(format);
    unsigned planeCount= fmtPlanes * (separateMipmaps ? mipLevelCount : 1);
    
    if (planeCount > 1)
    {
        MemoryHeap* pheap = (Flags & Flag_LocalHeap) ?
                            Memory::GetHeapByAddress(this) : Memory::GetGlobalHeap();
        pPlanes = (ImagePlane*)
            SF_HEAP_ALLOC(pheap, sizeof(ImagePlane)* planeCount, Stat_Default_Mem);
        if (!pPlanes)
        {
            RawPlaneCount = 1;
            pPlanes    = &Plane0;
            return false;
        }
        memset(pPlanes, 0, sizeof(ImagePlane)* planeCount);
        Flags |= Flag_AllocPlanes;
    }

    Format        = format;
    RawPlaneCount = (UInt16)planeCount;
    LevelCount    = (UByte)mipLevelCount;
    if (separateMipmaps)
        Flags |= Flag_SeparateMipmaps;
    return true;
}

void    ImageData::freePlanes()
{
    if (Flags & Flag_AllocPlanes)
    {
        SF_ASSERT(pPlanes && (pPlanes != &Plane0));
        Flags &= ~Flag_AllocPlanes;
        SF_FREE(pPlanes);        
    }
    pPlanes = &Plane0;
}

void    ImageData::Clear()
{
    freePlanes(); // Sets pPlanes.
    Format      = Image_None;
    Use         = 0;
    Flags       = (UByte)(Flags & (~Flag_LocalHeap));
    LevelCount  = 0;
    RawPlaneCount = 1;
    pPalette.Clear();
    Plane0.SetData(0, 0, 0);
}

// Configure planes & mipmaps
unsigned ImageData::GetPlaneCount() const
{
    return RawPlaneCount * ((Flags & Flag_SeparateMipmaps) ? LevelCount : 1);
}

void    ImageData::GetPlane(unsigned index, ImagePlane* pplane) const
{
    if (index < RawPlaneCount)
    {
        *pplane = GetPlaneRef(index);
        return;
    }

    // We get here only when indexing "virtual" non-separated mip-map planes;
    // i.e. the default case where all mipmaps represented by one raw plane.
    SF_ASSERT(!(Flags & Flag_SeparateMipmaps));
    
    unsigned level    = index / RawPlaneCount;
    unsigned rawPlane = index % RawPlaneCount;
    const ImagePlane& plane = GetPlaneRef(rawPlane);
    plane.GetMipLevel(Format, level, pplane, rawPlane);
}


// Set pixel, sets only the appropriate channels
void    ImageData::SetPixelARGB(unsigned x, unsigned y, UInt32 color)
{
    // Bounds check
    if ((x >= GetWidth()) || (y >= GetHeight()))
        return;
    if (Format >= Image_DXT1)
    {
        SF_ASSERT(0);
        return;
    }

    UByte *pline = GetScanline(y);

    switch(Format)
    {
    case Image_B8G8R8A8:
        *(pline + x * 4 + 0)    = (UByte) color & 0xFF;        // B
        *(pline + x * 4 + 1)    = (UByte) (color>>8) & 0xFF;   // G
        *(pline + x * 4 + 2)    = (UByte) (color>>16) & 0xFF;  // R
        *(pline + x * 4 + 3)    = (UByte) (color>>24);         // A
        break;
    case Image_R8G8B8A8:
        *(pline + x * 4 + 0)    = (UByte) (color>>16) & 0xFF;  // R
        *(pline + x * 4 + 1)    = (UByte) (color>>8) & 0xFF;   // G
        *(pline + x * 4 + 2)    = (UByte) color & 0xFF;        // B
        *(pline + x * 4 + 3)    = (UByte) (color>>24);         // A
        // Old:
        //*(((UInt32*)pline) + x) = Alg::ByteUtil::LEToSystem(color);
        break;
        
    case Image_R8G8B8:
        // Data order is packed 24-bit, RGBRGB...,
        // regardless of the endian-ness of the CPU.
        *(pline + x * 3 + 0)    = (UByte) (color>>16) & 0xFF; // R
        *(pline + x * 3 + 1)    = (UByte) (color>>8) & 0xFF;  // G
        *(pline + x * 3 + 2)    = (UByte) color & 0xFF;       // B
        break;
    case Image_B8G8R8:
        // Data order is packed 24-bit, RGBRGB...,
        // regardless of the endian-ness of the CPU.
        *(pline + x * 3 + 0)    = (UByte) color & 0xFF;       // B
        *(pline + x * 3 + 1)    = (UByte) (color>>8) & 0xFF;  // G
        *(pline + x * 3 + 2)    = (UByte) (color>>16) & 0xFF; // R
        break;
    case Image_A8:
        *(pline + x) = (UByte) (color >> 24);
        break;
    default:
        break;
    }
}

void    ImageData::SetPixelAlpha(unsigned x, unsigned y, UByte alpha)
{
    if ((x >= GetWidth()) || (y >= GetHeight()))
        return;
    if (Format >= Image_DXT1)
    {
        SF_ASSERT(0);
        return;
    }

    UByte *pline = GetScanline(y);

    switch(Format)
    {
    case Image_R8G8B8A8:
        // Target is always little-endian
        *(pline + x * 4 + 3)   = alpha;
        break;      
    case Image_A8:
        *(pline + x) = alpha;
        break;      
    default:
        break;
    }
}



// Computes a hash of the given data buffer.
// Hash function suggested by http://www.cs.yorku.ca/~oz/hash.html
// Due to Dan Bernstein.  Allegedly very good on strings.
//
// One problem with this hash function is that e.g. if you take a
// bunch of 32-bit ints and hash them, their hash values will be
// concentrated toward zero, instead of randomly distributed in
// [0,2^32-1], because of shifting up only 5 bits per byte.
SF_INLINE UPInt GImageBase_BernsteinHash(const void* pdataIn, UPInt size, UPInt seed = 5381)
{
    const UByte*    pdata = (const UByte*) pdataIn;
    UPInt           h    = seed;

    while (size > 0)
    {
        size--;
        h = ((h << 5) + h) ^ (unsigned) pdata[size];
    }

    return h;
}


// Compute a hash code based on image contents.  Can be useful
// for comparing images. Will return 0 if pData is null.
UPInt   ImageData::ComputeHash() const
{
    UPInt h = GImageBase_BernsteinHash(&RawPlaneCount, sizeof(RawPlaneCount));

    for (unsigned i = 0; i < RawPlaneCount; i++)
    { 
        ImagePlane& plane = pPlanes[i];
        h = GImageBase_BernsteinHash(&plane.Width, sizeof(plane.Width), h);
        h = GImageBase_BernsteinHash(&plane.Height, sizeof(plane.Height), h);
        h = GImageBase_BernsteinHash(&plane.Pitch, sizeof(plane.Pitch), h);
        h = GImageBase_BernsteinHash(&plane.DataSize, sizeof(plane.DataSize), h);
        
        if (!plane.pData)
            return 0;
        h = GImageBase_BernsteinHash(&plane.pData, plane.DataSize, h);
    }
    return h;
}

// Ensures that all aspects of the image match up, including layout.
bool    ImageData::operator == (const ImageData& other) const
{
    if ((Format != other.Format) ||
        (Use != other.Use) ||
      //  (Flags != other.Flags) ||   // It seems that comparing flags is not useful here. TBD.
        (LevelCount != other.LevelCount) ||
        (RawPlaneCount != other.RawPlaneCount))
        return false;

    unsigned plane;

    if (pPalette)
    {
        if (!other.pPalette || (*pPalette != *other.pPalette))
            return false;
    }
    else
    {
        if (other.pPalette)
        return false;
    }

    for (plane = 0; plane < RawPlaneCount; plane++)
    {   
        if (!pPlanes[plane].MatchSizes(other.pPlanes[plane]))
            return false;
    }
    
    // Compare data bits last, only if all other data matched.
    for (plane = 0; plane < RawPlaneCount; plane++)
    {   
        if (memcmp(pPlanes[plane].pData, other.pPlanes[plane].pData, 
                   pPlanes[plane].DataSize))
           return false;
    }
    return true;
}


// *** Image Static functions

unsigned ImageData::GetFormatPlaneCount(ImageFormat fmt)
{
    switch(fmt)
    {
    case Image_None:
        return 0;
    case Image_Y8_U2_V2:
        return 3;
    case Image_Y8_U2_V2_A8:
        return 4;
    }
    return 1;
}

unsigned  ImageData::GetFormatBytesPerPixel(ImageFormat fmt, unsigned plane)
{
    SF_ASSERT(plane < GetFormatPlaneCount(fmt));
    SF_UNUSED(plane);

    switch(fmt)
    {
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        return 4;
    case Image_R8G8B8:
    case Image_B8G8R8:
        return 3;
    case Image_DXT1:
        return 2;
    case Image_A8:
    case Image_P8:
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
    case Image_DXT3:
    case Image_DXT5:
        return 1;

    case Image_None:
    default:
        break;
    }
    return 0;
}

UPInt  ImageData::GetFormatPitch(ImageFormat fmt, UInt32 width, unsigned plane)
{
    switch((fmt) & ~ImageFormat_Convertible)
    {
    case Image_A8:    
    case Image_P8:
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
        SF_UNUSED(plane);
        return width;
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
        return width * 4;
    case Image_R8G8B8:
    case Image_B8G8R8:
        return (width * 3 + 3) & ~3;
    default:
        break;
    }
    return 0;
}

UPInt  ImageData::GetFormatScanlineCount(ImageFormat fmt, UInt32 height, unsigned plane)
{
    SF_UNUSED(plane);
    switch((fmt) & ~ImageFormat_Convertible)
    {
    case Image_DXT1:
    case Image_DXT3:
    case Image_DXT5:
        return (height + 3) / 4;
    default:
        break;    
    }
    return height;
}

ImageSize  ImageData::GetFormatPlaneSize(ImageFormat fmt, const ImageSize& sz, unsigned plane)
{
    switch((fmt) & ~ImageFormat_Convertible)
    {
    case Image_Y8_U2_V2:
    case Image_Y8_U2_V2_A8:
        if ((plane == 1) || (plane == 2))
            return ImageSize(sz.Width / 2, sz.Height / 2);
        break;
    default:
        break;
    }
    return sz;
}


UPInt ImageData::GetMipLevelSize(ImageFormat format, const ImageSize& sz, unsigned plane)
{
    UPInt levelSize;
    if (format == Image_DXT1)
        levelSize = Alg::Max<UInt32>(1u, (sz.Width+3) / 4) *
                    Alg::Max<UInt32>(1u, (sz.Height+3) / 4) * 8;
    else if (format >= Image_DXT3 && format <= Image_DXT5)
        levelSize = Alg::Max<UInt32>(1u, (sz.Width+3) / 4) *
                    Alg::Max<UInt32>(1u, (sz.Height+3) / 4) * 16;
    else
        levelSize = GetFormatPitch(format, sz.Width, plane) * sz.Height;
    return levelSize;
}

UPInt  ImageData::GetMipLevelsSize(ImageFormat format, const ImageSize& sz,
                                   unsigned levels, unsigned plane)
{
    UPInt     levelsSize = 0;
    ImageSize size(sz);

    for(unsigned lv = levels; lv != 0; lv--)
    {
        levelsSize += ImageData::GetMipLevelSize(format, size, plane);
        size = ImageSize_NextMipSize(size);        
    }
    return levelsSize;
}


//--------------------------------------------------------------------

void    ImageBase::CopyScanlineDefault(UByte* pd, const UByte* ps, UPInt size,
                                       Palette*, void*)
{
    memcpy(pd, ps, size);
}


#ifdef SF_AMP_SERVER
UInt32 ImageBase::GetNextImageId()
{
    static Lock staticLock;
    static UInt32 nextImageId = 0;

    Lock::Locker locker(&staticLock);
    return (++nextImageId);
}
#endif  // GFX_AMP_SERVER


//--------------------------------------------------------------------
// ***** Image

Image::~Image()
{
    releaseTexture();
    if (pInverseMatrix)
        SF_FREE(pInverseMatrix);
}

void Image::releaseTexture()
{
    Texture* ptexture = pTexture.Exchange_NoSync(0);
    if (ptexture)
    {
     //   if (pTexture->GetTextureManager() == pUpdateSync)
     //       pUpdateSync = 0;  // NTS?? What if user is calling Update() now?
        ptexture->ImageLost();
        ptexture->Release();
    }
}

void Image::initTexture_NoAddRef(Texture* ptexture)
{
    SF_ASSERT(pTexture == 0);
    pTexture = ptexture;    
}

Image* Image::Create(ImageFormat format, unsigned mipLevels, const ImageSize& size,
                     unsigned use, MemoryHeap* pheap, unsigned textureArena,
                     TextureManager* pmanager, ImageUpdateSync* pupdateSync)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();
    if (!pupdateSync)
        pupdateSync = pmanager;

    Image* pimage = 0;

    if (use & ImageUse_GenMipmaps)
    {
        if (mipLevels != 1)
        {
            SF_DEBUG_ERROR(1,
                "Image::Create failed - ImageUse_GenMipmaps requires mipLevels=1");
            return 0;
        }
    }

    if (pmanager)
    {
        unsigned useCaps = pmanager->GetTextureUseCaps(format);

        // If texture data can't be lost (ImageUse_InitOnly) and is
        // agreeable to our Mapping settings, create a TextureImage directly.
        if ((useCaps & ImageUse_InitOnly) &&
            ((use & ImageUse_Map_Mask) == (useCaps & ImageUse_Map_Mask)))
        {
            pimage = SF_HEAP_NEW(pheap) TextureImage(format, size, use, 0, pupdateSync);
        }
    }

    if (!pimage)
    {
        pimage = RawImage::Create(format, mipLevels, size, use,
                                  pheap, textureArena, pupdateSync);
        if (!pimage)
            return 0;
    }

    if (pmanager)
    {
        Texture* ptexture = pmanager->CreateTexture(format, mipLevels, size, use,
                                                    textureArena, pimage);
        if (!ptexture)
        {
            pimage->Release();
            return 0;
        }

        pimage->initTexture_NoAddRef(ptexture);
    }

    return pimage;
}


Image* Image::CreateSubImage(const ImageRect& rect, MemoryHeap* pheap)
{
    return SF_HEAP_NEW(pheap) SubImage(this, rect);
}


// Matrices
void Image::SetMatrix(const Matrix2F& mat, MemoryHeap* heap)
{
    if (!pInverseMatrix)
    {
        pInverseMatrix = (Matrix2F*)SF_HEAP_MEMALIGN(
            heap ? heap : Memory::GetHeapByAddress(this),
            sizeof(Matrix2F), 16, Stat_Image_Mem);
    }
    *pInverseMatrix = mat;
    pInverseMatrix->Invert();
}
void Image::SetMatrixInverse(const Matrix2F& mat, MemoryHeap* heap)
{
    if (!pInverseMatrix)
    {
        pInverseMatrix = (Matrix2F*)SF_HEAP_MEMALIGN(
            heap ? heap : Memory::GetHeapByAddress(this),
            sizeof(Matrix2F), 16, Stat_Image_Mem);
    }
    *pInverseMatrix = mat;
}

void Image::GetMatrix(Matrix2F* mat) const
{
    if (pInverseMatrix)
    {
        *mat = *pInverseMatrix;
        mat->Invert();
    }
    else
    {
        mat->SetIdentity();
    }
}
void Image::GetMatrixInverse(Matrix2F* mat) const
{
    if (pInverseMatrix)
    {
        *mat = *pInverseMatrix;
    }
    else
    {
        mat->SetIdentity();
    }
}

void Image::GetUVGenMatrix(Matrix2F* mat, TextureManager* manager)
{
    Texture* texture = GetTexture(manager);
    if (texture)
    {
        if (pInverseMatrix)
        {
            Matrix2F uvGenMatrix(Matrix2F::NoInit);
            texture->GetUVGenMatrix(&uvGenMatrix);
            mat->SetToAppend(*pInverseMatrix, uvGenMatrix);
        }
        else
        {
            texture->GetUVGenMatrix(mat);
        }

        ImageRect r = GetRect();
        mat->PrependTranslation((float)r.x1, (float)r.y1);
    }
    else
    {
        mat->SetIdentity();
    }
}

void Image::GetUVNormMatrix(Matrix2F* mat, TextureManager* manager)
{
    GetUVGenMatrix(mat, manager);
    ImageSize s = GetSize();
    mat->PrependScaling((float)s.Width, (float)s.Height);
}



//--------------------------------------------------------------------
// ***** ImageSource

// Utility functions for correct initialization of image.
bool ImageSource::IsDecodeOnlyImageCompatible(const ImageCreateArgs& args)
{
    // We don't need to keep Image in memory if:
    //  1. Texture manager supports NoDataLoss initialization and matches
    //     our map flags, since we'll creat an init-only texture.
    //  2. If map flags requested, as we'll create RawImage anyway.
    
    if (args.pManager)
    {
        unsigned textureCaps = args.pManager->GetTextureUseCaps(GetFormat());
        if ((args.Use & ImageUse_Map_Mask) == (args.Use & textureCaps & ImageUse_Map_Mask))
        {
            if (textureCaps & ImageUse_NoDataLoss)
                return false;
        }
    }
    if (args.Use & ImageUse_Map_Mask)
        return false;

    return true;
}

Image* ImageSource::CreateCompatibleImage(const ImageCreateArgs& args)
{
    ImageFormat       format = GetFormat();
    ImageUpdateSync*  updateSync = args.GetUpdateSync();
    MemoryHeap*       heap = args.pHeap ? args.pHeap : Memory::GetGlobalHeap();

    // If there is a TextureManager, try to either create an TextureImage directly,
    // or associate it with the image.    
    unsigned imageUse = args.Use;
    unsigned mipmapCount = GetMipmapCount();
    if (mipmapCount > 1)
        imageUse &= (~ImageUse_GenMipmaps);


    if (args.pManager)
    {
        unsigned textureCaps = args.pManager->GetTextureUseCaps(format);

        // If texture data can't be lost (ImageUse_NoDataLosts) and is
        // agreeable to our Mapping settings, create a TextureImage directly.
        if ((args.Use & ImageUse_Map_Mask) == (args.Use & textureCaps & ImageUse_Map_Mask))
        {
            if (textureCaps & ImageUse_NoDataLoss)
            {
                // If texture can't be lost 
                unsigned textureUse = imageUse|ImageUse_InitOnly;
                Ptr<Texture> ptexture = 
                    *args.pManager->CreateTexture(format, mipmapCount, GetSize(),
                                                  textureUse,
                                                  args.TextureArena, this);
                if (ptexture)
                {
                    return SF_HEAP_NEW(heap)
                        TextureImage(format, GetSize(), textureUse, ptexture, updateSync);
                }
                return 0;
            }
        }
    }

    // We are either Mappable, or temporary was passed -
    // Create Raw image, decoding into it.
    RawImage* rawImage = RawImage::Create(format, mipmapCount, GetSize(), imageUse,
                                          heap, args.TextureArena, updateSync);
    if (!rawImage) return 0;

    // Read / copy TGA data into RawImage buffer.
    ImageData rawData;
    rawImage->GetImageData(&rawData);
    if (!Decode(&rawData, &Image::CopyScanlineDefault))
    {
        rawImage->Release();
        return 0;
    }
    return rawImage;
}




//--------------------------------------------------------------------
// ***** Raw Image

RawImage::~RawImage()
{
    // Destructor still needs to handle partially-initialized
    // image since it may exist in Create.

    for (unsigned plane = 0; plane < Data.RawPlaneCount; plane++)
    {
        if (Data.pPlanes[plane].pData)
        {
            SF_FREE(Data.pPlanes[plane].pData);
            Data.pPlanes[plane].pData = 0;
        }
    }
}


RawImage* RawImage::Create(ImageFormat format, unsigned mipLevelCount,
                           const ImageSize& size, unsigned use,
                           MemoryHeap* pheap, unsigned textureArena,
                           ImageUpdateSync* pupdateSync)
{
    if (!pheap)
        pheap = Memory::GetGlobalHeap();

    if (use & ImageUse_GenMipmaps)
    {
        if (mipLevelCount != 1)
        {
            SF_DEBUG_ERROR(1,
                "RawImage::Create failed - ImageUse_GenMipmaps requires mipLevels=1");
            return 0;
        }
    }

    RawImage* pimage = SF_HEAP_NEW(pheap) RawImage;
    if (!pimage)
    {
rawimage_create_memory_error:
        SF_DEBUG_ERROR(1, "RawImage::Create failed - Allocation failure");
        return 0;
    }

    if (!pimage->Data.Initialize(format, mipLevelCount))
    {
        pimage->Release();
        goto rawimage_create_memory_error;
    }

    SF_ASSERT((use & 0xFFFF) == use);
    pimage->Data.Use     = (UInt16) use;
    pimage->TextureArena = textureArena;
    pimage->pUpdateSync  = pupdateSync;
        
    // Allocate planes.
    for (unsigned plane = 0; plane < pimage->Data.RawPlaneCount; plane++)
    {
        ImageSize planeSize= ImageData::GetFormatPlaneSize(format, size, plane);
        UPInt     pitch    = ImageData::GetFormatPitch(format, planeSize.Width, plane);
        UPInt     dataSize = ImageData::GetMipLevelsSize(format, planeSize,
                                                         mipLevelCount, plane);
        UByte*    pdata = (UByte*)SF_HEAP_ALLOC(pheap, dataSize, Stat_Image_Mem);
        if (!pdata)
        {
            // Free all remaining planes, release image.
            pimage->Release();
            goto rawimage_create_memory_error;
        }
        pimage->Data.SetPlane(plane, planeSize, pitch, dataSize, pdata);
    }

    return pimage;
}

bool RawImage::GetImageData(ImageData* pdata)
{
    // TBD: Should we have copy-constructor like Initialize() instead?
    pdata->Initialize(Data.GetFormat(), Data.GetMipLevelCount(),
                      Data.pPlanes, Data.RawPlaneCount, Data.HasSeparateMipmaps());
    pdata->Use = pdata->Use;    
    pdata->pPalette  = Data.pPalette;
    return true;
}


bool RawImage::Map(ImageData* pdata, unsigned levelIndex, unsigned levelCount)
{
    SF_ASSERT((levelIndex < Data.LevelCount) &&
              (levelIndex + levelCount <= Data.LevelCount));
    if (levelCount == 0)
        levelCount = Data.LevelCount - levelIndex;

    // If we are indexing base level or have separate mipmaps, just 
    // initialize the data planes pointer to internal planes what we already have.
    
    if ((levelIndex == 0) || (Data.Flags & ImageData::Flag_SeparateMipmaps))
    {
        unsigned formatPlaneCount = ImageData::GetFormatPlaneCount(Data.Format);
        unsigned planeCount       = ((Data.Flags & ImageData::Flag_SeparateMipmaps) ? levelCount : 1)
                                    * formatPlaneCount;

        pdata->Initialize(Data.Format, levelCount,
                          Data.pPlanes + levelIndex * formatPlaneCount,
                          planeCount, (Data.Flags & ImageData::Flag_SeparateMipmaps) != 0);
    }
    else
    {
        // If we are indexing non-base mipmap level and mipmaps are not separate,
        // new planes need to be initialized.
        if (!pdata->Initialize(Data.Format, levelCount, false))
            return false;

        SF_ASSERT(Data.RawPlaneCount == pdata->RawPlaneCount);   
        for (unsigned i = 0; i< Data.RawPlaneCount; i++)
        {
            ImagePlane& splane = Data.GetPlaneRef(i);
            ImagePlane& dplane = pdata->GetPlaneRef(i);
            splane.GetMipLevel(Data.Format, levelIndex, &dplane, i);
        }
    }

    Data.Flags |= ImageData::Flag_ImageMapped;
    return true;
}

bool RawImage::Unmap()
{    
    SF_ASSERT(Data.Flags & ImageData::Flag_ImageMapped);
    Data.Flags &= ~ImageData::Flag_ImageMapped;
    return true;
}


// *** Decode

bool RawImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    ImagePlane  splane, dplane;
    ImageFormat format           = Data.GetFormat();
    unsigned    formatPlaneCount = ImageData::GetFormatPlaneCount(format);
    unsigned    planeCount       = Data.GetPlaneCount(),
                destPlaneCount   = pdest->GetPlaneCount();
    unsigned    i;

    if (Data.IsHWSpecific())
    {
        // HW-Specific formats are handled as a unit; they must have 
        // have matching size.
        SF_ASSERT((Data.GetFormatNoConv() == pdest->GetFormat()) &&
                  (Data.GetSize() == pdest->GetSize()) &&
                  (planeCount == destPlaneCount));

        for (i = 0; i< planeCount; i++)
        {
            Data.GetPlane(i, &splane);
            pdest->GetPlane(i, &dplane);
            memcpy(dplane.pData, splane.pData, splane.DataSize);
        }
        return true;
    }

    // Generic format have looser constraints as they support data
    // conversion by relying on the provided copyScanline function.
    SF_ASSERT((Data.GetFormatNoConv() == pdest->GetFormatNoConv()) &&
              (Data.GetWidth() <= pdest->GetWidth()) &&
              (Data.GetHeight() <= pdest->GetHeight()) &&
              (formatPlaneCount <= destPlaneCount));
    
    planeCount = Alg::Min(planeCount, destPlaneCount);
        
    for (i = 0; i< planeCount; i++)
    {
        Data.GetPlane(i, &splane);
        pdest->GetPlane(i, &dplane);

        // Get number of scan-lines. Typically this is the same
        // as Height but is different for compressed formats.
        unsigned planeIndex    = planeCount % formatPlaneCount;
        UPInt    bytesPerPixel = ImageData::GetFormatBytesPerPixel(format, planeIndex);
        UPInt    scanlineCount = ImageData::GetFormatScanlineCount(format, splane.Height, planeIndex);
            
        UByte* psource = splane.pData;
        UByte* pdest   = dplane.pData;

        for (UPInt j = 0; j < scanlineCount; j++, psource += splane.Pitch, pdest += dplane.Pitch)
            copyScanline(pdest, psource, bytesPerPixel * splane.Width,
                         Data.pPalette.GetPtr(), arg);
    }
    return true;
}


Texture* RawImage::GetTexture(TextureManager* pmanager)
{
    if (pTexture)
    {
        SF_ASSERT(pTexture->GetTextureManager() == pmanager);
        return pTexture;
    }

    if (!pmanager)
        return 0;

    // Since RawImage does its own mapping, don't pass Map use flags to texture creation.
    Texture* ptexture = 
        pmanager->CreateTexture(Data.Format, Data.LevelCount,
                                Data.GetSize(), (Data.Use &~ImageUse_Map_Mask), TextureArena, this);
    initTexture_NoAddRef(ptexture);
    return ptexture;
}


//--------------------------------------------------------------------
// *** TextureImage
#ifdef SF_AMP_SERVER
bool TextureImage::Decode(ImageData* pdest, CopyScanlineFunc copyScanline, void* arg) const
{
    if ( !pTexture )
        return false;

    return pTexture->Copy(pdest);
}
#endif

// TextureImage mapping should only be allowed in single-threaded rendering
// environments, in which case ImageUse_MapLocal is set.
// Such mapping may, for example, be possible on PSP.
// Note that if mapping is necessary on threaded platforms, Image::Create
// will create RawImage with an attached texture instead.

bool    TextureImage::Map(ImageData* pdata, unsigned mipLevel, unsigned levelCount)
{
    SF_ASSERT(pTexture && (Use & ImageUse_MapLocal));
    return pTexture->Map(pdata, mipLevel, levelCount);
}
bool    TextureImage::Unmap()
{
    return pTexture->Unmap();
}



}}; // namespace Scaleform::Render

