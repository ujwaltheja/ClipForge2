package com.ucworks.clipforge.viewmodels

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.ucworks.clipforge.models.Project
import com.ucworks.clipforge.models.VideoClip
import com.ucworks.clipforge.models.AudioTrack
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * ViewModel for managing project state
 * Handles:
 * - Current project data
 * - Clip operations
 * - Audio track management
 * - Project persistence
 */
class ProjectViewModel : ViewModel() {

    // Project state
    private val _currentProject = MutableLiveData<Project>()
    val currentProject: LiveData<Project> = _currentProject

    // Clips
    private val _clips = MutableLiveData<List<VideoClip>>()
    val clips: LiveData<List<VideoClip>> = _clips

    // Audio tracks
    private val _audioTracks = MutableLiveData<List<AudioTrack>>()
    val audioTracks: LiveData<List<AudioTrack>> = _audioTracks

    // UI state
    private val _isLoading = MutableLiveData(false)
    val isLoading: LiveData<Boolean> = _isLoading

    private val _error = MutableLiveData<String>()
    val error: LiveData<String> = _error

    private val _hasUnsavedChanges = MutableLiveData(false)
    val hasUnsavedChanges: LiveData<Boolean> = _hasUnsavedChanges

    // Initialization
    fun initializeProject(projectId: String) {
        viewModelScope.launch {
            try {
                _isLoading.value = true
                Timber.d("Initializing project: $projectId")

                // In real app, would load from database
                val mockProject = Project(
                    id = projectId,
                    name = "New Project",
                    videoWidth = 1920,
                    videoHeight = 1080,
                    videoFrameRate = 30
                )

                _currentProject.value = mockProject
                _clips.value = emptyList()
                _audioTracks.value = emptyList()
                _hasUnsavedChanges.value = false

                Timber.d("Project initialized successfully")
            } catch (e: Exception) {
                Timber.e(e, "Error initializing project")
                _error.value = "Failed to initialize project: ${e.message}"
            } finally {
                _isLoading.value = false
            }
        }
    }

    // Clip operations
    fun addClip(clip: VideoClip) {
        viewModelScope.launch {
            try {
                val currentClips = _clips.value.orEmpty().toMutableList()
                currentClips.add(clip)
                _clips.value = currentClips
                _hasUnsavedChanges.value = true
                Timber.d("Clip added: ${clip.displayName}")
            } catch (e: Exception) {
                Timber.e(e, "Error adding clip")
                _error.value = "Failed to add clip"
            }
        }
    }

    fun removeClip(clipId: String) {
        viewModelScope.launch {
            try {
                val currentClips = _clips.value.orEmpty().toMutableList()
                currentClips.removeAll { it.id == clipId }
                _clips.value = currentClips
                _hasUnsavedChanges.value = true
                Timber.d("Clip removed: $clipId")
            } catch (e: Exception) {
                Timber.e(e, "Error removing clip")
                _error.value = "Failed to remove clip"
            }
        }
    }

    fun updateClip(clip: VideoClip) {
        viewModelScope.launch {
            try {
                val currentClips = _clips.value.orEmpty().toMutableList()
                val index = currentClips.indexOfFirst { it.id == clip.id }
                if (index >= 0) {
                    currentClips[index] = clip
                    _clips.value = currentClips
                    _hasUnsavedChanges.value = true
                    Timber.d("Clip updated: ${clip.displayName}")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error updating clip")
                _error.value = "Failed to update clip"
            }
        }
    }

    // Audio track operations
    fun addAudioTrack(track: AudioTrack) {
        viewModelScope.launch {
            try {
                val currentTracks = _audioTracks.value.orEmpty().toMutableList()
                currentTracks.add(track)
                _audioTracks.value = currentTracks
                _hasUnsavedChanges.value = true
                Timber.d("Audio track added: ${track.name}")
            } catch (e: Exception) {
                Timber.e(e, "Error adding audio track")
                _error.value = "Failed to add audio track"
            }
        }
    }

    fun removeAudioTrack(trackId: String) {
        viewModelScope.launch {
            try {
                val currentTracks = _audioTracks.value.orEmpty().toMutableList()
                currentTracks.removeAll { it.id == trackId }
                _audioTracks.value = currentTracks
                _hasUnsavedChanges.value = true
                Timber.d("Audio track removed: $trackId")
            } catch (e: Exception) {
                Timber.e(e, "Error removing audio track")
                _error.value = "Failed to remove audio track"
            }
        }
    }

    fun updateAudioTrack(track: AudioTrack) {
        viewModelScope.launch {
            try {
                val currentTracks = _audioTracks.value.orEmpty().toMutableList()
                val index = currentTracks.indexOfFirst { it.id == track.id }
                if (index >= 0) {
                    currentTracks[index] = track
                    _audioTracks.value = currentTracks
                    _hasUnsavedChanges.value = true
                    Timber.d("Audio track updated: ${track.name}")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error updating audio track")
                _error.value = "Failed to update audio track"
            }
        }
    }

    // Project operations
    fun saveProject() {
        viewModelScope.launch {
            try {
                _isLoading.value = true
                Timber.d("Saving project...")

                // In real app, would save to database
                _hasUnsavedChanges.value = false

                Timber.d("Project saved successfully")
            } catch (e: Exception) {
                Timber.e(e, "Error saving project")
                _error.value = "Failed to save project: ${e.message}"
            } finally {
                _isLoading.value = false
            }
        }
    }

    fun discardChanges() {
        viewModelScope.launch {
            try {
                Timber.d("Discarding changes...")
                _hasUnsavedChanges.value = false
                // Reload from database
            } catch (e: Exception) {
                Timber.e(e, "Error discarding changes")
            }
        }
    }

    fun clearError() {
        _error.value = null
    }

    override fun onCleared() {
        super.onCleared()
        Timber.d("ProjectViewModel cleared")
    }
}
