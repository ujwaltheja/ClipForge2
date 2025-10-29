#include "export_manager.h"
#include "../utils/logger.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace clipforge {
namespace encoding {

// ===== ExportManager Implementation =====

ExportManager::ExportManager()
    : m_exporting(false), m_cancelled(false), m_complete(false),
      m_videoEncoder(std::make_shared<VideoEncoder>()),
      m_startTimeMs(0) {
    LOG_DEBUG("ExportManager created");
}

ExportManager::~ExportManager() {
    if (m_exporting) {
        cancelExport();
    }
    if (m_exportThread && m_exportThread->joinable()) {
        m_exportThread->join();
    }
    LOG_DEBUG("ExportManager destroyed");
}

bool ExportManager::setConfig(const ExportConfig& config) {
    if (m_exporting) {
        m_lastError = "Cannot configure while export is in progress";
        LOG_ERROR("ExportManager: %s", m_lastError.c_str());
        return false;
    }

    m_config = config;
    m_progress = ExportProgress{};
    m_progress.totalFrames = static_cast<int64_t>(
        (config.durationMs * config.frameRate) / 1000);

    LOG_INFO("ExportManager configured: output=%s, format=%d, quality=%d",
         config.outputPath.c_str(), static_cast<int>(config.format),
         static_cast<int>(config.quality));

    return true;
}

int64_t ExportManager::getEstimatedFileSize() const {
    if (m_config.durationMs <= 0) {
        return 0;
    }

    // Estimate based on bitrate and duration
    int videoBitrate = getPresetBitrate(m_config.quality);
    int totalBitrate = videoBitrate + m_config.audioBitrate;
    int64_t durationSeconds = m_config.durationMs / 1000;

    // bitrate is in bits per second, convert to bytes per second
    int64_t estimatedBytes = (static_cast<int64_t>(totalBitrate) * durationSeconds) / 8;

    // Add container overhead (~5%)
    estimatedBytes = static_cast<int64_t>(static_cast<double>(estimatedBytes) * 1.05);

    return estimatedBytes;
}

bool ExportManager::startExport() {
    if (!m_config.timeline) {
        m_lastError = "Timeline not configured";
        LOG_ERROR("ExportManager: %s", m_lastError.c_str());
        return false;
    }

    if (m_exporting) {
        LOG_WARNING("ExportManager: Export already in progress");
        return false;
    }

    m_exporting = true;
    m_cancelled = false;
    m_complete = false;
    m_progress = ExportProgress{};
    m_startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    // Start export in background thread
    m_exportThread = std::make_unique<std::thread>(&ExportManager::exportThread, this);

    LOG_INFO("ExportManager: Export started");
    return true;
}

bool ExportManager::cancelExport() {
    if (!m_exporting) {
        return false;
    }

    LOG_INFO("ExportManager: Cancel requested");
    m_cancelled = true;

    // Wait for thread to finish
    if (m_exportThread && m_exportThread->joinable()) {
        m_exportThread->join();
    }

    m_exporting = false;
    m_progress.currentPhase = "Cancelled";

    return true;
}

bool ExportManager::waitForCompletion(int timeoutMs) {
    auto startTime = std::chrono::system_clock::now();

    while (m_exporting) {
        if (m_complete) {
            LOG_INFO("ExportManager: Export completed");
            return true;
        }

        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now() - startTime).count();
            if (elapsed > timeoutMs) {
                LOG_WARNING("ExportManager: Timeout waiting for completion");
                return false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return true;
}

bool ExportManager::applyQualityPreset(QualityPreset preset) {
    auto [width, height] = getPresetResolution(preset);
    m_config.width = width;
    m_config.height = height;
    m_config.quality = preset;

    int bitrate = getPresetBitrate(preset);
    m_config.codec = preset == QualityPreset::ULTRA ? VideoCodec::H265 : VideoCodec::H264;

    LOG_INFO("ExportManager: Quality preset applied: %dx%d @ %d bps",
         width, height, bitrate);

    return true;
}

int ExportManager::getPresetBitrate(QualityPreset preset) {
    switch (preset) {
        case QualityPreset::LOW:
            return 2000000;     // 2 Mbps
        case QualityPreset::MEDIUM:
            return 5000000;     // 5 Mbps
        case QualityPreset::HIGH:
            return 8000000;     // 8 Mbps
        case QualityPreset::ULTRA:
            return 25000000;    // 25 Mbps
        default:
            return 8000000;
    }
}

std::pair<int, int> ExportManager::getPresetResolution(QualityPreset preset) {
    switch (preset) {
        case QualityPreset::LOW:
            return {640, 480};       // 480p
        case QualityPreset::MEDIUM:
            return {1280, 720};      // 720p
        case QualityPreset::HIGH:
            return {1920, 1080};     // 1080p
        case QualityPreset::ULTRA:
            return {3840, 2160};     // 4K
        default:
            return {1920, 1080};
    }
}

ExportFormat ExportManager::getRecommendedFormat(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::VP9:
            return ExportFormat::WEBM;
        case VideoCodec::H264:
        case VideoCodec::H265:
        default:
            return ExportFormat::MP4;
    }
}

int64_t ExportManager::getFinalFileSize() const {
    // In a real implementation, would stat the output file
    // For now, return estimated size
    return getEstimatedFileSize();
}

std::string ExportManager::getStatistics() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);
    oss << "Export Statistics:\n";
    oss << "  Phase: " << m_progress.currentPhase << "\n";
    oss << "  Overall Progress: " << (m_progress.totalProgress * 100.0f) << "%\n";
    oss << "  Frames Encoded: " << m_progress.framesEncoded << " / " << m_progress.totalFrames << "\n";
    oss << "  Audio Samples: " << m_progress.audioSamplesProcessed << " / " << m_progress.totalAudioSamples << "\n";
    oss << "  Estimated Remaining: " << m_progress.estimatedRemainingSeconds << "s\n";
    oss << "  File Size: " << (getFinalFileSize() / 1024 / 1024) << " MB\n";

    return oss.str();
}

void ExportManager::exportThread() {
    LOG_INFO("ExportManager: Export thread started");

    try {
        // Phase 1: Encode Video
        if (!m_cancelled) {
            m_progress.currentPhase = "Video";
            LOG_INFO("ExportManager: Starting video encoding");
            if (!encodeVideo()) {
                m_lastError = "Video encoding failed";
                LOG_ERROR("ExportManager: %s", m_lastError.c_str());
                m_exporting = false;
                return;
            }
        }

        // Phase 2: Encode Audio
        if (!m_cancelled) {
            m_progress.currentPhase = "Audio";
            LOG_INFO("ExportManager: Starting audio encoding");
            if (!encodeAudio()) {
                m_lastError = "Audio encoding failed";
                LOG_ERROR("ExportManager: %s", m_lastError.c_str());
                m_exporting = false;
                return;
            }
        }

        // Phase 3: Mux Streams
        if (!m_cancelled) {
            m_progress.currentPhase = "Muxing";
            LOG_INFO("ExportManager: Starting stream multiplexing");
            if (!muxStreams()) {
                m_lastError = "Stream muxing failed";
                LOG_ERROR("ExportManager: %s", m_lastError.c_str());
                m_exporting = false;
                return;
            }
        }

        // Update final progress
        m_progress.totalProgress = 1.0f;
        m_progress.currentPhase = "Complete";
        m_progress.estimatedRemainingSeconds = 0.0f;
        m_complete = true;

        LOG_INFO("ExportManager: Export completed successfully");
        updateProgress();

    } catch (const std::exception& e) {
        m_lastError = std::string("Export exception: ") + e.what();
        LOG_ERROR("ExportManager: %s", m_lastError.c_str());
    }

    m_exporting = false;
}

bool ExportManager::encodeVideo() {
    if (!m_config.timeline) {
        m_lastError = "Timeline not available";
        return false;
    }

    // Configure video encoder
    VideoEncodingConfig encoderConfig;
    encoderConfig.outputPath = m_config.outputPath;
    encoderConfig.codec = m_config.codec;
    encoderConfig.width = m_config.width;
    encoderConfig.height = m_config.height;
    encoderConfig.frameRate = m_config.frameRate;
    encoderConfig.bitrate = getPresetBitrate(m_config.quality);

    if (!m_videoEncoder->configure(encoderConfig)) {
        m_lastError = m_videoEncoder->getLastError();
        return false;
    }

    if (!m_videoEncoder->start()) {
        m_lastError = m_videoEncoder->getLastError();
        return false;
    }

    // Simulate video encoding progress
    int64_t totalFrames = m_progress.totalFrames;
    for (int64_t frameNum = 0; frameNum < totalFrames && !m_cancelled; ++frameNum) {
        // Simulate frame encoding
        uint8_t dummyFrame[4] = {0, 0, 0, 0};
        m_videoEncoder->encodeFrame(dummyFrame, frameNum * (1000 / m_config.frameRate), false);

        // Update progress every 10 frames
        if (frameNum % 10 == 0) {
            m_progress.framesEncoded = frameNum;
            m_progress.videoProgress = static_cast<float>(frameNum) / static_cast<float>(totalFrames);
            m_progress.totalProgress = m_progress.videoProgress * 0.5f;  // Video is 50% of total

            auto currentTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            auto elapsedMs = currentTimeMs - m_startTimeMs;
            if (elapsedMs > 0) {
                float fps = (static_cast<float>(frameNum) * 1000.0f) / static_cast<float>(elapsedMs);
                m_progress.estimatedRemainingSeconds = static_cast<float>(totalFrames - frameNum) / std::max(1.0f, fps);
            }

            updateProgress();

            // Simulate some processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    m_videoEncoder->stop();
    m_progress.videoProgress = 1.0f;

    return !m_cancelled;
}

bool ExportManager::encodeAudio() {
    // Calculate total audio samples
    m_progress.totalAudioSamples = static_cast<int64_t>(
        (m_config.durationMs * m_config.audioSampleRate) / 1000);

    // Simulate audio encoding progress
    for (int64_t sample = 0; sample < m_progress.totalAudioSamples && !m_cancelled; ++sample) {
        m_progress.audioSamplesProcessed = sample;
        m_progress.audioProgress = static_cast<float>(sample) / static_cast<float>(m_progress.totalAudioSamples);
        m_progress.totalProgress = 0.5f + (m_progress.audioProgress * 0.25f);  // Audio is 25% of total

        if (sample % 44100 == 0) {  // Update every second
            updateProgress();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    m_progress.audioProgress = 1.0f;
    return !m_cancelled;
}

bool ExportManager::muxStreams() {
    // Simulate muxing progress
    int steps = 100;
    for (int step = 0; step < steps && !m_cancelled; ++step) {
        m_progress.muxingProgress = static_cast<float>(step) / static_cast<float>(steps);
        m_progress.totalProgress = 0.75f + (m_progress.muxingProgress * 0.25f);  // Muxing is 25% of total

        if (step % 10 == 0) {
            updateProgress();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    m_progress.muxingProgress = 1.0f;
    return !m_cancelled;
}

void ExportManager::updateProgress() {
    if (m_progressCallback) {
        m_progressCallback(m_progress);
    }
}

bool ExportManager::validateConfig() const {
    if (m_config.outputPath.empty()) {
        LOG_ERROR("ExportManager: Output path is empty");
        return false;
    }

    if (m_config.width <= 0 || m_config.height <= 0) {
        LOG_ERROR("ExportManager: Invalid resolution");
        return false;
    }

    if (m_config.frameRate <= 0 || m_config.frameRate > 120) {
        LOG_ERROR("ExportManager: Invalid frame rate");
        return false;
    }

    if (m_config.audioSampleRate <= 0) {
        LOG_ERROR("ExportManager: Invalid audio sample rate");
        return false;
    }

    if (m_config.audioChannels <= 0) {
        LOG_ERROR("ExportManager: Invalid audio channels");
        return false;
    }

    return true;
}

} // namespace encoding
} // namespace clipforge
