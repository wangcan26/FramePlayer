#ifndef FPN_PLAYER_H
#define FPN_PLAYER_H

#include <memory>
#include <thread>
#include <string>

namespace fpn 
{
    struct FPNContext;
    struct FPNImageData;
    class FPNWindow;
#ifdef FPN_USE_EXTRA_RENDER
    class FPNCanvas;
#endif 
#ifdef FPN_USE_EXTRA_PRODUCER
    class FPNGifProducer;
#endif 
    class FPNPlayer {
    public:
        FPNPlayer();
        ~FPNPlayer();

        void release();

        void setContentUri(const std::string& uri);
        void start();
        bool isStarted() const {return mIsReady && mStarted;}

        void makeCurrent();
        FPNContext *getContext() const;

        //The {FPNImageData} data will be copied and cached to enable efficient asynchronous execution
        //between cpu and gpu.
        //Avoid to change width, height or foramt of the data for performance.
        void frame(struct FPNImageData* data);
    private:
        void _render();

    private:
        std::shared_ptr<FPNContext> mContext;
        std::unique_ptr<FPNWindow>  mWindow;
#ifdef FPN_USE_EXTRA_RENDER
        std::unique_ptr<FPNCanvas>  mCanvas;
#endif 
#ifdef FPN_USE_EXTRA_PRODUCER
        std::unique_ptr<FPNGifProducer> mGifProducer;
#endif 
        //Rendering Thread resources
        std::thread mRenderThread;
        std::mutex mRenderMutex;
        std::condition_variable mRenderCond;
        enum RenderMessage {
            MSG_NONE = 0,
            MSG_WINDOW_CREATE = 1,
            MSG_WINDOW_UPDATE = 2,
            MSG_WINDOW_DESTROY = 3,
            MSG_LOOP_EXIT = 4
        };
        enum RenderMessage mMessage = RenderMessage::MSG_NONE;
        bool mIsPaused = false;
        bool mIsReady = false;
        bool mStarted = false;
        std::string mContentUri;

        //options 
        bool mTransparent = false;
        int mFrameCount = 0;
    };
}

#endif 