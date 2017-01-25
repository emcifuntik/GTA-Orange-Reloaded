/**********************************************************************

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

#include "Render/GL/GL_Shader.h"
#include "Render/GL/GL_HAL.h"
#include "Kernel/SF_Debug.h"

#if defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_ExtensionMacros.h"
#include "Render/GL/GLES_Shaders.cpp"
#else
#include "Render/GL/GL_ExtensionMacros.h"
#include "Render/GL/GL_Shaders.cpp"
#endif

namespace Scaleform { namespace Render { namespace GL {

extern const char* ShaderUniformNames[Uniform::SU_Count];

bool ShaderObject::Init(HAL* phal, FragShaderType fs, int vsoffset)
{
    pHal = phal;
    if (Prog)
        glDeleteProgram(Prog);

    pVDesc = VertexShaderDesc::Descs[FragShaderDesc::VShaderForFShader[fs] + vsoffset];
    pFDesc = FragShaderDesc::Descs[fs];

    GLint result;
    GLuint vp = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vp, 1, const_cast<const char**>(&pVDesc->pSource), 0);
    glCompileShader(vp);
    glGetShaderiv(vp, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        GLchar msg[512];
        glGetShaderInfoLog(vp, sizeof(msg), 0, msg);
        SF_DEBUG_ERROR2(1, "%s: %s\n", pVDesc->pSource, msg);
        glDeleteShader(vp);
        return 0;
    }
    Prog = glCreateProgram();
    glAttachShader(Prog, vp);
    GLuint fp = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fp, 1, const_cast<const char**>(&pFDesc->pSource), 0);
    glCompileShader(fp);
    glGetShaderiv(fp, GL_COMPILE_STATUS, &result);
    if (!result)
    {
        GLchar msg[1512];
        glGetShaderInfoLog(fp, sizeof(msg), 0, msg);
        SF_DEBUG_ERROR2(1, "%s: %s\n", pFDesc->pSource, msg);
        glDeleteShader(fp);
        glDeleteProgram(Prog);
        return 0;
    }
    glAttachShader(Prog, fp);

    for (int i = 0; i < pVDesc->NumAttribs; i++)
        glBindAttribLocation(Prog, i, pVDesc->Attributes[i].Name);

    glLinkProgram(Prog);
    glDeleteShader(vp);
    glDeleteShader(fp);
    glGetProgramiv(Prog, GL_LINK_STATUS, &result);
    if (!result)
    {
        GLchar msg[512];
        glGetProgramInfoLog(Prog, sizeof(msg), 0, msg);
        SF_DEBUG_ERROR1(1, "link: %s\n", msg);
        glDeleteProgram(Prog);
    	Prog = 0;
    }

    for (unsigned i = 0; i < Uniform::SU_Count; i++)
        if (pFDesc->Uniforms[i].Location >= 0 || pVDesc->Uniforms[i].Location >= 0)
        {
            Uniforms[i] = glGetUniformLocation(Prog, ShaderUniformNames[i]);
            SF_ASSERT(Uniforms[i] >= 0);

            if (pFDesc->Uniforms[i].Location >= 0)
            {
                SF_ASSERT(pVDesc->Uniforms[i].Location < 0);
                AllUniforms[i] = pFDesc->Uniforms[i];
            }
            else
                AllUniforms[i] = pVDesc->Uniforms[i];
        }
        else
            Uniforms[i] = -1;

    return result != 0;
}

ShaderObject::~ShaderObject()
{
    if (Prog)
        glDeleteProgram(Prog);
}


bool ShaderInterface::SetStaticShader(VertexShaderDesc::ShaderType vshader, FragShaderDesc::ShaderType shader)
{
    if (vshader == FragShaderDesc::VShaderForFShader[shader] + VertexShaderDesc::VS_base_Position3d)
        pCurShader = &pHal->StaticShaders[shader+FragShaderDesc::FS_Count];
    else
    {
        SF_ASSERT(vshader == FragShaderDesc::VShaderForFShader[shader]);
        pCurShader = &pHal->StaticShaders[shader];
    }

    glUseProgram(pCurShader->Prog);

    return true;
}

void ShaderInterface::SetTexture(Shader, unsigned stage, Texture* ptexture, ImageFillMode fm)
{
    GLint minfilter = (fm.GetSampleMode() == Sample_Point) ? GL_NEAREST : (ptexture->MipLevels>1 ? GL_LINEAR_MIPMAP_LINEAR  : GL_LINEAR);
    GLint magfilter = (fm.GetSampleMode() == Sample_Point) ? GL_NEAREST : GL_LINEAR;
    GLint address = (fm.GetWrapMode() == Wrap_Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);

    for (unsigned plane = 0; plane < ptexture->GetTextureStageCount(); plane++)
    {
        int stageIndex = stage + plane;

        glActiveTexture(GL_TEXTURE0 + stageIndex);
        glBindTexture(GL_TEXTURE_2D, ptexture->pTextures[plane].TexId);
        glUniform1i(pCurShader->Uniforms[pCurShader->pFDesc->TexParams[stageIndex]], stageIndex);

        if (ptexture->LastMinFilter != minfilter || ptexture->LastAddress != address)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minfilter);              
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, address);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, address);

            ptexture->LastMinFilter = minfilter;
            ptexture->LastAddress = address;
        }
    }
}

void ShaderInterface::Finish(unsigned batchCount)
{
    for (int var = 0; var < Uniform::SU_Count; var++)
        if (UniformSet[var])
        {
            unsigned size;
            if (pCurShader->AllUniforms[var].BatchSize > 0)
                size = batchCount * pCurShader->AllUniforms[var].BatchSize;
            else
                size = pCurShader->AllUniforms[var].Size / pCurShader->AllUniforms[var].ElementSize;

            switch (pCurShader->AllUniforms[var].ElementSize)
            {
                case 16:
                    glUniformMatrix4fv(pCurShader->Uniforms[var], size, false /* transpose */,
                        UniformData + pCurShader->AllUniforms[var].ShadowOffset);
                    break;
                case 4:
                    glUniform4fv(pCurShader->Uniforms[var], size,
                        UniformData + pCurShader->AllUniforms[var].ShadowOffset);
                    break;
                case 3:
                    glUniform3fv(pCurShader->Uniforms[var], size,
                        UniformData + pCurShader->AllUniforms[var].ShadowOffset);
                    break;
                case 2:
                    glUniform2fv(pCurShader->Uniforms[var], size,
                        UniformData + pCurShader->AllUniforms[var].ShadowOffset);
                    break;
                case 1:
                    glUniform1fv(pCurShader->Uniforms[var], size,
                        UniformData + pCurShader->AllUniforms[var].ShadowOffset);
                    break;

                default:
                    SF_ASSERT(0);
            }
        }

    memset(UniformSet, 0, Uniform::SU_Count);
}

}}}
