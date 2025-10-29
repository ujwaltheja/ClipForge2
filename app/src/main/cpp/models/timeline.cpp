#include "timeline.h"
#include <chrono>
#include <algorithm>

namespace clipforge {
namespace models {

// ============================================================================
// Timeline Implementation
// ============================================================================

Timeline::Timeline(int32_t width, int32_t height, float fps)
    : m_properties(), m_modifiedAt(getCurrentTimestamp()) {
    m_properties.width = width;
    m_properties.height = height;
    m_properties.frameRate = fps;
}

void Timeline::setCurrentPosition(int64_t position) {
    if (position < 0) {
        m_currentPosition = 0;
    } else if (position > m_totalDuration) {
        m_currentPosition = m_totalDuration;
    } else {
        m_currentPosition = position;
    }
}

void Timeline::updateDuration() {
    calculateDuration();
}

bool Timeline::addClip(std::shared_ptr<VideoClip> clip) {
    if (!clip) return false;

    // Check if clip already exists
    if (m_clipMap.find(clip->getId()) != m_clipMap.end()) {
        return false;  // Clip already exists
    }

    m_clipList.push_back(clip);
    m_clipMap[clip->getId()] = clip;

    // Resort and update duration
    sortClips();
    calculateDuration();
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();

    return true;
}

bool Timeline::removeClip(const std::string& clipId) {
    auto it = m_clipMap.find(clipId);
    if (it == m_clipMap.end()) {
        return false;  // Clip not found
    }

    // Remove from list
    auto listIt = std::find_if(m_clipList.begin(), m_clipList.end(),
                              [&clipId](const std::shared_ptr<VideoClip>& clip) {
                                  return clip && clip->getId() == clipId;
                              });

    if (listIt != m_clipList.end()) {
        m_clipList.erase(listIt);
    }

    // Remove from map
    m_clipMap.erase(it);

    // Update duration
    calculateDuration();
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();

    return true;
}

std::shared_ptr<VideoClip> Timeline::getClip(const std::string& clipId) const {
    auto it = m_clipMap.find(clipId);
    if (it != m_clipMap.end()) {
        return it->second;
    }
    return nullptr;
}

Timeline::ClipList Timeline::getClipsOnTrack(int32_t trackIndex) const {
    ClipList result;
    for (const auto& clip : m_clipList) {
        if (clip && clip->getTrackIndex() == trackIndex) {
            result.push_back(clip);
        }
    }
    return result;
}

Timeline::ClipList Timeline::getClipsAtTime(int64_t timeMs) const {
    ClipList result;
    for (const auto& clip : m_clipList) {
        if (clip &&
            clip->getStartPosition() <= timeMs &&
            timeMs <= clip->getEndPosition()) {
            result.push_back(clip);
        }
    }
    return result;
}

void Timeline::clearClips() {
    m_clipList.clear();
    m_clipMap.clear();
    m_selectedClipId.clear();
    m_totalDuration = 0;
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();
}

std::string Timeline::addAudioTrack(const std::string& name, const std::string& type) {
    auto track = std::make_shared<AudioTrack>(generateId(), name, type);
    m_audioTrackList.push_back(track);
    m_audioTrackMap[track->getId()] = track;
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();
    return track->getId();
}

bool Timeline::removeAudioTrack(const std::string& trackId) {
    auto it = m_audioTrackMap.find(trackId);
    if (it == m_audioTrackMap.end()) {
        return false;
    }

    // Remove from list
    auto listIt = std::find_if(m_audioTrackList.begin(), m_audioTrackList.end(),
                              [&trackId](const std::shared_ptr<AudioTrack>& track) {
                                  return track && track->getId() == trackId;
                              });

    if (listIt != m_audioTrackList.end()) {
        m_audioTrackList.erase(listIt);
    }

    // Remove from map
    m_audioTrackMap.erase(it);
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();

    return true;
}

std::shared_ptr<AudioTrack> Timeline::getAudioTrack(const std::string& trackId) const {
    auto it = m_audioTrackMap.find(trackId);
    if (it != m_audioTrackMap.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<AudioTrack> Timeline::getMainAudioTrack() const {
    for (const auto& track : m_audioTrackList) {
        if (track && track->getType() == "main") {
            return track;
        }
    }
    return nullptr;
}

void Timeline::clearAudioTracks() {
    m_audioTrackList.clear();
    m_audioTrackMap.clear();
    m_modified = true;
    m_modifiedAt = getCurrentTimestamp();
}

void Timeline::selectClip(const std::string& clipId) {
    m_selectedClipId = clipId;
}

void Timeline::deselectAll() {
    m_selectedClipId.clear();
}

std::shared_ptr<VideoClip> Timeline::getSelectedClip() const {
    if (m_selectedClipId.empty()) {
        return nullptr;
    }
    return getClip(m_selectedClipId);
}

bool Timeline::isValid() const {
    // Check if timeline has minimum requirements
    if (m_clipList.empty()) {
        return false;  // Need at least one clip
    }

    // Check all clips are valid
    for (const auto& clip : m_clipList) {
        if (!clip || !clip->isValid()) {
            return false;
        }
    }

    return true;
}

bool Timeline::hasOverlappingClips() const {
    for (size_t i = 0; i < m_clipList.size(); ++i) {
        for (size_t j = i + 1; j < m_clipList.size(); ++j) {
            const auto& clip1 = m_clipList[i];
            const auto& clip2 = m_clipList[j];

            if (!clip1 || !clip2) continue;

            // Only check overlaps on same track
            if (clip1->getTrackIndex() != clip2->getTrackIndex()) {
                continue;
            }

            // Check if clips overlap
            if (clip1->getStartPosition() < clip2->getEndPosition() &&
                clip2->getStartPosition() < clip1->getEndPosition()) {
                return true;
            }
        }
    }
    return false;
}

bool Timeline::getFirstOverlap(std::shared_ptr<VideoClip>& outClip1,
                               std::shared_ptr<VideoClip>& outClip2) const {
    for (size_t i = 0; i < m_clipList.size(); ++i) {
        for (size_t j = i + 1; j < m_clipList.size(); ++j) {
            const auto& clip1 = m_clipList[i];
            const auto& clip2 = m_clipList[j];

            if (!clip1 || !clip2) continue;
            if (clip1->getTrackIndex() != clip2->getTrackIndex()) continue;

            if (clip1->getStartPosition() < clip2->getEndPosition() &&
                clip2->getStartPosition() < clip1->getEndPosition()) {
                outClip1 = clip1;
                outClip2 = clip2;
                return true;
            }
        }
    }
    return false;
}

int32_t Timeline::getMaxTrackInUse() const {
    int32_t maxTrack = 0;
    for (const auto& clip : m_clipList) {
        if (clip && clip->getTrackIndex() > maxTrack) {
            maxTrack = clip->getTrackIndex();
        }
    }
    return maxTrack;
}

void Timeline::clearChangesFlag() {
    m_modified = false;
    for (const auto& clip : m_clipList) {
        if (clip) {
            clip->clearChangesFlag();
        }
    }
    for (const auto& track : m_audioTrackList) {
        if (track) {
            track->clearChangesFlag();
        }
    }
}

void Timeline::sortClips() {
    std::sort(m_clipList.begin(), m_clipList.end(),
             [](const std::shared_ptr<VideoClip>& a, const std::shared_ptr<VideoClip>& b) {
                 if (!a || !b) return false;
                 // First sort by track, then by start position
                 if (a->getTrackIndex() != b->getTrackIndex()) {
                     return a->getTrackIndex() < b->getTrackIndex();
                 }
                 return a->getStartPosition() < b->getStartPosition();
             });
}

void Timeline::calculateDuration() {
    m_totalDuration = 0;

    for (const auto& clip : m_clipList) {
        if (clip) {
            int64_t clipEnd = clip->getEndPosition();
            if (clipEnd > m_totalDuration) {
                m_totalDuration = clipEnd;
            }
        }
    }
}

int64_t Timeline::getCurrentTimestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    return duration_cast<milliseconds>(now.time_since_epoch()).count();
}

std::string Timeline::generateId() {
    static unsigned int counter = 0;
    std::string id = "tl_";
    id += std::to_string(std::time(nullptr));
    id += "_";
    id += std::to_string(++counter);
    return id;
}

} // namespace models
} // namespace clipforge
