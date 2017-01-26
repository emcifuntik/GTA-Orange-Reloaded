/**********************************************************************

Filename    :   D3D9_HALSetup.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "D3D9_HAL.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include <d3dx9.h>
#include <xutility>

namespace Scaleform { namespace Render { namespace D3D9 {

static PixMarker FrameMarker(0, false);
static PixMarker SceneMarker(0, false);
static PixMarker DisplayMarker(0, false);

// *** RenderHAL_D3D9 Implementation
    
bool HAL::SetDependentVideoMode(IDirect3DDeviceX* pd3dDevice,
                                      D3DPRESENT_PARAMETERS* ppresentParams,
                                      TextureManager* ptextureManager,
                                      UInt32 vmConfigFlags, HWND hwnd)
{
    if (!pd3dDevice || !ppresentParams)
        return 0;

    // TODO: Need to check device caps ?        
    pDevice = pd3dDevice;
    pDevice->AddRef();

    // Detect if we can use shaders.
    D3DCAPSx caps;
    pDevice->GetDeviceCaps(&caps);     

    if ((caps.PixelShaderVersion & 0xFFFF) >= SF_RENDERER_SHADER_VERSION)
        UsePixelShaders = 1;    


    if (!SManager.Initialize(pDevice, caps, &Profiler))
    {
fail_setmode_1:
        pDevice->Release();
        pDevice = 0;
        return 0;
    }

    if (!Cache.Initialize(pDevice))
        goto fail_setmode_1;

    // Create Texture manager if needed.
    if (ptextureManager)
        pTextureManager = ptextureManager;
    else
    {
        D3DCapFlags ourCaps;
        ourCaps.InitFromHWCaps(caps);

        pTextureManager = *SF_HEAP_AUTO_NEW(this) TextureManager(pDevice, ourCaps);
        if (!pTextureManager)
        {
            Cache.Reset();
            SManager.Reset();
            goto fail_setmode_1;
        }
    }

    // Detect stencil op.
    if (caps.StencilCaps & D3DSTENCILCAPS_INCR)
    {
        StencilOpInc = D3DSTENCILOP_INCR;
    }
    else if (caps.StencilCaps & D3DSTENCILCAPS_INCRSAT)
    {
        StencilOpInc = D3DSTENCILOP_INCRSAT;
    }
    else
    {   // Stencil ops not available.
        StencilOpInc = D3DSTENCILOP_REPLACE;
    }

    VMCFlags = vmConfigFlags;

    memcpy(&PresentParams, ppresentParams, sizeof(D3DPRESENT_PARAMETERS));
    hWnd    = hwnd;
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
    SManager.Reset();
    Cache.Reset();
 
    //ReleaseShaders();
    pDevice->Release();
    pDevice = 0;
    hWnd    = 0;        

    // Remove ModeSet and other state flags.
    HALState = 0;    
    return true;
}

    
// ***** Rendering

bool HAL::BeginFrame()
{
    FrameMarker.Begin(__FUNCTIONW__);

    if (!checkState(HS_ModeSet, "BeginFrame"))
        return false;
    HALState |= HS_InFrame;
    pTextureManager->ServiceQueues();
    return true;
}

void HAL::EndFrame()
{
    if (!checkState(HS_ModeSet|HS_InFrame, "EndFrame"))
        return;
    Cache.EndFrame();
    HALState &= ~HS_InFrame;

    FrameMarker.End();
}


bool HAL::BeginScene()
{
    SceneMarker.Begin(__FUNCTIONW__);

    if (!checkState(HS_InFrame, "BeginScene"))
        return false;
    HALState |= HS_InScene;

    // Blending render states.
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);    
    applyBlendMode(Blend_None);      

    // Not necessary of not alpha testing:
    //pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER); 
    //pDevice->SetRenderState(D3DRS_ALPHAREF, 0x00);
    pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);  // Important!

    union 
    {
        float fbias;
        DWORD d;
    } bias;
    bias.fbias = -0.5f;

    // Must reset both stages since ApplySampleMode modifies both.
    //SampleMode[0]       = Sample_Linear;      
    //SampleMode[1]       = Sample_Linear;      

    pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE);

    // Initialize to default texturing to trilinear filtering and wrap addressing.
    for ( int i = 0; i < 4; i++ )
        pTextureManager->SetSamplerState(i, D3DTEXF_LINEAR, D3DTADDRESS_WRAP);

    pDevice->SetSamplerState(0, D3DSAMP_MIPMAPLODBIAS, bias.d );
    pDevice->SetSamplerState(0, D3DSAMP_ELEMENTINDEX, 0);
    pDevice->SetSamplerState(1, D3DSAMP_ELEMENTINDEX, 0);

    // Textures off by default.
    pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_DISABLE);

    // No ZWRITE by default
    pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 0);
    pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    // Turn off D3D lighting, since we are providing our own vertex colors
    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

    // Clear more states..
    pDevice->SetRenderState(D3DRS_FOGENABLE,        FALSE );
    pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE,  0);
    pDevice->SetRenderState(D3DRS_SPECULARENABLE,   FALSE);
    //  pDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    //  pDevice->SetRenderState(D3DRS_INDEXEDVERTEXBLENDENABLE, FALSE);

    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN |
        D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);

    // Need to clear textures to avoid potential warnings.
    pDevice->SetTexture(0, 0);
    pDevice->SetTexture(1, 0);
    pDevice->SetTexture(2, 0); // Can't set texture
    pDevice->SetTexture(3, 0);
    // Texture stage clears not necessary, as SetPixelShader does so.
    // pDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    // pDevice->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    // Coord index 0 -> 0, 1 -> 1
    pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX,  0);
    pDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE);
    pDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX,  1);
    pDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS,  D3DTTFF_DISABLE);

    // Turn of back-face culling.
    pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    // Start the scene
    if (!(VMCFlags&VMConfig_NoSceneCalls))
        pDevice->BeginScene();

    SManager.BeginScene();
    return true;
}

void HAL::EndScene()
{
    if (!checkState(HS_InFrame|HS_InScene, "BeginScene"))
        return;    
    SManager.EndScene();

    if (!(VMCFlags&VMConfig_NoSceneCalls))
        pDevice->EndScene();
    HALState &= ~HS_InScene;

    SceneMarker.End();
}



void HAL::BeginDisplay(Color backgroundColor, const Viewport& vpin)
{
    DisplayMarker.Begin(__FUNCTIONW__);

    if (!checkState(HS_InFrame, "BeginScene"))
        return;
    HALState |= HS_InDisplay;

    // BeginScene automatically calls BeginScene if necessary.
    if (!(HALState & HS_InScene))
    {
        BeginScene();
        HALState |= HS_SceneInDisplay;
    }

    beginMaskDisplay();
    Profiler.SetDrawMode(0);
    // Call QueueProcessor from Begin/EndDisplay until BeginDisplay is queued up.
    // TBD: Once it is, we'll need to call it from BeginDisplay/EndDisplay functions, but
    // will also need pre-processing for BeginDisplay to call BeginDisplay before
    // enqueing the rest of the call.
    QueueProcessor.BeginDisplay();

    SF_UNUSED(backgroundColor);
    RenderMode = (vpin.Flags & Viewport::View_AlphaComposite);

    // Need to get surface size, so that we can clamp viewport
    D3DSURFACE_DESC surfaceDesc;
    surfaceDesc.Width = vpin.Width;
    surfaceDesc.Height= vpin.Height;
    {
        Ptr<IDirect3DSurfaceX> psurface;
        pDevice->GetRenderTarget(0, &psurface.GetRawRef());
        if (psurface)
            psurface->GetDesc(&surfaceDesc);

        // TBD: For now? Of should we use this as an extra clipping rectangle?
        SF_ASSERT(surfaceDesc.Width  == (UINT)vpin.BufferWidth);
        SF_ASSERT(surfaceDesc.Height == (UINT)vpin.BufferHeight);
    }
     
    // In some cases destination source coordinates can be outside D3D viewport.
    // D3D will not allow that; however, it is useful to support.
    // So if this happens, clamp the viewport and re-calculate source values.
    VP = vpin;
    if (vpin.GetClippedRect(&ViewRect))
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

    DisplayMarker.End();
}

// Updates D3D HW Viewport and ViewportMatrix based on provided viewport
// and view rectangle.
void HAL::updateViewport()
{
    D3DVIEWPORTx vp;
    Rect<int>    vpRect;

    if (HALState & HS_ViewValid)
    {
        int dx = ViewRect.x1 - VP.Left,
            dy = ViewRect.y1 - VP.Top;
        
        // Modify HW matrix and viewport to clip.
        CalcHWViewMatrix(&Matrices.ViewportMatrix, ViewRect, dx, dy);
        Matrices.ViewportMatrix.Prepend(Matrices.User);

        /*
        // TBD: Prepend UserMatrix here is incorrect for nested viewport-based
        // mask clipping; what's needed is a complex combination of viewport and
        // coordinate adjustment. Until this is done, mask viewport clipping will be
        // in the wrong place for UserMatrix.
        if (UserMatrix != Matrix2F::Identity)
        {
            Rect<int> viewportRect;
            Rect<int> userViewRect(
                ViewRect.x1 + (int)UserMatrix.Tx(),
                ViewRect.y1 + (int)UserMatrix.Ty(),
                Size<int>((int)(UserMatrix.Sx() * (float)ViewRect.Width()),
                          (int)(UserMatrix.Sy() * (float)ViewRect.Height())));

            VP.GetClippedRect(&viewportRect);
            viewportRect.IntersectRect(&vpRect, userViewRect);
        }
        */

        vpRect = ViewRect;
    }

    vp.X        = vpRect.x1;
    vp.Y        = vpRect.y1;
    vp.Width    = vpRect.Width();
    vp.Height   = vpRect.Height();

    // DX9 can't handle a vp with zero area.
    SF_ASSERT(vp.Width > 0 && vp.Height > 0 );
    vp.Width  = Alg::Max(vp.Width, (DWORD)1);
    vp.Height = Alg::Max(vp.Height, (DWORD)1);

    vp.MinZ     = 0.0f;
    vp.MaxZ     = 0.0f;
    pDevice->SetViewport(&vp);
}


void HAL::GetHWViewMatrix(Matrix* pmatrix, const Viewport& vp)
{
    Rect<int>  viewRect;
    int        dx, dy;
    vp.GetClippedRect(&viewRect, &dx, &dy);
    CalcHWViewMatrix(pmatrix, viewRect, dx, dy);
}

void HAL::CalcHWViewMatrix(Matrix* pmatrix,
                                 const Rect<int>& viewRect, int dx, int dy)
{
    float       vpWidth = (float)viewRect.Width();
    float       vpHeight= (float)viewRect.Height();
    // Adjust by -0.5 pixel to match DirectX pixel coverage rules.
    float       xhalfPixelAdjust = (viewRect.Width() > 0) ? (1.0f / vpWidth) : 0.0f;
    float       yhalfPixelAdjust = (viewRect.Height()> 0) ? (1.0f / vpHeight) : 0.0f;

    pmatrix->SetIdentity();
    pmatrix->Sx() = 2.0f  / vpWidth;
    pmatrix->Sy() = -2.0f / vpHeight;
    pmatrix->Tx() = -1.0f - pmatrix->Sx() * ((float)dx) - xhalfPixelAdjust; 
    pmatrix->Ty() = 1.0f  - pmatrix->Sy() * ((float)dy) + yhalfPixelAdjust;
}


}}} // Scaleform::Render::D3D9
