package com.ucworks.clipforge.models

/**
 * Represents an audio track in the timeline.
 */
data class AudioTrack(
    val id: String,
    val name: String,
    val type: AudioTrackType,  // MUSIC, VOICEOVER, SFX, AMBIENT
    val volume: Float = 1.0f,  // 0.0 - 2.0
    val isMuted: Boolean = false,
    val isSolo: Boolean = false,
    val pan: Float = 0.0f,  // -1.0 (left) to 1.0 (right)
    val durationMs: Long = 0L,
    val audioSource: String? = null,  // file path

    // EQ Settings
    val bassGain: Float = 0f,       // -12 to +12 dB
    val midGain: Float = 0f,        // -12 to +12 dB
    val trebleGain: Float = 0f,     // -12 to +12 dB

    // Effects
    val reverbIntensity: Float = 0f,       // 0-100
    val compressionRatio: Float = 0f,      // 0-100
    val pitchShift: Float = 0f,            // -24 to +24 semitones

    // Automation (for future use)
    val volumeAutomation: List<VolumeKeyframe> = emptyList()
) {
    val volumeDbString: String
        get() {
            val db = 20 * kotlin.math.log10(volume.coerceIn(0.01f, 2.0f))
            return String.format("%+.1f dB", db)
        }

    val panString: String
        get() = when {
            pan < -0.1f -> String.format("L %.0f%%", -pan * 100)
            pan > 0.1f -> String.format("R %.0f%%", pan * 100)
            else -> "Center"
        }
}

enum class AudioTrackType {
    MUSIC,
    VOICEOVER,
    SFX,
    AMBIENT
}

/**
 * Represents a volume automation keyframe for audio tracks.
 */
data class VolumeKeyframe(
    val timeMs: Long,
    val volume: Float
)

/**
 * Represents audio mixing preset configurations.
 */
object AudioPresets {
    val voiceoverPreset = AudioTrack(
        id = "vo_${System.currentTimeMillis()}",
        name = "Voiceover",
        type = AudioTrackType.VOICEOVER,
        volume = 1.0f,
        trebleGain = 3f,  // Boost treble for clarity
        compressionRatio = 50f  // Light compression
    )

    val musicPreset = AudioTrack(
        id = "music_${System.currentTimeMillis()}",
        name = "Music",
        type = AudioTrackType.MUSIC,
        volume = 0.7f,
        reverbIntensity = 20f
    )

    val sfxPreset = AudioTrack(
        id = "sfx_${System.currentTimeMillis()}",
        name = "Sound Effects",
        type = AudioTrackType.SFX,
        volume = 1.0f,
        compressionRatio = 30f
    )

    val ambientPreset = AudioTrack(
        id = "ambient_${System.currentTimeMillis()}",
        name = "Ambient",
        type = AudioTrackType.AMBIENT,
        volume = 0.3f,
        reverbIntensity = 60f
    )

    fun getPreset(type: AudioTrackType): AudioTrack {
        return when (type) {
            AudioTrackType.VOICEOVER -> voiceoverPreset
            AudioTrackType.MUSIC -> musicPreset
            AudioTrackType.SFX -> sfxPreset
            AudioTrackType.AMBIENT -> ambientPreset
        }
    }
}
