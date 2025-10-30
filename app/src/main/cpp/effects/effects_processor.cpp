#include "effects_processor.h"
#include <cmath>
#include <algorithm>
#include <cstring>

namespace clipforge {
namespace effects {

EffectsProcessor::EffectsProcessor() = default;

bool EffectsProcessor::applyEffect(uint8_t* frameData, int width, int height,
                                   EffectType effectType, const EffectParameters& params) {
    if (!frameData || width <= 0 || height <= 0) {
        return false;
    }

    switch (effectType) {
        case EffectType::COLOR_GRADE: {
            const auto* colorParams = dynamic_cast<const ColorGradeParams*>(&params);
            if (colorParams) {
                applyColorGrade(frameData, width, height, *colorParams);
            }
            return true;
        }

        case EffectType::BLUR: {
            const auto* blurParams = dynamic_cast<const BlurParams*>(&params);
            if (blurParams) {
                applyBlur(frameData, width, height, *blurParams);
            }
            return true;
        }

        case EffectType::VIGNETTE: {
            const auto* vignetteParams = dynamic_cast<const VignetteParams*>(&params);
            if (vignetteParams) {
                applyVignette(frameData, width, height, *vignetteParams);
            }
            return true;
        }

        case EffectType::GRAYSCALE: {
            applyGrayscale(frameData, width, height, 1.0f);
            return true;
        }

        case EffectType::FILM_GRAIN: {
            applyFilmGrain(frameData, width, height, 0.5f);
            return true;
        }

        default:
            return false;
    }
}

void EffectsProcessor::applyColorGrade(uint8_t* frameData, int width, int height,
                                       const ColorGradeParams& params) {
    const int pixelCount = width * height;

    for (int i = 0; i < pixelCount; i++) {
        int pixelIndex = i * 4;
        float r = frameData[pixelIndex] / 255.0f;
        float g = frameData[pixelIndex + 1] / 255.0f;
        float b = frameData[pixelIndex + 2] / 255.0f;
        float a = frameData[pixelIndex + 3] / 255.0f;

        r *= std::pow(2.0f, params.exposure);
        g *= std::pow(2.0f, params.exposure);
        b *= std::pow(2.0f, params.exposure);

        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        frameData[pixelIndex] = static_cast<uint8_t>(r * 255.0f);
        frameData[pixelIndex + 1] = static_cast<uint8_t>(g * 255.0f);
        frameData[pixelIndex + 2] = static_cast<uint8_t>(b * 255.0f);
        frameData[pixelIndex + 3] = static_cast<uint8_t>(a * 255.0f);
    }
}

void EffectsProcessor::applyHSLAdjustment(uint8_t* frameData, int width, int height,
                                          float hueShift, float saturation, float lightness) {
    const int pixelCount = width * height;

    for (int i = 0; i < pixelCount; i++) {
        int pixelIndex = i * 4;
        float r = frameData[pixelIndex] / 255.0f;
        float g = frameData[pixelIndex + 1] / 255.0f;
        float b = frameData[pixelIndex + 2] / 255.0f;

        // Apply saturation
        float gray = 0.299f * r + 0.587f * g + 0.114f * b;
        r = gray + (r - gray) * saturation;
        g = gray + (g - gray) * saturation;
        b = gray + (b - gray) * saturation;

        r = std::clamp(r, 0.0f, 1.0f);
        g = std::clamp(g, 0.0f, 1.0f);
        b = std::clamp(b, 0.0f, 1.0f);

        frameData[pixelIndex] = static_cast<uint8_t>(r * 255.0f);
        frameData[pixelIndex + 1] = static_cast<uint8_t>(g * 255.0f);
        frameData[pixelIndex + 2] = static_cast<uint8_t>(b * 255.0f);
    }
}

void EffectsProcessor::applyVignette(uint8_t* frameData, int width, int height,
                                     const VignetteParams& params) {
    const float centerX = width / 2.0f;
    const float centerY = height / 2.0f;
    const float maxDist = std::sqrt(centerX * centerX + centerY * centerY);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = x - centerX;
            float dy = y - centerY;
            float dist = std::sqrt(dx * dx + dy * dy);
            float normalized = dist / maxDist;

            float vignette = 1.0f - (normalized * params.intensity * params.scale);
            vignette = std::clamp(vignette, 1.0f - params.opacity, 1.0f);

            int pixelIndex = (y * width + x) * 4;
            frameData[pixelIndex] = static_cast<uint8_t>(frameData[pixelIndex] * vignette);
            frameData[pixelIndex + 1] = static_cast<uint8_t>(frameData[pixelIndex + 1] * vignette);
            frameData[pixelIndex + 2] = static_cast<uint8_t>(frameData[pixelIndex + 2] * vignette);
        }
    }
}

void EffectsProcessor::applyGlow(uint8_t* frameData, int width, int height,
                                 float intensity, float threshold) {
    // Implement glow by enhancing bright areas
    const int pixelCount = width * height;
    for (int i = 0; i < pixelCount; i++) {
        int idx = i * 4;
        float brightness = (frameData[idx] + frameData[idx + 1] + frameData[idx + 2]) / 3.0f / 255.0f;

        if (brightness > threshold) {
            for (int c = 0; c < 3; c++) {
                float enhanced = frameData[idx + c] * (1.0f + intensity);
                frameData[idx + c] = static_cast<uint8_t>(std::min(enhanced, 255.0f));
            }
        }
    }
}

void EffectsProcessor::applyGrayscale(uint8_t* frameData, int width, int height,
                                      float intensity) {
    const int pixelCount = width * height;

    for (int i = 0; i < pixelCount; i++) {
        int idx = i * 4;
        uint8_t r = frameData[idx];
        uint8_t g = frameData[idx + 1];
        uint8_t b = frameData[idx + 2];

        uint8_t gray = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b);

        frameData[idx] = static_cast<uint8_t>(r * (1.0f - intensity) + gray * intensity);
        frameData[idx + 1] = static_cast<uint8_t>(g * (1.0f - intensity) + gray * intensity);
        frameData[idx + 2] = static_cast<uint8_t>(b * (1.0f - intensity) + gray * intensity);
    }
}

void EffectsProcessor::applyFilmGrain(uint8_t* frameData, int width, int height,
                                      float intensity) {
    const int pixelCount = width * height;

    for (int i = 0; i < pixelCount; i++) {
        int idx = i * 4;
        int noise = (rand() % 256) - 128;

        for (int c = 0; c < 3; c++) {
            int value = frameData[idx + c] + static_cast<int>(noise * intensity * 0.1f);
            frameData[idx + c] = static_cast<uint8_t>(std::clamp(value, 0, 255));
        }
    }
}

void EffectsProcessor::applyBlur(uint8_t* frameData, int width, int height,
                                 const BlurParams& params) {
    if (params.type == BlurParams::BlurType::GAUSSIAN) {
        // Create temporary buffer for blur computation
        auto* tempBuffer = new uint8_t[width * height * 4];
        std::memcpy(tempBuffer, frameData, width * height * 4);
        gaussianBlur(frameData, width, height, tempBuffer, params.radius);
        delete[] tempBuffer;
    }
    // Other blur types (motion, radial, zoom) would require more complex implementations
}

void EffectsProcessor::applyCurves(uint8_t* frameData, int width, int height,
                                   const std::vector<uint8_t>& curve) {
    if (curve.size() != 256) return;

    const int pixelCount = width * height;
    for (int i = 0; i < pixelCount; i++) {
        int idx = i * 4;
        frameData[idx] = curve[frameData[idx]];
        frameData[idx + 1] = curve[frameData[idx + 1]];
        frameData[idx + 2] = curve[frameData[idx + 2]];
    }
}

void EffectsProcessor::applyChromaticAberration(uint8_t* frameData, int width, int height,
                                                const ChromaticAberrationParams& params) {
    const float amount = params.amount * 0.1f;
    const float angleRad = params.angle * 3.14159f / 180.0f;
    const float offsetX = amount * std::cos(angleRad);
    const float offsetY = amount * std::sin(angleRad);

    auto* tempR = new uint8_t[width * height];
    auto* tempG = new uint8_t[width * height];
    auto* tempB = new uint8_t[width * height];

    // Extract channels
    for (int i = 0; i < width * height; i++) {
        tempR[i] = frameData[i * 4];
        tempG[i] = frameData[i * 4 + 1];
        tempB[i] = frameData[i * 4 + 2];
    }

    // Simple shift (more advanced version would use proper interpolation)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int idx = (y * width + x) * 4;
            int shiftX = static_cast<int>(offsetX);
            int shiftY = static_cast<int>(offsetY);

            int srcX = std::clamp(x - shiftX, 0, width - 1);
            int srcY = std::clamp(y - shiftY, 0, height - 1);
            int srcIdx = (srcY * width + srcX);

            frameData[idx] = tempR[srcIdx];
            frameData[idx + 1] = tempG[srcIdx];
            frameData[idx + 2] = tempB[srcIdx];
        }
    }

    delete[] tempR;
    delete[] tempG;
    delete[] tempB;
}

void EffectsProcessor::applyGlitch(uint8_t* frameData, int width, int height,
                                   const GlitchParams& params) {
    const int pixelCount = width * height;
    const int glitchHeight = static_cast<int>(params.scanlineHeight);

    for (int y = 0; y < height; y++) {
        if ((y / glitchHeight) % 2 == (static_cast<int>(params.frequency * 10) % 2)) {
            int shiftAmount = static_cast<int>(params.strength * 20);

            for (int x = 0; x < width; x++) {
                int idx = (y * width + x) * 4;
                int shiftX = std::clamp(x + shiftAmount, 0, width - 1);
                int shiftIdx = (y * width + shiftX) * 4;

                if (params.useColorShift) {
                    // Shift color channels
                    uint8_t r = frameData[shiftIdx];
                    uint8_t g = frameData[shiftIdx + 1];
                    uint8_t b = frameData[shiftIdx + 2];

                    frameData[idx] = b;
                    frameData[idx + 1] = r;
                    frameData[idx + 2] = g;
                } else {
                    std::memcpy(&frameData[idx], &frameData[shiftIdx], 4);
                }
            }
        }
    }
}

bool EffectsProcessor::applyEffects(uint8_t* frameData, int width, int height,
                                    const std::vector<std::pair<EffectType, std::shared_ptr<EffectParameters>>>& effects) {
    for (const auto& [effectType, params] : effects) {
        if (!params || !applyEffect(frameData, width, height, effectType, *params)) {
            return false;
        }
    }
    return true;
}

void EffectsProcessor::gaussianBlur(uint8_t* frameData, int width, int height,
                                    uint8_t* tempBuffer, float radius) {
    const int kernelSize = static_cast<int>(radius * 2 + 1);
    const float sigma = radius / 3.0f;
    const float sigmaSq2 = 2.0f * sigma * sigma;

    // Create Gaussian kernel
    std::vector<float> kernel(kernelSize);
    float kernelSum = 0.0f;

    for (int i = 0; i < kernelSize; i++) {
        float x = i - kernelSize / 2.0f;
        kernel[i] = std::exp(-(x * x) / sigmaSq2);
        kernelSum += kernel[i];
    }

    for (float& k : kernel) {
        k /= kernelSum;
    }

    // Apply horizontal blur
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0;
            int centerSrcIdx = (y * width + x) * 4;

            for (int kx = 0; kx < kernelSize; kx++) {
                int px = std::clamp(x + kx - kernelSize / 2, 0, width - 1);
                int srcIdx = (y * width + px) * 4;

                r += tempBuffer[srcIdx] * kernel[kx];
                g += tempBuffer[srcIdx + 1] * kernel[kx];
                b += tempBuffer[srcIdx + 2] * kernel[kx];
            }

            int dstIdx = (y * width + x) * 4;
            frameData[dstIdx] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            frameData[dstIdx + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            frameData[dstIdx + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            frameData[dstIdx + 3] = tempBuffer[centerSrcIdx + 3];
        }
    }

    // Apply vertical blur
    std::memcpy(tempBuffer, frameData, width * height * 4);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float r = 0, g = 0, b = 0;
            int centerSrcIdx = (y * width + x) * 4;

            for (int ky = 0; ky < kernelSize; ky++) {
                int py = std::clamp(y + ky - kernelSize / 2, 0, height - 1);
                int srcIdx = (py * width + x) * 4;

                r += tempBuffer[srcIdx] * kernel[ky];
                g += tempBuffer[srcIdx + 1] * kernel[ky];
                b += tempBuffer[srcIdx + 2] * kernel[ky];
            }

            int dstIdx = (y * width + x) * 4;
            frameData[dstIdx] = static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f));
            frameData[dstIdx + 1] = static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f));
            frameData[dstIdx + 2] = static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f));
            frameData[dstIdx + 3] = tempBuffer[centerSrcIdx + 3];
        }
    }
}

float EffectsProcessor::perlinNoise(float x, float y, float z) const {
    // Simplified Perlin noise implementation
    const float freq = 0.1f;
    const float amp = 1.0f;

    x *= freq;
    y *= freq;
    z *= freq;

    // Hash-based pseudo-noise
    return std::sin(x * 12.9898f + y * 78.233f + z * 45.164f) * 43758.5453f;
}

std::string EffectsProcessor::getEffectName(EffectType effectType) {
    switch (effectType) {
        case EffectType::COLOR_GRADE: return "Color Grade";
        case EffectType::BLUR: return "Blur";
        case EffectType::VIGNETTE: return "Vignette";
        case EffectType::GLOW: return "Glow";
        case EffectType::GRAYSCALE: return "Grayscale";
        case EffectType::FILM_GRAIN: return "Film Grain";
        default: return "Unknown";
    }
}

std::vector<std::pair<EffectType, std::string>> EffectsProcessor::getAllEffects() {
    return {
        {EffectType::COLOR_GRADE, "Color Grade"},
        {EffectType::BLUR, "Blur"},
        {EffectType::VIGNETTE, "Vignette"},
        {EffectType::GLOW, "Glow"},
        {EffectType::GRAYSCALE, "Grayscale"},
        {EffectType::FILM_GRAIN, "Film Grain"},
    };
}

} // namespace effects
} // namespace clipforge
