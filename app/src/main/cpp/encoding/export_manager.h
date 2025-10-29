#ifndef CLIPFORGE_EXPORT_MANAGER_H
#define CLIPFORGE_EXPORT_MANAGER_H

#include "video_encoder.h"
#include "../models/timeline.h"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <atomic>
#include <thread>

namespace clipforge {
namespace encoding {

/**
 * @enum ExportFormat
 * @brief Output file format
 */
enum class ExportFormat {
    MP4,        // MPEG-4 container (H.264/H.265)
    WEBM,       // WebM container (VP9)
    MKV,        // Matroska (H.264/H.265/VP9)
};

/**
 * @enum QualityPreset
 * @brief Predefined quality presets
 */
enum class QualityPreset {
    LOW,        // 480p, low bitrate
    MEDIUM,     // 720p, medium bitrate
    HIGH,       // 1080p, high bitrate
    ULTRA,      // 2K/4K, maximum quality
};

/**
 * @struct ExportConfig
 * @brief Complete export configuration
 */
struct ExportConfig {
    // Output
    std::string outputPath;
    ExportFormat format = ExportFormat::MP4;
    QualityPreset quality = QualityPreset::HIGH;

    // Video
    int width = 1920;
    int height = 1080;
    int frameRate = 30;
    VideoCodec codec = VideoCodec::H264;

    // Audio
    int audioSampleRate = 44100;
    int audioChannels = 2;
    int audioBitrate = 128000;

    // Processing
    bool useHardwareEncoding = true;
    int encodingThreads = 2;
    bool parallelEncoding = true;

    // Duration
    int64_t durationMs = 0;

    // Timeline
    std::shared_ptr<models::Timeline> timeline;
};

/**
 * @struct ExportProgress
 * @brief Export operation progress
 */
struct ExportProgress {
    float videoProgress = 0.0f;        // Video encoding (0-1)
    float audioProgress = 0.0f;        // Audio mixing (0-1)
    float muxingProgress = 0.0f;       // Muxing/multiplexing (0-1)
    float totalProgress = 0.0f;        // Overall (0-1)

    int64_t framesEncoded = 0;
    int64_t totalFrames = 0;
    int64_t audioSamplesProcessed = 0;
    int64_t totalAudioSamples = 0;

    float estimatedRemainingSeconds = 0.0f;
    std::string currentPhase;          // "Video", "Audio", "Muxing"
    std::string status;
};

/**
 * @class ExportManager
 * @brief High-level export pipeline orchestrator
 *
 * Handles complete video export with:
 * - Format conversion
 * - Codec selection
 * - Quality presets
 * - Real-time progress tracking
 * - Cancellation support
 * - Error recovery
 *
 * Usage:
 * @code
 * auto manager = std::make_shared<ExportManager>();
 * ExportConfig config{timeline, "output.mp4", ExportFormat::MP4};
 * manager->setProgressCallback([](const ExportProgress& p) {
 *     printf("Progress: %.1f%%\n", p.totalProgress * 100);
 * });
 *
 * if (manager->startExport(config)) {
 *     // Export in progress
 *     while (manager->isExporting()) {
 *         // Monitor progress
 *     }
 * }
 * @endcode
 */
class ExportManager {
public:
    // Progress callback type
    using ProgressCallback = std::function<void(const ExportProgress&)>;

    /**
     * @brief Create export manager
     */
    ExportManager();

    /**
     * @brief Destructor
     */
    ~ExportManager();

    // Prevent copying
    ExportManager(const ExportManager&) = delete;
    ExportManager& operator=(const ExportManager&) = delete;

    // ===== Configuration =====

    /**
     * @brief Set export configuration
     * @param config Export parameters
     * @return true if valid
     */
    bool setConfig(const ExportConfig& config);

    /**
     * @brief Get current configuration
     * @return Export config
     */
    [[nodiscard]] const ExportConfig& getConfig() const { return m_config; }

    /**
     * @brief Get estimated file size
     * @return Estimated size in bytes
     */
    [[nodiscard]] int64_t getEstimatedFileSize() const;

    // ===== Export Control =====

    /**
     * @brief Start export operation
     * @return true if started successfully
     */
    bool startExport();

    /**
     * @brief Cancel ongoing export
     * @return true if cancelled
     */
    bool cancelExport();

    /**
     * @brief Check if export is in progress
     * @return true if exporting
     */
    [[nodiscard]] bool isExporting() const { return m_exporting.load(); }

    /**
     * @brief Check if export was cancelled
     * @return true if cancelled
     */
    [[nodiscard]] bool wasCancelled() const { return m_cancelled.load(); }

    /**
     * @brief Check if export completed successfully
     * @return true if complete
     */
    [[nodiscard]] bool isComplete() const { return m_complete.load(); }

    /**
     * @brief Wait for export to complete
     * @param timeoutMs Timeout in milliseconds (0 = infinite)
     * @return true if completed, false if timeout
     */
    bool waitForCompletion(int timeoutMs = 0);

    // ===== Progress Monitoring =====

    /**
     * @brief Get current progress
     * @return Export progress
     */
    [[nodiscard]] const ExportProgress& getProgress() const { return m_progress; }

    /**
     * @brief Get overall completion percentage
     * @return Progress (0-100)
     */
    [[nodiscard]] float getProgressPercentage() const {
        return m_progress.totalProgress * 100.0f;
    }

    /**
     * @brief Set progress callback
     * @param callback Called when progress updates
     */
    void setProgressCallback(ProgressCallback callback) {
        m_progressCallback = callback;
    }

    /**
     * @brief Get current phase
     * @return Phase name ("Video", "Audio", "Muxing", etc.)
     */
    [[nodiscard]] std::string getCurrentPhase() const { return m_progress.currentPhase; }

    /**
     * @brief Get estimated time remaining
     * @return Seconds remaining
     */
    [[nodiscard]] float getEstimatedTimeRemaining() const {
        return m_progress.estimatedRemainingSeconds;
    }

    // ===== Quality Presets =====

    /**
     * @brief Apply quality preset
     * @param preset Quality level
     * @return true if applied
     */
    bool applyQualityPreset(QualityPreset preset);

    /**
     * @brief Get bitrate for preset
     * @param preset Quality preset
     * @return Bitrate in bps
     */
    [[nodiscard]] static int getPresetBitrate(QualityPreset preset);

    /**
     * @brief Get resolution for preset
     * @param preset Quality preset
     * @return {width, height}
     */
    [[nodiscard]] static std::pair<int, int> getPresetResolution(QualityPreset preset);

    /**
     * @brief Get format recommendation for codec
     * @param codec Video codec
     * @return Recommended format
     */
    [[nodiscard]] static ExportFormat getRecommendedFormat(VideoCodec codec);

    // ===== Statistics =====

    /**
     * @brief Get final file size (after export)
     * @return File size in bytes
     */
    [[nodiscard]] int64_t getFinalFileSize() const;

    /**
     * @brief Get encoding statistics
     * @return Statistics string
     */
    [[nodiscard]] std::string getStatistics() const;

    /**
     * @brief Get error message if export failed
     * @return Error description
     */
    [[nodiscard]] std::string getErrorMessage() const { return m_lastError; }

    /**
     * @brief Check if export had error
     * @return true if error occurred
     */
    [[nodiscard]] bool hasError() const { return !m_lastError.empty(); }

private:
    ExportConfig m_config;
    ExportProgress m_progress;
    ProgressCallback m_progressCallback;

    std::atomic<bool> m_exporting{false};
    std::atomic<bool> m_cancelled{false};
    std::atomic<bool> m_complete{false};

    std::unique_ptr<std::thread> m_exportThread;
    std::shared_ptr<VideoEncoder> m_videoEncoder;

    std::string m_lastError;
    int64_t m_startTimeMs = 0;

    /**
     * @brief Main export thread function
     */
    void exportThread();

    /**
     * @brief Encode video from timeline
     * @return true if successful
     */
    bool encodeVideo();

    /**
     * @brief Mix and encode audio
     * @return true if successful
     */
    bool encodeAudio();

    /**
     * @brief Mux video and audio into container
     * @return true if successful
     */
    bool muxStreams();

    /**
     * @brief Update progress and trigger callback
     */
    void updateProgress();

    /**
     * @brief Validate configuration
     * @return true if valid
     */
    bool validateConfig() const;
};

} // namespace encoding
} // namespace clipforge

#endif // CLIPFORGE_EXPORT_MANAGER_H
