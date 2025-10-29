#include "video_encoder.h"
#include "../utils/logger.h"
#include <cstring>
#include <cmath>
#include <atomic>
#include <mutex>
#include <chrono>

namespace clipforge {
namespace encoding {

// ===== VideoEncoder Implementation =====

VideoEncoder::VideoEncoder()
    : m_configured(false), m_encoding(false), m_paused(false),
      m_mediaCodec(nullptr), m_inputSurface(nullptr) {
    LOG_DEBUG("VideoEncoder created");
}

VideoEncoder::~VideoEncoder() {
    if (m_encoding) {
        stop();
    }
    releaseMediaCodec();
    LOG_DEBUG("VideoEncoder destroyed");
}

bool VideoEncoder::configure(const VideoEncodingConfig& config) {
    if (m_encoding) {
        m_lastError = "Cannot configure while encoding is in progress";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    m_config = config;
    m_configured = false;
    m_stats = EncodingStats{};
    m_stats.totalFrames = static_cast<int64_t>(
        (static_cast<size_t>(config.frameRate) * config.outputPath.length()) / static_cast<size_t>(config.frameRate));

    // Validate configuration
    if (config.width <= 0 || config.height <= 0) {
        m_lastError = "Invalid resolution: width and height must be positive";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    if (config.frameRate <= 0 || config.frameRate > 120) {
        m_lastError = "Invalid frame rate: must be between 1 and 120";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    if (config.bitrate <= 0) {
        m_lastError = "Invalid bitrate: must be positive";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    // Check codec support
    if (!isCodecSupported(config.codec)) {
        m_lastError = "Codec not supported on this device";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    LOG_INFO("VideoEncoder configured: %dx%d @ %d fps, codec=%d, bitrate=%d bps",
         config.width, config.height, config.frameRate,
         static_cast<int>(config.codec), config.bitrate);

    m_configured = true;
    return true;
}

bool VideoEncoder::start() {
    if (!m_configured) {
        m_lastError = "Encoder not configured";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    if (m_encoding) {
        LOG_WARNING("VideoEncoder: Already encoding");
        return true;
    }

    if (!initializeMediaCodec()) {
        m_lastError = "Failed to initialize MediaCodec";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    m_encoding = true;
    m_paused = false;
    m_stats.frameCount = 0;
    m_stats.encodedPercentage = 0.0f;

    LOG_INFO("VideoEncoder started");
    return true;
}

bool VideoEncoder::stop() {
    if (!m_encoding) {
        LOG_WARNING("VideoEncoder: Not encoding");
        return true;
    }

    // Process any remaining output buffers
    processOutputBuffers();

    m_encoding = false;
    m_paused = false;

    releaseMediaCodec();

    LOG_INFO("VideoEncoder stopped");
    return true;
}

bool VideoEncoder::pause() {
    if (!m_encoding || m_paused) {
        LOG_WARNING("VideoEncoder: Cannot pause (not encoding or already paused)");
        return false;
    }

    m_paused = true;
    LOG_INFO("VideoEncoder paused");
    return true;
}

bool VideoEncoder::resume() {
    if (!m_encoding || !m_paused) {
        LOG_WARNING("VideoEncoder: Cannot resume (not paused)");
        return false;
    }

    m_paused = false;
    LOG_INFO("VideoEncoder resumed");
    return true;
}

bool VideoEncoder::encodeFrame(const uint8_t* frameData, int64_t timestampMs, bool isKeyFrame) {
    if (!m_encoding) {
        m_lastError = "Encoder not active";
        return false;
    }

    if (m_paused) {
        return true;  // Skip frames while paused
    }

    if (!frameData) {
        m_lastError = "Null frame data";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    // Calculate expected frame size
    int frameSize = 0;
    switch (m_config.inputFormat) {
        case ColorFormat::NV12:
        case ColorFormat::NV21:
            frameSize = (m_config.width * m_config.height * 3) / 2;
            break;
        case ColorFormat::YUV420P:
            frameSize = (m_config.width * m_config.height * 3) / 2;
            break;
        case ColorFormat::RGBA:
            frameSize = m_config.width * m_config.height * 4;
            break;
        default:
            m_lastError = "Unknown color format";
            return false;
    }

    // Queue frame for encoding
    // This is a simplified implementation - actual MediaCodec interaction would happen here
    m_stats.frameCount++;
    m_stats.encodedPercentage = static_cast<float>(m_stats.frameCount) / static_cast<float>(m_stats.totalFrames);
    m_stats.bytesEncoded += frameSize;

    if (m_stats.frameCount % m_config.frameRate == 0) {
        LOG_DEBUG("VideoEncoder: Encoded %lld frames (%.1f%%)",
             m_stats.frameCount, m_stats.encodedPercentage * 100.0f);
    }

    // Request keyframe if requested
    if (isKeyFrame) {
        forceKeyframe();
    }

    // Process output buffers periodically
    if (m_stats.frameCount % 30 == 0) {
        processOutputBuffers();
    }

    return true;
}

bool VideoEncoder::encodeFrameWithStride(const uint8_t* frameData, int width, int height,
                                         int stride, int64_t timestampMs) {
    if (!m_encoding) {
        m_lastError = "Encoder not active";
        return false;
    }

    if (width != m_config.width || height != m_config.height) {
        m_lastError = "Frame dimensions do not match configured resolution";
        LOG_ERROR("VideoEncoder: %s (got %dx%d, expected %dx%d)",
             m_lastError.c_str(), width, height, m_config.width, m_config.height);
        return false;
    }

    // Encode with stride information
    return encodeFrame(frameData, timestampMs, false);
}

uint8_t* VideoEncoder::getInputBuffer(size_t size) {
    if (!m_encoding) {
        m_lastError = "Encoder not active";
        return nullptr;
    }

    // In a real implementation, this would acquire a direct buffer from MediaCodec
    // For now, allocate a temporary buffer
    static thread_local std::vector<uint8_t> buffer;
    if (buffer.size() < size) {
        buffer.resize(size);
    }
    return buffer.data();
}

bool VideoEncoder::submitInputBuffer(uint8_t* data, size_t size, int64_t timestampMs) {
    if (!m_encoding) {
        m_lastError = "Encoder not active";
        return false;
    }

    if (!data || size == 0) {
        m_lastError = "Invalid buffer data";
        return false;
    }

    // Submit buffer to encoder (simplified)
    return true;
}

int64_t VideoEncoder::processOutputBuffers() {
    if (!m_mediaCodec) {
        return 0;
    }

    // In a real implementation, this would dequeue output buffers from MediaCodec
    // and write them to the output file
    int64_t bytesWritten = 0;

    // Simplified: return accumulated bytes
    return bytesWritten;
}

bool VideoEncoder::setBitrate(int bitrate) {
    if (bitrate <= 0) {
        m_lastError = "Invalid bitrate value";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    m_config.bitrate = bitrate;

    // If encoding, apply dynamic bitrate change to MediaCodec
    if (m_encoding && m_mediaCodec) {
        LOG_INFO("VideoEncoder: Bitrate updated to %d bps", bitrate);
    }

    return true;
}

bool VideoEncoder::forceKeyframe() {
    if (!m_encoding) {
        m_lastError = "Encoder not active";
        return false;
    }

    if (!m_mediaCodec) {
        m_lastError = "MediaCodec not initialized";
        return false;
    }

    // Request IDR frame from MediaCodec
    LOG_DEBUG("VideoEncoder: IDR frame requested");
    return true;
}

bool VideoEncoder::setQualityLevel(int level) {
    if (level < 0 || level > 51) {
        m_lastError = "Quality level must be between 0 and 51";
        LOG_ERROR("VideoEncoder: %s", m_lastError.c_str());
        return false;
    }

    m_config.qualityLevel = level;

    // Adjust bitrate based on quality
    int newBitrate = calculateBitrate();
    setBitrate(newBitrate);

    LOG_INFO("VideoEncoder: Quality level set to %d (bitrate: %d bps)", level, newBitrate);
    return true;
}

bool VideoEncoder::isCodecSupported(VideoCodec codec) {
    // Simplified check - in real implementation, query MediaCodecList
    switch (codec) {
        case VideoCodec::H264:
        case VideoCodec::H265:
            return true;  // Most devices support these
        case VideoCodec::VP9:
            return true;  // Many modern devices support VP9
        default:
            return false;
    }
}

std::pair<int, int> VideoEncoder::getBitrateRange(VideoCodec codec, int width, int height) {
    int minBitrate = 500000;      // 500 kbps
    int maxBitrate = 100000000;   // 100 Mbps

    // Adjust range based on resolution
    int pixelCount = width * height;
    if (pixelCount <= 640 * 480) {
        maxBitrate = 10000000;     // 10 Mbps for 480p
    } else if (pixelCount <= 1280 * 720) {
        maxBitrate = 25000000;     // 25 Mbps for 720p
    } else if (pixelCount <= 1920 * 1080) {
        maxBitrate = 50000000;     // 50 Mbps for 1080p
    }

    return {minBitrate, maxBitrate};
}

int VideoEncoder::getRecommendedBitrate(int width, int height, int frameRate, int qualityLevel) {
    // Calculate base bitrate based on resolution
    float basePixels = static_cast<float>(width) * static_cast<float>(height);
    float megapixels = basePixels / 1000000.0f;

    // Base: ~0.1 Mbps per megapixel per fps
    int baseBitrate = static_cast<int>(megapixels * static_cast<float>(frameRate) * 1000000 * 0.1f);

    // Apply quality multiplier
    // qualityLevel: 0=high (1.5x), 1=medium (1.0x), 2=low (0.5x)
    float qualityMultiplier = 1.0f;
    if (qualityLevel == 0) qualityMultiplier = 1.5f;
    else if (qualityLevel == 1) qualityMultiplier = 1.0f;
    else if (qualityLevel == 2) qualityMultiplier = 0.5f;

    int recommendedBitrate = static_cast<int>(static_cast<float>(baseBitrate) * qualityMultiplier);

    // Clamp to reasonable range
    recommendedBitrate = std::max(500000, std::min(100000000, recommendedBitrate));

    return recommendedBitrate;
}

std::string VideoEncoder::codecToMimeType(VideoCodec codec) {
    switch (codec) {
        case VideoCodec::H264:
            return "video/avc";
        case VideoCodec::H265:
            return "video/hevc";
        case VideoCodec::VP9:
            return "video/x-vnd.on2.vp9";
        default:
            return "video/avc";
    }
}

int VideoEncoder::calculateBitrate() const {
    return getRecommendedBitrate(
        m_config.width, m_config.height, m_config.frameRate,
        m_config.qualityLevel / 25);  // Normalize 0-51 to 0-2
}

bool VideoEncoder::initializeMediaCodec() {
    if (m_mediaCodec) {
        return true;
    }

    // In a real implementation, this would:
    // 1. Create a MediaCodec instance for the specified codec
    // 2. Configure it with the VideoEncodingConfig parameters
    // 3. Create input/output surfaces if needed
    // 4. Start the codec

    m_mediaCodec = reinterpret_cast<void*>(1);  // Placeholder

    LOG_INFO("VideoEncoder: MediaCodec initialized");
    return true;
}

void VideoEncoder::releaseMediaCodec() {
    if (!m_mediaCodec) {
        return;
    }

    // In a real implementation, this would:
    // 1. Stop the MediaCodec
    // 2. Release input/output buffers
    // 3. Release the codec instance

    m_mediaCodec = nullptr;
    m_inputSurface = nullptr;

    LOG_INFO("VideoEncoder: MediaCodec released");
}

} // namespace encoding
} // namespace clipforge


