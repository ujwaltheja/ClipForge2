#ifndef CLIPFORGE_TEXT_RENDERER_H
#define CLIPFORGE_TEXT_RENDERER_H

#include <string>
#include <vector>
#include <cstdint>
#include <memory>

namespace clipforge {
namespace graphics {

/**
 * @enum TextAlignment
 * @brief Text alignment options
 */
enum class TextAlignment {
    LEFT = 0,
    CENTER = 1,
    RIGHT = 2,
};

/**
 * @enum TextStyle
 * @brief Text styling options
 */
enum class TextStyle {
    NORMAL = 0,
    BOLD = 1,
    ITALIC = 2,
    BOLD_ITALIC = 3,
    OUTLINE = 4,
    SHADOW = 5,
};

/**
 * @struct TextProperties
 * @brief Text rendering properties
 */
struct TextProperties {
    std::string text;
    std::string fontName = "Roboto";
    int fontSize = 48;                      // In pixels
    uint32_t color = 0xFFFFFFFF;            // RGBA color
    TextAlignment alignment = TextAlignment::CENTER;
    TextStyle style = TextStyle::NORMAL;

    // Effects
    float opacity = 1.0f;                   // 0-1
    float outlineWidth = 0.0f;              // In pixels
    uint32_t outlineColor = 0xFF000000;     // RGBA

    // Shadow
    float shadowOffsetX = 0.0f;
    float shadowOffsetY = 0.0f;
    float shadowBlur = 0.0f;
    uint32_t shadowColor = 0x80000000;      // RGBA with alpha

    // Positioning
    float posX = 0.5f;                      // 0-1 (normalized)
    float posY = 0.5f;                      // 0-1 (normalized)
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotationDegrees = 0.0f;

    // Animation
    float animationStartMs = 0.0f;
    float animationDurationMs = 0.0f;
};

/**
 * @enum TextAnimation
 * @brief Text entrance/exit animations
 */
enum class TextAnimation {
    NONE = 0,
    FADE_IN = 1,
    FADE_OUT = 2,
    SLIDE_IN_LEFT = 3,
    SLIDE_IN_RIGHT = 4,
    SLIDE_IN_UP = 5,
    SLIDE_IN_DOWN = 6,
    SCALE_IN = 7,
    SCALE_OUT = 8,
    ROTATE_IN = 9,
    TYPEWRITER = 10,
    BOUNCE_IN = 11,
};

/**
 * @class TextRenderer
 * @brief Renders text overlays on video frames
 *
 * Features:
 * - Multiple font support
 * - Text styling (bold, italic, outline, shadow)
 * - Text animations (fade, slide, scale, rotate)
 * - Color gradients
 * - Kerning and spacing control
 * - Multi-line text support
 * - Text path/curved text
 */
class TextRenderer {
public:
    /**
     * @brief Create text renderer
     */
    TextRenderer();

    /**
     * @brief Destructor
     */
    ~TextRenderer();

    /**
     * @brief Render text to frame
     * @param frameData Output frame buffer (RGBA)
     * @param width Frame width
     * @param height Frame height
     * @param props Text properties
     * @param timeMs Current animation time
     */
    void renderText(uint8_t* frameData, int width, int height,
                   const TextProperties& props, int64_t timeMs);

    /**
     * @brief Render animated text
     * @param frameData Output frame buffer
     * @param width Frame width
     * @param height Frame height
     * @param props Text properties
     * @param animation Animation type
     * @param timeMs Current time
     * @param totalDurationMs Total animation duration
     */
    void renderAnimatedText(uint8_t* frameData, int width, int height,
                           const TextProperties& props,
                           TextAnimation animation, int64_t timeMs,
                           int64_t totalDurationMs);

    /**
     * @brief Get text bounding box
     * @param props Text properties
     * @param outWidth Bounding box width
     * @param outHeight Bounding box height
     */
    void getTextBounds(const TextProperties& props, int& outWidth, int& outHeight);

    /**
     * @brief Set font directory for custom fonts
     * @param fontDir Directory path
     */
    void setFontDirectory(const std::string& fontDir);

    /**
     * @brief Load custom font
     * @param fontName Font identifier
     * @param fontPath Path to font file
     * @return true if loaded successfully
     */
    bool loadFont(const std::string& fontName, const std::string& fontPath);

    /**
     * @brief Get available system fonts
     * @return Vector of font names
     */
    [[nodiscard]] std::vector<std::string> getAvailableFonts() const;

private:
    std::string m_fontDir;
    std::vector<std::pair<std::string, std::string>> m_loadedFonts;

    // Text rendering helpers
    void renderOutline(uint8_t* frameData, int width, int height,
                      const TextProperties& props);

    void renderShadow(uint8_t* frameData, int width, int height,
                     const TextProperties& props);

    // Animation helpers
    float calculateAnimationProgress(int64_t timeMs, int64_t totalDurationMs);

    void applyFadeAnimation(uint8_t* frameData, int width, int height,
                           float progress, bool fadeIn);

    void applySlideAnimation(uint8_t* frameData, int width, int height,
                            float progress, int direction);

    void applyScaleAnimation(uint8_t* frameData, int width, int height,
                            float progress, bool scaleIn);

    void applyRotateAnimation(uint8_t* frameData, int width, int height,
                             float progress);

    void applyTypewriterAnimation(const std::string& text, uint8_t* frameData,
                                 int width, int height, float progress,
                                 const TextProperties& props);

    // Utility
    void blendPixel(uint8_t* pixelData, uint32_t color, float alpha);
};

/**
 * @class MotionGraphics
 * @brief Motion graphics and animated shapes
 */
class MotionGraphics {
public:
    /**
     * @brief Draw animated rectangle
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param x Top-left X position (normalized 0-1)
     * @param y Top-left Y position (normalized 0-1)
     * @param w Width (normalized 0-1)
     * @param h Height (normalized 0-1)
     * @param color RGBA color
     * @param borderRadius Corner radius
     * @param opacity Opacity 0-1
     */
    static void drawRectangle(uint8_t* frameData, int width, int height,
                             float x, float y, float w, float h,
                             uint32_t color, float borderRadius = 0.0f,
                             float opacity = 1.0f);

    /**
     * @brief Draw animated circle
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param centerX Center X (normalized 0-1)
     * @param centerY Center Y (normalized 0-1)
     * @param radius Radius (normalized 0-1)
     * @param color RGBA color
     * @param opacity Opacity 0-1
     */
    static void drawCircle(uint8_t* frameData, int width, int height,
                          float centerX, float centerY, float radius,
                          uint32_t color, float opacity = 1.0f);

    /**
     * @brief Draw line
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param x1 Start X (normalized)
     * @param y1 Start Y (normalized)
     * @param x2 End X (normalized)
     * @param y2 End Y (normalized)
     * @param color RGBA color
     * @param thickness Line thickness in pixels
     */
    static void drawLine(uint8_t* frameData, int width, int height,
                        float x1, float y1, float x2, float y2,
                        uint32_t color, float thickness = 1.0f);

    /**
     * @brief Draw polygon
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param points Vector of normalized points (0-1)
     * @param color RGBA color
     * @param filled Fill polygon or outline
     */
    static void drawPolygon(uint8_t* frameData, int width, int height,
                           const std::vector<std::pair<float, float>>& points,
                           uint32_t color, bool filled = true);

    /**
     * @brief Draw particle effect
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param centerX Center X (normalized)
     * @param centerY Center Y (normalized)
     * @param particleCount Number of particles
     * @param color RGBA color
     * @param spread Spread angle in degrees
     * @param progress Animation progress 0-1
     */
    static void drawParticleEffect(uint8_t* frameData, int width, int height,
                                   float centerX, float centerY,
                                   int particleCount, uint32_t color,
                                   float spread = 360.0f, float progress = 0.5f);

    /**
     * @brief Draw gradient background
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param color1 Start color (RGBA)
     * @param color2 End color (RGBA)
     * @param horizontal true for horizontal, false for vertical
     */
    static void drawGradient(uint8_t* frameData, int width, int height,
                            uint32_t color1, uint32_t color2, bool horizontal = true);
};

/**
 * @class SubtitleRenderer
 * @brief Renders subtitles on video
 */
class SubtitleRenderer {
public:
    /**
     * @struct SubtitleEntry
     * @brief Single subtitle entry
     */
    struct SubtitleEntry {
        int64_t startMs;        // Start time
        int64_t endMs;          // End time
        std::string text;       // Subtitle text
        TextProperties style;   // Text styling
    };

    /**
     * @brief Create subtitle renderer
     */
    SubtitleRenderer();

    /**
     * @brief Add subtitle entry
     * @param entry Subtitle entry
     */
    void addSubtitle(const SubtitleEntry& entry);

    /**
     * @brief Remove subtitle at time
     * @param timeMs Time in milliseconds
     */
    void removeSubtitlesAtTime(int64_t timeMs);

    /**
     * @brief Render subtitles at time
     * @param frameData Output frame
     * @param width Frame width
     * @param height Frame height
     * @param timeMs Current time
     */
    void renderSubtitles(uint8_t* frameData, int width, int height, int64_t timeMs);

    /**
     * @brief Load subtitles from SRT file
     * @param srtPath Path to SRT file
     * @return true if loaded successfully
     */
    bool loadFromSRT(const std::string& srtPath);

    /**
     * @brief Export subtitles to SRT
     * @param outputPath Path to save SRT file
     * @return true if exported successfully
     */
    bool exportToSRT(const std::string& outputPath) const;

    /**
     * @brief Get all subtitles
     * @return Vector of subtitle entries
     */
    [[nodiscard]] const std::vector<SubtitleEntry>& getSubtitles() const {
        return m_subtitles;
    }

private:
    std::vector<SubtitleEntry> m_subtitles;
    TextRenderer m_textRenderer;
};

} // namespace graphics
} // namespace clipforge

#endif // CLIPFORGE_TEXT_RENDERER_H
