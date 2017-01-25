/**********************************************************************

Filename    :   Text/Text_StyleSheet.h
Content     :   StyleSheet parser
Created     :   May 20, 2008
Authors     :   Prasad Silva

Notes       :   
History     :   

Copyright   :   (c) 1998-2008 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_StyleSheet_H
#define INC_SF_GFX_StyleSheet_H

#include "Render/Text/Text_StyledText.h"

namespace Scaleform { 
namespace GFx {
namespace Text {

using Render::Text::StyleManagerBase;

class StyleManager;

#ifdef GFX_ENABLE_CSS

// 
// Forward declaration
//
class StyleParserHandlerBase;

//
// Hash key for CSS style objects
//
struct StyleKey
{
    typedef StyleManagerBase::KeyType KeyType;

    KeyType     Type;
    StringLH    Value;
    UPInt       HashValue;

    StyleKey() : Type(StyleManagerBase::CSS_None) { }

    StyleKey(KeyType t, const String& val)
        : Type(t), Value(val) 
    {
        // case sensitive 
        HashValue = String::BernsteinHashFunction(val.ToCStr(), val.GetSize());
        HashValue += Type;
    }

    void    Set(KeyType t, const String& val)
    {
        Type  = t;
        Value = val;
        HashValue = String::BernsteinHashFunction(val.ToCStr(), val.GetSize());
        HashValue += Type;
    }

};

bool operator== (const StyleKey& key1, const StyleKey& key2);


//
// Hash functions
//
template<class C>
class StyleHashFunc
{
public:
    typedef C ValueType;

    // Hash code is stored right in the key
//     UPInt  operator() (const C& data) const
//     {
//         return data->HashValue;
//     }

    UPInt   operator() (const C &str) const
    {
        return str.HashValue;
    }

    // Hash update - unused.
    static UPInt    GetCachedHash(const C& data)                { return data->HashValue; }
    static void     SetCachedHash(C& data, UPInt hashValue)     { SF_UNUSED2(data, hashValue); }
    // Value access.
    static C&       GetValue(C& data)                           { return data; }
    static const C& GetValue(const C& data)                     { return data; }

};


//
// Text style hash set 
//
// Keeps track of all text styles currently existing in the manager.
//
typedef HashUncachedLH<StyleKey, Style*,
    StyleHashFunc<StyleKey>, StatMV_Text_Mem> StyleHash;


//
// CSS style sheet object
//
// Manages a set of text stylse
//
class StyleManager : public Render::Text::StyleManagerBase
{
    friend class TextStyleParserHandlerBase;

public:
    StyleManager() {}
    ~StyleManager();

    bool ParseCSS(const char* buffer, UPInt len);
    bool ParseCSS(const wchar_t* buffer, UPInt len);

    const StyleHash& GetStyles() const;
    virtual const Style*    GetStyle(KeyType type, const String& name) const;
    virtual const Style*    GetStyle(KeyType type, const char* name, UPInt len = SF_MAX_UPINT) const;
    virtual const Style*    GetStyle(KeyType type, const wchar_t* name, UPInt len = SF_MAX_UPINT) const;

    void                    AddStyle(const StyleKey& key, Style* pstyle);

    void                    ClearStyles();
    void                    ClearStyle(KeyType type, const String& name);
    void                    ClearStyle(KeyType type, const char* name, UPInt len = SF_MAX_UPINT);

    // Obtains a temp key used for look; this is use to make sure that key is allocated
    // on the correct local heap.
    const StyleKey&         GetTempStyleKey(StyleKey::KeyType t, const String& val) const
    {
        TempKey.Set(t, val);
        return TempKey;
    }

private:
    StyleHash         Styles; 
    
    // Store key here to have a temporary for style lookup.
    mutable StyleKey  TempKey;

    template <typename Char>
    bool ParseCSSImpl(const Char* buffer, UPInt len);
};

#else

// Just a stub
class StyleManager : public Render::Text::StyleManagerBase
{
public:
};

#endif // SF_NO_CSS_SUPPORT

}}} // Scaleform::GFx::Text

#endif // INC_SF_GFX_StyleSheet_H

