/**********************************************************************

Filename    :   D3D9_ShaderManager.h
Content     :   Shader Manager - tracks vertex/pixel shaders and
                declarations.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_D3D9_ShaderManager_H
#define INC_SF_Render_D3D9_ShaderManager_H

#include "Render/Render_HAL.h"
#include "Render\Render_Primitive.h"
#include "Render\Render_Containers.h"
#include "D3D9_Texture.h"
#include "D3D9_MeshCache.h"
#include <d3d9.h>

namespace Scaleform { namespace Render { 
	
// Declare D3DVERTEXELEMENT9 to use it in MultiKeyCollection
inline bool operator == (const D3DVERTEXELEMENT9& e0, const D3DVERTEXELEMENT9& e1)
{   
	// TODO: this logic is opposite that of the X360, which seems correct.
	return memcmp(&e0, &e1, sizeof(D3DVERTEXELEMENT9)) != 0;
}


namespace D3D9 {

#define SF_RENDERER_SHADER_VERSION  0x0200
#define SF_RENDERER_VSHADER_PROFILE "vs_1_1"
    //#define SF_RENDERER_PSHADER_PROFILE "ps_2_0"


// *** Vertex Shader / Vertex Declaration constansts

enum VSGroupMode
{
    VSGroup_Single      = 0x0000,
    VSGroup_Batch       = 0x0100,
    VSGroup_Instanced   = 0x0200,
    VSGroup_Mask        = 0x0300
};
enum VSColorInput
{
    VSColor_None        = 0x0000,
    VSColor_Solid       = 0x0400,
    VSColor_Vertex      = 0x0800,
    VSColor_Mask        = 0x0C00,
};
enum VSTextureInput
{
    VSTexture_None      = 0x0000,
    VSTexture_Gen1      = 0x1000,
    VSTexture_Gen2      = 0x2000,
    VSTexture_UV1       = 0x3000,
    VSTexture_Mask      = 0xF000,
};

// Generate VertexElement
enum VertexElementBits
{    
    // Elements of VertexFormat
    VFE_IIndex          = 0x0001, // Instance index
    VFE_EAlpha          = 0x0002,
    VFE_Weight          = 0x0004,
    VFE_VColor          = 0x0008, 
    VFE_UV32f           = 0x0010,

    VFE_Factors_Mask    = VFE_EAlpha | VFE_Weight | VFE_IIndex,
    VFE_Mask            = 0x00FF,
    
    VSBit_Factor        = 0x10000
};


// VertexShaderDesc describes the features and type of vertex shader to use.
struct VertexShaderDesc
{
    UInt32  Flags;

    VSGroupMode     GetGroupMode() const    { return (VSGroupMode)(Flags & VSGroup_Mask); }
    VSColorInput    GetColorInput() const   { return (VSColorInput)(Flags & VSColor_Mask); }
    VSTextureInput  GetTextureInput() const { return (VSTextureInput)(Flags & VSTexture_Mask); }
    bool            HasFactors() const      { return (Flags & VFE_Factors_Mask) != 0; }

    unsigned        GetVertexShaderKey()
    {
        unsigned key  = Flags & (VSGroup_Mask | VSColor_Mask | VSTexture_Mask);
        if (HasFactors())
            key |= VSBit_Factor;
        return key;
    }
};


// Pixel shader texture key bit flags. These bits are used to both generate and uniquely
// identify a pixel shader; note that they don't map one-to-one to PSTypes, as some
// combinations are not needed.
enum PSKeyBits
{
    // Low 8 bits, always created.
    // In addition to being used as a part of PShader key, there 3 bits
    // also index ShaderManager::PSTable.
    PSBit_Multiply    = 0x0001,
    PSBit_AlphaWrite  = 0x0002,
    PSBit_Instanced   = 0x0004,
    PSBit_LowBitCount = 3,  
    // Shader has EAlpha stage after cxform (requires factor).
    PSBit_EAlpha      = 0x0008,
    // PSFillOP index occupies the higher bits, identifies operation used
    // to assign color from texture(s).
    PSBit_FillOp_Mask = 0x00F0,
    PSBit_FillOp_Shift= 4,
};


// PSType describes the significantly-different types of pixel shaders used in
// renderer and indexable through ShaderManager::PStable.
// For each type there are 8 versions matching low PSBit values
// (Multiply, AlphaWrite, Instanced).
enum PSType
{
    PS_None,
    PS_NoColor, // Color = 1.0, used for masks.
    PS_Color,
    PS_Color_EAlpha,
    PS_Texture,
    PS_Texture_EAlpha,
    PS_Texture_VColor,
    PS_Texture_VColor_EAlpha,
    PS_2Texture,
    PS_2Texture_EAlpha,
    PS_TextureA_VColor,
    PS_TextureYUV,
    PS_TextureYUVA,
    PS_Type_Count
};


// ShaderManager - maintains shaders and declarations needed for rendering with 
// different fill styles and includes methods for applying them to the device.

class ShaderManager
{
    IDirect3DDeviceX*   pDevice;
    const ProfileViews* Profiler;
    // Set if device supports instancing (vs_3_0 HW only)
    bool                InstancingSupport;

    struct VertexHandler
    {
        const VertexFormat *         pFormat[2];
        IDirect3DVertexDeclaration9* pDecl[2];
        IDirect3DVertexShader9*      pVShader;
        bool                         VShaderInit;

        VertexHandler()
        {
            pFormat[0] = pFormat[1] = 0;
            pDecl[0] = pDecl[1] = 0;
            pVShader = 0;
            VShaderInit = false;
        }
    };

    enum {
        PSTable_PSTypeShift     = 3,
        PSTable_ElementsPerType = 1 << PSTable_PSTypeShift,
        PSTable_Size            = PS_Type_Count * PSTable_ElementsPerType,
        PSTable_InstancedBit    = PSBit_Instanced
    };

    // VHandlerTable for each type { Single, Batch, Instanced }
    VertexHandler           VHandlerTable[PrimFill_Type_Count  * 3];
    // Pixels shaders
    IDirect3DPixelShader9*  PSTable[PSTable_Size];
    // Used in delayed shader binding: 'true' if shader was created
    bool                    PSInitTable[PSTable_Size];

    // Number of textures currently applied to device
    DWORD                   AppliedTextureCount;

    // PSType computed by ApplyPrimitiveFill
    PSType                  PrimitivePSType;

    // Information about previously applied texture to avoid redundant state apply.
    struct PrevTextureDesc
    {
        Texture*        pTexture;
        ImageFillMode   Fill;
        DWORD           Stage;
        void    Clear() { pTexture = 0; }
        void    Set(Texture* tex, ImageFillMode fill, DWORD stage = 0)
        { pTexture = tex; Fill = fill; Stage = stage; }
        bool    Match(Texture* tex, ImageFillMode fill) const
        { return (pTexture == tex) && (Fill == fill); }
        bool    Match(Texture* tex, ImageFillMode fill, DWORD stage) const
        { return (pTexture == tex) && (Fill == fill) && (Stage == stage); }
    };

    IDirect3DVertexDeclaration9* pPrevVDecl;
    IDirect3DVertexShader9*      pPrevVShader;
    unsigned                     PrevPSIndex;
    PrevTextureDesc              PrevTextures[2];

    MultiKeyCollection<D3DVERTEXELEMENT9,
                       Ptr<IDirect3DVertexDeclaration9>, 32>   VDecls;
    MultiKeyCollection<VertexElement, VertexFormat, 32>        VFormats;
    MultiKeyCollection<unsigned, Ptr<IDirect3DVertexShader9> > VShaders;
    MultiKeyCollection<unsigned, Ptr<IDirect3DPixelShader9> >  PShaders;


    bool            createVertexDeclaration(Ptr<IDirect3DVertexDeclaration9> *pdeclPtr,
                                            const D3DVERTEXELEMENT9 *pvertexElements);
    bool            createVertexShader(Ptr<IDirect3DVertexShader9> *pshaderPtr,
                                       const char* pshaderText);
    bool            createPixelShader(Ptr<IDirect3DPixelShader9> *pshaderPtr,
                                      const char* pshaderText);
    
    static bool     createVertexShaderString(StringBuffer& sb, VertexShaderDesc& desc);
    static bool     createPixelShaderString(StringBuffer& sb, unsigned psBits);


    IDirect3DVertexShader9*      registerVertexShader(VertexShaderDesc& desc);
    IDirect3DPixelShader9*       registerPixelShader(unsigned psBits);
    IDirect3DVertexDeclaration9* registerVertexDeclaration(VertexShaderDesc& desc, bool posFloat = false);
    const VertexFormat*          registerVertexFormat(VertexShaderDesc& desc, bool posFloat = false);

public:
    void  applySolidColorConstant(Color c)
    {
        c = Profiler->GetColor(c);
        const float mult  = 1.0f / 255.0f;
        const float alpha = c.GetAlpha() * mult;
        float       rgba[4] = { c.GetRed() * mult,  c.GetGreen() * mult, c.GetBlue() * mult, alpha };
        pDevice->SetVertexShaderConstantF(0, rgba, 1);
    }

public:
    ShaderManager();
    ~ShaderManager();

    bool   Initialize(IDirect3DDeviceX* pdevice, const D3DCAPSx& caps, const ProfileViews* profiler);
    void   Reset();

    bool    HasInstancingSupport() const { return InstancingSupport; }

    void    BeginScene();
    void    EndScene();

    // Applies the part of fill state that is shared between primitive batches.
    //  - Texture state is generally shared
    //  - If Apply returns false, rendering shouldn't take place
    bool   ApplyPrimitiveFill(PrimitiveFill* pfill);
    // Applies Primitive-Batch specific fills states.
    //  - pformat is passed since if is different from the one in pfill, due to being "Mapped"
    //    TBD: Can/should we map it as a part of pfill?
    bool   ApplyPrimitiveBatchFill(PrimitiveFillType fillType, UInt16 fillFlags,
                                   const VertexFormat* pformat, PrimitiveBatch::BatchType type);

    bool   ApplyClearMaskFill();
    bool   ApplySolidColorFill(Color c);

    // Obtains formats that renderer will use for single, batches and instanced rendering of
    // the specified source format.
    //   - Filled in pointer may be the same as sourceFormat.
    //   - 'instanced' format may be reported as 0,
    //     in which instancing is not supported for format.
    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
                            const VertexFormat** single,
                            const VertexFormat** batch, const VertexFormat** instanced);

};


}}} // Scaleform::Render::D3D9

#endif
