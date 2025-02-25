#include "fpn_player.h"
#include "fpn_context.h"
#include "fpn_window.h"
#include "log/fpn_log.h"
#ifdef FPN_USE_EXTRA_RENDER
#include "extra/render/fpn_canvas.h"
#endif
#ifdef FPN_USE_EXTRA_PRODUCER
#include "extra/producer/fpn_gif_producer.h"
#endif 

#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif
#endif 

#define LOG_TAG "FPNPlayer"

namespace fpn {
    FPNPlayer::FPNPlayer() {
        mContext = std::shared_ptr<FPNContext>(new FPNContext());
        mWindow.reset(new FPNWindow());
        mWindow->attach(mContext);
        mRenderThread = std::thread(&FPNPlayer::_render, this);
        FPN_LOGI(LOG_TAG, "FPNPlayer constructor");
    }

    FPNPlayer::~FPNPlayer() {
        FPN_LOGI(LOG_TAG, "FPNPlayer deconstructor");
        release();
    }

    void FPNPlayer::release() {
        std::unique_lock<std::mutex> rm(mRenderMutex);
        if (mMessage != MSG_LOOP_EXIT) {
            mMessage = MSG_LOOP_EXIT;
            mRenderCond.wait(rm);
        }
        if(mRenderThread.joinable()) {
            mRenderThread.join();
        }
        FPN_LOGI(LOG_TAG, "FPNPlayer is released");
    }

    void FPNPlayer::setContentUri(const std::string& uri) {
        mContentUri = uri;
    }

    void FPNPlayer::start() {
        mStarted = true;
    }

    void FPNPlayer::makeCurrent() {
        std::unique_lock<std::mutex> rm(mRenderMutex);
        mMessage = MSG_NONE;
        do {
            if (mContext && mContext->window)
            {
                mIsPaused = false;
                if (mWindow->isValid()) {
                    mMessage = MSG_WINDOW_UPDATE;
                    break;
                }
                mMessage = MSG_WINDOW_CREATE;
            } else {
                mIsPaused = true;
                mMessage = MSG_WINDOW_DESTROY;
            }
        } while (0);
        if (mMessage != MSG_NONE && mStarted) {
            mRenderCond.wait(rm);
        }
    }

    FPNContext* FPNPlayer::getContext() const  {return mContext.get();}

    void FPNPlayer::frame(struct FPNImageData* data) {
#ifdef FPN_USE_EXTRA_RENDER
        if (mCanvas && !mIsPaused) {
            mCanvas->opaque(data);
        }
#endif 
    }

    void FPNPlayer::_render() {
        bool run = true;
        while(run) {
            switch (mMessage)
            {
            case MSG_WINDOW_CREATE:
            {
                std::lock_guard<std::mutex> rm(mRenderMutex);
                run = mWindow->notify(!mWindow->isInited()? FLAG_WINDOW_CREATE: FLAG_WINDOW_RESTORE);
                mMessage = MSG_NONE;
                mRenderCond.notify_one();
                break;
            }
            case MSG_WINDOW_UPDATE:
            {
                std::lock_guard<std::mutex> rm(mRenderMutex);
                mWindow->notify(FLAG_WINDOW_RESIZE);
                mMessage = MSG_NONE;
                mRenderCond.notify_one();
                break;
            }
            case MSG_WINDOW_DESTROY:
            {
                std::lock_guard<std::mutex> rm(mRenderMutex);
                if (mWindow->isValid()) {
                    mWindow->notify(FLAG_WINDOW_RELEASE);
                }
                mMessage = MSG_NONE;
                mRenderCond.notify_one();
                break;
            }
            case MSG_LOOP_EXIT:
            {
                std::lock_guard<std::mutex> rm(mRenderMutex);
                if (mWindow) {
                    mWindow->notify(FLAG_WINDOW_DESTROY);
                }
                //mMessage = MSG_NONE;
                mRenderCond.notify_one();
                mStarted = false;
                run = false;
                break;
            }
            default:
                break;
            }
            if (mWindow->isValid() && !mIsPaused) {
#ifdef FPN_USE_OPENGL_API
#ifdef TARGET_OS_ANDROID   
                glDisable(GL_DEPTH_TEST);
                if (mTransparent) {
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }
                glClearColor(0.0, 0.0, 0.0, mTransparent ? 0.0 : 1.0);
                glClear(GL_COLOR_BUFFER_BIT); //
                glViewport(0, 0, mWindow->getWidth(), mWindow->getHeight());
                
#endif
#endif  
                
                //Begin draw
                if (!mIsReady && mStarted) {
#ifdef FPN_USE_EXTRA_RENDER
                    mCanvas.reset(new FPNCanvas(mWindow->getWidth(), mWindow->getHeight()));
#endif
#ifdef FPN_USE_EXTRA_PRODUCER
                    mGifProducer.reset(new FPNGifProducer(mContentUri, this));
                    mGifProducer->start();
#endif 
                    mIsReady = true;
                }


                if (isStarted()) {
#ifdef FPN_USE_EXTRA_RENDER
                    mCanvas->paint();
#endif
                }
                
                mWindow->notify(FLAG_WINDOW_PRERENT);
            }
#ifdef FPN_USE_EXTRA_PRODUCER
            if (mFrameCount < 10) {
                if (mGifProducer) {
                    mIsPaused? mGifProducer->pause() : mGifProducer->resume();
                }
                mFrameCount++;
            }
#endif 
        }
    }
}