#ifndef CLIPFORGE_EFFECT_H
#define CLIPFORGE_EFFECT_H

#include <string>
#include <vector>
#include <memory>
#include <variant>

namespace clipforge {
namespace models {

/**
 * @enum EffectType
 * @brief Enumeration of all supported effect types
 */
enum class EffectType {
    // Filters
    FILTER_VINTAGE,
    FILTER_BW,
    FILTER_WARM,
    FILTER_COOL,
    FILTER_SEPIA,
    FILTER_VIVID,
    FILTER_NOSTALGIA,
    FILTER_MIDNIGHT,
    FILTER_FESTIVE,

    // Transitions
    TRANSITION_FADE,
    TRANSITION_DISSOLVE,
    TRANSITION_WIPE,
    TRANSITION_SLIDE,
    TRANSITION_ZOOM,

    // Color Adjustments
    COLOR_BRIGHTNESS,
    COLOR_CONTRAST,
    COLOR_SATURATION,
    COLOR_TEMPERATURE,
    COLOR_TINT,
    COLOR_EXPOSURE,
    COLOR_SHADOWS,
    COLOR_HIGHLIGHTS,

    // Blur Effects
    BLUR_STANDARD,
    BLUR_MOTION,
    BLUR_RADIAL,
    BLUR_ZOOM,

    // Audio Effects
    AUDIO_REVERB,
    AUDIO_EQUALIZER,
    AUDIO_PITCH_SHIFT,
    AUDIO_COMPRESSION,
    AUDIO_DISTORTION,

    // Special Effects
    SPECIAL_EDGE_DETECTION,
    SPECIAL_SHARPEN,
    SPECIAL_DENOISE,
    SPECIAL_INVERT,
    SPECIAL_POSTERIZE,
    SPECIAL_VIGNETTE,
};

/**
 * @struct EffectParameter
 * @brief Represents a single effect parameter with its value
 */
struct EffectParameter {
    std::string name;              // Parameter name (e.g., "intensity")
    float value;                   // Current value
    float minValue;                // Minimum allowed value
    float maxValue;                // Maximum allowed value
    float defaultValue;            // Default value

    EffectParameter(const std::string& n, float v, float min, float max, float def)
        : name(n), value(v), minValue(min), maxValue(max), defaultValue(def) {}
};

/**
 * @class Effect
 * @brief Represents a video effect with parameters
 *
 * An effect encapsulates a specific visual or audio transformation
 * that can be applied to video clips. Effects are immutable once
 * created for thread safety.
 */
class Effect {
public:
    using ParameterList = std::vector<EffectParameter>;

    /**
     * @brief Construct an effect with type and parameters
     * @param type The type of effect to create
     * @param name Display name for the effect
     */
    explicit Effect(EffectType type, const std::string& name = "");

    /**
     * @brief Copy constructor
     */
    Effect(const Effect& other) = default;

    /**
     * @brief Move constructor
     */
    Effect(Effect&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    ~Effect() = default;

    /**
     * @brief Get the effect type
     * @return The type of this effect
     */
    [[nodiscard]] EffectType getType() const { return m_type; }

    /**
     * @brief Get the display name of the effect
     * @return Human-readable effect name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }

    /**
     * @brief Get unique effect ID
     * @return UUID string for this effect instance
     */
    [[nodiscard]] const std::string& getId() const { return m_id; }

    /**
     * @brief Add a parameter to this effect
     * @param parameter The parameter to add
     */
    void addParameter(const EffectParameter& parameter);

    /**
     * @brief Get all parameters for this effect
     * @return Vector of all parameters
     */
    [[nodiscard]] const ParameterList& getParameters() const { return m_parameters; }

    /**
     * @brief Set a parameter value
     * @param paramName Name of parameter to set
     * @param value New value (will be clamped to min/max)
     * @return true if parameter was found and set
     */
    bool setParameterValue(const std::string& paramName, float value);

    /**
     * @brief Get a parameter value by name
     * @param paramName Name of parameter to retrieve
     * @return Parameter value or 0.0f if not found
     */
    [[nodiscard]] float getParameterValue(const std::string& paramName) const;

    /**
     * @brief Get intensity/strength of this effect (0.0 to 1.0)
     * @return Effect intensity
     */
    [[nodiscard]] float getIntensity() const { return m_intensity; }

    /**
     * @brief Set intensity/strength of this effect
     * @param intensity Value between 0.0 and 1.0
     */
    void setIntensity(float intensity);

    /**
     * @brief Check if this effect is enabled
     * @return true if effect should be applied
     */
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    /**
     * @brief Enable or disable this effect
     * @param enabled true to enable, false to disable
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Get when this effect was created
     * @return Creation timestamp in milliseconds
     */
    [[nodiscard]] int64_t getCreatedAt() const { return m_createdAt; }

    /**
     * @brief Check if this is a serializable effect
     * @return true if effect can be saved/loaded
     */
    [[nodiscard]] bool isSerializable() const { return m_serializable; }

private:
    EffectType m_type;
    std::string m_id;              // Unique identifier
    std::string m_name;            // Display name
    ParameterList m_parameters;    // Effect parameters
    float m_intensity = 1.0f;      // Overall effect strength
    bool m_enabled = true;         // Whether effect is active
    int64_t m_createdAt;           // Creation timestamp
    bool m_serializable = true;    // Can be saved to project

    /**
     * @brief Generate unique ID for this effect
     * @return UUID string
     */
    static std::string generateId();
};

/**
 * @class EffectChain
 * @brief Represents a sequence of effects applied to a clip
 *
 * Effects are applied in order. The output of one effect
 * becomes the input to the next.
 */
class EffectChain {
public:
    using EffectList = std::vector<std::shared_ptr<Effect>>;

    /**
     * @brief Construct an empty effect chain
     */
    EffectChain() = default;

    /**
     * @brief Destructor
     */
    ~EffectChain() = default;

    /**
     * @brief Add an effect to the chain
     * @param effect The effect to add
     */
    void addEffect(std::shared_ptr<Effect> effect);

    /**
     * @brief Remove an effect from the chain
     * @param effectId ID of effect to remove
     * @return true if effect was found and removed
     */
    bool removeEffect(const std::string& effectId);

    /**
     * @brief Get all effects in this chain
     * @return Vector of effects in order
     */
    [[nodiscard]] const EffectList& getEffects() const { return m_effects; }

    /**
     * @brief Get effect count
     * @return Number of effects in chain
     */
    [[nodiscard]] size_t getEffectCount() const { return m_effects.size(); }

    /**
     * @brief Clear all effects from chain
     */
    void clear() { m_effects.clear(); }

    /**
     * @brief Check if chain is empty
     * @return true if no effects in chain
     */
    [[nodiscard]] bool isEmpty() const { return m_effects.empty(); }

private:
    EffectList m_effects;
};

} // namespace models
} // namespace clipforge

#endif // CLIPFORGE_EFFECT_H
