#include "fpn_log.h"

#define LOG_TAG "FPN_MAIN"

#ifdef TARGET_OS_ANDROID
#include "fpn_jni_base.h"

#ifdef __cplusplus
extern "C"
{
#endif 

//Global variables
UnionJNIEnvToVoid g_uenv;
JavaVM *g_vm = NULL;
JNIEnv *g_env = NULL;
int     g_attatched = 1;

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    g_uenv.venv = 0;
    jint result = -1;
    JNIEnv *env = 0;

    if(vm->GetEnv(&g_uenv.venv, JNI_VERSION_1_6)!=JNI_OK){
        FPN_LOGI(LOG_TAG, "jni load 1.6 faild");
        goto fail;
    }

    result = JNI_VERSION_1_6;
    FPN_LOGI(LOG_TAG, "jni load 1.6 successful");

    fail:
    return result;

}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    FPN_LOGI(LOG_TAG, "jni onLoad successful");
}

#ifdef __cplusplus
}
#endif 
#endif //TARGET_OS_ANDROID