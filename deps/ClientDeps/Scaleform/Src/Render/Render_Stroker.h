/**********************************************************************

Filename    :   Stroker.h
Content     :   Path-to-stroke converter
Created     :   2005
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2005-2009 Scaleform Corp. All Rights Reserved.
                Patent Pending. Contact Scaleform for more information.

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

#ifndef INC_SF_Render_Stroker_H
#define INC_SF_Render_Stroker_H

#include "Kernel/SF_Types.h"

#ifdef SF_RENDER_ENABLE_STROKER

#include "Render_Containers.h"
#include "Render_Math2D.h"
#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {

//-----------------------------------------------------------------------
struct StrokeVertex
{
    CoordType x;
    CoordType y;
    CoordType dist;

    StrokeVertex() {}
    StrokeVertex(CoordType x_, CoordType y_) :
        x(x_), y(y_), dist(0) {}

    bool Distance(const StrokeVertex& val)
    {
        dist = Math2D::Distance(*this, val);
        return dist > 0;
    }
};

//-----------------------------------------------------------------------
// This is an internal class, a stroke path storage. It calculates
// the distance between the neighbor vertices and filters coinciding
// vertices. It's important for stroke calculations. Function ClosePath()
// checks if the first and last vertices coincide and closes the path,
// i.e., removes the extra vertex and sets Close=true
//-----------------------------------------------------------------------
class StrokePath
{
public:
    typedef Scaleform::Render::CoordType CoordType;
    typedef ArrayPaged<StrokeVertex, 4, 16> ContainerType;

    StrokePath(LinearHeap* heap) : Path(heap) {}

    // Add vertices
    //------------------
    void        Clear();
    void        ClearAndRelease();
    CoordType   GetLastX() const { return Path.Back().x; }
    CoordType   GetLastY() const { return Path.Back().y; }
    void        AddVertex(const StrokeVertex& v);
    bool        ClosePath();

    // Access
    //------------------
    UPInt GetVertexCount() const 
    { 
        return Path.GetSize(); 
    }

    const StrokeVertex& GetVertexPrev(UPInt i) const
    {
        if(i == 0) i += Path.GetSize();
        return Path[i - 1];
    }

    const StrokeVertex& GetVertex(UPInt i) const
    {
        return Path[i];
    }

    StrokeVertex& GetVertex(UPInt i)
    {
        return Path[i];
    }

    const StrokeVertex& GetVertexNext(UPInt i) const
    {
        if(++i >= Path.GetSize()) i -= Path.GetSize();
        return Path[i];
    }

    const StrokeVertex& GetVertexForw(UPInt i) const
    {
        i += 2;
        if(i >= Path.GetSize()) i -= Path.GetSize();
        return Path[i];
    }

private:
    ContainerType Path;

    StrokePath(const StrokePath&);
    void operator = (StrokePath&);
};



// Stroker types, separated so that they can also be used in GStrokerAA.
//-----------------------------------------------------------------------
class StrokerTypes
{
public:
    enum LineCapType
    {
        ButtCap,
        SquareCap,
        RoundCap
    };

    enum LineJoinType
    {
        MiterJoin,      // When exceeding miter limit cut at the limit
        MiterBevelJoin, // When exceeding miter limit use bevel join
        RoundJoin,
        BevelJoin
    };
};

//-----------------------------------------------------------------------
class Stroker : public StrokerTypes
{
public:
    typedef Scaleform::Render::CoordType CoordType;

    Stroker(LinearHeap* heap);

    // Setup
    //---------------------
    void    SetToleranceParam(const ToleranceParams& param);
    void    SetWidth(CoordType v)           { Width = v/2;  }
    void    SetLineJoin(LineJoinType v)     { LineJoin = v; }
    void    SetLineCap(LineCapType v)       { StartLineCap = EndLineCap = v; }
    void    SetStartLineCap(LineCapType v)  { StartLineCap = v; }
    void    SetEndLineCap(LineCapType v)    { EndLineCap = v; }
    void    SetMiterLimit(CoordType v)      { MiterLimit = v; }

    // Add vertices and generate the stroke
    //---------------------
    void        Clear();
    CoordType   GetLastX() const { return Path.GetLastX(); }
    CoordType   GetLastY() const { return Path.GetLastY(); }
    void        AddVertex(CoordType x, CoordType y);
    void        ClosePath();
    void        FinalizePath(TessBase* tess);

private:
    // Copying is prohibited
    Stroker(const Stroker&);
    void operator = (Stroker&);

    void generateStroke(TessBase* tess, UPInt start, UPInt end, bool close);

    void calcArc(TessBase* tess,
                 CoordType x,   CoordType y,
                 CoordType dx1, CoordType dy1,
                 CoordType dx2, CoordType dy2);

    void calcMiter(TessBase* tess,
                   const StrokeVertex& v0,
                   const StrokeVertex& v1,
                   const StrokeVertex& v2,
                   CoordType dx1, CoordType dy1,
                   CoordType dx2, CoordType dy2,
                   LineJoinType lineJoin,
                   CoordType miterLimit,
                   CoordType epsilon,
                   CoordType dbevel);

    void calcCap(TessBase* tess,
                 const StrokeVertex& v0,
                 const StrokeVertex& v1,
                 CoordType len,
                 LineCapType cap);

    void calcJoin(TessBase* tess,
                  const StrokeVertex& v0,
                  const StrokeVertex& v1,
                  const StrokeVertex& v2,
                  CoordType len1,
                  CoordType len2);

private:
    LinearHeap*     pHeap;
    StrokePath      Path;
    CoordType       Width;
    LineJoinType    LineJoin;
    LineCapType     StartLineCap;
    LineCapType     EndLineCap;
    CoordType       MiterLimit;
    CoordType       CurveTolerance;
    CoordType       IntersectionEpsilon;
    bool            Closed;
};

//-----------------------------------------------------------------------
inline void Stroker::AddVertex(CoordType x, CoordType y)
{
    Path.AddVertex(StrokeVertex(x, y));
}



// Flash has a weird way of representing the strokes. The stroke
// may consist of a number of paths, that logically represent a 
// closed contour(s), but the paths may be unsorted. To construct 
// a closed contour it's necessary to sort the paths and sometimes
// even revert some of them. It's not a problem for the rasterizer,
// except for some minor slowdown, but critical for the tessellator.
//
// This class sorts the paths and constructs as long chains as 
// possible.
//-----------------------------------------------------------------------
class StrokeSorter
{
public:
    typedef Scaleform::Render::CoordType CoordType;

    struct VertexType
    {
        CoordType x, y;
        UInt8     segType;
        bool      snapX;
        bool      snapY;
    };

    StrokeSorter(LinearHeap* heap);

    void Clear();
    void AddVertex(CoordType x, CoordType y, unsigned segType=Math2D::Seg_LineTo);
    void AddQuad(CoordType x2, CoordType y2, CoordType x3, CoordType y3);
    void FinalizePath();
    void Sort();
    void AddOffset(CoordType offsetX, CoordType offsetY);
    void Snap(CoordType offsetX, CoordType offsetY);


    unsigned GetPathCount() const 
    { 
        return (unsigned)OutPaths.GetSize(); 
    }

    bool IsClosed(unsigned pathIdx) const
    {
        return (OutPaths[pathIdx].numVer & ClosedFlag) != 0;
    }

    unsigned GetVertexCount(unsigned pathIdx) const
    {
        return OutPaths[pathIdx].numVer & NumVerMask;
    }

    const VertexType& GetVertex(unsigned pathIdx, unsigned verIdx) const
    {
        const PathType& p = OutPaths[pathIdx];
        unsigned n = p.numVer & NumVerMask;
        return OutVertices[p.start + ((verIdx < n) ? verIdx : verIdx-n)];
    }

private:
    enum
    {
        NumVerMask  = 0x0FFFFFFF,
        InverseFlag = 0x80000000,
        VisitedFlag = 0x40000000,
        ClosedFlag  = 0x20000000,
        PrependFlag = 0x10000000
    };

    struct PathType
    {
        unsigned start;
        unsigned numVer;
    };

    struct SortedPathType
    {
        CoordType   x, y;
        PathType*   thisPath;
    };

    static bool cmpPaths(const SortedPathType& a, const SortedPathType& b)
    {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }

    void  appendPath(PathType* dst, PathType* src);

    unsigned    findNext(const PathType& outPath) const;
    VertexType* getVertex(unsigned pathIdx, unsigned verIdx)
    {
        const PathType& p = OutPaths[pathIdx];
        unsigned n = p.numVer & NumVerMask;
        return &OutVertices[p.start + ((verIdx < n) ? verIdx : verIdx-n)];
    }

private:
    LinearHeap*                     pHeap;
    ArrayPaged<VertexType, 4, 16>   SrcVertices;
    ArrayPaged<PathType,   4, 16>   SrcPaths;
    ArrayUnsafe<SortedPathType>     SortedPaths;
    ArrayPaged<VertexType, 4, 16>   OutVertices;
    ArrayPaged<PathType,   4, 16>   OutPaths;
    unsigned                        LastVertex;
};

}} // Scaleform::Render

#endif //#ifdef SF_RENDER_ENABLE_STROKER

#endif
