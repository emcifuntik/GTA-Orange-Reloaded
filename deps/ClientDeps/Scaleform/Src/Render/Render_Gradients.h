/**********************************************************************

PublicHeader:   Render
Filename    :   Render_Gradients.h
Content     :   Gradient Generator
Created     :   February 2010
Authors     :   Maxim Shemanarev

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_Gradients_H
#define INC_SF_Render_Gradients_H

#include "Render_Image.h"
#include "Render_Matrix2x4.h"

namespace Scaleform { namespace Render {


//-----------------------------------------------------------------------
class GradientRecord
{
public:
    GradientRecord() : Ratio(0) {}
    GradientRecord(UByte r, Color c) : Ratio(r), ColorV(c) {}

    UByte   Ratio;
    Color   ColorV;

    unsigned GetHashValue() const
    {
        return Ratio ^ (ColorV.Raw ^ (ColorV.Raw >> 16));
    }

    bool operator == (const GradientRecord& other) const
    {
        return (Ratio == other.Ratio) && (ColorV == other.ColorV);
    }
    bool operator != (const GradientRecord& other) const
    {
        return !operator == (other);
    }
};

//-----------------------------------------------------------------------
enum GradientType
{
    GradientLinear,
    GradientRadial,
    GradientFocalPoint  // Radial with a focal point
};

// Hash-able gradient descriptor - contains all of the info
// about the gradient. 
//-----------------------------------------------------------------------
// TO DO: Revise Stat_ID
class GradientData : public RefCountBase<GradientData, StatGroup_GFxMovieData>
{
    enum { DefaultImageSize = 64 };

public:
    GradientData(GradientType type, UInt16 recordCount = 0, bool linearRgb = 0);
    ~GradientData();

    bool            SetRecordCount(UInt16 count);
    unsigned        GetRecordCount() const      { return RecordCount; }

    const GradientRecord* GetRecords() const    { return pRecords; }

    void            SetFocalRatio(float ratio)  { FocalRatio = ratio; }
    float           GetFocalRatio() const       { return FocalRatio; }

    bool            IsLinearRGB() const         { return LinearRGB; }
    void            SetLinearRGB(bool lrgb)     { LinearRGB = lrgb; }

    GradientType    GetGradientType() const     { return (GradientType)Type; }

    // Calculate the image size on the basis of the gradient records.
    // See more comments in Render_Gradients.cpp 
    unsigned        CalcImageSize() const;

    GradientRecord& At(unsigned index)
    {
        SF_ASSERT(index < RecordCount);
        return pRecords[index];
    }
    const GradientRecord& At(unsigned index) const
    {
        SF_ASSERT(index < RecordCount);
        return pRecords[index];
    }

    // Compute hash code and equality; used for hashing.
    UPInt       GetHashValue() const;

    bool        operator == (const GradientData& other) const;

private:
    bool                LinearRGB;
    UByte               Type;       // Use UByte instead of GradientType for
                                    // potentially better data alignment
    UInt16              RecordCount;
    GradientRecord*     pRecords;
    float               FocalRatio;

    static unsigned const ImageSizeTable[];
};


//-----------------------------------------------------------------------
struct GradientKey
{
    GradientKey(const GradientData* data) : pData(data) {}

    UPInt   operator() () const { return pData->GetHashValue(); }

    const GradientData* pData;
};


//-----------------------------------------------------------------------
class PrimitiveFillManager;
class GradientImage : public Image
{
    enum { MaxImageSize = 256 };
public:
    GradientImage(PrimitiveFillManager* mng, GradientData* data = 0, unsigned texArena = 0);
    virtual ~GradientImage();

    virtual ImageType       GetImageType() const    { return Type_GradientImage; }
    virtual ImageFormat     GetFormat() const       { return Image_R8G8B8A8; }
    virtual ImageSize       GetSize() const         { return Size; }
    virtual unsigned        GetUse() const          { return 0; }
    virtual unsigned        GetMipmapCount() const  { return 1; }

    virtual bool            Decode(ImageData* pdest,
                                   CopyScanlineFunc copyScanline = CopyScanlineDefault,
                                   void* arg = 0) const;
    virtual Texture*        GetTexture(TextureManager* pmanager);

    const GradientData*     GetGradientData() const { return pData; }

    struct PtrHashFunctor
    {    
        UPInt  operator()(const GradientImage* gr) const
        { 
            return gr->GetGradientData()->GetHashValue(); 
        }
        UPInt  operator()(const GradientKey& data) const
        { 
            return data.pData->GetHashValue(); 
        }
    };

    bool operator == (const GradientImage& other) const
    { 
        return (*pData) == (*other.pData); 
    }
    SF_AMP_CODE(
        virtual UPInt   GetBytes() const { return 4 * Size.Area(); }
        virtual UPInt   IsExternal() const { return false; }
        virtual UInt32  GetImageId() const { return ImageId; }
    )
private:
    PrimitiveFillManager*   pManager;
    Ptr<GradientData>       pData;
    ImageSize               Size;
    unsigned                TextureArena;
    SF_AMP_CODE(UInt32 ImageId;)
};

inline bool operator == (const GradientImage* img, const GradientKey& key)
{
    return (*img->GetGradientData()) == (*key.pData);
}


// Helper functions to map gradient coordinates. See comments in .cpp
//-----------------------------------------------------------------------
Matrix2F LineToMatrix2D(float x1, float y1, float x2, float y2);
Matrix2F ParlToMatrix2D(float x1, float y1, float x2, float y2, float x3, float y3);

}} // Scaleform::Render

#endif // INC_SF_Render_Gradients_H

