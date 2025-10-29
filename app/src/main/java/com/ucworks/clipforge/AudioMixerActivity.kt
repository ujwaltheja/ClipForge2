package com.ucworks.clipforge

import android.os.Bundle
import android.widget.SeekBar
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.ucworks.clipforge.databinding.ActivityAudioMixerBinding
import kotlinx.coroutines.launch
import timber.log.Timber

/**
 * Activity for audio mixing and track management.
 * Allows adjustment of volume, EQ, and effects for each audio track.
 */
class AudioMixerActivity : AppCompatActivity() {

    private lateinit var binding: ActivityAudioMixerBinding
    private var enginePtr: Long = 0L

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        binding = ActivityAudioMixerBinding.inflate(layoutInflater)
        setContentView(binding.root)

        enginePtr = intent.getLongExtra("enginePtr", 0L)

        setupToolbar()
        setupMasterVolumeControl()
        setupAudioTracksList()
        setupActionButtons()
    }

    private fun setupToolbar() {
        setSupportActionBar(binding.toolbar)
        supportActionBar?.setDisplayHomeAsUpEnabled(true)
        supportActionBar?.title = getString(R.string.audio_mixing)
    }

    private fun setupMasterVolumeControl() {
        binding.masterVolume.setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean) {
                if (fromUser) {
                    val volume = progress / 100.0f
                    Timber.d("Master volume changed to: $volume")
                    updateMasterVolume(volume)
                }
            }

            override fun onStartTrackingTouch(seekBar: SeekBar?) {}
            override fun onStopTrackingTouch(seekBar: SeekBar?) {}
        })

        binding.masterVolume.progress = 100  // Default to full volume
    }

    private fun setupAudioTracksList() {
        binding.audioTracksList.apply {
            layoutManager = LinearLayoutManager(this@AudioMixerActivity)
            // Set adapter here - would be AudioTrackAdapter
            setHasFixedSize(true)
        }

        loadAudioTracks()
    }

    private fun setupActionButtons() {
        binding.btnAddTrack.setOnClickListener {
            Timber.d("Add track button clicked")
            showAddTrackDialog()
        }

        binding.btnDone.setOnClickListener {
            Timber.d("Done button clicked - closing audio mixer")
            finish()
        }
    }

    private fun loadAudioTracks() {
        lifecycleScope.launch {
            try {
                Timber.d("Loading audio tracks from engine")
                // Query engine for existing audio tracks
                // Update RecyclerView with track list
                val mockTracks = listOf(
                    AudioTrackInfo("Main Audio", "main", 1.0f),
                    AudioTrackInfo("Voiceover", "voiceover", 0.8f),
                    AudioTrackInfo("Music", "music", 0.6f)
                )
                Timber.d("Loaded ${mockTracks.size} audio tracks")
            } catch (e: Exception) {
                Timber.e(e, "Error loading audio tracks")
            }
        }
    }

    private fun updateMasterVolume(volume: Float) {
        lifecycleScope.launch {
            try {
                Timber.d("Updating master volume to: $volume")
                // Update all tracks proportionally or send to engine master control
            } catch (e: Exception) {
                Timber.e(e, "Error updating master volume")
            }
        }
    }

    fun updateTrackVolume(trackId: String, volume: Float) {
        lifecycleScope.launch {
            try {
                Timber.d("Updating track volume: trackId=$trackId, volume=$volume")
                val success = NativeLib.setAudioTrackVolume(enginePtr, trackId, volume)
                if (success) {
                    Timber.d("Track volume updated successfully")
                } else {
                    Timber.e("Failed to update track volume")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error updating track volume")
            }
        }
    }

    fun toggleTrackMute(trackId: String, muted: Boolean) {
        lifecycleScope.launch {
            try {
                Timber.d("Toggling track mute: trackId=$trackId, muted=$muted")
                val success = NativeLib.setAudioTrackMuted(enginePtr, trackId, muted)
                if (success) {
                    Timber.d("Track mute toggled successfully")
                } else {
                    Timber.e("Failed to toggle track mute")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error toggling track mute")
            }
        }
    }

    private fun showAddTrackDialog() {
        Timber.d("Showing add track dialog")
        // Implementation would show a dialog to select track type and name
        val trackType = "music"  // Example
        val trackName = "New Music Track"
        addAudioTrack(trackName, trackType)
    }

    private fun addAudioTrack(name: String, type: String) {
        lifecycleScope.launch {
            try {
                Timber.d("Adding new audio track: name=$name, type=$type")
                val trackId = NativeLib.addAudioTrack(enginePtr, name, type)
                if (trackId.isNotEmpty()) {
                    Timber.d("Audio track added successfully: $trackId")
                    loadAudioTracks()  // Refresh the list
                } else {
                    Timber.e("Failed to add audio track")
                }
            } catch (e: Exception) {
                Timber.e(e, "Error adding audio track")
            }
        }
    }

    override fun onSupportNavigateUp(): Boolean {
        finish()
        return true
    }

    data class AudioTrackInfo(
        val name: String,
        val type: String,
        val volume: Float
    )
}
