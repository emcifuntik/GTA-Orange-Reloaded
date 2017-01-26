/*********************************************************************

Filename    :   LineBuffer.cpp
Content     :   Text line buffer
Created     :   May, 2007
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "Kernel/SF_UTF8Util.h"

#include "Render/Render_Image.h"
#include "Render/Text/Text_LineBuffer.h"
#include "Kernel/SF_Alg.h"

namespace Scaleform { 
namespace Render {
namespace Text {

#ifdef SF_BUILD_DEBUG
//#define DEBUG_MASK
//#define DEBUG_LINE_WIDTHS
//#define DEBUG_VIS_RECT
#endif //SF_BUILD_DEBUG

LineBuffer::Line* LineBuffer::TextLineAllocator::AllocLine(unsigned size, LineBuffer::LineType lineType)
{
    //TODO: look for already allocated line in FreeLinesPool first
    Line* pline = (Line*)SF_HEAP_AUTO_ALLOC(this, size);// Was: GFxStatRender_Text_Mem);
    pline->SetMemSize(size);
    if (lineType == Line8)
        pline->InitLine8();
    else
        pline->InitLine32();
    return pline;
}

void LineBuffer::TextLineAllocator::FreeLine(LineBuffer::Line* ptr)
{
    //TODO: return line being freed in FreeLinesPool
    if (ptr)
    {
        ptr->Release();
        SF_FREE(ptr);
    }
}

void LineBuffer::ReleasePartOfLine(GlyphEntry* pglyphs, unsigned n, FormatDataEntry* pnextFormatData)
{
    for(unsigned i = 0; i < n; ++i, ++pglyphs)
    {
        GlyphEntry* pglyph = pglyphs;
        if (pglyph->IsNextFormat())
        {   
            if (pglyph->HasFmtFont())
            {
                FontHandle* pfont = pnextFormatData->pFont;
                pfont->Release();
                ++pnextFormatData;
            }
            if (pglyph->HasFmtColor())
            {
                ++pnextFormatData;
            }
            if (pglyph->HasFmtImage())
            {
                ImageDesc *pimage = pnextFormatData->pImage;
                pimage->Release();
                ++pnextFormatData;
            }
        }
    }
}

void LineBuffer::GlyphInserter::ResetTo(const GlyphInserter& savedPos)
{
    SF_ASSERT(savedPos.GlyphIndex <= GlyphIndex);
    if (savedPos.GlyphIndex < GlyphIndex && GlyphsCount > 0)
    {
        // first of all, we need to release all fonts and image descriptors
        // beyond the savedPos.GlyphIndex; otherwise memory leak is possible.
        
        GlyphEntry* pglyphs = savedPos.pGlyphs + savedPos.GlyphIndex;
        unsigned n = GlyphIndex - savedPos.GlyphIndex;
        FormatDataEntry* pnextFormatData 
            = reinterpret_cast<FormatDataEntry*>(savedPos.pNextFormatData + savedPos.FormatDataIndex);
        ReleasePartOfLine(pglyphs, n, pnextFormatData);
    }
    operator=(savedPos);
}

void LineBuffer::GlyphIterator::operator++()
{
    if (!IsFinished())
    {
        if (Delta == 0)
            Delta = pGlyphs->GetLength();
        ++pGlyphs;
        if (pGlyphs->GetLength() > 0 && Delta > 0 && !HighlighterIter.IsFinished())
        {
            HighlighterIter += Delta;
            Delta = 0;
        }
        UpdateDesc();
    }
}

void LineBuffer::GlyphIterator::UpdateDesc()
{
    pImage = NULL;
    if (!IsFinished())
    {
        if (pGlyphs->IsNextFormat())
        {   
            if (pGlyphs->HasFmtFont())
            {
                pFontHandle = pNextFormatData->pFont;
                ++pNextFormatData;
            }
            if (pGlyphs->HasFmtColor())
            {
                OrigColor = ColorV = pNextFormatData->ColorV;
                ++pNextFormatData;
            }
            if (pGlyphs->HasFmtImage())
            {
                pImage = pNextFormatData->pImage;
                ++pNextFormatData;
            }
        }
        if (pGlyphs->IsUnderline())
        {
            UnderlineStyle  = HighlightInfo::Underline_Single;
            UnderlineColor  = ColorV;
        }
        else
            UnderlineStyle = HighlightInfo::Underline_None;

        if (!HighlighterIter.IsFinished())
        {
            // get data from the highlighter
            const HighlightDesc& hd = *HighlighterIter;
            
            ColorV           = OrigColor;
            if ((pGlyphs->GetLength() > 0 || pGlyphs->IsWordWrapSeparator()))
            {
                if (hd.Info.HasTextColor())
                    ColorV = hd.Info.GetTextColor().ToColor32();

                if (hd.Info.HasUnderlineStyle())
                    UnderlineStyle = hd.Info.GetUnderlineStyle();
                if (hd.Info.HasUnderlineColor())
                    UnderlineColor = hd.Info.GetUnderlineColor().ToColor32();
                else
                    UnderlineColor  = ColorV;
                if (hd.Info.HasBackgroundColor())
                    SelectionColor = hd.Info.GetBackgroundColor().ToColor32();
                else
                    SelectionColor = 0;
            }
        }
        else
        {
            SelectionColor = 0;
            if (pGlyphs->IsUnderline())
            {
                UnderlineColor = ColorV;
                UnderlineStyle  = HighlightInfo::Underline_Single;
            }
        }
    }
}

LineBuffer::FormatDataEntry* LineBuffer::Line::GetFormatData() const 
{ 
    UByte* p = reinterpret_cast<UByte*>(GetGlyphs() + GetNumGlyphs());
    // align to boundary of FormatDataEntry
    UPInt pi = ((UPInt)p);
    UPInt delta = ((pi + sizeof(FormatDataEntry*) - 1) & ~(sizeof(FormatDataEntry*) - 1)) - pi;
    return reinterpret_cast<FormatDataEntry*>(p + delta);
}

void LineBuffer::Line::Release()
{
    if (!IsInitialized()) return;

    GlyphEntry* pglyphs = GetGlyphs();
    unsigned n = GetNumGlyphs();
    FormatDataEntry* pnextFormatData = GetFormatData();
    ReleasePartOfLine(pglyphs, n, pnextFormatData);
    SetNumGlyphs(0);
#ifdef SF_BUILD_DEBUG
    memset(((UByte*)this) + sizeof(MemSize), 0xf0, GetMemSize() - sizeof(MemSize));
#endif
}

bool LineBuffer::Line::HasNewLine() const
{
    unsigned n = GetNumGlyphs();
    if (n > 0)
    {
        GlyphEntry* pglyphs = GetGlyphs();
        return pglyphs[n - 1].IsNewLineChar() && !pglyphs[n - 1].IsEOFChar();
    }
    return false;
}

//////////////////////////////////
// LineBuffer
//
LineBuffer::LineBuffer()
{
//@REN    DummyStyle.SetColor(0xFFFFFFFF); // White, modified by CXForm.
    Geom.VisibleRect.Clear();
    LastHScrollOffset = ~0u;
}

LineBuffer::~LineBuffer()
{
    ClearLines();
}

LineBuffer::Line* LineBuffer::GetLine(unsigned lineIdx)
{
    if (lineIdx >= Lines.GetSize())
        return NULL;
    LineBuffer::Line* pline = Lines[lineIdx];
    InvalidateCache();
    return pline;
}

const LineBuffer::Line* LineBuffer::GetLine(unsigned lineIdx) const
{
    if (lineIdx >= Lines.GetSize())
        return NULL;
    const LineBuffer::Line* pline = Lines[lineIdx];
    return pline;
}

// @REN
/*
bool LineBuffer::DrawMask(DisplayContext &context, 
                          const Matrix2F& mat, 
                          const RectF& rect, 
                          Render::Renderer::SubmitMaskMode maskMode)
{
    Render::Renderer* prenderer = context.GetRenderer();

    // Only clear stencil if no masks were applied before us; otherwise
    // no clear is necessary because we are building a cumulative mask.
#ifndef DEBUG_MASK
    prenderer->BeginSubmitMask(maskMode);
#else
    SF_UNUSED(maskMode);
#endif

    Render::PointF                 coords[4];
    static const UInt16     indices[6] = { 0, 1, 2, 2, 1, 3 };

    // Show white background + black bounding box.
    //prenderer->SetCxform(cx); //?AB: do we need this for drawing mask?
#ifdef DEBUG_MASK
    Cxform colorCxform;
    Color color(255, 0, 0, 128);
    colorCxform.M[0][0] = (1.0f / 255.0f) * color.GetRed();
    colorCxform.M[1][0] = (1.0f / 255.0f) * color.GetGreen();
    colorCxform.M[2][0] = (1.0f / 255.0f) * color.GetBlue();
    colorCxform.M[3][0] = (1.0f / 255.0f) * color.GetAlpha();
    prenderer->SetCxform(colorCxform); //?AB: do we need this for drawing mask?
#endif
    Matrix m(mat);
    RectF newRect;
    Text::RecalculateRectToFit16Bit(m, rect, &newRect);
    prenderer->SetMatrix(m);

    coords[0] = newRect.TopLeft();
    coords[1] = newRect.TopRight();
    coords[2] = newRect.BottomLeft();
    coords[3] = newRect.BottomRight();

    Render::Renderer::VertexXY16i icoords[4];
    icoords[0].x = (SInt16) coords[0].x;
    icoords[0].y = (SInt16) coords[0].y;
    icoords[1].x = (SInt16) coords[1].x;
    icoords[1].y = (SInt16) coords[1].y;
    icoords[2].x = (SInt16) coords[2].x;
    icoords[2].y = (SInt16) coords[2].y;
    icoords[3].x = (SInt16) coords[3].x;
    icoords[3].y = (SInt16) coords[3].y;

    prenderer->FillStyleColor(Color(255, 255, 255, 255));
    prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);

    // Fill the inside
    prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
    prenderer->DrawIndexedTriList(0, 0, 4, 0, 2);

    // Done
    prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
    prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);

    // Done rendering mask.
#ifndef DEBUG_MASK
    prenderer->EndSubmitMask();
#endif
    return true;
}*/

unsigned LineBuffer::CalcLineSize
    (unsigned glyphCount, unsigned formatDataElementsCount, LineBuffer::LineType lineType)
{
    unsigned sz;
    if (lineType == Line8)
        sz = LineData8::GetLineStructSize();
    else
        sz = LineData32::GetLineStructSize();
    sz += sizeof(UInt32); // MemSize
    sz += sizeof(GlyphEntry) * glyphCount;
    // add alignment here
    sz = (sz + sizeof(FormatDataEntry) - 1) & ~(sizeof(FormatDataEntry) - 1);
    sz += sizeof(FormatDataEntry) * formatDataElementsCount;
    return sz;
}


LineBuffer::Line* LineBuffer::InsertNewLine
    (unsigned lineIdx, unsigned glyphCount, unsigned formatDataElementsCount, LineBuffer::LineType lineType)
{
    unsigned sz = CalcLineSize(glyphCount, formatDataElementsCount, lineType);
    Line* pline = LineAllocator.AllocLine(sz, lineType);
    if (!pline) return NULL;

    pline->SetNumGlyphs(glyphCount);
    Lines.InsertAt(lineIdx, pline);
    return pline;
}

void LineBuffer::RemoveLines(unsigned lineIdx, unsigned num)
{
    Iterator it = Begin() + lineIdx;
    for(unsigned i = 0; i < num && !it.IsFinished(); ++it, ++i)
    {
        LineAllocator.FreeLine(it.GetPtr());
    }
    Lines.RemoveMultipleAt(lineIdx, num);
}

unsigned   LineBuffer::GetVScrollOffsetInFixp() const
{
    int yOffset = 0;
    if (Geom.FirstVisibleLinePos != 0)
    {
        // calculate y-offset of first visible line
        ConstIterator visIt   = BeginVisible(0);
        ConstIterator linesIt = Begin();
        if (!visIt.IsFinished() && !linesIt.IsFinished())
        {
            yOffset = visIt->GetOffsetY() - linesIt->GetOffsetY();
            SF_ASSERT(yOffset >= 0);
        }
    }
    return unsigned(yOffset);
}

void LineBuffer::ResetCache() 
{ 
}


template<class Matrix>
static float GFx_CalcHeightRatio(const Matrix& matrix)
{
    Matrix mat2(matrix);
    mat2.x(0);
    mat2.y(0);
    Render::PointF p01 = mat2.Transform(Render::PointF(0,1));
    Render::PointF p10 = mat2.Transform(Render::PointF(1,0));
    return fabsf(Render::Math2D::LinePointDistance(0, 0, p10.x, p10.y, p01.x, p01.y));
}

#if defined(SF_BUILD_DEBUG) && (defined(DEBUG_LINE_WIDTHS) || defined(DEBUG_VIS_RECT))
static void DrawRect(Render::Renderer*  prenderer, const Matrix2F& mat, const RectF& rect, Color color)
{
    Cxform colorCxform;
    colorCxform.M[0][0] = (1.0f / 255.0f) * color.GetRed();
    colorCxform.M[1][0] = (1.0f / 255.0f) * color.GetGreen();
    colorCxform.M[2][0] = (1.0f / 255.0f) * color.GetBlue();
    colorCxform.M[3][0] = (1.0f / 255.0f) * color.GetAlpha();
    prenderer->SetCxform(colorCxform); //?AB: do we need this for drawing mask?

    Render::PointF                 coords[4];
    static const UInt16     indices[6] = { 0, 1, 2, 2, 1, 3 };

    Matrix2F m(mat);
    RectF newRect;
    RecalculateRectToFit16Bit(m, rect, &newRect);
    prenderer->SetMatrix(m);

    coords[0] = newRect.TopLeft();
    coords[1] = newRect.TopRight();
    coords[2] = newRect.BottomLeft();
    coords[3] = newRect.BottomRight();

    const SInt16  icoords[18] = 
    {
        // Strip (fill in)
        (SInt16) coords[0].x, (SInt16) coords[0].y,
        (SInt16) coords[1].x, (SInt16) coords[1].y,
        (SInt16) coords[2].x, (SInt16) coords[2].y,
        (SInt16) coords[3].x, (SInt16) coords[3].y
    };

    prenderer->FillStyleColor(Color(255, 255, 255, 255));
    prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);

    // Fill the inside
    prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
    prenderer->DrawIndexedTriList(0, 0, 4, 0, 2);

    // Done
    prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
    prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
}
#endif

//@REN
#ifdef d0
void    LineBuffer::Display(DisplayContext &context,
                                   const Matrix& matrix, const Cxform& cxform,
                                   bool nextFrame,
                                   const TextFieldParam& textFieldParam,
                                   const Highlighter* phighlighter)
{
    TextFieldParam param(textFieldParam);

// DBG
//param.ShadowColor = 0xC0FF0000;
//param.ShadowParam.SetBlurX(2.f);
//param.ShadowParam.SetBlurY(2.f);
//param.ShadowParam.SetBlurStrength(5.0f).
//param.ShadowParam.SetFineBlur(false);
//param.ShadowParam.SetKnockOut(true);
//param.TextParam.SetOptRead(true);
//param.TextParam.SetBitmapFont(true);
//param.TextParam.SetFauxItalic(true);
//param.TextParam.SetFauxBold(true);

    Render::Renderer*  prenderer = context.GetRenderer();

    bool dynamicCacheEnabled = false; 
    bool hasUnderline = false;
                
    float maxRasterScale = 1.25f;
    unsigned  steadyCount = 0;
    FontCacheManager* fm = context.pFontCacheManager;
    if (fm)
    {
        pCacheManager       = fm->GetImplementation();
        dynamicCacheEnabled = fm->IsDynamicCacheEnabled();
        maxRasterScale      = fm->GetMaxRasterScale();
        steadyCount         = fm->GetSteadyCount();
    }
    else
    {
        pCacheManager = 0;
    }

    Matrix mat(matrix);
    mat.PrependTranslation(-float(Geom.HScrollOffset), 0);

    float yOffset = -float(GetVScrollOffsetInTwips());

    float heightRatio = GFx_CalcHeightRatio(matrix) * 
                        PixelsToTwips(context.ViewportMatrix.Sy());

    if (heightRatio < 0.001f)
        heightRatio = 0.001f; // To prevent from Div by Zero

    if (steadyCount && !param.TextParam.IsOptRead())
    {
        bool animation = (matrix != Geom.SourceTextMatrix);
        if (animation)
        {
            Geom.SteadyCount = 0;
            if (Geom.IsReadability())
            {
                InvalidateCache();
                Geom.ClearReadability();
            }
        }
        else
        {
            if (nextFrame)
            {
                Geom.SteadyCount++;
                if (Geom.SteadyCount > steadyCount) 
                    Geom.SteadyCount = steadyCount;
            }
        }
        if (Geom.SteadyCount >= steadyCount)
        {
            param.TextParam.SetOptRead(true);
            param.ShadowParam.SetOptRead(true);
            if(!Geom.IsReadability())
            {
                InvalidateCache();
                Geom.SetReadability();
            }
        }
    }

    Matrix displayMatrix = matrix;
    bool   matrixChanged = false;
    if (param.TextParam.IsOptRead())
    {
        //// We have to check the 2x2 matrix. Theoretically it would be enough 
        //// to check for the heightRatio only, but it may result in using 
        //// a degenerate matrix for the first calculation, and so, 
        //// introducing huge inaccuracy. 
        ////------------------------------
        //matrixChanged = Geom.SourceTextMatrix.Sx() != matrix.Sx() ||
        //                Geom.SourceTextMatrix.Shx() != matrix.Shx() ||
        //                Geom.SourceTextMatrix.Shy() != matrix.Shy() ||
        //                Geom.SourceTextMatrix.Sy() != matrix.Sy() ||
        //                Geom.ViewportMatrix != context.ViewportMatrix;
        //Geom.SourceTextMatrix = matrix;


        // New method that eliminates text "shaking", when changing of 
        // coordinates is very small (less than 0.1 pixel).
        //----------------------------
        matrixChanged = matrix                 != Geom.SourceTextMatrix || 
                        context.ViewportMatrix != Geom.ViewportMatrix;

        if(matrixChanged)
        {
            Render::PointF p11 = Render::PointF(Geom.VisibleRect.x1,  Geom.VisibleRect.y1);
            Render::PointF p12 = Render::PointF(Geom.VisibleRect.x2, Geom.VisibleRect.y1);
            Render::PointF p13 = Render::PointF(Geom.VisibleRect.x2, Geom.VisibleRect.y2);
            Render::PointF p21 = p11;
            Render::PointF p22 = p12;
            Render::PointF p23 = p13;

            Geom.SourceTextMatrix.Transform2x2(&p11.x, &p11.y); p11 = Geom.ViewportMatrix.Transform(p11);
            Geom.SourceTextMatrix.Transform2x2(&p12.x, &p12.y); p12 = Geom.ViewportMatrix.Transform(p12);
            Geom.SourceTextMatrix.Transform2x2(&p13.x, &p13.y); p13 = Geom.ViewportMatrix.Transform(p13);

            matrix.Transform2x2(&p21.x, &p21.y); p21 = context.ViewportMatrix.Transform(p21);
            matrix.Transform2x2(&p22.x, &p22.y); p22 = context.ViewportMatrix.Transform(p22);
            matrix.Transform2x2(&p23.x, &p23.y); p23 = context.ViewportMatrix.Transform(p23);

            float d = p11.DistanceL1(p21) + p12.DistanceL1(p22) + p13.DistanceL1(p23);
            matrixChanged = d > 0.3f;
        }

        if (matrixChanged)
        {
            Geom.SourceTextMatrix = matrix;
        }
        else
        {
            displayMatrix = Geom.SourceTextMatrix;
            Render::PointF transl(Render::PointF(matrix.x - Geom.Translation.x, 
                                   matrix.y - Geom.Translation.y));
            float kx = context.ViewportMatrix.Sx();
            float ky = context.ViewportMatrix.Sy();
            displayMatrix.x = (Geom.Translation.x + floorf(transl.x * kx + 0.5f) / kx);
            displayMatrix.y = (Geom.Translation.y + floorf(transl.y * ky + 0.5f) / ky);
        }
    }
    else
    {
        matrixChanged = Geom.ViewportMatrix != context.ViewportMatrix;
        if (Geom.NeedsCheckPreciseScale() && heightRatio != Geom.HeightRatio)
        {
            matrixChanged = true;
        }
        Geom.SourceTextMatrix = matrix;
    }

    Geom.HeightRatio      = heightRatio;
    Geom.ViewportMatrix   = context.ViewportMatrix;

    if (phighlighter)
    {
        hasUnderline = phighlighter->HasUnderlineHighlight();
        if (!phighlighter->IsValid())
            InvalidateCache(); // if highligher was invalidated then invalidate the cache too
    }

    if (IsCacheInvalid() ||
        matrixChanged || 
        LastHScrollOffset != Geom.HScrollOffset)
    {
        pUnderlineDrawing = NULL;
    }

    if (IsCacheInvalid() || 
        param != Param ||
        matrixChanged || 
       !pBatchPackage || !pCacheManager ||
        context.MaskRenderCount > 0 || // if text is used as a mask - don't use batches (!AB)
        LastHScrollOffset != Geom.HScrollOffset ||
       !pCacheManager->VerifyBatchPackage(pBatchPackage, 
                                          context, 
                                          param.TextParam.IsOptRead() ? 0 : heightRatio))
    {
        //printf("\nInvalidate%d matrixChanged%d Readability%d\n", InvalidCache, matrixChanged, GFxTextParam::GetOptRead(textParam));
        ResetCache();
        ValidateCache();
    }

    LastHScrollOffset = Geom.HScrollOffset;

    // Prepare a batch for optimized rendering.
    // if text is used as a mask - don't use batches (!AB)
    unsigned numGlyphsInBatch = 0;
    if (!pBatchPackage && context.MaskRenderCount == 0)
    {      
        ClearBatchHasUnderline();
        Iterator     linesIt = BeginVisible(yOffset);
        Geom.ClearCheckPreciseScale();
        // set or clear "in-batch" flags for each glyph
        for(; linesIt.IsVisible(); ++linesIt)
        {
            LineBuffer::Line& line = *linesIt;
            LineBuffer::GlyphIterator glyphIt = line.Begin();
            for (; !glyphIt.IsFinished(); ++glyphIt)
            {
                LineBuffer::GlyphEntry& glyph = glyphIt.GetGlyph();
                glyph.ClearInBatch();

                if (glyphIt.IsUnderline())
                    SetBatchHasUnderline();

                if (glyph.GetIndex() == ~0u || glyph.IsCharInvisible() || glyphIt.HasImage())
                    continue;

                FontResource* presolvedFont = glyphIt.GetFont();
                SF_ASSERT(presolvedFont);

                unsigned fontSize = (unsigned)glyph.GetFontSize(); //?AB, support for fractional font size(?)
                if (fontSize == 0) 
                    continue;

                const TextureGlyphData* tgd = presolvedFont->GetTextureGlyphData();
                bool useGlyphTextures = false;
                if (tgd)
                {
                    float textScreenHeight = heightRatio * fontSize;
                    float maxGlyphHeight   = FontPackParams::GetTextureGlyphMaxHeight(presolvedFont);
                    useGlyphTextures       = ((textScreenHeight <= maxGlyphHeight * maxRasterScale) || 
                                               presolvedFont->GlyphShapesStripped());
                    Geom.SetCheckPreciseScale();
                }
                else
                if (dynamicCacheEnabled && pCacheManager)
                {
                    RectF b;
                    useGlyphTextures = 
                        pCacheManager->GlyphFits(presolvedFont->GetGlyphBounds(glyph.GetIndex(), &b),
                                                 fontSize, 
                                                 heightRatio,
                                                 param.TextParam,
                                                 maxRasterScale);
                }

                if (useGlyphTextures)
                {
                    glyph.SetInBatch();
                    ++numGlyphsInBatch;
                }
                else
                    Geom.SetCheckPreciseScale();
            }
        }

        Param = param;

        linesIt = BeginVisible(yOffset);
        Render::PointF offset = Geom.VisibleRect.TopLeft();
        offset.y += yOffset;

        // Create batches.
        // Value numGlyphsInBatch is just a hint to the allocator that
        // helps avoid extra reallocs. 
        if (pCacheManager)
        {
            linesIt.SetHighlighter(phighlighter);
            pBatchPackage = pCacheManager->CreateBatchPackage(Memory::pGlobalHeap->GetAllocHeap(this),
                                                              pBatchPackage, 
                                                              linesIt, 
                                                              context, 
                                                              offset,
                                                              &Geom,
                                                              param,
                                                              numGlyphsInBatch);
        }
        Geom.Translation.x = matrix.x;
        Geom.Translation.y = matrix.y;
    }

    bool maskApplied = false;

#ifdef DEBUG_VIS_RECT
    DrawRect(prenderer, mat, Geom.VisibleRect, Color(0, 0, 233, 128));
#endif

#ifdef DEBUG_LINE_WIDTHS
    // Draw bounds around every line
    {Iterator linesIt = BeginVisible(yOffset);
    for(; linesIt.IsVisible(); ++linesIt)
    {
        const Line& line = *linesIt;
        Render::PointF offset = line.GetOffset();
        offset += Geom.VisibleRect.TopLeft();
        offset.y += yOffset;

        RectF rect(offset.x, offset.y, offset.x + line.GetWidth(), offset.y + line.GetHeight());
        DrawRect(prenderer, mat, rect, Color(0, 222, 0, 128));
    }}
#endif //DEBUG_LINE_WIDTHS

    // Display batches
    if (pCacheManager)
    {
        pCacheManager->DisplayBatchPackage(pBatchPackage, 
                                           context, 
                                           displayMatrix,
                                           cxform);
    }

    if (!pCacheManager || 
         pCacheManager->IsVectorRenderingRequired(pBatchPackage))
    {
        // check for partially visible (vertically) line
        // if there is one - apply mask right here
        if (IsPartiallyVisible(yOffset))
        {
            maskApplied = ApplyMask(context, matrix, Geom.VisibleRect);
        }

        // We apply textColor through CXForm because that works for EdgeAA too.
        Cxform finalCxform = cxform;
        Color prevColor(0u);
        Iterator linesIt = BeginVisible(yOffset);
        RectF glyphBounds;
        for(; linesIt.IsVisible(); ++linesIt)
        {
            Line& line = *linesIt;
            Render::PointF offset;
            offset.x = (float)line.GetOffsetX();
            offset.y = (float)line.GetOffsetY();
            offset += Geom.VisibleRect.TopLeft();
            offset.y += yOffset + line.GetBaseLineOffset();
            int advance = 0;
            GlyphIterator glyphIt = line.Begin(phighlighter);
            for (; !glyphIt.IsFinished(); ++glyphIt, offset.x += advance)
            {
                const GlyphEntry& glyph = glyphIt.GetGlyph();

                advance = glyph.GetAdvance();
                if (glyph.IsInBatch() || (glyph.IsCharInvisible() && !glyphIt.IsUnderline() && !glyphIt.HasImage()))
                    continue;

                float scale = 1.0f;
                float approxSymW = 0;
                unsigned  index = glyph.GetIndex();
                ImageDesc* pimage = NULL;
                FontResource* presolvedFont = NULL;
                Ptr<ShapeDataBase> pglyphShape; 

                if (glyphIt.HasImage())
                {
                    pimage = glyphIt.GetImage();
                    approxSymW = pimage->GetScreenWidth();
                }
                else
                {
                    // render glyph as shape
                    float fontSize = (float)PixelsToTwips(glyph.GetFontSize());
                    scale = fontSize / 1024.0f; // the EM square is 1024 x 1024   

                    presolvedFont = glyphIt.GetFont();
                    SF_ASSERT(presolvedFont);

                    if (pCacheManager)
                    {
                        if (index != ~0u)
                            pglyphShape = *pCacheManager->GetGlyphShape(presolvedFont, index, 0,
                                                                       glyphIt.IsFauxBold() | param.TextParam.IsFauxBold(), 
                                                                       glyphIt.IsFauxItalic() | param.TextParam.IsFauxItalic(),
                                                                       context.pLog);
                        if (pglyphShape)
                            approxSymW = pglyphShape->GetBound().x2 * scale;
                    }
                    else
                    {
                        if (index != ~0u)
                            pglyphShape = *presolvedFont->GetGlyphShape(index, 0);

                        if (!IsStaticText()) // no mask or partial visibility test for static text
                        {
                            approxSymW = presolvedFont->GetGlyphBounds(index, &glyphBounds).x2 * scale;
                        }
                        else
                        {
                            // For static text GetGlyphBounds is expensive, and since we do not 
                            // need it, just don't get it for static text.
                            approxSymW = 0;
                        }
                    }
                }
                float adjox = offset.x - Geom.HScrollOffset;

                // check for invisibility/partial visibility of the glyph in order to determine
                // necessity of setting mask.
                if (!IsStaticText()) // no mask or partial visibility test for static text
                {
                    // test for complete invisibility, left side
                    if (index != ~0u && adjox + approxSymW <= Geom.VisibleRect.x1)
                        continue;

                    // test for complete invisibility, right side
                    if (adjox >= Geom.VisibleRect.x2)
                        break; // we can finish here
                }
                if (glyphIt.IsUnderline())
                {
                    if (!glyph.IsNewLineChar())
                        hasUnderline = true;
                    if (glyph.IsCharInvisible())
                        continue;
                }

                // test for partial visibility
                if (!maskApplied && !IsStaticText() && !Geom.IsNoClipping() &&
                    ((adjox < Geom.VisibleRect.x1 && adjox + approxSymW > Geom.VisibleRect.x1) || 
                    (adjox < Geom.VisibleRect.x2 && adjox + approxSymW > Geom.VisibleRect.x2)))
                {
                    maskApplied = ApplyMask(context, matrix, Geom.VisibleRect);
                }

                Matrix  m(mat);
                m.PrependTranslation(offset.x, offset.y);

                if (pimage)
                {
                    if (pimage->pImageShape)
                    {
                        m *= pimage->Matrix;
                        // draw image
                        DisplayParams params(context, m, cxform, Render::Blend_None);
                        pimage->pImageShape->GetFillAndLineStyles(&params);
                        pimage->pImageShape->Display(params, false, 0);
                    }
                    continue;
                }

                Color color(glyphIt.GetColor());
                if (color != prevColor)
                {
                    Cxform   colorCxform;
                    colorCxform.M[0][0] = (1.0f / 255.0f) * color.GetRed();
                    colorCxform.M[1][0] = (1.0f / 255.0f) * color.GetGreen();
                    colorCxform.M[2][0] = (1.0f / 255.0f) * color.GetBlue();
                    colorCxform.M[3][0] = (1.0f / 255.0f) * color.GetAlpha();

                    finalCxform = cxform;
                    finalCxform.Concatenate(colorCxform);
                }

                m.PrependScaling(scale);

                if (index == ~0u)
                {
                    // Invalid glyph; render it as an empty box.
                    prenderer->SetCxform(cxform);
                    prenderer->SetMatrix(m);
                    Color transformedColor = cxform.Transform(glyphIt.GetColor());
                    prenderer->LineStyleColor(transformedColor);

                    // The EM square is 1024x1024, but usually isn't filled up.
                    // We'll use about half the width, and around 3/4 the height.
                    // Values adjusted by eye.
                    // The Y baseline is at 0; negative Y is up.
                    static const SInt16 EmptyCharBox[5 * 2] =
                    {
                        32,     32,
                        480,    32,
                        480,    -656,
                        32,     -656,
                        32,     32
                    };

                    prenderer->SetVertexData(EmptyCharBox, 5, Render::Renderer::Vertex_XY16i);
                    prenderer->DrawLineStrip(0, 4);
                    prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
                }
                else
                {               
                    // Draw the GFxCharacter using the filled outline.
                    // MA TBD: Is 0 ok for mask in characters?
                    if (pglyphShape)
                    {
                        // Blend arg doesn't matter because no stroke is used for glyphs.
                        pglyphShape->SetNonZeroFill(true);
                        DisplayParams params(context, m, finalCxform, Render::Blend_None);
                        params.FillStylesNum = 1;
                        params.pFillStyles = &DummyStyle;

                        pglyphShape->Display(params, false, 0);
                        //pglyphShape->Display(context, m, finalCxform, Render::Blend_None,
                        //    0, DummyStyle, DummyLineStyle, 0);
                    }
                }
            }
        }
    }

    // draw underline if necessary
    if ((hasUnderline || HasBatchUnderline()) && !pUnderlineDrawing)
    {
        Iterator linesIt = BeginVisible(yOffset);
        //linesIt.SetHighlighter(phighlighter);

        // calculate the "hscrolled" view rect for clipping
        RectF hscrolledViewRect(Geom.VisibleRect);
        hscrolledViewRect.OffsetX(float(Geom.HScrollOffset));

        for(; linesIt.IsVisible(); ++linesIt)
        {
            Line& line = *linesIt;
            Render::PointF offset;
            offset.x = (float)line.GetOffsetX();
            offset.y = (float)line.GetOffsetY();
            offset += Geom.VisibleRect.TopLeft();

            bool newLine = true;

            float   offx = 0, offy = 0, singleUnderlineHeight = 0, singlePointWidth = 0;
            UInt32  lastColor = 0;
            int     lineLength = 0;
            unsigned    lastIndex = ~0u;
            HighlightInfo::UnderlineStyle lastStyle = HighlightInfo::Underline_None;

            Matrix globalMatrixDir;
            Matrix globalMatrixInv;
            int advance = 0;
            GlyphIterator glyphIt = line.Begin(phighlighter);
            for (unsigned i = 0; !glyphIt.IsFinished(); ++glyphIt, offset.x += advance, ++i)
            {
                const GlyphEntry& glyph = glyphIt.GetGlyph();

                advance = glyph.GetAdvance();
                if (glyphIt.IsUnderline() && !glyphIt.HasImage() && !glyph.IsNewLineChar())
                {
                    float adjox = offset.x - Geom.HScrollOffset;

                    // check for invisibility/partial visibility of the glyph in order to determine
                    // necessity of setting mask.

                    // test for complete invisibility, right side
                    if (adjox >= Geom.VisibleRect.x2)
                        break; // we can finish here

                    // test for complete invisibility, left side
                    if (adjox + advance < Geom.VisibleRect.x1)
                        continue;

                    // if this is the first visible underlined char in the line - 
                    // do some init stuff
                    if (newLine)
                    {
                        float descent = float(line.GetHeight() - line.GetBaseLineOffset());
                        offset.y += yOffset + line.GetBaseLineOffset() + descent/2;

                        // snap to pixel the x and y values
                        //globalMatrixDir = Geom.SourceTextMatrix;

                        globalMatrixDir = mat;

                        globalMatrixDir.Append(context.ViewportMatrix);
                        globalMatrixInv = globalMatrixDir;
                        globalMatrixInv.Invert();

                        Render::PointF offset1   = globalMatrixDir.Transform(offset);
                        offset1.x = floorf(offset1.x + 0.5f);
                        offset1.y = floorf(offset1.y + 0.5f);
                        offset    = globalMatrixInv.Transform(offset1);
                        offset.x = floorf(offset.x + 0.5f);
                        offset.y = floorf(offset.y + 0.5f);

                        offset1.x += 1;
                        offset1.y += 1;
                        offset1    = globalMatrixInv.Transform(offset1);
                        offset1.x = floorf(offset1.x + 0.5f);
                        offset1.y = floorf(offset1.y + 0.5f);

                        singleUnderlineHeight = offset1.y - offset.y;
                        singlePointWidth      = offset1.x - offset.x;
                        offx = offset.x;
                        offy = offset.y;

                        newLine = false;
                    }

                    UInt32 curColor = glyphIt.GetUnderlineColor().ToColor32();
                    HighlightInfo::UnderlineStyle curStyle = glyphIt.GetUnderlineStyle();
                    if (lineLength != 0 && 
                       (lastColor != curColor || lastIndex + 1 != i || curStyle != lastStyle))
                    {
                        DrawUnderline(offx, offy, singleUnderlineHeight, singlePointWidth, hscrolledViewRect, 
                            lineLength, lastStyle, lastColor, globalMatrixDir, globalMatrixInv);

                        lineLength = 0;
                        offx = offset.x;
                    }
                    lineLength += advance;
                    lastColor = curColor;
                    lastIndex = i;
                    lastStyle = curStyle;
                }
            }
            if (lineLength != 0)
            {
                DrawUnderline(offx, offy, singleUnderlineHeight, singlePointWidth, hscrolledViewRect, 
                    lineLength, lastStyle, lastColor, globalMatrixDir, globalMatrixInv);
            }
        }
    }
    if (pUnderlineDrawing)
        pUnderlineDrawing->Display(context, mat, cxform, Render::Blend_None, false);

    // done DisplayRecord
    if (maskApplied)
    {
#ifndef DEBUG_MASK
        RemoveMask(context, matrix, Geom.VisibleRect);
#endif
    }
}
#endif

void    LineBuffer::SetFirstVisibleLine(unsigned line)
{
    Geom.FirstVisibleLinePos = line;
    InvalidateCache();
}

void    LineBuffer::SetHScrollOffset(unsigned offset)
{
    Geom.HScrollOffset = offset;
}

LineBuffer::Iterator LineBuffer::FindLineByTextPos(UPInt textPos)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, (unsigned)textPos, LineIndexComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        unsigned lineTextPos = pline->GetTextPos();
        if (textPos >= lineTextPos && textPos <= lineTextPos + pline->GetTextLength())
            return Iterator(*this, (unsigned)i);
    }
    return Iterator();
}

LineBuffer::Iterator LineBuffer::FindLineAtYOffset(float yoff)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, yoff, LineYOffsetComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        if (yoff >= pline->GetOffsetY() && yoff < pline->GetOffsetY() + pline->GetHeight() + pline->GetLeading())
        {
            return Iterator(*this, (unsigned)i);
        }
    }
    return Iterator();
}

LineBuffer::Iterator LineBuffer::FindLineAtOffset(const Render::PointF& p)
{
    if (Lines.GetSize() > 0)
    {
        UPInt i = Alg::LowerBound(Lines, p.y, LineYOffsetComparator::Less);
        if (i == Lines.GetSize())
            --i;
        Line* pline = Lines[(unsigned)i];
        int   yoff = pline->GetOffsetY();
        while (pline)
        {
            if ( (p.y >= pline->GetOffsetY() && p.y < pline->GetOffsetY() + pline->GetHeight() + pline->GetLeading()) )
            {
                if (p.x >= pline->GetOffsetX() && p.x < pline->GetOffsetX() + pline->GetWidth())
                {
                    return Iterator(*this, (unsigned)i);
                }
                if ( (++i) == Lines.GetSize() ) break;
                pline = Lines[(unsigned)i];
                if (pline->GetOffsetY() != yoff)
                    break;
            }
            else
                break;
        }
    }
    return Iterator();
}

bool LineBuffer::IsLineVisible(unsigned lineIndex, float yOffset) const
{
    const Line& line = *Lines[lineIndex];
    if (lineIndex == Geom.FirstVisibleLinePos)
    {
        // a special case for the first VISIBLE line: display it even if it is only
        // partially visible
        return (line.GetOffsetY() + yOffset <= Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2); 
    }
    // subtract leading of the line before checking its complete visibility
    return lineIndex > Geom.FirstVisibleLinePos  && 
        (line.GetOffsetY() + line.GetHeight() + yOffset <= Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2); 
}

bool LineBuffer::IsPartiallyVisible(float yOffset) const
{
    if (Geom.FirstVisibleLinePos < GetSize())
    {
        const Line& line = *Lines[Geom.FirstVisibleLinePos];
        if (line.GetWidth() != 0 && line.GetHeight() != 0)
        {
            // XBox360 VC9 compiler had a problem compiling original complex condition
            // (optimizer generated incorrect code). So, have splitted it by more
            // simple parts.
			float lh     = (float)line.GetHeight();
			float vrectH = Geom.VisibleRect.Height() + GFX_TEXT_GUTTER/2;
			float yf     = (float)line.GetOffsetY() + yOffset;
            if (yf <= vrectH && (yf + lh) > vrectH)
			{
// 				bool b1 = (yf <= vrectH);
// 				bool b2 = ((yf + lh) > vrectH);
// 				printf("Mask: %f > %f,  %d && %d, yoff %f, ly  %f, lhei %f, visrHei %f\n", 
// 					float(yf + line.GetHeight()), float(vrectH), 
// 					(int)b1, (int)b2,
// 					float(yOffset), float(yf), float(lh), float(Geom.VisibleRect.Height()));
                return true;
			}
        }
    }
    return false;
}

void LineBuffer::Scale(float scaleFactor)
{
    Iterator it = Begin();
    for (;!it.IsFinished(); ++it)
    {
        Line& line = *it;
        SF_ASSERT(line.IsData32());

        float newLeading    = float(line.GetLeading())*scaleFactor;
        float newW          = float(line.GetWidth())*scaleFactor;
        float newH          = float(line.GetHeight())*scaleFactor;
        line.SetLeading(int(newLeading));
        line.SetDimensions(int(newW), int(newH));
        line.SetBaseLineOffset(line.GetBaseLineOffset()*scaleFactor);
        line.SetOffsetX(int(float(line.GetOffsetX())*scaleFactor));
        line.SetOffsetY(int(float(line.GetOffsetY())*scaleFactor));

        GlyphIterator git = line.Begin();
        for(; !git.IsFinished(); ++git)
        {
            GlyphEntry& ge = git.GetGlyph();
            float newAdv = float(ge.GetAdvance())*scaleFactor;
            ge.SetAdvance(int(newAdv));
            ge.SetFontSize(ge.GetFontSize()*scaleFactor);
        }
    }
    InvalidateCache();
}

int LineBuffer::GetMinLineHeight() const
{
    if (GetSize() == 0)
        return 0;
    ConstIterator it = Begin();
    int minH = SF_MAX_SINT;
    for (;!it.IsFinished(); ++it)
    {
        const Line& line = *it;
        int h = line.GetHeight();
        if (h < minH)
            minH = h;
    }
    return minH;
}

#ifdef SF_BUILD_DEBUG
void LineBuffer::Dump() const
{
    printf("Dumping lines...\n");
    printf("VisibleRect: { %f, %f, {%f, %f}}\n", 
        Geom.VisibleRect.x1, Geom.VisibleRect.y1, Geom.VisibleRect.Width(), Geom.VisibleRect.Height());
    ConstIterator linesIt = Begin();
    unsigned i = 0;
    for(; !linesIt.IsFinished(); ++linesIt)
    {
        const Line& line = *linesIt;
        printf("Line[%d]\n", i++);
        printf("   TextPos = %d, NumGlyphs = %d, Len = %d\n", line.GetTextPos(), line.GetNumGlyphs(), line.GetTextLength());
        printf("   BaseLine = %f, Leading = %d\n", line.GetBaseLineOffset(), line.GetLeading());
        printf("   Bounds = { %d, %d, {%d, %d}}\n", line.GetOffsetX(), line.GetOffsetY(), line.GetWidth(), line.GetHeight());
    }
    printf("...end\n\n");
}

void LineBuffer::CheckIntegrity() const
{
    ConstIterator linesIt = Begin();
    unsigned nextpos = 0;
    for(; !linesIt.IsFinished(); ++linesIt)
    {
        const Line& line = *linesIt;
        unsigned l = 0;
        GlyphIterator git = const_cast<Line&>(line).Begin();
        for (; !git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& glyph = git.GetGlyph();
            l += glyph.GetLength();
        }
        if (!(line.GetTextPos() == nextpos || line.GetTextPos() == nextpos + 1) )
        {
            Dump();
            SF_ASSERT(0);
        }
        nextpos = line.GetTextPos() + l;
    }
}
#endif

void RecalculateRectToFit16Bit(Matrix2F& matrix, const RectF& srcRect, RectF* pdestRect)
{
    matrix.PrependTranslation(srcRect.x1, srcRect.y1);

    float xscale = 1, width = srcRect.Width();
    if (width > 32767)
    {
        xscale = width / 32767;
        width = 32767;
    }
    float yscale = 1, height = srcRect.Height();
    if (height > 32767)
    {
        yscale = height / 32767;
        height = 32767;
    }
    matrix.PrependScaling(xscale, yscale);
    SF_ASSERT(pdestRect);
    pdestRect->y1 = pdestRect->x1 = 0;
    pdestRect->SetWidth(width);
    pdestRect->SetHeight(height);
}

static TextUnderlineStyle ConvertUnderlineStyle(HighlightInfo::UnderlineStyle st)
{
    switch(st)
    {
    case HighlightInfo::Underline_Single:           return TextUnderline_Single; break;
    case HighlightInfo::Underline_Thick:            return TextUnderline_Thick; break;
    case HighlightInfo::Underline_Dotted:           return TextUnderline_Dotted; break;
    case HighlightInfo::Underline_DitheredSingle:   return TextUnderline_DitheredSingle; break;
    case HighlightInfo::Underline_DitheredThick:    return TextUnderline_DitheredThick; break;
    default:;
    }
    SF_ASSERT(0); // shouldn't reach here!
    return TextUnderline_Single;
}

void LineBuffer::CreateVisibleTextLayout(TextLayout::Builder& bld, 
                                         const Highlighter* phighlighter, 
                                         const TextFieldParam& textFieldParam)
{
    float yOffset = -float(GetVScrollOffsetInFixp());

    Iterator linesIt = BeginVisible(yOffset);
    RectF glyphBounds;
    bool hasUnderline = false;
    if (phighlighter)
    {
        hasUnderline = phighlighter->HasUnderlineHighlight();
    }
    Color prevColor(0u);
    Font* prevFont = NULL;
    float prevFontSize = 0;
    bool  maskApplied;

    if (IsPartiallyVisible(yOffset))
    {
        bld.SetClipBox(Geom.VisibleRect);
        maskApplied = true;
    }
    else
        maskApplied = false;
    for(; linesIt.IsVisible(); ++linesIt)
    {
        Line& line = *linesIt;
        Render::PointF offset;
        offset.x = (float)line.GetOffsetX();
        offset.y = (float)line.GetOffsetY();
        offset += Geom.VisibleRect.TopLeft();
        offset.y += yOffset + line.GetBaseLineOffset();
        GlyphIterator glyphIt = line.Begin(phighlighter);
        bool firstVisible = true;
       
        Color prevSelectionColor(0u);
        RectF selectionRect(0,0,0,0);

        HighlightInfo::UnderlineStyle prevUnderlineStyle = HighlightInfo::Underline_None;
        Color                         prevUnderlineColor(0u);
        
        int     underlineLen = 0;
        PointF  underlineBeginPt(0,0);

        for (int advance = 0; !glyphIt.IsFinished(); ++glyphIt, offset.x += advance)
        {
            const GlyphEntry& glyph = glyphIt.GetGlyph();

            advance = glyph.GetAdvance();
//             if ((glyph.IsCharInvisible() && !glyphIt.IsUnderline() && !glyphIt.HasImage()))
//                 continue;

            float       scale       = 1.0f;
            float       approxSymW  = 0;
            unsigned    index       = glyph.GetIndex();
            ImageDesc*  pimage      = NULL;
            Font*       pfont       = NULL;
            float       fontSize    = -1;
            Color       selectionColor = glyphIt.GetSelectionColor();

            if (glyphIt.HasImage())
            {
                pimage = glyphIt.GetImage();
                approxSymW = pimage->GetScreenWidth();
            }
            else
            {
                fontSize = (float)PixelsToFixp(glyph.GetFontSize());
                scale = fontSize / 1024.0f; // the EM square is 1024 x 1024   

                pfont = glyphIt.GetFont();
                SF_ASSERT(pfont);

                if (!IsStaticText()) // no mask or partial visibility test for static text
                {
                    approxSymW = pfont->GetGlyphBounds(index, &glyphBounds).x2 * scale;
                }
                else
                {
                    // For static text GetGlyphBounds is expensive, and since we do not 
                    // need it, just don't get it for static text.
                    approxSymW = 0;
                }
            }
            float adjox = offset.x - Geom.HScrollOffset;

            // check for invisibility/partial visibility of the glyph in order to determine
            // necessity of setting mask.
            if (!IsStaticText()) // no mask or partial visibility test for static text
            {
                // test for complete invisibility, left side
                if (index != ~0u && adjox + approxSymW <= Geom.VisibleRect.x1)
                    continue;

                // test for complete invisibility, right side
                if (adjox >= Geom.VisibleRect.x2)
                    break; // we can finish here
            }
            if (glyphIt.IsUnderline())
            {
                if (!glyph.IsNewLineChar())
                    hasUnderline = true;
                if (glyph.IsCharInvisible())
                    continue;
            }
            else
                hasUnderline = false;

            if (firstVisible)
            {
                bld.SetNewLine(adjox, offset.y);
                firstVisible = false;
            }

            // test for partial visibility
            if (!maskApplied && !IsStaticText() && !Geom.IsNoClipping() &&
                ((adjox < Geom.VisibleRect.x1 && adjox + approxSymW > Geom.VisibleRect.x1) || 
                (adjox < Geom.VisibleRect.x2 && adjox + approxSymW > Geom.VisibleRect.x2)))
            {
                bld.SetClipBox(Geom.VisibleRect);
                maskApplied = true;
            }

            if (pimage)
            {
                if (pimage->pImage)
                    bld.AddImage(pimage->pImage, pimage->Matrix.GetXScale(), pimage->Matrix.GetYScale(), pimage->BaseLineX, float(advance));
                else
                    LogDebugMessage(Log_Warning, "An image in TextLayout is NULL");
                continue;
            }

            Color color(glyphIt.GetColor());
            if (color != prevColor)
            {
                bld.ChangeColor(color.ToColor32());
            }
            SF_ASSERT(fontSize > 0);
            if (pfont != prevFont || fontSize != prevFontSize)
            {
                bld.ChangeFont(pfont, fontSize);

                // just need to hold FontHandle inside the layout to avoid releasing memory heap
                // while the layout uses the font. FontHandle may store a strong pointer to font
                // container.
                FontHandle* fh = glyphIt.GetFontHandle();
                bld.AddRefCntData(fh);
            }

            // handle underline
            if (prevUnderlineColor != glyphIt.GetUnderlineColor() || prevUnderlineStyle != glyphIt.GetUnderlineStyle())
            {
                if (prevUnderlineColor != 0)
                {
                    bld.AddUnderline(underlineBeginPt.x, underlineBeginPt.y, float(underlineLen), 
                        ConvertUnderlineStyle(prevUnderlineStyle), prevUnderlineColor.ToColor32());
                }
                // new underline, no underline before
                underlineBeginPt = offset;
                underlineLen     = advance;
            }
            else if (glyphIt.GetUnderlineColor() != 0)
            {
                // extend
                underlineLen += advance;
            }


            // handle selection: accumulate or insert selection rect
            if (glyphIt.IsSelected() || prevSelectionColor != 0)
            {
                RectF glyphRect(0, 0,  float(advance), float(line.GetHeight() + line.GetNonNegLeading()));
                PointF rectOff(offset.x, offset.y - float(line.GetAscent()));
                glyphRect += rectOff;
                if (selectionColor != prevSelectionColor)
                {
                    if (prevSelectionColor != 0)
                    {
                        // previous selection existed, need to finish it first
                        bld.AddSelection(selectionRect, prevSelectionColor.ToColor32());
                    }
                    selectionRect = glyphRect;
                }
                else if (selectionColor != 0)
                {
                    // just extend selection rect, color is the same
                    selectionRect.Union(glyphRect);
                }
            }

            bld.AddChar(index, float(advance));
            prevColor           = color;
            prevFont            = pfont;
            prevFontSize        = fontSize;
            prevSelectionColor  = selectionColor;
            if (hasUnderline)
            {
                prevUnderlineColor  = glyphIt.GetUnderlineColor();
                prevUnderlineStyle  = glyphIt.GetUnderlineStyle();
            }
            else
            {
                prevUnderlineColor = Color(0u);
                prevUnderlineStyle = HighlightInfo::Underline_None;
            }
        }
        if (prevUnderlineColor != 0)
        {
            // finalize underline
            bld.AddUnderline(underlineBeginPt.x, underlineBeginPt.y, float(underlineLen), 
                             ConvertUnderlineStyle(prevUnderlineStyle), prevUnderlineColor.ToColor32());
        }
        if (prevSelectionColor != 0)
        {
            // finalize selection
            bld.AddSelection(selectionRect, prevSelectionColor.ToColor32());
        }
    }
    bld.SetParam(textFieldParam);
}

void LoadTextFieldParamFromTextFilter(TextFieldParam& params, const TextFilter& filter)
{
    ///////////////////////////////
    params.TextParam.SetBlurX(filter.BlurX);
    params.TextParam.SetBlurY(filter.BlurY);
    params.TextParam.Flags = GlyphParam::FineBlur;
    params.TextParam.SetBlurStrength(filter.BlurStrength);
    //params.TextParam.Outline      = ...;

    if ((filter.ShadowFlags & TextFieldParam::ShadowDisabled) == 0)
    {
        params.ShadowParam.Flags = UInt16(filter.ShadowFlags & ~TextFieldParam::ShadowDisabled);
        params.ShadowParam.SetBlurX(filter.ShadowBlurX);
        params.ShadowParam.SetBlurY(filter.ShadowBlurY);
        params.ShadowParam.SetBlurStrength(filter.ShadowStrength);
        //params.ShadowParam.Outline      = ...;
        params.ShadowColor              = filter.ShadowColor.ToColor32();
        params.ShadowOffsetX            = filter.ShadowOffsetX;
        params.ShadowOffsetY            = filter.ShadowOffsetY;

    }
}

}}} // Scaleform::Render::Text

