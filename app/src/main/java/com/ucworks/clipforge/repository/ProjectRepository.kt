package com.ucworks.clipforge.repository

import android.content.Context
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import com.ucworks.clipforge.models.Project
import com.ucworks.clipforge.models.RecentProject
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import timber.log.Timber
import java.io.File

/**
 * Repository for managing Project data persistence
 * Abstracts data sources (files, database, cloud) from ViewModels
 */
class ProjectRepository(private val context: Context) {

    private val projectsDir = File(context.filesDir, "projects")
    private val recentProjectsPrefs = "recent_projects"

    init {
        if (!projectsDir.exists()) {
            projectsDir.mkdirs()
            Timber.d("Projects directory created")
        }
    }

    // Live data for reactive updates
    private val _allProjects = MutableLiveData<List<RecentProject>>()
    val allProjects: LiveData<List<RecentProject>> = _allProjects

    private val _currentProject = MutableLiveData<Project?>()
    val currentProject: LiveData<Project?> = _currentProject

    /**
     * Load all recent projects
     */
    suspend fun loadRecentProjects(): List<RecentProject> = withContext(Dispatchers.IO) {
        try {
            Timber.d("Loading recent projects")
            val projects = mutableListOf<RecentProject>()

            projectsDir.listFiles()?.forEach { file ->
                if (file.isFile && file.extension == "clipforge") {
                    try {
                        val project = loadProjectFromFile(file)
                        if (project != null) {
                            projects.add(RecentProject.fromProject(project))
                        }
                    } catch (e: Exception) {
                        Timber.e(e, "Error loading project: ${file.name}")
                    }
                }
            }

            projects.sortByDescending { it.lastModified }
            _allProjects.postValue(projects)
            Timber.d("Loaded ${projects.size} recent projects")
            projects
        } catch (e: Exception) {
            Timber.e(e, "Error loading recent projects")
            emptyList()
        }
    }

    /**
     * Load a specific project by ID
     */
    suspend fun loadProject(projectId: String): Project? = withContext(Dispatchers.IO) {
        try {
            Timber.d("Loading project: $projectId")
            val file = File(projectsDir, "$projectId.clipforge")

            return@withContext if (file.exists()) {
                val project = loadProjectFromFile(file)
                _currentProject.postValue(project)
                project
            } else {
                Timber.w("Project file not found: $projectId")
                null
            }
        } catch (e: Exception) {
            Timber.e(e, "Error loading project: $projectId")
            null
        }
    }

    /**
     * Save a project
     */
    suspend fun saveProject(project: Project): Boolean = withContext(Dispatchers.IO) {
        try {
            Timber.d("Saving project: ${project.name}")
            val file = File(projectsDir, "${project.id}.clipforge")

            // In real app, would serialize to JSON
            file.writeText("Project: ${project.name}")

            _currentProject.postValue(project)
            Timber.d("Project saved successfully")
            true
        } catch (e: Exception) {
            Timber.e(e, "Error saving project")
            false
        }
    }

    /**
     * Delete a project
     */
    suspend fun deleteProject(projectId: String): Boolean = withContext(Dispatchers.IO) {
        try {
            Timber.d("Deleting project: $projectId")
            val file = File(projectsDir, "$projectId.clipforge")

            val deleted = file.delete()
            if (deleted) {
                loadRecentProjects()  // Refresh list
                Timber.d("Project deleted successfully")
            }
            deleted
        } catch (e: Exception) {
            Timber.e(e, "Error deleting project")
            false
        }
    }

    /**
     * Create a new project
     */
    suspend fun createProject(name: String, template: String): Project = withContext(Dispatchers.IO) {
        try {
            Timber.d("Creating new project: $name (template: $template)")

            val project = Project(
                id = java.util.UUID.randomUUID().toString(),
                name = name,
                createdAt = System.currentTimeMillis(),
                modifiedAt = System.currentTimeMillis(),
                videoWidth = getTemplateWidth(template),
                videoHeight = getTemplateHeight(template),
                videoFrameRate = getTemplateFrameRate(template)
            )

            saveProject(project)
            project
        } catch (e: Exception) {
            Timber.e(e, "Error creating project")
            throw e
        }
    }

    /**
     * Duplicate a project
     */
    suspend fun duplicateProject(projectId: String): Project? = withContext(Dispatchers.IO) {
        try {
            Timber.d("Duplicating project: $projectId")
            val original = loadProject(projectId) ?: return@withContext null

            val duplicate = original.copy(
                id = java.util.UUID.randomUUID().toString(),
                name = "${original.name} (Copy)",
                createdAt = System.currentTimeMillis(),
                modifiedAt = System.currentTimeMillis()
            )

            saveProject(duplicate)
            loadRecentProjects()  // Refresh list
            duplicate
        } catch (e: Exception) {
            Timber.e(e, "Error duplicating project")
            null
        }
    }

    /**
     * Export project data (for backup/sharing)
     */
    suspend fun exportProject(projectId: String, outputPath: String): Boolean = withContext(Dispatchers.IO) {
        try {
            Timber.d("Exporting project: $projectId to $outputPath")
            val sourceFile = File(projectsDir, "$projectId.clipforge")
            val outputFile = File(outputPath)

            sourceFile.copyTo(outputFile, overwrite = true)
            Timber.d("Project exported successfully")
            true
        } catch (e: Exception) {
            Timber.e(e, "Error exporting project")
            false
        }
    }

    /**
     * Import project data (from backup/sharing)
     */
    suspend fun importProject(inputPath: String): Project? = withContext(Dispatchers.IO) {
        try {
            Timber.d("Importing project from: $inputPath")
            val inputFile = File(inputPath)

            if (!inputFile.exists()) {
                Timber.w("Input file not found: $inputPath")
                return@withContext null
            }

            val projectId = java.util.UUID.randomUUID().toString()
            val destFile = File(projectsDir, "$projectId.clipforge")

            inputFile.copyTo(destFile)
            loadRecentProjects()  // Refresh list

            Timber.d("Project imported successfully")
            loadProject(projectId)
        } catch (e: Exception) {
            Timber.e(e, "Error importing project")
            null
        }
    }

    /**
     * Get list of recent projects (synchronous)
     */
    fun getRecentProjects(): List<RecentProject> {
        return _allProjects.value ?: emptyList()
    }

    /**
     * Check if project exists
     */
    fun projectExists(projectId: String): Boolean {
        val file = File(projectsDir, "$projectId.clipforge")
        return file.exists()
    }

    /**
     * Get project size
     */
    fun getProjectSize(projectId: String): Long {
        val file = File(projectsDir, "$projectId.clipforge")
        return if (file.exists()) file.length() else 0L
    }

    // Private helper methods

    private fun loadProjectFromFile(file: File): Project? {
        return try {
            // In real app, would deserialize from JSON
            // For now, create mock project
            Project(
                id = file.nameWithoutExtension,
                name = file.nameWithoutExtension,
                createdAt = file.lastModified(),
                modifiedAt = file.lastModified()
            )
        } catch (e: Exception) {
            Timber.e(e, "Error parsing project file: ${file.name}")
            null
        }
    }

    private fun getTemplateWidth(template: String): Int {
        return when {
            template.contains("4K") -> 3840
            template.contains("Story") || template.contains("TikTok") -> 1080
            else -> 1920
        }
    }

    private fun getTemplateHeight(template: String): Int {
        return when {
            template.contains("4K") -> 2160
            template.contains("Story") || template.contains("TikTok") -> 1920
            else -> 1080
        }
    }

    private fun getTemplateFrameRate(template: String): Int {
        return when {
            template.contains("60") -> 60
            else -> 30
        }
    }
}
