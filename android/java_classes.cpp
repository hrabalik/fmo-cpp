#include "java_classes.hpp"

Object::Object(JNIEnv* env, jobject obj) : mEnv(env), mObj(obj), mClass(env->GetObjectClass(obj)) {}

Callback::Callback(JNIEnv* env, jobject obj) : Object(env, obj) {}

void Callback::log(const char* cStr) {
    jmethodID method = mEnv->GetMethodID(mClass, "log", "(Ljava/lang/String;)V");
    jstring string = mEnv->NewStringUTF(cStr);
    mEnv->CallVoidMethod(mObj, method, string);
    mEnv->DeleteLocalRef(string);
}
