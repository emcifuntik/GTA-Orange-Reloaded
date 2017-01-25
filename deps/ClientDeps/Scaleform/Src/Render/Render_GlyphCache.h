/**********************************************************************

Filename    :   Render_GlyphCache.h
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

#ifndef INC_SF_Render_GlyphCache_H
#define INC_SF_Render_GlyphCache_H

#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayPaged.h"
#include "Kernel/SF_ArrayUnsafe.h"
#include "Kernel/SF_RefCount.h"
#include "Render_TextMeshProvider.h"
#include "Render_GlyphQueue.h"
#include "Render_GlyphCacheConfig.h"
#include "Render_TextLayout.h"
#include "Render_Image.h"
#include "Render_Rasterizer.h"

namespace Scaleform { namespace Render {

class HAL;
class TextureManager;
class GlyphCache;

//------------------------------------------------------------------------
struct RasterGlyphVertex
{
    float   x,y;
    UInt32  Clr;
    float   u,v;

    RasterGlyphVertex() {}
    RasterGlyphVertex(float x_, float y_, float u_, float v_, UInt32 color) :
        x(x_), y(y_), Clr(color), u(u_), v(v_)
    {}

    void    Set(float x_, float y_, float u_, float v_, UInt32 color)
    {
        x = x_;
        y = y_;
        u = u_;
        v = v_;
        Clr = color;
    }
    static VertexElement Elements[4];
    static VertexFormat  Format;
};

//------------------------------------------------------------------------
class GlyphTextureImage : public TextureImage
{
public:
    GlyphTextureImage(GlyphCache* cache, unsigned textureId, 
                      const ImageSize& size, unsigned use);

    static GlyphTextureImage* Create(MemoryHeap* heap, TextureManager* texMan, GlyphCache* cache, 
                                     unsigned textureId, const ImageSize& size, unsigned use);

    virtual void TextureLost(TextureLossReason reason);

private:
    GlyphCache*         pCache;
    unsigned            TextureId;
};

//------------------------------------------------------------------------
class GlyphTextureMapper
{
public:
    GlyphTextureMapper() : Valid(false), Method(0), pTexMan(0), pCache(0), TextureId(0), Mapped(false),
        NumGlyphsToUpdate(0) 
    {}

    bool Create(unsigned method, MemoryHeap* heap, HAL* ren, 
                TextureManager* texMan, PrimitiveFillManager* fillMan, GlyphCache* cache, 
                unsigned textureId, const ImageSize& size);

    ImagePlane* Map();
    bool        Unmap();
    bool        Update(const Texture::UpdateDesc* updates, unsigned count);

    Texture*    GetTexture();

    void        Invalidate()    { Valid = false; NumGlyphsToUpdate = 0; }
    bool        IsValid() const { return Valid; }

    const PrimitiveFill* GetFill() const { return pFill; }
          PrimitiveFill* GetFill()       { return pFill; }

    Image* GetImage() { return pRawImg.GetPtr() ? (Image*)pRawImg : (Image*)pTexImg; }

private:
    bool                    Valid;
    unsigned                Method; // See GlyphCache::TextureUpdateMethod
    TextureManager*         pTexMan;
    GlyphCache*             pCache;
    unsigned                TextureId;
    ImageData               Data;
    Ptr<RawImage>           pRawImg;
    Ptr<GlyphTextureImage>  pTexImg;
    Ptr<PrimitiveFill>      pFill;
    bool                    Mapped;
public:
    unsigned                NumGlyphsToUpdate;
};


//------------------------------------------------------------------------
class GlyphScanlineFilter
{
public:
    GlyphScanlineFilter(float prim, float second, float tert)
    {
        float norm = 1.0f / (prim + second*2 + tert*2);
        prim   *= norm;
        second *= norm;
        tert   *= norm;
        for(unsigned i = 0; i < 256; i++)
        {
            Primary[i]   = (UByte)floorf(prim   * i);
            Secondary[i] = (UByte)floorf(second * i);
            Tertiary[i]  = (UByte)floorf(tert   * i);
        }
    }

    void Filter(const UByte* src, UByte* dst) const
    {
        UByte v = src[2];
        dst[0] = UByte(dst[0] + Tertiary [v]);
        dst[1] = UByte(dst[1] + Secondary[v]);
        dst[2] = UByte(dst[2] + Primary  [v]);
        dst[3] = UByte(dst[3] + Secondary[v]);
        dst[4] = UByte(dst[4] + Tertiary [v]);
    }

private:
    UByte Primary[256];
    UByte Secondary[256];
    UByte Tertiary[256];
};


//------------------------------------------------------------------------
class TextureUpdatePacker
{
public:
    TextureUpdatePacker(unsigned width, unsigned height) : 
        Width(width), Height(height), 
        LastX(0), LastY(0), LastMaxHeight(0) {}


    unsigned GetWidth()  const { return Width; }
    unsigned GetHeight() const { return Height; }

    void Clear()
    {
        LastX = 0; 
        LastY = 0;
        LastMaxHeight = 0;
    }

    bool Allocate(unsigned w, unsigned h, unsigned* x, unsigned* y)
    {
        if (LastX + w <= Width && LastY + h <= Height)
        {
            *x = LastX;
            *y = LastY;
            LastX += w;
            if (h > LastMaxHeight)
                LastMaxHeight = h;
            return true;
        }

        LastY += LastMaxHeight;
        if (LastY + h <= Height)
        {
            *x = 0;
            *y = LastY;
            LastX = w;
            LastMaxHeight = h;
            return true;
        }
        return false;
    }

private:
    unsigned Width, Height, LastX, LastY, LastMaxHeight;
};






//------------------------------------------------------------------------
class GlyphCache : public RefCountBase<GlyphCache, Stat_Default_Mem>,
                   public CacheBase, public GlyphCacheConfig
{
    enum 
    {
        // TO DO: SID
        SID = Stat_Default_Mem
    };

    enum 
    {
        TexturePoolSize = 32
    };

    class EvictNotifier : public GlyphEvictNotifier
    {
    public:
        GlyphCache* pCache;
        virtual void Evict(TextMeshProvider* p) { pCache->EvictText(p); }
        virtual void ApplyInUseList()           { pCache->ApplyInUseList(); }
        virtual bool UpdatePinList()            { return pCache->UpdatePinList();}
    };

public:
    enum TextureUpdateMethod
    {
        TU_DirectMap,           // Can write directly to texture memory
        TU_MultipleUpdate,      // Can use multiple partial update
        TU_WholeImage           // Raw image copy in conventional memory is necessary
    };

    enum RasResult
    {
        Res_Success,
        Res_ShapeIsEmpty,
        Res_ShapeNotFound,
        Res_ShapeIsTooBig,
        Res_CacheFull
    };

    GlyphCache(MemoryHeap* heap);
    ~GlyphCache();

    void Initialize(HAL* ren, PrimitiveFillManager* fillMan);
    void Destroy();

    void OnBeginFrame();
    void OnEndFrame();

    bool CanReset() const { return TextInPin.IsEmpty(); }

    // *** GlyphCacheConfig interface
    virtual void ClearCache();
    virtual bool SetParams(const GlyphCacheParams& params);
    virtual const GlyphCacheParams& GetParams() const { return Param; }

    float GetScaleU() const { return ScaleU; }
    float GetScaleV() const { return ScaleV; }

    virtual void UnlockBuffers(); // CacheBase notification

    void TextureLost(unsigned textureId, unsigned reason);

    // Mesh cache and GlyphQueue interaction
    void CleanUpFont(FontCacheHandle* font);
    void MergeCacheSlots();
    void UnpinAllSlots();
    void EvictText(TextMeshProvider* b);
    void ApplyInUseList();
    bool UpdatePinList();
    void AddToInUseList(TextMeshProvider* b) { TextInUse.PushBack(b); }
    void PinSlot(GlyphSlot* s)               { Queue.PinSlot(s); }
    void UnpinSlot(GlyphSlot* s)             { Queue.UnpinSlot(s); }
    void RemoveNotifier(TextNotifier* n)     { Queue.RemoveNotifier(n); }

    int CmpFills(unsigned texId1, unsigned texId2) const
    {
        if (texId1 == texId2) return 0;
        const PrimitiveFill* f1 = Textures[texId1].GetFill();
        const PrimitiveFill* f2 = Textures[texId2].GetFill();
        return (f1 < f2) ? -1 : 1;
    }

    PrimitiveFill* GetFill(TextLayerType type, unsigned textureId);
    Image*         GetImage(unsigned textureId);


    // Glyph Cache allocation functions
    FontCacheHandle* RegisterFont(Font* font);
    float       GetCachedFontSize(const GlyphParam& gp, float screenSize, bool exactFit) const;
    float       GetCachedShadowSize(const GlyphParam& gp, float screenSize) const;
    GlyphNode*  FindGlyph(TextMeshProvider* tm, const GlyphParam& gp);
    GlyphNode*  RasterizeGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp);
    GlyphNode*  RasterizeShadow(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp, float screenSize);
    RasResult   GetRasResult() const { return Result; }
    UInt16      GetTextureId(const GlyphNode* node) const { return UInt16(node->pSlot->TextureId & ~GlyphSlot::Mask); }


    // TO DO: Possibly private
    bool UpdateTextureGlyph(unsigned textureId, const UByte* data, unsigned pitch, 
                            unsigned dstX, unsigned dstY, unsigned w, unsigned h);

    const ShapeDataInterface* GetGlyphShape(GlyphRunData& data, unsigned glyphIndex, float screenSize);
    float SnapFontSizeToRamp(float screenSize) const;
    float GetSharpeningFactor() const { return Param.SharpeningFactor; }
    bool  GlyphFits(unsigned h) const;

    void        PrintMemStats();
    void        VisitGlyphs(GlyphQueueVisitor* visitor) const { Queue.VisitGlyphs(visitor); }
    unsigned    GetNumRasterizedGlyphs() const { return Queue.GetNumPacked(); }
    unsigned    GetNumTextures() const;
    unsigned    ComputeUsedArea() const { return Queue.ComputeUsedArea(); }
    unsigned    ComputeTotalArea() const { return TextureWidth * TextureHeight * MaxNumTextures; }

private:
    void initialize();

    struct UpdateRect
    {
        unsigned SrcX, SrcY;
        unsigned DstX, DstY;
        unsigned w, h;
        unsigned TextureId;
    };

    typedef Texture::UpdateDesc UpdateDesc;

    GlyphNode* allocateGlyph(TextMeshProvider* tm, const GlyphParam& gp, unsigned w, unsigned h);
    GlyphNode* getPrerasterizedGlyph(GlyphRunData& data, TextMeshProvider* tm, const GlyphParam& gp);
    void partialUpdateTextures();
    void copyImageData(ImagePlane* pl, const UByte* data, unsigned pitch, 
                       unsigned dstX, unsigned dstY, unsigned w, unsigned h);
    void releaseAllTextures();
    void addShapeToRasterizer(const ShapeDataInterface* shape, float scaleX, float scaleY);


    void recursiveBlur(UByte* img, unsigned pitch, unsigned  sx, unsigned  sy, 
                       unsigned  w,  unsigned  h, float rx, float ry);

    void strengthenImage(UByte* img, unsigned pitch, unsigned  sx, unsigned  sy, 
                         unsigned  w,  unsigned  h, float ratio, int  bias);


private:
    GlyphCacheParams        Param;
    MemoryHeap*             pHeap;
    HAL*                    pRenderer;
    PrimitiveFillManager*   pFillMan;
    TextureManager*         pTexMan;
    unsigned                TextureWidth;
    unsigned                TextureHeight;
    unsigned                MaxNumTextures;
    unsigned                MaxSlotHeight;
    unsigned                SlotPadding;
    float                   ScaleU;
    float                   ScaleV;
    GlyphTextureMapper      Textures[TexturePoolSize];
    GlyphQueue              Queue;
    TextureUpdateMethod     Method;

    TextureUpdatePacker     UpdatePacker;
    Ptr<RawImage>           UpdateBuffer;
    ArrayPagedLH_POD<UpdateRect, 6, 16, SID>    GlyphsToUpdate;
    ArrayUnsafeLH_POD<UpdateDesc, SID>          RectsToUpdate;
    List<TextMeshProvider>                      TextInUse;
    List<TextMeshProvider>                      TextInPin;
    EvictNotifier                               Notifier;
    Ptr<FontCacheHandleManager>                 pFontHandleManager;
    RQCacheInterface*                           pRQCaches;

    Ptr<PrimitiveFill> pSolidFill;
    Ptr<PrimitiveFill> pMaskFill;

    // TO DO: Implement vector cache
    Ptr<GlyphShape> pGlyphShape;
    UByte           FontSizeMap[256];
    RasResult       Result;

    Rasterizer      Ras;
    ArrayLH_POD<UByte, SID> RasterData;
    unsigned                RasterPitch;
    ArrayLH_POD<UByte, SID> BlurStack;
    ArrayLH_POD<float, SID> BlurSum;
    GlyphRaster             Raster;
};







}} // Scaleform::Render

#endif

