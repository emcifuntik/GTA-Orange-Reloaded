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

#ifndef INC_SF_GL_Shader_H
#define INC_SF_GL_Shader_H

#include "Render/GL/GL_Common.h"

#include "Render/Render_Shader.h"

#if defined(GL_ES_VERSION_2_0)
#include "Render/GL/GLES_Shaders.h"
#else
#include "Render/GL/GL_Shaders.h"
#endif

namespace Scaleform { namespace Render { namespace GL {

class HAL;
class Texture;
                              

struct ShaderObject
{
    HAL*                    pHal;
    const VertexShaderDesc* pVDesc;
    const FragShaderDesc*   pFDesc;
    GLuint                  Prog;
    GLint                   Uniforms[Uniform::SU_Count];
    UniformVar              AllUniforms[Uniform::SU_Count];
    int                     Attributes[8]; // XXX

    ShaderObject()
    {
        Prog = 0;
        pVDesc = 0;
        pFDesc = 0;
    }
    ~ShaderObject();

    bool Init(HAL* phal, FragShaderType fs, int vsoffset);

    inline  HAL* GetHAL() const { return pHal; }
};

class ShaderInterface : public ShaderInterfaceBase<Uniform, const ShaderObject*>
{
    HAL*  pHal;
    const ShaderObject* pCurShader;

    inline  HAL* GetHAL() const { return pHal; }

public:
    typedef const ShaderObject* Shader;

    ShaderInterface(HAL* phal) { pHal = phal; }

    const Shader&       GetCurrentShaders() const { return pCurShader; }
    bool                SetStaticShader(VertexShaderDesc::ShaderType vshader, FragShaderDesc::ShaderType shader);

    void                SetTexture(Shader sp, unsigned stage, Texture* ptexture, ImageFillMode fm);

    void                Finish(unsigned batchCount);
};

typedef StaticShaderManager<FragShaderDesc, VertexShaderDesc, Uniform, ShaderInterface, Texture> ShaderManager;

}}}

#endif
