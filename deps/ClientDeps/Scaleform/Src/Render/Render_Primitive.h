/**********************************************************************

PublicHeader:   Render
Filename    :   Primitive.h
Content     :   Renderer HAL Mesh and Primitive objects.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_Primitive_H
#define INC_SF_Render_Primitive_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_AllocAddr.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"

#include "Render_Matrix2x4.h"
#include "Render_Color.h"
#include "Render_CxForm.h"
#include "Render_MatrixPool.h"
#include "Render_Stats.h"
#include "Render_Containers.h"
#include "Render_Image.h" // For PrimitiveFill
#include "Render_Gradients.h"
#include "Render_Vertex.h"
#include "Render_Types2D.h"
#include "Render_Math2D.h"

//#include <stdio.h>

namespace Scaleform { namespace Render {

// Forward Declarations
class HAL;
class Renderer2DImpl;
class VertexOutput;
class MeshBase;
class MeshProvider;
class MeshCacheItem;
class PrimitiveBatch;
class FillData;
class Mesh;


// MeshUseStatus describes current statues of MeshCacheItem. This status is used
// to guide TextMeshProvider eviction and is more real-time then ListType, since it
// considers items that haven't passed HW Fence as being in use.

enum MeshUseStatus
{
    MUS_Uncached,
    MUS_LRUTail,
    MUS_PrevFrame,
    MUS_ThisFrame,   
    // The following values are considered "InUse":
    // - InUse is reported for InFlight items, or items that haven't passed fence.
    // - Pinned is reported by TextMeshProvider system is glyph as been explicitly pinned.
    MUS_InUse,
    MUS_Pinned
};


// MeshStagingNode describes location of the mesh in the staging buffer, if any.

struct MeshStagingNode : public ListNode<MeshStagingNode>
{    
    // If StagingBufferSize != 0, this is location of cached mesh data
    // within the staging buffer, which holds mesh data after mesh-gen.
    UPInt       StagingBufferSize;
    UPInt       StagingBufferOffset;    
    UPInt       StagingBufferIndexOffset;
    // If not 0, this is the number of times a mesh has been pinned in the
    // staging buffer. Pinned meshes can not be swapped out.
    unsigned    PinCount;

    // Mesh Data size (useful even when the object is swapped out).
    unsigned    VertexCount;
    unsigned    IndexCount;

    MeshStagingNode()
    :   StagingBufferSize(0),
        StagingBufferOffset(0),
        StagingBufferIndexOffset(0),
        PinCount(0),
        VertexCount(0), IndexCount(0)
    {
    }

    virtual ~MeshStagingNode() { }

    virtual void OnStagingNodeEvict()
    {
        StagingBufferSize = 0;
    }
};


struct Scale9GridData : public RefCountBase<Scale9GridData, Stat_Default_Mem>
{
    RectF           S9Rect;
    RectF           Bounds;
    Matrix2F        ShapeMtx;
    Matrix2F        Scale9Mtx;
    Matrix2F        ViewMtx;

    enum { MeshKeySize = 14 };

    Scale9GridData()
        : S9Rect(RectF::NoInit), Bounds(RectF::NoInit),
          ShapeMtx(Matrix2F::NoInit), Scale9Mtx(Matrix2F::NoInit), ViewMtx(Matrix2F::NoInit)
    {
    }

    void MakeMeshKey(float* keyData) const
    {
        *keyData++ = S9Rect.x1;
        *keyData++ = S9Rect.y1;
        *keyData++ = S9Rect.x2;
        *keyData++ = S9Rect.y2;
        *keyData++ = Bounds.x1;
        *keyData++ = Bounds.y1;
        *keyData++ = Bounds.x2;
        *keyData++ = Bounds.y2;
        *keyData++ = ViewMtx.Sx();
        *keyData++ = ViewMtx.Sy();
        *keyData++ = ViewMtx.Shx();
        *keyData++ = ViewMtx.Shy();
        *keyData++ = ShapeMtx.Tx();
        *keyData++ = ShapeMtx.Ty();
    }
};


// MeshProvider is a source for vertex data and fills used by a mesh,
// which is obtained through its GetData/GetFillData methods. A single
// MeshProvider represents non-changing shape/mesh data that can be shared
// by multiple mesh instances. Each mesh can, however, display it with different
// transform matrices (and tessellations).
//
// MeshProvider can describe both simple and complex meshes. Simple
// meshes have fill count <=1 and are grouped into Primitives for batching.
// Complex mesh are represented by ComplexPrimitive instead and thus are not batched.

struct StrokeGenerator;

class MeshProvider
{
public:    
    virtual ~MeshProvider() { }
    // Ref-count "wrapper" implementation. Although RefCountBase functions are
    // non-virtual, we can still wrap them in virtual ones.
    virtual void    AddRef() { }
    virtual void    Release() { }

    // GetData is called by renderer to obtain mesh vertex data;
    // is is essentially an interface to the tessellator.
    virtual bool    GetData(MeshBase *mesh, VertexOutput* out,
        unsigned meshGenFlags) = 0;
    virtual void    OnEvict(MeshBase *mesh) { SF_UNUSED(mesh); }

    // Return cached bounds for identity matrix.
    // Default implementation calls GetBounds by default for test derived classes.
    virtual RectF   GetIdentityBounds() const
    { return GetBounds(Matrix2F::Identity); }

    virtual RectF   GetBounds(const Matrix2F& m) const
    { SF_UNUSED(m); return RectF(); }

    virtual RectF   GetCorrectBounds(const Matrix2F& m, StrokeGenerator* gen, const ToleranceParams& tol) const
    { SF_UNUSED3(m, gen, tol); return RectF(); }

    // *** Fill Interface

    virtual unsigned    GetLayerCount() const { return 1; }

    // GetFillCount returns the logical number of fills a shape has,
    // which also serves to whether this is a simple or complex shape.
    // Simple shapes have FillCount <= 1, complex shapes have more.
    // FillCount and data should not change after MeshProvider is created.
    // The number if fills may be affected by meshGenFlags; a mask, for example,
    // typically only needs one fill.
    virtual unsigned    GetFillCount(unsigned layer, unsigned meshGenFlags) const
    { SF_UNUSED2(layer, meshGenFlags); return 0; }

    virtual void        GetFillData(FillData* data, unsigned layer,
        unsigned fillIndex, unsigned meshGenFlags)
    {
        SF_ASSERT(0); SF_UNUSED4(data, layer, fillIndex, meshGenFlags);
    }
    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
        unsigned fillIndex, unsigned meshGenFlags)
    {
        SF_ASSERT(0); SF_UNUSED5(mesh, matrix, layer, fillIndex, meshGenFlags);
    }
};

// MeshProvider base implementation that adds reference counting.
//  - RefCount logic is separated to allow MeshKeySet provider implementation
//    without reference counting.
class MeshProvider_RCImpl : public RefCountBase<MeshProvider_RCImpl, Stat_Default_Mem>, 
    public MeshProvider
{
public:
    virtual void AddRef()  { RefCountBase<MeshProvider_RCImpl, Stat_Default_Mem>::AddRef(); }
    virtual void Release() { RefCountBase<MeshProvider_RCImpl, Stat_Default_Mem>::Release(); }
};


// MeshBase
//  - Common base class for Mesh and ComplexPrimitive, separated to pass it
//    as argument for MeshProvider.

class MeshBase : public RefCountBase<MeshBase, StatRender_Mesh_Mem>, public MeshStagingNode
{
    // Renderer2D use not ideal here since we only have access to HAL.
    // Perhaps this should be a void* pointer?
    Renderer2DImpl* pRenderer2D;

    Ptr<MeshProvider>   pProvider;
    Ptr<Scale9GridData> pScale9Grid;
    
    // ViewMatrix used at the time Mesh was created, needed for tessellation.
    // TBD: Not efficient, it is better to compute it in RenderQueueProcessor and
    // as an argument to Provider::GetData.
    Matrix2F    ViewMatrix;

    float       MorphRatio;
    
    // TBD: Remove since layer doesn't makes sence with ComplexPrimitive,
    // it simply supports the temporary multy-layer shape implementation.
    unsigned    Layer;
    // MeshGenFlags to use for accessing MeshProvider.
    unsigned    MGFlags;

public:

    MeshBase(Renderer2DImpl* prenderer, MeshProvider *provider,
             const Matrix2F& viewMatrix, float morphRatio, 
             unsigned layer, unsigned meshGenFlags)
    : pRenderer2D(prenderer), pProvider(provider), ViewMatrix(viewMatrix),
      MorphRatio(morphRatio), Layer(layer), MGFlags(meshGenFlags)
    { }

    MeshProvider*   GetProvider() const { return pProvider; }
    // Used by MeshKey.
    void            ClearProvider()         { pProvider.Clear(); }
    inline void     OnEvictProviderNotify();

    Renderer2DImpl* GetRenderer()     const { return pRenderer2D; }
    const Matrix2F& GetViewMatrix()   const { return ViewMatrix; }
    float           GetMorphRatio()   const { return MorphRatio; }
    unsigned        GetLayer()        const { return Layer; }
    unsigned        GetMeshGenFlags() const { return MGFlags; }

    Scale9GridData* GetScale9Grid()   const { return pScale9Grid; }
    void            SetScale9Grid(Scale9GridData* s9g) { pScale9Grid = s9g; }

    virtual bool    IsEvicted() const { return StagingBufferSize == 0; }
};


// Fill style merge flag bits for PrimitiveFillManager::CreateFill, reported
// as a part of VertexOutput::Fill.
enum FillMergeFlags
{
    FillMerge_None   = 0x00,
    FillMerge_EAlpha = 0x01, // Edge Alpha was recorder into vertices.
    FillMerge_Weight = 0x02,  // Fill blending weight was recorded.
    FillMerge_EAlpha_Weight = FillMerge_EAlpha|FillMerge_Weight
};


// VertexOutput is a vertex-generation interface used to supply mesh
// vertices from MeshProvider/tessellator to rendering caches. 
// VertexOutput object us passed to MeshProvider::GetData, which populates
// it calling BeginOutput, SetVertices, SetIndices calls.
//
// In case of EdgeAA and complex shapes, VertexOutput can receive
// multiple vertex sets having different fill styles. The type and
// amount of such data is described by VertexOutput::Fill array
// passed to BeginOutput; this information allows the cache system
// to pre-allocate the required memory.

class VertexOutput
{   
public:
    virtual ~VertexOutput() {}

    // Fill describes a single vertex set within VertexOutput, that
    // will be rendered with a common fill style. The fills to be used
    // index MeshProvider, with up two two fills applied simultaneously in
    // case of EdgeAA.
    struct Fill
    {
        unsigned            VertexCount;
        unsigned            IndexCount;
        const VertexFormat* pFormat;
        // Fill indices for MeshProvider::GetFillData().
        unsigned            FillIndex0, FillIndex1;
        unsigned            MergeFlags; // Described by FillMergeFlags.
        unsigned            MeshIndex;  // Used only in GetData.
    };

    // BeginOutput specified the number and types of vertex fill sets that
    // need to be allocated. This method is called before SetVertices/Indices.
    //  - fills array must remain alive until EndOutput is called.
    //  - fillCount is the number of independent fills, each having a
    //    number of triangles. For simple Mesh, this should be 1.
    //  - vertexMatrix is applied to vertex coordinates; all of the vertices
    //    will be multiplied by this matrix before the full transformation
    //    is applied. This matrix is most typically used to scale the 16-bit
    //    integer coordinate space to the intended shape coordinates.
    virtual bool    BeginOutput(const Fill* fills, unsigned fillCount,
                                const Matrix2F& vertexMatrix = Matrix2F::Identity) = 0;
    
    virtual void    EndOutput() = 0;

    // Provides vertex data for vertex fills; called after BeginOutput.
    virtual void    SetVertices(unsigned fillIndex, unsigned vertexOffset,
                                void* pvertices, unsigned vertexCount) = 0;
    virtual void    SetIndices(unsigned fillIndex, unsigned indexOffset,
                               UInt16* pindices, unsigned indexCount) = 0;
};


// Mesh generation flags that affect results reported by the MeshProvider,
// including the number and type of fills, including vertex format.
enum MeshGenFlags
{
    Mesh_EdgeAA       = 0x0001,
    Mesh_Mask         = 0x0002,
    Mesh_Scale9       = 0x0008,
    //Mesh_CxformAlpha  = 0x0004
};


inline void MeshBase::OnEvictProviderNotify()
{
    if (pProvider) pProvider->OnEvict(this);
}


// Mesh is an object that feeds coordinates to the renderer.
//  - An object of this type is created by HAL::CreateMesh

class Mesh : public MeshBase
{
public:    
    // List of MeshCacheItem we participate in; if the mesh
    // is thrown away all those items can as well.
    //   - A single mesh can live in several chunks and thus in
    //     several MeshCacheItem buffers.
    //   - Since chunks AddRef to us, we don't need to store pointers to them.
    ArrayReserveLH_Mov<MeshCacheItem*> CacheItems;

    Matrix2F VertexMatrix;

    bool     LargeMesh;

    Mesh(Renderer2DImpl* prenderer, MeshProvider *pdataProvider,
         const Matrix2F& viewMatrix, float morphRatio=0,
         unsigned layer=0, unsigned meshGenFlags = 0);
    ~Mesh();

    void            RemoveMeshCacheItem(MeshCacheItem* p)
    {
        CacheItems.Remove(p);
        // TBD: Should also check StagingBufferSize and detect Staging evictions
        if ((CacheItems.GetSize() == 0) && !StagingBufferSize)
            OnEvictProviderNotify();
    }
    virtual void    OnStagingNodeEvict()
    {
        MeshBase::OnStagingNodeEvict();
        if ((CacheItems.GetSize() == 0))
            OnEvictProviderNotify();
    }

    virtual bool    IsEvicted() const
    { return ((CacheItems.GetSize() == 0) && !StagingBufferSize); }

    // Returns real-time use status of cache, which considers fencing.
    // Used to drive eviction of dependent resources, such as font cache glyphs.
    enum MeshUseStatus GetUseStatus() const;


    // Reference counting helpers.
    void    AddRef_NTimes(unsigned count)  { while(count--) AddRef(); }
    void    Release_NTimes(unsigned count) { while(count--) Release(); }

    static void AddRef_Array(Mesh** pmeshes, unsigned count)
    {
        while (count--) (*(pmeshes++))->AddRef();
    }
    static void Release_Array(Mesh** pmeshes, unsigned count)
    {
        while (count--) (*(pmeshes++))->Release();
    }
};



// Rendering fill style that will be applied to all the elements
// of the DP. Rendering style dictates the elements that need to be
// present in the vertex declaration. Following elements may be
// required based on format:
//  - XY     - Position, always required.
//  - UV     - UV texture coordinates.
//  - Color  - Vertex color to be used in rendering.
//  - EAlpha - Edge AA Alpha factor channel.
//  - Weight - Texture weight, mixes firsts Texture with second one/EdgeAA alpha.

enum PrimitiveFillType
{
    // VColor    - refers to interpolated vertex color.
    // Texture   - texture with coordinates generated based on matrix.
    // UVTexture - texture coordinates provided explicitly.
    PrimFill_None,
    PrimFill_Mask,                      // { XY }
    PrimFill_SolidColor,                // { XY }
    PrimFill_VColor,                    // { XY, Color }
    PrimFill_VColor_EAlpha,             // { XY, Color, EAlpha }
    PrimFill_Texture,                   // { XY }
    PrimFill_Texture_EAlpha,            // { XY, EAlpha }
    PrimFill_Texture_VColor,            // { XY, Color, Weight1 }
    PrimFill_Texture_VColor_EAlpha,     // { XY, Color, Weight1, EAlpha }
    PrimFill_2Texture,                  // { XY, Weight1 }
    PrimFill_2Texture_EAlpha,           // { XY, Weight1, EAlpha  }
    PrimFill_UVTexture,                 // { XY, UV }
    PrimFill_UVTextureAlpha_VColor,     // { XY, UV, Color }
    PrimFill_Type_Count
};

// Modifier Flags applied to PrimitiveFill (which affect the shaders).
// 16 bits available
enum PrimitiveFillFlags
{
    FF_Multiply     = 0x01,
    FF_AlphaWrite   = 0x02,
    FF_3DProjection = 0x04  
};

class Texture;

// PrimitiveFillData holds initialization values for PrimitiveFill and
// is used to create that class. This data is separated so that it can be
// read-only within a PrimitiveFill and to make sure that it isn't accidentally
// accidentally used without PrimitiveFillManager::Create being used,
// which is required for sharing and proper batch sorting.

struct PrimitiveFillData
{
    PrimitiveFillType Type;
    Color             SolidColor;
    ImageFillMode     FillModes[2];
    Ptr<Texture>      Textures[2];
    const VertexFormat* pFormat;

    PrimitiveFillData()
        : Type(PrimFill_None), SolidColor(0) { }
    PrimitiveFillData(PrimitiveFillType type, const VertexFormat* format)
        : Type(type), SolidColor(0), pFormat(format)
    { SF_ASSERT(CheckVertexFormat(type, format)); }
    PrimitiveFillData(PrimitiveFillType type, const VertexFormat* format, Color c)
        : Type(type), SolidColor(c), pFormat(format)
    { SF_ASSERT(CheckVertexFormat(type, format)); }
    PrimitiveFillData(PrimitiveFillType type, const VertexFormat* format,
                      Texture* texture0, ImageFillMode fm0 = ImageFillMode(),
                      Texture* texture1 = 0, ImageFillMode fm1 = ImageFillMode())
        : Type(type), SolidColor(0), pFormat(format)
    {
        SF_ASSERT(CheckVertexFormat(type, format));
        FillModes[0] = fm0;
        FillModes[1] = fm1;
        Textures[0]  = texture0;
        Textures[1]  = texture1;
    }
    PrimitiveFillData(const PrimitiveFillData& src)
        : Type(src.Type), SolidColor(src.SolidColor), pFormat(src.pFormat)
    {
        FillModes[0] = src.FillModes[0];
        FillModes[1] = src.FillModes[1];
        Textures[0]  = src.Textures[0];
        Textures[1]  = src.Textures[1];
    }

    bool operator == (const PrimitiveFillData& other) const
    {
        return (Type == other.Type) && 
               (SolidColor == other.SolidColor) &&
               (FillModes[0] == other.FillModes[0]) && (FillModes[1] == other.FillModes[1]) &&
               (Textures[0] == other.Textures[0]) && (Textures[1] == other.Textures[1]) &&
               ((pFormat == other.pFormat) || (*pFormat == *other.pFormat));
    }
    bool operator != (const PrimitiveFillData& other) const
    {
        return !operator ==(other);
    }

    PrimitiveFillData& operator = (const PrimitiveFillData& src)
    {
        Type         = src.Type;
        SolidColor   = src.SolidColor;
        FillModes[0] = src.FillModes[0];
        FillModes[1] = src.FillModes[1];
        Textures[0]  = src.Textures[0];
        Textures[1]  = src.Textures[1];
        pFormat      = src.pFormat;
        return *this;
    }

    UByte   GetTextureCount() const
    {
        if ((Type >= PrimFill_Texture) && (Type <= PrimFill_2Texture_EAlpha))
            return (Type >= PrimFill_2Texture) ? 2 : 1;
        return 0;
    }

    UPInt    GetHashValue() const 
    {
        return Type ^ SolidColor.ToColor32() ^ 
               (FillModes[0].Fill << 2) ^ (FillModes[1].Fill << 4) ^
               (((UPInt)Textures[0].GetPtr()) >> 2) ^ ((UPInt)Textures[1].GetPtr()) ^
               pFormat->Size;
    }

    // Debugging functions - verifies that format has channel need for the fillType.
    // Will return false and output debug messages if there is a mismatch.
    static bool CheckVertexFormat(PrimitiveFillType fill, const VertexFormat* format);
};


enum FillType
{
    Fill_None,
    Fill_Mask,
    Fill_SolidColor,
    Fill_VColor,
    Fill_Image,
    Fill_Gradient,
    // Added for testing purposes; used Color as key.
    Fill_VColor_TestKey
};


// TO DO: Consider a separate header file
class FillData
{
public:
    // TO DO: Add AddRef/Release logic for potential use by the client (questionable)
    FillData(FillType type = Fill_VColor);
    FillData(UInt32 color);
    FillData(Image* p, ImageFillMode fm = ImageFillMode());
    FillData(GradientData* p);

public:
    FillType            Type;
    union
    {
        UInt32          Color;
        GradientData*   pGradient;
        Image*          pImage;
    };
    PrimitiveFillType   PrimFill;
    ImageFillMode       FillMode;
    const VertexFormat* pVFormat;
};



class PrimitiveFillManager;

// PrimitiveFill is a hashed structure that describes a hardware fill.
// Since identical values of this class are always created by PrimitiveFillManager::Create
// and cached based on their values, they can be compared based on pointer value.
// Used in render thread only.

class PrimitiveFill : public RefCountBaseNTS<PrimitiveFill, StatRender_Fill_Mem>
{
    void operator = (const PrimitiveFill&) { } // Assignment not allowed.
protected:
    const PrimitiveFillData Data;
    PrimitiveFillManager*   pManager;
public:
    PrimitiveFill(const PrimitiveFillData& srcData, PrimitiveFillManager* manager = 0)
        : Data(srcData), pManager(manager) { }
    ~PrimitiveFill();

    bool operator == (const PrimitiveFill& other) const     { return Data == other.Data; }
    bool operator != (const PrimitiveFill& other) const     { return Data != other.Data; }

    PrimitiveFillType   GetType() const                     { return Data.Type; }
    Color               GetSolidColor() const               { return Data.SolidColor; }
    Texture*            GetTexture(unsigned index = 0) const    { return Data.Textures[index]; }
    ImageFillMode       GetFillMode(unsigned index = 0) const   { return Data.FillModes[index]; }
    const VertexFormat* GetVertexFormat() const             { return Data.pFormat; }
    UByte               GetTextureCount() const             { return Data.GetTextureCount(); }
    UPInt               GetHashValue() const                { return Data.GetHashValue(); }

    const PrimitiveFillData& GetData() const { return Data; }

    void                SetManager(PrimitiveFillManager* manager)
    { SF_ASSERT(pManager == 0); pManager = manager; }
    PrimitiveFillManager* GetManager() const { return pManager; }

    // Hash operator used for PrimitiveFill*
    struct PtrHashFunctor
    {    
        UPInt  operator()(const PrimitiveFill* fill) const
        { return fill->GetHashValue(); }
        UPInt  operator()(const PrimitiveFillData& data) const
        { return data.GetHashValue(); }
    };
};

inline bool operator == (const PrimitiveFill* fill, const PrimitiveFillData& data)
{
    return fill->GetData() == data;
}

// PrimitiveFillManager maintains a set of all live PrimitiveFill types, allowing
// them to be reused. PrimitiveFill reuse is important fill pointer values serve as
// keys for batch sorting.

class PrimitiveFillManager : public RefCountBase<PrimitiveFillManager, StatRender_Fill_Mem>
{
    friend  class PrimitiveFill;
    friend  class GradientImage;

    typedef HashSetLH<PrimitiveFill*, PrimitiveFill::PtrHashFunctor> FillHashSet;
    typedef HashSetLH<GradientImage*, GradientImage::PtrHashFunctor> GradientHashSet;

public:
    PrimitiveFillManager(HAL* hal) : pHAL(hal) { }
    ~PrimitiveFillManager()
    {
    }

    PrimitiveFill* CreateFill(const PrimitiveFillData& initdata);
    PrimitiveFill* CreateFill(const FillData& initdata, Ptr<Image>* gradientImg, TextureManager* mng);

    // Creates a complex merged fill built out of one or more regular fills.
    PrimitiveFill* CreateMergedFill(unsigned mergeFlags, const VertexFormat* vformat,
                                    const FillData* fd0, const FillData* fd1,
                                    Ptr<Image>* gradientImg0, Ptr<Image>* gradientImg1,
                                    TextureManager* mng);

    HAL*    GetHAL() const { return pHAL; }

private:
    void    removeFill(PrimitiveFill* fill);
    void    removeGradient(GradientImage* img);
    // Creates and/or registers a gradient image; caller must store reference.
    Image*  createGradientImage(GradientData* data);

    HAL*            pHAL;
    FillHashSet     FillSet;
    GradientHashSet Gradients;
};



// CacheBase is a base class for cached that may need to be unlocked
// during render queue processing.
class CacheBase
{
public:
    virtual ~CacheBase() {}

    virtual void UnlockBuffers() = 0;
};

enum CacheType
{
    Cache_Mesh  = 0,
    Cache_Glyph = 1,
    Cache_Count
};

// RQCacheInterface maintains cache pointers needed for the RenderQueue
// and allows them to be unlocked efficiently.
class RQCacheInterface
{
public:

    RQCacheInterface() : LockFlags(0)
    {
        pCaches[Cache_Mesh] = 0;
        pCaches[Cache_Glyph]= 0;
    }

    CacheBase*  GetCache(CacheType type) const             { return pCaches[type]; }
    void        SetCache(CacheType type, CacheBase* cache) { pCaches[type] = cache; }

    // Returns
    bool        AreCachesLocked() const { return (LockFlags != 0); }
    void        UnlockCaches()
    {
        for (unsigned i=0; i < Cache_Count; i++)
        {
            if ((LockFlags & (1 << i)) && pCaches[i])
                pCaches[i]->UnlockBuffers();
        }
    }

    void        SetCacheLocked(CacheType type)   { LockFlags |= (1 << type); }
    void        ClearCacheLocked(CacheType type) { LockFlags &= (unsigned)~(1 << type); }

private:
    CacheBase*  pCaches[Cache_Count];
    unsigned    LockFlags;
};



// RenderQueueItem is any item that can be placed into a rendering queue.
// TBD: Should we place it in its own header?
// It includes items such as:
//  - Primitive, Complex 
//  - Stencil Op, FilterBuffer apply/op
//  - BlendMode/Viewport change
// The primary reason that the state change operations such as StencilOp
// are needed is that they are independent of primitives, as the same state
// can apply to multiple primitives/meshes/other commands. With that being the
// case, it's illogical to make them a part of a primitive.

class RenderQueueProcessor;
class PrimitivePrepareBuffer;
class PrimitiveEmitBuffer;
class MeshCache;

class RenderQueueItem
{
public:

    enum QIPrepareResult
    {
        QIP_Done,     // Prepare complete, can call Emit.
        QIP_NeedCache // Need to call Draw/Clear up cache.
        //  PC: Flush buffer. XBox/Console: Check queue and return.
    };

    class Interface
    {
    public:
        typedef RenderQueueItem::QIPrepareResult QIPrepareResult;

        virtual ~Interface() {}

        // QueueItem_Process prepares item for rendering, typically be bringing it
        // into the cache. It may need to be called in a loop with QueueItem_ApplyToHAL
        // if QIP_NeedCache is returned.        
        virtual QIPrepareResult  Prepare(RenderQueueItem&,
                                         RenderQueueProcessor&, bool waitForCache)
        { SF_UNUSED(waitForCache); return QIP_Done; }

        // Emits the queue item data to HAL, either by rendering it
        // or applying its state.
        virtual void             EmitToHAL(RenderQueueItem&, RenderQueueProcessor&) { }
    };

    RenderQueueItem() : pImpl(0), Data(0) { }
    RenderQueueItem(Interface* i, void* data) : pImpl(i), Data(data) { }
    RenderQueueItem(const RenderQueueItem& other) : pImpl(other.pImpl), Data(other.Data) { }

    void Clear()
    {
        pImpl = 0; Data = 0;
    }    
    void Init(Interface* i, void* data)
    {
        pImpl = i; Data  = data;
    }

    RenderQueueItem& operator = (const RenderQueueItem& other)
    {
        pImpl = other.pImpl;
        Data  = other.Data;
        return * this;
    }

    void*      GetData() const { return Data; }
    Interface* GetInterface() const { return pImpl; }

    // *** Item Interface delegate
    QIPrepareResult  Prepare(RenderQueueProcessor& rqp, bool waitForCache)
    {
        return pImpl->Prepare(*this, rqp, waitForCache);
    }
    void             EmitToHAL(RenderQueueProcessor& rqp)
    {
        pImpl->EmitToHAL(*this, rqp);
    }

protected:
    Interface* pImpl;
    void*      Data;
};



// Primitive is a collection of meshes such that all of those are drawn as a unit.
// Constraints:
//  - All the elements of DP will be drawn with the same fill style.

class Primitive : public RefCountBase<Primitive, StatRender_Primitive_Mem>,
                  public RenderQueueItem::Interface
{
    friend class PrimitiveBatch;
    friend class PrimitivePrepareBuffer;    
public:
    HAL*                    pHAL;
    Ptr<PrimitiveFill>      pFill; // Type of fill we have.  
    List<PrimitiveBatch>    Batches;
    const UByte             MatricesPerMesh; // 1, 2, 3 depending on fill


    // MeshEntry combines mesh with Variable-format matrix describing it,
    // stored in one array for modify efficiency.
    struct MeshEntry
    {
        HMatrix   M;
        Ptr<Mesh> pMesh;
    };

    ArrayLH<MeshEntry> Meshes;
    // Lowest index that was modified, PtimitiveBatch::MeshIndex values
    // grater or equal to this may be incorrect.
    unsigned           ModifyIndex;
    
    Primitive(HAL* phal, PrimitiveFill* pfill);
    ~Primitive();

    PrimitiveFill*       GetFill() const { return pFill; }
    const VertexFormat*  GetVertexFormat() const { return GetFill()->GetVertexFormat(); }

    bool        Insert(unsigned index, Mesh* pmesh, const HMatrix& m);
    void        Remove(unsigned index, unsigned count);

    // Accessors for Meshes/Matrices.    
    unsigned    GetMeshCount() const { return (unsigned)Meshes.GetSize();}
    Mesh*       GetMesh(unsigned index) const               { return Meshes[index].pMesh; }
    void        SetMesh(unsigned index, Mesh* pmesh);

    void        SetMatrix(unsigned index, const HMatrix& m) { Meshes[index].M = m; }
    HMatrix&    GetMatrix(unsigned index)                   { return Meshes[index].M; }
    const HMatrix& GetMatrix(unsigned index) const          { return Meshes[index].M; }
    
    // RenderQueueItem::Interface implementation
    virtual QIPrepareResult  Prepare(RenderQueueItem&, RenderQueueProcessor&,
                                     bool waitForCache);
    virtual void             EmitToHAL(RenderQueueItem&, RenderQueueProcessor&);
    
#ifdef SF_BUILD_DEBUG
    void VerifyMeshIndices();
#else
    void VerifyMeshIndices() { }
#endif

    // Updates PrimitiveBatch::MeshIndex values if necessary,
    // in case they became stale due to insert.
    void    UpdateMeshIndicies()
    {        
        if (ModifyIndex < GetMeshCount())
            updateMeshIndicies_Impl();     
    }
protected:

    // Helper functions for Primitive processing, may also be called
    // directly for text.
    QIPrepareResult  prepare(void* item,
                             PrimitivePrepareBuffer*, PrimitiveEmitBuffer*,
                             HAL* hal, MeshCache* cache, bool waitForCache);
    void             emitToHAL(void* item,
                               PrimitivePrepareBuffer*, PrimitiveEmitBuffer*,
                               HAL* hal);

    void    updateMeshIndicies_Impl();
};


struct MeshCacheItemUseNode : public ListNode<MeshCacheItemUseNode>
{
    // Pointer to the mesh "buffer" data.
    MeshCacheItem* pMeshItem;

    void        ClearMeshItem()
    {
        if (pMeshItem)
        {
            RemoveNode();
            pMeshItem = 0;
        }               
    }

    void        SetMeshItem(MeshCacheItem* p);    
};


class PrimitiveBatch : public ListNode<PrimitiveBatch>
{
    friend class Primitive;
    friend class PrimitivePrepareBuffer;
public:
    enum BatchType
    {
        // ShaderManager::ApplyFill requirement:
        //  - Single, Batch, Instanced must come in that order
        
        // A single mesh. We identify this separately because it can use more
        // compact vertex format and simpler shaders.
        DP_Single,
        // A batch mode with multiple meshes combined together into a single draw call.
        DP_Batch,
        // A single mesh replicated multiple times through HW instancing. There must be
        // more then 1 item in the mesh for this state to be set.
        DP_Instanced,

        // Virtual chunk haven't yet been processed by the renderer, so its
        // meshes data is not reserved. When Virtual chunk is processed, it
        // is converted into one a list of different chunk types.
        DP_Virtual,

        // This mesh failed mesh-gen, so can not be displayed. Can have multiple instances.
        DP_Failed,
        // No BatchType was used
        DP_None
    };

    MeshCacheItemUseNode MeshNode;

    BatchType            Type;
    // Destination vertex format applied to contents of this buffer.
    const VertexFormat*  pFormat;
    // Large meshes have size that is past the threshold so they can't be batched; they
    // can, however, be instanced.
    bool                 LargeMesh;

protected:     
    // Primitive that owns us and MeshIndex/MeshCount in it.
    // MeshIndex may be temporarily out of sync during updates.
    Primitive*  pPrimitive; 
    unsigned    MeshCount;
    unsigned    MeshIndex;
public:

    // Init is called by Create instead of constructor.
    void        Init(Primitive* p, BatchType type,
                     unsigned meshIndex, unsigned meshCount)
    {
        MeshNode.pMeshItem = 0;
        Type      = type;
        pFormat   = 0;
        LargeMesh = false;
        pPrimitive= p;
        MeshCount = meshCount;
        MeshIndex = meshIndex;
    }
/*
    StrideArray<MeshBase*> GetMeshes() const
    {
        pPrimitive->UpdateMeshIndicies();
        return StrideArray<MeshBase*>((MeshBase**)&pPrimitive->Meshes[MeshIndex],
                                      MeshCount,
                                      sizeof(Primitive::MeshEntry));
    }
    */
    StrideArray<MeshBase*> GetMeshes_InstanceAdjusted() const
    {
        pPrimitive->UpdateMeshIndicies();
        return StrideArray<MeshBase*>((MeshBase**)&pPrimitive->Meshes[MeshIndex].pMesh,
                                      IsInstanced() ? 1 : MeshCount,
                                      sizeof(Primitive::MeshEntry));
    }

    unsigned    GetMeshIndex() const
    {
        SF_ASSERT(pPrimitive->ModifyIndex >= pPrimitive->GetMeshCount());
        return MeshIndex;
    }
    unsigned    GetMeshCount() const   { return MeshCount; }
    Primitive*  GetPrimitive() const   { return pPrimitive; }

    void        ClearCacheItem()                { MeshNode.ClearMeshItem(); }
    MeshCacheItem* GetCacheItem() const         { return MeshNode.pMeshItem; }
    void        SetCacheItem(MeshCacheItem* p)  { MeshNode.SetMeshItem(p); }

    bool        IsVirtual() const   { return Type == DP_Virtual; }
    bool        IsInstanced() const { return Type == DP_Instanced; }

    static  PrimitiveBatch*   Create(Primitive* p, BatchType type,
                                     unsigned meshIndex, unsigned meshCount);

    // Removed this batch from the Primitive and frees its data.
    void            RemoveAndFree();    

    // Simple helper that computes the total vertex and index count in the batch.
    // For now, single mesh size returned for instanced meshes.
    void            CalcMeshSizes(unsigned* ptotalVertices, unsigned *ptotalIndices);

    // Returns pointer to our linked list node inside of Primitive::Batches. This
    // is used to disambiguate base class access
    //ListNode<PrimitiveBatch>* GetThisNode() { return (ListNode<PrimitiveBatch>*)this; }
    PrimitiveBatch* GetNext()                { return pNext; }
    PrimitiveBatch* GetPrev()                { return pPrev; }
};


//--------------------------------------------------------------------
// ***** ComplexMesh

// ComplexMesh is a mesh type with multiple fill styles, all drawn
// together and cached as a unit.
// Note that HAL ComplexPrimitive object is no longer needed, 
// since ComplexPrimitiveBundle draws meshes directly.

class ComplexMesh : public MeshBase
{
    friend class MeshCacheItem;
public:
    ComplexMesh(Renderer2DImpl* renderer, MeshProvider *meshProvider,
                PrimitiveFillManager *fillManager,
                const Matrix2F& viewMatrix, float morphRatio = 0, 
                unsigned layer = 0, unsigned mesgGenFlags = 0);
    ~ComplexMesh();
    
    PrimitiveFillManager* GetFillManager() { return pFillManager; }

    void            RemoveMeshCacheItem(MeshCacheItem* p)
    {
        SF_ASSERT(p == pCacheMeshItem);
        SF_UNUSED(p);
        pCacheMeshItem = 0;        
        OnEvictProviderNotify();
    }

    virtual bool    IsEvicted() const
    { return (!pCacheMeshItem && !StagingBufferSize); }


    /*
    // TBD: Looks like this is not needed.. because there is only one cache item.
    virtual void    OnStagingNodeEvict()
    {
        StagingBufferSize = 0;
        if (CacheItems.GetSize() == 0)
            pProvider->OnEvict(this);
    }
    */

    HAL* GetHAL() const { return pFillManager->GetHAL(); }

    // FillRecord descries a single fill style used by mesh; this
    // data is generated as a result of tessellation.
    struct FillRecord
    {
        Ptr<PrimitiveFill>  pFill;
        const VertexFormat* pFormats[2];        // [0] = Single, [1] = Instanced vertex formats.
        unsigned            IndexOffset;        // Index offset.. from start of IB buffer.
        unsigned            IndexCount;
        UPInt               VertexByteOffset;   // Offset in bytes, sue to different vertex sizes.
        unsigned            VertexCount;        // Number of vertices starting at VertexByteOffset.
        unsigned            FillMatrixIndex[2]; // Fill matrix indices
    };

    unsigned        GetLayerCount() const { return GetProvider()->GetLayerCount(); }
    unsigned        GetFillCount(unsigned layer, unsigned meshGenFlags) const
    { return GetProvider()->GetFillCount(layer, meshGenFlags); }
    void            GetFillData(FillData* data, unsigned layer,
                                unsigned fillIndex, unsigned meshGenFlags) const
    { return GetProvider()->GetFillData(data, layer, fillIndex, meshGenFlags); }

    const FillRecord* GetFillRecords() const   { return &FillRecords[0]; }
    unsigned          GetFillRecordCount() const { return (unsigned)FillRecords.GetSize(); }

    const Matrix2F* GetFillMatrixCache() const { return &FillMatrixCache[0]; }    
    MeshCacheItem*  GetCacheItem() const       { return pCacheMeshItem; }

    const Matrix2F& GetVertexMatrix() const    { return VertexMatrix; }

    // Initializes FillRecords/FillMatrixCache based on VertexOutput generated data,
    // also returning the required cumulative vertexbuffer size and counts.
    bool            InitFillRecords(const VertexOutput::Fill* fills, unsigned fillCount,
                                    const Matrix2F& vertexMatrix, HAL *hal,
                                    UPInt *vbSize, unsigned *vertexCount, unsigned *indexCount);
        
private:    
    void updateFillMatrixCache(const Matrix2F& vertexMatrix);

    PrimitiveFillManager* pFillManager;
    // MeshCacheItem contains our tessellated data (Vertex/Index buffer descriptor).
    MeshCacheItem*      pCacheMeshItem;
    Matrix2F            VertexMatrix;
    ArrayLH<FillRecord> FillRecords;
    // FillMatrices pre-multiplied by vertexMatrix (VertexMatrix * pProvider->GetFillMatrix).
    ArrayLH<Matrix2F>   FillMatrixCache;
    // Gradient image references for lifetime control (AddRef)
    ArrayLH<Ptr<Image> > GradientImages;
};



//--------------------------------------------------------------------
// ***** Mask Primitive

// MaskPrimitive is used for Push mask operations that specify mask coverage area
// vy including bounding box mask rectangles.
// Mask rectangles are represented at matrices used to map a unit rectangle {0,0, 1,1} to
// the mask area bounds with rotation - these are the areas that should be erased
// before masks are applied.

class MaskPrimitive : public RefCountBase<MaskPrimitive, StatRender_Primitive_Mem>,
                      public RenderQueueItem::Interface    
{
public:
    enum MaskAreaType
    {
        Mask_Combinable,
        Mask_Clipped
    };

    MaskPrimitive(HAL* hal, MaskAreaType type) : pHAL(hal), Type(type) { }

    MaskAreaType    GetType() const   { return Type; }
    bool            IsClipped() const { return Type == Mask_Clipped; }

    UPInt           GetMaskCount() const { return MaskAreas.GetSize(); }
    const HMatrix*  GetMaskAreaMatrices() const { return &MaskAreas[0]; }
    const HMatrix&  GetMaskAreaMatrix(UPInt index) const { return MaskAreas[index]; }

    void            Insert(UPInt index, const HMatrix& m);
    void            Remove(UPInt index, UPInt count);

    // RenderQueueItem::Interface impl
    virtual void    EmitToHAL(RenderQueueItem&, RenderQueueProcessor&);

private:
    HAL*             pHAL;
    MaskAreaType     Type;
    ArrayLH<HMatrix> MaskAreas;
};


}} // Scaleform::Render

#endif
