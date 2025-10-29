package com.ucworks.clipforge.models

import android.net.Uri

/**
 * Represents a media file (video or audio) available for import.
 */
data class MediaItem(
    val id: String,
    val displayName: String,
    val uri: Uri,
    val duration: Long,  // milliseconds
    val size: Long,      // bytes
    val mimeType: String,
    val dateAdded: Long, // unix timestamp
    val thumbnail: Uri? = null,
    val isVideo: Boolean
) {
    val formattedDuration: String
        get() {
            val totalSeconds = duration / 1000
            val minutes = totalSeconds / 60
            val seconds = totalSeconds % 60
            return String.format("%02d:%02d", minutes, seconds)
        }

    val formattedSize: String
        get() {
            return when {
                size >= 1024 * 1024 * 1024 -> String.format("%.2f GB", size / (1024.0 * 1024.0 * 1024.0))
                size >= 1024 * 1024 -> String.format("%.2f MB", size / (1024.0 * 1024.0))
                size >= 1024 -> String.format("%.2f KB", size / 1024.0)
                else -> "$size B"
            }
        }
}
