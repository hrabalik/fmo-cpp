#include "java_classes.hpp"

Object::Object(JNIEnv* env, jobject obj) : mEnv(env), mObj(obj), mClass(env->GetObjectClass(obj)) {}

Callback::Callback(JNIEnv* env, jobject obj) : Object(env, obj) {}

void Callback::frameTimings(float q50, float q95, float q99) const {
    jmethodID method = mEnv->GetMethodID(mClass, "frameTimings", "(FFF)V");
    mEnv->CallVoidMethod(mObj, method, q50, q95, q99);
}

void Callback::log(const char* cStr) {
    jmethodID method = mEnv->GetMethodID(mClass, "log", "(Ljava/lang/String;)V");
    jstring string = mEnv->NewStringUTF(cStr);
    mEnv->CallVoidMethod(mObj, method, string);
    mEnv->DeleteLocalRef(string);
}
