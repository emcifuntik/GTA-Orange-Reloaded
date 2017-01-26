/**********************************************************************

Filename    :   GFxFontProviderFT2.cpp
Content     :   FreeType2 font provider
Created     :   6/21/2007
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

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

#include "GFx/GFx_FontProviderFT2.h"
#include "Kernel/SF_String.h"
#include "Render/Render_Math2D.h"
#include "GFx/GFx_FT2Helper.h"

void GFxExternalFontFT2::cubicToQuadratic(GFxPathPacker& path, int hintedGlyphSize, 
                                          int x2, int y2, int x3, int y3, int x4, int y4)
{
    GFxFT2Helper::cubicToQuadratic(path, hintedGlyphSize, FontHeight, x2, y2, x3, y3, x4, y4);
}



//------------------------------------------------------------------------
bool GFxExternalFontFT2::decomposeGlyphOutline( const FT_Outline& outline, GFxShapeBase* shape )
{   
    GFxFTOutline goutline;
    goutline.NContours  = outline.n_contours;   
    goutline.NPoints    = outline.n_points;       
#ifdef GFC_BUILD_DEBUG
    FT_Vector testVector = {1,2};
    GFxFTVector *pgtestVector =  reinterpret_cast<GFxFTVector*>(&testVector);
    GASSERT((pgtestVector->x == 1) && (pgtestVector->y == 2));
#endif
    goutline.Points     = reinterpret_cast<GFxFTVector*>(outline.points);         
    goutline.Tags       = outline.tags;   
    goutline.Contours   = outline.contours; 
    goutline.Flags      = outline.flags;   

    return GFxFT2Helper::decomposeGlyphOutline(goutline, shape, FontHeight); 
}


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
void GFxExternalFontFT2::decomposeGlyphBitmap(const FT_Bitmap& bitmap,
                                              int x, int y,
                                              GFxGlyphRaster* raster)
{
    raster->Width   = bitmap.width;
    raster->Height  = bitmap.rows;
    raster->OriginX = -x;
    raster->OriginY =  y;
    raster->Raster.Resize(raster->Width * raster->Height);

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

//------------------------------------------------------------------------
GFxExternalFontFT2::~GFxExternalFontFT2()
{
    if (Face)
        FT_Done_Face(Face);
}

//------------------------------------------------------------------------
GFxExternalFontFT2::GFxExternalFontFT2(GFxFontProviderFT2* pprovider, 
                                       FT_Library lib, 
                                       const GString& fontName, 
                                       UInt fontFlags,
                                       const char* fileName, 
                                       UInt faceIndex) :
    GFxFont(fontFlags),
    pFontProvider(pprovider),
    Name(fontName),
    LastFontHeight(FontHeight)
{
    int err = FT_New_Face(lib, fileName, faceIndex, &Face);
    if (err)
    {
        Face = 0;
        return;
    }
    setFontMetrics();
}

//------------------------------------------------------------------------
GFxExternalFontFT2::GFxExternalFontFT2(GFxFontProviderFT2* pprovider, 
                                       FT_Library lib, 
                                       const GString& fontName, 
                                       UInt fontFlags,
                                       const char* fontMem, 
                                       UInt fontMemSize, 
                                       UInt faceIndex) :
    GFxFont(fontFlags),
    pFontProvider(pprovider),
    Name(fontName)
{
    int err = FT_New_Memory_Face(lib, 
                                 (const FT_Byte*)fontMem, 
                                 fontMemSize, 
                                 faceIndex,
                                 &Face);
    if (err)
    {
        Face = 0;
        return;
    }
    setFontMetrics();
}

//------------------------------------------------------------------------
void GFxExternalFontFT2::SetHinting(NativeHintingRange vectorHintingRange,
                                    NativeHintingRange rasterHintingRange, 
                                    UInt maxVectorHintedSize,
                                    UInt maxRasterHintedSize)
{
    VectorHintingRange  = vectorHintingRange;
    RasterHintingRange  = rasterHintingRange;
    MaxVectorHintedSize = maxVectorHintedSize;
    MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
void GFxExternalFontFT2::setFontMetrics()
{
    FT_Set_Pixel_Sizes(Face, FontHeight, FontHeight);
    LastFontHeight = FontHeight;
    Float ascent  =  Float(Face->ascender)  * FontHeight / Face->units_per_EM;
    Float descent = -Float(Face->descender) * FontHeight / Face->units_per_EM;
    Float height  =  Float(Face->height)    * FontHeight / Face->units_per_EM;
    SetFontMetrics(height - ascent + descent, ascent, descent);
}


//------------------------------------------------------------------------
int GFxExternalFontFT2::GetGlyphIndex(UInt16 code) const
{
    if (Face)
    {
        const UInt* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        GFxExternalFontFT2* pthis = const_cast<GFxExternalFontFT2*>(this);
        if (LastFontHeight != FontHeight)
        {
            // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
            FT_Set_Pixel_Sizes(Face, FontHeight, FontHeight);
            pthis->LastFontHeight = FontHeight;
        }
        UInt ftIndex = FT_Get_Char_Index(Face, code);
        int  err     = FT_Load_Glyph(Face, ftIndex, FT_LOAD_NO_HINTING);

        if (err)
            return -1;

        GlyphType glyph;
        glyph.Code          =  code;
        glyph.FtIndex       =  ftIndex;
        glyph.Advance       =  Float((Face->glyph->advance.x + 32) >> 6);
        glyph.Bounds.Left   =  Float(Face->glyph->metrics.horiBearingX >> 6);
        glyph.Bounds.Top    = -Float(Face->glyph->metrics.horiBearingY >> 6);
        glyph.Bounds.Right  =  Float(Face->glyph->metrics.width  >> 6) + glyph.Bounds.Left;
        glyph.Bounds.Bottom =  Float(Face->glyph->metrics.height >> 6) + glyph.Bounds.Top;

        pthis->Glyphs.PushBack(glyph);
        pthis->CodeTable.Add(code, (UInt)Glyphs.GetSize()-1);
        return (UInt)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool GFxExternalFontFT2::IsHintedVectorGlyph(UInt glyphIndex, UInt glyphSize) const
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
bool GFxExternalFontFT2::IsHintedRasterGlyph(UInt glyphIndex, UInt glyphSize) const
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
GFxShapeBase* GFxExternalFontFT2::GetGlyphShape(UInt glyphIndex,
                                                UInt glyphSize)
{
    if (glyphIndex == (UInt)-1)
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, glyphSize))
        glyphSize = 0;

    UInt pixelSize = glyphSize ? glyphSize : FontHeight;

    if (LastFontHeight != pixelSize)
    {
        // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
        FT_Set_Pixel_Sizes(Face, pixelSize, pixelSize);
        LastFontHeight = pixelSize;
    }

    const GlyphType& glyph = Glyphs[glyphIndex];
    int err = FT_Load_Glyph(Face, glyph.FtIndex, FT_LOAD_DEFAULT);
    if (err)
        return 0;

    GFxShapeBase* pshape = GFxFT2Helper::CreateShape(ShapePageSize, glyphSize);
    if (!decomposeGlyphOutline(Face->glyph->outline, pshape))
    {
        GFxFT2Helper::ReleaseShape(pshape);
        pshape = 0;
    }

    return pshape;
}

//------------------------------------------------------------------------
GFxGlyphRaster* GFxExternalFontFT2::GetGlyphRaster(UInt glyphIndex,
                                                   UInt glyphSize)
{
    if (!IsHintedRasterGlyph(glyphIndex, glyphSize))
        return 0;

    GlyphType& glyph = Glyphs[glyphIndex];

    if (LastFontHeight != glyphSize)
    {
        // FT_Set_Pixel_Sizes is expensive. Avoid calling it often.
        FT_Set_Pixel_Sizes(Face, glyphSize, glyphSize);
        LastFontHeight = glyphSize;
    }

    int err = FT_Load_Glyph(Face, glyph.FtIndex, FT_LOAD_DEFAULT);
    if (err)
        return 0;

    err = FT_Render_Glyph(Face->glyph, FT_RENDER_MODE_MONO);
    if (err)
        return 0;

    if (pRaster.GetPtr() == 0)
        pRaster = *new GFxGlyphRaster;

    decomposeGlyphBitmap(Face->glyph->bitmap, 
                         Face->glyph->bitmap_left,
                         Face->glyph->bitmap_top,
                         pRaster);
    return pRaster;
}

//------------------------------------------------------------------------
Float GFxExternalFontFT2::GetAdvance(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphWidth();

    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
Float GFxExternalFontFT2::GetKerningAdjustment(UInt lastCode, UInt thisCode) const
{
    if(Face && FT_HAS_KERNING(Face))
    {
        FT_Vector delta;
        FT_Get_Kerning(Face, 
                       FT_Get_Char_Index(Face, lastCode), 
                       FT_Get_Char_Index(Face, thisCode),
                       FT_KERNING_DEFAULT, &delta);
        return Float(delta.x >> 6);
    }
    return 0;
}

//------------------------------------------------------------------------
Float GFxExternalFontFT2::GetGlyphWidth(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphWidth();

    const GRectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
Float GFxExternalFontFT2::GetGlyphHeight(UInt glyphIndex) const
{
    if (glyphIndex == (UInt)-1)
        return GetDefaultGlyphHeight();

    const GRectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
GRectF& GFxExternalFontFT2::GetGlyphBounds(UInt glyphIndex, GRectF* prect) const
{
    if (glyphIndex == (UInt)-1)
        prect->SetRect(GetDefaultGlyphWidth(), GetDefaultGlyphHeight());
    else
        *prect = Glyphs[glyphIndex].Bounds;
    return *prect;
}





//------------------------------------------------------------------------
GFxFontProviderFT2::GFxFontProviderFT2(FT_Library lib):
    Lib(lib),
    ExtLibFlag(true),
    NamesEndIdx(0)
{
    if (Lib == 0)
    {
        int err = FT_Init_FreeType(&Lib);
        if (err) 
            Lib = 0;
        ExtLibFlag = false;
    }
}

//------------------------------------------------------------------------
GFxFontProviderFT2::~GFxFontProviderFT2()
{
    if (Lib && !ExtLibFlag)
        FT_Done_FreeType(Lib);
}

//------------------------------------------------------------------------
void GFxFontProviderFT2::MapFontToFile(const char* fontName, 
                                       UInt fontFlags,
                                       const char* fileName, 
                                       UInt faceIndex,
                                       GFxFont::NativeHintingRange vectorHintingRange,
                                       GFxFont::NativeHintingRange rasterHintingRange, 
                                       UInt maxVectorHintedSize,
                                       UInt maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont | GFxFont::FF_NativeHinting;

    FontType font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FileName     = fileName;
    font.FontData     = 0;
    font.FontDataSize = 0;
    font.FaceIndex    = faceIndex;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
void GFxFontProviderFT2::MapFontToMemory(const char* fontName, 
                                         UInt fontFlags, 
                                         const char* fontData, UInt dataSize,
                                         UInt faceIndex,
                                         GFxFont::NativeHintingRange vectorHintingRange,
                                         GFxFont::NativeHintingRange rasterHintingRange, 
                                         UInt maxVectorHintedSize,
                                         UInt maxRasterHintedSize)
{
    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont | GFxFont::FF_NativeHinting;

    FontType font;
    font.FontName     = fontName;
    font.FontFlags    = fontFlags;
    font.FontData     = fontData;
    font.FontDataSize = dataSize;
    font.FaceIndex    = faceIndex;
    font.VectorHintingRange  = vectorHintingRange;
    font.RasterHintingRange  = rasterHintingRange;
    font.MaxVectorHintedSize = maxVectorHintedSize;
    font.MaxRasterHintedSize = maxRasterHintedSize;
    Fonts.PushBack(font);
}

//------------------------------------------------------------------------
GFxExternalFontFT2* GFxFontProviderFT2::createFont(const FontType& font)
{
    GFxExternalFontFT2* newFont = font.FontData?
        new GFxExternalFontFT2(this, Lib, font.FontName, font.FontFlags, font.FontData, font.FontDataSize, font.FaceIndex):
        new GFxExternalFontFT2(this, Lib, font.FontName, font.FontFlags, font.FileName, font.FaceIndex);

    if (newFont && !newFont->IsValid())
    {
        newFont->Release();
        return 0;
    }
    newFont->SetHinting(font.VectorHintingRange,  font.RasterHintingRange,
                        font.MaxVectorHintedSize, font.MaxRasterHintedSize);
    return newFont;
}

//------------------------------------------------------------------------
GFxFont* GFxFontProviderFT2::CreateFont(const char* name, UInt fontFlags)
{
    if (Lib == 0)
        return 0;

    // Mask flags to be safe.
    fontFlags &= GFxFont::FF_CreateFont_Mask;
    fontFlags |= GFxFont::FF_DeviceFont;

    UInt i;
    for (i = 0; i < Fonts.GetSize(); ++i)
    {
        FontType& font = Fonts[i];
        if (font.FontName.CompareNoCase(name) == 0 && 
           (font.FontFlags & GFxFont::FF_Style_Mask) == (fontFlags & GFxFont::FF_Style_Mask))
        {
            return createFont(font);
        }
    }
    return 0;
}

//------------------------------------------------------------------------
void GFxFontProviderFT2::LoadFontNames(GStringHash<GString>& fontnames)
{
    for (UInt i = 0; i < Fonts.GetSize(); ++i)
    {
        fontnames.Set(Fonts[i].FontName, Fonts[i].FontName);
    }
}
