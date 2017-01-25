
#include "Render/GL/GL_Common.h"

namespace Scaleform { namespace Render { namespace GL {

bool Extensions::Init()
{
    bool result = 1;
    p_glUnmapBufferOES = (PFNGLUNMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glUnmapBufferOES");

    p_glMapBufferOES = (PFNGLMAPBUFFEROESPROC) SF_GL_RUNTIME_LINK("glMapBufferOES");

    return result;
}

}}}
