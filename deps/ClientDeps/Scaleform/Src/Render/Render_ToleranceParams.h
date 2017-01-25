/**********************************************************************

PublicHeader:   Render
Filename    :   Render_ToleranceParams.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/
#ifndef SF_Render_ToleranceParams_H
#define SF_Render_ToleranceParams_H

namespace Scaleform { namespace Render {

//--------------------------------------------------------------------
struct ToleranceParams
{
    float Epsilon;                  // Horizontal and vertical epsilon in screen space
    float CurveTolerance;           // Curve tolerance in screen space. 1.0 is approximately +/- 0.5 pix.
    float CollinearityTolerance;    // Tolerance in screen space, when the curves can be treated as line segments
    float IntersectionEpsilon;      // Epsilon to calculate intersections in screen space

    float FillLowerScale;           // Scale tolerances for EdgeAA
    float FillUpperScale;

    float FillAliasedLowerScale;    // Scale tolerances for non-EdgeAA
    float FillAliasedUpperScale;

    float StrokeLowerScale;         // Scale tolerances for regular strokes
    float StrokeUpperScale;

    float HintedStrokeLowerScale;   // Scale tolerances for hinted strokes
    float HintedStrokeUpperScale;

    float EdgeAAScale;              // Additional EdgeAA scale. Normally it's 1.0, but for more
                                    // consistent look it should be slightly less than 1.0. It 
                                    // significantly improves thin vector elements in UI-like
                                    // applications

    float MorphTolerance;           // Morphing ratio tolerance. Mostly used to eliminate inequity
                                    // of floats, when they are essentially equal

    ToleranceParams() : 
        Epsilon(1e-5f), 
        CurveTolerance(1.0f), 
        CollinearityTolerance(1.0f),
        IntersectionEpsilon(1e-3f),
        FillLowerScale(0.7071f),
        FillUpperScale(1.4142f),  
        FillAliasedLowerScale(0.5f),
        FillAliasedUpperScale(2.0f),  
        StrokeLowerScale(0.99f), // Default: 0.99
        StrokeUpperScale(1.01f), // Default: 1.01
        HintedStrokeLowerScale(0.999f),
        HintedStrokeUpperScale(1.001f),
        EdgeAAScale(0.95f),
        MorphTolerance(0.0001f)
    {}
};

}} // Scaleform::Render

#endif
