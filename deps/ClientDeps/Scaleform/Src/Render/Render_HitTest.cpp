/**********************************************************************

Filename    :   Render_HitTest.cpp
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

#include "../Kernel/SF_Alg.h"
#include "Render_Math2D.h"
#include "Render_HitTest.h"

namespace Scaleform { namespace Render { 

namespace Math2D {

//--------------------------------------------------------------------
bool CheckMonoCurveIntersection(float x1, float y1, float x2, float y2,
                                float x3, float y3, float x,  float y)
{
    // Check the Y-monotone quadratic curve for the intersection 
    // with a horizontal ray from (x,y) to the left. First check Y.
    //----------------------
    if (y >= y1 && y < y3) // Conditions >= && < are IMPORTANT!
    {
        // Early out. Check if all tree edges (triangle) lie on the left 
        // or on the right. First means "definitely no intersection", 
        // second - "definitely there is an intersection".
        // It's OK to use bitwise expressions rather than logical 
        // as potentially more efficient.
        //-----------------------
        unsigned cp1 = CrossProduct(x1, y1, x2, y2, x, y) > 0;
        unsigned cp2 = CrossProduct(x2, y2, x3, y3, x, y) > 0;
        unsigned cp3 = CrossProduct(x1, y1, x3, y3, x, y) > 0;

        if (cp1 & cp2 & cp3)             // cp1>0  && cp2>0  && cp3>0, on the right
            return true;

        if ((cp1^1) & (cp2^1) & (cp3^1)) // cp1<=0 && cp2<=0 && cp3<=0, on the left
            return false;

        // Intermediate case, the point is inside the triangle. 
        // Calculate the real intersection point between the curve and 
        // the horizontal line at Y. Then check if the intersection point 
        // lies on the left of the given x.
        //-----------------------
        float den = y1 - 2*y2 + y3;
        float t = -1;
        if(den == 0)
        {
            den = y3 - y1;
            if (den != 0)
                t = (y - y1) / den;
        }
        else
        {
            // The initial expression is: 
            // t = (sqrtf(-y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3) + y1 - y2) / den;
            // Theoretically the expression under the sqrtf 
            // (-y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3) must never be negative. 
            // However, it may occur because of finite precision of 
            // float/double. So that, it's a good idea to clamp the value to ]0...oo]
            //------------------------
            t = -y1 * (y3 - y) + y2*y2 - 2*y*y2 + y*y3;
            t = (t > 0) ? sqrtf(t) : 0;
            t = (t + y1 - y2) / den;
        }
        return CalcPointOnQuadCurve1D(x1, x2, x3, t) < x;
    }
    return false;
}

//--------------------------------------------------------------------
bool CheckCurveIntersection(float x1, float y1, float x2, float y2,
                            float x3, float y3, float x,  float y) 
{
    if(y2 >= y1 && y2 <= y3)
    {
        // A simple (Y-monotone) case
        //-----------------
        return CheckMonoCurveIntersection(x1, y1, x2, y2, x3, y3, x, y);
    }

    // The curve has a Y-extreme. Subdivide it at the 
    // extreme point and process separately.
    //--------------------
    float dy = (2*y2 - y1 - y3);
    float ty = (dy == 0) ? -1 : (y2 - y1) / dy;

    // Subdivide the curves they needs to be normalized 
    // (y1 must be less than y2). To do so it's theoretically
    // enough to check only one of (c1.y1, c1.y3) or (c2.y1, c2.y3),
    // but in practice it's better to check them both for the sake
    // of numerical stability.
    //--------------------
    QuadCoordType c1, c2;
    SubdivideQuadCurve(x1, y1, x2, y2, x3, y3, ty, &c1, &c2);
    if(c1.y1 > c1.y3)
    {
        Alg::Swap(c1.x1, c1.x3);
        Alg::Swap(c1.y1, c1.y3);
    }
    if(c2.y1 > c2.y3)
    {
        Alg::Swap(c2.x1, c2.x3);
        Alg::Swap(c2.y1, c2.y3);
    }

    // The results of these calls must differ. That is, if the hit-test point 
    // lies on the left or on the right of both sub-curves, it definitely
    // means that the curve is not contributing (0 or 2 intersections).
    // Different results mean that the point lies between the sub-curves
    // ("inside" the original curve).
    //-------------------
    return CheckMonoCurveIntersection(c1.x1, c1.y1, c1.x2, c1.y2, c1.x3, c1.y3, x, y) != 
           CheckMonoCurveIntersection(c2.x1, c2.y1, c2.x2, c2.y2, c2.x3, c2.y3, x, y);
}


} // Math2D

//--------------------------------------------------------------------
bool HitTestRoundStroke(const VertexPath& path, float w, float x, float y)
{
    unsigned i, j;
    w /= 2;

    for(i = 0; i < path.GetNumPaths(); ++i)
    {
        const PathBasic& p = path.GetPath(i);
        for(j = 1; j < p.Count; ++j)
        {
            const VertexPath::VertexType& v1 = path.GetVertex(p.Start + j-1);
            const VertexPath::VertexType& v2 = path.GetVertex(p.Start + j);
            float dx = v2.x - v1.x;
            float dy = v2.y - v1.y;
            VertexPath::VertexType p1 = { v1.x - dy, v1.y + dx };
            VertexPath::VertexType p2 = { v2.x - dy, v2.y + dx };
            if(Math2D::CrossProduct(v1.x, v1.y, p1.x, p1.y, x, y) >= 0 &&
               Math2D::CrossProduct(v2.x, v2.y, p2.x, p2.y, x, y) <= 0 &&
               Math2D::LinePointDistanceAbs(v1.x, v1.y, v2.x, v2.y, x, y) <= w)
            {
                return true;
            }
        }
    }

    w *= w;
    for(i = 0; i < path.GetNumVertices(); ++i)
    {
        const VertexPath::VertexType& v1 = path.GetVertex(i);
        if (Math2D::SqDistance(v1.x, v1.y, x, y) <= w)
        {
            return true;
        }
    }
    return false;
}


//--------------------------------------------------------------------
bool HitTestGeneralStroke(const VertexPath& path, float x, float y)
{
    int styleCount = 0;
    unsigned i, j;

    for(i = 0; i < path.GetNumPaths(); ++i)
    {
        const PathBasic& p = path.GetPath(i);
        for(j = 1; j < p.Count; ++j)
        {
            VertexPath::VertexType p1 = path.GetVertex(p.Start + j-1);
            VertexPath::VertexType p2 = path.GetVertex(p.Start + j);
            if(p1.y != p2.y)
            {
                int dir = 1;
                if(p1.y > p2.y)
                {
                    Alg::Swap(p1, p2);
                   dir = -1;
                }
                if(y >= p1.y && y < p2.y &&
                   Math2D::CrossProduct(p1.x, p1.y, p2.x, p2.y, x, y) > 0)
                {
                    styleCount += dir;
                }
            }
        }
    }
    return styleCount != 0;
}



}} // Scaleform::Render

