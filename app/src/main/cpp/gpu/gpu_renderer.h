#ifndef CLIPFORGE_GPU_RENDERER_H
#define CLIPFORGE_GPU_RENDERER_H

#include "opengl_context.h"
#include "gpu_effect.h"
#include "shader_program.h"
#include <memory>
#include <vector>
#include <unordered_map>
// #include <glm/glm.hpp>

namespace clipforge {
namespace gpu {

/**
 * @struct RenderConfig
 * @brief Configuration for GPU rendering pipeline
 */
struct RenderConfig {
    int renderWidth = 1920;
    int renderHeight = 1080;
    int outputWidth = 1920;
    int outputHeight = 1080;
    bool useMultisampling = true;
    int sampleCount = 4;
    bool enableCache = true;
    bool enableProfiling = false;
};

/**
 * @struct RenderStats
 * @brief Statistics from GPU rendering
 */
struct RenderStats {
    float gpuTimeMs = 0.0f;
    float cpuTimeMs = 0.0f;
    float totalTimeMs = 0.0f;
    int framesPerSecond = 0;
    float gpuMemoryUsedMB = 0.0f;
    int textureCount = 0;
    int framebufferCount = 0;
};

/**
 * @class GPURenderer
 * @brief High-level GPU rendering pipeline
 *
 * Manages:
 * - OpenGL context and initialization
 * - Framebuffer chains for multi-pass rendering
 * - Effect application and composition
 * - Real-time preview rendering
 * - Performance monitoring
 *
 * Usage:
 * @code
 * auto renderer = std::make_unique<GPURenderer>();
 * RenderConfig config{1920, 1080};
 * if (renderer->initialize(config)) {
 *     // Apply effects to frame
 *     renderer->renderFrame(inputTexture, outputFramebuffer);
 * }
 * @endcode
 */
class GPURenderer {
public:
    /**
     * @brief Create GPU renderer
     */
    GPURenderer();

    /**
     * @brief Destructor
     */
    ~GPURenderer();

    // Prevent copying
    GPURenderer(const GPURenderer&) = delete;
    GPURenderer& operator=(const GPURenderer&) = delete;

    // ===== Initialization =====

    /**
     * @brief Initialize GPU renderer
     * @param config Rendering configuration
     * @return true if initialization successful
     */
    bool initialize(const RenderConfig& config);

    /**
     * @brief Check if renderer is initialized
     * @return true if ready to render
     */
    [[nodiscard]] bool isInitialized() const { return m_context && m_context->isInitialized(); }

    /**
     * @brief Shutdown renderer
     */
    void shutdown();

    /**
     * @brief Get OpenGL context
     * @return Pointer to context
     */
    [[nodiscard]] OpenGLContext* getContext() const { return m_context.get(); }

    /**
     * @brief Get rendering configuration
     * @return Current config
     */
    [[nodiscard]] const RenderConfig& getConfig() const { return m_config; }

    // ===== Effect Management =====

    /**
     * @brief Add effect to rendering pipeline
     * @param effect GPU effect to add
     * @return true if added successfully
     */
    bool addEffect(std::shared_ptr<GPUEffect> effect);

    /**
     * @brief Remove effect from pipeline
     * @param effectName Effect name
     * @return true if removed
     */
    bool removeEffect(const std::string& effectName);

    /**
     * @brief Get effect by name
     * @param effectName Effect identifier
     * @return Effect pointer, nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<GPUEffect> getEffect(const std::string& effectName) const;

    /**
     * @brief Check if effect exists
     * @param effectName Effect identifier
     * @return true if found
     */
    [[nodiscard]] bool hasEffect(const std::string& effectName) const;

    /**
     * @brief Get list of active effects
     * @return Vector of effect names
     */
    [[nodiscard]] std::vector<std::string> getEffectNames() const;

    /**
     * @brief Clear all effects
     */
    void clearEffects();

    /**
     * @brief Get effect count
     * @return Number of effects in pipeline
     */
    [[nodiscard]] size_t getEffectCount() const { return m_effects.size(); }

    /**
     * @brief Enable/disable effect
     * @param effectName Effect identifier
     * @param enabled Enable or disable
     * @return true if found and changed
     */
    bool setEffectEnabled(const std::string& effectName, bool enabled);

    // ===== Rendering =====

    /**
     * @brief Render input texture through effect pipeline
     * @param inputTexture Input texture ID
     * @param outputFramebuffer Framebuffer to render to (0 = screen)
     * @return true if rendering successful
     */
    bool renderFrame(GLuint inputTexture, GLuint outputFramebuffer = 0);

    /**
     * @brief Render frame to texture (offscreen)
     * @param inputTexture Input texture
     * @return Output texture ID, 0 if error
     */
    [[nodiscard]] GLuint renderToTexture(GLuint inputTexture);

    /**
     * @brief Apply single effect to input texture
     * @param inputTexture Input texture ID
     * @param effectName Effect to apply
     * @param outputFramebuffer Output framebuffer (0 = screen)
     * @return true if successful
     */
    bool applyEffect(GLuint inputTexture, const std::string& effectName,
                    GLuint outputFramebuffer = 0);

    // ===== Performance =====

    /**
     * @brief Get rendering statistics
     * @return Current stats
     */
    [[nodiscard]] const RenderStats& getStats() const { return m_stats; }

    /**
     * @brief Reset performance statistics
     */
    void resetStats();

    /**
     * @brief Enable performance profiling
     * @param enable Enable or disable
     */
    void setProfilingEnabled(bool enable) { m_profilingEnabled = enable; }

    /**
     * @brief Get average GPU time per frame
     * @return Time in milliseconds
     */
    [[nodiscard]] float getAverageGPUTime() const { return m_stats.gpuTimeMs; }

    /**
     * @brief Get average CPU time per frame
     * @return Time in milliseconds
     */
    [[nodiscard]] float getAverageCPUTime() const { return m_stats.cpuTimeMs; }

    /**
     * @brief Get current FPS
     * @return Frames per second
     */
    [[nodiscard]] int getCurrentFPS() const { return m_stats.framesPerSecond; }

    // ===== Framebuffer Management =====

    /**
     * @brief Create intermediate framebuffer for effect chaining
     * @return Framebuffer ID, 0 on error
     */
    [[nodiscard]] GLuint createIntermediateFramebuffer();

    /**
     * @brief Release intermediate framebuffer
     * @param fbo Framebuffer ID
     */
    void releaseIntermediateFramebuffer(GLuint fbo);

    /**
     * @brief Get current render target dimensions
     * @return {width, height}
     */
    [[nodiscard]] glm::ivec2 getRenderTargetSize() const {
        return glm::ivec2(m_config.renderWidth, m_config.renderHeight);
    }

    // ===== Debug =====

    /**
     * @brief Get debug information
     * @return Debug string
     */
    [[nodiscard]] std::string getDebugInfo() const;

    /**
     * @brief Get GPU info
     * @return GPU vendor, renderer, version strings
     */
    [[nodiscard]] std::string getGPUInfo() const;

private:
    std::unique_ptr<OpenGLContext> m_context;
    RenderConfig m_config;
    RenderStats m_stats;

    // Effect pipeline
    std::unordered_map<std::string, std::shared_ptr<GPUEffect>> m_effects;
    std::vector<std::string> m_effectOrder;  // Ordered list of effect names

    // Framebuffer chain for multi-pass rendering
    std::vector<GLuint> m_intermediateFramebuffers;
    GLuint m_currentInputTexture = 0;
    GLuint m_currentOutputFramebuffer = 0;

    // Performance tracking
    bool m_profilingEnabled = false;
    std::vector<float> m_gpuTimeHistory;
    std::vector<float> m_cpuTimeHistory;
    std::vector<int> m_fpsHistory;
    static constexpr size_t HISTORY_SIZE = 60;  // 60 frame history

    /**
     * @brief Begin profiling frame
     */
    void beginProfiling();

    /**
     * @brief End profiling frame and update stats
     */
    void endProfiling();

    /**
     * @brief Update statistics from history
     */
    void updateStatistics();

    /**
     * @brief Apply effect chain to texture
     * @param inputTexture Input texture
     * @return Final output texture
     */
    [[nodiscard]] GLuint applyEffectChain(GLuint inputTexture);
};

} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_GPU_RENDERER_H
