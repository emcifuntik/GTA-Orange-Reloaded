#include "Render/GL/GL_HAL.h"
#ifdef SF_GL_RUNTIME_LINK

#define glUnmapBufferOES GetHAL()->glUnmapBufferOES
#define glMapBufferOES GetHAL()->glMapBufferOES

#endif