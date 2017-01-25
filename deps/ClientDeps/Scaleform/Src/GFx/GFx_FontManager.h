/**********************************************************************

Filename    :   FontManager.h
Content     :   Font manager functionality
Created     :   May 18, 2007
Authors     :   Artyom Bolgar, Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 1998-2007 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_FONTMANAGER_H
#define INC_SF_GFX_FONTMANAGER_H

#include "Kernel/SF_UTF8Util.h"
#include "Kernel/SF_Array.h"
#include "GFx/GFx_FontResource.h"
#include "GFx/GFx_StringHash.h"
#include "GFx/GFx_FontLib.h"
#include "Render/Text/Text_FontManager.h"

namespace Scaleform { namespace GFx {

class FontManager;
class MovieDefImpl;

class FontHandle : public Render::Text::FontHandle
{
    friend class FontManager;    

public:
    // A strong pointer to MovieDefImpl from which the font was created.
    // Font manager needs to hold these pointers for those fonts which came
    // from the FontProvider, since our instance can be the only reference
    // to them. Null for all other cases.
    Ptr<MovieDef>   pSourceMovieDef;

    FontHandle(Render::Text::FontManagerBase *pmanager, Font *pfont,
               const char* pfontName = 0, unsigned overridenFontFlags = 0,
               MovieDef* pdefImpl = 0)
        : Render::Text::FontHandle(pmanager, pfont, pfontName, overridenFontFlags),
          pSourceMovieDef(pdefImpl)
    {
    }
    FontHandle(const FontHandle& f) : Render::Text::FontHandle(f),
        pSourceMovieDef(f.pSourceMovieDef)
    {
    }
    inline ~FontHandle();

    bool operator==(const FontHandle& f) const
    {
        return Render::Text::FontHandle::operator==(f) && 
               pSourceMovieDef == f.pSourceMovieDef;
    }
};


class FontManagerStates : public RefCountBaseNTS<FontManagerStates, StatMV_Other_Mem>, public StateBag
{
public:
    FontManagerStates(StateBag* pdelegate) : pDelegate(pdelegate) {}

    virtual StateBag*   GetStateBagImpl() const { return pDelegate; }

    virtual State*      GetStateAddRef(State::StateType state) const;

    UInt8               CheckStateChange(FontLib*, FontMap*, FontProvider*, Translator*);

    Ptr<FontLib>      pFontLib;
    Ptr<FontMap>      pFontMap;
    Ptr<FontProvider> pFontProvider;
    Ptr<Translator>   pTranslator;

private:
    StateBag*       pDelegate;
};

class FontManager : public Render::Text::FontManagerBase
{
    friend class FontHandle;    

protected:
    
    // Make a hash-set entry that tracks font handle nodes.
    struct FontKey 
    {
        const char*    pFontName;
        unsigned       FontStyle;

        FontKey() : FontStyle(0) {}
        FontKey(const char* pfontName, unsigned style)
            : pFontName(pfontName), FontStyle(style) { }

        static UPInt CalcHashCode(const char* pname, UPInt namelen, unsigned fontFlags)
        {
            UPInt hash = String::BernsteinHashFunctionCIS(pname, namelen);
            return (hash ^ (fontFlags & Font::FF_Style_Mask));
        }
    };

    struct NodePtr
    {
        FontHandle* pNode;
               
        NodePtr() { }
        NodePtr(FontHandle* pnode) : pNode(pnode) { }
        NodePtr(const NodePtr& other) : pNode(other.pNode) { }
        
        // Two nodes are identical if their fonts match.
        bool operator == (const NodePtr& other) const
        {
            if (pNode == other.pNode)
                return 1;
            // For node comparisons we have to handle DeviceFont strictly ALL the time,
            // because it is possible to have BOTH versions of font it the hash table.
            enum { FontFlagsMask = Font::FF_CreateFont_Mask | Font::FF_DeviceFont | Font::FF_BoldItalic};
            unsigned ourFlags   = pNode->GetFontFlags() & FontFlagsMask;
            unsigned otherFlags = other.pNode->GetFontFlags() & FontFlagsMask;
            return ((ourFlags == otherFlags) &&
                    !String::CompareNoCase(pNode->GetFontName(), other.pNode->GetFontName()));
        }        

        // Key search uses MatchFont to ensure that DeviceFont flag is handled correctly.
        bool operator == (const FontKey &key) const
        {
            return (Font::MatchFontFlags_Static(pNode->GetFontFlags(), key.FontStyle) &&
                    !String::CompareNoCase(pNode->GetFontName(), key.pFontName));
        }
    };
    
    struct NodePtrHashOp
    {                
        // Hash code is computed based on a state key.
        UPInt  operator() (const NodePtr& other) const
        {            
            FontHandle* pnode = other.pNode;
            const char* pfontName = pnode->GetFontName();
            return (UPInt) FontKey::CalcHashCode(pfontName, SFstrlen(pfontName), 
                pnode->pFont->GetFontFlags() | pnode->GetFontStyle());
        }
        UPInt  operator() (const FontHandle* pnode) const
        {
            SF_ASSERT(pnode != 0);            
            const char* pfontName = pnode->GetFontName();
            return (UPInt) FontKey::CalcHashCode(pfontName, SFstrlen(pfontName), 
                pnode->pFont->GetFontFlags() | pnode->GetFontStyle());
        }
        UPInt  operator() (const FontKey& data) const
        {
            return (UPInt) FontKey::CalcHashCode(data.pFontName, SFstrlen(data.pFontName), data.FontStyle);
        }
    };


    // State hash
    typedef HashSetLH<NodePtr, NodePtrHashOp, NodePtrHashOp> FontSet;

    // Keep a hash of all allocated nodes, so that we can find one when necessary.
    // Nodes automatically remove themselves when all their references die.
    FontSet                 CreatedFonts;

    // MovieDefImpl which this font manager is associated with. We look up
    // fonts here by default before considering pFontProvider. Note that this
    // behavior changes if device flags are passed.
    MovieDefImpl*           pDefImpl;
    // pWeakLib might be set and used in the case if pDefImpl == NULL
    ResourceWeakLib*        pWeakLib;
    // Shared state for the instance, comes from GFxMovieImpl. Shouldn't need 
    // to AddRef since nested sprite (and font manager) should die before MovieRoot.    
    FontManagerStates*      pState;


    // Empty fake font returned if requested font is not found.
    Ptr<FontHandle>         pEmptyFont; 

    // FontMap Entry temporary; here to avoid extra constructor/destructor calls.
    FontMap::MapEntry       FontMapEntry;

#ifndef SF_NO_IME_SUPPORT
    Ptr<FontHandle>      pIMECandidateFont;
#endif //#ifdef SF_NO_IME_SUPPORT

   
    // Helper function to remove handle from CreatedFonts when it dies.
    void                    RemoveFontHandle(FontHandle *phandle);

    FontHandle*             FindOrCreateHandle(const char* pfontName, unsigned matchFontFlags, 
                                                FontResource** ppfoundFont, FontSearchPathInfo* searchInfo);

    FontHandle*             CreateFontHandleFromName(const char* pfontName, unsigned matchFontFlags, 
                                                      FontSearchPathInfo* searchInfo);
private:
    void commonInit();
public:
    FontManager(MovieDefImpl *pdefImpl,
                   FontManagerStates* pState);
    FontManager(ResourceWeakLib *pweakLib,
                   FontManagerStates* pState);
    ~FontManager();


    // Returns font by name and style. If a non-NULL searchInfo passed to the method
    // it assumes that it was called only for a diagnostic purpose and in this case font
    // will not be searched in the internal cache and a created font handle will not be cached.
    virtual FontHandle*         CreateFontHandle(const char* pfontName, unsigned matchFontFlags, 
                                                 bool allowListOfFonts = true, FontSearchPathInfo* searchInfo = NULL); 

    // Returns any font with the font name 'pfontName' or the first one
    // in the hash.
    //FontHandle*          CreateAnyFontHandle(const char* pfontName, FontSearchPathInfo* searchInfo = NULL);

    virtual FontHandle*         GetEmptyFont();

    // Clean internal cache. This method is called from Advance method of 
    // MovieRoot then it detects that FontLib, FontMap, FontProvider or Translator 
    // is changed
    virtual void                CleanCache();

    void                        SetIMECandidateFont(FontHandle* pfont);

#ifndef SF_NO_IME_SUPPORT
#endif //#ifdef SF_NO_IME_SUPPORT

};


// Font handle inlined after font manager because it uses manager's APIs.
inline FontHandle::~FontHandle()
{        
    // Remove from hash.
    if (pFontManager)
        static_cast<FontManager*>(pFontManager)->RemoveFontHandle(this);

    // Must release pFont before its containing MovieDefImpl destructor,
    // since otherwise we'd be accessing a dying heap.
    pFont = 0;
}

}} // namespace Scaleform::GFx

#endif //INC_SF_GFX_FONTMANAGER_H
