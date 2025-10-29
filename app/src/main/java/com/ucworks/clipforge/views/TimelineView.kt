package com.ucworks.clipforge.views

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.Rect
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import com.ucworks.clipforge.models.VideoClip
import timber.log.Timber
import kotlin.math.max
import kotlin.math.min

/**
 * Custom Timeline View for visualizing and interacting with video clips.
 *
 * Features:
 * - Display video clips on timeline
 * - Seek/scrub functionality
 * - Clip selection and interaction
 * - Time marker display
 * - Real-time playhead position
 */
class TimelineView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    // Paint objects for drawing
    private val backgroundPaint = Paint().apply {
        color = Color.parseColor("#0F0B15")  // background_darker
        style = Paint.Style.FILL
    }

    private val gridPaint = Paint().apply {
        color = Color.parseColor("#2D2438")  // surface_dark
        style = Paint.Style.STROKE
        strokeWidth = 1f
    }

    private val timelinePaint = Paint().apply {
        color = Color.parseColor("#666666")  // text_tertiary
        style = Paint.Style.FILL
    }

    private val clipPaint = Paint().apply {
        color = Color.parseColor("#7C5FD8")  // brand_purple
        style = Paint.Style.FILL
    }

    private val selectedClipPaint = Paint().apply {
        color = Color.parseColor("#8B7DE8")  // brand_purple_light
        style = Paint.Style.STROKE
        strokeWidth = 3f
    }

    private val playheadPaint = Paint().apply {
        color = Color.parseColor("#FF1493")  // accent_pink
        style = Paint.Style.FILL
        strokeWidth = 3f
    }

    private val textPaint = Paint().apply {
        color = Color.parseColor("#FFFFFF")  // text_primary
        textSize = 12f
        style = Paint.Style.FILL
    }

    // Timeline properties
    private var clips = mutableListOf<VideoClip>()
    private var timelineDuration = 0L  // milliseconds
    private var currentPosition = 0L    // milliseconds
    private var selectedClipId: String? = null
    private var pixelsPerSecond = 50f   // Zoom level

    // Dimensions
    private val timelineHeight = 80  // pixels
    private val rulerHeight = 30    // pixels
    private val clipPadding = 8     // pixels

    // Callbacks
    var onClipSelected: (VideoClip) -> Unit = {}
    var onSeek: (Long) -> Unit = {}
    var onClipMoved: (VideoClip) -> Unit = {}

    // Touch handling
    private var isDragging = false
    private var draggedClipId: String? = null
    private var dragStartX = 0f

    init {
        setBackgroundColor(Color.parseColor("#1A1625"))  // background_dark
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        // Draw background
        canvas.drawRect(0f, 0f, width.toFloat(), height.toFloat(), backgroundPaint)

        // Draw ruler (time markers)
        drawRuler(canvas)

        // Draw clips
        drawClips(canvas)

        // Draw playhead
        drawPlayhead(canvas)
    }

    private fun drawRuler(canvas: Canvas) {
        val rulerY = rulerHeight.toFloat()
        val secondWidth = pixelsPerSecond

        // Draw ruler background
        canvas.drawRect(0f, 0f, width.toFloat(), rulerY, timelinePaint)

        // Draw time markers
        var timeMs = 0L
        var x = 0f

        while (x < width) {
            // Draw marker every second
            canvas.drawLine(x, rulerY - 10, x, rulerY, gridPaint)

            // Draw time label every 5 seconds
            if ((timeMs / 1000) % 5 == 0L) {
                val timeText = formatTimeLabel(timeMs)
                canvas.drawText(timeText, x + 5, rulerY - 5, textPaint)
            }

            x += secondWidth
            timeMs += 1000
        }
    }

    private fun drawClips(canvas: Canvas) {
        for (clip in clips) {
            drawClip(canvas, clip)
        }
    }

    private fun drawClip(canvas: Canvas, clip: VideoClip) {
        val startX = positionToX(clip.positionMs).toFloat()
        val endX = positionToX(clip.endPositionMs).toFloat()
        val top = rulerHeight + clipPadding
        val bottom = rulerHeight + timelineHeight - clipPadding

        // Draw clip background
        canvas.drawRect(startX, top.toFloat(), endX, bottom.toFloat(), clipPaint)

        // Draw selected clip border
        if (clip.id == selectedClipId) {
            canvas.drawRect(startX, top.toFloat(), endX, bottom.toFloat(), selectedClipPaint)
        }

        // Draw clip label
        val label = clip.displayName.take(15)
        canvas.drawText(label, startX + 5, (top + 20).toFloat(), textPaint)

        // Draw clip duration
        val duration = clip.formattedDuration
        canvas.drawText(duration, startX + 5, (top + 35).toFloat(), textPaint)

        Timber.d("Drew clip: ${clip.displayName} at position $startX-$endX")
    }

    private fun drawPlayhead(canvas: Canvas) {
        val x = positionToX(currentPosition).toFloat()

        // Draw playhead line
        canvas.drawLine(
            x,
            rulerHeight.toFloat(),
            x,
            (rulerHeight + timelineHeight).toFloat(),
            playheadPaint
        )

        // Draw playhead marker
        canvas.drawCircle(x, rulerHeight.toFloat(), 5f, playheadPaint)

        Timber.d("Drew playhead at position: ${currentPosition}ms")
    }

    override fun onTouchEvent(event: MotionEvent?): Boolean {
        if (event == null) return false

        return when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                handleTouchDown(event)
                true
            }
            MotionEvent.ACTION_MOVE -> {
                handleTouchMove(event)
                true
            }
            MotionEvent.ACTION_UP -> {
                handleTouchUp(event)
                true
            }
            else -> super.onTouchEvent(event)
        }
    }

    private fun handleTouchDown(event: MotionEvent) {
        val y = event.y
        val x = event.x

        // Check if clicking on ruler (seek)
        if (y < rulerHeight) {
            handleSeek(x)
            return
        }

        // Check if clicking on clip
        val clickedClip = getClipAtPosition(x, y)
        if (clickedClip != null) {
            selectedClipId = clickedClip.id
            draggedClipId = clickedClip.id
            dragStartX = x
            onClipSelected(clickedClip)
            Timber.d("Selected clip: ${clickedClip.displayName}")
        }

        invalidate()
    }

    private fun handleTouchMove(event: MotionEvent) {
        if (draggedClipId != null) {
            val clip = clips.find { it.id == draggedClipId } ?: return
            val deltaX = event.x - dragStartX
            val deltaMs = xToPosition(deltaX.toLong())

            val newPosition = (clip.positionMs + deltaMs).coerceIn(0, timelineDuration - clip.durationMs)
            // Update clip position (would call onClipMoved callback)
            Timber.d("Dragging clip: delta=${deltaMs}ms, newPosition=${newPosition}ms")
        }
    }

    private fun handleTouchUp(event: MotionEvent) {
        if (draggedClipId != null) {
            val clip = clips.find { it.id == draggedClipId }
            if (clip != null) {
                onClipMoved(clip)
                Timber.d("Clip moved: ${clip.displayName}")
            }
        }
        draggedClipId = null
    }

    private fun handleSeek(x: Float) {
        val positionMs = xToPosition(x.toLong())
        currentPosition = positionMs.coerceIn(0, timelineDuration)
        onSeek(currentPosition)
        Timber.d("Seeked to: ${currentPosition}ms")
        invalidate()
    }

    private fun getClipAtPosition(x: Float, y: Float): VideoClip? {
        if (y < rulerHeight || y > rulerHeight + timelineHeight) {
            return null
        }

        return clips.find { clip ->
            val startX = positionToX(clip.positionMs)
            val endX = positionToX(clip.endPositionMs)
            x >= startX && x <= endX
        }
    }

    private fun positionToX(positionMs: Long): Long {
        return (positionMs / 1000) * pixelsPerSecond.toLong()
    }

    private fun xToPosition(x: Long): Long {
        return ((x / pixelsPerSecond) * 1000).toLong()
    }

    private fun formatTimeLabel(timeMs: Long): String {
        val totalSeconds = timeMs / 1000
        val minutes = totalSeconds / 60
        val seconds = totalSeconds % 60
        return String.format("%02d:%02d", minutes, seconds)
    }

    // Public API

    fun setClips(newClips: List<VideoClip>) {
        clips.clear()
        clips.addAll(newClips)
        if (clips.isNotEmpty()) {
            timelineDuration = clips.maxOf { it.endPositionMs }
        }
        invalidate()
        Timber.d("Timeline clips updated: ${clips.size} clips")
    }

    fun addClip(clip: VideoClip) {
        clips.add(clip)
        timelineDuration = max(timelineDuration, clip.endPositionMs)
        invalidate()
        Timber.d("Clip added to timeline: ${clip.displayName}")
    }

    fun removeClip(clipId: String) {
        clips.removeAll { it.id == clipId }
        timelineDuration = if (clips.isNotEmpty()) clips.maxOf { it.endPositionMs } else 0L
        if (selectedClipId == clipId) {
            selectedClipId = null
        }
        invalidate()
        Timber.d("Clip removed from timeline: $clipId")
    }

    fun setCurrentPosition(positionMs: Long) {
        currentPosition = positionMs.coerceIn(0, timelineDuration)
        invalidate()
    }

    fun selectClip(clipId: String) {
        selectedClipId = clipId
        val selectedClip = clips.find { it.id == clipId }
        if (selectedClip != null) {
            onClipSelected(selectedClip)
        }
        invalidate()
    }

    fun clearSelection() {
        selectedClipId = null
        invalidate()
    }

    fun setZoom(zoom: Float) {
        pixelsPerSecond = (50 * zoom).coerceIn(10f, 200f)
        invalidate()
        Timber.d("Timeline zoom changed to: $pixelsPerSecond pixels/second")
    }

    fun getTimelineDuration(): Long = timelineDuration

    fun getCurrentPosition(): Long = currentPosition

    fun getSelectedClip(): VideoClip? = clips.find { it.id == selectedClipId }
}
