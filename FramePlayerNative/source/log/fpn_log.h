#ifndef FPN_LOG_H
#define FPN_LOG_H

#include <stdio.h>

#ifdef TARGET_OS_ANDROID
#include <android/log.h>
#define FPN_LOGI(LOG_TAG, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define FPN_LOGE(LOG_TAG, ...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else 
#define FPN_LOGI(LOG_TAG, ...)                       \
    {                                        \
        printf("I/" LOG_TAG ": " __VA_ARGS__);   \
        printf("\n");                        \
    }
#define FPN_LOGI(LOG_TAG, ...)                       \
    {                                        \
        printf("E/" LOG_TAG ": " __VA_ARGS__);   \
        printf("\n");                        \
    }
#endif

#endif //FPN_LOG_H