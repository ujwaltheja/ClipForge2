#ifndef CLIPFORGE_AUDIO_EFFECTS_H
#define CLIPFORGE_AUDIO_EFFECTS_H

#include <vector>
#include <cstdint>
#include <cmath>
#include <queue>

namespace clipforge {
namespace audio {

/**
 * @enum AudioEffectType
 * @brief Available audio effects
 */
enum class AudioEffectType {
    EQUALIZER = 0,
    COMPRESSOR = 1,
    REVERB = 2,
    DELAY = 3,
    CHORUS = 4,
    DISTORTION = 5,
    LIMITER = 6,
    GATE = 7,
    FADE = 8,
};

/**
 * @struct EqualizerBand
 * @brief Single equalizer band
 */
struct EqualizerBand {
    float frequency = 1000.0f;  // Center frequency in Hz
    float gain = 0.0f;          // Gain in dB (-24 to +24)
    float q = 1.0f;             // Q factor (bandwidth)
};

/**
 * @class EqualizerEffect
 * @brief Parametric equalizer with multiple bands
 *
 * Supports up to 10 frequency bands for precise control
 */
class EqualizerEffect {
public:
    /**
     * @brief Create equalizer with default bands
     */
    EqualizerEffect();

    /**
     * @brief Set number of bands (1-10)
     * @param numBands Number of equalizer bands
     */
    void setNumBands(int numBands);

    /**
     * @brief Set band parameters
     * @param bandIndex Band index (0-9)
     * @param frequency Center frequency in Hz
     * @param gain Gain in dB (-24 to +24)
     * @param q Q factor
     */
    void setBand(int bandIndex, float frequency, float gain, float q);

    /**
     * @brief Get band
     * @param bandIndex Band index
     * @return Equalizer band parameters
     */
    [[nodiscard]] const EqualizerBand& getBand(int bandIndex) const;

    /**
     * @brief Process audio samples through equalizer
     * @param samples Audio samples
     * @param sampleRate Sample rate in Hz
     */
    void process(std::vector<float>& samples, int sampleRate);

    /**
     * @brief Set preset equalizer curve
     * @param presetName Preset name (bass_boost, treble_boost, presence, warm, bright, tight)
     */
    void setPreset(const std::string& presetName);

private:
    std::vector<EqualizerBand> m_bands;
    std::vector<float> m_history;  // For IIR filter state

    // Biquad filter coefficients
    struct BiquadCoefficients {
        float b0, b1, b2, a1, a2;
    };

    [[nodiscard]] BiquadCoefficients calculateBiquadCoefficients(
        const EqualizerBand& band, float sampleRate) const;

    void processBiquad(std::vector<float>& samples,
                      const BiquadCoefficients& coeffs);
};

/**
 * @struct CompressorParams
 * @brief Compressor/limiter parameters
 */
struct CompressorParams {
    float threshold = -20.0f;    // Threshold in dB
    float ratio = 4.0f;          // Compression ratio (1:1 to 8:1)
    float attackMs = 10.0f;      // Attack time in milliseconds
    float releaseMs = 100.0f;    // Release time in milliseconds
    float makeupGain = 0.0f;     // Makeup gain in dB
};

/**
 * @class Compressor
 * @brief Audio compressor/limiter effect
 *
 * Reduces dynamic range by lowering loud parts and raising quiet parts
 */
class Compressor {
public:
    /**
     * @brief Create compressor with parameters
     * @param params Compressor parameters
     */
    explicit Compressor(const CompressorParams& params = CompressorParams());

    /**
     * @brief Set compressor parameters
     * @param params New parameters
     */
    void setParameters(const CompressorParams& params);

    /**
     * @brief Get current parameters
     * @return Compressor parameters
     */
    [[nodiscard]] const CompressorParams& getParameters() const { return m_params; }

    /**
     * @brief Process audio samples
     * @param samples Audio samples
     * @param sampleRate Sample rate in Hz
     */
    void process(std::vector<float>& samples, int sampleRate);

    /**
     * @brief Get current gain reduction in dB
     * @return Gain reduction (negative value)
     */
    [[nodiscard]] float getCurrentGainReduction() const { return m_gainReduction; }

private:
    CompressorParams m_params;
    float m_gainReduction = 0.0f;
    float m_envelopeValue = 0.0f;

    [[nodiscard]] float calculateGainReduction(float inputDb) const;
    [[nodiscard]] float dbToLinear(float db) const;
    [[nodiscard]] float linearToDb(float linear) const;
};

/**
 * @struct ReverbParams
 * @brief Reverb effect parameters
 */
struct ReverbParams {
    float roomSize = 0.5f;       // Room size (0-1)
    float damping = 0.5f;        // Damping (0-1)
    float width = 1.0f;          // Stereo width (0-1)
    float dryLevel = 0.4f;       // Dry signal level (0-1)
    float wetLevel = 0.3f;       // Wet signal level (0-1)
};

/**
 * @class Reverb
 * @brief Reverb effect using Freeverb algorithm
 *
 * Creates realistic room acoustics simulation
 */
class Reverb {
public:
    /**
     * @brief Create reverb effect
     * @param params Reverb parameters
     */
    explicit Reverb(const ReverbParams& params = ReverbParams());

    /**
     * @brief Set reverb parameters
     * @param params New parameters
     */
    void setParameters(const ReverbParams& params);

    /**
     * @brief Get current parameters
     * @return Reverb parameters
     */
    [[nodiscard]] const ReverbParams& getParameters() const { return m_params; }

    /**
     * @brief Process audio samples
     * @param samples Audio samples
     * @param sampleRate Sample rate in Hz
     */
    void process(std::vector<float>& samples, int sampleRate);

    /**
     * @brief Set reverb preset
     * @param presetName Preset (small_room, hall, large_hall, cathedral, plate)
     */
    void setPreset(const std::string& presetName);

private:
    ReverbParams m_params;
    std::vector<std::queue<float>> m_comb_filters;
    std::vector<std::queue<float>> m_allpass_filters;

    void updateCombFilter(int index, float feedback);
};

/**
 * @struct DelayParams
 * @brief Delay effect parameters
 */
struct DelayParams {
    float delayMs = 500.0f;      // Delay time in milliseconds
    float feedback = 0.5f;       // Feedback amount (0-1)
    float mix = 0.3f;            // Dry/wet mix (0-1)
};

/**
 * @class Delay
 * @brief Delay/echo effect
 */
class Delay {
public:
    /**
     * @brief Create delay effect
     * @param params Delay parameters
     */
    explicit Delay(const DelayParams& params = DelayParams());

    /**
     * @brief Set delay parameters
     * @param params New parameters
     */
    void setParameters(const DelayParams& params);

    /**
     * @brief Process audio samples
     * @param samples Audio samples
     * @param sampleRate Sample rate in Hz
     */
    void process(std::vector<float>& samples, int sampleRate);

private:
    DelayParams m_params;
    std::vector<float> m_delayBuffer;
    size_t m_writeIndex = 0;
};

/**
 * @class AudioEffectChain
 * @brief Chain multiple audio effects in sequence
 */
class AudioEffectChain {
public:
    /**
     * @brief Create audio effect chain
     */
    AudioEffectChain();

    /**
     * @brief Add equalizer to chain
     * @return Reference to equalizer
     */
    EqualizerEffect& addEqualizer();

    /**
     * @brief Add compressor to chain
     * @param params Compressor parameters
     * @return Reference to compressor
     */
    Compressor& addCompressor(const CompressorParams& params = CompressorParams());

    /**
     * @brief Add reverb to chain
     * @param params Reverb parameters
     * @return Reference to reverb
     */
    Reverb& addReverb(const ReverbParams& params = ReverbParams());

    /**
     * @brief Add delay to chain
     * @param params Delay parameters
     * @return Reference to delay
     */
    Delay& addDelay(const DelayParams& params = DelayParams());

    /**
     * @brief Process audio through all effects
     * @param samples Audio samples
     * @param sampleRate Sample rate in Hz
     */
    void process(std::vector<float>& samples, int sampleRate);

    /**
     * @brief Clear all effects
     */
    void clearEffects();

private:
    std::vector<std::unique_ptr<EqualizerEffect>> m_equalizers;
    std::vector<std::unique_ptr<Compressor>> m_compressors;
    std::vector<std::unique_ptr<Reverb>> m_reverbs;
    std::vector<std::unique_ptr<Delay>> m_delays;
};

/**
 * @class VolumeFade
 * @brief Volume fade in/out effect
 */
class VolumeFade {
public:
    /**
     * @brief Apply fade in effect
     * @param samples Audio samples
     * @param durationMs Fade duration in milliseconds
     * @param sampleRate Sample rate in Hz
     */
    static void fadeIn(std::vector<float>& samples, float durationMs, int sampleRate);

    /**
     * @brief Apply fade out effect
     * @param samples Audio samples
     * @param durationMs Fade duration in milliseconds
     * @param sampleRate Sample rate in Hz
     */
    static void fadeOut(std::vector<float>& samples, float durationMs, int sampleRate);

    /**
     * @brief Apply crossfade between two audio segments
     * @param samples1 First audio samples
     * @param samples2 Second audio samples
     * @param crossfadeDurationMs Crossfade duration in milliseconds
     * @param sampleRate Sample rate in Hz
     * @return Combined audio with crossfade
     */
    static std::vector<float> crossfade(const std::vector<float>& samples1,
                                       const std::vector<float>& samples2,
                                       float crossfadeDurationMs, int sampleRate);
};

} // namespace audio
} // namespace clipforge

#endif // CLIPFORGE_AUDIO_EFFECTS_H
