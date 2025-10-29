#include "audio_analyzer.h"
#include "../utils/logger.h"
#include <algorithm>
#include <numeric>
#include <cstring>

namespace clipforge {
namespace audio {

// ===== FFTAnalyzer Implementation =====

FFTAnalyzer::FFTAnalyzer(int fftSize, int sampleRate)
    : fftSize(fftSize), sampleRate(sampleRate) {
    // Validate FFT size is power of 2
    int powerOf2 = 1;
    while (powerOf2 < fftSize) {
        powerOf2 *= 2;
    }
    if (powerOf2 != fftSize) {
        LOG_WARNING("FFT size %d is not power of 2, rounding to %d", fftSize, powerOf2);
        this->fftSize = powerOf2;
    }

    window = generateHannWindow();
    LOG_INFO("FFTAnalyzer created: %d-point, %d Hz sample rate", this->fftSize, sampleRate);
}

AudioSpectrum FFTAnalyzer::analyze(const std::vector<float>& samples, bool useWindow) {
    if (samples.size() != static_cast<size_t>(fftSize)) {
        LOG_WARNING("Sample size mismatch: expected %d, got %zu", fftSize, samples.size());
    }

    // Apply window
    std::vector<float> processedSamples = useWindow ? applyWindow(samples) : samples;

    // Convert to complex
    std::vector<std::complex<float>> input(static_cast<size_t>(fftSize));
    for (int i = 0; i < fftSize; i++) {
        input[static_cast<size_t>(i)] = std::complex<float>(processedSamples[static_cast<size_t>(i)], 0.0f);
    }

    // Perform FFT
    std::vector<std::complex<float>> output = input;
    performFFT(input, output);

    // Calculate magnitudes
    std::vector<float> magnitudes = calculateMagnitudes(output);

    // Find peak
    float peakMagnitude = 0.0f;
    int peakBin = 0;
    for (int i = 0; i < fftSize / 2; i++) {
        if (magnitudes[static_cast<size_t>(i)] > peakMagnitude) {
            peakMagnitude = magnitudes[static_cast<size_t>(i)];
            peakBin = i;
        }
    }

    // Create spectrum
    AudioSpectrum spectrum;
    spectrum.magnitudes = magnitudes;
    spectrum.peakMagnitude = peakMagnitude;
    spectrum.peakFrequency = binToFrequency(peakBin);
    spectrum.sampleRate = sampleRate;
    spectrum.fftSize = fftSize;
    spectrum.bandLevels = getBandLevels(spectrum);

    return spectrum;
}

AudioSpectrum FFTAnalyzer::analyzeStereo(const std::vector<float>& samples, bool useWindow) {
    if (samples.size() != static_cast<size_t>(fftSize * 2)) {
        LOG_WARNING("Stereo sample size mismatch: expected %d, got %zu", fftSize * 2, samples.size());
    }

    // Extract and analyze left channel
    std::vector<float> left(static_cast<size_t>(fftSize));
    std::vector<float> right(static_cast<size_t>(fftSize));

    for (int i = 0; i < fftSize; i++) {
        left[static_cast<size_t>(i)] = samples[static_cast<size_t>(i) * 2];
        right[static_cast<size_t>(i)] = samples[static_cast<size_t>(i) * 2 + 1];
    }

    AudioSpectrum specLeft = analyze(left, useWindow);
    AudioSpectrum specRight = analyze(right, useWindow);

    // Average the two channels
    AudioSpectrum result = specLeft;
    for (size_t i = 0; i < result.magnitudes.size(); i++) {
        result.magnitudes[i] = (specLeft.magnitudes[i] + specRight.magnitudes[i]) / 2.0f;
    }

    // Recalculate peak
    float peakMagnitude = 0.0f;
    int peakBin = 0;
    for (size_t i = 0; i < result.magnitudes.size() / 2; i++) {
        if (result.magnitudes[i] > peakMagnitude) {
            peakMagnitude = result.magnitudes[i];
            peakBin = static_cast<int>(i);
        }
    }

    result.peakMagnitude = peakMagnitude;
    result.peakFrequency = binToFrequency(peakBin);
    result.bandLevels = getBandLevels(result);

    return result;
}

std::vector<float> FFTAnalyzer::getMagnitudes(const std::vector<float>& samples) {
    std::vector<float> processedSamples = applyWindow(samples);

    std::vector<std::complex<float>> input(static_cast<size_t>(fftSize));
    for (int i = 0; i < fftSize; i++) {
        input[static_cast<size_t>(i)] = std::complex<float>(processedSamples[static_cast<size_t>(i)], 0.0f);
    }

    std::vector<std::complex<float>> output = input;
    performFFT(input, output);

    return calculateMagnitudes(output);
}

std::vector<float> FFTAnalyzer::getBandLevels(const AudioSpectrum& spectrum) const {
    std::vector<float> bandLevels(7);

    std::vector<std::pair<float, float>> bandRanges = {
        {20, 60},           // Subbass
        {60, 250},          // Bass
        {250, 500},         // Lowmid
        {500, 2000},        // Mid
        {2000, 4000},       // Highmid
        {4000, 6000},       // Presence
        {6000, 20000}       // Brilliance
    };

    for (size_t i = 0; i < bandRanges.size(); i++) {
        auto [minBin, maxBin] = getBandBinRange(static_cast<FrequencyBand>(i));
        maxBin = std::min(maxBin, static_cast<int>(spectrum.magnitudes.size() - 1));

        float bandEnergy = 0.0f;
        for (int bin = minBin; bin <= maxBin; bin++) {
            bandEnergy += spectrum.magnitudes[static_cast<size_t>(bin)];
        }

        // Average the band energy
        int binCount = maxBin - minBin + 1;
        bandLevels[i] = binCount > 0 ? bandEnergy / static_cast<float>(binCount) : 0.0f;
    }

    return bandLevels;
}

std::pair<float, float> FFTAnalyzer::getBandFrequencyRange(FrequencyBand band) const {
    switch (band) {
        case FrequencyBand::SUBBASS: return {20.0f, 60.0f};
        case FrequencyBand::BASS: return {60.0f, 250.0f};
        case FrequencyBand::LOWMID: return {250.0f, 500.0f};
        case FrequencyBand::MID: return {500.0f, 2000.0f};
        case FrequencyBand::HIGHMID: return {2000.0f, 4000.0f};
        case FrequencyBand::PRESENCE: return {4000.0f, 6000.0f};
        case FrequencyBand::BRILLIANCE: return {6000.0f, 20000.0f};
        default: return {0.0f, 0.0f};
    }
}

std::pair<int, int> FFTAnalyzer::getBandBinRange(FrequencyBand band) const {
    auto [minFreq, maxFreq] = getBandFrequencyRange(band);
    return {frequencyToBin(minFreq), frequencyToBin(maxFreq)};
}

void FFTAnalyzer::performFFT(const std::vector<std::complex<float>>& input,
                            std::vector<std::complex<float>>& output) {
    output = input;
    bitReversal(output);

    for (int s = 1; s <= static_cast<int>(std::log2(fftSize)); s++) {
        int m = 1 << s;
        int halfM = m / 2;

        std::complex<float> w(1.0f, 0.0f);
        std::complex<float> wm = std::exp(std::complex<float>(0.0f, -2.0f * static_cast<float>(M_PI) / static_cast<float>(m)));

        for (int k = 0; k < halfM; k++) {
            for (int j = k; j < fftSize; j += m) {
                int t = j + halfM;
                std::complex<float> u = output[static_cast<size_t>(j)];
                std::complex<float> v = output[static_cast<size_t>(t)] * w;
                output[static_cast<size_t>(j)] = u + v;
                output[static_cast<size_t>(t)] = u - v;
            }

            w *= wm;
        }
    }
}

void FFTAnalyzer::bitReversal(std::vector<std::complex<float>>& data) {
    int n = static_cast<int>(data.size());
    int j = 0;

    for (int i = 0; i < n - 1; i++) {
        if (i < j) {
            std::swap(data[static_cast<size_t>(i)], data[static_cast<size_t>(j)]);
        }

        int mask = n / 2;
        while (j & mask) {
            j &= ~mask;
            mask >>= 1;
        }
        j |= mask;
    }
}

std::vector<float> FFTAnalyzer::generateHannWindow() const {
    std::vector<float> windowCoeffs(static_cast<size_t>(fftSize));

    for (int i = 0; i < fftSize; i++) {
        float normalized = static_cast<float>(i) / static_cast<float>(fftSize - 1);
        windowCoeffs[static_cast<size_t>(i)] = 0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * normalized));
    }

    return windowCoeffs;
}

std::vector<float> FFTAnalyzer::applyWindow(const std::vector<float>& samples) const {
    std::vector<float> windowed(samples.size());

    for (size_t i = 0; i < samples.size(); i++) {
        windowed[i] = samples[i] * window[i];
    }

    return windowed;
}

std::vector<float> FFTAnalyzer::calculateMagnitudes(
    const std::vector<std::complex<float>>& fft) const {
    std::vector<float> magnitudes(fft.size() / 2);

    for (size_t i = 0; i < magnitudes.size(); i++) {
        float real = fft[i].real();
        float imag = fft[i].imag();
        float magnitude = std::sqrt(real * real + imag * imag);

        // Normalize
        magnitudes[i] = magnitude / (static_cast<float>(fftSize) / 2.0f);

        // Log scale (20 * log10)
        if (magnitudes[i] > 0.0f) {
            magnitudes[i] = 20.0f * std::log10(magnitudes[i] + 1e-6f) / 120.0f;
            magnitudes[i] = std::clamp(magnitudes[i], 0.0f, 1.0f);
        }
    }

    return magnitudes;
}

// ===== BeatDetector Implementation =====

BeatDetector::BeatDetector() {
    LOG_INFO("BeatDetector created");
}

std::vector<BeatInfo> BeatDetector::detectBeats(
    const AudioSpectrum& spectrum,
    const AudioSpectrum& prevSpectrum) {
    std::vector<BeatInfo> beats;

    // Calculate spectral flux
    float spectralFlux = calculateSpectralFlux(spectrum.magnitudes, prevSpectrum.magnitudes);

    // Calculate band energy
    float bassEnergy = calculateBandEnergy(spectrum, m_minFrequency, m_maxFrequency);

    // Store in history
    int32_t currentTime = static_cast<int32_t>(spectrum.fftSize * 1000 / spectrum.sampleRate);
    m_energyHistory.push_back({currentTime, bassEnergy});

    if (m_energyHistory.size() > MAX_HISTORY) {
        m_energyHistory.erase(m_energyHistory.begin());
    }

    // Calculate average energy over history
    float avgEnergy = 0.0f;
    for (const auto& [time, energy] : m_energyHistory) {
        avgEnergy += energy;
    }
    avgEnergy /= static_cast<float>(m_energyHistory.size());

    // Detect beat if energy spike
    float threshold = avgEnergy * (1.0f + m_sensitivity);
    if (bassEnergy > threshold && currentTime - m_lastBeatTime > 100) {  // Avoid double-detections
        m_lastBeatTime = currentTime;

        BeatInfo beat;
        beat.strength = std::min(1.0f, (bassEnergy - avgEnergy) / (threshold - avgEnergy));
        beat.frequency = spectrum.peakFrequency;
        beat.timestamp = currentTime;
        beat.band = FrequencyBand::BASS;

        beats.push_back(beat);

        LOG_INFO("Beat detected: strength=%.2f, freq=%.0fHz", beat.strength, beat.frequency);
    }

    return beats;
}

std::vector<int32_t> BeatDetector::getRecentBeats(int timeWindowMs) const {
    std::vector<int32_t> recentBeats;

    int32_t cutoffTime = m_lastBeatTime - timeWindowMs;

    for (const auto& [time, energy] : m_energyHistory) {
        if (time >= cutoffTime) {
            recentBeats.push_back(time);
        }
    }

    return recentBeats;
}

float BeatDetector::calculateSpectralFlux(
    const std::vector<float>& current,
    const std::vector<float>& previous) const {
    if (current.size() != previous.size() || current.empty()) {
        return 0.0f;
    }

    float flux = 0.0f;
    for (size_t i = 0; i < current.size(); i++) {
        float diff = current[i] - previous[i];
        if (diff > 0.0f) {
            flux += diff * diff;
        }
    }

    return std::sqrt(flux / static_cast<float>(current.size()));
}

float BeatDetector::calculateBandEnergy(
    const AudioSpectrum& spectrum,
    float minFreq,
    float maxFreq) const {
    int minBin = static_cast<int>(static_cast<float>(spectrum.fftSize) * minFreq / static_cast<float>(spectrum.sampleRate));
    int maxBin = static_cast<int>(static_cast<float>(spectrum.fftSize) * maxFreq / static_cast<float>(spectrum.sampleRate));

    minBin = std::max(0, minBin);
    maxBin = std::min(static_cast<int>(spectrum.magnitudes.size() - 1), maxBin);

    float energy = 0.0f;
    for (int i = minBin; i <= maxBin; i++) {
        energy += spectrum.magnitudes[static_cast<size_t>(i)];
    }

    return energy / static_cast<float>(maxBin - minBin + 1);
}

// ===== OnsetDetector Implementation =====

OnsetDetector::OnsetDetector() {
    LOG_INFO("OnsetDetector created");
}

float OnsetDetector::detectOnsets(const std::vector<float>& samples, float prevEnergy) {
    m_currentEnergy = calculateRMSEnergy(samples);

    float onsetStrength = 0.0f;

    if (prevEnergy > 0.0f) {
        float energyIncrease = m_currentEnergy / (prevEnergy + 1e-6f);

        if (energyIncrease > m_onsetThreshold) {
            onsetStrength = std::min(1.0f, (energyIncrease - m_onsetThreshold) / m_onsetThreshold);
        }
    }

    return onsetStrength;
}

float OnsetDetector::calculateRMSEnergy(const std::vector<float>& samples) const {
    if (samples.empty()) {
        return 0.0f;
    }

    float sumSquares = 0.0f;
    for (float sample : samples) {
        sumSquares += sample * sample;
    }

    return std::sqrt(sumSquares / static_cast<float>(samples.size()));
}

} // namespace audio
} // namespace clipforge
