/**********************************************************************

Filename    :   AvmTextField.cpp
Content     :   Implementation of AS3 part of TextField character
Created     :   Jan, 2010
Authors     :   Artem Bolgar

Copyright   :   (c) 2001-2010 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "GFx/AS3/AS3_AvmTextField.h"
#include "GFx/Text/Text_DocView.h"
#include "GFx/AS3/AS3_MovieRoot.h"
#include "GFx/AS3/Obj/Events/AS3_Obj_Events_TextEvent.h"
#include "GFx/AS3/Obj/Gfx/AS3_Obj_Gfx_TextEventEx.h"

namespace Scaleform {
namespace GFx {
namespace AS3 {

AvmTextField::AvmTextField(TextField* ptf) 
:
    AvmInteractiveObj(ptf)
{
    ptf->SetHtml(true); // seems to me AS3 text fields are always html-enabled
}

TextFieldDef* AvmTextField::GetTextFieldDef() const
{
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    return static_cast<TextField*>(pDispObj)->GetTextFieldDef();
}

bool AvmTextField::HasStyleSheet() const 
{ 
#ifdef GFX_ENABLE_CSS
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
    //const TextField::CSSHolderBase* pcss = GetTextField()->GetCSSData();
    //return pcss && static_cast<const CSSHolder*>(pcss)->pASStyleSheet.GetPtr() != NULL; 
    return false;
#else
    return false;
#endif
}

const Text::StyleManager* AvmTextField::GetStyleSheet() const 
{ 
#ifdef GFX_ENABLE_CSS
    SF_ASSERT(pDispObj); 
    SF_ASSERT(pDispObj->GetType() == CharacterDef::TextField);
//    const TextField::CSSHolderBase* pcss = GetTextField()->GetCSSData();
//    return (HasStyleSheet()) ? static_cast<const CSSHolder*>(pcss)->pASStyleSheet->GetTextStyleManager() : NULL; 
    return NULL;
#else
    return NULL;
#endif
}

bool AvmTextField::OnMouseEvent(const EventId& event)
{ 
    AvmInteractiveObj::OnEvent(event);
    if (event.Id == EventId::Event_Click)
    {
        TextField* ptf       = GetTextField();
        // if url is clicked - execute the action
        if (ptf->IsHtml() && ptf->MayHaveUrl())
        {
            Render::PointF p;
            if (ptf->IsUrlUnderMouseCursor(event.MouseIndex, &p))
            {
                UPInt pos = ptf->GetCharIndexAtPoint(p.x, p.y);
                if (!Alg::IsMax(pos))
                {
                    const Text::TextFormat* ptextFmt;
                    if (ptf->GetTextAndParagraphFormat(&ptextFmt, NULL, pos))
                    {
                        if (ptextFmt->IsUrlSet())
                        {
                            const char* url = ptextFmt->GetUrl();

                            // url should represent link in event protocol:
                            // "event:eventString"
                            MovieImpl* proot = GetMovieImpl();
                            if (proot && String::CompareNoCase(url, "event:", sizeof("event:") - 1) == 0)
                            {
                                url += sizeof("event:") - 1;
                                
                                // fire the link event
                                const ASString& evtName = GetAS3Root()->GetStringManager()->CreateString("link");
                                Instances::DisplayObject* as3obj = GetAS3Obj();
                                if (as3obj->HasEventHandler(evtName, false))
                                {
                                    SPtr<Instances::TextEvent> evt;
                                    Value params[] = { Value(evtName), Value(true), Value(true) };
                                    ASVM* asvm = GetAS3Root()->GetAVM();
                                    asvm->ConstructInstance(evt, asvm->ExtensionsEnabled ? asvm->TextEventExClass : asvm->TextEventClass, 3, params);
                                    SF_ASSERT(evt);
                                    evt->Target = as3obj;
                                    evt->SetText(GetAS3Root()->GetStringManager()->CreateString(url));

                                    if (asvm->ExtensionsEnabled)
                                    {
                                        Instances::TextEventEx* evtExt = static_cast<Instances::TextEventEx*>(evt.Get());
                                        evtExt->SetControllerIdx(event.ControllerIndex);
                                    }

                                    as3obj->Dispatch(evt, GetDispObj());
                                }
                            }
                            //printf ("%s\n", url);
                        }
                    }
                }
            }
        }

        return true;
    }
    return false; 
}

bool AvmTextField::OnCharEvent(wchar_t wcharCode, unsigned controllerIdx)
{
    SF_ASSERT(wcharCode);
    bool rv = true;

    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_textInput);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::TextEvent> evt;
        Value params[] = { Value(evtName), Value(true), Value(true) };
        ASVM* asvm = GetAS3Root()->GetAVM();
        asvm->ConstructInstance(evt, asvm->ExtensionsEnabled ? asvm->TextEventExClass : asvm->TextEventClass, 3, params);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        evt->SetText(wcharCode);

        if (asvm->ExtensionsEnabled)
        {
            Instances::TextEventEx* evtExt = static_cast<Instances::TextEventEx*>(evt.Get());
            evtExt->SetControllerIdx(controllerIdx);
        }

        rv = as3obj->Dispatch(evt, GetDispObj());
    }
    return rv;
//     // cursor positioning is available for selectable readonly textfields
//     MovieRoot* proot = GetAS2Root();
//     if (proot && HasEditorKit() && (!IsReadOnly() || IsSelectable()))
//     {
//         // need to queue up key processing. Flash doesn't handle 
//         // keys from keyevents, it executes them afterward.
//         GASValueArray params;
//         params.PushBack(Value(int(2)));
//         params.PushBack(Value(int(wcharCode)));
//         proot->InsertEmptyAction()->SetAction(this, 
//             AvmTextField::KeyProcessing, &params);
//     }
}

void AvmTextField::OnEventLoad()
{
    // should it be here still? For sprites it was moved in Super
    GetAvmParent()->InsertChildToPlayList(GetIntObj());
    GetIntObj()->ModifyOptimizedPlayList();

    AvmInteractiveObj::OnEventLoad();
//     // finalize the initialization. We need to initialize text here rather than in ctor
//     // since the name of instance is not set yet in the ctor and setting text might cause
//     // GFxTranslator to be invoked, which uses the instance name.
//     TextField* ptf = GetTextField();
//     if (GetTextFieldDef()->DefaultText.GetLength() > 0)
//     {
//         ptf->SetTextValue(GetTextFieldDef()->DefaultText.ToCStr(), ptf->IsHtml());
//     }
//     else
//     {
//         // This assigns both Text and HtmlText vars.
//         ptf->SetTextValue("", ptf->IsHtml(), false);
//     }
}

void AvmTextField::UpdateAutosizeSettings()
{
    TextField* ptf          = GetTextField();
    const bool  autoSizeByX = (ptf->IsAutoSize() && !ptf->IsWordWrap());
    const bool  autoSizeByY = (ptf->IsAutoSize());
    if (autoSizeByX)
        ptf->SetAutoSizeX();
    else
        ptf->ClearAutoSizeX();
    if (autoSizeByY)
        ptf->SetAutoSizeY();
    else
        ptf->ClearAutoSizeY();
    ptf->SetNeedUpdateGeomData();
}

#if 0
bool AvmTextField::SetMember(GASEnvironment* penv, const ASString& name, const Value& origVal, const GASPropFlags& flags)
{
    StandardMember member = GetStandardMemberConstant(name);

    // Check, if there are watch points set. Invoke, if any. Do not invoke
    // watch for built-in properties.
    Value val(origVal);
    if (member < M_BuiltInProperty_Begin || member > M_BuiltInProperty_End)
    {
        if (penv && GetTextFieldASObject() && ASTextFieldObj->HasWatchpoint()) // have set any watchpoints?
        {
            Value        newVal;
            if (ASTextFieldObj->InvokeWatchpoint(penv, name, val, &newVal))
            {
                val = newVal;
            }
        }
    }

    if (SetStandardMember(member, origVal, false))
        return true;

    TextField* ptf          = GetTextField();
    SF_ASSERT(ptf);
    // Handle TextField-specific "standard" members.
    switch(member)
    {
    case M_html:
        ptf->SetHtml(val.ToBool(penv));
        return true; 

    case M_htmlText:
        {
            ASString str(val.ToStringVersioned(penv, GetVersion()));
            ptf->SetTextValue(str.ToCStr(), ptf->IsHtml());
            return true;
        }

    case M_text:
        {
            ptf->ResetBlink(1);
            ASString str(val.ToStringVersioned(penv, GetVersion()));
            ptf->SetTextValue(str.ToCStr(), false);
            return true;
        }

    case M_textColor:
        {
            // The arg is 0xRRGGBB format.
            UInt32  rgb = val.ToUInt32(penv);
            ptf->SetTextColor(rgb);
            return true;
        }

    case M_autoSize:
        {
            ASString asStr(val.ToString(penv));
            String str = (val.IsBoolean()) ? 
                ((val.ToBool(penv)) ? "left":"none") : asStr.ToCStr();

            Text::DocView::ViewAlignment oldAlignment = ptf->GetAlignment();
            bool oldAutoSize = ptf->IsAutoSize();
            if (str == "none")
            {
                ptf->ClearAutoSize();
                ptf->SetAlignment(Text::DocView::Align_Left);
            }
            else 
            {
                ptf->SetAutoSize();
                if (str == "left")
                    ptf->SetAlignment(Text::DocView::Align_Left);
                else if (str == "right")
                    ptf->SetAlignment(Text::DocView::Align_Right);
                else if (str == "center")
                    ptf->SetAlignment(Text::DocView::Align_Center);
            }
            if ((oldAlignment != ptf->GetAlignment()) || (oldAutoSize != ptf->IsAutoSize()))
                UpdateAutosizeSettings();
            SetDirtyFlag();
            return true;
        }

    case M_wordWrap:
        {
            bool oldWordWrap = ptf->IsWordWrap();
            bool wordWrap = val.ToBool(penv);
            if (wordWrap != oldWordWrap)
            {
                if (wordWrap)
                    ptf->SetWordWrap();
                else
                    ptf->ClearWordWrap();
                UpdateAutosizeSettings();
            }
            SetDirtyFlag();
            return true;
        }

    case M_multiline:
        {
            bool oldML = ptf->IsMultiline();
            bool newMultiline = val.ToBool(penv);
            if (oldML != newMultiline)
            {
                if (newMultiline)
                    ptf->SetMultiline();
                else
                    ptf->ClearMultiline();
                UpdateAutosizeSettings();
            }
            SetDirtyFlag();
            return true;
        }

    case M_variable:
        {
            //if (val.IsNull() || val.IsUndefined())
            //    VariableName = "";
            //!AB: even if we set variable name pointing to invalid or non-exisiting variable
            // we need to save it and try to update text using it. If we can't get a value 
            // (variable doesn't exist), then text shouldn't be changed.
            {                        
                VariableName = val.ToString(penv);
                UpdateTextFromVariable();
                ptf->SetForceAdvance();
                ptf->ModifyOptimizedPlayListLocal<TextField>(GetMovieImpl());
            }
            return true;
        }

    case M_selectable:
        {
            ptf->SetSelectable(val.ToBool(penv));
            return true;
        }

    case M_embedFonts:
        {
            if (val.ToBool(penv))
                ptf->ClearUseDeviceFont();
            else
                ptf->SetUseDeviceFont();
            return true;
        }

    case M_password:
        {
            bool pswd = ptf->IsPassword();
            bool password = val.ToBool(penv);
            if (pswd != password)
            {
                ptf->SetPassword(password);
                if (password)
                    ptf->SetPasswordMode();
                else
                    ptf->ClearPasswordMode();
                ptf->ForceCompleteReformat();
            }
            SetDirtyFlag();
            return true;
        }

    case M_hscroll:
        {
            int scrollVal = (int)val.ToInt32(penv);
            if (scrollVal < 0)
                scrollVal = 0;
            ptf->SetHScrollOffset((Double)scrollVal);    
            SetDirtyFlag();
            return true;
        }

    case M_scroll:
        {
            int scrollVal = (int)val.ToInt32(penv);
            if (scrollVal < 1)
                scrollVal = 1;
            ptf->SetVScrollOffset((unsigned)(scrollVal - 1));    
            SetDirtyFlag();
            return true;
        }
    case M_border:
        {
            ptf->SetBorder(val.ToBool(penv));
            return true;
        }
    case M_background:
        {
            ptf->SetBackground(val.ToBool(penv));
            return true;
        }
    case M_backgroundColor:
        {
            GASNumber bc = val.ToNumber(penv);
            if (!ASNumberUtil::IsNaN(bc))
            {
                UInt32 c = val.ToUInt32(penv);
                ptf->SetBackgroundColor(c);
            }
            return true;
        }
    case M_borderColor:
        {
            GASNumber bc = val.ToNumber(penv);
            if (!ASNumberUtil::IsNaN(bc))
            {
                UInt32 c = val.ToUInt32(penv);
                ptf->SetBorderColor(c);
            }
            return true;
        }
    case M_type:
        {
            ASString typeStr = val.ToString(penv);
            if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "dynamic"))
            {
                if (ptf->HasEditorKit())
                    ptf->GetEditorKit()->SetReadOnly();
            }
            else if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "input") && !HasStyleSheet())
            {
                Text::EditorKit* peditor = ptf->CreateEditorKit();
                peditor->ClearReadOnly();
            }
            // need to reformat because scrolling might be changed
            ptf->ForceReformat();
            return true;
        }
    case M_maxChars:
        {
            GASNumber ml = val.ToNumber(penv);
            if (!ASNumberUtil::IsNaN(ml) && ml >= 0)
            {
                ptf->SetMaxLength((UPInt)val.ToUInt32(penv));
            }
            return true;
        }
    case M_condenseWhite:
        {
            ptf->SetCondenseWhite(val.ToBool(penv));
            return true;
        }
    case M_antiAliasType:
        {
            ASString typeStr = val.ToString(penv);
            if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "normal"))
            {
                ptf->ClearAAForReadability();
            }
            else if (penv->GetSC()->CompareConstString_CaseCheck(typeStr, "advanced"))
            {
                ptf->SetAAForReadability();                    
            }
            SetDirtyFlag();
            return true;
        }

    case M_mouseWheelEnabled:
        {
            ptf->SetMouseWheelEnabled(val.ToBool(penv));
            return true;
        }
    case M_styleSheet:
        {
#ifdef GFX_ENABLE_CSS
            Object* pobj = val.ToObject(penv);
            if (pobj && pobj->GetObjectType() == Object::Object_StyleSheet)
            {
                if (!pCSSData)
                    pCSSData = new CSSHolder;
                pCSSData->pASStyleSheet = static_cast<GASStyleSheetObject*>(pobj);

                // make the text field read-only
                if (ptf->HasEditorKit())
                {
                    ptf->GetEditorKit()->SetReadOnly();
                }

                SetDirtyFlag();
            }
            else if (pCSSData)
                pCSSData->pASStyleSheet = NULL;
            ptf->CollectUrlZones();
            ptf->UpdateUrlStyles();
#endif //GFX_ENABLE_CSS
            return true;
        }

        // extension
    case M_shadowStyle:
        if (!penv->CheckExtensions())
            break;

        {
            ASString   styleStr = val.ToString(penv);
            const char *pstr = styleStr.ToCStr();

            ptf->SetShadowStyle(pstr);
            return true;
        }

    case M_shadowColor:
        if (!penv->CheckExtensions())
            break;
        {
            ptf->SetShadowColor(val.ToUInt32(penv));
            return true;
        }

    case M_hitTestDisable:
        if (penv->CheckExtensions())
        {
            ptf->SetHitTestDisableFlag(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;

    case M_noTranslate:
        if (penv->CheckExtensions())
        {
            ptf->SetNoTranslate(val.ToBool(GetASEnvironment()));
            ptf->ForceCompleteReformat(); 
            return 1;
        }
        break;

    case M_autoFit:
        if (penv->CheckExtensions())
        {
            if (val.ToBool(penv))
                ptf->SetAutoFit();
            else
                ptf->ClearAutoFit();
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurX:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurX((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurY:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurY((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_blurStrength:
        if (penv->CheckExtensions())
        {
            ptf->SetBlurStrength((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_outline:
        // Not implemented
        //if (GetASEnvironment()->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_fauxBold:
        if (penv->CheckExtensions())
        {
            ptf->SetFauxBold(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_fauxItalic:
        if (penv->CheckExtensions())
        {
            ptf->SetFauxItalic(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowAlpha:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowAlpha((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowAngle:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowAngle((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowBlurX:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowBlurX((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowBlurY:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowBlurY((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowDistance:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowDistance((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowHideObject:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowHideObject(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowKnockOut:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowKnockOut(val.ToBool(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowQuality:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowQuality((unsigned)val.ToUInt32(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowStrength:
        if (penv->CheckExtensions())
        {
            ptf->EnableSoftShadow();
            ptf->SetShadowStrength((float)val.ToNumber(penv));
            SetDirtyFlag();
            return true;
        }
        break;

    case M_shadowOutline:
        // Not implemented
        //if (GetASEnvironment()->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_verticalAutoSize:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
            {
                ptf->ClearAutoSizeY();
                ptf->SetVAlignment(Text::DocView::VAlign_None);
            }
            else 
            {
                ptf->SetAutoSizeY();
                if (str == "top")
                    ptf->SetVAlignment(Text::DocView::VAlign_Top);
                else if (str == "bottom")
                    ptf->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (str == "center")
                    ptf->SetVAlignment(Text::DocView::VAlign_Center);
            }
            ptf->SetNeedUpdateGeomData();
            SetDirtyFlag();
            return true;
        }
        break;

    case M_fontScaleFactor:
        if (penv->CheckExtensions())
        {
            Number factor = val.ToNumber(penv);
            if (factor > 0 && factor < 1000)
                ptf->SetFontScaleFactor((float)factor);
            SetDirtyFlag();
        }
        break;

    case M_verticalAlign:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
                ptf->SetVAlignment(Text::DocView::VAlign_None);
            else 
            {
                if (str == "top")
                    ptf->SetVAlignment(Text::DocView::VAlign_Top);
                else if (str == "bottom")
                    ptf->SetVAlignment(Text::DocView::VAlign_Bottom);
                else if (str == "center")
                    ptf->SetVAlignment(Text::DocView::VAlign_Center);
            }
            SetDirtyFlag();
            return true;
        }
        break;

    case M_textAutoSize:
        if (penv->CheckExtensions())
        {
            ASString asStr(val.ToString(penv));
            String str = asStr.ToCStr();

            if (str == "none")
                ptf->SetTextAutoSize(Text::DocView::TAS_None);
            else 
            {
                if (str == "shrink")
                    ptf->SetTextAutoSize(Text::DocView::TAS_Shrink);
                else if (str == "fit")
                    ptf->SetTextAutoSize(Text::DocView::TAS_Fit);
            }
            SetDirtyFlag();
            return true;
        }
        break;

    case M_useRichTextClipboard:
        if (penv->CheckExtensions())
        {
            ptf->SetUseRichClipboard(val.ToBool(GetASEnvironment()));
            if (ptf->HasEditorKit())
            {
                if (ptf->DoesUseRichClipboard())
                    ptf->GetEditorKit()->SetUseRichClipboard();
                else
                    ptf->GetEditorKit()->ClearUseRichClipboard();
            }
            return 1;
        }
        break;

    case M_alwaysShowSelection:
        if (penv->CheckExtensions())
        {
            ptf->SetAlwaysShowSelection(val.ToBool(GetASEnvironment()));
            return 1;
        }
        break;

    case M_selectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetActiveSelectionTextColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_selectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetActiveSelectionBackgroundColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_inactiveSelectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetInactiveSelectionTextColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_inactiveSelectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            ptf->GetEditorKit()->SetInactiveSelectionBackgroundColor(val.ToUInt32(penv));
            return true;
        }
        break;
    case M_noAutoSelection:
        if (penv->CheckExtensions())
        {
            ptf->SetNoAutoSelection(val.ToBool(penv));
            return true;
        }
        break;
    case M_disableIME:
        if (penv->CheckExtensions())
        {
            ptf->SetIMEDisabledFlag(val.ToBool(penv));
            return true;
        }
    case M_filters:
        {
#ifdef GFX_AS2_ENABLE_FILTERS
            Object* pobj = val.ToObject(penv);
            if (pobj && pobj->InstanceOf(penv, penv->GetPrototype(ASBuiltin_Array)))
            {
                TextFilter textfilter;
                bool filterdirty = false;
                ArrayObject* parrobj = static_cast<ArrayObject*>(pobj);
                for (int i=0; i < parrobj->GetSize(); ++i)
                {
                    Value* arrval = parrobj->GetElementPtr(i);
                    if (arrval)
                    {
                        Object* pavobj = arrval->ToObject(penv);
                        if (pavobj && pavobj->InstanceOf(penv, penv->GetPrototype(ASBuiltin_BitmapFilter)))
                        {
                            BitmapFilterObject* pfilterobj = static_cast<BitmapFilterObject*>(pavobj);
                            textfilter.LoadFilterDesc(pfilterobj->GetFilter());
                            filterdirty = true;
                        }
                    }
                }
                if (filterdirty)
                {
                    ptf->SetFilters(textfilter);                                    
                    SetDirtyFlag();
                }
            }
#endif  // GFX_AS2_ENABLE_FILTERS
            return true;
        }
        break;
    default:
        break;
    }
    return AvmInteractiveObj::SetMember(penv, name, val, flags);
}

bool AvmTextField::GetMember(Environment* penv, const ASString& name, Value* pval)
{
    StandardMember member = GetStandardMemberConstant(name);
    TextField* ptf          = GetTextField();
    SF_ASSERT(ptf);

    // Handle TextField-specific "standard" members.
    // We put this before GetStandardMember so that we can handle _width
    // and _height in a custom way.
    switch(member)
    {
    case M_html:
        {               
            pval->SetBool(ptf->IsHtml());
            return true;
        }

    case M_htmlText:
        {
            pval->SetString(ptf->GetText(true));
            return true;
        }

    case M_text:
        {                   
            pval->SetString(ptf->GetText(false));
            return true;
        }

    case M_textColor:
        {
            // Return color in 0xRRGGBB format
            pval->SetUInt(ptf->GetTextColor32());
            return true;
        }

    case M_textWidth:           
        {
            // Return the width, in pixels, of the text as laid out.
            // (I.E. the actual text content, not our defined
            // bounding box.)
            //
            // In local coords.  Verified against Macromedia Flash.
            pval->SetNumber((Number)ptf->GetTextWidth());
            return true;
        }

    case M_textHeight:              
        {
            // Return the height, in pixels, of the text as laid out.
            // (I.E. the actual text content, not our defined
            // bounding box.)
            //
            // In local coords.  Verified against Macromedia Flash.
            pval->SetNumber((Number)ptf->GetTextHeight());
            return true;
        }

    case M_length:
        {
            pval->SetNumber((Number)ptf->GetTextLength());
            return true;
        }

    case M_autoSize:
        {
            if (!ptf->IsAutoSize())
                pval->SetString(penv->CreateConstString("none"));
            else
            {
                switch(ptf->GetAlignment())
                {
                case Text::DocView::Align_Left:
                    pval->SetString(penv->CreateConstString("left")); 
                    break;
                case Text::DocView::Align_Right:
                    pval->SetString(penv->CreateConstString("right")); 
                    break;
                case Text::DocView::Align_Center:
                    pval->SetString(penv->CreateConstString("center")); 
                    break;
                default: pval->SetUndefined();
                }
            }
            return true;
        }
    case M_wordWrap:
        {
            pval->SetBool(ptf->IsWordWrap());
            return true;
        }
    case M_multiline:
        {
            pval->SetBool(ptf->IsMultiline());
            return true;
        }
    case M_border:
        {
            pval->SetBool(ptf->HasBorder());
            return true;
        }
    case M_variable:
        {
            if (!VariableName.IsEmpty())
                pval->SetString(VariableName);
            else
                pval->SetNull();
            return true;
        }
    case M_selectable:
        {
            pval->SetBool(ptf->IsSelectable());
            return true;
        }
    case M_embedFonts:
        {
            pval->SetBool(!ptf->DoesUseDeviceFont());
            return true;
        }
    case M_password:
        {
            pval->SetBool(ptf->IsPassword());
            return true;
        }

    case M_shadowStyle:
        {
            if (!penv->CheckExtensions())
                break;

            pval->SetString(ptf->GetShadowStyle());
            return true;
        }
    case M_shadowColor:
        if (penv->CheckExtensions())
        {
            pval->SetInt(ptf->GetShadowColor32());
            return true;
        }
        break;

    case M_hscroll:
        {
            pval->SetNumber((Number)ptf->GetHScrollOffset());    
            return 1;
        }
    case M_scroll:
        {
            pval->SetNumber((Number)(ptf->GetVScrollOffset() + 1));    
            return 1;
        }
    case M_maxscroll:
        {
            pval->SetNumber((Number)(ptf->GetMaxVScroll() + 1));    
            return 1;
        }
    case M_maxhscroll:
        {
            pval->SetNumber((Number)(ptf->GetMaxHScroll()));    
            return 1;
        }
    case M_background:
        {
            pval->SetBool(ptf->HasBackground());
            return 1;
        }
    case M_backgroundColor:
        {
            pval->SetNumber(Number(ptf->GetBackgroundColor24()));
            return 1;
        }
    case M_borderColor:
        {
            pval->SetNumber(Number(ptf->GetBorderColor24()));
            return 1;
        }
    case M_bottomScroll:
        {
            pval->SetNumber((Number)(ptf->GetBottomVScroll() + 1));
            return 1;
        }
    case M_type:
        {
            pval->SetString(ptf->IsReadOnly() ? 
                penv->CreateConstString("dynamic") : penv->CreateConstString("input")); 
            return 1;
        }
    case M_maxChars:
        {
            if (!ptf->HasMaxLength())
                pval->SetNull();
            else
                pval->SetNumber(Number(ptf->GetMaxLength()));
            return 1;
        }
    case M_condenseWhite:
        {
            pval->SetBool(ptf->IsCondenseWhite());
            return 1;
        }

    case M_antiAliasType:
        {
            pval->SetString(ptf->IsAAForReadability() ? 
                penv->CreateConstString("advanced") : penv->CreateConstString("normal"));
            return 1;
        }
    case M_mouseWheelEnabled:
        {
            pval->SetBool(ptf->IsMouseWheelEnabled());
            return true;
        }
    case M_styleSheet:
        {
            pval->SetUndefined();
#ifdef GFX_ENABLE_CSS
            if (pCSSData && pCSSData->pASStyleSheet.GetPtr())
                pval->SetAsObject(pCSSData->pASStyleSheet);
#endif //GFX_ENABLE_CSS
            return true;
        }

        // extension
    case M_hitTestDisable:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(ptf->IsHitTestDisableFlagSet());
            return 1;
        }
        break;

    case M_noTranslate:
        if (GetASEnvironment()->CheckExtensions())
        {
            pval->SetBool(ptf->IsNoTranslate());
            return 1;
        }
        break;

    case M_caretIndex:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetCursorPos()));
            return 1;
        }
        break;
    case M_selectionBeginIndex:
        if (penv->CheckExtensions())
        {
            if (ptf->IsSelectable() && ptf->HasEditorKit())
                pval->SetNumber(Number(ptf->GetBeginIndex()));
            else
                pval->SetNumber(-1);
            return 1;
        }
        break;
    case M_selectionEndIndex:
        if (penv->CheckExtensions())
        {
            if (ptf->IsSelectable() && ptf->HasEditorKit())
                pval->SetNumber(Number(ptf->GetEndIndex()));
            else
                pval->SetNumber(-1);
            return 1;
        }
        break;
    case M_autoFit:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsAutoFit());
            return true;
        }
        break;

    case M_blurX:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetBlurX()));
            return true;
        }
        break;

    case M_blurY:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetBlurY()));
            return true;
        }
        break;

    case M_blurStrength:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetBlurStrength()));
            return true;
        }
        break;

    case M_outline:
        // Not implemented
        //if (penv->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_fauxBold:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->GetFauxBold());
            return true;
        }
        break;

    case M_fauxItalic:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->GetFauxItalic());
            return true;
        }
        break;


    case M_shadowAlpha:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowAlpha()));
            return true;
        }
        break;

    case M_shadowAngle:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowAngle()));
            return true;
        }
        break;

    case M_shadowBlurX:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowBlurX()));
            return true;
        }
        break;

    case M_shadowBlurY:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowBlurY()));
            return true;
        }
        break;

    case M_shadowDistance:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowDistance()));
            return true;
        }
        break;

    case M_shadowHideObject:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsShadowHiddenObject());
            return true;
        }
        break;

    case M_shadowKnockOut:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsShadowKnockOut());
            return true;
        }
        break;

    case M_shadowQuality:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowQuality()));
            return true;
        }
        break;

    case M_shadowStrength:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetShadowStrength()));
            return true;
        }
        break;

    case M_shadowOutline:
        // Not implemented
        //if (penv->CheckExtensions())
        //{
        //    return true;
        //}
        break;

    case M_numLines:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(Number(ptf->GetLinesCount()));
            return true;
        }
        break;

    case M_verticalAutoSize:
        if (penv->CheckExtensions())
        {
            if (!ptf->IsAutoSizeY())
                pval->SetString(penv->CreateConstString("none"));
            else
            {
                switch(ptf->GetVAlignment())
                {
                case Text::DocView::VAlign_None:
                    pval->SetString(penv->CreateConstString("none")); 
                    break;
                case Text::DocView::VAlign_Top:
                    pval->SetString(penv->CreateConstString("top")); 
                    break;
                case Text::DocView::VAlign_Bottom:
                    pval->SetString(penv->CreateConstString("bottom")); 
                    break;
                case Text::DocView::VAlign_Center:
                    pval->SetString(penv->CreateConstString("center")); 
                    break;
                default: pval->SetUndefined();
                }
            }
            return true;
        }
        break;

    case M_fontScaleFactor:
        if (penv->CheckExtensions())
        {
            pval->SetNumber(ptf->GetFontScaleFactor());
        }
        break;

    case M_verticalAlign:
        if (penv->CheckExtensions())
        {
            switch(ptf->GetVAlignment())
            {
            case Text::DocView::VAlign_None:
                pval->SetString(penv->CreateConstString("none")); 
                break;
            case Text::DocView::VAlign_Top:
                pval->SetString(penv->CreateConstString("top")); 
                break;
            case Text::DocView::VAlign_Bottom:
                pval->SetString(penv->CreateConstString("bottom")); 
                break;
            case Text::DocView::VAlign_Center:
                pval->SetString(penv->CreateConstString("center")); 
                break;
            default: pval->SetUndefined();
            }
            return true;
        }
        break;

    case M_textAutoSize:
        if (penv->CheckExtensions())
        {
            switch(ptf->GetTextAutoSize())
            {
            case Text::DocView::TAS_Shrink:
                pval->SetString(penv->CreateConstString("shrink")); 
                break;
            case Text::DocView::TAS_Fit:
                pval->SetString(penv->CreateConstString("fit")); 
                break;
            default:
                pval->SetString(penv->CreateConstString("none")); 
                break;
            }
        }
        break;

    case M_useRichTextClipboard:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->DoesUseRichClipboard());
        }
        break;

    case M_alwaysShowSelection:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->DoesAlwaysShowSelection());
        }
        break;

    case M_selectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetActiveSelectionTextColor()));
            return true;
        }
        break;
    case M_selectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetActiveSelectionBackgroundColor()));
            return true;
        }
        break;
    case M_inactiveSelectionTextColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetInactiveSelectionTextColor()));
            return true;
        }
        break;
    case M_inactiveSelectionBkgColor:
        if (penv->CheckExtensions() && ptf->HasEditorKit())
        {
            pval->SetNumber(Number(ptf->GetEditorKit()->GetInactiveSelectionBackgroundColor()));
            return true;
        }
        break;
    case M_noAutoSelection:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsNoAutoSelection());
            return true;
        }
        break;
    case M_disableIME:
        if (penv->CheckExtensions())
        {
            pval->SetBool(ptf->IsIMEDisabledFlag());
            return true;
        }
        break;
    case M_filters:
        {
            pval->SetUndefined();
#ifdef GFX_AS2_ENABLE_FILTERS
            // Not implemented because TextFilter does not distinguish
            // between drop shadow and glow filters
            Log* plog = penv->GetLog();
            if (plog)
                plog->LogWarning("Retrieval of the TextField.filters property is not implemented.");
#endif  // GFX_AS2_ENABLE_FILTERS
            return true;
        }

    default: 
        if (GetStandardMember(member, pval, 0))
            return true;
        break;
    }

    if (ASTextFieldObj)
    {
        return ASTextFieldObj->GetMember(penv, name, pval);
    }
    else 
    {
        // Now we can search in the __proto__
        Object* proto = Get__proto__();
        if (proto)
        {
            // ASMovieClipObj is not created yet; use __proto__ instead
            if (proto->GetMember(penv, name, pval))    
            {
                return true;
            }
        }
    }
    // looks like _global is accessable from any character
    if (name == "_global" && penv)
    {
        pval->SetAsObject(penv->GetGC()->pGlobal);
        return true;
    }
    return false;
}
#endif

bool    AvmTextField::IsTabable() const
{
    SF_ASSERT(pDispObj);

    if (!pDispObj->GetVisible()) 
        return false;
    if (GetIntObj()->IsTabEnabledFlagDefined())
        return GetIntObj()->IsTabEnabledFlagTrue();
    else if (GetIntObj()->GetTabIndex() > 0)
        return true;
    return !GetTextField()->IsReadOnly();
}

void AvmTextField::NotifyChanged()
{
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_change);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj && as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
}

void AvmTextField::OnScroll()
{
    const ASString& evtName = GetAS3Root()->GetBuiltinsMgr().GetBuiltin(AS3Builtin_scroll);
    Instances::DisplayObject* as3obj = GetAS3Obj();
    if (as3obj && as3obj->HasEventHandler(evtName, false))
    {
        SPtr<Instances::Event> evt = as3obj->CreateEventObject(evtName, true, false);
        SF_ASSERT(evt);
        evt->Target = as3obj;
        as3obj->Dispatch(evt, GetDispObj());
    }
}

#if 0
#ifdef GFX_AS2_ENABLE_BITMAPDATA
void AvmTextField::ProceedImageSubstitution(const GASFnCall& fn, int idx, const Value* pve)
{
    if (pve && pve->IsObject())
    {
        TextField* ptextField = GetTextField();
        Object* peobj = pve->ToObject(fn.Env);
        SF_ASSERT(peobj);
        Value val;
        Text::DocView::ImageSubstitutor* pimgSubst = ptextField->CreateImageSubstitutor();
        if (!pimgSubst)
            return;
        Text::DocView::ImageSubstitutor::Element isElem;
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "subString", &val))
        {
            ASString str = val.ToString(fn.Env);
            UPInt wstrLen = str.GetLength();

            if (wstrLen > 15)
            {
                ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - length of subString should not exceed 15 characters",
                    GetName().ToCStr(), idx);
                return;
            }
            UTF8Util::DecodeString(isElem.SubString, str.ToCStr(), str.GetSize() + 1);
            isElem.SubStringLen = (UByte)wstrLen;
        }
        else
        {
            // subString is mandatory!
            ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - subString should be specified",
                GetName().ToCStr(), idx);
            return;
        }
        Ptr<ShapeWithStyles> pimgShape;
        float screenWidth = 0, screenHeight = 0;
        float origWidth = 0, origHeight = 0;
        float baseLineX = 0, baseLineY = 0;
        const char* idStr = NULL;
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "image", &val))
        {
            Object* piobj = val.ToObject(fn.Env);
            if (piobj && piobj->GetObjectType() == Object_BitmapData)
            {
                GASBitmapData* pbmpData = static_cast<GASBitmapData*>(piobj);
                ImageResource* pimgRes = pbmpData->GetImage();
                pimgShape = *SF_HEAP_NEW(fn.Env->GetHeap()) ShapeWithStyles();
                pimgShape->SetToImage(pimgRes, true);

                Render::Rect<int> dimr = pimgRes->GetImageInfo()->GetRect();
                screenWidth  = origWidth  = (float)PixelsToTwips(dimr.Width());
                screenHeight = origHeight = (float)PixelsToTwips(dimr.Height());
                if (origWidth == 0 || origHeight == 0)
                {
                    ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - image has one zero dimension",
                        GetName().ToCStr(), idx);
                    return;
                }
            }
        }
        if (!pimgShape)
        {
            ptextField->LogScriptWarning("%s.setImageSubstitutions() failed for #%d element - 'image' is not specified or not a BitmapData",
                GetName().ToCStr(), idx);
            return;
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "width", &val))
        {
            GASNumber v = val.ToNumber(fn.Env);
            screenWidth = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "height", &val))
        {
            GASNumber v = val.ToNumber(fn.Env);
            screenHeight = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "baseLineX", &val))
        {
            GASNumber v = val.ToNumber(fn.Env);
            baseLineX = (float)PixelsToTwips(v);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "baseLineY", &val))
        {
            GASNumber v = val.ToNumber(fn.Env);
            baseLineY = (float)PixelsToTwips(v);
        }
        else
        {
            // if baseLineY is not specified, then use (origHeight - 1) pts instead
            baseLineY = origHeight - PixelsToTwips(1.0f);
        }
        if (peobj->GetConstMemberRaw (fn.Env->GetSC(), "id", &val))
        {
            idStr = val.ToString(fn.Env).ToCStr();
        }

        isElem.pImageDesc = *SF_HEAP_NEW(fn.Env->GetHeap()) Text::ImageDesc;
        SF_ASSERT(isElem.pImageDesc);
        isElem.pImageDesc->pImageShape  = pimgShape;
        isElem.pImageDesc->BaseLineX    = (int)baseLineX;
        isElem.pImageDesc->BaseLineY    = (int)baseLineY;
        isElem.pImageDesc->ScreenWidth  = (unsigned)screenWidth;
        isElem.pImageDesc->ScreenHeight = (unsigned)screenHeight;
        if (idStr)
            ptextField->AddIdImageDescAssoc(idStr, isElem.pImageDesc);
        isElem.pImageDesc->Matrix.AppendTranslation(-baseLineX, -baseLineY);
        isElem.pImageDesc->Matrix.AppendScaling(screenWidth/origWidth, screenHeight/origHeight);

        pimgSubst->AddImageDesc(isElem);
        ptextField->ForceCompleteReformat();
        SetDirtyFlag();
    }
}


void AvmTextField::SetImageSubstitutions(const GASFnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        if (fn.NArgs >= 1)
        {
            if (fn.Arg(0).IsNull())
            {
                // clear all substitutions
                pthis->ClearIdImageDescAssoc();
                pthis->ClearImageSubstitutor();
                pthis->ForceCompleteReformat();
            }
            else
            {
                Object* pobj = fn.Arg(0).ToObject(fn.Env);
                if (pobj)
                {
                    // if array is specified as a parameter, proceed it; otherwise
                    // if an object is specified - proceed it as single element.
                    if (pobj->GetObjectType() == Object_Array)
                    {
                        GASArrayObject* parr = static_cast<GASArrayObject*>(pobj);
                        for (int i = 0, n = parr->GetSize(); i < n; ++i)
                        {
                            Value* pve = parr->GetElementPtr(i);
                            ToAvmTextField(pthis)->ProceedImageSubstitution(fn, i, pve);
                        }
                    }
                    else
                    {
                        const Value& ve = fn.Arg(0);
                        ToAvmTextField(pthis)->ProceedImageSubstitution(fn, 0, &ve);
                    }
                }
                else
                {
                    pthis->LogScriptWarning("%s.setImageSubstitutions() failed: parameter should be either 'null', object or array",
                        pthis->GetName().ToCStr());
                }
            }
        }
    }
}

void AvmTextField::UpdateImageSubstitution(const GASFnCall& fn)
{
    fn.Result->SetBool(false);
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        if (fn.NArgs >= 1)
        {
            ASString idStr = fn.Arg(0).ToString(fn.Env);
            StringHashLH<Ptr<Text::ImageDesc> >* pimageDescAssoc = pthis->GetImageDescAssoc();
            if (pimageDescAssoc)
            {
                Ptr<Text::ImageDesc>* ppimgDesc = pimageDescAssoc->Get(idStr.ToCStr());
                if (ppimgDesc)
                {
                    Text::ImageDesc* pimageDesc = ppimgDesc->GetPtr();
                    SF_ASSERT(pimageDesc);
                    if (fn.NArgs >= 2)
                    {
                        if (fn.Arg(1).IsNull() || fn.Arg(1).IsUndefined())
                        {
                            // if null or undefined - remove the substitution and reformat
                            Text::DocView::ImageSubstitutor* pimgSubst = pthis->CreateImageSubstitutor();
                            if (pimgSubst)
                            {
                                pimgSubst->RemoveImageDesc(pimageDesc);
                                pthis->ForceCompleteReformat();
                                pthis->RemoveIdImageDescAssoc(idStr.ToCStr());
                                pthis->SetDirtyFlag();
                            }
                        }
                        else
                        {
                            Object* piobj = fn.Arg(1).ToObject(fn.Env);
                            if (piobj && piobj->GetObjectType() == Object_BitmapData)
                            {
                                GASBitmapData* pbmpData = static_cast<GASBitmapData*>(piobj);
                                ImageResource* pimgRes = pbmpData->GetImage();
                                Ptr<ShapeWithStyles> pimgShape = *SF_HEAP_NEW(fn.Env->GetHeap()) ShapeWithStyles();
                                pimgShape->SetToImage(pimgRes, true);

                                pimageDesc->pImageShape = pimgShape;
                                pthis->SetDirtyFlag();
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif //#ifdef GFX_AS2_ENABLE_BITMAPDATA

void AvmTextField::SetTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField*    pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs == 1)
        {
            // my_textField.setTextFormat(textFormat:TextFormat)
            // Applies the properties of textFormat to all text in the text field.
            Object* pobjVal = fn.Arg(0).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                pthis->SetTextFormat(ptextFormatObj->TextFormat);
                pthis->SetParagraphFormat(ptextFormatObj->ParagraphFormat);
                pthis->SetDirtyFlag();
            }
        }
        else if (fn.NArgs == 2)
        {
            // my_textField.setTextFormat(beginIndex:Number, textFormat:TextFormat)
            // Applies the properties of textFormat to the character at the beginIndex position.
            Object* pobjVal = fn.Arg(1).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                Number pos = fn.Arg(0).ToNumber(fn.Env);
                if (pos >= 0)
                {
                    unsigned upos = unsigned(pos);
                    pthis->SetTextFormat(ptextFormatObj->TextFormat, upos, upos + 1);
                    pthis->SetParagraphFormat(ptextFormatObj->ParagraphFormat, upos, upos + 1);
                    pthis->SetDirtyFlag();
                }
            }
        }
        else if (fn.NArgs >= 3)
        {
            // my_textField.setTextFormat(beginIndex:Number, endIndex:Number, textFormat:TextFormat)
            // Applies the properties of the textFormat parameter to the span of text from the 
            // beginIndex position to the endIndex position.
            Object* pobjVal = fn.Arg(2).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                Number beginIndex = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env));
                Number endIndex   = Alg::Max((Number)0, fn.Arg(1).ToNumber(fn.Env));
                if (beginIndex <= endIndex)
                {
                    TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);
                    pthis->SetTextFormat(ptextFormatObj->TextFormat, (unsigned)beginIndex, (unsigned)endIndex);
                    pthis->SetParagraphFormat(ptextFormatObj->ParagraphFormat, (unsigned)beginIndex, (unsigned)endIndex);
                    pthis->SetDirtyFlag();
                }
            }
        }
    }
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.setTextFormat is not supported - need GFX_AS2_ENABLE_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::GetTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        UPInt beginIndex = 0, endIndex = SF_MAX_UPINT;
        if (fn.NArgs >= 1)
        {
            // beginIndex
            Number v = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env));
            if (v >= 0)
                beginIndex = (unsigned)v;
        }
        if (fn.NArgs >= 2)
        {
            // endIndex
            Number v = Alg::Max((Number)0, fn.Arg(1).ToNumber(fn.Env));
            if (v >= 0)
                endIndex = (unsigned)v;
        }
        else if (fn.NArgs >= 1)
        {
            Number v = Alg::Max((Number)0, fn.Arg(0).ToNumber(fn.Env) + 1);
            if (v >= 0)
                endIndex = (unsigned)v;
        }
        if (beginIndex <= endIndex)
        {
            TextFormat textFmt(fn.Env->GetHeap());
            ParagraphFormat paraFmt;
            pthis->GetTextAndParagraphFormat(&textFmt, &paraFmt, beginIndex, endIndex);

            Ptr<TextFormatObject> pasTextFmt = *SF_HEAP_NEW(fn.Env->GetHeap()) TextFormatObject(fn.Env);
            pasTextFmt->SetTextFormat(fn.Env->GetSC(), textFmt);
            pasTextFmt->SetParagraphFormat(fn.Env->GetSC(), paraFmt);
            fn.Result->SetAsObject(pasTextFmt);
        }
        else
            fn.Result->SetUndefined();
    }
    else
        fn.Result->SetUndefined();
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.getTextFormat is not supported - need GFX_AS2_ENABLE_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::GetNewTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
//        AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        const TextFormat* ptextFmt = pthis->GetDefaultTextFormat();
        const ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();

        Ptr<TextFormatObject> pasTextFmt = *SF_HEAP_NEW(fn.Env->GetHeap()) TextFormatObject(fn.Env);
        if (ptextFmt)
            pasTextFmt->SetTextFormat(fn.Env->GetSC(), *ptextFmt);
        if (pparaFmt)
            pasTextFmt->SetParagraphFormat(fn.Env->GetSC(), *pparaFmt);
        fn.Result->SetAsObject(pasTextFmt);
    }
    else
        fn.Result->SetUndefined();
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.getNewTextFormat is not supported - need GFX_AS2_ENABLE_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::SetNewTextFormat(const FnCall& fn)
{
#ifdef GFX_AS2_ENABLE_TEXTFORMAT
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            Object* pobjVal = fn.Arg(0).ToObject(fn.Env);
            if (pobjVal && pobjVal->GetObjectType() == Object_TextFormat)
            {
                TextFormatObject* ptextFormatObj = static_cast<TextFormatObject*>(pobjVal);

                const TextFormat* ptextFmt = pthis->GetDefaultTextFormat();
                const ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();
                pthis->SetDefaultTextFormat(ptextFmt->Merge(ptextFormatObj->TextFormat));
                pthis->SetDefaultParagraphFormat(pparaFmt->Merge(ptextFormatObj->ParagraphFormat));
            }
        }
    }
#else
    SF_UNUSED(fn);
    SF_DEBUG_WARNING(1, "TextField.setNewTextFormat is not supported - need GFX_AS2_ENABLE_TEXTFORMAT macro.");
#endif //#ifdef GFX_AS2_ENABLE_TEXTFORMAT
}

void AvmTextField::ReplaceText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 3)
        {
            Number start = fn.Arg(0).ToNumber(fn.Env);
            Number end   = fn.Arg(1).ToNumber(fn.Env);
            ASString str   = fn.Arg(2).ToString(fn.Env);

            unsigned len = str.GetLength();
            unsigned startPos = unsigned(start);
            unsigned endPos   = unsigned(end);
            if (start >= 0 && end >= 0 && startPos <= endPos)
            {
                // replaceText does *NOT* use default text format, if replaced text
                // is located in the middle of the text; however, it uses the default
                // format, if the replacing text is at the very end of the text. This
                // is so, because Flash doesn't know what is the format at the cursor pos
                // if the cursor is behind the last char (caretIndex == length).
                // Our implementation knows always about formats, thus, we need artificially
                // replicate this behavior. Though, it still works differently from Flash,
                // since that default text format will be combined with current text format
                // at the cursor position. Not sure, we should worry about this.
                const TextFormat* ptextFmt;
                const ParagraphFormat* pparaFmt;
                UPInt prevLen = pthis->GetTextLength();
                UPInt newLen  = prevLen - (endPos - startPos) + len;
                if (startPos >= prevLen)
                {
                    // use default text format
                    ptextFmt = pthis->GetDefaultTextFormat();
                    pparaFmt = pthis->GetDefaultParagraphFormat();
                }
                else
                    pthis->GetTextAndParagraphFormat(&ptextFmt, &pparaFmt, startPos);

                wchar_t buf[1024];
                if (len < sizeof(buf)/sizeof(buf[0]))
                {
                    UTF8Util::DecodeString(buf, str.ToCStr());
                    pthis->ReplaceText(buf, startPos, endPos);
                }
                else
                {
                    wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
                    UTF8Util::DecodeString(pbuf, str.ToCStr());
                    pthis->ReplaceText(pbuf, startPos, endPos);
                    SF_FREE(pbuf);
                }
                if (pthis->HasEditorKit())
                {
                    // Replace text does not change the cursor position, unless
                    // this position doesn't exist anymore.

                    if (pthis->GetEditorKit()->GetCursorPos() > newLen)
                        pthis->GetEditorKit()->SetCursorPos(newLen, false);
                }
                if (pparaFmt)
                    pthis->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
                if (ptextFmt)
                {
                    pthis->SetTextFormat(*ptextFmt, startPos, startPos + len);
                }
                pthis->SetDirtyFlag();
            }
        }
    }
}

void AvmTextField::ReplaceSel(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1 && pthis->HasEditorKit())
        {
            ASString str = fn.Arg(0).ToString(fn.Env);

            const TextFormat* ptextFmt = pthis->GetDefaultTextFormat();
            const ParagraphFormat* pparaFmt = pthis->GetDefaultParagraphFormat();

            unsigned len = str.GetLength();
            UPInt startPos = pthis->GetEditorKit()->GetBeginSelection();
            UPInt endPos   = pthis->GetEditorKit()->GetEndSelection();
            wchar_t buf[1024];
            if (len < sizeof(buf)/sizeof(buf[0]))
            {
                UTF8Util::DecodeString(buf, str.ToCStr());
                pthis->ReplaceText(buf, startPos, endPos);
            }
            else
            {
                wchar_t* pbuf = (wchar_t*)SF_ALLOC((len + 1) * sizeof(wchar_t), StatMV_Text_Mem);
                UTF8Util::DecodeString(pbuf, str.ToCStr());
                pthis->ReplaceText(pbuf, startPos, endPos);
                SF_FREE(pbuf);
            }
            pthis->GetEditorKit()->SetCursorPos(startPos + len, false);
            if (pparaFmt)
                pthis->SetParagraphFormat(*pparaFmt, startPos, startPos + len);
            if (ptextFmt)
                pthis->SetTextFormat(*ptextFmt, startPos, startPos + len);
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::RemoveTextField(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->GetDepth() < 16384)
        {
            pthis->LogScriptWarning("%s.removeMovieClip() failed - depth must be >= 0",
                pthis->GetName().ToCStr());
            return;
        }
        pthis->RemoveDisplayObject();
    }
}

#ifndef SF_NO_TEXTFIELD_EXTENSIONS
void AvmTextField::AppendText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            ASString str = fn.Arg(0).ToString(fn.Env);
            pthis->AppendText(str.ToCStr());
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::AppendHtml(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasStyleSheet()) // doesn't work if style sheet is set
            return;
        if (fn.NArgs >= 1)
        {
            ASString str = fn.Arg(0).ToString(fn.Env);
            StyledText::HTMLImageTagInfoArray imageInfoArray(Memory::GetHeapByAddress(pthis));
            pthis->AppendHtml(str.ToCStr(), SF_MAX_UPINT, false, &imageInfoArray);
            if (imageInfoArray.GetSize() > 0)
            {
                pthis->ProcessImageTags(imageInfoArray);
            }
            pthis->SetDirtyFlag();
        }
    }
}

void AvmTextField::GetCharBoundaries(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            unsigned charIndex = (unsigned)fn.Arg(0).ToNumber(fn.Env);
            RectF charBounds;
            charBounds.Clear();
            if (pthis->GetCharBoundaries(&charBounds, charIndex))
            {

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);

                ASRect gasRect;
                gasRect.Left    = TwipsToPixels(Number(charBounds.x1));
                gasRect.Top     = TwipsToPixels(Number(charBounds.y1));
                gasRect.Right   = TwipsToPixels(Number(charBounds.x2));
                gasRect.Bottom  = TwipsToPixels(Number(charBounds.y2));

                prect->SetProperties(fn.Env, gasRect);

#else
                Ptr<Object> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

                ASStringContext *psc = fn.Env->GetSC();
                prect->SetConstMemberRaw(psc, "x", TwipsToPixels(Number(charBounds.x1)));
                prect->SetConstMemberRaw(psc, "y", TwipsToPixels(Number(charBounds.y1)));
                prect->SetConstMemberRaw(psc, "width", TwipsToPixels(Number(charBounds.x2)));
                prect->SetConstMemberRaw(psc, "height", TwipsToPixels(Number(charBounds.y2)));
#endif
                fn.Result->SetAsObject(prect.GetPtr());
            }
            else
                fn.Result->SetNull();
        }
    }
}

void AvmTextField::GetExactCharBoundaries(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            unsigned charIndex = (unsigned)fn.Arg(0).ToNumber(fn.Env);
            RectF charBounds;
            charBounds.Clear();
            if (pthis->GetExactCharBoundaries(&charBounds, charIndex))
            {

#ifdef GFX_AS2_ENABLE_RECTANGLE
                Ptr<RectangleObject> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) RectangleObject(fn.Env);

                ASRect gasRect;
                gasRect.Left    = TwipsToPixels(Number(charBounds.x1));
                gasRect.Top     = TwipsToPixels(Number(charBounds.y1));
                gasRect.Right   = TwipsToPixels(Number(charBounds.x2));
                gasRect.Bottom  = TwipsToPixels(Number(charBounds.y2));
                prect->SetProperties(fn.Env, gasRect);

#else
                Ptr<Object> prect = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);

                ASStringContext *psc = fn.Env->GetSC();
                prect->SetConstMemberRaw(psc, "x", TwipsToPixels(Number(charBounds.x1)));
                prect->SetConstMemberRaw(psc, "y", TwipsToPixels(Number(charBounds.y1)));
                prect->SetConstMemberRaw(psc, "width", TwipsToPixels(Number(charBounds.x2)));
                prect->SetConstMemberRaw(psc, "height", TwipsToPixels(Number(charBounds.y2)));
#endif
                fn.Result->SetAsObject(prect.GetPtr());
            }
            else
                fn.Result->SetNull();
        }
    }
}

void AvmTextField::GetCharIndexAtPoint(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 2)
        {
            Number x = fn.Arg(0).ToNumber(fn.Env);
            Number y = fn.Arg(1).ToNumber(fn.Env);
            UPInt pos = pthis->GetCharIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
            if (!Alg::IsMax(pos))
                fn.Result->SetNumber(Number(pos));
            else
                fn.Result->SetNumber(-1);
        }
    }
}

void AvmTextField::GetLineIndexAtPoint(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 2)
        {
            Number x = fn.Arg(0).ToNumber(fn.Env);
            Number y = fn.Arg(1).ToNumber(fn.Env);
            unsigned pos = pthis->GetLineIndexAtPoint(float(PixelsToTwips(x)), float(PixelsToTwips(y)));
            if (!Alg::IsMax(pos))
                fn.Result->SetNumber(Number(pos));
            else
                fn.Result->SetNumber(-1);
        }
    }
}

void AvmTextField::GetLineOffset(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt off = pthis->GetLineOffset(unsigned(lineIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineLength(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt len = pthis->GetLineLength(unsigned(lineIndex));
                if (!Alg::IsMax(len))
                    fn.Result->SetNumber(Number(len));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineMetrics(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetUndefined();
            else
            {
                Text::DocView::LineMetrics metrics;
                bool rv = pthis->GetLineMetrics(unsigned(lineIndex), &metrics);
                if (rv)
                {
                    Ptr<Object> pobj = *SF_HEAP_NEW(fn.Env->GetHeap()) Object(fn.Env);
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "ascent", Value(TwipsToPixels((Number)metrics.Ascent)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "descent", Value(TwipsToPixels((Number)metrics.Descent)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "width", Value(TwipsToPixels((Number)metrics.Width)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "height", Value(TwipsToPixels((Number)metrics.Height)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "leading", Value(TwipsToPixels((Number)metrics.Leading)));
                    pobj->SetConstMemberRaw(fn.Env->GetSC(), "x", Value(TwipsToPixels((Number)metrics.FirstCharXOff)));
                    fn.Result->SetAsObject(pobj);
                }
                else
                    fn.Result->SetUndefined();
            }
        }
    }
}

void AvmTextField::GetLineText(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int lineIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (lineIndex < 0)
                fn.Result->SetUndefined();
            else
            {
                UPInt len = 0;
                const wchar_t* ptext = pthis->GetLineText(unsigned(lineIndex), &len);
                if (ptext)
                {
                    String str;
                    str.AppendString(ptext, (SPInt)len);
                    ASString asstr = fn.Env->CreateString(str);
                    fn.Result->SetString(asstr);
                }
                else
                    fn.Result->SetString(fn.Env->CreateConstString(""));
            }
        }
    }
}


void AvmTextField::GetFirstCharInParagraph(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int charIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (charIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                UPInt off = pthis->GetFirstCharInParagraph(unsigned(charIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

void AvmTextField::GetLineIndexOfChar(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (fn.NArgs >= 1)
        {
            int charIndex = int(fn.Arg(0).ToNumber(fn.Env));
            if (charIndex < 0)
                fn.Result->SetNumber(-1);
            else
            {
                unsigned off = pthis->GetLineIndexOfChar(unsigned(charIndex));
                if (!Alg::IsMax(off))
                    fn.Result->SetNumber(Number(off));
                else
                    fn.Result->SetNumber(-1);
            }
        }
    }
}

// Extension method: copyToClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::CopyToClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->CopyToClipboard(startIndex, endIndex, richClipboard);
        }
    }
}

// Extension method: cutToClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::CutToClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->CutToClipboard(startIndex, endIndex, richClipboard);
            pthis->SetDirtyFlag();
        }
    }
}

// Extension method: pasteFromClipboard([richClipboard:Boolean], [startIndex:Number], [endIndex:Number])
// richClipboard by default is equal to "useRichTextClipboard";
// default values of startIndex and endIndex are equal to selectionBeginIndex/selectionEndIndex
void AvmTextField::PasteFromClipboard(const FnCall& fn)
{
    if (fn.ThisPtr && fn.ThisPtr->GetObjectType() == Object_TextField)
    {
        TextField* pthis = static_cast<TextField*>(fn.ThisPtr->ToCharacter());
        //AvmTextField* pthis = static_cast<AvmTextField*>(fn.ThisPtr);
        if (pthis->HasEditorKit())
        {
            Text::EditorKit* pedKit = pthis->GetEditorKit();
            bool richClipboard  = pedKit->DoesUseRichClipboard();
            unsigned startIndex     = (unsigned)pedKit->GetBeginSelection();
            unsigned endIndex       = (unsigned)pedKit->GetEndSelection();
            if (fn.NArgs >= 1)
                richClipboard = fn.Arg(0).ToBool(fn.Env);
            if (fn.NArgs >= 2)
                startIndex = fn.Arg(1).ToUInt32(fn.Env);
            if (fn.NArgs >= 3)
                endIndex = fn.Arg(2).ToUInt32(fn.Env);
            pedKit->PasteFromClipboard(startIndex, endIndex, richClipboard);
            pthis->SetDirtyFlag();
        }
    }
}
#endif //SF_NO_TEXTFIELD_EXTENSIONS
#endif //if 0

}}} // SF::GFx::AS3

