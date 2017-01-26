#ifndef INC_SF_Render_GLES_Shaders_H
#define INC_SF_Render_GLES_Shaders_H

namespace Scaleform { namespace Render { namespace GL {

struct Uniform
{
    enum UniformType
    {
        SU_cxadd,
        SU_cxmul,
        SU_ffmuniforms,
        SU_ffuniforms,
        SU_mvp,
        SU_tex,
        SU_tex0,
        SU_tex1,
        SU_tex_a,
        SU_tex_u,
        SU_tex_v,
        SU_tex_y,
        SU_texgen,
        SU_vfmuniforms,
        SU_vfuniforms,
        SU_Count,

        SU_TotalSize = 1560
    };
};

enum ShaderFlags
{
    Shader_Batched  = 1,
};
enum ShaderStages
{
    Shader_Vertex   = 1,
    Shader_Fragment = 3,
};
struct UniformVar
{
	int Location;
	int ShadowOffset;
	int ElementSize, Size;
	int BatchSize;
};

struct BatchVar
{
	Uniform::UniformType Array;
	int Offset;
	int Size;
};

struct VertexShaderDesc
{
    enum ShaderType
    {
        VS_None = 0,
        VS_start_base,
        VS_VTexTGVacolorVafactor = 1,
        VS_VTexTGPosition3dVacolorVafactor,
        VS_VTexTGBatchVacolorVafactor,
        VS_VTexTGBatchPosition3dVacolorVafactor,
        VS_VVacolor,
        VS_VPosition3dVacolor,
        VS_VBatchVacolor,
        VS_VBatchPosition3dVacolor,
        VS_VTexTGTexTGVafactor,
        VS_VTexTGTexTGPosition3dVafactor,
        VS_VTexTGTexTGBatchVafactor,
        VS_VTexTGTexTGBatchPosition3dVafactor,
        VS_VVacolorVafactor,
        VS_VPosition3dVacolorVafactor,
        VS_VBatchVacolorVafactor,
        VS_VBatchPosition3dVacolorVafactor,
        VS_VTexTG,
        VS_VTexTGPosition3d,
        VS_VTexTGBatch,
        VS_VTexTGBatchPosition3d,
        VS_end_base = 20,
        VS_start_text,
        VS_VVatcVacolor = 21,
        VS_VPosition3dVatcVacolor,
        VS_VBatchVatcVacolor,
        VS_VBatchPosition3dVatcVacolor,
        VS_V,
        VS_VPosition3d,
        VS_VBatch,
        VS_VBatchPosition3d,
        VS_VVatc,
        VS_VPosition3dVatc,
        VS_VBatchVatc,
        VS_VBatchPosition3dVatc,
        VS_end_text = 32,
        VS_Count,

        VS_base_Position3d           = 0x00000001,
        VS_base_Batch                = 0x00000002,
        VS_text_Position3d           = 0x00000001,
        VS_text_Batch                = 0x00000002,
    };


    struct VertexAttrDesc
    {
    	const char* Name;
        UInt32      Attr;
    };

    UInt32         Flags;
    int            NumAttribs;
    VertexAttrDesc Attributes[8];
    const char*    pSource;
    UniformVar     Uniforms[Uniform::SU_Count];
    BatchVar       BatchUniforms[Uniform::SU_Count];

    enum
    {
        MaxAttributes = 8,
        MaxTexParams  = 8,
    };

    static const VertexShaderDesc* Descs[VS_Count];
};

typedef VertexShaderDesc::ShaderType VertexShaderType;
struct FragShaderDesc
{
    enum ShaderType
    {
        FS_None = 0,
        FS_start_base,
        FS_FTexTGVertex = 1,
        FS_FTexTGVertexCMatrix,
        FS_FTexTGVertexBatch,
        FS_FTexTGVertexBatchCMatrix,
        FS_FTexTGVertexMul,
        FS_FTexTGVertexCMatrixMul,
        FS_FTexTGVertexBatchMul,
        FS_FTexTGVertexBatchCMatrixMul,
        FS_FTexTGTexTG,
        FS_FTexTGTexTGCMatrix,
        FS_FTexTGTexTGBatch,
        FS_FTexTGTexTGBatchCMatrix,
        FS_FTexTGTexTGMul,
        FS_FTexTGTexTGCMatrixMul,
        FS_FTexTGTexTGBatchMul,
        FS_FTexTGTexTGBatchCMatrixMul,
        FS_FVertexVertex,
        FS_FVertexVertexCMatrix,
        FS_FVertexVertexBatch,
        FS_FVertexVertexBatchCMatrix,
        FS_FVertexVertexMul,
        FS_FVertexVertexCMatrixMul,
        FS_FVertexVertexBatchMul,
        FS_FVertexVertexBatchCMatrixMul,
        FS_FTexTG,
        FS_FTexTGCMatrix,
        FS_FTexTGBatch,
        FS_FTexTGBatchCMatrix,
        FS_FTexTGMul,
        FS_FTexTGCMatrixMul,
        FS_FTexTGBatchMul,
        FS_FTexTGBatchCMatrixMul,
        FS_FVertex,
        FS_FVertexCMatrix,
        FS_FVertexBatch,
        FS_FVertexBatchCMatrix,
        FS_FVertexMul,
        FS_FVertexCMatrixMul,
        FS_FVertexBatchMul,
        FS_FVertexBatchCMatrixMul,
        FS_end_base = 40,
        FS_start_text,
        FS_FTextYUV = 41,
        FS_FTextYUVTextYUVA,
        FS_FTextYUVCMatrix,
        FS_FTextYUVTextYUVACMatrix,
        FS_FTextYUVBatch,
        FS_FTextYUVBatchTextYUVA,
        FS_FTextYUVBatchCMatrix,
        FS_FTextYUVBatchTextYUVACMatrix,
        FS_FTextYUVMul,
        FS_FTextYUVTextYUVAMul,
        FS_FTextYUVCMatrixMul,
        FS_FTextYUVTextYUVACMatrixMul,
        FS_FTextYUVBatchMul,
        FS_FTextYUVBatchTextYUVAMul,
        FS_FTextYUVBatchCMatrixMul,
        FS_FTextYUVBatchTextYUVACMatrixMul,
        FS_FTextColor,
        FS_FTextColorCMatrix = 59,
        FS_FTextColorBatch = 61,
        FS_FTextColorBatchCMatrix = 63,
        FS_FTextColorMul = 65,
        FS_FTextColorCMatrixMul = 67,
        FS_FTextColorBatchMul = 69,
        FS_FTextColorBatchCMatrixMul = 71,
        FS_FText = 73,
        FS_FTextBatch = 77,
        FS_FTextMul = 81,
        FS_FTextBatchMul = 85,
        FS_FSolid = 89,
        FS_FSolidBatch = 93,
        FS_end_text = 93,
        FS_Count,

        FS_base_CMatrix              = 0x00000001,
        FS_base_Batch                = 0x00000002,
        FS_base_Mul                  = 0x00000004,
        FS_text_TextYUVA             = 0x00000001,
        FS_text_CMatrix              = 0x00000002,
        FS_text_Batch                = 0x00000004,
        FS_text_Mul                  = 0x00000008,
    };


    UInt32      Flags;
    int         TexParams[8];
    const char* pSource;
    UniformVar  Uniforms[Uniform::SU_Count];
    BatchVar    BatchUniforms[Uniform::SU_Count];

    enum
    {
        MaxAttributes = 8,
        MaxTexParams  = 8,
    };

    static const FragShaderDesc* Descs[FS_Count];
    static VertexShaderType VShaderForFShader[FragShaderDesc::FS_Count];
};

typedef FragShaderDesc::ShaderType FragShaderType;

}}}

#endif
