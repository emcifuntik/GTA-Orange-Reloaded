
namespace Scaleform { namespace Render { namespace GL {

class Extensions
{
    PFNGLUNMAPBUFFEROESPROC p_glUnmapBufferOES;
    PFNGLMAPBUFFEROESPROC   p_glMapBufferOES;

public:
    bool Init();

    GLboolean glUnmapBufferOES(GLenum a0)
    {
        return p_glUnmapBufferOES(a0);
    }

    void* glMapBufferOES(GLenum a0, GLenum a1)
    {
        return p_glMapBufferOES(a0, a1);
    }

};

}}}
