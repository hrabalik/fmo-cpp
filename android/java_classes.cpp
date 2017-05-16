#include "java_classes.hpp"

// Object

Object::Object(JNIEnv* env, jobject obj, bool disposeOfObj) :
        mEnv(env),
        mObj(obj),
        mObjDelete(disposeOfObj) {}

Object::~Object() {
    if (mObjDelete) mEnv->DeleteLocalRef(mObj);
}

// Callback

namespace {
    struct CallbackBindings {
        jclass class_;
        jmethodID log;
        jmethodID onObjectsDetected;

        CallbackBindings(JNIEnv* env) {
            jclass local = env->FindClass("cz/fmo/Lib$Callback");
            class_ = (jclass) env->NewGlobalRef(local);
            env->DeleteLocalRef(local);

            log = env->GetMethodID(class_, "log", "(Ljava/lang/String;)V");
            onObjectsDetected = env->GetMethodID(class_, "onObjectsDetected",
                                                 "([Lcz/fmo/Lib$Detection;)V");
        }
    };

    std::unique_ptr<CallbackBindings> bCallback;
}

void Callback::log(const char* cStr) {
    jstring string = mEnv->NewStringUTF(cStr);
    mEnv->CallVoidMethod(mObj, bCallback->log, string);
    mEnv->DeleteLocalRef(string);
}

void Callback::onObjectsDetected(const DetectionArray& detections) {
    mEnv->CallVoidMethod(mObj, bCallback->onObjectsDetected, detections.mObj);
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
        Object(env,
               env->NewObject(bDetection->class_, bDetection->init_),
               true) {
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

// DetectionArray

DetectionArray::DetectionArray(JNIEnv* env, jsize length) :
        Object(env, env->NewObjectArray(length, bDetection->class_, nullptr), true) {}

void DetectionArray::set(int i, const Detection& detection) {
    mEnv->SetObjectArrayElement((jobjectArray) mObj, i, detection.mObj);
}

// initJavaClasses

void initJavaClasses(JNIEnv* env) {
    if (!bDetection) {
        bDetection = std::make_unique<DetectionBindings>(env);
    }
    if (!bCallback) {
        bCallback = std::make_unique<CallbackBindings>(env);
    }
}
