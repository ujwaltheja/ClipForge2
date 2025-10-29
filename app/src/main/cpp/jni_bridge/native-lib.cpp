#include <jni.h>
#include "jni_bridge.h"
#include "../core/video_engine.h"
#include "../utils/logger.h"
#include <memory>
#include <map>
#include <mutex>

#define JNI_POSSIBLE_UNUSED(x) (void)(x)

using namespace clipforge;
using namespace clipforge::jni;
using namespace clipforge::core;
using namespace clipforge::utils;

// ============================================================================
// Global Engine Management
// ============================================================================

static std::map<jlong, std::shared_ptr<VideoEngine>> g_engineMap;
static std::mutex g_engineMutex;

/**
 * @brief Get engine by pointer handle
 * @param enginePtr Pointer as jlong
 * @return Shared pointer to engine or nullptr
 */
static std::shared_ptr<VideoEngine> getEngine(jlong enginePtr) {
    std::lock_guard<std::mutex> lock(g_engineMutex);
    auto it = g_engineMap.find(enginePtr);
    if (it != g_engineMap.end()) {
        return it->second;
    }
    return nullptr;
}

/**
 * @brief Store engine and return pointer
 * @param engine Engine to store
 * @return Pointer as jlong
 */
static jlong storeEngine(const std::shared_ptr<VideoEngine>& engine) {
    std::lock_guard<std::mutex> lock(g_engineMutex);
    auto ptr = reinterpret_cast<jlong>(engine.get());
    g_engineMap[ptr] = engine;
    return ptr;
}

/**
 * @brief Remove engine from map
 * @param enginePtr Pointer as jlong
 */
static void removeEngine(jlong enginePtr) {
    std::lock_guard<std::mutex> lock(g_engineMutex);
    g_engineMap.erase(enginePtr);
}

// ============================================================================
// JNI_OnLoad / JNI_OnUnload
// ============================================================================

extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNI_POSSIBLE_UNUSED(vm);
    JNI_POSSIBLE_UNUSED(reserved);
    LOG_INFO("ClipForge NDK JNI_OnLoad called");

    // Initialize logger
    Logger::getInstance().initialize("", LogLevel::DEBUG, true);

    LOG_INFO("ClipForge NDK Library Loaded");
    LOG_INFO("Version: %s", VideoEngine::getVersion().c_str());

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    JNI_POSSIBLE_UNUSED(vm);
    JNI_POSSIBLE_UNUSED(reserved);
    LOG_INFO("ClipForge NDK JNI_OnUnload called");

    // Cleanup all engines
    {
        std::lock_guard<std::mutex> lock(g_engineMutex);
        g_engineMap.clear();
    }

    // Shutdown logger
    Logger::getInstance().shutdown();

    LOG_INFO("ClipForge NDK Library Unloaded");
}

// ============================================================================
// Engine Lifecycle Management
// ============================================================================

/**
 * @brief Create a new video engine instance
 *
 * Java Signature: native long createEngine()
 */
JNIEXPORT jlong JNICALL
Java_com_ucworks_clipforge_NativeLib_createEngine(JNIEnv* env, jclass clazz) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = std::make_shared<VideoEngine>();
        LOG_INFO("VideoEngine created");
        return storeEngine(engine);
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to create engine: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException",
                                       "Failed to create video engine");
        return 0;
    }
}

/**
 * @brief Initialize engine with default configuration
 *
 * Java Signature: native boolean initEngine(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_initEngine(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            JNIBridge::throw_java_exception(env, "java/lang/IllegalArgumentException",
                                           "Invalid engine pointer");
            return JNI_FALSE;
        }

        EngineConfig config;
        config.previewQuality = 720;
        config.useHardwareAccel = true;
        config.maxRenderThreads = 4;

        bool result = engine->initialize(config);
        if (!result) {
            JNIBridge::throw_java_exception(env, "java/lang/RuntimeException",
                                           "Failed to initialize engine");
        }
        return result ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error initializing engine: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Destroy engine and free resources
 *
 * Java Signature: native void destroyEngine(long enginePtr)
 */
JNIEXPORT void JNICALL
Java_com_ucworks_clipforge_NativeLib_destroyEngine(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (engine) {
            engine->shutdown();
            removeEngine(enginePtr);
            LOG_INFO("VideoEngine destroyed");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Error destroying engine: %s", e.what());
    }
}

/**
 * @brief Get engine version string
 *
 * Java Signature: native String getEngineVersion(long enginePtr)
 */
JNIEXPORT jstring JNICALL
Java_com_ucworks_clipforge_NativeLib_getEngineVersion(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    JNI_POSSIBLE_UNUSED(enginePtr);
    try {
        return JNIBridge::string_to_jstring(env, VideoEngine::getVersion());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting version: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return nullptr;
    }
}

// ============================================================================
// Clip Management
// ============================================================================

/**
 * @brief Add a video clip to the timeline
 *
 * Java Signature: native String addClip(long enginePtr, String sourcePath,
 *                                        long startPosition, int trackIndex)
 */
JNIEXPORT jstring JNICALL
Java_com_ucworks_clipforge_NativeLib_addClip(JNIEnv* env, jclass clazz, jlong enginePtr,
                                            jstring sourcePath, jlong startPosition,
                                            jint trackIndex) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            JNIBridge::throw_java_exception(env, "java/lang/IllegalArgumentException",
                                           "Invalid engine pointer");
            return nullptr;
        }

        std::string path = JNIBridge::jstring_to_string(env, sourcePath);
        std::string clipId = engine->addClip(path, startPosition, trackIndex);

        if (clipId.empty()) {
            JNIBridge::throw_java_exception(env, "java/lang/RuntimeException",
                                           engine->getErrorMessage());
            return nullptr;
        }

        return JNIBridge::string_to_jstring(env, clipId);
    } catch (const std::exception& e) {
        LOG_ERROR("Error adding clip: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return nullptr;
    }
}

/**
 * @brief Remove a clip from the timeline
 *
 * Java Signature: native boolean removeClip(long enginePtr, String clipId)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_removeClip(JNIEnv* env, jclass clazz, jlong enginePtr,
                                               jstring clipId) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            JNIBridge::throw_java_exception(env, "java/lang/IllegalArgumentException",
                                           "Invalid engine pointer");
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        bool result = engine->removeClip(id);

        if (!result) {
            JNIBridge::throw_java_exception(env, "java/lang/RuntimeException",
                                           engine->getErrorMessage());
        }

        return result ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error removing clip: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get total timeline duration
 *
 * Java Signature: native long getTimelineDuration(long enginePtr)
 */
JNIEXPORT jlong JNICALL
Java_com_ucworks_clipforge_NativeLib_getTimelineDuration(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return 0;
        }
        return engine->getTimelineDuration();
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting duration: %s", e.what());
        return 0;
    }
}

/**
 * @brief Get number of clips in timeline
 *
 * Java Signature: native int getClipCount(long enginePtr)
 */
JNIEXPORT jint JNICALL
Java_com_ucworks_clipforge_NativeLib_getClipCount(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return 0;
        }
        return static_cast<jint>(engine->getClipCount());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting clip count: %s", e.what());
        return 0;
    }
}

/**
 * @brief Get total effects applied
 *
 * Java Signature: native int getEffectCount(long enginePtr)
 */
JNIEXPORT jint JNICALL
Java_com_ucworks_clipforge_NativeLib_getEffectCount(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return 0;
        }
        return static_cast<jint>(engine->getEffectCount());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting effect count: %s", e.what());
        return 0;
    }
}

// ============================================================================
// Preview Control
// ============================================================================

/**
 * @brief Start preview playback
 *
 * Java Signature: native boolean startPreview(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_startPreview(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->startPreview() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error starting preview: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Stop preview playback
 *
 * Java Signature: native boolean stopPreview(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_stopPreview(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->stopPreview() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error stopping preview: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get current preview position
 *
 * Java Signature: native long getPreviewPosition(long enginePtr)
 */
JNIEXPORT jlong JNICALL
Java_com_ucworks_clipforge_NativeLib_getPreviewPosition(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return 0;
        }
        return engine->getPreviewPosition();
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting preview position: %s", e.what());
        return 0;
    }
}

// ============================================================================
// Export/Rendering
// ============================================================================

/**
 * @brief Start exporting the project
 *
 * Java Signature: native boolean startExport(long enginePtr, String outputPath,
 *                                           String format, String quality)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_startExport(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                jstring outputPath, jstring format,
                                                jstring quality) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string outPath = JNIBridge::jstring_to_string(env, outputPath);
        std::string fmt = JNIBridge::jstring_to_string(env, format);
        std::string qual = JNIBridge::jstring_to_string(env, quality);

        return engine->startExport(outPath, fmt, qual) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error starting export: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Check if currently exporting
 *
 * Java Signature: native boolean isExporting(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_isExporting(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->isExporting() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error checking export status: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get export progress (0-100)
 *
 * Java Signature: native float getExportProgress(long enginePtr)
 */
JNIEXPORT jstring JNICALL
Java_com_ucworks_clipforge_NativeLib_getExportProgress(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNIBridge::string_to_jstring(env, "{\"error\": \"Engine not found\"}");
        }

        auto progress = engine->getExportProgress();
        char buffer[256];
        snprintf(buffer, sizeof(buffer),
            "{\"currentFrame\": %lld, \"totalFrames\": %lld, \"percentage\": %.1f, \"status\": \"%s\"}",
            progress.currentFrame, progress.totalFrames, progress.percentage, progress.status.c_str());

        return JNIBridge::string_to_jstring(env, buffer);
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting export progress: %s", e.what());
        return JNIBridge::string_to_jstring(env, "{\"error\": \"Failed to get progress\"}");
    }
}

// ============================================================================
// Additional Methods
// ============================================================================

/**
 * @brief Create a new project
 *
 * Java Signature: native String createProject(long enginePtr, String projectName,
 *                                             int width, int height, int fps)
 */
JNIEXPORT jstring JNICALL
Java_com_ucworks_clipforge_NativeLib_createProject(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                  jstring projectName, jint width, jint height, jint fps) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            JNIBridge::throw_java_exception(env, "java/lang/IllegalArgumentException",
                                           "Invalid engine pointer");
            return nullptr;
        }

        std::string name = JNIBridge::jstring_to_string(env, projectName);
        auto timeline = std::make_shared<models::Timeline>(width, height, static_cast<float>(fps));

        if (!engine->setTimeline(timeline)) {
            JNIBridge::throw_java_exception(env, "java/lang/RuntimeException",
                                           engine->getErrorMessage());
            return nullptr;
        }

        std::string projectId = std::to_string(enginePtr) + "_" + name;
        return JNIBridge::string_to_jstring(env, projectId);
    } catch (const std::exception& e) {
        LOG_ERROR("Error creating project: %s", e.what());
        JNIBridge::throw_java_exception(env, "java/lang/RuntimeException", e.what());
        return nullptr;
    }
}

/**
 * @brief Load project from file
 *
 * Java Signature: native boolean loadProject(long enginePtr, String projectPath)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_loadProject(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                jstring projectPath) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string path = JNIBridge::jstring_to_string(env, projectPath);
        return engine->loadProject(path) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error loading project: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Save project to file
 *
 * Java Signature: native boolean saveProject(long enginePtr, String projectPath)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_saveProject(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                jstring projectPath) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string path = JNIBridge::jstring_to_string(env, projectPath);
        return engine->saveProject(path) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error saving project: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get recent projects
 *
 * Java Signature: native String[] getRecentProjects(long enginePtr)
 */
JNIEXPORT jobjectArray JNICALL
Java_com_ucworks_clipforge_NativeLib_getRecentProjects(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
        }

        // TODO: Implement recent projects retrieval from file system
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting recent projects: %s", e.what());
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    }
}

/**
 * @brief Get all clips
 *
 * Java Signature: native String[] getClips(long enginePtr)
 */
JNIEXPORT jobjectArray JNICALL
Java_com_ucworks_clipforge_NativeLib_getClips(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
        }

        // TODO: Implement clips JSON serialization
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting clips: %s", e.what());
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    }
}

/**
 * @brief Trim a clip
 *
 * Java Signature: native boolean trimClip(long enginePtr, String clipId, long trimStart, long trimEnd)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_trimClip(JNIEnv* env, jclass clazz, jlong enginePtr,
                                             jstring clipId, jlong trimStart, jlong trimEnd) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        return engine->trimClip(id, trimStart, trimEnd) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error trimming clip: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Set clip speed
 *
 * Java Signature: native boolean setClipSpeed(long enginePtr, String clipId, float speed)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_setClipSpeed(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                 jstring clipId, jfloat speed) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        return engine->setClipSpeed(id, speed) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error setting clip speed: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Set clip volume
 *
 * Java Signature: native boolean setClipVolume(long enginePtr, String clipId, float volume)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_setClipVolume(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                  jstring clipId, jfloat volume) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        return engine->setClipVolume(id, volume) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error setting clip volume: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get available effects
 *
 * Java Signature: native String[] getAvailableEffects(long enginePtr)
 */
JNIEXPORT jobjectArray JNICALL
Java_com_ucworks_clipforge_NativeLib_getAvailableEffects(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
        }

        auto effects = engine->getAvailableEffects();
        jobjectArray jArray = env->NewObjectArray(effects.size(), env->FindClass("java/lang/String"), nullptr);

        for (size_t i = 0; i < effects.size(); ++i) {
            jstring jEffect = env->NewStringUTF(effects[i]->getName().c_str());
            env->SetObjectArrayElement(jArray, i, jEffect);
            env->DeleteLocalRef(jEffect);
        }

        return jArray;
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting effects: %s", e.what());
        return env->NewObjectArray(0, env->FindClass("java/lang/String"), nullptr);
    }
}

/**
 * @brief Apply effect to clip
 *
 * Java Signature: native boolean applyEffect(long enginePtr, String clipId, String effectName, float intensity)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_applyEffect(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                jstring clipId, jstring effectName, jfloat intensity) {
    JNI_POSSIBLE_UNUSED(clazz);
    JNI_POSSIBLE_UNUSED(intensity);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        std::string name = JNIBridge::jstring_to_string(env, effectName);

        auto effect = engine->createEffect(name);
        if (!effect) {
            return JNI_FALSE;
        }

        return engine->applyEffect(id, effect) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error applying effect: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Remove effect from clip
 *
 * Java Signature: native boolean removeEffect(long enginePtr, String clipId, String effectId)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_removeEffect(JNIEnv* env, jclass clazz, jlong enginePtr,
                                                 jstring clipId, jstring effectId) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }

        std::string id = JNIBridge::jstring_to_string(env, clipId);
        std::string effectIdx = JNIBridge::jstring_to_string(env, effectId);

        return engine->removeEffect(id, effectIdx) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error removing effect: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Pause preview
 *
 * Java Signature: native boolean pausePreview(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_pausePreview(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->pausePreview() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error pausing preview: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Seek preview
 *
 * Java Signature: native boolean seekPreview(long enginePtr, long timeMs)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_seekPreview(JNIEnv* env, jclass clazz, jlong enginePtr, jlong timeMs) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->seekPreview(timeMs) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error seeking preview: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Cancel export
 *
 * Java Signature: native boolean cancelExport(long enginePtr)
 */
JNIEXPORT jboolean JNICALL
Java_com_ucworks_clipforge_NativeLib_cancelExport(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return JNI_FALSE;
        }
        return engine->cancelExport() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("Error cancelling export: %s", e.what());
        return JNI_FALSE;
    }
}

/**
 * @brief Get memory usage
 *
 * Java Signature: native long getMemoryUsage(long enginePtr)
 */
JNIEXPORT jlong JNICALL
Java_com_ucworks_clipforge_NativeLib_getMemoryUsage(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return 0;
        }
        return static_cast<jlong>(engine->getMemoryUsage());
    } catch (const std::exception& e) {
        LOG_ERROR("Error getting memory usage: %s", e.what());
        return 0;
    }
}

/**
 * @brief Get error message
 *
 * Java Signature: native String getErrorMessage(long enginePtr)
 */
JNIEXPORT jstring JNICALL
Java_com_ucworks_clipforge_NativeLib_getErrorMessage(JNIEnv* env, jclass clazz, jlong enginePtr) {
    JNI_POSSIBLE_UNUSED(clazz);
    try {
        auto engine = getEngine(enginePtr);
        if (!engine) {
            return env->NewStringUTF("Engine not found");
        }
        return env->NewStringUTF(engine->getErrorMessage().c_str());
    } catch (const std::exception& e) {
        return env->NewStringUTF(e.what());
    }
}

} // extern "C"
