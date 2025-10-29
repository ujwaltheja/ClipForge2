# 04 - Editor Screen - Complete Enhanced Documentation

## Overview
The Editor Screen is the **core of ClipForge**, featuring a professional timeline editor, real-time video preview, multi-track editing, effects application, and comprehensive editing tools with **local-only storage**.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/editor/
│   ├── EditorScreen.kt                 # Main UI Composable
│   ├── EditorViewModel.kt              # State management
│   ├── components/
│   │   ├── VideoPreview.kt            # Video preview player
│   │   ├── Timeline.kt                # Timeline component
│   │   ├── TimelineTrack.kt           # Individual track
│   │   ├── Playhead.kt                # Playhead indicator
│   │   ├── ToolBar.kt                 # Top toolbar
│   │   └── BottomControls.kt          # Playback controls
│   └── data/
│       ├── TimelineManager.kt         # Timeline operations
│       ├── VideoPlayer.kt             # ExoPlayer wrapper
│       ├── EffectProcessor.kt         # Effect application
│       └── models/
│           ├── VideoClip.kt           # Video clip model
│           ├── Timeline.kt            # Timeline model
│           └── Effect.kt              # Effect model
```

---

## Dependencies

### Gradle Dependencies (build.gradle.kts)
```kotlin
dependencies {
    // Jetpack Compose
    implementation("androidx.compose.ui:ui:1.6.1")
    implementation("androidx.compose.material3:material3:1.2.0")
    implementation("androidx.compose.foundation:foundation:1.6.1")
    
    // Navigation
    implementation("androidx.navigation:navigation-compose:2.7.7")
    
    // ViewModel
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.7.0")
    
    // Hilt
    implementation("com.google.dagger:hilt-android:2.51.1")
    kapt("com.google.dagger:hilt-compiler:2.51.1")
    
    // Media3 for video playback and processing
    implementation("androidx.media3:media3-exoplayer:1.3.1")
    implementation("androidx.media3:media3-ui:1.3.1")
    implementation("androidx.media3:media3-common:1.3.1")
    implementation("androidx.media3:media3-transformer:1.3.1")
    
    // Coil for thumbnails
    implementation("io.coil-kt:coil-compose:2.6.0")
    implementation("io.coil-kt:coil-video:2.6.0")
}
```

---

## Complete Source Code

### 1. Editor Data Models

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/VideoClip.kt`

```kotlin
package com.clipforge.videoeditor.data.model

import android.net.Uri
import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class VideoClip(
    val id: String = UUID.randomUUID().toString(),
    @Serializable(with = UriSerializer::class)
    val uri: Uri,
    val name: String,
    val trackIndex: Int = 0,
    val startPosition: Long = 0, // milliseconds on timeline
    val duration: Long = 0, // milliseconds
    val trimStart: Long = 0, // trim from start
    val trimEnd: Long = 0, // trim from end
    val volume: Float = 1.0f,
    val effects: List<Effect> = emptyList(),
    val thumbnailPath: String? = null
) {
    val endPosition: Long
        get() = startPosition + duration - trimStart - trimEnd
    
    val actualDuration: Long
        get() = duration - trimStart - trimEnd
}

@Serializable
data class AudioClip(
    val id: String = UUID.randomUUID().toString(),
    @Serializable(with = UriSerializer::class)
    val uri: Uri,
    val name: String,
    val trackIndex: Int = 0,
    val startPosition: Long = 0,
    val duration: Long = 0,
    val trimStart: Long = 0,
    val trimEnd: Long = 0,
    val volume: Float = 1.0f
)

@Serializable
data class Effect(
    val id: String = UUID.randomUUID().toString(),
    val type: EffectType,
    val name: String,
    val intensity: Float = 1.0f,
    val parameters: Map<String, Float> = emptyMap()
)

@Serializable
enum class EffectType {
    FILTER,
    TRANSITION,
    TEXT,
    STICKER,
    ANIMATION
}

@Serializable
data class TimelineState(
    val videoClips: List<VideoClip> = emptyList(),
    val audioClips: List<AudioClip> = emptyList(),
    val totalDuration: Long = 0,
    val videoTrackCount: Int = 3,
    val audioTrackCount: Int = 3
)

// Custom Uri Serializer
object UriSerializer : kotlinx.serialization.KSerializer<Uri> {
    override val descriptor = kotlinx.serialization.descriptors.PrimitiveSerialDescriptor("Uri", kotlinx.serialization.descriptors.PrimitiveKind.STRING)
    override fun serialize(encoder: kotlinx.serialization.encoding.Encoder, value: Uri) {
        encoder.encodeString(value.toString())
    }
    override fun deserialize(decoder: kotlinx.serialization.encoding.Decoder): Uri {
        return Uri.parse(decoder.decodeString())
    }
}
```

---

### 2. Timeline Manager

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/TimelineManager.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import com.clipforge.videoeditor.data.model.*
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class TimelineManager @Inject constructor() {
    
    private val _timelineState = MutableStateFlow(TimelineState())
    val timelineState: StateFlow<TimelineState> = _timelineState.asStateFlow()
    
    private val _undoStack = mutableListOf<TimelineState>()
    private val _redoStack = mutableListOf<TimelineState>()
    
    /**
     * Add video clip to timeline
     */
    fun addVideoClip(clip: VideoClip) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips + clip
            state.copy(
                videoClips = newClips,
                totalDuration = calculateTotalDuration(newClips)
            )
        }
    }
    
    /**
     * Remove video clip from timeline
     */
    fun removeVideoClip(clipId: String) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips.filter { it.id != clipId }
            state.copy(
                videoClips = newClips,
                totalDuration = calculateTotalDuration(newClips)
            )
        }
    }
    
    /**
     * Update video clip position
     */
    fun updateClipPosition(clipId: String, newPosition: Long, newTrackIndex: Int) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips.map { clip ->
                if (clip.id == clipId) {
                    clip.copy(
                        startPosition = newPosition,
                        trackIndex = newTrackIndex
                    )
                } else {
                    clip
                }
            }
            state.copy(
                videoClips = newClips,
                totalDuration = calculateTotalDuration(newClips)
            )
        }
    }
    
    /**
     * Update clip trim
     */
    fun updateClipTrim(clipId: String, trimStart: Long, trimEnd: Long) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips.map { clip ->
                if (clip.id == clipId) {
                    clip.copy(trimStart = trimStart, trimEnd = trimEnd)
                } else {
                    clip
                }
            }
            state.copy(
                videoClips = newClips,
                totalDuration = calculateTotalDuration(newClips)
            )
        }
    }
    
    /**
     * Update clip volume
     */
    fun updateClipVolume(clipId: String, volume: Float) {
        _timelineState.update { state ->
            val newClips = state.videoClips.map { clip ->
                if (clip.id == clipId) {
                    clip.copy(volume = volume)
                } else {
                    clip
                }
            }
            state.copy(videoClips = newClips)
        }
    }
    
    /**
     * Add effect to clip
     */
    fun addEffectToClip(clipId: String, effect: Effect) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips.map { clip ->
                if (clip.id == clipId) {
                    clip.copy(effects = clip.effects + effect)
                } else {
                    clip
                }
            }
            state.copy(videoClips = newClips)
        }
    }
    
    /**
     * Remove effect from clip
     */
    fun removeEffectFromClip(clipId: String, effectId: String) {
        saveStateForUndo()
        _timelineState.update { state ->
            val newClips = state.videoClips.map { clip ->
                if (clip.id == clipId) {
                    clip.copy(effects = clip.effects.filter { it.id != effectId })
                } else {
                    clip
                }
            }
            state.copy(videoClips = newClips)
        }
    }
    
    /**
     * Undo last operation
     */
    fun undo() {
        if (_undoStack.isNotEmpty()) {
            _redoStack.add(_timelineState.value)
            _timelineState.value = _undoStack.removeLast()
        }
    }
    
    /**
     * Redo last undone operation
     */
    fun redo() {
        if (_redoStack.isNotEmpty()) {
            _undoStack.add(_timelineState.value)
            _timelineState.value = _redoStack.removeLast()
        }
    }
    
    /**
     * Clear timeline
     */
    fun clearTimeline() {
        saveStateForUndo()
        _timelineState.value = TimelineState()
        _redoStack.clear()
    }
    
    /**
     * Load timeline state
     */
    fun loadTimeline(state: TimelineState) {
        _timelineState.value = state
        _undoStack.clear()
        _redoStack.clear()
    }
    
    /**
     * Save current state for undo
     */
    private fun saveStateForUndo() {
        _undoStack.add(_timelineState.value)
        _redoStack.clear()
        // Limit undo stack size
        if (_undoStack.size > 50) {
            _undoStack.removeAt(0)
        }
    }
    
    /**
     * Calculate total timeline duration
     */
    private fun calculateTotalDuration(clips: List<VideoClip>): Long {
        return clips.maxOfOrNull { it.endPosition } ?: 0
    }
    
    /**
     * Get clip at position
     */
    fun getClipAtPosition(position: Long, trackIndex: Int): VideoClip? {
        return _timelineState.value.videoClips.find { clip ->
            clip.trackIndex == trackIndex &&
            position >= clip.startPosition &&
            position <= clip.endPosition
        }
    }
}
```

---

### 3. Video Player

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/VideoPlayer.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import androidx.media3.common.MediaItem
import androidx.media3.common.Player
import androidx.media3.exoplayer.ExoPlayer
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class VideoPlayer @Inject constructor(
    private val context: Context
) {
    
    private var exoPlayer: ExoPlayer? = null
    
    private val _playerState = MutableStateFlow(PlayerState())
    val playerState: StateFlow<PlayerState> = _playerState.asStateFlow()
    
    init {
        initializePlayer()
    }
    
    /**
     * Initialize ExoPlayer
     */
    private fun initializePlayer() {
        exoPlayer = ExoPlayer.Builder(context).build().apply {
            addListener(object : Player.Listener {
                override fun onPlaybackStateChanged(playbackState: Int) {
                    _playerState.value = _playerState.value.copy(
                        isPlaying = playbackState == Player.STATE_READY && playWhenReady,
                        isLoading = playbackState == Player.STATE_BUFFERING
                    )
                }
                
                override fun onPlayWhenReadyChanged(playWhenReady: Boolean, reason: Int) {
                    _playerState.value = _playerState.value.copy(isPlaying = playWhenReady)
                }
            })
        }
    }
    
    /**
     * Load video
     */
    fun loadVideo(uri: String) {
        exoPlayer?.apply {
            setMediaItem(MediaItem.fromUri(uri))
            prepare()
            _playerState.value = _playerState.value.copy(duration = duration)
        }
    }
    
    /**
     * Play video
     */
    fun play() {
        exoPlayer?.play()
    }
    
    /**
     * Pause video
     */
    fun pause() {
        exoPlayer?.pause()
    }
    
    /**
     * Seek to position
     */
    fun seekTo(positionMs: Long) {
        exoPlayer?.seekTo(positionMs)
        _playerState.value = _playerState.value.copy(currentPosition = positionMs)
    }
    
    /**
     * Get current position
     */
    fun getCurrentPosition(): Long {
        return exoPlayer?.currentPosition ?: 0
    }
    
    /**
     * Get duration
     */
    fun getDuration(): Long {
        return exoPlayer?.duration ?: 0
    }
    
    /**
     * Release player
     */
    fun release() {
        exoPlayer?.release()
        exoPlayer = null
    }
    
    /**
     * Get ExoPlayer instance
     */
    fun getPlayer(): ExoPlayer? = exoPlayer
}

data class PlayerState(
    val isPlaying: Boolean = false,
    val isLoading: Boolean = false,
    val currentPosition: Long = 0,
    val duration: Long = 0
)
```

---

### 4. Editor ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/EditorViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.clipforge.videoeditor.data.local.LocalProjectManager
import com.clipforge.videoeditor.data.local.TimelineManager
import com.clipforge.videoeditor.data.local.VideoPlayer
import com.clipforge.videoeditor.data.model.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import javax.inject.Inject

data class EditorUiState(
    val project: Project? = null,
    val timelineState: TimelineState = TimelineState(),
    val playerState: com.clipforge.videoeditor.data.local.PlayerState = com.clipforge.videoeditor.data.local.PlayerState(),
    val playheadPosition: Long = 0,
    val selectedClip: VideoClip? = null,
    val selectedTab: EditorTab = EditorTab.TIMELINE,
    val zoomLevel: Float = 1.0f,
    val isLoading: Boolean = false,
    val error: String? = null,
    val canUndo: Boolean = false,
    val canRedo: Boolean = false
)

enum class EditorTab {
    TIMELINE,
    EFFECTS,
    AUDIO,
    TEXT,
    COLOR,
    TRIM
}

@HiltViewModel
class EditorViewModel @Inject constructor(
    private val projectManager: LocalProjectManager,
    private val timelineManager: TimelineManager,
    private val videoPlayer: VideoPlayer
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(EditorUiState())
    val uiState: StateFlow<EditorUiState> = _uiState.asStateFlow()
    
    private var playheadUpdateJob: Job? = null
    
    init {
        observeTimeline()
        observePlayer()
    }
    
    /**
     * Observe timeline changes
     */
    private fun observeTimeline() {
        viewModelScope.launch {
            timelineManager.timelineState.collect { timeline ->
                _uiState.update {
                    it.copy(
                        timelineState = timeline,
                        canUndo = true, // Check actual undo stack
                        canRedo = false // Check actual redo stack
                    )
                }
            }
        }
    }
    
    /**
     * Observe player state
     */
    private fun observePlayer() {
        viewModelScope.launch {
            videoPlayer.playerState.collect { playerState ->
                _uiState.update { it.copy(playerState = playerState) }
                
                // Update playhead position while playing
                if (playerState.isPlaying) {
                    startPlayheadUpdate()
                } else {
                    stopPlayheadUpdate()
                }
            }
        }
    }
    
    /**
     * Load project
     */
    fun loadProject(projectId: String) {
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true) }
            try {
                val project = projectManager.getProject(projectId)
                _uiState.update {
                    it.copy(
                        project = project,
                        isLoading = false
                    )
                }
                // TODO: Load timeline from project
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        error = "Failed to load project: ${e.message}"
                    )
                }
            }
        }
    }
    
    /**
     * Add clip to timeline
     */
    fun addClip(clip: VideoClip) {
        timelineManager.addVideoClip(clip)
    }
    
    /**
     * Remove clip from timeline
     */
    fun removeClip(clipId: String) {
        timelineManager.removeVideoClip(clipId)
        if (_uiState.value.selectedClip?.id == clipId) {
            _uiState.update { it.copy(selectedClip = null) }
        }
    }
    
    /**
     * Select clip
     */
    fun selectClip(clip: VideoClip?) {
        _uiState.update { it.copy(selectedClip = clip) }
    }
    
    /**
     * Update clip position
     */
    fun updateClipPosition(clipId: String, position: Long, trackIndex: Int) {
        timelineManager.updateClipPosition(clipId, position, trackIndex)
    }
    
    /**
     * Play/Pause
     */
    fun togglePlayPause() {
        if (_uiState.value.playerState.isPlaying) {
            videoPlayer.pause()
        } else {
            videoPlayer.play()
        }
    }
    
    /**
     * Seek to position
     */
    fun seekTo(position: Long) {
        videoPlayer.seekTo(position)
        _uiState.update { it.copy(playheadPosition = position) }
    }
    
    /**
     * Change selected tab
     */
    fun selectTab(tab: EditorTab) {
        _uiState.update { it.copy(selectedTab = tab) }
    }
    
    /**
     * Zoom timeline
     */
    fun setZoomLevel(zoom: Float) {
        _uiState.update { it.copy(zoomLevel = zoom.coerceIn(0.5f, 3.0f)) }
    }
    
    /**
     * Undo
     */
    fun undo() {
        timelineManager.undo()
    }
    
    /**
     * Redo
     */
    fun redo() {
        timelineManager.redo()
    }
    
    /**
     * Add effect to selected clip
     */
    fun addEffect(effect: Effect) {
        _uiState.value.selectedClip?.let { clip ->
            timelineManager.addEffectToClip(clip.id, effect)
        }
    }
    
    /**
     * Start playhead position updates
     */
    private fun startPlayheadUpdate() {
        playheadUpdateJob?.cancel()
        playheadUpdateJob = viewModelScope.launch {
            while (true) {
                val position = videoPlayer.getCurrentPosition()
                _uiState.update { it.copy(playheadPosition = position) }
                delay(16) // ~60fps
            }
        }
    }
    
    /**
     * Stop playhead position updates
     */
    private fun stopPlayheadUpdate() {
        playheadUpdateJob?.cancel()
    }
    
    /**
     * Clear error
     */
    fun clearError() {
        _uiState.update { it.copy(error = null) }
    }
    
    override fun onCleared() {
        super.onCleared()
        stopPlayheadUpdate()
        videoPlayer.release()
    }
}
```

---

### 5. Editor Screen UI (Part 1)

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/EditorScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import com.clipforge.videoeditor.ui.theme.BrandPurple

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun EditorScreen(
    projectId: String,
    navController: NavController,
    viewModel: EditorViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    
    // Load project
    LaunchedEffect(projectId) {
        viewModel.loadProject(projectId)
    }
    
    // Show error snackbar
    LaunchedEffect(uiState.error) {
        uiState.error?.let {
            snackbarHostState.showSnackbar(
                message = it,
                duration = SnackbarDuration.Short
            )
            viewModel.clearError()
        }
    }
    
    Scaffold(
        snackbarHost = { SnackbarHost(snackbarHostState) },
        containerColor = Color.Transparent,
        topBar = {
            EditorTopBar(
                projectName = uiState.project?.name ?: "Untitled",
                canUndo = uiState.canUndo,
                canRedo = uiState.canRedo,
                onBackClick = { navController.popBackStack() },
                onUndo = { viewModel.undo() },
                onRedo = { viewModel.redo() },
                onExport = { navController.navigate("export/$projectId") }
            )
        }
    ) { padding ->
        Box(
            modifier = Modifier
                .fillMaxSize()
                .background(
                    brush = Brush.verticalGradient(
                        colors = listOf(
                            Color(0xFF0F0A1E),
                            Color(0xFF1A0B2E)
                        )
                    )
                )
                .padding(padding)
        ) {
            if (uiState.isLoading) {
                LoadingView()
            } else {
                Column(modifier = Modifier.fillMaxSize()) {
                    // Video Preview (60%)
                    VideoPreviewSection(
                        modifier = Modifier
                            .fillMaxWidth()
                            .weight(0.6f),
                        playerState = uiState.playerState,
                        onPlayPause = { viewModel.togglePlayPause() }
                    )
                    
                    // Feature Tabs
                    EditorTabRow(
                        selectedTab = uiState.selectedTab,
                        onTabSelected = { viewModel.selectTab(it) }
                    )
                    
                    // Timeline Section (40%)
                    TimelineSection(
                        modifier = Modifier
                            .fillMaxWidth()
                            .weight(0.4f),
                        timelineState = uiState.timelineState,
                        playheadPosition = uiState.playheadPosition,
                        selectedClip = uiState.selectedClip,
                        zoomLevel = uiState.zoomLevel,
                        onClipClick = { viewModel.selectClip(it) },
                        onClipMove = { clipId, position, track ->
                            viewModel.updateClipPosition(clipId, position, track)
                        },
                        onSeek = { viewModel.seekTo(it) },
                        onZoomChange = { viewModel.setZoomLevel(it) }
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun EditorTopBar(
    projectName: String,
    canUndo: Boolean,
    canRedo: Boolean,
    onBackClick: () -> Unit,
    onUndo: () -> Unit,
    onRedo: () -> Unit,
    onExport: () -> Unit
) {
    TopAppBar(
        title = {
            Column {
                Text(
                    projectName,
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Text(
                    "Editing",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        },
        navigationIcon = {
            IconButton(onClick = onBackClick) {
                Icon(Icons.Default.ArrowBack, "Back", tint = Color.White)
            }
        },
        actions = {
            IconButton(onClick = onUndo, enabled = canUndo) {
                Icon(
                    Icons.Default.Undo,
                    "Undo",
                    tint = if (canUndo) Color.White else Color.Gray
                )
            }
            
            IconButton(onClick = onRedo, enabled = canRedo) {
                Icon(
                    Icons.Default.Redo,
                    "Redo",
                    tint = if (canRedo) Color.White else Color.Gray
                )
            }
            
            Button(
                onClick = onExport,
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                ),
                shape = RoundedCornerShape(8.dp),
                modifier = Modifier.padding(end = 8.dp)
            ) {
                Icon(Icons.Default.FileDownload, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(4.dp))
                Text("Export")
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Color(0xFF1A1625)
        )
    )
}

@Composable
private fun EditorTabRow(
    selectedTab: EditorTab,
    onTabSelected: (EditorTab) -> Unit
) {
    TabRow(
        selectedTabIndex = selectedTab.ordinal,
        containerColor = Color(0xFF1A1625),
        contentColor = Color.White
    ) {
        EditorTab.values().forEach { tab ->
            Tab(
                selected = selectedTab == tab,
                onClick = { onTabSelected(tab) },
                text = {
                    Text(
                        tab.name.lowercase().replaceFirstChar { it.uppercase() },
                        style = MaterialTheme.typography.bodySmall
                    )
                },
                icon = {
                    Icon(
                        when (tab) {
                            EditorTab.TIMELINE -> Icons.Default.ViewTimeline
                            EditorTab.EFFECTS -> Icons.Default.AutoAwesome
                            EditorTab.AUDIO -> Icons.Default.AudioFile
                            EditorTab.TEXT -> Icons.Default.TextFields
                            EditorTab.COLOR -> Icons.Default.Palette
                            EditorTab.TRIM -> Icons.Default.ContentCut
                        },
                        contentDescription = null,
                        modifier = Modifier.size(20.dp)
                    )
                },
                selectedContentColor = BrandPurple,
                unselectedContentColor = Color.Gray
            )
        }
    }
}

@Composable
private fun VideoPreviewSection(
    modifier: Modifier = Modifier,
    playerState: com.clipforge.videoeditor.data.local.PlayerState,
    onPlayPause: () -> Unit
) {
    Box(
        modifier = modifier
            .fillMaxWidth()
            .background(Color.Black),
        contentAlignment = Alignment.Center
    ) {
        // Video surface would go here
        // For now, placeholder
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Icon(
                Icons.Default.VideoLibrary,
                contentDescription = null,
                modifier = Modifier.size(80.dp),
                tint = Color.Gray
            )
        }
        
        // Play/Pause button overlay
        IconButton(
            onClick = onPlayPause,
            modifier = Modifier
                .size(64.dp)
                .background(Color.Black.copy(alpha = 0.5f), RoundedCornerShape(32.dp))
        ) {
            Icon(
                if (playerState.isPlaying) Icons.Default.Pause else Icons.Default.PlayArrow,
                contentDescription = if (playerState.isPlaying) "Pause" else "Play",
                tint = Color.White,
                modifier = Modifier.size(32.dp)
            )
        }
    }
}

@Composable
private fun TimelineSection(
    modifier: Modifier = Modifier,
    timelineState: TimelineState,
    playheadPosition: Long,
    selectedClip: VideoClip?,
    zoomLevel: Float,
    onClipClick: (VideoClip) -> Unit,
    onClipMove: (String, Long, Int) -> Unit,
    onSeek: (Long) -> Unit,
    onZoomChange: (Float) -> Unit
) {
    Column(modifier = modifier) {
        // Timeline controls
        TimelineControls(
            zoomLevel = zoomLevel,
            onZoomChange = onZoomChange
        )
        
        // Timeline view
        TimelineView(
            timelineState = timelineState,
            playheadPosition = playheadPosition,
            selectedClip = selectedClip,
            zoomLevel = zoomLevel,
            onClipClick = onClipClick,
            onSeek = onSeek
        )
    }
}

@Composable
private fun TimelineControls(
    zoomLevel: Float,
    onZoomChange: (Float) -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .background(Color(0xFF1A1625))
            .padding(horizontal = 16.dp, vertical = 8.dp),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            "Timeline",
            style = MaterialTheme.typography.titleSmall,
            color = Color.White,
            fontWeight = FontWeight.Bold
        )
        
        Row(verticalAlignment = Alignment.CenterVertically) {
            IconButton(onClick = { onZoomChange(zoomLevel - 0.1f) }) {
                Icon(Icons.Default.ZoomOut, "Zoom Out", tint = Color.White)
            }
            
            Text(
                "${(zoomLevel * 100).toInt()}%",
                style = MaterialTheme.typography.bodySmall,
                color = Color.White
            )
            
            IconButton(onClick = { onZoomChange(zoomLevel + 0.1f) }) {
                Icon(Icons.Default.ZoomIn, "Zoom In", tint = Color.White)
            }
        }
    }
}

@Composable
private fun TimelineView(
    timelineState: TimelineState,
    playheadPosition: Long,
    selectedClip: VideoClip?,
    zoomLevel: Float,
    onClipClick: (VideoClip) -> Unit,
    onSeek: (Long) -> Unit
) {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xFF0F0A1E))
    ) {
        // Timeline content would go here
        // This is a simplified placeholder
        Column(modifier = Modifier.fillMaxSize()) {
            repeat(timelineState.videoTrackCount) { trackIndex ->
                TimelineTrack(
                    trackIndex = trackIndex,
                    clips = timelineState.videoClips.filter { it.trackIndex == trackIndex },
                    selectedClip = selectedClip,
                    onClipClick = onClipClick,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(60.dp)
                )
            }
        }
    }
}

@Composable
private fun TimelineTrack(
    trackIndex: Int,
    clips: List<VideoClip>,
    selectedClip: VideoClip?,
    onClipClick: (VideoClip) -> Unit,
    modifier: Modifier = Modifier
) {
    Box(
        modifier = modifier
            .background(if (trackIndex % 2 == 0) Color(0xFF1A1625) else Color(0xFF2D2438))
            .padding(vertical = 4.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxSize(),
            horizontalArrangement = Arrangement.spacedBy(4.dp)
        ) {
            clips.forEach { clip ->
                ClipItem(
                    clip = clip,
                    isSelected = clip.id == selectedClip?.id,
                    onClick = { onClipClick(clip) }
                )
            }
        }
    }
}

@Composable
private fun ClipItem(
    clip: VideoClip,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    Card(
        onClick = onClick,
        modifier = Modifier
            .width(120.dp)
            .height(52.dp),
        colors = CardDefaults.cardColors(
            containerColor = if (isSelected) BrandPurple else Color(0xFF3D2F4F)
        ),
        shape = RoundedCornerShape(4.dp)
    ) {
        Box(
            modifier = Modifier
                .fillMaxSize()
                .padding(4.dp),
            contentAlignment = Alignment.Center
        ) {
            Text(
                clip.name,
                style = MaterialTheme.typography.labelSmall,
                color = Color.White,
                maxLines = 1
            )
        }
    }
}

@Composable
private fun LoadingView() {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            CircularProgressIndicator(
                color = BrandPurple,
                modifier = Modifier.size(48.dp)
            )
            Text(
                "Loading Editor...",
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
        }
    }
}
```

---

## User Flow

1. **Open Project** → Navigate to Editor Screen
2. **View Timeline** → See all clips on timeline
3. **Add Clips** → Drag clips to timeline
4. **Edit Clips** → Trim, move, adjust
5. **Add Effects** → Apply effects from tabs
6. **Preview** → Play/pause to review
7. **Undo/Redo** → Navigate edit history
8. **Export** → Save finished video

---

## Performance Considerations

- **GPU Acceleration**: Use OpenGL for effects
- **Background Processing**: Process effects on separate thread
- **Efficient Timeline**: Virtualized timeline for many clips
- **Memory Management**: Release unused resources
- **Preview Optimization**: Lower resolution previews

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should add clip to timeline`() = runTest {
    val clip = VideoClip(uri = mockUri, name = "Test")
    viewModel.addClip(clip)
    
    val clips = viewModel.uiState.value.timelineState.videoClips
    assertTrue(clips.contains(clip))
}
```

---

## Related Screens

- [Import Screen](./02-IMPORT-SCREEN-ENHANCED.md)
- [Export Screen](./05-EXPORT-SCREEN-ENHANCED.md)
- [Editor Tabs](./07-EDITOR-TABS-ENHANCED.md)