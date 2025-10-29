package com.ucworks.clipforge

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.ucworks.clipforge.ui.screens.*
import com.ucworks.clipforge.ui.theme.ClipForgeTheme

/**
 * ComposeActivity hosts all Compose UI screens using Jetpack Compose
 * This replaces the XML layout-based approach for modern, efficient UI
 */
class ComposeActivity : ComponentActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        setContent {
            ClipForgeTheme {
                ClipForgeNavigation()
            }
        }
    }
}

@Composable
private fun ClipForgeNavigation() {
    val navController = rememberNavController()

    // Sample projects for demo (replace with real data from database/API)
    val sampleProjects = listOf(
        ProjectUI(
            id = "1",
            name = "Vacation Vlog",
            createdAt = System.currentTimeMillis() - 86400000,
            modifiedAt = System.currentTimeMillis() - 3600000,
            clipCount = 5,
            duration = 120000
        ),
        ProjectUI(
            id = "2",
            name = "Birthday Party",
            createdAt = System.currentTimeMillis() - 172800000,
            modifiedAt = System.currentTimeMillis() - 7200000,
            clipCount = 8,
            duration = 180000
        ),
        ProjectUI(
            id = "3",
            name = "Wedding Highlights",
            createdAt = System.currentTimeMillis() - 259200000,
            modifiedAt = System.currentTimeMillis() - 10800000,
            clipCount = 12,
            duration = 300000
        )
    )

    var projects by remember { mutableStateOf(sampleProjects) }
    var currentProject by remember { mutableStateOf<ProjectUI?>(null) }

    NavHost(
        navController = navController,
        startDestination = "home"
    ) {
        composable("home") {
            HomeScreen(
                projects = projects,
                onNewProjectClick = {
                    // Create new project logic
                },
                onProjectClick = { project ->
                    currentProject = project
                    navController.navigate("editor/${project.id}")
                },
                onImportClick = {
                    navController.navigate("import")
                },
                onRecordClick = {
                    navController.navigate("record")
                },
                onTemplatesClick = {
                    navController.navigate("templates")
                },
                onSettingsClick = {
                    navController.navigate("settings")
                },
                onDeleteProject = { project ->
                    projects = projects.filter { it.id != project.id }
                }
            )
        }

        composable("editor/{projectId}") { backStackEntry ->
            val projectId = backStackEntry.arguments?.getString("projectId") ?: ""
            val project = projects.find { it.id == projectId }

            EditorScreen(
                projectId = projectId,
                projectName = project?.name ?: "Project",
                onBackClick = {
                    navController.popBackStack()
                },
                onPlayClick = {},
                onExportClick = {
                    navController.navigate("export")
                },
                onFiltersClick = {
                    navController.navigate("filters")
                }
            )
        }

        composable("filters") {
            FiltersScreen(
                onBackClick = {
                    navController.popBackStack()
                },
                onFilterClick = { filter ->
                    // Apply filter logic
                }
            )
        }

        composable("export") {
            ExportScreen(
                onBackClick = {
                    navController.popBackStack()
                },
                onExportClick = { platform ->
                    // Export logic for platform
                }
            )
        }

        composable("import") {
            ImportScreen(onBackClick = { navController.popBackStack() })
        }

        composable("record") {
            RecordScreen(onBackClick = { navController.popBackStack() })
        }

        composable("templates") {
            TemplatesScreen(onBackClick = { navController.popBackStack() })
        }

        composable("settings") {
            SettingsScreen(onBackClick = { navController.popBackStack() })
        }
    }
}
