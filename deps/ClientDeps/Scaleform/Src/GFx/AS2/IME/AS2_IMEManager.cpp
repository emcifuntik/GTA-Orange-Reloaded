/**********************************************************************

Filename    :   IMEManager.cpp
Content     :   IME Manager base functinality
Created     :   Dec 17, 2007
Authors     :   Artyom Bolgar

Notes       :   
History     :   

Copyright   :   (c) 1998-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
#include "AS2/IME/AS2_IMEManager.h"

#include "GFx/GFx_DisplayObject.h"
#include "GFx/GFx_PlayerImpl.h"
#include "GFx/GFx_TextField.h"
#include "GFx/GFx_Sprite.h"
#include "AS2/AS2_Object.h"
#include "AS2/AS2_MovieClipLoader.h"
#include "AS2/AS2_Action.h"
#include "AS2/AS2_MovieRoot.h"
#include "GFx/GFx_Sprite.h"
#include "AS2/AS2_AvmSprite.h"
#include "AS2/IME/GASIme.h"

namespace Scaleform { namespace GFx { namespace AS2{

#ifndef SF_NO_IME_SUPPORT

// An implementation for IMEManager class.

class IMEManagerImpl : public NewOverrideBase<StatIME_Mem>
{
public:
    Movie*              pMovie;
    Ptr<TextField>		pTextField;
    SPInt               CursorPosition;
    String              CandidateSwfPath;
    String              CandidateSwfErrorMsg;
    bool                IMEDisabled;

    IMEManagerImpl() : pMovie(NULL), pTextField(NULL) 
    { 
        IMEDisabled = false; 
        CandidateSwfPath = "IME.swf";
    }

    void SetActiveMovie(Movie* pmovie)
    {
        pMovie = static_cast<Movie*>(pmovie);
    }

	static void OnBroadcastSetConversionMode(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

		AS2::GASIme::BroadcastOnSetConversionMode(fn.Env, fn.Arg(0).ToString(fn.Env));
    }

    static void OnBroadcastIMEConversion(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnIMEComposition(fn.Env, fn.Arg(0).ToString(fn.Env));
    }

    static void OnBroadcastSwitchLanguage(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnSwitchLanguage(fn.Env, fn.Arg(0).ToString(fn.Env));

    }

    static void OnBroadcastSetSupportedLanguages(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnSetSupportedLanguages(fn.Env, fn.Arg(0).ToString(fn.Env));

    }

    static void OnBroadcastSetCurrentInputLanguage(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnSetCurrentInputLang(fn.Env, fn.Arg(0).ToString(fn.Env));

    }

    static void OnBroadcastSetSupportedIMEs(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnSetSupportedIMEs(fn.Env, fn.Arg(0).ToString(fn.Env));
    }

    static void OnBroadcastSetIMEName(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnSetIMEName(fn.Env, fn.Arg(0).ToString(fn.Env));
    }

    static void OnBroadcastRemoveStatusWindow(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnRemoveStatusWindow(fn.Env);
    }

    static void OnBroadcastDisplayStatusWindow(const AS2::FnCall& fn)
    {
        if (!fn.Env)
            return;

        AS2::GASIme::BroadcastOnDisplayStatusWindow(fn.Env);
    }

    void BroadcastIMEConversion(const char* pString)
    {
        // We have to set up this queing since advance on the actionscript queue is called from a different
        // thread. 
        
		AS2::ValueArray params;

        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			GFx::ASString compString = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(compString);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
                IMEManagerImpl::OnBroadcastIMEConversion, &params);
        }
    }

    void BroadcastSwitchLanguage(const char* pString)
    {
        // We have to set up this queing since advance on the actionscript queue is called from a different
        // thread. 

        AS2::ValueArray params;
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString compString = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(compString);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
                IMEManagerImpl::OnBroadcastSwitchLanguage, &params);
        }
    }
    
    void BroadcastSetSupportedLanguages(const char* pString)
    {
        AS2::ValueArray params;

        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString supportedLanguages = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(supportedLanguages);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
                IMEManagerImpl::OnBroadcastSetSupportedLanguages, &params);
        }
    }

    void BroadcastSetSupportedIMEs(const char* pString)
    {
        AS2::ValueArray params;

        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString supportedIMEs = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(supportedIMEs);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),  
                IMEManagerImpl::OnBroadcastSetSupportedIMEs, &params);
        }
    }

    void BroadcastSetCurrentInputLanguage(const char* pString)
    {
        AS2::ValueArray params;
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString currentInputLanguage = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(currentInputLanguage);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
                IMEManagerImpl::OnBroadcastSetCurrentInputLanguage, &params);
        }   
    }

    void BroadcastSetIMEName(const char* pString)
    {
        AS2::ValueArray params;
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString imeName = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(imeName);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),
                IMEManagerImpl::OnBroadcastSetIMEName, &params);
        }   
    }

    void BroadcastSetConversionStatus(const char* pString)
    {
        AS2::ValueArray params;
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			ASString convStatus = pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment()->GetGC()->CreateString(pString);
			GFx::AS2::Value val; val.SetString(convStatus);
			params.PushBack(val);
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),
                IMEManagerImpl::OnBroadcastSetConversionMode, &params);
        }   
    }

    void BroadcastRemoveStatusWindow()
    {
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
            pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(), 
                IMEManagerImpl::OnBroadcastRemoveStatusWindow, 0);
        }
    }

    void BroadcastDisplayStatusWindow()
    {
        if (pMovie)
        {
			AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pMovie));
			pmovieRoot->InsertEmptyAction()->SetAction(pmovieRoot->GetLevel0Movie(),  
                IMEManagerImpl::OnBroadcastDisplayStatusWindow, 0);
        }
    }

    void Reset() { pTextField = NULL; pMovie = NULL; }

};

IMEManager::IMEManager(): IMEManagerBase()
{ 
    pImpl = new IMEManagerImpl;
    UnsupportedIMEWindowsFlag = GFxIME_ShowUnsupportedIMEWindows;
}

IMEManager::~IMEManager()
{
    delete pImpl;
}

void IMEManager::StartComposition()
{
    // we need to save the current position in text field
    if (pImpl->pMovie)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pImpl->pMovie))->GetFocusedCharacter(0);
		if (pfocusedCh && pfocusedCh->GetType() == AS2::ObjectInterface::Object_TextField)
        {
            Ptr<TextField> ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
            if (ptextFld->IsIMEEnabled())
            {
                pImpl->pTextField = ptextFld;

                // first of all, if we have an active selection - remove it
				UPInt begin = pImpl->pTextField->GetBeginIndex();
				UPInt end	= pImpl->pTextField->GetEndIndex();
                pImpl->pTextField->ReplaceText(L"", begin, end, end-begin); 
                                               

                pImpl->CursorPosition = begin;
                pImpl->pTextField->SetSelection(begin, begin);
                pImpl->pTextField->CreateCompositionString();
            }
        }
    }
}

void IMEManager::FinalizeComposition(const wchar_t* pstr, UPInt len)
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->CommitCompositionString(pstr, len);
        //pImpl->pTextField->SetCompositionStringHighlighting(true);
    }
    else
    {
        // a special case, used for Chinese New ChangJie, if typing in English:
        // in this case, no start composition event is sent and whole
        // text is being sent through ime_finalize message. If we don't
        // have pTextField at this time we need to find currently focused
        // one and use it.
        if (pImpl->pMovie)
        {
            Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pImpl->pMovie))->GetFocusedCharacter(0);
            if (pfocusedCh && pfocusedCh->GetType() == AS2::ObjectInterface::Object_TextField)
            {
                TextField* ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
                ptextFld->ReplaceText(pstr, ptextFld->GetBeginIndex(), 
                                      ptextFld->GetEndIndex(), len);

            }
        }
    }
}

void IMEManager::ClearComposition()
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->ClearCompositionString();
        //pImpl->pTextField->SetCompositionStringHighlighting(true);
    }
}

void IMEManager::ReleaseComposition()
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->ReleaseCompositionString();
        //pImpl->pTextField->SetCompositionStringHighlighting(true);
    }
}

void IMEManager::SetCompositionText(const wchar_t* pstr, UPInt len)
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->SetCompositionStringText(pstr, len);
    }
}

void IMEManager::SetCompositionPosition()
{
    if (pImpl->pTextField)
    {
        UPInt pos = pImpl->pTextField->GetCaretIndex();
        pImpl->pTextField->SetCompositionStringPosition(pos);
    }
}

void IMEManager::SetCursorInComposition(UPInt pos)
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->SetCursorInCompositionString(pos);
    }
}

void IMEManager::SetWideCursor(bool ow)
{
    if (pImpl->pTextField)
    {
        pImpl->pTextField->SetWideCursor(ow);
        //pImpl->pTextField->SetCompositionStringHighlighting(!ow);
    }
}

void IMEManager::OnOpenCandidateList()
{
    SF_ASSERT(pImpl->pMovie);
    if (!pImpl->pMovie)
        return;


//  On Win Vista, it's possible for the candidate list to be loaded up before STARTCOMPOSITION
//  message arrives (which sets the textfield pointer), hence if the textfield pointer is found to be
//  null, we should try to get it from the currently focussed object. This happens with Chinese Traditional
//  IME DaYi 6.0 and Array
//  SF_ASSERT(pImpl->pTextField.GetPtr());

    TextField* ptextFld = pImpl->pTextField;
    if (!ptextFld)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pImpl->pMovie))->GetFocusedCharacter(0);
            if (pfocusedCh && pfocusedCh->GetType() == AS2::ObjectInterface::Object_TextField)
            {
                ptextFld = static_cast<TextField*>(pfocusedCh.GetPtr());
            }
        if(!ptextFld)
            return;
    }

    FontResource* pfont = ptextFld->GetFontResource();
    if (!pfont)
        return;
	GFx::Value v;
    if (!pImpl->pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
        v.SetNumber(0);
    
	AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pImpl->pMovie));
	Sprite* imeMovie = pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL);
    if (!imeMovie || v.GetNumber() != 2)
        return;
    imeMovie->SetIMECandidateListFont(pfont);

}

void IMEManager::HighlightText(UPInt pos, UPInt len, TextHighlightStyle style, bool clause)
{
    SF_UNUSED2(style, clause);
    if (pImpl->pTextField)
    {
        /*if (clause)
        {
            // clause flag is set in order to highlight whole composition string
            // by solid single underline (used for Japanese).
            pImpl->pTextField->HighlightCompositionString(Text::IMEStyle::SC_ConvertedSegment);
        }
        else
            pImpl->pTextField->HighlightCompositionString(Text::IMEStyle::SC_CompositionSegment);
            */
        Text::IMEStyle::Category cat;
        switch (style)
        {
        case IMEManager::THS_CompositionSegment: cat = Text::IMEStyle::SC_CompositionSegment; break;
        case IMEManager::THS_ClauseSegment:      cat = Text::IMEStyle::SC_ClauseSegment; break;
        case IMEManager::THS_ConvertedSegment:   cat = Text::IMEStyle::SC_ConvertedSegment; break;
        case IMEManager::THS_PhraseLengthAdj:    cat = Text::IMEStyle::SC_PhraseLengthAdj; break;
        case IMEManager::THS_LowConfSegment:     cat = Text::IMEStyle::SC_LowConfSegment; break;
        default: SF_ASSERT(0); cat = Text::IMEStyle::SC_CompositionSegment; // avoid warning
        }
        pImpl->pTextField->HighlightCompositionStringText(pos, len, cat);
    }
}

void IMEManager::GetMetrics(RectF* pviewRect, RectF* pcursorRect, int cursorOffset)
{
    SF_UNUSED(pcursorRect);
    if (pImpl->pTextField)
    {
        Render::Matrix2D wm = pImpl->pTextField->GetWorldMatrix();
        RectF vr = pImpl->pTextField->GetBounds(wm);
        if (pviewRect)
            *pviewRect = TwipsToPixels(vr);

        UPInt curspos = pImpl->pTextField->GetCompositionStringPosition();
        if (curspos == SF_MAX_UPINT)
            curspos = pImpl->pTextField->GetCaretIndex();
        else
            curspos += pImpl->pTextField->GetCompositionStringLength();
        curspos += cursorOffset;
        if ((SPInt)curspos < 0)
            curspos = 0;

        RectF cr = pImpl->pTextField->GetCursorBounds(curspos, 0, 0);
            
        cr = wm.EncloseTransform(cr);
        if (pcursorRect)
            *pcursorRect = TwipsToPixels(cr);
    }
}

bool IMEManager::IsTextFieldFocused() const
{
    if (pImpl->pMovie)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pImpl->pMovie))->GetFocusedCharacter(0);
        return IsTextFieldFocused(pfocusedCh);
    }
    return false;
}

bool IMEManager::IsTextFieldFocused(InteractiveObject* ptextfield) const
{
    if (pImpl->pMovie)
    {
        Ptr<InteractiveObject> pfocusedCh = ((MovieImpl*)(pImpl->pMovie))->GetFocusedCharacter(0);
        return (pfocusedCh && pfocusedCh->GetType() == AS2::ObjectInterface::Object_TextField &&
                static_cast<TextField*>(pfocusedCh.GetPtr())->IsIMEEnabled() &&
                pfocusedCh == ptextfield);
    }
    return false;
}

void IMEManager::SetIMEMoviePath(const char* pcandidateSwfPath)
{
    pImpl->CandidateSwfPath = pcandidateSwfPath;
}

bool IMEManager::GetIMEMoviePath(String& candidateSwfPath)
{
    candidateSwfPath = pImpl->CandidateSwfPath;
    return !candidateSwfPath.IsEmpty();
}

bool IMEManager::SetCandidateListStyle(const IMECandidateListStyle& st)
{

    if (pImpl->pMovie)
	{
        ((MovieImpl*)pImpl->pMovie)->SetIMECandidateListStyle(st);
	}
    if (IsCandidateListLoaded())
    {
        OnCandidateListStyleChanged(st);
        return true;
    }
    return false;
}

bool IMEManager::GetCandidateListStyle(IMECandidateListStyle* pst) const
{
	if (IsCandidateListLoaded())
    {
        OnCandidateListStyleRequest(pst);
        return true;
    }
    else
    {
        if (pImpl->pMovie)
        {
			((MovieImpl*)pImpl->pMovie)->GetIMECandidateListStyle(pst);
            return true;
        }
    }

    return false;
}

namespace
{
	class CandidateListLoader : public AS2::MovieClipLoader
    {
        IMEManager*      pIMEManager;
        IMEManagerImpl*  pImpl;
    public:
        CandidateListLoader(IMEManager* pimeManager, IMEManagerImpl* pimImpl, AS2::Environment* penv) :
          MovieClipLoader(penv), pIMEManager(pimeManager), pImpl(pimImpl) {}

        void NotifyOnLoadInit(AS2::Environment* penv, InteractiveObject* ptarget)
        {
            SF_UNUSED(penv);   
            if (pImpl->pMovie)
            {
				GFx::Value v;
                v.SetNumber(2); // indicates - "loaded"
                pImpl->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
                v.SetString(pImpl->CandidateSwfPath.ToCStr());
                pImpl->pMovie->SetVariable("_global.gfx_ime_candidate_list_path", v);
            }

            if (ptarget)
            {
                ptarget->GetResourceMovieDef()->PinResource(); // so, do pin instead
                String path;
                ptarget->GetAbsolutePath(&path);
                pIMEManager->OnCandidateListLoaded(path.ToCStr());
            }
            else
                pIMEManager->OnCandidateListLoaded(NULL);
        }
        void NotifyOnLoadError(AS2::Environment* penv, InteractiveObject* ptarget, const char* errorCode, int status)
        {
            SF_UNUSED3(penv, ptarget, status);
            pImpl->CandidateSwfErrorMsg = "Error in loading candidate list from ";
            pImpl->CandidateSwfErrorMsg += pImpl->CandidateSwfPath;
            if (pImpl->pMovie)
            {
                String level0Path;
				AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pImpl->pMovie));
                pmovieRoot->GetLevel0Path(&level0Path);
                pImpl->CandidateSwfErrorMsg += " at ";
                pImpl->CandidateSwfErrorMsg += level0Path;

				GFx::Value v;
                v.SetNumber(-1); // means - "failed to load"
                pImpl->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
            }
            pImpl->CandidateSwfErrorMsg += ": ";
            pImpl->CandidateSwfErrorMsg += errorCode;
        }
    };
}

bool IMEManager::IsCandidateListLoaded() const
{
    // check if we already have loaded candidate list into _level9999
    if (pImpl->pMovie)
    {
		GFx::Value v;
        if (!pImpl->pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
            v.SetNumber(0);
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pImpl->pMovie));
        return pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL) && v.GetNumber() == 2;
    }
    return false;
}

bool IMEManager::AcquireCandidateList()
{
	AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pImpl->pMovie));
    if (pImpl->pMovie && pmovieRoot->GetLevelMovie(0))
    {
        // check if we already have loaded candidate list into _level9999
        // or if we are loading it at the time. We can't store bool inside the 
        // IME Manager since candidate lists are different for different movies,
        // but IME Manager could be the same.
		GFx::Value v;
        if (!pImpl->pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
            v.SetNumber(0);
        if (v.GetNumber() < 0)
            return false;
        if (!pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL) && v.GetNumber() != 1)
        {
			GFx::Value v;
            v.SetNumber(1); // means - "loading"
            pImpl->pMovie->SetVariable("_global.gfx_ime_candidate_list_state", v);
            
			AS2::GFxAS2LoadQueueEntry* pentry = new AS2::GFxAS2LoadQueueEntry
				(GFX_CANDIDATELIST_LEVEL, pImpl->CandidateSwfPath, AS2::GFxAS2LoadQueueEntry::LM_None, false, true);
			Ptr<AS2::MovieClipLoader> clipLoader = *new CandidateListLoader
                (this, pImpl, pmovieRoot->GetAvmLevelMovie(0)->GetASEnvironment());
            pentry->MovieClipLoaderHolder.SetAsObject(clipLoader);
            pmovieRoot->AddMovieLoadQueueEntry(pentry);
            return false;
        }
        return true;
    }
    return false;
}

bool IMEManager::GetCandidateListErrorMsg(String* pdest)
{
    if (pdest)
        *pdest = pImpl->CandidateSwfErrorMsg;
    return (pImpl->CandidateSwfErrorMsg.GetLength() > 0);
}

void IMEManager::DoFinalize()
{
//    if (pImpl->pTextField)
    {
        OnFinalize();
        pImpl->pTextField = NULL;
    }
}

void IMEManager::EnableIME(bool enable)
{
    if (pImpl->IMEDisabled == enable)
    {
        pImpl->IMEDisabled = !pImpl->IMEDisabled;
        OnEnableIME(enable);
    }
}


void IMEManager::BroadcastIMEConversion(const wchar_t* pString)
{
    // Convert to UTF8
    String gfxStr(pString);
    pImpl->BroadcastIMEConversion(gfxStr.ToCStr());
}

void IMEManager::BroadcastSwitchLanguage(const char* pString)
{
    pImpl->BroadcastSwitchLanguage(pString);
}

void IMEManager::BroadcastSetSupportedLanguages(const char* pString)
{
    pImpl->BroadcastSetSupportedLanguages(pString);
}

void IMEManager::BroadcastSetSupportedIMEs(const char* pString)
{
    pImpl->BroadcastSetSupportedIMEs(pString);
}

void IMEManager::BroadcastSetCurrentInputLanguage(const char* pString)
{
    pImpl->BroadcastSetCurrentInputLanguage(pString);
}

void IMEManager::BroadcastSetIMEName(const char* pString)
{
    pImpl->BroadcastSetIMEName(pString);
}

void IMEManager::BroadcastSetConversionStatus(const char* pString)
{
    pImpl->BroadcastSetConversionStatus(pString);
}

void IMEManager::BroadcastRemoveStatusWindow()
{
    pImpl->BroadcastRemoveStatusWindow();
}

void IMEManager::BroadcastDisplayStatusWindow()
{
	pImpl->BroadcastDisplayStatusWindow();
}

InteractiveObject* IMEManager::HandleFocus(Movie* pmovie, 
                                           InteractiveObject* poldFocusedItem, 
                                           InteractiveObject* pnewFocusingItem, 
                                           InteractiveObject* ptopMostItem)
{
    
    if (IsMovieActive(pmovie))
    {
		AS2::MovieRoot* pmovieRoot = AS2::ToAS2Root((MovieImpl*)(pImpl->pMovie));
        // This case pertains to when the focus is transfered to a textfield for the first time before 
        // any ime messages are sent. This provides a safe place to set the candidate list font for the 
        // candidate list sprites.
        if (pnewFocusingItem && pnewFocusingItem->GetType() == AS2::ObjectInterface::Object_TextField)
        {
            FontResource* pfont = ((TextField*)pnewFocusingItem)->GetFontResource();
			GFx::Value v;
            if (pfont)
            {
                if (!pImpl->pMovie->GetVariable(&v, "_global.gfx_ime_candidate_list_state"))
                    v.SetNumber(0);
                Sprite* imeMovie = pmovieRoot->GetLevelMovie(GFX_CANDIDATELIST_LEVEL);
                if (imeMovie && v.GetNumber() == 2)
                    imeMovie->SetIMECandidateListFont(pfont);
            }
        }

        if (poldFocusedItem && 
            poldFocusedItem->GetType() == AS2::ObjectInterface::Object_TextField)
        {
            if (!pnewFocusingItem)
            {
                // empty, check for candidate list. Need to use ptopMostItem, since
                // pnewFocusingItem is NULL
                // check, if the newly focused item candidate list or not
                if (ptopMostItem)
                {
                    String path;
                    ptopMostItem->GetAbsolutePath(&path);
                    if (IsCandidateList(path))
                    {
                        // prevent currently focused text field from losing focus.
                        return poldFocusedItem;
                    }

                    InteractiveObject* currItem = ptopMostItem;
					AS2::Value val;
                    while (currItem != NULL)
                    {
						Sprite* sprite		 = currItem->CharToSprite();
						AS2::AvmSprite* avmSprite = AS2::ToAvmSprite(sprite);
                        if (avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
                            avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isLanguageBar"), &val)
                            ||
							avmSprite->GetMemberRaw(avmSprite->GetASEnvironment()->GetSC(), 
							avmSprite->GetASEnvironment()->GetSC()->CreateConstString("isStatusWindow"), &val))
                        {
                            return poldFocusedItem;
                        }
                        currItem = currItem->GetParent();
                    };

                    if (IsStatusWindow(path))
                    {
                        // prevent currently focused text field from losing focus.
                        return poldFocusedItem;
                    }

                    if (IsLangBar(path))
                    {
                        // prevent currently focused text field from losing focus.
                        return poldFocusedItem;
                    }
                }

                // finalize, something else was clicked
                 DoFinalize();
            }
            else 
            {
                if (pnewFocusingItem->GetType() == AS2::ObjectInterface::Object_TextField)
                {
                    // clicked on another text field
                }
                // finalize?
                DoFinalize();
            }
        }

     //   DoFinalize();

        EnableIME(pnewFocusingItem && 
            pnewFocusingItem->GetType() == AS2::ObjectInterface::Object_TextField &&
            static_cast<TextField*>(pnewFocusingItem)->IsIMEEnabled());

    }
    return pnewFocusingItem;
}

void IMEManager::SetActiveMovie(Movie* pmovie)
{
    if (pmovie != pImpl->pMovie)
    {
        if (pImpl->pMovie)
        {
            // finalize (or, cancel?)
            DoFinalize();
        }
        pImpl->Reset();
        pImpl->SetActiveMovie(pmovie);
    }
}

void IMEManager::ClearActiveMovie()
{
    if (pImpl->pMovie)
    {
        pImpl->Reset();
        // finalize (or, cancel?)
        OnShutdown();
    }
}

bool IMEManager::IsMovieActive(Movie* pmovie) const
{
    return pImpl->pMovie == pmovie;
}

Movie* IMEManager::GetActiveMovie() const
{
    return pImpl->pMovie;   
}

// callback, invoked when mouse button is down. buttonsState is a mask:
//   bit 0 - right button is pressed,
//   bit 1 - left
//   bit 2 - middle
void IMEManager::OnMouseDown(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr)
{
    SF_UNUSED(buttonsState);
    if (IsMovieActive(pmovie) && pImpl->pTextField && pitemUnderMousePtr == pImpl->pTextField)
    {
        // if mouse clicked on the same active text field then finalize.
        // Otherwise let HandleFocus to handle the stuff.
        DoFinalize();
    }
}

void IMEManager::OnMouseUp(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr)
{
    SF_UNUSED3(pmovie, buttonsState, pitemUnderMousePtr);
}

unsigned IMEManager::HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent)
{
    SF_UNUSED2(pmovie, imeEvent);
    return Movie::HE_NotHandled;
}

void IMEManager::OnCandidateListLoaded(const char* pcandidateListPath) 
{ 
    SF_UNUSED(pcandidateListPath); 
}

void IMEManager::OnCandidateListStyleRequest(IMECandidateListStyle* pstyle) const
{
    if (pImpl->pMovie)
    {
        ((MovieImpl*)pImpl->pMovie)->GetIMECandidateListStyle(pstyle);
    }
}
#else //#ifdef SF_NO_IME_SUPPORT

// just to avoid warning "LNK4221: no public symbols found; archive member will be inaccessible" 
// IMEManager::IMEManager() : State(State_IMEManager) 
// { 
// }

#endif //#ifdef SF_NO_IME_SUPPORT
}}} //namespace Scaleform { namespace GFx {
