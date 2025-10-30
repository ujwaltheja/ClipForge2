#ifndef CLIPFORGE_PROJECT_STORAGE_H
#define CLIPFORGE_PROJECT_STORAGE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <map>
#include <chrono>

namespace clipforge {
namespace storage {

/**
 * @struct ProjectMetadata
 * @brief Metadata for a video editing project
 */
struct ProjectMetadata {
    std::string projectId;              // Unique project identifier
    std::string projectName;            // Human-readable project name
    std::string description;            // Project description
    int64_t createdAtMs;                // Creation timestamp
    int64_t modifiedAtMs;               // Last modification timestamp
    int64_t durationMs;                 // Project duration in milliseconds
    int32_t width = 1920;               // Video width in pixels
    int32_t height = 1080;              // Video height in pixels
    float frameRate = 30.0f;            // Frames per second
    std::string thumbnailPath;          // Path to project thumbnail
    int64_t fileSizeBytes = 0;          // Total file size
    int trackCount = 1;                 // Number of video tracks
    int audioTrackCount = 1;            // Number of audio tracks
    std::string lastExportPath;         // Path to last export
};

/**
 * @struct ClipData
 * @brief Serializable clip data
 */
struct ClipData {
    std::string clipId;
    std::string mediaPath;
    int64_t startTimeMs;
    int64_t durationMs;
    int64_t trimStartMs = 0;
    int64_t trimEndMs;
    int trackIndex = 0;
    float opacity = 1.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float rotationDegrees = 0.0f;
    std::vector<std::string> appliedEffects;  // Effect IDs
};

/**
 * @struct AudioTrackData
 * @brief Serializable audio track data
 */
struct AudioTrackData {
    std::string trackId;
    std::string trackName;
    std::string trackType;  // "main", "voiceover", "music", "sfx"
    float volume = 1.0f;
    bool muted = false;
    bool soloed = false;
    std::vector<std::pair<int64_t, std::string>> audioClips;  // timeMs, filePath
};

/**
 * @struct EffectData
 * @brief Serializable effect data
 */
struct EffectData {
    std::string effectId;
    int effectType;         // enum value
    int64_t startTimeMs;
    int64_t durationMs;
    std::string parametersJson;  // JSON encoded parameters
    bool enabled = true;
};

/**
 * @class ProjectStorage
 * @brief Manages local project persistence
 *
 * Saves and loads projects to/from local device storage in binary format.
 * Projects are stored in app's private directory for security.
 * Format: Binary protobuf-like structure for efficient storage.
 */
class ProjectStorage {
public:
    /**
     * @brief Create project storage
     * @param baseStoragePath Base path for project storage (typically app's cache directory)
     */
    explicit ProjectStorage(const std::string& baseStoragePath);

    /**
     * @brief Initialize storage (create directories if needed)
     * @return true if initialization successful
     */
    bool initialize();

    /**
     * @brief Save project to disk
     * @param metadata Project metadata
     * @param clips Video clips
     * @param audioTracks Audio tracks
     * @param effects Applied effects
     * @return true if saved successfully
     */
    bool saveProject(const ProjectMetadata& metadata,
                    const std::vector<ClipData>& clips,
                    const std::vector<AudioTrackData>& audioTracks,
                    const std::vector<EffectData>& effects);

    /**
     * @brief Load project from disk
     * @param projectId Project ID to load
     * @param outMetadata Loaded metadata
     * @param outClips Loaded clips
     * @param outAudioTracks Loaded audio tracks
     * @param outEffects Loaded effects
     * @return true if loaded successfully
     */
    bool loadProject(const std::string& projectId,
                    ProjectMetadata& outMetadata,
                    std::vector<ClipData>& outClips,
                    std::vector<AudioTrackData>& outAudioTracks,
                    std::vector<EffectData>& outEffects);

    /**
     * @brief List all projects
     * @return Vector of project metadata for all saved projects
     */
    [[nodiscard]] std::vector<ProjectMetadata> listProjects();

    /**
     * @brief Get project metadata only (faster than full load)
     * @param projectId Project ID
     * @return Project metadata or empty if not found
     */
    [[nodiscard]] ProjectMetadata getProjectMetadata(const std::string& projectId);

    /**
     * @brief Delete project and all associated files
     * @param projectId Project ID to delete
     * @return true if deleted successfully
     */
    bool deleteProject(const std::string& projectId);

    /**
     * @brief Create backup of project
     * @param projectId Project ID
     * @param backupPath Path to save backup
     * @return true if backup created successfully
     */
    bool createBackup(const std::string& projectId, const std::string& backupPath);

    /**
     * @brief Restore project from backup
     * @param backupPath Path to backup file
     * @param newProjectId ID for restored project
     * @return true if restored successfully
     */
    bool restoreFromBackup(const std::string& backupPath, const std::string& newProjectId);

    /**
     * @brief Get project file size
     * @param projectId Project ID
     * @return File size in bytes
     */
    [[nodiscard]] int64_t getProjectSize(const std::string& projectId);

    /**
     * @brief Check if project exists
     * @param projectId Project ID
     * @return true if project exists
     */
    [[nodiscard]] bool projectExists(const std::string& projectId) const;

    /**
     * @brief Get storage quota information
     * @param outTotalBytes Total storage available
     * @param outUsedBytes Storage used by projects
     * @return true if successful
     */
    bool getStorageInfo(int64_t& outTotalBytes, int64_t& outUsedBytes);

    /**
     * @brief Get project directory path
     * @param projectId Project ID
     * @return Full path to project directory
     */
    [[nodiscard]] std::string getProjectPath(const std::string& projectId) const;

private:
    std::string m_baseStoragePath;
    std::string m_projectsDir;
    std::string m_backupsDir;
    std::string m_mediaCacheDir;

    // Project file format version
    static constexpr uint32_t FILE_FORMAT_VERSION = 1;
    static constexpr const char* METADATA_FILENAME = "project.meta";
    static constexpr const char* PROJECT_FILENAME = "project.bin";

    // Helper functions
    [[nodiscard]] std::string generateProjectId() const;
    [[nodiscard]] std::string getProjectMetadataPath(const std::string& projectId) const;
    [[nodiscard]] std::string getProjectDataPath(const std::string& projectId) const;

    bool ensureDirectoryExists(const std::string& path);
};

/**
 * @class ProjectCache
 * @brief In-memory cache for recently opened projects
 *
 * Reduces file I/O by caching project data in memory
 */
class ProjectCache {
public:
    /**
     * @brief Create project cache with size limit
     * @param maxProjects Maximum number of projects to cache
     */
    explicit ProjectCache(size_t maxProjects = 5);

    /**
     * @brief Add project to cache
     * @param metadata Project metadata
     * @param clips Video clips
     * @param audioTracks Audio tracks
     * @param effects Effects
     */
    void cacheProject(const ProjectMetadata& metadata,
                     const std::vector<ClipData>& clips,
                     const std::vector<AudioTrackData>& audioTracks,
                     const std::vector<EffectData>& effects);

    /**
     * @brief Retrieve project from cache
     * @param projectId Project ID
     * @param outMetadata Cached metadata
     * @param outClips Cached clips
     * @param outAudioTracks Cached audio tracks
     * @param outEffects Cached effects
     * @return true if project found in cache
     */
    bool getFromCache(const std::string& projectId,
                     ProjectMetadata& outMetadata,
                     std::vector<ClipData>& outClips,
                     std::vector<AudioTrackData>& outAudioTracks,
                     std::vector<EffectData>& outEffects);

    /**
     * @brief Remove project from cache
     * @param projectId Project ID
     */
    void removeFromCache(const std::string& projectId);

    /**
     * @brief Clear entire cache
     */
    void clear();

    /**
     * @brief Check if project is cached
     * @param projectId Project ID
     * @return true if cached
     */
    [[nodiscard]] bool isCached(const std::string& projectId) const;

private:
    size_t m_maxProjects;
    std::map<std::string, std::pair<ProjectMetadata,
             std::tuple<std::vector<ClipData>,
                       std::vector<AudioTrackData>,
                       std::vector<EffectData>>>> m_cache;
};

/**
 * @class ProjectManager
 * @brief High-level project management
 *
 * Combines storage and caching for efficient project access
 */
class ProjectManager {
public:
    /**
     * @brief Create project manager
     * @param baseStoragePath Base storage path
     */
    explicit ProjectManager(const std::string& baseStoragePath);

    /**
     * @brief Create new project
     * @param projectName Project name
     * @param width Video width
     * @param height Video height
     * @param frameRate Frames per second
     * @return Project ID or empty string if failed
     */
    std::string createProject(const std::string& projectName,
                             int32_t width = 1920, int32_t height = 1080,
                             float frameRate = 30.0f);

    /**
     * @brief Open existing project
     * @param projectId Project ID
     * @param outMetadata Loaded metadata
     * @param outClips Loaded clips
     * @param outAudioTracks Loaded audio tracks
     * @param outEffects Loaded effects
     * @return true if opened successfully
     */
    bool openProject(const std::string& projectId,
                    ProjectMetadata& outMetadata,
                    std::vector<ClipData>& outClips,
                    std::vector<AudioTrackData>& outAudioTracks,
                    std::vector<EffectData>& outEffects);

    /**
     * @brief Save current project
     * @param metadata Updated metadata
     * @param clips Updated clips
     * @param audioTracks Updated audio tracks
     * @param effects Updated effects
     * @return true if saved successfully
     */
    bool saveCurrentProject(const ProjectMetadata& metadata,
                           const std::vector<ClipData>& clips,
                           const std::vector<AudioTrackData>& audioTracks,
                           const std::vector<EffectData>& effects);

    /**
     * @brief Get recent projects
     * @param count Number of recent projects
     * @return Vector of recent project metadata
     */
    [[nodiscard]] std::vector<ProjectMetadata> getRecentProjects(int count = 10);

    /**
     * @brief Delete project
     * @param projectId Project ID
     * @return true if deleted
     */
    bool deleteProject(const std::string& projectId);

private:
    std::unique_ptr<ProjectStorage> m_storage;
    ProjectCache m_cache;
    std::string m_currentProjectId;
};

} // namespace storage
} // namespace clipforge

#endif // CLIPFORGE_PROJECT_STORAGE_H
