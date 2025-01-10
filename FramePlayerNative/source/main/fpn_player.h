#ifndef FPN_PLAYER_H
#define FPN_PLAYER_H

#include <memory>
#include <thread>

namespace fpn 
{
    struct FPNContext;
    class FPNWindow;
    class FPNPlayer {
    public:
        FPNPlayer();
        ~FPNPlayer();

        void makeCurrent();
        FPNContext *getContext() const;

    private:
        void _render();

    private:
        std::unique_ptr<FPNContext> mContext;
        std::unique_ptr<FPNWindow>  mWindow;
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
    };
}

#endif 