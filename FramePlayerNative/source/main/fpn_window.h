#ifndef FPN_WINDOW_H
#define FPN_WINDOW_H

#include <memory>

namespace fpn 
{
    struct FPNContext;
    enum WindowFlag {
        FLAG_WINDOW_CREATE = 0,
        FLAG_WINDOW_RESTORE = 1,
        FLAG_WINDOW_RESIZE = 2,
        FLAG_WINDOW_RELEASE = 3,
        FLAG_WINDOW_DESTROY = 4,
        FLAG_WINDOW_PRERENT = 5
    };

    class FPNWindow 
    {
    public: 
        FPNWindow();
        ~FPNWindow();

        void attach(const std::shared_ptr<FPNContext>& context);
        bool isInited() const {return mWindowInited;}
        bool isValid() const;

        int getWidth() const {return mWidth;}
        int getHeight() const {return mHeight;}


        bool notify(int what);
    private:
        bool _onCreate();
        bool _onRestore();
        void _onRelease();
        void _onDestroy();
        bool _onResize();
        void _onPresent();
    private:
        std::shared_ptr<FPNContext> mContext = nullptr;
        int mWidth, mHeight;
        bool mWindowInited = false;
    };
}

#endif //FPN_WINDOW_H