#ifndef CLIPFORGE_TIMELINE_H
#define CLIPFORGE_TIMELINE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "video_clip.h"
#include "audio_track.h"

namespace clipforge {
namespace models {

/**
 * @struct TimelineProperties
 * @brief Properties of the timeline/project
 */
struct TimelineProperties {
    int32_t width = 1920;          // Video width in pixels
    int32_t height = 1080;         // Video height in pixels
    float frameRate = 30.0f;       // Frames per second
    int32_t maxTracks = 3;         // Maximum video tracks
    std::string colorSpace = "BT.709";  // Color space standard
    bool useHardwareAccel = true;  // Use GPU acceleration

    TimelineProperties() = default;
};

/**
 * @class Timeline
 * @brief Main timeline containing all video clips and audio tracks
 *
 * The timeline is the core data structure representing a video project.
 * It manages multiple video tracks and audio tracks, with all clips
 * arranged along a time axis.
 */
class Timeline {
public:
    using ClipList = std::vector<std::shared_ptr<VideoClip>>;
    using AudioTrackList = std::vector<std::shared_ptr<AudioTrack>>;
    using ClipMap = std::unordered_map<std::string, std::shared_ptr<VideoClip>>;
    using AudioTrackMap = std::unordered_map<std::string, std::shared_ptr<AudioTrack>>;

    /**
     * @brief Create a new timeline
     * @param width Video width in pixels
     * @param height Video height in pixels
     * @param fps Frames per second
     */
    explicit Timeline(int32_t width = 1920, int32_t height = 1080, float fps = 30.0f);

    /**
     * @brief Destructor
     */
    ~Timeline() = default;

    /**
     * @brief Copy constructor
     */
    Timeline(const Timeline& other) = default;

    /**
     * @brief Move constructor
     */
    Timeline(Timeline&& other) noexcept = default;

    // ===== Properties =====

    /**
     * @brief Get timeline properties
     * @return Reference to properties
     */
    [[nodiscard]] const TimelineProperties& getProperties() const { return m_properties; }

    /**
     * @brief Set timeline properties
     * @param props New properties
     */
    void setProperties(const TimelineProperties& props) { m_properties = props; }

    /**
     * @brief Get total duration of timeline
     * @return Duration in milliseconds
     */
    [[nodiscard]] int64_t getTotalDuration() const { return m_totalDuration; }

    /**
     * @brief Get current playback position
     * @return Position in milliseconds
     */
    [[nodiscard]] int64_t getCurrentPosition() const { return m_currentPosition; }

    /**
     * @brief Set current playback position
     * @param position Position in milliseconds
     */
    void setCurrentPosition(int64_t position);

    /**
     * @brief Recalculate total duration based on all clips
     */
    void updateDuration();

    // ===== Video Clips =====

    /**
     * @brief Add a video clip to the timeline
     * @param clip The clip to add
     * @return true if successfully added
     */
    bool addClip(std::shared_ptr<VideoClip> clip);

    /**
     * @brief Remove a clip from the timeline
     * @param clipId ID of clip to remove
     * @return true if removed successfully
     */
    bool removeClip(const std::string& clipId);

    /**
     * @brief Get a clip by ID
     * @param clipId The clip's ID
     * @return Shared pointer to clip, or nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<VideoClip> getClip(const std::string& clipId) const;

    /**
     * @brief Get all clips in order
     * @return Vector of all clips sorted by start position
     */
    [[nodiscard]] const ClipList& getAllClips() const { return m_clipList; }

    /**
     * @brief Get clips on a specific track
     * @param trackIndex Track number
     * @return Vector of clips on that track
     */
    [[nodiscard]] ClipList getClipsOnTrack(int32_t trackIndex) const;

    /**
     * @brief Get clips at a specific time
     * @param timeMs Time in milliseconds
     * @return Vector of clips that contain this time
     */
    [[nodiscard]] ClipList getClipsAtTime(int64_t timeMs) const;

    /**
     * @brief Get total number of clips
     * @return Clip count
     */
    [[nodiscard]] size_t getClipCount() const { return m_clipList.size(); }

    /**
     * @brief Check if timeline has any clips
     * @return true if empty
     */
    [[nodiscard]] bool isEmpty() const { return m_clipList.empty(); }

    /**
     * @brief Clear all clips from timeline
     */
    void clearClips();

    // ===== Audio Tracks =====

    /**
     * @brief Create and add a new audio track
     * @param name Display name for track
     * @param type Track type (main, voiceover, music, sfx)
     * @return ID of created track
     */
    std::string addAudioTrack(const std::string& name = "Audio", const std::string& type = "main");

    /**
     * @brief Remove an audio track
     * @param trackId ID of track to remove
     * @return true if removed successfully
     */
    bool removeAudioTrack(const std::string& trackId);

    /**
     * @brief Get audio track by ID
     * @param trackId The track's ID
     * @return Shared pointer to track, or nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<AudioTrack> getAudioTrack(const std::string& trackId) const;

    /**
     * @brief Get all audio tracks
     * @return Vector of all audio tracks
     */
    [[nodiscard]] const AudioTrackList& getAllAudioTracks() const { return m_audioTrackList; }

    /**
     * @brief Get number of audio tracks
     * @return Track count
     */
    [[nodiscard]] size_t getAudioTrackCount() const { return m_audioTrackList.size(); }

    /**
     * @brief Get the main audio track (usually from first clip)
     * @return Shared pointer to main track, or nullptr
     */
    [[nodiscard]] std::shared_ptr<AudioTrack> getMainAudioTrack() const;

    /**
     * @brief Clear all audio tracks
     */
    void clearAudioTracks();

    // ===== Selection =====

    /**
     * @brief Select a clip
     * @param clipId ID of clip to select
     */
    void selectClip(const std::string& clipId);

    /**
     * @brief Deselect all clips
     */
    void deselectAll();

    /**
     * @brief Get currently selected clip
     * @return Shared pointer to selected clip, or nullptr
     */
    [[nodiscard]] std::shared_ptr<VideoClip> getSelectedClip() const;

    /**
     * @brief Get ID of selected clip
     * @return Clip ID or empty string if none selected
     */
    [[nodiscard]] const std::string& getSelectedClipId() const { return m_selectedClipId; }

    /**
     * @brief Check if a clip is selected
     * @param clipId The clip to check
     * @return true if selected
     */
    [[nodiscard]] bool isClipSelected(const std::string& clipId) const {
        return m_selectedClipId == clipId;
    }

    // ===== Validation =====

    /**
     * @brief Check if timeline is valid
     * @return true if timeline can be rendered
     */
    [[nodiscard]] bool isValid() const;

    /**
     * @brief Check if there are overlapping clips on same track
     * @return true if overlaps detected
     */
    [[nodiscard]] bool hasOverlappingClips() const;

    /**
     * @brief Get first overlapping clips if any
     * @param outClip1 First overlapping clip
     * @param outClip2 Second overlapping clip
     * @return true if overlap found
     */
    bool getFirstOverlap(std::shared_ptr<VideoClip>& outClip1,
                         std::shared_ptr<VideoClip>& outClip2) const;

    /**
     * @brief Get max track index in use
     * @return Highest track number with clips
     */
    [[nodiscard]] int32_t getMaxTrackInUse() const;

    // ===== Persistence =====

    /**
     * @brief Check if timeline has unsaved changes
     * @return true if modified
     */
    [[nodiscard]] bool hasChanges() const { return m_modified; }

    /**
     * @brief Mark all changes as saved
     */
    void clearChangesFlag();

    /**
     * @brief Get modification timestamp
     * @return Milliseconds since epoch
     */
    [[nodiscard]] int64_t getModifiedAt() const { return m_modifiedAt; }

private:
    TimelineProperties m_properties;

    // Playback
    int64_t m_currentPosition = 0; // Current playback position (ms)
    int64_t m_totalDuration = 0;   // Total timeline duration (ms)

    // Clips
    ClipList m_clipList;           // All clips sorted by position
    ClipMap m_clipMap;             // Quick lookup by ID
    std::string m_selectedClipId;  // Currently selected clip

    // Audio
    AudioTrackList m_audioTrackList;
    AudioTrackMap m_audioTrackMap;

    // State
    bool m_modified = true;
    int64_t m_modifiedAt;

    /**
     * @brief Resort clip list after modification
     */
    void sortClips();

    /**
     * @brief Recalculate total duration
     */
    void calculateDuration();

    /**
     * @brief Get current timestamp
     * @return Milliseconds since epoch
     */
    static int64_t getCurrentTimestamp();

    /**
     * @brief Generate unique ID
     * @return UUID string
     */
    static std::string generateId();
};

} // namespace models
} // namespace clipforge

#endif // CLIPFORGE_TIMELINE_H
