#ifndef CLIPFORGE_GPU_EFFECT_H
#define CLIPFORGE_GPU_EFFECT_H

#include "shader_program.h"
#include <string>
#include <memory>
#include <unordered_map>
// #include <glm/glm.hpp>

namespace clipforge {
namespace gpu {

/**
 * @enum EffectCategory
 * @brief Category of GPU effect
 */
enum class EffectCategory {
    COLOR,           // Color grading and adjustment
    DISTORTION,      // Geometric distortion
    BLUR,            // Blur effects
    LIGHT,           // Light and glow effects
    ARTISTIC,        // Artistic/stylization effects
    TEMPORAL,        // Time-based effects
    COMPOSITE,       // Compositing effects
};

/**
 * @struct EffectParameter
 * @brief Parameter for GPU effect
 */
struct EffectParameter {
    std::string name;
    std::string uniformName;
    float defaultValue = 0.5f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    std::string type;  // "float", "vec2", "vec3", "vec4"
};

/**
 * @class GPUEffect
 * @brief Base class for GPU-accelerated visual effects
 *
 * Encapsulates a shader-based effect with parameter management,
 * texture handling, and rendering pipeline integration.
 *
 * Usage:
 * @code
 * auto effect = std::make_shared<ColorGradeEffect>();
 * effect->setParameter("intensity", 0.8f);
 * effect->apply(inputTexture, outputFramebuffer);
 * @endcode
 */
class GPUEffect {
public:
    /**
     * @brief Create GPU effect
     * @param name Effect identifier
     * @param category Effect category
     * @param shader Compiled shader program
     */
    GPUEffect(const std::string& name, EffectCategory category,
             std::shared_ptr<ShaderProgram> shader);

    /**
     * @brief Virtual destructor
     */
    virtual ~GPUEffect() = default;

    // ===== Properties =====

    /**
     * @brief Get effect name
     * @return Effect identifier
     */
    [[nodiscard]] const std::string& getName() const { return m_name; }

    /**
     * @brief Get effect category
     * @return Category enum
     */
    [[nodiscard]] EffectCategory getCategory() const { return m_category; }

    /**
     * @brief Get shader program
     * @return Shared pointer to shader
     */
    [[nodiscard]] std::shared_ptr<ShaderProgram> getShader() const { return m_shader; }

    /**
     * @brief Check if effect is available (shader compiled)
     * @return true if ready to use
     */
    [[nodiscard]] bool isAvailable() const { return m_shader && m_shader->isValid(); }

    // ===== Parameters =====

    /**
     * @brief Get list of available parameters
     * @return Vector of parameter definitions
     */
    [[nodiscard]] virtual std::vector<EffectParameter> getParameters() const;

    /**
     * @brief Set effect parameter value
     * @param paramName Parameter name
     * @param value Value to set
     * @return true if parameter set successfully
     */
    virtual bool setParameter(const std::string& paramName, float value);

    /**
     * @brief Get parameter value
     * @param paramName Parameter name
     * @return Current value, 0.0 if not found
     */
    [[nodiscard]] virtual float getParameter(const std::string& paramName) const;

    /**
     * @brief Get parameter range
     * @param paramName Parameter name
     * @return {min, max, default}
     */
    [[nodiscard]] virtual glm::vec3 getParameterRange(const std::string& paramName) const;

    /**
     * @brief Reset parameters to defaults
     */
    virtual void resetParameters();

    // ===== Rendering =====

    /**
     * @brief Apply effect to input texture
     * @param inputTexture Input texture ID
     * @param outputFramebuffer Framebuffer to render to
     * @param width Render width
     * @param height Render height
     * @return true if rendering successful
     */
    virtual bool apply(GLuint inputTexture, GLuint outputFramebuffer, int width, int height);

    /**
     * @brief Apply effect with custom shader uniforms
     * Can be overridden by derived classes for custom behavior
     *
     * @param inputTexture Input texture
     * @param width Render width
     * @param height Render height
     */
    virtual void applyCustomUniforms(GLuint inputTexture, int width, int height);

    /**
     * @brief Get effect intensity (normalized 0-1)
     * @return Intensity value
     */
    [[nodiscard]] float getIntensity() const { return m_intensity; }

    /**
     * @brief Set effect intensity (normalized 0-1)
     * @param intensity Intensity value
     */
    void setIntensity(float intensity) { m_intensity = glm::clamp(intensity, 0.0f, 1.0f); }

    /**
     * @brief Get effect enabled state
     * @return true if effect should be applied
     */
    [[nodiscard]] bool isEnabled() const { return m_enabled; }

    /**
     * @brief Set effect enabled state
     * @param enabled Enable or disable effect
     */
    void setEnabled(bool enabled) { m_enabled = enabled; }

    // ===== Debugging =====

    /**
     * @brief Get effect info for debugging
     * @return Description string
     */
    [[nodiscard]] virtual std::string getDebugInfo() const;

protected:
    std::string m_name;
    EffectCategory m_category;
    std::shared_ptr<ShaderProgram> m_shader;
    std::unordered_map<std::string, float> m_parameters;
    std::unordered_map<std::string, EffectParameter> m_parameterDefinitions;
    float m_intensity = 1.0f;
    bool m_enabled = true;

    /**
     * @brief Set up vertex and texture data for rendering
     * Typically called before renderFullscreenQuad
     */
    void setupQuadVertices();

    /**
     * @brief Render full-screen quad (internal use)
     * @param width Screen width
     * @param height Screen height
     */
    void renderFullscreenQuad(int width, int height);

    /**
     * @brief Create parameter definition
     * @param name Parameter name
     * @param uniformName Shader uniform name
     * @param defaultVal Default value
     * @param minVal Minimum value
     * @param maxVal Maximum value
     * @param type Parameter type
     */
    void defineParameter(const std::string& name, const std::string& uniformName,
                        float defaultVal, float minVal, float maxVal,
                        const std::string& type = "float");

private:
    static GLuint s_quadVAO;
    static GLuint s_quadVBO;
    static bool s_quadInitialized;

    /**
     * @brief Initialize quad geometry (call once)
     */
    static void initializeQuad();
};

} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_GPU_EFFECT_H
