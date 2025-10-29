package com.ucworks.clipforge.models

/**
 * Represents a video clip on the timeline.
 */
data class VideoClip(
    val id: String,
    val sourceUri: String,        // Original video source
    val trackIndex: Int,          // Which video track
    val positionMs: Long,         // Start time on timeline
    val durationMs: Long,         // Duration (after trimming/speed adjustment)
    val originalDurationMs: Long, // Original source duration
    val volume: Float = 1.0f,     // 0.0 - 2.0
    val speed: Float = 1.0f,      // 0.25x - 4.0x
    val opacity: Float = 1.0f,    // 0.0 - 1.0
    val rotation: Float = 0f,     // 0, 90, 180, 270 degrees
    val isMuted: Boolean = false,
    val thumbnail: String? = null, // path to thumbnail
    val effects: List<String> = emptyList(), // effect IDs applied
    val trimStart: Long = 0L,     // Trim start in milliseconds
    val trimEnd: Long = 0L,       // Trim end in milliseconds
    val textOverlay: String? = null,
    val colorGrade: ColorGrade? = null
) {
    val endPositionMs: Long
        get() = positionMs + durationMs

    val displayName: String
        get() = sourceUri.substringAfterLast("/").substringBeforeLast(".")

    val speedString: String
        get() = String.format("%.2fx", speed)

    val formattedPosition: String
        get() {
            val totalSeconds = positionMs / 1000
            val minutes = totalSeconds / 60
            val seconds = totalSeconds % 60
            return String.format("%02d:%02d", minutes, seconds)
        }

    val formattedDuration: String
        get() {
            val totalSeconds = durationMs / 1000
            val minutes = totalSeconds / 60
            val seconds = totalSeconds % 60
            return String.format("%02d:%02d", minutes, seconds)
        }

    /**
     * Checks if this clip overlaps with another clip on the same track.
     */
    fun overlapsWithClip(other: VideoClip): Boolean {
        if (this.trackIndex != other.trackIndex) return false
        return this.positionMs < other.endPositionMs && this.endPositionMs > other.positionMs
    }

    /**
     * Returns the actual playback duration considering speed.
     */
    fun getPlaybackDuration(): Long {
        return (durationMs / speed).toLong()
    }
}

/**
 * Represents color grading settings for a clip.
 */
data class ColorGrade(
    val brightness: Float = 0f,    // -100 to +100
    val contrast: Float = 0f,      // -100 to +100
    val saturation: Float = 0f,    // -100 to +100
    val hueShift: Float = 0f,      // -180 to +180 degrees
    val highlights: Float = 0f,    // -100 to +100
    val shadows: Float = 0f,       // -100 to +100
    val temperature: Float = 0f,   // -100 (cool) to +100 (warm)
    val tint: Float = 0f,          // -100 (magenta) to +100 (green)
    val vibrance: Float = 0f,      // -100 to +100 (affects less saturated colors)
    val luts: List<String> = emptyList() // LUT file paths
) {
    val isModified: Boolean
        get() = brightness != 0f || contrast != 0f || saturation != 0f ||
                hueShift != 0f || highlights != 0f || shadows != 0f ||
                temperature != 0f || tint != 0f || vibrance != 0f ||
                luts.isNotEmpty()
}

/**
 * Represents a clip transition (between clips on timeline).
 */
data class Transition(
    val id: String,
    val name: String,
    val type: TransitionType,
    val durationMs: Long = 300,
    val easing: EasingType = EasingType.EASE_IN_OUT,
    val parameters: Map<String, Float> = emptyMap()
)

enum class TransitionType {
    CUT,
    FADE,
    SLIDE,
    WIPE,
    DISSOLVE,
    CROSS_FADE
}

enum class EasingType {
    LINEAR,
    EASE_IN,
    EASE_OUT,
    EASE_IN_OUT,
    BOUNCE,
    ELASTIC
}

/**
 * Pre-defined transitions library.
 */
object TransitionLibrary {
    val transitions = listOf(
        Transition("cut", "Cut", TransitionType.CUT, 0),
        Transition("fade", "Fade", TransitionType.FADE, 300),
        Transition("slide_left", "Slide Left", TransitionType.SLIDE, 300),
        Transition("slide_right", "Slide Right", TransitionType.SLIDE, 300),
        Transition("wipe_up", "Wipe Up", TransitionType.WIPE, 300),
        Transition("dissolve", "Dissolve", TransitionType.DISSOLVE, 300),
        Transition("cross_fade", "Cross Fade", TransitionType.CROSS_FADE, 300)
    )

    fun getById(id: String): Transition? = transitions.find { it.id == id }
}
