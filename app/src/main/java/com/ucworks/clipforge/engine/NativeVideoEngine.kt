package com.ucworks.clipforge.engine

import android.content.Context
import android.util.Log
import kotlin.math.roundToInt

/**
 * JNI bridge to C++ video engine
 * All heavy video processing is done in native C++ for performance
 */
class NativeVideoEngine(private val context: Context) {

    companion object {
        private const val TAG = "NativeVideoEngine"

        init {
            try {
                System.loadLibrary("clipforge-native")
                Log.d(TAG, "Native library loaded successfully")
            } catch (e: UnsatisfiedLinkError) {
                Log.e(TAG, "Failed to load native library: ${e.message}")
            }
        }
    }

    // ===== Initialization =====

    /**
     * Initialize the native video engine
     * @param cacheDir Cache directory for temporary files
     * @return true if initialization successful
     */
    fun initialize(cacheDir: String): Boolean {
        return nativeInitialize(cacheDir)
    }

    /**
     * Cleanup and release resources
     */
    fun cleanup() {
        nativeCleanup()
    }

    // ===== Timeline Management =====

    /**
     * Create new timeline
     * @param width Video width in pixels
     * @param height Video height in pixels
     * @param fps Frames per second
     * @return Timeline ID
     */
    fun createTimeline(width: Int, height: Int, fps: Float): String {
        return nativeCreateTimeline(width, height, fps)
    }

    /**
     * Add clip to timeline
     * @param timelineId Timeline ID
     * @param mediaPath Path to media file
     * @param startTimeMs Start time in milliseconds
     * @param trackIndex Track index
     * @return Clip ID
     */
    fun addClip(timelineId: String, mediaPath: String, startTimeMs: Long, trackIndex: Int): String {
        return nativeAddClip(timelineId, mediaPath, startTimeMs, trackIndex)
    }

    /**
     * Remove clip from timeline
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @return true if removed
     */
    fun removeClip(timelineId: String, clipId: String): Boolean {
        return nativeRemoveClip(timelineId, clipId)
    }

    /**
     * Move clip to new position
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @param newStartTimeMs New start time
     * @return true if moved
     */
    fun moveClip(timelineId: String, clipId: String, newStartTimeMs: Long): Boolean {
        return nativeMoveClip(timelineId, clipId, newStartTimeMs)
    }

    /**
     * Trim clip
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @param trimStartMs Trim start
     * @param trimEndMs Trim end
     * @return true if trimmed
     */
    fun trimClip(timelineId: String, clipId: String, trimStartMs: Long, trimEndMs: Long): Boolean {
        return nativeTrimClip(timelineId, clipId, trimStartMs, trimEndMs)
    }

    /**
     * Split clip at time
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @param splitTimeMs Split time
     * @return ID of new clip
     */
    fun splitClip(timelineId: String, clipId: String, splitTimeMs: Long): String {
        return nativeSplitClip(timelineId, clipId, splitTimeMs)
    }

    // ===== Effects Processing =====

    /**
     * Apply effect to clip
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @param effectType Effect type
     * @param params Effect parameters as JSON string
     * @return Effect ID
     */
    fun applyEffect(timelineId: String, clipId: String, effectType: String, params: String): String {
        return nativeApplyEffect(timelineId, clipId, effectType, params)
    }

    /**
     * Update effect parameters
     * @param effectId Effect ID
     * @param params New parameters as JSON string
     * @return true if updated
     */
    fun updateEffect(effectId: String, params: String): Boolean {
        return nativeUpdateEffect(effectId, params)
    }

    /**
     * Remove effect
     * @param effectId Effect ID
     * @return true if removed
     */
    fun removeEffect(effectId: String): Boolean {
        return nativeRemoveEffect(effectId)
    }

    /**
     * Apply color grading
     * @param timelineId Timeline ID
     * @param clipId Clip ID
     * @param exposure Exposure adjustment (-2 to 2)
     * @param saturation Saturation (0 to 2)
     * @param contrast Contrast (0 to 2)
     * @return true if applied
     */
    fun applyColorGrade(timelineId: String, clipId: String, exposure: Float,
                       saturation: Float, contrast: Float): Boolean {
        return nativeApplyColorGrade(timelineId, clipId, exposure, saturation, contrast)
    }

    // ===== Audio Processing =====

    /**
     * Set clip volume
     * @param clipId Clip ID
     * @param volume Volume (0 to 1)
     * @return true if set
     */
    fun setClipVolume(clipId: String, volume: Float): Boolean {
        return nativeSetClipVolume(clipId, volume.coerceIn(0f, 1f))
    }

    /**
     * Apply audio effect
     * @param clipId Clip ID
     * @param effectType Effect type (eq, compressor, reverb, delay)
     * @param params Parameters as JSON string
     * @return Effect ID
     */
    fun applyAudioEffect(clipId: String, effectType: String, params: String): String {
        return nativeApplyAudioEffect(clipId, effectType, params)
    }

    /**
     * Add audio track to timeline
     * @param timelineId Timeline ID
     * @param trackName Track name
     * @param trackType Track type (main, voiceover, music, sfx)
     * @return Track ID
     */
    fun addAudioTrack(timelineId: String, trackName: String, trackType: String): String {
        return nativeAddAudioTrack(timelineId, trackName, trackType)
    }

    /**
     * Extract audio spectrum for visualization
     * @param clipId Clip ID
     * @param timeMs Current time
     * @return Array of spectrum values (0-127)
     */
    fun getAudioSpectrum(clipId: String, timeMs: Long): IntArray {
        return nativeGetAudioSpectrum(clipId, timeMs)
    }

    /**
     * Detect beats in audio
     * @param clipId Clip ID
     * @return Array of beat times in milliseconds
     */
    fun detectBeats(clipId: String): LongArray {
        return nativeDetectBeats(clipId)
    }

    // ===== Speed & Time Effects =====

    /**
     * Set clip playback speed
     * @param clipId Clip ID
     * @param speed Speed multiplier (0.25 to 4.0)
     * @return true if set
     */
    fun setClipSpeed(clipId: String, speed: Float): Boolean {
        val constrainedSpeed = speed.coerceIn(0.25f, 4.0f)
        return nativeSetClipSpeed(clipId, constrainedSpeed)
    }

    /**
     * Enable reverse playback
     * @param clipId Clip ID
     * @param reversed true for reverse
     * @return true if set
     */
    fun setClipReverse(clipId: String, reversed: Boolean): Boolean {
        return nativeSetClipReverse(clipId, reversed)
    }

    /**
     * Create speed ramp
     * @param clipId Clip ID
     * @param startTimeMs Start time
     * @param endTimeMs End time
     * @param startSpeed Start speed
     * @param endSpeed End speed
     * @return true if created
     */
    fun createSpeedRamp(clipId: String, startTimeMs: Long, endTimeMs: Long,
                       startSpeed: Float, endSpeed: Float): Boolean {
        return nativeCreateSpeedRamp(clipId, startTimeMs, endTimeMs, startSpeed, endSpeed)
    }

    // ===== Keyframe Animation =====

    /**
     * Add keyframe to property animation
     * @param clipId Clip ID
     * @param propertyName Property name (opacity, scale, rotation, etc.)
     * @param timeMs Keyframe time
     * @param value Keyframe value
     * @param interpolationType Interpolation type
     * @return true if added
     */
    fun addKeyframe(clipId: String, propertyName: String, timeMs: Long,
                   value: Float, interpolationType: String): Boolean {
        return nativeAddKeyframe(clipId, propertyName, timeMs, value, interpolationType)
    }

    /**
     * Remove keyframe
     * @param clipId Clip ID
     * @param propertyName Property name
     * @param timeMs Keyframe time
     * @return true if removed
     */
    fun removeKeyframe(clipId: String, propertyName: String, timeMs: Long): Boolean {
        return nativeRemoveKeyframe(clipId, propertyName, timeMs)
    }

    /**
     * Get animation value at time
     * @param clipId Clip ID
     * @param propertyName Property name
     * @param timeMs Current time
     * @return Interpolated value
     */
    fun getAnimationValue(clipId: String, propertyName: String, timeMs: Long): Float {
        return nativeGetAnimationValue(clipId, propertyName, timeMs)
    }

    // ===== Transitions =====

    /**
     * Add transition between clips
     * @param clipId1 First clip ID
     * @param clipId2 Second clip ID
     * @param transitionType Transition type
     * @param durationMs Duration in milliseconds
     * @return Transition ID
     */
    fun addTransition(clipId1: String, clipId2: String, transitionType: String,
                     durationMs: Int): String {
        return nativeAddTransition(clipId1, clipId2, transitionType, durationMs)
    }

    /**
     * Remove transition
     * @param transitionId Transition ID
     * @return true if removed
     */
    fun removeTransition(transitionId: String): Boolean {
        return nativeRemoveTransition(transitionId)
    }

    // ===== Preview & Rendering =====

    /**
     * Get preview frame at time
     * @param timelineId Timeline ID
     * @param timeMs Time in milliseconds
     * @return Frame bitmap data or null
     */
    fun getPreviewFrame(timelineId: String, timeMs: Long): ByteArray? {
        return nativeGetPreviewFrame(timelineId, timeMs)
    }

    /**
     * Get timeline duration
     * @param timelineId Timeline ID
     * @return Duration in milliseconds
     */
    fun getTimelineDuration(timelineId: String): Long {
        return nativeGetTimelineDuration(timelineId)
    }

    /**
     * Get timeline properties
     * @param timelineId Timeline ID
     * @return JSON string with properties
     */
    fun getTimelineProperties(timelineId: String): String {
        return nativeGetTimelineProperties(timelineId)
    }

    // ===== Export =====

    /**
     * Export timeline to video file
     * @param timelineId Timeline ID
     * @param outputPath Output file path
     * @param width Export width in pixels
     * @param height Export height in pixels
     * @param bitrate Bitrate in kbps
     * @param codec Video codec (h264, h265, vp9)
     * @param audioCodec Audio codec (aac, opus)
     * @param progressCallback Callback for progress (0-100)
     * @return true if export successful
     */
    fun exportTimeline(timelineId: String, outputPath: String, width: Int, height: Int,
                      bitrate: Int, codec: String, audioCodec: String,
                      progressCallback: (Int) -> Unit): Boolean {
        return nativeExportTimeline(timelineId, outputPath, width, height, bitrate, codec, audioCodec)
    }

    /**
     * Cancel ongoing export
     */
    fun cancelExport() {
        nativeCancelExport()
    }

    // ===== Project Management =====

    /**
     * Save project
     * @param projectId Project ID
     * @param timelineId Timeline ID
     * @param projectData Project data as JSON string
     * @return true if saved
     */
    fun saveProject(projectId: String, timelineId: String, projectData: String): Boolean {
        return nativeSaveProject(projectId, timelineId, projectData)
    }

    /**
     * Load project
     * @param projectId Project ID
     * @return Timeline ID or empty string if failed
     */
    fun loadProject(projectId: String): String {
        return nativeLoadProject(projectId)
    }

    /**
     * Delete project
     * @param projectId Project ID
     * @return true if deleted
     */
    fun deleteProject(projectId: String): Boolean {
        return nativeDeleteProject(projectId)
    }

    // ===== History Management =====

    /**
     * Undo last operation
     * @param timelineId Timeline ID
     * @return true if undo successful
     */
    fun undo(timelineId: String): Boolean {
        return nativeUndo(timelineId)
    }

    /**
     * Redo last undone operation
     * @param timelineId Timeline ID
     * @return true if redo successful
     */
    fun redo(timelineId: String): Boolean {
        return nativeRedo(timelineId)
    }

    /**
     * Check if undo available
     * @param timelineId Timeline ID
     * @return true if can undo
     */
    fun canUndo(timelineId: String): Boolean {
        return nativeCanUndo(timelineId)
    }

    /**
     * Check if redo available
     * @param timelineId Timeline ID
     * @return true if can redo
     */
    fun canRedo(timelineId: String): Boolean {
        return nativeCanRedo(timelineId)
    }

    // ===== Native Methods =====

    private external fun nativeInitialize(cacheDir: String): Boolean
    private external fun nativeCleanup()

    private external fun nativeCreateTimeline(width: Int, height: Int, fps: Float): String
    private external fun nativeAddClip(timelineId: String, mediaPath: String, startTimeMs: Long, trackIndex: Int): String
    private external fun nativeRemoveClip(timelineId: String, clipId: String): Boolean
    private external fun nativeMoveClip(timelineId: String, clipId: String, newStartTimeMs: Long): Boolean
    private external fun nativeTrimClip(timelineId: String, clipId: String, trimStartMs: Long, trimEndMs: Long): Boolean
    private external fun nativeSplitClip(timelineId: String, clipId: String, splitTimeMs: Long): String

    private external fun nativeApplyEffect(timelineId: String, clipId: String, effectType: String, params: String): String
    private external fun nativeUpdateEffect(effectId: String, params: String): Boolean
    private external fun nativeRemoveEffect(effectId: String): Boolean
    private external fun nativeApplyColorGrade(timelineId: String, clipId: String, exposure: Float, saturation: Float, contrast: Float): Boolean

    private external fun nativeSetClipVolume(clipId: String, volume: Float): Boolean
    private external fun nativeApplyAudioEffect(clipId: String, effectType: String, params: String): String
    private external fun nativeAddAudioTrack(timelineId: String, trackName: String, trackType: String): String
    private external fun nativeGetAudioSpectrum(clipId: String, timeMs: Long): IntArray
    private external fun nativeDetectBeats(clipId: String): LongArray

    private external fun nativeSetClipSpeed(clipId: String, speed: Float): Boolean
    private external fun nativeSetClipReverse(clipId: String, reversed: Boolean): Boolean
    private external fun nativeCreateSpeedRamp(clipId: String, startTimeMs: Long, endTimeMs: Long, startSpeed: Float, endSpeed: Float): Boolean

    private external fun nativeAddKeyframe(clipId: String, propertyName: String, timeMs: Long, value: Float, interpolationType: String): Boolean
    private external fun nativeRemoveKeyframe(clipId: String, propertyName: String, timeMs: Long): Boolean
    private external fun nativeGetAnimationValue(clipId: String, propertyName: String, timeMs: Long): Float

    private external fun nativeAddTransition(clipId1: String, clipId2: String, transitionType: String, durationMs: Int): String
    private external fun nativeRemoveTransition(transitionId: String): Boolean

    private external fun nativeGetPreviewFrame(timelineId: String, timeMs: Long): ByteArray?
    private external fun nativeGetTimelineDuration(timelineId: String): Long
    private external fun nativeGetTimelineProperties(timelineId: String): String

    private external fun nativeExportTimeline(timelineId: String, outputPath: String, width: Int, height: Int, bitrate: Int, codec: String, audioCodec: String): Boolean
    private external fun nativeCancelExport()

    private external fun nativeSaveProject(projectId: String, timelineId: String, projectData: String): Boolean
    private external fun nativeLoadProject(projectId: String): String
    private external fun nativeDeleteProject(projectId: String): Boolean

    private external fun nativeUndo(timelineId: String): Boolean
    private external fun nativeRedo(timelineId: String): Boolean
    private external fun nativeCanUndo(timelineId: String): Boolean
    private external fun nativeCanRedo(timelineId: String): Boolean
}
