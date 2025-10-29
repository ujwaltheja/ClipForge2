package com.ucworks.clipforge.viewmodels

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * ViewModel for timeline editor state
 * Handles:
 * - Playback state
 * - Timeline position
 * - Selected clip
 * - Undo/redo operations
 */
class EditorViewModel : ViewModel() {

    // Playback state
    private val _isPlaying = MutableLiveData(false)
    val isPlaying: LiveData<Boolean> = _isPlaying

    // Timeline position
    private val _currentPosition = MutableLiveData(0L)
    val currentPosition: LiveData<Long> = _currentPosition

    private val _timelineDuration = MutableLiveData(0L)
    val timelineDuration: LiveData<Long> = _timelineDuration

    // Selection
    private val _selectedClipId = MutableLiveData<String?>()
    val selectedClipId: LiveData<String?> = _selectedClipId

    // UI state
    private val _activeToolTab = MutableLiveData(0)  // 0=Main, 1=Effects, 2=Library
    val activeToolTab: LiveData<Int> = _activeToolTab

    private val _isLoading = MutableLiveData(false)
    val isLoading: LiveData<Boolean> = _isLoading

    // Undo/Redo
    private val undoStack = mutableListOf<EditorState>()
    private val redoStack = mutableListOf<EditorState>()
    private val maxUndoHistory = 50

    private data class EditorState(
        val position: Long,
        val selectedClipId: String?
    )

    // Playback controls
    fun startPlayback() {
        viewModelScope.launch {
            try {
                _isPlaying.value = true
                Timber.d("Playback started")
            } catch (e: Exception) {
                Timber.e(e, "Error starting playback")
            }
        }
    }

    fun pausePlayback() {
        viewModelScope.launch {
            try {
                _isPlaying.value = false
                Timber.d("Playback paused")
            } catch (e: Exception) {
                Timber.e(e, "Error pausing playback")
            }
        }
    }

    fun stopPlayback() {
        viewModelScope.launch {
            try {
                _isPlaying.value = false
                _currentPosition.value = 0L
                Timber.d("Playback stopped")
            } catch (e: Exception) {
                Timber.e(e, "Error stopping playback")
            }
        }
    }

    // Timeline control
    fun seek(positionMs: Long) {
        viewModelScope.launch {
            try {
                _currentPosition.value = positionMs.coerceIn(0, _timelineDuration.value ?: 0)
                Timber.d("Seeked to: ${_currentPosition.value}ms")
            } catch (e: Exception) {
                Timber.e(e, "Error seeking")
            }
        }
    }

    fun updatePosition(positionMs: Long) {
        _currentPosition.value = positionMs.coerceIn(0, _timelineDuration.value ?: 0)
    }

    fun setTimelineDuration(durationMs: Long) {
        _timelineDuration.value = durationMs
        Timber.d("Timeline duration set to: ${durationMs}ms")
    }

    // Clip selection
    fun selectClip(clipId: String?) {
        saveState()  // Save for undo
        _selectedClipId.value = clipId
        Timber.d("Clip selected: $clipId")
    }

    fun deselectClip() {
        selectClip(null)
    }

    // Tool panel
    fun setToolTab(tabIndex: Int) {
        _activeToolTab.value = tabIndex
        Timber.d("Tool tab changed to: $tabIndex")
    }

    // Undo/Redo
    fun undo() {
        viewModelScope.launch {
            try {
                if (undoStack.isNotEmpty()) {
                    saveToRedo()
                    val previousState = undoStack.removeAt(undoStack.size - 1)
                    restoreState(previousState)
                    Timber.d("Undo performed")
                } else {
                    Timber.w("No undo history available")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error performing undo")
            }
        }
    }

    fun redo() {
        viewModelScope.launch {
            try {
                if (redoStack.isNotEmpty()) {
                    saveToUndo()
                    val nextState = redoStack.removeAt(redoStack.size - 1)
                    restoreState(nextState)
                    Timber.d("Redo performed")
                } else {
                    Timber.w("No redo history available")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error performing redo")
            }
        }
    }

    fun canUndo(): Boolean = undoStack.isNotEmpty()

    fun canRedo(): Boolean = redoStack.isNotEmpty()

    // Private helper methods
    private fun saveState() {
        val state = EditorState(
            position = _currentPosition.value ?: 0L,
            selectedClipId = _selectedClipId.value
        )
        undoStack.add(state)

        // Limit undo history
        if (undoStack.size > maxUndoHistory) {
            undoStack.removeAt(0)
        }

        // Clear redo stack when new action is performed
        redoStack.clear()
    }

    private fun saveToUndo() {
        val state = EditorState(
            position = _currentPosition.value ?: 0L,
            selectedClipId = _selectedClipId.value
        )
        undoStack.add(state)
    }

    private fun saveToRedo() {
        val state = EditorState(
            position = _currentPosition.value ?: 0L,
            selectedClipId = _selectedClipId.value
        )
        redoStack.add(state)
    }

    private fun restoreState(state: EditorState) {
        _currentPosition.value = state.position
        _selectedClipId.value = state.selectedClipId
        Timber.d("State restored: position=${state.position}, clip=${state.selectedClipId}")
    }

    override fun onCleared() {
        super.onCleared()
        undoStack.clear()
        redoStack.clear()
        Timber.d("EditorViewModel cleared")
    }
}
