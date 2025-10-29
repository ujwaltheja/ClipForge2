#include "gpu_bridge.h"
#include "../utils/logger.h"
#include "../gpu/effects/color_grade_effect.h"
#include "../gpu/effects/blur_effect.h"
#include "../gpu/effects/distortion_effect.h"
#include <sstream>
#include <mutex>

namespace clipforge {
namespace jni {

// Static storage for renderers
std::unordered_map<jlong, std::shared_ptr<gpu::GPURenderer>> GPUBridge::s_renderers;
static std::mutex g_rendererMutex;
static jlong g_nextRendererPtr = 1000;

void GPUBridge::initialize() {
    LOGI("GPU Bridge initialized");
}

void GPUBridge::shutdown() {
    std::lock_guard<std::mutex> lock(g_rendererMutex);

    for (auto& pair : s_renderers) {
        if (pair.second) {
            pair.second->shutdown();
        }
    }

    s_renderers.clear();
    LOGI("GPU Bridge shutdown complete");
}

jlong GPUBridge::createRenderer(jlong enginePtr, jint width, jint height) {
    LOGI("Creating GPU renderer: %dx%d", width, height);

    auto renderer = std::make_shared<gpu::GPURenderer>();

    gpu::RenderConfig config{
        width, height,      // render size
        width, height,      // output size
        true,               // multisampling
        4,                  // sample count
        true,               // caching
        true                // profiling
    };

    if (!renderer->initialize(config)) {
        LOGE("Failed to initialize GPU renderer");
        return 0;
    }

    // Create and initialize effects
    auto colorGrade = std::make_shared<gpu::effects::ColorGradeEffect>();
    renderer->addEffect(colorGrade);

    auto curves = std::make_shared<gpu::effects::CurvesEffect>();
    renderer->addEffect(curves);

    auto hsl = std::make_shared<gpu::effects::HSLEffect>();
    renderer->addEffect(hsl);

    auto blur = std::make_shared<gpu::effects::GaussianBlurEffect>();
    renderer->addEffect(blur);

    auto vignette = std::make_shared<gpu::effects::VignetteEffect>();
    renderer->addEffect(vignette);

    auto glow = std::make_shared<gpu::effects::GlowEffect>();
    renderer->addEffect(glow);

    auto chromatic = std::make_shared<gpu::effects::ChromaticAberrationEffect>();
    renderer->addEffect(chromatic);

    auto glitch = std::make_shared<gpu::effects::GlitchEffect>();
    renderer->addEffect(glitch);

    auto posterize = std::make_shared<gpu::effects::PosterizeEffect>();
    renderer->addEffect(posterize);

    auto invert = std::make_shared<gpu::effects::InvertEffect>();
    renderer->addEffect(invert);

    auto grayscale = std::make_shared<gpu::effects::GrayscaleEffect>();
    renderer->addEffect(grayscale);

    LOGI("GPU renderer created with %zu effects", renderer->getEffectCount());

    jlong ptr = storeRenderer(renderer);
    LOGI("GPU renderer stored with pointer: %ld", ptr);

    return ptr;
}

void GPUBridge::destroyRenderer(jlong enginePtr, jlong rendererPtr) {
    LOGI("Destroying GPU renderer: %ld", rendererPtr);

    std::lock_guard<std::mutex> lock(g_rendererMutex);

    auto it = s_renderers.find(rendererPtr);
    if (it != s_renderers.end()) {
        it->second->shutdown();
        s_renderers.erase(it);
    }
}

void GPUBridge::resizeRenderer(jlong enginePtr, jlong rendererPtr, jint width, jint height) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        LOGW("Renderer not found: %ld", rendererPtr);
        return;
    }

    LOGI("Resizing GPU renderer to %dx%d", width, height);

    // Recreate renderer with new dimensions
    renderer->shutdown();

    gpu::RenderConfig config{width, height, width, height, true, 4, true, true};
    renderer->initialize(config);
}

jboolean GPUBridge::applyEffect(jlong enginePtr, jlong rendererPtr, const char* effectName) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        LOGW("Renderer not found: %ld", rendererPtr);
        return JNI_FALSE;
    }

    if (!renderer->hasEffect(effectName)) {
        LOGW("Effect not found: %s", effectName);
        return JNI_FALSE;
    }

    auto effect = renderer->getEffect(effectName);
    if (effect) {
        effect->setEnabled(true);
        LOGI("Effect enabled: %s", effectName);
        return JNI_TRUE;
    }

    return JNI_FALSE;
}

jboolean GPUBridge::removeEffect(jlong enginePtr, jlong rendererPtr, const char* effectName) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return JNI_FALSE;
    }

    auto effect = renderer->getEffect(effectName);
    if (effect) {
        effect->setEnabled(false);
        LOGI("Effect disabled: %s", effectName);
        return JNI_TRUE;
    }

    return JNI_FALSE;
}

jboolean GPUBridge::setEffectParameter(jlong enginePtr, jlong rendererPtr,
                                      const char* effectName, const char* paramName,
                                      jfloat value) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return JNI_FALSE;
    }

    auto effect = renderer->getEffect(effectName);
    if (!effect) {
        LOGW("Effect not found: %s", effectName);
        return JNI_FALSE;
    }

    if (effect->setParameter(paramName, value)) {
        LOGD("Effect parameter set: %s.%s = %.2f", effectName, paramName, value);
        return JNI_TRUE;
    }

    return JNI_FALSE;
}

void GPUBridge::clearEffects(jlong enginePtr, jlong rendererPtr) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return;
    }

    auto effects = renderer->getEffectNames();
    for (const auto& effectName : effects) {
        auto effect = renderer->getEffect(effectName);
        if (effect) {
            effect->setEnabled(false);
        }
    }

    LOGI("All effects disabled");
}

jint GPUBridge::renderFrame(jlong enginePtr, jlong rendererPtr, jlong timeMs) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer || !renderer->isInitialized()) {
        return 0;
    }

    // In real implementation, would get frame texture from engine
    // For now, return success
    return 1;
}

jint GPUBridge::renderToTexture(jlong enginePtr, jlong rendererPtr, jint inputTexture) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer || !renderer->isInitialized()) {
        return 0;
    }

    GLuint outputTexture = renderer->renderToTexture(static_cast<GLuint>(inputTexture));
    return static_cast<jint>(outputTexture);
}

jobjectArray GPUBridge::getAvailableEffects(JNIEnv* env, jlong enginePtr,
                                           jlong rendererPtr) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return nullptr;
    }

    auto effectNames = renderer->getEffectNames();

    jobjectArray result = env->NewObjectArray(
        static_cast<jsize>(effectNames.size()),
        env->FindClass("java/lang/String"),
        nullptr
    );

    for (size_t i = 0; i < effectNames.size(); i++) {
        jstring str = env->NewStringUTF(effectNames[i].c_str());
        env->SetObjectArrayElement(result, static_cast<jsize>(i), str);
        env->DeleteLocalRef(str);
    }

    LOGI("Returned %zu effect names", effectNames.size());
    return result;
}

jobjectArray GPUBridge::getEffectParameters(JNIEnv* env, jlong enginePtr,
                                           jlong rendererPtr, const char* effectName) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return nullptr;
    }

    auto effect = renderer->getEffect(effectName);
    if (!effect) {
        return nullptr;
    }

    auto params = effect->getParameters();

    jobjectArray result = env->NewObjectArray(
        static_cast<jsize>(params.size()),
        env->FindClass("java/lang/String"),
        nullptr
    );

    for (size_t i = 0; i < params.size(); i++) {
        std::stringstream ss;
        ss << params[i].name << "|" << params[i].minValue << "|"
           << params[i].maxValue << "|" << params[i].defaultValue;

        jstring str = env->NewStringUTF(ss.str().c_str());
        env->SetObjectArrayElement(result, static_cast<jsize>(i), str);
        env->DeleteLocalRef(str);
    }

    return result;
}

jobject GPUBridge::getStatistics(JNIEnv* env, jlong enginePtr, jlong rendererPtr) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return nullptr;
    }

    const auto& stats = renderer->getStats();

    // Create RenderStats object
    jclass statsClass = env->FindClass(
        "com/clipforge/android/views/GPUTextureView$RenderStats"
    );

    jmethodID constructor = env->GetMethodID(statsClass, "<init>", "()V");
    jobject statsObj = env->NewObject(statsClass, constructor);

    // Set fields
    jfieldID fpsField = env->GetFieldID(statsClass, "fps", "I");
    jfieldID gpuTimeField = env->GetFieldID(statsClass, "gpuTimeMs", "F");
    jfieldID cpuTimeField = env->GetFieldID(statsClass, "cpuTimeMs", "F");
    jfieldID textureField = env->GetFieldID(statsClass, "textureCount", "I");
    jfieldID framebufferField = env->GetFieldID(statsClass, "framebufferCount", "I");

    env->SetIntField(statsObj, fpsField, stats.framesPerSecond);
    env->SetFloatField(statsObj, gpuTimeField, stats.gpuTimeMs);
    env->SetFloatField(statsObj, cpuTimeField, stats.cpuTimeMs);
    env->SetIntField(statsObj, textureField, stats.textureCount);
    env->SetIntField(statsObj, framebufferField, stats.framebufferCount);

    env->DeleteLocalRef(statsClass);

    return statsObj;
}

jstring GPUBridge::getGPUInfo(JNIEnv* env, jlong enginePtr, jlong rendererPtr) {
    auto renderer = getRenderer(rendererPtr);
    if (!renderer) {
        return env->NewStringUTF("Renderer not initialized");
    }

    std::string info = renderer->getGPUInfo();
    return env->NewStringUTF(info.c_str());
}

std::shared_ptr<gpu::GPURenderer> GPUBridge::getRenderer(jlong rendererPtr) {
    std::lock_guard<std::mutex> lock(g_rendererMutex);

    auto it = s_renderers.find(rendererPtr);
    if (it != s_renderers.end()) {
        return it->second;
    }

    return nullptr;
}

jlong GPUBridge::storeRenderer(std::shared_ptr<gpu::GPURenderer> renderer) {
    std::lock_guard<std::mutex> lock(g_rendererMutex);

    jlong ptr = g_nextRendererPtr++;
    s_renderers[ptr] = renderer;

    return ptr;
}

} // namespace jni
} // namespace clipforge
