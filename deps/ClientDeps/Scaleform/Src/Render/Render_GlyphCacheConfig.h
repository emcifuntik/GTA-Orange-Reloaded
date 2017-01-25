/**********************************************************************

PublicHeader:   Render
Filename    :   Render_GlyphCacheConfig.h
Content     :   GlyphCache parameters and configuration API
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Render_GlyphCacheConfig_H
#define INC_SF_Render_GlyphCacheConfig_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
// ***** GlyphCacheParams

struct GlyphCacheParams
{
    unsigned TextureWidth;
    unsigned TextureHeight; 
    unsigned NumTextures; 
    unsigned MaxSlotHeight; 
    unsigned SlotPadding;
    unsigned TexUpdWidth;
    unsigned TexUpdHeight;
    float    MaxRasterScale;
    unsigned MaxVectorCacheSize;
    float    FauxItalicAngle;
    float    FauxBoldRatio;
    float    OutlineRatio;
    float    SharpeningFactor;

    GlyphCacheParams() : 
        TextureWidth(1024),
        TextureHeight(1024),

        NumTextures(1),
        MaxSlotHeight(48),
        SlotPadding(2),
        TexUpdWidth(256),
        TexUpdHeight(512),
        MaxRasterScale(1.0f),
        MaxVectorCacheSize(500),
        FauxItalicAngle(0.3f),
        FauxBoldRatio(0.055f),
        OutlineRatio(0.01f),
        SharpeningFactor(1.4f)
    {}

    GlyphCacheParams(const GlyphCacheParams& src) : 
        TextureWidth(src.TextureWidth),
        TextureHeight(src.TextureHeight),
        NumTextures(src.NumTextures),
        MaxSlotHeight(src.MaxSlotHeight),
        SlotPadding(src.SlotPadding),
        TexUpdWidth(src.TexUpdWidth),
        TexUpdHeight(src.TexUpdHeight),
        MaxRasterScale(src.MaxRasterScale),
        MaxVectorCacheSize(src.MaxVectorCacheSize),
        FauxItalicAngle(src.FauxItalicAngle),
        FauxBoldRatio(src.FauxBoldRatio),
        OutlineRatio(src.OutlineRatio),
        SharpeningFactor(src.SharpeningFactor)
    {}
};


//------------------------------------------------------------------------
// ***** GlyphCacheConfig

// GlyphCacheConfig defined external configuration API for GlyphCache.

class GlyphCacheConfig
{   
public:
    virtual ~GlyphCacheConfig() {}

    // Sets new GlyphCache parameters, re-creating texture buffers if
    // cache is already initialized. Returns if false buffer resize failed.
    // Note that some parameters applied may be modified by HAL/Renderer to
    // match its capabilities.
    virtual bool    SetParams(const GlyphCacheParams& params) = 0;

    // Returns last applied GlyphCacheParams value, potentially adjusted
    // based on HAL/Renderer capabilities. If SetParams wasn't called,    
    // default values used are returned.
    virtual const GlyphCacheParams& GetParams() const = 0;

    // Clears glyph cache.
    virtual void    ClearCache() = 0;
};

}}

#endif
