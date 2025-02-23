#include "log/fpn_log.h"

#define LOG_TAG "FPN_MAIN"

#ifdef TARGET_OS_ANDROID
#include "android/fpn_jni_base.h"
#include "android/fpn_jni_class.h"

#include "android/fpn_jni_player.h"

#ifdef __cplusplus
extern "C"
{
#endif 

//Global variables
UnionJNIEnvToVoid g_uenv;
JavaVM *g_vm = NULL;
JNIEnv *g_env = NULL;
int     g_attatched = 1;

typedef std::vector<fpn::JniClass*> JniClassList;
JniClassList jni_class_list;

#define ADD_JNI_CLASS(type) fpn::JniClass *type##_ = new fpn::type; \
      jni_class_list.push_back(type##_);

static void add_jni_classes()
{
    ADD_JNI_CLASS(JniPlayer);
}

static int register_jni_classes(JNIEnv *env)
{
    int flag = JNI_TRUE;
    for(int i = 0; i < jni_class_list.size(); i++)
    {
        fpn::JniClass *clazz = jni_class_list.at(i);
        flag &= clazz->registerNativeMethods(env);
    }
    return flag;
}

static void remove_jni_classes()
{
    for(int i = 0; i < jni_class_list.size(); i++)
    {
        fpn::JniClass *clazz = jni_class_list.at(i);
        delete clazz;
    }
    jni_class_list.clear();
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    g_uenv.venv = 0;
    jint result = -1;
    JNIEnv *env = 0;

    if(vm->GetEnv(&g_uenv.venv, JNI_VERSION_1_6)!=JNI_OK){
        FPN_LOGI(LOG_TAG, "jni load 1.6 faild");
        goto fail;
    }
    env = g_uenv.env;

    add_jni_classes();
    
    if(register_jni_classes(env) != JNI_TRUE)
    {
        remove_jni_classes();
        goto fail;
    }

    result = JNI_VERSION_1_6;
    g_vm = vm;
    g_env = env;
    FPN_LOGI(LOG_TAG, "jni load 1.6 successful");

    fail:
    return result;

}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    FPN_LOGI(LOG_TAG, "jni onUnLoad...");
    remove_jni_classes();
}

#ifdef __cplusplus
}
#endif 
#endif //TARGET_OS_ANDROID