/**********************************************************************

Filename    :   GL_Common.h
Content     :   
Created     :   
Authors     :   

Copyright   :   (c) 2001-2009 Scaleform Corp. All Rights Reserved.

Licensees may use this file in accordance with the valid Scaleform
Commercial License Agreement provided with the software.

This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING 
THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR ANY PURPOSE.

***********************************************************************/

#ifndef INC_SF_Render_GL_Common_H
#define INC_SF_Render_GL_Common_H

#include "Kernel/SF_Types.h"


#if defined(SF_OS_ANDROID) || defined(SF_USE_GLES2)
#include <GLES2/gl2.h>
#if defined(SF_USE_EGL)
  #include <EGL/egl.h>
  #define SF_GL_RUNTIME_LINK(x) eglGetProcAddress(x)
#else
  #define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>

#elif defined(SF_OS_WIN32)
#include <windows.h>
#include <gl/gl.h>
#include "glext.h"
#define SF_GL_RUNTIME_LINK(x) wglGetProcAddress(x)

#elif defined(SF_OS_MAC)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>

#elif defined(SF_OS_IPHONE)
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#else

// Use GLX to link gl extensions at runtime; comment out if not using GLX
#define SF_GLX_RUNTIME_LINK

#ifdef SF_GLX_RUNTIME_LINK
#include <GL/glx.h>
#include <GL/glxext.h>
#define SF_GL_RUNTIME_LINK(x) glXGetProcAddressARB((const GLubyte*) (x))
#else
#define GL_GLEXT_PROTOTYPES
#endif

#include <GL/gl.h>
#include <GL/glext.h>
#endif

#ifdef SF_GL_RUNTIME_LINK
  #if defined(GL_ES_VERSION_2_0)
    #include "Render/GL/GLES_Extensions.h"
  #else
    #include "Render/GL/GL_Extensions.h"
  #endif
  #define GLEXT GetHAL()->
#else
  #define GLEXT
#endif

#if defined(GL_ES_VERSION_2_0)

// Unsupported features
#define GL_MIN GL_FUNC_ADD
#define GL_MAX GL_FUNC_ADD

// standard names
#define GL_FRAMEBUFFER_EXT GL_FRAMEBUFFER
#define GL_FRAMEBUFFER_COMPLETE_EXT GL_FRAMEBUFFER_COMPLETE
#define GL_RENDERBUFFER_EXT GL_RENDERBUFFER
#define GL_COLOR_ATTACHMENT0_EXT GL_COLOR_ATTACHMENT0
#define GL_DEPTH_ATTACHMENT_EXT GL_DEPTH_ATTACHMENT
#define GL_STENCIL_ATTACHMENT_EXT GL_STENCIL_ATTACHMENT

#ifndef GL_BGRA_EXT
#define GL_BGRA_EXT GL_BGRA
#endif
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES
#define glMapBuffer glMapBufferOES
#define glUnmapBuffer glUnmapBufferOES
#define glGenerateMipmapEXT glGenerateMipmap

typedef char GLchar;

#endif

#endif
