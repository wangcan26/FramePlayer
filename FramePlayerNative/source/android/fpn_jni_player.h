#ifndef FPN_JNIVIEW_H
#define FPN_JNIVIEW_H

#include "fpn_jni_class.h"

namespace fpn 
{
    class JniPlayer : public JniClass 
    {
    public:
        JniPlayer();
        JniPlayer(const std::string& path);
        ~JniPlayer();

        void initialize() override;
    };
}

#endif //FPN_JNIVIEW_H