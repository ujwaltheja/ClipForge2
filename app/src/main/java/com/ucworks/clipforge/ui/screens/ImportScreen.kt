package com.ucworks.clipforge.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.automirrored.filled.ArrowBack
import androidx.compose.material.icons.filled.Movie
import androidx.compose.material3.*
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

// Data class for media items
data class MediaItemUI(
    val id: String,
    val thumbnailUrl: String? = null,
    val duration: String = "00:00",
    val type: MediaType = MediaType.VIDEO
)

enum class MediaType {
    VIDEO, IMAGE
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ImportScreen(onBackClick: () -> Unit) {
    var selectedTabIndex by remember { mutableIntStateOf(0) }
    val tabs = listOf("Device", "Camera", "Library")

    // Sample media data matching the reference images
    val sampleMediaItems = remember {
        listOf(
            MediaItemUI(id = "1", duration = "03:58"),
            MediaItemUI(id = "2", duration = "01:24"),
            MediaItemUI(id = "3", duration = "02:42"),
            MediaItemUI(id = "4", duration = "04:31"),
        )
    }

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
        Column(modifier = Modifier.fillMaxSize()) {
            // Top Bar
            TopAppBar(
                title = { },
                navigationIcon = {
                    IconButton(onClick = onBackClick) {
                        Icon(
                            imageVector = Icons.AutoMirrored.Filled.ArrowBack,
                            contentDescription = "Back",
                            tint = TextPrimary
                        )
                    }
                },
                colors = TopAppBarDefaults.topAppBarColors(containerColor = BackgroundDark)
            )

            // Tab Row
            TabRow(
                selectedTabIndex = selectedTabIndex,
                containerColor = BackgroundDark,
                contentColor = TextPrimary,
                indicator = { tabPositions ->
                    TabRowDefaults.Indicator(
                        modifier = Modifier.tabIndicatorOffset(tabPositions[selectedTabIndex]),
                        height = 3.dp,
                        color = BrandPurple
                    )
                }
            ) {
                tabs.forEachIndexed { index, title ->
                    Tab(
                        selected = selectedTabIndex == index,
                        onClick = { selectedTabIndex = index },
                        text = {
                            Text(
                                text = title,
                                fontWeight = if (selectedTabIndex == index) FontWeight.Bold else FontWeight.Normal,
                                color = if (selectedTabIndex == index) TextPrimary else TextSecondary
                            )
                        }
                    )
                }
            }

            // Content for each tab
            when (selectedTabIndex) {
                0 -> DeviceMediaGrid(mediaItems = sampleMediaItems)
                1 -> CameraCaptureUI()
                2 -> LibraryGrid(mediaItems = sampleMediaItems)
            }
        }
    }
}

@Composable
private fun DeviceMediaGrid(mediaItems: List<MediaItemUI>) {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        Text(
            "Recent",
            style = MaterialTheme.typography.titleLarge,
            fontWeight = FontWeight.Bold,
            color = TextPrimary,
            modifier = Modifier.padding(bottom = 16.dp)
        )

        LazyVerticalGrid(
            columns = GridCells.Fixed(2),
            horizontalArrangement = Arrangement.spacedBy(12.dp),
            verticalArrangement = Arrangement.spacedBy(12.dp),
            contentPadding = PaddingValues(bottom = 80.dp)
        ) {
            items(mediaItems) { item ->
                MediaCard(item = item)
            }
        }

        Spacer(modifier = Modifier.weight(1f))

        // Bottom text
        Text(
            "Tap to add to Timeline",
            style = MaterialTheme.typography.titleMedium,
            color = TextPrimary,
            modifier = Modifier
                .fillMaxWidth()
                .padding(vertical = 16.dp),
            textAlign = androidx.compose.ui.text.style.TextAlign.Center
        )
    }
}

@Composable
private fun MediaCard(item: MediaItemUI) {
    Card(
        modifier = Modifier
            .aspectRatio(9f / 16f)
            .clip(RoundedCornerShape(12.dp)),
        colors = CardDefaults.cardColors(
            containerColor = SurfaceDark
        )
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            // Placeholder background
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(
                        brush = Brush.verticalGradient(
                            colors = listOf(
                                Color(0xFFFDB44B),
                                Color(0xFFEF6C00)
                            )
                        )
                    ),
                contentAlignment = Alignment.Center
            ) {
                Icon(
                    Icons.Default.Movie,
                    contentDescription = null,
                    modifier = Modifier.size(48.dp),
                    tint = Color.Black.copy(alpha = 0.3f)
                )
            }

            // Duration badge
            Box(
                modifier = Modifier
                    .align(Alignment.BottomEnd)
                    .padding(8.dp)
                    .background(Color.Black.copy(alpha = 0.7f), RoundedCornerShape(4.dp))
                    .padding(horizontal = 8.dp, vertical = 4.dp)
            ) {
                Text(
                    item.duration,
                    style = MaterialTheme.typography.labelSmall,
                    color = TextPrimary,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}

@Composable
private fun CameraCaptureUI() {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp),
        contentAlignment = Alignment.Center
    ) {
        Text(
            "Camera interface appears here",
            color = TextSecondary,
            style = MaterialTheme.typography.bodyLarge
        )
    }
}

@Composable
private fun LibraryGrid(mediaItems: List<MediaItemUI>) {
    DeviceMediaGrid(mediaItems = mediaItems)
}
