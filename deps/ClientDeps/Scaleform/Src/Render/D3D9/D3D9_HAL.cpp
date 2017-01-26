/**********************************************************************

Filename    :   D3D9_HAL.cpp
Content     :   D3D9 Renderer HAL Prototype implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Kernel/SF_Debug.h"
#include "Kernel/SF_Random.h"
#include "D3D9_HAL.h"
#include "Kernel/SF_HeapNew.h"
#include <d3dx9.h>

#include <stdio.h>
#include <xutility>

namespace Scaleform { namespace Render { namespace D3D9 {

static PixMarker MaskMarker(0, false);


// ***** HAL_D3D9

HAL::HAL()
:   pDevice(0),
    HALState(0),
    VMCFlags(0),
    hWnd(0),
    RenderMode(0),
    Cache(Memory::GetGlobalHeap(), MeshCacheParams::PC_Defaults),
    Queue(),
    QueueProcessor(Queue, getThis()),
    PrevBatchType(PrimitiveBatch::DP_None),
    // Mask/Stencil vars
    MaskStackTop(0),
    StencilChecked(false), StencilAvailable(false), MultiBitStencil(false),
    DepthBufferAvailable(false),
    StencilOpInc(D3DSTENCILOP_REPLACE),
    ProfileBatches(false),
    FillFlags(0)

{
    pHeap = Memory::GetGlobalHeap();

//    VStreamType = VST_None;
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
                      "D3D9::HAL::%s failed - Mode Not Set", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InFrame, 0),
                      "D3D9::HAL::%s failed - Begin/EndFrame missing/failed.", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_InDisplay, HS_InFrame|HS_ModeSet),
                      "D3D9::HAL::%s failed - Begin/EndDisplay missing/failed.", funcName);

    SF_DEBUG_WARNING1(needWarning(HS_DeviceValid, 0),
                      "D3D9::HAL::%s failed - Device Lost or not valid", funcName);
    SF_DEBUG_WARNING1(needWarning(HS_DrawingMask, HS_InFrame|HS_InDisplay),
                      "D3D9::HAL::%s failed - PushMask_BeginSubmit call missing", funcName);
}

#endif


PrimitiveFill*  HAL::CreatePrimitiveFill(const PrimitiveFillData &data)
{
    return SF_HEAP_NEW(pHeap) PrimitiveFill(data);
}

void HAL::EnableProfileViews(bool showBatches)
{
    ProfileBatches = showBatches;
}


Color HAL::GetColorForBatch(const BatchColorKey& key) const
{
    Color batchColor;
    Color* batchLookupColor = BatchColorLookup.Get(key);
    if (batchLookupColor)
    {
        batchColor = *batchLookupColor;
    }
    else
    {
        // Create a new color
        // PPS: Use HSV space? Using hue may produce better color variance and has a higher probability of
        //      avoiding the same color for multiple batches.
        //      Technically, for the best color variance we should be using a spatial metric.
        batchColor.SetHSV(Alg::Random::NextRandom() / (float)SF_MAX_UINT32, 0.8f, 0.9f);
        //batchColor = Scaleform::Alg::Random::NextRandom();

        BatchColorLookup.Add(key, batchColor);
    }

    batchColor.SetAlpha(192);
    return batchColor;
}

template< class _MatrixType >
class MatrixUpdateAdapter_Meshes
{
public:
    MatrixUpdateAdapter_Meshes( const Primitive::MeshEntry* meshes, unsigned count, unsigned matricesPerMesh ) : 
      Count(count), MatricesPerMesh(matricesPerMesh), Meshes(meshes) { }

    typedef _MatrixType MatrixType;

    unsigned        GetCount() const { return Count; }
    unsigned        GetElementsPerTransform() const { return 4; } // always 4, even for 2D case.
    unsigned        GetMatricesPerMesh() const { return MatricesPerMesh; }
    const Matrix2F& GetVertexMatrix(unsigned i ) const { return Meshes[i].pMesh->VertexMatrix; }
    const HMatrix&  GetHMatrix(unsigned i) const { return Meshes[i].M; }

private:
    unsigned Count;
    unsigned MatricesPerMesh;
    const Primitive::MeshEntry* Meshes;
};

template< class _MatrixType >
class MatrixUpdateAdapter_Matrices
{
public:
    MatrixUpdateAdapter_Matrices( const StrideArray<const HMatrix>& matrixArray, unsigned matricesPerMesh, const Matrix2F & vertexMatrix ) : 
      MatrixArray(matrixArray), MatricesPerMesh(matricesPerMesh), VertexMatrix(vertexMatrix) { }

    typedef _MatrixType MatrixType;

    unsigned        GetCount() const { return MatrixArray.GetSize(); }
    unsigned        GetElementsPerTransform() const { return 4; } // always 4, even for 2D case.
    unsigned        GetMatricesPerMesh() const { return MatricesPerMesh; }
    const Matrix2F& GetVertexMatrix(unsigned i ) const { SF_UNUSED(i); return VertexMatrix; }
    const HMatrix&  GetHMatrix(unsigned i) const { return MatrixArray[i]; }

private:
    // Hide warnings (this class is never assigned to).
    MatrixUpdateAdapter_Matrices & operator=( const MatrixUpdateAdapter_Matrices & k )
    { SF_ASSERT(0); return *this; }

    const StrideArray<const HMatrix>& MatrixArray;
    unsigned MatricesPerMesh;
    const Matrix2F& VertexMatrix;
};

// Draws a range of pre-cached and preprocessed primitives
void HAL::DrawProcessedPrimitive(Primitive* pprimitive,
                                       PrimitiveBatch* pstart, PrimitiveBatch *pend)
{
    PixMarker event(__FUNCTIONW__);

    if (!checkState(HS_InDisplay, "DrawProcessedPrimitive") ||
        !pprimitive->GetMeshCount() )
        return;

    SF_ASSERT(pend != 0);

    Primitive::MeshEntry* meshes = &pprimitive->Meshes[0];
    PrimitiveBatch* pbatch = pstart ? pstart : pprimitive->Batches.GetFirst();
    
    bool            shaderManagerSuccess = SManager.ApplyPrimitiveFill(pprimitive->pFill);

    // Assuption: all meshes within a batch are 2D or 3D.
    bool            has3D = pbatch->GetMeshCount() > 0 ? meshes[0].M.Has3D() : false;

    unsigned bidx = 0;
    while (pbatch != pend)
    {
        // pBatchMesh can be null in case of error, such as VB/IB lock failure.
        MeshCacheItem* pmesh = (MeshCacheItem*)pbatch->GetCacheItem();
        unsigned       meshIndex = pbatch->GetMeshIndex();

        if (pmesh)
        {
            Profiler.SetBatch((UPInt)pprimitive, bidx);

            bool primSuccess = SManager.ApplyPrimitiveBatchFill(pprimitive->pFill->GetType(), FillFlags, pbatch->pFormat, pbatch->Type);

            if (shaderManagerSuccess && (HALState & HS_ViewValid) && primSuccess)
            {
                SF_ASSERT((pbatch->Type != PrimitiveBatch::DP_Failed) &&
                          (pbatch->Type != PrimitiveBatch::DP_Virtual));

                // Draw the object with cached mesh.
                UPInt   indexOffset = pmesh->IBAllocOffset / sizeof (IndexType);

                if (pbatch->Type != PrimitiveBatch::DP_Instanced)
                {
                    if ( has3D )
                    {
                        applyMatrixConstants(MatrixUpdateAdapter_Meshes<Matrix4F>(
                            meshes + meshIndex, 
                            pbatch->GetMeshCount(), 
                            pprimitive->MatricesPerMesh));
                    }
                    else
                    {
                        applyMatrixConstants(MatrixUpdateAdapter_Meshes<Matrix2F>(
                            meshes + meshIndex, 
                            pbatch->GetMeshCount(), 
                            pprimitive->MatricesPerMesh));
                    }

                    // Setup for single/batch rendering.
                    setLinearStreamSource(pbatch->Type);

                    AccumulatedStats.Meshes += pmesh->MeshCount;
                    AccumulatedStats.Triangles += pmesh->IndexCount / 3;
                }
                else
                {
                    StrideArray<const HMatrix> matrixArray(&meshes[meshIndex].M, pbatch->GetMeshCount(), sizeof(Primitive::MeshEntry));
                    if ( has3D )
                    {
                        applyMatrixConstants(MatrixUpdateAdapter_Matrices<Matrix4F>(
                            matrixArray, 
                            pprimitive->MatricesPerMesh, 
                            meshes[meshIndex].pMesh->VertexMatrix));
                    }
                    else
                    {
                        applyMatrixConstants(MatrixUpdateAdapter_Matrices<Matrix2F>(
                            matrixArray, 
                            pprimitive->MatricesPerMesh, 
                            meshes[meshIndex].pMesh->VertexMatrix));
                    }

                    setInstancedStreamSource(pbatch->GetMeshCount());
   
                    AccumulatedStats.Meshes += pbatch->GetMeshCount();
                    AccumulatedStats.Triangles += (pmesh->IndexCount / 3) * pbatch->GetMeshCount();
                }

                pDevice->SetIndices(pmesh->pIndexBuffer->GetHWBuffer());
                pDevice->SetStreamSource(0, pmesh->pVertexBuffer->GetHWBuffer(),
                                         (UINT)pmesh->VBAllocOffset, pbatch->pFormat->Size);

                // DBG
                //printf("Drawing from: pmeshItem=%p vsize=%02d vcount=%d AOffset=%d\n",
                //        pmesh, pbatch->pFormat->Size, pmesh->VBAllocSize/pbatch->pFormat->Size,
                //        pmesh->VBAllocOffset); 
                //static bool doLock = false;
                //if (doLock)
                //{
                //    if (pmesh->pVertexBuffer->DoLock())
                //    {
                //        float *data = (float*) ((UByte*)pmesh->pVertexBuffer->pData + pmesh->VBAllocOffset);

                //        pmesh->pVertexBuffer->Unlock();
                //    }
                //}

                pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
                                              0, 0, (UINT)(pmesh->VBAllocSize / pbatch->pFormat->Size),
                                              (UINT)indexOffset, pmesh->IndexCount / 3 );
     
                AccumulatedStats.Primitives++;
            }

            pmesh->MoveToCacheListFront(MCL_ThisFrame);
        }
        
        pbatch = pbatch->GetNext();
        ++bidx;
    }

    // This assert shouldn't be here in the future since queue can have many items
   // SF_ASSERT(Cache.CachedItems[MCL_InFlight].IsEmpty());
}


class VSConstantsUpdateBuffer
{
public:

    enum {
        MaxInstances = (20 < SF_RENDER_D3D9_INSTANCE_MATRICES) ?
                       20 : SF_RENDER_D3D9_INSTANCE_MATRICES,
        MaxInstanceData = SF_RENDER_D3D9_ROWS_PER_INSTANCE
    };

    VSConstantsUpdateBuffer(HAL* hal, const ProfileViews* profiler)
        : pHAL(hal), Profiler(profiler), InstanceCount(0), TextureCount(0) { }


    static const int ept = 4;

    template< class MatrixType >
    void    SetInstanceData(const StrideArray<HMatrix>& matrices,
                            unsigned startIndex,
                            unsigned count, unsigned textureCount,
                            const Matrix2F& vertexMatrix)
    {        
        InstanceCount = count;
        TextureCount  = textureCount;
       
        for (unsigned i = 0; i < InstanceCount; i++)
        {            
            const HMatrix& hm = matrices[startIndex + i];
            unsigned       k = (ept + (textureCount+1) * 2) * i;
            HAL::calculateTransform<MatrixType>(vertexMatrix, hm, pHAL->GetMatrices(), &Rows[k]);

            Profiler->GetCxform(hm.GetCxform()).GetAsFloat2x4(&Rows[k+ept]);
        }
    }

    void    SetTextureData(const Matrix2F* matrices,
                           const unsigned *indices, unsigned count)
    {
        for (unsigned i = 0; i < InstanceCount; i++)
        {
            unsigned k = (ept + (TextureCount+1) * 2) * i;

            if (count)
            {
                const Matrix2F& m0 = matrices[indices[0]];
                m0.GetAsFloat2x4(&Rows[k+ept+2]);
                if (count > 1)
                {
                    const Matrix2F& m1 = matrices[indices[1]];
                    m1.GetAsFloat2x4(&Rows[k+ept+4]);
                }
            }
        }
    }

    void    ApplyAll()
    {
        const unsigned matrixConstBase = 1; 
        pHAL->pDevice->SetVertexShaderConstantF(
            matrixConstBase + 0, 
            &Rows[0][0], (ept + 2 + 2 * TextureCount) * InstanceCount);
    }

    void    ApplyTextureData()
    {        
        if (TextureCount)
        {
            // TBD: We just skip firs set of matrices for now;
            // not sure if it's worth it calling separate updates.
            const unsigned matrixConstBase = 1;
            enum { Delta = 6 };
            pHAL->pDevice->SetVertexShaderConstantF(
                matrixConstBase + Delta, 
                &Rows[Delta][0], (ept + 2 + 2 * TextureCount) * InstanceCount - Delta);
        }
    }

private:
    HAL*       pHAL;
    const ProfileViews* Profiler;
    unsigned   InstanceCount;
    unsigned   TextureCount;
    float      Rows[MaxInstances * MaxInstanceData][4];
};



void HAL::DrawProcessedComplexMeshes(ComplexMesh* complexMesh,
                                           const StrideArray<HMatrix>& matrices)
{    
    PixMarker event(__FUNCTIONW__);

    typedef ComplexMesh::FillRecord   FillRecord;
    typedef PrimitiveBatch::BatchType BatchType;
    
    MeshCacheItem* pmesh = (MeshCacheItem*)complexMesh->GetCacheItem();
    if (!checkState(HS_InDisplay, "DrawProcessedComplexMeshes") || !pmesh)
        return;

    const FillRecord* fillRecords = complexMesh->GetFillRecords();
    unsigned    fillCount     = complexMesh->GetFillRecordCount();
    unsigned    instanceCount = (unsigned)matrices.GetSize();
    unsigned    indexBufferOffset = (unsigned)(pmesh->IBAllocOffset / sizeof(IndexType));
    unsigned    vertexSize      = 0;     // 0 => VertexBuffer wasn't applied yet.
    unsigned    vertexBaseIndex = 0;
    UByte       prevTextureCount= 0xFF;  // 0xFF => vsConstants.SetInstanceData not yet called.
    unsigned    formatIndex;
    BatchType   batchType;
    bool        has3D = instanceCount >= 0 ? matrices[0].Has3D() : false;

    if (instanceCount <= 1)
    {
        batchType = PrimitiveBatch::DP_Single;
        formatIndex = 0;
        setLinearStreamSource(PrimitiveBatch::DP_Single);
    }
    else
    {
        batchType = PrimitiveBatch::DP_Instanced;
        formatIndex = 1;
        // If (instanceCount >= MaxInstanced), we might pass an argument out of HW range here;
        // however,  that case is fixed by SetStreamSourceFreq below.
        setInstancedStreamSource(instanceCount);
    }

    pDevice->SetIndices(pmesh->pIndexBuffer->GetHWBuffer());

    VSConstantsUpdateBuffer vsConstants(this,&Profiler);

    for (unsigned fillIndex = 0; fillIndex < fillCount; fillIndex++)
    {
        const FillRecord& fr = fillRecords[fillIndex];

        Profiler.SetBatch((unsigned)complexMesh, fillIndex);

        // Apply fill.
        if (!SManager.ApplyPrimitiveFill(fr.pFill) ||
            !SManager.ApplyPrimitiveBatchFill(fr.pFill->GetType(), FillFlags, fr.pFormats[formatIndex], batchType))
            continue;

        // Modify stream source only when VertexSize changes.
        if (fr.pFormats[formatIndex]->Size != vertexSize)
        {
            vertexSize      = fr.pFormats[formatIndex]->Size;
            vertexBaseIndex = 0;
            pDevice->SetStreamSource(0, pmesh->pVertexBuffer->GetHWBuffer(),
                                     (UINT)(fr.VertexByteOffset + pmesh->VBAllocOffset),
                                     vertexSize);
        }
        else
        {
            // Identical size vertex meshes should be allocated consecutively,
            // as this is needed for SetStreamSource optimization to work.
            SF_ASSERT(fr.VertexByteOffset == (fillRecords[fillIndex-1].VertexByteOffset +
                      vertexSize * fillRecords[fillIndex-1].VertexCount));
        }

        UByte textureCount = fr.pFill->GetTextureCount();

        // Use (instanceCount-1) to force (instanceCount == 0) into 'else',
        // where it'll get skipped.
        if ((instanceCount-1) < VSConstantsUpdateBuffer::MaxInstances)
        {
            // Optimization: Instance matrices/cxform in buffer don't need to
            // be updated more then once if textureCount hasn't changed.
            if (textureCount != prevTextureCount)
            {
                if ( has3D )
                {
                    vsConstants.SetInstanceData<Matrix4F>(matrices, 0,
                        instanceCount, textureCount,
                        complexMesh->GetVertexMatrix());
                }
                else
                {
                    vsConstants.SetInstanceData<Matrix2F>(matrices, 0,
                        instanceCount, textureCount,
                        complexMesh->GetVertexMatrix());
                }
                if (textureCount)
                    vsConstants.SetTextureData(complexMesh->GetFillMatrixCache(),
                                               fr.FillMatrixIndex, textureCount);
                vsConstants.ApplyAll();
                prevTextureCount = textureCount;
            }
            else if (textureCount)
            {
                vsConstants.SetTextureData(complexMesh->GetFillMatrixCache(),
                                           fr.FillMatrixIndex, textureCount);
                vsConstants.ApplyTextureData();
            }
         
            pDevice->DrawIndexedPrimitive(
                D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, fr.VertexCount,
                fr.IndexOffset + indexBufferOffset, fr.IndexCount / 3 );
            
            AccumulatedStats.Triangles += (fr.IndexCount / 3) * instanceCount;
            AccumulatedStats.Meshes += instanceCount;
            AccumulatedStats.Primitives++;
        }
        else
        {
            // Number of instances doesn't fit one buffer, so we must iterate in blocks,
            // updating instance count every time.
            unsigned instanceLeft = instanceCount;
            unsigned instanceIndex, drawCount;

            for (instanceIndex = 0; instanceLeft > 0;
                 instanceIndex += drawCount, instanceLeft-= drawCount)
            {
                drawCount = instanceLeft;
                if (drawCount > VSConstantsUpdateBuffer::MaxInstances)
                    drawCount = VSConstantsUpdateBuffer::MaxInstances;

                if ( has3D )
                {
                    vsConstants.SetInstanceData<Matrix4F>(matrices, instanceIndex,
                        drawCount, textureCount,
                        complexMesh->GetVertexMatrix());
                }
                else
                {
                    vsConstants.SetInstanceData<Matrix2F>(matrices, instanceIndex,
                        drawCount, textureCount,
                        complexMesh->GetVertexMatrix());
                }

                if (textureCount)
                    vsConstants.SetTextureData(complexMesh->GetFillMatrixCache(),
                                               fr.FillMatrixIndex, textureCount);
                vsConstants.ApplyAll();
                
                pDevice->SetStreamSourceFreq(0, D3DSTREAMSOURCE_INDEXEDDATA | drawCount);
                pDevice->DrawIndexedPrimitive(
                    D3DPT_TRIANGLELIST, (INT)vertexBaseIndex, 0, fr.VertexCount,
                    fr.IndexOffset + indexBufferOffset, fr.IndexCount / 3 );
                
                AccumulatedStats.Triangles += (fr.IndexCount / 3) * drawCount;
                AccumulatedStats.Meshes += drawCount;
                AccumulatedStats.Primitives++;
            }
        }

        vertexBaseIndex += fr.VertexCount;

    } // for (fill record)
  
    pmesh->MoveToCacheListFront(MCL_ThisFrame);
}

template< class MatrixUpdateAdapter >
void HAL::applyMatrixConstants(const MatrixUpdateAdapter & input )
{
    const unsigned count           = input.GetCount();
    const unsigned ept             = input.GetElementsPerTransform();
    const unsigned matricesPerMesh = input.GetMatricesPerMesh();
    float          rows[SF_RENDER_D3D9_INSTANCE_DATAROWS][4];

    unsigned i,j,k;
    for (i = 0, j = 0; i < count; i++, j+=matricesPerMesh)
    {
        const HMatrix &hm           = input.GetHMatrix(i);
        const Matrix2F vertexMatrix = input.GetVertexMatrix(i);

        k = i * (2*matricesPerMesh + ept);
        calculateTransform<MatrixUpdateAdapter::MatrixType>(vertexMatrix, hm, Matrices, &rows[k]);

        if (ProfileBatches)
        {
            Color& color = CurrentBatchColor;
            rows[k+ept][0] = 0; rows[k+ept][1] = 0; rows[k+ept][2] = 0; rows[k+ept][3] = 0;
            rows[k+ept+1][0] = color.GetRed()/255.f;    rows[k+ept+1][1] = color.GetGreen()/255.f;
            rows[k+ept+1][2] = color.GetBlue()/255.f;   rows[k+ept+1][3] = color.GetAlpha()/255.f;
        }
        else
            Profiler.GetCxform(hm.GetCxform()).GetAsFloat2x4(&rows[k+ept]);

        if (matricesPerMesh > 1)
        {
            // TBD: Do texture coordinates need to be adjusted
            //      by the VertexMatrix? What else?
            // Old renderer just passed the values through...
            Matrix  tm0(vertexMatrix, hm.GetTextureMatrix(0));
            tm0.GetAsFloat2x4(&rows[k+ept+2]);

            if (matricesPerMesh > 2)
            {
                Matrix tm1(vertexMatrix, hm.GetTextureMatrix(1));
                tm1.GetAsFloat2x4(&rows[k+ept+4]);
            }
        }
    }

    const unsigned matrixConstBase = 1;
    pDevice->SetVertexShaderConstantF(matrixConstBase + 0,
        &rows[0][0], (ept + 2*matricesPerMesh) * count);
}

// Explicit instantiation for 2D matrices.
template<>
void HAL::calculateTransform<Matrix2F>(const Matrix & m, const HMatrix& hm, const MatrixState & mstate, float (* dest)[4])
{
    // Get as 2D, and just set the remainder to identity.
    Matrix4F mtx(Matrix2F(m, hm.GetMatrix2D(), mstate.ViewportMatrix));
    mtx.GetAsFloat4x4(dest);
}

// Explicit instantiation for 4D matrices.
template<>
void HAL::calculateTransform<Matrix4F>(const Matrix & m, const HMatrix& hm, const MatrixState & mstate, float (* dest)[4])
{
    Matrix4F m4d = mstate.GetUVP() * hm.GetMatrix3D() * m;
    m4d.GetAsFloat4x4(dest);
}

void HAL::applyRawMatrixConstants(const Matrix& m, const Cxform& cx)
{
    float   rows[8][4];
    Matrix4F  mat(Matrix2F(m, Matrices.ViewportMatrix));

    mat.GetAsFloat4x4(&rows[0]);
    cx.GetAsFloat2x4(&rows[4]);

    const unsigned matrixConstBase = 1;
    pDevice->SetVertexShaderConstantF(matrixConstBase + 0, &rows[0][0], 6);
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


// Defines all D3Dx color channels for D3DRS_COLORWRITEENABLE.
#define D3DCOLORWRITEENABLE_ALL D3DCOLORWRITEENABLE_ALPHA | D3DCOLORWRITEENABLE_RED | \
                                D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_GREEN

void HAL::PushMask_BeginSubmit(MaskPrimitive* prim)
{
    MaskMarker.Begin(__FUNCTIONW__);

    if (!checkState(HS_InDisplay, "PushMask_BeginSubmit"))
        return;

    Profiler.SetDrawMode(1);
    if (!Profiler.ShouldDrawMask())
        return;

    if (!StencilAvailable && !DepthBufferAvailable)
    {
        if (!checkMaskBufferCaps())
            return;
    }
    
    // These states are applicable to most stencil operations below.
    if (StencilAvailable)
    {
        pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
        pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
        pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
    }

    bool viewportValid = (HALState & HS_ViewValid) != 0;

    // Erase previous mask if it existed above our current stack top.
    if (MaskStackTop && (MaskStack.GetSize() > MaskStackTop) && viewportValid)
    {
        // Erase rectangles of these matrices; must be done even for clipped masks.
        if (StencilAvailable && (MultiBitStencil && (StencilOpInc != D3DSTENCILOP_REPLACE)))
        {
            // Any stencil of value greater then MaskStackTop should be set to it;
            // i.e. replace when (MaskStackTop < stencil value).
            DWORD maxStencilValue = (DWORD)MaskStackTop;
            pDevice->SetRenderState(D3DRS_STENCILREF, maxStencilValue);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

            MaskPrimitive* erasePrim = MaskStack[MaskStackTop].pPrimitive;
            drawMaskClearRectangles(erasePrim->GetMaskAreaMatrices(), erasePrim->GetMaskCount());
        }
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
        // Apply new viewport clipping.
        const Matrix2F &m = prim->GetMaskAreaMatrix(0).GetMatrix2D();
        // Clipped matrices are always in View coordinate space, to allow
        // matrix to be use for erase operation above. This means that we don't
        // have to do an EncloseTransform.
        SF_ASSERT((m.Shx() == 0.0f) && (m.Shy() == 0.0f));
        Rect<int>  boundClip((int)m.Tx(), (int)m.Ty(),
                             (int)(m.Tx() + m.Sx()), (int)(m.Ty() + m.Sy()));
        boundClip.Offset(VP.Left, VP.Top);
        
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
            if (StencilAvailable)
                pDevice->Clear(0, 0, D3DCLEAR_STENCIL, 0, 0.0f, 0);
            else if (DepthBufferAvailable)
                pDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0, 1.0f, 0); // Z -> 1.0f
        }
    }

    else if ((MaskStackTop == 1) && viewportValid)
    {
        // Clear view rectangles.
        if (StencilAvailable)
        {
            // Unconditionally overwrite stencil rectangles with REF value of 0.
            pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_REPLACE);
            pDevice->SetRenderState(D3DRS_STENCILREF, 0);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);

            drawMaskClearRectangles(prim->GetMaskAreaMatrices(), prim->GetMaskCount());
            pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
        }
        else
        {
            // Depth clears bounds. Don't use drawMaskClearRectangles yet as it doesn't
            // set proper Z.
            UPInt maskCount = prim->GetMaskCount();
            for (UPInt i = 0; i< maskCount; i++)
            {
                const Matrix2F &m = prim->GetMaskAreaMatrix(i).GetMatrix2D();
                RectF           bounds(m.EncloseTransform(RectF(1.0f)));
                Rect<int>       boundClip((int)bounds.x1, (int)bounds.y1,
                                          (int)bounds.x2, (int)bounds.y2);
                boundClip.Offset(VP.Left, VP.Top);

                if (boundClip.IntersectRect(&boundClip, ViewRect))
                {
                    D3DRECT r = { boundClip.x1, boundClip.y1, boundClip.x2, boundClip.y2 };
                    pDevice->Clear(1, &r, D3DCLEAR_ZBUFFER, 0, 1.0f, 0); // Z -> 1.0f
                }
            }
        }
    }


    if (StencilAvailable)
    {
        bool canIncDec = (MultiBitStencil && (StencilOpInc != D3DSTENCILOP_REPLACE));

        // Increment only if we support it.
        if (canIncDec)
        {
            pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)(MaskStackTop-1));
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
            pDevice->SetRenderState(D3DRS_STENCILPASS, StencilOpInc);
        }
        else if (MaskStackTop == 1)
        {
            pDevice->SetRenderState(D3DRS_STENCILREF, 1);
            pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
        }
        else
        {   // If not supported, no change.
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        }
    }
    else if (DepthBufferAvailable)
    {
        if (MaskStackTop == 1)
        {
            // Set the correct render states in order to not modify the color buffer
            // but write the default Z-value everywhere. According to the shader code: should be 0.
            pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA);
            pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
            pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
            pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
        }
        else
        {
            // No color write. Incr/Decr not supported.
            pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
            pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
            pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        }
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

    pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALL);
    
    if (StencilAvailable)
    {
        // We draw only where the (MaskStackTop <= stencil), i.e. where the latest mask was drawn.
        // However, we don't change the stencil buffer
        pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
        pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP); 
        // Stencil counter.
        pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)MaskStackTop);
        pDevice->SetRenderState(D3DRS_STENCILMASK, 0xFF);
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write and write only where the mask had written
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
        pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_EQUAL);
    }
    MaskMarker.End();
}


void HAL::PopMask()
{
    PixMarker event(__FUNCTIONW__);

    if (!Profiler.ShouldDrawMask())
        return;

    if (!checkState(HS_InDisplay, "PopMask"))
        return;

    if (!StencilAvailable && !DepthBufferAvailable)
        return;
    SF_ASSERT(MaskStackTop);
    MaskStackTop--;

    if (MaskStack[MaskStackTop].pPrimitive->IsClipped())
    {
        // Restore viewport
        ViewRect = MaskStack[MaskStackTop].OldViewRect;

        if (MaskStack[MaskStackTop].OldViewportValid)
            HALState |= HS_ViewValid;
        else
            HALState &= ~HS_ViewValid;
        updateViewport();
    }

    // Disable mask or decrement stencil reference value.
    if (StencilAvailable)
    {
        if (MaskStackTop == 0)
            pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        else
        {
            // Change ref value down, so that we can draw using previous mask.
            pDevice->SetRenderState(D3DRS_STENCILREF, (DWORD)MaskStackTop);
            pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);
        }
    }
    else if (DepthBufferAvailable)
    {
        // Disable the Z-write and write only where the mask had written
        pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    }
}


bool HAL::checkMaskBufferCaps()
{
    if (!StencilChecked)
    {
        // Test for depth-stencil presence.
        IDirect3DSurfaceX *pdepthStencilSurface = 0;
        pDevice->GetDepthStencilSurface(&pdepthStencilSurface);
        if (pdepthStencilSurface)
        {
            D3DSURFACE_DESC sd;
            pdepthStencilSurface->GetDesc(&sd);

            switch(sd.Format)
            {
            case D3DFMT_D24S8:
            case D3DFMT_D24X4S4:
//#if (SF_D3D_VERSION == 9)
            case D3DFMT_D24FS8:
//#endif
                MultiBitStencil = 1;
            case D3DFMT_D15S1:
                StencilAvailable = 1;
                break;
            }

            pdepthStencilSurface->Release();
            pdepthStencilSurface = 0;
            DepthBufferAvailable = 1;
        }
        else
            StencilAvailable = 0;

        StencilChecked = 1;
    }

    if (!StencilAvailable && !DepthBufferAvailable)
    {
#ifdef SF_BUILD_DEBUG
        static bool StencilWarned = 0;
        if (!StencilWarned)
        {
            SF_DEBUG_WARNING(1, "RendererHAL::PushMask_BeginSubmit used, but stencil is not available");
            StencilWarned = 1;
        }
#endif
        return false;
    }
    return true;
}


void HAL::drawMaskClearRectangles(const HMatrix* matrices, UPInt count)
{
    PixMarker event(__FUNCTIONW__);
    if (!SManager.ApplyClearMaskFill())
    {
        return;
    }

    // This operation is used to clear bounds for masks.
    // Potential issue: Since our bounds are exact, right/bottom pixels may not
    // be drawn due to HW fill rules.
    //  - This shouldn't matter if a mask is tessellated within bounds of those
    //    coordinates, since same rules are applied to those render shapes.
    //  - EdgeAA must be turned off for masks, as that would extrude the bounds.

    setLinearStreamSource(PrimitiveBatch::DP_Batch);
    pDevice->SetStreamSource(0, Cache.pMaskEraseBatchVertexBuffer.GetPtr(),
                             0, sizeof(VertexXY16iAlpha));

    // Draw the object with cached mesh.
    unsigned drawRangeCount = 0;
    for (UPInt i = 0; i < count; i+= (UPInt)drawRangeCount)
    {
        drawRangeCount = Alg::Min<unsigned>((unsigned)count, MeshCache::MaxEraseBatchCount);
        StrideArray<const HMatrix> argMatrices(matrices + i, drawRangeCount, sizeof(HMatrix));
        applyMatrixConstants(MatrixUpdateAdapter_Matrices<Matrix2F>(argMatrices, drawRangeCount, Matrix2F::Identity));
        pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, (UINT)drawRangeCount * 2);

        AccumulatedStats.Meshes += drawRangeCount;
        AccumulatedStats.Triangles += drawRangeCount * 2;
        AccumulatedStats.Primitives++;
    }
}

//--------------------------------------------------------------------
// Background clear helper, expects viewport coordinates.
void HAL::clearSolidRectangle(const Rect<int>& r, Color color)
{
    color = Profiler.GetClearColor(color);

    PixMarker event(__FUNCTIONW__);
    if (color.GetAlpha() == 0xFF)
    {
        // Do more efficient HW clear. Device::Clear expects render-target coordinates.
        D3DRECT  cr = { r.x1 + VP.Left, r.y1 + VP.Top,
                        r.x2 + VP.Left, r.y2 + VP.Top };
        D3DCOLOR d3dc = D3DCOLOR_XRGB(color.GetRed(), color.GetGreen(), color.GetBlue());
        pDevice->Clear(1, &cr, D3DCLEAR_TARGET, d3dc, 1.0f,  0);
    }
    else
    {
        if (!SManager.ApplySolidColorFill(color))
            return;

        Matrix2F m((float)r.Width(), 0.0f, (float)r.x1,
                   0.0f, (float)r.Height(), (float)r.y1);
        applyRawMatrixConstants(m, Cxform::Identity);
        setLinearStreamSource(PrimitiveBatch::DP_Batch);
        pDevice->SetStreamSource(0, Cache.pMaskEraseBatchVertexBuffer.GetPtr(),
                                 0, sizeof(VertexXY16iAlpha));

        pDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
    }
}



//--------------------------------------------------------------------
// *** BlendMode Stack support
//--------------------------------------------------------------------

void HAL::PushBlendMode(BlendMode mode)
{
    PixMarker event(__FUNCTIONW__);
    if (!checkState(HS_InDisplay, "PushBlendMode"))
        return;

    BlendModeStack.PushBack(mode);
    applyBlendMode(mode);
}

void HAL::PopBlendMode()
{
    PixMarker event(__FUNCTIONW__);
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
    D3DBLENDOP  BlendOp;
    D3DBLEND    SrcArg, DestArg;
};

struct BlendModeDescAlpha
{
    D3DBLENDOP  BlendOp;
    D3DBLEND    SrcArg, DestArg;
    D3DBLEND    SrcAlphaArg, DestAlphaArg;
};

void HAL::applyBlendMode(BlendMode mode)
{    

    static BlendModeDesc modes[15] =
    {
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // None
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Normal
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Layer

        { D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO },          // Multiply
        // (For multiply, should src be pre-multiplied by its inverse alpha?)

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Screen *??

        { D3DBLENDOP_MAX, D3DBLEND_SRCALPHA, D3DBLEND_ONE },            // Lighten
        { D3DBLENDOP_MIN, D3DBLEND_SRCALPHA, D3DBLEND_ONE },                    // Darken

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Difference *??

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ONE },            // Add
        { D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_ONE },    // Subtract

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Invert *??

        { D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE },                // Alpha *??
        { D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ONE },                // Erase *??  What color do we erase to?
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // Overlay *??
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA },    // HardLight *??
    };

    static BlendModeDescAlpha acmodes[15] =
    {
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // None
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Normal
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Layer

        { D3DBLENDOP_ADD, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO, D3DBLEND_DESTCOLOR, D3DBLEND_ZERO },      // Multiply
        // (For multiply, should src be pre-multiplied by its inverse alpha?)

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Screen *??

        { D3DBLENDOP_MAX, D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_SRCALPHA, D3DBLEND_ONE },          // Lighten
        { D3DBLENDOP_MIN, D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_SRCALPHA, D3DBLEND_ONE },          // Darken

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Difference *??

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLEND_ONE },                   // Add
        { D3DBLENDOP_REVSUBTRACT, D3DBLEND_SRCALPHA, D3DBLEND_ONE, D3DBLEND_ZERO, D3DBLEND_ONE },           // Subtract

        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Invert *??

        { D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO, D3DBLEND_ONE, D3DBLEND_ONE               },    // Alpha *??
        { D3DBLENDOP_ADD, D3DBLEND_ZERO, D3DBLEND_ZERO, D3DBLEND_ONE, D3DBLEND_ONE               },    // Erase *??
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Overlay *??
        { D3DBLENDOP_ADD, D3DBLEND_SRCALPHA, D3DBLEND_INVSRCALPHA, D3DBLEND_ONE, D3DBLEND_INVSRCALPHA },    // Hardlight *??
    };

    if (!pDevice)
        return;

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

    //#if (SF_D3D_VERSION == 9)
    //if (UseAcBlend) // Separate alpha blend only for now.
    {
        if (RenderMode & Viewport::View_AlphaComposite)
        {
            pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
            pDevice->SetRenderState(D3DRS_BLENDOP, acmodes[mode].BlendOp);
            pDevice->SetRenderState(D3DRS_BLENDOPALPHA, acmodes[mode].BlendOp);
            pDevice->SetRenderState(D3DRS_SRCBLEND, acmodes[mode].SrcArg);        
            pDevice->SetRenderState(D3DRS_DESTBLEND, acmodes[mode].DestArg);
            pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, acmodes[mode].SrcAlphaArg);
            pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, acmodes[mode].DestAlphaArg);
        }
        else
        {
            pDevice->SetRenderState(D3DRS_BLENDOP, modes[mode].BlendOp);
            pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
            pDevice->SetRenderState(D3DRS_SRCBLEND, modes[mode].SrcArg);        
            pDevice->SetRenderState(D3DRS_DESTBLEND, modes[mode].DestArg);
        }
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


}}} // Scaleform::Render::D3D9

