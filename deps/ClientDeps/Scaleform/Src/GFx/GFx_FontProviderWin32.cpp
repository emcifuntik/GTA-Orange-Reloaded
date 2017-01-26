/**********************************************************************

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

#include "GFx/GFx_FontProviderWin32.h"
#include "GFx/GFx_String.h"
#include "GFx/GFx_Shape.h"
#include "Kernel/SF_Std.h"

namespace Scaleform { namespace GFx {

Render::FontProviderWinAPI* FontProviderWin32::GetFontProviderWinAPI()
{
    return static_cast<Render::FontProviderWinAPI*>(pFontProvider.GetPtr());
}

FontProviderWin32::FontProviderWin32(HDC dc) 
: FontProvider(Ptr<FontProviderWinAPI>(*new Render::FontProviderWinAPI(dc)).GetPtr())
{

}

void FontProviderWin32::SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
    Font::NativeHintingRange rasterRange, 
    unsigned maxVectorHintedSize,
    unsigned maxRasterHintedSize)
{
    GetFontProviderWinAPI()->SetHintingAllFonts(vectorRange, rasterRange, maxVectorHintedSize, maxRasterHintedSize);
}

void FontProviderWin32::SetHinting(const char* name, 
    Font::NativeHintingRange vectorRange, 
    Font::NativeHintingRange rasterRange, 
    unsigned maxVectorHintedSize,
    unsigned maxRasterHintedSize)
{
    GetFontProviderWinAPI()->SetHinting(name, vectorRange, rasterRange, maxVectorHintedSize, maxRasterHintedSize);  
}

Font* FontProviderWin32::CreateFont(const char* name, unsigned fontFlags)
{
    return GetFontProviderWinAPI()->CreateFont(name, fontFlags);
}

void FontProviderWin32::LoadFontNames(StringHash<String>& fontnames)
{
    GetFontProviderWinAPI()->LoadFontNames(fontnames);
}

#if 0
    //------------------------------------------------------------------------
struct GFxGdiSelectObjectGuard
{
    HDC     WinDC;
    HGDIOBJ WinObj;
    GFxGdiSelectObjectGuard(HDC dc, HGDIOBJ obj) : WinDC(dc), WinObj(::SelectObject(dc, obj)) {}
   ~GFxGdiSelectObjectGuard() { ::SelectObject(WinDC, WinObj); }
};

//------------------------------------------------------------------------
ExternalFontWin32::~ExternalFontWin32()
{
    if (HintedFont)
        ::DeleteObject(HintedFont);

    if (MasterFont)
        ::DeleteObject(MasterFont);
}

//------------------------------------------------------------------------
int CALLBACK EnumFontFamExProc(
                              ENUMLOGFONTEXW *lpelfe, // logical-font data
                              NEWTEXTMETRICEXW *lpntme, // physical-font data
                              DWORD FontType, // type of font
                              LPARAM lParam // application-defined data
                              )
{
    SF_UNUSED3(lpelfe,lpntme,FontType);
    bool* found = (bool*)lParam;
    *found = true;
    return 0;
}

ExternalFontWin32::ExternalFontWin32(FontProviderWin32 *pprovider,
                                           FontSysDataWin32* sysData, 
                                           const char* name, 
                                           unsigned fontFlags)
    : Font(fontFlags),
      pFontProvider(pprovider),
      pSysData(sysData),
      MasterFont(0), HintedFont(0), LastHintedFontSize(0),
      Scale1024(1024.0f / float(GGO_FontHeight))
      
{
    Name.Resize(SFstrlen(name) + 1);
    SFstrcpy(&Name[0], SFstrlen(name) + 1, name);
   
    // Font name is encoded as UTF8, so unpack.
    NameW.Resize(UTF8Util::GetLength(name) + 1);
    UTF8Util::DecodeString(&NameW[0], name);

    LOGFONTW lf;
    lstrcpyW((LPWSTR)&lf.lfFaceName, &NameW[0]);
    lf.lfCharSet = DEFAULT_CHARSET;

    bool found = false;
    EnumFontFamiliesExW(pSysData->WinHDC, &lf,   (FONTENUMPROCW)EnumFontFamExProc, (LPARAM)&found, 0);
    if (!found)
    {
        // do not abort if font name is _sans, _typewriter or _serif;
        // let system choose the font
        if (SFstrcmp(name, "_sans") != 0 && 
            SFstrcmp(name, "_typewriter") != 0 && 
            SFstrcmp(name, "_serif") != 0)
        return;
    }

    MasterFont = ::CreateFontW(-GGO_FontHeight,             // height of font
                                0,                          // average character width
                                0,                          // angle of escapement
                                0,                          // base-line orientation angle
                                IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                IsItalic(),                 // italic attribute option
                                0,                          // underline attribute option
                                0,                          // strikeout attribute option
                                DEFAULT_CHARSET,            // character set identifier
                                OUT_DEFAULT_PRECIS,         // output precision
                                CLIP_DEFAULT_PRECIS,        // clipping precision
                                ANTIALIASED_QUALITY,        // output quality
                                DEFAULT_PITCH,              // pitch and family
                                &NameW[0]);                 // typeface name

    if(MasterFont)
    {
        TEXTMETRICW tm;
        GFxGdiSelectObjectGuard g1(pSysData->WinHDC, MasterFont);
        ::GetTextMetricsW(pSysData->WinHDC, &tm);
        SetFontMetrics(float(tm.tmExternalLeading) * Scale1024, 
                       float(tm.tmAscent) * Scale1024, 
                       float(tm.tmDescent) * Scale1024);
        loadKerningPairs();
    }
}

//------------------------------------------------------------------------
void ExternalFontWin32::SetHinting(const Font::NativeHintingType& pnh)
{
    Hinting.MaxRasterHintedSize = pnh.MaxRasterHintedSize;
    Hinting.MaxVectorHintedSize = pnh.MaxVectorHintedSize;
    Hinting.RasterRange         = pnh.RasterRange;
    Hinting.VectorRange         = pnh.VectorRange;
}

//------------------------------------------------------------------------
void ExternalFontWin32::loadKerningPairs()
{
    Array<KERNINGPAIR> pairs;
    unsigned size = ::GetKerningPairsW(pSysData->WinHDC, 0, 0);
    if(size)
    {
        pairs.Resize(size);
        ::GetKerningPairsW(pSysData->WinHDC, size, &pairs[0]);
    }
    KerningPairs.Clear();
    for(unsigned i = 0; i < pairs.GetSize(); ++i)
    {
        KerningPairType pair;
        pair.Char0 = pairs[i].wFirst;
        pair.Char1 = pairs[i].wSecond;
        KerningPairs.Add(pair, float(pairs[i].iKernAmount) * Scale1024);
    }
}

//------------------------------------------------------------------------
bool ExternalFontWin32::decomposeGlyphOutline(const UByte* data, 
                                                 unsigned size, 
                                                 ShapeNoStyles* shape)
{
    const UByte* curGlyph = data;
    const UByte* endGlyph = data + size;
    bool  hinted = shape->GetHintedGlyphSize() != 0;
    
    PathPacker path;
    bool shapeValid = false;

    while(curGlyph < endGlyph)
    {
        const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)curGlyph;
        
        const UByte* endPoly = curGlyph + th->cb;
        const UByte* curPoly = curGlyph + sizeof(TTPOLYGONHEADER);

        path.Reset();
        path.SetFill0(1);
        path.SetFill1(0);
        if (hinted)
            path.SetMoveTo(FxToTwips(th->pfxStart.x), -FxToTwips(th->pfxStart.y));
        else
            path.SetMoveTo(FxToS1024(th->pfxStart.x), -FxToS1024(th->pfxStart.y));

        while(curPoly < endPoly)
        {
            const TTPOLYCURVE* pc = (const TTPOLYCURVE*)curPoly;
            
            if (pc->wType == TT_PRIM_LINE)
            {
                int i;
                for (i = 0; i < pc->cpfx; i++)
                {
                    if (hinted)
                        path.LineToAbs(FxToTwips(pc->apfx[i].x), -FxToTwips(pc->apfx[i].y));
                    else
                        path.LineToAbs(FxToS1024(pc->apfx[i].x), -FxToS1024(pc->apfx[i].y));
                }
            }
            
            if (pc->wType == TT_PRIM_QSPLINE)
            {
                int u;
                for (u = 0; u < pc->cpfx - 1; u++)  // Walk through points in spline
                {
                    POINTFX pntB = pc->apfx[u];     // B is always the current point
                    POINTFX pntC = pc->apfx[u+1];
                    
                    if (u < pc->cpfx - 2)           // If not on last spline, compute C
                    {
                        // midpoint (x,y)
                        *(int*)&pntC.x = (*(int*)&pntB.x + *(int*)&pntC.x) / 2;
                        *(int*)&pntC.y = (*(int*)&pntB.y + *(int*)&pntC.y) / 2;
                    }
                    if (hinted)
                        path.CurveToAbs(FxToTwips(pntB.x), -FxToTwips(pntB.y),
                                        FxToTwips(pntC.x), -FxToTwips(pntC.y));
                    else
                        path.CurveToAbs(FxToS1024(pntB.x), -FxToS1024(pntB.y),
                                        FxToS1024(pntC.x), -FxToS1024(pntC.y));
                }
            }
            curPoly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
        }
        curGlyph += th->cb;
        if (!path.IsEmpty())
        {
            path.ClosePath();
            shape->AddPath(&path);
            shapeValid = true;
        }
    }
    return shapeValid;
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
void ExternalFontWin32::decomposeGlyphBitmap(const UByte* data, 
                                                int w, int h,
                                                int x, int y,
                                                GlyphRaster* raster)
{
    raster->Width  = w;
    raster->Height = h;
    raster->OriginX = -x;
    raster->OriginY =  y;
    raster->Raster.Resize(w * h);

    int i;
    int pitch = ((w + 31) >> 5) << 2;

    const UByte* src = data;
          UByte* dst = &raster->Raster[0];
    for(i = 0; i < h; i++)
    {
        GFxBitsetIterator bits(src, 0);
        int j;
        for(j = 0; j < w; j++)
        {
            *dst++ = bits.GetBit() ? 255 : 0;
            ++bits;
        }
        src += pitch;
    }
}

//------------------------------------------------------------------------
int ExternalFontWin32::GetGlyphIndex(UInt16 code) const
{
    if (MasterFont)
    {
        const unsigned* indexPtr = CodeTable.Get(code);
        if (indexPtr)
            return *indexPtr;

        GFxGdiSelectObjectGuard g1(pSysData->WinHDC, MasterFont);
        GLYPHMETRICS gm;
        MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
        int ret = ::GetGlyphOutlineW(pSysData->WinHDC,
                                     code,
                                     GGO_METRICS,
                                     &gm,
                                     0, 0,
                                     &im);

        if (ret == GDI_ERROR)
            return -1;

        GlyphType glyph;
        glyph.Code          =  code;
        glyph.Advance       =  float(gm.gmCellIncX) * Scale1024;
        glyph.Bounds.x1     =  float(gm.gmptGlyphOrigin.x) * Scale1024;
        glyph.Bounds.y1     = -float(gm.gmptGlyphOrigin.y) * Scale1024;
        glyph.Bounds.x2     =  glyph.Bounds.x1 + float(gm.gmBlackBoxX) * Scale1024;
        glyph.Bounds.y2     =  glyph.Bounds.y1  + float(gm.gmBlackBoxY) * Scale1024;

        ExternalFontWin32* pthis = const_cast<ExternalFontWin32*>(this);
        pthis->Glyphs.PushBack(glyph);
        pthis->CodeTable.Add(code, (unsigned)Glyphs.GetSize()-1);
        return (unsigned)Glyphs.GetSize()-1;
    }
    return -1;
}

//------------------------------------------------------------------------
bool ExternalFontWin32::IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (glyphIndex == (unsigned)-1 ||
        Hinting.VectorRange == DontHint ||
        glyphSize > Hinting.MaxVectorHintedSize)
    {
        return false;
    }

    if (Hinting.VectorRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
bool ExternalFontWin32::IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const
{
    if (glyphIndex == (unsigned)-1 ||
        Hinting.RasterRange == DontHint ||
        glyphSize > Hinting.MaxRasterHintedSize)
    {
        return false;
    }

    if (Hinting.RasterRange == HintAll)
        return true;

    return IsCJK(UInt16(Glyphs[glyphIndex].Code));
}

//------------------------------------------------------------------------
ShapeDataBase* ExternalFontWin32::GetGlyphShape(unsigned glyphIndex,
                                                  unsigned glyphSize)
{
    if (glyphIndex == (unsigned)-1)
        return 0;

    if (!IsHintedVectorGlyph(glyphIndex, glyphSize))
        glyphSize = 0;

    GlyphType& glyph = Glyphs[glyphIndex];

    HFONT hFont = MasterFont;
    if (glyphSize)
    {
        if (glyphSize != LastHintedFontSize)
        {
            if (HintedFont)
                ::DeleteObject(HintedFont);

            HintedFont = ::CreateFontW(-int(glyphSize),             // height of font
                                        0,                          // average character width
                                        0,                          // angle of escapement
                                        0,                          // base-line orientation angle
                                        IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                        IsItalic(),                 // italic attribute option
                                        0,                          // underline attribute option
                                        0,                          // strikeout attribute option
                                        DEFAULT_CHARSET,            // character set identifier
                                        OUT_DEFAULT_PRECIS,         // output precision
                                        CLIP_DEFAULT_PRECIS,        // clipping precision
                                        ANTIALIASED_QUALITY,        // output quality
                                        DEFAULT_PITCH,              // pitch and family
                                        &NameW[0]);                 // typeface name
            LastHintedFontSize = glyphSize;
        }
        hFont = HintedFont;
    }

    GFxGdiSelectObjectGuard g1(pSysData->WinHDC, hFont);

    if (pSysData->GlyphBuffer.GetSize() == 0)
        pSysData->GlyphBuffer.Resize(FontSysDataWin32::BufSizeInc);

    #ifndef GGO_UNHINTED         // For compatibility with old SDKs.
    #define GGO_UNHINTED 0x0100
    #endif

    GLYPHMETRICS gm;
    int totalSize = 0;
    MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
    totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                   glyph.Code,
                                   GGO_NATIVE,
                                   &gm,
                                   (DWORD)pSysData->GlyphBuffer.GetSize(),
                                   &pSysData->GlyphBuffer[0],
                                   &im);

    if (totalSize == GDI_ERROR ||
        totalSize > (int)pSysData->GlyphBuffer.GetSize())
    {
        // An error has occured. Most probably it's because
        // of not enough room in the buffer. So, request the buffer size,
        // reallocate, and try again. The second call may fail for
        // some other reason (no glyph); in this case there's nothing 
        // else to do, just return NULL. 
        // The buffer is reallocated with adding BufSizeInc in order
        // to reduce the number of reallocations.
        //----------------------------------
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_NATIVE,
                                       &gm, 0, 0, &im);

        if(totalSize == GDI_ERROR) return 0;

        pSysData->GlyphBuffer.Resize(totalSize + FontSysDataWin32::BufSizeInc);
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_NATIVE,
                                       &gm,
                                       (DWORD)pSysData->GlyphBuffer.GetSize(),
                                       &pSysData->GlyphBuffer[0],
                                       &im);
        if (totalSize == GDI_ERROR ||
            totalSize > (int)pSysData->GlyphBuffer.GetSize())
            return 0;
    }

    ShapeNoStyles* pshape = 0;
    if (totalSize)
    {
        pshape = new ShapeNoStyles(ShapePageSize);
        pshape->SetHintedGlyphSize(glyphSize);
        if (!decomposeGlyphOutline(&pSysData->GlyphBuffer[0], totalSize, pshape))
        {
            pshape->Release();
            pshape = 0;
        }
    }
    return pshape;
}

//------------------------------------------------------------------------
GlyphRaster* ExternalFontWin32::GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize)
{
    if (!IsHintedRasterGlyph(glyphIndex, glyphSize))
        return 0;

    GlyphType& glyph = Glyphs[glyphIndex];
    if (glyphSize != LastHintedFontSize)
    {
        if (HintedFont)
            ::DeleteObject(HintedFont);

        HintedFont = ::CreateFontW(-int(glyphSize),             // height of font
                                    0,                          // average character width
                                    0,                          // angle of escapement
                                    0,                          // base-line orientation angle
                                    IsBold() ? FW_BOLD : FW_NORMAL, // font weight
                                    IsItalic(),                 // italic attribute option
                                    0,                          // underline attribute option
                                    0,                          // strikeout attribute option
                                    DEFAULT_CHARSET,            // character set identifier
                                    OUT_DEFAULT_PRECIS,         // output precision
                                    CLIP_DEFAULT_PRECIS,        // clipping precision
                                    ANTIALIASED_QUALITY,        // output quality
                                    DEFAULT_PITCH,              // pitch and family
                                    &NameW[0]);                 // typeface name
        LastHintedFontSize = glyphSize;
    }

    GFxGdiSelectObjectGuard g1(pSysData->WinHDC, HintedFont);

    if (pSysData->GlyphBuffer.GetSize() == 0)
        pSysData->GlyphBuffer.Resize(FontSysDataWin32::BufSizeInc);

    GLYPHMETRICS gm;
    int totalSize = 0;
    MAT2 im = { {0,1}, {0,0}, {0,0}, {0,1} };
    totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                   glyph.Code,
                                   GGO_BITMAP,
                                   &gm,
                                   (DWORD)pSysData->GlyphBuffer.GetSize(),
                                   &pSysData->GlyphBuffer[0],
                                   &im);

    if (totalSize == GDI_ERROR ||
        totalSize > (int)pSysData->GlyphBuffer.GetSize())
    {
        // An error has occured. Most probably it's because
        // of not enough room in the buffer. So, request the buffer size,
        // reallocate, and try again. The second call may fail for
        // some other reason (no glyph); in this case there's nothing 
        // else to do, just return NULL. 
        // The buffer is reallocated with adding BufSizeInc in order
        // to reduce the number of reallocations.
        //----------------------------------
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_BITMAP,
                                       &gm, 0, 0, &im);

        if(totalSize == GDI_ERROR) return 0;

        pSysData->GlyphBuffer.Resize(totalSize + FontSysDataWin32::BufSizeInc);
        totalSize = ::GetGlyphOutlineW(pSysData->WinHDC,
                                       glyph.Code,
                                       GGO_BITMAP,
                                       &gm,
                                       (DWORD)pSysData->GlyphBuffer.GetSize(),
                                       &pSysData->GlyphBuffer[0],
                                       &im);
        if (totalSize == GDI_ERROR ||
            totalSize > (int)pSysData->GlyphBuffer.GetSize())
            return 0;
    }

    if (pRaster.GetPtr() == 0)
        pRaster = *new GlyphRaster;

    if (totalSize)
    {
        decomposeGlyphBitmap(&pSysData->GlyphBuffer[0],
                             gm.gmBlackBoxX,
                             gm.gmBlackBoxY,
                             gm.gmptGlyphOrigin.x,
                             gm.gmptGlyphOrigin.y,
                             pRaster);
        return pRaster;
    }
    else
    {
        pRaster->Width  = 1;
        pRaster->Height = 1;
        pRaster->Raster.Resize(1);
        pRaster->Raster[0] = 0;
    }

    return pRaster;
}

//------------------------------------------------------------------------
float ExternalFontWin32::GetAdvance(unsigned glyphIndex) const
{
    if (glyphIndex == (unsigned)-1)
        return GetDefaultGlyphWidth();

    return Glyphs[glyphIndex].Advance;
}

//------------------------------------------------------------------------
float ExternalFontWin32::GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const
{
    float   adjustment;
    KerningPairType k;
    k.Char0 = (UInt16)lastCode;
    k.Char1 = (UInt16)thisCode;
    if (KerningPairs.Get(k, &adjustment))
    {
        return adjustment;
    }
    return 0;
}

//------------------------------------------------------------------------
float ExternalFontWin32::GetGlyphWidth(unsigned glyphIndex) const
{
    if (glyphIndex == (unsigned)-1)
        return GetDefaultGlyphWidth();

    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Width();
}

//------------------------------------------------------------------------
float ExternalFontWin32::GetGlyphHeight(unsigned glyphIndex) const
{
    if (glyphIndex == (unsigned)-1)
        return GetDefaultGlyphHeight();

    const RectF& r = Glyphs[glyphIndex].Bounds;
    return r.Height();
}

//------------------------------------------------------------------------
RectF& ExternalFontWin32::GetGlyphBounds(unsigned glyphIndex, RectF* prect) const
{
    if (glyphIndex == (unsigned)-1)
        prect->SetRect(GetDefaultGlyphWidth(), GetDefaultGlyphHeight());
    else
        *prect = Glyphs[glyphIndex].Bounds;
    return *prect;
}

//------------------------------------------------------------------------
const char*   ExternalFontWin32::GetName() const
{
    return &Name[0];
}




//------------------------------------------------------------------------
FontProviderWin32::FontProviderWin32(HDC dc): 
    SysData(dc)
{
    Font::NativeHintingType nhAllFonts;
    nhAllFonts.RasterRange = Font::HintCJK;
    nhAllFonts.VectorRange = Font::DontHint;
    nhAllFonts.MaxRasterHintedSize = 24;
    nhAllFonts.MaxVectorHintedSize = 24;
    NativeHinting.PushBack(nhAllFonts);
}

//------------------------------------------------------------------------
FontProviderWin32::~FontProviderWin32()
{    
}

//------------------------------------------------------------------------
Font::NativeHintingType* FontProviderWin32::findNativeHinting(const char* name)
{
    unsigned i;
    for (i = 0; i < NativeHinting.GetSize(); ++i)
    {
        if (NativeHinting[i].Typeface.CompareNoCase(name) == 0)
            return &NativeHinting[i];
    }
    return 0;
}

//------------------------------------------------------------------------
void FontProviderWin32::SetHinting(const char* name, 
                                      Font::NativeHintingRange vectorRange, 
                                      Font::NativeHintingRange rasterRange, 
                                      unsigned maxVectorHintedSize,
                                      unsigned maxRasterHintedSize)
{
    Font::NativeHintingType* pnh = findNativeHinting(name);
    if (pnh == 0)
    {
        Font::NativeHintingType nh = NativeHinting[0];
        nh.Typeface = name;
        NativeHinting.PushBack(nh);
        pnh = &NativeHinting[NativeHinting.GetSize()-1];
    }
    pnh->VectorRange         = vectorRange;
    pnh->RasterRange         = rasterRange;
    pnh->MaxVectorHintedSize = maxVectorHintedSize;
    pnh->MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
void FontProviderWin32::SetHintingAllFonts(Font::NativeHintingRange vectorRange, 
                                              Font::NativeHintingRange rasterRange, 
                                              unsigned maxVectorHintedSize,
                                              unsigned maxRasterHintedSize)
{
    NativeHinting[0].VectorRange         = vectorRange;
    NativeHinting[0].RasterRange         = rasterRange;
    NativeHinting[0].MaxVectorHintedSize = maxVectorHintedSize;
    NativeHinting[0].MaxRasterHintedSize = maxRasterHintedSize;
}

//------------------------------------------------------------------------
Font* FontProviderWin32::CreateFont(const char* name, unsigned fontFlags)
{
//if (fontFlags & Font::FF_BoldItalic) return 0; // DBG

    // Mask flags to be safe.
    fontFlags &= Font::FF_CreateFont_Mask;
    fontFlags |= Font::FF_DeviceFont | Font::FF_NativeHinting;

    // Return a newly created font with a default RefCount of 1.
    // It is users responsibility to cache the font and release
    // it when it is no longer necessary.
    ExternalFontWin32* pnewFont =
        new ExternalFontWin32(this, &SysData, name, fontFlags);
    if (pnewFont && !pnewFont->IsValid())
    {
        pnewFont->Release();
        return 0;
    }

    Font::NativeHintingType* pnh = findNativeHinting(name);
    if (pnh)
        pnewFont->SetHinting(*pnh);
    else
        pnewFont->SetHinting(NativeHinting[0]);

    return pnewFont;
}

//------------------------------------------------------------------------
static int CALLBACK LoadFontNamesProc(
                               ENUMLOGFONTEXW *lpelfe, // logical-font data
                               NEWTEXTMETRICEXW*, // physical-font data
                               DWORD, // type of font
                               LPARAM lParam // application-defined data
                               )
{
    StringHash<String>* pfontnames = static_cast<StringHash<String>*>((void*)lParam);
    String fontname(lpelfe->elfFullName);
    pfontnames->Set(fontname, fontname);
    return 1;
}

void FontProviderWin32::LoadFontNames(StringHash<String>& fontnames)
{
    LOGFONTW lf;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfFaceName[0] = '\0';
    lf.lfPitchAndFamily = 0;
    EnumFontFamiliesExW(SysData.WinHDC, &lf, (FONTENUMPROCW)LoadFontNamesProc, (LPARAM)&fontnames, 0);
}
#endif
}} //namespace Scaleform { namespace GFx {
