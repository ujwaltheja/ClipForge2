package com.ucworks.clipforge.data.repository

import com.ucworks.clipforge.data.models.Project

class ProjectRepository {

    //This is a placeholder. A real implementation would use a database.
    private val projects = mutableListOf<Project>()

    suspend fun getAllProjects(): List<Project> {
        return projects
    }

    suspend fun insertProject(project: Project) {
        projects.add(project)
    }

    suspend fun deleteProject(project: Project) {
        projects.remove(project)
    }
}