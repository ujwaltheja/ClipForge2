#include "distortion_effect.h"
#include "../../utils/logger.h"
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

namespace clipforge {
namespace gpu {
namespace effects {

// ===== ChromaticAberrationEffect Implementation =====

ChromaticAberrationEffect::ChromaticAberrationEffect()
    : GPUEffect("ChromaticAberration", EffectCategory::DISTORTION, nullptr) {
    defineParameter("intensity", "uIntensity", 0.5f, 0.0f, 1.0f);
    defineParameter("amount", "uAmount", 0.3f, 0.0f, 1.0f);
    defineParameter("direction", "uDirection", 0.0f, 0.0f, 360.0f);

    LOGI("ChromaticAberrationEffect created");
}

std::vector<EffectParameter> ChromaticAberrationEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void ChromaticAberrationEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    float amount = getParameter("amount");
    float direction = getParameter("direction") * 3.14159f / 180.0f;  // Convert to radians

    // Calculate offset based on direction
    glm::vec2 offset(
        glm::cos(direction) * amount * 0.02f,
        glm::sin(direction) * amount * 0.02f
    );

    m_shader->setUniform("uOffset", offset);
}

// ===== GlitchEffect Implementation =====

GlitchEffect::GlitchEffect()
    : GPUEffect("Glitch", EffectCategory::DISTORTION, nullptr) {
    defineParameter("intensity", "uIntensity", 0.5f, 0.0f, 1.0f);
    defineParameter("amount", "uAmount", 0.3f, 0.0f, 1.0f);
    defineParameter("frequency", "uFrequency", 0.5f, 0.0f, 1.0f);

    LOGI("GlitchEffect created");
}

std::vector<EffectParameter> GlitchEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void GlitchEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    m_shader->setUniform("uAmount", getParameter("amount"));
    m_shader->setUniform("uFrequency", getParameter("frequency"));
    m_shader->setUniform("uTime", m_time);
}

// ===== PosterizeEffect Implementation =====

PosterizeEffect::PosterizeEffect()
    : GPUEffect("Posterize", EffectCategory::ARTISTIC, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);
    defineParameter("levels", "uLevels", 128.0f, 2.0f, 256.0f);

    LOGI("PosterizeEffect created");
}

std::vector<EffectParameter> PosterizeEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void PosterizeEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    float levels = getParameter("levels");
    m_shader->setUniform("uLevels", levels);
}

// ===== InvertEffect Implementation =====

InvertEffect::InvertEffect()
    : GPUEffect("Invert", EffectCategory::ARTISTIC, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);

    LOGI("InvertEffect created");
}

std::vector<EffectParameter> InvertEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void InvertEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;
    // No custom uniforms needed beyond intensity
}

// ===== GrayscaleEffect Implementation =====

GrayscaleEffect::GrayscaleEffect()
    : GPUEffect("Grayscale", EffectCategory::ARTISTIC, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);

    LOGI("GrayscaleEffect created");
}

std::vector<EffectParameter> GrayscaleEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void GrayscaleEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;
    // No custom uniforms needed beyond intensity
}

} // namespace effects
} // namespace gpu
} // namespace clipforge
