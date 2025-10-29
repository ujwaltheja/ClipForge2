#ifndef CLIPFORGE_COLOR_GRADE_EFFECT_H
#define CLIPFORGE_COLOR_GRADE_EFFECT_H

#include "../gpu_effect.h"

namespace clipforge {
namespace gpu {
namespace effects {

/**
 * @class ColorGradeEffect
 * @brief LUT-based color grading effect
 *
 * Applies professional color grading using a 3D LUT (Look-Up Table).
 * The LUT is typically a 16x16x16 cube stored as a 2D texture.
 */
class ColorGradeEffect : public GPUEffect {
public:
    /**
     * @brief Create color grade effect
     */
    ColorGradeEffect();

    /**
     * @brief Set LUT texture for color grading
     * @param lutTexture Texture ID of 3D LUT
     */
    void setLUT(GLuint lutTexture);

    /**
     * @brief Get current LUT texture
     * @return Texture ID
     */
    [[nodiscard]] GLuint getLUT() const { return m_lutTexture; }

    /**
     * @brief Set color temperature adjustment
     * @param temp Temperature value (-100 to 100, warm to cool)
     */
    void setTemperature(float temp) { setParameter("temperature", (temp + 100.0f) / 200.0f); }

    /**
     * @brief Set tint adjustment
     * @param tint Tint value (-100 to 100, magenta to green)
     */
    void setTint(float tint) { setParameter("tint", (tint + 100.0f) / 200.0f); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
    GLuint m_lutTexture = 0;
};

/**
 * @class CurvesEffect
 * @brief RGB Curves adjustment effect
 *
 * Allows independent adjustment of red, green, and blue curves,
 * as well as luminance curves for contrast control.
 */
class CurvesEffect : public GPUEffect {
public:
    /**
     * @brief Create curves effect
     */
    CurvesEffect();

    /**
     * @brief Set curve texture
     * @param curveTexture Texture ID (256x3 or 256x4 for RGBA curves)
     */
    void setCurveTexture(GLuint curveTexture);

    /**
     * @brief Adjust red channel curve
     * @param midtone Midpoint adjustment (0.0-1.0)
     */
    void setRedCurve(float midtone) { setParameter("redMidtone", midtone); }

    /**
     * @brief Adjust green channel curve
     * @param midtone Midpoint adjustment
     */
    void setGreenCurve(float midtone) { setParameter("greenMidtone", midtone); }

    /**
     * @brief Adjust blue channel curve
     * @param midtone Midpoint adjustment
     */
    void setBlueCurve(float midtone) { setParameter("blueMidtone", midtone); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
    GLuint m_curveTexture = 0;
};

/**
 * @class HSLEffect
 * @brief Hue, Saturation, Lightness adjustment effect
 */
class HSLEffect : public GPUEffect {
public:
    /**
     * @brief Create HSL adjustment effect
     */
    HSLEffect();

    /**
     * @brief Set hue shift
     * @param hue Hue shift in degrees (-180 to 180)
     */
    void setHue(float hue) { setParameter("hue", (hue + 180.0f) / 360.0f); }

    /**
     * @brief Set saturation adjustment
     * @param saturation Saturation adjustment (-1.0 to 1.0)
     */
    void setSaturation(float saturation) { setParameter("saturation", (saturation + 1.0f) / 2.0f); }

    /**
     * @brief Set lightness adjustment
     * @param lightness Lightness adjustment (-1.0 to 1.0)
     */
    void setLightness(float lightness) { setParameter("lightness", (lightness + 1.0f) / 2.0f); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class GradePreset
 * @brief Pre-configured color grading preset
 */
struct GradePreset {
    std::string name;
    float temperature = 0.0f;
    float tint = 0.0f;
    float saturation = 0.0f;
    float contrast = 0.0f;
    float hue = 0.0f;
};

} // namespace effects
} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_COLOR_GRADE_EFFECT_H
