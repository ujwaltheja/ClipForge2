package com.ucworks.clipforge.data.repository

import android.content.Context
import android.media.MediaMetadataRetriever
import android.net.Uri
import android.provider.MediaStore
import android.util.Log
import kotlinx.coroutines.*
import java.io.File

/**
 * Media repository for importing and managing media files
 * Handles video, audio, and image imports from device storage
 */
class MediaRepository(private val context: Context) {

    companion object {
        private const val TAG = "MediaRepository"

        // Supported video formats
        val SUPPORTED_VIDEO_FORMATS = listOf(
            "mp4", "mkv", "avi", "mov", "flv", "webm",
            "3gp", "m4v", "mpg", "mpeg", "ts"
        )

        // Supported audio formats
        val SUPPORTED_AUDIO_FORMATS = listOf(
            "mp3", "aac", "flac", "opus", "ogg", "m4a",
            "wav", "aiff", "wma"
        )

        // Supported image formats
        val SUPPORTED_IMAGE_FORMATS = listOf(
            "jpg", "jpeg", "png", "webp", "gif", "bmp",
            "tiff", "heic", "raw"
        )
    }

    data class MediaInfo(
        val uri: Uri,
        val fileName: String,
        val filePath: String,
        val fileSize: Long,
        val mediaType: MediaType,
        val duration: Long = 0,           // ms for video/audio
        val width: Int = 0,               // pixels for video/image
        val height: Int = 0,              // pixels for video/image
        val frameRate: Float = 30f,       // fps for video
        val bitrate: Long = 0,            // bps for video/audio
        val audioChannels: Int = 2,       // for audio
        val sampleRate: Int = 44100,      // Hz for audio
        val codec: String = ""
    )

    enum class MediaType {
        VIDEO, AUDIO, IMAGE, UNKNOWN
    }

    private val scope = CoroutineScope(Dispatchers.IO + Job())

    /**
     * Import video file
     * @param uri URI to video file
     * @param callback Called when import completes
     */
    fun importVideo(uri: Uri, callback: (Result<MediaInfo>) -> Unit) {
        scope.launch {
            try {
                val mediaInfo = getMediaInfo(uri, MediaType.VIDEO)
                if (mediaInfo.mediaType == MediaType.VIDEO) {
                    callback(Result.success(mediaInfo))
                } else {
                    callback(Result.failure(Exception("Invalid video file")))
                }
            } catch (e: Exception) {
                Log.e(TAG, "Failed to import video: ${e.message}")
                callback(Result.failure(e))
            }
        }
    }

    /**
     * Import audio file
     * @param uri URI to audio file
     * @param callback Called when import completes
     */
    fun importAudio(uri: Uri, callback: (Result<MediaInfo>) -> Unit) {
        scope.launch {
            try {
                val mediaInfo = getMediaInfo(uri, MediaType.AUDIO)
                if (mediaInfo.mediaType == MediaType.AUDIO) {
                    callback(Result.success(mediaInfo))
                } else {
                    callback(Result.failure(Exception("Invalid audio file")))
                }
            } catch (e: Exception) {
                Log.e(TAG, "Failed to import audio: ${e.message}")
                callback(Result.failure(e))
            }
        }
    }

    /**
     * Import image file
     * @param uri URI to image file
     * @param callback Called when import completes
     */
    fun importImage(uri: Uri, callback: (Result<MediaInfo>) -> Unit) {
        scope.launch {
            try {
                val mediaInfo = getMediaInfo(uri, MediaType.IMAGE)
                if (mediaInfo.mediaType == MediaType.IMAGE) {
                    callback(Result.success(mediaInfo))
                } else {
                    callback(Result.failure(Exception("Invalid image file")))
                }
            } catch (e: Exception) {
                Log.e(TAG, "Failed to import image: ${e.message}")
                callback(Result.failure(e))
            }
        }
    }

    /**
     * Get all videos on device
     * @return List of video media info
     */
    suspend fun getAllVideos(): List<MediaInfo> = withContext(Dispatchers.IO) {
        val videos = mutableListOf<MediaInfo>()
        val projection = arrayOf(
            MediaStore.Video.Media._ID,
            MediaStore.Video.Media.DISPLAY_NAME,
            MediaStore.Video.Media.DATA,
            MediaStore.Video.Media.SIZE,
            MediaStore.Video.Media.DURATION,
            MediaStore.Video.Media.WIDTH,
            MediaStore.Video.Media.HEIGHT
        )

        val cursor = context.contentResolver.query(
            MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
            projection,
            null,
            null,
            "${MediaStore.Video.Media.DATE_MODIFIED} DESC"
        ) ?: return@withContext videos

        cursor.use {
            val idColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media._ID)
            val nameColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME)
            val dataColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.DATA)
            val sizeColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE)
            val durationColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION)
            val widthColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.WIDTH)
            val heightColumn = it.getColumnIndexOrThrow(MediaStore.Video.Media.HEIGHT)

            while (it.moveToNext()) {
                val id = it.getLong(idColumn)
                val name = it.getString(nameColumn)
                val data = it.getString(dataColumn)
                val size = it.getLong(sizeColumn)
                val duration = it.getLong(durationColumn)
                val width = it.getInt(widthColumn)
                val height = it.getInt(heightColumn)

                val uri = Uri.withAppendedPath(MediaStore.Video.Media.EXTERNAL_CONTENT_URI, id.toString())

                videos.add(MediaInfo(
                    uri = uri,
                    fileName = name,
                    filePath = data,
                    fileSize = size,
                    mediaType = MediaType.VIDEO,
                    duration = duration,
                    width = width,
                    height = height
                ))
            }
        }

        return@withContext videos
    }

    /**
     * Get all audio files on device
     * @return List of audio media info
     */
    suspend fun getAllAudio(): List<MediaInfo> = withContext(Dispatchers.IO) {
        val audioFiles = mutableListOf<MediaInfo>()
        val projection = arrayOf(
            MediaStore.Audio.Media._ID,
            MediaStore.Audio.Media.DISPLAY_NAME,
            MediaStore.Audio.Media.DATA,
            MediaStore.Audio.Media.SIZE,
            MediaStore.Audio.Media.DURATION,
            MediaStore.Audio.Media.ARTIST
        )

        val cursor = context.contentResolver.query(
            MediaStore.Audio.Media.EXTERNAL_CONTENT_URI,
            projection,
            null,
            null,
            "${MediaStore.Audio.Media.DATE_MODIFIED} DESC"
        ) ?: return@withContext audioFiles

        cursor.use {
            val idColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media._ID)
            val nameColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.DISPLAY_NAME)
            val dataColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.DATA)
            val sizeColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.SIZE)
            val durationColumn = it.getColumnIndexOrThrow(MediaStore.Audio.Media.DURATION)

            while (it.moveToNext()) {
                val id = it.getLong(idColumn)
                val name = it.getString(nameColumn)
                val data = it.getString(dataColumn)
                val size = it.getLong(sizeColumn)
                val duration = it.getLong(durationColumn)

                val uri = Uri.withAppendedPath(MediaStore.Audio.Media.EXTERNAL_CONTENT_URI, id.toString())

                audioFiles.add(MediaInfo(
                    uri = uri,
                    fileName = name,
                    filePath = data,
                    fileSize = size,
                    mediaType = MediaType.AUDIO,
                    duration = duration
                ))
            }
        }

        return@withContext audioFiles
    }

    /**
     * Get all images on device
     * @return List of image media info
     */
    suspend fun getAllImages(): List<MediaInfo> = withContext(Dispatchers.IO) {
        val images = mutableListOf<MediaInfo>()
        val projection = arrayOf(
            MediaStore.Images.Media._ID,
            MediaStore.Images.Media.DISPLAY_NAME,
            MediaStore.Images.Media.DATA,
            MediaStore.Images.Media.SIZE,
            MediaStore.Images.Media.WIDTH,
            MediaStore.Images.Media.HEIGHT
        )

        val cursor = context.contentResolver.query(
            MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
            projection,
            null,
            null,
            "${MediaStore.Images.Media.DATE_MODIFIED} DESC"
        ) ?: return@withContext images

        cursor.use {
            val idColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media._ID)
            val nameColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media.DISPLAY_NAME)
            val dataColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media.DATA)
            val sizeColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE)
            val widthColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media.WIDTH)
            val heightColumn = it.getColumnIndexOrThrow(MediaStore.Images.Media.HEIGHT)

            while (it.moveToNext()) {
                val id = it.getLong(idColumn)
                val name = it.getString(nameColumn)
                val data = it.getString(dataColumn)
                val size = it.getLong(sizeColumn)
                val width = it.getInt(widthColumn)
                val height = it.getInt(heightColumn)

                val uri = Uri.withAppendedPath(MediaStore.Images.Media.EXTERNAL_CONTENT_URI, id.toString())

                images.add(MediaInfo(
                    uri = uri,
                    fileName = name,
                    filePath = data,
                    fileSize = size,
                    mediaType = MediaType.IMAGE,
                    width = width,
                    height = height
                ))
            }
        }

        return@withContext images
    }

    /**
     * Get detailed media information
     * @param uri Media URI
     * @param expectedType Expected media type
     * @return MediaInfo with detailed information
     */
    private fun getMediaInfo(uri: Uri, expectedType: MediaType): MediaInfo {
        val retriever = MediaMetadataRetriever()
        return try {
            retriever.setDataSource(context, uri)

            val fileName = getFileName(uri)
            val filePath = getFilePath(uri) ?: fileName
            val fileSize = getFileSize(uri)
            val mediaType = detectMediaType(fileName)

            val duration = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_DURATION)?.toLongOrNull() ?: 0
            val width = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_WIDTH)?.toIntOrNull() ?: 0
            val height = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_HEIGHT)?.toIntOrNull() ?: 0
            val rotation = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_VIDEO_ROTATION)?.toIntOrNull() ?: 0
            val bitrate = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_BITRATE)?.toLongOrNull() ?: 0

            MediaInfo(
                uri = uri,
                fileName = fileName,
                filePath = filePath,
                fileSize = fileSize,
                mediaType = mediaType,
                duration = duration,
                width = width,
                height = height,
                bitrate = bitrate
            )
        } catch (e: Exception) {
            Log.e(TAG, "Error extracting media info: ${e.message}")
            throw e
        } finally {
            retriever.release()
        }
    }

    private fun detectMediaType(fileName: String): MediaType {
        val extension = fileName.substringAfterLast(".").lowercase()
        return when {
            extension in SUPPORTED_VIDEO_FORMATS -> MediaType.VIDEO
            extension in SUPPORTED_AUDIO_FORMATS -> MediaType.AUDIO
            extension in SUPPORTED_IMAGE_FORMATS -> MediaType.IMAGE
            else -> MediaType.UNKNOWN
        }
    }

    private fun getFileName(uri: Uri): String {
        return when (uri.scheme) {
            "content" -> {
                val cursor = context.contentResolver.query(uri, arrayOf(MediaStore.MediaColumns.DISPLAY_NAME), null, null, null)
                cursor?.use {
                    if (it.moveToFirst()) {
                        return@use it.getString(0)
                    }
                }
                uri.lastPathSegment ?: "unknown"
            }
            "file" -> File(uri.path ?: "").name
            else -> uri.lastPathSegment ?: "unknown"
        }
    }

    private fun getFilePath(uri: Uri): String? {
        return when (uri.scheme) {
            "content" -> {
                val cursor = context.contentResolver.query(uri, arrayOf(MediaStore.MediaColumns.DATA), null, null, null)
                cursor?.use {
                    if (it.moveToFirst()) {
                        return@use it.getString(0)
                    }
                }
                null
            }
            "file" -> uri.path
            else -> null
        }
    }

    private fun getFileSize(uri: Uri): Long {
        return try {
            context.contentResolver.openInputStream(uri)?.use { stream ->
                stream.available().toLong()
            } ?: 0
        } catch (e: Exception) {
            0
        }
    }

    fun cleanup() {
        scope.cancel()
    }
}
