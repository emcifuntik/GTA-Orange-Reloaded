/**********************************************************************

Filename    :   Render_ShapeDataFloatMP.h
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
#ifndef SF_Render_ShapeDataFloatMP_H
#define SF_Render_ShapeDataFloatMP_H

#include "Render_ShapeDataFloat.h"
#include "Render_ShapeMeshProvider.h"

namespace Scaleform { namespace Render {

// Convenience wrapper class that combines ShapeDataInterface and ShapeMeshProvider
//------------------------------------------------------------------------
class ShapeDataFloatMP : public ShapeMeshProvider
{
public:
    ShapeDataFloatMP();

    void Clear() { pData->Clear(); }
    unsigned AddFillStyle(const FillStyleType& fill) { return pData->AddFillStyle(fill); }
    unsigned AddStrokeStyle(const StrokeStyleType& str) { return pData->AddStrokeStyle(str); }

    unsigned AddStrokeStyle(float width, unsigned flags, float miter, UInt32 color)
    { 
        return pData->AddStrokeStyle(width, flags, miter, color);
    }

    void StartLayer() 
    { 
        pData->StartLayer(); 
    }

    void StartPath(unsigned leftStyle, unsigned rightStyle, unsigned strokeStyle) 
    { 
        pData->StartPath(leftStyle, rightStyle, strokeStyle); 
    }

    void MoveTo(float x, float y)
    {
        pData->MoveTo(x, y); 
    }

    void LineTo(float x, float y)
    {
        pData->LineTo(x, y); 
    }

    void QuadTo(float cx, float cy, float ax, float ay)
    {
        pData->QuadTo(cx, cy, ax, ay); 
    }

    void RectanglePath(float x1, float y1, float x2, float y2)
    {
        pData->MoveTo(x1, y1);
        pData->LineTo(x2, y1);
        pData->LineTo(x2, y2);
        pData->LineTo(x1, y2);
        pData->ClosePath();
        pData->EndPath();
    }

    void ClosePath()
    {
        pData->ClosePath();
    }

    void EndPath()
    {
        pData->EndPath();
    }

    void EndShape()
    {
        pData->EndShape();
    }

    void CountLayers();

    unsigned GetFillStyleCount() const { return pData->GetFillStyleCount(); }
    unsigned GetStrokeStyleCount() const { return pData->GetStrokeStyleCount(); }
    void     GetFillStyle(unsigned idx, FillStyleType* p) const { return pData->GetFillStyle(idx, p); }
    void     GetStrokeStyle(unsigned idx, StrokeStyleType* p) const { return pData->GetStrokeStyle(idx, p); }

private:
    Ptr<ShapeDataFloat> pData;
};


}} // Scaleform::Render

#endif
