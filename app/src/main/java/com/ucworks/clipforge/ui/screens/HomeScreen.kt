@file:OptIn(androidx.compose.material3.ExperimentalMaterial3Api::class)

package com.ucworks.clipforge.ui.screens

import androidx.compose.animation.AnimatedVisibility
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.grid.GridCells
import androidx.compose.foundation.lazy.grid.LazyVerticalGrid
import androidx.compose.foundation.lazy.grid.items
import androidx.compose.foundation.rememberScrollState
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
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.style.TextOverflow
import androidx.compose.ui.unit.dp
import coil.compose.AsyncImage
import com.ucworks.clipforge.ui.theme.*
import java.text.SimpleDateFormat
import java.util.*

// Data class for Project (matching the existing model)
data class ProjectUI(
    val id: String,
    val name: String,
    val createdAt: Long,
    val modifiedAt: Long,
    val thumbnailPath: String? = null,
    val clipCount: Int = 0,
    val duration: Long = 0
)

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun HomeScreen(
    projects: List<ProjectUI> = emptyList(),
    onNewProjectClick: () -> Unit = {},
    onProjectClick: (ProjectUI) -> Unit = {},
    onImportClick: () -> Unit = {},
    onRecordClick: () -> Unit = {},
    onTemplatesClick: () -> Unit = {},
    onSettingsClick: () -> Unit = {},
    onDeleteProject: (ProjectUI) -> Unit = {}
) {
    var showCreateDialog by remember { mutableStateOf(false) }
    var showDeleteDialog by remember { mutableStateOf<ProjectUI?>(null) }

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
            // Top App Bar
            HomeTopBar(onSettingsClick = onSettingsClick)

            // Content
            if (projects.isEmpty()) {
                EmptyStateView(
                    onNewProjectClick = {
                        showCreateDialog = true
                    },
                    onImportClick = onImportClick,
                    onRecordClick = onRecordClick,
                    onTemplatesClick = onTemplatesClick,
                    modifier = Modifier
                        .fillMaxSize()
                        .weight(1f)
                )
            } else {
                ProjectsContentView(
                    projects = projects,
                    onNewProjectClick = { showCreateDialog = true },
                    onProjectClick = onProjectClick,
                    onImportClick = onImportClick,
                    onRecordClick = onRecordClick,
                    onTemplatesClick = onTemplatesClick,
                    onDeleteClick = { showDeleteDialog = it },
                    modifier = Modifier
                        .fillMaxSize()
                        .weight(1f)
                )
            }
        }
    }

    // Dialogs
    if (showCreateDialog) {
        CreateProjectDialog(
            onDismiss = { showCreateDialog = false },
            onCreate = { name ->
                onNewProjectClick()
                showCreateDialog = false
            }
        )
    }

    showDeleteDialog?.let { project ->
        DeleteProjectDialog(
            project = project,
            onDismiss = { showDeleteDialog = null },
            onConfirm = {
                onDeleteProject(project)
                showDeleteDialog = null
            }
        )
    }
}

@OptIn(ExperimentalMaterial3Api::class)
@Composable
private fun HomeTopBar(
    onSettingsClick: () -> Unit = {}
) {
    TopAppBar(
        title = {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.padding(start = 8.dp)
            ) {
                Icon(
                    imageVector = Icons.Default.PlayArrow,
                    contentDescription = null,
                    tint = BrandPink,
                    modifier = Modifier.size(32.dp)
                )
                Spacer(modifier = Modifier.width(12.dp))
                Text(
                    "ClipForge",
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )
            }
        },
        actions = {
            IconButton(onClick = onSettingsClick) {
                Icon(
                    Icons.Default.Settings,
                    contentDescription = "Settings",
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
private fun EmptyStateView(
    onNewProjectClick: () -> Unit = {},
    onImportClick: () -> Unit = {},
    onRecordClick: () -> Unit = {},
    onTemplatesClick: () -> Unit = {},
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxWidth()
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
                tint = BrandPink
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        Text(
            "Welcome to ClipForge",
            style = MaterialTheme.typography.headlineMedium,
            fontWeight = FontWeight.Bold,
            color = TextPrimary,
            textAlign = TextAlign.Center
        )

        Spacer(modifier = Modifier.height(8.dp))

        Text(
            "Create amazing videos with professional tools",
            style = MaterialTheme.typography.bodyLarge,
            color = TextSecondary,
            textAlign = TextAlign.Center
        )

        Spacer(modifier = Modifier.height(32.dp))

        // New Project Button
        Button(
            onClick = onNewProjectClick,
            modifier = Modifier
                .fillMaxWidth()
                .height(56.dp),
            colors = ButtonDefaults.buttonColors(
                containerColor = BrandPurple
            ),
            shape = RoundedCornerShape(16.dp)
        ) {
            Icon(
                Icons.Default.Add,
                contentDescription = null,
                tint = TextPrimary,
                modifier = Modifier
                    .size(24.dp)
                    .padding(end = 8.dp)
            )
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
            color = TextPrimary,
            modifier = Modifier
                .fillMaxWidth()
                .padding(start = 8.dp)
        )

        Spacer(modifier = Modifier.height(16.dp))

        // Quick Action Cards - Row 1
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            QuickActionCard(
                title = "Import\nMedia",
                icon = Icons.Default.Upload,
                onClick = onImportClick,
                modifier = Modifier.weight(1f),
                backgroundColor = SurfaceDark
            )

            QuickActionCard(
                title = "Record",
                icon = Icons.Default.Camera,
                onClick = onRecordClick,
                modifier = Modifier.weight(1f),
                backgroundColor = SurfaceDark
            )
        }

        Spacer(modifier = Modifier.height(12.dp))

        // Quick Action Card - Templates
        QuickActionCard(
            title = "Trending\nTemplates",
            icon = Icons.Default.Star,
            backgroundColor = Color(0xFF92400E),
            iconTint = AccentOrange,
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
    backgroundColor: Color = SurfaceDark,
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
                modifier = Modifier.size(36.dp),
                tint = iconTint
            )
            Spacer(modifier = Modifier.height(8.dp))
            Text(
                title,
                style = MaterialTheme.typography.bodyMedium,
                fontWeight = FontWeight.SemiBold,
                color = TextPrimary,
                textAlign = TextAlign.Center
            )
        }
    }
}

@Composable
private fun ProjectsContentView(
    projects: List<ProjectUI>,
    onNewProjectClick: () -> Unit = {},
    onProjectClick: (ProjectUI) -> Unit = {},
    onImportClick: () -> Unit = {},
    onRecordClick: () -> Unit = {},
    onTemplatesClick: () -> Unit = {},
    onDeleteClick: (ProjectUI) -> Unit = {},
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .fillMaxWidth()
            .padding(16.dp)
    ) {
        // Quick Action Buttons
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .horizontalScroll(rememberScrollState()),
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            QuickActionCard(
                title = "New",
                icon = Icons.Default.Add,
                onClick = onNewProjectClick,
                modifier = Modifier
                    .width(100.dp)
                    .height(100.dp),
                backgroundColor = BrandPurple
            )

            QuickActionCard(
                title = "Import",
                icon = Icons.Default.Upload,
                onClick = onImportClick,
                modifier = Modifier
                    .width(100.dp)
                    .height(100.dp)
            )

            QuickActionCard(
                title = "Record",
                icon = Icons.Default.Camera,
                onClick = onRecordClick,
                modifier = Modifier
                    .width(100.dp)
                    .height(100.dp)
            )

            QuickActionCard(
                title = "Templates",
                icon = Icons.Default.Star,
                backgroundColor = Color(0xFF92400E),
                iconTint = AccentOrange,
                onClick = onTemplatesClick,
                modifier = Modifier
                    .width(100.dp)
                    .height(100.dp)
            )
        }

        Spacer(modifier = Modifier.height(24.dp))

        // Projects Header
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 8.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                "My Projects (${projects.size})",
                style = MaterialTheme.typography.titleLarge,
                fontWeight = FontWeight.Bold,
                color = TextPrimary
            )

            IconButton(onClick = {}) {
                Icon(
                    Icons.Default.Search,
                    contentDescription = "Search",
                    tint = TextPrimary
                )
            }
        }

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
    project: ProjectUI,
    onClick: () -> Unit = {},
    onDeleteClick: () -> Unit = {},
    modifier: Modifier = Modifier
) {
    var showMenu by remember { mutableStateOf(false) }

    Card(
        onClick = onClick,
        modifier = modifier.aspectRatio(9f / 16f),
        colors = CardDefaults.cardColors(
            containerColor = SurfaceDark
        ),
        shape = RoundedCornerShape(12.dp)
    ) {
        Box(modifier = Modifier.fillMaxSize()) {
            // Thumbnail or placeholder
            if (!project.thumbnailPath.isNullOrEmpty()) {
                AsyncImage(
                    model = project.thumbnailPath,
                    contentDescription = project.name,
                    contentScale = ContentScale.Crop,
                    modifier = Modifier.fillMaxSize()
                )
            } else {
                Box(
                    modifier = Modifier
                        .fillMaxSize()
                        .background(SurfaceDarker),
                    contentAlignment = Alignment.Center
                ) {
                    Icon(
                        Icons.Default.Movie,
                        contentDescription = null,
                        modifier = Modifier.size(48.dp),
                        tint = Color.Gray
                    )
                }
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
                    color = TextPrimary,
                    maxLines = 2,
                    overflow = TextOverflow.Ellipsis
                )

                Spacer(modifier = Modifier.height(4.dp))

                Text(
                    formatDate(project.modifiedAt),
                    style = MaterialTheme.typography.labelSmall,
                    color = TextSecondary
                )

                if (project.clipCount > 0) {
                    Text(
                        "${project.clipCount} clips",
                        style = MaterialTheme.typography.labelSmall,
                        color = TextSecondary
                    )
                }
            }

            // More options menu
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
                        tint = TextPrimary
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
private fun CreateProjectDialog(
    onDismiss: () -> Unit = {},
    onCreate: (String) -> Unit = {}
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
    project: ProjectUI,
    onDismiss: () -> Unit = {},
    onConfirm: () -> Unit = {}
) {
    AlertDialog(
        onDismissRequest = onDismiss,
        icon = {
            Icon(
                Icons.Default.Delete,
                contentDescription = null,
                tint = ErrorRed
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
                    containerColor = ErrorRed
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
