#ifndef CLIPFORGE_JNI_BRIDGE_H
#define CLIPFORGE_JNI_BRIDGE_H

/**
 * @file jni_bridge.h
 * @brief JNI helper utilities for safe Java/C++ interoperability
 *
 * This header provides utilities for converting between Java and C++ types,
 * managing JNI references, and error handling across the JNI boundary.
 */

#include <jni.h>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>

namespace clipforge {
namespace jni {

/**
 * @class JNIException
 * @brief Exception for JNI operation failures
 */
class JNIException : public std::runtime_error {
public:
    explicit JNIException(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @class JNIBridge
 * @brief Collection of JNI utility functions for safe operations
 *
 * All functions in this class are thread-safe and handle proper reference
 * management according to JNI specifications.
 */
class JNIBridge {
public:
    // ===== String Conversion =====

    /**
     * @brief Convert Java String to C++ std::string
     * @param env JNI environment pointer
     * @param jStr Java String object
     * @return C++ string (empty string if jStr is nullptr)
     * @throws JNIException if GetStringUTFChars fails
     */
    static std::string jstring_to_string(JNIEnv* env, jstring jStr);

    /**
     * @brief Convert C++ string to Java String
     * @param env JNI environment pointer
     * @param str C++ string
     * @return Java String object (must be released by caller if keeping)
     * @throws JNIException if NewStringUTF fails
     */
    static jstring string_to_jstring(JNIEnv* env, const std::string& str);

    /**
     * @brief Convert Java String array to C++ vector of strings
     * @param env JNI environment pointer
     * @param jArray Java String array
     * @return Vector of C++ strings
     * @throws JNIException on conversion failure
     */
    static std::vector<std::string> jobjectArray_to_string_vector(
        JNIEnv* env, jobjectArray jArray);

    /**
     * @brief Convert C++ vector of strings to Java String array
     * @param env JNI environment pointer
     * @param vec Vector of strings
     * @return Java String array
     * @throws JNIException on conversion failure
     */
    static jobjectArray string_vector_to_jobjectArray(
        JNIEnv* env, const std::vector<std::string>& vec);

    // ===== Integer Array Conversion =====

    /**
     * @brief Convert Java int array to C++ vector
     * @param env JNI environment pointer
     * @param jArray Java int array
     * @return Vector of integers
     * @throws JNIException on conversion failure
     */
    static std::vector<int32_t> jintArray_to_vector(JNIEnv* env, jintArray jArray);

    /**
     * @brief Convert C++ vector to Java int array
     * @param env JNI environment pointer
     * @param vec Vector of integers
     * @return Java int array
     * @throws JNIException on conversion failure
     */
    static jintArray vector_to_jintArray(JNIEnv* env, const std::vector<int32_t>& vec);

    /**
     * @brief Convert Java long array to C++ vector
     * @param env JNI environment pointer
     * @param jArray Java long array
     * @return Vector of longs
     * @throws JNIException on conversion failure
     */
    static std::vector<int64_t> jlongArray_to_vector(JNIEnv* env, jlongArray jArray);

    /**
     * @brief Convert C++ vector to Java long array
     * @param env JNI environment pointer
     * @param vec Vector of longs
     * @return Java long array
     * @throws JNIException on conversion failure
     */
    static jlongArray vector_to_jlongArray(JNIEnv* env, const std::vector<int64_t>& vec);

    /**
     * @brief Convert Java float array to C++ vector
     * @param env JNI environment pointer
     * @param jArray Java float array
     * @return Vector of floats
     * @throws JNIException on conversion failure
     */
    static std::vector<float> jfloatArray_to_vector(JNIEnv* env, jfloatArray jArray);

    /**
     * @brief Convert C++ vector to Java float array
     * @param env JNI environment pointer
     * @param vec Vector of floats
     * @return Java float array
     * @throws JNIException on conversion failure
     */
    static jfloatArray vector_to_jfloatArray(JNIEnv* env, const std::vector<float>& vec);

    // ===== Object Management =====

    /**
     * @brief Create a new instance of a Java class
     * @param env JNI environment pointer
     * @param className Fully qualified class name (e.g., "java/lang/Object")
     * @return Local reference to new object
     * @throws JNIException if creation fails
     */
    static jobject create_java_object(JNIEnv* env, const std::string& className);

    /**
     * @brief Get a field from a Java object
     * @param env JNI environment pointer
     * @param obj Java object
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param signature Field signature (e.g., "I" for int)
     * @return Field value as jvalue
     * @throws JNIException if field not found
     */
    static jvalue get_object_field(JNIEnv* env, jobject obj,
                                   const std::string& className,
                                   const std::string& fieldName,
                                   const std::string& signature);

    /**
     * @brief Set a field in a Java object
     * @param env JNI environment pointer
     * @param obj Java object
     * @param className Fully qualified class name
     * @param fieldName Field name
     * @param signature Field signature
     * @param value Value to set
     * @throws JNIException if operation fails
     */
    static void set_object_field(JNIEnv* env, jobject obj,
                                const std::string& className,
                                const std::string& fieldName,
                                const std::string& signature,
                                jvalue value);

    /**
     * @brief Call a method on a Java object
     * @param env JNI environment pointer
     * @param obj Java object
     * @param className Fully qualified class name
     * @param methodName Method name
     * @param signature Method signature
     * @param args Method arguments
     * @return Return value from method
     * @throws JNIException if method not found
     */
    static jvalue call_object_method(JNIEnv* env, jobject obj,
                                     const std::string& className,
                                     const std::string& methodName,
                                     const std::string& signature,
                                     const std::vector<jvalue>& args = {});

    // ===== Reference Management =====

    /**
     * @brief Convert local reference to global reference
     * @param env JNI environment pointer
     * @param localRef Local reference
     * @return Global reference (caller responsible for deletion)
     * @throws JNIException if conversion fails
     */
    static jobject local_to_global_ref(JNIEnv* env, jobject localRef);

    /**
     * @brief Delete a global reference
     * @param env JNI environment pointer
     * @param globalRef Global reference to delete
     *
     * Safe to call multiple times with same reference (will be no-op).
     */
    static void delete_global_ref(JNIEnv* env, jobject globalRef) noexcept;

    /**
     * @brief Delete a local reference
     * @param env JNI environment pointer
     * @param localRef Local reference to delete
     *
     * Safe to call multiple times with same reference (will be no-op).
     */
    static void delete_local_ref(JNIEnv* env, jobject localRef) noexcept;

    /**
     * @class GlobalRef
     * @brief RAII wrapper for JNI global references
     *
     * Automatically deletes global reference on destruction.
     */
    class GlobalRef {
    public:
        /**
         * @brief Construct from global reference
         * @param env JNI environment
         * @param ref Global reference
         */
        GlobalRef(JNIEnv* env, jobject ref) : m_env(env), m_ref(ref) {}

        /**
         * @brief Move constructor
         */
        GlobalRef(GlobalRef&& other) noexcept : m_env(other.m_env), m_ref(other.m_ref) {
            other.m_ref = nullptr;
        }

        /**
         * @brief Move assignment
         */
        GlobalRef& operator=(GlobalRef&& other) noexcept {
            if (this != &other) {
                delete_global_ref(m_env, m_ref);
                m_env = other.m_env;
                m_ref = other.m_ref;
                other.m_ref = nullptr;
            }
            return *this;
        }

        /**
         * @brief Destructor - releases reference
         */
        ~GlobalRef() { delete_global_ref(m_env, m_ref); }

        // Prevent copying
        GlobalRef(const GlobalRef&) = delete;
        GlobalRef& operator=(const GlobalRef&) = delete;

        /**
         * @brief Get raw reference
         */
        [[nodiscard]] jobject get() const { return m_ref; }

        /**
         * @brief Check if reference is valid
         */
        [[nodiscard]] bool is_valid() const { return m_ref != nullptr; }

    private:
        JNIEnv* m_env;
        jobject m_ref;
    };

    // ===== Error Handling =====

    /**
     * @brief Throw a Java exception from C++ code
     * @param env JNI environment pointer
     * @param exceptionClass Exception class name
     * @param message Exception message
     * @return 0 if successful, negative on failure
     *
     * Commonly used exception classes:
     * - "java/lang/Exception"
     * - "java/lang/IllegalArgumentException"
     * - "java/lang/IllegalStateException"
     * - "java/lang/RuntimeException"
     * - "java/io/IOException"
     */
    static int throw_java_exception(JNIEnv* env, const std::string& exceptionClass,
                                    const std::string& message);

    /**
     * @brief Check if a Java exception is pending
     * @param env JNI environment pointer
     * @return true if exception pending
     */
    static bool has_exception(JNIEnv* env);

    /**
     * @brief Get description of pending exception
     * @param env JNI environment pointer
     * @return Exception message
     */
    static std::string get_exception_message(JNIEnv* env);

    /**
     * @brief Clear pending exception
     * @param env JNI environment pointer
     */
    static void clear_exception(JNIEnv* env) noexcept;

    /**
     * @brief Print exception details to logcat
     * @param env JNI environment pointer
     */
    static void print_exception(JNIEnv* env);

    // ===== Utility =====

    /**
     * @brief Get JNI version
     * @param env JNI environment pointer
     * @return JNI version number
     */
    static int get_jni_version(JNIEnv* env);

    /**
     * @brief Check if we're on the main thread
     * @return true if on main/UI thread
     */
    static bool is_main_thread();

    /**
     * @brief Get a descriptor string from a value type
     * @param type Type indicator
     * @return JNI signature character
     */
    static char get_type_descriptor(const std::string& type);

private:
    // Private constructor - this is a utility class
    JNIBridge() = delete;
};

} // namespace jni
} // namespace clipforge

#endif // CLIPFORGE_JNI_BRIDGE_H
