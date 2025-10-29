cmake_minimum_required(VERSION 3.22.1)
project("clipforge" CXX)

# ============================================================================
# C++ Standard and Compiler Configuration
# ============================================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -fexceptions")

# Debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DCLIPFORGE_DEBUG_BUILD")
# Release flags - aggressive optimization
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -ffunction-sections -fdata-sections")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")

# Default to Release build if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "ClipForge Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "ClipForge C++ Standard: C++${CMAKE_CXX_STANDARD}")

# ============================================================================
# Source Files Organization
# ============================================================================

# Model/Data structure classes
set(MODEL_SOURCES
    models/effect.cpp
    models/video_clip.cpp
    models/audio_track.cpp
    models/timeline.cpp
)

# Core engine
set(CORE_SOURCES
    core/video_engine.cpp
)

# Effects processing
set(EFFECTS_SOURCES
    effects/effects_processor.cpp
    effects/filter_library.cpp
    effects/audio_processor.cpp
)

# Rendering and export
set(RENDERING_SOURCES
    rendering/video_renderer.cpp
    rendering/export_engine.cpp
)

# Media handling
set(MEDIA_SOURCES
    media/media_extractor.cpp
    media/thumbnail_generator.cpp
)

# Utilities
set(UTILS_SOURCES
    utils/logger.cpp
    utils/file_utils.cpp
)

# JNI Bridge
set(JNI_SOURCES
    jni_bridge/jni_bridge.cpp
    jni_bridge/native-lib.cpp
    # jni_bridge/gpu_bridge.cpp  # Temporarily disabled - depends on GPU sources
    jni_bridge/export_native_lib.cpp
)

# GPU Rendering (Phase 5) - Temporarily disabled due to missing GLM
# set(GPU_SOURCES
#     gpu/opengl_context.cpp
#     gpu/shader_program.cpp
#     gpu/gpu_effect.cpp
#     gpu/gpu_renderer.cpp
#     gpu/effects/color_grade_effect.cpp
#     gpu/effects/blur_effect.cpp
#     gpu/effects/distortion_effect.cpp
# )

# Audio Processing (Phase 5)
set(AUDIO_SOURCES
    audio/audio_analyzer.cpp
)

# Encoding/Export (Phase 6)
set(ENCODING_SOURCES
    encoding/video_encoder.cpp
    encoding/export_manager.cpp
)

# Combine all sources
set(ALL_SOURCES
    ${MODEL_SOURCES}
    ${CORE_SOURCES}
    ${EFFECTS_SOURCES}
    ${RENDERING_SOURCES}
    ${MEDIA_SOURCES}
    ${UTILS_SOURCES}
    ${JNI_SOURCES}
    # ${GPU_SOURCES}  # Temporarily disabled
    ${AUDIO_SOURCES}
    ${ENCODING_SOURCES}
)

# ============================================================================
# Create Main Library
# ============================================================================

add_library(${CMAKE_PROJECT_NAME} SHARED ${ALL_SOURCES})

# ============================================================================
# Include Directories
# ============================================================================

target_include_directories(${CMAKE_PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/models
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/effects
        ${CMAKE_CURRENT_SOURCE_DIR}/rendering
        ${CMAKE_CURRENT_SOURCE_DIR}/media
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/jni_bridge
        ${CMAKE_CURRENT_SOURCE_DIR}/gpu
        ${CMAKE_CURRENT_SOURCE_DIR}/audio
        ${CMAKE_CURRENT_SOURCE_DIR}/encoding
)

# ============================================================================
# Link Libraries
# ============================================================================

# Android system libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    PRIVATE
        android
        log
        GLESv3
        EGL
)

# ============================================================================
# Compiler Warnings and Settings
# ============================================================================

if(NOT MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wconversion
        -Wsign-conversion
        -Wunused-parameter
        -Wunused-variable
        -Wunreachable-code
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
            -Wdocumentation
        )
    endif()
endif()

# ============================================================================
# Version Information
# ============================================================================

set(CLIPFORGE_VERSION_MAJOR 1)
set(CLIPFORGE_VERSION_MINOR 0)
set(CLIPFORGE_VERSION_PATCH 0)

target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    CLIPFORGE_VERSION="${CLIPFORGE_VERSION_MAJOR}.${CLIPFORGE_VERSION_MINOR}.${CLIPFORGE_VERSION_PATCH}"
    CLIPFORGE_VERSION_MAJOR=${CLIPFORGE_VERSION_MAJOR}
    CLIPFORGE_VERSION_MINOR=${CLIPFORGE_VERSION_MINOR}
    CLIPFORGE_VERSION_PATCH=${CLIPFORGE_VERSION_PATCH}
)

# ============================================================================
# Build Information
# ============================================================================

message(STATUS "=== ClipForge Native Build Configuration ===")
message(STATUS "Project: ${CMAKE_PROJECT_NAME}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "GPU Sources: DISABLED (missing GLM)")
message(STATUS "Audio Sources: ${AUDIO_SOURCES}")
message(STATUS "Encoding Sources: ${ENCODING_SOURCES}")
message(STATUS "==========================================")
