package com.ucworks.clipforge.models

import java.io.Serializable

/**
 * Represents a complete video editing project.
 */
data class Project(
    val id: String,
    val name: String,
    val description: String = "",
    val createdAt: Long = System.currentTimeMillis(),
    val modifiedAt: Long = System.currentTimeMillis(),
    val thumbnailPath: String? = null,
    val projectFilePath: String? = null,
    val videoDurationMs: Long = 0,
    val videoWidth: Int = 1920,
    val videoHeight: Int = 1080,
    val videoFrameRate: Int = 30,
    val videoCodec: String = "h264",
    val audioSampleRate: Int = 48000,
    val audioChannels: Int = 2,
    val videoClips: List<VideoClip> = emptyList(),
    val audioTracks: List<AudioTrack> = emptyList(),
    val metadata: ProjectMetadata = ProjectMetadata()
) : Serializable {

    val formattedDuration: String
        get() {
            val totalSeconds = videoDurationMs / 1000
            val minutes = totalSeconds / 60
            val seconds = totalSeconds % 60
            return String.format("%02d:%02d", minutes, seconds)
        }

    val resolution: String
        get() = "${videoWidth}x${videoHeight}@${videoFrameRate}fps"

    val hasUnsavedChanges: Boolean
        get() = metadata.hasUnsavedChanges

    val clipCount: Int
        get() = videoClips.size

    val audioTrackCount: Int
        get() = audioTracks.size

    val totalFileSize: Long
        get() = videoClips.sumOf { it.originalDurationMs } * 1000  // Rough estimate
}

/**
 * Project metadata and settings.
 */
data class ProjectMetadata(
    val hasUnsavedChanges: Boolean = false,
    val isDraft: Boolean = false,
    val tags: List<String> = emptyList(),
    val exportSettings: ExportSettings = ExportSettings(),
    val lastSavedAt: Long = System.currentTimeMillis(),
    val version: Int = 1
) : Serializable

/**
 * Export/rendering configuration for a project.
 */
data class ExportSettings(
    val format: String = "mp4",  // mp4, webm, mov, mkv
    val quality: String = "high",  // low, medium, high, ultra
    val videoCodec: String = "h264",  // h264, h265, vp9, av1
    val audioCodec: String = "aac",  // aac, mp3, opus, flac
    val bitrate: String = "20mbps",
    val preset: String = "balanced",  // ultrafast, fast, balanced, slow, placebo
    val outputPath: String = "",
    val includeAudio: Boolean = true,
    val includeSubtitles: Boolean = false,
    val watermark: String? = null,
    val watermarkOpacity: Float = 1.0f
) : Serializable

/**
 * Project recent item (for recent projects list).
 */
data class RecentProject(
    val id: String,
    val name: String,
    val thumbnailPath: String?,
    val duration: String,
    val resolution: String,
    val lastModified: Long,
    val clipCount: Int
) : Serializable {
    companion object {
        fun fromProject(project: Project): RecentProject {
            return RecentProject(
                id = project.id,
                name = project.name,
                thumbnailPath = project.thumbnailPath,
                duration = project.formattedDuration,
                resolution = project.resolution,
                lastModified = project.modifiedAt,
                clipCount = project.videoClips.size
            )
        }
    }
}

/**
 * Project creation configuration.
 */
data class ProjectConfig(
    val name: String,
    val description: String = "",
    val width: Int = 1920,
    val height: Int = 1080,
    val frameRate: Int = 30,
    val audioSampleRate: Int = 48000,
    val audioChannels: Int = 2
) {
    val resolution: String
        get() = "${width}x${height}@${frameRate}fps"
}

/**
 * Project templates for quick creation.
 */
object ProjectTemplates {
    val templates = listOf(
        ProjectConfig(
            name = "1080p 30fps",
            width = 1920,
            height = 1080,
            frameRate = 30,
            audioSampleRate = 48000
        ),
        ProjectConfig(
            name = "720p 30fps",
            width = 1280,
            height = 720,
            frameRate = 30
        ),
        ProjectConfig(
            name = "1080p 60fps",
            width = 1920,
            height = 1080,
            frameRate = 60
        ),
        ProjectConfig(
            name = "4K 30fps",
            width = 3840,
            height = 2160,
            frameRate = 30
        ),
        ProjectConfig(
            name = "Instagram Story",
            width = 1080,
            height = 1920,
            frameRate = 30
        ),
        ProjectConfig(
            name = "TikTok",
            width = 1080,
            height = 1920,
            frameRate = 30
        ),
        ProjectConfig(
            name = "YouTube Thumbnail",
            width = 1280,
            height = 720,
            frameRate = 30
        )
    )
}
