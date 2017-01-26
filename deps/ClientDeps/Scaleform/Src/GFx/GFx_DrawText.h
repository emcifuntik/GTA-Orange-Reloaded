/**********************************************************************

Filename    :   DrawText.h
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

#ifndef INC_SF_GFX_GFXDRAWTEXT_H
#define INC_SF_GFX_GFXDRAWTEXT_H

#include "GFxConfig.h"
#ifdef GFX_ENABLE_DRAWTEXT
#include "Kernel/SF_Types.h"
#include "Kernel/SF_RefCount.h"
#include "Render/Render_Types2D.h"
#include "Render/Render_Viewport.h"
#include "Render/Render_TreeText.h"
#include "GFx/GFx_Loader.h"
#include "GFx/GFx_PlayerStats.h"
#include "GFx/GFx_String.h"
#include "GFx/Text/Text_DocView.h"

// a special case for Windows: windows.h renames DrawText to 
// either DrawTextA or DrawTextW depending on UNICODE macro.
// Avoiding the effect on our DrawText class.
#ifdef _WINDOWS
#ifdef DrawText
#undef DrawText
#endif // DrawText
#endif // _WINDOWS

namespace Scaleform { namespace GFx {

class FontManager;
class FontManagerStates;
class DrawTextImpl;
class DrawTextManager;

/* This class provides external text drawing functionality in GFx. User may use this
   class in conjunction with DrawTextManager to draw his own text without
   loading SWF/GFX files. 
   DrawTextManager::CreateText or DrawTextManager::CreateHtmlText should be 
   used to create an instance of DrawText:

       Ptr<DrawText> ptxt = *pdm->CreateText("FHWEUHF!", RectF(20, 20, 500, 400));

       Ptr<DrawText> ptxt3 = *pdm->CreateHtmlText("<p><FONT size='20'>AB <b>singleline</b><i> CD</i>O", RectF(20, 300, 400, 700));

   To render the text use the method Display. Note, it is necessary to call DrawTextManager::BeginDisplay
   before the first call to Display and to call DrawTextManager::EndDisplay after the last call to Display:

       Render::Viewport vp(GetWidth(), GetHeight(), 0, 0, GetWidth(), GetHeight(), 0);
       pdm->BeginDisplay(vp);

       ptxt->Display();
       ptxt2->Display();
       ptxt3->Display();

       pdm->EndDisplay();
   */
class DrawText : public RefCountBaseNTS<DrawText, StatMV_Text_Mem>
{
    friend class DrawTextManager;
public:
    typedef Render::Matrix2F                Matrix;
    typedef Render::Cxform                  Cxform;
    typedef Render::RectF                   RectF;
    typedef Render::SizeF                   SizeF;
    typedef Render::PointF                  PointF;
    typedef Render::TreeText::FontStyle     FontStyle;
    typedef Render::TreeText::Alignment     Alignment;
    typedef Render::TreeText::VAlignment    VAlignment;
    typedef Render::TreeText::AAMode        AAMode;

    enum
    {
        Align_Left          = Render::TreeText::Align_Left,
        Align_Default       = Render::TreeText::Align_Default,
        Align_Right         = Render::TreeText::Align_Right,
        Align_Center        = Render::TreeText::Align_Center,
        Align_Justify       = Render::TreeText::Align_Justify,

        VAlign_Top          = Render::TreeText::VAlign_Top,
        VAlign_Default      = Render::TreeText::VAlign_Default,
        VAlign_Center       = Render::TreeText::VAlign_Center,
        VAlign_Bottom       = Render::TreeText::VAlign_Bottom,

        Normal              = Render::TreeText::Normal,
        Bold                = Render::TreeText::Bold,
        Italic              = Render::TreeText::Italic,
        BoldItalic          = Render::TreeText::BoldItalic,
        ItalicBold          = Render::TreeText::ItalicBold,

        AA_Animation        = Render::TreeText::AA_Animation,
        AA_Readability      = Render::TreeText::AA_Readability,

        Filter_Unknown      = Render::TreeText::Filter_Unknown,
        Filter_DropShadow   = Render::TreeText::Filter_DropShadow,
        Filter_Blur         = Render::TreeText::Filter_Blur,
        Filter_Glow         = Render::TreeText::Filter_Glow,

        FilterFlag_KnockOut = Render::TreeText::FilterFlag_KnockOut,
        FilterFlag_HideObject = Render::TreeText::FilterFlag_HideObject,
        FilterFlag_FineBlur = Render::TreeText::FilterFlag_FineBlur
    };

    // SetText sets UTF-8, UCS-2 or String text value to the text object. The optional 
    // parameter �lengthInBytes� specifies number of bytes in the UTF-8 string; 
    // �lengthInChars� specifies number of characters in wide character string.
    // If these parameters are not specified then
    // the UTF-8 and UCS-2 strings should be null-terminated.
    virtual void SetText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1)) = 0;
    virtual void SetText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1)) = 0;
    virtual void SetText(const String& str) = 0;
    // Returns currently set text in UTF-8 format. It returns plain text value;
    // even if HTML is used then it returns the string with all HTML tags stripped out.
    virtual String GetText() const = 0;

    // SetHtmlText parses UTF-8, UCS-2 or String encoded HTML and initializes the text object
    // by the parsed HTML text.
    // The optional parameter �lengthInBytes� specifies number of bytes in the UTF-8 string; 
    // �lengthInChars� specifies number of characters in wide character string.
    // If these parameters are not specified then
    // the UTF-8 and UCS-2 strings should be null-terminated.
    virtual void SetHtmlText(const char* putf8Str, UPInt lengthInBytes = UPInt(-1)) = 0;
    virtual void SetHtmlText(const wchar_t* pstr,  UPInt lengthInChars = UPInt(-1)) = 0;
    virtual void SetHtmlText(const String& str) = 0;
    // Returns currently set text in HTML format. If plain text is used with setting formatting 
    // by calling methods, such as SetColor, SetFont, etc, then this text will be converted to 
    // appropriate HTML format by this method.
    virtual String GetHtmlText() const = 0;

    // Sets color (R, G, B, A) to whole text or to the part of text in interval [startPos..endPos].
    // Both �startPos� and �endPos� parameters are optional.
    virtual void SetColor(Color c, UPInt startPos = 0, UPInt endPos = UPInt(-1)) = 0;
    // Sets font to whole text or to the part of text in interval [startPos..endPos].
    // Both �startPos� and �endPos� parameters are optional.
    virtual void SetFont (const char* pfontName, UPInt startPos = 0, UPInt endPos = UPInt(-1)) = 0;
    // Sets font size to whole text or to the part of text in interval [startPos..endPos].
    // Both �startPos� and �endPos� parameters are optional.
    virtual void SetFontSize(float fontSize, UPInt startPos = 0, UPInt endPos = UPInt(-1)) = 0;

    // Sets font style to whole text or to the part of text in interval [startPos..endPos].
    // Both �startPos� and �endPos� parameters are optional.
    virtual void    SetFontStyle(FontStyle, UPInt startPos = 0, UPInt endPos = UPInt(-1)) = 0;
    // Sets or clears underline to whole text or to the part of text in interval [startPos..endPos].
    // Both �startPos� and �endPos� parameters are optional.
    virtual void    SetUnderline(bool underline, UPInt startPos = 0, UPInt endPos = UPInt(-1)) = 0;

    // Sets multiline (parameter �multiline� is set to true) or singleline (false) type of the text.
    virtual void    SetMultiline(bool multiline) = 0;
    // Returns 'true' if the text is multiline; �false� otherwise.
    virtual bool    IsMultiline() const = 0;

    // Turns wordwrapping on/off
    virtual void    SetWordWrap(bool wordWrap) = 0;
    // Returns state of wordwrapping.
    virtual bool    IsWordWrap() const = 0;

    // Sets view rectangle, coordinates are in pixels.
    virtual void    SetRect(const RectF& viewRect) = 0;
    // Returns currently used view rectangle, coordinates are in pixels.
    virtual RectF   GetRect() const = 0;

    // Sets transformation matrix to the text object.
    virtual void    SetMatrix(const Matrix& matrix) = 0;
    // Returns the currently using transformation matrix.
    virtual Matrix  GetMatrix() const = 0;

    // Set color transformation matrix to the text object.
    virtual void    SetCxform(const Cxform& cx) = 0;
    // Returns the currently using color transformation matrix.
    virtual const Cxform& GetCxform() const = 0;

    // Set color of border around the text. If alpha is set to 0 then border is 
    // not drawing (default behavior).
    virtual void    SetBorderColor(const Color& borderColor) = 0;
    
    // Returns currently set border color. If alpha is 0 then border is turned off.
    virtual Color   GetBorderColor() const = 0;

    // Set color of background under the text. If alpha is set to 0 then background is 
    // not drawing (default behavior).
    virtual void    SetBackgroundColor(const Color& bkgColor) = 0;

    // Returns currently set background color. If alpha is 0 then background is turned off.
    virtual Color   GetBackgroundColor() const = 0;

    // Sets horizontal text alignment (right, left, center)
    virtual void        SetAlignment(Alignment) = 0;
    // Returns horizontal text alignment (right, left, center)
    virtual Alignment   GetAlignment() const = 0;

    // Sets vertical text alignment (top, bottom, center)
    virtual void        SetVAlignment(VAlignment) = 0;
    // Returns vertical text alignment (top, bottom, center)
    virtual VAlignment  GetVAlignment() const = 0;

    // Displays the text. Note, GFxDrawTextManager�s BeginDisplay 
    // method should be called before the first call to this method, 
    // and EndDisplay method should be called after the last Display 
    // method call.
    //virtual void        Display() = 0;

    // Sets a string value that is passed to the Render::Renderer::PushUserData virtual function. 
    // Developers can use this interface to pass custom text field specific data to 
    // their own renderer implementations. 
    //virtual void        SetRendererString(const String& str) = 0;
    // Returns a string value previously set by SetRendererString.
    //virtual String      GetRendererString() const   = 0;

    // Sets a float value that is passed to the Render::Renderer::PushUserData virtual function. 
    // Developers can use this interface to pass custom text field specific data to 
    // their own renderer implementations. 
    //virtual void        SetRendererFloat(float f) = 0;
    // Returns a float value previously set by SetRendererFloat.
    //virtual float       GetRendererFloat() const  = 0;

    // Sets an array or matrix of floats (maximum size 16) that is passed to the Render::Renderer::PushUserData virtual function. 
    // Developers can use this interface to pass custom text field specific data to 
    // their own renderer implementations.
    //virtual void        SetRendererMatrix(float *m, unsigned count = 16) =0;

    // Sets anti-aliasing mode - AA_Readibility for readability and AA_Animation for animation.
    // The default setting is anti-aliasing for animation.
    virtual void        SetAAMode(AAMode) =0;
    // Returns currently set anti-aliasing mode.
    virtual AAMode      GetAAMode() const =0;

protected:
    virtual ~DrawText() {}

    //virtual Text::DocView* GetDocView() const = 0;
};

/* This class manages the DrawText objects. DrawTextManager should be used 
   for creation of DrawText objects. It also may be used to measure text extents. 
   To render text, BeginDisplay should be called before the first call to DrawText::Display
   and the EndDisplay method should be called after the last call to DrawText::Display. 
   One DrawTextManager instance may be used to manage (create, measure, render) multiple
   DrawText instances.
   
   There are 3 ways to create DrawTextManager: using default constructor, passing pointer on 
   MovieDef, and passing pointer on Loader / GFxStateBag:

   1.   Ptr<DrawTextManager> pdtm = *new DrawTextManager();
   2. 
        Loader loader;
        ...
        Ptr<MovieDef> pmd = *loader.CreateMovie("fonts.swf");
        Ptr<DrawTextManager> pdtm = *new DrawTextManager(pmd);
   3. 
        Loader loader;
        ...
        Ptr<DrawTextManager> pdtm = *new DrawTextManager(&loader);

   In the case #2 all created DrawText instances will be able to use all fonts defined by the loaded "fonts.swf".
   In the case #3 DrawTextManager will inherit all states (log, font cache manager, etc) from the loader.
   */
class DrawTextManager : public RefCountBaseNTS<DrawTextManager, Stat_Default_Mem>, public StateBag
{
    friend class DrawTextImpl;
public:
    typedef Render::Viewport Viewport;
    struct TextParams
    {
        Color                   TextColor;
        DrawText::Alignment     HAlignment;
        DrawText::VAlignment    VAlignment;
        DrawText::FontStyle     FontStyle;
        float                   FontSize;
        String                  FontName;
        bool                    Underline;
        bool                    Multiline;
        bool                    WordWrap;

        TextParams();
    };

protected:
    class DrawTextManagerImpl*  pImpl;
    MemoryHeap*                 pHeap;

    virtual StateBag* GetStateBagImpl() const;

    Text::Allocator*    GetTextAllocator();
    FontManager*        GetFontManager();
    FontManagerStates*  GetFontManagerStates();
    void                CheckFontStatesChange();

    void SetBeginDisplayInvokedFlag(bool v = true); 
    void ClearBeginDisplayInvokedFlag();
    bool IsBeginDisplayInvokedFlagSet() const;
public:
    // Constructor. If pmovieDef is specified then the manager inherits all 
    // states from it, including fonts, font manager, font providers, font cache manager, etc.
    DrawTextManager(MovieDef* pmovieDef = NULL);
    // Constructor. Copies all states from the 'ploader', plus, ResourceLib. 
    // This ctor gets a pointer to Loader for compatibility reasons; in future, it might be
    // changed to a reference.
    DrawTextManager(Loader* ploader);
    ~DrawTextManager();

    // Sets default text parameters. If optional 'ptxtParams' parameter for CreateText is not
    // set then these default parameters will be used.
    void SetDefaultTextParams(const TextParams& params);
    // Returns currently set default text parameters.
    const TextParams& GetDefaultTextParams() const;

    // creates an empty DrawText object.
    DrawText* CreateText();

    // creates and initialize a DrawText object. If ptxtParams specified the 
    // created instance will be initialized using it.
    DrawText* CreateText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams = NULL);
    DrawText* CreateText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams = NULL);
    DrawText* CreateText(const String& str, const RectF& viewRect, const TextParams* ptxtParams = NULL);

    // creates and initialize a DrawText object using specified HTML.
    DrawText* CreateHtmlText(const char* putf8Str, const RectF& viewRect, const TextParams* ptxtParams = NULL);
    DrawText* CreateHtmlText(const wchar_t* pwstr, const RectF& viewRect, const TextParams* ptxtParams = NULL);
    DrawText* CreateHtmlText(const String& str, const RectF& viewRect, const TextParams* ptxtParams = NULL);

    // Returns size of the text rectangle that would be necessary to render the
    // specified text using the text parameters from the txtParams parameter.
    // If WordWrap and Multiline in txtParams are set to 'true' then it uses 'width' parameter
    // as the width and calculates the height.  
    SizeF GetTextExtent(const char* putf8Str, float width = 0, const TextParams* ptxtParams = NULL);
    SizeF GetTextExtent(const wchar_t* pwstr, float width = 0, const TextParams* ptxtParams = NULL);
    SizeF GetTextExtent(const String& str, float width = 0, const TextParams* ptxtParams = NULL);

    // Returns size of the text rectangle that would be necessary to render the
    // specified HTML text.  
    // If 'width' contains positive value then it assumes word wrapping is on and only 
    // the height will be calculated. HTML is treated as multiline text.
    SizeF GetHtmlTextExtent(const char* putf8Str, float width = 0, const TextParams* ptxtParams = NULL);
    SizeF GetHtmlTextExtent(const wchar_t* pwstr, float width = 0, const TextParams* ptxtParams = NULL);
    SizeF GetHtmlTextExtent(const String& str, float width = 0, const TextParams* ptxtParams = NULL);

    // Begins/ends display of text objects. BeginDisplay method should be called before any call to
    // DrawText::Display method. It is possible to put multiple calls to DrawText::Display
    // for mutliple instances of DrawText. After all text is drawn call the EndDisplay method.
    void BeginDisplay(const Viewport& vp);
    void EndDisplay();

    Render::Context&    GetRenderContext();
    Render::TreeRoot*   GetRenderRoot();
protected:
    void            SetTextParams(Text::DocView* pdoc, const TextParams& txtParams,
                                  const Text::TextFormat* tfmt = NULL, const Text::ParagraphFormat* pfmt = NULL);
    Text::DocView* CreateTempDoc(const TextParams& txtParams,
                                  Text::TextFormat* tfmt, Text::ParagraphFormat *pfmt,
                                  float width, float height);
};

// Restore DrawTextA/DrawTextW macros. Note, to use Scaleform's
// DrawText you'll need to use fully qualified name (GFx::DrawText)
// if you uncomment the lines below.
// #ifdef _WINDOWS
// typedef DrawText DrawTextA;
// typedef DrawText DrawTextW;
// #ifdef UNICODE
// #define DrawText DrawTextW
// #else
// #define DrawText DrawTextA
// #endif
// #endif //_WINDOWS

}} // namespace Scaleform::GFx

#endif //GFX_ENABLE_DRAWTEXT


#endif //INC_SF_GFX_GFXDRAWTEXT_H

