/**********************************************************************
PublicHeader:   Render
Filename    :   FilterDesc.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Render_Text_FilterDesc_H
#define INC_SF_Render_Text_FilterDesc_H

#include <math.h>
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Stats.h"
#include "Render/Render_Color.h"

namespace Scaleform { namespace Render { namespace Text {

//------------------------------------------------------------------------
struct FilterDesc
{
    enum { MaxFilters = 4 };

    enum FilterType
    {
        Filter_DropShadow   = 0,
        Filter_Blur         = 1,
        Filter_Glow         = 2,
        Filter_Bevel        = 3,
        Filter_GradientGlow = 4,
        Filter_Convolution  = 5,
        Filter_AdjustColor  = 6,
        Filter_GradientBevel= 7,
    };

    enum FlagsType
    {
        KnockOut   = 0x20,
        HideObject = 0x40,
        FineBlur   = 0x80
    };

    unsigned    Flags;      // 0..3 - FilterType, 4..7 - Flags (GFxGlyphParam::Flags)
    float       BlurX;      // In Twips
    float       BlurY;      // In Twips
    float       Strength;
    SInt16      Angle;      // In 1/10 degree
    SInt16      Distance;   // In Twips
    Color       ColorV;     // Color with alpha

    void Clear()
    {
        Flags = 0;
        BlurX = BlurY = 0;
        Strength = 1.0f;
        Angle = 0;
        Distance = 0;
        ColorV = 0;
    }
};


//------------------------------------------------------------------------
struct TextFilter : public RefCountBaseNTS<TextFilter, StatRender_Text_Mem>
{
    float                           BlurX;            // In Twips
    float                           BlurY;            // In Twips
    float                           BlurStrength;
    unsigned                        ShadowFlags;
    float                           ShadowBlurX;      // In Twips
    float                           ShadowBlurY;      // In Twips
    float                           ShadowStrength;
    UInt8                           ShadowAlpha;
    SInt16                          ShadowAngle;      // In 1/10 degree
    SInt16                          ShadowDistance;   // In Twips
    SInt16                          ShadowOffsetX;    // In Twips, default 57 twips (cos(45) * 4 * 20)
    SInt16                          ShadowOffsetY;    // In Twips, default 57 twips (sin(45) * 4 * 20)
    Color                           ShadowColor;      // Color with alpha

    TextFilter()
    {
        SetDefaultShadow();
    }

    void SetDefaultShadow();

    void UpdateShadowOffset()
    {
        float a = float(ShadowAngle) * 3.1415926535897932384626433832795f / 1800.0f;
        ShadowOffsetX = SInt16(cosf(a) * ShadowDistance);
        ShadowOffsetY = SInt16(sinf(a) * ShadowDistance);
    }

    void LoadFilterDesc(const FilterDesc& filter);
};


}}} // Scaleform::Render::Text

#endif // INC_SF_Render_Text_FilterDesc_H
