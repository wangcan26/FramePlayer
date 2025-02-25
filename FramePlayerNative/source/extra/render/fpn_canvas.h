#ifndef FPN_TEST_NODE_H
#define FPN_TEST_NODE_H
#include <string>
#include <vector>
#include <thread>
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#endif 
#include "main/fpn_context.h"

#define FPN_IMAGE_BUFFER_SIZE_MAX 2 

namespace fpn 
{
    struct VertexBuffer {
#ifdef FPN_USE_OPENGL_API
        GLuint vertex;
        GLuint uv;
        GLuint indice;
#endif  
    };

    struct TextureBuffer {
#ifdef FPN_USE_OPENGL_API
        GLuint texture;
#endif  
    };

    struct RenderPipeline
    {
#ifdef FPN_USE_OPENGL_API
        GLuint program;
        GLuint position;
        GLuint uv;
        GLuint color;
        GLuint texture;
        GLuint transform;
        //green matting
        GLuint keyColor;
        GLuint similarity;
        GLuint smoothness;
        GLuint spill;
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
        struct Options {
            bool green_matting = true;
            //value bigger, the foreground is bigger
            float green_matting_similarity = 0.45;
            //value bigger, the alpha is more
            float green_matting_smoothness = 0.08;
            float green_matting_spill = 0.08;
            //make surface transparent defaulty
            bool transparent = true;
        };
        static const struct Options NullOpt;
    public:
        FPNCanvas(int width, int height, struct Options options = NullOpt);
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
        void _orthoProjection();

#ifdef FPN_USE_OPENGL_API
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

        float mProjection[16];
        
        std::mutex mCanvasMutex;
        //image data queue
        FPNImageData mImageBuffers[FPN_IMAGE_BUFFER_SIZE_MAX];
        bool  mBufferWind = true;
        int mReadIndex =  -1;
        int mWriteIndex = -1;
        bool mCreateTexture = true;
        int mWidth;
        int mHeight;

        struct Options mOptions;
    };
}


#endif //FPN_TEST_NODE_H