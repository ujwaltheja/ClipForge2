#ifndef CLIPFORGE_TIME_EFFECTS_H
#define CLIPFORGE_TIME_EFFECTS_H

#include <vector>
#include <memory>
#include <cstdint>
#include <cmath>
#include "../animation/keyframe_interpolator.h"

namespace clipforge {
namespace time_effects {

/**
 * @enum TimeEffectType
 * @brief Available time manipulation effects
 */
enum class TimeEffectType {
    SPEED_RAMP = 0,          // Smooth speed transitions
    SLOW_MOTION = 1,         // Constant slow motion
    FAST_FORWARD = 2,        // Constant speed up
    FREEZE_FRAME = 3,        // Pause on frame
    REVERSE = 4,             // Play backwards
    TIME_STRETCH = 5,        // Non-uniform time scaling
};

/**
 * @struct TimeSpeedPoint
 * @brief Speed control point for ramping
 */
struct TimeSpeedPoint {
    int64_t timeMs;          // Time in milliseconds
    float speed;             // Playback speed (1.0 = normal, 0.5 = half speed, 2.0 = double speed)
};

/**
 * @class SpeedRamper
 * @brief Handles speed ramping and time effects
 *
 * Enables smooth speed transitions like slow-mo to normal speed,
 * speed ramping effects, and complex time manipulations.
 */
class SpeedRamper {
public:
    /**
     * @brief Create speed ramper
     */
    SpeedRamper();

    /**
     * @brief Add speed control point
     * @param timeMs Time in milliseconds
     * @param speed Playback speed (1.0 = normal)
     */
    void addSpeedPoint(int64_t timeMs, float speed);

    /**
     * @brief Set constant speed throughout clip
     * @param speed Playback speed
     */
    void setConstantSpeed(float speed);

    /**
     * @brief Create linear speed ramp
     * @param startTimeMs Start time
     * @param endTimeMs End time
     * @param startSpeed Speed at start
     * @param endSpeed Speed at end
     */
    void createLinearRamp(int64_t startTimeMs, int64_t endTimeMs,
                         float startSpeed, float endSpeed);

    /**
     * @brief Create slow-motion segment
     * @param startTimeMs Start time
     * @param endTimeMs End time
     * @param slowMotionFactor Slow motion factor (0.5 = half speed)
     */
    void createSlowMotionSegment(int64_t startTimeMs, int64_t endTimeMs,
                                float slowMotionFactor);

    /**
     * @brief Create freeze frame
     * @param timeMs Time to freeze at
     * @param durationMs Duration to freeze for
     */
    void createFreezeFrame(int64_t timeMs, int64_t durationMs);

    /**
     * @brief Get playback speed at specific time
     * @param timeMs Time in milliseconds
     * @return Playback speed (1.0 = normal)
     */
    [[nodiscard]] float getSpeedAtTime(int64_t timeMs) const;

    /**
     * @brief Convert display time to source time
     * @param displayTimeMs Time in edited timeline
     * @return Corresponding time in source clip
     */
    [[nodiscard]] int64_t displayToSourceTime(int64_t displayTimeMs) const;

    /**
     * @brief Convert source time to display time
     * @param sourceTimeMs Time in source clip
     * @return Corresponding time in edited timeline
     */
    [[nodiscard]] int64_t sourceToDisplayTime(int64_t sourceTimeMs) const;

    /**
     * @brief Get duration scaling factor
     * @return How much the clip duration changes (e.g., 0.5 for double speed)
     */
    [[nodiscard]] float getDurationScaleFactor() const;

    /**
     * @brief Remove all speed points
     */
    void clear();

    /**
     * @brief Get all speed control points
     * @return Vector of speed points
     */
    [[nodiscard]] const std::vector<TimeSpeedPoint>& getSpeedPoints() const {
        return m_speedPoints;
    }

private:
    std::vector<TimeSpeedPoint> m_speedPoints;

    [[nodiscard]] float interpolateSpeed(int64_t timeMs) const;
};

/**
 * @class TimeSampler
 * @brief Samples frames at non-uniform time intervals
 *
 * Used for creating variable-speed playback with frame sampling
 */
class TimeSampler {
public:
    /**
     * @brief Create time sampler
     */
    TimeSampler();

    /**
     * @brief Generate sample times for variable speed playback
     * @param sourceDurationMs Duration of source clip
     * @param outputDurationMs Duration of output after time effects
     * @param speedRamper Speed ramp to apply
     * @return Vector of sample times in milliseconds
     */
    [[nodiscard]] std::vector<int64_t> generateSampleTimes(
        int64_t sourceDurationMs, int64_t outputDurationMs,
        const SpeedRamper& speedRamper);

    /**
     * @brief Sample frame at specific time using interpolation
     * @param frameData Output frame buffer (RGBA)
     * @param width Frame width
     * @param height Frame height
     * @param sourceTime Time to sample from
     * @param prevFrame Previous frame for blending
     * @param nextFrame Next frame for blending
     * @param blendFactor Blend factor between frames (0-1)
     */
    static void sampleFrame(uint8_t* frameData, int width, int height,
                           int64_t sourceTime,
                           const uint8_t* prevFrame, const uint8_t* nextFrame,
                           float blendFactor);

    /**
     * @brief Blend two frames for smooth playback
     * @param blendBuffer Output buffer
     * @param width Frame width
     * @param height Frame height
     * @param frame1 First frame
     * @param frame2 Second frame
     * @param blendFactor Blend factor (0 = frame1, 1 = frame2)
     */
    static void blendFrames(uint8_t* blendBuffer, int width, int height,
                           const uint8_t* frame1, const uint8_t* frame2,
                           float blendFactor);
};

/**
 * @class MotionBlur
 * @brief Motion blur effect for fast-moving subjects
 */
class MotionBlur {
public:
    /**
     * @brief Create motion blur
     */
    MotionBlur();

    /**
     * @brief Set motion blur intensity
     * @param intensity Intensity (0-1)
     */
    void setIntensity(float intensity) { m_intensity = intensity; }

    /**
     * @brief Apply motion blur based on speed
     * @param frameData Current frame
     * @param width Frame width
     * @param height Frame height
     * @param speed Playback speed
     */
    void apply(uint8_t* frameData, int width, int height, float speed);

private:
    float m_intensity = 0.5f;
};

/**
 * @class FrameBlending
 * @brief Frame blending for smooth slow-motion
 */
class FrameBlending {
public:
    /**
     * @brief Create frame blender
     */
    FrameBlending();

    /**
     * @brief Blend frames for smooth playback
     * @param frames Vector of frame buffers
     * @param blendCount Number of frames to blend
     * @param outputFrame Output frame buffer
     * @param width Frame width
     * @param height Frame height
     */
    static void blendMultipleFrames(const std::vector<uint8_t*>& frames,
                                    int blendCount,
                                    uint8_t* outputFrame,
                                    int width, int height);

    /**
     * @brief Apply optical flow-based frame interpolation
     * @param frame1 First frame
     * @param frame2 Second frame
     * @param interpolatedFrame Interpolated frame
     * @param width Frame width
     * @param height Frame height
     */
    static void opticalFlowInterpolate(const uint8_t* frame1,
                                       const uint8_t* frame2,
                                       uint8_t* interpolatedFrame,
                                       int width, int height);
};

/**
 * @class TimeController
 * @brief Master controller for time effects on clip
 */
class TimeController {
public:
    /**
     * @brief Create time controller
     * @param clipDurationMs Duration of the clip in milliseconds
     */
    explicit TimeController(int64_t clipDurationMs);

    /**
     * @brief Get current playback speed
     * @return Speed multiplier
     */
    [[nodiscard]] float getCurrentSpeed() const { return m_speed; }

    /**
     * @brief Set playback speed
     * @param speed Speed multiplier (0.25 = quarter speed, 2.0 = double speed)
     */
    void setSpeed(float speed);

    /**
     * @brief Enable slow-motion mode
     * @param factor Slow-motion factor (0.5 = half speed)
     */
    void setSlowMotion(float factor);

    /**
     * @brief Enable reverse playback
     * @param enabled true to play in reverse
     */
    void setReverse(bool enabled);

    /**
     * @brief Get duration after time effects
     * @return Effective duration in milliseconds
     */
    [[nodiscard]] int64_t getEffectiveDuration() const;

    /**
     * @brief Map clip time to timeline position
     * @param clipTimeMs Time in original clip
     * @return Time in edited timeline
     */
    [[nodiscard]] int64_t clipToTimelineTime(int64_t clipTimeMs) const;

    /**
     * @brief Map timeline position to clip time
     * @param timelineTimeMs Time in edited timeline
     * @return Time in original clip
     */
    [[nodiscard]] int64_t timelineToClipTime(int64_t timelineTimeMs) const;

    /**
     * @brief Access speed ramper for advanced control
     * @return Reference to speed ramper
     */
    SpeedRamper& getSpeedRamper() { return m_speedRamper; }

private:
    int64_t m_clipDurationMs;
    float m_speed = 1.0f;
    bool m_reverse = false;
    SpeedRamper m_speedRamper;
};

} // namespace time_effects
} // namespace clipforge

#endif // CLIPFORGE_TIME_EFFECTS_H
