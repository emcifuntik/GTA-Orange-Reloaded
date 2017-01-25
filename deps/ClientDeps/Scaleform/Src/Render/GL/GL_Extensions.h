
namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
    PFNGLGENERATEMIPMAPEXTPROC          p_glGenerateMipmapEXT;
    PFNGLFRAMEBUFFERTEXTURE2DEXTPROC    p_glFramebufferTexture2DEXT;
    PFNGLMAPBUFFERPROC                  p_glMapBuffer;
    PFNGLGENBUFFERSPROC                 p_glGenBuffers;
    PFNGLCREATESHADERPROC               p_glCreateShader;
    PFNGLBUFFERDATAPROC                 p_glBufferData;
    PFNGLCOMPRESSEDTEXIMAGE2DPROC       p_glCompressedTexImage2D;
    PFNGLLINKPROGRAMPROC                p_glLinkProgram;
    PFNGLACTIVETEXTUREPROC              p_glActiveTexture;
    PFNGLGETPROGRAMIVPROC               p_glGetProgramiv;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC  p_glCheckFramebufferStatusEXT;
    PFNGLUNIFORM1FVPROC                 p_glUniform1fv;
    PFNGLBLENDEQUATIONPROC              p_glBlendEquation;
    PFNGLDISABLEVERTEXATTRIBARRAYPROC   p_glDisableVertexAttribArray;
    PFNGLBINDATTRIBLOCATIONPROC         p_glBindAttribLocation;
    PFNGLCOMPILESHADERPROC              p_glCompileShader;
    PFNGLATTACHSHADERPROC               p_glAttachShader;
    PFNGLGETATTRIBLOCATIONPROC          p_glGetAttribLocation;
    PFNGLBLENDFUNCSEPARATEPROC          p_glBlendFuncSeparate;
    PFNGLGENFRAMEBUFFERSEXTPROC         p_glGenFramebuffersEXT;
    PFNGLUNIFORM1IPROC                  p_glUniform1i;
    PFNGLDELETERENDERBUFFERSEXTPROC     p_glDeleteRenderbuffersEXT;
    PFNGLGETSHADERIVPROC                p_glGetShaderiv;
    PFNGLDELETEPROGRAMPROC              p_glDeleteProgram;
    PFNGLDELETEFRAMEBUFFERSEXTPROC      p_glDeleteFramebuffersEXT;
    PFNGLBINDFRAMEBUFFEREXTPROC         p_glBindFramebufferEXT;
    PFNGLCREATEPROGRAMPROC              p_glCreateProgram;
    PFNGLUSEPROGRAMPROC                 p_glUseProgram;
    PFNGLBINDRENDERBUFFEREXTPROC        p_glBindRenderbufferEXT;
    PFNGLDELETESHADERPROC               p_glDeleteShader;
    PFNGLGETPROGRAMINFOLOGPROC          p_glGetProgramInfoLog;
    PFNGLDELETEBUFFERSPROC              p_glDeleteBuffers;
    PFNGLGENRENDERBUFFERSEXTPROC        p_glGenRenderbuffersEXT;
    PFNGLRENDERBUFFERSTORAGEEXTPROC     p_glRenderbufferStorageEXT;
    PFNGLSHADERSOURCEPROC               p_glShaderSource;
    PFNGLBINDBUFFERPROC                 p_glBindBuffer;
    PFNGLGETSHADERINFOLOGPROC           p_glGetShaderInfoLog;
    PFNGLUNIFORM4FVPROC                 p_glUniform4fv;
    PFNGLENABLEVERTEXATTRIBARRAYPROC    p_glEnableVertexAttribArray;
    PFNGLUNIFORMMATRIX4FVPROC           p_glUniformMatrix4fv;
    PFNGLUNMAPBUFFERPROC                p_glUnmapBuffer;
    PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC p_glFramebufferRenderbufferEXT;
    PFNGLVERTEXATTRIBPOINTERPROC        p_glVertexAttribPointer;
    PFNGLGETUNIFORMLOCATIONPROC         p_glGetUniformLocation;
    PFNGLUNIFORM2FVPROC                 p_glUniform2fv;
    PFNGLUNIFORM3FVPROC                 p_glUniform3fv;

public:
    bool Init();

    void glGenerateMipmapEXT(GLenum a0)
    {
        SF_ASSERT(!glGetError());
p_glGenerateMipmapEXT(a0);
  SF_ASSERT(!glGetError());
  }

    void glFramebufferTexture2DEXT(GLenum a0, GLenum a1, GLenum a2, GLuint a3, GLint a4)
    {
        SF_ASSERT(!glGetError());
p_glFramebufferTexture2DEXT(a0, a1, a2, a3, a4);
  SF_ASSERT(!glGetError());
  }

    GLvoid* glMapBuffer(GLenum a0, GLenum a1)
    {
        SF_ASSERT(!glGetError());
return p_glMapBuffer(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glGenBuffers(GLsizei a0, GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glGenBuffers(a0, a1);
  SF_ASSERT(!glGetError());
  }

    GLuint glCreateShader(GLenum a0)
    {
        SF_ASSERT(!glGetError());
return p_glCreateShader(a0);
  SF_ASSERT(!glGetError());
  }

    void glBufferData(GLenum a0, GLsizeiptr a1, const GLvoid * a2, GLenum a3)
    {
        SF_ASSERT(!glGetError());
p_glBufferData(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glCompressedTexImage2D(GLenum a0, GLint a1, GLenum a2, GLsizei a3, GLsizei a4, GLint a5, GLsizei a6, const GLvoid * a7)
    {
        SF_ASSERT(!glGetError());
p_glCompressedTexImage2D(a0, a1, a2, a3, a4, a5, a6, a7);
  SF_ASSERT(!glGetError());
  }

    void glLinkProgram(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glLinkProgram(a0);
  SF_ASSERT(!glGetError());
  }

    void glActiveTexture(GLenum a0)
    {
        SF_ASSERT(!glGetError());
p_glActiveTexture(a0);
  SF_ASSERT(!glGetError());
  }

    void glGetProgramiv(GLuint a0, GLenum a1, GLint * a2)
    {
        SF_ASSERT(!glGetError());
p_glGetProgramiv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    GLenum glCheckFramebufferStatusEXT(GLenum a0)
    {
        SF_ASSERT(!glGetError());
return p_glCheckFramebufferStatusEXT(a0);
  SF_ASSERT(!glGetError());
  }

    void glUniform1fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        SF_ASSERT(!glGetError());
p_glUniform1fv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    void glBlendEquation(GLenum a0)
    {
        SF_ASSERT(!glGetError());
p_glBlendEquation(a0);
  SF_ASSERT(!glGetError());
  }

    void glDisableVertexAttribArray(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glDisableVertexAttribArray(a0);
  SF_ASSERT(!glGetError());
  }

    void glBindAttribLocation(GLuint a0, GLuint a1, const GLchar * a2)
    {
        SF_ASSERT(!glGetError());
p_glBindAttribLocation(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    void glCompileShader(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glCompileShader(a0);
  SF_ASSERT(!glGetError());
  }

    void glAttachShader(GLuint a0, GLuint a1)
    {
        SF_ASSERT(!glGetError());
p_glAttachShader(a0, a1);
  SF_ASSERT(!glGetError());
  }

    GLint glGetAttribLocation(GLuint a0, const GLchar * a1)
    {
        SF_ASSERT(!glGetError());
return p_glGetAttribLocation(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glBlendFuncSeparate(GLenum a0, GLenum a1, GLenum a2, GLenum a3)
    {
        SF_ASSERT(!glGetError());
p_glBlendFuncSeparate(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glGenFramebuffersEXT(GLsizei a0, GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glGenFramebuffersEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glUniform1i(GLint a0, GLint a1)
    {
        SF_ASSERT(!glGetError());
p_glUniform1i(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glDeleteRenderbuffersEXT(GLsizei a0, const GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glDeleteRenderbuffersEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glGetShaderiv(GLuint a0, GLenum a1, GLint * a2)
    {
        SF_ASSERT(!glGetError());
p_glGetShaderiv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    void glDeleteProgram(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glDeleteProgram(a0);
  SF_ASSERT(!glGetError());
  }

    void glDeleteFramebuffersEXT(GLsizei a0, const GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glDeleteFramebuffersEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glBindFramebufferEXT(GLenum a0, GLuint a1)
    {
        SF_ASSERT(!glGetError());
p_glBindFramebufferEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    GLuint glCreateProgram()
    {
        SF_ASSERT(!glGetError());
return p_glCreateProgram();
  SF_ASSERT(!glGetError());
  }

    void glUseProgram(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glUseProgram(a0);
  SF_ASSERT(!glGetError());
  }

    void glBindRenderbufferEXT(GLenum a0, GLuint a1)
    {
        SF_ASSERT(!glGetError());
p_glBindRenderbufferEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glDeleteShader(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glDeleteShader(a0);
  SF_ASSERT(!glGetError());
  }

    void glGetProgramInfoLog(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        SF_ASSERT(!glGetError());
p_glGetProgramInfoLog(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glDeleteBuffers(GLsizei a0, const GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glDeleteBuffers(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glGenRenderbuffersEXT(GLsizei a0, GLuint * a1)
    {
        SF_ASSERT(!glGetError());
p_glGenRenderbuffersEXT(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glRenderbufferStorageEXT(GLenum a0, GLenum a1, GLsizei a2, GLsizei a3)
    {
        SF_ASSERT(!glGetError());
p_glRenderbufferStorageEXT(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glShaderSource(GLuint a0, GLsizei a1, const GLchar* * a2, const GLint * a3)
    {
        SF_ASSERT(!glGetError());
p_glShaderSource(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glBindBuffer(GLenum a0, GLuint a1)
    {
        SF_ASSERT(!glGetError());
p_glBindBuffer(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glGetShaderInfoLog(GLuint a0, GLsizei a1, GLsizei * a2, GLchar * a3)
    {
        SF_ASSERT(!glGetError());
p_glGetShaderInfoLog(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glUniform4fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        SF_ASSERT(!glGetError());
p_glUniform4fv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    void glEnableVertexAttribArray(GLuint a0)
    {
        SF_ASSERT(!glGetError());
p_glEnableVertexAttribArray(a0);
  SF_ASSERT(!glGetError());
  }

    void glUniformMatrix4fv(GLint a0, GLsizei a1, GLboolean a2, const GLfloat * a3)
    {
        SF_ASSERT(!glGetError());
p_glUniformMatrix4fv(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    GLboolean glUnmapBuffer(GLenum a0)
    {
        SF_ASSERT(!glGetError());
return p_glUnmapBuffer(a0);
  SF_ASSERT(!glGetError());
  }

    void glFramebufferRenderbufferEXT(GLenum a0, GLenum a1, GLenum a2, GLuint a3)
    {
        SF_ASSERT(!glGetError());
p_glFramebufferRenderbufferEXT(a0, a1, a2, a3);
  SF_ASSERT(!glGetError());
  }

    void glVertexAttribPointer(GLuint a0, GLint a1, GLenum a2, GLboolean a3, GLsizei a4, const GLvoid * a5)
    {
        SF_ASSERT(!glGetError());
p_glVertexAttribPointer(a0, a1, a2, a3, a4, a5);
  SF_ASSERT(!glGetError());
  }

    GLint glGetUniformLocation(GLuint a0, const GLchar * a1)
    {
        SF_ASSERT(!glGetError());
return p_glGetUniformLocation(a0, a1);
  SF_ASSERT(!glGetError());
  }

    void glUniform2fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        SF_ASSERT(!glGetError());
p_glUniform2fv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

    void glUniform3fv(GLint a0, GLsizei a1, const GLfloat * a2)
    {
        SF_ASSERT(!glGetError());
p_glUniform3fv(a0, a1, a2);
  SF_ASSERT(!glGetError());
  }

};

}}}
