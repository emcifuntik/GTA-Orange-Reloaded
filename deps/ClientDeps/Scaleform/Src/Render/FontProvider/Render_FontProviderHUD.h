/**********************************************************************

PublicHeader:   Render
Filename    :   Render_FontProviderHUD.h
Content     :   Raster Font provider for HUDs
Created     :   2009
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

#ifndef INC_SF_Render_FontProviderHUD_H
#define INC_SF_Render_FontProviderHUD_H

#include "Render/Render_Font.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
class FontProviderHUD;

class ExternalFontHUD : public Font
{
public:
    ExternalFontHUD(FontProviderHUD *provider,
                    const char* name, unsigned fontFlags);

    virtual ~ExternalFontHUD();

    virtual const char* GetName() const;

    virtual int     GetGlyphIndex(UInt16 code);
    virtual float   GetAdvance(unsigned glyphIndex) const;
    virtual float   GetKerningAdjustment(unsigned, unsigned) const { return 0; }
    virtual float   GetGlyphWidth(unsigned glyphIndex) const;
    virtual float   GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;
    virtual bool    IsHintedVectorGlyph(unsigned, unsigned) const { return false; }
    virtual bool    IsHintedRasterGlyph(unsigned, unsigned) const { return true; }
    virtual const ShapeDataInterface* GetPermanentGlyphShape(unsigned) const { return 0; }
    virtual bool    GetTemporaryGlyphShape(unsigned, unsigned, GlyphShape*)  { return false; }
    virtual bool    GetGlyphRaster(unsigned glyphIndex, unsigned hintedSize, GlyphRaster* raster);

    // Custom functions (not overridden)
    static unsigned     GetMonoFontWidth();
    static unsigned     GetMonoFontHeight();
    static unsigned     GetMonoFontBaseLine();
    static const UByte* GetGlyphBits(UInt16 code);

private:
    static int getGlyphIndex(UInt16 code);

    Ptr<FontProviderHUD>    pFontProvider;
    Array<char>             Name;
    float                   Advance;
};



//------------------------------------------------------------------------
class FontProviderHUD : public FontProvider
{
public:
    FontProviderHUD();
    virtual ~FontProviderHUD();

    virtual Font*   CreateFont(const char* name, unsigned fontFlags);
    virtual void    LoadFontNames(StringHash<String>& fontnames);
};


}} // Scaleform::Render

#endif
