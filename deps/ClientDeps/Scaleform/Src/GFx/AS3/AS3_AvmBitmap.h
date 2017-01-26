/**********************************************************************

Filename    :   AvmBitmap.h
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
#ifndef INC_AS3_AvmBitmap_H
#define INC_AS3_AvmBitmap_H

#include "GFx/AS3/AS3_AvmDisplayObj.h"

namespace Scaleform {
namespace GFx {

// ***** External Classes

namespace AS3 {

class BitmapObject;

namespace Instances
{
    class BitmapData;
    class Bitmap;
}

// An AS3-dependable part of displaylist's DisplayObject.
class AvmBitmap : public DisplayObject, public AvmDisplayObj
{
    AvmBitmap* GetThis() { return this; }
protected:
    mutable Ptr<ImageResource>      pImage;
//@IMG    Ptr<ShapeWithStyles>    pImageShape;
public:
    // Constructor.
    AvmBitmap(ASMovieRootBase* pasRoot,
        MovieDefImpl* pdefImpl,
        InteractiveObject* pparent, 
        ResourceId id);
    ~AvmBitmap();

    virtual unsigned    CreateASInstance(bool execute);
    virtual const char* GetDefaultASClassName() const { return "flash.display.Bitmap"; }
    virtual String&     GetASClassName(String* className) const
    {
        SF_ASSERT(className);
        // Always create "flash.display.Bitmap" class, since
        // the custom AS class is attached to BitmapData and not to Bitmap.
        *className = AvmBitmap::GetDefaultASClassName();
        return *className;
    }

    // DisplayObject methods
    virtual CharacterDef* GetCharacterDef() const
    {
        return NULL;
    }

//    virtual void    Display(DisplayContext &context);

    // These are used for finding bounds, width and height.
    virtual RectF  GetBounds(const Matrix &transform) const;
    virtual RectF  GetRectBounds(const Matrix &transform) const
    {     
        return GetBounds(transform);
    }   

    virtual bool    PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask = 0) const;

    // Override this to hit-test shapes and make Button-Mode sprites work.
    virtual TopMostResult GetTopMostMouseEntity(const Render::PointF &pt, TopMostDescr* pdescr);

    //////////////////////////////////////////////////////////////////////////
    Instances::Bitmap*     GetAS3Bitmap() const;
    Instances::BitmapData* GetBitmapData() const;
    void                   SetImage(ImageResource* img) { pImage = img; }

    Render::TreeNode*      RecreateRenderNode() const;

    SF_MEMORY_REDEFINE_NEW(AvmBitmap, StatMV_ActionScript_Mem)

protected:
    virtual Ptr<Render::TreeNode> CreateRenderNode(Render::Context& context) const;
};

}}} // namespace SF::GFx::AS3

#endif // INC_AS3_AvmBitmap_H
