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

static auto kGreenMattingShader = 
        "precision highp float;\n"
        "varying vec2 oUv;\n"
        "uniform sampler2D uTexture;\n"
        "uniform vec3 uKeyColor;\n"
        "uniform float uSmoothness;\n"
        "uniform float uSimilarity;\n"
        "uniform float uSpill;\n"
        "vec2 RGB2UV(vec3 rgb) {\n"
        "   return vec2(\n"
        "       rgb.r * -0.169 + rgb.g * -0.331 + rgb.b * 0.5 + 0.5,\n"
        "       rgb.r * 0.5 + rgb.g * -0.419 + rgb.b * -0.081 + 0.5\n"
        "   );\n"
        "}\n"
        "void main() {\n"
        "   vec4 color = texture2D(uTexture, oUv).bgra;\n"
        "   vec2 chromaVec = RGB2UV(color.rgb) - RGB2UV(uKeyColor);\n"
        "   float chromaDist = sqrt(dot(chromaVec, chromaVec));\n"
        "   float baseMask = chromaDist - uSimilarity;\n"
        "   float fullMask = pow(clamp(baseMask/uSmoothness, 0., 1.), 1.5);\n"
        "   color.a = fullMask;\n"
        "   float spillVal = pow(clamp(baseMask/uSpill, 0., 1.), 1.5);\n"
        "   float desat = clamp(color.r * 0.2126 + color.g * 0.7152 + color.b * 0.0722, 0., 1.);\n"
        "   color.bgr = mix(vec3(desat), color.rgb, spillVal);\n"
        "   gl_FragColor = color;\n"
        "}\n";

static auto kGreenMattingShader1 = 
        "precision mediump float;\n"
        "varying vec2 oUv;\n"
        "uniform sampler2D uTexture;\n"
        "uniform vec3 uKeyColor;\n"
        "uniform float uSmoothness;\n"
        "uniform float uSimilarity;\n"
        "uniform float uSpill;\n"
        "void main() {\n"
        "   vec4 color = texture2D(uTexture, oUv);\n"
        "   float mask = smoothstep(uSimilarity - 0.1, uSimilarity + 0.1, distance(color.rgb, uKeyColor));\n"
        "   gl_FragColor = vec4(vec3(mask), 1.0);\n"
        "}\n";

static const GLfloat kVertices[] = {
                                -1.0f, -1.0f, 0.0f,
                                -1.0f, 1.0f,  0.0f,
                                1.0f, 1.0f,  0.0f,
                                1.0f, -1.0f, 0.0f};

static const GLfloat kUvs[] = {0.0, 1.0, 0.0, 0.0, 1.0, 0.0, 1.0, 1.0};

static const GLushort kIndices[] = {0, 3, 1, 1, 3, 2};

static const float kTransparentColor[] = {0, 0, 0, 0};
static const float kDefaultColor[] = {0, 0, 0, 1};
static const float kSelectColor[] = {0, 1, 0};

namespace fpn 
{
    const struct FPNCanvas::Options FPNCanvas::NullOpt;
    FPNCanvas::FPNCanvas(int width, int height, struct Options options)
        :
        mWidth(width), 
        mHeight(height),
        mOptions(options) 
    {
        _initialize();
    }
    FPNCanvas::~FPNCanvas() {
        std::lock_guard<std::mutex> rm(mCanvasMutex);
        if (mWriteIndex != -1) {
            for (int i = 0; i < FPN_IMAGE_BUFFER_SIZE_MAX; i++) {
                FPNImageData& buffer = mImageBuffers[i];
                free(buffer.data);
            }
            mWriteIndex = mReadIndex = -1;
        }
    }

    void FPNCanvas::release() {
#ifdef FPN_USE_OPENGL_API  
        if (mDefaultPipeline.program != FPN_INVALID_GPU_VALUE) {
            glDeleteProgram(mDefaultPipeline.program);
            mDefaultPipeline.program = FPN_INVALID_GPU_VALUE;
        }

        if (mTexturePipeline.program != FPN_INVALID_GPU_VALUE) {
            glDeleteProgram(mTexturePipeline.program);
            mTexturePipeline.program = FPN_INVALID_GPU_VALUE;
        }

        if (mGeometry.vertex != FPN_INVALID_GPU_VALUE) {
            glDeleteBuffers(1, &mGeometry.vertex);
            mGeometry.vertex = FPN_INVALID_GPU_VALUE;
        }
        if (mGeometry.indice != FPN_INVALID_GPU_VALUE) {
            glDeleteBuffers(1, &mGeometry.indice);
            mGeometry.indice = FPN_INVALID_GPU_VALUE;
        }
#endif 
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
#ifdef FPN_USE_OPENGL_API
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
            unsigned int format = GL_RGBA;
            if (buffer.format == FPNImageFormat::RGB8Unorm) {
                format = GL_RGB;
            }
            if (mCreateTexture) {
                if (mTexture.texture != FPN_INVALID_GPU_VALUE) {
                    glDeleteTextures(1, &mTexture.texture);
                }
                glGenTextures(1, &mTexture.texture);
                glBindTexture(GL_TEXTURE_2D, mTexture.texture);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexImage2D(GL_TEXTURE_2D, 0, format, buffer.width, buffer.height, 0, format, GL_UNSIGNED_BYTE, buffer.data);
                glGenerateMipmap(GL_TEXTURE_2D);
                mCreateTexture = false;
            } else {
                glBindTexture(GL_TEXTURE_2D, mTexture.texture);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buffer.width, buffer.height, format, GL_UNSIGNED_BYTE, buffer.data);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        } while (0);
        RenderPipeline pipeline = chooseDefault? mDefaultPipeline: mTexturePipeline;
        glUseProgram(pipeline.program);
        if (chooseDefault) {
            glUniform4fv(pipeline.color, 1, mOptions.transparent ? kTransparentColor : kDefaultColor);
        } else {
            glBindTexture(GL_TEXTURE_2D, mTexture.texture);
            glUniform1i(pipeline.texture, 0);

            if (mOptions.green_matting) {
                glUniform3fv(pipeline.keyColor, 1, kSelectColor);
                glUniform1f(pipeline.similarity, mOptions.green_matting_similarity);
                glUniform1f(pipeline.smoothness, mOptions.green_matting_smoothness);
                glUniform1f(pipeline.spill, mOptions.green_matting_spill);
            }
        }

        glUniformMatrix4fv(pipeline.transform, 1, GL_FALSE, mProjection);

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
        _orthoProjection();
        
#ifdef FPN_USE_OPENGL_API
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
        mDefaultPipeline.transform = glGetUniformLocation(mDefaultPipeline.program, "uTransform");

        if (mOptions.green_matting) {
            mTexturePipeline.program = _createProgram(kVertexShader, kGreenMattingShader);
        } else {
            mTexturePipeline.program = _createProgram(kVertexShader, kTextureShader);
        }
        
        if(mTexturePipeline.program <= 0){
            FPN_LOGE(LOG_TAG, "FPNCanvas Could not create texture program.");
            return;
        }
        mTexturePipeline.position = glGetAttribLocation(mTexturePipeline.program, "vPosition");
        mTexturePipeline.uv = glGetAttribLocation(mTexturePipeline.program, "vUv");
        mTexturePipeline.texture = glGetUniformLocation(mTexturePipeline.program, "uTexture");
        mTexturePipeline.transform = glGetUniformLocation(mDefaultPipeline.program, "uTransform");
        if (mOptions.green_matting) {
            mTexturePipeline.keyColor = glGetUniformLocation(mTexturePipeline.program, "uKeyColor");
            mTexturePipeline.similarity = glGetUniformLocation(mTexturePipeline.program, "uSimilarity");
            mTexturePipeline.smoothness = glGetUniformLocation(mTexturePipeline.program, "uSmoothness");
            mTexturePipeline.spill = glGetUniformLocation(mTexturePipeline.program, "uSpill");
        }
#endif 
        mIsInited = true;
    }

    void FPNCanvas::_orthoProjection() {
        bool isPortrait = mWidth < mHeight;
        float actualAspect = isPortrait ? (float) mWidth / (float) mHeight : (float) mHeight / (float) mWidth;

        float aspect = mHeight <= 0 ? 9.0/16.0: actualAspect;
        float left = isPortrait? -1.0 * aspect : -1.0;
        float right = isPortrait? 1.0 * aspect : 1.0;
        float bottom = -1.0;
        float top = 1.0;
        float zNear = -1.0;
        float zFar = 1.0;
        float proj[16] = {
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            0.0, 0.0, 0.0, 1.0
        };
        float deltax = right - left;
        float deltay = top - bottom;
        float deltaz = zFar - zNear;
        proj[0] = 2.0f / deltax;
        proj[3] = -(right + left) / deltax;
        proj[5] = 2.0f / deltay;
        proj[7] = -(top + bottom) /deltay;
        proj[10] = 2.0f / deltaz;
        proj[11] = -(zFar + zNear) /deltaz;
        memcpy(mProjection, proj, sizeof(float) * 16);
        FPN_LOGE(LOG_TAG, "aspect: %f, projection:\n[%f, %f, %f, %f, \n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f]",
            aspect, 
            mProjection[0], mProjection[1], mProjection[2], mProjection[3],
            mProjection[4], mProjection[5], mProjection[6], mProjection[7],
            mProjection[8], mProjection[9], mProjection[10], mProjection[11],
            mProjection[12], mProjection[13], mProjection[14], mProjection[15]);
    }


#ifdef FPN_USE_OPENGL_API
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