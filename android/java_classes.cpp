#include "java_classes.hpp"

Object::Object(JNIEnv* env, jobject obj) : mEnv(env), mObj(obj), mClass(env->GetObjectClass(obj)) {}

Callback::Callback(JNIEnv* env, jobject obj)
    : Object(env, obj), mFrameTimings(env->GetMethodID(mClass, "frameTimings", "(FFF)V")) {}

void Callback::frameTimings(float q50, float q95, float q99) const {
    mEnv->CallVoidMethod(mObj, mFrameTimings, q50, q95, q99);
}
