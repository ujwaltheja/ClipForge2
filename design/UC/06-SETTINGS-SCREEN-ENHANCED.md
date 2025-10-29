# 06 - Settings Screen - Complete Enhanced Documentation

## Overview
The Settings Screen provides app configuration and preferences management using **SharedPreferences** for local storage. Users can configure video quality, manage storage, and customize app behavior.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/settings/
│   ├── SettingsScreen.kt              # Main UI Composable
│   ├── SettingsViewModel.kt           # State management
│   ├── components/
│   │   ├── SettingsSection.kt        # Section component
│   │   ├── SettingItem.kt            # Individual setting item
│   │   └── StorageCard.kt            # Storage info card
│   └── data/
│       ├── PreferencesManager.kt     # SharedPreferences wrapper
│       ├── StorageManager.kt         # Storage utilities
│       └── models/AppSettings.kt     # Settings data model
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
    
    // DataStore Preferences (modern alternative to SharedPreferences)
    implementation("androidx.datastore:datastore-preferences:1.0.0")
}
```

---

## Complete Source Code

### 1. Settings Data Model

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/AppSettings.kt`

```kotlin
package com.clipforge.videoeditor.data.model

data class AppSettings(
    // Video Settings
    val defaultVideoQuality: VideoQuality = VideoQuality.HIGH,
    val defaultFrameRate: Int = 30,
    val defaultAspectRatio: AspectRatio = AspectRatio.PORTRAIT,
    
    // Export Settings
    val defaultExportFormat: ExportFormat = ExportFormat.MP4,
    val autoSaveProjects: Boolean = true,
    val exportLocation: ExportLocation = ExportLocation.DOWNLOADS,
    
    // Performance Settings
    val enableGPUAcceleration: Boolean = true,
    val generateProxies: Boolean = true,
    val maxMemoryUsage: MemoryUsage = MemoryUsage.MEDIUM,
    
    // UI Settings
    val theme: AppTheme = AppTheme.DARK,
    val language: String = "en",
    val showTutorials: Boolean = true,
    
    // Storage Settings
    val autoClearCache: Boolean = true,
    val cacheRetentionDays: Int = 7,
    val maxCacheSize: Long = 500 * 1024 * 1024 // 500 MB
)

enum class VideoQuality {
    LOW,      // 480p
    MEDIUM,   // 720p
    HIGH,     // 1080p
    ULTRA     // 4K
}

enum class AspectRatio(val ratio: String) {
    PORTRAIT("9:16"),
    LANDSCAPE("16:9"),
    SQUARE("1:1"),
    CINEMATIC("21:9")
}

enum class ExportFormat {
    MP4,
    MOV,
    AVI
}

enum class ExportLocation {
    DOWNLOADS,
    MOVIES,
    CUSTOM
}

enum class MemoryUsage {
    LOW,
    MEDIUM,
    HIGH
}

enum class AppTheme {
    LIGHT,
    DARK,
    AUTO
}
```

---

### 2. Preferences Manager

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/PreferencesManager.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import android.content.SharedPreferences
import com.clipforge.videoeditor.data.model.*
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class PreferencesManager @Inject constructor(
    context: Context
) {
    
    private val prefs: SharedPreferences = context.getSharedPreferences(
        "clipforge_prefs",
        Context.MODE_PRIVATE
    )
    
    private val _settingsFlow = MutableStateFlow(loadSettings())
    val settingsFlow: Flow<AppSettings> = _settingsFlow.asStateFlow()
    
    companion object {
        // Keys
        private const val KEY_VIDEO_QUALITY = "video_quality"
        private const val KEY_FRAME_RATE = "frame_rate"
        private const val KEY_ASPECT_RATIO = "aspect_ratio"
        private const val KEY_EXPORT_FORMAT = "export_format"
        private const val KEY_AUTO_SAVE = "auto_save"
        private const val KEY_EXPORT_LOCATION = "export_location"
        private const val KEY_GPU_ACCELERATION = "gpu_acceleration"
        private const val KEY_GENERATE_PROXIES = "generate_proxies"
        private const val KEY_MEMORY_USAGE = "memory_usage"
        private const val KEY_THEME = "theme"
        private const val KEY_LANGUAGE = "language"
        private const val KEY_SHOW_TUTORIALS = "show_tutorials"
        private const val KEY_AUTO_CLEAR_CACHE = "auto_clear_cache"
        private const val KEY_CACHE_RETENTION = "cache_retention"
        private const val KEY_MAX_CACHE_SIZE = "max_cache_size"
    }
    
    /**
     * Load all settings from SharedPreferences
     */
    private fun loadSettings(): AppSettings {
        return AppSettings(
            defaultVideoQuality = VideoQuality.valueOf(
                prefs.getString(KEY_VIDEO_QUALITY, VideoQuality.HIGH.name) ?: VideoQuality.HIGH.name
            ),
            defaultFrameRate = prefs.getInt(KEY_FRAME_RATE, 30),
            defaultAspectRatio = AspectRatio.valueOf(
                prefs.getString(KEY_ASPECT_RATIO, AspectRatio.PORTRAIT.name) ?: AspectRatio.PORTRAIT.name
            ),
            defaultExportFormat = ExportFormat.valueOf(
                prefs.getString(KEY_EXPORT_FORMAT, ExportFormat.MP4.name) ?: ExportFormat.MP4.name
            ),
            autoSaveProjects = prefs.getBoolean(KEY_AUTO_SAVE, true),
            exportLocation = ExportLocation.valueOf(
                prefs.getString(KEY_EXPORT_LOCATION, ExportLocation.DOWNLOADS.name) ?: ExportLocation.DOWNLOADS.name
            ),
            enableGPUAcceleration = prefs.getBoolean(KEY_GPU_ACCELERATION, true),
            generateProxies = prefs.getBoolean(KEY_GENERATE_PROXIES, true),
            maxMemoryUsage = MemoryUsage.valueOf(
                prefs.getString(KEY_MEMORY_USAGE, MemoryUsage.MEDIUM.name) ?: MemoryUsage.MEDIUM.name
            ),
            theme = AppTheme.valueOf(
                prefs.getString(KEY_THEME, AppTheme.DARK.name) ?: AppTheme.DARK.name
            ),
            language = prefs.getString(KEY_LANGUAGE, "en") ?: "en",
            showTutorials = prefs.getBoolean(KEY_SHOW_TUTORIALS, true),
            autoClearCache = prefs.getBoolean(KEY_AUTO_CLEAR_CACHE, true),
            cacheRetentionDays = prefs.getInt(KEY_CACHE_RETENTION, 7),
            maxCacheSize = prefs.getLong(KEY_MAX_CACHE_SIZE, 500 * 1024 * 1024)
        )
    }
    
    /**
     * Save settings to SharedPreferences
     */
    private fun saveSettings(settings: AppSettings) {
        prefs.edit().apply {
            putString(KEY_VIDEO_QUALITY, settings.defaultVideoQuality.name)
            putInt(KEY_FRAME_RATE, settings.defaultFrameRate)
            putString(KEY_ASPECT_RATIO, settings.defaultAspectRatio.name)
            putString(KEY_EXPORT_FORMAT, settings.defaultExportFormat.name)
            putBoolean(KEY_AUTO_SAVE, settings.autoSaveProjects)
            putString(KEY_EXPORT_LOCATION, settings.exportLocation.name)
            putBoolean(KEY_GPU_ACCELERATION, settings.enableGPUAcceleration)
            putBoolean(KEY_GENERATE_PROXIES, settings.generateProxies)
            putString(KEY_MEMORY_USAGE, settings.maxMemoryUsage.name)
            putString(KEY_THEME, settings.theme.name)
            putString(KEY_LANGUAGE, settings.language)
            putBoolean(KEY_SHOW_TUTORIALS, settings.showTutorials)
            putBoolean(KEY_AUTO_CLEAR_CACHE, settings.autoClearCache)
            putInt(KEY_CACHE_RETENTION, settings.cacheRetentionDays)
            putLong(KEY_MAX_CACHE_SIZE, settings.maxCacheSize)
            apply()
        }
        _settingsFlow.value = settings
    }
    
    // Individual setters
    fun setVideoQuality(quality: VideoQuality) {
        saveSettings(_settingsFlow.value.copy(defaultVideoQuality = quality))
    }
    
    fun setFrameRate(fps: Int) {
        saveSettings(_settingsFlow.value.copy(defaultFrameRate = fps))
    }
    
    fun setAspectRatio(ratio: AspectRatio) {
        saveSettings(_settingsFlow.value.copy(defaultAspectRatio = ratio))
    }
    
    fun setExportFormat(format: ExportFormat) {
        saveSettings(_settingsFlow.value.copy(defaultExportFormat = format))
    }
    
    fun setAutoSave(enabled: Boolean) {
        saveSettings(_settingsFlow.value.copy(autoSaveProjects = enabled))
    }
    
    fun setGPUAcceleration(enabled: Boolean) {
        saveSettings(_settingsFlow.value.copy(enableGPUAcceleration = enabled))
    }
    
    fun setGenerateProxies(enabled: Boolean) {
        saveSettings(_settingsFlow.value.copy(generateProxies = enabled))
    }
    
    fun setTheme(theme: AppTheme) {
        saveSettings(_settingsFlow.value.copy(theme = theme))
    }
    
    fun setShowTutorials(show: Boolean) {
        saveSettings(_settingsFlow.value.copy(showTutorials = show))
    }
    
    fun setAutoClearCache(enabled: Boolean) {
        saveSettings(_settingsFlow.value.copy(autoClearCache = enabled))
    }
    
    fun resetToDefaults() {
        saveSettings(AppSettings())
    }
}
```

---

### 3. Storage Manager

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/StorageManager.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import android.os.Build
import android.os.Environment
import android.os.StatFs
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import javax.inject.Inject
import javax.inject.Singleton

data class StorageInfo(
    val totalSpace: Long,
    val availableSpace: Long,
    val usedSpace: Long,
    val cacheSize: Long,
    val projectsSize: Long
)

@Singleton
class StorageManager @Inject constructor(
    private val context: Context
) {
    
    /**
     * Get storage information
     */
    suspend fun getStorageInfo(): StorageInfo = withContext(Dispatchers.IO) {
        val cacheSize = calculateDirectorySize(context.cacheDir)
        val projectsSize = calculateDirectorySize(File(context.filesDir, "projects"))
        
        val stat = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2) {
            StatFs(Environment.getDataDirectory().path)
        } else {
            StatFs(context.filesDir.path)
        }
        
        val totalSpace = stat.totalBytes
        val availableSpace = stat.availableBytes
        val usedSpace = totalSpace - availableSpace
        
        StorageInfo(
            totalSpace = totalSpace,
            availableSpace = availableSpace,
            usedSpace = usedSpace,
            cacheSize = cacheSize,
            projectsSize = projectsSize
        )
    }
    
    /**
     * Clear app cache
     */
    suspend fun clearCache(): Boolean = withContext(Dispatchers.IO) {
        try {
            context.cacheDir.deleteRecursively()
            context.cacheDir.mkdirs()
            true
        } catch (e: Exception) {
            e.printStackTrace()
            false
        }
    }
    
    /**
     * Clear old cache files
     */
    suspend fun clearOldCache(retentionDays: Int): Boolean = withContext(Dispatchers.IO) {
        try {
            val cutoffTime = System.currentTimeMillis() - (retentionDays * 24 * 60 * 60 * 1000L)
            
            context.cacheDir.listFiles()?.forEach { file ->
                if (file.lastModified() < cutoffTime) {
                    file.deleteRecursively()
                }
            }
            true
        } catch (e: Exception) {
            e.printStackTrace()
            false
        }
    }
    
    /**
     * Calculate directory size recursively
     */
    private fun calculateDirectorySize(directory: File): Long {
        var size: Long = 0
        
        if (directory.exists()) {
            directory.listFiles()?.forEach { file ->
                size += if (file.isDirectory) {
                    calculateDirectorySize(file)
                } else {
                    file.length()
                }
            }
        }
        
        return size
    }
    
    /**
     * Format bytes to human readable string
     */
    fun formatBytes(bytes: Long): String {
        return when {
            bytes < 1024 -> "$bytes B"
            bytes < 1024 * 1024 -> String.format("%.2f KB", bytes / 1024.0)
            bytes < 1024 * 1024 * 1024 -> String.format("%.2f MB", bytes / (1024.0 * 1024.0))
            else -> String.format("%.2f GB", bytes / (1024.0 * 1024.0 * 1024.0))
        }
    }
}
```

---

### 4. Settings ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/settings/SettingsViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.settings

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.clipforge.videoeditor.data.local.PreferencesManager
import com.clipforge.videoeditor.data.local.StorageInfo
import com.clipforge.videoeditor.data.local.StorageManager
import com.clipforge.videoeditor.data.model.*
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class SettingsUiState(
    val settings: AppSettings = AppSettings(),
    val storageInfo: StorageInfo? = null,
    val isLoading: Boolean = false,
    val error: String? = null,
    val showResetDialog: Boolean = false,
    val showClearCacheDialog: Boolean = false
)

@HiltViewModel
class SettingsViewModel @Inject constructor(
    private val preferencesManager: PreferencesManager,
    private val storageManager: StorageManager
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(SettingsUiState())
    val uiState: StateFlow<SettingsUiState> = _uiState.asStateFlow()
    
    init {
        loadSettings()
        loadStorageInfo()
    }
    
    /**
     * Load settings from preferences
     */
    private fun loadSettings() {
        viewModelScope.launch {
            preferencesManager.settingsFlow.collect { settings ->
                _uiState.update { it.copy(settings = settings) }
            }
        }
    }
    
    /**
     * Load storage information
     */
    fun loadStorageInfo() {
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true) }
            try {
                val info = storageManager.getStorageInfo()
                _uiState.update {
                    it.copy(
                        storageInfo = info,
                        isLoading = false
                    )
                }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        error = "Failed to load storage info: ${e.message}"
                    )
                }
            }
        }
    }
    
    // Video Settings
    fun setVideoQuality(quality: VideoQuality) {
        preferencesManager.setVideoQuality(quality)
    }
    
    fun setFrameRate(fps: Int) {
        preferencesManager.setFrameRate(fps)
    }
    
    fun setAspectRatio(ratio: AspectRatio) {
        preferencesManager.setAspectRatio(ratio)
    }
    
    // Export Settings
    fun setExportFormat(format: ExportFormat) {
        preferencesManager.setExportFormat(format)
    }
    
    fun setAutoSave(enabled: Boolean) {
        preferencesManager.setAutoSave(enabled)
    }
    
    // Performance Settings
    fun setGPUAcceleration(enabled: Boolean) {
        preferencesManager.setGPUAcceleration(enabled)
    }
    
    fun setGenerateProxies(enabled: Boolean) {
        preferencesManager.setGenerateProxies(enabled)
    }
    
    // UI Settings
    fun setTheme(theme: AppTheme) {
        preferencesManager.setTheme(theme)
    }
    
    fun setShowTutorials(show: Boolean) {
        preferencesManager.setShowTutorials(show)
    }
    
    // Storage Settings
    fun setAutoClearCache(enabled: Boolean) {
        preferencesManager.setAutoClearCache(enabled)
    }
    
    /**
     * Clear cache
     */
    fun clearCache() {
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true) }
            try {
                val success = storageManager.clearCache()
                if (success) {
                    loadStorageInfo()
                } else {
                    _uiState.update {
                        it.copy(
                            isLoading = false,
                            error = "Failed to clear cache"
                        )
                    }
                }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        error = "Error clearing cache: ${e.message}"
                    )
                }
            }
        }
    }
    
    /**
     * Reset settings to defaults
     */
    fun resetToDefaults() {
        preferencesManager.resetToDefaults()
        _uiState.update { it.copy(showResetDialog = false) }
    }
    
    /**
     * Show/hide dialogs
     */
    fun showResetDialog(show: Boolean) {
        _uiState.update { it.copy(showResetDialog = show) }
    }
    
    fun showClearCacheDialog(show: Boolean) {
        _uiState.update { it.copy(showClearCacheDialog = show) }
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

### 5. Settings Screen UI

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/settings/SettingsScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.settings

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import com.clipforge.videoeditor.data.model.*
import com.clipforge.videoeditor.ui.theme.BrandPurple

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun SettingsScreen(
    navController: NavController,
    viewModel: SettingsViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    
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
            SettingsTopBar(
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
            LazyColumn(
                contentPadding = PaddingValues(16.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                // Storage Info Card
                item {
                    uiState.storageInfo?.let { storageInfo ->
                        StorageInfoCard(
                            storageInfo = storageInfo,
                            onClearCache = { viewModel.showClearCacheDialog(true) },
                            formatBytes = { /* Use StorageManager.formatBytes */ "" }
                        )
                    }
                }
                
                // Video Settings Section
                item {
                    SettingsSection(title = "Video Settings") {
                        SettingDropdown(
                            icon = Icons.Default.VideoSettings,
                            title = "Default Quality",
                            options = VideoQuality.values().map { it.name },
                            selectedOption = uiState.settings.defaultVideoQuality.name,
                            onOptionSelected = {
                                viewModel.setVideoQuality(VideoQuality.valueOf(it))
                            }
                        )
                        
                        SettingDropdown(
                            icon = Icons.Default.AspectRatio,
                            title = "Aspect Ratio",
                            options = AspectRatio.values().map { it.ratio },
                            selectedOption = uiState.settings.defaultAspectRatio.ratio,
                            onOptionSelected = {
                                val ratio = AspectRatio.values().find { r -> r.ratio == it }
                                ratio?.let { viewModel.setAspectRatio(it) }
                            }
                        )
                    }
                }
                
                // Export Settings Section
                item {
                    SettingsSection(title = "Export Settings") {
                        SettingDropdown(
                            icon = Icons.Default.VideoFile,
                            title = "Default Format",
                            options = ExportFormat.values().map { it.name },
                            selectedOption = uiState.settings.defaultExportFormat.name,
                            onOptionSelected = {
                                viewModel.setExportFormat(ExportFormat.valueOf(it))
                            }
                        )
                        
                        SettingSwitch(
                            icon = Icons.Default.Save,
                            title = "Auto-save Projects",
                            subtitle = "Automatically save project changes",
                            checked = uiState.settings.autoSaveProjects,
                            onCheckedChange = { viewModel.setAutoSave(it) }
                        )
                    }
                }
                
                // Performance Settings Section
                item {
                    SettingsSection(title = "Performance") {
                        SettingSwitch(
                            icon = Icons.Default.Speed,
                            title = "GPU Acceleration",
                            subtitle = "Use hardware acceleration for faster rendering",
                            checked = uiState.settings.enableGPUAcceleration,
                            onCheckedChange = { viewModel.setGPUAcceleration(it) }
                        )
                        
                        SettingSwitch(
                            icon = Icons.Default.Preview,
                            title = "Generate Proxies",
                            subtitle = "Create proxy files for smoother editing",
                            checked = uiState.settings.generateProxies,
                            onCheckedChange = { viewModel.setGenerateProxies(it) }
                        )
                    }
                }
                
                // UI Settings Section
                item {
                    SettingsSection(title = "Appearance") {
                        SettingDropdown(
                            icon = Icons.Default.Palette,
                            title = "Theme",
                            options = AppTheme.values().map { it.name },
                            selectedOption = uiState.settings.theme.name,
                            onOptionSelected = {
                                viewModel.setTheme(AppTheme.valueOf(it))
                            }
                        )
                        
                        SettingSwitch(
                            icon = Icons.Default.Help,
                            title = "Show Tutorials",
                            subtitle = "Display helpful tips and tutorials",
                            checked = uiState.settings.showTutorials,
                            onCheckedChange = { viewModel.setShowTutorials(it) }
                        )
                    }
                }
                
                // Storage Settings Section
                item {
                    SettingsSection(title = "Storage") {
                        SettingSwitch(
                            icon = Icons.Default.CleaningServices,
                            title = "Auto-clear Cache",
                            subtitle = "Automatically clear old cache files",
                            checked = uiState.settings.autoClearCache,
                            onCheckedChange = { viewModel.setAutoClearCache(it) }
                        )
                    }
                }
                
                // About Section
                item {
                    SettingsSection(title = "About") {
                        SettingItem(
                            icon = Icons.Default.Info,
                            title = "App Version",
                            subtitle = "1.0.0 (Build 1)",
                            onClick = {}
                        )
                        
                        SettingItem(
                            icon = Icons.Default.Star,
                            title = "Rate App",
                            subtitle = "Show your support",
                            onClick = {}
                        )
                        
                        SettingItem(
                            icon = Icons.Default.BugReport,
                            title = "Report Bug",
                            subtitle = "Help us improve",
                            onClick = {}
                        )
                    }
                }
                
                // Reset Button
                item {
                    Button(
                        onClick = { viewModel.showResetDialog(true) },
                        modifier = Modifier.fillMaxWidth(),
                        colors = ButtonDefaults.buttonColors(
                            containerColor = Color.Red.copy(alpha = 0.2f)
                        ),
                        shape = RoundedCornerShape(12.dp)
                    ) {
                        Icon(Icons.Default.RestartAlt, contentDescription = null)
                        Spacer(modifier = Modifier.width(8.dp))
                        Text("Reset to Defaults", color = Color.Red)
                    }
                }
            }
        }
    }
    
    // Reset Confirmation Dialog
    if (uiState.showResetDialog) {
        ResetDialog(
            onDismiss = { viewModel.showResetDialog(false) },
            onConfirm = {
                viewModel.resetToDefaults()
            }
        )
    }
    
    // Clear Cache Dialog
    if (uiState.showClearCacheDialog) {
        ClearCacheDialog(
            onDismiss = { viewModel.showClearCacheDialog(false) },
            onConfirm = {
                viewModel.clearCache()
                viewModel.showClearCacheDialog(false)
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun SettingsTopBar(
    onBackClick: () -> Unit
) {
    TopAppBar(
        title = {
            Text(
                "Settings",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
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
private fun SettingsSection(
    title: String,
    content: @Composable ColumnScope.() -> Unit
) {
    Column {
        Text(
            title,
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White,
            modifier = Modifier.padding(bottom = 8.dp)
        )
        
        Card(
            modifier = Modifier.fillMaxWidth(),
            colors = CardDefaults.cardColors(
                containerColor = Color(0xFF2D2438)
            ),
            shape = RoundedCornerShape(12.dp)
        ) {
            Column(
                modifier = Modifier.padding(4.dp),
                content = content
            )
        }
    }
}

@Composable
private fun SettingItem(
    icon: ImageVector,
    title: String,
    subtitle: String? = null,
    onClick: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick)
            .padding(16.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = BrandPurple,
            modifier = Modifier.size(24.dp)
        )
        
        Spacer(modifier = Modifier.width(16.dp))
        
        Column(modifier = Modifier.weight(1f)) {
            Text(
                title,
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
            subtitle?.let {
                Text(
                    it,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        
        Icon(
            Icons.Default.ChevronRight,
            contentDescription = null,
            tint = Color.Gray
        )
    }
}

@Composable
private fun SettingSwitch(
    icon: ImageVector,
    title: String,
    subtitle: String? = null,
    checked: Boolean,
    onCheckedChange: (Boolean) -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Icon(
            icon,
            contentDescription = null,
            tint = BrandPurple,
            modifier = Modifier.size(24.dp)
        )
        
        Spacer(modifier = Modifier.width(16.dp))
        
        Column(modifier = Modifier.weight(1f)) {
            Text(
                title,
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
            subtitle?.let {
                Text(
                    it,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        
        Switch(
            checked = checked,
            onCheckedChange = onCheckedChange,
            colors = SwitchDefaults.colors(
                checkedThumbColor = BrandPurple,
                checkedTrackColor = BrandPurple.copy(alpha = 0.5f)
            )
        )
    }
}

@Composable
private fun SettingDropdown(
    icon: ImageVector,
    title: String,
    options: List<String>,
    selectedOption: String,
    onOptionSelected: (String) -> Unit
) {
    var expanded by remember { mutableStateOf(false) }
    
    Box {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .clickable { expanded = true }
                .padding(16.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                icon,
                contentDescription = null,
                tint = BrandPurple,
                modifier = Modifier.size(24.dp)
            )
            
            Spacer(modifier = Modifier.width(16.dp))
            
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    title,
                    style = MaterialTheme.typography.bodyLarge,
                    color = Color.White
                )
                Text(
                    selectedOption,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
            
            Icon(
                Icons.Default.ArrowDropDown,
                contentDescription = null,
                tint = Color.Gray
            )
        }
        
        DropdownMenu(
            expanded = expanded,
            onDismissRequest = { expanded = false }
        ) {
            options.forEach { option ->
                DropdownMenuItem(
                    text = { Text(option) },
                    onClick = {
                        onOptionSelected(option)
                        expanded = false
                    }
                )
            }
        }
    }
}

@Composable
private fun StorageInfoCard(
    storageInfo: com.clipforge.videoeditor.data.local.StorageInfo,
    onClearCache: () -> Unit,
    formatBytes: (Long) -> String
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Storage",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                
                TextButton(onClick = onClearCache) {
                    Text("Clear Cache", color = BrandPurple)
                }
            }
            
            Spacer(modifier = Modifier.height(16.dp))
            
            StorageProgressBar(
                used = storageInfo.usedSpace,
                total = storageInfo.totalSpace
            )
            
            Spacer(modifier = Modifier.height(12.dp))
            
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween
            ) {
                StorageItem("Cache", formatBytes(storageInfo.cacheSize))
                StorageItem("Projects", formatBytes(storageInfo.projectsSize))
                StorageItem("Available", formatBytes(storageInfo.availableSpace))
            }
        }
    }
}

@Composable
private fun StorageProgressBar(
    used: Long,
    total: Long
) {
    val progress = (used.toFloat() / total.toFloat()).coerceIn(0f, 1f)
    
    Column {
        LinearProgressIndicator(
            progress = progress,
            modifier = Modifier
                .fillMaxWidth()
                .height(8.dp),
            color = BrandPurple,
            trackColor = Color.Gray.copy(alpha = 0.3f)
        )
    }
}

@Composable
private fun StorageItem(
    label: String,
    value: String
) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            value,
            style = MaterialTheme.typography.bodyMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        Text(
            label,
            style = MaterialTheme.typography.bodySmall,
            color = Color.Gray
        )
    }
}

@Composable
private fun ResetDialog(
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.Warning,
                contentDescription = null,
                tint = Color.Red
            )
        },
        title = { Text("Reset Settings?") },
        text = {
            Text("This will reset all settings to their default values. This action cannot be undone.")
        },
        confirmButton = {
            Button(
                onClick = onConfirm,
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color.Red
                )
            ) {
                Text("Reset")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Cancel")
            }
        }
    )
}

@Composable
private fun ClearCacheDialog(
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.CleaningServices,
                contentDescription = null,
                tint = BrandPurple
            )
        },
        title = { Text("Clear Cache?") },
        text = {
            Text("This will clear all cached files to free up storage space. Your projects will not be affected.")
        },
        confirmButton = {
            Button(
                onClick = onConfirm,
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                )
            ) {
                Text("Clear")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Cancel")
            }
        }
    )
}
```

---

## User Flow

1. **Navigate to Settings** → Settings screen displays
2. **Browse Sections** → Video, Export, Performance, UI, Storage, About
3. **Change Setting** → Tap to modify (dropdown, switch, etc.)
4. **Auto-Save** → Changes saved immediately to SharedPreferences
5. **View Storage** → See storage usage and clear cache
6. **Reset** → Restore all settings to defaults

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should save video quality to preferences`() = runTest {
    viewModel.setVideoQuality(VideoQuality.ULTRA)
    
    val settings = viewModel.uiState.value.settings
    assertEquals(VideoQuality.ULTRA, settings.defaultVideoQuality)
}
```

---

## Related Screens

- [Home Screen](./01-HOME-SCREEN-ENHANCED.md)
- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)
- [Export Screen](./05-EXPORT-SCREEN-ENHANCED.md)