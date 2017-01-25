/**********************************************************************

Filename    :   GFxFontProviderPS3.cpp
Content     :   FreeType2 font provider
Created     :   6/21/2007
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

----------------------------------------------------------------------
The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform. 
Permission to use without restrictions is hereby granted to 
Scaleform Corporation by the author of Anti-Grain Geometry Project.
See http://antigtain.com for details.
**********************************************************************/

#include <cell/sysmodule.h>
#include "GFx/GFx_FontProviderPS3.h"
#include "GFx/GFx_String.h"
#include "GFx/GFx_Shape.h"
#include "Render/Render_Math2D.h"

#define PS3_CURVE_TAG(flag) (flag & 3)
#define PS3_CURVE_TAG_ON     1
#define PS3_CURVE_TAG_CONIC  0
#define PS3_CURVE_TAG_CUBIC  2

namespace Scaleform { 
namespace GFx
{

//------------------------------------------------------------------------
bool GFxExternalFontPS3::decomposeGlyphOutline(const CellFontGlyph* glyph, 
                                               ShapeNoStyles* shape)
{   
    struct PointType { float x, y; };
    const CellFontGlyphOutline& outline = glyph->Outline;
    PointType v_last;
    PointType v_control;
    PointType v_start;

    PathPacker path;
    float ratio = (shape->GetHintedGlyphSize() > 0) ? 20.0f : 1.0f;

    const PointType*  point;
    const PointType*  limit;
    const uint8_t*    tags;

    int  n;         // index of contour in outline
    int  first = 0; // index of first point in contour
    char tag;       // current point's state
    bool shapeValid = false;

    for(n = 0; n < outline.contoursCount; n++)
    {
        int  last;  // index of last point in contour

        last  = outline.contourIndexs[n];
        limit = (const PointType*)outline.Points + last;

        v_start.x = outline.Points[first].x;
        v_start.y = outline.Points[first].y;
        v_last.x  = outline.Points[last].x;
        v_last.y  = outline.Points[last].y;

        v_control = v_start;

        point = (const PointType*)outline.Points + first;
        tags  = outline.pointTags  + first;
        tag   = PS3_CURVE_TAG(tags[0]);

        // A contour cannot start with a cubic control point!
        if(tag == PS3_CURVE_TAG_CUBIC) return false;

        // check first point to determine origin
        if( tag == PS3_CURVE_TAG_CONIC)
        {
            // first point is conic control.  Yes, this happens.
            if(PS3_CURVE_TAG(outline.pointTags[last]) == PS3_CURVE_TAG_ON)
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = (v_start.x + v_last.x) / 2;
                v_start.y = (v_start.y + v_last.y) / 2;
                v_last = v_start;
            }
            point--;
            tags--;
        }

        path.Reset();
        path.SetFill0(1);
        path.SetFill1(0);
        path.SetMoveTo(int(ratio * v_start.x), -int(ratio * v_start.y));

        while(point < limit)
        {
            point++;
            tags++;

            tag = PS3_CURVE_TAG(tags[0]);
            switch(tag)
            {
                case PS3_CURVE_TAG_ON:  // emit a single line_to
                {
                    path.LineToAbs(int(ratio * point->x), -int(ratio * point->y));
                    continue;
                }

                case PS3_CURVE_TAG_CONIC:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

                Do_Conic:
                    if(point < limit)
                    {
                        PointType vec;
                        PointType v_middle;

                        point++;
                        tags++;
                        tag = PS3_CURVE_TAG(tags[0]);

                        vec.x = point->x;
                        vec.y = point->y;

                        if(tag == PS3_CURVE_TAG_ON)
                        {
                            path.CurveToAbs(int(ratio * v_control.x), -int(ratio * v_control.y),
                                            int(ratio * vec.x),       -int(ratio * vec.y));
                            continue;
                        }

                        if(tag != PS3_CURVE_TAG_CONIC) return false;

                        v_middle.x = (v_control.x + vec.x) / 2;
                        v_middle.y = (v_control.y + vec.y) / 2;
                        path.CurveToAbs(int(ratio * v_control.x), -int(ratio * v_control.y),
                                        int(ratio * v_middle.x),  -int(ratio * v_middle.y));
                        v_control = vec;
                        goto Do_Conic;
                    }
                    path.CurveToAbs(int(ratio * v_control.x), -int(ratio * v_control.y),
                                    int(ratio * v_start.x),   -int(ratio * v_start.y));
                    goto Close;
                }

                default:  // PS3_CURVE_TAG_CUBIC
                {
                    PointType vec1, vec2;
                    if(point + 1 > limit || PS3_CURVE_TAG(tags[1]) != PS3_CURVE_TAG_CUBIC)
                    {
                        return false;
                    }

                    vec1 = point[0];
                    vec2 = point[1];

                    point += 2;
                    tags  += 2;

                    if(point <= limit)
                    {
                        PointType vec;
                        vec = *point;
                        path.CurveToAbs(int(ratio * (vec1.x + vec2.x) / 2),
                                       -int(ratio * (vec1.y + vec2.y) / 2),
                                        int(ratio * vec.x), -int(ratio * vec.y));
                        continue;
                    }
                    path.CurveToAbs(int(ratio * (vec1.x + vec2.x) / 2),
                                   -int(ratio * (vec1.y + vec2.y) / 2),
                                    int(ratio * v_start.x), -int(ratio * v_start.y));
                    goto Close;
                }
            }
        }

    Close:
        if (!path.IsEmpty())
        {
            path.ClosePath();
            shape->AddPath(&path);
            shapeValid = true;
        }
        first = last + 1; 
    }
    return shapeValid;
}

/*
//------------------------------------------------------------------------
class GFxBitsetIterator
{
public:
    GFxBitsetIterator(const UByte* bits, unsigned offset = 0) :
        Bits(bits + (offset >> 3)),
        Mask(UByte(0x80 >> (offset & 7)))
    {}

    void operator ++ ()
    {
        Mask >>= 1;
        if(Mask == 0)
        {
            ++Bits;
            Mask = 0x80;
        }
    }

    unsigned GetBit() const
    {
        return (*Bits) & Mask;
    }

private:
    const UByte* Bits;
    UByte        Mask;
};

//------------------------------------------------------------------------
void GFxExternalFontPS3::decomposeGlyphBitmap(const FT_Bitmap& bitmap,
                                              int x, int y,
                                              GFxGlyphRaster* raster)
{
    raster->Width   = bitmap.width;
    raster->Height  = bitmap.rows;
    raster->OriginX = -x;
    raster->OriginY =  y;
    raster->Raster.resize(raster->Width * raster->Height);

    int i;
    int pitch = bitmap.pitch;

    const UByte* src = (const UByte*)bitmap.buffer;
          UByte* dst = &raster->Raster[0];
    for(i = 0; i < bitmap.rows; i++)
    {
        GFxBitsetIterator bits(src, 0);
        int j;
        for(j = 0; j < bitmap.width; j++)
        {
            *dst++ = bits.GetBit() ? 255 : 0;
            ++bits;
        }
        src += pitch;
    }
}
*/



//------------------------------------------------------------------------
GFxExternalFontPS3::~GFxExternalFontPS3()
{
    if (FontOK == CELL_OK)
        cellFontCloseFont(&Font);
}

//------------------------------------------------------------------------
GFxExternalFontPS3::GFxExternalFontPS3(GFxFontProviderPS3* pprovider, 
                                       const CellFontLibrary* library, 
                                       GFxFontInfoPS3* fontInfo) :
    Font(fontInfo->FontFlags),
    pFontProvider(pprovider),
    Name(fontInfo->FontName),
    LastFontHeight(FontHeight),
    RasterHintingRange(fontInfo->RasterHintingRange),
    VectorHintingRange(fontInfo->VectorHintingRange),
    MaxRasterHintedSize(fontInfo->MaxRasterHintedSize),
    MaxVectorHintedSize(fontInfo->MaxVectorHintedSize),
    FontOK(-1)
{
    if (fontInfo->FontData)
    {
        FontOK = cellFontOpenFontMemory(
            library, 
            (void*)fontInfo->FontData,
            fontInfo->FontDataSize,
            fontInfo->SubNum,
            fontInfo->UniqueId,
            &Font);
    }
    else
    if (!fontInfo->FileName.IsEmpty())
    {
        FontOK = cellFontOpenFontFile(
            library, 
            (uint8_t*)(fontInfo->FileName.ToCStr()),
            fontInfo->SubNum,
            fontInfo->UniqueId,
            &Font);
    }
    else
    {
        FontOK = cellFontOpenFontset(library, &fontInfo->Type, &Font);
    }

    if(FontOK == CELL_OK)
        setFontMetrics();
}

//------------------------------------------------------------------------
void GFxExternalFontPS3::setFontMetrics()
{
    cellFontSetScalePixel(&Font, FontHeight, FontHeight);
    LastFontHeight = FontHeight;

    CellFontHorizontalLayout layout;
    cellFontGetHorizontalLayout(&Font, &layout);
    SetFontMetrics(0, layout.baseLineY, layout.lineHeight-layout.baseLineY);
    
//    Float ascent  =  Float(Face->ascender)  * FontHeight / Face->units_per_EM;
//    Float descent = -Float(Face->descender) * FontHeight / Face->units_per_EM;
//    Float height  =  Float(Face->height)    * FontHeight / Face->units_per_EM;
//    SetFontMetrics(height - ascent + descent, ascent, descent);
}


//------------------------------------------------------------------------
int GFxExternalFontPS3::GetGlyphIndex(UInt16 code) const
{
    if (FontOK == CELL_OK)
    {
        const UInt* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        GFxExternalFontPS3* pthis = const_cast<GFxExternalFontPS3*>(this);
        if (LastFontHeight != FontHeight)
        {
            // cellFontSetScalePixel is expensive. Avoid calling it often.
            cellFontSetScalePixel(&pthis->Font, FontHeight, FontHeight);
            pthis->LastFontHeight = FontHeight;
        }
        CellFontGlyphMetrics metrics;
        int err = cellFontGetCharGlyphMetrics(&pthis->Font, code, &metrics);
        if (err != CELL_OK)
            return -1;

        GlyphType glyph;
        glyph.Code          =  code;
        glyph.Advance       =  metrics.Horizontal.advance;
        glyph.Bounds.Left   =  metrics.Horizontal.bearingX;
        glyph.Bounds.Top    = -metrics.Horizontal.bearingY;
        glyph.Bounds.Right  =  glyph.Bounds.Left + metrics.width;
        glyph.Bounds.Bottom =  glyph.Bounds.Top  + metrics.height;

//        glyph.Bounds.Left   =  Float(Face->glyph->metrics.horiBearingX >> 6);
//        glyph.Bounds.Top    = -Float(Face->glyph->metrics.horiBearingY >> 6);
//        glyph.Bounds.Right  =  Float(Face->glyph->metrics.width  >> 6) + glyph.Bounds.Left;
//        glyph.Bounds.Bottom =  Float(Face->glyph->metrics.height >> 6) + glyph.Bounds.Top;

        pthis->Glyphs.PushBack(glyph);
        pthis->CodeTable.Add(code, (UInt)Glyphs.GetSize()-1);
        return (UInt)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool GFxExternalFontPS3::IsHintedVectorGlyph(UInt glyphIndex, UInt glyphSize) const
{
    if (glyphIndex == (UInt)-1 ||
        glyphSize > MaxVectorHintedSize ||
        VectorHintingRange == DontHint)
    {
        return false;
    }

    if (VectorHintingRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool GFxExternalFontPS3::IsHintedRasterGlyph(UInt glyphIndex, UInt glyphSize) const
{
    if (glyphIndex == (UInt)-1 ||
        glyphSize > MaxRasterHintedSize ||
        RasterHintingRange == DontHint)
    {
        return false;
    }

    if (VectorHintingRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
GFxShapeBase* GFxExternalFontPS3::GetGlyphShape(UInt glyphIndex, UInt glyphSize)
{
    if (glyphIndex == (UInt)-1)
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, glyphSize))
        glyphSize = 0;

    UInt pixelSize = glyphSize ? glyphSize : FontHeight;

    if (LastFontHeight != pixelSize)
    {
        // cellFontSetScalePixel is expensive. Avoid calling it often.
        cellFontSetScalePixel(&Font, (float)pixelSize, (float)pixelSize);
        LastFontHeight = pixelSize;
    }

    CellFontGlyph *glyph;
    int err = cellFontGenerateCharGlyph(&Font, Glyphs[glyphIndex].Code, &glyph);
    if (err)
        return 0;

    GFxShapeNoStyles* pshape = new GFxShapeNoStyles(ShapePageSize);
    pshape->SetHintedGlyphSize(glyphSize);
    if(!decomposeGlyphOutline(glyph, pshape))
    {
        pshape->Release();
        pshape = 0;
    }
    cellFontDeleteGlyph(&Font, glyph);
    return pshape;
}

//------------------------------------------------------------------------
GFxGlyphRaster* GFxExternalFontPS3::GetGlyphRaster(UInt glyphIndex,
                                                   UInt glyphSize)
{
    if (!IsHintedRasterGlyph(glyphIndex, glyphSize))
        return 0;

return 0;
    //GlyphType& glyph = Glyphs[glyphIndex];

    //if (LastFontHeight != glyphSize)
    //{
    //    // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
    //    FT_Set_Pixel_Sizes(Face, glyphSize, glyphSize);
    //    LastFontHeight = glyphSize;
    //}

    //int err = FT_Load_Glyph(Face, glyph.FtIndex, FT_LOAD_DEFAULT);
    //if (err)
    //    return 0;

    //err = FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_MONO);
    //if (err)
    //    return 0;

    //if (pRaster.GetPtr() == 0)
    //    pRaster = *new GFxGlyphRaster;

    //decomposeGlyphBitmap(Face->glyph->bitmap, 
    //                     Face->glyph->bitmap_left,
    //                     Face->glyph->bitmap_top,
    //                     pRaster);
    //return pRaster;
}

//------------------------------------------------------------------------
Float GFxExternalFontPS3::GetAdvance(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphWidth();

    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
Float GFxExternalFontPS3::GetKerningAdjustment(UInt lastCode, UInt thisCode) const
{
    if(FontOK == CELL_OK)
    {
        CellFontKerning kerning;
        CellFont tmpFont = Font;
        int err = cellFontGetKerning(&tmpFont, lastCode, thisCode, &kerning);
        if (err == CELL_OK)
            return kerning.offsetX;
    }
    return 0;
}

//------------------------------------------------------------------------
Float GFxExternalFontPS3::GetGlyphWidth(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphWidth();

    const GRectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
Float GFxExternalFontPS3::GetGlyphHeight(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphHeight();

    const GRectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
GRectF& GFxExternalFontPS3::GetGlyphBounds(UInt glyphIndex, GRectF* prect) const
{
    if (glyphIndex == (UInt)-1)
        prect->SetRect(GetDefaultGlyphWidth(), GetDefaultGlyphHeight());
    else
        *prect = Glyphs[glyphIndex].Bounds;
    return *prect;
}

static void* GFxFontProviderPS3_malloc( void*obj, uint32_t size )
{
    GUNUSED(obj);
    return GALLOC(size, GStat_Default_Mem);
}
static void  GFxFontProviderPS3_free( void*obj, void*p )
{
    GUNUSED(obj);
    GFREE(p);
}
static void* GFxFontProviderPS3_realloc( void*obj, void* p, uint32_t size )
{
    GUNUSED(obj);
    return GREALLOC(p, size, GStat_Default_Mem);
}
static void* GFxFontProviderPS3_calloc( void*obj, uint32_t numb, uint32_t blockSize )
{
    GUNUSED(obj);
    void* p = GALLOC(numb * blockSize, GStat_Default_Mem);
    memset(p, 0, numb * blockSize);
    return p;
}

//------------------------------------------------------------------------
GFxFontProviderPS3::GFxFontProviderPS3(UInt fontCacheSize, UInt numUserFontEntries, UInt firstId)
{
    FontOK = cellSysmoduleLoadModule(CELL_SYSMODULE_FONT);
    if (FontOK == CELL_OK)
        FontOK = cellSysmoduleLoadModule(CELL_SYSMODULE_FREETYPE);
    if (FontOK == CELL_OK)
        cellSysmoduleLoadModule(CELL_SYSMODULE_FONTFT);

    if (FontOK == CELL_OK)
    {
        FontFileCache.Resize(fontCacheSize/4);
        UserFontEntries.Resize(numUserFontEntries);

        FontConfig.FileCache.buffer = &FontFileCache[0]; // Set the file read cache
        FontConfig.FileCache.size   = (uint32_t)(FontFileCache.GetSize() * 4);
        FontConfig.userFontEntrys   = &UserFontEntries[0]; // Set entry for the number of user
        FontConfig.userFontEntryMax = (uint32_t)UserFontEntries.GetSize();
        FontOK = cellFontInit(&FontConfig);
        if(FontOK == CELL_OK)
        {
            CellFontLibraryConfigFT_initialize(&FtConfig); // Initial setting

            FtConfig.MemoryIF.Object  = NULL;
            FtConfig.MemoryIF.Malloc  = GFxFontProviderPS3_malloc;
            FtConfig.MemoryIF.Free    = GFxFontProviderPS3_free;
            FtConfig.MemoryIF.Realloc = GFxFontProviderPS3_realloc;
            FtConfig.MemoryIF.Calloc  = GFxFontProviderPS3_calloc;

            FontOK = cellFontInitLibraryFreeType(&FtConfig, &Library);
            if (FontOK != CELL_OK) 
            {
                cellFontEnd();
            }
        }
    }

    NextFontId = firstId;
}

//------------------------------------------------------------------------
GFxFontProviderPS3::~GFxFontProviderPS3()
{
    if (FontOK == CELL_OK) 
    {
        FontOK = cellFontEndLibrary(Library);
        if (FontOK == CELL_OK)
        {
            cellFontEnd();
        }
    }
}

//------------------------------------------------------------------------
void GFxFontProviderPS3::MapSystemFont(const char* fontName, UInt fontFlags,
                                       uint32_t fontType, uint32_t fontMap,
                                       GFxFont::NativeHintingRange vectorHintingRange,
                                       GFxFont::NativeHintingRange rasterHintingRange, 
                                       UInt maxVectorHintedSize,
                                       UInt maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont | GFxFont::FF_NativeHinting;

    GFxFontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.SubNum       = 0;
    font.UniqueId     = 0;
    font.Type.map     = fontMap;
    font.Type.type    = fontType;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void GFxFontProviderPS3::MapFontToFile(const char* fontName, UInt fontFlags,
                                       const char* fileName, uint32_t subNum,
                                       GFxFont::NativeHintingRange vectorHintingRange,
                                       GFxFont::NativeHintingRange rasterHintingRange, 
                                       UInt maxVectorHintedSize,
                                       UInt maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont | GFxFont::FF_NativeHinting;

    GFxFontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FileName     = fileName;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.SubNum       = subNum;
    font.UniqueId     = NextFontId++;
    font.Type.map     = ~0U;
    font.Type.type    = ~0U;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void GFxFontProviderPS3::MapFontToMemory(const char* fontName, UInt fontFlags, 
                                         const char* fontData, UInt dataSize,
                                         uint32_t subNum,
                                         GFxFont::NativeHintingRange vectorHintingRange,
                                         GFxFont::NativeHintingRange rasterHintingRange, 
                                         UInt maxVectorHintedSize,
                                         UInt maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont | GFxFont::FF_NativeHinting;

    GFxFontInfoPS3 font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = fontData;
    font.FontDataSize = dataSize;
    font.SubNum       = subNum;
    font.UniqueId     = NextFontId++;
    font.Type.map     = ~0U;
    font.Type.type    = ~0U;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
GFxFont* GFxFontProviderPS3::CreateFont(const char* name, UInt fontFlags)
{
    if (FontOK != CELL_OK)
        return 0;

    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont;

    UInt i;
    for (i = 0; i < Fonts.GetSize(); ++i)
    {
        GFxFontInfoPS3& font = Fonts[i];
        if (font.FontName.CompareNoCase(name) == 0 && 
           (font.FontFlags & GFxFont::FF_Style_Mask) == (fontFlags & GFxFont::FF_Style_Mask))
        {
            GFxExternalFontPS3* newFont = new GFxExternalFontPS3(this, Library, &font);
            if (newFont && !newFont->IsValid())
            {
                newFont->Release();
                return 0;
            }
            return newFont;
        }
    }
    return 0;
}

//------------------------------------------------------------------------
void GFxFontProviderPS3::LoadFontNames(GStringHash<GString>& fontnames)
{
    for (UInt i = 0; i < Fonts.GetSize(); ++i)
    {
        fontnames.Set(Fonts[i].FontName, Fonts[i].FontName);
    }
}

}}

