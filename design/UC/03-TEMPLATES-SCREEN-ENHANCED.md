# 03 - Templates Screen - Complete Enhanced Documentation

## Overview
The Templates Screen provides users with pre-designed video editing templates stored **locally in app assets**. Users can browse templates by category, preview them, and apply them to their projects with **local-only storage**.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/templates/
│   ├── TemplatesScreen.kt              # Main UI Composable
│   ├── TemplatesViewModel.kt           # State management
│   ├── components/
│   │   ├── TemplateCard.kt            # Template card component
│   │   ├── TemplateDetailSheet.kt     # Detail bottom sheet
│   │   └── CategoryFilter.kt          # Category filter chips
│   └── data/
│       ├── LocalTemplateManager.kt    # Local template storage
│       └── models/Template.kt         # Template data model
├── assets/
│   └── templates/
│       ├── templates.json             # Template metadata
│       └── previews/                  # Template preview images
│           ├── template_001.jpg
│           └── template_002.jpg
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
    
    // Kotlinx Serialization
    implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.6.3")
    
    // Coil for image loading
    implementation("io.coil-kt:coil-compose:2.6.0")
}
```

---

## Complete Source Code

### 1. Template Data Model

**File**: `app/src/main/java/com/clipforge/videoeditor/data/model/Template.kt`

```kotlin
package com.clipforge.videoeditor.data.model

import kotlinx.serialization.Serializable

@Serializable
data class Template(
    val id: String,
    val name: String,
    val description: String,
    val category: TemplateCategory,
    val thumbnailPath: String,
    val duration: Long, // in milliseconds
    val aspectRatio: String = "9:16",
    val fps: Int = 30,
    val tags: List<String> = emptyList(),
    val isPremium: Boolean = false,
    val difficulty: TemplateDifficulty = TemplateDifficulty.BEGINNER,
    val effects: List<String> = emptyList(),
    val transitions: List<String> = emptyList(),
    val musicTrack: String? = null
)

@Serializable
enum class TemplateCategory {
    TRENDING,
    SOCIAL_MEDIA,
    BUSINESS,
    TRAVEL,
    FOOD,
    FASHION,
    GAMING,
    EDUCATION,
    VLOG,
    WEDDING,
    BIRTHDAY,
    ADVERTISEMENT
}

@Serializable
enum class TemplateDifficulty {
    BEGINNER,
    INTERMEDIATE,
    ADVANCED
}

@Serializable
data class TemplateList(
    val templates: List<Template> = emptyList()
)

// Extension functions
fun TemplateCategory.displayName(): String = when (this) {
    TemplateCategory.TRENDING -> "Trending"
    TemplateCategory.SOCIAL_MEDIA -> "Social Media"
    TemplateCategory.BUSINESS -> "Business"
    TemplateCategory.TRAVEL -> "Travel"
    TemplateCategory.FOOD -> "Food"
    TemplateCategory.FASHION -> "Fashion"
    TemplateCategory.GAMING -> "Gaming"
    TemplateCategory.EDUCATION -> "Education"
    TemplateCategory.VLOG -> "Vlog"
    TemplateCategory.WEDDING -> "Wedding"
    TemplateCategory.BIRTHDAY -> "Birthday"
    TemplateCategory.ADVERTISEMENT -> "Advertisement"
}
```

---

### 2. Local Template Manager

**File**: `app/src/main/java/com/clipforge/videoeditor/data/local/LocalTemplateManager.kt`

```kotlin
package com.clipforge.videoeditor.data.local

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import com.clipforge.videoeditor.data.model.Template
import com.clipforge.videoeditor.data.model.TemplateCategory
import com.clipforge.videoeditor.data.model.TemplateList
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.serialization.json.Json
import java.io.File
import java.io.IOException
import javax.inject.Inject
import javax.inject.Singleton

@Singleton
class LocalTemplateManager @Inject constructor(
    private val context: Context
) {
    
    private val json = Json {
        prettyPrint = true
        ignoreUnknownKeys = true
    }
    
    /**
     * Load all templates from assets
     */
    suspend fun getAllTemplates(): List<Template> = withContext(Dispatchers.IO) {
        try {
            val jsonString = context.assets.open("templates/templates.json")
                .bufferedReader()
                .use { it.readText() }
            
            val templateList = json.decodeFromString<TemplateList>(jsonString)
            templateList.templates
        } catch (e: IOException) {
            e.printStackTrace()
            // Return sample templates if file not found
            getSampleTemplates()
        }
    }
    
    /**
     * Get templates by category
     */
    suspend fun getTemplatesByCategory(category: TemplateCategory): List<Template> {
        return getAllTemplates().filter { it.category == category }
    }
    
    /**
     * Search templates by name or tags
     */
    suspend fun searchTemplates(query: String): List<Template> {
        return getAllTemplates().filter { template ->
            template.name.contains(query, ignoreCase = true) ||
            template.description.contains(query, ignoreCase = true) ||
            template.tags.any { it.contains(query, ignoreCase = true) }
        }
    }
    
    /**
     * Get template by ID
     */
    suspend fun getTemplateById(id: String): Template? {
        return getAllTemplates().find { it.id == id }
    }
    
    /**
     * Load template thumbnail from assets
     */
    suspend fun loadTemplateThumbnail(thumbnailPath: String): Bitmap? = withContext(Dispatchers.IO) {
        try {
            context.assets.open(thumbnailPath).use { inputStream ->
                BitmapFactory.decodeStream(inputStream)
            }
        } catch (e: IOException) {
            e.printStackTrace()
            null
        }
    }
    
    /**
     * Get trending templates (top 10)
     */
    suspend fun getTrendingTemplates(): List<Template> {
        return getAllTemplates()
            .filter { it.category == TemplateCategory.TRENDING }
            .take(10)
    }
    
    /**
     * Get premium templates
     */
    suspend fun getPremiumTemplates(): List<Template> {
        return getAllTemplates().filter { it.isPremium }
    }
    
    /**
     * Sample templates for demonstration
     */
    private fun getSampleTemplates(): List<Template> {
        return listOf(
            Template(
                id = "temp_001",
                name = "Instagram Reel",
                description = "Trendy Instagram reel template with smooth transitions",
                category = TemplateCategory.SOCIAL_MEDIA,
                thumbnailPath = "templates/previews/instagram_reel.jpg",
                duration = 15000,
                aspectRatio = "9:16",
                tags = listOf("instagram", "reel", "social"),
                effects = listOf("Fade", "Zoom", "Glitch"),
                transitions = listOf("Slide", "Fade")
            ),
            Template(
                id = "temp_002",
                name = "YouTube Intro",
                description = "Professional YouTube channel intro template",
                category = TemplateCategory.BUSINESS,
                thumbnailPath = "templates/previews/youtube_intro.jpg",
                duration = 10000,
                aspectRatio = "16:9",
                tags = listOf("youtube", "intro", "professional"),
                effects = listOf("Text Animation", "Logo Reveal"),
                transitions = listOf("Fade", "Wipe")
            ),
            Template(
                id = "temp_003",
                name = "TikTok Trend",
                description = "Viral TikTok style template with music",
                category = TemplateCategory.TRENDING,
                thumbnailPath = "templates/previews/tiktok_trend.jpg",
                duration = 20000,
                aspectRatio = "9:16",
                tags = listOf("tiktok", "viral", "trending"),
                effects = listOf("Speed Ramp", "Beat Sync", "Color Pop"),
                transitions = listOf("Flash", "Spin"),
                musicTrack = "trending_beat_001.mp3"
            ),
            Template(
                id = "temp_004",
                name = "Travel Vlog",
                description = "Beautiful travel vlog template with cinematic feel",
                category = TemplateCategory.TRAVEL,
                thumbnailPath = "templates/previews/travel_vlog.jpg",
                duration = 30000,
                aspectRatio = "16:9",
                tags = listOf("travel", "vlog", "cinematic"),
                effects = listOf("Cinematic LUT", "Slow Motion", "Parallax"),
                transitions = listOf("Cross Dissolve", "Zoom")
            ),
            Template(
                id = "temp_005",
                name = "Food Recipe",
                description = "Delicious food recipe tutorial template",
                category = TemplateCategory.FOOD,
                thumbnailPath = "templates/previews/food_recipe.jpg",
                duration = 45000,
                aspectRatio = "1:1",
                tags = listOf("food", "recipe", "cooking"),
                effects = listOf("Time Lapse", "Text Overlay", "Split Screen"),
                transitions = listOf("Cut", "Fade")
            ),
            Template(
                id = "temp_006",
                name = "Product Ad",
                description = "Modern product advertisement template",
                category = TemplateCategory.ADVERTISEMENT,
                thumbnailPath = "templates/previews/product_ad.jpg",
                duration = 15000,
                aspectRatio = "1:1",
                tags = listOf("product", "ad", "commercial"),
                effects = listOf("3D Rotation", "Shine", "Glow"),
                transitions = listOf("Push", "Slide"),
                isPremium = true
            ),
            Template(
                id = "temp_007",
                name = "Wedding Highlights",
                description = "Romantic wedding highlights template",
                category = TemplateCategory.WEDDING,
                thumbnailPath = "templates/previews/wedding_highlights.jpg",
                duration = 60000,
                aspectRatio = "16:9",
                tags = listOf("wedding", "love", "romantic"),
                effects = listOf("Soft Focus", "Vignette", "Film Grain"),
                transitions = listOf("Heart Wipe", "Fade"),
                musicTrack = "romantic_piano.mp3",
                isPremium = true
            ),
            Template(
                id = "temp_008",
                name = "Gaming Montage",
                description = "Epic gaming montage with intense effects",
                category = TemplateCategory.GAMING,
                thumbnailPath = "templates/previews/gaming_montage.jpg",
                duration = 25000,
                aspectRatio = "16:9",
                tags = listOf("gaming", "montage", "epic"),
                effects = listOf("RGB Split", "Glitch", "Shake"),
                transitions = listOf("Flash", "Zoom"),
                musicTrack = "epic_dubstep.mp3"
            )
        )
    }
}
```

---

### 3. Templates ViewModel

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/templates/TemplatesViewModel.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.templates

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.clipforge.videoeditor.data.local.LocalProjectManager
import com.clipforge.videoeditor.data.local.LocalTemplateManager
import com.clipforge.videoeditor.data.model.Template
import com.clipforge.videoeditor.data.model.TemplateCategory
import dagger.hilt.android.lifecycle.HiltViewModel
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.flow.update
import kotlinx.coroutines.launch
import javax.inject.Inject

data class TemplatesUiState(
    val templates: List<Template> = emptyList(),
    val filteredTemplates: List<Template> = emptyList(),
    val selectedCategory: TemplateCategory? = null,
    val searchQuery: String = "",
    val selectedTemplate: Template? = null,
    val isLoading: Boolean = false,
    val error: String? = null,
    val showDetailSheet: Boolean = false
)

@HiltViewModel
class TemplatesViewModel @Inject constructor(
    private val templateManager: LocalTemplateManager,
    private val projectManager: LocalProjectManager
) : ViewModel() {
    
    private val _uiState = MutableStateFlow(TemplatesUiState())
    val uiState: StateFlow<TemplatesUiState> = _uiState.asStateFlow()
    
    init {
        loadTemplates()
    }
    
    /**
     * Load all templates
     */
    private fun loadTemplates() {
        viewModelScope.launch {
            _uiState.update { it.copy(isLoading = true, error = null) }
            try {
                val templates = templateManager.getAllTemplates()
                _uiState.update {
                    it.copy(
                        templates = templates,
                        filteredTemplates = templates,
                        isLoading = false
                    )
                }
            } catch (e: Exception) {
                _uiState.update {
                    it.copy(
                        isLoading = false,
                        error = "Failed to load templates: ${e.message}"
                    )
                }
            }
        }
    }
    
    /**
     * Filter templates by category
     */
    fun filterByCategory(category: TemplateCategory?) {
        _uiState.update { state ->
            val filtered = if (category == null) {
                state.templates
            } else {
                state.templates.filter { it.category == category }
            }
            state.copy(
                selectedCategory = category,
                filteredTemplates = filtered,
                searchQuery = ""
            )
        }
    }
    
    /**
     * Search templates
     */
    fun searchTemplates(query: String) {
        _uiState.update { it.copy(searchQuery = query) }
        
        viewModelScope.launch {
            try {
                val results = if (query.isBlank()) {
                    if (_uiState.value.selectedCategory != null) {
                        templateManager.getTemplatesByCategory(_uiState.value.selectedCategory!!)
                    } else {
                        templateManager.getAllTemplates()
                    }
                } else {
                    templateManager.searchTemplates(query)
                }
                
                _uiState.update { it.copy(filteredTemplates = results) }
            } catch (e: Exception) {
                _uiState.update { it.copy(error = "Search failed: ${e.message}") }
            }
        }
    }
    
    /**
     * Select a template
     */
    fun selectTemplate(template: Template) {
        _uiState.update {
            it.copy(
                selectedTemplate = template,
                showDetailSheet = true
            )
        }
    }
    
    /**
     * Close detail sheet
     */
    fun closeDetailSheet() {
        _uiState.update {
            it.copy(
                selectedTemplate = null,
                showDetailSheet = false
            )
        }
    }
    
    /**
     * Apply template to new project
     */
    suspend fun applyTemplate(template: Template, projectName: String): String? {
        return try {
            val project = projectManager.createNewProject(projectName)
            // TODO: Apply template effects and settings to project
            project.id
        } catch (e: Exception) {
            _uiState.update { it.copy(error = "Failed to apply template: ${e.message}") }
            null
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

### 4. Templates Screen UI

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/templates/TemplatesScreen.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.templates

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyRow
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.lazy.items
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
import com.clipforge.videoeditor.data.model.Template
import com.clipforge.videoeditor.data.model.TemplateCategory
import com.clipforge.videoeditor.data.model.displayName
import com.clipforge.videoeditor.ui.theme.BrandPurple

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun TemplatesScreen(
    navController: NavController,
    viewModel: TemplatesViewModel = hiltViewModel()
) {
    val uiState by viewModel.uiState.collectAsState()
    val snackbarHostState = remember { SnackbarHostState() }
    var showApplyDialog by remember { mutableStateOf(false) }
    
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
            TemplatesTopBar(
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
            if (uiState.isLoading) {
                LoadingView()
            } else {
                Column(modifier = Modifier.fillMaxSize()) {
                    // Search bar
                    SearchBar(
                        searchQuery = uiState.searchQuery,
                        onSearchChange = { viewModel.searchTemplates(it) }
                    )
                    
                    // Category filter
                    CategoryFilterRow(
                        selectedCategory = uiState.selectedCategory,
                        onCategorySelect = { viewModel.filterByCategory(it) }
                    )
                    
                    // Templates grid
                    if (uiState.filteredTemplates.isEmpty()) {
                        EmptyTemplatesView()
                    } else {
                        TemplatesGrid(
                            templates = uiState.filteredTemplates,
                            onTemplateClick = { viewModel.selectTemplate(it) }
                        )
                    }
                }
            }
        }
    }
    
    // Template detail sheet
    if (uiState.showDetailSheet && uiState.selectedTemplate != null) {
        TemplateDetailSheet(
            template = uiState.selectedTemplate!!,
            onDismiss = { viewModel.closeDetailSheet() },
            onApply = { showApplyDialog = true }
        )
    }
    
    // Apply template dialog
    if (showApplyDialog) {
        ApplyTemplateDialog(
            template = uiState.selectedTemplate!!,
            onDismiss = { showApplyDialog = false },
            onConfirm = { projectName ->
                showApplyDialog = false
                viewModel.closeDetailSheet()
                // TODO: Navigate to editor
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun TemplatesTopBar(
    onBackClick: () -> Unit
) {
    TopAppBar(
        title = {
            Text(
                "Templates",
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
private fun SearchBar(
    searchQuery: String,
    onSearchChange: (String) -> Unit
) {
    OutlinedTextField(
        value = searchQuery,
        onValueChange = onSearchChange,
        modifier = Modifier
            .fillMaxWidth()
            .padding(16.dp),
        placeholder = { Text("Search templates...") },
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
}

@Composable
private fun CategoryFilterRow(
    selectedCategory: TemplateCategory?,
    onCategorySelect: (TemplateCategory?) -> Unit
) {
    LazyRow(
        contentPadding = PaddingValues(horizontal = 16.dp),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        // All category
        item {
            FilterChip(
                selected = selectedCategory == null,
                onClick = { onCategorySelect(null) },
                label = { Text("All") },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
        
        // Category chips
        items(TemplateCategory.values()) { category ->
            FilterChip(
                selected = selectedCategory == category,
                onClick = { onCategorySelect(category) },
                label = { Text(category.displayName()) },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
    }
}

@Composable
private fun TemplatesGrid(
    templates: List<Template>,
    onTemplateClick: (Template) -> Unit
) {
    LazyVerticalGrid(
        columns = GridCells.Fixed(2),
        contentPadding = PaddingValues(16.dp),
        horizontalArrangement = Arrangement.spacedBy(12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        items(templates, key = { it.id }) { template ->
            TemplateCard(
                template = template,
                onClick = { onTemplateClick(template) }
            )
        }
    }
}

@Composable
private fun TemplateCard(
    template: Template,
    onClick: () -> Unit
) {
    Card(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .aspectRatio(9f / 16f),
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
                    Icons.Default.VideoLibrary,
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
            
            // Premium badge
            if (template.isPremium) {
                Surface(
                    modifier = Modifier
                        .align(Alignment.TopEnd)
                        .padding(8.dp),
                    color = Color(0xFFD97706),
                    shape = RoundedCornerShape(4.dp)
                ) {
                    Row(
                        modifier = Modifier.padding(horizontal = 6.dp, vertical = 3.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Icon(
                            Icons.Default.Star,
                            contentDescription = null,
                            modifier = Modifier.size(12.dp),
                            tint = Color.White
                        )
                        Spacer(modifier = Modifier.width(4.dp))
                        Text(
                            "PRO",
                            style = MaterialTheme.typography.labelSmall,
                            color = Color.White,
                            fontWeight = FontWeight.Bold
                        )
                    }
                }
            }
            
            // Template info
            Column(
                modifier = Modifier
                    .align(Alignment.BottomStart)
                    .padding(12.dp)
            ) {
                Text(
                    template.name,
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold,
                    color = Color.White,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )
                
                Spacer(modifier = Modifier.height(4.dp))
                
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        Icons.Default.Timer,
                        contentDescription = null,
                        modifier = Modifier.size(14.dp),
                        tint = Color.Gray
                    )
                    Spacer(modifier = Modifier.width(4.dp))
                    Text(
                        "${template.duration / 1000}s",
                        style = MaterialTheme.typography.labelSmall,
                        color = Color.Gray
                    )
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun TemplateDetailSheet(
    template: Template,
    onDismiss: () -> Unit,
    onApply: () -> Unit
) {
    ModalBottomSheet(
        onDismissRequest = onDismiss,
        containerColor = Color(0xFF1A1625)
    ) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(24.dp)
        ) {
            // Thumbnail
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .aspectRatio(16f / 9f)
                    .clip(RoundedCornerShape(12.dp))
                    .background(Color(0xFF2D2438)),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Default.PlayCircle,
                    contentDescription = null,
                    modifier = Modifier.size(64.dp),
                    tint = BrandPurple
                )
            }
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Title
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    template.name,
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                
                if (template.isPremium) {
                    Surface(
                        color = Color(0xFFD97706),
                        shape = RoundedCornerShape(8.dp)
                    ) {
                        Row(
                            modifier = Modifier.padding(horizontal = 8.dp, vertical = 4.dp),
                            verticalAlignment = Alignment.CenterVertically
                        ) {
                            Icon(
                                Icons.Default.Star,
                                contentDescription = null,
                                modifier = Modifier.size(16.dp),
                                tint = Color.White
                            )
                            Spacer(modifier = Modifier.width(4.dp))
                            Text(
                                "PRO",
                                style = MaterialTheme.typography.labelMedium,
                                color = Color.White,
                                fontWeight = FontWeight.Bold
                            )
                        }
                    }
                }
            }
            
            Spacer(modifier = Modifier.height(8.dp))
            
            // Category and duration
            Row(
                horizontalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Chip(text = template.category.displayName())
                Chip(text = "${template.duration / 1000}s")
                Chip(text = template.aspectRatio)
            }
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Description
            Text(
                template.description,
                style = MaterialTheme.typography.bodyMedium,
                color = Color.Gray
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Effects
            if (template.effects.isNotEmpty()) {
                Text(
                    "Effects",
                    style = MaterialTheme.typography.titleSmall,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    template.effects.joinToString(" • "),
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
                Spacer(modifier = Modifier.height(16.dp))
            }
            
            // Apply button
            Button(
                onClick = onApply,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp),
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                ),
                shape = RoundedCornerShape(12.dp)
            ) {
                Icon(Icons.Default.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(
                    "Use Template",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
private fun Chip(text: String) {
    Surface(
        color = Color(0xFF2D2438),
        shape = RoundedCornerShape(8.dp)
    ) {
        Text(
            text,
            modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
            style = MaterialTheme.typography.labelSmall,
            color = Color.White
        )
    }
}

@Composable
private fun ApplyTemplateDialog(
    template: Template,
    onDismiss: () -> Unit,
    onConfirm: (String) -> Unit
) {
    var projectName by remember { mutableStateOf("") }
    
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.VideoLibrary,
                contentDescription = null,
                tint = BrandPurple
            )
        },
        title = { Text("Apply Template") },
        text = {
            Column {
                Text("Create a new project with \"${template.name}\" template")
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
                    val name = projectName.ifBlank { template.name }
                    onConfirm(name)
                },
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple
                )
            ) {
                Text("Create Project")
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
                "Loading Templates...",
                style = MaterialTheme.typography.bodyLarge,
                color = Color.White
            )
        }
    }
}

@Composable
private fun EmptyTemplatesView() {
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
                "No Templates Found",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White,
                textAlign = TextAlign.Center
            )
            Text(
                "Try a different search or category",
                style = MaterialTheme.typography.bodyMedium,
                color = Color.Gray,
                textAlign = TextAlign.Center
            )
        }
    }
}
```

---

## Sample templates.json

**File**: `app/src/main/assets/templates/templates.json`

```json
{
  "templates": [
    {
      "id": "temp_001",
      "name": "Instagram Reel",
      "description": "Trendy Instagram reel template with smooth transitions",
      "category": "SOCIAL_MEDIA",
      "thumbnailPath": "templates/previews/instagram_reel.jpg",
      "duration": 15000,
      "aspectRatio": "9:16",
      "fps": 30,
      "tags": ["instagram", "reel", "social"],
      "isPremium": false,
      "difficulty": "BEGINNER",
      "effects": ["Fade", "Zoom", "Glitch"],
      "transitions": ["Slide", "Fade"]
    }
  ]
}
```

---

## User Flow

1. **Navigate to Templates** → Grid of templates displayed
2. **Browse Categories** → Filter chips for quick filtering
3. **Search** → Type to search templates
4. **Tap Template** → Detail sheet opens
5. **Preview** → See template details and effects
6. **Apply** → Create project dialog
7. **Create Project** → Navigate to editor with template

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should filter templates by category`() = runTest {
    viewModel.filterByCategory(TemplateCategory.SOCIAL_MEDIA)
    
    val filtered = viewModel.uiState.value.filteredTemplates
    assertTrue(filtered.all { it.category == TemplateCategory.SOCIAL_MEDIA })
}
```

---

## Related Screens

- [Home Screen](./01-HOME-SCREEN-ENHANCED.md)
- [Import Screen](./02-IMPORT-SCREEN-ENHANCED.md)
- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)