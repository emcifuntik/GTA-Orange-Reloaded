/**********************************************************************

Filename    :   GL_HALSetup.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "GL_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render { namespace GL {


// *** RenderHAL_GL Implementation
    
bool HAL::SetDependentVideoMode(TextureManager* ptextureManager)
{
    pTextureManager = ptextureManager;
    if (!pTextureManager)
        pTextureManager = *SF_HEAP_AUTO_NEW(this) TextureManager;

    pTextureManager->Initialize(this);

#ifdef SF_GL_RUNTIME_LINK
    Extensions::Init();
#endif

    for (unsigned i = 0; i < FragShaderDesc::FS_Count; i++)
        if (FragShaderDesc::Descs[i])
        {
            StaticShaders[i].Init(this, (FragShaderType)i, 0);
            StaticShaders[i + FragShaderDesc::FS_Count].Init(this, (FragShaderType)i, VertexShaderDesc::VS_base_Position3d);
        }

    if (!Cache.Initialize(this))
        return false;

    HALState|= HS_ModeSet;    
    notifyHandlers(HAL_SetVideoMode);
    return true;
}

// Returns back to original mode (cleanup)
bool HAL::ResetVideoMode()
{
    if (!(HALState & HS_ModeSet))
        return true;
    notifyHandlers(HAL_ResetVideoMode);

    pTextureManager.Clear();
    Cache.Reset();

    // Remove ModeSet and other state flags.
    HALState = 0;    
    return true;
}


    
// ***** Rendering

bool HAL::BeginFrame()
{
    if (!checkState(HS_ModeSet, "BeginFrame"))
        return false;
    HALState |= HS_InFrame;

    return true;
}

void HAL::EndFrame()
{
    if (!checkState(HS_ModeSet|HS_InFrame, "EndFrame"))
        return;
    Cache.EndFrame();
    HALState &= ~HS_InFrame;
}

// Set states not changed in our rendering, or that are reset after changes
bool HAL::BeginScene()
{
    if (!checkState(HS_InFrame, "BeginScene"))
        return false;
    HALState |= HS_InScene;

    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

#ifndef GL_ES_VERSION_2_0
    glDisable(GL_ALPHA_TEST);
#endif
    glStencilMask(0xffffffff);

    return true;
}

void HAL::EndScene()
{
    if (!checkState(HS_InFrame|HS_InScene, "BeginScene"))
        return;    

    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    HALState &= ~HS_InScene;
}


void HAL::BeginDisplay(Color backgroundColor, const Viewport& vpin)
{
    if (!checkState(HS_InFrame, "BeginScene"))
        return;
    HALState |= HS_InDisplay;

    // BeginDisplay automatically calls BeginScene if necessary.
    if (!(HALState & HS_InScene))
    {
        BeginScene();
        HALState |= HS_SceneInDisplay;
    }

    glEnable(GL_BLEND);
    applyBlendMode(Blend_None);

    glDisable(GL_STENCIL_TEST);
    beginMaskDisplay();
    Profiler.SetDrawMode(0);

    // Call QueueProcessor from Begin/EndDisplay until BeginDisplay is queued up.
    // TBD: Once it is, we'll need to call it from BeginDisplay/EndDisplay functions, but
    // will also need pre-processing for BeginDisplay to call BeginDisplay before
    // enqueing the rest of the call.
    QueueProcessor.BeginDisplay();

    SF_UNUSED(backgroundColor);
    RenderMode = (vpin.Flags & Viewport::View_AlphaComposite);

    VP = vpin;
    PointF tl = Matrices.UserVP.Transform(PointF(VP.Left, VP.Top));
    PointF br = Matrices.UserVP.Transform(PointF(VP.Left+VP.Width, VP.Top+VP.Height));
    float sizex = VP.BufferWidth, sizey = VP.BufferHeight;
    Matrices.UserVP.Transform2x2(&sizex, &sizey);
    VP.Left = (int)ceilf(Alg::Min(tl.x, br.x));
    VP.Top = (int)ceilf(Alg::Min(tl.y, br.y));
    VP.Width = (int)ceilf(fabsf(tl.x - br.x));
    VP.Height = (int)ceilf(fabsf(tl.y - br.y));
    VP.BufferWidth = (int)ceilf(fabsf(sizex));
    VP.BufferHeight = (int)ceilf(fabsf(sizey));
    if (VP.GetClippedRect(&ViewRect))
        HALState |= HS_ViewValid;
    else
        HALState &= ~HS_ViewValid;
    updateViewport();

    // Clear the background with a solid quad, if background has alpha > 0.
    if (backgroundColor.GetAlpha() > 0)
        clearSolidRectangle(ViewRect - Point<int>(VP.Left, VP.Top), backgroundColor);
}

void HAL::EndDisplay()
{
    if (!checkState(HS_InDisplay, "EndDisplay"))
        return;    

    QueueProcessor.EndDisplay();
    endMaskDisplay();
    SF_ASSERT(BlendModeStack.GetSize() == 0);

    // Must clear 
    HALState &= ~HS_InDisplay;

    if (HALState & HS_SceneInDisplay)
    {
        EndScene();
        HALState &= ~HS_SceneInDisplay;
    }
}

// Updates HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    Viewport vp;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(VP.Flags, &Matrices.ViewportMatrix, ViewRect, dx, dy);
        Matrices.ViewportMatrix *= Matrices.User;

        vp.Left     = ViewRect.x1;
        vp.Top      = ViewRect.y1;
        vp.Width    = ViewRect.Width();
        vp.Height   = ViewRect.Height();

        glViewport(vp.Left, VP.BufferHeight-vp.Top-vp.Height, vp.Width, vp.Height);

        if (VP.Flags & Viewport::View_UseScissorRect)
        {
            glEnable(GL_SCISSOR_TEST);
            glScissor(VP.ScissorLeft, VP.BufferHeight-VP.ScissorTop-VP.ScissorHeight, VP.ScissorWidth, VP.ScissorHeight);
        }
        else
        {
            glDisable(GL_SCISSOR_TEST);
        }
    }
    else
    {
        glViewport(0,0,0,0);
    }
}

void HAL::GetHWViewMatrix(Matrix* pmatrix, const Viewport& vp)
{
    Rect<int>  viewRect;
    int        dx, dy;
    vp.GetClippedRect(&viewRect, &dx, &dy);
    CalcHWViewMatrix(vp.Flags, pmatrix, viewRect, dx, dy);
}

void HAL::CalcHWViewMatrix(unsigned VPFlags, Matrix* pmatrix, const Rect<int>& viewRect, int dx, int dy)
{
    float       vpWidth = (float)viewRect.Width();
    float       vpHeight= (float)viewRect.Height();

    pmatrix->SetIdentity();
    if (VPFlags & Viewport::View_IsRenderTexture)
    {
        pmatrix->Sx() = 2.0f  / vpWidth;
        pmatrix->Sy() = 2.0f /  vpHeight;
        pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx); 
        pmatrix->Ty() = -1.0f - pmatrix->Sy() * ((float)dy);
    }
    else
    {
        pmatrix->Sx() = 2.0f  / vpWidth;
        pmatrix->Sy() = -2.0f / vpHeight;
        pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx); 
        pmatrix->Ty() = 1.0f  - pmatrix->Sy() * ((float)dy);
    }
}

}}} // Scaleform::Render::GL
