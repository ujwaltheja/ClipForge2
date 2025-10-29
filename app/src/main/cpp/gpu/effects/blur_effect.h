#ifndef CLIPFORGE_BLUR_EFFECT_H
#define CLIPFORGE_BLUR_EFFECT_H

#include "../gpu_effect.h"

namespace clipforge {
namespace gpu {
namespace effects {

/**
 * @class GaussianBlurEffect
 * @brief High-quality Gaussian blur effect
 *
 * Implements separable 2D Gaussian blur for efficient processing.
 * Can be applied as single pass for performance or multi-pass for quality.
 */
class GaussianBlurEffect : public GPUEffect {
public:
    /**
     * @brief Create Gaussian blur effect
     */
    GaussianBlurEffect();

    /**
     * @brief Set blur radius in pixels
     * @param radius Blur radius (0.5 to 50.0 pixels)
     */
    void setRadius(float radius) { setParameter("radius", radius); }

    /**
     * @brief Get blur radius
     * @return Current radius value
     */
    [[nodiscard]] float getRadius() const { return getParameter("radius"); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class VignetteEffect
 * @brief Vignette darkening effect
 *
 * Darkens the edges of the image, with adjustable radius and falloff.
 */
class VignetteEffect : public GPUEffect {
public:
    /**
     * @brief Create vignette effect
     */
    VignetteEffect();

    /**
     * @brief Set vignette radius
     * @param radius Vignette radius (0.2 to 1.0)
     */
    void setRadius(float radius) { setParameter("radius", radius); }

    /**
     * @brief Set vignette softness/feathering
     * @param softness Softness value (0.0 to 1.0)
     */
    void setSoftness(float softness) { setParameter("softness", softness); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class GlowEffect
 * @brief Bloom/glow effect
 *
 * Creates a glow effect by brightening highlights and applying bloom.
 */
class GlowEffect : public GPUEffect {
public:
    /**
     * @brief Create glow effect
     */
    GlowEffect();

    /**
     * @brief Set brightness threshold for glow
     * @param threshold Threshold value (0.0 to 1.0)
     */
    void setThreshold(float threshold) { setParameter("threshold", threshold); }

    /**
     * @brief Set glow spread/blur
     * @param spread Spread amount (0.0 to 1.0)
     */
    void setSpread(float spread) { setParameter("spread", spread); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

} // namespace effects
} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_BLUR_EFFECT_H
