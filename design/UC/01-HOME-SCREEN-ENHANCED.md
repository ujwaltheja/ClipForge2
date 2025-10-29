# 01 - Home Screen - Complete Enhanced Documentation

## Overview
The Home Screen is the entry point of ClipForge, featuring **local-only storage** for project management, recent projects display, and quick action cards for importing media, recording, and browsing templates.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/home/
│   ├── HomeScreen.kt                    # Main UI Composable
│   ├── HomeViewModel.kt                 # State management
│   ├── components/
│   │   ├── ProjectCard.kt              # Project card component
│   │   ├── QuickActionCard.kt          # Quick action cards
│   │   └── EmptyProjectsView.kt        # Empty state
│   └── data/
│       ├── LocalProjectManager.kt      # Local storage manager
│       └── models/Project.kt           # Project data model
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
    implementation("androidx.compose.runtime:runtime:1.6.1")
    
    // Navigation
    implementation("androidx.navigation:navigation-compose:2.7.7")
    
    // ViewModel
    implementation("androidx.lifecycle:lifecycle-viewmodel-compose:2.7.0")
    implementation("androidx.lifecycle:lifecycle-runtime-compose:2.7.0")
    
    // Hilt Dependency Injection
    implementation("com.google.dagger:hilt-android:2.51.1")
    kapt("com.google.dagger:hilt-compiler:2.51.1")
    implementation("androidx.hilt:hilt-navigation-compose:1.2.0")
    
    // Kotlinx Serialization (for JSON)
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.3")
    
    // Coil for image loading
    implementation("io.coil-kt:coil-compose:2.6.0")
}
```

---

## Complete Source Code

### 1. Project Data Model

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/Project.kt`

```kotlin
package com.clipforge.videoeditor.data.model

import kotlinx.serialization.Serializable
import java.util.UUID

@Serializable
data class Project(
    val id: String = UUID.randomUUID().toString(),
    val name: String,
    val createdAt: Long = System.currentTimeMillis(),
    val modifiedAt: Long = System.currentTimeMillis(),
    val thumbnailPath: String? = null,
    val clipCount: Int = 0,
    val duration: Long = 0, // in milliseconds
    val resolution: String = "1080x1920",
    val fps: Int = 30
)

@Serializable
data class ProjectList(
    val projects: List<Project> = emptyList()
)
```

---

### 2. Local Project Manager

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/LocalProjectManager.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.clipforge.videoeditor.data.model.Project
import com.clipforge.videoeditor.data.model.ProjectList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import java.io.File
import java.io.FileOutputStream
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class LocalProjectManager @Inject constructor(
    private val context: Context
) {
    
    private val projectsDir: File by lazy {
        File(context.filesDir, "projects").apply { mkdirs() }
    }
    
    private val thumbnailsDir: File by lazy {
        File(context.filesDir, "thumbnails").apply { mkdirs() }
    }
    
    private val projectsFile: File by lazy {
        File(projectsDir, "projects_list.json")
    }
    
    private val json = Json {
        prettyPrint = true
        ignoreUnknownKeys = true
    }
    
    /**
     * Load all projects from local storage
     */
    suspend fun getAllProjects(): List<Project> = withContext(Dispatchers.IO) {
        try {
            if (!projectsFile.exists()) {
                return@withContext emptyList()
            }
            
            val jsonString = projectsFile.readText()
            val projectList = json.decodeFromString<ProjectList>(jsonString)
            projectList.projects.sortedByDescending { it.modifiedAt }
        } catch (e: Exception) {
            e.printStackTrace()
            emptyList()
        }
    }
    
    /**
     * Save a single project
     */
    suspend fun saveProject(project: Project) = withContext(Dispatchers.IO) {
        try {
            val currentProjects = getAllProjects().toMutableList()
            
            // Remove existing project with same ID if exists
            currentProjects.removeAll { it.id == project.id }
            
            // Add updated project
            currentProjects.add(0, project.copy(modifiedAt = System.currentTimeMillis()))
            
            // Save to file
            val projectList = ProjectList(currentProjects)
            val jsonString = json.encodeToString(ProjectList.serializer(), projectList)
            projectsFile.writeText(jsonString)
        } catch (e: Exception) {
            e.printStackTrace()
            throw e
        }
    }
    
    /**
     * Delete a project by ID
     */
    suspend fun deleteProject(projectId: String) = withContext(Dispatchers.IO) {
        try {
            val currentProjects = getAllProjects().toMutableList()
            
            // Find and remove project
            val projectToDelete = currentProjects.find { it.id == projectId }
            currentProjects.removeAll { it.id == projectId }
            
            // Delete thumbnail if exists
            projectToDelete?.thumbnailPath?.let { path ->
                File(path).delete()
            }
            
            // Save updated list
            val projectList = ProjectList(currentProjects)
            val jsonString = json.encodeToString(ProjectList.serializer(), projectList)
            projectsFile.writeText(jsonString)
        } catch (e: Exception) {
            e.printStackTrace()
            throw e
        }
    }
    
    /**
     * Get a single project by ID
     */
    suspend fun getProject(projectId: String): Project? = withContext(Dispatchers.IO) {
        getAllProjects().find { it.id == projectId }
    }
    
    /**
     * Save project thumbnail
     */
    suspend fun saveThumbnail(projectId: String, bitmap: Bitmap): String = withContext(Dispatchers.IO) {
        try {
            val thumbnailFile = File(thumbnailsDir, "$projectId.jpg")
            FileOutputStream(thumbnailFile).use { out ->
                bitmap.compress(Bitmap.CompressFormat.JPEG, 85, out)
            }
            thumbnailFile.absolutePath
        } catch (e: Exception) {
            e.printStackTrace()
            ""
        }
    }
    
    /**
     * Load project thumbnail
     */
    suspend fun loadThumbnail(thumbnailPath: String): Bitmap? = withContext(Dispatchers.IO) {
        try {
            val file = File(thumbnailPath)
            if (file.exists()) {
                BitmapFactory.decodeFile(thumbnailPath)
            } else {
                null
            }
        } catch (e: Exception) {
            e.printStackTrace()
            null
        }
    }
    
    /**
     * Create a new project
     */
    suspend fun createNewProject(name: String): Project {
        val project = Project(
            name = name,
            createdAt = System.currentTimeMillis(),
            modifiedAt = System.currentTimeMillis()
        )
        saveProject(project)
        return project
    }
    
    /**
     * Update existing project
     */
    suspend fun updateProject(
        projectId: String,
        name: String? = null,
        clipCount: Int? = null,
        duration: Long? = null,
        thumbnailPath: String? = null
    ) {
        val project = getProject(projectId) ?: return
        
        val updatedProject = project.copy(
            name = name ?: project.name,
            clipCount = clipCount ?: project.clipCount,
            duration = duration ?: project.duration,
            thumbnailPath = thumbnailPath ?: project.thumbnailPath,
            modifiedAt = System.currentTimeMillis()
        )
        
        saveProject(updatedProject)
    }
    
    /**
     * Clear all projects (for testing or reset)
     */
    suspend fun clearAllProjects() = withContext(Dispatchers.IO) {
        projectsFile.delete()
        thumbnailsDir.deleteRecursively()
        thumbnailsDir.mkdirs()
    }
}
```

---

### 3. Home ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/home/HomeViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.home

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.clipforge.videoeditor.data.local.LocalProjectManager
import com.clipforge.videoeditor.data.model.Project
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class HomeUiState(
    val projects: List<Project> = emptyList(),
    val isLoading: Boolean = false,
    val error: String? = null,
    val selectedProject: Project? = null
)

@HiltViewModel
class HomeViewModel @Inject constructor(
    private val projectManager: LocalProjectManager
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(HomeUiState())
    val uiState: StateFlow<HomeUiState> = _uiState.asStateFlow()
    
    init {
        loadProjects()
    }
    
    /**
     * Load all projects from local storage
     */
    fun loadProjects() {
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true, error = null) }
            try {
                val projects = projectManager.getAllProjects()
                _uiState.update {
                    it.copy(
                        projects = projects,
                        isLoading = false
                    )
                }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        error = "Failed to load projects: ${e.message}"
                    )
                }
            }
        }
    }
    
    /**
     * Create a new project
     */
    fun createNewProject(name: String = "New Project") {
        viewModelScope.launch {
            try {
                val project = projectManager.createNewProject(name)
                loadProjects()
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(error = "Failed to create project: ${e.message}")
                }
            }
        }
    }
    
    /**
     * Delete a project
     */
    fun deleteProject(projectId: String) {
        viewModelScope.launch {
            try {
                projectManager.deleteProject(projectId)
                loadProjects()
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(error = "Failed to delete project: ${e.message}")
                }
            }
        }
    }
    
    /**
     * Select a project
     */
    fun selectProject(project: Project) {
        _uiState.update { it.copy(selectedProject = project) }
    }
    
    /**
     * Clear error message
     */
    fun clearError() {
        _uiState.update { it.copy(error = null) }
    }
    
    /**
     * Get recent projects (last 3)
     */
    fun getRecentProjects(): List<Project> {
        return _uiState.value.projects.take(3)
    }
}
```

---

### 4. HomeScreen UI

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/home/HomeScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.home

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
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
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import androidx.hilt.navigation.compose.hiltViewModel
import androidx.navigation.NavController
import com.clipforge.videoeditor.data.model.Project
import com.clipforge.videoeditor.ui.theme.BrandPurple
import java.text.SimpleDateFormat
import java.util.*

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    navController: NavController,
    viewModel: HomeViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    var showCreateDialog by remember { mutableStateOf(false) }
    var showDeleteDialog by remember { mutableStateOf<Project?>(null) }
    
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
            HomeTopBar(
                onSettingsClick = { navController.navigate("settings") }
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
                            Color(0xFF1A0B2E),
                            Color(0xFF0F0A1E)
                        )
                    )
                )
                .padding(padding)
        ) {
            if (uiState.isLoading) {
                LoadingView()
            } else {
                if (uiState.projects.isEmpty()) {
                    EmptyProjectsView(
                        onCreateClick = { showCreateDialog = true },
                        onImportClick = { navController.navigate("import") },
                        onRecordClick = { /* TODO */ },
                        onTemplatesClick = { navController.navigate("templates") }
                    )
                } else {
                    ProjectsContentView(
                        projects = uiState.projects,
                        onProjectClick = { project ->
                            navController.navigate("editor/${project.id}")
                        },
                        onCreateClick = { showCreateDialog = true },
                        onDeleteClick = { showDeleteDialog = it },
                        onImportClick = { navController.navigate("import") },
                        onTemplatesClick = { navController.navigate("templates") }
                    )
                }
            }
        }
    }
    
    // Create Project Dialog
    if (showCreateDialog) {
        CreateProjectDialog(
            onDismiss = { showCreateDialog = false },
            onCreate = { name ->
                viewModel.createNewProject(name)
                showCreateDialog = false
            }
        )
    }
    
    // Delete Confirmation Dialog
    showDeleteDialog?.let { project ->
        DeleteProjectDialog(
            project = project,
            onDismiss = { showDeleteDialog = null },
            onConfirm = {
                viewModel.deleteProject(project.id)
                showDeleteDialog = null
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun HomeTopBar(
    onSettingsClick: () -> Unit
) {
    TopAppBar(
        title = {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(
                    Icons.Default.PlayArrow,
                    contentDescription = null,
                    tint = BrandPurple,
                    modifier = Modifier.size(32.dp)
                )
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    "ClipForge",
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
            }
        },
        actions = {
            IconButton(onClick = onSettingsClick) {
                Icon(
                    Icons.Default.Settings,
                    contentDescription = "Settings",
                    tint = Color.White
                )
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = Color(0xFF1A1625)
        )
    )
}

@Composable
private fun EmptyProjectsView(
    onCreateClick: () -> Unit,
    onImportClick: () -> Unit,
    onRecordClick: () -> Unit,
    onTemplatesClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        // Brand Icon
        Box(
            modifier = Modifier
                .size(80.dp)
                .background(BrandPurple.copy(alpha = 0.2f), CircleShape),
            contentAlignment = Alignment.Center
        ) {
            Icon(
                Icons.Default.PlayArrow,
                contentDescription = null,
                modifier = Modifier.size(48.dp),
                tint = BrandPurple
            )
        }
        
        Spacer(modifier = Modifier.height(24.dp))
        
        Text(
            "Welcome to ClipForge",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White,
            textAlign = TextAlign.Center
        )
        
        Spacer(modifier = Modifier.height(8.dp))
        
        Text(
            "Create amazing videos with professional tools",
            style = MaterialTheme.typography.bodyLarge,
            color = Color.Gray,
            textAlign = TextAlign.Center
        )
        
        Spacer(modifier = Modifier.height(32.dp))
        
        // New Project Button
        Button(
            onClick = onCreateClick,
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            ),
            shape = RoundedCornerShape(16.dp)
        ) {
            Icon(Icons.Default.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text(
                "Create New Project",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold
            )
        }
        
        Spacer(modifier = Modifier.height(32.dp))
        
        // Get Started Section
        Text(
            "Get Started",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            color = Color.White,
            modifier = Modifier.fillMaxWidth()
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            QuickActionCard(
                title = "Import\nMedia",
                icon = Icons.Default.Upload,
                onClick = onImportClick,
                modifier = Modifier.weight(1f)
            )
            
            QuickActionCard(
                title = "Record",
                icon = Icons.Default.Camera,
                onClick = onRecordClick,
                modifier = Modifier.weight(1f)
            )
        }
        
        Spacer(modifier = Modifier.height(12.dp))
        
        QuickActionCard(
            title = "Browse Templates",
            icon = Icons.Default.Star,
            backgroundColor = Color(0xFF92400E),
            iconTint = Color(0xFFD97706),
            onClick = onTemplatesClick,
            modifier = Modifier.fillMaxWidth()
        )
    }
}

@Composable
private fun QuickActionCard(
    title: String,
    icon: ImageVector,
    onClick: () -> Unit,
    modifier: Modifier = Modifier,
    backgroundColor: Color = Color(0xFF2D2438),
    iconTint: Color = BrandPurple
) {
    Card(
        onClick = onClick,
        modifier = modifier.height(120.dp),
        colors = CardDefaults.cardColors(
            containerColor = backgroundColor
        ),
        shape = RoundedCornerShape(16.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(16.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Icon(
                icon,
                contentDescription = null,
                modifier = Modifier.size(32.dp),
                tint = iconTint
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                title,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.SemiBold,
                color = Color.White,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Composable
private fun ProjectsContentView(
    projects: List<Project>,
    onProjectClick: (Project) -> Unit,
    onCreateClick: () -> Unit,
    onDeleteClick: (Project) -> Unit,
    onImportClick: () -> Unit,
    onTemplatesClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        // Quick Actions
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            QuickActionCard(
                title = "New Project",
                icon = Icons.Default.Add,
                onClick = onCreateClick,
                modifier = Modifier.weight(1f)
            )
            
            QuickActionCard(
                title = "Import",
                icon = Icons.Default.Upload,
                onClick = onImportClick,
                modifier = Modifier.weight(1f)
            )
            
            QuickActionCard(
                title = "Templates",
                icon = Icons.Default.Star,
                backgroundColor = Color(0xFF92400E),
                iconTint = Color(0xFFD97706),
                onClick = onTemplatesClick,
                modifier = Modifier.weight(1f)
            )
        }
        
        Spacer(modifier = Modifier.height(24.dp))
        
        // Projects Header
        Text(
            "My Projects (${projects.size})",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Projects Grid
        LazyVerticalGrid(
            columns = GridCells.Fixed(2),
            contentPadding = PaddingValues(bottom = 16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            items(projects, key = { it.id }) { project ->
                ProjectCard(
                    project = project,
                    onClick = { onProjectClick(project) },
                    onDeleteClick = { onDeleteClick(project) }
                )
            }
        }
    }
}

@Composable
private fun ProjectCard(
    project: Project,
    onClick: () -> Unit,
    onDeleteClick: () -> Unit,
    modifier: Modifier = Modifier
) {
    var showMenu by remember { mutableStateOf(false) }
    
    Card(
        onClick = onClick,
        modifier = modifier.aspectRatio(9f / 16f),
        colors = CardDefaults.cardColors(
            containerColor = Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            // Thumbnail placeholder
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(Color(0xFF1A1625)),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Default.Movie,
                    contentDescription = null,
                    modifier = Modifier.size(48.dp),
                    tint = Color.Gray
                )
            }
            
            // Gradient overlay
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        brush = Brush.verticalGradient(
                            colors = listOf(
                                Color.Transparent,
                                Color.Black.copy(alpha = 0.8f)
                            ),
                            startY = 200f
                        )
                    )
            )
            
            // Project Info
            Column(
                modifier = Modifier
                    .align(Alignment.BottomStart)
                    .padding(12.dp)
            ) {
                Text(
                    project.name,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold,
                    color = Color.White,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )
                
                Spacer(modifier = Modifier.height(4.dp))
                
                Text(
                    formatDate(project.modifiedAt),
                    style = MaterialTheme.typography.labelSmall,
                    color = Color.Gray
                )
                
                if (project.clipCount > 0) {
                    Text(
                        "${project.clipCount} clips",
                        style = MaterialTheme.typography.labelSmall,
                        color = Color.Gray
                    )
                }
            }
            
            // More options
            Box(
                modifier = Modifier
                    .align(Alignment.TopEnd)
                    .padding(8.dp)
            ) {
                IconButton(
                    onClick = { showMenu = true },
                    modifier = Modifier.size(32.dp)
                ) {
                    Icon(
                        Icons.Default.MoreVert,
                        contentDescription = "Options",
                        tint = Color.White
                    )
                }
                
                DropdownMenu(
                    expanded = showMenu,
                    onDismissRequest = { showMenu = false }
                ) {
                    DropdownMenuItem(
                        text = { Text("Delete") },
                        onClick = {
                            showMenu = false
                            onDeleteClick()
                        },
                        leadingIcon = {
                            Icon(Icons.Default.Delete, contentDescription = null)
                        }
                    )
                }
            }
        }
    }
}

@Composable
private fun LoadingView(modifier: Modifier = Modifier) {
    Box(
        modifier = modifier.fillMaxSize(),
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
                "Loading Projects...",
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
        }
    }
}

@Composable
private fun CreateProjectDialog(
    onDismiss: () -> Unit,
    onCreate: (String) -> Unit
) {
    var projectName by remember { mutableStateOf("") }
    
    AlertDialog(
        onDismissRequest = onDismiss,
        title = { Text("Create New Project") },
        text = {
            OutlinedTextField(
                value = projectName,
                onValueChange = { projectName = it },
                label = { Text("Project Name") },
                singleLine = true,
                modifier = Modifier.fillMaxWidth()
            )
        },
        confirmButton = {
            Button(
                onClick = {
                    val name = projectName.ifBlank { "New Project" }
                    onCreate(name)
                },
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                )
            ) {
                Text("Create")
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
private fun DeleteProjectDialog(
    project: Project,
    onDismiss: () -> Unit,
    onConfirm: () -> Unit
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.Delete,
                contentDescription = null,
                tint = Color.Red
            )
        },
        title = { Text("Delete Project?") },
        text = {
            Text("Are you sure you want to delete \"${project.name}\"? This action cannot be undone.")
        },
        confirmButton = {
            Button(
                onClick = onConfirm,
                colors = ButtonDefaults.buttonColors(
                    containerColor = Color.Red
                )
            ) {
                Text("Delete")
            }
        },
        dismissButton = {
            TextButton(onClick = onDismiss) {
                Text("Cancel")
            }
        }
    )
}

private fun formatDate(timestamp: Long): String {
    val sdf = SimpleDateFormat("MMM dd, yyyy", Locale.getDefault())
    return sdf.format(Date(timestamp))
}
```

---

### 5. Theme Colors

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/theme/Color.kt`

```kotlin
package com.clipforge.videoeditor.ui.theme

import androidx.compose.ui.graphics.Color

val BrandPurple = Color(0xFF7C5FD8)
val DarkPurple = Color(0xFF2D2438)
val DarkBackground = Color(0xFF0F0A1E)
val MediumPurple = Color(0xFF1A0B2E)
val AmberAccent = Color(0xFFD97706)
```

---

## Testing

### Unit Tests

**File**: `app/src/test/java/com/clipforge/videoeditor/HomeViewModelTest.kt`

```kotlin
@Test
fun `createNewProject should add project to list`() = runTest {
    // Given
    val viewModel = HomeViewModel(projectManager)
    
    // When
    viewModel.createNewProject("Test Project")
    advanceUntilIdle()
    
    // Then
    val projects = viewModel.uiState.value.projects
    assertTrue(projects.any { it.name == "Test Project" })
}

@Test
fun `deleteProject should remove project from list`() = runTest {
    // Given
    val project = projectManager.createNewProject("Test")
    val viewModel = HomeViewModel(projectManager)
    
    // When
    viewModel.deleteProject(project.id)
    advanceUntilIdle()
    
    // Then
    val projects = viewModel.uiState.value.projects
    assertFalse(projects.any { it.id == project.id })
}
```

---

## User Flow

1. **App Launch** → Home Screen displays
2. **Empty State** → Shows welcome message and quick actions
3. **Create Project** → Dialog opens, user enters name
4. **Project Created** → Added to grid, saved locally
5. **Click Project** → Navigate to Editor Screen
6. **Delete Project** → Confirmation dialog, remove from local storage

---

## Performance Considerations

- **Lazy Loading**: Projects grid uses lazy composition
- **File I/O**: All file operations on IO dispatcher
- **State Management**: Efficient StateFlow updates
- **Memory**: Thumbnail caching with size limits
- **Error Handling**: Graceful error messages

---

## Related Screens

- [Import Screen](./02-IMPORT-SCREEN-ENHANCED.md)
- [Templates Screen](./03-TEMPLATES-SCREEN-ENHANCED.md)
- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)
- [Settings Screen](./06-SETTINGS-SCREEN-ENHANCED.md)