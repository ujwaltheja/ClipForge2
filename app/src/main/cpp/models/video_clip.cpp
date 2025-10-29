#include "video_clip.h"
#include <chrono>
#include <algorithm>

namespace clipforge {
namespace models {

// ============================================================================
// VideoClip Implementation
// ============================================================================

VideoClip::VideoClip(const std::string& id, const std::string& sourceFile)
    : m_id(id), m_metadata(), m_createdAt(getCurrentTimestamp()),
      m_modifiedAt(m_createdAt) {
    m_metadata.sourceFile = sourceFile;
    m_name = "Clip_" + sourceFile.substr(sourceFile.find_last_of("/\\") + 1);
}

void VideoClip::setSpeed(float speed) {
    // Clamp speed to reasonable bounds (0.25x to 4x)
    m_speed = std::max(0.25f, std::min(4.0f, speed));
    updateModificationTime();
}

void VideoClip::setVolume(float volume) {
    // Clamp volume to 0.0 - 2.0
    m_volume = std::max(0.0f, std::min(2.0f, volume));
    updateModificationTime();
}

void VideoClip::updateModificationTime() {
    m_modifiedAt = getCurrentTimestamp();
    m_modified = true;
}

bool VideoClip::isValid() const {
    // Clip is valid if it has required metadata and valid timeline position
    if (m_metadata.sourceFile.empty()) {
        return false;  // No source file
    }

    // Check trim range is valid
    if (m_trimStart < 0 || m_trimEnd < 0) {
        return false;  // Invalid trim values
    }

    if (m_trimStart >= m_trimEnd) {
        return false;  // Trim range inverted
    }

    if (m_startPosition < 0) {
        return false;  // Negative timeline position
    }

    if (m_duration <= 0) {
        return false;  // Invalid duration
    }

    return true;
}

int64_t VideoClip::getCurrentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

} // namespace models
} // namespace clipforge
