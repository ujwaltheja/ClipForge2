#include "audio_track.h"
#include <chrono>
#include <algorithm>

namespace clipforge {
namespace models {

// ============================================================================
// AudioTrack Implementation
// ============================================================================

AudioTrack::AudioTrack(const std::string& id, const std::string& name,
                      const std::string& type)
    : m_id(id), m_name(name.empty() ? "Audio Track" : name), m_type(type),
      m_createdAt(getCurrentTimestamp()), m_modifiedAt(m_createdAt) {}

void AudioTrack::setVolume(float volume) {
    m_volume = clamp(volume, 0.0f, 2.0f);
    markModified();
}

void AudioTrack::setPan(float pan) {
    m_pan = clamp(pan, -1.0f, 1.0f);
    markModified();
}

void AudioTrack::setBass(float bass) {
    m_bass = clamp(bass, -1.0f, 1.0f);
    markModified();
}

void AudioTrack::setMidrange(float mid) {
    m_midrange = clamp(mid, -1.0f, 1.0f);
    markModified();
}

void AudioTrack::setTreble(float treble) {
    m_treble = clamp(treble, -1.0f, 1.0f);
    markModified();
}

void AudioTrack::setReverb(float amount) {
    m_reverb = clamp(amount, 0.0f, 1.0f);
    markModified();
}

void AudioTrack::setCompression(float ratio) {
    m_compression = clamp(ratio, 1.0f, 8.0f);
    markModified();
}

void AudioTrack::setPitchShift(float semitones) {
    m_pitchShift = clamp(semitones, -12.0f, 12.0f);
    markModified();
}

void AudioTrack::markModified() {
    m_modifiedAt = getCurrentTimestamp();
    m_modified = true;
}

int64_t AudioTrack::getCurrentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

float AudioTrack::clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

} // namespace models
} // namespace clipforge
