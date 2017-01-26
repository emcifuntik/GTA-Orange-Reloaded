/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Math2D.h
Content     :   
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   Some simple 2D geometry math functions and constants

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_Math2D_H
#define INC_SF_Render_Math2D_H

#include "Kernel/SF_Types.h"
#include "Render_ToleranceParams.h"

#ifdef SF_MATH_H
#include SF_MATH_H
#else
#include <math.h>
#endif

namespace Scaleform { namespace Render {

//---------------------------------------------------------------------------
typedef float CoordType;
const CoordType MinCoord = -1e30f;
const CoordType MaxCoord =  1e30f;

namespace Math2D
{

const CoordType Pi = (CoordType)3.14159265358979323846;

enum CurveRecursionLimitType
{
    CurveRecursionLimit = 12
};

//-----------------------------------------------------------------------
inline CoordType CrossProduct(CoordType x1, CoordType y1,
                              CoordType x2, CoordType y2,
                              CoordType x,  CoordType y)
{
    return (x - x2)*(y2 - y1) - (y - y2)*(x2 - x1);
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType CrossProduct(const V1& v1, const V2& v2, const V3& v)
{
    return (v.x - v2.x)*(v2.y - v1.y) - (v.y - v2.y)*(v2.x - v1.x);
}

//-----------------------------------------------------------------------
inline CoordType AbsCrossProduct(CoordType x1, CoordType y1,
                                 CoordType x2, CoordType y2,
                                 CoordType x,  CoordType y)
{
    CoordType c = (x - x2)*(y2 - y1) - (y - y2)*(x2 - x1);
    return (c < 0.0f) ? -c : c;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType AbsCrossProduct(const V1& v1, const V2& v2, const V3& v)
{
    CoordType c = (v.x - v2.x)*(v2.y - v1.y) - 
                  (v.y - v2.y)*(v2.x - v1.x);
    return (c < 0.0f) ? -c : c;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
bool PointInTriangle(const V1& v1, const V2& v2, const V3& v3, 
                     CoordType x, CoordType y)
{
    bool cp1 = CrossProduct(v1.x, v1.y, v2.x, v2.y, x, y) < 0;
    bool cp2 = CrossProduct(v2.x, v2.y, v3.x, v3.y, x, y) < 0;
    bool cp3 = CrossProduct(v3.x, v3.y, v1.x, v1.y, x, y) < 0;
    return cp1 == cp2 && cp2 == cp3 && cp3 == cp1;
}

//-----------------------------------------------------------------------
inline CoordType Distance(CoordType x1, CoordType y1,
                          CoordType x2, CoordType y2)
{
    CoordType dx = x2 - x1;
    CoordType dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

//-----------------------------------------------------------------------
template<class V1, class V2>
inline CoordType Distance(const V1& v1, const V2& v2)
{
    CoordType dx = v2.x - v1.x;
    CoordType dy = v2.y - v1.y;
    return sqrtf(dx*dx + dy*dy);
}


//-----------------------------------------------------------------------
inline CoordType SqDistance(CoordType x1, CoordType y1,
                            CoordType x2, CoordType y2)
{
    CoordType dx = x2 - x1;
    CoordType dy = y2 - y1;
    return dx*dx + dy*dy;
}

//-----------------------------------------------------------------------
inline CoordType LinePointDistance(CoordType x1, CoordType y1, 
                                   CoordType x2, CoordType y2, 
                                   CoordType x,  CoordType y)
{
    CoordType dx = x2-x1;
    CoordType dy = y2-y1;
    CoordType d = sqrtf(dx * dx + dy * dy);
    if(d == 0)
    {
        return Distance(x1, y1, x, y);
    }
    return ((x - x2) * dy - (y - y2) * dx) / d;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType LinePointDistance(const V1& v1, const V2& v2, const V3& v)
{
    return LinePointDistance(v1.x, v1.y, v2.x, v2.y, v.x, v.y);
}


//-----------------------------------------------------------------------
inline CoordType LinePointDistanceAbs(CoordType x1, CoordType y1, 
                                      CoordType x2, CoordType y2, 
                                      CoordType x,  CoordType y)
{
    CoordType d = LinePointDistance(x1, y1, x2, y2, x, y);
    return (d < 0) ? -d : d;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType LinePointDistanceAbs(const V1& v1, const V2& v2, const V3& v)
{
    return LinePointDistanceAbs(v1.x, v1.y, v2.x, v2.y, v.x, v.y);
}


// Calculate the slope ratio of the line segment as a function
// of the angle with respect to the 0X axis. 
// The function is typically used to arrange a bundle of vectors in 
// the clockwise or counterclockwise direction.
// The return value is in range [-1...1) and essentially it corresponds 
// to the return value of atan2. But it's not an angle, it's a kind of
// a sinusoidal function.
//-----------------------------------------------------------------------
inline CoordType SlopeRatio(CoordType x1, CoordType y1,
                            CoordType x2, CoordType y2)
{
    CoordType bx = x2 - x1;
    CoordType by = y2 - y1;
    CoordType den = ((CoordType)(bx*bx + by*by) * 2);
    if (den == 0)
    {
        return 0;
    }
    CoordType a = (bx*bx) / den;
    if(bx < 0) a = -a;
    if(by > 0) a = 1-a;
    return a - 0.5f;
}

//-----------------------------------------------------------------------
template<class V1, class V2>
inline CoordType SlopeRatio(const V1& v1, const V2& v2)
{
    return SlopeRatio(v1.x, v1.y, v2.x, v2.y);
}


// Calculate the turn ratio of two consecutive line segments as a function
// of the angle between them. 
// The function is typically used to find a leftmost of a rightmost turn
// in a bundle of vectors with respect to the given vector.
// The return value is in range [-1...1) and corresponds to a sinusoidal
// function. The characteristic values are:
// -1.0 - the second line segment goes back,
// -0.5 - left turn with the right angle between the segments,
//  0.0 - the three points lie in a straight line, 
//  0.5 - right turn with the right angle between the segments.
//-----------------------------------------------------------------------
inline CoordType TurnRatio(CoordType x1, CoordType y1,
                           CoordType x2, CoordType y2,
                           CoordType x3, CoordType y3)
{
    CoordType ax = x2 - x1;
    CoordType ay = y2 - y1;
    CoordType bx = x3 - x2;
    CoordType by = y3 - y2;
    CoordType a = (ax*bx + ay*by) / 
                       (2 * sqrtf( (ax*ax + ay*ay)*(bx*bx + by*by) ));
    if(bx*ay > by*ax) a = 1-a;
    return a - 0.5f;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType TurnRatio(const V1& v1, const V2& v2, const V3& v3,
                           CoordType len1, CoordType len2)
{
    CoordType ax = v2.x - v1.x;
    CoordType ay = v2.y - v1.y;
    CoordType bx = v3.x - v2.x;
    CoordType by = v3.y - v2.y;
    CoordType a = (ax*bx + ay*by) / (len1 * len2 * 2);
    if(bx*ay > by*ax) a = 1-a;
    return a - 0.5f;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType TurnRatio(const V1& v1, const V2& v2,const V3& v3)
{
    return CalcTurnRatio(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
}

//-----------------------------------------------------------------------
inline CoordType TriangleInscribedRadius(CoordType x1, CoordType y1,
                                         CoordType x2, CoordType y2,
                                         CoordType x3, CoordType y3)
{
    CoordType a = Distance(x1, y1, x2, y2);
    CoordType b = Distance(x2, y2, x3, y3);
    CoordType c = Distance(x3, y3, x1, y1);
    CoordType p = (a + b + c) / 2;
    return sqrtf((p - a) * (p - b) * (p - c) / p);
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3>
inline CoordType TriangleInscribedRadius(const V1& v1, const V2& v2, const V3& v3)
{
    return TriangleInscribedRadius(v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);
}

//-----------------------------------------------------------------------
template<class V1, class V2>
inline void Parallel(V1* v1, V2* v2, CoordType w)
{
    CoordType dx  = v2->x - v1->x;
    CoordType dy  = v1->y - v2->y;
    CoordType d   = sqrtf(dx*dx + dy*dy);
    CoordType dx2 = w * dy / d;
    CoordType dy2 = w * dx / d;
    v1->x -= dx2;  
    v1->y -= dy2;
    v2->x -= dx2;  
    v2->y -= dy2;
}

//-----------------------------------------------------------------------
inline bool Intersection(CoordType ax, CoordType ay, 
                         CoordType bx, CoordType by,
                         CoordType cx, CoordType cy, 
                         CoordType dx, CoordType dy,
                         CoordType* x, CoordType* y,
                         CoordType epsilon)
{
    CoordType na  = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
//  CoordType nb  = (bx-ax) * (ay-cy) - (by-ay) * (ax-cx);
    CoordType den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);
    if(fabsf(den) < epsilon) 
    {
        return false; 
    }
    CoordType u = na / den;
    *x = ax + u * (bx-ax);
    *y = ay + u * (by-ay);
    return true;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3, class V4>
inline bool Intersection(const V1& a, const V2& b, 
                         const V3& c, const V4& d, 
                         CoordType* x, CoordType* y,
                         CoordType epsilon)
{
    return Intersection(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, 
                        x, y, epsilon);
}

//-----------------------------------------------------------------------
inline bool SegmentIntersection(CoordType ax, CoordType ay, 
                                CoordType bx, CoordType by,
                                CoordType cx, CoordType cy, 
                                CoordType dx, CoordType dy,
                                CoordType* x, CoordType* y,
                                CoordType epsilon)
{
    CoordType na  = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
    CoordType nb  = (bx-ax) * (ay-cy) - (by-ay) * (ax-cx);
    CoordType den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);

    if(fabsf(den) < epsilon) 
    {
        return false; 
    }

    CoordType ua = na / den;
    CoordType ub = nb / den;

    if(ua <= 0 || ua >= 1 || ub <= 0 || ub >= 1)
    {
        return false;
    }
    *x = ax + ua * (bx-ax);
    *y = ay + ua * (by-ay);
    return true;
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3, class V4>
inline bool SegmentIntersection(const V1& a, const V2& b, 
                                const V3& c, const V4& d, 
                                CoordType* x, CoordType* y,
                                CoordType epsilon)
{
    return SegmentIntersection(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y, 
                               x, y,
                               epsilon);
}

//-----------------------------------------------------------------------
inline CoordType PointToSegmentPos(CoordType x1, CoordType y1,
                                   CoordType x2, CoordType y2,
                                   CoordType x,  CoordType y)
{
    CoordType dx = x2 - x1;
    CoordType dy = y2 - y1;
    if(dx == 0 && dy == 0) return 0;
    return (dx*(x - x1) + dy*(y - y1)) / (dx*dx + dy*dy);
}

//-----------------------------------------------------------------------
template<class Vertex1, class Vertex2>
CoordType PointToSegmentPos(const Vertex1& v1, const Vertex1& v2,
                            const Vertex2& v)
{
    return PointToSegmentPos(v1.x, v1.y, v2.x, v2.y, v.x, v.y);
}

//-----------------------------------------------------------------------
inline bool SegmentIntersectionExists(CoordType x1, CoordType y1,
                                      CoordType x2, CoordType y2,
                                      CoordType x3, CoordType y3,
                                      CoordType x4, CoordType y4)
{
    // It's less expensive but you can't control the 
    // boundary conditions: Less or LessEqual
    Double dx1 = x2 - x1;
    Double dy1 = y2 - y1;
    Double dx2 = x4 - x3;
    Double dy2 = y4 - y3;
    return ((x3 - x2) * dy1 - (y3 - y2) * dx1 < 0) != 
           ((x4 - x2) * dy1 - (y4 - y2) * dx1 < 0) &&
           ((x1 - x4) * dy2 - (y1 - y4) * dx2 < 0) !=
           ((x2 - x4) * dy2 - (y2 - y4) * dx2 < 0);
}

//-----------------------------------------------------------------------
template<class V1, class V2, class V3, class V4>
inline bool SegmentIntersectionExists(const V1& a, const V2& b, 
                                      const V3& c, const V4& d)
{
    return SegmentIntersectionExists(a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
}

//--------------------------------------------------------------------
inline void Bisectrix(CoordType x1, CoordType y1, 
                      CoordType x2, CoordType y2, 
                      CoordType x3, CoordType y3, 
                      CoordType* x, CoordType* y,
                      CoordType epsilon)
{
    CoordType k = Distance(x2, y2, x3, y3) / 
                  Distance(x1, y1, x2, y2);
    *x = x3 - (x2 - x1) * k;
    *y = y3 - (y2 - y1) * k;


    if((*x - x2) * (*x - x2) + (*y - y2) * (*y - y2) < epsilon)
    {
        *x = (x2 + x2 + (y2 - y1) + (y3 - y3)) / 2;
        *y = (y2 + y2 - (x2 - x1) - (x3 - x3)) / 2;
    }
}

//--------------------------------------------------------------------
inline void PointOnQuadCurve(CoordType x1, CoordType y1,
                             CoordType x2, CoordType y2,
                             CoordType x3, CoordType y3,
                             CoordType t,
                             CoordType* x, CoordType* y)
{
    CoordType x12  = x1  + t*(x2  - x1);
    CoordType y12  = y1  + t*(y2  - y1);
    CoordType x23  = x2  + t*(x3  - x2);
    CoordType y23  = y2  + t*(y3  - y2);
                *x = x12 + t*(x23 - x12);
                *y = y12 + t*(y23 - y12);
}


//--------------------------------------------------------------------
inline CoordType QuadCurveExtremum(CoordType x1, CoordType x2, CoordType x3)
{
    CoordType dx = 2 * x2 - x1 - x3;
    return (dx == 0) ? -1 : (x2 - x1) / dx;
}

//--------------------------------------------------------------------
enum SegmentType
{
    Seg_MoveTo = 0,
    Seg_LineTo = 1,
    Seg_QuadTo = 2
};

//--------------------------------------------------------------------
template<class Container>
bool TestQuadCollinearity(Container* con, const ToleranceParams& param, 
                          CoordType x1, CoordType y1,
                          CoordType x2, CoordType y2,
                          CoordType x3, CoordType y3)
{
    // Checking the curve for collinearity. 
    // We need to calculate the distance between p2 and the line p1-p3
    // (the length of the perpendicular). Essentially it is: 
    // d = cross_product(p1, p3, p2) / length(p1, p3);
    // To calculate the length we have to calculate a square root, 
    // but it is possible to avoid it, since we can compare the squares
    // of values instead of values themselves. 
    // 
    // If the three points are collinear we can do without any subdivision.
    // The subdivision function correctly processes the simple collinear
    // case p1-p2-p3, but fails in case when the second line segment
    // makes a U-turn, that is, p1-p3-p2 or p3-p1-p2 (where p2 is the curve 
    // control point). In the simple collinear case the result consists of 
    // only two points, but in the U-turn case we have to calculate the 
    // third point.
    //-----------------
    CoordType da = SqDistance(x1, y1, x3, y3);
    CoordType d  = AbsCrossProduct(x1, y1, x3, y3, x2, y2);
    CoordType collinearityTolerance = param.CollinearityTolerance / 4;
    if(d * d <= collinearityTolerance * collinearityTolerance * da)
    {
        // The expression below verifies if point p2 lies between p1 and p3.
        // If it does (0 <= d <= 1) we can omit the third point. Otherwise
        // it's necessary to calculate it.
        //---------------------
        if(da == 0) d = 2;
        else        d = ((x2 - x1)*(x3 - x1) + (y2 - y1)*(y3 - y1)) / da;
        if(d < 0 || d > 1)
        {
            // Calculate the ratio (parameter t) to subdivide the curve. 
            // This ratio corresponds to the point on the curve with the maximal 
            // curvature. The formula is the proportion between d1 and d2, 
            // that is, d1 / (d1 + d2), where d1 and d2 are the Euclidean 
            // distances p1-p2 and p2-p3 respectively.
            CoordType d1 = Distance(x1, y1, x2, y2);
            CoordType d2 = Distance(x2, y2, x3, y3) + d1;
            if(d2 != 0)
            {
                CoordType t    = d1 / d2;
                CoordType x12  = x1 + t*(x2 - x1);
                CoordType y12  = y1 + t*(y2 - y1);
                CoordType x23  = x2 + t*(x3 - x2);
                CoordType y23  = y2 + t*(y3 - y2);
                con->AddVertex(x12 + t*(x23 - x12), y12 + t*(y23 - y12));
            }
        }
        con->AddVertex(x3, y3);
        return true;
    }
    return false;
}



//--------------------------------------------------------------------
template<class Container>
void TessellateQuadRecursively(Container* con, CoordType toleranceSq, 
                               CoordType x1, CoordType y1,
                               CoordType x2, CoordType y2,
                               CoordType x3, CoordType y3,
                               int level)
{
    // Check the conditions to stop curve subdivision. 
    // We need to calculate the distance between p2 and the line p1-p3
    // (the length of the perpendicular). Essentially it is: 
    // d = cross_product(p1, p3, p2) / length(p1, p3);
    // To calculate the length we have to calculate a square root, 
    // but it is possible to avoid it, since we can compare the squares
    // of values instead of values themselves. 
    //-----------------
    CoordType d = AbsCrossProduct(x1, y1, x3, y3, x2, y2);
    if(d == 0 ||
       d * d <= toleranceSq * SqDistance(x1, y1, x3, y3) ||
       level >= CurveRecursionLimit)
    {
        // The curve is flat enough, so that we add the point 
        // and stop subdivision.
        //---------------------
        con->AddVertex(x3, y3);
        return;
    }
    // Subdivide the curve at t=0.5.
    //-----------------
    CoordType x12  = (x1  + x2)  / 2;
    CoordType y12  = (y1  + y2)  / 2;
    CoordType x23  = (x2  + x3)  / 2;
    CoordType y23  = (y2  + y3)  / 2;
    CoordType x123 = (x12 + x23) / 2;
    CoordType y123 = (y12 + y23) / 2;
    TessellateQuadRecursively(con, toleranceSq, x1, y1, x12, y12, x123, y123, level+1);
    TessellateQuadRecursively(con, toleranceSq, x123, y123, x23, y23, x3, y3, level+1);
}




//--------------------------------------------------------------------
template<class Container>
void TessellateQuadCurve(Container* con, const ToleranceParams& param, 
                         CoordType x2, CoordType y2,
                         CoordType x3, CoordType y3)
{
    CoordType x1 = con->GetLastX();
    CoordType y1 = con->GetLastY(); 
    if(!TestQuadCollinearity(con, param, x1, y1, x2, y2, x3, y3))
    {
        CoordType toleranceSq = (param.CurveTolerance/4) * (param.CurveTolerance/4);
        TessellateQuadRecursively(con, toleranceSq, x1, y1, x2, y2, x3, y3, 0);
    }
}


//--------------------------------------------------------------------
struct NullTransformer
{
    void Transform(CoordType*, CoordType*) const {}
};

//--------------------------------------------------------------------
inline void ExpandBounds(CoordType  x,  CoordType  y,
                         CoordType* x1, CoordType* y1, 
                         CoordType* x2, CoordType* y2)
{
    if (x < *x1) *x1 = x;
    if (y < *y1) *y1 = y;
    if (x > *x2) *x2 = x;
    if (y > *y2) *y2 = y;
}

//--------------------------------------------------------------------
template<class PathReader, class PathInfo, class Transformer>
void ExpandBounds(PathReader* reader, PathInfo* pathInfo, 
                  CoordType* x1, CoordType* y1, CoordType* x2, CoordType* y2,
                  const Transformer& trans = NullTransformer())
{
    CoordType coord[4];
    CoordType ax = 0;
    CoordType ay = 0;
    unsigned  i;
    unsigned  n = pathInfo->GetSize();

    for(i = 0; i < n; ++i)
    {
        unsigned seg = reader->ReadSegment(pathInfo, coord);
        if (seg != Seg_QuadTo)
        {
            trans.Transform(&coord[0], &coord[1]);
            ExpandBounds(coord[0], coord[1], x1, y1, x2, y2);
            ax = coord[0];
            ay = coord[1];
        }
        else
        {
            trans.Transform(&coord[0], &coord[1]);
            trans.Transform(&coord[2], &coord[3]);
            CoordType t, x, y;
            t = QuadCurveExtremum(ax, coord[0], coord[2]);
            if (t > 0 && t < 1)
            {
                PointOnQuadCurve(ax, ay, coord[0], coord[1], coord[2], coord[3], t, &x, &y);
                ExpandBounds(x, y, x1, y1, x2, y2);
            }
            t = QuadCurveExtremum(ay, coord[1], coord[3]);
            if (t > 0 && t < 1)
            {
                PointOnQuadCurve(ax, ay, coord[0], coord[1], coord[2], coord[3], t, &x, &y);
                ExpandBounds(x, y, x1, y1, x2, y2);
            }
            ax = coord[2];
            ay = coord[3];
            ExpandBounds(ax, ay, x1, y1, x2, y2);
        }
    }
}

//--------------------------------------------------------------------
template<class PathReader, class PathInfo, class Transformer>
void ComputeBounds(PathReader* reader, PathInfo* pathInfo, 
                   CoordType* x1, CoordType* y1, CoordType* x2, CoordType* y2,
                   const Transformer& trans = NullTransformer())
{
    *x1 = MaxCoord;
    *y1 = MaxCoord;
    *x2 = MinCoord;
    *y2 = MinCoord;
    ExpandBounds(reader, pathInfo, x1, y1, x2, y2, trans);
}



}}} // Scaleform::Render::Math2D

#endif

