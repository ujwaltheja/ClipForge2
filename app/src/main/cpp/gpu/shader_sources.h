#ifndef CLIPFORGE_SHADER_SOURCES_H
#define CLIPFORGE_SHADER_SOURCES_H

#include "shader_program.h"
#include <string>

namespace clipforge {
namespace gpu {
namespace shaders {

/**
 * @namespace shaders
 * @brief Pre-built GLSL shader source code for common effects
 *
 * All shaders are OpenGL ES 3.0 compatible with:
 * - Precision: mediump for colors, highp for positions
 * - Texture format: RGBA for input/output
 * - Coordinate system: OpenGL standard (0,0 at bottom-left)
 */

// ===== BASE SHADERS =====

/**
 * @brief Basic pass-through vertex shader
 * Used by all screen-space effect shaders
 */
inline constexpr std::string_view VERTEX_PASSTHROUGH = R"glsl(
#version 300 es
precision highp float;

layout(location = 0) in vec4 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 vTexCoord;

uniform mat4 uProjection;
uniform mat4 uModelView;

void main() {
    gl_Position = uProjection * uModelView * aPosition;
    vTexCoord = aTexCoord;
}
)glsl";

/**
 * @brief Basic screen-space fragment shader (no processing)
 */
inline constexpr std::string_view FRAGMENT_PASSTHROUGH = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;

out vec4 outColor;

void main() {
    outColor = texture(uTexture, vTexCoord);
}
)glsl";

// ===== COLOR GRADING SHADERS =====

/**
 * @brief Color Lookup Table (LUT) shader
 * Applies 3D color mapping for professional color grading
 */
inline constexpr std::string_view FRAGMENT_COLOR_LUT = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform sampler2D uLUT;
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Map RGB to LUT coordinates (3D LUT as 2D texture)
    // Typically 16x16x16 cube stored as 256x16 or 16x256
    float lutSize = 16.0;
    float pixelSize = 1.0 / (lutSize * lutSize);

    // Convert color to LUT coordinates
    vec3 lut_coord = color.rgb * (lutSize - 1.0) / lutSize + 0.5 / (lutSize * lutSize);
    float offset = mod(lut_coord.b * lutSize, lutSize) / lutSize;

    vec2 lut_uv = vec2(
        lut_coord.r / lutSize + offset,
        lut_coord.g
    );

    vec4 graded = texture(uLUT, lut_uv);
    outColor = mix(color, graded, uIntensity);
}
)glsl";

/**
 * @brief Curves adjustment shader
 * Apply per-channel curve adjustments (RGB or luminance)
 */
inline constexpr std::string_view FRAGMENT_CURVES = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform sampler2D uCurveTexture;  // 256x3 texture storing RGB curves
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Sample curves for each channel
    float r = texture(uCurveTexture, vec2(color.r, 0.0 / 3.0)).r;
    float g = texture(uCurveTexture, vec2(color.g, 1.0 / 3.0)).g;
    float b = texture(uCurveTexture, vec2(color.b, 2.0 / 3.0)).b;

    vec4 curved = vec4(r, g, b, color.a);
    outColor = mix(color, curved, uIntensity);
}
)glsl";

/**
 * @brief HSL adjustment shader
 * Adjust Hue, Saturation, Lightness independently
 */
inline constexpr std::string_view FRAGMENT_HSL = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uHue;           // -180 to 180 degrees
uniform float uSaturation;    // -1 to 1
uniform float uLightness;     // -1 to 1
uniform float uIntensity;

out vec4 outColor;

// Convert RGB to HSL
vec3 rgb2hsl(vec3 rgb) {
    float maxc = max(rgb.r, max(rgb.g, rgb.b));
    float minc = min(rgb.r, min(rgb.g, rgb.b));
    float l = (maxc + minc) / 2.0;

    float h, s;
    if (maxc == minc) {
        h = s = 0.0;
    } else {
        float d = maxc - minc;
        s = l < 0.5 ? d / (maxc + minc) : d / (2.0 - maxc - minc);

        if (maxc == rgb.r) {
            h = mod((rgb.g - rgb.b) / d + (rgb.g < rgb.b ? 6.0 : 0.0), 6.0) / 6.0;
        } else if (maxc == rgb.g) {
            h = ((rgb.b - rgb.r) / d + 2.0) / 6.0;
        } else {
            h = ((rgb.r - rgb.g) / d + 4.0) / 6.0;
        }
    }

    return vec3(h, s, l);
}

// Convert HSL to RGB
vec3 hsl2rgb(vec3 hsl) {
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    float c = (1.0 - abs(2.0 * l - 1.0)) * s;
    float x = c * (1.0 - abs(mod(h * 6.0, 2.0) - 1.0));
    float m = l - c / 2.0;

    vec3 rgb;
    if (h < 1.0/6.0) rgb = vec3(c, x, 0.0);
    else if (h < 2.0/6.0) rgb = vec3(x, c, 0.0);
    else if (h < 3.0/6.0) rgb = vec3(0.0, c, x);
    else if (h < 4.0/6.0) rgb = vec3(0.0, x, c);
    else if (h < 5.0/6.0) rgb = vec3(x, 0.0, c);
    else rgb = vec3(c, 0.0, x);

    return rgb + m;
}

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    vec3 hsl = rgb2hsl(color.rgb);

    hsl.x = mod(hsl.x + (uHue / 360.0), 1.0);
    hsl.y = clamp(hsl.y + uSaturation, 0.0, 1.0);
    hsl.z = clamp(hsl.z + uLightness, 0.0, 1.0);

    vec3 adjusted = hsl2rgb(hsl);
    outColor = vec4(mix(color.rgb, adjusted, uIntensity), color.a);
}
)glsl";

// ===== BLUR EFFECTS =====

/**
 * @brief Gaussian blur shader
 * Separable 2-pass Gaussian blur (X then Y)
 */
inline constexpr std::string_view FRAGMENT_BLUR = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uTexelSize;      // 1.0 / texture_size
uniform float uRadius;         // Blur radius in pixels
uniform float uIntensity;

out vec4 outColor;

const int KERNEL_SIZE = 9;
const float KERNEL[9] = float[](
    0.077847, 0.123177, 0.077847,
    0.123177, 0.195346, 0.123177,
    0.077847, 0.123177, 0.077847
);

void main() {
    vec4 result = vec4(0.0);
    vec2 offset = uTexelSize * uRadius;

    int idx = 0;
    for (int y = -1; y <= 1; y++) {
        for (int x = -1; x <= 1; x++) {
            vec2 sampleCoord = vTexCoord + vec2(float(x), float(y)) * offset;
            result += texture(uTexture, sampleCoord) * KERNEL[idx];
            idx++;
        }
    }

    vec4 color = texture(uTexture, vTexCoord);
    outColor = mix(color, result, uIntensity);
}
)glsl";

// ===== DISTORTION EFFECTS =====

/**
 * @brief Chromatic aberration shader
 * RGB channel separation effect
 */
inline constexpr std::string_view FRAGMENT_CHROMATIC_ABERRATION = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform vec2 uOffset;          // Offset in UV space
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec2 center = vTexCoord - 0.5;

    float r = texture(uTexture, vTexCoord + uOffset * uIntensity).r;
    float g = texture(uTexture, vTexCoord).g;
    float b = texture(uTexture, vTexCoord - uOffset * uIntensity).b;

    vec4 color = texture(uTexture, vTexCoord);
    outColor = vec4(
        mix(color.r, r, uIntensity),
        mix(color.g, g, uIntensity),
        mix(color.b, b, uIntensity),
        color.a
    );
}
)glsl";

/**
 * @brief Glitch effect shader
 * Random video glitch lines
 */
inline constexpr std::string_view FRAGMENT_GLITCH = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uTime;           // Time for animation
uniform float uIntensity;

out vec4 outColor;

// Simple pseudo-random function
float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Create glitch lines
    float glitch = random(vec2(vTexCoord.y + uTime * 0.1, uTime)) * 2.0 - 1.0;
    vec2 glitchCoord = vTexCoord + vec2(glitch * uIntensity * 0.1, 0.0);

    vec4 glitchColor = texture(uTexture, glitchCoord);

    // Add glitch displacement
    outColor = mix(color, glitchColor, uIntensity * abs(glitch));
}
)glsl";

// ===== LIGHT EFFECTS =====

/**
 * @brief Vignette shader
 * Darkens edges of frame
 */
inline constexpr std::string_view FRAGMENT_VIGNETTE = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uIntensity;
uniform float uRadius;          // 0.0 to 1.0

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Distance from center
    vec2 center = vec2(0.5);
    float dist = distance(vTexCoord, center) * 1.414;

    // Vignette falloff
    float vignette = smoothstep(uRadius + uIntensity, uRadius - uIntensity, dist);

    outColor = color * vec4(vignette, vignette, vignette, 1.0);
}
)glsl";

/**
 * @brief Glow/Bloom shader
 * Brightens highlights and creates bloom effect
 */
inline constexpr std::string_view FRAGMENT_GLOW = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform sampler2D uBloomTexture;
uniform float uIntensity;
uniform float uThreshold;       // Brightness threshold

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    // Extract bright areas
    float brightness = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    if (brightness > uThreshold) {
        color.rgb += color.rgb * uIntensity;
    }

    // Optional: sample bloom texture for additional glow
    vec4 bloom = texture(uBloomTexture, vTexCoord);
    color.rgb += bloom.rgb * uIntensity;

    outColor = color;
}
)glsl";

// ===== ARTISTIC EFFECTS =====

/**
 * @brief Posterize shader
 * Reduces color palette
 */
inline constexpr std::string_view FRAGMENT_POSTERIZE = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uLevels;          // 2 to 256 color levels
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);

    vec4 posterized = floor(color * uLevels) / uLevels;
    outColor = mix(color, posterized, uIntensity);
}
)glsl";

/**
 * @brief Invert shader
 * Inverts all colors
 */
inline constexpr std::string_view FRAGMENT_INVERT = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    vec4 inverted = vec4(1.0 - color.rgb, color.a);
    outColor = mix(color, inverted, uIntensity);
}
)glsl";

/**
 * @brief Grayscale/Desaturate shader
 */
inline constexpr std::string_view FRAGMENT_GRAYSCALE = R"glsl(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;
uniform float uIntensity;

out vec4 outColor;

void main() {
    vec4 color = texture(uTexture, vTexCoord);
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    vec4 grayscale = vec4(vec3(gray), color.a);
    outColor = mix(color, grayscale, uIntensity);
}
)glsl";

} // namespace shaders
} // namespace gpu
} // namespace clipforge

#endif // CLIPFORGE_SHADER_SOURCES_H
