#ifndef CLIPFORGE_DISTORTION_EFFECT_H
#define CLIPFORGE_DISTORTION_EFFECT_H

#include "../gpu_effect.h"

namespace clipforge {
namespace gpu {
namespace effects {

/**
 * @class ChromaticAberrationEffect
 * @brief RGB channel separation effect
 *
 * Creates a chromatic aberration effect by separating color channels
 * with slight offsets, mimicking lens imperfections.
 */
class ChromaticAberrationEffect : public GPUEffect {
public:
    /**
     * @brief Create chromatic aberration effect
     */
    ChromaticAberrationEffect();

    /**
     * @brief Set aberration amount
     * @param amount Offset amount (0.0 to 1.0)
     */
    void setAmount(float amount) { setParameter("amount", amount); }

    /**
     * @brief Set aberration direction
     * @param direction Direction angle in degrees (0-360)
     */
    void setDirection(float direction) { setParameter("direction", direction); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class GlitchEffect
 * @brief Video glitch/digital distortion effect
 *
 * Creates random horizontal displacement lines for a glitchy appearance.
 * Useful for transition effects and creative styling.
 */
class GlitchEffect : public GPUEffect {
public:
    /**
     * @brief Create glitch effect
     */
    GlitchEffect();

    /**
     * @brief Set glitch amount
     * @param amount Glitch displacement (0.0 to 1.0)
     */
    void setAmount(float amount) { setParameter("amount", amount); }

    /**
     * @brief Set glitch frequency
     * @param frequency How often glitches occur (0.0 to 1.0)
     */
    void setFrequency(float frequency) { setParameter("frequency", frequency); }

    /**
     * @brief Set animation time (for temporal variation)
     * @param time Time value in seconds
     */
    void setTime(float time) { m_time = time; }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
    float m_time = 0.0f;
};

/**
 * @class PosterizeEffect
 * @brief Color quantization effect
 *
 * Reduces the color palette by quantizing to fewer levels,
 * creating a posterized/limited color appearance.
 */
class PosterizeEffect : public GPUEffect {
public:
    /**
     * @brief Create posterize effect
     */
    PosterizeEffect();

    /**
     * @brief Set number of color levels
     * @param levels Number of levels per channel (2 to 256)
     */
    void setLevels(float levels) { setParameter("levels", levels); }

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class InvertEffect
 * @brief Color inversion effect
 *
 * Inverts all color channels (RGB) while preserving alpha.
 */
class InvertEffect : public GPUEffect {
public:
    /**
     * @brief Create invert effect
     */
    InvertEffect();

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

/**
 * @class GrayscaleEffect
 * @brief Grayscale/desaturation effect
 *
 * Converts color to grayscale using luminance-preserving weights.
 */
class GrayscaleEffect : public GPUEffect {
public:
    /**
     * @brief Create grayscale effect
     */
    GrayscaleEffect();

    std::vector<EffectParameter> getParameters() const override;
    void applyCustomUniforms(GLuint inputTexture, int width, int height) override;

private:
};

} // namespace effects
} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_DISTORTION_EFFECT_H
