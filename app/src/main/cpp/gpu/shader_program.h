#ifndef CLIPFORGE_SHADER_PROGRAM_H
#define CLIPFORGE_SHADER_PROGRAM_H

#include <GLES3/gl3.h>
// #include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace clipforge {
namespace gpu {

/**
 * @enum ShaderType
 * @brief Type of shader in compilation/linking
 */
enum class ShaderType {
    VERTEX = GL_VERTEX_SHADER,
    FRAGMENT = GL_FRAGMENT_SHADER,
    GEOMETRY = GL_GEOMETRY_SHADER,
};

/**
 * @struct ShaderSource
 * @brief Container for vertex and fragment shader source code
 */
struct ShaderSource {
    std::string vertexSource;
    std::string fragmentSource;
    std::string geometrySource;  // Optional
    std::string name;
};

/**
 * @class ShaderProgram
 * @brief Manages shader compilation, linking, and uniform setting
 *
 * Provides a convenient wrapper around OpenGL shader programs with:
 * - Automatic compilation and error handling
 * - Uniform management (matrices, vectors, floats, textures)
 * - Vertex attribute binding
 * - Debug information on compilation/link failures
 *
 * Usage:
 * @code
 * ShaderSource source{vertexCode, fragmentCode, "", "MyShader"};
 * auto shader = std::make_shared<ShaderProgram>();
 * if (shader->compile(source)) {
 *     shader->use();
 *     shader->setUniform("uColor", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
 *     // Render with shader
 * }
 * @endcode
 */
class ShaderProgram {
public:
    /**
     * @brief Create empty shader program
     */
    ShaderProgram();

    /**
     * @brief Destructor - delete shader program
     */
    ~ShaderProgram();

    // Prevent copying
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    // ===== Compilation =====

    /**
     * @brief Compile and link shader program
     * @param source Shader source code
     * @return true if compilation successful
     */
    bool compile(const ShaderSource& source);

    /**
     * @brief Compile from individual shader sources
     * @param vertexSrc Vertex shader code
     * @param fragmentSrc Fragment shader code
     * @param geometrySrc Optional geometry shader code
     * @return true if successful
     */
    bool compileFromSources(const std::string& vertexSrc, const std::string& fragmentSrc,
                           const std::string& geometrySrc = "");

    /**
     * @brief Load and compile shader from file paths
     * @param vertexPath Path to vertex shader
     * @param fragmentPath Path to fragment shader
     * @param geometryPath Optional path to geometry shader
     * @return true if successful
     */
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath,
                      const std::string& geometryPath = "");

    /**
     * @brief Use this shader program
     */
    void use() const;

    /**
     * @brief Check if shader compiled successfully
     * @return true if valid program
     */
    [[nodiscard]] bool isValid() const { return m_programId != 0; }

    /**
     * @brief Get shader program ID
     * @return OpenGL program ID
     */
    [[nodiscard]] GLuint getId() const { return m_programId; }

    /**
     * @brief Get last compilation/linking error
     * @return Error message
     */
    [[nodiscard]] std::string getLastError() const { return m_lastError; }

    // ===== Uniforms =====

    /**
     * @brief Set uniform int
     * @param name Uniform name
     * @param value Value to set
     */
    void setUniform(const std::string& name, int value) const;

    /**
     * @brief Set uniform float
     * @param name Uniform name
     * @param value Value to set
     */
    void setUniform(const std::string& name, float value) const;

    /**
     * @brief Set uniform vec2
     * @param name Uniform name
     * @param value Vector value
     */
    void setUniform(const std::string& name, const glm::vec2& value) const;

    /**
     * @brief Set uniform vec3
     * @param name Uniform name
     * @param value Vector value
     */
    void setUniform(const std::string& name, const glm::vec3& value) const;

    /**
     * @brief Set uniform vec4
     * @param name Uniform name
     * @param value Vector value
     */
    void setUniform(const std::string& name, const glm::vec4& value) const;

    /**
     * @brief Set uniform mat3
     * @param name Uniform name
     * @param mat Matrix value
     */
    void setUniform(const std::string& name, const glm::mat3& mat) const;

    /**
     * @brief Set uniform mat4
     * @param name Uniform name
     * @param mat Matrix value
     */
    void setUniform(const std::string& name, const glm::mat4& mat) const;

    /**
     * @brief Set uniform texture sampler
     * @param name Uniform name
     * @param unit Texture unit (0-15)
     */
    void setUniform(const std::string& name, int unit) const;

    /**
     * @brief Set uniform int array
     * @param name Uniform name
     * @param values Array of values
     * @param count Number of elements
     */
    void setUniformArray(const std::string& name, const int* values, size_t count) const;

    /**
     * @brief Set uniform float array
     * @param name Uniform name
     * @param values Array of values
     * @param count Number of elements
     */
    void setUniformArray(const std::string& name, const float* values, size_t count) const;

    /**
     * @brief Get uniform location
     * @param name Uniform name
     * @return Location ID, -1 if not found
     */
    [[nodiscard]] GLint getUniformLocation(const std::string& name) const;

    /**
     * @brief Get attribute location
     * @param name Attribute name
     * @return Location ID, -1 if not found
     */
    [[nodiscard]] GLint getAttributeLocation(const std::string& name) const;

    // ===== Attributes =====

    /**
     * @brief Bind vertex attribute
     * @param index Attribute index
     * @param name Attribute name
     */
    void bindAttribute(GLuint index, const std::string& name) const;

    /**
     * @brief Enable vertex attribute array
     * @param index Attribute index
     */
    void enableAttribute(GLuint index) const;

    /**
     * @brief Disable vertex attribute array
     * @param index Attribute index
     */
    void disableAttribute(GLuint index) const;

    /**
     * @brief Set vertex attribute pointer
     * @param index Attribute index
     * @param size Component count (2, 3, or 4)
     * @param type Data type (GL_FLOAT, GL_INT, etc.)
     * @param normalized Normalize values
     * @param stride Bytes between attributes
     * @param offset Offset into buffer
     */
    void vertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized,
                            GLsizei stride, const void* offset) const;

    // ===== Debug =====

    /**
     * @brief Get shader info log
     * @return Compilation/linking information
     */
    [[nodiscard]] std::string getInfoLog() const;

    /**
     * @brief Validate shader program
     * @return true if valid
     */
    [[nodiscard]] bool validate() const;

private:
    GLuint m_programId = 0;
    std::string m_lastError;
    mutable std::unordered_map<std::string, GLint> m_uniformLocations;
    mutable std::unordered_map<std::string, GLint> m_attributeLocations;

    /**
     * @brief Compile single shader
     * @param type Shader type
     * @param source Shader source code
     * @return Compiled shader ID, 0 on error
     */
    [[nodiscard]] GLuint compileShader(GLenum type, const std::string& source);

    /**
     * @brief Link compiled shaders into program
     * @param vertexId Vertex shader ID
     * @param fragmentId Fragment shader ID
     * @param geometryId Optional geometry shader ID (0 if unused)
     * @return true if linking successful
     */
    bool linkProgram(GLuint vertexId, GLuint fragmentId, GLuint geometryId = 0);

    /**
     * @brief Get cached uniform location
     * @param name Uniform name
     * @return Location, -1 if not found
     */
    [[nodiscard]] GLint getCachedUniformLocation(const std::string& name) const;

    /**
     * @brief Cache uniform location
     * @param name Uniform name
     * @param location Location ID
     */
    void cacheUniformLocation(const std::string& name, GLint location);
};

/**
 * @class ShaderLibrary
 * @brief Central registry of compiled shader programs
 *
 * Manages creation, caching, and retrieval of shader programs to avoid
 * redundant compilation and facilitate reuse across the application.
 */
class ShaderLibrary {
public:
    /**
     * @brief Get singleton instance
     * @return Reference to ShaderLibrary
     */
    static ShaderLibrary& getInstance();

    // Prevent copying
    ShaderLibrary(const ShaderLibrary&) = delete;
    ShaderLibrary& operator=(const ShaderLibrary&) = delete;

    /**
     * @brief Create and register shader program
     * @param name Unique shader identifier
     * @param source Shader source
     * @return Shared pointer to shader, nullptr on error
     */
    std::shared_ptr<ShaderProgram> createShader(const std::string& name,
                                               const ShaderSource& source);

    /**
     * @brief Get existing shader by name
     * @param name Shader identifier
     * @return Shared pointer to shader, nullptr if not found
     */
    [[nodiscard]] std::shared_ptr<ShaderProgram> getShader(const std::string& name) const;

    /**
     * @brief Check if shader is registered
     * @param name Shader identifier
     * @return true if exists
     */
    [[nodiscard]] bool hasShader(const std::string& name) const;

    /**
     * @brief Remove shader from library
     * @param name Shader identifier
     * @return true if removed, false if not found
     */
    bool removeShader(const std::string& name);

    /**
     * @brief Get list of registered shader names
     * @return Vector of shader names
     */
    [[nodiscard]] std::vector<std::string> getShaderNames() const;

    /**
     * @brief Clear all shaders
     */
    void clear();

    /**
     * @brief Get shader count
     * @return Number of registered shaders
     */
    [[nodiscard]] size_t getShaderCount() const;

private:
    ShaderLibrary() = default;
    ~ShaderLibrary() = default;

    std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> m_shaders;
};

} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_SHADER_PROGRAM_H
