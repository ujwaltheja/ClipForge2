#include "color_grade_effect.h"
#include "../../gpu/shader_sources.h"
#include "../../utils/logger.h"

namespace clipforge {
namespace gpu {
namespace effects {

// ===== ColorGradeEffect Implementation =====

ColorGradeEffect::ColorGradeEffect()
    : GPUEffect("ColorGrade", EffectCategory::COLOR, nullptr) {
    // Note: Shader needs to be created separately and assigned
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);
    defineParameter("temperature", "uTemperature", 0.5f, 0.0f, 1.0f);
    defineParameter("tint", "uTint", 0.5f, 0.0f, 1.0f);

    LOGI("ColorGradeEffect created");
}

void ColorGradeEffect::setLUT(GLuint lutTexture) {
    m_lutTexture = lutTexture;
    LOGI("LUT texture set for ColorGradeEffect: %u", lutTexture);
}

std::vector<EffectParameter> ColorGradeEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void ColorGradeEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    // Bind LUT texture
    if (m_lutTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_lutTexture);
        m_shader->setUniform("uLUT", 1);
    }

    // Set parameters
    m_shader->setUniform("uTemperature", getParameter("temperature"));
    m_shader->setUniform("uTint", getParameter("tint"));
}

// ===== CurvesEffect Implementation =====

CurvesEffect::CurvesEffect()
    : GPUEffect("Curves", EffectCategory::COLOR, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);
    defineParameter("redMidtone", "uRedMidtone", 0.5f, 0.0f, 1.0f);
    defineParameter("greenMidtone", "uGreenMidtone", 0.5f, 0.0f, 1.0f);
    defineParameter("blueMidtone", "uBlueMidtone", 0.5f, 0.0f, 1.0f);
    defineParameter("luminanceMidtone", "uLuminanceMidtone", 0.5f, 0.0f, 1.0f);

    LOGI("CurvesEffect created");
}

void CurvesEffect::setCurveTexture(GLuint curveTexture) {
    m_curveTexture = curveTexture;
    LOGI("Curve texture set for CurvesEffect: %u", curveTexture);
}

std::vector<EffectParameter> CurvesEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void CurvesEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    // Bind curve texture
    if (m_curveTexture != 0) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_curveTexture);
        m_shader->setUniform("uCurveTexture", 1);
    }

    // Set curve parameters
    m_shader->setUniform("uRedMidtone", getParameter("redMidtone"));
    m_shader->setUniform("uGreenMidtone", getParameter("greenMidtone"));
    m_shader->setUniform("uBlueMidtone", getParameter("blueMidtone"));
    m_shader->setUniform("uLuminanceMidtone", getParameter("luminanceMidtone"));
}

// ===== HSLEffect Implementation =====

HSLEffect::HSLEffect()
    : GPUEffect("HSL", EffectCategory::COLOR, nullptr) {
    defineParameter("intensity", "uIntensity", 1.0f, 0.0f, 1.0f);
    defineParameter("hue", "uHue", 0.5f, 0.0f, 1.0f);           // Maps to -180 to 180
    defineParameter("saturation", "uSaturation", 0.5f, 0.0f, 1.0f);  // Maps to -1 to 1
    defineParameter("lightness", "uLightness", 0.5f, 0.0f, 1.0f);    // Maps to -1 to 1

    LOGI("HSLEffect created");
}

std::vector<EffectParameter> HSLEffect::getParameters() const {
    return GPUEffect::getParameters();
}

void HSLEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    if (!m_shader) return;

    // Convert normalized parameters to actual ranges
    float hueParam = getParameter("hue");
    float saturationParam = getParameter("saturation");
    float lightnessParam = getParameter("lightness");

    // Map normalized values to effect ranges
    float hue = (hueParam * 2.0f - 1.0f) * 180.0f;           // -180 to 180
    float saturation = saturationParam * 2.0f - 1.0f;        // -1 to 1
    float lightness = lightnessParam * 2.0f - 1.0f;          // -1 to 1

    m_shader->setUniform("uHue", hue);
    m_shader->setUniform("uSaturation", saturation);
    m_shader->setUniform("uLightness", lightness);
}

} // namespace effects
} // namespace gpu
} // namespace clipforge
