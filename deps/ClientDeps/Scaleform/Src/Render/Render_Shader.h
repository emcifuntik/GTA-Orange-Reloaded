/**********************************************************************

PublicHeader:   Render
Filename    :   
Content     :   
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_Shader_H
#define INC_SF_Render_Shader_H

#include "Kernel/SF_RefCount.h"
#include "Kernel/SF_Array.h"
#include "Kernel/SF_Hash.h"

#include "Render_Matrix2x4.h"
#include "Render_Matrix4x4.h"
#include "Render_Color.h"
#include "Render_CxForm.h"
#include "Render_HAL.h"
#include "Render_MatrixPool.h"
#include "Render_Stats.h"
#include "Render_Containers.h"
#include "Render_Primitive.h"
#include "Render_Gradients.h"
#include "Render_Vertex.h"
#include "Render_Types2D.h"
#include "Render_Math2D.h"

namespace Scaleform { namespace Render {

template<typename Uniforms, typename Shader>
class ShaderInterfaceBase
{
protected:
    float UniformData[Uniforms::SU_TotalSize];
    bool  UniformSet[Uniforms::SU_Count];

public:

    unsigned GetParameterStage(const Shader& sd, unsigned var, int index = 0) const
    {
        if (sd->pVDesc->BatchUniforms[var].Offset >= 0 && index < sd->pVDesc->BatchUniforms[var].Size)
            return 0x10000 | sd->pVDesc->BatchUniforms[var].Size;
        else if (sd->pVDesc->Uniforms[var].Location >= 0 && index < sd->pVDesc->Uniforms[var].Size)
            return 0x10000 | sd->pVDesc->Uniforms[var].Size / sd->pVDesc->Uniforms[var].ElementSize;
        else if (sd->pFDesc->BatchUniforms[var].Offset >= 0 && index < sd->pFDesc->BatchUniforms[var].Size)
            return 0x20000 | sd->pFDesc->BatchUniforms[var].Size;
        else if (sd->pFDesc->Uniforms[var].Location >= 0 && index < sd->pFDesc->Uniforms[var].Size)
            return 0x20000 | sd->pFDesc->Uniforms[var].Size / sd->pVDesc->Uniforms[var].ElementSize;
        else
            return 0;
    }

    void BeginPrimitive()
    {
    	memset(UniformSet, 0, sizeof(UniformSet));
    }

    void SetUniform0(const Shader& sd, unsigned var, const float* v, unsigned n, unsigned index = 0, unsigned batch = 0)
    {
	    SF_ASSERT(batch == 0);
	    SF_ASSERT(sd->pFDesc->Uniforms[var].ElementSize * int(index+n) <= sd->pFDesc->Uniforms[var].Size
               || sd->pVDesc->Uniforms[var].ElementSize * int(index+n) <= sd->pVDesc->Uniforms[var].Size);

        if (sd->pVDesc->Uniforms[var].Size)
    	    memcpy(UniformData + sd->pVDesc->Uniforms[var].ShadowOffset + sd->pVDesc->Uniforms[var].ElementSize * index, v, n * sizeof(float));
        if (sd->pFDesc->Uniforms[var].Size)
            memcpy(UniformData + sd->pFDesc->Uniforms[var].ShadowOffset + sd->pFDesc->Uniforms[var].ElementSize * index, v, n * sizeof(float));
	    UniformSet[var] = 1;
    }

    void SetUniform(const Shader& sd, unsigned var, const float* v, unsigned n, unsigned index = 0, unsigned batch = 0)
    {
	    if (sd->pVDesc->BatchUniforms[var].Offset >= 0)
	    {
	        unsigned bvar = sd->pVDesc->BatchUniforms[var].Array;
	        SetUniform0(sd, sd->pVDesc->BatchUniforms[var].Array, v, n,
			    batch * sd->pVDesc->Uniforms[bvar].BatchSize + sd->pVDesc->BatchUniforms[var].Offset + index);
	    }
        else if (sd->pFDesc->BatchUniforms[var].Offset >= 0)
        {
            unsigned bvar = sd->pFDesc->BatchUniforms[var].Array;
            SetUniform0(sd, sd->pFDesc->BatchUniforms[var].Array, v, n,
                batch * sd->pFDesc->Uniforms[bvar].BatchSize + sd->pFDesc->BatchUniforms[var].Offset + index);
        }
	    else
	        SetUniform0(sd, var, v, n, index, batch);
    }

    void SetColor(const Shader& sd, unsigned var, Color c, unsigned index = 0, unsigned batch = 0)
    {
	    const float mult  = 1.0f / 255.0f;
	    float rgba[4] = 
	    {
	        c.GetRed() * mult,  c.GetGreen() * mult,
	        c.GetBlue() * mult, c.GetAlpha() * mult
	    };
	    SetUniform(sd, var, rgba, 4, index, batch);
    }

    void SetMatrix(const Shader& sd, unsigned var, const Matrix2F &m, unsigned index = 0, unsigned batch = 0)
    {
	    float rows[2][4];
	    m.GetAsFloat2x4(rows);
	    SetUniform(sd, var, &rows[0][0], 8, index*2, batch);
    }

    void SetMatrix(const Shader& sd, unsigned var, const Matrix2F &m1, const HMatrix &m2, const MatrixState &Matrices, unsigned index = 0, unsigned batch = 0)
    {
        if (m2.Has3D())
        {
            Matrix4F m = Matrices.GetUVP() * m2.GetMatrix3D() * m1;

            float rows[4][4];
            m.GetAsFloat4x4(rows);
            SetUniform(sd, var, &rows[0][0], 16, index, batch);
        }
        else
        {
            Matrix2F  m(m1, m2.GetMatrix2D(), Matrices.ViewportMatrix);

            float rows[2][4];
            m.GetAsFloat2x4(rows);
            SetUniform(sd, var, &rows[0][0], 8, index*2, batch);
        }
    }

    void SetCxform(const Shader& sd, Cxform cx, unsigned index = 0, unsigned batch = 0)
    {
        float rows[2][4];
        cx.GetAsFloat2x4(rows);

        SetUniform(sd, Uniforms::SU_cxmul, rows[0], 4, index, batch);
        SetUniform(sd, Uniforms::SU_cxadd, rows[1], 4, index, batch);
    }
};

template<typename FShaderDesc, typename VShaderDesc,
         class Uniforms, class ShaderInterface, class NativeTexture>
class StaticShaderManager
{
    typedef typename VShaderDesc::VertexAttrDesc VertexAttrDesc;
    typedef typename VShaderDesc::ShaderType VShaderType;
    typedef typename FShaderDesc::ShaderType FShaderType;
    typedef typename ShaderInterface::Shader Shader;

    MultiKeyCollection<VertexElement, VertexFormat, 32>        VFormats;
    ProfileViews*                                              Profiler;

public:
    StaticShaderManager(ProfileViews* prof) : Profiler(prof) {}

    FShaderType StaticShaderForFill (PrimitiveFillType fill, UInt32 fillflags, bool batch)
    {
	    unsigned shader = FShaderDesc::FS_None;
	    bool text = 0;

	    switch (fill)
	    {
        default:
	    case PrimFill_None:
	    case PrimFill_Mask:
	    case PrimFill_SolidColor:
            text = 1;
	        shader = FShaderDesc::FS_FSolid;
	        break;

	    case PrimFill_VColor:
	        shader = FShaderDesc::FS_FVertex;
	        break;

	    case PrimFill_VColor_EAlpha:
	        shader = FShaderDesc::FS_FVertexVertex;
	        break;

	    case PrimFill_Texture:
	        shader = FShaderDesc::FS_FTexTG;
	        break;

	    case PrimFill_Texture_EAlpha:
	        // XXX
	        shader = FShaderDesc::FS_FTexTG;
	        break;
    		
	    case PrimFill_Texture_VColor:
	        // XXX
	        shader = FShaderDesc::FS_FTexTGVertex;
	        break;

	    case PrimFill_Texture_VColor_EAlpha:
	        shader = FShaderDesc::FS_FTexTGVertex;
	        break;

	    case PrimFill_2Texture:
	    case PrimFill_2Texture_EAlpha:
	        shader = FShaderDesc::FS_FTexTGTexTG;
	        break;

	    case PrimFill_UVTexture:
            text = 1;
            shader = FShaderDesc::FS_FTextColor;
            break;

        case PrimFill_UVTextureAlpha_VColor:
	        text = 1;
	        shader = FShaderDesc::FS_FText;
	        break;
	    }

	    if (batch)
	        shader += text ? FShaderDesc::FS_text_Batch : FShaderDesc::FS_base_Batch;

	    if (fillflags & FF_Multiply)
	        shader += text ? FShaderDesc::FS_text_Mul : FShaderDesc::FS_base_Mul;

	    return (FShaderType)shader;
    }

    const VertexFormat* GetVertexFormat(VertexElement* pelements, unsigned count, unsigned size)
    {
        VertexFormat  *pformat   = VFormats.Find(pelements, count);
        if (pformat)
            return pformat;

        pformat = VFormats.Add(&pelements, pelements, count);
        if (!pformat)
            return 0;

        pformat->Size      = size;
        pformat->pElements = pelements;
        return pformat;
    }

    void    MapVertexFormat(PrimitiveFillType fill, const VertexFormat* sourceFormat,
			    const VertexFormat** single,
			    const VertexFormat** batch, const VertexFormat** instanced)
    {
        FShaderType          shader = this->StaticShaderForFill(fill, 0, 0);
        const VShaderDesc*   pshader = VShaderDesc::Descs[FShaderDesc::VShaderForFShader[shader]];
        const VertexAttrDesc* psvf = pshader->Attributes;
        VertexElement        outf[8];
        unsigned             size = 0;
        int                  j = 0;

        for (int i = 0; i < pshader->NumAttribs; i++)
        {
            if ((psvf[i].Attr & (VET_Usage_Mask | VET_Index_Mask | VET_Components_Mask)) == (VET_Color | (1 << VET_Index_Shift) | 4))
            {
                // XXX - change shaders to use .rg instead of .ra for these
                outf[j].Offset = size;
                outf[j].Attribute = VET_T0Weight8;
                j++;
                outf[j].Offset = size+3;
                outf[j].Attribute = VET_FactorAlpha8;
                j++;
                size += 4;
                continue;
            }

            const VertexElement* pv = sourceFormat->GetElement(psvf[i].Attr & (VET_Usage_Mask|VET_Index_Mask), VET_Usage_Mask|VET_Index_Mask);
            if (!pv)
            {
                *batch = *single = *instanced = NULL;
                return;
            }
            outf[j] = *pv;
            outf[j].Offset = size;
            SF_ASSERT((outf[j].Attribute & VET_Components_Mask) > 0 && (outf[j].Attribute & VET_Components_Mask) <= 4);
            size += outf[j].Size();
            j++;
        }
        outf[j].Attribute = VET_None;
        outf[j].Offset = 0;
        *single = GetVertexFormat(outf, j+1, size);
        *instanced = 0;

        outf[j].Attribute = VET_Instance8;
        outf[j].Offset = size;
        outf[j+1].Attribute = VET_None;
        outf[j+1].Offset = 0;
        size += outf[j].Size();
        *batch = GetVertexFormat(outf, j+2, size);
    }

    const Shader& SetPrimitiveFill(PrimitiveFill* pfill, unsigned fillFlags, unsigned batchType, unsigned meshCount,
                                   const MatrixState& Matrices, const Primitive::MeshEntry* pmeshes, ShaderInterface* psi)
    {
        PrimitiveFillType fillType = pfill->GetType(); 

        FShaderType shader = this->StaticShaderForFill(fillType, fillFlags, batchType == PrimitiveBatch::DP_Batch);
        VShaderType vshader = FShaderDesc::VShaderForFShader[shader];

        // Check flag to determine whether to use a 3D shader.
        if (fillFlags & FF_3DProjection)
            vshader = (VShaderType)((int)vshader + (int)VShaderDesc::VS_base_Position3d);

        psi->SetStaticShader(vshader, shader);
        psi->BeginPrimitive();
        const Shader& pso = psi->GetCurrentShaders();
        bool solid = (fillType == PrimFill_None || fillType == PrimFill_Mask || fillType == PrimFill_SolidColor);

        if (solid)
            psi->SetColor(pso, Uniforms::SU_cxmul, Profiler->GetColor(pfill->GetSolidColor()));
        else if (fillType >= PrimFill_Texture)
        {     
            NativeTexture*  pt0 = (NativeTexture*)pfill->GetTexture(0);
            SF_ASSERT(pt0);

            /* XXX
               pfsd = pfsd->MatchImageFormat(pt0->GetImageFormat());
               if (!pfsd) // No shader existed for specified format!!
               {
               PrimitivePSType = PS_None;
               return false;
               }
            */
            ImageFillMode fm0 = pfill->GetFillMode(0);

            unsigned stage = 0;
            psi->SetTexture(pso, stage, pt0, fm0);
            stage += pt0->GetPlaneCount();

            if ((fillType == PrimFill_2Texture) || (fillType == PrimFill_2Texture_EAlpha))
            {
                NativeTexture* pt1 = (NativeTexture*)pfill->GetTexture(1);
                ImageFillMode  fm1 = pfill->GetFillMode(1);

                psi->SetTexture(pso, stage, pt1, fm1);
            }
        }

        if (batchType < PrimitiveBatch::DP_Instanced)
        {
            unsigned tmCount = (psi->GetParameterStage(pso, Uniforms::SU_texgen) & 0xffff) >> 1;

            for (unsigned i = 0; i < meshCount; i++)
            {
                psi->SetMatrix(pso, Uniforms::SU_mvp, pmeshes[i].pMesh->VertexMatrix, pmeshes[i].M, Matrices, 0, i);

                if (!solid)
                    psi->SetCxform(pso, Profiler->GetCxform(pmeshes[i].M.GetCxform()), 0, i);

                for (unsigned tm = 0; tm < tmCount; tm++)
                {
                    Matrix2F m(pmeshes[i].pMesh->VertexMatrix);
                    m.Append(pmeshes[i].M.GetTextureMatrix(tm));
                    psi->SetMatrix(pso, Uniforms::SU_texgen, m, tm, i);
                }
            }
    	}

        psi->Finish(meshCount);

        return pso;
    }

    const Shader& SetFill(PrimitiveFillType fillType, unsigned fillFlags, unsigned batchType, ShaderInterface* psi)
    {
        FShaderType shader = this->StaticShaderForFill(fillType, fillFlags, batchType == PrimitiveBatch::DP_Batch);
        VShaderType vshader = FShaderDesc::VShaderForFShader[shader];

        // Check a flag, to determine whether to use a 3D shader.
        if (fillFlags & FF_3DProjection)
            vshader = (VShaderType)((int)vshader + (int)VShaderDesc::VS_base_Position3d);

        psi->SetStaticShader(vshader, shader);
        psi->BeginPrimitive();
        return psi->GetCurrentShaders();
    }
};

}}

#endif
