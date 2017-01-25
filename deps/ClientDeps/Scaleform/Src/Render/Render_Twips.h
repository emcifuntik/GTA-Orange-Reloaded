/*****************************************************************

PublicHeader:   Render
Filename    :   Render_Twips.h
Content     :   
Created     :   January, 2010
Authors     :   Artem Bolgar
Copyright   :   (c) 1999-2010 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Render_Twips_H
#define INC_SF_Render_Twips_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace Render {

template <typename T> T TwipsToPixels(T x) { return T((x) / T(20.)); }
template <typename T> T PixelsToTwips(T x) { return T((x) * T(20.)); }

// Specializations to use mult instead of div op
template <> inline float  TwipsToPixels(float x)  { return x * 0.05f; }
template <> inline double TwipsToPixels(double x) { return x * 0.05;  }

inline RectF TwipsToPixels(const RectF& x) 
{ 
    return RectF(TwipsToPixels(x.x1),  TwipsToPixels(x.y1), 
                 TwipsToPixels(x.x2),  TwipsToPixels(x.y2));
}

inline RectF PixelsToTwips(const RectF& x) 
{ 
    return RectF(PixelsToTwips(x.x1),  PixelsToTwips(x.y1), 
                 PixelsToTwips(x.x2),  PixelsToTwips(x.y2));
}

// Necessary for now
#define SF_TWIPS_TO_PIXELS(x)  TwipsToPixels(float(x))
#define SF_PIXELS_TO_TWIPS(x)  PixelsToTwips(float(x))

template <typename T>
int RoundTwips(T v)
{
    return Alg::IRound(v);
}

template <typename T>
int AlignTwipsToPixel(T v)
{
    return Alg::IRound(v);
}

}} // Scaleform::Render

#endif
