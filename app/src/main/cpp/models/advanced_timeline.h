#ifndef CLIPFORGE_ADVANCED_TIMELINE_H
#define CLIPFORGE_ADVANCED_TIMELINE_H

#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>

namespace clipforge {
namespace models {

/**
 * @enum TransitionType
 * @brief Available transition effects between clips
 */
enum class TransitionType {
    CUT = 0,                  // No transition
    FADE = 1,                 // Fade to black
    CROSS_FADE = 2,           // Cross dissolve
    SLIDE = 3,                // Slide transition
    WIPE = 4,                 // Wipe transition
    ZOOM_FADE = 5,            // Zoom with fade
    MOTION_BLUR = 6,          // Motion blur transition
    LIGHT_LEAK = 7,           // Light leak transition
};

/**
 * @struct Transition
 * @brief Transition between two clips
 */
struct Transition {
    std::string id;
    TransitionType type = TransitionType::CROSS_FADE;
    int64_t durationMs = 300;      // Transition duration
    float intensity = 1.0f;        // Transition intensity (0-1)

    Transition() = default;
    Transition(const std::string& id, TransitionType t, int64_t duration)
        : id(id), type(t), durationMs(duration) {}
};

/**
 * @enum TrackType
 * @brief Timeline track types
 */
enum class TrackType {
    VIDEO = 0,
    AUDIO = 1,
    TEXT = 2,
    ADJUSTMENT = 3,  // Color correction, effects layer
};

/**
 * @struct TrackProperties
 * @brief Properties for timeline track
 */
struct TrackProperties {
    int trackIndex = 0;
    TrackType type = TrackType::VIDEO;
    std::string trackName;
    bool locked = false;          // Prevent editing
    bool muted = false;           // Hide/mute track
    bool soloed = false;          // Solo this track
    float opacity = 1.0f;         // Track opacity
    int blendMode = 0;            // Blend mode (0=normal, 1=multiply, 2=screen, etc.)
};

/**
 * @class AdvancedTimeline
 * @brief Professional timeline with multi-track support and transitions
 *
 * Features:
 * - Multiple video, audio, and text tracks
 * - Clip transitions
 * - Track locking and muting
 * - Group selections
 * - Undo/redo support
 * - Nested sequences
 */
class AdvancedTimeline {
public:
    /**
     * @brief Create advanced timeline
     * @param width Video width in pixels
     * @param height Video height in pixels
     * @param fps Frames per second
     */
    AdvancedTimeline(int32_t width = 1920, int32_t height = 1080, float fps = 30.0f);

    /**
     * @brief Destructor
     */
    ~AdvancedTimeline() = default;

    // ===== Track Management =====

    /**
     * @brief Add a new track
     * @param type Track type (video, audio, text)
     * @param name Track name
     * @param insertAtIndex Index to insert (optional)
     * @return Track index
     */
    int addTrack(TrackType type, const std::string& name = "", int insertAtIndex = -1);

    /**
     * @brief Remove track and all its clips
     * @param trackIndex Track index
     * @return true if removed successfully
     */
    bool removeTrack(int trackIndex);

    /**
     * @brief Lock track to prevent editing
     * @param trackIndex Track index
     * @param locked true to lock
     */
    void setTrackLocked(int trackIndex, bool locked);

    /**
     * @brief Mute track
     * @param trackIndex Track index
     * @param muted true to mute
     */
    void setTrackMuted(int trackIndex, bool muted);

    /**
     * @brief Solo track (hide all other tracks)
     * @param trackIndex Track index
     * @param soloed true to solo
     */
    void setTrackSoloed(int trackIndex, bool soloed);

    /**
     * @brief Get track properties
     * @param trackIndex Track index
     * @return Track properties
     */
    [[nodiscard]] const TrackProperties& getTrackProperties(int trackIndex) const;

    /**
     * @brief Get number of tracks
     * @return Track count
     */
    [[nodiscard]] int getTrackCount() const { return m_tracks.size(); }

    /**
     * @brief Get number of tracks of specific type
     * @param type Track type
     * @return Count of tracks with that type
     */
    [[nodiscard]] int getTrackCountByType(TrackType type) const;

    // ===== Transitions =====

    /**
     * @brief Add transition between clips
     * @param clipId1 First clip ID
     * @param clipId2 Second clip ID
     * @param transition Transition to add
     * @return Transition ID or empty if failed
     */
    std::string addTransition(const std::string& clipId1, const std::string& clipId2,
                             const Transition& transition);

    /**
     * @brief Remove transition
     * @param transitionId Transition ID
     * @return true if removed
     */
    bool removeTransition(const std::string& transitionId);

    /**
     * @brief Get transition between clips
     * @param clipId1 First clip ID
     * @param clipId2 Second clip ID
     * @return Transition or nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<Transition> getTransition(
        const std::string& clipId1, const std::string& clipId2) const;

    /**
     * @brief Get all transitions
     * @return Vector of all transitions
     */
    [[nodiscard]] std::vector<std::shared_ptr<Transition>> getAllTransitions() const;

    /**
     * @brief Get transition count
     * @return Number of transitions
     */
    [[nodiscard]] size_t getTransitionCount() const { return m_transitions.size(); }

    // ===== Multi-Track Operations =====

    /**
     * @brief Disable empty tracks (no clips)
     * @return Number of tracks disabled
     */
    int disableEmptyTracks();

    /**
     * @brief Get active track indices
     * @return Vector of indices with clips
     */
    [[nodiscard]] std::vector<int> getActiveTrackIndices() const;

    /**
     * @brief Render output timeline for preview/export
     * @param outputTrackIndex Output track to render (0 = composite all)
     * @return Processed timeline data
     */
    [[nodiscard]] std::vector<uint8_t> renderTimeline(int outputTrackIndex = 0) const;

    // ===== Grouping & Selection =====

    /**
     * @brief Group clips for synchronized editing
     * @param clipIds Vector of clip IDs to group
     * @return Group ID
     */
    std::string groupClips(const std::vector<std::string>& clipIds);

    /**
     * @brief Ungroup clips
     * @param groupId Group ID
     * @return true if ungrouped
     */
    bool ungroupClips(const std::string& groupId);

    /**
     * @brief Get clips in group
     * @param groupId Group ID
     * @return Vector of clip IDs
     */
    [[nodiscard]] std::vector<std::string> getGroupClips(const std::string& groupId) const;

    /**
     * @brief Select multiple clips
     * @param clipIds Vector of clip IDs
     */
    void selectClips(const std::vector<std::string>& clipIds);

    /**
     * @brief Get selected clips
     * @return Vector of selected clip IDs
     */
    [[nodiscard]] const std::vector<std::string>& getSelectedClips() const {
        return m_selectedClipIds;
    }

    // ===== History & Undo/Redo =====

    /**
     * @brief Undo last operation
     * @return true if undo successful
     */
    bool undo();

    /**
     * @brief Redo last undone operation
     * @return true if redo successful
     */
    bool redo();

    /**
     * @brief Check if undo available
     * @return true if can undo
     */
    [[nodiscard]] bool canUndo() const;

    /**
     * @brief Check if redo available
     * @return true if can redo
     */
    [[nodiscard]] bool canRedo() const;

    /**
     * @brief Clear undo/redo history
     */
    void clearHistory();

    // ===== Timeline Editing =====

    /**
     * @brief Duplicate clip
     * @param clipId Clip to duplicate
     * @return ID of duplicated clip
     */
    std::string duplicateClip(const std::string& clipId);

    /**
     * @brief Duplicate entire track
     * @param trackIndex Track to duplicate
     * @return Index of duplicated track
     */
    int duplicateTrack(int trackIndex);

    /**
     * @brief Ripple delete - remove clip and shift subsequent clips
     * @param clipId Clip to delete
     * @return true if deleted
     */
    bool rippleDelete(const std::string& clipId);

    /**
     * @brief Slip edit - move clip content without changing position
     * @param clipId Clip ID
     * @param slipAmount Milliseconds to slip (positive = move content forward)
     */
    void slipEdit(const std::string& clipId, int64_t slipAmount);

    /**
     * @brief Slide edit - move clip and adjust neighbors to maintain gaps
     * @param clipId Clip ID
     * @param slideAmount Milliseconds to slide
     */
    void slideEdit(const std::string& clipId, int64_t slideAmount);

    /**
     * @brief Extend clip to next event
     * @param clipId Clip ID
     * @param direction -1 = left, 1 = right
     */
    void extendClipToEvent(const std::string& clipId, int direction);

    // ===== Timeline Analysis =====

    /**
     * @brief Get next event (clip boundary, transition, etc.)
     * @param timeMs Current time
     * @return Time of next event in milliseconds
     */
    [[nodiscard]] int64_t getNextEvent(int64_t timeMs) const;

    /**
     * @brief Get previous event
     * @param timeMs Current time
     * @return Time of previous event in milliseconds
     */
    [[nodiscard]] int64_t getPreviousEvent(int64_t timeMs) const;

    /**
     * @brief Get timeline gaps (unused segments)
     * @return Vector of (start, duration) gap segments
     */
    [[nodiscard]] std::vector<std::pair<int64_t, int64_t>> getGaps() const;

    /**
     * @brief Close all gaps in timeline
     * @return Number of gaps closed
     */
    int closeGaps();

    /**
     * @brief Get timeline statistics
     * @param outNumClips Number of clips
     * @param outNumEffects Number of effects
     * @param outNumTransitions Number of transitions
     */
    void getStatistics(int& outNumClips, int& outNumEffects, int& outNumTransitions) const;

private:
    std::vector<TrackProperties> m_tracks;
    std::unordered_map<std::string, std::shared_ptr<Transition>> m_transitions;
    std::vector<std::string> m_selectedClipIds;
    std::vector<std::pair<std::string, std::vector<std::string>>> m_clipGroups;

    // Undo/Redo
    std::vector<std::string> m_undoStack;
    std::vector<std::string> m_redoStack;

    int32_t m_width = 1920;
    int32_t m_height = 1080;
    float m_fps = 30.0f;
};

} // namespace models
} // namespace clipforge

#endif // CLIPFORGE_ADVANCED_TIMELINE_H
