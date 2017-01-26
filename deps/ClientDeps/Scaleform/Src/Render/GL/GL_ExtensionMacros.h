
#include "Render/GL/GL_HAL.h"
#ifdef SF_GL_RUNTIME_LINK

#define glGenerateMipmapEXT GetHAL()->glGenerateMipmapEXT
#define glFramebufferTexture2DEXT GetHAL()->glFramebufferTexture2DEXT
#define glMapBuffer GetHAL()->glMapBuffer
#define glGenBuffers GetHAL()->glGenBuffers
#define glCreateShader GetHAL()->glCreateShader
#define glBufferData GetHAL()->glBufferData
#define glCompressedTexImage2D GetHAL()->glCompressedTexImage2D
#define glLinkProgram GetHAL()->glLinkProgram
#define glActiveTexture GetHAL()->glActiveTexture
#define glGetProgramiv GetHAL()->glGetProgramiv
#define glCheckFramebufferStatusEXT GetHAL()->glCheckFramebufferStatusEXT
#define glUniform1fv GetHAL()->glUniform1fv
#define glBlendEquation GetHAL()->glBlendEquation
#define glDisableVertexAttribArray GetHAL()->glDisableVertexAttribArray
#define glBindAttribLocation GetHAL()->glBindAttribLocation
#define glCompileShader GetHAL()->glCompileShader
#define glAttachShader GetHAL()->glAttachShader
#define glGetAttribLocation GetHAL()->glGetAttribLocation
#define glBlendFuncSeparate GetHAL()->glBlendFuncSeparate
#define glGenFramebuffersEXT GetHAL()->glGenFramebuffersEXT
#define glUniform1i GetHAL()->glUniform1i
#define glDeleteRenderbuffersEXT GetHAL()->glDeleteRenderbuffersEXT
#define glGetShaderiv GetHAL()->glGetShaderiv
#define glDeleteProgram GetHAL()->glDeleteProgram
#define glDeleteFramebuffersEXT GetHAL()->glDeleteFramebuffersEXT
#define glBindFramebufferEXT GetHAL()->glBindFramebufferEXT
#define glCreateProgram GetHAL()->glCreateProgram
#define glUseProgram GetHAL()->glUseProgram
#define glBindRenderbufferEXT GetHAL()->glBindRenderbufferEXT
#define glDeleteShader GetHAL()->glDeleteShader
#define glGetProgramInfoLog GetHAL()->glGetProgramInfoLog
#define glDeleteBuffers GetHAL()->glDeleteBuffers
#define glGenRenderbuffersEXT GetHAL()->glGenRenderbuffersEXT
#define glRenderbufferStorageEXT GetHAL()->glRenderbufferStorageEXT
#define glShaderSource GetHAL()->glShaderSource
#define glBindBuffer GetHAL()->glBindBuffer
#define glGetShaderInfoLog GetHAL()->glGetShaderInfoLog
#define glUniform4fv GetHAL()->glUniform4fv
#define glEnableVertexAttribArray GetHAL()->glEnableVertexAttribArray
#define glUniformMatrix4fv GetHAL()->glUniformMatrix4fv
#define glUnmapBuffer GetHAL()->glUnmapBuffer
#define glFramebufferRenderbufferEXT GetHAL()->glFramebufferRenderbufferEXT
#define glVertexAttribPointer GetHAL()->glVertexAttribPointer
#define glGetUniformLocation GetHAL()->glGetUniformLocation
#define glUniform2fv GetHAL()->glUniform2fv
#define glUniform3fv GetHAL()->glUniform3fv

#endif
