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

#include "GFx/GFx_Stream.h"
#include "GFx/GFx_FilterDesc.h"
#include "Kernel/SF_Debug.h"

namespace Scaleform { namespace GFx {

using Render::Text::FilterDesc;
using Render::Text::TextFilter;

// ***** Tag Loaders implementation


/*
    struct swf_filter_drop_shadow 
    {
        swf_filter_type     f_type;     // 0
        swf_rgba            f_rgba;
        signed long fixed   f_blur_horizontal;
        signed long fixed   f_blur_vertical;
        signed long fixed   f_radian_angle;
        signed long fixed   f_distance;
        signed short fixed  f_strength;
        unsigned            f_inner_shadow     : 1;
        unsigned            f_knock_out        : 1;
        unsigned            f_composite_source : 1;
        unsigned            f_reserved1        : 1;
        unsigned            f_reserved2        : 4;
    };


    struct swf_filter_blur 
    {
        swf_filter_type     f_type;     // 1 
        unsigned long fixed f_blur_horizontal;
        unsigned long fixed f_blur_vertical;
        unsigned            f_passes   : 5;
        unsigned            f_reserved : 3;
    };


    struct swf_filter_glow 
    {
        swf_filter_type     f_type;     // 2
        swf_rgba            f_rgba;
        signed long fixed   f_blur_horizontal;
        signed long fixed   f_blur_vertical;
        signed short fixed  f_strength;
        unsigned            f_inner_shadow     : 1;
        unsigned            f_knock_out        : 1;
        unsigned            f_composite_source : 1;
        unsigned            f_reserved1        : 1;
        unsigned            f_reserved2        : 4;
    };
*/


//------------------------------------------------------------------------
static inline float GFx_Fixed1616toFloat(UInt32 v)
{
    return float(v) / 65536.0f;
}

//------------------------------------------------------------------------
static inline float GFx_Fixed88toFloat(UInt16 v)
{
    return float(v) / 256.0f;
}

//------------------------------------------------------------------------
static inline SInt16 GFx_Radians1616toAngle(SInt32 v)
{
    float vf = float(v) / 65536.0f;
    return SInt16(fmodf(vf * 1800.0f / 3.1415926535897932384626433832795f, 3600.0f));
}

//------------------------------------------------------------------------
static inline SInt16 GFx_Fixed1616toTwips(SInt32 v)
{
    float vf = float(v) / 65536.0f;
    return SInt16(vf * 20.0f);
}

//------------------------------------------------------------------------
static inline UInt8 GFx_ParseFilterFlags(UInt8 f)
{
    UInt8 flags = 0;
    if  (f & 0x40)       flags |= FilterDesc::KnockOut;
    if ((f & 0x20) == 0) flags |= FilterDesc::HideObject;
    if ((f & 0x0F) > 1)  flags |= FilterDesc::FineBlur;
    return flags;
}

// Helper function used to load filters.
//------------------------------------------------------------------------
template <class T>
unsigned LoadFilters(T* ps, FilterDesc* filters, UPInt filterssz)
{
    UByte filterCount = ps->ReadU8();
    FilterDesc filterDesc;
    unsigned numFilters = 0;

    // Flag to notify that output filter buffer is full
    // Continue reading file, but do not store the extra filters
    bool done = false;

    while(filterCount--)
    {
        FilterDesc::FilterType filter = (FilterDesc::FilterType)ps->ReadU8();
        unsigned numBytes = 0;

        filterDesc.Clear();
        filterDesc.Flags = UInt8(filter);

        switch(filter)
        {
        case FilterDesc::Filter_DropShadow: // 23 bytes
            ps->ReadRgba(&filterDesc.ColorV);
            filterDesc.BlurX    = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.BlurY    = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.Angle    = GFx_Radians1616toAngle((UInt32)ps->ReadU32());
            filterDesc.Distance = GFx_Fixed1616toTwips((UInt32)ps->ReadU32());
            filterDesc.Strength = GFx_Fixed88toFloat((UInt16)ps->ReadU16());
            filterDesc.Flags   |= GFx_ParseFilterFlags((UByte)ps->ReadU8());
            if (!done && filters && numFilters < FilterDesc::MaxFilters) 
                filters[numFilters++] = filterDesc;
            break;

        case FilterDesc::Filter_Blur: // 9 bytes
            filterDesc.BlurX  = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.BlurY  = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.Flags |= ((ps->ReadU8() >> 3) > 1) ? FilterDesc::FineBlur : 0;
            if (!done && filters && numFilters < FilterDesc::MaxFilters) 
                filters[numFilters++] = filterDesc;
            break;

        case FilterDesc::Filter_Glow: // 15 bytes
            ps->ReadRgba(&filterDesc.ColorV);
            filterDesc.BlurX    = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.BlurY    = PixelsToTwips<float>(GFx_Fixed1616toFloat((UInt32)ps->ReadU32()));
            filterDesc.Strength = GFx_Fixed88toFloat((UInt16)ps->ReadU16());
            filterDesc.Flags   |= GFx_ParseFilterFlags((UByte)ps->ReadU8());
            if (!done && filters && numFilters < FilterDesc::MaxFilters) 
                filters[numFilters++] = filterDesc;
            break;

        case FilterDesc::Filter_Bevel:
            numBytes = 27; 
            break;

        case FilterDesc::Filter_GradientGlow:
            numBytes = 19 + ps->ReadU8()*5; 
            break;

        case FilterDesc::Filter_Convolution:
            {
                unsigned cols = ps->ReadU8();
                unsigned rows = ps->ReadU8();
                numBytes = 4 + 4 + 4*cols*rows + 4 + 1;
            }
            break;

        case FilterDesc::Filter_AdjustColor:
            numBytes = 80; 
            break;

        case FilterDesc::Filter_GradientBevel:
            numBytes = 19 + ps->ReadU8()*5; 
            break;
        }

        // Skip filter or the rest of the filter
        while(numBytes--)
            ps->ReadU8();

        // Check if output buffer is full
        if (!done && numFilters == filterssz)
        {
            done = true;
            SF_DEBUG_WARNING1(1, "Attempt to load more than %d filters!", filterssz);
        }
    }
    return numFilters;
}
// Force concrete definitions inside .cpp
template unsigned LoadFilters(Stream* ps, FilterDesc* filters, UPInt filterssz);
template unsigned LoadFilters(StreamContext* ps, FilterDesc* filters, UPInt filterssz);

}} // Scaleform::GFx
