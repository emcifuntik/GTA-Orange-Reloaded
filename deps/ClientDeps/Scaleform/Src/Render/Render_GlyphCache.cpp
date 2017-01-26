/**********************************************************************

Filename    :   Render_GlyphCache.cpp
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "Render_GlyphCache.h"
#include "Render_HAL.h"
#include "Render_Image.h"
#include "Render_FontCacheHandle.h"
#include "Render_BoundBox.h"
#include "Render_Math2D.h"

#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_AmpInterface.h"

namespace Scaleform { namespace Render {


//static unsigned FontSizeRamp[] = { 0,1,2,4,8,16,32,64,128,256,0};
//static unsigned FontSizeRamp[] = { 0,1,2,4,6,8,11,16,22,32,40,50,64,76,90,108,128,147,168,194,222,256,0};
static unsigned FontSizeRamp[] = { 0,0,2,4,6,8,12,16,18,22,26,32,36,40,45,50,56,64,72,80,90,108,128,147,168,194,222,256,0};


//------------------------------------------------------------------------
VertexElement RasterGlyphVertex::Elements[] =
{
    { 0,  VET_XY32f },
    { 8,  VET_ColorARGB8 },
    { 12, VET_UV32f },
    { 0,  VET_None }
};
VertexFormat RasterGlyphVertex::Format = { sizeof(RasterGlyphVertex), RasterGlyphVertex::Elements };


//------------------------------------------------------------------------
GlyphTextureImage::GlyphTextureImage(GlyphCache* cache, unsigned textureId, 
                                     const ImageSize& size, unsigned use) : 
    TextureImage(Image_A8, size, use), pCache(cache), TextureId(textureId)
{}



//------------------------------------------------------------------------
GlyphTextureImage* GlyphTextureImage::Create(MemoryHeap* heap, TextureManager* texMan, GlyphCache* cache, 
                                             unsigned textureId, const ImageSize& size, unsigned use)
{
    GlyphTextureImage* img = SF_HEAP_NEW(heap) GlyphTextureImage(cache, textureId, size, use);
    Texture* tex = texMan->CreateTexture(Image_A8, 1, size, use, 0, img);
    if (!tex)
    {
        img->Release();
        return 0;
    }
    img->initTexture_NoAddRef(tex);
    return img;
}


//------------------------------------------------------------------------
void GlyphTextureImage::TextureLost(TextureLossReason reason)
{
    TextureImage::TextureLost(reason);
    pCache->TextureLost(TextureId, reason);
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Create(unsigned method, MemoryHeap* heap, HAL* ren, 
                                TextureManager* texMan, PrimitiveFillManager* fillMan, GlyphCache* cache, 
                                unsigned textureId, const ImageSize& size)
{
    SF_AMP_SCOPE_RENDER_TIMER_ID("GlyphTextureMapper::Create", Amp_Native_Function_Id_GlyphTextureMapper_Create);

    pTexMan = texMan;
    unsigned use = 0;
    bool ret = false;
    Method = method;
    if (method == GlyphCache::TU_WholeImage)
    {
        pTexImg = 0;
        use |= ImageUse_Update;
        pRawImg = *RawImage::Create(Image_A8, 1, size, use, heap);
        ret = pRawImg.GetPtr() != 0;
        if (ret)
        {
            PrimitiveFillData fillData(PrimFill_UVTextureAlpha_VColor, &RasterGlyphVertex::Format, 
                                       pRawImg->GetTexture(texMan), ImageFillMode(Wrap_Clamp, Sample_Linear));
            pFill = *fillMan->CreateFill(fillData);
        }
    }
    else
    {
        pRawImg = 0;
        if (method == GlyphCache::TU_MultipleUpdate)
            use |= ImageUse_PartialUpdate;
        else
            use |= ImageUse_MapLocal | ImageUse_MapInUpdate;  // TO DO: Verify ImageUse_MapLocal!

        pTexImg = *GlyphTextureImage::Create(heap, texMan, cache, textureId, size, use);
        ret = pTexImg.GetPtr() != 0;
        if (ret)
        {
            PrimitiveFillData fillData(PrimFill_UVTextureAlpha_VColor, &RasterGlyphVertex::Format, 
                                       pTexImg->GetTexture(texMan), ImageFillMode(Wrap_Clamp, Sample_Linear));
            pFill = *fillMan->CreateFill(fillData);
        }
    }
    return Valid = ret;
}


//------------------------------------------------------------------------
ImagePlane* GlyphTextureMapper::Map()
{
    ImagePlane* plane = 0;
    switch(Method)
    {
    case GlyphCache::TU_DirectMap:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        if (!Mapped)
        {
            if (!pTexImg->Map(&Data, 0, 1))
                break;
            Mapped = true;
        }
        plane = Data.pPlanes;
        break;

    case GlyphCache::TU_MultipleUpdate:
        break;

    case GlyphCache::TU_WholeImage:
        SF_ASSERT(pRawImg.GetPtr() != 0);
        if (!Mapped)
        {
            if (!pRawImg->Map(&Data))
                break;
            Mapped = true;
        }
        plane = Data.pPlanes;
        break;
    }
    return plane;
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Unmap()
{
    bool ret = false;
    switch(Method)
    {
    case GlyphCache::TU_DirectMap:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        if (Mapped)
        {
            ret = pTexImg->Unmap();
            Mapped = false;
        }
        break;

    case GlyphCache::TU_MultipleUpdate:
        SF_ASSERT(pTexImg.GetPtr() != 0);
        break;

    case GlyphCache::TU_WholeImage:
        SF_ASSERT(pRawImg.GetPtr() != 0);
        if (Mapped)
        {
            ret = pRawImg->Unmap();
            Texture* tex = pRawImg->GetTexture(pTexMan);
            tex->Update();
            Mapped = false;
        }
        break;
    }
    return ret;
}


//------------------------------------------------------------------------
bool GlyphTextureMapper::Update(const Texture::UpdateDesc* updates, unsigned count)
{
    bool ret = false;
    if (Method == GlyphCache::TU_MultipleUpdate)
    {
        SF_ASSERT(pTexImg.GetPtr() != 0);
        Texture* tex = pTexImg->GetTexture(pTexMan);
        if (tex)
        {
            ret = tex->Update(updates, count);
        }
    }
    return ret;
}


//------------------------------------------------------------------------
Texture* GlyphTextureMapper::GetTexture()
{
    return (Method == GlyphCache::TU_WholeImage) ?
        pRawImg->GetTexture(pTexMan) : pTexImg->GetTexture(pTexMan);
}







//------------------------------------------------------------------------
GlyphCache::GlyphCache(MemoryHeap* heap) : 
    pHeap(heap),
    pRenderer(0),
    pFillMan(0),
    pTexMan(0),
    TextureWidth(0),
    TextureHeight(0),
    MaxNumTextures(0),
    MaxSlotHeight(0),
    SlotPadding(0),
    ScaleU(0),
    ScaleV(0),
    Method(TU_DirectMap),
    UpdatePacker(0,0),
    pRQCaches(0),
    Ras(heap)
{
    Notifier.pCache = this;

    unsigned i;
    unsigned ramp = 0;
    for (i = 0; i < 256; ++i)
    {
        if (i > FontSizeRamp[ramp + 1])
            ++ramp;
        FontSizeMap[i] = (UByte)ramp;
    }
}

//------------------------------------------------------------------------
GlyphCache::~GlyphCache()
{
    Destroy();
}

//------------------------------------------------------------------------
void GlyphCache::Destroy()
{
    UnpinAllSlots();
    Queue.Clear();
    releaseAllTextures();
    UpdatePacker.Clear();
    GlyphsToUpdate.Clear();
    RectsToUpdate.Clear();
    //pRenderer = 0;
    //pFillMan  = 0;
    //pTexMan   = 0;
    if (pFontHandleManager)
    {
        pFontHandleManager->DestroyAllFonts();
        pFontHandleManager.Clear();
    }
    if (pRQCaches)
    {
        pRQCaches->SetCache(Cache_Glyph, 0);
        pRQCaches = 0;
    }
}

//------------------------------------------------------------------------
void GlyphCache::ClearCache()
{
    UnpinAllSlots();
    Queue.Clear();
    UpdatePacker.Clear();
    GlyphsToUpdate.Clear();
    RectsToUpdate.Clear();
}

bool GlyphCache::SetParams(const GlyphCacheParams& params)
{
    Param = params;
    if (pRenderer)
        initialize();
    return true;
}


//------------------------------------------------------------------------
void GlyphCache::PrintMemStats()
{
    Queue.PrintMemStats();
}

unsigned GlyphCache::GetNumTextures() const
{
    unsigned numTextures = 0;
    for (unsigned i = 0; i < MaxNumTextures; ++i)
    {
        if (Textures[i].IsValid())
        {
            ++numTextures;
        }
    }
    return numTextures;

}

// Initialize or re-initialize the cache. The physical structure of the cache 
// slots is shown above. The function also clears the cache.
//
//------------------------------------------------------------------------
void GlyphCache::initialize()
{
    Destroy();

    SF_ASSERT( pTexMan != NULL );

    unsigned texWidth       = Param.TextureWidth;
    unsigned texHeight      = Param.TextureHeight;
    unsigned maxNumTextures = Param.NumTextures;
    unsigned maxSlotHeight  = Param.MaxSlotHeight;
    unsigned slotPadding    = Param.SlotPadding;
    unsigned texUpdWidth    = Param.TexUpdWidth;
    unsigned texUpdHeight   = Param.TexUpdHeight;

    unsigned w  = (texWidth  < 64) ? 63 : texWidth  - 1;
    unsigned h  = (texHeight < 64) ? 63 : texHeight - 1;
    unsigned sw = 0;
    unsigned sh = 0;

    while(w) { ++sw; w >>= 1; }
    while(h) { ++sh; h >>= 1; }

    if (maxNumTextures > TexturePoolSize)
        maxNumTextures = TexturePoolSize;

    TextureWidth         = 1 << sw;
    TextureHeight        = 1 << sh;
    MaxNumTextures       = maxNumTextures;
    MaxSlotHeight        = maxSlotHeight;
    SlotPadding          = slotPadding;
    ScaleU               = 1.0f / (float)TextureWidth;
    ScaleV               = 1.0f / (float)TextureHeight;

    Queue.Init(&Notifier, 0, maxNumTextures, texWidth, texHeight, maxSlotHeight);
    unsigned caps = pTexMan->GetTextureUseCaps(Image_A8);
    if (caps & ImageUse_MapInUpdate)
    {
        Method = TU_DirectMap;
    }
    else
    if (caps & ImageUse_PartialUpdate)
    {
        Method = TU_MultipleUpdate;
        UpdatePacker = TextureUpdatePacker(texUpdWidth, texUpdHeight);
        UpdateBuffer = *RawImage::Create(Image_A8, 1, ImageSize(texUpdWidth, texUpdHeight), 0, pHeap);
    }
    else
    {
        Method = TU_WholeImage;
    }

    unsigned i;
    for(i = 0; i < MaxNumTextures; ++i)
    {
        Textures[i].Create(Method, pHeap, pRenderer, pTexMan, pFillMan, this, i, 
                           ImageSize(TextureWidth, TextureHeight));
    }

    // Must be created in Global Heap
    pFontHandleManager = *SF_NEW FontCacheHandleManager(pHeap, this);

    PrimitiveFillData fillDataSolid(PrimFill_VColor_EAlpha, &VertexXY16iCF32::Format);
    PrimitiveFillData fillDataMask(PrimFill_Mask, &VertexXY16i::Format);
    pSolidFill = *pFillMan->CreateFill(fillDataSolid);
    pMaskFill  = *pFillMan->CreateFill(fillDataMask);

    pRQCaches  = &pRenderer->GetRQCacheInterface();
    pRQCaches->SetCache(Cache_Glyph, this);
}

//------------------------------------------------------------------------
void GlyphCache::Initialize(HAL* ren, PrimitiveFillManager* fillMan)
{
    pRenderer = ren;
    pFillMan  = fillMan;
    pTexMan   = ren->GetTextureManager();
    initialize();
}


//------------------------------------------------------------------------
void GlyphCache::OnBeginFrame()
{
    if (pFontHandleManager)
        pFontHandleManager->ProcessKillList();
}

//------------------------------------------------------------------------
void GlyphCache::MergeCacheSlots()
{
    Queue.MergeEmptySlots();
}

//------------------------------------------------------------------------
void GlyphCache::OnEndFrame()
{
}

//------------------------------------------------------------------------
void GlyphCache::CleanUpFont(FontCacheHandle* font)
{
    SF_ASSERT(font->pFont == 0);
    ApplyInUseList();
    UpdatePinList();
    Queue.CleanUpFont(font);
}

//------------------------------------------------------------------------
PrimitiveFill* GlyphCache::GetFill(TextLayerType type, unsigned textureId)
{
    switch(type)
    {
    case TextLayer_Background:
    case TextLayer_Selection:
        return pSolidFill;

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        return Textures[textureId].GetFill();

    case TextLayer_Images:
        return 0;       // TO DO

    case TextLayer_Shapes:
    case TextLayer_Underline:
    case TextLayer_Cursor:
        return pSolidFill;

    case TextLayer_Mask:
        return pMaskFill;

    case TextLayer_Shapes_Masked:
    case TextLayer_Underline_Masked:
        return pSolidFill;
    }
    return 0;
}


//------------------------------------------------------------------------
Image* GlyphCache::GetImage(unsigned textureId)
{
    SF_ASSERT(textureId < MaxNumTextures);
    return Textures[textureId].GetImage();
}


//------------------------------------------------------------------------
void GlyphCache::partialUpdateTextures()
{
    SF_ASSERT(UpdateBuffer.GetPtr() != 0);

    ImageData d;
    UpdateBuffer->GetImageData(&d);

    unsigned i, j;
    for(i = 0; i < MaxNumTextures; ++i)
    {
        GlyphTextureMapper& tex = Textures[i];
        if (tex.NumGlyphsToUpdate)
        {
            RectsToUpdate.Resize(tex.NumGlyphsToUpdate, 32);
            unsigned numRects = 0;
            for (j = 0; j < (unsigned)GlyphsToUpdate.GetSize(); j++)
            {
                if (GlyphsToUpdate[j].TextureId == i)
                {
                    SF_ASSERT(numRects < (unsigned)RectsToUpdate.GetSize());
                    const UpdateRect& r = GlyphsToUpdate[j];
                    UpdateDesc& desc = RectsToUpdate[numRects++];
                    ImageData data;
                    UpdateBuffer->GetImageData(&data);
                    desc.DestRect.x1 = r.DstX;
                    desc.DestRect.y1 = r.DstY;
                    desc.DestRect.x2 = r.DstX + r.w;
                    desc.DestRect.y2 = r.DstY + r.h;
                    desc.SourcePlane = d.pPlanes[0];
                    desc.SourcePlane.pData = data.pPlanes[0].GetScanline(r.SrcY) + r.SrcX;
                    desc.PlaneIndex = 0;
                }
            }
            tex.Update(&RectsToUpdate[0], numRects);
// DBG
//printf("\nUpd %d Packed %d\n", numRects, SlotQueue.GetNumPacked());
            tex.NumGlyphsToUpdate = 0;
        }
    }
    GlyphsToUpdate.Clear();
    UpdatePacker.Clear();
}



//------------------------------------------------------------------------
void GlyphCache::copyImageData(ImagePlane* pl, const UByte* data, unsigned pitch, 
                               unsigned dstX, unsigned dstY, unsigned w, unsigned h)
{
    unsigned i;
    for (i = 0; i < h; ++i)
    {
        UByte* p = pl->GetScanline(dstY+i);
        memcpy(p+dstX, data, w);
        data += pitch;
    }
}

//------------------------------------------------------------------------
bool GlyphCache::UpdateTextureGlyph(unsigned textureId, const UByte* data, unsigned pitch, 
                                    unsigned dstX, unsigned dstY, unsigned w, unsigned h)
{
    SF_AMP_SCOPE_RENDER_TIMER("GlyphCache::UpdateTextureGlyph");
    if (!Textures[textureId].IsValid())
    {
        Textures[textureId].Create(Method, pHeap, pRenderer, pTexMan, pFillMan, this, textureId, 
                                   ImageSize(TextureWidth, TextureHeight));
    }

    pRQCaches->SetCacheLocked(Cache_Glyph);
    if (Method == TU_MultipleUpdate)
    {
        unsigned updX, updY;
        if (!UpdatePacker.Allocate(w, h, &updX, &updY))
        {
            partialUpdateTextures();
            if (!UpdatePacker.Allocate(w, h, &updX, &updY))
            {
                return false;
            }
        }
        ImageData d;
        UpdateBuffer->GetImageData(&d);
        copyImageData(d.pPlanes, data, pitch, updX, updY, w, h);
        Textures[textureId].NumGlyphsToUpdate++;
        UpdateRect r;
        r.SrcX = updX;
        r.SrcY = updY;
        r.DstX = dstX;
        r.DstY = dstY;
        r.w = w;
        r.h = h;
        r.TextureId = textureId;
        GlyphsToUpdate.PushBack(r);
        return true;
    }
    else
    {
        ImagePlane* p = Textures[textureId].Map();
        if (p)
        {
            copyImageData(p, data, pitch, dstX, dstY, w, h);
            return true;
        }
    }
    return false;
}


//------------------------------------------------------------------------
void GlyphCache::UnlockBuffers()
{
    unsigned i;
    if (Method == TU_MultipleUpdate)
    {
        partialUpdateTextures();
    }
    for(i = 0; i < MaxNumTextures; ++i)
        Textures[i].Unmap();
    pRQCaches->ClearCacheLocked(Cache_Glyph);
}


//------------------------------------------------------------------------
void GlyphCache::releaseAllTextures()
{
    for(unsigned i = 0; i < MaxNumTextures; ++i)
        Textures[i].Invalidate();
    GlyphsToUpdate.Clear();
}


//------------------------------------------------------------------------
void GlyphCache::TextureLost(unsigned textureId, unsigned reason)
{
    Queue.CleanUpTexture(textureId);
    UPInt i, j;
    for(i = j = 0; i < GlyphsToUpdate.GetSize(); i++)
    {
        if (GlyphsToUpdate[i].TextureId != textureId)
            GlyphsToUpdate[j++] = GlyphsToUpdate[i];
    }
    GlyphsToUpdate.CutAt(j);
    Textures[textureId].Invalidate();
}



//------------------------------------------------------------------------
FontCacheHandle* GlyphCache::RegisterFont(Font* font)
{
    SF_ASSERT(pFontHandleManager.GetPtr() != 0);
    return pFontHandleManager->RegisterFont(font);
}


//------------------------------------------------------------------------
GlyphNode* GlyphCache::FindGlyph(TextMeshProvider* tm, const GlyphParam& gp)
{
    GlyphNode* node = Queue.FindGlyph(gp);
    if (node)
    {
        tm->AddNotifier(Queue.CreateNotifier(node, tm));
    }
    return node;
}

//------------------------------------------------------------------------
GlyphNode* GlyphCache::allocateGlyph(TextMeshProvider* tm, const GlyphParam& gp, unsigned w, unsigned h)
{
    GlyphNode* node = Queue.AllocateGlyph(gp, w, h);
    if (node)
    {
        tm->AddNotifier(Queue.CreateNotifier(node, tm));
    }
    return node;
}


//------------------------------------------------------------------------
// The function can be called from EndFrame, when all the batches are processed 
// for sure. In average it should be more efficient than calling UnpinSlots()
// for each TextMeshProvider.
void GlyphCache::UnpinAllSlots()
{
    TextMeshProvider* tm = TextInUse.GetFirst();
    while(!TextInUse.IsNull(tm))
    {
        tm->ClearInList();
        tm = tm->pNext;
    }
    tm = TextInPin.GetFirst();
    while(!TextInPin.IsNull(tm))
    {
        tm->ClearInList();
        tm = tm->pNext;
    }
    TextInUse.Clear();
    TextInPin.Clear();
    Queue.UnpinAllSlots();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs when the slot releases. The function
// is called for each TextMeshProvider associated with a particular slot in GlyphQueue.
void GlyphCache::EvictText(TextMeshProvider* tm)
{
    tm->OnEvictSlots();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs before eviction to apply and pin all slots 
// in batches that are in process.
void GlyphCache::ApplyInUseList()
{
    TextMeshProvider* tm = TextInUse.GetFirst();
    while(!TextInUse.IsNull(tm))
    {
        SF_ASSERT(tm->IsInUseList());
        TextMeshProvider* next = tm->pNext;
        tm->SetInUseList(false);
        tm->PinSlots();
        TextInPin.PushBack(tm);
        tm->SetInPinList(true);
        tm = next;
    }
    TextInUse.Clear();
}

//------------------------------------------------------------------------
// Call-back from GlyphQueue. Occurs when an attempt to evict a slot
// failed. This function iterates through the "pin list", verifies if the 
// batch is in process, and if not, unpins the slots, and removes the batch
// from the list. Then GlyphQueue makes a second attempt to allocate a slot.
bool GlyphCache::UpdatePinList()
{
    bool ret = false;
    TextMeshProvider* tm = TextInPin.GetFirst();
    while(!TextInPin.IsNull(tm))
    {
        SF_ASSERT(tm->IsInPinList());
        TextMeshProvider* next = tm->pNext;
        if (!tm->IsInUse())
        {
            tm->SetInPinList(false);
            tm->UnpinSlots();
            tm->RemoveNode();
            ret = true;
        }
        tm = next;
    }
    return ret;
}






//-----------------------------------------------------------------------
const ShapeDataInterface* GlyphCache::GetGlyphShape(GlyphRunData& data, unsigned glyphIndex, float screenSize)
{
    bool needsTmpShape = false;
    const ShapeDataInterface* shapeData = 0;
    data.HintedSize = 0;
    data.GlyphBounds = RectF(0,0,0,0);
    if (data.pFont->pFont->IsHintedVectorGlyph(glyphIndex, unsigned(screenSize)))
    {
        data.HintedSize = unsigned(screenSize);
        needsTmpShape = true;
    }
    else
    {
        shapeData = data.pFont->pFont->GetPermanentGlyphShape(glyphIndex);
        if (shapeData == 0)
            needsTmpShape = true;
    }
    data.NomWidth  = data.pFont->pFont->GetNominalGlyphWidth();
    data.NomHeight = data.pFont->pFont->GetNominalGlyphHeight();

    // TO DO: Implement vector glyph shape search

    if (needsTmpShape)
    {
        if (pGlyphShape.GetPtr())
            pGlyphShape->Clear();
        else
            pGlyphShape = *SF_HEAP_NEW(pHeap) GlyphShape;
        if (data.pFont->pFont->GetTemporaryGlyphShape(glyphIndex, data.HintedSize, pGlyphShape))
        {
            shapeData = pGlyphShape;
        }
    }

    // TO DO: Implement faux bold & italic
    // TO DO: Implement vector glyph shape store

    if (shapeData && !shapeData->IsEmpty())
        data.GlyphBounds = ComputeBoundsFill(*shapeData, Matrix2F(), Bound_AllEdges);
    return shapeData;
}

















//------------------------------------------------------------------------
class ImgBlurWrapperX 
{
public:
    ImgBlurWrapperX(UByte* img, unsigned pitch, unsigned x, unsigned y, unsigned w, unsigned h) :
        Img(img), Pitch(pitch), Sx(x), Sy(y), W(w), H(h) {}

    unsigned GetWidth()  const { return W; }
    unsigned GetHeight() const { return H; }
    UInt8 GetPixel(unsigned x, unsigned y) const { return Img[Pitch*(Sy+y) + Sx+x]; }

    void CopySpanTo(unsigned x, unsigned y, unsigned len, const UInt8* buf)
    {
        memcpy(Img + Pitch*(Sy+y) + Sx+x, buf, len);
    }

private:
    ImgBlurWrapperX(const ImgBlurWrapperX&);
    const ImgBlurWrapperX& operator = (const ImgBlurWrapperX&);
    UByte* Img;
    unsigned Pitch;
    unsigned Sx, Sy, W, H;
};


//------------------------------------------------------------------------
class ImgBlurWrapperY
{
public:
    ImgBlurWrapperY(UByte* img, unsigned pitch, unsigned x, unsigned y, unsigned w, unsigned h) :
        Img(img), Pitch(pitch), Sx(x), Sy(y), W(w), H(h) {}

    unsigned  GetWidth()  const { return H; }
    unsigned  GetHeight() const { return W; }
    UInt8 GetPixel(unsigned x, unsigned y) const { return Img[Pitch*(Sy+x) + Sx+y]; }

    void CopySpanTo(unsigned x, unsigned y, unsigned len, const UInt8* buf)
    {
        UInt8* p = Img + Pitch*(Sy+x) + Sx+y;
        do
        {
            *p  = *buf++;
             p += Pitch;
        }
        while(--len);
    }

private:
    ImgBlurWrapperY(const ImgBlurWrapperY&);
    const ImgBlurWrapperY& operator = (const ImgBlurWrapperY&);
    UByte* Img;
    unsigned Pitch;
    unsigned Sx, Sy, W, H;
};


//------------------------------------------------------------------------
template<class Img, class SumBuf, class ColorBuf>
void RecursiveBlur(Img& img, float radius, SumBuf& sum, ColorBuf& buf)
{
    if (radius < 0.62f) 
        radius = 0.62f;

    int w  = img.GetWidth();
    int h  = img.GetHeight();
    int wb = w + int(ceilf(radius));
    int x, y;

    if (wb < 3)
        return;

    float s = radius * 0.5f;
    float q = (s < 2.5f) ?
               3.97156f - 4.14554f * sqrtf(1 - 0.26891f * s) :
               0.98711f * s - 0.96330f;

    float q2 = q  * q;
    float q3 = q2 * q;

    float b0 =  1.0f / (1.578250f + 2.444130f * q + 1.428100f * q2 + 0.422205f * q3);
    float b2 =  2.44413f * q + 2.85619f * q2 + 1.26661f * q3;
    float b3 = -1.42810f * q2 + -1.26661f * q3;
    float b4 =  0.422205f * q3;

    float b1  = 1.0f - (b2 + b3 + b4) * b0;

    b2 *= b0;
    b3 *= b0;
    b4 *= b0;

    sum.Resize(2*wb);
    buf.Resize(wb);

    float* sum1 = &sum[0];
    float* sum2 = &sum[wb];

    for(y = 0; y < h; y++)
    {
        UInt8 c;
        c = img.GetPixel(0, y); sum1[0] = b1*c + b2*c       + b3*c       + b4*c;
        c = img.GetPixel(1, y); sum1[1] = b1*c + b2*sum1[0] + b3*sum1[0] + b4*sum1[0];
        c = img.GetPixel(2, y); sum1[2] = b1*c + b2*sum1[1] + b3*sum1[0] + b4*sum1[0];

        for(x = 3; x < wb; ++x)
        {
            c = (x < w) ? img.GetPixel(x, y) : 0;
            sum1[x] = b1*c + b2*sum1[x-1] + b3*sum1[x-2] + b4*sum1[x-3];
        }

        sum2[wb-1] = b1*sum1[wb-1] + b2*sum1[wb-1] + b3*sum1[wb-1] + b4*sum1[wb-1];
        sum2[wb-2] = b1*sum1[wb-2] + b2*sum2[wb-1] + b3*sum2[wb-1] + b4*sum2[wb-1];
        sum2[wb-3] = b1*sum1[wb-3] + b2*sum2[wb-2] + b3*sum2[wb-1] + b4*sum2[wb-1];
        buf [wb-1] = UInt8(sum2[wb-1] + 0.5f);
        buf [wb-2] = UInt8(sum2[wb-2] + 0.5f);
        buf [wb-3] = UInt8(sum2[wb-3] + 0.5f);

        for(x = wb-4; x >= 0; --x)
        {
            sum2[x] = b1*sum1[x] + b2*sum2[x+1] + b3*sum2[x+2] + b4*sum2[x+3];
            buf [x] = UInt8(sum2[x] + 0.5f);
        }
        img.CopySpanTo(0, y, w, &buf[0]);
    }
}

//------------------------------------------------------------------------
void GlyphCache::recursiveBlur(UByte* img, unsigned pitch,
                               unsigned  sx, unsigned  sy, 
                               unsigned  w,  unsigned  h, 
                               float rx, float ry)
{
    ImgBlurWrapperX imgWX(img, pitch, sx, sy, w, h); 
    RecursiveBlur(imgWX, rx, BlurSum, BlurStack);
    ImgBlurWrapperY imgWY(img, pitch, sx, sy, w, h); 
    RecursiveBlur(imgWY, ry, BlurSum, BlurStack);
}


//------------------------------------------------------------------------
void GlyphCache::strengthenImage(UByte* img, unsigned pitch, 
                                 unsigned  sx, unsigned  sy, 
                                 unsigned  w,  unsigned  h, 
                                 float ratio,
                                 int  bias)
{
    if (ratio != 1.0f)
    {
        unsigned x, y;
        for(y = 0; y < h; ++y)
        {
            UInt8* p = img + (sy + y)*pitch + sx;
            for(x = 0; x < w; ++x)
            {
                int v = int((int(*p) - bias) * ratio + 0.5f) + bias;
                if (v <   0) v = 0;
                if (v > 255) v = 255;
                *p++ = UInt8(v);
            }
        }
    }
}

/*
//------------------------------------------------------------------------
void GlyphCache::makeKnockOutCopy(const GImage& img, 
                                           UInt  sx, UInt  sy, 
                                           UInt  w,  UInt  h)
{
    if (KnockOutCopy.GetPtr() == 0 ||
        KnockOutCopy->Width < w ||
        KnockOutCopy->Height < h)
    {
        KnockOutCopy = *GHEAP_AUTO_NEW(this) 
            GImage(GImageBase::Image_A_8, w + 16, h + 16);
    }

    UInt y;
    for(y = 0; y < h; ++y)
    {
        const UInt8* src = img.GetScanline(sy + y) + sx;
              UInt8* dst = KnockOutCopy->GetScanline(y);
        memcpy(dst, src, w);
    }
}

//------------------------------------------------------------------------
void GFxGlyphRasterCache::knockOut(GImage& img, 
                                   UInt  sx, UInt  sy, 
                                   UInt  w,  UInt  h)
{
    if (KnockOutCopy.GetPtr() != 0)
    {
        UInt x, y;
        for(y = 0; y < h; ++y)
        {
            const UInt8* src = KnockOutCopy->GetScanline(y);
                  UInt8* dst = img.GetScanline(sy + y) + sx;
            for(x = 0; x < w; ++x)
            {
                UInt s = 255 - *src++;
                UInt d = *dst;
                *dst++ = UInt8((s * d + 255) >> 8);
            }
        }
    }
}
*/






//-----------------------------------------------------------------------
void GlyphCache::addShapeToRasterizer(const ShapeDataInterface* shape, float scaleX, float scaleY)
{
    if (shape->IsEmpty())
        return;

    // TO DO: Get from Renderer2D and possibly tweak.
    ToleranceParams param;

    ShapePosInfo pos(shape->GetStartingPos());
    ShapePathType pathType;
    PathEdgeType pathEdge;
    float coord[Edge_MaxCoord];
    unsigned styles[3];
    bool first = true;
    while((pathType = shape->ReadPathInfo(&pos, coord, styles)) != Shape_EndShape)
    {
        if (!first && pathType == Shape_NewLayer)
            break;
        first = false;

        if (styles[0] != styles[1])
        {
            coord[0] *= scaleX;
            coord[1] *= scaleY;
            Ras.MoveTo(coord[0], coord[1]);
            while((pathEdge = shape->ReadEdge(&pos, coord)) != Edge_EndPath)
            {
                if (pathEdge == Edge_LineTo)
                {
                    coord[0] *= scaleX;
                    coord[1] *= scaleY;
                    Ras.LineTo(coord[0], coord[1]);
                }
                else
                {
                    coord[0] *= scaleX;
                    coord[1] *= scaleY;
                    coord[2] *= scaleX;
                    coord[3] *= scaleY;
                    Math2D::TessellateQuadCurve(&Ras, param, coord[0], coord[1], coord[2], coord[3]);
                }
            }
            Ras.ClosePolygon();
        }
        else
        {
            shape->SkipPathData(&pos);
        }
    }
}




//-----------------------------------------------------------------------
float GlyphCache::SnapFontSizeToRamp(float screenSize) const
{
    unsigned size = unsigned(floor(screenSize));
    size = size + ((size + 3) >> 2);
    size =(size <= 255) ? FontSizeRamp[FontSizeMap[size] + 1] : 255;
    return (float)size;
}


//-----------------------------------------------------------------------
bool GlyphCache::GlyphFits(unsigned screenHeight) const
{
    return screenHeight < MaxSlotHeight * Param.MaxRasterScale;
}

//-----------------------------------------------------------------------
float GlyphCache::GetCachedFontSize(const GlyphParam& gp, float screenSize, bool exactFit) const
{
    // TO DO: Add logic for OptRead, Shadows, etc.
    //screenSize *= Param.SharpeningFactor; // DBG

    if (exactFit)// || gp.IsOptRead()) // TO DO: Restore
        return screenSize;

    screenSize = SnapFontSizeToRamp(screenSize);
    //return (screenSize <= 255.0f) ? screenSize : 255.0f;
    return (screenSize <= float(MaxSlotHeight-2*SlotPadding)) ? screenSize : float(MaxSlotHeight-2*SlotPadding); // TO DO: Revise. Temporary solution for big glyphs
}


//-----------------------------------------------------------------------
float GlyphCache::GetCachedShadowSize(const GlyphParam& gp, float screenSize) const
{
    //screenSize += gp.GetBlurY() * 2;
    screenSize = SnapFontSizeToRamp(screenSize);
    return (screenSize <= float(MaxSlotHeight-2*SlotPadding)) ? screenSize : float(MaxSlotHeight-2*SlotPadding); // TO DO: Revise. Temporary solution for big glyphs
}





//-----------------------------------------------------------------------
GlyphNode* GlyphCache::getPrerasterizedGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp)
{
    if (!data.pFont->pFont->GetGlyphRaster(gp.GlyphIndex, (unsigned)gp.GetFontSize(), &Raster))
    {
        Result = Res_ShapeNotFound;
        return 0;
    }

    int y1 = -Raster.OriginY;
    int y2 = y1 + Raster.Height;
    unsigned h = unsigned(y2 - y1) + 2*SlotPadding;

    if (h >= MaxSlotHeight)
    {
        Result = Res_ShapeIsTooBig;
        return 0;
    }

    int padX = SlotPadding;
    int padY = SlotPadding;

    int imgX1 = -Raster.OriginX - padX;
    int imgX2 = -Raster.OriginX + Raster.Width + padX;
    int imgY1 = -Raster.OriginY - padY;
    int imgY2 = -Raster.OriginY + Raster.Height + padY;

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
        imgH = MaxSlotHeight;

    GlyphNode* node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        return 0;
    }

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = 1;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);

    unsigned i;
    for(i = 0; i < Raster.Height && (padY+i) < imgH; ++i)
    {
        UByte* sl = &RasterData[(padY+i)*RasterPitch + padX];
        memcpy(sl, &Raster.Raster[i * Raster.Width], Raster.Width);
    }

    UpdateTextureGlyph(GetTextureId(node), &RasterData[0], RasterPitch, 
        node->mRect.x, node->mRect.y, node->mRect.w, node->mRect.h); 

    return node;
}

//-----------------------------------------------------------------------
GlyphNode* GlyphCache::RasterizeGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp)
{
    SF_AMP_SCOPE_RENDER_TIMER("GlyphCache::RasterizeGlyph");
    if (data.pFont->pFont->IsHintedRasterGlyph(gp.GlyphIndex, (unsigned)gp.GetFontSize()))
    {
        return getPrerasterizedGlyph(data, tm, gp);
    }

//printf("G");
    const ShapeDataInterface* shape = GetGlyphShape(data, gp.GlyphIndex, gp.GetFontSize());
    if (shape == 0)
    {
        Result = Res_ShapeNotFound;
        return 0;
    }
    float scale = gp.GetFontSize() / data.NomHeight;
    float y1 = floor(data.GlyphBounds.y1 * scale);
    float y2 =  ceil(data.GlyphBounds.y2 * scale);
    unsigned h = unsigned(y2 - y1) + 2*SlotPadding;

    if (h >= MaxSlotHeight)
    {
        Result = Res_ShapeIsTooBig;
        return 0;
    }

//if (h >= MaxSlotHeight)     // TO DO: Revise. Temporary solution for big glyphs
//{
//    scale *= float(MaxSlotHeight) / float(h+1);
//}

    GlyphNode* node = 0;

    // Rasterization
    Ras.Clear();
    addShapeToRasterizer(shape, scale, scale);

    // TO DO
    int stretch  = 1;
    int padX = SlotPadding;
    int padY = SlotPadding;

    int imgX1 = -padX;
    int imgX2 =  padX;
    int imgY1 = -padY;
    int imgY2 =  padY;

    bool cellsToSweep = false;
    //if (raster) // TO DO
    //{
    //    imgX1 = -raster->OriginX - padX;
    //    imgY1 = -raster->OriginY - padY;
    //    imgX2 =  raster->Width   - 1 - raster->OriginX + padX;
    //    imgY2 =  raster->Height  - 1 - raster->OriginY + padY;
    //}
    //else
    {
        if (Ras.SortCells())
        {
            cellsToSweep = true;
            imgX1 = Ras.GetMinX() - padX;
            imgX2 = Ras.GetMaxX() + padX;
            imgY1 = Ras.GetMinY() - padY;
            imgY2 = Ras.GetMaxY() + padY;
        }
    }

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
        imgH = MaxSlotHeight;
    //SF_ASSERT(imgH <= MaxSlotHeight);

    node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        return 0;
    }

    //UInt imgX, imgY;
    //if (!TexUpdPacker.Allocate(imgW, imgH, &imgX, &imgY))
    //{
    //    UpdateTextures(ren);
    //    if (!TexUpdPacker.Allocate(imgW, imgH, &imgX, &imgY))
    //        return 0;
    //}

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = 1;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);
/*
memset(&RasterData[(padY + 0)     *RasterPitch] + padX, 0xFF, imgW - padX*2);
memset(&RasterData[(imgH - padY*2)*RasterPitch] + padX, 0xFF, imgW - padX*2);
for(int i = padY; i < imgH-padY*2; ++i)
{
    UByte* sl = &RasterData[i*RasterPitch];
    sl[padX] = 0xFF;
    sl[imgW-padX] = 0xFF;
}
*/

    // TO DO: Add stretching and filtering logic
    unsigned i;
    for(i = 0; i < Ras.GetNumScanlines() && (padY+i) < imgH; ++i)
    {
        UByte* sl = &RasterData[(padY+i)*RasterPitch + padX];
        Ras.SweepScanline(i, sl);
    }

    UpdateTextureGlyph(GetTextureId(node), &RasterData[0], RasterPitch, 
        node->mRect.x, node->mRect.y, node->mRect.w, node->mRect.h); 


    //for (i = 0; i < imgH; ++i)
    //    memset(img.GetScanline(imgY + i) + imgX, 0, imgW);

// DBG
//for (i = 1; i < imgH-1; ++i)
//{
//    memset(img.GetScanline(imgY + i) + imgX + 1, 255, imgW-2);
//}
//if(imgH > 4 && imgW > 4)
//{
//    for (i = 2; i < imgH-2; ++i)
//    {
//        memset(img.GetScanline(imgY + i) + imgX + 2, 0, imgW-4);
//        img.GetScanline(imgY + i)[imgX + imgW / 2] = 255;
//    }
//    memset(img.GetScanline(imgY + imgH/2) + imgX + 2, 255, imgW-4);
//}

    if (cellsToSweep)
    {
        // TO DO:
        //float gamma = 1.0f;
        //if (gp.BlurX || gp.BlurY)
        //{
        //    // Gamma correction improves the "strength" of glow and shadow
        //    // TO DO: Think of smarter control of it.
        //    gamma = 0.4f;
        //}
        //if (gamma != Ras.GetGamma())
        //    Ras.SetGamma(gamma);


        //if (imgW < 5 || stretch < 3)
        //{
        //    for(i = 0; i < Ras.GetNumScanlines(); ++i)
        //    {
        //        Ras.SweepScanline(i, img.GetScanline(imgY + padY + i) + imgX + padX);
        //    }
        //}
        //else
        //{
        //    for(i = 0; i < Ras.GetNumScanlines(); ++i)
        //    {
        //        memset(img.GetScanline(imgY) + imgX, 0, imgW);
        //        Ras.SweepScanline(i, img.GetScanline(imgY) + imgX + padX);

        //        // TO DO
        //        //filterScanline(img.GetScanline(imgY) + imgX, 
        //        //               img.GetScanline(imgY + padY + i) + imgX, imgW);
        //    }
        //    memset(img.GetScanline(imgY) + imgX, 0, imgW);
        //}

        // TO DO
        // SInt bias = 0;
        //if (gp.IsKnockOut())
        //    makeKnockOutCopy(img, imgX, imgY, imgW, imgH);

        //if (gp.BlurX || gp.BlurY)
        //{
        //    if (gp.IsFineBlur())
        //    {
        //        recursiveBlur(img, imgX, imgY, imgW, imgH, gp.GetBlurX(), gp.GetBlurY());
        //        bias = 8;
        //    }
        //    else
        //    {
        //        stackBlur(img, imgX, imgY, imgW, imgH, intBlurX, intBlurY);
        //        bias = 2;
        //    }
        //}

        //if (gp.BlurStrength != 16)
        //    strengthenImage(img, imgX, imgY, imgW, imgH, 
        //                    gp.GetBlurStrength(), 
        //                   (gp.GetBlurStrength() <= 1) ? 0 : bias);

        //if (gp.IsKnockOut())
        //    knockOut(img, imgX, imgY, imgW, imgH);
    }

//static int gln = 1;
//char buf[20];
//sprintf(buf, "%03d.pgm", gln);
//FILE*fd = fopen(buf, "wb");
//fprintf(fd, "P5\n%d %d\n255\n", imgW, imgH);
//for(int i = 0; i < imgH; ++i)
//fwrite(img.GetScanline(imgY+i) + imgX, imgW, 1, fd);
//fclose(fd);
//gln++;

// DBG simulate 4bpp textures
//for(UInt y = 0; y < imgH; ++y)
//    for(UInt x = 0; x < imgW; ++x)
//        img.GetScanline(y + imgY)[x + imgX] &= 0xF0;

    Ras.Clear();
    return node;
}





//-----------------------------------------------------------------------
GlyphNode* GlyphCache::RasterizeShadow(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp, float screenSize)
{
    SF_AMP_SCOPE_RENDER_TIMER("GlyphCache::RasterizeShadow");
//printf("S");
    const ShapeDataInterface* shape = GetGlyphShape(data, gp.GlyphIndex, gp.GetFontSize());
    if (shape == 0)
    {
        Result = Res_ShapeNotFound;
        return 0;
    }

    float k = gp.GetFontSize() / screenSize;
    float blurX = gp.GetBlurX() * k * data.HeightRatio;
    float blurY = gp.GetBlurY() * k * data.HeightRatio;

    float maxH = float(MaxSlotHeight - 2*SlotPadding);
    float rasterScale = 1;
    float vectorScale = gp.GetFontSize() / data.NomHeight;

    float y1 = data.GlyphBounds.y1 * vectorScale - blurY;
    float y2 = data.GlyphBounds.y2 * vectorScale + blurY;
    float h = y2 - y1;

    if (h >= maxH)
    {
        k = maxH / h;
        vectorScale *= k;
        blurX *= k;
        blurY *= k;
        rasterScale = 1/k;
    }

//if (h >= MaxSlotHeight)     // TO DO: Revise. Temporary solution for big glyphs
//{
//    scale *= float(MaxSlotHeight) / float(h+1);
//}

    int intBlurX = int(ceil(blurX));
    int intBlurY = int(ceil(blurY));

    GlyphNode* node = 0;

    // Rasterization
    Ras.Clear();
    addShapeToRasterizer(shape, vectorScale, vectorScale);

    int stretch  = 1;

    int padX = SlotPadding + intBlurX;
    int padY = SlotPadding + intBlurY;

    int imgX1 = -padX;
    int imgX2 =  padX;
    int imgY1 = -padY;
    int imgY2 =  padY;

    bool cellsToSweep = false;
    //if (raster) // TO DO
    //{
    //    imgX1 = -raster->OriginX - padX;
    //    imgY1 = -raster->OriginY - padY;
    //    imgX2 =  raster->Width   - 1 - raster->OriginX + padX;
    //    imgY2 =  raster->Height  - 1 - raster->OriginY + padY;
    //}
    //else
    {
        if (Ras.SortCells())
        {
            cellsToSweep = true;
            imgX1 = Ras.GetMinX() - padX;
            imgX2 = Ras.GetMaxX() + padX;
            imgY1 = Ras.GetMinY() - padY;
            imgY2 = Ras.GetMaxY() + padY;
        }
    }

    unsigned imgW  = imgX2 - imgX1 + 1;
    unsigned imgH  = imgY2 - imgY1 + 1;

    if (imgH > MaxSlotHeight)
        imgH = MaxSlotHeight;
    //SF_ASSERT(imgH <= MaxSlotHeight);

    node = allocateGlyph(tm, gp, imgW, imgH);
    if (node == 0)
    {
        Result = Res_CacheFull;
        return 0;
    }

    //UInt imgX, imgY;
    //if (!TexUpdPacker.Allocate(imgW, imgH, &imgX, &imgY))
    //{
    //    UpdateTextures(ren);
    //    if (!TexUpdPacker.Allocate(imgW, imgH, &imgX, &imgY))
    //        return 0;
    //}

    node->Origin.x = SInt16(imgX1 * 16);
    node->Origin.y = SInt16(imgY1 * 16);
    node->Scale = rasterScale;

    RasterData.Resize(imgW * imgH);
    RasterPitch = imgW;

    memset(&RasterData[0], 0, imgW * imgH);


    float gamma = 1.0f;
    if (gp.BlurX || gp.BlurY)
    {
        // Gamma correction improves the "strength" of glow and shadow
        // TO DO: Think of smarter control of it.
        gamma = 0.4f;
    }
    if (gamma != Ras.GetGamma())
        Ras.SetGamma(gamma);

    unsigned i;
    for(i = 0; i < Ras.GetNumScanlines() && (padY+i) < imgH; ++i)
    {
        UByte* sl = &RasterData[(padY+i)*RasterPitch + padX];
        Ras.SweepScanline(i, sl);
    }

    int bias = 0;
    int imgX = 0;
    int imgY = 0;
    //if (gp.IsKnockOut())
    //    makeKnockOutCopy(img, imgX, imgY, imgW, imgH);

    if (blurX > 0 || blurY > 0)
    {
        recursiveBlur(&RasterData[0], RasterPitch, imgX, imgY, imgW, imgH, blurX, blurY);
        bias = 8;
    }

    strengthenImage(&RasterData[0], RasterPitch, imgX, imgY, imgW, imgH, 
                    gp.GetBlurStrength(), (gp.GetBlurStrength() <= 1) ? 0 : bias);

    //if (gp.IsKnockOut())
    //    knockOut(img, imgX, imgY, imgW, imgH);

    UpdateTextureGlyph(GetTextureId(node), &RasterData[0], RasterPitch, 
        node->mRect.x, node->mRect.y, node->mRect.w, node->mRect.h); 

    Ras.Clear();
    return node;
}







}} // Scaleform::Render

