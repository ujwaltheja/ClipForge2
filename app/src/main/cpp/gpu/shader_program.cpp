#include "shader_program.h"
#include "../utils/logger.h"
#include "../utils/file_utils.h"
#include <sstream>

namespace clipforge {
namespace gpu {

// ===== ShaderProgram Implementation =====

ShaderProgram::ShaderProgram() {
    LOGD("ShaderProgram created");
}

ShaderProgram::~ShaderProgram() {
    if (m_programId != 0) {
        glDeleteProgram(m_programId);
        LOGD("ShaderProgram deleted: %u", m_programId);
    }
}

bool ShaderProgram::compile(const ShaderSource& source) {
    return compileFromSources(source.vertexSource, source.fragmentSource,
                             source.geometrySource);
}

bool ShaderProgram::compileFromSources(const std::string& vertexSrc,
                                       const std::string& fragmentSrc,
                                       const std::string& geometrySrc) {
    // Compile vertex shader
    GLuint vertexId = compileShader(GL_VERTEX_SHADER, vertexSrc);
    if (vertexId == 0) {
        m_lastError = "Failed to compile vertex shader";
        LOGE("%s", m_lastError.c_str());
        return false;
    }

    // Compile fragment shader
    GLuint fragmentId = compileShader(GL_FRAGMENT_SHADER, fragmentSrc);
    if (fragmentId == 0) {
        m_lastError = "Failed to compile fragment shader";
        glDeleteShader(vertexId);
        LOGE("%s", m_lastError.c_str());
        return false;
    }

    // Compile geometry shader if provided
    GLuint geometryId = 0;
    if (!geometrySrc.empty()) {
        geometryId = compileShader(GL_GEOMETRY_SHADER, geometrySrc);
        if (geometryId == 0) {
            m_lastError = "Failed to compile geometry shader";
            glDeleteShader(vertexId);
            glDeleteShader(fragmentId);
            LOGE("%s", m_lastError.c_str());
            return false;
        }
    }

    // Link program
    if (!linkProgram(vertexId, fragmentId, geometryId)) {
        m_lastError = "Failed to link shader program";
        glDeleteShader(vertexId);
        glDeleteShader(fragmentId);
        if (geometryId != 0) glDeleteShader(geometryId);
        LOGE("%s", m_lastError.c_str());
        return false;
    }

    // Clean up individual shaders
    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);
    if (geometryId != 0) glDeleteShader(geometryId);

    LOGI("Shader program compiled successfully: %u", m_programId);
    return true;
}

bool ShaderProgram::loadFromFiles(const std::string& vertexPath,
                                  const std::string& fragmentPath,
                                  const std::string& geometryPath) {
    std::string vertexSrc = utils::FileUtils::readFile(vertexPath);
    std::string fragmentSrc = utils::FileUtils::readFile(fragmentPath);
    std::string geometrySrc;

    if (!vertexSrc.empty() && !fragmentSrc.empty()) {
        if (!geometryPath.empty()) {
            geometrySrc = utils::FileUtils::readFile(geometryPath);
        }
        return compileFromSources(vertexSrc, fragmentSrc, geometrySrc);
    }

    m_lastError = "Failed to load shader files";
    LOGE("%s: vertex=%s, fragment=%s", m_lastError.c_str(),
         vertexPath.c_str(), fragmentPath.c_str());
    return false;
}

GLuint ShaderProgram::compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);

    const char* sourcePtr = source.c_str();
    glShaderSource(shader, 1, &sourcePtr, nullptr);
    glCompileShader(shader);

    // Check compilation status
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::vector<char> logBuffer(logLength);
            glGetShaderInfoLog(shader, logLength, nullptr, logBuffer.data());
            std::string log(logBuffer.begin(), logBuffer.end());

            const char* shaderTypeName;
            switch (type) {
                case GL_VERTEX_SHADER: shaderTypeName = "Vertex"; break;
                case GL_FRAGMENT_SHADER: shaderTypeName = "Fragment"; break;
                case GL_GEOMETRY_SHADER: shaderTypeName = "Geometry"; break;
                default: shaderTypeName = "Unknown"; break;
            }

            LOGE("%s shader compilation error:\n%s", shaderTypeName, log.c_str());
            m_lastError = log;
        }

        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

bool ShaderProgram::linkProgram(GLuint vertexId, GLuint fragmentId, GLuint geometryId) {
    m_programId = glCreateProgram();

    glAttachShader(m_programId, vertexId);
    glAttachShader(m_programId, fragmentId);
    if (geometryId != 0) {
        glAttachShader(m_programId, geometryId);
    }

    glLinkProgram(m_programId);

    // Check link status
    GLint linked;
    glGetProgramiv(m_programId, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint logLength = 0;
        glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);

        if (logLength > 0) {
            std::vector<char> logBuffer(logLength);
            glGetProgramInfoLog(m_programId, logLength, nullptr, logBuffer.data());
            std::string log(logBuffer.begin(), logBuffer.end());

            LOGE("Program linking error:\n%s", log.c_str());
            m_lastError = log;
        }

        glDeleteProgram(m_programId);
        m_programId = 0;
        return false;
    }

    return true;
}

void ShaderProgram::use() const {
    if (m_programId != 0) {
        glUseProgram(m_programId);
    }
}

void ShaderProgram::setUniform(const std::string& name, int value) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniform1i(location, value);
    }
}

void ShaderProgram::setUniform(const std::string& name, float value) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec2& value) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniform2f(location, value.x, value.y);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec3& value) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniform3f(location, value.x, value.y, value.z);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::vec4& value) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat3& mat) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void ShaderProgram::setUniform(const std::string& name, const glm::mat4& mat) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void ShaderProgram::setUniformArray(const std::string& name, const int* values, size_t count) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1 && count > 0) {
        glUniform1iv(location, static_cast<GLsizei>(count), values);
    }
}

void ShaderProgram::setUniformArray(const std::string& name, const float* values, size_t count) const {
    GLint location = getCachedUniformLocation(name);
    if (location != -1 && count > 0) {
        glUniform1fv(location, static_cast<GLsizei>(count), values);
    }
}

GLint ShaderProgram::getUniformLocation(const std::string& name) const {
    return glGetUniformLocation(m_programId, name.c_str());
}

GLint ShaderProgram::getAttributeLocation(const std::string& name) const {
    return glGetAttribLocation(m_programId, name.c_str());
}

GLint ShaderProgram::getCachedUniformLocation(const std::string& name) const {
    auto it = m_uniformLocations.find(name);
    if (it != m_uniformLocations.end()) {
        return it->second;
    }

    GLint location = getUniformLocation(name);
    cacheUniformLocation(name, location);

    if (location == -1) {
        LOGW("Uniform not found: %s", name.c_str());
    }

    return location;
}

void ShaderProgram::cacheUniformLocation(const std::string& name, GLint location) {
    const_cast<std::unordered_map<std::string, GLint>&>(m_uniformLocations)[name] = location;
}

void ShaderProgram::bindAttribute(GLuint index, const std::string& name) const {
    glBindAttribLocation(m_programId, index, name.c_str());
}

void ShaderProgram::enableAttribute(GLuint index) const {
    glEnableVertexAttribArray(index);
}

void ShaderProgram::disableAttribute(GLuint index) const {
    glDisableVertexAttribArray(index);
}

void ShaderProgram::vertexAttribPointer(GLuint index, GLint size, GLenum type,
                                       GLboolean normalized, GLsizei stride,
                                       const void* offset) const {
    glVertexAttribPointer(index, size, type, normalized, stride, offset);
}

std::string ShaderProgram::getInfoLog() const {
    GLint logLength = 0;
    glGetProgramiv(m_programId, GL_INFO_LOG_LENGTH, &logLength);

    if (logLength > 0) {
        std::vector<char> logBuffer(logLength);
        glGetProgramInfoLog(m_programId, logLength, nullptr, logBuffer.data());
        return std::string(logBuffer.begin(), logBuffer.end());
    }

    return "";
}

bool ShaderProgram::validate() const {
    if (m_programId == 0) {
        return false;
    }

    glValidateProgram(m_programId);

    GLint isValid;
    glGetProgramiv(m_programId, GL_VALIDATE_STATUS, &isValid);

    if (!isValid) {
        std::string log = getInfoLog();
        LOGW("Program validation failed:\n%s", log.c_str());
    }

    return isValid == GL_TRUE;
}

// ===== ShaderLibrary Implementation =====

ShaderLibrary& ShaderLibrary::getInstance() {
    static ShaderLibrary instance;
    return instance;
}

std::shared_ptr<ShaderProgram> ShaderLibrary::createShader(const std::string& name,
                                                          const ShaderSource& source) {
    auto shader = std::make_shared<ShaderProgram>();

    if (!shader->compile(source)) {
        LOGE("Failed to compile shader: %s", name.c_str());
        return nullptr;
    }

    m_shaders[name] = shader;
    LOGI("Shader registered: %s", name.c_str());

    return shader;
}

std::shared_ptr<ShaderProgram> ShaderLibrary::getShader(const std::string& name) const {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }

    LOGW("Shader not found: %s", name.c_str());
    return nullptr;
}

bool ShaderLibrary::hasShader(const std::string& name) const {
    return m_shaders.find(name) != m_shaders.end();
}

bool ShaderLibrary::removeShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        m_shaders.erase(it);
        LOGI("Shader removed: %s", name.c_str());
        return true;
    }

    return false;
}

std::vector<std::string> ShaderLibrary::getShaderNames() const {
    std::vector<std::string> names;
    for (const auto& pair : m_shaders) {
        names.push_back(pair.first);
    }
    return names;
}

void ShaderLibrary::clear() {
    m_shaders.clear();
    LOGI("Shader library cleared");
}

size_t ShaderLibrary::getShaderCount() const {
    return m_shaders.size();
}

} // namespace gpu
} // namespace clipforge
