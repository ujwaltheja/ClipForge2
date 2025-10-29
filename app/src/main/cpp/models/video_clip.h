#ifndef CLIPFORGE_VIDEO_CLIP_H
#define CLIPFORGE_VIDEO_CLIP_H

#include <string>
#include <memory>
#include <vector>
#include "effect.h"

namespace clipforge {
namespace models {

/**
 * @struct VideoClipMetadata
 * @brief Metadata about a video clip's source file
 */
struct VideoClipMetadata {
    std::string sourceFile;        // Original file path
    int64_t fileSize;              // File size in bytes
    int64_t duration;              // Total duration in milliseconds
    int32_t width;                 // Video width in pixels
    int32_t height;                // Video height in pixels
    float frameRate;               // Frames per second
    std::string codecName;         // Video codec (h264, h265, vp9, etc.)
    int64_t bitRate;               // Video bitrate in bps
    std::string mimeType;          // MIME type

    VideoClipMetadata() : fileSize(0), duration(0), width(0), height(0),
                          frameRate(30.0f), bitRate(0) {}
};

/**
 * @class VideoClip
 * @brief Represents a single video clip in the timeline
 *
 * A clip is a segment of video with applied effects, audio,
 * and timing information. Clips can be arranged on a timeline
 * across multiple tracks.
 */
class VideoClip {
public:
    /**
     * @brief Construct a video clip
     * @param id Unique identifier for this clip
     * @param sourceFile Path to the video file
     */
    explicit VideoClip(const std::string& id, const std::string& sourceFile);

    /**
     * @brief Copy constructor
     */
    VideoClip(const VideoClip& other) = default;

    /**
     * @brief Move constructor
     */
    VideoClip(VideoClip&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    ~VideoClip() = default;

    // ===== Identity =====

    /**
     * @brief Get the unique clip ID
     * @return Clip identifier
     */
    [[nodiscard]] const std::string& getId() const { return m_id; }

    /**
     * @brief Get the clip name (display name)
     * @return Human-readable clip name
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }

    /**
     * @brief Set the clip name
     * @param name New display name
     */
    void setName(const std::string& name) { m_name = name; }

    /**
     * @brief Get source file path
     * @return Path to original video file
     */
    [[nodiscard]] const std::string& getSourceFile() const { return m_metadata.sourceFile; }

    // ===== Timeline Position =====

    /**
     * @brief Get start position on timeline (milliseconds)
     * @return Start position relative to timeline start
     */
    [[nodiscard]] int64_t getStartPosition() const { return m_startPosition; }

    /**
     * @brief Set start position on timeline
     * @param position Position in milliseconds
     */
    void setStartPosition(int64_t position) { m_startPosition = position; }

    /**
     * @brief Get the track index (0 = bottom/main, 1+ = overlay tracks)
     * @return Track index
     */
    [[nodiscard]] int32_t getTrackIndex() const { return m_trackIndex; }

    /**
     * @brief Set the track index
     * @param track Track number (0, 1, 2, etc.)
     */
    void setTrackIndex(int32_t track) { m_trackIndex = track; }

    /**
     * @brief Get the duration of this clip in milliseconds
     * @return Clip duration
     */
    [[nodiscard]] int64_t getDuration() const { return m_duration; }

    /**
     * @brief Set custom duration (for trimming)
     * @param duration Duration in milliseconds
     */
    void setDuration(int64_t duration) { m_duration = duration; }

    /**
     * @brief Get the source duration (untrimmed)
     * @return Source video duration
     */
    [[nodiscard]] int64_t getSourceDuration() const { return m_metadata.duration; }

    /**
     * @brief Get end position on timeline
     * @return startPosition + duration
     */
    [[nodiscard]] int64_t getEndPosition() const { return m_startPosition + m_duration; }

    // ===== Trim & Speed =====

    /**
     * @brief Get trim start point (where video starts playing)
     * @return Trim start in source video milliseconds
     */
    [[nodiscard]] int64_t getTrimStart() const { return m_trimStart; }

    /**
     * @brief Set trim start point
     * @param position Position in source video milliseconds
     */
    void setTrimStart(int64_t position) { m_trimStart = position; }

    /**
     * @brief Get trim end point (where video stops playing)
     * @return Trim end in source video milliseconds
     */
    [[nodiscard]] int64_t getTrimEnd() const { return m_trimEnd; }

    /**
     * @brief Set trim end point
     * @param position Position in source video milliseconds
     */
    void setTrimEnd(int64_t position) { m_trimEnd = position; }

    /**
     * @brief Get playback speed
     * @return Speed multiplier (1.0 = normal, 2.0 = 2x, 0.5 = slow motion)
     */
    [[nodiscard]] float getSpeed() const { return m_speed; }

    /**
     * @brief Set playback speed
     * @param speed Speed multiplier (min 0.25, max 4.0)
     */
    void setSpeed(float speed);

    /**
     * @brief Get volume level
     * @return Volume (0.0 = mute, 1.0 = normal, >1.0 = amplified)
     */
    [[nodiscard]] float getVolume() const { return m_volume; }

    /**
     * @brief Set volume level
     * @param volume Volume multiplier (clamped 0.0-2.0)
     */
    void setVolume(float volume);

    // ===== Effects =====

    /**
     * @brief Get the effect chain for this clip
     * @return Reference to effect chain
     */
    [[nodiscard]] const EffectChain& getEffectChain() const { return m_effectChain; }

    /**
     * @brief Get mutable effect chain
     * @return Reference to effect chain
     */
    EffectChain& getEffectChain() { return m_effectChain; }

    /**
     * @brief Apply an effect to this clip
     * @param effect The effect to apply
     */
    void applyEffect(std::shared_ptr<Effect> effect) {
        m_effectChain.addEffect(effect);
    }

    /**
     * @brief Remove an effect from this clip
     * @param effectId ID of effect to remove
     * @return true if removed successfully
     */
    bool removeEffect(const std::string& effectId) {
        return m_effectChain.removeEffect(effectId);
    }

    /**
     * @brief Get all effects on this clip
     * @return Vector of effects
     */
    [[nodiscard]] const EffectChain::EffectList& getEffects() const {
        return m_effectChain.getEffects();
    }

    /**
     * @brief Get number of effects applied
     * @return Effect count
     */
    [[nodiscard]] size_t getEffectCount() const {
        return m_effectChain.getEffectCount();
    }

    // ===== Metadata =====

    /**
     * @brief Get clip metadata
     * @return Reference to metadata structure
     */
    [[nodiscard]] const VideoClipMetadata& getMetadata() const { return m_metadata; }

    /**
     * @brief Update metadata from source file
     * @param metadata New metadata
     */
    void setMetadata(const VideoClipMetadata& metadata) { m_metadata = metadata; }

    /**
     * @brief Get creation timestamp
     * @return Timestamp in milliseconds since epoch
     */
    [[nodiscard]] int64_t getCreatedAt() const { return m_createdAt; }

    /**
     * @brief Get last modification timestamp
     * @return Timestamp in milliseconds since epoch
     */
    [[nodiscard]] int64_t getModifiedAt() const { return m_modifiedAt; }

    /**
     * @brief Update modification timestamp (called when clip is modified)
     */
    void updateModificationTime();

    // ===== Validation =====

    /**
     * @brief Validate that clip is in valid state
     * @return true if clip can be rendered
     */
    [[nodiscard]] bool isValid() const;

    /**
     * @brief Check if clip has unsaved changes
     * @return true if modified since last save
     */
    [[nodiscard]] bool hasChanges() const { return m_modified; }

    /**
     * @brief Mark clip as saved
     */
    void clearChangesFlag() { m_modified = false; }

private:
    std::string m_id;
    std::string m_name;            // Display name
    std::string m_description;     // Optional description

    // Timeline position
    int64_t m_startPosition = 0;   // Where clip appears on timeline (ms)
    int32_t m_trackIndex = 0;      // Which track (0 = main)
    int64_t m_duration = 0;        // Displayed duration (may differ from source)

    // Source trimming
    int64_t m_trimStart = 0;       // Trim start in source video (ms)
    int64_t m_trimEnd = 0;         // Trim end in source video (ms)

    // Playback
    float m_speed = 1.0f;          // Playback speed multiplier
    float m_volume = 1.0f;         // Audio volume multiplier

    // Effects
    EffectChain m_effectChain;     // Applied effects

    // Metadata
    VideoClipMetadata m_metadata;

    // Timestamps
    int64_t m_createdAt;
    int64_t m_modifiedAt;
    bool m_modified = true;        // Track if modified

    /**
     * @brief Get current timestamp
     * @return Milliseconds since epoch
     */
    static int64_t getCurrentTimestamp();
};

} // namespace models
} // namespace clipforge

#endif // CLIPFORGE_VIDEO_CLIP_H
