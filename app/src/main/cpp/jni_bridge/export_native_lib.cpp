#include <jni.h>
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include "../encoding/export_manager.h"
#include "../encoding/video_encoder.h"
#include "../utils/logger.h"

#define JNI_POSSIBLE_UNUSED(x) (void)(x)

using namespace clipforge::encoding;

// ===== Global State =====

static std::mutex g_managerMutex;
static std::map<jlong, std::shared_ptr<ExportManager>> g_managers;
static jlong g_nextManagerId = 1;

// ===== Helper Functions =====

std::shared_ptr<ExportManager> getManager(jlong ptr) {
    std::lock_guard<std::mutex> lock(g_managerMutex);
    auto it = g_managers.find(ptr);
    if (it != g_managers.end()) {
        return it->second;
    }
    return nullptr;
}

void storeManager(jlong ptr, std::shared_ptr<ExportManager> manager) {
    std::lock_guard<std::mutex> lock(g_managerMutex);
    g_managers[ptr] = manager;
}

void removeManager(jlong ptr) {
    std::lock_guard<std::mutex> lock(g_managerMutex);
    g_managers.erase(ptr);
}

std::string jstringToString(JNIEnv* env, jstring str) {
    if (!str) return "";
    const char* chars = env->GetStringUTFChars(str, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(str, chars);
    return result;
}

jstring stringToJstring(JNIEnv* env, const std::string& str) {
    return env->NewStringUTF(str.c_str());
}

QualityPreset stringToQualityPreset(const std::string& str) {
    if (str == "LOW") return QualityPreset::LOW;
    if (str == "MEDIUM") return QualityPreset::MEDIUM;
    if (str == "HIGH") return QualityPreset::HIGH;
    if (str == "ULTRA") return QualityPreset::ULTRA;
    return QualityPreset::HIGH;
}

ExportFormat stringToExportFormat(const std::string& str) {
    if (str == "WEBM") return ExportFormat::WEBM;
    if (str == "MKV") return ExportFormat::MKV;
    return ExportFormat::MP4;
}

VideoCodec stringToVideoCodec(const std::string& str) {
    if (str == "H265") return VideoCodec::H265;
    if (str == "VP9") return VideoCodec::VP9;
    return VideoCodec::H264;
}

// ===== JNI Functions =====

extern "C" {

// ===== Export Manager Creation/Destruction =====

JNIEXPORT jlong JNICALL Java_com_clipforge_android_engine_ExportNativeLib_createExportManager(
    JNIEnv* env, jobject /* this */, jstring outputPath) {
    JNI_POSSIBLE_UNUSED(env);
    JNI_POSSIBLE_UNUSED(outputPath);
    try {
        auto manager = std::make_shared<ExportManager>();
        jlong id = g_nextManagerId++;
        storeManager(id, manager);
        LOG_DEBUG("JNI: ExportManager created with ID %lld", id);
        return id;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to create ExportManager: %s", e.what());
        return 0;
    }
}

JNIEXPORT void JNICALL Java_com_clipforge_android_engine_ExportNativeLib_destroyExportManager(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (manager && manager->isExporting()) {
            manager->cancelExport();
        }
        removeManager(managerPtr);
        LOG_DEBUG("JNI: ExportManager destroyed (ID: %lld)", managerPtr);
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to destroy ExportManager: %s", e.what());
    }
}

// ===== Configuration =====

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_configureExport(
    JNIEnv* env, jobject /* this */, jlong managerPtr, jint width, jint height,
    jint frameRate, jstring quality, jstring format) {
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            LOG_ERROR("JNI: Invalid manager pointer");
            return JNI_FALSE;
        }

        ExportConfig config;
        config.width = width;
        config.height = height;
        config.frameRate = frameRate;
        config.quality = stringToQualityPreset(jstringToString(env, quality));
        config.format = stringToExportFormat(jstringToString(env, format));

        bool result = manager->setConfig(config);
        LOG_DEBUG("JNI: Export configured: %dx%d @ %d fps", width, height, frameRate);
        return result ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to configure export: %s", e.what());
        return JNI_FALSE;
    }
}

// ===== Export Control =====

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_startExport(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            LOG_ERROR("JNI: Invalid manager pointer");
            return JNI_FALSE;
        }

        bool result = manager->startExport();
        LOG_DEBUG("JNI: Export started (result: %d)", result);
        return result ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to start export: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_cancelExport(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            LOG_ERROR("JNI: Invalid manager pointer");
            return JNI_FALSE;
        }

        bool result = manager->cancelExport();
        LOG_DEBUG("JNI: Export cancelled (result: %d)", result);
        return result ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to cancel export: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_pauseExport(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return JNI_FALSE;
        }
        // Note: Pause functionality would need to be implemented in ExportManager
        LOG_DEBUG("JNI: Export pause requested");
        return JNI_TRUE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to pause export: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_resumeExport(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return JNI_FALSE;
        }
        // Note: Resume functionality would need to be implemented in ExportManager
        LOG_DEBUG("JNI: Export resume requested");
        return JNI_TRUE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to resume export: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_isExporting(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return JNI_FALSE;
        }
        return manager->isExporting() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to check export status: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_isExportComplete(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return JNI_FALSE;
        }
        return manager->isComplete() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to check completion: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_isExportCancelled(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return JNI_FALSE;
        }
        return manager->wasCancelled() ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to check cancellation: %s", e.what());
        return JNI_FALSE;
    }
}

// ===== Progress Monitoring =====

JNIEXPORT jobject JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportProgress(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return nullptr;
        }

        const auto& progress = manager->getProgress();

        // Find the ExportProgress class
        jclass progressClass = env->FindClass("com/clipforge/android/engine/ExportNativeLib$ExportProgress");
        if (!progressClass) {
            LOG_ERROR("JNI: Could not find ExportProgress class");
            return nullptr;
        }

        // Find the constructor
        jmethodID constructor = env->GetMethodID(progressClass, "<init>",
            "(FFFFJJJJFLjava/lang/String;Ljava/lang/String;)V");
        if (!constructor) {
            LOG_ERROR("JNI: Could not find ExportProgress constructor");
            env->DeleteLocalRef(progressClass);
            return nullptr;
        }

        jstring phaseStr = env->NewStringUTF(progress.currentPhase.c_str());
        jstring statusStr = env->NewStringUTF(progress.status.c_str());

        jobject result = env->NewObject(progressClass, constructor,
            progress.videoProgress,
            progress.audioProgress,
            progress.muxingProgress,
            progress.totalProgress,
            progress.framesEncoded,
            progress.totalFrames,
            progress.audioSamplesProcessed,
            progress.totalAudioSamples,
            progress.estimatedRemainingSeconds,
            phaseStr,
            statusStr);

        env->DeleteLocalRef(phaseStr);
        env->DeleteLocalRef(statusStr);
        env->DeleteLocalRef(progressClass);

        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get progress: %s", e.what());
        return nullptr;
    }
}

JNIEXPORT jstring JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportPhase(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return env->NewStringUTF("");
        }
        return env->NewStringUTF(manager->getCurrentPhase().c_str());
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get phase: %s", e.what());
        return env->NewStringUTF("");
    }
}

JNIEXPORT jstring JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportOutputPath(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return env->NewStringUTF("");
        }
        return env->NewStringUTF(manager->getConfig().outputPath.c_str());
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get output path: %s", e.what());
        return env->NewStringUTF("");
    }
}

JNIEXPORT jlong JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportFileSize(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return 0;
        }
        return manager->getFinalFileSize();
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get file size: %s", e.what());
        return 0;
    }
}

JNIEXPORT jstring JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportError(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return env->NewStringUTF("");
        }
        return env->NewStringUTF(manager->getErrorMessage().c_str());
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get error: %s", e.what());
        return env->NewStringUTF("");
    }
}

JNIEXPORT jfloat JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getExportEstimatedTimeRemaining(
    JNIEnv* env, jobject /* this */, jlong managerPtr) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        auto manager = getManager(managerPtr);
        if (!manager) {
            return 0.0f;
        }
        return manager->getEstimatedTimeRemaining();
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get time remaining: %s", e.what());
        return 0.0f;
    }
}

// ===== Video Encoder Functions =====

JNIEXPORT jint JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getRecommendedBitrate(
    JNIEnv* env, jobject /* this */, jint width, jint height, jint frameRate, jint quality) {
    JNI_POSSIBLE_UNUSED(env);
    try {
        return VideoEncoder::getRecommendedBitrate(width, height, frameRate, quality);
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get recommended bitrate: %s", e.what());
        return 0;
    }
}

JNIEXPORT jboolean JNICALL Java_com_clipforge_android_engine_ExportNativeLib_isCodecSupported(
    JNIEnv* env, jobject /* this */, jstring codec) {
    try {
        std::string codecStr = jstringToString(env, codec);
        VideoCodec vc = stringToVideoCodec(codecStr);
        return VideoEncoder::isCodecSupported(vc) ? JNI_TRUE : JNI_FALSE;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to check codec support: %s", e.what());
        return JNI_FALSE;
    }
}

JNIEXPORT jobject JNICALL Java_com_clipforge_android_engine_ExportNativeLib_getCodecBitrateRange(
    JNIEnv* env, jobject /* this */, jstring codec, jint width, jint height) {
    try {
        std::string codecStr = jstringToString(env, codec);
        VideoCodec vc = stringToVideoCodec(codecStr);
        auto [minBitrate, maxBitrate] = VideoEncoder::getBitrateRange(vc, width, height);

        // Find the BitrateRange class
        jclass rangeClass = env->FindClass("com/clipforge/android/engine/ExportNativeLib$BitrateRange");
        if (!rangeClass) {
            LOG_ERROR("JNI: Could not find BitrateRange class");
            return nullptr;
        }

        // Find the constructor
        jmethodID constructor = env->GetMethodID(rangeClass, "<init>", "(II)V");
        if (!constructor) {
            LOG_ERROR("JNI: Could not find BitrateRange constructor");
            env->DeleteLocalRef(rangeClass);
            return nullptr;
        }

        jobject result = env->NewObject(rangeClass, constructor, minBitrate, maxBitrate);
        env->DeleteLocalRef(rangeClass);

        return result;
    } catch (const std::exception& e) {
        LOG_ERROR("JNI: Failed to get bitrate range: %s", e.what());
        return nullptr;
    }
}

} // extern "C"


