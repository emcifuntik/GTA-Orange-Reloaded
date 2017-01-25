/**********************************************************************

PublicHeader:   GFx
Filename    :   FontLib.h
Content     :   Implementation of font sharing and lookup through
                a list of registed MovieDef's.
Created     :   July 9, 2007
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2007 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_FontLib_H
#define INC_SF_GFX_FontLib_H

#include "GFx/GFx_Loader.h"

namespace Scaleform { namespace GFx {

// ***** FontLib

// An user-managed state used to register MovieDefs so that they can 
// be searched for fonts when creating other movies. Fonts in registered
// files form a library, which is searched before GFxFontProvider.
// Unlike GFxFontProvider, FontLib does not require system font support
// and is thus portable to consoles.

class FontResource;
class MovieDef;



class FontLib : public State
{
    class FontLibImpl *pImpl;
public:
    FontLib();
    virtual ~FontLib();

    // Registers a MovieDef that will be used as a source for fonts.
    // By default, the the movie is pinned in the loader so that its
    // textures don't get unloaded.
    void                AddFontsFrom(MovieDef* md, bool pin = 1);

    // A structure filled in if LookupFont succeeds. Combines a font
    // resource with MovieDef which it came from. It is important
    // for users to keep a reference to both for as long as font is used.
    class FontResult
    {
        friend class FontLib;
        // These should be GPtrs, but they are not for compiler compatibility.
        MovieDef*       pMovieDef;
        FontResource*   pFontResource;
        void            SetResult(MovieDef* pmovieDef, FontResource* pfont);
    public:
        FontResult() { pMovieDef = 0; pFontResource = 0; }
        ~FontResult();                   

        MovieDef*     GetMovieDef() const { return pMovieDef; }
        FontResource* GetFontResource() const { return pFontResource; }
    };

    // Finds or creates a font of specified style by searching all of the
    // registered MovieDef objects. The font resource is created to be compatible
    // with puserDef's binding states and utilizing pstates for non-binding
    // state values.
    // fontFlags style as described by GFxFont::FontFlags
    virtual bool        FindFont(FontResult *presult,
                                 const char* name, unsigned fontFlags,
                                 MovieDef* puserDef, StateBag *pstates, 
                                 ResourceWeakLib* pweakLib = NULL);

    // Checks if all font movies are completely loaded
    bool                AreAllMoviesLoaded() const;

    // Loads all available font names into destination array
    void                LoadFontNames(StringHash<String>& fontnames);
};



// *** External class inlines dependent on us

inline void StateBag::SetFontLib(FontLib* pri)
{
    SetState(State::State_FontLib, pri);
}
inline Ptr<FontLib>  StateBag::GetFontLib() const
{
    return *(FontLib*) GetStateAddRef(State::State_FontLib);
}

}} // Scaleform::GFx

#endif
