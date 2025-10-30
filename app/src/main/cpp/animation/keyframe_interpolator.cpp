#include "keyframe_interpolator.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace clipforge {
namespace animation {

KeyframeInterpolator::KeyframeInterpolator() = default;

void KeyframeInterpolator::addKeyframe(int64_t timeMs, float value,
                                      InterpolationType interpolation) {
    Keyframe kf;
    kf.timeMs = timeMs;
    kf.value = value;
    kf.interpolation = interpolation;

    // Insert in sorted order
    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), kf,
        [](const Keyframe& a, const Keyframe& b) { return a.timeMs < b.timeMs; });

    m_keyframes.insert(it, kf);
}

void KeyframeInterpolator::addBezierKeyframe(int64_t timeMs, float value,
                                            float inTangentX, float inTangentY,
                                            float outTangentX, float outTangentY) {
    Keyframe kf;
    kf.timeMs = timeMs;
    kf.value = value;
    kf.interpolation = InterpolationType::CUBIC;
    kf.inTangentX = inTangentX;
    kf.inTangentY = inTangentY;
    kf.outTangentX = outTangentX;
    kf.outTangentY = outTangentY;

    auto it = std::lower_bound(m_keyframes.begin(), m_keyframes.end(), kf,
        [](const Keyframe& a, const Keyframe& b) { return a.timeMs < b.timeMs; });

    m_keyframes.insert(it, kf);
}

bool KeyframeInterpolator::removeKeyframe(int64_t timeMs) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(),
        [timeMs](const Keyframe& kf) { return kf.timeMs == timeMs; });

    if (it != m_keyframes.end()) {
        m_keyframes.erase(it);
        return true;
    }
    return false;
}

void KeyframeInterpolator::clearKeyframes() {
    m_keyframes.clear();
}

float KeyframeInterpolator::getValue(int64_t timeMs) const {
    if (m_keyframes.empty()) return 0.0f;

    // If time is before first keyframe, return first value
    if (timeMs <= m_keyframes.front().timeMs) {
        return m_keyframes.front().value;
    }

    // If time is after last keyframe, return last value
    if (timeMs >= m_keyframes.back().timeMs) {
        return m_keyframes.back().value;
    }

    // Find surrounding keyframes
    size_t idx1, idx2;
    if (!getSurroundingKeyframes(timeMs, idx1, idx2)) {
        return m_keyframes.back().value;
    }

    const Keyframe& kf1 = m_keyframes[idx1];
    const Keyframe& kf2 = m_keyframes[idx2];

    // Calculate normalized time (0-1) between keyframes
    int64_t timeDiff = kf2.timeMs - kf1.timeMs;
    float t = timeDiff > 0 ? static_cast<float>(timeMs - kf1.timeMs) / timeDiff : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);

    // Interpolate based on type
    switch (kf1.interpolation) {
        case InterpolationType::LINEAR:
            return interpolateLinear(kf1, kf2, t);

        case InterpolationType::EASE_IN:
        case InterpolationType::EASE_IN_QUAD:
            return interpolateEaseInQuad(kf1, kf2, t);

        case InterpolationType::EASE_OUT:
        case InterpolationType::EASE_OUT_QUAD:
            return interpolateEaseOutQuad(kf1, kf2, t);

        case InterpolationType::EASE_IN_OUT:
        case InterpolationType::EASE_IN_OUT_QUAD:
            return interpolateEaseInOutQuad(kf1, kf2, t);

        case InterpolationType::EASE_IN_CUBIC:
            return interpolateEaseInCubic(kf1, kf2, t);

        case InterpolationType::EASE_OUT_CUBIC:
            return interpolateEaseOutCubic(kf1, kf2, t);

        case InterpolationType::EASE_IN_OUT_CUBIC:
            return interpolateEaseInOutCubic(kf1, kf2, t);

        case InterpolationType::CUBIC:
            return interpolateBezier(kf1, kf2, t);

        case InterpolationType::BOUNCE:
            return interpolateBounce(kf1, kf2, t);

        case InterpolationType::ELASTIC:
            return interpolateElastic(kf1, kf2, t);

        case InterpolationType::STEP:
            return interpolateStep(kf1, kf2, t);

        default:
            return interpolateLinear(kf1, kf2, t);
    }
}

float KeyframeInterpolator::getValueClamped(int64_t timeMs, float clampMin, float clampMax) const {
    float value = getValue(timeMs);
    return std::clamp(value, clampMin, clampMax);
}

float KeyframeInterpolator::getVelocity(int64_t timeMs) const {
    // Calculate derivative using finite differences
    const float delta = 1.0f; // 1ms delta
    float v1 = getValue(timeMs - static_cast<int64_t>(delta));
    float v2 = getValue(timeMs + static_cast<int64_t>(delta));
    return (v2 - v1) / (2.0f * delta);
}

int64_t KeyframeInterpolator::getDuration() const {
    if (m_keyframes.empty()) return 0;
    return m_keyframes.back().timeMs - m_keyframes.front().timeMs;
}

bool KeyframeInterpolator::editKeyframe(int64_t timeMs, float newValue) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(),
        [timeMs](const Keyframe& kf) { return kf.timeMs == timeMs; });

    if (it != m_keyframes.end()) {
        it->value = newValue;
        return true;
    }
    return false;
}

bool KeyframeInterpolator::getSurroundingKeyframes(int64_t timeMs, size_t& outIndex1,
                                                  size_t& outIndex2) const {
    if (m_keyframes.size() < 2) return false;

    for (size_t i = 0; i < m_keyframes.size() - 1; i++) {
        if (m_keyframes[i].timeMs <= timeMs && timeMs <= m_keyframes[i + 1].timeMs) {
            outIndex1 = i;
            outIndex2 = i + 1;
            return true;
        }
    }
    return false;
}

// Interpolation implementations
float KeyframeInterpolator::interpolateLinear(const Keyframe& kf1, const Keyframe& kf2,
                                             float t) const {
    return kf1.value + (kf2.value - kf1.value) * t;
}

float KeyframeInterpolator::interpolateEaseInQuad(const Keyframe& kf1, const Keyframe& kf2,
                                                 float t) const {
    float eased = t * t;
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateEaseOutQuad(const Keyframe& kf1, const Keyframe& kf2,
                                                  float t) const {
    float eased = 1.0f - (1.0f - t) * (1.0f - t);
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateEaseInOutQuad(const Keyframe& kf1, const Keyframe& kf2,
                                                    float t) const {
    float eased = t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateEaseInCubic(const Keyframe& kf1, const Keyframe& kf2,
                                                  float t) const {
    float eased = t * t * t;
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateEaseOutCubic(const Keyframe& kf1, const Keyframe& kf2,
                                                   float t) const {
    float eased = 1.0f - std::pow(1.0f - t, 3.0f);
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateEaseInOutCubic(const Keyframe& kf1, const Keyframe& kf2,
                                                     float t) const {
    float eased = t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateBezier(const Keyframe& kf1, const Keyframe& kf2,
                                             float t) const {
    // Use control points from keyframe tangents
    float p0 = kf1.value;
    float p1 = kf1.value + kf1.outTangentY;
    float p2 = kf2.value + kf2.inTangentY;
    float p3 = kf2.value;

    return evaluateBezier(p0, p1, p2, p3, t);
}

float KeyframeInterpolator::interpolateBounce(const Keyframe& kf1, const Keyframe& kf2,
                                             float t) const {
    // Bounce effect - elastic with decay
    float c4 = (2.0f * 3.14159f) / 3.0f;

    float eased;
    if (t == 0.0f) {
        eased = 0.0f;
    } else if (t == 1.0f) {
        eased = 1.0f;
    } else if (t < 0.5f) {
        eased = -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4)) / 2.0f;
    } else {
        eased = (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((t * 10.0f - 10.75f) * c4)) / 2.0f + 1.0f;
    }

    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateElastic(const Keyframe& kf1, const Keyframe& kf2,
                                              float t) const {
    // Elastic effect
    float c4 = (2.0f * 3.14159f) / 3.0f;

    float eased;
    if (t == 0.0f) {
        eased = 0.0f;
    } else if (t == 1.0f) {
        eased = 1.0f;
    } else {
        eased = -(std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4));
    }

    return kf1.value + (kf2.value - kf1.value) * eased;
}

float KeyframeInterpolator::interpolateStep(const Keyframe& kf1, const Keyframe& kf2,
                                           float t) const {
    return t >= 1.0f ? kf2.value : kf1.value;
}

float KeyframeInterpolator::evaluateBezier(float p0, float p1, float p2, float p3,
                                          float t) const {
    float mt = 1.0f - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;
    float t2 = t * t;
    float t3 = t2 * t;

    return mt3 * p0 + 3.0f * mt2 * t * p1 + 3.0f * mt * t2 * p2 + t3 * p3;
}

// ===== ParameterAnimator =====

ParameterAnimator::ParameterAnimator() = default;

void ParameterAnimator::addParameter(const std::string& paramName) {
    if (m_animators.find(paramName) == m_animators.end()) {
        m_animators[paramName] = std::make_unique<KeyframeInterpolator>();
    }
}

void ParameterAnimator::addKeyframe(const std::string& paramName, int64_t timeMs, float value,
                                   InterpolationType interpolation) {
    auto it = m_animators.find(paramName);
    if (it != m_animators.end()) {
        it->second->addKeyframe(timeMs, value, interpolation);
    }
}

float ParameterAnimator::getValue(const std::string& paramName, int64_t timeMs) const {
    auto it = m_animators.find(paramName);
    if (it != m_animators.end()) {
        return it->second->getValue(timeMs);
    }
    return 0.0f;
}

std::vector<std::string> ParameterAnimator::getParameterNames() const {
    std::vector<std::string> names;
    for (const auto& [name, _] : m_animators) {
        names.push_back(name);
    }
    return names;
}

int64_t ParameterAnimator::getDuration() const {
    int64_t maxDuration = 0;
    for (const auto& [_, animator] : m_animators) {
        maxDuration = std::max(maxDuration, animator->getDuration());
    }
    return maxDuration;
}

void ParameterAnimator::removeParameter(const std::string& paramName) {
    m_animators.erase(paramName);
}

void ParameterAnimator::clearAnimations() {
    m_animators.clear();
}

} // namespace animation
} // namespace clipforge
