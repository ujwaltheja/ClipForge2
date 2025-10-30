package com.ucworks.clipforge.ui.viewmodels

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.ucworks.clipforge.data.models.Project
import com.ucworks.clipforge.data.models.ProjectTemplate
import com.ucworks.clipforge.data.repository.ProjectRepository
import kotlinx.coroutines.launch
import java.util.Date
import java.util.UUID

class MainViewModel : ViewModel() {
    
    private val repository = ProjectRepository()
    
    private val _projects = MutableLiveData<List<Project>>()
    val projects: LiveData<List<Project>> = _projects
    
    private val _error = MutableLiveData<String>()
    val error: LiveData<String> = _error
    
    fun loadProjects() {
        viewModelScope.launch {
            try {
                val projectList = repository.getAllProjects()
                _projects.value = projectList
            } catch (e: Exception) {
                _error.value = "Failed to load projects: ${e.message}"
            }
        }
    }
    
    fun createProject(name: String, template: ProjectTemplate) {
        viewModelScope.launch {
            try {
                val project = Project(
                    id = UUID.randomUUID().toString(),
                    name = name,
                    template = template,
                    createdAt = Date(),
                    updatedAt = Date(),
                    thumbnailPath = null,
                    duration = 0,
                    clipCount = 0
                )
                repository.insertProject(project)
                loadProjects()
            } catch (e: Exception) {
                _error.value = "Failed to create project: ${e.message}"
            }
        }
    }
    
    fun deleteProject(project: Project) {
        viewModelScope.launch {
            try {
                repository.deleteProject(project)
                loadProjects()
            } catch (e: Exception) {
                _error.value = "Failed to delete project: ${e.message}"
            }
        }
    }
}