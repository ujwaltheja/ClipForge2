#ifndef CLIPFORGE_OPENGL_CONTEXT_H
#define CLIPFORGE_OPENGL_CONTEXT_H

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <memory>
#include <vector>
#include <cstdint>

namespace clipforge {
namespace gpu {

/**
 * @struct OpenGLCapabilities
 * @brief OpenGL ES capabilities detected at runtime
 */
struct OpenGLCapabilities {
    int majorVersion = 3;
    int minorVersion = 0;
    std::vector<std::string> extensions;
    int maxTextureSize = 2048;
    int maxRenderBufferSize = 2048;
    int maxUniformVectors = 256;
    bool supportsFramebufferObject = false;
    bool supportsTextureFloat = false;
    bool supportsDepthTexture = false;
    bool supportsHalfFloat = false;
    int maxColorAttachments = 4;
};

/**
 * @class OpenGLContext
 * @brief Manages EGL context, display, and surface for OpenGL ES 3.0 rendering
 *
 * Handles initialization of:
 * - EGL display and context
 * - Framebuffer objects (FBO) for offscreen rendering
 * - Texture management
 * - Capability detection and validation
 */
class OpenGLContext {
public:
    /**
     * @brief Create OpenGL ES 3.0 context
     * Requires a valid ANativeWindow or uses PBuffer for offscreen rendering
     *
     * @param displayWidth Window/texture width
     * @param displayHeight Window/texture height
     */
    OpenGLContext(int displayWidth = 1920, int displayHeight = 1080);

    /**
     * @brief Destructor - cleanup EGL resources
     */
    ~OpenGLContext();

    // Prevent copying
    OpenGLContext(const OpenGLContext&) = delete;
    OpenGLContext& operator=(const OpenGLContext&) = delete;

    // ===== Initialization =====

    /**
     * @brief Initialize EGL and create context
     * @return true if successful
     */
    bool initialize();

    /**
     * @brief Make context current for rendering
     * @return true if successful
     */
    bool makeCurrent();

    /**
     * @brief Release context (swap it out as current)
     * @return true if successful
     */
    bool releaseContext();

    /**
     * @brief Destroy context and cleanup resources
     */
    void shutdown();

    /**
     * @brief Check if context is initialized
     * @return true if context exists and is ready
     */
    [[nodiscard]] bool isInitialized() const { return m_display != EGL_NO_DISPLAY && m_context != EGL_NO_CONTEXT; }

    // ===== Capabilities =====

    /**
     * @brief Get detected OpenGL capabilities
     * @return Capabilities struct
     */
    [[nodiscard]] const OpenGLCapabilities& getCapabilities() const { return m_capabilities; }

    /**
     * @brief Validate required extensions are available
     * @return true if all required extensions present
     */
    [[nodiscard]] bool validateRequiredExtensions() const;

    /**
     * @brief Check specific extension support
     * @param extension Extension name (e.g., "GL_EXT_texture_storage")
     * @return true if supported
     */
    [[nodiscard]] bool hasExtension(const std::string& extension) const;

    // ===== Framebuffer Objects =====

    /**
     * @brief Create framebuffer object for offscreen rendering
     * @param width Texture width
     * @param height Texture height
     * @param colorFormat Color format (GL_RGBA, GL_RGB565, etc.)
     * @return Framebuffer ID, 0 on error
     */
    GLuint createFramebuffer(int width, int height, GLenum colorFormat = GL_RGBA);

    /**
     * @brief Delete framebuffer object
     * @param fbo Framebuffer ID
     */
    void deleteFramebuffer(GLuint fbo);

    /**
     * @brief Bind framebuffer for rendering to texture
     * @param fbo Framebuffer ID (0 for default framebuffer)
     * @return true if successful
     */
    bool bindFramebuffer(GLuint fbo = 0);

    /**
     * @brief Get framebuffer texture attachment
     * @param fbo Framebuffer ID
     * @return Color texture handle
     */
    [[nodiscard]] GLuint getFramebufferTexture(GLuint fbo) const;

    // ===== Texture Management =====

    /**
     * @brief Create texture from image data
     * @param width Texture width
     * @param height Texture height
     * @param data Image data pointer (RGBA format)
     * @param format Data format (GL_RGBA, GL_RGB, etc.)
     * @return Texture ID, 0 on error
     */
    GLuint createTexture(int width, int height, const uint8_t* data = nullptr,
                        GLenum format = GL_RGBA);

    /**
     * @brief Create texture for video frame data
     * @param width Frame width
     * @param height Frame height
     * @return Texture ID
     */
    GLuint createVideoFrameTexture(int width, int height);

    /**
     * @brief Update texture with new data
     * @param textureId Texture to update
     * @param data New image data
     * @param width Data width
     * @param height Data height
     * @return true if successful
     */
    bool updateTexture(GLuint textureId, const uint8_t* data, int width, int height);

    /**
     * @brief Delete texture
     * @param textureId Texture ID
     */
    void deleteTexture(GLuint textureId);

    /**
     * @brief Get next available texture unit
     * @return GL_TEXTURE0, GL_TEXTURE1, etc.
     */
    [[nodiscard]] GLenum getNextTextureUnit();

    /**
     * @brief Reset texture unit counter
     */
    void resetTextureUnits() { m_currentTextureUnit = 0; }

    // ===== Rendering =====

    /**
     * @brief Clear screen with color
     * @param r Red (0.0-1.0)
     * @param g Green (0.0-1.0)
     * @param b Blue (0.0-1.0)
     * @param a Alpha (0.0-1.0)
     */
    void clearScreen(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f);

    /**
     * @brief Set viewport dimensions
     * @param x X offset
     * @param y Y offset
     * @param width Viewport width
     * @param height Viewport height
     */
    void setViewport(int x, int y, int width, int height);

    /**
     * @brief Swap buffers (present rendered frame)
     * @return true if successful
     */
    bool swapBuffers();

    /**
     * @brief Wait for GPU to finish rendering (glFinish)
     */
    void waitForGPU();

    /**
     * @brief Get current OpenGL error
     * @return GL error code
     */
    [[nodiscard]] GLenum getGLError() const;

    /**
     * @brief Get OpenGL error description
     * @param error GL error code
     * @return Human-readable error string
     */
    [[nodiscard]] static std::string getGLErrorString(GLenum error);

    // ===== Display Info =====

    /**
     * @brief Get display width
     * @return Width in pixels
     */
    [[nodiscard]] int getWidth() const { return m_width; }

    /**
     * @brief Get display height
     * @return Height in pixels
     */
    [[nodiscard]] int getHeight() const { return m_height; }

    /**
     * @brief Get display aspect ratio
     * @return Width / Height
     */
    [[nodiscard]] float getAspectRatio() const {
        return m_height > 0 ? static_cast<float>(m_width) / static_cast<float>(m_height) : 1.0f;
    }

private:
    // EGL objects
    EGLDisplay m_display = EGL_NO_DISPLAY;
    EGLContext m_context = EGL_NO_CONTEXT;
    EGLSurface m_surface = EGL_NO_SURFACE;

    // Display dimensions
    int m_width = 1920;
    int m_height = 1080;

    // Capabilities
    OpenGLCapabilities m_capabilities;

    // Texture management
    int m_currentTextureUnit = 0;
    static constexpr int MAX_TEXTURE_UNITS = 16;

    /**
     * @brief Query and cache OpenGL capabilities
     */
    void detectCapabilities();

    /**
     * @brief Create EGL configuration
     * @return EGL config, or EGL_NO_CONFIG if error
     */
    [[nodiscard]] EGLConfig chooseConfig();

    /**
     * @brief Initialize offscreen (PBuffer) surface for headless rendering
     * @return true if successful
     */
    bool initializeOffscreenSurface();

    /**
     * @brief Query extension list
     * @return Vector of extension names
     */
    [[nodiscard]] std::vector<std::string> queryExtensions();

    /**
     * @brief Validate OpenGL version
     * @return true if version >= 3.0
     */
    [[nodiscard]] bool validateVersion() const;
};

} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_OPENGL_CONTEXT_H
