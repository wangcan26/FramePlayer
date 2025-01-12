#include "fpn_canvas.h"
#include "log/fpn_log.h"

#define LOG_TAG "FPNCanvas"

#define FPN_TEST_TRIANGLE 1

static auto kVertexShader =
        "attribute vec4 vPosition;\n"
        "attribute vec2 vUv;\n"
        "varying vec2 oUv;\n"
        "uniform mat4 uTransform;\n"
        "void main() {\n"
        "   oUv = vUv;\n"
        "   gl_Position =  vec4(vPosition.xyz, 1.0);\n"
        "}\n";

static auto kColorShader =
        "precision mediump float;\n"
        "varying vec2 oUv;\n"
        "uniform vec4 uColor;\n"
        "void main() {\n"
        "  gl_FragColor = uColor;\n"
        "}\n";

static auto kTextureShader =
        "precision mediump float;\n"
        "varying vec2 oUv;\n"
        "uniform sampler2D uTexture;\n"
        "void main() {\n"
        "   vec4 color = texture2D(uTexture, oUv);\n"
        "   gl_FragColor = color;\n"
        "}\n";

static const GLfloat kVertices[] = {
                                -1.0f, -1.0f, 0.0f,
                                -1.0f, 1.0f,  0.0f,
                                1.0f, 1.0f,  0.0f,
                                1.0f, -1.0f, 0.0f};

static const GLfloat kUvs[] = {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0};

static const GLushort kIndices[] = {0, 3, 1, 1, 3, 2};

static const float kDefaultColor[] = {0, 1, 0, 1};

namespace fpn 
{
    FPNCanvas::FPNCanvas() {
        _initialize();
    }
    FPNCanvas::~FPNCanvas() {
        std::lock_guard<std::mutex> rm(mCanvasMutex);
        if (!mBufferWind) {
            for (int i = 0; i < FPN_IMAGE_BUFFER_SIZE_MAX; i++) {
                FPNImageData& buffer = mImageBuffers[mWriteIndex];
                free(buffer.data);
            }
        }
    }

    void FPNCanvas::opaque(struct FPNImageData* data) {
        std::lock_guard<std::mutex> rm(mCanvasMutex);
        if (data->format == FPNImageFormat::Invalid) {
            return;
        }
        if(!mBufferWind) {
            int width = mImageBuffers[0].width;
            int height = mImageBuffers[0].height;
            int format = mImageBuffers[0].format;
            if (width != data->width || height != data->height || format != data->format) {
                mBufferWind = true;
            }
        }
        int byteSize = 1;
        if (data->format == FPNImageFormat::RGBA8Unorm) {
            byteSize = 4;
        } else if (data->format == FPNImageFormat::RGB8Unorm) {
            byteSize = 3;
        }
        if (mBufferWind) {
            for (int i = 0; i < FPN_IMAGE_BUFFER_SIZE_MAX; i++) {
                FPNImageData buffer;
                buffer.width = data->width;
                buffer.height = data->height;
                buffer.format = data->format;
                size_t size = buffer.width * buffer.height * byteSize;
                buffer.data = malloc(size);
                memcpy(buffer.data, data->data, size);
                mImageBuffers[i] = buffer;
            }
            mBufferWind = false;
            mReadIndex = 0;
            mWriteIndex = 1;
            mCreateTexture = true;
        } else {
            FPNImageData& buffer = mImageBuffers[mWriteIndex];
            size_t size = buffer.width * buffer.height * byteSize;
            memcpy(buffer.data, data->data, size);
            //swap
            int tmp = mReadIndex;
            mReadIndex = mWriteIndex;
            mWriteIndex = tmp;
        }
        //FPN_LOGI(LOG_TAG, "FPNCanvas swap buffer: [%d, %d]", mReadIndex, mWriteIndex);
    }

    void FPNCanvas::paint() {
        if (!mIsInited) return;
#ifdef TARGET_OS_ANDROID
        bool chooseDefault = mReadIndex == -1;
        //upload
        do {
            if (chooseDefault) 
            {
                break;
            }                
            FPNImageData &buffer = mImageBuffers[mReadIndex];
            if (!buffer.data)
            {
                break;
            }
            if (mCreateTexture) {
                glGenTextures(1, &mTexture.texture);
                glBindTexture(GL_TEXTURE_2D, mTexture.texture);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffer.width, buffer.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data);
                glGenerateMipmap(GL_TEXTURE_2D);
                mCreateTexture = false;
            } else {
                glBindTexture(GL_TEXTURE_2D, mTexture.texture);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        } while (0);
        
       
        RenderPipeline pipeline = chooseDefault? mDefaultPipeline: mTexturePipeline;
        glUseProgram(pipeline.program);
        if (chooseDefault) {
            glUniform4fv(pipeline.color, 1, kDefaultColor);
        } else {
            glBindTexture(GL_TEXTURE_2D, mTexture.texture);
            glUniform1i(pipeline.texture, 0);
        }

        glVertexAttribPointer(pipeline.position, 3, GL_FLOAT, GL_FALSE, 0, kVertices);
        glEnableVertexAttribArray(pipeline.position);
        glVertexAttribPointer(pipeline.uv, 2, GL_FLOAT, GL_FALSE, 0, kUvs);
        glEnableVertexAttribArray(pipeline.uv);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGeometry.indice);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif 
    }

    void FPNCanvas::_initialize() {
#ifdef TARGET_OS_ANDROID
        glGenBuffers(1, &mGeometry.vertex);
        glBindBuffer(GL_ARRAY_BUFFER, mGeometry.vertex);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, kVertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &mGeometry.uv);
        glBindBuffer(GL_ARRAY_BUFFER, mGeometry.uv);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 4, kUvs, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &mGeometry.indice);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGeometry.indice);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, kIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        mDefaultPipeline.program = _createProgram(kVertexShader, kColorShader);
        if(mDefaultPipeline.program <= 0){
            FPN_LOGE(LOG_TAG, "FPNCanvas Could not create default program.");
            return;
        }
        mDefaultPipeline.position = glGetAttribLocation(mDefaultPipeline.program, "vPosition");
        mDefaultPipeline.uv = glGetAttribLocation(mDefaultPipeline.program, "vUv");
        mDefaultPipeline.color = glGetUniformLocation(mDefaultPipeline.program, "uColor");

        mTexturePipeline.program = _createProgram(kVertexShader, kTextureShader);
        if(mTexturePipeline.program <= 0){
            FPN_LOGE(LOG_TAG, "FPNCanvas Could not create texture program.");
            return;
        }
        mTexturePipeline.position = glGetAttribLocation(mTexturePipeline.program, "vPosition");
        mTexturePipeline.uv = glGetAttribLocation(mTexturePipeline.program, "vUv");
        mTexturePipeline.texture = glGetUniformLocation(mTexturePipeline.program, "uTexture");

#endif 
        mIsInited = true;
    }

#ifdef TARGET_OS_ANDROID
    void FPNCanvas::_checkGLError(const char *op) {
        for (GLint error = glGetError(); error; error = glGetError()) {
            FPN_LOGE(LOG_TAG,"nv log renderer after %s() glError (0x%x)\n", op, error);
        }
    }

    GLuint FPNCanvas::_createProgram(const char* vertex_source, const char* fragment_source) {
        GLuint vertex_shader = _loaderShader(GL_VERTEX_SHADER, vertex_source);
        if(!vertex_shader){
            return 0;
        }
        GLuint pixel_shader = _loaderShader(GL_FRAGMENT_SHADER, fragment_source);
        if(!pixel_shader) {
            return 0;
        }

        GLuint program = glCreateProgram();
        if(program)
        {
            glAttachShader(program, vertex_shader);
            _checkGLError("glAttachShader");
            glAttachShader(program, pixel_shader);
            _checkGLError("glAttachShader");
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

            if(linkStatus != GL_TRUE){
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if(bufLength){
                    char* buf = (char*) malloc(bufLength);
                    if(buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        FPN_LOGE(LOG_TAG, "Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }

        return program;
    }

    GLuint FPNCanvas::_loaderShader(GLenum shader_type, const char *source) 
    {
        GLuint shader = glCreateShader(shader_type);
        if(shader)
        {
            glShaderSource(shader, 1, &source, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if(!compiled){
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if(infoLen){
                    char *buf = (char*)malloc(infoLen);
                    if(buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        FPN_LOGE(LOG_TAG, "Could not compile shader %d:\n%s\n", shader_type, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        return shader;
    }
#endif 

}