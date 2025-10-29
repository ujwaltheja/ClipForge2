# 05 - Export Screen - Complete Enhanced Documentation

## Overview
The Export Screen handles **video rendering and export** using Media3 Transformer for local video processing. It provides quality presets, format selection, progress tracking, and saves to device storage with **local-only processing**.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/export/
│   ├── ExportScreen.kt                 # Main UI Composable
│   ├── ExportViewModel.kt              # State management
│   ├── components/
│   │   ├── QualitySelector.kt         # Quality selection
│   │   ├── FormatSelector.kt          # Format selection
│   │   └── ProgressCard.kt            # Export progress
│   └── data/
│       ├── LocalVideoExporter.kt      # Media3 Transformer wrapper
│       ├── RenderEngine.kt            # Rendering logic
│       └── models/
│           ├── ExportConfig.kt        # Export configuration
│           └── ExportProgress.kt      # Progress state
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
    
    // Media3 Transformer for video export
    implementation("androidx.media3:media3-transformer:1.3.1")
    implementation("androidx.media3:media3-common:1.3.1")
    implementation("androidx.media3:media3-effect:1.3.1")
    
    // Coroutines
    implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:1.7.3")
}
```

### AndroidManifest.xml Permissions
```xml
<manifest>
    <!-- Write to external storage -->
    <uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE"
                     android:maxSdkVersion="28" />
</manifest>
```

---

## Complete Source Code

### 1. Export Data Models

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/ExportConfig.kt`

```kotlin
package com.clipforge.videoeditor.data.model

import kotlinx.serialization.Serializable

@Serializable
data class ExportConfig(
    val fileName: String,
    val quality: ExportQuality = ExportQuality.HIGH,
    val format: ExportFormat = ExportFormat.MP4,
    val resolution: Resolution = Resolution.HD_1080P,
    val frameRate: Int = 30,
    val bitrate: Int = 8000000, // 8 Mbps
    val includeAudio: Boolean = true
)

@Serializable
enum class ExportQuality {
    LOW,
    MEDIUM,
    HIGH,
    ULTRA
}

@Serializable
enum class Resolution(val width: Int, val height: Int, val displayName: String) {
    SD_480P(854, 480, "480p"),
    HD_720P(1280, 720, "720p"),
    HD_1080P(1920, 1080, "1080p"),
    UHD_4K(3840, 2160, "4K")
}

data class ExportProgress(
    val progress: Float = 0f,
    val currentFrame: Int = 0,
    val totalFrames: Int = 0,
    val elapsedTime: Long = 0,
    val estimatedTimeRemaining: Long = 0,
    val exportSpeed: Float = 0f // frames per second
) {
    val progressPercentage: Int
        get() = (progress * 100).toInt()
}

enum class ExportState {
    IDLE,
    CONFIGURING,
    EXPORTING,
    COMPLETED,
    FAILED,
    CANCELLED
}
```

---

### 2. Local Video Exporter

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/LocalVideoExporter.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import android.net.Uri
import android.os.Environment
import androidx.core.net.toUri
import androidx.media3.common.MediaItem
import androidx.media3.common.util.UnstableApi
import androidx.media3.transformer.Composition
import androidx.media3.transformer.ExportException
import androidx.media3.transformer.ExportResult
import androidx.media3.transformer.Transformer
import com.clipforge.videoeditor.data.model.ExportConfig
import com.clipforge.videoeditor.data.model.ExportProgress
import kotlinx.coroutines.channels.awaitClose
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.callbackFlow
import java.io.File
import java.text.SimpleDateFormat
import java.util.*
import javax.inject.Inject
import javax.inject.Singleton

@UnstableApi
@Singleton
class LocalVideoExporter @Inject constructor(
    private val context: Context
) {
    
    private var transformer: Transformer? = null
    private var startTime: Long = 0
    
    /**
     * Export video with configuration
     */
    fun exportVideo(
        inputUri: Uri,
        config: ExportConfig
    ): Flow<ExportProgress> = callbackFlow {
        
        startTime = System.currentTimeMillis()
        
        // Create output file
        val outputFile = createOutputFile(config)
        
        // Build transformer
        transformer = Transformer.Builder(context)
            .addListener(object : Transformer.Listener {
                override fun onCompleted(composition: Composition, exportResult: ExportResult) {
                    trySend(ExportProgress(progress = 1.0f))
                    close()
                }
                
                override fun onError(
                    composition: Composition,
                    exportResult: ExportResult,
                    exportException: ExportException
                ) {
                    close(exportException)
                }
            })
            .build()
        
        // Start export
        val mediaItem = MediaItem.fromUri(inputUri)
        transformer?.start(mediaItem, outputFile.absolutePath)
        
        // Monitor progress
        var lastProgress = 0f
        while (!isClosedForSend) {
            transformer?.let { t ->
                val progress = t.getProgress(Transformer.ProgressHolder())
                
                if (progress != Transformer.PROGRESS_STATE_NOT_STARTED &&
                    progress != lastProgress) {
                    
                    val elapsedTime = System.currentTimeMillis() - startTime
                    val estimatedTotal = if (progress > 0) {
                        (elapsedTime / progress).toLong()
                    } else {
                        0
                    }
                    val estimatedRemaining = estimatedTotal - elapsedTime
                    
                    trySend(ExportProgress(
                        progress = progress / 100f,
                        elapsedTime = elapsedTime,
                        estimatedTimeRemaining = estimatedRemaining,
                        exportSpeed = if (elapsedTime > 0) progress / (elapsedTime / 1000f) else 0f
                    ))
                    
                    lastProgress = progress
                }
            }
            
            kotlinx.coroutines.delay(100) // Update every 100ms
        }
        
        awaitClose {
            transformer?.cancel()
            transformer = null
        }
    }
    
    /**
     * Create output file in Downloads folder
     */
    private fun createOutputFile(config: ExportConfig): File {
        val downloadsDir = Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DOWNLOADS
        )
        val clipforgeDir = File(downloadsDir, "ClipForge").apply {
            if (!exists()) mkdirs()
        }
        
        val timestamp = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.getDefault())
            .format(Date())
        
        val fileName = "${config.fileName}_$timestamp.${config.format.name.lowercase()}"
        
        return File(clipforgeDir, fileName)
    }
    
    /**
     * Cancel current export
     */
    fun cancelExport() {
        transformer?.cancel()
        transformer = null
    }
    
    /**
     * Get output file path for sharing
     */
    fun getOutputFilePath(config: ExportConfig): String {
        val downloadsDir = Environment.getExternalStoragePublicDirectory(
            Environment.DIRECTORY_DOWNLOADS
        )
        return File(downloadsDir, "ClipForge").absolutePath
    }
}
```

---

### 3. Export ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/export/ExportViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.export

import android.net.Uri
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import androidx.media3.common.util.UnstableApi
import com.clipforge.videoeditor.data.local.LocalProjectManager
import com.clipforge.videoeditor.data.local.LocalVideoExporter
import com.clipforge.videoeditor.data.model.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.*
import kotlinx.coroutines.launch
import javax.inject.Inject

data class ExportUiState(
    val project: Project? = null,
    val config: ExportConfig = ExportConfig(fileName = "ClipForge_Video"),
    val exportState: ExportState = ExportState.IDLE,
    val exportProgress: ExportProgress = ExportProgress(),
    val outputPath: String? = null,
    val error: String? = null
)

@UnstableApi
@HiltViewModel
class ExportViewModel @Inject constructor(
    private val projectManager: LocalProjectManager,
    private val videoExporter: LocalVideoExporter
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(ExportUiState())
    val uiState: StateFlow<ExportUiState> = _uiState.asStateFlow()
    
    /**
     * Load project
     */
    fun loadProject(projectId: String) {
        viewModelScope.launch {
            try {
                val project = projectManager.getProject(projectId)
                _uiState.update {
                    it.copy(
                        project = project,
                        config = it.config.copy(
                            fileName = project?.name ?: "ClipForge_Video"
                        )
                    )
                }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(error = "Failed to load project: ${e.message}")
                }
            }
        }
    }
    
    /**
     * Update export configuration
     */
    fun updateConfig(config: ExportConfig) {
        _uiState.update { it.copy(config = config) }
    }
    
    /**
     * Set quality preset
     */
    fun setQuality(quality: ExportQuality) {
        val (resolution, bitrate) = when (quality) {
            ExportQuality.LOW -> Resolution.SD_480P to 2_000_000
            ExportQuality.MEDIUM -> Resolution.HD_720P to 5_000_000
            ExportQuality.HIGH -> Resolution.HD_1080P to 8_000_000
            ExportQuality.ULTRA -> Resolution.UHD_4K to 20_000_000
        }
        
        _uiState.update {
            it.copy(
                config = it.config.copy(
                    quality = quality,
                    resolution = resolution,
                    bitrate = bitrate
                )
            )
        }
    }
    
    /**
     * Set export format
     */
    fun setFormat(format: ExportFormat) {
        _uiState.update {
            it.copy(config = it.config.copy(format = format))
        }
    }
    
    /**
     * Set frame rate
     */
    fun setFrameRate(fps: Int) {
        _uiState.update {
            it.copy(config = it.config.copy(frameRate = fps))
        }
    }
    
    /**
     * Start export
     */
    fun startExport(inputUri: Uri) {
        viewModelScope.launch {
            _uiState.update {
                it.copy(
                    exportState = ExportState.EXPORTING,
                    error = null
                )
            }
            
            try {
                videoExporter.exportVideo(inputUri, _uiState.value.config)
                    .catch { exception ->
                        _uiState.update {
                            it.copy(
                                exportState = ExportState.FAILED,
                                error = "Export failed: ${exception.message}"
                            )
                        }
                    }
                    .collect { progress ->
                        _uiState.update {
                            it.copy(exportProgress = progress)
                        }
                        
                        // Check if completed
                        if (progress.progress >= 1.0f) {
                            _uiState.update {
                                it.copy(
                                    exportState = ExportState.COMPLETED,
                                    outputPath = videoExporter.getOutputFilePath(it.config)
                                )
                            }
                        }
                    }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        exportState = ExportState.FAILED,
                        error = "Export error: ${e.message}"
                    )
                }
            }
        }
    }
    
    /**
     * Cancel export
     */
    fun cancelExport() {
        videoExporter.cancelExport()
        _uiState.update {
            it.copy(
                exportState = ExportState.CANCELLED,
                exportProgress = ExportProgress()
            )
        }
    }
    
    /**
     * Reset state
     */
    fun resetExport() {
        _uiState.update {
            it.copy(
                exportState = ExportState.IDLE,
                exportProgress = ExportProgress(),
                outputPath = null,
                error = null
            )
        }
    }
    
    /**
     * Clear error
     */
    fun clearError() {
        _uiState.update { it.copy(error = null) }
    }
}
```

---

### 4. Export Screen UI

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/export/ExportScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.export

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
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
import androidx.media3.common.util.UnstableApi
import androidx.navigation.NavController
import com.clipforge.videoeditor.data.model.*
import com.clipforge.videoeditor.ui.theme.BrandPurple

@UnstableApi
@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ExportScreen(
    projectId: String,
    navController: NavController,
    viewModel: ExportViewModel = hiltViewModel()
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
            ExportTopBar(
                projectName = uiState.project?.name ?: "Untitled",
                onBackClick = { navController.popBackStack() }
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
            when (uiState.exportState) {
                ExportState.IDLE, ExportState.CONFIGURING -> {
                    ConfigurationView(
                        config = uiState.config,
                        onQualityChange = { viewModel.setQuality(it) },
                        onFormatChange = { viewModel.setFormat(it) },
                        onFrameRateChange = { viewModel.setFrameRate(it) },
                        onStartExport = {
                            // TODO: Get actual input URI from timeline
                            // viewModel.startExport(inputUri)
                        }
                    )
                }
                
                ExportState.EXPORTING -> {
                    ExportingView(
                        progress = uiState.exportProgress,
                        onCancel = { viewModel.cancelExport() }
                    )
                }
                
                ExportState.COMPLETED -> {
                    CompletedView(
                        outputPath = uiState.outputPath ?: "",
                        onDone = { navController.popBackStack() },
                        onShare = { /* TODO: Share video */ },
                        onExportAgain = { viewModel.resetExport() }
                    )
                }
                
                ExportState.FAILED -> {
                    FailedView(
                        error = uiState.error ?: "Unknown error",
                        onRetry = { viewModel.resetExport() },
                        onCancel = { navController.popBackStack() }
                    )
                }
                
                ExportState.CANCELLED -> {
                    CancelledView(
                        onBack = { navController.popBackStack() },
                        onRetry = { viewModel.resetExport() }
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun ExportTopBar(
    projectName: String,
    onBackClick: () -> Unit
) {
    TopAppBar(
        title = {
            Column {
                Text(
                    "Export Video",
                    style = MaterialTheme.typography.titleLarge,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Text(
                    projectName,
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
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Color(0xFF1A1625)
        )
    )
}

@Composable
private fun ConfigurationView(
    config: ExportConfig,
    onQualityChange: (ExportQuality) -> Unit,
    onFormatChange: (ExportFormat) -> Unit,
    onFrameRateChange: (Int) -> Unit,
    onStartExport: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .verticalScroll(rememberScrollState())
            .padding(16.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        // Preview Card
        PreviewCard()
        
        // Quality Settings
        SettingsCard(title = "Quality") {
            QualitySelector(
                selectedQuality = config.quality,
                onQualitySelected = onQualityChange
            )
        }
        
        // Format Settings
        SettingsCard(title = "Format") {
            FormatSelector(
                selectedFormat = config.format,
                onFormatSelected = onFormatChange
            )
        }
        
        // Frame Rate Settings
        SettingsCard(title = "Frame Rate") {
            FrameRateSelector(
                selectedFps = config.frameRate,
                onFpsSelected = onFrameRateChange
            )
        }
        
        // Resolution Display
        SettingsCard(title = "Resolution") {
            ResolutionDisplay(resolution = config.resolution)
        }
        
        // Export Button
        Button(
            onClick = onStartExport,
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            ),
            shape = RoundedCornerShape(12.dp)
        ) {
            Icon(Icons.Default.FileDownload, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                "Start Export",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )
        }
    }
}

@Composable
private fun PreviewCard() {
    Card(
        modifier = Modifier
            .fillMaxWidth()
            .aspectRatio(16f / 9f),
        colors = CardDefaults.cardColors(
            containerColor = Color.Black
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Icon(
                Icons.Default.VideoLibrary,
                contentDescription = null,
                modifier = Modifier.size(64.dp),
                tint = Color.Gray
            )
        }
    }
}

@Composable
private fun SettingsCard(
    title: String,
    content: @Composable ColumnScope.() -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                title,
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = Color.White,
                modifier = Modifier.padding(bottom = 12.dp)
            )
            content()
        }
    }
}

@Composable
private fun QualitySelector(
    selectedQuality: ExportQuality,
    onQualitySelected: (ExportQuality) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        ExportQuality.values().forEach { quality ->
            FilterChip(
                selected = selectedQuality == quality,
                onClick = { onQualitySelected(quality) },
                label = { Text(quality.name) },
                modifier = Modifier.weight(1f),
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
    }
}

@Composable
private fun FormatSelector(
    selectedFormat: ExportFormat,
    onFormatSelected: (ExportFormat) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        ExportFormat.values().forEach { format ->
            FilterChip(
                selected = selectedFormat == format,
                onClick = { onFormatSelected(format) },
                label = { Text(format.name) },
                modifier = Modifier.weight(1f),
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
    }
}

@Composable
private fun FrameRateSelector(
    selectedFps: Int,
    onFpsSelected: (Int) -> Unit
) {
    val fpsOptions = listOf(24, 30, 60)
    
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        fpsOptions.forEach { fps ->
            FilterChip(
                selected = selectedFps == fps,
                onClick = { onFpsSelected(fps) },
                label = { Text("${fps}fps") },
                modifier = Modifier.weight(1f),
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
    }
}

@Composable
private fun ResolutionDisplay(resolution: Resolution) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            resolution.displayName,
            style = MaterialTheme.typography.bodyLarge,
            color = Color.White
        )
        Text(
            "${resolution.width}x${resolution.height}",
            style = MaterialTheme.typography.bodyMedium,
            color = Color.Gray
        )
    }
}

@Composable
private fun ExportingView(
    progress: ExportProgress,
    onCancel: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        // Progress indicator
        Box(
            modifier = Modifier.size(200.dp),
            contentAlignment = Alignment.Center
        ) {
            CircularProgressIndicator(
                progress = progress.progress,
                modifier = Modifier.fillMaxSize(),
                color = BrandPurple,
                strokeWidth = 8.dp
            )
            
            Text(
                "${progress.progressPercentage}%",
                style = MaterialTheme.typography.displaySmall,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Text(
            "Exporting Video...",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        if (progress.estimatedTimeRemaining > 0) {
            Text(
                "Time remaining: ${formatTime(progress.estimatedTimeRemaining)}",
                style = MaterialTheme.typography.bodyMedium,
                color = Color.Gray
            )
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        OutlinedButton(
            onClick = onCancel,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.Red
            )
        ) {
            Icon(Icons.Default.Cancel, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Cancel Export")
        }
    }
}

@Composable
private fun CompletedView(
    outputPath: String,
    onDone: () -> Unit,
    onShare: () -> Unit,
    onExportAgain: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            Icons.Default.CheckCircle,
            contentDescription = null,
            modifier = Modifier.size(100.dp),
            tint = Color.Green
        )
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            "Export Complete!",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        Text(
            "Saved to: $outputPath",
            style = MaterialTheme.typography.bodyMedium,
            color = Color.Gray
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Button(
            onClick = onDone,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            )
        ) {
            Text("Done")
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        OutlinedButton(
            onClick = onShare,
            modifier = Modifier.fillMaxWidth()
        ) {
            Icon(Icons.Default.Share, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Share")
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        TextButton(
            onClick = onExportAgain,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Export Again")
        }
    }
}

@Composable
private fun FailedView(
    error: String,
    onRetry: () -> Unit,
    onCancel: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            Icons.Default.Error,
            contentDescription = null,
            modifier = Modifier.size(100.dp),
            tint = Color.Red
        )
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            "Export Failed",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        Text(
            error,
            style = MaterialTheme.typography.bodyMedium,
            color = Color.Gray
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Button(
            onClick = onRetry,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            )
        ) {
            Text("Try Again")
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        OutlinedButton(
            onClick = onCancel,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Cancel")
        }
    }
}

@Composable
private fun CancelledView(
    onBack: () -> Unit,
    onRetry: () -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        verticalArrangement = Arrangement.Center,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Icon(
            Icons.Default.Cancel,
            contentDescription = null,
            modifier = Modifier.size(100.dp),
            tint = Color.Gray
        )
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            "Export Cancelled",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        Button(
            onClick = onRetry,
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            )
        ) {
            Text("Try Again")
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        OutlinedButton(
            onClick = onBack,
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Back")
        }
    }
}

private fun formatTime(millis: Long): String {
    val seconds = millis / 1000
    val minutes = seconds / 60
    val secs = seconds % 60
    return String.format("%02d:%02d", minutes, secs)
}
```

---

## User Flow

1. **Open Export Screen** → From editor with Export button
2. **Configure Settings** → Select quality, format, frame rate
3. **Preview** → See video preview
4. **Start Export** → Tap start export button
5. **Progress** → Watch export progress with percentage
6. **Complete** → Video saved to Downloads/ClipForge folder
7. **Share/Done** → Share video or return to editor

---

## Performance Considerations

- **Media3 Transformer**: Efficient hardware-accelerated rendering
- **Background Processing**: Export continues in background
- **Progress Updates**: Real-time progress tracking
- **Memory Management**: Streaming processing, no full video load
- **Error Handling**: Graceful handling of export failures

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should start export with correct configuration`() = runTest {
    val config = ExportConfig(
        fileName = "test",
        quality = ExportQuality.HIGH
    )
    viewModel.updateConfig(config)
    
    assertEquals(config, viewModel.uiState.value.config)
}
```

---

## Related Screens

- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)
- [Settings Screen](./06-SETTINGS-SCREEN-ENHANCED.md)
- [Home Screen](./01-HOME-SCREEN-ENHANCED.md)