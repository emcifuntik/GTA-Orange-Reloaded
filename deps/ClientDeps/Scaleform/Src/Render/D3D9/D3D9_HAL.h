/**********************************************************************

Filename    :   D3D9_HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_D3D9_HAL_H
#define INC_SF_Render_D3D9_HAL_H

#include "Render\Render_HAL.h"
#include "Render\Render_Queue.h"
#include "D3D9_MeshCache.h"
#include "D3D9_ShaderManager.h"
#include "D3D9_Texture.h"

#include <d3d9.h>

namespace Scaleform { namespace Render { namespace D3D9 {

#define SF_RENDERER_SHADER_VERSION  0x0200
#define SF_RENDERER_VSHADER_PROFILE "vs_1_1"


class HAL : public Render::HAL
{
public:

    // Direct3D Device we are using.
    IDirect3DDeviceX*   pDevice;
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

    bool                UsePixelShaders;
    // Video Mode Configuration Flags (VMConfigFlags)
    UInt32              VMCFlags;
	UInt16				FillFlags;

    // Presentation parameters specified to configure the mode.
    D3DPRESENT_PARAMETERS   PresentParams;
    HWND                    hWnd;

    // Output size.    
    Viewport            VP;
    int                 RenderMode;
    
    Rect<int>           ViewRect;    
    
    MeshCache           Cache;
    RenderQueue         Queue;
    RenderQueueProcessor QueueProcessor;

    ShaderManager       SManager;
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

    // This flag indicates whether we've checked for stencil after BeginDisplay or not.
    bool            StencilChecked;
    // This flag is stencil is available, after check.
    bool            StencilAvailable;
    bool            MultiBitStencil;
    bool            DepthBufferAvailable;    
    // Increment op we need for stencil.
    D3DSTENCILOP    StencilOpInc;    

    // BlendModeStack (holds blend modes).
    typedef ArrayLH<BlendMode, Stat_Default_Mem, NeverShrinkPolicy> BlendStackType;
    BlendStackType  BlendModeStack;
    

    bool                                ProfileBatches;
    struct BatchColorKey
    {
        unsigned    Base;
        unsigned    Offset;
        BatchColorKey(unsigned base, unsigned offset) : Base(base), Offset(offset) {}
        bool operator== (const BatchColorKey& key) const { return (Base == key.Base) && (Offset == key.Offset); }
    };
    mutable Hash<BatchColorKey, Color>   BatchColorLookup;
    Color                               CurrentBatchColor;
    Color                               GetColorForBatch(const BatchColorKey& key) const;
    
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

    enum VMConfigFlags
    {
        // Prevents BeginScene/EndScene from being called inside BeginDisplay/EndDisplay
        // - assumes that Direct3D is already in scene by the time BeginDisplay/EndDisplay are called
        // - allows user to manage their own begin/end scene calls 
        VMConfig_NoSceneCalls = 0x00000001,
        VMConfig_CombinePrims = 0x00000002,

        // Use D3DUSAGE_DYNAMIC updateable textures for compatibility with some broken drivers
        VMConfig_UseDynamicTex = 0x00000004,
    };

    virtual bool        SetDependentVideoMode(IDirect3DDeviceX* pd3dDevice,
                                              D3DPRESENT_PARAMETERS* ppresentParams,
                                              TextureManager* ptextureManager = 0,
                                              UInt32 vmConfigFlags = 0, HWND hwnd = 0);

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

    void                CalcHWViewMatrix(Matrix* pmatrix, const Rect<int>& viewRect,
                                         int dx, int dy);

    // Updates D3D HW Viewport and ViewportMatrix based on the current
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

    template< class MatrixUpdateAdapter >
    void                applyMatrixConstants(const MatrixUpdateAdapter & input);

    void                applyRawMatrixConstants(const Matrix& m, const Cxform& cx);

    template< class MatrixType >
    static void         calculateTransform(const Matrix & m, const HMatrix& hm, const MatrixState & mstate, float (* dest)[4]);

    
    // Stream Source modification

    inline void         setLinearStreamSource(PrimitiveBatch::BatchType type)
    {
        if (PrevBatchType >= PrimitiveBatch::DP_Instanced)
        {
            pDevice->SetStreamSource(1, NULL, 0, 0);
            pDevice->SetStreamSourceFreq(0, 1);
            pDevice->SetStreamSourceFreq(1, 1);
        }
        PrevBatchType = type;
    }

    inline void         setInstancedStreamSource(unsigned instanceCount)
    {
        if (PrevBatchType != PrimitiveBatch::DP_Instanced)
        {
            pDevice->SetStreamSource(1, Cache.pInstancingVertexBuffer.GetPtr(),
                                     0, sizeof(Render_InstanceData));
            pDevice->SetStreamSourceFreq(1, (UINT) D3DSTREAMSOURCE_INSTANCEDATA | 1);
            PrevBatchType = PrimitiveBatch::DP_Instanced;
        }                
        pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | instanceCount);
    }


    // *** Mask Support

    // Mask support is implemented as a stack, with three possible operations doing
    // the following:
    //  PushMask_BeginSubmit - pushes a new mask on stack and begins "submit mask" rendering.
    //  EndMaskSubmit - Ends submit mask and begins content rendering, clipped by the mask.
    //  PopMask - pops the mask from stack; further rendering will use previous masks, if any.

    virtual void    PushMask_BeginSubmit(MaskPrimitive* primitive);
    virtual void    EndMaskSubmit();
    virtual void    PopMask();

    void    beginMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        StencilChecked  = 0;
        StencilAvailable= 0;
        MultiBitStencil = 0;
        DepthBufferAvailable = 0;
        HALState &= ~HS_DrawingMask;
    }
    void   endMaskDisplay()
    {
        SF_ASSERT(MaskStackTop == 0);
        MaskStackTop = 0;
        MaskStack.Clear();
    }

    bool    checkMaskBufferCaps();
    void    drawMaskClearRectangles(const HMatrix* matrices, UPInt count);

    // Background clear helper, expects viewport coordinates.
    void    clearSolidRectangle(const Rect<int>& r, Color color);


    // *** BlendMode

    virtual void    PushBlendMode(BlendMode mode);
    virtual void    PopBlendMode();

    void            applyBlendMode(BlendMode mode);



    IDirect3DDeviceX*   GetDevice() const { return pDevice; }


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

    virtual class MeshCache& GetMeshCache()
    {
        return Cache;
    }
    
    virtual RQCacheInterface& GetRQCacheInterface()
    {
        return QueueProcessor.GetQueueCachesRef();
    }
        
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced)
    {
        return SManager.MapVertexFormat(fill, sourceFormat,
                                        single, batch, instanced);
    }

    // Profile view modes
    virtual void    EnableProfileViews(bool showBatches);
};

// Used to create heirarchies in PIX captures.
class PixMarker
{
public:
    PixMarker( LPCWSTR eventName, bool trigger = true )
    {
        // Colors aren't shown in PIX.
        if ( trigger )
            Begin(eventName);
    }
    void Begin( LPCWSTR eventName)
    {
        D3DPERF_BeginEvent(D3DCOLOR_XRGB(0,0,0), eventName);
    }
    void End()
    {
        D3DPERF_EndEvent();
    }

    ~PixMarker( )
    {
        End();
    }
};


}}} // Scaleform::Render::D3D9

#endif
