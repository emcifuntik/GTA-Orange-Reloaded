/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Viewport.h
Content     :   Viewport class definition
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_Viewport_H
#define INC_SF_Render_Viewport_H

#include "Render_Types2D.h"

namespace Scaleform { namespace Render {



// HAL Viewport.
//
// NOTE: Scale and AspectRatio variables have been removed from HW viewport 
//       they should be applied as a part of Flash scaling by ViewMatrix,
//       which can be a part of RootNode and converts Flash coordinate system
//       into the pixel-based one used by Render2D.

class Viewport
{
public:
    int      BufferWidth, BufferHeight;
    int      Left, Top;
    int      Width, Height;
    // Scissor left & top are in same coordinate system as Left & Top (in buffer's ones)
    int      ScissorLeft,  ScissorTop; 
    int      ScissorWidth, ScissorHeight;    
    unsigned Flags;

    enum
    {
        View_IsRenderTexture    = 1,

        // Support blending into the destination through a render target texture.
        // Only blend/alpha and add modes are supported. Render the texture with
        // source blend factor 1, dest blend factor 1-Sa.
        View_AlphaComposite     = 2,
        View_UseScissorRect     = 4,

        // Don't set viewport/scissor state on device. Currently only supported by some
        // Renderer implementations.
        View_NoSetState         = 8,

        View_RenderTextureAlpha = View_IsRenderTexture|View_AlphaComposite,
    };

    Viewport()
    { 
        BufferWidth = 0; BufferHeight = 0;
        Left = Top = 0; Width = Height = 1;
        ScissorLeft = ScissorTop = ScissorWidth = ScissorHeight = 0; Flags = 0;
    }
    Viewport(int bw, int bh, int left, int top, int w, int h, int scleft, int sctop, int scw, int sch, unsigned flags = 0)
    { 
        BufferWidth = bw; BufferHeight = bh;
        Left = left; Top = top; Width = w; Height = h; Flags = flags | View_UseScissorRect;
        SetScissorRect(scleft, sctop, scw, sch);
    }    
    Viewport(int bw, int bh, int left, int top, int w, int h, unsigned flags = 0)
    { 
        BufferWidth = bw; BufferHeight = bh; Left = left; Top = top; Width = w; Height = h; Flags = flags; 
        ScissorLeft = ScissorTop = ScissorWidth = ScissorHeight = 0;
    }
    Viewport(const Viewport& src)
    { 
        BufferWidth = src.BufferWidth; BufferHeight = src.BufferHeight;
        Left = src.Left; Top = src.Top; Width = src.Width; Height = src.Height; Flags = src.Flags; 
        ScissorLeft = src.ScissorLeft; ScissorTop = src.ScissorTop; ScissorWidth = src.ScissorWidth; ScissorHeight = src.ScissorHeight;         
    }

    void SetScissorRect(int scleft, int sctop, int scw, int sch)
    {
        ScissorLeft = scleft; ScissorTop = sctop; ScissorWidth = scw; ScissorHeight = sch;
        Flags |= View_UseScissorRect;
    }

    bool    IsValid() const { return (BufferWidth != 0) && (BufferHeight != 0); }

    // Calculates clipped viewport rectangle, together with dx and dy adjustments that
    // are made for the clipping to be done. Returns false if the resulting clipping
    // rectangle is empty (so no rendering can take place).
    // Clipping includes:
    //  1. Ensuring that {Left, Top, Width, Height} fall within Buffer size.
    //  2. Clipping to UseScissorRect if needed.
    template <typename IntType>
    bool GetClippedRect(Rect<IntType> *prect) const
    {
        if (!Rect<IntType>(Left, Top, Left+Width, Top+Height)
            .IntersectRect(prect, Rect<IntType>(BufferWidth, BufferHeight)))
        {
            prect->Clear();
            return false;
        }

        if (Flags & View_UseScissorRect)
        {
            Rect<IntType> scissor(ScissorLeft, ScissorTop,
                ScissorLeft + ScissorWidth, ScissorTop + ScissorHeight);
            if (!prect->IntersectRect(prect, scissor))
            {
                prect->Clear();
                return false;
            }
        }
        return true;
    }

    template <typename IntType>
    bool GetClippedRect(Rect<IntType> *prect, IntType* pdx, IntType* pdy) const
    {
        if (!GetClippedRect(prect))
            return false;
        *pdx = prect->x1 - Left;
        *pdy = prect->y1 - Top;
        return true;
    }

    // Obtain rectangle that will be used for calling, in viewport-relative
    // coordinates (floating-point).
    bool GetCullRectF(RectF* prect) const
    {
        Rect<int> clipRect;
        if (!GetClippedRect(&clipRect))
            return false;
        prect->SetRect((float)(clipRect.x1 - Left), (float)(clipRect.y1 - Top),
                       (float)(clipRect.x2 - Left), (float)(clipRect.y2 - Top));
        return true;
    }    
};

}} // Scaleform::Render

#endif
