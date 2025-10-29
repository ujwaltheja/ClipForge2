#ifndef CLIPFORGE_GPU_BRIDGE_H
#define CLIPFORGE_GPU_BRIDGE_H

#include <jni.h>
#include <memory>
#include <unordered_map>
#include "../gpu/gpu_renderer.h"

namespace clipforge {
namespace jni {

/**
 * @class GPUBridge
 * @brief JNI bridge for GPU renderer operations
 *
 * Provides JNI entry points for Android layer to control GPU rendering,
 * apply effects, and monitor performance from Kotlin.
 *
 * Thread Safety:
 * - GPU operations marshaled to render thread
 * - Effect parameters are atomic
 * - Statistics are lock-free reads
 */
class GPUBridge {
public:
    /**
     * @brief Initialize GPU bridge
     */
    static void initialize();

    /**
     * @brief Shutdown GPU bridge
     */
    static void shutdown();

    // ===== Renderer Management =====

    /**
     * @brief Create GPU renderer instance
     * @param enginePtr Pointer to video engine
     * @param width Render width
     * @param height Render height
     * @return Renderer pointer (long)
     */
    static jlong createRenderer(jlong enginePtr, jint width, jint height);

    /**
     * @brief Destroy GPU renderer
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     */
    static void destroyRenderer(jlong enginePtr, jlong rendererPtr);

    /**
     * @brief Resize renderer output
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param width New width
     * @param height New height
     */
    static void resizeRenderer(jlong enginePtr, jlong rendererPtr, jint width, jint height);

    // ===== Effect Application =====

    /**
     * @brief Apply GPU effect to renderer
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param effectName Effect name (C string)
     * @return true if applied
     */
    static jboolean applyEffect(jlong enginePtr, jlong rendererPtr, const char* effectName);

    /**
     * @brief Remove GPU effect
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param effectName Effect name
     * @return true if removed
     */
    static jboolean removeEffect(jlong enginePtr, jlong rendererPtr, const char* effectName);

    /**
     * @brief Set effect parameter
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param effectName Effect name
     * @param paramName Parameter name
     * @param value Parameter value
     * @return true if set
     */
    static jboolean setEffectParameter(jlong enginePtr, jlong rendererPtr,
                                      const char* effectName, const char* paramName,
                                      jfloat value);

    /**
     * @brief Clear all effects
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     */
    static void clearEffects(jlong enginePtr, jlong rendererPtr);

    // ===== Rendering =====

    /**
     * @brief Render single frame through GPU pipeline
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param timeMs Current time in milliseconds
     * @return Output texture ID
     */
    static jint renderFrame(jlong enginePtr, jlong rendererPtr, jlong timeMs);

    /**
     * @brief Render to texture for offscreen processing
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param inputTexture Input texture ID
     * @return Output texture ID
     */
    static jint renderToTexture(jlong enginePtr, jlong rendererPtr, jint inputTexture);

    // ===== Info Queries =====

    /**
     * @brief Get available effects
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @return Array of effect names
     */
    static jobjectArray getAvailableEffects(JNIEnv* env, jlong enginePtr,
                                           jlong rendererPtr);

    /**
     * @brief Get effect parameters
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @param effectName Effect name
     * @return Array of parameter info
     */
    static jobjectArray getEffectParameters(JNIEnv* env, jlong enginePtr,
                                           jlong rendererPtr, const char* effectName);

    /**
     * @brief Get rendering statistics
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @return Statistics as Java object
     */
    static jobject getStatistics(JNIEnv* env, jlong enginePtr, jlong rendererPtr);

    /**
     * @brief Get GPU information
     * @param enginePtr Engine pointer
     * @param rendererPtr Renderer pointer
     * @return GPU info string
     */
    static jstring getGPUInfo(JNIEnv* env, jlong enginePtr, jlong rendererPtr);

private:
    // Internal renderer storage
    static std::unordered_map<jlong, std::shared_ptr<gpu::GPURenderer>> s_renderers;

    /**
     * @brief Get renderer from pointer
     * @param rendererPtr Renderer pointer
     * @return Shared pointer, nullptr if not found
     */
    static std::shared_ptr<gpu::GPURenderer> getRenderer(jlong rendererPtr);

    /**
     * @brief Store renderer instance
     * @param renderer Renderer shared pointer
     * @return Pointer value
     */
    static jlong storeRenderer(std::shared_ptr<gpu::GPURenderer> renderer);
};

} // namespace jni
} // namespace clipforge

#endif // CLIPFORGE_GPU_BRIDGE_H
