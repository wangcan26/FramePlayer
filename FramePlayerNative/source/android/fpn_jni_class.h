#ifndef FPN_JNICLASS_H
#define FPN_JNICLASS_H

#include "fpn_jni_base.h"

namespace fpn 
{
    class NativeRegisterListener{
    public:
        virtual ~NativeRegisterListener() {}
        virtual void onNativeRegistered(JNIEnv *env) = 0;
    };

    class JniClass
    {
    public:

        JniClass(const std::string &path);

        virtual ~JniClass();

        virtual void initialize() = 0;

        virtual int registerNativeMethods(JNIEnv *env);

        virtual void setListener(NativeRegisterListener *listener);

    protected:
        std::vector<JNINativeMethod> mMethods;
        std::string mClassPath;
        NativeRegisterListener *mListener;
    };
}

#endif //FPN_JNICLASS_H