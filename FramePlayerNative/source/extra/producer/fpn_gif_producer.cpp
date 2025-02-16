#include "fpn_gif_producer.h"
#include "main/fpn_player.h"
#include "log/fpn_log.h"
#include "main/fpn_context.h"
#include <chrono>


#define LOG_TAG "FPNGifProducer"

#ifdef __cplusplus
extern "C"
{
#endif
#include "nsgif/libnsgif.h"
static void *bitmap_create(int width, int height) {
    return calloc((size_t) (width * height), 4);
}

static void bitmap_set_opaque(void *bitmap, bool opaque) {
    (void) opaque; /* unused */
}

static bool bitmap_test_opaque(void *bitmap) {
    return false;
}

static unsigned char *bitmap_get_buffer(void *bitmap) {
    return (unsigned char*)bitmap;
}

static void bitmap_destroy(void *bitmap) {
    free(bitmap);
}

static void bitmap_modified(void *bitmap) {
    return;
}

static gif_bitmap_callback_vt bitmap_callbacks = {bitmap_create, bitmap_destroy, bitmap_get_buffer,
                                           bitmap_set_opaque, bitmap_test_opaque, bitmap_modified};

static void showError(const char *context, gif_result code) {
    switch (code) {
        case GIF_INSUFFICIENT_FRAME_DATA:
            FPN_LOGE(LOG_TAG, "%s failed: GIF_INSUFFICIENT_FRAME_DATA", context);
            break;
        case GIF_FRAME_DATA_ERROR:
            FPN_LOGE(LOG_TAG, "%s failed: GIF_FRAME_DATA_ERROR", context);
            break;
        case GIF_INSUFFICIENT_DATA:
            FPN_LOGE(LOG_TAG, "%s failed: GIF_INSUFFICIENT_DATA", context);
            break;
        case GIF_DATA_ERROR:
            FPN_LOGE(LOG_TAG, "%s failed: GIF_DATA_ERROR", context);
            break;
        case GIF_INSUFFICIENT_MEMORY:
            FPN_LOGE(LOG_TAG, "%s failed: GIF_INSUFFICIENT_MEMORY", context);
            break;
        default:
            FPN_LOGE(LOG_TAG, "%s failed: unknown code %i", context, code);
            break;
    }
}

static int getFrame(gif_animation *gif, int index) {
    gif_result code;
    if (gif == NULL) {
        FPN_LOGE(LOG_TAG, "gif is null");
        return -1;
    }
    if (index < 0 || index >= gif->frame_count) {
        FPN_LOGE(LOG_TAG, "mCurIndex error");
        return -1;
    }
    code = gif_decode_frame(gif, index);
    if (code != GIF_OK) {
        showError("gif_decode_frame", code);
        return -1;
    }
    return gif->frames[index].frame_delay;
}


static unsigned char *loadFile(const char *path, size_t *pFileLength) {
    FILE *fd;
    long size;
    unsigned char *data = NULL;
    size_t n;

    fd = fopen(path, "rbe");
    if (!fd) {
        FPN_LOGE(LOG_TAG, "File open error[%s]", path);
        return NULL;
    }
    fseek(fd, 0, SEEK_END);
    size = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    data = (unsigned char*)malloc(size);
    if (data == NULL) {
        FPN_LOGE(LOG_TAG, "Unable to allocate [%lld] bytes", (long long) size);
        fclose(fd);
        return NULL;
    }
    n = fread(data, 1, size, fd);
    fclose(fd);
    if (n != size) {
        FPN_LOGE(LOG_TAG, "Read to buffer from [%s] error", path);
        free(data);
        return NULL;
    }
    *pFileLength = size;
    return data;
}

#ifdef __cplusplus
}
#endif

namespace fpn 
{
    FPNGifProducer::FPNGifProducer(const std::string& uri, FPNPlayer* player)
    {
        mPlayer = player;
        mGifFilePath = uri + std::string("/gif/testGif.gif");
        mIsRunning = true;
        mDecoderThread = std::thread(&FPNGifProducer::_decode, this);
    }

    FPNGifProducer::~FPNGifProducer() 
    {
        mIsRunning = false;
        stop();
        if(mDecoderThread.joinable()) {
            mDecoderThread.join();
        }
    }

    void FPNGifProducer::start()
    {
        mIsPlaying = true;
    }

    void FPNGifProducer::resume()
    {
        mIsPaused = false;
    }

    void FPNGifProducer::pause()
    {
        mIsPaused = true;
    }

    void FPNGifProducer::stop()
    {
        mIsPlaying = false;
        mIsPaused = false;
        mCurIndex = -1;
    }

    void FPNGifProducer::_decode() {
        FPN_LOGI(LOG_TAG, "begin to decode gif: %s", mGifFilePath.c_str());
        gif_result code;
        gif_animation *gif = (gif_animation*) malloc(sizeof(gif_animation));
        if (gif == NULL) {
            FPN_LOGE(LOG_TAG, "unable to allocate gif_animation");
            return;
        }
        gif_create(gif, &bitmap_callbacks);
        size_t length;
        unsigned char *buffer = loadFile(mGifFilePath.c_str(), &length);
        if (buffer == NULL) 
        {
            free(gif);
            FPN_LOGE(LOG_TAG, "gif data read failed");
            return;
        }
        FPN_LOGI(LOG_TAG, "begin to decoding: %zu", length);
        do {
            code = gif_initialise(gif, length, buffer);
            if (code != GIF_OK && code != GIF_WORKING) {
                showError("gif_initialise", code);
                return;
            }
        } while(code != GIF_OK);
        FPN_LOGI(LOG_TAG,  "decode info: { frame count: %d, width: %d, height: %d}", gif->frame_count, gif->width, gif->height);

        while(mIsRunning) {
            if (mIsPlaying) {
                int frameIndex = (mCurIndex + 1) % gif->frame_count;
                //decode Frame
                FPNImageData image;
                image.format = FPNImageFormat::RGBA8Unorm;
                image.width = gif->width;
                image.height = gif->height;
                int delay = getFrame(gif, frameIndex);
                image.data = gif->frame_image;
                if (mPlayer) {
                    mPlayer->frame(&image);
                }
                mCurIndex = frameIndex;
                std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            }
        }

    }
}