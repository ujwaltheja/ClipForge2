#include "blur_effect.h"
#include "../../utils/logger.h"

namespace clipforge {
namespace gpu {
namespace effects {

// ===== GaussianBlurEffect Implementation =====

GaussianBlurEffect::GaussianBlurEffect()
    : GPUEffect("GaussianBlur", EffectCategory::BLUR, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);
    defineParameter("radius", "uRadius", 5.0f, 0.5f, 50.0f);

    LOGI("GaussianBlurEffect created");
}

std::vector<EffectParameter> GaussianBlurEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void GaussianBlurEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    // Calculate texel size
    float texelWidth = 1.0f / width;
    float texelHeight = 1.0f / height;
    glm::vec2 texelSize(texelWidth, texelHeight);

    m_shader->setUniform("uTexelSize", texelSize);
    m_shader->setUniform("uRadius", getParameter("radius"));
}

// ===== VignetteEffect Implementation =====

VignetteEffect::VignetteEffect()
    : GPUEffect("Vignette", EffectCategory::LIGHT, nullptr) {
    defineParameter("intensity", "uIntensity", 0.5f, 0.0f, 1.0f);
    defineParameter("radius", "uRadius", 0.5f, 0.2f, 1.0f);
    defineParameter("softness", "uSoftness", 0.3f, 0.0f, 1.0f);

    LOGI("VignetteEffect created");
}

std::vector<EffectParameter> VignetteEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void VignetteEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    m_shader->setUniform("uRadius", getParameter("radius"));

    // Softness parameter can be mapped to effect intensity
    float softness = getParameter("softness");
    m_shader->setUniform("uIntensity", m_intensity * softness);
}

// ===== GlowEffect Implementation =====

GlowEffect::GlowEffect()
    : GPUEffect("Glow", EffectCategory::LIGHT, nullptr) {
    defineParameter("intensity", "uIntensity", 0.5f, 0.0f, 1.0f);
    defineParameter("threshold", "uThreshold", 0.5f, 0.0f, 1.0f);
    defineParameter("spread", "uSpread", 0.3f, 0.0f, 1.0f);

    LOGI("GlowEffect created");
}

std::vector<EffectParameter> GlowEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void GlowEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    float threshold = getParameter("threshold");
    float spread = getParameter("spread");

    m_shader->setUniform("uThreshold", threshold);
    m_shader->setUniform("uIntensity", m_intensity * (1.0f + spread * 2.0f));
}

} // namespace effects
} // namespace gpu
} // namespace clipforge
