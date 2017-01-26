/**********************************************************************

Filename    :   FontGlyphPacker.cpp
Content     :   FontGlyphPacker implementation
Created     :   6/14/2007
Authors     :   Maxim Shemanarev, Artyom Bolgar

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/GFx_FontGlyphPacker.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace GFx {

#ifdef SF_RENDER_ENABLE_GLYPH_PACKER

// The font glyphs are cached into textures; these textures are rendered 
// in software using shape rasterization with anti-aliasing. A packing
// algorithm is used to pack individual glyph images into textures.

//------------------------------------------------------------------------
FontGlyphPacker::FontGlyphPacker(FontPackParams* params,
                                 ImageCreator *pimageCreator,
                                 RenderConfig *prenderConfig,
                                 Log* plog,
                                 ResourceId* ptextureIdGen,
                                 MemoryHeap* fontHeap,
                                 bool threadedLoading):
    pFontPackParams(params),
    pTextureIdGen(ptextureIdGen),
    pImageCreator(pimageCreator),
    pRenderConfig(prenderConfig),
    pLog(plog),
    pFontHeap(fontHeap),
    ThreadedLoading(threadedLoading)
{
    if (pFontPackParams)
        pFontPackParams->GetTextureConfig(&PackTextureConfig);
    Packer.SetWidth (PackTextureConfig.TextureWidth);
    Packer.SetHeight(PackTextureConfig.TextureHeight);
}

//------------------------------------------------------------------------
FontGlyphPacker::~FontGlyphPacker()
{
}

//------------------------------------------------------------------------
void FontGlyphPacker::generateGlyphInfo(Array<GlyphInfo>* glyphs, 
                                        FontResource* font)
{
    SF_ASSERT(glyphs != 0 && font != 0);

    if (font->GetGlyphShapeCount() == 0)
        return;

    // The font must not have texture glyph data yet; if it does, as might happen
    // for loaded pre-generated textures, it is filtered out by our caller function.       
    SF_ASSERT(font->GetTextureGlyphData() == 0);

    Ptr<TextureGlyphData> ptextureGlyphData = 
        *SF_HEAP_NEW(pFontHeap) TextureGlyphData(font->GetGlyphShapeCount());

    ptextureGlyphData->SetTextureConfig(PackTextureConfig);
    font->SetTextureGlyphData(ptextureGlyphData);

    float    glyphToPix = PackTextureConfig.NominalSize / FontPackParams::GlyphBoundBox;
    unsigned i;
    unsigned n = font->GetGlyphShapeCount();

    for (i = 0; i < n; i++)
    {
        const TextureGlyph& tg = ptextureGlyphData->GetTextureGlyph(i);

        // If image has not yet been initialized
        if (!tg.HasImageResource())
        {
            Ptr<ShapeDataBase>   sh = *font->GetGlyphShape(i, 0);
            if (sh)
            {
                RectF  glyphBounds;
                sh->ComputeBound(&glyphBounds);

                if (glyphBounds.Width() > 0 && glyphBounds.Height() > 0)
                {
                    // Add a glyph.
                    GlyphInfo gi;
                    gi.Bounds.x1   = glyphBounds.x1   * glyphToPix - PackTextureConfig.PadPixels;
                    gi.Bounds.y1    = glyphBounds.y1    * glyphToPix - PackTextureConfig.PadPixels;
                    gi.Bounds.x2  = glyphBounds.x2  * glyphToPix + PackTextureConfig.PadPixels;
                    gi.Bounds.y2 = glyphBounds.y2 * glyphToPix + PackTextureConfig.PadPixels;
                    gi.Origin.x      = 0;
                    gi.Origin.y      = 0;

                    if (gi.Bounds.Width() > 0 && gi.Bounds.Height() > 0)
                    {
                        gi.pFont       = font;
                        gi.GlyphIndex  = i;
                        gi.GlyphReuse  = ~0U;  
                        gi.TextureIdx  = ~0U;

                        // Try to reuse the glyph
                        GlyphGeometryKey key(font, sh, sh->ComputeGeometryHash());
                        const unsigned*      val = GlyphGeometryHash.Get(key);
                        if (val)
                        {
                            gi.GlyphReuse = *val;
                        }
                        else
                        {
                            unsigned glyphIdx = (unsigned)glyphs->GetSize();
                            GlyphGeometryHash.Add(key, glyphIdx);
                        }
                        glyphs->PushBack(gi);
                    }
                }
            }
        }
    }
}

//------------------------------------------------------------------------
unsigned FontGlyphPacker::packGlyphRects(Array<GlyphInfo>* glyphs, 
                                         unsigned start, unsigned end, unsigned texIdx)
{
    Packer.Clear();
    unsigned i, j, w, h;
    for (i = start; i < end; i++)
    {
        const GlyphInfo& gi = (*glyphs)[i];
        if(gi.GlyphReuse == ~0U)
        {
            w = int(ceilf(gi.Bounds.x2)  - floorf(gi.Bounds.x1));
            h = int(ceilf(gi.Bounds.y2) - floorf(gi.Bounds.y1));
            Packer.AddRect(w, h, i);
        }
    }
    Packer.Pack();
    for (i = 0; i < Packer.GetNumPacks(); i++)
    {
        const Render::RectPacker::PackType& pack = Packer.GetPack(i);
        for(j = 0; j < pack.NumRects; j++)
        {
            const Render::RectPacker::RectType& rect = Packer.GetRect(pack, j);
            GlyphInfo& gi = (*glyphs)[rect.Id];
            w = unsigned(ceilf(gi.Bounds.x2)  - floorf(gi.Bounds.x1));
            h = unsigned(ceilf(gi.Bounds.y2) - floorf(gi.Bounds.y1));
            gi.Origin.x      = float(rect.x) - gi.Bounds.x1;
            gi.Origin.y      = float(rect.y) - gi.Bounds.y1;
            gi.Bounds.x1   = float(rect.x);
            gi.Bounds.y1    = float(rect.y);
            gi.Bounds.x2  = float(rect.x + w);
            gi.Bounds.y2 = float(rect.y + h);
            gi.TextureIdx = texIdx + i;
        }
    }
    return (unsigned)(texIdx + Packer.GetNumPacks());
}

//------------------------------------------------------------------------
unsigned FontGlyphPacker::packGlyphRects(Array<GlyphInfo>* glyphs)
{
    unsigned numTextures = 0;
    if(pFontPackParams->GetUseSeparateTextures())
    {
        unsigned i;
        unsigned start = 0;
        for (i = 1; i < glyphs->GetSize(); i++)
        {
            if ((*glyphs)[i-1].pFont != (*glyphs)[i].pFont)
            {
                numTextures = packGlyphRects(glyphs, start, i, numTextures);
                start = i;
            }
        }
        numTextures = packGlyphRects(glyphs, start, (unsigned)glyphs->GetSize(), numTextures);
    }
    else
    {
        numTextures = packGlyphRects(glyphs, 0, (unsigned)glyphs->GetSize(), 0);
    }
    return numTextures;
}

//------------------------------------------------------------------------
void FontGlyphPacker::rasterizeGlyph(Render::Image* texImage, GlyphInfo* gi)
{
    Ptr<ShapeDataBase> sh = *gi->pFont->GetGlyphShape(gi->GlyphIndex, 0);
    if (sh)
    {
        sh->MakeCompoundShape(&CompoundShape, 
            FontPackParams::GlyphBoundBox / PackTextureConfig.NominalSize * 0.5f);

        Rasterizer.Clear();
        Rasterizer.AddShape(CompoundShape, 
                            PackTextureConfig.NominalSize / 
                            FontPackParams::GlyphBoundBox);

        if(Rasterizer.SortCells()) // If there is anything to sweep...
        {
            // Sweep the raster writing the scan lines to the image.
            unsigned h = Rasterizer.GetMaxY() - Rasterizer.GetMinY() + 1;
            int  x = int(floorf(gi->Bounds.x1)) + PackTextureConfig.PadPixels;
            int  y = int(floorf(gi->Bounds.y1))  + PackTextureConfig.PadPixels;
            for (unsigned i = 0; i < h; i++)
            {
                Rasterizer.SweepScanline(i, texImage->GetScanline(y+i) + x);
            }
        }
    }
}

//------------------------------------------------------------------------
void FontGlyphPacker::generateTextures(Array<GlyphInfo>* glyphs, unsigned numTextures)
{
    unsigned i, j;

    for (i = 0; i < numTextures; i++)
    {
        unsigned maxWidth  = 0;
        unsigned maxHeight = 0;
        for(j = 0; j < glyphs->GetSize(); j++)
        {
            const GlyphInfo& gi = (*glyphs)[j];
            if (gi.TextureIdx == i)
            {
                if (int(ceilf(gi.Bounds.x2)) > (int)maxWidth)
                    maxWidth = int(ceilf(gi.Bounds.x2));

                if (int(ceilf(gi.Bounds.y2)) > (int)maxHeight)
                    maxHeight = int(ceilf(gi.Bounds.y2));
            }
        }
    
        unsigned texWidth  = PackTextureConfig.TextureWidth;
        unsigned texHeight = PackTextureConfig.TextureHeight;
        if (maxWidth <= (unsigned)PackTextureConfig.TextureWidth / 2)
        {
            texWidth = 1;
            while (texWidth < maxWidth) texWidth <<= 1;
        }
        if (maxHeight <= (unsigned)PackTextureConfig.TextureHeight / 2)
        {
            texHeight = 1;
            while (texHeight < maxHeight) texHeight <<= 1;
        }

        // TBD: Future improvements: create Render::Image in the temp global heap
        //      in case it is supposed to be destroyed after pImageCreator->CreateImage().
        //      It may occur when the renderer never loses textures.
        Ptr<Render::Image> texImage = 
            *SF_HEAP_NEW(pFontHeap) Render::Image(Render::Image::Image_A_8, texWidth, texHeight);

        memset(texImage->pData, 0, texWidth * texHeight);

        for(j = 0; j < glyphs->GetSize(); j++)
        {
            GlyphInfo& gi = (*glyphs)[j];
            if (gi.TextureIdx == i)
            {
                rasterizeGlyph(texImage, &gi);
            }
        }

        float pixToU = 1.0f / texImage->Width;
        float pixToV = 1.0f / texImage->Height;
        ResourceId          textureId = pTextureIdGen->GenerateNextId();
        ImageCreateInfo     icreateInfo(texImage, Resource::Use_FontTexture);
        icreateInfo.SetStates(0, pRenderConfig, pLog, NULL,NULL);
        icreateInfo.ThreadedLoading = ThreadedLoading;
        icreateInfo.pHeap           = pFontHeap;
        Ptr<ImageInfoBase>   pimageInfo = *pImageCreator->CreateImage(icreateInfo);
        Ptr<ImageResource> pimageRes  = 
            *SF_HEAP_NEW(pFontHeap) ImageResource(pimageInfo.GetPtr(), 
                                                   Resource::Use_FontTexture);
        for(j = 0; j < glyphs->GetSize(); j++)
        {
            GlyphInfo gi = (*glyphs)[j];
            if (gi.GlyphReuse != ~0U)
            {
                // If the glyph is reused switch to the original.
                const GlyphInfo& gi2 = (*glyphs)[gi.GlyphReuse];
                SF_ASSERT(gi.pFont == gi2.pFont); // Font must be the same
                gi.TextureIdx = gi2.TextureIdx;
                gi.Bounds     = gi2.Bounds;
                gi.Origin     = gi2.Origin;
            }
            if (gi.TextureIdx == i)
            {
                Ptr<TextureGlyph> tg = *SF_HEAP_NEW(pFontHeap) TextureGlyph();
                tg->pImage          = pimageRes;
                tg->UvBounds.x1     = gi.Bounds.x1   * pixToU;
                tg->UvBounds.y1     = gi.Bounds.y1    * pixToV;
                tg->UvBounds.x2     = gi.Bounds.x2  * pixToU;
                tg->UvBounds.y2     = gi.Bounds.y2 * pixToV;
                tg->UvOrigin.x      = gi.Origin.x      * pixToU;
                tg->UvOrigin.y      = gi.Origin.y      * pixToV;

                TextureGlyphData* tgd = gi.pFont->GetTextureGlyphData();
                tg->SetImageResource(pimageRes.GetPtr());

                // The texture is only added to font data.
                // This means that VisitFonts will need to collect fonts,
                // matching them through a hash-table if necessary.
                tgd->AddTextureGlyph(gi.GlyphIndex, *tg);

                // add textureId -> texture assoc into the font
                tgd->AddTexture(textureId, pimageRes);
            }
        }
    }
}

//#include <time.h>

//------------------------------------------------------------------------
void FontGlyphPacker::GenerateFontBitmaps(const Array<FontResource*>& fonts)
{
//clock_t cl = clock();
    unsigned i;
    unsigned totalNumGlyphs = 0;
    for (i = 0; i < fonts.GetSize(); i++)
    {
        if (fonts[i]->GetTextureGlyphData() != 0)
            continue;

        if (pFontPackParams->GetGlyphCountLimit() && 
            (int)fonts[i]->GetGlyphShapeCount() > pFontPackParams->GetGlyphCountLimit())
            continue;

        totalNumGlyphs += fonts[i]->GetGlyphShapeCount();
    }

    Array<GlyphInfo> glyphInfo;
    glyphInfo.Reserve(totalNumGlyphs);
    GlyphGeometryHash.Clear();
    for (i = 0; i < fonts.GetSize(); i++)
    {
        if (fonts[i]->GetTextureGlyphData() != 0)
            continue;

        if (pFontPackParams->GetGlyphCountLimit() && 
            (int)fonts[i]->GetGlyphShapeCount() > pFontPackParams->GetGlyphCountLimit())
            continue;

        generateGlyphInfo(&glyphInfo, fonts[i]);
    }

    unsigned numTextures = packGlyphRects(&glyphInfo);
    generateTextures(&glyphInfo, numTextures);
//printf("%d %f\n", glyphInfo.GetSize(), double(clock() - cl) / CLOCKS_PER_SEC);
}

#endif //SF_NO_FONT_GLYPH_PACKER






///////////////////////////////////////////////////////////
//
// ***** FontPackParams
//


// Size (in TWIPS) of the box that the glyph should stay within.
// this *should* be 1024, but some glyphs in some fonts exceed it!
//------------------------------------------------------------------------
const float FontPackParams::GlyphBoundBox = 1536.0f;


//------------------------------------------------------------------------
void FontPackParams::SetTextureConfig(const TextureConfig& config)
{
    PackTextureConfig = config;

    const int    minSize = 4;
    const int    maxSize = PackTextureConfig.TextureHeight / 2;

    if (PackTextureConfig.NominalSize < minSize)
    {
        SF_DEBUG_WARNING2(1, "SetTextureConfig - NominalSize (%d) too small, clamping to %d\n",
                              PackTextureConfig.NominalSize, minSize);
        PackTextureConfig.NominalSize = minSize;
    }
    else if (PackTextureConfig.NominalSize > maxSize)
    {
        SF_DEBUG_WARNING2(1, "SetTextureConfig - NominalSize (%d) too large, clamping to %d\n",
                              PackTextureConfig.NominalSize, maxSize);
        PackTextureConfig.NominalSize = maxSize;
    }

    if (config.PadPixels < 1)
    {
        SF_DEBUG_WARNING2(1, "SetTextureConfig - PadPixels (%d) too small, clamping to %d\n",
                              PackTextureConfig.PadPixels, 1);
        PackTextureConfig.PadPixels = minSize;
    }
}


//------------------------------------------------------------------------
#ifdef SF_ASSERT_ON_FONT_BITMAP_GEN
#define SF_ASSERT_ON_FONT_BITMAP_GEN SF_ASSERT(0)
#else
#define SF_ASSERT_ON_FONT_BITMAP_GEN ((void)0)
#endif //SF_ASSERT_ON_FONT_BITMAP_GEN



// Build cached textures from glyph outlines.
//------------------------------------------------------------------------
void    GFx_GenerateFontBitmaps(FontPackParams *params,
                                const Array<FontResource*>& fonts,
                                ImageCreator *pimageCreator,
                                Log* plog,
                                ResourceId* pidGenerator,
                                MemoryHeap* fontHeap,
                                bool threadedLoading)
{
    if (!params)
    {
        // User should not be calling us with no font params. Null params are checked
        // by caller during movieDef binding, ensuring that correct warning can be issued
        // in case when there is also no dynamic cache.
        return;
    }
    if (!pimageCreator)
    {
        SF_DEBUG_WARNING(1, "Bitmap font texture gen failed - ImageCreator not installed");
        return;
    }

    SF_ASSERT(pidGenerator != 0);

  //if (powner->IsRenderingFonts())
    {
        SF_ASSERT_ON_FONT_BITMAP_GEN;
#ifdef SF_RENDER_ENABLE_GLYPH_PACKER
        Ptr<FontGlyphPacker> packer = 
            *SF_NEW FontGlyphPacker(params, pimageCreator, prenderConfig,
                                     plog, pidGenerator, fontHeap, threadedLoading);
        packer->GenerateFontBitmaps(fonts);
#else
        SF_UNUSED3(fonts, plog, pidGenerator);        
        SF_UNUSED2(threadedLoading, fontHeap);        
#endif//SF_NO_FONT_GLYPH_PACKER
    }
}

//------------------------------------------------------------------------
float   FontPackParams::GetDrawGlyphScale(int nominalGlyphHeight)
{
    // Scale from uv coords to the 1024x1024 glyph square.
    // This equation used to be in FontLibImpl::DrawGlyph
    return PackTextureConfig.TextureHeight * FontPackParams::GlyphBoundBox / nominalGlyphHeight;
}

//------------------------------------------------------------------------
float   FontPackParams::GetTextureGlyphMaxHeight(const FontResource* f)
{
    TextureConfig conf;
    f->GetTextureConfig(&conf);
    return 1024.0f / FontPackParams::GlyphBoundBox * conf.NominalSize; 
}
}} //namespace Scaleform { namespace GFx {

