/**********************************************************************

Filename    :   FT2Helper.cpp
Content     :   Helper for FreeType2 font provider
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

#include "GFx/GFx_FT2Helper.h"
#include "GFx/GFx_Shape.h"
#include "Render/Render_Math2D.h"

namespace Scaleform { namespace GFx {

static const Render::CoordType GFxExternalFontFT2_CubicTolerance = 2.0f;

namespace Math2D
{
    // The code of Cubic to Quadratic approximation was taken from the 
    // Anti-Grain Geometry research works and modified for the use by Scaleform. 
    // Permission to use without restrictions is hereby granted to 
    // Scaleform Corporation by the author of Anti-Grain Geometry Project.
    // See http://antigtain.com for details.
    //-----------------------------------------------------------------------




    //-----------------------------------------------------------------------
    template<class Path> 
    void SubdivideCubicToQuadratic(
        Render::CoordType x1, Render::CoordType y1,
        Render::CoordType x2, Render::CoordType y2,
        Render::CoordType x3, Render::CoordType y3,
        Render::CoordType x4, Render::CoordType y4, 
        Path& path, Render::CoordType tolerance)
    {
        Render::CoordType xc;
        Render::CoordType yc;
        if(!Render::Math2D::Intersection(x1, y1, x2, y2, x3, y3, x4, y4, &xc, &yc, tolerance))
        {
            xc = (x2 + x3) / 2;
            yc = (y2 + y3) / 2;
        }

        Render::CoordType x12   = (x1 + xc ) / 2;
        Render::CoordType y12   = (y1 + yc ) / 2;
        Render::CoordType x23   = (xc + x4 ) / 2;
        Render::CoordType y23   = (yc + y4 ) / 2;
        Render::CoordType xq    = (x12+ x23) / 2;
        Render::CoordType yq    = (y12+ y23) / 2;

        x12   = (x1  + x2  ) / 2;
        y12   = (y1  + y2  ) / 2;
        x23   = (x2  + x3  ) / 2;
        y23   = (y2  + y3  ) / 2;
        Render::CoordType x34   = (x3  + x4  ) / 2;
        Render::CoordType y34   = (y3  + y4  ) / 2;
        Render::CoordType x123  = (x12 + x23 ) / 2;
        Render::CoordType y123  = (y12 + y23 ) / 2;
        Render::CoordType x234  = (x23 + x34 ) / 2;
        Render::CoordType y234  = (y23 + y34 ) / 2;
        Render::CoordType x1234 = (x123+ x234) / 2;
        Render::CoordType y1234 = (y123+ y234) / 2;

        Render::CoordType d;
        d = fabsf(fabsf(Render::Math2D::LinePointDistance(x1, y1, x4, y4, xq, yq)) - 
            fabsf(Render::Math2D::LinePointDistance(x1, y1, x4, y4, x1234, y1234))) +
            fabsf(Render::Math2D::LinePointDistance(x123, y123, x234, y234, xq, yq));


        if(d < tolerance)
        {
            path.CurveToAbs(int(xc), int(yc), int(x4), int(y4));
            return;
        }
        SubdivideCubicToQuadratic(x1, y1, x12, y12, x123, y123, x1234, y1234, path, tolerance);
        SubdivideCubicToQuadratic(x1234, y1234, x234, y234, x34, y34, x4, y4, path, tolerance);
    }



    //-----------------------------------------------------------------------
    struct CubicCoordType
    {
        Render::CoordType x1, y1, x2, y2, x3, y3, x4, y4;
    };


    //-----------------------------------------------------------------------
    template<class CurveType>
    void SubdivideCubicCurve(
        const CurveType& c, Render::CoordType t, CurveType* c1, CurveType* c2)
    {
        // Local variables are necessary in case of reuse curve c,
        // that is, when c1 or c2 points to the same physical c.
        //---------------------
        Render::CoordType x1    = c.x1;
        Render::CoordType y1    = c.y1;
        Render::CoordType x12   = c.x1 + t*(c.x2 - c.x1);
        Render::CoordType y12   = c.y1 + t*(c.y2 - c.y1);
        Render::CoordType x23   = c.x2 + t*(c.x3 - c.x2);
        Render::CoordType y23   = c.y2 + t*(c.y3 - c.y2);
        Render::CoordType x34   = c.x3 + t*(c.x4 - c.x3);
        Render::CoordType y34   = c.y3 + t*(c.y4 - c.y3);
        Render::CoordType x123  = x12  + t*(x23  - x12 );
        Render::CoordType y123  = y12  + t*(y23  - y12 );
        Render::CoordType x234  = x23  + t*(x34  - x23 );
        Render::CoordType y234  = y23  + t*(y34  - y23 );
        Render::CoordType x1234 = x123 + t*(x234 - x123);
        Render::CoordType y1234 = y123 + t*(y234 - y123);
        Render::CoordType x4    = c.x4;
        Render::CoordType y4    = c.y4;
        c1->x1 = x1; 
        c1->y1 = y1;
        c1->x2 = x12; 
        c1->y2 = y12; 
        c1->x3 = x123;
        c1->y3 = y123;
        c1->x4 = x1234;
        c1->y4 = y1234;
        c2->x1 = x1234;
        c2->y1 = y1234; 
        c2->x2 = x234;
        c2->y2 = y234; 
        c2->x3 = x34; 
        c2->y3 = y34;
        c2->x4 = x4;
        c2->y4 = y4;
    }

    //-----------------------------------------------------------------------
    template<class Path> 
    void CubicToQuadratic(
        Render::CoordType x1, Render::CoordType y1,
        Render::CoordType x2, Render::CoordType y2,
        Render::CoordType x3, Render::CoordType y3,
        Render::CoordType x4, Render::CoordType y4, 
        Path& path, 
        Render::CoordType tolerance)
    {
        Render::CoordType ax  =   -x1 + 3*x2 - 3*x3 + x4;
        Render::CoordType ay  =   -y1 + 3*y2 - 3*y3 + y4;
        Render::CoordType bx  =  3*x1 - 6*x2 + 3*x3;
        Render::CoordType by  =  3*y1 - 6*y2 + 3*y3;
        Render::CoordType cx  = -3*x1 + 3*x2;
        Render::CoordType cy  = -3*y1 + 3*y2;
        Render::CoordType den = ay*bx - ax*by;

        Render::CoordType t1 = -1;
        Render::CoordType t2 = -1;

        if (den != 0)
        {
            Render::CoordType tc = -0.5f * (ay*cx - ax*cy) / den;
            Render::CoordType d  = sqrtf(tc*tc - (by*cx - bx*cy) / (3*den));
            t1 = tc - d;
            t2 = tc + d;
        }

        unsigned numSubcurves = 1;
        CubicCoordType cc;
        CubicCoordType sc[3];
        cc.x1 = x1; cc.y1 = y1;
        cc.x2 = x2; cc.y2 = y2;
        cc.x3 = x3; cc.y3 = y3;
        cc.x4 = x4; cc.y4 = y4;

        switch(int(t2 > 0 && t2 < 1) * 2 + int(t1 > 0 && t1 < 1))
        {
        case 0:
            sc[0] = cc;
            numSubcurves = 1;
            break;

        case 1:
            SubdivideCubicCurve(cc, t1, &sc[0], &sc[1]);
            numSubcurves = 2;
            break;

        case 2:
            SubdivideCubicCurve(cc, t2, &sc[0], &sc[1]);
            numSubcurves = 2;
            break;

        case 3:
            if(t2 < t1) 
            {
                Render::CoordType t = t1; t1 = t2; t2 = t;
            }
            SubdivideCubicCurve(cc, t1, &sc[0], &sc[1]);
            SubdivideCubicCurve(sc[1], (t2 - t1) / (1 - t1), &sc[1], &sc[2]);
            numSubcurves = 3;
            break;
        }

        unsigned i;
        for(i = 0; i < numSubcurves; ++i)
        {
            const CubicCoordType& c = sc[i];
            SubdivideCubicToQuadratic(c.x1, c.y1, c.x2, c.y2, c.x3, c.y3, c.x4, c.y4, path, tolerance);
        }
    }

} // namespace GMath2D

void FT2Helper::cubicToQuadratic(
    PathPacker& path, 
    int hintedGlyphSize, 
    unsigned fontHeight, 
    int x2, int y2, int x3, int y3, int x4, int y4)
{
    int x1, y1;
    path.GetLastVertex(&x1, &y1);
    Render::CoordType k = 1;
    if (hintedGlyphSize)
    {
        x2 = FtToTwips(x2); y2 = FtToTwips(y2);
        x3 = FtToTwips(x3); y3 = FtToTwips(y3);
        x4 = FtToTwips(x4); y4 = FtToTwips(y4);
        k  = Render::CoordType(hintedGlyphSize * 20) / Render::CoordType(fontHeight);
    }
    else
    {
        x2 = FtToS1024(x2); y2 = FtToS1024(y2);
        x3 = FtToS1024(x3); y3 = FtToS1024(y3);
        x4 = FtToS1024(x4); y4 = FtToS1024(y4);
    }
    Math2D::CubicToQuadratic(
        Render::CoordType(x1), Render::CoordType(y1), 
        Render::CoordType(x2), Render::CoordType(y2),
        Render::CoordType(x3), Render::CoordType(y3),
        Render::CoordType(x4), Render::CoordType(y4),
        path, GFxExternalFontFT2_CubicTolerance * k);
}



bool FT2Helper::decomposeGlyphOutline(const FTOutline& outline, ShapeDataBase* shape, unsigned fontHeight)
{   
    FTVector   v_last;
    FTVector   v_control;
    FTVector   v_start;

    PathPacker path;

    bool hinted = shape->GetHintedGlyphSize() > 0;

    FTVector*  point;
    FTVector*  limit;
    char*		  tags;

    int  n;         // index of contour in outline
    int  first = 0; // index of first point in contour
    char tag;       // current point's state
    bool shapeValid = false;

    for (n = 0; n < outline.NContours; n++)
    {
        int  last;  // index of last point in contour

        last  = outline.Contours[n];
        limit = outline.Points + last;

        v_start = outline.Points[first];
        v_last  = outline.Points[last];

        v_control = v_start;

        point = outline.Points + first;
        tags  = outline.Tags  + first;
        tag   = GetCurveTag(tags[0]);

        // A contour cannot start with a cubic control point!
        if(tag == GFxFTCurveTagCubic) return false;

        // check first point to determine origin
        if (tag == GFxFTCurveTagConic)
        {
            // first point is conic control.  Yes, this happens.
            if (GetCurveTag(outline.Tags[last]) == GFxFTCurveTagOn)
            {
                // start at last point if it is on the curve
                v_start = v_last;
                limit--;
            }
            else
            {
                // if both first and last points are conic,
                // start at their middle and record its position
                // for closure
                v_start.x = (v_start.x + v_last.x) / 2;
                v_start.y = (v_start.y + v_last.y) / 2;

                v_last = v_start;
            }
            point--;
            tags--;
        }

        path.Reset();
        path.SetFill0(1);
        path.SetFill1(0);
        if (hinted)
            path.SetMoveTo(FtToTwips(v_start.x), -FtToTwips(v_start.y));
        else
            path.SetMoveTo(FtToS1024(v_start.x), -FtToS1024(v_start.y));

        while (point < limit)
        {
            point++;
            tags++;

            tag = GetCurveTag(tags[0]);
            switch(tag)
            {
            case GFxFTCurveTagOn:  // emit a single line_to
                {
                    if (hinted)
                        path.LineToAbs(FtToTwips(point->x), -FtToTwips(point->y));
                    else
                        path.LineToAbs(FtToS1024(point->x), -FtToS1024(point->y));
                    continue;
                }

            case GFxFTCurveTagConic:  // consume conic arcs
                {
                    v_control.x = point->x;
                    v_control.y = point->y;

Do_Conic:
                    if (point < limit)
                    {
                        FTVector vec;
                        FTVector v_middle;

                        point++;
                        tags++;
                        tag = GetCurveTag(tags[0]);

                        vec.x = point->x;
                        vec.y = point->y;

                        if(tag == GFxFTCurveTagOn)
                        {
                            if (hinted)
                                path.CurveToAbs(FtToTwips(v_control.x), -FtToTwips(v_control.y),
                                FtToTwips(vec.x),       -FtToTwips(vec.y));
                            else
                                path.CurveToAbs(FtToS1024(v_control.x), -FtToS1024(v_control.y),
                                FtToS1024(vec.x),       -FtToS1024(vec.y));

                            continue;
                        }

                        if (tag != GFxFTCurveTagConic) return false;

                        v_middle.x = (v_control.x + vec.x) / 2;
                        v_middle.y = (v_control.y + vec.y) / 2;
                        if (hinted)
                            path.CurveToAbs(FtToTwips(v_control.x), -FtToTwips(v_control.y),
                            FtToTwips(v_middle.x),  -FtToTwips(v_middle.y));
                        else
                            path.CurveToAbs(FtToS1024(v_control.x), -FtToS1024(v_control.y),
                            FtToS1024(v_middle.x),  -FtToS1024(v_middle.y));

                        v_control = vec;
                        goto Do_Conic;
                    }
                    if (hinted)
                        path.CurveToAbs(FtToTwips(v_control.x), -FtToTwips(v_control.y),
                        FtToTwips(v_start.x),   -FtToTwips(v_start.y));
                    else
                        path.CurveToAbs(FtToS1024(v_control.x), -FtToS1024(v_control.y),
                        FtToS1024(v_start.x),   -FtToS1024(v_start.y));
                    goto Close;
                }

            default:  // GFxFTCurveTagCubic
                {
                    FTVector vec1, vec2;
                    if (point + 1 > limit || GetCurveTag(tags[1]) != GFxFTCurveTagCubic)
                    {
                        return false;
                    }

                    vec1.x = point[0].x; 
                    vec1.y = point[0].y;
                    vec2.x = point[1].x; 
                    vec2.y = point[1].y;

                    point += 2;
                    tags  += 2;

                    if (point <= limit)
                    {
                        FTVector vec;
                        vec.x = point->x;
                        vec.y = point->y;
                        cubicToQuadratic(path, shape->GetHintedGlyphSize(), fontHeight, 
                            vec1.x, -vec1.y, vec2.x, -vec2.y, vec.x,  -vec.y);
                        continue;
                    }
                    cubicToQuadratic(path, shape->GetHintedGlyphSize(), fontHeight, 
                        vec1.x, -vec1.y, vec2.x, -vec2.y, v_start.x, -v_start.y);
                    goto Close;
                }
            }
        }

Close:
        if (!path.IsEmpty())
        {
            path.ClosePath();
            static_cast<ShapeNoStyles*>(shape)->AddPath(&path);
            shapeValid = true;
        }
        first = last + 1; 
    }
    return shapeValid;
}

ShapeDataBase* FT2Helper::CreateShape(unsigned shapePageSize, unsigned glyphSize)
{
    ShapeNoStyles* pshape = 	new ShapeNoStyles(shapePageSize);
    pshape->SetHintedGlyphSize(glyphSize);
    return pshape;
}

void FT2Helper::ReleaseShape(ShapeDataBase* pshape)
{
    pshape->Release();
}
}} //namespace Scaleform { namespace GFx {
