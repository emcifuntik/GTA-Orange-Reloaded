/**********************************************************************

PublicHeader:   FontProvider_FT2
Filename    :   FT2Helper.h
Content     :   Helper for FreeType2 font provider.
                Removes dependency on nonpublic headers from GFxFontProviderFT2 
Created     :   3/18/2009
Authors     :   Dmitry Polenur, Maxim Shemanarev

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

#ifndef INC_SF_GFX_FT2Helper_H
#define INC_SF_GFX_FT2Helper_H

#include "Kernel/SF_Types.h"

namespace Scaleform { namespace GFx {

class PathPacker;
class ShapeDataBase;

struct FTVector
{
    SInt32 x;
    SInt32 y;
};

struct  FTOutline
{
    SInt16      NContours;      // number of contours in glyph        
    SInt16      NPoints;        // number of points in the glyph      

    FTVector*   Points;         // the outline's points               
    char*		Tags;           // the points flags                   
    SInt16*     Contours;       // the contour end points             

    int			Flags;          // outline masks
};

// A helper class for GFxFontProviderFT2. Provides functionality to create shapes
// from FreeType's glyphs.
class FT2Helper
{
    enum 
    {
        GFxFTCurveTagOn			= 1,		
        GFxFTCurveTagConic		= 0,
        GFxFTCurveTagCubic		= 2
    };
public:
    static void SF_STDCALL cubicToQuadratic(PathPacker& path, 
                                 int hintedGlyphSize, 
                                 unsigned fontHeight, 
                                 int x2, int y2, int x3, int y3, int x4, int y4);
    static bool SF_STDCALL decomposeGlyphOutline(const FTOutline& outline, ShapeDataBase* shape, unsigned fontHeight);

    static ShapeDataBase* SF_STDCALL CreateShape(unsigned shapePageSize, unsigned glyphSize);
    static void SF_STDCALL ReleaseShape(ShapeDataBase* shape);

private:
    static inline int SF_STDCALL FtToTwips(int v)
    {
        return (v * 20) >> 6;
    }

    static inline int SF_STDCALL FtToS1024(int v)
    {
        return v >> 6;
    }

    static inline char SF_STDCALL GetCurveTag(char flag)
    {
        return flag & 3;
    }
};

}} // namespace Scaleform { namespace GFx {

#endif //#ifndef INC_SF_GFX_FT2Helper_H
