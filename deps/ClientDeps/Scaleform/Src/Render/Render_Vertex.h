/**********************************************************************

PublicHeader:   Render
Filename    :   Context.h
Content     :   Vertex Element declarations 
Created     :   August 17, 2009
Authors     :   Michael Antonov

Notes       :   
History     :   

Copyright   :   (c) 2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

For information regarding Commercial License Agreements go to:
online - http://www.scaleform.com/licensing.html or
email  - sales@scaleform.com 

**********************************************************************/

#ifndef INC_SF_Render_Vertex_H
#define INC_SF_Render_Vertex_H

// This file provides Vertex Element declarations representing different vertex types,
// both on the renderer input and internal sides. 

#include "Kernel/SF_RefCount.h"
#include "Render_Matrix2x4.h"
#include "Render_Color.h"

namespace Scaleform { namespace Render {

// TBD: We might want to have a flag that indicates whether precision
// can be dropped to lower resolution or not (?). For example, it can be
// used to determine whether 16-bit integer indices are ok or not. This
// result depend on matrix scale (similar to mesh quality).

enum VertexElementType
{
    // If this flag is set, all the vertex elements of this type will
    // be assigned the same value from the argument type.
    VET_Argument_Flag   = 0x10000,

    VET_Type_Mask       = 0xffff,
    VET_Components_Mask = 0xf,
    VET_CompType_Mask   = 0xf0,
    VET_CompType_Shift  = 4,
    VET_Usage_Mask      = 0xf00,
    VET_Usage_Shift     = 8,
    VET_Index_Mask      = 0xf000,
    VET_Index_Shift     = 12,

    VET_U8N             = 0x10,
    VET_U8              = 0x20,
    VET_S16             = 0x30,
    VET_U16             = 0x40,
    VET_U32             = 0x50,
    VET_F32             = 0x60,

    VET_None            = 0,
    VET_Pos             = 0x100,
    VET_Color           = 0x200,
    VET_TexCoord        = 0x300,
    VET_Instance        = 0x400,

    VET_XY16i           = VET_Pos      | VET_S16 | 2,
    VET_XYZ16i          = VET_Pos      | VET_S16 | 3,
    VET_XY32f           = VET_Pos      | VET_F32 | 2,
    VET_XYZ32f          = VET_Pos      | VET_F32 | 3,
    VET_UV32f           = VET_TexCoord | VET_F32 | 2,
    VET_ColorRGBA8      = VET_Color    | VET_U8N | 4,
    VET_ColorARGB8      = VET_Color    | VET_U32 | 1,
    VET_FactorAlpha8    = VET_Color    | VET_U8N | 1 | (1 << VET_Index_Shift),
    VET_T0Weight8       = VET_Color    | VET_U8N | 1 | (2 << VET_Index_Shift),

    VET_Instance8       = VET_Instance | VET_U8  | 1 | VET_Argument_Flag
};

static unsigned VertexTypeSizes[] = {1,1,2,2,4,4};

struct VertexElement
{
    unsigned Offset;
    unsigned Attribute;

    inline unsigned CompSize() const
    {
        return VertexTypeSizes[((Attribute & VET_CompType_Mask) >> VET_CompType_Shift) - 1];
    }

    inline unsigned Size() const
    {
        return (Attribute & VET_Components_Mask) * CompSize();
    }

    inline bool operator == (const VertexElement& o) const
    {
        return (Offset == o.Offset) && (Attribute == o.Attribute);
    }
    inline bool operator != (const VertexElement& o) const
    {
        return !operator == (o);
    }
};


// TBD: Should we support vertex declarations with multiple streams; i.e.
// a color channel being specified in a separate array? Not for now.

// When vertex format is reported right now, it is expected to have global
// lifetime. Perhaps in the future this can have an allocated flag...

struct VertexFormat
{
    unsigned          Size;
    VertexElement*    pElements;

    bool operator == (const VertexFormat& o) const
    {
        if (Size != o.Size) return false;
        const VertexElement *e0, *e1;
        for (e0 = pElements, e1 = o.pElements; e0->Attribute != VET_None; e0++, e1++)
        {
            if (*e0 != *e1)
                return false;
        }
        return *e0 == *e1;
    }

    const VertexElement* GetElement(unsigned attrValue, unsigned attrMask) const
    {
         const VertexElement *e = pElements;
         while (e->Attribute != VET_None)
         {
             if ((e->Attribute & attrMask) == attrValue)
                 return e;
             e++;
         }
         return 0;
    }
    bool    HasUsage(VertexElementType usage) const
    {
        return GetElement(usage, VET_Usage_Mask) != 0;        
    }
};

// Converts vertices from the source to destination format based on format declarations.
// Each destination element that has VET_Argument_Flag flag set receives as extra
// argument from pargumentData array, which can be used to initialize it.
//   - arguments are consumed in the order of vertex elements, only when the VET_Argument_Flag is set.
void ConvertVertices(const VertexFormat& sourceFormat, void* psource,
                     const VertexFormat& destFormat, void* pdest,
                     unsigned count, void** pargumentData = 0);
// Same conversion as a above, but goes through a temporary buffer to minimize
// destination writes, which may be more efficient for video memory.
void ConvertVertices_Buffered(const VertexFormat& sourceFormat, void* psource,
                              const VertexFormat& destFormat, void* pdest,
                              unsigned count, void** pargumentData = 0);

// Initializes vertex argument channels to specified argument values.
// This function can be used to initialize instance values or other identical value
// elements in an array of vertices efficiently.
void InitializeVertices(const VertexFormat& format, void* pvertices,
                        unsigned count, void** pargs);

// Copies and converts index buffer by adding delta to each index.
// Uses non-cached write to destination if availabe on this platform.
void ConvertIndices(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);
void ConvertIndices_NonOpt(UInt16* pdest, UInt16* psource, unsigned count, UInt16 delta);



// **** Common Vertex Formats

// Declared for convenience here

// Vertex with coordinates only
//------------------------------------------------------------------------
struct VertexXY16i
{
    SInt16  x,y;

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with color
//------------------------------------------------------------------------
struct VertexXY16iC32
{
    SInt16  x,y;
    UInt32  Color;

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};


// Vertex with alpha
//------------------------------------------------------------------------
struct VertexXY16iAlpha
{
    SInt16  x,y;
    UInt8   Alpha[4];

    static VertexElement VertexElements[3];
    static VertexFormat  Format;
};

// Vertex with color and factors
//------------------------------------------------------------------------
struct VertexXY16iCF32
{
    SInt16  x,y;
    UInt32  Color;
    UInt8   Factors[4];

    static VertexElement VertexElements[5];
    static VertexFormat  Format;
};

// Vertex with instance index
//------------------------------------------------------------------------
struct VertexXY16iInstance
{
    SInt16  x,y;
    UInt8   Instance[4];

    static VertexElement VertexElements[4];
    static VertexFormat  Format;
};

}} // Scaleform::Render




// New Vertex formats:
//
//  Plain vertex: XY, i
//  Colored vertex: XY, C, i                    <--+ Merged?
//  Colored vertex with factors: XY, C, f, i    <--+
//
//  Colored vertex with texture: XY, UV, C, i

#endif
