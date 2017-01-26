/**********************************************************************

Filename    :   GL_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_HAL_H
#define INC_SF_Render_GL_HAL_H

#include "Render/Render_HAL.h"
#include "Render/Render_Queue.h"
#include "GL_MeshCache.h"
#include "GL_Shader.h"
#include "GL_Texture.h"


namespace Scaleform { namespace Render { namespace GL {


class HAL : public Render::HAL
#ifdef SF_GL_RUNTIME_LINK
    , public GL::Extensions
#endif
{
public:

    MemoryHeap*         pHeap;

    // HAL State flags are checked for during most API calls to ensure that
    // state is valid for the operation. Check is done by checkState(bits),
    // which will emit error message for some cases.
    enum HALStateFlags
    {
        HS_ModeSet          = 0x0001,
        HS_InFrame          = 0x0002,
        HS_InScene          = 0x0004,
        HS_InDisplay        = 0x0008,  // Rendering fails if not in display.
        HS_ViewValid        = 0x0010,  // Non-empty viewport; culls rendering.
        HS_DeviceValid      = 0x0020,  // If not valid, device is Lost.
        HS_DrawingMask      = 0x0100,  // Set when we are drawing mask
        HS_SceneInDisplay   = 0x1000,  // Set if BeginScene was called by BeginDisplay.
    };

    unsigned            HALState;

    bool                UsePixelShaders, MultiBitStencil;
    // Video Mode Configuration Flags (VMConfigFlags)
    UInt32              VMCFlags;
    UInt32		        FillFlags;
   

    // Output size.    
    Viewport            VP;
    int                 RenderMode;

    Rect<int>           ViewRect;
    
    MeshCache           Cache;
    RenderQueue         Queue;
    RenderQueueProcessor QueueProcessor;

    ShaderManager       SManager;
    ShaderObject        StaticShaders[FragShaderDesc::FS_Count * 2];
    ShaderInterface     ShaderData;
    Ptr<TextureManager> pTextureManager;
    
    // Previous batching mode
    PrimitiveBatch::BatchType PrevBatchType;

    Stats               AccumulatedStats;


    // *** Mask Support
    
    struct MaskStackEntry
    {
        Ptr<MaskPrimitive> pPrimitive;
        bool               OldViewportValid;
        Rect<int>          OldViewRect;
    };

    typedef ArrayConstPolicy<0, 8, true> NeverShrinkPolicy;
    typedef ArrayLH<MaskStackEntry, Stat_Default_Mem, NeverShrinkPolicy> MaskStackType;

    MaskStackType   MaskStack;
    // Active stack top; 0 for empty stack. We track this separately from stack size
    // to allow PopMask optimization. Last item above top may be needed to 
    // erased previous mask bounds when entering a new nested mask.
    UPInt           MaskStackTop;

    // BlendModeStack (holds blend modes).
    typedef ArrayLH<BlendMode, Stat_Default_Mem, NeverShrinkPolicy> BlendStackType;
    BlendStackType  BlendModeStack;

    // Self-accessor used to avoid constructor warning.
    HAL*      getThis() { return this; }

public:    
    

    HAL();
    virtual ~HAL();

    
    // Outputs debug warnings for missing states.
#ifdef SF_BUILD_DEBUG    
    void        checkState_EmitWarnings(unsigned stateFlags, const char* funcName);
#else
    void        checkState_EmitWarnings(unsigned, const char*) { }
#endif

    // Checks that the required HALState bit flags are present; called in front of
    // various rendering calls.
    inline bool checkState(unsigned stateFlags, const char* funcName)
    {
        if ((HALState & stateFlags) != stateFlags)
        {
            checkState_EmitWarnings(stateFlags, funcName);
            return false;
        }
        return true;
    }
    

    // *** Implement Dependent Video Mode configuration

    virtual bool        SetDependentVideoMode(TextureManager* ptextureManager = 0);

    // Returns back to original mode (cleanup)
    virtual bool        ResetVideoMode();

    virtual bool        IsVideoModeSet() const { return (HALState & HS_ModeSet) != 0; }

    // *** Rendering

    virtual bool        BeginFrame();
    virtual void        EndFrame();

    virtual bool        BeginScene();
    virtual void        EndScene();

    // Bracket the displaying of a frame from a movie.
    // Fill the background color, and set up default transforms, etc.
    virtual void        BeginDisplay(Color backgroundColor,
                                     const Viewport& viewport);

    virtual void        EndDisplay();

    virtual void        GetHWViewMatrix(Matrix* pmatrix, const Viewport& vp);

    void                CalcHWViewMatrix(unsigned VPFlags, Matrix* pmatrix, const Rect<int>& viewRect,
                                         int dx, int dy);

    // Updates HW Viewport and ViewportMatrix based on the current
    // values of VP, ViewRect and ViewportValid.
    void                updateViewport();


    // Creates / Destroys mesh and DP data 

    virtual PrimitiveFill*  CreatePrimitiveFill(const PrimitiveFillData& data);    

    // Draw a DP where every mesh element is transformed by the specified matrix.
    // Note: matrixCount isn't technically necessary.
    virtual void        Draw(const RenderQueueItem& item);
    
    inline  void        Draw(RenderQueueItem::Interface* i, void* data = 0)
    {
        Draw(RenderQueueItem(i, data));
    }

    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend);

    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix>& matrices);
    
    // *** Mask Support

    // Mask support is implemented as a stack, with three possible operations doing
    // the following:
    //  PushMask_BeginSubmit - pushes a new mask on stack and begins "submit mask" rendering.
    //  EndMaskSubmit - Ends submit mask and begins content rendering, clipped by the mask.
    //  PopMask - pops the mask from stack; further rendering will use previous masks, if any.

    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);    
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    void    drawMaskClearRectangles(const HMatrix* matrices, UPInt count);
    void    clearSolidRectangle(const Rect<int>& r, Color color);

    void    beginMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        HALState &= ~HS_DrawingMask;
    }
    void   endMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        MaskStackTop = 0;
        MaskStack.Clear();
    }

    bool SetVertexArray(const ShaderObject* pshader, const VertexFormat* pFormat, GLuint buffer, UPInt vertexOffset);


    // *** BlendMode

    virtual void    PushBlendMode(BlendMode mode);
    virtual void    PopBlendMode();

    void            applyBlendMode(BlendMode mode);



    virtual void        GetStats(Stats* pstats, bool clear = true)
    {
        *pstats = AccumulatedStats;
        if (clear)
            AccumulatedStats.Clear();
    }

    virtual Render::TextureManager* GetTextureManager() const
    {
        return pTextureManager.GetPtr();
    }

    virtual class MeshCache&              GetMeshCache()
    {
        return Cache;
    }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced);

    virtual RQCacheInterface& GetRQCacheInterface()
    {
        return QueueProcessor.GetQueueCachesRef();
    }
};


}}} // Scaleform::Render::GL

#endif
