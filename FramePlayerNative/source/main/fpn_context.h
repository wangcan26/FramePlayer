#ifndef FPN_CONTEXT_H
#define FPN_CONTEXT_H

#if TARGET_OS_ANDROID
#include <android/native_window.h>
#include <EGL/egl.h>
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
        void *data = nullptr;
    };

    struct FPNContext {
#if TARGET_OS_ANDROID
        ANativeWindow         *window    = nullptr;

        EGLDisplay  display;
        EGLSurface  surface;
        EGLContext  context;
        EGLConfig   config;
#else 
        void                 *window = nullptr;
#endif
    };
}

#endif 