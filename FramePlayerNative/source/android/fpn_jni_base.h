#ifndef FPN_JNIBASE_H
#define FPN_JNIBASE_H

#include <jni.h>
#include <string>
#include <vector>

extern std::string jstring2string(JNIEnv *env, jstring jStr);
extern jstring string2jstring(JNIEnv *env, const char* pat);

#ifdef __cplusplus
extern "C"
{
#endif 

typedef union 
{
    JNIEnv *env;
    void    *venv;
}UnionJNIEnvToVoid;

extern JavaVM *g_vm;
extern JNIEnv *g_env;
extern int g_attatched;
extern UnionJNIEnvToVoid g_uenv;

#define ATTATCH_JNI_THREAD  g_attatched =  g_vm->AttachCurrentThread(&g_env, NULL);\
            if(g_attatched > 0)\
            {\
                g_vm->DetachCurrentThread();\
            }else{\
            }

#define DETATCH_JNI_THREAD if(g_attatched <=0)\
            {\
                g_vm->DetachCurrentThread();\
            }


#ifdef __cplusplus
}
#endif 

#endif //FPN_JNIBASE_H