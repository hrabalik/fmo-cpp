#include "java_classes.hpp"

// Object

Object::Object(JNIEnv* env, jobject obj, bool disposeOfObj) : mEnv(env), mObj(obj),
                                                                 mClass(env->GetObjectClass(obj)),
                                                                 mObjDelete(disposeOfObj),
                                                                 mClassDelete(true) {}

Object::Object(JNIEnv* env, jclass cls, bool disposeOfCls) : mEnv(env),
                                                               mObj(env->NewObject(cls,
                                                                                   env->GetMethodID(
                                                                                           cls,
                                                                                           "<init>",
                                                                                           "()V"))),
                                                               mClass(cls), mObjDelete(true),
                                                               mClassDelete(disposeOfCls) {}

Object::~Object() {
    if (mObjDelete) mEnv->DeleteLocalRef(mObj);
    if (mClassDelete) mEnv->DeleteLocalRef(mClass);
}

// Callback

Callback::Callback(JNIEnv* env, jobject obj, bool disposeOfObj) : Object(env, obj, disposeOfObj) {}

void Callback::log(const char* cStr) {
    jmethodID method = mEnv->GetMethodID(mClass, "log", "(Ljava/lang/String;)V");
    jstring string = mEnv->NewStringUTF(cStr);
    mEnv->CallVoidMethod(mObj, method, string);
    mEnv->DeleteLocalRef(string);
}

// Detection

Detection::Detection(JNIEnv* env, const fmo::Algorithm::Detection& det) :
        Object(env, env->FindClass("cz/fmo/Lib$Detection"), true) {}
