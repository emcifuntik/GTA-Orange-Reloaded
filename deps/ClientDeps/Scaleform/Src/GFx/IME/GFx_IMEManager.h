/**********************************************************************

Filename    :   IMEManager
Content     :   
Created     :   Mar 27, 2008
Authors     :   A. Bolgar

Copyright   :   (c) 2001-2008 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/
// IME manager interface class. This class may be used as a base class for various
// IME implementations. It also provides utility functions to control composition, 
// candidate list and so on.

#ifndef INC_IMEMANAGER_H
#define INC_IMEMANAGER_H
#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

#define GFX_CANDIDATELIST_LEVEL 9999
#define GFX_CANDIDATELIST_FONTNAME "$IMECandidateListFont"

	//class InteractiveObject;
	class IMEEvent;

	enum IMEConversionModes
	{
		IME_FullShape       = 0x01,
		IME_HalfShape       = 0x02,
		IME_Hanja           = 0x03,
		IME_NonHanja        = 0x04,
		IME_Katakana        = 0x05,
		IME_Hiragana        = 0x06,
		IME_Native          = 0x07,
		IME_AlphaNumeric    = 0x08

	};

	// A structure used to transfer IME candidate list styles
	// from ActionScript function to IME implementation.

	class IMECandidateListStyle : public NewOverrideBase<StatMV_Other_Mem>
	{
		UInt32 TextColor;
		UInt32 BackgroundColor;
		UInt32 IndexBackgroundColor;
		UInt32 SelectedTextColor;
		UInt32 SelectedBackgroundColor;
		UInt32 SelectedIndexBackgroundColor;
		UInt32 ReadingWindowTextColor;
		UInt32 ReadingWindowBackgroundColor;
		unsigned   FontSize; // in points 
		unsigned   ReadingWindowFontSize; 
		enum
		{
			Flag_TextColor                      = 0x01,
			Flag_BackgroundColor                = 0x02,
			Flag_IndexBackgroundColor           = 0x04,
			Flag_SelectedTextColor              = 0x08,
			Flag_SelectedBackgroundColor        = 0x10,
			Flag_SelectedIndexBackgroundColor   = 0x20,
			Flag_FontSize                       = 0x40,
			Flag_ReadingWindowTextColor         = 0x80,
			Flag_ReadingWindowBackgroundColor   = 0x100,
			Flag_ReadingWindowFontSize          = 0x200
		};
		UInt16  Flags;
	public:
		IMECandidateListStyle():Flags(0) {}

		void    SetTextColor(UInt32 color)         { Flags |= Flag_TextColor; TextColor = color; }
		UInt32  GetTextColor() const               { return (HasTextColor()) ? TextColor : 0; }
		void    ClearTextColor()                   { Flags &= (~Flag_TextColor); }
		bool    HasTextColor() const               { return (Flags & Flag_TextColor) != 0; }

		void    SetBackgroundColor(UInt32 backgr)  { Flags |= Flag_BackgroundColor; BackgroundColor = backgr; }
		UInt32  GetBackgroundColor() const         { return (HasBackgroundColor()) ? BackgroundColor : 0; }
		void    ClearBackgroundColor()             { Flags &= (~Flag_BackgroundColor); }
		bool    HasBackgroundColor() const         { return (Flags & Flag_BackgroundColor) != 0; }

		void    SetIndexBackgroundColor(UInt32 backgr) { Flags |= Flag_IndexBackgroundColor; IndexBackgroundColor = backgr; }
		UInt32  GetIndexBackgroundColor() const        { return (HasIndexBackgroundColor()) ? IndexBackgroundColor : 0; }
		void    ClearIndexBackgroundColor()            { Flags &= (~Flag_IndexBackgroundColor); }
		bool    HasIndexBackgroundColor() const        { return (Flags & Flag_IndexBackgroundColor) != 0; }

		void    SetSelectedTextColor(UInt32 color) { Flags |= Flag_SelectedTextColor; SelectedTextColor = color; }
		UInt32  GetSelectedTextColor() const       { return (HasSelectedTextColor()) ? SelectedTextColor : 0; }
		void    ClearSelectedTextColor()           { Flags &= (~Flag_SelectedTextColor); }
		bool    HasSelectedTextColor() const       { return (Flags & Flag_SelectedTextColor) != 0; }

		void    SetSelectedBackgroundColor(UInt32 backgr) { Flags |= Flag_SelectedBackgroundColor; SelectedBackgroundColor = backgr; }
		UInt32  GetSelectedBackgroundColor() const        { return (HasSelectedBackgroundColor()) ? SelectedBackgroundColor : 0; }
		void    ClearSelectedBackgroundColor()            { Flags &= (~Flag_SelectedBackgroundColor); }
		bool    HasSelectedBackgroundColor() const        { return (Flags & Flag_SelectedBackgroundColor) != 0; }

		void    SetSelectedIndexBackgroundColor(UInt32 backgr) { Flags |= Flag_SelectedIndexBackgroundColor; SelectedIndexBackgroundColor = backgr; }
		UInt32  GetSelectedIndexBackgroundColor() const        { return (HasSelectedIndexBackgroundColor()) ? SelectedIndexBackgroundColor : 0; }
		void    ClearSelectedIndexBackgroundColor()            { Flags &= (~Flag_SelectedIndexBackgroundColor); }
		bool    HasSelectedIndexBackgroundColor() const        { return (Flags & Flag_SelectedIndexBackgroundColor) != 0; }

		void    SetFontSize(unsigned fs) { Flags |= Flag_FontSize; FontSize = fs; }
		unsigned    GetFontSize() const  { return (HasFontSize()) ? FontSize : 0; }
		void    ClearFontSize()      { Flags &= (~Flag_FontSize); }
		bool    HasFontSize() const  { return (Flags & Flag_FontSize) != 0; }

		// Reading Window styles.
		void    SetReadingWindowTextColor(unsigned fs) { Flags |= Flag_ReadingWindowTextColor; ReadingWindowTextColor = fs; }
		unsigned    GetReadingWindowTextColor() const  { return (HasReadingWindowTextColor()) ? ReadingWindowTextColor : 0; }
		void    ClearReadingWindowTextColor()      { Flags &= (~Flag_ReadingWindowTextColor); }
		bool    HasReadingWindowTextColor() const  { return (Flags & Flag_ReadingWindowTextColor) != 0; }

		void    SetReadingWindowBackgroundColor(unsigned fs) { Flags |= Flag_ReadingWindowBackgroundColor; ReadingWindowBackgroundColor = fs; }
		unsigned    GetReadingWindowBackgroundColor() const  { return (HasReadingWindowBackgroundColor()) ? ReadingWindowBackgroundColor : 0; }
		void    ClearReadingWindowBackgroundColor()      { Flags &= (~Flag_ReadingWindowBackgroundColor); }
		bool    HasReadingWindowBackgroundColor() const  { return (Flags & Flag_ReadingWindowBackgroundColor) != 0; }

		void    SetReadingWindowFontSize(unsigned fs) { Flags |= Flag_ReadingWindowFontSize; ReadingWindowFontSize = fs; }
		unsigned    GetReadingWindowFontSize() const  { return (HasReadingWindowFontSize()) ? ReadingWindowFontSize : 0; }
		void    ClearReadingWindowFontSize()      { Flags &= (~Flag_ReadingWindowFontSize); }
		bool    HasReadingWindowFontSize() const  { return (Flags & Flag_ReadingWindowFontSize) != 0; }
	};

class IMEManagerBase : public State
{
public:
	IMEManagerBase(): State(State_IMEManager) {};
    void OnMouseDown(Movie* pmovie, int buttonState, InteractiveObject* pitemUnderMousePtr)
	{
		SF_UNUSED3(pmovie, buttonState, pitemUnderMousePtr);
	}
    void OnMouseUp(Movie* pmovie, int buttonState, InteractiveObject* pitemUnderMousePtr)
	{
		SF_UNUSED3(pmovie, buttonState, pitemUnderMousePtr);
	}

    // Returns true, if the specified text field is currently focused.
	bool IsTextFieldFocused(InteractiveObject* ptextfield) const {SF_UNUSED(ptextfield); return false;}

    // Finalize the composition and release the text field.
	void DoFinalize() {};

	virtual UInt32 Init(Log* plog){ plog = 0; return 1;};
	// sets currently focused movie view to IME manager.
	virtual void SetActiveMovie(Movie* pmovie) { SF_UNUSED(pmovie);};
	virtual bool IsMovieActive(Movie* pmovie) const { SF_UNUSED(pmovie); return false;};
	virtual Movie* GetActiveMovie() const {return NULL; };
	// cleans out the movie from the IME. NO ACTIONSCRIPT should be used
	// with movie being cleaned up! As well as, do not save it in Ptr.
	virtual void ClearActiveMovie() {};
	// Handles IME events, calling callbacks and switching states.
	virtual unsigned HandleIMEEvent(Movie* pmovie, const IMEEvent& imeEvent)
	{
		 SF_UNUSED2(pmovie, imeEvent); return 0;
	};
	InteractiveObject* HandleFocus(Movie* pmovie, 
		InteractiveObject* poldFocusedItem, 
		InteractiveObject* pnewFocusingItem, 
		InteractiveObject* ptopMostItem)
	{
		SF_UNUSED4(pmovie, poldFocusedItem, pnewFocusingItem, ptopMostItem);
		return NULL;
	};

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
	virtual void BroadcastIMEConversion(const wchar_t* pString) { SF_UNUSED(pString);}

	// Support for OnSwitchLanguage event
	virtual void BroadcastSwitchLanguage(const char* pString) { SF_UNUSED(pString);};

	// Support for OnSetSupportedLanguages event
	virtual void BroadcastSetSupportedLanguages(const char* pString) { SF_UNUSED(pString);};

	// Support for OnSetSupportedIMEs event
	virtual void BroadcastSetSupportedIMEs(const char* pString) { SF_UNUSED(pString);};

	// Support for OnSetCurrentInputLanguage event
	virtual void BroadcastSetCurrentInputLanguage(const char* pString) { SF_UNUSED(pString);};

	// Support for OnSetIMEName event
	virtual void BroadcastSetIMEName(const char* pString) { SF_UNUSED(pString);};

	// Support for OnSetConversionStatus event
	virtual void BroadcastSetConversionStatus(const char* pString) { SF_UNUSED(pString);};

	// Support for OnBroadcastRemoveStatusWindow event
	virtual void BroadcastRemoveStatusWindow() {};

	// Support for OnBroadcastDisplayStatusWindow event
	virtual void BroadcastDisplayStatusWindow() {};

	// handle "_global.imecommand(cmd, param)"
	virtual void IMECommand(Movie* pmovie, const char* pcommand, const char* pparam) 
	{ SF_UNUSED3(pmovie, pcommand, pparam); }

	// SetCompositionString
	virtual bool SetCompositionString(const char* pCompString) { SF_UNUSED(pCompString); return false; }; 

};

inline void                 StateBag::SetIMEManager(IMEManagerBase *ptr)       
{ 
    SetState(State::State_IMEManager, ptr); 
}

inline Ptr<IMEManagerBase>  StateBag::GetIMEManager() const                   
{ 
    return *(IMEManagerBase*) GetStateAddRef(State::State_IMEManager); 
}

//DOM-IGNORE-BEGIN

}} // namespace Scaleform { namespace GFx {
//DOM-IGNORE-END
#endif //INC_IMEMANAGER_H
