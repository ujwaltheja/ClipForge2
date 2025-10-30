#ifndef CLIPFORGE_EFFECTS_PROCESSOR_H
#define CLIPFORGE_EFFECTS_PROCESSOR_H

#include <vector>
#include <memory>
#include <cstdint>
#include <string>
#include <map>
#include <functional>

namespace clipforge {
namespace effects {

/**
 * @enum EffectType
 * @brief Available video effects
 */
enum class EffectType {
    // Color effects
    COLOR_GRADE = 0,
    HUE_SATURATION = 1,
    CURVES = 2,
    LEVELS = 3,
    WHITE_BALANCE = 4,

    // Visual effects
    BLUR = 5,
    VIGNETTE = 6,
    GLOW = 7,
    SHARPEN = 8,
    BLOOM = 9,

    // Distortion effects
    CHROMATIC_ABERRATION = 10,
    GLITCH = 11,
    LENS_DISTORTION = 12,
    WAVE_DISTORTION = 13,

    // Special effects
    GRAYSCALE = 14,
    POSTERIZE = 15,
    INVERT = 16,
    COLOR_OVERLAY = 17,

    // Advanced
    LIGHT_LEAK = 18,
    FILM_GRAIN = 19,
    MOTION_BLUR = 20,
};

/**
 * @struct EffectParameters
 * @brief Base effect parameters structure
 */
struct EffectParameters {
    virtual ~EffectParameters() = default;
    virtual std::unique_ptr<EffectParameters> clone() const = 0;
};

/**
 * @struct ColorGradeParams
 * @brief Color grading parameters
 */
struct ColorGradeParams : public EffectParameters {
    // Shadows, Midtones, Highlights
    float shadowsR = 0.0f, shadowsG = 0.0f, shadowsB = 0.0f;
    float midtonesR = 0.0f, midtonesG = 0.0f, midtonesB = 0.0f;
    float highlightsR = 0.0f, highlightsG = 0.0f, highlightsB = 0.0f;

    // Global adjustments
    float exposure = 0.0f;      // -2.0 to 2.0
    float saturation = 1.0f;    // 0.0 to 2.0
    float contrast = 1.0f;      // 0.0 to 2.0
    float temperature = 0.0f;   // -100 to 100 (blue to warm)
    float tint = 0.0f;          // -100 to 100 (green to magenta)

    // Curves (simplified)
    std::vector<float> redCurve;
    std::vector<float> greenCurve;
    std::vector<float> blueCurve;
    std::vector<float> luminanceCurve;

    std::unique_ptr<EffectParameters> clone() const override {
        return std::make_unique<ColorGradeParams>(*this);
    }
};

/**
 * @struct BlurParams
 * @brief Blur effect parameters
 */
struct BlurParams : public EffectParameters {
    float radius = 5.0f;        // 0 to 50
    enum class BlurType { GAUSSIAN, MOTION, RADIAL, ZOOM } type = BlurType::GAUSSIAN;
    float motionAngle = 0.0f;   // 0 to 360 degrees

    std::unique_ptr<EffectParameters> clone() const override {
        return std::make_unique<BlurParams>(*this);
    }
};

/**
 * @struct ChromaticAberrationParams
 * @brief Chromatic aberration parameters
 */
struct ChromaticAberrationParams : public EffectParameters {
    float amount = 2.0f;        // 0 to 10
    float angle = 0.0f;         // 0 to 360

    std::unique_ptr<EffectParameters> clone() const override {
        return std::make_unique<ChromaticAberrationParams>(*this);
    }
};

/**
 * @struct VignetteParams
 * @brief Vignette effect parameters
 */
struct VignetteParams : public EffectParameters {
    float opacity = 0.5f;       // 0 to 1
    float intensity = 0.5f;     // 0 to 1
    float scale = 1.0f;         // 0.5 to 2.0

    std::unique_ptr<EffectParameters> clone() const override {
        return std::make_unique<VignetteParams>(*this);
    }
};

/**
 * @struct GlitchParams
 * @brief Glitch effect parameters
 */
struct GlitchParams : public EffectParameters {
    float strength = 0.5f;      // 0 to 1
    float frequency = 0.5f;     // 0 to 1
    float scanlineHeight = 5.0f;
    bool useColorShift = true;

    std::unique_ptr<EffectParameters> clone() const override {
        return std::make_unique<GlitchParams>(*this);
    }
};

/**
 * @class EffectsProcessor
 * @brief Main effects processor for GPU-accelerated video effects
 */
class EffectsProcessor {
public:
    /**
     * @brief Create effects processor
     */
    EffectsProcessor();

    /**
     * @brief Destructor
     */
    ~EffectsProcessor() = default;

    /**
     * @brief Apply effect to frame
     * @param frameData Raw video frame data (RGBA)
     * @param width Frame width
     * @param height Frame height
     * @param effectType Effect type to apply
     * @param params Effect parameters
     * @return true if effect applied successfully
     */
    bool applyEffect(uint8_t* frameData, int width, int height,
                     EffectType effectType, const EffectParameters& params);

    /**
     * @brief Apply multiple effects in sequence
     * @param frameData Raw video frame data
     * @param width Frame width
     * @param height Frame height
     * @param effects List of (effectType, parameters)
     * @return true if all effects applied
     */
    bool applyEffects(uint8_t* frameData, int width, int height,
                      const std::vector<std::pair<EffectType, std::shared_ptr<EffectParameters>>>& effects);

    /**
     * @brief Apply color grading
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param params Color grading parameters
     */
    void applyColorGrade(uint8_t* frameData, int width, int height,
                         const ColorGradeParams& params);

    /**
     * @brief Apply HSL adjustment (Hue, Saturation, Lightness)
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param hueShift Hue shift in degrees (-180 to 180)
     * @param saturation Saturation multiplier (0 to 2)
     * @param lightness Lightness adjustment (-1 to 1)
     */
    void applyHSLAdjustment(uint8_t* frameData, int width, int height,
                            float hueShift, float saturation, float lightness);

    /**
     * @brief Apply curves adjustment
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param curve Curve values (256 points from 0-255 input to 0-255 output)
     */
    void applyCurves(uint8_t* frameData, int width, int height,
                     const std::vector<uint8_t>& curve);

    /**
     * @brief Apply blur effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param params Blur parameters
     */
    void applyBlur(uint8_t* frameData, int width, int height,
                   const BlurParams& params);

    /**
     * @brief Apply vignette effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param params Vignette parameters
     */
    void applyVignette(uint8_t* frameData, int width, int height,
                       const VignetteParams& params);

    /**
     * @brief Apply glow effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param intensity Glow intensity (0 to 1)
     * @param threshold Glow threshold (0 to 1)
     */
    void applyGlow(uint8_t* frameData, int width, int height,
                   float intensity, float threshold);

    /**
     * @brief Apply chromatic aberration
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param params Chromatic aberration parameters
     */
    void applyChromaticAberration(uint8_t* frameData, int width, int height,
                                  const ChromaticAberrationParams& params);

    /**
     * @brief Apply glitch effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param params Glitch parameters
     */
    void applyGlitch(uint8_t* frameData, int width, int height,
                     const GlitchParams& params);

    /**
     * @brief Apply grayscale effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param intensity Grayscale intensity (0 to 1)
     */
    void applyGrayscale(uint8_t* frameData, int width, int height,
                        float intensity);

    /**
     * @brief Apply film grain effect
     * @param frameData RGBA frame data
     * @param width Frame width
     * @param height Frame height
     * @param intensity Grain intensity (0 to 1)
     */
    void applyFilmGrain(uint8_t* frameData, int width, int height,
                        float intensity);

    /**
     * @brief Get effect name
     * @param effectType Effect type
     * @return Human-readable effect name
     */
    static std::string getEffectName(EffectType effectType);

    /**
     * @brief Get all available effects
     * @return Vector of (EffectType, Name) pairs
     */
    static std::vector<std::pair<EffectType, std::string>> getAllEffects();

private:
    // Helper functions
    void rgbToHsv(float r, float g, float b, float& h, float& s, float& v) const;
    void hsvToRgb(float h, float s, float v, float& r, float& g, float& b) const;
    void rgbToHsl(float r, float g, float b, float& h, float& s, float& l) const;
    void hslToRgb(float h, float s, float l, float& r, float& g, float& b) const;

    // Gaussian blur helper
    void gaussianBlur(uint8_t* frameData, int width, int height,
                      uint8_t* tempBuffer, float radius);

    // Perlin noise for effects
    float perlinNoise(float x, float y, float z) const;
};

} // namespace effects
} // namespace clipforge

#endif // CLIPFORGE_EFFECTS_PROCESSOR_H
