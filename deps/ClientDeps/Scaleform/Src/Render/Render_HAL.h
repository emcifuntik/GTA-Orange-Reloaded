/**********************************************************************

PublicHeader:   Render
Filename    :   HAL.h
Content     :   Renderer HAL Prototype header.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_RenderHAL_H
#define INC_SF_Render_RenderHAL_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_AllocAddr.h"

#include "Render_Types2D.h"
#include "Render_Color.h"
#include "Render_Vertex.h"
#include "Render_Containers.h"
#include "Render_Primitive.h"
#include "Render_Viewport.h"
#include "Render_States.h"

namespace Scaleform { namespace Render {

typedef Matrix2F Matrix;



struct MatrixState
{
    Matrix2F ViewportMatrix;
 
    Matrix3F View3D;
    Matrix4F Proj3D;

    Matrix2F User;
    Matrix4F User3D;
    Matrix2F UserVP;

    mutable Matrix4F UVP;
    mutable bool     UVPChanged;
    bool             UserVPSet;

    MatrixState()
    {
        UVPChanged = 0;
        UserVPSet = 0;
    }

    const Matrix4F& GetUVP() const
    {
        if (UVPChanged)
        {
            UVP = User3D * Proj3D * View3D;
            UVPChanged = 0;
        }
        return UVP;
    }
};

// 
enum HALNotifyType
{
    HAL_SetVideoMode,
    HAL_ResetVideoMode
};

// HALNotify interface can be installed on HAL to be informed of events such
// as video mode changes. Overridden by Renderer2DImpl.
class HALNotify : public ListNode<HALNotify>
{
public:
    HALNotify() { pPrev = pNext = 0; }
    virtual ~HALNotify() {}

    virtual void OnHALEvent(HALNotifyType type) = 0;
};


class ProfileViews
{
public:
    enum ProfileFlags
    {
        Channel_Red    = 0,
        Channel_Green  = 16,
        Channel_Blue   = 32,
        Profile_Fill   = 0x100,
        Profile_Mask   = 0x200,
        Profile_Clear  = 0x400,
        Profile_Batch  = 0x800,
    };

#ifdef SF_RENDERER_PROFILE
private:
    BlendMode OverrideBlend;
    bool      OverrideMasks, FillMode;
    unsigned  BatchMode;
    Cxform    FillCxforms[3];

    int       DrawMode;
    Color     BatchColor;

    mutable Hash<UInt64, Color>   BatchColorLookup;

    Color     GetColorForBatch(UPInt base, unsigned index) const;

public:
    ProfileViews()
    {
        DisableProfileViews();
    }

    void DisableProfileViews() 
    {
        OverrideBlend = Blend_None;
        OverrideMasks = 0;
        FillMode = 0;
        BatchMode = 0;
    }

    void SetProfileViews (UInt64 modes);


    void      SetDrawMode(int mode)
    {
        DrawMode = mode;
    }
    void      SetBatch(UPInt batchKey, unsigned batchIndex)
    {
        BatchColor = GetColorForBatch(batchKey, batchIndex);
    }

    Color     GetClearColor(Color c) const
    {
        return (FillMode || BatchMode) ? Color(0,0,0,255) : c;
    }

    BlendMode GetBlendMode(BlendMode mode) const
    {
        return (OverrideBlend == Blend_None) ? mode : OverrideBlend;
    }

    PrimitiveFillType GetFillType(PrimitiveFillType fill) const
    {
        return (OverrideMasks && fill == PrimFill_Mask) ? PrimFill_SolidColor : fill;
    }

    bool      ShouldDrawMask() const
    {
        return !OverrideMasks;
    }

    Color     GetColor(Color color) const
    {
        if (FillMode || BatchMode)
            return GetCxform(Cxform::Identity).Transform(color);
        else
            return color;
    }

    Cxform    GetCxform(const Cxform& cx) const
    {
        Cxform outCx(Cxform::NoInit);
        if (FillMode)
            outCx = FillCxforms[DrawMode];
        else
            outCx = cx;
        if (BatchMode)
        {
            Cxform batch (BatchColor);
            for (int i = 0; i < 4; i++)
                if (BatchMode & (1 << i))
                {
                    outCx.M[i][0] = batch.M[i][0];
                    outCx.M[i][1] = batch.M[i][1];
                }
        }
        return outCx;
    }

#else
    void      DisableProfile() {}
    void      ProfileCounters (UInt64) {}

    void      SetDrawMode(int) {}
    void      SetBatch(UPInt, unsigned) {}

    Color             GetClearColor(Color c) const { return c; }
    BlendMode         GetBlendMode(BlendMode mode) const { return mode; }
    PrimitiveFillType GetFillType(PrimitiveFillType fill) const { return fill; }
    Color             GetColor(Color color) const { return color; }
    Cxform            GetCxform(const Cxform& cx) const { return cx; }

    bool              ShouldDrawMask() const { return true; }
#endif
};

//--------------------------------------------------------------------
// Abstract renderer interface.

class HAL : public RefCountBase<HAL, StatRender_Mem>
{
public:

    HAL() { Link_RenderStats(); }

    struct Stats
    {
        unsigned Primitives;
        unsigned Meshes;
        unsigned Triangles;

        Stats() : Primitives(0), Meshes(0), Triangles(0) { }

        void Clear() { Primitives = Meshes = Triangles = 0; }
    };

    virtual bool        IsVideoModeSet() const = 0;

    // ***** Rendering

    // Begins rendering frame, which is a caching unit. BeginFrame/EndFrame pair
    // must always be called around BeginScene/EndScene.
    virtual bool        BeginFrame() = 0;
    virtual void        EndFrame() = 0;

    // BeginScene begins scene rendering, initializing various render states needed
    // externally; EndScene completes scene rendering. In Direct3D, this will call
    // BeginScene/EndScene flags on the device if the 
    // If not called explicitly, these functions will be automatically called from
    // BeginDisplay and EndDisplay, explicitly. Calling them externally is more
    // efficient if multiple BeginDisplay/EndDisplay blocks will take place, as it
    // optimizes state changes and eliminates queue flush.
    virtual bool        BeginScene() = 0;
    virtual void        EndScene() = 0;

    // Bracket rendering of a display unit that potentially has its own
    // viewport, such Render::TreeNode (GFx::MovieView).
    // Fill the background color, and set up default transforms, etc.
    virtual void        BeginDisplay(Color backgroundColor,
                                     const Viewport& viewport) = 0;
    virtual void        EndDisplay() = 0;

    // Set "User" matrix that is applied to shift the view just before viewport.
    // Should be called before BeginDisplay.
    virtual void        SetUserMatrix(const Matrix& m) { Matrices.User = m; }
    virtual void        SetUserMatrix3D(const Matrix4F& m) { Matrices.User3D = m; Matrices.UVPChanged = 1; }
    // Set matrix used to adjust the viewport before passing it to the device.
    // This should match the UserMatrix for transforms of the entire movie.
    virtual void        SetUserViewportMatrix(const Matrix& m)
    {
        Matrices.UserVP = m;
        Matrices.UserVPSet = (Matrices.UserVP != Matrix::Identity);
    }

    // TBD: No longer needed
    virtual void        GetHWViewMatrix(Matrix* pmatrix, const Viewport& vp)    = 0;


    // Creates / Destroys mesh and DP data 

    virtual PrimitiveFill*  CreatePrimitiveFill(const PrimitiveFillData& data) = 0;    


    // Draw a DP where every mesh element is transformed by the specified matrix.
    // Note: matrixCount isn't technically necessary.
    virtual void        Draw(const RenderQueueItem& item) = 0;
    
    inline  void        Draw(RenderQueueItem::Interface* i, void* data = 0)
    {
        Draw(RenderQueueItem(i, data));
    }
    
    virtual void        DrawProcessedPrimitive(Primitive* pprimitive,
                                               PrimitiveBatch* pstart, PrimitiveBatch *pend) = 0;
    virtual void        DrawProcessedComplexMeshes(ComplexMesh* p,
                                                   const StrideArray<HMatrix> &matrices) = 0;

    virtual void        PushMask_BeginSubmit(MaskPrimitive* primitive) = 0;
    virtual void        EndMaskSubmit() = 0;
    virtual void        PopMask() = 0;

    // BlendMode
    virtual void        PushBlendMode(BlendMode mode) = 0;
    virtual void        PopBlendMode() = 0;


    virtual void        GetStats(Stats* pstats, bool clear = true)
    {
        SF_UNUSED(clear);
        // for now
        pstats->Primitives = 0;
        pstats->Meshes     = 0;
        pstats->Triangles  = 0;
    }

    // Texture manager
    virtual TextureManager* GetTextureManager() const = 0;

    // for now    
    virtual class MeshCache&  GetMeshCache() = 0;
    virtual RQCacheInterface& GetRQCacheInterface() = 0;

    // Obtains formats that renderer will use for single, batches and instanced rendering of
    // the specified source format.
    //   - Filled in pointer may be the same as sourceFormat.
    //   - 'instanced' format may be reported as 0, in which instancing is not supported for format.
    virtual void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                    const VertexFormat** single,
                                    const VertexFormat** batch, const VertexFormat** instanced) = 0;
                                     
    // Profile view modes
    virtual void    SetProfileViews (UInt64 modes) { 
#ifdef SF_RENDERER_PROFILE
        Profiler.SetProfileViews(modes); 
#else
        SF_UNUSED(modes);
#endif
    }

    // Adds an external notification object that will be told about HAL
    // events such as SetVideoMode/ResetVideoMode. Should be removed with RemoveNotify.
    void AddNotify(HALNotify *notify)
    {
        SF_ASSERT(notify->pPrev == 0);
        NotifyList.PushBack(notify);
    }
    void RemoveNotify(HALNotify *notify)
    {
        notify->RemoveNode();
    }    

    // MOOSE TODO - TEMP
    void SetView3D(const Matrix3F &m) { Matrices.View3D = m; Matrices.UVPChanged = 1; }
    void SetProj3D(const Matrix4F &m) { Matrices.Proj3D = m; Matrices.UVPChanged = 1; }

    const Matrix3F &GetView3D() const { return Matrices.View3D; }
    const Matrix4F &GetProj3D() const { return Matrices.Proj3D; }

    const MatrixState& GetMatrices() const { return Matrices; }

protected:

    void notifyHandlers(HALNotifyType type)
    {
        HALNotify *p = NotifyList.GetFirst();
        while (!NotifyList.IsNull(p))
        {
            // Allow item itself to be removed during notification.
            HALNotify *next = p->pNext;
            p->OnHALEvent(type);
            p = next;
        }
    }

    List<HALNotify> NotifyList;
    MatrixState     Matrices;
    ProfileViews    Profiler;
};


}} // Scaleform::Render

#endif
