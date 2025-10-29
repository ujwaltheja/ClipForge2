# 02 - Import Screen - Complete Enhanced Documentation

## Overview
The Import Screen allows users to browse and select media files from their device storage using the **MediaStore API**. It features **local-only storage**, permission handling, thumbnail generation, and multi-select functionality.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/import/
│   ├── ImportScreen.kt                  # Main UI Composable
│   ├── ImportViewModel.kt               # State management
│   ├── components/
│   │   ├── MediaGrid.kt                # Media grid component
│   │   ├── MediaItem.kt                # Individual media card
│   │   └── PermissionHandler.kt        # Permission request UI
│   └── data/
│       ├── LocalMediaRepository.kt     # Media scanning
│       ├── MediaScanner.kt             # MediaStore scanner
│       └── models/MediaFile.kt         # Media data model
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
    implementation("androidx.lifecycle:lifecycle-runtime-compose:2.7.0")
    
    // Hilt
    implementation("com.google.dagger:hilt-android:2.51.1")
    kapt("com.google.dagger:hilt-compiler:2.51.1")
    
    // Accompanist Permissions
    implementation("com.google.accompanist:accompanist-permissions:0.34.0")
    
    // Coil for image loading
    implementation("io.coil-kt:coil-compose:2.6.0")
    implementation("io.coil-kt:coil-video:2.6.0")
    
    // Media3 for video metadata
    implementation("androidx.media3:media3-exoplayer:1.3.1")
    implementation("androidx.media3:media3-common:1.3.1")
}
```

### AndroidManifest.xml Permissions
```xml
<manifest>
    <!-- For Android 13+ (API 33+) -->
    <uses-permission android:name="android.permission.READ_MEDIA_VIDEO" />
    <uses-permission android:name="android.permission.READ_MEDIA_IMAGES" />
    
    <!-- For Android 12 and below -->
    <uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE"
                     android:maxSdkVersion="32" />
</manifest>
```

---

## Complete Source Code

### 1. Media Data Model

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/MediaFile.kt`

```kotlin
package com.clipforge.videoeditor.data.model

import android.net.Uri
import android.os.Parcelable
import kotlinx.parcelize.Parcelize

@Parcelize
data class MediaFile(
    val id: Long,
    val uri: Uri,
    val displayName: String,
    val duration: Long, // in milliseconds
    val size: Long, // in bytes
    val mimeType: String,
    val dateAdded: Long,
    val width: Int = 0,
    val height: Int = 0,
    val thumbnailUri: Uri? = null,
    val isSelected: Boolean = false
) : Parcelable {
    
    val durationFormatted: String
        get() {
            val seconds = duration / 1000
            val minutes = seconds / 60
            val secs = seconds % 60
            return String.format("%02d:%02d", minutes, secs)
        }
    
    val sizeFormatted: String
        get() {
            return when {
                size < 1024 -> "$size B"
                size < 1024 * 1024 -> "${size / 1024} KB"
                size < 1024 * 1024 * 1024 -> "${size / (1024 * 1024)} MB"
                else -> String.format("%.2f GB", size / (1024f * 1024f * 1024f))
            }
        }
    
    val resolution: String
        get() = if (width > 0 && height > 0) "${width}x${height}" else "Unknown"
}

enum class MediaType {
    VIDEO, IMAGE, ALL
}
```

---

### 2. Media Scanner (MediaStore API)

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/MediaScanner.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.ContentResolver
import android.content.ContentUris
import android.content.Context
import android.net.Uri
import android.os.Build
import android.provider.MediaStore
import com.clipforge.videoeditor.data.model.MediaFile
import com.clipforge.videoeditor.data.model.MediaType
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class MediaScanner @Inject constructor(
    private val context: Context
) {
    
    private val contentResolver: ContentResolver = context.contentResolver
    
    /**
     * Scan device for video files using MediaStore API
     */
    suspend fun scanVideos(): List<MediaFile> = withContext(Dispatchers.IO) {
        val mediaFiles = mutableListOf<MediaFile>()
        
        val collection = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            MediaStore.Video.Media.getContentUri(MediaStore.VOLUME_EXTERNAL)
        } else {
            MediaStore.Video.Media.EXTERNAL_CONTENT_URI
        }
        
        val projection = arrayOf(
            MediaStore.Video.Media._ID,
            MediaStore.Video.Media.DISPLAY_NAME,
            MediaStore.Video.Media.DURATION,
            MediaStore.Video.Media.SIZE,
            MediaStore.Video.Media.MIME_TYPE,
            MediaStore.Video.Media.DATE_ADDED,
            MediaStore.Video.Media.WIDTH,
            MediaStore.Video.Media.HEIGHT
        )
        
        val sortOrder = "${MediaStore.Video.Media.DATE_ADDED} DESC"
        
        try {
            contentResolver.query(
                collection,
                projection,
                null,
                null,
                sortOrder
            )?.use { cursor ->
                val idColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media._ID)
                val nameColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DISPLAY_NAME)
                val durationColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DURATION)
                val sizeColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.SIZE)
                val mimeTypeColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.MIME_TYPE)
                val dateAddedColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.DATE_ADDED)
                val widthColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.WIDTH)
                val heightColumn = cursor.getColumnIndexOrThrow(MediaStore.Video.Media.HEIGHT)
                
                while (cursor.moveToNext()) {
                    val id = cursor.getLong(idColumn)
                    val name = cursor.getString(nameColumn)
                    val duration = cursor.getLong(durationColumn)
                    val size = cursor.getLong(sizeColumn)
                    val mimeType = cursor.getString(mimeTypeColumn)
                    val dateAdded = cursor.getLong(dateAddedColumn)
                    val width = cursor.getInt(widthColumn)
                    val height = cursor.getInt(heightColumn)
                    
                    val contentUri = ContentUris.withAppendedId(
                        MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                        id
                    )
                    
                    mediaFiles.add(
                        MediaFile(
                            id = id,
                            uri = contentUri,
                            displayName = name,
                            duration = duration,
                            size = size,
                            mimeType = mimeType,
                            dateAdded = dateAdded,
                            width = width,
                            height = height,
                            thumbnailUri = getThumbnailUri(id)
                        )
                    )
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        
        mediaFiles
    }
    
    /**
     * Scan device for image files
     */
    suspend fun scanImages(): List<MediaFile> = withContext(Dispatchers.IO) {
        val mediaFiles = mutableListOf<MediaFile>()
        
        val collection = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL)
        } else {
            MediaStore.Images.Media.EXTERNAL_CONTENT_URI
        }
        
        val projection = arrayOf(
            MediaStore.Images.Media._ID,
            MediaStore.Images.Media.DISPLAY_NAME,
            MediaStore.Images.Media.SIZE,
            MediaStore.Images.Media.MIME_TYPE,
            MediaStore.Images.Media.DATE_ADDED,
            MediaStore.Images.Media.WIDTH,
            MediaStore.Images.Media.HEIGHT
        )
        
        val sortOrder = "${MediaStore.Images.Media.DATE_ADDED} DESC"
        
        try {
            contentResolver.query(
                collection,
                projection,
                null,
                null,
                sortOrder
            )?.use { cursor ->
                val idColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media._ID)
                val nameColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DISPLAY_NAME)
                val sizeColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.SIZE)
                val mimeTypeColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.MIME_TYPE)
                val dateAddedColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.DATE_ADDED)
                val widthColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.WIDTH)
                val heightColumn = cursor.getColumnIndexOrThrow(MediaStore.Images.Media.HEIGHT)
                
                while (cursor.moveToNext()) {
                    val id = cursor.getLong(idColumn)
                    val name = cursor.getString(nameColumn)
                    val size = cursor.getLong(sizeColumn)
                    val mimeType = cursor.getString(mimeTypeColumn)
                    val dateAdded = cursor.getLong(dateAddedColumn)
                    val width = cursor.getInt(widthColumn)
                    val height = cursor.getInt(heightColumn)
                    
                    val contentUri = ContentUris.withAppendedId(
                        MediaStore.Images.Media.EXTERNAL_CONTENT_URI,
                        id
                    )
                    
                    mediaFiles.add(
                        MediaFile(
                            id = id,
                            uri = contentUri,
                            displayName = name,
                            duration = 0, // Images have no duration
                            size = size,
                            mimeType = mimeType,
                            dateAdded = dateAdded,
                            width = width,
                            height = height,
                            thumbnailUri = contentUri // Image itself is thumbnail
                        )
                    )
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
        
        mediaFiles
    }
    
    /**
     * Get thumbnail URI for video
     */
    private fun getThumbnailUri(videoId: Long): Uri? {
        return try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                ContentUris.withAppendedId(
                    MediaStore.Video.Media.EXTERNAL_CONTENT_URI,
                    videoId
                )
            } else {
                ContentUris.withAppendedId(
                    MediaStore.Video.Thumbnails.EXTERNAL_CONTENT_URI,
                    videoId
                )
            }
        } catch (e: Exception) {
            null
        }
    }
}
```

---

### 3. Local Media Repository

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/LocalMediaRepository.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import com.clipforge.videoeditor.data.model.MediaFile
import com.clipforge.videoeditor.data.model.MediaType
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.flow
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class LocalMediaRepository @Inject constructor(
    private val mediaScanner: MediaScanner
) {
    
    /**
     * Get all media files based on type
     */
    fun getMediaFiles(mediaType: MediaType): Flow<List<MediaFile>> = flow {
        val files = when (mediaType) {
            MediaType.VIDEO -> mediaScanner.scanVideos()
            MediaType.IMAGE -> mediaScanner.scanImages()
            MediaType.ALL -> mediaScanner.scanVideos() + mediaScanner.scanImages()
        }
        emit(files.sortedByDescending { it.dateAdded })
    }
    
    /**
     * Search media files by name
     */
    fun searchMedia(query: String, mediaType: MediaType): Flow<List<MediaFile>> = flow {
        val files = when (mediaType) {
            MediaType.VIDEO -> mediaScanner.scanVideos()
            MediaType.IMAGE -> mediaScanner.scanImages()
            MediaType.ALL -> mediaScanner.scanVideos() + mediaScanner.scanImages()
        }
        
        val filtered = if (query.isBlank()) {
            files
        } else {
            files.filter { it.displayName.contains(query, ignoreCase = true) }
        }
        
        emit(filtered.sortedByDescending { it.dateAdded })
    }
}
```

---

### 4. Import ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/import/ImportViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.import

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.clipforge.videoeditor.data.local.LocalMediaRepository
import com.clipforge.videoeditor.data.local.LocalProjectManager
import com.clipforge.videoeditor.data.model.MediaFile
import com.clipforge.videoeditor.data.model.MediaType
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.catch
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class ImportUiState(
    val mediaFiles: List<MediaFile> = emptyList(),
    val selectedFiles: Set<MediaFile> = emptySet(),
    val isLoading: Boolean = false,
    val error: String? = null,
    val mediaType: MediaType = MediaType.VIDEO,
    val searchQuery: String = "",
    val hasPermission: Boolean = false
)

@HiltViewModel
class ImportViewModel @Inject constructor(
    private val mediaRepository: LocalMediaRepository,
    private val projectManager: LocalProjectManager
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(ImportUiState())
    val uiState: StateFlow<ImportUiState> = _uiState.asStateFlow()
    
    /**
     * Load media files from device
     */
    fun loadMedia(hasPermission: Boolean) {
        _uiState.update { it.copy(hasPermission = hasPermission) }
        
        if (!hasPermission) return
        
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true, error = null) }
            
            mediaRepository.getMediaFiles(_uiState.value.mediaType)
                .catch { exception ->
                    _uiState.update {
                        it.copy(
                            isLoading = false,
                            error = "Failed to load media: ${exception.message}"
                        )
                    }
                }
                .collect { files ->
                    _uiState.update {
                        it.copy(
                            mediaFiles = files,
                            isLoading = false
                        )
                    }
                }
        }
    }
    
    /**
     * Toggle selection of a media file
     */
    fun toggleSelection(mediaFile: MediaFile) {
        _uiState.update { state ->
            val updatedFiles = state.mediaFiles.map {
                if (it.id == mediaFile.id) {
                    it.copy(isSelected = !it.isSelected)
                } else {
                    it
                }
            }
            
            val selectedFiles = updatedFiles.filter { it.isSelected }.toSet()
            
            state.copy(
                mediaFiles = updatedFiles,
                selectedFiles = selectedFiles
            )
        }
    }
    
    /**
     * Select all media files
     */
    fun selectAll() {
        _uiState.update { state ->
            val updatedFiles = state.mediaFiles.map { it.copy(isSelected = true) }
            state.copy(
                mediaFiles = updatedFiles,
                selectedFiles = updatedFiles.toSet()
            )
        }
    }
    
    /**
     * Clear all selections
     */
    fun clearSelection() {
        _uiState.update { state ->
            val updatedFiles = state.mediaFiles.map { it.copy(isSelected = false) }
            state.copy(
                mediaFiles = updatedFiles,
                selectedFiles = emptySet()
            )
        }
    }
    
    /**
     * Change media type filter
     */
    fun setMediaType(mediaType: MediaType) {
        _uiState.update { it.copy(mediaType = mediaType) }
        loadMedia(_uiState.value.hasPermission)
    }
    
    /**
     * Search media files
     */
    fun searchMedia(query: String) {
        _uiState.update { it.copy(searchQuery = query) }
        
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true) }
            
            mediaRepository.searchMedia(query, _uiState.value.mediaType)
                .catch { exception ->
                    _uiState.update {
                        it.copy(
                            isLoading = false,
                            error = "Search failed: ${exception.message}"
                        )
                    }
                }
                .collect { files ->
                    _uiState.update {
                        it.copy(
                            mediaFiles = files,
                            isLoading = false
                        )
                    }
                }
        }
    }
    
    /**
     * Import selected files and create project
     */
    suspend fun importAndCreateProject(projectName: String): String? {
        val selectedFiles = _uiState.value.selectedFiles
        
        if (selectedFiles.isEmpty()) {
            _uiState.update { it.copy(error = "No files selected") }
            return null
        }
        
        return try {
            val project = projectManager.createNewProject(projectName)
            // TODO: Add clips to project timeline
            project.id
        } catch (e: Exception) {
            _uiState.update { it.copy(error = "Failed to create project: ${e.message}") }
            null
        }
    }
    
    /**
     * Clear error message
     */
    fun clearError() {
        _uiState.update { it.copy(error = null) }
    }
}
```

---

### 5. Import Screen UI

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/import/ImportScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.import

import android.Manifest
import android.os.Build
import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import coil.compose.AsyncImage
import com.clipforge.videoeditor.data.model.MediaFile
import com.clipforge.videoeditor.data.model.MediaType
import com.clipforge.videoeditor.ui.theme.BrandPurple
import com.google.accompanist.permissions.ExperimentalPermissionsApi
import com.google.accompanist.permissions.rememberMultiplePermissionsState

@OptIn(ExperimentalMaterial3Api::class, ExperimentalPermissionsApi::class)
@Composable
fun ImportScreen(
    navController: NavController,
    viewModel: ImportViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    var showImportDialog by remember { mutableStateOf(false) }
    
    // Permission handling
    val permissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
        listOf(
            Manifest.permission.READ_MEDIA_VIDEO,
            Manifest.permission.READ_MEDIA_IMAGES
        )
    } else {
        listOf(Manifest.permission.READ_EXTERNAL_STORAGE)
    }
    
    val permissionsState = rememberMultiplePermissionsState(permissions) { permissionsMap ->
        val allGranted = permissionsMap.values.all { it }
        viewModel.loadMedia(allGranted)
    }
    
    // Load media when permissions granted
    LaunchedEffect(permissionsState.allPermissionsGranted) {
        if (permissionsState.allPermissionsGranted) {
            viewModel.loadMedia(true)
        }
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
            ImportTopBar(
                selectedCount = uiState.selectedFiles.size,
                onBackClick = { navController.popBackStack() },
                onClearSelection = { viewModel.clearSelection() },
                onSelectAll = { viewModel.selectAll() }
            )
        },
        bottomBar = {
            if (uiState.selectedFiles.isNotEmpty()) {
                ImportBottomBar(
                    selectedCount = uiState.selectedFiles.size,
                    onImportClick = { showImportDialog = true }
                )
            }
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
            if (!permissionsState.allPermissionsGranted) {
                PermissionRequestView(
                    onRequestPermission = { permissionsState.launchMultiplePermissionRequest() }
                )
            } else {
                Column(modifier = Modifier.fillMaxSize()) {
                    // Search and filter bar
                    SearchAndFilterBar(
                        searchQuery = uiState.searchQuery,
                        mediaType = uiState.mediaType,
                        onSearchChange = { viewModel.searchMedia(it) },
                        onMediaTypeChange = { viewModel.setMediaType(it) }
                    )
                    
                    // Media grid
                    if (uiState.isLoading) {
                        LoadingView()
                    } else if (uiState.mediaFiles.isEmpty()) {
                        EmptyMediaView()
                    } else {
                        MediaGrid(
                            mediaFiles = uiState.mediaFiles,
                            onMediaClick = { viewModel.toggleSelection(it) }
                        )
                    }
                }
            }
        }
    }
    
    // Import dialog
    if (showImportDialog) {
        ImportDialog(
            selectedCount = uiState.selectedFiles.size,
            onDismiss = { showImportDialog = false },
            onConfirm = { projectName ->
                showImportDialog = false
                // TODO: Navigate to editor
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun ImportTopBar(
    selectedCount: Int,
    onBackClick: () -> Unit,
    onClearSelection: () -> Unit,
    onSelectAll: () -> Unit
) {
    TopAppBar(
        title = {
            Text(
                if (selectedCount > 0) "$selectedCount selected" else "Import Media",
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
        actions = {
            if (selectedCount > 0) {
                IconButton(onClick = onClearSelection) {
                    Icon(Icons.Default.Clear, "Clear", tint = Color.White)
                }
            }
            IconButton(onClick = onSelectAll) {
                Icon(Icons.Default.SelectAll, "Select All", tint = Color.White)
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Color(0xFF1A1625)
        )
    )
}

@Composable
private fun ImportBottomBar(
    selectedCount: Int,
    onImportClick: () -> Unit
) {
    Surface(
        modifier = Modifier.fillMaxWidth(),
        color = Color(0xFF1A1625),
        shadowElevation = 8.dp
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    "$selectedCount items selected",
                    style = MaterialTheme.typography.titleMedium,
                    color = Color.White,
                    fontWeight = FontWeight.Bold
                )
                Text(
                    "Ready to import",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
            
            Button(
                onClick = onImportClick,
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                ),
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier.height(48.dp)
            ) {
                Icon(Icons.Default.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    "Import",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
private fun SearchAndFilterBar(
    searchQuery: String,
    mediaType: MediaType,
    onSearchChange: (String) -> Unit,
    onMediaTypeChange: (MediaType) -> Unit
) {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp)
    ) {
        // Search bar
        OutlinedTextField(
            value = searchQuery,
            onValueChange = onSearchChange,
            modifier = Modifier.fillMaxWidth(),
            placeholder = { Text("Search media...") },
            leadingIcon = {
                Icon(Icons.Default.Search, contentDescription = null)
            },
            trailingIcon = {
                if (searchQuery.isNotEmpty()) {
                    IconButton(onClick = { onSearchChange("") }) {
                        Icon(Icons.Default.Clear, contentDescription = "Clear")
                    }
                }
            },
            singleLine = true,
            shape = RoundedCornerShape(12.dp),
            colors = OutlinedTextFieldDefaults.colors(
                focusedContainerColor = Color(0xFF2D2438),
                unfocusedContainerColor = Color(0xFF2D2438),
                focusedTextColor = Color.White,
                unfocusedTextColor = Color.White
            )
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        // Media type filter
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            MediaType.values().forEach { type ->
                FilterChip(
                    selected = mediaType == type,
                    onClick = { onMediaTypeChange(type) },
                    label = {
                        Text(
                            when (type) {
                                MediaType.VIDEO -> "Videos"
                                MediaType.IMAGE -> "Images"
                                MediaType.ALL -> "All"
                            }
                        )
                    },
                    colors = FilterChipDefaults.filterChipColors(
                        selectedContainerColor = BrandPurple,
                        selectedLabelColor = Color.White
                    )
                )
            }
        }
    }
}

@Composable
private fun MediaGrid(
    mediaFiles: List<MediaFile>,
    onMediaClick: (MediaFile) -> Unit
) {
    LazyVerticalGrid(
        columns = GridCells.Fixed(3),
        contentPadding = PaddingValues(8.dp),
        horizontalArrangement = Arrangement.spacedBy(4.dp),
        verticalArrangement = Arrangement.spacedBy(4.dp)
    ) {
        items(mediaFiles, key = { it.id }) { mediaFile ->
            MediaItemCard(
                mediaFile = mediaFile,
                onClick = { onMediaClick(mediaFile) }
            )
        }
    }
}

@Composable
private fun MediaItemCard(
    mediaFile: MediaFile,
    onClick: () -> Unit
) {
    Box(
        modifier = Modifier
            .aspectRatio(1f)
            .clickable(onClick = onClick)
    ) {
        // Thumbnail
        AsyncImage(
            model = mediaFile.thumbnailUri ?: mediaFile.uri,
            contentDescription = mediaFile.displayName,
            modifier = Modifier.fillMaxSize(),
            contentScale = ContentScale.Crop
        )
        
        // Duration overlay (for videos)
        if (mediaFile.duration > 0) {
            Surface(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(4.dp),
                color = Color.Black.copy(alpha = 0.7f),
                shape = RoundedCornerShape(4.dp)
            ) {
                Text(
                    mediaFile.durationFormatted,
                    modifier = Modifier.padding(horizontal = 4.dp, vertical = 2.dp),
                    style = MaterialTheme.typography.labelSmall,
                    color = Color.White
                )
            }
        }
        
        // Selection indicator
        AnimatedVisibility(
            visible = mediaFile.isSelected,
            modifier = Modifier
                .align(Alignment.TopEnd)
                .padding(4.dp)
        ) {
            Box(
                modifier = Modifier
                    .size(24.dp)
                    .background(BrandPurple, CircleShape)
                    .border(2.dp, Color.White, CircleShape),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Default.Check,
                    contentDescription = "Selected",
                    tint = Color.White,
                    modifier = Modifier.size(16.dp)
                )
            }
        }
        
        // Selection overlay
        if (mediaFile.isSelected) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(BrandPurple.copy(alpha = 0.3f))
            )
        }
    }
}

@Composable
private fun PermissionRequestView(
    onRequestPermission: () -> Unit
) {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp),
            modifier = Modifier.padding(32.dp)
        ) {
            Icon(
                Icons.Default.PermMedia,
                contentDescription = null,
                modifier = Modifier.size(80.dp),
                tint = BrandPurple
            )
            
            Text(
                "Media Access Required",
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
                color = Color.White,
                textAlign = TextAlign.Center
            )
            
            Text(
                "ClipForge needs access to your media files to import videos and images for editing.",
                style = MaterialTheme.typography.bodyLarge,
                color = Color.Gray,
                textAlign = TextAlign.Center
            )
            
            Spacer(modifier = Modifier.height(8.dp))
            
            Button(
                onClick = onRequestPermission,
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                ),
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp)
            ) {
                Text(
                    "Grant Permission",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
            }
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
                "Scanning media files...",
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
        }
    }
}

@Composable
private fun EmptyMediaView() {
    Box(
        modifier = Modifier.fillMaxSize(),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier.padding(32.dp)
        ) {
            Icon(
                Icons.Default.VideoLibrary,
                contentDescription = null,
                modifier = Modifier.size(80.dp),
                tint = Color.Gray
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            Text(
                "No Media Found",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White,
                textAlign = TextAlign.Center
            )
            
            Text(
                "No videos or images found on your device",
                style = MaterialTheme.typography.bodyMedium,
                color = Color.Gray,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Composable
private fun ImportDialog(
    selectedCount: Int,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var projectName by remember { mutableStateOf("") }
    
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.CreateNewFolder,
                contentDescription = null,
                tint = BrandPurple
            )
        },
        title = { Text("Create Project") },
        text = {
            Column {
                Text("Import $selectedCount items into a new project")
                Spacer(modifier = Modifier.height(16.dp))
                OutlinedTextField(
                    value = projectName,
                    onValueChange = { projectName = it },
                    label = { Text("Project Name") },
                    singleLine = true,
                    modifier = Modifier.fillMaxWidth()
                )
            }
        },
        confirmButton = {
            Button(
                onClick = {
                    val name = projectName.ifBlank { "New Project" }
                    onConfirm(name)
                },
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                )
            ) {
                Text("Create & Import")
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

1. **App Launch** → Navigate to Import Screen
2. **Permission Request** → User grants media access
3. **Media Scan** → App scans device using MediaStore API
4. **Grid Display** → Videos/images displayed in grid
5. **Selection** → User taps to select multiple items
6. **Import** → Tap import button, create project dialog
7. **Project Created** → Navigate to Editor with selected media

---

## Performance Considerations

- **Lazy Loading**: Grid uses lazy composition
- **Thumbnail Caching**: Coil handles image caching
- **Background Scanning**: MediaStore queries on IO dispatcher
- **Memory Efficient**: Only thumbnails loaded in grid
- **Permission Handling**: Proper Android 13+ permission flow

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should toggle media selection`() = runTest {
    val mediaFile = MediaFile(id = 1, uri = mockUri, ...)
    viewModel.toggleSelection(mediaFile)
    
    assertTrue(viewModel.uiState.value.selectedFiles.contains(mediaFile))
}
```

---

## Related Screens

- [Home Screen](./01-HOME-SCREEN-ENHANCED.md)
- [Templates Screen](./03-TEMPLATES-SCREEN-ENHANCED.md)
- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)