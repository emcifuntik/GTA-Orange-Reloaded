/**********************************************************************

PublicHeader:   Render
Filename    :   Render_ShapeDataDefs.h
Content     :   
Created     :   2007
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   Compact path data storage

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_ShapeDataDefs_H
#define INC_SF_Render_ShapeDataDefs_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Matrix2x4.h"
#include "Render/Render_Image.h"
#include "Render/Render_Gradients.h"

namespace Scaleform { namespace Render {

//------------------------------------------------------------------------
struct ShapePosInfo
{
    unsigned Pos;
    int StartX, StartY;
    int LastX, LastY;
    unsigned FillBase, StrokeBase;
    unsigned NumFillBits, NumStrokeBits;
    unsigned Fill0, Fill1, Stroke;
    float Sfactor;
    bool Initialized;

    ShapePosInfo() : Pos(0), StartX(0), StartY(0), LastX(0), LastY(0), 
        FillBase(0), StrokeBase(0), NumFillBits(0), NumStrokeBits(0), 
        Fill0(0), Fill1(0), Stroke(0), Sfactor(1.0f), Initialized(false) {}
    ShapePosInfo(unsigned pos) : Pos(pos), StartX(0), StartY(0), 
        LastX(0), LastY(0),         FillBase(0), StrokeBase(0), NumFillBits(0), NumStrokeBits(0), 
        Fill0(0), Fill1(0), Stroke(0), Sfactor(1.0f), Initialized(false) {}

};

// Path types when calling ShapeReader::ReadPathInfo
//------------------------------------------------------------------------
enum ShapePathType
{
    Shape_NewPath  = 1, // New path begins
    Shape_NewLayer = 2, // New layer and new path begins
    Shape_EndShape = 0  // No more shape data
};

// Path segment types
//------------------------------------------------------------------------
enum PathEdgeType
{
    Edge_LineTo  = 1,   // Straight line
    Edge_QuadTo  = 2,   // Quadratic curve
    Edge_EndPath = 0,   // No more path data

    Edge_MaxCoord = 4   // Max number of coordinates to read. For Quadratic curves only it's 4,
                        // for cubic ones it must be 6
};

//------------------------------------------------------------------------
enum StrokeStyleFlags
{
    StrokeFlag_StrokeHinting  = 0x0001,

    // Scaling
    StrokeScaling_Normal      = 0x0000,
    StrokeScaling_Horizontal  = 0x0002,
    StrokeScaling_Vertical    = 0x0004,
    StrokeScaling_None        = 0x0006,
    StrokeScaling_Mask        = 0x0006,

    // Complex fill flag
    StrokeFlag_ComplexFill    = 0x0008,

    // Joins
    StrokeJoin_Round          = 0x0000,
    StrokeJoin_Bevel          = 0x0010,
    StrokeJoin_Miter          = 0x0020,
    StrokeJoin_Mask           = 0x0030,

    // Stroke start caps
    StrokeCap_Round           = 0x0000,
    StrokeCap_None            = 0x0040,
    StrokeCap_Square          = 0x0080,
    StrokeCap_Mask            = 0x00C0,

    // Stroke end caps (internal use only)
    StrokeEndCap_Round        = 0x0000,
    StrokeEndCap_None         = 0x0100,
    StrokeEndCap_Square       = 0x0200,
    StrokeEndCap_Mask         = 0x0300
};

class Image;
class GradientData;

//------------------------------------------------------------------------
struct ComplexFill : public RefCountBase<ComplexFill, Stat_Default_Mem>
{
    Ptr<Image>          pImage;
    Ptr<GradientData>   pGradient;
    Matrix2F            ImageMatrix;
    ImageFillMode       FillMode;
    unsigned            BindIndex;

    ComplexFill():BindIndex(~0u) {}
};

//------------------------------------------------------------------------
struct FillStyleType
{
    UInt32              Color;
    Ptr<ComplexFill>    pFill;
};

//------------------------------------------------------------------------
struct StrokeStyleType
{
    float               Width;      // Line width in local units
    float               Units;      // String width units, 1.0 for pixels, 1/20 for TWIPS. Affects StrokeScaling_None
    unsigned            Flags;      // See StrokeStyleFlags
    float               Miter;      // Miter limit
    UInt32              Color;
    Ptr<ComplexFill>    pFill;
};


// ShapeDataInterface
//------------------------------------------------------------------------
//    Usage (example):
//
//    ShapeDataFloat data(. . .);
//
//    unsigned fills = data.GetFillStyleCount();
//    unsigned strokes = data.GetStrokeStyleCount();
//    . . .
//
//    ShapePosInfo pos(startPos);
//    ShapePathType pathType;
//    float coords[Edge_MaxCoord];
//    unsigned styles[3];
//
//    while((pathType = data.ReadPathInfo(&pos, coords, styles)) != Shape_EndShape)
//    {
//        // coord contain MoveTo(x, y);
//        // styles contain LeftStyle, RightStyle, StrokeStyle
//        PathEdgeType edgeType;
//        while((edgeType = data.ReadEdge(&pos, coords)) != Edge_EndPath)
//        {
//            if(edgeType == Edge_LineTo)
//            {
//                // Line
//            }
//            else
//            if(edgeType == Edge_QuadTo)
//            {
//                // Quadratic Curve
//            }
//        }
//    }
//
//------------------------------------------------------------------------



//------------------------------------------------------------------------
class ShapeDataInterface : public RefCountBase<ShapeDataInterface, Stat_Default_Mem>
{
public:
    virtual bool     IsEmpty() const { return false; }
    virtual unsigned GetFillStyleCount()   const = 0;
    virtual unsigned GetStrokeStyleCount() const = 0;

    // idx is in range [1...GetFillStyleCount()] (inclusive)
    virtual void GetFillStyle(unsigned idx, FillStyleType* p) const = 0;

    // idx is range [1...GetStrokeStyleCount()] (inclusive)
    virtual void GetStrokeStyle(unsigned idx, StrokeStyleType* p) const = 0;

    // The implementation must provide some abstract index (position)
    // to quickly navigate to the beginning of the shape data. In simple case,
    // when the data is stored is an isolated array it can just return 0.
    virtual unsigned GetStartingPos() const = 0;

    // Read path info. Returns Shape_NewPath, Shape_NewLayer, or Shape_EndShape and 
    // starting path info. Shape_NewLayer is the same as Shape_NewPath, but just
    // indicates that it's a new layer.
    // Coord must be at least float[2], styles must be unsigned[3].
    virtual ShapePathType ReadPathInfo(ShapePosInfo* pos, float* coord, unsigned* styles) const = 0;

    // Read next edge. Returns Edge_LineTo, Edge_QuadTo, or Edge_EndPath. 
    // Coord must be float[Seg_MaxCoord].
    virtual PathEdgeType ReadEdge(ShapePosInfo* pos, float* coord) const = 0;

    // Skip path data to the next path. Pos must point to the beginning
    // of the path data (right after ReadPathInfo). By default it just iterates
    // through all edges, but can be overridden for more optimal implementation.
    virtual void SkipPathData(ShapePosInfo* pos) const
    {
        PathEdgeType edgeType;
        float coords[Edge_MaxCoord];
        while((edgeType = ReadEdge(pos, coords)) != Edge_EndPath);
    }

    // Skip path data and count edges (non-virtual). Returns the number of edges in the path.
    unsigned CountPathEdges(ShapePosInfo* pos) const
    {
        PathEdgeType edgeType;
        float coords[Edge_MaxCoord];
        unsigned edgeCount = 0;
        while((edgeType = ReadEdge(pos, coords)) != Edge_EndPath)
        {
            ++edgeCount;
        }
        return edgeCount;
    }
};



}} // Scaleform::Render

#endif

