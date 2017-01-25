/**********************************************************************

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

#include "Text_FilterDesc.h"
#include "Kernel/SF_Debug.h"
#include "Text_Core.h"

namespace Scaleform { namespace Render { namespace Text {

void TextFilter::SetDefaultShadow()
{
    BlurX = 0;
    BlurY = 0;
    BlurStrength   = 1.0f;
    ShadowFlags    = FilterDesc::FineBlur;
    ShadowBlurX    = 0;
    ShadowBlurY    = 0;
    ShadowStrength = 1.0f;
    ShadowAlpha    = 255;
    ShadowAngle    = 450;
    ShadowDistance = (SInt16)Text::PixelsToFixp(4);
    ShadowOffsetX  = 57;  // In Twips, default 57 twips (cos(45) * 4 * 20)
    ShadowOffsetY  = 57;  // In Twips, default 57 twips (sin(45) * 4 * 20)
    ShadowColor    = 0;   // Color with alpha
}

// Loads a filter desc into a text filter object
//------------------------------------------------------------------------
void TextFilter::LoadFilterDesc(const FilterDesc& filter)
{
    if ((filter.Flags & 0xF) == FilterDesc::Filter_Blur)
    {
        BlurX        = filter.BlurX;
        BlurY        = filter.BlurY;
        BlurStrength = filter.Strength;
        return;
    }
    else if ((filter.Flags & 0xF) == FilterDesc::Filter_DropShadow ||
        (filter.Flags & 0xF) == FilterDesc::Filter_Glow)
    {
        ShadowFlags    = filter.Flags & 0xF0;
        ShadowBlurX    = filter.BlurX;
        ShadowBlurY    = filter.BlurY;
        ShadowStrength = filter.Strength;
        ShadowAlpha    = filter.ColorV.GetAlpha();
        ShadowAngle    = filter.Angle;
        ShadowDistance = filter.Distance;
        ShadowOffsetX  = 0;
        ShadowOffsetY  = 0;
        ShadowColor    = filter.ColorV;
        UpdateShadowOffset();
        return;
    }
}
}}} // Scaleform::Render::Text
