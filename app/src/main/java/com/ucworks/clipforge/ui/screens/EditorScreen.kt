package com.ucworks.clipforge.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.material3.TabRowDefaults.tabIndicatorOffset
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import com.ucworks.clipforge.ui.theme.*

// Data classes for timeline clips
data class TimelineClip(
    val id: String,
    val thumbnailUrl: String? = null,
    val duration: String = "00:00",
    val startTime: Float = 0f,
    val track: Int = 0 // 0 = main, 1 = overlay, etc.
)

@Composable
fun EditorScreen(
    projectId: String = "",
    projectName: String = "",
    onBackClick: () -> Unit = {},
    onPlayClick: () -> Unit = {},
    onExportClick: () -> Unit = {},
    onFiltersClick: () -> Unit = {}
) {
    var selectedTab by remember { mutableStateOf("Main") }
    val tabs = listOf("Effects", "Main", "Library")

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(
                brush = Brush.verticalGradient(
                    colors = listOf(
                        BackgroundDarker,
                        Color(0xFF1A0B2E),
                        BackgroundDarker
                    )
                )
            )
    ) {
        Column(
            modifier = Modifier.fillMaxSize()
        ) {
            // Top Bar
            EditorTopBar(
                title = "Editing Timeline",
                onBackClick = onBackClick,
                onFiltersClick = onFiltersClick,
                onExportClick = onExportClick
            )

            // Video Preview Area
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .aspectRatio(9f / 16f)
                    .padding(16.dp)
                    .background(SurfaceDarker, RoundedCornerShape(12.dp)),
                contentAlignment = Alignment.Center
            ) {
                // Placeholder for video preview
                Icon(
                    Icons.Default.Movie,
                    contentDescription = null,
                    modifier = Modifier.size(64.dp),
                    tint = Color.Gray
                )
            }

            // Video Player Controls
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(horizontal = 16.dp, vertical = 8.dp),
                horizontalArrangement = Arrangement.SpaceEvenly,
                verticalAlignment = Alignment.CenterVertically
            ) {
                IconButton(
                    onClick = onPlayClick,
                    modifier = Modifier.size(48.dp)
                ) {
                    Icon(
                        Icons.Default.PlayArrow,
                        contentDescription = "Play",
                        tint = TextPrimary,
                        modifier = Modifier.size(32.dp)
                    )
                }

                // Progress Bar
                Slider(
                    value = 0.33f,
                    onValueChange = {},
                    modifier = Modifier
                        .weight(1f)
                        .padding(horizontal = 12.dp),
                    colors = SliderDefaults.colors(
                        thumbColor = AccentOrange,
                        activeTrackColor = AccentOrange,
                        inactiveTrackColor = SurfaceDark
                    )
                )

                Text(
                    "00:33",
                    style = MaterialTheme.typography.bodySmall,
                    color = TextPrimary,
                    fontWeight = FontWeight.Bold
                )
            }

            Spacer(modifier = Modifier.height(8.dp))

            // Tab Navigation
            TabRow(
                selectedTabIndex = tabs.indexOf(selectedTab),
                modifier = Modifier.fillMaxWidth(),
                containerColor = SurfaceDark,
                contentColor = BrandPurple,
                indicator = { tabPositions ->
                    if (selectedTab in tabs) {
                        TabRowDefaults.Indicator(
                            modifier = Modifier.tabIndicatorOffset(tabPositions[tabs.indexOf(selectedTab)]),
                            height = 3.dp,
                            color = AccentOrange
                        )
                    }
                }
            ) {
                tabs.forEachIndexed { index, tab ->
                    Tab(
                        selected = selectedTab == tab,
                        onClick = { selectedTab = tab },
                        text = {
                            Text(
                                tab,
                                color = if (selectedTab == tab) TextPrimary else TextSecondary,
                                fontWeight = if (selectedTab == tab) FontWeight.Bold else FontWeight.Normal
                            )
                        }
                    )
                }
            }

            // Timeline View
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .weight(1f)
                    .padding(16.dp)
                    .verticalScroll(rememberScrollState())
            ) {
                when (selectedTab) {
                    "Effects" -> EffectsTabContent()
                    "Main" -> MainTabContent()
                    "Library" -> LibraryTabContent()
                }
            }
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun EditorTopBar(
    title: String = "Editing Timeline",
    onBackClick: () -> Unit = {},
    onFiltersClick: () -> Unit = {},
    onExportClick: () -> Unit = {}
) {
    TopAppBar(
        title = {
            Text(
                title,
                style = MaterialTheme.typography.headlineSmall,
                fontWeight = FontWeight.Bold,
                color = TextPrimary
            )
        },
        navigationIcon = {
            IconButton(onClick = onBackClick) {
                Icon(
                    Icons.Default.ArrowBack,
                    contentDescription = "Back",
                    tint = TextPrimary
                )
            }
        },
        actions = {
            // Filters button
            IconButton(onClick = onFiltersClick) {
                Icon(
                    Icons.Default.FilterAlt,
                    contentDescription = "Filters",
                    tint = TextPrimary
                )
            }
            // Export button
            IconButton(onClick = onExportClick) {
                Icon(
                    Icons.Default.Upload,
                    contentDescription = "Export",
                    tint = TextPrimary
                )
            }
        },
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = BackgroundDark
        ),
        modifier = Modifier.height(56.dp)
    )
}

@Composable
private fun EffectsTabContent() {
    Column(
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(
            "Effects",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
            color = TextPrimary,
            modifier = Modifier.padding(bottom = 12.dp)
        )

        // Placeholder for effects list
        repeat(3) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(80.dp)
                    .padding(bottom = 8.dp),
                colors = CardDefaults.cardColors(
                    containerColor = SurfaceDark
                ),
                shape = RoundedCornerShape(8.dp)
            ) {
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        "Effect ${it + 1}",
                        color = TextSecondary
                    )
                }
            }
        }
    }
}

@Composable
private fun MainTabContent() {
    // Sample timeline clips matching reference
    val sampleClips = remember {
        listOf(
            TimelineClip(id = "1", duration = "03:58", track = 0),
            TimelineClip(id = "2", duration = "01:24", track = 1),
            TimelineClip(id = "3", duration = "04:31", track = 1)
        )
    }

    Column(
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(
            "Main",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
            color = TextPrimary,
            modifier = Modifier.padding(bottom = 12.dp)
        )

        // Timeline tracks
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 16.dp),
            verticalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            // Effects track (top)
            TimelineTrack(
                trackName = "Effects",
                clips = sampleClips.filter { it.track == 0 }
            )

            // Main track
            TimelineTrack(
                trackName = "Main",
                clips = sampleClips.filter { it.track == 1 }
            )

            // Music track (empty)
            TimelineTrack(
                trackName = "Music",
                clips = emptyList()
            )
        }
    }
}

@Composable
private fun TimelineTrack(
    trackName: String,
    clips: List<TimelineClip>
) {
    Column(
        modifier = Modifier.fillMaxWidth()
    ) {
        // Track label
        Text(
            trackName,
            style = MaterialTheme.typography.labelMedium,
            color = TextSecondary,
            modifier = Modifier.padding(bottom = 4.dp)
        )

        // Track timeline
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(80.dp)
                .background(SurfaceDarker, RoundedCornerShape(8.dp))
                .padding(8.dp)
        ) {
            if (clips.isNotEmpty()) {
                Row(
                    modifier = Modifier
                        .fillMaxSize()
                        .horizontalScroll(rememberScrollState()),
                    horizontalArrangement = Arrangement.spacedBy(4.dp)
                ) {
                    clips.forEach { clip ->
                        TimelineClipCard(clip = clip)
                    }
                }
            } else {
                // Empty track placeholder
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        "Empty",
                        color = TextSecondary.copy(alpha = 0.5f),
                        style = MaterialTheme.typography.labelSmall
                    )
                }
            }
        }
    }
}

@Composable
private fun TimelineClipCard(clip: TimelineClip) {
    Box(
        modifier = Modifier
            .width(120.dp)
            .fillMaxHeight()
            .clip(RoundedCornerShape(6.dp))
            .background(
                brush = Brush.horizontalGradient(
                    colors = listOf(
                        Color(0xFFFDB44B),
                        Color(0xFFEF6C00)
                    )
                )
            )
            .padding(4.dp),
        contentAlignment = Alignment.Center
    ) {
        // Duration text
        Text(
            clip.duration,
            style = MaterialTheme.typography.labelSmall,
            fontWeight = FontWeight.Bold,
            color = Color.White
        )
    }
}

@Composable
private fun LibraryTabContent() {
    Column(
        modifier = Modifier.fillMaxWidth()
    ) {
        Text(
            "Library",
            style = MaterialTheme.typography.titleMedium,
            fontWeight = FontWeight.Bold,
            color = TextPrimary,
            modifier = Modifier.padding(bottom = 12.dp)
        )

        // Placeholder for library items
        repeat(4) {
            Card(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(70.dp)
                    .padding(bottom = 8.dp),
                colors = CardDefaults.cardColors(
                    containerColor = SurfaceDark
                ),
                shape = RoundedCornerShape(8.dp)
            ) {
                Box(
                    modifier = Modifier.fillMaxSize(),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        "Library Item ${it + 1}",
                        color = TextSecondary
                    )
                }
            }
        }
    }
}
