/*********************************************************************

PublicHeader:   Render
Filename    :   Render_Scale9Grid.h
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

#ifndef INC_SF_Render_Scale9Grid_H
#define INC_SF_Render_Scale9Grid_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_ArrayStaticBuff.h"
#include "Render_Matrix2x4.h"
#include "Render_ShapeDataDefs.h"
#include "Render_Vertex.h"



namespace Scaleform { namespace Render {


struct Scale9GridData;

//------------------------------------------------------------------------
struct Scale9GridRect : public RefCountBase<Scale9GridRect, Stat_Default_Mem>
{
    RectF Scale9;
};



//------------------------------------------------------------------------
struct Scale9GridInfo : public RefCountBase<Scale9GridInfo, Stat_Default_Mem>
{
    RectF               Scale9;
    Matrix2F            S9gMatrix;
    Matrix2F            ShapeMatrix;
    RectF               Bounds;

    RectF               ResultingGrid;
    Matrix2F            InverseMatrix;
    Matrix2F            ResultingMatrices[9];
 
    Scale9GridInfo() {}
    Scale9GridInfo(const Scale9GridData* s9g, const Matrix2F& viewMtx);

    unsigned    Transform(float* x, float* y) const;
    unsigned    GetAreaCode(float x, float y) const;
    RectF       AdjustBounds(const RectF& bounds) const;
};


//------------------------------------------------------------------------
struct Image9GridVertex
{
    float   x,y;
    float   u,v;

    void    Set(float x_, float y_, float u_, float v_)
    {
        x = x_;
        y = y_;
        u = u_;
        v = v_;
    }

    static VertexElement Elements[3];
    static VertexFormat  Format;
};



//------------------------------------------------------------------------
struct Scale9GridTess
{
    struct TmpVertexType
    {
        unsigned AreaCode;
        unsigned VerIdx;
        float    Slope;
    };

    typedef ArrayStaticBuffPOD<TmpVertexType, 8*9> TmpVertices;


public:
    Scale9GridTess(MemoryHeap* heap, const Scale9GridInfo& info, const RectF& imgRect, 
                   const Matrix2F& uvMatrix, const Matrix2F& fillMatrix);

    unsigned      GetVertexCount()  const { return VerCount; }
    Image9GridVertex* GetVertices()       { return Vertices; }
    unsigned      GetIndexCount()   const { return (unsigned)Indices.GetSize(); }
    UInt16*       GetIndices()            { return &Indices[0]; }


private:
    unsigned getAreaCode(const RectF& r, float x, float y) const;
    void addVertex(TmpVertices& ver, float x, float y, float u, float v, unsigned areaCode);
    void addVertices(TmpVertices& ver, const Matrix2F& toUV, float x, float y, unsigned code1, unsigned code2);
    void addCorner(TmpVertices& ver, const float* coord, float x, float y, const Matrix2F& toUV, 
                   unsigned code1, unsigned code2, unsigned code3, unsigned code4);
    void transformVertex(const Scale9GridInfo& s9g, Image9GridVertex& v) const;
    void tessellateArea(TmpVertices& ver, unsigned i1, unsigned i2);

    static bool cmpCodes(const TmpVertexType& a, const TmpVertexType& b)
    {
        return a.AreaCode < b.AreaCode;
    }

    static bool cmpSlopes(const TmpVertexType& a, const TmpVertexType& b)
    {
        return a.Slope < b.Slope;
    }

    Image9GridVertex                Vertices[24];
    unsigned                        VerCount;
    ArrayStaticBuffPOD<UInt16, 8*9> Indices;
};




}} // Scaleform::Render

#endif
