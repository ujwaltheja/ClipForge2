package com.ucworks.clipforge.ui.views

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.*
import android.util.AttributeSet
import android.view.GestureDetector
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View
import kotlin.math.max
import kotlin.math.min

/**
 * Professional timeline view with:
 * - Horizontal scrolling
 * - Pinch zoom
 * - Clip dragging and selection
 * - Playhead scrubbing
 * - Snap to grid
 */
class TimelineView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    // Colors
    private val colorBackground = Color.parseColor("#1A1625")
    private val colorGrid = Color.parseColor("#3D3D4D")
    private val colorClip = Color.parseColor("#7C5FD8")
    private val colorClipSelected = Color.parseColor("#9D7FFF")
    private val colorPlayhead = Color.parseColor("#FF6B6B")
    private val colorText = Color.parseColor("#E6E1E5")

    // Paint objects
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val gridPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = colorGrid
        strokeWidth = 1f
    }
    private val playheadPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = colorPlayhead
        strokeWidth = 3f
    }

    // Timeline state
    private var timelineStartMs = 0L
    private var timelineDurationMs = 60000L     // 60 seconds default
    private var zoomLevel = 1.0f                // Pixels per millisecond
    private var scrollOffsetX = 0f
    private var currentPositionMs = 0L

    // Clips
    private data class ClipView(
        val id: String,
        val startMs: Long,
        val durationMs: Long,
        var x: Float = 0f,
        var y: Float = 0f,
        var width: Float = 0f,
        var height: Float = 0f,
        var isSelected: Boolean = false
    )

    private val clips = mutableListOf<ClipView>()
    private var selectedClipId: String? = null
    private var draggedClipId: String? = null
    private var dragStartX = 0f

    // Gesture handling
    private val gestureDetector: GestureDetector
    private val scaleGestureDetector: ScaleGestureDetector
    private var isScaling = false

    // Listeners
    var onClipSelected: ((String) -> Unit)? = null
    var onClipMoved: ((String, Long) -> Unit)? = null
    var onPlayheadPositionChanged: ((Long) -> Unit)? = null

    // Constants
    private val minZoom = 0.1f
    private val maxZoom = 5.0f
    private val trackHeight = 80f
    private val trackMargin = 4f

    init {
        setBackgroundColor(colorBackground)

        gestureDetector = GestureDetector(context, GestureListener())
        scaleGestureDetector = ScaleGestureDetector(context, ScaleListener())
    }

    /**
     * Add clip to timeline
     * @param id Clip ID
     * @param startMs Clip start time in milliseconds
     * @param durationMs Clip duration in milliseconds
     */
    fun addClip(id: String, startMs: Long, durationMs: Long) {
        clips.add(ClipView(id, startMs, durationMs))
        updateClipLayout()
        invalidate()
    }

    /**
     * Remove clip from timeline
     * @param id Clip ID
     */
    fun removeClip(id: String) {
        clips.removeAll { it.id == id }
        if (selectedClipId == id) {
            selectedClipId = null
        }
        updateClipLayout()
        invalidate()
    }

    /**
     * Move clip to new position
     * @param id Clip ID
     * @param newStartMs New start time in milliseconds
     */
    fun moveClip(id: String, newStartMs: Long) {
        val index = clips.indexOfFirst { it.id == id }
        if (index >= 0) {
            val clip = clips[index]
            // Create a new clip with updated startMs
            clips[index] = clip.copy(startMs = newStartMs)
            updateClipLayout()
            invalidate()
        }
    }

    /**
     * Select clip
     * @param id Clip ID
     */
    fun selectClip(id: String) {
        selectedClipId = id
        clips.forEach { it.isSelected = it.id == id }
        invalidate()
    }

    /**
     * Set current playback position
     * @param positionMs Position in milliseconds
     */
    fun setCurrentPosition(positionMs: Long) {
        currentPositionMs = positionMs.coerceIn(0, timelineDurationMs)
        invalidate()
    }

    /**
     * Set zoom level
     * @param zoom Zoom level (pixels per millisecond)
     */
    fun setZoom(zoom: Float) {
        zoomLevel = zoom.coerceIn(minZoom, maxZoom)
        updateClipLayout()
        invalidate()
    }

    /**
     * Get current zoom level
     */
    fun getZoom(): Float = zoomLevel

    /**
     * Pan timeline
     * @param offsetMs Time offset in milliseconds
     */
    fun panTimeline(offsetMs: Long) {
        timelineStartMs = (timelineStartMs + offsetMs).coerceIn(0, max(0, timelineDurationMs - 10000))
        updateClipLayout()
        invalidate()
    }

    /**
     * Zoom to fit entire timeline
     */
    fun zoomToFit() {
        zoomLevel = width.toFloat() / timelineDurationMs
        updateClipLayout()
        invalidate()
    }

    @SuppressLint("DrawAllocation")
    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)

        drawGrid(canvas)
        drawTracks(canvas)
        drawClips(canvas)
        drawPlayhead(canvas)
        drawRuler(canvas)
    }

    private fun drawGrid(canvas: Canvas) {
        val gridInterval = (1000 / zoomLevel).toLong().coerceIn(100, 5000)
        var timeMs = (timelineStartMs / gridInterval) * gridInterval

        while (timeMs <= timelineStartMs + (width / zoomLevel).toLong()) {
            val x = msToPixels(timeMs) - scrollOffsetX
            if (x >= 0 && x <= width) {
                canvas.drawLine(x, 0f, x, height.toFloat(), gridPaint)
            }
            timeMs += gridInterval
        }
    }

    private fun drawTracks(canvas: Canvas) {
        val trackCount = (height / (trackHeight + trackMargin)).toInt()
        for (i in 0 until trackCount) {
            val y = i * (trackHeight + trackMargin)
            val rect = RectF(0f, y, width.toFloat(), y + trackHeight)

            if (i % 2 == 0) {
                paint.color = Color.parseColor("#2A2430")
            } else {
                paint.color = Color.parseColor("#1A1625")
            }
            canvas.drawRect(rect, paint)

            // Draw border
            paint.color = colorGrid
            paint.style = Paint.Style.STROKE
            canvas.drawRect(rect, paint)
            paint.style = Paint.Style.FILL
        }
    }

    private fun drawClips(canvas: Canvas) {
        for (clip in clips) {
            val rect = RectF(clip.x, clip.y, clip.x + clip.width, clip.y + clip.height)

            // Draw clip background
            paint.color = if (clip.isSelected) colorClipSelected else colorClip
            canvas.drawRoundRect(rect, 4f, 4f, paint)

            // Draw clip border if selected
            if (clip.isSelected) {
                paint.color = colorText
                paint.style = Paint.Style.STROKE
                paint.strokeWidth = 2f
                canvas.drawRoundRect(rect, 4f, 4f, paint)
                paint.style = Paint.Style.FILL
            }

            // Draw clip text
            paint.color = colorText
            paint.textSize = 12f
            canvas.drawText(clip.id, clip.x + 8f, clip.y + clip.height - 8f, paint)
        }
    }

    private fun drawPlayhead(canvas: Canvas) {
        val x = msToPixels(currentPositionMs) - scrollOffsetX
        if (x >= 0 && x <= width) {
            canvas.drawLine(x, 0f, x, height.toFloat(), playheadPaint)

            // Draw triangle at top
            val path = Path().apply {
                moveTo(x - 8, 0f)
                lineTo(x + 8, 0f)
                lineTo(x, 16f)
                close()
            }
            paint.color = colorPlayhead
            canvas.drawPath(path, paint)
        }
    }

    private fun drawRuler(canvas: Canvas) {
        val rulerHeight = 30f
        val gridInterval = (1000 / zoomLevel).toLong().coerceIn(100, 5000)
        var timeMs = (timelineStartMs / gridInterval) * gridInterval

        paint.color = colorText
        paint.textSize = 10f

        while (timeMs <= timelineStartMs + (width / zoomLevel).toLong()) {
            val x = msToPixels(timeMs) - scrollOffsetX
            if (x >= 0 && x <= width) {
                canvas.drawLine(x, 0f, x, 4f, gridPaint)

                val timeStr = formatTime(timeMs)
                canvas.drawText(timeStr, x + 4, 12f, paint)
            }
            timeMs += gridInterval
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleGestureDetector.onTouchEvent(event)
        gestureDetector.onTouchEvent(event)

        when (event.action) {
            MotionEvent.ACTION_DOWN -> {
                val clipId = getClipAtPoint(event.x, event.y)
                if (clipId != null) {
                    selectClip(clipId)
                    draggedClipId = clipId
                    dragStartX = event.x
                    onClipSelected?.invoke(clipId)
                } else {
                    // Scrub timeline
                    val timeMs = pixelsToMs(event.x + scrollOffsetX)
                    currentPositionMs = timeMs
                    onPlayheadPositionChanged?.invoke(timeMs)
                    invalidate()
                }
            }

            MotionEvent.ACTION_MOVE -> {
                if (draggedClipId != null && !isScaling) {
                    val clip = clips.find { it.id == draggedClipId }
                    if (clip != null) {
                        val deltaPixels = event.x - dragStartX
                        val deltaMs = (deltaPixels / zoomLevel).toLong()
                        val newStartMs = (clip.startMs + deltaMs).coerceAtLeast(0)
                        onClipMoved?.invoke(clip.id, newStartMs)
                        dragStartX = event.x
                    }
                }
            }

            MotionEvent.ACTION_UP -> {
                draggedClipId = null
            }
        }

        return true
    }

    private fun getClipAtPoint(x: Float, y: Float): String? {
        for (clip in clips) {
            if (x >= clip.x && x <= clip.x + clip.width &&
                y >= clip.y && y <= clip.y + clip.height
            ) {
                return clip.id
            }
        }
        return null
    }

    private fun updateClipLayout() {
        var currentY = 0f
        for ((index, clip) in clips.withIndex()) {
            clip.x = msToPixels(clip.startMs) - scrollOffsetX
            clip.y = currentY
            clip.width = (clip.durationMs * zoomLevel).coerceAtLeast(2f)
            clip.height = trackHeight

            currentY += trackHeight + trackMargin
        }
    }

    private fun msToPixels(timeMs: Long): Float {
        return (timeMs - timelineStartMs) * zoomLevel
    }

    private fun pixelsToMs(pixels: Float): Long {
        return timelineStartMs + (pixels / zoomLevel).toLong()
    }

    private fun formatTime(ms: Long): String {
        val seconds = ms / 1000
        val minutes = seconds / 60
        val hours = minutes / 60
        return String.format("%02d:%02d:%02d", hours, minutes % 60, seconds % 60)
    }

    // Gesture listeners
    private inner class GestureListener : GestureDetector.SimpleOnGestureListener() {
        override fun onScroll(
            e1: MotionEvent?, e2: MotionEvent, distanceX: Float, distanceY: Float
        ): Boolean {
            if (!isScaling) {
                scrollOffsetX = (scrollOffsetX + distanceX).coerceIn(
                    0f, max(0f, width - (timelineDurationMs * zoomLevel))
                )
                updateClipLayout()
                invalidate()
            }
            return true
        }
    }

    private inner class ScaleListener : ScaleGestureDetector.SimpleOnScaleGestureListener() {
        override fun onScale(detector: ScaleGestureDetector): Boolean {
            isScaling = detector.scaleFactor != 1.0f
            val newZoom = zoomLevel * detector.scaleFactor
            setZoom(newZoom)
            return true
        }

        override fun onScaleEnd(detector: ScaleGestureDetector) {
            isScaling = false
        }
    }
}
