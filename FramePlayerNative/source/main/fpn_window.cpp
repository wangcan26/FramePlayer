#include "fpn_window.h"
#include "fpn_context.h"
#include "log/fpn_log.h"

#define LOG_TAG "FPNWindow"

namespace fpn 
{
    FPNWindow::FPNWindow() {}
    FPNWindow::~FPNWindow() {
        mContext = nullptr;
    }

    void FPNWindow::attach(FPNContext *context) {
        mContext = context;
    }

    bool FPNWindow::isValid() const {
        bool valid = true;
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        valid = valid && mContext && mContext->surface != EGL_NO_SURFACE;
#endif 
#endif 
        return mWindowInited && valid;
    }

    bool FPNWindow::notify(int what) {
        switch(what) {
            case FLAG_WINDOW_CREATE:
                return _onCreate();
            case FLAG_WINDOW_RESTORE:
            {
                _onRestore();
                break;
            }
            case FLAG_WINDOW_RESIZE:
                return _onResize();;
            case FLAG_WINDOW_RELEASE:
            {
                _onRelease();
                break;
            }
            case FLAG_WINDOW_DESTROY:
            {
                _onDestroy();
                break;
            }
            case FLAG_WINDOW_PRERENT:
            {
                _onPresent();
                break;
            }
        }
        return true;
    }

    bool FPNWindow::_onCreate() {
        if (!mContext || !mContext->window) {
            return false;
        }

#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        const EGLint attribs[] = {
                EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                EGL_BLUE_SIZE, 8,
                EGL_GREEN_SIZE, 8,
                EGL_RED_SIZE, 8,
                EGL_ALPHA_SIZE, 8,
                EGL_NONE
        };
        EGLint contextAtt[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE};
        EGLDisplay  display;
        EGLConfig   config;
        EGLint      numConfigs;
        EGLint      format;
        EGLSurface  surface;
        EGLContext  context;
        if( (display =eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY )
        {
            FPN_LOGE(LOG_TAG,"eglGetDisplay() returned error %d", eglGetError());
            return false;
        }
        if( (display =eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY )
        {
            FPN_LOGE(LOG_TAG,"eglGetDisplay() returned error %d", eglGetError());
            return false;
        }
        if(!eglChooseConfig(display, attribs, &config, 1, &numConfigs))
        {
            FPN_LOGE(LOG_TAG,"eglChooseConfig() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
        if (!eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format)) {
            FPN_LOGE(LOG_TAG,"eglGetConfigAttrib() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }

        ANativeWindow_setBuffersGeometry(mContext->window, 0, 0, format);
        if (!(surface = eglCreateWindowSurface(display, config, mContext->window, 0))) {
            FPN_LOGE(LOG_TAG,"eglCreateWindowSurface() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
        if (!(context = eglCreateContext(display, config, 0, contextAtt))) {
            FPN_LOGE(LOG_TAG,"eglCreateContext() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
        if (!eglMakeCurrent(display, surface, surface, context)) {
            FPN_LOGE(LOG_TAG,"eglMakeCurrent() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }

        if (!eglQuerySurface(display, surface, EGL_WIDTH, &mWidth) ||
            !eglQuerySurface(display, surface, EGL_HEIGHT, &mHeight)) {
            FPN_LOGE(LOG_TAG,"eglQuerySurface() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
        mContext->display = display;
        mContext->surface = surface;
        mContext->context = context;
        mContext->config = config;
#endif 
#endif
        mWindowInited = true;
        FPN_LOGI(LOG_TAG,"Render Window is Created: [%d, %d]", mWidth, mHeight);
        return true;
    }

    bool FPNWindow::_onRestore() {
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        EGLSurface  surface;

        if(!(surface = eglCreateWindowSurface(mContext->display, mContext->config, mContext->window, 0)))
        {
            FPN_LOGE(LOG_TAG,"eglCreateWindowSurface() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
        if(!eglMakeCurrent(mContext->display, surface, surface, mContext->context)){
            FPN_LOGE(LOG_TAG,"eglMakeCurrent() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }

        mContext->surface = surface;
        return true;
#endif 
#endif     
    }

    bool FPNWindow::_onResize() {
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        if(!eglMakeCurrent(mContext->display, mContext->surface, mContext->surface, mContext->context)){
            FPN_LOGE(LOG_TAG,"eglMakeCurrent() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }

        if(!eglQuerySurface(mContext->display, mContext->surface, EGL_WIDTH, &mWidth) ||
            !eglQuerySurface(mContext->display, mContext->surface, EGL_HEIGHT, &mHeight)){
            FPN_LOGE(LOG_TAG,"eglQuerySurface() returned error %d", eglGetError());
            _onDestroy();
            return false;
        }
#endif 
#endif 
        FPN_LOGI(LOG_TAG,"Render Window is Resized: [%d, %d]", mWidth, mHeight);
        return true;
    }

    void FPNWindow::_onRelease() {
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        eglMakeCurrent(mContext->display, EGL_NO_SURFACE, EGL_NO_SURFACE, mContext->context);
        eglDestroySurface(mContext->display, mContext->surface);
        mContext->surface = EGL_NO_SURFACE;
#endif    
#endif 
    }

    void FPNWindow::_onDestroy() {
        if (!mContext) return;
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        eglMakeCurrent(mContext->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroyContext(mContext->display, mContext->context);
        if(mContext->surface != EGL_NO_SURFACE)
        {
            eglDestroySurface(mContext->display, mContext->surface);
        }
        eglTerminate(mContext->display);

        mContext->display = EGL_NO_DISPLAY;
        mContext->context = EGL_NO_CONTEXT;
        mContext->config = 0;
        mContext->window = 0;
#endif 
#endif 
        mWindowInited = false;
    }

    void FPNWindow::_onPresent() {
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
        if (!eglSwapBuffers(mContext->display, mContext->surface)) {
            FPN_LOGE(LOG_TAG,"eglSwapBuffers() returned error %d", eglGetError());
        }
#endif 
#endif 
    }

}