/**********************************************************************

Filename    :   Render_TextMeshProvider.cpp
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

#include "Render_TextMeshProvider.h"
#include "Render_TextPrimitiveBundle.h"
#include "Render_GlyphCache.h"
#include "Render_TessGen.h"
#include "Render_FontCacheHandle.h"
#include "Renderer2DImpl.h"

namespace Scaleform { namespace Render {


//------------------------------------------------------------------------
TextMeshProvider::TextMeshProvider(GlyphCache* cache) 
    : pCache(cache), Flags(0), PinCount(0), pBundle(0), pBundleEntry(0),
      HeightRatio(0), ClipBox(0,0,0,0)
{}

//------------------------------------------------------------------------
TextMeshProvider::~TextMeshProvider()
{  
    Clear();
}

//------------------------------------------------------------------------
void TextMeshProvider::ClearEntries()
{
    for(UPInt i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->RemoveNotifier(Notifiers[i]);
    }
    Notifiers.ClearAndRelease();
    Entries.ClearAndRelease();
    Flags &= ~BF_Created;
}

//------------------------------------------------------------------------
void TextMeshProvider::Clear()
{
    //SF_ASSERT(!IsInList()); // Strict verification

    // Permissive verification
    //---------------------
    if (IsInList())
    {
        SetInUseList(false);
        if (IsInPinList())
        {
            SetInPinList(false);
            UnpinSlots();
        }
        RemoveNode();
    }
    //---------------------

    if (pBundle)
    {
        pBundle->removeEntryFromLayers(pBundleEntry);
        ClearBundle();
    }

    ClearEntries();
    Entries.ClearAndRelease();
    Layers.ClearAndRelease();
}


//------------------------------------------------------------------------
// Call-back from Renderer that performs a "delayed pin" of the batch
void TextMeshProvider::AddToInUseList()
{
    //SF_ASSERT(!IsInList()); // Strict verification

    if (Flags & (BF_InUseList | BF_InPinList)) // Permissive verification 
        return;    
    SetInUseList(true);
    pCache->AddToInUseList(this);
}




// GlyphQueue interaction functions
//
//------------------------------------------------------------------------
// Call-back from GlyphQueue, when some glyph in the batch is evicted and released.
void TextMeshProvider::OnEvictSlots()
{
    SF_ASSERT(!IsInList());
    SF_ASSERT((Flags & BF_Creating) == 0);

    //if (Creating)
    //{
    //    ClearEntries();
    //    if (pBundle)
    //    {
    //        pBundle->removeEntryFromLayers(pBundleEntry);
    //        ClearBundle();
    //    }
    //}
    //else
    {
        Clear();
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::AddNotifier(TextNotifier* notifier)
{
    if (notifier)
    {
        Notifiers.PushBack(notifier);
        pCache->PinSlot(notifier->pSlot);
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::PinSlots()
{
    SF_ASSERT(!IsInList());
    for(UPInt i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->PinSlot(Notifiers[i]->pSlot);
    }
}

//------------------------------------------------------------------------
void TextMeshProvider::UnpinSlots()
{
    SF_ASSERT(!IsInList());
    for(UPInt i = 0; i < Notifiers.GetSize(); ++i)
    {
        pCache->UnpinSlot(Notifiers[i]->pSlot);
    }
}



//------------------------------------------------------------------------
MeshUseStatus TextMeshProvider::GetMeshUseStatus() const
{
    MeshUseStatus status = MUS_Uncached;
    if (PinCount)
        return MUS_Pinned;
    for (UPInt i=0; i< GetLayerCount(); i++)
    {
        const TextMeshLayer& l = GetLayer(i);
        if (l.Type > TextLayer_RasterText)
            break;
        if ((l.Type >= TextLayer_Shadow) &&  l.pMesh)
        {
            MeshUseStatus s = l.pMesh->GetUseStatus();
            if (s > status)
                status = s;
        }
    }
    return status;
}


//------------------------------------------------------------------------
void TextMeshProvider::GetFillData(FillData* data, unsigned layer,
                                   unsigned fillIndex, unsigned meshGenFlags)
{
    SF_UNUSED(fillIndex);
    if (meshGenFlags & Mesh_Mask)

    {
        *data = FillData(Fill_Mask);
        return;
    }

    switch(Layers[layer].Type)
    {
    case TextLayer_Background:
    case TextLayer_Selection:
    case TextLayer_Shapes:
    case TextLayer_Underline:
    case TextLayer_Cursor:
    case TextLayer_Shapes_Masked:
    case TextLayer_Underline_Masked:
        {
            *data = FillData(Fill_VColor);
            data->PrimFill = PrimFill_VColor;
            data->pVFormat = &VertexXY16iCF32::Format;
            break;
        }

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        {
            Image* img = pCache->GetImage(Entries[Layers[layer].Start].TextureId);
            *data = FillData(img, ImageFillMode(Wrap_Clamp, Sample_Linear));
            data->PrimFill = PrimFill_UVTextureAlpha_VColor;
            data->pVFormat = &RasterGlyphVertex::Format;
            break;
        }

    case TextLayer_Images:
        {
            *data = FillData(Fill_VColor); // TO DO
            break;
        }

    case TextLayer_Mask:
        {
            *data = FillData(Fill_Mask);
            break;
        }
    }
}


static UInt8 Factors[4] = {0, 0x7F, 0xFF, 0xFF};


//------------------------------------------------------------------------
void TextMeshProvider::setMeshData(TessBase* tess, VertexOutput* verOut, const UInt32* colors, VertexCountType& verCount)
{
    enum { ChunkSize = 64 };
    TessVertex tessVer[ChunkSize];
    VertexXY16iCF32 vertices[ChunkSize];
    UInt16 tri[ChunkSize * 3];

    TessMesh tessMesh;
    tess->GetMesh(0, &tessMesh);

    unsigned retVer;
    while((retVer = tess->GetVertices(&tessMesh, tessVer, ChunkSize)) > 0)
    {
        unsigned j;
        for(j = 0; j < retVer; ++j)
        {
            const TessVertex& tv = tessVer[j];
            VertexXY16iCF32&  mv = vertices[j];
            mv.x = (SInt16)floorf((tv.x < 0) ? tv.x-0.5f : tv.x+0.5f);
            mv.y = (SInt16)floorf((tv.y < 0) ? tv.y-0.5f : tv.y+0.5f);

            UInt32 c1, c2;
            if (TessStyleMixesColors(tv.Flags))
            {
                c1 = colors[tv.Styles[0]-1];
                c2 = colors[tv.Styles[1]-1];
                mv.Color = ((c1 & 0xFEFEFEFE) >> 1) | ((c2 & 0xFEFEFEFE) >> 1);
            }
            else
            {
                mv.Color = colors[tv.Styles[TessGetUsedStyle(tv.Flags)]-1];
            }
            mv.Factors[0] = Factors[TessGetAlpha(tv.Flags)];
            mv.Factors[1] = Factors[TessGetFactor(tv.Flags)];
        }
        verOut->SetVertices(0, verCount.VStart, vertices, retVer);
        verCount.VStart += retVer;
    }

    unsigned start = 0;
    unsigned nTri = tess->GetMeshTriangleCount(0);
    while (start < nTri)
    {
        unsigned retTri = ChunkSize;
        if (start + retTri > nTri)
        {
            retTri = nTri - start;
            if (retTri == 0) break;
        }
        tess->GetTrianglesI16(0, tri, start, retTri);
        verOut->SetIndices(0, verCount.IStart*3, tri, retTri*3);
        start += retTri;
        verCount.IStart += retTri;
    }
}


//------------------------------------------------------------------------
bool TextMeshProvider::generateSelection(Renderer2DImpl* ren, VertexOutput* verOut, 
                                         const TextMeshLayer& layer, const Matrix2F& mtx)
{
    MeshGenerator* gen = ren->GetMeshGen();
    gen->Clear();
    gen->mTess.SetFillRule(Tessellator::FillNonZero);
    gen->mTess.SetToleranceParam(ren->GetToleranceParams());

    ArrayStaticBuffPOD<UInt32, 16> colors(Memory::GetHeapByAddress(this));

    // TO DO: Process EdgeAA settings
    gen->mTess.SetEdgeAAWidth(0.5f);

    for(unsigned i = 0; i < layer.Count; ++i)
    {
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const TextMeshEntry::Selection& sel = ent.EntryData.SelectionData;
        RectF r(sel.Coord[0], sel.Coord[1], sel.Coord[2], sel.Coord[3]);

        if (Flags & BF_Clip)
            r.Intersect(ClipBox);

        if (!r.IsEmpty())
        {
            float x, y;
            x = r.x1; y = r.y1; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x2; y = r.y1; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x2; y = r.y2; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
            x = r.x1; y = r.y2; mtx.Transform(&x, &y); gen->mTess.AddVertex(x, y);
        }
        colors.PushBack(ent.mColor);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, i+1, false, false);
    }

    gen->mTess.Tessellate(false);
    gen->mTess.Transform(mtx.GetInverse());
    Matrix m16i = gen->mTess.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

    VertexOutput::Fill vfill = { gen->mTess.GetMeshVertexCount(0), 
                                 gen->mTess.GetMeshTriangleCount(0) * 3, 
                                 &VertexXY16iCF32::Format, 0, 0, 0 };

    if (!verOut->BeginOutput(&vfill, 1, m16i))
        return false;

    VertexCountType verCount;
    setMeshData(&gen->mTess, verOut, &colors[0], verCount);

    verOut->EndOutput();
    gen->Clear();
    return true;
}



struct CornerVertex
{
    float x, y;
    CornerVertex() {}
    CornerVertex(float x_, float y_) : x(x_), y(y_) {}
};

//------------------------------------------------------------------------
template<class VertexType> 
static void calcMiter(const VertexType v0, const VertexType v1, const VertexType v2, 
                      float width, float* x, float* y)
{
    float len1 = Math2D::Distance(v0, v1);
    float len2 = Math2D::Distance(v1, v2);
    float dx1 = width * (v1.y - v0.y) / len1;
    float dy1 = width * (v0.x - v1.x) / len1;
    float dx2 = width * (v2.y - v1.y) / len2;
    float dy2 = width * (v1.x - v2.x) / len2;
    *x  = v1.x;
    *y  = v1.y;
    Math2D::Intersection(v0.x+dx1, v0.y+dy1, v1.x+dx1, v1.y+dy1,
                         v1.x+dx2, v1.y+dy2, v2.x+dx2, v2.y+dy2,
                         x, y, 0.001f);
}



//------------------------------------------------------------------------
bool TextMeshProvider::generateRectangleAA(Renderer2DImpl* ren, VertexOutput* verOut, 
                                           const Matrix2F& mtx, const float* coord, 
                                           UInt32 fillColor, UInt32 borderColor)
{
    UInt32 colors[2] = { fillColor, borderColor };
    MeshGenerator* gen = ren->GetMeshGen();
    gen->Clear();
    gen->mTess.SetFillRule(Tessellator::FillNonZero);
    gen->mTess.SetToleranceParam(ren->GetToleranceParams());

    // TO DO: Process EdgeAA settings
    gen->mTess.SetEdgeAAWidth(0.5f);

    float coordTr[8] = 
    {
        coord[0], coord[1],
        coord[2], coord[1],
        coord[2], coord[3],
        coord[0], coord[3]
    };
    mtx.Transform(coordTr+0, coordTr+1);
    mtx.Transform(coordTr+2, coordTr+3);
    mtx.Transform(coordTr+4, coordTr+5);
    mtx.Transform(coordTr+6, coordTr+7);

    if (fillColor)
    {
        gen->mTess.AddVertex(coordTr[0], coordTr[1]);
        gen->mTess.AddVertex(coordTr[2], coordTr[3]);
        gen->mTess.AddVertex(coordTr[4], coordTr[5]);
        gen->mTess.AddVertex(coordTr[6], coordTr[7]);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 1, false, false);
    }

    if (borderColor)
    {
        gen->mTess.AddVertex(coordTr[0], coordTr[1]);
        gen->mTess.AddVertex(coordTr[2], coordTr[3]);
        gen->mTess.AddVertex(coordTr[4], coordTr[5]);
        gen->mTess.AddVertex(coordTr[6], coordTr[7]);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 2, false, false);

        float x, y;
        CornerVertex v1(coordTr[0], coordTr[1]);
        CornerVertex v2(coordTr[2], coordTr[3]);
        CornerVertex v3(coordTr[4], coordTr[5]);
        CornerVertex v4(coordTr[6], coordTr[7]);
        calcMiter(v2, v1, v4, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v1, v4, v3, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v4, v3, v2, 1, &x, &y); gen->mTess.AddVertex(x, y);
        calcMiter(v3, v2, v1, 1, &x, &y); gen->mTess.AddVertex(x, y);
        gen->mTess.ClosePath();
        gen->mTess.FinalizePath(0, 2, false, false);
    }

    gen->mTess.Tessellate(false);
    gen->mTess.Transform(mtx.GetInverse());
    Matrix m16i = gen->mTess.StretchTo(-32768+4, -32768+4, 32768-4, 32768-4);

    VertexOutput::Fill vfill = { gen->mTess.GetMeshVertexCount(0), 
                                 gen->mTess.GetMeshTriangleCount(0) * 3, 
                                 &VertexXY16iCF32::Format, 0, 0, 0 };

    if (!verOut->BeginOutput(&vfill, 1, m16i))
        return false;

    VertexCountType verCount;
    setMeshData(&gen->mTess, verOut, colors, verCount);

    verOut->EndOutput();
    gen->Clear();
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::clipGlyphRect(RectF& chr, RectF& tex) const
{
    if (Flags & BF_Clip)
    {
        RectF clippedChr = chr;
        clippedChr.Intersect(ClipBox);
        if (clippedChr.IsEmpty())
        {
            chr = ClipBox;
            chr.x2 = chr.x1; chr.y2 = chr.y1;
            tex.x2 = tex.x1; tex.y2 = tex.y1;
            return false;
        }

        if (clippedChr != chr)
        {
            RectF clippedTex = tex;

            if (chr.x1 != clippedChr.x1)
                clippedTex.x1 = tex.x1 + (clippedChr.x1 - chr.x1) * tex.Width() / chr.Width();

            if (chr.y1 != clippedChr.y1)
                clippedTex.y1 = tex.y1 + (clippedChr.y1 - chr.y1) * tex.Height() / chr.Height();

            if (chr.x2 != clippedChr.x2)
                clippedTex.x2 = tex.x2 - (chr.x2 - clippedChr.x2) * tex.Width() / chr.Width();

            if (chr.y2 != clippedChr.y2)
                clippedTex.y2 = tex.y2 - (chr.y2 - clippedChr.y2) * tex.Height() / chr.Height();

            chr = clippedChr;
            tex = clippedTex;
        }
    }
    return true;
}


//------------------------------------------------------------------------
bool TextMeshProvider::generateRasterMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx)
{
//printf("GetData %f\n", mtx.Sx());

    enum { GlyphChunkSize = 64 };

    RasterGlyphVertex vertices[GlyphChunkSize * 4];
    UInt16 indices[GlyphChunkSize * 6];

    unsigned totalGlyphs = layer.Count;
    VertexOutput::Fill vfill = { 4 * totalGlyphs, 6 * totalGlyphs, &RasterGlyphVertex::Format, 0, 0, 0 };
    float scaleU = pCache->GetScaleU();
    float scaleV = pCache->GetScaleV();

    if (!verOut->BeginOutput(&vfill, 1))
        return false;

    unsigned i = 0;
    unsigned glyphCount = 0;
    unsigned glyphOffset = 0;
    for(i = 0; i < totalGlyphs; ++i)
    {
        if(glyphCount >= GlyphChunkSize)
        {
            verOut->SetVertices(0, glyphOffset*4, vertices, GlyphChunkSize * 4);
            verOut->SetIndices(0, glyphOffset*6, indices, GlyphChunkSize * 6);
            glyphCount = 0;
            glyphOffset += GlyphChunkSize;
        }
        const TextMeshEntry& ent = Entries[i+layer.Start];
        const TextMeshEntry::GlyphRaster& ras = ent.EntryData.RasterData;
        const GlyphRect& r = ras.Glyph->mRect;
        UInt32 c = ent.mColor;

        unsigned verIdx = glyphCount*4;
        unsigned indIdx = glyphCount*6;
        unsigned idx = i*4;
        RectF chr(ras.Coord[0], ras.Coord[1], ras.Coord[2], ras.Coord[3]);
        float texX1 = (r.x + 1) * scaleU;
        float texY1 = (r.y + 1) * scaleV;
        float texX2 = texX1 + (r.w - 2) * scaleU;
        float texY2 = texY1 + (r.h - 2) * scaleV;
        RectF tex(texX1, texY1, texX2, texY2);

        clipGlyphRect(chr, tex);

        vertices[verIdx+0].Set(chr.x1, chr.y1, tex.x1, tex.y1, c);
        vertices[verIdx+1].Set(chr.x2, chr.y1, tex.x2, tex.y1, c);
        vertices[verIdx+2].Set(chr.x2, chr.y2, tex.x2, tex.y2, c);
        vertices[verIdx+3].Set(chr.x1, chr.y2, tex.x1, tex.y2, c);
        indices[indIdx+0] = UInt16(idx+0);
        indices[indIdx+1] = UInt16(idx+1);
        indices[indIdx+2] = UInt16(idx+2);
        indices[indIdx+3] = UInt16(idx+2);
        indices[indIdx+4] = UInt16(idx+3);
        indices[indIdx+5] = UInt16(idx+0);
        ++glyphCount;
    }

    if(glyphCount)
    {
        verOut->SetVertices(0, glyphOffset*4, vertices, glyphCount * 4);
        verOut->SetIndices(0, glyphOffset*6, indices, glyphCount * 6);
    }

    verOut->EndOutput();
    return true;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateImageMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx)
{
    return false;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateVectorMesh(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx)
{
    return false;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateUnderlines(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx)
{
    return false;
}

//------------------------------------------------------------------------
bool TextMeshProvider::generateMask(Renderer2DImpl* ren, VertexOutput* verOut, const TextMeshLayer& layer, const Matrix2F& mtx)
{
    return false;
}



//------------------------------------------------------------------------
bool TextMeshProvider::GetData(MeshBase *mesh, VertexOutput* verOut, unsigned meshGenFlags)
{
    unsigned        layerIdx = mesh->GetLayer();
    Renderer2DImpl* ren = mesh->GetRenderer();
    const Matrix2F& mtx = mesh->GetViewMatrix(); // tbd: Better passed as argument.
    const TextMeshLayer& layer = GetLayer(layerIdx);
    TextMeshEntry*  ent = &Entries[layer.Start];

    switch(layer.Type)
    {
    case TextLayer_Background:
        return generateRectangleAA(ren, verOut, mtx, ent->EntryData.BackgroundData.Coord, 
            ent->mColor, ent->EntryData.BackgroundData.BorderColor);

    case TextLayer_Selection:
        return generateSelection(ren, verOut, layer, mtx);

    case TextLayer_Shadow:
    case TextLayer_ShadowText:
    case TextLayer_RasterText:
        return generateRasterMesh(ren, verOut, layer, mtx);

    case TextLayer_Images:
        return generateImageMesh(ren, verOut, layer, mtx);

    case TextLayer_Shapes:
    case TextLayer_Shapes_Masked:
        return generateVectorMesh(ren, verOut, layer, mtx);

    case TextLayer_Underline:
    case TextLayer_Underline_Masked:
        return generateUnderlines(ren, verOut, layer, mtx);

    case TextLayer_Cursor:
        return generateRectangleAA(ren, verOut, mtx, ent->EntryData.SelectionData.Coord, ent->mColor, 0);

    case TextLayer_Mask: // Mask rectangle shape.
        return generateMask(ren, verOut, layer, mtx);
    }

    return false;
}









//------------------------------------------------------------------------
void TextMeshProvider::addRasterGlyph(TextLayerType type, UInt32 color, UInt16 textureId, const RectF& rect, const GlyphNode* node)
{
    TextMeshEntry e;
    e.LayerType = (UInt16)type;
    e.TextureId = textureId;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.RasterData.Coord[0] = rect.x1;
    e.EntryData.RasterData.Coord[1] = rect.y1;
    e.EntryData.RasterData.Coord[2] = rect.x2;
    e.EntryData.RasterData.Coord[3] = rect.y2;
    e.EntryData.RasterData.Glyph = node;
    Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addVectorGlyph(UInt32 color, const Font* font, UInt16 glyphIndex, UInt16 hintedSize, 
                                      UInt16 flags, UInt16 outline, float x, float y)
{
    TextMeshEntry e;
    e.LayerType = TextLayer_Shapes;
    e.TextureId = 0;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.VectorData.pFont      = font;
    e.EntryData.VectorData.GlyphIndex = glyphIndex;
    e.EntryData.VectorData.HintedSize = hintedSize;
    e.EntryData.VectorData.Flags      = flags;
    e.EntryData.VectorData.Outline    = outline;
    e.EntryData.VectorData.x          = x;
    e.EntryData.VectorData.y          = y;
    Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addBackground(UInt32 color, UInt32 borderColor, const RectF& rect)
{
    TextMeshEntry e;
    e.LayerType = TextLayer_Background;
    e.TextureId = 0;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.BackgroundData.Coord[0] = rect.x1;
    e.EntryData.BackgroundData.Coord[1] = rect.y1;
    e.EntryData.BackgroundData.Coord[2] = rect.x2;
    e.EntryData.BackgroundData.Coord[3] = rect.y2;
    e.EntryData.BackgroundData.BorderColor = borderColor;
    Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addSelection(UInt32 color, const RectF& rect)
{
    TextMeshEntry e;
    e.LayerType = TextLayer_Selection;
    e.TextureId = 0;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.SelectionData.Coord[0] = rect.x1;
    e.EntryData.SelectionData.Coord[1] = rect.y1;
    e.EntryData.SelectionData.Coord[2] = rect.x2;
    e.EntryData.SelectionData.Coord[3] = rect.y2;
    Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addCursor(UInt32 color, const RectF& rect)
{
    TextMeshEntry e;
    e.LayerType = TextLayer_Cursor;
    e.TextureId = 0;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.SelectionData.Coord[0] = rect.x1;
    e.EntryData.SelectionData.Coord[1] = rect.y1;
    e.EntryData.SelectionData.Coord[2] = rect.x2;
    e.EntryData.SelectionData.Coord[3] = rect.y2;
    Entries.PushBack(e);
}

//------------------------------------------------------------------------
void TextMeshProvider::addUnderline(UInt32 color, TextUnderlineStyle style, float x, float y, float len)
{
    TextMeshEntry e;
    e.LayerType = TextLayer_Underline;
    e.TextureId = 0;
    e.EntryIdx  = Entries.GetSize();
    e.mColor    = color;
    e.EntryData.UnderlineData.Style = style;
    e.EntryData.UnderlineData.x     = x;
    e.EntryData.UnderlineData.y     = y;
    e.EntryData.UnderlineData.Len   = len;
}

//------------------------------------------------------------------------
bool TextMeshProvider::CmpEntries::operator() (const TextMeshEntry& a, const TextMeshEntry& b) const
{
    if (a.LayerType != b.LayerType) return a.LayerType < b.LayerType;
    if (a.TextureId != b.TextureId) return pCache->CmpFills(a.TextureId, b.TextureId) < 0;
    return a.EntryIdx < b.EntryIdx;
}


//------------------------------------------------------------------------
void TextMeshProvider::addLayer(TextLayerType type, unsigned start, unsigned count)
{
    TextMeshLayer layer;
    layer.Type = type;
    layer.Start = start;
    layer.Count = count;
    layer.pMesh = 0;
    layer.pFill = pCache->GetFill(type, Entries[start].TextureId);
    Layers.PushBack(layer);
}

//------------------------------------------------------------------------
void TextMeshProvider::addLayer(UPInt start, UPInt end)
{
    // Merge shadow and text layers if possible
    if (Entries[start].LayerType == TextLayer_RasterText && Layers.GetSize())
    {
        TextMeshLayer& prevLayer = Layers.Back();
        if ((prevLayer.Type == TextLayer_Shadow || prevLayer.Type == TextLayer_ShadowText) &&
            Entries[start].TextureId == Entries[prevLayer.Start].TextureId)
        {
            prevLayer.Type  = TextLayer_ShadowText;
            prevLayer.Count += end - start;
            return;
        }
    }

    TextMeshLayer layer;

    // Use separate layers for vector Shapes 
    if (Entries[start].LayerType == TextLayer_Shapes)
    {
        for(UPInt i = start; i < end; ++i)
            addLayer(TextLayer_Shapes, i, 1);
        return;
    }
    addLayer((TextLayerType)Entries[start].LayerType, start, end - start);
}


//------------------------------------------------------------------------
void TextMeshProvider::sortEntries()
{
    Layers.Clear();
    CmpEntries cmp(pCache);
    Alg::QuickSort(Entries, cmp);

    UPInt start = 0;
    UPInt end = 0;
    for(; end < Entries.GetSize(); ++end)
    {
        if (Entries[start].LayerType != Entries[end].LayerType || 
            Entries[start].TextureId != Entries[end].TextureId)
        {
            if (end > start)
            {
                addLayer(start, end);
                start = end;
            }
        }
    }
    if (end > start)
    {
        addLayer(start, end);
    }
}






//------------------------------------------------------------------------
float TextMeshProvider::calcHeightRatio(const Matrix2F& matrix)
{
    Matrix2F mat2(matrix);
    mat2.Tx() = 0;
    mat2.Ty() = 0;
    float x01=0, y01=1, x10=1, y10=0;
    mat2.Transform(&x01, &y01);
    mat2.Transform(&x10, &y10);
    return fabsf(Math2D::LinePointDistance(0, 0, x10, y10, x01, y01));
}

//------------------------------------------------------------------------
bool TextMeshProvider::NeedsUpdate(const Matrix2F& matrix) const
{
    // TO DO: make a decision depending on the flags such ad OptRead 
    float k1 = 0.85f;
    float k2 = 1.0f/k1;
    float heightRatio = calcHeightRatio(matrix);
    return heightRatio < HeightRatio * k1 || heightRatio > HeightRatio * k2;
}

/*
//------------------------------------------------------------------------
bool TextMeshProvider::hasRasterGlyph(GlyphRunData& data, unsigned glyphIndex, float screenSize) const
{
    data.HintedSize = 0;
    if (data.pFont->pFont)
    {
        if (data.pFont->pFont->IsHintedRasterGlyph(glyphIndex, unsigned(screenSize)))
        {
            data.HintedSize = unsigned(screenSize);
            return true;
        }
    }
    return false;
}
*/


/*
//------------------------------------------------------------------------
bool TextMeshProvider::glyphFits(GlyphRunData& data, const GlyphParam& param, float screenSize) const
{
    data.RasterHeight = 0;
    if (param.BlurX || param.BlurY)
         return true;

    if (!param.IsOptRead())
        screenSize = pCache->SnapFontSizeToRamp(screenSize);

    unsigned h = (ceilf(data.GlyphBounds.y2 * screenSize / data.NomHeight) -
                 floorf(data.GlyphBounds.y1 * screenSize / data.NomHeight)) + 1;

    return pCache->GlyphFits(h);
}
*/



//------------------------------------------------------------------------
void TextMeshProvider::addRasterGlyph(TextLayerType type, GlyphRunData& data, UInt32 color, GlyphNode* node, float screenSize)
{
    UInt16 textureId = pCache->GetTextureId(node);
    const GlyphRect& r = node->mRect;
    RectF rect;
    rect.x1 = (float(node->Origin.x) / 16.0f) + 1.0f;
    rect.y1 = (float(node->Origin.y) / 16.0f) + 1.0f;
    rect.x2 = rect.x1 + r.w - 2.0f;
    rect.y2 = rect.y1 + r.h - 2.0f;

    float stretch = 1.0f; // TO DO: Process stretch
    float k = (node->Scale * screenSize / node->Param.GetFontSize()) / data.HeightRatio; // TO DO: Revise. Temporary solution for big glyphs
//k=1;
    rect.x1 *= k / stretch;
    rect.y1 *= k;
    rect.x2 *= k / stretch;
    rect.y2 *= k;

    float offsetX = 0;
    float offsetY = 0;
    if (type == TextLayer_Shadow)
    {
        offsetX = data.Param.ShadowOffsetX;
        offsetY = data.Param.ShadowOffsetY;
    }

    rect.x1 += data.NewLineX + offsetX;
    rect.y1 += data.NewLineY + offsetY;
    rect.x2 += data.NewLineX + offsetX;
    rect.y2 += data.NewLineY + offsetY;
    addRasterGlyph(type, color, textureId, rect, node);
}


//------------------------------------------------------------------------
bool TextMeshProvider::addGlyph(GlyphRunData& data, unsigned glyphIndex)
{
    bool needVector = false;
    float screenSize = data.FontSize * data.HeightRatio;
    GlyphParam gp;
    GlyphNode* node;

    gp = data.Param.TextParam;
    gp.pFont = data.pFont;
    gp.GlyphIndex = glyphIndex;
    gp.SetFontSize(pCache->GetCachedFontSize(gp, screenSize, data.pFont->pFont->IsRasterOnly()));
    node = pCache->FindGlyph(this, gp);
    if (node)
    {
        addRasterGlyph(TextLayer_RasterText, data, data.mColor, node, screenSize);
    }
    else
    {
        node = pCache->RasterizeGlyph(data, this, gp);
        if (node)
        {
//printf("R");
            addRasterGlyph(TextLayer_RasterText, data, data.mColor, node, screenSize);
        }
        else
        {
            GlyphCache::RasResult res = pCache->GetRasResult();

            if (res == GlyphCache::Res_ShapeNotFound)
            {
                // TO DO: Draw square
                return true;
            }

            if (res == GlyphCache::Res_ShapeIsEmpty)
                return true;

            if (res == GlyphCache::Res_CacheFull)
                return false;

            if (res == GlyphCache::Res_ShapeIsTooBig)
                needVector = true;
        }
    }

    if (needVector)
    {
        // add vector glyph
    }

    if (data.Param.ShadowColor)
    {
        UInt32 color = data.Param.ShadowColor;
        gp = data.Param.ShadowParam;
        gp.pFont = data.pFont;
        gp.SetFontSize(pCache->GetCachedShadowSize(gp, screenSize));
        gp.GlyphIndex = glyphIndex;
        node = pCache->FindGlyph(this, gp);
        if (node)
        {
            // add raster shadow
            addRasterGlyph(TextLayer_Shadow, data, color, node, screenSize);
        }
        else
        {
            node = pCache->RasterizeShadow(data, this, gp, screenSize);
            if (node)
            {
//printf("R");
                // add raster shadow
                addRasterGlyph(TextLayer_Shadow, data, color, node, screenSize);
            }
            else
            {
                if (pCache->GetRasResult() == GlyphCache::Res_CacheFull)
                    return false;
            }
        }
    }
    return true;
}


//------------------------------------------------------------------------
float TextMeshProvider::snapX(const GlyphRunData& data) const
{
    float x = data.NewLineX;
    float y = data.NewLineY;
    data.DirMtx.Transform(&x, &y);
    x = floorf(x + 0.5f);
    data.InvMtx.Transform(&x, &y);
    return x;
}

//------------------------------------------------------------------------
float TextMeshProvider::snapY(const GlyphRunData& data) const
{
    float x = data.NewLineX;
    float y = data.NewLineY;
    data.DirMtx.Transform(&x, &y);
    y = floorf(y + 0.5f);
    data.InvMtx.Transform(&x, &y);
    return y;
}


//------------------------------------------------------------------------
bool TextMeshProvider::CreateMeshData(const TextLayout* layout, Renderer2DImpl* ren, const HMatrix& m, unsigned meshGenFlags)
{
    Flags &= ~BF_Created;
    for(int pass = 0; pass < 2; ++pass)
    {
        SF_ASSERT(Layers.GetSize() == 0);
        SF_ASSERT(Entries.GetSize() == 0);
        GlyphRunData data;
        data.Param      = layout->GetParam();
        data.Bounds     = layout->GetBounds();
        data.pFont      = 0;
        data.FontSize   = 0;
        data.HintedSize = 0;
        data.NomWidth   = 0;
        data.NomHeight  = 0;
        data.mColor     = 0;
        data.NewLineX   = 0;
        data.NewLineY   = 0;
        data.GlyphBounds= RectF(0,0,0,0);
        data.DirMtx     = m.GetMatrix2D();
        data.InvMtx     = data.DirMtx.GetInverse();
        data.HeightRatio= calcHeightRatio(data.DirMtx);

        HeightRatio = data.HeightRatio;

        Flags &= ~BF_Clip;
        ClipBox = layout->GetClipBox();
        if (!ClipBox.IsEmpty())
            Flags |= BF_Clip;

    //printf("Create %f\n", data.DirMtx.Sx());

        Font* font = 0;

        Flags |= BF_Creating;

        TextLayout::Record rec;
        UPInt pos = 0;
        Matrix2F viewMatrix = m.GetMatrix2D();
        bool cacheIsOK = true;
        while(cacheIsOK && (pos = layout->ReadNext(pos, &rec)) != 0)
        {
            switch(layout->GetRecordType(rec))
            {
            case TextLayout::Record_Char:
                SF_ASSERT(data.pFont != 0);
                if (data.pFont->pFont->IsRasterOnly()) // TO DO: Consider OptRead and other cases
                    data.NewLineX = snapX(data);
                cacheIsOK = addGlyph(data, rec.mChar.GlyphIndex);
                data.NewLineX += rec.mChar.Advance;
                break;

            case TextLayout::Record_Color:
                data.mColor = rec.mColor.mColor;
                break;

            case TextLayout::Record_Background:
                addBackground(rec.mBackground.BackgroundColor, rec.mBackground.BorderColor, data.Bounds);
                break;

            case TextLayout::Record_NewLine:
                data.NewLineX = rec.mLine.x;
                data.NewLineY = rec.mLine.y;
                if (data.pFont && data.pFont->pFont->IsRasterOnly()) // TO DO: Consider OptRead and other cases
                    data.NewLineY = snapY(data);
                break;

            case TextLayout::Record_Font:
                font = rec.mFont.pFont;
                data.pFont = pCache->RegisterFont(font);
                data.FontSize = rec.mFont.mSize;
                if (data.pFont->pFont->IsRasterOnly()) // TO DO: Consider OptRead and other cases
                    data.NewLineY = snapY(data);
                break;

            case TextLayout::Record_Selection:
                addSelection(rec.mSelection.mColor, 
                    RectF(rec.mSelection.x1, rec.mSelection.y1, rec.mSelection.x2, rec.mSelection.y2));
                break;

            case TextLayout::Record_Underline:
                addUnderline(rec.mUnderline.mColor, (TextUnderlineStyle)rec.mUnderline.Style, 
                    rec.mUnderline.x, rec.mUnderline.y, rec.mUnderline.Len);
                break;

            case TextLayout::Record_Cursor:
                addCursor(rec.mCursor.mColor, 
                    RectF(rec.mCursor.x1, rec.mCursor.y1, rec.mCursor.x2, rec.mCursor.y2));
                break;

            case TextLayout::Record_Image:
                // TO DO
                break;
            }
        }

        UnpinSlots();
        Flags &= ~BF_Creating;

        if (cacheIsOK)
        {
            sortEntries();
            for(UPInt i = 0; i < Layers.GetSize(); ++i)
            {
                TextMeshLayer& layer = Layers[i];
                layer.M = m;
                layer.pMesh = *SF_HEAP_AUTO_NEW(this)
                    Mesh(ren, this, viewMatrix, 0, (unsigned)i, meshGenFlags);
            }
            Flags |= BF_Created;
            break;
        }
        else
        {
            ClearEntries();
            if (pCache->CanReset())
            {
                pCache->ClearCache();
//printf("R"); // DBG
            }
            else
                break;
        }
    }

//pCache->PrintMemStats();

    return (Flags & BF_Created) != 0;
}



}} // Scaleform::Render

