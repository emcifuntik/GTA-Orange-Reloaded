/**********************************************************************

PublicHeader:   FontProvider_FT2
Filename    :   GFxFontProviderFT2.h
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

#ifndef INC_SF_GFX_FontProviderFT2_H
#define INC_SF_GFX_FontProviderFT2_H


#include <ft2build.h>
#include FT_FREETYPE_H

#include "GFx/GFx_Font.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_String.h"

//------------------------------------------------------------------------
class GFxFontProviderFT2;
class PathPacker;
class ShapeNoStyles;

class GFxExternalFontFT2 : public Font
{
    enum { FontHeight = 1024, ShapePageSize = 256-2 - 8-4 };
public:
    GFxExternalFontFT2(GFxFontProviderFT2* pprovider, 
                       FT_Library lib, 
                       const String& fontName, 
                       unsigned fontFlags,
                       const char* fileName, 
                       unsigned faceIndex);

    GFxExternalFontFT2(GFxFontProviderFT2* pprovider, 
                       FT_Library lib, 
                       const String& fontName, 
                       unsigned fontFlags,
                       const char* fontMem, 
                       unsigned fontMemSize, 
                       unsigned faceIndex);

    void SetHinting(NativeHintingRange vectorHintingRange,
                    NativeHintingRange rasterHintingRange, 
                    unsigned maxVectorHintedSize,
                    unsigned maxRasterHintedSize);

    virtual ~GFxExternalFontFT2();

    bool    IsValid() const { return Face != 0; }
    
    virtual TextureGlyphData*    GetTextureGlyphData() const { return 0; }

    virtual int                     GetGlyphIndex(UInt16 code) const;
    virtual bool                    IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual bool                    IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual ShapeDataBase*           GetGlyphShape(unsigned glyphIndex, unsigned glyphSize);
    virtual GlyphRaster*         GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize);
    virtual float                   GetAdvance(unsigned glyphIndex) const;
    virtual float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual float                   GetGlyphWidth(unsigned glyphIndex) const;
    virtual float                   GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&                 GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    virtual const char*             GetName() const { return &Name[0]; }

private:
    void    setFontMetrics();
    bool    decomposeGlyphOutline(const FT_Outline& outline, ShapeDataBase* shape);
    void    decomposeGlyphBitmap(const FT_Bitmap& bitmap, int x, int y, GlyphRaster* raster);

    static inline int SF_STDCALL FtToTwips(int v)
    {
        return (v * 20) >> 6;
    }

    static inline int SF_STDCALL FtToS1024(int v)
    {
        return v >> 6;
    }

    static void SF_STDCALL cubicToQuadratic(PathPacker& path, int hintedGlyphSize, 
                                 int x2, int y2, int x3, int y3, int x4, int y4);

    struct GlyphType
    {
        unsigned                Code;
        unsigned                FtIndex;
        float                   Advance;
        RectF                  Bounds;
    };

    struct KerningPairType
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPairType& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };

    // AddRef for font provider since it contains our cache.
    Ptr<GFxFontProviderFT2>            pFontProvider;    
    String                             Name;
    FT_Face                             Face;
    Array<GlyphType>                   Glyphs;
    HashIdentity<UInt16, unsigned>         CodeTable;
    Hash<KerningPairType, float>       KerningPairs;
    Ptr<GlyphRaster>                pRaster;
    unsigned                            LastFontHeight;
    NativeHintingRange                  RasterHintingRange;
    NativeHintingRange                  VectorHintingRange;
    unsigned                            MaxRasterHintedSize;
    unsigned                            MaxVectorHintedSize;
};



//------------------------------------------------------------------------
class GFxFontProviderFT2 : public FontProvider
{
public:
    GFxFontProviderFT2(FT_Library lib=0);
    virtual ~GFxFontProviderFT2();

    FT_Library GetFT_Library() { return Lib; }

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, unsigned faceIndex=0, 
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         unsigned faceIndex=0, 
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);

    virtual void        LoadFontNames(StringHash<String>& fontnames);

private:
    struct FontType
    {
        String                     FontName;
        unsigned                    FontFlags;
        String                     FileName;
        const char*                 FontData;
        unsigned                    FontDataSize;
        unsigned                    FaceIndex;
        Font::NativeHintingRange RasterHintingRange;
        Font::NativeHintingRange VectorHintingRange;
        unsigned                    MaxRasterHintedSize;
        unsigned                    MaxVectorHintedSize;
    };

    GFxExternalFontFT2* createFont(const FontType& font);

    FT_Library          Lib;
    bool                ExtLibFlag;
    Array<FontType>    Fonts;
    unsigned            NamesEndIdx;
};




#endif
