#include "fpn_jni_player.h"
#include "log/fpn_log.h"
#include <unordered_map>
#include <memory>
#include "main/fpn_context.h"
#include "main/fpn_player.h"
#include <android/native_window.h> // requires ndk r5 or newer
#include <android/native_window_jni.h> // requires ndk r5 or newer

#define LOG_TAG "FPNPlayer"

#ifdef __cplusplus
extern "C"
{
#endif

static jlong player_create(JNIEnv* jenv, jobject obj) {
    fpn::FPNPlayer *player = new fpn::FPNPlayer();
    jlong handle = (jlong)player;
    return handle;
}

static void player_release(JNIEnv* jenv, jobject obj, jlong handle) {
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    if (player) {
        player->release();
        player = nullptr;
    }
    FPN_LOGI(LOG_TAG, "player released: %ld", handle);
}

static void player_start(JNIEnv* jenv, jobject obj, jlong handle) {
    FPN_LOGI(LOG_TAG, "player started: %ld", handle);
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    player->start();
}

static void player_set_content(JNIEnv* jenv, jobject obj, jlong handle, jstring uri) {
    FPN_LOGI(LOG_TAG, "player set content: %s, %ld", "", handle);
    std::string contentUri = jstring2string(jenv, uri);
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    player->setContentUri(contentUri);
}

static void player_set_surface(JNIEnv* jenv, jobject obj, jlong handle, jobject surface) {
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    if (surface != 0) {
        ANativeWindow *window = ANativeWindow_fromSurface(jenv, surface);
        ANativeWindow_acquire(window);
        /*std::shared_ptr<ANativeWindow> windowRef(window, [](ANativeWindow *window) {
            ANativeWindow_release(window);
        });*/
        player->getContext()->window = window;
        player->makeCurrent();
    } else {
        ANativeWindow *window = player->getContext()->window;
        player->getContext()->window = nullptr;
        player->makeCurrent();
        ANativeWindow_release(window);
    }
    FPN_LOGI(LOG_TAG, "player set surface, %p, %ld", surface, handle);
}

#ifdef __cplusplus
}
#endif

namespace fpn {
    JniPlayer::JniPlayer(): JniClass("com/demo/player/FPNPlayer") {
        initialize();
    }

    JniPlayer::JniPlayer(const std::string &path): JniClass(path) {
        initialize();
    }

    JniPlayer::~JniPlayer() {}

    void JniPlayer::initialize() {
        mMethods = {
                {"nativeSetSurface", "(JLandroid/view/Surface;)V", (void*)player_set_surface},
                {"nativeSetContentUri", "(JLjava/lang/String;)V", (void*)player_set_content},
                {"nativeCreate", "()J", (void*)player_create},
                {"nativeStart", "(J)V", (void*)player_start},
                {"nativeRelease", "(J)V", (void*)player_release}
        };
    }
}