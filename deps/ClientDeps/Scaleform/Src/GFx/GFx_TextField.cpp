/**********************************************************************

Filename    :   TextField.cpp
Content     :   Dynamic and input text field character implementation
Created     :   Dec, 2009
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/GFx_TextField.h"
#include "Text/Text_LineBuffer.h"
#include "Text/Text_StyleSheet.h"
#include "GFx/GFx_PlayerImpl.h"
#include "Render/Render_TreeLog.h"
#include "GFx/GFx_ASUtils.h"
#include "GFx/IME/GFx_IMEManager.h"
#include "Render/Render_TreeText.h"

namespace Scaleform {
namespace GFx {


TextField::TextField(TextFieldDef* def, 
                     MovieDefImpl *pbindingDefImpl, 
                     ASMovieRootBase* pasRoot,
                     InteractiveObject* parent, 
                     ResourceId id)
:
    InteractiveObject(pbindingDefImpl, pasRoot, parent, id),
    pDef(def),
    RestrictString(GetStringManager()->CreateEmptyString())
{
    SF_ASSERT(pDef);
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    pImageDescAssoc = NULL;
#endif
    if (pbindingDefImpl)
        pBinding = &pbindingDefImpl->GetResourceBinding();
    else
        pBinding = NULL;

    Flags = 0;
    FocusedControllerIdx = ~0u;

    //TextColor   = def->Color;
    Alignment   = def->Alignment;
    SetPassword(def->IsPassword());
    SetHtml(def->IsHtml());
    SetMouseWheelEnabled(true);
    pShadow     = 0;
    pRestrict   = 0;

    MovieImpl* proot = pasRoot->GetMovieImpl();

    Ptr<Text::Allocator> ptextAllocator = proot->GetTextAllocator();
    FontManager* fontMgr = (parent) ? parent->GetFontManager() : proot->FindFontManager();
    pDocument = 
        *SF_HEAP_NEW(proot->GetMovieHeap()) Text::DocView(ptextAllocator, fontMgr, GetLog());
    pDocument->SetDocumentListener(&TextDocListener);
    pDocument->GetStyledText()->SetNewLine0D(); // Flash uses '\x0D' ('\r', #13) as a new-line char

    if (def->IsBorder())
    {
        SetBackgroundColor(Color(255, 255, 255, 255));
        SetBorderColor(Color(0, 0, 0, 255));
    }

    SetInitialFormatsAsDefault();

    if (def->MaxLength > 0)
        pDocument->SetMaxLength(UPInt(def->MaxLength));

    pDocument->SetViewRect(def->TextRect);
    if (def->DoesUseDeviceFont())
    {
        pDocument->SetUseDeviceFont();
        pDocument->SetAAForReadability();
    }
    if (def->IsMultiline())
        pDocument->SetMultiline();
    else
        pDocument->ClearMultiline();
    const bool  autoSizeByX = (def->IsAutoSize() && !(def->IsWordWrap() && def->IsMultiline()));
    const bool  autoSizeByY = (def->IsAutoSize() /*&& Multiline*/);
    if (autoSizeByX)
        pDocument->SetAutoSizeX();
    if (autoSizeByY)
        pDocument->SetAutoSizeY();
    if (def->IsWordWrap())
        pDocument->SetWordWrap();
    if (IsPassword())
        pDocument->SetPasswordMode();
    if (def->IsAAForReadability())
        pDocument->SetAAForReadability();
    if (!IsReadOnly() || def->IsSelectable())
    {
        CreateEditorKit();
    }
    SetNeedUpdateLayoutFlag();
}

TextField::~TextField()
{
#ifdef GFX_AS2_ENABLE_BITMAPDATA
    ClearIdImageDescAssoc();
#endif
    pDocument->Close();
    delete pShadow;
    delete pRestrict;
}

bool TextField::IsSelectable() const         
{ 
    return (GetEditorKit()) ? 
        GetEditorKit()->IsSelectable() : GetTextFieldDef()->IsSelectable(); 
}

bool TextField::IsReadOnly() const 
{ 
    return (GetEditorKit()) ? 
        GetEditorKit()->IsReadOnly() : GetTextFieldDef()->IsReadOnly(); 
}

void TextField::SetFilters(const TextFilter& f)
{ 
     if (pDocument)
     {
         pDocument->SetFilters(f);
         SetDirtyFlag();
     }
}

Double TextField::GetTextWidth() const
{
    return TwipsToPixels(pDocument->GetTextWidth());
}

Double TextField::GetTextHeight() const
{
    return TwipsToPixels(pDocument->GetTextHeight());
}

TextField::CSSHolderBase* TextField::GetCSSData() const 
{ 
#ifdef GFX_ENABLE_CSS
    return pCSSData.GetPtr(); 
#else
    return NULL;
#endif
}

void TextField::SetCSSData(TextField::CSSHolderBase* css)
{
#ifdef GFX_ENABLE_CSS
    pCSSData = css; 
#else
    SF_UNUSED(css);
#endif
}

void TextField::SetDirtyFlag()
{
    MovieImpl* proot = GetMovieImpl();
    if (proot)
        proot->SetDirtyFlag();
    static_cast<TreeText*>(GetRenderNode())->NotifyLayoutChanged();
}

void TextField::SetSelectable(bool v) 
{ 
#ifdef GFX_ENABLE_TEXT_INPUT
    SF_ASSERT(pDocument);
    Ptr<Text::EditorKit> peditor = GetEditorKit();
    if (v)
    {
        peditor = CreateEditorKit();
        peditor->SetSelectable();
    }
    else
    {
        if (peditor)
            peditor->ClearSelectable();
    }
#else
    SF_UNUSED(v);
#endif //SF_NO_TEXT_INPUT_SUPPORT
}

Ptr<Text::EditorKit> TextField::CreateEditorKit()
{
#ifdef GFX_ENABLE_TEXT_INPUT
    SF_ASSERT(pDocument);
    Ptr<Text::EditorKit> peditor = GetEditorKit();
    if (!peditor)
    {
        peditor = *SF_HEAP_AUTO_NEW(this) Text::EditorKit(pDocument);

        if (GetTextFieldDef()->IsReadOnly())
            peditor->SetReadOnly();
        if (GetTextFieldDef()->IsSelectable())
            peditor->SetSelectable();

        Ptr<TextClipboard> pclipBoard = GetMovieImpl()->GetStateBagImpl()->GetTextClipboard();
        peditor->SetClipboard(pclipBoard);
        Ptr<TextKeyMap> pkeymap = GetMovieImpl()->GetStateBagImpl()->GetTextKeyMap();
        peditor->SetKeyMap(pkeymap);

        if (DoesUseRichClipboard())
            peditor->SetUseRichClipboard();
        else
            peditor->ClearUseRichClipboard();
    }
    return peditor;
#else
    return NULL;
#endif
}

void TextField::GetInitialFormats(Text::TextFormat* ptextFmt, Text::ParagraphFormat* pparaFmt)
{
    ptextFmt->InitByDefaultValues();
    pparaFmt->InitByDefaultValues();

    // get font params and use them as initial text format params.
    if (GetTextFieldDef()->FontId.GetIdIndex())
    {
        // resolve font name, if possible
        ResourceBindData fontData = pBinding->GetResourceData(GetTextFieldDef()->pFont);
        if (!fontData.pResource)
        {
            Ptr<Log> plog = GetLog();
            if (plog)
            {
                plog->LogError("Resource for font id = %d is not found in text field id = %d, def text = '%s'", 
                    GetTextFieldDef()->FontId.GetIdIndex(), GetId().GetIdIndex(), GetTextFieldDef()->DefaultText.ToCStr());
            }
        }
        else if (fontData.pResource->GetResourceType() != Resource::RT_Font)
        {
            Ptr<Log> plog = GetLog();
            if (plog)
            {
                plog->LogError("Font id = %d is referring to non-font resource in text field id = %d, def text = '%s'", 
                    GetTextFieldDef()->FontId.GetIdIndex(), GetId().GetIdIndex(), GetTextFieldDef()->DefaultText.ToCStr());
            }
        }
        else
        {
            FontResource* pfontRes = (FontResource*)fontData.pResource.GetPtr();
            if (pfontRes)
            {
                Font* pfont = pfontRes->GetFont();
                ptextFmt->SetFontName(pfont->GetName());

                // Don't set bold and italic as part of initial format, if HTML is used.
                // Also, don't use font handle, since HTML should to set its own font by name.
                if (!GetTextFieldDef()->IsHtml()/* && pfont->GetFont()->IsResolved()*/)
                {
                    ptextFmt->SetBold(pfont->IsBold());
                    ptextFmt->SetItalic(pfont->IsItalic());
                    if (!GetTextFieldDef()->DoesUseDeviceFont() && pfont->IsResolved() )
                    {
                        Ptr<FontHandle> pfontHandle = 
                            *SF_NEW FontHandle(NULL, pfont, 0, 0, pBinding->GetOwnerDefImpl());
                        ptextFmt->SetFontHandle(pfontHandle);
                    }
                }
            }
        }
    }
    else if (GetTextFieldDef()->FontClass.GetSize() > 0)
    {
        // use font class as a font name
        ptextFmt->SetFontName(GetTextFieldDef()->FontClass);
    }
    ptextFmt->SetFontSizeInFixp(unsigned(GetTextFieldDef()->TextHeight));
    ptextFmt->SetColor(GetTextFieldDef()->ColorV);

    Text::ParagraphFormat defaultParagraphFmt;
    switch (GetTextFieldDef()->Alignment)
    {
    case TextFieldDef::ALIGN_LEFT: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Left);
        break;
    case TextFieldDef::ALIGN_RIGHT: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Right);
        break;
    case TextFieldDef::ALIGN_CENTER: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Center);
        break;
    case TextFieldDef::ALIGN_JUSTIFY: 
        pparaFmt->SetAlignment(Text::ParagraphFormat::Align_Justify);
        break;
    }
    if (GetTextFieldDef()->HasLayout())
    {
        pparaFmt->SetLeftMargin((unsigned)TwipsToPixels(GetTextFieldDef()->LeftMargin));
        pparaFmt->SetRightMargin((unsigned)TwipsToPixels(GetTextFieldDef()->RightMargin));
        pparaFmt->SetIndent((int)TwipsToPixels(GetTextFieldDef()->Indent));
        pparaFmt->SetLeading((int)TwipsToPixels(GetTextFieldDef()->Leading));
    }
}

void TextField::SetInitialFormatsAsDefault()
{
    Text::TextFormat          defaultTextFmt(Memory::GetHeapByAddress(this));
    Text::ParagraphFormat defaultParagraphFmt;

    const Text::TextFormat*          ptextFmt = pDocument->GetDefaultTextFormat();
    const Text::ParagraphFormat* pparaFmt = pDocument->GetDefaultParagraphFormat();

    if (!GetTextFieldDef()->IsEmptyTextDef())
    {
        GetInitialFormats(&defaultTextFmt, &defaultParagraphFmt);
        if (ptextFmt)
            defaultTextFmt      = ptextFmt->Merge(defaultTextFmt);
        if (pparaFmt)
            defaultParagraphFmt = pparaFmt->Merge(defaultParagraphFmt);
    }
    else
    {
        if (ptextFmt)
            defaultTextFmt      = *ptextFmt;
        if (pparaFmt)
            defaultParagraphFmt = *pparaFmt;

        Text::TextFormat          eTextFmt(Memory::GetHeapByAddress(this));
        Text::ParagraphFormat eParagraphFmt;
        eTextFmt.InitByDefaultValues();
        eParagraphFmt.InitByDefaultValues();
        defaultTextFmt      = eTextFmt.Merge(defaultTextFmt);
        defaultParagraphFmt = eParagraphFmt.Merge(defaultParagraphFmt);
    }
    SetDefaultTextFormat(defaultTextFmt);
    SetDefaultParagraphFormat(defaultParagraphFmt);
}

int TextField::CheckAdvanceStatus(bool playingNow)
{
    int rv = 0;
    bool advanceDisabled = IsAdvanceDisabled() || !CanAdvanceChar();
    if (!advanceDisabled)
    {
        // Textfield is always in 'playing' status if it has a variable associated.
        bool playing = IsForceOneTimeAdvance() || IsForceAdvance();

        if (!playing)
        {
            // It is playing if there is a blinking cursor. It blinks only if 
            // the textfield is focused.
            // It is also playing if mouse is captured (for performing scrolling)
            MovieImpl* proot = GetMovieImpl();
            Text::EditorKit* peditorKit = GetEditorKit();
            if (IsVisibleFlagSet() && peditorKit && 
                ((peditorKit->HasCursor() && proot->IsFocused(this)) || peditorKit->IsMouseCaptured()))
                playing = true;
        }

        if (playing && !playingNow)
            rv = 1;
        else if (!playing && playingNow)
            rv = -1;
    }
    else if (playingNow)
        rv = -1;
    return rv;
}

void TextField::SetStateChangeFlags(UInt8 flags) 
{ 
    InteractiveObject::SetStateChangeFlags(flags);
    SetForceOneTimeAdvance();
    ModifyOptimizedPlayListLocal<TextField>();
}


// Override AdvanceFrame so that variable dependencies can be updated.
void    TextField::AdvanceFrame(bool nextFrame, float framePos)
{
    UInt8 stateFlags        = GetStateChangeFlags();
    const UInt8 stateMask   = DisplayObjectBase::StateChanged_FontLib | DisplayObjectBase::StateChanged_FontMap |
                              DisplayObjectBase::StateChanged_FontProvider | DisplayObjectBase::StateChanged_Translator;
    if ((stateFlags & stateMask) && pDocument)
    {
        pDocument->SetCompleteReformatReq();
        if (stateFlags & DisplayObjectBase::StateChanged_Translator)
            TextDocListener.TranslatorChanged();
        SetTextValue(OriginalTextValue.ToCStr(), IsOriginalHtml());
    }
    // use parent class' SetStateChangeFlags here to avoid extra calls to 
    // ModifyOptimizedPlayListLocal.
    InteractiveObject::SetStateChangeFlags(stateFlags & (~stateMask));

    if (IsForceOneTimeAdvance())
    {
        ClearForceOneTimeAdvance();
        ModifyOptimizedPlayListLocal<TextField>();
    }

    SF_UNUSED(framePos);
    if (nextFrame)
    {
        if (IsForceAdvance() && HasAvmObject())
            GetAvmTextField()->UpdateTextFromVariable();
        Flags |= Flags_NextFrame;
    }
    else
    {
        Flags &= ~Flags_NextFrame;
    }

    if (pDocument->HasEditorKit())
    {
        MovieImpl* proot = GetMovieImpl();
        Text::EditorKit* peditorKit = GetEditorKit();
        if (proot->IsFocused(this) || peditorKit->IsMouseCaptured())
        {
            Double timer = proot->GetTimeElapsed();
            peditorKit->Advance(timer);
            if (peditorKit->HasCursor())
                proot->SetDirtyFlag(); // because cursor may blink
        }
    }
}

bool TextField::HasStyleSheet() const 
{ 
    if (HasAvmObject())
        return GetAvmTextField()->HasStyleSheet();
    return false;
}

const Text::StyleManager* TextField::GetStyleSheet() const 
{ 
    if (HasAvmObject())
        return GetAvmTextField()->GetStyleSheet();
    return NULL;
}

bool TextField::IsTabable() const
{
    //return GetVisible() && !IsReadOnly() && !IsTabEnabledFlagFalse();
    if (GetVisible() && !IsReadOnly())
    {
        if (!IsTabEnabledFlagDefined())
        {
            if (HasAvmObject())
                return GetAvmTextField()->IsTabable();
            }
        return !IsTabEnabledFlagFalse();
    }
    return false;
}

// Set our text to the given string. pnewText can have HTML or
// regular formatting based on argument.
bool    TextField::SetTextValue(const char* pnewText, bool html, bool notifyVariable)
{
    // Check if we can avoid setting text (and reformatting / rebuilding layout) if:
    // 1) text that is being set is exactly the same as it was before;
    // 2) default text format was not changed (since default text format may cause
    // different formatting even if text is the same).
    // 3) replace/append functions were not called (they change the actual text without
    // modifying OriginalTextValue)
    if (OriginalTextValue == pnewText && !IsNeedUpdateLayoutFlag())
        return false;
    else
        ClearNeedUpdateLayoutFlag();

    // If CSS StyleSheet is set, then assignment to "text" works the same
    // way as to "htmlText".
    if (HasStyleSheet())
        html = true;

    OriginalTextValue = pnewText;
    SetOriginalHtml(html);
    // need to update the pnewText pointer, since assignment
    // to OriginalTextValue may cause its reallocation.
    pnewText = OriginalTextValue.ToCStr();

    bool doNotifyChanged = false;
    bool translated = false;
    if (!IsNoTranslate())
    {
        Ptr<Translator> ptrans = GetMovieImpl()->GetTranslator();
        if (ptrans)
        {
            class TranslateInfo : public Translator::TranslateInfo
            {
            public:
                WStringBuffer::Reserve<512> Res1, Res2;
                WStringBuffer ResultBuf;
                WStringBuffer KeyBuf;

                TranslateInfo(const char* instanceName):ResultBuf(Res1), KeyBuf(Res2)
                {
                    pResult       = &ResultBuf;
                    pInstanceName = instanceName;
                }

                void SetKey(const char* pkey)
                {
                    int nchars = (int)UTF8Util::GetLength(pkey);
                    KeyBuf.Resize(nchars + 1);
                    UTF8Util::DecodeString(KeyBuf.GetBuffer(), pkey);
                    pKey = KeyBuf.GetBuffer();
                }
                void    SetKey() { pKey = KeyBuf.GetBuffer(); }

                bool    IsResultHtml() const { return (Flags & Flag_ResultHtml) != 0; }
                bool    IsTranslated() const { return (Flags & Flag_Translated) != 0; }

                void    SetKeyHtml() { Flags |= Flag_SourceHtml; }
            } translateInfo((!HasInstanceBasedName()) ? GetName().ToCStr() : "");

            if (!html || ptrans->CanReceiveHtml())
            {
                // convert html or plain text to unicode w/o parsing
                translateInfo.SetKey(pnewText);
                if (html)
                    translateInfo.SetKeyHtml();
                ptrans->Translate(&translateInfo);
            }
            else
            {
                // if html, but need to pass plain text
                Text::TextFormat txtFmt(Memory::GetHeapByAddress(this));
                Text::ParagraphFormat paraFmt;
                GetInitialFormats(&txtFmt, &paraFmt);                
                pDocument->ParseHtml(pnewText, SF_MAX_UPINT, IsCondenseWhite(), NULL, GetStyleSheet(), &txtFmt, &paraFmt);
                pDocument->GetStyledText()->GetText(&translateInfo.KeyBuf);
                if (ptrans->NeedStripNewLines())
                    translateInfo.KeyBuf.StripTrailingNewLines();
                translateInfo.SetKey();
                ptrans->Translate(&translateInfo);
            }
            translated = translateInfo.IsTranslated();
            if (translated)
            {
                if (translateInfo.IsResultHtml())
                {
                    Text::TextFormat txtFmt(Memory::GetHeapByAddress(this));
                    Text::ParagraphFormat paraFmt;
                    GetInitialFormats(&txtFmt, &paraFmt);                
                    Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(this));
                    pDocument->ParseHtml(translateInfo.ResultBuf.ToWStr(), SF_MAX_UPINT, IsCondenseWhite(), &imageInfoArray, GetStyleSheet(), &txtFmt, &paraFmt);
                    if (imageInfoArray.GetSize() > 0)
                    {
                        ProcessImageTags(imageInfoArray);
                    }
                }
                else
                {
                    // Attempt to keep original formatting: get the format of the old text's first letter
                    // and apply to whole new text. This should prevent from losing formatting (bold, italic, etc)
                    // after translation.
                    const Text::TextFormat*         ptextFmt;
                    const Text::ParagraphFormat*    pparaFmt;
                    pDocument->GetTextAndParagraphFormat(&ptextFmt, &pparaFmt, 0);
                    pDocument->SetDefaultTextFormat(ptextFmt);
                    pDocument->SetDefaultParagraphFormat(pparaFmt);

                    pDocument->SetText(translateInfo.ResultBuf.ToWStr());
                }
                doNotifyChanged = true;
            }
        }
    }

    if (!translated)
    {
        if (html)
        {
            Text::TextFormat txtFmt(Memory::GetHeapByAddress(this));
            Text::ParagraphFormat paraFmt;
            GetInitialFormats(&txtFmt, &paraFmt);                
            Text::StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(this));
            pDocument->ParseHtml(pnewText, SF_MAX_UPINT, IsCondenseWhite(), &imageInfoArray, GetStyleSheet(), &txtFmt, &paraFmt);
            if (imageInfoArray.GetSize() > 0)
            {
                ProcessImageTags(imageInfoArray);
            }
        }
        else
        {
            pDocument->SetText(pnewText);        
        }
    }

    if (pDocument->HasEditorKit())
    {
        if (!IsReadOnly()/* || IsSelectable()*/)
        {
            //!AB, this causes single line text scrolled to the end when text is assigned and
            // it is longer than the visible text field.
            // This is not happening in Flash, so at the moment just comment
            // this out.
            //if (!pDocument->IsMultiline())
            //    GetEditorKit()->SetCursorPos(pDocument->GetLength(), false);
            //else
            //    GetEditorKit()->SetCursorPos(0, false);
            UPInt docLen = pDocument->GetLength();
            if (GetEditorKit()->GetCursorPos() > docLen)
                GetEditorKit()->SetCursorPos(docLen, false);
        }
    }

    if (HasStyleSheet() && pDocument->MayHaveUrl())
    {
        // collect info about active zones for each URL
        CollectUrlZones();
    }

    // update variable, if necessary
    if (notifyVariable && HasAvmObject())
        GetAvmTextField()->UpdateVariable();

    //!AB MaxLength is available only for InputText; though, if
    // you set text by variable the maxlength is ignored.
    if (doNotifyChanged)
        NotifyChanged();

    SetNeedUpdateGeomData(); // it will call SetDirtyFlag.
    return true;
}

// Special method for complex objects API (see Value::ObjectInterface::SetDisplayInfo)
Render::PointF    TextField::TransformToTextRectSpace(const Value::DisplayInfo& info) const
{
    const Matrix& m = GetMatrix();
    Render::PointF     p((float)info.GetX(), (float)info.GetY());
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.x         = -r.x;
    p.y         = -r.y;
    p           = m.Transform(p);
    p.x         = TwipsToPixels(p.x);
    p.y         = TwipsToPixels(p.y);
    return p;
}


// Special code for complex objects API (see Value::ObjectInterface::SetPositionInfo)
void    TextField::GetPosition(Value::DisplayInfo* pinfo)
{
    GeomDataType geomData;
    UpdateAndGetGeomData(&geomData);
    Double x = TwipsToPixels(Double(geomData.X));
    Double y = TwipsToPixels(Double(geomData.Y));
    Double rotation = geomData.Rotation;
    Double xscale = geomData.XScale;
    Double yscale = geomData.YScale;
    Double alpha = GetCxform().M[3][0] * 100.F;
    bool visible = GetVisible();

    pinfo->Set(x, y, rotation, xscale, yscale, alpha, visible);
}

ASString    TextField::GetText(bool reqHtml) const 
{
    ASStringManager* pmgr = GetStringManager();
    if (reqHtml)
    {
        if (HasStyleSheet())
        {
            // return original html if CSS is set
            return pmgr->CreateString(OriginalTextValue);
        }
        else
        {
            // For non-HTML field, non-HTML text is returned.
            if (IsHtml())
                return pmgr->CreateString(pDocument->GetHtml());
            else
                return pmgr->CreateString(pDocument->GetText());
        }
    }
    else
    {                   
        return pmgr->CreateString(pDocument->GetText());
    }
}

// Return the topmost entity that the given point covers.  NULL if none.
// I.E. check against ourself.
DisplayObjectBase::TopMostResult TextField::GetTopMostMouseEntity(
    const Render::PointF &pt, TopMostDescr* pdescr)
{
    pdescr->pResult = NULL;
    if (IsHitTestDisableFlagSet() || !GetVisible())
        return TopMost_FoundNothing;

    if (pdescr->pIgnoreMC == this)
        return TopMost_FoundNothing;

    Render::PointF p;
    TransformPointToLocal(p, pt);

    if ((ClipDepth == 0) && PointTestLocal(p, HitTest_TestShape))
    {
        if (pdescr->TestAll || IsSelectable())
        {
            pdescr->pResult = this;
            return TopMost_Found;
        }
        else if (!IsSelectable() && IsHtml() && 
            pDocument->MayHaveUrl() && pDocument->IsUrlAtPoint(p.x, p.y))
        {
            // if not selectable, will need to check for url under the
            // mouse cursor. Return "this", if there is an url under 
            // the mouse cursor.
            pdescr->pResult = this;
            return TopMost_Found;
        }
        else
        {
            InteractiveObject* pparent = GetParent();
            while (pparent && pparent->IsSprite())
            {
                // Parent sprite would have to be in button mode to grab attention.
                Sprite * psprite = pparent->CharToSprite_Unsafe();
                if (pdescr->TestAll || psprite->ActsAsButton())
                {
                    // Check if sprite should be ignored
                    if (!pdescr->pIgnoreMC || (psprite != pdescr->pIgnoreMC))
                    {
                        pdescr->pResult = psprite;
                        return TopMost_Found;
                    }
                }
                pparent = pparent->GetParent ();
            }
        }
    }

    pdescr->LocalPt = p;
    pdescr->pResult = NULL;
    return TopMost_Continue;
}

unsigned    TextField::GetCursorType() const 
{
    if (IsHandCursor())
        return MouseCursorEvent::HAND;
    if (IsSelectable())
        return MouseCursorEvent::IBEAM;
    return InteractiveObject::GetCursorType();
}

bool TextField::IsUrlUnderMouseCursor(unsigned mouseIndex, Render::PointF* pPnt, Range* purlRangePos)
{
    // Local coord of mouse IN PIXELS.
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return false;
    SF_ASSERT(proot->GetMouseState(mouseIndex));
    Render::PointF a = proot->GetMouseState(mouseIndex)->GetLastPosition();

    Matrix  m = GetWorldMatrix();
    Render::PointF b;

    m.TransformByInverse(&b, a);
    if (pPnt) *pPnt = b;

    return pDocument->IsUrlAtPoint(b.x, b.y, purlRangePos);
}

bool TextField::IsUrlTheSame(unsigned mouseIndex, const Range& urlRange)
{
    bool rv = true;
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
        {
            if (pCSSData->UrlZones[i].Intersects(urlRange))
            {
                if (pCSSData->MouseState[mouseIndex].UrlZoneIndex != i + 1)
                {
                    rv = false;
                    break;
                }
            }
        }
    }
#else
    SF_UNUSED2(mouseIndex, urlRange);
#endif
    return rv;
}

void TextField::CollectUrlZones()
{
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        memset(pCSSData->MouseState, 0, sizeof(pCSSData->MouseState));
        pCSSData->UrlZones.RemoveAll();
        Text::StyledText* pstyledText = pDocument->GetStyledText();
        String currentUrl;
        UPInt startPos = 0, length = 0;
        unsigned n = pstyledText->GetParagraphsCount();
        for (unsigned i = 0; i < n; ++i)
        {
            const Text::Paragraph* ppara = pstyledText->GetParagraph(i);
            Text::Paragraph::FormatRunIterator it = ppara->GetIterator();
            for (; !it.IsFinished(); ++it)
            {
                UPInt indexInDoc = (UPInt)it->Index + ppara->GetStartIndex();
                if (it->pFormat->IsUrlSet())
                {
                    if (!currentUrl.IsEmpty())
                    {
                        if (indexInDoc != startPos + length || currentUrl != it->pFormat->GetUrl())
                        {
                            CSSHolderBase::UrlZone urlZone;
                            urlZone.SavedFmt = 
                                *pDocument->GetStyledText()->CopyStyledText(startPos, startPos + length);
                            pCSSData->UrlZones.InsertRange(startPos, length, urlZone);
                            currentUrl.Clear();
                        }
                        else
                        {
                            length += it->Length;
                        }
                    }
                    if (currentUrl.IsEmpty())
                    {
                        startPos = indexInDoc;
                        length   = it->Length;
                        currentUrl = it->pFormat->GetUrl();
                    }
                }
            }
        }
        if (!currentUrl.IsEmpty())
        {
            CSSHolderBase::UrlZone urlZone;
            urlZone.SavedFmt = 
                *pDocument->GetStyledText()->CopyStyledText(startPos, startPos + length);
            pCSSData->UrlZones.InsertRange(startPos, length, urlZone);
        }
    }
#endif // GFX_ENABLE_CSS
}

void    TextField::PropagateMouseEvent(const EventId& id)
{
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return;

    if (id.Id == EventId::Event_MouseMove)
    {
        // Implement mouse-drag.
        InteractiveObject::DoMouseDrag();
    }

    Ptr<InteractiveObject> ptopMostChar = proot->GetMouseState(id.MouseIndex)->GetTopmostEntity();

    // if ptopMostChar != thus - means that the cursor is out of the URL => restore URL fmt
    if ((ptopMostChar != this || id.Id == EventId::Event_RollOut) && 
        HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
    {
        ChangeUrlFormat(Link_release, id.MouseIndex);
        ChangeUrlFormat(Link_rollout, id.MouseIndex);
    }

    if (ptopMostChar == this ||
        (pDocument->HasEditorKit() && GetEditorKit()->IsMouseCaptured()))
    {
        if (!GetVisible())  
            return;
        switch(id.Id)
        {
        case EventId::Event_MouseDown:
            {
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Range urlRange;
                    if (IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange) && 
                        pCSSData && pCSSData->HasASStyleSheet() && (pms->GetButtonsState() & MouseState::MouseButton_Left))
                    {
                        // apply CSS style - a:active
                        ChangeUrlFormat(Link_press, id.MouseIndex, &urlRange);
                    }
                }
#endif // GFX_ENABLE_CSS
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseDown(b.x, b.y, pms->GetButtonsState());

                    ModifyOptimizedPlayListLocal<TextField>();
                }
            }
            break;
        case EventId::Event_MouseUp:
        case EventId::Event_ReleaseOutside:
            {
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Range urlRange;
                    if (IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange) && 
                        pCSSData && pCSSData->HasASStyleSheet() && !(pms->GetButtonsState() & MouseState::MouseButton_Left))
                    {
                        // apply CSS style - a:active
                        ChangeUrlFormat(Link_release, id.MouseIndex, &urlRange);
                    }
                    ChangeUrlFormat(Link_release, id.MouseIndex);
                }
#endif // GFX_ENABLE_CSS
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.
                    const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                    SF_ASSERT(pms);

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseUp(b.x, b.y, pms->GetButtonsState());

                    ModifyOptimizedPlayListLocal<TextField>();
                }
            }
            break;
        case EventId::Event_MouseMove:
            {
                const MouseState* pms = proot->GetMouseState(id.MouseIndex);
                SF_ASSERT(pms);
                if (pDocument->HasEditorKit())
                {
                    // Local coord of mouse IN PIXELS.

                    Matrix  m = GetWorldMatrix();
                    Render::PointF b;

                    m.TransformByInverse(&b, pms->GetLastPosition());
                    GetEditorKit()->OnMouseMove(b.x, b.y);
                }
#ifdef GFX_ENABLE_CSS
                if (HasStyleSheet() && IsHtml() && pDocument->MayHaveUrl())
                {
                    Range urlRange;
                    bool urlIsUnderCursor = IsUrlUnderMouseCursor(id.MouseIndex, NULL, &urlRange);

                    // apply CSS style - either a:active or a:hover
                    if (urlIsUnderCursor)
                    {
                        if (!IsUrlTheSame(id.MouseIndex, urlRange))
                        {
                            ChangeUrlFormat(Link_release, id.MouseIndex);
                            ChangeUrlFormat(Link_rollout, id.MouseIndex);
                        }
                        ChangeUrlFormat(
                            (pms->GetButtonsState() & MouseState::MouseButton_Left) ? Link_press : Link_rollover, 
                            id.MouseIndex, &urlRange);
                    }
                    else
                    {
                        ChangeUrlFormat(Link_release, id.MouseIndex);
                        ChangeUrlFormat(Link_rollout, id.MouseIndex);
                    }
                    SetHandCursor(urlIsUnderCursor);
                    proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                }
                else 
#endif // GFX_ENABLE_CSS
                {
                    if (IsHtml() && pDocument->MayHaveUrl())
                    {
                        SetHandCursor(IsUrlUnderMouseCursor(id.MouseIndex));
                        proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                    }
                    else if (IsHandCursor())
                    {
                        ChangeUrlFormat(Link_rollout, id.MouseIndex);
                        ClearHandCursor();
                        proot->ChangeMouseCursorType(id.MouseIndex, GetCursorType());
                    }
                }
            }
            break;
        default: break;
        }
    }
    InteractiveObject::PropagateMouseEvent(id);
}

// handle Event_MouseDown to grab focus
bool TextField::OnMouseEvent(const EventId& event)
{ 
    PropagateMouseEvent(event);
    if (HasAvmObject())
        return GetAvmTextField()->OnMouseEvent(event);
    return false; 
}

bool TextField::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx) const
{
    return (FocusedControllerIdx == ~0u || FocusedControllerIdx == controllerIdx) && 
        InteractiveObject::IsFocusAllowed(proot, controllerIdx);
}

bool TextField::IsFocusAllowed(MovieImpl* proot, unsigned controllerIdx)
{
    return (FocusedControllerIdx == ~0u || FocusedControllerIdx == controllerIdx) && 
        InteractiveObject::IsFocusAllowed(proot, controllerIdx);
}


void TextField::OnFocus(FocusEventType event, InteractiveObject* oldOrNewFocusCh, 
                        unsigned controllerIdx, FocusMovedType fmt)
{
    if (IsSelectable())
    {
        // manage the selection
        if (event == InteractiveObject::SetFocus)
        {
            if (!IsNoAutoSelection() && fmt != GFx_FocusMovedByMouse)
            {
                UPInt len = pDocument->GetLength();
                SetSelection(0, (SPInt)len);
            }
            FocusedControllerIdx = controllerIdx;
            if (pDocument->HasEditorKit())
            {
                GetEditorKit()->OnSetFocus();
                SetDirtyFlag();
            }
        }
        else if (event == InteractiveObject::KillFocus)
        {
            FocusedControllerIdx = ~0u;
            if (pDocument->HasEditorKit())
            {
                if (!DoesAlwaysShowSelection())
                    GetEditorKit()->ClearSelection();

                GetEditorKit()->OnKillFocus();
                SetDirtyFlag();
            }
        }
    }
    if (!IsReadOnly() || IsSelectable())
    {
        if (event == InteractiveObject::SetFocus)
        {
            ResetBlink(1, 1);
        }
        else
        {
            ResetBlink(0, 0);        
        }

        InteractiveObject::OnFocus(event, oldOrNewFocusCh, controllerIdx, fmt);

        if (pDocument->HasEditorKit() && GetEditorKit()->HasCursor())
        {
            SetForceOneTimeAdvance();
            ModifyOptimizedPlayListLocal<TextField>();
        }
    }
}

bool TextField::OnLosingKeyboardFocus(InteractiveObject*, unsigned , FocusMovedType fmt) 
{
    if (fmt == GFx_FocusMovedByMouse)
    {
        //!AB: Not sure, is this case still actual or not.
        // Though, this "if" may lead to incorrect behavior if the text field
        // hides itself in onPress handler: mouseCaptured flag won't be cleared
        // since no OnMouseUp will be received.
        //if (pDocument->HasEditorKit() && GetEditorKit()->IsMouseCaptured())
        //    return false; // prevent focus loss, if mouse is captured
    }
    return true;
}

RectF  TextField::GetBounds(const Matrix &t) const
{
    return t.EncloseTransform(pDocument->GetViewRect());
}

bool    TextField::PointTestLocal(const Render::PointF &pt, UInt8 hitTestMask) const
{
    if (IsHitTestDisableFlagSet())
        return false;
    if ((hitTestMask & HitTest_IgnoreInvisible) && !GetVisible())
        return false;
    // Flash only uses the bounding box for text - even if shape flag is used
    return pDocument->GetViewRect().Contains(pt);
}

// Draw the dynamic string.
// @REN
/*
void    TextField::Display(DisplayContext &context)   
{        
    Render::Renderer* prenderer = context.GetRenderer();
    if (!prenderer)
        return;
    MovieImpl* proot = GetMovieImpl();
    if (!proot)
        return;

    DisplayContextStates save;
    if (!BeginDisplay(context, save))
        return;

    const Matrix2F& mat = *context.pParentMatrix;         
    const Cxform&   cx  = *context.pParentCxform;

    // Do viewport culling if bounds are available.
    if (!proot->GetVisibleFrameRectInTwips().Intersects(mat.EncloseTransform(pDocument->GetViewRect())))
        if (!(context.GetRenderFlags() & RenderConfig::RF_NoViewCull))
            return;
    // If Alpha is Zero, don't draw text.        
    if ((Alg::Abs(cx.M[3][0]) < 0.001f) &&
        (Alg::Abs(cx.M[3][1]) < 1.0f) )
        return;

    // Show white background + black bounding box.
    if (BorderColor.GetAlpha() > 0 || BackgroundColor.GetAlpha() > 0)
    {        
        static const UInt16   indices[6] = { 0, 1, 2, 2, 1, 3 };

        // Show white background + black bounding box.
        prenderer->SetCxform(cx);
        Matrix m(mat);

        RectF newRect;
        Text::RecalculateRectToFit16Bit(m, pDocument->GetViewRect(), &newRect);
        prenderer->SetMatrix(m);

        Render::PointF         coords[4];
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

        const SInt16  linecoords[10] = 
        {
            // outline
            (SInt16) coords[0].x, (SInt16) coords[0].y,
            (SInt16) coords[1].x, (SInt16) coords[1].y,
            (SInt16) coords[3].x, (SInt16) coords[3].y,
            (SInt16) coords[2].x, (SInt16) coords[2].y,
            (SInt16) coords[0].x, (SInt16) coords[0].y,
        };

        prenderer->FillStyleColor(BackgroundColor);
        prenderer->SetVertexData(icoords, 4, Render::Renderer::Vertex_XY16i);

        // Fill the inside
        prenderer->SetIndexData(indices, 6, Render::Renderer::Index_16);
        prenderer->DrawIndexedTriList(0, 0, 4, 0, 2);
        prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
        // And draw outline
        prenderer->SetVertexData(linecoords, 5, Render::Renderer::Vertex_XY16i);
        prenderer->LineStyleColor(BorderColor);
        prenderer->DrawLineStrip(0, 4);
        // Done
        prenderer->SetVertexData(0, 0, Render::Renderer::Vertex_None);
        prenderer->SetIndexData(0, 0, Render::Renderer::Index_None);
    }

    bool nextFrame = (Flags & Flags_NextFrame) != 0;
    Flags &= ~Flags_NextFrame;

    bool isFocused = proot->IsFocused(this);
    pDocument->PreDisplay(context, mat, cx, isFocused);
    if (pShadow)
    {
        for (unsigned i = 0; i < pShadow->ShadowOffsets.GetSize(); i++)
        {
            Render::Matrix2F   shm(mat);
            Cxform c(cx);
            c.M[0][0] = c.M[1][0] = c.M[2][0] = 0;
            c.M[0][1] = (float)(pShadow->ShadowColor.GetRed());   // R
            c.M[1][1] = (float)(pShadow->ShadowColor.GetGreen()); // G
            c.M[2][1] = (float)(pShadow->ShadowColor.GetBlue());  // B

            shm.PrependTranslation(pShadow->ShadowOffsets[i].x, pShadow->ShadowOffsets[i].y);
            pDocument->Display(context, shm, c, nextFrame);
        }

        // Draw our actual text.
        if (pShadow->TextOffsets.GetSize() == 0)
            pDocument->Display(context, mat, cx, nextFrame);
        else
        {
            for (unsigned j = 0; j < pShadow->TextOffsets.GetSize(); j++)
            {
                Render::Matrix2F   shm(mat);

                shm.PrependTranslation(pShadow->TextOffsets[j].x, pShadow->TextOffsets[j].y);
                pDocument->Display(context, shm, cx, nextFrame);
            }
        }
    }
    else
    {
        pDocument->Display(context, mat, cx, nextFrame);
    }
    if (pDocument->HasEditorKit() && proot->IsFocused(this))
        GetEditorKit()->Display(context, mat, cx);

    pDocument->PostDisplay(context, mat, cx, isFocused);

    EndDisplay(context, save);
}
*/

void TextField::ResetBlink(bool state, bool blocked)
{
    if (pDocument->HasEditorKit())  
    {
        if (GetEditorKit()->HasCursor())
        {
            GetEditorKit()->ResetBlink(state, blocked);
            SetDirtyFlag();
        }
    }
}

void TextField::SetSelection(SPInt beginIndex, SPInt endIndex)
{
#ifdef GFX_ENABLE_TEXT_INPUT
    // Flash allows do selection in read-only non-selectable textfields.
    // To reproduce the same behavior we need to create EditorKit, even, 
    // though read-only non-selectable textfield doesn't need it.
    if (!pDocument->HasEditorKit())
        CreateEditorKit();
    if (pDocument->HasEditorKit())
    {
        beginIndex = Alg::Max((SPInt)0, beginIndex);
        endIndex   = Alg::Max((SPInt)0, endIndex);
        UPInt len = pDocument->GetLength();
        beginIndex = Alg::Min((SPInt)len, beginIndex);
        endIndex   = Alg::Min((SPInt)len, endIndex);
        GetEditorKit()->SetSelection((UPInt)beginIndex, (UPInt)endIndex);
        SetDirtyFlag();
    }
#else
    SF_UNUSED2(beginIndex, endIndex);
#endif //SF_NO_TEXT_INPUT_SUPPORT
}

SPInt TextField::GetCaretIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    if (!IsReadOnly() || IsSelectable())
    {
        if (pDocument->HasEditorKit())
        {
            return (SPInt)GetEditorKit()->GetCursorPos();
        }
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return -1;
}

UPInt TextField::GetBeginIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    if ((!IsReadOnly() || IsSelectable()) && pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetBeginSelection();
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return 0;
}

UPInt TextField::GetEndIndex() const
{
#ifdef GFX_ENABLE_TEXT_INPUT
    if ((!IsReadOnly() || IsSelectable()) && pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetEndSelection();
    }
#endif //SF_NO_TEXT_INPUT_SUPPORT
    return 0;
}

void TextField::OnEventLoad()
{
    // finalize the initialization. We need to initialize text here rather than in ctor
    // since the name of instance is not set yet in the ctor and setting text might cause
    // GFxTranslator to be invoked, which uses the instance name.
    if (pDef->DefaultText.GetLength() > 0)
    {
        SetTextValue(pDef->DefaultText.ToCStr(), IsHtml(), false);
    }
    else
    {
        // This assigns both Text and HtmlText vars.
        SetTextValue("", IsHtml(), false);
    }

//     if (HasAvmObject())
//         GetAvmTextField()->OnEventLoad();
    pDocument->Format();

    InteractiveObject::OnEventLoad();

    // invoked when textfield is being added into displaylist;
    // we need to modify GeomData to reflect correct X and Y coords.
    // It is incorrect to take X and Y coordinates directly from the matrix
    // as it usually happens for movieclip. The matrix of textfields 
    // contains some offset (usually, 2 pixels by both axis), because TextRect's
    // topleft corner is usually (-2, -2), not (0,0).
    // Thus, we need to recalculate X and Y received from the matrix with using 
    // TextRect topleft corner.
    if (!pGeomData)
    {
        GeomDataType geomData;
        UpdateAndGetGeomData(&geomData, true);
    }
}

void TextField::OnEventUnload()
{
    SetUnloading();
#ifndef SF_NO_IME_SUPPORT
    // text field is unloading, so we need to make sure IME was finalized
    // for this text field.
    MovieImpl* proot = GetMovieImpl();
    if (proot)
    {
        Ptr<IMEManagerBase> pIMEManager = proot->GetIMEManager();
        if (pIMEManager && pIMEManager->IsTextFieldFocused(this))
        {
            pIMEManager->DoFinalize();
        }
    }
#endif //#ifdef SF_NO_IME_SUPPORT
    SetDirtyFlag();
    InteractiveObject::OnEventUnload();
}

DisplayObjectBase::GeomDataType& TextField::UpdateAndGetGeomData(GeomDataType* pgeomData, bool force)
{
    SF_ASSERT(pgeomData);
    GetGeomData(*pgeomData);
    if (force || (Flags & Flags_NeedUpdateGeomData) != 0)
    {
        RectF rr   = pDocument->GetViewRect();
        Matrix m    = GetMatrix();
        Render::PointF p   = rr.TopLeft();

        // same as m.Transform(p), but with two exception:
        // all calculations are being done in doubles.
        Double x = Double(m.Sx()) * p.x + Double(m.Shx()) * p.y + Double(m.Tx());
        Double y = Double(m.Shy()) * p.x + Double(m.Sy()) * p.y + Double(m.Ty());

        pgeomData->X = Alg::IRound(x);
        pgeomData->Y = Alg::IRound(y);
        SetGeomData(*pgeomData);
        Flags &= ~Flags_NeedUpdateGeomData;
    }
    return *pgeomData;
}

#ifdef GFX_ENABLE_KEYBOARD
bool TextField::OnKeyEvent(const EventId& id, int* pkeyMask)
{
    SF_UNUSED(pkeyMask);

    MovieImpl* proot = GetMovieImpl();
    unsigned focusGroup     = proot->GetFocusGroupIndex(id.KeyboardIndex);
    if (!((*pkeyMask) & ((1 << focusGroup) & InteractiveObject::KeyMask_FocusedItemHandledMask)))
    {
        if (proot && proot->IsFocused(this, id.KeyboardIndex))
        {
            if (pDocument->HasEditorKit() && (!IsReadOnly() || IsSelectable()))
            {
                if (HasAvmObject())
                    GetAvmTextField()->OnKeyEvent(id, pkeyMask);
                if (id.Id == EventId::Event_KeyDown)
                {
                    GetEditorKit()->OnKeyDown
                        ((int)id.KeyCode, id.KeysState.States);

                    // toggle overwrite/insert modes
                    if (id.KeyCode == Key::Insert)
                    {
                        SetOverwriteMode(!IsOverwriteMode());
                    }
                }
                else if (id.Id == EventId::Event_KeyUp)
                {
                    GetEditorKit()->OnKeyUp
                        ((int)id.KeyCode, id.KeysState.States);
                }
            }
            (*pkeyMask) |= ((1 << focusGroup) & InteractiveObject::KeyMask_FocusedItemHandledMask);
            return true;
        }
    }
    return false;
}

bool TextField::OnCharEvent(UInt32 wcharCode, unsigned controllerIdx)
{
    // cursor positioning is available for selectable readonly textfields
    MovieImpl* proot = GetMovieImpl();
    if (proot && pDocument->HasEditorKit() && (!IsReadOnly() || IsSelectable()) && 
        FocusedControllerIdx == controllerIdx)
    {
        if (pRestrict)
        {
            if (pRestrict->RestrictRanges.GetIteratorAt(int(wcharCode)).IsFinished())
            {
                int up  = SFtowupper((wchar_t)wcharCode); 
                int low = SFtowlower((wchar_t)wcharCode); 
                if ((int)wcharCode != up)
                    wcharCode = (UInt32)up;
                else 
                    wcharCode = (UInt32)low;
                if (pRestrict->RestrictRanges.GetIteratorAt(int(wcharCode)).IsFinished())
                    return false;
            }
        }
        SF_ASSERT(HasAvmObject());
        if (GetAvmTextField()->OnCharEvent((wchar_t)wcharCode, controllerIdx))
            GetEditorKit()->OnChar(wcharCode);
    }
    return true;
}

void  TextField::SetOverwriteMode(bool overwMode)
{
    if (pDocument->HasEditorKit())
    {
        if (overwMode)
            GetEditorKit()->SetOverwriteMode();
        else
            GetEditorKit()->ClearOverwriteMode();
    }
}
bool TextField::IsOverwriteMode() const
{
    if (pDocument->HasEditorKit())
        return GetEditorKit()->IsOverwriteMode();
    return false;
}

void  TextField::SetWideCursor(bool wideCursor)
{
    if (pDocument->HasEditorKit())
    {
        if (wideCursor)
            GetEditorKit()->SetWideCursor();
        else
            GetEditorKit()->ClearWideCursor();
        SetDirtyFlag();
    }
}
#else
void TextField::SetOverwriteMode(bool) {}
void TextField::SetWideCursor(bool)    {}
bool TextField::IsOverwriteMode() const { return false; }
#endif //#ifdef GFX_ENABLE_KEYBOARD

// Special event handler; mouse wheel support
bool TextField::OnMouseWheelEvent(int mwDelta)
{ 
    SF_UNUSED(mwDelta); 
    if (IsMouseWheelEnabled() && IsSelectable())
    {
        int  vscroll = (int)pDocument->GetVScrollOffset();
        vscroll -= mwDelta;
        if (vscroll < 0)
            vscroll = 0;
        if (vscroll > (int)pDocument->GetMaxVScroll())
            vscroll = (int)pDocument->GetMaxVScroll();
        pDocument->SetVScrollOffset((unsigned)vscroll);
        SetDirtyFlag();
        return true;
    }
    return false; 
}

#ifdef GFX_AS2_ENABLE_BITMAPDATA
void TextField::AddIdImageDescAssoc(const char* idStr, Text::ImageDesc* pdesc)
{
    if (!pImageDescAssoc)
        pImageDescAssoc = SF_HEAP_AUTO_NEW(this) StringHashLH<Ptr<Text::ImageDesc> >;
    pImageDescAssoc->Set(idStr, pdesc);
}
void TextField::RemoveIdImageDescAssoc(const char* idStr)
{
    if (pImageDescAssoc)
    {
        pImageDescAssoc->Remove(idStr);
    }
}
void TextField::ClearIdImageDescAssoc()
{
    delete pImageDescAssoc;
    pImageDescAssoc = NULL;
}
#else //SF_NO_FXPLAYER_AS_BITMAPDATA
void TextField::AddIdImageDescAssoc(const char*, Text::ImageDesc*) {}
void TextField::RemoveIdImageDescAssoc(const char*) {}
void TextField::ClearIdImageDescAssoc() {}
#endif //SF_NO_FXPLAYER_AS_BITMAPDATA


#if !defined(SF_NO_FXPLAYER_AS_BITMAPDATA)
void TextField::ProcessImageTags(Text::StyledText::HTMLImageTagInfoArray& imageInfoArray)
{
    SF_UNUSED(imageInfoArray);

    //@IMG
    /*
    UPInt n = imageInfoArray.GetSize();

    if (n > 0)
        SetDirtyFlag();

    for (UPInt i = 0; i < n; ++i)
    {
        Text::StyledText::HTMLImageTagInfo& imgTagInfo = imageInfoArray[i];
        SF_ASSERT(imgTagInfo.pTextImageDesc);
        // TODO: Url should be used to load image by the same way as loadMovie does.
        // At the moment there is no good way to get a callback when movie is loaded.
        // Therefore, at the moment, Url is used as export name only. "imgps://" also
        // may be used.

        ResourceBindData resBindData;
        Ptr<MovieDefImpl> md = GetResourceMovieDef();
        if (md) // can it be NULL?
        {
            bool userImageProtocol = false;
            // check for user protocol img:// and imgps://
            if (imgTagInfo.Url.GetLength() > 0 && (imgTagInfo.Url[0] == 'i' || imgTagInfo.Url[0] == 'I'))
            {
                String urlLowerCase = imgTagInfo.Url.ToLower();
                if (urlLowerCase.Substring(0, 6) == "img://")
                    userImageProtocol = true;
                else if (urlLowerCase.Substring(0, 8) == "imgps://")
                    userImageProtocol = true;
            }
            if (!userImageProtocol)
            {
                if (!GetMovieImpl()->FindExportedResource(md, &resBindData, imgTagInfo.Url))
                {
                    if (GetLog())
                        GetLog()->LogScriptWarning
                        ("ProcessImageTags: can't find a resource for export name '%s'\n", imgTagInfo.Url.ToCStr());
                    continue;
                }
                SF_ASSERT(resBindData.pResource.GetPtr() != 0);
            }
            // Must check resource type, since users can theoretically pass other resource ids.
            if (userImageProtocol || resBindData.pResource->GetResourceType() == Resource::RT_Image)
            {
                // bitmap
                ImageResource* pimgRes = GetMovieImpl()->GetImageResourceByLinkageId(GetResourceMovieDef(), imgTagInfo.Url);
                if (!pimgRes)
                {
                    if (GetLog())
                        GetLog()->LogScriptWarning
                        ("ProcessImageTags: can't load the image '%s'\n", imgTagInfo.Url.ToCStr());
                    continue;
                }
                Ptr<ShapeWithStyles> pimgShape = *SF_HEAP_NEW(GetMovieImpl()->GetMovieHeap()) ShapeWithStyles();
                SF_ASSERT(pimgShape);
                pimgShape->SetToImage(pimgRes, true);
                imgTagInfo.pTextImageDesc->pImageShape = pimgShape;

                float origWidth = 0, origHeight = 0;
                float screenWidth = 0, screenHeight = 0;
                Rect<int> dimr = pimgRes->GetImageInfo()->GetRect();
                screenWidth  = origWidth  = (float)PixelsToTwips(dimr.Width());
                screenHeight = origHeight = (float)PixelsToTwips(dimr.Height());
                if (imgTagInfo.Width)
                    screenWidth = float(imgTagInfo.Width);
                if (imgTagInfo.Height)
                    screenHeight = float(imgTagInfo.Height);

                float baseLineY = origHeight - PixelsToTwips(1.0f);
                baseLineY += imgTagInfo.VSpace;
                imgTagInfo.pTextImageDesc->ScreenWidth  = (unsigned)screenWidth;
                imgTagInfo.pTextImageDesc->ScreenHeight = (unsigned)screenHeight;
                imgTagInfo.pTextImageDesc->Matrix.AppendTranslation(0, -baseLineY);
                imgTagInfo.pTextImageDesc->Matrix.AppendScaling(screenWidth/origWidth, screenHeight/origHeight);

                pDocument->SetCompleteReformatReq();
            }
            else if (resBindData.pResource->GetResourceType() == Resource::RT_SpriteDef)
            {
                // sprite
                //SpriteDef* psprRes = (SpriteDef*)resBindData.pResource.GetPtr();
            }
        }
    }*/
}
#else
void TextField::ProcessImageTags(Text::StyledText::HTMLImageTagInfoArray&) 
{
    SF_DEBUG_WARNING(1, "<IMG> tags are not supported due to defined SF_NO_FXPLAYER_AS_BITMAPDATA.\n");
}
#endif //#if !defined(SF_NO_FXPLAYER_AS_BITMAPDATA)

void TextField::ReplaceText(const wchar_t* ptext, UPInt beginPos, UPInt endPos, UPInt textLen)
{
    if (pDocument->HasEditorKit())
    {
        textLen = pDocument->ReplaceText(ptext, beginPos, endPos, textLen);
        if (pDocument->HasEditorKit())
        {
            // Replace text does not change the cursor position, unless
            // this position doesn't exist anymore.
            UPInt newLen = pDocument->GetLength();
            if (GetEditorKit()->GetCursorPos() > newLen)
                GetEditorKit()->SetCursorPos(newLen, false);
        }
        if (HasAvmObject())
            GetAvmTextField()->UpdateVariable();
        SetNeedUpdateLayoutFlag();
        NotifyChanged();
    }
}

RectF TextField::GetCursorBounds(UPInt cursorPos, float* phscroll, float* pvscroll)
{
    if (pDocument->HasEditorKit())
    {
        Text::EditorKit* peditor = GetEditorKit();
        RectF newCursorRect;
        if (peditor->CalcAbsCursorRectOnScreen(cursorPos, &newCursorRect, NULL, NULL))
        {
            if (phscroll)
                *phscroll = float(pDocument->GetHScrollOffset());
            if (pvscroll)
                *pvscroll = float(pDocument->GetVScrollOffset());
            return newCursorRect;
        }
    }
    if (phscroll)
        *phscroll = 0;
    if (pvscroll)
        *pvscroll = 0;
    return RectF();
}


#ifndef SF_NO_IME_SUPPORT
FontResource* TextField::GetFontResource()
{
    if (!pDef->FontId.GetIdIndex())
        return NULL;
    // resolve font name, if possible
    ResourceBindData fontData = pBinding->GetResourceData(pDef->pFont);
    if (!fontData.pResource)
    {
        Ptr<Log> plog = GetLog();
        if (plog)
        {
            plog->LogError("Resource for font id = %d is not found in text field id = %d, def text = '%s'", 
                           pDef->FontId.GetIdIndex(), GetId().GetIdIndex(), pDef->DefaultText.ToCStr());
        }
    }
    else if (fontData.pResource->GetResourceType() != Resource::RT_Font)
    {
        Ptr<Log> plog = GetLog();
        if (plog)
        {
            plog->LogError("Font id = %d is referring to non-font resource in text field id = %d, def text = '%s'", 
                           pDef->FontId.GetIdIndex(), GetId().GetIdIndex(), pDef->DefaultText.ToCStr());
        }
    }
    else
    {
        FontResource* pfont = (FontResource*)fontData.pResource.GetPtr();
        return pfont;
    }
    return NULL;
}

void TextField::CreateCompositionString()
{
    if (pDocument->HasEditorKit())
    {
		Ptr<Text::CompositionString> cs = GetEditorKit()->CreateCompositionString();
        // need to grab styles for IME compos/clause/etc
/*       
		if (ASTextFieldObj)
        {
            Text::IMEStyle* pcsStyles = ASTextFieldObj->GetIMECompositionStringStyles();
            if (pcsStyles)
                cs->UseStyles(*pcsStyles);
        }
*/
        cs->SetText(L"");
        cs->SetPosition(GetEditorKit()->GetCursorPos());
        SetDirtyFlag();
    }
}

void TextField::ClearCompositionString()
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
            cs->SetText(L"");

        SetDirtyFlag();
    }
}

void TextField::ReleaseCompositionString()
{
    if (pDocument->HasEditorKit())
    {
        GetEditorKit()->ReleaseCompositionString();
        SetDirtyFlag();
    }
}

void TextField::CommitCompositionString(const wchar_t* pstr, UPInt len)
{
    if (pDocument->HasEditorKit())
    {
		Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            if (!pstr)
            {
                pstr = cs->GetText();
                len  = cs->GetLength();
            }
            else if (len == SF_MAX_UPINT)
            {
                len = SFwcslen(pstr);
            }
            // check for max length
            if (pDocument->HasMaxLength() && pDocument->GetLength() + len > pDocument->GetMaxLength())
                len = pDocument->GetMaxLength() - pDocument->GetLength();
            pDocument->GetStyledText()->InsertString(pstr, cs->GetPosition(), len, 
                cs->GetDefaultTextFormat(), pDocument->GetDefaultParagraphFormat());
            cs->SetText(L"");
            UPInt newCursorPos = cs->GetPosition() + len;
            cs->SetPosition(newCursorPos);
            GetEditorKit()->SetCursorPos(newCursorPos, false);
            if (HasAvmObject())
                GetAvmTextField()->UpdateVariable();
            NotifyChanged();
        }
    }
}

void TextField::SetCompositionStringText(const wchar_t* pstr, UPInt len)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetText(pstr, len);
            SetDirtyFlag();
        }
    }
}

void TextField::SetCompositionStringPosition(UPInt pos)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetPosition(pos);
            SetDirtyFlag();
        }
    }
}

UPInt TextField::GetCompositionStringPosition() const
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            return cs->GetPosition();
        }
    }
    return SF_MAX_UPINT;
}

UPInt TextField::GetCompositionStringLength() const
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            return cs->GetLength();
        }
    }
    return 0;
}

void TextField::SetCursorInCompositionString(UPInt pos)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->SetCursorPosition(pos);
            SetDirtyFlag();
        }
    }
}

void TextField::HighlightCompositionStringText(UPInt posInCompStr, UPInt len, Text::IMEStyle::Category hcategory)
{
    if (pDocument->HasEditorKit())
    {
        Ptr<Text::CompositionString> cs = GetEditorKit()->GetCompositionString();
        if (cs)
        {
            cs->HighlightText(posInCompStr, len, hcategory);
            SetDirtyFlag();
        }
    }
}
#endif //#ifdef SF_NO_IME_SUPPORT

UInt32 TextField::GetSelectedTextColor() const
{
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetActiveSelectionTextColor();
    }
    return GFX_ACTIVE_SEL_TEXTCOLOR;
}

UInt32 TextField::GetSelectedBackgroundColor() const
{
    if (pDocument->HasEditorKit())
    {
        return GetEditorKit()->GetActiveSelectionBackgroundColor();
    }
    return GFX_ACTIVE_SEL_BKCOLOR;
}

void TextField::UpdateUrlStyles()
{
#ifdef GFX_ENABLE_CSS
    if (pCSSData)
    {
        for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
        {
            if (pCSSData->HasASStyleSheet())
            {
                const Text::Style* pAstyle = 
                    pCSSData->GetTextStyleManager()->GetStyle(Text::StyleManager::CSS_Tag, "a");

                const Text::Style* pALinkStyle = 
                    pCSSData->GetTextStyleManager()->GetStyle(Text::StyleKey::CSS_Tag, "a:link");

                Text::TextFormat fmt(Memory::GetHeapByAddress(this));
                if (pAstyle)
                    fmt = fmt.Merge(pAstyle->mTextFormat);
                if (pALinkStyle)
                    fmt = fmt.Merge(pALinkStyle->mTextFormat);

                // save current formatting data first
                UPInt stPos = (UPInt)pCSSData->UrlZones[i].FirstIndex(), endPos = (UPInt)pCSSData->UrlZones[i].NextIndex();

                pDocument->SetTextFormat(fmt, stPos, endPos);

                // Update saved fmt
                pCSSData->UrlZones[i].GetData().SavedFmt = 
                    *pDocument->GetStyledText()->CopyStyledText(stPos, endPos);
            }
        }
    }
#endif
}

void TextField::ChangeUrlFormat(LinkEvent event, unsigned mouseIndex, const Range* purlRange)
{
#ifdef GFX_ENABLE_CSS
    if (!HasStyleSheet())
        return;

    const char* styleName = NULL;

    //CSSHolder::UrlZone* purlZone = NULL;
    RangeData<CSSHolderBase::UrlZone>* purlZone = NULL;
    switch(event)
    {
    case Link_press:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (pCSSData->MouseState[mouseIndex].HitBit)
                return; // this mouse already has hit the url
            pCSSData->MouseState[mouseIndex].HitBit = true;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().HitCount > 0)
            {
                ++purlZone->GetData().HitCount;
                return; // hit counter is not zero - do nothing
            }
        }
        else
        {
            for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
            {
                if (pCSSData->UrlZones[i].Intersects(*purlRange))
                {
                    purlZone = &pCSSData->UrlZones[i];
                    //purlZone->StartPos = (UPInt)pCSSData->UrlZones[i].Index;
                    //purlZone->EndPos = purlZone->StartPos + pCSSData->UrlZones[i].Length;
                    pCSSData->MouseState[mouseIndex].UrlZoneIndex = i + 1;
                    pCSSData->MouseState[mouseIndex].HitBit = true;
                    break;
                }
            }
        }
        if (purlZone)
        {
            if (purlZone->GetData().HitCount++ == 0)
            {
                styleName = "a:active";
            }
            else
                return;
        }
        break;
    case Link_release:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (!pCSSData->MouseState[mouseIndex].HitBit)
                return; // this mouse didn't hit the url
            pCSSData->MouseState[mouseIndex].HitBit = false;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            if (!pCSSData->MouseState[mouseIndex].OverBit) // if neither hit nor over left - clean up
                pCSSData->MouseState[mouseIndex].UrlZoneIndex = 0;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().HitCount > 0)
            {
                --purlZone->GetData().HitCount;
                if (purlZone->GetData().HitCount != 0)
                    return; // hit count is still bigger than zero, return
                if (purlZone->GetData().OverCount > 0)
                    styleName = "a:hover";
            }
        }
        break;
    case Link_rollover:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (pCSSData->MouseState[mouseIndex].OverBit)
                return; // this mouse already has rolled over the url
            pCSSData->MouseState[mouseIndex].OverBit = true;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (purlZone->GetData().OverCount > 0)
            {
                ++purlZone->GetData().OverCount;
                return; // over counter is not zero - do nothing
            }
        }
        else
        {
            for (unsigned i = 0, zonesNum = (unsigned)pCSSData->UrlZones.Count(); i < zonesNum; ++i)
            {
                if (pCSSData->UrlZones[i].Intersects(*purlRange))
                {
                    purlZone = &pCSSData->UrlZones[i];
                    //purlZone->StartPos = (UPInt)pCSSData->UrlZones[i].Index;
                    //purlZone->EndPos = purlZone->StartPos + pCSSData->UrlZones[i].Length;
                    pCSSData->MouseState[mouseIndex].UrlZoneIndex = i + 1;
                    pCSSData->MouseState[mouseIndex].OverBit = true;
                    break;
                }
            }
        }
        if (purlZone)
        {
            if (purlZone->GetData().HitCount == 0 && purlZone->GetData().OverCount++ == 0)
            {
                styleName = "a:hover";
            }
            else
                return;
        }
        break;
    case Link_rollout:
        if (pCSSData->MouseState[mouseIndex].UrlZoneIndex > 0)
        {
            if (!pCSSData->MouseState[mouseIndex].OverBit)
                return; // this mouse didn't hit the url
            pCSSData->MouseState[mouseIndex].OverBit = false;
            UPInt urlZoneIdx = pCSSData->MouseState[mouseIndex].UrlZoneIndex - 1;
            purlZone = &pCSSData->UrlZones[urlZoneIdx];
            if (!pCSSData->MouseState[mouseIndex].HitBit) // if neither hit nor over left - clean up
                pCSSData->MouseState[mouseIndex].UrlZoneIndex = 0;
            if (purlZone->GetData().OverCount > 0)
            {
                --purlZone->GetData().OverCount;
                if (purlZone->GetData().OverCount != 0)
                    return; // over count is still bigger than zero, return
                if (purlZone->GetData().HitCount > 0)
                    styleName = "a:active";
            }
        }
        break;
    }

    if (purlZone && purlZone->GetData().SavedFmt)
    {
        // restore originally saved format
        UPInt stPos = (UPInt)purlZone->FirstIndex();
        UPInt enPos = (UPInt)purlZone->NextIndex();
        pDocument->RemoveText(stPos, enPos);
        pDocument->InsertStyledText(*purlZone->GetData().SavedFmt.GetPtr(), stPos);
    }

    if (styleName)
    {
        // now need to apply a:hover style from CSS, combined with "a:link" and "a" ones
        const Text::Style* pstyle = 
            pCSSData->GetTextStyleManager()->GetStyle(Text::StyleKey::CSS_Tag, styleName);
        if (pstyle && purlZone)
        {
            const Text::Style* pAstyle = 
                pCSSData->GetTextStyleManager()->GetStyle(Text::StyleKey::CSS_Tag, "a");

            const Text::Style* pALinkStyle = 
                pCSSData->GetTextStyleManager()->GetStyle(Text::StyleKey::CSS_Tag, "a:link");

            Text::TextFormat fmt(Memory::GetHeapByAddress(this));
            if (pAstyle)
                fmt = fmt.Merge(pAstyle->mTextFormat);
            if (pALinkStyle)
                fmt = fmt.Merge(pALinkStyle->mTextFormat);
            fmt = fmt.Merge(pstyle->mTextFormat);

            // save current formatting data first
            UPInt stPos = (UPInt)purlZone->FirstIndex(), endPos = (UPInt)purlZone->NextIndex();

            pDocument->SetTextFormat(fmt, stPos, endPos);
        }
    }
#else
    SF_UNUSED3(event, mouseIndex, purlRange);
#endif // GFX_ENABLE_CSS
}

void TextField::SetX(Double x)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    // and TransformToTextRectSpace() above.
    if (NumberUtil::IsNaN(x))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(x) || NumberUtil::IsPOSITIVE_INFINITY(x))
        x = 0;
    Matrix m    = GetMatrix();
    Double newX = PixelsToTwips(x);
    // We need to modify the coordinate being set. We need to
    // use TextRect.TopLeft point for this modification. 
    // First of all translate the coordinate to the TextRect coordinate
    // space, do the adjustment by the -TextRect.TopLeft() and translate
    // coordinate back to parent's coordinate space.
    Render::PointF     p((float)newX);
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.x         -= r.x;
    p           = m.Transform(p);
    
    DisplayObjectBase::SetX(TwipsToPixels(p.x));
    if (pGeomData)
        pGeomData->X = Alg::IRound(newX);
}

void TextField::SetY(Double y)
{
    // NOTE: If updating this logic, also update Value::ObjectInterface::SetDisplayInfo
    // and TransformToTextRectSpace() above.
    if (NumberUtil::IsNaN(y))
        return;
    if (NumberUtil::IsNEGATIVE_INFINITY(y) || NumberUtil::IsPOSITIVE_INFINITY(y))
        y = 0;
    Double newY = PixelsToTwips(y);
    Matrix m    = GetMatrix();
    // We need to modify the coordinate being set. We need to
    // use TextRect.TopLeft point for this modification. 
    // First of all translate the coordinate to the TextRect coordinate
    // space, do the adjustment by the -TextRect.TopLeft() and translate
    // coordinate back to parent's coordinate space.
    Render::PointF     p(0, (float)newY);
    p           = m.TransformByInverse(p);
    Render::PointF r   = pDocument->GetViewRect().TopLeft();
    p.y         -= r.y;
    p           = m.Transform(p);
    DisplayObjectBase::SetY(TwipsToPixels(p.y));
    if (pGeomData)
        pGeomData->Y = Alg::IRound(newY);
}

Double TextField::GetX() const
{
    GeomDataType geomData;
    const_cast<TextField*>(this)->UpdateAndGetGeomData(&geomData);//? revise
    return TwipsToPixels(Double(geomData.X));
}

Double TextField::GetY() const
{
    GeomDataType geomData;
    const_cast<TextField*>(this)->UpdateAndGetGeomData(&geomData); //? revise
    return TwipsToPixels(Double(geomData.Y));
}

void TextField::SetWidth(Double width)
{
    float newWidth = float(PixelsToTwips(width));
    RectF viewRect = pDocument->GetViewRect();
    viewRect.x2 = viewRect.x1 + newWidth;
    pDocument->SetViewRect(viewRect);
    SetNeedUpdateGeomData();
}

void TextField::SetHeight(Double height)
{
    float newHeight = float(PixelsToTwips(height));
    RectF viewRect = pDocument->GetViewRect();
    viewRect.y2 = viewRect.y1 + newHeight;
    pDocument->SetViewRect(viewRect);
    SetNeedUpdateGeomData();
}

void TextField::SetXScale(Double xscale)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetXScale(xscale);
}

void TextField::SetYScale(Double yscale)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetYScale(yscale);
}

void TextField::SetRotation(Double rotation)
{
    SetNeedUpdateGeomData();
    DisplayObjectBase::SetRotation(rotation);
}

void TextField::SetBackground(bool b)
{
    Color c = GetBackgroundColor();
    if (b)
        c.SetAlpha(255);
    else
        c.SetAlpha(0);
    SetBackgroundColor(c);
    SetDirtyFlag();
}

void TextField::SetBackgroundColor(UInt32 rgb)
{
    pDocument->SetBackgroundColor((GetBackgroundColor32() & 0xFF000000u) | (rgb & 0xFFFFFFu));
    SetDirtyFlag();
}

void TextField::SetBackgroundColor(const Color& c)
{
    pDocument->SetBackgroundColor(c.ToColor32());
    SetDirtyFlag();
}

void TextField::SetBorder(bool b)
{
    Color c = GetBorderColor();
    if (b)
        c.SetAlpha(255);
    else
        c.SetAlpha(0);
    SetBorderColor(c);
    SetDirtyFlag();
}

void TextField::SetBorderColor(const Color& c)
{
    pDocument->SetBorderColor(c.ToColor32());
    SetDirtyFlag();
}

void TextField::SetBorderColor(UInt32 rgb)
{
    pDocument->SetBorderColor((GetBorderColor32() & 0xFF000000u) | (rgb & 0xFFFFFFu));
    SetDirtyFlag();
}

void TextField::SetText(const wchar_t* pnewText, bool reqHtml)
{
    bool currHtml = IsHtml();
    if (reqHtml && !currHtml)        { SetHtml(true); }
    else if (!reqHtml && currHtml)   { SetHtml(false); }

    enum { StackBuffSize = 256 };
    char* pbuff;
    char stackBuff[StackBuffSize];
    UPInt charSz = SFwcslen(pnewText);
    pbuff = charSz < StackBuffSize ? stackBuff : (char*)SF_HEAP_AUTO_ALLOC(this, charSz * 5 + 1);
    UTF8Util::EncodeString(pbuff, pnewText);

    SetTextValue(pbuff, reqHtml);

    if (charSz >= StackBuffSize) SF_FREE(pbuff);
}

void TextField::SetTextColor(UInt32 rgb)
{
    // The arg is 0xRRGGBB format.
    Text::TextFormat colorTextFmt(Memory::GetHeapByAddress(this));
    colorTextFmt.SetColor32(rgb);

    pDocument->SetTextFormat(colorTextFmt);

    colorTextFmt = *pDocument->GetDefaultTextFormat();
    colorTextFmt.SetColor32(rgb);
    SetDefaultTextFormat(colorTextFmt);
    SetDirtyFlag();
}

Color TextField::GetTextColor() const
{
    const Text::TextFormat* ptf = pDocument->GetDefaultTextFormat();
    SF_ASSERT(ptf);
    return ptf->GetColor();
}

UInt32 TextField::GetTextColor32() const
{
    Color c = GetTextColor();
    return (((UInt32)c.GetRed()) << 16) | (((UInt32)c.GetGreen()) << 8) | ((UInt32)c.GetBlue());
}

bool TextField::SetShadowStyle(const char *pstr)
{
    if (!pShadow)
        if ((pShadow = SF_HEAP_AUTO_NEW(this) ShadowParams(GetStringManager())) == 0)
            return false;

    // The arg is 0xRRGGBB format.
    UInt32  rgb = pDocument->GetShadowColor();
    pShadow->ShadowColor.SetBlue((UByte)rgb);
    pShadow->ShadowColor.SetGreen((UByte)(rgb >> 8));
    pShadow->ShadowColor.SetRed((UByte)(rgb >> 16));

    // When using the old fasioned shadow we have to disable the new one.
    pDocument->DisableSoftShadow();

reset:
    pShadow->ShadowOffsets.Clear();
    pShadow->TextOffsets.Clear();

    // Parse shadow style text to generate the offset arrays.
    // Format is similar to "s{1,0.5}{-1,0.5}t{0,0}".
    const char *p = pstr;
    ArrayLH<Render::PointF> *offsets = 0;
    while (*p)
    {
        if (*p == 's' || *p == 'S')
        {
            offsets = &pShadow->ShadowOffsets;
            p++;
        }
        else if (*p == 't' || *p == 'T')
        {
            offsets = &pShadow->TextOffsets;
            p++;
        }
        else if (*p == '{' && offsets) // {x,y}
        {
            char pn[24];
            p++;
            const char *q = p;
            while (*q && *q != ',') q++;
            if (!*q || q-p > 23) { pstr = pShadow->ShadowStyleStr.ToCStr(); goto reset; }
            memcpy(pn,p,q-p);
            pn[q-p] = 0;
            float x = (float)(SFstrtod(pn, NULL) * 20.0);
            q++;
            p = q;

            while (*q && *q != '}') q++;
            if (!*q || q-p > 23) { pstr = pShadow->ShadowStyleStr.ToCStr(); goto reset; }
            memcpy(pn,p,q-p);
            pn[q-p] = 0;
            float y = (float)(SFstrtod(pn, NULL) * 20.0);
            q++;
            p = q;

            offsets->PushBack(Render::PointF(x,y));
        }
        else
        {
            pstr = pShadow->ShadowStyleStr.ToCStr();
            goto reset;
        }
    }

    if (*pstr)
        pShadow->ShadowStyleStr = GetStringManager()->CreateString(pstr);
    SetDirtyFlag();
    return true;
}

void TextField::SetShadowColor(UInt32 rgb)
{
    pDocument->SetShadowColor(rgb);
    if (pShadow)
    {
        // The arg is 0xRRGGBB format.
        UInt32  rgb = pDocument->GetShadowColor();
        pShadow->ShadowColor.SetBlue((UByte)rgb);
        pShadow->ShadowColor.SetGreen((UByte)(rgb >> 8));
        pShadow->ShadowColor.SetRed((UByte)(rgb >> 16));
    }
    SetDirtyFlag();
}

ASString TextField::GetShadowStyle() const
{
    return (pShadow ? pShadow->ShadowStyleStr : GetStringManager()->CreateEmptyString());
}


SPInt TextField::GetCursorPos() const
{
    return (HasEditorKit()) ? (SPInt)GetEditorKit()->GetCursorPos() : -1;
}

bool TextField::SetRestrict(const ASString& restrStr)
{
    RestrictString = restrStr;
    return ParseRestrict(restrStr);
}

const ASString* TextField::GetRestrict() const
{
    if (pRestrict)
        return &RestrictString;
    return NULL;
}

void TextField::ClearRestrict()
{
    delete pRestrict;
    pRestrict = NULL;
    RestrictString = GetStringManager()->CreateEmptyString();
}

bool TextField::ParseRestrict(const ASString& restrStr)
{
    delete pRestrict;

    if ((pRestrict = new RestrictParams()) == 0)
        return false;
    void* ptr = 0;

    const char *pstr    = restrStr.ToCStr();
    bool negative       = false;
    const char *pestr   = pstr + restrStr.GetSize();
    UInt32 firstChInRange = 0, prevCh = 0;
    for (; pstr < pestr; )
    {
        UInt32 ch = UTF8Util::DecodeNextChar(&pstr);
        if (ch == '^')
        {
            negative = !negative;
            continue;
        }
        else if (ch == '\\')
        {
            if (pstr >= pestr)
                break; // safety check
            ch = UTF8Util::DecodeNextChar(&pstr); // skip slash
        }
        else if (ch == '-')
        {
            firstChInRange = prevCh;
            continue;
        }

        if (!firstChInRange)
            firstChInRange = ch;
        if (ch < firstChInRange)
            ch = firstChInRange;

        // range is firstChInRange..ch
        if (!negative)
            pRestrict->RestrictRanges.SetRange(
            (SPInt)firstChInRange, 
            ch - firstChInRange + 1, ptr);
        else
        {
            if (pRestrict->RestrictRanges.Count() == 0)
                // TODO, ranges do not support full 32-bit (SPInt is used for index)
                pRestrict->RestrictRanges.SetRange(0, 65536, ptr);

            pRestrict->RestrictRanges.ClearRange(
                (SPInt)firstChInRange, 
                ch - firstChInRange + 1);
        }

        prevCh          = ch;
        firstChInRange  = 0;
    }
    return true;
}

Ptr<Render::TreeNode> TextField::CreateRenderNode(Render::Context& context) const
{
    Ptr<Render::TreeText> tshp = *context.CreateEntry<Render::TreeText>();
    tshp->Init(pDocument);
    return tshp;
}

void TextField::NotifyChanged()
{
    if (HasAvmObject())
        GetAvmTextField()->NotifyChanged();
    SetDirtyFlag();
}

//////////////////////////////////////////////////////////////////////////
// TextDocumentListener
//
TextField* TextField::TextDocumentListener::GetTextField() const
{
    return (TextField*)(((UByte*)this) - 
        ((UByte*)&(((TextField*)this)->TextDocListener) - 
        (UByte*)((TextField*)this)) );
}
TextField::TextDocumentListener::TextDocumentListener() : 
DocumentListener(Mask_All & (~Mask_OnLineFormat)) 
{
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans)
    {
        if (ptrans->HandlesCustomWordWrapping())
        {
            HandlersMask |= Mask_OnLineFormat;
        }
    }
}

void TextField::TextDocumentListener::TranslatorChanged()
{
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans && ptrans->HandlesCustomWordWrapping())
        HandlersMask |= Mask_OnLineFormat;
    else
        HandlersMask &= (~Mask_OnLineFormat);
}

void TextField::TextDocumentListener::OnScroll(Text::DocView&)
{
    TextField* ptextField = GetTextField();
    if (ptextField->HasAvmObject())
        ptextField->GetAvmTextField()->OnScroll();
}

// events
void TextField::TextDocumentListener::View_OnHScroll(Text::DocView& view, unsigned newScroll)
{
    SF_UNUSED(newScroll);
    OnScroll(view);
}
void TextField::TextDocumentListener::View_OnVScroll(Text::DocView& view, unsigned newScroll)
{
    SF_UNUSED(newScroll);
    OnScroll(view);
}
void TextField::TextDocumentListener::View_OnMaxScrollChanged(Text::DocView& view)
{
    OnScroll(view);
}
bool TextField::TextDocumentListener::View_OnLineFormat(Text::DocView&, Text::DocView::LineFormatDesc& desc) 
{ 
    Ptr<Translator> ptrans = GetTextField()->GetMovieImpl()->GetTranslator();
    if (ptrans)
    {
        Translator::LineFormatDesc tdesc;
        tdesc.CurrentLineWidth = TwipsToPixels(desc.CurrentLineWidth);
        tdesc.DashSymbolWidth  = TwipsToPixels(desc.DashSymbolWidth);
        tdesc.LineStartPos     = desc.LineStartPos;
        tdesc.LineWidthBeforeWordWrap = TwipsToPixels(desc.LineWidthBeforeWordWrap);
        tdesc.NumCharsInLine   = desc.NumCharsInLine;
        tdesc.pWidths          = desc.pWidths;
        tdesc.pParaText        = desc.pParaText;
        tdesc.ParaTextLen      = desc.ParaTextLen;
        tdesc.ProposedWordWrapPoint = desc.ProposedWordWrapPoint;
        tdesc.VisibleRectWidth = TwipsToPixels(desc.VisibleRectWidth);
        tdesc.UseHyphenation   = desc.UseHyphenation;
        tdesc.Alignment        = desc.Alignment;
        // it is ok to modify values in the desc.pCharWidths since
        // they are not used by further formatting logic.
        for (UPInt i = 0; i < tdesc.NumCharsInLine; ++i)
            desc.pWidths[i] = TwipsToPixels(desc.pWidths[i]);
        if (ptrans->OnWordWrapping(&tdesc))
        {
            desc.ProposedWordWrapPoint = tdesc.ProposedWordWrapPoint;
            desc.UseHyphenation        = tdesc.UseHyphenation;
            return true;
        }
    }
    return false; 
}

void    TextField::TextDocumentListener::View_OnChanged(Text::DocView&)    
{
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag();
}

// editor events
void    TextField::TextDocumentListener::Editor_OnChanged(Text::EditorKitBase& editor) 
{ 
    SF_UNUSED(editor); 
    TextField* peditChar = GetTextField();
    if (peditChar->HasAvmObject())
        peditChar->GetAvmTextField()->UpdateVariable();
    peditChar->NotifyChanged();
}

void    TextField::TextDocumentListener::Editor_OnCursorMoved(Text::EditorKitBase& editor)
{
    SF_UNUSED(editor); 
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag();
}

void    TextField::TextDocumentListener::Editor_OnCursorBlink(Text::EditorKitBase& editor, bool cursorState)
{
    SF_UNUSED(editor);
    SF_UNUSED(cursorState);
    TextField* peditChar = GetTextField();
    peditChar->SetDirtyFlag(); //? will we blink cursor more gracefully?
}

String TextField::TextDocumentListener::GetCharacterPath()
{
    String ret;
    GetTextField()->GetAbsolutePath(&ret);
    return ret;
}

}} // namespace Scaleform::GFx
