/**********************************************************************

Filename    :   DisplayObjectBase.cpp
Content     :   Implementation of DisplayObjectBase
Created     :   
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_InteractiveObject.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_ASUtils.h"
#include "GFx/GFx_DrawingContext.h"
#include "Render/Render_TreeLog.h"

namespace Scaleform { namespace GFx {

// ***** DisplayObjectBase   

DisplayObjectBase::DisplayObjectBase
    (ASMovieRootBase* pasRoot, InteractiveObject* parent, ResourceId id)
:
    pASRoot(pasRoot),
    Id(id),
    Depth(INVALID_DEPTH),
    CreateFrame(0),
    pParent(parent),
    pGeomData(0),
    ClipDepth(0),
    Flags(0),
    BlendMode((UInt8)Render::Blend_None),
    AvmObjOffset(0),
    FOV(0)
{
    SF_ASSERT(pASRoot);
    SF_ASSERT((parent == NULL /*&& (Id == ResourceId::InvalidId || Id == 0)*/) ||
        (parent != NULL && Id.GetIdIndex() >= 0));
    SetVisibleFlag();
}

DisplayObjectBase::~DisplayObjectBase()
{
    if (pGeomData)
        delete pGeomData;
    if (HasAvmObject())
        GetAvmObjImpl()->~AvmDisplayObjBase();
}

Render::Context& DisplayObjectBase::GetRenderContext() const 
{ 
    return GetMovieImpl()->GetRenderContext(); 
}

Render::TreeNode* DisplayObjectBase::GetRenderNode() const
{
    if (!pRenNode)
    {
        pRenNode = CreateRenderNode(GetMovieImpl()->GetRenderContext());
        SF_ASSERT(pRenNode);
        pRenNode->SetVisible(IsVisibleFlagSet());
    }
    return pRenNode;
}

Render::TreeContainer* DisplayObjectBase::ConvertToTreeContainer() const
{
    if (!pRenNode) GetRenderNode();
    SF_ASSERT(pRenNode);

    SF_ASSERT(!pRenNode->IsMaskNode()); // @TODO mask support
    Render::TreeContainer* cont = static_cast<Render::TreeContainer*>(pRenNode->GetParent());

    // create a container....
    Ptr<Render::TreeContainer> newnode = *GetRenderContext().CreateEntry<Render::TreeContainer>();
    if (cont)
    {
        // find the index of pRenNode first
        UPInt i = 0;
        for (UPInt n = cont->GetSize(); i < n; ++i)
        {
            Render::TreeNode* nd = cont->GetAt(i);
            if (nd == pRenNode)
                break;
        }
        SF_ASSERT(i < cont->GetSize());

        cont->Remove(i, 1);

        // reinsert a new node at appropriate position and add old node as its child....
        cont->Insert(i, newnode.GetPtr());
    }
    // need to transfer matrix and color transform from old to new node;
    // old node should be set to Identity for both matrix and cxform.
    newnode->SetMatrix(GetMatrix());
    pRenNode->SetMatrix(Matrix2F::Identity);

    newnode->SetCxform(GetCxform());
    pRenNode->SetCxform(Cxform::Identity);

    newnode->Add(pRenNode);

    pRenNode = newnode;
    return newnode;
}

const Matrix& DisplayObjectBase::GetMatrix() const                   
{ 
    if (pRenNode)
        return pRenNode->M2D();
    return Matrix::Identity; 
}

void DisplayObjectBase::SetMatrix(const Matrix& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    GetRenderNode()->SetMatrix(m);
}

const Matrix3F& DisplayObjectBase::GetMatrix3D() const                   
{ 
    if (pRenNode)       // check Is3D() first?
        return pRenNode->M3D();
    return Matrix3F::Identity; 
}

void DisplayObjectBase::SetMatrix3D(const Matrix3F& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    GetRenderNode()->SetMatrix3D(m);
}

bool DisplayObjectBase::GetViewMatrix3D(Matrix3F *m, bool bInherit) const
{ 
    if (pRenNode && pRenNode->GetViewMatrix3D(m))
        return true;

    if (!bInherit)
        return false;

#if 1                   // TEMP
    if (!pParent)       // inheriting but no parent so get from movieimpl
    {
        *m = *GetMovieImpl()->GetViewMatrix3D();  
        return true;
    }
#endif

    return pParent->GetViewMatrix3D(m, bInherit);
}

void DisplayObjectBase::SetViewMatrix3D(const Matrix3F& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    GetRenderNode()->SetViewMatrix3D(m);
}

bool DisplayObjectBase::GetProjectionMatrix3D(Matrix4F *m, bool bInherit) const
{ 
    if (pRenNode && pRenNode->GetProjectionMatrix3D(m))
        return true;

    if (!bInherit)
        return false;

#if 1                   // TEMP
    if (!pParent)       // inheriting but no parent so get from movieimpl
    {
        *m = *GetMovieImpl()->GetProjectionMatrix3D();  
        return true;
    }
#endif

    return pParent->GetProjectionMatrix3D(m, bInherit);
}

void DisplayObjectBase::SetProjectionMatrix3D(const Matrix4F& m)          
{ 
    SF_ASSERT(m.IsValid()); 
    GetRenderNode()->SetProjectionMatrix3D(m);
}

void DisplayObjectBase::UpdateViewAndPerspective()
{
    MovieImpl * pMovie = GetMovieImpl();
    Matrix3F view;
    Matrix4F proj;
    RectF vfr = pMovie->GetVisibleFrameRectInTwips();
    float displayWidth  = fabsf(vfr.Width());     
    float displayHeight = fabsf(vfr.Height());      
    float centerX       = (vfr.x1 + vfr.x2)/2.f;
    float centerY       = (vfr.y1 + vfr.y2)/2.f;
    MovieImpl::MakeViewAndPersp3D(&view, &proj, displayWidth, displayHeight, PointF(centerX, centerY));
    SetViewMatrix3D(view);
    SetProjectionMatrix3D(proj);
}

// MOOSE TODO - support perspective FOV
void DisplayObjectBase::SetFOV(Double fovdeg)
{
    if (NumberUtil::IsNaN(fovdeg))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(fovdeg) || NumberUtil::IsPOSITIVE_INFINITY(fovdeg))
        fovdeg = 0;
#if 0
    GFxMovieRoot *pRoot = GetMovieRoot();
    if (fovdeg != FOV && pRoot && pRoot->GetRenderer())
    {
        FOV = fovdeg; 
        Matrix3F matView;
        Matrix4F matPersp;
//        pRoot->GetRenderer()->MakeViewAndPersp3D(pRoot->VisibleFrameRect, matView, matPersp, PerspFOV);
        SetProjectionMatrix3D(matPersp);
        SetViewMatrix3D(matView);
    }
#else
    FOV = fovdeg; 
#endif
}

// MOOSE TODO - support perspective FOV
Double DisplayObjectBase::GetFOV() const
{
#if 0
    if (FOV != 0 || checkAncestors == false)
        return FOV; 

    if (pParent == NULL)
        return GetMovieRoot() ? GetMovieRoot()->FOV : FOV;

    return pParent->GetFOV(checkAncestors);
#else
    return FOV;
#endif
}

const Cxform& DisplayObjectBase::GetCxform() const                   
{ 
    if (pRenNode)
        return pRenNode->GetCxform();
    return Cxform::Identity; 
}

void DisplayObjectBase::SetCxform(const Cxform& cx)         
{ 
    GetRenderNode()->SetCxform(cx);
}

void DisplayObjectBase::ConcatenateCxform(const Cxform& cx) 
{ 
    Cxform cx1(GetCxform());
    cx1.Prepend(cx); 
    SetCxform(cx1);
}

void DisplayObjectBase::ConcatenateMatrix(const Matrix& m)  
{ 
    Matrix m1(GetMatrix());
    m1.Prepend(m);
    SetMatrix(m1); 
}

void DisplayObjectBase::SetBlendMode(BlendType blend)
{ 
	GetRenderNode()->SetBlendMode(blend);
	BlendMode = (UInt8)blend; 
}

DisplayObjectBase::BlendType DisplayObjectBase::GetBlendMode() const
{
	if (GetRenderNode())
		return GetRenderNode()->GetBlendMode();
	return (BlendType)BlendMode;
}

void DisplayObjectBase::SetVisibleFlag(bool v) 
{ 
    if (v)
    {
        Flags |= Mask_Visible;
        if (pRenNode)
            pRenNode->SetVisible(v);
    }
    else
    {
        Flags &= (~Mask_Visible);
        GetRenderNode()->SetVisible(false);
    }
}

void DisplayObjectBase::BindAvmObj(AvmDisplayObjBase* p) 
{ 
    SF_ASSERT(AvmObjOffset == 0);
    SF_ASSERT((size_t)((UByte*)p - (UByte*)this) < (size_t)1024); 
    SF_ASSERT((((size_t)p) & 0x3) == 0); // 4-bytes aligned 

    AvmObjOffset = UInt8(((size_t)((UByte*)p - (UByte*)this) + 3)/4);
}


// Character transform implementation.
void DisplayObjectBase::GetWorldMatrix(Matrix *pmat) const
{           
    if (pParent)
    {
        pParent->GetWorldMatrix(pmat);
        pmat->Prepend(GetMatrix());
    }
    else
    {
        *pmat = GetMatrix();
    }
}
 
void DisplayObjectBase::GetWorldMatrix3D(Matrix3F *pmat) const
{
    if (pParent)
    {
        pParent->GetWorldMatrix3D(pmat);
        pmat->Prepend(GetMatrix3D());
    }
    else
    {
        *pmat = GetMatrix3D();
    }
}

// Character transform implementation.
void DisplayObjectBase::GetLevelMatrix(Matrix *pmat) const
{           
    if (pParent)
    {
        pParent->GetLevelMatrix(pmat);
        pmat->Prepend(GetMatrix());
    }
    else
    {
        *pmat = Matrix();
    }
}

// Character color transform implementation.
void DisplayObjectBase::GetWorldCxform(Cxform *pcxform) const
{
    if (pParent)
    {
        pParent->GetWorldCxform(pcxform);
        pcxform->Prepend(GetCxform());
    }
    else
    {
        *pcxform = GetCxform();     
    }
}

void DisplayObjectBase::TransformPointToLocal(Render::PointF & p, const Render::PointF & pt, bool bPtInParentSpace) const
{
    if ( !Is3D() )
    {
        if (bPtInParentSpace)
            GetMatrix().TransformByInverse(&p, pt);
        else
        {
            Matrix2F worldMat;
            GetWorldMatrix(&worldMat);
            worldMat.TransformByInverse(&p, pt);
        }
    }
    else
    {
        Matrix3F view;
        Matrix4F proj;
        Matrix3F world = GetWorldMatrix3D();
        Render::ScreenToWorld & stw = GetMovieImpl()->ScreenToWorld;
        
        GetProjectionMatrix3D(&proj, true /* inherit */);
        stw.SetProjection(proj);

        GetViewMatrix3D(&view, true /* inherit */);
        stw.SetView(view);

        stw.SetWorld(world);
        stw.GetWorldPoint(&p);       
    }
}
// Used during rendering.

// Temporary - used until blending logic is improved.
Render::BlendMode DisplayObjectBase::GetActiveBlendMode() const
{
    Render::BlendMode blend = Render::Blend_None;
    const DisplayObjectBase*    pchar = this;

    while (pchar)
    {
        blend = pchar->GetBlendMode();
        if (blend > Render::Blend_Layer)
            return blend;
        pchar = pchar->GetParent();
    }
    // Return last blend mode.
    return blend;
}


// This is not because of MovieDefImpl dependency not available in the header.
unsigned DisplayObjectBase::GetVersion() const
{
    return GetResourceMovieDef()->GetVersion();
}

// (needs to be implemented in .cpp, so that MovieImpl is visible)
Log*     DisplayObjectBase::GetLog() const
{
    // By default, GetMovieImpl will delegate to parent.
    // GFxSprite should override GetMovieImpl to return the correct object.
    return FindMovieImpl()->GetCachedLog();
}
bool        DisplayObjectBase::IsVerboseAction() const
{
    return FindMovieImpl()->IsVerboseAction();
}

bool        DisplayObjectBase::IsVerboseActionErrors() const
{
    return !FindMovieImpl()->IsSuppressActionErrors();
}

void DisplayObjectBase::OnEventUnload()
{
    SetUnloading();
    // should it be before Event_Unload or after? (AB)
    if (IsTopmostLevelFlagSet())
    {
        FindMovieImpl()->RemoveTopmostLevelCharacter(this);
    }
    if (!IsUnloaded())
    {
        OnEvent(EventId::Event_Unload);
        SetUnloaded();
    }
}

bool DisplayObjectBase::OnUnloading()
{
    return true;
}

bool DisplayObjectBase::CheckLastHitResult(float x, float y) const
{
    return 
        (Flags & Mask_HitTest) != 0 &&
        x == LastHitTestX && y == LastHitTestY;
}

void DisplayObjectBase::SetLastHitResult(float x, float y, bool result) const // TO DO: Revise "const"
{
    LastHitTestX = x;
    LastHitTestY = y;
    Flags &= ~Mask_HitTest;
    Flags |= result ? Mask_HitTestPositive : Mask_HitTestNegative;
}

void DisplayObjectBase::InvalidateHitResult() const                           // TO DO: Revise "const"
{
    Flags &= ~Mask_HitTest;
}

// MovieImpl*   DisplayObjectBase::GetMovieImpl() const                
// { 
//     return (pParentChar) ? pParentChar->GetMovieImpl() : NULL; 
// }

DisplayObjectBase::GeomDataType& DisplayObjectBase::GetGeomData(GeomDataType& geomData) const
{
    if (!pGeomData)
    {
        // fill GeomData using Matrix_1
        const Matrix& m = GetMatrix();
        geomData.X = int(m.Tx());
        geomData.Y = int(m.Ty());
        geomData.XScale = m.GetXScaleDouble()*(Double)100.;
        geomData.YScale = m.GetYScaleDouble()*(Double)100.;
        geomData.Rotation = (m.GetRotationDouble()*(Double)180.)/SF_MATH_PI;
        geomData.OrigMatrix = GetMatrix();
    }
    else
    {
        geomData = *pGeomData;
    }
    return geomData;
}

void    DisplayObjectBase::SetGeomData(const GeomDataType& gd)
{
    if (pGeomData)
        *pGeomData = gd;
    else
        pGeomData = SF_HEAP_AUTO_NEW(this) GeomDataType(gd);
}

void    DisplayObjectBase::EnsureGeomDataCreated()
{
    if (!pGeomData)
    {
        GeomDataType geomData;
        SetGeomData(GetGeomData(geomData));
    }
}

void ASCharacter_MatrixScaleAndRotate2x2(Render::Matrix2F& m, float sx, float sy, float radians)
{
    float   cosAngle = cosf(radians);
    float   sinAngle = sinf(radians);
    float   x00 = m.Sx();
    float   x01 = m.Shx();
    float   x10 = m.Shy();
    float   x11 = m.Sy();

    m.Sx() = (x00*cosAngle-x10*sinAngle)*sx;
    m.Shx() = (x01*cosAngle-x11*sinAngle)*sy;
    m.Shy() = (x00*sinAngle+x10*cosAngle)*sx;
    m.Sy() = (x01*sinAngle+x11*cosAngle)*sy;
}

void DisplayObjectBase::SetX(Double x)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(x))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(x) || NumberUtil::IsPOSITIVE_INFINITY(x))
        x = 0;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix  m = GetMatrix();
    pGeomData->X = int(floor(PixelsToTwips(x)));
    m.Tx() = (float) pGeomData->X;
    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetY(Double y)
{
    if (NumberUtil::IsNaN(y))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(y) || NumberUtil::IsPOSITIVE_INFINITY(y))
        y = 0;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix  m = GetMatrix();
    pGeomData->Y = int(floor(PixelsToTwips(y)));
    m.Ty() = (float) pGeomData->Y;
    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetZ(Double z)
{
    if (NumberUtil::IsNaN(z))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(z) || NumberUtil::IsPOSITIVE_INFINITY(z))
        z = 0;

    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline

    SF_ASSERT(pGeomData);

    pGeomData->Z = z;
    UpdateTransform3D();
}

void DisplayObjectBase::SetXScale(Double xscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(xscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(xscale) || NumberUtil::IsPOSITIVE_INFINITY(xscale))
    {
        return;
    }
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    const Matrix& chm = GetMatrix();
    Matrix m = pGeomData->OrigMatrix;
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Double origXScale = m.GetXScaleDouble();
    pGeomData->XScale = xscale;
    if (origXScale == 0 || xscale > 1E+16)
    {
        xscale = 0;
        origXScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(xscale/(origXScale*100.)), 
        float(pGeomData->YScale/(m.GetYScaleDouble()*100.)),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetYScale(Double yscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(yscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(yscale) || NumberUtil::IsPOSITIVE_INFINITY(yscale))
    {
        return;
    }
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    const Matrix& chm = GetMatrix();
    Matrix m = pGeomData->OrigMatrix;
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Double origYScale = m.GetYScaleDouble();
    pGeomData->YScale = yscale;
    if (origYScale == 0 || yscale > 1E+16)
    {
        yscale = 0;
        origYScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(pGeomData->XScale/(m.GetXScaleDouble()*100.)), 
        float(yscale/(origYScale* 100.)),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetZScale(Double zscale)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(zscale) ||
        NumberUtil::IsNEGATIVE_INFINITY(zscale) || NumberUtil::IsPOSITIVE_INFINITY(zscale))
    {
        return;
    }

    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline

    SF_ASSERT(pGeomData);

    pGeomData->ZScale = zscale;
    UpdateTransform3D();
}

void DisplayObjectBase::SetRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Double r = fmod((Double)rotation, (Double)360.);
    if (r > 180)
        r -= 360;
    else if (r < -180)
        r += 360;
    pGeomData->Rotation = r;

    const Matrix& chm = GetMatrix();
    Matrix m = pGeomData->OrigMatrix;
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Double origRotation = m.GetRotationDouble();

    // remove old rotation by rotate back and add new one

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(pGeomData->XScale/(m.GetXScaleDouble()*100.)), 
        float(pGeomData->YScale/(m.GetYScaleDouble()*100.)),
        float(-origRotation + r * SF_MATH_PI / 180.));

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetXRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    
    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline
    
    SF_ASSERT(pGeomData);

    Double r = fmod((Double)rotation, (Double)360.);
    if (r > 180)
        r -= 360;
    else if (r < -180)
        r += 360;
    pGeomData->XRotation = r;
    UpdateTransform3D();
}

void DisplayObjectBase::SetYRotation(Double rotation)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(rotation))
        return;
    
    if (pASRoot && pASRoot->GetMovieImpl()->AcceptAnimMovesWith3D())
        EnsureGeomDataCreated();
    else
        SetAcceptAnimMoves(0);      // default for AS3 is this, to not allow timeline
    
    SF_ASSERT(pGeomData);

    Double r = fmod((Double)rotation, (Double)360.);
    if (r > 180)
        r -= 360;
    else if (r < -180)
        r += 360;
    pGeomData->YRotation = r;
    UpdateTransform3D();
}

void DisplayObjectBase::SetWidth(Double width)
{
    // MA: Width/Height modification in Flash is unusual in that it performs
    // relative axis scaling in the x local axis (width scaling) and y local
    // axis (height scaling). The resulting width after scaling does not
    // actually equal the original, instead, it is related to rotation!
    // AB: I am second on that! Looks like a bug in Flash.

    // NOTE: Although it works for many cases, this is still not correct. Modification 
    // of width seems very strange (if not buggy) in Flash.
    if (NumberUtil::IsNaN(width) || NumberUtil::IsNEGATIVE_INFINITY(width))
        return;
    if (NumberUtil::IsPOSITIVE_INFINITY(width))
        width = 0;

    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix m = pGeomData->OrigMatrix;
    const Matrix& chm = GetMatrix();
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Matrix im = m;
    im.AppendRotation(float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    float oldWidth      = GetBounds(im).Width(); // width should be in local coords!
    float newWidth      = float(PixelsToTwips(width));
    float multiplier    = (fabsf(oldWidth) > 1e-6f) ? (newWidth / oldWidth) : 0.0f;

    Double origXScale = m.GetXScaleDouble();
    Double newXScale = origXScale * multiplier * 100;
    pGeomData->XScale = newXScale;
    if (origXScale == 0)
    {
        newXScale = 0;
        origXScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(Alg::Abs(newXScale/(origXScale*100.))), 
        float(Alg::Abs(pGeomData->YScale/(m.GetYScaleDouble()*100.))),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    pGeomData->XScale = Alg::Abs(pGeomData->XScale);
    pGeomData->YScale = Alg::Abs(pGeomData->YScale);

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetHeight(Double height)
{
    // MA: Width/Height modification in Flash is unusual in that it performs
    // relative axis scaling in the x local axis (width scaling) and y local
    // axis (height scaling). The resulting width after scaling does not
    // actually equal the original, instead, it is related to rotation!
    // AB: I am second on that! Looks like a bug in Flash.

    // NOTE: Although it works for many cases, this is still not correct. Modification 
    // of height seems very strange (if not buggy) in Flash.

    if (NumberUtil::IsNaN(height) || NumberUtil::IsNEGATIVE_INFINITY(height))
        return;
    if (NumberUtil::IsPOSITIVE_INFINITY(height))
        height = 0;

    SetAcceptAnimMoves(0);
    SF_ASSERT(pGeomData);

    Matrix m = pGeomData->OrigMatrix;
    const Matrix& chm = GetMatrix();
    m.Tx() = chm.Tx();
    m.Ty() = chm.Ty();

    Matrix im = m;
    im.AppendRotation(float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    float oldHeight     = GetBounds(im).Height(); // height should be in local coords!
    float newHeight     = float(PixelsToTwips(height));
    float multiplier    = (fabsf(oldHeight) > 1e-6f) ? (newHeight / oldHeight) : 0.0f;

    Double origYScale = m.GetYScaleDouble();
    Double newYScale = origYScale * multiplier * 100;;
    pGeomData->YScale = newYScale;
    if (origYScale == 0)
    {
        newYScale = 0;
        origYScale = 1;
    }

    ASCharacter_MatrixScaleAndRotate2x2(m,
        float(Alg::Abs(pGeomData->XScale/(m.GetXScaleDouble()*100.))), 
        float(Alg::Abs(newYScale/(origYScale* 100.))),
        float(-m.GetRotationDouble() + pGeomData->Rotation * SF_MATH_PI / 180.));

    pGeomData->XScale = Alg::Abs(pGeomData->XScale);
    pGeomData->YScale = Alg::Abs(pGeomData->YScale);

    if (m.IsValid())
        SetMatrix(m);
}

void DisplayObjectBase::SetAlpha(Double alpha)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    if (NumberUtil::IsNaN(alpha))
        return;

    // Set alpha modulate, in percent.
    Cxform  cx = GetCxform();
    cx.M[3][0] = float(alpha / 100.);
    SetCxform(cx);
    SetAcceptAnimMoves(0);
}

Double DisplayObjectBase::GetX() const
{
    GeomDataType geomData;
    return TwipsToPixels(Double(GetGeomData(geomData).X));
}

Double DisplayObjectBase::GetY() const
{
    GeomDataType geomData;
    return TwipsToPixels(Double(GetGeomData(geomData).Y));
}

Double DisplayObjectBase::GetZ() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).Z);
}

Double DisplayObjectBase::GetXScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).XScale);
}

Double DisplayObjectBase::GetYScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).YScale);
}

Double DisplayObjectBase::GetZScale() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).ZScale);
}

Double DisplayObjectBase::GetRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).Rotation);
}

Double DisplayObjectBase::GetXRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).XRotation);
}

Double DisplayObjectBase::GetYRotation() const
{
    GeomDataType geomData;
    return Double(GetGeomData(geomData).YRotation);
}

Double DisplayObjectBase::GetWidth() const
{
    //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
    RectF  boundRect = GetBounds(GetMatrix());
    return TwipsToPixels(floor((Double)boundRect.Width()));
}

Double DisplayObjectBase::GetHeight() const
{
    //!AB: width and height of nested movieclips returned in the coordinate space of its parent!
    RectF  boundRect = GetBounds(GetMatrix());
    return TwipsToPixels(floor((Double)boundRect.Height()));
}

Double DisplayObjectBase::GetAlpha() const
{
    return GetCxform().M[3][0] * 100.F;
}

MovieImpl* DisplayObjectBase::FindMovieImpl() const
{
    const DisplayObjectBase* dobj = this;
    while (dobj && !dobj->IsInteractiveObject())
    {
        dobj = dobj->GetParent();
    }
    if (dobj)
        return dobj->CharToInteractiveObject_Unsafe()->GetMovieImpl();
    return NULL;
}

InteractiveObject*  DisplayObjectBase::GetTopMostMouseEntityDef
(CharacterDef* pdef, const Render::PointF &pt, bool testAll, const InteractiveObject* ignoreMC)
{   
    if (!GetVisible())
        return 0;

    Render::PointF p;          
    TransformPointToLocal(p, pt);

    if ((ClipDepth == 0) && pdef->DefPointTestLocal(p, 1, this))
    {
        // The mouse is inside the shape.
        InteractiveObject* pParent = GetParent();
        while (pParent && pParent->IsSprite())
        {
            Sprite* spr = pParent->CharToSprite_Unsafe();
            // Parent sprite would have to be in button mode to grab attention.
            if (testAll || pParent->ActsAsButton() 
                || (spr->GetHitAreaHolder() && spr->GetHitAreaHolder()->ActsAsButton()))
            {
                // Check if sprite should be ignored
                if (!ignoreMC || (ignoreMC != pParent))
                    return pParent;
            }
            pParent = pParent->GetParent ();
        }
    }
    return 0;
}

bool DisplayObjectBase::IsAncestor(DisplayObjectBase* ch) const
{
    DisplayObjectBase* p;
    for (p = ch->GetParent(); p; p = p->GetParent())
    {
        if (p == this)
            return true;
    }
    return false;
}

void DisplayObjectBase::SetDirtyFlag()
{
    GetMovieImpl()->SetDirtyFlag();
}

void DisplayObjectBase::UpdateTransform3D()
{
    Matrix3F rotXMat(Matrix3F::NoInit), transMat(Matrix3F::NoInit), scaleMat(Matrix3F::NoInit), rotYMat(Matrix3F::NoInit), rotZMat(Matrix3F::NoInit);

    // build new matrix 
    transMat = Matrix3F::Translation((float)pGeomData->X, (float)pGeomData->Y,(float)pGeomData->Z);
    scaleMat = Matrix3F::Scaling((float)pGeomData->XScale/100.f, (float)pGeomData->YScale/100.f, (float)pGeomData->ZScale/100.f);

    // MOOSE TODO - collapse this into one matrix?
    rotXMat = pGeomData->XRotation != 0.0 ? Matrix3F::RotationX((float)SF_DEGTORAD(pGeomData->XRotation)) : Matrix3F::Identity; 
    rotYMat = pGeomData->YRotation != 0.0 ? Matrix3F::RotationY((float)SF_DEGTORAD(pGeomData->YRotation)) : Matrix3F::Identity; 
    rotZMat =  pGeomData->Rotation != 0.0 ? Matrix3F::RotationZ((float)SF_DEGTORAD(pGeomData->Rotation))  : Matrix3F::Identity; 

    // Apply transforms in SRT order
    Matrix3F m = transMat * rotZMat * rotYMat * rotXMat * scaleMat;   // operations happen right to left 

    if (m.IsValid())
        SetMatrix3D(m);
}

//////////////////////////////////////////////////////////////////////////
DisplayObject::DisplayObject(ASMovieRootBase* pasRoot, InteractiveObject* pparent, ResourceId id)
:   
    DisplayObjectBase(pasRoot, pparent, id),
    pScrollRect(0),
    Flags(0)
{
    pMaskCharacter = NULL;
    SetScriptableObjectFlag();

    SetInstanceBasedNameFlag();
    SetAcceptAnimMovesFlag();
}

DisplayObject::~DisplayObject()
{
    delete pScrollRect;
    // free/reset mask
    if (GetMask())
        SetMask(NULL);
    DisplayObject* pmaskOwner = GetMaskOwner();
    if (pmaskOwner)
        pmaskOwner->SetMask(NULL);

    if (pNameHandle)
        pNameHandle->ReleaseCharacter();
}

void DisplayObject::OnEventLoad()
{
    DisplayObjectBase::OnEventLoad();
    if (HasAvmObject())
        GetAvmObjImpl()->OnEventLoad();
}

void DisplayObject::OnEventUnload()
{
    DisplayObjectBase::OnEventUnload();
    // need to release the character to avoid accidental reusing unloaded character.
    if (pNameHandle)
        pNameHandle->ReleaseCharacter();
    if (HasAvmObject())
        GetAvmObjImpl()->OnEventUnload();
}

ASString        DisplayObject::CreateNewInstanceName() const
{
    MovieImpl* proot = GetMovieImpl();
    return ASString(proot->CreateNewInstanceName());
}

CharacterHandle* DisplayObject::CreateCharacterHandle() const
{
    if (!pNameHandle)
    {   
        MovieImpl* proot = GetMovieImpl();

        // Hacky, but this can happen.
        // Will clearing child handles recursively on parent release work better?
        if (IsUnloaded())
        {
            SF_DEBUG_WARNING(1, "GetCharacterHandle called on unloaded sprite");
            // Returns temp handle which is essentially useless.
            pNameHandle = *SF_HEAP_NEW(proot->GetMovieHeap())
                CharacterHandle(proot->GetStringManager()->CreateEmptyString(), 0, 0);
            return pNameHandle;
        }        

        // Create new instance names as necessary.
        ASString name(CreateNewInstanceName());
        // SetName logic duplicated to take advantage of 'mutable' pNameHandle.
        pNameHandle = *SF_HEAP_NEW(proot->GetMovieHeap())
            CharacterHandle(name, pParent, const_cast<DisplayObject*>(this));
    }

    return pNameHandle;
}

Double DisplayObject::GetMouseX() const
{
    Render::PointF a = GetMovieImpl()->GetMouseState(0)->GetLastPosition();

    Matrix2F  m = GetWorldMatrix();
    Render::PointF b;

    m.TransformByInverse(&b, a);
    return TwipsToPixels(floor(b.x+0.5));
}

Double DisplayObject::GetMouseY() const
{
    Render::PointF a = GetMovieImpl()->GetMouseState(0)->GetLastPosition();

    Matrix2F  m = GetWorldMatrix();
    Render::PointF b;

    m.TransformByInverse(&b, a);
    return TwipsToPixels(floor(b.y+0.5));
}

void    DisplayObject::SetName(const ASString& name)
{
    if (!name.IsEmpty())
        ClearInstanceBasedNameFlag();

    if (pNameHandle)
    {
        pNameHandle->ChangeName(name, pParent);

        // TBD: Propagate update to all children ??
    }
    else
    {
        pNameHandle = *SF_HEAP_AUTO_NEW(this) CharacterHandle(name, pParent, this);
    }
}

void    DisplayObject::SetOriginalName(const ASString& name)
{
    DisplayObject::SetName(name);
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->SetOriginalName(name);
}

ASString DisplayObject::GetOriginalName() const
{
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->GetOriginalName();
    return GetName();
}

ASString DisplayObject::GetName() const
{
    CharacterHandle* pnameHandle = GetCharacterHandle();
    if (pnameHandle)
        return pnameHandle->GetName();
    return GetMovieImpl()->GetStringManager()->CreateEmptyString();
}

// Determines the absolute path of the character.
const char*    DisplayObject::GetAbsolutePath(String *ppath) const
{
    if (HasAvmObject())
        return GetAvmObjImpl()->GetAbsolutePath(ppath);
    return "";
}

void    DisplayObject::SetAcceptAnimMoves(bool accept)
{ 
    if (!pGeomData)
    {
        GeomDataType geomData;
        SetGeomData(GetGeomData(geomData));
    }
    SetAcceptAnimMovesFlag(accept); 

    // cache the flag locally
    SetContinueAnimationFlag(GetMovieImpl()->IsContinueAnimationFlagSet()); 

    if (GetMovieImpl()->IsContinueAnimationFlagSet() && accept)
    {
        // if continueAnimation flag is set and we restore timeline acceptance - 
        // remove the pGeomData to provide correct coordinates/scales by accessing
        // via ActionScript.
        delete pGeomData;
        pGeomData = NULL;
    }
    SetDirtyFlag();
}

class ClipRectDisplayObject : public DisplayObject
{
    RectF               Rect;
public:
    ClipRectDisplayObject(ASMovieRootBase* pasRoot, const RectF& r) :
        DisplayObject(pasRoot, NULL, ResourceId(ResourceId::InvalidId)),
        Rect(r)
    {
    }
    virtual CharacterDef* GetCharacterDef() const
    {
      return NULL;
    }
    //@REN
//     virtual void Display(DisplayContext& context)
//     {
//         Render::Renderer* prenderer = context.GetRenderer();
// 
//         Render::PointF          coords[4];
//         static const UInt16     indices[6] = { 0, 1, 2, 2, 1, 3 };
// 
//         Matrix m(*context.pParentMatrix);
//         m *= GetMatrix();
//         
//         RectF newRect;
//         Text::RecalculateRectToFit16Bit(m, Rect, &newRect);
//         prenderer->SetMatrix(m);
// 
//         coords[0] = newRect.TopLeft();
//         coords[1] = newRect.TopRight();
//         coords[2] = newRect.BottomLeft();
//         coords[3] = newRect.BottomRight();
// 
//         Render::Renderer::VertexXY16i icoords[4];
//         icoords[0].x = (SInt16) coords[0].x;
//         icoords[0].y = (SInt16) coords[0].y;
//         icoords[1].x = (SInt16) coords[1].x;
//         icoords[1].y = (SInt16) coords[1].y;
//         icoords[2].x = (SInt16) coords[2].x;
//         icoords[2].y = (SInt16) coords[2].y;
//         icoords[3].x = (SInt16) coords[3].x;
//         icoords[3].y = (SInt16) coords[3].y;
// 
//         prenderer->FillStyleColor(Color(255, 255, 255, 255));
//         prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);
// 
//         // Fill the inside
//         prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
//         prenderer->DrawIndexedTriList(0, 0, 4, 0, 2);
// 
//         // Done
//         prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
//         prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
// 
//     }
};

bool DisplayObject::ApplyMask(DisplayContext &context)
{   
    SF_UNUSED(context);
    // @REN
//     // if scrollRect is set up the mask is ignored and scrollRect's 
//     // clipping mask is used instead.
// 
//     DisplayObject* pmask;  
// 
//     if (pScrollRect)
//     {
//         if (!pScrollRect->Mask)
//         {
//             // no mask created yet; create one.
//             RectF maskRect(0, 0, SizeF((float)pScrollRect->Width(), (float)pScrollRect->Height()));
//             pScrollRect->Mask = *SF_HEAP_AUTO_NEW(this) 
//                 ClipRectDisplayObject(GetASMovieRoot(), maskRect);
//             pScrollRect->Mask->SetUsedAsMask();
//             pScrollRect->Mask->SetMatrix(GetMatrix());
//         }
//         pmask = pScrollRect->Mask;
//     }
//     else
//         pmask = GetMask();  
//     bool          masked = false;
//     if (pmask)
//     {
//         if (!pmask->IsUsedAsMask() || pmask->IsUnloaded())
//         {
//             // if mask character was unloaded or unmarked as a mask - remove it from the sprite
//             SetMask(NULL);
//         }
//         else
//         {
//             // mask can be outside the hierarchy
//             Matrix2F  matrix, *savedM = context.pParentMatrix;
//             if (pmask->GetParent())
//                 pmask->GetParent()->GetWorldMatrix(&matrix);
//             else if (GetParent())
//                 GetParent()->GetWorldMatrix(&matrix); // use maskee parent's matrix if mask doesn't have a parent
//             else
//                 SF_ASSERT(0); // ?? no parent? and displaying?
//             context.pParentMatrix = &matrix;
// 
//             context.PushAndDrawMask(pmask);
//             context.pParentMatrix = savedM;
//             masked = true;
//         }
//     }
//     return masked;
    return false;
}

void DisplayObject::ResetMask(bool masked, DisplayContext &context)
{
    SF_UNUSED2(masked, context);
    // @REN
//     if (masked)
//         context.PopMask();
}

void DisplayObject::SetMask(DisplayObject* pmaskSprite) 
{ 
    DisplayObject* poldMask = GetMask();
    if (poldMask)
    {
        // remove this movie clip from the mask holder for the old mask
        poldMask->SetMaskOwner(NULL);
    }

    // the sprite being masked cannot be a mask for another sprite
    DisplayObject* poldOwner = GetMaskOwner();
    if (poldOwner)
    {
        poldOwner->SetMask(NULL);
    }

    if (pMaskCharacter && !IsUsedAsMask())
        pMaskCharacter->Release();
    pMaskCharacter = pmaskSprite;
    SetUsedAsMask(false);
    if (pmaskSprite)
        pmaskSprite->AddRef();

    if (pmaskSprite) 
    {
        // setMask method override layers' masking (by zeroing ClipDepth).
        // (Do we need to do same for the mask sprite too? !AB)
        SetClipDepth(0); 

        DisplayObject* poldOwner = pmaskSprite->GetMaskOwner();
        if (poldOwner)
        {
            poldOwner->SetMask(NULL);
        }

        pmaskSprite->SetMaskOwner(this);
    }
    SetDirtyFlag();
}

DisplayObject* DisplayObject::GetMask() const
{
    if (pMaskCharacter && !IsUsedAsMask())
    {
        SF_ASSERT(pMaskCharacter->IsUsedAsMask() && pMaskCharacter->pMaskOwner == this);
        return pMaskCharacter;
    }
    return NULL;
}

void DisplayObject::SetMaskOwner(DisplayObject* pmaskOwner)
{
    if (GetMask()) 
        SetMask(NULL);

    SetUsedAsMask((pmaskOwner != NULL)?true:false);
    pMaskOwner = pmaskOwner;
}

DisplayObject* DisplayObject::GetMaskOwner() const
{
    if (pMaskOwner && IsUsedAsMask())
    {
        SF_ASSERT(!pMaskOwner->IsUsedAsMask() && pMaskOwner->pMaskCharacter == this);
        return pMaskOwner;
    }
    return NULL;
}

void DisplayObject::SetScrollRect(const Render::RectD* r)
{
    if (!r)
    {
        delete pScrollRect;
        pScrollRect = NULL;
    }
    else
    {
        if (!pScrollRect)
            pScrollRect = SF_HEAP_AUTO_NEW(this) ScrollRectInfo;
        *(static_cast<Render::RectD*>(pScrollRect)) = *r;

        // force mask character to be re-created (in Display)
        pScrollRect->Mask = NULL;
    }
}

bool  DisplayObject::HasEventHandler(const EventId& id) const
{
    if (HasAvmObject())
        return GetAvmObjImpl()->HasEventHandler(id);
    return false;
}

bool  DisplayObject::OnEvent(const EventId& id) 
{ 
    if (HasAvmObject())
        return GetAvmObjImpl()->OnEvent(id);
    return false; 
}

void DisplayObject::SetScale9Grid(const RectF& gr)
{
    GetRenderNode()->SetScale9Grid(gr);
    SetDirtyFlag();
}

RectF DisplayObject::GetScale9Grid() const
{
    return GetRenderNode()->GetScale9Grid();
}

// @REN
/*
bool    DisplayObject::BeginDisplay(DisplayContext &context, DisplayContextStates& save)
{
    // We're invisible, so don't display!
    //!AB: but mask should be drawn even if it is invisible!
    if (!IsDisplayable())
        return false;

    save.pParentMatrix = context.pParentMatrix;
    save.pParentCxform = context.pParentCxform;

#ifdef GFX_RTREELOG_ENABLED
    if (Render::RenderTreeLog::IsLogging())
    {
        String name("Sprite='", GetName().ToCStr(), "'");        
        Render::RenderTreeLog::StartObject(name.ToCStr(), GetBounds(Render::Matrix2F::Identity), GetMatrix());
    }    
#endif

    // We must apply a new binding table if the character needs to
    // use resource data from the loaded/imported MovieDefImpl.
    save.pResourceBinding = context.pResourceBinding;
    MovieDefImpl* pdefImpl;
    if ((pdefImpl = GetResourceMovieDef()) != NULL)
        context.pResourceBinding = &pdefImpl->GetResourceBinding();

    save.Masked = ApplyMask(context);

    save.CurMatrix = *save.pParentMatrix;
    save.CurMatrix.Prepend(GetMatrix());
    context.pParentMatrix = &save.CurMatrix;

    if (pScrollRect)
    {
        save.CurMatrix.PrependTranslation((float)-pScrollRect->x1, (float)-pScrollRect->y1);
    }

    save.CurCxform = *save.pParentCxform;
    save.CurCxform.Concatenate(GetCxform());
    context.pParentCxform = &save.CurCxform;
    return true;
}

void    DisplayObject::EndDisplay(DisplayContext &context, const DisplayContextStates& save)
{
    context.pResourceBinding    = save.pResourceBinding;
    context.pParentMatrix       = save.pParentMatrix;
    context.pParentCxform       = save.pParentCxform;

    DoDisplayCallback();

    ResetMask(save.Masked, context);

    GFX_RTREELOG_ENDOBJECT();
}
*/

// ***** CharacterHandle
CharacterHandle::CharacterHandle(const ASString& name, DisplayObject *pparent, DisplayObject* pcharacter)
: Name(name), NamePath(name.GetManager()->CreateEmptyString()), OriginalName(name)
{
    RefCount    = 1;
    pCharacter  = pcharacter;    

    if (pcharacter && pcharacter->IsAVM1())
    {
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);
    }
}

CharacterHandle::~CharacterHandle()
{
}

// Release a character reference, used when character dies
void    CharacterHandle::ReleaseCharacter()
{
    pCharacter = 0;
}


// Changes the name.
void    CharacterHandle::ChangeName(const ASString& name, DisplayObject *pparent)
{
    Name = name;
    if (pparent && pparent->IsAVM1())
    {
        // Compute path based on parent
        String namePathBuff;
        if (pparent)
        {
            pparent->GetAbsolutePath(&namePathBuff);
            namePathBuff += ".";
        }
        namePathBuff += Name.ToCStr();
        NamePath = name.GetManager()->CreateString(namePathBuff);

        // Do we need to update paths in all parents ??
    }
}

// Resolve the character, considering path if necessary.
DisplayObject* CharacterHandle::ResolveCharacter(MovieImpl *proot) const
{
    if (pCharacter)
        return pCharacter;
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

DisplayObject* CharacterHandle::ForceResolveCharacter(MovieImpl *proot) const
{
    // Resolve a global path based on Root.
    return proot->FindTarget(NamePath);
}

void CharacterHandle::ResetName(ASStringManager* psm)
{
    Name     = psm->CreateEmptyString();
    NamePath = Name; 
}


}} // namespace Scaleform::GFx

