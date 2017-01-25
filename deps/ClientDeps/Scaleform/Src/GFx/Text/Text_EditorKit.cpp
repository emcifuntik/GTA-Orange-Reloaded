/**********************************************************************

Filename    :   EditorKit.cpp
Content     :   Editor implementation
Created     :   April 29, 2008
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#include "GFx/Text/Text_EditorKit.h"
#include "GFx/GFx_Input.h"
#include "Kernel/SF_Timer.h"

namespace Scaleform { namespace GFx { namespace Text {

#ifdef GFX_ENABLE_TEXT_INPUT
////////////////////////////////////////////
// EditorKit
//
EditorKit::EditorKit(DocView* pdocview):pDocView(pdocview), Flags(0)
{
    CursorTimer     = 0.0f;
    CursorPos       = 0;
    CursorColor     = 0xFF000000; // black, opaque
    LastAdvanceTime = 0.0f;
    LastHorizCursorPos = -1;
    LastClickTime   = 0;

    CursorRect->Clear();
    ActiveSelectionBkColor      = GFX_ACTIVE_SEL_BKCOLOR;
    ActiveSelectionTextColor    = GFX_ACTIVE_SEL_TEXTCOLOR;
    InactiveSelectionBkColor    = GFX_INACTIVE_SEL_BKCOLOR;
    InactiveSelectionTextColor  = GFX_INACTIVE_SEL_TEXTCOLOR;
    pdocview->SetEditorKit(this);
}

EditorKit::~EditorKit()
{
}

void EditorKit::SetCursorPos(UPInt pos, bool selectionAllowed)
{
    if (pos != SF_MAX_UPINT)
    {
        UPInt len = pDocView->GetLength();
        if (pos > len)
            pos = len;

        ResetBlink(1);

        if (pDocView->GetImageSubstitutor())
        {
            // if image substitution is on, then we need to check that new cursor pos
            // is not inside the image. If so - correct the cursor position.
            UPInt textPos;
            const LineBuffer::GlyphEntry* pge = GetGlyphEntryAtIndex(pos, &textPos);
            if (pge && pge->GetLength() != 1 && textPos != pos)
            {
                // determine the vector of cursor changing
                if (pos >= CursorPos)
                    pos = textPos + pge->GetLength();
                else
                    pos = textPos;
            }
        }
    }

    CursorPos = pos;
    CursorRect.Invalidate();
    LastHorizCursorPos = -1;

    if (CursorPos != SF_MAX_UPINT)
    {
        ScrollToCursor();

        GetDocument()->SetDefaultTextAndParaFormat(CursorPos);
    }
    if (IsSelectable())
    {
        if (selectionAllowed)
        {
            // handle selection here
            if (IsShiftPressed() || IsMouseCaptured())
            {
                if (pDocView->GetEndSelection() != CursorPos)
                    pDocView->SetEndSelection(CursorPos);            
            }
            else
            {
                if (pDocView->GetBeginSelection() != CursorPos || pDocView->GetEndSelection() != CursorPos)
                    pDocView->SetSelection(CursorPos, CursorPos);
            }
        }
        else
        {
            pDocView->SetSelection(CursorPos, CursorPos);
        }
    }

#ifdef DEBUG_CURSOR_POS
    printf("CursorPos = %d\n", CursorPos);
#endif // DEBUG_CURSOR_POS

    if (pDocView->GetDocumentListener())
    {
        pDocView->GetDocumentListener()->Editor_OnCursorMoved(*this);
    }
}

UPInt EditorKit::GetCursorPos() const
{
    return CursorPos;
}

void EditorKit::Advance(Double timer)
{
    Double delta = timer - LastAdvanceTime;
    LastAdvanceTime = timer;

    // handle cursor blinking
    if (HasCursor())
    {
        if (CursorTimer+delta > 0.5)
        {
            if (!IsCursorTimerBlocked())
            {
                if (IsCursorBlink())
                    ClearCursorBlink();
                else
                    SetCursorBlink();
                if (pDocView->GetDocumentListener())
                {
                    pDocView->GetDocumentListener()->Editor_OnCursorBlink(*this, IsCursorBlink());
                }
            }
            CursorTimer = 0.0;
            ClearCursorTimerBlocked();
        }
        else
            CursorTimer += delta;
    }
    // handle auto-scrolling, if mouse is captured
    if (IsMouseCaptured())
    {
        //printf("%f t = %f, b = %f\n", LastMousePos.y, pDocView->GetViewRect().y1, pDocView->GetViewRect().y2);
        if (LastMousePos.y <= pDocView->GetViewRect().y1)
        {
            // scroll up
            unsigned lineIndex = pDocView->GetVScrollOffsetValue();

            if (lineIndex > 0)
            {
                --lineIndex;
                UPInt newPos = pDocView->GetCursorPosInLine(lineIndex, LastMousePos.y);
                SF_ASSERT(newPos != SF_MAX_UPINT);
                if (newPos != CursorPos)
                    SetCursorPos(newPos);
            }
        }
        else if (LastMousePos.y >= pDocView->GetViewRect().y2)
        {
            // scroll down
            unsigned lineIndex = pDocView->GetBottomVScroll();
            ++lineIndex;

            if (lineIndex < pDocView->GetLinesCount())
            {
                UPInt newPos = pDocView->GetCursorPosInLine(lineIndex, LastMousePos.y);
                SF_ASSERT(newPos != SF_MAX_UPINT);
                if (newPos != CursorPos)
                    SetCursorPos(newPos);
            }
        }
    }
}

bool EditorKit::CalcCursorRectInLineBuffer
(UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex, unsigned* pglyphIndex, bool avoidComposStr, 
 LineBuffer::Line::Alignment* plineAlignment)
{
    SF_ASSERT(pcursorRect);    
    pDocView->ForceReformat();
    unsigned lineIndex = pDocView->GetLineIndexOfChar(charIndex);
    //pDocView->LineBuffer.Dump();

    if (lineIndex != ~0u)
    {
        const LineBuffer::Line& line = pDocView->GetLineBuffer()[lineIndex];
        if (plineAlignment)
            *plineAlignment = line.GetAlignment();

        UPInt posInLine = charIndex - line.GetTextPos();
        int   xoffset = 0;

        // now need to find the glyph entry. Do the exhaustive search for now
        LineBuffer::GlyphIterator git = line.Begin();
        unsigned nGlyph = 0;

        // skip all leading "empty" glyphs (with length == 0)
        for(;!git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& ge = git.GetGlyph();
            if (ge.GetLength() != 0 || ge.IsNewLineChar())
                break;
            xoffset += ge.GetAdvance();
            ++nGlyph;
        }
        for(unsigned i = 0;i < posInLine && !git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& ge = git.GetGlyph();
            xoffset += ge.GetAdvance();

            if (!avoidComposStr || !ge.IsComposStrGlyph())
                i += ge.GetLength();
            ++nGlyph;
        }
        float w = 0;
        if (!git.IsFinished())
        {
            w = float(git.GetGlyph().GetAdvance());
        }
        pcursorRect->x1 = line.GetOffsetX() + float(xoffset);
        pcursorRect->y1  = float(line.GetOffsetY());
        pcursorRect->x2  = pcursorRect->x1 + w;
        pcursorRect->y2 = pcursorRect->y1  + line.GetHeight();
        if (plineIndex)
            *plineIndex = lineIndex;
        if (pglyphIndex)
            *pglyphIndex = nGlyph;
        return true;
    }
    return false;
}

bool EditorKit::CalcCursorRectOnScreen
(UPInt charIndex, RectF* pcursorRect, unsigned* plineIndex, unsigned* pglyphIndex, bool avoidComposStr,
 LineBuffer::Line::Alignment* plineAlignment)
{
    bool rv = CalcCursorRectInLineBuffer(charIndex, pcursorRect, plineIndex, pglyphIndex, avoidComposStr, plineAlignment);
    if (rv)
    {
        pcursorRect->Offset(Render::PointF(-float(pDocView->GetLineBuffer().GetHScrollOffset()), 
            -float(pDocView->GetLineBuffer().GetVScrollOffsetInFixp())));
        const RectF& rect = pDocView->GetViewRect();
        pcursorRect->SetTopLeft(pDocView->GetLineBuffer().BufferToView(rect, pcursorRect->TopLeft()));
        pcursorRect->SetBottomRight(pDocView->GetLineBuffer().BufferToView(rect, pcursorRect->BottomRight()));
        pcursorRect->Offset(Render::PointF(rect.x1, rect.y1));
    }
    return rv;
}

const LineBuffer::GlyphEntry* EditorKit::GetGlyphEntryAtIndex(UPInt charIndex, UPInt* ptextPos)
{
    unsigned lineIndex = pDocView->GetLineIndexOfChar(charIndex);
    if (lineIndex != ~0u)
    {
        const LineBuffer::Line* pline = pDocView->GetLineBuffer().GetLine(lineIndex);
        if (!pline)
            return NULL;
        unsigned textPos   = pline->GetTextPos();
        UPInt    posInLine = charIndex - textPos;

        // now need to find the glyph entry. 
        LineBuffer::GlyphIterator git = pline->Begin();
        const LineBuffer::GlyphEntry* pge = NULL;

        // skip all leading "empty" glyphs (with length == 0)
        for(;!git.IsFinished(); ++git)
        {
            const LineBuffer::GlyphEntry& ge = git.GetGlyph();
            if (ge.GetLength() != 0 || ge.IsNewLineChar())
                break;
        }
        unsigned prevLen = 0;
        for(unsigned i = 0; i <= posInLine; ++git)
        {
            textPos += prevLen;
            if (git.IsFinished())
                break;
            pge = &git.GetGlyph();

            prevLen = pge->GetLength();
            i += prevLen;
        }
        if (ptextPos)
            *ptextPos = textPos;
        return pge;
    }
    return NULL;
}

// @REN
/*
void EditorKit::PreDisplay(DisplayContext &context, const Matrix& mat, const Cxform& cx)
{
    SF_UNUSED3(context, mat, cx);

    // handle wide cursor. Wide cursor is drawing in overwrite mode by using highlighter.
    if (HasCursor() && IsWideCursor())
    {
        bool cursorBlinkState;
        Highlighter* phighlighter = pDocView->CreateHighlighterManager();
        SF_ASSERT(phighlighter);

        HighlightDesc* pdesc = NULL;
        if ((pdesc = phighlighter->GetHighlighterPtr(GFX_WIDECURSOR_HIGHLIGHTING_INDEX)) == NULL)
        {
            HighlightDesc desc;
            desc.StartPos = 0;
            desc.Length   = 0;
            desc.Offset   = 0;
            desc.Id = GFX_WIDECURSOR_HIGHLIGHTING_INDEX;
            desc.Info.SetBackgroundColor(GFX_ACTIVE_SEL_BKCOLOR);
            desc.Info.SetTextColor(GFX_ACTIVE_SEL_TEXTCOLOR);
            pdesc = phighlighter->CreateHighlighter(desc);
            SF_ASSERT(pdesc);
            cursorBlinkState = false;
        }
        else
            cursorBlinkState = (pdesc->Length == 0) ? 0 : 1;
        if (pdesc->StartPos != CursorPos || cursorBlinkState != IsCursorBlink())
        {
            pdesc->StartPos = CursorPos;
            pdesc->Length   = IsCursorBlink() ? 1 : 0;
            pDocView->UpdateHighlight(*pdesc);
        }
    }
}
*/
void EditorKit::ClearWideCursor()   
{ 
    Flags &= (~Flags_WideCursor); 

    Highlighter* phighlighter = pDocView->GetHighlighterManager();
    if (phighlighter)
    {
        HighlightDesc* pdesc = NULL;
        if ((pdesc = phighlighter->GetHighlighterPtr(GFX_WIDECURSOR_HIGHLIGHTING_INDEX)) != NULL)
        {
            if (pdesc->Length)
            {
                pdesc->Length = 0;
                pDocView->UpdateHighlight(*pdesc);
            }
        }
    }
}

void EditorKit::AddDrawCursorInfo(TextLayout::Builder& bld)
{
    if (IsCursorBlink() && !IsReadOnly() && !IsWideCursor())
    {
        // check is CursorRect valid
        unsigned lineIndex = SF_MAX_UINT;
        if (!CursorRect.IsValid(pDocView->GetFormatCounter()))
        {
            // not valid, calculate
            RectF    newCursorRect;
            unsigned glyphIndex;
            UPInt    cursorPos = CursorPos;

#ifdef GFX_ENABLE_IME
            // check if composition string is active. If so, check, does it require
            // cursor to be drawn inside the composition string.
            if (HasCompositionString())
            {
                cursorPos += pComposStr->GetCursorPosition();
            }
#endif //#ifdef GFX_ENABLE_IME

            if (CalcAbsCursorRectInLineBuffer(cursorPos, &newCursorRect, &lineIndex, &glyphIndex))
            {
                //const LineBuffer::Line& line = pDocView->LineBuffer[lineIndex];
                //newCursorRect.SetRect(topLeft, SizeF(0.f, float(line.GetHeight())));
                newCursorRect.x2 = newCursorRect.x1; // set width to 0

                // determine the cursor's color
                const TextFormat* pdefFormat = pDocView->GetDefaultTextFormat();

                if (pdefFormat && pdefFormat->IsColorSet())
                    CursorColor = pdefFormat->GetColor();

                // replace the cursor pos by the highlighted text color, if exists
                Highlighter* phighlighter;
                if ((phighlighter = pDocView->GetHighlighterManager()) != NULL)
                {
                    HighlighterPosIterator it = phighlighter->GetPosIterator(CursorPos);
                    const HighlightDesc& desc = *it;
                    if (desc.Info.HasTextColor())
                        CursorColor = desc.Info.GetTextColor();
                }
            }
            else
            {
                // not necessary anymore since we always have at least one char (EOF) DEL!
                newCursorRect.Clear();
                SF_ASSERT(0); // shouldn't happen anymore!
            }

            // CursorRect is in text area coordinate space, where topLeft corner is (0,0)
            // Later, calculating actualCursorRect we will need to translate them to textRect's
            // coordinate space, where topLeft corner may have any coordinates, different from (0,0).
            CursorRect.SetValue(newCursorRect, pDocView->GetFormatCounter());
        }

        RectF actualCursorRect = *CursorRect;
        const RectF& textRect = pDocView->GetTextRect();
        actualCursorRect -= Render::PointF(float(pDocView->GetLineBuffer().GetHScrollOffset()), 
                                           float(pDocView->GetLineBuffer().GetVScrollOffsetInFixp()));
        actualCursorRect += textRect.TopLeft();

        // sometimes, cursor rect may be higher than the actual viewRect, especially in the case
        // of single line edit with big leading value. So, correct the bottom of cursor rect here
        if (actualCursorRect.y2 > textRect.y2)
        {
            if (lineIndex == SF_MAX_UINT)
                lineIndex = pDocView->GetLineIndexOfChar(CursorPos);
            if (pDocView->GetLineBuffer().IsLineVisible(lineIndex))
                actualCursorRect.y2 = textRect.y2;
        }

        if (textRect.Contains(actualCursorRect))
        {
            actualCursorRect.x2 = actualCursorRect.x1 + PixelsToFixp(1.f);
            bld.AddCursor(actualCursorRect, CursorColor.ToColor32());
        }
    }
}

// Draws cursor, if necessary
// @REN
/*
void EditorKit::Display(DisplayContext &context, const Matrix& mat, const Cxform& cx)
{
    Render::Renderer* prenderer = context.GetRenderer();

    // Draw the cursor
    if (IsCursorBlink() && !IsReadOnly() && !IsWideCursor())
    {
        // check is CursorRect valid
        unsigned lineIndex = SF_MAX_UINT;
        if (!CursorRect.IsValid(pDocView->FormatCounter))
        {
            // not valid, calculate
            RectF    newCursorRect;
            unsigned glyphIndex;
            UPInt    cursorPos = CursorPos;

#ifndef SF_NO_IME_SUPPORT
            // check if composition string is active. If so, check, does it require
            // cursor to be drawn inside the composition string.
            if (HasCompositionString())
            {
                cursorPos += pComposStr->GetCursorPosition();
            }
#endif //#ifdef SF_NO_IME_SUPPORT

            if (CalcAbsCursorRectInLineBuffer(cursorPos, &newCursorRect, &lineIndex, &glyphIndex))
            {
                //const LineBuffer::Line& line = pDocView->LineBuffer[lineIndex];
                //newCursorRect.SetRect(topLeft, SizeF(0.f, float(line.GetHeight())));
                newCursorRect.x2 = newCursorRect.x1; // set width to 0

                // determine the cursor's color
                const TextFormat* pdefFormat = pDocView->GetDefaultTextFormat();

                if (pdefFormat && pdefFormat->IsColorSet())
                    CursorColor = pdefFormat->GetColor();

                // replace the cursor pos by the highlighted text color, if exists
                Highlighter* phighlighter;
                if ((phighlighter = pDocView->GetHighlighterManager()) != NULL)
                {
                    HighlighterPosIterator it = phighlighter->GetPosIterator(CursorPos);
                    const HighlightDesc& desc = *it;
                    if (desc.Info.HasTextColor())
                        CursorColor = desc.Info.GetTextColor();
                }
            }
            else
            {
                // not necessary anymore since we always have at least one char (EOF) DEL!
                newCursorRect.Clear();
                SF_ASSERT(0); // shouldn't happen anymore!
            }

            // CursorRect is in text area coordinate space, where topLeft corner is (0,0)
            // Later, calculating actualCursorRect we will need to translate them to textRect's
            // coordinate space, where topLeft corner may have any coordinates, different from (0,0).
            CursorRect.SetValue(newCursorRect, pDocView->FormatCounter);
        }

        RectF actualCursorRect = *CursorRect;
        const RectF& textRect = pDocView->GetTextRect();
        actualCursorRect -= Render::PointF(float(pDocView->LineBuffer.GetHScrollOffset()), float(pDocView->LineBuffer.GetVScrollOffsetInTwips()));
        actualCursorRect += textRect.TopLeft();

        // sometimes, cursor rect may be higher than the actual viewRect, especially in the case
        // of single line edit with big leading value. So, correct the bottom of cursor rect here
        if (actualCursorRect.y2 > textRect.y2)
        {
            if (lineIndex == SF_MAX_UINT)
                lineIndex = pDocView->GetLineIndexOfChar(CursorPos);
            if (pDocView->LineBuffer.IsLineVisible(lineIndex))
                actualCursorRect.y2 = textRect.y2;
        }

        if (textRect.Contains(actualCursorRect))
        {
            actualCursorRect.x2 = actualCursorRect.x1 + 20.0f;

            Matrix m(mat);
            RectF newRect;
            RecalculateRectToFit16Bit(m, actualCursorRect, &newRect);
            prenderer->SetMatrix(m);

            prenderer->SetCxform(cx);

            // snap to pixel the x value
            Matrix mm(m); 
            mm.Append(context.ViewportMatrix);
            Matrix invmm(mm);
            invmm.Invert();

            Render::PointF p1 = mm.Transform(newRect.TopLeft());
            p1.x = floorf(p1.x + 0.5f);
            newRect.x1 = invmm.Transform(p1).x;

            // calculate the width of one pixel in twips
            ++p1.x;
            p1 = invmm.Transform(p1);
            newRect.x2 = p1.x;

            Color  transformedColor    = cx.Transform(CursorColor);

            // Force the size of the cursor width
            // TODO: clamp to viewport to prevent cursor from becoming less than 1 pixel

            Render::PointF         coords[4];
            static const UInt16   indices[6] = { 0, 1, 2, 2, 1, 3 };

            coords[0] = newRect.TopLeft();
            coords[1] = newRect.TopRight();
            coords[2] = newRect.BottomLeft();
            coords[3] = newRect.BottomRight();

            Render::Renderer::VertexXY16i icoords[4];
            icoords[0].x = (SInt16) floorf(coords[0].x + 0.5f);
            icoords[0].y = (SInt16) coords[0].y;
            icoords[1].x = (SInt16) floorf(coords[1].x + 0.5f);
            icoords[1].y = (SInt16) coords[1].y;
            icoords[2].x = (SInt16) floorf(coords[2].x + 0.5f);
            icoords[2].y = (SInt16) coords[2].y;
            icoords[3].x = (SInt16) floorf(coords[3].x + 0.5f);
            icoords[3].y = (SInt16) coords[3].y;

            // Draw the cursor rect
            prenderer->FillStyleColor(transformedColor);
            prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);            
            prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
            prenderer->DrawIndexedTriList(0, 0, 4, 0, 2);
            // Done
            prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
            prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
        }
    }
}
*/

void    EditorKit::ResetBlink(bool state, bool blocked)
{
    if (!IsReadOnly())
    {
        if (state)
            SetCursorBlink();
        else
            ClearCursorBlink();
    }
    else
    {
        ClearCursorBlink();
    }
    CursorTimer = 0.0f;
    if (blocked)
        SetCursorTimerBlocked();
    // do not clear "blocked" flag; it will be cleared at the next advance
    //else
    //    ClearCursorTimerBlocked();
}

// scrolls view to make cursor visible
bool EditorKit::ScrollToPosition(UPInt pos, bool avoidComposStr, bool wideCursor)
{
    RectF    newCursorRect;
    unsigned lineIndex;
    bool     rv = false;
    LineBuffer::Line::Alignment lineAlignment;

    if (CalcCursorRectOnScreen(pos, &newCursorRect, &lineIndex, NULL, avoidComposStr, &lineAlignment))
    {
        const RectF& textRect = pDocView->GetTextRect();

        if (!wideCursor)
            newCursorRect.SetWidth(20.f);

        // Check, is the cursor fully visible or not. If not - scroll horizontally. 
        // Note, if text auto size mode is on, then scrolling is forbidden.
        if (!textRect.Contains(newCursorRect) && pDocView->GetTextAutoSize() == DocView::TAS_None)
        {
            // cursor is invisible, calculate scrolling
            // hscroll first
            int hscroll = (int)pDocView->GetLineBuffer().GetHScrollOffset();
            int prevHScroll = hscroll;

            int delta = GFX_EDIT_HSCROLL_DELTA;
            if (lineAlignment != LineBuffer::Line::Align_Left)
                delta = 0;
            if (newCursorRect.x2 > textRect.x2)
            {
                // make sure whole cursor is visible.
                hscroll += int(newCursorRect.x1 - textRect.x2 + delta);

                RectF lastChar;
                if (pos > 0 && pDocView->GetExactCharBoundaries(&lastChar, pos - 1))
                    hscroll = Alg::Min((int)(lastChar.x1 - GFX_TEXT_GUTTER), hscroll);
                hscroll = Alg::Max(0, hscroll);
            }
            else if (newCursorRect.x1 < textRect.x1)
            {
                hscroll -= int(textRect.x1 - newCursorRect.x1) + delta;
                hscroll = Alg::Max(0, hscroll);
            }
            SF_ASSERT(hscroll >= 0);

            // do not scroll left horizontally, if word-wrapping is on
            if (!pDocView->IsWordWrap() || hscroll < prevHScroll)
                rv = pDocView->SetHScrollOffset(unsigned(hscroll));

            // calculate new vscroll
            unsigned fvline = pDocView->GetVScrollOffsetValue();
            if (lineIndex < fvline)
            {
                rv |= pDocView->SetVScrollOffset(lineIndex);
            }
            else
            {
                unsigned bottomV = pDocView->GetBottomVScroll();
                if (lineIndex > bottomV)
                {
                    rv |= pDocView->SetBottomVScroll(lineIndex);
                }
            }
        }
    }
    return rv;
}

void    EditorKit::OnMouseDown(float x, float y, int buttons)
{
    if (buttons & 1) // left mouse button is down
    {
        float ax = x;
        float ay = y;
        const RectF& r = pDocView->GetViewRect();
        ax = floor(ax - r.x1);
        ay = floor(ay - r.y1);
        UInt32 tm = (UInt32)(Timer::GetTicks() / 1000);
        
        bool doubleClicked = false, tripleClick = false;
        // detect double click
        if (LastMousePos.x == ax && LastMousePos.y == ay && tm <= LastClickTime + GFX_DOUBLE_CLICK_TIME_MS)
        {
            if (IsLastDoubleClickFlagSet())
                tripleClick = true;
            else
                doubleClicked = true;
            SetLastDoubleClickFlag();
            //(tripleClick) ? printf("TRIPLE CLICK!\n") : printf("DOUBLE CLICK!\n");
        }
        else
            ClearLastDoubleClickFlag();
        //printf("new x = %f    y = %f     tm = %d\n", ax, ay, tm);
        LastMousePos.x = ax;
        LastMousePos.y = ay;
        LastClickTime  = tm;

        UPInt pos = pDocView->GetCursorPosAtPoint(ax, ay);
        if (pos != SF_MAX_UPINT)
        {
            SetCursorPos(pos);
            if (IsSelectable() && !IsMouseCaptured())
            {
                SetMouseCaptured();
                if (doubleClicked || tripleClick)
                {
                    // handle double- and triple- clicks
                    UPInt indexInPara;
                    StyledText::ParagraphsIterator it = pDocView->GetStyledText()->GetParagraphByIndex(pos, &indexInPara);
                    if (!it.IsFinished())
                    {
                        Paragraph* ppara = *it;
                        UPInt textLen = ppara->GetLength();
                        UPInt st = indexInPara, en = indexInPara;
                        if (doubleClicked)
                        {
                            // find boundaries of word and select it
                            const wchar_t* ptext = ppara->GetText();
                            for (; st > 0; --st)
                            {
                                if (SFiswspace(ptext[st - 1]) || !SFiswalnum(ptext[st - 1]))
                                    break;
                            }
                            for (; en < textLen; ++en)
                            {
                                if (SFiswspace(ptext[en]) || !SFiswalnum(ptext[en]))
                                    break;
                            }
                        }
                        else
                        {
                            // select entire paragraph
                            st = 0; 
                            en = textLen;
                        }
                        UPInt paraStartIndex = ppara->GetStartIndex();
                        st += paraStartIndex;
                        en += paraStartIndex;
                        SetCursorPos(en);
                        pDocView->SetSelection(st, en);
                    }
                }
                else if (!IsShiftPressed())
                    pDocView->SetSelection(pos, pos);
            }
        }
    }
}

void    EditorKit::OnMouseUp(float x, float y, int buttons)
{
    SF_UNUSED2(x, y);
    if (!(buttons & 1)) // left mouse button is up
    {
        if (IsSelectable() && IsMouseCaptured())
        {
            ClearMouseCaptured();
            //ClearShiftPressed();
        }
    }
}

void    EditorKit::OnMouseMove(float x, float y)
{
    if (IsMouseCaptured()) 
    {
        float ax = x;// + float(pDocView->LineBuffer.GetHScrollOffset());
        float ay = y;// + float(pDocView->LineBuffer.GetVScrollOffsetInTwips()) - pDocView->LineBuffer.GetVScrollOffsetInTwips();
        const RectF& r = pDocView->GetViewRect();
        ax = floor(ax - r.x1);
        ay = floor(ay - r.y1);
        LastMousePos.x = ax;
        LastMousePos.y = ay;

        UPInt pos = pDocView->GetCursorPosAtPoint(ax, ay);
        if (pos != SF_MAX_UPINT)
        {
            //printf("%d x %f y %f\n", pos, (float)ax, (float)ay);
            SetCursorPos(pos);
            if (IsSelectable())
                pDocView->SetEndSelection(pos);
        }
    }
}

bool EditorKit::OnKeyUp(int keyCode, const KeyModifiers& specKeysState)
{
    SF_UNUSED(specKeysState);

    if (pKeyMap)
    {
        const TextKeyMap::KeyMapEntry* pke = pKeyMap->Find(keyCode, specKeysState, TextKeyMap::State_Up);
        if (pke)
        {
            switch(pke->Action)
            {
            case TextKeyMap::KeyAct_LeaveSelectionMode:
                if (IsSelectable() && IsShiftPressed())
                {
                    ClearShiftPressed();
                }
                break;
            default: break;
            }
        }
    }
    return true;
}

bool EditorKit::OnKeyDown(int keyCode, const KeyModifiers& specKeysState)
{
    DocView* pdocument = GetDocument();
    UPInt len = pdocument->GetStyledText()->GetLength();
    bool rv = false;
    UPInt newPos = CursorPos;

    if (pdocument->GetDocumentListener())
    {
        keyCode = pdocument->GetDocumentListener()->Editor_OnKey(*this, keyCode);
    }
    bool  needChangeNotify = false, noActions = false, isShiftPressed = IsShiftPressed();
    float lastHorizCursorPos = -1;
    UPInt beginSel = pDocView->GetBeginSelection(), 
          endSel   = pDocView->GetEndSelection();

    if (pKeyMap)
    {
        const TextKeyMap::KeyMapEntry* pke = pKeyMap->Find(keyCode, specKeysState, TextKeyMap::State_Down);
        if (!pke)
            noActions = true;
        else
        {
            switch(pke->Action)
            {
            case TextKeyMap::KeyAct_EnterSelectionMode:
                if (IsSelectable())
                {
                    isShiftPressed = true;
                }
                noActions = true;
                break;
            case TextKeyMap::KeyAct_Left:
                {
                    if (newPos == SF_MAX_UPINT)
                        newPos = len;
                    if (newPos > 0)
                        newPos -= 1;
                    if (!IsShiftPressed())
                    {
                        // not selecting but BegSel != EndSel? Jump to the lowest pos
                        if (beginSel != endSel)
                            newPos = Alg::PMin(beginSel, endSel);
                    }
                    break;
                }
                // x2 arrow key
            case TextKeyMap::KeyAct_Right:
                {
                    if (newPos == SF_MAX_UPINT)
                        newPos = len;
                    else if (newPos < len)
                        newPos += 1;
                    if (!IsShiftPressed())
                    {
                        // not selecting but BegSel != EndSel? Jump to the greatest pos
                        if (beginSel != endSel)
                            newPos = Alg::PMax(beginSel, endSel);
                    }
                    break;
                }
            case TextKeyMap::KeyAct_LineHome:
                {
                    unsigned lineIndex = pDocView->GetLineIndexOfChar(CursorPos);
                    if (lineIndex != ~0u)
                    {
                        newPos = pDocView->GetLineOffset(lineIndex);
                        SF_ASSERT(newPos != SF_MAX_UPINT);
                    }
                    break;
                }
            case TextKeyMap::KeyAct_LineEnd:
                {
                    unsigned lineIndex = pDocView->GetLineIndexOfChar(CursorPos);
                    if (lineIndex != ~0u)
                    {
                        SF_ASSERT(pDocView->GetLineOffset(lineIndex) != SF_MAX_UPINT && pDocView->GetLineLength(lineIndex) != SF_MAX_UPINT);
                        bool hasNewLine = false;
                        UPInt len = pDocView->GetLineLength(lineIndex, &hasNewLine);
                        if (hasNewLine)
                            --len;
                        newPos = pDocView->GetLineOffset(lineIndex) + len;
                    }
                }
                break;

            case TextKeyMap::KeyAct_DocHome:
                newPos = 0;
                break;

            case TextKeyMap::KeyAct_DocEnd:
                newPos = len;
                break;

            case TextKeyMap::KeyAct_PageHome:
                {
                    unsigned lineIndex = pDocView->GetVScrollOffsetValue();
                    newPos = pDocView->GetLineOffset(lineIndex);
                    SF_ASSERT(newPos != SF_MAX_UPINT);
                    break;
                }
            case TextKeyMap::KeyAct_PageEnd:
                {
                    unsigned lineIndex = pDocView->GetBottomVScroll();
                    bool     hasNewLine = false;
                    UPInt    len = pDocView->GetLineLength(lineIndex, &hasNewLine);
                    if (hasNewLine)
                        --len;
                    newPos = pDocView->GetLineOffset(lineIndex) + len;
                    break;
                }
            case TextKeyMap::KeyAct_PageUp:
                {
                    RectF    cursorRect;
                    unsigned lineIndex;

                    if (CalcCursorRectOnScreen(CursorPos, &cursorRect, &lineIndex))
                    {
                        if (LastHorizCursorPos < 0)
                            lastHorizCursorPos = cursorRect.x1;
                        else
                            lastHorizCursorPos = LastHorizCursorPos;

                        unsigned bottomV = pDocView->GetBottomVScroll();
                        unsigned numLinesOnScreen = bottomV - pDocView->GetVScrollOffsetValue() + 1;
                        if (lineIndex < numLinesOnScreen)
                            newPos = 0;
                        else
                        {
                            lineIndex -= numLinesOnScreen;
                            newPos = pDocView->GetCursorPosInLine(lineIndex, lastHorizCursorPos);
                        }
                        SF_ASSERT(newPos != SF_MAX_UPINT);
                    }
                    break;
                }
            case TextKeyMap::KeyAct_PageDown:
                {
                    RectF    cursorRect;
                    unsigned lineIndex;

                    if (CalcCursorRectOnScreen(CursorPos, &cursorRect, &lineIndex))
                    {
                        if (LastHorizCursorPos < 0)
                            lastHorizCursorPos = cursorRect.x1;
                        else
                            lastHorizCursorPos = LastHorizCursorPos;

                        unsigned bottomV = pDocView->GetBottomVScroll();
                        unsigned numLinesOnScreen = bottomV - pDocView->GetVScrollOffsetValue() + 1;
                        lineIndex += numLinesOnScreen;
                        if (lineIndex >= pDocView->GetLinesCount())
                            newPos = len;
                        else
                            newPos = pDocView->GetCursorPosInLine(lineIndex, lastHorizCursorPos);
                        SF_ASSERT(newPos != SF_MAX_UPINT);
                    }
                    break;
                }

            case TextKeyMap::KeyAct_Up:
                {
                    RectF    cursorRect;
                    unsigned lineIndex;

                    if (!IsShiftPressed() && beginSel != endSel)
                    {
                        // not selecting but BegSel != EndSel? Jump to the lowest pos
                        newPos = Alg::PMin(beginSel, endSel);
                    }
                    else if (CalcCursorRectOnScreen(CursorPos, &cursorRect, &lineIndex))
                    {
                        if (lineIndex > 0)
                        {
                            if (LastHorizCursorPos < 0)
                                lastHorizCursorPos = cursorRect.x1;
                            else
                                lastHorizCursorPos = LastHorizCursorPos;

                            --lineIndex;
                            newPos = pDocView->GetCursorPosInLine(lineIndex, lastHorizCursorPos);
                            SF_ASSERT(newPos != SF_MAX_UPINT);
                        }
                        else
                            newPos = 0;
                    }
                    break;
                }
            case TextKeyMap::KeyAct_Down:
                {
                    RectF    cursorRect;
                    unsigned lineIndex;

                    if (!IsShiftPressed() && beginSel != endSel)
                    {
                        // not selecting but BegSel != EndSel? Jump to the greatest pos
                        newPos = Alg::PMax(beginSel, endSel);
                    }
                    else if (CalcCursorRectOnScreen(CursorPos, &cursorRect, &lineIndex))
                    {
                        if (lineIndex + 1 < pDocView->GetLinesCount())
                        {
                            if (LastHorizCursorPos < 0)
                                lastHorizCursorPos = cursorRect.x1;
                            else
                                lastHorizCursorPos = LastHorizCursorPos;

                            ++lineIndex;
                            newPos = pDocView->GetCursorPosInLine(lineIndex, lastHorizCursorPos);
                            SF_ASSERT(newPos != SF_MAX_UPINT);
                        }
                        else
                            newPos = len;
                    }
                    break;
                }

            case TextKeyMap::KeyAct_Backspace:
                {
                    if (!IsReadOnly())
                    {
                        if (beginSel == endSel)
                        {
                            DocView::DeleteCharCommand cmd(newPos);
                            newPos -= pDocView->EditCommand(DocView::Cmd_BackspaceChar, &cmd);
                            SF_ASSERT(SPInt(newPos) >= 0);
                            needChangeNotify = true;
                        }
                        else
                        {
                            DocView::DeleteTextCommand cmd(beginSel, endSel);
                            pDocView->EditCommand(DocView::Cmd_DeleteText, &cmd);
                            newPos = Alg::PMin(beginSel, endSel);
                            needChangeNotify = true;
                        }
                    }
                    break;
                }
            case TextKeyMap::KeyAct_Delete:
                {
                    if (!IsReadOnly())
                    {
                        if (beginSel == endSel)
                        {
                            if (CursorPos < len)
                            {
                                DocView::DeleteCharCommand cmd(newPos);
                                pDocView->EditCommand(DocView::Cmd_DeleteChar, &cmd);
                                needChangeNotify = true;
                            }
                        }
                        else
                        {
                            DocView::DeleteTextCommand cmd(beginSel, endSel);
                            pDocView->EditCommand(DocView::Cmd_DeleteText, &cmd);
                            newPos = Alg::PMin(beginSel, endSel);
                            needChangeNotify = true;
                        }
                    }
                    break;
                }
            case TextKeyMap::KeyAct_Return:
                {
                    if (!IsReadOnly())
                    {
                        if (pDocView->IsMultiline())
                        {
                            ClearShiftPressed();
                            if (beginSel == endSel)
                            {
                                DocView::InsertCharCommand cmd(CursorPos, pDocView->NewLineChar());
                                pDocView->EditCommand(DocView::Cmd_InsertChar, &cmd);
                                ++newPos;
                                needChangeNotify = true;
                            }
                            else
                            {
                                // replace the selection by the input
                                DocView::ReplaceTextByCharCommand cmd(beginSel, endSel, pDocView->NewLineChar());
                                pDocView->EditCommand(DocView::Cmd_ReplaceTextByChar, &cmd);
                                newPos = Alg::PMin(beginSel, endSel) + 1;
                                needChangeNotify = true;
                            }
                        }
                    }
                    break;
                }

            case TextKeyMap::KeyAct_SelectAll:
                {
                    beginSel = 0;
                    endSel   = newPos = len;
                    SetCursorPos(newPos);
                    pDocView->SetSelection(beginSel, endSel);
                    noActions = true;
                }
                break;
            case TextKeyMap::KeyAct_Copy:
            case TextKeyMap::KeyAct_Cut:
                if (pClipboard)
                {
                    if (IsReadOnly() || pke->Action == TextKeyMap::KeyAct_Copy)
                    {
                        ClearShiftPressed();
                        CopyToClipboard(beginSel, endSel, DoesUseRichClipboard());
                        noActions = true;
                    }
                    else
                    {
                        ClearShiftPressed();
                        CutToClipboard(beginSel, endSel, DoesUseRichClipboard());
                        newPos = Alg::PMin(beginSel, endSel);
                        needChangeNotify = true;
                    }
                }
                break;
            case TextKeyMap::KeyAct_Paste:
                {
                    UPInt _newPos = PasteFromClipboard(beginSel, endSel, DoesUseRichClipboard());
                    if (_newPos != SF_MAX_UPINT)
                    {
                        newPos = _newPos;
                        needChangeNotify = true;
                    }
                }
                break;

            default: noActions = true; break;
            }
        }
    }
    else
        noActions = true;

#ifdef SF_BUILD_DEBUG
    if (noActions)
    {
        switch(keyCode)
        {
        case 19: // pause
            {
                pDocView->GetLineBuffer().Dump();
                break;
            }
            /*case 192:
            {
            pDocView->pDocument->GetStyledText()->Remove(1, 20);
            }
            break;*/
            /*case 192: // `
            //pDocView->pDocument->GetStyledText()->InsertString(L"AAAA\nBBBB\n334", 10, ~0u);
            pDocView->pDocument->GetStyledText()->InsertString(L"AAAA\nBBBB\nTheEND! ", 0, ~0u);
            break;*/
        default: break;
        }
    }
#endif

    if (!noActions)
    {
        if (CursorPos != newPos)
        {
            SetCursorPos(newPos);
            LastHorizCursorPos = lastHorizCursorPos;
            rv = true;
        }
        else if ((!IsShiftPressed() && beginSel != endSel))
        {
            SetCursorPos(newPos);
            rv = true;
        }

        if (needChangeNotify)
        {
            if (!rv)
                pDocView->SetDefaultTextAndParaFormat(CursorPos);

            if (pdocument->GetDocumentListener())
            {
                pdocument->GetDocumentListener()->Editor_OnChanged(*this);
            }
        }
    }
    if (isShiftPressed) // set selecting mode back
        SetShiftPressed();
    return rv;
}

bool EditorKit::OnChar(UInt32 wcharCode)
{
    if (!wcharCode || IsMouseCaptured())
        return false;
    DocView* pdocument = GetDocument();
    bool rv = false;
    UPInt newPos = CursorPos;

    UPInt beginSel = pDocView->GetBeginSelection(), 
          endSel   = pDocView->GetEndSelection();

    bool needChangeNotify = false;
    if (!IsReadOnly())
    {
        wchar_t wc;
        wc = (wchar_t)wcharCode;
        if (wc >= 32)
        {
            //@TODO - remove
            //if (wc && pdocument->GetDocumentListener())
            //{
            //    wc = pdocument->GetDocumentListener()->Editor_OnCharacter(*this, wc);
            //}
            //if (wc)
            {
                //ClearShiftPressed();
                if (beginSel == endSel && IsOverwriteMode())
                {
                    ++endSel;
                }
                if (beginSel == endSel)
                {
                    // regular input
                    DocView::InsertCharCommand cmd(CursorPos, wc);
                    newPos += pDocView->EditCommand(DocView::Cmd_InsertChar, &cmd);
                    needChangeNotify = true;
                }
                else
                {
                    // replace the selection by the input
                    DocView::ReplaceTextByCharCommand cmd(beginSel, endSel, wc);
                    UPInt res = pDocView->EditCommand(DocView::Cmd_ReplaceTextByChar, &cmd);
                    newPos = Alg::PMin(beginSel, endSel) + res;
                    needChangeNotify = true;
                }
            }
            if (CursorPos != newPos || beginSel != newPos || endSel != newPos)
            {
                SetCursorPos(newPos, false);
                rv = true;
            }
            if (needChangeNotify)
            {
                //if (IsSelectable())
                //    pDocView->SetSelection(beginSel, endSel);
                if (pdocument->GetDocumentListener())
                {
                    pdocument->GetDocumentListener()->Editor_OnChanged(*this);
                }
            }
        }
    }

    return rv;
}

void EditorKit::OnDocumentChanged(unsigned notifyMask)
{
    if (notifyMask & DocView::ViewNotify_SignificantTextChange)
    {
        if (!IsReadOnly() || IsSelectable())
        {
            //if (!pDocView->IsMultiline())
            //    SetCursorPos(pDocView->GetLength());
            //else
            SetCursorPos(0);
        }
    }
    else
    {
        if (CursorPos > pDocView->GetLength())
            SetCursorPos(pDocView->GetLength());
    }
}

void EditorKit::OnSetFocus()
{
    pDocView->SetSelectionTextColor(ActiveSelectionTextColor);
    pDocView->SetSelectionBackgroundColor(ActiveSelectionBkColor);
    Flags |= Flags_Focused;
}

void EditorKit::OnKillFocus()
{
    pDocView->SetSelectionTextColor(InactiveSelectionTextColor);
    pDocView->SetSelectionBackgroundColor(InactiveSelectionBkColor);
    ClearShiftPressed();
    ClearMouseCaptured();
    Flags &= ~Flags_Focused;
}

void EditorKit::SetSelection(UPInt startPos, UPInt endPos)
{
    if (!IsReadOnly() || IsSelectable())
    {
        SetCursorPos(endPos);
    }
    pDocView->SetSelection(startPos, endPos);
}

void EditorKit::CopyToClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard)
{
    if (pClipboard && !pDocView->IsPasswordMode())
    {
        if (endPos < startPos)
        {
            UPInt t = endPos;
            endPos = startPos;
            startPos = t;
        }

        // copy to clipboard
        WStringBuffer wbuf;
        pDocView->GetStyledText()->GetText(&wbuf, startPos, endPos);
        if (useRichClipboard)
        {
            Ptr<StyledText> ptext = 
                *pDocView->GetStyledText()->CopyStyledText(startPos, endPos);
            pClipboard->SetTextAndStyledText(wbuf.ToWStr(), wbuf.GetLength(), ptext);
        }
        else
            pClipboard->SetText(wbuf.ToWStr(), wbuf.GetLength());
    }
}

void EditorKit::CutToClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard)
{
    if (pClipboard)
    {
        if (endPos < startPos)
        {
            UPInt t = endPos;
            endPos = startPos;
            startPos = t;
        }
        CopyToClipboard(startPos, endPos, useRichClipboard);
        if (!IsReadOnly())
            pDocView->RemoveText(startPos, endPos);
    }
}

// if successful, returns new projected cursor position; SF_MAX_UPINT otherwise.
UPInt EditorKit::PasteFromClipboard(UPInt startPos, UPInt endPos, bool useRichClipboard)
{
    UPInt newPos = SF_MAX_UPINT;
    if (!IsReadOnly() && pClipboard)
    {
        if (endPos < startPos)
        {
            UPInt t = endPos;
            endPos = startPos;
            startPos = t;
        }
        // paste from clipboard
        if (useRichClipboard && pClipboard->ContainsRichText())
        {
            // rich text
            Ptr<StyledText> pstr = pClipboard->GetStyledText();
            if (pstr && pstr->GetLength() > 0)
            {
                ClearShiftPressed();
                if (startPos == endPos)
                {
                    DocView::InsertStyledTextCommand cmd(startPos, pstr);
                    UPInt len = pDocView->EditCommand(DocView::Cmd_InsertStyledText, &cmd);
                    newPos = startPos + len;
                }
                else
                {
                    // replace the selection by the input
                    DocView::ReplaceTextByStyledTextCommand cmd(startPos, endPos, pstr);
                    UPInt len = pDocView->EditCommand(DocView::Cmd_ReplaceTextByStyledText, &cmd);
                    newPos = startPos + len;
                }
            }
        }
        else
        {
            // plain text
            const WStringBuffer& str = pClipboard->GetText();
            if (str.GetLength() > 0)
            {
                ClearShiftPressed();
                if (startPos == endPos)
                {
                    DocView::InsertPlainTextCommand cmd(startPos, str.ToWStr(), str.GetLength());
                    UPInt len = pDocView->EditCommand(DocView::Cmd_InsertPlainText, &cmd);
                    newPos = startPos + len;
                }
                else
                {
                    // replace the selection by the input
                    DocView::ReplaceTextByPlainTextCommand cmd
                        (startPos, endPos, str.ToWStr(), str.GetLength());
                    UPInt len = pDocView->EditCommand(DocView::Cmd_ReplaceTextByPlainText, &cmd);
                    newPos = startPos + len;
                }
            }
        }
    }
    return newPos;
}

void EditorKit::InvalidateSelectionColors()
{
    if (Flags & Flags_Focused)
    {
        pDocView->SetSelectionTextColor(ActiveSelectionTextColor);
        pDocView->SetSelectionBackgroundColor(ActiveSelectionBkColor);
    }
    else
    {
        pDocView->SetSelectionTextColor(InactiveSelectionTextColor);
        pDocView->SetSelectionBackgroundColor(InactiveSelectionBkColor);
    }
}


#ifndef SF_NO_IME_SUPPORT
CompositionString* EditorKit::CreateCompositionString()
{
    pComposStr = *SF_HEAP_AUTO_NEW(this) CompositionString(this);
    return pComposStr;
}

void EditorKit::ReleaseCompositionString()
{
    if (pComposStr)
    {
        Ptr<CompositionString> pcs = pComposStr;
        pComposStr = NULL;
        pcs->Reformat(); // schedule reformatting of src paragraph
    }
}
#endif //#ifdef SF_NO_IME_SUPPORT

UPInt EditorKit::TextPos2GlyphOffset(UPInt textPos)
{
#ifndef SF_NO_IME_SUPPORT
    if (HasCompositionString())
    {
        if (textPos > pComposStr->GetPosition())
        {
            return textPos + pComposStr->GetLength();
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    return textPos;
}

UPInt EditorKit::TextPos2GlyphPos(UPInt textPos)
{

#ifndef SF_NO_IME_SUPPORT
    if (HasCompositionString())
    {
        if (textPos >= pComposStr->GetPosition())
        {
            return textPos + pComposStr->GetLength();
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    return textPos;
}

UPInt EditorKit::GlyphPos2TextPos(UPInt glyphPos)
{

#ifndef SF_NO_IME_SUPPORT
    if (HasCompositionString())
    {
        if (glyphPos <= pComposStr->GetPosition())
            return glyphPos;
        if (glyphPos >= pComposStr->GetPosition() + pComposStr->GetLength())
        {
            return glyphPos - pComposStr->GetLength();
        }
        else
            return pComposStr->GetPosition();
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    return glyphPos;
}

#ifndef SF_NO_IME_SUPPORT
//////////////////////////////////////////////////////////////////////////
// CompositionString
//
CompositionString::CompositionString(EditorKit* peditorKit) :
    pEditorKit(peditorKit), pAllocator(pEditorKit->GetDocument()->GetAllocator()), 
    String(peditorKit->GetDocument()->GetAllocator()),
    CursorPos(0), HasHighlightingFlag(false), HighlightIdsUsed(0)
{
    TextFormat fmt(pAllocator->GetHeap()); // create empty text format
    const TextFormat* pdefFmt = peditorKit->GetDocument()->GetDefaultTextFormat();
    if (!pdefFmt)
        pdefFmt = &fmt;
    pDefaultFormat = *GetAllocator()->AllocateTextFormat(*pdefFmt);

    String.AppendTermNull(GetAllocator(), pDefaultFormat);

    // create highlighting for composition string

    Highlighter* phighlighter = pEditorKit->GetDocument()->CreateHighlighterManager();
    SF_ASSERT(phighlighter);
    for (UPInt i = 0; i < sizeof(HighlightIds)/sizeof(HighlightIds[0]); i++)
    {
        HighlightDesc* pdesc;
        HighlightDesc desc;
        desc.StartPos = 0;
        desc.Offset   = 0;
        desc.Length   = 0;
        pdesc = phighlighter->CreateNewHighlighter(&desc);
        SF_ASSERT(pdesc);
        HighlightIds[i] = pdesc->Id;
    }

    // set default styles
    Styles = GetDefaultStyles();
}

CompositionString::~CompositionString()
{
    Highlighter* phighlighter = pEditorKit->GetDocument()->CreateHighlighterManager();
    SF_ASSERT(phighlighter);
    for (UPInt i = 0; i < sizeof(HighlightIds)/sizeof(HighlightIds[0]); i++)
    {
        phighlighter->FreeHighlighter(HighlightIds[i]);
    }
    String.FreeText(GetAllocator());
}

IMEStyle CompositionString::GetDefaultStyles()
{
    IMEStyle styles;
    styles.SetElement(IMEStyle::SC_CompositionSegment, 
        HighlightInfo(HighlightInfo::Underline_Dotted));
    styles.SetElement(IMEStyle::SC_ClauseSegment,      
        HighlightInfo(HighlightInfo::Underline_Thick));
    styles.SetElement(IMEStyle::SC_ConvertedSegment,   
        HighlightInfo(HighlightInfo::Underline_Single));
    HighlightInfo plainfo;
    plainfo.SetBackgroundColor(GFX_ACTIVE_SEL_BKCOLOR);
    plainfo.SetTextColor(GFX_ACTIVE_SEL_TEXTCOLOR);
    styles.SetElement(IMEStyle::SC_PhraseLengthAdj, plainfo);
    HighlightInfo lcsinfo(HighlightInfo::Underline_Single);
    lcsinfo.SetUnderlineColor(Color(0,255,0));
    styles.SetElement(IMEStyle::SC_LowConfSegment, lcsinfo);
    return styles;
}

Paragraph* CompositionString::GetSourceParagraph()
{
    UPInt pos = GetPosition();
    StyledText::ParagraphsIterator it = 
        pEditorKit->GetDocument()->GetStyledText()->GetParagraphByIndex(pos, NULL);
    if (!it.IsFinished())
        return *it;
    return NULL;
}

void CompositionString::Reformat()
{
    Paragraph* ppara = GetSourceParagraph();
    if (ppara)
    {
        ppara->MarkToReformat();
        pEditorKit->GetDocument()->SetReformatReq();
    }
}

void CompositionString::SetText(const wchar_t* pwstr, UPInt nchars)
{
    ClearHighlighting();
    String.Clear();
    String.SetText(GetAllocator(), pwstr, nchars);
    String.SetTextFormat(GetAllocator(), *pDefaultFormat, 0, String.GetSize());
    String.AppendTermNull(GetAllocator(), pDefaultFormat);

    String.CheckIntegrity();
    Reformat();
}

void CompositionString::SetPosition(UPInt pos)
{
    String.SetStartIndex(pos);

    Reformat();
}

void CompositionString::SetCursorPosition(UPInt pos)
{
    CursorPos = pos;
    pEditorKit->CursorRect.Invalidate();
    pEditorKit->ResetBlink(1);
    pEditorKit->ScrollToPosition(GetPosition() + CursorPos, false, pEditorKit->IsWideCursor());
}

void CompositionString::HighlightText(UPInt pos, UPInt len, IMEStyle::Category styleCategory)
{
    HasHighlightingFlag = true;
    SF_ASSERT(HighlightIdsUsed < sizeof(HighlightIds)/sizeof(HighlightIds[0]));
    if (HighlightIdsUsed < sizeof(HighlightIds)/sizeof(HighlightIds[0]))
    {
        Highlighter* phighlighter = pEditorKit->GetDocument()->CreateHighlighterManager();
        SF_ASSERT(phighlighter);
        HighlightDesc* pdesc = phighlighter->GetHighlighterPtr(HighlightIds[HighlightIdsUsed++]);
        SF_ASSERT(pdesc);
        pdesc->StartPos = GetPosition();
        pdesc->Offset = pos;
        pdesc->Length = len;
        pdesc->Info = Styles.GetElement(styleCategory);
        pEditorKit->GetDocument()->UpdateHighlight(*pdesc);

        //Reformat();

        // we also need to make sure that the highlighted clause is visible
        if (len > 0)
        {
            pEditorKit->ScrollToPosition(GetPosition() + pos + len, false, pEditorKit->IsWideCursor());
            pEditorKit->ScrollToPosition(GetPosition() + pos, false, pEditorKit->IsWideCursor());
        }
        String.CheckIntegrity();
    }
}

void  CompositionString::UseStyles(const IMEStyle& styles)
{
    Styles.Unite(styles);
}

void  CompositionString::ClearHighlighting() 
{ 
    HasHighlightingFlag = false; 

    Highlighter* phighlighter = pEditorKit->GetDocument()->CreateHighlighterManager();
    SF_ASSERT(phighlighter);
    for (UPInt i = 0; i < HighlightIdsUsed; i++)
    {
        HighlightDesc* pdesc = phighlighter->GetHighlighterPtr(HighlightIds[i]);
        SF_ASSERT(pdesc);
        pdesc->StartPos = 0;
        pdesc->Length   = 0;
        pEditorKit->GetDocument()->UpdateHighlight(*pdesc);
    }
    HighlightIdsUsed = 0;
}
#endif //#ifdef SF_NO_IME_SUPPORT
#endif //#ifdef GFX_ENABLE_TEXT_INPUT

}}} // Scaleform::GFx::Text
