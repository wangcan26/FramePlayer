#ifndef FPN_GIF_PRODUCER_H
#define FPN_GIF_PRODUCER_H
#include <string>
#include <thread>


namespace fpn 
{
    class FPNPlayer;
    /**
     * FPNGifProducer 
     */
    class FPNGifProducer 
    {
    public:
        FPNGifProducer(const std::string& uri, FPNPlayer *player);
        ~FPNGifProducer();

        void start();
        void stop();
        void resume();
        void pause();
    private:
        void _decode();
    private:
        FPNPlayer  *mPlayer = nullptr;
        std::thread mDecoderThread;
        std::string mGifFilePath;

        bool mIsRunning = false;
        bool mIsPlaying = false;
        bool mIsPaused = false;
        int  mCurIndex = -1;
    };
}

#endif //FPN_GIF_PRODUCER_H