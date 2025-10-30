package com.ucworks.clipforge.data.models

/**
 * Represents an animation keyframe
 */
data class Keyframe(
    val timeMs: Long,
    val value: Float,
    val interpolationType: InterpolationType = InterpolationType.LINEAR,
    val inTangentX: Float = 0f,
    val inTangentY: Float = 0f,
    val outTangentX: Float = 0f,
    val outTangentY: Float = 0f
)

enum class InterpolationType {
    LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,
    CUBIC,
    EASE_IN_QUAD,
    EASE_OUT_QUAD,
    EASE_IN_OUT_QUAD,
    EASE_IN_CUBIC,
    EASE_OUT_CUBIC,
    EASE_IN_OUT_CUBIC,
    BOUNCE,
    ELASTIC,
    STEP
}

/**
 * Manages keyframe-based animations for clip properties
 */
class KeyframeAnimation(
    val propertyName: String
) {
    private val keyframes = mutableListOf<Keyframe>()

    fun addKeyframe(keyframe: Keyframe) {
        keyframes.add(keyframe)
        keyframes.sortBy { it.timeMs }
    }

    fun removeKeyframe(timeMs: Long) {
        keyframes.removeAll { it.timeMs == timeMs }
    }

    fun getValueAtTime(timeMs: Long): Float {
        if (keyframes.isEmpty()) return 0f

        val kf1 = keyframes.lastOrNull { it.timeMs <= timeMs }
            ?: return keyframes.first().value

        val kf2 = keyframes.firstOrNull { it.timeMs > timeMs }
            ?: return kf1.value

        // Interpolate between keyframes
        val timeDiff = kf2.timeMs - kf1.timeMs
        if (timeDiff <= 0) return kf1.value

        val t = (timeMs - kf1.timeMs).toFloat() / timeDiff
        return interpolate(kf1, kf2, t)
    }

    fun getDuration(): Long {
        if (keyframes.isEmpty()) return 0
        return keyframes.last().timeMs - keyframes.first().timeMs
    }

    fun getKeyframes(): List<Keyframe> = keyframes.toList()

    private fun interpolate(kf1: Keyframe, kf2: Keyframe, t: Float): Float {
        val t = t.coerceIn(0f, 1f)

        return when (kf1.interpolationType) {
            InterpolationType.LINEAR -> kf1.value + (kf2.value - kf1.value) * t
            InterpolationType.EASE_IN -> {
                val eased = t * t
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_OUT -> {
                val eased = 1f - (1f - t) * (1f - t)
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_IN_OUT -> {
                val eased = if (t < 0.5f) {
                    2f * t * t
                } else {
                    1f - (-2f * t + 2f).let { it * it } / 2f
                }
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_IN_QUAD -> {
                val eased = t * t
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_OUT_QUAD -> {
                val eased = 1f - (1f - t) * (1f - t)
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_IN_OUT_QUAD -> {
                val eased = if (t < 0.5f) {
                    2f * t * t
                } else {
                    1f - (-2f * t + 2f).let { it * it } / 2f
                }
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_IN_CUBIC -> {
                val eased = t * t * t
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_OUT_CUBIC -> {
                val eased = 1f - (1f - t).let { it * it * it }
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.EASE_IN_OUT_CUBIC -> {
                val eased = if (t < 0.5f) {
                    4f * t * t * t
                } else {
                    1f - (-2f * t + 2f).let { it * it * it } / 2f
                }
                kf1.value + (kf2.value - kf1.value) * eased
            }
            InterpolationType.CUBIC -> {
                // Bezier interpolation
                val p0 = kf1.value
                val p1 = kf1.value + kf1.outTangentY
                val p2 = kf2.value + kf2.inTangentY
                val p3 = kf2.value

                val mt = 1f - t
                val mt2 = mt * mt
                val mt3 = mt2 * mt
                val t2 = t * t
                val t3 = t2 * t

                mt3 * p0 + 3f * mt2 * t * p1 + 3f * mt * t2 * p2 + t3 * p3
            }
            else -> kf1.value + (kf2.value - kf1.value) * t
        }
    }
}

/**
 * Manages multiple parameter animations
 */
class AnimationController {
    private val animations = mutableMapOf<String, KeyframeAnimation>()

    fun createAnimation(propertyName: String): KeyframeAnimation {
        return KeyframeAnimation(propertyName).also {
            animations[propertyName] = it
        }
    }

    fun getAnimation(propertyName: String): KeyframeAnimation? {
        return animations[propertyName]
    }

    fun removeAnimation(propertyName: String) {
        animations.remove(propertyName)
    }

    fun getPropertyValue(propertyName: String, timeMs: Long): Float {
        return animations[propertyName]?.getValueAtTime(timeMs) ?: 0f
    }

    fun getDuration(): Long {
        return animations.values.maxOfOrNull { it.getDuration() } ?: 0L
    }

    fun getAllAnimations(): Map<String, KeyframeAnimation> = animations.toMap()
}
