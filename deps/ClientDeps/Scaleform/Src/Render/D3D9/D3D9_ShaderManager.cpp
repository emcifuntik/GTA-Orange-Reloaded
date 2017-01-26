/**********************************************************************

Filename    :   D3D9_ShaderManager.cpp
Content     :   Shader Manager - track vertex/pixel shaders and
                declarations.
Created     :   May 2009
Authors     :   Michael Antonov

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#include "Kernel/SF_String.h"
#include "D3D9_ShaderManager.h"
#include "D3D9_MeshCache.h"

#include <d3d9.h>
#include <d3dx9.h>

namespace Scaleform { namespace Render { namespace D3D9 {

// Define this if shaders are to be compiled on-demand
// Minimizes startup time. Should be off for debugging shader compatibility.
#define SF_RENDER_D3D9_SHADER_ON_DEMAND

#define SF_RENDERER_PSHADER_PROFILE "ps_2_0"


#define DEF_FILLSET(vshader, declFlags)     (declFlags | vshader)
#define DEF_VSHADER(group, color, texture)  (VSGroup_##group | VSColor_##color | VSTexture_##texture)

static VertexShaderDesc  ShaderManager_VertexShaderDescriptors[(PrimFill_Type_Count-1) * 3] =
{
    // PrimFill_Mask
    DEF_FILLSET(DEF_VSHADER(Single, None, None),      0),
    DEF_FILLSET(DEF_VSHADER(Batch, None, None),       VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, None),   0),
    // PrimFill_SolidColor
    DEF_FILLSET(DEF_VSHADER(Single, Solid, None),     0),
    DEF_FILLSET(DEF_VSHADER(Batch, Solid, None),      VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, Solid, None),  0),
    // PrimFill_VColor
    DEF_FILLSET(DEF_VSHADER(Single, Vertex, None),    VFE_VColor),
    DEF_FILLSET(DEF_VSHADER(Batch, Vertex, None),     VFE_VColor|VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, Vertex, None), VFE_VColor),
    // PrimFill_VColor_EAlpha
    DEF_FILLSET(DEF_VSHADER(Single, Vertex, None),    VFE_VColor|VFE_EAlpha),
    DEF_FILLSET(DEF_VSHADER(Batch, Vertex, None),     VFE_VColor|VFE_EAlpha|VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, Vertex, None), VFE_VColor|VFE_EAlpha),
    // PrimFill_Texture
    DEF_FILLSET(DEF_VSHADER(Single, None, Gen1),      0),
    DEF_FILLSET(DEF_VSHADER(Batch, None, Gen1),       VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, Gen1),   0),
    // PrimFill_Texture_EAlpha
    DEF_FILLSET(DEF_VSHADER(Single, None, Gen1),      VFE_EAlpha),
    DEF_FILLSET(DEF_VSHADER(Batch, None, Gen1),       VFE_EAlpha|VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, Gen1),   VFE_EAlpha),
    // PrimFill_Texture_VColor
    DEF_FILLSET(DEF_VSHADER(Single, Vertex, Gen1),    VFE_VColor|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Batch, Vertex, Gen1),     VFE_VColor|VFE_IIndex|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Instanced, Vertex, Gen1), VFE_VColor|VFE_Weight),
    // PrimFill_Texture_VColor_EAlpha
    DEF_FILLSET(DEF_VSHADER(Single, Vertex, Gen1),    VFE_VColor|VFE_EAlpha|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Batch, Vertex, Gen1),     VFE_VColor|VFE_EAlpha|VFE_IIndex|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Instanced, Vertex, Gen1), VFE_VColor|VFE_EAlpha|VFE_Weight),
    // PrimFill_2Texture
    DEF_FILLSET(DEF_VSHADER(Single, None, Gen2),      VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Batch, None, Gen2),       VFE_IIndex|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, Gen2),   VFE_Weight),
    // PrimFill_2Texture_EAlpha
    DEF_FILLSET(DEF_VSHADER(Single, None, Gen2),      VFE_EAlpha|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Batch, None, Gen2),       VFE_EAlpha|VFE_IIndex|VFE_Weight),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, Gen2),   VFE_EAlpha|VFE_Weight),
    // PrimFill_UVTexture
    DEF_FILLSET(DEF_VSHADER(Single, None, UV1),       VFE_UV32f),
    DEF_FILLSET(DEF_VSHADER(Batch, None, UV1),        VFE_UV32f|VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, None, UV1),    VFE_UV32f),  // Not needed
    // PrimFill_UVTextureAlpha_VColor
    DEF_FILLSET(DEF_VSHADER(Single, Vertex, UV1),     VFE_VColor|VFE_UV32f),
    DEF_FILLSET(DEF_VSHADER(Batch, Vertex, UV1),      VFE_VColor|VFE_UV32f|VFE_IIndex),
    DEF_FILLSET(DEF_VSHADER(Instanced, Vertex, UV1),  VFE_VColor|VFE_UV32f),  // Not needed
};



// Texture sample/weight operation done by pixel shader
// Indexed based on PrimitiveFillType + Texture type(s).

enum PSFillOp
{
    PSFillOp_None,
    PSFillOp_NoColor,
    PSFillOp_Texture,
    PSFillOp_Texture_VColor,
    PSFillOp_2Texture,
    PSFillOp_TextureA_VColor,
    PSFillOp_TextureYUV,
    PSFillOp_TextureYUVA,
    PSFillOp_Count
};

// ShaderManager_PixelShaderDescriptors combines PSFillOp_Texture and extra PSCap bits
// to define a 'psBits' key for each supported PSType.
// PSBit_Multiply, PSBit_AlphaWtite and PSBit_Instanced bits are not included because
// all of their combinations are created for each type declared here.
// All of these PSTypes are created during ShaderManager::Initialize


#define DEF_PSHADER(fillOp, bits) ((fillOp<<PSBit_FillOp_Shift)|bits)

// PSType -> { PSFillOp, EAlpha flag }
unsigned ShaderManager_PixelShaderDescriptors[PS_Type_Count-1] = 
{   
    DEF_PSHADER(PSFillOp_NoColor, 0),                      // PS_NoColor,
    DEF_PSHADER(PSFillOp_None, 0),                         // PS_Color,
    DEF_PSHADER(PSFillOp_None, PSBit_EAlpha),              // PS_Color_EAlpha,
    DEF_PSHADER(PSFillOp_Texture, 0),                      // PS_Texture,
    DEF_PSHADER(PSFillOp_Texture, PSBit_EAlpha),           // PS_Texture_EAlpha,
    DEF_PSHADER(PSFillOp_Texture_VColor, 0),               // PS_Texture_VColor,
    DEF_PSHADER(PSFillOp_Texture_VColor, PSBit_EAlpha),    // PS_Texture_VColor_EAlpha,
    DEF_PSHADER(PSFillOp_2Texture, 0),                     // PS_2Texture,
    DEF_PSHADER(PSFillOp_2Texture, PSBit_EAlpha),          // PS_2Texture_EAlpha,
    DEF_PSHADER(PSFillOp_TextureA_VColor, 0),              // PS_TextureA_VColor,
    DEF_PSHADER(PSFillOp_TextureYUV, 0),                   // PS_TextureYUV,
    DEF_PSHADER(PSFillOp_TextureYUVA, 0),                  // PS_TextureYUVA,
};



bool ShaderManager::createVertexShaderString(StringBuffer& sb, VertexShaderDesc& desc)
{
    VSGroupMode     groupMode = desc.GetGroupMode();
    VSColorInput    colorInput= desc.GetColorInput();
    VSTextureInput  textureInput = desc.GetTextureInput();
    bool            hasFactors = desc.HasFactors();

    sb.Clear();

    // Declarations

    if (colorInput == VSColor_Solid)
        sb += "float4 scolor : register(c0);\n";


    if (groupMode != VSGroup_Single)
        sb += "float4 mat[240] : register(c1);\n"; // 10 * 24 (max instanced).
    else
        sb += "float4 mat[10] : register(c1);\n"; // 10 * 1.

    sb += "void main(float4 pos : POSITION\n";

    // Arguments

    if (colorInput == VSColor_Vertex)
        sb += ",float4 vcolor : COLOR0\n";

    if (hasFactors)
    {
        sb += ",float4 factors : COLOR1\n";
        if (groupMode == VSGroup_Instanced)
            sb += ",float4 inst : COLOR2\n";
    }
    else
    {
        SF_ASSERT(groupMode != VSGroup_Batch);
         // Instance -> factor
        if (groupMode == VSGroup_Instanced)
            sb += ",float4 factors : COLOR1\n";
    }
    
    if (textureInput == VSTexture_UV1)
        sb += ",float2 tc0 : TEXCOORD0\n";

    sb += ",out float4 opos : POSITION\n";

    if (textureInput > VSTexture_None)
        sb += ",out float2 otc0 : TEXCOORD0\n";
    if (textureInput == VSTexture_Gen2)
        sb += ",out float2 otc1 : TEXCOORD1\n";

    if ((colorInput == VSColor_Solid) || (colorInput == VSColor_Vertex))
        sb += ",out float4 oc0 : COLOR0\n";

    // Use TEXCOORD for 'ofactors' as COLOR gets clamped to 1.
    if (hasFactors || (groupMode != VSGroup_Single))
        sb += ",out float4 ofactors : TEXCOORD2\n";

    sb += ",out float4 cm : TEXCOORD3\n";
    sb += ",out float4 ca : TEXCOORD4\n";
           
    // Body
    sb += ") {\n";
    sb += "opos = pos; \n";

    if (groupMode != VSGroup_Single)
    {
        if ((groupMode == VSGroup_Instanced) && hasFactors)
            sb += "int index = inst.b * 255;\n";
        else
            sb += "int index = factors.b * 255;\n";        

        switch(textureInput)
        {
        case VSTexture_None: sb += "index *= 6;\n"; break;
        case VSTexture_Gen1: sb += "index *= 8;\n"; break;
        case VSTexture_Gen2: sb += "index *= 10;\n"; break;
        case VSTexture_UV1:  sb += "index *= 6;\n"; break;
        }
    }
    else
    {
        sb += "int index = 0;\n";
    }


    // Transform the output position (4d).
    sb += "opos.x = dot(pos, mat[index]);\n";
    sb += "opos.y = dot(pos, mat[index+1]);\n";
    sb += "opos.z = dot(pos, mat[index+2]);\n";
    sb += "opos.w = dot(pos, mat[index+3]);\n";

    // Color modification.
    sb += "cm = mat[index + 4];\n"
          "ca = mat[index + 5];\n";

    // Apply texturing (if required).
    switch(textureInput)
    {
        case VSTexture_Gen1:
            sb += "otc0.x = dot(pos, mat[index+6]);\n"
                  "otc0.y = dot(pos, mat[index+7]);\n";
            break;
        
        case VSTexture_Gen2:
            sb += "otc0.x = dot(pos, mat[index+6]);\n"
                "otc0.y = dot(pos, mat[index+7]);\n"
                "otc1.x = dot(pos, mat[index+8]);\n"
                "otc1.y = dot(pos, mat[index+9]);\n";
            break;

        case VSTexture_UV1:
            sb += "otc0 = tc0;\n";
            break;
    }

    if (colorInput == VSColor_Solid)
        sb += "oc0 = scolor;\n";
    else if (colorInput == VSColor_Vertex)
        sb += "oc0 = vcolor;\n";
    

    if (hasFactors || (groupMode == VSGroup_Instanced))
    {
        sb += "ofactors = factors;\n";
    }

    sb+= "}\n";

    return true;
}





// TexturePShader is used to associate an ImageFormat with a PixelShader type
// that supports it for a given fill style. Some pixel shaders may support multiple
// formats; however, some formats, such as YUV require a custom-tailored shader.

struct TexturePShader
{
    // Pixel shader texture type capability flags. Every bit identifies texture format 
    // type that a shader can accept. ImageFormat is mapped to one of these types.
    // If the texture cap bit is set, this shader can be used with texture.
    enum TextureType
    {
        Texture_None  = 0x0010,
        Texture_A     = 0x0020,
        Texture_RGBA  = 0x0040,
        Texture_YUV   = 0x0100,
        Texture_YUVA  = 0x0200,
    };
    enum {
        PSType_Mask = 0x0F, 
    };

    unsigned           Flags;
    TexturePShader*    pNext;

    static  TextureType ImageFormatToTextureType(ImageFormat format);

    PSType      GetShader() const                         { return (PSType)(Flags & PSType_Mask); }
    bool        SupportsTextureType(TextureType tt) const { return (Flags & tt) != 0; }

    inline TexturePShader* MatchImageFormat(ImageFormat format);
};

// PixelShader definition combined PSType with set of texture type bits that it can support.
// DEF_TEXTURE_PSHADER_MULTIPLE is used to create a linked-list of different shaders so that
// the appropriate one is picked based on format.
#define DEF_TEXTURE_PSHADER(shader, t0)                 { (shader|TexturePShader::Texture_##t0), 0}
#define DEF_TEXTURE_PSHADER2(shader, t0, t1)            { (shader|TexturePShader::Texture_##t0|TexturePShader::Texture_##t1), 0 }
#define DEF_TEXTURE_PSHADER_MULTIPLE(shader, t0, other) { (shader|TexturePShader::Texture_##t0), other }

// Additional TexturePShader nodes needed for YUV/YUVA formats.
static TexturePShader TPS_SelectYUVA = DEF_TEXTURE_PSHADER(PS_TextureYUVA, YUVA);
static TexturePShader TPS_SelectYUV  = DEF_TEXTURE_PSHADER_MULTIPLE(PS_TextureYUV, YUV, &TPS_SelectYUVA);

// FillToPixelShader maps a fill style to as list of pixel shaders that can be
// used with it. Within the list, TextureType, obtained from the textures ImageFormat
// is used to determine the exact shader applied. 
static TexturePShader ShaderManager_FillToPixelShader[PrimFill_Type_Count] = 
{
    DEF_TEXTURE_PSHADER(PS_None, None),                                 // PrimFill_None
    DEF_TEXTURE_PSHADER(PS_NoColor, None),                              // PrimFill_Mask
    DEF_TEXTURE_PSHADER(PS_Color, None),                                // PrimFill_SolidColor
    DEF_TEXTURE_PSHADER(PS_Color, None),                                // PrimFill_VColor
    DEF_TEXTURE_PSHADER(PS_Color_EAlpha, None),                         // PrimFill_VColor_EAlpha
    DEF_TEXTURE_PSHADER_MULTIPLE(PS_Texture, RGBA, &TPS_SelectYUV),     // PrimFill_Texture
    DEF_TEXTURE_PSHADER(PS_Texture_EAlpha, RGBA),                       // PrimFill_Texture_EAlpha
    DEF_TEXTURE_PSHADER(PS_Texture_VColor, RGBA),                       // PrimFill_Texture_VColor
    DEF_TEXTURE_PSHADER(PS_Texture_VColor_EAlpha, RGBA),                // PrimFill_Texture_VColor_EAlpha
    DEF_TEXTURE_PSHADER(PS_2Texture, RGBA),                             // PrimFill_2Texture
    DEF_TEXTURE_PSHADER(PS_2Texture_EAlpha, RGBA),                      // PrimFill_2Texture_EAlpha
    DEF_TEXTURE_PSHADER_MULTIPLE(PS_Texture, RGBA, &TPS_SelectYUV),     // PrimFill_UVTexture
    DEF_TEXTURE_PSHADER2(PS_TextureA_VColor, A, RGBA)                   // PrimFill_UVTextureAlpha_VColor
};

TexturePShader::TextureType TexturePShader::ImageFormatToTextureType(ImageFormat format)
{
    switch(format)
    {
    case Image_R8G8B8A8:
    case Image_B8G8R8A8:
    case Image_R8G8B8:
    case Image_B8G8R8:
    case Image_DXT1:
    case Image_DXT3:
    case Image_DXT5:
        return Texture_RGBA;
    case Image_A8:
        return Texture_A;
    case Image_Y8_U2_V2:
        return Texture_YUV;
    case Image_Y8_U2_V2_A8:
        return Texture_YUVA;

    case Image_None:
    default:
        break;
    }
    return Texture_None;
}

inline TexturePShader* TexturePShader::MatchImageFormat(ImageFormat format)
{
    TexturePShader* p  = this;
    TextureType     tt = ImageFormatToTextureType(format);

    if (!p->SupportsTextureType(tt))
    {
        do { p = p->pNext; }
        while (p && !p->SupportsTextureType(tt));        
        SF_DEBUG_WARNING(!p, "D3D9::ShaderManager failed to set pixel shader - unsupported ImageFormat");        
    }
    return p;
}




enum PSFillOpTextureType
{
    PSFillOpTex_None,
    PSFillOpTex_1,
    PSFillOpTex_2,
    PSFillOpTex_YUV,
    PSFillOpTex_YUVA
};

static UByte FillOp_TextureTypes[PSFillOp_Count] =
{
    PSFillOpTex_None,   // PSFillOp_None,
    PSFillOpTex_None,   // PSFillOp_NoColor,
    PSFillOpTex_1,      // PSFillOp_Texture,
    PSFillOpTex_1,      // PSFillOp_Texture_VColor,
    PSFillOpTex_2,      // PSFillOp_2Texture,
    PSFillOpTex_1,      // PSFillOp_TextureA_VColor,
    PSFillOpTex_YUV,    // PSFillOp_TextureYUV,
    PSFillOpTex_YUVA    // PSFillOp_TextureYUVA,
};
static bool FillOp_NeedsFactorArg[PSFillOp_Count] = 
{
    false,
    false,
    false,
    true,   // PSFillOp_Texture_VColor
    true,   // PSFillOp_2Texture
    false,
    false,
    false
};
static bool FillOp_NeedsColorArg[PSFillOp_Count] = 
{
    true,   // PSFillOp_None,
    false,  // PSFillOp_NoColor
    false,  // PSFillOp_Texture,
    true,   // PSFillOp_Texture_VColor,
    false,  // PSFillOp_2Texture,
    true,   // PSFillOp_TextureA_VColor,
    false,  // PSFillOp_TextureYUV,
    false   // PSFillOp_TextureYUVA,
};

static const char* FillOp_Body[PSFillOp_Count] = 
{
    // PSFillOp_None - pass along argument value
    "",
    // PSFillOp_NoColor
    "color = float4(1.0, 1.0, 1.0, 1.0);",
    // PSFillOp_Texture
    "color = tex2D(tex0, tc0);\n",
    // PSFillOp_Texture_VColor
    "color = lerp(color, tex2D(tex0, tc0), factors.g);\n",
    // PSFillOp_2Texture
    "color = lerp(tex2D(tex1, tc1), tex2D(tex0, tc0), factors.g);\n",
    
    // PSFillOp_TextureA_VColor
    // TBD: In old shaders this multiply was done after cxform
    "color.a *= tex2D(tex0, tc0).a;\n",
    
    // PSFillOp_TextureYUV
    "float  Y = (float)tex2D(tex_y, tc0).a;\n"
    "float  U = (float)tex2D(tex_u, tc0).a - 128.f/255.f;\n"
    "float  V = (float)tex2D(tex_v, tc0).a - 128.f/255.f;\n"
    "color = float4( float3((float3(Y, Y, Y) - float3(16.f/255.f, 16.f/255.f, 16.f/255.f)) * 1.164), 1);\n"
    "color.r +=  V * 1.596f;\n"
    "color.g += -U * 0.392f - V * 0.813f;\n"
    "color.b +=  U * 2.017f;\n",
    // PSFillOp_TextureYUVA
    "float  Y = (float)tex2D(tex_y, tc0).a;\n"
    "float  U = (float)tex2D(tex_u, tc0).a - 128.f/255.f;\n"
    "float  V = (float)tex2D(tex_v, tc0).a - 128.f/255.f;\n"
    "color = float4( float3((float3(Y, Y, Y) - float3(16.f/255.f, 16.f/255.f, 16.f/255.f)) * 1.164), tex2D(tex_a, tc0).a);\n"
    "color.r +=  V * 1.596f;\n"
    "color.g += -U * 0.392f - V * 0.813f;\n"
    "color.b +=  U * 2.017f;\n"
};


bool ShaderManager::createPixelShaderString(StringBuffer& sb, unsigned psBits)
{
    PSFillOp            fillOp      = (PSFillOp)((psBits & PSBit_FillOp_Mask) >> PSBit_FillOp_Shift);
    PSFillOpTextureType textureType = (PSFillOpTextureType)FillOp_TextureTypes[fillOp];
    bool                colorArg    = FillOp_NeedsColorArg[fillOp];
    bool                factorArg   = FillOp_NeedsFactorArg[fillOp];
   
    if (psBits & (PSBit_Instanced|PSBit_EAlpha))
        factorArg = true;    

    sb.Clear();

    // PS 2.0 has 32 constant registers. Literals count to this value.
    // PS 1.1 has only 8.
    // Increasing the number of these constants decreases performance?!?
  //  sb += "float4 cx[2] : register(c0);\n";

    /*
    P.S./V.S. 2.0 Shader performance observations:
     - 'Single' Cxform indexing through pixel shader constant is cheap (almost free)
     - Dynamic pixel shader indexing is very expensive with cost increasing with
       array size, making it not an option.
     - VertexShader indexing, with passing Cxform through shader constans as TEXCOORD
       seems to be a best anthough it does introduce overhead.
     - Passing more VertexShader/PixelShader constants per frame reduces performance,
       for Cxform this is ~15%. This difference is most pronounced on NVidia XP
       (not as much on ATI/Intel Vista).
     - Overall, extra TEXCOORDS + shader constant setting reduces Instancing test
       performace by about 20%.
    */

    switch(textureType)
    {
    case PSFillOpTex_2:
        sb += "sampler tex1  : register(s1);\n";
    case PSFillOpTex_1:
        sb += "sampler tex0  : register(s0);\n";
        break;
    case PSFillOpTex_YUVA:
        sb += "sampler tex_a : register(s3);\n";
    case PSFillOpTex_YUV:
        sb += "sampler tex_y : register(s0);\n"
              "sampler tex_u : register(s1);\n"
              "sampler tex_v : register(s2);\n";
    default:
        break;
    }

    sb += "void main(\n";

    if (colorArg)
        sb += "float4 color : COLOR0,\n";
    
    if (fillOp != PSFillOp_None)
    {
        sb += "float2 tc0 : TEXCOORD0,\n";
        if (fillOp == PSFillOp_2Texture)
            sb += "float2 tc1 : TEXCOORD1,\n";
    }

    if (factorArg)
        sb += "float4 factors : TEXCOORD2,\n";

    if (fillOp != PSFillOp_NoColor)
        sb += "float4 cm : TEXCOORD3,\n"
              "float4 ca : TEXCOORD4,\n";

    sb += "out float4 oc : COLOR)\n{\n"; 

    // Begin main() {

    if (!colorArg)
        sb += "float4 color;\n";

    sb += FillOp_Body[fillOp];

    if (fillOp != PSFillOp_NoColor)
    {
        // Color transform 
        sb += "color = color * cm + ca;\n";

        if (psBits & PSBit_EAlpha)
            sb += "color.a *= factors.a;\n";

        // Multiply lerp / Alpha write
        switch(psBits & (PSBit_Multiply|PSBit_AlphaWrite))
        {
        case PSBit_Multiply|PSBit_AlphaWrite:
            sb += "color  = lerp(1, color, color.a);\n";
            sb += "oc.rgb = color * color.a;\n"
                  "oc.a   = color.a;\n";
            break;
        case PSBit_AlphaWrite:
            sb += "oc.rgb = color * color.a;\n"
                  "oc.a   = color.a;\n";
            break;
        case PSBit_Multiply:
            sb += "oc = lerp(1, color, color.a);\n";
            break;
        case 0:
            sb += "oc = color;\n";
            break;
        }
    }
    else
    {   // No color, assign solid color constant (used for Masks).
        sb += "oc = color;\n";
    }
    
    sb+= "}\n";

    return true;
}



IDirect3DVertexShader9* ShaderManager::registerVertexShader(VertexShaderDesc& desc)
{
    StringBuffer                 sb;
    Ptr<IDirect3DVertexShader9>  pvs;
    unsigned                     key = desc.GetVertexShaderKey();        
    Ptr<IDirect3DVertexShader9>* ppvs= VShaders.Find(&key);

    if (!ppvs)
    {
        if (!createVertexShaderString(sb, desc) ||
            !createVertexShader(&pvs, sb.ToCStr()))
            return 0;
        ppvs = VShaders.Add(&key);
        if (!ppvs)
            return 0;
        *ppvs = pvs;
    }
    return *ppvs;
}

IDirect3DPixelShader9*  ShaderManager::registerPixelShader(unsigned psBits)
{
    StringBuffer                sb;
    Ptr<IDirect3DPixelShader9>  ps;    
    Ptr<IDirect3DPixelShader9>* pps= PShaders.Find(&psBits);
    if (pps)
        return *pps;

    if (!createPixelShaderString(sb, psBits) ||
        !createPixelShader(&ps, sb.ToCStr()))
        return 0;
    pps = PShaders.Add(&psBits);
    if (!pps)
        return 0;
    *pps = ps;    
    return *pps;
}


struct VertexElementBuilder
{
    VertexElement Elements[8];
    unsigned      Size, Count;

    VertexElementBuilder() : Size(0), Count(0) { }

    void    Add(unsigned attr, unsigned offsetDelta, unsigned size)
    {
        Elements[Count].Offset    = Size + offsetDelta;
        Elements[Count].Attribute = attr;
        Size += size;        
        SF_ASSERT(Count < 8);
        Count++;
    }
    void    Add(unsigned attr, unsigned size)   { Add(attr, 0, size); }

    void    Finish()
    {
        Elements[Count].Offset      = 0;
        Elements[Count++].Attribute = VET_None;
    }
};

struct D3DVertexDeclBuilder
{
    D3DVERTEXELEMENT9 Elements[8];
    WORD              Size[2], Count;

    D3DVertexDeclBuilder() : Count(0)
    { Size[0] = Size[1] = 0; }

    void    Add(WORD stream, D3DDECLTYPE type, WORD size,
                D3DDECLUSAGE usage, BYTE usageIndex = 0)
    {
        D3DVERTEXELEMENT9 &e = Elements[Count++];
        e.Stream    = stream;
        e.Offset    = Size[stream];
        e.Type      = (BYTE)type;
        e.Method    = D3DDECLMETHOD_DEFAULT;
        e.Usage     = (BYTE)usage;
        e.UsageIndex= usageIndex;
        Size[stream]= Size[stream] + size;
        SF_ASSERT(Count < 8);
    }

    void    Finish()
    {
        D3DVERTEXELEMENT9 e = D3DDECL_END();
        Elements[Count++] = e;
    }
};


const VertexFormat* ShaderManager::registerVertexFormat(VertexShaderDesc& desc, bool posFloat)
{
    unsigned               flags = desc.Flags;
    VertexElementBuilder   ve;
        
    if (posFloat)
        ve.Add(VET_XY32f, 8);
    else
        ve.Add(VET_XY16i, 4);
    
    if (flags & VFE_VColor)
        ve.Add(VET_ColorARGB8, 4);
    
    if (desc.HasFactors())
    {
        if (flags & VFE_IIndex)
            ve.Add(VET_Instance8, 0, 0);
        if (flags & VFE_EAlpha)
            ve.Add(VET_FactorAlpha8, 3, 0);
        if (flags & VFE_Weight)
            ve.Add(VET_T0Weight8, 1, 0);
        ve.Size += 4;
    }

    if (flags & VFE_UV32f)
        ve.Add(VET_UV32f, 8);
    ve.Finish();

    // Register formats if not registered yet.
    VertexElement *pelements = 0;
    VertexFormat  *pformat   = VFormats.Find(ve.Elements, ve.Count);
    if (pformat)
        return pformat;

    pformat = VFormats.Add(&pelements, ve.Elements, ve.Count);
    if (!pformat)
        return 0;
    pformat->Size      = ve.Size;
    pformat->pElements = pelements;
    return pformat;
}

IDirect3DVertexDeclaration9* ShaderManager::registerVertexDeclaration(VertexShaderDesc& desc, bool posFloat)
{
    unsigned             flags = desc.Flags;
    D3DVertexDeclBuilder vd;
    
    if (posFloat)
        vd.Add(0, D3DDECLTYPE_FLOAT2, 8, D3DDECLUSAGE_POSITION);
    else
        vd.Add(0, D3DDECLTYPE_SHORT2, 4, D3DDECLUSAGE_POSITION);        

    if (flags & VFE_VColor)
        vd.Add(0, D3DDECLTYPE_D3DCOLOR, 4, D3DDECLUSAGE_COLOR);

    if (desc.HasFactors())
        vd.Add(0, D3DDECLTYPE_D3DCOLOR, 4, D3DDECLUSAGE_COLOR, 1);
    
    if (flags & VFE_UV32f)
        vd.Add(0, D3DDECLTYPE_FLOAT2, 8, D3DDECLUSAGE_TEXCOORD);

    if (desc.GetGroupMode() == VSGroup_Instanced)    
        vd.Add(1, D3DDECLTYPE_D3DCOLOR, 4, D3DDECLUSAGE_COLOR, desc.HasFactors() ? 2 : 1);

    vd.Finish();

    // Find/Create D3D Declaration
    Ptr<IDirect3DVertexDeclaration9>* ppdecl = VDecls.Find(vd.Elements, vd.Count);
    if (ppdecl)
        return *ppdecl;    
    ppdecl = VDecls.Add(vd.Elements, vd.Count);
    if (!ppdecl)
        return 0;
    if (!createVertexDeclaration(ppdecl, vd.Elements))
        return 0;
    return *ppdecl;
}



// ***** ShaderManager

ShaderManager::ShaderManager()
: pDevice(0), InstancingSupport(false),
  AppliedTextureCount(0), PrimitivePSType(PS_None)
{
    memset(PSTable, 0, sizeof(PSTable));
    memset(VHandlerTable, 0, sizeof(VHandlerTable));

    pPrevVDecl   = 0;
    PrevPSIndex  = 0;
    pPrevVShader = 0;
    PrevTextures[0].Set(0, ImageFillMode());
    PrevTextures[1].Set(0, ImageFillMode());
}

ShaderManager::~ShaderManager()
{   
}


// Helpers used to create vertex declarations and shaders. 
bool    ShaderManager::createVertexDeclaration(Ptr<IDirect3DVertexDeclaration9> *pdeclPtr,
                                               const D3DVERTEXELEMENT9 *pvertexElements)
{
    if (!*pdeclPtr)
    {
        if (pDevice->CreateVertexDeclaration(pvertexElements,
                                             &pdeclPtr->GetRawRef()) != S_OK)
        {
            SF_DEBUG_WARNING(1, "RendererD3Dx - failed to create vertex declaration");
            return 0;
        }
    }
    return 1;
}


bool    ShaderManager::createVertexShader(Ptr<IDirect3DVertexShader9> *pshaderPtr,
                                          const char* pshaderText)
{
    if (!*pshaderPtr)
    {
        Ptr<ID3DXBuffer> pshader;
        Ptr<ID3DXBuffer> pmsg;
        HRESULT hr;

//        printf("%s\n\n", pshaderText);

        hr = D3DXCompileShader(pshaderText, (unsigned)strlen(pshaderText),
                               NULL, NULL, "main",
                               SF_RENDERER_VSHADER_PROFILE, D3DXSHADER_DEBUG,
                               &pshader.GetRawRef(), &pmsg.GetRawRef(), NULL);


        if (FAILED(hr))
        {
            SF_DEBUG_WARNING1(1, "GRendererD3Dx - VertexShader errors:\n %s ",
                              pmsg->GetBufferPointer() );
            return 0;
        }
        if (!pshader ||
            ((hr = pDevice->CreateVertexShader((DWORD*)pshader->GetBufferPointer(),
                                               &pshaderPtr->GetRawRef())) != S_OK) )
        {
            SF_DEBUG_WARNING1(1, "GRendererD3Dx - Can't create D3Dx vshader; error code = %d", hr);
            return 0;
        }
    }
    return 1;
}

bool    ShaderManager::createPixelShader(Ptr<IDirect3DPixelShader9> *pshaderPtr,
                                         const char* pshaderText)
{
    if (!*pshaderPtr)
    {
        Ptr<ID3DXBuffer> pmsg;
        Ptr<ID3DXBuffer> pshader;
        HRESULT hr;

//        printf("%s\n\n", pshaderText);

#if defined(SF_RENDERER_PSHADER_PROFILE)
        hr = D3DXCompileShader(pshaderText, (unsigned)strlen(pshaderText),
                               NULL, NULL, "main",
                               SF_RENDERER_PSHADER_PROFILE, D3DXSHADER_DEBUG,
                               &pshader.GetRawRef(), &pmsg.GetRawRef(), NULL);
#else
        hr = D3DXAssembleShader(pshaderText, (unsigned)strlen(pshaderText),
                                NULL, NULL, D3DXSHADER_DEBUG,
                                &pshader.GetRawRef(), &pmsg.GetRawRef());
#endif


        if (FAILED(hr))
        {
            SF_DEBUG_WARNING1(1, "GRendererD3D9 - PixelShader errors:\n %s ",
                                  pmsg->GetBufferPointer() );
            return 0;
        }
        if (!pshader ||
            ((hr = pDevice->CreatePixelShader( (DWORD*)pshader->GetBufferPointer(),
                                               &pshaderPtr->GetRawRef())) != S_OK) )
        {
            SF_DEBUG_WARNING1(1, "GRendererD3D9 - Can't create D3D9 pshader; error code = %d", hr);
            return 0;
        }
    }
    return 1;
}

bool   ShaderManager::Initialize(IDirect3DDeviceX* pdevice, const D3DCAPSx& caps, const ProfileViews* profiler)
{
    SF_ASSERT(!pDevice);
    pDevice = pdevice;
    Profiler = profiler;

    unsigned i, j;

    // HW instancing is supported with VS 3.0 only.
    InstancingSupport = (caps.VertexShaderVersion >= D3DVS_VERSION(3,0));
    //InstancingSupport = false;
    
    // Register VertexShaders and Declarations
    for (i = 3; i < PrimFill_Type_Count * 3; i++)
    {
        // If instancing is not supported, skip its entries in declaration table.
        if (!InstancingSupport && (i%3 == 2))
            continue;

        VertexShaderDesc& desc = ShaderManager_VertexShaderDescriptors[i-3];
        VertexHandler&    vh   = VHandlerTable[i];

        // TBD: Skip Instance if not enabled.
        // if (third index)...

        vh.pDecl[0]   = registerVertexDeclaration(desc, false);
        vh.pDecl[1]   = registerVertexDeclaration(desc, true);
        vh.pFormat[0] = registerVertexFormat(desc, false);
        vh.pFormat[1] = registerVertexFormat(desc, true);
#ifdef SF_RENDER_D3D9_SHADER_ON_DEMAND
        vh.pVShader   = 0;
        vh.VShaderInit= false;
#else
        vh.pVShader   = registerVertexShader(desc);
        vh.VShaderInit= true;
#endif

        // TBD: What if this fails.. no shaders?
        if (!vh.pDecl[0]   || !vh.pDecl[1]   ||
            !vh.pFormat[0] || !vh.pFormat[1] || (!vh.pVShader && vh.VShaderInit) )
        {
            Reset();
            return false;
        }
    }
    
    // Register pixel shaders-> separate loop
    for(i = 1; i < PS_Type_Count; i++)
    {
        unsigned maxLowCount = PSTable_ElementsPerType;

        // Don't take out Instanced bit because it's also used with batching
        // TBD: may not be necessary of combinable EdgeAA/etc since factor.g is no
        // longer used in pixel shaders...
       // if (!InstancingSupport)
       //     maxLowCount /= 2;

        for (j = 0; j < maxLowCount; j++)
        {
#ifdef SF_RENDER_D3D9_SHADER_ON_DEMAND
            PSInitTable[i* PSTable_ElementsPerType + j] = false;
#else
            IDirect3DPixelShader9* pshader = 
                registerPixelShader(ShaderManager_PixelShaderDescriptors[i-1]|j);
            if (!pshader)
            {
                Reset();
                return false;
            }
            PSTable[i* PSTable_ElementsPerType + j] = pshader;
            PSInitTable[i* PSTable_ElementsPerType + j] = true;
#endif
        }
    }
    return true;
}


void   ShaderManager::Reset()
{
    // Clear collections as they AddRef to shaders
    VDecls.Clear();
    VFormats.Clear();
    VShaders.Clear();
    PShaders.Clear();

    pPrevVDecl   = 0;
    PrevPSIndex  = 0;    
    pPrevVShader = 0;
    PrevTextures[0].Clear();
    PrevTextures[1].Clear();

    pDevice = 0;
    memset(PSTable, 0, sizeof(PSTable));
    memset(VHandlerTable, 0, sizeof(VHandlerTable));
}

void    ShaderManager::BeginScene()
{
    pPrevVDecl   = 0;
    PrevPSIndex  = 0;
    pPrevVShader = 0;
    PrevTextures[0].Clear();
    PrevTextures[1].Clear();
}

void    ShaderManager::EndScene()
{
    // TBD: Not sure if clearing textures optimizes anything.. perhaps we should only
    //      do this in the end of the frame?
    while(AppliedTextureCount > 0)
    {
        AppliedTextureCount--;
        pDevice->SetTexture(AppliedTextureCount, 0);
    }

    // AppliedTextureCount...
    pDevice->SetVertexShader(0);
   // pDevice->SetTexture(0,0);
}


// Applies the part of fill state that is shared between primitive batches.
bool   ShaderManager::ApplyPrimitiveFill(PrimitiveFill* pfill)
{
    // SolidColor value and Texture State is shared across the whole primitive,
    // so we apply it here.
    
    PrimitiveFillType fillType = Profiler->GetFillType(pfill->GetType()); 
    TexturePShader* pshader = &ShaderManager_FillToPixelShader[fillType];    

    if (fillType == PrimFill_SolidColor)
    {
        // Set vertex shader constant 0 to color.
        applySolidColorConstant(pfill->GetSolidColor());
    }

    else if (fillType >= PrimFill_Texture)
    {        
        D3D9::Texture*  pt0 = (D3D9::Texture*)pfill->GetTexture(0);
        DWORD           textureIndex = 0;
        SF_ASSERT(pt0);
        if (!pt0) return false;

        pshader = pshader->MatchImageFormat(pt0->GetImageFormat());
        if (!pshader) // No shader existed for specified format!!
        {
            PrimitivePSType = PS_None;
            return false;
        }
        
        ImageFillMode fm0 = pfill->GetFillMode(0);
        if (!PrevTextures[0].Match(pt0, fm0))
        {
            PrevTextures[0].Set(pt0, fm0);
            pt0->ApplyTexture(textureIndex, fm0);
        }
        textureIndex += pt0->GetTextureStageCount();

        if ((fillType == PrimFill_2Texture) || (fillType == PrimFill_2Texture_EAlpha))
        {
            D3D9::Texture* pt1 = (D3D9::Texture*)pfill->GetTexture(1);
            ImageFillMode  fm1 = pfill->GetFillMode(1);
            if (!PrevTextures[1].Match(pt1, fm1, textureIndex))
            {
                PrevTextures[1].Set(pt1, fm1, textureIndex);
                pt1->ApplyTexture(textureIndex, pfill->GetFillMode(1));
            }
            textureIndex += pt1->GetTextureStageCount();
        }
        
        // We no longer clear textures here, as it not clear if that would optimize anything.
        // Clearing textures would break/affect PrevTexture matching above.
        if (AppliedTextureCount < textureIndex)
            AppliedTextureCount = textureIndex;
    }

    // Store PSType so that it can be applied in ApplyPrimitiveBatchFill
    PrimitivePSType = pshader->GetShader();
    return true;
}


bool ShaderManager::ApplyPrimitiveBatchFill(PrimitiveFillType fillType, UInt16 fillFlags,
                                            const VertexFormat* pformat,
                                            PrimitiveBatch::BatchType batchType)
{
    fillType = Profiler->GetFillType(fillType);

    // Determine vertex shader/declaration

    // TBD: SLOW.. take format/flag/vh from Batch?
    const VertexElement *ve = pformat->GetElement(VET_Pos, VET_Usage_Mask);
    SF_ASSERT(ve);    

    if (batchType > PrimitiveBatch::DP_Instanced)
        return false;
    SF_ASSERT(PrimitiveBatch::DP_Instanced == 2);
    
    VertexHandler&    vh = VHandlerTable[fillType * 3 + batchType];
    unsigned          declIndex = ((ve->Attribute & VET_CompType_Mask) != VET_F32) ? 0 : 1;

    if (pPrevVDecl != vh.pDecl[declIndex])
    {
        pDevice->SetVertexDeclaration(vh.pDecl[declIndex]);
        pPrevVDecl = vh.pDecl[declIndex];
    }

    if ( pPrevVShader != vh.pVShader || !vh.VShaderInit)
    {
        // Delayed vertex shader creation
        if (!vh.VShaderInit)
        {
            VertexShaderDesc& desc = 
                ShaderManager_VertexShaderDescriptors[(fillType-1) * 3 + batchType];
            vh.pVShader = registerVertexShader(desc);
            vh.VShaderInit = true;
        }
        if (!vh.pVShader)
            return false;

        pDevice->SetVertexShader(vh.pVShader);
        pPrevVShader = vh.pVShader;
    }
    
    // Apply textures and their sample modes
    SF_COMPILER_ASSERT(PSBit_Multiply == FF_Multiply);
    SF_COMPILER_ASSERT(PSBit_AlphaWrite == FF_AlphaWrite);
    
    unsigned psIndex = (fillFlags & (FF_Multiply|FF_AlphaWrite)) | 
                       ((batchType == PrimitiveBatch::DP_Single) ? 0 : PSTable_InstancedBit) |
                       (PrimitivePSType << PSTable_PSTypeShift);
    
    if (PrevPSIndex != psIndex)
    {
        // Delayed pixel shader creation
        if (!PSInitTable[psIndex])
        {
            PSTable[psIndex] = 
                registerPixelShader(ShaderManager_PixelShaderDescriptors[PrimitivePSType-1]|
                                    (psIndex & 0x7));
            PSInitTable[psIndex] = true;
        }
        if (!PSTable[psIndex])
            return false;

        pDevice->SetPixelShader(PSTable[psIndex]);
        PrevPSIndex = psIndex;
    }
    
    return true;
}

bool ShaderManager::ApplyClearMaskFill()
{
    // We use batched solid-color fill.    
    
    PrimitiveFillType fillType = Profiler->GetFillType(PrimFill_Mask);
    TexturePShader* pshader = &ShaderManager_FillToPixelShader[fillType]; 
    PrimitivePSType = pshader->GetShader();

    // TBD: It may make sense to pre-register a vertex format for MaskFill instead.

    // TBD: We may need to use a vertex with Z value if we plan to clear depth
    //      with draw commands in the future.
    return ApplyPrimitiveBatchFill(fillType, 0,
                                   &VertexXY16i::Format, PrimitiveBatch::DP_Batch);
}

bool ShaderManager::ApplySolidColorFill(Color c)
{    
    PrimitiveFillType fillType = Profiler->GetFillType(PrimFill_SolidColor);
    TexturePShader* pshader = &ShaderManager_FillToPixelShader[fillType]; 
    PrimitivePSType = pshader->GetShader();

    applySolidColorConstant(c);
    return ApplyPrimitiveBatchFill(fillType, 0,
                                   &VertexXY16i::Format, PrimitiveBatch::DP_Batch);
}


// Obtains formats that renderer will use for single, batches and instanced rendering of
// the specified source format.
//   - Filled in pointer may be the same as sourceFormat.
//   - 'instanced' format may be reported as 0, in which instancing is not supported for format.
void    ShaderManager::MapVertexFormat(
            PrimitiveFillType fill, const VertexFormat* sourceFormat,
            const VertexFormat** single,
            const VertexFormat** batch, const VertexFormat** instanced)
{

    // 1. Verify that sourceFormat has channels needed for fill.
    //    - same as matching destination non-instance formats
    //    - redundant since CheckVertexFormat iscalled in FillData constructor
    SF_ASSERT(PrimitiveFillData::CheckVertexFormat(fill, sourceFormat));    

    // Map: Fill -> VertexFormat, VertexDeclaration:
    //                16bit, 32bit Pos

    const VertexElement *ve = sourceFormat->GetElement(VET_Pos, VET_Usage_Mask);
    SF_ASSERT(ve);        
    unsigned            declIndex = ((ve->Attribute & VET_CompType_Mask) != VET_F32) ? 0 : 1;

    // 2. Get float/NoFloat value from VertexFormat
    // Index above based on this.
    
    *single    = VHandlerTable[fill * 3].pFormat[declIndex];
    *batch     = VHandlerTable[fill * 3 + 1].pFormat[declIndex];
    *instanced = VHandlerTable[fill * 3 + 2].pFormat[declIndex];
}



}}} // Scaleform::Render::D3D9
