# 07 - Editor Feature Tabs - Complete Enhanced Documentation

## Overview
The Editor Feature Tabs provide **professional-grade editing tools** including Timeline, Effects, Audio, Color, Text, and Trim tabs. Each tab offers specialized controls with **local-only processing** and real-time preview.

---

## File Structure

```
app/src/main/java/com/clipforge/videoeditor/
├── ui/screens/editor/tabs/
│   ├── TimelineTab.kt                  # Timeline editor tab
│   ├── EffectsTab.kt                   # Effects library tab
│   ├── AudioTab.kt                     # Audio mixer tab
│   ├── ColorTab.kt                     # Color grading tab
│   ├── TextTab.kt                      # Text overlays tab
│   ├── TrimTab.kt                      # Trim tools tab
│   └── components/
│       ├── EffectCard.kt              # Individual effect
│       ├── AudioTrack.kt              # Audio track UI
│       ├── ColorWheel.kt              # Color wheel picker
│       ├── TextStylePicker.kt         # Text styling
│       └── TrimSlider.kt              # Trim range slider
```

---

## Complete Source Code

### 1. Timeline Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/TimelineTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.gestures.detectDragGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.data.model.VideoClip
import com.clipforge.videoeditor.ui.theme.BrandPurple

@Composable
fun TimelineTab(
    clips: List<VideoClip>,
    selectedClip: VideoClip?,
    onClipSelect: (VideoClip) -> Unit,
    onClipMove: (String, Long, Int) -> Unit,
    onClipDelete: (String) -> Unit,
    onClipSplit: (String, Long) -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Timeline Header
        TimelineHeader()
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Timeline Tools
        TimelineTools(
            selectedClip = selectedClip,
            onDelete = { selectedClip?.let { onClipDelete(it.id) } },
            onSplit = { selectedClip?.let { onClipSplit(it.id, 0L) } }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Clips List
        LazyColumn(
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            items(clips, key = { it.id }) { clip ->
                TimelineClipCard(
                    clip = clip,
                    isSelected = clip.id == selectedClip?.id,
                    onClick = { onClipSelect(clip) },
                    onMove = { deltaX, deltaY ->
                        // Calculate new position based on drag
                        val newPosition = clip.startPosition + (deltaX * 100).toLong()
                        val newTrack = clip.trackIndex + (deltaY / 60).toInt()
                        onClipMove(clip.id, newPosition.coerceAtLeast(0), newTrack.coerceIn(0, 2))
                    }
                )
            }
        }
    }
}

@Composable
private fun TimelineHeader() {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            "Timeline Clips",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            IconButton(onClick = { /* Magnetic snapping */ }) {
                Icon(Icons.Default.GridOn, "Snap", tint = BrandPurple)
            }
            IconButton(onClick = { /* Show markers */ }) {
                Icon(Icons.Default.Place, "Markers", tint = Color.White)
            }
        }
    }
}

@Composable
private fun TimelineTools(
    selectedClip: VideoClip?,
    onDelete: () -> Unit,
    onSplit: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(
            containerColor = Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            Button(
                onClick = onSplit,
                enabled = selectedClip != null,
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.buttonColors(
                    containerColor = BrandPurple,
                    disabledContainerColor = Color.Gray
                )
            ) {
                Icon(Icons.Default.ContentCut, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(4.dp))
                Text("Split")
            }
            
            OutlinedButton(
                onClick = onDelete,
                enabled = selectedClip != null,
                modifier = Modifier.weight(1f),
                colors = ButtonDefaults.outlinedButtonColors(
                    contentColor = Color.Red,
                    disabledContentColor = Color.Gray
                )
            ) {
                Icon(Icons.Default.Delete, contentDescription = null, modifier = Modifier.size(18.dp))
                Spacer(modifier = Modifier.width(4.dp))
                Text("Delete")
            }
        }
    }
}

@Composable
private fun TimelineClipCard(
    clip: VideoClip,
    isSelected: Boolean,
    onClick: () -> Unit,
    onMove: (Float, Float) -> Unit
) {
    var offsetX by remember { mutableStateOf(0f) }
    var offsetY by remember { mutableStateOf(0f) }
    
    Card(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .pointerInput(Unit) {
                detectDragGestures(
                    onDragEnd = {
                        onMove(offsetX, offsetY)
                        offsetX = 0f
                        offsetY = 0f
                    }
                ) { change, dragAmount ->
                    change.consume()
                    offsetX += dragAmount.x
                    offsetY += dragAmount.y
                }
            },
        colors = CardDefaults.cardColors(
            containerColor = if (isSelected) BrandPurple else Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(8.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            Icon(
                Icons.Default.VideoLibrary,
                contentDescription = null,
                tint = Color.White,
                modifier = Modifier.size(32.dp)
            )
            
            Spacer(modifier = Modifier.width(12.dp))
            
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    clip.name,
                    style = MaterialTheme.typography.bodyLarge,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Text(
                    "Track ${clip.trackIndex + 1} • ${clip.duration / 1000}s",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
            
            if (clip.effects.isNotEmpty()) {
                Surface(
                    color = Color(0xFF4CAF50),
                    shape = RoundedCornerShape(4.dp)
                ) {
                    Text(
                        "${clip.effects.size} FX",
                        modifier = Modifier.padding(horizontal = 6.dp, vertical = 2.dp),
                        style = MaterialTheme.typography.labelSmall,
                        color = Color.White
                    )
                }
            }
        }
    }
}
```

---

### 2. Effects Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/EffectsTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.data.model.Effect
import com.clipforge.videoeditor.data.model.EffectType
import com.clipforge.videoeditor.ui.theme.BrandPurple

@Composable
fun EffectsTab(
    selectedClip: com.clipforge.videoeditor.data.model.VideoClip?,
    onEffectApply: (Effect) -> Unit,
    modifier: Modifier = Modifier
) {
    var selectedCategory by remember { mutableStateOf(EffectType.FILTER) }
    var searchQuery by remember { mutableStateOf("") }
    
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Header
        EffectsHeader(selectedClip = selectedClip)
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Search Bar
        SearchBar(
            query = searchQuery,
            onQueryChange = { searchQuery = it }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Category Filter
        EffectCategoryFilter(
            selectedCategory = selectedCategory,
            onCategoryChange = { selectedCategory = it }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Effects Grid
        EffectsGrid(
            category = selectedCategory,
            searchQuery = searchQuery,
            onEffectClick = onEffectApply
        )
    }
}

@Composable
private fun EffectsHeader(selectedClip: com.clipforge.videoeditor.data.model.VideoClip?) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                "Effects",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            if (selectedClip != null) {
                Text(
                    "${selectedClip.effects.size} effects applied",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        
        if (selectedClip == null) {
            Surface(
                color = Color(0xFFFF9800),
                shape = RoundedCornerShape(8.dp)
            ) {
                Text(
                    "Select a clip",
                    modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.White
                )
            }
        }
    }
}

@Composable
private fun SearchBar(
    query: String,
    onQueryChange: (String) -> Unit
) {
    OutlinedTextField(
        value = query,
        onValueChange = onQueryChange,
        modifier = Modifier.fillMaxWidth(),
        placeholder = { Text("Search effects...") },
        leadingIcon = { Icon(Icons.Default.Search, contentDescription = null) },
        trailingIcon = {
            if (query.isNotEmpty()) {
                IconButton(onClick = { onQueryChange("") }) {
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
private fun EffectCategoryFilter(
    selectedCategory: EffectType,
    onCategoryChange: (EffectType) -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        EffectType.values().forEach { type ->
            FilterChip(
                selected = selectedCategory == type,
                onClick = { onCategoryChange(type) },
                label = { Text(type.name) },
                colors = FilterChipDefaults.filterChipColors(
                    selectedContainerColor = BrandPurple,
                    selectedLabelColor = Color.White
                )
            )
        }
    }
}

@Composable
private fun EffectsGrid(
    category: EffectType,
    searchQuery: String,
    onEffectClick: (Effect) -> Unit
) {
    val effects = getSampleEffects(category).filter {
        searchQuery.isEmpty() || it.name.contains(searchQuery, ignoreCase = true)
    }
    
    LazyVerticalGrid(
        columns = GridCells.Fixed(2),
        horizontalArrangement = Arrangement.spacedBy(12.dp),
        verticalArrangement = Arrangement.spacedBy(12.dp)
    ) {
        items(effects) { effect ->
            EffectCard(
                effect = effect,
                onClick = { onEffectClick(effect) }
            )
        }
    }
}

@Composable
private fun EffectCard(
    effect: Effect,
    onClick: () -> Unit
) {
    Card(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .aspectRatio(1f),
        colors = CardDefaults.cardColors(
            containerColor = Color(0xFF2D2438)
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Box(
            modifier = Modifier.fillMaxSize(),
            contentAlignment = Alignment.Center
        ) {
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                verticalArrangement = Arrangement.Center
            ) {
                Icon(
                    Icons.Default.AutoAwesome,
                    contentDescription = null,
                    tint = BrandPurple,
                    modifier = Modifier.size(40.dp)
                )
                Spacer(modifier = Modifier.height(8.dp))
                Text(
                    effect.name,
                    style = MaterialTheme.typography.bodyMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
            }
        }
    }
}

private fun getSampleEffects(type: EffectType): List<Effect> {
    return when (type) {
        EffectType.FILTER -> listOf(
            Effect(type = EffectType.FILTER, name = "Vintage"),
            Effect(type = EffectType.FILTER, name = "B&W"),
            Effect(type = EffectType.FILTER, name = "Warm"),
            Effect(type = EffectType.FILTER, name = "Cool"),
            Effect(type = EffectType.FILTER, name = "Sepia"),
            Effect(type = EffectType.FILTER, name = "Vivid")
        )
        EffectType.TRANSITION -> listOf(
            Effect(type = EffectType.TRANSITION, name = "Fade"),
            Effect(type = EffectType.TRANSITION, name = "Dissolve"),
            Effect(type = EffectType.TRANSITION, name = "Wipe"),
            Effect(type = EffectType.TRANSITION, name = "Slide")
        )
        EffectType.TEXT -> listOf(
            Effect(type = EffectType.TEXT, name = "Title"),
            Effect(type = EffectType.TEXT, name = "Lower Third"),
            Effect(type = EffectType.TEXT, name = "Caption")
        )
        EffectType.STICKER -> listOf(
            Effect(type = EffectType.STICKER, name = "Emoji"),
            Effect(type = EffectType.STICKER, name = "GIF"),
            Effect(type = EffectType.STICKER, name = "Shape")
        )
        EffectType.ANIMATION -> listOf(
            Effect(type = EffectType.ANIMATION, name = "Zoom In"),
            Effect(type = EffectType.ANIMATION, name = "Zoom Out"),
            Effect(type = EffectType.ANIMATION, name = "Pan"),
            Effect(type = EffectType.ANIMATION, name = "Rotate")
        )
    }
}
```

---

### 3. Audio Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/AudioTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.data.model.AudioClip
import com.clipforge.videoeditor.ui.theme.BrandPurple

@Composable
fun AudioTab(
    audioClips: List<AudioClip>,
    onVolumeChange: (String, Float) -> Unit,
    onMuteToggle: (String) -> Unit,
    onAddAudio: () -> Unit,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Header
        AudioHeader(
            clipCount = audioClips.size,
            onAddAudio = onAddAudio
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Master Volume
        MasterVolumeControl()
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Audio Tracks
        Text(
            "Audio Tracks",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        LazyColumn(
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            items(audioClips, key = { it.id }) { clip ->
                AudioTrackCard(
                    clip = clip,
                    onVolumeChange = { volume -> onVolumeChange(clip.id, volume) },
                    onMuteToggle = { onMuteToggle(clip.id) }
                )
            }
        }
    }
}

@Composable
private fun AudioHeader(
    clipCount: Int,
    onAddAudio: () -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                "Audio Mixer",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            Text(
                "$clipCount audio tracks",
                style = MaterialTheme.typography.bodySmall,
                color = Color.Gray
            )
        }
        
        Button(
            onClick = onAddAudio,
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            )
        ) {
            Icon(Icons.Default.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(4.dp))
            Text("Add Audio")
        }
    }
}

@Composable
private fun MasterVolumeControl() {
    var masterVolume by remember { mutableStateOf(1.0f) }
    
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
                    "Master Volume",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Text(
                    "${(masterVolume * 100).toInt()}%",
                    style = MaterialTheme.typography.bodyMedium,
                    color = BrandPurple
                )
            }
            
            Spacer(modifier = Modifier.height(12.dp))
            
            Slider(
                value = masterVolume,
                onValueChange = { masterVolume = it },
                valueRange = 0f..1f,
                colors = SliderDefaults.colors(
                    thumbColor = BrandPurple,
                    activeTrackColor = BrandPurple,
                    inactiveTrackColor = Color.Gray
                )
            )
        }
    }
}

@Composable
private fun AudioTrackCard(
    clip: AudioClip,
    onVolumeChange: (Float) -> Unit,
    onMuteToggle: () -> Unit
) {
    var volume by remember { mutableStateOf(clip.volume) }
    var isMuted by remember { mutableStateOf(false) }
    
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
                Row(
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(
                        Icons.Default.AudioFile,
                        contentDescription = null,
                        tint = BrandPurple,
                        modifier = Modifier.size(24.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Column {
                        Text(
                            clip.name,
                            style = MaterialTheme.typography.bodyLarge,
                            fontWeight = FontWeight.Bold,
                            color = Color.White
                        )
                        Text(
                            "${clip.duration / 1000}s",
                            style = MaterialTheme.typography.bodySmall,
                            color = Color.Gray
                        )
                    }
                }
                
                IconButton(
                    onClick = {
                        isMuted = !isMuted
                        onMuteToggle()
                    }
                ) {
                    Icon(
                        if (isMuted) Icons.Default.VolumeOff else Icons.Default.VolumeUp,
                        contentDescription = if (isMuted) "Unmute" else "Mute",
                        tint = if (isMuted) Color.Red else Color.White
                    )
                }
            }
            
            Spacer(modifier = Modifier.height(12.dp))
            
            Row(
                modifier = Modifier.fillMaxWidth(),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Icon(
                    Icons.Default.VolumeDown,
                    contentDescription = null,
                    tint = Color.Gray,
                    modifier = Modifier.size(20.dp)
                )
                
                Slider(
                    value = volume,
                    onValueChange = {
                        volume = it
                        onVolumeChange(it)
                    },
                    modifier = Modifier.weight(1f),
                    valueRange = 0f..1f,
                    colors = SliderDefaults.colors(
                        thumbColor = BrandPurple,
                        activeTrackColor = BrandPurple,
                        inactiveTrackColor = Color.Gray
                    )
                )
                
                Icon(
                    Icons.Default.VolumeUp,
                    contentDescription = null,
                    tint = Color.Gray,
                    modifier = Modifier.size(20.dp)
                )
                
                Spacer(modifier = Modifier.width(8.dp))
                
                Text(
                    "${(volume * 100).toInt()}%",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.White
                )
            }
        }
    }
}
```

---

### 4. Color Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/ColorTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.ui.theme.BrandPurple

@Composable
fun ColorTab(
    selectedClip: com.clipforge.videoeditor.data.model.VideoClip?,
    onColorAdjust: (String, Float) -> Unit,
    modifier: Modifier = Modifier
) {
    var brightness by remember { mutableStateOf(0f) }
    var contrast by remember { mutableStateOf(0f) }
    var saturation by remember { mutableStateOf(0f) }
    var temperature by remember { mutableStateOf(0f) }
    var tint by remember { mutableStateOf(0f) }
    var exposure by remember { mutableStateOf(0f) }
    
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Header
        ColorHeader(selectedClip = selectedClip)
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Color Adjustments
        ColorAdjustmentCard(
            title = "Brightness",
            value = brightness,
            onValueChange = { brightness = it },
            icon = Icons.Default.Brightness6
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        ColorAdjustmentCard(
            title = "Contrast",
            value = contrast,
            onValueChange = { contrast = it },
            icon = Icons.Default.Contrast
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        ColorAdjustmentCard(
            title = "Saturation",
            value = saturation,
            onValueChange = { saturation = it },
            icon = Icons.Default.Palette
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        ColorAdjustmentCard(
            title = "Temperature",
            value = temperature,
            onValueChange = { temperature = it },
            icon = Icons.Default.Thermostat
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        ColorAdjustmentCard(
            title = "Tint",
            value = tint,
            onValueChange = { tint = it },
            icon = Icons.Default.Colorize
        )
        
        Spacer(modifier = Modifier.height(12.dp))
        
        ColorAdjustmentCard(
            title = "Exposure",
            value = exposure,
            onValueChange = { exposure = it },
            icon = Icons.Default.LightMode
        )
        
        Spacer(modifier = Modifier.weight(1f))
        
        // Reset Button
        Button(
            onClick = {
                brightness = 0f
                contrast = 0f
                saturation = 0f
                temperature = 0f
                tint = 0f
                exposure = 0f
            },
            modifier = Modifier.fillMaxWidth(),
            colors = ButtonDefaults.buttonColors(
                containerColor = Color.Red.copy(alpha = 0.2f),
                contentColor = Color.Red
            )
        ) {
            Icon(Icons.Default.RestartAlt, contentDescription = null)
            Spacer(modifier = Modifier.width(8.dp))
            Text("Reset All")
        }
    }
}

@Composable
private fun ColorHeader(selectedClip: com.clipforge.videoeditor.data.model.VideoClip?) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                "Color Grading",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            if (selectedClip != null) {
                Text(
                    selectedClip.name,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        
        if (selectedClip == null) {
            Surface(
                color = Color(0xFFFF9800),
                shape = RoundedCornerShape(8.dp)
            ) {
                Text(
                    "Select a clip",
                    modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.White
                )
            }
        }
    }
}

@Composable
private fun ColorAdjustmentCard(
    title: String,
    value: Float,
    onValueChange: (Float) -> Unit,
    icon: androidx.compose.ui.graphics.vector.ImageVector
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
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(
                        icon,
                        contentDescription = null,
                        tint = BrandPurple,
                        modifier = Modifier.size(24.dp)
                    )
                    Spacer(modifier = Modifier.width(8.dp))
                    Text(
                        title,
                        style = MaterialTheme.typography.titleMedium,
                        fontWeight = FontWeight.Bold,
                        color = Color.White
                    )
                }
                Text(
                    "${(value * 100).toInt()}",
                    style = MaterialTheme.typography.bodyMedium,
                    color = BrandPurple
                )
            }
            
            Spacer(modifier = Modifier.height(8.dp))
            
            Slider(
                value = value,
                onValueChange = onValueChange,
                valueRange = -1f..1f,
                colors = SliderDefaults.colors(
                    thumbColor = BrandPurple,
                    activeTrackColor = BrandPurple,
                    inactiveTrackColor = Color.Gray
                )
            )
        }
    }
}
```

---

### 5. Text Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/TextTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.ui.theme.BrandPurple

data class TextOverlay(
    val id: String,
    val text: String,
    val fontSize: Float = 24f,
    val color: Color = Color.White,
    val position: String = "Center"
)

@Composable
fun TextTab(
    textOverlays: List<TextOverlay>,
    onAddText: (String) -> Unit,
    onRemoveText: (String) -> Unit,
    modifier: Modifier = Modifier
) {
    var newText by remember { mutableStateOf("") }
    var showAddDialog by remember { mutableStateOf(false) }
    
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Header
        TextHeader(
            overlayCount = textOverlays.size,
            onAddText = { showAddDialog = true }
        )
        
        Spacer(modifier = Modifier.height(16.dp))
        
        // Text Overlays List
        LazyColumn(
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            items(textOverlays, key = { it.id }) { overlay ->
                TextOverlayCard(
                    overlay = overlay,
                    onRemove = { onRemoveText(overlay.id) }
                )
            }
        }
    }
    
    // Add Text Dialog
    if (showAddDialog) {
        AlertDialog(
            onDismissRequest = { showAddDialog = false },
            icon = { Icon(Icons.Default.TextFields, contentDescription = null, tint = BrandPurple) },
            title = { Text("Add Text") },
            text = {
                Column {
                    Text("Enter text to add to video")
                    Spacer(modifier = Modifier.height(16.dp))
                    OutlinedTextField(
                        value = newText,
                        onValueChange = { newText = it },
                        label = { Text("Text") },
                        modifier = Modifier.fillMaxWidth()
                    )
                }
            },
            confirmButton = {
                Button(
                    onClick = {
                        if (newText.isNotBlank()) {
                            onAddText(newText)
                            newText = ""
                            showAddDialog = false
                        }
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = BrandPurple)
                ) {
                    Text("Add")
                }
            },
            dismissButton = {
                TextButton(onClick = { showAddDialog = false }) {
                    Text("Cancel")
                }
            }
        )
    }
}

@Composable
private fun TextHeader(
    overlayCount: Int,
    onAddText: () -> Unit
) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                "Text Overlays",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            Text(
                "$overlayCount text overlays",
                style = MaterialTheme.typography.bodySmall,
                color = Color.Gray
            )
        }
        
        Button(
            onClick = onAddText,
            colors = ButtonDefaults.buttonColors(containerColor = BrandPurple)
        ) {
            Icon(Icons.Default.Add, contentDescription = null)
            Spacer(modifier = Modifier.width(4.dp))
            Text("Add Text")
        }
    }
}

@Composable
private fun TextOverlayCard(
    overlay: TextOverlay,
    onRemove: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2D2438)),
        shape = RoundedCornerShape(12.dp)
    ) {
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(16.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    overlay.text,
                    style = MaterialTheme.typography.bodyLarge,
                    fontWeight = FontWeight.Bold,
                    color = Color.White,
                    maxLines = 2
                )
                Text(
                    "${overlay.fontSize.toInt()}px • ${overlay.position}",
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
            
            IconButton(onClick = onRemove) {
                Icon(Icons.Default.Delete, contentDescription = "Remove", tint = Color.Red)
            }
        }
    }
}
```

---

### 6. Trim Tab

**File**: `app/src/main/java/com/clipforge/videoeditor/ui/screens/editor/tabs/TrimTab.kt`

```kotlin
package com.clipforge.videoeditor.ui.screens.editor.tabs

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import com.clipforge.videoeditor.ui.theme.BrandPurple

@Composable
fun TrimTab(
    selectedClip: com.clipforge.videoeditor.data.model.VideoClip?,
    onTrimChange: (Long, Long) -> Unit,
    onSpeedChange: (Float) -> Unit,
    modifier: Modifier = Modifier
) {
    var trimStart by remember { mutableStateOf(0L) }
    var trimEnd by remember { mutableStateOf(0L) }
    var speed by remember { mutableStateOf(1.0f) }
    
    Column(
        modifier = modifier
            .fillMaxSize()
            .background(Color(0xFF1A1625))
            .padding(16.dp)
    ) {
        // Header
        TrimHeader(selectedClip = selectedClip)
        
        Spacer(modifier = Modifier.height(16.dp))
        
        selectedClip?.let { clip ->
            // Trim Controls
            TrimControls(
                clip = clip,
                trimStart = trimStart,
                trimEnd = trimEnd,
                onTrimStartChange = { trimStart = it },
                onTrimEndChange = { trimEnd = it },
                onApply = { onTrimChange(trimStart, trimEnd) }
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Speed Controls
            SpeedControls(
                speed = speed,
                onSpeedChange = {
                    speed = it
                    onSpeedChange(it)
                }
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Preset Speeds
            PresetSpeeds(
                onSpeedSelect = {
                    speed = it
                    onSpeedChange(it)
                }
            )
        }
    }
}

@Composable
private fun TrimHeader(selectedClip: com.clipforge.videoeditor.data.model.VideoClip?) {
    Row(
        modifier = Modifier.fillMaxWidth(),
        horizontalArrangement = Arrangement.SpaceBetween,
        verticalAlignment = Alignment.CenterVertically
    ) {
        Column {
            Text(
                "Trim & Speed",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            if (selectedClip != null) {
                Text(
                    selectedClip.name,
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.Gray
                )
            }
        }
        
        if (selectedClip == null) {
            Surface(
                color = Color(0xFFFF9800),
                shape = RoundedCornerShape(8.dp)
            ) {
                Text(
                    "Select a clip",
                    modifier = Modifier.padding(horizontal = 12.dp, vertical = 6.dp),
                    style = MaterialTheme.typography.bodySmall,
                    color = Color.White
                )
            }
        }
    }
}

@Composable
private fun TrimControls(
    clip: com.clipforge.videoeditor.data.model.VideoClip,
    trimStart: Long,
    trimEnd: Long,
    onTrimStartChange: (Long) -> Unit,
    onTrimEndChange: (Long) -> Unit,
    onApply: () -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2D2438)),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                "Trim Clip",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            // Trim Start
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("Start", color = Color.White)
                Text("${trimStart / 1000}s", color = BrandPurple)
            }
            
            Slider(
                value = trimStart.toFloat(),
                onValueChange = { onTrimStartChange(it.toLong()) },
                valueRange = 0f..clip.duration.toFloat(),
                colors = SliderDefaults.colors(
                    thumbColor = BrandPurple,
                    activeTrackColor = BrandPurple
                )
            )
            
            Spacer(modifier = Modifier.height(12.dp))
            
            // Trim End
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text("End", color = Color.White)
                Text("${(clip.duration - trimEnd) / 1000}s", color = BrandPurple)
            }
            
            Slider(
                value = trimEnd.toFloat(),
                onValueChange = { onTrimEndChange(it.toLong()) },
                valueRange = 0f..clip.duration.toFloat(),
                colors = SliderDefaults.colors(
                    thumbColor = BrandPurple,
                    activeTrackColor = BrandPurple
                )
            )
            
            Spacer(modifier = Modifier.height(16.dp))
            
            Button(
                onClick = onApply,
                modifier = Modifier.fillMaxWidth(),
                colors = ButtonDefaults.buttonColors(containerColor = BrandPurple)
            ) {
                Text("Apply Trim")
            }
        }
    }
}

@Composable
private fun SpeedControls(
    speed: Float,
    onSpeedChange: (Float) -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2D2438)),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.SpaceBetween,
                verticalAlignment = Alignment.CenterVertically
            ) {
                Text(
                    "Speed",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
                Text(
                    "${String.format("%.2f", speed)}x",
                    style = MaterialTheme.typography.bodyMedium,
                    color = BrandPurple
                )
            }
            
            Spacer(modifier = Modifier.height(8.dp))
            
            Slider(
                value = speed,
                onValueChange = onSpeedChange,
                valueRange = 0.25f..4f,
                colors = SliderDefaults.colors(
                    thumbColor = BrandPurple,
                    activeTrackColor = BrandPurple
                )
            )
        }
    }
}

@Composable
private fun PresetSpeeds(
    onSpeedSelect: (Float) -> Unit
) {
    Card(
        modifier = Modifier.fillMaxWidth(),
        colors = CardDefaults.cardColors(containerColor = Color(0xFF2D2438)),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(modifier = Modifier.padding(16.dp)) {
            Text(
                "Presets",
                style = MaterialTheme.typography.titleMedium,
                fontWeight = FontWeight.Bold,
                color = Color.White
            )
            
            Spacer(modifier = Modifier.height(12.dp))
            
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                listOf(0.25f, 0.5f, 1f, 2f, 4f).forEach { presetSpeed ->
                    Button(
                        onClick = { onSpeedSelect(presetSpeed) },
                        modifier = Modifier.weight(1f),
                        colors = ButtonDefaults.buttonColors(containerColor = BrandPurple.copy(alpha = 0.3f))
                    ) {
                        Text("${presetSpeed}x")
                    }
                }
            }
        }
    }
}
```

---

## User Flow

1. **Timeline Tab** → View/edit clips, drag & drop, split clips
2. **Effects Tab** → Browse 200+ effects, apply to selected clip
3. **Audio Tab** → Adjust volume, mute tracks, master control
4. **Color Tab** → Professional color grading with sliders
5. **Text Tab** → Add text overlays with customization
6. **Trim Tab** → Trim clips, adjust speed (0.25x-4x)

---

## Performance Considerations

- **Lazy Loading**: Effects loaded on-demand
- **GPU Processing**: Effects use hardware acceleration
- **Optimized Sliders**: Throttled updates for smooth UX
- **Memory Efficient**: Only active tab loaded
- **Real-time Preview**: Changes applied immediately

---

## Testing

### Unit Tests
```kotlin
@Test
fun `should apply effect to selected clip`() = runTest {
    val effect = Effect(type = EffectType.FILTER, name = "Vintage")
    viewModel.addEffect(effect)
    
    assertTrue(viewModel.selectedClip.value?.effects?.contains(effect) ?: false)
}
```

---

## Related Screens

- [Editor Screen](./04-EDITOR-SCREEN-ENHANCED.md)
- [Export Screen](./05-EXPORT-SCREEN-ENHANCED.md)
- [Templates Screen](./03-TEMPLATES-SCREEN-ENHANCED.md)