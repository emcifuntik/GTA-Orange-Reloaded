/**********************************************************************

Filename    :   ButtonDef.h
Content     :   Button definition
Created     :   Nov, 2009
Authors     :   Michael Antonov, Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_BUTTONDEF_H
#define INC_SF_GFX_BUTTONDEF_H

#include "Kernel/SF_RefCount.h"

#include "GFx/GFx_Tags.h"
#include "GFx/GFx_CharacterDef.h"
#include "GFx/GFx_FilterDesc.h"
#include "Render/Render_States.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace GFx {

// ***** Declared Classes
class MouseState;
class SoundSampleImpl;

class LoadProcess;
#ifdef GFX_ENABLE_SOUND
class ButtonSoundDef;
#endif // SF_NO_SOUND

struct Scale9Grid;

using Render::Text::TextFilter;

//
// button characters
//
class ButtonRecord
{
public:
    enum MouseState
    {
        MouseUp,
        MouseDown,
        MouseOver
    };

    Ptr<TextFilter> mTextFilter;

    Matrix2F   ButtonMatrix;
    Cxform     ButtonCxform;

    ResourceId CharacterId;    
    UInt16     Depth;

    Render::BlendMode BlendMode;
    enum RecordFlags
    {
        Mask_HitTest = 1,
        Mask_Down    = 2,
        Mask_Over    = 4,
        Mask_Up      = 8,

        Mask_All = (Mask_HitTest | Mask_Down | Mask_Over | Mask_Up)
    };
    UInt8               Flags;

    ButtonRecord() : Flags(0) {}

    bool    MatchMouseState(MouseState mouseState) const
    {
        if ((mouseState == MouseUp && IsUp())      ||
            (mouseState == MouseDown && IsDown())  ||
            (mouseState == MouseOver && IsOver()) )
            return true;
        return false;
    }

    bool    Read(LoadProcess* p, TagType tagType);

    bool    IsHitTest() const { return (Flags & Mask_HitTest) != 0; }
    bool    IsDown() const    { return (Flags & Mask_Down) != 0; }
    bool    IsOver() const    { return (Flags & Mask_Over) != 0; }
    bool    IsUp() const      { return (Flags & Mask_Up) != 0; }

    UInt8   GetFlags() const  { return Flags; }
};

// Base class for Button Actions. Empty for AS3, overriden for AS2
class ButtonActionBase : public RefCountBase<ButtonActionBase, StatMD_CharDefs_Mem>
{
protected:
    virtual ~ButtonActionBase() {}

public:
    virtual void Read(Stream* pin, TagType tagType, unsigned actionLength) =0;
};

class ButtonDef : public CharacterDef
{
public:
#ifdef GFX_ENABLE_SOUND
    ButtonSoundDef*                                     pSound;
#endif // SF_NO_SOUND

    ArrayLH<ButtonRecord,StatMD_CharDefs_Mem>           ButtonRecords;
    ArrayLH<Ptr<ButtonActionBase>,StatMD_CharDefs_Mem>  ButtonActions;
    Scale9Grid*                                         pScale9Grid;
    bool                                                Menu;


    ButtonDef();
    virtual ~ButtonDef();

    void    InitEmptyButtonDef() {}

    void    Read(LoadProcess* p, TagType tagType);

    virtual CharacterDefType GetType() const { return Button; }

    // Obtains character bounds in local coordinate space.
    virtual RectF  GetBoundsLocal() const
    {
        // This is not useful, because button bounds always depend on its state.
        SF_ASSERT(0);
        return RectF(0);
        /*
        float h = 0;
        for(unsigned i=0; i<ButtonRecords.GetSize(); i++)
        if (!ButtonRecords[i].HitTest)
        h = Alg::Max<float>(h, ButtonRecords[i].pCharacterDef->GetHeightLocal());
        return h; */
    }


    void SetScale9Grid(const RectF& r) 
    {        
        if (pScale9Grid == 0) 
        {
            // This is MovieDef's loaded version of Scale9Grid,
            // so allocate it from the same heap as ButtonDef.
            pScale9Grid = SF_HEAP_AUTO_NEW(this) Scale9Grid(r);
        }
        else
            *pScale9Grid = r;
    }

    RectF GetScale9Grid() const { return (pScale9Grid) ? pScale9Grid->Rect : RectF(0); }
    bool  HasScale9Grid() const { return pScale9Grid != NULL; }

    // *** Resource implementation

    // Query Resource type code, which is a combination of ResourceType and ResourceUse.
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_ButtonDef); }

};

}} // namespace Scaleform::GFx
#endif // INC_SF_GFX_BUTTONDEF_H
