package com.ucworks.clipforge.utils

import timber.log.Timber
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

/**
 * String and Time Extension Functions
 * Provides formatting and utility functions as extension methods
 */

// Time Formatting Extensions
fun Long.formatTime(): String {
    val totalSeconds = (this / 1000).coerceAtLeast(0)
    val minutes = totalSeconds / 60
    val seconds = totalSeconds % 60
    return String.format("%02d:%02d", minutes, seconds)
}

fun Long.formatTimeWithHours(): String {
    val totalSeconds = (this / 1000).coerceAtLeast(0)
    val hours = totalSeconds / 3600
    val minutes = (totalSeconds % 3600) / 60
    val seconds = totalSeconds % 60
    return if (hours > 0) {
        String.format("%02d:%02d:%02d", hours, minutes, seconds)
    } else {
        String.format("%02d:%02d", minutes, seconds)
    }
}

fun Long.formatDuration(): String {
    val totalSeconds = (this / 1000).coerceAtLeast(0)
    return when {
        totalSeconds < 60 -> "${totalSeconds}s"
        totalSeconds < 3600 -> {
            val minutes = totalSeconds / 60
            val seconds = totalSeconds % 60
            "${minutes}m ${seconds}s"
        }
        else -> {
            val hours = totalSeconds / 3600
            val minutes = (totalSeconds % 3600) / 60
            "${hours}h ${minutes}m"
        }
    }
}

// File Size Formatting Extensions
fun Long.formatSize(): String {
    return when {
        this >= 1024 * 1024 * 1024 -> String.format("%.2f GB", this / (1024.0 * 1024.0 * 1024.0))
        this >= 1024 * 1024 -> String.format("%.2f MB", this / (1024.0 * 1024.0))
        this >= 1024 -> String.format("%.2f KB", this / 1024.0)
        else -> "$this B"
    }
}

fun Int.formatSize(): String = toLong().formatSize()

// Date Formatting Extensions
fun Long.formatDate(format: String = "MMM dd, yyyy"): String {
    return try {
        val sdf = SimpleDateFormat(format, Locale.getDefault())
        sdf.format(Date(this))
    } catch (e: Exception) {
        Timber.e(e, "Error formatting date")
        "Unknown"
    }
}

fun Long.formatDateTime(format: String = "MMM dd, yyyy HH:mm"): String {
    return try {
        val sdf = SimpleDateFormat(format, Locale.getDefault())
        sdf.format(Date(this))
    } catch (e: Exception) {
        Timber.e(e, "Error formatting datetime")
        "Unknown"
    }
}

fun Long.timeAgo(): String {
    val diff = System.currentTimeMillis() - this
    return when {
        diff < 60000 -> "just now"
        diff < 3600000 -> "${diff / 60000}m ago"
        diff < 86400000 -> "${diff / 3600000}h ago"
        diff < 604800000 -> "${diff / 86400000}d ago"
        diff < 2592000000 -> "${diff / 604800000}w ago"
        else -> "${diff / 2592000000}mo ago"
    }
}

// String Utilities
fun String.isValidFileName(): Boolean {
    val invalidChars = listOf('/', '\\', ':', '*', '?', '"', '<', '>', '|')
    return !invalidChars.any { this.contains(it) } && this.isNotEmpty()
}

fun String.sanitizeFileName(): String {
    val invalidChars = listOf('/', '\\', ':', '*', '?', '"', '<', '>', '|')
    var result = this
    invalidChars.forEach { char ->
        result = result.replace(char.toString(), "_")
    }
    return result.take(255)  // Max filename length
}

fun String.getFileExtension(): String {
    return if (contains(".")) {
        substring(lastIndexOf(".") + 1).lowercase()
    } else {
        ""
    }
}

fun String.getFileName(): String {
    return if (contains("/")) {
        substring(lastIndexOf("/") + 1)
    } else {
        this
    }
}

fun String.removeFileExtension(): String {
    return if (contains(".")) {
        substring(0, lastIndexOf("."))
    } else {
        this
    }
}

// Parsing Extensions
fun String.toMilliseconds(): Long {
    return try {
        val parts = split(":")
        when {
            parts.size == 2 -> {
                val minutes = parts[0].toLong()
                val seconds = parts[1].toLong()
                (minutes * 60 + seconds) * 1000
            }
            parts.size == 3 -> {
                val hours = parts[0].toLong()
                val minutes = parts[1].toLong()
                val seconds = parts[2].toLong()
                (hours * 3600 + minutes * 60 + seconds) * 1000
            }
            else -> 0L
        }
    } catch (e: Exception) {
        Timber.e(e, "Error parsing time string: $this")
        0L
    }
}

fun String.parseFileSize(): Long {
    return try {
        val value = filter { it.isDigit() || it == '.' }.toDoubleOrNull() ?: return 0L
        when {
            uppercase().endsWith("GB") -> (value * 1024 * 1024 * 1024).toLong()
            uppercase().endsWith("MB") -> (value * 1024 * 1024).toLong()
            uppercase().endsWith("KB") -> (value * 1024).toLong()
            uppercase().endsWith("B") -> value.toLong()
            else -> 0L
        }
    } catch (e: Exception) {
        Timber.e(e, "Error parsing file size: $this")
        0L
    }
}

// Validation Extensions
fun String.isValidUrl(): Boolean {
    return try {
        java.net.URL(this)
        true
    } catch (e: Exception) {
        false
    }
}

fun String.isValidEmail(): Boolean {
    val emailPattern = "^[A-Za-z0-9+_.-]+@(.+)\$"
    return matches(emailPattern.toRegex())
}

fun String.isNumeric(): Boolean {
    return matches("\\d+".toRegex())
}

fun String.isAlphanumeric(): Boolean {
    return matches("[a-zA-Z0-9]+".toRegex())
}

// Text Processing Extensions
fun String.capitalize(): String {
    return replaceFirstChar { if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString() }
}

fun String.truncate(maxLength: Int, suffix: String = "..."): String {
    return if (length > maxLength) {
        substring(0, (maxLength - suffix.length).coerceAtLeast(0)) + suffix
    } else {
        this
    }
}

fun String.camelToSnakeCase(): String {
    return replace(Regex("([a-z])([A-Z])"), "$1_$2").lowercase()
}

fun String.snakeToCamelCase(): String {
    return split("_").mapIndexed { index, part ->
        if (index == 0) part else part.capitalize()
    }.joinToString("")
}

// Codec/Format Extensions
fun String.isVideoMimeType(): Boolean {
    return startsWith("video/")
}

fun String.isAudioMimeType(): Boolean {
    return startsWith("audio/")
}

fun String.isImageMimeType(): Boolean {
    return startsWith("image/")
}

fun String.getMimeTypeFriendlyName(): String {
    return when {
        equals("video/mp4") -> "MP4 Video"
        equals("video/webm") -> "WebM Video"
        equals("audio/mpeg") -> "MP3 Audio"
        equals("audio/wav") -> "WAV Audio"
        equals("image/jpeg") -> "JPEG Image"
        equals("image/png") -> "PNG Image"
        else -> this
    }
}

// Format Conversion
fun String.spaceToUnderscore(): String = replace(" ", "_")

fun String.underscoreToSpace(): String = replace("_", " ")

fun String.toPascalCase(): String {
    return split(" ", "_", "-").joinToString("") { word ->
        word.capitalize()
    }
}

// Safe Parsing
fun String?.toIntOrZero(): Int = this?.toIntOrNull() ?: 0

fun String?.toLongOrZero(): Long = this?.toLongOrNull() ?: 0L

fun String?.toFloatOrZero(): Float = this?.toFloatOrNull() ?: 0f

fun String?.toDoubleOrZero(): Double = this?.toDoubleOrNull() ?: 0.0

// Null/Empty Checks
fun String?.isNotBlank(): Boolean = !this.isNullOrBlank()

fun String?.orEmpty(): String = this ?: ""

fun String?.orDefault(default: String): String = this ?: default
