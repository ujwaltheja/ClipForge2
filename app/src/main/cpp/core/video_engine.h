#ifndef CLIPFORGE_VIDEO_ENGINE_H
#define CLIPFORGE_VIDEO_ENGINE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <atomic>
#include <thread>
#include <mutex>
#include <cstdint>

#include "../models/timeline.h"
#include "../models/video_clip.h"
#include "../models/audio_track.h"
#include "../models/effect.h"

namespace clipforge {
namespace core {

/**
 * @enum EngineState
 * @brief State of the video engine
 */
enum class EngineState {
    IDLE,              // Engine ready but not processing
    PREVIEW_PLAYING,   // Playing preview
    RENDERING,         // Rendering export
    PAUSED,            // Processing paused
    ERROR,             // Error state
    SHUTDOWN,          // Engine shutting down
};

/**
 * @struct EngineConfig
 * @brief Configuration for the video engine
 */
struct EngineConfig {
    int32_t previewQuality = 720;  // Preview resolution height
    bool useHardwareAccel = true;  // Use GPU acceleration
    int32_t maxRenderThreads = 4;  // Maximum rendering threads
    bool enablePreviewCache = true; // Cache preview frames
    size_t maxCacheSize = 500 * 1024 * 1024; // 500 MB cache
    std::string tempDirectory = "/tmp/clipforge";

    EngineConfig() = default;
};

/**
 * @struct RenderProgress
 * @brief Progress information for rendering
 */
struct RenderProgress {
    int64_t currentFrame = 0;       // Current frame being processed
    int64_t totalFrames = 0;        // Total frames to render
    float percentage = 0.0f;        // Completion percentage (0-100)
    double elapsedTime = 0.0;       // Seconds elapsed
    double estimatedRemaining = 0.0; // Estimated seconds remaining
    int64_t bytesProcessed = 0;    // Bytes written to output
    std::string status;             // Status message

    /**
     * @brief Get progress as percentage
     * @return Value from 0 to 100
     */
    [[nodiscard]] float getPercentage() const { return percentage; }

    /**
     * @brief Get frames per second being rendered
     * @return FPS value
     */
    [[nodiscard]] float getFPS() const {
        if (elapsedTime <= 0.0) return 0.0f;
        return static_cast<float>(static_cast<double>(currentFrame) / elapsedTime);
    }
};

/**
 * @class VideoEngine
 * @brief Main video editing and rendering engine
 *
 * The VideoEngine is the core coordinator for all video editing operations.
 * It manages the timeline, processes effects, handles preview rendering,
 * and orchestrates the export process. The engine is thread-safe and can
 * handle concurrent preview updates and export operations.
 *
 * Usage:
 * @code
 * auto engine = std::make_unique<VideoEngine>();
 * engine->initialize(config);
 *
 * // Create or load project
 * auto timeline = std::make_shared<models::Timeline>(1920, 1080, 30.0f);
 * engine->setTimeline(timeline);
 *
 * // Manipulate clips
 * auto clip = engine->addClip("/path/to/video.mp4");
 * engine->applyEffect(clip->getId(), effect);
 *
 * // Render
 * engine->startExport("/output/video.mp4", config);
 * @endcode
 */
class VideoEngine {
public:
    /**
     * @brief Create a new video engine instance
     */
    VideoEngine();

    /**
     * @brief Destructor - cleanup resources
     */
    ~VideoEngine();

    // Prevent copying
    VideoEngine(const VideoEngine&) = delete;
    VideoEngine& operator=(const VideoEngine&) = delete;

    // ===== Lifecycle =====

    /**
     * @brief Initialize the engine with configuration
     * @param config Engine configuration
     * @return true if initialization successful
     */
    bool initialize(const EngineConfig& config);

    /**
     * @brief Shutdown the engine gracefully
     */
    void shutdown();

    /**
     * @brief Get current engine state
     * @return Current state
     */
    [[nodiscard]] EngineState getState() const;

    /**
     * @brief Get engine configuration
     * @return Reference to current config
     */
    [[nodiscard]] const EngineConfig& getConfig() const { return m_config; }

    /**
     * @brief Get engine error message if in error state
     * @return Error description
     */
    [[nodiscard]] std::string getErrorMessage() const;

    // ===== Project/Timeline =====

    /**
     * @brief Set the current timeline
     * @param timeline The timeline to work with
     * @return true if set successfully
     */
    bool setTimeline(std::shared_ptr<models::Timeline> timeline);

    /**
     * @brief Get current timeline
     * @return Shared pointer to timeline
     */
    [[nodiscard]] std::shared_ptr<models::Timeline> getTimeline() const {
        return m_timeline;
    }

    /**
     * @brief Check if a timeline is set
     * @return true if timeline exists
     */
    [[nodiscard]] bool hasTimeline() const { return m_timeline != nullptr; }

    // ===== Clip Management =====

    /**
     * @brief Add a video clip to the timeline
     * @param sourcePath Path to video file
     * @param startPosition Position on timeline (ms)
     * @param trackIndex Track number
     * @return Clip ID if successful, empty string on error
     */
    std::string addClip(const std::string& sourcePath, int64_t startPosition = 0,
                       int32_t trackIndex = 0);

    /**
     * @brief Remove a clip from timeline
     * @param clipId ID of clip to remove
     * @return true if removed successfully
     */
    bool removeClip(const std::string& clipId);

    /**
     * @brief Move a clip to a new position/track
     * @param clipId Clip ID
     * @param newStartPosition New position on timeline (ms)
     * @param newTrackIndex New track number
     * @return true if moved successfully
     */
    bool moveClip(const std::string& clipId, int64_t newStartPosition,
                 int32_t newTrackIndex);

    /**
     * @brief Trim a clip
     * @param clipId Clip ID
     * @param trimStart Trim start in source (ms)
     * @param trimEnd Trim end in source (ms)
     * @return true if trimmed successfully
     */
    bool trimClip(const std::string& clipId, int64_t trimStart, int64_t trimEnd);

    /**
     * @brief Set clip playback speed
     * @param clipId Clip ID
     * @param speed Speed multiplier (0.25-4.0)
     * @return true if set successfully
     */
    bool setClipSpeed(const std::string& clipId, float speed);

    /**
     * @brief Set clip volume
     * @param clipId Clip ID
     * @param volume Volume multiplier (0.0-2.0)
     * @return true if set successfully
     */
    bool setClipVolume(const std::string& clipId, float volume);

    /**
     * @brief Split a clip at a specific time
     * @param clipId Clip to split
     * @param splitTime Time in clip to split at (ms from clip start)
     * @return ID of new clip created, empty string on error
     */
    std::string splitClip(const std::string& clipId, int64_t splitTime);

    // ===== Effects =====

    /**
     * @brief Apply an effect to a clip
     * @param clipId Clip ID
     * @param effect Effect to apply
     * @return true if applied successfully
     */
    bool applyEffect(const std::string& clipId, std::shared_ptr<models::Effect> effect);

    /**
     * @brief Remove an effect from a clip
     * @param clipId Clip ID
     * @param effectId Effect ID
     * @return true if removed successfully
     */
    bool removeEffect(const std::string& clipId, const std::string& effectId);

    /**
     * @brief Get available effects library
     * @return Vector of all available effects
     */
    [[nodiscard]] std::vector<std::shared_ptr<models::Effect>> getAvailableEffects() const;

    /**
     * @brief Create an effect by name
     * @param effectName Name of effect to create
     * @return Effect shared pointer, or nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<models::Effect> createEffect(const std::string& effectName);

    // ===== Audio Management =====

    /**
     * @brief Add an audio track
     * @param name Track display name
     * @param type Track type (main, voiceover, music, sfx)
     * @return Track ID if successful, empty string on error
     */
    std::string addAudioTrack(const std::string& name = "Audio",
                             const std::string& type = "main");

    /**
     * @brief Remove an audio track
     * @param trackId Track ID
     * @return true if removed successfully
     */
    bool removeAudioTrack(const std::string& trackId);

    /**
     * @brief Set audio track volume
     * @param trackId Track ID
     * @param volume Volume multiplier (0.0-2.0)
     * @return true if set successfully
     */
    bool setAudioTrackVolume(const std::string& trackId, float volume);

    /**
     * @brief Set audio track mute state
     * @param trackId Track ID
     * @param muted true to mute
     * @return true if set successfully
     */
    bool setAudioTrackMuted(const std::string& trackId, bool muted);

    /**
     * @brief Get total timeline duration
     * @return Duration in milliseconds
     */
    [[nodiscard]] int64_t getTimelineDuration() const;

    // ===== Preview =====

    /**
     * @brief Start preview playback
     * @return true if preview started
     */
    bool startPreview();

    /**
     * @brief Pause preview playback
     * @return true if paused
     */
    bool pausePreview();

    /**
     * @brief Stop preview playback
     * @return true if stopped
     */
    bool stopPreview();

    /**
     * @brief Seek to a specific time in preview
     * @param timeMs Position in milliseconds
     * @return true if seeked successfully
     */
    bool seekPreview(int64_t timeMs);

    /**
     * @brief Check if preview is playing
     * @return true if currently playing
     */
    [[nodiscard]] bool isPreviewPlaying() const;

    /**
     * @brief Get current preview position
     * @return Position in milliseconds
     */
    [[nodiscard]] int64_t getPreviewPosition() const;

    /**
     * @brief Get preview frame at specific time
     * @param timeMs Time in milliseconds
     * @return Raw frame data (ownership transferred to caller)
     *
     * Warning: This is an expensive operation. Prefer using callbacks
     * for real-time preview.
     */
    [[nodiscard]] std::vector<uint8_t> getPreviewFrame(int64_t timeMs);

    // ===== Export =====

    /**
     * @brief Start rendering/exporting the project
     * @param outputPath Where to save the output file
     * @param format Output format (mp4, webm, etc.)
     * @param quality Quality preset (low, medium, high, ultra)
     * @return true if export started successfully
     */
    bool startExport(const std::string& outputPath, const std::string& format = "mp4",
                    const std::string& quality = "high");

    /**
     * @brief Cancel current export operation
     * @return true if cancelled
     */
    bool cancelExport();

    /**
     * @brief Get export progress
     * @return Current render progress
     */
    [[nodiscard]] RenderProgress getExportProgress() const;

    /**
     * @brief Check if export is in progress
     * @return true if currently rendering
     */
    [[nodiscard]] bool isExporting() const;

    /**
     * @brief Get export output file path
     * @return File path of export output
     */
    [[nodiscard]] std::string getExportOutputPath() const;

    // ===== Persistence =====

    /**
     * @brief Save project to file
     * @param projectPath Where to save project file
     * @return true if saved successfully
     */
    bool saveProject(const std::string& projectPath);

    /**
     * @brief Load project from file
     * @param projectPath Path to project file
     * @return true if loaded successfully
     */
    bool loadProject(const std::string& projectPath);

    /**
     * @brief Check if project has unsaved changes
     * @return true if modified since last save
     */
    [[nodiscard]] bool hasUnsavedChanges() const;

    /**
     * @brief Get info about what changed
     * @return Description of changes
     */
    [[nodiscard]] std::string getChangesSummary() const;

    // ===== Statistics =====

    /**
     * @brief Get memory usage statistics
     * @return Memory used in bytes
     */
    [[nodiscard]] size_t getMemoryUsage() const;

    /**
     * @brief Get total number of clips
     * @return Clip count
     */
    [[nodiscard]] size_t getClipCount() const;

    /**
     * @brief Get total number of effects applied
     * @return Effect count across all clips
     */
    [[nodiscard]] size_t getEffectCount() const;

    /**
     * @brief Get engine version
     * @return Version string
     */
    [[nodiscard]] static std::string getVersion();

private:
    EngineConfig m_config;
    std::shared_ptr<models::Timeline> m_timeline;

    // State management
    std::atomic<EngineState> m_state{EngineState::IDLE};
    std::string m_errorMessage;
    mutable std::mutex m_stateMutex;

    // Preview
    std::atomic<bool> m_previewPlaying{false};
    std::atomic<int64_t> m_previewPosition{0};
    std::unique_ptr<std::thread> m_previewThread;

    // Export
    std::atomic<bool> m_exporting{false};
    std::atomic<bool> m_cancelExport{false};
    RenderProgress m_exportProgress;
    std::string m_exportOutputPath;
    std::unique_ptr<std::thread> m_exportThread;
    mutable std::mutex m_exportMutex;

    /**
     * @brief Preview playback thread function
     */
    void previewPlaybackThread();

    /**
     * @brief Export rendering thread function
     */
    void exportRenderingThread(const std::string& outputPath, const std::string& format,
                              const std::string& quality);

    /**
     * @brief Set engine state
     * @param state New state
     */
    void setState(EngineState state);

    /**
     * @brief Set error state with message
     * @param message Error message
     */
    void setError(const std::string& message);

    /**
     * @brief Validate timeline
     * @return true if valid
     */
    [[nodiscard]] bool validateTimeline() const;

    /**
     * @brief Load effects library
     * @return true if loaded
     */
    bool loadEffectsLibrary();
};

} // namespace core
} // namespace clipforge

#endif // CLIPFORGE_VIDEO_ENGINE_H
