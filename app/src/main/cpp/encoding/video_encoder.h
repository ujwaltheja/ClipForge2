#ifndef CLIPFORGE_VIDEO_ENCODER_H
#define CLIPFORGE_VIDEO_ENCODER_H

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

namespace clipforge {
namespace encoding {

/**
 * @enum VideoCodec
 * @brief Supported video codecs
 */
enum class VideoCodec {
    H264,       // H.264 (AVC) - High compatibility
    H265,       // H.265 (HEVC) - Higher compression
    VP9,        // VP9 - Open format, high quality
};

/**
 * @enum ColorFormat
 * @brief Video frame color format
 */
enum class ColorFormat {
    NV12,       // YUV 4:2:0 (Native to most devices)
    NV21,       // YUV 4:2:0 (Variant)
    YUV420P,    // Planar YUV 4:2:0
    RGBA,       // 32-bit RGBA
};

/**
 * @enum VideoProfile
 * @brief H.264/H.265 encoding profiles
 */
enum class VideoProfile {
    BASELINE,   // H.264 baseline (most compatible)
    MAIN,       // H.264/H.265 main profile
    HIGH,       // H.264/H.265 high profile
};

/**
 * @enum BitrateMode
 * @brief Bitrate control mode
 */
enum class BitrateMode {
    VBR,        // Variable bitrate (quality-based)
    CBR,        // Constant bitrate
    CQ,         // Constant quality
};

/**
 * @struct VideoEncodingConfig
 * @brief Configuration for video encoding
 */
struct VideoEncodingConfig {
    // Output
    std::string outputPath;           // Output file path
    VideoCodec codec = VideoCodec::H264;
    std::string mimeType = "video/avc"; // MIME type for codec

    // Resolution
    int width = 1920;
    int height = 1080;
    int frameRate = 30;               // FPS

    // Bitrate
    int bitrate = 8000000;            // bits per second (8 Mbps)
    BitrateMode bitrateMode = BitrateMode::VBR;

    // Encoding
    VideoProfile profile = VideoProfile::HIGH;
    int iFrameInterval = 1;           // IDR interval in seconds
    int keyFrameInterval = 30;        // Keyframe interval in frames
    int bframes = 0;                  // B-frame count (H.264/H.265)

    // Hardware
    bool useHardwareEncoding = true;
    bool realizeSurface = true;       // Use surface encoding

    // Quality
    int qualityLevel = 25;            // 0-51 (0=highest, 51=lowest)

    // Input
    ColorFormat inputFormat = ColorFormat::NV12;
    int colorSpace = 0;               // SMPTE 170M (0) or BT.709 (1)

    // Flags
    bool enableLowLatency = false;
    bool enablePTSOffset = true;
};

/**
 * @struct EncodingStats
 * @brief Statistics from encoding operation
 */
struct EncodingStats {
    int64_t frameCount = 0;
    int64_t totalFrames = 0;
    float encodedPercentage = 0.0f;
    float encodingFPS = 0.0f;
    int64_t bytesEncoded = 0;
    float estimatedTimeRemainingSeconds = 0.0f;
    std::string currentStatus;
};

/**
 * @class VideoEncoder
 * @brief Hardware-accelerated video encoder
 *
 * Provides interface to Android's MediaCodec for video encoding.
 * Supports multiple codecs and quality presets with real-time
 * progress monitoring.
 *
 * Usage:
 * @code
 * auto encoder = std::make_shared<VideoEncoder>();
 * VideoEncodingConfig config;
 * config.codec = VideoCodec::H264;
 * config.bitrate = 8000000;
 *
 * if (encoder->configure(config)) {
 *     encoder->start();
 *     // Feed frames
 *     encoder->encodeFrame(frameData, timestampMs);
 *     // Monitor progress
 *     auto stats = encoder->getStats();
 * }
 * @endcode
 */
class VideoEncoder {
public:
    /**
     * @brief Create video encoder
     */
    VideoEncoder();

    /**
     * @brief Destructor
     */
    ~VideoEncoder();

    // Prevent copying
    VideoEncoder(const VideoEncoder&) = delete;
    VideoEncoder& operator=(const VideoEncoder&) = delete;

    // ===== Configuration =====

    /**
     * @brief Configure encoder with parameters
     * @param config Encoding configuration
     * @return true if configuration successful
     */
    bool configure(const VideoEncodingConfig& config);

    /**
     * @brief Get current configuration
     * @return Current encoding config
     */
    [[nodiscard]] const VideoEncodingConfig& getConfig() const { return m_config; }

    /**
     * @brief Check if encoder is configured
     * @return true if ready to start
     */
    [[nodiscard]] bool isConfigured() const { return m_configured; }

    // ===== Encoding Control =====

    /**
     * @brief Start encoding
     * @return true if started successfully
     */
    bool start();

    /**
     * @brief Stop encoding and finalize output
     * @return true if stopped successfully
     */
    bool stop();

    /**
     * @brief Check if encoding is in progress
     * @return true if actively encoding
     */
    [[nodiscard]] bool isEncoding() const { return m_encoding; }

    /**
     * @brief Pause encoding (can be resumed)
     * @return true if paused
     */
    bool pause();

    /**
     * @brief Resume paused encoding
     * @return true if resumed
     */
    bool resume();

    // ===== Frame Input =====

    /**
     * @brief Encode raw frame data
     * @param frameData Frame data pointer
     * @param timestampMs Frame timestamp in milliseconds
     * @param isKeyFrame Force keyframe (IDR)
     * @return true if frame queued successfully
     */
    bool encodeFrame(const uint8_t* frameData, int64_t timestampMs, bool isKeyFrame = false);

    /**
     * @brief Queue frame with stride information
     * @param frameData Frame data pointer
     * @param width Frame width in pixels
     * @param height Frame height in pixels
     * @param stride Bytes per row (may differ from width)
     * @param timestampMs Frame timestamp
     * @return true if successful
     */
    bool encodeFrameWithStride(const uint8_t* frameData, int width, int height,
                              int stride, int64_t timestampMs);

    /**
     * @brief Get input buffer for direct writing (zero-copy)
     * @param size Size of buffer needed
     * @return Pointer to buffer, nullptr if unavailable
     */
    [[nodiscard]] uint8_t* getInputBuffer(size_t size);

    /**
     * @brief Submit input buffer for encoding
     * @param data Data pointer (from getInputBuffer)
     * @param size Data size
     * @param timestampMs Frame timestamp
     * @return true if submitted
     */
    bool submitInputBuffer(uint8_t* data, size_t size, int64_t timestampMs);

    // ===== Monitoring =====

    /**
     * @brief Get encoding statistics
     * @return Current stats
     */
    [[nodiscard]] const EncodingStats& getStats() const { return m_stats; }

    /**
     * @brief Get frames encoded so far
     * @return Frame count
     */
    [[nodiscard]] int64_t getFramesEncoded() const { return m_stats.frameCount; }

    /**
     * @brief Get encoding progress percentage
     * @return Percentage (0-100)
     */
    [[nodiscard]] float getProgress() const { return m_stats.encodedPercentage; }

    /**
     * @brief Get estimated time remaining
     * @return Seconds remaining
     */
    [[nodiscard]] float getEstimatedTimeRemaining() const {
        return m_stats.estimatedTimeRemainingSeconds;
    }

    /**
     * @brief Get current encoding FPS
     * @return Frames per second
     */
    [[nodiscard]] float getEncodingFPS() const { return m_stats.encodingFPS; }

    /**
     * @brief Get bytes written to output
     * @return Byte count
     */
    [[nodiscard]] int64_t getBytesEncoded() const { return m_stats.bytesEncoded; }

    // ===== Quality Control =====

    /**
     * @brief Set bitrate dynamically during encoding
     * @param bitrate New bitrate in bps
     * @return true if set successfully
     */
    bool setBitrate(int bitrate);

    /**
     * @brief Force next frame to be keyframe (IDR)
     * @return true if queued
     */
    bool forceKeyframe();

    /**
     * @brief Adjust quality level
     * @param level Quality level (0-51, 0=highest)
     * @return true if set
     */
    bool setQualityLevel(int level);

    // ===== Codec Information =====

    /**
     * @brief Check if codec is supported on this device
     * @param codec Codec to check
     * @return true if supported
     */
    [[nodiscard]] static bool isCodecSupported(VideoCodec codec);

    /**
     * @brief Get supported bitrate range for codec
     * @param codec Video codec
     * @param width Video width
     * @param height Video height
     * @return {minBitrate, maxBitrate}
     */
    [[nodiscard]] static std::pair<int, int> getBitrateRange(
        VideoCodec codec, int width, int height);

    /**
     * @brief Get recommended bitrate for quality
     * @param width Video width
     * @param height Video height
     * @param frameRate FPS
     * @param qualityLevel Quality (0=high, 1=medium, 2=low)
     * @return Recommended bitrate in bps
     */
    [[nodiscard]] static int getRecommendedBitrate(
        int width, int height, int frameRate, int qualityLevel);

    // ===== Error Handling =====

    /**
     * @brief Get last error message
     * @return Error description
     */
    [[nodiscard]] std::string getLastError() const { return m_lastError; }

    /**
     * @brief Check if encoder is in error state
     * @return true if error occurred
     */
    [[nodiscard]] bool hasError() const { return !m_lastError.empty(); }

    /**
     * @brief Clear error state
     */
    void clearError() { m_lastError.clear(); }

private:
    VideoEncodingConfig m_config;
    bool m_configured = false;
    bool m_encoding = false;
    bool m_paused = false;
    EncodingStats m_stats;
    std::string m_lastError;

    // Hardware encoder pointers
    void* m_mediaCodec = nullptr;
    void* m_inputSurface = nullptr;

    /**
     * @brief Initialize MediaCodec
     * @return true if successful
     */
    bool initializeMediaCodec();

    /**
     * @brief Release MediaCodec resources
     */
    void releaseMediaCodec();

    /**
     * @brief Process output buffers
     * @return Number of bytes written
     */
    int64_t processOutputBuffers();

    /**
     * @brief Calculate bitrate from quality level
     * @return Bitrate in bps
     */
    [[nodiscard]] int calculateBitrate() const;

    /**
     * @brief Convert codec enum to MIME type
     * @param codec Video codec
     * @return MIME type string
     */
    [[nodiscard]] static std::string codecToMimeType(VideoCodec codec);
};

} // namespace encoding
} // namespace clipforge

#endif // CLIPFORGE_VIDEO_ENCODER_H
