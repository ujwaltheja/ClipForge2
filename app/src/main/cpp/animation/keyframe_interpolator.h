#ifndef CLIPFORGE_KEYFRAME_INTERPOLATOR_H
#define CLIPFORGE_KEYFRAME_INTERPOLATOR_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cmath>
#include <algorithm>

namespace clipforge {
namespace animation {

/**
 * @enum InterpolationType
 * @brief Keyframe interpolation methods
 */
enum class InterpolationType {
    LINEAR = 0,           // Linear interpolation
    EASE_IN = 1,          // Slow start, fast end
    EASE_OUT = 2,         // Fast start, slow end
    EASE_IN_OUT = 3,      // Slow start and end, fast middle
    CUBIC = 4,            // Cubic interpolation for smooth curves
    EASE_IN_QUAD = 5,     // Quadratic ease in
    EASE_OUT_QUAD = 6,    // Quadratic ease out
    EASE_IN_OUT_QUAD = 7, // Quadratic ease in-out
    EASE_IN_CUBIC = 8,    // Cubic ease in
    EASE_OUT_CUBIC = 9,   // Cubic ease out
    EASE_IN_OUT_CUBIC = 10, // Cubic ease in-out
    BOUNCE = 11,          // Bounce effect
    ELASTIC = 12,         // Elastic effect
    STEP = 13,            // Step function
};

/**
 * @struct Keyframe
 * @brief Single animation keyframe
 */
struct Keyframe {
    int64_t timeMs;              // Time in milliseconds
    float value;                 // Value at this keyframe
    InterpolationType interpolation = InterpolationType::LINEAR;

    // Bezier control points for advanced interpolation
    float inTangentX = 0.0f;     // Incoming control point X
    float inTangentY = 0.0f;     // Incoming control point Y
    float outTangentX = 0.0f;    // Outgoing control point X
    float outTangentY = 0.0f;    // Outgoing control point Y

    Keyframe() = default;
    Keyframe(int64_t time, float val)
        : timeMs(time), value(val) {}
};

/**
 * @class KeyframeInterpolator
 * @brief Handles keyframe-based animation and interpolation
 *
 * Supports multiple interpolation types for smooth animations.
 * Used for:
 * - Opacity changes
 * - Scale animations
 * - Position animations
 * - Effect parameter animations
 * - Speed ramping
 */
class KeyframeInterpolator {
public:
    /**
     * @brief Create keyframe interpolator
     */
    KeyframeInterpolator();

    /**
     * @brief Add a keyframe
     * @param timeMs Time in milliseconds
     * @param value Value at this time
     * @param interpolation Interpolation type to use until next keyframe
     */
    void addKeyframe(int64_t timeMs, float value,
                    InterpolationType interpolation = InterpolationType::LINEAR);

    /**
     * @brief Add keyframe with bezier control points
     * @param timeMs Time in milliseconds
     * @param value Value at this time
     * @param inTangentX Incoming tangent X
     * @param inTangentY Incoming tangent Y
     * @param outTangentX Outgoing tangent X
     * @param outTangentY Outgoing tangent Y
     */
    void addBezierKeyframe(int64_t timeMs, float value,
                          float inTangentX, float inTangentY,
                          float outTangentX, float outTangentY);

    /**
     * @brief Remove keyframe at specific time
     * @param timeMs Time of keyframe to remove
     * @return true if removed
     */
    bool removeKeyframe(int64_t timeMs);

    /**
     * @brief Remove all keyframes
     */
    void clearKeyframes();

    /**
     * @brief Get interpolated value at specific time
     * @param timeMs Current time in milliseconds
     * @return Interpolated value
     */
    [[nodiscard]] float getValue(int64_t timeMs) const;

    /**
     * @brief Get value with clamping
     * @param timeMs Current time
     * @param clampMin Minimum clamped value
     * @param clampMax Maximum clamped value
     * @return Clamped interpolated value
     */
    [[nodiscard]] float getValueClamped(int64_t timeMs, float clampMin, float clampMax) const;

    /**
     * @brief Get velocity (rate of change) at specific time
     * @param timeMs Current time in milliseconds
     * @return Derivative of interpolated value
     */
    [[nodiscard]] float getVelocity(int64_t timeMs) const;

    /**
     * @brief Get all keyframes
     * @return Reference to keyframe list
     */
    [[nodiscard]] const std::vector<Keyframe>& getKeyframes() const { return m_keyframes; }

    /**
     * @brief Get keyframe count
     * @return Number of keyframes
     */
    [[nodiscard]] size_t getKeyframeCount() const { return m_keyframes.size(); }

    /**
     * @brief Get total duration from first to last keyframe
     * @return Duration in milliseconds
     */
    [[nodiscard]] int64_t getDuration() const;

    /**
     * @brief Check if there are any keyframes
     * @return true if empty
     */
    [[nodiscard]] bool isEmpty() const { return m_keyframes.empty(); }

    /**
     * @brief Get keyframe at index
     * @param index Keyframe index
     * @return Keyframe at index
     */
    [[nodiscard]] const Keyframe& getKeyframeAt(size_t index) const {
        return m_keyframes[index];
    }

    /**
     * @brief Edit keyframe at specific time
     * @param timeMs Time of keyframe to edit
     * @param newValue New value
     * @return true if edited
     */
    bool editKeyframe(int64_t timeMs, float newValue);

    /**
     * @brief Get keyframe indices surrounding a time
     * @param timeMs Time in milliseconds
     * @param outIndex1 Previous keyframe index
     * @param outIndex2 Next keyframe index
     * @return true if two keyframes found (time is between them)
     */
    bool getSurroundingKeyframes(int64_t timeMs, size_t& outIndex1, size_t& outIndex2) const;

private:
    std::vector<Keyframe> m_keyframes;

    // Helper interpolation functions
    [[nodiscard]] float interpolateLinear(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseInQuad(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseOutQuad(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseInOutQuad(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseInCubic(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseOutCubic(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateEaseInOutCubic(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateBezier(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateBounce(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateElastic(const Keyframe& kf1, const Keyframe& kf2, float t) const;
    [[nodiscard]] float interpolateStep(const Keyframe& kf1, const Keyframe& kf2, float t) const;

    // Bezier curve evaluation
    [[nodiscard]] float evaluateBezier(float p0, float p1, float p2, float p3, float t) const;
};

/**
 * @class ParameterAnimator
 * @brief Animates multiple parameters simultaneously
 *
 * Manages animations for different properties (opacity, scale, etc.)
 */
class ParameterAnimator {
public:
    /**
     * @brief Create parameter animator
     */
    ParameterAnimator();

    /**
     * @brief Add animation track for a parameter
     * @param paramName Parameter name (e.g., "opacity", "scaleX", "rotationZ")
     */
    void addParameter(const std::string& paramName);

    /**
     * @brief Add keyframe to parameter
     * @param paramName Parameter name
     * @param timeMs Time in milliseconds
     * @param value Value at this time
     * @param interpolation Interpolation type
     */
    void addKeyframe(const std::string& paramName, int64_t timeMs, float value,
                    InterpolationType interpolation = InterpolationType::LINEAR);

    /**
     * @brief Get animated value
     * @param paramName Parameter name
     * @param timeMs Current time
     * @return Animated value
     */
    [[nodiscard]] float getValue(const std::string& paramName, int64_t timeMs) const;

    /**
     * @brief Get all parameter names
     * @return Vector of parameter names
     */
    [[nodiscard]] std::vector<std::string> getParameterNames() const;

    /**
     * @brief Get total animation duration
     * @return Duration in milliseconds
     */
    [[nodiscard]] int64_t getDuration() const;

    /**
     * @brief Remove parameter animation
     * @param paramName Parameter name
     */
    void removeParameter(const std::string& paramName);

    /**
     * @brief Clear all animations
     */
    void clearAnimations();

private:
    std::unordered_map<std::string, std::unique_ptr<KeyframeInterpolator>> m_animators;
};

} // namespace animation
} // namespace clipforge

#endif // CLIPFORGE_KEYFRAME_INTERPOLATOR_H
