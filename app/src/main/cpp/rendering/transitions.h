#ifndef CLIPFORGE_TRANSITIONS_H
#define CLIPFORGE_TRANSITIONS_H

#include <cstdint>
#include <cmath>
#include <vector>
#include <cstring>

namespace clipforge {
namespace rendering {

/**
 * @enum TransitionType
 * @brief Available transition effects between clips
 */
enum class TransitionType {
    CUT = 0,                    // Instant cut
    FADE = 1,                   // Fade to black/color
    CROSS_FADE = 2,             // Cross dissolve
    SLIDE = 3,                  // Slide transition
    WIPE = 4,                   // Directional wipe
    ZOOM_FADE = 5,              // Zoom with fade
    MOTION_BLUR = 6,            // Motion blur transition
    LIGHT_LEAK = 7,             // Light leak effect
    PUSH = 8,                   // Push transition
    TWIST = 9,                  // Twist effect
    PIXELATE = 10,              // Pixelate dissolve
    PARTICLE = 11,              // Particle effect
};

/**
 * @class TransitionEffect
 * @brief Base class for transition effects
 */
class TransitionEffect {
public:
    virtual ~TransitionEffect() = default;

    /**
     * @brief Apply transition between two frames
     * @param frameA First frame (RGBA)
     * @param frameB Second frame (RGBA)
     * @param outputFrame Output frame (RGBA)
     * @param width Frame width
     * @param height Frame height
     * @param progress Transition progress (0-1)
     */
    virtual void apply(const uint8_t* frameA, const uint8_t* frameB,
                      uint8_t* outputFrame, int width, int height,
                      float progress) = 0;

    /**
     * @brief Get transition duration in milliseconds
     * @return Duration
     */
    virtual int getDurationMs() const { return 300; }
};

/**
 * @class CrossFadeTransition
 * @brief Simple cross-fade between two clips
 */
class CrossFadeTransition : public TransitionEffect {
public:
    explicit CrossFadeTransition(int durationMs = 300) : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;
};

/**
 * @class SlideTransition
 * @brief Slide one clip over another
 */
class SlideTransition : public TransitionEffect {
public:
    enum class Direction { LEFT, RIGHT, UP, DOWN };

    explicit SlideTransition(Direction dir = Direction::LEFT, int durationMs = 300)
        : m_direction(dir), m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    Direction m_direction;
    int m_durationMs;
};

/**
 * @class WipeTransition
 * @brief Directional wipe transition
 */
class WipeTransition : public TransitionEffect {
public:
    enum class Direction { LEFT_TO_RIGHT, RIGHT_TO_LEFT, TOP_TO_BOTTOM, BOTTOM_TO_TOP, DIAGONAL };

    explicit WipeTransition(Direction dir = Direction::LEFT_TO_RIGHT, int durationMs = 300)
        : m_direction(dir), m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    Direction m_direction;
    int m_durationMs;
};

/**
 * @class ZoomFadeTransition
 * @brief Zoom with fade transition
 */
class ZoomFadeTransition : public TransitionEffect {
public:
    explicit ZoomFadeTransition(int durationMs = 300)
        : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;
};

/**
 * @class MotionBlurTransition
 * @brief Motion blur transition effect
 */
class MotionBlurTransition : public TransitionEffect {
public:
    explicit MotionBlurTransition(int durationMs = 300)
        : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;
};

/**
 * @class LightLeakTransition
 * @brief Light leak effect transition
 */
class LightLeakTransition : public TransitionEffect {
public:
    explicit LightLeakTransition(int durationMs = 300)
        : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;

    float perlinNoise(float x, float y, float z) const;
};

/**
 * @class PushTransition
 * @brief Push transition - one clip pushes the other out
 */
class PushTransition : public TransitionEffect {
public:
    enum class Direction { LEFT, RIGHT, UP, DOWN };

    explicit PushTransition(Direction dir = Direction::RIGHT, int durationMs = 300)
        : m_direction(dir), m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    Direction m_direction;
    int m_durationMs;
};

/**
 * @class TwistTransition
 * @brief Twist/spiral transition effect
 */
class TwistTransition : public TransitionEffect {
public:
    explicit TwistTransition(int durationMs = 300)
        : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;
};

/**
 * @class PixelateTransition
 * @brief Pixelate dissolve transition
 */
class PixelateTransition : public TransitionEffect {
public:
    explicit PixelateTransition(int durationMs = 300)
        : m_durationMs(durationMs) {}

    void apply(const uint8_t* frameA, const uint8_t* frameB,
              uint8_t* outputFrame, int width, int height,
              float progress) override;

    int getDurationMs() const override { return m_durationMs; }

private:
    int m_durationMs;
};

/**
 * @class TransitionFactory
 * @brief Factory for creating transition effects
 */
class TransitionFactory {
public:
    /**
     * @brief Create transition effect
     * @param type Transition type
     * @param durationMs Duration in milliseconds
     * @return Unique pointer to transition effect
     */
    static std::unique_ptr<TransitionEffect> createTransition(
        TransitionType type, int durationMs = 300);

    /**
     * @brief Get transition name
     * @param type Transition type
     * @return Human-readable name
     */
    static const char* getTransitionName(TransitionType type);
};

} // namespace rendering
} // namespace clipforge

#endif // CLIPFORGE_TRANSITIONS_H
