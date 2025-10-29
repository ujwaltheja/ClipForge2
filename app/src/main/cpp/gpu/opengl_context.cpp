#include "opengl_context.h"
#include "../utils/logger.h"
#include <sstream>
#include <algorithm>

// Define missing EGL constants for Android
#ifndef EGL_NO_CONFIG
#define EGL_NO_CONFIG ((EGLConfig)0)
#endif

#ifndef GL_MAX_UNIFORM_VECTORS
#define GL_MAX_UNIFORM_VECTORS 0x8DFD
#endif

namespace clipforge {
namespace gpu {

OpenGLContext::OpenGLContext(int displayWidth, int displayHeight)
    : m_width(displayWidth), m_height(displayHeight) {
    LOG_INFO("OpenGLContext created: %dx%d", displayWidth, displayHeight);
}

OpenGLContext::~OpenGLContext() {
    shutdown();
}

bool OpenGLContext::initialize() {
    LOG_INFO("Initializing OpenGL ES 3.0 context...");

    // Get EGL display
    m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (m_display == EGL_NO_DISPLAY) {
        LOG_ERROR("Failed to get EGL display");
        return false;
    }

    // Initialize EGL
    EGLint majorVersion, minorVersion;
    if (!eglInitialize(m_display, &majorVersion, &minorVersion)) {
        LOG_ERROR("Failed to initialize EGL: %s", getGLErrorString(eglGetError()).c_str());
        return false;
    }

    LOG_INFO("EGL initialized: %d.%d", majorVersion, minorVersion);

    // Choose EGL configuration
    EGLConfig config = chooseConfig();
    if (config == EGL_NO_CONFIG) {
        LOG_ERROR("Failed to choose EGL config");
        return false;
    }

    // Create EGL context
    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,  // OpenGL ES 3.0
        EGL_CONTEXT_PRIORITY_LEVEL_IMG, EGL_CONTEXT_PRIORITY_HIGH_IMG,
        EGL_NONE
    };

    m_context = eglCreateContext(m_display, config, EGL_NO_CONTEXT, contextAttribs);
    if (m_context == EGL_NO_CONTEXT) {
        LOG_ERROR("Failed to create EGL context: %s", getGLErrorString(eglGetError()).c_str());
        return false;
    }

    LOG_INFO("EGL context created");

    // Create surface (offscreen rendering)
    if (!initializeOffscreenSurface()) {
        LOG_ERROR("Failed to initialize offscreen surface");
        eglDestroyContext(m_display, m_context);
        m_context = EGL_NO_CONTEXT;
        return false;
    }

    // Make context current
    if (!makeCurrent()) {
        LOG_ERROR("Failed to make context current");
        return false;
    }

    // Detect capabilities
    detectCapabilities();

    // Validate version
    if (!validateVersion()) {
        LOG_ERROR("OpenGL ES version is not 3.0+");
        return false;
    }

    // Validate extensions
    if (!validateRequiredExtensions()) {
        LOG_WARNING("Some required extensions not available");
    }

    LOG_INFO("OpenGL context initialized successfully");
    LOG_INFO("GL Vendor: %s", (const char*)glGetString(GL_VENDOR));
    LOG_INFO("GL Renderer: %s", (const char*)glGetString(GL_RENDERER));
    LOG_INFO("GL Version: %s", (const char*)glGetString(GL_VERSION));
    LOG_INFO("GLSL Version: %s", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    return true;
}

EGLConfig OpenGLContext::chooseConfig() {
    EGLint configAttribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT | EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    EGLint numConfigs;
    EGLConfig config;

    if (!eglChooseConfig(m_display, configAttribs, &config, 1, &numConfigs)) {
        LOG_ERROR("eglChooseConfig failed");
        return EGL_NO_CONFIG;
    }

    if (numConfigs == 0) {
        LOG_ERROR("No matching EGL config found");
        return EGL_NO_CONFIG;
    }

    return config;
}

bool OpenGLContext::initializeOffscreenSurface() {
    EGLConfig config = chooseConfig();
    if (config == EGL_NO_CONFIG) {
        return false;
    }

    EGLint surfaceAttribs[] = {
        EGL_WIDTH, m_width,
        EGL_HEIGHT, m_height,
        EGL_NONE
    };

    m_surface = eglCreatePbufferSurface(m_display, config, surfaceAttribs);
    if (m_surface == EGL_NO_SURFACE) {
        LOG_ERROR("Failed to create PBuffer surface: %s", getGLErrorString(eglGetError()).c_str());
        return false;
    }

    LOG_INFO("PBuffer surface created: %dx%d", m_width, m_height);
    return true;
}

bool OpenGLContext::makeCurrent() {
    if (!eglMakeCurrent(m_display, m_surface, m_surface, m_context)) {
        LOG_ERROR("Failed to make EGL context current: %s", getGLErrorString(eglGetError()).c_str());
        return false;
    }
    return true;
}

bool OpenGLContext::releaseContext() {
    if (!eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        LOG_ERROR("Failed to release EGL context");
        return false;
    }
    return true;
}

void OpenGLContext::shutdown() {
    LOG_INFO("Shutting down OpenGL context...");

    if (m_context != EGL_NO_CONTEXT) {
        eglDestroyContext(m_display, m_context);
        m_context = EGL_NO_CONTEXT;
    }

    if (m_surface != EGL_NO_SURFACE) {
        eglDestroySurface(m_display, m_surface);
        m_surface = EGL_NO_SURFACE;
    }

    if (m_display != EGL_NO_DISPLAY) {
        eglTerminate(m_display);
        m_display = EGL_NO_DISPLAY;
    }

    LOG_INFO("OpenGL context shutdown complete");
}

void OpenGLContext::detectCapabilities() {
    // Get version
    const char* versionStr = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    if (versionStr) {
        sscanf(versionStr, "OpenGL ES %d.%d", &m_capabilities.majorVersion,
               &m_capabilities.minorVersion);
    }

    // Get maximum texture size
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_capabilities.maxTextureSize);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &m_capabilities.maxRenderBufferSize);
    glGetIntegerv(GL_MAX_UNIFORM_VECTORS, &m_capabilities.maxUniformVectors);
    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &m_capabilities.maxColorAttachments);

    // Query extensions
    m_capabilities.extensions = queryExtensions();

    // Check specific features
    m_capabilities.supportsFramebufferObject = hasExtension("GL_EXT_framebuffer_object");
    m_capabilities.supportsTextureFloat = hasExtension("GL_EXT_color_buffer_float");
    m_capabilities.supportsDepthTexture = hasExtension("GL_OES_depth_texture");
    m_capabilities.supportsHalfFloat = hasExtension("GL_OES_texture_half_float");

    LOG_INFO("OpenGL Capabilities:");
    LOG_INFO("  Version: %d.%d", m_capabilities.majorVersion, m_capabilities.minorVersion);
    LOG_INFO("  Max Texture Size: %d", m_capabilities.maxTextureSize);
    LOG_INFO("  Max Renderbuffer Size: %d", m_capabilities.maxRenderBufferSize);
    LOG_INFO("  Max Uniform Vectors: %d", m_capabilities.maxUniformVectors);
    LOG_INFO("  FBO Support: %d", m_capabilities.supportsFramebufferObject);
    LOG_INFO("  Float Texture Support: %d", m_capabilities.supportsTextureFloat);
}

std::vector<std::string> OpenGLContext::queryExtensions() {
    std::vector<std::string> extensions;

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    for (int i = 0; i < numExtensions; i++) {
        const char* ext = reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, i));
        if (ext) {
            extensions.push_back(ext);
        }
    }

    LOG_INFO("Found %zu OpenGL extensions", extensions.size());
    return extensions;
}

bool OpenGLContext::validateVersion() const {
    return m_capabilities.majorVersion >= 3;
}

bool OpenGLContext::validateRequiredExtensions() const {
    // Core functionality is available without extensions in ES 3.0
    return true;
}

bool OpenGLContext::hasExtension(const std::string& extension) const {
    return std::find(m_capabilities.extensions.begin(),
                     m_capabilities.extensions.end(),
                     extension) != m_capabilities.extensions.end();
}

GLuint OpenGLContext::createFramebuffer(int width, int height, GLenum colorFormat) {
    GLuint fbo, colorTexture, depthRenderBuffer;

    // Create framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create color texture
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, colorFormat, width, height, 0, colorFormat, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach color texture
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create depth renderbuffer
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderBuffer);

    // Check framebuffer status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOG_ERROR("Framebuffer incomplete: %s", getGLErrorString(status).c_str());
        glDeleteFramebuffers(1, &fbo);
        glDeleteTextures(1, &colorTexture);
        glDeleteRenderbuffers(1, &depthRenderBuffer);
        return 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG_INFO("Framebuffer created: %d (texture: %d)", fbo, colorTexture);

    return fbo;
}

void OpenGLContext::deleteFramebuffer(GLuint fbo) {
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
        LOG_INFO("Framebuffer deleted: %d", fbo);
    }
}

bool OpenGLContext::bindFramebuffer(GLuint fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLenum error = getGLError();
    if (error != GL_NO_ERROR) {
        LOG_ERROR("Error binding framebuffer %d: %s", fbo, getGLErrorString(error).c_str());
        return false;
    }
    return true;
}

GLuint OpenGLContext::getFramebufferTexture(GLuint fbo) const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLint texture;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                         GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &texture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return static_cast<GLuint>(texture);
}

GLuint OpenGLContext::createTexture(int width, int height, const uint8_t* data, GLenum format) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOG_INFO("Texture created: %d (%dx%d)", texture, width, height);
    return texture;
}

GLuint OpenGLContext::createVideoFrameTexture(int width, int height) {
    // Create texture for video frames (typically RGB or YUV)
    return createTexture(width, height, nullptr, GL_RGBA);
}

bool OpenGLContext::updateTexture(GLuint textureId, const uint8_t* data, int width, int height) {
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLenum error = getGLError();
    if (error != GL_NO_ERROR) {
        LOG_ERROR("Error updating texture: %s", getGLErrorString(error).c_str());
        return false;
    }

    return true;
}

void OpenGLContext::deleteTexture(GLuint textureId) {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
        LOG_INFO("Texture deleted: %d", textureId);
    }
}

GLenum OpenGLContext::getNextTextureUnit() {
    GLenum unit = GL_TEXTURE0 + m_currentTextureUnit;
    m_currentTextureUnit++;
    if (m_currentTextureUnit >= MAX_TEXTURE_UNITS) {
        m_currentTextureUnit = 0;
    }
    return unit;
}

void OpenGLContext::clearScreen(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLContext::setViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

bool OpenGLContext::swapBuffers() {
    // For PBuffer surface, just finish rendering
    glFinish();
    return true;
}

void OpenGLContext::waitForGPU() {
    glFinish();
}

GLenum OpenGLContext::getGLError() const {
    return glGetError();
}

std::string OpenGLContext::getGLErrorString(GLenum error) {
    switch (error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case EGL_SUCCESS: return "EGL_SUCCESS";
        case EGL_NOT_INITIALIZED: return "EGL_NOT_INITIALIZED";
        case EGL_BAD_ACCESS: return "EGL_BAD_ACCESS";
        case EGL_BAD_ALLOC: return "EGL_BAD_ALLOC";
        case EGL_BAD_ATTRIBUTE: return "EGL_BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG: return "EGL_BAD_CONFIG";
        case EGL_BAD_CONTEXT: return "EGL_BAD_CONTEXT";
        case EGL_BAD_CURRENT_SURFACE: return "EGL_BAD_CURRENT_SURFACE";
        case EGL_BAD_DISPLAY: return "EGL_BAD_DISPLAY";
        case EGL_BAD_MATCH: return "EGL_BAD_MATCH";
        case EGL_BAD_NATIVE_PIXMAP: return "EGL_BAD_NATIVE_PIXMAP";
        case EGL_BAD_NATIVE_WINDOW: return "EGL_BAD_NATIVE_WINDOW";
        case EGL_BAD_PARAMETER: return "EGL_BAD_PARAMETER";
        case EGL_BAD_SURFACE: return "EGL_BAD_SURFACE";
        default: return "UNKNOWN_ERROR";
    }
}

} // namespace gpu
} // namespace clipforge
