/**********************************************************************

Filename    :   Render_ShapeMeshProvider.h
Created     :   2005-2006
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/
#ifndef SF_Render_ShapeMeshProvider_H
#define SF_Render_ShapeMeshProvider_H

#include "Kernel/SF_List.h"
#include "Kernel/SF_ArrayStaticBuff.h"
#include "Render_TreeNode.h"
#include "Render_Gradients.h"
#include "Render_Primitive.h"
#include "Render_ShapeDataDefs.h"
#include "Render_MeshKey.h"
#include "Render_ShapeDataFloat.h"

namespace Scaleform { namespace Render {

struct MeshGenerator;
struct ToleranceParams;

class TessBase;


//------------------------------------------------------------------------
class TransformerBase
{
public:
    virtual ~TransformerBase() {}
    virtual void Transform(float* x, float* y) const = 0;
};

template<class TransformerType>
class TransformerWrapper : public TransformerBase
{
public:
    const TransformerType* Tr;
    TransformerWrapper() : Tr(0) {}
    TransformerWrapper(const TransformerType* tr) : Tr(tr) {}
    virtual void Transform(float* x, float* y) const { Tr->Transform(x, y); }
};



//------------------------------------------------------------------------
struct MorphShapeData : public RefCountBase<MorphShapeData, Stat_Default_Mem>
{
    typedef ArrayLH_POD<UByte> ContainerType;

    Ptr<ShapeDataInterface>             pMorphTo;
    ArrayLH_POD<UByte>                  Container1; // TO DO: Use more memory-efficient containers
    ArrayLH_POD<UByte>                  Container2;
    ShapeDataFloatTempl<ContainerType>  ShapeData1;
    ShapeDataFloatTempl<ContainerType>  ShapeData2;

    MorphShapeData(ShapeDataInterface* morphTo);
};



//------------------------------------------------------------------------
class ShapeMeshProvider : public MeshProvider_KeySupport
{
friend class TreeCacheShapeLayer;
friend class TreeCacheShape;
public:
    // Image Scale9Grid types, for automatic slicing
    enum I9gType 
    {
        I9gNone,            // No image there
        I9gNormalSlice,     // Normal image for slicing
        I9gMergedSlice      // Merged adjacent 9 images
    };

    struct DrawLayerType
    {
        unsigned StartPos;      // Starting position in shape
        unsigned StartFill;     // Starting position in Fill-to-Style array
        unsigned FillCount;     // Total fill count in layer. 
        unsigned StrokeStyle;
        I9gType  Image9GridType;
    };

    enum { VerBufSize = 256, TriBufSize = 256 };

    ShapeMeshProvider()
        : pShapeData(0), pMorphData(0), IdentityBounds()
    {}

    ShapeMeshProvider(ShapeDataInterface* shape, ShapeDataInterface* shapeMorph = 0);


    ~ShapeMeshProvider()
    {
    }

    void AttachShape(ShapeDataInterface* shape, ShapeDataInterface* shapeMorph = 0);

    // Mesh provider virtual function
    virtual bool    GetData(MeshBase *pmesh, VertexOutput* pout, unsigned meshGenFlags);
    virtual void    OnEvict(MeshBase *pmesh) { SF_UNUSED(pmesh); }
    virtual RectF   GetIdentityBounds() const { return IdentityBounds; }
    virtual RectF   GetBounds(const Matrix2F& m) const;
    virtual RectF   GetCorrectBounds(const Matrix2F& m, StrokeGenerator* gen, const ToleranceParams& tol) const;

    virtual unsigned    GetLayerCount() const { return (unsigned)DrawLayers.GetSize(); }
    virtual unsigned    GetFillCount(unsigned drawLayer, unsigned meshGenFlags) const;
    virtual void        GetFillData(FillData* data, unsigned drawLayer,
                                    unsigned fillIndex, unsigned meshGenFlags);
    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned drawLayer,
                                      unsigned fillIndex, unsigned meshGenFlags);

    // Shape data specific functions (non-virtual)
    unsigned GetFillStyleCount() const { return pShapeData->GetFillStyleCount(); }
    unsigned GetStrokeStyleCount() const { return pShapeData->GetStrokeStyleCount(); }
    void     GetFillStyle(unsigned idx, FillStyleType* p, float morphRatio) const;
    void     GetStrokeStyle(unsigned idx, StrokeStyleType* , float morphRatiop) const;

    unsigned GetLayerStroke(unsigned drawLayer) const { return DrawLayers[drawLayer].StrokeStyle; }
    unsigned GetLayerStartPos(unsigned drawLayer) const { return DrawLayers[drawLayer].StartPos; }

    unsigned GetStyleByFillIdx(unsigned drawLayer, unsigned fillIdx) const
    {
        return FillToStyleTable[DrawLayers[drawLayer].StartFill + fillIdx];
    }

    const ShapeDataInterface* GetShapeData() const { return pShapeData; }
private:
    //--------------------------------------------------------------------
    struct TmpPathInfoType
    {
        unsigned ShapeLayer;
        unsigned Pos;
        unsigned Styles[3];
        unsigned EdgeCount;
    };

    typedef ArrayStaticBuffPOD<TmpPathInfoType, 32> TmpPathsArray;


    //--------------------------------------------------------------------
    void createMorphData();
    void acquireShapeData();
    void countComplexFills(const TmpPathsArray& paths, UPInt i1, UPInt i2, DrawLayerType* dl);

    void createDrawLayers(const TmpPathsArray& paths, UPInt i1, UPInt i2);

    bool checkI9gMergedSlice() const;
    bool checkI9gLayer(const DrawLayerType& dl) const;
    void perceiveImage9GridLayers();
    RectF getLayerBounds(unsigned drawLayer) const;

    ComplexFill* getComplexFill(unsigned drawLayer, unsigned fillIndex, unsigned* imgFillStyle) const;
    Matrix2F     getMorphMatrix(unsigned drawLayer, unsigned fillIndex) const;

    bool generateImage9Grid(Scale9GridInfo* s9g, MeshBase *mesh, VertexOutput* verOut, unsigned drawLayer);

    bool tessellateFill(const Scale9GridInfo* s9g, unsigned drawLayerIdx, MeshBase *pmesh, 
                        VertexOutput* pout, unsigned meshGenFlags);

    bool tessellateStroke(const Scale9GridInfo* s9g, unsigned drawStyleIdx, unsigned shapeLayerIdx,
                          MeshBase *pmesh, VertexOutput* pout, unsigned meshGenFlags);

    void computeImgAdjustMatrix(const Scale9GridData* s9g, unsigned drawLayer, 
                                unsigned imgFillStyle, Matrix2F* mtx);

    bool createNullMesh(VertexOutput* pout);

    bool acquireTessMeshes(TessBase* tess, const Matrix2F& mtx, VertexOutput* pout, 
                           unsigned drawLayerIdx, unsigned strokeStyleIdx, unsigned meshGenFlags,
                           float morphRatio);

    void addFill(MeshGenerator* gen, const ToleranceParams& param,
                 const TransformerBase* tr, unsigned startPos,
                 float morphRatio);

    void addToStrokeSorter(MeshGenerator* gen,
                           const TransformerBase* tr, unsigned startPos,
                           unsigned strokeStyleIdx, float snapOffset,
                           float morphRatio);

    void addStroke(MeshGenerator* gen, const ToleranceParams& param,
                   const TransformerBase* tr, unsigned startPos,
                   unsigned strokeStyleIdx, float snapOffset, 
                   float morphRatio);

    void addHairline(MeshGenerator* gen, const ToleranceParams& param,
                     const TransformerBase* tr, unsigned startPos,
                     unsigned strokeStyleIdx, float snapOffset, 
                     float morphRatio);

    void addStrokeAA(MeshGenerator* gen, const ToleranceParams& param,
                     const TransformerBase* tr, unsigned startPos,
                     unsigned strokeStyleIdx, float snapOffset, 
                     float morphRatio);

    ShapePathType readPathInfo(ShapePosInfo* pos1, ShapePosInfo* pos2, 
                               float* coord, unsigned* styles, float morphRatio) const;

    PathEdgeType  readEdge(ShapePosInfo* pos1, ShapePosInfo* pos2, float* coord, 
                           float morphRatio) const;

    void          skipPathData(ShapePosInfo* pos1, ShapePosInfo* pos2) const;

    ArrayLH_POD<DrawLayerType>  DrawLayers;         // TO DO: More efficient memory use!
    ArrayLH_POD<unsigned>       FillToStyleTable;
    Ptr<ShapeDataInterface>     pShapeData;
    Ptr<MorphShapeData>         pMorphData;
    RectF                       IdentityBounds;
};


}} // Scaleform::Render


#endif

