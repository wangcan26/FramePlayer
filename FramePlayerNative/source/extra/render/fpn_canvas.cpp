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

static const GLfloat kVertices[] = {
                                -1.0f, -1.0f, 0.0f,
                                -1.0f, 1.0f,  0.0f,
                                1.0f, 1.0f,  0.0f,
                                1.0f, -1.0f, 0.0f};

static const GLfloat kUvs[] = {0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0};

static const GLushort kIndices[] = {0, 3, 1, 1, 3, 2};

static const float kDefaultColor[] = {0, 1, 1, 1};

namespace fpn 
{
    FPNCanvas::FPNCanvas() {
        _initialize();
    }
    FPNCanvas::~FPNCanvas() {}

    void FPNCanvas::paint() {
        if (!mIsInited) return;
#ifdef TARGET_OS_ANDROID
        glUseProgram(mPipeline.program);
        glUniform4fv(mPipeline.color, 1, kDefaultColor);

        glVertexAttribPointer(mPipeline.position, 3, GL_FLOAT, GL_FALSE, 0, kVertices);
        glEnableVertexAttribArray(mPipeline.position);

        glVertexAttribPointer(mPipeline.uv, 2, GL_FLOAT, GL_FALSE, 0, kUvs);
        glEnableVertexAttribArray(mPipeline.uv);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGeometry.indice);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
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

        mPipeline.program = _createProgram(kVertexShader, kColorShader);
        if(mPipeline.program <= 0){
            FPN_LOGE(LOG_TAG, "FPNCanvas Could not create program.");
            return;
        }
        mPipeline.position = glGetAttribLocation(mPipeline.program, "vPosition");
        mPipeline.uv = glGetAttribLocation(mPipeline.program, "vUv");
        mPipeline.color = glGetUniformLocation(mPipeline.program, "uColor");
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