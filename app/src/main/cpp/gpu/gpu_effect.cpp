#include "gpu_effect.h"
#include "../utils/logger.h"

namespace clipforge {
namespace gpu {

// Static member initialization
GLuint GPUEffect::s_quadVAO = 0;
GLuint GPUEffect::s_quadVBO = 0;
bool GPUEffect::s_quadInitialized = false;

GPUEffect::GPUEffect(const std::string& name, EffectCategory category,
                    std::shared_ptr<ShaderProgram> shader)
    : m_name(name), m_category(category), m_shader(shader) {
    LOGI("GPUEffect created: %s", name.c_str());
}

std::vector<EffectParameter> GPUEffect::getParameters() const {
    std::vector<EffectParameter> params;
    for (const auto& pair : m_parameterDefinitions) {
        params.push_back(pair.second);
    }
    return params;
}

bool GPUEffect::setParameter(const std::string& paramName, float value) {
    auto it = m_parameterDefinitions.find(paramName);
    if (it != m_parameterDefinitions.end()) {
        const auto& def = it->second;
        float clampedValue = glm::clamp(value, def.minValue, def.maxValue);
        m_parameters[paramName] = clampedValue;
        return true;
    }

    LOGW("Parameter not found: %s", paramName.c_str());
    return false;
}

float GPUEffect::getParameter(const std::string& paramName) const {
    auto it = m_parameters.find(paramName);
    if (it != m_parameters.end()) {
        return it->second;
    }

    return 0.0f;
}

glm::vec3 GPUEffect::getParameterRange(const std::string& paramName) const {
    auto it = m_parameterDefinitions.find(paramName);
    if (it != m_parameterDefinitions.end()) {
        const auto& def = it->second;
        return glm::vec3(def.minValue, def.maxValue, def.defaultValue);
    }

    return glm::vec3(0.0f, 1.0f, 0.5f);
}

void GPUEffect::resetParameters() {
    for (auto& pair : m_parameterDefinitions) {
        m_parameters[pair.first] = pair.second.defaultValue;
    }

    LOGI("Effect parameters reset: %s", m_name.c_str());
}

bool GPUEffect::apply(GLuint inputTexture, GLuint outputFramebuffer, int width, int height) {
    if (!isAvailable()) {
        LOGE("Effect not available: %s", m_name.c_str());
        return false;
    }

    if (!m_enabled) {
        return true;  // Effect disabled, just return success
    }

    // Initialize quad if needed
    if (!s_quadInitialized) {
        initializeQuad();
    }

    // Make shader current
    m_shader->use();

    // Bind output framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, outputFramebuffer);
    glViewport(0, 0, width, height);

    // Bind input texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    m_shader->setUniform("uTexture", 0);

    // Bind quad VAO
    glBindVertexArray(s_quadVAO);

    // Apply custom uniforms (override in derived classes)
    applyCustomUniforms(inputTexture, width, height);

    // Set intensity uniform
    m_shader->setUniform("uIntensity", m_intensity);

    // Render full-screen quad
    renderFullscreenQuad(width, height);

    // Unbind
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void GPUEffect::applyCustomUniforms(GLuint inputTexture, int width, int height) {
    // Default implementation - no custom uniforms
    // Override in derived effect classes
    (void)inputTexture;  // Unused
    (void)width;         // Unused
    (void)height;        // Unused
}

void GPUEffect::defineParameter(const std::string& name, const std::string& uniformName,
                               float defaultVal, float minVal, float maxVal,
                               const std::string& type) {
    EffectParameter param{
        name, uniformName, defaultVal, minVal, maxVal, type
    };

    m_parameterDefinitions[name] = param;
    m_parameters[name] = defaultVal;
}

void GPUEffect::initializeQuad() {
    if (s_quadInitialized) return;

    // Full-screen quad vertices and texture coordinates
    float quadVertices[] = {
        // Position       // Texture Coords
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f
    };

    GLuint quadIndices[] = {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &s_quadVAO);
    glGenBuffers(1, &s_quadVBO);
    GLuint quadEBO;
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(s_quadVAO);

    // Vertex positions and texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, s_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                         (void*)(2 * sizeof(float)));

    glBindVertexArray(0);

    s_quadInitialized = true;
    LOGI("Quad geometry initialized for GPU effects");
}

void GPUEffect::setupQuadVertices() {
    if (!s_quadInitialized) {
        initializeQuad();
    }

    glBindVertexArray(s_quadVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
}

void GPUEffect::renderFullscreenQuad(int width, int height) {
    // Render full-screen quad
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

std::string GPUEffect::getDebugInfo() const {
    std::string info = "Effect: " + m_name + "\n";
    info += "  Available: " + std::string(isAvailable() ? "yes" : "no") + "\n";
    info += "  Enabled: " + std::string(m_enabled ? "yes" : "no") + "\n";
    info += "  Intensity: " + std::to_string(m_intensity) + "\n";
    info += "  Parameters: " + std::to_string(m_parameters.size()) + "\n";

    for (const auto& param : getParameters()) {
        info += "    - " + param.name + ": " + std::to_string(getParameter(param.name)) + "\n";
    }

    return info;
}

} // namespace gpu
} // namespace clipforge
