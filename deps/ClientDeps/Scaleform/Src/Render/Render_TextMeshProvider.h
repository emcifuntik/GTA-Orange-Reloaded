/**********************************************************************

PublicHeader:   Render
Filename    :   Render_TextMeshProvider.h
Content     :   
Created     :   
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_Render_TextMeshProvider_H
#define INC_SF_Render_TextMeshProvider_H

#include "Kernel/SF_Types.h"
#include "Kernel/SF_List.h"
#include "Render_Types2D.h"
#include "Render_GlyphParam.h"
#include "Render_MatrixPool.h"
#include "Render_Primitive.h"
#include "Render_MeshCache.h"
#include "Render_TextLayout.h"
#include "Render_Font.h"
#include "Render_TessDefs.h"

namespace Scaleform { namespace Render {


class Texture;
class TextMeshProvider;
struct TextMeshEntry;
class GlyphCache;
struct GlyphNode;
class TextPrimitiveBundle;
class BundleEntry;
struct TextNotifier;


//------------------------------------------------------------------------
struct TextMeshEntry
{
    struct GlyphRaster
    {
        float            Coord[4];
        const GlyphNode* Glyph;
    };

    struct GlyphVector
    {
        const Font*     pFont;
        UInt16          GlyphIndex;
        UInt16          HintedSize;
        UInt16          Flags;
        UInt16          Outline;
        float           x, y;
    };

    struct Background
    {
        float   Coord[4];
        UInt32  BorderColor;
    };

    struct Selection
    {
        float   Coord[4];
    };

    struct Underline
    {
        unsigned Style;
        float    x,y;
        float    Len;
    };

    union EntryUnion
    {
        GlyphRaster RasterData;
        GlyphVector VectorData;
        Background  BackgroundData;
        Selection   SelectionData;
        Underline   UnderlineData;
    };

    UInt16      LayerType;  // Determines the order of drawing. 
    UInt16      TextureId;
    unsigned    EntryIdx;   // Entry index required for stable sorting 
    UInt32      mColor;
    EntryUnion  EntryData;
};


//------------------------------------------------------------------------
struct TextMeshLayer
{
    TextLayerType       Type;
    unsigned            Start;
    unsigned            Count;
    Ptr<Mesh>           pMesh;
    HMatrix             M;
    Ptr<PrimitiveFill>  pFill;
};


struct GlyphRunData
{
    TextFieldParam      Param;
    RectF               Bounds;
    FontCacheHandle*    pFont;
    float               FontSize;
    unsigned            HintedSize;
    float               NomWidth;
    float               NomHeight;
    UInt32              mColor;
    float               NewLineX;
    float               NewLineY;
    RectF               GlyphBounds;
    Matrix2F            DirMtx;
    Matrix2F            InvMtx;
    float               HeightRatio;
    unsigned            RasterHeight;
};


//------------------------------------------------------------------------
// TextMeshProvider is associated with one text field and holds its cached
// glyph batch records and their layers with meshes.
//  - Can NOT be RefCounted due to MeshProvider -> Mesh dependency,
//    for now, we just store it as value.
class TextMeshProvider : public ListNode<TextMeshProvider>, public MeshProvider
{
public:
    enum BatchFlags
    {
        BF_Valid     = 0x0001,
        BF_InUseList = 0x0002,
        BF_InPinList = 0x0004,

        BF_Clip      = 0x0008,
        BF_Creating  = 0x0010,
        BF_Created   = 0x0020
    };

    TextMeshProvider(GlyphCache* cache);
    ~TextMeshProvider();

    // Add entries
    void Clear();
    void ClearEntries();

    // Access elements
    const TextMeshLayer& GetLayer(unsigned idx) const { return Layers[idx]; }
    const TextMeshEntry& GetEntry(const TextMeshLayer& layer, unsigned idx) const
    {
        return Entries[layer.Start + idx];
    }

    // GlyphQueue and MeshCache interaction functions
    //-------------------------------
    void AddNotifier(TextNotifier* notifier);
    void SetInUseList(bool f) { if(f) Flags |= BF_InUseList; else Flags &= ~BF_InUseList; }
    void SetInPinList(bool f) { if(f) Flags |= BF_InPinList; else Flags &= ~BF_InPinList; }
    void ClearInList()        { Flags &= ~(BF_InUseList | BF_InPinList); } 

    bool IsInUseList() const  { return (Flags & (BF_InUseList | BF_InPinList)) == BF_InUseList; }
    bool IsInPinList() const  { return (Flags & (BF_InUseList | BF_InPinList)) == BF_InPinList; }
    bool IsInList()    const  { return (Flags & (BF_InUseList | BF_InPinList)) != 0; }

    void PinSlots();
    void UnpinSlots();
    void OnEvictSlots();

    bool IsInUse() const
    {
        return GetMeshUseStatus() >= MUS_InUse;
    }
    void AddToInUseList();

    bool NeedsUpdate(const Matrix2F& matrix) const;
    bool CreateMeshData(const TextLayout* layout, Renderer2DImpl* ren, const HMatrix& m, unsigned meshGenFlags);

    // MeshProvder is crated by CreateMeshData; reset by Clear.
    bool IsCreated() const { return (Flags & BF_Created) != 0; }

    //-------------------------------
    // MeshProvider Interface
    virtual unsigned    GetLayerCount() const { return (unsigned)Layers.GetSize(); }
    virtual bool        GetData(MeshBase *mesh, VertexOutput* out, unsigned meshGenFlags);

    virtual unsigned    GetFillCount(unsigned layer, unsigned meshGenFlags) const
    { SF_UNUSED2(layer, meshGenFlags); return 1; }

    virtual void        GetFillData(FillData* data, unsigned layer,
                                    unsigned fillIndex, unsigned meshGenFlags);

    virtual void        GetFillMatrix(MeshBase *mesh, Matrix2F* matrix, unsigned layer,
                                      unsigned fillIndex, unsigned meshGenFlags)
    {
        matrix->SetIdentity();
        SF_ASSERT(0); SF_UNUSED5(mesh, matrix, layer, fillIndex, meshGenFlags);
    }
    //-------------------------------


    // Pin layer batches; they are pinned while layers are being prepared,
    // they are unpinned once prepare is done (at which point they are held by Mesh status).
    void Pin()      { PinCount++; }
    void Unpin()    { PinCount--; }

    TextPrimitiveBundle* GetBundle() const { return pBundle; }
    void ClearBundle()                     { SetBundle(0, 0);}
    void SetBundle(TextPrimitiveBundle* bundle, BundleEntry* entry)
    {
        pBundle = bundle;
        pBundleEntry = entry;
    }

    MeshUseStatus  GetMeshUseStatus() const;
    bool           HasMask() const { return false; }

    // If batch uses masking, this is the mask clear bounds used
    // for cumulative clear; it should include the area of all shapes
    // and masks that stick out.
    // This must be a cached value, so that it can be searched for during removal.
    HMatrix GetMaskClearBounds() { return HMatrix(); }


private:
    struct CmpEntries
    {
        const GlyphCache* pCache;
        CmpEntries(GlyphCache* c) : pCache(c) {}
        bool operator() (const TextMeshEntry& a, const TextMeshEntry& b) const;
    };

    void addLayer(TextLayerType type, unsigned start, unsigned count);
    void addLayer(UPInt start, UPInt end);


    void addEntry(const TextMeshEntry& entry) { Entries.PushBack(entry); }

    // Add raster glyph. Type may be only TextLayer_Shadow or TextLayer_RasterText
    void addRasterGlyph(TextLayerType type, UInt32 color, UInt16 textureId, const RectF& rect, const GlyphNode* node);

    // Vector glyph (shapes)
    void addVectorGlyph(UInt32 color, const Font* font, UInt16 glyphIndex, UInt16 hintedSize, 
                        UInt16 flags, UInt16 outline, float x, float y);

    // Auxiliary entries
    void addBackground(UInt32 color, UInt32 borderColor, const RectF& rect);
    void addSelection(UInt32 color, const RectF& rect);
    void addCursor(UInt32 color, const RectF& rect);
    void addUnderline(UInt32 color, TextUnderlineStyle style, float x, float y, float len);

    void sortEntries();     // Sort entries and and count layers

    // Create mesh entries functions
    static float calcHeightRatio(const Matrix2F& matrix);
    //bool hasRasterGlyph(GlyphRunData& data, unsigned glyphIndex, float screenSize) const;
    //bool glyphFits(GlyphRunData& data, const GlyphParam& param, float screenSize) const;
    //bool addTextGlyph(GlyphRunData& data, unsigned glyphIndex, float screenSize, const ShapeDataInterface* shape);
    //bool addGlowGlyph(GlyphRunData& data, unsigned glyphIndex, float screenSize, const ShapeDataInterface* shape);

    void addRasterGlyph(TextLayerType type, GlyphRunData& data, UInt32 color, GlyphNode* node, float screenSize);
    bool addGlyph(GlyphRunData& data, unsigned glyphIndex);

    struct VertexCountType
    {
        unsigned VStart;
        unsigned IStart;
        VertexCountType() : VStart(0), IStart(0) {}
    };

    // Mesh generation functions
    void setMeshData(TessBase* tess, VertexOutput* verOut, const UInt32* colors, VertexCountType& verCount);
    bool generateRectangleAA(Renderer2DImpl* ren, VertexOutput* verOut, const Matrix2F& mtx, const float* coord, 
                             UInt32 fillColor, UInt32 borderColor);
    bool generateSelection(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);
    bool clipGlyphRect(RectF& chr, RectF& tex) const;
    bool generateRasterMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);
    bool generateImageMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);
    bool generateVectorMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);
    bool generateUnderlines(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);
    bool generateMask(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx);

    float snapX(const GlyphRunData& data) const;
    float snapY(const GlyphRunData& data) const;

    GlyphCache*                 pCache;
    unsigned                    Flags;
    ArrayLH_POD<TextNotifier*>  Notifiers;  // TO DO: Consider more efficient memory management
    ArrayLH_POD<TextMeshEntry>  Entries;
    ArrayLH    <TextMeshLayer>  Layers;

    // Data for TextPrimitiveBundle
    UPInt                PinCount;    
    // TBD: Pointer back to TextPrimitiveBundle to detect membership?
    TextPrimitiveBundle* pBundle;
    BundleEntry*         pBundleEntry;
    // Flag tracking creation status of MeshProvider object.
    float                HeightRatio;
    RectF                ClipBox;

};



}} // Scaleform::Render

#endif
