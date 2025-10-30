#ifndef CLIPFORGE_EXPORT_PRESETS_H
#define CLIPFORGE_EXPORT_PRESETS_H

#include <string>
#include <map>
#include <vector>

namespace clipforge {
namespace encoding {

/**
 * @struct ExportPreset
 * @brief Video export preset configuration
 */
struct ExportPreset {
    std::string name;
    int width;                  // Video width in pixels
    int height;                 // Video height in pixels
    int bitrate;                // Bitrate in kbps
    int audiobitrate;           // Audio bitrate in kbps
    float frameRate;            // Frames per second
    std::string videoCodec;     // h264, h265, vp9
    std::string audioCodec;     // aac, opus, flac
    bool hardwareAccel;         // Use hardware encoding
};

/**
 * @class ExportPresets
 * @brief Manages export quality presets
 *
 * Pre-configured presets for common platforms and use cases
 */
class ExportPresets {
public:
    /**
     * @brief Get preset by name
     * @param presetName Preset name
     * @return Export preset configuration
     */
    static ExportPreset getPreset(const std::string& presetName);

    /**
     * @brief Get all available presets
     * @return Map of preset names to configurations
     */
    static std::map<std::string, ExportPreset> getAllPresets();

    /**
     * @brief Get preset names for specific platform
     * @param platform Platform name (instagram, tiktok, youtube, twitter, etc.)
     * @return Vector of preset names
     */
    static std::vector<std::string> getPresetsForPlatform(const std::string& platform);

    /**
     * @brief Create custom preset
     * @param name Preset name
     * @param width Video width
     * @param height Video height
     * @param bitrate Bitrate in kbps
     * @param videoCodec Video codec
     * @param audioCodec Audio codec
     * @return Custom preset
     */
    static ExportPreset createCustomPreset(const std::string& name, int width, int height,
                                           int bitrate, const std::string& videoCodec,
                                           const std::string& audioCodec);

private:
    static std::map<std::string, ExportPreset> initializePresets();
};

/**
 * @enum SocialMediaPlatform
 * @brief Popular social media platforms
 */
enum class SocialMediaPlatform {
    INSTAGRAM_REEL = 0,     // 1080x1920, 9:16
    INSTAGRAM_FEED = 1,     // 1080x1080, 1:1
    INSTAGRAM_STORY = 2,    // 1080x1920, 9:16
    TIKTOK = 3,             // 1080x1920, 9:16
    YOUTUBE_SHORT = 4,      // 1080x1920, 9:16
    YOUTUBE_STANDARD = 5,   // 1920x1080, 16:9
    YOUTUBE_4K = 6,         // 3840x2160, 16:9
    TWITTER = 7,            // 1200x675, 16:9
    FACEBOOK = 8,           // 1200x628, 16:9
    SNAPCHAT = 9,           // 1080x1920, 9:16
    TWITCH = 10,            // 1920x1080, 16:9
    CUSTOM = 11,
};

/**
 * @class SocialMediaPresets
 * @brief Presets optimized for social media platforms
 */
class SocialMediaPresets {
public:
    /**
     * @brief Get preset for social media platform
     * @param platform Social media platform
     * @return Export preset optimized for that platform
     */
    static ExportPreset getPresetForPlatform(SocialMediaPlatform platform);

    /**
     * @brief Get all social media presets
     * @return Vector of available presets
     */
    static std::vector<ExportPreset> getAllSocialMediaPresets();

    /**
     * @brief Get recommended preset for multiple platforms
     * @param platforms Vector of platforms
     * @return Common preset that works for all platforms
     */
    static ExportPreset getCommonPreset(const std::vector<SocialMediaPlatform>& platforms);
};

/**
 * @class DevicePresets
 * @brief Device-optimized presets
 */
class DevicePresets {
public:
    enum class Device {
        PHONE_HD = 0,           // 720p
        PHONE_FULL_HD = 1,      // 1080p
        PHONE_ULTRA_HD = 2,     // 1440p
        TABLET_HD = 3,          // 720p
        TABLET_FULL_HD = 4,     // 1080p
        DESKTOP_HD = 5,         // 720p
        DESKTOP_FULL_HD = 6,    // 1080p
        DESKTOP_4K = 7,         // 2160p
    };

    /**
     * @brief Get preset for device
     * @param device Device type
     * @return Export preset optimized for device
     */
    static ExportPreset getPresetForDevice(Device device);
};

/**
 * @class QualityPresets
 * @brief Quality-based presets
 */
class QualityPresets {
public:
    enum class Quality {
        LOW = 0,        // 480p, 500 kbps
        MEDIUM = 1,     // 720p, 1000 kbps
        HIGH = 2,       // 1080p, 3000 kbps
        ULTRA = 3,      // 1440p, 6000 kbps
        CINEMA = 4,     // 2160p (4K), 12000 kbps
    };

    /**
     * @brief Get preset by quality level
     * @param quality Quality level
     * @param aspectRatio Aspect ratio (e.g., 16:9, 9:16)
     * @return Export preset
     */
    static ExportPreset getPresetByQuality(Quality quality, const std::string& aspectRatio = "16:9");

    /**
     * @brief Get file size estimate
     * @param preset Export preset
     * @param durationSeconds Video duration
     * @return Estimated file size in bytes
     */
    static int64_t estimateFileSize(const ExportPreset& preset, int durationSeconds);
};

} // namespace encoding
} // namespace clipforge

#endif // CLIPFORGE_EXPORT_PRESETS_H
