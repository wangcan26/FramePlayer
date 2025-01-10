#include "fpn_player.h"
#include "fpn_context.h"
#include "fpn_window.h"
#include "log/fpn_log.h"

#define LOG_TAG "FPNPlayer"

namespace fpn {
    FPNPlayer::FPNPlayer() {
        mContext.reset(new FPNContext());
        mWindow.reset(new FPNWindow());
        mRenderThread = std::thread(&FPNPlayer::_render, this);
    }

    FPNPlayer::~FPNPlayer() {
        mMessage = MSG_LOOP_EXIT;
        if(mRenderThread.joinable()) {
            mRenderThread.join();
        }
    }

    void FPNPlayer::makeCurrent() {
        std::unique_lock<std::mutex> rm(mRenderMutex);
        if (mContext)
        {
            mWindow->attach(mContext.get());
            if (mWindow->isValid()) {
                mMessage = MSG_WINDOW_UPDATE;
                return;
            }
            mMessage = MSG_WINDOW_CREATE;
        } else {
            mMessage = MSG_WINDOW_DESTROY;
        }
        mRenderCond.wait(rm);
    }

    FPNContext* FPNPlayer::getContext() const  {return mContext.get();}

    void FPNPlayer::_render() {
        FPN_LOGI(LOG_TAG, "FPNPlayer render...");
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
                mMessage = MSG_NONE;
                if (mWindow && mWindow->isValid()) {
                    mWindow->notify(FLAG_WINDOW_DESTROY);
                }
                mRenderCond.notify_one();
                run = false;
                break;
            }
            default:
                break;
            }
        }
    }
}