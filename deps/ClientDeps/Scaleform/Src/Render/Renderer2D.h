/**********************************************************************

PublicHeader:   Render
Filename    :   Renderer2D.h
Content     :   2D renderer interface that renderers TreeRoot
Created     :   September 23, 2009
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

#ifndef INC_SF_Render_Renderer2D_H
#define INC_SF_Render_Renderer2D_H

#include "Render_TreeNode.h"
#include "Render_MeshCacheConfig.h"
#include "Render_GlyphCacheConfig.h"
#include "Kernel/SF_HeapNew.h"

namespace Scaleform { namespace Render {

class Renderer2DImpl;
class GlyphCache;
struct ToleranceParams;


// ***** Renderer2D

// Renderer2D implements rendering of a tree staring with TreeRoot through
// its Display operation.
//
// Internally, the renderer caches the rendering tree structure and its resulting
// batches so that it can minimize the number of rendering calls, and support
// concurrent tree update through the RenderContext. 

class Renderer2D : public RefCountBase<Renderer2D, StatRender_Mem>
{
    Renderer2DImpl* pImpl;
    void* getThis() { return this; }

public:
    Renderer2D(HAL* phal);
    ~Renderer2D();

    Renderer2DImpl*         GetImpl() const { return pImpl; }
    Context::RenderNotify*  GetContextNotify() const;

    HAL*         GetHAL() const;
    // Query MeshCache configuration interface.
    MeshCacheConfig*   GetMeshCacheConfig() const;
    // Query GlyphCache configuration interface.   
    GlyphCacheConfig*  GetGlyphCacheConfig() const;

    const ToleranceParams&  GetToleranceParams() const;
    void                    SetToleranceParams(const ToleranceParams& params);
        
    // Delegated interface.
    bool    BeginFrame();
    void    EndFrame();
    bool    BeginScene();
    void    EndScene();
    void    BeginDisplay(Color backgroundColor, const Viewport& viewport);
    void    EndDisplay();
    
    // Renders a display tree starting with TreeRoot. TreeRoot must've
    // been allocated from the specified context.
    void    Display(TreeRoot *pnode);

    void    Display(const DisplayHandle<TreeRoot>& hroot)
    { Display(hroot.GetRenderEntry()); }
};

}} // Scaleform::Render

#endif
