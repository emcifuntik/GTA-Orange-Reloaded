/**********************************************************************

Filename    :   FontGlyphPacker.h
Content     :   FontGlyphPacker implementation declaration
Created     :   6/14/2007
Authors     :   Maxim Shemanarev, Artyom Bolgar

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_FontGlyphPacker_H
#define INC_SF_GFX_FontGlyphPacker_H

#include "Kernel/SF_Array.h"
#include "Render/Render_Types2D.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_FontResource.h"

#ifdef SF_RENDER_ENABLE_GLYPH_PACKER
#include "Render/Render_CompoundShape.h"
#include "Render/Render_RectPacker.h"
#include "Render/Render_Rasterizer.h"
#include "GFx/GFx_Shape.h"

namespace Scaleform { namespace GFx {

//------------------------------------------------------------------------
class FontGlyphPacker : public RefCountBaseNTS<FontGlyphPacker, Stat_Default_Mem>
{
    // This is for keeping track of our rendered glyphs, before
    // packing them into textures and registering with the FontResource.
    struct GlyphInfo
    {
        FontResource*    pFont;
        unsigned GlyphIndex;
        unsigned GlyphReuse;
        unsigned TextureIdx;
        RectF   Bounds;
        Render::PointF  Origin;
    };

    struct GlyphGeometryKey
    {
        const FontResource*      pFont;
        const ShapeDataBase*         pShape;
        UInt32                      Hash;

        GlyphGeometryKey(): pFont(0), pShape(0), Hash(0) {}
        GlyphGeometryKey(const FontResource* font, 
                         const ShapeDataBase* shape, 
                         UInt32 hash): 
            pFont(font), pShape(shape), Hash(hash) {}
        UPInt operator()(const GlyphGeometryKey& data) const
        {
            return (UPInt)data.Hash ^ 
                  ((UPInt)data.pFont >> 6) ^ 
                   (UPInt)data.pFont;
        }
        bool operator== (const GlyphGeometryKey& cmpWith) const 
        { 
            return pFont == cmpWith.pFont && 
                   pShape->IsEqualGeometry(*cmpWith.pShape);
        }
    };

public:
    FontGlyphPacker(FontPackParams* params,
                       ImageCreator *pimageCreator,
                       RenderConfig *prenderConfig,
                       Log* plog,
                       ResourceId* ptextureIdGen,
                       MemoryHeap* fontHeap,
                       bool threadedLoading);
   ~FontGlyphPacker();

    void GenerateFontBitmaps(const Array<FontResource*>& fonts);


private:
    // Prohibit Copying
    FontGlyphPacker(const FontGlyphPacker&);
    const FontGlyphPacker& operator = (const FontGlyphPacker&);

    void generateGlyphInfo(Array<GlyphInfo>* glyphs, FontResource* f);
    unsigned packGlyphRects(Array<GlyphInfo>* glyphs, 
                        unsigned start, unsigned end, unsigned texIdx);
    unsigned packGlyphRects(Array<GlyphInfo>* glyphs);
    void rasterizeGlyph(Render::Image* texImage, GlyphInfo* gi);
    void generateTextures(Array<GlyphInfo>* glyphs, unsigned numTextures);

    typedef HashLH<GlyphGeometryKey, unsigned, GlyphGeometryKey,  Stat_Default_Mem> GlyphGeometryHashType;

    FontPackParams*                  pFontPackParams;
    // Texture glyph configuration cached from FontPackParams.
    FontPackParams::TextureConfig    PackTextureConfig;

    // Texture id generator, specified externally.
    ResourceId*                      pTextureIdGen;
    Ptr<ImageCreator>               pImageCreator;
    Ptr<RenderConfig>               pRenderConfig; // Passed for in CreateImage.
    Ptr<Log>                        pLog;
    MemoryHeap*                        pFontHeap;

    Render::RectPacker                         Packer;
    GRasterizer                         Rasterizer;
    GCompoundShape                      CompoundShape;
    GlyphGeometryHashType               GlyphGeometryHash;
    bool                                ThreadedLoading;
};




// Builds cached glyph textures from shape info.    

// extern void GFx_GenerateFontBitmaps(FontPackParams *params,
//                                     const Array<FontResource*>& fonts,
//                                     ImageCreator *pimageCreator,
//                                     RenderConfig *prenderConfig,
//                                     Log* plog,
//                                     ResourceId* pidGenerator,
//                                     MemoryHeap* fontHeap,
//                                     bool threadedLoading);


}} // namespace Scaleform { namespace GFx {
#endif // SF_NO_FONT_GLYPH_PACKER
#endif // INC_SF_GFX_FONTLIBIMPL_H
