/**********************************************************************

Filename    :   DrawingContext.h
Content     :   Drawing API implementation
Created     :   Aug 7, 2007
Authors     :   Maxim Shemanarev, Artem Bolgar

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.
Notes       :   


Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_GFXDRAWINGCONTEXT_H
#define INC_SF_GFX_GFXDRAWINGCONTEXT_H

#include "GFx/GFx_ShapeSwf.h"
#include "Render/Render_ShapeDataPacked.h"
#include "Render/Render_States.h"
#include "Render/Render_TreeNode.h"

namespace Scaleform { namespace GFx {

class ImageResource;
class ImageCreator;

//
// DrawingContext
//

class DrawingContext : public RefCountBaseNTS<DrawingContext, StatMV_MovieClip_Mem>, public ListNode<DrawingContext>
{
    friend class MovieImpl;
public:
    typedef Matrix2F                    Matrix;
    typedef Render::Cxform              Cxform;
    typedef Render::BlendMode           BlendType;
    typedef Render::FillStyleType       FillStyle;
    typedef Render::StrokeStyleType     LineStyle;
    typedef ArrayDH<UByte>              ContainerType;
    typedef Render::ShapeDataPacked<ContainerType> ShapeType;
    typedef Render::ComplexFill         ComplexFill;
    typedef Render::TreeContainer       TreeContainer;
    typedef Render::TreeShape           TreeShape;
    typedef Render::GradientData        GradientData;

protected: // only MovieImpl can create DrawingContext
    DrawingContext(MemoryHeap* pheap, Render::Context&, ImageCreator* imgCreator);
public:
    ~DrawingContext();

    // Add accumulated data from FGxPackedPath to GFxShapeCharacterDef.
    // Returns true if the path is not empty.
    // Function is used internally. 
    bool    AcquirePath(bool newShapeFlag);

    void    SetNonZeroFill(bool fill);

    void    BeginSolidFill(unsigned rgba);
    bool    IsSolidFill(unsigned rgba) const;

    // begins fill with current style
    void    BeginFill();

    void    BeginBitmapFill(FillType fillType,
                            ImageResource* pimageRes,
                            const Matrix& mtx);

    void    Clear();
    void    ComputeBound(RectF *pRect) const;

    // creates new fill style with ComplexFill and returns ComplexFill.
    ComplexFill* CreateNewComplexFill();
    // creates new line style with ComplexFill and returns ComplexFill.
    ComplexFill* CreateLineComplexFill();

    void    EndFill();

    // all coordinates are in twips!
    void    MoveTo(float x, float y);
    void    LineTo(float x, float y);
    void    CurveTo(float cx, float cy, float ax, float ay);

    bool    DefPointTestLocal(const Render::PointF &pt, bool testShape = 0, const DisplayObjectBase *pinst = 0) const;

    //void StartNewShape() { Shapes->StartNewShape(); }
    void    SetNoLine();
    void    SetNoFill();

    // lineWidth - in pixels
    void    ChangeLineStyle(float lineWidth, 
                            unsigned  rgba, 
                            bool  hinting, 
                            unsigned scaling, 
                            unsigned caps,
                            unsigned joins,
                            float miterLimit);

    bool    NoLine() const;

    bool    IsEmpty() const 
    { 
        return !Shapes || Shapes->IsEmpty();
    }

    bool    IsDirty() const { return (States & State_Dirty) != 0; }

    TreeContainer*                  GetTreeNode() { return pTreeContainer; }
    Render::Context&                GetRenderContext() const { return RenContext; }

    void                            UpdateRenderNode();

protected:
    // The function begins new fill with an "empty" style. 
    // It returns an index of just created fill style, so the caller
    // can set any values. The function is used in Action Script, beginGradientFill
    unsigned                        SetNewFill();

    void                            NewPath(float x, float y);
    void                            FinishPath();

    // lineWidth - in twips
    void                            SetLineStyle(float lineWidth, 
                                                 unsigned rgba, 
                                                 bool hinting, 
                                                 unsigned scaling, 
                                                 unsigned caps,
                                                 unsigned joins,
                                                 float miterLimit);

    // lineWidth - in twips
    bool                            SameLineStyle(float lineWidth, 
                                                  unsigned rgba, 
                                                  bool hinting, 
                                                  unsigned scaling, 
                                                  unsigned caps,
                                                  unsigned joins,
                                                  float miterLimit) const;
protected:
    MemoryHeap*                 pHeap;
    Ptr<TreeContainer>          pTreeContainer;
    Render::Context&            RenContext;
    Ptr<ImageCreator>           ImgCreator;
    ContainerType               Container;
    Ptr<ShapeType>              Shapes;
    FillStyle                   mFillStyle;
    LineStyle                   mLineStyle;
    unsigned                    StrokeStyle;
    unsigned                    FillStyle0, FillStyle1;
    ShapePosInfo                PosInfo;
    float                       Ex, Ey;
    float                       StX, StY;
    enum
    {
        State_NewShapeFlag   = 0x01,
        State_FreshLineStyle = 0x02,
        State_FreshFillStyle = 0x04,
        State_NeedEndPath    = 0x08,
        State_NeedClosePath  = 0x10,
        State_Dirty          = 0x80
    };
    UByte                       States;

private:
    DrawingContext& operator=(const DrawingContext&) { return *this; }
};

}} // namespace Scaleform::GFx

#endif //INC_SF_GFX_GFXDRAWINGCONTEXT_H
