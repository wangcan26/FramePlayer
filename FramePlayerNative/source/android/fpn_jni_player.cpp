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

std::unordered_map<long, std::shared_ptr<fpn::FPNPlayer>> m_players;

static long player_create(JNIEnv* jenv, jobject obj) {
    FPN_LOGI(LOG_TAG, "player created");
    fpn::FPNPlayer *player = new fpn::FPNPlayer();
    long handle = (long)player;
    m_players[handle] = std::shared_ptr<fpn::FPNPlayer>(player);
    return handle;
}

static void player_release(JNIEnv* jenv, jobject obj, jlong handle) {
    FPN_LOGI(LOG_TAG, "player released");
    m_players[handle] = nullptr;
}

static void player_start(JNIEnv* jenv, jobject obj, jlong handle) {
    FPN_LOGI(LOG_TAG, "player started");
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    player->start();
}

static void player_set_content(JNIEnv* jenv, jobject obj, jlong handle, jstring uri) {
    std::string contentUri = jstring2string(jenv, uri);
    FPN_LOGI(LOG_TAG, "player set content: %s", contentUri.c_str());
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    player->setContentUri(contentUri);
}

static void player_set_surface(JNIEnv* jenv, jobject obj, jlong handle, jobject surface) {
    FPN_LOGI(LOG_TAG, "player set surface, %d, %ld", surface, handle);
    fpn::FPNPlayer *player = (fpn::FPNPlayer*)handle;
    if (surface != 0) {
        ANativeWindow *window = ANativeWindow_fromSurface(jenv, surface);
        player->getContext()->window = window;
    } else {
        ANativeWindow_release(player->getContext()->window);
        player->getContext()->window = nullptr;
    }
    player->makeCurrent();
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