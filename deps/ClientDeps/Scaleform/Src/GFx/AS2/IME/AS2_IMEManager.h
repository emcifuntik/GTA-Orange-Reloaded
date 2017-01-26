
/**********************************************************************

PublicHeader:   GFx
Filename    :   IME/IMEManager.h
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
#ifndef INC_AS2IMEMANAGER_H
#define INC_AS2IMEMANAGER_H

#include "GFxConfig.h"
#include "GFx/GFx_Loader.h"
#include "Render/Render_Types2D.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/IME/GFx_IMEManager.h"
namespace Scaleform { namespace GFx { namespace AS2{

#ifndef SF_NO_IME_SUPPORT


// IME manager interface class. This class may be used as a base class for various
// IME implementations. It also provides utility functions to control composition, 
// candidate list and so on.

class IMEManager : public IMEManagerBase
{
    friend class MovieImpl;

    class IMEManagerImpl *pImpl;

    unsigned UnsupportedIMEWindowsFlag;
    // handles focus. 
    InteractiveObject* HandleFocus(Movie* pmovie, 
        InteractiveObject* poldFocusedItem, 
        InteractiveObject* pnewFocusingItem, 
        InteractiveObject* ptopMostItem);

    // callback, invoked when mouse button is down. buttonsState is a mask:
    //   bit 0 - right button is pressed,
    //   bit 1 - left
    //   bit 2 - middle
    void OnMouseDown(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);
    void OnMouseUp(Movie* pmovie, int buttonsState, InteractiveObject* pitemUnderMousePtr);

public:

    enum 
    {
        GFxIME_AlwaysHideUnsupportedIMEWindows          = 0x01,
        GFxIME_HideUnsupportedIMEWindowsInFullScreen    = 0x02,
        GFxIME_ShowUnsupportedIMEWindows                = 0x03
    } ; 


    Log* pLog;

    IMEManager();
    ~IMEManager();

    //**** utility methods
    // creates the composition string, if not created yet
    void StartComposition();

    // finalizes the composition string by inserting the string into the
    // actual text. If pstr is not NULL then the content of pstr is being used;
    // otherwise, the current content of composition string will be used.
    void FinalizeComposition(const wchar_t* pstr, UPInt len = SF_MAX_UPINT);

    // clears the composition string. FinalizeComposition with pstr != NULL
    // still may be used after ClearComposition is invoked.
    void ClearComposition();

    // release the composition string, so no further composition string related
    // functions may be used.
    void ReleaseComposition();

    // changes the text in composition string
    void SetCompositionText(const wchar_t* pstr, UPInt len = SF_MAX_UPINT);

    // relocates the composition string to the current caret position
    void SetCompositionPosition();

    // sets cursor inside the composition string. "pos" is specified relative to 
    // composition string.
    void SetCursorInComposition(UPInt pos);

    // turns on/off wide cursor.
    void SetWideCursor(bool = true);

    // styles of text highlighting (used in HighlightText)
    enum TextHighlightStyle
    {
        THS_CompositionSegment   = 0,
        THS_ClauseSegment        = 1,
        THS_ConvertedSegment     = 2,
        THS_PhraseLengthAdj      = 3,
        THS_LowConfSegment       = 4
    };
    // highlights the clause in composition string.
    // Parameter "clause" should be true, if this method is called to highlight
    // the clause (for example, for Japanese IME). In this case, in addition to the
    // requested highlighting whole composition string will be underline by single
    // underline.
    void HighlightText(UPInt pos, UPInt len, TextHighlightStyle style, bool clause);

    // returns view rectangle of currently focused text field
    // and cursor rectangle, both in stage (root) coordinate space.
    // cursorOffset may be negative, specifies the offset from the 
    // actual cursor pos.
    void GetMetrics(RectF* pviewRect, RectF* pcursorRect, int cursorOffset = 0);

    // Returns true, if text field is currently focused.
    bool IsTextFieldFocused() const;

    // Returns true, if the specified text field is currently focused.
    bool IsTextFieldFocused(InteractiveObject* ptextfield) const;

    // Checks if candidate list is loaded
    bool IsCandidateListLoaded() const;

    // Sets candidate list movie path
    void SetIMEMoviePath(const char* pcandidateSwfPath);

    // Gets candidate list path for logging
    bool GetIMEMoviePath(String& candidateSwfPath);

    // Sets style of candidate list. Invokes OnCandidateListStyleChanged callback.
    bool SetCandidateListStyle(const IMECandidateListStyle& st);

    // Gets style of candidate list
    bool GetCandidateListStyle(IMECandidateListStyle* pst) const;

    void ShowUnsupportedIMEWindows(unsigned unsupportedIMEWindowsFlag) {UnsupportedIMEWindowsFlag = unsupportedIMEWindowsFlag;};

    unsigned GetUnsupportedIMEWindowsFlag(){ return UnsupportedIMEWindowsFlag; };

    /****** Flash IME class functions **************/

    // Sets conversion mode. Base class version does nothing.
    virtual bool SetConversionMode(const unsigned convMode){ SF_UNUSED(convMode); return false;};

    // retrieves conversion mode. Base class version does nothing.
    virtual const char* GetConversionMode() {return (const char*)("UNKNOWN"); }; 

    // Enables/Disables IME. 
    virtual bool SetEnabled(bool enable) { SF_UNUSED(enable); return false; }; 

    // Retrieves IME state. 
    virtual bool GetEnabled() { return false; }; 

    // Retrieves current input language
    virtual const String GetInputLanguage() { return String("UNKNOWN");};

    // Support for OnIMEComposition event
    virtual void BroadcastIMEConversion(const wchar_t* pString);

    // Support for OnSwitchLanguage event
    virtual void BroadcastSwitchLanguage(const char* pString);

    // Support for OnSetSupportedLanguages event
    virtual void BroadcastSetSupportedLanguages(const char* pString);

    // Support for OnSetSupportedIMEs event
    virtual void BroadcastSetSupportedIMEs(const char* pString);

    // Support for OnSetCurrentInputLanguage event
    virtual void BroadcastSetCurrentInputLanguage(const char* pString);

    // Support for OnSetIMEName event
    virtual void BroadcastSetIMEName(const char* pString);

    // Support for OnSetConversionStatus event
    virtual void BroadcastSetConversionStatus(const char* pString);

    // Support for OnBroadcastRemoveStatusWindow event
    virtual void BroadcastRemoveStatusWindow();

    // Support for OnBroadcastDisplayStatusWindow event
    virtual void BroadcastDisplayStatusWindow();

    // SetCompositionString
    virtual bool SetCompositionString(const char* pCompString) { SF_UNUSED(pCompString); return false; }; 

    /****** End Flash IME class functions **************/

    // Loads candidate list movie, if it wasn't loaded yet. It will invoke
    // OnCandidateListLoaded(path) virtual method once movie is loaded.
    bool AcquireCandidateList();

    // Loads the Status window movie.
    bool AcquireStatusWindowMovie();

    // Loads the Language bar movie.
    bool AcquireLanguageBarMovie();

    // Fills pdest with error message, if candidate list failed to load.
    // Returns 'true' if error occurred.
    bool GetCandidateListErrorMsg(String* pdest);

    // This callback function is called then the candidate list dialog
    // is ready to be shown. It is used to set the font to show the dialog 
    // from the current text field.
    void OnOpenCandidateList();

    // Finalize the composition and release the text field.
    void DoFinalize();

    // enables/disables IME
    void EnableIME(bool enable);

    virtual UInt32 Init(Log* plog){ plog = 0; return 1;};
    // sets currently focused movie view to IME manager.
    virtual void SetActiveMovie(Movie*);
    virtual bool IsMovieActive(Movie*) const;
    virtual Movie* GetActiveMovie() const;
    // cleans out the movie from the IME. NO ACTIONSCRIPT should be used
    // with movie being cleaned up! As well as, do not save it in Ptr.
    virtual void ClearActiveMovie();

    // *** callbacks, invoked from core. These virtual methods
    // might be overloaded by the implementation.


    // invoked to check does the symbol belong to candidate list or not
    virtual bool IsCandidateList(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to status window or not
    virtual bool IsStatusWindow(const char* ppath) { SF_UNUSED(ppath); return false; }

    // invoked to check does the symbol belong to lang bar window or not
    virtual bool IsLangBar(const char* ppath) { SF_UNUSED(ppath); return false; }

    // Handles IME events, calling callbacks and switching states.
    virtual unsigned HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent);

    // handle "_global.imecommand(cmd, param)"
    virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
    { SF_UNUSED3(pmovie, pcommand, pparam); }

    // invoked when need to finalize the composition.
    virtual void OnFinalize() {}

    // invoked when need to cleanup and shutdown the IME. Do not invoke ActionScript from it!
    virtual void OnShutdown() {}

    // invoked when candidate list loading is completed.
    virtual void OnCandidateListLoaded(const char* pcandidateListPath);

    // invoked when candidate list's style has been changed by ActionScript
    virtual void OnCandidateListStyleChanged(const IMECandidateListStyle& style) { SF_UNUSED(style); }

    // invoked when ActionScript is going to get candidate list style properties.
    // The default implementation just retrieves styles from movie view.
    virtual void OnCandidateListStyleRequest(IMECandidateListStyle* pstyle) const;

    // handles enabling/disabling IME, invoked from EnableIME method
    virtual void OnEnableIME(bool enable) { SF_UNUSED(enable); }
};

#else // #ifdef SF_NO_IME_SUPPORT
class IMECandidateListStyle : public NewOverrideBase<Stat_Default_Mem>
{
};

#endif // #ifdef SF_NO_IME_SUPPORT

}}} // namespace Scaleform { namespace GFx {

#endif //INC_IMEMANAGER_H
