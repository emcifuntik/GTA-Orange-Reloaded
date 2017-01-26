/**********************************************************************

Filename    :   AvmBitmap.cpp
Content     :   Implementation of timeline part of Bitmap.
Created     :   Jun, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/AS3/AS3_AvmBitmap.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "Render/Render_TreeLog.h"
#include "Render/Render_ShapeDataFloat.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_Bitmap.h"
#include "GFx/AS3/Obj/Display/AS3_Obj_Display_BitmapData.h"

namespace Scaleform { namespace GFx { namespace AS3 {

AvmBitmap::AvmBitmap(ASMovieRootBase* pasRoot,
                     MovieDefImpl* pdefImpl,
                     InteractiveObject* pparent, 
                     ResourceId id)
:   DisplayObject(pasRoot, pparent, id),
    AvmDisplayObj(GetThis())
{
    ResourceHandle      rh;

    if (id != ResourceId::InvalidId)
    {
        //MovieImpl *proot = pasRoot->GetMovieImpl();
        if (pdefImpl->GetDataDef()->GetResourceHandle(&rh, id))
        {        
            Resource* pres = rh.GetResource(&pdefImpl->GetResourceBinding());
            if (pres)
            {
                if (pres->GetResourceType() == Resource::RT_Image)
                {
                    pImage = static_cast<ImageResource*>(pres);
                }
            }
        }
    }
}

AvmBitmap::~AvmBitmap()
{
}

unsigned AvmBitmap::CreateASInstance(bool execute)
{
    unsigned rv = AvmDisplayObj::CreateASInstance(execute);
    if (GetAS3Obj())
    {
        // create BitmapData
        Instances::Bitmap* as3bmp = static_cast<Instances::Bitmap*>(GetAS3Obj());

        Value params[] = { Value(0.f), Value(0.f) };
        SPtr<Instances::BitmapData> bmpData;

        // resolve class name, check if id is associated with a custom class;
        // use "flash.display.BitmapData" otherwise.
        String className;
        const String* psymbolName = pDispObj->GetResourceMovieDef()->GetNameOfExportedResource(pDispObj->GetId());
        if (!psymbolName)
            className = "flash.display.BitmapData";
        else
            className = *psymbolName;
        GetAS3Root()->GetAVM()->ConstructBuiltinObject(bmpData, className, 2, params);
        
        bmpData->CreateLibraryObject(pImage);
        as3bmp->SetBitmapData(bmpData);
    }
    return rv;
}

DisplayObjectBase::TopMostResult AvmBitmap::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr)
{   
    if (!GetVisible())
        return TopMost_FoundNothing;

    if (pImage)
    {
        Render::PointF p;          
        TransformPointToLocal(p, pt);

        RectF r(0, 0, 
            SizeF(PixelsToTwips((float)pImage->GetWidth()), PixelsToTwips((float)pImage->GetHeight())));
        if (r.Contains(p))
        {
            // need to return parent (since it is an InteractiveObject)
            pdescr->pResult = DisplayObject::GetParent();
            return TopMost_Found;
        }
    }

    return TopMost_FoundNothing;
}

bool AvmBitmap::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{     
    SF_UNUSED(hitTestMask);
    if (pImage)
    {
        RectF r(0, 0, 
            SizeF(PixelsToTwips((float)pImage->GetWidth()), PixelsToTwips((float)pImage->GetHeight())));
        if (r.Contains(pt))
        {
            return true;
        }
    }
    return false;
}

/*
void AvmBitmap::Display(DisplayContext &context)
{        
    DisplayContextStates save;
    if (!BeginDisplay(context, save))
        return;

    bool smoothing  = true;
    if (GetAS3Obj())
    {
        Instances::Bitmap* as3bmp = static_cast<Instances::Bitmap*>(GetAS3Obj());
        if (as3bmp->IsDirty() || !pImageShape)
        {
            // bitmap has been changed, re-create image shape
            pImage      = as3bmp->GetImageResource();
            pImageShape = NULL; // force to re-create
            smoothing   = as3bmp->IsSmoothed();
        }
    }
    if (!pImageShape && pImage)
    {
        pImageShape = *SF_HEAP_NEW(GetMovieImpl()->GetMovieHeap()) ShapeWithStyles();
        SF_ASSERT(pImageShape);
        pImageShape->SetToImage(pImage, true);
    }

    if (pImageShape)
    {
        // draw image
        DisplayParams params(context, save.CurMatrix, save.CurCxform, Render::Blend_None);
        pImageShape->GetFillAndLineStyles(&params);
        pImageShape->Display(params, false, 0);
    }

    EndDisplay(context, save);
}*/

// "transform" matrix describes the transform applied to parent and us,
// including the object's matrix itself. This means that if transform is
// identity, GetBoundsTransformed will return local bounds and NOT parent bounds.
RectF  AvmBitmap::GetBounds(const Matrix &transform) const
{
    RectF r;//(transform.EncloseTransform(pDef->GetBoundsLocal()));
    r.Clear();
    if (pImage)
    {
        r.SetWidth(PixelsToTwips((float)pImage->GetWidth()));
        r.SetHeight(PixelsToTwips((float)pImage->GetHeight()));
    }
    return transform.EncloseTransform(r);
}

Instances::Bitmap* AvmBitmap::GetAS3Bitmap() const
{
    SF_ASSERT(GetAS3Root()->GetAVM()->IsOfType(Value(GetAS3Obj()), "flash.display.Bitmap"));
    return static_cast<Instances::Bitmap*>(GetAS3Obj());
}


Instances::BitmapData* AvmBitmap::GetBitmapData() const
{
    Instances::Bitmap* bmp = GetAS3Bitmap();
    return bmp->GetBitmapData();
}

Render::TreeNode* AvmBitmap::RecreateRenderNode() const
{
    Instances::Bitmap* as3bmp = GetAS3Bitmap();
    UPInt index = ~0u;
    Render::TreeContainer* parent = NULL;
    bool renNodeExisted = !!pRenNode;
    if (as3bmp)
    {
        if (pRenNode && pRenNode->GetParent())
        {
            if (!pRenNode->IsMaskNode())
                parent = static_cast<Render::TreeContainer*>(pRenNode->GetParent());
            else
                SF_ASSERT(0); // mask is not supported yet @TODO
            // remove old render tree node with saving position to re-insert a new one at.
            // currently, if bitmap is changed we need to re-create the TreeNode. 
            index = 0;
            for (UPInt n = parent->GetSize(); index < n; ++index)
            {
                if (parent->GetAt(index) == pRenNode)
                    break;
            }
            parent->Remove(index, 1);
        }
        pRenNode = NULL;

        // bitmap has been changed, re-create image shape
        pImage      = as3bmp->GetImageResource();
    }
    if (renNodeExisted)
    {
        if (!pRenNode)
        {
            pRenNode = CreateRenderNode(GetMovieImpl()->GetRenderContext());
            SF_ASSERT(pRenNode);
            pRenNode->SetVisible(IsVisibleFlagSet());
        }
        // re-insert the new render tree node at the appropriate position
        if (parent)
        {
            SF_ASSERT(index != ~0u);
            parent->Insert(index, pRenNode);
        }
    }
    return pRenNode;
}

Ptr<Render::TreeNode> AvmBitmap::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeShape> tshp;
    Ptr<Render::ShapeDataFloat> shapeData = *SF_HEAP_AUTO_NEW(this) Render::ShapeDataFloat();
    Render::FillStyleType fill;
    fill.pFill = *SF_HEAP_AUTO_NEW(this) Render::ComplexFill();
    if (pImage)
    {
        Ptr<Render::Image> img;
        if (pImage->GetImage()->GetImageType() != Render::ImageBase::Type_ImageBase)
            img = static_cast<Render::Image*>(pImage->GetImage());
        else
        {
            ImageCreator* imgCreator = GetMovieImpl()->GetImageCreator();
            if (imgCreator)
            {
                ImageCreateInfo cinfo(ImageCreateInfo::Create_SourceImage, GetMovieImpl()->GetMovieHeap());
                img = *imgCreator->CreateImage(cinfo, static_cast<Render::ImageSource*>(pImage->GetImage()));
            }
            else
            {
                LogDebugMessage(Log_Warning, "Image is not created: can't find ImageCreator.");
                return NULL;
            }
        }
        fill.pFill->pImage = img;
        if (!img)
        {
            LogDebugMessage(Log_Warning, "Image is not created.");
            return NULL;
        }
        fill.pFill->ImageMatrix.SetIdentity();
        fill.pFill->ImageMatrix.AppendScaling(0.05f);

        Render::ImageRect r = img->GetRect();
        shapeData->AddFillStyle(fill);
        shapeData->StartPath(1, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), 0);
        shapeData->LineTo(float(PixelsToTwips(r.Width())), float(PixelsToTwips(r.Height())));
        shapeData->LineTo(0, float(PixelsToTwips(r.Height())));
        shapeData->ClosePath();
        shapeData->EndPath();
        shapeData->EndShape();
        Ptr<ShapeMeshProvider> pr = *SF_HEAP_AUTO_NEW(this) Render::ShapeMeshProvider(shapeData);
        tshp = *context.CreateEntry<Render::TreeShape>();
        tshp->SetShape(pr);
    }
    else
    {
        // no image, but still need to create an empty treeshape...
        shapeData->StartPath(0, 0, 0);
        shapeData->MoveTo(0, 0);
        shapeData->LineTo(1, 1);
        shapeData->ClosePath();
        shapeData->EndPath();
        shapeData->EndShape();
        Ptr<ShapeMeshProvider> pr = *SF_HEAP_AUTO_NEW(this) Render::ShapeMeshProvider(shapeData);
        tshp = *context.CreateEntry<Render::TreeShape>();
        tshp->SetShape(pr);
    }
    return tshp;
}

}}} // SF::GFx::AS3
