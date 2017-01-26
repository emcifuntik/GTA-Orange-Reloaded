/**********************************************************************

Filename    :   DrawText.cpp
Content     :   External text interface
Created     :   May 23, 2008
Authors     :   Artem Bolgar

Notes       :   

Copyright   :   (c) 2005-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/GFx_DrawText.h"
#include "GFx/GFx_FontManager.h"
#include "GFx/Text/Text_DocView.h"
#include "GFx/GFx_LoaderImpl.h"
#include "GFx/GFx_MovieDef.h"
#include "Kernel/SF_AutoPtr.h"


#ifdef GFX_ENABLE_DRAWTEXT

namespace Scaleform { namespace GFx {

using namespace Scaleform::Render;

class DrawTextManagerImpl : public NewOverrideBase<StatMV_Text_Mem>
{
public:
    Ptr<TreeContainer>          pRootNode;
    Ptr<StateBagImpl>           pStateBag;
    Ptr<MovieDef>               pMovieDef;
    Ptr<Text::Allocator>        pTextAllocator;
    Ptr<FontManager>            pFontManager;
    Ptr<FontManagerStates>      pFontStates;
    Ptr<ResourceWeakLib>        pWeakLib;
    //Ptr<AMP::ViewStats>         DisplayContextStats;
    DrawTextManager::TextParams DefaultTextParams;
    Ptr<LoaderImpl>             pLoaderImpl;
    enum
    {
        RTFlags_BeginDisplayInvoked = 1,
    };
    UInt8                       RTFlags; // run-time flags
    // should be defined last to make sure its dtor is called first.
    Context                     RenderContext; 

    DrawTextManagerImpl():RTFlags(0), RenderContext(Memory::GetGlobalHeap())
    {
        //DisplayContextStats = *SF_NEW AMP::ViewStats();
        //DisplayContext.pStats = DisplayContextStats;
    }
};

class DrawTextImpl : public DrawText
{
public:
    Ptr<DrawTextManager>    pDrawTextCtxt;
    Ptr<TreeText>           pTextNode;
    Matrix                  Matrix_1;
    Cxform                  Cxform_1;
    MemoryHeap*             pHeap;
//     struct UserRendererData : public NewOverrideBase<StatMV_Text_Mem>
//     {           
//         String                  StringVal;
//         float                   FloatVal;
//         float                   MatrixVal[16];
//         //Render::Renderer::UserData UserData;
// 
//         UserRendererData() : FloatVal(0.0f) { }
//     };
//    AutoPtr<UserRendererData>  pUserData;
public:
    DrawTextImpl(DrawTextManager* pdtMgr);

    void SetText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(putf8Str, lengthInBytes);
    }
    void SetText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(pstr, lengthInChars);
    }
    void SetText(const String& str)
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetText(str.ToCStr(), str.GetSize());
    }
    String GetText() const
    {
        return pTextNode->GetText();
    }

    void SetHtmlText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetHtmlText(putf8Str, lengthInBytes);
    }
    void SetHtmlText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1))
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetHtmlText(pstr, lengthInChars);
    }
    void SetHtmlText(const String& str)
    {
        pDrawTextCtxt->CheckFontStatesChange();
        pTextNode->SetHtmlText(str.ToCStr(), str.GetLength());
    }
    String GetHtmlText() const
    {
        return pTextNode->GetHtmlText();
    }

    void SetRect(const RectF& viewRect)
    {
        pTextNode->SetBounds(PixelsToTwips(viewRect));
    }
    RectF GetRect() const
    {
        return TwipsToPixels(pTextNode->GetBounds());
    }

    void SetMatrix(const Matrix& matrix)
    {
        Matrix2F m(matrix);
        m.Tx() = PixelsToTwips(Matrix_1.Tx());
        m.Ty() = PixelsToTwips(Matrix_1.Ty());
        pTextNode->SetMatrix(m);
    }
    Matrix GetMatrix() const 
    { 
        Matrix m(pTextNode->M2D());
        m.Tx() = TwipsToPixels(m.Tx());
        m.Ty() = TwipsToPixels(m.Ty());
        return m; 
    }

    void SetCxform(const Cxform& cx)
    {
        pTextNode->SetCxform(cx);
    }
    const Cxform& GetCxform() const
    {
        return pTextNode->GetCxform();
    }

    void SetColor(Color c, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetColor(c, startPos, endPos);
    }
    void SetFont (const char* pfontName, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFont(pfontName, startPos, endPos);
    }
    void SetFontSize(float fontSize, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFontSize(fontSize, startPos, endPos);
    }
    void SetFontStyle(FontStyle fontStyle, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetFontStyle(fontStyle, startPos, endPos);
    }
    void SetUnderline(bool underline, UPInt startPos = 0, UPInt endPos = UPInt(-1))
    {
        pTextNode->SetUnderline(underline, startPos, endPos);
    }
    void SetMultiline(bool multiline)
    {
        pTextNode->SetMultiline(multiline);
    }
    bool IsMultiline() const
    {
        return pTextNode->IsMultiline();
    }

    // Turns wordwrapping on/off
    void SetWordWrap(bool wordWrap)
    {
        pTextNode->SetWordWrap(wordWrap);
    }
    // Returns state of wordwrapping.
    bool IsWordWrap() const
    {
        return pTextNode->IsWordWrap();
    }

    void SetAlignment(Alignment a)
    {
        pTextNode->SetAlignment(a);
    }
    Alignment  GetAlignment() const
    {
        return pTextNode->GetAlignment();
    }

    void SetVAlignment(VAlignment a)
    {
        pTextNode->SetVAlignment(a);
    }
    VAlignment  GetVAlignment() const
    {
        return pTextNode->GetVAlignment();
    }

    void   SetBorderColor(const Color& borderColor)
    {
        pTextNode->SetBorderColor(borderColor);
    }
    Color GetBorderColor() const { return pTextNode->GetBorderColor(); }

    void   SetBackgroundColor(const Color& bkgColor)
    {
        pTextNode->SetBackgroundColor(bkgColor);
    }
    Color GetBackgroundColor() const { return pTextNode->GetBackgroundColor(); }

    //void Display();

//     void SetRendererString(const String& str)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         pUserData->StringVal = str;
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasString;
//         pUserData->UserData.pString = pUserData->StringVal.ToCStr();
//     }
//     String GetRendererString() const
//     {
//         if (!pUserData)
//             return String();
//         return pUserData->StringVal;
//     }
// 
//     void SetRendererFloat(float num)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         pUserData->FloatVal = num;
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasFloat;
//         pUserData->UserData.pFloat = &pUserData->FloatVal;
//     }
// 
//     float GetRendererFloat() const
//     {
//         if (!pUserData)
//             return 0;
//         return pUserData->FloatVal;
//     }

//     void SetRendererMatrix(float *m, unsigned count = 16)
//     {
//         if (!pUserData)
//             pUserData = SF_HEAP_NEW(pHeap) UserRendererData();
//         memcpy(pUserData->MatrixVal, m, count*sizeof(float));
//         pUserData->UserData.PropFlags |= Render::Renderer::UD_HasMatrix;
//         pUserData->UserData.pMatrix = pUserData->MatrixVal;
//         pUserData->UserData.MatrixSize = count;
//     }
    
    void        SetAAMode(AAMode aa)
    {
        pTextNode->SetAAMode(aa);
    }

    AAMode      GetAAMode() const 
    { 
        return pTextNode->GetAAMode();
    }

    Text::DocView* GetDocView() const { return pTextNode->GetDocView(); }
};

DrawTextImpl::DrawTextImpl(DrawTextManager* pdtMgr) : 
    pDrawTextCtxt(pdtMgr)
{
    pHeap = Memory::GetHeapByAddress(this);

    pTextNode = *pdtMgr->GetRenderContext().CreateEntry<TreeText>();
    pTextNode->Init(pDrawTextCtxt->GetTextAllocator(), pDrawTextCtxt->GetFontManager(), pDrawTextCtxt->GetLog());
}

// void DrawTextImpl::Display()
// {
//     if (pDrawTextCtxt)
//     {
//         if (!pDrawTextCtxt->IsBeginDisplayInvokedFlagSet())
//         {
//             Ptr<Log> plog = pDrawTextCtxt->pImpl->pStateBag->GetLog();
//             if (plog)
//                 plog->LogWarning("DrawText::Display is called w/o calling DrawTextManager::BeginDisplay().");
//             return;
//         }
//         
//         // Draw background and/or border.
//         if (BorderColor.GetAlpha() > 0 || BackgroundColor.GetAlpha() > 0)
//         {        
//             Render::Renderer* prenderer = pDrawTextCtxt->pImpl->DisplayContext.GetRenderer();
//             static const UInt16   indices[6] = { 0, 1, 2, 2, 1, 3 };
// 
//             prenderer->SetCxform(Cxform_1);
//             Matrix m(Matrix_1);
// 
//             RectF newRect;
//             Text::RecalculateRectToFit16Bit(m, pDocView->GetViewRect(), &newRect);
//             prenderer->SetMatrix(m);
// 
//             PointF         coords[4];
//             coords[0] = newRect.TopLeft();
//             coords[1] = newRect.TopRight();
//             coords[2] = newRect.BottomLeft();
//             coords[3] = newRect.BottomRight();
// 
//             Render::Renderer::VertexXY16i icoords[4];
//             icoords[0].x = (SInt16) coords[0].x;
//             icoords[0].y = (SInt16) coords[0].y;
//             icoords[1].x = (SInt16) coords[1].x;
//             icoords[1].y = (SInt16) coords[1].y;
//             icoords[2].x = (SInt16) coords[2].x;
//             icoords[2].y = (SInt16) coords[2].y;
//             icoords[3].x = (SInt16) coords[3].x;
//             icoords[3].y = (SInt16) coords[3].y;
// 
//             const SInt16  linecoords[10] = 
//             {
//                 // outline
//                 (SInt16) coords[0].x, (SInt16) coords[0].y,
//                 (SInt16) coords[1].x, (SInt16) coords[1].y,
//                 (SInt16) coords[3].x, (SInt16) coords[3].y,
//                 (SInt16) coords[2].x, (SInt16) coords[2].y,
//                 (SInt16) coords[0].x, (SInt16) coords[0].y,
//             };
// 
//             prenderer->FillStyleColor(BackgroundColor);
//             prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);
// 
//             // Fill the inside
//             prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
//             prenderer->DrawIndexedTriList(0, 0, 6, 0, 2);
//             prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
//             // And draw outline
//             prenderer->SetVertexData(linecoords, 5, Render::Renderer::Vertex_XY16i);
//             prenderer->LineStyleColor(BorderColor);
//             prenderer->DrawLineStrip(0, 4);
//             // Done
//             prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
//             prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
//         }
// 
//         if (!pUserData)
//             pDocView->Display(pDrawTextCtxt->pImpl->DisplayContext, Matrix_1, Cxform_1, false);
//         else
//         {
//             // Push the props
//             pDrawTextCtxt->pImpl->DisplayContext.GetRenderer()->PushUserData(&pUserData->UserData);
// 
//             pDocView->Display(pDrawTextCtxt->pImpl->DisplayContext, Matrix_1, Cxform_1, false);
// 
//             pDrawTextCtxt->pImpl->DisplayContext.GetRenderer()->PopUserData();
//         }
//     }
// }

//////////////////////////////////////////////////////////////////////////
// DrawTextManager
DrawTextManager::DrawTextManager(MovieDef* pmovieDef)
{
    pHeap = Memory::GetGlobalHeap()->CreateHeap("DrawText Manager");
    
    pImpl = SF_HEAP_NEW(pHeap) DrawTextManagerImpl();
    pImpl->pMovieDef = pmovieDef;
    if (pmovieDef)
    {
        pmovieDef->WaitForLoadFinish();
        MovieDefImpl* pdefImpl = static_cast<MovieDefImpl*>(pmovieDef);
        pImpl->pStateBag   = *new StateBagImpl(pdefImpl->pStateBag);
    }
    else
    {
        pImpl->pStateBag   = *new StateBagImpl(NULL);
        pImpl->pStateBag->SetLog(Ptr<Log>(*new Log));
    }
    if (pImpl->pStateBag)
    {
        // By default there should be no glyph packer
        //pImpl->pStateBag->SetFontPackParams(0);

        pImpl->pTextAllocator  = *SF_HEAP_NEW(pHeap) Text::Allocator(pHeap);
        pImpl->pFontStates     = *new FontManagerStates(pImpl->pStateBag);

        if (pImpl->pMovieDef)
        {
            //pStateBag->SetFontCacheManager(pMovieDef->GetFontCacheManager());
            //pStateBag->SetFontLib(pMovieDef->GetFontLib());
            //pStateBag->SetFontMap(pMovieDef->GetFontMap());

            MovieDefImpl* pdefImpl = static_cast<MovieDefImpl*>(pmovieDef);
            pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(pdefImpl, pImpl->pFontStates);
            pImpl->pWeakLib        = pdefImpl->GetWeakLib();
        }
        else
        {
            // It's mandatory to have the cache manager for text rendering to work,
            // even if the dynamic cache isn't used. 
            //pImpl->pStateBag->SetFontCacheManager(Ptr<FontCacheManager>(*new FontCacheManager(true)));
            pImpl->pWeakLib        = *new ResourceWeakLib(NULL);
            pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(pImpl->pWeakLib, pImpl->pFontStates);
        }
        //pImpl->pStateBag->SetMeshCacheManager(Ptr<MeshCacheManager>(*SF_NEW MeshCacheManager(false)));
    }
}

DrawTextManager::DrawTextManager(Loader* ploader)
{
    pHeap = Memory::GetGlobalHeap()->CreateHeap("DrawText Manager");

    pImpl = SF_HEAP_NEW(pHeap) DrawTextManagerImpl();
    pImpl->pMovieDef = 0;
    pImpl->pStateBag    = *new StateBagImpl(NULL);
    if (ploader->GetLog())
        pImpl->pStateBag->SetLog(ploader->GetLog());
    else
        pImpl->pStateBag->SetLog(Ptr<Log>(*new Log));

    // By default there should be no glyph packer
    //pImpl->pStateBag->SetFontPackParams(0);

    pImpl->pTextAllocator  = *SF_HEAP_NEW(pHeap) Text::Allocator(pHeap);
    pImpl->pFontStates     = *new FontManagerStates(pImpl->pStateBag);

    // It's mandatory to have the cache manager for text rendering to work,
    // even if the dynamic cache isn't used.
//     if (ploader->GetFontCacheManager())
//         pImpl->pStateBag->SetFontCacheManager(ploader->GetFontCacheManager());
//     else
//         pImpl->pStateBag->SetFontCacheManager(Ptr<FontCacheManager>(*new FontCacheManager(true)));

    //if (ploader->GetRenderConfig())
    //    pImpl->pStateBag->SetRenderConfig(ploader->GetRenderConfig());

    if (ploader->GetFontLib())
        pImpl->pStateBag->SetFontLib(ploader->GetFontLib());
    if (ploader->GetFontMap())
        pImpl->pStateBag->SetFontMap(ploader->GetFontMap());
    if (ploader->GetFontProvider())
        pImpl->pStateBag->SetFontProvider(ploader->GetFontProvider());

    Ptr<ResourceLib> pstrongLib = ploader->GetResourceLib();
    if (pstrongLib)
        pImpl->pWeakLib    = pstrongLib->GetWeakLib();
    else
        pImpl->pWeakLib    = *new ResourceWeakLib(NULL);
    pImpl->pFontManager    = *SF_HEAP_NEW(pHeap) FontManager(pImpl->pWeakLib, pImpl->pFontStates);
    //pImpl->pStateBag->SetMeshCacheManager(ploader->GetMeshCacheManager());
}

DrawTextManager::~DrawTextManager()
{
    delete pImpl;
    pHeap->Release();
}

void DrawTextManager::SetBeginDisplayInvokedFlag(bool v) 
{ 
    (v) ? pImpl->RTFlags |= DrawTextManagerImpl::RTFlags_BeginDisplayInvoked : 
          pImpl->RTFlags &= (~DrawTextManagerImpl::RTFlags_BeginDisplayInvoked); 
}
void DrawTextManager::ClearBeginDisplayInvokedFlag()            
{ 
    SetBeginDisplayInvokedFlag(false); 
}

bool DrawTextManager::IsBeginDisplayInvokedFlagSet() const      
{ 
    return (pImpl->RTFlags & DrawTextManagerImpl::RTFlags_BeginDisplayInvoked) != 0; 
}

DrawTextManager::TextParams::TextParams()
{
    TextColor = Color(0, 0, 0, 255);
    HAlignment = TreeText::Align_Left;
    VAlignment = TreeText::VAlign_Top;
    FontStyle  = TreeText::Normal;
    FontSize   = 12;
    FontName   = "Times New Roman";
    Underline  = false;
    Multiline  = true;
    WordWrap   = true;
}

void DrawTextManager::SetDefaultTextParams(const DrawTextManager::TextParams& params) 
{ 
    pImpl->DefaultTextParams = params; 
}

// Returns currently set default text parameters.
const DrawTextManager::TextParams& DrawTextManager::GetDefaultTextParams() const 
{ 
    return pImpl->DefaultTextParams; 
}

void DrawTextManager::SetTextParams(Text::DocView* pdoc, const TextParams& txtParams,
                                    const Text::TextFormat* tfmt, const Text::ParagraphFormat* pfmt)
{
    Text::TextFormat textFmt(pHeap);
    Text::ParagraphFormat paraFmt;
    if (tfmt)
        textFmt = *tfmt;
    if (pfmt)
        paraFmt = *pfmt;
    textFmt.SetColor(txtParams.TextColor);
    switch(txtParams.FontStyle)
    {
    case DrawText::Normal: 
        textFmt.SetBold(false);
        textFmt.SetItalic(false);
        break;
    case DrawText::Bold:
        textFmt.SetBold(true);
        textFmt.SetItalic(false);
        break;
    case DrawText::Italic:
        textFmt.SetBold(false);
        textFmt.SetItalic(true);
        break;
    case DrawText::BoldItalic:
        textFmt.SetBold(true);
        textFmt.SetItalic(true);
        break;
    }
    textFmt.SetFontName(txtParams.FontName);
    textFmt.SetFontSize(txtParams.FontSize);
    textFmt.SetUnderline(txtParams.Underline);

    Text::ParagraphFormat::AlignType pa;
    switch(txtParams.HAlignment)
    {
    case DrawText::Align_Right:   pa = Text::ParagraphFormat::Align_Right; break;
    case DrawText::Align_Center:  pa = Text::ParagraphFormat::Align_Center; break;
    case DrawText::Align_Justify: pa = Text::ParagraphFormat::Align_Justify; break;
    default:                      pa = Text::ParagraphFormat::Align_Left; break;
    }
    paraFmt.SetAlignment(pa);

    Text::DocView::ViewVAlignment va;
    switch(txtParams.VAlignment)
    {
    case DrawText::VAlign_Bottom:  va = Text::DocView::VAlign_Bottom; break;
    case DrawText::VAlign_Center:  va = Text::DocView::VAlign_Center; break;
    default:                          va = Text::DocView::VAlign_Top; break;
    }
    pdoc->SetVAlignment(va);

    if (txtParams.Multiline)
    {
        pdoc->SetMultiline();
        if (txtParams.WordWrap)
            pdoc->SetWordWrap();
    }
    pdoc->SetTextFormat(textFmt);
    pdoc->SetParagraphFormat(paraFmt);
    pdoc->SetDefaultTextFormat(textFmt);
    pdoc->SetDefaultParagraphFormat(paraFmt);
}

DrawText* DrawTextManager::CreateText()
{
    return SF_HEAP_NEW(pHeap) DrawTextImpl(this);
}

DrawText* DrawTextManager::CreateText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(putf8Str);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    return ptext;
}

DrawText* DrawTextManager::CreateText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(pwstr);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    return ptext;
}

DrawText* DrawTextManager::CreateText(const String& str, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    ptext->SetText(str);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    //ptext->GetDocView()->Dump();
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(putf8Str);
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(pwstr);
    return ptext;
}

DrawText* DrawTextManager::CreateHtmlText(const String& str, const RectF& viewRect, const TextParams* ptxtParams)
{
    DrawTextImpl* ptext = SF_HEAP_NEW(pHeap) DrawTextImpl(this);
    ptext->SetRect(viewRect);
    SetTextParams(ptext->GetDocView(), (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams);
    ptext->SetHtmlText(str);
    return ptext;
}

Text::DocView* DrawTextManager::CreateTempDoc(const TextParams& txtParams, 
                                                  Text::TextFormat* tfmt, Text::ParagraphFormat *pfmt,
                                                  float width, float height)
{
    Text::DocView* ptempText = SF_HEAP_NEW(pHeap) Text::DocView(pImpl->pTextAllocator, pImpl->pFontManager, GetLog());
    tfmt->InitByDefaultValues();
    pfmt->InitByDefaultValues();

    SizeF sz(width, height);
    ptempText->SetViewRect(RectF(0, 0, sz));
    if (txtParams.Multiline)
        ptempText->SetMultiline();
    else      
        ptempText->ClearMultiline();

    if (txtParams.WordWrap && width > 0)
    {
        ptempText->SetWordWrap();
        if (txtParams.Multiline)
            ptempText->SetAutoSizeY();
    }
    else
    {
        ptempText->SetAutoSizeX();
        ptempText->ClearWordWrap();
    }

    return ptempText;
}

SizeF DrawTextManager::GetTextExtent(const char* putf8Str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(putf8Str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetTextExtent(const wchar_t* pwstr, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(pwstr);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetTextExtent(const String& str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->SetText(str);

    //ptempText->Dump();
    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}

SizeF DrawTextManager::GetHtmlTextExtent(const char* putf8Str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(putf8Str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetHtmlTextExtent(const wchar_t* pwstr, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(pwstr);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}
SizeF DrawTextManager::GetHtmlTextExtent(const String& str, float width, const TextParams* ptxtParams)
{
    CheckFontStatesChange();
    Text::TextFormat tfmt(pHeap);
    Text::ParagraphFormat pfmt;
    TextParams txtParams = (ptxtParams) ? *ptxtParams : pImpl->DefaultTextParams;
    Ptr<Text::DocView> ptempText = *CreateTempDoc(txtParams, &tfmt, &pfmt, PixelsToTwips(width), 0);
    // need to reset Multiline and WordWrap, since they are already set up
    // correctly inside the CreateTempDoc.
    txtParams.Multiline = txtParams.WordWrap = false; 
    SetTextParams(ptempText, txtParams, &tfmt, &pfmt);
    ptempText->ParseHtml(str);

    SizeF sz(TwipsToPixels(ptempText->GetTextWidth()), TwipsToPixels(ptempText->GetTextHeight()));
    sz.Expand(TwipsToPixels(GFX_TEXT_GUTTER*2));
    return sz;
}

StateBag* DrawTextManager::GetStateBagImpl() const 
{ 
    return pImpl->pStateBag; 
}

Text::Allocator*       DrawTextManager::GetTextAllocator() 
{ 
    return pImpl->pTextAllocator; 
}

FontManager*         DrawTextManager::GetFontManager()   
{ 
    return pImpl->pFontManager; 
}

FontManagerStates*   DrawTextManager::GetFontManagerStates() 
{ 
    return pImpl->pFontStates; 
}

void DrawTextManager::CheckFontStatesChange()
{
    pImpl->pFontStates->CheckStateChange(GetFontLib(), GetFontMap(), GetFontProvider(), GetTranslator());
}

void DrawTextManager::BeginDisplay(const Viewport& vp)
{
    if (IsBeginDisplayInvokedFlagSet())
    {
        Ptr<Log> plog = pImpl->pStateBag->GetLog();
        if (plog)
            plog->LogWarning("Nested DrawTextManager::BeginDisplay() call detected");
        return;
    }
    DrawText::Matrix viewportMat;
    viewportMat.AppendScaling(1.f/20);

//     pImpl->DisplayContext.Init(this, NULL, NULL, 1, viewportMat);
//     if (!pImpl->DisplayContext.GetRenderConfig() ||
//         !pImpl->DisplayContext.GetRenderer())
//     {
//         Ptr<Log> plog = pImpl->pStateBag->GetLog();
//         if (plog)
//             plog->LogWarning("DrawTextManager: Renderer is not set! Nothing will be rendered!");
//         return;
//     }
    SetBeginDisplayInvokedFlag();
    //pImpl->DisplayContext.GetRenderer()->BeginDisplay(Color::White, vp, 0, vp.Width*20.f, 0, vp.Height*20.f);
}

void DrawTextManager::EndDisplay()
{
    if (!IsBeginDisplayInvokedFlagSet())
    {
        Ptr<Log> plog = pImpl->pStateBag->GetLog();
        if (plog)
            plog->LogWarning("DrawTextManager::EndDisplay() is called w/o BeginDisplay() call");
        return;
    }
    ClearBeginDisplayInvokedFlag();
    //pImpl->DisplayContext.GetRenderer()->EndDisplay();
}

Render::Context&    DrawTextManager::GetRenderContext()
{
    return pImpl->RenderContext;
}

}} // namespace Scaleform::GFx

#endif //GFX_ENABLE_DRAWTEXT
