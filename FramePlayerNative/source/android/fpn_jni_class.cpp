#include "fpn_jni_class.h"

namespace fpn
{
    JniClass::JniClass(const std::string& path):
            mClassPath(path),
            mListener(NULL)
    {

    }

    JniClass::~JniClass(){}

    void JniClass::setListener(NativeRegisterListener* listener)
    {
        mListener = listener;
    }

    int JniClass::registerNativeMethods(JNIEnv *env)
    {
        jclass clazz = env->FindClass(mClassPath.c_str());


        if(clazz == 0){
            return JNI_FALSE;
        }

        if(env->RegisterNatives(clazz, mMethods.data(), mMethods.size()) < 0)
        {
            return JNI_FALSE;
        }

        if(mListener != NULL)
        {
            mListener->onNativeRegistered(env);

        }

        return JNI_TRUE;
    }

}

