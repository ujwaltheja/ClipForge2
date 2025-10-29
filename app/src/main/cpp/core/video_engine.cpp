#include "video_engine.h"
#include "../utils/logger.h"
#include <thread>
#include <chrono>

namespace clipforge {
namespace core {

// ============================================================================
// VideoEngine Implementation
// ============================================================================

VideoEngine::VideoEngine() {
    LOG_DEBUG("VideoEngine instance created");
}

VideoEngine::~VideoEngine() {
    LOG_DEBUG("VideoEngine destructing");
    shutdown();
}

bool VideoEngine::initialize(const EngineConfig& config) {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    if (m_state != EngineState::IDLE) {
        setError("Engine already initialized or in error state");
        return false;
    }

    m_config = config;
    LOG_INFO("VideoEngine initialized with config");
    LOG_INFO("  Preview Quality: %d", m_config.previewQuality);
    LOG_INFO("  Hardware Accel: %s", m_config.useHardwareAccel ? "true" : "false");
    LOG_INFO("  Max Threads: %d", m_config.maxRenderThreads);

    // Initialize subsystems
    if (!loadEffectsLibrary()) {
        setError("Failed to load effects library");
        return false;
    }

    m_state = EngineState::IDLE;
    return true;
}

void VideoEngine::shutdown() {
    std::lock_guard<std::mutex> lock(m_stateMutex);

    LOG_INFO("Shutting down VideoEngine");

    // Stop preview if running
    if (m_previewPlaying.exchange(false)) {
        if (m_previewThread && m_previewThread->joinable()) {
            m_previewThread->join();
        }
    }

    // Cancel export if in progress
    if (m_exporting.exchange(false)) {
        m_cancelExport = true;
        if (m_exportThread && m_exportThread->joinable()) {
            m_exportThread->join();
        }
    }

    m_timeline = nullptr;
    m_state = EngineState::SHUTDOWN;
}

EngineState VideoEngine::getState() const {
    return m_state.load();
}

std::string VideoEngine::getErrorMessage() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_errorMessage;
}

bool VideoEngine::setTimeline(std::shared_ptr<models::Timeline> timeline) {
    if (!timeline) {
        setError("Null timeline provided");
        return false;
    }

    m_timeline = timeline;
    LOG_INFO("Timeline set: %d clips, %lld ms duration",
            timeline->getClipCount(), timeline->getTotalDuration());
    return true;
}

std::string VideoEngine::addClip(const std::string& sourcePath,
                                int64_t startPosition, int32_t trackIndex) {
    if (!m_timeline) {
        setError("No timeline set");
        return "";
    }

    // Create new clip
    auto clip = std::make_shared<models::VideoClip>("clip_" + std::to_string(std::time(nullptr)),
                                                     sourcePath);
    clip->setStartPosition(startPosition);
    clip->setTrackIndex(trackIndex);

    if (!m_timeline->addClip(clip)) {
        setError("Failed to add clip to timeline");
        return "";
    }

    LOG_INFO("Clip added: %s (track %d, pos %lld ms)",
            clip->getId().c_str(), trackIndex, startPosition);
    return clip->getId();
}

bool VideoEngine::removeClip(const std::string& clipId) {
    if (!m_timeline) {
        setError("No timeline set");
        return false;
    }

    if (!m_timeline->removeClip(clipId)) {
        setError("Clip not found: " + clipId);
        return false;
    }

    LOG_INFO("Clip removed: %s", clipId.c_str());
    return true;
}

bool VideoEngine::moveClip(const std::string& clipId, int64_t newStartPosition,
                          int32_t newTrackIndex) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    clip->setStartPosition(newStartPosition);
    clip->setTrackIndex(newTrackIndex);

    m_timeline->updateDuration();
    LOG_DEBUG("Clip moved: %s (new pos %lld ms, track %d)",
             clipId.c_str(), newStartPosition, newTrackIndex);
    return true;
}

bool VideoEngine::trimClip(const std::string& clipId, int64_t trimStart, int64_t trimEnd) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    if (trimStart >= trimEnd) {
        setError("Invalid trim range");
        return false;
    }

    clip->setTrimStart(trimStart);
    clip->setTrimEnd(trimEnd);
    clip->updateModificationTime();

    LOG_DEBUG("Clip trimmed: %s (%lld-%lld ms)",
             clipId.c_str(), trimStart, trimEnd);
    return true;
}

bool VideoEngine::setClipSpeed(const std::string& clipId, float speed) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    clip->setSpeed(speed);
    LOG_DEBUG("Clip speed set: %s (%.2fx)", clipId.c_str(), speed);
    return true;
}

bool VideoEngine::setClipVolume(const std::string& clipId, float volume) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    clip->setVolume(volume);
    LOG_DEBUG("Clip volume set: %s (%.2f)", clipId.c_str(), volume);
    return true;
}

std::string VideoEngine::splitClip(const std::string& clipId, int64_t splitTime) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return "";
    }

    // Create new clip for the second part
    auto newClip = std::make_shared<models::VideoClip>(
        "clip_" + std::to_string(std::time(nullptr)),
        clip->getSourceFile()
    );

    // Copy metadata and settings
    newClip->setMetadata(clip->getMetadata());
    newClip->setTrackIndex(clip->getTrackIndex());
    newClip->setStartPosition(clip->getStartPosition() + splitTime);
    newClip->setTrimStart(clip->getTrimStart() + splitTime);
    newClip->setTrimEnd(clip->getTrimEnd());

    if (!m_timeline->addClip(newClip)) {
        setError("Failed to create split clip");
        return "";
    }

    // Update original clip
    clip->setDuration(splitTime);
    clip->setTrimEnd(clip->getTrimStart() + splitTime);

    LOG_INFO("Clip split: %s at %lld ms, created %s",
            clipId.c_str(), splitTime, newClip->getId().c_str());
    return newClip->getId();
}

bool VideoEngine::applyEffect(const std::string& clipId,
                             std::shared_ptr<models::Effect> effect) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    clip->applyEffect(effect);
    LOG_DEBUG("Effect applied: %s to clip %s",
             effect->getName().c_str(), clipId.c_str());
    return true;
}

bool VideoEngine::removeEffect(const std::string& clipId, const std::string& effectId) {
    auto clip = m_timeline->getClip(clipId);
    if (!clip) {
        setError("Clip not found: " + clipId);
        return false;
    }

    if (!clip->removeEffect(effectId)) {
        setError("Effect not found: " + effectId);
        return false;
    }

    LOG_DEBUG("Effect removed: %s from clip %s",
             effectId.c_str(), clipId.c_str());
    return true;
}

std::vector<std::shared_ptr<models::Effect>> VideoEngine::getAvailableEffects() const {
    std::vector<std::shared_ptr<models::Effect>> effects;

    // Create basic effects library
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::FILTER_VINTAGE, "Vintage"));
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::FILTER_BW, "Black & White"));
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::FILTER_SEPIA, "Sepia"));
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::COLOR_BRIGHTNESS, "Brightness"));
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::COLOR_CONTRAST, "Contrast"));
    effects.push_back(std::make_shared<models::Effect>(
        models::EffectType::COLOR_SATURATION, "Saturation"));

    return effects;
}

std::shared_ptr<models::Effect> VideoEngine::createEffect(const std::string& effectName) {
    // Map effect names to types
    if (effectName == "Vintage") {
        return std::make_shared<models::Effect>(
            models::EffectType::FILTER_VINTAGE, "Vintage");
    } else if (effectName == "Black & White") {
        return std::make_shared<models::Effect>(
            models::EffectType::FILTER_BW, "Black & White");
    }

    return nullptr;
}

std::string VideoEngine::addAudioTrack(const std::string& name, const std::string& type) {
    if (!m_timeline) {
        setError("No timeline set");
        return "";
    }

    std::string trackId = m_timeline->addAudioTrack(name, type);
    LOG_INFO("Audio track added: %s (type: %s)", trackId.c_str(), type.c_str());
    return trackId;
}

bool VideoEngine::removeAudioTrack(const std::string& trackId) {
    if (!m_timeline) {
        setError("No timeline set");
        return false;
    }

    if (!m_timeline->removeAudioTrack(trackId)) {
        setError("Audio track not found: " + trackId);
        return false;
    }

    LOG_INFO("Audio track removed: %s", trackId.c_str());
    return true;
}

bool VideoEngine::setAudioTrackVolume(const std::string& trackId, float volume) {
    auto track = m_timeline->getAudioTrack(trackId);
    if (!track) {
        setError("Audio track not found: " + trackId);
        return false;
    }

    track->setVolume(volume);
    LOG_DEBUG("Audio track volume set: %s (%.2f)", trackId.c_str(), volume);
    return true;
}

bool VideoEngine::setAudioTrackMuted(const std::string& trackId, bool muted) {
    auto track = m_timeline->getAudioTrack(trackId);
    if (!track) {
        setError("Audio track not found: " + trackId);
        return false;
    }

    track->setMuted(muted);
    LOG_DEBUG("Audio track muted: %s (%s)", trackId.c_str(),
             muted ? "true" : "false");
    return true;
}

int64_t VideoEngine::getTimelineDuration() const {
    if (!m_timeline) return 0;
    return m_timeline->getTotalDuration();
}

bool VideoEngine::startPreview() {
    if (m_previewPlaying) {
        return false;  // Already playing
    }

    if (!m_timeline || m_timeline->isEmpty()) {
        setError("No timeline to preview");
        return false;
    }

    m_previewPlaying = true;
    m_previewThread = std::make_unique<std::thread>(
        &VideoEngine::previewPlaybackThread, this);

    setState(EngineState::PREVIEW_PLAYING);
    LOG_INFO("Preview started");
    return true;
}

bool VideoEngine::pausePreview() {
    if (!m_previewPlaying) {
        return false;
    }

    m_previewPlaying = false;
    setState(EngineState::IDLE);
    LOG_INFO("Preview paused");
    return true;
}

bool VideoEngine::stopPreview() {
    if (!m_previewPlaying.exchange(false)) {
        return false;
    }

    if (m_previewThread && m_previewThread->joinable()) {
        m_previewThread->join();
    }

    m_previewPosition = 0;
    setState(EngineState::IDLE);
    LOG_INFO("Preview stopped");
    return true;
}

bool VideoEngine::seekPreview(int64_t timeMs) {
    if (timeMs < 0 || timeMs > getTimelineDuration()) {
        return false;
    }

    m_previewPosition = timeMs;
    LOG_DEBUG("Preview seeked to %lld ms", timeMs);
    return true;
}

bool VideoEngine::isPreviewPlaying() const {
    return m_previewPlaying.load();
}

int64_t VideoEngine::getPreviewPosition() const {
    return m_previewPosition.load();
}

std::vector<uint8_t> VideoEngine::getPreviewFrame(int64_t timeMs) {
    LOG_DEBUG("Requesting preview frame at %lld ms", timeMs);
    // Placeholder: return empty vector
    // Real implementation would decode frame from timeline
    return std::vector<uint8_t>();
}

bool VideoEngine::startExport(const std::string& outputPath, const std::string& format,
                             const std::string& quality) {
    if (m_exporting) {
        setError("Export already in progress");
        return false;
    }

    if (!m_timeline || !validateTimeline()) {
        setError("Invalid timeline for export");
        return false;
    }

    m_exporting = true;
    m_cancelExport = false;
    m_exportOutputPath = outputPath;

    m_exportThread = std::make_unique<std::thread>(
        &VideoEngine::exportRenderingThread, this, outputPath, format, quality);

    setState(EngineState::RENDERING);
    LOG_INFO("Export started: %s (format: %s, quality: %s)",
            outputPath.c_str(), format.c_str(), quality.c_str());
    return true;
}

bool VideoEngine::cancelExport() {
    if (!m_exporting) {
        return false;
    }

    m_cancelExport = true;
    LOG_INFO("Export cancellation requested");
    return true;
}

RenderProgress VideoEngine::getExportProgress() const {
    std::lock_guard<std::mutex> lock(m_exportMutex);
    return m_exportProgress;
}

bool VideoEngine::isExporting() const {
    return m_exporting.load();
}

std::string VideoEngine::getExportOutputPath() const {
    return m_exportOutputPath;
}

bool VideoEngine::saveProject(const std::string& projectPath) {
    LOG_INFO("Saving project to: %s", projectPath.c_str());
    // Placeholder: implement project serialization
    return true;
}

bool VideoEngine::loadProject(const std::string& projectPath) {
    LOG_INFO("Loading project from: %s", projectPath.c_str());
    // Placeholder: implement project deserialization
    return true;
}

bool VideoEngine::hasUnsavedChanges() const {
    if (!m_timeline) return false;
    return m_timeline->hasChanges();
}

std::string VideoEngine::getChangesSummary() const {
    if (!m_timeline) return "No changes";

    std::string summary;
    summary += "Clips: " + std::to_string(m_timeline->getClipCount()) + "\n";
    summary += "Duration: " + std::to_string(m_timeline->getTotalDuration()) + " ms\n";

    return summary;
}

size_t VideoEngine::getMemoryUsage() const {
    // Placeholder: implement memory profiling
    return 0;
}

size_t VideoEngine::getClipCount() const {
    if (!m_timeline) return 0;
    return m_timeline->getClipCount();
}

size_t VideoEngine::getEffectCount() const {
    if (!m_timeline) return 0;

    size_t count = 0;
    for (const auto& clip : m_timeline->getAllClips()) {
        if (clip) {
            count += clip->getEffectCount();
        }
    }
    return count;
}

std::string VideoEngine::getVersion() {
    return "1.0.0";
}

void VideoEngine::previewPlaybackThread() {
    LOG_DEBUG("Preview playback thread started");

    while (m_previewPlaying) {
        // Simulate playback
        std::this_thread::sleep_for(std::chrono::milliseconds(16));  // ~60 FPS
        m_previewPosition += 16;

        if (m_previewPosition > m_timeline->getTotalDuration()) {
            m_previewPosition = m_timeline->getTotalDuration();
            m_previewPlaying = false;
            setState(EngineState::IDLE);
            LOG_DEBUG("Preview playback completed");
            break;
        }
    }
}

void VideoEngine::exportRenderingThread(const std::string& outputPath,
                                       const std::string& format,
                                       const std::string& quality) {
    // Note: outputPath, format, quality are used for future implementation
    (void)outputPath;
    (void)format;
    (void)quality;

    LOG_DEBUG("Export rendering thread started");

    // Simulate rendering progress
    {
        std::lock_guard<std::mutex> lock(m_exportMutex);
        m_exportProgress.totalFrames = m_timeline->getTotalDuration() / 33;  // ~30 fps
        m_exportProgress.status = "Starting export...";
    }

    // Simulate frame-by-frame export
    for (int64_t i = 0; i < m_timeline->getTotalDuration() && m_exporting; i += 33) {
        if (m_cancelExport) {
            std::lock_guard<std::mutex> lock(m_exportMutex);
            m_exportProgress.status = "Export cancelled";
            break;
        }

        std::lock_guard<std::mutex> lock(m_exportMutex);
        m_exportProgress.currentFrame = i / 33;
        m_exportProgress.percentage = static_cast<float>(static_cast<double>(m_exportProgress.currentFrame) / static_cast<double>(m_exportProgress.totalFrames) * 100.0);
        m_exportProgress.status = "Rendering...";

        std::this_thread::sleep_for(std::chrono::milliseconds(10));  // Simulate work
    }

    m_exporting = false;
    setState(EngineState::IDLE);
    LOG_INFO("Export rendering thread completed");
}

void VideoEngine::setState(EngineState state) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_state = state;
    LOG_DEBUG("Engine state changed to %d", static_cast<int>(state));
}

void VideoEngine::setError(const std::string& message) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_state = EngineState::ERROR;
    m_errorMessage = message;
    LOG_ERROR("Engine error: %s", message.c_str());
}

bool VideoEngine::validateTimeline() const {
    if (!m_timeline) return false;
    return m_timeline->isValid();
}

bool VideoEngine::loadEffectsLibrary() {
    LOG_INFO("Loading effects library");
    // Placeholder: load effects
    return true;
}

} // namespace core
} // namespace clipforge
