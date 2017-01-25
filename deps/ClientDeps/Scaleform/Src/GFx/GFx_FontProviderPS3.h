/**********************************************************************

PublicHeader:   FontProvider_PS3
Filename    :   GFxFontProviderPS3.h
Content     :   PS3 Cell font provider
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

#ifndef INC_SF_GFX_FontProviderPS3_H
#define INC_SF_GFX_FontProviderPS3_H

#include <cell/font/libfont.h>
#include <cell/font/libfontFT.h>
#include <cell/font/ps3fontset.h>

#include "GFx/GFx_Font.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_String.h"

namespace Scaleform { 
namespace GFx
{

//------------------------------------------------------------------------
class GFxFontProviderPS3;
class PathPacker;
class ShapeNoStyles;


struct GFxFontInfoPS3
{
    String      FontName;
    unsigned     FontFlags;
    String      FileName;
    const char*  FontData;
    unsigned     FontDataSize;
    uint32_t     SubNum;
    int32_t      UniqueId;
    CellFontType Type;

    Font::NativeHintingRange RasterHintingRange;
    Font::NativeHintingRange VectorHintingRange;
    unsigned                    MaxRasterHintedSize;
    unsigned                    MaxVectorHintedSize;
};


class GFxExternalFontPS3 : public Font
{
    enum { FontHeight = 1024, ShapePageSize = 256-2 - 8-4 };
public:
    GFxExternalFontPS3(GFxFontProviderPS3* pprovider, 
                       const CellFontLibrary* library, 
                       GFxFontInfoPS3* fontInfo);

    virtual ~GFxExternalFontPS3();

    bool    IsValid() const { return FontOK == CELL_OK; }
    
    virtual TextureGlyphData* GetTextureGlyphData() const { return 0; }

    virtual int             GetGlyphIndex(UInt16 code) const;
    virtual bool            IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual bool            IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual ShapeDataBase*   GetGlyphShape(unsigned glyphIndex, unsigned glyphSize);
    virtual GlyphRaster* GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize);
    virtual float           GetAdvance(unsigned glyphIndex) const;
    virtual float           GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual float           GetGlyphWidth(unsigned glyphIndex) const;
    virtual float           GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&         GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    virtual const char*     GetName() const { return &Name[0]; }

private:
    void    setFontMetrics();
    bool    decomposeGlyphOutline(const CellFontGlyph* glyph, ShapeNoStyles* shape);
//    void    decomposeGlyphBitmap(const FT_Bitmap& bitmap, int x, int y, GlyphRaster* raster);

/*
    static inline int FtToTwips(int v)
    {
        return (v * 20) >> 6;
    }

    static inline int FtToS1024(int v)
    {
        return v >> 6;
    }
*/

    struct GlyphType
    {
        unsigned    Code;
        //unsigned    FtIndex;
        float       Advance;
        RectF       Bounds;
    };

    struct KerningPairType
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPairType& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };

    // AddRef for font provider since it contains our cache.
    Ptr<GFxFontProviderPS3>        pFontProvider;    
    String                         Name;
    CellFont                       Font;
    Array<GlyphType>               Glyphs;
    HashIdentity<UInt16, unsigned> CodeTable;
    Hash<KerningPairType, float>   KerningPairs;
    Ptr<GlyphRaster>               pRaster;
    unsigned                       LastFontHeight;
    NativeHintingRange             RasterHintingRange;
    NativeHintingRange             VectorHintingRange;
    unsigned                       MaxRasterHintedSize;
    unsigned                       MaxVectorHintedSize;
    int                            FontOK;
};



//------------------------------------------------------------------------
class GFxFontProviderPS3 : public FontProvider
{
public:
    GFxFontProviderPS3(unsigned fontCacheSize=1024*1024, unsigned numUserFontEntries=8, unsigned firstId=0);
    virtual ~GFxFontProviderPS3();

    void MapSystemFont(const char* fontName, unsigned fontFlags, 
                       uint32_t fontType, uint32_t fontMap,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToFile(const char* fontName, unsigned fontFlags, 
                       const char* fileName, uint32_t subNum=0,
                       Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                       Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                       unsigned maxVectorHintedSize=24,
                       unsigned maxRasterHintedSize=24);

    void MapFontToMemory(const char* fontName, unsigned fontFlags, 
                         const char* fontData, unsigned dataSize, 
                         uint32_t subNum=0,
                         Font::NativeHintingRange vectorHintingRange = Font::DontHint,
                         Font::NativeHintingRange rasterHintingRange = Font::HintCJK, 
                         unsigned maxVectorHintedSize=24,
                         unsigned maxRasterHintedSize=24);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);

    virtual void        LoadFontNames(StringHash<String>& fontnames);

private:
    GFxExternalFontPS3* createFont(const GFxFontInfoPS3& font);

    Array<uint32_t>            FontFileCache;
    Array<CellFontEntry>       UserFontEntries;
    CellFontConfig             FontConfig;
    CellFontLibraryConfigFT    FtConfig;
    const CellFontLibrary*     Library;
    int32_t                    NextFontId;

    int                        FontOK;

    Array<GFxFontInfoPS3>      Fonts;
};

}}

#endif

