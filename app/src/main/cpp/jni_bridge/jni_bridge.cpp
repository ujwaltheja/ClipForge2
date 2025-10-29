#include "jni_bridge.h"
#include "../utils/logger.h"
#include <cstring>
#include <sstream>

namespace clipforge {
namespace jni {

// ============================================================================
// JNIBridge String Conversion Implementation
// ============================================================================

std::string JNIBridge::jstring_to_string(JNIEnv* env, jstring jStr) {
    if (!env || !jStr) {
        throw JNIException("Invalid JNI environment or string");
    }

    const char* cStr = env->GetStringUTFChars(jStr, nullptr);
    if (!cStr) {
        throw JNIException("Failed to get string UTF chars");
    }

    std::string result(cStr);
    env->ReleaseStringUTFChars(jStr, cStr);

    return result;
}

jstring JNIBridge::string_to_jstring(JNIEnv* env, const std::string& str) {
    if (!env) {
        throw JNIException("Invalid JNI environment");
    }

    jstring result = env->NewStringUTF(str.c_str());
    if (!result) {
        throw JNIException("Failed to create Java string");
    }

    return result;
}

std::vector<std::string> JNIBridge::jobjectArray_to_string_vector(JNIEnv* env, jobjectArray jArray) {
    std::vector<std::string> result;

    if (!env || !jArray) {
        return result;
    }

    jsize arrayLength = env->GetArrayLength(jArray);
    for (jsize i = 0; i < arrayLength; ++i) {
        jstring jStr = static_cast<jstring>(env->GetObjectArrayElement(jArray, i));
        if (jStr) {
            result.push_back(jstring_to_string(env, jStr));
            env->DeleteLocalRef(jStr);
        }
    }

    return result;
}

jobjectArray JNIBridge::string_vector_to_jobjectArray(JNIEnv* env, const std::vector<std::string>& vec) {
    if (!env) {
        throw JNIException("Invalid JNI environment");
    }

    jclass stringClass = env->FindClass("java/lang/String");
    if (!stringClass) {
        throw JNIException("Failed to find String class");
    }

    jobjectArray result = env->NewObjectArray(static_cast<jsize>(vec.size()), stringClass, nullptr);
    if (!result) {
        env->DeleteLocalRef(stringClass);
        throw JNIException("Failed to create String array");
    }

    for (size_t i = 0; i < vec.size(); ++i) {
        jstring jStr = string_to_jstring(env, vec[i]);
        env->SetObjectArrayElement(result, static_cast<jsize>(i), jStr);
        env->DeleteLocalRef(jStr);
    }

    env->DeleteLocalRef(stringClass);
    return result;
}

// ============================================================================
// Array Conversion Implementation
// ============================================================================

std::vector<int32_t> JNIBridge::jintArray_to_vector(JNIEnv* env, jintArray jArray) {
    std::vector<int32_t> result;

    if (!env || !jArray) {
        return result;
    }

    jint* elements = env->GetIntArrayElements(jArray, nullptr);
    jsize length = env->GetArrayLength(jArray);

    result.assign(elements, elements + length);
    env->ReleaseIntArrayElements(jArray, elements, JNI_ABORT);

    return result;
}

jintArray JNIBridge::vector_to_jintArray(JNIEnv* env, const std::vector<int32_t>& vec) {
    if (!env) {
        throw JNIException("Invalid JNI environment");
    }

    jintArray result = env->NewIntArray(static_cast<jsize>(vec.size()));
    if (!result) {
        throw JNIException("Failed to create int array");
    }

    env->SetIntArrayRegion(result, 0, static_cast<jsize>(vec.size()), reinterpret_cast<const jint*>(vec.data()));
    return result;
}

std::vector<int64_t> JNIBridge::jlongArray_to_vector(JNIEnv* env, jlongArray jArray) {
    std::vector<int64_t> result;

    if (!env || !jArray) {
        return result;
    }

    jlong* elements = env->GetLongArrayElements(jArray, nullptr);
    jsize length = env->GetArrayLength(jArray);

    result.assign(elements, elements + length);
    env->ReleaseLongArrayElements(jArray, elements, JNI_ABORT);

    return result;
}

jlongArray JNIBridge::vector_to_jlongArray(JNIEnv* env, const std::vector<int64_t>& vec) {
    if (!env) {
        throw JNIException("Invalid JNI environment");
    }

    jlongArray result = env->NewLongArray(static_cast<jsize>(vec.size()));
    if (!result) {
        throw JNIException("Failed to create long array");
    }

    env->SetLongArrayRegion(result, 0, static_cast<jsize>(vec.size()), reinterpret_cast<const jlong*>(vec.data()));
    return result;
}

std::vector<float> JNIBridge::jfloatArray_to_vector(JNIEnv* env, jfloatArray jArray) {
    std::vector<float> result;

    if (!env || !jArray) {
        return result;
    }

    jfloat* elements = env->GetFloatArrayElements(jArray, nullptr);
    jsize length = env->GetArrayLength(jArray);

    result.assign(elements, elements + length);
    env->ReleaseFloatArrayElements(jArray, elements, JNI_ABORT);

    return result;
}

jfloatArray JNIBridge::vector_to_jfloatArray(JNIEnv* env, const std::vector<float>& vec) {
    if (!env) {
        throw JNIException("Invalid JNI environment");
    }

    jfloatArray result = env->NewFloatArray(static_cast<jsize>(vec.size()));
    if (!result) {
        throw JNIException("Failed to create float array");
    }

    env->SetFloatArrayRegion(result, 0, static_cast<jsize>(vec.size()), reinterpret_cast<const jfloat*>(vec.data()));
    return result;
}

// ============================================================================
// Reference Management Implementation
// ============================================================================

jobject JNIBridge::local_to_global_ref(JNIEnv* env, jobject localRef) {
    if (!env || !localRef) {
        throw JNIException("Invalid JNI environment or reference");
    }

    jobject globalRef = env->NewGlobalRef(localRef);
    if (!globalRef) {
        throw JNIException("Failed to create global reference");
    }

    return globalRef;
}

void JNIBridge::delete_global_ref(JNIEnv* env, jobject globalRef) noexcept {
    if (env && globalRef) {
        env->DeleteGlobalRef(globalRef);
    }
}

void JNIBridge::delete_local_ref(JNIEnv* env, jobject localRef) noexcept {
    if (env && localRef) {
        env->DeleteLocalRef(localRef);
    }
}

// ============================================================================
// Error Handling Implementation
// ============================================================================

int JNIBridge::throw_java_exception(JNIEnv* env, const std::string& exceptionClass, const std::string& message) {
    if (!env) {
        LOG_ERROR("Cannot throw exception: null JNI environment");
        return -1;
    }

    jclass exceptionClazz = env->FindClass(exceptionClass.c_str());
    if (!exceptionClazz) {
        LOG_ERROR("Cannot find exception class: %s", exceptionClass.c_str());
        return -1;
    }

    int result = env->ThrowNew(exceptionClazz, message.c_str());
    env->DeleteLocalRef(exceptionClazz);

    return result;
}

bool JNIBridge::has_exception(JNIEnv* env) {
    if (!env) {
        return false;
    }

    return env->ExceptionCheck() == JNI_TRUE;
}

std::string JNIBridge::get_exception_message(JNIEnv* env) {
    if (!env || !has_exception(env)) {
        return "";
    }

    jthrowable exception = env->ExceptionOccurred();
    if (!exception) {
        return "";
    }

    jclass exceptionClass = env->GetObjectClass(exception);
    jmethodID toStringMethod = env->GetMethodID(exceptionClass, "toString", "()Ljava/lang/String;");

    if (toStringMethod) {
        jstring jMessage = static_cast<jstring>(env->CallObjectMethod(exception, toStringMethod));
        if (jMessage) {
            std::string message = jstring_to_string(env, jMessage);
            env->DeleteLocalRef(jMessage);
            env->DeleteLocalRef(exceptionClass);
            return message;
        }
    }

    env->DeleteLocalRef(exceptionClass);
    return "Unknown exception";
}

void JNIBridge::clear_exception(JNIEnv* env) noexcept {
    if (env) {
        env->ExceptionClear();
    }
}

void JNIBridge::print_exception(JNIEnv* env) {
    if (!env || !has_exception(env)) {
        return;
    }

    LOG_ERROR("JNI Exception: %s", get_exception_message(env).c_str());
    env->ExceptionDescribe();
    clear_exception(env);
}

// ============================================================================
// Utility Implementation
// ============================================================================

int JNIBridge::get_jni_version(JNIEnv* env) {
    if (!env) {
        return 0;
    }

    return env->GetVersion();
}

bool JNIBridge::is_main_thread() {
    // This is a simplified check - proper implementation would use pthread
    return true;  // Placeholder
}

char JNIBridge::get_type_descriptor(const std::string& type) {
    if (type == "int") return 'I';
    if (type == "long") return 'J';
    if (type == "float") return 'F';
    if (type == "double") return 'D';
    if (type == "boolean") return 'Z';
    if (type == "byte") return 'B';
    if (type == "char") return 'C';
    if (type == "short") return 'S';
    if (type == "void") return 'V';
    return 'L';
}

} // namespace jni
} // namespace clipforge
