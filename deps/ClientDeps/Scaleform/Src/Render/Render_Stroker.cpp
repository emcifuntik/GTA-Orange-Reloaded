/**********************************************************************

Filename    :   Stroker.cpp
Content     :   
Created     :   2005
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Notes       :   Path-to-stroke converter

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

----------------------------------------------------------------------
The code of these classes was taken from the Anti-Grain Geometry
Project and modified for the use by Scaleform. 
Permission to use without restrictions is hereby granted to 
Scaleform Corporation by the author of Anti-Grain Geometry Project.
**********************************************************************/

#include "Render_Stroker.h"

#ifdef SF_RENDER_ENABLE_STROKER

#include "Kernel/SF_Alg.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
void StrokePath::Clear()
{ 
    Path.Clear(); 
}

//------------------------------------------------------------------------
void StrokePath::ClearAndRelease()
{ 
    Path.ClearAndRelease(); 
}

//------------------------------------------------------------------------
void StrokePath::AddVertex(const StrokeVertex& v)
{
    if (Path.GetSize() == 0 || Path.Back().Distance(v))
    {
        Path.PushBack(v);
    }
}

//------------------------------------------------------------------------
bool StrokePath::ClosePath()
{
    if (Path.GetSize() > 1)
    {
        if (!Path.Back().Distance(Path[0]))
        {
            Path.PopBack();
            return true;    // Closed path
        }
    }
    return false;
}

//-----------------------------------------------------------------------
Stroker::Stroker(LinearHeap* heap) :
    pHeap(heap),
    Path(heap),
    Width(CoordType(1.0)),
    LineJoin(RoundJoin),
    StartLineCap(RoundCap),
    EndLineCap(RoundCap),
    MiterLimit(3.0f),
    CurveTolerance(1),
    IntersectionEpsilon(0.01f),
    Closed(false)
{}


//-----------------------------------------------------------------------
void Stroker::Clear()
{ 
    Path.ClearAndRelease(); 
    Closed = false; 
    pHeap->ClearAndRelease();
}

//-----------------------------------------------------------------------
void Stroker::SetToleranceParam(const ToleranceParams& param)
{
    CurveTolerance      = param.CurveTolerance;
    IntersectionEpsilon = param.IntersectionEpsilon;
}


//------------------------------------------------------------------------
void Stroker::ClosePath()
{
    Path.ClosePath();
    Closed = true;
}

//------------------------------------------------------------------------
void Stroker::FinalizePath(TessBase* tess)
{
    if (!Closed)
    {
        Closed = Path.ClosePath();
    }
    if(Path.GetVertexCount() > 1)
    {
        UPInt i;
        if(Closed)
        {
            for(i = 0; i < Path.GetVertexCount(); ++i)
            {
                calcJoin(tess, 
                         Path.GetVertexPrev(i), 
                         Path.GetVertex    (i), 
                         Path.GetVertexNext(i), 
                         Path.GetVertexPrev(i).dist,
                         Path.GetVertex    (i).dist);
            }
            tess->ClosePath();
            tess->FinalizePath(0, 1, false, false);
            for(i = Path.GetVertexCount(); i > 0; --i)
            {
                calcJoin(tess,
                         Path.GetVertexNext(i-1), 
                         Path.GetVertex    (i-1), 
                         Path.GetVertexPrev(i-1), 
                         Path.GetVertex    (i-1).dist, 
                         Path.GetVertexPrev(i-1).dist);
            }
            tess->ClosePath();
            tess->FinalizePath(0, 1, false, false);
        }
        else
        {
            calcCap(tess,
                    Path.GetVertex(0),
                    Path.GetVertex(1),
                    Path.GetVertex(0).dist,
                    StartLineCap);

            for(i = 1; i+1 < Path.GetVertexCount(); i++)
            {
                calcJoin(tess, 
                         Path.GetVertexPrev(i), 
                         Path.GetVertex    (i), 
                         Path.GetVertexNext(i), 
                         Path.GetVertexPrev(i).dist,
                         Path.GetVertex    (i).dist);
            }

            calcCap(tess,
                    Path.GetVertex(Path.GetVertexCount()-1), 
                    Path.GetVertex(Path.GetVertexCount()-2), 
                    Path.GetVertex(Path.GetVertexCount()-2).dist,
                    EndLineCap);

            for(i = Path.GetVertexCount()-2; i > 0; --i)
            {
                calcJoin(tess,
                         Path.GetVertexNext(i), 
                         Path.GetVertex    (i), 
                         Path.GetVertexPrev(i), 
                         Path.GetVertex    (i).dist, 
                         Path.GetVertexPrev(i).dist);
            }
            tess->ClosePath();
            tess->FinalizePath(0, 1, false, false);
        }
    }
    Path.Clear();
    Closed = false; 
}




//------------------------------------------------------------------------
void Stroker::calcArc(TessBase* tess,
                      CoordType x,   CoordType y, 
                      CoordType dx1, CoordType dy1, 
                      CoordType dx2, CoordType dy2)
{
    CoordType a1 = atan2f(dy1, dx1);
    CoordType a2 = atan2f(dy2, dx2);
    CoordType da = acosf(Width / (Width + 0.25f * CurveTolerance)) * 2;

    tess->AddVertex(x + dx1, y + dy1);

    if(a1 > a2) a2 += 2 * Math2D::Pi;
    int n = int((a2 - a1) / da);
    da = (a2 - a1) / (n + 1);
    a1 += da;

    int i;
    for(i = 0; i < n; i++)
    {
        tess->AddVertex(x + cosf(a1) * Width, 
                        y + sinf(a1) * Width);
        a1 += da;
    }

    tess->AddVertex(x + dx2, y + dy2);
}

//------------------------------------------------------------------------
void Stroker::calcMiter(TessBase* tess,
                        const StrokeVertex& v0, 
                        const StrokeVertex& v1, 
                        const StrokeVertex& v2,
                        CoordType dx1, CoordType dy1, 
                        CoordType dx2, CoordType dy2,
                        LineJoinType lineJoin,
                        CoordType miterLimit,
                        CoordType epsilon,
                        CoordType dbevel)
{
    CoordType xi  = v1.x;
    CoordType yi  = v1.y;
    CoordType di  = 1;
    CoordType lim = Width * miterLimit;
    bool miterLimitExceeded = true; // Assume the worst
    bool intersectionFailed = true; // Assume the worst

    if(Math2D::Intersection(v0.x+dx1, v0.y+dy1, v1.x+dx1, v1.y+dy1,
                            v1.x+dx2, v1.y+dy2, v2.x+dx2, v2.y+dy2,
                            &xi, &yi, epsilon))
    {
        // Calculation of the intersection succeeded
        //---------------------
        di = Math2D::Distance(v1.x, v1.y, xi, yi);
        if(di <= lim)
        {
            // Inside the miter limit
            //---------------------
            tess->AddVertex(xi, yi);
            miterLimitExceeded = false;
        }
        intersectionFailed = false;
    }
    else
    {
        // Calculation of the intersection failed, most probably
        // the three points lie one straight line. 
        // First check if v0 and v2 lie on the opposite sides of vector: 
        // (v1.x, v1.y) -> (v1.x+dx1, v1.y-dy1), that is, the perpendicular
        // to the line determined by vertices v0 and v1.
        // This condition determines whether the next line segments continues
        // the previous one or goes back.
        //----------------
        CoordType x2 = v1.x + dx1;
        CoordType y2 = v1.y + dy1;
        if(((v0.x - x2)*dy1 - (v0.y - y2)*dx1 < 0.0) !=
           ((v2.x - x2)*dy1 - (v2.y - y2)*dx1 < 0.0))
        {
            // This case means that the next segment continues 
            // the previous one (straight line)
            //-----------------
            tess->AddVertex(v1.x + dx1, v1.y + dy1);
            miterLimitExceeded = false;
        }
    }

    if(miterLimitExceeded)
    {
        // Miter limit exceeded
        //------------------------
        switch(lineJoin)
        {
        case MiterBevelJoin:
            // For the compatibility with SVG, PDF, etc, 
            // we use a simple bevel join instead of
            // "smart" bevel
            //-------------------
            tess->AddVertex(v1.x + dx1, v1.y + dy1);
            tess->AddVertex(v1.x + dx2, v1.y + dy2);
            break;

        default:
            // If no miter-revert, calculate new dx1, dy1, dx2, dy2
            //----------------
            if(intersectionFailed)
            {
                tess->AddVertex(v1.x + dx1 - dy1 * miterLimit, 
                                v1.y + dy1 + dx1 * miterLimit);
                tess->AddVertex(v1.x + dx2 + dy2 * miterLimit, 
                                v1.y + dy2 - dx2 * miterLimit);
            }
            else
            {
                CoordType x1 = v1.x + dx1;
                CoordType y1 = v1.y + dy1;
                CoordType x2 = v1.x + dx2;
                CoordType y2 = v1.y + dy2;
                di = (lim - dbevel) / (di - dbevel);
                tess->AddVertex(x1 + (xi - x1) * di, y1 + (yi - y1) * di);
                tess->AddVertex(x2 + (xi - x2) * di, y2 + (yi - y2) * di);
            }
            break;
        }
    }
}

//------------------------------------------------------------------------
void Stroker::calcCap(TessBase* tess,
                      const StrokeVertex& v0, 
                      const StrokeVertex& v1, 
                      CoordType len,
                      LineCapType cap)
{
    CoordType dx1 = (v1.y - v0.y) / len;
    CoordType dy1 = (v0.x - v1.x) / len;
    CoordType dx2 = 0;
    CoordType dy2 = 0;

    dx1 *= Width;
    dy1 *= Width;

    if(cap != RoundCap)
    {
        if(cap == SquareCap)
        {
            dx2 = dy1;
            dy2 = dx1;
        }
        tess->AddVertex(v0.x - dx1 + dx2, v0.y - dy1 - dy2);
        tess->AddVertex(v0.x + dx1 + dx2, v0.y + dy1 - dy2);
    }
    else
    {
        CoordType a1 = atan2f(-dy1, -dx1);
        CoordType a2 = a1 + Math2D::Pi;
        CoordType da = acosf(Width / (Width + 0.25f * CurveTolerance)) * 2;
        int i;
        int n = int((a2 - a1) / da);
        da = (a2 - a1) / (n + 1);
        a1 += da;

        tess->AddVertex(v0.x - dx1, v0.y - dy1);
        for(i = 0; i < n; i++)
        {
            tess->AddVertex(v0.x + cosf(a1) * Width, 
                            v0.y + sinf(a1) * Width);
            a1 += da;
        }
        tess->AddVertex(v0.x + dx1, v0.y + dy1);
    }
}

//------------------------------------------------------------------------
void Stroker::calcJoin(TessBase* tess,
                       const StrokeVertex& v0, 
                       const StrokeVertex& v1, 
                       const StrokeVertex& v2,
                       CoordType len1, 
                       CoordType len2)
{
    CoordType dx1, dy1, dx2, dy2;
    CoordType epsilon = (len1 + len2) * IntersectionEpsilon;

    dx1 = Width * (v1.y - v0.y) / len1;
    dy1 = Width * (v0.x - v1.x) / len1;
    dx2 = Width * (v2.y - v1.y) / len2;
    dy2 = Width * (v1.x - v2.x) / len2;

    if(Math2D::CrossProduct(v0, v1, v2) > 0)
    {
        // Inner join
        //---------------
        CoordType minLen = (len1 < len2) ? len1 : len2;
        calcMiter(tess,
                  v0, v1, v2, dx1, dy1, dx2, dy2, 
                  MiterBevelJoin, 
                  minLen / Width,
                  epsilon, 0);
    }
    else
    {
        // Outer join
        //---------------
        CoordType dx = (dx1 + dx2) / 2;
        CoordType dy = (dy1 + dy2) / 2;
        CoordType dbevel = sqrtf(dx * dx + dy * dy);
        LineJoinType lineJoin = LineJoin;
        if(lineJoin == RoundJoin || lineJoin == BevelJoin)
        {
            // This is an optimization that reduces the number of points 
            // in cases of almost collinear segments. If there's no
            // visible difference between bevel and miter joins we'd rather
            // use miter join because it adds only one point instead of two. 
            //
            // Here we calculate the middle point between the bevel points 
            // and then, the distance between v1 and this middle point. 
            // At outer joins this distance always less than stroke width, 
            // because it's actually the height of an isosceles triangle of
            // v1 and its two bevel points. If the difference between this
            // width and this value is small (no visible bevel) we can switch
            // to the miter join. 
            //
            // The constant in the expression makes the result approximately 
            // the same as in round joins and caps. 
            //-------------------
            if((Width - dbevel) < CurveTolerance * (0.25f/2))
            {
                if(Math2D::Intersection(v0.x + dx1, v0.y + dy1,
                                        v1.x + dx1, v1.y + dy1,
                                        v1.x + dx2, v1.y + dy2,
                                        v2.x + dx2, v2.y + dy2,
                                        &dx, &dy,
                                        epsilon))
                {
                    tess->AddVertex(dx, dy);
                }
                else
                {
                    tess->AddVertex(v1.x + dx1, v1.y + dy1);
                }
                return;
            }
        }

        switch(lineJoin)
        {
        case MiterJoin:
        case MiterBevelJoin:
            calcMiter(tess, 
                      v0, v1, v2, dx1, dy1, dx2, dy2, 
                      lineJoin, 
                      MiterLimit,
                      epsilon,
                      dbevel);
            break;

        case RoundJoin:
            calcArc(tess, v1.x, v1.y, dx1, dy1, dx2, dy2);
            break;

        default: // Bevel join
            tess->AddVertex(v1.x + dx1, v1.y + dy1);
            tess->AddVertex(v1.x + dx2, v1.y + dy2);
            break;
        }
    }
}
























//-----------------------------------------------------------------------
StrokeSorter::StrokeSorter(LinearHeap* heap) :
    pHeap(heap),
    SrcVertices(heap), 
    SrcPaths(heap), 
    SortedPaths(heap),
    OutVertices(heap),
    OutPaths(heap), 
    LastVertex(0) 
{}

//-----------------------------------------------------------------------
void StrokeSorter::Clear()
{
    SrcVertices.ClearAndRelease();
    SrcPaths.ClearAndRelease();
    SortedPaths.ClearAndRelease();
    OutVertices.ClearAndRelease();
    OutPaths.ClearAndRelease();
    LastVertex = 0;
    pHeap->ClearAndRelease();
}

//-----------------------------------------------------------------------
void StrokeSorter::AddVertex(CoordType x, CoordType y, unsigned segType)
{
    if (SrcVertices.GetSize() == LastVertex && SrcVertices.GetSize() && 
        SrcVertices.Back().x == x && SrcVertices.Back().y == y)
    {
        // If the first path vertex coincides with the end of the 
        // last path, open the path and continue adding to it.
        //-----------------
        LastVertex = SrcPaths.Back().start;
        SrcPaths.PopBack();
    }
    else
    {
        VertexType v = { x, y, (UInt8)segType, false, false };
        SrcVertices.PushBack(v);
    }
}

//-----------------------------------------------------------------------
void StrokeSorter::AddQuad(CoordType x2, CoordType y2, CoordType x3, CoordType y3)
{
    AddVertex(x2, y2, Math2D::Seg_QuadTo);
    AddVertex(x3, y3, Math2D::Seg_QuadTo);
}

//-----------------------------------------------------------------------
void StrokeSorter::FinalizePath()
{
    if (SrcVertices.GetSize() > LastVertex+1)
    {
        //const VertexType& v = SrcVertices[LastVertex];
        PathType p = { LastVertex, (unsigned)SrcVertices.GetSize() - LastVertex };
        SrcPaths.PushBack(p);
        LastVertex = (unsigned)SrcVertices.GetSize();
    }
    else
    {
        SrcVertices.CutAt(LastVertex);
    }
}

//-----------------------------------------------------------------------
void StrokeSorter::appendPath(PathType* dst, PathType* src)
{
    if (dst->numVer == 0)
    {
        dst->start = (unsigned)OutVertices.GetSize();
        OutVertices.PushBack(SrcVertices[src->start]);
        ++dst->numVer;
    }
    unsigned i;
    unsigned n = src->numVer & NumVerMask;
    for (i = 1; i < n; ++i)
    {
        OutVertices.PushBack(SrcVertices[src->start+i]);
        ++dst->numVer;
    }
    SF_ASSERT(OutVertices[dst->start].segType != Math2D::Seg_QuadTo);
}


//-----------------------------------------------------------------------
unsigned StrokeSorter::findNext(const PathType& outPath) const
{
    SF_ASSERT(outPath.numVer);
    const VertexType& end = OutVertices[outPath.start + (outPath.numVer & NumVerMask) - 1];
    SortedPathType find = { end.x, end.y };
    UPInt idx = Alg::LowerBound(SortedPaths, find, cmpPaths);

    for (; idx < SortedPaths.GetSize(); ++idx)
    {
        const SortedPathType& p2 = SortedPaths[idx];
        if (p2.x != find.x || p2.y != find.y)
        {
            break;
        }
        if ((p2.thisPath->numVer & VisitedFlag) == 0)
        {
            return (unsigned)idx;
        }
    }
    return ~0U;
}

//-----------------------------------------------------------------------
void StrokeSorter::Sort()
{
    UPInt i, n;

    // Duplicate the paths with the "inverse" flag.
    //-------------------
    n = SrcPaths.GetSize();
    for (i = 0; i < n; ++i)
    {
        PathType p = SrcPaths[i];
        p.numVer |= InverseFlag;
        SrcPaths.PushBack(p);
    }

    // Fill and sort the SortedPaths array
    //-------------------
    SortedPaths.Resize(n);
    for (i = 0; i < n; ++i)
    {
        const PathType& p = SrcPaths[i];
        const VertexType& v = SrcVertices[p.start];
        SortedPathType& sp = SortedPaths[i];
        sp.x = v.x;
        sp.y = v.y;
        sp.thisPath = &SrcPaths[i];
    }
    Alg::QuickSort(SortedPaths, cmpPaths);

    // Construct the chains
    //--------------------
    for (i = 0; i < SortedPaths.GetSize(); ++i)
    {
        if (SortedPaths[i].thisPath->numVer & VisitedFlag) continue;

        unsigned next = (unsigned)i;
        const VertexType* v1;
        const VertexType* v2;
        PathType  p = { 0, 0 };
        do
        {
            SortedPathType* sp = &SortedPaths[next & NumVerMask];
            sp->thisPath->numVer |= VisitedFlag;
            appendPath(&p, SortedPaths[next & NumVerMask].thisPath);
            v1 = &OutVertices[p.start];
            v2 = &OutVertices[p.start + (p.numVer & NumVerMask) - 1];
            if (v1->x == v2->x && v1->y == v2->y) break; // Closed path detected
        }
        while((next = findNext(p)) != ~0U);

        v1 = &OutVertices[p.start];
        v2 = &OutVertices[p.start + (p.numVer & NumVerMask) - 1];
        if (v1->x == v2->x && v1->y == v2->y) 
        {
            p.numVer |= ClosedFlag;
        }
        OutPaths.PushBack(p);
    }
}


//-----------------------------------------------------------------------
void StrokeSorter::AddOffset(CoordType offsetX, CoordType offsetY)
{
    unsigned i, j;
    for(i = 0; i < OutPaths.GetSize(); ++i)
    {
        unsigned start  = OutPaths[i].start;
        unsigned numVer = OutPaths[i].numVer & NumVerMask;

        for (j = 0; j < numVer; ++j)
        {
            OutVertices[start+j].x += offsetX;
            OutVertices[start+j].y += offsetY;
        }
    }
}


//-----------------------------------------------------------------------
void StrokeSorter::Snap(CoordType offsetX, CoordType offsetY)
{
    // If there are one or more vertical segments, snap all vertically
    // if there are one or more horizontal segments, snap all horizontally
    //---------------------------
    unsigned i, j;
    for(i = 0; i < OutPaths.GetSize(); ++i)
    {
        unsigned start  = OutPaths[i].start;
        unsigned numVer = OutPaths[i].numVer & NumVerMask;
        unsigned closed = OutPaths[i].numVer & ClosedFlag;
    
        bool snapX = false;
        bool snapY = false;
        unsigned s = 1;
        VertexType* v1 = &OutVertices[start];
        if (closed)
        {
            s = 0;
            v1 = &OutVertices[start+numVer-1];
        }
        for (j = s; j < numVer; ++j)
        {
            VertexType* v2 = &OutVertices[start+j];
            if (v1->x == v2->x && v1->y != v2->y) snapX = true;
            if (v1->y == v2->y && v1->x != v2->x) snapY = true;
            v1 = v2;
        }
        if (snapX)
        {
            for (j = 0; j < numVer; ++j)
            {
                CoordType x = OutVertices[start+j].x;
                OutVertices[start+j].x = floorf((x < 0) ? x-0.5f : x+0.5f) + offsetX;
            }
        }
        if (snapY)
        {
            for (j = 0; j < numVer; ++j)
            {
                CoordType y = OutVertices[start+j].y;
                OutVertices[start+j].y = floorf((y < 0) ? y-0.5f : y+0.5f) + offsetY;
            }
        }
    }


    //// An attempt to use a "smart-ass" method. Mark vertices to snap
    //// and then snap only marked vertices. Produces nasty "rays" on rounded
    //// rectangles after heavy zoom-out.
    ////---------------------------
    //unsigned i, j;
    //for(i = 0; i < OutPaths.GetSize(); ++i)
    //{
    //    unsigned start  = OutPaths[i].start;
    //    unsigned numVer = OutPaths[i].numVer & NumVerMask;
    //    unsigned closed = OutPaths[i].numVer & ClosedFlag;
    //
    //    unsigned s = 1;
    //    VertexType* v1 = &OutVertices[start];
    //    VertexType* v2;
    //    VertexType* v3;
    //    if (closed)
    //    {
    //        s = 0;
    //        v1 = &OutVertices[start+numVer-1];
    //    }
    //    for (j = s; j < numVer; ++j)
    //    {
    //        VertexType* v2 = &OutVertices[start+j];
    //        if (v1->x == v2->x) v1->snapX = v2->snapX = true;
    //        if (v1->y == v2->y) v1->snapY = v2->snapY = true;
    //        v1 = v2;
    //    }
    //    for (j = 0; j < numVer; ++j)
    //    {
    //        v1 = &OutVertices[start+j];
    //        if (v1->snapX)
    //            v1->x = floorf((v1->x < 0) ? v1->x-0.5f : v1->x+0.5f) + offsetX;
    //        if (v1->snapY)
    //            v1->y = floorf((v1->y < 0) ? v1->y-0.5f : v1->y+0.5f) + offsetY;
    //    }
    //    // An attempt to cure the "rays". Doesn't help
    //    //------------------
    //    //v1 = 0;
    //    //for (j = 0; j < numVer;)
    //    //{
    //    //    v3 = getVertex(i, j++);
    //    //    if (v3->segType == Math2D::Seg_QuadTo)
    //    //    {
    //    //        v2 = v3;
    //    //        v3 = getVertex(i, j++);
    //    //        if (v1)
    //    //        {
    //    //            //if ((!v2->snapX && (v1->snapX || v3->snapX)) ||
    //    //            //    (!v2->snapY && (v1->snapY || v3->snapY)))
    //    //            //{
    //    //            //    v2->x = floorf((v2->x < 0) ? v2->x-0.5f : v2->x+0.5f) + offsetX;
    //    //            //    v2->y = floorf((v2->y < 0) ? v2->y-0.5f : v2->y+0.5f) + offsetY;
    //    //            //}
    //    //
    //    //            if (!v2->snapX && (v1->snapX || v3->snapX))
    //    //                v2->x = floorf((v2->x < 0) ? v2->x-0.5f : v2->x+0.5f) + offsetX;
    //    //            if (!v2->snapY && (v1->snapY || v3->snapY))
    //    //                v2->y = floorf((v2->y < 0) ? v2->y-0.5f : v2->y+0.5f) + offsetY;
    //    //        }
    //    //    }
    //    //    v1 = v3;
    //    //}
    //}


    //// Simplest. Just snap all. Distorts rotated rounded rectangles.
    ////---------------------------
    //unsigned i;
    //for (i = 0; i < OutVertices.GetSize(); ++i)
    //{
    //    CoordType x = OutVertices[i].x;
    //    CoordType y = OutVertices[i].y;
    //    OutVertices[i].x = floorf((x < 0) ? x-0.5f : x+0.5f) + offsetX;
    //    OutVertices[i].y = floorf((y < 0) ? y-0.5f : y+0.5f) + offsetY;
    //}

}

}} // Scaleform::Render

#endif // SF_RENDER_ENABLE_STROKER
