#include <jni.h>
#include "java_interface.hpp"

namespace {
    const char *const HELLO_STR = "Hello from C++";
}

jstring Java_cz_fmo_Lib_getHelloString(JNIEnv *env, jclass) {
    return env->NewStringUTF(HELLO_STR);
}
