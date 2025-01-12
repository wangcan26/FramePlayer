#ifndef FPN_TEST_NODE_H
#define FPN_TEST_NODE_H
#include <string>
#ifdef TARGET_OS_ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

namespace fpn 
{
    enum FPNImageFormat {
        Invalid = 0,
        RGB8Unorm,
        RGBA8Unorm
    };
    struct FPNImageData 
    {
        int width;
        int height;
        enum FPNImageFormat format;
        void *data;
    };

    struct VertexBuffer {
#ifdef TARGET_OS_ANDROID
        GLuint vertex;
        GLuint uv;
        GLuint indice;
#endif   
    };

    struct RenderPipeline
    {
#ifdef TARGET_OS_ANDROID
        GLuint program;
        GLuint position;
        GLuint uv;
        GLuint color;
#endif 
    };

    /**
     * FPNCanvas muse be created in render thread which has a gl context.
     * it consumes and shows the raw image data.
     * If no image data, FPNCanvas will show a default color. 
     */
    class FPNCanvas
    {
    public:
        FPNCanvas();
        virtual ~FPNCanvas();

        //must be called in render thread
        void paint();
        //receive image data from producer thread to a locked buffer queue
        void lock(struct FPNImageData& data);
        //unlock the buffer queue and notify producer thread continue to send image data.
        void unlock();
    private:
        void _initialize();
#ifdef TARGET_OS_ANDROID
        GLuint _createProgram(const char* vertex_source, const char* fragment_source);
        GLuint _loaderShader(GLenum shader_type, const char *source);
        void _checkGLError(const char *op);
#endif 
    private:
        bool mIsInited = false;
        std::string mName = "gl_canvas";
        struct VertexBuffer mGeometry;
        struct RenderPipeline mPipeline;

        //image data queue
    };
}


#endif //FPN_TEST_NODE_H