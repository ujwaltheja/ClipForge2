@file:OptIn(androidx.compose.material3.ExperimentalMaterial3Api::class)

package com.ucworks.clipforge.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
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
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import com.ucworks.clipforge.ui.theme.*

data class ExportPlatform(
    val id: String,
    val name: String,
    val icon: String = "📱",
    val preset: String = "",
    val color: Color = BrandPurple
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun ExportScreen(
    onBackClick: () -> Unit = {},
    onExportClick: (ExportPlatform) -> Unit = {}
) {
    val platforms = listOf(
        ExportPlatform(
            id = "tiktok",
            name = "TikTok Made",
            color = Color(0xFF000000)
        ),
        ExportPlatform(
            id = "tiktok_video",
            name = "TikTok Gros",
            color = Color(0xFF000000)
        ),
        ExportPlatform(
            id = "instagram",
            name = "Instagram Reel",
            color = Color(0xFFE1306C)
        ),
        ExportPlatform(
            id = "youtube",
            name = "YouTube Video",
            color = Color(0xFFFF0000)
        ),
        ExportPlatform(
            id = "instagram_feed",
            name = "Instagram Feed",
            color = Color(0xFFE1306C)
        ),
        ExportPlatform(
            id = "facebook",
            name = "Facebook",
            color = Color(0xFF1877F2)
        )
    )

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
            ExportTopBar(onBackClick = onBackClick)

            // Content
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .weight(1f)
                    .padding(16.dp)
                    .verticalScroll(rememberScrollState())
            ) {
                Text(
                    "Export Your Reel",
                    style = MaterialTheme.typography.headlineMedium,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary,
                    modifier = Modifier.padding(bottom = 24.dp)
                )

                // Export Platforms Grid
                LazyVerticalGrid(
                    columns = GridCells.Fixed(3),
                    contentPadding = PaddingValues(bottom = 16.dp),
                    horizontalArrangement = Arrangement.spacedBy(12.dp),
                    verticalArrangement = Arrangement.spacedBy(12.dp),
                    modifier = Modifier.fillMaxWidth()
                ) {
                    items(platforms) { platform ->
                        ExportPlatformCard(
                            platform = platform,
                            onClick = { onExportClick(platform) }
                        )
                    }
                }

                Spacer(modifier = Modifier.height(24.dp))

                // Export Settings
                Text(
                    "Export Settings",
                    style = MaterialTheme.typography.titleMedium,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary,
                    modifier = Modifier.padding(bottom = 12.dp)
                )

                // Quality Selector
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(bottom = 12.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = SurfaceDark
                    ),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    Column(
                        modifier = Modifier.padding(12.dp)
                    ) {
                        Text(
                            "Quality",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.SemiBold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceEvenly
                        ) {
                            listOf("HD", "Full HD", "4K").forEach { quality ->
                                OutlinedButton(
                                    onClick = {},
                                    modifier = Modifier
                                        .weight(1f)
                                        .height(40.dp),
                                    colors = ButtonDefaults.outlinedButtonColors(
                                        contentColor = TextPrimary
                                    )
                                ) {
                                    Text(quality)
                                }
                            }
                        }
                    }
                }

                // Format Selector
                Card(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(bottom = 12.dp),
                    colors = CardDefaults.cardColors(
                        containerColor = SurfaceDark
                    ),
                    shape = RoundedCornerShape(8.dp)
                ) {
                    Column(
                        modifier = Modifier.padding(12.dp)
                    ) {
                        Text(
                            "Format",
                            style = MaterialTheme.typography.bodyMedium,
                            fontWeight = FontWeight.SemiBold,
                            color = TextPrimary
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Row(
                            modifier = Modifier.fillMaxWidth(),
                            horizontalArrangement = Arrangement.SpaceEvenly
                        ) {
                            listOf("MP4", "MOV", "WebM").forEach { format ->
                                OutlinedButton(
                                    onClick = {},
                                    modifier = Modifier
                                        .weight(1f)
                                        .height(40.dp),
                                    colors = ButtonDefaults.outlinedButtonColors(
                                        contentColor = TextPrimary
                                    )
                                ) {
                                    Text(format)
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

@Composable
private fun ExportPlatformCard(
    platform: ExportPlatform,
    onClick: () -> Unit = {}
) {
    Card(
        onClick = onClick,
        modifier = Modifier
            .fillMaxWidth()
            .aspectRatio(1f)
            .border(
                width = 2.dp,
                color = platform.color.copy(alpha = 0.3f),
                shape = RoundedCornerShape(12.dp)
            ),
        colors = CardDefaults.cardColors(
            containerColor = SurfaceDark
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(12.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Box(
                modifier = Modifier
                    .size(40.dp)
                    .background(platform.color.copy(alpha = 0.2f), RoundedCornerShape(8.dp)),
                contentAlignment = Alignment.Center
            ) {
                when (platform.id) {
                    "tiktok", "tiktok_video" -> {
                        Icon(
                            Icons.Default.MusicNote,
                            contentDescription = null,
                            tint = platform.color,
                            modifier = Modifier.size(24.dp)
                        )
                    }
                    "instagram", "instagram_feed" -> {
                        Icon(
                            Icons.Default.PhotoCamera,
                            contentDescription = null,
                            tint = platform.color,
                            modifier = Modifier.size(24.dp)
                        )
                    }
                    "youtube" -> {
                        Icon(
                            Icons.Default.PlayArrow,
                            contentDescription = null,
                            tint = platform.color,
                            modifier = Modifier.size(24.dp)
                        )
                    }
                    else -> {
                        Icon(
                            Icons.Default.Share,
                            contentDescription = null,
                            tint = platform.color,
                            modifier = Modifier.size(24.dp)
                        )
                    }
                }
            }

            Spacer(modifier = Modifier.height(8.dp))

            Text(
                platform.name,
                style = MaterialTheme.typography.labelSmall,
                fontWeight = FontWeight.SemiBold,
                color = TextPrimary,
                maxLines = 2
            )
        }
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun ExportTopBar(
    onBackClick: () -> Unit = {}
) {
    TopAppBar(
        title = {
            Text(
                "Export",
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
        colors = TopAppBarDefaults.topAppBarColors(
            containerColor = BackgroundDark
        ),
        modifier = Modifier.height(56.dp)
    )
}
