#include "gpu_renderer.h"
#include "../utils/logger.h"
#include <chrono>
#include <sstream>
#include <algorithm>

namespace clipforge {
namespace gpu {

GPURenderer::GPURenderer() {
    LOGI("GPURenderer created");
}

GPURenderer::~GPURenderer() {
    shutdown();
}

bool GPURenderer::initialize(const RenderConfig& config) {
    LOGI("Initializing GPURenderer...");

    m_config = config;

    // Create and initialize OpenGL context
    m_context = std::make_unique<OpenGLContext>(config.renderWidth, config.renderHeight);

    if (!m_context->initialize()) {
        LOGE("Failed to initialize OpenGL context");
        return false;
    }

    LOGI("GPURenderer initialized successfully");
    LOGI("Render target: %dx%d", config.renderWidth, config.renderHeight);
    LOGI("Output size: %dx%d", config.outputWidth, config.outputHeight);

    return true;
}

void GPURenderer::shutdown() {
    LOGI("Shutting down GPURenderer...");

    // Clear effects
    clearEffects();

    // Release intermediate framebuffers
    for (GLuint fbo : m_intermediateFramebuffers) {
        m_context->deleteFramebuffer(fbo);
    }
    m_intermediateFramebuffers.clear();

    // Shutdown context
    if (m_context) {
        m_context->shutdown();
    }

    LOGI("GPURenderer shutdown complete");
}

bool GPURenderer::addEffect(std::shared_ptr<GPUEffect> effect) {
    if (!effect || !effect->getName().empty()) {
        LOGE("Invalid effect");
        return false;
    }

    std::string name = effect->getName();

    if (m_effects.find(name) != m_effects.end()) {
        LOGW("Effect already exists: %s", name.c_str());
        return false;
    }

    m_effects[name] = effect;
    m_effectOrder.push_back(name);

    LOGI("Effect added: %s (total: %zu)", name.c_str(), m_effects.size());
    return true;
}

bool GPURenderer::removeEffect(const std::string& effectName) {
    auto it = m_effects.find(effectName);
    if (it == m_effects.end()) {
        LOGW("Effect not found: %s", effectName.c_str());
        return false;
    }

    m_effects.erase(it);

    auto orderIt = std::find(m_effectOrder.begin(), m_effectOrder.end(), effectName);
    if (orderIt != m_effectOrder.end()) {
        m_effectOrder.erase(orderIt);
    }

    LOGI("Effect removed: %s", effectName.c_str());
    return true;
}

std::shared_ptr<GPUEffect> GPURenderer::getEffect(const std::string& effectName) const {
    auto it = m_effects.find(effectName);
    if (it != m_effects.end()) {
        return it->second;
    }

    return nullptr;
}

bool GPURenderer::hasEffect(const std::string& effectName) const {
    return m_effects.find(effectName) != m_effects.end();
}

std::vector<std::string> GPURenderer::getEffectNames() const {
    return m_effectOrder;
}

void GPURenderer::clearEffects() {
    m_effects.clear();
    m_effectOrder.clear();
    LOGI("All effects cleared");
}

bool GPURenderer::setEffectEnabled(const std::string& effectName, bool enabled) {
    auto effect = getEffect(effectName);
    if (!effect) {
        LOGW("Effect not found: %s", effectName.c_str());
        return false;
    }

    effect->setEnabled(enabled);
    LOGI("Effect enabled: %s = %d", effectName.c_str(), enabled);

    return true;
}

bool GPURenderer::renderFrame(GLuint inputTexture, GLuint outputFramebuffer) {
    if (!isInitialized()) {
        LOGE("Renderer not initialized");
        return false;
    }

    if (m_profilingEnabled) {
        beginProfiling();
    }

    // Make context current
    if (!m_context->makeCurrent()) {
        LOGE("Failed to make context current");
        return false;
    }

    // Apply effect chain
    GLuint outputTexture = applyEffectChain(inputTexture);

    if (outputFramebuffer != 0) {
        // Copy final result to output framebuffer
        m_context->bindFramebuffer(outputFramebuffer);
        m_context->clearScreen();

        // Render final texture to output
        // This would use a blit or render operation
    }

    m_context->releaseContext();

    if (m_profilingEnabled) {
        endProfiling();
    }

    return true;
}

GLuint GPURenderer::renderToTexture(GLuint inputTexture) {
    if (!isInitialized()) {
        LOGE("Renderer not initialized");
        return 0;
    }

    // Make context current
    if (!m_context->makeCurrent()) {
        LOGE("Failed to make context current");
        return 0;
    }

    // Create intermediate framebuffer if needed
    GLuint fbo = createIntermediateFramebuffer();
    if (fbo == 0) {
        LOGE("Failed to create framebuffer");
        return 0;
    }

    // Apply effects
    applyEffectChain(inputTexture);

    // Get output texture
    GLuint outputTexture = m_context->getFramebufferTexture(fbo);

    m_context->releaseContext();

    return outputTexture;
}

bool GPURenderer::applyEffect(GLuint inputTexture, const std::string& effectName,
                             GLuint outputFramebuffer) {
    auto effect = getEffect(effectName);
    if (!effect) {
        LOGE("Effect not found: %s", effectName.c_str());
        return false;
    }

    if (!m_context->makeCurrent()) {
        LOGE("Failed to make context current");
        return false;
    }

    // Create framebuffer if needed
    if (outputFramebuffer == 0) {
        outputFramebuffer = createIntermediateFramebuffer();
        if (outputFramebuffer == 0) {
            LOGE("Failed to create output framebuffer");
            return false;
        }
    }

    // Apply effect
    bool result = effect->apply(inputTexture, outputFramebuffer,
                               m_config.renderWidth, m_config.renderHeight);

    m_context->releaseContext();

    return result;
}

GLuint GPURenderer::applyEffectChain(GLuint inputTexture) {
    GLuint currentTexture = inputTexture;

    for (const auto& effectName : m_effectOrder) {
        auto effect = m_effects[effectName];

        if (!effect->isEnabled()) {
            continue;  // Skip disabled effects
        }

        // Create intermediate framebuffer
        GLuint fbo = createIntermediateFramebuffer();
        if (fbo == 0) {
            LOGW("Failed to create framebuffer for effect: %s", effectName.c_str());
            continue;
        }

        // Apply effect
        if (!effect->apply(currentTexture, fbo, m_config.renderWidth, m_config.renderHeight)) {
            LOGW("Failed to apply effect: %s", effectName.c_str());
            m_context->deleteFramebuffer(fbo);
            continue;
        }

        // Next effect uses output as input
        currentTexture = m_context->getFramebufferTexture(fbo);
    }

    return currentTexture;
}

GLuint GPURenderer::createIntermediateFramebuffer() {
    GLuint fbo = m_context->createFramebuffer(m_config.renderWidth,
                                             m_config.renderHeight, GL_RGBA);

    if (fbo != 0) {
        m_intermediateFramebuffers.push_back(fbo);
        LOGD("Intermediate framebuffer created: %u", fbo);
    }

    return fbo;
}

void GPURenderer::releaseIntermediateFramebuffer(GLuint fbo) {
    auto it = std::find(m_intermediateFramebuffers.begin(),
                       m_intermediateFramebuffers.end(), fbo);

    if (it != m_intermediateFramebuffers.end()) {
        m_context->deleteFramebuffer(fbo);
        m_intermediateFramebuffers.erase(it);
        LOGD("Intermediate framebuffer released: %u", fbo);
    }
}

void GPURenderer::beginProfiling() {
    // Implementation would use glGetQueryObjectuiv for GPU timing
    // For now, just CPU timing
}

void GPURenderer::endProfiling() {
    // Update stats
    updateStatistics();
}

void GPURenderer::resetStats() {
    m_stats = RenderStats();
    m_gpuTimeHistory.clear();
    m_cpuTimeHistory.clear();
    m_fpsHistory.clear();
}

void GPURenderer::updateStatistics() {
    // Calculate averages from history
    if (!m_gpuTimeHistory.empty()) {
        float sum = 0.0f;
        for (float time : m_gpuTimeHistory) {
            sum += time;
        }
        m_stats.gpuTimeMs = sum / m_gpuTimeHistory.size();
    }

    if (!m_cpuTimeHistory.empty()) {
        float sum = 0.0f;
        for (float time : m_cpuTimeHistory) {
            sum += time;
        }
        m_stats.cpuTimeMs = sum / m_cpuTimeHistory.size();
    }

    m_stats.totalTimeMs = m_stats.gpuTimeMs + m_stats.cpuTimeMs;

    if (m_stats.totalTimeMs > 0.0f) {
        m_stats.framesPerSecond = static_cast<int>(1000.0f / m_stats.totalTimeMs);
    }

    // Update resource counts
    m_stats.textureCount = m_intermediateFramebuffers.size();
    m_stats.framebufferCount = m_intermediateFramebuffers.size();
}

std::string GPURenderer::getDebugInfo() const {
    std::stringstream ss;

    ss << "GPURenderer Debug Info\n";
    ss << "======================\n";
    ss << "Initialized: " << (isInitialized() ? "yes" : "no") << "\n";
    ss << "Render Target: " << m_config.renderWidth << "x" << m_config.renderHeight << "\n";
    ss << "Output Size: " << m_config.outputWidth << "x" << m_config.outputHeight << "\n";
    ss << "Effects Active: " << m_effects.size() << "\n";
    ss << "Intermediate FBOs: " << m_intermediateFramebuffers.size() << "\n";
    ss << "Profiling Enabled: " << (m_profilingEnabled ? "yes" : "no") << "\n";
    ss << "\nStatistics:\n";
    ss << "  GPU Time: " << m_stats.gpuTimeMs << " ms\n";
    ss << "  CPU Time: " << m_stats.cpuTimeMs << " ms\n";
    ss << "  Total Time: " << m_stats.totalTimeMs << " ms\n";
    ss << "  FPS: " << m_stats.framesPerSecond << "\n";
    ss << "  Memory: " << m_stats.gpuMemoryUsedMB << " MB\n";

    ss << "\nActive Effects:\n";
    for (const auto& name : m_effectOrder) {
        auto effect = m_effects.at(name);
        ss << "  - " << name << " (enabled: " << (effect->isEnabled() ? "yes" : "no") << ")\n";
    }

    return ss.str();
}

std::string GPURenderer::getGPUInfo() const {
    if (!m_context) {
        return "GPU context not initialized";
    }

    std::stringstream ss;
    ss << "GPU Information\n";
    ss << "===============\n";
    ss << "Vendor: " << glGetString(GL_VENDOR) << "\n";
    ss << "Renderer: " << glGetString(GL_RENDERER) << "\n";
    ss << "Version: " << glGetString(GL_VERSION) << "\n";
    ss << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";

    const auto& caps = m_context->getCapabilities();
    ss << "\nCapabilities:\n";
    ss << "  OpenGL ES: " << caps.majorVersion << "." << caps.minorVersion << "\n";
    ss << "  Max Texture Size: " << caps.maxTextureSize << "\n";
    ss << "  Max Renderbuffer: " << caps.maxRenderBufferSize << "\n";
    ss << "  Max Color Attachments: " << caps.maxColorAttachments << "\n";
    ss << "  FBO Support: " << (caps.supportsFramebufferObject ? "yes" : "no") << "\n";
    ss << "  Float Texture: " << (caps.supportsTextureFloat ? "yes" : "no") << "\n";

    return ss.str();
}

} // namespace gpu
} // namespace clipforge
