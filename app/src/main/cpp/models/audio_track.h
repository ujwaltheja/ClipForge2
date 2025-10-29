#ifndef CLIPFORGE_AUDIO_TRACK_H
#define CLIPFORGE_AUDIO_TRACK_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace clipforge {
namespace models {

/**
 * @enum AudioChannelLayout
 * @brief Audio channel configuration
 */
enum class AudioChannelLayout {
    MONO = 0x0001,                 // 1 channel
    STEREO = 0x0003,               // 2 channels (L+R)
    STEREO_SURROUND = 0x0F,        // 4 channels
    SURROUND_5_1 = 0x3F,           // 6 channels
    SURROUND_7_1 = 0xFF,           // 8 channels
};

/**
 * @enum AudioSampleRate
 * @brief Common audio sample rates
 */
enum class AudioSampleRate {
    RATE_8K = 8000,
    RATE_16K = 16000,
    RATE_22K = 22050,
    RATE_44K = 44100,
    RATE_48K = 48000,
    RATE_96K = 96000,
};

/**
 * @struct AudioMetadata
 * @brief Metadata about an audio track
 */
struct AudioMetadata {
    int32_t sampleRate;            // Samples per second
    int32_t channels;              // Number of channels
    int64_t bitRate;               // Bitrate in bps
    int64_t duration;              // Duration in milliseconds
    float peakLevel;               // Highest volume level (0-1)
    float rmsLevel;                // RMS (average) level (0-1)
    std::string codecName;         // Audio codec (aac, mp3, opus, etc.)

    AudioMetadata() : sampleRate(48000), channels(2), bitRate(128000),
                      duration(0), peakLevel(0.0f), rmsLevel(0.0f) {}
};

/**
 * @class AudioTrack
 * @brief Represents an audio track that can contain one or more audio clips
 *
 * Audio tracks are separate from video clips and can be:
 * - Original video audio
 * - Background music
 * - Voice-over narration
 * - Sound effects
 */
class AudioTrack {
public:
    /**
     * @brief Create an audio track
     * @param id Unique identifier
     * @param name Display name
     * @param type Type of track (main, voiceover, music, sfx)
     */
    explicit AudioTrack(const std::string& id, const std::string& name = "",
                       const std::string& type = "main");

    /**
     * @brief Destructor
     */
    ~AudioTrack() = default;

    /**
     * @brief Copy constructor
     */
    AudioTrack(const AudioTrack& other) = default;

    /**
     * @brief Move constructor
     */
    AudioTrack(AudioTrack&& other) noexcept = default;

    // ===== Identity =====

    /**
     * @brief Get track ID
     * @return Unique identifier
     */
    [[nodiscard]] const std::string& getId() const { return m_id; }

    /**
     * @brief Get track name
     * @return Display name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }

    /**
     * @brief Set track name
     * @param name New name
     */
    void setName(const std::string& name) { m_name = name; }

    /**
     * @brief Get track type
     * @return Type string (main, voiceover, music, sfx)
     */
    [[nodiscard]] const std::string& getType() const { return m_type; }

    // ===== Visibility & State =====

    /**
     * @brief Check if track is enabled
     * @return true if track will be rendered
     */
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    /**
     * @brief Enable or disable track
     * @param enabled true to enable
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    /**
     * @brief Check if track is muted
     * @return true if audio is muted
     */
    [[nodiscard]] bool isMuted() const { return m_muted; }

    /**
     * @brief Mute or unmute track
     * @param muted true to mute
     */
    void setMuted(bool muted) { m_muted = muted; }

    /**
     * @brief Check if track is solo (only this track plays)
     * @return true if solo is active
     */
    [[nodiscard]] bool isSolo() const { return m_solo; }

    /**
     * @brief Set solo mode
     * @param solo true to enable solo
     */
    void setSolo(bool solo) { m_solo = solo; }

    /**
     * @brief Check if track is locked (cannot be modified)
     * @return true if locked
     */
    [[nodiscard]] bool isLocked() const { return m_locked; }

    /**
     * @brief Lock or unlock track
     * @param locked true to lock
     */
    void setLocked(bool locked) { m_locked = locked; }

    // ===== Volume Control =====

    /**
     * @brief Get track volume level
     * @return Volume (0.0 = mute, 1.0 = normal, >1.0 = amplified)
     */
    [[nodiscard]] float getVolume() const { return m_volume; }

    /**
     * @brief Set track volume
     * @param volume Volume multiplier (clamped 0.0-2.0)
     */
    void setVolume(float volume);

    /**
     * @brief Get pan (left-right balance)
     * @return Pan value (-1.0 = full left, 0.0 = center, 1.0 = full right)
     */
    [[nodiscard]] float getPan() const { return m_pan; }

    /**
     * @brief Set pan position
     * @param pan Pan value (clamped -1.0 to 1.0)
     */
    void setPan(float pan);

    // ===== Metadata =====

    /**
     * @brief Get audio metadata
     * @return Reference to metadata
     */
    [[nodiscard]] const AudioMetadata& getMetadata() const { return m_metadata; }

    /**
     * @brief Set audio metadata
     * @param metadata New metadata
     */
    void setMetadata(const AudioMetadata& metadata) { m_metadata = metadata; }

    /**
     * @brief Get source audio file path
     * @return File path or empty string if not from file
     */
    [[nodiscard]] const std::string& getSourceFile() const { return m_sourceFile; }

    /**
     * @brief Set source file path
     * @param filePath Path to audio file
     */
    void setSourceFile(const std::string& filePath) { m_sourceFile = filePath; }

    /**
     * @brief Get duration of this track
     * @return Duration in milliseconds
     */
    [[nodiscard]] int64_t getDuration() const { return m_metadata.duration; }

    // ===== Equalization =====

    /**
     * @brief Get bass level adjustment
     * @return Bass adjustment (-1.0 to 1.0)
     */
    [[nodiscard]] float getBass() const { return m_bass; }

    /**
     * @brief Set bass level
     * @param bass Bass adjustment (clamped -1.0 to 1.0)
     */
    void setBass(float bass);

    /**
     * @brief Get midrange level adjustment
     * @return Mid adjustment (-1.0 to 1.0)
     */
    [[nodiscard]] float getMidrange() const { return m_midrange; }

    /**
     * @brief Set midrange level
     * @param mid Mid adjustment (clamped -1.0 to 1.0)
     */
    void setMidrange(float mid);

    /**
     * @brief Get treble level adjustment
     * @return Treble adjustment (-1.0 to 1.0)
     */
    [[nodiscard]] float getTreble() const { return m_treble; }

    /**
     * @brief Set treble level
     * @param treble Treble adjustment (clamped -1.0 to 1.0)
     */
    void setTreble(float treble);

    // ===== Advanced Effects =====

    /**
     * @brief Get reverb amount
     * @return Reverb level (0.0 = none, 1.0 = full)
     */
    [[nodiscard]] float getReverb() const { return m_reverb; }

    /**
     * @brief Set reverb amount
     * @param amount Reverb level (clamped 0.0-1.0)
     */
    void setReverb(float amount);

    /**
     * @brief Get compression ratio
     * @return Compression ratio (1.0 = no compression, 4.0 = moderate)
     */
    [[nodiscard]] float getCompression() const { return m_compression; }

    /**
     * @brief Set compression
     * @param ratio Compression ratio (clamped 1.0-8.0)
     */
    void setCompression(float ratio);

    /**
     * @brief Get pitch shift amount
     * @return Pitch shift in semitones (-12 to 12)
     */
    [[nodiscard]] float getPitchShift() const { return m_pitchShift; }

    /**
     * @brief Set pitch shift
     * @param semitones Semitones to shift (clamped -12 to 12)
     */
    void setPitchShift(float semitones);

    // ===== Timestamps =====

    /**
     * @brief Get creation timestamp
     * @return Milliseconds since epoch
     */
    [[nodiscard]] int64_t getCreatedAt() const { return m_createdAt; }

    /**
     * @brief Get modification timestamp
     * @return Milliseconds since epoch
     */
    [[nodiscard]] int64_t getModifiedAt() const { return m_modifiedAt; }

    /**
     * @brief Mark track as modified
     */
    void markModified();

    /**
     * @brief Check if track has unsaved changes
     * @return true if modified
     */
    [[nodiscard]] bool hasChanges() const { return m_modified; }

    /**
     * @brief Clear modified flag
     */
    void clearChangesFlag() { m_modified = false; }

private:
    std::string m_id;
    std::string m_name;
    std::string m_type;            // "main", "voiceover", "music", "sfx"
    std::string m_sourceFile;      // Source audio file path

    // State
    bool m_enabled = true;
    bool m_muted = false;
    bool m_solo = false;
    bool m_locked = false;

    // Audio properties
    float m_volume = 1.0f;         // Volume multiplier
    float m_pan = 0.0f;            // Left-right balance
    float m_bass = 0.0f;           // Bass EQ
    float m_midrange = 0.0f;       // Mid EQ
    float m_treble = 0.0f;         // Treble EQ

    // Effects
    float m_reverb = 0.0f;         // Reverb amount
    float m_compression = 1.0f;    // Compression ratio
    float m_pitchShift = 0.0f;     // Pitch shift in semitones

    // Metadata
    AudioMetadata m_metadata;

    // Timestamps
    int64_t m_createdAt;
    int64_t m_modifiedAt;
    bool m_modified = true;

    /**
     * @brief Get current timestamp
     * @return Milliseconds since epoch
     */
    static int64_t getCurrentTimestamp();

    /**
     * @brief Clamp a value to a range
     * @param value Value to clamp
     * @param min Minimum value
     * @param max Maximum value
     * @return Clamped value
     */
    static float clamp(float value, float min, float max);
};

} // namespace models
} // namespace clipforge

#endif // CLIPFORGE_AUDIO_TRACK_H
