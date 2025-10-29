#ifndef CLIPFORGE_AUDIO_ANALYZER_H
#define CLIPFORGE_AUDIO_ANALYZER_H

#include <vector>
#include <complex>
#include <memory>
#include <cstdint>
#include <cmath>

namespace clipforge {
namespace audio {

/**
 * @enum FrequencyBand
 * @brief Audio frequency bands for visualization and analysis
 */
enum class FrequencyBand {
    SUBBASS = 0,      // 20-60 Hz
    BASS = 1,         // 60-250 Hz
    LOWMID = 2,       // 250-500 Hz
    MID = 3,          // 500-2000 Hz
    HIGHMID = 4,      // 2000-4000 Hz
    PRESENCE = 5,     // 4000-6000 Hz
    BRILLIANCE = 6,   // 6000-20000 Hz
};

/**
 * @struct AudioSpectrum
 * @brief Frequency domain representation of audio
 */
struct AudioSpectrum {
    std::vector<float> magnitudes;           // Magnitude of each frequency bin
    std::vector<float> phases;               // Phase of each frequency bin
    std::vector<float> bandLevels;           // Energy in each frequency band (0-1)
    float peakFrequency = 0.0f;              // Dominant frequency
    float peakMagnitude = 0.0f;              // Peak magnitude
    int sampleRate = 44100;
    int fftSize = 2048;
};

/**
 * @struct BeatInfo
 * @brief Information about detected beat
 */
struct BeatInfo {
    float strength = 0.0f;                   // Beat confidence (0-1)
    float frequency = 0.0f;                  // Frequency of beat
    int32_t timestamp = 0;                   // Time in milliseconds
    FrequencyBand band = FrequencyBand::BASS; // Band where beat detected
};

/**
 * @class FFTAnalyzer
 * @brief Fast Fourier Transform implementation for audio analysis
 *
 * Implements Cooley-Tukey FFT algorithm for efficient frequency domain analysis.
 * Used for:
 * - Spectrum visualization
 * - Beat detection
 * - Frequency-specific effect control
 *
 * Performance:
 * - O(n log n) complexity
 * - 1024/2048/4096 point FFT
 * - ~1-2ms for 2048 point FFT on mobile
 */
class FFTAnalyzer {
public:
    /**
     * @brief Create FFT analyzer
     * @param fftSize FFT size (must be power of 2: 512, 1024, 2048, 4096)
     * @param sampleRate Audio sample rate (44100 or 48000)
     */
    FFTAnalyzer(int fftSize = 2048, int sampleRate = 44100);

    /**
     * @brief Destructor
     */
    ~FFTAnalyzer() = default;

    // ===== FFT Processing =====

    /**
     * @brief Perform FFT on audio samples
     * @param samples Audio samples (must be fftSize length)
     * @param useWindow Apply Hann window to avoid spectral leakage
     * @return Frequency spectrum
     */
    [[nodiscard]] AudioSpectrum analyze(const std::vector<float>& samples, bool useWindow = true);

    /**
     * @brief Perform FFT on interleaved stereo audio
     * @param samples Stereo samples (length = fftSize * 2)
     * @param useWindow Apply windowing
     * @return Combined spectrum (averaged across channels)
     */
    [[nodiscard]] AudioSpectrum analyzeStereo(const std::vector<float>& samples, bool useWindow = true);

    /**
     * @brief Get magnitude spectrum only (faster than full analysis)
     * @param samples Audio samples
     * @return Magnitude vector
     */
    [[nodiscard]] std::vector<float> getMagnitudes(const std::vector<float>& samples);

    /**
     * @brief Get frequency band energy levels
     * @param spectrum Spectrum from analyze()
     * @return Energy in each of 7 bands
     */
    [[nodiscard]] std::vector<float> getBandLevels(const AudioSpectrum& spectrum) const;

    // ===== Frequency Conversion =====

    /**
     * @brief Convert bin index to frequency
     * @param binIndex FFT bin index
     * @return Frequency in Hz
     */
    [[nodiscard]] float binToFrequency(int binIndex) const {
        return (static_cast<float>(binIndex) * sampleRate) / fftSize;
    }

    /**
     * @brief Convert frequency to bin index
     * @param frequency Frequency in Hz
     * @return FFT bin index
     */
    [[nodiscard]] int frequencyToBin(float frequency) const {
        return static_cast<int>((frequency * fftSize) / sampleRate);
    }

    /**
     * @brief Get frequency band range
     * @param band Frequency band
     * @return {minFrequency, maxFrequency}
     */
    [[nodiscard]] std::pair<float, float> getBandFrequencyRange(FrequencyBand band) const;

    /**
     * @brief Get bin range for frequency band
     * @param band Frequency band
     * @return {minBin, maxBin}
     */
    [[nodiscard]] std::pair<int, int> getBandBinRange(FrequencyBand band) const;

    // ===== Properties =====

    /**
     * @brief Get FFT size
     * @return FFT size (power of 2)
     */
    [[nodiscard]] int getFFTSize() const { return fftSize; }

    /**
     * @brief Get sample rate
     * @return Sample rate in Hz
     */
    [[nodiscard]] int getSampleRate() const { return sampleRate; }

    /**
     * @brief Get frequency resolution
     * @return Hz per bin
     */
    [[nodiscard]] float getFrequencyResolution() const {
        return static_cast<float>(sampleRate) / fftSize;
    }

    /**
     * @brief Get Nyquist frequency (max representable frequency)
     * @return Nyquist frequency
     */
    [[nodiscard]] float getNyquistFrequency() const {
        return sampleRate / 2.0f;
    }

private:
    int fftSize;
    int sampleRate;
    std::vector<float> window;  // Hann window coefficients

    /**
     * @brief Cooley-Tukey FFT algorithm
     * @param input Complex input signal
     * @param output Complex output spectrum
     */
    void performFFT(const std::vector<std::complex<float>>& input,
                   std::vector<std::complex<float>>& output);

    /**
     * @brief Bit-reversal permutation
     * @param data Complex data array
     */
    void bitReversal(std::vector<std::complex<float>>& data);

    /**
     * @brief Generate Hann window coefficients
     * @return Window values
     */
    [[nodiscard]] std::vector<float> generateHannWindow() const;

    /**
     * @brief Apply window to samples
     * @param samples Input samples
     * @return Windowed samples
     */
    [[nodiscard]] std::vector<float> applyWindow(const std::vector<float>& samples) const;

    /**
     * @brief Calculate magnitude spectrum from complex FFT result
     * @param fft Complex FFT output
     * @return Magnitude spectrum (0-1 normalized)
     */
    [[nodiscard]] std::vector<float> calculateMagnitudes(
        const std::vector<std::complex<float>>& fft) const;
};

/**
 * @class BeatDetector
 * @brief Real-time beat detection from audio spectrum
 *
 * Uses spectral flux and energy analysis to detect beats in audio.
 * Particularly responsive to kicks and drums in bass frequencies.
 */
class BeatDetector {
public:
    /**
     * @brief Create beat detector
     */
    BeatDetector();

    /**
     * @brief Process audio spectrum and detect beats
     * @param spectrum Current audio spectrum
     * @param prevSpectrum Previous spectrum for flux calculation
     * @return Beat information if beat detected
     */
    [[nodiscard]] std::vector<BeatInfo> detectBeats(
        const AudioSpectrum& spectrum,
        const AudioSpectrum& prevSpectrum
    );

    /**
     * @brief Set beat sensitivity
     * @param sensitivity Sensitivity (0.1 = very sensitive, 1.0 = normal, 10.0 = insensitive)
     */
    void setSensitivity(float sensitivity) { m_sensitivity = sensitivity; }

    /**
     * @brief Set minimum beat frequency range
     * @param minHz Minimum frequency
     * @param maxHz Maximum frequency
     */
    void setFrequencyRange(float minHz, float maxHz) {
        m_minFrequency = minHz;
        m_maxFrequency = maxHz;
    }

    /**
     * @brief Reset beat detector state
     */
    void reset() {
        m_lastBeatTime = 0;
        m_energyHistory.clear();
    }

    /**
     * @brief Get recent beat times
     * @return Vector of beat timestamps
     */
    [[nodiscard]] std::vector<int32_t> getRecentBeats(int timeWindowMs = 2000) const;

private:
    float m_sensitivity = 1.0f;
    float m_minFrequency = 60.0f;   // Focus on bass region
    float m_maxFrequency = 250.0f;
    int32_t m_lastBeatTime = 0;
    std::vector<std::pair<int32_t, float>> m_energyHistory;  // timestamp, energy
    static constexpr int MAX_HISTORY = 200;

    /**
     * @brief Calculate spectral flux (change between spectra)
     * @param current Current magnitude spectrum
     * @param previous Previous magnitude spectrum
     * @return Spectral flux value
     */
    [[nodiscard]] float calculateSpectralFlux(
        const std::vector<float>& current,
        const std::vector<float>& previous
    ) const;

    /**
     * @brief Calculate energy in frequency band
     * @param spectrum Audio spectrum
     * @param minFreq Minimum frequency
     * @param maxFreq Maximum frequency
     * @return Energy level (0-1)
     */
    [[nodiscard]] float calculateBandEnergy(
        const AudioSpectrum& spectrum,
        float minFreq,
        float maxFreq
    ) const;
};

/**
 * @class OnsetDetector
 * @brief Detects note onsets and transients in audio
 *
 * Useful for detecting percussive sounds, attacks in instruments,
 * and other quick-onset events.
 */
class OnsetDetector {
public:
    /**
     * @brief Create onset detector
     */
    OnsetDetector();

    /**
     * @brief Detect onsets in audio frame
     * @param samples Audio samples
     * @param prevEnergy Previous frame's RMS energy
     * @return Onset strength (0-1)
     */
    [[nodiscard]] float detectOnsets(const std::vector<float>& samples, float prevEnergy);

    /**
     * @brief Get current energy
     * @return RMS energy of current frame
     */
    [[nodiscard]] float getCurrentEnergy() const { return m_currentEnergy; }

private:
    float m_currentEnergy = 0.0f;
    float m_onsetThreshold = 1.5f;  // Energy increase multiplier

    /**
     * @brief Calculate RMS (Root Mean Square) energy
     * @param samples Audio samples
     * @return RMS energy value
     */
    [[nodiscard]] float calculateRMSEnergy(const std::vector<float>& samples) const;
};

} // namespace audio
} // namespace clipforge

#endif // CLIPFORGE_AUDIO_ANALYZER_H
