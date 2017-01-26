/**********************************************************************

Filename    :   Renderer2D.cpp
Content     :   2D renderer interface that renderers TreeRoot
Created     :   September 3, 2009
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#include "Renderer2DImpl.h"
#include "Render_GlyphCache.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

// ***** Renderer2D
    
// Renderer2DImpl is a wrapper for Renderer2DImpl, intended to hide
// implementation details (user doesn't need to know about Tesselator, etc).

Renderer2D::Renderer2D(HAL* hal)
{    
    pImpl = SF_HEAP_AUTO_NEW(this) Renderer2DImpl(hal);

    SF_AMP_CODE(AmpServer::GetInstance().SetRenderer(this);)
}
Renderer2D::~Renderer2D()
{
    delete pImpl;
}

Context::RenderNotify* Renderer2D::GetContextNotify() const
{
    return pImpl;
}

HAL* Renderer2D::GetHAL() const
{
    return pImpl->GetHAL();
}

MeshCacheConfig* Renderer2D::GetMeshCacheConfig() const
{
    return pImpl->GetMeshCacheConfig();
}
GlyphCacheConfig* Renderer2D::GetGlyphCacheConfig() const
{
    return pImpl->GetGlyphCache();
}

const ToleranceParams& Renderer2D::GetToleranceParams() const
{
    return pImpl->GetToleranceParams();
}
void Renderer2D::SetToleranceParams(const ToleranceParams& params)
{
    pImpl->SetToleranceParams(params);
}

// Delegated interface.
bool Renderer2D::BeginFrame()
{
    return pImpl->BeginFrame();
}
void Renderer2D::EndFrame()
{
    pImpl->EndFrame();
}
bool Renderer2D::BeginScene()
{
    return pImpl->BeginScene();
}
void Renderer2D::EndScene()
{
    pImpl->EndScene();
}
void Renderer2D::BeginDisplay(Color backgroundColor, const Viewport& viewport)
{
    pImpl->BeginDisplay(backgroundColor, viewport);
}
void Renderer2D::EndDisplay()
{
    pImpl->EndDisplay();
}

void Renderer2D::Display(TreeRoot *node)
{
    return pImpl->Draw(node);
}

}} // Scaleform::Render

