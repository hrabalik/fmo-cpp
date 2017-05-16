#ifndef FMO_ANDROID_JAVA_CLASSES_HPP
#define FMO_ANDROID_JAVA_CLASSES_HPP

#include <algorithm>
#include <cstdint>
#include <fmo/assert.hpp>
#include <fmo/algorithm.hpp>
#include <jni.h>

// forward declarations
struct Detection;
struct DetectionArray;

/**
 * Models java.lang.Object
 */
class Object {
public:
    virtual ~Object();

    Object(const Object&) = default;

    Object& operator=(const Object&) = default;

    Object(JNIEnv* env, jobject obj, bool disposeOfObj);

protected:
    JNIEnv* const mEnv;
    const jobject mObj;
    const bool mObjDelete;
};

/**
 * Models cz.fmo.Lib$FrameCallback
 */
struct Callback : public Object {
    using Object::Object;

    virtual ~Callback() override = default;

    void log(const char* cStr);

    void onObjectsDetected(const DetectionArray& detections);
};

/**
 * Models cz.fmo.Lib$Detection
 */
struct Detection : public Object {
    using Object::Object;

    virtual ~Detection() override = default;

    Detection(JNIEnv* env, const fmo::Algorithm::Detection& det);

    friend struct DetectionArray;
};

/**
 * Models cz.fmo.Lib$Detection[]
 */
struct DetectionArray : public Object {
    using Object::Object;

    virtual ~DetectionArray() override = default;

    DetectionArray(JNIEnv* env, jsize length);

    void set(int i, const Detection& detection);

    friend void Callback::onObjectsDetected(const DetectionArray&);
};

/**
 * Wraps other objects, extending their lifetime past the duration of the native function.
 */
template<typename T>
struct Reference {
    Reference(const Reference&) = delete;

    Reference& operator=(const Reference&) = delete;

    Reference() noexcept : mObj(nullptr) {}

    Reference(Reference&& rhs) noexcept : Reference() { std::swap(mObj, rhs.mObj); }

    Reference& operator=(Reference&& rhs) noexcept {
        std::swap(mObj, rhs.mObj);
        return *this;
    }

    Reference(JNIEnv* env, jobject obj) : mObj(env->NewGlobalRef(obj)) {}

    ~Reference() {
        FMO_ASSERT(mObj == nullptr, "release() must be called before a Reference is destroyed");
    }

    void release(JNIEnv* env) {
        env->DeleteGlobalRef(mObj);
        mObj = nullptr;
    }

    T get(JNIEnv* env) { return {env, mObj, false}; }

private:
    jobject mObj;
};

/**
 * Call before using above objects. Must be called in a Java thread.
 */
void initJavaClasses(JNIEnv* env);

#endif // FMO_ANDROID_JAVA_CLASSES_HPP
