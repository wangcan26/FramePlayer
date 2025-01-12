#ifndef FPN_TEST_NODE_H
#define FPN_TEST_NODE_H
#include <string>
#include <vector>
#include <thread>
#ifdef TARGET_OS_ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#include "main/fpn_context.h"

#define FPN_IMAGE_BUFFER_SIZE_MAX 2 

namespace fpn 
{
    struct VertexBuffer {
#ifdef TARGET_OS_ANDROID
        GLuint vertex;
        GLuint uv;
        GLuint indice;
#endif   
    };

    struct TextureBuffer {
#ifdef TARGET_OS_ANDROID
        GLuint texture;
#endif 
    };

    struct RenderPipeline
    {
#ifdef TARGET_OS_ANDROID
        GLuint program;
        GLuint position;
        GLuint uv;
        GLuint color;
        GLuint texture;
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
        
        /*
        * must be called in producer thread
        * obtain a memory opaque of FPNImageData for producer to write data into it.
        */
        void opaque(struct FPNImageData* data);
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
        struct TextureBuffer mTexture;
        struct RenderPipeline mDefaultPipeline;
        struct RenderPipeline mTexturePipeline;
        
        std::mutex mCanvasMutex;
        //image data queue
        FPNImageData mImageBuffers[FPN_IMAGE_BUFFER_SIZE_MAX];
        bool  mBufferWind = true;
        int mReadIndex =  -1;
        int mWriteIndex = -1;
        bool mCreateTexture = true;
    };
}


#endif //FPN_TEST_NODE_H