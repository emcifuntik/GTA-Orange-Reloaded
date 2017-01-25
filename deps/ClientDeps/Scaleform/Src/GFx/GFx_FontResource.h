/**********************************************************************

Filename    :   FontResource.h
Content     :   Representation of SWF/GFX font data and resources.
Created     :   
Authors     :   

Copyright   :   (c) 2001-2006 Scaleform Corp. All Rights Reserved.

Notes       :   

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

**********************************************************************/

#ifndef INC_SF_GFX_FontResource_H
#define INC_SF_GFX_FontResource_H

#include "GFx/GFx_Types.h"
#include "Kernel/SF_Array.h"
//#include "Render/Render_Renderer.h"

//#include "Render/Render_ImageInfo.h"

#include "GFx/GFx_Player.h"
#include "GFx/GFx_Loader.h"

#include "GFx/GFx_ResourceHandle.h"
#include "GFx/GFx_ImageResource.h"

#include "GFx/GFx_Tags.h"

#include "Render/Render_Font.h"

#include "GFx/GFx_FontCompactor.h"

namespace Scaleform { 

class File;

//using Render::ImageInfoBase;
//using Render::ImageInfo;

namespace GFx {

// ***** Declared Classes
class FontResource;
class FontData;
class TextureGlyph;
class TextureGlyphData;


// ***** External Classes
class MovieDataDef;
class ShapeDataBase;
class Stream;

class LoadProcess;

typedef Render::Font Font;

#ifndef GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM
#define GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM 32
#endif // GFX_FONT_CACHE_TEXTURE_SIZE_MINIMUM


// Class for Holding (cached) textured glyph info.
class TextureGlyph : public RefCountBaseNTS<TextureGlyph, StatMD_Fonts_Mem>
{
public:
    ResourcePtr<ImageResource>  pImage;
    RectF                       UvBounds;
    PointF                      UvOrigin;   // the origin of the glyph box, in uv coords

    TextureGlyph()
        {}
    ~TextureGlyph()
        { }
    TextureGlyph(const TextureGlyph &src)
        : RefCountBaseNTS<TextureGlyph, StatMD_Fonts_Mem>()
    {
        pImage      = src.pImage;
        UvBounds    = src.UvBounds;
        UvOrigin    = src.UvOrigin;
    }
    const TextureGlyph& operator = (const TextureGlyph &src)
    {
        pImage      = src.pImage;
        UvBounds    = src.UvBounds;
        UvOrigin    = src.UvOrigin;
        return *this;
    }
    
    
    // Sets image info to a resource.
    void            SetImageResource(ImageResource *pres)        { pImage = pres; }
    void            SetImageResource(const ResourceHandle& rh)   { pImage.SetFromHandle(rh); }

    // Return this texture glyph has an image resource assigned,
    // and can thus be used for rendering.
    bool            HasImageResource() const    { return !pImage.IsNull(); }

    //@IMG ImageInfoBase*  GetImageInfo(const ResourceBinding *pbinding) const;    
};



//
// *** TextureGlyphData
//

// TextureGlyphData stores a collection of all textures and glyphs that
// can be used while rendering a font. It can either (1) be generated  automatically
// during SWF binding, or (2) be loaded from the GFX file. In the first case,
// the only pointer to data will be stored in FontResource, in the second case it
// will also be stored in FontData, so that it can be shared.
//
// The 'gfxexport' tool can cause TextureGlyphData to transition from
// dynamically generated to file-loaded state.


class TextureGlyphData : public RefCountBaseNTS<TextureGlyphData, StatMD_Fonts_Mem>
{
public:
    typedef FontPackParams::TextureConfig TextureConfig;

private:
    // Describes texture packing settings used.
    TextureConfig  PackTextureConfig;

    typedef HashLH<ResourceId, ResourcePtr<ImageResource>,
                    FixedSizeHash<ResourceId> >                 ImageResourceHash;

    // Texture glyphs structure; either built up by tha font packer
    // or loaded from GFX file by tag loaders.
    ArrayLH<TextureGlyph>           TextureGlyphs;
    // Generated textureID -> texture.
    ImageResourceHash                   GlyphsTextures;

    // Set to 1 if info was loaded from GFX file, 0 if created dynamically.
    bool                FileCreation;
public:

    TextureGlyphData(unsigned glyphCount = 0, bool isLoadedFromFile = false)
    {
        FileCreation = isLoadedFromFile;
        TextureGlyphs.Resize(glyphCount);
    }

    // Obtain/modify variables that control how glyphs are packed in textures.
    void    GetTextureConfig(TextureConfig* pconfig) const { *pconfig = PackTextureConfig; }
    void    SetTextureConfig(const TextureConfig& config)  { PackTextureConfig   = config; }
  
    float   GetTextureGlyphScale() const;

    bool    IsLoadedFromFile() const { return FileCreation;  }


    // *** Texture assignment / enumeration interfaces

    unsigned                GetTextureGlyphCount() const { return (unsigned)TextureGlyphs.GetSize(); }
    const TextureGlyph&     GetTextureGlyph(unsigned glyphIndex) const;

    void                    WipeTextureGlyphs();
    void                    AddTextureGlyph(unsigned glyphIndex, const TextureGlyph& glyph);

    void                    AddTexture(ResourceId textureId, ImageResource* pimageRes);
    void                    AddTexture(ResourceId textureId, const ResourceHandle &rh);

    unsigned                GetConfigTextureWidth()  const { return PackTextureConfig.TextureWidth;  }
    unsigned                GetConfigTextureHeight() const { return PackTextureConfig.TextureHeight; }

    struct TextureGlyphVisitor
    {
        virtual ~TextureGlyphVisitor() {}
        virtual void Visit(unsigned index, TextureGlyph* ptextureGlyph) = 0;
    };
    void                    VisitTextureGlyphs(TextureGlyphVisitor*);
    
    struct TexturesVisitor
    {
        virtual ~TexturesVisitor() {}
        virtual void Visit(ResourceId textureId, ImageResource* pimageRes) = 0;
    };
    void                    VisitTextures(TexturesVisitor*, ResourceBinding* pbinding);
};



//
// *** FontData
//

// FontData contains font data that is loaded from an SWF/GFX file.
// This data is associated with MovieDataDef and is thus shared among
// all of the MovieDefImpl instances that can potentially be created.
// This means that FontData object can not cache any instance-specific
// information, such as information about texture size and packing.
//  
// In general, FontData stores the following types of information
//  - Font name and attributes
//  - Kerning, Adance and layout variables
//  - Code points identifying characters in the font
//  - pTextureGlyphData loaded from exported file (*)
//
//  (*) pTextureGlyphData is special in that it contains texture size and
//  image information normally associated with a binding state. This information
//  is stored here ONLY if is loaded from that file; in that case it is shared
//  among all instances. If TextureGlyphData is instead generated dynamically
//  based on FontPackParams setting, as happens during SWF loading, it will only be
//  stored within FontResource instances (FontData::pTextureGlyphData will be null).


class FontData : public Render::Font
{
public:
    FontData();
    FontData(const char* name, unsigned fontFlags);
    ~FontData();

    // *** GFX/SWF Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);
    void            ReadFontInfo(Stream* in, TagType tagType);
   
    
    // *** Font implementation
    
    TextureGlyphData*       GetTextureGlyphData() const { return pTextureGlyphData;  }

    virtual int             GetGlyphIndex(UInt16 code);
//     virtual bool            IsHintedVectorGlyph(unsigned, unsigned) const { return false; }
//     virtual bool            IsHintedRasterGlyph(unsigned, unsigned) const { return false; }
    virtual float           GetAdvance(unsigned glyphIndex) const;
    virtual float           GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    virtual int             GetCharValue(unsigned glyphIndex) const;

    virtual unsigned        GetGlyphShapeCount() const      { return (unsigned)Glyphs.GetSize(); }

    virtual float           GetGlyphWidth(unsigned glyphIndex) const;
    virtual float           GetGlyphHeight(unsigned glyphIndex) const;
    virtual RectF&          GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    // Slyph/State access.
    virtual const char*     GetName() const         { return Name; }

    // Only used for diagnostic purpose
    virtual String          GetCharRanges() const;

    virtual const Render::ShapeDataInterface* GetPermanentGlyphShape(unsigned glyphIndex) const;
     
    // *** Other APIs

    virtual bool            HasVectorOrRasterGlyphs() const { return CodeTable.GetSize() != 0; }    
    
    // TextureGlyphData access - only available if loaded from file.   
    void                    SetTextureGlyphData(TextureGlyphData* pdata)
    {
        SF_ASSERT(pdata->IsLoadedFromFile());
        pTextureGlyphData = pdata;
    }
private:
    void    ReadCodeTable(Stream* in);
    
    // General font information. 
    char*                   Name;

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    Ptr<TextureGlyphData>   pTextureGlyphData;
    
    // If Glyph shapes are loaded and not stripped, they are stored here.    
    ArrayLH<Ptr<ShapeDataBase> > Glyphs;


    // This table maps from Unicode character number to glyph index.
    // CodeTable[CharacterCode] = GlyphIndex    
    HashIdentityLH<UInt16, UInt16> CodeTable;

    // Layout stuff.
    // Note that Leading, Advance and Descent are in the base class
    struct AdvanceEntry
    {
        float       Advance;
        SInt16      Left;   // offset by X-axis, in twips
        SInt16      Top;    // offset by Y-axis, in twips
        UInt16      Width;  // width, in twips
        UInt16      Height; // height, in twips
    };
    ArrayLH<AdvanceEntry>          AdvanceTable;
    
    struct KerningPair
    {           
        UInt16  Char0, Char1;
        bool    operator==(const KerningPair& k) const
            { return Char0 == k.Char0 && Char1 == k.Char1; }
    };
    HashLH<KerningPair, float, FixedSizeHash<KerningPair> >  KerningPairs;

    // @@ we don't seem to use this thing at all, so don't bother keeping it.
    // Array<Render::Renderer::Rect> BoundsTable;    // @@ this thing should be optional.
};


#ifdef GFX_ENABLE_FONT_COMPACTOR
//
// *** FontDataCompactedSwf
//
// This is used for DefineFont2 and DefineFont3 tags and when FontCompactorParams 
// state is set to the loader. It reads SWF font data and then convert it into more compact
// form.
class FontDataCompactedSwf : public Font
{
public:
    typedef FontCompactor::ContainerType ContainerType;
    typedef CompactedFont<ContainerType> CompactedFontType;

    FontDataCompactedSwf();
    ~FontDataCompactedSwf();

    // *** GFX/SWF Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);

    // *** Font implementation

    int                     GetGlyphIndex(UInt16 code) const;
    bool                    IsHintedVectorGlyph(unsigned, unsigned) const { return false; }
    bool                    IsHintedRasterGlyph(unsigned, unsigned) const { return false; }
    ShapeDataBase*              GetGlyphShape(unsigned glyphIndex, unsigned glyphSize);
    GlyphRaster*            GetGlyphRaster(unsigned, unsigned) { return 0; }
    float                   GetAdvance(unsigned glyphIndex) const;
    float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    int                     GetCharValue(unsigned glyphIndex) const;

    unsigned                GetGlyphShapeCount() const;

    float                   GetGlyphWidth(unsigned glyphIndex) const;
    float                   GetGlyphHeight(unsigned glyphIndex) const;
    RectF&             GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    // Slyph/State access.
    virtual const char*     GetName() const;

    // Only used for diagnostic purpose
    virtual String         GetCharRanges() const;

    TextureGlyphData*       GetTextureGlyphData() const { return pTextureGlyphData;  }
    // TextureGlyphData access - only available if loaded from file.   
    void                    SetTextureGlyphData(TextureGlyphData* pdata)
    {
        SF_ASSERT(pdata->IsLoadedFromFile());
        pTextureGlyphData = pdata;
    }
    // *** Other APIs

    bool                    HasVectorOrRasterGlyphs() const;

    const ContainerType& GetContainer () const {return Container;}
    const CompactedFontType& GetCompactedFont() const { return CompactedFontValue; }

private:

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    Ptr<TextureGlyphData>   pTextureGlyphData;
 
    ContainerType           Container;
    CompactedFontType       CompactedFontValue;
    unsigned                NumGlyphs;
};
#endif //#ifdef GFX_ENABLE_FONT_COMPACTOR

#ifdef GFX_ENABLE_COMPACTED_FONTS
//
// *** FontDataCompactedGfx

//
// This is used for DefineCompactedFont tag. This tag is created
// by gfxexport when font is converted into a compact form.

class FontDataCompactedGfx : public Font
{
public:
    typedef ArrayUnsafeLH_POD<UByte, StatMD_Fonts_Mem> ContainerType;
    typedef CompactedFont<ContainerType> CompactedFontType;

    FontDataCompactedGfx();
    ~FontDataCompactedGfx();

    // *** GFX Loading methods.
    void            Read(LoadProcess* p, const TagInfo& tagInfo);

    // *** Font implementation

    int                     GetGlyphIndex(UInt16 code) const;
    bool                    IsHintedVectorGlyph(unsigned, unsigned) const { return false; }
    bool                    IsHintedRasterGlyph(unsigned, unsigned) const { return false; }
    ShapeDataBase*              GetGlyphShape(unsigned glyphIndex, unsigned glyphSize);
    GlyphRaster*            GetGlyphRaster(unsigned, unsigned) { return 0; }
    float                   GetAdvance(unsigned glyphIndex) const;
    float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const;

    int                     GetCharValue(unsigned glyphIndex) const;

    unsigned                GetGlyphShapeCount() const;

    float                   GetGlyphWidth(unsigned glyphIndex) const;
    float                   GetGlyphHeight(unsigned glyphIndex) const;
    RectF&             GetGlyphBounds(unsigned glyphIndex, RectF* prect) const;

    // Slyph/State access.
    virtual const char*     GetName() const;

    // Only used for diagnostic purpose
    virtual String          GetCharRanges() const;

    TextureGlyphData*       GetTextureGlyphData() const { return pTextureGlyphData;  }
    // TextureGlyphData access - only available if loaded from file.   
    void                    SetTextureGlyphData(TextureGlyphData* pdata)
    {
        SF_ASSERT(pdata->IsLoadedFromFile());
        pTextureGlyphData = pdata;
    }

    // *** Other APIs

    bool                    HasVectorOrRasterGlyphs() const;

    const ContainerType& GetContainer () const {return Container;}
    const CompactedFontType& GetCompactedFont() const { return CompactedFontValue; }

private:

    // We store texture glyph data only if it comes from a file. Dynamically
    // created texture glyphs are accessible only through FontResource.
    Ptr<TextureGlyphData>  pTextureGlyphData;

    ContainerType     Container;
    CompactedFontType CompactedFontValue;
};
#endif //#ifdef GFX_ENABLE_COMPACTED_FONTS

//
// *** FontResource
//
// FontResource is a font-instance resource that delegates a lot of its functionality
// to FontData and GFxTextureData.


class FontResource : public Resource
{
public:
    typedef Render::Font::FontFlags FontFlags;
    
    // We store fonts binding within font resource for convenient access; this
    // binding is used to look up image resources from TextureGlyphData. It is ok
    // because separate FontResource is always created for every MovieDefImpl.
    // Note that the pbinding can also be null for system fonts.
    FontResource(Font *pfont, ResourceBinding* pbinding);
    // For system provider fonts we pass a key.
    FontResource(Font *pfont, const ResourceKey& key);
    ~FontResource();


    
    Font*               GetFont()    const { return pFont; }
    ResourceBinding*    GetBinding() const { return pBinding; }

    
    // *** Handler List support.

//     // Handlers are used to allow font cache manager to
//     // detect when fonts are released.        
//     class DisposeHandler
//     {
//     public:
//         virtual ~DisposeHandler() {}
//         virtual void OnDispose(FontResource*) = 0;
//     };
// 
//     void                    AddDisposeHandler(DisposeHandler* h);
//     void                    RemoveDisposeHandler(DisposeHandler* h);


    // *** Delegates to Font

    // implement FontData.States = *pFont;

    bool                    MatchFont(const char* name, unsigned matchFlags) const { return pFont->MatchFont(name, matchFlags); }
    bool                    MatchFontFlags(unsigned matchFlags) const              { return pFont->MatchFontFlags(matchFlags); }
    
    inline const char*      GetName() const                     { return pFont->GetName(); }
//     inline unsigned         GetGlyphShapeCount() const          { return pFont->GetGlyphShapeCount(); }

//     TextureGlyphData*       GetTextureGlyphData() const          { return pTGData.GetPtr(); }
//     ShapeDataBase*              GetGlyphShape(unsigned glyphIndex, unsigned glyphSize) const 
//     { 
//         return pFont->GetGlyphShape(glyphIndex, glyphSize); 
//     }
//     GlyphRaster*            GetGlyphRaster(unsigned glyphIndex, unsigned glyphSize) const 
//     {
//         return pFont->GetGlyphRaster(glyphIndex, glyphSize); 
//     }

//     int                     GetGlyphIndex(UInt16 code) const     { return pFont->GetGlyphIndex(code); }
//     bool                    IsHintedVectorGlyph(unsigned glyphIndex, unsigned glyphSize) const { return pFont->IsHintedVectorGlyph(glyphIndex, glyphSize); } 
//     bool                    IsHintedRasterGlyph(unsigned glyphIndex, unsigned glyphSize) const { return pFont->IsHintedRasterGlyph(glyphIndex, glyphSize); }
//     float                   GetAdvance(unsigned glyphIndex) const    { return pFont->GetAdvance(glyphIndex); }
//     float                   GetKerningAdjustment(unsigned lastCode, unsigned thisCode) const { return pFont->GetKerningAdjustment(lastCode, thisCode);  }
//     float                   GetGlyphWidth(unsigned glyphIndex) const  { return pFont->GetGlyphWidth(glyphIndex); }
//     float                   GetGlyphHeight(unsigned glyphIndex) const { return pFont->GetGlyphHeight(glyphIndex);  }
//     RectF&                  GetGlyphBounds(unsigned glyphIndex, RectF* prect) const { return pFont->GetGlyphBounds(glyphIndex, prect);  }
//     float                   GetLeading() const                   { return pFont->GetLeading(); }
//     float                   GetDescent() const                   { return pFont->GetDescent(); }
//     float                   GetAscent() const                    { return pFont->GetAscent(); }
// 
//     inline unsigned         GetFontFlags() const                { return pFont->GetFontFlags(); }
//     inline unsigned         GetFontStyleFlags() const           { return pFont->GetFontStyleFlags(); }
//     inline unsigned         GetCreateFontFlags() const          { return pFont->GetCreateFontFlags(); }
//     inline bool             IsItalic() const                    { return pFont->IsItalic(); } 
//     inline bool             IsBold() const                      { return pFont->IsBold(); } 
//     inline bool             IsDeviceFont() const                { return pFont->IsDeviceFont(); } 
//     inline bool             HasNativeHinting() const            { return pFont->HasNativeHinting(); }
// 
//     inline bool             AreUnicodeChars() const             { return pFont->AreUnicodeChars(); } 
//     inline FontFlags        GetCodePage() const                 { return pFont->GetCodePage(); } 
//     inline bool             GlyphShapesStripped() const         { return pFont->GlyphShapesStripped(); }
//     inline bool             HasLayout() const                   { return pFont->HasLayout(); }     
//     inline bool             AreWideCodes() const                { return pFont->AreWideCodes(); }     
//     inline bool             IsPixelAligned() const              { return pFont->IsPixelAligned(); }

    inline UInt16 GetLowerCaseTop(Log* log)
    { 
        if (LowerCaseTop == 0)
            calcLowerUpperTop(log);
        return (LowerCaseTop <= 0) ? 0 : LowerCaseTop; 
    }
    inline UInt16 GetUpperCaseTop(Log* log) 
    { 
        if (UpperCaseTop == 0)
            calcLowerUpperTop(log);
        return (UpperCaseTop <= 0) ? 0 : UpperCaseTop; 
    }

    // *** Delegates to TextureGlyphData

    typedef FontPackParams::TextureConfig TextureConfig;

    // TBD: Necessary?

    inline void GetTextureConfig(TextureConfig* pconfig) const { if (pTGData) pTGData->GetTextureConfig(pconfig); }

    // TextureGlyphData access
    void                    SetTextureGlyphData(TextureGlyphData* pdata) { pTGData = pdata; }


    // *** Resource implementation
    
    virtual unsigned        GetResourceTypeCode() const     { return MakeTypeCode(RT_Font); }
    virtual ResourceKey     GetKey()                        { return FontKey; }

    // Helper function used to lookup and/or create a font resource from provider.
    static FontResource*    CreateFontResource(const char* pname, unsigned fontFlags,
                                               FontProvider* pprovider, ResourceWeakLib *plib);
    // Creates a system font resource key.
    static ResourceKey      CreateFontResourceKey(const char* pname, unsigned fontFlags,
                                                  FontProvider* pfontProvider);

private:
    UInt16 calcTopBound(UInt16 code);
    void   calcLowerUpperTop(Log* log);

    Ptr<Font>               pFont;

    // Font texture glyphs data. This data is either generated dynamically based
    // on FontPackParams arguments or loaded from disc (in which case it is 
    // shared through pFont).
    Ptr<TextureGlyphData>   pTGData;

    // Pointer to font binding that should be used with embedded fonts; null for
    // system fonts. This pointer would be different in case of imports. It is the 
    // responsibility of the importing file (for direct imports) or font manager 
    // (for font provider lookups) to hold the MovieDefImpl for this binding.    
    ResourceBinding*        pBinding;

    // Font key - we store these only for provider-obtained fonts.
    ResourceKey             FontKey;
   
    // Handlers list data structures. 
//     bool                    HandlerArrayFlag;
//     union {
//         DisposeHandler*             pHandler;
//         Array<DisposeHandler*> *    pHandlerArray;  
//     };

    // Values used for pixel-grid fitting. Defined as the
    // top Y coordinate for 'z' and 'Z' respectively
    SInt16 LowerCaseTop;
    SInt16 UpperCaseTop;
};

}} // Scaleform::GFx

#endif 
