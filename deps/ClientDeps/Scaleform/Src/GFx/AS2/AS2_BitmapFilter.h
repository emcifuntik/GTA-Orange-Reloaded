/**********************************************************************

Filename    :   GFxASBitmapFilter.h
Content     :   BitmapFilter reference class for ActionScript 2.0
Created     :   12/10/2008
Authors     :   Prasad Silva
Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_GASBitmapFilter_H
#define INC_GASBitmapFilter_H

#include "GFxConfig.h"

#ifdef GFX_AS2_ENABLE_FILTERS

#include "AS2/AS2_Object.h"
#include "AS2/AS2_ObjectProto.h"

#include "GFx/GFx_FilterDesc.h"

namespace Scaleform { namespace GFx { namespace AS2 {

using Render::Text::FilterDesc;

// ***** Declared Classes
class BitmapFilterObject;
class BitmapFilterProto;
class BitmapFilterCtorFunction;

// 
// Actionscript BitmapFilter object declaration
//

class BitmapFilterObject : public Object
{
    friend class BitmapFilterProto;
    friend class BitmapFilterCtorFunction;
private:
    BitmapFilterObject(const BitmapFilterObject&) : Object(GetCollector()) {} 
protected:

    FilterDesc   Filter;

//?    BitmapFilterObject(FilterDesc::FilterType ft); //?
//?    BitmapFilterObject(const BitmapFilterObject& obj) 
//?        : Object(obj->GetGC()), Filter(obj.Filter) {} //?AB: do we need it? no copy ctor for Object
    BitmapFilterObject(ASStringContext *psc, Object* pprototype)
        : Object(psc)
    { 
        Set__proto__(psc, pprototype); // this ctor is used for prototype obj only
    }

    virtual         ~BitmapFilterObject();

public:

    BitmapFilterObject(Environment* penv, FilterDesc::FilterType ft);

    virtual ObjectType      GetObjectType() const { return Object_BitmapFilter; }

    SF_INLINE void                 SetFilter(const FilterDesc& filter) { Filter = filter; }
    SF_INLINE const FilterDesc&    GetFilter() const { return Filter; }

    // The offset distance for the shadow, in pixels.
    SF_INLINE void            SetDistance(SInt16 d)   { Filter.Distance = d * 20; }
    // The angle of the shadow. Valid values are 0 to 360? (floating point).
    SF_INLINE void            SetAngle(SInt16 a)      { Filter.Angle = a * 10; }
    // The color of the shadow. Valid values are in hexadecimal format 0xRRGGBB
    SF_INLINE void            SetColor(UInt32 c)      { Filter.ColorV.SetColor(c, Filter.ColorV.GetAlpha()); }
    // The alpha transparency value for the shadow color. Valid values are 0 to 1.
    SF_INLINE void            SetAlpha(float a)       { Filter.ColorV.SetAlphaFloat(a); }
    // The amount of horizontal blur. Valid values are 0 to 255 (floating point).
    SF_INLINE void            SetBlurX(float b)       { Filter.BlurX = PixelsToTwips(b); }
    // The amount of vertical blur. Valid values are 0 to 255 (floating point).
    SF_INLINE void            SetBlurY(float b)       { Filter.BlurY = PixelsToTwips(b); }
    // The strength of the imprint or spread. The higher the value, 
    // the more color is imprinted and the stronger the contrast between 
    // the shadow and the background. Valid values are from 0 to 255.
    SF_INLINE void            SetStrength(float s)    { Filter.Strength = s; }
    // Applies a knockout effect (true), which effectively makes the 
    // object's fill transparent and reveals the background color of the document.
    SF_INLINE void            SetKnockOut(bool k) 
    { 
        if (k) 
            Filter.Flags |= FilterDesc::KnockOut;  
        else 
            Filter.Flags &= ~FilterDesc::KnockOut;
    }
    // Indicates whether or not the object is hidden. The value true indicates that 
    // the object itself is not drawn; only the shadow is visible.
    SF_INLINE void            SetHideObject(bool h) 
    { 
        if (h) 
            Filter.Flags |= FilterDesc::HideObject; 
        else 
            Filter.Flags &= ~FilterDesc::HideObject;
    }

    SF_INLINE SInt16          GetDistance()   { return Filter.Distance / 20; }
    SF_INLINE SInt16          GetAngle()      { return Filter.Angle / 10; }
    SF_INLINE UInt32          GetColor()      { return Filter.ColorV.ToColor32() & 0x00FFFFFF; }
    SF_INLINE float           GetAlpha()      { float a; Filter.ColorV.GetAlphaFloat(&a); return a; }
    SF_INLINE float           GetBlurX()      { return TwipsToPixels(Filter.BlurX); }
    SF_INLINE float           GetBlurY()      { return TwipsToPixels(Filter.BlurY); }
    SF_INLINE float           GetStrength()   { return Filter.Strength; }
    SF_INLINE bool            IsKnockOut()    { return (Filter.Flags & FilterDesc::KnockOut) != 0; }
    SF_INLINE bool            IsHideObject()  { return (Filter.Flags & FilterDesc::HideObject) != 0; }
};


class BitmapFilterProto : public Prototype<BitmapFilterObject>
{
public:
    BitmapFilterProto(ASStringContext *psc, Object* prototype, 
        const FunctionRef& constructor);
};


class BitmapFilterCtorFunction : public CFunctionObject
{
public:
    BitmapFilterCtorFunction(ASStringContext *psc);

    //static void GlobalCtor(const GASFnCall& fn);

    static FunctionRef Register(GlobalContext* pgc);
};

}}} //SF::GFx::AS2

#endif  // SF_NO_FXPLAYER_AS_FILTERS

#endif  // INC_GASBitmapFilter_H
