
#include "Render/GL/GL_Common.h"

#ifdef SF_GL_RUNTIME_LINK

namespace Scaleform { namespace Render { namespace GL {

bool Extensions::Init()
{
    bool result = 1;
    p_glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC) SF_GL_RUNTIME_LINK("glGenerateMipmapEXT");

    p_glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC) SF_GL_RUNTIME_LINK("glFramebufferTexture2DEXT");

    p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBuffer");
    if (!p_glMapBuffer)
    {
         p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferARB");
         if (!p_glMapBuffer)
         {
              p_glMapBuffer = (PFNGLMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glMapBufferEXT");
              if (!p_glMapBuffer)
                  result = 0;
         }
    }


    p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffers");
    if (!p_glGenBuffers)
    {
         p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffersARB");
         if (!p_glGenBuffers)
         {
              p_glGenBuffers = (PFNGLGENBUFFERSPROC) SF_GL_RUNTIME_LINK("glGenBuffersEXT");
              if (!p_glGenBuffers)
                  result = 0;
         }
    }


    p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShader");
    if (!p_glCreateShader)
    {
         p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShaderARB");
         if (!p_glCreateShader)
         {
              p_glCreateShader = (PFNGLCREATESHADERPROC) SF_GL_RUNTIME_LINK("glCreateShaderEXT");
              if (!p_glCreateShader)
                  result = 0;
         }
    }


    p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferData");
    if (!p_glBufferData)
    {
         p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferDataARB");
         if (!p_glBufferData)
         {
              p_glBufferData = (PFNGLBUFFERDATAPROC) SF_GL_RUNTIME_LINK("glBufferDataEXT");
              if (!p_glBufferData)
                  result = 0;
         }
    }


    p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2D");
    if (!p_glCompressedTexImage2D)
    {
         p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2DARB");
         if (!p_glCompressedTexImage2D)
         {
              p_glCompressedTexImage2D = (PFNGLCOMPRESSEDTEXIMAGE2DPROC) SF_GL_RUNTIME_LINK("glCompressedTexImage2DEXT");
              if (!p_glCompressedTexImage2D)
                  result = 0;
         }
    }


    p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgram");
    if (!p_glLinkProgram)
    {
         p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgramARB");
         if (!p_glLinkProgram)
         {
              p_glLinkProgram = (PFNGLLINKPROGRAMPROC) SF_GL_RUNTIME_LINK("glLinkProgramEXT");
              if (!p_glLinkProgram)
                  result = 0;
         }
    }


    p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTexture");
    if (!p_glActiveTexture)
    {
         p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTextureARB");
         if (!p_glActiveTexture)
         {
              p_glActiveTexture = (PFNGLACTIVETEXTUREPROC) SF_GL_RUNTIME_LINK("glActiveTextureEXT");
              if (!p_glActiveTexture)
                  result = 0;
         }
    }


    p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramiv");
    if (!p_glGetProgramiv)
    {
         p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivARB");
         if (!p_glGetProgramiv)
         {
              p_glGetProgramiv = (PFNGLGETPROGRAMIVPROC) SF_GL_RUNTIME_LINK("glGetProgramivEXT");
              if (!p_glGetProgramiv)
                  result = 0;
         }
    }


    p_glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC) SF_GL_RUNTIME_LINK("glCheckFramebufferStatusEXT");

    p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fv");
    if (!p_glUniform1fv)
    {
         p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fvARB");
         if (!p_glUniform1fv)
         {
              p_glUniform1fv = (PFNGLUNIFORM1FVPROC) SF_GL_RUNTIME_LINK("glUniform1fvEXT");
              if (!p_glUniform1fv)
                  result = 0;
         }
    }


    p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquation");
    if (!p_glBlendEquation)
    {
         p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquationARB");
         if (!p_glBlendEquation)
         {
              p_glBlendEquation = (PFNGLBLENDEQUATIONPROC) SF_GL_RUNTIME_LINK("glBlendEquationEXT");
              if (!p_glBlendEquation)
                  result = 0;
         }
    }


    p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArray");
    if (!p_glDisableVertexAttribArray)
    {
         p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArrayARB");
         if (!p_glDisableVertexAttribArray)
         {
              p_glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glDisableVertexAttribArrayEXT");
              if (!p_glDisableVertexAttribArray)
                  result = 0;
         }
    }


    p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocation");
    if (!p_glBindAttribLocation)
    {
         p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocationARB");
         if (!p_glBindAttribLocation)
         {
              p_glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glBindAttribLocationEXT");
              if (!p_glBindAttribLocation)
                  result = 0;
         }
    }


    p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShader");
    if (!p_glCompileShader)
    {
         p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShaderARB");
         if (!p_glCompileShader)
         {
              p_glCompileShader = (PFNGLCOMPILESHADERPROC) SF_GL_RUNTIME_LINK("glCompileShaderEXT");
              if (!p_glCompileShader)
                  result = 0;
         }
    }


    p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShader");
    if (!p_glAttachShader)
    {
         p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShaderARB");
         if (!p_glAttachShader)
         {
              p_glAttachShader = (PFNGLATTACHSHADERPROC) SF_GL_RUNTIME_LINK("glAttachShaderEXT");
              if (!p_glAttachShader)
                  result = 0;
         }
    }


    p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocation");
    if (!p_glGetAttribLocation)
    {
         p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocationARB");
         if (!p_glGetAttribLocation)
         {
              p_glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetAttribLocationEXT");
              if (!p_glGetAttribLocation)
                  result = 0;
         }
    }


    p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparate");
    if (!p_glBlendFuncSeparate)
    {
         p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateARB");
         if (!p_glBlendFuncSeparate)
         {
              p_glBlendFuncSeparate = (PFNGLBLENDFUNCSEPARATEPROC) SF_GL_RUNTIME_LINK("glBlendFuncSeparateEXT");
              if (!p_glBlendFuncSeparate)
                  result = 0;
         }
    }


    p_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC) SF_GL_RUNTIME_LINK("glGenFramebuffersEXT");

    p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1i");
    if (!p_glUniform1i)
    {
         p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1iARB");
         if (!p_glUniform1i)
         {
              p_glUniform1i = (PFNGLUNIFORM1IPROC) SF_GL_RUNTIME_LINK("glUniform1iEXT");
              if (!p_glUniform1i)
                  result = 0;
         }
    }


    p_glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC) SF_GL_RUNTIME_LINK("glDeleteRenderbuffersEXT");

    p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderiv");
    if (!p_glGetShaderiv)
    {
         p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderivARB");
         if (!p_glGetShaderiv)
         {
              p_glGetShaderiv = (PFNGLGETSHADERIVPROC) SF_GL_RUNTIME_LINK("glGetShaderivEXT");
              if (!p_glGetShaderiv)
                  result = 0;
         }
    }


    p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgram");
    if (!p_glDeleteProgram)
    {
         p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgramARB");
         if (!p_glDeleteProgram)
         {
              p_glDeleteProgram = (PFNGLDELETEPROGRAMPROC) SF_GL_RUNTIME_LINK("glDeleteProgramEXT");
              if (!p_glDeleteProgram)
                  result = 0;
         }
    }


    p_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC) SF_GL_RUNTIME_LINK("glDeleteFramebuffersEXT");

    p_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC) SF_GL_RUNTIME_LINK("glBindFramebufferEXT");

    p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgram");
    if (!p_glCreateProgram)
    {
         p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgramARB");
         if (!p_glCreateProgram)
         {
              p_glCreateProgram = (PFNGLCREATEPROGRAMPROC) SF_GL_RUNTIME_LINK("glCreateProgramEXT");
              if (!p_glCreateProgram)
                  result = 0;
         }
    }


    p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgram");
    if (!p_glUseProgram)
    {
         p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgramARB");
         if (!p_glUseProgram)
         {
              p_glUseProgram = (PFNGLUSEPROGRAMPROC) SF_GL_RUNTIME_LINK("glUseProgramEXT");
              if (!p_glUseProgram)
                  result = 0;
         }
    }


    p_glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC) SF_GL_RUNTIME_LINK("glBindRenderbufferEXT");

    p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShader");
    if (!p_glDeleteShader)
    {
         p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShaderARB");
         if (!p_glDeleteShader)
         {
              p_glDeleteShader = (PFNGLDELETESHADERPROC) SF_GL_RUNTIME_LINK("glDeleteShaderEXT");
              if (!p_glDeleteShader)
                  result = 0;
         }
    }


    p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLog");
    if (!p_glGetProgramInfoLog)
    {
         p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLogARB");
         if (!p_glGetProgramInfoLog)
         {
              p_glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetProgramInfoLogEXT");
              if (!p_glGetProgramInfoLog)
                  result = 0;
         }
    }


    p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffers");
    if (!p_glDeleteBuffers)
    {
         p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffersARB");
         if (!p_glDeleteBuffers)
         {
              p_glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) SF_GL_RUNTIME_LINK("glDeleteBuffersEXT");
              if (!p_glDeleteBuffers)
                  result = 0;
         }
    }


    p_glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC) SF_GL_RUNTIME_LINK("glGenRenderbuffersEXT");

    p_glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC) SF_GL_RUNTIME_LINK("glRenderbufferStorageEXT");

    p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSource");
    if (!p_glShaderSource)
    {
         p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSourceARB");
         if (!p_glShaderSource)
         {
              p_glShaderSource = (PFNGLSHADERSOURCEPROC) SF_GL_RUNTIME_LINK("glShaderSourceEXT");
              if (!p_glShaderSource)
                  result = 0;
         }
    }


    p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBuffer");
    if (!p_glBindBuffer)
    {
         p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBufferARB");
         if (!p_glBindBuffer)
         {
              p_glBindBuffer = (PFNGLBINDBUFFERPROC) SF_GL_RUNTIME_LINK("glBindBufferEXT");
              if (!p_glBindBuffer)
                  result = 0;
         }
    }


    p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLog");
    if (!p_glGetShaderInfoLog)
    {
         p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLogARB");
         if (!p_glGetShaderInfoLog)
         {
              p_glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) SF_GL_RUNTIME_LINK("glGetShaderInfoLogEXT");
              if (!p_glGetShaderInfoLog)
                  result = 0;
         }
    }


    p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fv");
    if (!p_glUniform4fv)
    {
         p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fvARB");
         if (!p_glUniform4fv)
         {
              p_glUniform4fv = (PFNGLUNIFORM4FVPROC) SF_GL_RUNTIME_LINK("glUniform4fvEXT");
              if (!p_glUniform4fv)
                  result = 0;
         }
    }


    p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArray");
    if (!p_glEnableVertexAttribArray)
    {
         p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArrayARB");
         if (!p_glEnableVertexAttribArray)
         {
              p_glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) SF_GL_RUNTIME_LINK("glEnableVertexAttribArrayEXT");
              if (!p_glEnableVertexAttribArray)
                  result = 0;
         }
    }


    p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fv");
    if (!p_glUniformMatrix4fv)
    {
         p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fvARB");
         if (!p_glUniformMatrix4fv)
         {
              p_glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) SF_GL_RUNTIME_LINK("glUniformMatrix4fvEXT");
              if (!p_glUniformMatrix4fv)
                  result = 0;
         }
    }


    p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBuffer");
    if (!p_glUnmapBuffer)
    {
         p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferARB");
         if (!p_glUnmapBuffer)
         {
              p_glUnmapBuffer = (PFNGLUNMAPBUFFERPROC) SF_GL_RUNTIME_LINK("glUnmapBufferEXT");
              if (!p_glUnmapBuffer)
                  result = 0;
         }
    }


    p_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC) SF_GL_RUNTIME_LINK("glFramebufferRenderbufferEXT");

    p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointer");
    if (!p_glVertexAttribPointer)
    {
         p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerARB");
         if (!p_glVertexAttribPointer)
         {
              p_glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) SF_GL_RUNTIME_LINK("glVertexAttribPointerEXT");
              if (!p_glVertexAttribPointer)
                  result = 0;
         }
    }


    p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocation");
    if (!p_glGetUniformLocation)
    {
         p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocationARB");
         if (!p_glGetUniformLocation)
         {
              p_glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) SF_GL_RUNTIME_LINK("glGetUniformLocationEXT");
              if (!p_glGetUniformLocation)
                  result = 0;
         }
    }


    p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fv");
    if (!p_glUniform2fv)
    {
         p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fvARB");
         if (!p_glUniform2fv)
         {
              p_glUniform2fv = (PFNGLUNIFORM2FVPROC) SF_GL_RUNTIME_LINK("glUniform2fvEXT");
              if (!p_glUniform2fv)
                  result = 0;
         }
    }


    p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fv");
    if (!p_glUniform3fv)
    {
         p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fvARB");
         if (!p_glUniform3fv)
         {
              p_glUniform3fv = (PFNGLUNIFORM3FVPROC) SF_GL_RUNTIME_LINK("glUniform3fvEXT");
              if (!p_glUniform3fv)
                  result = 0;
         }
    }


    return result;
}

}}}

#endif
