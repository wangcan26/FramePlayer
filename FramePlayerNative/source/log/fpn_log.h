#ifndef FPN_LOG_H_
#define FPN_LOG_H_

#ifdef TARGET_OS_ANDROID
#include <android/log.h>

#define FPN_LOGI(LOG_TAG, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define FPN_LOGE(LOG_TAG, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#endif

#endif //FPN_LOG_H_