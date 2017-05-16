#include "java_classes.hpp"

// Object

Object::Object(JNIEnv* env, jobject obj, bool disposeOfObj) : mEnv(env), mObj(obj),
                                                              mClass(env->GetObjectClass(obj)),
                                                              mObjDelete(disposeOfObj),
                                                              mClassDelete(true) {}

Object::Object(JNIEnv* env, jobject obj, bool disposeOfObj, jclass cls, bool disposeOfCls) :
        mEnv(env),
        mObj(obj),
        mClass(cls),
        mObjDelete(
                disposeOfObj),
        mClassDelete(
                disposeOfCls) {}

Object::~Object() {
    if (mObjDelete) mEnv->DeleteLocalRef(mObj);
    if (mClassDelete) mEnv->DeleteLocalRef(mClass);
}

// Callback

namespace {
    struct CallbackBindings {
        jclass class_;
        jmethodID log;

        CallbackBindings(JNIEnv* env) {
            jclass local = env->FindClass("cz/fmo/Lib$Callback");
            class_ = (jclass) env->NewGlobalRef(local);
            env->DeleteLocalRef(local);

            log = env->GetMethodID(class_, "log", "(Ljava/lang/String;)V");
        }
    };

    std::unique_ptr<CallbackBindings> bCallback;
}

Callback::Callback(JNIEnv* env, jobject obj, bool disposeOfObj) : Object(env, obj, disposeOfObj) {}

void Callback::log(const char* cStr) {
    jstring string = mEnv->NewStringUTF(cStr);
    mEnv->CallVoidMethod(mObj, bCallback->log, string);
    mEnv->DeleteLocalRef(string);
}

// Detection

namespace {
    struct DetectionBindings {
        jclass class_;
        jmethodID init_;
        jfieldID id;
        jfieldID predecessorId;
        jfieldID centerX;
        jfieldID centerY;
        jfieldID directionX;
        jfieldID directionY;
        jfieldID length;
        jfieldID radius;
        jfieldID velocity;

        DetectionBindings(JNIEnv* env) {
            jclass local = env->FindClass("cz/fmo/Lib$Detection");
            class_ = (jclass) env->NewGlobalRef(local);
            env->DeleteLocalRef(local);

            init_ = env->GetMethodID(class_, "<init>", "()V");

            id = env->GetFieldID(class_, "id", "I");
            predecessorId = env->GetFieldID(class_, "predecessorId", "I");
            centerX = env->GetFieldID(class_, "centerX", "I");
            centerY = env->GetFieldID(class_, "centerY", "I");
            directionX = env->GetFieldID(class_, "directionX", "F");
            directionY = env->GetFieldID(class_, "directionY", "F");
            length = env->GetFieldID(class_, "length", "F");
            radius = env->GetFieldID(class_, "radius", "F");
            velocity = env->GetFieldID(class_, "velocity", "F");
        }
    };

    std::unique_ptr<DetectionBindings> bDetection;
}

Detection::Detection(JNIEnv* env, const fmo::Algorithm::Detection& det) :
        Object(env, env->NewObject(bDetection->class_,
                                   bDetection->init_), true,
               bDetection->class_, false) {
    mEnv->SetIntField(mObj, bDetection->id, det.object.id);
    mEnv->SetIntField(mObj, bDetection->predecessorId, det.predecessor.id);
    mEnv->SetIntField(mObj, bDetection->centerX, det.object.center.x);
    mEnv->SetIntField(mObj, bDetection->centerY, det.object.center.y);
    mEnv->SetFloatField(mObj, bDetection->directionX, det.object.direction[0]);
    mEnv->SetFloatField(mObj, bDetection->directionY, det.object.direction[1]);
    mEnv->SetFloatField(mObj, bDetection->length, det.object.length);
    mEnv->SetFloatField(mObj, bDetection->radius, det.object.radius);
    mEnv->SetFloatField(mObj, bDetection->velocity, det.object.velocity);
}

// initJavaClasses, releaseJavaClasses

void initJavaClasses(JNIEnv* env) {
    if (!bDetection) {
        bDetection = std::make_unique<DetectionBindings>(env);
    }
    if (!bCallback) {
        bCallback = std::make_unique<CallbackBindings>(env);
    }
}
