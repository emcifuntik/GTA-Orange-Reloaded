/**********************************************************************

PublicHeader:   FontProvider_Win32
Filename    :   FontProviderWin32.h
Content     :   Win32 API Font provider (GetGlyphOutline)
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

#ifndef INC_SF_GFX_FontProviderWin32_H
#define INC_SF_GFX_FontProviderWin32_H

#include "Render/FontProvider/Render_FontProviderWinAPI.h"
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {


typedef Render::ExternalFontWinAPI ExternalFontWin32;

class FontProviderWin32 : public FontProvider
{
    typedef Render::Font Font;

    Render::FontProviderWinAPI* GetFontProviderWinAPI();
public:
    FontProviderWin32(HDC dc);

    void SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
        Font::NativeHintingRange rasterRange, 
        unsigned maxVectorHintedSize=24,
        unsigned maxRasterHintedSize=24);
    void SetHinting(const char* name, 
        Font::NativeHintingRange vectorRange, 
        Font::NativeHintingRange rasterRange, 
        unsigned maxVectorHintedSize=24,
        unsigned maxRasterHintedSize=24);

    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    

    virtual void     LoadFontNames(StringHash<String>& fontnames);
};

#if 0
class ShapeNoStyles;

//------------------------------------------------------------------------
struct FontSysDataWin32
{
    enum { BufSizeInc = 1024-8 };

    Array<UByte>    GlyphBuffer;
    HDC             WinHDC;

    FontSysDataWin32(HDC dc) : WinHDC(dc) {}
};



//------------------------------------------------------------------------
class FontProviderWin32;

class ExternalFontWin32 : public Font
{
    // GGO_FontHeight is the font height to request the 
    // glyphs with the nominal size (not the specific hinted size). 
    // It appears to be extremelly expensive to call GetGlyphOutline
    // with font height of 256 or more. So, the GGO_FontHeight
    // is the "maximum inexpensive size" for GetGlyphOutline.
    enum 
    { 
        GGO_FontHeight = 240, 
        ShapePageSize = 256-2 - 8-4
    };
public:
    ExternalFontWin32(FontProviderWin32 *pprovider,
                         FontSysDataWin32* pSysData, 
                         const char* name, unsigned fontFlags);

    virtual ~ExternalFontWin32();


    bool    IsValid() const { return MasterFont != 0; }
    void    SetHinting(const Font::NativeHintingType& pnh);

    virtual TextureGlyphData*   GetTextureGlyphData() const { return 0; }

    virtual int                 GetGlyphIndex(UInt16 code) const;
    virtual bool                IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual bool                IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const;
    virtual ShapeDataBase*          GetGlyphShape(unsigned glyphIndex, unsigned glyphSize);
    virtual GlyphRaster*        GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize);
    virtual float               GetAdvance(unsigned glyphIndex) const;
    virtual float               GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual float               GetGlyphWidth(unsigned glyphIndex) const;
    virtual float               GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&              GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    virtual const char*         GetName() const;

private:
    bool    decomposeGlyphOutline(const UByte* data, unsigned size, ShapeNoStyles* shape);
    void    decomposeGlyphBitmap(const UByte* data, int w, int h, int x, int y, GlyphRaster* raster);
    void    loadKerningPairs();

    static inline int FxToTwips(FIXED v)
    {
        return int(v.value) * 20 + int(v.fract) * 20 / 65536;
    }

    static inline int FxToS1024(FIXED v)
    {
        return ((int(v.value) << 8) + (int(v.fract) >> 8)) * 4 / GGO_FontHeight;
    }

    struct GlyphType
    {
        unsigned            Code;
        float               Advance;
        RectF              Bounds;
    };

    struct KerningPairType
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPairType& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };

    // AddRef for font provider since it contains our SysData.
    Ptr<FontProviderWin32>          pFontProvider;    
    FontSysDataWin32*               pSysData;
    ArrayLH<char>                   Name; 
    ArrayLH<WCHAR>                  NameW;
    HFONT                           MasterFont;
    HFONT                           HintedFont;
    unsigned                        LastHintedFontSize;
    ArrayLH<GlyphType>              Glyphs;
    HashIdentityLH<UInt16, unsigned> CodeTable;
    HashLH<KerningPairType, float>  KerningPairs;
    Ptr<GlyphRaster>                pRaster;
    float                           Scale1024;
    NativeHintingType               Hinting;
};



//------------------------------------------------------------------------
class FontProviderWin32 : public FontProvider
{
public:
    FontProviderWin32(HDC dc);
    virtual ~FontProviderWin32();

    void SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                            Font::NativeHintingRange rasterRange, 
                            unsigned maxVectorHintedSize=24,
                            unsigned maxRasterHintedSize=24);
    void SetHinting(const char* name, 
                    Font::NativeHintingRange vectorRange, 
                    Font::NativeHintingRange rasterRange, 
                    unsigned maxVectorHintedSize=24,
                    unsigned maxRasterHintedSize=24);
    
    virtual Font*    CreateFont(const char* name, unsigned fontFlags);    

    virtual void        LoadFontNames(StringHash<String>& fontnames);

private:
    Font::NativeHintingType* findNativeHinting(const char* name);

    FontSysDataWin32                SysData;
    Array<Font::NativeHintingType>  NativeHinting;
};
#endif
}} // namespace Scaleform { namespace GFx {

#endif
