/**********************************************************************

Filename    :   GL_HAL.cpp
Content     :   GL Renderer HAL Prototype implementation.
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"
#include "Kernel/SF_Random.h"

#include "GL_HAL.h"

namespace Scaleform { namespace Render { namespace GL {


// ***** RenderHAL_GL

HAL::HAL()
:   HALState(0),MultiBitStencil(1),
    FillFlags(0),
    RenderMode(0),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    Queue(),
    QueueProcessor(Queue, getThis()),
    SManager(&Profiler),
    ShaderData(getThis()),
    PrevBatchType(PrimitiveBatch::DP_None),
    // Mask/Stencil vars
    MaskStackTop(0)
{
    pHeap = Memory::GetGlobalHeap();
    memset(StaticShaders, 0, sizeof(StaticShaders));
}

HAL::~HAL()
{
    ResetVideoMode();
}

#ifdef SF_BUILD_DEBUG

void HAL::checkState_EmitWarnings(unsigned stateFlags, const char* funcName)
{    
    // Outputs debug warnings for missing states.
    struct WarnHelper
    {
        unsigned CheckFlags, HALState;
        WarnHelper(unsigned cf, unsigned hs) : CheckFlags(cf), HALState(hs) { }

        bool operator ()(unsigned checkBit, unsigned requiredBits) const
        {
            return (CheckFlags & checkBit) &&
                   ((HALState & (requiredBits | checkBit)) == requiredBits);
        }
    };

    WarnHelper needWarning(stateFlags, HALState);

    // TBD: WE need a better solution then secondary mask for when to NOT display warnings.
    //      Once BeginFrame fails, for example, there is no need to warn on all other calls.
    SF_DEBUG_WARNING1(needWarning(HS_ModeSet, 0),
                      "GL::HAL::%s failed - Mode Not Set", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InFrame, 0),
                      "GL::HAL::%s failed - Begin/EndFrame missing/failed.", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InDisplay, HS_InFrame|HS_ModeSet),
                      "GL::HAL::%s failed - Begin/EndDisplay missing/failed.", funcName);

    SF_DEBUG_WARNING1(needWarning(HS_DeviceValid, 0),
                      "GL::HAL::%s failed - Device Lost or not valid", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_DrawingMask, HS_InFrame|HS_InDisplay),
                      "GL::HAL::%s failed - PushMask_BeginSubmit call missing", funcName);
}

#endif

void   HAL::MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                                  const VertexFormat** single,
                                  const VertexFormat** batch, const VertexFormat** instanced)
{
    VertexElement        outElements[8];
    VertexFormat         outFormat;
    outFormat.Size = sourceFormat->Size;
    outFormat.pElements = outElements;

    const VertexElement* pve = sourceFormat->pElements;
    VertexElement* pveo = outElements;
    for (; pve->Attribute != VET_None; pve++, pveo++)
    {
        *pveo = *pve;
        if ((pve->Attribute & (VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask)) == VET_ColorARGB8)
            pveo->Attribute = VET_ColorRGBA8 | (pve->Attribute & ~(VET_Usage_Mask|VET_CompType_Mask|VET_Components_Mask));
    }

    return SManager.MapVertexFormat(fill, &outFormat, single, batch, instanced);
}

bool HAL::SetVertexArray(const ShaderObject* pshader, const VertexFormat* pFormat, GLuint buffer, UPInt vertexOffset)
{
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    const VertexElement* pve = pFormat->pElements;
    int vi = 0;
    for (; pve->Attribute != VET_None; pve++, vi++)
    {
        GLenum vet; bool norm;
        GLuint offset = pve->Offset;
        GLuint ac = (pve->Attribute & VET_Components_Mask);

        switch (pve->Attribute & VET_CompType_Mask)
        {
        case VET_U8:  vet = GL_UNSIGNED_BYTE; norm = false; break;
        case VET_U8N: vet = GL_UNSIGNED_BYTE; norm = true; break;
        case VET_U16: vet = GL_UNSIGNED_SHORT; norm = false; break;
        case VET_S16: vet = GL_SHORT; norm = false; break;
        case VET_U32: vet = GL_UNSIGNED_INT; norm = false; break;
        case VET_F32: vet = GL_FLOAT; norm = false;  break;

        default: SF_ASSERT(0); vet = GL_FLOAT; norm = false; break;
        }

        // Packed factors
        if (((pve->Attribute | pve[1].Attribute) & (VET_Usage_Mask|VET_Index_Mask)) == (VET_Color | (3 << VET_Index_Shift)))
        {
            ac = 4;
            pve++;
            offset = pve->Offset-3;
        }

        glEnableVertexAttribArray(vi);
        glVertexAttribPointer(vi, ac, vet, norm, pFormat->Size, (void*)(vertexOffset + offset));
    }

    for (int i = vi; i < 16; i++)
        glDisableVertexAttribArray(i);

    SF_UNUSED(pshader);
    return true;
}

PrimitiveFill*  HAL::CreatePrimitiveFill(const PrimitiveFillData &data)
{
    return SF_HEAP_NEW(pHeap) PrimitiveFill(data);
}

// Draws a range of pre-cached and preprocessed primitives
void        HAL::DrawProcessedPrimitive(Primitive* pprimitive,
                                        PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    if (!checkState(HS_InDisplay, "DrawProcessedPrimitive") ||
        !pprimitive->GetMeshCount() )
        return;

    SF_ASSERT(pend != 0);
    
    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();

    ShaderData.BeginPrimitive();

    unsigned bidx = 0;
    while (pbatch != pend)
    {        
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        MeshCacheItem* pmesh = (MeshCacheItem*)pbatch->GetCacheItem();
        unsigned       meshIndex = pbatch->GetMeshIndex();
        unsigned       batchMeshCount = pbatch->GetMeshCount();

        if (pmesh)
        {
            Profiler.SetBatch((UPInt)pprimitive, bidx);

            UInt32 fillFlags = FillFlags;
            if ( batchMeshCount > 0 )
                fillFlags |= pprimitive->Meshes[0].M.Has3D() ? FF_3DProjection : 0;

            const ShaderObject* pShader =
                SManager.SetPrimitiveFill(pprimitive->pFill, fillFlags, pbatch->Type, batchMeshCount, Matrices,
                                          &pprimitive->Meshes[meshIndex], &ShaderData);

            if ((HALState & HS_ViewValid) && pShader &&
                SetVertexArray(pShader, pbatch->pFormat, pmesh->pVertexBuffer->GetBuffer(), pmesh->VBAllocOffset))
            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                          (pbatch->Type != PrimitiveBatch::DP_Virtual));

                // Draw the object with cached mesh.
                if (pbatch->Type != PrimitiveBatch::DP_Instanced)
                {
                    AccumulatedStats.Meshes += pmesh->MeshCount;
                    AccumulatedStats.Triangles += pmesh->IndexCount / 3;
                }
                else
                {
                    SF_DEBUG_WARNING(1, "instanced draw");
                    AccumulatedStats.Meshes += batchMeshCount;
                    AccumulatedStats.Triangles += (pmesh->IndexCount / 3) * batchMeshCount;
                }

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pmesh->pIndexBuffer->GetBuffer());
                glDrawElements(GL_TRIANGLES, pmesh->IndexCount, GL_UNSIGNED_SHORT, (void*)pmesh->IBAllocOffset);

                AccumulatedStats.Primitives++;
            }

            pmesh->MoveToCacheListFront(MCL_ThisFrame);
        }

        pbatch = pbatch->GetNext();
        bidx++;
    }

    // This assert shouldn't be here in the future since queue can have many items
   // SF_ASSERT(Cache.CachedItems[MCL_InFlight].IsEmpty());
}


void HAL::DrawProcessedComplexMeshes(ComplexMesh* complexMesh,
                                           const StrideArray<HMatrix>& matrices)
{    
    typedef ComplexMesh::FillRecord   FillRecord;
    typedef PrimitiveBatch::BatchType BatchType;
    
    MeshCacheItem* pmesh = (MeshCacheItem*)complexMesh->GetCacheItem();
    if (!checkState(HS_InDisplay, "DrawProcessedComplexMeshes") || !pmesh)
        return;
    
    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    BatchType   batchType = PrimitiveBatch::DP_Single;

    const Matrix2F* textureMatrices = complexMesh->GetFillMatrixCache();

    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        Profiler.SetBatch((UPInt)complexMesh, fillIndex);
        ShaderData.BeginPrimitive();

        UInt32 fillFlags = FillFlags;
        if ( instanceCount > 0 )
        {
            const HMatrix& hm = matrices[0];
            fillFlags |= hm.Has3D() ? FF_3DProjection : 0;
        }

        // Apply fill.
        PrimitiveFillType fillType = Profiler.GetFillType(fr.pFill->GetType());
        const ShaderObject* pso = SManager.SetFill(fillType, fillFlags, batchType, &ShaderData);
        SetVertexArray(pso, fr.pFormats[0], pmesh->pVertexBuffer->GetBuffer(), pmesh->VBAllocOffset + fr.VertexByteOffset);

        UByte textureCount = fr.pFill->GetTextureCount();
        unsigned startIndex = 0;
        bool solid = (fillType == PrimFill_None || fillType == PrimFill_Mask || fillType == PrimFill_SolidColor);

        for (unsigned i = 0; i < instanceCount; i++)
        {            
            const HMatrix& hm = matrices[startIndex + i];

            ShaderData.SetMatrix(pso, Uniform::SU_mvp, complexMesh->GetVertexMatrix(), hm, Matrices);
            if (solid)
                ShaderData.SetColor(pso, Uniform::SU_cxmul, Profiler.GetColor(fr.pFill->GetSolidColor()));
            else
                ShaderData.SetCxform(pso, Profiler.GetCxform(hm.GetCxform()));

            for (unsigned tm = 0, stage = 0; tm < textureCount; tm++)
            {
                ShaderData.SetMatrix(pso, Uniform::SU_texgen, textureMatrices[fr.FillMatrixIndex[tm]], tm);
                Texture* ptex = (Texture*)fr.pFill->GetTexture(tm);
                ShaderData.SetTexture(pso, stage, ptex, fr.pFill->GetFillMode(tm));
                stage += ptex->GetPlaneCount();
            }

            ShaderData.Finish(0);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pmesh->pIndexBuffer->GetBuffer());
            glDrawElements(GL_TRIANGLES, fr.IndexCount, GL_UNSIGNED_SHORT,
                (void*)(pmesh->IBAllocOffset + sizeof(IndexType) * fr.IndexOffset));
        }

        AccumulatedStats.Triangles += (fr.IndexCount / 3) * instanceCount;
        AccumulatedStats.Meshes += instanceCount;
        AccumulatedStats.Primitives += instanceCount;

    } // for (fill record)
  
    pmesh->MoveToCacheListFront(MCL_ThisFrame);
}


//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color)
{
    color = Profiler.GetClearColor(color);

    if (color.GetAlpha() == 0xFF)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(VP.Left + r.x1, VP.BufferHeight-VP.Top-r.y1-(r.y2-r.y1), r.x2-r.x1, r.y2-r.y1);
        glClearColor(color.GetRed() * 1.f/255.f, color.GetGreen() * 1.f/255.f, color.GetBlue() * 1.f/255.f, 1);
        glClear(GL_COLOR_BUFFER_BIT);

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
        float colorf[4];
        color.GetRGBAFloat(colorf, colorf+1, colorf+2, colorf+3);
        Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
                   0.0f, (float)r.Height(), (float)r.y1);

        Matrix2F  mvp(m, Matrices.ViewportMatrix);

        const ShaderObject* pso = SManager.SetFill(PrimFill_SolidColor, 0, PrimitiveBatch::DP_Single, &ShaderData);
        ShaderData.SetMatrix(pso, Uniform::SU_mvp, mvp);
        ShaderData.SetUniform(pso, Uniform::SU_cxmul, colorf, 4);
        ShaderData.Finish(1);

        SetVertexArray(pso, &VertexXY16iInstance::Format, Cache.MaskEraseBatchVertexBuffer, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
}

//--------------------------------------------------------------------
// *** Mask / Stencil support
//--------------------------------------------------------------------

// Mask support is implemented as a stack, enabling for a number of optimizations:
//
// 1. Large "Clipped" marks are clipped to a custom viewport, allowing to save on
//    fill-rate when rendering both the mask and its content. The mask area threshold
//    that triggers this behavior is determined externally.
//      - Clipped masks can be nested, but not batched. When erased, clipped masks
//        clear the clipped intersection area.
// 2. Small masks can be Batched, having multiple mask areas with multiple mask
//    content items inside.
//      - Small masks can contain clipped masks either regular or clipped masks.
// 3. Mask area dimensions are provided as HMatrix, which maps a unit rectangle {0,0,1,1}
//    to a mask bounding rectangle. This rectangle can be rotated (non-axis aligned),
//    allowing for more efficient fill.
// 4. PopMask stack optimization is implemented that does not erase nested masks; 
//    Stencil Reference value is changed instead. Erase of a mask only becomes
//    necessary if another PushMask_BeginSubmit is called, in which case previous
//    mask bounding rectangles are erased. This setup avoids often unnecessary erase 
//    operations when drawing content following a nested mask.
//      - To implement this MaskStack keeps a previous "stale" MaskPrimitive
//        located above the MaskStackTop.


void HAL::PushMask_BeginSubmit(MaskPrimitive* prim)
{
    if (!checkState(HS_InDisplay, "PushMask_BeginSubmit"))
        return;

    Profiler.SetDrawMode(1);
    if (!Profiler.ShouldDrawMask())
        return;

    glColorMask(0,0,0,0);                       // disable framebuffer writes
    glEnable(GL_STENCIL_TEST);

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid && MultiBitStencil)
    {
        glStencilFunc(GL_LEQUAL, MaskStackTop, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        MaskPrimitive* erasePrim = MaskStack[MaskStackTop].pPrimitive;
        drawMaskClearRectangles(erasePrim->GetMaskAreaMatrices(), erasePrim->GetMaskCount());
    }

    MaskStack.Resize(MaskStackTop+1);
    MaskStackEntry &e = MaskStack[MaskStackTop];
    e.pPrimitive       = prim;
    e.OldViewportValid = viewportValid;
    e.OldViewRect      = ViewRect; // TBD: Must assign
    MaskStackTop++;

    HALState |= HS_DrawingMask;

    if (prim->IsClipped() && viewportValid)
    {
        Rect<int> boundClip;

        // Apply new viewport clipping.
        if (!Matrices.UserVPSet)
        {
            const Matrix2F& m = prim->GetMaskAreaMatrix(0).GetMatrix2D();

            // Clipped matrices are always in View coordinate space, to allow
            // matrix to be use for erase operation above. This means that we don't
            // have to do an EncloseTransform.
            SF_ASSERT((m.Shx() == 0.0f) && (m.Shy() == 0.0f));
            boundClip = Rect<int>(VP.Left + (int)m.Tx(), VP.Top + (int)m.Ty(),
                                  VP.Left + (int)(m.Tx() + m.Sx()), VP.Top + (int)(m.Ty() + m.Sy()));
        }
        else
        {
            Matrix2F m = prim->GetMaskAreaMatrix(0).GetMatrix2D();
            m.Append(Matrices.UserVP);

            RectF rect = m.EncloseTransform(RectF(0,0,1,1));
            boundClip = Rect<int>(VP.Left + (int)rect.x1, VP.Top + (int)rect.y1,
                                  VP.Left + (int)rect.x2, VP.Top + (int)rect.y2);
        }

        if (!ViewRect.IntersectRect(&ViewRect, boundClip))
        {
            ViewRect.Clear();
            HALState &= ~HS_ViewValid;
            viewportValid = false;
        }
        updateViewport();
        
        // Clear full viewport area, which has been resized to our smaller bounds.
        if ((MaskStackTop == 1) && viewportValid)
        {
            glClearStencil(0);
            glClear(GL_STENCIL_BUFFER_BIT);
        }
    }
    else if ((MaskStackTop == 1) && viewportValid)
    {
        glStencilFunc(GL_ALWAYS, 0, 0xff);
        glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

        drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());

        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }

    if (MultiBitStencil)
    {
        glStencilFunc(GL_EQUAL, MaskStackTop-1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    }
    else if (MaskStackTop == 1)
    {
        glStencilFunc(GL_ALWAYS, 1, 0xff);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    }
}


void HAL::EndMaskSubmit()
{
    Profiler.SetDrawMode(0);
    if (!Profiler.ShouldDrawMask())
        return;

    if (!checkState(HS_InDisplay|HS_DrawingMask, "EndMaskSubmit"))
        return;

    HALState &= ~HS_DrawingMask;    
    SF_ASSERT(MaskStackTop);

    glColorMask(1,1,1,1);
    glStencilFunc(GL_LEQUAL, MaskStackTop, 0xff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}


void HAL::PopMask()
{
    if (!checkState(HS_InDisplay, "PopMask"))
        return;
    if (!Profiler.ShouldDrawMask())
        return;

    SF_ASSERT(MaskStackTop);
    MaskStackTop--;

    if (MaskStack[MaskStackTop].pPrimitive->IsClipped())
    {
        // Restore viewport
        ViewRect      = MaskStack[MaskStackTop].OldViewRect;

        if (MaskStack[MaskStackTop].OldViewportValid)
            HALState |= HS_ViewValid;
        else
            HALState &= ~HS_ViewValid;
        updateViewport();
    }

    if (MaskStackTop == 0)
        glDisable(GL_STENCIL_TEST);
    else
        glStencilFunc(GL_LEQUAL, MaskStackTop, 0xff);
}


void HAL::drawMaskClearRectangles(const HMatrix* matrices, UPInt count)
{
    // This operation is used to clear bounds for masks.
    // Potential issue: Since our bounds are exact, right/bottom pixels may not
    // be drawn due to HW fill rules.
    //  - This shouldn't matter if a mask is tessellated within bounds of those
    //    coordinates, since same rules are applied to those render shapes.
    //  - EdgeAA must be turned off for masks, as that would extrude the bounds.

    const ShaderObject* pso = SManager.SetFill(PrimFill_SolidColor, 0, PrimitiveBatch::DP_Batch, &ShaderData);

    unsigned drawRangeCount = 0;
    for (UPInt i = 0; i < count; i+= (UPInt)drawRangeCount)
    {
        drawRangeCount = Alg::Min<unsigned>((unsigned)count, MeshCache::MaxEraseBatchCount);

        for (unsigned j = 0; j < drawRangeCount; j++)
        {
            ShaderData.SetMatrix(pso, Uniform::SU_mvp, Matrix2F::Identity, matrices[i+j], Matrices, 0, j);
        }
        ShaderData.Finish(drawRangeCount);

        SetVertexArray(pso, &VertexXY16iInstance::Format, Cache.MaskEraseBatchVertexBuffer, 0);
        glDrawArrays(GL_TRIANGLES, 0, drawRangeCount * 6);

        AccumulatedStats.Meshes += drawRangeCount;
        AccumulatedStats.Triangles += drawRangeCount * 2;
        AccumulatedStats.Primitives++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}



//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

void HAL::PushBlendMode(BlendMode mode)
{
    if (!checkState(HS_InDisplay, "PushBlendMode"))
        return;

    BlendModeStack.PushBack(mode);
    applyBlendMode(mode);
}

void HAL::PopBlendMode()
{
    if (!checkState(HS_InDisplay, "PopBlendMode"))
        return;
    
    UPInt stackSize = BlendModeStack.GetSize();
    SF_ASSERT(stackSize != 0);
    BlendModeStack.PopBack();
    applyBlendMode((stackSize>1) ? BlendModeStack[stackSize-2] : Blend_Normal);
}


// Structure describing color combines applied for a given blend mode.
struct BlendModeDesc
{
    GLenum op, src, dest;
};

struct BlendModeDescAlpha
{
    GLenum op, srcc, srca, destc, desta;
};

void HAL::applyBlendMode(BlendMode mode)
{    
    bool sourceAc = false, forceAc = false;

    static BlendModeDesc modes[15] =
    {
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // None
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Normal
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Layer

        { GL_FUNC_ADD,              GL_DST_COLOR,           GL_ZERO                 }, // Multiply
        // (For multiply, should src be pre-multiplied by its inverse alpha?)

        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Screen *??

        { GL_MAX,                   GL_SRC_ALPHA,           GL_ONE                  }, // Lighten
        { GL_MIN,                   GL_SRC_ALPHA,           GL_ONE                  }, // Darken

        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Difference *??

        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE                  }, // Add
        { GL_FUNC_REVERSE_SUBTRACT, GL_SRC_ALPHA,           GL_ONE                  }, // Subtract

        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Invert *??

        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Alpha *??
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Erase *??
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // Overlay *??
        { GL_FUNC_ADD,              GL_SRC_ALPHA,           GL_ONE_MINUS_SRC_ALPHA  }, // HardLight *??
    };

    // Blending into alpha textures with premultiplied colors
    static BlendModeDescAlpha acmodes[15] =
    {
        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // None
        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Normal
        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Layer

        { GL_FUNC_ADD,              GL_DST_COLOR,  GL_DST_ALPHA,  GL_ZERO,                GL_ZERO                 }, // Multiply

        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Screen *??

        { GL_MAX,                   GL_SRC_ALPHA,  GL_SRC_ALPHA,  GL_ONE,                 GL_ONE                  }, // Lighten *??
        { GL_MIN,                   GL_SRC_ALPHA,  GL_SRC_ALPHA,  GL_ONE,                 GL_ONE                  }, // Darken *??

        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Difference

        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ZERO,       GL_ONE,                 GL_ONE                  }, // Add
        { GL_FUNC_REVERSE_SUBTRACT, GL_SRC_ALPHA,  GL_ZERO,       GL_ONE,                 GL_ONE                  }, // Subtract

        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Invert *??

        { GL_FUNC_ADD,              GL_ZERO,       GL_ZERO,       GL_ONE,                 GL_ONE                  }, // Alpha *??
        { GL_FUNC_ADD,              GL_ZERO,       GL_ZERO,       GL_ONE,                 GL_ONE                  }, // Erase *??
        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // Overlay *??
        { GL_FUNC_ADD,              GL_SRC_ALPHA,  GL_ONE,        GL_ONE_MINUS_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA  }, // HardLight *??
    };

    // For debug build
    SF_ASSERT(((unsigned) mode) < 15);
    // For release
    if (((unsigned) mode) >= 15)
        mode = Blend_None;

    mode = Profiler.GetBlendMode(mode);

	// Multiply requires different fill mode, save it in the HAL's fill flags.
	if ( mode == Blend_Multiply )
		FillFlags |= FF_Multiply;
	else
		FillFlags &= ~FF_Multiply;

    if (RenderMode & Viewport::View_AlphaComposite || forceAc)
    {
        BlendModeDescAlpha ms = acmodes[mode];
        if (sourceAc && ms.srcc == GL_SRC_ALPHA)
            ms.srcc = GL_ONE;
        glBlendFuncSeparate(ms.srcc, ms.destc, ms.srca, ms.desta);
        glBlendEquation(ms.op);
    }
    else
    {
        BlendModeDesc ms = modes[mode];
        if (sourceAc && ms.src == GL_SRC_ALPHA)
            ms.src = GL_ONE;
        glBlendFunc(ms.src, ms.dest);
        glBlendEquation(ms.op);
    }
}



//--------------------------------------------------------------------

// Draws a RenerQueueItem by placing it into a queue.

void HAL::Draw(const RenderQueueItem& item)
{    
    if (!checkState(HS_InDisplay, "Draw"))
        return;

    RenderQueueItem* pitem = Queue.ReserveHead();
    if (!pitem)
    {
        QueueProcessor.ProcessQueue(RenderQueueProcessor::QPM_One);
        pitem = Queue.ReserveHead();
        SF_ASSERT(pitem);
    }

    // We can add our primitive.
    // On Consoles, this may end up building up multiple primitives since cache
    // eviction will not rely on lock but will wait instead.
    *pitem = item;    
    Queue.AdvanceHead();

    // Process as many items as possible.
    QueueProcessor.ProcessQueue(RenderQueueProcessor::QPM_Any);
}


}}} // Scaleform::Render::GL

