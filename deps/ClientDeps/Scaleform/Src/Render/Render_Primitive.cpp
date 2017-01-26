/**********************************************************************

Filename    :   Primitive.cpp
Content     :   Renderer HAL Mesh and Primitive objects implementation.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Render_Primitive.h"
#include "Render_HAL.h"
#include "Render_MeshCache.h"
#include "Render_Queue.h"
#include "Kernel/SF_Debug.h"
#include "Kernel/SF_HeapNew.h"

//#include <stdio.h> // printf

namespace Scaleform { namespace Render {


// ***** Mesh Implementation

Mesh::Mesh(Renderer2DImpl* prenderer, MeshProvider *provider,
           const Matrix2F& viewMatrix, float morphRatio, 
           unsigned layer, unsigned meshGenFlags)
: MeshBase(prenderer, provider, viewMatrix, morphRatio, layer, meshGenFlags), LargeMesh(false)
{
}

Mesh::~Mesh()
{
 //   if (Data.pInterface)
 //       Data.pInterface->ReleaseMeshData(Data.hMesh);

    // Go through CacheItems and release them.
    UPInt           count = CacheItems.GetSize();
    MeshCacheItem** pdata = CacheItems.GetBuffer();
    while(count--)
    {
        if (*pdata)
            (*pdata)->NotifyMeshRelease(this);
        pdata++;
    }

    // Remove us from the staging buffer.
    if (StagingBufferSize)
    {
        SF_ASSERT(PinCount == 0);
        RemoveNode();
    }
}

MeshUseStatus Mesh::GetUseStatus() const
{
    MeshUseStatus   status = MUS_Uncached;
    UPInt           count = CacheItems.GetSize();
    MeshCacheItem* const * pdata = CacheItems.GetBuffer();
    for(UPInt i = 0; i < count; i++)
    {
        MeshUseStatus s = pdata[i]->GetUseStatus();
        if (s > status)
            status = s;
    }
    return status;
}




// ***** PrimitiveFillData

// Check fillType dependencies
const char* PrimitiveFill_FormatCheck[PrimFill_Type_Count] = 
{
    "",     // PrimFill_None,
    "",     // PrimFill_SolidColor,                { XY }
    "",     // PrimFill_Mask,                      { XY }
    "C",    // PrimFill_VColor,                    { XY, Color }
    "CE",   // PrimFill_VColor_EAlpha,             { XY, Color, EAlpha }
    "",     // PrimFill_Texture,                   { XY }
    "E",    // PrimFill_Texture_EAlpha,            { XY, EAlpha }
    "CW",   // PrimFill_Texture_VColor,            { XY, Color, Weight1 }
    "CWE",  // PrimFill_Texture_VColor_EAlpha,     { XY, Color, Weight1, EAlpha }
    "W",    // PrimFill_2Texture,                  { XY, Weight1 }
    "EW",   // PrimFill_2Texture_EAlpha,           { XY, Weight1, EAlpha  }
    "U",    // PrimFill_UVTexture,                 { XY, UV }
    "UC"    // PrimFill_UVTextureAlpha_VColor,     { XY, UV, Color }
};

bool    PrimitiveFillData::CheckVertexFormat(PrimitiveFillType fill, const VertexFormat* format)
{        
    if (!format->HasUsage(VET_Pos))
    {
        SF_DEBUG_ERROR(1, "D3D9::ShaderManager - VertexFormat missing Pos attribute");
        return false;
    }

    const char* pattrs;   
    for (pattrs = PrimitiveFill_FormatCheck[fill]; *pattrs; pattrs++)
    {
        const char* notFound = 0;

        switch(*pattrs)
        {
        case 'C':
            if (!format->HasUsage(VET_Color))
                notFound = "VColor";
            break;
        case 'E':
            if (!format->GetElement(VET_FactorAlpha8, VET_Type_Mask))
                notFound = "FactorAlpha8";
            break;
        case 'W':
            if (!format->GetElement(VET_T0Weight8, VET_Type_Mask))
                notFound = "T0Weight8";
            break;
        case 'U':
            if (!format->HasUsage(VET_TexCoord))
                notFound = "TexCoord";
            break;
        }
        if (notFound)
        {
            SF_DEBUG_ERROR1(1, 
                "D3D9::ShaderManager - VertexFormat missing %s attribute", notFound);
            return false;
        }
    }
    return true;
}



// ***** PrimitiveFill 

PrimitiveFill::~PrimitiveFill()
{
    if (pManager)
        pManager->removeFill(this);
}

void PrimitiveFillManager::removeFill(PrimitiveFill* fill)
{
    FillSet.Remove(fill);
}

void PrimitiveFillManager::removeGradient(GradientImage* img)
{
    //Gradients.Remove(GradientKey(img->GetGradientData()));
    Gradients.Remove(img);
}
// Creates and/or registers a gradient image; caller must store reference.
Image*  PrimitiveFillManager::createGradientImage(GradientData* data)
{
    GradientImage* found = 0;
    if (Gradients.GetAlt(GradientKey(data), &found))
    {
        found->AddRef();
    }
    else
    {
        // TO DO: Revise Heap, SID, texArena
        found = SF_HEAP_AUTO_NEW(this) GradientImage(this, data);
        Gradients.Add(found);
    }
    return found;
}


PrimitiveFill* PrimitiveFillManager::CreateFill(const PrimitiveFillData& initdata)
{
    PrimitiveFill* fill = 0;
    
    if (FillSet.GetAlt(initdata, &fill))
        fill->AddRef();
    else
    {
        fill = pHAL->CreatePrimitiveFill(initdata);
        if (!fill)
            return 0;
        FillSet.Add(fill);
        fill->SetManager(this);
    }
    return fill;
}


PrimitiveFill* PrimitiveFillManager::CreateFill(const FillData& initdata, Ptr<Image>* gradientImg, 
                                                TextureManager* mng)
{
    switch(initdata.Type)
    {    
    case Fill_Mask:
        {
            PrimitiveFillData data(PrimFill_Mask, initdata.pVFormat);
            return CreateFill(data);
        }
        break;
    case Fill_SolidColor:
        {
            PrimitiveFillData data(PrimFill_SolidColor, initdata.pVFormat, initdata.Color);
            return CreateFill(data);
        }        
        break;

    case Fill_VColor:
    case Fill_VColor_TestKey:
        {
            PrimitiveFillData data(initdata.PrimFill, initdata.pVFormat);
            if (initdata.Type == Fill_VColor_TestKey)
            {
                // MA: We assign color to use it as a unique key for batching,
                // as needed by SortedRender2D and SortedRenderTree tests.
                data.SolidColor = initdata.Color;
            }            
            return CreateFill(data);
        }
        break;

    case Fill_Image:
        {
            PrimitiveFillData data(initdata.PrimFill,
                                   initdata.pVFormat, 
                                   initdata.pImage->GetTexture(mng), 
                                   initdata.FillMode);
            return CreateFill(data);
        }
        break;

    case Fill_Gradient:
        {
            // Assign ownership externally.
            *gradientImg = *createGradientImage(initdata.pGradient);
            if (!*gradientImg)
                break;
            PrimitiveFillData data(initdata.PrimFill,
                                   initdata.pVFormat, 
                                   (*gradientImg)->GetTexture(mng), 
                                   initdata.FillMode);
            return CreateFill(data);
        }
        break;
    }
    return 0;
}


// FillTypeMergeTable maps {fill0, fill1, mergeFlags} data to PrimitiveFillType result.
// Not all combinations are supported, so some cases will be missed.
// In particular, we expect fills to be normalized by tessellation, allowing
// only {Image, VColor} pair fill ordering and nor {VColor, Image}.
// If we choose to allow the later, we'd need to swap fills and invert Weight
// channel in vertices.

struct FillTypeMergeInfo
{
    UByte F0, F1;    
    UByte MergeFlags;
    UByte Result;
};

static FillTypeMergeInfo FillTypeMergeTable[] =
{
    // VColor
    { Fill_VColor,      Fill_None,      FillMerge_None,             PrimFill_VColor},
    { Fill_VColor,      Fill_None,      FillMerge_EAlpha,           PrimFill_VColor_EAlpha},
    // Image
    { Fill_Image,       Fill_None,      FillMerge_None,             PrimFill_Texture},
    { Fill_Image,       Fill_None,      FillMerge_EAlpha,           PrimFill_Texture_EAlpha},
    { Fill_Image,       Fill_VColor,    FillMerge_Weight,           PrimFill_Texture_VColor},
    { Fill_Image,       Fill_VColor,    FillMerge_EAlpha_Weight,    PrimFill_Texture_VColor_EAlpha},
    { Fill_Image,       Fill_Image,     FillMerge_Weight,           PrimFill_2Texture},
    { Fill_Image,       Fill_Image,     FillMerge_EAlpha_Weight,    PrimFill_2Texture_EAlpha},
    // Solid
    { Fill_SolidColor,  Fill_None,      FillMerge_None,             PrimFill_SolidColor},
    { Fill_Mask,        Fill_None,      FillMerge_None,             PrimFill_Mask},
    // End list
    { Fill_None,        Fill_None,      FillMerge_None,             PrimFill_None},
};

PrimitiveFillType GetMergedFillType(FillType ft0, FillType ft1, unsigned mergeFlags)
{
    FillTypeMergeInfo* mergeInfo = FillTypeMergeTable;

    // These types should've been masked out by caller.
    SF_ASSERT((ft0 != Fill_Gradient) && (ft0 != Fill_VColor_TestKey));
    SF_ASSERT((ft1 != Fill_Gradient) && (ft1 != Fill_VColor_TestKey));
    
    // Exhaustive search for matching target type.
    // This can be optimized if it becomes an issue.
    do {
        if ((mergeInfo->F0 == ft0) && (mergeFlags == mergeInfo->MergeFlags))
            if ((mergeInfo->F1 == ft1) || (!(mergeFlags & FillMerge_Weight)))
                break;
        mergeInfo++;
    } while(mergeInfo->F0 != Fill_None);

    return (PrimitiveFillType) mergeInfo->Result;
}


// Creates a complex merged fill built out of one or more regular fills.
PrimitiveFill* PrimitiveFillManager::CreateMergedFill(
    unsigned mergeFlags, const VertexFormat* vformat,
    const FillData* fd0, const FillData* fd1,    
    Ptr<Image>* gradientImg0, Ptr<Image>* gradientImg1, TextureManager* textureManager)
{
    bool merge = (mergeFlags & FillMerge_Weight) != 0;
    SF_UNUSED(merge); // debug only

    // Convert gradient types to image
    FillType ft0  = fd0->Type;
    FillType ft1  = fd1->Type;
    Image*   img0 = fd0->pImage; // Warning: May be invalid.
    Image*   img1 = fd1->pImage; // Warning: May be invalid.

    // Convert gradients to images.
    if (ft0 == Fill_Gradient)
    {
        *gradientImg0 = *createGradientImage(fd0->pGradient);
        img0 = *gradientImg0;
        ft0 = Fill_Image;
    }    
    if (ft1 == Fill_Gradient)
    {
        SF_ASSERT(merge);
        *gradientImg1 = *createGradientImage(fd1->pGradient);
        img1 = *gradientImg1;
        ft1 = Fill_Image;
    }
    
    // TBD: Remove Fill_VColor_TestKey once not needed
    if (ft0 == Fill_VColor_TestKey)
        ft0 = Fill_VColor;
    if (ft1 == Fill_VColor_TestKey)
        ft1 = Fill_VColor;

    PrimitiveFillType mergedFillType = GetMergedFillType(ft0, ft1, mergeFlags);
    PrimitiveFillData data(mergedFillType, vformat);

    switch(mergedFillType)
    {
        case PrimFill_Mask:
            break;
        case PrimFill_SolidColor:
            data.SolidColor = fd0->Color;
            break;

        case PrimFill_VColor:
        case PrimFill_VColor_EAlpha:            
            if (fd0->Type== Fill_VColor_TestKey)
            {
                // MA: We assign color to use it as a unique key for batching,
                // as needed by SortedRender2D and SortedRenderTree tests.
                data.SolidColor = fd0->Color;
            }
            break;

        case PrimFill_2Texture:
        case PrimFill_2Texture_EAlpha:
            data.Textures[1] = img1->GetTexture(textureManager);
            data.FillModes[1]= fd1->FillMode;
            // Fall through to set fill 0.
        case PrimFill_Texture:
        case PrimFill_Texture_VColor:
        case PrimFill_Texture_EAlpha:
        case PrimFill_Texture_VColor_EAlpha:
            data.Textures[0] = img0->GetTexture(textureManager);
            data.FillModes[0]= fd0->FillMode;
            break;

        default:
            // This shouldn't happen, as it typically means that proper 
            // merged fill type wasn't found.
            SF_DEBUG_WARNING(1, "PrimitiveFillManager::CreateMergedFill failed");
            return 0;
    }

    return CreateFill(data);
}


// TO DO: Determine vertex format in TreeCacheShape or ShapeMeshProvider
FillData::FillData(FillType type)
:   Type(type), 
    Color(0), 
    PrimFill(PrimFill_VColor_EAlpha),
    FillMode(),
    pVFormat(&VertexXY16iCF32::Format)
{
    if (type != Fill_VColor)
    {
        if (type == Fill_Mask)
        {
            PrimFill = PrimFill_Mask;
            pVFormat = &VertexXY16i::Format;
        }
        else
        {
            SF_ASSERT(1);
        }
    }
}

FillData::FillData(UInt32 color) : 
    Type(Fill_SolidColor), 
    Color(color), 
    PrimFill(PrimFill_SolidColor),
    FillMode(),
    pVFormat(&VertexXY16i::Format)
{}

FillData::FillData(Image* p, ImageFillMode fm) : 
    Type(Fill_Image), 
    pImage(p),
    PrimFill(PrimFill_Texture_EAlpha),
    FillMode(fm),
    pVFormat(&VertexXY16iCF32::Format)
{}

FillData::FillData(GradientData* p) : 
    Type(Fill_Gradient), 
    pGradient(p),
    PrimFill(PrimFill_Texture_EAlpha),
    FillMode(ImageFillMode(Wrap_Clamp, Sample_Linear)),
    pVFormat(&VertexXY16iCF32::Format)
{}


//--------------------------------------------------------------------
// ***** Primitive implementation

static unsigned Primitive_CreateCount = 0;
static unsigned Primitive_Total = 0;
static unsigned Primitive_Insert = 0;

Primitive::Primitive(HAL* phal, PrimitiveFill* pfill)
: pHAL(phal), pFill(pfill), 
  MatricesPerMesh(pfill->GetTextureCount()+1), ModifyIndex(0)
{
    Primitive_CreateCount++;
    Primitive_Total++;
//    printf("\nPrimitive create %d - total: %d", Primitive_CreateCount, Primitive_Total);
}

Primitive::~Primitive()
{
    Primitive_Total--;

    while(!Batches.IsEmpty())
        Batches.GetFirst()->RemoveAndFree();
}


bool Primitive::Insert(unsigned index, Mesh* pmesh, const HMatrix& m)
{
    SF_ASSERT(index <= GetMeshCount());

    PrimitiveBatch *pprevBatch = 0;
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    const MeshCacheParams& params = pHAL->GetMeshCache().GetParams();

    // Verify/assign vertex format. All meshes withing a primitive
    // should share the same vertex format.  
    /*
    if (pVertexFormat == 0)    
        pVertexFormat = pmesh->GetVertexFormat();
    else
    {
        SF_ASSERT(*pVertexFormat == *pmesh->GetVertexFormat());
    }   */

    while(1)
    {
        // If we are at the index, index fell between the blocks
        if (i == index)
        {
            if (pprevBatch && pprevBatch->IsVirtual())
            {
                // InsertMeshes.
                pprevBatch->MeshCount++;
                break;
            }
            // Batches.IsNull(pbatch) can hold for an empty list
            else if (Batches.IsNull(pbatch) || !pbatch->IsVirtual())
            {
                PrimitiveBatch* pnewBatch =
                    PrimitiveBatch::Create(this, PrimitiveBatch::DP_Virtual, i, 1);
                SF_ASSERT(pnewBatch);
                if (!pnewBatch)
                    return false;
                // Insert a new chunk here; this works even if Batches list is empty.
                pbatch->InsertNodeBefore(pnewBatch);
                break;
            }  
        }

        // Terminate loop after we reached the end. This check is here instead of
        // top/bottom of loop to ensure that the last item index is properly handled.
        if (Batches.IsNull(pbatch))
            break;

        // If we land within this chunk, insert us into this primitive.
        if ((index - i) < pbatch->MeshCount)
        {
            // Insert meshes.
            pbatch->MeshCount++;
            if (!pbatch->IsInstanced() || (Meshes[i].pMesh != pmesh) ||
                (pbatch->MeshCount > params.MaxBatchInstances))
            {
                pbatch->Type = PrimitiveBatch::DP_Virtual;
                pbatch->ClearCacheItem();
            }
            break;
        }

        i += pbatch->MeshCount;
        pprevBatch = pbatch;
        pbatch     = pbatch->GetNext();
    }

    // Resize arrays.
    MeshEntry e = { m, pmesh };
    Meshes.InsertAt(index, e); 
    if (index < ModifyIndex)
        ModifyIndex = index;

    Primitive_Insert++;
    //  printf(" I%d", Primitive_Insert);

    return true;
}


void Primitive::Remove(unsigned index, unsigned count)
{
    // When we remove, we can modify up to two batches and remove even more from the middle.
    SF_ASSERT((index + count) <= GetMeshCount());

    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;
    unsigned        leftToRemove = count;

    do {

        if ((index - i) < pbatch->MeshCount)
        {
            if (index == i)
            {
                if (leftToRemove >= pbatch->MeshCount)
                {
                   PrimitiveBatch* poldBatch = pbatch;

                    // Remove All.  Note that 'i' doesn't change.
                    leftToRemove -= pbatch->MeshCount;
                    pbatch = pbatch->GetNext();
                    poldBatch->RemoveAndFree();
                    continue;
                }
                else
                {
                    // RemoveMeshes() from front.
                    pbatch->MeshCount -= leftToRemove;
                    if (!pbatch->IsInstanced())
                    {
                        pbatch->Type = PrimitiveBatch::DP_Virtual;
                        pbatch->ClearCacheItem();
                    }
                    leftToRemove = 0;
                    break;
                }
            }
            else
            {
                // Remove Tail
                unsigned removeCount = (leftToRemove > (pbatch->MeshCount - (index - i))) ?
                                       pbatch->MeshCount - (index - i) : leftToRemove;

                // RemoveMeshes():
                pbatch->MeshCount -= removeCount;
                if (!pbatch->IsInstanced())
                {
                    pbatch->Type = PrimitiveBatch::DP_Virtual;
                    pbatch->ClearCacheItem();
                }

                leftToRemove -= removeCount;
                // Advance index by leftover mesh count.
                i += pbatch->MeshCount;
            }
        }
        else
        {
            i += pbatch->MeshCount;
        }
        
        pbatch = pbatch->GetNext();

    } while(leftToRemove && !Batches.IsNull(pbatch));


    // Remove from arrays.
    Meshes.RemoveMultipleAt(index, count);
    if (index < ModifyIndex)
        ModifyIndex = index;
}


void Primitive::SetMesh(unsigned index, Mesh* pmesh)
{
    if (Meshes[index].pMesh == pmesh)
        return;
    
    Meshes[index].pMesh = pmesh;
        
    // Find PrimitiveBatch and mark it as Virtual.
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    while(index >= (i + pbatch->MeshCount))
    {
        i += pbatch->MeshCount;
        pbatch = pbatch->GetNext();
        SF_ASSERT(!Batches.IsNull(pbatch));
    }
     
    pbatch->Type = PrimitiveBatch::DP_Virtual;
    pbatch->ClearCacheItem();
}

#ifdef SF_BUILD_DEBUG
void Primitive::VerifyMeshIndices()
{
    // Find PrimitiveBatch and mark it as Virtual.
    PrimitiveBatch *pbatch = Batches.GetFirst();
    unsigned        i = 0;

    while(!Batches.IsNull(pbatch))
    {
        SF_ASSERT(pbatch->MeshIndex == i);
        i += pbatch->MeshCount;
        pbatch = pbatch->GetNext();
    }
}
#endif


void Primitive::updateMeshIndicies_Impl()
{    
    if (ModifyIndex >= GetMeshCount())
        return;
    
    // Walk backwards and re-assign indices.
    // This is more efficient then walking forward,
    // since front nodes don't need update.
    PrimitiveBatch *pbatch = Batches.GetLast();
    unsigned        i = GetMeshCount();

    while(!Batches.IsNull(pbatch))
    {
        i -= pbatch->MeshCount;
        pbatch->MeshIndex = i;
        if (i < ModifyIndex)
            break;
        pbatch = pbatch->GetPrev();
    }

    ModifyIndex = GetMeshCount();
}


//--------------------------------------------------------------------
// *** PrimitiveBatch implementation

PrimitiveBatch* PrimitiveBatch::Create(Primitive* p, BatchType type,
                                       unsigned meshIndex, unsigned meshCount)
{
    PrimitiveBatch* pbatch = 
        (PrimitiveBatch*)SF_HEAP_AUTO_ALLOC(p, sizeof(PrimitiveBatch));
    pbatch->Init(p, type, meshIndex, meshCount);
    return pbatch;
}

void PrimitiveBatch::RemoveAndFree()
{
    RemoveNode();
    ClearCacheItem();
    SF_FREE(this);
}

// Simple helper that computes the number of vertices and indices (total) in the batch.
void PrimitiveBatch::CalcMeshSizes(unsigned* ptotalVertices, unsigned *ptotalIndices)
{
    pPrimitive->UpdateMeshIndicies();

    unsigned arrayMeshCount   = IsInstanced() ? 1 : MeshCount; 
    unsigned totalVertexCount = 0,
             totalIndexCount  = 0;
    for (unsigned imesh = 0; imesh < arrayMeshCount; imesh++)
    {
        Mesh* pmesh = pPrimitive->Meshes[MeshIndex + imesh].pMesh;
        totalVertexCount += pmesh->VertexCount;
        totalIndexCount  += pmesh->IndexCount;
    }
    *ptotalVertices = totalVertexCount;
    *ptotalIndices  = totalIndexCount;
}


//--------------------------------------------------------------------
// ***** ComplexMesh & Complex Primitive

ComplexMesh::ComplexMesh(Renderer2DImpl* renderer, MeshProvider *meshProvider,
                         PrimitiveFillManager *fillManager,
                         const Matrix2F& viewMatrix, float morphRatio, 
                         unsigned layer, unsigned meshGenFlags)
: MeshBase(renderer, meshProvider, viewMatrix, morphRatio, layer, meshGenFlags),
  pFillManager(fillManager), pCacheMeshItem(0)
{
}
ComplexMesh::~ComplexMesh()
{
    //unsigned warn_unfinished;    
    if (pCacheMeshItem)
        pCacheMeshItem->NotifyMeshRelease(this);

    // Staging buffer not used for complex mesh.
    SF_ASSERT(StagingBufferSize == 0); 
}

bool ComplexMesh::InitFillRecords(const VertexOutput::Fill* fills, unsigned fillRecordCount,
                                  const Matrix2F& vertexMatrix, HAL *hal,
                                  UPInt *vbSize, unsigned *vertexCount, unsigned *indexCount)
{    
    FillRecords.Resize(fillRecordCount);
    if (FillRecords.GetSize() != fillRecordCount)
        return false;

    TextureManager* textureManager = hal->GetTextureManager();

    // Initialize rendering fill records by
    //  - Creating PrimitiveFills, potentially merged from two FillDatas for EdgeAA.
    //  - Mapping vertex formats to hardware version.
    //  - Assigning buffer offsets.

    *vbSize      = 0;
    *vertexCount = *indexCount = 0;    
    
    for (unsigned i = 0; i < fillRecordCount; i++)
    {
        const VertexOutput::Fill& src = fills[i];
        FillRecord &              fr  = FillRecords[i];
        
        // 1. Create primitive fill
        FillData    fd[2];
        Ptr<Image>  gi[2];
        GetProvider()->GetFillData(&fd[0], GetLayer(), src.FillIndex0, GetMeshGenFlags());
        if (src.MergeFlags & FillMerge_Weight)
            GetProvider()->GetFillData(&fd[1], GetLayer(), src.FillIndex1, GetMeshGenFlags());

        fr.pFill = *pFillManager->CreateMergedFill(src.MergeFlags, src.pFormat,
                                                   &fd[0], &fd[1],
                                                   &gi[0], &gi[1], textureManager);
        if (!fr.pFill)
        {
            // Error... either fail or skip it in list. Fail for now.
            return false;
        }
        
        fr.IndexOffset        = *indexCount;
        fr.IndexCount         = src.IndexCount;
        fr.VertexByteOffset   = *vbSize;
        fr.VertexCount        = src.VertexCount;
        fr.FillMatrixIndex[0] = src.FillIndex0;
        fr.FillMatrixIndex[1] = src.FillIndex1;

        // Keep gradient image references, if any, so that it doesn't die,
        // in case texture needs to be re-generated.
        if (gi[0]) GradientImages.PushBack(gi[0]);
        if (gi[1]) GradientImages.PushBack(gi[1]);

        const VertexFormat* tempBatchVF;
        hal->MapVertexFormat(fr.pFill->GetType(), fr.pFill->GetVertexFormat(),
                             &fr.pFormats[0], &tempBatchVF, &fr.pFormats[1]);

        *vbSize += fr.pFormats[0]->Size * src.VertexCount;
        *vertexCount += src.VertexCount;
        *indexCount += src.IndexCount;
    }

    VertexMatrix = vertexMatrix;
    updateFillMatrixCache(vertexMatrix);
    return true;
}

void ComplexMesh::updateFillMatrixCache(const Matrix2F& vertexMatrix)
{
    unsigned fillCount = GetFillCount(GetLayer(), GetMeshGenFlags());
    FillMatrixCache.Resize(fillCount);
    if (FillMatrixCache.GetSize() != fillCount)
        return;
    for (unsigned i=0; i< fillCount; i++)
    {            
        GetProvider()->GetFillMatrix(this, &FillMatrixCache[i], GetLayer(),
                                     i, GetMeshGenFlags());
        FillMatrixCache[i].Prepend(vertexMatrix);
    }
}



//--------------------------------------------------------------------
// ***** MaskPrimitive

void MaskPrimitive::Insert(UPInt index, const HMatrix& m)
{
    SF_ASSERT(!IsClipped() || (MaskAreas.GetSize() == 0));
    MaskAreas.InsertAt(index, m);   
}
void MaskPrimitive::Remove(UPInt index, UPInt count)
{
    MaskAreas.RemoveMultipleAt(index, count);
}

// RenderQueueItem::Interface impl
void MaskPrimitive::EmitToHAL(RenderQueueItem&, RenderQueueProcessor& qp)
{
    qp.GetHAL()->PushMask_BeginSubmit(this);
}


}} // Scaleform::Render

