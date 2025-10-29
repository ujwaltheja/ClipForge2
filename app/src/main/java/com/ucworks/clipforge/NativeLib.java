package com.ucworks.clipforge;

/**
 * Native library interface for ClipForge video editor.
 *
 * This class provides the JNI wrapper for all native C++ VideoEngine operations.
 * It acts as a bridge between Java and the native C++ implementation.
 *
 * Usage:
 * <pre>
 *     // Create engine
 *     long enginePtr = NativeLib.createEngine();
 *     NativeLib.initEngine(enginePtr);
 *
 *     // Add clips
 *     String clipId = NativeLib.addClip(enginePtr, "/path/to/video.mp4", 0, 0);
 *
 *     // Start preview
 *     NativeLib.startPreview(enginePtr);
 *
 *     // Export
 *     NativeLib.startExport(enginePtr, "/path/to/output.mp4", "mp4", "high");
 *
 *     // Cleanup
 *     NativeLib.destroyEngine(enginePtr);
 * </pre>
 */
public class NativeLib {

    static {
        System.loadLibrary("clipforge");
    }

    // ========================================================================
    // Engine Lifecycle Management
    // ========================================================================

    /**
     * Create a new VideoEngine instance.
     *
     * @return Pointer to engine (as jlong)
     * @throws RuntimeException if creation fails
     */
    public static native long createEngine();

    /**
     * Initialize the engine with default configuration.
     *
     * Must be called after createEngine() before using the engine.
     *
     * @param enginePtr Engine pointer from createEngine()
     * @return true if initialization successful
     * @throws IllegalArgumentException if enginePtr is invalid
     */
    public static native boolean initEngine(long enginePtr);

    /**
     * Destroy engine and free all resources.
     *
     * Must be called when done with engine. Do not use enginePtr after this.
     *
     * @param enginePtr Engine pointer
     */
    public static native void destroyEngine(long enginePtr);

    /**
     * Get the native library version string.
     *
     * @param enginePtr Engine pointer
     * @return Version string (e.g., "1.0.0")
     */
    public static native String getEngineVersion(long enginePtr);

    // ========================================================================
    // Clip Management
    // ========================================================================

    /**
     * Add a video clip to the timeline.
     *
     * @param enginePtr Engine pointer
     * @param sourcePath Path to video file
     * @param startPosition Position on timeline in milliseconds
     * @param trackIndex Track number (0 = main, 1+ = overlay)
     * @return Clip ID if successful, empty string on error
     * @throws RuntimeException if operation fails
     */
    public static native String addClip(long enginePtr, String sourcePath,
                                       long startPosition, int trackIndex);

    /**
     * Remove a clip from the timeline.
     *
     * @param enginePtr Engine pointer
     * @param clipId ID of clip to remove
     * @return true if removed successfully
     * @throws RuntimeException if clip not found
     */
    public static native boolean removeClip(long enginePtr, String clipId);

    /**
     * Move a clip to a new position and/or track.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param newPosition New timeline position (ms)
     * @param newTrack New track index
     * @return true if moved successfully
     */
    public static native boolean moveClip(long enginePtr, String clipId,
                                         long newPosition, int newTrack);

    /**
     * Trim a clip (set in/out points).
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param trimStart Trim start point (ms from source)
     * @param trimEnd Trim end point (ms from source)
     * @return true if trimmed successfully
     */
    public static native boolean trimClip(long enginePtr, String clipId,
                                         long trimStart, long trimEnd);

    /**
     * Set clip playback speed.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param speed Speed multiplier (0.25f to 4.0f)
     * @return true if set successfully
     */
    public static native boolean setClipSpeed(long enginePtr, String clipId, float speed);

    /**
     * Set clip volume.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param volume Volume multiplier (0.0f to 2.0f)
     * @return true if set successfully
     */
    public static native boolean setClipVolume(long enginePtr, String clipId, float volume);

    /**
     * Split a clip at a specific time.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip to split
     * @param splitTime Time to split at (ms from clip start)
     * @return ID of new clip, empty string on error
     */
    public static native String splitClip(long enginePtr, String clipId, long splitTime);

    /**
     * Get total timeline duration in milliseconds.
     *
     * @param enginePtr Engine pointer
     * @return Duration in ms
     */
    public static native long getTimelineDuration(long enginePtr);

    /**
     * Get number of clips in timeline.
     *
     * @param enginePtr Engine pointer
     * @return Clip count
     */
    public static native int getClipCount(long enginePtr);

    /**
     * Get total number of effects applied to all clips.
     *
     * @param enginePtr Engine pointer
     * @return Total effect count
     */
    public static native int getEffectCount(long enginePtr);

    // ========================================================================
    // Effect Management
    // ========================================================================

    /**
     * Apply an effect to a clip.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param effectName Effect name (e.g., "Vintage", "B&W")
     * @return true if applied successfully
     */
    public static native boolean applyEffect(long enginePtr, String clipId, String effectName);

    /**
     * Remove an effect from a clip.
     *
     * @param enginePtr Engine pointer
     * @param clipId Clip ID
     * @param effectId Effect ID
     * @return true if removed successfully
     */
    public static native boolean removeEffect(long enginePtr, String clipId, String effectId);

    // ========================================================================
    // Audio Management
    // ========================================================================

    /**
     * Add an audio track to the project.
     *
     * @param enginePtr Engine pointer
     * @param name Track display name
     * @param type Track type ("main", "voiceover", "music", "sfx")
     * @return Track ID if successful
     */
    public static native String addAudioTrack(long enginePtr, String name, String type);

    /**
     * Remove an audio track.
     *
     * @param enginePtr Engine pointer
     * @param trackId Track ID
     * @return true if removed successfully
     */
    public static native boolean removeAudioTrack(long enginePtr, String trackId);

    /**
     * Set audio track volume.
     *
     * @param enginePtr Engine pointer
     * @param trackId Track ID
     * @param volume Volume multiplier (0.0f to 2.0f)
     * @return true if set successfully
     */
    public static native boolean setAudioTrackVolume(long enginePtr, String trackId,
                                                     float volume);

    /**
     * Set audio track mute state.
     *
     * @param enginePtr Engine pointer
     * @param trackId Track ID
     * @param muted true to mute
     * @return true if set successfully
     */
    public static native boolean setAudioTrackMuted(long enginePtr, String trackId,
                                                    boolean muted);

    // ========================================================================
    // Preview Control
    // ========================================================================

    /**
     * Start preview playback from current position.
     *
     * @param enginePtr Engine pointer
     * @return true if preview started
     */
    public static native boolean startPreview(long enginePtr);

    /**
     * Pause preview playback.
     *
     * @param enginePtr Engine pointer
     * @return true if paused
     */
    public static native boolean pausePreview(long enginePtr);

    /**
     * Stop preview playback completely.
     *
     * @param enginePtr Engine pointer
     * @return true if stopped
     */
    public static native boolean stopPreview(long enginePtr);

    /**
     * Seek preview to a specific time.
     *
     * @param enginePtr Engine pointer
     * @param timeMs Time to seek to (milliseconds)
     * @return true if seeked successfully
     */
    public static native boolean seekPreview(long enginePtr, long timeMs);

    /**
     * Check if preview is currently playing.
     *
     * @param enginePtr Engine pointer
     * @return true if playing
     */
    public static native boolean isPreviewPlaying(long enginePtr);

    /**
     * Get current preview playback position.
     *
     * @param enginePtr Engine pointer
     * @return Current position in milliseconds
     */
    public static native long getPreviewPosition(long enginePtr);

    // ========================================================================
    // Export/Rendering
    // ========================================================================

    /**
     * Start exporting/rendering the project to file.
     *
     * This operation runs asynchronously. Use isExporting() and
     * getExportProgress() to monitor progress.
     *
     * @param enginePtr Engine pointer
     * @param outputPath Where to save output file
     * @param format Output format ("mp4", "webm", etc.)
     * @param quality Quality preset ("low", "medium", "high", "ultra")
     * @return true if export started successfully
     * @throws RuntimeException if operation fails
     */
    public static native boolean startExport(long enginePtr, String outputPath,
                                            String format, String quality);

    /**
     * Cancel current export operation.
     *
     * @param enginePtr Engine pointer
     * @return true if cancelled
     */
    public static native boolean cancelExport(long enginePtr);

    /**
     * Check if export is currently in progress.
     *
     * @param enginePtr Engine pointer
     * @return true if currently exporting
     */
    public static native boolean isExporting(long enginePtr);

    /**
     * Get export progress as percentage.
     *
     * @param enginePtr Engine pointer
     * @return Progress 0-100
     */
    public static native float getExportProgress(long enginePtr);

    // ========================================================================
    // Project Management
    // ========================================================================

    /**
     * Save project to file.
     *
     * @param enginePtr Engine pointer
     * @param projectPath Where to save project
     * @return true if saved successfully
     */
    public static native boolean saveProject(long enginePtr, String projectPath);

    /**
     * Load project from file.
     *
     * @param enginePtr Engine pointer
     * @param projectPath Path to project file
     * @return true if loaded successfully
     */
    public static native boolean loadProject(long enginePtr, String projectPath);

    /**
     * Check if project has unsaved changes.
     *
     * @param enginePtr Engine pointer
     * @return true if modified
     */
    public static native boolean hasUnsavedChanges(long enginePtr);

    // ========================================================================
    // Utility Methods
    // ========================================================================

    /**
     * Get engine state as string for debugging.
     *
     * @param enginePtr Engine pointer
     * @return State string ("IDLE", "PREVIEW_PLAYING", "RENDERING", etc.)
     */
    public static native String getEngineState(long enginePtr);

    /**
     * Get last error message if engine is in error state.
     *
     * @param enginePtr Engine pointer
     * @return Error message or empty string
     */
    public static native String getErrorMessage(long enginePtr);

    /**
     * Get memory usage in bytes.
     *
     * @param enginePtr Engine pointer
     * @return Memory used in bytes
     */
    public static native long getMemoryUsage(long enginePtr);
}
